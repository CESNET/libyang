/**
 * @file mod.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yang update example 'mod' plugin
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE

#include "yang_update.h"

#include <assert.h>
#include <string.h>

static LY_ERR
cont_list1_cb1(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    assert(data_old && node_old && schema_old && parent_new && schema_new && node_cmp && user_data && node_new);

    /* remove the instance */

    return LY_SUCCESS;
}

static LY_ERR
cont_l1_cb1(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    char *new_val;

    assert(data_old && node_old && schema_old && schema_new && node_cmp && user_data);

    if (!strcmp(lyd_get_value(lyd_child(parent_new)), "k1")) {
        /* list1 key1 'k1' */
        asprintf(&new_val, "%s10", lyd_get_value(node_old));
        lyd_new_term(parent_new, NULL, schema_new->name, new_val, 0, node_new);
        free(new_val);
    } else {
        /* list1 key1 'k5' */
        asprintf(&new_val, "%s50", lyd_get_value(node_old));
        lyd_new_term(parent_new, NULL, schema_new->name, new_val, 0, node_new);
        free(new_val);
    }

    return LY_SUCCESS;
}

static LY_ERR
l2_cb1(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    char *new_val;

    assert(data_old && node_old && schema_old && !parent_new && node_cmp && user_data);

    /* multiply old value by 1000 */
    asprintf(&new_val, "%s000", lyd_get_value(node_old));
    lyd_new_term(NULL, schema_new->module, schema_new->name, new_val, 0, node_new);
    free(new_val);

    return LY_SUCCESS;
}

static struct lyu_plg_rule rules_mod_1_3[] = {
    { .node_path = "/mod:cont/list1[key1='k8']", .node_cb = cont_list1_cb1 },
    { .node_path = "/mod:cont/list1[key1='k1']/l1", .node_cb = cont_l1_cb1 },
    { .node_path = "/mod:cont/list1[key1='k5']/l1", .node_cb = cont_l1_cb1 },
    { .node_path = "/mod:l2", .node_cb = l2_cb1 },
    { 0 }
};

struct lyu_plg plg_mod_1_3 = {
    .module_name = "mod",
    .revision_old = "2025-01-01",
    .revision_new = "2025-03-01",
    .rules = rules_mod_1_3
};

static LY_ERR
cont_list1_cb3(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    const char *key_val;

    assert(data_old && schema_old && schema_new && node_cmp && user_data);

    if (node_old) {
        key_val = lyd_get_value(lyd_child(node_old));

        /* keep only 'k1' - 'k5' */
        if ((key_val[1] > '0') && (key_val[1] < '6')) {
            lyd_dup_single_to_ctx(node_old, schema_new->module->ctx, parent_new, 0, node_new);
        }
    } else {
        /* create 2 new instances */
        key_val = *user_data;
        if (!key_val) {
            lyd_new_list(parent_new, NULL, schema_new->name, 0, node_new, "k100");
            *user_data = lyd_get_value(lyd_child(*node_new));
        } else if (!strcmp(key_val, "k100")) {
            lyd_new_list(parent_new, NULL, schema_new->name, 0, node_new, "k101");
            *user_data = lyd_get_value(lyd_child(*node_new));
        } else if (!strcmp(key_val, "k101")) {
            *user_data = NULL;
        }
    }

    return LY_SUCCESS;
}

static struct lyu_plg_rule rules_mod_3_6[] = {
    { .node_path = "/mod:cont/list1", .node_cb = cont_list1_cb3 },
    { 0 }
};

struct lyu_plg plg_mod_3_6 = {
    .module_name = "mod",
    .revision_old = "2025-03-01",
    .revision_new = "2025-06-01",
    .rules = rules_mod_3_6
};
