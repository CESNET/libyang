/**
 * @file tree_data.c
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
#include "xml.h"
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

LY_ERR
lyd_value_parse(struct lyd_node_term *node, const char *value, size_t value_len, int *dynamic, int second,
                ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format, const struct lyd_node *tree)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    int options = LY_TYPE_OPTS_STORE | (second ? LY_TYPE_OPTS_SECOND_CALL : 0) |
            (dynamic && *dynamic ? LY_TYPE_OPTS_DYNAMIC : 0) | (tree ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);
    assert(node);

    ctx = node->schema->module->ctx;

    type = ((struct lysc_node_leaf*)node->schema)->type;
    if (!second) {
        node->value.realtype = type;
    }
    ret = type->plugin->store(ctx, type, value, value_len, options, get_prefix, parser, format,
                             tree ? (void *)node : (void *)node->schema, tree,
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
lyd_value_parse_meta(struct ly_ctx *ctx, struct lyd_meta *meta, const char *value, size_t value_len, int *dynamic,
                     int second, ly_clb_resolve_prefix get_prefix, void *parser, LYD_FORMAT format,
                     const struct lysc_node *ctx_snode, const struct lyd_node *tree)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct lyext_metadata *ant;
    int options = LY_TYPE_OPTS_STORE | (second ? LY_TYPE_OPTS_SECOND_CALL : 0) |
            (dynamic && *dynamic ? LY_TYPE_OPTS_DYNAMIC : 0) | (tree ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);

    assert(ctx && meta && ((tree && meta->parent) || ctx_snode));

    ant = meta->annotation->data;

    if (!second) {
        meta->value.realtype = ant->type;
    }
    ret = ant->type->plugin->store(ctx, ant->type, value, value_len, options, get_prefix, parser, format,
                                  tree ? (void *)meta->parent : (void *)ctx_snode, tree, &meta->value, NULL, &err);
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
                   ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format, const struct lyd_node *tree)
{
    LY_ERR rc;
    struct ly_err_item *err = NULL;
    struct lysc_type *type;
    int options = (tree ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);

    LY_CHECK_ARG_RET(ctx, node, value, LY_EINVAL);

    type = ((struct lysc_node_leaf*)node->schema)->type;
    rc = type->plugin->store(ctx ? ctx : node->schema->module->ctx, type, value, value_len, options,
                             get_prefix, get_prefix_data, format, tree ? (void*)node : (void*)node->schema, tree,
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
                  ly_clb_resolve_prefix get_prefix, void *get_prefix_data, LYD_FORMAT format, const struct lyd_node *tree)
{
    LY_ERR ret = LY_SUCCESS, rc;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    struct lyd_value data = {0};
    int options = LY_TYPE_OPTS_STORE | (tree ? 0 : LY_TYPE_OPTS_INCOMPLETE_DATA);

    LY_CHECK_ARG_RET(node ? node->schema->module->ctx : NULL, node, value, LY_EINVAL);

    ctx = node->schema->module->ctx;
    type = ((struct lysc_node_leaf*)node->schema)->type;
    rc = type->plugin->store(ctx, type, value, value_len, options, get_prefix, get_prefix_data, format, (struct lyd_node*)node,
                             tree, &data, NULL, &err);
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
lyd_meta2str(const struct lyd_meta *meta, int *dynamic)
{
    LY_CHECK_ARG_RET(meta ? meta->parent->schema->module->ctx : NULL, meta, dynamic, NULL);

    return meta->value.realtype->plugin->print(&meta->value, LYD_JSON, json_print_get_prefix, NULL, dynamic);
}

API struct lyd_node *
lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options)
{
    struct lyd_node *result = NULL;
#if 0
    const char *yang_data_name = NULL;
#endif

    LY_CHECK_ARG_RET(ctx, ctx, NULL);

    if ((options & LYD_OPT_PARSE_ONLY) && (options & LYD_VALOPT_MASK)) {
        LOGERR(ctx, LY_EINVAL, "Passing validation flags with LYD_OPT_PARSE_ONLY is not allowed.");
        return NULL;
    }

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
        lyd_parse_xml_data(ctx, data, options, &result);
        break;
#if 0
    case LYD_JSON:
        lyd_parse_json(ctx, data, options, trees, &result);
        break;
    case LYD_LYB:
        lyd_parse_lyb(ctx, data, options, trees, &result);
        break;
#endif
    case LYD_SCHEMA:
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

    if (!keys_str) {
        /* nothing to parse */
        return LY_SUCCESS;
    }

    keys->str = strndup(keys_str, keys_len);
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
        LY_CHECK_GOTO(ret = lyd_create_term((struct lysc_node *)keys.keys[i].schema, keys.keys[i].value,
                                            strlen(keys.keys[i].value), NULL, lydjson_resolve_prefix, NULL, LYD_JSON, &key), cleanup);
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

LY_ERR
lyd_create_opaq(const struct ly_ctx *ctx, const char *name, size_t name_len, const char *value, size_t value_len,
                int *dynamic, LYD_FORMAT format, struct ly_prefix *val_prefs, const char *prefix, size_t pref_len,
                const char *ns, struct lyd_node **node)
{
    struct lyd_node_opaq *opaq;

    assert(ctx && name && name_len && ns);

    if (!value_len) {
        value = "";
    }

    opaq = calloc(1, sizeof *opaq);
    LY_CHECK_ERR_RET(!opaq, LOGMEM(ctx), LY_EMEM);

    opaq->prev = (struct lyd_node *)opaq;

    opaq->name = lydict_insert(ctx, name, name_len);
    opaq->format = format;
    if (pref_len) {
        opaq->prefix.pref = lydict_insert(ctx, prefix, pref_len);
    }
    opaq->prefix.ns = lydict_insert(ctx, ns, 0);
    opaq->val_prefs = val_prefs;
    if (dynamic && *dynamic) {
        opaq->value = lydict_insert_zc(ctx, (char *)value);
        *dynamic = 0;
    } else {
        opaq->value = lydict_insert(ctx, value, value_len);
    }
    opaq->ctx = ctx;

    *node = (struct lyd_node *)opaq;
    return LY_SUCCESS;
}

API struct lyd_node *
lyd_new_inner(struct lyd_node *parent, const struct lys_module *module, const char *name)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, name, NULL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_CONTAINER | LYS_NOTIF | LYS_ACTION, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Inner node \"%s\" not found.", name), NULL);

    if (!lyd_create_inner(schema, &ret) && parent) {
        lyd_insert_node(parent, NULL, ret);
    }
    return ret;
}

API struct lyd_node *
lyd_new_list(struct lyd_node *parent, const struct lys_module *module, const char *name, ...)
{
    struct lyd_node *ret = NULL, *key;
    const struct lysc_node *schema, *key_s;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);
    va_list ap;
    const char *key_val;
    LY_ERR rc = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, parent || module, name, NULL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_LIST, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found.", name), NULL);

    /* create list inner node */
    LY_CHECK_RET(lyd_create_inner(schema, &ret), NULL);

    va_start(ap, name);

    /* create and insert all the keys */
    for (key_s = lysc_node_children(schema, 0); key_s && (key_s->flags & LYS_KEY); key_s = key_s->next) {
        key_val = va_arg(ap, const char *);

        rc = lyd_create_term(key_s, key_val, key_val ? strlen(key_val) : 0, NULL, lydjson_resolve_prefix, NULL, LYD_JSON, &key);
        LY_CHECK_GOTO(rc, cleanup);
        lyd_insert_node(ret, NULL, key);
    }

    /* hash having all the keys */
    lyd_hash(ret);

    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }

cleanup:
    if (rc) {
        lyd_free_tree(ret);
        ret = NULL;
    }
    va_end(ap);
    return ret;
}

API struct lyd_node *
lyd_new_list2(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *keys)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, name, NULL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_LIST, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found.", name), NULL);

    if (!lyd_create_list(schema, keys, keys ? strlen(keys) : 0, &ret) && parent) {
        lyd_insert_node(parent, NULL, ret);
    }
    return ret;
}

API struct lyd_node *
lyd_new_term(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, name, NULL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYD_NODE_TERM, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Term node \"%s\" not found.", name), NULL);

    if (!lyd_create_term(schema, val_str, val_str ? strlen(val_str) : 0, NULL, lydjson_resolve_prefix, NULL, LYD_JSON, &ret)
            && parent) {
        lyd_insert_node(parent, NULL, ret);
    }
    return ret;
}

API struct lyd_node *
lyd_new_any(struct lyd_node *parent, const struct lys_module *module, const char *name, const void *value,
            LYD_ANYDATA_VALUETYPE value_type)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, name, NULL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYD_NODE_ANY, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Any node \"%s\" not found.", name), NULL);

    if (!lyd_create_any(schema, value, value_type, &ret) && parent) {
        lyd_insert_node(parent, NULL, ret);
    }
    return ret;
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
 * Handles inserting into NP containers and key-less lists.
 *
 * @param[in] sibling Sibling to insert after.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_after_node(struct lyd_node *sibling, struct lyd_node *node)
{
    struct lyd_node_inner *par;

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

    for (par = node->parent; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
        if ((par->schema->nodetype == LYS_LIST) && (par->schema->flags & LYS_KEYLESS)) {
            /* rehash key-less list */
            lyd_hash((struct lyd_node *)par);
        }
    }

    /* insert into hash table */
    lyd_insert_hash(node);
}

/**
 * @brief Insert node before a sibling.
 *
 * Handles inserting into NP containers and key-less lists.
 *
 * @param[in] sibling Sibling to insert before.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_before_node(struct lyd_node *sibling, struct lyd_node *node)
{
    struct lyd_node_inner *par;

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

    for (par = node->parent; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
        if ((par->schema->nodetype == LYS_LIST) && (par->schema->flags & LYS_KEYLESS)) {
            /* rehash key-less list */
            lyd_hash((struct lyd_node *)par);
        }
    }

    /* insert into hash table */
    lyd_insert_hash(node);
}

/**
 * @brief Insert node as the last child of a parent.
 *
 * Handles inserting into NP containers and key-less lists.
 *
 * @param[in] parent Parent to insert into.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_last_node(struct lyd_node *parent, struct lyd_node *node)
{
    struct lyd_node_inner *par;

    assert(parent && !node->next && (node->prev == node));
    assert(!parent->schema || (parent->schema->nodetype & LYD_NODE_INNER));

    par = (struct lyd_node_inner *)parent;

    if (!par->child) {
        par->child = node;
    } else {
        node->prev = par->child->prev;
        par->child->prev->next = node;
        par->child->prev = node;
    }
    node->parent = par;

    for (; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
        if (par->schema && (par->schema->nodetype == LYS_LIST) && (par->schema->flags & LYS_KEYLESS)) {
            /* rehash key-less list */
            lyd_hash((struct lyd_node *)par);
        }
    }

    /* insert into hash table */
    lyd_insert_hash(node);
}

void
lyd_insert_node(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node *node)
{
    struct lyd_node *anchor;
    const struct lysc_node *skey = NULL;
    int has_keys;

    assert((parent || first_sibling) && node && (node->hash || !node->schema));

    if (!parent && first_sibling && (*first_sibling) && (*first_sibling)->parent) {
        parent = (struct lyd_node *)(*first_sibling)->parent;
    }

    if (parent) {
        if (node->schema && (node->schema->flags & LYS_KEY)) {
            /* it is key and we need to insert it at the correct place */
            anchor = lyd_get_prev_key_anchor(lyd_node_children(parent), node->schema);
            if (anchor) {
                lyd_insert_after_node(anchor, node);
            } else if (lyd_node_children(parent)) {
                lyd_insert_before_node((struct lyd_node *)lyd_node_children(parent), node);
            } else {
                lyd_insert_last_node(parent, node);
            }

            /* hash list if all its keys were added */
            assert(parent->schema->nodetype == LYS_LIST);
            anchor = (struct lyd_node *)lyd_node_children(parent);
            has_keys = 1;
            while ((skey = lys_getnext(skey, parent->schema, NULL, 0)) && (skey->flags & LYS_KEY)) {
                if (!anchor || (anchor->schema != skey)) {
                    /* key missing */
                    has_keys = 0;
                    break;
                }

                anchor = anchor->next;
            }
            if (has_keys) {
                lyd_hash(parent);
            }

        } else {
            /* last child */
            lyd_insert_last_node(parent, node);
        }
    } else if (*first_sibling) {
        /* top-level siblings */
        anchor = (*first_sibling)->prev;
        while (anchor->prev->next && (lyd_owner_module(anchor) != lyd_owner_module(node))) {
            anchor = anchor->prev;
        }

        if (lyd_owner_module(anchor) == lyd_owner_module(node)) {
            /* insert after last sibling from this module */
            lyd_insert_after_node(anchor, node);
        } else {
            /* no data from this module, insert at the last position */
            lyd_insert_after_node((*first_sibling)->prev, node);
        }
    } else {
        /* the only sibling */
        *first_sibling = node;
    }
}

static LY_ERR
lyd_insert_check_schema(const struct lysc_node *parent, const struct lysc_node *schema)
{
    const struct lysc_node *par2;

    assert(schema);

    /* adjust parent first */
    while (parent && (parent->nodetype & (LYS_CASE | LYS_CHOICE))) {
        parent = parent->parent;
    }

    /* find schema parent */
    for (par2 = schema->parent; par2 && (par2->nodetype & (LYS_CASE | LYS_CHOICE)); par2 = par2->parent);

    if (parent) {
        /* inner node */
        if (par2 != parent) {
            LOGERR(parent->module->ctx, LY_EINVAL, "Cannot insert, parent of \"%s\" is not \"%s\".", schema->name, parent->name);
            return LY_EINVAL;
        }
    } else {
        /* top-level node */
        if (par2) {
            LOGERR(parent->module->ctx, LY_EINVAL, "Cannot insert, node \"%s\" is not top-level.", schema->name);
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_insert(struct lyd_node *parent, struct lyd_node *node)
{
    struct lyd_node *iter;

    LY_CHECK_ARG_RET(NULL, parent, node, !(parent->schema->nodetype & LYD_NODE_INNER), LY_EINVAL);

    LY_CHECK_RET(lyd_insert_check_schema(parent->schema, node->schema));

    if (node->schema->flags & LYS_KEY) {
        LOGERR(parent->schema->module->ctx, LY_EINVAL, "Cannot insert key \"%s\".", node->schema->name);
        return LY_EINVAL;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink_tree(node);
    }

    while (node) {
        iter = node->next;
        lyd_unlink_tree(node);
        lyd_insert_node(parent, NULL, node);
        node = iter;
    }
    return LY_SUCCESS;
}

API LY_ERR
lyd_insert_sibling(struct lyd_node *sibling, struct lyd_node *node)
{
    struct lyd_node *iter;

    LY_CHECK_ARG_RET(NULL, sibling, node, LY_EINVAL);

    LY_CHECK_RET(lyd_insert_check_schema(sibling->schema->parent, node->schema));

    if (node->schema->flags & LYS_KEY) {
        LOGERR(sibling->schema->module->ctx, LY_EINVAL, "Cannot insert key \"%s\".", node->schema->name);
        return LY_EINVAL;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink_tree(node);
    }

    while (node) {
        iter = node->next;
        lyd_unlink_tree(node);
        lyd_insert_node(NULL, &sibling, node);
        node = iter;
    }
    return LY_SUCCESS;
}

static LY_ERR
lyd_insert_after_check_place(struct lyd_node *anchor, struct lyd_node *sibling, struct lyd_node *node)
{
    if (sibling->parent) {
        /* nested, we do not care for the order */
        return LY_SUCCESS;
    }

    if (anchor) {
        if (anchor->next && (lyd_owner_module(anchor) == lyd_owner_module(anchor->next))
                && (lyd_owner_module(node) != lyd_owner_module(anchor))) {
            LOGERR(sibling->schema->module->ctx, LY_EINVAL, "Cannot insert top-level module \"%s\" data into module \"%s\" data.",
                   lyd_owner_module(node)->name, lyd_owner_module(anchor)->name);
            return LY_EINVAL;
        }

        if ((lyd_owner_module(node) == lyd_owner_module(anchor))
                || (anchor->next && (lyd_owner_module(node) == lyd_owner_module(anchor->next)))) {
            /* inserting before/after its module data */
            return LY_SUCCESS;
        }
    }

    /* find first sibling */
    while (sibling->prev->next) {
        sibling = sibling->prev;
    }

    if (!anchor) {
        if (lyd_owner_module(node) == lyd_owner_module(sibling)) {
            /* inserting before its module data */
            return LY_SUCCESS;
        }
    }

    /* check there are no data of this module */
    LY_LIST_FOR(sibling, sibling) {
        if (lyd_owner_module(node) == lyd_owner_module(sibling)) {
            /* some data of this module found */
            LOGERR(sibling->schema->module->ctx, LY_EINVAL, "Top-level data of module \"%s\" already exist,"
                   " they must be directly connected.", lyd_owner_module(node)->name);
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node)
{
    struct lyd_node *iter;

    LY_CHECK_ARG_RET(NULL, sibling, node, LY_EINVAL);

    LY_CHECK_RET(lyd_insert_check_schema(sibling->schema->parent, node->schema));

    if (node->schema->flags & LYS_KEY) {
        LOGERR(sibling->schema->module->ctx, LY_EINVAL, "Cannot insert key \"%s\".", node->schema->name);
        return LY_EINVAL;
    } else if (sibling->schema->flags & LYS_KEY) {
        LOGERR(sibling->schema->module->ctx, LY_EINVAL, "Cannot insert into keys.");
        return LY_EINVAL;
    }

    LY_CHECK_RET(lyd_insert_after_check_place(sibling->prev->next ? sibling->prev : NULL, sibling, node));

    if (node->parent || node->prev->next) {
        lyd_unlink_tree(node);
    }

    /* insert in reverse order to get the original order */
    node = node->prev;
    while (node) {
        iter = node->prev;
        lyd_unlink_tree(node);

        lyd_insert_before_node(sibling, node);
        /* move the anchor accordingly */
        sibling = node;

        node = (iter == node) ? NULL : iter;
    }
    return LY_SUCCESS;
}

API LY_ERR
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node)
{
    struct lyd_node *iter;

    LY_CHECK_ARG_RET(NULL, sibling, node, LY_EINVAL);

    LY_CHECK_RET(lyd_insert_check_schema(sibling->schema->parent, node->schema));

    if (node->schema->flags & LYS_KEY) {
        LOGERR(sibling->schema->module->ctx, LY_EINVAL, "Cannot insert key \"%s\".", node->schema->name);
        return LY_EINVAL;
    } else if (sibling->next && (sibling->next->schema->flags & LYS_KEY)) {
        LOGERR(sibling->schema->module->ctx, LY_EINVAL, "Cannot insert into keys.");
        return LY_EINVAL;
    }

    LY_CHECK_RET(lyd_insert_after_check_place(sibling, sibling, node));

    if (node->parent || node->prev->next) {
        lyd_unlink_tree(node);
    }

    while (node) {
        iter = node->next;
        lyd_unlink_tree(node);

        lyd_insert_after_node(sibling, node);
        /* move the anchor accordingly */
        sibling = node;

        node = iter;
    }
    return LY_SUCCESS;
}

API void
lyd_unlink_tree(struct lyd_node *node)
{
    struct lyd_node *iter;

    if (!node) {
        return;
    }

    /* unlink from siblings */
    if (node->prev->next) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last node */
        if (node->parent) {
            iter = node->parent->child;
        } else {
            iter = node->prev;
            while (iter->prev != node) {
                iter = iter->prev;
            }
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

        lyd_unlink_hash(node);

        /* check for keyless list and update its hash */
        for (iter = (struct lyd_node *)node->parent; iter; iter = (struct lyd_node *)iter->parent) {
            if (iter->schema->flags & LYS_KEYLESS) {
                lyd_hash(iter);
            }
        }

        node->parent = NULL;
    }

    node->next = NULL;
    node->prev = node;
}

LY_ERR
lyd_create_meta(struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod, const char *name,
                size_t name_len, const char *value, size_t value_len, int *dynamic, ly_clb_resolve_prefix resolve_prefix,
                void *prefix_data, LYD_FORMAT format, const struct lysc_node *ctx_snode)
{
    LY_ERR ret;
    struct lysc_ext_instance *ant = NULL;
    struct lyd_meta *mt, *last;
    uint32_t v;

    assert((parent || meta) && mod);

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

    mt = calloc(1, sizeof *mt);
    LY_CHECK_ERR_RET(!mt, LOGMEM(mod->ctx), LY_EMEM);
    mt->parent = parent;
    mt->annotation = ant;
    ret = lyd_value_parse_meta(mod->ctx, mt, value, value_len, dynamic, 0, resolve_prefix, prefix_data, format, ctx_snode, NULL);
    if ((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE)) {
        free(mt);
        return ret;
    }
    mt->name = lydict_insert(mod->ctx, name, name_len);

    /* insert as the last attribute */
    if (parent) {
        if (parent->meta) {
            for (last = parent->meta; last->next; last = last->next);
            last->next = mt;
        } else {
            parent->meta = mt;
        }
    } else if (*meta) {
        for (last = *meta; last->next; last = last->next);
        last->next = mt;
    }

    /* remove default flags from NP containers */
    while (parent && (parent->flags & LYD_DEFAULT)) {
        parent->flags &= ~LYD_DEFAULT;
        parent = (struct lyd_node *)parent->parent;
    }

    if (meta) {
        *meta = mt;
    }
    return ret;
}

LY_ERR
ly_create_attr(struct lyd_node *parent, struct ly_attr **attr, const struct ly_ctx *ctx, const char *name,
               size_t name_len, const char *value, size_t value_len, int *dynamic, LYD_FORMAT format,
               struct ly_prefix *val_prefs, const char *prefix, size_t prefix_len, const char *ns)
{
    struct ly_attr *at, *last;
    struct lyd_node_opaq *opaq;

    assert(ctx && (parent || attr) && (!parent || !parent->schema));
    assert(name && name_len);
    assert((prefix_len && ns) || (!prefix_len && !ns));

    if (!value_len) {
        value = "";
    }

    at = calloc(1, sizeof *at);
    LY_CHECK_ERR_RET(!at, LOGMEM(ctx), LY_EMEM);
    at->parent = (struct lyd_node_opaq *)parent;
    at->name = lydict_insert(ctx, name, name_len);
    if (dynamic && *dynamic) {
        at->value = lydict_insert_zc(ctx, (char *)value);
        *dynamic = 0;
    } else {
        at->value = lydict_insert(ctx, value, value_len);
    }

    at->format = format;
    at->val_prefs = val_prefs;
    if (ns) {
        at->prefix.pref = lydict_insert(ctx, prefix, prefix_len);
        at->prefix.ns = lydict_insert(ctx, ns, 0);
    }

    /* insert as the last attribute */
    if (parent) {
        opaq = (struct lyd_node_opaq *)parent;
        if (opaq->attr) {
            for (last = opaq->attr; last->next; last = last->next);
            last->next = at;
        } else {
            opaq->attr = at;
        }
    } else if (*attr) {
        for (last = *attr; last->next; last = last->next);
        last->next = at;
    }

    if (attr) {
        *attr = at;
    }
    return LY_SUCCESS;
}

API const struct lyd_node_term *
lyd_target(struct lyd_value_path *path, const struct lyd_node *tree)
{
    unsigned int u, x;
    const struct lyd_node *parent = NULL, *start_search;
    struct lyd_node *node = NULL;
    uint64_t pos = 1;

    LY_CHECK_ARG_RET(NULL, path, tree, NULL);

    LY_ARRAY_FOR(path, u) {
        if (parent) {
            start_search = lyd_node_children(parent);
search_inner:
            lyd_find_sibling_next(start_search, path[u].node->module, path[u].node->name, 0, NULL, 0, &node);
        } else {
            start_search = tree;
search_toplevel:
            /* WARNING! to use search_toplevel label correctly, variable v must be preserved and not changed! */
            lyd_find_sibling_next(start_search, path[u].node->module, path[u].node->name, 0, NULL, 0, &node);
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
    struct lyd_node_opaq *opaq1, *opaq2;
    struct lysc_type *type;
    size_t len1, len2;

    if (!node1 || !node2) {
        if (node1 == node2) {
            return LY_SUCCESS;
        } else {
            return LY_ENOT;
        }
    }

    if ((LYD_NODE_CTX(node1) != LYD_NODE_CTX(node2)) || (node1->schema != node2->schema)) {
        return LY_ENOT;
    }

    if (node1->hash != node2->hash) {
        return LY_ENOT;
    }
    /* equal hashes do not mean equal nodes, they can be just in collision (or both be 0) so the nodes must be checked explicitly */

    if (!node1->schema) {
        opaq1 = (struct lyd_node_opaq *)node1;
        opaq2 = (struct lyd_node_opaq *)node2;
        if ((opaq1->name != opaq2->name) || (opaq1->prefix.ns != opaq2->prefix.ns) || (opaq1->format != opaq2->format)) {
            return LY_ENOT;
        }
        switch (opaq1->format) {
        case LYD_XML:
            if (lyxml_value_compare(opaq1->value, opaq1->val_prefs, opaq2->value, opaq2->val_prefs)) {
                return LY_ENOT;
            }
            break;
        case LYD_SCHEMA:
            /* not allowed */
            LOGINT(LYD_NODE_CTX(node1));
            return LY_EINT;
        }
        if (options & LYD_COMPARE_FULL_RECURSION) {
            iter1 = opaq1->child;
            iter2 = opaq2->child;
            goto all_children_compare;
        }
        return LY_SUCCESS;
    } else {
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
    }

    LOGINT(LYD_NODE_CTX(node1));
    return LY_EINT;
}

/**
 * @brief Duplicate a single node and connect it into @p parent (if present) or last of @p first siblings.
 *
 * Ignores LYD_DUP_WITH_PARENTS and LYD_DUP_WITH_SIBLINGS which are supposed to be handled by lyd_dup().
 */
static LY_ERR
lyd_dup_recursive(const struct lyd_node *node, struct lyd_node *parent, struct lyd_node **first, int options,
                  struct lyd_node **dup_p)
{
    LY_ERR ret;
    struct lyd_node *dup = NULL;
    uint32_t u;

    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);

    if (!node->schema) {
        dup = calloc(1, sizeof(struct lyd_node_opaq));
    } else {
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
            LOGINT(LYD_NODE_CTX(node));
            ret = LY_EINT;
            goto error;
        }
    }
    LY_CHECK_ERR_GOTO(!dup, LOGMEM(LYD_NODE_CTX(node)); ret = LY_EMEM, error);

    /* TODO implement LYD_DUP_WITH_WHEN */
    dup->flags = node->flags;
    dup->schema = node->schema;
    dup->prev = dup;

    /* TODO duplicate attributes, implement LYD_DUP_NO_ATTR */

    /* nodetype-specific work */
    if (!dup->schema) {
        struct lyd_node_opaq *opaq = (struct lyd_node_opaq *)dup;
        struct lyd_node_opaq *orig = (struct lyd_node_opaq *)node;
        struct lyd_node *child;

        if (options & LYD_DUP_RECURSIVE) {
            /* duplicate all the children */
            LY_LIST_FOR(orig->child, child) {
                LY_CHECK_GOTO(ret = lyd_dup_recursive(child, dup, NULL, options, NULL), error);
            }
        }
        opaq->name = lydict_insert(LYD_NODE_CTX(node), orig->name, 0);
        opaq->format = orig->format;
        if (orig->prefix.pref) {
            opaq->prefix.pref = lydict_insert(LYD_NODE_CTX(node), orig->prefix.pref, 0);
        }
        if (orig->prefix.ns) {
            opaq->prefix.ns = lydict_insert(LYD_NODE_CTX(node), orig->prefix.ns, 0);
        }
        if (orig->val_prefs) {
            LY_ARRAY_CREATE_GOTO(LYD_NODE_CTX(node), opaq->val_prefs, LY_ARRAY_SIZE(orig->val_prefs), ret, error);
            LY_ARRAY_FOR(orig->val_prefs, u) {
                opaq->val_prefs[u].pref = lydict_insert(LYD_NODE_CTX(node), orig->val_prefs[u].pref, 0);
                opaq->val_prefs[u].ns = lydict_insert(LYD_NODE_CTX(node), orig->val_prefs[u].ns, 0);
                LY_ARRAY_INCREMENT(opaq->val_prefs);
            }
        }
        opaq->value = lydict_insert(LYD_NODE_CTX(node), orig->value, 0);
        opaq->ctx = orig->ctx;
    } else if (dup->schema->nodetype & LYD_NODE_TERM) {
        struct lyd_node_term *term = (struct lyd_node_term *)dup;
        struct lyd_node_term *orig = (struct lyd_node_term *)node;

        term->hash = orig->hash;
        term->value.realtype = orig->value.realtype;
        LY_CHECK_ERR_GOTO(term->value.realtype->plugin->duplicate(LYD_NODE_CTX(node), &orig->value, &term->value),
                          LOGERR(LYD_NODE_CTX(node), LY_EINT, "Value duplication failed."); ret = LY_EINT, error);
    } else if (dup->schema->nodetype & LYD_NODE_INNER) {
        struct lyd_node_inner *orig = (struct lyd_node_inner *)node;
        struct lyd_node *child;

        if (options & LYD_DUP_RECURSIVE) {
            /* duplicate all the children */
            LY_LIST_FOR(orig->child, child) {
                LY_CHECK_GOTO(ret = lyd_dup_recursive(child, dup, NULL, options, NULL), error);
            }
        } else if (dup->schema->nodetype == LYS_LIST && !(dup->schema->flags & LYS_KEYLESS)) {
            /* always duplicate keys of a list */
            child = orig->child;
            for (struct lysc_node *key = ((struct lysc_node_list *)dup->schema)->child;
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
                LY_CHECK_GOTO(ret = lyd_dup_recursive(child, dup, NULL, options, NULL), error);
                child = child->next;
            }
        }
        lyd_hash(dup);
    } else if (dup->schema->nodetype & LYD_NODE_ANY) {
        struct lyd_node_any *any = (struct lyd_node_any *)dup;
        struct lyd_node_any *orig = (struct lyd_node_any *)node;

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
                any->value.str = lydict_insert(LYD_NODE_CTX(node), orig->value.str, strlen(orig->value.str));
            }
            break;
        }
    }

    /* insert */
    lyd_insert_node(parent, first, dup);
    lyd_insert_hash(dup);

    if (dup_p) {
        *dup_p = dup;
    }
    return LY_SUCCESS;

error:
    lyd_free_tree(dup);
    return ret;
}

API struct lyd_node *
lyd_dup(const struct lyd_node *node, struct lyd_node_inner *parent, int options)
{
    struct ly_ctx *ctx;
    const struct lyd_node *orig;          /* original node to be duplicated */
    struct lyd_node *first = NULL;        /* the first duplicated node, this is returned */
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
                iter = NULL;
                LY_CHECK_GOTO(lyd_dup_recursive((struct lyd_node *)orig_parent, NULL, (struct lyd_node **)&iter, 0,
                                                (struct lyd_node **)&iter), error);
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

    LY_LIST_FOR(node, orig) {
        /* if there is no local parent, it will be inserted into first */
        LY_CHECK_GOTO(lyd_dup_recursive(orig, (struct lyd_node *)local_parent, &first, options, first ? NULL : &first), error);
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
        lyd_free_siblings(first);
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

    LY_CHECK_ARG_RET(NULL, target, LY_EINVAL);

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

    if (key_or_value && !val_len) {
        val_len = strlen(key_or_value);
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
