/**
 * @file xpath.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions header
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
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

#include "log.h"

struct ly_ctx;
struct lysc_node;

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
 * [1] Expr ::= OrExpr // just an alias
 *
 * [2] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [3] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 * [4] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [5] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 * [7] Predicate ::= '[' Expr ']'
 * [8] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 * [9] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 * [10] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [11] OrExpr ::= AndExpr | OrExpr 'or' AndExpr
 * [12] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 * [13] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 * [14] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 * [15] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 * [16] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 * [17] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [18] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
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
 * @brief XPath (sub)expressions that can be repeated.
 */
enum lyxp_expr_type {
    LYXP_EXPR_NONE = 0,
    LYXP_EXPR_OR,
    LYXP_EXPR_AND,
    LYXP_EXPR_EQUALITY,
    LYXP_EXPR_RELATIONAL,
    LYXP_EXPR_ADDITIVE,
    LYXP_EXPR_MULTIPLICATIVE,
    LYXP_EXPR_UNARY,
    LYXP_EXPR_UNION,
};

/**
 * @brief Types of context nodes, #LYXP_NODE_ROOT_CONFIG used only in when or must conditions.
 */
enum lyxp_node_type {
    /* XML document roots */
    LYXP_NODE_ROOT,             /* access to all the data (node value first top-level node) */
    LYXP_NODE_ROOT_CONFIG,      /* <running> data context, no state data (node value first top-level node) */

    /* XML elements */
    LYXP_NODE_ELEM,             /* XML element (most common) */
    LYXP_NODE_TEXT,             /* XML text element (extremely specific use, unlikely to be ever needed) */
    LYXP_NODE_ATTR              /* XML attribute (in YANG cannot happen, do not use for the context node) */
};

/**
 * @brief Structure holding a parsed XPath expression.
 */
struct lyxp_expr {
    enum lyxp_token *tokens; /* array of tokens */
    uint16_t *tok_pos;       /* array of the token offsets in expr */
    uint16_t *tok_len;       /* array of token lengths in expr */
    enum lyxp_expr_type **repeat; /* array of expression types that this token begins and is repeated ended with 0,
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
 * of expression types that this token begins. These values
 * are used during evaluation to know whether we need to
 * duplicate the current context or not and to decide what
 * the current expression is (for example, if we are only
 * starting the parsing and the first token has no repeat,
 * we do not parse it as an OrExpr but directly as PathExpr).
 * Examples:
 *
 * Expression: "/ *[key1 and key2 or key1 < key2]"
 * Tokens: '/',  '*',  '[',  NameTest,  'and', NameTest, 'or', NameTest,        '<',  NameTest, ']'
 * Repeat: NULL, NULL, NULL, [AndExpr,  NULL,  NULL,     NULL, [RelationalExpr, NULL, NULL,     NULL
 *                            OrExpr,                           0],
 *                            0],
 *
 * Expression: "//node[key and node2]/key | /cont"
 * Tokens: '//',       'NameTest', '[',  'NameTest', 'and', 'NameTest', ']',  '/',  'NameTest', '|',  '/',  'NameTest'
 * Repeat: [UnionExpr, NULL,       NULL, [AndExpr,   NULL,  NULL,       NULL, NULL, NULL,       NULL, NULL, NULL
 *          0],                           0],
 *
 * Operators between expressions which this concerns:
 *     'or', 'and', '=', '!=', '<', '>', '<=', '>=', '+', '-', '*', 'div', 'mod', '|'
 */

/**
 * @brief Supported types of (partial) XPath results.
 */
enum lyxp_set_type {
    LYXP_SET_EMPTY = 0,
    LYXP_SET_NODE_SET,
    LYXP_SET_SCNODE_SET,
    LYXP_SET_BOOLEAN,
    LYXP_SET_NUMBER,
    LYXP_SET_STRING
};

/**
 * @brief Item stored in an XPath set hash table.
 */
struct lyxp_set_hash_node {
    struct lyd_node *node;
    enum lyxp_node_type type;
} _PACKED;

/**
 * @brief XPath set - (partial) result.
 */
struct lyxp_set {
    enum lyxp_set_type type;
    union {
        struct lyxp_set_node {
            struct lyd_node *node;
            enum lyxp_node_type type;
            uint32_t pos;
        } *nodes;
        struct lyxp_set_scnode {
            struct lysc_node *scnode;
            enum lyxp_node_type type;
            /* 0 - scnode was traversed, but not currently in the context,
             * 1 - scnode currently in context,
             * 2 - scnode in context and just added, so skip it for the current operation,
             * >=3 - scnode is not in context because we are in a predicate and this scnode was used/will be used later */
            uint32_t in_ctx;
        } *scnodes;
        struct lyxp_set_attr {
            struct lyd_attr *attr;
            enum lyxp_node_type type;
            uint32_t pos; /* if node_type is LYXP_SET_NODE_ATTR, it is the parent node position */
        } *attrs;
        char *str;
        long double num;
        int bool;
    } val;

    /* this is valid only for type LYXP_SET_NODE_SET and LYXP_SET_SCNODE_SET */
    uint32_t used;
    uint32_t size;
    struct hash_table *ht;

    /* XPath context information, this is valid only for type LYXP_SET_NODE_SET */
    uint32_t ctx_pos;
    uint32_t ctx_size;

    /* general context */
    struct ly_ctx *ctx;
    union {
        const struct lyd_node *ctx_node;
        const struct lysc_node *ctx_scnode;
    };
    enum lyxp_node_type root_type;
    const struct lys_module *local_mod;
    const struct lyd_node **trees;
    LYD_FORMAT format;
};

/**
 * @brief Evaluate an XPath expression on data. Be careful when using this function, the result can often
 * be confusing without thorough understanding of XPath evaluation rules defined in RFC 7950.
 *
 * @param[in] exp Parsed XPath expression to be evaluated.
 * @param[in] format Format of the XPath expression (more specifcally, of any used prefixes).
 * @param[in] local_mod Local module relative to the @p expr.
 * @param[in] ctx_node Current (context) data node, NULL for root node.
 * @param[in] ctx_node_type Current (context) data node type. For every standard case use #LYXP_NODE_ELEM. But there are
 * cases when the context node @p ctx_node is actually supposed to be the XML root, there is no such data node. So, in
 * this case just pass NULL for @p ctx_node and use an enum value for this kind of root (#LYXP_NODE_ROOT_CONFIG if
 * @p ctx_node has config true, otherwise #LYXP_NODE_ROOT). #LYXP_NODE_TEXT and #LYXP_NODE_ATTR can also be used,
 * but there are no use-cases in YANG.
 * @param[in] trees Data trees on which to perform the evaluation, they must include all the available tree (including
 * the tree of @p ctx_node).
 * @param[out] set Result set. Must be valid and in the same libyang context as @p ctx_node.
 * To be safe, always either zero or cast the @p set to empty. After done using, either cast
 * the @p set to empty (if allocated statically) or free it (if allocated dynamically) to
 * prevent memory leaks.
 * @param[in] options Whether to apply some evaluation restrictions.
 * @return LY_ERR (LY_EINVAL, LY_EMEM, LY_EINT, LY_EVALID for invalid argument types/count,
 * LY_EINCOMPLETE for unresolved when).
 */
LY_ERR lyxp_eval(struct lyxp_expr *exp, LYD_FORMAT format, const struct lys_module *local_mod, const struct lyd_node *ctx_node,
                 enum lyxp_node_type ctx_node_type, const struct lyd_node **trees, struct lyxp_set *set, int options);

#define LYXP_SCHEMA 0x01 /**< Apply data node access restrictions defined for 'when' and 'must' evaluation. */

/**
 * @brief Get all the partial XPath nodes (atoms) that are required for @p exp to be evaluated.
 *
 * If any LYXP_SCNODE* options is set, only fatal errors are printed, otherwise they are downgraded
 * to warnings.
 *
 * @param[in] exp Parsed XPath expression to be evaluated.
 * @param[in] format Format of the XPath expression (more specifcally, of any used prefixes).
 * @param[in] local_mod Local module relative to the @p exp.
 * @param[in] ctx_scnode Current (context) schema node, NULL for root node.
 * @param[in] ctx_scnode_type Current (context) schema node type.
 * @param[out] set Result set. Must be valid and in the same libyang context as @p ctx_scnode.
 * To be safe, always either zero or cast the @p set to empty. After done using, either cast
 * the @p set to empty (if allocated statically) or free it (if allocated dynamically) to
 * prevent memory leaks.
 * @param[in] options Whether to apply some evaluation restrictions, one flag must always be used.
 * @return LY_ERR (same as lyxp_eval()).
 */
LY_ERR lyxp_atomize(struct lyxp_expr *exp, LYD_FORMAT format, const struct lys_module *local_mod, const struct lysc_node *ctx_scnode,
                    enum lyxp_node_type ctx_scnode_type, struct lyxp_set *set, int options);

/* these are used only internally */
#define LYXP_SCNODE 0x02        /**< No special data tree access modifiers. */
#define LYXP_SCNODE_SCHEMA 0x04 /**< Apply schema node access restrictions defined for 'when' and 'must' evaluation. */
#define LYXP_SCNODE_OUTPUT 0x08 /**< Search RPC/action output instead of input. */

#define LYXP_SCNODE_ALL 0x0E

/**
 * @brief Cast XPath set to another type.
 *        Indirectly context position aware.
 *
 * @param[in] set Set to cast.
 * @param[in] target Target type to cast \p set into.
 * @return LY_ERR
 */
LY_ERR lyxp_set_cast(struct lyxp_set *set, enum lyxp_set_type target);

/**
 * @brief Insert schema node into set.
 *
 * @param[in] set Set to insert into.
 * @param[in] node Node to insert.
 * @param[in] node_type Node type of @p node.
 * @return Index of the inserted node in set.
 */
int lyxp_set_scnode_insert_node(struct lyxp_set *set, const struct lysc_node *node, enum lyxp_node_type node_type);

/**
 * @brief Check for duplicates in a schema node set.
 *
 * @param[in] set Set to check.
 * @param[in] node Node to look for in @p set.
 * @param[in] node_type Type of @p node.
 * @param[in] skip_idx Index from @p set to skip.
 * @return Index of the found node, -1 if not found.
 */
int lyxp_set_scnode_dup_node_check(struct lyxp_set *set, const struct lysc_node *node, enum lyxp_node_type node_type,
                                   int skip_idx);

/**
 * @brief Merge 2 schema node sets.
 *
 * @param[in] set1 Set to merge into.
 * @param[in] set2 Set to merge. Its content is freed.
 */
void lyxp_set_scnode_merge(struct lyxp_set *set1, struct lyxp_set *set2);

/**
 * @brief Parse an XPath expression into a structure of tokens.
 *        Logs directly.
 *
 * https://www.w3.org/TR/1999/REC-xpath-19991116/#exprlex
 *
 * @param[in] ctx Context for errors.
 * @param[in] expr XPath expression to parse. It is duplicated.
 * @return Filled expression structure or NULL on error.
 */
struct lyxp_expr *lyxp_expr_parse(struct ly_ctx *ctx, const char *expr);

/**
 * @brief Frees a parsed XPath expression. @p expr should not be used afterwards.
 *
 * @param[in] ctx libyang context of the expression.
 * @param[in] expr Expression to free.
 */
void lyxp_expr_free(struct ly_ctx *ctx, struct lyxp_expr *expr);

#endif /* _XPATH_H */
