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
 * Use the same parameters for #LY_TREE_DFS_BEGIN and #LY_TREE_DFS_END. While
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
 * @brief Data input/output formats supported by libyang [parser](@ref howtodataparsers) and
 * [printer](@ref howtodataprinters) functions.
 */
typedef enum {
    LYD_UNKNOWN = 0,     /**< unknown format, used as return value in case of error */
    LYD_XML,             /**< XML format of the instance data */
#if 0
    LYD_JSON,            /**< JSON format of the instance data */
    LYD_LYB,             /**< LYB format of the instance data */
#endif
} LYD_FORMAT;

/**
 * @brief List of possible value types stored in ::lyd_node_anydata.
 */
typedef enum {
    LYD_ANYDATA_CONSTSTRING = 0x00, /**< value is constant string (const char *) which is internally duplicated for
                                         storing in the anydata structure; XML sensitive characters (such as & or \>)
                                         are automatically escaped when the anydata is printed in XML format. */
    LYD_ANYDATA_STRING = 0x01,      /**< value is dynamically allocated string (char*), so the data are used directly
                                         without duplication and caller is supposed to not manipulate with the data
                                         after a successful call (including calling free() on the provided data); XML
                                         sensitive characters (such as & or \>) are automatically escaped when the
                                         anydata is printed in XML format */
    LYD_ANYDATA_JSON = 0x02,        /**< value is string containing the data modeled by YANG and encoded as I-JSON. The
                                         string is handled as constant string. In case of using the value as input
                                         parameter, the #LYD_ANYDATA_JSOND can be used for dynamically allocated
                                         string. */
    LYD_ANYDATA_JSOND = 0x03,       /**< In case of using value as input parameter, this enumeration is supposed to be
                                         used for dynamically allocated strings (it is actually combination of
                                         #LYD_ANYDATA_JSON and #LYD_ANYDATA_STRING (and it can be also specified as
                                         ORed value of the mentioned values. */
    LYD_ANYDATA_SXML = 0x04,        /**< value is string containing the serialized XML data. The string is handled as
                                         constant string. In case of using the value as input parameter, the
                                         #LYD_ANYDATA_SXMLD can be used for dynamically allocated string. */
    LYD_ANYDATA_SXMLD = 0x05,       /**< In case of using serialized XML value as input parameter, this enumeration is
                                         supposed to be used for dynamically allocated strings (it is actually
                                         combination of #LYD_ANYDATA_SXML and #LYD_ANYDATA_STRING (and it can be also
                                         specified as ORed value of the mentioned values). */
    LYD_ANYDATA_XML = 0x08,         /**< value is struct lyxml_elem*, the structure is directly connected into the
                                         anydata node without duplication, caller is supposed to not manipulate with the
                                         data after a successful call (including calling lyxml_free() on the provided
                                         data) */
    LYD_ANYDATA_DATATREE = 0x10,    /**< value is struct lyd_node* (first sibling), the structure is directly connected
                                         into the anydata node without duplication, caller is supposed to not manipulate
                                         with the data after a successful call (including calling lyd_free() on the
                                         provided data) */
    LYD_ANYDATA_LYB = 0x20,         /**< value is a memory with serialized data tree in LYB format. The data are handled
                                         as a constant string. In case of using the value as input parameter,
                                         the #LYD_ANYDATA_LYBD can be used for dynamically allocated string. */
    LYD_ANYDATA_LYBD = 0x21,        /**< In case of using LYB value as input parameter, this enumeration is
                                         supposed to be used for dynamically allocated strings (it is actually
                                         combination of #LYD_ANYDATA_LYB and #LYD_ANYDATA_STRING (and it can be also
                                         specified as ORed value of the mentioned values). */
} LYD_ANYDATA_VALUETYPE;

/** @} */

/**
 * @brief YANG data representation
 */
struct lyd_value {
    const char *canonized;          /**< Canonical string representation of value (for comparison, printing,...), canonized according to the
                                         rules implemented in the type's canonization callback (if any). Note that not all the types
                                         have a canonical representation, so this value can be even NULL (identityref or instance-identifiers
                                         are built-in examples of such a case). The lyd_value::realtype's print callback provides possibility
                                         to get correct string representation of the value for the specific data format. */
    union {
        const char *string;         /**< original, non-canonized string value. Useful for example for unions where the type (and therefore
                                         the canonization rules) can change by changing value (e.g. leafref target) somewhere else. */
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

    struct lysc_type *realtype; /**< pointer to the real type of the data stored in the value structure. This type can differ from the type
                                          in the schema node of the data node since the type's store plugin can use other types/plugins for
                                          storing data. Speaking about built-in types, this is the case of leafref which stores data as its
                                          target type. In contrast, union type also use its subtype's callbacks, but inside an internal data
                                          lyd_value::subvalue structure, so here is the pointer to the union type.
                                          In general, this type is used to get free callback for this lyd_value structure, so it must reflect
                                          the type used to store data directly in the same lyd_value instance. */
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
    void *annotation;                /**< TODO pointer to the attribute/annotation's definition */
    const char *name;                /**< attribute name */
    struct lyd_value value;            /**< attribute's value representation */
};


#define LYD_NODE_INNER (LYS_CONTAINER|LYS_LIST) /**< Schema nodetype mask for lyd_node_inner */
#define LYD_NODE_TERM (LYS_LEAF|LYS_LEAFLIST)   /**< Schema nodetype mask for lyd_node_term */
#define LYD_NODE_ANY (LYS_ANYDATA)   /**< Schema nodetype mask for lyd_node_any */

/**
 * @brief Generic structure for a data node.
 */
struct lyd_node {
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list) */
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
};

/**
 * @brief Data node structure for the inner data tree nodes - containers and lists.
 */
struct lyd_node_inner {
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list) */
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
};

/**
 * @brief Data node structure for the terminal data tree nodes - leafs and leaf-lists.
 */
struct lyd_node_term {
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list) */
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
    uint32_t hash;                   /**< hash of this particular node (module name + schema name + key string values if list) */
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

    /* TODO - anydata representation */
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
                                     state data. To handle possibly missing (but by default required) ietf-yang-library
                                     data, use #LYD_OPT_DATA_NO_YANGLIB or #LYD_OPT_DATA_ADD_YANGLIB options. */
#define LYD_OPT_CONFIG     0x01 /**< A configuration datastore - complete datastore without state data.
                                     Validation modifications:
                                     - status data are not allowed */
#define LYD_OPT_GET        0x02 /**< Data content from a NETCONF reply message to the NETCONF \<get\> operation.
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier resolution is allowed to fail
                                     - list's keys/unique nodes are not required (so duplication is not checked)
                                     - must and when evaluation skipped */
#define LYD_OPT_GETCONFIG  0x04 /**< Data content from a NETCONF reply message to the NETCONF \<get-config\> operation
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier resolution is allowed to fail
                                     - list's keys/unique nodes are not required (so duplication is not checked)
                                     - must and when evaluation skipped
                                     - status data are not allowed */
#define LYD_OPT_EDIT       0x08 /**< Content of the NETCONF \<edit-config\>'s config element.
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier resolution is allowed to fail
                                     - must and when evaluation skipped
                                     - status data are not allowed */
#define LYD_OPT_RPC        0x10 /**< Data represents RPC or action input parameters. */
#define LYD_OPT_RPCREPLY   0x20 /**< Data represents RPC or action output parameters (maps to NETCONF <rpc-reply> data). */
#define LYD_OPT_NOTIF      0x40 /**< Data represents an event notification data. */
#define LYD_OPT_NOTIF_FILTER 0x80 /**< Data represents a filtered event notification data.
                                       Validation modification:
                                       - the only requirement is that the data tree matches the schema tree */
#define LYD_OPT_TYPEMASK   0x10000ff /**< Mask to filter data type options. Always only a single data type option (only
                                          single bit from the lower 8 bits) can be set. */

/* 0x100 reserved, used internally */
#define LYD_OPT_STRICT     0x0200 /**< Instead of silent ignoring data without schema definition, raise an error. */
#define LYD_OPT_DESTRUCT   0x0400 /**< Free the provided XML tree during parsing the data. With this option, the
                                       provided XML tree is affected and all successfully parsed data are freed.
                                       This option is applicable only to lyd_parse_xml() function. */
#define LYD_OPT_OBSOLETE   0x0800 /**< Raise an error when an obsolete statement (status set to obsolete) is used. */
#define LYD_OPT_NOSIBLINGS 0x1000 /**< Parse only a single XML tree from the input. This option applies only to
                                       XML input data. */
#define LYD_OPT_TRUSTED    0x2000 /**< Data comes from a trusted source and it is not needed to validate them. Data
                                       are connected with the schema, but the most validation checks (mandatory nodes,
                                       list instance uniqueness, etc.) are not performed. This option does not make
                                       sense for lyd_validate() so it is ignored by this function. */
#define LYD_OPT_WHENAUTODEL 0x4000 /**< Automatically delete subtrees with false when-stmt condition. The flag is
                                        applicable only in combination with #LYD_OPT_DATA and #LYD_OPT_CONFIG flags.
                                        If used, libyang will not generate a validation error. */
#define LYD_OPT_NOEXTDEPS  0x8000 /**< Allow external dependencies (external leafrefs, instance-identifiers, must,
                                       and when) to not be resolved/satisfied during validation. */
#define LYD_OPT_DATA_NO_YANGLIB  0x10000 /**< Ignore (possibly) missing ietf-yang-library data. Applicable only with #LYD_OPT_DATA. */
#define LYD_OPT_DATA_ADD_YANGLIB 0x20000 /**< Add missing ietf-yang-library data into the validated data tree. Applicable
                                              only with #LYD_OPT_DATA. If some ietf-yang-library data are present, they are
                                              preserved and option is ignored. */
#define LYD_OPT_VAL_DIFF 0x40000 /**< Flag only for validation, store all the data node changes performed by the validation
                                      in a diff structure. */
#define LYD_OPT_DATA_TEMPLATE 0x1000000 /**< Data represents YANG data template. */

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
 * search restriction is limited to compare canonical representation of the type. Some of the types have no canonical
 * representation and 2 different strings can represent the same value (e.g. different prefixes of the same namespace in instance-identifiers).
 * In this case there is more advanced lyd_value_compare() to check if the values matches.
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
 * @param[in] options Parser options, see @ref parseroptions. \p format LYD_LYB uses #LYD_OPT_TRUSTED implicitly.
 * @param[in] ... Variable arguments depend on \p options. If they include:
 *                - #LYD_OPT_DATA:
 *                - #LYD_OPT_CONFIG:
 *                - #LYD_OPT_GET:
 *                - #LYD_OPT_GETCONFIG:
 *                - #LYD_OPT_EDIT:
 *                  - no variable arguments expected.
 *                - #LYD_OPT_RPC:
 *                - #LYD_OPT_NOTIF:
 *                  - struct lyd_node *data_tree - additional data tree that will be used
 *                    when checking any "when" or "must" conditions in the parsed tree that require
 *                    some nodes outside their subtree. It must be a list of top-level elements!
 *                - #LYD_OPT_RPCREPLY:
 *                  - const struct ::lyd_node *rpc_act - pointer to the whole RPC or (top-level) action operation
 *                    data tree (the request) of the reply.
 *                  - const struct ::lyd_node *data_tree - additional data tree that will be used
 *                    when checking any "when" or "must" conditions in the parsed tree that require
 *                    some nodes outside their subtree. It must be a list of top-level elements!
 * @return Pointer to the built data tree or NULL in case of empty \p data. To free the returned structure,
 *         use lyd_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case of error,
 *         #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyd_node *lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, ...);

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
 * @param[in] options Parser options, see @ref parseroptions. \p format LYD_LYB uses #LYD_OPT_TRUSTED implicitly.
 * @param[in] ... Variable arguments depend on \p options. If they include:
 *                - #LYD_OPT_DATA:
 *                - #LYD_OPT_CONFIG:
 *                - #LYD_OPT_GET:
 *                - #LYD_OPT_GETCONFIG:
 *                - #LYD_OPT_EDIT:
 *                  - no variable arguments expected.
 *                - #LYD_OPT_RPC:
 *                - #LYD_OPT_NOTIF:
 *                  - struct lyd_node *data_tree - additional data tree that will be used
 *                    when checking any "when" or "must" conditions in the parsed tree that require
 *                    some nodes outside their subtree. It must be a list of top-level elements!
 *                - #LYD_OPT_RPCREPLY:
 *                  - const struct ::lyd_node *rpc_act - pointer to the whole RPC or action operation data
 *                    tree (the request) of the reply.
 *                  - const struct ::lyd_node *data_tree - additional data tree that will be used
 *                    when checking any "when" or "must" conditions in the parsed tree that require
 *                    some nodes outside their subtree. It must be a list of top-level elements!
 * @return Pointer to the built data tree or NULL in case of empty file. To free the returned structure,
 *         use lyd_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case of error,
 *         #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyd_node *lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...);

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
 * @param[in] options Parser options, see @ref parseroptions. \p format LYD_LYB uses #LYD_OPT_TRUSTED implicitly.
 * @param[in] ... Variable arguments depend on \p options. If they include:
 *                - #LYD_OPT_DATA:
 *                - #LYD_OPT_CONFIG:
 *                - #LYD_OPT_GET:
 *                - #LYD_OPT_GETCONFIG:
 *                - #LYD_OPT_EDIT:
 *                  - no variable arguments expected.
 *                - #LYD_OPT_RPC:
 *                - #LYD_OPT_NOTIF:
 *                  - struct lyd_node *data_tree - additional data tree that will be used
 *                    when checking any "when" or "must" conditions in the parsed tree that require
 *                    some nodes outside their subtree. It must be a list of top-level elements!
 *                - #LYD_OPT_RPCREPLY:
 *                  - const struct ::lyd_node *rpc_act - pointer to the whole RPC or action operation data
 *                    tree (the request) of the reply.
 *                  - const struct ::lyd_node *data_tree - additional data tree that will be used
 *                    when checking any "when" or "must" conditions in the parsed tree that require
 *                    some nodes outside their subtree. It must be a list of top-level elements!
 * @return Pointer to the built data tree or NULL in case of empty file. To free the returned structure,
 *         use lyd_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case of error,
 *         #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyd_node *lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, ...);

/**
 * @brief Free all the nodes in the data tree.
 *
 * @param[in] node Any of the nodes inside the tree.
 */
void lyd_free_all(struct lyd_node *node);

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
 * @brief Resolve instance-identifier defined by lyd_value_path structure.
 *
 * @param[in] path Path structure specifying the instance-identifier target.
 * @param[in] trees ([Sized array](@ref sizedarrays)) of data trees to be searched.
 *            To simply prepare this structure, use lyd_trees_new().
 * @return Target node of the instance-identifier present in the given data @p trees.
 */
const struct lyd_node_term *lyd_target(struct lyd_value_path *path, const struct lyd_node **trees);

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_DATA_H_ */
