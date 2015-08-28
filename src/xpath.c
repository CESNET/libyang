/**
 * @file xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>

#include "libyang.h"
#include "xml.h"
#include "tree.h"
#include "common.h"
#include "dict.h"

/* expression tokens allocation */
#define LYXP_EXPR_SIZE_START 10
#define LYXP_EXPR_SIZE_STEP 5

/* XPath matches allocation */
#define LYXP_SET_SIZE_START 2
#define LYXP_SET_SIZE_STEP 2

/*
 * PARSED GRAMMAR
 *
 * Full axes are not supported, abbreviated forms must be used,
 * variables are not supported, "id()" function is not supported.
 * Undefined rules and constants are tokens.
 *
 * Otherwise the grammar from http://www.w3.org/TR/1999/REC-xpath-19991116/.
 *
 * Modified full grammar:
 *
 * [1] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [2] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 * [3] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [4] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [5] NodeTest ::= NameTest | NodeType '(' ')' | 'processing-instruction' '(' Literal ')'
 * [6] Predicate ::= '[' Expr ']'
 * [7] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 * [8] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 * [9] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [10] Expr ::= AndExpr | Expr 'or' AndExpr
 * [11] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 * [12] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 * [13] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 * [14] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 * [15] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 * [16] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [17] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 */

enum lyxp_token {
    LYXP_TOKEN_NONE = 0,
    LYXP_TOKEN_PAR1,          /* '(' */
    LYXP_TOKEN_PAR2,          /* ')' */
    LYXP_TOKEN_BRACK1,        /* '[' */
    LYXP_TOKEN_BRACK2,        /* ']' */
    LYXP_TOKEN_DOT,           /* '.' */
    LYXP_TOKEN_DDOT,          /* '..' */
    LYXP_TOKEN_AT,            /* '@' */
    LYXP_TOKEN_COMMA,         /* ',' */
    /* LYXP_TOKEN_DCOLON,      * '::' * axes not supported */
    LYXP_TOKEN_NAMETEST,      /* NameTest */
    LYXP_TOKEN_NODETYPE,      /* NodeType */
    LYXP_TOKEN_FUNCNAME,      /* FunctionName */
    LYXP_TOKEN_OPERATOR_LOG,  /* Operator 'and', 'or' */
    LYXP_TOKEN_OPERATOR_COMP, /* Operator '=', '!=', '<', '<=', '>', '>=' */
    LYXP_TOKEN_OPERATOR_MATH, /* Operator '+', '-', '*', 'div', 'mod', '-' (unary) */
    LYXP_TOKEN_OPERATOR_UNI,  /* Operator '|' */
    LYXP_TOKEN_OPERATOR_PATH, /* Operator '/', '//' */
    /* LYXP_TOKEN_AXISNAME,    * AxisName * axes not supported */
    LYXP_TOKEN_LITERAL,       /* Literal - with both single and double quote */
    LYXP_TOKEN_NUMBER         /* Number */
};

struct lyxp_expr {
    enum lyxp_token *tokens; /* array of tokens */
    uint16_t *expr_pos;      /* array of pointers to the expression in expr (idx of the beginning) */
    uint8_t *tok_len;        /* array of token lengths in expr */
    uint16_t used;       /* used array items */
    uint16_t size;       /* allocated array items */

    const char *expr;        /* the original XPath expression */
};

enum lyxp_set_type {
    LYXP_SET_NODE_SET,
    LYXP_SET_ATTR_SET,
    LYXP_SET_BOOLEAN,
    LYXP_SET_NUMBER,
    LYXP_SET_STRING
};

struct lyxp_set {
    enum lyxp_set_type type;  /* LYXP_SET_STRING or LYXP_SET_NUMBER or LYXP_SET_BOOLEAN */
    union {
        const char *str;
        long double num;
        int bool;
    } value;
};

struct lyxp_set_node {
    enum lyxp_set_type type;  /* LYXP_SET_NODE_SET or LYXP_SET_ATTR_SET */
    union {
        struct lyd_node **nodes;
        struct lyd_attr **attrs;
    } value;
    uint16_t used;
    uint16_t size;
};

static void
set_free(struct lyxp_set *set, struct ly_ctx *ctx)
{
    if (!set) {
        return;
    }
    assert(ctx);

    if ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_ATTR_SET)) {
        free(((struct lyxp_set_node *)set)->value.nodes);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }
    free(set);
}

static struct lyxp_set *
set_copy(struct lyxp_set *set, struct ly_ctx *ctx)
{
    struct lyxp_set *ret;
    struct lyxp_set_node *ret_node, *set_node;

    if ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_ATTR_SET)) {
        set_node = (struct lyxp_set_node *)set;

        ret_node = malloc(sizeof *ret_node);
        ret_node->type = set->type;
        ret_node->value.nodes = malloc(set_node->used * sizeof *ret_node->value.nodes);
        memcpy(ret_node->value.nodes, set_node->value.nodes, set_node->used * sizeof *ret_node->value.nodes);
        ret_node->used = ret_node->size = set_node->used;

        ret = (struct lyxp_set *)ret_node;
    } else {
       ret = malloc(sizeof *ret);
       memcpy(ret, set, sizeof *ret);
       if (set->type == LYXP_SET_STRING) {
           ret->value.str = lydict_insert(ctx, set->value.str, 0);
       }
    }

    return ret;
}

static void
set_fill_string(struct lyxp_set *set, const char *string, uint16_t str_len, struct ly_ctx *ctx)
{
    set->type = LYXP_SET_STRING;
    set->value.str = lydict_insert(ctx, string, str_len);
}

static void
set_fill_number(struct lyxp_set *set, long double number)
{
    set->type = LYXP_SET_NUMBER;
    set->value.num = number;
}

static void
set_fill_boolean(struct lyxp_set *set, int boolean)
{
    set->type = LYXP_SET_BOOLEAN;
    set->value.bool = boolean;
}

static void /* TODO */
set_add_node(struct lyxp_set *set, struct lyd_node *node, uint16_t idx)
{
    struct lyxp_set_node *set_node = (struct lyxp_set_node *)set;

    if (!set_node->value.nodes) {
        set_node->type = LYXP_SET_NODE_SET;
        set_node->value.nodes = calloc(LYXP_SET_SIZE_START, sizeof *set_node->value.nodes);
        set_node->value.nodes[0] = node;
        set_node->used = 1;
        set_node->size = LYXP_SET_SIZE_START;
    } else {
        if (set_node->used == set_node->size) {
            set_node->value.nodes = realloc(set_node->value.nodes,
                                          (set_node->size + LYXP_SET_SIZE_STEP) * sizeof *set_node->value.nodes);
            set_node->size += LYXP_SET_SIZE_STEP;
        }
        set_node->value.nodes[set_node->used] = node;
        ++set_node->used;
    }
}

static void
set_remove_node(struct lyxp_set *set, uint16_t idx)
{
    struct lyxp_set_node *set_node = (struct lyxp_set_node *)set;

    assert(idx < set_node->used);

    --set_node->used;
    memmove(&set_node->value.nodes[idx], &set_node->value.nodes[idx + 1], set_node->used - idx);
}

static const char *
print_token(enum lyxp_token tok)
{
    switch (tok) {
    case LYXP_TOKEN_PAR1:
        return "(";
    case LYXP_TOKEN_PAR2:
        return ")";
    case LYXP_TOKEN_BRACK1:
        return "[";
    case LYXP_TOKEN_BRACK2:
        return "]";
    case LYXP_TOKEN_DOT:
        return ".";
    case LYXP_TOKEN_DDOT:
        return "..";
    case LYXP_TOKEN_AT:
        return "@";
    case LYXP_TOKEN_COMMA:
        return ",";
    case LYXP_TOKEN_NAMETEST:
        return "NameTest";
    case LYXP_TOKEN_NODETYPE:
        return "NodeType";
    case LYXP_TOKEN_FUNCNAME:
        return "FunctionName";
    case LYXP_TOKEN_OPERATOR_LOG:
        return "Operator(Logic)";
    case LYXP_TOKEN_OPERATOR_COMP:
        return "Operator(Comparison)";
    case LYXP_TOKEN_OPERATOR_MATH:
        return "Operator(Math)";
    case LYXP_TOKEN_OPERATOR_UNI:
        return "Operator(Union)";
    case LYXP_TOKEN_OPERATOR_PATH:
        return "Operator(Path)";
    case LYXP_TOKEN_LITERAL:
        return "Literal";
    case LYXP_TOKEN_NUMBER:
        return "Number";
    default:
        LOGINT;
        return "";
    }
}

static void
debug_print_expr_struct(struct lyxp_expr *exp)
{
    uint16_t i;

    if (!exp) {
        return;
    }

    LOGDBG("XPATH: expression \"%s\":", exp->expr);
    for (i = 0; i < exp->used; ++i) {
        LOGDBG("XPATH:\tToken %s, in expression \"%.*s\"", print_token(exp->tokens[i]), exp->tok_len[i], &exp->expr[exp->expr_pos[i]]);
    }
}

static void
add_expr_tok(struct lyxp_expr *exp, enum lyxp_token token, uint16_t expr_pos, uint16_t tok_len)
{
    if (exp->used == exp->size) {
        exp->size += LYXP_EXPR_SIZE_STEP;
        exp->tokens = realloc(exp->tokens, exp->size * sizeof *exp->tokens);
        exp->expr_pos = realloc(exp->expr_pos, exp->size * sizeof *exp->expr_pos);
        exp->tok_len = realloc(exp->tok_len, exp->size * sizeof *exp->tok_len);
    }

    exp->tokens[exp->used] = token;
    exp->expr_pos[exp->used] = expr_pos;
    exp->tok_len[exp->used] = tok_len;
    ++exp->used;
}

static uint16_t
parse_ncname(const char *ncname)
{
    uint16_t parsed = 0;
    int uc;
    unsigned int size;

    uc = lyxml_getutf8(&ncname[parsed], &size);
    if (!is_xmlnamestartchar(uc) || (uc == ':')) {
       return parsed;
    }

    do {
        parsed += size;
        if (!ncname[parsed]) {
            break;
        }
        uc = lyxml_getutf8(&ncname[parsed], &size);
    } while (is_xmlnamechar(uc) && (uc != ':'));

    return parsed;
}

/**
 * @brief Parses an XPath expression into a structure. Logs directly.
 *
 * http://www.w3.org/TR/1999/REC-xpath-19991116/ section 3.7
 *
 * @param[in] expr XPath expression to parse.
 * @param[in] line Line of the expression in the input file.
 *
 * @return Filled expression structure or NULL on error.
 */
static struct lyxp_expr *
parse_expr(const char *expr, uint32_t line)
{
    struct lyxp_expr *ret;
    uint16_t parsed = 0, tok_len, ncname_len;
    enum lyxp_token tok_type;
    int prev_function_check = 0;

    /* init lyxp_expr structure */
    ret = malloc(sizeof *ret);
    ret->expr = expr;
    ret->used = 0;
    ret->size = LYXP_EXPR_SIZE_START;
    ret->tokens = malloc(ret->size * sizeof *ret->tokens);
    ret->expr_pos = malloc(ret->size * sizeof *ret->expr_pos);
    ret->tok_len = malloc(ret->size * sizeof *ret->tok_len);

    while (is_xmlws(expr[parsed])) {
        ++parsed;
    }

    do {
        if (expr[parsed] == '(') {

            /* '(' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR1;

            if (prev_function_check && ret->used && (ret->tokens[ret->used - 1] == LYXP_TOKEN_NAMETEST)) {
                /* it is a NodeType/FunctionName after all */
                if (((ret->tok_len[ret->used - 1] == 4)
                        && (!strncmp(&expr[ret->expr_pos[ret->used - 1]], "node", 4)
                        || !strncmp(&expr[ret->expr_pos[ret->used - 1]], "text", 4))) ||
                        ((ret->tok_len[ret->used - 1] == 7)
                        && !strncmp(&expr[ret->expr_pos[ret->used - 1]], "comment", 7)) ||
                        ((ret->tok_len[ret->used - 1] == 22)
                        && !strncmp(&expr[ret->expr_pos[ret->used - 1]], "processing-instruction", 22))) {
                    ret->tokens[ret->used - 1] = LYXP_TOKEN_NODETYPE;
                } else {
                    ret->tokens[ret->used - 1] = LYXP_TOKEN_FUNCNAME;
                }
                prev_function_check = 0;
            }

        } else if (expr[parsed] == ')') {

            /* ')' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR2;

        } else if (expr[parsed] == '[') {

            /* '[' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK1;

        } else if (expr[parsed] == ']') {

            /* ']' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK2;

        } else if (!strncmp(&expr[parsed], "..", 2)) {

            /* '..' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_DDOT;

        } else if ((expr[parsed] == '.') && (!isdigit(expr[parsed+1]))) {

            /* '.' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_DOT;

        } else if (expr[parsed] == '@') {

            /* '@' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_AT;

        } else if (expr[parsed] == ',') {

            /* ',' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_COMMA;

        } else if (expr[parsed] == '\'') {

            /* Literal with ' */
            for (tok_len = 1; expr[parsed + tok_len] != '\''; ++tok_len);
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if (expr[parsed] == '\"') {

            /* Literal with " */
            for (tok_len = 1; expr[parsed + tok_len] != '\"'; ++tok_len);
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if ((expr[parsed] == '.') || (isdigit(expr[parsed]))) {

            /* Number */
            for (tok_len = 0; isdigit(expr[parsed + tok_len]); ++tok_len);
            if (expr[parsed + tok_len] == '.') {
                ++tok_len;
                for (; isdigit(expr[parsed + tok_len]); ++tok_len);
            }
            tok_type = LYXP_TOKEN_NUMBER;

        } else if (expr[parsed] == '/') {

            /* Operator '/', '//' */
            if (!strncmp(&expr[parsed], "//", 2)) {
                tok_len = 2;
            } else {
                tok_len = 1;
            }
            tok_type = LYXP_TOKEN_OPERATOR_PATH;

        } else if  (!strncmp(&expr[parsed], "!=", 2) || !strncmp(&expr[parsed], "<=", 2)
                || !strncmp(&expr[parsed], ">=", 2)) {

            /* Operator '!=', '<=', '>=' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_OPERATOR_COMP;

        } else if (expr[parsed] == '|') {

            /* Operator '|' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_UNI;

        } else if ((expr[parsed] == '+') || (expr[parsed] == '-')) {

            /* Operator '+', '-' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_MATH;

        } else if ((expr[parsed] == '=') || (expr[parsed] == '<') || (expr[parsed] == '>')) {

            /* Operator '=', '<', '>' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_COMP;

        } else if (ret->used && (ret->tokens[ret->used - 1] != LYXP_TOKEN_AT)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_PAR1)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_BRACK1)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_COMMA)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_LOG)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_COMP)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_MATH)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_UNI)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_PATH)) {

            /* Operator '*', 'or', 'and', 'mod', or 'div' */
            if (expr[parsed] == '*') {
                tok_len = 1;
                tok_type = LYXP_TOKEN_OPERATOR_MATH;

            } else if (!strncmp(&expr[parsed], "or", 2)) {
                tok_len = 2;
                tok_type = LYXP_TOKEN_OPERATOR_LOG;

            } else if (!strncmp(&expr[parsed], "and", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPERATOR_LOG;

            } else if (!strncmp(&expr[parsed], "mod", 3) || !strncmp(&expr[parsed], "div", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPERATOR_MATH;

            } else {
                LOGVAL(LYE_INCHAR, line, expr[parsed], &expr[parsed]);
                goto error;
            }
        } else if (expr[parsed] == '*') {

            /* NameTest '*' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_NAMETEST;

        } else {

            /* NameTest (NCName ':' '*' | QName) or NodeType/FunctionName */
            ncname_len = parse_ncname(&expr[parsed]);
            if (!ncname_len) {
                LOGVAL(LYE_INCHAR, line, expr[parsed], &expr[parsed]);
                goto error;
            }
            tok_len = ncname_len;

            if (expr[parsed + tok_len] == ':') {
                ++tok_len;
                if (expr[parsed + tok_len] == '*') {
                    ++tok_len;
                } else {
                    ncname_len = parse_ncname(&expr[parsed + tok_len]);
                    if (!ncname_len) {
                        LOGVAL(LYE_INCHAR, line, expr[parsed], &expr[parsed]);
                        goto error;
                    }
                    tok_len += ncname_len;
                }
                /* remove old flag to prevent ambiguities */
                prev_function_check = 0;
                tok_type = LYXP_TOKEN_NAMETEST;
            } else {
                /* there is no prefix so it can still be NodeType/FunctioName, we can't finally decide now */
                prev_function_check = 1;
                tok_type = LYXP_TOKEN_NAMETEST;
            }
        }

        /* store the token, move on to the next one */
        add_expr_tok(ret, tok_type, parsed, tok_len);
        parsed += tok_len;
        while (is_xmlws(expr[parsed])) {
            ++parsed;
        }

    } while (expr[parsed]);

    return ret;

error:
    free(ret->tokens);
    free(ret->expr_pos);
    free(ret->tok_len);
    free(ret);

    return NULL;
}

/*
 * XPath functions
 */
static int
xpath_boolean(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_ceiling(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_comment(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_concat(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_contains(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
               uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_count(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_current(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_false(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_floor(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_lang(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_last(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_local_name(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                 uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_name(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_namespace_uri(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                    uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_node(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_normalize_space(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                      uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_not(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_number(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_position(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
               uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_processing_instruction(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node,
                             struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_round(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_starts_with(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                  uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_string(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_string_length(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                    uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_substring(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_substring_after(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                      uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_substring_before(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                       uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_sum(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_text(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_translate(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);
    return EXIT_SUCCESS;
}

static int
xpath_true(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    LOGDBG("XPATH: %s call", __func__);

    set_fill_boolean(set, 1);
    return EXIT_SUCCESS;
}

/*
 * moveto functions
 */

/* absolute '/' or '//' */
static int
moveto_root(struct lyxp_set *set, uint32_t line)
{
    struct lyxp_set_node *set_node = (struct lyxp_set_node *)set;
    uint16_t i;

    if (!set) {
        return EXIT_SUCCESS;
    }

    /* move the first node to the root */
    for (; set_node->value.nodes[0]->parent; set_node->value.nodes[0] = set_node->value.nodes[0]->parent);

    /* there is only one root node */
    assert(set_node->value.nodes[0]->prev == set_node->value.nodes[0]);

    /* delete the rest */
    for (i = set_node->used - 1; i > 0 ; --i) {
        set_remove_node(set, i);
    }

    return EXIT_SUCCESS;
}

/* '/' or '//' and '*' or 'NAME' or TODO 'PREFIX:*' or 'PREFIX:NAME' */
static int
moveto_node(const char *qname, uint16_t qname_len, struct lyxp_set *set, int all_desc, uint32_t line)
{
    uint16_t i;
    int replaced, all = 0;
    struct lyd_node *sub;
    struct lyxp_set_node *set_node = (struct lyxp_set_node *)set;

    if (!set) {
        return EXIT_SUCCESS;
    }

    /* TODO */
    assert(!strnchr(qname, ':', qname_len));

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set_node->used; ) {
        replaced = 0;

        LY_TREE_FOR(set_node->value.nodes[i]->child, sub) {
            if (all || (!strncmp(sub->schema->name, qname, qname_len) && !sub->schema->name[qname_len])) {
                /* match */
                if (!replaced) {
                    set_node->value.nodes[i] = sub;
                    replaced = 1;
                } else {
                    set_add_node(set, sub, i + 1);
                    ++i;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        } else {
            ++i;
        }
    }
    return EXIT_SUCCESS;
}

/* '/' or '//' and '@*' or '@NAME' or TODO '@PREFIX:*' or '@PREFIX:NAME' */
static int
moveto_attr(const char *qname, uint16_t qname_len, struct lyxp_set *set, int all_desc, uint32_t line)
{
    uint16_t i;
    int replaced, all = 0;
    struct lyd_attr *sub;
    struct lyxp_set_node *set_node = (struct lyxp_set_node *)set;

    if (!set) {
        return EXIT_SUCCESS;
    }

    /* TODO */
    assert(!strnchr(qname, ':', qname_len));

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set_node->used; ) {
        replaced = 0;

        LY_TREE_FOR(set_node->value.nodes[i]->attr, sub) {
            if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                /* match */
                /* HACK we're temporarily assigning lyd_attr * into lyd_node *, but it's a union of those two */
                if (!replaced) {
                    set_node->value.nodes[i] = (struct lyd_node *)sub;
                    replaced = 1;
                } else {
                    set_add_node(set, (struct lyd_node *)sub, i + 1);
                    ++i;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        } else {
            ++i;
        }
    }
    set_node->type = LYXP_SET_ATTR_SET;

    return EXIT_SUCCESS;
}

/* '/' or TODO '//' and '..' */
static int
moveto_parent(struct lyxp_set *set, int all_desc, uint32_t line)
{
    uint16_t i;
    struct lyxp_set_node *set_node = (struct lyxp_set_node *)set;

    if (!set) {
        return EXIT_SUCCESS;
    }

    for (i = 0; i < set_node->used; ) {
        if (!set_node->value.nodes[i]->parent) {
            set_remove_node(set, i);
        } else {
            set_node->value.nodes[i] = set_node->value.nodes[i]->parent;
            ++i;
        }
    }

    return EXIT_SUCCESS;
}

/* '/' or TODO '//' and '.' */
static int
moveto_self(struct lyxp_set *set, int all_desc, uint32_t line)
{
    /* nothing to do */
    if (!set || !all_desc) {
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

/* TODO '|', result is in set1, set2 is freed */
static int
moveto_union(struct lyxp_set *set1, struct lyxp_set *set2, struct ly_ctx *ctx)
{
    return EXIT_SUCCESS;
}

/* TODO unary '-' */
static int
moveto_unary_minus(struct lyxp_set *set, struct ly_ctx *ctx)
{
    return EXIT_SUCCESS;
}

/*
 * eval functions
 */
static int eval_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                     uint32_t line);

static int
check_token(struct lyxp_expr *exp, uint16_t cur_exp, enum lyxp_token want_tok, uint32_t line)
{
    if (exp->used == cur_exp) {
        LOGVAL(LYE_XPATH_EOF, line);
        return -1;
    }

    if (want_tok && (exp->tokens[cur_exp] != want_tok)) {
        LOGVAL(LYE_XPATH_INTOK, line, print_token(exp->tokens[cur_exp]), &exp->expr[exp->expr_pos[cur_exp]]);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates Literal. Logs directly on error.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] ctx libyang context with the dictionary.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_literal(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyxp_set *set, struct ly_ctx *ctx, uint32_t line)
{
    if (check_token(exp, *cur_exp, LYXP_TOKEN_LITERAL, line)) {
        return -1;
    }

    if (set) {
        set_fill_string(set, &exp->expr[exp->expr_pos[*cur_exp] + 1], exp->tok_len[*cur_exp] - 1, ctx);
    }
    LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
    ++(*cur_exp);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates NodeTest. Logs directly on error.
 *
 * [5] NodeTest ::= NameTest | NodeType '(' ')' | 'processing-instruction' '(' Literal ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in] attr_axis Whether to search attributes or standard nodes.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_node_test(struct lyxp_expr *exp, uint16_t *cur_exp, int attr_axis, int all_desc, struct lyxp_set *set,
               uint32_t line)
{
    int nodetype_exp, rc;
    struct lyxp_set *set_literal = NULL;
    struct ly_ctx *ctx = NULL;

    assert(!set || (set->type == LYXP_SET_NODE_SET));

    if (set) {
        ctx = ((struct lyxp_set_node *)set)->value.nodes[0]->schema->module->ctx;
    }

    if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
        return -1;
    }

    switch (exp->tokens[*cur_exp]) {
    case LYXP_TOKEN_NAMETEST:
        if (!attr_axis && (rc = moveto_node(&exp->expr[exp->expr_pos[*cur_exp]], exp->tok_len[*cur_exp], set, all_desc, line))) {
            return rc;
        }
        if (attr_axis && (rc = moveto_attr(&exp->expr[exp->expr_pos[*cur_exp]], exp->tok_len[*cur_exp], set, all_desc, line))) {
            return rc;
        }
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);
        break;
    case LYXP_TOKEN_NODETYPE:
        nodetype_exp = (signed)*cur_exp;

        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        /* '(' */
        if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR1, line)) {
            return -1;
        }
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        /* 'processing-instruction' '(' Literal ')' */
        if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
            return -1;
        }
        if ((exp->tok_len[nodetype_exp] == 22) || (exp->tokens[*cur_exp] == LYXP_TOKEN_LITERAL)) {
            if (set) {
                set_literal = calloc(1, sizeof *set_literal);
            }
            if ((rc = eval_literal(exp, cur_exp, set_literal, ctx, line))) {
                set_free(set_literal, ctx);
                return rc;
            }
        }

        /* ')' */
        if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR2, line)) {
            set_free(set_literal, ctx);
            return -1;
        }
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        if (set) {
            if (exp->tok_len[nodetype_exp] == 4) {
                if (!strncmp(&exp->expr[exp->expr_pos[nodetype_exp]], "node", 4)) {
                    if (xpath_node(NULL, 0, NULL, set, line)) {
                        return -1;
                    }
                } else {
                    assert(!strncmp(&exp->expr[exp->expr_pos[nodetype_exp]], "text", 4));
                    if (xpath_text(NULL, 0, NULL, set, line)) {
                        return -1;
                    }
                }
            } else if (exp->tok_len[nodetype_exp] == 7) {
                assert(!strncmp(&exp->expr[exp->expr_pos[nodetype_exp]], "comment", 7));
                if (xpath_comment(NULL, 0, NULL, set, line)) {
                    return -1;
                }
            } else {
                assert(exp->tok_len[nodetype_exp] == 22);
                assert(!strncmp(&exp->expr[exp->expr_pos[nodetype_exp]], "processing-instruction", 22));
                if (xpath_processing_instruction(set_literal, (set_literal ? 1 : 0), NULL, set, line)) {
                    set_free(set_literal, ctx);
                    return -1;
                }
                set_free(set_literal, ctx);
            }
        }

        break;
    default:
        LOGVAL(LYE_XPATH_INTOK, line, print_token(exp->tokens[*cur_exp]), &exp->expr[exp->expr_pos[*cur_exp]]);
        return -1;
    }

    return EXIT_SUCCESS;
}

/** TODO
 * @brief Evaluates Predicate. Logs directly on error.
 *
 * [6] Predicate ::= '[' Expr ']'
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_predicate(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
               uint32_t line)
{
    int rc;

    if (check_token(exp, *cur_exp, LYXP_TOKEN_BRACK1, line)) {
        return -1;
    }
    LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
    ++(*cur_exp);

    if ((rc = eval_expr(exp, cur_exp, cur_node, set, line))) {
        return rc;
    }

    if (check_token(exp, *cur_exp, LYXP_TOKEN_BRACK2, line)) {
        return -1;
    }
    LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
    ++(*cur_exp);

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates RelativeLocationPath. Logs directly on error.
 *
 * [3] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [4] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_relative_location_path(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, int all_desc,
                            struct lyxp_set *set, uint32_t line)
{
    int attr_axis, rc;

    if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
        return -1;
    }

    goto step;
    do {
        /* evaluate '/' or '//' */
        if (exp->tok_len[*cur_exp] == 1) {
            all_desc = 0;
        } else {
            assert(exp->tok_len[*cur_exp] == 2);
            all_desc = 1;
        }
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
            return -1;
        }
step:
        /* Step */
        attr_axis = 0;
        switch (exp->tokens[*cur_exp]) {
        case LYXP_TOKEN_DOT:
            /* evaluate '.' */
            if ((rc = moveto_self(set, all_desc, line))) {
                return rc;
            }
            LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
            ++(*cur_exp);
            break;
        case LYXP_TOKEN_DDOT:
            /* evaluate '..' */
            if ((rc = moveto_parent(set, all_desc, line))) {
                return rc;
            }
            LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
            ++(*cur_exp);
            break;

        case LYXP_TOKEN_AT:
            /* evaluate '@' */
            attr_axis = 1;
            LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
            ++(*cur_exp);

            if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
                return -1;
            }
            if ((exp->tokens[*cur_exp] != LYXP_TOKEN_NAMETEST) && (exp->tokens[*cur_exp] != LYXP_TOKEN_NODETYPE)) {
                LOGVAL(LYE_XPATH_INTOK, line, print_token(exp->tokens[*cur_exp]), &exp->expr[exp->expr_pos[*cur_exp]]);
                return -1;
            }

            /* fall through */
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            if ((rc = eval_node_test(exp, cur_exp, attr_axis, all_desc, set, line))) {
                return rc;
            }
            while ((exp->used > *cur_exp) && (exp->tokens[*cur_exp] == LYXP_TOKEN_BRACK1)) {
                if ((rc = eval_predicate(exp, cur_exp, cur_node, set, line))) {
                    return rc;
                }
            }
            break;
        default:
            LOGVAL(LYE_XPATH_INTOK, line, print_token(exp->tokens[*cur_exp]), &exp->expr[exp->expr_pos[*cur_exp]]);
            return -1;
        }
    } while ((exp->used > *cur_exp) && (exp->tokens[*cur_exp] == LYXP_TOKEN_OPERATOR_PATH));

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates AbsoluteLocationPath. Logs directly on error.
 *
 * [2] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_absolute_location_path(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node,
                            struct lyxp_set *set, uint32_t line)
{
    int rc, all_desc;

    assert(!set || ((set->type == LYXP_SET_NODE_SET) && (((struct lyxp_set_node *)set)->used == 1)));

    if (check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_PATH, line)) {
        return -1;
    }

    if (set) {
        /* no matter what tokens follow, we need to be at the root */
        if ((rc = moveto_root(set, line))) {
            return rc;
        }
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*cur_exp] == 1) {
        /* evaluate '/' - deferred */
        all_desc = 0;
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, UINT_MAX)) {
            /* TODO this should be a kind of a new root or something weird */
            return EXIT_SUCCESS;
        }
        switch (exp->tokens[*cur_exp]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            if ((rc = eval_relative_location_path(exp, cur_exp, cur_node, all_desc, set, line))) {
                return rc;
            }
        default:
            /* TODO weird root again */
            break;
        }

    /* '//' RelativeLocationPath */
    } else {
        /* evaluate '//' - deferred so as not to waste memory by remembering all the nodes */
        all_desc = 1;
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        if ((rc = eval_relative_location_path(exp, cur_exp, cur_node, all_desc, set, line))) {
            return rc;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates FunctionCall. Logs directly on error.
 *
 * [8] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_function_call(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                   uint32_t line)
{
    int rc;
    int (*xpath_func)(struct lyxp_set *, uint16_t, struct lyd_node *, struct lyxp_set *, uint32_t) = NULL;
    uint16_t arg_count = 0, i;
    struct lyxp_set *args = NULL;

    if (check_token(exp, *cur_exp, LYXP_TOKEN_FUNCNAME, line)) {
        return -1;
    }

    if (set) {
        /* FunctionName */
        switch (exp->tok_len[*cur_exp]) {
        case 3:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "not", 3)) {
                xpath_func = xpath_not;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "sum", 3)) {
                xpath_func = xpath_sum;
            }
            break;
        case 4:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "lang", 4)) {
                xpath_func = xpath_lang;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "last", 4)) {
                xpath_func = xpath_last;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "name", 4)) {
                xpath_func = xpath_name;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "true", 4)) {
                xpath_func = xpath_true;
            }
            break;
        case 5:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "count", 5)) {
                xpath_func = xpath_count;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "false", 5)) {
                xpath_func = xpath_false;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "floor", 5)) {
                xpath_func = xpath_floor;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "round", 5)) {
                xpath_func = xpath_round;
            }
            break;
        case 6:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "concat", 6)) {
                xpath_func = xpath_concat;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "number", 6)) {
                xpath_func = xpath_number;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "string", 6)) {
                xpath_func = xpath_string;
            }
            break;
        case 7:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "boolean", 7)) {
                xpath_func = xpath_boolean;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "ceiling", 7)) {
                xpath_func = xpath_ceiling;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "current", 7)) {
                xpath_func = xpath_current;
            }
            break;
        case 8:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "contains", 8)) {
                xpath_func = xpath_contains;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "position", 8)) {
                xpath_func = xpath_position;
            }
            break;
        case 9:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "substring", 9)) {
                xpath_func = xpath_substring;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "translate", 9)) {
                xpath_func = xpath_translate;
            }
            break;
        case 10:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "local-name", 10)) {
                xpath_func = xpath_local_name;
            }
            break;
        case 11:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "starts-with", 11)) {
                xpath_func = xpath_starts_with;
            }
            break;
        case 13:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "namespace-uri", 13)) {
                xpath_func = xpath_namespace_uri;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "string-length", 13)) {
                xpath_func = xpath_string_length;
            }
            break;
        case 15:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "normalize-space", 15)) {
                xpath_func = xpath_normalize_space;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "substring-after", 15)) {
                xpath_func = xpath_substring_after;
            }
            break;
        case 16:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "substring-before", 16)) {
                xpath_func = xpath_substring_before;
            }
            break;
        }

        if (!xpath_func) {
            LOGVAL(LYE_SPEC, line, "Unknown XPath function \"%.*s\".", exp->tok_len[*cur_exp], &exp->expr[exp->expr_pos[*cur_exp]]);
            return -1;
        }
    }

    LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
    ++(*cur_exp);

    /* '(' */
    if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR1, line)) {
        return -1;
    }
    LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
    ++(*cur_exp);

    /* ( Expr ( ',' Expr )* )? */
    if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
        return -1;
    }
    if (exp->tokens[*cur_exp] != LYXP_TOKEN_PAR2) {
        if (set) {
            arg_count = 1;
            args = calloc(1, sizeof *args);
        }
        if ((rc = eval_expr(exp, cur_exp, cur_node, args, line))) {
            goto cleanup;
        }
    }
    while ((exp->used > *cur_exp) && (exp->tokens[*cur_exp] == LYXP_TOKEN_COMMA)) {
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        if (set) {
            ++arg_count;
            args = realloc(args, arg_count * sizeof *args);
            memset(&args[arg_count - 1], 0, sizeof *args);

            if ((rc = eval_expr(exp, cur_exp, cur_node, &args[arg_count - 1], line))) {
                goto cleanup;
            }
        } else {
            if ((rc = eval_expr(exp, cur_exp, cur_node, NULL, line))) {
                goto cleanup;
            }
        }
    }

    /* ')' */
    if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR2, line)) {
        rc = -1;
        goto cleanup;
    }
    LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
    ++(*cur_exp);

    if (set) {
        /* evaluate function */
        rc = xpath_func(args, arg_count, cur_node, set, line);
    }

cleanup:
    for (i = 0; i < arg_count; ++i) {
        set_free(&args[i], cur_node->schema->module->ctx);
    }
    free(args);

    return rc;
}

/**
 * @brief Evaluates Number. Logs directly on error.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_number(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyxp_set *set, uint32_t line)
{
    long double num;
    char *endptr;

    if (check_token(exp, *cur_exp, LYXP_TOKEN_NUMBER, line)) {
        return -1;
    }

    if (set) {
        errno = 0;
        num = strtold(&exp->expr[exp->expr_pos[*cur_exp]], &endptr);
        if (errno) {
            LOGVAL(LYE_SPEC, line, "Failed to convert \"%.*s\" into a long double (%s).", exp->tok_len[*cur_exp],
                &exp->expr[exp->expr_pos[*cur_exp]], strerror(errno));
            return -1;
        } else if (endptr - &exp->expr[exp->expr_pos[*cur_exp]] != exp->tok_len[*cur_exp]) {
            LOGVAL(LYE_SPEC, line, "Failed to convert \"%.*s\" into a long double.", exp->tok_len[*cur_exp],
                &exp->expr[exp->expr_pos[*cur_exp]]);
            return -1;
        }

        set_fill_number(set, num);
    }

    LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
    ++(*cur_exp);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates PathExpr. Logs directly on error.
 *
 * [9] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [1] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [7] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_path_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
               uint32_t line)
{
    int rc, all_desc;

    if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
        return -1;
    }

    switch (exp->tokens[*cur_exp]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' Predicate* */
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        if ((rc = eval_expr(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }

        if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR2, line)) {
            return -1;
        }
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);
        goto predicate;
        break;
    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        if ((rc = eval_relative_location_path(exp, cur_exp, cur_node, 0, set, line))) {
            return rc;
        }
        break;
    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if ((rc = eval_function_call(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }
        goto predicate;
        break;
    case LYXP_TOKEN_OPERATOR_PATH:
        /* AbsoluteLocationPath */
        if ((rc = eval_absolute_location_path(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }
        break;
    case LYXP_TOKEN_LITERAL:
        /* Literal */
        if ((rc = eval_literal(exp, cur_exp, set, cur_node->schema->module->ctx, line))) {
            return rc;
        }
        goto predicate;
        break;
    case LYXP_TOKEN_NUMBER:
        /* Number */
        if ((rc = eval_number(exp, cur_exp, set, line))) {
            return rc;
        }
        goto predicate;
        break;
    default:
        LOGVAL(LYE_XPATH_INTOK, line, print_token(exp->tokens[*cur_exp]), &exp->expr[exp->expr_pos[*cur_exp]]);
        return -1;
    }

    return EXIT_SUCCESS;

predicate:
    /* Predicate* */
    while ((exp->used > *cur_exp) && (exp->tokens[*cur_exp] == LYXP_TOKEN_BRACK1)) {
        if ((rc = eval_predicate(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }
    }

    /* ('/' or '//') RelativeLocationPath */
    if ((exp->used > *cur_exp) && (exp->tokens[*cur_exp] == LYXP_TOKEN_OPERATOR_PATH)) {

        /* evaluate '/' or '//' */
        if (exp->tok_len[*cur_exp] == 1) {
            all_desc = 0;
        } else {
            assert(exp->tok_len[*cur_exp] == 2);
            all_desc = 1;
        }

        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        if ((rc = eval_relative_location_path(exp, cur_exp, cur_node, all_desc, set, line))) {
            return rc;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates UnaryExpr. Logs directly on error.
 *
 * [16] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [17] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_unary_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                uint32_t line)
{
    int unary_minus = 0, rc;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    /* '-'* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && (exp->expr[exp->expr_pos[*cur_exp]] == '-')) {
        if (!unary_minus) {
            unary_minus = 1;
        } else {
            /* double '-' makes '+', ignore */
            unary_minus = 0;
        }
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);
    }

    /* PathExpr */
    prev_exp = *cur_exp;
    if ((rc = eval_path_expr(exp, cur_exp, cur_node, NULL, line))) {
        return rc;
    }
    if (set) {
        if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_UNI, UINT_MAX)) {
            /* there is a union, we need to store the context */
            orig_set = set_copy(set, cur_node->schema->module->ctx);
        }
        *cur_exp = prev_exp;
        if ((rc = eval_path_expr(exp, cur_exp, cur_node, set, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
    }

    /* ('|' PathExpr)* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_UNI, UINT_MAX)) {
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

        prev_exp = *cur_exp;
        if ((rc = eval_path_expr(exp, cur_exp, cur_node, NULL, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
        if (set) {
            if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_UNI, UINT_MAX)) {
                /* there is another union */
                set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            } else {
                /* there is no other union, we can modify the original context */
                set2 = orig_set;
            }
            *cur_exp = prev_exp;
            if ((rc = eval_path_expr(exp, cur_exp, cur_node, set2, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }

            if ((rc = moveto_union(set, set2, cur_node->schema->module->ctx))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }
        }
    }

    /* now we have all the unions in set and no other memory allocated */

    if (unary_minus) {
        if ((rc = moveto_unary_minus(set, cur_node->schema->module->ctx))) {
            return rc;
        }
    }

    return EXIT_SUCCESS;
}

/** TODO
 * @brief Evaluates AdditiveExpr. Logs directly on error.
 *
 * [14] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 * [15] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_additive_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                   uint32_t line)
{
    int rc;

    /*
     * MultiplicativeExpr
     */
    goto multiplicativeexpr;

    /*
     * ('+' / '-' MultiplicativeExpr)*
     */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*cur_exp]] == '+') || (exp->expr[exp->expr_pos[*cur_exp]] == '-'))) {
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

multiplicativeexpr:
        /* UnaryExpr */
        if ((rc = eval_unary_expr(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }

        /* ('*' / 'div' / 'mod' UnaryExpr)* */
        while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*cur_exp]] == '*') || (exp->tok_len[*cur_exp] == 3))) {
            LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
            ++(*cur_exp);

            if ((rc = eval_unary_expr(exp, cur_exp, cur_node, set, line))) {
                return rc;
            }

            /* TODO eval */
        }

        /* TODO eval */
    }

    return EXIT_SUCCESS;
}

/** TODO
 * @brief Evaluates EqualityExpr. Logs directly on error.
 *
 * [12] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 * [13] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_equality_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                   uint32_t line)
{
    int rc;

    /*
     * RelationalExpr
     */
    goto relationalexpr;

    /*
     * ('=' / '!=' RelationalExpr)*
     */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*cur_exp]] == '=') || (exp->expr[exp->expr_pos[*cur_exp]] == '!'))) {
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

relationalexpr:
        /* AdditiveExpr */
        if ((rc = eval_additive_expr(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }

        /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
        while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*cur_exp]] == '<') || (exp->expr[exp->expr_pos[*cur_exp]] == '>'))) {
            LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
            ++(*cur_exp);

            if ((rc = eval_additive_expr(exp, cur_exp, cur_node, set, line))) {
                return rc;
            }

            /* TODO eval */
        }

        /* TODO eval */
    }

    return EXIT_SUCCESS;
}

/** TODO
 * @brief Evaluates Expr. Logs directly on error.
 *
 * [10] Expr ::= AndExpr | Expr 'or' AndExpr
 * [11] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    int rc;

    /*
     * AndExpr
     */
    goto andexpr;

    /*
     * ('or' AndExpr)*
     */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 2)) {
        LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
        ++(*cur_exp);

andexpr:
        /* EqualityExpr */
        if ((rc = eval_equality_expr(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }

        /* ('and' EqualityExpr)* */
        while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 3)) {
            LOGDBG("XPATH: %s parsed %s", __func__, print_token(exp->tokens[*cur_exp]));
            ++(*cur_exp);

            if ((rc = eval_equality_expr(exp, cur_exp, cur_node, set, line))) {
                return rc;
            }

            /* TODO eval */
        }

        /* TODO eval */
    }

    return EXIT_SUCCESS;
}

int
lyxp_eval(const char *expr, struct lyd_node *cur_node, struct lyxp_set **set, uint32_t line)
{
    struct lyxp_expr *exp;
    struct lyd_node *root, *node;
    uint16_t cur_exp = 0;
    int rc = -1;

    assert(expr && cur_node && set);

    /* find root, beginning */
    for (node = cur_node; node->parent; node = node->parent);
    for (node = cur_node; node->prev->next; node = node->prev);

    exp = parse_expr(expr, line);
    if (exp) {
        debug_print_expr_struct(exp);

        /* TODO add fake root */
        root = calloc(1, sizeof *root);
        root->prev = root;
        root->child = node;
        /* TODO um? */
        root->schema = node->schema;
        LY_TREE_FOR(node, node) {
            node->parent = root;
        }

        *set = calloc(1, sizeof(struct lyxp_set_node));
        set_add_node(*set, root, 0);

        rc = eval_expr(exp, &cur_exp, cur_node, *set, line);

        /* TODO remove fake root */
        LY_TREE_FOR(root->child, node) {
            node->parent = NULL;
        }
        free(root);

        if (!rc && (exp->used > cur_exp)) {
            LOGVAL(LYE_SPEC, line, "Unparsed characters \"%s\" left at the end of an XPath expression.",
                &exp->expr[exp->expr_pos[cur_exp]]);
            set_free(*set, cur_node->schema->module->ctx);
            *set = NULL;
            rc = -1;
        }
        free(exp->tokens);
        free(exp->expr_pos);
        free(exp->tok_len);
        free(exp);
    }
    return rc;
}

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int
main(int argc, char **argv)
{
    struct ly_ctx *ctx;
    struct lyd_node *data = NULL, *next, *iter;
    struct lyxp_set_node *set;
    struct stat sb;
    int fd;
    char *addr;
    uint16_t i;

    ctx = ly_ctx_new(NULL);

    fd = open("./test.yin", O_RDONLY);
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    lys_parse(ctx, addr, LYS_IN_YIN);
    munmap(addr, sb.st_size);
    close(fd);

    fd = open("./data.xml", O_RDONLY);
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    data = lyd_parse(ctx, addr, LYD_XML, 0);
    munmap(addr, sb.st_size);
    close(fd);

    ly_verb(3);
    if (argc == 2) {
        if (!lyxp_eval(argv[1], data, (struct lyxp_set **)&set, 0)) {
            switch (set->type) {
            case LYXP_SET_NODE_SET:
                printf("RESULT NODE SET\n");
                for (i = 0; i < set->used; ++i) {
                    printf("RESULT %d: %s\n", i, set->value.nodes[i]->schema->name);
                }
                break;
            case LYXP_SET_ATTR_SET:
                printf("RESULT ATTR SET\n");
                for (i = 0; i < set->used; ++i) {
                    printf("RESULT %d: %s\n", i, set->value.attrs[i]->name);
                }
                break;
            default:
                printf("RESULT SOMETHING NOT SUPPORTED YET\n");
                break;
            }
            set_free((struct lyxp_set *)set, ctx);
        }
    }

    LY_TREE_FOR_SAFE(data, next, iter) {
        lyd_free(iter);
    }
    ly_ctx_destroy(ctx);

    return 0;
}
