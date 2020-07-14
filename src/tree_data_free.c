/**
 * @file tree_data_free.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Freeing functions for data tree structures
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */


#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "hash_table.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_data_internal.h"
#include "plugins_types.h"

static void
lyd_free_meta(struct lyd_meta *meta, int siblings)
{
    struct lyd_meta *iter;

    if (!meta) {
        return;
    }

    if (meta->parent) {
        if (meta->parent->meta == meta) {
            if (siblings) {
                meta->parent->meta = NULL;
            } else {
                meta->parent->meta = meta->next;
            }
        } else {
            for (iter = meta->parent->meta; iter->next != meta; iter = iter->next);
            if (iter->next) {
                if (siblings) {
                    iter->next = NULL;
                } else {
                    iter->next = meta->next;
                }
            }
        }
    }

    if (!siblings) {
        meta->next = NULL;
    }

    for (iter = meta; iter; ) {
        meta = iter;
        iter = iter->next;

        FREE_STRING(meta->annotation->module->ctx, meta->name);
        meta->value.realtype->plugin->free(meta->annotation->module->ctx, &meta->value);
        free(meta);
    }
}

API void
lyd_free_meta_single(struct lyd_meta *meta)
{
    lyd_free_meta(meta, 0);
}

API void
lyd_free_meta_siblings(struct lyd_meta *meta)
{
    lyd_free_meta(meta, 1);
}

static void
ly_free_attr(const struct ly_ctx *ctx, struct lyd_attr *attr, int siblings)
{
    struct lyd_attr *iter;
    LY_ARRAY_COUNT_TYPE u;

    LY_CHECK_ARG_RET(NULL, ctx, );
    if (!attr) {
        return;
    }

    if (attr->parent) {
        if (attr->parent->attr == attr) {
            if (siblings) {
                attr->parent->attr = NULL;
            } else {
                attr->parent->attr = attr->next;
            }
        } else {
            for (iter = attr->parent->attr; iter->next != attr; iter = iter->next);
            if (iter->next) {
                if (siblings) {
                    iter->next = NULL;
                } else {
                    iter->next = attr->next;
                }
            }
        }
    }

    if (!siblings) {
        attr->next = NULL;
    }

    for (iter = attr; iter; ) {
        attr = iter;
        iter = iter->next;

        LY_ARRAY_FOR(attr->val_prefs, u) {
            FREE_STRING(ctx, attr->val_prefs[u].id);
            FREE_STRING(ctx, attr->val_prefs[u].module_ns);
        }
        LY_ARRAY_FREE(attr->val_prefs);
        FREE_STRING(ctx, attr->name);
        FREE_STRING(ctx, attr->value);
        FREE_STRING(ctx, attr->prefix.id);
        FREE_STRING(ctx, attr->prefix.module_ns);
        free(attr);
    }
}

API void
ly_free_attr_single(const struct ly_ctx *ctx, struct lyd_attr *attr)
{
    ly_free_attr(ctx, attr, 0);
}

API void
ly_free_attr_siblings(const struct ly_ctx *ctx, struct lyd_attr *attr)
{
    ly_free_attr(ctx, attr, 1);
}

void
ly_free_val_prefs(const struct ly_ctx *ctx, struct ly_prefix *val_prefs)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(val_prefs, u) {
        FREE_STRING(ctx, val_prefs[u].id);
        FREE_STRING(ctx, val_prefs[u].module_ns);
    }
    LY_ARRAY_FREE(val_prefs);
}

/**
 * @brief Free Data (sub)tree.
 * @param[in] node Data node to be freed.
 * @param[in] top Recursion flag to unlink the root of the subtree being freed.
 */
static void
lyd_free_subtree(struct lyd_node *node, int top)
{
    struct lyd_node *iter, *next;
    struct lyd_node *children;
    struct lyd_node_opaq *opaq = NULL;

    assert(node);

    if (!node->schema) {
        opaq = (struct lyd_node_opaq *)node;

        /* free the children */
        children = lyd_node_children(node, 0);
        LY_LIST_FOR_SAFE(children, next, iter) {
            lyd_free_subtree(iter, 0);
        }

        FREE_STRING(LYD_NODE_CTX(opaq), opaq->name);
        FREE_STRING(LYD_NODE_CTX(opaq), opaq->prefix.id);
        FREE_STRING(LYD_NODE_CTX(opaq), opaq->prefix.module_ns);
        ly_free_val_prefs(LYD_NODE_CTX(opaq), opaq->val_prefs);
        FREE_STRING(LYD_NODE_CTX(opaq), opaq->value);
    } else if (node->schema->nodetype & LYD_NODE_INNER) {
        /* remove children hash table in case of inner data node */
        lyht_free(((struct lyd_node_inner *)node)->children_ht);
        ((struct lyd_node_inner *)node)->children_ht = NULL;

        /* free the children */
        children = lyd_node_children(node, 0);
        LY_LIST_FOR_SAFE(children, next, iter) {
            lyd_free_subtree(iter, 0);
        }
    } else if (node->schema->nodetype & LYD_NODE_ANY) {
        /* only frees the value this way */
        lyd_any_copy_value(node, NULL, 0);
    } else if (node->schema->nodetype & LYD_NODE_TERM) {
        ((struct lysc_node_leaf *)node->schema)->type->plugin->free(LYD_NODE_CTX(node), &((struct lyd_node_term *)node)->value);
    }

    if (!node->schema) {
        ly_free_attr_siblings(LYD_NODE_CTX(node), opaq->attr);
    } else {
        /* free the node's metadata */
        lyd_free_meta_siblings(node->meta);
    }

    /* unlink only the nodes from the first level, nodes in subtree are freed all, so no unlink is needed */
    if (top) {
        lyd_unlink_tree(node);
    }

    free(node);
}

API void
lyd_free_tree(struct lyd_node *node)
{
    if (!node) {
        return;
    }

    lyd_free_subtree(node, 1);
}

static void
lyd_free_(struct lyd_node *node, int top)
{
    struct lyd_node *iter, *next;

    if (!node) {
        return;
    }

    /* get the first (top-level) sibling */
    if (top) {
        for (; node->parent; node = (struct lyd_node *)node->parent);
    }
    while (node->prev->next) {
        node = node->prev;
    }

    LY_LIST_FOR_SAFE(node, next, iter) {
        /* in case of the top-level nodes (node->parent is NULL), no unlinking needed */
        lyd_free_subtree(iter, iter->parent ? 1 : 0);
    }
}

API void
lyd_free_siblings(struct lyd_node *node)
{
    lyd_free_(node, 0);
}

API void
lyd_free_all(struct lyd_node *node)
{
    lyd_free_(node, 1);
}
