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
#include "hash_table.h"
#include "tree_schema.h"
#include "plugins_exts_metadata.h"
#include "plugins_exts_internal.h"

struct ly_keys {
    char *str;
    struct {
        const struct lysc_node_leaf *schema;
        char *value;
        struct lyd_value val;
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
lyd_value_parse(struct lyd_node_term *node, const char *value, size_t value_len, int *dynamic, int second,
                ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format, const struct lyd_node **trees)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    int options = LY_TYPE_OPTS_STORE | (second ? LY_TYPE_OPTS_SECOND_CALL : 0) |
            (dynamic && *dynamic ? LY_TYPE_OPTS_DYNAMIC : 0) | (trees ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);
    assert(node);

    ctx = node->schema->module->ctx;

    type = ((struct lysc_node_leaf*)node->schema)->type;
    if (!second) {
        node->value.realtype = type;
    }
    ret = type->plugin->store(ctx, type, value, value_len, options, get_prefix, parser, format,
                             trees ? (void*)node : (void*)node->schema, trees,
                             &node->value, NULL, &err);
    if (ret && (ret != LY_EINCOMPLETE)) {
        if (err) {
            ly_err_print(err);
            LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
            ly_err_free(err);
        }
        goto error;
    } else if (dynamic) {
        *dynamic = 0;
    }

error:
    return ret;
}

/* similar to lyd_value_parse except can be used just to store the value, hence does also not support a second call */
static LY_ERR
lyd_value_store(struct lyd_value *val, const struct lysc_node *schema, const char *value, size_t value_len, int *dynamic,
                ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    int options = LY_TYPE_OPTS_STORE | LY_TYPE_OPTS_INCOMPLETE_DATA | (dynamic && *dynamic ? LY_TYPE_OPTS_DYNAMIC : 0);

    assert(val && schema && (schema->nodetype & LYD_NODE_TERM));

    ctx = schema->module->ctx;
    type = ((struct lysc_node_leaf *)schema)->type;
    val->realtype = type;
    ret = type->plugin->store(ctx, type, value, value_len, options, get_prefix, parser, format, (void *)schema, NULL,
                              val, NULL, &err);
    if (ret == LY_EINCOMPLETE) {
        /* this is fine, we do not need it resolved */
        ret = LY_SUCCESS;
    } else if (ret && err) {
        ly_err_print(err);
        LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
        ly_err_free(err);
    }
    if (!ret && dynamic) {
        *dynamic = 0;
    }

    return ret;
}

LY_ERR
lyd_value_parse_attr(struct ly_ctx *ctx, struct lyd_attr *attr, const char *value, size_t value_len, int *dynamic,
                     int second, ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format,
                     const struct lysc_node *ctx_snode, const struct lyd_node **trees)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct lyext_metadata *ant;
    int options = LY_TYPE_OPTS_STORE | (second ? LY_TYPE_OPTS_SECOND_CALL : 0) |
            (dynamic && *dynamic ? LY_TYPE_OPTS_DYNAMIC : 0) | (trees ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);

    assert(ctx && attr && ((trees && attr->parent) || ctx_snode));

    ant = attr->annotation->data;

    if (!second) {
        attr->value.realtype = ant->type;
    }
    ret = ant->type->plugin->store(ctx, ant->type, value, value_len, options, get_prefix, parser, format,
                                  trees ? (void *)attr->parent : (void *)ctx_snode, trees, &attr->value, NULL, &err);
    if (ret && (ret != LY_EINCOMPLETE)) {
        if (err) {
            ly_err_print(err);
            LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
            ly_err_free(err);
        }
        goto error;
    } else if (dynamic) {
        *dynamic = 0;
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

LY_ERR
lyd_create_term(const struct lysc_node *schema, const char *value, size_t value_len, int *dynamic,
                ly_clb_resolve_prefix get_prefix, void *prefix_data, LYD_FORMAT format, struct lyd_node **node)
{
    LY_ERR ret;
    struct lyd_node_term *term;

    assert(schema->nodetype & LYD_NODE_TERM);

    term = calloc(1, sizeof *term);
    LY_CHECK_ERR_RET(!term, LOGMEM(schema->module->ctx), LY_EMEM);

    term->schema = schema;
    term->prev = (struct lyd_node *)term;
    term->flags = LYD_NEW;

    ret = lyd_value_parse(term, value, value_len, dynamic, 0, get_prefix, prefix_data, format, NULL);
    if (ret && (ret != LY_EINCOMPLETE)) {
        free(term);
        return ret;
    }
    lyd_hash((struct lyd_node *)term);

    *node = (struct lyd_node *)term;
    return ret;
}

LY_ERR
lyd_create_term2(const struct lysc_node *schema, const struct lyd_value *val, struct lyd_node **node)
{
    LY_ERR ret;
    struct lyd_node_term *term;
    struct lysc_type *type;

    assert(schema->nodetype & LYD_NODE_TERM);

    term = calloc(1, sizeof *term);
    LY_CHECK_ERR_RET(!term, LOGMEM(schema->module->ctx), LY_EMEM);

    term->schema = schema;
    term->prev = (struct lyd_node *)term;
    term->flags = LYD_NEW;

    type = ((struct lysc_node_leaf *)schema)->type;
    ret = type->plugin->duplicate(schema->module->ctx, val, &term->value);
    if (ret) {
        LOGERR(schema->module->ctx, ret, "Value duplication failed.");
        free(term);
        return ret;
    }
    lyd_hash((struct lyd_node *)term);

    *node = (struct lyd_node *)term;
    return ret;
}

LY_ERR
lyd_create_inner(const struct lysc_node *schema, struct lyd_node **node)
{
    struct lyd_node_inner *in;

    assert(schema->nodetype & LYD_NODE_INNER);

    in = calloc(1, sizeof *in);
    LY_CHECK_ERR_RET(!in, LOGMEM(schema->module->ctx), LY_EMEM);

    in->schema = schema;
    in->prev = (struct lyd_node *)in;
    in->flags = LYD_NEW;

    /* do not hash list with keys, we need them for the hash */
    if ((schema->nodetype != LYS_LIST) || (schema->flags & LYS_KEYLESS)) {
        lyd_hash((struct lyd_node *)in);
    }
    if ((schema->nodetype == LYS_CONTAINER) && !(schema->flags & LYS_PRESENCE)) {
        /* NP cotnainer always a default */
        in->flags |= LYD_DEFAULT;
    }

    *node = (struct lyd_node *)in;
    return LY_SUCCESS;
}

static void
ly_keys_clean(struct ly_keys *keys)
{
    size_t i;

    for (i = 0; i < keys->key_count; ++i) {
        keys->keys[i].schema->type->plugin->free(keys->keys[i].schema->module->ctx, &keys->keys[i].val);
    }
    free(keys->str);
    free(keys->keys);
}

static char *
ly_keys_parse_next(char **next_key, char **key_name)
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
    *next_key = ptr;
    return NULL;
}

/* fill keys structure; if store is set, fill also each val */
static LY_ERR
ly_keys_parse(const struct lysc_node *list, const char *keys_str, size_t keys_len, int store, struct ly_keys *keys)
{
    LY_ERR ret = LY_SUCCESS;
    char *next_key, *name;
    const struct lysc_node *key;
    size_t i;

    assert(list->nodetype == LYS_LIST);

    memset(keys, 0, sizeof *keys);

    keys->str = strndup(keys_str, keys_len ? keys_len : strlen(keys_str));
    LY_CHECK_ERR_GOTO(!keys->str, LOGMEM(list->module->ctx); ret = LY_EMEM, cleanup);

    next_key = keys->str;
    while (next_key[0]) {
        /* new key */
        keys->keys = ly_realloc(keys->keys, (keys->key_count + 1) * sizeof *keys->keys);
        LY_CHECK_ERR_GOTO(!keys->keys, LOGMEM(list->module->ctx); ret = LY_EMEM, cleanup);

        /* fill */
        keys->keys[keys->key_count].value = ly_keys_parse_next(&next_key, &name);
        if (!keys->keys[keys->key_count].value) {
            LOGERR(list->module->ctx, LY_EINVAL, "Invalid keys string (at \"%s\").", next_key);
            ret = LY_EINVAL;
            goto cleanup;
        }

        /* find schema node */
        key = lys_find_child(list, list->module, name, 0, LYS_LEAF, 0);
        if (!key) {
            LOGERR(list->module->ctx, LY_EINVAL, "List \"%s\" has no key \"%s\".", list->name, name);
            ret = LY_EINVAL;
            goto cleanup;
        }
        keys->keys[keys->key_count].schema = (const struct lysc_node_leaf *)key;

        /* check that we do not have it already */
        for (i = 0; i < keys->key_count; ++i) {
            if (keys->keys[i].schema == keys->keys[keys->key_count].schema) {
                LOGERR(list->module->ctx, LY_EINVAL, "Duplicit key \"%s\" value.", name);
                ret = LY_EINVAL;
                goto cleanup;
            }
        }

        if (store) {
            /* store the value */
            ret = lyd_value_store(&keys->keys[keys->key_count].val, key, keys->keys[keys->key_count].value, 0, 0,
                                  lydjson_resolve_prefix, NULL, LYD_JSON);
            LY_CHECK_GOTO(ret, cleanup);
        }

        /* another valid key */
        ++keys->key_count;
    }

cleanup:
    ly_keys_clean(keys);
    return ret;
}

LY_ERR
lyd_create_list(const struct lysc_node *schema, const char *keys_str, size_t keys_len, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lysc_node *key_s;
    struct lyd_node *list = NULL, *key;
    struct ly_keys keys = {0};
    size_t i;

    assert((schema->nodetype == LYS_LIST) && !(schema->flags & LYS_KEYLESS));

    /* parse keys */
    LY_CHECK_GOTO(ret = ly_keys_parse(schema, keys_str, keys_len, 0, &keys), cleanup);

    /* create list */
    LY_CHECK_GOTO(ret = lyd_create_inner(schema, &list), cleanup);

    /* everything was checked except that all keys are set */
    i = 0;
    for (key_s = lysc_node_children(schema, 0); key_s && (key_s->flags & LYS_KEY); key_s = key_s->next) {
        ++i;
    }
    if (i != keys.key_count) {
        LOGERR(schema->module->ctx, LY_EINVAL, "List \"%s\" is missing some keys.", schema->name);
        ret = LY_EINVAL;
        goto cleanup;
    }

    /* create and insert all the keys */
    for (i = 0; i < keys.key_count; ++i) {
        LY_CHECK_GOTO(ret = lyd_create_term((struct lysc_node *)keys.keys[i].schema, keys.keys[i].value, 0, 0,
                                            lydjson_resolve_prefix, NULL, LYD_JSON, &key), cleanup);
        lyd_insert_node(list, NULL, key);
    }

    /* hash having all the keys */
    lyd_hash(list);

    /* success */
    *node = list;
    list = NULL;

cleanup:
    lyd_free_tree(list);
    ly_keys_clean(&keys);
    return ret;
}

LY_ERR
lyd_create_any(const struct lysc_node *schema, const void *value, LYD_ANYDATA_VALUETYPE value_type, struct lyd_node **node)
{
    struct lyd_node_any *any;

    assert(schema->nodetype & LYD_NODE_ANY);

    any = calloc(1, sizeof *any);
    LY_CHECK_ERR_RET(!any, LOGMEM(schema->module->ctx), LY_EMEM);

    any->schema = schema;
    any->prev = (struct lyd_node *)any;
    any->flags = LYD_NEW;

    any->value.xml = value;
    any->value_type = value_type;
    lyd_hash((struct lyd_node *)any);

    *node = (struct lyd_node *)any;
    return LY_SUCCESS;
}

struct lyd_node *
lyd_get_prev_key_anchor(const struct lyd_node *first_sibling, const struct lysc_node *new_key)
{
    const struct lysc_node *prev_key;
    struct lyd_node *match = NULL;

    if (!first_sibling) {
        return NULL;
    }

    for (prev_key = new_key->prev; !match && prev_key->next; prev_key = prev_key->prev) {
        lyd_find_sibling_val(first_sibling, prev_key, NULL, 0, &match);
    }

    return match;
}

/**
 * @brief Insert node after a sibling.
 *
 * @param[in] sibling Sibling to insert after.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node)
{
    assert(!node->next && (node->prev == node));

    node->next = sibling->next;
    node->prev = sibling;
    sibling->next = node;
    if (node->next) {
        /* sibling had a succeeding node */
        node->next->prev = node;
    } else {
        /* sibling was last, find first sibling and change its prev */
        if (sibling->parent) {
            sibling = sibling->parent->child;
        } else {
            for (; sibling->prev->next != node; sibling = sibling->prev);
        }
        sibling->prev = node;
    }
    node->parent = sibling->parent;
}

/**
 * @brief Insert node before a sibling.
 *
 * @param[in] sibling Sibling to insert before.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node)
{
    assert(!node->next && (node->prev == node));

    node->next = sibling;
    /* covers situation of sibling being first */
    node->prev = sibling->prev;
    sibling->prev = node;
    if (node->prev->next) {
        /* sibling had a preceding node */
        node->prev->next = node;
    } else if (sibling->parent) {
        /* sibling was first and we must also change parent child pointer */
        sibling->parent->child = node;
    }
    node->parent = sibling->parent;
}

/**
 * @brief Insert node as the last child of a parent.
 *
 * @param[in] parent Parent to insert into.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_last(struct lyd_node *parent, struct lyd_node *node)
{
    struct lyd_node_inner *par;

    assert(!node->next && (node->prev == node));
    assert(parent->schema->nodetype & LYD_NODE_INNER);

    par = (struct lyd_node_inner *)parent;

    if (!par->child) {
        par->child = node;
    } else {
        node->prev = par->child->prev;
        par->child->prev->next = node;
        par->child->prev = node;
    }
    node->parent = par;
}

void
lyd_insert_node(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node *node)
{
    struct lyd_node *anchor;

    assert((parent || first_sibling) && node && node->hash);

    if (!parent && first_sibling && (*first_sibling) && (*first_sibling)->parent) {
        parent = (struct lyd_node *)(*first_sibling)->parent;
    }

    if (parent) {
        if (node->schema->flags & LYS_KEY) {
            /* it is key and we need to insert it at the correct place */
            anchor = lyd_get_prev_key_anchor(lyd_node_children(parent), node->schema);
            if (anchor) {
                lyd_insert_after(anchor, node);
            } else if (lyd_node_children(parent)) {
                lyd_insert_before((struct lyd_node *)lyd_node_children(parent), node);
            } else {
                lyd_insert_last(parent, node);
            }
        } else {
            /* last child */
            lyd_insert_last(parent, node);
        }
    } else if (*first_sibling) {
        /* top-level siblings, find the last one from this module, or simply the last */
        anchor = (*first_sibling)->prev;
        while (anchor->prev->next && (lyd_top_node_module(anchor) != lyd_top_node_module(node))) {
            anchor = anchor->prev;
        }

        /* insert */
        lyd_insert_after(anchor, node);
    } else {
        /* the only sibling */
        *first_sibling = node;
    }

    if (!(node->flags & LYD_DEFAULT)) {
        /* remove default flags from NP containers */
        while (parent && (parent->flags & LYD_DEFAULT)) {
            parent->flags &= ~LYD_DEFAULT;
            parent = (struct lyd_node *)parent->parent;
        }
    }

    /* insert into hash table */
    lyd_insert_hash(node);
}

LY_ERR
lyd_create_attr(struct lyd_node *parent, struct lyd_attr **attr, const struct lys_module *mod, const char *name,
                size_t name_len, const char *value, size_t value_len, int *dynamic, ly_clb_resolve_prefix get_prefix,
                void *prefix_data, LYD_FORMAT format, const struct lysc_node *ctx_snode)
{
    LY_ERR ret;
    struct lysc_ext_instance *ant = NULL;
    struct lyd_attr *at, *last;
    uint32_t v;

    assert((parent || attr) && mod);

    LY_ARRAY_FOR(mod->compiled->exts, v) {
        if (mod->compiled->exts[v].def->plugin == lyext_plugins_internal[LYEXT_PLUGIN_INTERNAL_ANNOTATION].plugin &&
                !ly_strncmp(mod->compiled->exts[v].argument, name, name_len)) {
            /* we have the annotation definition */
            ant = &mod->compiled->exts[v];
            break;
        }
    }
    if (!ant) {
        /* attribute is not defined as a metadata annotation (RFC 7952) */
        LOGERR(mod->ctx, LY_EINVAL, "Annotation definition for attribute \"%s:%.*s\" not found.",
                mod->name, name_len, name);
        return LY_EINVAL;
    }

    at = calloc(1, sizeof *at);
    LY_CHECK_ERR_RET(!at, LOGMEM(mod->ctx), LY_EMEM);
    at->parent = parent;
    at->annotation = ant;
    ret = lyd_value_parse_attr(mod->ctx, at, value, value_len, dynamic, 0, get_prefix, prefix_data, format, ctx_snode, NULL);
    if ((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE)) {
        free(at);
        return ret;
    }
    at->name = lydict_insert(mod->ctx, name, name_len);

    /* insert as the last attribute */
    if (parent) {
        if (parent->attr) {
            for (last = parent->attr; last->next; last = last->next);
            last->next = at;
        } else {
            parent->attr = at;
        }
    } else if (*attr) {
        for (last = *attr; last->next; last = last->next);
        last->next = at;
    }

    /* remove default flags from NP containers */
    while (parent && (parent->flags & LYD_DEFAULT)) {
        parent->flags &= ~LYD_DEFAULT;
        parent = (struct lyd_node *)parent->parent;
    }

    if (attr) {
        *attr = at;
    }
    return ret;
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

LY_ERR
lyd_find_sibling_next2(const struct lyd_node *first, const struct lysc_node *schema, const char *key_or_value,
                       size_t val_len, struct lyd_node **match)
{
    LY_ERR rc;
    const struct lyd_node *node = NULL;
    struct lyd_node_term *term;
    struct ly_keys keys = {0};
    struct lyd_value val = {0};
    size_t i;

    LY_CHECK_ARG_RET(NULL, schema, LY_EINVAL);

    if (!first) {
        /* no data */
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (key_or_value && !val_len) {
        val_len = strlen(key_or_value);
    }

    if (key_or_value && (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        /* store the value */
        LY_CHECK_GOTO(rc = lyd_value_store(&val, schema, key_or_value, val_len, 0, lydjson_resolve_prefix, NULL, LYD_JSON), cleanup);
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
                    LOGINT_RET(schema->module->ctx);
                }
                LY_CHECK_GOTO(rc, cleanup);

                /* compare values */
                if (!term->value.realtype->plugin->compare(&term->value, &keys.keys[i].val)) {
                    break;
                }
            }

            if (i < keys.key_count) {
                /* not a match */
                continue;
            }
        } else if ((schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && val.realtype) {
            term = (struct lyd_node_term *)node;

            /* compare values */
            if (!term->value.realtype->plugin->compare(&term->value, &val)) {
                /* not a match */
                continue;
            }
        }

        /* all criteria passed */
        break;
    }

    if (!node) {
        rc = LY_ENOTFOUND;
        if (match) {
            *match = NULL;
        }
        goto cleanup;
    }

    /* success */
    if (match) {
        *match = (struct lyd_node *)node;
    }
    rc = LY_SUCCESS;

cleanup:
    ly_keys_clean(&keys);
    if (val.realtype) {
        val.realtype->plugin->free(schema->module->ctx, &val);
    }
    return rc;
}

API LY_ERR
lyd_find_sibling_next(const struct lyd_node *first, const struct lys_module *module, const char *name, size_t name_len,
                      const char *key_or_value, size_t val_len, struct lyd_node **match)
{
    const struct lysc_node *schema;

    LY_CHECK_ARG_RET(NULL, module, name, match, LY_EINVAL);

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

    return lyd_find_sibling_next2(first, schema, key_or_value, val_len, match);
}

API LY_ERR
lyd_find_sibling_first(const struct lyd_node *siblings, const struct lyd_node *target, struct lyd_node **match)
{
    struct lyd_node **match_p;
    struct lyd_node_inner *parent;

    LY_CHECK_ARG_RET(NULL, target, match, LY_EINVAL);

    if (!siblings) {
        /* no data */
        if (match) {
            *match = NULL;
        }
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
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (match) {
        *match = (struct lyd_node *)siblings;
    }
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

static int
lyd_hash_table_schema_val_equal(void *val1_p, void *val2_p, int UNUSED(mod), void *UNUSED(cb_data))
{
    struct lysc_node *val1;
    struct lyd_node *val2;

    val1 = *((struct lysc_node **)val1_p);
    val2 = *((struct lyd_node **)val2_p);

    assert(val1->nodetype & (LYD_NODE_INNER | LYS_LEAF));

    if (val1 == val2->schema) {
        /* schema match is enough */
        return 1;
    } else {
        return 0;
    }
}

static LY_ERR
lyd_find_sibling_schema(const struct lyd_node *siblings, const struct lysc_node *schema, struct lyd_node **match)
{
    struct lyd_node **match_p;
    struct lyd_node_inner *parent;
    uint32_t hash;
    values_equal_cb ht_cb;

    assert(siblings && schema && (schema->nodetype & (LYD_NODE_INNER | LYS_LEAF)));

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
        /* calculate our hash */
        hash = dict_hash_multi(0, schema->module->name, strlen(schema->module->name));
        hash = dict_hash_multi(hash, schema->name, strlen(schema->name));
        hash = dict_hash_multi(hash, NULL, 0);

        /* use special hash table function */
        ht_cb = lyht_set_cb(parent->children_ht, lyd_hash_table_schema_val_equal);

        /* find by hash */
        if (!lyht_find(parent->children_ht, &schema, hash, (void **)&match_p)) {
            siblings = *match_p;
        } else {
            /* not found */
            siblings = NULL;
        }

        /* set the original hash table compare function back */
        lyht_set_cb(parent->children_ht, ht_cb);
    } else {
        /* no children hash table */
        for (; siblings; siblings = siblings->next) {
            if (siblings->schema == schema) {
                /* schema match is enough */
                break;
            }
        }
    }

    if (!siblings) {
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (match) {
        *match = (struct lyd_node *)siblings;
    }
    return LY_SUCCESS;
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
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    /* create data node if needed and find it */
    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_ANYXML:
    case LYS_ANYDATA:
    case LYS_NOTIF:
    case LYS_ACTION:
    case LYS_LEAF:
        /* find it based on schema only */
        rc = lyd_find_sibling_schema(siblings, schema, match);
        break;
    case LYS_LEAFLIST:
        /* target used attributes: schema, hash, value */
        LY_CHECK_RET(lyd_create_term(schema, key_or_value, val_len, NULL, lydjson_resolve_prefix, NULL, LYD_JSON, &target));
        /* fallthrough */
    case LYS_LIST:
        if (schema->nodetype == LYS_LIST) {
            /* target used attributes: schema, hash, child (all keys) */
            LY_CHECK_RET(lyd_create_list(schema, key_or_value, val_len, &target));
        }

        /* find it */
        rc = lyd_find_sibling_first(siblings, target, match);
        break;
    default:
        /* unreachable */
        LOGINT(schema->module->ctx);
        return LY_EINT;
    }

    lyd_free_tree(target);
    return rc;
}
