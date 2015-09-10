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
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include "libyang.h"
#include "xml.h"
#include "tree.h"
#include "context.h"
#include "tree_internal.h"
#include "common.h"
#include "resolve.h"
#include "dict.h"

/* expression tokens allocation */
#define LYXP_EXPR_SIZE_START 10
#define LYXP_EXPR_SIZE_STEP 5

/* XPath matches allocation */
#define LYXP_SET_SIZE_START 2
#define LYXP_SET_SIZE_STEP 2

/* building string when casting */
#define LYXP_STRING_CAST_SIZE_START 64
#define LYXP_STRING_CAST_SIZE_STEP 16

/*
 * XPath evaluator fully compliant with http://www.w3.org/TR/1999/REC-xpath-19991116/
 * except the following restrictions in the grammar.
 *
 * PARSED GRAMMAR
 *
 * Full axes are not supported, abbreviated forms must be used,
 * variables are not supported, "id()" function is not supported,
 * and processing instruction and comment nodes are not supported,
 * which is also reflected in the grammar. Undefined rules and
 * constants are tokens.
 *
 * Modified full grammar:
 *
 * [1] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [2] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 * [3] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [4] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [5] NodeTest ::= NameTest | NodeType '(' ')'
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
    LYXP_TOKEN_LITERAL,       /* Literal - with either single or double quote */
    LYXP_TOKEN_NUMBER         /* Number */
};

struct lyxp_expr {
    enum lyxp_token *tokens; /* array of tokens */
    uint16_t *expr_pos;      /* array of pointers to the expression in expr (idx of the beginning) */
    uint8_t *tok_len;        /* array of token lengths in expr */
    uint16_t used;           /* used array items */
    uint16_t size;           /* allocated array items */

    const char *expr;        /* the original XPath expression */
};

enum lyxp_set_type {
    LYXP_SET_EMPTY = 0,
    LYXP_SET_NODE_SET,
    LYXP_SET_BOOLEAN,
    LYXP_SET_NUMBER,
    LYXP_SET_STRING
};

struct lyxp_set {
    enum lyxp_set_type type;
    union {
        struct lyd_node **nodes;
        struct lyd_attr **attrs;
        const char *str;
        long double num;
        int bool;
    } value;

    /* this is valid only for type == LYXP_NODE_SET */
    enum lyxp_node_type *node_type;  /* item with this index is of this node type */
    uint16_t used;
    uint16_t size;
    uint16_t pos;                    /* current context position, indexed from 1, relevant only for predicates */
};

enum lyxp_node_type {
    LYXP_NODE_ROOT,
    LYXP_NODE_ELEM,
    LYXP_NODE_TEXT,
    LYXP_NODE_ATTR
};

static const char *
print_set_type(struct lyxp_set *set)
{
    switch (set->type) {
    case LYXP_SET_EMPTY:
        return "empty";
    case LYXP_SET_NODE_SET:
        return "node set";
    case LYXP_SET_BOOLEAN:
        return "boolean";
    case LYXP_SET_NUMBER:
        return "number";
    case LYXP_SET_STRING:
        return "string";
    }

    return NULL;
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
string_cast_realloc(uint16_t needed, char **str, uint16_t *used, uint16_t *size)
{
    if (*size - *used < needed) {
        do {
            *size += LYXP_STRING_CAST_SIZE_STEP;
        } while (*size - *used < needed);
        *str = realloc(*str, *size * sizeof(char));
    }
}

static void
string_cast_recursive(struct lyd_node *node, uint16_t indent, char **str, uint16_t *used, uint16_t *size)
{
    FILE *stream;
    char *buf;
    const char *value_str;
    struct lyd_node *child;
    size_t buf_size;
    int is_leaf = 0;

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_INPUT:
    case LYS_OUTPUT:
        string_cast_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        LY_TREE_FOR(node->child, child) {
            string_cast_recursive(child, indent + 1, str, used, size);
        }

        string_cast_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);
        break;

    case LYS_LEAF:
        is_leaf = 1;
        /* fallthrough */
    case LYS_LEAFLIST:
        if (is_leaf) {
            value_str = ((struct lyd_node_leaf *)node)->value_str;
        } else {
            value_str = ((struct lyd_node_leaflist *)node)->value_str;
        }

        /* print indent */
        string_cast_realloc(indent * 2 + strlen(value_str) + 1, str, used, size);
        memset(*str + (*used - 1), ' ', indent * 2);
        *used += indent * 2;

        /* print value */
        if (*used == 1) {
            sprintf(*str + (*used - 1), "%s", value_str);
            *used += strlen(value_str);
        } else {
            sprintf(*str + (*used - 1), "%s\n", value_str);
            *used += strlen(value_str) + 1;
        }

        break;

    case LYS_ANYXML:
        stream = open_memstream(&buf, &buf_size);
        lyxml_dump(stream, ((struct lyd_node_anyxml *)node)->value->child, 0);
        fclose(stream);

        string_cast_realloc(buf_size, str, used, size);

        strcpy(*str + (*used - 1), buf);
        *used += buf_size;
        free(buf);
        break;

    default:
        LOGINT;
        break;
    }
}

/* returns a string in the dictionary */
static const char *
string_cast_elem(struct lyd_node *node, struct ly_ctx *ctx)
{
    char *str;
    uint16_t used, size;

    str = malloc(LYXP_STRING_CAST_SIZE_START * sizeof(char));
    str[0] = '\0';
    used = 1;
    size = LYXP_STRING_CAST_SIZE_START;

    string_cast_recursive(node, 0, &str, &used, &size);

    if (size > used) {
        str = realloc(str, used * sizeof(char));
    }
    return lydict_insert_zc(ctx, str);
}

/*
 * lyxp_set(_node) manipulation functions
 */
static void
set_free(struct lyxp_set *set, struct ly_ctx *ctx)
{
    if (!set) {
        return;
    }

    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }
    free(set);
}

static struct lyxp_set *
set_copy(struct lyxp_set *set, struct ly_ctx *ctx)
{
    struct lyxp_set *ret;

    ret = malloc(sizeof *ret);
    if (set->type == LYXP_SET_NODE_SET) {
        ret->type = set->type;
        ret->value.nodes = malloc(set->used * sizeof *ret->value.nodes);
        memcpy(ret->value.nodes, set->value.nodes, set->used * sizeof *ret->value.nodes);
        ret->node_type = malloc(set->used * sizeof *ret->node_type);
        memcpy(ret->node_type, set->node_type, set->used * sizeof *ret->node_type);
        ret->used = ret->size = set->used;
    } else {
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
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }

    set->type = LYXP_SET_STRING;
    set->value.str = lydict_insert(ctx, string, str_len);
}

static void
set_fill_number(struct lyxp_set *set, long double number, struct ly_ctx *ctx)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }

    set->type = LYXP_SET_NUMBER;
    set->value.num = number;
}

static void
set_fill_boolean(struct lyxp_set *set, int boolean, struct ly_ctx *ctx)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }

    set->type = LYXP_SET_BOOLEAN;
    set->value.bool = boolean;
}

static void
set_remove_node(struct lyxp_set *set, uint16_t idx)
{
    assert(set && (set->type == LYXP_SET_NODE_SET));
    assert(idx < set->used);

    --set->used;
    if (set->used) {
        memmove(&set->value.nodes[idx], &set->value.nodes[idx + 1],
                (set->used - idx) * sizeof *set->value.nodes);
        memmove(&set->node_type[idx], &set->node_type[idx + 1],
                (set->used - idx) * sizeof *set->node_type);
    } else {
        free(set->value.nodes);
        free(set->node_type);
        /* this changes it to LYXP_SET_EMPTY */
        memset(set, 0, sizeof *set);
    }
}

static int
set_dup_node_check(struct lyxp_set *set, void *node, enum lyxp_node_type node_type, int skip_idx)
{
    uint16_t i;

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->value.nodes[i] == node) && (set->node_type[i] == node_type)) {
            return i;
        }
    }

    return -1;
}

static int
set_sorted_dup_node_clean(struct lyxp_set *set)
{
    uint16_t i = 0;
    int ret = EXIT_SUCCESS;

    while (i < set->used - 1) {
        if ((set->value.nodes[i] == set->value.nodes[i + 1])
                && (set->node_type[i] == set->node_type[i + 1])) {
            set_remove_node(set, i + 1);
        ret = EXIT_FAILURE;
        } else {
            ++i;
        }
    }

    return ret;
}

static void
set_add_node(struct lyxp_set *set, void *node, enum lyxp_node_type node_type, uint16_t idx)
{
    assert(set && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)));

    if (set->type == LYXP_SET_EMPTY) {

        /* first item */
        if (idx) {
            /* no real harm done, but it is a bug */
            LOGINT;
        }
        set->type = LYXP_SET_NODE_SET;
        set->value.nodes = calloc(LYXP_SET_SIZE_START, sizeof *set->value.nodes);
        set->value.nodes[0] = node;
        set->node_type = malloc(LYXP_SET_SIZE_START * sizeof *set->node_type);
        set->node_type[0] = node_type;
        set->used = 1;
        set->size = LYXP_SET_SIZE_START;
    } else {

#ifndef NDEBUG
        if (set_dup_node_check(set, node, node_type, -1) > -1) {
            LOGINT;
            return;
        }
#endif

        /* not an empty set */
        if (set->used == set->size) {

            /* set is full */
            set->value.nodes = realloc(set->value.nodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->value.nodes);
            set->node_type = realloc(set->node_type, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->node_type);
            set->size += LYXP_SET_SIZE_STEP;
        }

        if (idx > set->used) {
            LOGINT;
            idx = set->used;
        }

        /* make space for the new node */
        if (idx < set->used) {
            memmove(&set->value.nodes[idx + 1], &set->value.nodes[idx], (set->used - idx) * sizeof *set->value.nodes);
            memmove(&set->node_type[idx + 1], &set->node_type[idx], (set->used - idx) * sizeof *set->node_type);
        }

        /* finally assign the value */
        set->value.nodes[idx] = node;
        set->node_type[idx] = node_type;
        ++set->used;
    }
}

static void
set_cast(struct lyxp_set *set, enum lyxp_set_type target, struct ly_ctx *ctx)
{
    char *ptr;
    long double num;
    struct lyd_node *node;
    const char *attr_val;

    if (set->type == target) {
        return;
    }

    /* it's not possible to convert anything into a node set */
    if (target == LYXP_SET_NODE_SET) {
        LOGINT;
        return;
    }

    /* to STRING */
    if ((target == LYXP_SET_STRING) || ((target == LYXP_SET_NUMBER)
            && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)))) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if (set->value.num == NAN) {
                set->value.str = lydict_insert(ctx, "NaN", 0);
            } else if ((set->value.num == 0) || (set->value.num == -0)) {
                set->value.str = lydict_insert(ctx, "0", 0);
            } else if (set->value.num == INFINITY) {
                set->value.str = lydict_insert(ctx, "Infinity", 0);
            } else if (set->value.num == -INFINITY) {
                set->value.str = lydict_insert(ctx, "-Infinity", 0);
            } else if ((long long)set->value.num == set->value.num) {
                asprintf(&ptr, "%lld", (long long)set->value.num);
                set->value.str = lydict_insert_zc(ctx, ptr);
            } else {
                asprintf(&ptr, "%03.1Lf", set->value.num);
                set->value.str = lydict_insert_zc(ctx, ptr);
            }
            break;
        case LYXP_SET_BOOLEAN:
            if (set->value.bool) {
                set->value.str = lydict_insert(ctx, "true", 0);
            } else {
                set->value.str = lydict_insert(ctx, "false", 0);
            }
            break;
        case LYXP_SET_NODE_SET:
            assert(set->used);
            switch (set->node_type[0]) {
            case LYXP_NODE_ROOT:
                node = set->value.nodes[0]->child;
                free(set->value.nodes);
                set->value.str = string_cast_elem(node, ctx);
                break;
            case LYXP_NODE_ELEM:
            case LYXP_NODE_TEXT:
                node = set->value.nodes[0];
                free(set->value.nodes);
                set->value.str = string_cast_elem(node, ctx);
                break;
            case LYXP_NODE_ATTR:
                attr_val = set->value.attrs[0]->value;
                free(set->value.nodes);
                set->value.str = lydict_insert(ctx, attr_val, 0);
                break;
            }

            free(set->node_type);
            break;
        case LYXP_SET_EMPTY:
            set->value.str = lydict_insert(ctx, "", 0);
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_STRING;
    }

    /* to NUMBER */
    if (target == LYXP_SET_NUMBER) {
        switch (set->type) {
        case LYXP_SET_STRING:
            errno = 0;
            num = strtold(set->value.str, &ptr);
            if (errno || *ptr) {
                num = NAN;
            }
            lydict_remove(ctx, set->value.str);
            set->value.num = num;
            break;
        case LYXP_SET_BOOLEAN:
            if (set->value.bool) {
                set->value.num = 1;
            } else {
                set->value.num = 0;
            }
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_NUMBER;
    }

    /* to BOOLEAN */
    if (target == LYXP_SET_BOOLEAN) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if ((set->value.num == 0) || (set->value.num == -0) || (set->value.num == NAN)) {
                set->value.bool = 0;
            } else {
                set->value.bool = 1;
            }
            break;
        case LYXP_SET_STRING:
            if (set->value.str[0]) {
                set->value.bool = 1;
            } else {
                set->value.bool = 0;
            }
            break;
        case LYXP_SET_NODE_SET:
            free(set->value.nodes);
            free(set->node_type);

            if (set->used) {
                set->value.bool = 1;
            } else {
                set->value.bool = 0;
            }
            break;
        case LYXP_SET_EMPTY:
            /* all we need is to change the type */
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_BOOLEAN;
    }

    /* to EMPTY */
    if (target == LYXP_SET_EMPTY) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
        case LYXP_SET_BOOLEAN:
            /* nothing to do */
            break;
        case LYXP_SET_STRING:
            lydict_remove(ctx, set->value.str);
            break;
        case LYXP_SET_NODE_SET:
            free(set->value.nodes);
            free(set->node_type);
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_EMPTY;
    }
}

static void
debug_print_set(struct lyxp_set *set, struct lyd_node *any_node)
{
    uint16_t i;
    struct lyxp_set *set2;

    switch (set->type) {
    case LYXP_SET_NODE_SET:
        LOGDBG("XPATH: set NODE SET:");
        for (i = 0; i < set->used; ++i) {
            switch (set->node_type[i]) {
            case LYXP_NODE_ROOT:
                LOGDBG("XPATH:\t%d: ROOT", i + 1);
                break;
            case LYXP_NODE_ELEM:
                if ((set->value.nodes[i]->schema->nodetype == LYS_LIST)
                        && (set->value.nodes[i]->child->schema->nodetype == LYS_LEAF)) {
                    LOGDBG("XPATH:\t%d: ELEM %s (1st child val: %s)", i + 1, set->value.nodes[i]->schema->name,
                           ((struct lyd_node_leaf *)set->value.nodes[i]->child)->value_str);
                } else if (set->value.nodes[i]->schema->nodetype == LYS_LEAFLIST) {
                    LOGDBG("XPATH:\t%d: ELEM %s (val: %s)", i + 1, set->value.nodes[i]->schema->name,
                           ((struct lyd_node_leaflist *)set->value.nodes[i])->value_str);
                } else {
                    LOGDBG("XPATH:\t%d: ELEM %s", i + 1, set->value.nodes[i]->schema->name);
                }
                break;
            case LYXP_NODE_TEXT:
                if (set->value.nodes[i]->schema->nodetype == LYS_ANYXML) {
                    LOGDBG("XPATH:\t%d: TEXT <anyxml>", i + 1);
                } else {
                    LOGDBG("XPATH:\t%d: TEXT %s", i + 1, ((struct lyd_node_leaf *)set->value.nodes[i])->value_str);
                }
                break;
            case LYXP_NODE_ATTR:
                LOGDBG("XPATH:\t%d: ATTR %s = %s", i + 1, set->value.attrs[i]->name, set->value.attrs[i]->value);
                break;
            }
        }
        break;

    case LYXP_SET_EMPTY:
        LOGDBG("XPATH: set EMPTY");
        break;

    case LYXP_SET_BOOLEAN:
        LOGDBG("XPATH: set BOOLEAN");
        LOGDBG("XPATH:\t%s", (set->value.bool ? "true" : "false"));
        break;

    case LYXP_SET_STRING:
        LOGDBG("XPATH: set STRING");
        LOGDBG("XPATH:\t%s", set->value.str);
        break;

    case LYXP_SET_NUMBER:
        LOGDBG("XPATH: set NUMBER");

        set2 = set_copy(set, any_node->schema->module->ctx);
        set_cast(set2, LYXP_SET_STRING, any_node->schema->module->ctx);
        LOGDBG("XPATH:\t%s", set2->value.str);
        set_free(set2, any_node->schema->module->ctx);
    }
}

static uint16_t
get_node_pos(struct lyd_node *node)
{
    struct lyd_node *root, *next, *elem;
    uint16_t pos = 1;

    /* TODO fake root */
    if (!node->schema) {
        return 0;
    }

    for (root = node; root->parent; root = root->parent);
    for (; root->prev->next; root = root->prev);

    /* TREE DFS */
    for (elem = next = root; elem; elem = next) {
        if (elem == node) {
            break;
        }
        ++pos;

        /* TREE DFS END */
        /* select element for the next run - children first */
        next = elem->child;
        /* child exception for lyd_node_leaf and lyd_node_leaflist, but not the root */
        if (elem->schema && (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
            next = NULL;
        }
        if (!next) {
            /* no children, so try siblings */
            next = elem->next;
        }
        while (!next) {
            /* no siblings, go back through parents */
            if (elem->parent == root->parent) {
                /* we are done, no next element to process */
                break;
            }
            /* parent is already processed, go to its sibling */
            elem = elem->parent;
            next = elem->next;
        }
    }

    if (!elem) {
        LOGINT;
    }

    return pos;
}

static uint16_t
get_attr_pos(struct lyd_attr *attr, struct lyd_node *parent)
{
    uint16_t pos = 0;
    struct lyd_attr *attr2;

    assert(attr->type == LYD_ATTR_STD);

    for (attr2 = parent->attr; attr2 && (attr2 != attr); attr2 = attr2->next) {
        ++pos;
    }

    assert(attr2);
    return pos;
}

/* TODO select sort -> bubble sort */
static int
set_sort(struct lyxp_set *set, struct lyd_node *any_node)
{
    uint16_t min, i, j, min_node_pos, node_pos, min_attr_pos, attr_pos;
    int ret = EXIT_SUCCESS;
    struct lyd_node *tmp_node;
    enum lyxp_node_type tmp_type;

    if (set->type != LYXP_SET_NODE_SET) {
        return ret;
    }

    LOGDBG("XPATH: SORT BEGIN");
    debug_print_set(set, any_node);

    /* make cur_node into root */
    for (; any_node->parent; any_node = any_node->parent);
    assert(any_node->prev == any_node);

    for (i = 0; i < set->used - 1; ++i) {

        /* set the first node as the min at the beginning */
        if (set->node_type[i] == LYXP_NODE_ATTR) {
            tmp_node = lyd_attr_parent(any_node, set->value.attrs[i]);
            if (!tmp_node) {
                LOGINT;
                return -1;
            }
            min_node_pos = get_node_pos(tmp_node);
            min_attr_pos = get_attr_pos(set->value.attrs[i], tmp_node);
        } else {
            min_node_pos = get_node_pos(set->value.nodes[i]);
        }
        min = i;

        for (j = i + 1; j < set->used; ++j) {
            /* get position */
            if (set->node_type[j] == LYXP_NODE_ATTR) {
                tmp_node = lyd_attr_parent(any_node, set->value.attrs[j]);
                if (!tmp_node) {
                    LOGINT;
                    return -1;
                }
                node_pos = get_node_pos(tmp_node);
                attr_pos = get_attr_pos(set->value.attrs[j], tmp_node);
            } else {
                node_pos = get_node_pos(set->value.nodes[j]);
            }

            /*
             * compare with the current min
             */
            if (min_node_pos < node_pos) {
                continue;
            }

            if (min_node_pos > node_pos) {
                min_node_pos = node_pos;
                if (set->node_type[j] == LYXP_NODE_ATTR) {
                    min_attr_pos = attr_pos;
                }
                min = j;
                continue;
            }

            /* node positions are equal, the fun case */

            /* min ELEM - == - cur TEXT, min TEXT - == - cur ELEM */
            /* special case since text nodes are actually saved as their parents */
            if ((set->value.nodes[min] == set->value.nodes[j])
                    && (set->node_type[min] != set->node_type[j])) {
                if (set->node_type[min] == LYXP_NODE_ELEM) {
                    assert(set->node_type[j] == LYXP_NODE_TEXT);
                    continue;
                } else {
                    assert((set->node_type[min] == LYXP_NODE_TEXT) && (set->node_type[j] == LYXP_NODE_ELEM));
                    min_node_pos = node_pos;
                    if (set->node_type[j] == LYXP_NODE_ATTR) {
                        min_attr_pos = attr_pos;
                    }
                    min = j;
                    continue;
                }
            }

            /* min ROOT - cur ROOT, min ELEM - cur ELEM, min TEXT - cur TEXT, min ATTR - =pos= - cur ATTR */
            /* check for duplicates */
            if (set->value.nodes[min] == set->value.nodes[j]) {
                assert((set->node_type[min] == set->node_type[j])
                       && ((set->node_type[min] != LYXP_NODE_ATTR) || (min_attr_pos == attr_pos)));
                /* removing it now would screw up loops, duplicate removal should be performed after sotring */
                continue;
            }

            /* min ELEM - cur TEXT, min ELEM - any pos - cur ATTR */
            /* elem is always first, cur node is after it */
            if (set->node_type[min] == LYXP_NODE_ELEM) {
                assert(set->node_type[j] != LYXP_NODE_ELEM);
                continue;
            }

            /* min TEXT - cur ELEM, min TEXT - any pos - cur ATTR, min ATTR - any pos - cur ELEM, min ATTR - >pos> - cur ATTR */
            /* cur is before min */
            if (((set->node_type[min] == LYXP_NODE_TEXT)
                    && ((set->node_type[j] == LYXP_NODE_ELEM) || (set->node_type[j] == LYXP_NODE_ATTR)))
                    || ((set->node_type[min] == LYXP_NODE_ATTR) && (set->node_type[j] == LYXP_NODE_ELEM))
                    || (((set->node_type[min] == set->node_type[j]) == LYXP_NODE_ATTR)
                    && (min_attr_pos > attr_pos))) {
                min_node_pos = node_pos;
                if (set->node_type[j] == LYXP_NODE_ATTR) {
                    min_attr_pos = attr_pos;
                }
                min = j;
            }

            /* min ATTR - any pos - cur TEXT, min ATTR <pos< - cur ATTR */
            /* cur is after min */
        }

        /* on i-th position is not the min, swap */
        if (min > i) {
            ret = EXIT_FAILURE;

            tmp_node = set->value.nodes[i];
            tmp_type = set->node_type[i];

            set->value.nodes[i] = set->value.nodes[min];
            set->node_type[i] = set->node_type[min];

            set->value.nodes[min] = tmp_node;
            set->node_type[min] = tmp_type;
        }
    }

    LOGDBG("XPATH: SORT END %d", ret);
    debug_print_set(set, any_node);

    return ret;
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
 * @brief Parses an XPath expression into a structure of tokens.
 *        Logs directly.
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
                        && !strncmp(&expr[ret->expr_pos[ret->used - 1]], "comment", 7))) {
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

    set_fill_boolean(set, 1, cur_node->schema->module->ctx);
    return EXIT_SUCCESS;
}

/*
 * moveto functions
 */

/* is_name: 1 - is module name, 0 - is module namespace */
static struct lys_module *
moveto_resolve_model(const char *mod_name_ns, uint16_t mod_nam_ns_len, struct lyd_node *any_node, int is_name)
{
    uint16_t i;
    const char *str;

    for (i = 0; i < any_node->schema->module->ctx->models.used; ++i) {
        str = (is_name ? any_node->schema->module->ctx->models.list[i]->name
               : any_node->schema->module->ctx->models.list[i]->ns);
        if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
            return any_node->schema->module->ctx->models.list[i];
        }
    }

    LOGINT;
    return NULL;
}

/* absolute path */
static int
moveto_root(struct lyxp_set *set, struct lyd_node *any_node, uint32_t line)
{
    uint16_t i;

    if (!set) {
        return EXIT_SUCCESS;
    }

    if ((set->type != LYXP_SET_NODE_SET) && (set->type != LYXP_SET_EMPTY)) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    /* move the node to the root */
    for (; any_node->parent; any_node = any_node->parent);
    assert(any_node->prev = any_node);

    if (set->type == LYXP_SET_NODE_SET) {
        set->value.nodes[0] = any_node;
        set->node_type[0] = LYXP_NODE_ROOT;

        /* delete the rest */
        for (i = set->used - 1; i > 0 ; --i) {
            set_remove_node(set, i);
        }
    } else {
        set_add_node(set, any_node, LYXP_NODE_ROOT, 0);
    }

    return EXIT_SUCCESS;
}

/* '/' and '*' or 'NAME' or 'PREFIX:*' or 'PREFIX:NAME' */
static int
moveto_node(const char *qname, uint16_t qname_len, struct lyd_node *any_node, struct lyxp_set *set, uint32_t line)
{
    uint16_t i;
    int replaced, all = 0, pref_len;
    struct lys_module *moveto_mod, *cur_mod;
    struct lyd_node *sub;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, any_node, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        pref_len = 0;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* skip nodes without children, leaves, leaflists, and anyxmls */
        if ((set->node_type[i] == LYXP_NODE_ROOT) || ((set->node_type[i] == LYXP_NODE_ELEM)
                && !(set->value.nodes[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)))) {

            LY_TREE_FOR(set->value.nodes[i]->child, sub) {
                /* module check */
                if (pref_len) {
                    cur_mod = sub->schema->module;
                    if (cur_mod->type) {
                        cur_mod = ((struct lys_submodule *)cur_mod)->belongsto;
                    }
                    if (cur_mod != moveto_mod) {
                        continue;
                    }
                }

                /* name check */
                if (all || (!strncmp(sub->schema->name, qname, qname_len) && !sub->schema->name[qname_len])) {
                    if (!replaced) {
                        set->value.nodes[i] = sub;
                        set->node_type[i] = LYXP_NODE_ELEM;
                        replaced = 1;
                    } else {
                        set_add_node(set, sub, LYXP_NODE_ELEM, set->used);
                        ++i;
                    }
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

    set_sort(set, any_node);
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/* '//' and '*' or 'NAME' or 'PREFIX:*' or 'PREFIX:NAME' */
static int
moveto_node_alldesc(const char *qname, uint16_t qname_len, struct lyd_node *any_node, struct lyxp_set *set,
                    uint32_t line)
{
    uint16_t i;
    int pref_len, all = 0, replace, match;
    struct lyd_node *next, *elem, *start;
    struct lys_module *moveto_mod, *cur_mod;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, any_node, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        pref_len = 0;
    }

    /* replace the original nodes (and throws away all text and attr nodes) */
    if (moveto_node("*", 1, any_node, set, line)) {
        return -1;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    /* this loop traverses all the nodes in the set and addds/keeps only
     * those that match qname */
    for (i = 0; i < set->used; ) {
        /* TREE DFS */
        start = set->value.nodes[i];
        replace = 0;
        for (elem = next = start; elem; elem = next) {
            match = 1;

            /* module check */
            if (pref_len) {
                cur_mod = elem->schema->module;
                if (cur_mod->type) {
                    cur_mod = ((struct lys_submodule *)cur_mod)->belongsto;
                }
                if (cur_mod != moveto_mod) {
                    /* no match */
                    match = 0;
                }
            }

            /* name check */
            if (!all && (strncmp(elem->schema->name, qname, qname_len) || elem->schema->name[qname_len])) {
                /* no match */
                match = 0;
            }

            if (match && (elem != start)) {
                if (set_dup_node_check(set, elem, LYXP_NODE_ELEM, i) > -1) {
                    /* we'll process it later */
                    goto skip_children;
                } else if (replace) {
                    set->value.nodes[i] = elem;
                    assert(set->node_type[i] == LYXP_NODE_ELEM);
                    replace = 0;
                } else {
                    set_add_node(set, elem, LYXP_NODE_ELEM, i + 1);
                    ++i;
                }
            } else if (!match && (elem == start)) {
                /* we need to replace a node that is already in the set */
                replace = 1;
            }

            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            next = elem->child;
            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
                next = NULL;
            }
            if (!next) {
skip_children:
                /* no children, so try siblings, but only if it's not the start,
                 * that is considered to be the root and it's siblings are not traversed */
                if (elem != start) {
                    next = elem->next;
                } else {
                    break;
                }
            }
            while (!next) {
                /* no siblings, go back through the parents */
                if (elem->parent == start) {
                    /* we are done, no next element to process */
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = elem->parent;
                next = elem->next;
            }
        }

        if (replace) {
            set_remove_node(set, i);
        } else {
            ++i;
        }
    }

    set_sort(set, any_node);
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/* '/' and '@*' or '@NAME' or '@PREFIX:*' or '@PREFIX:NAME' */
static int
moveto_attr(const char *qname, uint16_t qname_len, struct lyd_node *any_node, struct lyxp_set *set, uint32_t line)
{
    uint16_t i;
    int replaced, all = 0, pref_len;
    struct lys_module *moveto_mod, *cur_mod;
    struct lyd_attr *sub;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, any_node, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        pref_len = 0;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* skip root, since it's artificial it cannot have any attributes */
        assert((set->node_type[i] != LYXP_NODE_ROOT) || !set->value.nodes[i]->attr);
        /* only some attributes of an elem can be in the result, skip all the rest */
        if (set->node_type[i] == LYXP_NODE_ELEM) {
            LY_TREE_FOR(set->value.nodes[i]->attr, sub) {
                /* TODO? since axes are not supported, there is no way how to select namespaces */
                if (sub->type == LYD_ATTR_NS) {
                    continue;
                }

                /* module check */
                if ((pref_len && !sub->ns) || (!pref_len && sub->ns)) {
                    continue;
                }
                if (pref_len) {
                    cur_mod = moveto_resolve_model(sub->ns->value, strlen(sub->ns->value), any_node, 0);
                    if (cur_mod != moveto_mod) {
                        /* no match */
                        continue;
                    }
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->value.attrs[i] = sub;
                        set->node_type[i] = LYXP_NODE_ATTR;
                        replaced = 1;
                    } else {
                        set_add_node(set, (struct lyd_node *)sub, LYXP_NODE_ATTR, i + 1);
                        ++i;
                    }
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

    /* no need to sort */
    assert(!set_sort(set, any_node));
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/* '|', result is in set1, set2 is freed */
static int
moveto_union(struct lyxp_set *set1, struct lyxp_set *set2, struct lyd_node *any_node, uint32_t line)
{
    if (((set1->type != LYXP_SET_NODE_SET) && (set1->type != LYXP_SET_EMPTY))
            || ((set2->type != LYXP_SET_NODE_SET) && (set2->type != LYXP_SET_EMPTY))) {
        LOGVAL(LYE_XPATH_INOP_2, line, "union", print_set_type(set1), print_set_type(set2));
        return -1;
    }

    /* set2 is empty or both set1 and set2 */
    if (set2->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }

    if (set1->type == LYXP_SET_EMPTY) {
        memcpy(set1, set2, sizeof *set1);
        free(set2);
        return EXIT_SUCCESS;
    }

    /* make sure there is enough memory */
    if (set1->size - set1->used < set2->used) {
        set1->size = set1->used + set2->used;
        set1->value.nodes = realloc(set1->value.nodes, set1->size * sizeof *set1->value.nodes);
        set1->node_type = realloc(set1->node_type, set1->size * sizeof *set1->node_type);
    }

    /* copy nodes and free the old set */
    memcpy(&set1->value.nodes[set1->used], set2->value.nodes, set2->used * sizeof *set2->value.nodes);
    memcpy(&set1->node_type[set1->used], set2->node_type, set2->used * sizeof *set2->node_type);
    set1->used += set2->used;
    set_free(set2, any_node->schema->module->ctx);

    /* sort, remove duplicates */
    set_sort(set1, any_node);
    set_sorted_dup_node_clean(set1);

    return EXIT_SUCCESS;
}

/* '//' and '@*' or '@NAME' or '@PREFIX:*' or '@PREFIX:NAME' */
static int
moveto_attr_alldesc(const char *qname, uint16_t qname_len, struct lyd_node *any_node, struct lyxp_set *set,
                    uint32_t line)
{
    uint16_t i;
    int pref_len, replaced, all = 0;
    struct lyd_attr *sub;
    struct lys_module *moveto_mod, *cur_mod;
    struct lyxp_set *set_all_desc = NULL;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, any_node, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        pref_len = 0;
    }

    /* TODO? can be optimized similarly to moveto_node_alldesc() and save considerable amount of memory */
    /* copy the context */
    set_all_desc = set_copy(set, any_node->schema->module->ctx);
    /* get all descendant nodes (the original context nodes are removed) */
    if (moveto_node_alldesc("*", 1, any_node, set_all_desc, line)) {
        return -1;
    }
    /* prepend the original context nodes */
    if (moveto_union(set, set_all_desc, any_node, line)) {
        return -1;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* skip root, since it's artificial it cannot have any attributes */
        assert((set->node_type[i] != LYXP_NODE_ROOT) || !set->value.nodes[i]->attr);
        /* only some attributes of an elem can be in the result, skip all the rest */
        if (set->node_type[i] == LYXP_NODE_ELEM) {
            LY_TREE_FOR(set->value.nodes[i]->attr, sub) {
                /* TODO? since axes are not supported, there is no way how to select namespaces */
                if (sub->type == LYD_ATTR_NS) {
                    continue;
                }

                /* module check */
                if ((pref_len && !sub->ns) || (!pref_len && sub->ns)) {
                    continue;
                }
                if (pref_len) {
                    cur_mod = moveto_resolve_model(sub->ns->value, strlen(sub->ns->value), any_node, 0);
                    if (cur_mod != moveto_mod) {
                        /* no match */
                        continue;
                    }
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->value.attrs[i] = sub;
                        set->node_type[i] = LYXP_NODE_ATTR;
                        replaced = 1;
                    } else {
                        set_add_node(set, (struct lyd_node *)sub, LYXP_NODE_ATTR, i + 1);
                        ++i;
                    }
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

    /* no need to sort */
    assert(!set_sort(set, any_node));
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/* '/' or '//' and '.' */
static int
moveto_self(struct lyxp_set *set, int all_desc, struct lyd_node *any_node, uint32_t line)
{
    struct lyd_node *sub;
    uint16_t i, cont_i;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    /* nothing to do */
    if (!all_desc) {
        return EXIT_SUCCESS;
    }

    /* add all the children, they get added recursively */
    for (i = 0; i < set->used; ++i) {
        cont_i = 0;

        /* do not touch attributes and text nodes */
        if ((set->node_type[i] == LYXP_NODE_ROOT) || (set->node_type[i] == LYXP_NODE_ELEM)) {
            /* add all the children ... */
            if (!(set->value.nodes[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
                LY_TREE_FOR(set->value.nodes[i]->child, sub) {
                    if (set_dup_node_check(set, sub, LYXP_NODE_ELEM, -1) == -1) {
                        set_add_node(set, sub, LYXP_NODE_ELEM, i + cont_i + 1);
                        ++cont_i;
                    }
                }

            /* ... or add their text node, ... */
            } else {
                /* ... but only non-empty */
                sub = set->value.nodes[i];
                if (((sub->schema->nodetype == LYS_ANYXML) && (((struct lyd_node_anyxml *)sub)->value->child))
                        || ((sub->schema->nodetype != LYS_ANYXML) && (((struct lyd_node_leaf *)sub)->value_str))) {
                    if (set_dup_node_check(set, sub, LYXP_NODE_TEXT, -1) == -1) {
                        set_add_node(set, sub, LYXP_NODE_TEXT, i + 1);
                    }
                }
            }
        }
    }

    set_sort(set, any_node);
    assert(!set_sorted_dup_node_clean(set));
    return EXIT_SUCCESS;
}

/* '/' or '//' and '..' */
static int
moveto_parent(struct lyxp_set *set, int all_desc, struct lyd_node *any_node, uint32_t line)
{
    uint16_t i;
    struct lyd_node *new_node;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        if (moveto_self(set, 1, any_node, line)) {
            return -1;
        }
    }

    for (i = 0; i < set->used; ) {
        if (set->node_type[i] == LYXP_NODE_ELEM) {
            new_node = set->value.nodes[i]->parent;
        } else if (set->node_type[i] == LYXP_NODE_TEXT) {
            new_node = set->value.nodes[i];
        } else if (set->node_type[i] == LYXP_NODE_ATTR) {
            while (any_node->parent) {
                any_node = any_node->parent;
            }
            new_node = lyd_attr_parent(any_node, set->value.attrs[i]);
            if (!new_node) {
                LOGINT;
                return -1;
            }
        } else {
            /* root does not have a parent */
            set_remove_node(set, i);
            continue;
        }

        /* check for duplicate */
        if ((set_dup_node_check(set, new_node, LYXP_NODE_ELEM, -1) > -1)
                || (!new_node->parent && set_dup_node_check(set, new_node, LYXP_NODE_ROOT, -1))) {
            set_remove_node(set, i);
            continue;
        }

        /* update it */
        set->value.nodes[i] = new_node;
        set->node_type[i] = (new_node->parent ? LYXP_NODE_ELEM : LYXP_NODE_ROOT);

        ++i;
    }

    set_sort(set, any_node);
    assert(!set_sorted_dup_node_clean(set));
    return EXIT_SUCCESS;
}

/* '=', '!=', '<=', '<', '>=', '>' */
static void
moveto_op_comp(const char *op, struct lyxp_set *set1, struct lyxp_set *set2, struct lyd_node *any_node)
{
    /*
     * NODE SET + NODE SET = STRING + STRING  /1 STRING, 2 STRING
     * NODE SET + STRING = STRING + STRING    /1 STRING (2 STRING)
     * NODE SET + NUMBER = NUMBER + NUMBER    /1 NUMBER (2 NUMBER)
     * NODE SET + BOOLEAN = BOOLEAN + BOOLEAN /1 BOOLEAN (2 BOOLEAN)
     * STRING + NODE SET = STRING + STRING    /(1 STRING) 2 STRING
     * NUMBER + NODE SET = NUMBER + NUMBER    /(1 NUMBER) 2 NUMBER
     * BOOLEAN + NODE SET = BOOLEAN + BOOLEAN /(1 BOOLEAN) 2 BOOLEAN
     *
     * '=' or '!='
     * BOOLEAN + BOOLEAN
     * BOOLEAN + STRING = BOOLEAN + BOOLEAN   /(1 BOOLEAN) 2 BOOLEAN
     * BOOLEAN + NUMBER = BOOLEAN + BOOLEAN   /(1 BOOLEAN) 2 BOOLEAN
     * STRING + BOOLEAN = BOOLEAN + BOOLEAN   /1 BOOLEAN (2 BOOLEAN)
     * NUMBER + BOOLEAN = BOOLEAN + BOOLEAN   /1 BOOLEAN (2 BOOLEAN)
     * NUMBER + NUMBER
     * NUMBER + STRING = NUMBER + NUMBER      /(1 NUMBER) 2 NUMBER
     * STRING + NUMBER = NUMBER + NUMBER      /1 NUMBER (2 NUMBER)
     * STRING + STRING
     *
     * '<=', '<', '>=', '>'
     * NUMBER + NUMBER
     * BOOLEAN + BOOLEAN = NUMBER + NUMBER    /1 NUMBER, 2 NUMBER
     * BOOLEAN + NUMBER = NUMBER + NUMBER     /1 NUMBER (2 NUMBER)
     * BOOLEAN + STRING = NUMBER + NUMBER     /1 NUMBER, 2 NUMBER
     * NUMBER + STRING = NUMBER + NUMBER      /(1 NUMBER) 2 NUMBER
     * STRING + STRING = NUMBER + NUMBER      /1 NUMBER, 2 NUMBER
     * STRING + NUMBER = NUMBER + NUMBER      /1 NUMBER (2 NUMBER)
     * NUMBER + BOOLEAN = NUMBER + NUMBER     /(1 NUMBER) 2 NUMBER
     * STRING + BOOLEAN = NUMBER + NUMBER     /(1 NUMBER) 2 NUMBER
     */
    int result;

    /* we can evaluate it immediately */
    if ((set1->type == set2->type) && (set1->type != LYXP_SET_EMPTY) && (set1->type != LYXP_SET_NODE_SET)
            && (((op[0] == '=') || (op[0] == '!')) || ((set1->type != LYXP_SET_BOOLEAN) && (set1->type != LYXP_SET_STRING)))) {

        /* compute result */
        if (op[0] == '=') {
            if (set1->type == LYXP_SET_BOOLEAN) {
                result = (set1->value.bool == set2->value.bool);
            } else if (set1->type == LYXP_SET_NUMBER) {
                result = (set1->value.num == set2->value.num);
            } else {
                result = (set1->value.str == set2->value.str);
            }
        } else if (op[0] == '!') {
            if (set1->type == LYXP_SET_BOOLEAN) {
                result = (set1->value.bool != set2->value.bool);
            } else if (set1->type == LYXP_SET_NUMBER) {
                result = (set1->value.num != set2->value.num);
            } else {
                result = (set1->value.str != set2->value.str);
            }
        } else {
            if (set1->type != LYXP_SET_NUMBER) {
                LOGINT;
                return;
            }

            if (op[0] == '<') {
                if (op[1] == '=') {
                    result = (set1->value.num <= set2->value.num);
                } else {
                    result = (set1->value.num < set2->value.num);
                }
            } else {
                if (op[1] == '=') {
                    result = (set1->value.num >= set2->value.num);
                } else {
                    result = (set1->value.num > set2->value.num);
                }
            }
        }

        /* assign result */
        if (result) {
            set_fill_boolean(set1, 1, any_node->schema->module->ctx);
        } else {
            set_fill_boolean(set1, 0, any_node->schema->module->ctx);
        }

        set_free(set2, any_node->schema->module->ctx);
        return;
    }

    /* convert first */
    if (((set1->type == LYXP_SET_NODE_SET) || (set1->type == LYXP_SET_EMPTY) || (set1->type == LYXP_SET_STRING))
            && ((set2->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_EMPTY) || (set2->type == LYXP_SET_STRING))) {
        set_cast(set1, LYXP_SET_STRING, any_node->schema->module->ctx);
        set_cast(set2, LYXP_SET_STRING, any_node->schema->module->ctx);

    } else if ((((set1->type == LYXP_SET_NODE_SET) || (set1->type == LYXP_SET_EMPTY) || (set1->type == LYXP_SET_BOOLEAN))
            && ((set2->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_EMPTY) || (set2->type == LYXP_SET_BOOLEAN)))
            || (((op[0] == '=') || (op[0] == '!')) && ((set1->type == LYXP_SET_BOOLEAN) || (set2->type == LYXP_SET_BOOLEAN)))) {
        set_cast(set1, LYXP_SET_BOOLEAN, any_node->schema->module->ctx);
        set_cast(set2, LYXP_SET_BOOLEAN, any_node->schema->module->ctx);

    } else {
        set_cast(set1, LYXP_SET_NUMBER, any_node->schema->module->ctx);
        set_cast(set2, LYXP_SET_NUMBER, any_node->schema->module->ctx);
    }

    /* now we can evaluate */
    moveto_op_comp(op, set1, set2, any_node);
}

/* '+', '-', unary '-', '*', 'div', 'mod' */
static void
moveto_op_math(const char *op, struct lyxp_set *set1, struct lyxp_set *set2, struct lyd_node *any_node)
{
    /* unary '-' */
    if (!set2 && (op[0] == '-')) {
        set_cast(set1, LYXP_SET_NUMBER, any_node->schema->module->ctx);
        set1->value.num *= -1;
        set_free(set2, any_node->schema->module->ctx);
        return;
    }

    assert(set1 && set2);

    set_cast(set1, LYXP_SET_NUMBER, any_node->schema->module->ctx);
    set_cast(set2, LYXP_SET_NUMBER, any_node->schema->module->ctx);

    switch (op[0]) {
    /* '+' */
    case '+':
        set1->value.num += set2->value.num;
        break;

    /* '-' */
    case '-':
        set1->value.num -= set2->value.num;
        break;

    /* '*' */
    case '*':
        set1->value.num *= set2->value.num;
        break;

    /* 'div' */
    case 'd':
        set1->value.num /= set2->value.num;
        break;

    /* 'mod' */
    case 'm':
        set1->value.num = ((long long)set1->value.num) % ((long long)set2->value.num);
        break;

    default:
        LOGINT;
        break;
    }

    set_free(set2, any_node->schema->module->ctx);
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
        set_fill_string(set, &exp->expr[exp->expr_pos[*cur_exp] + 1], exp->tok_len[*cur_exp] - 2, ctx);
    }
    LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
    ++(*cur_exp);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates NodeTest. Logs directly on error.
 *
 * [5] NodeTest ::= NameTest | NodeType '(' ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] cur_exp Pointer to the current token in \p exp.
 * @param[in] attr_axis Whether to search attributes or standard nodes.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set at the same time. On NULL the rule is only parsed.
 * @param[in] ctx libyang context with the dictionary.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_node_test(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, int attr_axis, int all_desc,
               struct lyxp_set *set, uint32_t line)
{
    int nodetype_exp, rc;
    struct lyxp_set *set_literal = NULL;

    if (check_token(exp, *cur_exp, LYXP_TOKEN_NONE, line)) {
        return -1;
    }

    switch (exp->tokens[*cur_exp]) {
    case LYXP_TOKEN_NAMETEST:
        if (attr_axis) {
            if (all_desc) {
                rc = moveto_attr_alldesc(&exp->expr[exp->expr_pos[*cur_exp]], exp->tok_len[*cur_exp], cur_node, set, line);
            } else {
                rc = moveto_attr(&exp->expr[exp->expr_pos[*cur_exp]], exp->tok_len[*cur_exp], cur_node, set, line);
            }
        } else {
            if (all_desc) {
                rc = moveto_node_alldesc(&exp->expr[exp->expr_pos[*cur_exp]], exp->tok_len[*cur_exp], cur_node, set, line);
            } else {
                rc = moveto_node(&exp->expr[exp->expr_pos[*cur_exp]], exp->tok_len[*cur_exp], cur_node, set, line);
            }
        }
        if (rc) {
            return rc;
        }

        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);
        break;
    case LYXP_TOKEN_NODETYPE:
        nodetype_exp = (signed)*cur_exp;

        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        /* '(' */
        if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR1, line)) {
            return -1;
        }
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        /* ')' */
        if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR2, line)) {
            set_free(set_literal, cur_node->schema->module->ctx);
            return -1;
        }
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        if (set) {
            assert(exp->tok_len[nodetype_exp] == 4);
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
        }

        break;
    default:
        LOGVAL(LYE_XPATH_INTOK, line, print_token(exp->tokens[*cur_exp]), &exp->expr[exp->expr_pos[*cur_exp]]);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
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
    uint16_t i, orig_i, orig_exp;
    int rc;
    struct lyxp_set *set2, *orig_set;

    if (check_token(exp, *cur_exp, LYXP_TOKEN_BRACK1, line)) {
        return -1;
    }
    LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
    ++(*cur_exp);

    if (!set) {
        eval_expr(exp, cur_exp, cur_node, NULL, line);
    } else if (set->type == LYXP_SET_NODE_SET) {
        orig_set = set_copy(set, cur_node->schema->module->ctx);
        orig_exp = *cur_exp;

        i = 0;
        for (orig_i = 0; orig_i < orig_set->used; ++orig_i) {
            set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            set2->pos = orig_i + 1;
            *cur_exp = orig_exp;

            if ((rc = eval_expr(exp, cur_exp, cur_node, set2, line))) {
                return rc;
            }

            /* number is a position */
            if (set2->type == LYXP_SET_NUMBER) {
                if ((long long)set2->value.num == orig_i + 1) {
                    set2->value.num = 1;
                } else {
                    set2->value.num = 0;
                }
            }
            set_cast(set2, LYXP_SET_BOOLEAN, cur_node->schema->module->ctx);

            /* predicate satisfied or not? */
            if (set2->value.bool) {
                ++i;
            } else {
                set_remove_node(set, i);
            }
            set_free(set2, cur_node->schema->module->ctx);
        }

        set_free(orig_set, cur_node->schema->module->ctx);
    } else {
        set2 = set_copy(set, cur_node->schema->module->ctx);

        if ((rc = eval_expr(exp, cur_exp, cur_node, set2, line))) {
            return rc;
        }

        set_cast(set2, LYXP_SET_BOOLEAN, cur_node->schema->module->ctx);
        if (!set2->value.bool) {
            set_cast(set, LYXP_SET_EMPTY, cur_node->schema->module->ctx);
        }
        set_free(set2, cur_node->schema->module->ctx);
    }

    if (check_token(exp, *cur_exp, LYXP_TOKEN_BRACK2, line)) {
        return -1;
    }
    LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
            if ((rc = moveto_self(set, all_desc, cur_node, line))) {
                return rc;
            }
            LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
            ++(*cur_exp);
            break;
        case LYXP_TOKEN_DDOT:
            /* evaluate '..' */
            if ((rc = moveto_parent(set, all_desc, cur_node, line))) {
                return rc;
            }
            LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
            ++(*cur_exp);
            break;

        case LYXP_TOKEN_AT:
            /* evaluate '@' */
            attr_axis = 1;
            LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
            if ((rc = eval_node_test(exp, cur_exp, cur_node, attr_axis, all_desc, set, line))) {
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

    if (check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_PATH, line)) {
        return -1;
    }

    if (set) {
        /* no matter what tokens follow, we need to be at the root */
        if ((rc = moveto_root(set, cur_node, line))) {
            return rc;
        }
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*cur_exp] == 1) {
        /* evaluate '/' - deferred */
        all_desc = 0;
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
                xpath_func = &xpath_not;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "sum", 3)) {
                xpath_func = &xpath_sum;
            }
            break;
        case 4:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "lang", 4)) {
                xpath_func = &xpath_lang;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "last", 4)) {
                xpath_func = &xpath_last;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "name", 4)) {
                xpath_func = &xpath_name;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "true", 4)) {
                xpath_func = &xpath_true;
            }
            break;
        case 5:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "count", 5)) {
                xpath_func = &xpath_count;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "false", 5)) {
                xpath_func = &xpath_false;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "floor", 5)) {
                xpath_func = &xpath_floor;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "round", 5)) {
                xpath_func = &xpath_round;
            }
            break;
        case 6:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "concat", 6)) {
                xpath_func = &xpath_concat;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "number", 6)) {
                xpath_func = &xpath_number;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "string", 6)) {
                xpath_func = &xpath_string;
            }
            break;
        case 7:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "boolean", 7)) {
                xpath_func = &xpath_boolean;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "ceiling", 7)) {
                xpath_func = &xpath_ceiling;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "current", 7)) {
                xpath_func = &xpath_current;
            }
            break;
        case 8:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "contains", 8)) {
                xpath_func = &xpath_contains;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "position", 8)) {
                xpath_func = &xpath_position;
            }
            break;
        case 9:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "substring", 9)) {
                xpath_func = &xpath_substring;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "translate", 9)) {
                xpath_func = &xpath_translate;
            }
            break;
        case 10:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "local-name", 10)) {
                xpath_func = &xpath_local_name;
            }
            break;
        case 11:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "starts-with", 11)) {
                xpath_func = &xpath_starts_with;
            }
            break;
        case 13:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "namespace-uri", 13)) {
                xpath_func = &xpath_namespace_uri;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "string-length", 13)) {
                xpath_func = &xpath_string_length;
            }
            break;
        case 15:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "normalize-space", 15)) {
                xpath_func = &xpath_normalize_space;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "substring-after", 15)) {
                xpath_func = &xpath_substring_after;
            }
            break;
        case 16:
            if (!strncmp(&exp->expr[exp->expr_pos[*cur_exp]], "substring-before", 16)) {
                xpath_func = &xpath_substring_before;
            }
            break;
        }

        if (!xpath_func) {
            LOGVAL(LYE_SPEC, line, "Unknown XPath function \"%.*s\".", exp->tok_len[*cur_exp], &exp->expr[exp->expr_pos[*cur_exp]]);
            return -1;
        }
    }

    LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
    ++(*cur_exp);

    /* '(' */
    if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR1, line)) {
        return -1;
    }
    LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
    LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
    ++(*cur_exp);

    if (set) {
        /* evaluate function */
        rc = xpath_func(args, arg_count, cur_node, set, line);
    } else {
        rc = EXIT_SUCCESS;
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
eval_number(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *any_node, struct lyxp_set *set, uint32_t line)
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

        set_fill_number(set, num, any_node->schema->module->ctx);
    }

    LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        if ((rc = eval_expr(exp, cur_exp, cur_node, set, line))) {
            return rc;
        }

        if (check_token(exp, *cur_exp, LYXP_TOKEN_PAR2, line)) {
            return -1;
        }
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
        if ((rc = eval_number(exp, cur_exp, cur_node, set, line))) {
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

        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
    int rc, unary_minus;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    if (set && (set->type != LYXP_SET_NODE_SET)) {
        LOGVAL(LYE_XPATH_INCTX, line, print_set_type(set), "UnaryExpr");
        return -1;
    }

    /* ('-')* */
    unary_minus = -1;
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && (exp->expr[exp->expr_pos[*cur_exp]] == '-')) {
        if (unary_minus == -1) {
            unary_minus = *cur_exp;
        } else {
            /* double '-' makes '+', ignore */
            unary_minus = -1;
        }
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
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

            /* eval */
            if ((rc = moveto_union(set, set2, cur_node, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }
        }
    }

    /* now we have all the unions in set and no other memory allocated */

    if (set && (unary_minus > -1)) {
        moveto_op_math(&exp->expr[exp->expr_pos[unary_minus]], set, NULL, cur_node);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates MultiplicativeExpr. Logs directly on error.
 *
 * [15] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
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
eval_multiplicative_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                         uint32_t line)
{
    int rc, prev_op;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    /* UnaryExpr */
    prev_exp = *cur_exp;
    if ((rc = eval_unary_expr(exp, cur_exp, cur_node, NULL, line))) {
        return rc;
    }
    if (set) {
        if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*cur_exp]] == '*') || (exp->tok_len[*cur_exp] == 3))) {
            /* there is an operator, we need to store the context */
            orig_set = set_copy(set, cur_node->schema->module->ctx);
        }
        *cur_exp = prev_exp;
        if ((rc = eval_unary_expr(exp, cur_exp, cur_node, set, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
    }

    /* ('*' / 'div' / 'mod' UnaryExpr)* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*cur_exp]] == '*') || (exp->tok_len[*cur_exp] == 3))) {
        prev_op = *cur_exp;

        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        prev_exp = *cur_exp;
        if ((rc = eval_unary_expr(exp, cur_exp, cur_node, NULL, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
        if (set) {
            if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
                    && ((exp->expr[exp->expr_pos[*cur_exp]] == '*') || (exp->tok_len[*cur_exp] == 3))) {
                /* there is another operator */
                set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            } else {
                /* there is no other '*', 'div', or 'mod', we can modify the original context */
                set2 = orig_set;
            }
            *cur_exp = prev_exp;
            if ((rc = eval_unary_expr(exp, cur_exp, cur_node, set2, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }

            /* eval */
            moveto_op_math(&exp->expr[exp->expr_pos[prev_op]], set, set2, cur_node);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates AdditiveExpr. Logs directly on error.
 *
 * [14] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
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
eval_additive_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                   uint32_t line)
{
    int rc, prev_op;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    /* MultiplicativeExpr */
    prev_exp = *cur_exp;
    if ((rc = eval_multiplicative_expr(exp, cur_exp, cur_node, NULL, line))) {
        return rc;
    }
    if (set) {
        if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*cur_exp]] == '+') || (exp->expr[exp->expr_pos[*cur_exp]] == '-'))) {
            /* there is an operator, we need to store the context */
            orig_set = set_copy(set, cur_node->schema->module->ctx);
        }
        *cur_exp = prev_exp;
        if ((rc = eval_multiplicative_expr(exp, cur_exp, cur_node, set, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
    }

    /* ('+' / '-' MultiplicativeExpr)* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*cur_exp]] == '+') || (exp->expr[exp->expr_pos[*cur_exp]] == '-'))) {
        prev_op = *cur_exp;

        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        prev_exp = *cur_exp;
        if ((rc = eval_multiplicative_expr(exp, cur_exp, cur_node, NULL, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
        if (set) {
            if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
                    && ((exp->expr[exp->expr_pos[*cur_exp]] == '+') || (exp->expr[exp->expr_pos[*cur_exp]] == '-'))) {
                /* there is another operator */
                set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            } else {
                /* there is no other '+', '-', we can modify the original context */
                set2 = orig_set;
            }
            *cur_exp = prev_exp;
            if ((rc = eval_multiplicative_expr(exp, cur_exp, cur_node, set2, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }

            /* eval */
            moveto_op_math(&exp->expr[exp->expr_pos[prev_op]], set, set2, cur_node);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates RelationalExpr. Logs directly on error.
 *
 * [13] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
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
eval_relational_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                     uint32_t line)
{
    int rc, prev_op;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    /* AdditiveExpr */
    prev_exp = *cur_exp;
    if ((rc = eval_additive_expr(exp, cur_exp, cur_node, NULL, line))) {
        return rc;
    }
    if (set) {
        if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*cur_exp]] == '<') || (exp->expr[exp->expr_pos[*cur_exp]] == '>'))) {
            /* there is an operator, we need to store the context */
            orig_set = set_copy(set, cur_node->schema->module->ctx);
        }
        *cur_exp = prev_exp;
        if ((rc = eval_additive_expr(exp, cur_exp, cur_node, set, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
    }

    /* ('<' / '>' / '<=' / '>=' MultiplicativeExpr)* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*cur_exp]] == '<') || (exp->expr[exp->expr_pos[*cur_exp]] == '>'))) {
        prev_op = *cur_exp;

        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        prev_exp = *cur_exp;
        if ((rc = eval_additive_expr(exp, cur_exp, cur_node, NULL, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
        if (set) {
            if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
                    && ((exp->expr[exp->expr_pos[*cur_exp]] == '<') || (exp->expr[exp->expr_pos[*cur_exp]] == '>'))) {
                /* there is another operator */
                set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            } else {
                /* there is no other comp op, we can modify the original context */
                set2 = orig_set;
            }
            *cur_exp = prev_exp;
            if ((rc = eval_additive_expr(exp, cur_exp, cur_node, set2, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }

            /* eval */
            moveto_op_comp(&exp->expr[exp->expr_pos[prev_op]], set, set2, cur_node);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates EqualityExpr. Logs directly on error.
 *
 * [12] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
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
eval_equality_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set,
                   uint32_t line)
{
    int rc, prev_op;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    /* RelationalExpr */
    prev_exp = *cur_exp;
    if ((rc = eval_relational_expr(exp, cur_exp, cur_node, NULL, line))) {
        return rc;
    }
    if (set) {
        if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*cur_exp]] == '=') || (exp->expr[exp->expr_pos[*cur_exp]] == '!'))) {
            /* there is an operator, we need to store the context */
            orig_set = set_copy(set, cur_node->schema->module->ctx);
        }
        *cur_exp = prev_exp;
        if ((rc = eval_relational_expr(exp, cur_exp, cur_node, set, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
    }

    /* ('=' / '!=' MultiplicativeExpr)* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*cur_exp]] == '=') || (exp->expr[exp->expr_pos[*cur_exp]] == '!'))) {
        prev_op = *cur_exp;

        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        prev_exp = *cur_exp;
        if ((rc = eval_relational_expr(exp, cur_exp, cur_node, NULL, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }
        if (set) {
            if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
                    && ((exp->expr[exp->expr_pos[*cur_exp]] == '=') || (exp->expr[exp->expr_pos[*cur_exp]] == '!'))) {
                /* there is another operator */
                set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            } else {
                /* there is no other comp op, we can modify the original context */
                set2 = orig_set;
            }
            *cur_exp = prev_exp;
            if ((rc = eval_relational_expr(exp, cur_exp, cur_node, set2, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }

            /* eval */
            moveto_op_comp(&exp->expr[exp->expr_pos[prev_op]], set, set2, cur_node);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates AndExpr. Logs directly on error.
 *
 * [11] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
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
eval_and_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    int rc;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    /* EqualityExpr */
    prev_exp = *cur_exp;
    if ((rc = eval_equality_expr(exp, cur_exp, cur_node, NULL, line))) {
        return rc;
    }
    if (set) {
        if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 3)) {
            /* there is an operator, we need to store the context */
            orig_set = set_copy(set, cur_node->schema->module->ctx);
        }
        *cur_exp = prev_exp;
        if ((rc = eval_equality_expr(exp, cur_exp, cur_node, set, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }

        /* cast to boolean, we know that will be the final result */
        if (orig_set) {
            set_cast(set, LYXP_SET_BOOLEAN, cur_node->schema->module->ctx);
            /* we are finished with evaluating, we're just gonna parse the rest */
            if (set->value.bool) {
                set_free(orig_set, cur_node->schema->module->ctx);
            }
        }
    }

    /* ('and' EqualityExpr)* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 3)) {
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        prev_exp = *cur_exp;
        if ((rc = eval_equality_expr(exp, cur_exp, cur_node, NULL, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }

        /* lazy evaluation */
        if (set && set->value.bool) {
            if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 3)) {
                /* there is another operator */
                set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            } else {
                /* there is no other comp op, we can modify the original context */
                set2 = orig_set;
            }
            *cur_exp = prev_exp;
            if ((rc = eval_equality_expr(exp, cur_exp, cur_node, set2, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }

            /* eval - just get boolean value actually */
            set_cast(set2, LYXP_SET_BOOLEAN, cur_node->schema->module->ctx);
            if (!set2->value.bool) {
                set->value.bool = 0;
                if (orig_set != set2) {
                    set_free(orig_set, cur_node->schema->module->ctx);
                }
            }
            set_free(set2, cur_node->schema->module->ctx);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates Expr. Logs directly on error.
 *
 * [10] Expr ::= AndExpr | Expr 'or' AndExpr
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
eval_expr(struct lyxp_expr *exp, uint16_t *cur_exp, struct lyd_node *cur_node, struct lyxp_set *set, uint32_t line)
{
    int rc;
    uint16_t prev_exp;
    struct lyxp_set *orig_set = NULL, *set2;

    /* AndExpr */
    prev_exp = *cur_exp;
    if ((rc = eval_and_expr(exp, cur_exp, cur_node, NULL, line))) {
        return rc;
    }
    if (set) {
        if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 2)) {
            /* there is an operator, we need to store the context */
            orig_set = set_copy(set, cur_node->schema->module->ctx);
        }
        *cur_exp = prev_exp;
        if ((rc = eval_and_expr(exp, cur_exp, cur_node, set, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }

        /* cast to boolean, we know that will be the final result */
        if (orig_set) {
            set_cast(set, LYXP_SET_BOOLEAN, cur_node->schema->module->ctx);
            /* we are finished with evaluating, we're just gonna parse the rest */
            if (!set->value.bool) {
                set_free(orig_set, cur_node->schema->module->ctx);
            }
        }
    }

    /* ('or' AndExpr)* */
    while (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 2)) {
        LOGDBG("XPATH: %s %sparsed %s[%u]", __func__, (set ? "" : "pre"), print_token(exp->tokens[*cur_exp]), exp->expr_pos[*cur_exp]);
        ++(*cur_exp);

        prev_exp = *cur_exp;
        if ((rc = eval_and_expr(exp, cur_exp, cur_node, NULL, line))) {
            set_free(orig_set, cur_node->schema->module->ctx);
            return rc;
        }

        /* lazy evaluation */
        if (set && !set->value.bool) {
            if (!check_token(exp, *cur_exp, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*cur_exp] == 2)) {
                /* there is another operator */
                set2 = set_copy(orig_set, cur_node->schema->module->ctx);
            } else {
                /* there is no other 'or' op, we can modify the original context */
                set2 = orig_set;
            }
            *cur_exp = prev_exp;
            if ((rc = eval_and_expr(exp, cur_exp, cur_node, set2, line))) {
                set_free(orig_set, cur_node->schema->module->ctx);
                if (set2 != orig_set) {
                    set_free(set2, cur_node->schema->module->ctx);
                }
                return rc;
            }

            /* eval - just get boolean value actually */
            set_cast(set2, LYXP_SET_BOOLEAN, cur_node->schema->module->ctx);
            if (set2->value.bool) {
                set->value.bool = 1;
                if (orig_set != set2) {
                    set_free(orig_set, cur_node->schema->module->ctx);
                }
            }
            set_free(set2, cur_node->schema->module->ctx);
        }
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
        /* TODO only for TREE_DFS, um? */
        root->schema = calloc(1, sizeof *root->schema);
        root->schema->nodetype = LYS_CONTAINER;
        LY_TREE_FOR(node, node) {
            node->parent = root;
        }

        *set = calloc(1, sizeof **set);
        set_add_node(*set, root, LYXP_NODE_ROOT, 0);

        rc = eval_expr(exp, &cur_exp, cur_node, *set, line);

        /* TODO remove fake root */
        LY_TREE_FOR(root->child, node) {
            node->parent = NULL;
        }
        /* TODO it may be in the results, we cannot free it */
        free(root->schema);
        free(root);

        if (!rc && (exp->used > cur_exp)) {
            LOGVAL(LYE_SPEC, line, "Unparsed characters \"%s\" left at the end of an XPath expression.",
                &exp->expr[exp->expr_pos[cur_exp]]);
            set_free(*set, cur_node->schema->module->ctx);
            *set = NULL;
            rc = -1;
        }
        if (rc) {
            set_free(*set, cur_node->schema->module->ctx);
            *set = NULL;
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
    struct lyxp_set *set;
    struct stat sb;
    int fd;
    char *addr;

    ctx = ly_ctx_new(NULL);

    fd = open("./test.yin", O_RDONLY);
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    lys_parse(ctx, addr, LYS_IN_YIN);
    munmap(addr, sb.st_size);
    close(fd);

    fd = open("./test_augment.yin", O_RDONLY);
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
        if (!lyxp_eval(argv[1], data, &set, 0)) {
            LOGDBG("XPATH: RESULT");
            debug_print_set(set, data);
            set_free(set, ctx);
        }
    }

    LY_TREE_FOR_SAFE(data, next, iter) {
        lyd_free(iter);
    }
    ly_ctx_destroy(ctx);

    return 0;
}
