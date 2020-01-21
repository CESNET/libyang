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

static LY_ERR
lyd_validate_mandatory(const struct lysc_node *snode, struct lyd_node *sibling)
{
    struct lyd_node *node;
    int is_choice = 0;

    if (snode->nodetype == LYS_CHOICE) {
        is_choice = 1;
    }

    for (node = sibling; node; node = node->next) {
        if (is_choice) {
            if (node->schema->parent && (node->schema->parent->nodetype & LYS_CASE) && (node->schema->parent->parent == snode)) {
                /* case data instance found */
                return LY_SUCCESS;
            }
        } else {
            if (node->schema == snode) {
                /* data instance found */
                return LY_SUCCESS;
            }
        }
    }

    /* node instance not found */
    LOGVAL(snode->module->ctx, LY_VLOG_LYSC, snode, LY_VCODE_NOMAND, snode->name);
    return LY_EVALID;
}

static LY_ERR
lyd_validate_minmax(const struct lysc_node *snode, uint32_t min, uint32_t max, struct lyd_node *sibling)
{
    /* TODO snode count in siblings */
    return LY_SUCCESS;
}

static LY_ERR
lyd_validate_unique(const struct lysc_node *snode, struct lysc_node_leaf ***uniques, struct lyd_node *sibling)
{
    /* TODO check uniques in siblings and children */
    return LY_SUCCESS;
}

static LY_ERR
lyd_validate_cases(const struct lysc_node_case *cases, struct lyd_node *sibling)
{
    /* TODO check there are nodes only from a single case,
     * what if not? validation error or autodelete */
    return LY_SUCCESS;
}

static LY_ERR
lyd_validate_siblings_r(struct lyd_node *sibling, const struct lysc_node *sparent, const struct lysc_module *mod, int options)
{
    LY_ERR ret;
    const struct lysc_node *snode = NULL;
    struct lysc_node_list *slist;
    struct lyd_node *node;

    /* disabled nodes are skipped by lys_getnext */
    while ((snode = lys_getnext(snode, sparent, mod, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE))) {
        /* check mandatory existence */
        if (snode->flags & LYS_MAND_TRUE) {
            LY_CHECK_RET(lyd_validate_mandatory(snode, sibling));
        }

        /* check min-elements and max-elements */
        if (snode->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            slist = (struct lysc_node_list *)snode;
            if (slist->min || slist->max) {
                LY_CHECK_RET(lyd_validate_minmax(snode, slist->min, slist->max, sibling));
            }
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
    }

    for (node = sibling; node; node = node->next) {
        /* TODO node's must */
        /* TODO node instance duplicites */
        /* TODO node status */
        /* TODO node's if-features */
        /* TODO node list keys */
        /* node value including if-feature is checked by plugins */

        /* validate all children */
        LY_CHECK_RET(lyd_validate_siblings_r((struct lyd_node *)lyd_node_children(sibling), node->schema, mod, options));
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_validate_modules(const struct lyd_node **trees, const struct lys_module **modules, int mod_count, struct ly_ctx *ctx,
                     int options)
{
    LY_ERR ret;
    uint32_t i = 0, j;
    const struct lys_module *mod;
    struct lyd_node *tree;

    if (options & LYD_OPT_VAL_DATA_ONLY) {
        if (trees) {
            for (j = 0; j < LY_ARRAY_SIZE(trees); ++j) {
                tree = (struct lyd_node *)trees[j];

                /* validate all top-level nodes and then inner nodes recursively */
                LY_CHECK_RET(lyd_validate_siblings_r(tree, NULL, tree->schema->module->compiled, options));
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
            LY_CHECK_RET(lyd_validate_siblings_r(tree, NULL, mod->compiled, options));
        }
    }

    return LY_SUCCESS;
}
