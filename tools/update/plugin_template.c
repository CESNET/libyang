/**
 * @file plugin_template.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yang update plugin template
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

/* Mandatory header with all the basic definitions. This source file will be compiled as part of 'libyang_update'
 * library and the directory with this header will be added to the include directories. */
#include "yang_update.h"

/* Example callback for updating a single node. */
static LY_ERR
NODE_CB(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        struct lyd_node **node_new)
{
    /* This is the default update operation of simply copying the old node into the new data tree. */
    return lyd_dup_single_to_ctx(node_old, schema_new->module->ctx, parent_new, 0, node_new);
}

/* Array of rules that must always be terminated by an empty rule. Each rule consists of a data node path and a callback
 * that should be called to update this node. The path may include simple predicates for identifying specific instances
 * of nodes and/or their parents.
 *
 * For example, a rule with the path '/MOD_NAME:CONTAINER/LIST/LEAF' would be applied for all the 'LEAF' nodes in all
 * the 'LIST' instances. But a rule for '/MOD_NAME:CONTAINER/LIST[KEY='VALUE']/LEAF' would be applied only for a single
 * instance of 'LEAF' with the specific 'LIST' node parent instance. */
static struct lyu_plg_rule RULES_MOD_NAME[] = {
    { .node_path = "NODE_DATA_PATH", .node_cb = NODE_CB },
    { 0 }
};

/* This line and all similar ones are searched for in all the plugin source files to generate a complete list of
 * available plugins. There can be several plugins in every file. Because a single array of these plugin structures is
 * generated, each plugin must have a globally unique name.
 *
 * The members identify the plugin by its affected YANG module name and the 2 revisions of this module whose data this
 * plugin structure transforms and updates. */
struct lyu_plg PLG_MOD_NAME = {
    .module_name = "MOD_NAME",
    .revision_old = "OLD_REVISION",
    .revision_new = "NEW_REVISION",
    .rules = RULES_MOD_NAME
};
