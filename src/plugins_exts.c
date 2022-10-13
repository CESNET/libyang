/**
 * @file plugins_exts.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief helper functions for extension plugins
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_exts.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "dict.h"
#include "parser_internal.h"
#include "printer_internal.h"
#include "schema_compile.h"
#include "schema_compile_amend.h"
#include "schema_compile_node.h"
#include "schema_features.h"
#include "tree_schema_internal.h"

LIBYANG_API_DEF const struct lysp_module *
lyplg_ext_parse_get_cur_pmod(const struct lysp_ctx *pctx)
{
    return PARSER_CUR_PMOD(pctx);
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_parse_extension_instance(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_stmt *stmt;

    /* check for invalid substatements */
    LY_LIST_FOR(ext->child, stmt) {
        if (stmt->flags & (LYS_YIN_ATTR | LYS_YIN_ARGUMENT)) {
            continue;
        }
        LY_ARRAY_FOR(ext->substmts, u) {
            if (ext->substmts[u].stmt == stmt->kw) {
                break;
            }
        }
        if (u == LY_ARRAY_COUNT(ext->substmts)) {
            LOGVAL(PARSER_CTX(pctx), LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s%s%s\" extension instance.",
                    stmt->stmt, ext->name, ext->argument ? " " : "", ext->argument ? ext->argument : "");
            rc = LY_EVALID;
            goto cleanup;
        }
    }

    /* parse all the known statements */
    LY_ARRAY_FOR(ext->substmts, u) {
        LY_LIST_FOR(ext->child, stmt) {
            if (ext->substmts[u].stmt != stmt->kw) {
                continue;
            }

            if ((rc = lys_parse_ext_instance_stmt(pctx, &ext->substmts[u], stmt))) {
                goto cleanup;
            }
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Compile an instance extension statement.
 *
 * @param[in] ctx Compile context.
 * @param[in] parsed Parsed ext instance substatement structure.
 * @param[in] ext Compiled ext instance.
 * @param[in] substmt Compled ext instance substatement info.
 * @param[in,out] aug_target Optional augment target where to append all schema data nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_ext_instance_stmt(struct lysc_ctx *ctx, const void *parsed, struct lysc_ext_instance *ext,
        struct lysc_ext_substmt *substmt, struct lysc_node *aug_target)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool length_restr = 0;
    LY_DATA_TYPE basetype;

    /* compilation wthout any storage */
    if (substmt->stmt == LY_STMT_IF_FEATURE) {
        ly_bool enabled;

        /* evaluate */
        LY_CHECK_GOTO(rc = lys_eval_iffeatures(ctx->ctx, parsed, &enabled), cleanup);
        if (!enabled) {
            /* it is disabled, remove the whole extension instance */
            rc = LY_ENOT;
        }
    }

    if (!substmt->storage || !parsed) {
        /* nothing to store or nothing parsed to compile */
        goto cleanup;
    }

    switch (substmt->stmt) {
    case LY_STMT_NOTIFICATION:
    case LY_STMT_INPUT:
    case LY_STMT_OUTPUT:
    case LY_STMT_ACTION:
    case LY_STMT_RPC:
    case LY_STMT_ANYDATA:
    case LY_STMT_ANYXML:
    case LY_STMT_CASE:
    case LY_STMT_CHOICE:
    case LY_STMT_CONTAINER:
    case LY_STMT_LEAF:
    case LY_STMT_LEAF_LIST:
    case LY_STMT_LIST:
    case LY_STMT_USES: {
        const uint16_t flags;
        struct lysp_node *pnodes, *pnode;
        struct lysc_node *node;

        lyplg_ext_get_storage(ext, LY_STMT_STATUS, (const void **)&flags);
        pnodes = (struct lysp_node *)parsed;
        if (aug_target) {
            /* compile augmented nodes */
            LY_CHECK_GOTO(rc = lys_compile_augment_children(ctx, NULL, 0, (struct lysp_node *)pnodes, aug_target, 0), cleanup);
        } else {
            /* compile nodes */
            LY_LIST_FOR(pnodes, pnode) {
                if (pnode->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
                    /* manual compile */
                    node = calloc(1, sizeof(struct lysc_node_action_inout));
                    LY_CHECK_ERR_GOTO(!node, LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);
                    LY_CHECK_GOTO(rc = lys_compile_node_action_inout(ctx, pnode, node), cleanup);
                    LY_CHECK_GOTO(rc = lys_compile_node_connect(ctx, NULL, node), cleanup);
                } else {
                    /* ctx->ext substatement storage is used as the document root */
                    LY_CHECK_GOTO(rc = lys_compile_node(ctx, pnode, NULL, flags ? &flags : NULL, NULL), cleanup);
                }
            }
        }
        break;
    }
    case LY_STMT_ARGUMENT:
    case LY_STMT_CONTACT:
    case LY_STMT_DESCRIPTION:
    case LY_STMT_ERROR_APP_TAG:
    case LY_STMT_ERROR_MESSAGE:
    case LY_STMT_KEY:
    case LY_STMT_MODIFIER:
    case LY_STMT_NAMESPACE:
    case LY_STMT_ORGANIZATION:
    case LY_STMT_PRESENCE:
    case LY_STMT_REFERENCE:
    case LY_STMT_UNITS:
        /* just make a copy */
        LY_CHECK_GOTO(rc = lydict_insert(ctx->ctx, parsed, 0, substmt->storage), cleanup);
        break;

    case LY_STMT_BIT:
        basetype = LY_TYPE_BITS;
    /* fallthrough */
    case LY_STMT_ENUM:
        if (substmt->stmt == LY_STMT_ENUM) {
            basetype = LY_TYPE_ENUM;
        }

        /* compile */
        LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, parsed, basetype, NULL, substmt->storage), cleanup);
        break;

    case LY_STMT_CONFIG: {
        const uint16_t flags = (uintptr_t)parsed;

        if (!(ctx->compile_opts & LYS_COMPILE_NO_CONFIG)) {
            if (flags & LYS_CONFIG_MASK) {
                /* explicitly set */
                *(uint16_t *)substmt->storage = flags | LYS_SET_CONFIG;
            } else if (ext->parent_stmt & LY_STMT_DATA_NODE_MASK) {
                /* inherit */
                *(uint16_t *)substmt->storage = ((struct lysc_node *)ext->parent)->flags & LYS_CONFIG_MASK;
            } else {
                /* default config */
                *(uint16_t *)substmt->storage = LYS_CONFIG_W;
            }
        } /* else leave zero */
        break;
    }
    case LY_STMT_MUST: {
        const struct lysp_restr *restrs = parsed;
        struct lysc_must *musts = *(struct lysc_must **)substmt->storage;

        /* sized array */
        COMPILE_ARRAY_GOTO(ctx, restrs, musts, lys_compile_must, rc, cleanup);
        break;
    }
    case LY_STMT_WHEN: {
        const uint16_t flags;
        const struct lysp_when *when = parsed;

        /* read compiled status */
        lyplg_ext_get_storage(ext, LY_STMT_STATUS, (const void **)&flags);

        /* compile */
        LY_CHECK_GOTO(rc = lys_compile_when(ctx, when, flags, NULL, NULL, NULL, substmt->storage), cleanup);
        break;
    }
    case LY_STMT_FRACTION_DIGITS:
    case LY_STMT_REQUIRE_INSTANCE:
        /* just make a copy */
        *(uint8_t *)substmt->storage = (uintptr_t)parsed;
        break;

    case LY_STMT_MANDATORY:
    case LY_STMT_ORDERED_BY:
    case LY_STMT_STATUS:
        /* just make a copy */
        *(uint16_t *)substmt->storage = (uintptr_t)parsed;
        break;

    case LY_STMT_MAX_ELEMENTS:
    case LY_STMT_MIN_ELEMENTS:
        /* just make a copy */
        *(uint32_t *)substmt->storage = (uintptr_t)parsed;
        break;

    case LY_STMT_POSITION:
    case LY_STMT_VALUE:
        /* just make a copy */
        *(int64_t *)substmt->storage = (uintptr_t)parsed;
        break;

    case LY_STMT_IDENTITY:
        /* compile */
        LY_CHECK_GOTO(rc = lys_identity_precompile(ctx, NULL, NULL, parsed, substmt->storage), cleanup);
        break;

    case LY_STMT_LENGTH:
        length_restr = 1;
    /* fallthrough */
    case LY_STMT_RANGE:
        /* compile, use uint64 default range */
        LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, parsed, LY_TYPE_UINT64, length_restr, 0, NULL, substmt->storage),
                cleanup);
        break;

    case LY_STMT_PATTERN:
        /* compile */
        LY_CHECK_GOTO(rc = lys_compile_type_patterns(ctx, parsed, NULL, substmt->storage), cleanup);
        break;

    case LY_STMT_TYPE: {
        const uint16_t flags;
        const char *units;
        const struct lysp_type *ptype = parsed;

        /* read compiled info */
        lyplg_ext_get_storage(ext, LY_STMT_STATUS, (const void **)&flags);
        lyplg_ext_get_storage(ext, LY_STMT_UNITS, (const void **)&units);

        /* compile */
        LY_CHECK_GOTO(rc = lys_compile_type(ctx, NULL, flags, ext->def->name, ptype, substmt->storage, &units, NULL), cleanup);
        break;
    }
    case LY_STMT_EXTENSION_INSTANCE: {
        struct lysp_ext_instance *extps = (struct lysp_ext_instance *)parsed;
        struct lysc_ext_instance *exts = *(struct lysc_ext_instance **)substmt->storage;

        /* compile sized array */
        COMPILE_EXTS_GOTO(ctx, extps, exts, ext, rc, cleanup);
        break;
    }
    case LY_STMT_AUGMENT:
    case LY_STMT_GROUPING:
    case LY_STMT_BASE:
    case LY_STMT_BELONGS_TO:
    case LY_STMT_DEFAULT:
    case LY_STMT_DEVIATE:
    case LY_STMT_DEVIATION:
    case LY_STMT_EXTENSION:
    case LY_STMT_FEATURE:
    case LY_STMT_IF_FEATURE:
    case LY_STMT_IMPORT:
    case LY_STMT_INCLUDE:
    case LY_STMT_MODULE:
    case LY_STMT_PATH:
    case LY_STMT_PREFIX:
    case LY_STMT_REFINE:
    case LY_STMT_REVISION:
    case LY_STMT_REVISION_DATE:
    case LY_STMT_SUBMODULE:
    case LY_STMT_TYPEDEF:
    case LY_STMT_UNIQUE:
    case LY_STMT_YANG_VERSION:
    case LY_STMT_YIN_ELEMENT:
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Statement \"%s\" compilation is not supported.", lyplg_ext_stmt2str(substmt->stmt));
        rc = LY_EVALID;
        goto cleanup;

    default:
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Statement \"%s\" is not supported as an extension "
                "(found in \"%s%s%s\") substatement.", lyplg_ext_stmt2str(substmt->stmt), ext->def->name,
                ext->argument ? " " : "", ext->argument ? ext->argument : "");
        rc = LY_EVALID;
        goto cleanup;
    }

cleanup:
    return rc;
}

static LY_ERR
lys_compile_extension_instance_(struct lysc_ctx *ctx, const struct lysp_ext_instance *extp, struct lysc_ext_instance *ext,
        struct lysc_node *aug_target)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    enum ly_stmt stmtp;
    const void *storagep;
    struct ly_set storagep_compiled = {0};

    /* note into the compile context that we are processing extension now */
    ctx->ext = ext;

    LY_ARRAY_FOR(extp->substmts, u) {
        stmtp = extp->substmts[u].stmt;
        storagep = *(void **)extp->substmts[u].storage;

        if (ly_set_contains(&storagep_compiled, storagep, NULL)) {
            /* this parsed statement has already been compiled (for example, if it is a linked list of parsed nodes) */
            continue;
        }

        LY_ARRAY_FOR(ext->substmts, v) {
            if (stmtp != ext->substmts[v].stmt) {
                continue;
            }

            if ((rc = lys_compile_ext_instance_stmt(ctx, storagep, ext, &ext->substmts[v], aug_target))) {
                goto cleanup;
            }

            /* parsed substatement compiled */
            break;
        }

        /* compiled */
        ly_set_add(&storagep_compiled, storagep, 1, NULL);
    }

cleanup:
    ctx->ext = NULL;
    ly_set_erase(&storagep_compiled, NULL);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_compile_extension_instance(struct lysc_ctx *ctx, const struct lysp_ext_instance *ext_p, struct lysc_ext_instance *ext)
{
    LY_CHECK_ARG_RET(ctx ? ctx->ctx : NULL, ctx, ext_p, ext, LY_EINVAL);

    return lys_compile_extension_instance_(ctx, ext_p, ext, NULL);
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_compile_extension_instance_augment(struct lysc_ctx *ctx, const struct lysp_ext_instance *ext_p,
        struct lysc_ext_instance *ext, struct lysc_node *aug_target)
{
    LY_CHECK_ARG_RET(ctx ? ctx->ctx : NULL, ctx, ext_p, ext, aug_target, LY_EINVAL);

    return lys_compile_extension_instance_(ctx, ext_p, ext, aug_target);
}

LIBYANG_API_DEF struct ly_ctx *
lyplg_ext_compile_get_ctx(const struct lysc_ctx *ctx)
{
    return ctx->ctx;
}

LIBYANG_API_DEF uint32_t *
lyplg_ext_compile_get_options(const struct lysc_ctx *ctx)
{
    return &((struct lysc_ctx *)ctx)->compile_opts;
}

LIBYANG_API_DEF const struct lys_module *
lyplg_ext_compile_get_cur_mod(const struct lysc_ctx *ctx)
{
    return ctx->cur_mod;
}

LIBYANG_API_DEF struct lysp_module *
lyplg_ext_compile_get_pmod(const struct lysc_ctx *ctx)
{
    return ctx->pmod;
}

LIBYANG_API_DEF struct ly_out **
lyplg_ext_print_get_out(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->out;
}

LIBYANG_API_DEF uint32_t *
lyplg_ext_print_get_options(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->options;
}

LIBYANG_API_DEF uint16_t *
lyplg_ext_print_get_level(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->level;
}

LIBYANG_API_DEF const char *
lyplg_ext_stmt2str(enum ly_stmt stmt)
{
    if (stmt == LY_STMT_EXTENSION_INSTANCE) {
        return "extension instance";
    } else {
        return stmt_attr_info[stmt].name;
    }
}

LIBYANG_API_DEF enum ly_stmt
lyplg_ext_nodetype2stmt(uint16_t nodetype)
{
    switch (nodetype) {
    case LYS_CONTAINER:
        return LY_STMT_CONTAINER;
    case LYS_CHOICE:
        return LY_STMT_CHOICE;
    case LYS_LEAF:
        return LY_STMT_LEAF;
    case LYS_LEAFLIST:
        return LY_STMT_LEAF_LIST;
    case LYS_LIST:
        return LY_STMT_LIST;
    case LYS_ANYXML:
        return LY_STMT_ANYXML;
    case LYS_ANYDATA:
        return LY_STMT_ANYDATA;
    case LYS_CASE:
        return LY_STMT_CASE;
    case LYS_RPC:
        return LY_STMT_RPC;
    case LYS_ACTION:
        return LY_STMT_ACTION;
    case LYS_NOTIF:
        return LY_STMT_NOTIFICATION;
    case LYS_USES:
        return LY_STMT_USES;
    case LYS_INPUT:
        return LY_STMT_INPUT;
    case LYS_OUTPUT:
        return LY_STMT_OUTPUT;
    default:
        return LY_STMT_NONE;
    }
}

LY_ERR
lyplg_ext_get_storage_p(const struct lysc_ext_instance *ext, int stmt, const void ***storage_p)
{
    LY_ARRAY_COUNT_TYPE u;
    enum ly_stmt match = 0;

    *storage_p = NULL;

    if (!(stmt & LY_STMT_NODE_MASK)) {
        /* matching a non-node statement */
        match = stmt;
    }

    LY_ARRAY_FOR(ext->substmts, u) {
        if ((match && (ext->substmts[u].stmt == match)) || (!match && (ext->substmts[u].stmt & stmt))) {
            *storage_p = ext->substmts[u].storage;
            return LY_SUCCESS;
        }
    }

    return LY_ENOT;
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_get_storage(const struct lysc_ext_instance *ext, int stmt, const void **storage)
{
    LY_ERR r;
    const void **s;

    *storage = NULL;

    if ((r = lyplg_ext_get_storage_p(ext, stmt, &s))) {
        return r;
    }

    *storage = *s;
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_parsed_get_storage(const struct lysc_ext_instance *ext, int stmt, const void **storage)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lysp_ext_instance *extp = NULL;
    enum ly_stmt match = 0;

    *storage = NULL;

    /* find the parsed ext instance */
    LY_ARRAY_FOR(ext->module->parsed->exts, u) {
        extp = &ext->module->parsed->exts[u];

        if (ext->def == extp->def->compiled) {
            break;
        }
        extp = NULL;
    }
    assert(extp);

    if (!(stmt & LY_STMT_NODE_MASK)) {
        /* matching a non-node statement */
        match = stmt;
    }

    /* get the substatement */
    LY_ARRAY_FOR(extp->substmts, u) {
        if ((match && (extp->substmts[u].stmt == match)) || (!match && (extp->substmts[u].stmt & stmt))) {
            *storage = *(void **)extp->substmts[u].storage;
            return LY_SUCCESS;
        }
    }

    return LY_ENOT;
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_get_data(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, void **ext_data, ly_bool *ext_data_free)
{
    if (!ctx->ext_clb) {
        lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_EINVAL, "Failed to get extension data, no callback set.");
        return LY_EINVAL;
    }

    return ctx->ext_clb(ext, ctx->ext_clb_data, ext_data, ext_data_free);
}
