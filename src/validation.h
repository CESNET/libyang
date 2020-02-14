/**
 * @file validation.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Validation routines.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_VALIDATION_H_
#define LY_VALIDATION_H_

#include "log.h"
#include "tree_data.h"

/**
 * @brief Finish validation of nodes and attributes. Specifically, type and when validation.
 *
 * @param[in] node_types Set with nodes with unresolved types, can be NULL
 * @param[in] attr_types Set with attributes with unresolved types, can be NULL.
 * @param[in] node_when Set with nodes with "when" conditions, can be NULL.
 * @param[in] format Format of the unresolved data.
 * @param[in] get_prefix_clb Format-specific getter to resolve prefixes.
 * @param[in] parser_data Parser's data for @p get_prefix_clb.
 * @param[in] trees Array of all data trees.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_unres(struct ly_set *node_types, struct ly_set *attr_types, struct ly_set *node_when, LYD_FORMAT format,
                          ly_clb_resolve_prefix get_prefix_clb, void *parser_data, const struct lyd_node **trees);

/**
 * @brief Perform all validation tasks, the data tree must be complete when calling this function.
 *
 * @param[in] trees Array of all data trees.
 * @param[in] modules Array of modules that should be validated, NULL for all modules.
 * @param[in] mod_count Modules count.
 * @param[in] ctx Context if all modules should be validated, NULL for only selected modules.
 * @param[in] val_opts Validation options.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_data(const struct lyd_node **trees, const struct lys_module **modules, int mod_count,
                         struct ly_ctx *ctx, int val_opts);

/**
 * @brief Check the existence and create any non-existing default siblings, recursively for the created nodes.
 *
 * @param[in] parent Parent of the potential default values.
 * @param[in,out] first First sibling.
 * @param[in] schema Schema parent of the default values, NULL for top-level siblings.
 * @param[in] mod Module of the default values, NULL for nested (non top-level) siblings.
 * @param[in] node_types Set to add nodes with unresolved types into.
 * @param[in] node_when Set to add nodes with "when" conditions into.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_defaults_r(struct lyd_node_inner *parent, struct lyd_node **first, const struct lysc_node *schema,
                               const struct lysc_module *mod, struct ly_set *node_types, struct ly_set *node_when);

/**
 * @brief Check the existence and create any non-existing default top-level nodes.
 *
 * @param[in,out] first First top-level sibling. There may be no explicit nodes.
 * @param[in] modules Array of modules that should be considered, NULL for all modules.
 * @param[in] mod_count Modules count.
 * @param[in] ctx Context if all modules should be considered, NULL for only selected modules.
 * @param[in] node_types Set to add nodes with unresolved types into, can be NULL if not needed.
 * @param[in] node_when Set to add nodes with "when" conditions into, can be NULL if not needed.
 * @param[in] val_opts Relevant validation options (#LYD_VALOPT_DATA_ONLY).
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_defaults_top(struct lyd_node **first, const struct lys_module **modules, int mod_count,
                                 struct ly_ctx *ctx, struct ly_set *node_types, struct ly_set *node_when, int val_opts);

#endif /* LY_VALIDATION_H_ */
