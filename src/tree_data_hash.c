/**
 * @file tree_data_hash.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Functions to manipulate with the data node's hashes.
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

#include "hash_table.h"
#include "tree_data.h"
#include "plugins_types.h"


static void
lyd_hash_keyless_list_dfs(struct lyd_node *child, uint32_t *hash)
{
    struct lyd_node *iter;

    LY_LIST_FOR(child, iter) {
        switch (iter->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
            lyd_hash_keyless_list_dfs(((struct lyd_node_inner*)iter)->child, hash);
            break;
        case LYS_LEAFLIST:
        case LYS_ANYXML:
        case LYS_ANYDATA:
        case LYS_LEAF:
            *hash = dict_hash_multi(*hash, (char *)&iter->hash, sizeof iter->hash);
            break;
        default:
            LOGINT(NULL);
        }
    }
}

LY_ERR
lyd_hash(struct lyd_node *node)
{
    struct lyd_node *iter;

    if (!node->schema) {
        return LY_SUCCESS;
    }

    node->hash = dict_hash_multi(0, node->schema->module->name, strlen(node->schema->module->name));
    node->hash = dict_hash_multi(node->hash, node->schema->name, strlen(node->schema->name));

    if (node->schema->nodetype == LYS_LIST) {
        struct lyd_node_inner *list = (struct lyd_node_inner*)node;
        if (!(node->schema->flags & LYS_KEYLESS)) {
            /* list's hash is made of its keys */
            struct lysc_node *key;
            for (key = ((struct lysc_node_list*)node->schema)->child, iter = list->child;
                    key && key->nodetype == LYS_LEAF && (key->flags & LYS_KEY) && iter;
                    key = key->next, iter = iter->next) {
                for ( ; iter && iter->schema != key; iter = iter->next);
                if (!iter) {
                    break;
                }
                int dynamic = 0;
                struct lysc_type *type = ((struct lysc_node_leaf*)iter->schema)->type;
                const char *value = type->plugin->print(&((struct lyd_node_term*)iter)->value, LYD_JSON, json_print_get_prefix, NULL, &dynamic);
                node->hash = dict_hash_multi(node->hash, value, strlen(value));
                if (dynamic) {
                    free((char*)value);
                }
            }
        } else {
            /* keyless status list */
            lyd_hash_keyless_list_dfs(list->child, &node->hash);
        }
    } else if (node->schema->nodetype == LYS_LEAFLIST) {
        struct lyd_node_term *llist = (struct lyd_node_term*)node;
        int dynamic = 0;
        const char *value = ((struct lysc_node_leaflist*)node->schema)->type->plugin->print(&llist->value, LYD_JSON,
                                                                                            json_print_get_prefix, NULL, &dynamic);
        node->hash = dict_hash_multi(node->hash, value, strlen(value));
        if (dynamic) {
            free((char*)value);
        }
    }
    /* finish the hash */
    node->hash = dict_hash_multi(node->hash, NULL, 0);

    return LY_SUCCESS;
}

static int
lyd_hash_table_val_equal(void *val1_p, void *val2_p, int mod, void *UNUSED(cb_data))
{
    struct lyd_node *val1, *val2;

    val1 = *((struct lyd_node **)val1_p);
    val2 = *((struct lyd_node **)val2_p);

    if (mod) {
        if (val1 == val2) {
            return 1;
        } else {
            return 0;
        }
    }

    if (!lyd_compare(val1, val2, 0)) {
        return 1;
    } else {
        return 0;
    }
}

LY_ERR
lyd_insert_hash(struct lyd_node *node)
{
    struct lyd_node *iter;

    if (!node->parent || !node->schema || !node->parent->schema) {
        /* nothing to do */
        return LY_SUCCESS;
    }

    /* create parent hash table if required, otherwise just add the new child */
    if (!node->parent->children_ht) {
        unsigned int u;

        /* the hash table is created only when the number of children in a node exceeds the
         * defined minimal limit LY_CACHE_HT_MIN_CHILDREN
         */
        for (u = 0, iter = node->parent->child; iter; ++u, iter = iter->next);
        if (u >= LYD_HT_MIN_ITEMS) {
            /* create hash table, insert all the children */
            node->parent->children_ht = lyht_new(1, sizeof(struct lyd_node *), lyd_hash_table_val_equal, NULL, 1);
            LY_LIST_FOR(node->parent->child, iter) {
                if (lyht_insert(node->parent->children_ht, &iter, iter->hash, NULL)) {
                    LOGINT(node->schema->module->ctx);
                    return LY_EINT;
                }
            }
        }
    } else {
        if (lyht_insert(node->parent->children_ht, &node, node->hash, NULL)) {
            LOGINT(node->schema->module->ctx);
            return LY_EINT;
        }
    }

    return LY_SUCCESS;
}

void
lyd_unlink_hash(struct lyd_node *node)
{
    if (node->parent && node->parent->children_ht) {
        lyht_remove(node->parent->children_ht, &node, node->hash);
    }
}
