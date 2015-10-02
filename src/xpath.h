/**
 * @file xpath.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions header
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
 * variables are not supported, "id()" function is not supported,
 * processing instruction and comment nodes are not supported,
 * bsolute path "/" is not supported, which is also reflected
 * in the grammar. Undefined rules and constants are tokens.
 *
 * Modified full grammar:
 *
 * [1] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [2] AbsoluteLocationPath ::= '/' RelativeLocationPath | '//' RelativeLocationPath
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

    /* Value of the node of this type determines the context to a certain extent.
     * Node value in schema:
     *      NULL       - top-level root
     *      LYS_NOTIF  - notification
     *      LYS_RPC    - RPC input
     *      LYS_OUTPUT - RPC output
     *
     * Node value in data:
     *      LYS_CONTAINER && !node->parent - top-level root
     *      LYS_NOTIF                      - notification
     *      LYS_RPC                        - RPC input
     *      LYS_OUTPUT                     - RPC output
     */
    LYXP_NODE_ROOT,

    LYXP_NODE_ELEM,
    LYXP_NODE_TEXT,
    LYXP_NODE_ATTR
};

/**
 * @brief Evaluate the XPath expression \p expr on data. The context must have
 * a single root without configuration meaning, but with schema with nodetype set.
 *
 * @param[in] expr XPath expression to use.
 * @param[in] cur_node Current (context) data node.
 * @param[out] set Result set.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int lyxp_eval(const char *expr, struct lyd_node *cur_node, struct lyxp_set **set, uint32_t line);

/**
 * @brief Evaluate the XPath expression \p expr on schema. The context root is only
 * virtual and is simulated on the models list of libyang context structure.
 *
 * @param[in] expr XPath expression to use.
 * @param[in] cur_snode Current (context) schema node.
 * @param[out] set Result set.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int lyxp_eval_schema(const char *expr, struct lys_node *cur_snode, struct lyxp_set **set, uint32_t line);

#endif /* _XPATH_H */
