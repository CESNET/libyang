/**
 * @file yang_semver.c
 * @author Joe Clarke <jclarke@cisco.com>
 * @brief libyang extension plugin - YANG Semantic Versioning (ietf-yang-semver)
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
 * @brief Validate semantic version format (MAJOR.MINOR.PATCH with optional modifiers).
 *
 * @param[in] version Version string to validate.
 * @param[in] allow_modifiers Whether to allow _compatible/_non_compatible and metadata.
 * @return LY_SUCCESS if valid, LY_EVALID otherwise.
 */
static LY_ERR
semver_validate_format(const char *version, ly_bool allow_modifiers)
{
    const char *ptr = version;
    int dots = 0;
    int digits = 0;

    if (!version || !*version) {
        return LY_EVALID;
    }

    /* Parse MAJOR.MINOR.PATCH */
    while (*ptr) {
        if (isdigit(*ptr)) {
            digits++;
            ptr++;
        } else if (*ptr == '.') {
            if (digits == 0 || dots >= 2) {
                return LY_EVALID;
            }
            dots++;
            digits = 0;
            ptr++;
        } else {
            /* Must be at modifier or metadata */
            break;
        }
    }

    /* Must have exactly 2 dots and digits after last dot */
    if (dots != 2 || digits == 0) {
        return LY_EVALID;
    }

    /* For recommended-min-version, no modifiers allowed */
    if (!allow_modifiers && *ptr != '\0') {
        return LY_EVALID;
    }

    /* Optional: check for _compatible or _non_compatible */
    if (*ptr == '_') {
        if (strncmp(ptr, "_compatible", 11) == 0) {
            ptr += 11;
        } else if (strncmp(ptr, "_non_compatible", 15) == 0) {
            ptr += 15;
        } else {
            return LY_EVALID;
        }
    }

    /* Optional: check for pre-release version (dash followed by alphanumeric and dots/dashes) */
    if (*ptr == '-') {
        ptr++;
        if (!isalnum(*ptr)) {
            return LY_EVALID;
        }
        while (*ptr && *ptr != '+') {
            if (!isalnum(*ptr) && *ptr != '.' && *ptr != '-') {
                return LY_EVALID;
            }
            ptr++;
        }
    }

    /* Optional: check for metadata (plus followed by alphanumeric and dots/dashes) */
    if (*ptr == '+') {
        ptr++;
        if (!isalnum(*ptr)) {
            return LY_EVALID;
        }
        while (*ptr) {
            if (!isalnum(*ptr) && *ptr != '.' && *ptr != '-') {
                return LY_EVALID;
            }
            ptr++;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse version extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
version_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    const struct lysp_module *pmod;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysp_revision *rev;
    const char *version_arg = ext->argument;

    /* Check that the extension is instantiated at an allowed place - revision statement */
    if (ext->parent_stmt != LY_STMT_REVISION) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is allowed only in a revision statement, but it is placed in \"%s\" statement.",
                ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* Validate the argument format */
    if (!ext->argument) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s requires a YANG Semver argument.", ext->name);
        return LY_EVALID;
    }

    if (semver_validate_format(ext->argument, 1) != LY_SUCCESS) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s has invalid YANG Semver version format: %s", ext->name, ext->argument);
        return LY_EVALID;
    }

    /* Get the module to check all revisions */
    pmod = lyplg_ext_parse_get_cur_pmod(pctx);
    if (!pmod || !pmod->revs) {
        return LY_SUCCESS;
    }

    /* Check that this version is unique across all revisions */
    LY_ARRAY_FOR(pmod->revs, u) {
        rev = &pmod->revs[u];
        if (!rev->exts) {
            continue;
        }
        LY_ARRAY_FOR(rev->exts, v) {
            /* Skip if this is the current extension instance */
            if (&rev->exts[v] == ext) {
                continue;
            }

            /* Check if this is also a version extension by name and module */
            if (rev->exts[v].name && !strcmp(rev->exts[v].name, ext->name) &&
                    rev->exts[v].parent_stmt == LY_STMT_REVISION) {
                /* Check for duplicate in same revision */
                if (rev->exts[v].parent == ext->parent) {
                    lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                            "Extension %s is instantiated multiple times in the same revision.", ext->name);
                    return LY_EVALID;
                }
                /* Compare versions across different revisions */
                if (rev->exts[v].argument && !strcmp(rev->exts[v].argument, version_arg)) {
                    lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                            "Version %s is not unique - it appears in multiple revisions.", version_arg);
                    return LY_EVALID;
                }
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse recommended-min-version extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
recommended_min_version_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
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

    /* Validate the argument format - must be MAJOR.MINOR.PATCH only (no modifiers) */
    if (!ext->argument) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s requires a MAJOR.MINOR.PATCH version argument.", ext->name);
        return LY_EVALID;
    }

    if (semver_validate_format(ext->argument, 0) != LY_SUCCESS) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s argument must be MAJOR.MINOR.PATCH format (no modifiers): %s",
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
 * @brief Plugin descriptions for the ietf-yang-semver extensions
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_yang_semver[] = {
    {
        .module = "ietf-yang-semver",
        .revision = "2025-09-29",
        .name = "version",

        .plugin.id = "ly2 ietf-yang-semver v1",
        .plugin.parse = version_parse,
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
        .module = "ietf-yang-semver",
        .revision = "2025-09-29",
        .name = "recommended-min-version",

        .plugin.id = "ly2 ietf-yang-semver v1",
        .plugin.parse = recommended_min_version_parse,
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
