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
 * @brief Perform all vaidation tasks that depend on other nodes, the data tree must
 * be complete when calling this function.
 *
 * @param[in] trees Array of all data trees.
 * @param[in] modules Array of modules that should be validated, NULL for all modules.
 * @param[in] mod_count Modules count.
 * @param[in] ctx Context if all modules should be validated, NULL for only selected modules.
 * @param[in] options Validation options.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_modules(const struct lyd_node **trees, const struct lys_module **modules, int mod_count,
                            struct ly_ctx *ctx, int options);

#endif /* LY_VALIDATION_H_ */
