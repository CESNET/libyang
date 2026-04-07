/**
 * @file data-mod-plugin-2026-01-10-to-2026-01-15.c
 * @brief yangupdate plugin for ydct 2026-01-10 -> 2026-01-15
 *
 * Handles the following incompatible modifications:
 *   MODIF-01: leaf-list-01 → list-02 with cross-module augment
 *   MODIF-02: mandatory choice-02 in container-04 (leaf-03 = "")
 *   MODIF-03: rename container-03/list-01/leaf-04 → leaf-07
 *   MODIF-04: remove container-03/list-01/container-01 (auto-drop)
 *   MODIF-05: new mandatory leaf-08 in container-03/list-01 (computed)
 *   MODIF-06: merge root leaf-07 + leaf-08 → leaf-13
 *   MODIF-07: remove feature-01, leaf-10 unconditional (compat)
 *   MODIF-08: new feature-02 with leaf-14 (compat)
 */
#define _GNU_SOURCE

#include "yang_update.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * Root-level callbacks
 * ======================================================================== */

/**
 * MODIF-06: Merge root leaf-07 (boolean) and leaf-08 (boolean) into leaf-13 (string).
 * Value: "<leaf-07>-<leaf-08>" e.g. "true-true".
 * This callback is for the NEW node leaf-13 (node_old is NULL since it didn't exist).
 */
static LY_ERR
root_leaf13_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *old_leaf07, *old_leaf08;
    const char *val07 = "false", *val08 = "false";
    char buf[64];

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!lyd_find_path(data_old, "/ydct:leaf-07", 0, &old_leaf07)) {
        val07 = lyd_get_value(old_leaf07);
    }
    if (!lyd_find_path(data_old, "/ydct:leaf-08", 0, &old_leaf08)) {
        val08 = lyd_get_value(old_leaf08);
    }

    snprintf(buf, sizeof(buf), "%s-%s", val07, val08);

    return lyd_new_term(parent_new, schema_new->module, schema_new->name,
            buf, 0, node_new);
}

/**
 * MODIF-01: Convert leaf-list-01 → list-02 entries.
 * Each leaf-list value becomes a list entry with leaf-01 = value.
 * Cross-module augment: ygrp-aug01-leaf-02 (mandatory) = false.
 * Called repeatedly until all entries are created.
 */
static LY_ERR
root_list02_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    long call_idx = (long)*user_data;
    const struct lyd_node *iter;
    struct lyd_node *entry;
    const char *val;
    long count = 0;
    LY_ERR rc;

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;

    /* Iterate top-level siblings to find the call_idx-th leaf-list-01 instance.
     * Cannot use lyd_find_path because it requires LY_PATH_TARGET_SINGLE
     * which needs predicates for leaf-list nodes. */
    val = NULL;
    for (iter = data_old; iter; iter = iter->next) {
        if (iter->schema && !strcmp(iter->schema->name, "leaf-list-01")) {
            if (count == call_idx) {
                val = lyd_get_value(iter);
                break;
            }
            count++;
        }
    }
    if (!val) {
        *node_new = NULL;
        return LY_SUCCESS;
    }

    /* Create list entry with key leaf-01 */
    rc = lyd_new_list(parent_new, schema_new->module, schema_new->name,
            0, &entry, val);
    if (rc) {
        return rc;
    }

    /* Cross-module augment: mandatory ygrp-aug01-leaf-02 = false
     * Find the augmented schema node to get its owning module. */
    {
        const struct lysc_node *list_schema, *child_schema;
        const struct lys_module *grp_mod = NULL;

        list_schema = schema_new;
        /* Iterate schema children to find ygrp-aug01-leaf-02 */
        child_schema = lysc_node_child(list_schema);
        while (child_schema) {
            if (!strcmp(child_schema->name, "ygrp-aug01-leaf-02")) {
                grp_mod = child_schema->module;
                break;
            }
            child_schema = child_schema->next;
        }
        if (grp_mod) {
            rc = lyd_new_term(entry, grp_mod, "ygrp-aug01-leaf-02", "false", 0, NULL);
            if (rc) {
                return rc;
            }
        }
    }

    *node_new = entry;
    *user_data = (const void *)(call_idx + 1);
    return LY_SUCCESS;
}

/* ========================================================================
 * container-03/list-01 callbacks
 * ======================================================================== */

/**
 * MODIF-03: Rename leaf-04 → leaf-07 in list-01.
 * Read the old leaf-04 value and create leaf-07 with same value.
 */
static LY_ERR
list01_leaf07_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *child;
    const char *old_val = NULL;

    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    /* Find leaf-04 in the old list entry (parent_new's corresponding old entry) */
    /* node_old is NULL since leaf-07 didn't exist in old schema.
     * We need to find leaf-04 from the old list entry.
     * The parent_new is the new list entry; we need to find the old list entry. */

    /* Search in data_old for the corresponding list entry.
     * parent_new has keys that match the old entry. */
    {
        struct lyd_node *old_list01;
        const char *key_val = NULL;
        const char *key5_val = NULL;

        /* Get key values from parent_new */
        child = lyd_child(parent_new);
        while (child) {
            if (!strcmp(child->schema->name, "leaf-01")) {
                key_val = lyd_get_value(child);
            } else if (!strcmp(child->schema->name, "leaf-05")) {
                key5_val = lyd_get_value(child);
            }
            child = child->next;
        }

        if (!key_val || !key5_val) {
            *node_new = NULL;
            return LY_SUCCESS;
        }

        /* Find the old list entry */
        {
            char xpath[256];
            snprintf(xpath, sizeof(xpath),
                    "/ydct:container-03/list-01[leaf-01='%s'][leaf-05='%s']/leaf-04",
                    key_val, key5_val);
            if (!lyd_find_path(data_old, xpath, 0, &old_list01)) {
                old_val = lyd_get_value(old_list01);
            }
        }
    }

    if (!old_val) {
        /* leaf-04 didn't exist (when condition not met) */
        *node_new = NULL;
        return LY_SUCCESS;
    }

    return lyd_new_term(parent_new, NULL, schema_new->name,
            old_val, 0, node_new);
}

/**
 * MODIF-05: New mandatory leaf-08 in list-01.
 * Value = "<root-leaf-10>-<leaf-01-of-this-entry>".
 */
static LY_ERR
list01_leaf08_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *leaf10_node, *child;
    const char *leaf10_val = NULL;
    const char *leaf01_val = NULL;
    char buf[256];

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    /* Get root leaf-10 from old data */
    if (!lyd_find_path(data_old, "/ydct:leaf-10", 0, &leaf10_node)) {
        leaf10_val = lyd_get_value(leaf10_node);
    }
    if (!leaf10_val) {
        leaf10_val = "unknown";
    }

    /* Get leaf-01 (key) from current list entry (parent_new) */
    child = lyd_child(parent_new);
    while (child) {
        if (!strcmp(child->schema->name, "leaf-01")) {
            leaf01_val = lyd_get_value(child);
            break;
        }
        child = child->next;
    }
    if (!leaf01_val) {
        leaf01_val = "unknown";
    }

    snprintf(buf, sizeof(buf), "%s-%s", leaf10_val, leaf01_val);

    return lyd_new_term(parent_new, NULL, schema_new->name,
            buf, 0, node_new);
}

/* ========================================================================
 * container-04 callbacks
 * ======================================================================== */

/**
 * MODIF-02: Mandatory choice-02 in container-04.
 * leaf-03 (string, mandatory) inside case-01 → value = "".
 */
static LY_ERR
cont04_leaf03_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_term(parent_new, NULL, schema_new->name,
            "", 0, node_new);
}

/* ========================================================================
 * Rules and plugin struct
 * ======================================================================== */

static struct lyu_plg_rule rules_ydct_10_15[] = {
    /* MODIF-06: merge leaf-07+leaf-08 → leaf-13 */
    { .node_path = "/ydct:leaf-13",                                           .node_cb = root_leaf13_cb },
    /* MODIF-01: leaf-list-01 → list-02 */
    { .node_path = "/ydct:list-02",                                           .node_cb = root_list02_cb },
    /* MODIF-03: rename leaf-04 → leaf-07 in list-01 */
    { .node_path = "/ydct:container-03/list-01/leaf-07",                      .node_cb = list01_leaf07_cb },
    /* MODIF-05: new mandatory leaf-08 in list-01 */
    { .node_path = "/ydct:container-03/list-01/leaf-08",                      .node_cb = list01_leaf08_cb },
    /* MODIF-02: mandatory leaf-03 in container-04/choice-02 */
    { .node_path = "/ydct:container-04/leaf-03",                              .node_cb = cont04_leaf03_cb },
    { 0 }
};

/* MODIF-07: feature-01 removed — features_new does NOT include feature-01 */
/* MODIF-08: feature-02 added */
static const char *features_new_ydct_10_15[] = {
    "feature-02",
    NULL
};

static const char *features_old_ydct_10_15[] = {
    "feature-01",
    NULL
};

static const char *ydct_types_features[] = {
    "feature-01",
    NULL
};

static struct lyu_plg_module imports_old_ydct_10_15[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { .name = "ydct-types", .revision = "2026-01-10", .features = (const char **)ydct_types_features, .implemented = 1 },
    { .name = "ydct-group", .revision = "2026-01-10", .features = NULL, .implemented = 0 },
    { 0 }
};

static struct lyu_plg_module imports_new_ydct_10_15[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { .name = "ydct-types", .revision = "2026-01-10", .features = (const char **)ydct_types_features, .implemented = 1 },
    { .name = "ydct-group", .revision = "2026-01-15", .features = NULL, .implemented = 1 },
    { 0 }
};

struct lyu_plg plg_ydct_10_15 = {
    .module_name = "ydct",
    .revision_old = "2026-01-10",
    .features_old = (const char **)features_old_ydct_10_15,
    .imports_old = imports_old_ydct_10_15,
    .revision_new = "2026-01-15",
    .features_new = (const char **)features_new_ydct_10_15,
    .imports_new = imports_new_ydct_10_15,
    .rules = rules_ydct_10_15
};
