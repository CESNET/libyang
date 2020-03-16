/**
 * @file tree_data_helpers.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsing and validation helper functions for data trees
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "common.h"

#include <assert.h>
#include <stdlib.h>

#include "log.h"
#include "dict.h"
#include "plugins_types.h"
#include "tree_data.h"
#include "tree_schema.h"

struct lyd_node **
lyd_node_children_p(struct lyd_node *node)
{
    assert(node);

    if (!node->schema) {
        return &((struct lyd_node_opaq *)node)->child;
    } else {
        switch (node->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_ACTION:
        case LYS_NOTIF:
            return &((struct lyd_node_inner *)node)->child;
        default:
            return NULL;
        }
    }
}

API const struct lyd_node *
lyd_node_children(const struct lyd_node *node)
{
    struct lyd_node **children;

    if (!node) {
        return NULL;
    }

    children = lyd_node_children_p((struct lyd_node*)node);
    if (children) {
        return *children;
    } else {
        return NULL;
    }
}

API const struct lys_module *
lyd_owner_module(const struct lyd_node *node)
{
    const struct lysc_node *schema;

    if (!node || !node->schema) {
        return NULL;
    }

    for (schema = node->schema; schema->parent; schema = schema->parent);
    return schema->module;
}

const struct lys_module *
lyd_mod_next_module(struct lyd_node *tree, const struct lys_module **modules, int mod_count, const struct ly_ctx *ctx,
                    uint32_t *i, struct lyd_node **first)
{
    struct lyd_node *iter;
    const struct lys_module *mod;

    /* get the next module */
    if (modules && mod_count) {
        if (*i < (unsigned)mod_count) {
            mod = modules[(*i)++];
        } else {
            mod = NULL;
        }
    } else {
        do {
            mod = ly_ctx_get_module_iter(ctx, i);
        } while (mod && !mod->implemented);
    }

    /* find its data */
    *first = NULL;
    if (mod) {
        LY_LIST_FOR(tree, iter) {
            if (lyd_owner_module(iter) == mod) {
                *first = iter;
                break;
            }
        }
    }

    return mod;
}

const struct lys_module *
lyd_data_next_module(struct lyd_node **next, struct lyd_node **first)
{
    const struct lys_module *mod;

    if (!*next) {
        /* all data traversed */
        *first = NULL;
        return NULL;
    }

    *first = *next;

    /* prepare next */
    mod = lyd_owner_module(*next);
    LY_LIST_FOR(*next, *next) {
        if (lyd_owner_module(*next) != mod) {
            break;
        }
    }

    return mod;
}

LY_ERR
lyd_parse_check_keys(struct lyd_node *node)
{
    const struct lysc_node *skey = NULL;
    const struct lyd_node *key;

    assert(node->schema->nodetype == LYS_LIST);

    key = lyd_node_children(node);
    while ((skey = lys_getnext(skey, node->schema, NULL, 0)) && (skey->flags & LYS_KEY)) {
        if (!key || (key->schema != skey)) {
            LOGVAL(node->schema->module->ctx, LY_VLOG_LYD, node, LY_VCODE_NOKEY, skey->name);
            return LY_EVALID;
        }

        key = key->next;
    }

    return LY_SUCCESS;
}
