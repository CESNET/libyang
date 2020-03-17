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

#include "tree_data.h"
#include "log.h"
#include "tree.h"

struct ly_ctx;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Macro to iterate via all elements in a schema tree which can be instantiated in data tree
 * (skips cases, input, output). This is the opening part to the #LYSC_TREE_DFS_END - they always have to be used together.
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
    { int LYSC_TREE_DFS_continue = 0; struct lysc_node *LYSC_TREE_DFS_next; \
    for ((ELEM) = (LYSC_TREE_DFS_next) = (START); \
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
        (LYSC_TREE_DFS_next) = (struct lysc_node*)lysc_node_children(ELEM, 0); \
    }\
    if (!(LYSC_TREE_DFS_next)) { \
        /* in case of RPC/action, get also the output children */ \
        if (!LYSC_TREE_DFS_continue && (ELEM)->nodetype == LYS_ACTION) { \
            (LYSC_TREE_DFS_next) = (struct lysc_node*)lysc_node_children(ELEM, LYS_CONFIG_R); \
        } \
        if (!(LYSC_TREE_DFS_next)) { \
            /* no children */ \
            if ((ELEM) == (struct lysc_node*)(START)) { \
                /* we are done, (START) has no children */ \
                break; \
            } \
            /* try siblings */ \
            (LYSC_TREE_DFS_next) = (ELEM)->next; \
        } \
    } \
    while (!(LYSC_TREE_DFS_next)) { \
        /* parent is already processed, go to its sibling */ \
        (ELEM) = (ELEM)->parent; \
        /* no siblings, go back through parents */ \
        if ((ELEM) == (struct lysc_node*)(START)) { \
            /* we are done, no next element to process */ \
            break; \
        } \
        if ((ELEM)->nodetype == LYS_ACTION) { \
            /* there is actually next node as a child of action's output */ \
            (LYSC_TREE_DFS_next) = (struct lysc_node*)lysc_node_children(ELEM, LYS_CONFIG_R); \
        } \
        if (!(LYSC_TREE_DFS_next)) { \
            (LYSC_TREE_DFS_next) = (ELEM)->next; \
        } \
    } } \

/**
 * @brief Schema input formats accepted by libyang [parser functions](@ref howtoschemasparsers).
 */
typedef enum {
    LYS_IN_UNKNOWN = 0,  /**< unknown format, used as return value in case of error */
    LYS_IN_YANG = 1,     /**< YANG schema input format */
    LYS_IN_YIN = 3       /**< YIN schema input format */
} LYS_INFORMAT;

/**
 * @brief Schema output formats accepted by libyang [printer functions](@ref howtoschemasprinters).
 */
typedef enum {
    LYS_OUT_UNKNOWN = 0, /**< unknown format, used as return value in case of error */
    LYS_OUT_YANG = 1,    /**< YANG schema output format */
    LYS_OUT_YANG_COMPILED = 2, /**< YANG schema output format of the compiled schema tree */
    LYS_OUT_YIN = 3,     /**< YIN schema output format */

    LYS_OUT_TREE,        /**< Tree schema output format, for more information see the [printers](@ref howtoschemasprinters) page */
    LYS_OUT_INFO,        /**< Info schema output format, for more information see the [printers](@ref howtoschemasprinters) page */
    LYS_OUT_JSON,        /**< JSON schema output format, reflecting YIN format with conversion of attributes to object's members */
} LYS_OUTFORMAT;

#define LY_REV_SIZE 11   /**< revision data string length (including terminating NULL byte) */

#define LYS_UNKNOWN 0x0000        /**< uninitalized unknown statement node */
#define LYS_CONTAINER 0x0001      /**< container statement node */
#define LYS_CHOICE 0x0002         /**< choice statement node */
#define LYS_LEAF 0x0004           /**< leaf statement node */
#define LYS_LEAFLIST 0x0008       /**< leaf-list statement node */
#define LYS_LIST 0x0010           /**< list statement node */
#define LYS_ANYXML 0x0020         /**< anyxml statement node */
#define LYS_ANYDATA 0x0120        /**< anydata statement node, in tests it can be used for both #LYS_ANYXML and #LYS_ANYDATA */

#define LYS_ACTION 0x400          /**< RPC or action */
#define LYS_RPC LYS_ACTION        /**< RPC or action (for backward compatibility) */
#define LYS_NOTIF 0x800

#define LYS_CASE 0x0040           /**< case statement node */
#define LYS_USES 0x0080           /**< uses statement node */
#define LYS_INPUT 0x100
#define LYS_OUTPUT 0x200
#define LYS_INOUT 0x300
#define LYS_GROUPING 0x1000
#define LYS_AUGMENT 0x2000

/**
 * @brief List of YANG statements
 */
enum ly_stmt {
    LY_STMT_NONE = 0,
    LY_STMT_STATUS,             /**< in lysc_ext_substmt::storage stored as a pointer to `uint16_t`, only cardinality < #LY_STMT_CARD_SOME is allowed */
    LY_STMT_CONFIG,             /**< in lysc_ext_substmt::storage stored as a pointer to `uint16_t`, only cardinality < #LY_STMT_CARD_SOME is allowed */
    LY_STMT_MANDATORY,
    LY_STMT_UNITS,              /**< in lysc_ext_substmt::storage stored as a pointer to `const char *` (cardinality < #LY_STMT_CARD_SOME)
                                     or as a pointer to a [sized array](@ref sizedarrays) `const char **` */
    LY_STMT_DEFAULT,
    LY_STMT_TYPE,               /**< in lysc_ext_substmt::storage stored as a pointer to `struct lysc_type *` (cardinality < #LY_STMT_CARD_SOME)
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
    LY_STMT_IF_FEATURE,         /**< in lysc_ext_substmt::storage stored as a pointer to `struct lysc_iffeature` (cardinality < #LY_STMT_CARD_SOME)
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
    LYEXT_PAR_FEATURE,   /**< ::lysc_feature */
    LYEXT_PAR_MUST,      /**< ::lysc_must */
    LYEXT_PAR_PATTERN,   /**< ::lysc_pattern */
    LYEXT_PAR_LENGTH,    /**< ::lysc_range */
    LYEXT_PAR_RANGE,     /**< ::lysc_range */
    LYEXT_PAR_WHEN,      /**< ::lysc_when */
    LYEXT_PAR_IDENT,     /**< ::lysc_ident */
    LYEXT_PAR_EXT,       /**< ::lysc_ext */
    LYEXT_PAR_IMPORT,    /**< ::lysc_import */
//    LYEXT_PAR_TPDF,      /**< ::lysp_tpdf */
//    LYEXT_PAR_EXTINST,   /**< ::lysp_ext_instance */
//    LYEXT_PAR_REFINE,    /**< ::lysp_refine */
//    LYEXT_PAR_DEVIATION, /**< ::lysp_deviation */
//    LYEXT_PAR_DEVIATE,   /**< ::lysp_deviate */
//    LYEXT_PAR_INCLUDE,   /**< ::lysp_include */
//    LYEXT_PAR_REVISION,  /**< ::lysp_revision */
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
    LYEXT_SUBSTMT_IFFEATURE,     /**< extension of the if-feature statement */
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
    uint32_t insubstmt_index;               /**< in case the instance is in a substatement, this identifies
                                                 the index of that substatement */
    uint8_t yin;                            /** flag for YIN source format, can be set to LYS_YIN */
    LYEXT_PARENT parent_type;               /**< type of the parent structure */
};

/**
 * @brief YANG feature-stmt
 */
struct lysp_feature {
    const char *name;                /**< feature name (mandatory) */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement  */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values allowed */
};

/**
 * @brief YANG identity-stmt
 */
struct lysp_ident {
    const char *name;                /**< identity name (mandatory), including possible prefix */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
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
    const char *arg;                 /**< The restriction expression/value (mandatory);
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
    char date[LY_REV_SIZE];           /**< revision date (madatory) */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
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
    const char *path;                /**< path - leafref */
    const char **bases;              /**< list of base identifiers ([sized array](@ref sizedarrays)) - identityref */
    struct lysp_type *types;         /**< list of sub-types ([sized array](@ref sizedarrays)) - union */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

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
    const char *dflt;                /**< default value of the newly defined type */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    const char *presence;            /**< presence description */
    const char **dflts;              /**< list of default values ([sized array](@ref sizedarrays)) */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
};

/**
 * @defgroup deviatetypes Deviate types
 * @{
 */
#define LYS_DEV_NOT_SUPPORTED 1      /**< deviate type not-supported */
#define LYS_DEV_ADD 2                /**< deviate type add */
#define LYS_DEV_DELETE 3             /**< deviate type delete */
#define LYS_DEV_REPLACE 4            /**< deviate type replace */
/** @} */

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
    const char **uniques;            /**< list of uniques specifications ([sized array](@ref sizedarrays)) */
    const char **dflts;              /**< list of default values ([sized array](@ref sizedarrays)) */
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
    const char **uniques;            /**< list of uniques specifications ([sized array](@ref sizedarrays)) */
    const char **dflts;              /**< list of default values ([sized array](@ref sizedarrays)) */
};

struct lysp_deviate_rpl {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysp_type *type;          /**< type of the node */
    const char *units;               /**< units of the values */
    const char *dflt;                /**< default value */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_deviation {
    const char *nodeid;              /**< target absolute schema nodeid (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_deviate* deviates;   /**< list of deviate specifications (linked list) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @defgroup spnodeflags Parsed schema nodes flags
 * @ingroup snodeflags
 *
 * Various flags for parsed schema nodes.
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
 *       1 LYS_CONFIG_W     |x|x|x|x|x|x|x| | | | | | | | | | |x| |x| | | |
 *         LYS_SET_BASE     | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       2 LYS_CONFIG_R     |x|x|x|x|x|x|x| | | | | | | | | | |x| |x| | | |
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
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     12 LYS_YIN_ARGUMENT  | | | | | | | | | | | | | | | | | | | | | | |x|
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/**
 * @defgroup scnodeflags Compiled schema nodes flags
 * @ingroup snodeflags
 *
 * Various flags for compiled schema nodes.
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
 *         LYS_FENABLED     | | | | | | | | | |x| | | | |
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
 * @ingroup schematree
 * @{
 */
#define LYS_CONFIG_W     0x01        /**< config true; */
#define LYS_CONFIG_R     0x02        /**< config false; */
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
#define LYS_PRESENCE     0x80        /**< flag for presence property of a container, applicable only to ::lysc_node_container */
#define LYS_UNIQUE       0x80        /**< flag for leafs being part of a unique set, applicable only to ::lysc_node_leaf */
#define LYS_KEY          0x100       /**< flag for leafs being a key of a list, applicable only to ::lysc_node_leaf */
#define LYS_KEYLESS      0x200       /**< flag for list without any key, applicable only to ::lysc_node_list */
#define LYS_FENABLED     0x100       /**< feature enabled flag, applicable only to ::lysc_feature */
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

#define LYS_ISENUM       0x200       /**< flag to simply distinguish type in struct lysc_type_bitenum_item */

#define LYS_FLAGS_COMPILED_MASK 0xff /**< mask for flags that maps to the compiled structures */
/** @} */

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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* leaf */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    const char *dflt;                /**< default value */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* leaf-list */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    const char **dflts;              /**< list of default values ([sized array](@ref sizedarrays)) */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* list */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    const char *key;                 /**< keys specification */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    const char **uniques;            /**< list of unique specifications ([sized array](@ref sizedarrays)) */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* choice */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    const char* dflt;                /**< default case */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    /* case */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
};

struct lysp_node_anydata {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_ANYXML || LYS_ANYDATA */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
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
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
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
    uint16_t nodetype;               /**< LYS_INOUT */
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
    uint16_t nodetype;               /**< LYS_ACTION */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_action_inout input;  /**< RPC's/Action's input */
    struct lysp_action_inout output; /**< RPC's/Action's output */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG notification-stmt
 */
struct lysp_notif {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< LYS_NOTIF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief supported YANG schema version values
 */
typedef enum LYS_VERSION {
    LYS_VERSION_UNDEF = 0,  /**< no specific version, YANG 1.0 as default */
    LYS_VERSION_1_0 = 1,    /**< YANG 1.0 */
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
    struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    uint8_t parsing:1;               /**< flag for circular check */
};

struct lysp_submodule {
    const char *belongsto;           /**< belongs to parent module (submodule - mandatory) */

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
    struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    uint8_t parsing:1;               /**< flag for circular check */

    uint8_t latest_revision:2;       /**< flag to mark the latest available revision:
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
    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
};

/**
 * @brief Free the printable YANG schema tree structure.
 *
 * @param[in] module Printable YANG schema tree structure to free.
 */
void lysp_module_free(struct lysp_module *module);

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
 * @brief Compiled YANG if-feature-stmt
 */
struct lysc_iffeature {
    uint8_t *expr;                   /**< 2bits array describing the if-feature expression in prefix format, see @ref ifftokens */
    struct lysc_feature **features;  /**< array of pointers to the features used in expression ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG import-stmt
 */
struct lysc_import {
    struct lys_module *module;       /**< link to the imported module */
    const char *prefix;              /**< prefix for the data from the imported schema (mandatory) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG when-stmt
 */
struct lysc_when {
    struct lys_module *module;       /**< module where the must was defined */
    struct lyxp_expr *cond;          /**< XPath when condition */
    struct lysc_node *context;       /**< context node for evaluating the expression, NULL if the context is root node */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/**
 * @brief YANG feature-stmt
 */
struct lysc_feature {
    const char *name;                /**< feature name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lys_module *module;       /**< module structure */
    struct lysc_feature **depfeatures;/**< list of pointers to other features depending on this one ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* and
                                          #LYS_FENABLED values allowed */
};

/**
 * @defgroup ifftokens if-feature expression tokens
 * Tokens of if-feature expression used in ::lysc_iffeature#expr
 *
 * @{
 */
#define LYS_IFF_NOT  0x00 /**< operand "not" */
#define LYS_IFF_AND  0x01 /**< operand "and" */
#define LYS_IFF_OR   0x02 /**< operand "or" */
#define LYS_IFF_F    0x03 /**< feature */
/**
 * @}
 */

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
    uint32_t inverted:1;             /**< invert-match flag */
    uint32_t refcount:31;            /**< reference counter */
};

struct lysc_must {
    struct lys_module *module;       /**< module where the must was defined */
    struct lyxp_expr *cond;          /**< XPath when condition */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_type {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyd_value *dflt;          /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
};

struct lysc_type_num {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyd_value *dflt;          /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_dec {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyd_value *dflt;          /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    uint8_t fraction_digits;         /**< fraction digits specification */
    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_str {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyd_value *dflt;          /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    union {
        int32_t value;           /**< integer value associated with the enumeration */
        uint32_t position;       /**< non-negative integer value associated with the bit */
    };
    uint16_t flags;              /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ and LYS_SET_VALUE
                                          values are allowed */
};

struct lysc_type_enum {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type_bitenum_item *enums; /**< enumerations list ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_bits {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type_bitenum_item *bits; /**< bits list ([sized array](@ref sizedarrays)), mandatory (at least 1 item),
                                              the items are ordered by their position value. */
};

struct lysc_type_leafref {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    const char *path;                /**< target path */
    struct lys_module *path_context; /**< module where the path is defined, so it provides context to resolve prefixes */
    struct lysc_type *realtype;      /**< pointer to the real (first non-leafref in possible leafrefs chain) type. */
    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_identityref {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_ident **bases;       /**< list of pointers to the base identities ([sized array](@ref sizedarrays)),
                                          mandatory (at least 1 item) */
};

struct lysc_type_instanceid {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_union {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type **types;        /**< list of types in the union ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_bin {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    struct lys_module *dflt_mod;     /**< module where the lysc_type::dflt value was defined (needed to correctly map prefixes). */
    struct lysc_type_plugin *plugin; /**< type's plugin with built-in as well as user functions to canonize or validate the value of the type */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *length;       /**< Optional length limitation */
};

struct lysc_action_inout {
    struct lysc_node *data;          /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
};

struct lysc_action {
    uint16_t nodetype;               /**< LYS_ACTION */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_action *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node - RPC) */

    struct lysc_ext_instance *input_exts;  /**< list of the extension instances of input ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *output_exts; /**< list of the extension instances of outpu ([sized array](@ref sizedarrays)) */

    const char *name;                /**< action/RPC name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */

    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

    struct lysc_action_inout input;  /**< RPC's/action's input */
    struct lysc_action_inout output; /**< RPC's/action's output */

};

struct lysc_notif {
    uint16_t nodetype;               /**< LYS_NOTIF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_notif *sp;           /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */

    struct lysc_node *data;          /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */

    const char *name;                /**< Notification name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */

    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Compiled YANG data node
 */
struct lysc_node {
    uint16_t nodetype;               /**< type of the node (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
};

struct lysc_node_container {
    uint16_t nodetype;               /**< LYS_CONTAINER */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysc_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
};

struct lysc_node_case {
    uint16_t nodetype;               /**< LYS_CASE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */

    struct lysc_node *child;         /**< first child node of the case (linked list). Note that all the children of all the sibling cases are linked
                                          each other as siblings with the parent pointer pointing to appropriate case node. */
};

struct lysc_node_choice {
    uint16_t nodetype;               /**< LYS_CHOICE */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */

    struct lysc_node_case *cases;    /**< list of the cases (linked list). Note that all the children of all the cases are linked each other
                                          as siblings. Their parent pointers points to the specific case they belongs to, so distinguish the
                                          case is simple. */
    struct lysc_node_case *dflt;     /**< default case of the choice, only a pointer into the cases array. */
};

struct lysc_node_leaf {
    uint16_t nodetype;               /**< LYS_LEAF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    struct lyd_value *dflt;          /**< default value */
    struct lys_module *dflt_mod;     /**< module where the lysc_node_leaf::dflt value was defined (needed to correctly map prefixes). */
};

struct lysc_node_leaflist {
    uint16_t nodetype;               /**< LYS_LEAFLIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    struct lyd_value **dflts;        /**< list ([sized array](@ref sizedarrays)) of default values */
    struct lys_module **dflts_mods;  /**< list ([sized array](@ref sizedarrays)) of modules where the lysc_node_leaflist::dflts values were defined
                                          (needed to correctly map prefixes). */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint */

};

struct lysc_node_list {
    uint16_t nodetype;               /**< LYS_LIST */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */

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
    struct lys_module *module;       /**< module structure */
    struct lysp_node *sp;            /**< simply parsed (SP) original of the node, NULL if the SP schema was removed or in case of implicit case node. */
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
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */

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
    struct lysc_import *imports;     /**< list of imported modules ([sized array](@ref sizedarrays)) */

    struct lysc_feature *features;   /**< list of feature definitions ([sized array](@ref sizedarrays)) */
    struct lysc_ident *identities;   /**< list of identities ([sized array](@ref sizedarrays)) */
    struct lysc_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysc_action *rpcs;        /**< list of RPCs ([sized array](@ref sizedarrays)) */
    struct lysc_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

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
 * Decides the node's type and in case it has a children list, returns it.
 * @param[in] node Node to examine.
 * @param[in] flags Config flag to distinguish input (LYS_CONFIG_W) and output (LYS_CONFIG_R) data in case of RPC/action node.
 * @return The node's children linked list if any, NULL otherwise.
 */
const struct lysc_node *lysc_node_children(const struct lysc_node *node, uint16_t flags);

/**
 * @brief Get how the if-feature statement currently evaluates.
 *
 * @param[in] iff Compiled if-feature statement to evaluate.
 * @return If the statement evaluates to true, 1 is returned. 0 is returned when the statement evaluates to false.
 */
int lysc_iffeature_value(const struct lysc_iffeature *iff);

/**
 * @brief Get the current status of the provided feature.
 *
 * @param[in] feature Compiled feature statement to examine.
 * @return
 * - 1 if feature is enabled,
 * - 0 if feature is disabled,
 * - -1 in case of error (invalid argument)
 */
int lysc_feature_value(const struct lysc_feature *feature);

/**
 * @brief Types of the different schema paths.
 */
typedef enum {
    LYSC_PATH_LOG /**< Descriptive path format used in log messages */
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
    struct lysc_feature *off_features;/**< List of pre-compiled features of the module in non implemented modules ([sized array](@ref sizedarrays)).
                                          These features are always disabled and cannot be enabled until the module
                                          become implemented. The features are present in this form to allow their linkage
                                          from if-feature statements of the compiled schemas and their proper use in case
                                          the module became implemented in future (no matter if implicitly via augment/deviate
                                          or explicitly via ly_ctx_module_implement()). */
    uint8_t implemented;             /**< flag if the module is implemented, not just imported. The module is implemented if
                                          the flag has non-zero value. Specific values are used internally:
                                          1 - implemented module
                                          2 - recently implemented module by dependency, it can be reverted in rollback procedure */
    uint8_t latest_revision;         /**< flag to mark the latest available revision:
                                          1 - the latest revision in searchdirs was not searched yet and this is the
                                          latest revision in the current context
                                          2 - searchdirs were searched and this is the latest available revision */
    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
};

/**
 * @brief Enable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * If all features are being enabled, it must be possible respecting their if-feature conditions. For example,
 * enabling all features on the following feature set will fail since it is not possible to enable both features
 * (and it is unclear which of them should be enabled then). In this case the LY_EDENIED is returned and the feature
 * is untouched.
 *
 *     feature f1;
 *     feature f2 { if-feature 'not f1';}
 *
 * @param[in] module Module where the feature will be enabled.
 * @param[in] feature Name of the feature to enable. To enable all features at once, use asterisk (`*`) character.
 * @return LY_ERR value.
 */
LY_ERR lys_feature_enable(const struct lys_module *module, const char *feature);

/**
 * @brief Disable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be disabled.
 * @param[in] feature Name of the feature to disable. To disable all features at once, use asterisk (`*`) character.
 * @return LY_ERR value
 */
LY_ERR lys_feature_disable(const struct lys_module *module, const char *feature);

/**
 * @brief Get the current status of the specified feature in the module.
 *
 * @param[in] module Module where the feature is defined.
 * @param[in] feature Name of the feature to inspect.
 * @return
 * - 1 if feature is enabled,
 * - 0 if feature is disabled,
 * - -1 in case of error (e.g. feature is not defined or invalid arguments)
 */
int lys_feature_value(const struct lys_module *module, const char *feature);

/**
 * @brief Load a schema into the specified context.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format);

/**
 * @brief Read a schema from file descriptor into the specified context.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format);

/**
 * @brief Read a schema into the specified context from a file.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format);

/**
 * @brief Search for the schema file in the specified searchpaths.
 *
 * @param[in] searchpaths NULL-terminated array of paths to be searched (recursively). Current working
 * directory is searched automatically (but non-recursively if not in the provided list). Caller can use
 * result of the ly_ctx_get_searchdirs().
 * @param[in] cwd Flag to implicitly search also in the current working directory (non-recursively).
 * @param[in] name Name of the schema to find.
 * @param[in] revision Revision of the schema to find. If NULL, the newest found schema filepath is returned.
 * @param[out] localfile Mandatory output variable containing absolute path of the found schema. If no schema
 * complying the provided restriction is found, NULL is set.
 * @param[out] format Optional output variable containing expected format of the schema document according to the
 * file suffix.
 * @return LY_ERR value (LY_SUCCESS is returned even if the file is not found, then the *localfile is NULL).
 */
LY_ERR lys_search_localfile(const char * const *searchpaths, int cwd, const char *name, const char *revision, char **localfile, LYS_INFORMAT *format);

/**
 * @brief Get next schema tree (sibling) node element that can be instantiated in a data tree. Returned node can
 * be from an augment.
 *
 * lys_getnext() is supposed to be called sequentially. In the first call, the \p last parameter is usually NULL
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
                                    const struct lysc_module *module, int options);

/**
 * @defgroup sgetnextflags lys_getnext() flags
 * @ingroup schematree
 *
 * @{
 */
#define LYS_GETNEXT_WITHCHOICE   0x01 /**< lys_getnext() option to allow returning #LYS_CHOICE nodes instead of looking into them */
#define LYS_GETNEXT_NOCHOICE     0x02 /**< lys_getnext() option to ignore (kind of conditional) nodes within choice node */
#define LYS_GETNEXT_WITHCASE     0x04 /**< lys_getnext() option to allow returning #LYS_CASE nodes instead of looking into them */
#define LYS_GETNEXT_INTONPCONT   0x40 /**< lys_getnext() option to look into non-presence container, instead of returning container itself */
#define LYS_GETNEXT_NOSTATECHECK 0x100 /**< lys_getnext() option to skip checking module validity (import-only, disabled) and
                                            relevant if-feature conditions state */
#define LYS_GETNEXT_OUTPUT       0x200 /**< lys_getnext() option to provide RPC's/action's output schema nodes instead of input schema nodes
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
                                       const char *name, size_t name_len, uint16_t nodetype, int options);

/**
 * @brief Make the specific module implemented.
 *
 * @param[in] mod Module to make implemented. It is not an error
 * to provide already implemented module, it just does nothing.
 * @return LY_SUCCESS on success.
 * @return LY_EDENIED in case the context contains some other revision of the same module which is already implemented.
 */
LY_ERR lys_set_implemented(struct lys_module *mod);

/**
 * @brief Check if the schema node is disabled in the schema tree, i.e. there is any disabled if-feature statement
 * affecting the node.
 *
 * @param[in] node Schema node to check.
 * @param[in] recursive - 0 to check if-feature only in the \p node schema node,
 * - 1 to check if-feature in all ascendant schema nodes until there is a node possibly having an instance in a data tree
 * @return NULL if enabled,
 * @return pointer to the node with the unsatisfied (disabled) if-feature expression.
 */
const struct lysc_node *lysc_node_is_disabled(const struct lysc_node *node, int recursive);

/**
 * @brief Check type restrictions applicable to the particular leaf/leaf-list with the given string @p value.
 *
 * This function check just the type's restriction, if you want to check also the data tree context (e.g. in case of
 * require-instance restriction), use lyd_value_validate().
 *
 * @param[in] ctx libyang context for logging (function does not log errors when @p ctx is NULL)
 * @param[in] node Schema node for the @p value.
 * @param[in] value String value to be checked.
 * @param[in] value_len Length of the given @p value (mandatory).
 * @param[in] get_prefix Callback function to resolve prefixes used in the @p value string.
 * @param[in] get_prefix_data Private data for the @p get_prefix callback.
 * @param[in] format Input format of the @p value.
 * @return LY_SUCCESS on success
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lys_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len,
                          ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format);

/**
 * @brief Stringify schema nodetype.
 * @param[in] nodetype Nodetype to stringify.
 * @return Constant string with the name of the node's type.
 */
const char *lys_nodetype2str(uint16_t nodetype);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
