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
    node->value.plugin = type->plugin;
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
    type->plugin->free(ctx, type, &data);

    return ret;
}

static struct lyd_node *
lyd_parse_mem_(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, va_list ap)
{
    struct lyd_node *result = NULL;
    const struct lyd_node *rpc_act = NULL, *data_tree = NULL, *iter;
#if 0
    const char *yang_data_name = NULL;
#endif

    if (lyd_parse_check_options(ctx, options, __func__)) {
        return NULL;
    }

    if (options & LYD_OPT_RPCREPLY) {
        rpc_act = va_arg(ap, const struct lyd_node *);
        if (!rpc_act || rpc_act->parent || !(rpc_act->schema->nodetype & (LYS_ACTION | LYS_LIST | LYS_CONTAINER))) {
            LOGERR(ctx, LY_EINVAL, "Data parser invalid variable parameter (const struct lyd_node *rpc_act).");
            return NULL;
        }
    }
    if (options & (LYD_OPT_RPC | LYD_OPT_NOTIF | LYD_OPT_RPCREPLY)) {
        data_tree = va_arg(ap, const struct lyd_node *);
        if (data_tree) {
            if (options & LYD_OPT_NOEXTDEPS) {
                LOGERR(ctx, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree and LYD_OPT_NOEXTDEPS set).",
                       __func__);
                return NULL;
            }

            LY_LIST_FOR(data_tree, iter) {
                if (iter->parent) {
                    /* a sibling is not top-level */
                    LOGERR(ctx, LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *data_tree).", __func__);
                    return NULL;
                }
            }

            /* move it to the beginning */
            for (; data_tree->prev->next; data_tree = data_tree->prev);

            /* LYD_OPT_NOSIBLINGS cannot be set in this case */
            if (options & LYD_OPT_NOSIBLINGS) {
                LOGERR(ctx, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree with LYD_OPT_NOSIBLINGS).", __func__);
                return NULL;
            }
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
        lyd_parse_xml(ctx, data, options, &result);
        break;
#if 0
    case LYD_JSON:
        lyd_parse_json(ctx, data, options, rpc_act, data_tree, yang_data_name);
        break;
    case LYD_LYB:
        lyd_parse_lyb(ctx, data, options, data_tree, yang_data_name, NULL);
        break;
#endif
    case LYD_UNKNOWN:
        LOGINT(ctx);
        break;
    }

    return result;
}

API struct lyd_node *
lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, ...)
{
    va_list ap;
    struct lyd_node *result;

    va_start(ap, options);
    result = lyd_parse_mem_(ctx, data, format, options, ap);
    va_end(ap);

    return result;
}

static struct lyd_node *
lyd_parse_fd_(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, va_list ap)
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
    result = lyd_parse_mem_(ctx, addr ? addr : "", format, options, ap);
    if (addr) {
        ly_munmap(addr, length);
    }

    return result;
}

API struct lyd_node *
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...)
{
    struct lyd_node *ret;
    va_list ap;

    va_start(ap, options);
    ret = lyd_parse_fd_(ctx, fd, format, options, ap);
    va_end(ap);

    return ret;
}

API struct lyd_node *
lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, ...)
{
    int fd;
    struct lyd_node *result;
    size_t len;
    va_list ap;

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

    va_start(ap, options);
    result = lyd_parse_fd_(ctx, fd, format, options, ap);

    va_end(ap);
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


