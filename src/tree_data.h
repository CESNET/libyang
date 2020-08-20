/**
 * @file tree_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of YANG data trees.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_DATA_H_
#define LY_TREE_DATA_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_ctx;
struct ly_path;
struct ly_set;
struct lyd_node;
struct lyd_node_opaq;
struct lys_module;
struct lysc_node;

/**
 * @defgroup datatree Data Tree
 * @ingroup trees
 * @{
 *
 * Data structures and functions to manipulate and access instance data tree.
 */

/**
 * @brief Macro for getting child pointer of a generic data node.
 *
 * @param[in] node Node whose child pointer to get.
 */
#define LYD_CHILD(node) ((node)->schema ? (lyd_node_children(node, 0)) : ((struct lyd_node_opaq *)(node))->child)

/**
 * @brief Macro for getting child pointer of a generic data node but skipping its keys in case it is ::LYS_LIST.
 *
 * @param[in] node Node whose child pointer to get.
 */
#define LYD_CHILD_NO_KEYS(node) ((node)->schema ? (lyd_node_children(node, LYD_CHILDREN_SKIP_KEYS)) : ((struct lyd_node_opaq *)(node))->child)

/**
 * @brief Macro for getting generic parent pointer of a node.
 *
 * @param[in] node Node whose parent pointer to get.
 */
#define LYD_PARENT(node) ((struct lyd_node *)(node)->parent)

/**
 * @brief Macro to iterate via all elements in a data tree. This is the opening part
 * to the #LYD_TREE_DFS_END - they always have to be used together.
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
 * Use the same parameters for #LYD_TREE_DFS_BEGIN and #LYD_TREE_DFS_END. While
 * START can be any of the lyd_node* types, ELEM variable must be a pointer to
 * the generic struct lyd_node.
 *
 * To skip a particular subtree, instead of the continue statement, set LYD_TREE_DFS_continue
 * variable to non-zero value.
 *
 * Use with opening curly bracket '{' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param ELEM Iterator intended for use in the block.
 */
#define LYD_TREE_DFS_BEGIN(START, ELEM) \
    { int LYD_TREE_DFS_continue = 0; struct lyd_node *LYD_TREE_DFS_next; \
    for ((ELEM) = (LYD_TREE_DFS_next) = (struct lyd_node *)(START); \
         (ELEM); \
         (ELEM) = (LYD_TREE_DFS_next), LYD_TREE_DFS_continue = 0)

/**
 * @brief Macro to iterate via all elements in a tree. This is the closing part
 * to the #LYD_TREE_DFS_BEGIN - they always have to be used together.
 *
 * Use the same parameters for #LYD_TREE_DFS_BEGIN and #LYD_TREE_DFS_END. While
 * START can be any of the lyd_node* types, ELEM variable must be a pointer
 * to the generic struct lyd_node.
 *
 * Use with closing curly bracket '}' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param ELEM Iterator intended for use in the block.
 */

#define LYD_TREE_DFS_END(START, ELEM) \
    /* select element for the next run - children first */ \
    if (LYD_TREE_DFS_continue) { \
        (LYD_TREE_DFS_next) = NULL; \
    } else { \
        (LYD_TREE_DFS_next) = lyd_node_children(ELEM, 0); \
    }\
    if (!(LYD_TREE_DFS_next)) { \
        /* no children */ \
        if ((ELEM) == (struct lyd_node*)(START)) { \
            /* we are done, (START) has no children */ \
            break; \
        } \
        /* try siblings */ \
        (LYD_TREE_DFS_next) = (ELEM)->next; \
    } \
    while (!(LYD_TREE_DFS_next)) { \
        /* parent is already processed, go to its sibling */ \
        (ELEM) = (struct lyd_node*)(ELEM)->parent; \
        /* no siblings, go back through parents */ \
        if ((ELEM)->parent == (START)->parent) { \
            /* we are done, no next element to process */ \
            break; \
        } \
        (LYD_TREE_DFS_next) = (ELEM)->next; \
    } } \

/**
 * @brief Macro to get context from a data tree node.
 */
#define LYD_CTX(node) ((node)->schema ? (node)->schema->module->ctx : ((struct lyd_node_opaq *)(node))->ctx)

/**
 * @brief Data input/output formats supported by libyang [parser](@ref howtodataparsers) and
 * [printer](@ref howtodataprinters) functions.
 */
typedef enum {
    LYD_UNKNOWN = 0,     /**< unknown data format, invalid value */
    LYD_XML,             /**< XML instance data format */
    LYD_JSON,            /**< JSON instance data format */
    LYD_LYB,             /**< LYB instance data format */
} LYD_FORMAT;

/**
 * @brief All kinds of supported prefix mappings to modules.
 */
typedef enum {
    LY_PREF_SCHEMA,          /**< value prefixes map to YANG import prefixes */
    LY_PREF_XML,             /**< value prefixes map to XML namespace prefixes */
    LY_PREF_JSON,            /**< value prefixes map to module names */
} LY_PREFIX_FORMAT;

/**
 * @brief List of possible value types stored in ::lyd_node_any.
 */
typedef enum {
    LYD_ANYDATA_DATATREE,            /**< Value is a pointer to lyd_node structure (first sibling). When provided as input parameter, the pointer
                                          is directly connected into the anydata node without duplication, caller is supposed to not manipulate
                                          with the data after a successful call (including calling lyd_free() on the provided data) */
    LYD_ANYDATA_STRING,              /**< Value is a generic string without any knowledge about its format (e.g. anyxml value in JSON encoded
                                          as string). XML sensitive characters (such as & or \>) are automatically escaped when the anydata
                                          is printed in XML format. */
    LYD_ANYDATA_XML,                 /**< Value is a string containing the serialized XML data. */
    LYD_ANYDATA_JSON,                /**< Value is a string containing the data modeled by YANG and encoded as I-JSON. */
    LYD_ANYDATA_LYB,                 /**< Value is a memory chunk with the serialized data tree in LYB format. */
} LYD_ANYDATA_VALUETYPE;

/** @} */

/**
 * @brief Special lyd_value structure for union.
 *
 * Represents data with multiple types (union). Original value is stored in the main lyd_value:canonical_cache while
 * the lyd_value_subvalue::value contains representation according to one of the union's types.
 * The lyd_value_subvalue:prefixes provides (possible) mappings from prefixes in the original value to YANG modules.
 * These prefixes are necessary to parse original value to the union's subtypes.
 */
struct lyd_value_subvalue {
    struct lyd_value *value;     /**< representation of the value according to the selected union's subtype
                                      (stored as lyd_value::realpath here, in subvalue structure */
    const char *original;        /**< Original value in the dictionary. */
    LY_PREFIX_FORMAT format;     /**< Prefix format of the value. However, this information is also used to decide
                                      whether a value is valid for the specific format or not on later validations
                                      (instance-identifier in XML looks different than in JSON). */
    void *prefix_data;           /**< Format-specific data for prefix resolution (see ::ly_resolve_prefix) */
    int parser_hint;             /**< Hint options from the parser */
};

/**
 * @brief YANG data representation
 */
struct lyd_value {
    const char *canonical;           /**< Canonical string representation of the value in the dictionary. It is never
                                          NULL and in case of no canonical value, its JSON representation is used instead. */
    union {
        int8_t boolean;              /**< 0 as false, 1 as true */
        int64_t dec64;               /**< decimal64: value = dec64 / 10^fraction-digits  */
        int8_t int8;                 /**< 8-bit signed integer */
        int16_t int16;               /**< 16-bit signed integer */
        int32_t int32;               /**< 32-bit signed integer */
        int64_t int64;               /**< 64-bit signed integer */
        uint8_t uint8;               /**< 8-bit unsigned integer */
        uint16_t uint16;             /**< 16-bit unsigned integer */
        uint32_t uint32;             /**< 32-bit unsigned integer */
        uint64_t uint64;             /**< 64-bit unsigned integer */
        struct lysc_type_bitenum_item *enum_item;  /**< pointer to the definition of the enumeration value */
        struct lysc_type_bitenum_item **bits_items; /**< list of set pointers to the specification of the set bits ([sized array](@ref sizedarrays)) */
        struct lysc_ident *ident;    /**< pointer to the schema definition of the identityref value */
        struct ly_path *target;      /**< Instance-identifier target path. */
        struct lyd_value_subvalue *subvalue; /** Union value with some metadata. */
        void *ptr;                   /**< generic data type structure used to store the data */
    };  /**< The union is just a list of shorthands to possible values stored by a type's plugin. libyang itself uses the lyd_value::realtype
             plugin's callbacks to work with the data. */

    struct lysc_type *realtype;      /**< pointer to the real type of the data stored in the value structure. This type can differ from the type
                                          in the schema node of the data node since the type's store plugin can use other types/plugins for
                                          storing data. Speaking about built-in types, this is the case of leafref which stores data as its
                                          target type. In contrast, union type also use its subtype's callbacks, but inside an internal data
                                          lyd_value::subvalue structure, so here is the pointer to the union type.
                                          In general, this type is used to get free callback for this lyd_value structure, so it must reflect
                                          the type used to store data directly in the same lyd_value instance. */
};

/**
 * @brief Macro for getting the string canonical value from a term node.
 *
 * @param[in] node Term node with the value.
 * @return Canonical value.
 */
#define LYD_CANON_VALUE(node) ((struct lyd_node_term *)(node))->value.canonical

/**
 * @brief Metadata structure.
 *
 * The structure provides information about metadata of a data element. Such attributes must map to
 * annotations as specified in RFC 7952. The only exception is the filter type (in NETCONF get operations)
 * and edit-config's operation attributes. In XML, they are represented as standard XML attributes. In JSON,
 * they are represented as JSON elements starting with the '@' character (for more information, see the
 * YANG metadata RFC.
 *
 */
struct lyd_meta {
    struct lyd_node *parent;         /**< data node where the metadata is placed */
    struct lyd_meta *next;           /**< pointer to the next metadata of the same element */
    struct lysc_ext_instance *annotation; /**< pointer to the annotation's definition */
    const char *name;                /**< metadata name */
    struct lyd_value value;          /**< metadata value representation */
};

/**
 * @brief Generic prefix and namespace mapping, meaning depends on the format.
 *
 * The union is used as a reference to the data's module and according to the format, it can be used as a key for
 * ly_ctx_get_module_implemented_ns() or ly_ctx_get_module_implemented(). While the module reference is always present,
 * the id member can be omitted in case it is not present in the source data as a reference to the default namespace.
 */
struct ly_prefix {
    const char *id;               /**< identifier used in the qualified name of the item to reference data node namespace */
    union {
        const char *module_ns;    /**< namespace of the module where the data are supposed to belongs to, used for LYD_XML format. */
        const char *module_name;  /**< name of the module where the data are supposed to belongs to, used for LYD_JSON format. */
    };
};

/**
 * @brief Generic attribute structure.
 */
struct lyd_attr {
    struct lyd_node_opaq *parent;   /**< data node where the attribute is placed */
    struct lyd_attr *next;
    struct ly_prefix *val_prefs;    /**< list of prefixes in the value ([sized array](@ref sizedarrays)) */
    const char *name;
    const char *value;

    LYD_FORMAT format;              /**< format of the prefixes, only LYD_XML and LYD_JSON values can appear at this place */
    int hint;                       /**< additional information about from the data source, see the [hints list](@ref lydopaqhints) */
    struct ly_prefix prefix;        /**< name prefix, it is stored because they are a real pain to generate properly */

};

#define LYD_NODE_INNER (LYS_CONTAINER|LYS_LIST|LYS_RPC|LYS_ACTION|LYS_NOTIF) /**< Schema nodetype mask for lyd_node_inner */
#define LYD_NODE_TERM (LYS_LEAF|LYS_LEAFLIST)   /**< Schema nodetype mask for lyd_node_term */
#define LYD_NODE_ANY (LYS_ANYDATA)   /**< Schema nodetype mask for lyd_node_any */

/**
 * @defgroup dnodeflags Data node flags
 * @ingroup datatree
 * @{
 *
 * Various flags of data nodes.
 *
 *     1 - container    5 - anydata/anyxml
 *     2 - list         6 - rpc/action
 *     3 - leaf         7 - notification
 *     4 - leaflist
 *
 *     bit name              1 2 3 4 5 6 7
 *     ---------------------+-+-+-+-+-+-+-+
 *       1 LYD_DEFAULT      |x| |x|x| | | |
 *                          +-+-+-+-+-+-+-+
 *       2 LYD_WHEN_TRUE    |x|x|x|x|x| | |
 *                          +-+-+-+-+-+-+-+
 *       3 LYD_NEW          |x|x|x|x|x|x|x|
 *     ---------------------+-+-+-+-+-+-+-+
 *
 */

#define LYD_DEFAULT      0x01        /**< default (implicit) node */
#define LYD_WHEN_TRUE    0x02        /**< all when conditions of this node were evaluated to true */
#define LYD_NEW          0x04        /**< node was created after the last validation, is needed for the next validation */

/** @} */

/**
 * @brief Generic structure for a data node.
 */
struct lyd_node {
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list or
                                          hashes of all nodes of subtree in case of keyless list). Note that while hash can be
                                          used to get know that nodes are not equal, it cannot be used to decide that the
                                          nodes are equal due to possible collisions. */
    uint32_t flags;                  /**< [data node flags](@ref dnodeflags) */
    const struct lysc_node *schema;  /**< pointer to the schema definition of this node, note that the target can be not just
                                          ::struct lysc_node but ::struct lysc_action or ::struct lysc_notif as well */
    struct lyd_node_inner *parent;   /**< pointer to the parent node, NULL in case of root node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_meta *meta;           /**< pointer to the list of metadata of this node */

#ifdef LY_ENABLED_LYD_PRIV
    void *priv;                      /**< private user data, not used by libyang */
#endif
};

/**
 * @brief Data node structure for the inner data tree nodes - containers, lists, RPCs, actions and Notifications.
 */
struct lyd_node_inner {
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list or
                                          hashes of all nodes of subtree in case of keyless list). Note that while hash can be
                                          used to get know that nodes are not equal, it cannot be used to decide that the
                                          nodes are equal due to possible collisions. */
    uint32_t flags;                  /**< [data node flags](@ref dnodeflags) */
    const struct lysc_node *schema;  /**< pointer to the schema definition of this node */
    struct lyd_node_inner *parent;   /**< pointer to the parent node, NULL in case of root node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_meta *meta;           /**< pointer to the list of metadata of this node */

#ifdef LY_ENABLED_LYD_PRIV
    void *priv;                      /**< private user data, not used by libyang */
#endif

    struct lyd_node *child;          /**< pointer to the first child node. */
    struct hash_table *children_ht;  /**< hash table with all the direct children (except keys for a list, lists without keys) */
#define LYD_HT_MIN_ITEMS 4           /**< minimal number of children to create lyd_node_inner::children_ht hash table. */
};

/**
 * @brief Data node structure for the terminal data tree nodes - leaves and leaf-lists.
 */
struct lyd_node_term {
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list or
                                          hashes of all nodes of subtree in case of keyless list). Note that while hash can be
                                          used to get know that nodes are not equal, it cannot be used to decide that the
                                          nodes are equal due to possible collisions. */
    uint32_t flags;                  /**< [data node flags](@ref dnodeflags) */
    const struct lysc_node *schema;  /**< pointer to the schema definition of this node */
    struct lyd_node_inner *parent;   /**< pointer to the parent node, NULL in case of root node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_meta *meta;           /**< pointer to the list of metadata of this node */

#ifdef LY_ENABLED_LYD_PRIV
    void *priv;                      /**< private user data, not used by libyang */
#endif

    struct lyd_value value;            /**< node's value representation */
};

/**
 * @brief Data node structure for the anydata data tree nodes - anydatas and anyxmls.
 */
struct lyd_node_any {
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list or
                                          hashes of all nodes of subtree in case of keyless list). Note that while hash can be
                                          used to get know that nodes are not equal, it cannot be used to decide that the
                                          nodes are equal due to possible collisions. */
    uint32_t flags;                  /**< [data node flags](@ref dnodeflags) */
    const struct lysc_node *schema;  /**< pointer to the schema definition of this node */
    struct lyd_node_inner *parent;   /**< pointer to the parent node, NULL in case of root node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_meta *meta;           /**< pointer to the list of attributes of this node */

#ifdef LY_ENABLED_LYD_PRIV
    void *priv;                      /**< private user data, not used by libyang */
#endif

    union lyd_any_value {
        struct lyd_node *tree;       /**< data tree */
        const char *str;             /**< Generic string data */
        const char *xml;             /**< Serialized XML data */
        const char *json;            /**< I-JSON encoded string */
        char *mem;                   /**< LYD_ANYDATA_LYB memory chunk */
    } value;                         /**< pointer to the stored value representation of the anydata/anyxml node */
    LYD_ANYDATA_VALUETYPE value_type;/**< type of the data stored as lyd_node_any::value */
};

/**
 * @defgroup lydopaqhints Opaq data node hints.
 *
 * Additional information about the opaq nodes from their source. The flags are stored in lyd_node_opaq::hint
 * and they can have a slightly different meaning for the specific lyd_node_opaq::format.
 * @{
 */
#define LYD_NODE_OPAQ_ISLIST       0x001 /**< LYD_JSON: node is expected to be a list or leaf-list */
#define LYD_NODE_OPAQ_ISENVELOPE   0x002 /**< LYD_JSON, LYD_XML: RPC/Action/Notification envelope out of the YANG schemas */

#define LYD_NODE_OPAQ_ISSTRING     0x100 /**< LYD_JSON: value is expected to be string as defined in JSON encoding. */
#define LYD_NODE_OPAQ_ISNUMBER     0x200 /**< LYD_JSON: value is expected to be number as defined in JSON encoding. */
#define LYD_NODE_OPAQ_ISBOOLEAN    0x400 /**< LYD_JSON: value is expected to be boolean as defined in JSON encoding. */
#define LYD_NODE_OPAQ_ISEMPTY      0x800 /**< LYD_JSON: value is expected to be empty as defined in JSON encoding. */

/** @} lydopaqhints */

/**
 * @brief Data node structure for unparsed (opaque) nodes.
 */
struct lyd_node_opaq {
    uint32_t hash;                  /**< always 0 */
    uint32_t flags;                 /**< always 0 */
    const struct lysc_node *schema; /**< always NULL */
    struct lyd_node *parent;        /**< pointer to the parent node (NULL in case of root node) */
    struct lyd_node *next;          /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;          /**< pointer to the previous sibling node (last sibling if there is none) */
    struct lyd_attr *attr;

#ifdef LY_ENABLED_LYD_PRIV
    void *priv;                     /**< private user data, not used by libyang */
#endif

    struct lyd_node *child;         /**< pointer to the child node (NULL if there are none) */
    const char *name;
    LYD_FORMAT format;
    struct ly_prefix prefix;        /**< name prefix */
    struct ly_prefix *val_prefs;    /**< list of prefixes in the value ([sized array](@ref sizedarrays)) */
    const char *value;              /**< original value */
    int hint;                       /**< additional information about from the data source, see the [hints list](@ref lydopaqhints) */
    const struct ly_ctx *ctx;       /**< libyang context */
};

/**
 * @defgroup children_options Children traversal options.
 * @ingroup datatree
 */

#define LYD_CHILDREN_SKIP_KEYS  0x01    /**< If list children are returned, skip its keys. */

/** @} children_options */

/**
 * @brief Get the node's children list if any.
 *
 * Decides the node's type and in case it has a children list, returns it.
 * @param[in] node Node to check.
 * @param[in] options Bitmask of options, see @ref
 * @return Pointer to the first child node (if any) of the \p node.
 */
struct lyd_node *lyd_node_children(const struct lyd_node *node, int options);

/**
 * @brief Get the owner module of the data node. It is the module of the top-level schema node. Generally,
 * in case of augments it is the target module, recursively, otherwise it is the module where the data node is defined.
 *
 * @param[in] node Data node to examine.
 * @return Module owner of the node.
 */
const struct lys_module *lyd_owner_module(const struct lyd_node *node);

/**
 * @brief Learn the length of LYB data.
 *
 * @param[in] data LYB data to examine.
 * @return Length of the LYB data chunk,
 * @return -1 on error.
 */
int lyd_lyb_data_length(const char *data);

/**
 * @brief Copy anydata value from one node to another. Target value is freed first.
 *
 * @param[in,out] trg Target node.
 * @param[in] value Source value, may be NULL when the target value is only freed.
 * @param[in] value_type Source value type.
 * @return LY_ERR value.
 */
LY_ERR lyd_any_copy_value(struct lyd_node *trg, const union lyd_any_value *value, LYD_ANYDATA_VALUETYPE value_type);

/**
 * @brief Create a new inner node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node can be #LYS_CONTAINER, #LYS_NOTIF, #LYS_RPC, or #LYS_ACTION.
 * @param[out] node Optional created node.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_inner(struct lyd_node *parent, const struct lys_module *module, const char *name, struct lyd_node **node);

/**
 * @brief Create a new list node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node must be #LYS_LIST.
 * @param[out] node Optional created node.
 * @param[in] ... Ordered key values of the new list instance, all must be set. In case of an instance-identifier
 * or identityref value, the JSON format is expected (module names instead of prefixes). No keys are expected for
 * key-less lists.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_list(struct lyd_node *parent, const struct lys_module *module, const char *name, struct lyd_node **node, ...);

/**
 * @brief Create a new list node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node must be #LYS_LIST.
 * @param[in] keys All key values predicate in the form of "[key1='val1'][key2='val2']...", they do not have to be ordered.
 * In case of an instance-identifier or identityref value, the JSON format is expected (module names instead of prefixes).
 * Use NULL or string of length 0 in case of key-less list.
 * @param[out] node Optional created node.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_list2(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *keys,
                     struct lyd_node **node);

/**
 * @brief Create a new term node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node can be #LYS_LEAF or #LYS_LEAFLIST.
 * @param[in] val_str String form of the value of the node being created. In case of an instance-identifier or identityref
 * value, the JSON format is expected (module names instead of prefixes).
 * @param[out] node Optional created node.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_term(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str,
                    struct lyd_node **node);

/**
 * @brief Create a new any node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node can be #LYS_ANYDATA or #LYS_ANYXML.
 * @param[in] value Value to be directly assigned to the node. Expected type is determined by @p value_type.
 * @param[in] value_type Type of the provided value in @p value.
 * @param[out] node Optional created node.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_any(struct lyd_node *parent, const struct lys_module *module, const char *name, const void *value,
                   LYD_ANYDATA_VALUETYPE value_type, struct lyd_node **node);

/**
 * @brief Create new metadata for a data node.
 *
 * @param[in] parent Parent node for the metadata being created.
 * @param[in] module Module of the metadata being created. If NULL, @p name must include module name as the prefix.
 * @param[in] name Annotation name of the new metadata. It can include the annotation module as the prefix.
 *            If the prefix is specified it is always used but if not specified, @p module must be set.
 * @param[in] val_str String form of the value of the metadata. In case of an instance-identifier or identityref
 * value, the JSON format is expected (module names instead of prefixes).
 * @param[out] meta Optional created metadata.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_meta(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str,
                    struct lyd_meta **meta);

/**
 * @brief Create a new opaque node in the data tree.
 *
 * @param[in] parent Parent node for the node beaing created. NULL in case of creating a top level element.
 * @param[in] ctx libyang context. If NULL, @p parent context will be used.
 * @param[in] name Node name.
 * @param[in] value Node value, may be NULL.
 * @param[in] module_name Node module name.
 * @param[out] node Optional created node.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_opaq(struct lyd_node *parent, const struct ly_ctx *ctx, const char *name, const char *value,
                    const char *module_name, struct lyd_node **node);

/**
 * @brief Create new attribute for an opaque data node.
 *
 * @param[in] parent Parent opaque node for the attribute being created.
 * @param[in] module Module name of the attribute being created. There may be none.
 * @param[in] name Attribute name. It can include the module name as the prefix.
 * @param[in] val_str String value of the attribute. Is stored directly.
 * @param[out] attr Optional created attribute.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_attr(struct lyd_node *parent, const char *module_name, const char *name, const char *val_str,
                    struct lyd_attr **attr);

/**
 * @defgroup pathoptions Data path creation options
 * @ingroup datatree
 *
 * Various options to change lyd_new_path*() behavior.
 *
 * Default behavior:
 * - if the target node already exists (and is not default), an error is returned.
 * - the whole path to the target node is created (with any missing parents) if necessary.
 * - RPC output schema children are completely ignored in all modules. Input is searched and nodes created normally.
 * @{
 */

#define LYD_NEWOPT_UPDATE 0x01 /**< If the target node exists, is a leaf, and it is updated with a new value or its
                                    default flag is changed, it is returned. If the target node exists and is not
                                    a leaf or generally no change occurs in the @p parent tree, NULL is returned and
                                    no error set. */
#define LYD_NEWOPT_OUTPUT 0x02 /**< Changes the behavior to ignoring RPC/action input schema nodes and using only
                                    output ones. */
#define LYD_NEWOPT_OPAQ   0x04 /**< Enables the creation of opaque nodes with some specific rules. If the __last node__
                                    in the path is not uniquely defined ((leaf-)list without a predicate) or has an
                                    invalid value (leaf/leaf-list), it is created as opaque. */

/** @} pathoptions */

/**
 * @brief Create a new node in the data tree based on a path. Cannot be used for anyxml/anydata nodes,
 * for those use ::lyd_new_path_any.
 *
 * If @p path points to a list key and the list instance does not exist, the key value from the predicate is used
 * and @p value is ignored. Also, if a leaf-list is being created and both a predicate is defined in @p path
 * and @p value is set, the predicate is preferred.
 *
 * @param[in] parent Data parent to add to/modify, can be NULL.
 * @param[in] ctx libyang context, must be set if @p parent is NULL.
 * @param[in] path Path to create (TODO ref path).
 * @param[in] value Value of the new leaf/leaf-list. For other node types, it is ignored.
 * @param[in] options Bitmask of options, see @ref pathoptions.
 * @param[out] node Optional first created node.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_path(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const char *value,
                    int options, struct lyd_node **node);

/**
 * @brief Create a new node in the data tree based on a path. All node types can be created.
 *
 * If @p path points to a list key and the list instance does not exist, the key value from the predicate is used
 * and @p value is ignored. Also, if a leaf-list is being created and both a predicate is defined in @p path
 * and @p value is set, the predicate is preferred.
 *
 * @param[in] parent Data parent to add to/modify, can be NULL.
 * @param[in] ctx libyang context, must be set if @p parent is NULL.
 * @param[in] path Path to create (TODO ref path).
 * @param[in] value Value of the new leaf/leaf-list/anyxml/anydata. For other node types, it is ignored.
 * @param[in] value_type Anyxml/anydata node @p value type.
 * @param[in] options Bitmask of options, see @ref pathoptions.
 * @param[out] node Optional first created node.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_path_any(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const void *value,
                        LYD_ANYDATA_VALUETYPE value_type, int options, struct lyd_node **node);

/**
 * @brief Create a new node in the data tree based on a path. All node types can be created.
 *
 * If @p path points to a list key and the list instance does not exist, the key value from the predicate is used
 * and @p value is ignored. Also, if a leaf-list is being created and both a predicate is defined in @p path
 * and @p value is set, the predicate is preferred.
 *
 * @param[in] parent Data parent to add to/modify, can be NULL.
 * @param[in] ctx libyang context, must be set if @p parent is NULL.
 * @param[in] path Path to create (TODO ref path).
 * @param[in] value Value of the new leaf/leaf-list/anyxml/anydata. For other node types, it is ignored.
 * @param[in] value_type Anyxml/anydata node @p value type.
 * @param[in] options Bitmask of options, see @ref pathoptions.
 * @param[out] new_parent Optional first parent node created. If only one node was created, equals to @p new_node.
 * @param[out] new_node Optional last node created.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_path2(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const void *value,
                     LYD_ANYDATA_VALUETYPE value_type, int options, struct lyd_node **new_parent, struct lyd_node **new_node);

/**
 * @defgroup implicitoptions Implicit node creation options
 * @ingroup datatree
 *
 * Various options to change lyd_new_implicit*() behavior.
 *
 * Default behavior:
 * - both configuration and state missing implicit nodes are added.
 * - all implicit node types are added (non-presence containers, default leaves, and default leaf-lists).
 * @{
 */

#define LYD_IMPLICIT_NO_STATE    0x01   /**< Do not add any implicit state nodes. */
#define LYD_IMPLICIT_NO_CONFIG   0x02   /**< Do not add any implicit config nodes. */
#define LYD_IMPLICIT_NO_DEFAULTS 0x04   /**< Do not add any default nodes (leaves/leaf-lists), only non-presence
                                             containers. */

/** @} implicitoptions */

/**
 * @brief Add any missing implicit nodes into a data subtree.
 *
 * @param[in] tree Tree to add implicit nodes into.
 * @param[in] implicit_options Options for implicit node creation, see @ref implicitoptions.
 * @param[out] diff Optional diff with any created nodes.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_implicit_tree(struct lyd_node *tree, int implicit_options, struct lyd_node **diff);

/**
 * @brief Add any missing implicit nodes.
 *
 * @param[in,out] tree Tree to add implicit nodes into.
 * @param[in] ctx libyang context, must be set only if @p tree is an empty tree.
 * @param[in] implicit_options Options for implicit node creation, see @ref implicitoptions.
 * @param[out] diff Optional diff with any created nodes.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_implicit_all(struct lyd_node **tree, const struct ly_ctx *ctx, int implicit_options, struct lyd_node **diff);

/**
 * @brief Add any missing implicit nodes of one module.
 *
 * @param[in,out] tree Tree to add implicit nodes into.
 * @param[in] module Module whose implicit nodes to create.
 * @param[in] implicit_options Options for implicit node creation, see @ref implicitoptions.
 * @param[out] diff Optional diff with any created nodes.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_implicit_module(struct lyd_node **tree, const struct lys_module *module, int implicit_options,
                               struct lyd_node **diff);

/**
 * @brief Change the value of a term (leaf or leaf-list) node.
 *
 * Node changed this way is always considered explicitly set, meaning its default flag
 * is always cleared.
 *
 * @param[in] term Term node to change.
 * @param[in] val_str New value to set, any prefixes are expected in JSON format.
 * @return LY_SUCCESS if value was changed,
 * @return LY_EEXIST if value was the same and only the default flag was cleared,
 * @return LY_ENOT if the values were equal and no change occured,
 * @return LY_ERR value on other errors.
 */
LY_ERR lyd_change_term(struct lyd_node *term, const char *val_str);

/**
 * @brief Change the value of a metadata instance.
 *
 * @param[in] ctx libyang context.
 * @param[in] meta Metadata to change.
 * @param[in] val_str New value to set, any prefixes are expected in JSON format.
 * @return LY_SUCCESS if value was changed,
 * @return LY_ENOT if the values were equal and no change occured,
 * @return LY_ERR value on other errors.
 */
LY_ERR lyd_change_meta(struct lyd_meta *meta, const char *val_str);

/**
 * @brief Insert a child into a parent.
 *
 * - if the node is part of some other tree, it is automatically unlinked.
 * - if the node is the first node of a node list (with no parent), all the subsequent nodes are also inserted.
 *
 * @param[in] parent Parent node to insert into.
 * @param[in] node Node to insert.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LY_ERR lyd_insert_child(struct lyd_node *parent, struct lyd_node *node);

/**
 * @brief Insert a node into siblings.
 *
 * - if the node is part of some other tree, it is automatically unlinked.
 * - if the node is the first node of a node list (with no parent), all the subsequent nodes are also inserted.
 *
 * @param[in] sibling Siblings to insert into, can even be NULL.
 * @param[in] node Node to insert.
 * @param[out] first Optionally return the first sibling after insertion. Can be the address of @p sibling.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LY_ERR lyd_insert_sibling(struct lyd_node *sibling, struct lyd_node *node, struct lyd_node **first);

/**
 * @brief Insert a node before another node, can be used only for user-ordered nodes.
 *
 * - if the node is part of some other tree, it is automatically unlinked.
 * - if the node is the first node of a node list (with no parent), all the subsequent nodes are also inserted.
 *
 * @param[in] sibling Sibling node to insert before.
 * @param[in] node Node to insert.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LY_ERR lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Insert a node after another node, can be used only for user-ordered nodes.
 *
 * - if the node is part of some other tree, it is automatically unlinked.
 * - if the node is the first node of a node list (with no parent), all the subsequent nodes are also inserted.
 *
 * @param[in] sibling Sibling node to insert after.
 * @param[in] node Node to insert.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LY_ERR lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Unlink the specified data subtree.
 *
 * @param[in] node Data tree node to be unlinked (together with all the children).
 */
void lyd_unlink_tree(struct lyd_node *node);

/**
 * @brief Free all the nodes (even parents of the node) in the data tree.
 *
 * @param[in] node Any of the nodes inside the tree.
 */
void lyd_free_all(struct lyd_node *node);

/**
 * @brief Free all the sibling nodes (preceding as well as succeeding).
 *
 * @param[in] node Any of the sibling nodes to free.
 */
void lyd_free_siblings(struct lyd_node *node);

/**
 * @brief Free (and unlink) the specified data (sub)tree.
 *
 * @param[in] node Root of the (sub)tree to be freed.
 */
void lyd_free_tree(struct lyd_node *node);

/**
 * @brief Free a single metadata instance.
 *
 * @param[in] meta Metadata to free.
 */
void lyd_free_meta_single(struct lyd_meta *meta);

/**
 * @brief Free the metadata instance with any following instances.
 *
 * @param[in] meta Metadata to free.
 */
void lyd_free_meta_siblings(struct lyd_meta *meta);

/**
 * @brief Free a single attribute.
 *
 * @param[in] ctx Context where the attributes were created.
 * @param[in] attr Attribute to free.
 */
void ly_free_attr_single(const struct ly_ctx *ctx, struct lyd_attr *attr);

/**
 * @brief Free the attribute with any following attributes.
 *
 * @param[in] ctx Context where the attributes were created.
 * @param[in] attr First attribute to free.
 */
void ly_free_attr_siblings(const struct ly_ctx *ctx, struct lyd_attr *attr);

/**
 * @brief Check type restrictions applicable to the particular leaf/leaf-list with the given string @p value.
 *
 * The given node is not modified in any way - it is just checked if the @p value can be set to the node.
 *
 * If there is no data node instance and you are fine with checking just the type's restrictions without the
 * data tree context (e.g. for the case of require-instance restriction), use lys_value_validate().
 *
 * @param[in] ctx libyang context for logging (function does not log errors when @p ctx is NULL)
 * @param[in] node Data node for the @p value.
 * @param[in] value String value to be checked, it is expected to be in JSON format.
 * @param[in] value_len Length of the given @p value (mandatory).
 * @param[in] tree Data tree (e.g. when validating RPC/Notification) where the required data instance (leafref target,
 *            instance-identifier) can be placed. NULL in case the data tree is not yet complete,
 *            then LY_EINCOMPLETE can be returned.
 * @param[out] realtype Optional real type of the value.
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case the @p trees is not provided and it was needed to finish the validation (e.g. due to require-instance).
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_validate(const struct ly_ctx *ctx, const struct lyd_node_term *node, const char *value, size_t value_len,
                          const struct lyd_node *tree, struct lysc_type **realtype);

/**
 * @brief Compare the node's value with the given string value. The string value is first validated according to the node's type.
 *
 * @param[in] node Data node to compare.
 * @param[in] value String value to be compared. It does not need to be in a canonical form - as part of the process,
 * it is validated and canonized if possible. But it is expected to be in JSON format.
 * @param[in] value_len Length of the given @p value (mandatory).
 * @param[in] tree Data tree (e.g. when validating RPC/Notification) where the required data instance (leafref target,
 *            instance-identifier) can be placed. NULL in case the data tree is not yet complete,
 *            then LY_EINCOMPLETE can be returned.
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case of success when the @p trees is not provided and it was needed to finish the validation of
 * the given string @p value (e.g. due to require-instance).
 * @return LY_ENOT if the values do not match.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_compare(const struct lyd_node_term *node, const char *value, size_t value_len, const struct lyd_node *tree);

/**
 * @defgroup datacompareoptions Data compare options
 * @ingroup datatree
 *
 * Various options to change the lyd_compare() behavior.
 */
#define LYD_COMPARE_FULL_RECURSION 0x01 /* lists and containers are the same only in case all they children
                                           (subtree, so direct as well as indirect children) are the same. By default,
                                           containers are the same in case of the same schema node and lists are the same
                                           in case of equal keys (keyless lists do the full recursion comparison all the time). */
#define LYD_COMPARE_DEFAULTS 0x02       /* By default, implicit and explicit default nodes are considered to be equal. This flag
                                           changes this behavior and implicit (automatically created default node) and explicit
                                           (explicitly created node with the default value) default nodes are considered different. */
/** @} datacompareoptions */

/**
 * @brief Compare 2 data nodes if they are equivalent.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @param[in] options Various @ref datacompareoptions.
 * @return LY_SUCCESS if the nodes are equivalent.
 * @return LY_ENOT if the nodes are not equivalent.
 */
LY_ERR lyd_compare_single(const struct lyd_node *node1, const struct lyd_node *node2, int options);

/**
 * @brief Compare 2 lists of siblings if they are equivalent.
 *
 * @param[in] node1 The first sibling list to compare.
 * @param[in] node2 The second sibling list to compare.
 * @param[in] options Various @ref datacompareoptions.
 * @return LY_SUCCESS if all the siblings are equivalent.
 * @return LY_ENOT if the siblings are not equivalent.
 */
LY_ERR lyd_compare_siblings(const struct lyd_node *node1, const struct lyd_node *node2, int options);

/**
 * @brief Compare 2 metadata.
 *
 * @param[in] meta1 First metadata.
 * @param[in] meta2 Second metadata.
 * @return LY_SUCCESS if the metadata are equivalent.
 * @return LY_ENOT if not.
 */
LY_ERR lyd_compare_meta(const struct lyd_meta *meta1, const struct lyd_meta *meta2);

/**
 * @defgroup dupoptions Data duplication options
 * @ingroup datatree
 *
 * Various options to change lyd_dup() behavior.
 *
 * Default behavior:
 * - only the specified node is duplicated without siblings, parents, or children.
 * - all the metadata of the duplicated nodes are also duplicated.
 * @{
 */

#define LYD_DUP_RECURSIVE    0x01  /**< Duplicate not just the node but also all the children. Note that
                                        list's keys are always duplicated. */
#define LYD_DUP_NO_META      0x02  /**< Do not duplicate metadata of any node. */
#define LYD_DUP_WITH_PARENTS 0x04  /**< If a nested node is being duplicated, duplicate also all the parents.
                                        Keys are also duplicated for lists. Return value does not change! */
#define LYD_DUP_WITH_FLAGS   0x08  /**< Also copy any data node flags. That will cause the duplicated data to preserve
                                        its validation/default node state. */

/** @} dupoptions */

/**
 * @brief Create a copy of the specified data tree \p node. Schema references are kept the same.
 *
 * @param[in] node Data tree node to be duplicated.
 * @param[in] parent Optional parent node where to connect the duplicated node(s).
 * If set in combination with LYD_DUP_WITH_PARENTS, the parents chain is duplicated until it comes to and connects with
 * the @p parent.
 * @param[in] options Bitmask of options flags, see @ref dupoptions.
 * @param[out] dup Optional created copy of the node. Note that in case the parents chain is duplicated for the duplicated
 * node(s) (when LYD_DUP_WITH_PARENTS used), the first duplicated node is still returned.
 * @return LY_ERR value.
 */
LY_ERR lyd_dup_single(const struct lyd_node *node, struct lyd_node_inner *parent, int options, struct lyd_node **dup);

/**
 * @brief Create a copy of the specified data tree \p node with any following siblings. Schema references are kept the same.
 *
 * @param[in] node Data tree node to be duplicated.
 * @param[in] parent Optional parent node where to connect the duplicated node(s).
 * If set in combination with LYD_DUP_WITH_PARENTS, the parents chain is duplicated until it comes to and connects with
 * the @p parent.
 * @param[in] options Bitmask of options flags, see @ref dupoptions.
 * @param[out] dup Optional created copy of the node. Note that in case the parents chain is duplicated for the duplicated
 * node(s) (when LYD_DUP_WITH_PARENTS used), the first duplicated node is still returned.
 * @return LY_ERR value.
 */
LY_ERR lyd_dup_siblings(const struct lyd_node *node, struct lyd_node_inner *parent, int options, struct lyd_node **dup);

/**
 * @brief Create a copy of the metadata.
 *
 * @param[in] meta Metadata to copy.
 * @param[in] parent Node where to append the new metadata.
 * @param[out] dup Optional created metadata copy.
 * @return LY_ERR value.
 */
LY_ERR lyd_dup_meta_single(const struct lyd_meta *meta, struct lyd_node *parent, struct lyd_meta **dup);

/**
 * @defgroup mergeoptions Data merge options.
 * @ingroup datatree
 *
 * Various options to change lyd_merge() behavior.
 *
 * Default behavior:
 * - source data tree is not modified in any way,
 * - any default nodes in the source are ignored if there are explicit nodes in the target.
 * @{
 */

#define LYD_MERGE_DESTRUCT      0x01 /**< Spend source data tree in the function, it cannot be used afterwards! */
#define LYD_MERGE_DEFAULTS      0x02 /**< Default nodes in the source tree replace even explicit nodes in the target. */

/** @} mergeoptions */

/**
 * @brief Merge the source data subtree into the target data tree. Merge may not be complete until validation
 * is called on the resulting data tree (data from more cases may be present, default and non-default values).
 *
 * @param[in,out] target Target data tree to merge into, must be a top-level tree.
 * @param[in] source Source data tree to merge, must be a top-level tree.
 * @param[in] options Bitmask of option flags, see @ref mergeoptions.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
LY_ERR lyd_merge_tree(struct lyd_node **target, const struct lyd_node *source, int options);

/**
 * @brief Merge the source data tree with any following siblings into the target data tree. Merge may not be
 * complete until validation called on the resulting data tree (data from more cases may be present, default
 * and non-default values).
 *
 * @param[in,out] target Target data tree to merge into, must be a top-level tree.
 * @param[in] source Source data tree to merge, must be a top-level tree.
 * @param[in] options Bitmask of option flags, see @ref mergeoptions.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
LY_ERR lyd_merge_siblings(struct lyd_node **target, const struct lyd_node *source, int options);

/**
 * @defgroup diffoptions Data diff options.
 * @ingroup datatree
 *
 * Various options to change lyd_diff() behavior.
 *
 * Default behavior:
 * - any default nodes are treated as non-existent and ignored.
 * @{
 */

#define LYD_DIFF_DEFAULTS   0x01 /**< Default nodes in the trees are not ignored but treated similarly to explicit
                                      nodes. Also, leaves and leaf-lists are added into diff even in case only their
                                      default flag (state) was changed. */

/** @} diffoptions */

/**
 * @brief Learn the differences between 2 data trees.
 *
 * The resulting diff is represented as a data tree with specific metadata from the internal 'yang'
 * module. Most importantly, every node has an effective 'operation' metadata. If there is none
 * defined on the node, it inherits the operation from the nearest parent. Top-level nodes must
 * always have the 'operation' metadata defined. Additional metadata ('orig-default', 'value',
 * 'orig-value', 'key', 'orig-key') are used for storing more information about the value in the first
 * or the second tree.
 *
 * The diff tree is completely independent on the @p first and @p second trees, meaning all
 * the information about the change is stored in the diff and the trees are not needed.
 *
 * !! Caution !!
 * The diff tree should never be validated because it may easily not be valid! For example,
 * when data from one case branch are deleted and data from another branch created - data from both
 * branches are then stored in the diff tree simultaneously.
 *
 * @param[in] first First data tree.
 * @param[in] second Second data tree.
 * @param[in] options Bitmask of options flags, see @ref diffoptions.
 * @param[out] diff Generated diff.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_tree(const struct lyd_node *first, const struct lyd_node *second, int options, struct lyd_node **diff);

/**
 * @brief Learn the differences between 2 data trees including all the following siblings.
 *
 * @param[in] first First data tree.
 * @param[in] second Second data tree.
 * @param[in] options Bitmask of options flags, see @ref diffoptions.
 * @param[out] diff Generated diff.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_siblings(const struct lyd_node *first, const struct lyd_node *second, int options, struct lyd_node **diff);

/**
 * @brief Callback for diff nodes.
 *
 * @param[in] diff_node Diff node.
 * @param[in] data_node Matching node in data.
 * @param[in] cb_data Arbitrary callback data.
 * @return LY_ERR value.
 */
typedef LY_ERR (*lyd_diff_cb)(const struct lyd_node *diff_node, struct lyd_node *data_node, void *cb_data);

/**
 * @brief Apply the whole diff on a data tree but restrict the operation to one module.
 *
 * @param[in,out] data Data to apply the diff on.
 * @param[in] diff Diff to apply.
 * @param[in] mod Module, whose diff/data only to consider, NULL for all modules.
 * @param[in] diff_cb Optional diff callback that will be called for every changed node.
 * @param[in] cb_data Arbitrary callback data.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_apply_module(struct lyd_node **data, const struct lyd_node *diff, const struct lys_module *mod,
                             lyd_diff_cb diff_cb, void *cb_data);

/**
 * @brief Apply the whole diff tree on a data tree.
 *
 * @param[in,out] data Data to apply the diff on.
 * @param[in] diff Diff to apply.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_apply_all(struct lyd_node **data, const struct lyd_node *diff);

/**
 * @brief Merge 2 diffs into each other but restrict the operation to one module.
 *
 * The diffs must be possible to be merged, which is guaranteed only if the source diff was
 * created on data that had the target diff applied on them. In other words, this sequence is legal
 *
 * 1) diff1 from data1 and data2 -> data11 from apply diff1 on data1 -> diff2 from data11 and data3 ->
 *    -> data 33 from apply diff2 on data1
 *
 * and reusing these diffs
 *
 * 2) diff11 from merge diff1 and diff2 -> data33 from apply diff11 on data1
 *
 * @param[in,out] diff Target diff to merge into.
 * @param[in] src_diff Source diff.
 * @param[in] mod Module, whose diff only to consider, NULL for all modules.
 * @param[in] diff_cb Optional diff callback that will be called for every changed node.
 * @param[in] cb_data Arbitrary callback data.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_merge_module(struct lyd_node **diff, const struct lyd_node *src_diff, const struct lys_module *mod,
                             lyd_diff_cb diff_cb, void *cb_data);

/**
 * @brief Merge 2 diff trees into each other.
 *
 * @param[in,out] diff_first Target diff first sibling to merge into.
 * @param[in] diff_parent Target diff parent to merge into.
 * @param[in] src_sibling Source diff sibling to merge.
 * @param[in] diff_cb Optional diff callback that will be called for every changed node.
 * @param[in] cb_data Arbitrary callback data.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_merge_tree(struct lyd_node **diff_first, struct lyd_node *diff_parent, const struct lyd_node *src_sibling,
                           lyd_diff_cb diff_cb, void *cb_data);

/**
 * @brief Merge 2 diffs into each other.
 *
 * @param[in,out] diff Target diff to merge into.
 * @param[in] src_diff Source diff.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_merge_all(struct lyd_node **diff, const struct lyd_node *src_diff);

/**
 * @brief Reverse a diff and make the opposite changes. Meaning change create to delete, delete to create,
 * or move from place A to B to move from B to A and so on.
 *
 * @param[in] src_diff Diff to reverse.
 * @param[out] diff Reversed diff.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR lyd_diff_reverse_all(const struct lyd_node *src_diff, struct lyd_node **diff);

/**
 * @brief Find the target in data of a compiled ly_path structure (instance-identifier).
 *
 * @param[in] path Compiled path structure.
 * @param[in] tree Data tree to be searched.
 * @return Found target node,
 * @return NULL if not found.
 */
const struct lyd_node_term *lyd_target(const struct ly_path *path, const struct lyd_node *tree);

/**
 * @brief Types of the different data paths.
 */
typedef enum {
    LYD_PATH_LOG, /**< Descriptive path format used in log messages */
    LYD_PATH_LOG_NO_LAST_PRED, /**< Similar to ::LYD_PATH_LOG except there is never a predicate on the last node */
} LYD_PATH_TYPE;

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
char *lyd_path(const struct lyd_node *node, LYD_PATH_TYPE pathtype, char *buffer, size_t buflen);

/**
 * @brief Find a specific metadata.
 *
 * @param[in] first First metadata to consider.
 * @param[in] module Module of the metadata definition, may be NULL if @p name includes a prefix.
 * @param[in] name Name of the metadata to find, may not include a prefix (module name) if @p module is set.
 * @return Found metadata,
 * @return NULL if not found.
 */
struct lyd_meta *lyd_find_meta(const struct lyd_meta *first, const struct lys_module *module, const char *name);

/**
 * @brief Search in the given siblings (NOT recursively) for the first target instance with the same value.
 * Uses hashes - should be used whenever possible for best performance.
 *
 * @param[in] siblings Siblings to search in including preceding and succeeding nodes.
 * @param[in] target Target node to find.
 * @param[out] match Can be NULL, otherwise the found data node.
 * @return LY_SUCCESS on success, @p match set.
 * @return LY_ENOTFOUND if not found, @p match set to NULL.
 * @return LY_ERR value if another error occurred.
 */
LY_ERR lyd_find_sibling_first(const struct lyd_node *siblings, const struct lyd_node *target, struct lyd_node **match);

/**
 * @brief Search in the given siblings for the first schema instance.
 * Uses hashes - should be used whenever possible for best performance.
 *
 * @param[in] siblings Siblings to search in including preceding and succeeding nodes.
 * @param[in] schema Schema node of the data node to find.
 * @param[in] key_or_value If it is NULL, the first schema node data instance is found. For nodes with many
 * instances, it can be set based on the type of @p schema:
 *              LYS_LEAFLIST:
 *                  Searched instance value.
 *              LYS_LIST:
 *                  Searched instance key values in the form of "[key1='val1'][key2='val2']...".
 *                  The keys do not have to be ordered but all of them must be set.
 *
 *              Note that any explicit values (leaf-list or list key values) will be canonized first
 *              before comparison. But values that do not have a canonical value are expected to be in the
 *              JSON format!
 * @param[in] val_len Optional length of @p key_or_value in case it is not 0-terminated.
 * @param[out] match Can be NULL, otherwise the found data node.
 * @return LY_SUCCESS on success, @p match set.
 * @return LY_ENOTFOUND if not found, @p match set to NULL.
 * @return LY_EINVAL if @p schema is a key-less list.
 * @return LY_ERR value if another error occurred.
 */
LY_ERR lyd_find_sibling_val(const struct lyd_node *siblings, const struct lysc_node *schema, const char *key_or_value,
                            size_t val_len, struct lyd_node **match);

/**
 * @brief Search in the given data for instances of nodes matching the provided XPath.
 *
 * The expected format of the expression is ::LYD_JSON, meaning the first node in every path
 * must have its module name as prefix or be the special `*` value for all the nodes.
 *
 * If a list instance is being selected with all its key values specified (but not necessarily ordered)
 * in the form `list[key1='val1'][key2='val2'][key3='val3']` or a leaf-list instance in the form
 * `leaf-list[.='val']`, these instances are found using hashes with constant (*O(1)*) complexity
 * (unless they are defined in top-level). Other predicates can still follow the aforementioned ones.
 *
 * @param[in] ctx_node XPath context node.
 * @param[in] xpath Data XPath expression filtering the matching nodes. ::LYD_JSON format is expected.
 * @param[out] set Set of found data nodes. In case the result is a number, a string, or a boolean,
 * the returned set is empty.
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_find_xpath(const struct lyd_node *ctx_node, const char *xpath, struct ly_set **set);

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_DATA_H_ */
