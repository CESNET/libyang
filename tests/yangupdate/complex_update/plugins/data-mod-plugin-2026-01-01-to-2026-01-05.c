/**
 * @file data-mod-plugin-2026-01-01-to-2026-01-05.c
 * @brief yangupdate plugin for ydct 2026-01-01 -> 2026-01-05
 *
 * Handles the following incompatible modifications:
 *   MODIF-06: New mandatory leaf-04 (bits) at root
 *   MODIF-07: New mandatory leaf-03 (enumeration) in container-01
 *   MODIF-08: Narrowed container-01/leaf-01 range 1..1000 -> 1..100
 *   MODIF-09: Renamed root leaf-03 -> leaf-05
 *   MODIF-10: Removed enum e-01 from container-01/leaf-02
 */
#define _GNU_SOURCE

#include "yang_update.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * MODIF-09: Renamed leaf-03 -> leaf-05.
 * Read the old leaf-03 value from data_old and create leaf-05 in the new tree.
 */
static LY_ERR
root_leaf05_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *old_leaf03;

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    /* find the old leaf-03 in the old data tree */
    if (!lyd_find_path(data_old, "/ydct:leaf-03", 0, &old_leaf03)) {
        return lyd_new_term(parent_new, schema_new->module, schema_new->name,
                lyd_get_value(old_leaf03), 0, node_new);
    }

    /* old leaf-03 not present, use default */
    return lyd_new_term(parent_new, schema_new->module, schema_new->name, "false", 0, node_new);
}

/**
 * MODIF-06: New mandatory leaf-04 (bits b-01 b-02) at root.
 * No predecessor value exists; set a required value.
 */
static LY_ERR
root_leaf04_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_term(parent_new, schema_new->module, schema_new->name, "b-01 b-02", 0, node_new);
}

/**
 * MODIF-08: Narrowed container-01/leaf-01 range from 1..1000 to 1..100.
 * Clamp values > 100 to 100.
 */
static LY_ERR
cont01_leaf01_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    const char *old_val;
    long val;

    (void)data_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!node_old) {
        return LY_SUCCESS;
    }

    old_val = lyd_get_value(node_old);
    val = strtol(old_val, NULL, 10);
    if (val > 100) {
        return lyd_new_term(parent_new, NULL, schema_new->name, "100", 0, node_new);
    }

    return lyd_dup_single_to_ctx(node_old, schema_new->module->ctx, parent_new, 0, node_new);
}

/**
 * MODIF-10: Removed enum e-01 from container-01/leaf-02.
 * If old value is e-01, map to e-02; otherwise copy as-is.
 */
static LY_ERR
cont01_leaf02_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    const char *old_val;

    (void)data_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!node_old) {
        return LY_SUCCESS;
    }

    old_val = lyd_get_value(node_old);
    if (!strcmp(old_val, "e-01")) {
        return lyd_new_term(parent_new, NULL, schema_new->name, "e-02", 0, node_new);
    }

    return lyd_dup_single_to_ctx(node_old, schema_new->module->ctx, parent_new, 0, node_new);
}

/**
 * MODIF-07: New mandatory leaf-03 (enumeration e-01/e-02) in container-01.
 * No predecessor value exists; set a required value.
 */
static LY_ERR
cont01_leaf03_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_term(parent_new, NULL, schema_new->name, "e-01", 0, node_new);
}

static struct lyu_plg_rule rules_ydct_01_05[] = {
    { .node_path = "/ydct:leaf-04",             .node_cb = root_leaf04_cb },
    { .node_path = "/ydct:leaf-05",             .node_cb = root_leaf05_cb },
    { .node_path = "/ydct:container-01/leaf-01", .node_cb = cont01_leaf01_cb },
    { .node_path = "/ydct:container-01/leaf-02", .node_cb = cont01_leaf02_cb },
    { .node_path = "/ydct:container-01/leaf-03", .node_cb = cont01_leaf03_cb },
    { 0 }
};

static struct lyu_plg_module imports_new_ydct_01_05[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { 0 }
};

struct lyu_plg plg_ydct_01_05 = {
    .module_name = "ydct",
    .revision_old = "2026-01-01",
    .features_old = NULL,
    .imports_old = NULL,
    .revision_new = "2026-01-05",
    .features_new = NULL,
    .imports_new = imports_new_ydct_01_05,
    .rules = rules_ydct_01_05
};
