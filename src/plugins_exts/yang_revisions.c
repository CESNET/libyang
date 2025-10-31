/**
 * @file ang_revisions.c
 * @author Joe Clarke <jclarke@cisco.com>
 * @brief libyang extension plugin - YANG Revisions (ietf-yang-revisions)
 *
 * Copyright (c) 2025 Cisco Systems, Inc.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "libyang.h"
#include "plugins_exts.h"

/**
 * @brief Parse non-backwards-compatible extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
non_backwards_compatible_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    struct lysp_revision *parent = NULL;
    LY_ARRAY_COUNT_TYPE u;

    /* Check that the extension is instantiated at an allowed place - revision statement */
    if (ext->parent_stmt != LY_STMT_REVISION) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is allowed only in a revision statement, but it is placed in \"%s\" statement.",
                ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* This extension should not have an argument */
    if (ext->argument && *ext->argument) {
        lyplg_ext_parse_log(pctx, ext, LY_LLWRN, 0,
                "Extension %s should not have an argument, but has: %s", ext->name, ext->argument);
    }

    parent = ext->parent;

    /* Check for duplication within the same revision */
    LY_ARRAY_FOR(parent->exts, u) {
        if ((&parent->exts[u] != ext) && parent->exts[u].record &&
                !strcmp(parent->exts[u].record->plugin.id, ext->record->plugin.id)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Extension %s is instantiated multiple times in the same revision.", ext->name);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Validate revision-date format (YYYY-MM-DD).
 *
 * @param[in] date Date string to validate.
 * @return LY_SUCCESS if valid, LY_EVALID otherwise.
 */
static LY_ERR
revision_date_validate(const char *date)
{
    int year, month, day;

    if (!date || (strlen(date) != 10)) {
        return LY_EVALID;
    }

    if ((date[4] != '-') || (date[7] != '-')) {
        return LY_EVALID;
    }

    if (sscanf(date, "%4d-%2d-%2d", &year, &month, &day) != 3) {
        return LY_EVALID;
    }

    if ((year < 1000) || (year > 9999) || (month < 1) || (month > 12) || (day < 1) || (day > 31)) {
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse recommended-min-date extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
recommended_min_date_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    struct lysp_import *parent = NULL;
    LY_ARRAY_COUNT_TYPE u;

    /* Check that the extension is instantiated at an allowed place - import statement */
    if (ext->parent_stmt != LY_STMT_IMPORT) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is allowed only in an import statement, but it is placed in \"%s\" statement.",
                ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* Validate the argument format - must be revision-date (YYYY-MM-DD) */
    if (!ext->argument) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s requires a revision-date argument (YYYY-MM-DD).", ext->name);
        return LY_EVALID;
    }

    if (revision_date_validate(ext->argument) != LY_SUCCESS) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s has invalid revision-date format: %s (expected YYYY-MM-DD and a valid date)",
                ext->name, ext->argument);
        return LY_EVALID;
    }

    parent = ext->parent;

    /* Check for duplication within the same import */
    LY_ARRAY_FOR(parent->exts, u) {
        if ((&parent->exts[u] != ext) && parent->exts[u].record &&
                !strcmp(parent->exts[u].record->plugin.id, ext->record->plugin.id)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                    "Extension %s is instantiated multiple times in the same import.", ext->name);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Plugin descriptions for the ietf-yang-revisions extensions
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_yang_revisions[] = {
    {
        .module = "ietf-yang-revisions",
        .revision = "2025-01-28",
        .name = "non-backwards-compatible",

        .plugin.id = "ly2 ietf-yang-revisions v1",
        .plugin.parse = non_backwards_compatible_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = NULL,
        .plugin.printer_ptree = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
    },
    {
        .module = "ietf-yang-revisions",
        .revision = "2025-01-28",
        .name = "recommended-min-date",

        .plugin.id = "ly2 ietf-yang-revisions v1",
        .plugin.parse = recommended_min_date_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = NULL,
        .plugin.printer_ptree = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
    },
    {0} /* terminating zeroed record */
};
