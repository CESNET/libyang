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

#include "common.h"

#include <assert.h>
#include <stdlib.h>

#include "hash_table.h"
#include "log.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_data_internal.h"
#include "plugins_types.h"

void
lyd_value_free_path(const struct ly_ctx *ctx, struct lyd_value_path *path)
{
    unsigned int u, v;

    LY_ARRAY_FOR(path, u) {
        LY_ARRAY_FOR(path[u].predicates, v) {
            if (path[u].predicates[v].type > 0) {
                ((struct lysc_node_leaf*)path[u].predicates[v].key)->type->plugin->free(ctx, path[u].predicates[v].value);
                free(path[u].predicates[v].value);
            }
        }
        LY_ARRAY_FREE(path[u].predicates);
    }
    LY_ARRAY_FREE(path);
}

API void
lyd_free_meta(const struct ly_ctx *ctx, struct lyd_meta *meta, int recursive)
{
    struct lyd_meta *iter;

    LY_CHECK_ARG_RET(NULL, ctx, );
    if (!meta) {
        return;
    }

    if (meta->parent) {
        if (meta->parent->meta == meta) {
            if (recursive) {
                meta->parent->meta = NULL;
            } else {
                meta->parent->meta = meta->next;
            }
        } else {
            for (iter = meta->parent->meta; iter->next != meta; iter = iter->next);
            if (iter->next) {
                if (recursive) {
                    iter->next = NULL;
                } else {
                    iter->next = meta->next;
                }
            }
        }
    }

    if (!recursive) {
        meta->next = NULL;
    }

    for (iter = meta; iter; ) {
        meta = iter;
        iter = iter->next;

        FREE_STRING(ctx, meta->name);
        meta->value.realtype->plugin->free(ctx, &meta->value);
        free(meta);
    }
}

API void
ly_free_attr(const struct ly_ctx *ctx, struct ly_attr *attr, int recursive)
{
    struct ly_attr *iter;
    uint32_t u;

    LY_CHECK_ARG_RET(NULL, ctx, );
    if (!attr) {
        return;
    }

    if (attr->parent) {
        if (attr->parent->attr == attr) {
            if (recursive) {
                attr->parent->attr = NULL;
            } else {
                attr->parent->attr = attr->next;
            }
        } else {
            for (iter = attr->parent->attr; iter->next != attr; iter = iter->next);
            if (iter->next) {
                if (recursive) {
                    iter->next = NULL;
                } else {
                    iter->next = attr->next;
                }
            }
        }
    }

    if (!recursive) {
        attr->next = NULL;
    }

    for (iter = attr; iter; ) {
        attr = iter;
        iter = iter->next;

        LY_ARRAY_FOR(attr->val_prefs, u) {
            FREE_STRING(ctx, attr->val_prefs[u].pref);
            FREE_STRING(ctx, attr->val_prefs[u].ns);
        }
        LY_ARRAY_FREE(attr->val_prefs);
        FREE_STRING(ctx, attr->name);
        FREE_STRING(ctx, attr->value);
        FREE_STRING(ctx, attr->prefix.pref);
        FREE_STRING(ctx, attr->prefix.ns);
        free(attr);
    }
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
    struct lyd_node_opaq *opaq;
    uint32_t u;

    assert(node);

    if (!node->schema) {
        opaq = (struct lyd_node_opaq *)node;

        /* free the children */
        children = (struct lyd_node *)lyd_node_children(node);
        LY_LIST_FOR_SAFE(children, next, iter) {
            lyd_free_subtree(iter, 0);
        }

        FREE_STRING(LYD_NODE_CTX(opaq), opaq->name);
        FREE_STRING(LYD_NODE_CTX(opaq), opaq->prefix.pref);
        FREE_STRING(LYD_NODE_CTX(opaq), opaq->prefix.ns);
        if (opaq->val_prefs) {
            LY_ARRAY_FOR(opaq->val_prefs, u) {
                FREE_STRING(LYD_NODE_CTX(opaq), opaq->val_prefs[u].pref);
                FREE_STRING(LYD_NODE_CTX(opaq), opaq->val_prefs[u].ns);
            }
            LY_ARRAY_FREE(opaq->val_prefs);
        }
        FREE_STRING(LYD_NODE_CTX(opaq), opaq->value);
    } else if (node->schema->nodetype & LYD_NODE_INNER) {
        /* remove children hash table in case of inner data node */
        lyht_free(((struct lyd_node_inner *)node)->children_ht);
        ((struct lyd_node_inner *)node)->children_ht = NULL;

        /* free the children */
        children = (struct lyd_node *)lyd_node_children(node);
        LY_LIST_FOR_SAFE(children, next, iter) {
            lyd_free_subtree(iter, 0);
        }
    } else if (node->schema->nodetype & LYD_NODE_ANY) {
        switch (((struct lyd_node_any *)node)->value_type) {
        case LYD_ANYDATA_DATATREE:
            lyd_free_all(((struct lyd_node_any *)node)->value.tree);
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_XML:
        case LYD_ANYDATA_JSON:
            FREE_STRING(LYD_NODE_CTX(node), ((struct lyd_node_any *)node)->value.str);
            break;
#if 0 /* TODO LYB format */
        case LYD_ANYDATA_LYB:
            free(((struct lyd_node_any *)node)->value.mem);
            break;
#endif
        }
    } else if (node->schema->nodetype & LYD_NODE_TERM) {
        ((struct lysc_node_leaf *)node->schema)->type->plugin->free(LYD_NODE_CTX(node), &((struct lyd_node_term *)node)->value);
    }

    if (!node->schema) {
        ly_free_attr(LYD_NODE_CTX(node), opaq->attr, 1);
    } else {
        /* free the node's metadata */
        lyd_free_meta(LYD_NODE_CTX(node), node->meta, 1);
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
