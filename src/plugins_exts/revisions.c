/**
 * @file revisions.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - ietf-yang-revisions
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "libyang.h"
#include "ly_common.h"
#include "plugins_exts.h"
#include "plugins_internal.h"
#include "tree_schema_internal.h"

/**
 * @brief Parse ietf-yang-revisions non-backwards-compatible extension.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
nbc_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    struct lysp_revision *rev;
    LY_ARRAY_COUNT_TYPE u;

    /* check that the extension is instantiated at an allowed place - revision */
    if (ext->parent_stmt != LY_STMT_REVISION) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is allowed only in a \"revision\" statement, "
                "but it is placed in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* check argument */
    if (ext->argument) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s requires no argument but \"%s\" used.",
                ext->name, ext->argument);
        return LY_EVALID;
    }

    /* check for duplication */
    rev = ext->parent;
    LY_ARRAY_FOR(rev->exts, u) {
        if ((&rev->exts[u] != ext) && (rev->exts[u].name == ext->name)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse ietf-yang-revisions recommended-min-date extension.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
min_date_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    struct lysp_import *imp;
    LY_ARRAY_COUNT_TYPE u;

    /* check that the extension is instantiated at an allowed place - import */
    if (ext->parent_stmt != LY_STMT_IMPORT) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is allowed only in an \"import\" statement, "
                "but it is placed in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* check argument */
    assert(ext->argument);
    if (lys_check_date(NULL, ext->argument, strlen(ext->argument), NULL)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s argument revision-date \"%s\" invalid.",
                ext->name, ext->argument);
        return LY_EVALID;
    }

    /* check for duplication */
    imp = ext->parent;
    LY_ARRAY_FOR(imp->exts, u) {
        if ((&imp->exts[u] != ext) && (imp->exts[u].name == ext->name)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Plugin descriptions for the ietf-yang-revisions non-backwards-compatible and recommended-min-date extensions
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_revisions[] = {
    {
        .module = "ietf-yang-revisions",
        .revision = NULL,
        .name = "non-backwards-compatible",

        .plugin.id = "ly2 revisions",
        .plugin.parse = nbc_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.node_xpath = NULL,
        .plugin.snode_xpath = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    }, {
        .module = "ietf-yang-revisions",
        .revision = NULL,
        .name = "recommended-min-date",

        .plugin.id = "ly2 revisions",
        .plugin.parse = min_date_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.node_xpath = NULL,
        .plugin.snode_xpath = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    },
    {0} /* terminating zeroed item */
};
