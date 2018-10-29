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

#include <stdint.h>

#include "extensions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief XPath representation.
 */
struct lyxp_expr;

/**
 * @brief Macro selector for other LY_ARRAY_* macros, do not use directly!
 */
#define LY_ARRAY_SELECT(_1, _2, NAME, ...) NAME

/**
 * @brief Helper macro to go through sized-arrays with a pointer iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * @param[in] ARRAY Array to go through
 * @param[in] TYPE Type of the records in the ARRAY
 * @param[out] ITER Iterating pointer to the item being processed in each loop
 */
#define LY_ARRAY_FOR_ITER(ARRAY, TYPE, ITER) \
    for (ITER = ARRAY; \
         (ARRAY) && ((void*)ITER - (void*)ARRAY)/(sizeof(TYPE)) < (*((uint32_t*)(ARRAY) - 1)); \
         ITER = (void*)((TYPE*)ITER + 1))

/**
 * @brief Helper macro to go through sized-arrays with a numeric iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * To access an item with the INDEX value, use always LY_ARRAY_INDEX macro!
 *
 * @param[in] ARRAY Array to go through
 * @param[out] INDEX Iterating index of the item being processed in each loop
 */
#define LY_ARRAY_FOR_INDEX(ARRAY, INDEX) \
    for (INDEX = 0; \
         ARRAY && INDEX < (*((uint32_t*)(ARRAY) - 1)); \
         ++INDEX)

/**
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 */

/**
 * @brief Get a number of records in the ARRAY.
 *
 * Does not check if array exists!
 */
#define LY_ARRAY_SIZE(ARRAY) (*((uint32_t*)(ARRAY) - 1))

/**
 * @brief Sized-array iterator (for-loop).
 *
 * Use with opening curly bracket (`{`).
 *
 * There are 2 variants:
 *
 *     LY_ARRAY_FOR(ARRAY, TYPE, ITER)
 *
 * Where ARRAY is a sized-array to go through, TYPE is the type of the items in the ARRAY and ITER is a pointer variable
 * providing the items of the ARRAY in the loops. This functionality is provided by LY_ARRAY_FOR_ITER macro
 *
 *     LY_ARRAY_FOR(ARRAY, INDEX)
 *
 * The ARRAY is again a sized-array to go through, the INDEX is a variable (unsigned integer) for storing iterating ARRAY's index
 * to access the items of ARRAY in the loops. This functionality is provided by LY_ARRAY_FOR_INDEX macro.
 */
#define LY_ARRAY_FOR(ARRAY, ...) LY_ARRAY_SELECT(__VA_ARGS__, LY_ARRAY_FOR_ITER, LY_ARRAY_FOR_INDEX)(ARRAY, __VA_ARGS__)

/**
 * @brief Macro to iterate via all sibling elements without affecting the list itself
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type.
 *
 * Use with opening curly bracket (`{`). All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param ELEM Iterator.
 */
#define LY_LIST_FOR(START, ELEM) \
    for ((ELEM) = (START); \
         (ELEM); \
         (ELEM) = (ELEM)->next)

/**
 * @ingroup datatree
 * @brief Macro to iterate via all sibling elements allowing to modify the list itself (e.g. removing elements)
 *
 * Use with opening curly bracket (`{`). All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param NEXT Temporary storage to allow removing of the current iterator content.
 * @param ELEM Iterator.
 */
#define LY_LIST_FOR_SAFE(START, NEXT, ELEM) \
    for ((ELEM) = (START); \
         (ELEM) ? (NEXT = (ELEM)->next, 1) : 0; \
         (ELEM) = (NEXT))

/**
 * @brief Schema input formats accepted by libyang [parser functions](@ref howtoschemasparsers).
 */
typedef enum {
    LYS_IN_UNKNOWN = 0,  /**< unknown format, used as return value in case of error */
    LYS_IN_YANG = 1,     /**< YANG schema input format */
    LYS_IN_YIN = 2       /**< YIN schema input format */
} LYS_INFORMAT;

/**
 * @brief Schema output formats accepted by libyang [printer functions](@ref howtoschemasprinters).
 */
typedef enum {
    LYS_OUT_UNKNOWN = 0, /**< unknown format, used as return value in case of error */
    LYS_OUT_YANG = 1,    /**< YANG schema output format */
    LYS_OUT_YIN = 2,     /**< YIN schema output format */
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
#define LYS_CASE 0x0040           /**< case statement node */
#define LYS_USES 0x0080           /**< uses statement node */
#define LYS_ANYDATA 0x0120        /**< anydata statement node, in tests it can be used for both #LYS_ANYXML and #LYS_ANYDATA */

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
    struct lysp_module *submodule;   /**< pointer to the parsed submodule structure
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
    uint16_t flags;                  /**< LYS_STATUS_* and LYS_YINELEM values (@ref snodeflags) */
};

/**
 * @brief Helper structure for generic storage of the extension instances content.
 */
struct lysp_stmt {
    const char *stmt;                /**< identifier of the statement */
    const char *arg;                 /**< statement's argument */
    struct lysp_stmt *next;          /**< link to the next statement */
    struct lysp_stmt *child;         /**< list of the statement's substatements (linked list) */
};

/**
 * @brief YANG extension instance
 */
struct lysp_ext_instance {
    const char *name;                /**< extension identifier, including possible prefix */
    const char *argument;            /**< optional value of the extension's argument */
    LYEXT_SUBSTMT insubstmt;         /**< value identifying placement of the extension instance */
    uint32_t insubstmt_index;        /**< in case the instance is in a substatement, this identifies
                                          the index of that substatement */
    struct lysp_stmt *child;         /**< list of the extension's substatements (linked list) */
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

    uint8_t fraction_digits;         /**< number of fraction digits - decimal64 */
    uint8_t require_instance;        /**< require-instance flag - leafref, instance */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_SET_REQINST allowed */
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
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values allowed */
};

/**
 * @brief YANG grouping-stmt
 */
struct lysp_grp {
    const char *name;                /**< grouping name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
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
 * @brief YANG uses-augment-stmt and augment-stmt
 */
struct lysp_augment {
    const char *nodeid;              /**< target schema nodeid (mandatory) - absolute for global augments, descendant for uses's augments */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
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
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
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

#define LYS_CONFIG_W     0x01        /**< config true; */
#define LYS_CONFIG_R     0x02        /**< config false; */
#define LYS_CONFIG_MASK  0x03        /**< mask for config value */
#define LYS_STATUS_CURR  0x08        /**< status current; */
#define LYS_STATUS_DEPRC 0x10        /**< status deprecated; */
#define LYS_STATUS_OBSLT 0x20        /**< status obsolete; */
#define LYS_STATUS_MASK  0x38        /**< mask for status value */
#define LYS_MAND_TRUE    0x40        /**< mandatory true; applicable only to
                                          ::lysp_node_choice/::lysc_node_choice, ::lysp_node_leaf/::lysc_node_leaf
                                          and ::lysp_node_anydata/::lysc_node_anydata */
#define LYS_MAND_FALSE   0x80        /**< mandatory false; applicable only to
                                          ::lysp_node_choice/::lysc_node_choice, ::lysp_node_leaf/::lysc_node_leaf
                                          and ::lysp_node_anydata/::lysc_node_anydata */
#define LYS_MAND_MASK    0xc0        /**< mask for mandatory values */
#define LYS_ORDBY_SYSTEM 0x100       /**< ordered-by system lists, applicable only to
                                          ::lysp_node_list/lysc_node_list and ::lysp_node_leaflist/::lysc_node_list */
#define LYS_ORDBY_USER   0x200       /**< ordered-by user lists, applicable only to
                                          ::lysp_node_list/lysc_node_list and ::lysp_node_leaflist/::lysc_node_list */
#define LYS_ORDBY_MASK   0x300       /**< mask for ordered-by flags */
#define LYS_FENABLED     0x100       /**< feature enabled flag, applicable only to ::lysp_feature/::lysc_feature */
#define LYS_AUTOASSIGNED 0x01        /**< value was auto-assigned, applicable only to
                                          ::lysp_type/::lysc_type enum and bits flags */
#define LYS_YINELEM_TRUE 0x01        /**< yin-element true for extension's argument */
#define LYS_YINELEM_FALSE 0x02       /**< yin-element false for extension's argument */
#define LYS_YINELEM_MASK 0x03        /**< mask for yin-element value */
#define LYS_SET_VALUE    0x01        /**< value attribute is set */
#define LYS_SET_MAX      0x400       /**< max attribute is set */
#define LYS_SET_MIN      0x800       /**< min attribute is set */
#define LYS_SET_REQINST  0x01        /**< require_instance attribute is set */

/**
 * @brief Generic YANG data node
 */
struct lysp_node {
    uint16_t nodetype;               /**< type of the node (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
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
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_action_inout *input; /**< RPC's/Action's input */
    struct lysp_action_inout *output;/**< RPC's/Action's output */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
};

/**
 * @brief YANG notification-stmt
 */
struct lysp_notif {
    const char *name;                /**< grouping name reference (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_grp *groupings;      /**< list of groupings ([sized array](@ref sizedarrays)) */
    struct lysp_node *data;          /**< list of data nodes (linked list) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
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
    struct ly_ctx *ctx;              /**< libyang context of the module (mandatory) */
    const char *name;                /**< name of the module (mandatory) */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    union {
        /* module */
        const char *ns;              /**< namespace of the module (module - mandatory) */
        /* submodule */
        const char *belongsto;       /**< belongs to parent module (submodule - mandatory) */
    };
    const char *prefix;              /**< module prefix or submodule belongsto prefix of main module (mandatory) */
    struct lysp_revision *revs;      /**< list of the module revisions ([sized array](@ref sizedarrays)), the first revision
                                          in the list is always the last (newest) revision of the module */
    struct lysp_import *imports;     /**< list of imported modules ([sized array](@ref sizedarrays)) */
    struct lysp_include *includes;   /**< list of included submodules ([sized array](@ref sizedarrays)) */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
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

    uint8_t submodule:1;             /**< flag to distinguish main modules and submodules */
    uint8_t implemented:1;           /**< flag if the module is implemented, not just imported */
    uint8_t latest_revision:2;       /**< flag to mark the latest available revision:
                                          1 - the latest revision in searchdirs was not searched yet and this is the
                                          latest revision in the current context
                                          2 - searchdirs were searched and this is the latest available revision */
    uint8_t parsing:1;               /**< flag for circular check */
    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
    uint16_t refcount;               /**< 0 in modules, number of includes of a submodules */
};

/**
 * @brief Free the printable YANG schema tree structure.
 *
 * @param[in] module Printable YANG schema tree structure to free.
 */
void lysp_module_free(struct lysp_module *module);

/**
 * @brief YANG extension instance
 */
struct lysc_ext_instance {
    struct lyext_plugin *plugin;     /**< pointer to the plugin implementing the extension (if present) */
    void *parent;                    /**< pointer to the parent element holding the extension instance(s), use
                                          ::lysc_ext_instance#parent_type to access the schema element */
    const char *argument;            /**< optional value of the extension's argument */
    LYEXT_SUBSTMT insubstmt;         /**< value identifying placement of the extension instance */
    uint32_t insubstmt_index;        /**< in case the instance is in a substatement that can appear multiple times,
                                          this identifies the index of the substatement for this extension instance */
    LYEXT_PARENT parent_type;        /**< type of the parent structure */
#if 0
    uint8_t ext_type;                /**< extension type (#LYEXT_TYPE) */
    uint8_t padding;                 /**< 32b padding */
    struct lys_module *module;       /**< pointer to the extension instance's module (mandatory) */
    LYS_NODE nodetype;               /**< LYS_EXT */
#endif
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private caller's data, not used by libyang */
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
    struct lyxp_expr *cond;          /**< XPath when condition */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG identity-stmt
 */
struct lysc_ident {
    const char *name;                /**< identity name (mandatory), including possible prefix */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_ident **derived;     /**< list of (pointers to the) derived identities ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/**
 * @brief YANG feature-stmt
 */
struct lysc_feature {
    const char *name;                /**< feature name (mandatory) */
    struct lysc_feature **depfeatures;/**< list of pointers to other features depending on this one ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
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

/**
 * @brief Compiled YANG if-feature-stmt
 */
struct lysc_iffeature {
    uint8_t *expr;                   /**< 2bits array describing the if-feature expression in prefix format, see @ref ifftokens */
    struct lysc_feature **features;  /**< array of pointers to the features used in expression ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Compiled YANG data node
 */
struct lysc_node {
    uint16_t nodetype;               /**< type of the node (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *sp;            /**< link to the simply parsed (SP) original of the node, NULL if the SP schema was removed. */
    struct lysc_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    struct lysc_when *when;          /**< when statement */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Compiled YANG schema tree structure representing YANG module.
 *
 * Semantically validated YANG schema tree for data tree parsing.
 * Contains only the necessary information for the data validation.
 */
struct lysc_module {
    struct ly_ctx *ctx;              /**< libyang context of the module (mandatory) */
    const char *name;                /**< name of the module (mandatory) */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    const char *ns;                  /**< namespace of the module (mandatory) */
    const char *prefix;              /**< module prefix (mandatory) */
    struct lysc_revision *revs;      /**< list of the module revisions ([sized array](@ref sizedarrays)), the first revision
                                          in the list is always the last (newest) revision of the module */
    struct lysc_import *imports;     /**< list of imported modules ([sized array](@ref sizedarrays)) */

    struct lysc_feature *features;   /**< list of feature definitions ([sized array](@ref sizedarrays)) */
    struct lysc_ident *identities;   /**< list of identities ([sized array](@ref sizedarrays)) */
    struct lysc_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysc_action *rpcs;        /**< list of RPCs ([sized array](@ref sizedarrays)) */
    struct lysc_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    uint8_t implemented:1;           /**< flag if the module is implemented, not just imported */
    uint8_t latest_revision:1;       /**< flag if the module was loaded without specific revision and is
                                          the latest revision found */
    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
};

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
 * @brief Available YANG schema tree structures representing YANG module.
 */
struct lys_module {
    struct lysp_module *parsed;      /**< Simply parsed (unresolved) YANG schema tree */
    struct lysc_module *compiled;    /**< Compiled and fully validated YANG schema tree for data parsing */
};

/**
 * @brief Enable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be enabled.
 * @param[in] feature Name of the feature to enable. To enable all features at once, use asterisk (`*`) character.
 * @return LY_ERR value.
 */
LY_ERR lys_feature_enable(struct lys_module *module, const char *feature);

/**
 * @brief Disable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be disabled.
 * @param[in] feature Name of the feature to disable. To disable all features at once, use asterisk (`*`) character.
 * @return LY_ERR value
 */
LY_ERR lys_feature_disable(struct lys_module *module, const char *feature);

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
const struct lys_module *lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format);

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
const struct lys_module *lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format);

/**
 * @brief Read a schema into the specified context from a file.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
const struct lys_module *lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format);

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
 * @defgroup scflags Schema compile flags
 * @ingroup schematree
 *
 * @{
 */
#define LYSC_OPT_FREE_SP 1           /**< Free the input printable schema */

/**
 * @}
 */

/**
 * @brief Compile printable schema into a validated schema linking all the references.
 *
 * @param[in] sp Simple parsed printable schema to compile. Can be changed according to the provided options.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[out] sc Resulting compiled schema structure.
 * @return LY_ERR value.
 */
LY_ERR lys_compile(struct lysp_module *sp, int options, struct lysc_module **sc);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
