/**
 * @file printer_sid.c
 * @author Petr Hanzlik <Petr.Hanzlik@cesnet.cz>
 * @brief Generation and update of .sid files (RFC 9595).
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compat.h"
#include "context.h"
#include "log.h"
#include "ly_common.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "version.h"

/**
 * @brief Internal representation of a single SID item before it is written
 * into the ietf-sid-file data tree.
 *
 * Ownership of the strings depends on the producer and must be respected by every cleanup path; the type itself does not express it.
 */
struct sid_item {
    char *ns;       /**< Namespace string: "module", "identity", "feature", or "data". */
    char *ident;    /**< Identifier: module/submodule name, identity/feature name, or absolute schema-node path. */
    uint64_t sid;   /**< SID assigned to the item (0 until assigned). */
    char *status;   /**< Item status: "unstable"/"obsolete", or NULL. */
};

/**
 * @brief Collection state for gathering SID items, also passed as data to the DFS callback collecting "data" namespace items.
 */
struct sid_collect_data {
    struct sid_item *items;   /**< Items array being filled. */
    uint64_t count;           /**< Number of items collected so far; exceeding @p max, surplus items are only counted. */
    uint64_t max;             /**< Maximum number of items (capacity of the array). */
    struct ly_ctx *ctx;       /**< libyang context used for error logging. */
    const struct lys_module *module;  /**< Module whose nodes are collected; used to attribute augmented nodes to their defining module. */
};

/**
 * @brief Assignment range of an .sid file.
 */
struct sid_range {
    uint64_t entry_point;  /**< First SID of the range. */
    uint64_t size;         /**< Number of SIDs in the range. */
};

/**
 * @brief Comparator for qsort that sorts SID items.
 *
 * Sort order alphabetical:
 *   1. Namespace descending.
 *   2. Identifier ascending.
 *
 * @param[in] a First item (struct sid_item *).
 * @param[in] b Second item (struct sid_item *).
 * @return <0 if @p a comes before @p b, 0 if equal, >0 if @p a comes after @p b.
 */
static int
compare_sid(const void *a, const void *b)
{
    const struct sid_item *item_a = a, *item_b = b;
    char ns_a, ns_b;

    ns_a = item_a->ns[0];
    ns_b = item_b->ns[0];

    if (ns_a == ns_b) {
        return strcmp(item_a->ident, item_b->ident);
    }

    return ns_b - ns_a;
}

/**
 * @brief Add one collected item to the collection array.
 *
 * @param[in,out] collect Collection state (items/count/max, ctx used for error logging).
 * @param[in] ns Namespace string: "module", "identity", "feature", or "data".
 * @param[in] name Identifier string to duplicate into the item.
 * @return LY_SUCCESS on success (also if the capacity is exceeded, the item is then only counted and the caller reports the overflow after full collection).
 * @return LY_EMEM on memory allocation failure.
 */
static LY_ERR
sid_item_add(struct sid_collect_data *collect, const char *ns, const char *name)
{
    /* overflow is only counted here; the exact number of excess items is
     * reported by the caller after the whole schema has been collected */
    if (collect->count >= collect->max) {
        collect->count++;
        return LY_SUCCESS;
    }

    collect->items[collect->count].ns = (char *)ns;
    collect->items[collect->count].ident = strdup(name);
    LY_CHECK_ERR_RET(!collect->items[collect->count].ident, LOGMEM(collect->ctx), LY_EMEM);

    collect->count++;
    return LY_SUCCESS;
}

/**
 * @brief Build the schema-node-path identifier of a compiled node as required by RFC 9595:
 * like ::lysc_path with ::LYSC_PATH_LOG, but choice and case node names are omitted
 * (they never get a SID and never appear in identifiers), while input and output are kept.
 *
 * @param[in] node Node to build the identifier for.
 * @return Newly allocated path, the caller frees it.
 * @return NULL on memory allocation failure.
 */
static char *
sid_node_path(const struct lysc_node *node)
{
    const struct lysc_node *iter, *par;
    char *path = NULL;
    int len = 0;

    for (iter = node; iter && (len >= 0); iter = iter->parent) {
        char *s;

        if (iter->nodetype & (LYS_CHOICE | LYS_CASE)) {
            /* never part of a SID identifier, but children must be reached */
            continue;
        }

        /* nearest non-choice/case ancestor, for the module prefix decision */
        for (par = iter->parent; par && (par->nodetype & (LYS_CHOICE | LYS_CASE)); par = par->parent) {}

        s = path;
        if (!par || (par->module != iter->module)) {
            /* module prefix (top-level node or module change through an augment) */
            len = asprintf(&path, "/%s:%s%s", iter->module->name, iter->name, s ? s : "");
        } else {
            len = asprintf(&path, "/%s%s", iter->name, s ? s : "");
        }
        free(s);
    }

    if (len < 0) {
        free(path);
        return NULL;
    }
    return path;
}

/**
 * @brief DFS callback for ::lysc_module_dfs_full that collects each schema node as a "data" namespace item.
 *
 * @param[in] node Current schema node.
 * @param[in,out] data Pointer to ::sid_collect_data.
 * @param[out] dfs_continue Unused, the whole tree is always traversed.
 * @return LY_SUCCESS on success.
 * @return LY_EMEM on allocation failure.
 */
static LY_ERR
collect_data_cb(struct lysc_node *node, void *data, ly_bool *UNUSED(dfs_continue))
{
    struct sid_collect_data *collect_data = data;
    LY_ERR rc;
    char *path;

    if (node->nodetype & (LYS_CHOICE | LYS_CASE)) {
        return LY_SUCCESS; /* choice/case get no item, but their subtree is still traversed */
    }

    /* collect only nodes defined by the module being processed; when another
     * module's tree is traversed this keeps exactly the nodes augmented in by
     * the processed module and skips the target module's own nodes */
    if (node->module != collect_data->module) {
        return LY_SUCCESS;
    }

    path = sid_node_path(node);
    LY_CHECK_ERR_RET(!path, LOGMEM(collect_data->ctx), LY_EMEM);

    rc = sid_item_add(collect_data, "data", path);
    free(path);
    return rc;
}

/**
 * @brief Add assignment range to the sid-file data tree.
 *
 * @param[in] tree Sid-file root to add the range to.
 * @param[in] entry_point First SID of the range.
 * @param[in] size Number of SIDs in the range.
 * @return LY_SUCCESS on success.
 * @return LY_EMEM on memory allocation failure.
 * @return LY_ERR on other errors.
 */
static LY_ERR
sid_range_add(struct lyd_node *tree, uint64_t entry_point, uint64_t size)
{
    LY_ERR rc = LY_SUCCESS;
    char size_str[21];
    char *item_path = NULL;

    /* the whole new range must fit inside the SID data type bound 2^63-1; */
    LY_CHECK_ERR_GOTO(size - 1 > (((uint64_t)1 << 63) - 1) - entry_point,
            (LOGERR(LYD_CTX(tree), LY_EINVAL,
                    "The assignment range [%" PRIu64 ", %" PRIu64 "] exceeds the SID data type bounds.",
                    entry_point, entry_point + size - 1), rc = LY_EINVAL), cleanup);

    snprintf(size_str, sizeof size_str, "%" PRIu64, size);

    LY_CHECK_ERR_GOTO(asprintf(&item_path, "assignment-range[entry-point='%" PRIu64 "']/size", entry_point) == -1,
            rc = LY_EMEM, cleanup);
    LY_CHECK_GOTO((rc = lyd_new_path(tree, NULL, item_path, size_str, 0, NULL)), cleanup);

cleanup:
    free(item_path);
    return rc;
}

/**
 * @brief Build the .sid file skeleton: root structure, module identity, status, description, dependency-revision list, and assignment-range list.
 *
 * @param[in] module Module used for module-name, module-revision, and dependency-revision entries.
 * @param[in] ranges Array of assignment ranges.
 * @param[in] range_count Number of elements in @p ranges.
 * @param[in] version Value of the sid-file-version leaf, 0 to omit it
 * @param[in] status Status of the SID file; for ::LYS_SID_FILE_PUBLISHED the "sid-file-status" leaf is
 *      omitted (its schema default is "published").
 * @param[in] description Optional description string, NULL to auto-generate.
 * @param[out] tree Generated /ietf-sid-file:sid-file node, NULL if creation failed.
 * @return LY_SUCCESS on success.
 * @return LY_EMEM on memory allocation failure.
 * @return LY_ERR on other errors.
 */
static LY_ERR
sid_build_skeleton(const struct lys_module *module, const struct sid_range *ranges, size_t range_count,
        uint32_t version, LYS_SID_FILE_STATUS status, const char *description, struct lyd_node **tree)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysp_import *imports;
    char ver_str[11];
    char *item_path = NULL, *desc_buf = NULL, *time_str = NULL;
    time_t now = time(NULL);
    LY_ARRAY_COUNT_TYPE iter;

    *tree = NULL;

    LY_CHECK_GOTO((rc = lyd_new_path(NULL, module->ctx, "/ietf-sid-file:sid-file", NULL, 0, tree)), cleanup);

    LY_CHECK_GOTO((rc = lyd_new_path(*tree, NULL, "module-name", module->name, 0, NULL)), cleanup);

    if (module->revision) {
        LY_CHECK_GOTO((rc = lyd_new_path(*tree, NULL, "module-revision", module->revision, 0, NULL)), cleanup);
    }

    if (version) {
        snprintf(ver_str, sizeof ver_str, "%" PRIu32, version);
        LY_CHECK_GOTO((rc = lyd_new_path(*tree, NULL, "sid-file-version", ver_str, 0, NULL)), cleanup);
    }

    if (status == LYS_SID_FILE_UNPUBLISHED) {
        LY_CHECK_GOTO((rc = lyd_new_path(*tree, NULL, "sid-file-status", "unpublished", 0, NULL)), cleanup);
    }

    if (!description) {
        /* auto-generate with libyang version and UTC timestamp */
        LY_CHECK_GOTO((rc = ly_time_time2str(now, NULL, &time_str)), cleanup);
        LY_CHECK_ERR_GOTO(asprintf(&desc_buf, "Generated by libyang %s, at %s", ly_version_so_str(), time_str) == -1,
                rc = LY_EMEM, cleanup);
        description = desc_buf;
    }
    LY_CHECK_GOTO((rc = lyd_new_path(*tree, NULL, "description", description, 0, NULL)), cleanup);
    free(desc_buf);
    desc_buf = NULL;

    /* dependency-revision: the ietf-sid-file model requires the module revision here,
       but the 'revision' statement is optional in YANG and some modules simply do
       not have one, so it can happen that an import has no revision to record and the entry is skipped. */
    imports = module->parsed->imports;
    LY_ARRAY_FOR(imports, iter) {
        if (imports[iter].module->revision) {
            LY_CHECK_ERR_GOTO(asprintf(&item_path, "dependency-revision[module-name='%s']/module-revision",
                    imports[iter].name) == -1, rc = LY_EMEM, cleanup);
            LY_CHECK_GOTO((rc = lyd_new_path(*tree, NULL, item_path, imports[iter].module->revision, 0, NULL)), cleanup);
            free(item_path);
            item_path = NULL;
        }
    }

    /* One .sid file may legally declare several assignment ranges. */
    for (size_t i = 0; i < range_count; i++) {
        LY_CHECK_GOTO((rc = sid_range_add(*tree, ranges[i].entry_point, ranges[i].size)), cleanup);
    }

cleanup:
    if (rc) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    free(item_path);
    free(desc_buf);
    free(time_str);
    return rc;
}

/**
 * @brief Collect all SID items of a module ("module", "identity", "feature", and "data" namespaces) and sort them.
 *
 * @param[in,out] callback_data Collection state: preallocated items array, capacity in max, ctx set for error logging.
 * @param[in] module Compiled module to collect the items from.
 * @return LY_SUCCESS on success, the sorted items and their count are in @p callback_data.
 * @return LY_EINVAL if the capacity (the assignment range size) is too small for all collected items.
 * @return LY_EMEM on memory allocation failure.
 * @return LY_ERR on other errors.
 */
static LY_ERR
sid_collect_items(struct sid_collect_data *callback_data, const struct lys_module *module)
{
    LY_ERR rc;
    LY_ARRAY_COUNT_TYPE i;
    uint32_t feature_idx = 0;
    const struct lysp_feature *feature = NULL;
    struct lysc_node **top = NULL;

    /* module namespace: the module name itself (RFC 9595) */
    LY_CHECK_RET((rc = sid_item_add(callback_data, "module", module->name)), rc);

    /* module namespace: each submodule name (RFC 9595) */
    LY_ARRAY_FOR(module->submodules, i) {
        LY_CHECK_RET((rc = sid_item_add(callback_data, "module", module->submodules[i].name)), rc);
    }

    /* identity namespace: all identities defined in the module and its submodules. */
    LY_ARRAY_FOR(module->identities, i) {
        LY_CHECK_RET((rc = sid_item_add(callback_data, "identity", module->identities[i].name)), rc);
    }

    /* feature namespace: all features defined in the module and its submodules.*/
    while ((feature = lysp_feature_next(feature, module->parsed, &feature_idx))) {
        LY_CHECK_RET((rc = sid_item_add(callback_data, "feature", feature->name)), rc);
    }

    /* collect_data_cb attributes nodes to their defining module, so foreign
       nodes are skipped whenever another module's tree is traversed below */
    callback_data->module = module;

    /* data namespace: walk the entire compiled schema tree depth-first.
       lysc_module_dfs_full traverses all nodes including RPCs, actions,
       notifications, input, output, choice and case nodes. sid_node_path() builds the
       RFC 9595 schema-node-path identifiers (choice/case names omitted). */
    LY_CHECK_RET((rc = lysc_module_dfs_full(module, collect_data_cb, callback_data)), rc);

    /* data namespace: nodes that this module augments into other modules live in
       the target modules' trees (RFC 9595 still assigns them to this module's .sid).
       Traverse every context module that lists this module in its augmented_by and
       collect the nodes defined here (collect_data_cb filters by defining module). */
    {
        const struct lys_module *aug_target;
        uint32_t mod_idx = 0;

        while ((aug_target = ly_ctx_get_module_iter(module->ctx, &mod_idx))) {
            ly_bool augmented = 0;

            if ((aug_target == module) || !aug_target->compiled) {
                continue;
            }
            LY_ARRAY_FOR(aug_target->augmented_by, i) {
                if (aug_target->augmented_by[i] == module) {
                    augmented = 1;
                    break;
                }
            }
            if (augmented) {
                LY_CHECK_RET((rc = lysc_module_dfs_full(aug_target, collect_data_cb, callback_data)), rc);
            }
        }
    }

    /* data namespace: also traverse the data trees of compiled top-level extension
       instances that define their own data tree outside the standard module trees
       (rc:yang-data, sx:structure); the top-level data node is obtained from the
       generic compiled extension storage via lyplg_ext_get_storage_p(). */
    LY_ARRAY_FOR(module->compiled->exts, i) {
        lyplg_ext_get_storage_p(&module->compiled->exts[i], LY_STMT_DATA_NODE_MASK, (void ***)&top);

        if (top) {
            LY_CHECK_RET((rc = lysc_tree_dfs_full(*top, collect_data_cb, callback_data)), rc);
        }
    }

    /* the range must cover all collected items; the excess is counted by sid_item_add */
    LY_CHECK_RET(callback_data->count > callback_data->max, (LOGERR(module->ctx, LY_EINVAL, ".sid assignment range(s) size %" PRIu64 " is too small, %" PRIu64 " extra SID(s) are required.",
            callback_data->max, callback_data->count - callback_data->max), LY_EINVAL));

    qsort(callback_data->items, callback_data->count, sizeof(struct sid_item), compare_sid);

    return LY_SUCCESS;
}

/**
 * @brief Free the collected items array including all owned identifiers.
 *
 * @param[in,out] callback_data Collection state; its items array is freed and set to NULL.
 */
static void
sid_collect_data_free(struct sid_collect_data *callback_data)
{
    if (!callback_data || !callback_data->items) {
        return;
    }

    /* The count may exceed the maximum capacity when the assignment range was too small. */
    for (uint64_t i = 0; (i < callback_data->count) && (i < callback_data->max); i++) {
        free(callback_data->items[i].ident);
    }
    free(callback_data->items);
    callback_data->items = NULL;
}

/**
 * @brief Emit an item entry into the sid-file data tree.
 *
 * @param[in] item Item to add to sid-file.
 * @param[in, out] tree Sid-file root to add the item to.
 * @return LY_SUCCESS on success.
 * @return LY_EMEM on memory allocation failure.
 * @return LY_ERR on other errors.
 */
static LY_ERR
sid_emit_item(const struct sid_item *item, struct lyd_node *tree)
{
    LY_ERR rc = LY_SUCCESS;
    char sid_str[21];
    char *item_path = NULL;

    snprintf(sid_str, sizeof sid_str, "%" PRIu64, item->sid);
    LY_CHECK_ERR_GOTO(asprintf(&item_path, "item[namespace='%s'][identifier='%s']/sid", item->ns, item->ident) == -1,
            rc = LY_EMEM, cleanup);
    LY_CHECK_GOTO((rc = lyd_new_path(tree, NULL, item_path, sid_str, 0, NULL)), cleanup);
    free(item_path);
    item_path = NULL;

    if (item->status) {
        LY_CHECK_ERR_GOTO(asprintf(&item_path, "item[namespace='%s'][identifier='%s']/status", item->ns, item->ident) == -1,
                rc = LY_EMEM, cleanup);
        LY_CHECK_GOTO((rc = lyd_new_path(tree, NULL, item_path, item->status, 0, NULL)), cleanup);
        free(item_path);
        item_path = NULL;
    }

cleanup:
    free(item_path);
    return rc;
}

LY_ERR
sid_file_gen(const struct lys_module *module, uint64_t entry_point, uint64_t size,
        LYS_SID_FILE_STATUS status, const char *description, struct lyd_node **sid_file)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *tree = NULL;
    struct sid_item *items = NULL;
    struct sid_collect_data cb_data = {0};
    struct sid_range range = {entry_point, size};
    uint64_t sid;

    /* --- Phase 1: build the sid-file skeleton --- */
    LY_CHECK_GOTO((rc = sid_build_skeleton(module, &range, 1, 0, status, description, &tree)), cleanup);

    /* --- Phase 2: collect items from the compiled schema and sort --- */
    items = calloc(size, sizeof *items);
    LY_CHECK_ERR_GOTO(!items, rc = LY_EMEM, cleanup);

    cb_data.items = items;
    cb_data.max = size;
    cb_data.ctx = module->ctx;

    LY_CHECK_GOTO((rc = sid_collect_items(&cb_data, module)), cleanup);

    /* --- Phase 3: assign SIDs and build the item list --- */

    /* assign sequential SIDs in sorted order */
    sid = entry_point;
    for (uint64_t i = 0; i < cb_data.count; i++) {
        items[i].sid = sid++;
        items[i].status = (status == LYS_SID_FILE_UNPUBLISHED) ? (char *)"unstable" : NULL;

        LY_CHECK_GOTO((rc = sid_emit_item(&items[i], tree)), cleanup);
    }

    *sid_file = tree;
    tree = NULL;

cleanup:
    lyd_free_all(tree);
    sid_collect_data_free(&cb_data);
    return rc;
}

/**
 * @brief One item extracted from a previous .sid file.
 *
 * All strings inside @p item are OWNED by this array entry (strdup'd during
 * extraction) and freed per entry by ::sid_old_items_free.
 */
struct sid_old_item {
    struct sid_item item;  /**< Namespace/identifier/SID/status from the previous file. */
    ly_bool matched;       /**< true once a current schema item claims this entry during merge.
                            *   An obsolete item matched back into the schema keeps its SID and
                            *   is revived: its status is cleared to stable (NULL) at merge time. */
};

/**
 * @brief Comparator for qsort that sorts previous items the same way as ::compare_sid
 *
 * Sort order alphabetical:
 *   1. Namespace descending.
 *   2. Identifier ascending.
 *
 * @param[in] a First item (struct sid_old_item *).
 * @param[in] b Second item (struct sid_old_item *).
 * @return <0 if @p a comes before @p b, 0 if equal, >0 if @p a comes after @p b.
 */
static int
compare_old(const void *a, const void *b)
{
    const struct sid_old_item *item_a = a, *item_b = b;

    return compare_sid(&item_a->item, &item_b->item);
}

/**
 * @brief Look up an item by (namespace, identifier) in the sorted array of
 * previous items.
 *
 * The array must be sorted with ::compare_old and must not contain duplicate
 * (namespace, identifier) keys - when the previous file has duplicates, the
 * first occurrence is kept during extraction and the rest is dropped.
 *
 * @param[in] olds Sorted array of previous items.
 * @param[in] count Number of elements in @p olds.
 * @param[in] ns Namespace of the item.
 * @param[in] ident Identifier of the item.
 * @return Pointer to the matching item.
 * @return NULL if not present.
 */
static struct sid_old_item *
sid_old_find(const struct sid_old_item *olds, uint64_t count, const char *ns, const char *ident)
{
    uint64_t lo = 0, hi = count, mid;
    int cmp;

    while (lo < hi) {
        mid = lo + (hi - lo) / 2;
        if (olds[mid].item.ns[0] == ns[0]) {
            cmp = strcmp(ident, olds[mid].item.ident);
        } else {
            cmp = olds[mid].item.ns[0] - ns[0];
        }
        if (!cmp) {
            return (struct sid_old_item *)&olds[mid];
        } else if (cmp < 0) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return NULL;
}

/**
 * @brief Free an array of previous items including all owned strings.
 *
 * @param[in,out] olds Array to free, set to NULL afterwards.
 * @param[in] count Number of elements in @p olds.
 */
static void
sid_old_items_free(struct sid_old_item **olds, uint64_t count)
{
    if (*olds) {
        for (uint64_t i = 0; i < count; i++) {
            free((*olds)[i].item.ns);
            free((*olds)[i].item.ident);
            free((*olds)[i].item.status);
        }
        free(*olds);
        *olds = NULL;
    }
}

/**
 * @brief Get the canonical string value of a direct leaf child of a data node.
 *
 * @param[in] node Parent data node (sid-file root or a list instance).
 * @param[in] name Schema name of the leaf child.
 * @return Canonical value (borrowed, do not free).
 * @return NULL if the leaf is missing.
 */
static const char *
sid_child_value(const struct lyd_node *node, const char *name)
{
    const struct lyd_node *child;

    LY_LIST_FOR(lyd_child(node), child) {
        if (!strcmp(child->schema->name, name)) {
            return lyd_get_value(child);
        }
    }
    return NULL;
}

/**
 * @brief Find a specific child terminal node (leaf or leaf-list) by schema name.
 *
 * @param[in] node Parent data node whose children will be searched.
 * @param[in] name Schema identifier (name) of the target child node to match.
 *
 * @return Pointer to the matched terminal node (`struct lyd_node_term *`).
 * @return NULL if no matching child is found.
 */
static struct lyd_node_term *
sid_child_term(const struct lyd_node *node, const char *name)
{
    const struct lyd_node *child;

    LY_LIST_FOR(lyd_child(node), child) {
        if (!strcmp(child->schema->name, name)) {
            return (struct lyd_node_term *)child;
        }
    }
    return NULL;
}

LY_ERR
sid_range_append(struct lyd_node *tree, uint64_t entry_point, uint64_t size)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lyd_node *child = NULL;
    uint64_t ep = 0, sz = 0;
    struct lyd_node_term *term = NULL;

    LY_CHECK_ERR_GOTO(!tree->schema || strcmp(tree->schema->name, "sid-file"),
            (LOGERR(LYD_CTX(tree), LY_EINVAL,
                    "Invalid previous .sid file data tree (expected the \"sid-file\" top node)."),
            rc = LY_EINVAL), cleanup);

    /* the new range must not overlap any already declared assignment-range */
    for (child = lyd_child(tree); child; child = child->next) {
        if (strcmp(child->schema->name, "assignment-range")) {
            continue;
        }
        term = sid_child_term(child, "entry-point");
        LY_CHECK_ERR_GOTO(!term,
                (LOGERR(LYD_CTX(tree), LY_EINVAL, "Missing required data in the previous .sid file."), rc = LY_EINVAL), cleanup);
        ep = term->value.uint64;
        term = sid_child_term(child, "size");
        LY_CHECK_ERR_GOTO(!term,
                (LOGERR(LYD_CTX(tree), LY_EINVAL, "Missing required data in the previous .sid file."), rc = LY_EINVAL), cleanup);
        sz = term->value.uint64;

        /* unsigned wrap-around makes a single subtraction sufficient per direction */
        LY_CHECK_ERR_GOTO((ep - entry_point < size) || (entry_point - ep < sz),
                (LOGERR(LYD_CTX(tree), LY_EINVAL,
                        "The assignment range [%" PRIu64 ", %" PRIu64 "] overlaps the existing assignment range [%" PRIu64 ", %" PRIu64 "].",
                        entry_point, entry_point + size - 1, ep, ep + sz - 1), rc = LY_EINVAL), cleanup);
    }

    rc = sid_range_add(tree, entry_point, size);

cleanup:
    return rc;
}

/**
 * @brief Extract the data needed to update an existing .sid file.
 *
 * @param[in] module Module the file belongs to; its name must match the file.
 * @param[in] prev Root node of the previous .sid data tree.
 * @param[out] ranges Newly allocated assignment ranges from the file.
 * @param[out] ranges_count Number of elements in @p ranges.
 * @param[out] version sid-file-version from the file, 0 if the leaf is absent.
 * @param[out] items Newly allocated items with dense entries (duplicate keys excluded), in file order; free with ::sid_old_items_free.
 * @param[out] items_count Number of used entries in @p items.
 * @return LY_SUCCESS on success.
 * @return LY_EINVAL on invalid file content (name mismatch, missing data, no ranges).
 * @return LY_EMEM on memory allocation failure.
 * @return LY_ERR on other errors.
 */
static LY_ERR
sid_extract_file(const struct lys_module *module, const struct lyd_node *prev, struct sid_range **ranges,
        size_t *ranges_count, uint32_t *version, struct sid_old_item **items, uint64_t *items_count)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lyd_node *child = NULL;
    struct sid_range *extracted_ranges = NULL;
    struct sid_old_item *extracted_items = NULL;
    size_t range_idx = 0, total_ranges = 0;
    uint64_t item_idx = 0, total_items = 0, sid_value;
    const char *name = NULL, *ns = NULL, *ident = NULL, *status = NULL;
    char *ns_copy = NULL, *ident_copy = NULL, *status_copy = NULL;
    struct lyd_node_term *term = NULL;
    ly_bool duplicate = 0;

    /* the leaf is optional with default 0 in the schema */
    *version = 0;

    /* module identity, version, counts of both lists */
    LY_LIST_FOR(lyd_child(prev), child) {
        if (!child->schema) {
            continue;
        }
        if (!strcmp(child->schema->name, "module-name")) {
            name = lyd_get_value(child);
            LY_CHECK_ERR_GOTO(strcmp(name, module->name),
                    (LOGERR(module->ctx, LY_EINVAL, "The .sid file module name \"%s\" does not match the "
                            "module \"%s\".", name, module->name), rc = LY_EINVAL), cleanup);
        } else if (!strcmp(child->schema->name, "sid-file-version")) {
            *version = ((const struct lyd_node_term *)child)->value.uint32;
        } else if (!strcmp(child->schema->name, "assignment-range")) {
            total_ranges++;
        } else if (!strcmp(child->schema->name, "item")) {
            total_items++;
        }
    }

    LY_CHECK_ERR_GOTO(!total_ranges,
            (LOGERR(module->ctx, LY_EINVAL, "The .sid file \"%s\" has no assignment-range.",
                    module->name), rc = LY_EINVAL), cleanup);

    extracted_ranges = calloc(total_ranges, sizeof *extracted_ranges);
    LY_CHECK_ERR_GOTO(!extracted_ranges, (LOGMEM(module->ctx), rc = LY_EMEM), cleanup);
    if (total_items) {
        extracted_items = calloc(total_items, sizeof *extracted_items);
        LY_CHECK_ERR_GOTO(!extracted_items, (LOGMEM(module->ctx), rc = LY_EMEM), cleanup);
    }

    /* fill ranges and items */
    LY_LIST_FOR(lyd_child(prev), child) {
        if (!child->schema) {
            continue;
        }

        if (!strcmp(child->schema->name, "assignment-range")) {
            term = sid_child_term(child, "entry-point");
            LY_CHECK_ERR_GOTO(!term,
                    (LOGERR(module->ctx, LY_EINVAL, "Missing required data in the previous .sid file."),
                    rc = LY_EINVAL), cleanup);
            extracted_ranges[range_idx].entry_point = term->value.uint64;

            term = sid_child_term(child, "size");
            LY_CHECK_ERR_GOTO(!term,
                    (LOGERR(module->ctx, LY_EINVAL, "Missing required data in the previous .sid file."),
                    rc = LY_EINVAL), cleanup);
            extracted_ranges[range_idx].size = term->value.uint64;
            range_idx++;

        } else if (!strcmp(child->schema->name, "item")) {
            ns = sid_child_value(child, "namespace");
            ident = sid_child_value(child, "identifier");
            term = sid_child_term(child, "sid");
            LY_CHECK_ERR_GOTO(!ns || !ident || !term,
                    (LOGERR(module->ctx, LY_EINVAL, "Missing required data in the previous .sid file."),
                    rc = LY_EINVAL), cleanup);
            sid_value = term->value.uint64;

            /* duplicate keys: the first occurrence wins */
            duplicate = 0;
            for (uint64_t i = 0; i < item_idx; i++) {
                if (!strcmp(extracted_items[i].item.ns, ns) && !strcmp(extracted_items[i].item.ident, ident)) {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate) {
                continue;
            }

            status = sid_child_value(child, "status");

            ns_copy = strdup(ns);
            LY_CHECK_ERR_GOTO(!ns_copy, (LOGMEM(module->ctx), rc = LY_EMEM), cleanup);
            ident_copy = strdup(ident);
            LY_CHECK_ERR_GOTO(!ident_copy, (free(ns_copy), LOGMEM(module->ctx), rc = LY_EMEM), cleanup);
            /* "stable" is the schema default and is stored as NULL */
            status_copy = (status && strcmp(status, "stable")) ? strdup(status) : NULL;
            LY_CHECK_ERR_GOTO(status && strcmp(status, "stable") && !status_copy,
                    (free(ns_copy), free(ident_copy), LOGMEM(module->ctx), rc = LY_EMEM), cleanup);

            extracted_items[item_idx].item.ns = ns_copy;
            extracted_items[item_idx].item.ident = ident_copy;
            extracted_items[item_idx].item.sid = sid_value;
            extracted_items[item_idx].item.status = status_copy;
            extracted_items[item_idx].matched = 0;
            item_idx++;
        }
    }

    *ranges = extracted_ranges;
    *ranges_count = total_ranges;
    *items = extracted_items;
    *items_count = item_idx;
    return LY_SUCCESS;

cleanup:
    free(extracted_ranges);
    sid_old_items_free(&extracted_items, item_idx);
    return rc;
}

/**
 * @brief Comparator for qsort that sorts assignment ranges by entry-point ascending.
 *
 * @param[in] a First range (struct sid_range *).
 * @param[in] b Second range (struct sid_range *).
 * @return <0 if @p a comes before @p b, 0 if equal, >0 if @p a comes after @p b.
 */
static int
compare_range(const void *a, const void *b)
{
    const struct sid_range *ra = a, *rb = b;

    if (ra->entry_point == rb->entry_point) {
        return 0;
    }
    return (ra->entry_point > rb->entry_point) ? 1 : -1;
}

LY_ERR
sid_file_update(const struct lys_module *module, const struct lyd_node *prev_sid_file,
        LYS_SID_FILE_STATUS status, const char *description, struct lyd_node **sid_file)
{
    LY_ERR rc = LY_SUCCESS;
    struct sid_range *ranges = NULL;
    struct sid_old_item *olds = NULL, *match = NULL;
    struct sid_item *items = NULL, *merged = NULL;
    struct sid_collect_data cb_data = {0};
    uint64_t *next_free = NULL;
    size_t ranges_count = 0, alloc_range_idx = 0;
    uint64_t old_count = 0, capacity = 0, merged_count = 0, extra = 0;
    uint32_t version = 0;
    ly_bool in_range = 0;
    struct lyd_node *tree = NULL;

    /* --- step 1: validation and extraction from the previous file --- */
    LY_CHECK_ERR_GOTO(!prev_sid_file->schema || strcmp(prev_sid_file->schema->name, "sid-file"),
            (LOGERR(module->ctx, LY_EINVAL,
                    "Invalid previous .sid file data tree (expected the \"sid-file\" top node)."),
            rc = LY_EINVAL), cleanup);

    LY_CHECK_GOTO((rc = sid_extract_file(module, prev_sid_file,
            &ranges, &ranges_count, &version, &olds, &old_count)), cleanup);

    /* assignment order: ranges ascending by entry-point */
    qsort(ranges, ranges_count, sizeof *ranges, compare_range);

    /* every old SID must lie inside some declared range; track the highest used SID per range so new items start right after it */
    next_free = calloc(ranges_count, sizeof *next_free);
    LY_CHECK_ERR_GOTO(!next_free, (LOGMEM(module->ctx), rc = LY_EMEM), cleanup);
    for (size_t j = 0; j < ranges_count; j++) {
        next_free[j] = ranges[j].entry_point;
    }
    for (uint64_t i = 0; i < old_count; i++) {
        in_range = 0;
        for (size_t j = 0; j < ranges_count; j++) {
            if ((olds[i].item.sid >= ranges[j].entry_point) && (olds[i].item.sid - ranges[j].entry_point < ranges[j].size)) {
                in_range = 1;
                if (olds[i].item.sid >= next_free[j]) {
                    next_free[j] = olds[i].item.sid + 1;
                }
                break;
            }
        }
        LY_CHECK_ERR_GOTO(!in_range,
                (LOGERR(module->ctx, LY_EINVAL,
                        "The previous .sid file assigns SID %" PRIu64 " (\"%s\"), which is "
                        "outside of any assignment range.", olds[i].item.sid, olds[i].item.ident),
                rc = LY_EINVAL), cleanup);
    }

    /* --- step 2: collect current schema items --- */
    for (size_t i = 0; i < ranges_count; i++) {
        capacity += ranges[i].size;
    }
    items = calloc(capacity, sizeof *items);
    LY_CHECK_ERR_GOTO(!items, (LOGMEM(module->ctx), rc = LY_EMEM), cleanup);

    cb_data.items = items;
    cb_data.max = capacity;
    cb_data.ctx = module->ctx;

    LY_CHECK_GOTO((rc = sid_collect_items(&cb_data, module)), cleanup);

    /* --- step 3: merge collected items with previous items --- */

    /* sort olds by (namespace, identifier) for binary lookup */
    qsort(olds, old_count, sizeof *olds, compare_old);

    /* merged array holds: matched items (old SID), new items (free SID), and obsolete items */
    merged = calloc(cb_data.count + old_count, sizeof *merged);
    LY_CHECK_ERR_GOTO(!merged, (LOGMEM(module->ctx), rc = LY_EMEM), cleanup);

    for (uint64_t i = 0; i < cb_data.count; i++) {
        match = sid_old_find(olds, old_count, cb_data.items[i].ns, cb_data.items[i].ident);
        if (match) {
            match->matched = 1;
            merged[merged_count].ns = cb_data.items[i].ns;
            merged[merged_count].ident = cb_data.items[i].ident;
            merged[merged_count].sid = match->item.sid;
            /* preserve old status; obsolete-matched is revived to stable (NULL) */
            merged[merged_count].status = (match->item.status && strcmp(match->item.status, "obsolete")) ?
                    match->item.status : NULL;
        } else {
            /* new item: first SID after the highest used one, walking ranges forward */
            for ( ;; ) {
                if (alloc_range_idx >= ranges_count) {
                    /* count the remaining items (this one included) that still need a new SID */
                    for (uint64_t j = i; j < cb_data.count; j++) {
                        if (!sid_old_find(olds, old_count, cb_data.items[j].ns, cb_data.items[j].ident)) {
                            extra++;
                        }
                    }
                    LOGERR(module->ctx, LY_EINVAL,
                            ".sid assignment range(s) size %" PRIu64 " is too small, %" PRIu64 " extra SID(s) are required.",
                            capacity, extra);
                    rc = LY_EINVAL;
                    goto cleanup;
                }
                if (next_free[alloc_range_idx] - ranges[alloc_range_idx].entry_point <
                        ranges[alloc_range_idx].size) {
                    break;
                }
                alloc_range_idx++;
            }
            merged[merged_count].ns = cb_data.items[i].ns;
            merged[merged_count].ident = cb_data.items[i].ident;
            merged[merged_count].sid = next_free[alloc_range_idx]++;
            merged[merged_count].status = (status == LYS_SID_FILE_UNPUBLISHED) ? "unstable" : NULL;
        }
        merged_count++;
    }

    /* unmatched old items are kept as obsolete (RFC 9595: MUST NOT be removed) */
    for (uint64_t i = 0; i < old_count; i++) {
        if (!olds[i].matched) {
            merged[merged_count] = olds[i].item;
            merged[merged_count].status = (char *)"obsolete";
            merged_count++;
        }
    }

    /* --- step 4: build the updated .sid file data tree --- */

    LY_CHECK_GOTO((rc = sid_build_skeleton(module, ranges, ranges_count, version + 1,
            status, description, &tree)), cleanup);

    /* published files do not contain unstable items (finalize) */
    for (uint64_t i = 0; i < merged_count; i++) {
        if (status == LYS_SID_FILE_PUBLISHED) {
            if (merged[i].status && !strcmp(merged[i].status, "unstable")) {
                merged[i].status = NULL;
            }
        }

        LY_CHECK_GOTO((rc = sid_emit_item(&merged[i], tree)), cleanup);
    }

    *sid_file = tree;
    tree = NULL;

cleanup:
    lyd_free_all(tree);
    free(merged);
    free(next_free);
    free(ranges);
    sid_old_items_free(&olds, old_count);
    sid_collect_data_free(&cb_data);
    return rc;
}
