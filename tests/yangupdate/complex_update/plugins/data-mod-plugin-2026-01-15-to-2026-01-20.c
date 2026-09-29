/**
 * @file data-mod-plugin-2026-01-15-to-2026-01-20.c
 * @brief yangupdate plugin for ydct 2026-01-15 -> 2026-01-20
 *
 * Contains TWO plugin structs for the same revision pair:
 *   plg_ydct_15_20_feat   — variant A: features_old includes feature-02
 *   plg_ydct_15_20_nofeat — variant B: features_old is NULL (no feature-02)
 *
 * Handles:
 *   MODIF-01: feature-02 removed, leaf-14 removed, new leaf-16 unconditional
 *   MODIF-02: mandatory leaf-03 (enumeration) added to list-02
 *   MODIF-03: leaf-06 and leaf-12 if-feature changed (compatible, no rule)
 */
#define _GNU_SOURCE

#include "yang_update.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * Shared callbacks (used by both variants)
 * ======================================================================== */

/**
 * MODIF-02: Add mandatory leaf-03 (enumeration e-01) to each list-02 entry.
 * This callback handles the new leaf inside the list.
 */
static LY_ERR
list02_leaf03_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_term(parent_new, NULL, schema_new->name,
            "e-01", 0, node_new);
}

/* ========================================================================
 * Variant A callbacks (feature-02 was enabled — leaf-14 existed)
 * ======================================================================== */

/**
 * MODIF-01, variant A: leaf-16 gets value from old leaf-14.
 */
static LY_ERR
root_leaf16_feat_cb(const struct lyd_node *data_old, const struct lyd_node *node_old,
        const struct lysc_node *schema_old, struct lyd_node *parent_new, const struct lysc_node *schema_new,
        const struct lyd_node *node_cmp, const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *old_leaf14;
    const char *val = "default-val";

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!lyd_find_path(data_old, "/ydct:leaf-14", 0, &old_leaf14)) {
        val = lyd_get_value(old_leaf14);
    }

    return lyd_new_term(parent_new, schema_new->module, schema_new->name,
            val, 0, node_new);
}

/* ========================================================================
 * Variant B callbacks (feature-02 was NOT enabled — leaf-14 didn't exist)
 * ======================================================================== */

/**
 * MODIF-01, variant B: leaf-16 gets default value "default-val".
 */
static LY_ERR
root_leaf16_nofeat_cb(const struct lyd_node *data_old, const struct lyd_node *node_old,
        const struct lysc_node *schema_old, struct lyd_node *parent_new, const struct lysc_node *schema_new,
        const struct lyd_node *node_cmp, const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_term(parent_new, schema_new->module, schema_new->name,
            "default-val", 0, node_new);
}

/* ========================================================================
 * Rules — Variant A (feature-02 enabled in old)
 * ======================================================================== */

static struct lyu_plg_rule rules_ydct_15_20_feat[] = {
    { .node_path = "/ydct:leaf-16",            .node_cb = root_leaf16_feat_cb },
    { .node_path = "/ydct:list-02/leaf-03",    .node_cb = list02_leaf03_cb },
    { 0 }
};

/* ========================================================================
 * Rules — Variant B (feature-02 NOT enabled in old)
 * ======================================================================== */

static struct lyu_plg_rule rules_ydct_15_20_nofeat[] = {
    { .node_path = "/ydct:leaf-16",            .node_cb = root_leaf16_nofeat_cb },
    { .node_path = "/ydct:list-02/leaf-03",    .node_cb = list02_leaf03_cb },
    { 0 }
};

/* ========================================================================
 * Feature arrays
 * ======================================================================== */

static const char *features_old_feat[] = {
    "feature-02",
    NULL
};

/* MODIF-03: features_new includes feature-03 */
static const char *features_new_15_20[] = {
    "feature-03",
    NULL
};

static const char *ydct_types_features[] = {
    "feature-01",
    NULL
};

/* ========================================================================
 * Import arrays
 * ======================================================================== */

static struct lyu_plg_module imports_old_15_20_feat[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { .name = "ydct-types", .revision = "2026-01-10", .features = (const char **)ydct_types_features, .implemented = 1 },
    { .name = "ydct-group", .revision = "2026-01-15", .features = NULL, .implemented = 1 },
    { 0 }
};

static struct lyu_plg_module imports_old_15_20_nofeat[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { .name = "ydct-types", .revision = "2026-01-10", .features = (const char **)ydct_types_features, .implemented = 1 },
    { .name = "ydct-group", .revision = "2026-01-15", .features = NULL, .implemented = 1 },
    { 0 }
};

static struct lyu_plg_module imports_new_15_20[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { .name = "ydct-types", .revision = "2026-01-10", .features = (const char **)ydct_types_features, .implemented = 1 },
    { .name = "ydct-group", .revision = "2026-01-15", .features = NULL, .implemented = 1 },
    { 0 }
};

/* ========================================================================
 * Plugin structs — two variants for the same revision pair
 * ======================================================================== */

struct lyu_plg plg_ydct_15_20_feat = {
    .module_name = "ydct",
    .revision_old = "2026-01-15",
    .features_old = (const char **)features_old_feat,
    .imports_old = imports_old_15_20_feat,
    .revision_new = "2026-01-20",
    .features_new = (const char **)features_new_15_20,
    .imports_new = imports_new_15_20,
    .rules = rules_ydct_15_20_feat
};

struct lyu_plg plg_ydct_15_20_nofeat = {
    .module_name = "ydct",
    .revision_old = "2026-01-15",
    .features_old = NULL,
    .imports_old = imports_old_15_20_nofeat,
    .revision_new = "2026-01-20",
    .features_new = (const char **)features_new_15_20,
    .imports_new = imports_new_15_20,
    .rules = rules_ydct_15_20_nofeat
};
