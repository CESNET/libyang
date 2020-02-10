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
#include "plugins_exts_metadata.h"

struct ly_keys {
    char *str;
    struct {
        const struct lysc_node_leaf *schema;
        char *value;
        const char *can_val;
    } *keys;
    size_t key_count;
};

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

LY_ERR
lyd_value_parse_attr(struct lyd_attr *attr, const char *value, size_t value_len, int dynamic, int second,
                     ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format, const struct lyd_node **trees)
{
    LY_ERR ret = LY_SUCCESS, rc;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lyext_metadata *ant;
    int options = LY_TYPE_OPTS_STORE | (second ? LY_TYPE_OPTS_SECOND_CALL : 0) |
            (dynamic ? LY_TYPE_OPTS_DYNAMIC : 0) | (trees ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);
    assert(attr);

    ctx = attr->parent->schema->module->ctx;
    ant = attr->annotation->data;

    if (!second) {
        attr->value.realtype = ant->type;
    }
    rc = ant->type->plugin->store(ctx, ant->type, value, value_len, options, get_prefix, parser, format,
                                  trees ? (void*)attr->parent : (void*)attr->parent->schema, trees,
                                  &attr->value, NULL, &err);
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
                  ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format, const struct lyd_node **trees)
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

API const char *
lyd_value2str(const struct lyd_node_term *node, int *dynamic)
{
    LY_CHECK_ARG_RET(node ? node->schema->module->ctx : NULL, node, dynamic, NULL);

    return node->value.realtype->plugin->print(&node->value, LYD_JSON, json_print_get_prefix, NULL, dynamic);
}

API const char *
lyd_attr2str(const struct lyd_attr *attr, int *dynamic)
{
    LY_CHECK_ARG_RET(attr ? attr->parent->schema->module->ctx : NULL, attr, dynamic, NULL);

    return attr->value.realtype->plugin->print(&attr->value, LYD_JSON, json_print_get_prefix, NULL, dynamic);
}

API struct lyd_node *
lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options)
{
    struct lyd_node *result = NULL;
#if 0
    const char *yang_data_name = NULL;
#endif

    LY_CHECK_ARG_RET(ctx, ctx, NULL);

#if 0
    if (options & LYD_OPT_RPCREPLY) {
        /* first item in trees is mandatory - the RPC/action request */
        LY_CHECK_ARG_RET(ctx, trees, LY_ARRAY_SIZE(trees) >= 1, NULL);
        if (!trees[0] || trees[0]->parent || !(trees[0]->schema->nodetype & (LYS_ACTION | LYS_LIST | LYS_CONTAINER))) {
            LOGERR(ctx, LY_EINVAL, "Data parser invalid argument trees - the first item in the array must be the RPC/action request when parsing %s.",
                   lyd_parse_options_type2str(options));
            return NULL;
        }
    }

    if (options & LYD_OPT_DATA_TEMPLATE) {
        yang_data_name = va_arg(ap, const char *);
    }
#endif

    if (!format) {
        /* TODO try to detect format from the content */
    }

    switch (format) {
    case LYD_XML:
        lyd_parse_xml(ctx, data, options, &result);
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
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options)
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
    result = lyd_parse_mem(ctx, addr ? addr : "", format, options);
    if (addr) {
        ly_munmap(addr, length);
    }

    return result;
}

API struct lyd_node *
lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options)
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

    result = lyd_parse_fd(ctx, fd, format, options);
    close(fd);

    return result;
}

API const struct lyd_node_term *
lyd_target(struct lyd_value_path *path, const struct lyd_node **trees)
{
    unsigned int u, v, x;
    const struct lyd_node *parent = NULL, *start_search;
    struct lyd_node *node = NULL;
    uint64_t pos = 1;

    LY_CHECK_ARG_RET(NULL, path, trees, NULL);

    LY_ARRAY_FOR(path, u) {
        if (parent) {
            start_search = lyd_node_children(parent);
search_inner:
            lyd_find_sibling_next(start_search, path[u].node->module, path[u].node->name, 0, NULL, 0, &node);
        } else {
            LY_ARRAY_FOR(trees, v) {
                start_search = trees[v];
search_toplevel:
                /* WARNING! to use search_toplevel label correctly, variable v must be preserved and not changed! */
                lyd_find_sibling_next(start_search, path[u].node->module, path[u].node->name, 0, NULL, 0, &node);
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
                struct lyd_node *key;
                lyd_find_sibling_next(lyd_node_children(node), path[u].predicates[x].key->module,
                                      path[u].predicates[x].key->name, 0, NULL, 0, &key);
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

    if (node1->schema->module->ctx != node2->schema->module->ctx || node1->schema != node2->schema) {
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

static LY_ERR
lyd_path_str_enlarge(char **buffer, size_t *buflen, size_t reqlen, int is_static)
{
    /* ending \0 */
    ++reqlen;

    if (reqlen > *buflen) {
        if (is_static) {
            return LY_EINCOMPLETE;
        }

        *buffer = ly_realloc(*buffer, reqlen * sizeof **buffer);
        if (!*buffer) {
            return LY_EMEM;
        }

        *buflen = reqlen;
    }

    return LY_SUCCESS;
}

/**
 * @brief Append all list key predicates to path.
 *
 * @param[in] node Node with keys to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR
 */
static LY_ERR
lyd_path_list_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, int is_static)
{
    const struct lyd_node *key;
    int dynamic = 0;
    size_t len;
    const char *val;
    char quot;
    LY_ERR rc;

    for (key = lyd_node_children(node); key && (key->schema->flags & LYS_KEY); key = key->next) {
        val = lyd_value2str((struct lyd_node_term *)key, &dynamic);
        len = 1 + strlen(key->schema->name) + 2 + strlen(val) + 2;
        rc = lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static);
        if (rc != LY_SUCCESS) {
            if (dynamic) {
                free((char *)val);
            }
            return rc;
        }

        quot = '\'';
        if (strchr(val, '\'')) {
            quot = '"';
        }
        *bufused += sprintf(*buffer + *bufused, "[%s=%c%s%c]", key->schema->name, quot, val, quot);

        if (dynamic) {
            free((char *)val);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Append leaf-list value predicate to path.
 *
 * @param[in] node Node to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR
 */
static LY_ERR
lyd_path_leaflist_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, int is_static)
{
    int dynamic = 0;
    size_t len;
    const char *val;
    char quot;
    LY_ERR rc;

    val = lyd_value2str((struct lyd_node_term *)node, &dynamic);
    len = 4 + strlen(val) + 2;
    rc = lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static);
    if (rc != LY_SUCCESS) {
        goto cleanup;
    }

    quot = '\'';
    if (strchr(val, '\'')) {
        quot = '"';
    }
    *bufused += sprintf(*buffer + *bufused, "[.=%c%s%c]", quot, val, quot);

cleanup:
    if (dynamic) {
        free((char *)val);
    }
    return rc;
}

/**
 * @brief Append node position (relative to its other instances) predicate to path.
 *
 * @param[in] node Node to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR
 */
static LY_ERR
lyd_path_position_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, int is_static)
{
    const struct lyd_node *first, *iter;
    size_t len;
    int pos;
    char *val = NULL;
    LY_ERR rc;

    if (node->parent) {
        first = node->parent->child;
    } else {
        for (first = node; node->prev->next; node = node->prev);
    }
    pos = 1;
    for (iter = first; iter != node; iter = iter->next) {
        if (iter->schema == node->schema) {
            ++pos;
        }
    }
    if (asprintf(&val, "%d", pos) == -1) {
        return LY_EMEM;
    }

    len = 1 + strlen(val) + 1;
    rc = lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static);
    if (rc != LY_SUCCESS) {
        goto cleanup;
    }

    *bufused += sprintf(*buffer + *bufused, "[%s]", val);

cleanup:
    free(val);
    return rc;
}

API char *
lyd_path(const struct lyd_node *node, LYD_PATH_TYPE pathtype, char *buffer, size_t buflen)
{
    int is_static = 0, i, depth;
    size_t bufused = 0, len;
    const struct lyd_node *iter;
    const struct lys_module *mod;
    LY_ERR rc;

    LY_CHECK_ARG_RET(NULL, node, NULL);
    if (buffer) {
        LY_CHECK_ARG_RET(node->schema->module->ctx, buflen > 1, NULL);
        is_static = 1;
    } else {
        buflen = 0;
    }

    switch (pathtype) {
    case LYD_PATH_LOG:
        depth = 1;
        for (iter = node; iter->parent; iter = (const struct lyd_node *)iter->parent) {
            ++depth;
        }

        goto iter_print;
        while (depth) {
            /* find the right node */
            for (iter = node, i = 1; i < depth; iter = (const struct lyd_node *)iter->parent, ++i);
iter_print:
            /* print prefix and name */
            mod = NULL;
            if (!iter->parent || (iter->schema->module != iter->parent->schema->module)) {
                mod = iter->schema->module;
            }

            /* realloc string */
            len = 1 + (mod ? strlen(mod->name) + 1 : 0) + strlen(iter->schema->name);
            rc = lyd_path_str_enlarge(&buffer, &buflen, bufused + len, is_static);
            if (rc != LY_SUCCESS) {
                break;
            }

            /* print next node */
            bufused += sprintf(buffer + bufused, "/%s%s%s", mod ? mod->name : "", mod ? ":" : "", iter->schema->name);

            switch (iter->schema->nodetype) {
            case LYS_LIST:
                if (iter->schema->flags & LYS_KEYLESS) {
                    /* print its position */
                    rc = lyd_path_position_predicate(iter, &buffer, &buflen, &bufused, is_static);
                } else {
                    /* print all list keys in predicates */
                    rc = lyd_path_list_predicate(iter, &buffer, &buflen, &bufused, is_static);
                }
                break;
            case LYS_LEAFLIST:
                if (iter->schema->flags & LYS_CONFIG_W) {
                    /* print leaf-list value */
                    rc = lyd_path_leaflist_predicate(iter, &buffer, &buflen, &bufused, is_static);
                } else {
                    /* print its position */
                    rc = lyd_path_position_predicate(iter, &buffer, &buflen, &bufused, is_static);
                }
                break;
            default:
                /* nothing to print more */
                rc = LY_SUCCESS;
                break;
            }
            if (rc != LY_SUCCESS) {
                break;
            }

            --depth;
        }
        break;
    }

    return buffer;
}

API struct ly_set *
lyd_find_instance(const struct lyd_node *sibling, const struct lysc_node *schema)
{
    struct ly_set *ret, *ret_aux, *spath;
    const struct lysc_node *siter, *sparent;
    const struct lyd_node *iter;
    unsigned int i, j;

    LY_CHECK_ARG_RET(NULL, sibling, schema, NULL);
    if (schema->nodetype & (LYS_CHOICE | LYS_CASE)) {
        LOGARG(schema->module->ctx, schema);
        return NULL;
    }

    ret = ly_set_new();
    spath = ly_set_new();
    LY_CHECK_ERR_GOTO(!ret || !spath, LOGMEM(schema->module->ctx), error);

    /* build schema path until sibling parent */
    sparent = sibling->parent ? sibling->parent->schema : NULL;
    for (siter = schema; siter && (siter != sparent); siter = siter->parent) {
        if (siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYDATA | LYS_NOTIF | LYS_RPC | LYS_ACTION)) {
            /* standard data node */
            ly_set_add(spath, (void *)siter, LY_SET_OPT_USEASLIST);

        } /* else skip the rest node types */
    }
    /* no valid path */
    LY_CHECK_GOTO(!spath->count, error);

    /* start searching */
    LY_LIST_FOR(sibling, iter) {
        if (iter->schema == spath->objs[spath->count - 1]) {
            ly_set_add(ret, (void *)iter, LY_SET_OPT_USEASLIST);
        }
    }
    for (i = spath->count - 1; i; i--) {
        if (!ret->count) {
            /* nothing found */
            break;
        }

        ret_aux = ly_set_new();
        LY_CHECK_ERR_GOTO(!ret_aux, LOGMEM(schema->module->ctx), error);
        for (j = 0; j < ret->count; j++) {
            LY_LIST_FOR(lyd_node_children(ret->objs[j]), iter) {
                if (iter->schema == spath->objs[i - 1]) {
                    ly_set_add(ret_aux, (void *)iter, LY_SET_OPT_USEASLIST);
                }
            }
        }
        ly_set_free(ret, NULL);
        ret = ret_aux;
    }

    ly_set_free(spath, NULL);
    return ret;

error:
    ly_set_free(ret, NULL);
    ly_set_free(spath, NULL);

    return NULL;
}

static char *
lyd_keys_parse_next(char **next_key, char **key_name)
{
    char *ptr, *ptr2, *val, quot;

    ptr = *next_key;

    /* "[" */
    LY_CHECK_GOTO(ptr[0] != '[', error);
    ++ptr;

    /* key name */
    ptr2 = strchr(ptr, '=');
    LY_CHECK_GOTO(!ptr2, error);

    *key_name = ptr;
    ptr2[0] = '\0';

    /* \0, was '=' */
    ptr = ptr2 + 1;

    /* quote */
    LY_CHECK_GOTO((ptr[0] != '\'') && (ptr[0] != '\"'), error);
    quot = ptr[0];
    ++ptr;

    /* value, terminate it */
    val = ptr;
    ptr2 = strchr(ptr, quot);
    LY_CHECK_GOTO(!ptr2, error);
    ptr2[0] = '\0';

    /* \0, was quote */
    ptr = ptr2 + 1;

    /* "]" */
    LY_CHECK_GOTO(ptr[0] != ']', error);
    ++ptr;

    *next_key = ptr;
    return val;

error:
    LOGERR(NULL, LY_EINVAL, "Invalid arguments - keys at \"%s\" (%s()).", ptr, __func__);
    return NULL;
}

static void
ly_keys_clean(struct ly_keys *keys)
{
    size_t i;

    for (i = 0; i < keys->key_count; ++i) {
        lydict_remove(keys->keys[i].schema->module->ctx, keys->keys[i].can_val);
    }
    free(keys->str);
    free(keys->keys);
}

static LY_ERR
lyd_value_canonize(const struct lysc_node *schema, const char *value, size_t val_len, const char **can_val)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_type *type;
    struct ly_err_item *err;

    assert(schema->nodetype & (LYS_LEAF | LYS_LEAFLIST));

    if (!val_len) {
        val_len = strlen(value);
    }

    /* check canonical value exists */
    type = (struct lysc_type *)&((struct lysc_node_leaf *)schema)->type;
    if (!type->plugin->has_canon(type)) {
        LOGERR(schema->module->ctx, LY_EINVAL, "Key \"%s\" has no canonical value.", schema->name);
        return LY_EINVAL;
    }

    /* make the value canonical */
    rc = type->plugin->store(schema->module->ctx, type, value, val_len, LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_SCHEMA,
                             NULL, NULL, 0, NULL, NULL, NULL, can_val, &err);
    if (rc != LY_SUCCESS) {
        ly_err_print(err);
        ly_err_free(err);
        return rc;
    }

    return LY_SUCCESS;
}

static LY_ERR
ly_keys_parse(const struct lysc_node *list, const char *keys_str, size_t keys_len, int canonize, struct ly_keys *keys)
{
    LY_ERR rc = LY_SUCCESS;
    char *next_key, *name;
    const struct lysc_node *key;

    assert(list->nodetype == LYS_LIST);

    memset(keys, 0, sizeof *keys);

    keys->str = strndup(keys_str, keys_len ? keys_len : strlen(keys_str));
    LY_CHECK_ERR_GOTO(!keys->str, LOGMEM(list->module->ctx); rc = LY_EMEM, cleanup);

    next_key = keys->str;
    while (next_key[0]) {
        /* new key */
        keys->keys = ly_realloc(keys->keys, (keys->key_count + 1) * sizeof *keys->keys);
        LY_CHECK_ERR_GOTO(!keys->keys, LOGMEM(list->module->ctx); rc = LY_EMEM, cleanup);

        /* fill */
        keys->keys[keys->key_count].value = lyd_keys_parse_next(&next_key, &name);

        /* find schema node */
        key = lys_find_child(list, list->module, name, 0, LYS_LEAF, 0);
        if (!key) {
            LOGERR(list->module->ctx, LY_EINVAL, "Node \"%s\" has no key \"%s\".", list->name, name);
            rc = LY_EINVAL;
            goto cleanup;
        }
        keys->keys[keys->key_count].schema = (const struct lysc_node_leaf *)key;

        if (canonize) {
            /* canonize the value */
            rc = lyd_value_canonize(key, keys->keys[keys->key_count].value, 0, &keys->keys[keys->key_count].can_val);
            LY_CHECK_GOTO(rc != LY_SUCCESS, cleanup);
        }

        /* another valid key */
        ++keys->key_count;
    }

cleanup:
    ly_keys_clean(keys);
    return rc;
}

API LY_ERR
lyd_find_sibling_next(const struct lyd_node *first, const struct lys_module *module, const char *name, size_t name_len,
                      const char *key_or_value, size_t val_len, struct lyd_node **match)
{
    LY_ERR rc;
    const struct lyd_node *node = NULL;
    struct lyd_node_term *term;
    const struct lysc_node *schema;
    struct ly_keys keys = {0};
    const char *value = NULL, *node_val;
    size_t i;
    int dynamic;

    LY_CHECK_ARG_RET(NULL, module, name, LY_EINVAL);

    if (!first) {
        /* no data */
        *match = NULL;
        return LY_ENOTFOUND;
    }

    /* find schema */
    schema = lys_find_child(first->parent ? first->parent->schema : NULL, module, name, name_len, 0, 0);
    if (!schema) {
        LOGERR(module->ctx, LY_EINVAL, "Schema node not found.");
        return LY_EINVAL;
    }

    if (key_or_value && !val_len) {
        val_len = strlen(key_or_value);
    }

    if (key_or_value && (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        /* canonize the value */
        LY_CHECK_GOTO(rc = lyd_value_canonize(schema, key_or_value, val_len, &value), cleanup);
    } else if (key_or_value && (schema->nodetype == LYS_LIST)) {
        /* parse keys into canonical values */
        LY_CHECK_GOTO(rc = ly_keys_parse(schema, key_or_value, val_len, 1, &keys), cleanup);
    }

    /* find first matching value */
    LY_LIST_FOR(first, node) {
        if (node->schema != schema) {
            continue;
        }

        if ((schema->nodetype == LYS_LIST) && keys.str) {
            /* compare all set keys */
            for (i = 0; i < keys.key_count; ++i) {
                /* find key */
                rc = lyd_find_sibling_val(lyd_node_children(node), (struct lysc_node *)keys.keys[i].schema, NULL, 0,
                                          (struct lyd_node **)&term);
                if (rc == LY_ENOTFOUND) {
                    /* all keys must always exist */
                    LOGINT_RET(module->ctx);
                }
                LY_CHECK_GOTO(rc, cleanup);

                /* compare values */
                node_val = term->value.realtype->plugin->print(&term->value, 0, NULL, NULL, &dynamic);
                assert(!dynamic);
                if (strcmp(node_val, keys.keys[i].can_val)) {
                    break;
                }
            }

            if (i < keys.key_count) {
                /* not a match */
                continue;
            }
        } else if ((schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && value) {
            term = (struct lyd_node_term *)node;

            /* compare values */
            node_val = term->value.realtype->plugin->print(&term->value, 0, NULL, NULL, &dynamic);
            assert(!dynamic);
            if (strcmp(node_val, value)) {
                /* not a match */
                continue;
            }
        }

        /* all criteria passed */
        break;
    }

    if (!node) {
        rc = LY_ENOTFOUND;
        *match = NULL;
        goto cleanup;
    }

    /* success */
    *match = (struct lyd_node *)node;
    rc = LY_SUCCESS;

cleanup:
    ly_keys_clean(&keys);
    lydict_remove(module->ctx, value);
    return rc;
}

API LY_ERR
lyd_find_sibling_first(const struct lyd_node *siblings, const struct lyd_node *target, struct lyd_node **match)
{
    struct lyd_node **match_p;
    struct lyd_node_inner *parent;

    LY_CHECK_ARG_RET(NULL, target, match, LY_EINVAL);

    if (!siblings) {
        /* no data */
        *match = NULL;
        return LY_ENOTFOUND;
    }

    /* find first sibling */
    if (siblings->parent) {
        siblings = siblings->parent->child;
    } else {
        while (siblings->prev->next) {
            siblings = siblings->prev;
        }
    }

    parent = (struct lyd_node_inner *)siblings->parent;
    if (parent && parent->children_ht) {
        assert(target->hash);

        /* find by hash */
        if (!lyht_find(parent->children_ht, &target, target->hash, (void **)&match_p)) {
            siblings = *match_p;
        } else {
            /* not found */
            siblings = NULL;
        }
    } else {
        /* no children hash table */
        for (; siblings; siblings = siblings->next) {
            if (!lyd_compare(siblings, target, 0)) {
                break;
            }
        }
    }

    if (!siblings) {
        *match = NULL;
        return LY_ENOTFOUND;
    }

    *match = (struct lyd_node *)siblings;
    return LY_SUCCESS;
}

API LY_ERR
lyd_find_sibling_set(const struct lyd_node *siblings, const struct lyd_node *target, struct ly_set **set)
{
    struct lyd_node_inner *parent;
    struct lyd_node *match;
    struct lyd_node **match_p;
    struct ly_set *ret;

    LY_CHECK_ARG_RET(NULL, target, set, LY_EINVAL);

    if (!siblings) {
        /* no data */
        return LY_ENOTFOUND;
    }

    ret = ly_set_new();
    LY_CHECK_ERR_RET(!ret, LOGMEM(target->schema->module->ctx), LY_EMEM);

    /* find first sibling */
    if (siblings->parent) {
        siblings = siblings->parent->child;
    } else {
        while (siblings->prev->next) {
            siblings = siblings->prev;
        }
    }

    parent = (struct lyd_node_inner *)siblings->parent;
    if (parent && parent->children_ht) {
        assert(target->hash);

        /* find by hash */
        if (!lyht_find(parent->children_ht, &target, target->hash, (void **)&match_p)) {
            match = *match_p;
        } else {
            /* not found */
            match = NULL;
        }
        while (match) {
            /* add all found nodes into the return set */
            if (ly_set_add(ret, match, LY_SET_OPT_USEASLIST) == -1) {
                goto error;
            }

            /* find next instance */
            if (lyht_find_next(parent->children_ht, &match, match->hash, (void **)&match_p)) {
                match = NULL;
            } else {
                match = *match_p;
            }
        }
    } else {
        /* no children hash table */
        for (; siblings; siblings = siblings->next) {
            if (!lyd_compare(siblings, target, 0)) {
                /* a match */
                if (ly_set_add(ret, (struct lyd_node *)siblings, LY_SET_OPT_USEASLIST) == -1) {
                    goto error;
                }
            }
        }
    }

    if (!ret->count) {
        ly_set_free(ret, NULL);
        return LY_ENOTFOUND;
    }

    *set = ret;
    return LY_SUCCESS;

error:
    ly_set_free(ret, NULL);
    return LY_EMEM;
}

API LY_ERR
lyd_find_sibling_val(const struct lyd_node *siblings, const struct lysc_node *schema, const char *key_or_value,
                     size_t val_len, struct lyd_node **match)
{
    LY_ERR rc;
    struct lyd_node *target = NULL;

    LY_CHECK_ARG_RET(NULL, schema, LY_EINVAL);
    if ((schema->nodetype == LYS_LIST) && schema->flags & LYS_KEYLESS) {
        LOGERR(schema->module->ctx, LY_EINVAL, "Invalid arguments - key-less list (%s()).", __func__);
        return LY_EINVAL;
    } else if ((schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) && !key_or_value) {
        LOGERR(schema->module->ctx, LY_EINVAL, "Invalid arguments - no value/keys for a (leaf-)list (%s()).", __func__);
        return LY_EINVAL;
    } else if (schema->nodetype & (LYS_CHOICE | LYS_CASE)) {
        LOGERR(schema->module->ctx, LY_EINVAL, "Invalid arguments - schema type %s (%s()).",
               lys_nodetype2str(schema->nodetype), __func__);
        return LY_EINVAL;
    }

    if (!siblings) {
        /* no data */
        *match = NULL;
        return LY_ENOTFOUND;
    }

    /* create data node */
    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_ANYXML:
    case LYS_ANYDATA:
    case LYS_NOTIF:
    case LYS_RPC:
        /* target used attributes: schema, hash */
        //TODO target = lyd_create(schema, 0);
        LY_CHECK_RET(!target, LY_EMEM);
        break;
    case LYS_LEAF:
        /* target used attributes: schema, hash */
        //TODO target = lyd_create_term(schema, NULL, 0, 0);
        LY_CHECK_RET(!target, LY_EMEM);
        break;
    case LYS_LEAFLIST:
        /* target used attributes: schema, hash, value */
        //TODO target = lyd_create_term(schema, key_or_value, val_len, 0);
        LY_CHECK_RET(!target, LY_EMEM);
        break;
    case LYS_LIST:
        /* target used attributes: schema, hash, child (all keys) */
        //TODO target = lyd_create_list(schema, key_or_value, val_len);
        LY_CHECK_RET(!target, LY_EMEM);
        break;
    default:
        /* unreachable */
        LOGINT(schema->module->ctx);
        return LY_EINT;
    }

    /* find it */
    rc = lyd_find_sibling_first(siblings, target, match);

    lyd_free_tree(target);
    return rc;
}
