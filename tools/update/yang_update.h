/**
 * @file yang_update.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yang update header
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_YANG_UPDATE_H_
#define LY_YANG_UPDATE_H_

#include <stdint.h>

#include <libyang/libyang.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback called for every schema node found in both old and new revision of a schema or only in the new
 * revision of the schema in case the node has been added. If a schema node has been removed, the data node is also
 * removed and no callback is called.
 *
 * Callback set for a rule with path to a (leaf-)list without a predicate will be called with @p node_old NULL at least
 * once until it returns empty @p node_new allowing to create any number of new instances.
 *
 * @param[in] data_old Whole data tree in the old revision.
 * @param[in] node_old Current data node from @p data_old being updated, if exists. In case the node can have multiple
 * instances, the callback is called for each one individually.
 * @param[in] schema_old Current schema node in the old revision being updated, if exists. Not set for added schema nodes.
 * @param[in] parent_new Current data node parent in new updated revision to use, if exists. Not set for top-level nodes.
 * @param[in] schema_new Current schema node in the new revision.
 * @param[in] node_cmp Generated schema comparison node data of the current schema nodes, if any. Not set for schema
 * nodes without any changes.
 * @param[in,out] user_data Pointer to arbitrary user data, is kept for all the callback calls for a single @p schema_new.
 * @param[out] node_new Created updated data node, if any. Not set if the node is supposed to be deleted or no more
 * nodes are supposed to be created.
 * @return LY_SUCCESS on success;
 * @return LY_ERR value on error.
 */
typedef LY_ERR (*lyu_plg_node_cb)(const struct lyd_node *data_old, const struct lyd_node *node_old,
        const struct lysc_node *schema_old, struct lyd_node *parent_new, const struct lysc_node *schema_new,
        const struct lyd_node *node_cmp, const void **user_data, struct lyd_node **node_new);

/**
 * @brief Rule for updating a single node.
 *
 * Data nodes that exist in the old data tree, were not removed in the new schema revision, and have no rule are simply
 * copied which is the default implicit update rule.
 */
struct lyu_plg_rule {
    const char *node_path;      /**< data node path optionally with predicates identifying specific instances of (leaf-)lists */
    lyu_plg_node_cb node_cb;    /**< callback for creating the updated node */
};

/**
 * @brief Plugin for updating the data from a specific old revision to a specific new revision of a YANG module.
 */
struct lyu_plg {
    const char *module_name;    /**< YANG module name */
    const char *revision_old;   /**< old YANG revision of the input data */
    const char *revision_new;   /**< new YANG revision of the output data */
    struct lyu_plg_rule *rules; /**< array of rules to apply on the nodes terminated by an empty rule */
};

/**
 * @brief Find the new YANG module revision to update data for.
 *
 * There is a chain of plugins created to find the latest suitable YANG module or the matching one.
 *
 * @param[in] mod_old Old YANG module revision.
 * @param[in] ctx_new Context to use for loading the new YANG module.
 * @param[in] revision Optional new revision. If not set, find the latest one.
 * @param[out] mod_new New YANG module revision.
 * @return LY_SUCCESS on success;
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lyd_update_find_new(const struct lys_module *mod_old, struct ly_ctx *ctx_new,
        const char *revision, struct lys_module **mod_new);

/**
 * @brief Update data in a specific revision of a YANG module to a newer revision.
 *
 * All the required YANG modules are loaded into temporary contexts that will use the same search paths and import
 * callbacks as the context of @p mod_old.
 *
 * To get @p mod_new, ::lyd_update_find_new function can be used.
 *
 * @param[in] mod_old Old YANG module revision.
 * @param[in] data_old Old data of @p mod_old to update.
 * @param[in] mod_new New YANG module revision.
 * @param[out] data_new New updated data of @p mod_new.
 * @return LY_SUCCESS on success;
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lyd_update(const struct lys_module *mod_old, const struct lyd_node *data_old,
        const struct lys_module *mod_new, struct lyd_node **data_new);

/**
 * @brief Print all the changes between the old and updated data in a summarized form into a file.
 *
 * @param[in] data_old Old data tree.
 * @param[in] data_new New updated data tree.
 * @param[in] out File to print to.
 * @return LY_SUCCESS on success;
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lyd_update_print(const struct lyd_node *data_old, const struct lyd_node *data_new, FILE *out);

#ifdef __cplusplus
}
#endif

#endif /* LY_YANG_UPDATE_H_ */
