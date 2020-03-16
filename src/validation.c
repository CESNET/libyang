/**
 * @file validation.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Validation
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "xpath.h"
#include "tree_data_internal.h"
#include "tree_schema_internal.h"

static struct lyd_node *
lys_getnext_data(const struct lyd_node *last, const struct lyd_node *sibling, const struct lysc_node **slast,
                 const struct lysc_node *parent, const struct lysc_module *module)
{
    const struct lysc_node *siter = NULL;
    struct lyd_node *match = NULL;

    assert(parent || module);
    assert(!last || (slast && *slast));

    if (slast) {
        siter = *slast;
    }

    if (last && last->next) {
        /* find next data instance */
        lyd_find_sibling_next2(last->next, siter, NULL, 0, &match);
        if (match) {
            return match;
        }
    }

    /* find next schema node data instance */
    while ((siter = lys_getnext(siter, parent, module, 0))) {
        switch (siter->nodetype) {
        case LYS_CONTAINER:
        case LYS_ANYXML:
        case LYS_ANYDATA:
        case LYS_LEAF:
            lyd_find_sibling_val(sibling, siter, NULL, 0, &match);
            break;
        case LYS_LIST:
        case LYS_LEAFLIST:
            lyd_find_sibling_next2(sibling, siter, NULL, 0, &match);
            break;
        default:
            assert(0);
            LOGINT(NULL);
        }

        if (match) {
            break;
        }
    }

    if (slast) {
        *slast = siter;
    }
    return match;
}

/**
 * @brief Evaluate a single "when" condition.
 *
 * @param[in,out] tree Data tree, is updated if some nodes are autodeleted.
 * @param[in] node Node whose existence depends on this when.
 * @param[in] when When to evaluate.
 * @return LY_ERR value (LY_EINCOMPLETE if a referenced node does not have its when evaluated)
 */
static LY_ERR
lyd_validate_when(struct lyd_node **tree, struct lyd_node *node, struct lysc_when *when)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyd_node *ctx_node;
    struct lyxp_set xp_set;

    memset(&xp_set, 0, sizeof xp_set);

    if (when->context == node->schema) {
        ctx_node = node;
    } else {
        assert((!when->context && !node->parent) || (when->context == node->parent->schema));
        ctx_node = (struct lyd_node *)node->parent;
    }

    /* evaluate when */
    ret = lyxp_eval(when->cond, LYD_SCHEMA, when->module, ctx_node, ctx_node ? LYXP_NODE_ELEM : LYXP_NODE_ROOT_CONFIG,
                    *tree, &xp_set, LYXP_SCHEMA);
    lyxp_set_cast(&xp_set, LYXP_SET_BOOLEAN);

    /* return error or LY_EINCOMPLETE for dependant unresolved when */
    LY_CHECK_RET(ret);

    /* take action based on the result */
    if (!xp_set.val.bool) {
        if (node->flags & LYD_WHEN_TRUE) {
            /* autodelete */
            if (LYD_DEL_IS_ROOT(*tree, node)) {
                *tree = (*tree)->next;
            }
            lyd_free_tree(node);
        } else {
            /* invalid data */
            LOGVAL(node->schema->module->ctx, LY_VLOG_LYD, node, LY_VCODE_NOWHEN, when->cond->expr);
            ret = LY_EVALID;
        }
    } else {
        /* remember that when evaluated to true */
        node->flags |= LYD_WHEN_TRUE;
    }

    return ret;
}

LY_ERR
lyd_validate_unres(struct lyd_node **tree, struct ly_set *node_when, struct ly_set *node_types, struct ly_set *meta_types,
                   LYD_FORMAT format, ly_clb_resolve_prefix get_prefix_clb, void *parser_data)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t u;

    if (node_when) {
        /* evaluate all when conditions */
        uint32_t prev_count;
        do {
            prev_count = node_when->count;
            u = 0;
            while (u < node_when->count) {
                /* evaluate all when expressions that affect this node's existence */
                struct lyd_node *node = (struct lyd_node *)node_when->objs[u];
                const struct lysc_node *schema = node->schema;
                int unres_when = 0;

                do {
                    uint32_t i;
                    LY_ARRAY_FOR(schema->when, i) {
                        ret = lyd_validate_when(tree, node, schema->when[i]);
                        if (ret) {
                            break;
                        }
                    }
                    if (ret == LY_EINCOMPLETE) {
                        /* could not evaluate this when */
                        unres_when = 1;
                        break;
                    } else if (ret) {
                        /* error */
                        return ret;
                    }
                    schema = schema->parent;
                } while (schema && (schema->nodetype & (LYS_CASE | LYS_CHOICE)));

                if (unres_when) {
                    /* keep in set and go to the next node */
                    ++u;
                } else {
                    /* remove this node from the set */
                    ly_set_rm_index(node_when, u, NULL);
                }
            }

        /* there must have been some when conditions resolved */
        } while (prev_count > node_when->count);

        /* there could have been no cyclic when dependencies, checked during compilation */
        assert(!node_when->count);
    }

    if (node_types && node_types->count) {
        /* finish incompletely validated terminal values (traverse from the end for efficient set removal) */
        u = node_types->count;
        do {
            --u;

            struct lyd_node_term *node = (struct lyd_node_term *)node_types->objs[u];

            /* validate and store the value of the node */
            ret = lyd_value_parse(node, node->value.original, strlen(node->value.original), 0, 1, get_prefix_clb,
                                  parser_data, format, *tree);
            LY_CHECK_RET(ret);

            /* remove this node from the set */
            ly_set_rm_index(node_types, u, NULL);
        } while (u);
    }

    if (meta_types && meta_types->count) {
        /* ... and metadata values */
        u = meta_types->count;
        do {
            --u;

            struct lyd_meta *meta = (struct lyd_meta *)meta_types->objs[u];

            /* validate and store the value of the metadata */
            ret = lyd_value_parse_meta(meta->parent->schema->module->ctx, meta, meta->value.original,
                                       strlen(meta->value.original), 0, 1, get_prefix_clb, parser_data, format, NULL, *tree);
            LY_CHECK_RET(ret);

            /* remove this attr from the set */
            ly_set_rm_index(meta_types, u, NULL);
        } while (u);
    }

    return ret;
}

static LY_ERR
lyd_validate_duplicates(const struct lyd_node *first, const struct lyd_node *node)
{
    struct lyd_node **match_p;
    int fail = 0;

    if ((node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) && (node->schema->flags & LYS_CONFIG_R)) {
        /* duplicate instances allowed */
        return LY_SUCCESS;
    }

    /* find exactly the same next instance using hashes if possible */
    if (node->parent && node->parent->children_ht) {
        if (!lyht_find_next(node->parent->children_ht, &node, node->hash, (void **)&match_p)) {
            fail = 1;
        }
    } else {
        for (; first; first = first->next) {
            if (first == node) {
                continue;
            }

            if (node->schema->nodetype & (LYD_NODE_ANY | LYS_LEAF)) {
                if (first->schema == node->schema) {
                    fail = 1;
                    break;
                }
            } else if (!lyd_compare(first, node, 0)) {
                fail = 1;
                break;
            }
        }
    }

    if (fail) {
        LOGVAL(node->schema->module->ctx, LY_VLOG_LYD, node, LY_VCODE_DUP, node->schema->name);
        return LY_EVALID;
    }
    return LY_SUCCESS;
}

static LY_ERR
lyd_validate_cases(struct lyd_node **first, const struct lysc_node_choice *choic)
{
    const struct lysc_node *scase, *iter, *old_case = NULL, *new_case = NULL;
    struct lyd_node *match, *to_del;
    int found;

    LY_LIST_FOR((struct lysc_node *)choic->cases, scase) {
        found = 0;
        iter = NULL;
        match = NULL;
        while ((match = lys_getnext_data(match, *first, &iter, scase, NULL))) {
            if (match->flags & LYD_NEW) {
                /* a new case data found, nothing more to look for */
                found = 2;
                break;
            } else {
                /* and old case data found */
                if (found == 0) {
                    found = 1;
                }
            }
        }

        if (found == 1) {
            /* there should not be 2 old cases */
            if (old_case) {
                /* old data from 2 cases */
                LOGVAL(choic->module->ctx, LY_VLOG_LYSC, choic, LY_VCODE_DUPCASE, old_case->name, scase->name);
                return LY_EVALID;
            }

            /* remember an old existing case */
            old_case = scase;
        } else if (found == 2) {
            if (new_case) {
                /* new data from 2 cases */
                LOGVAL(choic->module->ctx, LY_VLOG_LYSC, choic, LY_VCODE_DUPCASE, new_case->name, scase->name);
                return LY_EVALID;
            }

            /* remember a new existing case */
            new_case = scase;
        }
    }

    if (old_case && new_case) {
        /* auto-delete old case */
        iter = NULL;
        match = NULL;
        to_del = NULL;
        while ((match = lys_getnext_data(match, *first, &iter, old_case, NULL))) {
            if (LYD_DEL_IS_ROOT(*first, to_del)) {
                *first = (*first)->next;
            }
            lyd_free_tree(to_del);
            to_del = match;
        }
        if (LYD_DEL_IS_ROOT(*first, to_del)) {
            *first = (*first)->next;
        }
        lyd_free_tree(to_del);
    }

    return LY_SUCCESS;
}

static int
lyd_val_has_default(const struct lysc_node *schema)
{
    switch (schema->nodetype) {
    case LYS_LEAF:
        if (((struct lysc_node_leaf *)schema)->dflt) {
            return 1;
        }
        break;
    case LYS_LEAFLIST:
        if (((struct lysc_node_leaflist *)schema)->dflts) {
            return 1;
        }
        break;
    case LYS_CONTAINER:
        if (!(schema->flags & LYS_PRESENCE)) {
            return 1;
        }
        break;
    default:
        break;
    }

    return 0;
}

static void
lyd_validate_autodel_dup(struct lyd_node **first, struct lyd_node *node, struct lyd_node **next_p)
{
    struct lyd_node *match, *next;

    if (lyd_val_has_default(node->schema)) {
        assert(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_CONTAINER));
        if (node->schema->nodetype == LYS_LEAFLIST) {
            lyd_find_sibling_next2(*first, node->schema, NULL, 0, &match);
        } else {
            lyd_find_sibling_val(*first, node->schema, NULL, 0, &match);
        }

        while (match) {
            next = match->next;
            if ((match->flags & LYD_DEFAULT) && !(match->flags & LYD_NEW)) {
                /* default instance found, remove it */
                if (LYD_DEL_IS_ROOT(*first, match)) {
                    *first = (*first)->next;
                }
                if (match == *next_p) {
                    *next_p = (*next_p)->next;
                }
                lyd_free_tree(match);

                /* remove only a single container/leaf default instance, if there are more, it is an error */
                if (node->schema->nodetype & (LYS_LEAF | LYS_CONTAINER)) {
                    break;
                }
            }

            lyd_find_sibling_next2(next, node->schema, NULL, 0, &match);
        }
    }
}

LY_ERR
lyd_validate_new(struct lyd_node **first, const struct lysc_node *sparent, const struct lys_module *mod)
{
    struct lyd_node *next, *node;
    const struct lysc_node *snode = NULL;

    assert(first && (sparent || mod));

    while (*first && (snode = lys_getnext(snode, sparent, mod ? mod->compiled : NULL, LYS_GETNEXT_WITHCHOICE))) {
        /* check case duplicites */
        if (snode->nodetype == LYS_CHOICE) {
            LY_CHECK_RET(lyd_validate_cases(first, (struct lysc_node_choice *)snode));
        }
    }

    LY_LIST_FOR_SAFE(*first, next, node) {
        if (mod && (lyd_owner_module(node) != mod)) {
            /* all top-level data from this module checked */
            break;
        }

        if (!(node->flags & LYD_NEW)) {
            /* check only new nodes */
            continue;
        }

        /* remove old default(s) if it exists */
        lyd_validate_autodel_dup(first, node, &next);

        /* then check new node instance duplicities */
        LY_CHECK_RET(lyd_validate_duplicates(*first, node));

        /* this node is valid */
        node->flags &= ~LYD_NEW;
    }

    return LY_SUCCESS;
}

static LY_ERR
lyd_validate_mandatory(const struct lyd_node *first, const struct lysc_node *snode)
{
    if (snode->nodetype == LYS_CHOICE) {
        /* some data of a choice case exist */
        if (lys_getnext_data(NULL, first, NULL, snode, NULL)) {
            return LY_SUCCESS;
        }
    } else {
        assert(snode->nodetype & (LYS_LEAF | LYS_CONTAINER | LYD_NODE_ANY));

        if (!lyd_find_sibling_val(first, snode, NULL, 0, NULL)) {
            /* data instance found */
            return LY_SUCCESS;
        }
    }

    /* node instance not found */
    LOGVAL(snode->module->ctx, LY_VLOG_LYSC, snode, LY_VCODE_NOMAND, snode->name);
    return LY_EVALID;
}

static LY_ERR
lyd_validate_minmax(const struct lyd_node *first, const struct lysc_node *snode, uint32_t min, uint32_t max)
{
    uint32_t count = 0;
    const struct lyd_node *iter;

    assert(min || max);

    LY_LIST_FOR(first, iter) {
        if (iter->schema == snode) {
            ++count;

            if (min && (count == min)) {
                /* satisfied */
                min = 0;
                if (!max) {
                    /* nothing more to check */
                    break;
                }
            }
            if (max && (count > max)) {
                /* not satisifed */
                break;
            }
        }
    }

    if (min) {
        assert(count < min);
        LOGVAL(snode->module->ctx, LY_VLOG_LYSC, snode, LY_VCODE_NOMIN, snode->name);
        return LY_EVALID;
    } else if (max && (count > max)) {
        LOGVAL(snode->module->ctx, LY_VLOG_LYSC, snode, LY_VCODE_NOMAX, snode->name);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

static struct lyd_node *
lyd_val_uniq_find_leaf(const struct lysc_node_leaf *uniq_leaf, struct lyd_node *list)
{
    struct lyd_node *node;
    const struct lysc_node *iter;
    size_t depth = 0, i;

    /* get leaf depth */
    for (iter = (struct lysc_node *)uniq_leaf; iter && (iter != list->schema); iter = iter->parent) {
        if (!(iter->nodetype & (LYS_CHOICE | LYS_CASE))) {
            ++depth;
        }
    }

    node = list;
    while (node && depth) {
        /* find schema node with this depth */
        for (i = depth - 1, iter = (struct lysc_node *)uniq_leaf; i; iter = iter->parent) {
            if (!(iter->nodetype & (LYS_CHOICE | LYS_CASE))) {
                --i;
            }
        }

        /* find iter instance in children */
        assert(iter->nodetype & (LYS_CONTAINER | LYS_LEAF));
        lyd_find_sibling_val(lyd_node_children(node), iter, NULL, 0, &node);
        --depth;
    }

    return node;
}

/*
 * actions (cb_data):
 * 0  - compare all uniques
 * n  - compare n-th unique
 */
static int
lyd_val_uniq_list_equal(void *val1_p, void *val2_p, int UNUSED(mod), void *cb_data)
{
    struct ly_ctx *ctx;
    struct lysc_node_list *slist;
    struct lyd_node *diter, *first, *second;
    struct lyd_value *val1, *val2;
    char *path1, *path2, *uniq_str, *ptr;
    uint32_t i, j, action;

    assert(val1_p && val2_p);

    first = *((struct lyd_node **)val1_p);
    second = *((struct lyd_node **)val2_p);
    action = (uintptr_t)cb_data;

    assert(first && (first->schema->nodetype == LYS_LIST));
    assert(second && (second->schema == first->schema));

    ctx = first->schema->module->ctx;

    slist = (struct lysc_node_list *)first->schema;

    /* compare unique leaves */
    if (action > 0) {
        i = action - 1;
        if (i < LY_ARRAY_SIZE(slist->uniques)) {
            goto uniquecheck;
        }
    }
    LY_ARRAY_FOR(slist->uniques, i) {
uniquecheck:
        LY_ARRAY_FOR(slist->uniques[i], j) {
            /* first */
            diter = lyd_val_uniq_find_leaf(slist->uniques[i][j], first);
            if (diter) {
                val1 = &((struct lyd_node_term *)diter)->value;
            } else {
                /* use default value */
                val1 = slist->uniques[i][j]->dflt;
            }

            /* second */
            diter = lyd_val_uniq_find_leaf(slist->uniques[i][j], second);
            if (diter) {
                val2 = &((struct lyd_node_term *)diter)->value;
            } else {
                /* use default value */
                val2 = slist->uniques[i][j]->dflt;
            }

            if (!val1 || !val2 || val1->realtype->plugin->compare(val1, val2)) {
                /* values differ or either one is not set */
                break;
            }
        }
        if (j && (j == LY_ARRAY_SIZE(slist->uniques[i]))) {
            /* all unique leafs are the same in this set, create this nice error */
            path1 = lyd_path(first, LYD_PATH_LOG, NULL, 0);
            path2 = lyd_path(second, LYD_PATH_LOG, NULL, 0);

            /* use buffer to rebuild the unique string */
            uniq_str = malloc(1024);
            uniq_str[0] = '\0';
            ptr = uniq_str;
            LY_ARRAY_FOR(slist->uniques[i], j) {
                if (j) {
                    strcpy(ptr, " ");
                    ++ptr;
                }
                ptr = lysc_path_until((struct lysc_node *)slist->uniques[i][j], (struct lysc_node *)slist, LYSC_PATH_LOG,
                                      ptr, 1024 - (ptr - uniq_str));
                if (!ptr) {
                    /* path will be incomplete, whatever */
                    break;
                }

                ptr += strlen(ptr);
            }
            LOGVAL(ctx, LY_VLOG_LYD, second, LY_VCODE_NOUNIQ, uniq_str, path1, path2);

            free(path1);
            free(path2);
            free(uniq_str);
            return 1;
        }

        if (action > 0) {
            /* done */
            return 0;
        }
    }

    return 0;
}

static LY_ERR
lyd_validate_unique(const struct lyd_node *first, const struct lysc_node *snode, struct lysc_node_leaf ***uniques)
{
    const struct lyd_node *diter;
    struct ly_set *set;
    uint32_t i, j, n = 0;
    LY_ERR ret = LY_SUCCESS;
    uint32_t hash, u, usize = 0;
    int dynamic;
    const char *str;
    struct hash_table **uniqtables = NULL;
    struct lyd_value *val;
    struct ly_ctx *ctx = snode->module->ctx;

    assert(uniques);

    /* get all list instances */
    set = ly_set_new();
    LY_CHECK_ERR_RET(!set, LOGMEM(ctx), LY_EMEM);
    LY_LIST_FOR(first, diter) {
        if (diter->schema == snode) {
            ly_set_add(set, (void *)diter, LY_SET_OPT_USEASLIST);
        }
    }

    if (set->count == 2) {
        /* simple comparison */
        if (lyd_val_uniq_list_equal(&set->objs[0], &set->objs[1], 0, (void *)0)) {
            /* instance duplication */
            ret = LY_EVALID;
            goto cleanup;
        }
    } else if (set->count > 2) {
        /* use hashes for comparison */
        /* first, allocate the table, the size depends on number of items in the set */
        for (u = 31; u > 0; u--) {
            usize = set->count << u;
            usize = usize >> u;
            if (usize == set->count) {
                break;
            }
        }
        LY_CHECK_ERR_GOTO(!u, LOGINT(ctx); ret = LY_EINT, cleanup);
        u = 32 - u;
        usize = 1 << u;

        uniqtables = malloc(LY_ARRAY_SIZE(uniques) * sizeof *uniqtables);
        LY_CHECK_ERR_GOTO(!uniqtables, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        n = LY_ARRAY_SIZE(uniques);
        for (j = 0; j < n; j++) {
            uniqtables[j] = lyht_new(usize, sizeof(struct lyd_node *), lyd_val_uniq_list_equal, (void *)(j + 1L), 0);
            LY_CHECK_ERR_GOTO(!uniqtables[j], LOGMEM(ctx); ret = LY_EMEM, cleanup);
        }

        for (u = 0; u < set->count; u++) {
            /* loop for unique - get the hash for the instances */
            for (i = 0; i < n; i++) {
                val = NULL;
                for (j = hash = 0; j < LY_ARRAY_SIZE(uniques[i]); j++) {
                    diter = lyd_val_uniq_find_leaf(uniques[i][j], set->objs[u]);
                    if (diter) {
                        val = &((struct lyd_node_term *)diter)->value;
                    } else {
                        /* use default value */
                        val = uniques[i][j]->dflt;
                    }
                    if (!val) {
                        /* unique item not present nor has default value */
                        break;
                    }

                    /* get canonical string value */
                    str = val->realtype->plugin->print(val, LYD_JSON, json_print_get_prefix, NULL, &dynamic);
                    hash = dict_hash_multi(hash, str, strlen(str));
                    if (dynamic) {
                        free((char *)str);
                    }
                }
                if (!val) {
                    /* skip this list instance since its unique set is incomplete */
                    continue;
                }

                /* finish the hash value */
                hash = dict_hash_multi(hash, NULL, 0);

                /* insert into the hashtable */
                ret = lyht_insert(uniqtables[i], &set->objs[u], hash, NULL);
                if (ret == LY_EEXIST) {
                    /* instance duplication */
                    ret = LY_EVALID;
                }
                LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
            }
        }
    }

cleanup:
    ly_set_free(set, NULL);
    for (j = 0; j < n; j++) {
        if (!uniqtables[j]) {
            /* failed when allocating uniquetables[j], following j are not allocated */
            break;
        }
        lyht_free(uniqtables[j]);
    }
    free(uniqtables);

    return ret;
}

static LY_ERR
lyd_validate_siblings_schema_r(const struct lyd_node *first, const struct lysc_node *sparent,
                               const struct lysc_module *mod, int val_opts)
{
    const struct lysc_node *snode = NULL;
    struct lysc_node_list *slist;

    /* disabled nodes are skipped by lys_getnext */
    while ((snode = lys_getnext(snode, sparent, mod, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE))) {
        if ((val_opts & LYD_VALOPT_NO_STATE) && (snode->flags & LYS_CONFIG_R)) {
            continue;
        }

        /* check min-elements and max-elements */
        if (snode->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            slist = (struct lysc_node_list *)snode;
            if (slist->min || slist->max) {
                LY_CHECK_RET(lyd_validate_minmax(first, snode, slist->min, slist->max));
            }

        /* check generic mandatory existence */
        } else if (snode->flags & LYS_MAND_TRUE) {
            LY_CHECK_RET(lyd_validate_mandatory(first, snode));
        }

        /* check unique */
        if (snode->nodetype == LYS_LIST) {
            slist = (struct lysc_node_list *)snode;
            if (slist->uniques) {
                LY_CHECK_RET(lyd_validate_unique(first, snode, slist->uniques));
            }
        }

        if (snode->nodetype & (LYS_CHOICE | LYS_CASE)) {
            /* go recursively for schema-only nodes */
            LY_CHECK_RET(lyd_validate_siblings_schema_r(first, snode, mod, val_opts));
        }
    }

    return LY_SUCCESS;
}

static void
lyd_validate_obsolete(const struct lyd_node *node)
{
    const struct lysc_node *snode;

    snode = node->schema;
    do {
        if (snode->flags & LYS_STATUS_OBSLT) {
            LOGWRN(snode->module->ctx, "Obsolete schema node \"%s\" instantiated in data.", snode->name);
            break;
        }

        snode = snode->parent;
    } while (snode && (snode->nodetype & (LYS_CHOICE | LYS_CASE)));
}

LY_ERR
lyd_validate_siblings_r(struct lyd_node *first, const struct lysc_node *sparent, const struct lys_module *mod,
                        int val_opts)
{
    struct lyd_node *next, *node;
    const struct lysc_node *snode;

    /* validate all restrictions of nodes themselves */
    LY_LIST_FOR_SAFE(first, next, node) {
        if (mod && (lyd_owner_module(node) != mod)) {
            /* all top-level data from this module checked */
            break;
        }

        /* no state data */
        if ((val_opts & LYD_VALOPT_NO_STATE) && (node->schema->flags & LYS_CONFIG_R)) {
            LOGVAL(node->schema->module->ctx, LY_VLOG_LYD, node, LY_VCODE_INSTATE, node->schema->name);
            return LY_EVALID;
        }

        /* obsolete data */
        lyd_validate_obsolete(node);

        /* node's schema if-features */
        if ((snode = lysc_node_is_disabled(node->schema, 1))) {
            LOGVAL(node->schema->module->ctx, LY_VLOG_LYD, node, LY_VCODE_NOIFF, snode->name);
            return LY_EVALID;
        }

        /* TODO node's must */
        /* TODO list all keys existence (take LYD_OPT_EMPTY_INST into consideration) */
        /* node value including if-feature is checked by plugins */
    }

    /* validate schema-based restrictions */
    LY_CHECK_RET(lyd_validate_siblings_schema_r(first, sparent, mod ? mod->compiled : NULL, val_opts));

    LY_LIST_FOR(first, node) {
        /* validate all children recursively */
        LY_CHECK_RET(lyd_validate_siblings_r((struct lyd_node *)lyd_node_children(node), node->schema, NULL, val_opts));

        /* set default for containers */
        if ((node->schema->nodetype == LYS_CONTAINER) && !(node->schema->flags & LYS_PRESENCE)) {
            LY_LIST_FOR((struct lyd_node *)lyd_node_children(node), next) {
                if (!(next->flags & LYD_DEFAULT)) {
                    break;
                }
            }
            if (!next) {
                node->flags |= LYD_DEFAULT;
            }
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_validate_defaults_r(struct lyd_node *parent, struct lyd_node **first, const struct lysc_node *sparent,
                        const struct lys_module *mod, struct ly_set *node_types, struct ly_set *node_when, int val_opts)
{
    LY_ERR ret;
    const struct lysc_node *iter = NULL;
    struct lyd_node *node;
    struct lyd_value **dflts;
    size_t i;

    assert(first && (parent || sparent || mod) && node_types && node_when);

    if (!sparent && parent) {
        sparent = parent->schema;
    }

    while ((iter = lys_getnext(iter, sparent, mod ? mod->compiled : NULL, LYS_GETNEXT_WITHCHOICE))) {
        if ((val_opts & LYD_VALOPT_NO_STATE) && (iter->flags & LYS_CONFIG_R)) {
            continue;
        }

        switch (iter->nodetype) {
        case LYS_CHOICE:
            if (((struct lysc_node_choice *)iter)->dflt && !lys_getnext_data(NULL, *first, NULL, iter, NULL)) {
                /* create default case data */
                LY_CHECK_RET(lyd_validate_defaults_r(parent, first, (struct lysc_node *)((struct lysc_node_choice *)iter)->dflt,
                                                     NULL, node_types, node_when, val_opts));
            }
            break;
        case LYS_CONTAINER:
            if (!(iter->flags & LYS_PRESENCE) && lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default NP container */
                LY_CHECK_RET(lyd_create_inner(iter, &node));
                node->flags = LYD_DEFAULT;
                lyd_insert_node(parent, first, node);

                if (iter->when) {
                    /* remember to resolve when */
                    ly_set_add(node_when, node, LY_SET_OPT_USEASLIST);
                }

                /* create any default children */
                LY_CHECK_RET(lyd_validate_defaults_r(node, lyd_node_children_p(node), NULL, NULL, node_types, node_when, val_opts));
            }
            break;
        case LYS_LEAF:
            if (((struct lysc_node_leaf *)iter)->dflt && lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default leaf */
                ret = lyd_create_term2(iter, ((struct lysc_node_leaf *)iter)->dflt, &node);
                if (ret == LY_EINCOMPLETE) {
                    /* remember to resolve type */
                    ly_set_add(node_types, node, LY_SET_OPT_USEASLIST);
                } else if (ret) {
                    return ret;
                }
                node->flags = LYD_DEFAULT;
                lyd_insert_node(parent, first, node);

                if (iter->when) {
                    /* remember to resolve when */
                    ly_set_add(node_when, node, LY_SET_OPT_USEASLIST);
                }
            }
            break;
        case LYS_LEAFLIST:
            if (((struct lysc_node_leaflist *)iter)->dflts && lyd_find_sibling_next2(*first, iter, NULL, 0, NULL)) {
                /* create all default leaf-lists */
                dflts = ((struct lysc_node_leaflist *)iter)->dflts;
                LY_ARRAY_FOR(dflts, i) {
                    ret = lyd_create_term2(iter, dflts[i], &node);
                    if (ret == LY_EINCOMPLETE) {
                        /* remember to resolve type */
                        ly_set_add(node_types, node, LY_SET_OPT_USEASLIST);
                    } else if (ret) {
                        return ret;
                    }
                    node->flags = LYD_DEFAULT;
                    lyd_insert_node(parent, first, node);

                    if (iter->when) {
                        /* remember to resolve when */
                        ly_set_add(node_when, node, LY_SET_OPT_USEASLIST);
                    }
                }
            }
            break;
        default:
            /* without defaults */
            break;
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
_lyd_validate(struct lyd_node **tree, const struct lys_module **modules, int mod_count, const struct ly_ctx *ctx,
              int val_opts)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *first, *next, *node, **first2;
    const struct lys_module *mod;
    const struct lyd_meta *meta;
    struct ly_set type_check = {0}, type_meta_check = {0}, when_check = {0};
    uint32_t i = 0;

    LY_CHECK_ARG_RET(NULL, tree, *tree || ctx || (modules && mod_count), LY_EINVAL);

    if (val_opts & ~LYD_VALOPT_MASK) {
        LOGERR(ctx, LY_EINVAL, "Some invalid flags passed to validation.");
        return LY_EINVAL;
    }

    next = *tree;
    while (1) {
        if (val_opts & LYD_VALOPT_DATA_ONLY) {
            mod = lyd_data_next_module(&next, &first);
        } else {
            mod = lyd_mod_next_module(next, modules, mod_count, ctx, &i, &first);
        }
        if (!mod) {
            break;
        }
        if (first == *tree) {
            /* make sure first2 changes are carried to tree */
            first2 = tree;
        } else {
            first2 = &first;
        }

        /* validate new top-level nodes of this module, autodelete */
        ret = lyd_validate_new(first2, NULL, mod);
        LY_CHECK_GOTO(ret, cleanup);

        /* add all top-level defaults for this module */
        ret = lyd_validate_defaults_r(NULL, first2, NULL, mod, &type_check, &when_check, val_opts);
        LY_CHECK_GOTO(ret, cleanup);

        /* process nested nodes */
        LY_LIST_FOR(*first2, first) {
            LYD_TREE_DFS_BEGIN(first, next, node) {
                /* skip added default nodes */
                if ((node->flags & (LYD_DEFAULT | LYD_NEW)) != (LYD_DEFAULT | LYD_NEW)) {
                    LY_LIST_FOR(node->meta, meta) {
                        /* metadata type resolution */
                        ly_set_add(&type_meta_check, (void *)meta, LY_SET_OPT_USEASLIST);
                    }

                    if (node->schema->nodetype & LYD_NODE_TERM) {
                        /* node type resolution */
                        ly_set_add(&type_check, (void *)node, LY_SET_OPT_USEASLIST);
                    } else if (node->schema->nodetype & LYD_NODE_INNER) {
                        /* new node validation, autodelete */
                        ret = lyd_validate_new(lyd_node_children_p((struct lyd_node *)node), node->schema, NULL);
                        LY_CHECK_GOTO(ret, cleanup);

                        /* add nested defaults */
                        ret = lyd_validate_defaults_r(node, lyd_node_children_p((struct lyd_node *)node), NULL, NULL, &type_check,
                                                      &when_check, val_opts);
                        LY_CHECK_GOTO(ret, cleanup);
                    }

                    if (!(node->schema->nodetype & (LYS_ACTION | LYS_NOTIF)) && node->schema->when) {
                        /* when evaluation */
                        ly_set_add(&when_check, (void *)node, LY_SET_OPT_USEASLIST);
                    }
                }

                LYD_TREE_DFS_END(first, next, node);
            }
        }

        /* finish incompletely validated terminal values/attributes and when conditions */
        ret = lyd_validate_unres(tree, &when_check, &type_check, &type_meta_check, LYD_JSON, lydjson_resolve_prefix, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* perform final validation that assumes the data tree is final */
        ret = lyd_validate_siblings_r(*first2, NULL, mod, val_opts);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    ly_set_erase(&type_check, NULL);
    ly_set_erase(&type_meta_check, NULL);
    ly_set_erase(&when_check, NULL);
    return ret;
}

API LY_ERR
lyd_validate(struct lyd_node **tree, const struct ly_ctx *ctx, int val_opts)
{
    return _lyd_validate(tree, NULL, 0, ctx, val_opts);
}

API LY_ERR
lyd_validate_modules(struct lyd_node **tree, const struct lys_module **modules, int mod_count, int val_opts)
{
    return _lyd_validate(tree, modules, mod_count, NULL, val_opts);
}
