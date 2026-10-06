/**
 * @file schema_comparison.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - schema-comparison extensions
 *
 * Copyright (c) 2025 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "ly_array.h"
#include "parser_internal.h"
#include "plugins_exts.h"
#include "plugins_internal.h"
#include "tree_schema_internal.h"

/**
 * @brief Parse backwards-compatible extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
schema_cmp_change_at_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    const struct lysp_ext_instance *exts = NULL;
    LYA_COUNT_T u;
    struct lyplg_ext *ext_plugin, *parent_ext_plugin;

    /* check that the extension is instantiated at an allowed place */
    switch (ext->parent_stmt) {
    case LY_STMT_PATTERN:
    case LY_STMT_MUST:
        exts = ((struct lysp_restr *)ext->parent)->exts;
        break;
    case LY_STMT_WHEN:
        exts = ((struct lysp_when *)ext->parent)->exts;
        break;
    case LY_STMT_DESCRIPTION:
    case LY_STMT_REFERENCE:
    case LY_STMT_PRESENCE:
        /* meh */
        break;
    case LY_STMT_EXTENSION_INSTANCE:
        exts = ((struct lysp_ext_instance *)ext->parent)->exts;
        break;
    default:
        lyplg_ext_parse_log(pctx, ext, LY_LLWRN, 0, "Extension %s is not allowed in a \"%s\" statement.", ext->name,
                lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_ENOT;
    }

    /* check argument */
    if (lyplg_ext_semver_parse(PARSER_CTX(pctx), ext->argument, 0, 0, NULL)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s argument semver \"%s\" invalid.", ext->name,
                ext->argument);
        return LY_EVALID;
    }

    ext_plugin = LYSC_GET_EXT_PLG(ext->plugin_ref);

    /* check for duplication */
    LYA_FOR(exts, u) {
        parent_ext_plugin = LYSC_GET_EXT_PLG(exts[u].plugin_ref);
        if ((&exts[u] != ext) && parent_ext_plugin && !strcmp(parent_ext_plugin->id, ext_plugin->id) &&
                !strcmp(exts[u].argument, ext->argument)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s semver \"%s\" is a duplicate of %s semver \"%s\".",
                    ext->name, ext->argument, exts[u].name, exts[u].argument);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Plugin descriptions for the schema-comparison extensions
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_schema_cmp[] = {
    {
        .module = "ietf-yang-schema-comparison",
        .revision = NULL,
        .name = "ed-change-at",

        .plugin.id = "ly2 schema-comparison",
        .plugin.parse = schema_cmp_change_at_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.node_xpath = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    },
    {
        .module = "ietf-yang-schema-comparison",
        .revision = NULL,
        .name = "bc-change-at",

        .plugin.id = "ly2 schema-comparison",
        .plugin.parse = schema_cmp_change_at_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.node_xpath = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    },
    {
        .module = "ietf-yang-schema-comparison",
        .revision = NULL,
        .name = "nbc-change-at",

        .plugin.id = "ly2 schema-comparison",
        .plugin.parse = schema_cmp_change_at_parse,
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
