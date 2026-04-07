/**
 * @file data-mod-plugin-2026-01-05-to-2026-01-10.c
 * @brief yangupdate plugin for ydct 2026-01-05 -> 2026-01-10
 *
 * Handles the following incompatible modifications:
 *   MODIF-01: container-01/leaf-02 enum -> identityref
 *   MODIF-02: leaf-04/leaf-05 removed, replaced by choice-01 (leaf-07, leaf-08)
 *   MODIF-03: container-02 renamed to container-03
 *   MODIF-04: list-01/leaf-03 moved to list-01/container-01/leaf-01
 *   MODIF-05: container-01/leaf-01 must ". <= 80"
 *   MODIF-06: list-01/leaf-02 enum -> uint8
 *   MODIF-07: list-01/leaf-04 when "../leaf-02 = 1"
 *   MODIF-08: list-01 second key leaf-05
 *   MODIF-09: container-01 uses grouping-01, refine mandatory
 *   MODIF-10: feature-01, mandatory leaf-10
 *   MODIF-11: augment container-01 with aug01-leaf-list-01
 *   MODIF-12: leaf-list-01 min-elements 2
 *   MODIF-13: leaf-01 type leafref
 *   MODIF-14: list-01 max-elements 3
 *   MODIF-15: leaf-02 uint32 -> uint8 range 1..50
 */
#define _GNU_SOURCE

#include "yang_update.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * Root-level callbacks
 * ======================================================================== */

/**
 * MODIF-13: leaf-01 type changed to leafref (path "/container-03/list-01/leaf-01").
 * Old value may not be a valid list key -> dynamically find the lexicographically
 * smallest key from old container-02/list-01.
 */
static LY_ERR
root_leaf01_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *old_container, *old_entry, *key_node;
    const char *best_key = NULL;
    const char *cur_key;

    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!node_old) {
        return LY_SUCCESS;
    }

    /* Find lexicographically smallest key in old container-02/list-01 */
    if (!lyd_find_path(data_old, "/ydct:container-02", 0, &old_container)) {
        old_entry = lyd_child(old_container);
        while (old_entry) {
            key_node = lyd_child(old_entry);
            while (key_node) {
                if (!strcmp(key_node->schema->name, "leaf-01")) {
                    cur_key = lyd_get_value(key_node);
                    if (!best_key || strcmp(cur_key, best_key) < 0) {
                        best_key = cur_key;
                    }
                    break;
                }
                key_node = key_node->next;
            }
            old_entry = old_entry->next;
        }
    }

    if (!best_key) {
        return LY_SUCCESS;
    }

    return lyd_new_term(parent_new, schema_new->module, schema_new->name,
            best_key, 0, node_new);
}

/**
 * MODIF-15: leaf-02 type narrowed from uint32 to uint8 (range 1..50).
 * Clamp values > 50 to 50.
 */
static LY_ERR
root_leaf02_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
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
    if (val > 50) {
        return lyd_new_term(parent_new, schema_new->module, schema_new->name,
                "50", 0, node_new);
    }
    if (val < 1) {
        return lyd_new_term(parent_new, schema_new->module, schema_new->name,
                "1", 0, node_new);
    }

    return lyd_dup_single_to_ctx(node_old, schema_new->module->ctx, parent_new, 0, node_new);
}

/**
 * MODIF-02: leaf-07 (boolean) from bits b-01 in old leaf-04.
 * Read old leaf-04 value and check for "b-01".
 */
static LY_ERR
root_leaf07_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *old_leaf04;
    const char *bits_val;

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!lyd_find_path(data_old, "/ydct:leaf-04", 0, &old_leaf04)) {
        bits_val = lyd_get_value(old_leaf04);
        if (strstr(bits_val, "b-01")) {
            return lyd_new_term(parent_new, schema_new->module, schema_new->name,
                    "true", 0, node_new);
        }
    }

    return lyd_new_term(parent_new, schema_new->module, schema_new->name,
            "false", 0, node_new);
}

/**
 * MODIF-02: leaf-08 (boolean) from bits b-02 in old leaf-04.
 * Read old leaf-04 value and check for "b-02".
 */
static LY_ERR
root_leaf08_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    struct lyd_node *old_leaf04;
    const char *bits_val;

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!lyd_find_path(data_old, "/ydct:leaf-04", 0, &old_leaf04)) {
        bits_val = lyd_get_value(old_leaf04);
        if (strstr(bits_val, "b-02")) {
            return lyd_new_term(parent_new, schema_new->module, schema_new->name,
                    "true", 0, node_new);
        }
    }

    return lyd_new_term(parent_new, schema_new->module, schema_new->name,
            "false", 0, node_new);
}

/**
 * MODIF-10: leaf-10 (string, mandatory, if-feature "feature-01").
 * New mandatory leaf — provide default value.
 */
static LY_ERR
root_leaf10_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_term(parent_new, schema_new->module, schema_new->name,
            "enabled", 0, node_new);
}

/**
 * MODIF-12: leaf-list-01 (string, min-elements 2, ordered-by user).
 * New mandatory leaf-list — provide at least 2 values.
 * Called repeatedly until node_new is NULL.
 */
static LY_ERR
root_leaf_list01_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    long idx = (long)*user_data;

    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;

    if (idx == 0) {
        *user_data = (const void *)1;
        return lyd_new_term(parent_new, schema_new->module, schema_new->name,
                "item-01", 0, node_new);
    } else if (idx == 1) {
        *user_data = (const void *)2;
        return lyd_new_term(parent_new, schema_new->module, schema_new->name,
                "item-02", 0, node_new);
    }

    *node_new = NULL;
    return LY_SUCCESS;
}

/* ========================================================================
 * container-01 callbacks
 * ======================================================================== */

/**
 * MODIF-05: container-01/leaf-01 must ". <= 80".
 * Clamp values > 80 to 80.
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
    if (val > 80) {
        return lyd_new_term(parent_new, NULL, schema_new->name, "80", 0, node_new);
    }

    return lyd_dup_single_to_ctx(node_old, schema_new->module->ctx, parent_new, 0, node_new);
}

/**
 * MODIF-01: container-01/leaf-02 enum -> identityref.
 * Map: "e-02" -> "ydct-types:identity-02", "e-03" -> "ydct-types:identity-03".
 */
static LY_ERR
cont01_leaf02_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    const char *old_val;
    const char *new_val;

    (void)data_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    if (!node_old) {
        return LY_SUCCESS;
    }

    old_val = lyd_get_value(node_old);
    if (!strcmp(old_val, "e-02")) {
        new_val = "ydct-types:identity-02";
    } else if (!strcmp(old_val, "e-03")) {
        new_val = "ydct-types:identity-03";
    } else {
        new_val = "ydct-types:identity-02";
    }

    return lyd_new_term(parent_new, NULL, schema_new->name, new_val, 0, node_new);
}

/**
 * MODIF-09: container-01/ygrp-g01-leaf-01 (mandatory from grouping).
 * New mandatory leaf from uses grouping-01 with refine.
 */
static LY_ERR
cont01_ygrp_g01_leaf01_cb(const struct lyd_node *data_old, const struct lyd_node *node_old,
        const struct lysc_node *schema_old, struct lyd_node *parent_new, const struct lysc_node *schema_new,
        const struct lyd_node *node_cmp, const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_term(parent_new, NULL, schema_new->name,
            "default-label", 0, node_new);
}

/**
 * MODIF-11: container-01/aug01-leaf-list-01 (min-elements 1).
 * New mandatory leaf-list from same-module augment.
 * Called repeatedly until node_new is NULL.
 */
static LY_ERR
cont01_aug01_leaf_list01_cb(const struct lyd_node *data_old, const struct lyd_node *node_old,
        const struct lysc_node *schema_old, struct lyd_node *parent_new, const struct lysc_node *schema_new,
        const struct lyd_node *node_cmp, const void **user_data, struct lyd_node **node_new)
{
    long idx = (long)*user_data;

    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;

    if (idx == 0) {
        *user_data = (const void *)1;
        return lyd_new_term(parent_new, NULL, schema_new->name,
                "default-tag", 0, node_new);
    }

    *node_new = NULL;
    return LY_SUCCESS;
}

/* ========================================================================
 * container-03 callbacks (renamed from container-02)
 * ======================================================================== */

/**
 * MODIF-03: container-03 (renamed from container-02).
 * Create the new container node.
 */
static LY_ERR
cont03_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_inner(parent_new, schema_new->module, schema_new->name, 0, node_new);
}

/**
 * MODIF-17: container-04 (presence container).
 * Create the presence container so it appears in converted data.
 */
static LY_ERR
cont04_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    (void)data_old;
    (void)node_old;
    (void)schema_old;
    (void)node_cmp;
    (void)user_data;

    return lyd_new_inner(parent_new, schema_new->module, schema_new->name, 0, node_new);
}

/**
 * Compare helper for qsort — sorts list entries lexicographically by leaf-01 key.
 */
static int
list_entry_cmp(const void *a, const void *b)
{
    const struct lyd_node *ea = *(const struct lyd_node *const *)a;
    const struct lyd_node *eb = *(const struct lyd_node *const *)b;
    const struct lyd_node *ka, *kb;
    const char *va = "", *vb = "";

    ka = lyd_child(ea);
    while (ka) {
        if (!strcmp(ka->schema->name, "leaf-01")) {
            va = lyd_get_value(ka);
            break;
        }
        ka = ka->next;
    }
    kb = lyd_child(eb);
    while (kb) {
        if (!strcmp(kb->schema->name, "leaf-01")) {
            vb = lyd_get_value(kb);
            break;
        }
        kb = kb->next;
    }
    return strcmp(va, vb);
}

/**
 * MODIF-03/04/06/07/08/14: container-03/list-01 entries.
 * Creates list entries from old container-02/list-01 with all transformations:
 *   - MODIF-06: enum->uint8 mapping for leaf-02
 *   - MODIF-07: when condition for leaf-04
 *   - MODIF-08: sequential leaf-05 as second key
 *   - MODIF-04: move leaf-03 to container-01/leaf-01
 *   - MODIF-14: max-elements 3 (keep first 3 in lexicographic order by key)
 * Called repeatedly until node_new is NULL.
 */
static LY_ERR
cont03_list01_cb(const struct lyd_node *data_old, const struct lyd_node *node_old, const struct lysc_node *schema_old,
        struct lyd_node *parent_new, const struct lysc_node *schema_new, const struct lyd_node *node_cmp,
        const void **user_data, struct lyd_node **node_new)
{
    long call_idx = (long)*user_data;
    struct lyd_node *old_container, *old_entry, *child;
    const char *leaf01_val, *leaf02_val, *leaf03_val, *leaf04_val;
    int leaf02_uint;
    char idx_str[21];
    struct lyd_node *entry, *inner;
    /* Max 64 list entries for sorting */
    struct lyd_node *entries[64];
    long entry_count = 0;
    LY_ERR rc;

    (void)node_old;
    (void)schema_old;
    (void)node_cmp;

    /* Find old container-02 */
    if (lyd_find_path(data_old, "/ydct:container-02", 0, &old_container)) {
        *node_new = NULL;
        return LY_SUCCESS;
    }

    /* Collect all list entries into array */
    old_entry = lyd_child(old_container);
    while (old_entry && entry_count < 64) {
        entries[entry_count++] = old_entry;
        old_entry = old_entry->next;
    }

    /* Sort lexicographically by leaf-01 key */
    qsort(entries, entry_count, sizeof(entries[0]), list_entry_cmp);

    /* MODIF-14: max-elements 3 — take first 3 in sorted order */
    if (call_idx >= 3 || call_idx >= entry_count) {
        *node_new = NULL;
        return LY_SUCCESS;
    }

    old_entry = entries[call_idx];

    /* Read old leaf values */
    leaf01_val = NULL;
    leaf02_val = NULL;
    leaf03_val = NULL;
    leaf04_val = NULL;

    child = lyd_child(old_entry);
    while (child) {
        if (!strcmp(child->schema->name, "leaf-01")) {
            leaf01_val = lyd_get_value(child);
        } else if (!strcmp(child->schema->name, "leaf-02")) {
            leaf02_val = lyd_get_value(child);
        } else if (!strcmp(child->schema->name, "leaf-03")) {
            leaf03_val = lyd_get_value(child);
        } else if (!strcmp(child->schema->name, "leaf-04")) {
            leaf04_val = lyd_get_value(child);
        }
        child = child->next;
    }

    /* MODIF-06: map enum to uint8 */
    if (leaf02_val && !strcmp(leaf02_val, "e-01")) {
        leaf02_uint = 1;
    } else {
        leaf02_uint = 2;
    }

    /* MODIF-08: sequential key leaf-05 (1-based) */
    snprintf(idx_str, sizeof(idx_str), "%ld", call_idx + 1);

    /* Create new list entry with both keys */
    rc = lyd_new_list(parent_new, schema_new->module, schema_new->name,
            0, &entry, leaf01_val, idx_str);
    if (rc) {
        return rc;
    }

    /* MODIF-06: leaf-02 as uint8 */
    {
        char uint_str[16];
        snprintf(uint_str, sizeof(uint_str), "%d", leaf02_uint);
        rc = lyd_new_term(entry, NULL, "leaf-02", uint_str, 0, NULL);
        if (rc) {
            return rc;
        }
    }

    /* MODIF-07: leaf-04 only if leaf-02 = 1 (when condition) */
    if (leaf04_val && leaf02_uint == 1) {
        rc = lyd_new_term(entry, NULL, "leaf-04", leaf04_val, 0, NULL);
        if (rc) {
            return rc;
        }
    }

    /* MODIF-04: move old leaf-03 to container-01/leaf-01 */
    if (leaf03_val) {
        rc = lyd_new_inner(entry, NULL, "container-01", 0, &inner);
        if (rc) {
            return rc;
        }
        rc = lyd_new_term(inner, NULL, "leaf-01", leaf03_val, 0, NULL);
        if (rc) {
            return rc;
        }
    }

    *node_new = entry;
    *user_data = (const void *)(call_idx + 1);
    return LY_SUCCESS;
}

/* ========================================================================
 * Rules and plugin struct
 * ======================================================================== */

static struct lyu_plg_rule rules_ydct_05_10[] = {
    /* Root level */
    { .node_path = "/ydct:leaf-01",                                .node_cb = root_leaf01_cb },
    { .node_path = "/ydct:leaf-02",                                .node_cb = root_leaf02_cb },
    { .node_path = "/ydct:leaf-07",                                .node_cb = root_leaf07_cb },
    { .node_path = "/ydct:leaf-08",                                .node_cb = root_leaf08_cb },
    { .node_path = "/ydct:leaf-10",                                .node_cb = root_leaf10_cb },
    { .node_path = "/ydct:leaf-list-01",                           .node_cb = root_leaf_list01_cb },
    /* container-01 */
    { .node_path = "/ydct:container-01/leaf-01",                   .node_cb = cont01_leaf01_cb },
    { .node_path = "/ydct:container-01/leaf-02",                   .node_cb = cont01_leaf02_cb },
    { .node_path = "/ydct:container-01/ygrp-g01-leaf-01",          .node_cb = cont01_ygrp_g01_leaf01_cb },
    { .node_path = "/ydct:container-01/aug01-leaf-list-01",        .node_cb = cont01_aug01_leaf_list01_cb },
    /* container-03 (renamed from container-02) */
    { .node_path = "/ydct:container-03",                           .node_cb = cont03_cb },
    { .node_path = "/ydct:container-03/list-01",                   .node_cb = cont03_list01_cb },
    /* container-04 (presence) */
    { .node_path = "/ydct:container-04",                           .node_cb = cont04_cb },
    { 0 }
};

static const char *features_new_ydct_05_10[] = {
    "feature-01",
    NULL
};

static const char *ydct_types_features[] = {
    "feature-01",
    NULL
};

static struct lyu_plg_module imports_new_ydct_05_10[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { .name = "ydct-types", .revision = "2026-01-10", .features = (const char **)ydct_types_features, .implemented = 1 },
    { .name = "ydct-group", .revision = "2026-01-10", .features = NULL, .implemented = 0 },
    { 0 }
};

static struct lyu_plg_module imports_old_ydct_05_10[] = {
    { .name = "ietf-inet-types", .revision = "2025-12-22", .features = NULL, .implemented = 0 },
    { 0 }
};

struct lyu_plg plg_ydct_05_10 = {
    .module_name = "ydct",
    .revision_old = "2026-01-05",
    .features_old = NULL,
    .imports_old = imports_old_ydct_05_10,
    .revision_new = "2026-01-10",
    .features_new = (const char **)features_new_ydct_05_10,
    .imports_new = imports_new_ydct_05_10,
    .rules = rules_ydct_05_10
};
