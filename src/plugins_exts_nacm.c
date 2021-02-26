/**
 * @file plugins_exts_nacm.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - NACM (RFC 6536)
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "common.h"

#include <stdlib.h>

#include "plugins_exts.h"
#include "tree_schema.h"

/**
 * @brief Storage for ID used to check plugin API version compatibility.
 * Ignored here in the internal plugin.
LYEXT_VERSION_CHECK
 */

struct nacm_dfs_arg {
    struct lysc_ext_instance *c_ext;
    struct lysc_node *parent;
};

/**
 * @brief DFS callback implementation for inheriting the NACM extension.
 */
static LY_ERR
nacm_inherit_clb(struct lysc_node *node, void *data, ly_bool *dfs_continue)
{
    struct nacm_dfs_arg *arg = data;
    struct lysc_ext_instance *inherited;
    LY_ARRAY_COUNT_TYPE u;

    /* ignore the parent from which we inherit and input/output nodes */
    if ((node != arg->parent) && !(node->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
        /* check that the node does not have its own NACM extension instance */
        LY_ARRAY_FOR(node->exts, u) {
            if (node->exts[u].def == arg->c_ext->def) {
                /* the child already have its own NACM flag, so skip the subtree */
                *dfs_continue = 1;
                return LY_SUCCESS;
            }
        }

        /* duplicate this one to inherit it to the child */
        LY_ARRAY_NEW_RET(node->module->ctx, node->exts, inherited, LY_EMEM);

        inherited->def = lysc_ext_dup(arg->c_ext->def);
        inherited->parent = node;
        inherited->parent_type = LYEXT_PAR_NODE;
        if (arg->c_ext->argument) {
            LY_CHECK_RET(lydict_insert(node->module->ctx, arg->c_ext->argument, strlen(arg->c_ext->argument),
                    &inherited->argument));
        }
        /* TODO duplicate extension instances */
        inherited->data = arg->c_ext->data;
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile NAMC's extension instances.
 *
 * Implementation of lyext_clb_compile callback set as lyext_plugin::compile.
 */
LY_ERR
nacm_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    struct lysc_node *parent = NULL;
    LY_ARRAY_COUNT_TYPE u;
    struct nacm_dfs_arg dfs_arg;

    static const uint8_t nacm_deny_all = 1;
    static const uint8_t nacm_deny_write = 2;

    /* store the NACM flag */
    if (!strcmp(c_ext->def->name, "default-deny-write")) {
        c_ext->data = (void *)&nacm_deny_write;
    } else if (!strcmp(c_ext->def->name, "default-deny-all")) {
        c_ext->data = (void *)&nacm_deny_all;
    } else {
        return LY_EINT;
    }

    /* check that the extension is instantiated at an allowed place - data node */
    if (c_ext->parent_type != LYEXT_PAR_NODE) {
        lyext_log(c_ext, LY_LLWRN, 0, cctx->path, "Extension %s is allowed only in a data nodes, but it is placed in \"%s\" statement.",
                p_ext->name, lyext_parent2str(c_ext->parent_type));
        return LY_ENOT;
    } else {
        parent = (struct lysc_node *)c_ext->parent;
        if (!(parent->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE | LYS_ANYDATA |
                LYS_CASE | LYS_RPC | LYS_ACTION | LYS_NOTIF))) {
            /* note LYS_AUGMENT and LYS_USES is not in the list since they are not present in the compiled tree. Instead, libyang
             * passes all their extensions to their children nodes */
invalid_parent:
            lyext_log(c_ext, LY_LLWRN, 0, cctx->path,
                    "Extension %s is not allowed in %s statement.", p_ext->name, lys_nodetype2str(parent->nodetype));
            return LY_ENOT;
        }
        if ((c_ext->data == (void *)&nacm_deny_write) && (parent->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))) {
            goto invalid_parent;
        }
    }

    /* check for duplication */
    LY_ARRAY_FOR(parent->exts, u) {
        if ((&parent->exts[u] != c_ext) && (parent->exts[u].def->plugin == c_ext->def->plugin)) {
            /* duplication of a NACM extension on a single node
             * We check plugin since we want to catch even the situation that there is default-deny-all
             * AND default-deny-write */
            if (parent->exts[u].def == c_ext->def) {
                lyext_log(c_ext, LY_LLERR, LY_EVALID, cctx->path, "Extension %s is instantiated multiple times.", p_ext->name);
            } else {
                lyext_log(c_ext, LY_LLERR, LY_EVALID, cctx->path, "Extension nacm:default-deny-write is mixed with nacm:default-deny-all.");
            }
            return LY_EVALID;
        }
    }

    /* inherit the extension instance to all the children nodes */
    dfs_arg.c_ext = c_ext;
    dfs_arg.parent = parent;
    LY_CHECK_RET(lysc_tree_dfs_full(parent, nacm_inherit_clb, &dfs_arg));

    return LY_SUCCESS;
}

/**
 * @brief Plugin for the NACM's default-deny-write and default-deny-all extensions
 */
struct lyext_plugin nacm_plugin = {
    .id = "libyang 2 - NACM, version 1",
    .compile = &nacm_compile,
    .validate = NULL,
    .free = NULL
};
