/**
 * @file validation.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data tree validation for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_VALIDATION_H_
#define LY_VALIDATION_H_

#include "libyang.h"
#include "resolve.h"
#include "tree_data.h"

/**
 * @brief Check, that the data node of the given schema node can even appear in a data tree.
 *
 * Checks included:
 * - data node is not disabled via if-features
 * - data node is not disabled via an unsatisfied when condition
 * - data node is not status in case of edit-config content (options includes LYD_OPT_EDIT)
 * - data node is in correct place (options includes LYD_OPT_RPC or LYD_OPT_RPCREPLY), since elements order matters
 *   in RPCs and RPC replies.
 *
 * @param[in] node Data tree node to be checked.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] line Optional line of the input to be printed in case of an error.
 * @param[out] unres Structure to store unresolved items into. Can be NULL.
 * @return EXIT_SUCCESS or EXIT_FAILURE with ly_errno set.
 */
int lyv_data_context(const struct lyd_node *node, int options, unsigned int line, struct unres_data *unres);

/**
 * @brief Validate if the node's content is valid in the context it is placed.
 *
 * Expects that the node is already interconnected to the target tree and all its children
 * are already resolved. All currently connected siblings are included to the tests.
 *
 * @param[in] node Data tree node to be checked.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] line Optional line of the input to be printed in case of an error.
 * @param[out] unres Structure to store unresolved items into. Can be NULL.
 * @return EXIT_SUCCESS or EXIT_FAILURE with set ly_errno. If EXIT_FAILURE is returned
 * but ly_errno is not set, the issue was internally resolved and caller is supposed to
 * unlink and free the node and continue;
 */
int lyv_data_content(struct lyd_node *node, int options, unsigned int line, struct unres_data *unres);

/**
 * @brief Validate the node's value. Applies only to referrence values where the validity can change by
 * modifying a value/tree outside the node itself (leafrefs).
 *
 * @param[in] node Data tree node to be checked.
 * @param[in] options Parser options, see @ref parseroptions.
 * @return EXIT_SUCCESS or EXIT_FAILURE with ly_errno set.
 */
int lyv_data_value(struct lyd_node *node, int options);

#endif /* LY_VALIDATION_H_ */
