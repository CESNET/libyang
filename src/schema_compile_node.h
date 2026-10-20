/**
 * @file schema_compile_node.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for schema compilation of common nodes.
 *
 * Copyright (c) 2015 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_COMPILE_NODE_H_
#define LY_SCHEMA_COMPILE_NODE_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "tree_schema.h"

struct ly_set;
struct lysc_ctx;

/**
 * @brief Compile status information of the given statement.
 *
 * To simplify getting status of the node, the flags are set following inheritance rules, so all the nodes
 * has the status correctly set during the compilation.
 *
 * @param[in] ctx Compile context
 * @param[in] parsed_flags Parsed statement flags.
 * @param[in] inherited_flags Parsed inherited flags from a schema-only statement (augment, uses, ext instance, ...).
 * @param[in] parent_flags Compiled parent node flags.
 * @param[in] parent_name Name of the parent node, for logging.
 * @param[in] stmt_name Statement name, for logging.
 * @param[in,out] stmt_flags Statement flags with the correct status set.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_status(struct lysc_ctx *ctx, uint16_t parsed_flags, uint16_t inherited_flags, uint16_t parent_flags,
        const char *parent_name, const char *stmt_name, uint16_t *stmt_flags);

/**
 * @brief Compile information from the when statement by either standard compilation or by reusing
 * another compiled when structure.
 *
 * @param[in] ctx Compile context.
 * @param[in] when_p Parsed when structure.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in] parent Parent node, if any.
 * @param[in] ctx_node Context node for the when statement.
 * @param[in] node Compiled node to which to add the compiled when.
 * @param[in,out] when_c Optional, pointer to the previously compiled @p when_p to be reused. Set to NULL
 * for the first call.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_when(struct lysc_ctx *ctx, const struct lysp_when *when_p, uint16_t inherited_flags,
        const struct lysc_node *parent, const struct lysc_node *ctx_node, struct lysc_node *node, struct lysc_when **when_c);

/**
 * @brief Compile information from the must statement
 *
 * @param[in] ctx Compile context.
 * @param[in] must_p The parsed must statement structure.
 * @param[in,out] must Prepared (empty) compiled must structure to fill.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_must(struct lysc_ctx *ctx, const struct lysp_restr *must_p, struct lysc_must *must);

/**
 * @brief Connect the node into the siblings list and check its name uniqueness. Also,
 * keep specific order of augments targetting the same node.
 *
 * @param[in] ctx Compile context
 * @param[in] parent Parent node holding the children list, in case of node from a choice's case,
 * the choice itself is expected instead of a specific case node.
 * @param[in] node Schema node to connect into the list.
 * @return LY_ERR value - LY_SUCCESS or LY_EEXIST.
 * In case of LY_EEXIST, the node is actually kept in the tree, so do not free it directly.
 */
LY_ERR lys_compile_node_connect(struct lysc_ctx *ctx, struct lysc_node *parent, struct lysc_node *node);

/**
 * @brief Compile parsed action's input/output node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed inout node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node_() with filled generic node information
 * is enriched with the inout-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_node_action_inout(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node);

/**
 * @brief Compile choice children.
 *
 * @param[in] ctx Compile context
 * @param[in] child_p Parsed choice children nodes.
 * @param[in] node Compiled choice node to compile and add children to.
 * @param[in,out] child_set Optional set to add all the compiled nodes into (can be more in case of uses).
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_node_choice_child(struct lysc_ctx *ctx, struct lysp_node *child_p, struct lysc_node *node,
        struct ly_set *child_set);

/**
 * @brief Set LYS_MAND_TRUE flag for the non-presence container parents.
 *
 * A non-presence container is mandatory in case it has at least one mandatory children. This function propagate
 * the flag to such parents from a mandatory children.
 *
 * @param[in] parent A schema node to be examined if the mandatory child make it also mandatory.
 * @param[in] add Flag to distinguish adding the mandatory flag (new mandatory children appeared) or removing the flag
 * (mandatory children was removed).
 */
void lys_compile_mandatory_parents(struct lysc_node *parent, ly_bool add);

/**
 * @brief Validate grouping that was defined but not used in the schema itself.
 *
 * The grouping does not need to be compiled (and it is compiled here, but the result is forgotten immediately),
 * but to have the complete result of the schema validity, even such groupings are supposed to be checked.
 *
 * @param[in] ctx Compile context.
 * @param[in] pnode Parsed parent node of the grouping, NULL for top-level.
 * @param[in] grp Parsed grouping node to check.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_grouping(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysp_node_grp *grp);

/**
 * @brief Compile parsed schema node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed schema node.
 * @param[in] parent Compiled parent node where the current node is supposed to be connected. It is
 * NULL for top-level nodes, in such a case the module where the node will be connected is taken from
 * the compile context.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in,out] child_set Optional set to add all the compiled nodes into (can be more in case of uses).
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_node(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *parent, uint16_t inherited_flags,
        struct ly_set *child_set);

#endif /* LY_SCHEMA_COMPILE_NODE_H_ */
