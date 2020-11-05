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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "hash_table.h"
#include "log.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"

static void
lyd_hash_keyless_list_dfs(struct lyd_node *child, uint32_t *hash)
{
    struct lyd_node *iter;

    LY_LIST_FOR(child, iter) {
        switch (iter->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
            lyd_hash_keyless_list_dfs(((struct lyd_node_inner *)iter)->child, hash);
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
        struct lyd_node_inner *list = (struct lyd_node_inner *)node;
        if (!(node->schema->flags & LYS_KEYLESS)) {
            /* list's hash is made of its keys */
            for (iter = list->child; iter && (iter->schema->flags & LYS_KEY); iter = iter->next) {
                const char *value = LYD_CANON_VALUE(iter);
                node->hash = dict_hash_multi(node->hash, value, strlen(value));
            }
        } else {
            /* keyless status list */
            lyd_hash_keyless_list_dfs(list->child, &node->hash);
        }
    } else if (node->schema->nodetype == LYS_LEAFLIST) {
        const char *value = LYD_CANON_VALUE(node);
        node->hash = dict_hash_multi(node->hash, value, strlen(value));
    }
    /* finish the hash */
    node->hash = dict_hash_multi(node->hash, NULL, 0);

    return LY_SUCCESS;
}

/**
 * @brief Compare callback for values in hash table.
 *
 * Implementation of ::values_equal_cb.
 */
static ly_bool
lyd_hash_table_val_equal(void *val1_p, void *val2_p, ly_bool mod, void *UNUSED(cb_data))
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

    if (val1->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* match on exact instance */
        if (!lyd_compare_single(val1, val2, 0)) {
            return 1;
        }
    } else if (val1->schema == val2->schema) {
        /* just schema match */
        return 1;
    }
    return 0;
}

/**
 * @brief Add single node into children hash table.
 *
 * @param[in] ht Children hash table.
 * @param[in] node Node to insert.
 * @param[in] empty_ht Whether we started with an empty HT meaning no nodes were inserted yet.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_insert_hash_add(struct hash_table *ht, struct lyd_node *node, ly_bool empty_ht)
{
    uint32_t hash;

    assert(ht && node && node->schema);

    /* add node itself */
    if (lyht_insert(ht, &node, node->hash, NULL)) {
        LOGINT(LYD_CTX(node));
        return LY_EINT;
    }

    /* add first instance of a (leaf-)list */
    if ((node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) &&
            (!node->prev->next || (node->prev->schema != node->schema))) {
        /* get the simple hash */
        hash = dict_hash_multi(0, node->schema->module->name, strlen(node->schema->module->name));
        hash = dict_hash_multi(hash, node->schema->name, strlen(node->schema->name));
        hash = dict_hash_multi(hash, NULL, 0);

        /* remove any previous stored instance, only if we did not start with an empty HT */
        if (!empty_ht && node->next && (node->next->schema == node->schema)) {
            if (lyht_remove(ht, &node->next, hash)) {
                LOGINT(LYD_CTX(node));
                return LY_EINT;
            }
        }

        /* insert this instance as the first (leaf-)list instance */
        if (lyht_insert(ht, &node, hash, NULL)) {
            LOGINT(LYD_CTX(node));
            return LY_EINT;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_insert_hash(struct lyd_node *node)
{
    struct lyd_node *iter;
    uint32_t u;

    if (!node->parent || !node->schema || !node->parent->schema) {
        /* nothing to do */
        return LY_SUCCESS;
    }

    /* create parent hash table if required, otherwise just add the new child */
    if (!node->parent->children_ht) {
        /* the hash table is created only when the number of children in a node exceeds the
         * defined minimal limit LYD_HT_MIN_ITEMS
         */
        u = 0;
        LY_LIST_FOR(node->parent->child, iter) {
            if (iter->schema) {
                ++u;
            }
        }
        if (u >= LYD_HT_MIN_ITEMS) {
            /* create hash table, insert all the children */
            node->parent->children_ht = lyht_new(1, sizeof(struct lyd_node *), lyd_hash_table_val_equal, NULL, 1);
            LY_LIST_FOR(node->parent->child, iter) {
                if (iter->schema) {
                    LY_CHECK_RET(lyd_insert_hash_add(node->parent->children_ht, iter, 1));
                }
            }
        }
    } else {
        LY_CHECK_RET(lyd_insert_hash_add(node->parent->children_ht, node, 0));
    }

    return LY_SUCCESS;
}

void
lyd_unlink_hash(struct lyd_node *node)
{
    uint32_t hash;

    if (!node->parent || !node->schema || !node->parent->schema || !node->parent->children_ht) {
        /* not in any HT */
        return;
    }

    /* remove from the parent HT */
    if (lyht_remove(node->parent->children_ht, &node, node->hash)) {
        LOGINT(LYD_CTX(node));
        return;
    }

    /* first instance of the (leaf-)list, needs to be removed from HT */
    if ((node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) && (!node->prev->next || (node->prev->schema != node->schema))) {
        /* get the simple hash */
        hash = dict_hash_multi(0, node->schema->module->name, strlen(node->schema->module->name));
        hash = dict_hash_multi(hash, node->schema->name, strlen(node->schema->name));
        hash = dict_hash_multi(hash, NULL, 0);

        /* remove the instance */
        if (lyht_remove(node->parent->children_ht, &node, hash)) {
            LOGINT(LYD_CTX(node));
            return;
        }

        /* add the next instance */
        if (node->next && (node->next->schema == node->schema)) {
            if (lyht_insert(node->parent->children_ht, &node->next, hash, NULL)) {
                LOGINT(LYD_CTX(node));
                return;
            }
        }
    }
}
