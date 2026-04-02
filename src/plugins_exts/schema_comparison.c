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
#include "libyang.h"
#include "ly_common.h"
#include "plugins_exts.h"
#include "plugins_internal.h"

/**
 * @brief Parse backwards-compatible extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
schema_cmp_change_at_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    /* check that the extension is instantiated at an allowed place */
    if ((ext->parent_stmt != LY_STMT_PATTERN) && (ext->parent_stmt != LY_STMT_WHEN) &&
            (ext->parent_stmt != LY_STMT_MUST) && (ext->parent_stmt != LY_STMT_DESCRIPTION) &&
            (ext->parent_stmt != LY_STMT_PRESENCE) && (ext->parent_stmt != LY_STMT_EXTENSION_INSTANCE)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLWRN, 0, "Extension %s is not allowed in \"%s\" statement.", ext->name,
                lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_ENOT;
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
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    },
    {0} /* terminating zeroed item */
};
