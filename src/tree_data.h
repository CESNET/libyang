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
#include "xml.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup datatree Data Tree
 * @{
 *
 * Data structures and functions to manipulate and access instance data tree.
 */

/**
 * @brief Data input/output formats supported by libyang [parser](@ref howtodataparsers) and
 * [printer](@ref howtodataprinters) functions.
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
    struct lyd_node *instance;   /**< pointer to the instance-identifier target, note that if the tree was modified,
                                      the target (address) can be invalid - the pointer is correctly checked and updated
                                      by lyd_validate() */
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
 * @defgroup parseroptions Data parser options
 * @ingroup datatree
 *
 * Various options to change the data tree parsers behavior.
 *
 * Default behavior:
 * - in case of XML, parser reads all data from its input (file, memory, XML tree) including the case of not well-formed
 * XML document (multiple top-level elements) and if there is an unknown element, it is skipped including its subtree
 * (see the next point). This can be changed by the #LYD_OPT_NOSIBLINGS option which make parser to read only a single
 * tree (with a single root element) from its input.
 * - parser silently ignores the data without a matching node in schema trees. If the caller want to stop
 * parsing in case of presence of unknown data, the #LYD_OPT_STRICT can be used. The strict mode is useful for
 * NETCONF servers, since NETCONF clients should always send data according to the capabilities announced by the server.
 * On the other hand, the default non-strict mode is useful for clients receiving data from NETCONF server since
 * clients are not required to understand everything the server does. Of course, the optimal strategy for clients is
 * to use filtering to get only the required data. Having an unknown element of the known namespace is always an error.
 * The behavior can be changed by #LYD_OPT_STRICT option.
 * - using obsolete statements (status set to obsolete) just generates a warning, but the processing continues. The
 * behavior can be changed by #LYD_OPT_OBSOLETE option.
 * - parser expects that the provided data provides complete datastore content (both the configuration and state data)
 * and performs data validation according to all YANG rules. This can be a problem in case of representing NETCONF's
 * subtree filter data, edit-config's data or other type of data set - such data do not represent a complete data set
 * and some of the validation rules can fail. Therefore there are other options (within lower 8 bits) to make parser
 * to accept such a data.
 * @{
 */

#define LYD_OPT_DATA       0x00 /**< Default type of data - complete datastore content with configuration as well as
                                     state data. */
#define LYD_OPT_CONFIG     0x01 /**< A configuration datastore - complete datastore without state data.
                                     Validation modifications:
                                     - status data are not allowed */
#define LYD_OPT_GET        0x02 /**< Data content from a NETCONF reply message to the NETCONF \<get\> operation.
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - list's keys/unique nodes are not required (so duplication is not checked) */
#define LYD_OPT_GETCONFIG  0x04 /**< Data content from a NETCONF reply message to the NETCONF \<get-config\> operation
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - list's keys/unique nodes are not required (so duplication is not checked)
                                     - status data are not allowed */
#define LYD_OPT_EDIT       0x08 /**< Content of the NETCONF \<edit-config\>'s config element.
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - status data are not allowed */
#define LYD_OPT_RPC        0x10 /**< Data represents RPC's input parameters. */
#define LYD_OPT_RPCREPLY   0x20 /**< Data represents RPC's output parameters (maps to NETCONF <rpc-reply> data). */
#define LYD_OPT_NOTIF      0x40 /**< Data represents an event notification data. */
#define LYD_OPT_FILTER     0x80 /**< Data represents NETCONF subtree filter. Validation modifications:
                                     - leafs/leaf-lists with no data are allowed (even not allowed e.g. by length restriction)
                                     - multiple instances of container/leaf/.. are allowed
                                     - list's keys/unique nodes are not required
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - data from different choice's branches are allowed */
#define LYD_OPT_TYPEMASK   0xff /**< Mask to filter data type options. Always only a single data type option (only
                                     single bit from the lower 8 bits) can be set. */

#define LYD_OPT_STRICT     0x0100 /**< Instead of silent ignoring data without schema definition, raise an error. */
#define LYD_OPT_DESTRUCT   0x0200 /**< Free the provided XML tree during parsing the data. With this option, the
                                       provided XML tree is affected and all succesfully parsed data are freed.
                                       This option is applicable only to lyd_parse_xml() function. */
#define LYD_OPT_OBSOLETE   0x0400 /**< Raise an error when an obsolete statement (status set to obsolete) is used. */
#define LYD_OPT_NOSIBLINGS 0x0800 /**< Parse only a single XML tree from the input. This option applies only to
                                       XML input data. */
#define LYD_OPT_TRUSTED    0x1000 /**< Data comes from a trusted source and it is not needed to validate them. Data
                                       are connected with the schema, but the most validation checks (mandatory nodes,
                                       list instance uniqueness, etc.) are not performed. This option does not make
                                       sense for lyd_validate() so it is ignored by this function. */

/**@} parseroptions */

/**
 * @brief Parse (and validate according to appropriate schema from the given context) data.
 *
 * In case of LY_XML format, the data string is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] data Serialized data in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree or NULL in case of empty \p data. To free the returned structure,
 *         use lyd_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case of error,
 *         #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyd_node *lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, ...);

/**
 * @brief Read data from the given file descriptor.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * In case of LY_XML format, the file content is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] fd The standard file descriptor of the file containing the data tree in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree or NULL in case of empty file. To free the returned structure,
 *         use lyd_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case of error,
 *         #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyd_node *lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...);

/**
 * @brief Read data from the given file path.
 *
 * In case of LY_XML format, the file content is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] path Path to the file containing the data tree in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree or NULL in case of empty file. To free the returned structure,
 *         use lyd_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case of error,
 *         #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyd_node *lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, ...);

/**
 * @brief Parse (and validate according to appropriate schema from the given context) XML tree.
 *
 * The output data tree is parsed from the given XML tree previously parsed by one of the
 * lyxml_read* functions.
 *
 * If there are some sibling elements of the \p root (data were read with #LYXML_PARSE_MULTIROOT option
 * or the provided root is a root element of a subtree), all the sibling nodes (previous as well as
 * following) are processed as well. The returned data node is a root of the first tree with other
 * trees connected via the next pointer. This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * When the function is used with #LYD_OPT_DESTRUCT, all the successfully parsed data including the
 * XML \p root and all its siblings (if #LYD_OPT_NOSIBLINGS is not used) are freed. Only with
 * #LYD_OPT_DESTRUCT option the \p root pointer is changed - if all the data are parsed, it is set
 * to NULL, otherwise it will hold the XML tree without the successfully parsed elements.
 *
 * The context must be the same as the context used to parse XML tree by lyxml_read* function.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in,out] root XML tree to parse (convert) to data tree. By default, parser do not change the XML tree. However,
 *            when #LYD_OPT_DESTRUCT is specified in \p options, parser frees all successfully parsed data.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree or NULL in case of empty \p root. To free the returned structure,
 *         use lyd_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case of error,
 *         #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyd_node *lyd_parse_xml(struct ly_ctx *ctx, struct lyxml_elem **root, int options,...);

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
 * @param[in] ... libyang context for the data (used only in case the \p node is NULL, so in case of checking empty data tree)
 * @return 0 on success (if options include #LYD_OPT_FILTER, some nodes can be deleted as an
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
* @brief Print data tree in the specified format.
*
* Same as lyd_print(), but it allocates memory and store the data into it.
* It is up to caller to free the returned string by free().
*
* @param[out] strp Pointer to store the resulting dump.
* @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
* node of the data tree to print the specific subtree.
* @param[in] format Data output format.
* @param[in] options [printer flags](@ref printerflags).
* @return 0 on success, 1 on failure (#ly_errno is set).
*/
int lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * Same as lyd_print(), but output is written into the specified file descriptor.
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] fd File descriptor where to print the data.
 * @param[in] format Data output format.
 * @param[in] options [printer flags](@ref printerflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * To write data into a file descriptor, use lyd_print_fd().
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] f File stream where to print the data.
 * @param[in] format Data output format.
 * @param[in] options [printer flags](@ref printerflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * Same as lyd_print(), but output is written via provided callback.
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Data output format.
 * @param[in] options [printer flags](@ref printerflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                  const struct lyd_node *root, LYD_FORMAT format, int options);

/**@} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_DATA_H_ */
