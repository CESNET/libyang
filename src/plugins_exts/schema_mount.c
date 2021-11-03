/**
 * @file schema_mount.c
 * @author Tadeas Vintrlik <xvintr04@stud.fit.vutbr.cz>
 * @brief libyang extension plugin - Schema Mount (RFC 8528)
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"
#include "libyang.h"
#include "log.h"
#include "plugins_exts.h"
#include "tree_data.h"
#include "tree_schema.h"

/**
 * @brief Check if given mount point is unique among its' siblings
 *
 * @param cctx Compilation context.
 * @param c_ext Compiled extension instance for checking uniqueness.
 * @param p_ext Extension instance of the mount-point for comparison.
 *
 * @return LY_SUCCESS if is unique. LY_EINVAL otherwise.
 */
static LY_ERR
schema_mount_unique_mount_point(struct lysc_ctx *cctx, const struct lysc_ext_instance *c_ext,
        const struct lysp_ext_instance *p_ext)
{
    struct lysp_module *pmod;
    struct lysp_ext_instance *exts;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysp_node *parent;
    struct lysp_import *module;
    char *ext_prefix, *ext_name;

    /* Check if it is the only instance of the mount-point among its' siblings */
    parent = (struct lysp_node *) c_ext->parent;
    exts = parent->exts;
    pmod = lysc_ctx_get_pmod(cctx);
    LY_ARRAY_FOR(exts, u) {
        /* Extract prefix and name of the extension */
        ext_prefix = strdup(exts[u].name);
        ext_name = strstr(exts[u].name, ":");
        ext_name++;
        ext_prefix[strstr(ext_prefix, ":") - ext_prefix] = '\0';

        module = NULL;
        LY_ARRAY_FOR(pmod->imports, v) {
            if (!strcmp(pmod->imports[v].prefix, ext_prefix)) {
                /* Found the matching module */
                module = &pmod->imports[v];
                break;
            }
        }
        free(ext_prefix);
        if ((&exts[u] != p_ext) && module && (!strcmp(module->name, "ietf-yang-schema-mount")) &&
                (!strcmp(exts[u].name, "mount-point"))) {
            /* Found another instance of mount-point only one allowed per node */
            return LY_EINVAL;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Schema mount compile.
 *
 * Checks if it can be a valid extension instance for yang schema mount.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
schema_mount_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext,
        struct lysc_ext_instance *c_ext)
{
    const struct lys_module *cur_mod;

    /* Check if processing right callback */
    assert(!strcmp(p_ext->name, "yangmnt:mount-point"));

    /* Check if mount point was found in YANG version 1.1 module */
    cur_mod = lysc_ctx_get_cur_mod(cctx);
    if (cur_mod->parsed->version != LYS_VERSION_1_1) {
        return LY_EINVAL;
    }

    /* Check if its' parent is a container or a list */
    if ((p_ext->parent_stmt != LY_STMT_CONTAINER) && (p_ext->parent_stmt != LY_STMT_LIST)) {
        return LY_EINVAL;
    }

    /* Check if the only mount-point among siblings */
    if (schema_mount_unique_mount_point(cctx, c_ext, p_ext)) {
        return LY_EINVAL;
    }

    (void)c_ext;

    return LY_SUCCESS;
}

/**
 * @brief Parse callback for schema mount.
 *
 * Check if data is valid for schema mount and inserts it to the parent.
 */
static LY_ERR
schema_mount_parse(struct ly_in *in, LYD_FORMAT format, struct lysc_ext_instance *ext,
        struct lyd_node *parent, uint32_t parse_opts, uint32_t val_opts)
{
    LY_ERR ret = LY_SUCCESS;
    const struct ly_ctx *ctx;
    struct lyd_node *subtree, *yanglib, *mount_point, *final = NULL;
    struct ly_err_item *err;
    ly_bool found_yanglib = 0, found_mount_point = 0;
    uint32_t old_log_opts;

    ctx = LYD_CTX(parent);

    old_log_opts = ly_log_options(LY_LOSTORE_LAST);
    /* Check if intended for schema-mount - had both required data nodes */
    while (1) {
        /* Parse by sub-trees */
        if (lyd_parse_data(ctx, NULL, in, 0, parse_opts, val_opts, &subtree)) {
            /* Either end or error - must check */
            err = ly_err_first(ctx);
            if (err->vecode == LYVE_SYNTAX_XML) {
                /* Could just be EOF - check */
                /* TODO: Search in error message  if EOF then break */
                lyd_insert_sibling(final, subtree, NULL);
                break;
            } else {
                /* Other parsing error encountered */
                ret = LY_EINVAL;
                goto cleanup;
            }
        }

        if (!final) {
            /* If there was nothing inserted yet this subtree becomes the one to insert into */
            final = subtree;
        }

        lyd_find_path(subtree, "/ietf-yang-library:yang-library", 0, &yanglib);
        if (yanglib && !(yanglib->flags & LYD_DEFAULT)) {
            /* Found and not created by flags */
            found_yanglib = 1;
            lyd_insert_sibling(final, yanglib, NULL);
            continue;
        }
        lyd_find_path(subtree, "/ietf-yang-schema-mount:mount-points", 0, &mount_point);
        if (mount_point && !(mount_point->flags & LYD_DEFAULT)) {
            /* Was found and not created by flags */
            found_mount_point = 1;
            lyd_insert_sibling(final, mount_point, NULL);
            continue;
        }
    }

    if (found_mount_point && found_yanglib) {
        /* It is valid data and can be inserted into the parent */
        lyd_insert_child(parent, final);
    } else {
        /* It was not data for schema mount */
        lyd_free_tree(final);
        ret = LY_ENOT;
    }

cleanup:
    ly_log_options(old_log_opts);
    return ret;
}

/**
 * @brief Plugin descriptions for the Yang Schema Mount extension.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_schema_mount[] = {
    {
        .module = "ietf-yang-schema-mount",
        .revision = "2019-01-14",
        .name = "mount-point",

        .plugin.id = "libyang 2 - Schema Mount, version 1",
        .plugin.compile = &schema_mount_compile,
        .plugin.parse = &schema_mount_parse,
        .plugin.validate = NULL,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    },
    {0} /* terminating zeroed item */
};
