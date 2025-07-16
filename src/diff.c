/**
 * @file diff.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief diff functions
 *
 * Copyright (c) 2020 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf, strdup */

#include "diff.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "context.h"
#include "dict.h"
#include "log.h"
#include "ly_common.h"
#include "plugins_exts.h"
#include "plugins_exts/metadata.h"
#include "plugins_types.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

#define LOGERR_META(ctx, meta_name, node) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Failed to find metadata \"%s\" for node \"%s\".", meta_name, __path); \
            free(__path); \
        }

#define LOGERR_NOINST(ctx, node) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Failed to find node \"%s\" instance in data.", __path); \
            free(__path); \
        }

#define LOGERR_UNEXPVAL(ctx, node, data_source) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Unexpected value of node \"%s\" in %s.", __path, data_source); \
            free(__path); \
        }

#define LOGERR_MERGEOP(ctx, node, src_op, trg_op) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Unable to merge operation \"%s\" with \"%s\" for node \"%s\".", \
                    lyd_diff_op2str(trg_op), lyd_diff_op2str(src_op), __path); \
            free(__path); \
        }

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

static enum lyd_diff_op
lyd_diff_str2op(const char *str)
{
    switch (str[0]) {
    case 'c':
        assert(!strcmp(str, "create"));
        return LYD_DIFF_OP_CREATE;
    case 'd':
        assert(!strcmp(str, "delete"));
        return LYD_DIFF_OP_DELETE;
    case 'r':
        assert(!strcmp(str, "replace"));
        return LYD_DIFF_OP_REPLACE;
    case 'n':
        assert(!strcmp(str, "none"));
        return LYD_DIFF_OP_NONE;
    }

    LOGINT(NULL);
    return 0;
}

/**
 * @brief Create diff metadata for a nested user-ordered node with the effective operation "create".
 *
 * @param[in] node User-rodered node to update.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_add_create_nested_userord(struct lyd_node *node)
{
    LY_ERR rc = LY_SUCCESS;
    const char *meta_name, *meta_val;
    size_t buflen = 0, bufused = 0;
    uint32_t pos;
    char *dyn = NULL;

    assert(lysc_is_userordered(node->schema));

    /* get correct metadata name and value */
    if (lysc_is_dup_inst_list(node->schema)) {
        meta_name = "yang:position";

        pos = lyd_list_pos(node);
        if (pos > 1) {
            if (asprintf(&dyn, "%" PRIu32, pos - 1) == -1) {
                LOGMEM(LYD_CTX(node));
                rc = LY_EMEM;
                goto cleanup;
            }
            meta_val = dyn;
        } else {
            meta_val = "";
        }
    } else if (node->schema->nodetype == LYS_LIST) {
        meta_name = "yang:key";

        if (node->prev->next && (node->prev->schema == node->schema)) {
            LY_CHECK_GOTO(rc = lyd_path_list_predicate(node->prev, &dyn, &buflen, &bufused, 0), cleanup);
            meta_val = dyn;
        } else {
            meta_val = "";
        }
    } else {
        meta_name = "yang:value";

        if (node->prev->next && (node->prev->schema == node->schema)) {
            meta_val = lyd_get_value(node->prev);
        } else {
            meta_val = "";
        }
    }

    /* create the metadata */
    LY_CHECK_GOTO(rc = lyd_new_meta(NULL, node, NULL, meta_name, meta_val, LYD_NEW_VAL_STORE_ONLY, NULL), cleanup);

cleanup:
    free(dyn);
    return rc;
}

/**
 * @brief Find metadata/an attribute of a node.
 *
 * @param[in] node Node to search.
 * @param[in] name Metadata/attribute name.
 * @param[out] meta Metadata found, NULL if not found.
 * @param[out] attr Attribute found, NULL if not found.
 */
static void
lyd_diff_find_meta(const struct lyd_node *node, const char *name, struct lyd_meta **meta, struct lyd_attr **attr)
{
    struct lyd_meta *m;
    struct lyd_attr *a;

    if (meta) {
        *meta = NULL;
    }
    if (attr) {
        *attr = NULL;
    }

    if (node->schema) {
        assert(meta);

        LY_LIST_FOR(node->meta, m) {
            if (!strcmp(m->name, name) && !strcmp(m->annotation->module->name, "yang")) {
                *meta = m;
                break;
            }
        }
    } else {
        assert(attr);

        LY_LIST_FOR(((struct lyd_node_opaq *)node)->attr, a) {
            /* name */
            if (strcmp(a->name.name, name)) {
                continue;
            }

            /* module */
            switch (a->format) {
            case LY_VALUE_JSON:
                if (strcmp(a->name.module_name, "yang")) {
                    continue;
                }
                break;
            case LY_VALUE_XML:
                if (strcmp(a->name.module_ns, "urn:ietf:params:xml:ns:yang:1")) {
                    continue;
                }
                break;
            default:
                LOGINT(LYD_CTX(node));
                return;
            }

            *attr = a;
            break;
        }
    }
}

/**
 * @brief Learn operation of a diff node.
 *
 * @param[in] diff_node Diff node.
 * @param[out] op Operation.
 * @param[out] found Whether any @p op was found. If not set, no found operation is an error.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_get_op(const struct lyd_node *diff_node, enum lyd_diff_op *op, ly_bool *found)
{
    struct lyd_meta *meta = NULL;
    struct lyd_attr *attr = NULL;
    const struct lyd_node *diff_parent;
    const char *str;
    char *path;

    for (diff_parent = diff_node; diff_parent; diff_parent = lyd_parent(diff_parent)) {
        lyd_diff_find_meta(diff_parent, "operation", &meta, &attr);
        if (!meta && !attr) {
            continue;
        }

        str = meta ? lyd_get_meta_value(meta) : attr->value;
        if ((str[0] == 'r') && (diff_parent != diff_node)) {
            /* we do not care about this operation if it's in our parent */
            continue;
        }
        *op = lyd_diff_str2op(str);
        if (found) {
            *found = 1;
        }
        return LY_SUCCESS;
    }

    /* operation not found */
    if (found) {
        *found = 0;
        return LY_SUCCESS;
    } else {
        path = lyd_path(diff_node, LYD_PATH_STD, NULL, 0);
        LOGERR(LYD_CTX(diff_node), LY_EINVAL, "Node \"%s\" without an operation.", path);
        free(path);
        return LY_EINT;
    }
}

/**
 * @brief Remove metadata/an attribute from a node.
 *
 * @param[in] node Node to update.
 * @param[in] name Metadata/attribute name.
 */
static void
lyd_diff_del_meta(struct lyd_node *node, const char *name)
{
    struct lyd_meta *meta;
    struct lyd_attr *attr;

    lyd_diff_find_meta(node, name, &meta, &attr);

    if (meta) {
        lyd_free_meta_single(meta);
    } else if (attr) {
        lyd_free_attr_single(LYD_CTX(node), attr);
    }
}

/**
 * @brief Insert a node into siblings.
 *
 * - if the node is part of some other tree, it is automatically unlinked.
 * - difference with the lyd_insert_sibling() is that the subsequent nodes are never inserted.
 * - insert ignores node ordering, which is fine since it's not desirable to sort diff nodes.
 *
 * @param[in] sibling Siblings to insert into, can even be NULL.
 * @param[in] node Node to insert.
 * @param[out] first Return the first sibling after insertion. Can be the address of @p sibling.
 */
static void
lyd_diff_insert_sibling(struct lyd_node *sibling, struct lyd_node *node, struct lyd_node **first_sibling)
{
    assert(node && first_sibling);

    lyd_unlink_ignore_lyds(NULL, node);
    *first_sibling = lyd_first_sibling(sibling);
    lyd_insert_node(NULL, first_sibling, node, LYD_INSERT_NODE_LAST_BY_SCHEMA);
}

LY_ERR
lyd_diff_add(const struct lyd_node *node, enum lyd_diff_op op, const char *orig_default, const char *orig_value,
        const char *key, const char *value, const char *position, const char *orig_key, const char *orig_position,
        struct lyd_node **diff, struct lyd_node **diff_node)
{
    struct lyd_node *dup, *siblings, *match = NULL, *diff_parent = NULL, *elem;
    const struct lyd_node *parent = NULL;
    enum lyd_diff_op cur_op;
    struct lyd_meta *meta;
    uint32_t diff_opts;
    ly_bool found;

    assert(diff);

    /* replace leaf always needs orig-default and orig-value */
    assert((node->schema->nodetype != LYS_LEAF) || (op != LYD_DIFF_OP_REPLACE) || (orig_default && orig_value));

    /* create on userord needs key/value */
    assert((node->schema->nodetype != LYS_LIST) || !(node->schema->flags & LYS_ORDBY_USER) || (op != LYD_DIFF_OP_CREATE) ||
            (lysc_is_dup_inst_list(node->schema) && position) || key);
    assert((node->schema->nodetype != LYS_LEAFLIST) || !(node->schema->flags & LYS_ORDBY_USER) ||
            (op != LYD_DIFF_OP_CREATE) || (lysc_is_dup_inst_list(node->schema) && position) || value);

    /* move on userord needs both key and orig-key/value and orig-value */
    assert((node->schema->nodetype != LYS_LIST) || !(node->schema->flags & LYS_ORDBY_USER) || (op != LYD_DIFF_OP_REPLACE) ||
            (lysc_is_dup_inst_list(node->schema) && position && orig_position) || (key && orig_key));
    assert((node->schema->nodetype != LYS_LEAFLIST) || !(node->schema->flags & LYS_ORDBY_USER) ||
            (op != LYD_DIFF_OP_REPLACE) || (lysc_is_dup_inst_list(node->schema) && position && orig_position) ||
            (value && orig_value));

    if (diff_node) {
        *diff_node = NULL;
    }

    /* find the first existing parent */
    siblings = *diff;
    do {
        /* find next node parent */
        parent = node;
        while (parent->parent && (!diff_parent || (parent->parent->schema != diff_parent->schema))) {
            parent = lyd_parent(parent);
        }

        if (lysc_is_dup_inst_list(parent->schema)) {
            /* assume it never exists, we are not able to distinguish whether it does or not */
            match = NULL;
            break;
        }

        /* check whether it exists in the diff */
        if (lyd_find_sibling_first(siblings, parent, &match)) {
            break;
        }

        /* another parent found */
        diff_parent = match;

        /* move down in the diff */
        siblings = lyd_child_no_keys(match);
    } while (parent != node);

    if (match && (parent == node)) {
        /* special case when there is already an operation on our descendant */
        assert(!lyd_diff_get_op(diff_parent, &cur_op, NULL));

        /* move it to the end where it is expected (matters for user-ordered lists) */
        if (lysc_is_userordered(diff_parent->schema)) {
            for (elem = diff_parent; elem->next && (elem->next->schema == elem->schema); elem = elem->next) {}
            if (elem != diff_parent) {
                LY_CHECK_RET(lyd_insert_after(elem, diff_parent));
            }
        }

        /* will be replaced by the new operation but keep the current op for descendants */
        lyd_diff_del_meta(diff_parent, "operation");
        LY_LIST_FOR(lyd_child_no_keys(diff_parent), elem) {
            lyd_diff_find_meta(elem, "operation", &meta, NULL);
            if (meta) {
                /* explicit operation, fine */
                continue;
            }

            /* set the none operation */
            LY_CHECK_RET(lyd_new_meta(NULL, elem, NULL, "yang:operation", "none", LYD_NEW_VAL_STORE_ONLY, NULL));
        }

        dup = diff_parent;
    } else {
        diff_opts = LYD_DUP_NO_META | LYD_DUP_WITH_PARENTS | LYD_DUP_WITH_FLAGS | LYD_DUP_NO_LYDS;
        if ((op != LYD_DIFF_OP_REPLACE) || !lysc_is_userordered(node->schema) || (node->schema->flags & LYS_CONFIG_R)) {
            /* move applies only to the user-ordered list, no descendants */
            diff_opts |= LYD_DUP_RECURSIVE;
        }

        /* duplicate the subtree (and connect to the diff if possible) */
        LY_CHECK_RET(lyd_dup_single(node, (struct lyd_node_inner *)diff_parent, diff_opts, &dup));

        /* find the first duplicated parent */
        if (!diff_parent) {
            diff_parent = lyd_parent(dup);
            while (diff_parent && diff_parent->parent) {
                diff_parent = lyd_parent(diff_parent);
            }
        } else {
            diff_parent = dup;
            while (diff_parent->parent && (diff_parent->parent->schema == parent->schema)) {
                diff_parent = lyd_parent(diff_parent);
            }
        }

        /* no parent existed, must be manually connected */
        if (!diff_parent) {
            /* there actually was no parent to duplicate */
            lyd_diff_insert_sibling(*diff, dup, diff);
        } else if (!diff_parent->parent) {
            lyd_diff_insert_sibling(*diff, diff_parent, diff);
        }

        /* add parent operation, if any */
        if (diff_parent && (diff_parent != dup)) {
            LY_CHECK_RET(lyd_new_meta(NULL, diff_parent, NULL, "yang:operation", "none", LYD_NEW_VAL_STORE_ONLY, NULL));
        }
    }

    /* add subtree operation if needed */
    LY_CHECK_RET(lyd_diff_get_op(dup, &cur_op, &found));
    if (!found || (cur_op != op)) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:operation", lyd_diff_op2str(op), LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    if (op == LYD_DIFF_OP_CREATE) {
        /* all nested user-ordered (leaf-)lists need special metadata for create op */
        LYD_TREE_DFS_BEGIN(dup, elem) {
            if ((elem != dup) && lysc_is_userordered(elem->schema)) {
                LY_CHECK_RET(lyd_diff_add_create_nested_userord(elem));
            }
            LYD_TREE_DFS_END(dup, elem);
        }
    }

    /* orig-default */
    if (orig_default) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-default", orig_default, LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    /* orig-value */
    if (orig_value) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-value", orig_value, LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    /* key */
    if (key) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:key", key, LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    /* value */
    if (value) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:value", value, LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    /* position */
    if (position) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:position", position, LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    /* orig-key */
    if (orig_key) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-key", orig_key, LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    /* orig-position */
    if (orig_position) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-position", orig_position, LYD_NEW_VAL_STORE_ONLY, NULL));
    }

    if (diff_node) {
        *diff_node = dup;
    }
    return LY_SUCCESS;
}

/**
 * @brief Get a userord entry for a specific user-ordered list/leaf-list. Create if does not exist yet.
 *
 * @param[in] first Node from the first tree, can be NULL (on create).
 * @param[in] schema Schema node of the list/leaf-list.
 * @param[in,out] userord Sized array of userord items.
 * @return Userord item for all the user-ordered list/leaf-list instances.
 */
static struct lyd_diff_userord *
lyd_diff_userord_get(const struct lyd_node *first, const struct lysc_node *schema, struct lyd_diff_userord **userord)
{
    struct lyd_diff_userord *item;
    struct lyd_node *iter;
    const struct lyd_node **node;
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
        LYD_LIST_FOR_INST(lyd_first_sibling(first), first->schema, iter) {
            LY_ARRAY_NEW_RET(schema->module->ctx, item->inst, node, NULL);
            *node = iter;
        }
    }

    return item;
}

/**
 * @brief Check whether there are any metadata differences on 2 nodes.
 *
 * @param[in] first First node.
 * @param[in] second Second node.
 * @return 1 if there are some differences;
 * @return 0 otherwise.
 */
static ly_bool
lyd_diff_node_metadata_check(const struct lyd_node *first, const struct lyd_node *second)
{
    ly_bool rc = 0;
    const struct lys_module *mod;
    const struct lyd_meta *m, **meta_second = NULL;
    uint32_t i, m_second_count = 0;
    const struct lyd_node *first_ch, *second_ch;

    assert(first && second);

    mod = ly_ctx_get_module_implemented(LYD_CTX(first), "yang");
    assert(mod);

    /* collect second node metadata that we can delete from */
    LY_LIST_FOR(second->meta, m) {
        if (m->annotation->module == mod) {
            continue;
        }

        meta_second = ly_realloc(meta_second, (m_second_count + 1) * sizeof *meta_second);
        LY_CHECK_ERR_GOTO(!meta_second, LOGMEM(LYD_CTX(first)), cleanup);
        meta_second[m_second_count] = m;
        ++m_second_count;
    }

    /* go through first metadata and search for a match in second */
    LY_LIST_FOR(first->meta, m) {
        if (m->annotation->module == mod) {
            continue;
        }

        for (i = 0; i < m_second_count; ++i) {
            if (!lyd_compare_meta(m, meta_second[i])) {
                break;
            }
        }

        if (i == m_second_count) {
            /* not found */
            rc = 1;
            goto cleanup;
        }

        /* found, remove from the second metadata to consider */
        --m_second_count;
        if (i < m_second_count) {
            memcpy(&meta_second[i], &meta_second[i + 1], (m_second_count - i) * sizeof *meta_second);
        }
    }

    if (m_second_count) {
        /* not found */
        rc = 1;
        goto cleanup;
    }

    /* for lists, we also need to check their keys */
    if (first->schema->nodetype == LYS_LIST) {
        first_ch = lyd_child(first);
        second_ch = lyd_child(second);
        while (first_ch && lysc_is_key(first_ch->schema)) {
            /* check every key */
            assert(second_ch && (first_ch->schema == second_ch->schema));
            rc = lyd_diff_node_metadata_check(first_ch, second_ch);
            LY_CHECK_GOTO(rc, cleanup);

            first_ch = first_ch->next;
            second_ch = second_ch->next;
        }
    }

cleanup:
    free(meta_second);
    return rc;
}

/**
 * @brief Get all the metadata to be stored in a diff for the 2 nodes. Can be used only for user-ordered
 * lists/leaf-lists.
 *
 * @param[in] first Node from the first tree, can be NULL (on create).
 * @param[in] second Node from the second tree, can be NULL (on delete).
 * @param[in] options Diff options.
 * @param[in] userord_item Userord item of @p first and/or @p second node.
 * @param[out] op Operation.
 * @param[out] orig_default Original default metadata.
 * @param[out] value Value metadata.
 * @param[out] orig_value Original value metadata
 * @param[out] key Key metadata.
 * @param[out] orig_key Original key metadata.
 * @param[out] position Position metadata.
 * @param[out] orig_position Original position metadata.
 * @return LY_SUCCESS on success,
 * @return LY_ENOT if there is no change to be added into diff,
 * @return LY_ERR value on other errors.
 */
static LY_ERR
lyd_diff_userord_attrs(const struct lyd_node *first, const struct lyd_node *second, uint16_t options,
        struct lyd_diff_userord *userord_item, enum lyd_diff_op *op, const char **orig_default, char **value,
        char **orig_value, char **key, char **orig_key, char **position, char **orig_position)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysc_node *schema;
    size_t buflen, bufused;
    uint32_t first_pos, second_pos, comp_opts;

    assert(first || second);

    *orig_default = NULL;
    *value = NULL;
    *orig_value = NULL;
    *key = NULL;
    *orig_key = NULL;
    *position = NULL;
    *orig_position = NULL;

    schema = first ? first->schema : second->schema;
    assert(lysc_is_userordered(schema));

    /* find user-ordered first position */
    if (first) {
        for (first_pos = 0; first_pos < LY_ARRAY_COUNT(userord_item->inst); ++first_pos) {
            if (userord_item->inst[first_pos] == first) {
                break;
            }
        }
        assert(first_pos < LY_ARRAY_COUNT(userord_item->inst));
    } else {
        first_pos = 0;
    }

    /* prepare position of the next instance */
    second_pos = userord_item->pos++;

    /* learn operation first */
    if (!second) {
        *op = LYD_DIFF_OP_DELETE;
    } else if (!first) {
        *op = LYD_DIFF_OP_CREATE;
    } else {
        comp_opts = lysc_is_dup_inst_list(second->schema) ? LYD_COMPARE_FULL_RECURSION : 0;
        if (lyd_compare_single(second, userord_item->inst[second_pos], comp_opts)) {
            /* in first, there is a different instance on the second position, we are going to move 'first' node */
            *op = LYD_DIFF_OP_REPLACE;
        } else if ((options & LYD_DIFF_DEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
            /* default flag change */
            *op = LYD_DIFF_OP_NONE;
        } else if ((options & LYD_DIFF_META) && lyd_diff_node_metadata_check(first, second)) {
            /* metadata changes */
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
    if ((schema->nodetype == LYS_LEAFLIST) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_NONE))) {
        if (first->flags & LYD_DEFAULT) {
            *orig_default = "true";
        } else {
            *orig_default = "false";
        }
    }

    /* value */
    if ((schema->nodetype == LYS_LEAFLIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            *value = strdup(lyd_get_value(userord_item->inst[second_pos - 1]));
            LY_CHECK_ERR_GOTO(!*value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        } else {
            *value = strdup("");
            LY_CHECK_ERR_GOTO(!*value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* orig-value */
    if ((schema->nodetype == LYS_LEAFLIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            *orig_value = strdup(lyd_get_value(userord_item->inst[first_pos - 1]));
            LY_CHECK_ERR_GOTO(!*orig_value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        } else {
            *orig_value = strdup("");
            LY_CHECK_ERR_GOTO(!*orig_value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* key */
    if ((schema->nodetype == LYS_LIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            buflen = bufused = 0;
            LY_CHECK_GOTO(rc = lyd_path_list_predicate(userord_item->inst[second_pos - 1], key, &buflen, &bufused, 0), cleanup);
        } else {
            *key = strdup("");
            LY_CHECK_ERR_GOTO(!*key, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* orig-key */
    if ((schema->nodetype == LYS_LIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            buflen = bufused = 0;
            LY_CHECK_GOTO(rc = lyd_path_list_predicate(userord_item->inst[first_pos - 1], orig_key, &buflen, &bufused, 0), cleanup);
        } else {
            *orig_key = strdup("");
            LY_CHECK_ERR_GOTO(!*orig_key, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* position */
    if (lysc_is_dup_inst_list(schema) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            if (asprintf(position, "%" PRIu32, second_pos) == -1) {
                LOGMEM(schema->module->ctx);
                rc = LY_EMEM;
                goto cleanup;
            }
        } else {
            *position = strdup("");
            LY_CHECK_ERR_GOTO(!*position, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* orig-position */
    if (lysc_is_dup_inst_list(schema) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            if (asprintf(orig_position, "%" PRIu32, first_pos) == -1) {
                LOGMEM(schema->module->ctx);
                rc = LY_EMEM;
                goto cleanup;
            }
        } else {
            *orig_position = strdup("");
            LY_CHECK_ERR_GOTO(!*orig_position, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /*
     * update our instances - apply the change
     */
    if (*op == LYD_DIFF_OP_CREATE) {
        /* insert the instance */
        LY_ARRAY_CREATE_GOTO(schema->module->ctx, userord_item->inst, 1, rc, cleanup);
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

cleanup:
    if (rc) {
        free(*value);
        *value = NULL;
        free(*orig_value);
        *orig_value = NULL;
        free(*key);
        *key = NULL;
        free(*orig_key);
        *orig_key = NULL;
        free(*position);
        *position = NULL;
        free(*orig_position);
        *orig_position = NULL;
    }
    return rc;
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
lyd_diff_attrs(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, enum lyd_diff_op *op,
        const char **orig_default, char **orig_value)
{
    const struct lysc_node *schema;
    const char *str_val;

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
            if ((options & LYD_DIFF_META) && lyd_diff_node_metadata_check(first, second)) {
                /* metadata changes */
                *op = LYD_DIFF_OP_NONE;
            } else {
                /* no changes */
                return LY_ENOT;
            }
            break;
        case LYS_LIST:
        case LYS_LEAFLIST:
            if ((options & LYD_DIFF_DEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
                /* default flag change */
                *op = LYD_DIFF_OP_NONE;
            } else if ((options & LYD_DIFF_META) && lyd_diff_node_metadata_check(first, second)) {
                /* metadata changes */
                *op = LYD_DIFF_OP_NONE;
            } else {
                /* no changes */
                return LY_ENOT;
            }
            break;
        case LYS_LEAF:
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (lyd_compare_single(first, second, 0)) {
                /* different values */
                *op = LYD_DIFF_OP_REPLACE;
            } else if ((options & LYD_DIFF_DEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
                /* default flag change */
                *op = LYD_DIFF_OP_NONE;
            } else if ((options & LYD_DIFF_META) && lyd_diff_node_metadata_check(first, second)) {
                /* metadata changes */
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
    if ((schema->nodetype & LYD_NODE_TERM) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_NONE))) {
        if (first->flags & LYD_DEFAULT) {
            *orig_default = "true";
        } else {
            *orig_default = "false";
        }
    }

    /* orig-value */
    if ((schema->nodetype & (LYS_LEAF | LYS_ANYDATA)) && (*op == LYD_DIFF_OP_REPLACE)) {
        if (schema->nodetype == LYS_LEAF) {
            str_val = lyd_get_value(first);
            *orig_value = strdup(str_val ? str_val : "");
            LY_CHECK_ERR_RET(!*orig_value, LOGMEM(schema->module->ctx), LY_EMEM);
        } else {
            LY_CHECK_RET(lyd_any_value_str(first, orig_value));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Find a matching instance of a node in a data tree.
 *
 * @param[in] siblings Siblings to search in.
 * @param[in] target Target node to search for.
 * @param[in] defaults Whether to consider (or ignore) default values.
 * @param[in,out] dup_inst_ht Duplicate instance cache.
 * @param[out] match Found match, NULL if no matching node found.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_find_match(const struct lyd_node *siblings, const struct lyd_node *target, ly_bool defaults,
        struct ly_ht **dup_inst_ht, struct lyd_node **match)
{
    LY_ERR r;

    if (!target->schema) {
        /* try to find the same opaque node */
        r = lyd_find_sibling_opaq_next(siblings, LYD_NAME(target), match);
    } else if (target->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* try to find the exact instance */
        r = lyd_find_sibling_first(siblings, target, match);
    } else {
        /* try to simply find the node, there cannot be more instances */
        r = lyd_find_sibling_val(siblings, target->schema, NULL, 0, match);
    }
    if (r && (r != LY_ENOTFOUND)) {
        return r;
    }

    /* update match as needed */
    LY_CHECK_RET(lyd_dup_inst_next(match, siblings, dup_inst_ht));

    if (*match && ((*match)->flags & LYD_DEFAULT) && !defaults) {
        /* ignore default nodes */
        *match = NULL;
    }
    return LY_SUCCESS;
}

/**
 * @brief Create a diff metadata instance.
 *
 * @param[in,out] parent Parent node of the diff metadata.
 * @param[in] diff_meta_name Diff metadata name with a mdoule name.
 * @param[in] meta_module Changed metadata module name.
 * @param[in] meta_name Changed metadata name.
 * @param[in] meta_value Changed metadata value.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_node_metadata_add(struct lyd_node *parent, const char *diff_meta_name, const char *meta_module,
        const char *meta_name, const char *meta_value)
{
    LY_ERR rc = LY_SUCCESS;
    char *val = NULL;

    /* prepare the value */
    if (asprintf(&val, "%s:%s=%s", meta_module, meta_name, meta_value) == -1) {
        LOGMEM(LYD_CTX(parent));
        rc = LY_EMEM;
        goto cleanup;
    }

    /* create the metadata */
    LY_CHECK_GOTO(rc = lyd_new_meta(NULL, parent, NULL, diff_meta_name, val, 0, NULL), cleanup);

cleanup:
    free(val);
    return rc;
}

/**
 * @brief Add metadata differences of 2 nodes into a diff node.
 *
 * @param[in] first First node.
 * @param[in] second Second node.
 * @param[in,out] diff_node Diff node to add to.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_node_metadata(const struct lyd_node *first, const struct lyd_node *second, struct lyd_node *diff_node)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lys_module *mod;
    const struct lyd_meta *m, **meta_second = NULL;
    uint32_t i, m_second_count = 0, match_ann_idx;

    mod = ly_ctx_get_module_implemented(LYD_CTX(diff_node), "yang");
    assert(mod);

    /* collect second node metadata that we can delete from */
    if (second) {
        LY_LIST_FOR(second->meta, m) {
            if (m->annotation->module == mod) {
                continue;
            }

            meta_second = ly_realloc(meta_second, (m_second_count + 1) * sizeof *meta_second);
            LY_CHECK_ERR_GOTO(!meta_second, LOGMEM(LYD_CTX(diff_node)); rc = LY_EMEM, cleanup);
            meta_second[m_second_count] = m;
            ++m_second_count;
        }
    }

    /* go through first metadata and search for match in second */
    if (first) {
        LY_LIST_FOR(first->meta, m) {
            if (m->annotation->module == mod) {
                continue;
            }

            match_ann_idx = m_second_count;
            for (i = 0; i < m_second_count; ++i) {
                /* annotation match */
                if (m->annotation != meta_second[i]->annotation) {
                    continue;
                }
                if (match_ann_idx == m_second_count) {
                    match_ann_idx = i;
                }

                /* value match */
                if (m->value.realtype->plugin->compare(LYD_CTX(diff_node), &m->value, &meta_second[i]->value)) {
                    continue;
                }
                break;
            }

            if (i < m_second_count) {
                /* found, no change */
                --m_second_count;
                if (i < m_second_count) {
                    memcpy(&meta_second[i], &meta_second[i + 1], (m_second_count - i) * sizeof *meta_second);
                }
            } else if (match_ann_idx < m_second_count) {
                /* found with a different value, replace */
                i = match_ann_idx;
                rc = lyd_diff_node_metadata_add(diff_node, "yang:meta-replace", meta_second[i]->annotation->module->name,
                        meta_second[i]->name, lyd_get_meta_value(meta_second[i]));
                LY_CHECK_GOTO(rc, cleanup);
                rc = lyd_diff_node_metadata_add(diff_node, "yang:meta-orig", m->annotation->module->name, m->name,
                        lyd_get_meta_value(m));
                LY_CHECK_GOTO(rc, cleanup);

                --m_second_count;
                if (i < m_second_count) {
                    memcpy(&meta_second[i], &meta_second[i + 1], (m_second_count - i) * sizeof *meta_second);
                }
            } else {
                /* not found, delete */
                rc = lyd_diff_node_metadata_add(diff_node, "yang:meta-delete", m->annotation->module->name, m->name,
                        lyd_get_meta_value(m));
                LY_CHECK_GOTO(rc, cleanup);
            }
        }
    }

    for (i = 0; i < m_second_count; ++i) {
        /* not processed, create */
        rc = lyd_diff_node_metadata_add(diff_node, "yang:meta-create", meta_second[i]->annotation->module->name,
                meta_second[i]->name, lyd_get_meta_value(meta_second[i]));
        LY_CHECK_GOTO(rc, cleanup);
    }

cleanup:
    free(meta_second);
    return rc;
}

/**
 * @brief Add metadata differences of 2 subtrees into a diff subtree.
 *
 * @param[in] first_subtree First subtree.
 * @param[in] second_subtree Second subtree.
 * @param[in] keys_only Whether to check all the subtrees recursively or only the direct key children.
 * @param[in,out] diff_subtree Diff subtree to add to.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_node_metadata_r(const struct lyd_node *first_subtree, const struct lyd_node *second_subtree, ly_bool keys_only,
        struct lyd_node *diff_subtree)
{
    struct lyd_node *diff_node, *first, *second;

    /* metadata diff on the node itself */
    LY_CHECK_RET(lyd_diff_node_metadata(first_subtree, second_subtree, diff_subtree));

    /* metadata diff on all the children, recursively */
    LY_LIST_FOR(lyd_child(diff_subtree), diff_node) {
        if (keys_only && !lysc_is_key(diff_node->schema)) {
            break;
        }

        lyd_find_sibling_first(lyd_child(first_subtree), diff_node, &first);
        lyd_find_sibling_first(lyd_child(second_subtree), diff_node, &second);
        LY_CHECK_RET(lyd_diff_node_metadata_r(first, second, keys_only, diff_node));
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
 * @param[in] nosiblings Whether to skip following siblings.
 * @param[in,out] diff Diff to append to.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_siblings_r(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, ly_bool nosiblings,
        struct lyd_node **diff)
{
    LY_ERR rc = LY_SUCCESS, r;
    const struct lyd_node *iter_first, *iter_second;
    struct lyd_node *match_second, *match_first, *diff_node;
    struct lyd_diff_userord *userord = NULL, *userord_item;
    struct ly_ht *dup_inst_first = NULL, *dup_inst_second = NULL;
    LY_ARRAY_COUNT_TYPE u;
    enum lyd_diff_op op;
    const char *orig_default;
    char *orig_value, *key, *value, *position, *orig_key, *orig_position;

    /* compare first tree to the second tree - delete, replace, none */
    LY_LIST_FOR(first, iter_first) {
        if (!iter_first->schema) {
            continue;
        }

        assert(!(iter_first->schema->flags & LYS_KEY));
        if ((iter_first->flags & LYD_DEFAULT) && !(options & LYD_DIFF_DEFAULTS)) {
            /* skip default nodes */
            continue;
        }

        diff_node = NULL;

        /* find a match in the second tree */
        LY_CHECK_GOTO(rc = lyd_diff_find_match(second, iter_first, options & LYD_DIFF_DEFAULTS, &dup_inst_second,
                &match_second), cleanup);

        if (lysc_is_userordered(iter_first->schema)) {
            /* get (create) userord entry */
            userord_item = lyd_diff_userord_get(iter_first, iter_first->schema, &userord);
            LY_CHECK_ERR_GOTO(!userord_item, LOGMEM(LYD_CTX(iter_first)); rc = LY_EMEM, cleanup);

            /* we are handling only user-ordered node delete now */
            if (!match_second) {
                /* get all the attributes */
                LY_CHECK_GOTO(rc = lyd_diff_userord_attrs(iter_first, match_second, options, userord_item, &op,
                        &orig_default, &value, &orig_value, &key, &orig_key, &position, &orig_position), cleanup);

                /* there must be changes, it is deleted */
                assert(op == LYD_DIFF_OP_DELETE);
                rc = lyd_diff_add(iter_first, op, orig_default, orig_value, key, value, position, orig_key,
                        orig_position, diff, &diff_node);
                free(orig_value);
                free(key);
                free(value);
                free(position);
                free(orig_key);
                free(orig_position);
                LY_CHECK_GOTO(rc, cleanup);
            }
        } else {
            /* get all the attributes */
            r = lyd_diff_attrs(iter_first, match_second, options, &op, &orig_default, &orig_value);
            if (r && (r != LY_ENOT)) {
                goto cleanup;
            }

            /* add into diff if there are any changes */
            if (!r) {
                if (op == LYD_DIFF_OP_DELETE) {
                    rc = lyd_diff_add(iter_first, op, orig_default, orig_value, NULL, NULL, NULL, NULL, NULL, diff, &diff_node);
                } else {
                    assert(match_second);
                    rc = lyd_diff_add(match_second, op, orig_default, orig_value, NULL, NULL, NULL, NULL, NULL, diff, &diff_node);
                }
                free(orig_value);
                LY_CHECK_GOTO(rc, cleanup);
            }
        }

        if (match_second) {
            if ((options & LYD_DIFF_META) && diff_node) {
                /* create metadata diff for the node (and list keys, if relevant) */
                LY_CHECK_GOTO(rc = lyd_diff_node_metadata_r(iter_first, match_second, 1, diff_node), cleanup);
            }

            /* check descendants, if any, recursively */
            LY_CHECK_GOTO(rc = lyd_diff_siblings_r(lyd_child_no_keys(iter_first), lyd_child_no_keys(match_second),
                    options, 0, diff), cleanup);
        } else {
            if ((options & LYD_DIFF_META) && diff_node) {
                /* create metadata diff for the node and all its descendants */
                LY_CHECK_GOTO(rc = lyd_diff_node_metadata_r(iter_first, NULL, 0, diff_node), cleanup);
            }
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
        if (!iter_second->schema) {
            continue;
        }

        assert(!(iter_second->schema->flags & LYS_KEY));
        if ((iter_second->flags & LYD_DEFAULT) && !(options & LYD_DIFF_DEFAULTS)) {
            /* skip default nodes */
            continue;
        }

        diff_node = NULL;

        /* find a match in the first tree */
        LY_CHECK_GOTO(rc = lyd_diff_find_match(first, iter_second, options & LYD_DIFF_DEFAULTS, &dup_inst_first,
                &match_first), cleanup);

        if (lysc_is_userordered(iter_second->schema)) {
            /* get userord entry */
            userord_item = lyd_diff_userord_get(match_first, iter_second->schema, &userord);
            LY_CHECK_ERR_GOTO(!userord_item, LOGMEM(LYD_CTX(iter_second)); rc = LY_EMEM, cleanup);

            /* get all the attributes */
            r = lyd_diff_userord_attrs(match_first, iter_second, options, userord_item, &op, &orig_default,
                    &value, &orig_value, &key, &orig_key, &position, &orig_position);
            if (r && (r != LY_ENOT)) {
                goto cleanup;
            }

            /* add into diff if there are any changes */
            if (!r) {
                rc = lyd_diff_add(iter_second, op, orig_default, orig_value, key, value, position, orig_key,
                        orig_position, diff, &diff_node);
                free(orig_value);
                free(key);
                free(value);
                free(position);
                free(orig_key);
                free(orig_position);
                LY_CHECK_GOTO(rc, cleanup);
            }
        } else if (!match_first) {
            /* get all the attributes */
            LY_CHECK_GOTO(rc = lyd_diff_attrs(match_first, iter_second, options, &op, &orig_default, &orig_value), cleanup);

            /* there must be changes, it is created */
            assert(op == LYD_DIFF_OP_CREATE);
            rc = lyd_diff_add(iter_second, op, orig_default, orig_value, NULL, NULL, NULL, NULL, NULL, diff, &diff_node);
            free(orig_value);
            LY_CHECK_GOTO(rc, cleanup);
        } /* else was handled */

        if ((options & LYD_DIFF_META) && diff_node) {
            /* create metadata diff for the node and all its descendants */
            LY_CHECK_GOTO(rc = lyd_diff_node_metadata_r(match_first, iter_second, 0, diff_node), cleanup);
        }

        if (nosiblings) {
            break;
        }
    }

cleanup:
    lyd_dup_inst_free(dup_inst_first);
    lyd_dup_inst_free(dup_inst_second);
    LY_ARRAY_FOR(userord, u) {
        LY_ARRAY_FREE(userord[u].inst);
    }
    LY_ARRAY_FREE(userord);
    if (rc) {
        lyd_free_siblings(*diff);
        *diff = NULL;
    }
    return rc;
}

static LY_ERR
lyd_diff(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, ly_bool nosiblings,
        struct lyd_node **diff)
{
    const struct ly_ctx *ctx;

    LY_CHECK_ARG_RET(NULL, diff, LY_EINVAL);

    if (first) {
        ctx = LYD_CTX(first);
    } else if (second) {
        ctx = LYD_CTX(second);
    } else {
        ctx = NULL;
    }

    if (first && second && (lysc_data_parent(first->schema) != lysc_data_parent(second->schema))) {
        LOGERR(ctx, LY_EINVAL, "Invalid arguments - cannot create diff for unrelated data (%s()).", __func__);
        return LY_EINVAL;
    }

    *diff = NULL;

    return lyd_diff_siblings_r(first, second, options, nosiblings, diff);
}

LIBYANG_API_DEF LY_ERR
lyd_diff_tree(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, struct lyd_node **diff)
{
    return lyd_diff(first, second, options, 1, diff);
}

LIBYANG_API_DEF LY_ERR
lyd_diff_siblings(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, struct lyd_node **diff)
{
    return lyd_diff(first, second, options, 0, diff);
}

/**
 * @brief Insert a diff node into a data tree.
 *
 * @param[in,out] first_node First sibling of the data tree.
 * @param[in] parent_node Data tree sibling parent node.
 * @param[in] new_node Node to insert.
 * @param[in] userord_anchor Optional anchor (key, value, or position) of relative (leaf-)list instance. If not set,
 * the user-ordered instance will be inserted at the first position.
 * @return err_info, NULL on success.
 */
static LY_ERR
lyd_diff_insert(struct lyd_node **first_node, struct lyd_node *parent_node, struct lyd_node *new_node,
        const char *userord_anchor)
{
    LY_ERR ret;
    struct lyd_node *anchor;
    uint32_t pos, anchor_pos;
    int found;

    assert(new_node);

    if (!*first_node) {
        if (!parent_node) {
            /* no parent or siblings */
            *first_node = new_node;
            return LY_SUCCESS;
        }

        /* simply insert into parent, no other children */
        if (userord_anchor) {
            LOGERR(LYD_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                    new_node->schema->name);
            return LY_EINVAL;
        }
        return lyd_insert_child(parent_node, new_node);
    }

    assert(!(*first_node)->parent || (lyd_parent(*first_node) == parent_node));

    if (!lysc_is_userordered(new_node->schema)) {
        /* simple insert */
        return lyd_insert_sibling(*first_node, new_node, first_node);
    }

    if (userord_anchor) {
        /* find the anchor sibling */
        if (lysc_is_dup_inst_list(new_node->schema)) {
            anchor_pos = atoi(userord_anchor);
            if (!anchor_pos) {
                LOGERR(LYD_CTX(new_node), LY_EINVAL, "Invalid user-ordered anchor value \"%s\".", userord_anchor);
                return LY_EINVAL;
            }

            found = 0;
            pos = 1;
            LYD_LIST_FOR_INST(*first_node, new_node->schema, anchor) {
                if (pos == anchor_pos) {
                    found = 1;
                    break;
                }
                ++pos;
            }
            if (!found) {
                LOGERR(LYD_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                        new_node->schema->name);
                return LY_EINVAL;
            }
        } else {
            ret = lyd_find_sibling_val(*first_node, new_node->schema, userord_anchor, 0, &anchor);
            if (ret == LY_ENOTFOUND) {
                LOGERR(LYD_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                        new_node->schema->name);
                return LY_EINVAL;
            } else if (ret) {
                return ret;
            }
        }

        /* insert after */
        LY_CHECK_RET(lyd_insert_after(anchor, new_node));
        assert(new_node->prev == anchor);
        if (*first_node == new_node) {
            *first_node = anchor;
        }
    } else {
        /* find the first instance */
        ret = lyd_find_sibling_val(*first_node, new_node->schema, NULL, 0, &anchor);
        LY_CHECK_RET(ret && (ret != LY_ENOTFOUND), ret);

        if (anchor) {
            /* insert before the first instance */
            LY_CHECK_RET(lyd_insert_before(anchor, new_node));
            if ((*first_node)->prev->next) {
                assert(!new_node->prev->next);
                *first_node = new_node;
            }
        } else {
            /* insert anywhere */
            LY_CHECK_RET(lyd_insert_sibling(*first_node, new_node, first_node));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse a diff metadata value into the changed metadata instance name (with module name) and value.
 *
 * @param[in] meta Diff metadata instance.
 * @param[out] name Optional changed metadata name.
 * @param[out] value Optional changed metadata value.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_apply_metadata_parse(const struct lyd_meta *meta, char **name, const char **value)
{
    LY_ERR rc = LY_SUCCESS;
    const char *v, *ptr;

    v = lyd_get_meta_value(meta);
    ptr = strchr(v, '=');
    LY_CHECK_ERR_GOTO(!ptr, LOGINT(meta->annotation->module->ctx); rc = LY_EINT, cleanup);

    if (name) {
        *name = strndup(v, ptr - v);
        LY_CHECK_ERR_GOTO(!*name, LOGMEM(meta->annotation->module->ctx); rc = LY_EMEM, cleanup);
    }

    if (value) {
        *value = ptr + 1;
    }

cleanup:
    return rc;
}

/**
 * @brief Find a metadata instance with a specific value.
 *
 * @param[in] meta First metadata to consider.
 * @param[in] name Metadata name with module name.
 * @param[in] value Metadata value.
 * @param[in] log Whether to log an error if not found.
 * @param[out] match Found metadata.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_metadata_find(const struct lyd_meta *meta, const char *name, const char *value, ly_bool log,
        struct lyd_meta **match)
{
    const struct lyd_meta *m;

    for (m = meta; (m = lyd_find_meta(m, NULL, name)); m = m->next) {
        if (!strcmp(lyd_get_meta_value(m), value)) {
            *match = (struct lyd_meta *)m;
            return LY_SUCCESS;
        }
    }

    *match = NULL;
    if (log) {
        LOGINT(meta->annotation->module->ctx);
        return LY_EINT;
    } else {
        return LY_SUCCESS;
    }
}

/**
 * @brief Add a metadata into an array.
 *
 * @param[in] meta Metadata to add.
 * @param[in,out] meta_a Metadata array.
 * @param[in,out] meta_a_count Count of @p meta_a items.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_meta_store(struct lyd_meta *meta, struct lyd_meta ***meta_a, uint32_t *meta_a_count)
{
    *meta_a = ly_realloc(*meta_a, (*meta_a_count + 1) * sizeof **meta_a);
    LY_CHECK_ERR_RET(!*meta_a, LOGMEM(meta->annotation->module->ctx), LY_EMEM);
    (*meta_a)[*meta_a_count] = meta;
    ++(*meta_a_count);

    return LY_SUCCESS;
}

/**
 * @brief Align all meta-replace metadata with its matching meta-orig metadata.
 *
 * @param[in] meta_replace Meta-replace metadata.
 * @param[in] mr_count Count of @p meta_replace.
 * @param[in,out] meta_orig Meta-orig metadata to align and reorder.
 * @param[in] mo_count Count of @p meta_orig.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_metadata_replace_orig_align(struct lyd_meta **meta_replace, uint32_t mr_count, struct lyd_meta **meta_orig,
        uint32_t mo_count)
{
    const struct ly_ctx *ctx;
    struct lyd_meta *m;
    const char *val1, *val2, *ptr;
    uint32_t i, j;

    if (!mr_count) {
        return LY_SUCCESS;
    }

    ctx = meta_replace[0]->annotation->module->ctx;

    LY_CHECK_ERR_RET(mr_count != mo_count, LOGINT(ctx), LY_EINT);

    for (i = 0; i < mr_count; ++i) {
        /* meta-replace value */
        val1 = lyd_get_meta_value(meta_replace[i]);
        ptr = strchr(val1, '=');
        LY_CHECK_ERR_RET(!ptr, LOGINT(ctx), LY_EINT);
        ++ptr;

        /* find matching meta-orig value */
        j = i;
        while (j < mo_count) {
            val2 = lyd_get_meta_value(meta_orig[j]);
            if (!strncmp(val1, val2, ptr - val1)) {
                break;
            }

            ++j;
        }
        LY_CHECK_ERR_RET(j == mo_count, LOGINT(ctx), LY_EINT);

        if (j != i) {
            /* non-matching index, move it */
            m = meta_orig[i];
            meta_orig[i] = meta_orig[j];
            meta_orig[j] = m;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Apply any metadata changes in the diff.
 *
 * @param[in,out] node Node to change.
 * @param[in] diff_node Diff node to read the metadata changes from.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_apply_metadata(struct lyd_node *node, const struct lyd_node *diff_node)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_meta *m, *m2, **meta_replace = NULL, **meta_orig = NULL;
    uint32_t i, mr_count = 0, mo_count = 0;
    const struct lys_module *mod;
    const char *meta_value, *old_meta_value;
    char *meta_name = NULL;
    const struct lyd_node *diff_ch;
    struct lyd_node *node_ch;

    mod = ly_ctx_get_module_implemented(LYD_CTX(node), "yang");
    assert(mod);

    /* go through all the metadata */
    LY_LIST_FOR(diff_node->meta, m) {
        if (m->annotation->module != mod) {
            continue;
        }

        if (!strcmp(m->name, "meta-create")) {
            /* parse the value */
            LY_CHECK_GOTO(rc = lyd_diff_apply_metadata_parse(m, &meta_name, &meta_value), cleanup);

            /* create the metadata instance */
            LY_CHECK_GOTO(rc = lyd_new_meta(NULL, node, NULL, meta_name, meta_value, 0, NULL), cleanup);

            free(meta_name);
            meta_name = NULL;
        } else if (!strcmp(m->name, "meta-delete")) {
            /* parse the value */
            LY_CHECK_GOTO(rc = lyd_diff_apply_metadata_parse(m, &meta_name, &meta_value), cleanup);

            /* find the metadata instance and free it */
            LY_CHECK_GOTO(rc = lyd_diff_metadata_find(node->meta, meta_name, meta_value, 1, &m2), cleanup);
            lyd_free_meta_single(m2);

            free(meta_name);
            meta_name = NULL;
        } else if (!strcmp(m->name, "meta-replace")) {
            /* just store it, to be able to correctly match to 'meta-orig' */
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &meta_replace, &mr_count), cleanup);
        } else if (!strcmp(m->name, "meta-orig")) {
            /* just store it */
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &meta_orig, &mo_count), cleanup);
        }
    }

    /* make sure meta_replace and meta_orig arrays are aligned */
    LY_CHECK_GOTO(rc = lyd_diff_metadata_replace_orig_align(meta_replace, mr_count, meta_orig, mo_count), cleanup);

    /* process replaced metadata */
    LY_CHECK_ERR_GOTO(mr_count != mo_count, LOGINT(LYD_CTX(node)); rc = LY_EINT, cleanup);
    for (i = 0; i < mr_count; ++i) {
        /* get the changed meta name with '=' */
        LY_CHECK_GOTO(rc = lyd_diff_apply_metadata_parse(meta_replace[i], &meta_name, &meta_value), cleanup);

        /* parse the orig value */
        LY_CHECK_GOTO(rc = lyd_diff_apply_metadata_parse(meta_orig[i], NULL, &old_meta_value), cleanup);

        /* find the metadata instance */
        LY_CHECK_GOTO(rc = lyd_diff_metadata_find(node->meta, meta_name, old_meta_value, 0, &m2), cleanup);
        LY_CHECK_ERR_GOTO(!m2, LOGINT(LYD_CTX(node)); rc = LY_EINT, cleanup);

        /* change its value */
        LY_CHECK_GOTO(rc = lyd_change_meta(m2, meta_value), cleanup);

        /* meta-orig spent */
        meta_orig[i] = NULL;

        free(meta_name);
        meta_name = NULL;
    }

    /* for lists, we also need to process their keys */
    if (diff_node->schema->nodetype == LYS_LIST) {
        diff_ch = lyd_child(diff_node);
        node_ch = lyd_child(node);
        while (diff_ch && lysc_is_key(diff_ch->schema)) {
            /* process every key */
            assert(node_ch && (diff_ch->schema == node_ch->schema));
            rc = lyd_diff_apply_metadata(node_ch, diff_ch);
            LY_CHECK_GOTO(rc, cleanup);

            diff_ch = diff_ch->next;
            node_ch = node_ch->next;
        }
    }

cleanup:
    free(meta_name);
    free(meta_replace);
    free(meta_orig);
    return rc;
}

/**
 * @brief Apply diff subtree on data tree nodes, recursively.
 *
 * @param[in,out] first_node First sibling of the data tree.
 * @param[in] parent_node Parent of the first sibling.
 * @param[in] diff_node Current diff node.
 * @param[in] diff_cb Optional diff callback.
 * @param[in] cb_data User data for @p diff_cb.
 * @param[in,out] dup_inst Duplicate instance cache for all @p diff_node siblings.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_apply_r(struct lyd_node **first_node, struct lyd_node *parent_node, const struct lyd_node *diff_node,
        lyd_diff_cb diff_cb, void *cb_data, struct ly_ht **dup_inst)
{
    LY_ERR rc = LY_SUCCESS, r;
    struct lyd_node *match, *diff_child;
    const char *str_val, *meta_str;
    enum lyd_diff_op op;
    struct lyd_meta *meta;
    struct ly_ht *child_dup_inst = NULL;
    const struct ly_ctx *ctx = LYD_CTX(diff_node);

    /* read all the valid attributes */
    LY_CHECK_RET(lyd_diff_get_op(diff_node, &op, NULL));

    /* handle specific user-ordered (leaf-)lists operations separately */
    if (lysc_is_userordered(diff_node->schema) && ((op == LYD_DIFF_OP_CREATE) || (op == LYD_DIFF_OP_REPLACE))) {
        if (op == LYD_DIFF_OP_REPLACE) {
            /* find the node (we must have some siblings because the node was only moved) */
            LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
            LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);
        } else {
            /* duplicate the node */
            LY_CHECK_RET(lyd_dup_single(diff_node, NULL, LYD_DUP_NO_META, &match));
        }

        /* get "key", "value", or "position" metadata string value */
        if (lysc_is_dup_inst_list(diff_node->schema)) {
            meta_str = "yang:position";
        } else if (diff_node->schema->nodetype == LYS_LIST) {
            meta_str = "yang:key";
        } else {
            meta_str = "yang:value";
        }
        meta = lyd_find_meta(diff_node->meta, NULL, meta_str);
        LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_str, diff_node), LY_EINVAL);
        str_val = lyd_get_meta_value(meta);

        /* insert/move the node */
        if (str_val[0]) {
            r = lyd_diff_insert(first_node, parent_node, match, str_val);
        } else {
            r = lyd_diff_insert(first_node, parent_node, match, NULL);
        }
        if (r) {
            if (op == LYD_DIFF_OP_CREATE) {
                lyd_free_tree(match);
            }
            return r;
        }
    } else {
        /* apply operation */
        switch (op) {
        case LYD_DIFF_OP_NONE:
            /* find the node */
            LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
            LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);

            if (match->schema->nodetype & LYD_NODE_TERM) {
                /* special case of only dflt flag change */
                if (diff_node->flags & LYD_DEFAULT) {
                    match->flags |= LYD_DEFAULT;
                } else {
                    match->flags &= ~LYD_DEFAULT;
                }
            }
            break;
        case LYD_DIFF_OP_CREATE:
            /* duplicate the node */
            LY_CHECK_RET(lyd_dup_single(diff_node, NULL, LYD_DUP_NO_META, &match));

            /* insert it at the end */
            if (parent_node) {
                if (match->flags & LYD_EXT) {
                    r = lyplg_ext_insert(parent_node, match);
                } else {
                    r = lyd_insert_child(parent_node, match);
                }
            } else {
                r = lyd_insert_sibling(*first_node, match, first_node);
            }
            if (r) {
                lyd_free_tree(match);
                return r;
            }

            break;
        case LYD_DIFF_OP_DELETE:
            /* find the node */
            LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
            LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);

            /* remove it */
            if ((match == *first_node) && !match->parent) {
                assert(!parent_node);
                /* we have removed the top-level node */
                *first_node = (*first_node)->next;
            }
            lyd_free_tree(match);

            /* we are not going recursively in this case, the whole subtree was already deleted */
            return LY_SUCCESS;
        case LYD_DIFF_OP_REPLACE:
            if (!(diff_node->schema->nodetype & (LYS_LEAF | LYS_ANYDATA))) {
                LOGERR(ctx, LY_EINVAL, "Operation \"replace\" is invalid for %s node \"%s\".",
                        lys_nodetype2str(diff_node->schema->nodetype), LYD_NAME(diff_node));
                return LY_EINVAL;
            }

            /* find the node */
            LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
            LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);

            /* update the value */
            if (diff_node->schema->nodetype == LYS_LEAF) {
                r = lyd_change_term(match, lyd_get_value(diff_node));
                LY_CHECK_ERR_RET(r && (r != LY_EEXIST), LOGERR_UNEXPVAL(ctx, match, "data"), LY_EINVAL);
            } else {
                struct lyd_node_any *any = (struct lyd_node_any *)diff_node;

                LY_CHECK_RET(lyd_any_copy_value(match, &any->value, any->value_type));
            }

            /* with flags */
            match->flags = diff_node->flags;
            break;
        default:
            LOGINT_RET(ctx);
        }
    }

    /* apply any metadata changes */
    LY_CHECK_RET(lyd_diff_apply_metadata(match, diff_node));

    if (diff_cb) {
        /* call callback */
        LY_CHECK_RET(diff_cb(diff_node, match, cb_data));
    }

    /* apply diff recursively */
    LY_LIST_FOR(lyd_child_no_keys(diff_node), diff_child) {
        rc = lyd_diff_apply_r(lyd_node_child_p(match), match, diff_child, diff_cb, cb_data, &child_dup_inst);
        if (rc) {
            break;
        }
    }

    lyd_dup_inst_free(child_dup_inst);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_apply_module(struct lyd_node **data, const struct lyd_node *diff, const struct lys_module *mod,
        lyd_diff_cb diff_cb, void *cb_data)
{
    const struct lyd_node *root;
    struct ly_ht *dup_inst = NULL;
    LY_ERR ret = LY_SUCCESS;

    LY_LIST_FOR(diff, root) {
        if (mod && (lyd_owner_module(root) != mod)) {
            /* skip data nodes from different modules */
            continue;
        }

        /* apply relevant nodes from the diff datatree */
        ret = lyd_diff_apply_r(data, NULL, root, diff_cb, cb_data, &dup_inst);
        if (ret) {
            break;
        }
    }

    lyd_dup_inst_free(dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_apply_all(struct lyd_node **data, const struct lyd_node *diff)
{
    return lyd_diff_apply_module(data, diff, NULL, NULL, NULL);
}

/**
 * @brief Update operations on a diff node when the new operation is NONE.
 *
 * @param[in] diff_match Node from the diff.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_none(struct lyd_node *diff_match, enum lyd_diff_op cur_op, const struct lyd_node *src_diff)
{
    switch (cur_op) {
    case LYD_DIFF_OP_NONE:
    case LYD_DIFF_OP_CREATE:
    case LYD_DIFF_OP_REPLACE:
        if (src_diff->schema->nodetype & LYD_NODE_TERM) {
            /* NONE on a term means only its dflt flag was changed */
            diff_match->flags &= ~LYD_DEFAULT;
            diff_match->flags |= src_diff->flags & LYD_DEFAULT;
        }
        break;
    default:
        /* delete operation is not valid */
        LOGERR_MERGEOP(LYD_CTX(diff_match), diff_match, cur_op, LYD_DIFF_OP_NONE);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Set a specific operation of a node. Delete the previous operation, if any.
 * Does not change the default flag.
 *
 * @param[in] node Node to change.
 * @param[in] op Operation to set.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_change_op(struct lyd_node *node, enum lyd_diff_op op)
{
    lyd_diff_del_meta(node, "operation");

    if (node->schema) {
        return lyd_new_meta(LYD_CTX(node), node, NULL, "yang:operation", lyd_diff_op2str(op), LYD_NEW_VAL_STORE_ONLY, NULL);
    } else {
        return lyd_new_attr(node, "yang", "operation", lyd_diff_op2str(op), NULL);
    }
}

/**
 * @brief Update operations on a diff node when the new operation is REPLACE.
 *
 * @param[in] diff_match Node from the diff.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_replace(struct lyd_node *diff_match, enum lyd_diff_op cur_op, const struct lyd_node *src_diff)
{
    LY_ERR ret;
    const char *str_val, *meta_name, *orig_meta_name;
    struct lyd_meta *meta;
    const struct lys_module *mod;
    const struct lyd_node_any *any;
    const struct ly_ctx *ctx = LYD_CTX(diff_match);

    /* get "yang" module for the metadata */
    mod = ly_ctx_get_module_latest(LYD_CTX(diff_match), "yang");
    assert(mod);

    switch (cur_op) {
    case LYD_DIFF_OP_REPLACE:
    case LYD_DIFF_OP_CREATE:
        switch (diff_match->schema->nodetype) {
        case LYS_LIST:
        case LYS_LEAFLIST:
            /* it was created/moved somewhere, but now it will be created/moved somewhere else,
             * keep orig_key/orig_value (only replace oper) and replace key/value */
            assert(lysc_is_userordered(diff_match->schema));
            if (lysc_is_dup_inst_list(diff_match->schema)) {
                meta_name = "position";
            } else if (diff_match->schema->nodetype == LYS_LIST) {
                meta_name = "key";
            } else {
                meta_name = "value";
            }

            lyd_diff_del_meta(diff_match, meta_name);
            meta = lyd_find_meta(src_diff->meta, mod, meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_name, src_diff), LY_EINVAL);
            LY_CHECK_RET(lyd_dup_meta_single(meta, diff_match, NULL));
            break;
        case LYS_LEAF:
            /* replaced with the exact same value, impossible */
            if (!lyd_compare_single(diff_match, src_diff, 0)) {
                LOGERR_UNEXPVAL(ctx, diff_match, "target diff");
                return LY_EINVAL;
            }

            /* modify the node value */
            if (lyd_change_term(diff_match, lyd_get_value(src_diff))) {
                LOGINT_RET(LYD_CTX(src_diff));
            }

            if (cur_op == LYD_DIFF_OP_REPLACE) {
                /* compare values whether there is any change at all */
                meta = lyd_find_meta(diff_match->meta, mod, "orig-value");
                LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, "orig-value", diff_match), LY_EINVAL);
                str_val = lyd_get_meta_value(meta);
                ret = lyd_value_compare((struct lyd_node_term *)diff_match, str_val, strlen(str_val));
                if (!ret) {
                    /* values are the same, remove orig-value meta and set oper to NONE */
                    lyd_free_meta_single(meta);
                    LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_NONE));
                }
            }

            /* modify the default flag */
            diff_match->flags &= ~LYD_DEFAULT;
            diff_match->flags |= src_diff->flags & LYD_DEFAULT;
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (!lyd_compare_single(diff_match, src_diff, 0)) {
                LOGERR_UNEXPVAL(ctx, diff_match, "target diff");
                return LY_EINVAL;
            }

            /* modify the node value */
            any = (struct lyd_node_any *)src_diff;
            LY_CHECK_RET(lyd_any_copy_value(diff_match, &any->value, any->value_type));
            break;
        default:
            LOGINT_RET(LYD_CTX(src_diff));
        }
        break;
    case LYD_DIFF_OP_NONE:
        switch (diff_match->schema->nodetype) {
        case LYS_LIST:
            /* it is moved now */
            assert(lysc_is_userordered(diff_match->schema));

            /* change the operation */
            LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_REPLACE));

            /* set orig-meta and meta */
            if (lysc_is_dup_inst_list(diff_match->schema)) {
                meta_name = "position";
                orig_meta_name = "orig-position";
            } else {
                meta_name = "key";
                orig_meta_name = "orig-key";
            }

            meta = lyd_find_meta(src_diff->meta, mod, orig_meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, orig_meta_name, src_diff), LY_EINVAL);
            LY_CHECK_RET(lyd_dup_meta_single(meta, diff_match, NULL));

            meta = lyd_find_meta(src_diff->meta, mod, meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_name, src_diff), LY_EINVAL);
            LY_CHECK_RET(lyd_dup_meta_single(meta, diff_match, NULL));
            break;
        case LYS_LEAF:
            /* only dflt flag changed, now value changed as well, update the operation */
            LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_REPLACE));

            /* modify the node value */
            if (lyd_change_term(diff_match, lyd_get_value(src_diff))) {
                LOGINT_RET(LYD_CTX(src_diff));
            }
            break;
        default:
            LOGINT_RET(LYD_CTX(src_diff));
        }
        break;
    default:
        /* delete operation is not valid */
        LOGERR_MERGEOP(ctx, diff_match, cur_op, LYD_DIFF_OP_REPLACE);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Update operations in a diff node when the new operation is CREATE.
 *
 * @param[in,out] diff_match Node from the diff, may be replaced.
 * @param[in,out] diff Diff root node, may be updated.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @param[in] options Diff merge options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_create(struct lyd_node **diff_match, struct lyd_node **diff, enum lyd_diff_op cur_op,
        const struct lyd_node *src_diff, uint16_t options)
{
    struct lyd_node *child, *src_dup, *to_free = NULL;
    const struct lysc_node_leaf *sleaf = NULL;
    uint32_t trg_flags;
    const char *meta_name, *orig_meta_name;
    struct lyd_meta *meta, *orig_meta;
    const struct ly_ctx *ctx = LYD_CTX(*diff_match);
    LY_ERR r;

    /* create operation is valid only for data nodes */
    LY_CHECK_ERR_RET(!src_diff->schema, LOGINT(ctx), LY_EINT);

    switch (cur_op) {
    case LYD_DIFF_OP_DELETE:
        /* remember current flags */
        trg_flags = (*diff_match)->flags;

        if (lysc_is_userordered(src_diff->schema)) {
            assert((*diff_match)->schema);

            /* get anchor metadata */
            if (lysc_is_dup_inst_list((*diff_match)->schema)) {
                meta_name = "yang:position";
                orig_meta_name = "yang:orig-position";
            } else if ((*diff_match)->schema->nodetype == LYS_LIST) {
                meta_name = "yang:key";
                orig_meta_name = "yang:orig-key";
            } else {
                meta_name = "yang:value";
                orig_meta_name = "yang:orig-value";
            }
            meta = lyd_find_meta(src_diff->meta, NULL, meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_name, src_diff), LY_EINVAL);
            orig_meta = lyd_find_meta((*diff_match)->meta, NULL, orig_meta_name);
            LY_CHECK_ERR_RET(!orig_meta, LOGERR_META(ctx, orig_meta_name, *diff_match), LY_EINVAL);

            if (strcmp(lyd_get_meta_value(meta), lyd_get_meta_value(orig_meta))) {
                /* deleted + created at another position -> operation REPLACE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_REPLACE));

                /* add anchor metadata */
                LY_CHECK_RET(lyd_dup_meta_single(meta, *diff_match, NULL));

                /* previous created nodes affect the metadata so move it at the end (of the instances) */
                child = *diff_match;
                while (child->next && (child->next->schema == (*diff_match)->schema)) {
                    child = child->next;
                }
                if (child != *diff_match) {
                    LY_CHECK_RET(lyd_insert_after(child, *diff_match));
                }
            } else {
                /* deleted + created at the same position -> operation NONE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));

                /* delete anchor metadata */
                lyd_free_meta_single(orig_meta);
            }
        } else if (src_diff->schema->nodetype == LYS_LEAF) {
            if (options & LYD_DIFF_MERGE_DEFAULTS) {
                /* we are dealing with a leaf and are handling default values specially (as explicit nodes) */
                sleaf = (struct lysc_node_leaf *)src_diff->schema;
            }

            if (sleaf && sleaf->dflt && !sleaf->dflt->realtype->plugin->compare(ctx, sleaf->dflt,
                    &((struct lyd_node_term *)src_diff)->value)) {
                /* we deleted it, so a default value was in-use, and it matches the created value -> operation NONE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));
            } else if (!lyd_compare_single(*diff_match, src_diff, 0)) {
                /* deleted + created -> operation NONE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));
            } else if ((*diff_match)->schema) {
                /* we deleted it, but it was created with a different value -> operation REPLACE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_REPLACE));

                /* current value is the previous one (meta) */
                LY_CHECK_RET(lyd_new_meta(LYD_CTX(src_diff), *diff_match, NULL, "yang:orig-value",
                        lyd_get_value(*diff_match), LYD_NEW_VAL_STORE_ONLY, NULL));

                /* update the value itself */
                LY_CHECK_RET(lyd_change_term(*diff_match, lyd_get_value(src_diff)));
            } else {
                /* also operation REPLACE but we need to change an opaque node into a data node */
                LY_CHECK_RET(lyd_dup_single(src_diff, (*diff_match)->parent, LYD_DUP_NO_META | LYD_DUP_WITH_FLAGS, &src_dup));
                if (!(*diff_match)->parent) {
                    /* will always be inserted before diff_match, which is opaque */
                    LY_CHECK_RET(lyd_insert_sibling(*diff_match, src_dup, diff));
                }
                to_free = *diff_match;
                *diff_match = src_dup;

                r = lyd_new_meta(ctx, src_dup, NULL, "yang:orig-value", lyd_get_value(to_free), LYD_NEW_VAL_STORE_ONLY, NULL);
                lyd_free_tree(to_free);
                LY_CHECK_RET(r);
                LY_CHECK_RET(lyd_new_meta(ctx, src_dup, NULL, "yang:operation", lyd_diff_op2str(LYD_DIFF_OP_REPLACE), LYD_NEW_VAL_STORE_ONLY, NULL));
            }
        } else {
            /* deleted + created -> operation NONE */
            LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));
        }

        assert((*diff_match)->schema);
        if ((*diff_match)->schema->nodetype & LYD_NODE_TERM) {
            /* add orig-dflt metadata */
            LY_CHECK_RET(lyd_new_meta(LYD_CTX(src_diff), *diff_match, NULL, "yang:orig-default",
                    trg_flags & LYD_DEFAULT ? "true" : "false", LYD_NEW_VAL_STORE_ONLY, NULL));

            /* update dflt flag itself */
            (*diff_match)->flags &= ~LYD_DEFAULT;
            (*diff_match)->flags |= src_diff->flags & LYD_DEFAULT;
        }

        /* but the operation of its children should remain DELETE */
        LY_LIST_FOR(lyd_child_no_keys(*diff_match), child) {
            LY_CHECK_RET(lyd_diff_change_op(child, LYD_DIFF_OP_DELETE));
        }
        break;
    default:
        /* create and replace operations are not valid */
        LOGERR_MERGEOP(LYD_CTX(src_diff), *diff_match, cur_op, LYD_DIFF_OP_CREATE);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Update operations on a diff node when the new operation is DELETE.
 *
 * @param[in] diff_match Node from the diff.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_delete(struct lyd_node *diff_match, enum lyd_diff_op cur_op, const struct lyd_node *src_diff)
{
    struct lyd_node *child;
    struct lyd_meta *meta;
    struct lyd_attr *attr;
    const char *meta_name;
    const struct ly_ctx *ctx = LYD_CTX(diff_match);
    LY_ERR r;

    /* we can delete only exact existing nodes */
    LY_CHECK_ERR_RET(lyd_compare_single(diff_match, src_diff, 0), LOGINT(LYD_CTX(src_diff)), LY_EINT);

    switch (cur_op) {
    case LYD_DIFF_OP_CREATE:
        /* it was created, but then deleted -> set NONE operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_NONE));

        if (diff_match->schema->nodetype & LYD_NODE_TERM) {
            /* add orig-default meta because it is expected */
            LY_CHECK_RET(lyd_new_meta(LYD_CTX(src_diff), diff_match, NULL, "yang:orig-default",
                    src_diff->flags & LYD_DEFAULT ? "true" : "false", LYD_NEW_VAL_STORE_ONLY, NULL));
        }
        break;
    case LYD_DIFF_OP_REPLACE:
        /* remove the redundant metadata */
        if (lysc_is_userordered(diff_match->schema)) {
            if (lysc_is_dup_inst_list(diff_match->schema)) {
                meta_name = "position";
            } else if (diff_match->schema->nodetype == LYS_LIST) {
                meta_name = "key";
            } else {
                meta_name = "value";
            }
        } else {
            assert(diff_match->schema->nodetype == LYS_LEAF);

            /* switch value for the original one */
            meta = lyd_find_meta(diff_match->meta, NULL, "yang:orig-value");
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, "yang:orig-value", diff_match), LY_EINVAL);
            if (lyd_change_term(diff_match, lyd_get_meta_value(meta))) {
                LOGERR_UNEXPVAL(ctx, diff_match, "target diff");
                return LY_EINVAL;
            }

            /* switch default for the original one, then remove the meta */
            meta = lyd_find_meta(diff_match->meta, NULL, "yang:orig-default");
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, "yang:orig-default", diff_match), LY_EINVAL);
            diff_match->flags &= ~LYD_DEFAULT;
            if (meta->value.boolean) {
                diff_match->flags |= LYD_DEFAULT;
            }
            lyd_free_meta_single(meta);

            meta_name = "orig-value";
        }
        lyd_diff_del_meta(diff_match, meta_name);

        /* it was being changed, but should be deleted instead -> set DELETE operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_DELETE));
        break;
    case LYD_DIFF_OP_NONE:
        /* it was not modified, but should be deleted -> set DELETE operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_DELETE));
        break;
    default:
        /* delete operation is not valid */
        LOGERR_MERGEOP(LYD_CTX(diff_match), diff_match, cur_op, LYD_DIFF_OP_DELETE);
        return LY_EINVAL;
    }

    if (!lysc_is_dup_inst_list(diff_match->schema)) {
        /* keep operation without one for descendants that are yet to be merged */
        LY_LIST_FOR(lyd_child_no_keys(diff_match), child) {
            lyd_diff_find_meta(child, "operation", &meta, &attr);
            if (meta || attr) {
                continue;
            }

            if (!child->schema) {
                r = lyd_find_sibling_opaq_next(lyd_child(src_diff), LYD_NAME(child), NULL);
            } else if (child->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
                r = lyd_find_sibling_first(lyd_child(src_diff), child, NULL);
            } else {
                r = lyd_find_sibling_val(lyd_child(src_diff), child->schema, NULL, 0, NULL);
            }
            if (!r) {
                LY_CHECK_RET(lyd_diff_change_op(child, cur_op));
            } else if (r != LY_ENOTFOUND) {
                return r;
            }
        }
    } /* else key-less list, for which all the descendants act as keys */

    return LY_SUCCESS;
}

/**
 * @brief Check a node is redundant based on having any diff metadata.
 *
 * @param[in] diff Diff node to check.
 * @return 1 if the node is redundant;
 * @return 0 otherwise.
 */
static ly_bool
lyd_diff_is_redundant_meta(const struct lyd_node *diff)
{
    const struct lyd_meta *m;
    const struct lyd_node *child;

    /* diff metadata on the node */
    LY_LIST_FOR(diff->meta, m) {
        if (!strncmp(m->name, "meta-", 5)) {
            return 0;
        }
    }

    /* diff metadata on keys */
    LY_LIST_FOR(lyd_child(diff), child) {
        if (!lysc_is_key(child->schema)) {
            break;
        }

        LY_LIST_FOR(child->meta, m) {
            if (!strncmp(m->name, "meta-", 5)) {
                return 0;
            }
        }
    }

    return 1;
}

/**
 * @brief Check whether this diff node is redundant (does not change data).
 *
 * @param[in] diff Diff node.
 * @return 0 if not, non-zero if it is.
 */
static ly_bool
lyd_diff_is_redundant(struct lyd_node *diff)
{
    enum lyd_diff_op op;
    struct lyd_meta *meta, *orig_val_meta = NULL, *val_meta = NULL;
    struct lyd_node *child;
    const struct lys_module *mod;
    const char *str, *orig_meta_name, *meta_name;

    assert(diff);

    if (lysc_is_dup_inst_list(diff->schema)) {
        /* all descendants are keys */
        child = NULL;
    } else {
        child = lyd_child_no_keys(diff);
    }
    mod = ly_ctx_get_module_implemented(LYD_CTX(diff), "yang");
    assert(mod);

    /* get node operation */
    LY_CHECK_RET(lyd_diff_get_op(diff, &op, NULL), 0);

    if ((op == LYD_DIFF_OP_REPLACE) && lysc_is_userordered(diff->schema)) {
        /* get metadata names */
        if (lysc_is_dup_inst_list(diff->schema)) {
            meta_name = "position";
            orig_meta_name = "orig-position";
        } else if (diff->schema->nodetype == LYS_LIST) {
            meta_name = "key";
            orig_meta_name = "orig-key";
        } else {
            meta_name = "value";
            orig_meta_name = "orig-value";
        }

        /* check for redundant move */
        orig_val_meta = lyd_find_meta(diff->meta, mod, orig_meta_name);
        val_meta = lyd_find_meta(diff->meta, mod, meta_name);
        assert(orig_val_meta && val_meta);

        if (!lyd_compare_meta(orig_val_meta, val_meta)) {
            /* there is actually no move */
            lyd_free_meta_single(orig_val_meta);
            lyd_free_meta_single(val_meta);
            if (child) {
                /* change operation to NONE, we have siblings */
                lyd_diff_change_op(diff, LYD_DIFF_OP_NONE);
                return 0;
            }

            /* redundant node, BUT !!
             * In diff the move operation is always converted to be INSERT_AFTER, which is fine
             * because the data that this is applied on should not change for the diff lifetime.
             * However, when we are merging 2 diffs, this conversion is actually lossy because
             * if the data change, the move operation can also change its meaning. In this specific
             * case the move operation will be lost. But it can be considered a feature, it is not supported.
             */
            return 1;
        }
    } else if (op == LYD_DIFF_OP_NONE) {
        if (!diff->schema) {
            /* opaque node with none must be redundant */
            return 1;
        }

        /* check for diff metadata */
        if (!lyd_diff_is_redundant_meta(diff)) {
            return 0;
        }

        if (diff->schema->nodetype & LYD_NODE_TERM) {
            /* check whether at least the default flags are different */
            meta = lyd_find_meta(diff->meta, mod, "orig-default");
            assert(meta);
            str = lyd_get_meta_value(meta);

            /* if previous and current dflt flags are the same, this node is redundant */
            if ((!strcmp(str, "true") && (diff->flags & LYD_DEFAULT)) || (!strcmp(str, "false") && !(diff->flags & LYD_DEFAULT))) {
                return 1;
            }
            return 0;
        }
    }

    if (!child && (op == LYD_DIFF_OP_NONE)) {
        return 1;
    }

    return 0;
}

/**
 * @brief Merge all diff metadata found on a source diff node.
 *
 * @param[in] src_diff Source node.
 * @param[in,out] trg_diff Target node to update.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_metadata(const struct lyd_node *src_diff, struct lyd_node *trg_diff)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lys_module *mod;
    struct lyd_meta *m, **src_meta_replace = NULL, **src_meta_orig = NULL;
    struct lyd_meta **trg_meta_replace = NULL, **trg_meta_orig = NULL, *m1, *m2;
    uint32_t i, j, src_mr_count = 0, src_mo_count = 0, trg_mr_count = 0, trg_mo_count = 0;

    assert(src_diff->schema == trg_diff->schema);

    mod = ly_ctx_get_module_implemented(LYD_CTX(src_diff), "yang");
    assert(mod);

    /* collect all the metadata so we can safely modify them */
    LY_LIST_FOR(trg_diff->meta, m) {
        if (m->annotation->module != mod) {
            continue;
        }

        if (!strcmp(m->name, "meta-replace")) {
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &trg_meta_replace, &trg_mr_count), cleanup);
        } else if (!strcmp(m->name, "meta-orig")) {
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &trg_meta_orig, &trg_mo_count), cleanup);
        }
    }

    /* make sure meta_replace and meta_orig arrays are aligned */
    rc = lyd_diff_metadata_replace_orig_align(trg_meta_replace, trg_mr_count, trg_meta_orig, trg_mo_count);
    LY_CHECK_GOTO(rc, cleanup);

    LY_LIST_FOR(src_diff->meta, m) {
        if (m->annotation->module != mod) {
            continue;
        }

        if (!strcmp(m->name, "meta-create")) {
            /* find relevant metadata in the target */
            rc = lyd_diff_metadata_find(trg_diff->meta, "yang:meta-delete", lyd_get_meta_value(m), 0, &m1);
            LY_CHECK_GOTO(rc, cleanup);
            m2 = NULL;
            for (i = 0; i < trg_mo_count; ++i) {
                if (lyd_get_meta_value(m) == lyd_get_meta_value(trg_meta_orig[i])) {
                    m2 = trg_meta_orig[i];
                    break;
                }
            }

            if (m1) {
                /* create + delete -> no change */
                lyd_free_meta_single(m1);
            } else if (m2) {
                /* create + replace -> create with updated value */
                rc = lyd_new_meta(NULL, trg_diff, mod, "meta-create", lyd_get_meta_value(trg_meta_replace[i]), 0, NULL);
                LY_CHECK_GOTO(rc, cleanup);

                /* remove meta-replace and meta-orig */
                lyd_free_meta_single(trg_meta_replace[i]);
                --trg_mr_count;
                if (i < trg_mr_count) {
                    memmove(&trg_meta_replace[i], &trg_meta_replace[i + 1], (trg_mr_count - i) * sizeof *trg_meta_replace);
                }

                lyd_free_meta_single(trg_meta_orig[i]);
                --trg_mo_count;
                if (i < trg_mo_count) {
                    memmove(&trg_meta_orig[i], &trg_meta_orig[i + 1], (trg_mo_count - i) * sizeof *trg_meta_orig);
                }
            } else {
                /* copy to the target */
                LY_CHECK_GOTO(rc = lyd_dup_meta_single(m, trg_diff, NULL), cleanup);
            }
        } else if (!strcmp(m->name, "meta-delete")) {
            /* find relevant metadata in the target */
            rc = lyd_diff_metadata_find(trg_diff->meta, "yang:meta-create", lyd_get_meta_value(m), 0, &m1);
            LY_CHECK_GOTO(rc, cleanup);

            if (m1) {
                /* delete + create -> no change */
                lyd_free_meta_single(m1);
            } else {
                /* copy to the target */
                LY_CHECK_GOTO(rc = lyd_dup_meta_single(m, trg_diff, NULL), cleanup);
            }
        } else if (!strcmp(m->name, "meta-replace")) {
            /* collect all meta-replace metadata */
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &src_meta_replace, &src_mr_count), cleanup);
        } else if (!strcmp(m->name, "meta-orig")) {
            /* collect all meta-orig metadata */
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &src_meta_orig, &src_mo_count), cleanup);
        }
    }

    /* make sure meta_replace and meta_orig arrays are aligned */
    rc = lyd_diff_metadata_replace_orig_align(src_meta_replace, src_mr_count, src_meta_orig, src_mo_count);
    LY_CHECK_GOTO(rc, cleanup);

    for (i = 0; i < src_mr_count; ++i) {
        /* find relevant metadata in the target */
        rc = lyd_diff_metadata_find(trg_diff->meta, "yang:meta-delete", lyd_get_meta_value(src_meta_replace[i]), 0, &m1);
        LY_CHECK_GOTO(rc, cleanup);
        m2 = NULL;
        for (j = 0; j < trg_mo_count; ++j) {
            if (lyd_get_meta_value(trg_meta_orig[j]) == lyd_get_meta_value(src_meta_replace[i])) {
                m2 = trg_meta_orig[j];
                break;
            }
        }

        if (m1) {
            /* replace + delete -> delete with updated value */
            LY_CHECK_GOTO(rc = lyd_change_meta(m1, lyd_get_meta_value(src_meta_orig[i])), cleanup);
        } else if (m2) {
            /* replace + replace -> replace (orig) with updated value */
            LY_CHECK_GOTO(rc = lyd_change_meta(m2, lyd_get_meta_value(src_meta_orig[i])), cleanup);
        } else {
            /* copy to the target */
            LY_CHECK_GOTO(rc = lyd_dup_meta_single(src_meta_replace[i], trg_diff, NULL), cleanup);
            LY_CHECK_GOTO(rc = lyd_dup_meta_single(src_meta_orig[i], trg_diff, NULL), cleanup);
        }
    }

cleanup:
    free(src_meta_replace);
    free(src_meta_orig);
    free(trg_meta_replace);
    free(trg_meta_orig);
    return rc;
}

/**
 * @brief Merge all diff metadata found on a source diff subtree, recursively.
 *
 * @param[in] src_diff Source subtree.
 * @param[in,out] trg_diff Target subtree to update.
 * @param[in] keys_only Whether to process the node with keys only or with all the descendants.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_metadata_r(const struct lyd_node *src_diff, struct lyd_node *trg_diff, ly_bool keys_only)
{
    const struct lyd_node *src_child;
    struct lyd_node *trg_child;

    /* merge metadata on the node itself */
    LY_CHECK_RET(lyd_diff_merge_metadata(src_diff, trg_diff));

    /* merge descendants recursively */
    trg_child = lyd_child(trg_diff);
    LY_LIST_FOR(lyd_child(src_diff), src_child) {
        if (keys_only && !lysc_is_key(src_child->schema)) {
            break;
        }

        LY_CHECK_RET(lyd_diff_merge_metadata(src_child, trg_child));

        trg_child = trg_child->next;
    }

    return LY_SUCCESS;
}

/**
 * @brief Merge sysrepo diff subtree with another diff, recursively.
 *
 * @param[in] src_diff Source diff node.
 * @param[in] diff_parent Current sysrepo diff parent.
 * @param[in] diff_cb Optional diff callback.
 * @param[in] cb_data User data for @p diff_cb.
 * @param[in,out] dup_inst Duplicate instance cache for all @p src_diff siblings.
 * @param[in] options Diff merge options.
 * @param[in,out] diff Diff root node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_r(const struct lyd_node *src_diff, struct lyd_node *diff_parent, lyd_diff_cb diff_cb, void *cb_data,
        struct ly_ht **dup_inst, uint16_t options, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *child, *diff_node = NULL;
    enum lyd_diff_op src_op, cur_op;
    struct ly_ht *child_dup_inst = NULL;

    /* get source node operation */
    LY_CHECK_RET(lyd_diff_get_op(src_diff, &src_op, NULL));

    /* find an equal node in the current diff */
    LY_CHECK_RET(lyd_diff_find_match(diff_parent ? lyd_child_no_keys(diff_parent) : *diff, src_diff, 1, dup_inst, &diff_node));

    if (diff_node) {
        /* get target (current) operation */
        LY_CHECK_RET(lyd_diff_get_op(diff_node, &cur_op, NULL));

        /* merge operations */
        switch (src_op) {
        case LYD_DIFF_OP_REPLACE:
            ret = lyd_diff_merge_replace(diff_node, cur_op, src_diff);
            break;
        case LYD_DIFF_OP_CREATE:
            if ((cur_op == LYD_DIFF_OP_CREATE) && lysc_is_dup_inst_list(diff_node->schema)) {
                /* special case of creating duplicate (leaf-)list instances */
                goto add_diff;
            }

            ret = lyd_diff_merge_create(&diff_node, diff, cur_op, src_diff, options);
            break;
        case LYD_DIFF_OP_DELETE:
            ret = lyd_diff_merge_delete(diff_node, cur_op, src_diff);
            break;
        case LYD_DIFF_OP_NONE:
            /* key-less list can never have "none" operation since all its descendants are acting as "keys" */
            assert((src_diff->schema->nodetype != LYS_LIST) || !lysc_is_dup_inst_list(src_diff->schema));
            ret = lyd_diff_merge_none(diff_node, cur_op, src_diff);
            break;
        default:
            LOGINT_RET(LYD_CTX(src_diff));
        }
        if (ret) {
            LOGERR(LYD_CTX(src_diff), LY_EOTHER, "Merging operation \"%s\" failed.", lyd_diff_op2str(src_op));
            return ret;
        }

        /* merge any metadata */
        LY_CHECK_RET(lyd_diff_merge_metadata_r(src_diff, diff_node, lysc_is_dup_inst_list(src_diff->schema) ? 0 : 1));

        if (diff_cb) {
            /* call callback */
            LY_CHECK_RET(diff_cb(src_diff, diff_node, cb_data));
        }

        /* update diff parent */
        diff_parent = diff_node;

        /* for diff purposes, all key-less list descendants actually act as keys (identifying the same instances),
         * so there is nothing to merge for these "keys" */
        if (!lysc_is_dup_inst_list(src_diff->schema)) {
            /* merge src_diff recursively */
            LY_LIST_FOR(lyd_child_no_keys(src_diff), child) {
                ret = lyd_diff_merge_r(child, diff_parent, diff_cb, cb_data, &child_dup_inst, options, diff);
                if (ret) {
                    break;
                }
            }
            lyd_dup_inst_free(child_dup_inst);
            LY_CHECK_RET(ret);
        }
    } else {
add_diff:
        /* add new diff node with all descendants */
        LY_CHECK_RET(lyd_dup_single(src_diff, (struct lyd_node_inner *)diff_parent,
                LYD_DUP_RECURSIVE | LYD_DUP_WITH_FLAGS | LYD_DUP_NO_LYDS, &diff_node));

        /* insert node into diff if not already */
        if (!diff_parent) {
            lyd_diff_insert_sibling(*diff, diff_node, diff);
        }

        /* update operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_node, src_op));

        if (diff_cb) {
            /* call callback with no source diff node since it was duplicated and just added */
            LY_CHECK_RET(diff_cb(NULL, diff_node, cb_data));
        }

        /* update diff parent */
        diff_parent = diff_node;
    }

    /* remove any redundant nodes */
    if (lyd_diff_is_redundant(diff_parent)) {
        if (diff_parent == *diff) {
            *diff = (*diff)->next;
        }
        lyd_free_tree(diff_parent);
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_merge_module(struct lyd_node **diff, const struct lyd_node *src_diff, const struct lys_module *mod,
        lyd_diff_cb diff_cb, void *cb_data, uint16_t options)
{
    const struct lyd_node *src_root;
    struct ly_ht *dup_inst = NULL;
    LY_ERR ret = LY_SUCCESS;

    LY_LIST_FOR(src_diff, src_root) {
        if (mod && (lyd_owner_module(src_root) != mod)) {
            /* skip data nodes from different modules */
            continue;
        }

        /* apply relevant nodes from the diff datatree */
        LY_CHECK_GOTO(ret = lyd_diff_merge_r(src_root, NULL, diff_cb, cb_data, &dup_inst, options, diff), cleanup);
    }

cleanup:
    lyd_dup_inst_free(dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_merge_tree(struct lyd_node **diff_first, struct lyd_node *diff_parent, const struct lyd_node *src_sibling,
        lyd_diff_cb diff_cb, void *cb_data, uint16_t options)
{
    LY_ERR ret;
    struct ly_ht *dup_inst = NULL;

    if (!src_sibling) {
        return LY_SUCCESS;
    }

    ret = lyd_diff_merge_r(src_sibling, diff_parent, diff_cb, cb_data, &dup_inst, options, diff_first);
    lyd_dup_inst_free(dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_merge_all(struct lyd_node **diff, const struct lyd_node *src_diff, uint16_t options)
{
    return lyd_diff_merge_module(diff, src_diff, NULL, NULL, NULL, options);
}

/**
 * @brief Reverse diff value meta by switching it for the node value.
 *
 * @param[in] node Parent meta node.
 * @param[in] mod Meta module.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_reverse_value(struct lyd_node *node, const struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *meta;
    const char *val1 = NULL;
    char *val2;
    uint32_t flags;

    assert(node->schema->nodetype & (LYS_LEAF | LYS_ANYDATA));

    meta = lyd_find_meta(node->meta, mod, "orig-value");
    LY_CHECK_ERR_RET(!meta, LOGERR_META(LYD_CTX(node), "orig-value", node), LY_EINVAL);

    /* orig-value */
    val1 = lyd_get_meta_value(meta);

    /* current value */
    if (node->schema->nodetype == LYS_LEAF) {
        val2 = strdup(lyd_get_value(node));
    } else {
        LY_CHECK_RET(lyd_any_value_str(node, &val2));
    }

    /* switch values, keep default flag */
    flags = node->flags;
    if (node->schema->nodetype == LYS_LEAF) {
        LY_CHECK_GOTO(ret = lyd_change_term(node, val1), cleanup);
    } else {
        union lyd_any_value anyval = {.str = val1};

        LY_CHECK_GOTO(ret = lyd_any_copy_value(node, &anyval, LYD_ANYDATA_STRING), cleanup);
    }
    node->flags = flags;
    LY_CHECK_GOTO(ret = lyd_change_meta(meta, val2), cleanup);

cleanup:
    free(val2);
    return ret;
}

/**
 * @brief Reverse diff default meta by switching it for the node dflt flag.
 *
 * @param[in] node Parent meta node.
 * @param[in] mod Meta module.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_reverse_default(struct lyd_node *node, const struct lys_module *mod)
{
    struct lyd_meta *meta;
    uint32_t flag1, flag2;

    meta = lyd_find_meta(node->meta, mod, "orig-default");
    LY_CHECK_ERR_RET(!meta, LOGINT(mod->ctx), LY_EINT);

    /* orig-default */
    if (meta->value.boolean) {
        flag1 = LYD_DEFAULT;
    } else {
        flag1 = 0;
    }

    /* current default */
    flag2 = node->flags & LYD_DEFAULT;

    if (flag1 == flag2) {
        /* no default state change so nothing to reverse */
        return LY_SUCCESS;
    }

    /* switch defaults */
    node->flags &= ~LYD_DEFAULT;
    node->flags |= flag1;
    LY_CHECK_RET(lyd_change_meta(meta, flag2 ? "true" : "false"));

    return LY_SUCCESS;
}

/**
 * @brief Reverse diff meta by switching their values.
 *
 * @param[in] node Parent meta node.
 * @param[in] mod Meta module.
 * @param[in] name1 First meta name.
 * @param[in] name2 Second meta name.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_reverse_meta(struct lyd_node *node, const struct lys_module *mod, const char *name1, const char *name2)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *meta1, *meta2;
    const char *val1 = NULL;
    char *val2 = NULL;

    meta1 = lyd_find_meta(node->meta, mod, name1);
    LY_CHECK_ERR_RET(!meta1, LOGERR_META(LYD_CTX(node), name1, node), LY_EINVAL);

    meta2 = lyd_find_meta(node->meta, mod, name2);
    LY_CHECK_ERR_RET(!meta2, LOGERR_META(LYD_CTX(node), name2, node), LY_EINVAL);

    /* value1 */
    val1 = lyd_get_meta_value(meta1);

    /* value2 */
    val2 = strdup(lyd_get_meta_value(meta2));

    /* switch values */
    LY_CHECK_GOTO(ret = lyd_change_meta(meta1, val2), cleanup);
    LY_CHECK_GOTO(ret = lyd_change_meta(meta2, val1), cleanup);

cleanup:
    free(val2);
    return ret;
}

/**
 * @brief Remove specific operation from all the nodes in a subtree.
 *
 * @param[in] diff Diff subtree to process.
 * @param[in] op Only expected operation.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_reverse_remove_op_r(struct lyd_node *diff, enum lyd_diff_op op)
{
    struct lyd_node *elem;
    struct lyd_meta *meta;

    LYD_TREE_DFS_BEGIN(diff, elem) {
        meta = lyd_find_meta(elem->meta, NULL, "yang:operation");
        if (meta) {
            LY_CHECK_ERR_RET(lyd_diff_str2op(lyd_get_meta_value(meta)) != op, LOGINT(LYD_CTX(diff)), LY_EINT);
            lyd_free_meta_single(meta);
        }

        LYD_TREE_DFS_END(diff, elem);
    }

    return LY_SUCCESS;
}

/**
 * @brief Reverse all metadata diff meta.
 *
 * @param[in,out] diff Diff node with metadata diff to reverse.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_reverse_metadata_diff(struct lyd_node *node)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_meta *m, **meta_create = NULL, **meta_delete = NULL, **meta_replace = NULL, **meta_orig = NULL;
    uint32_t i, mc_count = 0, md_count = 0, mr_count = 0, mo_count = 0;
    const struct lys_module *mod;
    const char *val1;

    mod = ly_ctx_get_module_implemented(LYD_CTX(node), "yang");
    assert(mod);

    /* collect all the metadata so we can safely modify them */
    LY_LIST_FOR(node->meta, m) {
        if (m->annotation->module != mod) {
            continue;
        }

        if (!strcmp(m->name, "meta-create")) {
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &meta_create, &mc_count), cleanup);
        } else if (!strcmp(m->name, "meta-delete")) {
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &meta_delete, &md_count), cleanup);
        } else if (!strcmp(m->name, "meta-replace")) {
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &meta_replace, &mr_count), cleanup);
        } else if (!strcmp(m->name, "meta-orig")) {
            LY_CHECK_GOTO(rc = lyd_diff_meta_store(m, &meta_orig, &mo_count), cleanup);
        }
    }

    /* make sure meta_replace and meta_orig arrays are aligned */
    LY_CHECK_GOTO(rc = lyd_diff_metadata_replace_orig_align(meta_replace, mr_count, meta_orig, mo_count), cleanup);

    /* reverse all the meta-create metadata */
    for (i = 0; i < mc_count; ++i) {
        rc = lyd_new_meta(NULL, node, mod, "meta-delete", lyd_get_meta_value(meta_create[i]), 0, NULL);
        LY_CHECK_GOTO(rc, cleanup);
        lyd_free_meta_single(meta_create[i]);
    }

    /* reverse all the meta-replace and meta-orig metadata */
    for (i = 0; i < mr_count; ++i) {
        LY_CHECK_GOTO(rc = lydict_dup(LYD_CTX(node), lyd_get_meta_value(meta_replace[i]), &val1), cleanup);

        rc = lyd_change_meta(meta_replace[i], lyd_get_meta_value(meta_orig[i]));
        if (rc) {
            lydict_remove(LYD_CTX(node), val1);
            goto cleanup;
        }

        rc = lyd_change_meta(meta_orig[i], val1);
        lydict_remove(LYD_CTX(node), val1);
        LY_CHECK_GOTO(rc, cleanup);
    }

    /* reverse all the meta-delete metadata */
    for (i = 0; i < md_count; ++i) {
        rc = lyd_new_meta(NULL, node, mod, "meta-create", lyd_get_meta_value(meta_delete[i]), 0, NULL);
        LY_CHECK_GOTO(rc, cleanup);
        lyd_free_meta_single(meta_delete[i]);
    }

cleanup:
    free(meta_create);
    free(meta_delete);
    free(meta_replace);
    free(meta_orig);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_reverse_all(const struct lyd_node *src_diff, struct lyd_node **diff)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lys_module *mod = NULL;
    struct lyd_node *root, *elem, *iter;
    enum lyd_diff_op op;

    LY_CHECK_ARG_RET(NULL, diff, LY_EINVAL);

    if (!src_diff) {
        *diff = NULL;
        return LY_SUCCESS;
    }

    /* duplicate diff */
    LY_CHECK_RET(lyd_dup_siblings(src_diff, NULL, LYD_DUP_RECURSIVE | LYD_DUP_NO_LYDS, diff));

    LY_LIST_FOR(*diff, root) {
        LYD_TREE_DFS_BEGIN(root, elem) {
            /* skip all keys */
            if (!lysc_is_key(elem->schema)) {
                /* find module with metadata needed for later in the current node context */
                if (!mod || (mod->ctx != LYD_CTX(elem))) {
                    mod = ly_ctx_get_module_latest(LYD_CTX(elem), "yang");
                    LY_CHECK_ERR_GOTO(!mod, LOGINT(LYD_CTX(src_diff)); rc = LY_EINT, cleanup);
                }

                /* find operation attribute, if any */
                LY_CHECK_GOTO(rc = lyd_diff_get_op(elem, &op, NULL), cleanup);

                switch (op) {
                case LYD_DIFF_OP_CREATE:
                    /* reverse create to delete */
                    LY_CHECK_GOTO(rc = lyd_diff_change_op(elem, LYD_DIFF_OP_DELETE), cleanup);

                    /* check all the children for the same operation, nothing else is expected */
                    LY_LIST_FOR(lyd_child(elem), iter) {
                        lyd_diff_reverse_remove_op_r(iter, LYD_DIFF_OP_CREATE);
                    }

                    LYD_TREE_DFS_continue = 1;
                    break;
                case LYD_DIFF_OP_DELETE:
                    /* reverse delete to create */
                    LY_CHECK_GOTO(rc = lyd_diff_change_op(elem, LYD_DIFF_OP_CREATE), cleanup);

                    /* check all the children for the same operation, nothing else is expected */
                    LY_LIST_FOR(lyd_child(elem), iter) {
                        lyd_diff_reverse_remove_op_r(iter, LYD_DIFF_OP_DELETE);
                    }

                    LYD_TREE_DFS_continue = 1;
                    break;
                case LYD_DIFF_OP_REPLACE:
                    switch (elem->schema->nodetype) {
                    case LYS_LEAF:
                        /* leaf value change */
                        LY_CHECK_GOTO(rc = lyd_diff_reverse_value(elem, mod), cleanup);
                        LY_CHECK_GOTO(rc = lyd_diff_reverse_default(elem, mod), cleanup);
                        break;
                    case LYS_ANYXML:
                    case LYS_ANYDATA:
                        /* any value change */
                        LY_CHECK_GOTO(rc = lyd_diff_reverse_value(elem, mod), cleanup);
                        break;
                    case LYS_LEAFLIST:
                        /* leaf-list move */
                        LY_CHECK_GOTO(rc = lyd_diff_reverse_default(elem, mod), cleanup);
                        if (lysc_is_dup_inst_list(elem->schema)) {
                            LY_CHECK_GOTO(rc = lyd_diff_reverse_meta(elem, mod, "orig-position", "position"), cleanup);
                        } else {
                            LY_CHECK_GOTO(rc = lyd_diff_reverse_meta(elem, mod, "orig-value", "value"), cleanup);
                        }
                        break;
                    case LYS_LIST:
                        /* list move */
                        if (lysc_is_dup_inst_list(elem->schema)) {
                            LY_CHECK_GOTO(rc = lyd_diff_reverse_meta(elem, mod, "orig-position", "position"), cleanup);
                        } else {
                            LY_CHECK_GOTO(rc = lyd_diff_reverse_meta(elem, mod, "orig-key", "key"), cleanup);
                        }
                        break;
                    default:
                        LOGINT(LYD_CTX(src_diff));
                        rc = LY_EINT;
                        goto cleanup;
                    }
                    break;
                case LYD_DIFF_OP_NONE:
                    switch (elem->schema->nodetype) {
                    case LYS_LEAF:
                    case LYS_LEAFLIST:
                        /* default flag change */
                        LY_CHECK_GOTO(rc = lyd_diff_reverse_default(elem, mod), cleanup);
                        break;
                    default:
                        /* nothing to do */
                        break;
                    }
                    break;
                }
            }

            /* reverse any metadata diff */
            LY_CHECK_GOTO(rc = lyd_diff_reverse_metadata_diff(elem), cleanup);

            LYD_TREE_DFS_END(root, elem);
        }
    }

cleanup:
    if (rc) {
        lyd_free_siblings(*diff);
        *diff = NULL;
    }
    return rc;
}
