/**
 * @file tree_data_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_DATA_INTERNAL_H_
#define LY_TREE_DATA_INTERNAL_H_

#include "lyb.h"
#include "plugins_types.h"
#include "set.h"
#include "tree_data.h"

#include <stddef.h>

struct ly_path_predicate;

/**
 * @brief Internal data parser flags.
 */
#define LYD_INTOPT_RPC      0x01    /**< RPC/action invocation is being parsed */
#define LYD_INTOPT_NOTIF    0x02    /**< notification is being parsed */
#define LYD_INTOPT_REPLY    0x04    /**< RPC/action reply is being parsed */

/**
 * @brief Hash schema sibling to be used for LYB data.
 *
 * @param[in] sibling Sibling to hash.
 * @param[in] collision_id Collision ID of the hash to generate.
 * @return Generated hash.
 */
LYB_HASH lyb_hash(struct lysc_node *sibling, uint8_t collision_id);

/**
 * @brief Check whether a sibling module is in a module array.
 *
 * @param[in] sibling Sibling to check.
 * @param[in] models Modules in a sized array.
 * @return non-zero if the module was found,
 * @return 0 if not found.
 */
int lyb_has_schema_model(const struct lysc_node *sibling, const struct lys_module **models);

/**
 * @brief Check whether a node to be deleted is the first top-level sibling.
 *
 * @param[in] first First sibling.
 * @param[in] to_del Node to be deleted.
 */
#define LYD_DEL_IS_ROOT(first, to_del) (((first) == (to_del)) && !(first)->parent && !(first)->prev->next)

/**
 * @brief Shorthand for getting data children without its keys.
 *
 * @param[in] node Node, whose children to traverse.
 * @return Node children, skipping any keys of a list.
 */
#define LYD_CHILD(node) lyd_node_children(node, LYD_CHILDREN_SKIP_KEYS)

/**
 * @brief Get address of a node's child pointer if any.
 *
 * @param[in] node Node to check.
 * @return Address of the node's child member,
 * @return NULL if there is no child pointer.
 */
struct lyd_node **lyd_node_children_p(struct lyd_node *node);

/**
 * @brief Create a term (leaf/leaf-list) node from a string value.
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the @p value string.
 * @param[in] prefix_data User data for @p get_prefix.
 * @param[in] format Input format of @p value.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE in case data tree is needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_term(const struct lysc_node *schema, const char *value, size_t value_len, int *dynamic,
                       ly_clb_resolve_prefix get_prefix, void *prefix_data, LYD_FORMAT format, struct lyd_node **node);

/**
 * @brief Create a term (leaf/leaf-list) node from a parsed value by duplicating it.
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] val Parsed value to use.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_term2(const struct lysc_node *schema, const struct lyd_value *val, struct lyd_node **node);

/**
 * @brief Create an inner (container/list/RPC/action/notification) node.
 *
 * Hash is calculated and new node flag is set except
 * for list with keys, when the hash is not calculated!
 * Also, non-presence container has its default flag set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_inner(const struct lysc_node *schema, struct lyd_node **node);

/**
 * @brief Create a list with all its keys (cannot be used for key-less list).
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] predicates Compiled key list predicates.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_list(const struct lysc_node *schema, const struct ly_path_predicate *predicates, struct lyd_node **node);

/**
 * @brief Create an anyxml/anydata node.
 *
 * Hash is calculated and flags are properly set based on @p is_valid.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value Value of the any node, is directly assigned into the data node.
 * @param[in] value_type Value type of the value.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_any(const struct lysc_node *schema, const void *value, LYD_ANYDATA_VALUETYPE value_type,
                      struct lyd_node **node);

/**
 * @brief Create an opaque node.
 *
 * @param[in] ctx libyang context.
 * @param[in] name Element name.
 * @param[in] name_len Length of @p name, must be set correctly.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value and @p ns.
 * @param[in] val_prefs Possible value prefixes, array is spent.
 * @param[in] prefix Element prefix.
 * @param[in] pref_len Length of @p prefix, must be set correctly.
 * @param[in] ns Node namespace, meaning depends on @p format.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_opaq(const struct ly_ctx *ctx, const char *name, size_t name_len, const char *value, size_t value_len,
                       int *dynamic, LYD_FORMAT format, struct ly_prefix *val_prefs, const char *prefix, size_t pref_len,
                       const char *ns, struct lyd_node **node);

/**
 * @brief Find the next node, before which to insert the new node.
 *
 * @param[in] first_sibling First sibling of the nodes to consider.
 * @param[in] new_node Node that will be inserted.
 * @return Node to insert after.
 * @return NULL if the new node should be first.
 */
struct lyd_node *lyd_insert_get_next_anchor(const struct lyd_node *first_sibling, const struct lyd_node *new_node);

/**
 * @brief Insert a node into parent/siblings. Order and hashes are fully handled.
 *
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first_sibling First sibling, NULL if no top-level sibling exist yet. Can be also NULL if @p parent is set.
 * @param[in] node Individual node (without siblings) to insert.
 */
void lyd_insert_node(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node *node);

/**
 * @brief Create and insert a metadata (last) into a parent.
 *
 * @param[in] parent Parent of the metadata, can be NULL.
 * @param[in,out] meta Metadata list to add at its end if @p parent is NULL, returned created attribute.
 * @param[in] mod Metadata module (with the annotation definition).
 * @param[in] name Attribute name.
 * @param[in] name_len Length of @p name, must be set correctly.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] resolve_prefix Parser-specific getter to resolve prefixes used in the @p value string.
 * @param[in] prefix_data User data for @p get_prefix.
 * @param[in] format Input format of @p value.
 * @param[in] ctx_snode Context node for value resolution in schema.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE in case data tree is needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_meta(struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod, const char *name,
                       size_t name_len, const char *value, size_t value_len, int *dynamic, ly_clb_resolve_prefix resolve_prefix,
                       void *prefix_data, LYD_FORMAT format, const struct lysc_node *ctx_snode);

/**
 * @brief Create and insert a generic attribute (last) into a parent.
 *
 * @param[in] parent Parent of the attribute, can be NULL.
 * @param[in,out] attr Attribute list to add at its end if @p parent is NULL, returned created attribute.
 * @param[in] ctx libyang context.
 * @param[in] name Attribute name.
 * @param[in] name_len Length of @p name, must be set correctly.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value and @p ns.
 * @param[in] val_prefs Possible value prefixes, array is spent.
 * @param[in] prefix Attribute prefix.
 * @param[in] prefix_len Attribute prefix length.
 * @param[in] ns Attribute namespace, meaning depends on @p format.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR ly_create_attr(struct lyd_node *parent, struct ly_attr **attr, const struct ly_ctx *ctx, const char *name,
                      size_t name_len, const char *value, size_t value_len, int *dynamic, LYD_FORMAT format,
                      struct ly_prefix *val_prefs, const char *prefix, size_t prefix_len, const char *ns);

/**
 * @brief Validate, canonize and store the given @p value into the node according to the node's type's rules.
 *
 * @param[in] node Data node for the @p value.
 * @param[in] value String value to be parsed, must not be NULL.
 * @param[in] value_len Length of the give @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] second Flag for the second call after returning LY_EINCOMPLETE
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the @p value string.
 * @param[in] parser Parser's data for @p get_prefix
 * @param[in] format Input format of @p value.
 * @param[in] tree Data tree (e.g. when validating RPC/Notification) where the required
 *            data instance (leafref target, instance-identifier) can be placed. NULL in case the data tree are not yet complete,
 *            then LY_EINCOMPLETE can be returned.
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case the @p trees is not provided and it was needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_parse(struct lyd_node_term *node, const char *value, size_t value_len, int *dynamic, int second,
                       ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format, const struct lyd_node *tree);

/* similar to lyd_value_parse except can be used just to store the value, hence does also not support a second call */
LY_ERR lyd_value_store(struct lyd_value *val, const struct lysc_node *schema, const char *value, size_t value_len,
                       int *dynamic, ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format);

/**
 * @brief Validate, canonize and store the given @p value into the metadata according to the annotation type's rules.
 *
 * @param[in] ctx libyang context.
 * @param[in] meta Metadata for the @p value.
 * @param[in] value String value to be parsed, must not be NULL.
 * @param[in] value_len Length of the give @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] second Flag for the second call after returning LY_EINCOMPLETE
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the @p value string.
 * @param[in] parser Parser's data for @p get_prefix
 * @param[in] format Input format of the data.
 * @param[in] ctx_snode Context node for value resolution in schema.
 * @param[in] tree Data tree (e.g. when validating RPC/Notification) where the required
 *            data instance (leafref target, instance-identifier) can be placed. NULL in case the data tree are not yet complete,
 *            then LY_EINCOMPLETE can be returned.
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case the @p trees is not provided and it was needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_parse_meta(const struct ly_ctx *ctx, struct lyd_meta *meta, const char *value, size_t value_len,
                            int *dynamic, int second, ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format,
                            const struct lysc_node *ctx_snode, const struct lyd_node *tree);

/**
 * @brief Parse XML string as YANG data tree.
 *
 * @param[in] ctx libyang context
 * @param[in] in Input structure.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Parsed data tree. Note that NULL can be a valid result.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_data(const struct ly_ctx *ctx, struct ly_in *in, int parse_options, int validate_options,
                          struct lyd_node **tree);

/**
 * @brief Parse XML string as YANG RPC/action invocation.
 *
 * Optional \<rpc\> envelope element is accepted if present. It is [checked](https://tools.ietf.org/html/rfc6241#section-4.1) and all
 * its XML attributes are parsed. As a content of the enveloper, an RPC data or \<action\> envelope element is expected. The \<action\> envelope element is
 * also [checked](https://tools.ietf.org/html/rfc7950#section-7.15.2) and then an action data is expected as a content of this envelope.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree Parsed full RPC/action tree.
 * @param[out] op Optional pointer to the actual operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree, struct lyd_node **op);

/**
 * @brief Parse XML string as YANG notification.
 *
 * Optional \<notification\> envelope element, if present, is [checked](https://tools.ietf.org/html/rfc5277#page-25)
 * and parsed. Specifically, its namespace and the child \<eventTime\> element and its value.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree Parsed full notification tree.
 * @param[out] op Optional pointer to the actual notification. Useful mainly for nested notifications.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree, struct lyd_node **ntf);

/**
 * @brief Parse XML string as YANG RPC/action reply.
 *
 * Optional \<rpc-reply\> envelope element, if present, is [checked](https://tools.ietf.org/html/rfc6241#section-4.2)
 * and all its XML attributes parsed.
 *
 * @param[in] request Data tree of the RPC/action request.
 * @param[in] in Input structure.
 * @param[out] tree Parsed full reply tree. It always includes duplicated operation and parents of the @p request.
 * @param[out] op Optional pointer to the reply operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_reply(const struct lyd_node *request, struct ly_in *in, struct lyd_node **tree, struct lyd_node **op);

/**
 * @brief Parse binary data as YANG data tree.
 *
 * @param[in] ctx libyang context
 * @param[in] in Input structure.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Parsed data tree. Note that NULL can be a valid result.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_data(const struct ly_ctx *ctx, struct ly_in *in, int parse_options, int validate_options,
                          struct lyd_node **tree);

/**
 * @brief Parse binary data as YANG RPC/action invocation.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree Parsed full RPC/action tree.
 * @param[out] op Optional pointer to the actual operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree, struct lyd_node **op);

/**
 * @brief Parse binary data as YANG notification.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree Parsed full notification tree.
 * @param[out] op Optional pointer to the actual notification. Useful mainly for nested notifications.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree, struct lyd_node **ntf);

/**
 * @brief Parse binary data as YANG RPC/action reply.
 *
 * @param[in] request Data tree of the RPC/action request.
 * @param[in] in Input structure.
 * @param[out] tree Parsed full reply tree. It always includes duplicated operation and parents of the @p request.
 * @param[out] op Optional pointer to the reply operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_reply(const struct lyd_node *request, struct ly_in *in, struct lyd_node **tree, struct lyd_node **op);

/**
 * @defgroup datahash Data nodes hash manipulation
 * @ingroup datatree
 */

/**
 * @brief Generate hash for the node.
 *
 * @param[in] node Data node to (re)generate hash value.
 * @return LY_ERR value.
 */
LY_ERR lyd_hash(struct lyd_node *node);

/**
 * @brief Insert hash of the node into the hash table of its parent.
 *
 * @param[in] node Data node which hash will be inserted into the lyd_node_inner::children_hash hash table of its parent.
 * @return LY_ERR value.
 */
LY_ERR lyd_insert_hash(struct lyd_node *node);

/**
 * @brief Maintain node's parent's children hash table when unlinking the node.
 *
 * When completely freeing data tree, it is expected to free the parent's children hash table first, at once.
 *
 * @param[in] node The data node being unlinked from its parent.
 */
void lyd_unlink_hash(struct lyd_node *node);

/** @} datahash */

/**
 * @brief Iterate over implemented modules for functions that accept specific modules or the whole context.
 *
 * @param[in] tree Data tree.
 * @param[in] module Selected module, NULL for all.
 * @param[in] ctx Context, NULL for selected modules.
 * @param[in,out] i Iterator, set to 0 on first call.
 * @param[out] first First sibling of the returned module.
 * @return Next module.
 * @return NULL if all modules were traversed.
 */
const struct lys_module *lyd_mod_next_module(struct lyd_node *tree, const struct lys_module *module,
                                             const struct ly_ctx *ctx, uint32_t *i, struct lyd_node **first);

/**
 * @brief Iterate over modules for functions that want to traverse all the top-level data.
 *
 * @param[in,out] next Pointer to the next module data, set to first top-level sibling on first call.
 * @param[out] first First sibling of the returned module.
 * @return Next module.
 * @return NULL if all modules were traversed.
 */
const struct lys_module *lyd_data_next_module(struct lyd_node **next, struct lyd_node **first);

/**
 * @brief Check that a list has all its keys.
 *
 * @param[in] node List to check.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT on a missing key.
 */
LY_ERR lyd_parse_check_keys(struct lyd_node *node);

/**
 * @brief Set data flags for a newly parsed node.
 *
 * @param[in] node Node to use.
 * @param[in] when_check Set of nodes with unresolved when.
 * @param[in,out] meta Node metadata, may be removed from.
 * @param[in] options Parse options.
 */
void lyd_parse_set_data_flags(struct lyd_node *node, struct ly_set *when_check, struct lyd_meta **meta, int options);

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
 * @brief Free value prefixes.
 *
 * @param[in] ctx libyang context.
 * @param[in] val_prefis Value prefixes to free, sized array (@ref sizedarrays).
 */
void ly_free_val_prefs(const struct ly_ctx *ctx, struct ly_prefix *val_prefs);

/**
 * @brief Append all list key predicates to path.
 *
 * @param[in] node Node with keys to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR
 */
LY_ERR lyd_path_list_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, int is_static);

#endif /* LY_TREE_DATA_INTERNAL_H_ */
