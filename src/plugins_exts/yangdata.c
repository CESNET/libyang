/**
 * @file yangdata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - yang-data (RFC 8040)
 *
 * Copyright (c) 2021 - 2026 CESNET, z.s.p.o.
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
#include "plugins_types.h"

static LY_ERR yangdata_snode(struct lysc_ext_instance *ext, const struct lyd_node *parent, const struct lysc_node *sparent,
        const char *prefix, uint32_t prefix_len, LY_VALUE_FORMAT format, void *prefix_data, const char *name,
        uint32_t name_len, const struct lysc_node **snode);
static void yangdata_cfree(const struct ly_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Parse yang-data extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
yangdata_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_module *pmod;

    /* yang-data can appear only at the top level of a YANG module or submodule */
    if ((ext->parent_stmt != LY_STMT_MODULE) && (ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLWRN, 0, "Extension %s is ignored since it appears as a non top-level statement "
                "in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_ENOT;
    }

    pmod = ext->parent;

    /* check for duplication */
    LY_ARRAY_FOR(pmod->exts, u) {
        if ((&pmod->exts[u] != ext) && (pmod->exts[u].name == ext->name) && !strcmp(pmod->exts[u].argument, ext->argument)) {
            /* duplication of the same yang-data extension in a single module */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    /* parse yang-data substatements */
    LY_ARRAY_CREATE_GOTO(lyplg_ext_parse_get_cur_pmod(pctx)->mod->ctx, ext->substmts, 3, ret, emem);
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_CONTAINER;
    ext->substmts[0].storage_p = (void **)&ext->parsed;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_CHOICE;
    ext->substmts[1].storage_p = (void **)&ext->parsed;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_USES;
    ext->substmts[2].storage_p = (void **)&ext->parsed;

    if ((ret = lyplg_ext_parse_extension_instance(pctx, ext))) {
        return ret;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief Compile yang-data extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
yangdata_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *extp, struct lysc_ext_instance *ext)
{
    LY_ERR ret;
    const struct lysc_node *child;
    ly_bool valid = 1;
    uint32_t prev_options = *lyplg_ext_compile_get_options(cctx);

    /* compile yangg-data substatements */
    LY_ARRAY_CREATE_GOTO(cctx->ctx, ext->substmts, 3, ret, emem);
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_CONTAINER;
    ext->substmts[0].storage_p = (void **)&ext->compiled;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_CHOICE;
    ext->substmts[1].storage_p = (void **)&ext->compiled;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_USES;
    ext->substmts[2].storage_p = (void **)&ext->compiled;

    *lyplg_ext_compile_get_options(cctx) |= LYS_COMPILE_NO_CONFIG | LYS_COMPILE_NO_DISABLED;
    ret = lyplg_ext_compile_extension_instance(cctx, extp, ext, NULL);
    *lyplg_ext_compile_get_options(cctx) = prev_options;
    if (ret) {
        return ret;
    }

    /* check that we have really just a single container data definition in the top */
    child = ext->compiled;
    if (!child) {
        valid = 0;
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is instantiated without any top level data node, but exactly one container data node is expected.",
                extp->name);
    } else if (child->next) {
        valid = 0;
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is instantiated with multiple top level data nodes, but only a single container data node is allowed.",
                extp->name);
    } else if (child->nodetype == LYS_CHOICE) {
        /* all the choice's case are expected to result to a single container node */
        struct lysc_module *mod_c = ext->parent;
        const struct lysc_node *snode = NULL;

        while ((snode = lys_getnext(snode, child, mod_c, 0))) {
            if (snode->next) {
                valid = 0;
                lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                        "Extension %s is instantiated with multiple top level data nodes (inside a single choice's case), "
                        "but only a single container data node is allowed.", extp->name);
                break;
            } else if (snode->nodetype != LYS_CONTAINER) {
                valid = 0;
                lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                        "Extension %s is instantiated with %s top level data node (inside a choice), "
                        "but only a single container data node is allowed.", extp->name, lys_nodetype2str(snode->nodetype));
                break;
            }
        }
    } else if (child->nodetype != LYS_CONTAINER) {
        /* via uses */
        valid = 0;
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is instantiated with %s top level data node, but only a single container data node is allowed.",
                extp->name, lys_nodetype2str(child->nodetype));
    }

    if (!valid) {
        yangdata_cfree(lyplg_ext_compile_get_ctx(cctx), ext);
        ext->compiled = ext->substmts = NULL;
        return LY_EVALID;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief INFO printer
 *
 * Implementation of ::lyplg_ext_sprinter_info_clb set as ::lyext_plugin::printer_info
 */
static LY_ERR
yangdata_printer_info(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag)
{
    lyplg_ext_print_info_extension_instance(ctx, ext, flag);
    return LY_SUCCESS;
}

/**
 * @brief Snode xpath callback for yang-data.
 */
static LY_ERR
yangdata_snode_xpath(struct lysc_ext_instance *ext, const char *prefix, uint32_t prefix_len, LY_VALUE_FORMAT format,
        void *prefix_data, const char *name, uint32_t name_len, const struct lysc_node **snode)
{
    return yangdata_snode(ext, NULL, NULL, prefix, prefix_len, format, prefix_data, name, name_len, snode);
}

/**
 * @brief Snode callback for yang-data.
 */
static LY_ERR
yangdata_snode(struct lysc_ext_instance *ext, const struct lyd_node *parent, const struct lysc_node *sparent,
        const char *prefix, uint32_t prefix_len, LY_VALUE_FORMAT format, void *prefix_data, const char *name,
        uint32_t name_len, const struct lysc_node **snode)
{
    const struct lysc_node *schema = ext->compiled;
    const struct lys_module *mod;

    if (parent || sparent) {
        /* matches only top-level data */
        return LY_ENOT;
    }

    if (prefix && prefix_len) {
        /* check module */
        mod = lys_find_module(ext->module->ctx, NULL, prefix, prefix_len, format, prefix_data);
        if (!mod || (ext->module != mod)) {
            return LY_ENOT;
        }
    }

    /* check name */
    if (name && name_len) {
        LY_LIST_FOR(schema, schema) {
            if (!ly_strncmp(schema->name, name, name_len)) {
                break;
            }
        }
    }

    /* first top-level node or the found one, if any */
    *snode = schema;
    return schema ? LY_SUCCESS : LY_ENOT;
}

/**
 * @brief Validate callback for yang-data.
 */
static LY_ERR
yangdata_validate(struct lysc_ext_instance *ext, struct lyd_node *node, const struct lyd_node *UNUSED(dep_tree),
        enum lyd_type data_type, uint32_t val_opts, struct lyd_node **diff)
{
    if (data_type != LYD_TYPE_DATA_YANG) {
        /* not supported */
        return LY_ENOT;
    }

    /* validate all the modules with data */
    return lyd_validate_ext(&node, ext, val_opts, diff);
}

/**
 * @brief Free parsed yang-data extension instance data.
 *
 * Implementation of ::lyplg_clb_parse_free_clb callback set as lyext_plugin::pfree.
 */
static void
yangdata_pfree(const struct ly_ctx *ctx, struct lysp_ext_instance *ext)
{
    lyplg_ext_pfree_instance_substatements(ctx, ext->substmts);
}

/**
 * @brief Free compiled yang-data extension instance data.
 *
 * Implementation of ::lyplg_clb_compile_free_clb callback set as lyext_plugin::cfree.
 */
static void
yangdata_cfree(const struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    lyplg_ext_cfree_instance_substatements(ctx, ext->substmts);
}

static int
yandgata_compiled_size(const struct lysc_ext_instance *ext, struct ly_ht *addr_ht)
{
    return lyplg_ext_compiled_stmts_storage_size(ext->substmts, addr_ht);
}

static LY_ERR
yangdata_compiled_print(const struct lysc_ext_instance *orig_ext, struct lysc_ext_instance *ext, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    ext->compiled = NULL;
    ext->substmts[0].storage_p = (void **)&ext->compiled;
    ext->substmts[1].storage_p = (void **)&ext->compiled;
    ext->substmts[2].storage_p = (void **)&ext->compiled;

    return lyplg_ext_compiled_stmts_storage_print(orig_ext->substmts, ext->substmts, addr_ht, ptr_set, mem);
}

/**
 * @brief Plugin descriptions for the yang-data extension
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_yangdata[] = {
    {
        .module = "ietf-restconf",
        .revision = "2017-01-26",
        .name = "yang-data",

        .plugin.id = "ly2 yang-data",
        .plugin.parse = yangdata_parse,
        .plugin.compile = yangdata_compile,
        .plugin.printer_info = yangdata_printer_info,
        .plugin.node_xpath = NULL,
        .plugin.snode_xpath = yangdata_snode_xpath,
        .plugin.snode = yangdata_snode,
        .plugin.validate = yangdata_validate,
        .plugin.pfree = yangdata_pfree,
        .plugin.cfree = yangdata_cfree,
        .plugin.compiled_size = yandgata_compiled_size,
        .plugin.compiled_print = yangdata_compiled_print
    },
    {0}     /* terminating zeroed record */
};
