/**
 * @file diff.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief diff functions
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _XOPEN_SOURCE 500

#include "diff.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "common.h"
#include "log.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

static const char *
lyd_diff_op2str(enum lyd_diff_op op)
{
    switch (op) {
    case LYD_DIFF_OP_CREATE:
        return "create";
    case LYD_DIFF_OP_DELETE:
        return "delete";
    case LYD_DIFF_OP_REPLACE:
        return "replace";
    case LYD_DIFF_OP_NONE:
        return "none";
    }

    LOGINT(NULL);
    return NULL;
}

LY_ERR
lyd_diff_add(const struct lyd_node *node, enum lyd_diff_op op, const char *orig_default, const char *orig_value,
             const char *key, const char *value, const char *orig_key, struct lyd_node **diff)
{
    struct lyd_node *dup, *siblings, *match = NULL, *diff_parent = NULL;
    const struct lyd_node *parent = NULL;
    const struct lys_module *yang_mod;

    assert(diff);

    /* find the first existing parent */
    siblings = *diff;
    while (1) {
        /* find next node parent */
        parent = node;
        while (parent->parent && (!diff_parent || (parent->parent->schema != diff_parent->schema))) {
            parent = (struct lyd_node *)parent->parent;
        }
        if (parent == node) {
            /* no more parents to find */
            break;
        }

        /* check whether it exists in the diff */
        if (lyd_find_sibling_first(siblings, parent, &match)) {
            break;
        }

        /* another parent found */
        diff_parent = match;

        /* move down in the diff */
        siblings = LYD_CHILD(match);
    }

    /* duplicate the subtree (and connect to the diff if possible) */
    dup = lyd_dup(node, (struct lyd_node_inner *)diff_parent, LYD_DUP_RECURSIVE | LYD_DUP_NO_META | LYD_DUP_WITH_PARENTS);
    LY_CHECK_RET(!dup, LY_EMEM);

    /* find the first duplicated parent */
    if (!diff_parent) {
        diff_parent = (struct lyd_node *)dup->parent;
        while (diff_parent && diff_parent->parent) {
            diff_parent = (struct lyd_node *)diff_parent->parent;
        }
    } else {
        diff_parent = (struct lyd_node *)dup;
        while (diff_parent->parent && (diff_parent->parent->schema == parent->schema)) {
            diff_parent = (struct lyd_node *)diff_parent->parent;
        }
    }

    /* no parent existed, must be manually connected */
    if (!diff_parent) {
        /* there actually was no parent to duplicate */
        if (*diff) {
            lyd_insert_sibling(*diff, dup);
        } else {
            *diff = dup;
        }
    } else if (!diff_parent->parent) {
        if (*diff) {
            lyd_insert_sibling(*diff, diff_parent);
        } else {
            *diff = diff_parent;
        }
    }

    /* get module with the operation metadata */
    yang_mod = LYD_NODE_CTX(node)->list.objs[1];
    assert(!strcmp(yang_mod->name, "yang"));

    /* add parent operation, if any */
    if (diff_parent && (diff_parent != dup) && !lyd_new_meta(diff_parent, yang_mod, "operation", "none")) {
        return LY_EMEM;
    }

    /* add subtree operation */
    if (!lyd_new_meta(dup, yang_mod, "operation", lyd_diff_op2str(op))) {
        return LY_EMEM;
    }

    /* orig-default */
    if (orig_default && !lyd_new_meta(dup, yang_mod, "orig-default", orig_default)) {
        return LY_EMEM;
    }

    /* orig-value */
    if (orig_value && !lyd_new_meta(dup, yang_mod, "orig-value", orig_value)) {
        return LY_EMEM;
    }

    /* key */
    if (key && !lyd_new_meta(dup, yang_mod, "key", key)) {
        return LY_EMEM;
    }

    /* value */
    if (value && !lyd_new_meta(dup, yang_mod, "value", value)) {
        return LY_EMEM;
    }

    /* orig-key */
    if (orig_key && !lyd_new_meta(dup, yang_mod, "orig-key", orig_key)) {
        return LY_EMEM;
    }

    return LY_SUCCESS;
}

/**
 * @brief Get a userord entry for a specific user-ordered list/leaf-list. Create if does not exist yet.
 *
 * @param[in] first
 * @param[in] schema Schema node of the list/leaf-list.
 * @param[in,out] userord Sized array of userord items.
 * @return Userord item for all the user-ordered list/leaf-list instances.
 */
static struct lyd_diff_userord *
lyd_diff_userord_get(const struct lyd_node *first, const struct lysc_node *schema, struct lyd_diff_userord **userord)
{
    struct lyd_diff_userord *item;
    const struct lyd_node *iter, **node;
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(*userord, u) {
        if ((*userord)[u].schema == schema) {
            return &(*userord)[u];
        }
    }

    /* it was not added yet, add it now */
    LY_ARRAY_NEW_RET(schema->module->ctx, *userord, item, NULL);

    item->schema = schema;
    item->pos = 0;
    item->inst = NULL;

    /* store all the instance pointers in the current order */
    if (first) {
        if (first->parent) {
            iter = first->parent->child;
        } else {
            for (iter = first; iter->prev->next; iter = iter->prev);
        }
        for (; iter; iter = iter->next) {
            if (iter->schema == first->schema) {
                LY_ARRAY_NEW_RET(schema->module->ctx, item->inst, node, NULL);
                *node = iter;
            }
        }
    }

    return item;
}

/**
 * @brief Get all the metadata to be stored in a diff for the 2 nodes. Can be used only for user-ordered
 * lists/leaf-lists.
 *
 * @param[in] first Node from the first tree, can be NULL (on create).
 * @param[in] second Node from the second tree, can be NULL (on delete).
 * @param[in] options Diff options.
 * @param[in,out] userord Sized array of userord items for keeping the current node order.
 * @param[out] op Operation.
 * @param[out] orig_default Original default metadata.
 * @param[out] value Value metadata.
 * @param[out] orig_value Original value metadata
 * @param[out] key Key metadata.
 * @param[out] orig_key Original key metadata.
 * @return LY_SUCCESS on success,
 * @return LY_ENOT if there is no change to be added into diff,
 * @return LY_ERR value on other errors.
 */
static LY_ERR
lyd_diff_userord_attrs(const struct lyd_node *first, const struct lyd_node *second, int options,
                       struct lyd_diff_userord **userord, enum lyd_diff_op *op, const char **orig_default, char **value,
                       char **orig_value, char **key, char **orig_key)
{
    const struct lysc_node *schema;
    int dynamic;
    size_t buflen, bufused, first_pos, second_pos;
    struct lyd_diff_userord *userord_item;

    assert(first || second);

    *orig_default = NULL;
    *value = NULL;
    *orig_value = NULL;
    *key = NULL;
    *orig_key = NULL;

    schema = first ? first->schema : second->schema;
    assert(lysc_is_userordered(schema));

    /* get userord entry */
    userord_item = lyd_diff_userord_get(first, schema, userord);
    LY_CHECK_RET(!userord_item, LY_EMEM);

    /* prepare position of the next instance */
    second_pos = userord_item->pos++;

    /* find user-ordered first position */
    if (first) {
        for (first_pos = second_pos; first_pos < LY_ARRAY_COUNT(userord_item->inst); ++first_pos) {
            if (userord_item->inst[first_pos] == first) {
                break;
            }
        }
        assert(first_pos < LY_ARRAY_COUNT(userord_item->inst));
    } else {
        first_pos = 0;
    }

    /* learn operation first */
    if (!second) {
        *op = LYD_DIFF_OP_DELETE;
    } else if (!first) {
        *op = LYD_DIFF_OP_CREATE;
    } else {
        assert(schema->nodetype & (LYS_LIST | LYS_LEAFLIST));
        if (lyd_compare(second, userord_item->inst[second_pos], 0)) {
            /* in first, there is a different instance on the second position, we are going to move 'first' node */
            *op = LYD_DIFF_OP_REPLACE;
        } else if ((options & LYD_DIFF_WITHDEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
            /* default flag change */
            *op = LYD_DIFF_OP_NONE;
        } else {
            /* no changes */
            return LY_ENOT;
        }
    }

    /*
     * set each attribute correctly based on the operation and node type
     */

    /* orig-default */
    if ((options & LYD_DIFF_WITHDEFAULTS) && (schema->nodetype == LYS_LEAFLIST)
            && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_NONE))) {
        if (first->flags & LYD_DEFAULT) {
            *orig_default = "true";
        } else {
            *orig_default = "false";
        }
    }

    /* value */
    if ((schema->nodetype == LYS_LEAFLIST) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            *value = (char *)lyd_value2str((struct lyd_node_term *)userord_item->inst[second_pos - 1], &dynamic);
            if (!dynamic) {
                *value = strdup(*value);
                LY_CHECK_ERR_RET(!*value, LOGMEM(schema->module->ctx), LY_EMEM);
            }
        } else {
            *value = strdup("");
            LY_CHECK_ERR_RET(!*value, LOGMEM(schema->module->ctx), LY_EMEM);
        }
    }

    /* orig-value */
    if ((schema->nodetype == LYS_LEAFLIST) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            *orig_value = (char *)lyd_value2str((struct lyd_node_term *)userord_item->inst[first_pos - 1], &dynamic);
            if (!dynamic) {
                *orig_value = strdup(*orig_value);
                LY_CHECK_ERR_RET(!*orig_value, LOGMEM(schema->module->ctx), LY_EMEM);
            }
        } else {
            *orig_value = strdup("");
            LY_CHECK_ERR_RET(!*orig_value, LOGMEM(schema->module->ctx), LY_EMEM);
        }
    }

    /* key */
    if ((schema->nodetype == LYS_LIST) && ((*op == LYD_DIFF_OP_REPLACE) || (*op ==LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            buflen = bufused = 0;
            LY_CHECK_RET(lyd_path_list_predicate(userord_item->inst[second_pos - 1], key, &buflen, &bufused, 0));
        } else {
            *key = strdup("");
            LY_CHECK_ERR_RET(!*key, LOGMEM(schema->module->ctx), LY_EMEM);
        }
    }

    /* orig-key */
    if ((schema->nodetype == LYS_LIST) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            buflen = bufused = 0;
            LY_CHECK_RET(lyd_path_list_predicate(userord_item->inst[first_pos - 1], orig_key, &buflen, &bufused, 0));
        } else {
            *orig_key = strdup("");
            LY_CHECK_ERR_RET(!*orig_key, LOGMEM(schema->module->ctx), LY_EMEM);
        }
    }

    /*
     * update our instances - apply the change
     */
    if (*op == LYD_DIFF_OP_CREATE) {
        /* insert the instance */
        LY_ARRAY_RESIZE_ERR_RET(schema->module->ctx, userord_item->inst, LY_ARRAY_COUNT(userord_item->inst) + 1,
                                ;, LY_EMEM);
        if (second_pos < LY_ARRAY_COUNT(userord_item->inst)) {
            memmove(userord_item->inst + second_pos + 1, userord_item->inst + second_pos,
                    (LY_ARRAY_COUNT(userord_item->inst) - second_pos) * sizeof *userord_item->inst);
        }
        LY_ARRAY_INCREMENT(userord_item->inst);
        userord_item->inst[second_pos] = second;

    } else if (*op == LYD_DIFF_OP_DELETE) {
        /* remove the instance */
        if (first_pos + 1 < LY_ARRAY_COUNT(userord_item->inst)) {
            memmove(userord_item->inst + first_pos, userord_item->inst + first_pos + 1,
                    (LY_ARRAY_COUNT(userord_item->inst) - first_pos - 1) * sizeof *userord_item->inst);
        }
        LY_ARRAY_DECREMENT(userord_item->inst);

    } else if (*op == LYD_DIFF_OP_REPLACE) {
        /* move the instances */
        memmove(userord_item->inst + second_pos + 1, userord_item->inst + second_pos,
                (first_pos - second_pos) * sizeof *userord_item->inst);
        userord_item->inst[second_pos] = first;
    }

    return LY_SUCCESS;
}

/**
 * @brief Get all the metadata to be stored in a diff for the 2 nodes. Cannot be used for user-ordered
 * lists/leaf-lists.
 *
 * @param[in] first Node from the first tree, can be NULL (on create).
 * @param[in] second Node from the second tree, can be NULL (on delete).
 * @param[in] options Diff options.
 * @param[out] op Operation.
 * @param[out] orig_default Original default metadata.
 * @param[out] orig_value Original value metadata.
 * @return LY_SUCCESS on success,
 * @return LY_ENOT if there is no change to be added into diff,
 * @return LY_ERR value on other errors.
 */
static LY_ERR
lyd_diff_attrs(const struct lyd_node *first, const struct lyd_node *second, int options, enum lyd_diff_op *op,
               const char **orig_default, char **orig_value)
{
    const struct lysc_node *schema;
    int dynamic;

    assert(first || second);

    *orig_default = NULL;
    *orig_value = NULL;

    schema = first ? first->schema : second->schema;
    assert(!lysc_is_userordered(schema));

    /* learn operation first */
    if (!second) {
        *op = LYD_DIFF_OP_DELETE;
    } else if (!first) {
        *op = LYD_DIFF_OP_CREATE;
    } else {
        switch (schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
            /* no changes */
            return LY_ENOT;
        case LYS_LIST:
        case LYS_LEAFLIST:
            if ((options & LYD_DIFF_WITHDEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
                /* default flag change */
                *op = LYD_DIFF_OP_NONE;
            } else {
                /* no changes */
                return LY_ENOT;
            }
            break;
        case LYS_LEAF:
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (lyd_compare(first, second, 0)) {
                /* different values */
                *op = LYD_DIFF_OP_REPLACE;
            } else if ((options & LYD_DIFF_WITHDEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
                /* default flag change */
                *op = LYD_DIFF_OP_NONE;
            } else {
                /* no changes */
                return LY_ENOT;
            }
            break;
        default:
            LOGINT_RET(schema->module->ctx);
        }
    }

    /*
     * set each attribute correctly based on the operation and node type
     */

    /* orig-default */
    if ((options & LYD_DIFF_WITHDEFAULTS) && (schema->nodetype & LYD_NODE_TERM)
            && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_NONE))) {
        if (first->flags & LYD_DEFAULT) {
            *orig_default = "true";
        } else {
            *orig_default = "false";
        }
    }

    /* orig-value */
    if ((schema->nodetype == LYS_LEAF) && (*op == LYD_DIFF_OP_REPLACE)) {
        /* leaf */
        *orig_value = (char *)lyd_value2str((struct lyd_node_term *)first, &dynamic);
        if (!dynamic) {
            *orig_value = strdup(*orig_value);
            LY_CHECK_ERR_RET(!*orig_value, LOGMEM(schema->module->ctx), LY_EMEM);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Perform diff for all siblings at certain depth, recursively.
 *
 * For user-ordered lists/leaf-lists a specific structure is used for storing
 * the current order. The idea is to apply all the generated diff changes
 * virtually on the first tree so that we can continue to generate correct
 * changes after some were already generated.
 *
 * The algorithm then uses second tree position-based changes with a before
 * (preceding) item anchor.
 *
 * Example:
 *
 * Virtual first tree leaf-list order:
 * 1 2 [3] 4 5
 *
 * Second tree leaf-list order:
 * 1 2 [5] 3 4
 *
 * We are at the 3rd node now. We look at whether the nodes on the 3rd position
 * match - they do not - move nodes so that the 3rd position node is final ->
 * -> move node 5 to the 3rd position -> move node 5 after node 2.
 *
 * Required properties:
 * Stored operations (move) should not be affected by later operations -
 * - would cause a redundantly long list of operations, possibly inifinite.
 *
 * Implemenation justification:
 * First, all delete operations and only then move/create operations are stored.
 * Also, preceding anchor is used and after each iteration another node is
 * at its final position. That results in the invariant that all preceding
 * nodes are final and will not be changed by the later operations, meaning
 * they can safely be used as anchors for the later operations.
 *
 * @param[in] first First tree first sibling.
 * @param[in] second Second tree first sibling.
 * @param[in] options Diff options.
 * @param[in,out] diff Diff to append to.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_siblings_r(const struct lyd_node *first, const struct lyd_node *second, int options, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyd_node *iter_first, *iter_second;
    struct lyd_node *match_second, *match_first;
    int nosiblings = 0;
    struct lyd_diff_userord *userord = NULL;
    LY_ARRAY_COUNT_TYPE u;
    enum lyd_diff_op op;
    const char *orig_default;
    char *orig_value, *key, *value, *orig_key;

    if (options & LYD_DIFF_NOSIBLINGS) {
        /* remember it for this function call only, should not be passed recursively */
        nosiblings = 1;
        options &= ~LYD_DIFF_NOSIBLINGS;
    }

    /* compare first tree to the second tree - delete, replace, none */
    LY_LIST_FOR(first, iter_first) {
        assert(!(iter_first->schema->flags & LYS_KEY));
        if ((iter_first->flags & LYD_DEFAULT) && !(options & LYD_DIFF_WITHDEFAULTS)) {
            /* skip default nodes */
            continue;
        }

        if (iter_first->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            /* try to find the exact instance */
            lyd_find_sibling_first(second, iter_first, &match_second);
        } else {
            /* try to simply find the node, there cannot be more instances */
            lyd_find_sibling_val(second, iter_first->schema, NULL, 0, &match_second);
        }

        if (match_second && (match_second->flags & LYD_DEFAULT) && !(options & LYD_DIFF_WITHDEFAULTS)) {
            /* ignore default nodes */
            match_second = NULL;
        }

        if (lysc_is_userordered(iter_first->schema)) {
            if (match_second) {
                /* we are handling only user-ordered node delete now */
                continue;
            }

            /* get all the attributes */
            LY_CHECK_GOTO(lyd_diff_userord_attrs(iter_first, match_second, options, &userord, &op, &orig_default,
                                                 &value, &orig_value, &key, &orig_key), cleanup);

            /* there must be changes, it is deleted */
            assert(op == LYD_DIFF_OP_DELETE);
            ret = lyd_diff_add(iter_first, op, orig_default, orig_value, key, value, orig_key, diff);

            free(orig_value);
            free(key);
            free(value);
            free(orig_key);
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            /* get all the attributes */
            ret = lyd_diff_attrs(iter_first, match_second, options, &op, &orig_default, &orig_value);

            /* add into diff if there are any changes */
            if (!ret) {
                if (op == LYD_DIFF_OP_DELETE) {
                    ret = lyd_diff_add(iter_first, op, orig_default, orig_value, NULL, NULL, NULL, diff);
                } else {
                    ret = lyd_diff_add(match_second, op, orig_default, orig_value, NULL, NULL, NULL, diff);
                }

                free(orig_value);
                LY_CHECK_GOTO(ret, cleanup);
            } else if (ret == LY_ENOT) {
                ret = LY_SUCCESS;
            } else {
                goto cleanup;
            }
        }

        /* check descendants, if any, recursively */
        if (match_second) {
            LY_CHECK_GOTO(lyd_diff_siblings_r(LYD_CHILD(iter_first), LYD_CHILD(match_second), options, diff), cleanup);
        }

        if (nosiblings) {
            break;
        }
    }

    /* reset all cached positions */
    LY_ARRAY_FOR(userord, u) {
        userord[u].pos = 0;
    }

    /* compare second tree to the first tree - create, user-ordered move */
    LY_LIST_FOR(second, iter_second) {
        assert(!(iter_second->schema->flags & LYS_KEY));
        if ((iter_second->flags & LYD_DEFAULT) && !(options & LYD_DIFF_WITHDEFAULTS)) {
            /* skip default nodes */
            continue;
        }

        if (iter_second->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            lyd_find_sibling_first(first, iter_second, &match_first);
        } else {
            lyd_find_sibling_val(first, iter_second->schema, NULL, 0, &match_first);
        }

        if (match_first && (match_first->flags & LYD_DEFAULT) && !(options & LYD_DIFF_WITHDEFAULTS)) {
            /* ignore default nodes */
            match_first = NULL;
        }

        if (lysc_is_userordered(iter_second->schema)) {
            /* get all the attributes */
            ret = lyd_diff_userord_attrs(match_first, iter_second, options, &userord, &op, &orig_default,
                                         &value, &orig_value, &key, &orig_key);

            /* add into diff if there are any changes */
            if (!ret) {
                ret = lyd_diff_add(iter_second, op, orig_default, orig_value, key, value, orig_key, diff);

                free(orig_value);
                free(key);
                free(value);
                free(orig_key);
                LY_CHECK_GOTO(ret, cleanup);
            } else if (ret == LY_ENOT) {
                ret = LY_SUCCESS;
            } else {
                goto cleanup;
            }
        } else if (!match_first) {
            /* get all the attributes */
            LY_CHECK_GOTO(lyd_diff_attrs(match_first, iter_second, options, &op, &orig_default, &orig_value), cleanup);

            /* there must be changes, it is created */
            assert(op == LYD_DIFF_OP_CREATE);
            ret = lyd_diff_add(iter_second, op, orig_default, orig_value, NULL, NULL, NULL, diff);

            free(orig_value);
            LY_CHECK_GOTO(ret, cleanup);
        } /* else was handled */

        if (nosiblings) {
            break;
        }
    }

cleanup:
    LY_ARRAY_FOR(userord, u) {
        LY_ARRAY_FREE(userord[u].inst);
    }
    LY_ARRAY_FREE(userord);
    return ret;
}

API LY_ERR
lyd_diff(const struct lyd_node *first, const struct lyd_node *second, int options, struct lyd_node **diff)
{
    const struct ly_ctx *ctx;

    LY_CHECK_ARG_RET(NULL, diff, LY_EINVAL);

    if (first) {
        ctx = LYD_NODE_CTX(first);
    } else if (second) {
        ctx = LYD_NODE_CTX(second);
    } else {
        ctx = NULL;
    }

    if (first && second && (lysc_data_parent(first->schema) != lysc_data_parent(second->schema))) {
        LOGERR(ctx, LY_EINVAL, "Invalid arguments - cannot create diff for unrelated data (%s()).", __func__);
        return LY_EINVAL;
    }

    *diff = NULL;

    return lyd_diff_siblings_r(first, second, options, diff);
}

/**
 * @brief Find a matching node in data tree for a diff node.
 *
 * @param[in] first_node First sibling in the data tree.
 * @param[in] diff_node Diff node to match.
 * @param[out] match_p Matching node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_find_node(const struct lyd_node *first_node, const struct lyd_node *diff_node, struct lyd_node **match_p)
{
    if (diff_node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* try to find the exact instance */
        lyd_find_sibling_first(first_node, diff_node, match_p);
    } else {
        /* try to simply find the node, there cannot be more instances */
        lyd_find_sibling_val(first_node, diff_node->schema, NULL, 0, match_p);
    }
    LY_CHECK_ERR_RET(!*match_p, LOGINT(LYD_NODE_CTX(diff_node)), LY_EINT);

    return LY_SUCCESS;
}

/**
 * @brief Learn operation of a diff node.
 *
 * @param[in] diff_node Diff node.
 * @param[out] op Operation.
 * @param[out] key_or_value Optional list instance keys predicate or leaf-list value for move operation.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_get_op(const struct lyd_node *diff_node, const char **op, const char **key_or_value)
{
    struct lyd_meta *meta = NULL;
    const struct lyd_node *diff_parent;
    const char *meta_name, *str;
    int dynamic;

    for (diff_parent = diff_node; diff_parent; diff_parent = (struct lyd_node *)diff_parent->parent) {
        LY_LIST_FOR(diff_parent->meta, meta) {
            if (!strcmp(meta->name, "operation") && !strcmp(meta->annotation->module->name, "yang")) {
                str = lyd_meta2str(meta, &dynamic);
                assert(!dynamic);
                if ((str[0] == 'r') && (diff_parent != diff_node)) {
                    /* we do not care about this operation if it's in our parent */
                    continue;
                }
                *op = str;
                break;
            }
        }
        if (meta) {
            break;
        }
    }
    LY_CHECK_ERR_RET(!meta, LOGINT(LYD_NODE_CTX(diff_node)), LY_EINT);

    *key_or_value = NULL;
    if (lysc_is_userordered(diff_node->schema) && (((*op)[0] == 'c') || ((*op)[0] == 'r'))) {
        if (diff_node->schema->nodetype == LYS_LIST) {
            meta_name = "key";
        } else {
            meta_name = "value";
        }

        LY_LIST_FOR(diff_node->meta, meta) {
            if (!strcmp(meta->name, meta_name) && !strcmp(meta->annotation->module->name, "yang")) {
                str = lyd_meta2str(meta, &dynamic);
                assert(!dynamic);
                *key_or_value = str;
                break;
            }
        }
        LY_CHECK_ERR_RET(!meta, LOGINT(LYD_NODE_CTX(diff_node)), LY_EINT);
    }

    return LY_SUCCESS;
}

/**
 * @brief Insert a diff node into a data tree.
 *
 * @param[in,out] first_node First sibling of the data tree.
 * @param[in] parent_node Data tree sibling parent node.
 * @param[in] new_node Node to insert.
 * @param[in] keys_or_value Optional predicate of relative (leaf-)list instance. If not set, the user-ordered
 * instance will be inserted at the first position.
 * @return err_info, NULL on success.
 */
static LY_ERR
lyd_diff_insert(struct lyd_node **first_node, struct lyd_node *parent_node, struct lyd_node *new_node,
                const char *key_or_value)
{
    LY_ERR ret;
    struct lyd_node *anchor;

    assert(new_node);

    if (!*first_node) {
        if (!parent_node) {
            /* no parent or siblings */
            *first_node = new_node;
            return LY_SUCCESS;
        }

        /* simply insert into parent, no other children */
        if (key_or_value) {
            LOGERR(LYD_NODE_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                   new_node->schema->name);
            return LY_EINVAL;
        }
        return lyd_insert(parent_node, new_node);
    }

    assert(!(*first_node)->parent || ((struct lyd_node *)(*first_node)->parent == parent_node));

    /* simple insert */
    if (!lysc_is_userordered(new_node->schema)) {
        /* insert at the end */
        return lyd_insert_sibling(*first_node, new_node);
    }

    if (key_or_value) {
        /* find the anchor sibling */
        ret = lyd_find_sibling_val(*first_node, new_node->schema, key_or_value, 0, &anchor);
        if (ret == LY_ENOTFOUND) {
            LOGERR(LYD_NODE_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                   new_node->schema->name);
            return LY_EINVAL;
        } else if (ret) {
            return ret;
        }

        /* insert after */
        LY_CHECK_RET(lyd_insert_after(anchor, new_node));
        assert(new_node->prev == anchor);
        if (*first_node == new_node) {
            *first_node = anchor;
        }
    } else {
        if ((*first_node)->schema->flags & LYS_KEY) {
            assert(parent_node && (parent_node->schema->nodetype == LYS_LIST));

            /* find last key */
            anchor = *first_node;
            while (anchor->next && (anchor->next->schema->flags & LYS_KEY)) {
                anchor = anchor->next;
            }
            /* insert after the last key */
            LY_CHECK_RET(lyd_insert_after(anchor, new_node));
        } else {
            /* insert at the beginning */
            LY_CHECK_RET(lyd_insert_before(*first_node, new_node));
            *first_node = new_node;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Apply diff subtree on data tree nodes, recursively.
 *
 * @param[in,out] first_node First sibling of the data tree.
 * @param[in] parent_node Parent of the first sibling.
 * @param[in] diff_node Current diff node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_apply_r(struct lyd_node **first_node, struct lyd_node *parent_node, const struct lyd_node *diff_node,
                 lyd_diff_cb diff_cb, void *cb_data)
{
    LY_ERR ret;
    struct lyd_node *match, *diff_child;
    const char *op, *key_or_value, *str_val;
    int dynamic;
    const struct ly_ctx *ctx = LYD_NODE_CTX(diff_node);

    /* read all the valid attributes */
    LY_CHECK_RET(lyd_diff_get_op(diff_node, &op, &key_or_value));

    /* handle user-ordered (leaf-)lists separately */
    if (key_or_value) {
        assert((op[0] == 'c') || (op[0] == 'r'));
        if (op[0] == 'r') {
            /* find the node (we must have some siblings because the node was only moved) */
            LY_CHECK_RET(lyd_diff_find_node(*first_node, diff_node, &match));
        } else {
            /* duplicate the node(s) */
            match = lyd_dup(diff_node, NULL, LYD_DUP_NO_META);
            LY_CHECK_RET(!match, LY_EMEM);
        }

        /* insert/move the node */
        if (key_or_value[0]) {
            ret = lyd_diff_insert(first_node, parent_node, match, key_or_value);
        } else {
            ret = lyd_diff_insert(first_node, parent_node, match, NULL);
        }
        if (ret) {
            if (op[0] == 'c') {
                lyd_free_tree(match);
            }
            return ret;
        }

        goto next_iter_r;
    }

    /* apply operation */
    switch (op[0]) {
    case 'n':
        /* find the node */
        LY_CHECK_RET(lyd_diff_find_node(*first_node, diff_node, &match));

        if (match->schema->nodetype & LYD_NODE_TERM) {
            /* special case of only dflt flag change */
            if (diff_node->flags & LYD_DEFAULT) {
                match->flags |= LYD_DEFAULT;
            } else {
                match->flags &= ~LYD_DEFAULT;
            }
        } else {
            /* none operation on nodes without children is redundant and hence forbidden */
            if (!LYD_CHILD(diff_node)) {
                LOGINT_RET(ctx);
            }
        }
        break;
    case 'c':
        /* duplicate the node */
        match = lyd_dup(diff_node, NULL, LYD_DUP_NO_META);
        LY_CHECK_RET(!match, LY_EMEM);

        /* insert it at the end */
        ret = 0;
        if (*first_node) {
            ret = lyd_insert_after((*first_node)->prev, match);
        } else if (parent_node) {
            ret = lyd_insert(parent_node, match);
        } else {
            *first_node = match;
        }
        if (ret) {
            lyd_free_tree(match);
            return ret;
        }

        break;
    case 'd':
        /* find the node */
        LY_CHECK_RET(lyd_diff_find_node(*first_node, diff_node, &match));

        /* remove it */
        if ((match == *first_node) && !match->parent) {
            assert(!parent_node);
            /* we have removed the top-level node */
            *first_node = (*first_node)->next;
        }
        lyd_free_tree(match);

        /* we are not going recursively in this case, the whole subtree was already deleted */
        return LY_SUCCESS;
    case 'r':
        LY_CHECK_ERR_RET(diff_node->schema->nodetype != LYS_LEAF, LOGINT(ctx), LY_EINT);

        /* find the node */
        LY_CHECK_RET(lyd_diff_find_node(*first_node, diff_node, &match));

        /* update its value */
        str_val = lyd_value2str((struct lyd_node_term *)diff_node, &dynamic);
        ret = lyd_change_term(match, str_val);
        if (dynamic) {
            free((char *)str_val);
        }
        if (ret && (ret != LY_EEXIST)) {
            LOGINT_RET(ctx);
        }

        /* with flags */
        match->flags = diff_node->flags;
        break;
    default:
        LOGINT_RET(ctx);
    }

next_iter_r:
    if (diff_cb) {
        /* call callback */
        LY_CHECK_RET(diff_cb(diff_node, match, cb_data));
    }

    /* apply diff recursively */
    LY_LIST_FOR(LYD_CHILD(diff_node), diff_child) {
        LY_CHECK_RET(lyd_diff_apply_r(lyd_node_children_p(match), match, diff_child, diff_cb, cb_data));
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_diff_apply_module(struct lyd_node **data, const struct lyd_node *diff, const struct lys_module *mod,
                      lyd_diff_cb diff_cb, void *cb_data)
{
    const struct lyd_node *root;

    LY_LIST_FOR(diff, root) {
        if (mod && (lyd_owner_module(root) != mod)) {
            /* skip data nodes from different modules */
            continue;
        }

        /* apply relevant nodes from the diff datatree */
        LY_CHECK_RET(lyd_diff_apply_r(data, NULL, root, diff_cb, cb_data));
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_diff_apply(struct lyd_node **data, const struct lyd_node *diff)
{
    return lyd_diff_apply_module(data, diff, NULL, NULL, NULL);
}
