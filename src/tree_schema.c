/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "tree_schema.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_internal.h"
#include "parser_schema.h"
#include "path.h"
#include "schema_compile.h"
#include "schema_compile_amend.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema_internal.h"
#include "xpath.h"

API LY_ERR
lysc_tree_dfs_full(const struct lysc_node *root, lysc_dfs_clb dfs_clb, void *data)
{
    struct lysc_node *elem, *elem2;
    const struct lysc_action *acts;
    const struct lysc_notif *notifs;
    LY_ARRAY_COUNT_TYPE u;

    LY_CHECK_ARG_RET(NULL, root, dfs_clb, LY_EINVAL);

    LYSC_TREE_DFS_BEGIN(root, elem) {
        /* schema node */
        LY_CHECK_RET(dfs_clb(elem, data, &LYSC_TREE_DFS_continue));

        acts = lysc_node_actions(elem);
        LY_ARRAY_FOR(acts, u) {
            LYSC_TREE_DFS_BEGIN(&acts[u], elem2) {
                /* action subtree */
                LY_CHECK_RET(dfs_clb(elem2, data, &LYSC_TREE_DFS_continue));

                LYSC_TREE_DFS_END(&acts[u], elem2);
            }
        }

        notifs = lysc_node_notifs(elem);
        LY_ARRAY_FOR(notifs, u) {
            LYSC_TREE_DFS_BEGIN(&notifs[u], elem2) {
                /* notification subtree */
                LY_CHECK_RET(dfs_clb(elem2, data, &LYSC_TREE_DFS_continue));

                LYSC_TREE_DFS_END(&notifs[u], elem2);
            }
        }

        LYSC_TREE_DFS_END(root, elem);
    }

    return LY_SUCCESS;
}

API LY_ERR
lysc_module_dfs_full(const struct lys_module *mod, lysc_dfs_clb dfs_clb, void *data)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lysc_node *root;

    LY_CHECK_ARG_RET(NULL, mod, mod->compiled, dfs_clb, LY_EINVAL);

    /* schema nodes */
    LY_LIST_FOR(mod->compiled->data, root) {
        LY_CHECK_RET(lysc_tree_dfs_full(root, dfs_clb, data));
    }

    /* RPCs */
    LY_ARRAY_FOR(mod->compiled->rpcs, u) {
        LY_CHECK_RET(lysc_tree_dfs_full((struct lysc_node *)&mod->compiled->rpcs[u], dfs_clb, data));
    }

    /* notifications */
    LY_ARRAY_FOR(mod->compiled->notifs, u) {
        LY_CHECK_RET(lysc_tree_dfs_full((struct lysc_node *)&mod->compiled->notifs[u], dfs_clb, data));
    }

    return LY_SUCCESS;
}

static void
lys_getnext_into_case(const struct lysc_node_case *first_case, const struct lysc_node **last, const struct lysc_node **next)
{
    for ( ; first_case; first_case = (const struct lysc_node_case *)first_case->next) {
        if (first_case->child) {
            /* there is something to return */
            (*next) = first_case->child;
            return;
        }
    }

    /* no children in choice's cases, so go to the choice's sibling instead of into it */
    (*last) = (*next);
    (*next) = (*next)->next;
}

API const struct lysc_node *
lys_getnext(const struct lysc_node *last, const struct lysc_node *parent, const struct lysc_module *module, uint32_t options)
{
    const struct lysc_node *next = NULL;
    struct lysc_node **snode;
    ly_bool action_flag = 0, notif_flag = 0;
    const struct lysc_action *actions;
    const struct lysc_notif *notifs;
    LY_ARRAY_COUNT_TYPE u;

    LY_CHECK_ARG_RET(NULL, parent || module, NULL);

next:
    if (!last) {
        /* first call */

        /* get know where to start */
        if (parent) {
            /* schema subtree */
            if ((parent->nodetype == LYS_CHOICE) && (options & LYS_GETNEXT_WITHCASE)) {
                if (((struct lysc_node_choice *)parent)->cases) {
                    next = last = (const struct lysc_node *)((struct lysc_node_choice *)parent)->cases;
                }
            } else {
                snode = lysc_node_children_p(parent, (options & LYS_GETNEXT_OUTPUT) ? LYS_CONFIG_R : LYS_CONFIG_W);
                /* do not return anything if the node does not have any children */
                if (snode && *snode) {
                    next = last = *snode;
                }
            }
        } else {
            /* top level data */
            next = last = module->data;
        }
        if (!next) {
            /* try to get action or notification */
            goto repeat;
        }
        /* test if the next can be returned */
        goto check;

    } else if (last->nodetype & (LYS_RPC | LYS_ACTION)) {
        action_flag = 1;
        if (last->parent) {
            actions = lysc_node_actions(last->parent);
        } else {
            actions = module->rpcs;
        }
        LY_ARRAY_FOR(actions, u) {
            if (&actions[u] == (struct lysc_action *)last) {
                break;
            }
        }
        if (u + 1 < LY_ARRAY_COUNT(actions)) {
            next = (struct lysc_node *)(&actions[u + 1]);
        }
        goto repeat;
    } else if (last->nodetype == LYS_NOTIF) {
        action_flag = notif_flag = 1;
        if (last->parent) {
            notifs = lysc_node_notifs(last->parent);
        } else {
            notifs = module->notifs;
        }
        LY_ARRAY_FOR(notifs, u) {
            if (&notifs[u] == (struct lysc_notif *)last) {
                break;
            }
        }
        if (u + 1 < LY_ARRAY_COUNT(notifs)) {
            next = (struct lysc_node *)(&notifs[u + 1]);
        }
        goto repeat;
    } else {
        next = last->next;
    }

repeat:
    if (!next) {
        /* possibly go back to parent */
        if (last && (last->parent != parent)) {
            last = last->parent;
            goto next;
        } else if (!action_flag) {
            action_flag = 1;
            next = parent ? (struct lysc_node *)lysc_node_actions(parent) : (struct lysc_node *)module->rpcs;
        } else if (!notif_flag) {
            notif_flag = 1;
            next = parent ? (struct lysc_node *)lysc_node_notifs(parent) : (struct lysc_node *)module->notifs;
        } else {
            return NULL;
        }
        goto repeat;
    }
check:
    switch (next->nodetype) {
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
    case LYS_LIST:
    case LYS_LEAFLIST:
        break;
    case LYS_CASE:
        if (options & LYS_GETNEXT_WITHCASE) {
            break;
        } else {
            /* go into */
            lys_getnext_into_case((const struct lysc_node_case *)next, &last, &next);
        }
        goto repeat;
    case LYS_CONTAINER:
        if (!(((struct lysc_node_container *)next)->flags & LYS_PRESENCE) && (options & LYS_GETNEXT_INTONPCONT)) {
            if (((struct lysc_node_container *)next)->child) {
                /* go into */
                next = ((struct lysc_node_container *)next)->child;
            } else {
                last = next;
                next = next->next;
            }
            goto repeat;
        }
        break;
    case LYS_CHOICE:
        if (options & LYS_GETNEXT_WITHCHOICE) {
            break;
        } else if ((options & LYS_GETNEXT_NOCHOICE) || !((struct lysc_node_choice *)next)->cases) {
            next = next->next;
        } else {
            if (options & LYS_GETNEXT_WITHCASE) {
                next = (struct lysc_node *)((struct lysc_node_choice *)next)->cases;
            } else {
                /* go into */
                lys_getnext_into_case(((struct lysc_node_choice *)next)->cases, &last, &next);
            }
        }
        goto repeat;
    default:
        /* we should not be here */
        LOGINT(module ? module->mod->ctx : parent->module->ctx);
        return NULL;
    }

    return next;
}

API const struct lysc_node *
lys_find_child(const struct lysc_node *parent, const struct lys_module *module, const char *name, size_t name_len,
        uint16_t nodetype, uint32_t options)
{
    const struct lysc_node *node = NULL;

    LY_CHECK_ARG_RET(NULL, module, name, NULL);
    if (!nodetype) {
        nodetype = 0xffff;
    }

    while ((node = lys_getnext(node, parent, module->compiled, options))) {
        if (!(node->nodetype & nodetype)) {
            continue;
        }
        if (node->module != module) {
            continue;
        }

        if (name_len) {
            if (!ly_strncmp(node->name, name, name_len)) {
                return node;
            }
        } else {
            if (!strcmp(node->name, name)) {
                return node;
            }
        }
    }
    return NULL;
}

API LY_ERR
lys_find_xpath_atoms(const struct lysc_node *ctx_node, const char *xpath, uint32_t options, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set;
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx_node, xpath, set, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }

    memset(&xp_set, 0, sizeof xp_set);

    /* compile expression */
    ret = lyxp_expr_parse(ctx_node->module->ctx, xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* atomize expression */
    ret = lyxp_atomize(exp, NULL, LY_PREF_JSON, NULL, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    ret = ly_set_new(set);
    LY_CHECK_GOTO(ret, cleanup);

    /* transform into ly_set */
    (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
    LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(ctx_node->module->ctx); ret = LY_EMEM, cleanup);
    (*set)->size = xp_set.used;

    for (i = 0; i < xp_set.used; ++i) {
        if (xp_set.val.scnodes[i].type == LYXP_NODE_ELEM) {
            ret = ly_set_add(*set, xp_set.val.scnodes[i].scnode, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free(ctx_node->module->ctx, exp);
    return ret;
}

API LY_ERR
lys_find_expr_atoms(const struct lysc_node *ctx_node, const struct lys_module *cur_mod, const struct lyxp_expr *expr,
        const struct lysc_prefix *prefixes, uint32_t options, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, cur_mod, expr, prefixes, set, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }

    /* atomize expression */
    ret = lyxp_atomize(expr, cur_mod, LY_PREF_SCHEMA_RESOLVED, (void *)prefixes, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    ret = ly_set_new(set);
    LY_CHECK_GOTO(ret, cleanup);

    /* transform into ly_set */
    (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
    LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(cur_mod->ctx); ret = LY_EMEM, cleanup);
    (*set)->size = xp_set.used;

    for (i = 0; i < xp_set.used; ++i) {
        if ((xp_set.val.scnodes[i].type == LYXP_NODE_ELEM) && (xp_set.val.scnodes[i].in_ctx == 1)) {
            ret = ly_set_add(*set, xp_set.val.scnodes[i].scnode, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    if (ret) {
        ly_set_free(*set, NULL);
        *set = NULL;
    }
    return ret;
}

API LY_ERR
lys_find_xpath(const struct lysc_node *ctx_node, const char *xpath, uint32_t options, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx_node, xpath, set, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }

    /* compile expression */
    ret = lyxp_expr_parse(ctx_node->module->ctx, xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* atomize expression */
    ret = lyxp_atomize(exp, NULL, LY_PREF_JSON, NULL, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    ret = ly_set_new(set);
    LY_CHECK_GOTO(ret, cleanup);

    /* transform into ly_set */
    (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
    LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(ctx_node->module->ctx); ret = LY_EMEM, cleanup);
    (*set)->size = xp_set.used;

    for (i = 0; i < xp_set.used; ++i) {
        if ((xp_set.val.scnodes[i].type == LYXP_NODE_ELEM) && (xp_set.val.scnodes[i].in_ctx == 1)) {
            ret = ly_set_add(*set, xp_set.val.scnodes[i].scnode, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free(ctx_node->module->ctx, exp);
    if (ret) {
        ly_set_free(*set, NULL);
        *set = NULL;
    }
    return ret;
}

API LY_ERR
lys_find_lypath_atoms(const struct ly_path *path, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;

    LY_CHECK_ARG_RET(NULL, path, set, LY_EINVAL);

    /* allocate return set */
    LY_CHECK_RET(ly_set_new(set));

    LY_ARRAY_FOR(path, u) {
        /* add nodes from the path */
        LY_CHECK_GOTO(ret = ly_set_add(*set, (void *)path[u].node, 0, NULL), cleanup);
        if (path[u].pred_type == LY_PATH_PREDTYPE_LIST) {
            LY_ARRAY_FOR(path[u].predicates, v) {
                /* add all the keys in a predicate */
                LY_CHECK_GOTO(ret = ly_set_add(*set, (void *)path[u].predicates[v].key, 0, NULL), cleanup);
            }
        }
    }

cleanup:
    if (ret) {
        ly_set_free(*set, NULL);
        *set = NULL;
    }
    return ret;
}

API LY_ERR
lys_find_path_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path, ly_bool output,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t oper;
    struct lyxp_expr *expr = NULL;
    struct ly_path *p = NULL;

    LY_CHECK_ARG_RET(ctx, ctx || ctx_node, path, set, LY_EINVAL);

    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* parse */
    ret = lyxp_expr_parse(ctx, path, strlen(path), 0, &expr);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile */
    oper = output ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;
    ret = ly_path_compile(ctx, NULL, ctx_node, expr, LY_PATH_LREF_FALSE, oper, LY_PATH_TARGET_MANY,
            LY_PREF_JSON, NULL, &p);
    LY_CHECK_GOTO(ret, cleanup);

    /* resolve */
    ret = lys_find_lypath_atoms(p, set);

cleanup:
    ly_path_free(ctx, p);
    lyxp_expr_free(ctx, expr);
    return ret;
}

API const struct lysc_node *
lys_find_path(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path, ly_bool output)
{
    const struct lysc_node *snode = NULL;
    struct lyxp_expr *exp = NULL;
    struct ly_path *p = NULL;
    LY_ERR ret;
    uint8_t oper;

    LY_CHECK_ARG_RET(ctx, ctx || ctx_node, NULL);

    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* parse */
    ret = lyxp_expr_parse(ctx, path, strlen(path), 0, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile */
    oper = output ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;
    ret = ly_path_compile(ctx, NULL, ctx_node, exp, LY_PATH_LREF_FALSE, oper, LY_PATH_TARGET_MANY,
            LY_PREF_JSON, NULL, &p);
    LY_CHECK_GOTO(ret, cleanup);

    /* get last node */
    snode = p[LY_ARRAY_COUNT(p) - 1].node;

cleanup:
    ly_path_free(ctx, p);
    lyxp_expr_free(ctx, exp);
    return snode;
}

char *
lysc_path_until(const struct lysc_node *node, const struct lysc_node *parent, LYSC_PATH_TYPE pathtype, char *buffer,
        size_t buflen)
{
    const struct lysc_node *iter;
    char *path = NULL;
    int len = 0;

    LY_CHECK_ARG_RET(NULL, node, NULL);
    if (buffer) {
        LY_CHECK_ARG_RET(node->module->ctx, buflen > 1, NULL);
    }

    switch (pathtype) {
    case LYSC_PATH_LOG:
    case LYSC_PATH_DATA:
        for (iter = node; iter && (iter != parent) && (len >= 0); iter = iter->parent) {
            char *s, *id;
            const char *slash;

            if ((pathtype == LYSC_PATH_DATA) && (iter->nodetype & (LYS_CHOICE | LYS_CASE))) {
                /* schema-only node */
                continue;
            }

            s = buffer ? strdup(buffer) : path;
            id = strdup(iter->name);
            if (parent && (iter->parent == parent)) {
                slash = "";
            } else {
                slash = "/";
            }
            if (!iter->parent || (iter->parent->module != iter->module)) {
                /* print prefix */
                if (buffer) {
                    len = snprintf(buffer, buflen, "%s%s:%s%s", slash, iter->module->name, id, s ? s : "");
                } else {
                    len = asprintf(&path, "%s%s:%s%s", slash, iter->module->name, id, s ? s : "");
                }
            } else {
                /* prefix is the same as in parent */
                if (buffer) {
                    len = snprintf(buffer, buflen, "%s%s%s", slash, id, s ? s : "");
                } else {
                    len = asprintf(&path, "%s%s%s", slash, id, s ? s : "");
                }
            }
            free(s);
            free(id);

            if (buffer && (buflen <= (size_t)len)) {
                /* not enough space in buffer */
                break;
            }
        }

        if (len < 0) {
            free(path);
            path = NULL;
        } else if (len == 0) {
            if (buffer) {
                strcpy(buffer, "/");
            } else {
                path = strdup("/");
            }
        }
        break;
    }

    if (buffer) {
        return buffer;
    } else {
        return path;
    }
}

API char *
lysc_path(const struct lysc_node *node, LYSC_PATH_TYPE pathtype, char *buffer, size_t buflen)
{
    return lysc_path_until(node, NULL, pathtype, buffer, buflen);
}

API LY_ERR
lysc_set_private(const struct lysc_node *node, void *priv, void **prev_priv_p)
{
    struct lysc_action *act;
    struct lysc_notif *notif;

    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);

    switch (node->nodetype) {
    case LYS_CONTAINER:
    case LYS_CHOICE:
    case LYS_CASE:
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_LIST:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        if (prev_priv_p) {
            *prev_priv_p = node->priv;
        }
        ((struct lysc_node *)node)->priv = priv;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        act = (struct lysc_action *)node;
        if (prev_priv_p) {
            *prev_priv_p = act->priv;
        }
        act->priv = priv;
        break;
    case LYS_NOTIF:
        notif = (struct lysc_notif *)node;
        if (prev_priv_p) {
            *prev_priv_p = notif->priv;
        }
        notif->priv = priv;
        break;
    default:
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

API LY_ERR
lys_set_implemented(struct lys_module *mod, const char **features)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *m;
    uint32_t i, idx;

    LY_CHECK_ARG_RET(NULL, mod, LY_EINVAL);

    if (mod->implemented) {
        /* mod is already implemented */
        return LY_SUCCESS;
    }

    /* we have module from the current context */
    m = ly_ctx_get_module_implemented(mod->ctx, mod->name);
    if (m) {
        assert(m != mod);

        /* check collision with other implemented revision */
        LOGERR(mod->ctx, LY_EDENIED, "Module \"%s%s%s\" is present in the context in other implemented revision (%s).",
                mod->name, mod->revision ? "@" : "", mod->revision ? mod->revision : "", m->revision ? m->revision : "none");
        return LY_EDENIED;
    }

    /* enable features */
    LY_CHECK_RET(lys_enable_features(mod->parsed, features));

    /* add the module into newly implemented module set */
    LY_CHECK_RET(ly_set_add(&mod->ctx->implementing, mod, 1, NULL));

    /* mark the module implemented, check for collision was already done */
    mod->implemented = 1;

    /* compile the schema */
    ret = lys_compile(mod, 0);

    if (mod == mod->ctx->implementing.objs[0]) {
        /* the first module being implemented, consolidate the set */
        if (ret) {
            /* failure, full compile revert */
            for (i = 0; i < mod->ctx->list.count; ++i) {
                m = mod->ctx->list.objs[i];
                if (ly_set_contains(&mod->ctx->implementing, m, &idx)) {
                    assert(m->implemented);

                    /* make the module correctly non-implemented again */
                    m->implemented = 0;
                    ly_set_rm_index(&mod->ctx->implementing, idx, NULL);
                    lys_precompile_augments_deviations_revert(mod->ctx, m);
                }
            }

            /* recompile, do not overwrite return value */
            lys_recompile(mod->ctx, NULL);
        }

        ly_set_erase(&mod->ctx->implementing, NULL);
    }
    return ret;
}

static LY_ERR
lys_resolve_import_include(struct lys_parser_ctx *pctx, struct lysp_module *pmod)
{
    struct lysp_import *imp;
    struct lysp_include *inc;
    LY_ARRAY_COUNT_TYPE u, v;

    pmod->parsing = 1;
    LY_ARRAY_FOR(pmod->imports, u) {
        imp = &pmod->imports[u];
        if (!imp->module) {
            LY_CHECK_RET(lysp_load_module(PARSER_CTX(pctx), imp->name, imp->rev[0] ? imp->rev : NULL, 0, 0, NULL, &imp->module));
        }
        /* check for importing the same module twice */
        for (v = 0; v < u; ++v) {
            if (imp->module == pmod->imports[v].module) {
                LOGWRN(PARSER_CTX(pctx), "Single revision of the module \"%s\" imported twice.", imp->name);
            }
        }
    }
    LY_ARRAY_FOR(pmod->includes, u) {
        inc = &pmod->includes[u];
        if (!inc->submodule) {
            LY_CHECK_RET(lysp_load_submodule(pctx, inc));
        }
    }
    pmod->parsing = 0;

    return LY_SUCCESS;
}

LY_ERR
lys_parse_submodule(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, struct lys_parser_ctx *main_ctx,
        LY_ERR (*custom_check)(const struct ly_ctx *, struct lysp_module *, struct lysp_submodule *, void *),
        void *check_data, struct lysp_submodule **submodule)
{
    LY_ERR ret;
    struct lysp_submodule *submod = NULL, *latest_sp;
    struct lys_yang_parser_ctx *yangctx = NULL;
    struct lys_yin_parser_ctx *yinctx = NULL;
    struct lys_parser_ctx *pctx;

    LY_CHECK_ARG_RET(ctx, ctx, in, LY_EINVAL);

    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_submodule(&yinctx, ctx, main_ctx, in, &submod);
        pctx = (struct lys_parser_ctx *)yinctx;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_submodule(&yangctx, ctx, main_ctx, in, &submod);
        pctx = (struct lys_parser_ctx *)yangctx;
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        ret = LY_EINVAL;
        break;
    }
    LY_CHECK_GOTO(ret, error);
    assert(submod);

    /* make sure that the newest revision is at position 0 */
    lysp_sort_revisions(submod->revs);

    /* decide the latest revision */
    latest_sp = ly_ctx_get_submodule(NULL, submod->mod, submod->name, NULL);
    if (latest_sp) {
        if (submod->revs) {
            if (!latest_sp->revs) {
                /* latest has no revision, so mod is anyway newer */
                submod->latest_revision = latest_sp->latest_revision;
                /* the latest_sp is zeroed later when the new module is being inserted into the context */
            } else if (strcmp(submod->revs[0].date, latest_sp->revs[0].date) > 0) {
                submod->latest_revision = latest_sp->latest_revision;
                /* the latest_sp is zeroed later when the new module is being inserted into the context */
            } else {
                latest_sp = NULL;
            }
        } else {
            latest_sp = NULL;
        }
    } else {
        submod->latest_revision = 1;
    }

    if (custom_check) {
        LY_CHECK_GOTO(ret = custom_check(ctx, NULL, submod, check_data), error);
    }

    if (latest_sp) {
        latest_sp->latest_revision = 0;
    }

    lys_parser_fill_filepath(ctx, in, &submod->filepath);

    /* resolve imports and includes */
    LY_CHECK_GOTO(ret = lys_resolve_import_include(pctx, (struct lysp_module *)submod), error);

    /* remap possibly changed and reallocated typedefs and groupings list back to the main context */
    memcpy(&main_ctx->tpdfs_nodes, &pctx->tpdfs_nodes, sizeof main_ctx->tpdfs_nodes);
    memcpy(&main_ctx->grps_nodes, &pctx->grps_nodes, sizeof main_ctx->grps_nodes);

    if (format == LYS_IN_YANG) {
        yang_parser_ctx_free(yangctx);
    } else {
        yin_parser_ctx_free(yinctx);
    }
    *submodule = submod;
    return LY_SUCCESS;

error:
    lysp_module_free((struct lysp_module *)submod);
    if (format == LYS_IN_YANG) {
        yang_parser_ctx_free(yangctx);
    } else {
        yin_parser_ctx_free(yinctx);
    }
    return ret;
}

/**
 * @brief Add ietf-netconf metadata to the parsed module. Operation, filter, and select are added.
 *
 * @param[in] mod Parsed module to add to.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
static LY_ERR
lys_parsed_add_internal_ietf_netconf(struct lysp_module *mod)
{
    struct lysp_ext_instance *ext_p;
    struct lysp_stmt *stmt;
    struct lysp_import *imp;

    /*
     * 1) edit-config's operation
     */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "operation", 0, &ext_p->argument));
    ext_p->flags = LYS_INTERNAL;
    ext_p->insubstmt = LYEXT_SUBSTMT_SELF;
    ext_p->insubstmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enumeration", 0, &stmt->arg));
    stmt->kw = LY_STMT_TYPE;

    stmt->child = calloc(1, sizeof *stmt->child);
    stmt = stmt->child;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "merge", 0, &stmt->arg));
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "replace", 0, &stmt->arg));
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "create", 0, &stmt->arg));
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "delete", 0, &stmt->arg));
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "remove", 0, &stmt->arg));
    stmt->kw = LY_STMT_ENUM;

    /*
     * 2) filter's type
     */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &ext_p->argument));
    ext_p->flags = LYS_INTERNAL;
    ext_p->insubstmt = LYEXT_SUBSTMT_SELF;
    ext_p->insubstmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enumeration", 0, &stmt->arg));
    stmt->kw = LY_STMT_TYPE;

    stmt->child = calloc(1, sizeof *stmt->child);
    stmt = stmt->child;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "subtree", 0, &stmt->arg));
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "xpath", 0, &stmt->arg));
    stmt->kw = LY_STMT_ENUM;

    /* if-feature for enum allowed only for YANG 1.1 modules */
    if (mod->version >= LYS_VERSION_1_1) {
        stmt->child = calloc(1, sizeof *stmt->child);
        stmt = stmt->child;
        LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
        LY_CHECK_RET(lydict_insert(mod->mod->ctx, "if-feature", 0, &stmt->stmt));
        LY_CHECK_RET(lydict_insert(mod->mod->ctx, "xpath", 0, &stmt->arg));
        stmt->kw = LY_STMT_IF_FEATURE;
    }

    /*
     * 3) filter's select
     */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "select", 0, &ext_p->argument));
    ext_p->flags = LYS_INTERNAL;
    ext_p->insubstmt = LYEXT_SUBSTMT_SELF;
    ext_p->insubstmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "yang_:xpath1.0", 0, &stmt->arg));
    stmt->kw = LY_STMT_TYPE;

    /* create new imports for the used prefixes */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->imports, imp, LY_EMEM);

    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "ietf-yang-metadata", 0, &imp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_", 0, &imp->prefix));
    imp->flags = LYS_INTERNAL;

    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->imports, imp, LY_EMEM);

    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "ietf-yang-types", 0, &imp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "yang_", 0, &imp->prefix));
    imp->flags = LYS_INTERNAL;

    return LY_SUCCESS;
}

/**
 * @brief Add ietf-netconf-with-defaults "default" metadata to the parsed module.
 *
 * @param[in] mod Parsed module to add to.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
static LY_ERR
lys_parsed_add_internal_ietf_netconf_with_defaults(struct lysp_module *mod)
{
    struct lysp_ext_instance *ext_p;
    struct lysp_stmt *stmt;
    struct lysp_import *imp;

    /* add new extension instance */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);

    /* fill in the extension instance fields */
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "default", 0, &ext_p->argument));
    ext_p->flags = LYS_INTERNAL;
    ext_p->insubstmt = LYEXT_SUBSTMT_SELF;
    ext_p->insubstmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "boolean", 0, &stmt->arg));
    stmt->kw = LY_STMT_TYPE;

    /* create new import for the used prefix */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->imports, imp, LY_EMEM);

    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "ietf-yang-metadata", 0, &imp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_", 0, &imp->prefix));
    imp->flags = LYS_INTERNAL;

    return LY_SUCCESS;
}

LY_ERR
lys_create_module(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, ly_bool implement,
        LY_ERR (*custom_check)(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
        void *check_data, const char **features, struct lys_module **module)
{
    struct lys_module *mod = NULL, *latest, *mod_dup;
    struct lysp_submodule *submod;
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    struct lys_yang_parser_ctx *yangctx = NULL;
    struct lys_yin_parser_ctx *yinctx = NULL;
    struct lys_parser_ctx *pctx = NULL;
    char *filename, *rev, *dot;
    size_t len;

    LY_CHECK_ARG_RET(ctx, ctx, in, !features || implement, LY_EINVAL);
    if (module) {
        *module = NULL;
    }

    mod = calloc(1, sizeof *mod);
    LY_CHECK_ERR_RET(!mod, LOGMEM(ctx), LY_EMEM);
    mod->ctx = ctx;

    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_module(&yinctx, in, mod);
        pctx = (struct lys_parser_ctx *)yinctx;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_module(&yangctx, in, mod);
        pctx = (struct lys_parser_ctx *)yangctx;
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        ret = LY_EINVAL;
        break;
    }
    LY_CHECK_GOTO(ret, error);

    /* make sure that the newest revision is at position 0 */
    lysp_sort_revisions(mod->parsed->revs);
    if (mod->parsed->revs) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, mod->parsed->revs[0].date, 0, &mod->revision), error);
    }

    /* decide the latest revision */
    latest = (struct lys_module *)ly_ctx_get_module_latest(ctx, mod->name);
    if (latest) {
        if (mod->revision) {
            if (!latest->revision) {
                /* latest has no revision, so mod is anyway newer */
                mod->latest_revision = latest->latest_revision;
                /* the latest is zeroed later when the new module is being inserted into the context */
            } else if (strcmp(mod->revision, latest->revision) > 0) {
                mod->latest_revision = latest->latest_revision;
                /* the latest is zeroed later when the new module is being inserted into the context */
            } else {
                latest = NULL;
            }
        } else {
            latest = NULL;
        }
    } else {
        mod->latest_revision = 1;
    }

    if (custom_check) {
        LY_CHECK_GOTO(ret = custom_check(ctx, mod->parsed, NULL, check_data), error);
    }

    /* check for duplicity in the context */
    if (implement && ly_ctx_get_module_implemented(ctx, mod->name)) {
        LOGERR(ctx, LY_EDENIED, "Module \"%s\" is already implemented in the context.", mod->name);
        ret = LY_EDENIED;
        goto error;
    }
    mod_dup = (struct lys_module *)ly_ctx_get_module(ctx, mod->name, mod->revision);
    if (mod_dup) {
        if (mod->parsed->revs) {
            LOGERR(ctx, LY_EEXIST, "Module \"%s\" of revision \"%s\" is already present in the context.",
                    mod->name, mod->parsed->revs[0].date);
        } else {
            LOGERR(ctx, LY_EEXIST, "Module \"%s\" with no revision is already present in the context.",
                    mod->name);
        }
        ret = LY_EEXIST;
        goto error;
    }

    switch (in->type) {
    case LY_IN_FILEPATH:
        /* check that name and revision match filename */
        filename = strrchr(in->method.fpath.filepath, '/');
        if (!filename) {
            filename = in->method.fpath.filepath;
        } else {
            filename++;
        }
        rev = strchr(filename, '@');
        dot = strrchr(filename, '.');

        /* name */
        len = strlen(mod->name);
        if (strncmp(filename, mod->name, len) ||
                ((rev && (rev != &filename[len])) || (!rev && (dot != &filename[len])))) {
            LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, mod->name);
        }
        if (rev) {
            len = dot - ++rev;
            if (!mod->parsed->revs || (len != 10) || strncmp(mod->parsed->revs[0].date, rev, len)) {
                LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                        mod->parsed->revs ? mod->parsed->revs[0].date : "none");
            }
        }

        break;
    case LY_IN_FD:
    case LY_IN_FILE:
    case LY_IN_MEMORY:
        /* nothing special to do */
        break;
    case LY_IN_ERROR:
        LOGINT(ctx);
        ret = LY_EINT;
        goto error;
    }
    lys_parser_fill_filepath(ctx, in, &mod->filepath);

    if (latest) {
        latest->latest_revision = 0;
    }

    /* add internal data in case specific modules were parsed */
    if (!strcmp(mod->name, "ietf-netconf")) {
        LY_CHECK_GOTO(ret = lys_parsed_add_internal_ietf_netconf(mod->parsed), error);
    } else if (!strcmp(mod->name, "ietf-netconf-with-defaults")) {
        LY_CHECK_GOTO(ret = lys_parsed_add_internal_ietf_netconf_with_defaults(mod->parsed), error);
    }

    /* add into context */
    ret = ly_set_add(&ctx->list, mod, 1, NULL);
    LY_CHECK_GOTO(ret, error);
    ctx->module_set_id++;

    /* resolve includes and all imports */
    LY_CHECK_GOTO(ret = lys_resolve_import_include(pctx, mod->parsed), error_ctx);

    /* check name collisions */
    LY_CHECK_GOTO(ret = lysp_check_dup_typedefs(pctx, mod->parsed), error_ctx);
    /* TODO groupings */
    LY_CHECK_GOTO(ret = lysp_check_dup_features(pctx, mod->parsed), error_ctx);
    LY_CHECK_GOTO(ret = lysp_check_dup_identities(pctx, mod->parsed), error_ctx);

    /* compile features */
    LY_CHECK_GOTO(ret = lys_compile_feature_iffeatures(mod->parsed), error_ctx);

    if (!implement) {
        /* pre-compile identities of the module */
        LY_CHECK_GOTO(ret = lys_identity_precompile(NULL, ctx, mod->parsed, mod->parsed->identities, &mod->identities), error);

        /* pre-compile identities of any submodules */
        LY_ARRAY_FOR(mod->parsed->includes, u) {
            submod = mod->parsed->includes[u].submodule;
            ret = lys_identity_precompile(NULL, ctx, (struct lysp_module *)submod, submod->identities, &mod->identities);
            LY_CHECK_GOTO(ret, error);
        }
    } else {
        /* implement (compile) */
        LY_CHECK_GOTO(ret = lys_set_implemented(mod, features), error_ctx);
    }

    if (format == LYS_IN_YANG) {
        yang_parser_ctx_free(yangctx);
    } else {
        yin_parser_ctx_free(yinctx);
    }
    if (module) {
        *module = mod;
    }
    return LY_SUCCESS;

error_ctx:
    ly_set_rm(&ctx->list, mod, NULL);
error:
    lys_module_free(mod, NULL);
    if (pctx) {
        ly_set_erase(&pctx->tpdfs_nodes, NULL);
    }
    if (format == LYS_IN_YANG) {
        yang_parser_ctx_free(yangctx);
    } else {
        yin_parser_ctx_free(yinctx);
    }

    return ret;
}

API LY_ERR
lys_parse(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, const char **features, const struct lys_module **module)
{
    if (module) {
        *module = NULL;
    }
    LY_CHECK_ARG_RET(NULL, ctx, in, format > LYS_IN_UNKNOWN, LY_EINVAL);

    /* remember input position */
    in->func_start = in->current;

    return lys_create_module(ctx, in, format, 1, NULL, NULL, features, (struct lys_module **)module);
}

API LY_ERR
lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, const struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, data, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_memory(data, &in), LOGERR(ctx, ret, "Unable to create input handler."), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

API LY_ERR
lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, fd > -1, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_fd(fd, &in), LOGERR(ctx, ret, "Unable to create input handler."), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

API LY_ERR
lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format, const struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, path, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_filepath(path, 0, &in),
            LOGERR(ctx, ret, "Unable to create input handler for filepath %s.", path), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

API LY_ERR
lys_search_localfile(const char * const *searchpaths, ly_bool cwd, const char *name, const char *revision,
        char **localfile, LYS_INFORMAT *format)
{
    LY_ERR ret = LY_EMEM;
    size_t len, flen, match_len = 0, dir_len;
    ly_bool implicit_cwd = 0;
    char *wd, *wn = NULL;
    DIR *dir = NULL;
    struct dirent *file;
    char *match_name = NULL;
    LYS_INFORMAT format_aux, match_format = 0;
    struct ly_set *dirs;
    struct stat st;

    LY_CHECK_ARG_RET(NULL, localfile, LY_EINVAL);

    /* start to fill the dir fifo with the context's search path (if set)
     * and the current working directory */
    LY_CHECK_RET(ly_set_new(&dirs));

    len = strlen(name);
    if (cwd) {
        wd = get_current_dir_name();
        if (!wd) {
            LOGMEM(NULL);
            goto cleanup;
        } else {
            /* add implicit current working directory (./) to be searched,
             * this directory is not searched recursively */
            ret = ly_set_add(dirs, wd, 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
            implicit_cwd = 1;
        }
    }
    if (searchpaths) {
        for (uint64_t i = 0; searchpaths[i]; i++) {
            /* check for duplicities with the implicit current working directory */
            if (implicit_cwd && !strcmp(dirs->objs[0], searchpaths[i])) {
                implicit_cwd = 0;
                continue;
            }
            wd = strdup(searchpaths[i]);
            if (!wd) {
                LOGMEM(NULL);
                goto cleanup;
            } else {
                ret = ly_set_add(dirs, wd, 0, NULL);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }
    }
    wd = NULL;

    /* start searching */
    while (dirs->count) {
        free(wd);
        free(wn); wn = NULL;

        dirs->count--;
        wd = (char *)dirs->objs[dirs->count];
        dirs->objs[dirs->count] = NULL;
        LOGVRB("Searching for \"%s\" in %s.", name, wd);

        if (dir) {
            closedir(dir);
        }
        dir = opendir(wd);
        dir_len = strlen(wd);
        if (!dir) {
            LOGWRN(NULL, "Unable to open directory \"%s\" for searching (sub)modules (%s).", wd, strerror(errno));
        } else {
            while ((file = readdir(dir))) {
                if (!strcmp(".", file->d_name) || !strcmp("..", file->d_name)) {
                    /* skip . and .. */
                    continue;
                }
                free(wn);
                if (asprintf(&wn, "%s/%s", wd, file->d_name) == -1) {
                    LOGMEM(NULL);
                    goto cleanup;
                }
                if (stat(wn, &st) == -1) {
                    LOGWRN(NULL, "Unable to get information about \"%s\" file in \"%s\" when searching for (sub)modules (%s)",
                            file->d_name, wd, strerror(errno));
                    continue;
                }
                if (S_ISDIR(st.st_mode) && (dirs->count || !implicit_cwd)) {
                    /* we have another subdirectory in searchpath to explore,
                     * subdirectories are not taken into account in current working dir (dirs->set.g[0]) */
                    ret = ly_set_add(dirs, wn, 0, NULL);
                    LY_CHECK_GOTO(ret, cleanup);

                    /* continue with the next item in current directory */
                    wn = NULL;
                    continue;
                } else if (!S_ISREG(st.st_mode)) {
                    /* not a regular file (note that we see the target of symlinks instead of symlinks */
                    continue;
                }

                /* here we know that the item is a file which can contain a module */
                if (strncmp(name, file->d_name, len) ||
                        ((file->d_name[len] != '.') && (file->d_name[len] != '@'))) {
                    /* different filename than the module we search for */
                    continue;
                }

                /* get type according to filename suffix */
                flen = strlen(file->d_name);
                if (!strcmp(&file->d_name[flen - 5], ".yang")) {
                    format_aux = LYS_IN_YANG;
                } else if (!strcmp(&file->d_name[flen - 4], ".yin")) {
                    format_aux = LYS_IN_YIN;
                } else {
                    /* not supportde suffix/file format */
                    continue;
                }

                if (revision) {
                    /* we look for the specific revision, try to get it from the filename */
                    if (file->d_name[len] == '@') {
                        /* check revision from the filename */
                        if (strncmp(revision, &file->d_name[len + 1], strlen(revision))) {
                            /* another revision */
                            continue;
                        } else {
                            /* exact revision */
                            free(match_name);
                            match_name = wn;
                            wn = NULL;
                            match_len = dir_len + 1 + len;
                            match_format = format_aux;
                            goto success;
                        }
                    } else {
                        /* continue trying to find exact revision match, use this only if not found */
                        free(match_name);
                        match_name = wn;
                        wn = NULL;
                        match_len = dir_len + 1 + len;
                        match_format = format_aux;
                        continue;
                    }
                } else {
                    /* remember the revision and try to find the newest one */
                    if (match_name) {
                        if ((file->d_name[len] != '@') ||
                                lysp_check_date(NULL, &file->d_name[len + 1], flen - ((format_aux == LYS_IN_YANG) ? 5 : 4) - len - 1, NULL)) {
                            continue;
                        } else if ((match_name[match_len] == '@') &&
                                (strncmp(&match_name[match_len + 1], &file->d_name[len + 1], LY_REV_SIZE - 1) >= 0)) {
                            continue;
                        }
                        free(match_name);
                    }

                    match_name = wn;
                    wn = NULL;
                    match_len = dir_len + 1 + len;
                    match_format = format_aux;
                    continue;
                }
            }
        }
    }

success:
    (*localfile) = match_name;
    match_name = NULL;
    if (format) {
        (*format) = match_format;
    }
    ret = LY_SUCCESS;

cleanup:
    free(wn);
    free(wd);
    if (dir) {
        closedir(dir);
    }
    free(match_name);
    ly_set_free(dirs, free);

    return ret;
}
