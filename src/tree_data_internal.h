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
 * @brief Validate the given value according to the type's rules.
 *
 * According to the given options, the value can be also canonized or stored into the node's value structure.
 *
 * @param[in] options [Type validation options ](@ref plugintypeopts).
 */
LY_ERR lyd_value_validate(struct lyd_node_term *node, const char *value, size_t value_len, int options);

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
 * @brief Maintain node's parent's children hash table when unlinking the node.
 *
 * When completely freeing data tree, it is expected to free the parent's children hash table first, at once.
 *
 * @param[in] node The data node being unlinked from its parent.
 */
void lyd_unlink_hash(struct lyd_node *node);

/** @} datahash */

#endif /* LY_TREE_DATA_INTERNAL_H_ */
