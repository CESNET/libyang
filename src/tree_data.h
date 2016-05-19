/**
 * @file tree_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of data trees.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
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
#define LYD_VAL_NOT      0x07    /**< node was not validated yet */
#define LYD_VAL_INUSE    0x08    /**< Internal flag for note about various processing on data, should be used only
                                      internally and removed before the libyang returns to the caller */
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
    uint8_t validity:4;              /**< [validity flags](@ref validityflags) */
    uint8_t dflt:1;                  /**< flag for default node (applicable only on leafs) to be marked with default attribute */
    uint8_t when_status:3;           /**< bit for checking if the when-stmt condition is resolved - internal use only,
                                          do not use this value! */

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
    uint8_t validity:4;              /**< [validity flags](@ref validityflags) */
    uint8_t dflt:1;                  /**< flag for default node (applicable only on leafs) to be marked with default attribute */
    uint8_t when_status:3;           /**< bit for checking if the when-stmt condition is resolved - internal use only,
                                          do not use this value! */

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

union lyd_node_anyxml_value {
    const char *str;
    struct lyxml_elem *xml;
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
    uint8_t validity:4;              /**< [validity flags](@ref validityflags) */
    uint8_t dflt:1;                  /**< flag for default node (applicable only on leafs) to be marked with default attribute */
    uint8_t when_status:3;           /**< bit for checking if the when-stmt condition is resolved - internal use only,
                                          do not use this value! */

    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */
    struct lyd_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lyd_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    struct lyd_node *parent;         /**< pointer to the parent node, NULL in case of root node */

    /* struct lyd_node *child; should be here, but is not */

    /* anyxml's specific members */
    uint8_t xml_struct;              /**< 1 for value.xml, 0 for value.str */
    union lyd_node_anyxml_value value; /**< anyxml value, everything is in the dictionary, there can be more XML siblings */
};

/**
 * @brief list of possible types of differencies in #lyd_difflist
 */
typedef enum {
    LYD_DIFF_END = 0,        /**< end of the differences list */
    LYD_DIFF_DELETED,        /**< deleted node
                                  - Node is present in the first tree, but not in the second tree.
                                  - To make both trees the same the node in lyd_difflist::first can be deleted from the
                                    first tree. The pointer at the same index in the lyd_difflist::second array is
                                    NULL */
    LYD_DIFF_CHANGED,        /**< value of a leaf or anyxml is changed, the lyd_difflist::first and lyd_difflist::second
                                  points to the leaf/anyxml instances in the first and the second tree respectively. */
    LYD_DIFF_MOVEDAFTER1,    /**< user-ordered (leaf-)list item was moved.
                                  - To make both trees the same, all #LYD_DIFF_MOVEDAFTER1 transactions must be applied
                                  to the first tree in the strict order they appear in the difflist. The
                                  lyd_difflist::first points to the first tree node being moved and the
                                  lyd_difflist::second points to the first tree node after which the first node is
                                  supposed to be moved. If the second pointer is NULL, the node is being moved into
                                  the beginning as the first node of the (leaf-)list instances. */
    LYD_DIFF_CREATED,        /**< newly created node
                                  - Node is present in the second tree, but not in the first tree.
                                  - To make both trees the same the node in lyd_difflist::second is supposed to be
                                    inserted (copied via lyd_dup()) into the node (as a child) at the same index in the
                                    lyd_difflist::first array (where is its parent). If the lyd_difflist::first at the
                                    index is NULL, the missing node is top-level. */
    LYD_DIFF_MOVEDAFTER2     /**< similar to LYD_DIFF_MOVEDAFTER1, but this time the moved item is in the second tree.
                                  This type is always used in combination with (as a successor of) #LYD_DIFF_CREATED
                                  as an instruction to move the newly created node to a specific position. Note, that
                                  due to applicability to the second tree, the meaning of lyd_difflist:first and
                                  lyd_difflist:second is inverse in comparison to #LYD_DIFF_MOVEDAFTER1. The
                                  lyd_difflist::second points to the (previously) created node in the second tree and
                                  the lyd_difflist::first points to the predecessor node in the second tree. If the
                                  predecessor is NULL, the node is supposed to bes the first sibling. */
} LYD_DIFFTYPE;

/**
 * @brief Structure for the result of lyd_diff(), describing differences between two data trees.
 */
struct lyd_difflist {
    LYD_DIFFTYPE *type;      /**< array of the differences types, terminated by #LYD_DIFF_END value. */
    struct lyd_node **first; /**< array of nodes in the first tree for the specific type of difference, see the
                                  description of #LYD_DIFFTYPE values for more information. */
    struct lyd_node **second;/**< array of nodes in the second tree for the specific type of difference, see the
                                  description of #LYD_DIFFTYPE values for more information. */
};

/**
 * @brief Free the result of lyd_diff(). It frees the structure of the lyd_diff() result, not the referenced nodes.
 *
 * @param[in] diff The lyd_diff() result to free.
 */
void lyd_free_diff(struct lyd_difflist *diff);

/**
 * @brief Compare two data trees and provide list of differences.
 *
 * Note, that the \p first and the \p second must have the same schema parent (or they must be top-level elements).
 * In case of using #LYD_OPT_NOSIBLINGS, they both must be instances of the same schema node.
 *
 * Order of the resulting set follows these rules:
 * - To change the first tree into the second tree, the resulting transactions are supposed to be applied in the order
 *   they appear in the result. First, the changed (#LYD_DIFF_CHANGED) nodes are described followed by the deleted
 *   (#LYD_DIFF_DELETED) nodes. Then, the moving of the user-ordered nodes present in both trees (#LYD_DIFF_MOVEDAFTER1)
 *   follows and the last transactions in the results are the newly created (#LYD_DIFF_CREATED) nodes. These nodes are
 *   supposed to be added as the last siblings, but in some case they can need additional move. In such a case, the
 *   #LYD_DIFF_MOVEDAFTER2 transactions can appear.
 * - The order of the changed (#LYD_DIFF_CHANGED) and created (#LYD_DIFF_CREATED) follows the nodes order in the
 *   second tree - the current siblings are processed first and then the children are processed. Note, that this is
 *   actually not the BFS:
 *
 *           1     2
 *          / \   / \
 *         3   4 7   8
 *        / \
 *       5   6
 *
 * - The order of the deleted (#LYD_DIFF_DELETED) nodes is the DFS:
 *
 *           1     6
 *          / \   / \
 *         2   5 7   8
 *        / \
 *       3   4
 *
 * To change the first tree into the second one, it is necessary to follow the order of transactions described in
 * the result. Note, that it is not possible just to use the transactions in the reverse order to transform the
 * second tree into the first one. The transactions can be generalized (to be used on a different instance of the
 * first tree) using lyd_path() to get identifiers for the nodes used in the transactions.
 *
 * @param[in] first The first (sub)tree to compare. Without #LYD_OPT_NOSIBLINGS option, all siblings are
 *            taken into comparison. If NULL, all the \p second nodes are supposed to be top level and they will
 *            be marked as #LYD_DIFF_CREATED.
 * @param[in] second The second (sub)tree to compare. Without #LYD_OPT_NOSIBLINGS option, all siblings are
 *            taken into comparison. If NULL, all the \p first nodes will be marked as #LYD_DIFF_DELETED.
 * @param[in] options The following @ref parseroptions with the described meanings are accepted:
 *            - #LYD_OPT_NOSIBLINGS - the \p first and the \p second have to instantiate the same schema node and
 *              only their content (subtree) is compared.
 * @return NULL on error, the list of differences on success. In case the trees are the same, the first item in the
 *         lyd_difflist::type array is #LYD_DIFF_END. The returned structure is supposed to be freed by lyd_free_diff().
 */
struct lyd_difflist *lyd_diff(struct lyd_node *first, struct lyd_node *second, int options);

/**
 * @brief Build path (usable as XPath) of the data node.
 * @param[in] node Data node to be processed. Note that the node should be from a complete data tree, having a subtree
 *            (after using lyd_unlink()) can cause generating invalid paths.
 * @return NULL on error, on success the buffer for the resulting path is allocated and caller is supposed to free it
 * with free().
 */
char *lyd_path(struct lyd_node *node);

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
 * - when parser evaluates when-stmt condition to false, the constrained subtree is automatically removed. If the
 * #LYD_OPT_NOAUTODEL is used, error is raised instead of silent auto delete. The option (and also this default
 * behavior) takes effect only in case of #LYD_OPT_DATA or #LYD_OPT_CONFIG type of data.
 * - whenever the parser see empty non-presence container, it is automatically removed to minimize memory usage. This
 * behavior can be changed by #LYD_OPT_KEEPEMPTYCONT.
 * - for validation, parser needs to add default nodes into the data tree. By default, these additional (implicit)
 * nodes are removed before the parser returns. However, if caller use one of the LYD_WD_* option, the default nodes
 * added by parser are kept in the resulting tree or even the explicit nodes with the default values can be removed
 * (in case of #LYD_WD_TRIM option).
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
/* 0x80 reserved, formerly LYD_OPT_FILTER */
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
#define LYD_OPT_NOAUTODEL  0x2000 /**< Avoid automatic delete of subtrees with false when-stmt condition. The flag is
                                       applicable only in combination with LYD_OPT_DATA and LYD_OPT_CONFIG flags.
                                       If used, libyang generates validation error instead of silently removing the
                                       constrained subtree. */
#define LYD_OPT_KEEPEMPTYCONT 0x4000 /**< Do not automatically delete empty non-presence containers. */

#define LYD_WD_MASK        0x1F0000  /**< Mask for with-defaults modes */
#define LYD_WD_EXPLICIT    0x100000  /**< Explicit mode - add missing default status data, but only in case the data
                                          type is supposed to include status data (all except #LYD_OPT_CONFIG,
                                          #LYD_OPT_GETCONFIG and #LYD_OPT_EDIT */
#define LYD_WD_TRIM        0x010000  /**< Remove all nodes with the value equal to their default value */
#define LYD_WD_ALL         0x020000  /**< Explicitly add all missing nodes with their default value */
#define LYD_WD_ALL_TAG     0x040000  /**< Same as LYD_WD_ALL but also adds attribute 'default' with value 'true' to
                                          all nodes that has its default value. The 'default' attribute has namespace:
                                          urn:ietf:params:xml:ns:netconf:default:1.0 and thus the attributes are
                                          created only when the ietf-netconf-with-defaults module is present in libyang
                                          context. */
#define LYD_WD_IMPL_TAG    0x080000  /**< Same as LYD_WD_ALL_TAG but the attributes are added only to the nodes that
                                          are being created and were not part of the original data tree despite their
                                          value is equal to their default value. There is the same limitation regarding
                                          the presence of ietf-netconf-with-defaults module in libyang context. */

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
 * Also, changing the value of a list key is prohibited.
 *
 * As for the other data tree manipulation functions, the change is not fully validated to allow multiple changes
 * in the data tree. Therefore, when all changes on the data tree are done, caller is supposed to call lyd_validate()
 * to check that the result is valid data tree. Specifically, if a leafref leaf is changed, it is not checked that
 * the (leafref) value is correct.
 *
 * @param[in] leaf A leaf node to change.
 * @param[in] val_str String form of the new value to be set to the \p leaf. In case the type is #LY_TYPE_INST
 * or #LY_TYPE_IDENT, JSON node-id format is expected (nodes are prefixed with module names, not XML namespaces).
 * @return 0 on success, non-zero on error.
 */
int lyd_change_leaf(struct lyd_node_leaf_list *leaf, const char *val_str);

/**
 * @brief Create a new anyxml node in a data tree with a string value.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node.
 * @param[in] val_str Well-formed XML string value of the node being created. Must be dynamically allocated
 * and is freed with the data.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_anyxml_str(struct lyd_node *parent, const struct lys_module *module, const char *name,
                                    char *val_str);

/**
 * @brief Create a new anyxml node in a data tree with an XML structure value.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node.
 * @param[in] val_xml XML structure value of the node being created. There can be more siblings,
 * they are freed with the data.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_anyxml_xml(struct lyd_node *parent, const struct lys_module *module, const char *name,
                                    struct lyxml_elem *val_xml);

/**
 * @brief Create a new container node in a data tree. Ignore RPC input nodes and instead use RPC output ones.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node. The node can be #LYS_CONTAINER, #LYS_LIST,
 * #LYS_NOTIF, or #LYS_RPC.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_output(struct lyd_node *parent, const struct lys_module *module, const char *name);

/**
 * @brief Create a new leaf or leaflist node in a data tree with a string value that is converted to
 * the actual value. Ignore RPC input nodes and instead use RPC output ones.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node.
 * @param[in] val_str String form of the value of the node being created. In case the type is #LY_TYPE_INST
 * or #LY_TYPE_IDENT, JSON node-id format is expected (nodes are prefixed with module names, not XML namespaces).
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_output_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name,
                                     const char *val_str);

/**
 * @brief Create a new anyxml node in a data tree with a string value. Ignore RPC input nodes and instead use
 * RPC output ones.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node.
 * @param[in] val_str Well-formed XML string value of the node being created. Must be dynamically allocated
 * and is freed with the data.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_output_anyxml_str(struct lyd_node *parent, const struct lys_module *module, const char *name,
                                           char *val_str);

/**
 * @brief Create a new anyxml node in a data tree with an XML structure value. Ignore RPC input nodes and
 * instead use RPC output ones.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating top level element.
 * @param[in] module Module with the node being created.
 * @param[in] name Schema node name of the new data node.
 * @param[in] val_xml XML structure value of the node being created. There can be more siblings,
 * they are freed with the data.
 * @return New node, NULL on error.
 */
struct lyd_node *lyd_new_output_anyxml_xml(struct lyd_node *parent, const struct lys_module *module, const char *name,
                                           struct lyxml_elem *val_xml);

/**
 * @defgroup pathoptions Data path creation options
 * @ingroup datatree
 *
 * Various options to change lyd_new_path() behavior.
 *
 * Default behavior:
 * - if the target node already exists, an error is returned.
 * - the whole path to the target node is created (with any missing parents) if necessary.
 * - RPC output schema children are completely ignored in all modules. Input is searched and nodes created normally.
 * @{
 */

#define LYD_PATH_OPT_UPDATE   0x01 /**< If the target node exists and is a leaf, it is updated with the new value and returned.
                                        If the target node exists and is not a leaf, NULL is returned and no error set. */
#define LYD_PATH_OPT_NOPARENT 0x02 /**< If any parents of the target node exist, return an error. */
#define LYD_PATH_OPT_OUTPUT   0x04 /**< Changes the behavior to ignoring RPC input schema nodes and using only output ones. */

/** @} pathoptions */

/**
 * @brief Create a new data node based on a simple XPath.
 *
 * The new node is normally inserted at the end, either as the last child of a parent or as the last sibling
 * if working with top-level elements. However, when manipulating RPC input or output, schema ordering is
 * required and always guaranteed.
 *
 * If \p path points to a list key and the list does not exist, the key value from the predicate is used
 * and \p value is ignored.
 *
 * @param[in] data_tree Existing data tree to add to/modify. It is expected to be valid. If creating RPCs,
 * there should only be one RPC and either input or output. Can be NULL.
 * @param[in] ctx Context to use. Mandatory if \p data_tree is NULL.
 * @param[in] path Simple data XPath of the new node. It can contain only simple node addressing with optional
 * module names as prefixes. List nodes must have predicates, one for each list key in the correct order and
 * with its value as well, leaves and leaf-lists can have predicates too that have preference over \p value,
 * see @ref howtoxpath.
 * @param[in] value Value of the new leaf/lealf-list. If creating anyxml, this value is internally duplicated
 * (for other options use lyd_*_new_anyxml_*()). If creating nodes of other types, set to NULL.
 * @param[in] options Bitmask of options flags, see @ref pathoptions.
 * @return First created (or updated with #LYD_PATH_OPT_UPDATE) node,
 * NULL if #LYD_PATH_OPT_UPDATE was used and the full path exists or the leaf original value matches \p value,
 * NULL and ly_errno is set on error.
 */
struct lyd_node *lyd_new_path(struct lyd_node *data_tree, struct ly_ctx *ctx, const char *path, const char *value,
                              int options);

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
 * @brief Merge a (sub)tree into a data tree. Missing nodes are merged, leaf values updated.
 * If \p target and \p source do not share the top-level schema node, even if they
 * are from different modules, \p source parents up to top-level node will be created and
 * linked to the \p target (but only containers can be created this way, lists need keys,
 * so if lists are missing, an error will be returned).
 *
 * In short, this function will always try to return a fully valid data tree and will fail
 * if it is not possible. Also, in some less common cases, despite both trees \p target and
 * \p source are valid, the resulting tree may be invalid and this function will succeed.
 * If you know there are such possibilities in your data trees or you are not sure, always
 * validate the resulting merged \p target tree.
 *
 * @param[in] target Top-level (or an RPC output child) data tree to merge to. Must be valid.
 * @param[in] source Data tree to merge \p target with. Must be valid (at least as a subtree).
 * @param[in] options Bitmask of 2 option flags:
 * LYD_OPT_DESTRUCT - spend \p source in the function, otherwise \p source is left untouched,
 * LYD_OPT_NOSIBLINGS - merge only the \p source subtree (ignore siblings), otherwise merge
 * \p source and all its succeeding siblings (preceeding ones are still ignored!).
 * @return 0 on success, nonzero in case of an error.
 */
int lyd_merge(struct lyd_node *target, const struct lyd_node *source, int options);

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
 * @param[in] node The data tree node to be inserted. If the node is connected somewhere, it is unlinked first.
 * @return 0 on success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Insert the \p node element after the \p sibling element. If \p node and \p siblings are already
 * siblings (just moving \p node position), skip validation.
 *
 * @param[in] sibling The data tree node before which the \p node will be inserted. If \p node and \p siblings
 * are already siblings (just moving \p node position), skip validation.
 * @param[in] node The data tree node to be inserted. If the node is connected somewhere, it is unlinked first.
 * @return 0 on success, nonzero in case of error, e.g. when the node is being inserted to an inappropriate place
 * in the data tree.
 */
int lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Order siblings according to the schema node ordering.
 *
 * If the siblings include data nodes from other modules, they are
 * sorted based on the module order in the context.
 *
 * @param[in] sibling Node, whose siblings will be sorted.
 * @param[in] recursive Whether sort all siblings of siblings, recursively.
 * @return 0 on success, nonzero in case of an error.
 */
int lyd_schema_sort(struct lyd_node *sibling, int recursive);

/**
 * @brief Search in the given data for instances of nodes matching the provided XPath expression.
 *
 * The \p data is used to find the data root and function then searches in the whole tree and all sibling trees.
 * The XPath expression is evaluated on data -> skip all non-data nodes (input, output, choice, case).
 *
 * Expr examples:
 *      "/ietf-yang-library:modules-state/module[name = 'ietf-yang-library']/namespace"
 *      "/ietf-netconf:get-config/source"
 *
 * @param[in] data Node in the data tree considered the context node. If the node is a configuration one,
 * any state nodes in its tree are not accessible!
 * @param[in] expr XPath expression filtering the matching nodes.
 * @return Set of found data nodes (use dset member of ::ly_set). If no nodes are matching \p expr or the result
 * would be a number, a string, or a boolean, the returned set is empty. In case of an error, NULL is returned.
 */
struct ly_set *lyd_get_node(const struct lyd_node *data, const char *expr);

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
struct ly_set *lyd_get_node2(const struct lyd_node *data, const struct lys_node *schema);

/**
 * @brief Validate \p node data subtree.
 *
 * @param[in,out] node Data tree to be validated. In case the \p options does not includes #LYD_OPT_NOAUTODEL, libyang
 *                 can modify the provided tree including the root \p node.
 * @param[in] options Options for the inserting data to the target data tree options, see @ref parseroptions.
 * @param[in] ... libyang context for the data (used only in case the \p node is NULL, so in case of checking empty data tree).
 * @return 0 on success, nonzero in case of an error.
 */
int lyd_validate(struct lyd_node **node, int options, ...);

/**
 * @brief Add default nodes into the data tree.
 *
 * The function expects that the provided data tree is valid. If not, the result is undefined - in general, the
 * result is not more invalid than the provided data tree input, so if the input data tree is invalid, result will
 * be also invalid and the process of adding default values could be incomplete.
 *
 * Since default nodes are also needed by the validation process, to optimize your application you can add default
 * values directly in lyd_validate() and lyd_parse*() functions using appropriate options value. By default, these
 * functions remove the default nodes at the end of their processing.
 *
 * \p ctx parameter and \p options with #LYD_OPT_NOSIBLINGS values can result in 4 different scenarios:
 *
 * - If \p ctx is set and \p options include #LYD_OPT_NOSIBLINGS, tree default nodes will be added ONLY to \p root,
 * top-level default nodes will be added from ALL the modules (so it has no effect for #LYD_WD_TRIM).
 * - If \p ctx is set and \p options do not include #LYD_OPT_NOSIBLINGS, tree default values will be added to ALL
 * the \p root siblings, top-level nodes will be added from ALL the modules.
 * - If \p ctx is NULL and \p options include #LYD_OPT_NOSIBLINGS, tree default nodes will be added ONLY to \p root,
 * top-level default nodes will be added ONLY from the module of \p root.
 * - If \p ctx is NULL and \p options do not include #LYD_OPT_NOSIBLINGS, tree default nodes will be added to ALL
 * the \p root siblings, top-level default nodes will be added from ALL the \p root siblings modules.
 *
 * @param[in] ctx Optional parameter. Exact meaning described in this function description last paragraph.
 * @param[in] root Data tree root. In case of #LYD_WD_TRIM the data tree can be modified so the root can be changed or
 *            removed. In other modes and with empty data tree, new default nodes can be created so the root pointer
 *            will contain/return the newly created data tree.
 * @param[in] options Options for the inserting data to the target data tree options, see @ref parseroptions - only the
 *            LYD_WD_* options are used to select functionality:
 * - #LYD_WD_TRIM - remove all nodes that have value equal to their default value
 * - #LYD_WD_EXPLICIT - add only status default nodes
 * - #LYD_WD_ALL - add all (status as well as config) default nodes
 * - #LYD_WD_ALL_TAG - add default nodes and add attribute 'default' with value 'true' to all nodes having their default value
 * - #LYD_WD_IMPL_TAG - add default nodes, but add attribute 'default' only to the added nodes
 * @note The LYD_WD_*_TAG modes require to have ietf-netconf-with-defaults module in the context of the data tree.
 * @note If you already added some tagged default nodes (by parser or previous call of lyd_validate()), you should
 * specify the same LYD_WD_*_TAG in all subsequent call to lyd_validate(). Otherwise, the tagged nodes will be removed.
 * @return EXIT_SUCCESS ot EXIT_FAILURE
 */
int lyd_wd_add(struct ly_ctx *ctx, struct lyd_node **root, int options);

/**
 * @brief Remove all default nodes, respectively all nodes with set ::lyd_node#dflt added by
 * #LYD_WD_ALL_TAG or #LYD_WD_IMPL_TAG options in lyd_wd_add(), lyd_validate() or lyd_parse_*() functions.
 *
 * @param[in] root Data tree root. The data tree can be modified so the root can be changed or completely removed.
 * @param[in] options Options for the inserting data to the target data tree options, see @ref parseroptions.
 *            If #LYD_WD_EXPLICIT is found in options, the default status nodes are kept, so it is better to
 *            erase all LYD_WD_* flags from the value.
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int lyd_wd_cleanup(struct lyd_node **root, int options);

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
 * @param[in] mod An alternative way to specify attribute's module (namespace) used in case the \p name does
 *            not include prefix. If neither prefix in the \p name nor mod is specified, the attribute's
 *            module is inherited from the \p parent node. It is not allowed to have attributes with no
 *            module (namespace).
 * @param[in] name Attribute name. The string can include the attribute's module (namespace) as the name's
 *            prefix (prefix:name). Prefix must be the name of one of the schema in the \p parent's context.
 *            If the prefix is not specified, the \p mod parameter is used. If neither of these parameters is
 *            usable, attribute inherits module (namespace) from the \p parent node. It is not allowed to
 *            have attributes with no module (namespace).
 * @param[in] value Attribute value
 * @return pointer to the created attribute (which is already connected in \p parent) or NULL on error.
 */
struct lyd_attr *lyd_insert_attr(struct lyd_node *parent, const struct lys_module *mod, const char *name,
                                 const char *value);

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
 * @brief Return main module of the data tree node.
 *
 * In case of regular YANG module, it returns ::lys_node#module pointer,
 * but in case of submodule, it returns pointer to the main module.
 *
 * @param[in] node Data tree node to be examined
 * @return pointer to the main module (schema structure), NULL in case of error.
 */
struct lys_module *lyd_node_module(const struct lyd_node *node);

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
