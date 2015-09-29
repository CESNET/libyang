/**
 * @file tree.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of data model and data trees.
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

#ifndef LY_TREE_DATA_H_
#define LY_TREE_DATA_H_

#include <stddef.h>
#include <stdint.h>

#include "tree_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup datatree
 * @{
 */

/**
 * @brief Data input/output formats supported by libyang [parser](@ref parsers) and [printer](@ref printers) functions.
 */
typedef enum {
    LYD_UNKNOWN,         /**< unknown format, used as return value in case of error */
    LYD_XML,             /**< XML format of the instance data */
    LYD_JSON,            /**< JSON format of the instance data */
} LYD_FORMAT;

/**
 * @brief Data attribute's type to distinguish between a standard (XML) attribute and namespace definition
 */
typedef enum lyd_attr_type {
    LYD_ATTR_STD = 1,                /**< standard attribute, see ::lyd_attr structure */
    LYD_ATTR_NS = 2                  /**< namespace definition, see ::lyd_ns structure */
} LYD_ATTR_TYPE;

/**
 * @brief Namespace definition structure.
 *
 * Usually, the object is provided as ::lyd_attr structure. The structure is compatible with
 * ::lyd_attr within the first two members (#type and #next) to allow passing through and type
 * detection interchangeably.  When the type member is set to #LYD_ATTR_NS, the ::lyd_attr
 * structure should be cast to ::lyd_ns to access the rest of members.
 */
struct lyd_ns {
    LYD_ATTR_TYPE type;              /**< always #LYD_ATTR_NS, compatible with ::lyd_attr */
    struct lyd_attr *next;           /**< pointer to the next attribute or namespace definition of an element,
                                          compatible with ::lyd_attr */
    struct lyd_node *parent;         /**< pointer to the element where the namespace definition is placed */
    const char *prefix;              /**< namespace prefix value */
    const char *value;               /**< namespace value */
};

/**
 * @brief Attribute structure.
 *
 * The structure provides information about attributes of a data element and covers not only
 * attributes but also namespace definitions. Therefore, the first two members (#type and #next)
 * can be safely accessed to pass through the attributes list and type detection. When the #type
 * member has #LYD_ATTR_STD value, the rest of the members can be used. Otherwise, the object
 * should be cast to the appropriate structure according to #LYD_ATTR_TYPE enumeration.
 */
struct lyd_attr {
    LYD_ATTR_TYPE type;              /**< type of the attribute, to access the last three members, the value
                                          must be ::LYD_ATTR_STD */
    struct lyd_attr *next;           /**< pointer to the next attribute or namespace definition of an element */
    struct lyd_ns *ns;               /**< pointer to the definition of the namespace of the attribute */
    const char *name;                /**< attribute name */
    const char *value;               /**< attribute value */
};

/**
 * @brief node's value representation
 */
typedef union lyd_value_u {
    const char *binary;          /**< base64 encoded, NULL terminated string */
    struct lys_type_bit **bit;   /**< bitmap of pointers to the schema definition of the bit value that are set,
                                      its size is always the number of defined bits in the schema */
    int8_t bool;                 /**< 0 as false, 1 as true */
    int64_t dec64;               /**< decimal64: value = dec64 / 10^fraction-digits  */
    struct lys_type_enum *enm;   /**< pointer to the schema definition of the enumeration value */
    struct lys_ident *ident;     /**< pointer to the schema definition of the identityref value */
    struct lyd_node *instance;   /**< instance-identifier, pointer to the referenced data tree node */
    int8_t int8;                 /**< 8-bit signed integer */
    int16_t int16;               /**< 16-bit signed integer */
    int32_t int32;               /**< 32-bit signed integer */
    int64_t int64;               /**< 64-bit signed integer */
    struct lyd_node *leafref;    /**< pointer to the referenced leaf/leaflist instance in data tree */
    const char *string;          /**< string */
    uint8_t uint8;               /**< 8-bit unsigned integer */
    uint16_t uint16;             /**< 16-bit signed integer */
    uint32_t uint32;             /**< 32-bit signed integer */
    uint64_t uint64;             /**< 64-bit signed integer */
} lyd_val;

/**
 * @brief Generic structure for a data node, directly applicable to the data nodes defined as #LYS_CONTAINER, #LYS_LIST
 * and #LYS_CHOICE.
 *
 * Completely fits to containers and choices and is compatible (can be used interchangeably except the #child member)
 * with all other lyd_node_* structures. All data nodes are provides as ::lyd_node structure by default.
 * According to the schema's ::lys_node#nodetype member, the specific object is supposed to be cast to
 * ::lyd_node_list, ::lyd_node_leaf, ::lyd_node_leaflist or ::lyd_node_anyxml structures. This structure fits only to
 * #LYS_CONTAINER and #LYS_CHOICE values.
 *
 * To traverse through all the child elements or attributes, use #LY_TREE_FOR or #LY_TREE_FOR_SAFE macro.
 */
struct lyd_node {
    struct lys_node *schema;         /**< pointer to the schema definition of this node */

    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_node *parent;         /**< pointer to the parent node, NULL in case of root node */
    struct lyd_node *child;          /**< pointer to the first child node \note Since other lyd_node_*
                                          structures (except ::lyd_node_list) represent end nodes, this member
                                          is replaced in those structures. Therefore, be careful with accessing
                                          this member without having information about the node type from the schema's
                                          ::lys_node#nodetype member. */
};

/**
 * @brief Structure for data nodes defined as #LYS_LEAF or #LYS_LEAFLIST.
 *
 * Extension for ::lyd_node structure. It replaces the ::lyd_node#child member by
 * three new members (#value, #value_str and #value_type) to provide
 * information about the value. The first five members (#schema, #attr, #next,
 * #prev and #parent) are compatible with the ::lyd_node's members.
 *
 * To traverse through all the child elements or attributes, use #LY_TREE_FOR or #LY_TREE_FOR_SAFE macro.
 */
struct lyd_node_leaf_list {
    struct lys_node *schema;         /**< pointer to the schema definition of this node which is ::lys_node_leaflist
                                          structure */

    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_node *parent;         /**< pointer to the parent node, NULL in case of root node */

    /* struct lyd_node *child; should be here, but is not */

    /* leaflist's specific members */
    lyd_val value;                   /**< node's value representation */
    const char *value_str;           /**< string representation of value (for comparison, printing,...) */
    LY_DATA_TYPE value_type;         /**< type of the value in the node, mainly for union to avoid repeating of type detection */
};

/**
 * @brief Structure for data nodes defined as #LYS_ANYXML.
 *
 * Extension for ::lyd_node structure - replaces the ::lyd_node#child member by new #value member. The first five
 * members (#schema, #attr, #next, #prev and #parent) are compatible with the ::lyd_node's members.
 *
 * To traverse through all the child elements or attributes, use #LY_TREE_FOR or #LY_TREE_FOR_SAFE macro.
 */
struct lyd_node_anyxml {
    struct lys_node *schema;         /**< pointer to the schema definition of this node which is ::lys_node_anyxml
                                          structure */

    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_node *parent;         /**< pointer to the parent node, NULL in case of root node */

    /* struct lyd_node *child; should be here, but is not */

    /* anyxml's specific members */
    struct lyxml_elem *value;       /**< anyxml name is the root element of value! */
};

/**
 * @brief Create a new container node in a data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node \p name.
 * @param[in] name Schema node name of the new data node. The node can be #LYS_CONTAINER, #LYS_LIST,
 * #LYS_INPUT, #LYS_OUTPUT, #LYS_NOTIF, or #LYS_RPC.
 */
struct lyd_node *lyd_new(struct lyd_node *parent, struct lys_module *module, const char *name);

/**
 * @brief Create a new leaf or leaflist node in a data tree with a specific value.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] snode Schema node of the new data node. Can be #LYS_LEAF or #LYS_LEAFLIST.
 * @param[in] type Type of the value provided in the \p value parameter. Cannot be #LY_TYPE_DER, #LY_TYPE_UNION,
 * or #LY_TYPE_INST.
 * @param[in] value Value of the node being created. Can be NULL only if \p type is #LY_TYPE_EMPTY. Must be
 * statically allocated!
 */
struct lyd_node *lyd_new_leaf_val(struct lyd_node *parent, struct lys_module *module, const char *name,
                                  LY_DATA_TYPE type, lyd_val value);

/**
 * @brief Create a new leaf or leaflist node in a data tree with a string value that is converted to
 * the actual value.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] snode Schema node of the new data node. Can be #LYS_LEAF or #LYS_LEAFLIST.
 * @param[in] type Interpretation of the string provided in the \p val_str parameter. After appropriate
 * conversion this will be the resulting type of the value in the node. Cannot be #LY_TYPE_DER or #LY_TYPE_UNION.
 * @param[in] val_str String form of the value of the node being created. Can be NULL only if \p type is
 * #LY_TYPE_EMPTY.
 */
struct lyd_node *lyd_new_leaf_str(struct lyd_node *parent, struct lys_module *module, const char *name,
                                  LY_DATA_TYPE type, const char *val_str);

/**
 * @brief Create a new anyxml node in a data tree.
 *
 * TODO not implemented
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] snode Schema node of the new data node. Can only be #LYS_ANYXML.
 * @param[in] val_xml Value of the node being created. Must be a well-formed XML.
 */
struct lyd_node *lyd_new_anyxml(struct lyd_node *parent, struct lys_module *module, const char *name,
                                const char *val_xml);

/**
 * @brief Create a copy of the specified data tree \p node
 *
 * TODO not implemented
 *
 * @param[in] node Data tree node to be duplicated.
 * @param[in] recursive 1 if all children are supposed to be also duplicated.
 * @return Created copy of the provided data \p node.
 */
struct lyd_node *lyd_dup(struct lyd_node *node, int recursive);

/**
 * @brief Insert the \p node element as child to the \p parent element. The \p node is inserted as a last child of the
 * \p parent.
 *
 * If the node is part of some other tree, it is automatically unlinked.
 * If the node is the first node of a node list (with no parent), all
 * the subsequent nodes are also inserted.
 *
 * @param[in] parent Parent node for the \p node being inserted.
 * @param[in] node The node being inserted.
 * @param[in] options Options for the inserting data to the target data tree options, see @ref parseroptions.
 * @return 0 fo success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert(struct lyd_node *parent, struct lyd_node *node, int options);

/**
 * @brief Insert the \p node element after the \p sibling element.
 *
 * TODO not implemented
 *
 * @param[in] sibling The data tree node before which the \p node will be inserted.
 * @param[in] node The data tree node to be inserted.
 * @return 0 fo success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Insert the \p node element after the \p sibling element.
 *
 * @param[in] sibling The data tree node before which the \p node will be inserted.
 * @param[in] node The data tree node to be inserted.
 * @param[in] options Options for the inserting data to the target data tree options, see @ref parseroptions.
 * @return 0 fo success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node, int options);

/**
 * @brief Move the data tree \p node before the specified \p sibling node
 *
 * Both the data nodes must be in the same children list, i.e. they have the same parent.
 *
 * TODO not implemented
 *
 * @param[in] sibling The data tree node before which the \p node will be moved.
 * @param[in] node The data tree node to be moved.
 * @return 0 for success, nonzero in case of error
 */
int lyd_move_before(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Move the data tree \p node after the specified \p sibling node
 *
 * Both the data nodes must be in the same children list, i.e. they have the same parent.
 *
 * TODO not implemented
 *
 * @param[in] sibling The data tree node after which the \p node will be moved.
 * @param[in] node The data tree node to be moved.
 * @return 0 for success, nonzero in case of error
 */
int lyd_move_after(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Unlink the specified data subtree.
 *
 * Note, that the node's connection with the schema tree is kept. Therefore, in case of
 * reconnecting the node to a data tree using lyd_paste() it is necessary to paste it
 * to the appropriate place in the data tree following the schema.
 *
 * @param[in] node Data tree node to be unlinked (together with all children).
 * @return 0 for success, nonzero for error
 */
int lyd_unlink(struct lyd_node *node);

/**
 * @brief Free (and unlink) the specified data (sub)tree.
 *
 * @param[in] node Root of the (sub)tree to be freed.
 */
void lyd_free(struct lyd_node *node);

/**
 * @brief Opaque internal structure, do not access it from outside.
 */
struct lyxml_elem;

/**
 * @brief Serialize anyxml content for further processing.
 *
 * TODO not implemented
 *
 * @param[in] anyxml Anyxml content from ::lyd_node_anyxml#value to serialize ax XML string
 * @return Serialized content of the anyxml or NULL in case of error
 */
char *lyxml_serialize(struct lyxml_elem *anyxml);

/**
 * @brief Structure to hold a set of (not necessary somehow connected) ::lyd_node objects.
 *
 * To free the structure, use lyd_set_free() function, to manipulate with the structure, use other
 * lyd_set_* functions.
 */
struct lyd_set {
    unsigned int size;               /**< allocated size of the set array */
    unsigned int number;             /**< number of elements in (used size of) the set array */
    struct lyd_node **set;           /**< array of pointers to a ::lyd_node objects */
};

/**
 * @brief Create and initiate new ::lyd_set structure.
 *
 * @return Created ::lyd_set structure or NULL in case of error.
 */
struct lyd_set *lyd_set_new(void);

/**
 * @brief Add a ::lyd_node object into the set
 *
 * @param[in] set Set where the \p node will be added.
 * @param[in] node The ::lyd_node object to be added into the \p set;
 * @return 0 on success
 */
int lyd_set_add(struct lyd_set *set, struct lyd_node *node);

/**
 * @brief Free the ::lyd_set data. Frees only the set structure content, not the referred data.
 *
 * @param[in] set The set to be freed.
 */
void lyd_set_free(struct lyd_set *set);

/**@} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_DATA_H_ */
