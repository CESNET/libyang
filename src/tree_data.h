/**
 * @file tree_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of data trees.
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
    LYD_XML_FORMAT,      /**< For input data, it is interchangeable with #LYD_XML, for output it formats XML with indentantion */
    LYD_JSON,            /**< JSON format of the instance data */
} LYD_FORMAT;

/**
 * @brief Attribute structure.
 *
 * The structure provides information about attributes of a data element. Such attributes partially
 * maps to annotations from draft-ietf-netmod-yang-metadata. In XML, they are represented as standard
 * XML attrbutes. In JSON, they are represented as JSON elements starting with the '@' character
 * (for more information, see the yang metadata draft.
 *
 */
struct lyd_attr {
    struct lyd_attr *next;           /**< pointer to the next attribute of the same element */
    struct lys_module *module;       /**< pointer to the attribute's module.
                                          TODO when annotations will be supported, point to the annotation definition
                                          and validate that the attribute is really defined there. Currently, we just
                                          believe that it is defined in the module it says */
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
    int8_t bln;                  /**< 0 as false, 1 as true */
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
 * @defgroup validityflags Validity flags
 * @ingroup datatree
 *
 * Validity flags for data nodes.
 *
 * @{
 */
#define LYD_VAL_OK       0x00    /**< node is successfully validated including whole subtree */
#define LYD_VAL_UNIQUE   0x01    /**< Unique value(s) changed, applicable only to ::lys_node_list data nodes */
#define LYD_VAL_NOT      0xff    /**< node was not validated yet */
/**
 * @}
 */

/**
 * @brief Generic structure for a data node, directly applicable to the data nodes defined as #LYS_CONTAINER, #LYS_LIST
 * and #LYS_CHOICE.
 *
 * Completely fits to containers and choices and is compatible (can be used interchangeably except the #child member)
 * with all other lyd_node_* structures. All data nodes are provides as ::lyd_node structure by default.
 * According to the schema's ::lys_node#nodetype member, the specific object is supposed to be cast to
 * ::lyd_node_leaf_list or ::lyd_node_anyxml structures. This structure fits only to #LYS_CONTAINER, #LYS_LIST and
 * #LYS_CHOICE values.
 *
 * To traverse through all the child elements or attributes, use #LY_TREE_FOR or #LY_TREE_FOR_SAFE macro.
 */
struct lyd_node {
    struct lys_node *schema;         /**< pointer to the schema definition of this node */
    uint8_t validity;                /**< [validity flags](@ref validityflags) */

    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_node *parent;         /**< pointer to the parent node, NULL in case of root node */
    struct lyd_node *child;          /**< pointer to the first child node \note Since other lyd_node_*
                                          structures represent end nodes, this member
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
    uint8_t validity;                /**< [validity flags](@ref validityflags) */

    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_node *parent;         /**< pointer to the parent node, NULL in case of root node */

    /* struct lyd_node *child; should be here, but is not */

    /* leaflist's specific members */
    const char *value_str;           /**< string representation of value (for comparison, printing,...) */
    lyd_val value;                   /**< node's value representation */
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
    uint8_t validity;                /**< [validity flags](@ref validityflags) */

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
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node. The node can be #LYS_CONTAINER, #LYS_LIST,
 * #LYS_NOTIF, or #LYS_RPC.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new(struct lyd_node *parent, const struct lys_module *module, const char *name);

/**
 * @brief Create a new leaf or leaflist node in a data tree with a string value that is converted to
 * the actual value.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node.
 * @param[in] val_str String form of the value of the node being created. In case the type is #LY_TYPE_INST
 * or #LY_TYPE_IDENT, JSON node-id format is expected (nodes are prefixed with module names, not XML namespaces).
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name,
                              const char *val_str);

/**
 * @brief Change value of a leaf node.
 *
 * Despite the prototype allows to provide a leaflist node as \p leaf parameter, only leafs are accepted.
 *
 * @param[in] leaf A leaf node to change.
 * @param[in] val_str String form of the new value to be set to the \p leaf. In case the type is #LY_TYPE_INST
 * or #LY_TYPE_IDENT, JSON node-id format is expected (nodes are prefixed with module names, not XML namespaces).
 * @return 0 on success, non-zero on error.
 */
int lyd_change_leaf(struct lyd_node_leaf_list *leaf, const char *val_str);

/**
 * @brief Create a new anyxml node in a data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node.
 * @param[in] val_xml Value of the node being created. Must be a well-formed XML.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_anyxml(struct lyd_node *parent, const struct lys_module *module, const char *name,
                                const char *val_xml);

/**
 * @brief Create a new container node in a data tree, whose schema parent is #LYS_OUTPUT.
 *
 * @param[in] schema Schema node of the container.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_output_new(const struct lys_node *schema);

/**
 * @brief Create a new leaf or leaflist node in a data tree, whose schema parent is #LYS_OUTPUT.
 *
 * @param[in] schema Schema node of the leaf.
 * @param[in] val_str String form of the value of the node being created. In case the type is #LY_TYPE_INST
 * or #LY_TYPE_IDENT, JSON node-id format is expected (nodes are prefixed with module names, not XML namespaces).
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_output_new_leaf(const struct lys_node *schema, const char *val_str);

/**
 * @brief Create a new anyxml node in a data tree, whose schema parent is #LYS_OUTPUT.
 *
 * @param[in] schema Schema node of the leaf.
 * @param[in] val_xml Value of the node being created. Must be a well-formed XML.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_output_new_anyxml(const struct lys_node *schema, const char *val_xml);

/**
 * @brief Create a copy of the specified data tree \p node. Namespaces are copied as needed,
 * schema references are kept the same.
 *
 * @param[in] node Data tree node to be duplicated.
 * @param[in] recursive 1 if all children are supposed to be also duplicated.
 * @return Created copy of the provided data \p node.
 */
struct lyd_node *lyd_dup(const struct lyd_node *node, int recursive);

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
 * @return 0 on success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert(struct lyd_node *parent, struct lyd_node *node);

/**
 * @brief Insert the \p node element after the \p sibling element. If \p node and \p siblings are already
 * siblings (just moving \p node position), skip validation.
 *
 * @param[in] sibling The data tree node before which the \p node will be inserted.
 * @param[in] node The data tree node to be inserted.
 * @return 0 on success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Insert the \p node element after the \p sibling element.
 *
 * @param[in] sibling The data tree node before which the \p node will be inserted. If \p node and \p siblings
 * are already siblings (just moving \p node position), skip validation.
 * @param[in] node The data tree node to be inserted.
 * @return 0 on success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Search in the given data for instances of the provided schema node.
 *
 * The \p data is used to find the data root and function then searches in the whole tree and all sibling trees.
 *
 * @param[in] data A node in the data tree to search.
 * @param[in] schema Schema node of the data nodes caller want to find.
 * @return Set of found data nodes (use dset member of ::ly_set). If no data node is found, the returned set is empty.
 * In case of error, NULL is returned.
 */
struct ly_set *lyd_get_node(const struct lyd_node *data, const struct lys_node *schema);

/**
 * @brief Validate \p node data subtree.
 *
 * @param[in] node Data tree to be validated.
 * @param[in] options Options for the inserting data to the target data tree options, see @ref parseroptions.
 * @param[in] ctx libyang context for the data (used only in case the \p node is NULL, so in case of checking empty data tree)
 * @return 0 on success (if options include #LYD_OPT_FILTER, some nodes could still have been deleted as an
 * optimization, which can have a bad consequences when the \p node stores a subtree instead of a tree with
 * a top-level node(s)), nonzero in case of an error.
 */
int lyd_validate(struct lyd_node *node, int options, ...);

/**
 * @brief Unlink the specified data subtree. All referenced namespaces are copied.
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
 * @brief Free (and unlink) the specified data (sub)tree and all its siblings (preceding as well as following).
 *
 * @param[in] node One of the siblings root element of the (sub)trees to be freed.
 */
void lyd_free_withsiblings(struct lyd_node *node);

/**
 * @brief Insert attribute into the data node.
 *
 * @param[in] parent Data node where to place the attribute
 * @param[in] name Attribute name including the prefix (prefix:name). Prefix must be the name of one of the
 *            schema in the \p parent's context.
 * @param[in] value Attribute value
 * @return pointer to the created attribute (which is already connected in \p parent) or NULL on error.
 */
struct lyd_attr *lyd_insert_attr(struct lyd_node *parent, const char *name, const char *value);

/**
 * @brief Destroy data attribute
 *
 * If the attribute to destroy is a member of a node attribute list, it is necessary to
 * provide the node itself as \p parent to keep the list consistent.
 *
 * @param[in] ctx Context where the attribute was created (usually it is the context of the \p parent)
 * @param[in] parent Parent node where the attribute is placed
 * @param[in] attr Attribute to destroy
 * @param[in] recursive Zero to destroy only the attribute, non-zero to destroy also all the subsequent attributes
 *            in the list.
 */
void lyd_free_attr(struct ly_ctx *ctx, struct lyd_node *parent, struct lyd_attr *attr, int recursive);

/**
 * @brief Opaque internal structure, do not access it from outside.
 */
struct lyxml_elem;

/**
 * @brief Serialize anyxml content for further processing.
 *
 * @param[in] anyxml Anyxml content from ::lyd_node_anyxml#value to serialize ax XML string
 * @return Serialized content of the anyxml or NULL in case of error. Need to be freed after
 * done using.
 */
char *lyxml_serialize(const struct lyxml_elem *anyxml);

/**@} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_DATA_H_ */
