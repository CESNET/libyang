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

#include <pcre.h>
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
#define LYS_ANYDATA 0x0120        /**< anydata statement node, in tests it can be used for both #LYS_ANYXML and #LYS_ANYDATA */

#define LYS_CASE 0x0040           /**< case statement node */
#define LYS_USES 0x0080           /**< uses statement node */
#define LYS_INOUT 0x200
#define LYS_ACTION 0x400
#define LYS_NOTIF 0x800
#define LYS_GROUPING 0x1000
#define LYS_AUGMENT 0x2000

/**
 * @brief YANG built-in types
 */
typedef enum {
    LY_TYPE_UNKNOWN = 0,  /**< Unknown type */
    LY_TYPE_BINARY,       /**< Any binary data ([RFC 6020 sec 9.8](http://tools.ietf.org/html/rfc6020#section-9.8)) */
    LY_TYPE_UINT8,        /**< 8-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT16,       /**< 16-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT32,       /**< 32-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT64,       /**< 64-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_STRING,       /**< Human-readable string ([RFC 6020 sec 9.4](http://tools.ietf.org/html/rfc6020#section-9.4)) */
    LY_TYPE_BITS,         /**< A set of bits or flags ([RFC 6020 sec 9.7](http://tools.ietf.org/html/rfc6020#section-9.7)) */
    LY_TYPE_BOOL,         /**< "true" or "false" ([RFC 6020 sec 9.5](http://tools.ietf.org/html/rfc6020#section-9.5)) */
    LY_TYPE_DEC64,        /**< 64-bit signed decimal number ([RFC 6020 sec 9.3](http://tools.ietf.org/html/rfc6020#section-9.3))*/
    LY_TYPE_EMPTY,        /**< A leaf that does not have any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
    LY_TYPE_ENUM,         /**< Enumerated strings ([RFC 6020 sec 9.6](http://tools.ietf.org/html/rfc6020#section-9.6)) */
    LY_TYPE_IDENT,        /**< A reference to an abstract identity ([RFC 6020 sec 9.10](http://tools.ietf.org/html/rfc6020#section-9.10)) */
    LY_TYPE_INST,         /**< References a data tree node ([RFC 6020 sec 9.13](http://tools.ietf.org/html/rfc6020#section-9.13)) */
    LY_TYPE_LEAFREF,      /**< A reference to a leaf instance ([RFC 6020 sec 9.9](http://tools.ietf.org/html/rfc6020#section-9.9))*/
    LY_TYPE_UNION,        /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */
    LY_TYPE_INT8,         /**< 8-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT16,        /**< 16-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT32,        /**< 32-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT64,        /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20 /**< Number of different types */

/**
 * @brief Stringified YANG built-in data types
 */
extern const char* ly_data_type2str[LY_DATA_TYPE_COUNT];

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
 * @brief YANG uses-augment-stmt and augment-stmt
 */
struct lysp_augment {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level augment) */
    uint16_t nodetype;               /**< LYS_AUGMENT */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
    const char *nodeid;              /**< target schema nodeid (mandatory) - absolute for global augments, descendant for uses's augments */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_when *when;          /**< when statement */
    const char **iffeatures;         /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysp_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
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

/**
 * @defgroup spnodeflags Parsed schema nodes flags
 * @ingroup snodeflags
 *
 * Various flags for parsed schema nodes.
 *
 *     1 - container    6 - anydata/anyxml    11 - output       16 - grouping   21 - enum
 *     2 - choice       7 - case              12 - feature      17 - uses       22 - type
 *     3 - leaf         8 - notification      13 - identity     18 - refine
 *     4 - leaflist     9 - rpc               14 - extension    19 - augment
 *     5 - list        10 - input             15 - typedef      20 - deviate
 *
 *                                             1 1 1 1 1 1 1 1 1 1 2 2 2
 *     bit name              1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       1 LYS_CONFIG_W     |x|x|x|x|x|x|x| | | | | | | | | | |x| |x| | |
 *         LYS_SET_BASE     | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       2 LYS_CONFIG_R     |x|x|x|x|x|x|x| | | | | | | | | | |x| |x| | |
 *         LYS_SET_BIT      | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       3 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| |
 *         LYS_SET_ENUM     | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       4 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| |
 *         LYS_SET_FRDIGITS | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       5 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| |
 *         LYS_SET_LENGTH   | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       6 LYS_MAND_TRUE    | |x|x| | |x| | | | | | | | | | | |x| |x| | |
 *         LYS_SET_PATH     | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       7 LYS_MAND_FALSE   | |x|x| | |x| | | | | | | | | | | |x| |x| | |
 *         LYS_ORDBY_USER   | | | |x|x| | | | | | | | | | | | | | | | | |
 *         LYS_SET_PATTERN  | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       8 LYS_ORDBY_SYSTEM | | | |x|x| | | | | | | | | | | | | | | | | |
 *         LYS_YINELEM_TRUE | | | | | | | | | | | | | |x| | | | | | | | |
 *         LYS_SET_RANGE    | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       9 LYS_YINELEM_FALSE| | | | | | | | | | | | | |x| | | | | | | | |
 *         LYS_SET_TYPE     | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      10 LYS_SET_VALUE    | | | | | | | | | | | | | | | | | | | | |x| |
 *         LYS_SET_REQINST  | | | | | | | | | | | | | | | | | | | | | |x|
 *         LYS_SET_MIN      | | | |x|x| | | | | | | | | | | | |x| |x| | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      11 LYS_SET_MAX      | | | |x|x| | | | | | | | | | | | |x| |x| | |
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/**
 * @defgroup scnodeflags Compiled schema nodes flags
 * @ingroup snodeflags
 *
 * Various flags for compiled schema nodes.
 *
 *     1 - container    6 - anydata/anyxml    11 - output
 *     2 - choice       7 - case              12 - feature
 *     3 - leaf         8 - notification      13 - identity
 *     4 - leaflist     9 - rpc               14 - extension
 *     5 - list        10 - input
 *
 *                                             1 1 1 1 1
 *     bit name              1 2 3 4 5 6 7 8 9 0 1 2 3 4
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       1 LYS_CONFIG_W     |x|x|x|x|x|x|x| | |x| | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       2 LYS_CONFIG_R     |x|x|x|x|x|x|x| | | |x| | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       3 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x| | |x|x|x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       4 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x| | |x|x|x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       5 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x| | |x|x|x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       6 LYS_MAND_TRUE    |x|x|x|x|x|x| | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       7 LYS_ORDBY_USER   | | | |x|x| | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       8 LYS_ORDBY_SYSTEM | | | |x|x| | | | | | | | | |
 *         LYS_PRESENCE     |x| | | | | | | | | | | | | |
 *         LYS_UNIQUE       | | |x| | | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       9 LYS_KEY          | | |x| | | | | | | | | | | |
 *         LYS_FENABLED     | | | | | | | | | | | |x| | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      10 LYS_SET_DFLT     | | |x| | | |x| | | | | | | |
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
#define LYS_MAND_FALSE   0x40        /**< mandatory false; applicable only to ::lysp_node_choice, ::lysp_node_leaf and ::lysp_node_anydata */
#define LYS_MAND_MASK    0x60        /**< mask for mandatory values */
#define LYS_PRESENCE     0x80        /**< flag for presence property of a container, applicable only to ::lysc_node_container */
#define LYS_UNIQUE       0x80        /**< flag for leafs being part of a unique set, applicable only to ::lysc_node_leaf */
#define LYS_KEY          0x100       /**< flag for leafs being a key of a list, applicable only to ::lysc_node_leaf */
#define LYS_FENABLED     0x100       /**< feature enabled flag, applicable only to ::lysc_feature */
#define LYS_ORDBY_SYSTEM 0x80        /**< ordered-by user lists, applicable only to ::lysc_node_leaflist/::lysp_node_leaflist and
                                          ::lysc_node_list/::lysp_node_list */
#define LYS_ORDBY_USER   0x40        /**< ordered-by user lists, applicable only to ::lysc_node_leaflist/::lysp_node_leaflist and
                                          ::lysc_node_list/::lysp_node_list */
#define LYS_ORDBY_MASK   0x60        /**< mask for ordered-by values */
#define LYS_YINELEM_TRUE 0x80        /**< yin-element true for extension's argument */
#define LYS_YINELEM_FALSE 0x100      /**< yin-element false for extension's argument */
#define LYS_YINELEM_MASK 0x180       /**< mask for yin-element value */
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
#define LYS_SET_DFLT     0x0200      /**< flag to mark leaf with own (or refined) default value, not a default value taken from its type, and default
                                          cases of choice. This information is important for refines, since it is prohibited to make leafs
                                          with default statement mandatory. In case the default leaf value is taken from type, it is thrown
                                          away when it is refined to be mandatory node. */

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
    struct lysp_action_inout *input; /**< RPC's/Action's input */
    struct lysp_action_inout *output;/**< RPC's/Action's output */
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
    struct lyxp_expr *cond;          /**< XPath when condition */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_node *context;       /**< context node for evaluating the expression */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint32_t refcount;               /**< reference counter since some of the when statements are shared among several nodes */
};

/**
 * @brief YANG identity-stmt
 */
struct lysc_ident {
    const char *name;                /**< identity name (mandatory), including possible prefix */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
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
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
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

struct lysc_range {
    struct lysc_range_part {
        union {                      /**< min boundary TODO decimal */
            int64_t min_64;          /**< for int8, int16, int32 and int64 */
            uint64_t min_u64;        /**< for uint8, uint16, uint32 and uint64 */
        };
        union {                      /**< max boundary TODO decimal */
            int64_t max_64;          /**< for int8, int16, int32 and int64 */
            uint64_t max_u64;        /**< for uint8, uint16, uint32 and uint64 */
        };
    } *parts;                        /**< compiled range expression ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_pattern {
    pcre *expr;                      /**< compiled regular expression */
    pcre_extra *expr_extra;          /**< additional information to speed up matching */
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
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
};

struct lysc_type_num {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_dec {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    uint8_t fraction_digits;         /**< fraction digits specification */
    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_str {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *length;       /**< Optional length limitation */
    struct lysc_pattern **patterns;  /**< Optional list of pointers to pattern limitations ([sized array](@ref sizedarrays)) */
};

struct lysc_type_enum {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type_enum_item {
        const char *name;            /**< enumeration identifier */
        const char *dsc;             /**< description */
        const char *ref;             /**< reference */
        struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
        struct lysc_ext_instance *exts;    /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        int32_t value;               /**< integer value associated with the enumeration */
    } *enums;                        /**< enumerations list ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_leafref {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    const char* path;                /**< target path */
    struct lys_module *path_context; /**< module where the path is defined, so it provides context to resolve prefixes */
    struct lysc_type *realtype;      /**< pointer to the real (first non-leafref in possible leafrefs chain) type. */
    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_identityref {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_ident **bases;       /**< list of pointers to the base identities ([sized array](@ref sizedarrays)),
                                          mandatory (at least 1 item) */
};

struct lysc_type_instanceid {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_bits {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type_bits_item {
        const char *name;            /**< bit identifier */
        const char *dsc;             /**< description */
        const char *ref;             /**< reference */
        struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
        struct lysc_ext_instance *exts;    /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        uint32_t position;           /**< non-negative integer value associated with the bit */
    } *bits;                         /**< bits list ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_union {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_type **types;        /**< list of types in the union ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_bin {
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *dflt;                /**< type's default value if any */
    LY_DATA_TYPE basetype;           /**< Base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */
    struct lysc_range *length;       /**< Optional length limitation */
};

struct lysc_action {
    uint16_t nodetype;               /**< LYS_ACTION */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    const char *name;                /**< action/RPC name (mandatory) */
    /* TODO */
};

struct lysc_notif {
    uint16_t nodetype;               /**< LYS_NOTIF */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lys_module *module;       /**< module structure */
    const char *name;                /**< Notification name (mandatory) */
    /* TODO */
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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

    struct lysc_node *child;         /**< first child node of the case (linked list). Note that all the children of all the sibling cases are linked
                                          each other as siblings with the parent pointer pointing to the choice node holding the case. To distinguish
                                          which children node belongs to which case, it is needed to match the first children of the cases while going
                                          through the children linked list. */
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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    const char *dflt;                /**< default value */
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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    const char **dflts;              /**< list of default values ([sized array](@ref sizedarrays)) */
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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_action *actions;     /**< list of actions ([sized array](@ref sizedarrays)) */
    struct lysc_notif *notifs;       /**< list of notifications ([sized array](@ref sizedarrays)) */

    struct lysc_node_leaf **keys;    /**< list of pointers to the keys ([sized array](@ref sizedarrays)) */
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
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */

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
 * @return The node's children linked list if any, NULL otherwise.
 */
const struct lysc_node *lysc_node_children(const struct lysc_node *node);

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
const struct lysc_node *lys_child(const struct lysc_node *parent, const struct lys_module *module,
                                  const char *name, size_t name_len, uint16_t nodetype, int options);

/**
 * @brief Check if the schema node is disabled in the schema tree, i.e. there is any disabled if-feature statement
 * affecting the node.
 *
 * @param[in] node Schema node to check.
 * @param[in] recursive - 0 to check if-feature only in the \p node schema node,
 * - 1 to check if-feature in all ascendant schema nodes until there is a node possibly having an instance in a data tree
 * @return - NULL if enabled,
 * - pointer to the node with the unsatisfied (disabling) if-feature expression.
 */
const struct lysc_iffeature *lys_is_disabled(const struct lysc_node *node, int recursive);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
