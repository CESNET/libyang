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
 * @brief Check validity of data parser options.
 *
 * @param ctx libyang context
 * @param options Data parser options to check
 * @param func name of the function where called
 * @return LY_SUCCESS when options are ok
 * @return LY_EINVAL when multiple data types bits are set, or incompatible options are used together.
 */
LY_ERR lyd_parse_check_options(struct ly_ctx *ctx, int options, const char *func);

/**
 * @brief Validate, canonize and store the given @p value into the node according to the node's type's rules.
 *
 * @param[in] node Data node for with the @p value.
 * @param[in] value String value to be parsed, must not be NULL.
 * @param[in] value_len Length of the give @p value (mandatory).
 * @param[in] dynamic Flag if @p value is a dynamically allocated memory and should be directly consumed/freed inside the function.
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
LY_ERR lyd_value_parse(struct lyd_node_term *node, const char *value, size_t value_len, int dynamic, int second,
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
