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
 * @brief Finish validation of nodes and attributes. Specifically, when (is processed first) and type validation.
 *
 * !! It is assumed autodeleted nodes cannot be in the unresolved node type set !!
 *
 * @param[in,out] tree Data tree, is updated if some nodes are autodeleted.
 * @param[in] node_when Set with nodes with "when" conditions, can be NULL.
 * @param[in] node_types Set with nodes with unresolved types, can be NULL
 * @param[in] meta_types Set with metdata with unresolved types, can be NULL.
 * @param[in] format Format of the unresolved data.
 * @param[in] get_prefix_clb Format-specific getter to resolve prefixes.
 * @param[in] parser_data Parser's data for @p get_prefix_clb.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_unres(struct lyd_node **tree, struct ly_set *node_when, struct ly_set *node_types, struct ly_set *meta_types,
                          LYD_FORMAT format, ly_clb_resolve_prefix get_prefix_clb, void *parser_data);

/**
 * @brief Validate new siblings. Specifically, check duplicated instances, autodelete default values and cases.
 *
 * !! It is assumed autodeleted nodes cannot yet be in the unresolved node type set !!
 *
 * @param[in,out] first First sibling.
 * @param[in] sparent Schema parent of the siblings, NULL for top-level siblings.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_new(struct lyd_node **first, const struct lysc_node *sparent, const struct lys_module *mod);

/**
 * @brief Perform all remaining validation tasks, the data tree must be final when calling this function.
 *
 * @param[in] first First sibling.
 * @param[in] sparent Schema parent of the siblings, NULL for top-level siblings.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @param[in] op Operation to validate (@ref datavalidateop) or 0 for data tree
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_final_r(struct lyd_node *first, const struct lysc_node *sparent, const struct lys_module *mod,
                            int val_opts, LYD_VALIDATE_OP op);

/**
 * @brief Check the existence and create any non-existing default siblings, recursively for the created nodes.
 *
 * @param[in] parent Parent of the potential default values, NULL for top-level siblings.
 * @param[in,out] first First sibling.
 * @param[in] sparent Schema parent of the siblings, NULL if schema of @p parent can be used.
 * @param[in] mod Module of the default values, NULL for nested siblings.
 * @param[in] node_types Set to add nodes with unresolved types into.
 * @param[in] node_when Set to add nodes with "when" conditions into.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_defaults_r(struct lyd_node_inner *parent, struct lyd_node **first, const struct lysc_node *sparent,
                               const struct lys_module *mod, struct ly_set *node_types, struct ly_set *node_when, int val_opts);

#endif /* LY_VALIDATION_H_ */
