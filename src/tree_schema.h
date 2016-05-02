/**
 * @file tree_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of data model trees.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_H_
#define LY_TREE_SCHEMA_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup datatree
 * @brief Macro to iterate via all sibling elements without affecting the list itself
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type.
 *
 * Use with opening curly bracket '{'. All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param ELEM Iterator.
 */
#define LY_TREE_FOR(START, ELEM) \
    for ((ELEM) = (START); \
         (ELEM); \
         (ELEM) = (ELEM)->next)

/**
 * @ingroup datatree
 * @brief Macro to iterate via all sibling elements allowing to modify the list itself (e.g. removing elements)
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type.
 *
 * Use with opening curly bracket '{'. All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param NEXT Temporary storage to allow removing of the current iterator content.
 * @param ELEM Iterator.
 */
#define LY_TREE_FOR_SAFE(START, NEXT, ELEM) \
    for ((ELEM) = (START); \
         (ELEM) ? (NEXT = (ELEM)->next, 1) : 0; \
         (ELEM) = (NEXT))

/**
 * @ingroup datatree
 * @brief Macro to iterate via all elements in a tree. This is the opening part
 * to the #LY_TREE_DFS_END - they always have to be used together.
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
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type. Use the same parameters for
 * #LY_TREE_DFS_BEGIN and #LY_TREE_DFS_END.
 *
 * Since the next node is selected as part of #LY_TREE_DFS_END, do not use
 * continue statement between the #LY_TREE_DFS_BEGIN and #LY_TREE_DFS_BEGIN.
 *
 * Use with opening curly bracket '{' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param NEXT Temporary storage, do not use.
 * @param ELEM Iterator intended for use in the block.
 */
#define LY_TREE_DFS_BEGIN(START, NEXT, ELEM)                                  \
    for ((ELEM) = (NEXT) = (START);                                           \
         (ELEM);                                                              \
         (ELEM) = (NEXT))

/**
 * @ingroup datatree
 * @brief Macro to iterate via all elements in a tree. This is the closing part
 * to the #LY_TREE_DFS_BEGIN - they always have to be used together.
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type - basic type of the tree (struct
 * lys_node*, struct lyd_node* or struct lyxml_elem*). Use the same parameters for
 * #LY_TREE_DFS_BEGIN and #LY_TREE_DFS_END.
 *
 * Use with closing curly bracket '}' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param NEXT Temporary storage, do not use.
 * @param ELEM Iterator intended for use in the block.
 */
#define LY_TREE_DFS_END(START, NEXT, ELEM)                                    \
    /* select element for the next run - children first */                    \
    (NEXT) = (ELEM)->child;                                                   \
    if (sizeof(typeof(*(START))) == sizeof(struct lyd_node)) {                \
        /* child exception for leafs, leaflists and anyxml without children */\
        if (((struct lyd_node *)(ELEM))->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) { \
            (NEXT) = NULL;                                                    \
        }                                                                     \
    } else if (sizeof(typeof(*(START))) == sizeof(struct lys_node)) {         \
        /* child exception for leafs, leaflists and anyxml without children */\
        if (((struct lys_node *)(ELEM))->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) { \
            (NEXT) = NULL;                                                    \
        }                                                                     \
    }                                                                         \
    if (!(NEXT)) {                                                            \
        /* no children */                                                     \
        if ((ELEM) == (START)) {                                              \
            /* we are done, (START) has no children */                        \
            break;                                                            \
        }                                                                     \
        /* try siblings */                                                    \
        (NEXT) = (ELEM)->next;                                                \
    }                                                                         \
    while (!(NEXT)) {                                                         \
        /* parent is already processed, go to its sibling */                  \
        if ((sizeof(typeof(*(START))) == sizeof(struct lys_node))             \
                && (((struct lys_node *)(ELEM)->parent)->nodetype == LYS_AUGMENT)) {  \
            (ELEM) = (ELEM)->parent->prev;                                    \
        } else {                                                              \
            (ELEM) = (ELEM)->parent;                                          \
        }                                                                     \
        /* no siblings, go back through parents */                            \
        if (sizeof(typeof(*(START))) == sizeof(struct lys_node)) {            \
            /* due to possible augments */                                    \
            if (lys_parent((struct lys_node *)(ELEM)) == lys_parent((struct lys_node *)(START))) { \
                /* we are done, no next element to process */                 \
                break;                                                        \
            }                                                                 \
        } else if ((ELEM)->parent == (START)->parent) {                       \
            /* we are done, no next element to process */                     \
            break;                                                            \
        }                                                                     \
        (NEXT) = (ELEM)->next;                                                \
    }

/**
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 */

#define LY_REV_SIZE 11   /**< revision data string length (including terminating NULL byte) */

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
} LYS_OUTFORMAT;

/* shortcuts for common in and out formats */
#define LYS_YANG 1       /**< YANG schema format, used for #LYS_INFORMAT and #LYS_OUTFORMAT */
#define LY_YIN 2         /**< YIN schema format, used for #LYS_INFORMAT and #LYS_OUTFORMAT */

/**
 * @brief YANG schema node types
 *
 * Values are defined as separated bit values to allow checking using bitwise operations for multiple nodes.
 */
typedef enum lys_nodetype {
    LYS_UNKNOWN = 0x0000,        /**< uninitalized unknown statement node */
    LYS_CONTAINER = 0x0001,      /**< container statement node */
    LYS_CHOICE = 0x0002,         /**< choice statement node */
    LYS_LEAF = 0x0004,           /**< leaf statement node */
    LYS_LEAFLIST = 0x0008,       /**< leaf-list statement node */
    LYS_LIST = 0x0010,           /**< list statement node */
    LYS_ANYXML = 0x0020,         /**< anyxml statement node */
    LYS_CASE = 0x0040,           /**< case statement node */
    LYS_NOTIF = 0x0080,          /**< notification statement node */
    LYS_RPC = 0x0100,            /**< rpc statement node */
    LYS_INPUT = 0x0200,          /**< input statement node */
    LYS_OUTPUT = 0x0400,         /**< output statement node */
    LYS_GROUPING = 0x0800,       /**< grouping statement node */
    LYS_USES = 0x1000,           /**< uses statement node */
    LYS_AUGMENT = 0x2000         /**< augment statement node */
} LYS_NODE;

/* all nodes sharing the node namespace except RPCs and notifications */
#define LYS_NO_RPC_NOTIF_NODE 0x007F

#define LYS_ANY 0x3FFF

/**
 * @brief Main schema node structure representing YANG module.
 *
 * Compatible with ::lys_submodule structure with exception of the last, #ns member, which is replaced by
 * ::lys_submodule#belongsto member. Sometimes, ::lys_submodule can be provided casted to ::lys_module. Such a thing
 * can be determined via the #type member value.
 *
 *
 */
struct lys_module {
    struct ly_ctx *ctx;              /**< libyang context of the module (mandatory) */
    const char *name;                /**< name of the module (mandatory) */
    const char *prefix;              /**< prefix of the module (mandatory) */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    uint8_t type:1;                  /**< 0 - structure type used to distinguish structure from ::lys_submodule */
    uint8_t version:5;               /**< yang-version:
                                          - 0 = not specified, YANG 1.0 as default,
                                          - 1 = YANG 1.0,
                                          - 2 = YANG 1.1 not yet supported */
    uint8_t deviated:1;              /**< deviated flag (true/false) if the module is deviated by some other module */
    uint8_t implemented:1;           /**< flag if the module is implemented, not just imported */

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in #rev array */
    uint8_t imp_size;                /**< number of elements in #imp array */
    uint8_t inc_size;                /**< number of elements in #inc array */
    uint8_t tpdf_size;               /**< number of elements in #tpdf array */
    uint32_t ident_size;             /**< number of elements in #ident array */
    uint8_t features_size;           /**< number of elements in #features array */
    uint8_t augment_size;            /**< number of elements in #augment array */
    uint8_t deviation_size;          /**< number of elements in #deviation array */

    struct lys_revision *rev;        /**< array of the module revisions, revisions[0] is always the last (newest)
                                          revision of the module */
    struct lys_import *imp;          /**< array of imported modules */
    struct lys_include *inc;         /**< array of included submodules */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_ident *ident;         /**< array of identities */
    struct lys_feature *features;    /**< array of feature definitions */
    struct lys_node_augment *augment;/**< array of augments */
    struct lys_deviation *deviation; /**< array of specified deviations */

    /* specific module's items in comparison to submodules */
    struct lys_node *data;           /**< first data statement, includes also RPCs and Notifications */
    const char *ns;                  /**< namespace of the module (mandatory) */
};

/**
 * @brief Submodule schema node structure that can be included into a YANG module.
 *
 * Compatible with ::lys_module structure with exception of the last, #belongsto member, which is replaced by
 * ::lys_module#data and ::lys_module#ns members. Sometimes, ::lys_submodule can be provided casted to ::lys_module.
 * Such a thing can be determined via the #type member value.
 *
 */
struct lys_submodule {
    struct ly_ctx *ctx;              /**< libyang context of the submodule (mandatory) */
    const char *name;                /**< name of the submodule (mandatory) */
    const char *prefix;              /**< prefix of the belongs-to module */
    const char *dsc;                 /**< description of the submodule */
    const char *ref;                 /**< cross-reference for the submodule */
    const char *org;                 /**< party responsible for the submodule */
    const char *contact;             /**< contact information for the submodule */
    const char *filepath;            /**< path to the file from which the submodule was read */
    uint8_t type:1;                  /**< 1 - structure type used to distinguish structure from ::lys_module */
    uint8_t padding:7;               /**< not used, kept for compatibility with ::lys_module */

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in #rev array */
    uint8_t imp_size;                /**< number of elements in #imp array */
    uint8_t inc_size;                /**< number of elements in #inc array */
    uint8_t tpdf_size;               /**< number of elements in #tpdf array */
    uint32_t ident_size;             /**< number of elements in #ident array */
    uint8_t features_size;           /**< number of elements in #features array */
    uint8_t augment_size;            /**< number of elements in #augment array */
    uint8_t deviation_size;          /**< number of elements in #deviation array */

    struct lys_revision *rev;        /**< array of the module revisions, revisions[0] is always the last (newest)
                                          revision of the submodule */
    struct lys_import *imp;          /**< array of imported modules */
    struct lys_include *inc;         /**< array of included submodules */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_ident *ident;         /**< array if identities */
    struct lys_feature *features;    /**< array of feature definitions */
    struct lys_node_augment *augment;/**< array of augments */
    struct lys_deviation *deviation; /**< array of specified deviations */

    /* specific submodule's items in comparison to modules */
    struct lys_module *belongsto;    /**< belongs-to (parent module) */
};

/**
 * @brief YANG built-in types
 */
typedef enum {
    LY_TYPE_DER = 0,     /**< Derived type */
    LY_TYPE_BINARY,      /**< Any binary data ([RFC 6020 sec 9.8](http://tools.ietf.org/html/rfc6020#section-9.8)) */
    LY_TYPE_BITS,        /**< A set of bits or flags ([RFC 6020 sec 9.7](http://tools.ietf.org/html/rfc6020#section-9.7)) */
    LY_TYPE_BOOL,        /**< "true" or "false" ([RFC 6020 sec 9.5](http://tools.ietf.org/html/rfc6020#section-9.5)) */
    LY_TYPE_DEC64,       /**< 64-bit signed decimal number ([RFC 6020 sec 9.3](http://tools.ietf.org/html/rfc6020#section-9.3))*/
    LY_TYPE_EMPTY,       /**< A leaf that does not have any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
    LY_TYPE_ENUM,        /**< Enumerated strings ([RFC 6020 sec 9.6](http://tools.ietf.org/html/rfc6020#section-9.6)) */
    LY_TYPE_IDENT,       /**< A reference to an abstract identity ([RFC 6020 sec 9.10](http://tools.ietf.org/html/rfc6020#section-9.10)) */
    LY_TYPE_INST,        /**< References a data tree node ([RFC 6020 sec 9.13](http://tools.ietf.org/html/rfc6020#section-9.13)) */
    LY_TYPE_LEAFREF,     /**< A reference to a leaf instance ([RFC 6020 sec 9.9](http://tools.ietf.org/html/rfc6020#section-9.9))*/
    LY_TYPE_STRING,      /**< Human-readable string ([RFC 6020 sec 9.4](http://tools.ietf.org/html/rfc6020#section-9.4)) */
    LY_TYPE_UNION,       /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */
    LY_TYPE_INT8,        /**< 8-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT8,       /**< 8-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT16,       /**< 16-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT16,      /**< 16-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT32,       /**< 32-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT32,      /**< 32-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT64,       /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT64,      /**< 64-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20        /**< number of #LY_DATA_TYPE built-in types */
#define LY_DATA_TYPE_MASK 0x3f       /**< mask for valid type values, 2 bits are reserver for #LY_TYPE_LEAFREF_UNRES and
                                          #LY_TYPE_INST_UNRES in case of parsing with #LYD_OPT_EDIT options. */
#define LY_TYPE_LEAFREF_UNRES 0x40   /**< flag for unresolved leafref, the rest of bits store the target node's type */
#define LY_TYPE_INST_UNRES 0x80      /**< flag for unresolved instance-identifier, always use in conjunction with LY_TYPE_INST */

/**
 *
 */
struct lys_type_info_binary {
    struct lys_restr *length;    /**< length restriction (optional), see
                                      [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4) */
};

/**
 * @brief Single bit value specification for ::lys_type_info_bits.
 */
struct lys_type_bit {
    const char *name;    /**< bit's name (mandatory) */
    const char *dsc;     /**< bit's description (optional) */
    const char *ref;     /**< bit's reference (optional) */
    uint8_t flags;       /**< bit's flags, whether the position was auto-assigned
                              and the status(one of LYS_NODE_STATUS_* values or 0 for default) */
    uint32_t pos;        /**< bit's position (mandatory) */
};

/**
 * @brief Container for information about bits types (#LY_TYPE_BINARY), used in ::lys_type_info.
 */
struct lys_type_info_bits {
    struct lys_type_bit *bit;/**< array of bit definitions */
    int count;               /**< number of bit definitions in the bit array */
} bits;                      /**< part for #LY_TYPE_BITS */

/**
 * @brief Container for information about decimal64 types (#LY_TYPE_DEC64), used in ::lys_type_info.
 */
struct lys_type_info_dec64 {
    struct lys_restr *range; /**< range restriction (optional), see
                                  [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4) */
    uint8_t dig;             /**< fraction-digits restriction (mandatory) */
};

/**
 * @brief Single enumeration value specification for ::lys_type_info_enums.
 */
struct lys_type_enum {
    const char *name;        /**< enum's name (mandatory) */
    const char *dsc;         /**< enum's description (optional) */
    const char *ref;         /**< enum's reference (optional) */
    uint8_t flags;           /**< enum's flags, whether the value was auto-assigned
                                  and the status(one of LYS_NODE_STATUS_* values or 0 for default) */
    int32_t value;           /**< enum's value (mandatory) */
};

/**
 * @brief Container for information about enumeration types (#LY_TYPE_ENUM), used in ::lys_type_info.
 */
struct lys_type_info_enums {
    struct lys_type_enum *enm;/**< array of enum definitions */
    int count;               /**< number of enum definitions in the enm array */
};

/**
 * @brief Container for information about identity types (#LY_TYPE_IDENT), used in ::lys_type_info.
 */
struct lys_type_info_ident {
    struct lys_ident *ref;   /**< pointer (reference) to the identity definition (mandatory) */
};

/**
 * @brief Container for information about instance-identifier types (#LY_TYPE_INST), used in ::lys_type_info.
 */
struct lys_type_info_inst {
    int8_t req;              /**< require-identifier restriction, see
                                  [RFC 6020 sec. 9.13.2](http://tools.ietf.org/html/rfc6020#section-9.13.2):
                                  - -1 = false,
                                  - 0 not defined,
                                  - 1 = true */
};

/**
 * @brief Container for information about integer types, used in ::lys_type_info.
 */
struct lys_type_info_num {
    struct lys_restr *range; /**< range restriction (optional), see
                                  [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4) */
};

/**
 * @brief Container for information about leafref types (#LY_TYPE_LEAFREF), used in ::lys_type_info.
 */
struct lys_type_info_lref {
    const char *path;        /**< path to the referred leaf or leaf-list node (mandatory), see
                                  [RFC 6020 sec. 9.9.2](http://tools.ietf.org/html/rfc6020#section-9.9.2) */
    struct lys_node_leaf* target; /**< target schema node according to path */
};

/**
 * @brief Container for information about string types (#LY_TYPE_STRING), used in ::lys_type_info.
 */
struct lys_type_info_str {
    struct lys_restr *length;/**< length restriction (optional), see
                                  [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4) */
    struct lys_restr *patterns; /**< array of pattern restrictions (optional), see
                                  [RFC 6020 sec. 9.4.6](http://tools.ietf.org/html/rfc6020#section-9.4.6) */
    int pat_count;           /**< number of pattern definitions in the patterns array */
};

/**
 * @brief Container for information about union types (#LY_TYPE_UNION), used in ::lys_type_info.
 */
struct lys_type_info_union {
    struct lys_type *types;  /**< array of union's subtypes */
    int count;               /**< number of subtype definitions in types array */
};

/**
 * @brief Union for holding type-specific information in ::lys_type.
 */
union lys_type_info {
    struct lys_type_info_binary binary; /**< part for #LY_TYPE_BINARY */
    struct lys_type_info_bits bits;     /**< part for #LY_TYPE_BITS */
    struct lys_type_info_dec64 dec64;   /**< part for #LY_TYPE_DEC64 */
    struct lys_type_info_enums enums;   /**< part for #LY_TYPE_ENUM */
    struct lys_type_info_ident ident;   /**< part for #LY_TYPE_IDENT */
    struct lys_type_info_inst inst;     /**< part for #LY_TYPE_INST */
    struct lys_type_info_num num;       /**< part for integer types */
    struct lys_type_info_lref lref;     /**< part for #LY_TYPE_LEAFREF */
    struct lys_type_info_str str;       /**< part for #LY_TYPE_STRING */
    struct lys_type_info_union uni;     /**< part for #LY_TYPE_UNION */
};

/**
 * @brief YANG type structure providing information from the schema
 */
struct lys_type {
    const char *module_name;         /**< module name of the type referenced in der pointer*/
    LY_DATA_TYPE base;               /**< base type */
    struct lys_tpdf *der;            /**< pointer to the superior typedef. If NULL,
                                          structure provides information about one of the built-in types */
    struct lys_tpdf *parent;         /**< except ::lys_tpdf, it can points also to ::lys_node_leaf or ::lys_node_leaflist
                                          so access only the compatible members! */
    union lys_type_info info;        /**< detailed type-specific information */
    /*
     * here is an overview of the info union:
     * LY_TYPE_BINARY (binary)
     * struct lys_restr *binary.length;   length restriction (optional), see
     *                                    [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4)
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_BITS (bits)
     * struct lys_type_bit bits.bit;      array of bit definitions
     *   const char *bits.bit[i].name;    bit's name (mandatory)
     *   const char *bits.bit[i].dsc;     bit's description (optional)
     *   const char *bits.bit[i].ref;     bit's reference (optional)
     *   uint8_t bits.bit[i].flags;       bit's flags, whether the position was auto-assigned
     *                                    and the status(one of LYS_NODE_STATUS_* values or 0 for default)
     *   uint32_t bits.bit[i].pos;        bit's position (mandatory)
     * int bits.count;                    number of bit definitions in the bit array
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_DEC64 (dec64)
     * struct lys_restr *dec64.range;     range restriction (optional), see
     *                                    [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4)
     * uint8_t dec64.dig;                 fraction-digits restriction (mandatory)
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_ENUM (enums)
     * struct lys_type_enum *enums.enm;   array of enum definitions
     *   const char *enums.enm[i].name;   enum's name (mandatory)
     *   const char *enums.enm[i].dsc;    enum's description (optional)
     *   const char *enums.enm[i].ref;    enum's reference (optional)
     *   uint8_t enums.enm[i].flags;      enum's flags, whether the value was auto-assigned
     *                                    and the status(one of LYS_NODE_STATUS_* values or 0 for default)
     *   int32_t enums.enm[i].value;      enum's value (mandatory)
     * int enums.count;                   number of enum definitions in the enm array
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_IDENT (ident)
     * struct lys_ident *ident.ref;       pointer (reference) to the identity definition (mandatory)
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_INST (inst)
     * int8_t inst.req;                   require-identifier restriction, see
     *                                    [RFC 6020 sec. 9.13.2](http://tools.ietf.org/html/rfc6020#section-9.13.2):
     *                                    - -1 = false,
     *                                    - 0 not defined,
     *                                    - 1 = true
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_*INT* (num)
     * struct lys_restr *num.range;       range restriction (optional), see
     *                                    [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4)
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_LEAFREF (lref)
     * const char *lref.path;             path to the referred leaf or leaf-list node (mandatory), see
     *                                    [RFC 6020 sec. 9.9.2](http://tools.ietf.org/html/rfc6020#section-9.9.2)
     * struct lys_node_leaf *lref.target; target schema node according to path
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_STRING (str)
     * struct lys_restr *str.length;      length restriction (optional), see
     *                                    [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4)
     * struct lys_restr *str.patterns;    array of pattern restrictions (optional), see
     *                                    [RFC 6020 sec. 9.4.6](http://tools.ietf.org/html/rfc6020#section-9.4.6)
     * int str.pat_count;                 number of pattern definitions in the patterns array
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_UNION (uni)
     * struct lys_type *uni.types;        array of union's subtypes
     * int uni.count;                     number of subtype definitions in types array
     */
};

/**
 * @defgroup nacmflags NACM flags
 * @ingroup schematree
 *
 * Flags to support NACM YANG extensions following the [RFC 6536](https://tools.ietf.org/html/rfc6536)
 *
 * @{
 */
#define LYS_NACM_DENYW   0x01        /**< default-deny-write extension used */
#define LYS_NACM_DENYA   0x02        /**< default-deny-all extension used */
/**
 * @}
 */

/**
 * @defgroup snodeflags Schema nodes flags
 * @ingroup schematree
 *
 * Various flags for schema nodes.
 *
 *     1 - container    6 - anyxml            11 - rpc output   16 - type(def)
 *     2 - choice       7 - case              12 - grouping     17 - identity
 *     3 - leaf         8 - notification      13 - uses         18 - refine
 *     4 - leaflist     9 - rpc               14 - augment
 *     5 - list        10 - rpc input         15 - feature
 *
 *                                           1 1 1 1 1 1 1 1 1
 *                         1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
 *     -------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     1 LYS_USESGRP      | | | | | | | | | | | | |x| | | | | |
 *       LYS_AUTOASSIGNED | | | | | | | | | | | | | | | |x| | |
 *       LYS_CONFIG_W     |x|x|x|x|x|x| | | | | | | | | | | |x|
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     2 LYS_CONFIG_R     |x|x|x|x|x|x| | | | | | | | | | | |x|
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     3 LYS_CONFIG_SET   |x|x|x|x|x|x| | | | | | | | | | | | |
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     4 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |
 *       LYS_RFN_MAXSET   | | | | | | | | | | | | | | | | | |x|
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     5 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |
 *       LYS_RFN_MINSET   | | | | | | | | | | | | | | | | | |x|
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     6 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     7 LYS_MAND_TRUE    | |x|x| | |x| | | | | | | | | | | |x|
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     8 LYS_MAND_FALSE   | |x|x| | |x| | | | | | | | | | | |x|
 *       LYS_INCL_STATUS  |x| | | |x| | | | | | | | | | | | | |
 *                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     9 LYS_USERORDERED  | | | |x|x| | | | | | | | | | | | | |
 *       LYS_UNIQUE       | | |x| | | | | | | | | | | | | | | |
 *       LYS_FENABLED     | | | | | | | | | | | | | | |x| | | |
 *
 * @{
 */
#define LYS_CONFIG_W     0x01        /**< config true; */
#define LYS_CONFIG_R     0x02        /**< config false; */
#define LYS_CONFIG_SET   0x04        /**< config explicitely set in the node */
#define LYS_CONFIG_MASK  0x03        /**< mask for config value */
#define LYS_STATUS_CURR  0x08        /**< status current; */
#define LYS_STATUS_DEPRC 0x10        /**< status deprecated; */
#define LYS_STATUS_OBSLT 0x20        /**< status obsolete; */
#define LYS_STATUS_MASK  0x38        /**< mask for status value */
#define LYS_RFN_MAXSET   0x08        /**< refine has max-elements set */
#define LYS_RFN_MINSET   0x10        /**< refine has min-elements set */
#define LYS_MAND_TRUE    0x40        /**< mandatory true; applicable only to
                                          ::lys_node_choice, ::lys_node_leaf and ::lys_node_anyxml */
#define LYS_MAND_FALSE   0x80        /**< mandatory false; applicable only to
                                          ::lys_node_choice, ::lys_node_leaf and ::lys_node_anyxml */
#define LYS_INCL_STATUS  0x80        /**< flag that the subtree includes status node(s), applicable only to
                                          ::lys_node_container and lys_node_list */
#define LYS_MAND_MASK    0xc0        /**< mask for mandatory values */
#define LYS_USERORDERED  0x100       /**< ordered-by user lists, applicable only to
                                          ::lys_node_list and ::lys_node_leaflist */
#define LYS_FENABLED     0x100       /**< feature enabled flag, applicable only to ::lys_feature */
#define LYS_UNIQUE       0x100       /**< part of the list's unique, applicable only to ::lys_node_leaf */
#define LYS_AUTOASSIGNED 0x01        /**< value was auto-assigned, applicable only to
                                          ::lys_type enum and bits flags */
#define LYS_USESGRP      0x01        /**< flag for resolving uses in groupings, applicable only to ::lys_node_uses */
/**
 * @}
 */

/**
 * @brief Common structure representing single YANG data statement describing.
 *
 * This is a common structure to allow having a homogeneous tree of nodes despite the nodes are actually
 * heterogeneous. It allow one to go through the tree in a simple way. However, if you want to work with
 * the node in some way or get more appropriate information, you are supposed to cast it to the appropriate
 * lys_node_* structure according to the #nodetype value.
 *
 * To traverse through all the child elements, use #LY_TREE_FOR or #LY_TREE_FOR_SAFE macro.
 *
 * To cover all possible schema nodes, the ::lys_node type is used in ::lyd_node#schema for referencing schema
 * definition for a specific data node instance.
 *
 * The #priv member is completely out of libyang control. It is just a pointer to allow libyang
 * caller to store some proprietary data (e.g. callbacks) connected with the specific schema tree node.
 */
struct lys_node {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */
};

/**
 * @brief Schema container node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when,
 * #presence, #must_size, #tpdf_size, #must and #tpdf members.
 *
 * The container schema node can be instantiated in the data tree, so the ::lys_node_container can be directly
 * referenced from ::lyd_node#schema.
 */
struct lys_node_container {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_CONTAINER */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific container's data */
    struct lys_when *when;           /**< when statement (optional) */
    const char *presence;            /**< presence description, used also as a presence flag (optional) */

    uint8_t must_size;               /**< number of elements in the #must array */
    uint8_t tpdf_size;               /**< number of elements in the #tpdf array */

    struct lys_restr *must;          /**< array of must constraints */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
};

/**
 * @brief Schema choice node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when and
 * #dflt members.
 *
 * The choice schema node has no instance in the data tree, so the ::lys_node_choice cannot be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_choice {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_CHOICE */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific choice's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_node *dflt;           /**< default case of the choice (optional) */
};

/**
 * @brief Schema leaf node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #type,
 * #units, #must_size, #must and #dflt members. In addition, the structure is compatible with the ::lys_node_leaflist
 * structure except the last #dflt member, which is replaced by ::lys_node_leaflist#min and ::lys_node_leaflist#max
 * members.
 *
 * ::lys_node_leaf is terminating node in the schema tree, so the #child member value is always NULL.
 *
 * The leaf schema node can be instantiated in the data tree, so the ::lys_node_leaf can be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_leaf {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_LEAF */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< always NULL except the leaf/leaflist is target of a leafref, in that case
                                          the pointer stores set of ::lys_node leafref objects with path referencing
                                          the current ::lys_node_leaf */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific leaf's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_type type;            /**< YANG data type definition of the leaf (mandatory) */
    const char *units;               /**< units of the data type (optional) */

    uint8_t must_size;               /**< number of elements in the #must array */
    struct lys_restr *must;          /**< array of must constraints */

    /* to this point, struct lys_node_leaf is compatible with struct lys_node_leaflist */
    const char *dflt;                /**< default value of the type */
};

/**
 * @brief Schema leaf-list node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #type,
 * #units, #must_size, #must, #min and #max members. In addition, the structure is compatible with the ::lys_node_leaf
 * structure except the last #min and #max members, which are replaced by ::lys_node_leaf#dflt member.
 *
 * ::lys_node_leaflist is terminating node in the schema tree, so the #child member value is always NULL.
 *
 * The leaf-list schema node can be instantiated in the data tree, so the ::lys_node_leaflist can be directly
 * referenced from ::lyd_node#schema.
 */
struct lys_node_leaflist {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_LEAFLIST */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< always NULL except the leaf/leaflist is target of a leafref, in that case
                                          the pointer stores set of ::lys_node leafref objects with path referencing
                                          the current ::lys_node_leaflist */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific leaf-list's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_type type;            /**< YANG data type definition of the leaf (mandatory) */
    const char *units;               /**< units of the data type (optional) */

    uint8_t must_size;               /**< number of elements in the #must array */
    struct lys_restr *must;          /**< array of must constraints */

    /* to this point, struct lys_node_leaflist is compatible with struct lys_node_leaf */
    uint32_t min;                    /**< min-elements constraint (optional) */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded (optional) */
};

/**
 * @brief Schema list node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #min,
 * #max, #must_size, #tpdf_size, #keys_size, #unique_size, #must, #tpdf, #keys and #unique members.
 *
 * The list schema node can be instantiated in the data tree, so the ::lys_node_list can be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_list {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_LIST */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific list's data */
    struct lys_when *when;           /**< when statement (optional) */

    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */

    uint8_t must_size;               /**< number of elements in the #must array */
    uint8_t tpdf_size;               /**< number of elements in the #tpdf array */
    uint8_t keys_size;               /**< number of elements in the #keys array */
    uint8_t unique_size;             /**< number of elements in the #unique array (number of unique statements) */

    struct lys_restr *must;          /**< array of must constraints */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_node_leaf **keys;     /**< array of pointers to the key nodes */
    struct lys_unique *unique;       /**< array of unique statement structures */
};

/**
 * @brief Schema anyxml node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #must_size
 * and #must members.
 *
 * ::lys_node_anyxml is terminating node in the schema tree, so the #child member value is always NULL.
 *
 * The anyxml schema node can be instantiated in the data tree, so the ::lys_node_anyxml can be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_anyxml {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_ANYXML */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< always NULL */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific anyxml's data */
    struct lys_when *when;           /**< when statement (optional) */
    uint8_t must_size;               /**< number of elements in the #must array */
    struct lys_restr *must;          /**< array of must constraints */
};

/**
 * @brief Schema uses node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #grp,
 * #refine_size, #augment_size, #refine and #augment members.
 *
 * ::lys_node_uses is terminating node in the schema tree. However, it references data from a specific grouping so the
 * #child pointer points to the copy of grouping data applying specified refine and augment statements.
 *
 * The uses schema node has no instance in the data tree, so the ::lys_node_uses cannot be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_uses {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* and LYS_USESGRP
                                          values are allowed */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_USES */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node imported from the referenced grouping */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific uses's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_node_grp *grp;        /**< referred grouping definition (mandatory) */

    uint16_t refine_size;            /**< number of elements in the #refine array */
    uint16_t augment_size;           /**< number of elements in the #augment array */

    struct lys_refine *refine;       /**< array of refine changes to the referred grouping */
    struct lys_node_augment *augment;/**< array of local augments to the referred grouping */
};

/**
 * @brief Schema grouping node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #tpdf_size and
 * #tpdf members.
 *
 * In contrast to ::lys_node, ::lys_node_grp has bigger #nacm member and smaller #flags member. This is needed
 * internally by the libyang parser and it is possible because groupings uses a limited set of
 * [schema node flags](@ref snodeflags).
 *
 * ::lys_node_grp contains data specifications in the schema tree. However, the data does not directly form the schema
 * data tree. Instead, they are referenced via uses (::lys_node_uses) statement and copies of the grouping data are
 * actually placed into the uses nodes. Therefore, the nodes you can find under the ::lys_node_grp are not referenced
 * from ::lyd_node#schema.
 */
struct lys_node_grp {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:8;                /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
    uint16_t nacm:8;                 /**< [NACM extension flags](@ref nacmflags) - always 0 in ::lys_node_grp */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_GROUPING */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific grouping's data */
    uint8_t tpdf_size;               /**< number of elements in #tpdf array */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
};

/**
 * @brief Schema case node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when member.
 *
 * The case schema node has no instance in the data tree, so the ::lys_node_case cannot be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_case {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_CASE */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific case's data */
    struct lys_when *when;           /**< when statement (optional) */
};

/**
 * @brief RPC input and output node structure.
 *
 * The structure is compatible with ::lys_node, but the most parts are not usable. Therefore the ::lys_node#name,
 * ::lys_node#dsc, ::lys_node#ref, ::lys_node#flags and ::lys_node#nacm were replaced by empty bytes in fill arrays.
 * The reason to keep these useless bytes in the structure is to keep the #nodetype, #parent, #child, #next and #prev
 * members accessible when functions are using the object via a generic ::lyd_node structure. But note that the
 * ::lys_node#features_size is replaced by the #tpdf_size member and ::lys_node#features is replaced by the #tpdf
 * member.
 *
 */
struct lys_node_rpc_inout {
    const char *name;
    void *fill1[2];                  /**< padding for compatibility with ::lys_node - dsc and ref */
    uint16_t fill2[1];               /**< padding for compatibility with ::lys_node - flags and nacm */
    struct lys_module *module;       /**< link to the node's data model */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_INPUT or #LYS_OUTPUT */
    struct lys_node *parent;         /**< pointer to the parent rpc node  */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    /* specific list's data */
    uint8_t tpdf_size;                /**< number of elements in the #tpdf array */
    struct lys_tpdf *tpdf;            /**< array of typedefs */

    /* again ::lys_node compatible data */
    void *priv;                       /**< private caller's data, not used by libyang */
};

/**
 * @brief Schema notification node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #tpdf_size and
 * #tpdf members.
 */
struct lys_node_notif {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_NOTIF */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific rpc's data */
    uint8_t tpdf_size;               /**< number of elements in the #tpdf array */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
};

/**
 * @brief Schema rpc node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #tpdf_size and
 * #tpdf members.
 */
struct lys_node_rpc {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_RPC */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific rpc's data */
    uint8_t tpdf_size;               /**< number of elements in the #tpdf array */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
};

/**
 * @brief YANG augment structure (covering both possibilities - uses's substatement as well as (sub)module's substatement).
 *
 * This structure is partially interchangeable with ::lys_node structure with the following exceptions:
 * - ::lys_node#name member is replaced by ::lys_node_augment#target_name member
 * - ::lys_node_augment structure is extended by the #when and #target member
 *
 * ::lys_node_augment is not placed between all other nodes defining data node. However, it must be compatible with
 * ::lys_node structure since its children actually keeps the parent pointer to point to the original augment node
 * instead of the target node they augments (the target node is accessible via the ::lys_node_augment#target pointer).
 * The fact that a schema node comes from augment can be get via testing the #nodetype of its parent - the value in
 * ::lys_node_augment is #LYS_AUGMENT.
 */
struct lys_node_augment {
    const char *target_name;         /**< schema node identifier of the node where the augment content is supposed to be
                                          placed (mandatory). */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) */
    uint16_t nacm:2;                 /**< [NACM extension flags](@ref nacmflags) */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< #LYS_AUGMENT */
    struct lys_node *parent;         /**< uses node or NULL in case of module's top level augment */
    struct lys_node *child;          /**< augmenting data \note The child here points to the data which are also
                                          placed as children in the target node. Children are connected within the
                                          child list of the target, but their parent member still points to the augment
                                          node (this way they can be distinguished from the original target's children).
                                          It is necessary to check this carefully. */

    /* replaces #next and #prev members of ::lys_node */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_node *target;         /**< pointer to the target node */

    /* again compatible members with ::lys_node */
    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
    void *priv;                      /**< private caller's data, not used by libyang */

};

/**
 * @brief Container for list modifications in ::lys_refine_mod.
 */
struct lys_refine_mod_list {
    uint32_t min;            /**< new min-elements value. Applicable to #LYS_LIST and #LYS_LEAFLIST target nodes */
    uint32_t max;            /**< new max-elements value. Applicable to #LYS_LIST and #LYS_LEAFLIST target nodes */
};

/**
 * @brief Union to hold target modification in ::lys_refine.
 */
union lys_refine_mod {
    const char *dflt;            /**< new default value. Applicable to #LYS_LEAF and #LYS_CHOICE target nodes. In case of
                                      #LYS_CHOICE, it must be possible to resolve the value to the default branch node */
    const char *presence;        /**< presence description. Applicable to #LYS_CONTAINER target node */
    struct lys_refine_mod_list list;  /**< container for list's attributes,
                                      applicable to #LYS_LIST and #LYS_LEAFLIST target nodes */
};

/**
 * @brief YANG uses's refine substatement structure, see [RFC 6020 sec. 7.12.2](http://tools.ietf.org/html/rfc6020#section-7.12.2)
 */
struct lys_refine {
    const char *target_name;         /**< descendant schema node identifier of the target node to be refined (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint8_t flags;                   /**< [schema node flags](@ref snodeflags) - only config and mandatory flags apply */

    uint16_t target_type;            /**< limitations (get from specified refinements) for target node type:
                                          - 0 = no limitations,
                                          - ORed #LYS_NODE values if there are some limitations */

    uint8_t must_size;               /**< number of elements in the #must array */
    struct lys_restr *must;          /**< array of additional must restrictions to be added to the target */

    union lys_refine_mod mod;        /**< mutually exclusive target modifications according to the possible target_type */
};


/**
 * @brief Possible deviation modifications, see [RFC 6020 sec. 7.18.3.2](http://tools.ietf.org/html/rfc6020#section-7.18.3.2)
 */
typedef enum lys_deviate_type {
    LY_DEVIATE_NO,                   /**< not-supported */
    LY_DEVIATE_ADD,                  /**< add */
    LY_DEVIATE_RPL,                  /**< replace */
    LY_DEVIATE_DEL                   /**< delete */
} LYS_DEVIATE_TYPE;

/**
 * @brief YANG deviate statement structure, see [RFC 6020 sec. 7.18.3.2](http://tools.ietf.org/html/rfc6020#section-7.18.3.2)
 */
struct lys_deviate {
    LYS_DEVIATE_TYPE mod;            /**< type of deviation modification */

    uint8_t flags;                   /**< Properties: config, mandatory */
    const char *dflt;                /**< Properties: default (both type and choice represented as string value */
    uint32_t min;                    /**< Properties: min-elements */
    uint32_t max;                    /**< Properties: max-elements */
    uint8_t min_set;                 /**< Since min can be 0, this flag says if it is default value or 0 was set */
    uint8_t max_set;                 /**< Since max can be 0, this flag says if it is default value or 0 (unbounded) was set */
    uint8_t must_size;               /**< Properties: must - number of elements in the #must array */
    uint8_t unique_size;             /**< Properties: unique - number of elements in the #unique array */
    struct lys_restr *must;          /**< Properties: must - array of must constraints */
    struct lys_unique *unique;       /**< Properties: unique - array of unique statement structures */
    struct lys_type *type;           /**< Properties: type - pointer to type in target, type cannot be deleted or added */
    const char *units;               /**< Properties: units */
};

/**
 * @brief YANG deviation statement structure, see [RFC 6020 sec. 7.18.3](http://tools.ietf.org/html/rfc6020#section-7.18.3)
 */
struct lys_deviation {
    const char *target_name;          /**< schema node identifier of the node where the deviation is supposed to be
                                           applied (mandatory). */
    const char *dsc;                  /**< description (optional) */
    const char *ref;                  /**< reference (optional) */
    struct lys_node *orig_node;       /**< original (non-deviated) node (mandatory) */

    uint8_t deviate_size;             /**< number of elements in the #deviate array */
    struct lys_deviate *deviate;      /**< deviate information */
};

/**
 * @brief YANG import structure used to reference other schemas (modules).
 */
struct lys_import {
    struct lys_module *module;       /**< link to the imported module (mandatory) */
    const char *prefix;              /**< prefix for the data from the imported schema (mandatory) */
    char rev[LY_REV_SIZE];           /**< revision-date of the imported module (optional) */
    uint8_t external;                /**< 0 - normal import, 1 import record from a submodule or a deviating module, 2 for a deviating module */
};

/**
 * @brief YANG include structure used to reference submodules.
 */
struct lys_include {
    struct lys_submodule *submodule; /**< link to the included submodule (mandatory) */
    char rev[LY_REV_SIZE];           /**< revision-date of the included submodule (optional) */
    uint8_t external;                /**< flag for include records from submodules */
};

/**
 * @brief YANG revision statement for (sub)modules
 */
struct lys_revision {
    char date[LY_REV_SIZE];          /**< revision-date (mandatory) */
    const char *dsc;                 /**< revision's dsc (optional) */
    const char *ref;                 /**< revision's reference (optional) */
};

/**
 * @brief YANG typedef structure providing information from the schema
 */
struct lys_tpdf {
    const char *name;                /**< name of the newly defined type (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values (or 0) are allowed */
    struct lys_module *module;       /**< pointer to the module where the data type is defined (mandatory),
                                          NULL in case of built-in typedefs */

    struct lys_type type;            /**< base type from which the typedef is derived (mandatory). In case of a special
                                          built-in typedef (from yang_types.c), only the base member is filled */
    const char *units;               /**< units of the newly defined type (optional) */
    const char *dflt;                /**< default value of the newly defined type (optional) */
};

/**
 * @brief YANG list's unique statement structure, see [RFC 6020 sec. 7.8.3](http://tools.ietf.org/html/rfc6020#section-7.8.3)
 */
struct lys_unique {
    const char **expr;               /**< array of unique expressions specifying target leafs to be unique */
    uint8_t expr_size;               /**< size of the #expr array */
};

/**
 * @brief YANG feature definition structure
 */
struct lys_feature {
    const char *name;                /**< feature name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags:14;               /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values and
                                          #LYS_FENABLED value are allowed */
    uint16_t padding:2;
    struct lys_module *module;       /**< link to the features's data model (mandatory) */

    uint8_t features_size;           /**< number of elements in the #features array */
    struct lys_feature **features;   /**< array of pointers to feature definitions, this is not the array of feature
                                          definitions themselves, but the array of if-feature references */
};

/**
 * @brief YANG validity restriction (must, length, etc.) structure providing information from the schema
 */
struct lys_restr {
    const char *expr;                /**< The restriction expression/value (mandatory) */
    const char *dsc;                 /**< description (optional) */
    const char *ref;                 /**< reference (optional) */
    const char *eapptag;             /**< error-app-tag value (optional) */
    const char *emsg;                /**< error-message (optional) */
};

/**
 * @brief YANG when restriction, see [RFC 6020 sec. 7.19.5](http://tools.ietf.org/html/rfc6020#section-7.19.5)
 */
struct lys_when {
    const char *cond;                /**< specified condition (mandatory) */
    const char *dsc;                 /**< description (optional) */
    const char *ref;                 /**< reference (optional) */
};

/**
 * @brief Structure to hold information about identity, see  [RFC 6020 sec. 7.16](http://tools.ietf.org/html/rfc6020#section-7.16)
 *
 * First 5 members maps to ::lys_node.
 */
struct lys_ident {
    const char *name;                /**< identity name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint8_t flags;                   /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
    struct lys_module *module;       /**< pointer to the module where the identity is defined */

    struct lys_ident *base;          /**< pointer to the base identity */
    struct lys_ident **der;          /**< array of pointers to the derived identities */
};

/**
 * @brief Load a schema into the specified context.
 *
 * LY_IN_YANG (YANG) format is not yet supported.
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
 * LY_IN_YANG (YANG) format is not yet supported.
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
 * @brief Load a schema into the specified context from a file.
 *
 * LY_IN_YANG (YANG) format is not yet supported.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
const struct lys_module *lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format);

/**
 * @brief Get list of all the defined features in the module and its submodules.
 *
 * @param[in] module Module to explore.
 * @param[out] states Optional output parameter providing states of all features
 * returned by function in the resulting array. Indexes in both arrays corresponds
 * each other. Similarly to lys_feature_state(), possible values in the state array
 * are 1 (enabled) and 0 (disabled). Caller is supposed to free the array when it
 * is no more needed.
 * @return NULL-terminated array of all the defined features. The returned array
 * must be freed by the caller, do not free names in the array. Also remember
 * that the names will be freed with freeing the context of the module.
 */
const char **lys_features_list(const struct lys_module *module, uint8_t **states);

/**
 * @brief Enable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be enabled.
 * @param[in] feature Name of the feature to enable. To enable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int lys_features_enable(const struct lys_module *module, const char *feature);

/**
 * @brief Disable specified feature in the module
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be disabled.
 * @param[in] feature Name of the feature to disable. To disable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int lys_features_disable(const struct lys_module *module, const char *feature);

/**
 * @brief Get the current status of the specified feature in the module.
 *
 * @param[in] module Module where the feature is defined.
 * @param[in] feature Name of the feature to inspect.
 * @return
 * - 1 if feature is enabled,
 * - 0 if feature is disabled,
 * - -1 in case of error (e.g. feature is not defined)
 */
int lys_features_state(const struct lys_module *module, const char *feature);

/**
 * @brief Check if the schema node is disabled in the schema tree, i.e. there is any disabled if-feature statement
 * affecting the node.
 *
 * @param[in] node Schema node to check.
 * @param[in] recursive - 0 to check if-feature only in the \p node schema node,
 * - 1 to check if-feature in all ascendant schema nodes
 * - 2 to check if-feature in all ascendant schema nodes until there a node having an instance in a data tree
 * @return - NULL if enabled,
 * - pointer to the disabling feature if disabled.
 */
const struct lys_feature *lys_is_disabled(const struct lys_node *node, int recursive);

/**
 * @brief Get next schema tree (sibling) node element that can be instantiated in a data tree. Returned node can
 * be from an augment.
 *
 * lys_getnext() is supposed to be called sequentially. In the first call, the \p last parameter is usually NULL
 * and function starts returning i) the first \p parent child or ii) the first top level element of the \p module.
 * Consequent calls suppose to provide the previously returned node as the \p last parameter and still the same
 * \p parent and \p module parameters.
 *
 * @param[in] last Previously returned schema tree node, or NULL in case of the first call.
 * @param[in] parent Parent of the subtree where the function starts processing
 * @param[in] module In case of iterating on top level elements, the \p parent is NULL and module must be specified.
 * @param[in] options ORed options LYS_GETNEXT_*.
 * @return Next schema tree node that can be instanciated in a data tree, NULL in case there is no such element
 */
const struct lys_node *lys_getnext(const struct lys_node *last, const struct lys_node *parent,
                                   const struct lys_module *module, int options);

#define LYS_GETNEXT_WITHCHOICE   0x01 /**< lys_getnext() option to allow returning #LYS_CHOICE nodes instead of immediately looking into them */
#define LYS_GETNEXT_WITHCASE     0x02 /**< lys_getnext() option to allow returning #LYS_CASE nodes instead of immediately looking into them */
#define LYS_GETNEXT_WITHGROUPING 0x04 /**< lys_getnext() option to allow returning #LYS_GROUPING nodes instead of skipping them */
#define LYS_GETNEXT_WITHINOUT    0x08 /**< lys_getnext() option to allow returning #LYS_INPUT and #LYS_OUTPUT nodes instead of immediately looking into them */

/**
 * @brief Return parent node in the schema tree.
 *
 * In case of augmenting node, it returns the target tree node where the augmenting
 * node was placed, not the augment definition node. Function just wraps usage of the
 * ::lys_node#parent pointer in this special case.
 *
 * @param[in] node Child node to the returned parent node.
 * @return The parent node from the schema tree, NULL in case of top level nodes.
 */
struct lys_node *lys_parent(const struct lys_node *node);

/**
 * @brief Return main module of the schema tree node.
 *
 * In case of regular YANG module, it returns ::lys_node#module pointer,
 * but in case of submodule, it returns pointer to the main module.
 *
 * @param[in] node Schema tree node to be examined
 * @return pointer to the main module (schema structure), NULL in case of error.
 */
struct lys_module *lys_node_module(const struct lys_node *node);

/**
 * @brief Return main module of the module.
 *
 * In case of regular YANG module, it returns itself,
 * but in case of submodule, it returns pointer to the main module.
 *
 * @param[in] module Module to be examined
 * @return pointer to the main module (schema structure).
 */
struct lys_module *lys_main_module(const struct lys_module *module);

/**
 * @brief Set a schema private pointer to a user pointer.
 *
 * @param[in] node Node, whose private field will be assigned.
 * @param[in] priv Arbitrary user-specified pointer.
 * @return previous private object of the \p node (NULL if this is the first call on the \p node). Note, that
 * the caller is in this case responsible (if it is necessary) for freeing the replaced private object. In case
 * of invalid (NULL) \p node, NULL is returned and #ly_errno is set to #LY_EINVAL.
 */
void *lys_set_private(const struct lys_node *node, void *priv);

/**
 * @brief Print schema tree in the specified format.
 *
 * Same as lys_print(),  but it allocates memory and store the data into it.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @brief Print schema tree in the specified format.
 *
 * Same as lys_print(), but output is written into the specified file descriptor.
 *
 * @param[in] module Schema tree to print.
 * @param[in] fd File descriptor where to print the data.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @brief Print schema tree in the specified format.
 *
 * To write data into a file descriptor, use lys_print_fd().
 *
 * @param[in] module Schema tree to print.
 * @param[in] f File stream where to print the schema.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @brief Print schema tree in the specified format.
 *
 * Same as lys_print(), but output is written via provided callback.
 *
 * @param[in] module Schema tree to print.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                  const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**@} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
