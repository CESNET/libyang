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
#include "tree.h"
#include "tree_schema.h"

struct ly_ctx;

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
 * START can be any of the lyd_node* types, NEXT and ELEM variables are expected
 * to be pointers to a generic struct lyd_node.
 *
 * Since the next node is selected as part of #LYD_TREE_DFS_END, do not use
 * continue statement between the #LYD_TREE_DFS_BEGIN and #LYD_TREE_DFS_END.
 *
 * Use with opening curly bracket '{' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param NEXT Temporary storage, do not use.
 * @param ELEM Iterator intended for use in the block.
 */
#define LYD_TREE_DFS_BEGIN(START, NEXT, ELEM) \
    for ((ELEM) = (NEXT) = (START); \
         (ELEM); \
         (ELEM) = (NEXT))

/**
 * @brief Macro to iterate via all elements in a tree. This is the closing part
 * to the #LYD_TREE_DFS_BEGIN - they always have to be used together.
 *
 * Use the same parameters for #LYD_TREE_DFS_BEGIN and #LYD_TREE_DFS_END. While
 * START can be any of the lyd_node* types, NEXT and ELEM variables are expected
 * to be pointers to a generic struct lyd_node.
 *
 * Use with closing curly bracket '}' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param NEXT Temporary storage, do not use.
 * @param ELEM Iterator intended for use in the block.
 */

#define LYD_TREE_DFS_END(START, NEXT, ELEM) \
    /* select element for the next run - children first */ \
    (NEXT) = (struct lyd_node*)lyd_node_children((struct lyd_node*)ELEM); \
    if (!(NEXT)) { \
        /* no children */ \
        if ((ELEM) == (struct lyd_node*)(START)) { \
            /* we are done, (START) has no children */ \
            break; \
        } \
        /* try siblings */ \
        (NEXT) = (ELEM)->next; \
    } \
    while (!(NEXT)) { \
        /* parent is already processed, go to its sibling */ \
        (ELEM) = (struct lyd_node*)(ELEM)->parent; \
        /* no siblings, go back through parents */ \
        if ((ELEM)->parent == (START)->parent) { \
            /* we are done, no next element to process */ \
            break; \
        } \
        (NEXT) = (ELEM)->next; \
    }

/**
 * @brief Macro to get context from a data tree node.
 */
#define LYD_NODE_CTX(node) ((node)->schema->module->ctx)

/**
 * @brief Data input/output formats supported by libyang [parser](@ref howtodataparsers) and
 * [printer](@ref howtodataprinters) functions.
 */
typedef enum {
    LYD_UNKNOWN = 0,     /**< unknown format, used as return value in case of error */
    LYD_XML,             /**< XML format of the instance data */
    LYD_JSON,            /**< JSON format of the instance data */
#if 0
    LYD_LYB,             /**< LYB format of the instance data */
#endif
} LYD_FORMAT;

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
#if 0 /* TODO LYB format */
    LYD_ANYDATA_LYB,                 /**< Value is a memory chunk with the serialized data tree in LYB format. */
#endif
} LYD_ANYDATA_VALUETYPE;

/** @} */

/**
 * @brief YANG data representation
 */
struct lyd_value {
    const char *original;           /**< Original string representation of the value. It is never NULL, but (canonical) string representation
                                         of the value should be always obtained via the type's printer callback (lyd_value::realtype::plugin::print). */
    union {
        int8_t boolean;              /**< 0 as false, 1 as true */
        int64_t dec64;               /**< decimal64: value = dec64 / 10^fraction-digits  */
        int8_t int8;                 /**< 8-bit signed integer */
        int16_t int16;               /**< 16-bit signed integer */
        int32_t int32;               /**< 32-bit signed integer */
        int64_t int64;               /**< 64-bit signed integer */
        uint8_t uint8;               /**< 8-bit unsigned integer */
        uint16_t uint16;             /**< 16-bit signed integer */
        uint32_t uint32;             /**< 32-bit signed integer */
        uint64_t uint64;             /**< 64-bit signed integer */
        struct lysc_type_bitenum_item *enum_item;  /**< pointer to the definition of the enumeration value */
        struct lysc_type_bitenum_item **bits_items; /**< list of set pointers to the specification of the set bits ([sized array](@ref sizedarrays)) */
        struct lysc_ident *ident;    /**< pointer to the schema definition of the identityref value */

        struct lyd_value_subvalue {
            struct lyd_value_prefix {
                const char *prefix;           /**< prefix string used in the canonized string to identify the mod of the YANG schema */
                const struct lys_module *mod; /**< YANG schema module identified by the prefix string */
            } *prefixes;                 /**< list of mappings between prefix in canonized value to a YANG schema ([sized array](@ref sizedarrays)) */
            struct lyd_value *value;     /**< representation of the value according to the selected union's subtype (stored as lyd_value::realpath
                                              here, in subvalue structure */
        } *subvalue;                     /**< data to represent data with multiple types (union). Original value is stored in the main
                                              lyd_value:canonized while the lyd_value_subvalue::value contains representation according to the
                                              one of the union's type. The lyd_value_subvalue:prefixes provides (possible) mappings from prefixes
                                              in original value to YANG modules. These prefixes are necessary to parse original value to the union's
                                              subtypes. */

        struct lyd_value_path {
            const struct lysc_node *node; /**< Schema node representing the path segment */
            struct lyd_value_path_predicate {
                union {
                    struct {
                        const struct lysc_node *key; /**< key node of the predicate, in case of the leaf-list-predicate, it is the leaf-list node itself */
                        struct lyd_value *value;     /**< value representation according to the key's type */
                    };                    /**< key-value pair for leaf-list-predicate and key-predicate (type 1 and 2) */
                    uint64_t position;    /**< position value for the position-predicate (type 0) */
                };
                uint8_t type;        /**< Predicate types (see YANG ABNF): 0 - position, 1 - key-predicate, 2 - leaf-list-predicate */
            } *predicates;           /**< [Sized array](@ref sizedarrays) of the path segment's predicates */
        } *target;                   /**< [Sized array](@ref sizedarrays) of (instance-identifier's) path segments. */

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
    void *canonical_cache;           /**< Generic cache for type plugins to store data necessary to print canonical value. It can be the canonical
                                          value itself or anything else useful to print the canonical form of the value. Plugin is responsible for
                                          freeing the cache in its free callback. */
};

/**
 * @brief Attribute structure.
 *
 * The structure provides information about attributes of a data element. Such attributes must map to
 * annotations as specified in RFC 7952. The only exception is the filter type (in NETCONF get operations)
 * and edit-config's operation attributes. In XML, they are represented as standard XML attributes. In JSON,
 * they are represented as JSON elements starting with the '@' character (for more information, see the
 * YANG metadata RFC.
 *
 */
struct lyd_attr {
    struct lyd_node *parent;         /**< data node where the attribute is placed */
    struct lyd_attr *next;           /**< pointer to the next attribute of the same element */
    struct lysc_ext_instance *annotation; /**< pointer to the attribute/annotation's definition */
    const char *name;                /**< attribute name */
    struct lyd_value value;          /**< attribute's value representation */
};


#define LYD_NODE_INNER (LYS_CONTAINER|LYS_LIST|LYS_ACTION|LYS_NOTIF) /**< Schema nodetype mask for lyd_node_inner */
#define LYD_NODE_TERM (LYS_LEAF|LYS_LEAFLIST)   /**< Schema nodetype mask for lyd_node_term */
#define LYD_NODE_ANY (LYS_ANYDATA)   /**< Schema nodetype mask for lyd_node_any */

/**
 * @defgroup dnodeflags Data nodes flags
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
 *     ---------------------+-+-+-+-+-+-+-+
 *
 */

#define LYD_DEFAULT      0x01        /**< default (implicit) node */
#define LYD_WHEN_TRUE    0x02        /**< all when conditions of this node were evaluated to true */
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
    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */

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
    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */

#ifdef LY_ENABLED_LYD_PRIV
    void *priv;                      /**< private user data, not used by libyang */
#endif

    struct lyd_node *child;          /**< pointer to the first child node. */
    struct hash_table *children_ht;  /**< hash table with all the direct children (except keys for a list, lists without keys) */
#define LYD_HT_MIN_ITEMS 4           /**< minimal number of children to create lyd_node_inner::children_ht hash table. */
};

/**
 * @brief Data node structure for the terminal data tree nodes - leafs and leaf-lists.
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
    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */

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
    struct lyd_attr *attr;           /**< pointer to the list of attributes of this node */

#ifdef LY_ENABLED_LYD_PRIV
    void *priv;                      /**< private user data, not used by libyang */
#endif

    union {
        struct lyd_node *tree;       /**< data tree */
        const char *str;             /**< Generic string data */
        const char *xml;             /**< Serialized XML data */
        const char *json;            /**< I-JSON encoded string */
        char *mem;                   /**< LYD_ANYDATA_LYB memory chunk */
    } value;                         /**< pointer to the stored value representation of the anydata/anyxml node */
    LYD_ANYDATA_VALUETYPE value_type;/**< type of the data stored as lyd_node_any::value */
};

/**
 * @defgroup dataparseroptions Data parser options
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
 * - when parser evaluates when-stmt condition to false, a validation error is raised. If the
 * #LYD_OPT_WHENAUTODEL is used, the invalid node is silently removed instead of an error. The option (and also this default
 * behavior) takes effect only in case of #LYD_OPT_DATA or #LYD_OPT_CONFIG type of data.
 * @{
 */

#define LYD_OPT_DATA       0x00 /**< Default type of data - complete datastore content with configuration as well as
                                     state data. */
#define LYD_OPT_CONFIG     LYD_OPT_NO_STATE /**< A configuration datastore - complete datastore without state data. */
#define LYD_OPT_GET        LYD_OPT_PARSE_ONLY /**< Data content from a NETCONF reply message to the NETCONF \<get\> operation. */
#define LYD_OPT_GETCONFIG  LYD_OPT_PARSE_ONLY | LYD_OPT_NO_STATE /**< Data content from a NETCONF reply message to
                                                                      the NETCONF \<get-config\> operation. */
#define LYD_OPT_EDIT       LYD_OPT_PARSE_ONLY | LYD_OPT_EMPTY_INST /**< Content of the NETCONF \<edit-config\>'s config element. */

#define LYD_OPT_STRICT     0x0001 /**< Instead of silent ignoring data without schema definition raise an error. */
#define LYD_OPT_PARSE_ONLY 0x0002 /**< Data will be only parsed and no (only required) validation will be performed. */
#define LYD_OPT_NO_STATE   0x0004 /**< Consider state data not allowed and raise an error if they are found. */
#define LYD_OPT_EMPTY_INST 0x0008 /**< Allow leaf/leaf-list instances without values and lists without keys. */
#define LYD_OPT_VAL_DATA_ONLY 0x0010 /**< Validate only modules whose data actually exist. */
//#define LYD_OPT_DESTRUCT   0x0400 /**< Free the provided XML tree during parsing the data. With this option, the
//                                       provided XML tree is affected and all successfully parsed data are freed.
//                                       This option is applicable only to lyd_parse_xml() function. */
//#define LYD_OPT_OBSOLETE   0x0800 /**< Raise an error when an obsolete statement (status set to obsolete) is used. */
//#define LYD_OPT_NOSIBLINGS 0x1000 /**< Parse only a single XML tree from the input. This option applies only to
//                                       XML input data. */
//#define LYD_OPT_VAL_DIFF 0x40000 /**< Flag only for validation, store all the data node changes performed by the validation
//                                      in a diff structure. */
//#define LYD_OPT_DATA_TEMPLATE 0x1000000 /**< Data represents YANG data template. */

/**@} dataparseroptions */

/**
 * @brief Get the node's children list if any.
 *
 * Decides the node's type and in case it has a children list, returns it.
 * @param[in] node Node to check.
 * @return Pointer to the first child node (if any) of the \p node.
 */
const struct lyd_node *lyd_node_children(const struct lyd_node *node);

/**
 * @brief Find the node, in the list, satisfying the given restrictions.
 *
 * @param[in] first Starting child node for search.
 * @param[in] module Module of the node to find (mandatory argument).
 * @param[in] name Name of the node to find (mandatory argument).
 * @param[in] name_len Optional length of the @p name argument in case it is not NULL-terminated string.
 * @param[in] nodetype Optional mask for the nodetype of the node to find, 0 is understood as all nodetypes.
 * @param[in] value Optional restriction for lyd_node_term nodes to select node with the specific value. Note that this
 * search restriction is limited to compare original string representation of the @p first value. Some of the types have
 * canonical representation defined so the same value can be represented by multiple lexical representation. In such a
 * case the @p value not matching the original representation of @p first may still be the same.
 * For such a case there is more advanced lyd_value_compare() to check if the values matches.
 * @param[in] value_len Optional length of the @p value argument in case it is not NULL-terminated string.
 * @return The sibling node of the @p first (or itself), satisfying the given restrictions.
 * @return NULL in case there is no node satisfying the restrictions.
 */
const struct lyd_node *lyd_search(const struct lyd_node *first, const struct lys_module *module,
                                  const char *name, size_t name_len, uint16_t nodetype, const char *value, size_t value_len);

/**
 * @brief Parse (and validate) data from memory.
 *
 * In case of LY_XML format, the data string is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] data Serialized data in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions. \p format LYD_LYB uses #LYD_OPT_PARSE_ONLY implicitly.
 * @return Pointer to the built data tree or NULL in case of empty \p data. To free the returned structure,
 *         use lyd_free_all().
 * @return NULL in case of error. The error information can be then obtained using ly_err* functions.
 */
struct lyd_node *lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options);

/**
 * @brief Read (and validate) data from the given file descriptor.
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
 * @param[in] options Parser options, see @ref parseroptions. \p format LYD_LYB uses #LYD_OPT_PARSE_ONLY implicitly.
 * @return Pointer to the built data tree or NULL in case of empty file. To free the returned structure,
 *         use lyd_free_all().
 * @return NULL in case of error. The error information can be then obtained using ly_err* functions.
 */
struct lyd_node *lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options);

/**
 * @brief Read (and validate) data from the given file path.
 *
 * In case of LY_XML format, the file content is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] path Path to the file containing the data tree in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions. \p format LYD_LYB uses #LYD_OPT_PARSE_ONLY implicitly.
 * @return Pointer to the built data tree or NULL in case of empty file. To free the returned structure,
 *         use lyd_free_all().
 * @return NULL in case of error. The error information can be then obtained using ly_err* functions.
 */
struct lyd_node *lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options);

/**
 * @brief Free all the nodes (even parents of the node) in the data tree.
 *
 * @param[in] node Any of the nodes inside the tree.
 */
void lyd_free_all(struct lyd_node *node);

/**
 * @brief Free all the sibling nodes.
 *
 * @param[in] node Any of the sibling nodes to free.
 */
void lyd_free_withsiblings(struct lyd_node *node);

/**
 * @brief Free (and unlink) the specified data (sub)tree.
 *
 * __PARTIAL CHANGE__ - validate after the final change on the data tree (see @ref howtodatamanipulators).
 *
 * @param[in] node Root of the (sub)tree to be freed.
 */
void lyd_free_tree(struct lyd_node *node);

/**
 * @brief Unlink the specified data subtree. All referenced namespaces are copied.
 *
 * Note, that the node's connection with the schema tree is kept. Therefore, in case of
 * reconnecting the node to a data tree using lyd_paste() it is necessary to paste it
 * to the appropriate place in the data tree following the schema.
 *
 * __PARTIAL CHANGE__ - validate after the final change on the data tree (see @ref howtodatamanipulators).
 *
 * @param[in] node Data tree node to be unlinked (together with all children).
 * @return LY_SUCCESS for success
 * @return LY_E* values in case of error
 */
LY_ERR lyd_unlink_tree(struct lyd_node *node);

/**
 * @brief Destroy data attribute.
 *
 * @param[in] ctx Context where the attribute was created.
 * @param[in] attr Attribute to destroy
 * @param[in] recursive Zero to destroy only the attribute (the attribute list is corrected),
 * non-zero to destroy also all the subsequent attributes in the list.
 */
void lyd_free_attr(struct ly_ctx *ctx, struct lyd_attr *attr, int recursive);

/**
 * @brief Prepare ([sized array](@ref sizedarrays)) of data trees required by various (mostly validation) functions.
 *
 * @param[in] count Number of trees to include (including the mandatory @p tree).
 * @param[in] tree First (and mandatory) tree to be included into the resulting ([sized array](@ref sizedarrays)).
 * @return NULL in case of memory allocation failure or invalid argument, prepared ([sized array](@ref sizedarrays)) otherwise.
 */
const struct lyd_node **lyd_trees_new(size_t count, const struct lyd_node *tree, ...);

/**
 * @brief Add tree into the ([sized array](@ref sizedarrays)) of data trees created by lyd_trees_new(),
 *
 * @param[in] trees Existing [sized array](@ref sizedarrays)) of data trees to be extended.
 * @param[in] tree Data tree to be included into the provided @p trees ([sized array](@ref sizedarrays)).
 * @return NULL in case of memory allocation failure or invalid argument, extended @p trees ([sized array](@ref sizedarrays)) otherwise.
 */
const struct lyd_node **lyd_trees_add(const struct lyd_node **trees, const struct lyd_node *tree);

/**
 * @brief Free the trees ([sized array](@ref sizedarrays)).
 *
 * @param[in] trees ([Sized array](@ref sizedarrays)) of data trees.
 * @param[in] free_data Flag to free also the particular trees in the @p trees ([sized array](@ref sizedarrays)).
 * If set to zero, only the trees envelope is freed and data are untouched.
 */
void lyd_trees_free(const struct lyd_node **trees, int free_data);

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
 * @param[in] value String value to be checked.
 * @param[in] value_len Length of the given @p value (mandatory).
 * @param[in] get_prefix Callback function to resolve prefixes used in the @p value string.
 * @param[in] get_prefix_data Private data for the @p get_prefix callback.
 * @param[in] format Input format of the data.
 * @param[in] trees ([Sized array](@ref sizedarrays)) of data trees (e.g. when validating RPC/Notification) where the required
 *            data instance (leafref target, instance-identifier) can be placed. NULL in case the data tree are not yet complete,
 *            then LY_EINCOMPLETE can be returned. To simply prepare this structure, use lyd_trees_new().
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case the @p trees is not provided and it was needed to finish the validation (e.g. due to require-instance).
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_validate(struct ly_ctx *ctx, const struct lyd_node_term *node, const char *value, size_t value_len,
                          ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format, const struct lyd_node **trees);

/**
 * @brief Compare the node's value with the given string value. The string value is first validated according to the node's type.
 *
 * @param[in] node Data node to compare.
 * @param[in] value String value to be compared. It does not need to be in a canonical form - as part of the process,
 * it is validated and canonized if possible.
 * @param[in] value_len Length of the given @p value (mandatory).
 * @param[in] get_prefix Callback function to resolve prefixes used in the @p value string.
 * @param[in] get_prefix_data Private data for the @p get_prefix callback.
 * @param[in] format Input format of the data.
 * @param[in] trees ([Sized array](@ref sizedarrays)) of data trees (e.g. when validating RPC/Notification) where the required
 *            data instance (leafref target, instance-identifier) can be placed. NULL in case the data tree are not yet complete,
 *            then LY_EINCOMPLETE can be returned in case the validation was not completed, but values matches. To simply prepare
 *            this structure, use lyd_trees_new(). To simply prepare this structure, use lyd_trees_new().
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case of success when the @p trees is not provided and it was needed to finish the validation of
 * the given string @p value (e.g. due to require-instance).
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_compare(const struct lyd_node_term *node, const char *value, size_t value_len,
                         ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format, const struct lyd_node **trees);

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
/**@} datacompareoptions */

/**
 * @brief Compare 2 data nodes if they are equivalent.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @param[in] options Various @ref datacompareoptions.
 * @return LY_SUCCESS if the nodes are equivalent.
 * @return LY_ENOT if the nodes are not equivalent.
 */
LY_ERR lyd_compare(const struct lyd_node *node1, const struct lyd_node *node2, int options);

/**
 * @defgroup dupoptions Data duplication options
 * @ingroup datatree
 *
 * Various options to change lyd_dup() behavior.
 *
 * Default behavior:
 * - only the specified node is duplicated without siblings, parents, or children.
 * - all the attributes of the duplicated nodes are also duplicated.
 * @{
 */

#define LYD_DUP_RECURSIVE    0x01  /**< Duplicate not just the node but also all the children. Note that
                                        list's keys are always duplicated. */
#define LYD_DUP_NO_ATTR      0x02  /**< Do not duplicate attributes of any node. */
#define LYD_DUP_WITH_PARENTS 0x04  /**< If a nested node is being duplicated, duplicate also all the parents.
                                        Keys are also duplicated for lists. Return value does not change! */
#define LYD_DUP_WITH_SIBLINGS 0x08 /**< Duplicate also all the sibling of the given node. */
#define LYD_DUP_WITH_WHEN     0x10 /**< Also copy any when evaluation state flags. This is useful in case the copied
                                        nodes are actually still part of the same datastore meaning no dependency data
                                        could have changed. Otherwise nothing is assumed about the copied node when
                                        state and it is evaluated from scratch during validation. */

/** @} dupoptions */

/**
 * @brief Create a copy of the specified data tree \p node. Schema references are kept the same.
 *
 * __PARTIAL CHANGE__ - validate after the final change on the data tree (see @ref howtodatamanipulators).
 *
 * @param[in] node Data tree node to be duplicated.
 * @param[in] parent Optional parent node where to connect the duplicated node(s).
 * If set in combination with LYD_DUP_WITH_PARENTS, the parents chain is duplicated until it comes to and connect with the @p parent
 * (if the parents chain does not match at some node the schema node of the provided @p parent, duplication fails).
 * @param[in] options Bitmask of options flags, see @ref dupoptions.
 * @return Created copy of the provided data \p node (the first of the duplicated siblings when LYD_DUP_WITH_SIBLINGS used).
 * Note that in case the parents chain is duplicated for the duplicated node(s) (when LYD_DUP_WITH_PARENTS used), the first duplicated node
 * is still returned, not a pointer to the duplicated parents.
 */
struct lyd_node *lyd_dup(const struct lyd_node *node, struct lyd_node_inner *parent, int options);

/**
 * @brief Resolve instance-identifier defined by lyd_value_path structure.
 *
 * @param[in] path Path structure specifying the instance-identifier target.
 * @param[in] trees ([Sized array](@ref sizedarrays)) of data trees to be searched.
 *            To simply prepare this structure, use lyd_trees_new().
 * @return Target node of the instance-identifier present in the given data @p trees.
 */
const struct lyd_node_term *lyd_target(struct lyd_value_path *path, const struct lyd_node **trees);

/**
 * @brief Get string value of a term data \p node.
 *
 * @param[in] node Data tree node with the value.
 * @param[out] dynamic Whether the string value was dynmically allocated.
 * @return String value of @p node, if @p dynamic, needs to be freed.
 */
const char *lyd_value2str(const struct lyd_node_term *node, int *dynamic);

/**
 * @brief Get string value of an attribute \p attr.
 *
 * @param[in] attr Attribute with the value.
 * @param[out] dynamic Whether the string value was dynmically allocated.
 * @return String value of @p attr, if @p dynamic, needs to be freed.
 */
const char *lyd_attr2str(const struct lyd_attr *attr, int *dynamic);

/**
 * @brief Types of the different data paths.
 */
typedef enum {
    LYD_PATH_LOG, /**< Descriptive path format used in log messages */
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

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_DATA_H_ */
