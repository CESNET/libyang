/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"

API void
lyd_trees_free(const struct lyd_node **trees, int free_data)
{
    if (!trees) {
        return;
    }

    if (free_data) {
        unsigned int u;
        LY_ARRAY_FOR(trees, u) {
            lyd_free_all((struct lyd_node *)trees[u]);
        }
    }
    LY_ARRAY_FREE(trees);
}

static const struct lyd_node *
lyd_trees_getstart(const struct lyd_node *tree)
{
    if (!tree) {
        return NULL;
    }
    while (tree->prev->next) {
        tree = tree->prev;
    }
    return tree;
}

API const struct lyd_node **
lyd_trees_new(size_t count, const struct lyd_node *tree, ...)
{
    LY_ERR ret;
    const struct lyd_node **trees = NULL;
    va_list ap;

    LY_CHECK_ARG_RET(NULL, tree, count > 0, NULL);

    va_start(ap, tree);

    LY_ARRAY_CREATE_GOTO(tree->schema->module->ctx, trees, count, ret, error);
    /* first, mandatory, tree to insert */
    trees[0] = lyd_trees_getstart(tree);
    LY_ARRAY_INCREMENT(trees);

    /* variable arguments */
    for (unsigned int u = 1; u < count; ++u) {
        trees[u] = lyd_trees_getstart(va_arg(ap, const struct lyd_node *));
        LY_ARRAY_INCREMENT(trees);
    }

    va_end(ap);
    return trees;

error:
    (void)ret; /* unused */
    lyd_trees_free(trees, 1);
    va_end(ap);
    return NULL;
}

API const struct lyd_node **
lyd_trees_add(const struct lyd_node **trees, const struct lyd_node *tree)
{
    const struct lyd_node **t = NULL;

    LY_CHECK_ARG_RET(NULL, tree, trees, trees);

    LY_ARRAY_NEW_RET(tree->schema->module->ctx, trees, t, NULL);
    *t = lyd_trees_getstart(tree);

    return trees;
}

static int
cmp_str(const char *refstr, const char *str, size_t str_len)
{

    if (str_len) {
        int r = strncmp(refstr, str, str_len);
        if (!r && !refstr[str_len]) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return strcmp(refstr, str);
    }
}

API const struct lyd_node *
lyd_search(const struct lyd_node *first, const struct lys_module *module,
          const char *name, size_t name_len, uint16_t nodetype, const char *value, size_t value_len)
{
    const struct lyd_node *node = NULL;
    const struct lysc_node *snode;

    LY_CHECK_ARG_RET(NULL, module, name, NULL);
    if (!nodetype) {
        nodetype = 0xffff;
    }

    LY_LIST_FOR(first, node) {
        snode = node->schema;
        if (!(snode->nodetype & nodetype)) {
            continue;
        }
        if (snode->module != module) {
            continue;
        }

        if (cmp_str(snode->name, name, name_len)) {
            continue;
        }

        if (value) {
            if (snode->nodetype == LYS_LIST) {
                /* TODO handle value as keys of the list instance */
            } else if (snode->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                if (cmp_str(((struct lyd_node_term*)node)->value.canonized, value, value_len)) {
                    continue;
                }
            } else {
                continue;
            }
        }

        /* all criteria passed */
        return node;
    }
    return NULL;
}

LY_ERR
lyd_value_parse(struct lyd_node_term *node, const char *value, size_t value_len, int dynamic, int second,
                ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format, const struct lyd_node **trees)
{
    LY_ERR ret = LY_SUCCESS, rc;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    int options = LY_TYPE_OPTS_STORE | (second ? LY_TYPE_OPTS_SECOND_CALL : 0) |
            (dynamic ? LY_TYPE_OPTS_DYNAMIC : 0) | (trees ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);
    assert(node);

    ctx = node->schema->module->ctx;

    type = ((struct lysc_node_leaf*)node->schema)->type;
    if (!second) {
        node->value.realtype = type;
    }
    rc = type->plugin->store(ctx, type, value, value_len, options, get_prefix, parser, format,
                             trees ? (void*)node : (void*)node->schema, trees,
                             &node->value, NULL, &err);
    if (rc == LY_EINCOMPLETE) {
        ret = rc;
        /* continue with storing, just remember what to return if storing is ok */
    } else if (rc) {
        ret = rc;
        if (err) {
            ly_err_print(err);
            LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
            ly_err_free(err);
        }
        goto error;
    }

error:
    return ret;
}

API LY_ERR
lys_value_validate(struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len,
                   ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct lysc_type *type;

    LY_CHECK_ARG_RET(ctx, node, value, LY_EINVAL);

    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGARG(ctx, node);
        return LY_EINVAL;
    }

    type = ((struct lysc_node_leaf*)node)->type;
    /* just validate, no storing of enything */
    rc = type->plugin->store(ctx ? ctx : node->module->ctx, type, value, value_len, LY_TYPE_OPTS_INCOMPLETE_DATA,
                             get_prefix, get_prefix_data, format, node, NULL, NULL, NULL, &err);
    if (rc == LY_EINCOMPLETE) {
        /* actually success since we do not provide the context tree and call validation with
         * LY_TYPE_OPTS_INCOMPLETE_DATA */
        rc = LY_SUCCESS;
    } else if (rc && err) {
        if (ctx) {
            /* log only in case the ctx was provided as input parameter */
            ly_err_print(err);
            LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
        }
        ly_err_free(err);
    }

    return rc;
}

API LY_ERR
lyd_value_validate(struct ly_ctx *ctx, const struct lyd_node_term *node, const char *value, size_t value_len,
                   ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format, const struct lyd_node **trees)
{
    LY_ERR rc;
    struct ly_err_item *err = NULL;
    struct lysc_type *type;
    int options = (trees ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);

    LY_CHECK_ARG_RET(ctx, node, value, LY_EINVAL);

    type = ((struct lysc_node_leaf*)node->schema)->type;
    rc = type->plugin->store(ctx ? ctx : node->schema->module->ctx, type, value, value_len, options,
                             get_prefix, get_prefix_data, format, trees ? (void*)node : (void*)node->schema, trees,
                             NULL, NULL, &err);
    if (rc == LY_EINCOMPLETE) {
        return rc;
    } else if (rc) {
        if (err) {
            if (ctx) {
                ly_err_print(err);
                LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
            }
            ly_err_free(err);
        }
        return rc;
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_value_compare(const struct lyd_node_term *node, const char *value, size_t value_len,
                  ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format,  const struct lyd_node **trees)
{
    LY_ERR ret = LY_SUCCESS, rc;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    struct lyd_value data = {0};
    int options = LY_TYPE_OPTS_STORE | (trees ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);

    LY_CHECK_ARG_RET(node ? node->schema->module->ctx : NULL, node, value, LY_EINVAL);

    ctx = node->schema->module->ctx;
    type = ((struct lysc_node_leaf*)node->schema)->type;
    rc = type->plugin->store(ctx, type, value, value_len, options, get_prefix, get_prefix_data, format, (struct lyd_node*)node,
                             trees, &data, NULL, &err);
    if (rc == LY_EINCOMPLETE) {
        ret = rc;
        /* continue with comparing, just remember what to return if storing is ok */
    } else if (rc) {
        /* value to compare is invalid */
        ret = LY_EINVAL;
        if (err) {
            ly_err_free(err);
        }
        goto cleanup;
    }

    /* compare data */
    if (type->plugin->compare(&node->value, &data)) {
        /* do not assign it directly from the compare callback to keep possible LY_EINCOMPLETE from validation */
        ret = LY_EVALID;
    }

cleanup:
    type->plugin->free(ctx, &data);

    return ret;
}

API struct lyd_node *
lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, const struct lyd_node **trees)
{
    struct lyd_node *result = NULL;
#if 0
    const char *yang_data_name = NULL;
#endif

    LY_CHECK_ARG_RET(ctx, ctx, NULL);

    if (lyd_parse_options_check(ctx, options, __func__)) {
        return NULL;
    }

    if (options & LYD_OPT_RPCREPLY) {
        /* first item in trees is mandatory - the RPC/action request */
        LY_CHECK_ARG_RET(ctx, trees, LY_ARRAY_SIZE(trees) >= 1, NULL);
        if (!trees[0] || trees[0]->parent || !(trees[0]->schema->nodetype & (LYS_ACTION | LYS_LIST | LYS_CONTAINER))) {
            LOGERR(ctx, LY_EINVAL, "Data parser invalid argument trees - the first item in the array must be the RPC/action request when parsing %s.",
                   lyd_parse_options_type2str(options));
            return NULL;
        }
    }

#if 0
    if (options & LYD_OPT_DATA_TEMPLATE) {
        yang_data_name = va_arg(ap, const char *);
    }
#endif

    if (!format) {
        /* TODO try to detect format from the content */
    }

    switch (format) {
    case LYD_XML:
        lyd_parse_xml(ctx, data, options, trees, &result);
        break;
#if 0
    case LYD_JSON:
        lyd_parse_json(ctx, data, options, trees, &result);
        break;
    case LYD_LYB:
        lyd_parse_lyb(ctx, data, options, trees, &result);
        break;
#endif
    case LYD_UNKNOWN:
        LOGINT(ctx);
        break;
    }

    return result;
}

API struct lyd_node *
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, const struct lyd_node **trees)
{
    struct lyd_node *result;
    size_t length;
    char *addr;

    LY_CHECK_ARG_RET(ctx, ctx, NULL);
    if (fd < 0) {
        LOGARG(ctx, fd);
        return NULL;
    }

    LY_CHECK_RET(ly_mmap(ctx, fd, &length, (void **)&addr), NULL);
    result = lyd_parse_mem(ctx, addr ? addr : "", format, options, trees);
    if (addr) {
        ly_munmap(addr, length);
    }

    return result;
}

API struct lyd_node *
lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, const struct lyd_node **trees)
{
    int fd;
    struct lyd_node *result;
    size_t len;

    LY_CHECK_ARG_RET(ctx, ctx, path, NULL);

    fd = open(path, O_RDONLY);
    LY_CHECK_ERR_RET(fd == -1, LOGERR(ctx, LY_ESYS, "Opening file \"%s\" failed (%s).", path, strerror(errno)), NULL);

    if (!format) {
        /* unknown format - try to detect it from filename's suffix */
        len = strlen(path);

        /* ignore trailing whitespaces */
        for (; len > 0 && isspace(path[len - 1]); len--);

        if (len >= 5 && !strncmp(&path[len - 4], ".xml", 4)) {
            format = LYD_XML;
#if 0
        } else if (len >= 6 && !strncmp(&path[len - 5], ".json", 5)) {
            format = LYD_JSON;
        } else if (len >= 5 && !strncmp(&path[len - 4], ".lyb", 4)) {
            format = LYD_LYB;
#endif
        } /* else still unknown, try later to detect it from the content */
    }

    result = lyd_parse_fd(ctx, fd, format, options, trees);
    close(fd);

    return result;
}

API const struct lyd_node_term *
lyd_target(struct lyd_value_path *path, const struct lyd_node **trees)
{
    unsigned int u, v, x;
    const struct lyd_node *node = NULL, *parent = NULL, *start_search;
    uint64_t pos = 1;

    LY_CHECK_ARG_RET(NULL, path, trees, NULL);

    LY_ARRAY_FOR(path, u) {
        if (parent) {
            start_search = lyd_node_children(parent);
search_inner:
            node = lyd_search(start_search, path[u].node->module, path[u].node->name, strlen(path[u].node->name), path[u].node->nodetype, NULL, 0);
        } else {
            LY_ARRAY_FOR(trees, v) {
                start_search = trees[v];
search_toplevel:
                /* WARNING! to use search_toplevel label correctly, variable v must be preserved and not changed! */
                node = lyd_search(start_search, path[u].node->module, path[u].node->name, strlen(path[u].node->name), path[u].node->nodetype, NULL, 0);
                if (node) {
                    break;
                }
            }
        }
        if (!node) {
            return NULL;
        }

        /* check predicate if any */
        LY_ARRAY_FOR(path[u].predicates, x) {
            if (path[u].predicates[x].type == 0) {
                /* position predicate */
                if (pos != path[u].predicates[x].position) {
                    pos++;
                    goto search_repeat;
                }
                /* done, no more predicates are allowed here */
                break;
            } else if (path[u].predicates[x].type == 1) {
                /* key-predicate */
                struct lysc_type *type = ((struct lysc_node_leaf*)path[u].predicates[x].key)->type;
                const struct lyd_node *key = lyd_search(lyd_node_children(node), path[u].predicates[x].key->module,
                                                        path[u].predicates[x].key->name, strlen(path[u].predicates[x].key->name),
                                                        LYS_LEAF, NULL, 0);
                if (!key) {
                    /* probably error and we shouldn't be here due to previous checks when creating path */
                    goto search_repeat;
                }
                if (type->plugin->compare(&((struct lyd_node_term*)key)->value, path[u].predicates[x].value)) {
                    goto search_repeat;
                }
            } else if (path[u].predicates[x].type == 2) {
                /* leaf-list-predicate */
                struct lysc_type *type = ((struct lysc_node_leaf*)path[u].node)->type;
                if (type->plugin->compare(&((struct lyd_node_term*)node)->value, path[u].predicates[x].value)) {
                    goto search_repeat;
                }
            } else {
                LOGINT(NULL);
            }
        }

        parent = node;
    }

    return (const struct lyd_node_term*)node;

search_repeat:
    start_search = node->next;
    if (parent) {
        goto search_inner;
    } else {
        goto search_toplevel;
    }
}

API LY_ERR
lyd_compare(const struct lyd_node *node1, const struct lyd_node *node2, int options)
{
    const struct lyd_node *iter1, *iter2;
    struct lyd_node_term *term1, *term2;
    struct lyd_node_any *any1, *any2;
    struct lysc_type *type;
    size_t len1, len2;

    if (!node1 || !node2) {
        if (node1 == node2) {
            return LY_SUCCESS;
        } else {
            return LY_ENOT;
        }
    }

    if (node1->schema->module->ctx != node2->schema->module->ctx || node1->schema != node2 ->schema) {
        return LY_ENOT;
    }

    if (node1->hash != node2->hash) {
        return LY_ENOT;
    }

    /* equal hashes do not mean equal nodes, they can be just in collision so the nodes must be checked explicitly */

    switch (node1->schema->nodetype) {
    case LYS_LEAF:
    case LYS_LEAFLIST:
        if (options & LYD_COMPARE_DEFAULTS) {
            if ((node1->flags & LYD_DEFAULT) != (node2->flags & LYD_DEFAULT)) {
                return LY_ENOT;
            }
        }

        term1 = (struct lyd_node_term*)node1;
        term2 = (struct lyd_node_term*)node2;
        type = ((struct lysc_node_leaf*)node1->schema)->type;

        return type->plugin->compare(&term1->value, &term2->value);
    case LYS_CONTAINER:
        if (options & LYD_COMPARE_DEFAULTS) {
            if ((node1->flags & LYD_DEFAULT) != (node2->flags & LYD_DEFAULT)) {
                return LY_ENOT;
            }
        }
        if (options & LYD_COMPARE_FULL_RECURSION) {
            iter1 = ((struct lyd_node_inner*)node1)->child;
            iter2 = ((struct lyd_node_inner*)node2)->child;
            goto all_children_compare;
        }
        return LY_SUCCESS;
    case LYS_ACTION:
        if (options & LYD_COMPARE_FULL_RECURSION) {
            /* TODO action/RPC
            goto all_children_compare;
            */
        }
        return LY_SUCCESS;
    case LYS_NOTIF:
        if (options & LYD_COMPARE_FULL_RECURSION) {
            /* TODO Notification
            goto all_children_compare;
            */
        }
        return LY_SUCCESS;
    case LYS_LIST:
        iter1 = ((struct lyd_node_inner*)node1)->child;
        iter2 = ((struct lyd_node_inner*)node2)->child;

        if (!(node1->schema->flags & LYS_KEYLESS) && !(options & LYD_COMPARE_FULL_RECURSION)) {
            /* lists with keys, their equivalence is based on their keys */
            for (struct lysc_node *key = ((struct lysc_node_list*)node1->schema)->child;
                    key && key->nodetype == LYS_LEAF && (key->flags & LYS_KEY);
                    key = key->next) {
                if (lyd_compare(iter1, iter2, options)) {
                    return LY_ENOT;
                }
                iter1 = iter1->next;
                iter2 = iter2->next;
            }
        } else {
            /* lists without keys, their equivalence is based on equivalence of all the children (both direct and indirect) */

all_children_compare:
            if (!iter1 && !iter2) {
                /* no children, nothing to compare */
                return LY_SUCCESS;
            }

            for (; iter1 && iter2; iter1 = iter1->next, iter2 = iter2->next) {
                if (lyd_compare(iter1, iter2, options | LYD_COMPARE_FULL_RECURSION)) {
                    return LY_ENOT;
                }
            }
            if (iter1 || iter2) {
                return LY_ENOT;
            }
        }
        return LY_SUCCESS;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        any1 = (struct lyd_node_any*)node1;
        any2 = (struct lyd_node_any*)node2;

        if (any1->value_type != any2->value_type) {
            return LY_ENOT;
        }
        switch (any1->value_type) {
        case LYD_ANYDATA_DATATREE:
            iter1 = any1->value.tree;
            iter2 = any2->value.tree;
            goto all_children_compare;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_XML:
        case LYD_ANYDATA_JSON:
            len1 = strlen(any1->value.str);
            len2 = strlen(any2->value.str);
            if (len1 != len2 || strcmp(any1->value.str, any2->value.str)) {
                return LY_ENOT;
            }
            return LY_SUCCESS;
#if 0 /* TODO LYB format */
        case LYD_ANYDATA_LYB:
            int len1 = lyd_lyb_data_length(any1->value.mem);
            int len2 = lyd_lyb_data_length(any2->value.mem);
            if (len1 != len2 || memcmp(any1->value.mem, any2->value.mem, len1)) {
                return LY_ENOT;
            }
            return LY_SUCCESS;
#endif
        }
    }

    LOGINT(node1->schema->module->ctx);
    return LY_EINT;
}

/**
 * @brief Duplicates just a single node and interconnect it into a @p parent (if present) and after the @p prev
 * sibling (if present).
 *
 * Ignores LYD_DUP_WITH_PARENTS and LYD_DUP_WITH_SIBLINGS which are supposed to be handled by lyd_dup().
 */
static struct lyd_node *
lyd_dup_recursive(const struct lyd_node *node, struct lyd_node_inner *parent, struct lyd_node *prev, int options)
{
    struct ly_ctx *ctx;
    struct lyd_node *dup = NULL;

    LY_CHECK_ARG_RET(NULL, node, NULL);
    ctx = node->schema->module->ctx;

    switch (node->schema->nodetype) {
    case LYS_ACTION:
    case LYS_NOTIF:
    case LYS_CONTAINER:
    case LYS_LIST:
        dup = calloc(1, sizeof(struct lyd_node_inner));
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        dup = calloc(1, sizeof(struct lyd_node_term));
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        dup = calloc(1, sizeof(struct lyd_node_any));
        break;
    default:
        LOGINT(ctx);
        goto error;
    }

    /* TODO implement LYD_DUP_WITH_WHEN */
    dup->flags = node->flags;
    dup->schema = node->schema;

    /* interconnect the node at the end */
    dup->parent = parent;
    if (prev) {
        dup->prev = prev;
        prev->next = dup;
    } else {
        dup->prev = dup;
        if (parent) {
            parent->child = dup;
        }
    }
    if (parent) {
        parent->child->prev = dup;
    } else if (prev) {
        struct lyd_node *first;
        for (first = prev; first->prev != prev; first = first->prev);
        first->prev = dup;
    }

    /* TODO duplicate attributes, implement LYD_DUP_NO_ATTR */

    /* nodetype-specific work */
    if (dup->schema->nodetype & LYD_NODE_TERM) {
        struct lyd_node_term *term = (struct lyd_node_term*)dup;
        struct lyd_node_term *orig = (struct lyd_node_term*)node;

        term->hash = orig->hash;
        term->value.realtype = orig->value.realtype;
        LY_CHECK_ERR_GOTO(term->value.realtype->plugin->duplicate(ctx, &orig->value, &term->value),
                          LOGERR(ctx, LY_EINT, "Value duplication failed."), error);
    } else if (dup->schema->nodetype & LYD_NODE_INNER) {
        struct lyd_node_inner *inner = (struct lyd_node_inner*)dup;
        struct lyd_node_inner *orig = (struct lyd_node_inner*)node;
        struct lyd_node *child, *last = NULL;

        if (options & LYD_DUP_RECURSIVE) {
            /* duplicate all the children */
            LY_LIST_FOR(orig->child, child) {
                last = lyd_dup_recursive(child, inner, last, options);
                LY_CHECK_GOTO(!last, error);
            }
        } else if (dup->schema->nodetype == LYS_LIST && !(dup->schema->flags & LYS_KEYLESS)) {
            /* always duplicate keys of a list */
            child = orig->child;
            for (struct lysc_node *key = ((struct lysc_node_list*)dup->schema)->child;
                    key && key->nodetype == LYS_LEAF && (key->flags & LYS_KEY);
                    key = key->next) {
                if (!child) {
                    /* possibly not keys are present in filtered tree */
                    break;
                } else if (child->schema != key) {
                    /* possibly not all keys are present in filtered tree,
                     * but there can be also some non-key nodes */
                    continue;
                }
                last = lyd_dup_recursive(child, inner, last, options);
                child = child->next;
            }
        }
        lyd_hash(dup);
    } else if (dup->schema->nodetype & LYD_NODE_ANY) {
        struct lyd_node_any *any = (struct lyd_node_any*)dup;
        struct lyd_node_any *orig = (struct lyd_node_any*)node;

        any->hash = orig->hash;
        any->value_type = orig->value_type;
        switch (any->value_type) {
        case LYD_ANYDATA_DATATREE:
            if (orig->value.tree) {
                any->value.tree = lyd_dup(orig->value.tree, NULL, LYD_DUP_RECURSIVE | LYD_DUP_WITH_SIBLINGS);
                LY_CHECK_GOTO(!any->value.tree, error);
            }
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_XML:
        case LYD_ANYDATA_JSON:
            if (orig->value.str) {
                any->value.str = lydict_insert(ctx, orig->value.str, strlen(orig->value.str));
            }
            break;
        }
    }

    lyd_insert_hash(dup);
    return dup;

error:
    if (!parent && !prev) {
        lyd_free_tree(dup);
    }
    return NULL;
}

API struct lyd_node *
lyd_dup(const struct lyd_node *node, struct lyd_node_inner *parent, int options)
{
    struct ly_ctx *ctx;
    const struct lyd_node *orig;          /* original node to be duplicated */
    struct lyd_node *first = NULL;        /* the first duplicated node, this is returned */
    struct lyd_node *last = NULL;         /* the last sibling of the duplicated nodes */
    struct lyd_node *top = NULL;          /* the most higher created node */
    struct lyd_node_inner *local_parent = NULL; /* the direct parent node for the duplicated node(s) */
    int keyless_parent_list = 0;

    LY_CHECK_ARG_RET(NULL, node, NULL);
    ctx = node->schema->module->ctx;

    if (options & LYD_DUP_WITH_PARENTS) {
        struct lyd_node_inner *orig_parent, *iter;
        int repeat = 1;
        for (top = NULL, orig_parent = node->parent; repeat && orig_parent; orig_parent = orig_parent->parent) {
            if (parent && parent->schema == orig_parent->schema) {
                /* stop creating parents, connect what we have into the provided parent */
                iter = parent;
                repeat = 0;
                /* get know if there is a keyless list which we will have to rehash */
                for (struct lyd_node_inner *piter = parent; piter; piter = piter->parent) {
                    if (piter->schema->nodetype == LYS_LIST && (piter->schema->flags & LYS_KEYLESS)) {
                        keyless_parent_list = 1;
                        break;
                    }
                }
            } else {
                iter = (struct lyd_node_inner*)lyd_dup_recursive((struct lyd_node*)orig_parent, NULL, NULL, 0);
                LY_CHECK_GOTO(!iter, error);
            }
            if (!local_parent) {
                local_parent = iter;
            }
            if (iter->child) {
                /* 1) list - add after keys
                 * 2) provided parent with some children */
                iter->child->prev->next = top;
                if (top) {
                    top->prev = iter->child->prev;
                    iter->child->prev = top;
                }
            } else {
                iter->child = top;
                if (iter->schema->nodetype == LYS_LIST) {
                    /* keyless list - we will need to rehash it since we are going to add nodes into it */
                    keyless_parent_list = 1;
                }
            }
            if (top) {
                top->parent = iter;
            }
            top = (struct lyd_node*)iter;
        }
        if (repeat && parent) {
            /* given parent and created parents chain actually do not interconnect */
            LOGERR(ctx, LY_EINVAL, "Invalid argument parent (%s()) - does not interconnect with the created node's parents chain.", __func__);
            goto error;
        }
    } else {
        local_parent = parent;
    }

    if (local_parent && local_parent->child) {
        last = local_parent->child->prev;
    }

    LY_LIST_FOR(node, orig) {
        last = lyd_dup_recursive(orig, local_parent, last, options);
        LY_CHECK_GOTO(!last, error);
        if (!first) {
            first = last;
        }

        if (!(options & LYD_DUP_WITH_SIBLINGS)) {
            break;
        }
    }
    if (keyless_parent_list) {
        /* rehash */
        for (; local_parent; local_parent = local_parent->parent) {
            if (local_parent->schema->nodetype == LYS_LIST && (local_parent->schema->flags & LYS_KEYLESS)) {
                lyd_hash((struct lyd_node*)local_parent);
            }
        }
    }
    return first;

error:
    if (top) {
        lyd_free_tree(top);
    } else {
        lyd_free_withsiblings(first);
    }
    return NULL;
}
