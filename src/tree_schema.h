/**
 * @file tree_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of YANG schema trees.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_H_
#define LY_TREE_SCHEMA_H_

#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#include <stdint.h>
#include <stdio.h>

#include "log.h"
#include "tree.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_ctx;
struct ly_path;
struct ly_set;
struct lys_module;
struct lysc_node;
struct lyxp_expr;

/**
 * @page howtoSchema YANG Modules
 *
 * To be able to work with YANG data instances, libyang has to represent YANG data models. All the processed modules are stored
 * in libyang [context](@ref howtoContext) and loaded using [parser functions](@ref howtoSchemaParsers). It means, that there is
 * no way to create/change YANG module programmatically. However, all the YANG model definitions are available and can be examined
 * through the C structures. All the context's modules together form YANG Schema for the data being instantiated.
 *
 * Any YANG module is represented as ::lys_module. In fact, the module is represented in two different formats. As ::lys_module.parsed,
 * there is a parsed schema reflecting the source YANG module. It is exactly what is read from the input. This format is good for
 * converting from one format to another (YANG to YIN and vice versa), but it is not very useful for validating/manipulating YANG
 * data. Therefore, there is ::lys_module.compiled storing the compiled YANG module. It is based on the parsed module, but all the
 * references are resolved. It means that, for example, there are no `grouping`s or `typedef`s since they are supposed to be placed instead of
 * `uses` or `type` references. This split also means, that the YANG module is fully validated after compilation of the parsed
 * representation of the module. YANG submodules are available only in the parsed representation. When a submodule is compiled, it
 * is fully integrated into its main module.
 *
 * The context can contain even modules without the compiled representation. Such modules are still useful as imports of other
 * modules. The grouping or typedef definition can be even compiled into the importing modules. This is actually the main
 * difference between the imported and implemented modules in the libyang context. The implemented modules are compiled while the
 * imported modules are only parsed.
 *
 * By default, the module is implemented (and compiled) in case it is explicitly loaded or referenced in another module as
 * target of leafref, augment or deviation. This behavior can be changed via context options ::LY_CTX_ALL_IMPLEMENTED, when
 * all the modules in the context are marked as implemented (note the problem with multiple revisions of a single module),
 * or by ::LY_CTX_REF_IMPLEMENTED option, extending the set of references making the module implemented by when, must and
 * default statements.
 *
 * All modules with deviation definition are always marked as implemented. The imported (not implemented) module can be set implemented by ::lys_set_implemented(). But
 * the implemented module cannot be changed back to just imported module. Note also that only one revision of a specific module
 * can be implemented in a single context. The imported modules are used only as a
 * source of definitions for types and groupings for uses statements. The data in such modules are ignored - caller
 * is not allowed to create the data (including instantiating identities) defined in the model via data parsers,
 * the default nodes are not added into any data tree and mandatory nodes are not checked in the data trees.
 *
 * The compiled schema tree nodes are able to hold private objects (::lysc_node.priv as a pointer to a structure, function, variable, ...) used by
 * a caller application. Such an object can be assigned to a specific node using ::lysc_set_private() function.
 * Note that the object is not freed by libyang when the context is being destroyed. So the caller is responsible
 * for freeing the provided structure after the context is destroyed or the private pointer is set to NULL in
 * appropriate schema nodes where the object was previously set. This can be automated via destructor function
 * to free these private objects. The destructor is passed to the ::ly_ctx_destroy() function.
 *
 * Despite all the schema structures and their members are available as part of the libyang API and callers can use
 * it to navigate through the schema tree structure or to obtain various information, we recommend to use the following
 * macros for the specific actions.
 * - ::LYSC_TREE_DFS_BEGIN and ::LYSC_TREE_DFS_END to traverse the schema tree (depth-first).
 * - ::LY_LIST_FOR and ::LY_ARRAY_FOR as described on @ref howtoStructures page.
 *
 * Further information about modules handling can be found on the following pages:
 * - @subpage howtoSchemaParsers
 * - @subpage howtoSchemaFeatures
 * - @subpage howtoPlugins
 * - @subpage howtoSchemaPrinters
 *
 * \note There are many functions to access information from the schema trees. Details are available in
 * the [Schema Tree module](@ref schematree).
 *
 * For information about difference between implemented and imported modules, see the
 * [context description](@ref howtoContext).
 *
 * Functions List (not assigned to above subsections)
 * --------------------------------------------------
 * - ::lys_getnext()
 * - ::lys_nodetype2str()
 * - ::lys_set_implemented()
 * - ::lys_value_validate()
 *
 * - ::lysc_set_private()
 *
 * - ::lysc_has_when()
 *
 * - ::lysc_node_children()
 * - ::lysc_node_children_full()
 * - ::lysc_node_parent_full()
 * - ::lysc_node_actions()
 * - ::lysc_node_notifs()
 *
 * - ::lysp_node_children()
 * - ::lysp_node_actions()
 * - ::lysp_node_notifs()
 * - ::lysp_node_groupings()
 * - ::lysp_node_typedefs()
 */

/**
 * @page howtoSchemaFeatures YANG Features
 *
 * YANG feature statement is an important part of the language which can significantly affect the meaning of the schemas.
 * Modifying features may have similar effects as loading/removing schema from the context so it is limited to context
 * preparation period before working with data. YANG features, respectively their use in if-feature
 * statements, are evaluated as part of schema compilation so a feature-specific compiled schema tree is generated
 * as a result.
 *
 * To enable any features, they must currently be specified when implementing a new schema with ::lys_parse() or
 * ::ly_ctx_load_module(). To later examine what the status of a feature is, check its ::LYS_FENABLED flag or
 * search for it first with ::lys_feature_value(). Lastly, to evaluate compiled if-features, use ::lysc_iffeature_value().
 *
 * To iterate over all features of a particular YANG module, use ::lysp_feature_next().
 *
 * Note, that the feature's state can affect some of the output formats (e.g. Tree format).
 *
 * Functions List
 * --------------
 * - ::lys_feature_value()
 * - ::lysc_iffeature_value()
 * - ::lysp_feature_next()
 */

/**
 * @ingroup trees
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 */

/* *INDENT-OFF* */

/**
 * @brief Macro to iterate via all elements in a schema (sub)tree including input and output.
 * Note that __actions__ and __notifications__ of traversed nodes __are ignored__! To traverse
 * on all the nodes including those, use ::lysc_tree_dfs_full() instead.
 *
 * This is the opening part to the #LYSC_TREE_DFS_END - they always have to be used together.
 *
 * The function follows deep-first search algorithm:
 * <pre>
 *     1
 *    / \
 *   2   4
 *  /   / \
 * 3   5   6
 * </pre>
 *
 * Use the same parameters for #LYSC_TREE_DFS_BEGIN and #LYSC_TREE_DFS_END. While
 * START can be any of the lysc_node* types (including lysc_action and lysc_notif),
 * ELEM variable must be of the struct lysc_node* type.
 *
 * To skip a particular subtree, instead of the continue statement, set LYSC_TREE_DFS_continue
 * variable to non-zero value.
 *
 * Use with opening curly bracket '{' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param ELEM Iterator intended for use in the block.
 */
#define LYSC_TREE_DFS_BEGIN(START, ELEM) \
    { ly_bool LYSC_TREE_DFS_continue = 0; struct lysc_node *LYSC_TREE_DFS_next; \
    for ((ELEM) = (LYSC_TREE_DFS_next) = (struct lysc_node*)(START); \
         (ELEM); \
         (ELEM) = (LYSC_TREE_DFS_next), LYSC_TREE_DFS_continue = 0)

/**
 * @brief Macro to iterate via all elements in a (sub)tree. This is the closing part
 * to the #LYSC_TREE_DFS_BEGIN - they always have to be used together.
 *
 * Use the same parameters for #LYSC_TREE_DFS_BEGIN and #LYSC_TREE_DFS_END. While
 * START can be a pointer to any of the lysc_node* types (including lysc_action and lysc_notif),
 * ELEM variable must be pointer to the lysc_node type.
 *
 * Use with closing curly bracket '}' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param ELEM Iterator intended for use in the block.
 */
#define LYSC_TREE_DFS_END(START, ELEM) \
    /* select element for the next run - children first */ \
    if (LYSC_TREE_DFS_continue) { \
        (LYSC_TREE_DFS_next) = NULL; \
    } else { \
        (LYSC_TREE_DFS_next) = (struct lysc_node *)lysc_node_children_full(ELEM, 0); \
    } \
    if (!(LYSC_TREE_DFS_next)) { \
        /* no children, try siblings */ \
        _LYSC_TREE_DFS_NEXT(START, ELEM, LYSC_TREE_DFS_next); \
    } \
    while (!(LYSC_TREE_DFS_next)) { \
        /* parent is already processed, go to its sibling */ \
        (ELEM) = (struct lysc_node *)lysc_node_parent_full(ELEM); \
        _LYSC_TREE_DFS_NEXT(START, ELEM, LYSC_TREE_DFS_next); \
    } }

/**
 * @brief Helper macro for #LYSC_TREE_DFS_END, should not be used directly!
 */
#define _LYSC_TREE_DFS_NEXT(START, ELEM, NEXT) \
    if ((ELEM) == (struct lysc_node *)(START)) { \
        /* we are done, no next element to process */ \
        break; \
    } \
    if ((ELEM)->nodetype == LYS_INPUT) { \
        /* after input, get output */ \
        (NEXT) = (struct lysc_node *)lysc_node_children_full(lysc_node_parent_full(ELEM), LYS_CONFIG_R); \
    } else if ((ELEM)->nodetype == LYS_OUTPUT) { \
        /* no sibling of output */ \
        (NEXT) = NULL; \
    } else { \
        (NEXT) = (ELEM)->next; \
    }

/* *INDENT-ON* */

#define LY_REV_SIZE 11   /**< revision data string length (including terminating NULL byte) */

#define LYS_UNKNOWN     0x0000    /**< uninitalized unknown statement node */
#define LYS_CONTAINER   0x0001    /**< container statement node */
#define LYS_CHOICE      0x0002    /**< choice statement node */
#define LYS_LEAF        0x0004    /**< leaf statement node */
#define LYS_LEAFLIST    0x0008    /**< leaf-list statement node */
#define LYS_LIST        0x0010    /**< list statement node */
#define LYS_ANYXML      0x0020    /**< anyxml statement node */
#define LYS_ANYDATA     0x0060    /**< anydata statement node, in tests it can be used for both #LYS_ANYXML and #LYS_ANYDATA */
#define LYS_CASE        0x0080    /**< case statement node */

#define LYS_RPC         0x0100    /**< RPC statement node */
#define LYS_ACTION      0x0200    /**< action statement node */
#define LYS_NOTIF       0x0400    /**< notification statement node */

#define LYS_USES        0x0800    /**< uses statement node */
#define LYS_INPUT       0x1000    /**< RPC/action input node */
#define LYS_OUTPUT      0x2000    /**< RPC/action output node */
#define LYS_GROUPING    0x4000
#define LYS_AUGMENT     0x8000

#define LYS_NODETYPE_MASK 0xffff  /**< Mask for nodetypes, the value is limited for 16 bits */

/**
 * @brief List of YANG statements
 */
enum ly_stmt {
    LY_STMT_NONE = 0,
    LY_STMT_STATUS,             /**< in ::lysc_ext_substmt.storage stored as a pointer to `uint16_t`, only cardinality < #LY_STMT_CARD_SOME is allowed */
    LY_STMT_CONFIG,             /**< in ::lysc_ext_substmt.storage stored as a pointer to `uint16_t`, only cardinality < #LY_STMT_CARD_SOME is allowed */
    LY_STMT_MANDATORY,
    LY_STMT_UNITS,              /**< in ::lysc_ext_substmt.storage stored as a pointer to `const char *` (cardinality < #LY_STMT_CARD_SOME)
                                     or as a pointer to a [sized array](@ref sizedarrays) `const char **` */
    LY_STMT_DEFAULT,
    LY_STMT_TYPE,               /**< in ::lysc_ext_substmt.storage stored as a pointer to `struct lysc_type *` (cardinality < #LY_STMT_CARD_SOME)
                                     or as a pointer to a [sized array](@ref sizedarrays) `struct lysc_type **` */

    LY_STMT_ACTION,
    LY_STMT_ANYDATA,
    LY_STMT_ANYXML,
    LY_STMT_ARGUMENT,
    LY_STMT_AUGMENT,
    LY_STMT_BASE,
    LY_STMT_BELONGS_TO,
    LY_STMT_BIT,
    LY_STMT_CASE,
    LY_STMT_CHOICE,
    LY_STMT_CONTACT,
    LY_STMT_CONTAINER,
    LY_STMT_DESCRIPTION,
    LY_STMT_DEVIATE,
    LY_STMT_DEVIATION,
    LY_STMT_ENUM,
    LY_STMT_ERROR_APP_TAG,
    LY_STMT_ERROR_MESSAGE,
    LY_STMT_EXTENSION,
    LY_STMT_FEATURE,
    LY_STMT_FRACTION_DIGITS,
    LY_STMT_GROUPING,
    LY_STMT_IDENTITY,
    LY_STMT_IF_FEATURE,         /**< in ::lysc_ext_substmt.storage stored as a pointer to `struct lysc_iffeature` (cardinality < #LY_STMT_CARD_SOME)
                                     or as a pointer to a [sized array](@ref sizedarrays) `struct lysc_iffeature *` */
    LY_STMT_IMPORT,
    LY_STMT_INCLUDE,
    LY_STMT_INPUT,
    LY_STMT_KEY,
    LY_STMT_LEAF,
    LY_STMT_LEAF_LIST,
    LY_STMT_LENGTH,
    LY_STMT_LIST,
    LY_STMT_MAX_ELEMENTS,
    LY_STMT_MIN_ELEMENTS,
    LY_STMT_MODIFIER,
    LY_STMT_MODULE,
    LY_STMT_MUST,
    LY_STMT_NAMESPACE,
    LY_STMT_NOTIFICATION,
    LY_STMT_ORDERED_BY,
    LY_STMT_ORGANIZATION,
    LY_STMT_OUTPUT,
    LY_STMT_PATH,
    LY_STMT_PATTERN,
    LY_STMT_POSITION,
    LY_STMT_PREFIX,
    LY_STMT_PRESENCE,
    LY_STMT_RANGE,
    LY_STMT_REFERENCE,
    LY_STMT_REFINE,
    LY_STMT_REQUIRE_INSTANCE,
    LY_STMT_REVISION,
    LY_STMT_REVISION_DATE,
    LY_STMT_RPC,
    LY_STMT_SUBMODULE,
    LY_STMT_TYPEDEF,
    LY_STMT_UNIQUE,
    LY_STMT_USES,
    LY_STMT_VALUE,
    LY_STMT_WHEN,
    LY_STMT_YANG_VERSION,
    LY_STMT_YIN_ELEMENT,
    LY_STMT_EXTENSION_INSTANCE,

    LY_STMT_SYNTAX_SEMICOLON,
    LY_STMT_SYNTAX_LEFT_BRACE,
    LY_STMT_SYNTAX_RIGHT_BRACE,

    LY_STMT_ARG_TEXT,
    LY_STMT_ARG_VALUE
};

/**
 * @brief Extension instance structure parent enumeration
 */
typedef enum {
    LYEXT_PAR_MODULE,    /**< ::lysc_module */
    LYEXT_PAR_NODE,      /**< ::lysc_node (and the derived structures including ::lysc_action and ::lysc_notif) */
    LYEXT_PAR_INPUT,     /**< ::lysc_action_inout */
    LYEXT_PAR_OUTPUT,    /**< ::lysc_action_inout */
    LYEXT_PAR_TYPE,      /**< ::lysc_type */
    LYEXT_PAR_TYPE_BIT,  /**< ::lysc_type_bitenum_item */
    LYEXT_PAR_TYPE_ENUM, /**< ::lysc_type_bitenum_item */
    LYEXT_PAR_MUST,      /**< ::lysc_must */
    LYEXT_PAR_PATTERN,   /**< ::lysc_pattern */
    LYEXT_PAR_LENGTH,    /**< ::lysc_range */
    LYEXT_PAR_RANGE,     /**< ::lysc_range */
    LYEXT_PAR_WHEN,      /**< ::lysc_when */
    LYEXT_PAR_IDENT,     /**< ::lysc_ident */
    LYEXT_PAR_EXT,       /**< ::lysc_ext */
    LYEXT_PAR_IMPORT     /**< ::lysp_import */
#if 0
    LYEXT_PAR_TPDF,      /**< ::lysp_tpdf */
    LYEXT_PAR_EXTINST,   /**< ::lysp_ext_instance */
    LYEXT_PAR_REFINE,    /**< ::lysp_refine */
    LYEXT_PAR_DEVIATION, /**< ::lysp_deviation */
    LYEXT_PAR_DEVIATE,   /**< ::lysp_deviate */
    LYEXT_PAR_INCLUDE,   /**< ::lysp_include */
    LYEXT_PAR_REVISION   /**< ::lysp_revision */
#endif
} LYEXT_PARENT;

/**
 * @brief Stringify extension instance parent type.
 * @param[in] type Parent type to stringify.
 * @return Constant string with the name of the parent statement.
 */
const char *lyext_parent2str(LYEXT_PARENT type);

/**
 * @brief Enum of substatements in which extension instances can appear.
 */
typedef enum {
    LYEXT_SUBSTMT_SELF = 0,      /**< extension of the structure itself, not substatement's */
    LYEXT_SUBSTMT_ARGUMENT,      /**< extension of the argument statement, can appear in lys_ext */
    LYEXT_SUBSTMT_BASE,          /**< extension of the base statement, can appear (repeatedly) in lys_type and lys_ident */
    LYEXT_SUBSTMT_BELONGSTO,     /**< extension of the belongs-to statement, can appear in lys_submodule */
    LYEXT_SUBSTMT_CONTACT,       /**< extension of the contact statement, can appear in lys_module */
    LYEXT_SUBSTMT_DEFAULT,       /**< extension of the default statement, can appear in lys_node_leaf, lys_node_leaflist,
                                      lys_node_choice and lys_deviate */
    LYEXT_SUBSTMT_DESCRIPTION,   /**< extension of the description statement, can appear in lys_module, lys_submodule,
                                      lys_node, lys_import, lys_include, lys_ext, lys_feature, lys_tpdf, lys_restr,
                                      lys_ident, lys_deviation, lys_type_enum, lys_type_bit, lys_when and lys_revision */
    LYEXT_SUBSTMT_ERRTAG,        /**< extension of the error-app-tag statement, can appear in lys_restr */
    LYEXT_SUBSTMT_ERRMSG,        /**< extension of the error-message statement, can appear in lys_restr */
    LYEXT_SUBSTMT_KEY,           /**< extension of the key statement, can appear in lys_node_list */
    LYEXT_SUBSTMT_NAMESPACE,     /**< extension of the namespace statement, can appear in lys_module */
    LYEXT_SUBSTMT_ORGANIZATION,  /**< extension of the organization statement, can appear in lys_module and lys_submodule */
    LYEXT_SUBSTMT_PATH,          /**< extension of the path statement, can appear in lys_type */
    LYEXT_SUBSTMT_PREFIX,        /**< extension of the prefix statement, can appear in lys_module, lys_submodule (for
                                      belongs-to's prefix) and lys_import */
    LYEXT_SUBSTMT_PRESENCE,      /**< extension of the presence statement, can appear in lys_node_container */
    LYEXT_SUBSTMT_REFERENCE,     /**< extension of the reference statement, can appear in lys_module, lys_submodule,
                                      lys_node, lys_import, lys_include, lys_revision, lys_tpdf, lys_restr, lys_ident,
                                      lys_ext, lys_feature, lys_deviation, lys_type_enum, lys_type_bit and lys_when */
    LYEXT_SUBSTMT_REVISIONDATE,  /**< extension of the revision-date statement, can appear in lys_import and lys_include */
    LYEXT_SUBSTMT_UNITS,         /**< extension of the units statement, can appear in lys_tpdf, lys_node_leaf,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_VALUE,         /**< extension of the value statement, can appear in lys_type_enum */
    LYEXT_SUBSTMT_VERSION,       /**< extension of the yang-version statement, can appear in lys_module and lys_submodule */
    LYEXT_SUBSTMT_MODIFIER,      /**< extension of the modifier statement, can appear in lys_restr */
    LYEXT_SUBSTMT_REQINSTANCE,   /**< extension of the require-instance statement, can appear in lys_type */
    LYEXT_SUBSTMT_YINELEM,       /**< extension of the yin-element statement, can appear in lys_ext */
    LYEXT_SUBSTMT_CONFIG,        /**< extension of the config statement, can appear in lys_node and lys_deviate */
    LYEXT_SUBSTMT_MANDATORY,     /**< extension of the mandatory statement, can appear in lys_node_leaf, lys_node_choice,
                                      lys_node_anydata and lys_deviate */
    LYEXT_SUBSTMT_ORDEREDBY,     /**< extension of the ordered-by statement, can appear in lys_node_list and lys_node_leaflist */
    LYEXT_SUBSTMT_STATUS,        /**< extension of the status statement, can appear in lys_tpdf, lys_node, lys_ident,
                                      lys_ext, lys_feature, lys_type_enum and lys_type_bit */
    LYEXT_SUBSTMT_FRACDIGITS,    /**< extension of the fraction-digits statement, can appear in lys_type */
    LYEXT_SUBSTMT_MAX,           /**< extension of the max-elements statement, can appear in lys_node_list,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_MIN,           /**< extension of the min-elements statement, can appear in lys_node_list,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_POSITION,      /**< extension of the position statement, can appear in lys_type_bit */
    LYEXT_SUBSTMT_UNIQUE,        /**< extension of the unique statement, can appear in lys_node_list and lys_deviate */
    LYEXT_SUBSTMT_IFFEATURE      /**< extension of the if-feature statement */
} LYEXT_SUBSTMT;

/**
 * @brief YANG import-stmt
 */
struct lysp_import {
    struct lys_module *module;       /**< pointer to the imported module
                                          (mandatory, but resolved when the referring module is completely parsed) */
    const char *name;                /**< name of the imported module (mandatory) */
    const char *prefix;              /**< prefix for the data from the imported schema (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< LYS_INTERNAL value (@ref snodeflags) */
    char rev[LY_REV_SIZE];           /**< revision-date of the imported module */
};

/**
 * @brief YANG include-stmt
 */
struct lysp_include {
    struct lysp_submodule *submodule;/**< pointer to the parsed submodule structure
                                         (mandatory, but resolved when the referring module is completely parsed) */
    const char *name;                /**< name of the included submodule (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    char rev[LY_REV_SIZE];           /**< revision-date of the included submodule */
};

/**
 * @brief YANG extension-stmt
 */
struct lysp_ext {
    const char *name;                /**< extension name */
    const char *argument;            /**< argument name, NULL if not specified */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< LYS_STATUS_* and LYS_YINELEM_* values (@ref snodeflags) */

    struct lysc_ext *compiled;       /**< pointer to the compiled extension definition */
};

/**
 * @brief Helper structure for generic storage of the extension instances content.
 */
struct lysp_stmt {
    const char *stmt;                /**< identifier of the statement */
    const char *arg;                 /**< statement's argument */
    struct lysp_stmt *next;          /**< link to the next statement */
    struct lysp_stmt *child;         /**< list of the statement's substatements (linked list) */
    uint16_t flags;                  /**< statement flags, can be set to LYS_YIN_ATTR */
    enum ly_stmt kw;                 /**< numeric respresentation of the stmt value */
};

#define LYS_YIN 0x1 /**< used to specify input format of extension instance */

/**
 * @brief YANG extension instance
 */
struct lysp_ext_instance {
    const char *name;                       /**< extension identifier, including possible prefix */
    const char *argument;                   /**< optional value of the extension's argument */
    void *parent;                           /**< pointer to the parent element holding the extension instance(s), use
                                                 ::lysp_ext_instance#parent_type to access the schema element */
    struct lysp_stmt *child;                /**< list of the extension's substatements (linked list) */
    struct lysc_ext_instance *compiled;     /**< pointer to the compiled data if any - in case the source format is YIN,
                                                 some of the information (argument) are available only after compilation */
    LYEXT_SUBSTMT insubstmt;                /**< value identifying placement of the extension instance */
    LY_ARRAY_COUNT_TYPE insubstmt_index;    /**< in case the instance is in a substatement, this identifies
                                                 the index of that substatement */
    uint16_t flags;                         /**< LYS_INTERNAL value (@ref snodeflags) */
    uint8_t yin;                            /**< flag for YIN source format, can be set to LYS_YIN */
    LYEXT_PARENT parent_type;               /**< type of the parent structure */
};

/**
 * @brief YANG feature-stmt
 */
struct lysp_feature {
    const char *name;                /**< feature name (mandatory) */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures_c;    /**< compiled if-features */
    struct lysp_feature **depfeatures;  /**< list of pointers to other features depending on this one
                                          ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement  */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values and
                                          LYS_FENABLED are allowed */
};

/**
 * @brief Compiled YANG if-feature-stmt
 */
struct lysc_iffeature {
    uint8_t *expr;                   /**< 2bits array describing the if-feature expression in prefix format, see @ref ifftokens */
    struct lysp_feature **features;  /**< array of pointers to the features used in expression ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Qualified name (optional prefix followed by an identifier).
 */
struct lysp_qname {
    const char *str;                 /**< qualified name string */
    const struct lysp_module *mod;   /**< module to resolve any prefixes found in the string, it must be
                                          stored explicitly because of deviations/refines */
};

/**
 * @brief YANG identity-stmt
 */
struct lysp_ident {
    const char *name;                /**< identity name (mandatory), including possible prefix */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    const char **bases;              /**< list of base identifiers ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/**
 * @brief Covers restrictions: range, length, pattern, must
 */
struct lysp_restr {
#define LYSP_RESTR_PATTERN_ACK   0x06
#define LYSP_RESTR_PATTERN_NACK  0x15
    struct lysp_qname arg;           /**< The restriction expression/value (mandatory);
                                          in case of pattern restriction, the first byte has a special meaning:
                                          0x06 (ACK) for regular match and 0x15 (NACK) for invert-match */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG revision-stmt
 */
struct lysp_revision {
    char date[LY_REV_SIZE];          /**< revision date (madatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Enumeration/Bit value definition
 */
struct lysp_type_enum {
    const char *name;                /**< name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    int64_t value;                   /**< enum's value or bit's position */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ and LYS_SET_VALUE
                                          values are allowed */
};

/**
 * @brief YANG type-stmt
 *
 * Some of the items in the structure may be mandatory, but it is necessary to resolve the type's base type first
 */
struct lysp_type {
    const char *name;                /**< name of the type (mandatory) */
    struct lysp_restr *range;        /**< allowed values range - numerical, decimal64 */
    struct lysp_restr *length;       /**< allowed length of the value - string, binary */
    struct lysp_restr *patterns;     /**< list of patterns ([sized array](@ref sizedarrays)) - string */
    struct lysp_type_enum *enums;    /**< list of enum-stmts ([sized array](@ref sizedarrays)) - enum */
    struct lysp_type_enum *bits;     /**< list of bit-stmts ([sized array](@ref sizedarrays)) - bits */
    struct lyxp_expr *path;          /**< parsed path - leafref */
    const char **bases;              /**< list of base identifiers ([sized array](@ref sizedarrays)) - identityref */
    struct lysp_type *types;         /**< list of sub-types ([sized array](@ref sizedarrays)) - union */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    const struct lysp_module *pmod;  /**< (sub)module where the type is defined (needed for deviations) */
    struct lysc_type *compiled;      /**< pointer to the compiled type */

    uint8_t fraction_digits;         /**< number of fraction digits - decimal64 */
    uint8_t require_instance;        /**< require-instance flag - leafref, instance */
    uint16_t flags;                  /**< [schema node flags](@ref spnodeflags) */
};

/**
 * @brief YANG typedef-stmt
 */
struct lysp_tpdf {
    const char *name;                /**< name of the newly defined type (mandatory) */
    const char *units;               /**< units of the newly defined type */
    struct lysp_qname dflt;          /**< default value of the newly defined type, it may or may not be a qualified name */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysp_type type;           /**< base type from which the typedef is derived (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref spnodeflags) */
};

/**
 * @brief YANG grouping-stmt
 */
struct lysp_grp {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level grouping) */
    uint16_t nodetype;               /**< LYS_GROUPING */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
    const char *name;                /**< grouping name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG when-stmt
 */
struct lysp_when {
    const char *cond;                /**< specified condition (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG refine-stmt
 */
struct lysp_refine {
    const char *nodeid;              /**< target descendant schema nodeid (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    const char *presence;            /**< presence description */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
};

/**
 * @brief YANG uses-augment-stmt and augment-stmt (compatible with struct lysp_node )
 */
struct lysp_augment {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level augment) */
    uint16_t nodetype;               /**< LYS_AUGMENT */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    const char *nodeid;              /**< target schema nodeid (mandatory) - absolute for global augments, descendant for uses's augments */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
};

/**
 * @ingroup schematree
 * @defgroup deviatetypes Deviate types
 *
 * Type of the deviate operation (used as ::lysp_deviate.mod)
 *
 * @{
 */
#define LYS_DEV_NOT_SUPPORTED 1      /**< deviate type not-supported */
#define LYS_DEV_ADD 2                /**< deviate type add */
#define LYS_DEV_DELETE 3             /**< deviate type delete */
#define LYS_DEV_REPLACE 4            /**< deviate type replace */
/** @} deviatetypes */

/**
 * @brief Generic deviate structure to get type and cast to lysp_deviate_* structure
 */
struct lysp_deviate {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysp_deviate_add {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *units;               /**< units of the values */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_qname *uniques;      /**< list of uniques specifications ([sized array](@ref sizedarrays)) */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_deviate_del {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *units;               /**< units of the values */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_qname *uniques;      /**< list of uniques specifications ([sized array](@ref sizedarrays)) */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)) */
};

struct lysp_deviate_rpl {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysp_type *type;          /**< type of the node */
    const char *units;               /**< units of the values */
    struct lysp_qname dflt;          /**< default value */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_deviation {
    const char *nodeid;              /**< target absolute schema nodeid (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_deviate *deviates;   /**< list of deviate specifications (linked list) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @ingroup snodeflags
 * @defgroup spnodeflags Parsed schema nodes flags
 *
 * Various flags for parsed schema nodes (used as ::lysp_node.flags).
 *
 *     1 - container    6 - anydata/anyxml    11 - output       16 - grouping   21 - enum
 *     2 - choice       7 - case              12 - feature      17 - uses       22 - type
 *     3 - leaf         8 - notification      13 - identity     18 - refine     23 - stmt
 *     4 - leaflist     9 - rpc               14 - extension    19 - augment
 *     5 - list        10 - input             15 - typedef      20 - deviate
 *
 *                                             1 1 1 1 1 1 1 1 1 1 2 2 2 2
 *     bit name              1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       1 LYS_CONFIG_W     |x|x|x|x|x|x| | | | | | | | | | | |x| |x| | | |
 *         LYS_SET_BASE     | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       2 LYS_CONFIG_R     |x|x|x|x|x|x| | | | | | | | | | | |x| |x| | | |
 *         LYS_SET_BIT      | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       3 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| | |
 *         LYS_SET_ENUM     | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       4 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| | |
 *         LYS_SET_FRDIGITS | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       5 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| | |
 *         LYS_SET_LENGTH   | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       6 LYS_MAND_TRUE    | |x|x| | |x| | | | | | | | | | | |x| |x| | | |
 *         LYS_SET_PATH     | | | | | | | | | | | | | | | | | | | | | |x| |
 *         LYS_FENABLED     | | | | | | | | | | | |x| | | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       7 LYS_MAND_FALSE   | |x|x| | |x| | | | | | | | | | | |x| |x| | | |
 *         LYS_ORDBY_USER   | | | |x|x| | | | | | | | | | | | | | | | | | |
 *         LYS_SET_PATTERN  | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       8 LYS_ORDBY_SYSTEM | | | |x|x| | | | | | | | | | | | | | | | | | |
 *         LYS_YINELEM_TRUE | | | | | | | | | | | | | |x| | | | | | | | | |
 *         LYS_SET_RANGE    | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       9 LYS_YINELEM_FALSE| | | | | | | | | | | | | |x| | | | | | | | | |
 *         LYS_SET_TYPE     | | | | | | | | | | | | | | | | | | | | | |x| |
 *         LYS_SINGLEQUOTED | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      10 LYS_SET_VALUE    | | | | | | | | | | | | | | | | | | | | |x| | |
 *         LYS_SET_REQINST  | | | | | | | | | | | | | | | | | | | | | |x| |
 *         LYS_SET_MIN      | | | |x|x| | | | | | | | | | | | |x| |x| | | |
 *         LYS_DOUBLEQUOTED | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      11 LYS_SET_MAX      | | | |x|x| | | | | | | | | | | | |x| |x| | | |
 *         LYS_USED_GRP     | | | | | | | | | | | | | | | |x| | | | | | | |
 *         LYS_YIN_ATTR     | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      12 LYS_YIN_ARGUMENT | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      13 LYS_INTERNAL     |x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/**
 * @ingroup snodeflags
 * @defgroup scnodeflags Compiled schema nodes flags
 *
 * Various flags for compiled schema nodes (used as ::lysc_node.flags).
 *
 *     1 - container    6 - anydata/anyxml    11 - identity
 *     2 - choice       7 - case              12 - extension
 *     3 - leaf         8 - notification      13 - bitenum
 *     4 - leaflist     9 - rpc/action        14 - when
 *     5 - list        10 - feature
 *
 *                                             1 1 1 1 1
 *     bit name              1 2 3 4 5 6 7 8 9 0 1 2 3 4
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       1 LYS_CONFIG_W     |x|x|x|x|x|x|x| | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       2 LYS_CONFIG_R     |x|x|x|x|x|x|x| | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       3 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x|x|x|x| |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       4 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x|x|x|x| |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       5 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x|x|x|x| |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       6 LYS_MAND_TRUE    |x|x|x|x|x|x| | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       7 LYS_ORDBY_USER   | | | |x|x| | | | | | | | | |
 *         LYS_MAND_FALSE   | |x|x| | |x| | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       8 LYS_ORDBY_SYSTEM | | | |x|x| | | | | | | | | |
 *         LYS_PRESENCE     |x| | | | | | | | | | | | | |
 *         LYS_UNIQUE       | | |x| | | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       9 LYS_KEY          | | |x| | | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      10 LYS_SET_DFLT     | | |x|x| | |x| | | | | | | |
 *         LYS_ISENUM       | | | | | | | | | | | | |x| |
 *         LYS_KEYLESS      | | | | |x| | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      11 LYS_SET_UNITS    | | |x|x| | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      12 LYS_SET_CONFIG   |x|x|x|x|x|x| | | | | | | | |
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/**
 * @defgroup snodeflags Schema nodes flags
 *
 * Various flags for schema nodes ([parsed](@ref spnodeflags) as well as [compiled](@ref scnodeflags)).
 *
 * @{
 */
#define LYS_CONFIG_W     0x01        /**< config true; also set for input children nodes */
#define LYS_CONFIG_R     0x02        /**< config false; also set for output and notification children nodes */
#define LYS_CONFIG_MASK  0x03        /**< mask for config value */
#define LYS_STATUS_CURR  0x04        /**< status current; */
#define LYS_STATUS_DEPRC 0x08        /**< status deprecated; */
#define LYS_STATUS_OBSLT 0x10        /**< status obsolete; */
#define LYS_STATUS_MASK  0x1C        /**< mask for status value */
#define LYS_MAND_TRUE    0x20        /**< mandatory true; applicable only to ::lysp_node_choice/::lysc_node_choice,
                                          ::lysp_node_leaf/::lysc_node_leaf and ::lysp_node_anydata/::lysc_node_anydata.
                                          The ::lysc_node_leaflist and ::lysc_node_leaflist have this flag in case that min-elements > 0.
                                          The ::lysc_node_container has this flag if it is not a presence container and it has at least one
                                          child with LYS_MAND_TRUE. */
#define LYS_MAND_FALSE   0x40        /**< mandatory false; applicable only to ::lysp_node_choice/::lysc_node_choice,
                                          ::lysp_node_leaf/::lysc_node_leaf and ::lysp_node_anydata/::lysc_node_anydata.
                                          This flag is present only in case the mandatory false statement was explicitly specified. */
#define LYS_MAND_MASK    0x60        /**< mask for mandatory values */
#define LYS_PRESENCE     0x80        /**< flag for presence property of a container, but it is not only for explicit presence
                                          containers, but also for NP containers with some meaning, applicable only to
                                          ::lysc_node_container */
#define LYS_UNIQUE       0x80        /**< flag for leafs being part of a unique set, applicable only to ::lysc_node_leaf */
#define LYS_KEY          0x100       /**< flag for leafs being a key of a list, applicable only to ::lysc_node_leaf */
#define LYS_KEYLESS      0x200       /**< flag for list without any key, applicable only to ::lysc_node_list */
#define LYS_FENABLED     0x20        /**< feature enabled flag, applicable only to ::lysp_feature. */
#define LYS_ORDBY_SYSTEM 0x80        /**< ordered-by user lists, applicable only to ::lysc_node_leaflist/::lysp_node_leaflist and
                                          ::lysc_node_list/::lysp_node_list */
#define LYS_ORDBY_USER   0x40        /**< ordered-by user lists, applicable only to ::lysc_node_leaflist/::lysp_node_leaflist and
                                          ::lysc_node_list/::lysp_node_list */
#define LYS_ORDBY_MASK   0x60        /**< mask for ordered-by values */
#define LYS_YINELEM_TRUE 0x80        /**< yin-element true for extension's argument */
#define LYS_YINELEM_FALSE 0x100      /**< yin-element false for extension's argument */
#define LYS_YINELEM_MASK 0x180       /**< mask for yin-element value */
#define LYS_USED_GRP     0x400       /**< internal flag for validating not-instantiated groupings
                                          (resp. do not validate again the instantiated groupings). */
#define LYS_SET_VALUE    0x200       /**< value attribute is set */
#define LYS_SET_MIN      0x200       /**< min attribute is set */
#define LYS_SET_MAX      0x400       /**< max attribute is set */

#define LYS_SET_BASE     0x0001      /**< type's flag for present base substatement */
#define LYS_SET_BIT      0x0002      /**< type's flag for present bit substatement */
#define LYS_SET_ENUM     0x0004      /**< type's flag for present enum substatement */
#define LYS_SET_FRDIGITS 0x0008      /**< type's flag for present fraction-digits substatement */
#define LYS_SET_LENGTH   0x0010      /**< type's flag for present length substatement */
#define LYS_SET_PATH     0x0020      /**< type's flag for present path substatement */
#define LYS_SET_PATTERN  0x0040      /**< type's flag for present pattern substatement */
#define LYS_SET_RANGE    0x0080      /**< type's flag for present range substatement */
#define LYS_SET_TYPE     0x0100      /**< type's flag for present type substatement */
#define LYS_SET_REQINST  0x0200      /**< type's flag for present require-instance substatement */
#define LYS_SET_DFLT     0x0200      /**< flag to mark leaf/leaflist with own (or refined) default value, not a default value taken from its type, and default
                                          cases of choice. This information is important for refines, since it is prohibited to make leafs
                                          with default statement mandatory. In case the default leaf value is taken from type, it is thrown
                                          away when it is refined to be mandatory node. Similarly it is used for deviations to distinguish
                                          between own default or the default values taken from the type. */
#define LYS_SET_UNITS    0x0400      /**< flag to know if the leaf's/leaflist's units are their own (flag set) or it is taken from the type. */
#define LYS_SET_CONFIG   0x0800      /**< flag to know if the config property was set explicitly (flag set) or it is inherited. */

#define LYS_SINGLEQUOTED 0x100       /**< flag for single-quoted argument of an extension instance's substatement, only when the source is YANG */
#define LYS_DOUBLEQUOTED 0x200       /**< flag for double-quoted argument of an extension instance's substatement, only when the source is YANG */

#define LYS_YIN_ATTR     0x400       /**< flag to identify YIN attribute parsed as extension's substatement, only when the source is YIN */
#define LYS_YIN_ARGUMENT 0x800       /**< flag to identify statement representing extension's argument, only when the source is YIN */

#define LYS_INTERNAL     0x1000      /**< flag to identify internal parsed statements that should not be printed */

#define LYS_ISENUM       0x200       /**< flag to simply distinguish type in struct lysc_type_bitenum_item */

#define LYS_FLAGS_COMPILED_MASK 0xff /**< mask for flags that maps to the compiled structures */
/** @} snodeflags */

/**
 * @brief Generic YANG data node
 */
struct lysp_node {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< type of the node (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)),
                                          must be qname because of refines */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Extension structure of the lysp_node for YANG container
 */
struct lysp_node_container {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_CONTAINER */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* container */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    const char *presence;            /**< presence description */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
};

struct lysp_node_leaf {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_LEAF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* leaf */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    struct lysp_qname dflt;          /**< default value, it may or may not be a qualified name */
};

struct lysp_node_leaflist {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_LEAFLIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* leaf-list */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)), they may or
                                          may not be qualified names */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_node_list {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_LIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* list */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    const char *key;                 /**< keys specification */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysp_qname *uniques;      /**< list of unique specifications ([sized array](@ref sizedarrays)) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_node_choice {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_CHOICE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* choice */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_qname dflt;          /**< default case */
};

struct lysp_node_case {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_CASE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* case */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
};

struct lysp_node_anydata {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_ANYXML or LYS_ANYDATA */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* anyxml/anydata */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
};

struct lysp_node_uses {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_USES */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* uses */
    struct lysp_refine *refines;     /**< list of uses's refines ([sized array](@ref sizedarrays)) */
    struct lysp_augment *augments;   /**< list of uses's augment ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG input-stmt and output-stmt
 */
struct lysp_action_inout {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_INPUT or LYS_OUTPUT */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG rpc-stmt and action-stmt
 */
struct lysp_action {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_RPC or LYS_ACTION */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* action */
    struct lysp_action_inout input;  /**< RPC's/Action's input */
    struct lysp_action_inout output; /**< RPC's/Action's output */
};

/**
 * @brief YANG notification-stmt
 */
struct lysp_notif {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_NOTIF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* notif */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
};

/**
 * @brief supported YANG schema version values
 */
typedef enum LYS_VERSION {
    LYS_VERSION_UNDEF = 0,  /**< no specific version, YANG 1.0 as default */
    LYS_VERSION_1_0 = 1,    /**< YANG 1 (1.0) */
    LYS_VERSION_1_1 = 2     /**< YANG 1.1 */
} LYS_VERSION;

/**
 * @brief Printable YANG schema tree structure representing YANG module.
 *
 * Simple structure corresponding to the YANG format. The schema is only syntactically validated.
 */
struct lysp_module {
    struct lys_module *mod;          /**< covering module structure */

    struct lysp_revision *revs;      /**< list of the module revisions ([sized array](@ref sizedarrays)), the first revision
                                          in the list is always the last (newest) revision of the module */
    struct lysp_import *imports;     /**< list of imported modules ([sized array](@ref sizedarrays)) */
    struct lysp_include *includes;   /**< list of included submodules ([sized array](@ref sizedarrays)) */
    struct lysp_ext *extensions;     /**< list of extension statements ([sized array](@ref sizedarrays)) */
    struct lysp_feature *features;   /**< list of feature definitions ([sized array](@ref sizedarrays)) */
    struct lysp_ident *identities;   /**< list of identities ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysp_augment *augments;   /**< list of augments ([sized array](@ref sizedarrays)) */
    struct lysp_action *rpcs;        /**< list of RPCs ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysp_deviation *deviations; /**< list of deviations ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
    uint8_t parsing : 1;             /**< flag for circular check */
    uint8_t is_submod : 1;           /**< always 0 */
};

struct lysp_submodule {
    struct lys_module *mod;          /**< belongs to parent module (submodule - mandatory) */

    struct lysp_revision *revs;      /**< list of the module revisions ([sized array](@ref sizedarrays)), the first revision
                                          in the list is always the last (newest) revision of the module */
    struct lysp_import *imports;     /**< list of imported modules ([sized array](@ref sizedarrays)) */
    struct lysp_include *includes;   /**< list of included submodules ([sized array](@ref sizedarrays)) */
    struct lysp_ext *extensions;     /**< list of extension statements ([sized array](@ref sizedarrays)) */
    struct lysp_feature *features;   /**< list of feature definitions ([sized array](@ref sizedarrays)) */
    struct lysp_ident *identities;   /**< list of identities ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysp_augment *augments;   /**< list of augments ([sized array](@ref sizedarrays)) */
    struct lysp_action *rpcs;        /**< list of RPCs ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysp_deviation *deviations; /**< list of deviations ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
    uint8_t parsing : 1;             /**< flag for circular check */
    uint8_t is_submod : 1;           /**< always 1 */

    uint8_t latest_revision : 2;     /**< flag to mark the latest available revision:
                                          1 - the latest revision in searchdirs was not searched yet and this is the
                                          latest revision in the current context
                                          2 - searchdirs were searched and this is the latest available revision */
    const char *name;                /**< name of the module (mandatory) */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    const char *prefix;              /**< submodule belongsto prefix of main module (mandatory) */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
};

/**
 * @brief Get the parsed module or submodule name.
 *
 * @param[in] PMOD Parsed module or submodule.
 * @return Module or submodule name.
 */
#define LYSP_MODULE_NAME(PMOD) (PMOD->is_submod ? ((struct lysp_submodule *)PMOD)->name : ((struct lysp_module *)PMOD)->mod->name)

/**
 * @brief Compiled prefix data pair mapping of prefixes to modules. In case the format is ::LY_PREF_SCHEMA_RESOLVED,
 * the expected prefix data is a sized array of these structures.
 */
struct lysc_prefix {
    char *prefix;                   /**< used prefix */
    const struct lys_module *mod;   /**< mapping to a module */
};

/**
 * @brief Compiled YANG extension-stmt
 */
struct lysc_ext {
    const char *name;                /**< extension name */
    const char *argument;            /**< argument name, NULL if not specified */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyext_plugin *plugin;     /**< Plugin implementing the specific extension */
    struct lys_module *module;       /**< module structure */
    uint32_t refcount;               /**< reference counter since extension definition is shared among all its instances */
    uint16_t flags;                  /**< LYS_STATUS_* value (@ref snodeflags) */
};

/**
 * @brief YANG extension instance
 */
struct lysc_ext_instance {
    uint32_t insubstmt_index;        /**< in case the instance is in a substatement that can appear multiple times,
                                          this identifies the index of the substatement for this extension instance */
    struct lys_module *module;       /**< module where the extension instantiated is defined */
    struct lysc_ext *def;            /**< pointer to the extension definition */
    void *parent;                    /**< pointer to the parent element holding the extension instance(s), use
                                          ::lysc_ext_instance#parent_type to access the schema element */
    const char *argument;            /**< optional value of the extension's argument */
    LYEXT_SUBSTMT insubstmt;         /**< value identifying placement of the extension instance */
    LYEXT_PARENT parent_type;        /**< type of the parent structure */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    void *data;                      /**< private plugins's data, not used by libyang */
};

/**
 * @brief YANG when-stmt
 */
struct lysc_when {
    struct lyxp_expr *cond;          /**< XPath when condition */
    struct lysc_node *context;       /**< context node for evaluating the expression, NULL if the context is root node */
    struct lysc_prefix *prefixes;    /**< compiled used prefixes in the condition */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint32_t refcount;               /**< reference counter since some of the when statements are shared among several nodes */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS is allowed */
};

/**
 * @brief YANG identity-stmt
 */
struct lysc_ident {
    const char *name;                /**< identity name (mandatory), including possible prefix */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lys_module *module;       /**< module structure */
    struct lysc_ident **derived;     /**< list of (pointers to the) derived identities ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/**
 * @defgroup ifftokens if-feature expression tokens
 * Tokens of if-feature expression used in ::lysc_iffeature.expr.
 *
 * @{
 */
#define LYS_IFF_NOT  0x00 /**< operand "not" */
#define LYS_IFF_AND  0x01 /**< operand "and" */
#define LYS_IFF_OR   0x02 /**< operand "or" */
#define LYS_IFF_F    0x03 /**< feature */
/** @} ifftokens */

/**
 * @brief Compiled YANG revision statement
 */
struct lysc_revision {
    char date[LY_REV_SIZE];          /**< revision-date (mandatory) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_range {
    struct lysc_range_part {
        union {                      /**< min boundary */
            int64_t min_64;          /**< for int8, int16, int32, int64 and decimal64 ( >= LY_TYPE_DEC64) */
            uint64_t min_u64;        /**< for uint8, uint16, uint32, uint64, string and binary ( < LY_TYPE_DEC64) */
        };
        union {                      /**< max boundary */
            int64_t max_64;          /**< for int8, int16, int32, int64 and decimal64 ( >= LY_TYPE_DEC64) */
            uint64_t max_u64;        /**< for uint8, uint16, uint32, uint64, string and binary ( < LY_TYPE_DEC64) */
        };
    } *parts;                        /**< compiled range expression ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_pattern {
    const char *expr;                /**< original, not compiled, regular expression */
    pcre2_code *code;                /**< compiled regular expression */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint32_t inverted : 1;             /**< invert-match flag */
    uint32_t refcount : 31;            /**< reference counter */
};

struct lysc_must {
    struct lyxp_expr *cond;          /**< XPath when condition */
    struct lysc_prefix *prefixes;    /**< compiled used prefixes in the condition */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_type {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
};

struct lysc_type_num {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_dec {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    uint8_t fraction_digits;         /**< fraction digits specification */
    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_str {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *length;       /**< Optional length limitation */
    struct lysc_pattern **patterns;  /**< Optional list of pointers to pattern limitations ([sized array](@ref sizedarrays)) */
};

struct lysc_type_bitenum_item {
    const char *name;            /**< enumeration identifier */
    const char *dsc;             /**< description */
    const char *ref;             /**< reference */
    struct lysc_ext_instance *exts;    /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    union {
        int32_t value;           /**< integer value associated with the enumeration */
        uint32_t position;       /**< non-negative integer value associated with the bit */
    };
    uint16_t flags;              /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ and LYS_SET_VALUE
                                          values are allowed */
};

struct lysc_type_enum {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type_bitenum_item *enums; /**< enumerations list ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_bits {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type_bitenum_item *bits; /**< bits list ([sized array](@ref sizedarrays)), mandatory (at least 1 item),
                                              the items are ordered by their position value. */
};

struct lysc_type_leafref {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lyxp_expr *path;          /**< parsed target path, compiled path cannot be stored because of type sharing */
    struct lysc_prefix *prefixes;    /**< resolved prefixes used in the path */
    const struct lys_module *cur_mod;/**< current module for the leafref (path) */
    struct lysc_type *realtype;      /**< pointer to the real (first non-leafref in possible leafrefs chain) type. */
    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_identityref {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_ident **bases;       /**< list of pointers to the base identities ([sized array](@ref sizedarrays)),
                                          mandatory (at least 1 item) */
};

struct lysc_type_instanceid {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_union {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type **types;        /**< list of types in the union ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_bin {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *length;       /**< Optional length limitation */
};

struct lysc_action_inout {
    uint16_t nodetype;               /**< LYS_INPUT or LYS_OUTPUT */
    struct lysc_node *data;          /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Maximum number of hashes stored in a schema node.
 */
#define LYS_NODE_HASH_COUNT 4

struct lysc_action {
    uint16_t nodetype;               /**< LYS_RPC or LYS_ACTION */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node - RPC) */

    struct lysc_ext_instance *input_exts;  /**< list of the extension instances of input ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *output_exts; /**< list of the extension instances of outpu ([sized array](@ref sizedarrays)) */

    const char *name;                /**< action/RPC name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    struct lysc_action_inout input;  /**< RPC's/action's input */
    struct lysc_action_inout output; /**< RPC's/action's output */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /** private arbitrary user data, not used by libyang */

};

struct lysc_notif {
    uint16_t nodetype;               /**< LYS_NOTIF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */

    struct lysc_node *data;          /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */

    const char *name;                /**< Notification name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /** private arbitrary user data, not used by libyang */
};

/**
 * @brief Compiled YANG data node
 */
struct lysc_node {
    uint16_t nodetype;               /**< type of the node (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */
};

struct lysc_node_container {
    uint16_t nodetype;               /**< LYS_CONTAINER */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysc_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
};

struct lysc_node_case {
    uint16_t nodetype;               /**< LYS_CASE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser, unused */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< name of the case, including the implicit case */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */

    struct lysc_node *child;         /**< first child node of the case (linked list). Note that all the children of all the sibling cases are linked
                                          each other as siblings with the parent pointer pointing to appropriate case node. */
};

struct lysc_node_choice {
    uint16_t nodetype;               /**< LYS_CHOICE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser, unused */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */

    struct lysc_node_case *cases;    /**< list of the cases (linked list). Note that all the children of all the cases are linked each other
                                          as siblings. Their parent pointers points to the specific case they belongs to, so distinguish the
                                          case is simple. */
    struct lysc_node_case *dflt;     /**< default case of the choice, only a pointer into the cases array. */
};

struct lysc_node_leaf {
    uint16_t nodetype;               /**< LYS_LEAF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    struct lyd_value *dflt;          /**< default value */
};

struct lysc_node_leaflist {
    uint16_t nodetype;               /**< LYS_LEAFLIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    struct lyd_value **dflts;        /**< list ([sized array](@ref sizedarrays)) of default values */

    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint */

};

struct lysc_node_list {
    uint16_t nodetype;               /**< LYS_LIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysc_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */

    struct lysc_node_leaf ***uniques; /**< list of sized arrays of pointers to the unique nodes ([sized array](@ref sizedarrays)) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint */
};

struct lysc_node_anydata {
    uint16_t nodetype;               /**< LYS_ANYXML or LYS_ANYDATA */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang */

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Compiled YANG schema tree structure representing YANG module.
 *
 * Semantically validated YANG schema tree for data tree parsing.
 * Contains only the necessary information for the data validation.
 */
struct lysc_module {
    struct lys_module *mod;          /**< covering module structure */

    struct lysc_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysc_action *rpcs;        /**< list of RPCs ([sized array](@ref sizedarrays)) */
    struct lysc_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Examine whether a node is user-ordered list or leaf-list.
 *
 * @param[in] lysc_node Schema node to examine.
 * @return Boolean value whether the @p node is user-ordered or not.
 */
#define lysc_is_userordered(lysc_node) \
    ((!lysc_node || !(lysc_node->nodetype & (LYS_LEAFLIST | LYS_LIST)) || !(lysc_node->flags & LYS_ORDBY_USER)) ? 0 : 1)

/**
 * @brief Examine whether a node is a list's key.
 *
 * @param[in] lysc_node Schema node to examine.
 * @return Boolean value whether the @p node is a key or not.
 */
#define lysc_is_key(lysc_node) \
    ((!lysc_node || !(lysc_node->nodetype & (LYS_LEAF)) || !(lysc_node->flags & LYS_KEY)) ? 0 : 1)

/**
 * @brief Examine whether a node is a non-presence container.
 *
 * @param[in] lysc_node Schema node to examine.
 * @return Boolean value whether the @p node is a NP container or not.
 */
#define lysc_is_np_cont(lysc_node) \
    ((!lysc_node || !(lysc_node->nodetype & (LYS_CONTAINER)) || (lysc_node->flags & LYS_PRESENCE)) ? 0 : 1)

/**
 * @brief Check whether the schema node data instance existence depends on any when conditions.
 * This node and any direct parent choice and case schema nodes are also examined for when conditions.
 *
 * Be careful, this function is not recursive and checks only conditions that apply to this node directly.
 * Meaning if there are any conditions associated with any data parent instance of @p node, they are not returned.
 *
 * @param[in] node Schema node to examine.
 * @return When condition associated with the node data instance, NULL if there is none.
 */
const struct lysc_when *lysc_has_when(const struct lysc_node *node);

/**
 * @brief Get the groupings sized array of the given (parsed) schema node.
 * Decides the node's type and in case it has a groupings array, returns it.
 * @param[in] node Node to examine.
 * @return The node's groupings sized array if any, NULL otherwise.
 */
const struct lysp_grp *lysp_node_groupings(const struct lysp_node *node);

/**
 * @brief Get the typedefs sized array of the given (parsed) schema node.
 * Decides the node's type and in case it has a typedefs array, returns it.
 * @param[in] node Node to examine.
 * @return The node's typedefs sized array if any, NULL otherwise.
 */
const struct lysp_tpdf *lysp_node_typedefs(const struct lysp_node *node);

/**
 * @brief Get the actions/RPCs sized array of the given (parsed) schema node.
 * Decides the node's type and in case it has a actions/RPCs array, returns it.
 * @param[in] node Node to examine.
 * @return The node's actions/RPCs sized array if any, NULL otherwise.
 */
const struct lysp_action *lysp_node_actions(const struct lysp_node *node);

/**
 * @brief Get the Notifications sized array of the given (parsed) schema node.
 * Decides the node's type and in case it has a Notifications array, returns it.
 * @param[in] node Node to examine.
 * @return The node's Notifications sized array if any, NULL otherwise.
 */
const struct lysp_notif *lysp_node_notifs(const struct lysp_node *node);

/**
 * @brief Get the children linked list of the given (parsed) schema node.
 * Decides the node's type and in case it has a children list, returns it.
 * @param[in] node Node to examine.
 * @return The node's children linked list if any, NULL otherwise.
 */
const struct lysp_node *lysp_node_children(const struct lysp_node *node);

/**
 * @brief Get the actions/RPCs sized array of the given (compiled) schema node.
 * Decides the node's type and in case it has a actions/RPCs array, returns it.
 * @param[in] node Node to examine.
 * @return The node's actions/RPCs sized array if any, NULL otherwise.
 */
const struct lysc_action *lysc_node_actions(const struct lysc_node *node);

/**
 * @brief Get the Notifications sized array of the given (compiled) schema node.
 * Decides the node's type and in case it has a Notifications array, returns it.
 * @param[in] node Node to examine.
 * @return The node's Notifications sized array if any, NULL otherwise.
 */
const struct lysc_notif *lysc_node_notifs(const struct lysc_node *node);

/**
 * @brief Get the children linked list of the given (compiled) schema node.
 * Skips over input and output nodes. To return them, use ::lysc_node_children_full().
 *
 * @param[in] node Node to examine.
 * @param[in] flags Config flag to distinguish input (LYS_CONFIG_W) and output (LYS_CONFIG_R) data in case of RPC/action node.
 * @return The node's children linked list if any, NULL otherwise.
 */
const struct lysc_node *lysc_node_children(const struct lysc_node *node, uint16_t flags);

/**
 * @brief Get the children linked list of the given (compiled) schema node.
 * Returns all children node types including input and output. To skip them, use ::lysc_node_children().
 *
 * @param[in] node Node to examine.
 * @param[in] flags Config flag to distinguish input (LYS_CONFIG_W) and output (LYS_CONFIG_R) child in case of RPC/action node.
 * @return Children linked list if any,
 * @return NULL otherwise.
 */
const struct lysc_node *lysc_node_children_full(const struct lysc_node *node, uint16_t flags);

/**
 * @brief Get the parent pointer from any type of (compiled) schema node.
 * Returns input or output for direct descendants of RPC/action nodes.
 * To skip them, use ::lysc_node.parent pointer directly.
 *
 * @param[in] node Node whose parent to get.
 * @return Node parent.
 * @return NULL is there is none.
 */
const struct lysc_node *lysc_node_parent_full(const struct lysc_node *node);

/**
 * @brief Callback to be called for every schema node in a DFS traversal.
 *
 * @param[in] node Current node.
 * @param[in] data Arbitrary user data.
 * @param[out] dfs_continue Set to true if the current subtree should be skipped and continue with siblings instead.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value to terminate DFS and return this value.
 */
typedef LY_ERR (*lysc_dfs_clb)(struct lysc_node *node, void *data, ly_bool *dfs_continue);

/**
 * @brief DFS traversal of all the schema nodes in a (sub)tree including any actions and nested notifications.
 *
 * Node with children, actions, and notifications is traversed in this order:
 * 1) each child subtree;
 * 2) each action subtree;
 * 3) each notification subtree.
 *
 * For algorithm illustration or traversal with actions and notifications skipped, see ::LYSC_TREE_DFS_BEGIN.
 *
 * @param[in] root Schema root to fully traverse.
 * @param[in] dfs_clb Callback to call for each node.
 * @param[in] data Arbitrary user data passed to @p dfs_clb.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value returned by @p dfs_clb.
 */
LY_ERR lysc_tree_dfs_full(const struct lysc_node *root, lysc_dfs_clb dfs_clb, void *data);

/**
 * @brief DFS traversal of all the schema nodes in a module including RPCs and notifications.
 *
 * For more details, see ::lysc_tree_dfs_full().
 *
 * @param[in] mod Module to fully traverse.
 * @param[in] dfs_clb Callback to call for each node.
 * @param[in] data Arbitrary user data passed to @p dfs_clb.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value returned by @p dfs_clb.
 */
LY_ERR lysc_module_dfs_full(const struct lys_module *mod, lysc_dfs_clb dfs_clb, void *data);

/**
 * @brief Set a schema private pointer to a user pointer.
 *
 * @param[in] node Node, whose private field will be assigned. Works also for RPCs, actions, and notifications.
 * @param[in] priv Arbitrary user-specified pointer.
 * @param[out] prev_priv_p Optional previous private object of the \p node. Note, that
 * the caller is in this case responsible (if it is necessary) for freeing the replaced private object.
 * @return LY_ERR value.
 */
LY_ERR lysc_set_private(const struct lysc_node *node, void *priv, void **prev_priv_p);

/**
 * @brief Get how the if-feature statement currently evaluates.
 *
 * @param[in] iff Compiled if-feature statement to evaluate.
 * @return LY_SUCCESS if the statement evaluates to true,
 * @return LY_ENOT if it evaluates to false,
 * @return LY_ERR on error.
 */
LY_ERR lysc_iffeature_value(const struct lysc_iffeature *iff);

/**
 * @brief Get the next feature in the module or submodules.
 *
 * @param[in] last Last returned feature.
 * @param[in] pmod Parsed module and submodoules whose features to iterate over.
 * @param[in,out] idx Submodule index, set to 0 on first call.
 * @return Next found feature, NULL if the last has already been returned.
 */
struct lysp_feature *lysp_feature_next(const struct lysp_feature *last, const struct lysp_module *pmod, uint32_t *idx);

/**
 * @defgroup findxpathoptions Atomize XPath options
 * Options to modify behavior of ::lys_find_xpath() and ::lys_find_xpath_atoms() searching for schema nodes in schema tree.
 * @{
 */
#define LYS_FIND_XP_SCHEMA 0x08 /**< Apply node access restrictions defined for 'when' and 'must' evaluation. */
#define LYS_FIND_XP_OUTPUT 0x10 /**< Search RPC/action output nodes instead of input ones. */
/** @} findxpathoptions */

/**
 * @brief Get all the schema nodes that are required for @p xpath to be evaluated (atoms).
 *
 * @param[in] ctx_node XPath schema context node.
 * @param[in] xpath Data XPath expression filtering the matching nodes. ::LY_PREF_JSON prefix format is expected.
 * @param[in] options Whether to apply some node access restrictions, see @ref findxpathoptions.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value on error.
 */
LY_ERR lys_find_xpath_atoms(const struct lysc_node *ctx_node, const char *xpath, uint32_t options, struct ly_set **set);

/**
 * @brief Get all the schema nodes that are required for @p expr to be evaluated (atoms).
 *
 * @param[in] ctx_node XPath schema context node.
 * @param[in] cur_mod Current module for the expression (where it was "instantiated").
 * @param[in] expr Parsed expression to use.
 * @param[in] prefixes Sized array of compiled prefixes.
 * @param[in] options Whether to apply some node access restrictions, see @ref findxpathoptions.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value on error.
 */
LY_ERR lys_find_expr_atoms(const struct lysc_node *ctx_node, const struct lys_module *cur_mod,
        const struct lyxp_expr *expr, const struct lysc_prefix *prefixes, uint32_t options, struct ly_set **set);

/**
 * @brief Evaluate an @p xpath expression on schema nodes.
 *
 * @param[in] ctx_node XPath schema context node.
 * @param[in] xpath Data XPath expression filtering the matching nodes. ::LY_PREF_JSON prefix format is expected.
 * @param[in] options Whether to apply some node access restrictions, see @ref findxpathoptions.
 * @param[out] set Set of found schema nodes.
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lys_find_xpath(const struct lysc_node *ctx_node, const char *xpath, uint32_t options, struct ly_set **set);

/**
 * @brief Get all the schema nodes that are required for @p path to be evaluated (atoms).
 *
 * @param[in] path Compiled path to use.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value on error.
 */
LY_ERR lys_find_lypath_atoms(const struct ly_path *path, struct ly_set **set);

/**
 * @brief Get all the schema nodes that are required for @p path to be evaluated (atoms).
 *
 * @param[in] ctx libyang context, set for absolute paths.
 * @param[in] ctx_node Starting context node for a relative data path, set for relative paths.
 * @param[in] path JSON path to examine.
 * @param[in] output Search operation output instead of input.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_ERR value on error.
 */
LY_ERR lys_find_path_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path, ly_bool output,
        struct ly_set **set);

/**
 * @brief Get a schema node based on the given data path (JSON format, see @ref howtoXPath).
 *
 * @param[in] ctx libyang context, set for absolute paths.
 * @param[in] ctx_node Starting context node for a relative data path, set for relative paths.
 * @param[in] path JSON path of the node to get.
 * @param[in] output Search operation output instead of input.
 * @return Found schema node or NULL.
 */
const struct lysc_node *lys_find_path(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path,
        ly_bool output);

/**
 * @brief Types of the different schema paths.
 */
typedef enum {
    LYSC_PATH_LOG,  /**< Descriptive path format used in log messages */
    LYSC_PATH_DATA  /**< Similar to ::LYSC_PATH_LOG except that schema-only nodes (choice, case) are skipped */
} LYSC_PATH_TYPE;

/**
 * @brief Generate path of the given node in the requested format.
 *
 * @param[in] node Schema path of this node will be generated.
 * @param[in] pathtype Format of the path to generate.
 * @param[in,out] buffer Prepared buffer of the @p buflen length to store the generated path.
 *                If NULL, memory for the complete path is allocated.
 * @param[in] buflen Size of the provided @p buffer.
 * @return NULL in case of memory allocation error, path of the node otherwise.
 * In case the @p buffer is NULL, the returned string is dynamically allocated and caller is responsible to free it.
 */
char *lysc_path(const struct lysc_node *node, LYSC_PATH_TYPE pathtype, char *buffer, size_t buflen);

/**
 * @brief Available YANG schema tree structures representing YANG module.
 */
struct lys_module {
    struct ly_ctx *ctx;              /**< libyang context of the module (mandatory) */
    const char *name;                /**< name of the module (mandatory) */
    const char *revision;            /**< revision of the module (if present) */
    const char *ns;                  /**< namespace of the module (module - mandatory) */
    const char *prefix;              /**< module prefix or submodule belongsto prefix of main module (mandatory) */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */

    struct lysp_module *parsed;      /**< Simply parsed (unresolved) YANG schema tree */
    struct lysc_module *compiled;    /**< Compiled and fully validated YANG schema tree for data parsing.
                                          Available only for implemented modules. */

    struct lysc_ident *identities;   /**< List of compiled identities of the module ([sized array](@ref sizedarrays))
                                          Identities are outside the compiled tree to allow their linkage to the identities from
                                          the implemented modules. This avoids problems when the module became implemented in
                                          future (no matter if implicitly via augment/deviate or explicitly via
                                          ::lys_set_implemented()). Note that if the module is not implemented (compiled), the
                                          identities cannot be instantiated in data (in identityrefs). */

    struct lys_module **augmented_by;/**< List of modules that augment this module ([sized array](@ref sizedarrays)) */
    struct lys_module **deviated_by; /**< List of modules that deviate this module ([sized array](@ref sizedarrays)) */

    ly_bool implemented;             /**< flag if the module is implemented, not just imported */
    uint8_t latest_revision;         /**< flag to mark the latest available revision:
                                          1 - the latest revision in searchdirs was not searched yet and this is the
                                          latest revision in the current context
                                          2 - searchdirs were searched and this is the latest available revision */
};

/**
 * @brief Get the current real status of the specified feature in the module.
 *
 * If the feature is enabled, but some of its if-features are false, the feature is considered
 * disabled.
 *
 * @param[in] module Module where the feature is defined.
 * @param[in] feature Name of the feature to inspect.
 * @return LY_SUCCESS if the feature is enabled,
 * @return LY_ENOT if the feature is disabled,
 * @return LY_ENOTFOUND if the feature was not found.
 */
LY_ERR lys_feature_value(const struct lys_module *module, const char *feature);

/**
 * @brief Get next schema tree (sibling) node element that can be instantiated in a data tree. Returned node can
 * be from an augment.
 *
 * ::lys_getnext() is supposed to be called sequentially. In the first call, the \p last parameter is usually NULL
 * and function starts returning i) the first \p parent's child or ii) the first top level element of the \p module.
 * Consequent calls suppose to provide the previously returned node as the \p last parameter and still the same
 * \p parent and \p module parameters.
 *
 * Without options, the function is used to traverse only the schema nodes that can be paired with corresponding
 * data nodes in a data tree. By setting some \p options the behavior can be modified to the extent that
 * all the schema nodes are iteratively returned.
 *
 * @param[in] last Previously returned schema tree node, or NULL in case of the first call.
 * @param[in] parent Parent of the subtree where the function starts processing.
 * @param[in] module In case of iterating on top level elements, the \p parent is NULL and
 * module must be specified.
 * @param[in] options [ORed options](@ref sgetnextflags).
 * @return Next schema tree node that can be instantiated in a data tree, NULL in case there is no such element.
 */
const struct lysc_node *lys_getnext(const struct lysc_node *last, const struct lysc_node *parent,
        const struct lysc_module *module, uint32_t options);

/**
 * @defgroup sgetnextflags Options for ::lys_getnext().
 *
 * Various options setting behavior of ::lys_getnext().
 *
 * @{
 */
#define LYS_GETNEXT_WITHCHOICE   0x01 /**< ::lys_getnext() option to allow returning #LYS_CHOICE nodes instead of looking into them */
#define LYS_GETNEXT_NOCHOICE     0x02 /**< ::lys_getnext() option to ignore (kind of conditional) nodes within choice node */
#define LYS_GETNEXT_WITHCASE     0x04 /**< ::lys_getnext() option to allow returning #LYS_CASE nodes instead of looking into them */
#define LYS_GETNEXT_INTONPCONT   0x08 /**< ::lys_getnext() option to look into non-presence container, instead of returning container itself */
#define LYS_GETNEXT_OUTPUT       0x10 /**< ::lys_getnext() option to provide RPC's/action's output schema nodes instead of input schema nodes
                                            provided by default */
/** @} sgetnextflags */

/**
 * @brief Get child node according to the specified criteria.
 *
 * @param[in] parent Optional parent of the node to find. If not specified, the module's top-level nodes are searched.
 * @param[in] module module of the node to find. It is also limitation for the children node of the given parent.
 * @param[in] name Name of the node to find.
 * @param[in] name_len Optional length of the name in case it is not NULL-terminated string.
 * @param[in] nodetype Optional criteria (to speedup) specifying nodetype(s) of the node to find.
 * Used as a bitmask, so multiple nodetypes can be specified.
 * @param[in] options [ORed options](@ref sgetnextflags).
 * @return Found node if any.
 */
const struct lysc_node *lys_find_child(const struct lysc_node *parent, const struct lys_module *module,
        const char *name, size_t name_len, uint16_t nodetype, uint32_t options);

/**
 * @brief Make the specific module implemented.
 *
 * @param[in] mod Module to make implemented. It is not an error
 * to provide already implemented module, it just does nothing.
 * @param[in] features Optional array of features ended with NULL to be enabled. NULL for all features disabled
 * and '*' for all enabled. If the module is already implemented, these features are still correctly set and all
 * the modules recompiled.
 * @return LY_SUCCESS on success.
 * @return LY_EDENIED in case the context contains some other revision of the same module which is already implemented.
 * @return LY_ERR on other errors during module compilation.
 */
LY_ERR lys_set_implemented(struct lys_module *mod, const char **features);

/**
 * @brief Check type restrictions applicable to the particular leaf/leaf-list with the given string @p value.
 *
 * This function check just the type's restriction, if you want to check also the data tree context (e.g. in case of
 * require-instance restriction), use ::lyd_value_validate().
 *
 * @param[in] ctx libyang context for logging (function does not log errors when @p ctx is NULL)
 * @param[in] node Schema node for the @p value.
 * @param[in] value String value to be checked, expected to be in JSON format.
 * @param[in] value_len Length of the given @p value (mandatory).
 * @return LY_SUCCESS on success
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lys_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len);

/**
 * @brief Stringify schema nodetype.
 *
 * @param[in] nodetype Nodetype to stringify.
 * @return Constant string with the name of the node's type.
 */
const char *lys_nodetype2str(uint16_t nodetype);

/**
 * @brief Getter for original XPath expression from a parsed expression.
 *
 * @param[in] path Parsed expression.
 * @return Original string expression.
 */
const char *lyxp_get_expr(const struct lyxp_expr *path);

/** @} schematree */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
