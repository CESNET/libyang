/**
 * @file openconfig.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - OpenConfig extensions
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
 * @brief Parse 'regexp-posix' extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
regexp_posix_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_ext_instance *exts;

    /* check that the extension is instantiated at an allowed place - top-level */
    if ((ext->parent_stmt != LY_STMT_MODULE) && (ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension \"%s\" is allowed only in 'module' or 'submodule' "
                "statements, but it is placed in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EINVAL;
    }

    exts = ((struct lysp_module *)ext->parent)->exts;

    /* check for duplication */
    LY_ARRAY_FOR(exts, u) {
        if ((&exts[u] != ext) && (exts[u].name == ext->name)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension \"%s\" is instantiated multiple times.", ext->name);
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse 'posix-pattern' extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
posix_pattern_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    struct lysp_type *type;

    /* check the argument exists, will be compiled during compilation */
    if (!ext->argument) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension \"%s\" requires an argument.", ext->name);
        return LY_EINVAL;
    }

    /* check that the extension is instantiated at an allowed place - type string */
    if (ext->parent_stmt != LY_STMT_TYPE) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension \"%s\" is allowed only in a string 'type' statement, "
                "but it is placed in a \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EINVAL;
    }

    type = (struct lysp_type *)ext->parent;
    if (strcmp(type->name, "string") && !strchr(type->name, ':')) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension \"%s\" is allowed only in a string 'type' statement or "
                "derived from it, but it is placed in a \"%s\" type.", ext->name, type->name);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile 'posix-pattern' extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
posix_pattern_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *UNUSED(extp), struct lysc_ext_instance *ext)
{
    LY_ERR r;
    struct ly_err_item *err = NULL;

    r = ly_pat_compile(ext->argument, 1, &ext->compiled, &err);
    if (r) {
        lyplg_ext_compile_log(cctx, ext, err->level, err->err, "%s", err->msg);
        ly_err_free(err);
        return r;
    }

    return LY_SUCCESS;
}

/**
 * @brief Validate data with 'posix-pattern' extension instances.
 *
 * Implementation of ::lyplg_ext_data_validate_clb callback set as lyext_plugin::validate.
 */
static LY_ERR
posix_pattern_data_validate(struct lysc_ext_instance *ext, struct lyd_node *sibling,
        const struct lyd_node *UNUSED(dep_tree), enum lyd_type UNUSED(data_type), uint32_t UNUSED(val_opts),
        struct lyd_node **UNUSED(diff))
{
    LY_ERR r;
    const char *val_str;
    struct ly_err_item *err = NULL;
    char *path;

    assert(!(sibling->flags & LYD_EXT));
    assert(sibling->schema->nodetype & LYD_NODE_TERM);

    /* match the string */
    val_str = lyd_get_value(sibling);
    r = ly_pat_match(ext->compiled, ext->argument, 1, val_str, strlen(val_str), &err);
    if (r) {
        path = lyd_path(sibling, LYD_PATH_STD, NULL, 0);
        lyplg_ext_compile_log_path(path, ext, err->level, err->err, "%s", err->msg);
        free(path);
        ly_err_free(err);
        return (r == LY_ENOT) ? LY_EVALID : r;
    }

    return LY_SUCCESS;
}

/**
 * @brief Free compiled 'posix-pattern' extension instances.
 *
 * Implementation of ::lyplg_ext_compile_free_clb callback set as lyext_plugin::cfree.
 */
static void
posix_pattern_compile_free(const struct ly_ctx *UNUSED(ctx), struct lysc_ext_instance *ext)
{
    ly_pat_free(ext->compiled, 1);
}

/**
 * @brief Plugin descriptions for the OpenConfig extensions
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_openconfig[] = {
    {
        .module = "openconfig-extensions",
        .revision = NULL,
        .name = "regexp-posix",

        .plugin.id = "ly2 OpenConfig",
        .plugin.parse = regexp_posix_parse,
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
        .module = "openconfig-extensions",
        .revision = NULL,
        .name = "posix-pattern",

        .plugin.id = "ly2 OpenConfig",
        .plugin.parse = posix_pattern_parse,
        .plugin.compile = posix_pattern_compile,
        .plugin.printer_info = NULL,
        .plugin.node_xpath = NULL,
        .plugin.snode_xpath = NULL,
        .plugin.snode = NULL,
        .plugin.validate = posix_pattern_data_validate,
        .plugin.pfree = NULL,
        .plugin.cfree = posix_pattern_compile_free,
        .plugin.compiled_size = NULL,
        .plugin.compiled_print = NULL
    },
    {0} /* terminating zeroed item */
};
