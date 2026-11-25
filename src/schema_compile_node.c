/**
 * @file schema_compile_node.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema compilation of common nodes.
 *
 * Copyright (c) 2015 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include "schema_compile_node.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "dict.h"
#include "log.h"
#include "ly_array.h"
#include "ly_common.h"
#include "parser_internal.h"
#include "plugins.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "schema_compile.h"
#include "schema_compile_amend.h"
#include "schema_compile_type.h"
#include "schema_features.h"
#include "set.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "utils.h"
#include "xpath.h"

/**
 * @brief Add a node with a when to unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] when Specific compiled when to check.
 * @param[in] node Compiled node with when(s).
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_when_add(struct lysc_ctx *ctx, struct lysc_when *when, struct lysc_node *node)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_unres_when *w = NULL;

    /* do not check when(s) in a grouping or in disabled data (high risk of false-positives) */
    if (ctx->compile_opts & (LYS_COMPILE_GROUPING | LYS_COMPILE_DISABLED | LYS_COMPILE_LOCAL_ONLY)) {
        goto cleanup;
    }

    /* add new unres when */
    w = calloc(1, sizeof *w);
    LY_CHECK_ERR_GOTO(!w, LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);

    w->node = node;
    w->when = when;

    /* add into the unres set */
    LY_CHECK_ERR_GOTO(ly_set_add(&ctx->unres->whens, w, 1, NULL), LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);
    w = NULL;

cleanup:
    free(w);
    return rc;
}

/**
 * @brief Add a node with must(s) to unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Compiled node with must(s).
 * @param[in] pnode Parsed ndoe with must(s).
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_must_add(struct lysc_ctx *ctx, struct lysc_node *node, struct lysp_node *pnode)
{
    struct lysc_unres_must *m = NULL;
    LYA_COUNT_T u;
    struct lysc_must *musts;
    struct lysp_restr *pmusts;
    LY_ERR ret;

    /* do not check must(s) in a grouping or in disabled data (high risk of false-positives) */
    if (ctx->compile_opts & (LYS_COMPILE_GROUPING | LYS_COMPILE_DISABLED | LYS_COMPILE_LOCAL_ONLY)) {
        return LY_SUCCESS;
    }

    musts = lysc_node_musts(node);
    pmusts = lysp_node_musts(pnode);
    assert(LYA_COUNT(musts) == LYA_COUNT(pmusts));

    if (!musts) {
        /* no must */
        return LY_SUCCESS;
    }

    /* add new unres must */
    m = calloc(1, sizeof *m);
    LY_CHECK_ERR_GOTO(!m, ret = LY_EMEM, error);
    m->node = node;

    /* add must local modules */
    LYA_PREALLOC(m->local_mods, LYA_COUNT(pmusts), LOGMEM(ctx->ctx); ret = LY_EMEM; goto error);
    LYA_FOR(pmusts, u) {
        m->local_mods[u] = pmusts[u].arg.mod;
        LYA_INCREMENT(m->local_mods);
    }

    /* store ext */
    m->ext = ctx->ext;

    LY_CHECK_ERR_GOTO(ly_set_add(&ctx->unres->musts, m, 1, NULL), ret = LY_EMEM, error);

    return LY_SUCCESS;

error:
    if (m) {
        LYA_FREE(m->local_mods);
        free(m);
    }
    LOGMEM(ctx->ctx);
    return ret;
}

static LY_ERR
lysc_unres_leafref_add(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf, const struct lysp_module *local_mod)
{
    struct lysc_unres_leafref *l = NULL;
    struct ly_set *leafrefs_set;
    LYA_COUNT_T u;
    int is_lref = 0;

    if (ctx->compile_opts & (LYS_COMPILE_GROUPING | LYS_COMPILE_LOCAL_ONLY)) {
        /* do not check leafrefs in groupings */
        return LY_SUCCESS;
    }

    /* use special set for disabled leafrefs */
    leafrefs_set = ctx->compile_opts & LYS_COMPILE_DISABLED ? &ctx->unres->disabled_leafrefs : &ctx->unres->leafrefs;

    if (leaf->type->basetype == LY_TYPE_LEAFREF) {
        /* leafref */
        is_lref = 1;
    } else if (leaf->type->basetype == LY_TYPE_UNION) {
        /* union with leafrefs */
        LYA_FOR(((struct lysc_type_union *)leaf->type)->types, u) {
            if (((struct lysc_type_union *)leaf->type)->types[u]->basetype == LY_TYPE_LEAFREF) {
                is_lref = 1;
                break;
            }
        }
    }

    if (is_lref) {
        /* add new unresolved leafref node */
        l = calloc(1, sizeof *l);
        LY_CHECK_ERR_RET(!l, LOGMEM(ctx->ctx), LY_EMEM);

        l->node = &leaf->node;
        l->local_mod = local_mod;
        l->ext = ctx->ext;

        LY_CHECK_ERR_RET(ly_set_add(leafrefs_set, l, 1, NULL), free(l); LOGMEM(ctx->ctx), LY_EMEM);
    }

    return LY_SUCCESS;
}

/**
 * @brief Add/replace a leaf default value in unres.
 * Can also be used for a single leaf-list default value.
 *
 * @param[in] ctx Compile context.
 * @param[in] leaf Leaf with the default value.
 * @param[in] dflt Default value to use.
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_leaf_dflt_add(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf, struct lysp_qname *dflt)
{
    struct lysc_unres_dflt *r = NULL;
    uint32_t i;
    LY_VALUE_FORMAT format;

    if (ctx->compile_opts & (LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING)) {
        return LY_SUCCESS;
    }

    if (ctx->compile_opts & LYS_COMPILE_LOCAL_ONLY) {
        /* store the local value with the resolved prefixes */
        LY_CHECK_RET(lysdict_insert(ctx->ctx, dflt->str, 0, &leaf->dflt.str));
        LY_CHECK_RET(lyplg_type_prefix_data_new(ctx->ctx, dflt->str, strlen(dflt->str), LY_VALUE_SCHEMA, ctx->pmod,
                &format, (void **)&leaf->dflt.prefixes));
        return LY_SUCCESS;
    }

    for (i = 0; i < ctx->unres->dflts.count; ++i) {
        if (((struct lysc_unres_dflt *)ctx->unres->dflts.objs[i])->leaf == leaf) {
            /* just replace the default */
            r = ctx->unres->dflts.objs[i];
            lysp_qname_free(ctx->ctx, r->dflt);
            free(r->dflt);
            break;
        }
    }
    if (!r) {
        /* add new unres item */
        r = calloc(1, sizeof *r);
        LY_CHECK_ERR_RET(!r, LOGMEM(ctx->ctx), LY_EMEM);
        r->leaf = leaf;

        LY_CHECK_RET(ly_set_add(&ctx->unres->dflts, r, 1, NULL));
    }

    r->dflt = malloc(sizeof *r->dflt);
    LY_CHECK_GOTO(!r->dflt, error);
    LY_CHECK_GOTO(lysp_qname_dup(ctx->ctx, dflt, r->dflt), error);

    return LY_SUCCESS;

error:
    free(r->dflt);
    LOGMEM(ctx->ctx);
    return LY_EMEM;
}

/**
 * @brief Add/replace a leaf-list default value(s) in unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] llist Leaf-list with the default value.
 * @param[in] dflts Sized array of the default values.
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_llist_dflts_add(struct lysc_ctx *ctx, struct lysc_node_leaflist *llist, struct lysp_qname *dflts)
{
    struct lysc_unres_dflt *r = NULL;
    uint32_t i;
    LYA_COUNT_T u;
    LY_VALUE_FORMAT format;

    if (ctx->compile_opts & (LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING)) {
        return LY_SUCCESS;
    }

    if (ctx->compile_opts & LYS_COMPILE_LOCAL_ONLY) {
        /* store the local value with the resolved prefixes */
        LYA_PREALLOC(llist->dflts, LYA_COUNT(dflts), LOGMEM(ctx->ctx); return LY_EMEM);
        LYA_FOR(dflts, u) {
            LY_CHECK_RET(lysdict_insert(ctx->ctx, dflts[u].str, 0, &llist->dflts[u].str));
            LY_CHECK_RET(lyplg_type_prefix_data_new(ctx->ctx, dflts[u].str, strlen(dflts[u].str), LY_VALUE_SCHEMA,
                    ctx->pmod, &format, (void **)&llist->dflts[u].prefixes));
        }
        return LY_SUCCESS;
    }

    for (i = 0; i < ctx->unres->dflts.count; ++i) {
        if (((struct lysc_unres_dflt *)ctx->unres->dflts.objs[i])->llist == llist) {
            /* just replace the defaults */
            r = ctx->unres->dflts.objs[i];
            lysp_qname_free(ctx->ctx, r->dflt);
            free(r->dflt);
            r->dflt = NULL;
            FREE_ARRAY(ctx->ctx, r->dflts, lysp_qname_free);
            r->dflts = NULL;
            break;
        }
    }
    if (!r) {
        r = calloc(1, sizeof *r);
        LY_CHECK_ERR_RET(!r, LOGMEM(ctx->ctx), LY_EMEM);
        r->llist = llist;

        LY_CHECK_RET(ly_set_add(&ctx->unres->dflts, r, 1, NULL));
    }

    DUP_ARRAY(ctx->ctx, dflts, r->dflts, lysp_qname_dup);

    return LY_SUCCESS;
}

/**
 * @brief Add a bits/enumeration type to unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] leaf Leaf of type bits/enumeration whose disabled items to free.
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_bitenum_add(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf)
{
    if (ctx->compile_opts & (LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING | LYS_COMPILE_LOCAL_ONLY)) {
        /* skip groupings and redundant for disabled nodes */
        return LY_SUCCESS;
    }

    LY_CHECK_RET(ly_set_add(&ctx->unres->disabled_bitenums, leaf, 1, NULL));

    return LY_SUCCESS;
}

/**
 * @brief Print status into a string.
 *
 * @param[in] flags Flags with the status to print.
 * @return String status.
 */
static const char *
lys_status2str(uint16_t flags)
{
    flags &= LYS_STATUS_MASK;

    switch (flags) {
    case 0:
    case LYS_STATUS_CURR:
        return "current";
    case LYS_STATUS_DEPRC:
        return "deprecated";
    case LYS_STATUS_OBSLT:
        return "obsolete";
    default:
        LOGINT(NULL);
        return NULL;
    }
}

LY_ERR
lys_compile_status(struct lysc_ctx *ctx, uint16_t parsed_flags, uint16_t inherited_flags, uint16_t parent_flags,
        const char *parent_name, const char *stmt_name, uint16_t *stmt_flags)
{
    /* normalize to status-only */
    parsed_flags &= LYS_STATUS_MASK;
    inherited_flags &= LYS_STATUS_MASK;
    parent_flags &= LYS_STATUS_MASK;

    /* check for conflicts */
    if (parent_flags && parsed_flags && (parent_flags > parsed_flags)) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Status \"%s\" of \"%s\" is in conflict with \"%s\" status of parent \"%s\".",
                lys_status2str(parsed_flags), stmt_name, lys_status2str(parent_flags), parent_name);
        return LY_EVALID;
    } else if (inherited_flags && parsed_flags && (inherited_flags > parsed_flags)) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Inherited schema-only status \"%s\" is in conflict with \"%s\" status of \"%s\".",
                lys_status2str(inherited_flags), lys_status2str(parsed_flags), stmt_name);
        return LY_EVALID;
    } else if (parent_flags && inherited_flags && (parent_flags > inherited_flags)) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                "Status \"%s\" of parent \"%s\" is in conflict with inherited schema-only status \"%s\".",
                lys_status2str(parent_flags), parent_name, lys_status2str(inherited_flags));
        return LY_EVALID;
    }

    /* clear */
    (*stmt_flags) &= ~LYS_STATUS_MASK;

    if (parsed_flags) {
        /* explicit status */
        (*stmt_flags) |= parsed_flags;
    } else if (inherited_flags) {
        /* inherited status from a schema-only statement */
        (*stmt_flags) |= inherited_flags;
    } else if (parent_flags) {
        /* inherited status from a parent node */
        (*stmt_flags) |= parent_flags;
    } else {
        /* default status */
        (*stmt_flags) |= LYS_STATUS_CURR;
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile information from the when statement
 *
 * @param[in] ctx Compile context.
 * @param[in] when_p Parsed when structure.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in] parent Parent node, if any.
 * @param[in] ctx_node Context node for the when statement.
 * @param[out] when Pointer where to store pointer to the created compiled when structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_when_(struct lysc_ctx *ctx, const struct lysp_when *when_p, uint16_t inherited_flags,
        const struct lysc_node *parent, const struct lysc_node *ctx_node, struct lysc_when **when)
{
    LY_ERR ret = LY_SUCCESS;
    LY_VALUE_FORMAT format;

    *when = calloc(1, sizeof **when);
    LY_CHECK_ERR_RET(!(*when), LOGMEM(ctx->ctx), LY_EMEM);
    (*when)->refcount = 1;
    LY_CHECK_RET(lyxp_expr_parse(ctx->ctx, NULL, when_p->cond, 0, 1, &(*when)->cond));
    LY_CHECK_RET(lyplg_type_prefix_data_new(ctx->ctx, when_p->cond, strlen(when_p->cond),
            LY_VALUE_SCHEMA, ctx->pmod, &format, (void **)&(*when)->prefixes));
    (*when)->context = (struct lysc_node *)ctx_node;
    DUP_STRING_GOTO(ctx->ctx, when_p->dsc, (*when)->dsc, ret, done);
    DUP_STRING_GOTO(ctx->ctx, when_p->ref, (*when)->ref, ret, done);
    COMPILE_EXTS_GOTO(ctx, when_p->exts, (*when)->exts, (*when), ret, done);
    LY_CHECK_RET(lys_compile_status(ctx, 0, inherited_flags, parent ? parent->flags : 0, parent ? parent->name : NULL,
            "when", &(*when)->flags));

done:
    return ret;
}

LY_ERR
lys_compile_when(struct lysc_ctx *ctx, const struct lysp_when *when_p, uint16_t inherited_flags, const struct lysc_node *parent,
        const struct lysc_node *ctx_node, struct lysc_node *node, struct lysc_when **when_c)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_when **new_when, ***node_when, *ptr;

    assert(when_p && (node || when_c));

    if (node) {
        /* get the when array */
        node_when = lysc_node_when_p(node);

        /* create new when pointer */
        LYA_ADD_ITEM(*node_when, new_when, LOGMEM(ctx->ctx); rc = LY_EMEM; goto cleanup);
    } else {
        /* individual when */
        new_when = &ptr;
        *new_when = calloc(1, sizeof **new_when);
        LY_CHECK_ERR_GOTO(!*new_when, LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);
    }

    if (!when_c || !(*when_c)) {
        /* compile when */
        LY_CHECK_GOTO(rc = lys_compile_when_(ctx, when_p, inherited_flags, parent, ctx_node, new_when), cleanup);

        /* remember the compiled when for sharing */
        if (when_c) {
            *when_c = *new_when;
        }
    } else {
        /* use the previously compiled when */
        ++(*when_c)->refcount;
        *new_when = *when_c;
    }

    if (node) {
        /* add when to unres if there is a node for evaluation (not for extension instances) */
        LY_CHECK_GOTO(rc = lysc_unres_when_add(ctx, *new_when, node), cleanup);
    }

cleanup:
    return rc;
}

LY_ERR
lys_compile_must(struct lysc_ctx *ctx, const struct lysp_restr *must_p, struct lysc_must *must)
{
    LY_ERR ret = LY_SUCCESS;
    LY_VALUE_FORMAT format;

    LY_CHECK_RET(lyxp_expr_parse(ctx->ctx, NULL, must_p->arg.str, 0, 1, &must->cond));
    LY_CHECK_RET(lyplg_type_prefix_data_new(ctx->ctx, must_p->arg.str, strlen(must_p->arg.str),
            LY_VALUE_SCHEMA, must_p->arg.mod, &format, (void **)&must->prefixes));
    DUP_STRING_GOTO(ctx->ctx, must_p->eapptag, must->eapptag, ret, done);
    DUP_STRING_GOTO(ctx->ctx, must_p->emsg, must->emsg, ret, done);
    DUP_STRING_GOTO(ctx->ctx, must_p->dsc, must->dsc, ret, done);
    DUP_STRING_GOTO(ctx->ctx, must_p->ref, must->ref, ret, done);
    COMPILE_EXTS_GOTO(ctx, must_p->exts, must->exts, must, ret, done);

done:
    return ret;
}

/**
 * @brief Check uniqness of the node/action/notification name.
 *
 * Data nodes, actions/RPCs and Notifications are stored separately (in distinguish lists) in the schema
 * structures, but they share the namespace so we need to check their name collisions.
 *
 * @param[in] ctx Compile context.
 * @param[in] parent Parent of the nodes to check, can be NULL.
 * @param[in] name Name of the item to find in the given lists.
 * @param[in] exclude Node that was just added that should be excluded from the name checking.
 * @return LY_SUCCESS in case of unique name, LY_EEXIST otherwise.
 */
static LY_ERR
lys_compile_node_uniqness(struct lysc_ctx *ctx, const struct lysc_node *parent, const char *name,
        const struct lysc_node *exclude)
{
    const struct lysc_node *iter, *iter2, *dup = NULL, **node_list_p;
    const struct lysc_node_action *actions;
    const struct lysc_node_notif *notifs;
    uint32_t getnext_flags;
    struct ly_set parent_choices = {0};
    const char *node_type_str = "data definition/RPC/action/notification";
    char *spath;

#define CHECK_NODE(iter, exclude, name) (iter != (void *)exclude && (iter)->module == exclude->module && !strcmp(name, (iter)->name))

    if (exclude->nodetype == LYS_CASE) {
        /* check restricted only to all the cases */
        assert(parent->nodetype == LYS_CHOICE);
        LY_LIST_FOR(lysc_node_child(parent), iter) {
            if (CHECK_NODE(iter, exclude, name)) {
                node_type_str = "case";
                dup = iter;
                goto cleanup;
            }
        }

        return LY_SUCCESS;
    }

    /* no reason for our parent to be choice anymore */
    assert(!parent || (parent->nodetype != LYS_CHOICE));

    if (parent && (parent->nodetype == LYS_CASE)) {
        /* move to the first data definition parent */

        /* but remember the choice nodes on the parents path to avoid believe they collide with our node */
        iter = lysc_data_parent(parent);
        do {
            parent = parent->parent;
            if (parent && (parent->nodetype == LYS_CHOICE)) {
                ly_set_add(&parent_choices, (void *)parent, 1, NULL);
            }
        } while (parent != iter);
    }

    getnext_flags = LYS_GETNEXT_WITHCHOICE;
    if (parent && (parent->nodetype & (LYS_RPC | LYS_ACTION))) {
        /* move to the inout to avoid traversing a not-filled-yet (the other) node */
        if (exclude->flags & LYS_IS_OUTPUT) {
            getnext_flags |= LYS_GETNEXT_OUTPUT;
            parent = lysc_node_child(parent)->next;
        } else {
            parent = lysc_node_child(parent);
        }
    }

    iter = NULL;
    if (!parent && ctx->ext) {
        lyplg_ext_get_storage_p(ctx->ext, LY_STMT_DATA_NODE_MASK, (void ***)&node_list_p);
        iter = node_list_p ? *node_list_p : NULL;
        while (iter) {
            if (!ly_set_contains(&parent_choices, (void *)iter, NULL) && CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }

            /* we must compare with both the choice and all its nested data-definiition nodes (but not recursively) */
            if (iter->nodetype == LYS_CHOICE) {
                iter2 = NULL;
                while ((iter2 = lys_getnext(iter2, iter, NULL, 0))) {
                    if (CHECK_NODE(iter2, exclude, name)) {
                        dup = iter2;
                        goto cleanup;
                    }
                }
            }

            /* next iter */
            iter = lys_getnext(iter, parent, NULL, getnext_flags);
        }
    } else {
        while ((iter = lys_getnext(iter, parent, ctx->cmod, getnext_flags))) {
            if (!ly_set_contains(&parent_choices, (void *)iter, NULL) && CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }

            /* we must compare with both the choice and all its nested data-definiition nodes (but not recursively) */
            if (iter->nodetype == LYS_CHOICE) {
                iter2 = NULL;
                while ((iter2 = lys_getnext(iter2, iter, NULL, 0))) {
                    if (CHECK_NODE(iter2, exclude, name)) {
                        dup = iter2;
                        goto cleanup;
                    }
                }
            }
        }

        actions = parent ? lysc_node_actions(parent) : ctx->cmod->rpcs;
        LY_LIST_FOR((struct lysc_node *)actions, iter) {
            if (CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }
        }

        notifs = parent ? lysc_node_notifs(parent) : ctx->cmod->notifs;
        LY_LIST_FOR((struct lysc_node *)notifs, iter) {
            if (CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }
        }
    }

cleanup:
    ly_set_erase(&parent_choices, NULL);
    if (dup) {
        spath = lysc_path(dup, LYSC_PATH_LOG, NULL, 0);
        LOGVAL(ctx->ctx, NULL, LY_VCODE_DUPIDENT, spath, node_type_str);
        free(spath);
        return LY_EEXIST;
    }
    return LY_SUCCESS;

#undef CHECK_NODE
}

LY_ERR
lys_compile_node_connect(struct lysc_ctx *ctx, struct lysc_node *parent, struct lysc_node *node)
{
    struct lysc_node **children, *anchor = NULL;
    int insert_after = 0;

    node->parent = parent;

    if (parent) {
        if (node->nodetype == LYS_INPUT) {
            assert(parent->nodetype & (LYS_ACTION | LYS_RPC));
            /* input node is part of the action but link it with output */
            node->next = &((struct lysc_node_action *)parent)->output.node;
            node->prev = node->next;
            return LY_SUCCESS;
        } else if (node->nodetype == LYS_OUTPUT) {
            /* output node is part of the action but link it with input */
            node->next = NULL;
            node->prev = &((struct lysc_node_action *)parent)->input.node;
            return LY_SUCCESS;
        } else if (node->nodetype == LYS_ACTION) {
            children = (struct lysc_node **)lysc_node_actions_p(parent);
        } else if (node->nodetype == LYS_NOTIF) {
            children = (struct lysc_node **)lysc_node_notifs_p(parent);
        } else {
            children = lysc_node_child_p(parent);
        }
        assert(children);

        if (!(*children)) {
            /* first child */
            *children = node;
        } else if (node->flags & LYS_KEY) {
            /* special handling of adding keys */
            assert(node->module == parent->module);
            anchor = *children;
            if (anchor->flags & LYS_KEY) {
                while ((anchor->flags & LYS_KEY) && anchor->next) {
                    anchor = anchor->next;
                }
                /* insert after the last key */
                insert_after = 1;
            } /* else insert before anchor (at the beginning) */
        } else if ((*children)->prev->module == node->module) {
            /* last child is from the same module, keep the order and insert at the end */
            anchor = (*children)->prev;
            insert_after = 1;
        } else if (parent->module == node->module) {
            /* adding module child after some augments were connected */
            for (anchor = *children; anchor->module == node->module; anchor = anchor->next) {}
        } else {
            /* some augments are already connected and we are connecting new ones,
             * keep module name order and insert the node into the children list */
            anchor = *children;
            do {
                anchor = anchor->prev;

                /* check that we have not found the last augment node from our module or
                 * the first augment node from a "smaller" module or
                 * the first node from a local module */
                if ((anchor->module == node->module) || (strcmp(anchor->module->name, node->module->name) < 0) ||
                        (anchor->module == parent->module)) {
                    /* insert after */
                    insert_after = 1;
                    break;
                }

                /* we have traversed all the nodes, insert before anchor (as the first node) */
            } while (anchor->prev->next);
        }

        /* insert */
        if (anchor) {
            if (insert_after) {
                node->next = anchor->next;
                node->prev = anchor;
                anchor->next = node;
                if (node->next) {
                    /* middle node */
                    node->next->prev = node;
                } else {
                    /* last node */
                    (*children)->prev = node;
                }
            } else {
                node->next = anchor;
                node->prev = anchor->prev;
                anchor->prev = node;
                if (anchor == *children) {
                    /* first node */
                    *children = node;
                } else {
                    /* middle node */
                    node->prev->next = node;
                }
            }
        }

        /* check the name uniqueness (even for an only child, it may be in case) */
        if (lys_compile_node_uniqness(ctx, parent, node->name, node)) {
            return LY_EEXIST;
        }
    } else {
        /* top-level element */
        struct lysc_node **list = NULL;

        if (ctx->ext) {
            lyplg_ext_get_storage_p(ctx->ext, LY_STMT_DATA_NODE_MASK, (void ***)&list);
        } else if (node->nodetype == LYS_RPC) {
            list = (struct lysc_node **)&ctx->cmod->rpcs;
        } else if (node->nodetype == LYS_NOTIF) {
            list = (struct lysc_node **)&ctx->cmod->notifs;
        } else {
            list = &ctx->cmod->data;
        }
        if (!(*list)) {
            *list = node;
        } else {
            /* insert at the end of the module's top-level nodes list */
            (*list)->prev->next = node;
            node->prev = (*list)->prev;
            (*list)->prev = node;
        }

        /* check the name uniqueness on top-level */
        if (lys_compile_node_uniqness(ctx, NULL, node->name, node)) {
            return LY_EEXIST;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Set config and operation flags for a node.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Compiled node flags to set.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_config(struct lysc_ctx *ctx, struct lysc_node *node)
{
    /* case never has any explicit config */
    assert((node->nodetype != LYS_CASE) || !(node->flags & LYS_CONFIG_MASK));

    if (ctx->compile_opts & LYS_COMPILE_NO_CONFIG) {
        /* ignore config statements inside Notification/RPC/action/... data */
        node->flags &= ~LYS_CONFIG_MASK;
    } else if (!(node->flags & LYS_CONFIG_MASK)) {
        /* config not explicitly set, inherit it from parent */
        assert(!node->parent || (node->parent->flags & LYS_CONFIG_MASK) || (node->parent->nodetype & LYS_AUGMENT));
        if (node->parent && (node->parent->flags & LYS_CONFIG_MASK)) {
            node->flags |= node->parent->flags & LYS_CONFIG_MASK;
        } else {
            /* default is config true */
            node->flags |= LYS_CONFIG_W;
        }
    } else {
        /* config set explicitly */
        node->flags |= LYS_SET_CONFIG;
    }

    if (node->parent && (node->parent->flags & LYS_CONFIG_R) && (node->flags & LYS_CONFIG_W)) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Configuration node cannot be child of any state data node.");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Set various flags of the compiled nodes
 *
 * @param[in] ctx Compile context.
 * @param[in] parsed_flags Parsed node flags.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in,out] node Compiled node where the flags will be set.
 */
static LY_ERR
lys_compile_node_flags(struct lysc_ctx *ctx, uint16_t parsed_flags, uint16_t inherited_flags, struct lysc_node *node)
{
    uint16_t parent_flags;
    const char *parent_name;

    /* copy flags except for status */
    node->flags = (parsed_flags & LYS_FLAGS_COMPILED_MASK) & ~LYS_STATUS_MASK;

    /* inherit config flags */
    LY_CHECK_RET(lys_compile_config(ctx, node));

    /* compile status */
    parent_flags = node->parent ? node->parent->flags : 0;
    parent_name = node->parent ? node->parent->name : NULL;
    LY_CHECK_RET(lys_compile_status(ctx, parsed_flags, inherited_flags, parent_flags, parent_name, node->name, &node->flags));

    /* other flags */
    if ((ctx->compile_opts & LYS_IS_INPUT) && (node->nodetype != LYS_INPUT)) {
        node->flags |= LYS_IS_INPUT;
    } else if ((ctx->compile_opts & LYS_IS_OUTPUT) && (node->nodetype != LYS_OUTPUT)) {
        node->flags |= LYS_IS_OUTPUT;
    } else if ((ctx->compile_opts & LYS_IS_NOTIF) && (node->nodetype != LYS_NOTIF)) {
        node->flags |= LYS_IS_NOTIF;
    }

    return LY_SUCCESS;
}

static LY_ERR
lys_compile_node_(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *parent, uint16_t inherited_flags,
        LY_ERR (*node_compile_spec)(struct lysc_ctx *, struct lysp_node *, struct lysc_node *),
        struct lysc_node *node, struct ly_set *child_set)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool not_supported, enabled;
    struct lysp_node *dev_pnode = NULL;
    struct lysp_when *pwhen = NULL;
    uint32_t prev_opts = ctx->compile_opts;

    node->nodetype = pnode->nodetype;
    node->module = ctx->cur_mod;
    node->parent = parent;
    node->prev = node;
    node->priv = ctx->ctx->opts & LY_CTX_SET_PRIV_PARSED ? pnode : NULL;

    /* compile any deviations for this node */
    LY_CHECK_GOTO(ret = lys_compile_node_deviations_refines(ctx, pnode, parent, &dev_pnode, &not_supported), error);
    if (not_supported && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING |
            LYS_COMPILE_LOCAL_ONLY))) {
        /* if not supported, keep it just like disabled nodes by if-feature */
        ly_set_add(&ctx->unres->disabled, node, 1, NULL);
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
    }
    if (dev_pnode) {
        pnode = dev_pnode;
    }

    DUP_STRING_GOTO(ctx->ctx, pnode->name, node->name, ret, error);
    DUP_STRING_GOTO(ctx->ctx, pnode->dsc, node->dsc, ret, error);
    DUP_STRING_GOTO(ctx->ctx, pnode->ref, node->ref, ret, error);

    /* if-features */
    LY_CHECK_GOTO(ret = lys_eval_iffeatures(ctx->ctx, pnode->iffeatures, &enabled), error);
    if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING |
            LYS_COMPILE_LOCAL_ONLY))) {
        ly_set_add(&ctx->unres->disabled, node, 1, NULL);
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
    }

    /* config, status and other flags */
    LY_CHECK_GOTO(ret = lys_compile_node_flags(ctx, pnode->flags, inherited_flags, node), error);
    if ((node->flags & LYS_STATUS_OBSLT) && !(ctx->ctx->opts & LY_CTX_COMPILE_OBSOLETE) &&
            !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING | LYS_COMPILE_LOCAL_ONLY))) {
        /* obsolete, will not be in the compiled tree, treat as disabled */
        ly_set_add(&ctx->unres->disabled, node, 1, NULL);
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
    }

    /* list ordering */
    if (node->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        if ((node->flags & (LYS_CONFIG_R | LYS_IS_OUTPUT | LYS_IS_NOTIF)) && (node->flags & LYS_ORDBY_MASK)) {
            node->flags &= ~LYS_ORDBY_MASK;
            LOGVRB("The ordered-by statement is ignored in lists representing %s (%s).",
                    (node->flags & LYS_IS_OUTPUT) ? "RPC/action output parameters" :
                    (ctx->compile_opts & LYS_IS_NOTIF) ? "notification content" : "state data", ctx->path);
        }
        if (node->flags & (LYS_IS_OUTPUT | LYS_IS_NOTIF | LYS_CONFIG_R)) {
            /* it is probably better not to order them */
            node->flags |= LYS_ORDBY_USER;
        } else if (!(node->flags & LYS_ORDBY_MASK)) {
            /* default ordering is system */
            node->flags |= LYS_ORDBY_SYSTEM;
        }
    }

    /* insert into parent's children/compiled module (we can no longer free the node separately on error) */
    LY_CHECK_GOTO(ret = lys_compile_node_connect(ctx, parent, node), cleanup);

    if ((pwhen = lysp_node_when(pnode))) {
        /* compile when */
        ret = lys_compile_when(ctx, pwhen, pnode->flags, node, lysc_data_node(node), node, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* nodetype-specific part */
    LY_CHECK_GOTO(ret = node_compile_spec(ctx, pnode, node), cleanup);

    /* final compilation tasks that require the node to be connected */
    COMPILE_EXTS_GOTO(ctx, pnode->exts, node->exts, node, ret, cleanup);
    if (node->flags & LYS_MAND_TRUE) {
        /* inherit LYS_MAND_TRUE in parent containers */
        lys_compile_mandatory_parents(parent, 1);
    }

    if (child_set) {
        /* add the new node into set */
        LY_CHECK_GOTO(ret = ly_set_add(child_set, node, 1, NULL), cleanup);
    }

    goto cleanup;

error:
    lysc_node_free(ctx->ctx, node, 0);

cleanup:
    if (ret && dev_pnode) {
        LOGVAL(ctx->ctx, NULL, LYVE_OTHER, "Compilation of a deviated and/or refined node failed.");
    }
    ctx->compile_opts = prev_opts;
    lysp_dev_node_free(ctx, dev_pnode);
    return ret;
}

LY_ERR
lys_compile_node_action_inout(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *child_p;
    uint32_t prev_options = ctx->compile_opts;

    struct lysp_node_action_inout *inout_p = (struct lysp_node_action_inout *)pnode;
    struct lysc_node_action_inout *inout = (struct lysc_node_action_inout *)node;

    COMPILE_ARRAY_GOTO(ctx, inout_p->musts, inout->musts, lys_compile_must, ret, done);
    ctx->compile_opts |= (inout_p->nodetype == LYS_INPUT) ? LYS_COMPILE_RPC_INPUT : LYS_COMPILE_RPC_OUTPUT;

    LY_LIST_FOR(inout_p->child, child_p) {
        LY_CHECK_GOTO(ret = lys_compile_node(ctx, child_p, node, 0, NULL), done);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    ctx->compile_opts = prev_options;

done:
    return ret;
}

/**
 * @brief Compile parsed action node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed action node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the action-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_action(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret;
    struct lysp_node_action *action_p = (struct lysp_node_action *)pnode;
    struct lysc_node_action *action = (struct lysc_node_action *)node;
    struct lysp_node_action_inout *input, implicit_input = {
        .nodetype = LYS_INPUT,
        .name = "input",
        .parent = pnode,
    };
    struct lysp_node_action_inout *output, implicit_output = {
        .nodetype = LYS_OUTPUT,
        .name = "output",
        .parent = pnode,
    };

    /* input */
    lysc_update_path(ctx, action->module, "input");
    if (action_p->input.nodetype == LYS_UNKNOWN) {
        input = &implicit_input;
    } else {
        input = &action_p->input;
    }
    ret = lys_compile_node_(ctx, &input->node, &action->node, 0, lys_compile_node_action_inout, &action->input.node, NULL);
    lysc_update_path(ctx, NULL, NULL);
    LY_CHECK_GOTO(ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, &action->input.node, &input->node);
    LY_CHECK_GOTO(ret, done);

    /* output */
    lysc_update_path(ctx, action->module, "output");
    if (action_p->output.nodetype == LYS_UNKNOWN) {
        output = &implicit_output;
    } else {
        output = &action_p->output;
    }
    ret = lys_compile_node_(ctx, &output->node, &action->node, 0, lys_compile_node_action_inout, &action->output.node, NULL);
    lysc_update_path(ctx, NULL, NULL);
    LY_CHECK_GOTO(ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, &action->output.node, &output->node);
    LY_CHECK_GOTO(ret, done);

done:
    return ret;
}

/**
 * @brief Compile parsed action node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed action node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the action-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_notif(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node_notif *notif_p = (struct lysp_node_notif *)pnode;
    struct lysc_node_notif *notif = (struct lysc_node_notif *)node;
    struct lysp_node *child_p;

    COMPILE_ARRAY_GOTO(ctx, notif_p->musts, notif->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    LY_LIST_FOR(notif_p->child, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

done:
    return ret;
}

/**
 * @brief Compile parsed container node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed container node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the container-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_container(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_container *cont_p = (struct lysp_node_container *)pnode;
    struct lysc_node_container *cont = (struct lysc_node_container *)node;
    struct lysp_node *child_p;
    LY_ERR ret = LY_SUCCESS;

    if (cont_p->presence) {
        /* presence container */
        cont->flags |= LYS_PRESENCE;
    }

    /* more cases when the container has meaning but is kept NP for convenience:
     *   - when condition
     *   - direct child action/notification
     */

    LY_LIST_FOR(cont_p->child, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

    COMPILE_ARRAY_GOTO(ctx, cont_p->musts, cont->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    LY_LIST_FOR((struct lysp_node *)cont_p->actions, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }
    LY_LIST_FOR((struct lysp_node *)cont_p->notifs, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

done:
    return ret;
}

/**
 * @brief Compile type in leaf/leaf-list node and do all the necessary checks.
 * @param[in] ctx Compile context.
 * @param[in] context_node Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] type_p Parsed type to compile.
 * @param[in,out] leaf Compiled leaf structure (possibly cast leaf-list) to provide node information and to store the compiled type information.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_node_type(struct lysc_ctx *ctx, struct lysp_node *context_node, struct lysp_type *type_p,
        struct lysc_node_leaf *leaf)
{
    struct lysp_qname *dflt;
    struct lysc_type **t;
    LYA_COUNT_T u, count;
    ly_bool in_unres = 0;

    LY_CHECK_RET(lys_compile_type(ctx, context_node, leaf->flags, leaf->name, type_p, &leaf->type,
            leaf->units ? NULL : &leaf->units, &dflt));
    if (!leaf->type) {
        /* foreign typedef in local-only module */
        return LY_SUCCESS;
    }

    LY_ATOMIC_INC_BARRIER(leaf->type->refcount);

    /* store default value, if any */
    if (dflt && !(leaf->flags & LYS_SET_DFLT)) {
        LY_CHECK_RET(lysc_unres_leaf_dflt_add(ctx, leaf, dflt));
    }

    /* store leafref(s) to be resolved */
    LY_CHECK_RET(lysc_unres_leafref_add(ctx, leaf, type_p->pmod));

    /* type-specific checks */
    if (leaf->type->basetype == LY_TYPE_UNION) {
        t = ((struct lysc_type_union *)leaf->type)->types;
        count = LYA_COUNT(t);
    } else {
        t = &leaf->type;
        count = 1;
    }
    for (u = 0; u < count; ++u) {
        if (t[u]->basetype == LY_TYPE_EMPTY) {
            if ((leaf->nodetype == LYS_LEAFLIST) && (ctx->pmod->version < LYS_VERSION_1_1)) {
                LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules.");
                return LY_EVALID;
            }
        } else if (!in_unres && ((t[u]->basetype == LY_TYPE_BITS) || (t[u]->basetype == LY_TYPE_ENUM))) {
            /* store in unres for all disabled bits/enums to be removed */
            LY_CHECK_RET(lysc_unres_bitenum_add(ctx, leaf));
            in_unres = 1;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile parsed leaf node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed leaf node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the leaf-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_leaf(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_leaf *leaf_p = (struct lysp_node_leaf *)pnode;
    struct lysc_node_leaf *leaf = (struct lysc_node_leaf *)node;
    LY_ERR rc = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, leaf_p->musts, leaf->musts, lys_compile_must, rc, cleanup);

    /* add must(s) to unres */
    LY_CHECK_GOTO(rc = lysc_unres_must_add(ctx, node, pnode), cleanup);

    if (leaf_p->units) {
        LY_CHECK_GOTO(rc = lysdict_insert(ctx->ctx, leaf_p->units, 0, &leaf->units), cleanup);
        leaf->flags |= LYS_SET_UNITS;
    }

    /* compile type */
    LY_CHECK_GOTO(rc = lys_compile_node_type(ctx, pnode, &leaf_p->type, leaf), cleanup);

    /* store/update default value */
    if (leaf_p->dflt.str) {
        LY_CHECK_GOTO(rc = lysc_unres_leaf_dflt_add(ctx, leaf, &leaf_p->dflt), cleanup);
        leaf->flags |= LYS_SET_DFLT;
    }

    /* checks */
    if ((leaf->flags & LYS_SET_DFLT) && (leaf->flags & LYS_MAND_TRUE)) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Invalid mandatory leaf with a default value.");
        rc = LY_EVALID;
        goto cleanup;
    }

cleanup:
    return rc;
}

/**
 * @brief Compile parsed leaf-list node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed leaf-list node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the leaf-list-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_leaflist(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_leaflist *llist_p = (struct lysp_node_leaflist *)pnode;
    struct lysc_node_leaflist *llist = (struct lysc_node_leaflist *)node;
    LY_ERR rc = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, llist_p->musts, llist->musts, lys_compile_must, rc, cleanup);

    /* add must(s) to unres */
    LY_CHECK_GOTO(rc = lysc_unres_must_add(ctx, node, pnode), cleanup);

    if (llist_p->units) {
        LY_CHECK_GOTO(rc = lysdict_insert(ctx->ctx, llist_p->units, 0, &llist->units), cleanup);
        llist->flags |= LYS_SET_UNITS;
    }

    /* compile type */
    LY_CHECK_GOTO(rc = lys_compile_node_type(ctx, pnode, &llist_p->type, (struct lysc_node_leaf *)llist), cleanup);

    /* store/update default values */
    if (llist_p->dflts) {
        if (ctx->pmod->version < LYS_VERSION_1_1) {
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Leaf-list default values are allowed only in YANG 1.1 modules.");
            rc = LY_EVALID;
            goto cleanup;
        }

        LY_CHECK_GOTO(rc = lysc_unres_llist_dflts_add(ctx, llist, llist_p->dflts), cleanup);
        llist->flags |= LYS_SET_DFLT;
    }

    llist->min = llist_p->min;
    if (llist->min) {
        llist->flags |= LYS_MAND_TRUE;
    }
    llist->max = llist_p->max ? llist_p->max : UINT32_MAX;

    if (llist->flags & LYS_CONFIG_R) {
        /* state leaf-list is always ordered-by user */
        llist->flags &= ~LYS_ORDBY_SYSTEM;
        llist->flags |= LYS_ORDBY_USER;
    }

    /* checks */
    if ((llist->flags & LYS_SET_DFLT) && (llist->flags & LYS_MAND_TRUE)) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "The default statement is present on leaf-list with a nonzero min-elements.");
        rc = LY_EVALID;
        goto cleanup;
    }

    if (llist->min > llist->max) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Leaf-list min-elements %" PRIu32 " is bigger than max-elements %" PRIu32 ".",
                llist->min, llist->max);
        rc = LY_EVALID;
        goto cleanup;
    }

cleanup:
    return rc;
}

/**
 * @brief Find the node according to the given descendant/absolute schema nodeid.
 * Used in unique, refine and augment statements.
 *
 * @param[in] ctx Compile context
 * @param[in] nodeid Descendant-schema-nodeid (according to the YANG grammar)
 * @param[in] nodeid_len Length of the given nodeid, if it is not NULL-terminated string.
 * @param[in] ctx_node Context node for a relative nodeid.
 * @param[in] format Format of any prefixes.
 * @param[in] prefix_data Format-specific prefix data (see ::ly_resolve_prefix).
 * @param[in] nodetype Optional (can be 0) restriction for target's nodetype. If target exists, but does not match
 * the given nodetype, LY_EDENIED is returned (and target is provided), but no error message is printed.
 * The value can be even an ORed value to allow multiple nodetypes.
 * @param[out] target Found target node if any.
 * @param[out] result_flag Output parameter to announce if the schema nodeid goes through the action's input/output or a Notification.
 * The LYSC_OPT_RPC_INPUT, LYSC_OPT_RPC_OUTPUT and LYSC_OPT_NOTIFICATION are used as flags.
 * @return LY_ERR values - LY_ENOTFOUND, LY_EVALID, LY_EDENIED or LY_SUCCESS.
 */
static LY_ERR
lysc_resolve_schema_nodeid(struct lysc_ctx *ctx, const char *nodeid, size_t nodeid_len, const struct lysc_node *ctx_node,
        LY_VALUE_FORMAT format, void *prefix_data, uint16_t nodetype, const struct lysc_node **target, uint16_t *result_flag)
{
    LY_ERR ret = LY_EVALID;
    const char *name, *prefix, *id;
    uint32_t name_len, prefix_len, getnext_extra_flag = 0;
    const struct lys_module *mod = NULL;
    const char *nodeid_type;
    uint16_t current_nodetype = 0;

    assert(nodeid);
    assert(target);
    assert(result_flag);
    *target = NULL;
    *result_flag = 0;

    id = nodeid;

    if (ctx_node) {
        /* descendant-schema-nodeid */
        nodeid_type = "descendant";

        if (*id == '/') {
            LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                    "Invalid descendant-schema-nodeid value \"%.*s\" - absolute-schema-nodeid used.",
                    (int)(nodeid_len ? nodeid_len : strlen(nodeid)), nodeid);
            return LY_EVALID;
        }
    } else {
        /* absolute-schema-nodeid */
        nodeid_type = "absolute";

        if (*id != '/') {
            LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                    "Invalid absolute-schema-nodeid value \"%.*s\" - missing starting \"/\".",
                    (int)(nodeid_len ? nodeid_len : strlen(nodeid)), nodeid);
            return LY_EVALID;
        }
        ++id;
    }

    while (*id && (ret = ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len)) == LY_SUCCESS) {
        if (prefix) {
            mod = ly_resolve_prefix(ctx->ctx, prefix, prefix_len, format, prefix_data);
            if (!mod) {
                /* module must always be found */
                assert(prefix);
                LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                        "Invalid %s-schema-nodeid value \"%.*s\" - prefix \"%.*s\" not defined in module \"%s\".",
                        nodeid_type, (int)(id - nodeid), nodeid, (int)prefix_len, prefix, LYSP_MODULE_NAME(ctx->pmod));
                return LY_ENOTFOUND;
            }
        } else {
            switch (format) {
            case LY_VALUE_SCHEMA:
            case LY_VALUE_SCHEMA_RESOLVED:
                /* use the current module */
                mod = ctx->cur_mod;
                break;
            case LY_VALUE_CBOR:
            case LY_VALUE_JSON:
            case LY_VALUE_LYB:
                if (!ctx_node) {
                    LOGINT_RET(ctx->ctx);
                }
                /* inherit the module of the previous context node */
                mod = ctx_node->module;
                break;
            case LY_VALUE_CANON:
            case LY_VALUE_XML:
            case LY_VALUE_STR_NS:
                /* not really defined */
                LOGINT_RET(ctx->ctx);
            }
        }

        if (ctx_node && (ctx_node->nodetype & (LYS_RPC | LYS_ACTION))) {
            /* move through input/output manually */
            if (mod != ctx_node->module) {
                LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Invalid %s-schema-nodeid value \"%.*s\" - target node not found.",
                        nodeid_type, (int)(id - nodeid), nodeid);
                return LY_ENOTFOUND;
            }
            if (!ly_strncmp("input", name, name_len)) {
                ctx_node = &((struct lysc_node_action *)ctx_node)->input.node;
            } else if (!ly_strncmp("output", name, name_len)) {
                ctx_node = &((struct lysc_node_action *)ctx_node)->output.node;
                getnext_extra_flag = LYS_GETNEXT_OUTPUT;
            } else {
                /* only input or output is valid */
                ctx_node = NULL;
            }
        } else {
            ctx_node = lys_find_child(ctx->ctx, ctx_node, mod, NULL, 0, name, name_len,
                    getnext_extra_flag | LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE);
            getnext_extra_flag = 0;
        }
        if (!ctx_node) {
            LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Invalid %s-schema-nodeid value \"%.*s\" - target node not found.",
                    nodeid_type, (int)(id - nodeid), nodeid);
            return LY_ENOTFOUND;
        }
        current_nodetype = ctx_node->nodetype;

        if (current_nodetype == LYS_NOTIF) {
            (*result_flag) |= LYS_COMPILE_NOTIFICATION;
        } else if (current_nodetype == LYS_INPUT) {
            (*result_flag) |= LYS_COMPILE_RPC_INPUT;
        } else if (current_nodetype == LYS_OUTPUT) {
            (*result_flag) |= LYS_COMPILE_RPC_OUTPUT;
        }

        if (!*id || (nodeid_len && ((size_t)(id - nodeid) >= nodeid_len))) {
            break;
        }
        if (*id != '/') {
            LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                    "Invalid %s-schema-nodeid value \"%.*s\" - missing \"/\" as node-identifier separator.",
                    nodeid_type, (int)(id - nodeid + 1), nodeid);
            return LY_EVALID;
        }
        ++id;
    }

    if (ret == LY_SUCCESS) {
        *target = ctx_node;
        if (nodetype && !(current_nodetype & nodetype)) {
            return LY_EDENIED;
        }
    } else {
        LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                "Invalid %s-schema-nodeid value \"%.*s\" - unexpected end of expression.",
                nodeid_type, (int)(nodeid_len ? nodeid_len : strlen(nodeid)), nodeid);
    }

    return ret;
}

/**
 * @brief Compile information about list's uniques.
 * @param[in] ctx Compile context.
 * @param[in] uniques Sized array list of unique statements.
 * @param[in] list Compiled list where the uniques are supposed to be resolved and stored.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_node_list_unique(struct lysc_ctx *ctx, struct lysp_qname *uniques, struct lysc_node_list *list)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_node_leaf **key, ***unique;
    struct lysc_node *parent;
    const char *keystr, *delim;
    size_t len;
    LYA_COUNT_T v;
    int8_t config; /* -1 - not yet seen; 0 - LYS_CONFIG_R; 1 - LYS_CONFIG_W */
    uint16_t flags;

    LYA_FOR(uniques, v) {
        config = -1;
        LYA_ADD_ITEM(list->uniques, unique, LOGMEM(ctx->ctx); return LY_EMEM);
        keystr = uniques[v].str;
        while (keystr) {
            delim = strpbrk(keystr, " \t\n");
            if (delim) {
                len = delim - keystr;
                while (isspace(*delim)) {
                    ++delim;
                }
            } else {
                len = strlen(keystr);
            }

            /* unique node must be present */
            LYA_ADD_ITEM(*unique, key, LOGMEM(ctx->ctx); return LY_EMEM);
            ret = lysc_resolve_schema_nodeid(ctx, keystr, len, &list->node, LY_VALUE_SCHEMA, (void *)uniques[v].mod,
                    LYS_LEAF, (const struct lysc_node **)key, &flags);
            if (ret != LY_SUCCESS) {
                if (ret == LY_EDENIED) {
                    LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                            "Unique's descendant-schema-nodeid \"%.*s\" refers to %s node instead of a leaf.",
                            (int)len, keystr, lys_nodetype2str((*key)->nodetype));
                }
                return LY_EVALID;
            } else if (flags) {
                LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                        "Unique's descendant-schema-nodeid \"%.*s\" refers into %s node.",
                        (int)len, keystr, flags & LYS_IS_NOTIF ? "notification" : "RPC/action");
                return LY_EVALID;
            }

            /* all referenced leafs must be of the same config type */
            if ((config != -1) && ((((*key)->flags & LYS_CONFIG_W) && (config == 0)) ||
                    (((*key)->flags & LYS_CONFIG_R) && (config == 1)))) {
                LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                        "Unique statement \"%s\" refers to leaves with different config type.", uniques[v].str);
                return LY_EVALID;
            } else if ((*key)->flags & LYS_CONFIG_W) {
                config = 1;
            } else { /* LYS_CONFIG_R */
                config = 0;
            }

            /* we forbid referencing nested lists because it is unspecified what instance of such a list to use */
            for (parent = (*key)->parent; parent != (struct lysc_node *)list; parent = parent->parent) {
                if (parent->nodetype == LYS_LIST) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                            "Unique statement \"%s\" refers to a leaf in nested list \"%s\".", uniques[v].str, parent->name);
                    return LY_EVALID;
                }
            }

            /* check status */
            LY_CHECK_RET(lysc_check_status(ctx, NULL, list->flags, uniques[v].mod->mod, list->name,
                    (*key)->flags, (*key)->module, (*key)->name));

            /* mark leaf as unique */
            (*key)->flags |= LYS_UNIQUE;

            /* next unique value in line */
            keystr = delim;
        }
        /* next unique definition */
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile parsed list node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed list node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the list-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_list(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_list *list_p = (struct lysp_node_list *)pnode;
    struct lysc_node_list *list = (struct lysc_node_list *)node;
    struct lysp_node *child_p;
    struct lysc_node *parent;
    struct lysc_node_leaf *key, *prev_key = NULL;
    size_t len;
    const char *keystr, *delim;
    LY_ERR ret = LY_SUCCESS;

    list->min = list_p->min;
    if (list->min) {
        list->flags |= LYS_MAND_TRUE;
    }
    list->max = list_p->max ? list_p->max : UINT32_MAX;

    LY_LIST_FOR(list_p->child, child_p) {
        LY_CHECK_RET(lys_compile_node(ctx, child_p, node, 0, NULL));
    }

    COMPILE_ARRAY_GOTO(ctx, list_p->musts, list->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    /* keys */
    if (list->flags & LYS_CONFIG_W) {
        parent = node;
        if (ctx->compile_opts & LYS_COMPILE_GROUPING) {
            /* compiling individual grouping, we can check this only if there is an explicit config set */
            while (parent) {
                if (parent->flags & LYS_SET_CONFIG) {
                    break;
                }
                parent = parent->parent;
            }
        }

        if (parent && (!list_p->key || !list_p->key[0])) {
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Missing key in list representing configuration data.");
            return LY_EVALID;
        }
    }

    /* find all the keys (must be direct children) */
    keystr = list_p->key;
    if (!keystr) {
        /* keyless list */
        list->flags &= ~LYS_ORDBY_SYSTEM;
        list->flags |= LYS_KEYLESS | LYS_ORDBY_USER;
    }
    while (keystr) {
        delim = strpbrk(keystr, " \t\n");
        if (delim) {
            len = delim - keystr;
            while (isspace(*delim)) {
                ++delim;
            }
        } else {
            len = strlen(keystr);
        }

        /* key node must be present */
        key = (struct lysc_node_leaf *)lys_find_child(NULL, node, node->module, NULL, 0, keystr, len, LYS_GETNEXT_NOCHOICE);
        if (key && (key->nodetype != LYS_LEAF)) {
            key = NULL;
        }
        if (!key) {
            LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "The list's key \"%.*s\" not found.", (int)len, keystr);
            return LY_EVALID;
        }
        /* keys must be unique */
        if (key->flags & LYS_KEY) {
            /* the node was already marked as a key */
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Duplicated key identifier \"%.*s\".", (int)len, keystr);
            return LY_EVALID;
        }

        lysc_update_path(ctx, list->module, key->name);

        /* key must have the same config flag as the list itself */
        if ((list->flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK)) {
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Key of a configuration list must not be a state leaf.");
            goto key_invalid;
        }
        if (ctx->pmod->version < LYS_VERSION_1_1) {
            /* YANG 1.0 denies key to be of empty type */
            if (key->type->basetype == LY_TYPE_EMPTY) {
                LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                        "List key of the \"empty\" type is allowed only in YANG 1.1 modules.");
                goto key_invalid;
            }
        } else {
            /* when and if-feature are illegal on list keys */
            if (key->when) {
                LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "List's key must not have any \"when\" statement.");
                goto key_invalid;
            }
            /* unable to check if-features but compilation would fail if disabled */
        }

        /* check status */
        LY_CHECK_GOTO(lysc_check_status(ctx, NULL, list->flags, list->module, list->name, key->flags, key->module,
                key->name), key_invalid);

        /* ignore default values of the key */
        if (key->dflt.str) {
            lysc_value_free(ctx->ctx, &key->dflt);
            memset(&key->dflt, 0, sizeof key->dflt);
        }
        /* mark leaf as key */
        key->flags |= LYS_KEY;

        /* move it to the correct position */
        if ((prev_key && ((struct lysc_node *)prev_key != key->prev)) || (!prev_key && key->prev->next)) {
            /* fix links in closest previous siblings of the key */
            if (key->next) {
                key->next->prev = key->prev;
            } else {
                /* last child */
                list->child->prev = key->prev;
            }
            if (key->prev->next) {
                key->prev->next = key->next;
            }
            /* fix links in the key */
            if (prev_key) {
                key->prev = &prev_key->node;
                key->next = prev_key->next;
            } else {
                key->prev = list->child->prev;
                key->next = list->child;
            }
            /* fix links in closes future siblings of the key */
            if (prev_key) {
                if (prev_key->next) {
                    prev_key->next->prev = &key->node;
                } else {
                    list->child->prev = &key->node;
                }
                prev_key->next = &key->node;
            } else {
                list->child->prev = &key->node;
            }
            /* fix links in parent */
            if (!key->prev->next) {
                list->child = &key->node;
            }
        }

        /* next key value */
        prev_key = key;
        keystr = delim;
        lysc_update_path(ctx, NULL, NULL);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    /* uniques */
    if (list_p->uniques) {
        LY_CHECK_RET(lys_compile_node_list_unique(ctx, list_p->uniques, list));
    }

    LY_LIST_FOR((struct lysp_node *)list_p->actions, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }
    LY_LIST_FOR((struct lysp_node *)list_p->notifs, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

    /* checks */
    if (list->min > list->max) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "List min-elements %" PRIu32 " is bigger than max-elements %" PRIu32 ".",
                list->min, list->max);
        return LY_EVALID;
    }

done:
    return ret;

key_invalid:
    lysc_update_path(ctx, NULL, NULL);
    return LY_EVALID;
}

/**
 * @brief Do some checks and set the default choice's case.
 *
 * Selects (and stores into ::lysc_node_choice#dflt) the default case and set LYS_SET_DFLT flag on it.
 *
 * @param[in] ctx Compile context.
 * @param[in] dflt Name of the default branch. Can even contain a prefix.
 * @param[in,out] ch The compiled choice node, its dflt member is filled to point to the default case node of the choice.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_node_choice_dflt(struct lysc_ctx *ctx, struct lysp_qname *dflt, struct lysc_node_choice *ch)
{
    struct lysc_node *iter;
    const struct lys_module *mod;
    const char *prefix = NULL, *name;
    size_t prefix_len = 0;

    /* could use lys_parse_nodeid(), but it checks syntax which is already done in this case by the parsers */
    name = strchr(dflt->str, ':');
    if (name) {
        prefix = dflt->str;
        prefix_len = name - prefix;
        ++name;
    } else {
        name = dflt->str;
    }
    if (prefix) {
        mod = ly_resolve_prefix(ctx->ctx, prefix, prefix_len, LY_VALUE_SCHEMA, (void *)dflt->mod);
        if (!mod) {
            LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Default case prefix \"%.*s\" not found "
                    "in imports of \"%s\".", (int)prefix_len, prefix, LYSP_MODULE_NAME(dflt->mod));
            return LY_EVALID;
        }
    } else {
        mod = ch->module;
    }

    ch->dflt = (struct lysc_node_case *)lys_find_child(NULL, &ch->node, mod, NULL, 0, name, 0, LYS_GETNEXT_WITHCASE);
    if (ch->dflt && (ch->dflt->nodetype != LYS_CASE)) {
        ch->dflt = NULL;
    }
    if (!ch->dflt) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Default case \"%s\" not found.", dflt->str);
        return LY_EVALID;
    }

    /* no mandatory nodes directly under the default case */
    LY_LIST_FOR(ch->dflt->child, iter) {
        if (iter->parent != (struct lysc_node *)ch->dflt) {
            break;
        }
        if (iter->flags & LYS_MAND_TRUE) {
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                    "Mandatory node \"%s\" under the default case \"%s\".", iter->name, dflt->str);
            return LY_EVALID;
        }
    }

    if (ch->flags & LYS_MAND_TRUE) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Invalid mandatory choice with a default case.");
        return LY_EVALID;
    }

    ch->dflt->flags |= LYS_SET_DFLT;
    return LY_SUCCESS;
}

LY_ERR
lys_compile_node_choice_child(struct lysc_ctx *ctx, struct lysp_node *child_p, struct lysc_node *node,
        struct ly_set *child_set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *child_p_next = child_p->next;
    struct lysp_node_case *cs_p;
    struct lysc_node_case *cs_c;

    if (child_p->nodetype == LYS_CASE) {
        /* standard case under choice */
        ret = lys_compile_node(ctx, child_p, node, 0, child_set);
    } else {
        /* we need the implicit case first, so create a fake parsed (shorthand) case */
        cs_p = lysp_parser_node_new(sizeof *cs_p, NULL);
        LY_CHECK_ERR_RET(!cs_p, LOGMEM(ctx->ctx), LY_EMEM);
        cs_p->nodetype = LYS_CASE;
        DUP_STRING_GOTO(ctx->ctx, child_p->name, cs_p->name, ret, revert_sh_case);
        cs_p->child = child_p;

        /* make the child the only case child */
        child_p->next = NULL;

        /* compile it normally */
        LY_CHECK_GOTO(ret = lys_compile_node(ctx, (struct lysp_node *)cs_p, node, 0, child_set), revert_sh_case);

        if (((struct lysc_node_choice *)node)->cases) {
            /* find our case node */
            cs_c = (struct lysc_node_case *)((struct lysc_node_choice *)node)->cases;
            while (cs_c->name != cs_p->name) {
                cs_c = (struct lysc_node_case *)cs_c->next;
                assert(cs_c);
            }

            if (ctx->ctx->opts & LY_CTX_SET_PRIV_PARSED) {
                /* compiled case node cannot point to his corresponding parsed node
                 * because it exists temporarily so it must be set to NULL
                 */
                assert(cs_c->priv == cs_p);
                cs_c->priv = NULL;
            }

            /* status is copied from his child and not from his parent as usual. */
            if (cs_c->child) {
                cs_c->flags &= ~LYS_STATUS_MASK;
                cs_c->flags |= (LYS_STATUS_MASK & cs_c->child->flags);
            }
        } /* else it was removed by a deviation */

revert_sh_case:
        /* free the parsed shorthand case and correct pointers back */
        cs_p->child = NULL;
        lysp_node_free(ctx->ctx, (struct lysp_node *)cs_p);
        child_p->next = child_p_next;
    }

    return ret;
}

/**
 * @brief Compile parsed choice node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed choice node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the choice-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_choice(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_choice *ch_p = (struct lysp_node_choice *)pnode;
    struct lysc_node_choice *ch = (struct lysc_node_choice *)node;
    struct lysp_node *child_p;
    LY_ERR ret = LY_SUCCESS;

    assert(node->nodetype == LYS_CHOICE);

    LY_LIST_FOR(ch_p->child, child_p) {
        LY_CHECK_GOTO(ret = lys_compile_node_choice_child(ctx, child_p, node, NULL), done);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    /* default branch */
    if (ch_p->dflt.str) {
        LY_CHECK_GOTO(ret = lys_compile_node_choice_dflt(ctx, &ch_p->dflt, ch), done);
    }

done:
    return ret;
}

/**
 * @brief Compile parsed anydata or anyxml node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed anydata or anyxml node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the any-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_any(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_anydata *any_p = (struct lysp_node_anydata *)pnode;
    struct lysc_node_anydata *any = (struct lysc_node_anydata *)node;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, any_p->musts, any->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    if (any->flags & LYS_CONFIG_W) {
        LOGVRB("Use of %s to define configuration data is not recommended. %s",
                lyplg_ext_stmt2str(any->nodetype == LYS_ANYDATA ? LY_STMT_ANYDATA : LY_STMT_ANYXML), ctx->path);
    }
done:
    return ret;
}

/**
 * @brief Prepare the case structure in choice node for the new data node.
 *
 * It is able to handle implicit as well as explicit cases and the situation when the case has multiple data nodes and the case was already
 * created in the choice when the first child was processed.
 *
 * @param[in] ctx Compile context.
 * @param[in] pnode Node image from the parsed tree. If the case is explicit, it is the LYS_CASE node, but in case of implicit case,
 *                   it is the LYS_CHOICE, LYS_AUGMENT or LYS_GROUPING node.
 * @param[in] ch The compiled choice structure where the new case structures are created (if needed).
 * @param[in] child The new data node being part of a case (no matter if explicit or implicit).
 * @return The case structure where the child node belongs to, NULL in case of error. Note that the child is not connected into the siblings list,
 * it is linked from the case structure only in case it is its first child.
 */
static LY_ERR
lys_compile_node_case(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *child_p;
    struct lysp_node_case *cs_p = (struct lysp_node_case *)pnode;

    if (pnode->nodetype & (LYS_CHOICE | LYS_AUGMENT | LYS_GROUPING)) {
        /* we have to add an implicit case node into the parent choice */
    } else if (pnode->nodetype == LYS_CASE) {
        /* explicit parent case */
        LY_LIST_FOR(cs_p->child, child_p) {
            LY_CHECK_GOTO(ret = lys_compile_node(ctx, child_p, node, 0, NULL), done);
        }
    } else {
        LOGINT_RET(ctx->ctx);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

done:
    return ret;
}

void
lys_compile_mandatory_parents(struct lysc_node *parent, ly_bool add)
{
    const struct lysc_node *iter;

    if (add) { /* set flag */
        for ( ; parent && parent->nodetype == LYS_CONTAINER && !(parent->flags & LYS_MAND_TRUE) && !(parent->flags & LYS_PRESENCE);
                parent = parent->parent) {
            parent->flags |= LYS_MAND_TRUE;
        }
    } else { /* unset flag */
        for ( ; parent && parent->nodetype == LYS_CONTAINER && (parent->flags & LYS_MAND_TRUE); parent = parent->parent) {
            for (iter = lysc_node_child(parent); iter; iter = iter->next) {
                if (iter->flags & LYS_MAND_TRUE) {
                    /* there is another mandatory node */
                    return;
                }
            }
            /* unset mandatory flag - there is no mandatory children in the non-presence container */
            parent->flags &= ~LYS_MAND_TRUE;
        }
    }
}

/**
 * @brief Get the grouping with the specified name from given groupings sized array.
 *
 * @param[in] node Linked list of nodes with groupings.
 * @param[in] name Name of the grouping to find,
 * @return NULL when there is no grouping with the specified name
 * @return Pointer to the grouping of the specified @p name.
 */
static struct lysp_node_grp *
match_grouping(const struct lysp_node_grp *node, const char *name)
{
    LY_LIST_FOR(node, node) {
        if ((node->nodetype == LYS_GROUPING) && !strcmp(node->name, name)) {
            return (struct lysp_node_grp *)node;
        }
    }

    return NULL;
}

/**
 * @brief Find grouping for a uses.
 *
 * @param[in] ctx Compile context.
 * @param[in] uses_p Parsed uses node.
 * @param[out] gpr_p Found grouping on success.
 * @param[out] grp_pmod Module of @p grp_p on success.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_uses_find_grouping(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, struct lysp_node_grp **grp_p,
        struct lysp_module **grp_pmod)
{
    struct lysp_node *pnode;
    struct lysp_node_grp *grp;
    const struct lysp_node_grp *ext_grp;
    LYA_COUNT_T u;
    const char *id, *name, *prefix, *local_pref;
    uint32_t prefix_len, name_len;
    struct lysp_module *pmod, *found = NULL;
    const struct lys_module *mod;

    *grp_p = NULL;
    *grp_pmod = NULL;

    /* search for the grouping definition */
    id = uses_p->name;
    LY_CHECK_RET(ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len), LY_EVALID);
    local_pref = ctx->pmod->is_submod ? ((struct lysp_submodule *)ctx->pmod)->prefix : ctx->pmod->mod->prefix;
    if (!prefix || !ly_strncmp(local_pref, prefix, prefix_len)) {
        /* current module, search local groupings first */
        pmod = ctx->pmod->mod->parsed; /* make sure that we will start in main_module, not submodule */
        for (pnode = uses_p->parent; !found && pnode; pnode = pnode->parent) {
            if ((grp = match_grouping(lysp_node_groupings(pnode), name))) {
                found = ctx->pmod;
                break;
            }
        }

        /* if in an extension, search possible groupings in it */
        if (!found && ctx->ext) {
            lyplg_ext_parsed_get_storage(ctx->ext, LY_STMT_GROUPING, sizeof ext_grp, (const void **)&ext_grp);
            if ((grp = match_grouping(ext_grp, name))) {
                found = ctx->pmod;
            }
        }
    } else {
        /* foreign module, find it first */
        mod = ly_resolve_prefix(ctx->ctx, prefix, prefix_len, LY_VALUE_SCHEMA, ctx->pmod);
        if (!mod) {
            LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Invalid prefix used for grouping \"%s\" reference.", uses_p->name);
            return LY_EVALID;
        }
        pmod = mod->parsed;
    }

    if (!found) {
        /* search in top-level groupings of the main module ... */
        if ((grp = match_grouping(pmod->groupings, name))) {
            found = pmod;
        } else {
            /* ... and all the submodules */
            LYA_FOR(pmod->includes, u) {
                if ((grp = match_grouping(pmod->includes[u].submodule->groupings, name))) {
                    found = (struct lysp_module *)pmod->includes[u].submodule;
                    break;
                }
            }
        }
    }
    if (!found) {
        LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Grouping \"%s\" referenced by a uses statement not found.", uses_p->name);
        return LY_EVALID;
    }

    if (!(ctx->compile_opts & LYS_COMPILE_GROUPING)) {
        /* remember that the grouping is instantiated to avoid its standalone validation */
        grp->flags |= LYS_USED_GRP;
    }

    *grp_p = grp;
    *grp_pmod = found;
    return LY_SUCCESS;
}

/**
 * @brief Compile uses grouping children.
 *
 * @param[in] ctx Compile context.
 * @param[in] uses_p Parsed uses.
 * @param[in] inherited_flags Inherited flags from the uses.
 * @param[in] child First grouping child to compile.
 * @param[in] grp_mod Grouping parsed module.
 * @param[in] parent Uses compiled parent, may be NULL if top-level.
 * @param[in,out] child_set Set of all compiled child nodes.
 * @param[in] child_unres_disabled Whether the children are to be put into unres disabled set or not.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID on failure.
 */
static LY_ERR
lys_compile_uses_children(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, uint16_t inherited_flags,
        struct lysp_node *child, struct lysp_module *grp_mod, struct lysc_node *parent, struct ly_set *child_set,
        ly_bool child_unres_disabled)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysp_module *mod_old = ctx->pmod;
    uint32_t child_i, opt_prev = ctx->compile_opts;
    ly_bool enabled;
    struct lysp_node *pnode;
    struct lysc_node *node;
    struct lysc_when *when_shared = NULL;

    assert(child_set);

    child_i = child_set->count;
    LY_LIST_FOR(child, pnode) {
        /* compile the nodes with their parsed (grouping) module */
        ctx->pmod = grp_mod;
        LY_CHECK_GOTO(rc = lys_compile_node(ctx, pnode, parent, inherited_flags, child_set), cleanup);

        /* eval if-features again for the rest of this node processing */
        LY_CHECK_GOTO(rc = lys_eval_iffeatures(ctx->ctx, pnode->iffeatures, &enabled), cleanup);
        if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
            ctx->compile_opts |= LYS_COMPILE_DISABLED;
        }

        /* restore the parsed module */
        ctx->pmod = mod_old;

        /* since the uses node is not present in the compiled tree, we need to pass some of its
         * statements to all its children */
        while (child_i < child_set->count) {
            node = child_set->snodes[child_i];

            if (uses_p->when) {
                /* pass uses when to all the children */
                rc = lys_compile_when(ctx, uses_p->when, inherited_flags, parent, lysc_data_node(parent), node, &when_shared);
                LY_CHECK_GOTO(rc, cleanup);
            }

            if (child_unres_disabled) {
                /* child is disabled by the uses if-features */
                ly_set_add(&ctx->unres->disabled, node, 1, NULL);
            }

            /* child processed */
            ++child_i;
        }

        /* next iter */
        ctx->compile_opts = opt_prev;
    }

cleanup:
    ctx->compile_opts = opt_prev;
    return rc;
}

/**
 * @brief Compile parsed uses statement - resolve target grouping and connect its content into parent.
 * If present, also apply uses's modificators.
 *
 * @param[in] ctx Compile context
 * @param[in] uses_p Parsed uses schema node.
 * @param[in] parent Compiled parent node where the content of the referenced grouping is supposed to be connected. It is
 * NULL for top-level nodes, in such a case the module where the node will be connected is taken from
 * the compile context.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in] child_set Optional set of all the compiled children.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_uses(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, struct lysc_node *parent, uint16_t inherited_flags,
        struct ly_set *child_set)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool enabled, child_unres_disabled = 0;
    uint32_t i, grp_stack_count, opt_prev = ctx->compile_opts;
    LYA_COUNT_T orig_u, u;
    struct lysp_node_grp *grp = NULL;
    uint16_t uses_flags = 0;
    struct lysp_module *grp_mod;
    struct ly_set uses_child_set = {0};

    /* find the referenced grouping */
    LY_CHECK_RET(lys_compile_uses_find_grouping(ctx, uses_p, &grp, &grp_mod));

    if ((ctx->compile_opts & LYS_COMPILE_LOCAL_ONLY) && (ctx->pmod->mod != grp_mod->mod)) {
        /* external grouping, do not resolve or compile */
        return LY_SUCCESS;
    }

    /* grouping must not reference themselves - stack in ctx maintains list of groupings currently being applied */
    grp_stack_count = ctx->groupings.count;
    LY_CHECK_RET(ly_set_add(&ctx->groupings, (void *)grp, 0, NULL));
    if (grp_stack_count == ctx->groupings.count) {
        /* the target grouping is already in the stack, so we are already inside it -> circular dependency */
        LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE,
                "Grouping \"%s\" references itself through a uses statement.", grp->name);
        return LY_EVALID;
    }

    /* nodetype checks */
    if (grp->actions && (parent && !lysc_node_actions_p(parent))) {
        LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Invalid child %s \"%s\" of uses parent %s \"%s\" node.",
                grp->actions->name, lys_nodetype2str(grp->actions->nodetype),
                parent->name, lys_nodetype2str(parent->nodetype));
        rc = LY_EVALID;
        goto cleanup;
    }
    if (grp->notifs && (parent && !lysc_node_notifs_p(parent))) {
        LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Invalid child %s \"%s\" of uses parent %s \"%s\" node.",
                grp->notifs->name, lys_nodetype2str(grp->notifs->nodetype),
                parent->name, lys_nodetype2str(parent->nodetype));
        rc = LY_EVALID;
        goto cleanup;
    }

    /* check status */
    rc = lysc_check_status(ctx, NULL, uses_p->flags, ctx->pmod, uses_p->name, grp->flags, grp_mod, grp->name);
    LY_CHECK_GOTO(rc, cleanup);

    /* compile any augments and refines so they can be applied during the grouping nodes compilation */
    rc = lys_precompile_uses_augments_refines(ctx, uses_p, parent);
    LY_CHECK_GOTO(rc, cleanup);

    /* compile special uses status flags */
    rc = lys_compile_status(ctx, uses_p->flags, inherited_flags, parent ? parent->flags : 0,
            parent ? parent->name : NULL, "<uses>", &uses_flags);
    LY_CHECK_GOTO(rc, cleanup);

    /* uses if-features */
    LY_CHECK_GOTO(rc = lys_eval_iffeatures(ctx->ctx, uses_p->iffeatures, &enabled), cleanup);
    if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
        child_unres_disabled = 1;
    }

    /* uses grouping children */
    rc = lys_compile_uses_children(ctx, uses_p, uses_flags, grp->child, grp_mod, parent,
            child_set ? child_set : &uses_child_set, child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* uses grouping RPCs/actions */
    rc = lys_compile_uses_children(ctx, uses_p, uses_flags, (struct lysp_node *)grp->actions, grp_mod, parent,
            child_set ? child_set : &uses_child_set, child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* uses grouping notifications */
    rc = lys_compile_uses_children(ctx, uses_p, uses_flags, (struct lysp_node *)grp->notifs, grp_mod, parent,
            child_set ? child_set : &uses_child_set, child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* check that all augments were applied */
    for (i = 0; i < ctx->uses_augs.count; ++i) {
        if (((struct lysc_augment *)ctx->uses_augs.objs[i])->aug_p->parent != (struct lysp_node *)uses_p) {
            /* augment of some parent uses, irrelevant now */
            continue;
        }

        LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Augment target node \"%s\" in grouping \"%s\" was not found.",
                ((struct lysc_augment *)ctx->uses_augs.objs[i])->nodeid->str, grp->name);
        rc = LY_ENOTFOUND;
    }
    LY_CHECK_GOTO(rc, cleanup);

    /* check that all refines were applied */
    for (i = 0; i < ctx->uses_rfns.count; ++i) {
        if (((struct lysc_refine *)ctx->uses_rfns.objs[i])->uses_p != uses_p) {
            /* refine of some parent uses, irrelevant now */
            continue;
        }

        LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Refine(s) target node \"%s\" in grouping \"%s\" was not found.",
                ((struct lysc_refine *)ctx->uses_rfns.objs[i])->nodeid->str, grp->name);
        rc = LY_ENOTFOUND;
    }
    LY_CHECK_GOTO(rc, cleanup);

    if (parent) {
        /* compile uses and grouping extensions into the parent */
        COMPILE_EXTS_GOTO(ctx, uses_p->exts, parent->exts, parent, rc, cleanup);
        COMPILE_EXTS_GOTO(ctx, grp->exts, parent->exts, parent, rc, cleanup);
    } else {
        /* compile uses and grouping extensions into the module */
        orig_u = LYA_COUNT(ctx->cmod->exts);
        COMPILE_EXTS_GOTO(ctx, uses_p->exts, ctx->cmod->exts, uses_p, rc, cleanup);
        COMPILE_EXTS_GOTO(ctx, grp->exts, ctx->cmod->exts, grp, rc, cleanup);
        for (u = orig_u; u < LYA_COUNT(ctx->cmod->exts); ++u) {
            ctx->cmod->exts[u].parent = ctx->cmod;
            ctx->cmod->exts[u].parent_stmt = LY_STMT_MODULE;
        }
    }

cleanup:
    /* restore previous context */
    ctx->compile_opts = opt_prev;

    /* remove the grouping from the stack for circular groupings dependency check */
    ly_set_rm_index(&ctx->groupings, ctx->groupings.count - 1, NULL);
    assert(ctx->groupings.count == grp_stack_count);

    ly_set_erase(&uses_child_set, NULL);
    return rc;
}

/**
 * @brief Generate path of a grouping for logging.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Grouping node.
 * @param[out] path Generated path.
 * @return Length of the generated @p path;
 * @return -1 on error.
 */
static int
lys_compile_grouping_pathlog(struct lysc_ctx *ctx, struct lysp_node *node, char **path)
{
    struct lysp_node *iter;
    int len = 0, r;
    char *s, *id;

    *path = NULL;

    for (iter = node; iter && (len >= 0); iter = iter->parent) {
        s = *path;

        /* next node segment */
        switch (iter->nodetype) {
        case LYS_USES:
            r = asprintf(&id, "{uses='%s'}", iter->name);
            break;
        case LYS_GROUPING:
            r = asprintf(&id, "{grouping='%s'}", iter->name);
            break;
        case LYS_AUGMENT:
            r = asprintf(&id, "{augment='%s'}", iter->name);
            break;
        default:
            id = strdup(iter->name);
            r = id ? 1 : -1;
            break;
        }
        if (r == -1) {
            len = -1;
            goto cleanup;
        }

        /* append the segment to the path */
        if (!iter->parent) {
            /* print prefix */
            r = asprintf(path, "/%s:%s%s", ctx->cur_mod->name, id, s ? s : "");
        } else {
            /* prefix is the same as in parent */
            r = asprintf(path, "/%s%s", id, s ? s : "");
        }
        free(s);
        free(id);
        if (r == -1) {
            len = -1;
            goto cleanup;
        }

        /* remember the length of the full path */
        len = r;
    }

cleanup:
    if (len < 0) {
        free(*path);
        *path = NULL;
    }
    return len;
}

LY_ERR
lys_compile_grouping(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysp_node_grp *grp)
{
    LY_ERR rc = LY_SUCCESS;
    char *path;
    int len;

    /* use grouping status to avoid errors */
    struct lysp_node_uses fake_uses = {
        .parent = pnode,
        .nodetype = LYS_USES,
        .flags = grp->flags & LYS_STATUS_MASK, .next = NULL,
        .name = grp->name,
        .dsc = NULL, .ref = NULL, .when = NULL, .iffeatures = NULL, .exts = NULL,
        .refines = NULL, .augments = NULL
    };
    struct lysc_node_container fake_container = {
        .nodetype = LYS_CONTAINER,
        .flags = 0,
        .module = ctx->cur_mod,
        .parent = NULL, .next = NULL,
        .prev = &fake_container.node,
        .name = "fake",
        .dsc = NULL, .ref = NULL, .exts = NULL, .when = NULL,
        .child = NULL, .musts = NULL, .actions = NULL, .notifs = NULL
    };

    /* compile fake container flags */
    LY_CHECK_GOTO(rc = lys_compile_node_flags(ctx, pnode ? pnode->flags : 0, 0, &fake_container.node), cleanup);

    if (grp->parent) {
        LOGWRN(ctx->ctx, "Locally scoped grouping \"%s\" not used.", grp->name);
    }

    len = lys_compile_grouping_pathlog(ctx, grp->parent, &path);
    if (len < 0) {
        LOGMEM(ctx->ctx);
        return LY_EMEM;
    }
    free(ctx->path);
    ctx->path = path;
    ctx->path_used = len;
    ctx->path_size = ctx->path_used ? ctx->path_used + 1 : 0;

    lysc_update_path(ctx, NULL, "{grouping}");
    lysc_update_path(ctx, NULL, grp->name);
    rc = lys_compile_uses(ctx, &fake_uses, &fake_container.node, 0, NULL);
    lysc_update_path(ctx, NULL, NULL);
    lysc_update_path(ctx, NULL, NULL);

    free(ctx->path);
    ctx->path = NULL;
    ctx->path_used = 0;
    ctx->path_size = 0;

cleanup:
    lysc_node_container_free(ctx->ctx, &fake_container);
    FREE_ARRAY(ctx->ctx, fake_container.exts, lysc_ext_instance_free);
    return rc;
}

LY_ERR
lys_compile_node(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *parent, uint16_t inherited_flags,
        struct ly_set *child_set)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_node *node = NULL;
    uint32_t prev_opts = ctx->compile_opts;

    LY_ERR (*node_compile_spec)(struct lysc_ctx *, struct lysp_node *, struct lysc_node *);

    if (pnode->nodetype != LYS_USES) {
        lysc_update_path(ctx, parent ? parent->module : NULL, pnode->name);
    } else {
        lysc_update_path(ctx, NULL, "{uses}");
        lysc_update_path(ctx, NULL, pnode->name);
    }

    switch (pnode->nodetype) {
    case LYS_CONTAINER:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_container));
        node_compile_spec = lys_compile_node_container;
        break;
    case LYS_LEAF:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_leaf));
        node_compile_spec = lys_compile_node_leaf;
        break;
    case LYS_LIST:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_list));
        node_compile_spec = lys_compile_node_list;
        break;
    case LYS_LEAFLIST:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_leaflist));
        node_compile_spec = lys_compile_node_leaflist;
        break;
    case LYS_CHOICE:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_choice));
        node_compile_spec = lys_compile_node_choice;
        break;
    case LYS_CASE:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_case));
        node_compile_spec = lys_compile_node_case;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_anydata));
        node_compile_spec = lys_compile_node_any;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        if (ctx->compile_opts & (LYS_IS_INPUT | LYS_IS_OUTPUT | LYS_IS_NOTIF)) {
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                    "Action \"%s\" is placed inside %s.", pnode->name,
                    (ctx->compile_opts & LYS_IS_NOTIF) ? "notification" : "another RPC/action");
            rc = LY_EVALID;
            goto cleanup;
        }
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_action));
        node_compile_spec = lys_compile_node_action;
        ctx->compile_opts |= LYS_COMPILE_NO_CONFIG;
        break;
    case LYS_NOTIF:
        if (ctx->compile_opts & (LYS_IS_INPUT | LYS_IS_OUTPUT | LYS_IS_NOTIF)) {
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                    "Notification \"%s\" is placed inside %s.", pnode->name,
                    (ctx->compile_opts & LYS_IS_NOTIF) ? "another notification" : "RPC/action");
            rc = LY_EVALID;
            goto cleanup;
        }
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_notif));
        node_compile_spec = lys_compile_node_notif;
        ctx->compile_opts |= LYS_COMPILE_NOTIFICATION;
        break;
    case LYS_USES:
        rc = lys_compile_uses(ctx, (struct lysp_node_uses *)pnode, parent, inherited_flags, child_set);
        goto cleanup;
    default:
        LOGINT(ctx->ctx);
        rc = LY_EINT;
        goto cleanup;
    }
    LY_CHECK_ERR_GOTO(!node, LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);

    rc = lys_compile_node_(ctx, pnode, parent, inherited_flags, node_compile_spec, node, child_set);

cleanup:
    ctx->compile_opts = prev_opts;
    if (pnode->nodetype != LYS_USES) {
        lysc_update_path(ctx, NULL, NULL);
    } else {
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }
    return rc;
}
