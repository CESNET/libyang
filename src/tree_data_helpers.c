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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "hash_table.h"
#include "log.h"
#include "lyb.h"
#include "parser_data.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
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
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
            return &((struct lyd_node_inner *)node)->child;
        default:
            return NULL;
        }
    }
}

API struct lyd_node *
lyd_node_children(const struct lyd_node *node, int options)
{
    struct lyd_node **children, *child;

    if (!node) {
        return NULL;
    }

    children = lyd_node_children_p((struct lyd_node *)node);
    if (children) {
        child = *children;
        if (options & LYD_CHILDREN_SKIP_KEYS) {
            while (child && child->schema && (child->schema->flags & LYS_KEY)) {
                child = child->next;
            }
        }
        return child;
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

    key = lyd_node_children(node, 0);
    while ((skey = lys_getnext(skey, node->schema, NULL, 0)) && (skey->flags & LYS_KEY)) {
        if (!key || (key->schema != skey)) {
            LOGVAL(node->schema->module->ctx, LY_VLOG_LYD, node, LY_VCODE_NOKEY, skey->name);
            return LY_EVALID;
        }

        key = key->next;
    }

    return LY_SUCCESS;
}

void
lyd_parse_set_data_flags(struct lyd_node *node, struct ly_set *when_check, struct lyd_meta **meta, int options)
{
    struct lyd_meta *meta2, *prev_meta = NULL;

    if (!(node->schema->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) && node->schema->when) {
        if (options & LYD_PARSE_TRUSTED) {
            /* just set it to true */
            node->flags |= LYD_WHEN_TRUE;
        } else {
            /* remember we need to evaluate this node's when */
            ly_set_add(when_check, node, LY_SET_OPT_USEASLIST);
        }
    }

    if (options & LYD_PARSE_TRUSTED) {
        /* node is valid */
        node->flags &= ~LYD_NEW;
    }

    LY_LIST_FOR(*meta, meta2) {
        if (!strcmp(meta2->name, "default") && !strcmp(meta2->annotation->module->name, "ietf-netconf-with-defaults")
                && meta2->value.boolean) {
            /* node is default according to the metadata */
            node->flags |= LYD_DEFAULT;

            /* delete the metadata */
            if (prev_meta) {
                prev_meta->next = meta2->next;
            } else {
                *meta = (*meta)->next;
            }
            lyd_free_meta(LYD_NODE_CTX(node), meta2, 0);
            break;
        }

        prev_meta = meta2;
    }
}

LYB_HASH
lyb_hash(struct lysc_node *sibling, uint8_t collision_id)
{
    const struct lys_module *mod;
    int ext_len;
    uint32_t full_hash;
    LYB_HASH hash;

    if ((collision_id < LYS_NODE_HASH_COUNT) && sibling->hash[collision_id]) {
        return sibling->hash[collision_id];
    }

    mod = sibling->module;

    full_hash = dict_hash_multi(0, mod->name, strlen(mod->name));
    full_hash = dict_hash_multi(full_hash, sibling->name, strlen(sibling->name));
    if (collision_id) {
        if (collision_id > strlen(mod->name)) {
            /* fine, we will not hash more bytes, just use more bits from the hash than previously */
            ext_len = strlen(mod->name);
        } else {
            /* use one more byte from the module name than before */
            ext_len = collision_id;
        }
        full_hash = dict_hash_multi(full_hash, mod->name, ext_len);
    }
    full_hash = dict_hash_multi(full_hash, NULL, 0);

    /* use the shortened hash */
    hash = full_hash & (LYB_HASH_MASK >> collision_id);
    /* add colision identificator */
    hash |= LYB_HASH_COLLISION_ID >> collision_id;

    /* save this hash */
    if (collision_id < LYS_NODE_HASH_COUNT) {
        sibling->hash[collision_id] = hash;
    }

    return hash;
}

int
lyb_has_schema_model(const struct lysc_node *sibling, const struct lys_module **models)
{
    LY_ARRAY_SIZE_TYPE u;

    LY_ARRAY_FOR(models, u) {
        if (sibling->module == models[u]) {
            return 1;
        }
    }

    return 0;
}
