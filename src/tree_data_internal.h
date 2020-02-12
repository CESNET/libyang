/**
 * @file tree_data_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_DATA_INTERNAL_H_
#define LY_TREE_DATA_INTERNAL_H_

#include "tree_data.h"
#include "plugins_types.h"

/**
 * @brief Get address of a node's child pointer if any.
 *
 * Decides the node's type and in case it has a children list, returns its address.
 * @param[in] node Node to check.
 * @return Address of the node's child member if any, NULL otherwise.
 */
struct lyd_node **lyd_node_children_p(struct lyd_node *node);

/**
 * @brief Create a term (leaf/leaf-list) node. Hash is not calculated.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value.
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
 * @brief Create an inner (container/list/RPC/action/notification) node. Hash is not calculated.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_inner(const struct lysc_node *schema, struct lyd_node **node);

/**
 * @brief Create a list with all its keys (cannot be used for key-less list). Hash is not calculated.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] keys_str List instance key values in the form of "[key1='val1'][key2='val2']...".
 *            The keys do not have to be ordered but all of them must be set.
 * @param[in] keys_len Length of @p keys_str.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_list(const struct lysc_node *schema, const char *keys_str, size_t keys_len, struct lyd_node **node);

/**
 * @brief Create an anyxml/anydata node. Hash is not calculated.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value Value of the any node, is directly assigned into the data node.
 * @param[in] value_type Value type of the value.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_any(const struct lysc_node *schema, const void *value, LYD_ANYDATA_VALUETYPE value_type, struct lyd_node **node);

/**
 * @brief Find the key after which to insert the new key.
 *
 * @param[in] first_sibling List first sibling.
 * @param[in] new_key Key that will be inserted.
 * @return Key to insert after.
 * @return NULL if the new key should be first.
 */
struct lyd_node *lyd_get_prev_key_anchor(const struct lyd_node *first_sibling, const struct lysc_node *new_key);

/**
 * @brief Insert a node into parent/siblings. In case a key is being inserted into a list, the correct position
 * is found and inserted into. Otherwise it is inserted at the last place.
 *
 * @param[in] parent Parent to insert into, NULL if @p first_sibling is set.
 * @param[in] first_sibling First top-level sibling, NULL if @p parent is set.
 * @param[in] node Individual node (without siblings) to insert.
 */
void lyd_insert_node(struct lyd_node *parent, struct lyd_node *first_sibling, struct lyd_node *node);

/**
 * @brief Create and insert an attribute (last) into a parent.
 *
 * @param[in] parent Parent of the attribute.
 * @param[in] mod Attribute module (with the annotation definition).
 * @param[in] name Attribute name.
 * @param[in] name_len Length of @p name.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the @p value string.
 * @param[in] prefix_data User data for @p get_prefix.
 * @param[in] format Input format of @p value.
 * @param[out] attr CReated attribute.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE in case data tree is needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_attr(struct lyd_node *parent, const struct lys_module *mod, const char *name, size_t name_len,
                       const char *value, size_t value_len, int *dynamic, ly_clb_resolve_prefix get_prefix, void *prefix_data,
                       LYD_FORMAT format, struct lyd_attr **attr);

/**
 * @brief Validate, canonize and store the given @p value into the node according to the node's type's rules.
 *
 * @param[in] node Data node for the @p value.
 * @param[in] value String value to be parsed, must not be NULL.
 * @param[in] value_len Length of the give @p value (mandatory).
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] second Flag for the second call after returning LY_EINCOMPLETE
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the @p value string.
 * @param[in] parser Parser's data for @p get_prefix
 * @param[in] format Input format of @p value.
 * @param[in] trees ([Sized array](@ref sizedarrays)) of data trees (e.g. when validating RPC/Notification) where the required
 *            data instance (leafref target, instance-identifier) can be placed. NULL in case the data tree are not yet complete,
 *            then LY_EINCOMPLETE can be returned.
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case the @p trees is not provided and it was needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_parse(struct lyd_node_term *node, const char *value, size_t value_len, int *dynamic, int second,
                       ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format, const struct lyd_node **trees);

/**
 * @brief Validate, canonize and store the given @p value into the attribute according to the metadata annotation type's rules.
 *
 * @param[in] attr Data attribute for the @p value.
 * @param[in] value String value to be parsed, must not be NULL.
 * @param[in] value_len Length of the give @p value (mandatory).
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] second Flag for the second call after returning LY_EINCOMPLETE
 * @param[in] get_prefix Parser-specific getter to resolve prefixes used in the @p value string.
 * @param[in] parser Parser's data for @p get_prefix
 * @param[in] format Input format of the data.
 * @param[in] trees ([Sized array](@ref sizedarrays)) of data trees (e.g. when validating RPC/Notification) where the required
 *            data instance (leafref target, instance-identifier) can be placed. NULL in case the data tree are not yet complete,
 *            then LY_EINCOMPLETE can be returned.
 * @return LY_SUCCESS on success
 * @return LY_EINCOMPLETE in case the @p trees is not provided and it was needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_value_parse_attr(struct lyd_attr *attr, const char *value, size_t value_len, int *dynamic, int second,
                            ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format, const struct lyd_node **trees);

/**
 * @brief Parse XML string as YANG data tree.
 *
 * @param[in] ctx libyang context
 * @param[in] data Pointer to the XML string representation of the YANG data to parse.
 * @param[in] options @ref dataparseroptions
 * @param[out] result Resulting list of the parsed data trees. Note that NULL can be a valid result.
 * @reutn LY_ERR value.
 */
LY_ERR lyd_parse_xml(struct ly_ctx *ctx, const char *data, int options, struct lyd_node **result);

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
 * @brief Free path (target) structure of the lyd_value.
 *
 * @param[in] ctx libyang context.
 * @param[in] path The structure ([sized array](@ref sizedarrays)) to free.
 */
void lyd_value_free_path(struct ly_ctx *ctx, struct lyd_value_path *path);

#endif /* LY_TREE_DATA_INTERNAL_H_ */
