/**
 * @file tree_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang data structures
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */
#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "parser.h"
#include "resolve.h"
#include "xml.h"
#include "tree_internal.h"
#include "validation.h"

API struct lyd_node *
lyd_parse(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options)
{
    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LYD_XML:
        return xml_read_data(ctx, data, options);
    case LYD_JSON:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
}

API int
lyd_insert(struct lyd_node *parent, struct lyd_node *node, int options)
{
    struct lys_node *sparent;
    struct lyd_node *iter, *next, *last;

    if (!node || !parent) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink(node);
    }

    /* check placing the node to the appropriate place according to the schema */
    sparent = node->schema->parent;
    while (!(sparent->nodetype & (LYS_CONTAINER | LYS_LIST))) {
        sparent = sparent->parent;
    }
    if (sparent != parent->schema) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (!parent->child) {
        /* add as the only child of the parent */
        parent->child = node;
    } else {
        /* add as the last child of the parent */
        parent->child->prev->next = node;
        node->prev = parent->child->prev;
        for (iter = node; iter->next; iter = iter->next);
        parent->child->prev = iter;
    }
    LY_TREE_FOR(node, iter) {
        iter->parent = parent;
        last = iter; /* remember the last of the inserted nodes */
    }

    ly_errno = 0;
    LY_TREE_FOR_SAFE(node, next, iter) {
        /* various validation checks */
        if (lyv_data_content(iter, 0, options, NULL)) {
            if (ly_errno) {
                return EXIT_FAILURE;
            } else {
                lyd_free(iter);
            }
        }

        if (iter == last) {
            /* we are done - checking only the inserted nodes */
            break;
        }
    }

    return EXIT_SUCCESS;
}

API int
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node, int options)
{
    struct lys_node *par1, *par2;
    struct lyd_node *iter, *next, *last;

    if (!node || !sibling) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink(node);
    }

    /* check placing the node to the appropriate place according to the schema */
    for (par1 = sibling->schema->parent; par1 && (par1->nodetype & (LYS_CONTAINER | LYS_LIST)); par1 = par1->parent);
    for (par2 = node->schema->parent; par2 && (par2->nodetype & (LYS_CONTAINER | LYS_LIST)); par2 = par2->parent);
    if (par1 != par2) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    LY_TREE_FOR(node, iter) {
        iter->parent = sibling->parent;
        last = iter; /* remember the last of the inserted nodes */
    }

    if (sibling->next) {
        /* adding into a middle - fix the prev pointer of the node after inserted nodes */
        last->next = sibling->next;
        sibling->next->prev = last;
    } else {
        /* at the end - fix the prev pointer of the first node */
        if (sibling->parent) {
            sibling->parent->child->prev = last;
        } else {
            for (iter = sibling; iter->prev->next; iter = iter->prev);
            iter->prev = last;
        }
    }
    sibling->next = node;
    node->prev = sibling;

    ly_errno = 0;
    LY_TREE_FOR_SAFE(node, next, iter) {
        /* various validation checks */
        if (lyv_data_content(iter, 0, options, NULL)) {
            if (ly_errno) {
                return EXIT_FAILURE;
            } else {
                lyd_free(iter);
            }
        }

        if (iter == last) {
            /* we are done - checking only the inserted nodes */
            break;
        }
    }

    return EXIT_SUCCESS;
}

API int
lyd_unlink(struct lyd_node *node)
{
    struct lyd_node *iter;

    if (!node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* unlink from siblings */
    if (node->prev->next) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last node */
        iter = node->prev;
        while (iter->prev != node) {
            iter = iter->prev;
        }
        /* update the "last" pointer from the first node */
        iter->prev = node->prev;
    }

    /* unlink from parent */
    if (node->parent) {
        if (node->parent->child == node) {
            /* the node is the first child */
            node->parent->child = node->next;
        }
        node->parent = NULL;
    }

    node->next = NULL;
    node->prev = node;

    return EXIT_SUCCESS;
}

static void
lyd_attr_free(struct ly_ctx *ctx, struct lyd_attr *attr)
{
    if (!attr) {
        return;
    }

    if (attr->next) {
        lyd_attr_free(ctx, attr->next);
    }
    lydict_remove(ctx, attr->name);
    lydict_remove(ctx, attr->value);
    free(attr);
}

struct lyd_node *
lyd_attr_parent(struct lyd_node *root, struct lyd_attr *attr)
{
    struct lyd_node *next, *elem;
    struct lyd_attr *node_attr;

    LY_TREE_DFS_BEGIN(root, next, elem) {
        for (node_attr = elem->attr; node_attr; node_attr = node_attr->next) {
            if (node_attr == attr) {
                return elem;
            }
        }
        LY_TREE_DFS_END(root, next, elem)
    }

    return NULL;
}

API void
lyd_free(struct lyd_node *node)
{
    struct lyd_node *next, *child;

    if (!node) {
        return;
    }

    if (!(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
        /* free children */
        LY_TREE_FOR_SAFE(node->child, next, child) {
            lyd_free(child);
        }
    } else if (node->schema->nodetype == LYS_ANYXML) {
        lyxml_free_elem(node->schema->module->ctx, ((struct lyd_node_anyxml *)node)->value);
    } else {
        /* free value */
        switch(((struct lyd_node_leaf *)node)->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_leaf *)node)->value.string);
            break;
        case LY_TYPE_BITS:
            if (((struct lyd_node_leaf *)node)->value.bit) {
                free(((struct lyd_node_leaf *)node)->value.bit);
            }
            break;
        default:
            /* TODO nothing needed : LY_TYPE_BOOL, LY_TYPE_DEC64*/
            break;
        }
    }

    lyd_unlink(node);
    lyd_attr_free(node->schema->module->ctx, node->attr);
    free(node);
}

int
lyd_compare(struct lyd_node *first, struct lyd_node *second, int unique)
{
    struct lys_node_list *slist;
    struct lys_node *snode;
    struct lyd_node *diter;
    const char *val1, *val2;
    int i, j;

    assert(first);
    assert(second);

    if (first->schema != second->schema) {
        return 1;
    }

    switch (first->schema->nodetype) {
    case LYS_LEAFLIST:
        /* compare values */
        if (((struct lyd_node_leaflist *)first)->value_str == ((struct lyd_node_leaflist *)second)->value_str) {
            return 0;
        }
        return 1;
    case LYS_LIST:
        slist = (struct lys_node_list*)first->schema;

        if (unique) {
            /* compare unique leafs */
            for (i = 0; i < slist->unique_size; i++) {
                for (j = 0; j < slist->unique[i].leafs_size; j++) {
                    snode = (struct lys_node *)slist->unique[i].leafs[j];
                    /* use default values if the instances of unique leafs are not present */
                    val1 = val2 = ((struct lys_node_leaf *)snode)->dflt;
                    LY_TREE_FOR(first->child, diter) {
                        if (diter->schema == snode) {
                            val1 = ((struct lyd_node_leaf *)diter)->value_str;
                            break;
                        }
                    }
                    LY_TREE_FOR(second->child, diter) {
                        if (diter->schema == snode) {
                            val2 = ((struct lyd_node_leaf *)diter)->value_str;
                            break;
                        }
                    }
                    if (val1 != val2) {
                        break;
                    }
                }
                if (j && j == slist->unique[i].leafs_size) {
                    /* all unique leafs are the same in this set */
                    return 0;
                }
            }
        }

        /* compare keys */
        for (i = 0; i < slist->keys_size; i++) {
            snode = (struct lys_node *)slist->keys[i];
            val1 = val2 = NULL;
            LY_TREE_FOR(first->child, diter) {
                if (diter->schema == snode) {
                    val1 = ((struct lyd_node_leaf *)diter)->value_str;
                    break;
                }
            }
            LY_TREE_FOR(second->child, diter) {
                if (diter->schema == snode) {
                    val2 = ((struct lyd_node_leaf *)diter)->value_str;
                    break;
                }
            }
            if (val1 != val2) {
                return 1;
            }
        }

        return 0;
    default:
        /* no additional check is needed */
        return 0;
    }
}

API struct lyd_set *
lyd_set_new(void)
{
    return calloc(1, sizeof(struct lyd_set));
}

API void
lyd_set_free(struct lyd_set *set)
{
    if (!set) {
        return;
    }

    free(set->set);
    free(set);
}

API int
lyd_set_add(struct lyd_set *set, struct lyd_node *node)
{
    struct lyd_node **new;

    if (!set) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (set->size == set->number) {
        new = realloc(set->set, (set->size + 8) * sizeof *(set->set));
        if (!new) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        set->size += 8;
        set->set = new;
    }

    set->set[set->number++] = node;

    return EXIT_SUCCESS;
}
