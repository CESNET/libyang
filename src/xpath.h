/**
 * @file xpath.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions header
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_XPATH_H
#define LY_XPATH_H

#include <stddef.h>
#include <stdint.h>

#include "compat.h"
#include "log.h"
#include "tree.h"
#include "tree_schema.h"

struct ly_ctx;
struct lyd_node;

/**
 * @internal
 * @page internals
 * @section internalsXpath XPath Implementation
 *
 * XPath evaluator fully compliant with http://www.w3.org/TR/1999/REC-xpath-19991116/
 * except the following restrictions in the grammar.
 *
 * @subsection internalsXpathGrammar Parsed Grammar
 *
 * Full axes are not supported, abbreviated forms must be used,
 * "id()" function is not supported, and processing instruction and comment nodes are not supported,
 * which is also reflected in the grammar. Undefined rules and constants are tokens.
 *
 * Modified full grammar:
 * @code
 * [1] Expr ::= OrExpr // just an alias
 *
 * [2] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [3] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 * [4] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [5] Step ::= (AxisName '::' | '@')? NodeTest Predicate* | '.' | '..'
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 * [7] NameTest ::= '*' | NCName ':' '*' | QName
 * [8] NodeType ::= 'text' | 'node'
 * [9] Predicate ::= '[' Expr ']'
 * [10] PrimaryExpr ::= VariableReference | '(' Expr ')' | Literal | Number | FunctionCall
 * [11] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 * [12] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [13] OrExpr ::= AndExpr | OrExpr 'or' AndExpr
 * [14] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 * [15] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 * [16] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 * [17] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 * [18] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 * [19] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [20] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 * @endcode
 */

/* expression tokens allocation */
#define LYXP_EXPR_SIZE_START 10
#define LYXP_EXPR_SIZE_STEP 5

/* XPath matches allocation */
#define LYXP_SET_SIZE_START 4
#define LYXP_SET_SIZE_MUL_STEP 2

/* building string when casting */
#define LYXP_STRING_CAST_SIZE_START 64
#define LYXP_STRING_CAST_SIZE_STEP 16

/* Maximum number of nested expressions. */
#define LYXP_MAX_BLOCK_DEPTH 100

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
    LYXP_TOKEN_DCOLON,        /* '::' */
    LYXP_TOKEN_NAMETEST,      /* NameTest */
    LYXP_TOKEN_NODETYPE,      /* NodeType */
    LYXP_TOKEN_VARREF,        /* VariableReference */
    LYXP_TOKEN_FUNCNAME,      /* FunctionName */
    LYXP_TOKEN_OPER_LOG,      /* Operator 'and', 'or' */
    LYXP_TOKEN_OPER_EQUAL,    /* Operator '=' */
    LYXP_TOKEN_OPER_NEQUAL,   /* Operator '!=' */
    LYXP_TOKEN_OPER_COMP,     /* Operator '<', '<=', '>', '>=' */
    LYXP_TOKEN_OPER_MATH,     /* Operator '+', '-', '*', 'div', 'mod', '-' (unary) */
    LYXP_TOKEN_OPER_UNI,      /* Operator '|' */
    LYXP_TOKEN_OPER_PATH,     /* Operator '/' */
    LYXP_TOKEN_OPER_RPATH,    /* Operator '//' (recursive path) */
    LYXP_TOKEN_AXISNAME,      /* AxisName */
    LYXP_TOKEN_LITERAL,       /* Literal - with either single or double quote */
    LYXP_TOKEN_NUMBER         /* Number */
};

/**
 * @brief XPath Axes types.
 */
enum lyxp_axis {
    LYXP_AXIS_ANCESTOR,
    LYXP_AXIS_ANCESTOR_OR_SELF,
    LYXP_AXIS_ATTRIBUTE,
    LYXP_AXIS_CHILD,
    LYXP_AXIS_DESCENDANT,
    LYXP_AXIS_DESCENDANT_OR_SELF,
    LYXP_AXIS_FOLLOWING,
    LYXP_AXIS_FOLLOWING_SIBLING,
    // LYXP_AXIS_NAMESPACE,          /* not supported */
    LYXP_AXIS_PARENT,
    LYXP_AXIS_PRECEDING,
    LYXP_AXIS_PRECEDING_SIBLING,
    LYXP_AXIS_SELF
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
    LYXP_EXPR_UNION
};

/**
 * @brief Types of context nodes, #LYXP_NODE_ROOT_CONFIG used only in when or must conditions.
 */
enum lyxp_node_type {
    LYXP_NODE_NONE,             /* invalid node type */

    /* XML document roots */
    LYXP_NODE_ROOT,             /* access to all the data (node value first top-level node) */
    LYXP_NODE_ROOT_CONFIG,      /* <running> data context, no state data (node value first top-level node) */

    /* XML elements */
    LYXP_NODE_ELEM,             /* YANG data element (most common) */
    LYXP_NODE_TEXT,             /* YANG data text element (extremely specific use, unlikely to be ever needed) */
    LYXP_NODE_META              /* YANG metadata (do not use for the context node) */
};

/**
 * @brief Structure holding a parsed XPath expression.
 */
struct lyxp_expr {
    enum lyxp_token *tokens; /**< Array of tokens. */
    uint32_t *tok_pos;       /**< Array of the token offsets in expr. */
    uint32_t *tok_len;       /**< Array of token lengths in expr. */
    enum lyxp_expr_type **repeat; /**< Array of expression types that this token begins and is repeated ended with 0,
                                       more in the comment after this declaration. */
    uint32_t used;           /**< Used array items. */
    uint32_t size;           /**< Allocated array items. */

    const char *expr;        /**< The original XPath expression. */
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
 * Expr:   "/ *[key1 and key2 or key1 < key2]"
 * Tokens: '/'  '*'  '['  NameTest 'and'  NameTest 'or' NameTest       '<'  NameTest ']'
 * Repeat: NULL NULL NULL _        NULL   NULL     NULL _              NULL NULL     NULL
 *                        |                             v
 *                        v                             RelationalExpr 0
 *                        AndExpr  OrExpr 0
 *
 * Expr:   "//node[key and node2]/key | /cont"
 * Tokens: '//'      NameTest '['  NameTest 'and' NameTest ']'  '/'  NameTest '|'  '/'  NameTest
 * Repeat: _         NULL     NULL _        NULL  NULL     NULL NULL NULL     NULL NULL NULL
 *         |                       v
 *         v                       AndExpr  0
 *         UnionExpr 0
 *
 * Operators between expressions which this concerns:
 *     'or', 'and', '=', '!=', '<', '>', '<=', '>=', '+', '-', '*', 'div', 'mod', '|'
 */

/**
 * @brief Supported types of (partial) XPath results.
 */
enum lyxp_set_type {
    LYXP_SET_NODE_SET = 0,
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
 * @brief XPath variable bindings.
 */
struct lyxp_var {
    char *name;     /**< Variable name. In the XPath expression, the name is preceded by a '$' character. */
    char *value;    /**< The value of a variable is an object, which can be of any of the type that are possible
                         for the value of an expression. */
};

/**
 * @brief XPath set - (partial) result.
 */
struct lyxp_set {
    enum lyxp_set_type type;             /**< Type of the object (value). */

    union {
        struct lyxp_set_node {
            struct lyd_node *node;       /**< Data node. */
            enum lyxp_node_type type;    /**< Type of the node. */
            uint32_t pos;                /**< Unique node position in the data. */
        } *nodes;                        /**< Set of data nodes. */
        struct lyxp_set_scnode {
            struct lysc_node *scnode;    /**< Compiled YANG node. */
            enum lyxp_node_type type;    /**< Type of the node. */

/* _START and _ATOM values should have grouped values */
#define LYXP_SET_SCNODE_START         -2 /**< scnode not traversed, currently (the only node) in context */
#define LYXP_SET_SCNODE_START_USED    -1 /**< scnode not traversed except for the eval start, not currently in the context */
#define LYXP_SET_SCNODE_ATOM_NODE      0 /**< scnode was traversed, but not currently in the context */
#define LYXP_SET_SCNODE_ATOM_VAL       1 /**< scnode was traversed and its value used, but not currently in the context */
#define LYXP_SET_SCNODE_ATOM_CTX       2 /**< scnode currently in context */
#define LYXP_SET_SCNODE_ATOM_NEW_CTX   3 /**< scnode in context and just added, so skip it for the current operation */
#define LYXP_SET_SCNODE_ATOM_PRED_CTX  4 /**< includes any higher value - scnode is not in context because we are in
                                              a predicate and this scnode was used/will be used later */
            int32_t in_ctx;             /**< Flag specifies the state of the node in context. Values are defined
                                             as LYXP_SET_SCNODE_* */
            enum lyxp_axis axis;        /**< Axis defines on what axis was this schema node reached. */
        } *scnodes;                     /**< Set of compiled YANG data nodes. */
        struct lyxp_set_meta {
            struct lyd_meta *meta;      /**< Node that provides information about metadata of a data element. */
            enum lyxp_node_type type;   /**< Type of the node. */
            uint32_t pos;               /**< Unique node position in the data. if node_type is LYXP_SET_NODE_META,
                                             it is the parent node position */
        } *meta;                        /**< Set of YANG metadata objects. */
        char *str;                      /**< String object. */
        long double num;                /**< Object of the floating-point number. */
        ly_bool bln;                    /**< Boolean object. */
    } val;                              /**< Evaluated object (value). */

    /* this is valid only for type LYXP_SET_NODE_SET and LYXP_SET_SCNODE_SET */
    uint32_t used;              /**< Number of nodes in the set. */
    uint32_t size;              /**< Allocated size for the set. */
    struct ly_ht *ht;           /**< Hash table for quick determination of whether a node is in the set. */

    /* XPath context information, this is valid only for type LYXP_SET_NODE_SET */
    uint32_t ctx_pos;           /**< Position of the current examined node in the set. */
    uint32_t ctx_size;          /**< Position of the last node at the time the node was examined. */
    ly_bool non_child_axis;     /**< Whether any node change was performed on a non-child axis. */

    ly_bool not_found;          /**< Set if a node is not found and it is considered an error. */

    /* general context */
    struct ly_ctx *ctx;                     /**< General context for logging. */

    union {
        const struct lyd_node *cur_node;    /**< Current (original context) node. */
        const struct lysc_node *cur_scnode; /**< Current (original context) compiled node. */
    };
    enum lyxp_node_type root_type;          /**< Type of root node. */
    const struct lysc_node *context_op;     /**< Schema of the current node. */
    const struct lyd_node *tree;            /**< Data tree on which to perform the evaluation. */
    const struct lys_module *cur_mod;       /**< Current module for the expression (where it was "instantiated"). */
    LY_VALUE_FORMAT format;                 /**< Format of the XPath expression. */
    void *prefix_data;                      /**< Format-specific prefix data (see ::ly_resolve_prefix). */
    const struct lyxp_var *vars;            /**< XPath variables. [Sized array](@ref sizedarrays).
                                                 Set of variable bindings. */
};

/**
 * @brief Get string format of an XPath token.
 *
 * @param[in] tok Token to transform.
 * @return Token type string.
 */
const char *lyxp_token2str(enum lyxp_token tok);

/**
 * @brief Evaluate an XPath expression on data. Be careful when using this function, the result can often
 * be confusing without thorough understanding of XPath evaluation rules defined in RFC 7950.
 *
 * Traverses (valid) opaque nodes in the evaluation.
 *
 * @param[in] ctx libyang context to use.
 * @param[in] exp Parsed XPath expression to be evaluated.
 * @param[in] cur_mod Current module for the expression (where it was "instantiated").
 * @param[in] format Format of the XPath expression (more specifically, of any used prefixes).
 * @param[in] prefix_data Format-specific prefix data (see ::ly_resolve_prefix).
 * @param[in] cur_node Current data node, NULL in case of the root node. Equal to @p ctx_node unless a
 * subexpression is being evaluated.
 * @param[in] ctx_node Starting context data node, NULL in case of the root node. Equal to @p cur_node unless a
 * subexpression is being evaluated.
 * @param[in] tree Data tree on which to perform the evaluation, it must include all the available data (including
 * the tree of @p ctx_node). Can be any node of the tree, it is adjusted.
 * @param[in] vars [Sized array](@ref sizedarrays) of XPath variables.
 * @param[out] set Result set.
 * @param[in] options Whether to apply some evaluation restrictions.
 * @return LY_EVALID for invalid argument types/count,
 * @return LY_EINCOMPLETE for unresolved when,
 * @return LY_EINVAL, LY_EMEM, LY_EINT for other errors.
 */
LY_ERR lyxp_eval(const struct ly_ctx *ctx, const struct lyxp_expr *exp, const struct lys_module *cur_mod,
        LY_VALUE_FORMAT format, void *prefix_data, const struct lyd_node *cur_node, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, const struct lyxp_var *vars, struct lyxp_set *set, uint32_t options);

/**
 * @brief Get all the partial XPath nodes (atoms) that are required for @p exp to be evaluated.
 *
 * @param[in] ctx libyang context to use.
 * @param[in] exp Parsed XPath expression to be evaluated.
 * @param[in] cur_mod Current module for the expression (where it was "instantiated").
 * @param[in] format Format of the XPath expression (more specifically, of any used prefixes).
 * @param[in] prefix_data Format-specific prefix data (see ::ly_resolve_prefix).
 * @param[in] cur_scnode Current schema node, NULL in case of the root node. Equal to @p ctx_scnode unless a
 * subexpression is being atomized.
 * @param[in] ctx_scnode Starting context schema node, NULL in case of the root node. Equal to @p cur_scnode unless a
 * subexpression is being atomized.
 * @param[out] set Result set.
 * @param[in] options Whether to apply some evaluation restrictions, one flag must always be used.
 * @return LY_ERR (same as ::lyxp_eval()).
 */
LY_ERR lyxp_atomize(const struct ly_ctx *ctx, const struct lyxp_expr *exp, const struct lys_module *cur_mod,
        LY_VALUE_FORMAT format, void *prefix_data, const struct lysc_node *cur_scnode,
        const struct lysc_node *ctx_scnode, struct lyxp_set *set, uint32_t options);

/** used only internally, maps with @ref findxpathoptions */
#define LYXP_IGNORE_WHEN     0x01   /**< Ignore unevaluated when in data nodes and do not return ::LY_EINCOMPLETE. */
#define LYXP_SCHEMA          0x02   /**< Apply data node access restrictions defined for 'when' and 'must' evaluation. */
#define LYXP_SCNODE          0x04   /**< No special tree access modifiers. */
#define LYXP_SCNODE_SCHEMA   LYS_FIND_XP_SCHEMA /**< Apply node access restrictions defined for 'when' and 'must' evaluation. */
#define LYXP_SCNODE_OUTPUT   LYS_FIND_XP_OUTPUT /**< Search RPC/action output nodes instead of input ones. */
#define LYXP_SCNODE_ALL      0x1C   /**< mask for all the LYXP_SCNODE_* values */
#define LYXP_SKIP_EXPR       0x20   /**< The rest of the expression will not be evaluated (lazy evaluation) */
#define LYXP_SCNODE_ERROR    LYS_FIND_NO_MATCH_ERROR    /**< Return error if a path segment matches no nodes, otherwise only
                                                             warning is printed. */
#define LYXP_ACCESS_TREE_ALL 0x80   /**< Explicit accessible tree of all the nodes. */
#define LYXP_ACCESS_TREE_CONFIG 0x0100  /**< Explicit accessible tree of only configuration data. */
#define LYXP_SCNODE_SCHEMAMOUNT LYS_FIND_SCHEMAMOUNT    /**< Nodes from mounted modules are also accessible. */

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
 * @brief Free dynamic content of a set.
 *
 * @param[in] set Set to modify.
 */
void lyxp_set_free_content(struct lyxp_set *set);

/**
 * @brief Check for duplicates in a schema node set.
 *
 * @param[in] set Set to check.
 * @param[in] node Node to look for in @p set.
 * @param[in] node_type Type of @p node.
 * @param[in] skip_idx Index from @p set to skip.
 * @param[out] index_p Optional pointer to store index if the node is found.
 * @return Boolean value whether the @p node found or not.
 */
ly_bool lyxp_set_scnode_contains(struct lyxp_set *set, const struct lysc_node *node, enum lyxp_node_type node_type,
        int skip_idx, uint32_t *index_p);

/**
 * @brief Merge 2 schema node sets.
 *
 * @param[in] set1 Set to merge into.
 * @param[in] set2 Set to merge. Its content is freed.
 */
void lyxp_set_scnode_merge(struct lyxp_set *set1, struct lyxp_set *set2);

/**
 * @brief Insert schema node into set.
 *
 * @param[in] set Set to insert into.
 * @param[in] node Node to insert.
 * @param[in] node_type Node type of @p node.
 * @param[in] axis Axis that @p node was reached on.
 * @param[out] index_p Optional pointer to store the index of the inserted @p node.
 * @return LY_SUCCESS on success.
 * @return LY_EMEM on memory allocation failure.
 */
LY_ERR lyxp_set_scnode_insert_node(struct lyxp_set *set, const struct lysc_node *node, enum lyxp_node_type node_type,
        enum lyxp_axis axis, uint32_t *index_p);

/**
 * @brief Parse an XPath expression into a structure of tokens.
 *        Logs directly.
 *
 * https://www.w3.org/TR/1999/REC-xpath-19991116/#exprlex
 *
 * @param[in] ctx Context for errors.
 * @param[in] expr_str XPath expression to parse. It is duplicated.
 * @param[in] expr_len Length of @p expr, can be 0 if @p expr is 0-terminated.
 * @param[in] reparse Whether to re-parse the expression to finalize full XPath parsing and fill
 * information about expressions and their operators (fill repeat).
 * @param[out] expr_p Pointer to return the filled expression structure.
 * @return LY_SUCCESS in case of success.
 * @return LY_EMEM in case of memory allocation failure.
 * @return LY_EVALID in case of invalid XPath expression in @p expr_str.
 */
LY_ERR lyxp_expr_parse(const struct ly_ctx *ctx, const char *expr_str, size_t expr_len, ly_bool reparse,
        struct lyxp_expr **expr_p);

/**
 * @brief Duplicate parsed XPath expression.
 *
 * If @p start_idx and @p end_idx are both 0, the whole expression is duplicated.
 *
 * @param[in] ctx Context with a dictionary.
 * @param[in] exp Parsed expression.
 * @param[in] start_idx Starting @p exp index to duplicate.
 * @param[in] end_idx Last @p exp index to duplicate.
 * @param[out] dup Duplicated structure.
 * @return LY_ERR value.
 */
LY_ERR lyxp_expr_dup(const struct ly_ctx *ctx, const struct lyxp_expr *exp, uint32_t start_idx, uint32_t end_idx,
        struct lyxp_expr **dup);

/**
 * @brief Look at the next token and check its kind.
 *
 * @param[in] ctx Context for logging, not logged if NULL.
 * @param[in] exp Expression to use.
 * @param[in] tok_idx Token index in the expression \p exp.
 * @param[in] want_tok Expected token.
 * @return LY_EINCOMPLETE on EOF,
 * @return LY_ENOT on non-matching token,
 * @return LY_SUCCESS on success.
 */
LY_ERR lyxp_check_token(const struct ly_ctx *ctx, const struct lyxp_expr *exp, uint32_t tok_idx, enum lyxp_token want_tok);

/**
 * @brief Look at the next token and skip it if it matches the expected one.
 *
 * @param[in] ctx Context for logging, not logged if NULL.
 * @param[in] exp Expression to use.
 * @param[in,out] tok_idx Token index in the expression \p exp, is updated.
 * @param[in] want_tok Expected token.
 * @return LY_EINCOMPLETE on EOF,
 * @return LY_ENOT on non-matching token,
 * @return LY_SUCCESS on success.
 */
LY_ERR lyxp_next_token(const struct ly_ctx *ctx, const struct lyxp_expr *exp, uint32_t *tok_idx, enum lyxp_token want_tok);

/**
 * @brief Look at the next token and skip it if it matches either of the 2 expected ones.
 *
 * @param[in] ctx Context for logging, not logged if NULL.
 * @param[in] exp Expression to use.
 * @param[in,out] tok_idx Token index in the expression \p exp, is updated.
 * @param[in] want_tok1 Expected token 1.
 * @param[in] want_tok2 Expected token 2.
 * @return LY_EINCOMPLETE on EOF,
 * @return LY_ENOT on non-matching token,
 * @return LY_SUCCESS on success.
 */
LY_ERR lyxp_next_token2(const struct ly_ctx *ctx, const struct lyxp_expr *exp, uint32_t *tok_idx,
        enum lyxp_token want_tok1, enum lyxp_token want_tok2);

/**
 * @brief Find variable named @p name in @p vars.
 *
 * @param[in] ctx Context for logging, not logged if NULL.
 * @param[in] vars [Sized array](@ref sizedarrays) of XPath variables.
 * @param[in] name Name of the variable being searched.
 * @param[in] name_len Name length can be set to 0 if @p name is terminated by null byte.
 * @param[out] var Variable that was found. The parameter is optional.
 * @return LY_SUCCESS if the variable was found, otherwise LY_ENOTFOUND.
 */
LY_ERR lyxp_vars_find(const struct ly_ctx *ctx, const struct lyxp_var *vars, const char *name, size_t name_len,
        struct lyxp_var **var);

/**
 * @brief Frees a parsed XPath expression. @p expr should not be used afterwards.
 *
 * @param[in] ctx libyang context of the expression.
 * @param[in] expr Expression to free.
 */
void lyxp_expr_free(const struct ly_ctx *ctx, struct lyxp_expr *expr);

#endif /* LY_XPATH_H */
