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

#include <assert.h>
#include <string.h>

#include "common.h"
#include "xpath.h"
#include "tree_data_internal.h"
#include "tree_schema_internal.h"

/**
 * @brief Evaluate a single "when" condition.
 *
 * @param[in] when When to evaluate.
 * @param[in] node Node whose existence depends on this when.
 * @param[in] trees Array of all data trees.
 * @return LY_ERR value (LY_EINCOMPLETE if a referenced node does not have its when evaluated)
 */
static LY_ERR
lyd_val_when(struct lysc_when *when, struct lyd_node *node, const struct lyd_node **trees)
{
    LY_ERR ret;
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
    ret = lyxp_eval(when->cond, LYD_UNKNOWN, when->module, ctx_node, ctx_node ? LYXP_NODE_ELEM : LYXP_NODE_ROOT_CONFIG,
                    trees, &xp_set, LYXP_SCHEMA);
    lyxp_set_cast(&xp_set, LYXP_SET_BOOLEAN);

    /* return error or LY_EINCOMPLETE for dependant unresolved when */
    LY_CHECK_RET(ret);

    /* take action based on the result */
    if (!xp_set.val.bool) {
        if (node->flags & LYD_WHEN_TRUE) {
            /* autodelete */
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
lyd_validate_unres(struct ly_set *node_types, struct ly_set *attr_types, struct ly_set *node_when, LYD_FORMAT format,
                   ly_clb_resolve_prefix get_prefix_clb, void *parser_data, const struct lyd_node **trees)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t u;

    /* finish incompletely validated terminal values */
    for (u = 0; node_types && (u < node_types->count); u++) {
        struct lyd_node_term *node = (struct lyd_node_term *)node_types->objs[u];

        /* validate and store the value of the node */
        ret = lyd_value_parse(node, node->value.original, strlen(node->value.original), 0, 1, get_prefix_clb,
                            parser_data, format, trees);
        LY_CHECK_RET(ret);
    }

    /* ... and attribute values */
    for (u = 0; attr_types && (u < attr_types->count); u++) {
        struct lyd_attr *attr = (struct lyd_attr *)attr_types->objs[u];

        /* validate and store the value of the node */
        ret = lyd_value_parse_attr(attr, attr->value.original, strlen(attr->value.original), 0, 1, get_prefix_clb,
                                   parser_data, format, trees);
        LY_CHECK_RET(ret);
    }

    /* no when conditions */
    if (!node_when || !node_when->count) {
        return ret;
    }

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
                    ret = lyd_val_when(schema->when[i], node, trees);
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

    return ret;
}

static const struct lys_module *
lyd_val_next_module(const struct lys_module **modules, int mod_count, struct ly_ctx *ctx, uint32_t *i)
{
    if (modules && mod_count) {
        return modules[(*i)++];
    }

    return ly_ctx_get_module_iter(ctx, i);
}

static int
lyd_val_has_choice_data(const struct lysc_node *snode, struct lyd_node *sibling)
{
    const struct lysc_node *iter = NULL;

    assert(snode->nodetype == LYS_CHOICE);

    while ((iter = lys_getnext(iter, snode, NULL, 0))) {
        switch (iter->nodetype) {
        case LYS_CONTAINER:
        case LYS_ANYXML:
        case LYS_ANYDATA:
        case LYS_LEAF:
            if (!lyd_find_sibling_val(sibling, iter, NULL, 0, NULL)) {
                /* one case child data instance found */
                return 1;
            }
            break;
        case LYS_LIST:
        case LYS_LEAFLIST:
            if (!lyd_find_sibling_next2(sibling, iter, NULL, 0, NULL)) {
                /* one case child data instance found */
                return 1;
            }
            break;
        default:
            assert(0);
            LOGINT(snode->module->ctx);
            return 0;
        }
    }

    return 0;
}

static LY_ERR
lyd_validate_mandatory(const struct lysc_node *snode, struct lyd_node *sibling)
{
    if (snode->nodetype == LYS_CHOICE) {
        /* some data of a choice case exist */
        if (lyd_val_has_choice_data(snode, sibling)) {
            return LY_SUCCESS;
        }
    } else {
        assert(snode->nodetype & (LYS_LEAF | LYS_CONTAINER | LYD_NODE_ANY));

        if (!lyd_find_sibling_val(sibling, snode, NULL, 0, NULL)) {
            /* data instance found */
            return LY_SUCCESS;
        }
    }

    /* node instance not found */
    LOGVAL(snode->module->ctx, LY_VLOG_LYSC, snode, LY_VCODE_NOMAND, snode->name);
    return LY_EVALID;
}

static LY_ERR
lyd_validate_minmax(const struct lysc_node *snode, uint32_t min, uint32_t max, struct lyd_node *sibling)
{
    uint32_t count = 0;
    struct lyd_node *iter;

    assert(min || max);

    LY_LIST_FOR(sibling, iter) {
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
lyd_validate_unique(const struct lysc_node *snode, struct lysc_node_leaf ***uniques, struct lyd_node *sibling)
{
    struct lyd_node *diter;
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
    LY_LIST_FOR(sibling, diter) {
        if (diter->schema == snode) {
            ly_set_add(set, diter, LY_SET_OPT_USEASLIST);
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
lyd_validate_cases(const struct lysc_node_case *cases, struct lyd_node *sibling)
{
    /* TODO check there are nodes only from a single case,
     * what if not? validation error or autodelete */
    return LY_SUCCESS;
}

static LY_ERR
lyd_validate_siblings_schema_r(struct lyd_node *sibling, const struct lysc_node *sparent, const struct lysc_module *mod,
                               int options)
{
    const struct lysc_node *snode = NULL;
    struct lysc_node_list *slist;

    /* disabled nodes are skipped by lys_getnext */
    while ((snode = lys_getnext(snode, sparent, mod, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE))) {
        /* check min-elements and max-elements */
        if (snode->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            slist = (struct lysc_node_list *)snode;
            if (slist->min || slist->max) {
                LY_CHECK_RET(lyd_validate_minmax(snode, slist->min, slist->max, sibling));
            }

        /* check generic mandatory existence */
        } else if (snode->flags & LYS_MAND_TRUE) {
            LY_CHECK_RET(lyd_validate_mandatory(snode, sibling));
        }

        /* check unique */
        if (snode->nodetype == LYS_LIST) {
            slist = (struct lysc_node_list *)snode;
            if (slist->uniques) {
                LY_CHECK_RET(lyd_validate_unique(snode, slist->uniques, sibling));
            }
        }

        /* check case duplicites */
        if (snode->nodetype == LYS_CHOICE) {
            LY_CHECK_RET(lyd_validate_cases(((struct lysc_node_choice *)snode)->cases, sibling));
        }

        if (snode->nodetype & (LYS_CHOICE | LYS_CASE)) {
            /* go recursively for schema-only nodes */
            LY_CHECK_RET(lyd_validate_siblings_schema_r(sibling, snode, mod, options));
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
lyd_validate_siblings_r(struct lyd_node *sibling, const struct lysc_node *sparent, const struct lysc_module *mod, int options)
{
    struct lyd_node *node;

    /* validate all restrictions of nodes themselves */
    LY_LIST_FOR(sibling, node) {
        /* TODO node instance duplicities */
        /* TODO node's must */
        /* TODO node status */
        /* TODO node's if-features */
        /* TODO node list keys */
        /* node value including if-feature is checked by plugins */
    }

    /* validate schema-based restrictions */
    LY_CHECK_RET(lyd_validate_siblings_schema_r(sibling, sparent, mod, options));

    LY_LIST_FOR(sibling, node) {
        /* this sibling is valid */
        node->flags &= ~LYD_NEW;

        /* validate all children recursively */
        LY_CHECK_RET(lyd_validate_siblings_r((struct lyd_node *)lyd_node_children(node), node->schema, mod, options));
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_validate_data(const struct lyd_node **trees, const struct lys_module **modules, int mod_count, struct ly_ctx *ctx,
                  int val_opts)
{
    uint32_t i = 0, j;
    const struct lys_module *mod;
    struct lyd_node *tree;

    if (val_opts & LYD_VALOPT_DATA_ONLY) {
        if (trees) {
            for (j = 0; j < LY_ARRAY_SIZE(trees); ++j) {
                tree = (struct lyd_node *)trees[j];

                /* validate all top-level nodes and then inner nodes recursively */
                LY_CHECK_RET(lyd_validate_siblings_r(tree, NULL, tree->schema->module->compiled, val_opts));
            }
        }
    } else {
        while ((mod = lyd_val_next_module(modules, mod_count, ctx, &i))) {
            if (!mod->implemented) {
                continue;
            }

            /* find data of this module, if any */
            tree = NULL;
            if (trees) {
                for (j = 0; j < LY_ARRAY_SIZE(trees); ++j) {
                    if (trees[j]->schema->module == mod) {
                        tree = (struct lyd_node *)trees[j];
                        break;
                    }
                }
            }

            /* validate all top-level nodes and then inner nodes recursively */
            LY_CHECK_RET(lyd_validate_siblings_r(tree, NULL, mod->compiled, val_opts));
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_validate_defaults_r(struct lyd_node_inner *parent, struct lyd_node **first, const struct lysc_node *schema,
                        const struct lysc_module *mod, struct ly_set *node_types, struct ly_set *node_when)
{
    const struct lysc_node *iter = NULL;
    struct lyd_node *node;
    struct lyd_value **dflts;
    size_t i;

    assert(first && (schema || mod) && node_types && node_when);

    while ((iter = lys_getnext(iter, schema, mod, LYS_GETNEXT_WITHCHOICE))) {
        switch (iter->nodetype) {
        case LYS_CHOICE:
            if (((struct lysc_node_choice *)iter)->dflt && !lyd_val_has_choice_data(iter, *first)) {
                /* create default case data */
                LY_CHECK_RET(lyd_validate_defaults_r(parent, first, (struct lysc_node *)((struct lysc_node_choice *)iter)->dflt,
                                                     NULL, node_types, node_when));
            }
            break;
        case LYS_CONTAINER:
            if (!(iter->flags & LYS_PRESENCE) && lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default NP container (default flag automatically set) */
                LY_CHECK_RET(lyd_create_inner(iter, &node));
                lyd_insert_node((struct lyd_node *)parent, first, node);

                if (iter->when) {
                    /* remember to resolve when */
                    ly_set_add(node_when, node, LY_SET_OPT_USEASLIST);
                }
            }
            break;
        case LYS_LEAF:
            if (((struct lysc_node_leaf *)iter)->dflt && lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default leaf */
                LY_CHECK_RET(lyd_create_term2(iter, ((struct lysc_node_leaf *)iter)->dflt, &node));
                node->flags |= LYD_DEFAULT;
                lyd_insert_node((struct lyd_node *)parent, first, node);

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
                    LY_CHECK_RET(lyd_create_term2(iter, dflts[i], &node));
                    node->flags |= LYD_DEFAULT;
                    lyd_insert_node((struct lyd_node *)parent, first, node);

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

LY_ERR
lyd_validate_defaults_top(struct lyd_node **first, const struct lys_module **modules, int mod_count, struct ly_ctx *ctx,
                          struct ly_set *node_types, struct ly_set *node_when, int val_opts)
{
    uint32_t i = 0;
    const struct lys_module *mod;
    struct lyd_node *sibling;

    assert(node_types && node_when);

    if (val_opts & LYD_VALOPT_DATA_ONLY) {
        mod = NULL;
        LY_LIST_FOR(*first, sibling) {
            if (lyd_top_node_module(sibling) != mod) {
                /* remember this module */
                mod = lyd_top_node_module(sibling);

                /* add all top-level defaults for this module */
                LY_CHECK_RET(lyd_validate_defaults_r(NULL, first, NULL, mod->compiled, node_types, node_when));
            }
        }
    } else {
        while ((mod = lyd_val_next_module(modules, mod_count, ctx, &i))) {
            if (!mod->implemented) {
                continue;
            }

            /* add all top-level defaults for this module */
            LY_CHECK_RET(lyd_validate_defaults_r(NULL, first, NULL, mod->compiled, node_types, node_when));
        }
    }

    return LY_SUCCESS;
}
