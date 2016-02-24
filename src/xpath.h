/**
 * @file xpath.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions header
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef _XPATH_H
#define _XPATH_H

#include <stdint.h>

#include "tree_schema.h"
#include "tree_data.h"

/*
 * XPath evaluator fully compliant with http://www.w3.org/TR/1999/REC-xpath-19991116/
 * except the following restrictions in the grammar.
 *
 * PARSED GRAMMAR
 *
 * Full axes are not supported, abbreviated forms must be used,
 * variables are not supported, "id()" and "name(node-set?)"
 * functions are not supported, and processing instruction and
 * comment nodes are not supported, which is also reflected in
 * the grammar. Undefined rules and constants are tokens.
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

/* expression tokens allocation */
#define LYXP_EXPR_SIZE_START 10
#define LYXP_EXPR_SIZE_STEP 5

/* XPath matches allocation */
#define LYXP_SET_SIZE_START 2
#define LYXP_SET_SIZE_STEP 2

/* building string when casting */
#define LYXP_STRING_CAST_SIZE_START 64
#define LYXP_STRING_CAST_SIZE_STEP 16

/**
 * @brief Tokens that can be in an XPath expression.
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

/**
 * @brief Structure holding a parsed XPath expression.
 */
struct lyxp_expr {
    enum lyxp_token *tokens; /* array of tokens */
    uint16_t *expr_pos;      /* array of pointers to the expression in expr (idx of the beginning) */
    uint8_t *tok_len;        /* array of token lengths in expr */
    uint8_t **repeat;        /* array of the operator token indices that succeed this expression ended with 0,
                                more in the comment after this declaration */
    uint16_t used;           /* used array items */
    uint16_t size;           /* allocated array items */

    const char *expr;        /* the original XPath expression */
};

/*
 * lyxp_expr repeat
 *
 * This value is NULL for all the tokens that do not begin an
 * expression which can be repeated. Otherwise it is an array
 * of indices in the tokens array that are an operator for
 * which the current expression is an operand. These values
 * are used during evaluation to know whether we need to
 * duplicate the current context or not. Examples:
 *
 * Expression: "/ *[key1 and key2 or key1 < key2]"
 * Tokens: '/',  '*',  '[',  NameTest,  'and', NameTest, 'or', NameTest, '<',  NameTest, ']'
 * Repeat: NULL, NULL, NULL, [4, 6, 0], NULL,  NULL,     NULL, [8, 0],   NULL, NULL,     NULL
 *
 * Expression: "//node[key and node2]/key | /cont"
 * Tokens: '//',   'NameTest', '[',  'NameTest', 'and', 'NameTest', ']',  '/',  'NameTest', '|',  '/',  'NameTest'
 * Repeat: [9, 0], NULL,       NULL, [4, 0],     NULL,  NULL,       NULL, NULL, NULL,       NULL, NULL, NULL
 *
 * Operators between expressions that are concerned:
 *     'or', 'and', '=', '!=', '<', '>', '<=', '>=', '+', '-', '*', 'div', 'mod', '|'
 */

/**
 * @brief Supported types of (partial) XPath results.
 */
enum lyxp_set_type {
    LYXP_SET_EMPTY = 0,
    LYXP_SET_NODE_SET,
    LYXP_SET_BOOLEAN,
    LYXP_SET_NUMBER,
    LYXP_SET_STRING
};

/**
 * @brief XPath set - (partial) result.
 */
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

/**
 * @brief Types of nodes that can be in an LYXP_SET_NODE_SET XPath set.
 */
enum lyxp_node_type {
    LYXP_NODE_ROOT_CONFIG,      /* <running> data context (node value NULL) */
    LYXP_NODE_ROOT_STATE,       /* <running> + state data context (node value NULL) */
    LYXP_NODE_ROOT_NOTIF,       /* notification context (node value LYS_NOTIF) */
    LYXP_NODE_ROOT_RPC,         /* RPC (input) context (node value LYS_RPC) */
    LYXP_NODE_ROOT_OUTPUT,      /* RPC output-only context (node value LYS_RPC) */

    LYXP_NODE_ELEM,
    LYXP_NODE_TEXT,
    LYXP_NODE_ATTR
};

/**
 * @brief Evaluate the XPath expression \p expr on data. Be careful when using this function, the result can often
 * be confusing without thorough understanding of XPath evaluation rules defined in RFC 6020.
 *
 * @param[in] expr XPath expression to evaluate. Must be in JSON format (prefixes are model names).
 * @param[in] cur_node Current (context) data node.
 * @param[out] set Result set. Must be valid (zeroed usually).
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int lyxp_eval(const char *expr, const struct lyd_node *cur_node, struct lyxp_set *set, int when_must_eval, uint32_t line);

/**
 * @brief Check the syntax of an XPath expression \p expr. Since it's only syntactic,
 * node and function names may still be invalid.
 *
 * @param[in] expr XPath expression to check.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on pass, -1 on failure.
 */
int lyxp_syntax_check(const char *expr, uint32_t line);

/**
 * @brief Print \p set contents.
 *
 * @param[in] f File stream to use.
 * @param[in] set Set to print.
 */
void lyxp_set_print_xml(FILE *f, struct lyxp_set *set);

/**
 * @brief Cast XPath set to another type.
 *        Indirectly context position aware.
 *
 * @param[in] set Set to cast.
 * @param[in] target Target type to cast \p set into.
 * @param[in] cur_node Current (context) data node.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 */
void lyxp_set_cast(struct lyxp_set *set, enum lyxp_set_type target, const struct lyd_node *cur_node, int when_must_eval);

/**
 * @brief Free contents of an XPath \p set.
 *
 * @param[in] set Set to free.
 * @param[in] ctx libyang context to use.
 */
void lyxp_set_free(struct lyxp_set *set, struct ly_ctx *ctx);

#endif /* _XPATH_H */
