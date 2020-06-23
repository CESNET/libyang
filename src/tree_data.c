/**
 * @file tree_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data tree functions
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "tree_data.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "log.h"
#include "path.h"
#include "plugins_exts.h"
#include "plugins_exts_metadata.h"
#include "plugins_exts_internal.h"
#include "plugins_types.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

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
                              tree ? (void *)node : (void *)node->schema, tree, &node->value, NULL, &err);
    if (ret && (ret != LY_EINCOMPLETE)) {
        if (err) {
            /* node may not be connected yet so use the schema node */
            if (!node->parent && lysc_data_parent(node->schema)) {
                LOGVAL(ctx, LY_VLOG_LYSC, node->schema, err->vecode, err->msg);
            } else {
                LOGVAL(ctx, LY_VLOG_LYD, node, err->vecode, err->msg);
            }
            ly_err_free(err);
        }
        goto error;
    } else if (dynamic) {
        *dynamic = 0;
    }

error:
    return ret;
}

/* similar to lyd_value_parse except can be used just to store the value, hence also does not support a second call */
LY_ERR
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
lys_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len,
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
lyd_value_validate(const struct ly_ctx *ctx, const struct lyd_node_term *node, const char *value, size_t value_len,
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
    assert(val && val->realtype);

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
    term->value.realtype = val->realtype;
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

LY_ERR
lyd_create_list(const struct lysc_node *schema, const struct ly_path_predicate *predicates, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *list = NULL, *key;
    LY_ARRAY_SIZE_TYPE u;

    assert((schema->nodetype == LYS_LIST) && !(schema->flags & LYS_KEYLESS));

    /* create list */
    LY_CHECK_GOTO(ret = lyd_create_inner(schema, &list), cleanup);

    /* create and insert all the keys */
    LY_ARRAY_FOR(predicates, u) {
        LY_CHECK_GOTO(ret = lyd_create_term2(predicates[u].key, &predicates[u].value, &key), cleanup);
        lyd_insert_node(list, NULL, key);
    }

    /* hash having all the keys */
    lyd_hash(list);

    /* success */
    *node = list;
    list = NULL;

cleanup:
    lyd_free_tree(list);
    return ret;
}

static LY_ERR
lyd_create_list2(const struct lysc_node *schema, const char *keys, size_t keys_len, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *expr = NULL;
    uint16_t exp_idx = 0;
    enum ly_path_pred_type pred_type = 0;
    struct ly_path_predicate *predicates = NULL;

    /* parse keys */
    LY_CHECK_GOTO(ret = ly_path_parse_predicate(schema->module->ctx, keys, keys_len, LY_PATH_PREFIX_OPTIONAL,
                                                LY_PATH_PRED_KEYS, &expr), cleanup);

    /* compile them */
    LY_CHECK_GOTO(ret = ly_path_compile_predicate(schema->module->ctx, NULL, schema, expr, &exp_idx, lydjson_resolve_prefix,
                                                  NULL, LYD_JSON, &predicates, &pred_type), cleanup);

    /* create the list node */
    LY_CHECK_GOTO(ret = lyd_create_list(schema, predicates, node), cleanup);

cleanup:
    lyxp_expr_free(schema->module->ctx, expr);
    ly_path_predicates_free(schema->module->ctx, pred_type, NULL, predicates);
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

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_CONTAINER | LYS_NOTIF | LYS_RPC | LYS_ACTION, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Inner node (and not a list) \"%s\" not found.", name), NULL);

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
        LY_CHECK_GOTO(rc && (rc != LY_EINCOMPLETE), cleanup);
        rc = LY_SUCCESS;
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
    if (!keys) {
        keys = "";
    }

    /* find schema node */
    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_LIST, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found.", name), NULL);

    if ((schema->flags & LYS_KEYLESS) && !keys[0]) {
        /* key-less list */
        LY_CHECK_RET(lyd_create_inner(schema, &ret), NULL);
    } else {
        /* create the list node */
        LY_CHECK_RET(lyd_create_list2(schema, keys, strlen(keys), &ret), NULL);
    }

    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }
    return ret;
}

API struct lyd_node *
lyd_new_term(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    LY_ERR rc;
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, name, NULL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYD_NODE_TERM, 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Term node \"%s\" not found.", name), NULL);

    rc = lyd_create_term(schema, val_str, val_str ? strlen(val_str) : 0, NULL, lydjson_resolve_prefix, NULL, LYD_JSON, &ret);
    LY_CHECK_RET(rc && (rc != LY_EINCOMPLETE), NULL);

    if (parent) {
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

/**
 * @brief Update node value.
 *
 * @param[in] node Node to update.
 * @param[in] value New value to set.
 * @param[in] value_type Type of @p value for any node.
 * @param[out] new_parent Set to @p node if the value was updated, otherwise set to NULL.
 * @param[out] new_node Set to @p node if the value was updated, otherwise set to NULL.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_new_path_update(struct lyd_node *node, const void *value, LYD_ANYDATA_VALUETYPE value_type,
                    struct lyd_node **new_parent, struct lyd_node **new_node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *new_any;

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_LIST:
    case LYS_LEAFLIST:
        /* if it exists, there is nothing to update */
        *new_parent = NULL;
        *new_node = NULL;
        break;
    case LYS_LEAF:
        ret = lyd_change_term(node, value);
        if ((ret == LY_SUCCESS) || (ret == LY_EEXIST)) {
            /* there was an actual change (at least of the default flag) */
            *new_parent = node;
            *new_node = node;
            ret = LY_SUCCESS;
        } else if (ret == LY_ENOT) {
            /* no change */
            *new_parent = NULL;
            *new_node = NULL;
            ret = LY_SUCCESS;
        } /* else error */
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        /* create a new any node */
        LY_CHECK_RET(lyd_create_any(node->schema, value, value_type, &new_any));

        /* compare with the existing one */
        if (lyd_compare(node, new_any, 0)) {
            /* not equal, switch values (so that we can use generic node free) */
            ((struct lyd_node_any *)new_any)->value = ((struct lyd_node_any *)node)->value;
            ((struct lyd_node_any *)new_any)->value_type = ((struct lyd_node_any *)node)->value_type;
            ((struct lyd_node_any *)node)->value.str = value;
            ((struct lyd_node_any *)node)->value_type = value_type;

            *new_parent = node;
            *new_node = node;
        } else {
            /* they are equal */
            *new_parent = NULL;
            *new_node = NULL;
        }
        lyd_free_tree(new_any);
        break;
    default:
        LOGINT(LYD_NODE_CTX(node));
        ret = LY_EINT;
        break;
    }

    return ret;
}

API struct lyd_meta *
lyd_new_meta(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    struct lyd_meta *ret = NULL;
    const struct ly_ctx *ctx;
    const char *prefix, *tmp;
    char *str;
    size_t pref_len, name_len;

    LY_CHECK_ARG_RET(NULL, parent, name, module || strchr(name, ':'), NULL);

    ctx = LYD_NODE_CTX(parent);

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Metadata name \"%s\" is not valid.", name);
        return NULL;
    }

    /* find the module */
    if (prefix) {
        str = strndup(name, name_len);
        module = ly_ctx_get_module_implemented(ctx, str);
        free(str);
        LY_CHECK_ERR_RET(!module, LOGERR(ctx, LY_EINVAL, "Module \"%.*s\" not found.", pref_len, prefix), NULL);
    }

    /* set value if none */
    if (!val_str) {
        val_str = "";
    }

    lyd_create_meta(parent, &ret, module, name, name_len, val_str, strlen(val_str), NULL, lydjson_resolve_prefix, NULL,
                    LYD_JSON, parent->schema);
    return ret;
}

API struct lyd_node *
lyd_new_opaq(struct lyd_node *parent, const struct ly_ctx *ctx, const char *name, const char *value,
             const char *module_name)
{
    struct lyd_node *ret = NULL;

    LY_CHECK_ARG_RET(ctx, parent || ctx, name, module_name, NULL);

    if (!ctx) {
        ctx = LYD_NODE_CTX(parent);
    }
    if (!value) {
        value = "";
    }

    if (!lyd_create_opaq(ctx, name, strlen(name), value, strlen(value), NULL, LYD_JSON, NULL, NULL, 0, module_name, &ret)
            && parent) {
        lyd_insert_node(parent, NULL, ret);
    }
    return ret;
}

API struct ly_attr *
lyd_new_attr(struct lyd_node *parent, const char *module_name, const char *name, const char *val_str)
{
    struct ly_attr *ret = NULL;
    const struct ly_ctx *ctx;
    const char *prefix, *tmp;
    size_t pref_len, name_len;

    LY_CHECK_ARG_RET(NULL, parent, !parent->schema, name, NULL);

    ctx = LYD_NODE_CTX(parent);

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Metadata name \"%s\" is not valid.", name);
        return NULL;
    }

    /* set value if none */
    if (!val_str) {
        val_str = "";
    }

    ly_create_attr(parent, &ret, ctx, name, name_len, val_str, strlen(val_str), NULL, LYD_JSON, NULL, prefix,
                   pref_len, module_name);
    return ret;
}

API LY_ERR
lyd_change_term(struct lyd_node *term, const char *val_str)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type *type;
    struct lyd_node_term *t;
    struct lyd_node *parent;
    struct lyd_value val = {0};
    int dflt_change, val_change;

    LY_CHECK_ARG_RET(NULL, term, term->schema, term->schema->nodetype & LYD_NODE_TERM, LY_EINVAL);

    if (!val_str) {
        val_str = "";
    }
    t = (struct lyd_node_term *)term;
    type = ((struct lysc_node_leaf *)term->schema)->type;

    /* parse the new value */
    LY_CHECK_GOTO(ret = lyd_value_store(&val, term->schema, val_str, strlen(val_str), NULL, lydjson_resolve_prefix, NULL,
                                        LYD_JSON), cleanup);

    /* compare original and new value */
    if (type->plugin->compare(&t->value, &val)) {
        /* values differ, switch them */
        type->plugin->free(LYD_NODE_CTX(term), &t->value);
        t->value = val;
        memset(&val, 0, sizeof val);
        val_change = 1;
    } else {
        val_change = 0;
    }

    /* always clear the default flag */
    if (term->flags & LYD_DEFAULT) {
        for (parent = term; parent; parent = (struct lyd_node *)parent->parent) {
            parent->flags &= ~LYD_DEFAULT;
        }
        dflt_change = 1;
    } else {
        dflt_change = 0;
    }

    if (val_change || dflt_change) {
        /* make the node non-validated */
        term->flags &= LYD_NEW;
    }

    if (val_change) {
        if (term->schema->nodetype == LYS_LEAFLIST) {
            /* leaf-list needs to be hashed again and re-inserted into parent */
            lyd_unlink_hash(term);
            lyd_hash(term);
            LY_CHECK_GOTO(ret = lyd_insert_hash(term), cleanup);
        } else if ((term->schema->flags & LYS_KEY) && term->parent) {
            /* list needs to be updated if its key was changed */
            assert(term->parent->schema->nodetype == LYS_LIST);
            lyd_unlink_hash((struct lyd_node *)term->parent);
            lyd_hash((struct lyd_node *)term->parent);
            LY_CHECK_GOTO(ret = lyd_insert_hash((struct lyd_node *)term->parent), cleanup);
        } /* else leaf that is not a key, its value is not used for its hash so it does not change */
    }

    /* retrun value */
    if (!val_change) {
        if (dflt_change) {
            /* only default flag change */
            ret = LY_EEXIST;
        } else {
            /* no change */
            ret = LY_ENOT;
        }
    } /* else value changed, LY_SUCCESS */

cleanup:
    type->plugin->free(LYD_NODE_CTX(term), &val);
    return ret;
}

API struct lyd_node *
lyd_new_path(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const char *value, int options)
{
    struct lyd_node *new_parent = NULL;

    lyd_new_path2(parent, ctx, path, value, 0, options, &new_parent, NULL);
    return new_parent;
}

API struct lyd_node *
lyd_new_path_any(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const void *value,
                 LYD_ANYDATA_VALUETYPE value_type, int options)
{
    struct lyd_node *new_parent = NULL;

    lyd_new_path2(parent, ctx, path, value, value_type, options, &new_parent, NULL);
    return new_parent;
}

API LY_ERR
lyd_new_path2(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const void *value,
              LYD_ANYDATA_VALUETYPE value_type, int options, struct lyd_node **new_parent, struct lyd_node **new_node)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lyxp_expr *exp = NULL;
    struct ly_path *p = NULL;
    struct lyd_node *nparent = NULL, *nnode = NULL, *node = NULL, *cur_parent;
    const struct lysc_node *schema;
    LY_ARRAY_SIZE_TYPE path_idx = 0;
    struct ly_path_predicate *pred;

    LY_CHECK_ARG_RET(ctx, parent || ctx, path, (path[0] == '/') || parent, LY_EINVAL);

    if (!ctx) {
        ctx = LYD_NODE_CTX(parent);
    }

    /* parse path */
    LY_CHECK_GOTO(ret = ly_path_parse(ctx, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_FALSE,
                                      LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_SIMPLE, &exp), cleanup);

    /* compile path */
    LY_CHECK_GOTO(ret = ly_path_compile(ctx, NULL, parent ? parent->schema : NULL, exp, LY_PATH_LREF_FALSE,
                                        options & LYD_NEWOPT_OUTPUT ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT,
                                        LY_PATH_TARGET_MANY, lydjson_resolve_prefix, NULL, LYD_JSON, &p), cleanup);

    schema = p[LY_ARRAY_SIZE(p) - 1].node;
    if ((schema->nodetype == LYS_LIST) && (p[LY_ARRAY_SIZE(p) - 1].pred_type == LY_PATH_PREDTYPE_NONE)
            && !(options & LYD_NEWOPT_OPAQ)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_XPATH, "Predicate missing for %s \"%s\" in path.",
               lys_nodetype2str(schema->nodetype), schema->name);
        ret = LY_EINVAL;
        goto cleanup;
    } else if ((schema->nodetype == LYS_LEAFLIST) && (p[LY_ARRAY_SIZE(p) - 1].pred_type == LY_PATH_PREDTYPE_NONE)) {
        /* parse leafref value into a predicate, if not defined in the path */
        p[LY_ARRAY_SIZE(p) - 1].pred_type = LY_PATH_PREDTYPE_LEAFLIST;
        LY_ARRAY_NEW_GOTO(ctx, p[LY_ARRAY_SIZE(p) - 1].predicates, pred, ret, cleanup);

        if (!value) {
            value = "";
        }

        r = LY_SUCCESS;
        if (options & LYD_NEWOPT_OPAQ) {
            r = lys_value_validate(NULL, schema, value, strlen(value), lydjson_resolve_prefix, NULL, LYD_JSON);
        }
        if (!r) {
            LY_CHECK_GOTO(ret = lyd_value_store(&pred->value, schema, value, strlen(value), NULL, lydjson_resolve_prefix,
                                                NULL, LYD_JSON), cleanup);
        } /* else we have opaq flag and the value is not valid, leavne no predicate and then create an opaque node */
    }

    /* try to find any existing nodes in the path */
    if (parent) {
        ret = ly_path_eval_partial(p, parent, &path_idx, &node);
        if (ret == LY_SUCCESS) {
            /* the node exists, are we supposed to update it or is it just a default? */
            if (!(options & LYD_NEWOPT_UPDATE) && !(node->flags & LYD_DEFAULT)) {
                LOGERR(ctx, LY_EEXIST, "Path \"%s\" already exists", path);
                ret = LY_EEXIST;
                goto cleanup;
            }

            /* update the existing node */
            ret = lyd_new_path_update(node, value, value_type, &nparent, &nnode);
            goto cleanup;
        } else if (ret == LY_EINCOMPLETE) {
            /* some nodes were found, adjust the iterator to the next segment */
            ++path_idx;
        } else if (ret == LY_ENOTFOUND) {
            /* we will create the nodes from top-level, default behavior (absolute path), or from the parent (relative path) */
            if (lysc_data_parent(p[LY_ARRAY_SIZE(p) - 1].node)) {
                node = parent;
            }
        } else {
            /* error */
            goto cleanup;
        }
    }

    /* create all the non-existing nodes in a loop */
    for (; path_idx < LY_ARRAY_SIZE(p); ++path_idx) {
        cur_parent = node;
        schema = p[path_idx].node;

        switch (schema->nodetype) {
        case LYS_LIST:
            if (!(schema->flags & LYS_KEYLESS)) {
                if ((options & LYD_NEWOPT_OPAQ) && (p[path_idx].pred_type == LY_PATH_PREDTYPE_NONE)) {
                    /* creating opaque list without keys */
                    LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), NULL, 0, NULL,
                                                        LYD_JSON, NULL, NULL, 0, schema->module->name, &node), cleanup);
                } else {
                    assert(p[path_idx].pred_type == LY_PATH_PREDTYPE_LIST);
                    LY_CHECK_GOTO(ret = lyd_create_list(schema, p[path_idx].predicates, &node), cleanup);
                }
                break;
            }
            /* fallthrough */
        case LYS_CONTAINER:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            LY_CHECK_GOTO(ret = lyd_create_inner(schema, &node), cleanup);
            break;
        case LYS_LEAFLIST:
            if ((options & LYD_NEWOPT_OPAQ) && (p[path_idx].pred_type == LY_PATH_PREDTYPE_NONE)) {
                /* creating opaque leaf-list without value */
                LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), NULL, 0, NULL,
                                                    LYD_JSON, NULL, NULL, 0, schema->module->name, &node), cleanup);
            } else {
                assert(p[path_idx].pred_type == LY_PATH_PREDTYPE_LEAFLIST);
                LY_CHECK_GOTO(ret = lyd_create_term2(schema, &p[path_idx].predicates[0].value, &node), cleanup);
            }
            break;
        case LYS_LEAF:
            /* make there is some value */
            if (!value) {
                value = "";
            }

            r = LY_SUCCESS;
            if (options & LYD_NEWOPT_OPAQ) {
                r = lys_value_validate(NULL, schema, value, strlen(value), lydjson_resolve_prefix, NULL, LYD_JSON);
            }
            if (!r) {
                LY_CHECK_GOTO(ret = lyd_create_term(schema, value, strlen(value), NULL, lydjson_resolve_prefix, NULL,
                                                    LYD_JSON, &node), cleanup);
            } else {
                /* creating opaque leaf without value */
                LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), NULL, 0, NULL,
                                                    LYD_JSON, NULL, NULL, 0, schema->module->name, &node), cleanup);
            }
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            LY_CHECK_GOTO(ret = lyd_create_any(schema, value, value_type, &node), cleanup);
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
            goto cleanup;
        }

        if (cur_parent) {
            /* connect to the parent */
            lyd_insert_node(cur_parent, NULL, node);
        } else if (parent) {
            /* connect to top-level siblings */
            lyd_insert_node(NULL, &parent, node);
        }

        /* update remembered nodes */
        if (!nparent) {
            nparent = node;
        }
        nnode = node;
    }

cleanup:
    lyxp_expr_free(ctx, exp);
    ly_path_free(ctx, p);
    if (!ret) {
        /* set out params only on success */
        if (new_parent) {
            *new_parent = nparent;
        }
        if (new_node) {
            *new_node = nnode;
        }
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
            anchor = lyd_get_prev_key_anchor(lyd_node_children(parent, 0), node->schema);
            if (anchor) {
                lyd_insert_after_node(anchor, node);
            } else if (lyd_node_children(parent, 0)) {
                lyd_insert_before_node(lyd_node_children(parent, 0), node);
            } else {
                lyd_insert_last_node(parent, node);
            }

            /* hash list if all its keys were added */
            assert(parent->schema->nodetype == LYS_LIST);
            anchor = lyd_node_children(parent, 0);
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
    assert(!parent || !(parent->nodetype & (LYS_CASE | LYS_CHOICE)));

    /* find schema parent */
    par2 = lysc_data_parent(schema);

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

    LY_CHECK_ARG_RET(NULL, parent, node, parent->schema->nodetype & LYD_NODE_INNER, LY_EINVAL);

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

    LY_CHECK_RET(lyd_insert_check_schema(lysc_data_parent(sibling->schema), node->schema));

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

    LY_CHECK_RET(lyd_insert_check_schema(lysc_data_parent(sibling->schema), node->schema));

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

    LY_CHECK_RET(lyd_insert_check_schema(lysc_data_parent(sibling->schema), node->schema));

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
            if (iter->schema && (iter->schema->flags & LYS_KEYLESS)) {
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
    LY_ARRAY_SIZE_TYPE u;

    assert((parent || meta) && mod);

    LY_ARRAY_FOR(mod->compiled->exts, u) {
        if (mod->compiled->exts[u].def->plugin == lyext_plugins_internal[LYEXT_PLUGIN_INTERNAL_ANNOTATION].plugin &&
                !ly_strncmp(mod->compiled->exts[u].argument, name, name_len)) {
            /* we have the annotation definition */
            ant = &mod->compiled->exts[u];
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
lyd_target(const struct ly_path *path, const struct lyd_node *tree)
{
    struct lyd_node *target;

    if (ly_path_eval(path, tree, &target)) {
        return NULL;
    }

    return (struct lyd_node_term *)target;
}

API LY_ERR
lyd_compare(const struct lyd_node *node1, const struct lyd_node *node2, int options)
{
    const struct lyd_node *iter1, *iter2;
    struct lyd_node_term *term1, *term2;
    struct lyd_node_any *any1, *any2;
    struct lyd_node_opaq *opaq1, *opaq2;
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

            term1 = (struct lyd_node_term *)node1;
            term2 = (struct lyd_node_term *)node2;
            if (term1->value.realtype != term2->value.realtype) {
                return LY_ENOT;
            }

            return term1->value.realtype->plugin->compare(&term1->value, &term2->value);
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
        case LYS_RPC:
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
                        key && (key->flags & LYS_KEY);
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
 *
 * @param[in] node Original node to duplicate
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first First sibling, NULL if no top-level sibling exist yet. Can be also NULL if @p parent is set.
 * @param[in] options Bitmask of options flags, see @ref dupoptions.
 * @param[out] dup_p Pointer where the created duplicated node is placed (besides connecting it int @p parent / @p first sibling).
 * @return LY_ERR value
 */
static LY_ERR
lyd_dup_recursive(const struct lyd_node *node, struct lyd_node *parent, struct lyd_node **first, int options,
                  struct lyd_node **dup_p)
{
    LY_ERR ret;
    struct lyd_node *dup = NULL;
    LY_ARRAY_SIZE_TYPE u;

    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);

    if (!node->schema) {
        dup = calloc(1, sizeof(struct lyd_node_opaq));
    } else {
        switch (node->schema->nodetype) {
        case LYS_RPC:
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

    if (options & LYD_DUP_WITH_FLAGS) {
        dup->flags = node->flags;
    } else {
        dup->flags = (node->flags & LYD_DEFAULT) | LYD_NEW;
    }
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
                    key && (key->flags & LYS_KEY);
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
                if (!any->value.tree) {
                    /* get the last error's error code recorded by lyd_dup */
                    struct ly_err_item *ei = ly_err_first(LYD_NODE_CTX(node));
                    ret = ei ? ei->prev->no : LY_EOTHER;
                    goto error;
                }
                LY_CHECK_ERR_GOTO(!any->value.tree, ret = 0 ,error);
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

/**
 * @brief Merge a source sibling into target siblings.
 *
 * @param[in,out] first_trg First target sibling, is updated if top-level.
 * @param[in] parent_trg Target parent.
 * @param[in,out] sibling_src Source sibling to merge, set to NULL if spent.
 * @param[in] options Merge options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_merge_sibling_r(struct lyd_node **first_trg, struct lyd_node *parent_trg, const struct lyd_node **sibling_src_p,
                    int options)
{
    LY_ERR ret;
    const struct lyd_node *child_src, *tmp, *sibling_src;
    struct lyd_node *match_trg, *dup_src, *next, *elem;
    struct lysc_type *type;
    LYD_ANYDATA_VALUETYPE tmp_val_type;
    union lyd_any_value tmp_val;

    sibling_src = *sibling_src_p;
    if (sibling_src->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* try to find the exact instance */
        ret = lyd_find_sibling_first(*first_trg, sibling_src, &match_trg);
    } else {
        /* try to simply find the node, there cannot be more instances */
        ret = lyd_find_sibling_val(*first_trg, sibling_src->schema, NULL, 0, &match_trg);
    }

    if (!ret) {
        /* node found, make sure even value matches for all node types */
        if ((match_trg->schema->nodetype == LYS_LEAF) && lyd_compare(sibling_src, match_trg, LYD_COMPARE_DEFAULTS)) {
            /* since they are different, they cannot both be default */
            assert(!(sibling_src->flags & LYD_DEFAULT) || !(match_trg->flags & LYD_DEFAULT));

            /* update value (or only LYD_DEFAULT flag) only if no flag set or the source node is not default */
            if (!(options & LYD_MERGE_EXPLICIT) || !(sibling_src->flags & LYD_DEFAULT)) {
                type = ((struct lysc_node_leaf *)match_trg->schema)->type;
                type->plugin->free(LYD_NODE_CTX(match_trg), &((struct lyd_node_term *)match_trg)->value);
                LY_CHECK_RET(type->plugin->duplicate(LYD_NODE_CTX(match_trg), &((struct lyd_node_term *)sibling_src)->value,
                                                     &((struct lyd_node_term *)match_trg)->value));

                /* copy flags and add LYD_NEW */
                match_trg->flags = sibling_src->flags | LYD_NEW;
            }
        } else if ((match_trg->schema->nodetype & LYS_ANYDATA) && lyd_compare(sibling_src, match_trg, 0)) {
            if (options & LYD_MERGE_DESTRUCT) {
                dup_src = (struct lyd_node *)sibling_src;
                lyd_unlink_tree(dup_src);
                /* spend it */
                *sibling_src_p = NULL;
            } else {
                dup_src = lyd_dup(sibling_src, NULL, 0);
                LY_CHECK_RET(!dup_src, LY_EMEM);
            }
            /* just switch values */
            tmp_val_type = ((struct lyd_node_any *)match_trg)->value_type;
            tmp_val = ((struct lyd_node_any *)match_trg)->value;
            ((struct lyd_node_any *)match_trg)->value_type = ((struct lyd_node_any *)sibling_src)->value_type;
            ((struct lyd_node_any *)match_trg)->value = ((struct lyd_node_any *)sibling_src)->value;
            ((struct lyd_node_any *)sibling_src)->value_type = tmp_val_type;
            ((struct lyd_node_any *)sibling_src)->value = tmp_val;

            /* copy flags and add LYD_NEW */
            match_trg->flags = sibling_src->flags | LYD_NEW;

            /* dup_src is not needed, actually */
            lyd_free_tree(dup_src);
        } else {
            /* check descendants, recursively */
            LY_LIST_FOR_SAFE(LYD_CHILD(sibling_src), tmp, child_src) {
                LY_CHECK_RET(lyd_merge_sibling_r(lyd_node_children_p(match_trg), match_trg, &child_src, options));
            }
        }
    } else {
        /* node not found, merge it */
        if (options & LYD_MERGE_DESTRUCT) {
            dup_src = (struct lyd_node *)sibling_src;
            lyd_unlink_tree(dup_src);
            /* spend it */
            *sibling_src_p = NULL;
        } else {
            dup_src = lyd_dup(sibling_src, NULL, LYD_DUP_RECURSIVE | LYD_DUP_WITH_FLAGS);
            LY_CHECK_RET(!dup_src, LY_EMEM);
        }

        /* set LYD_NEW for all the new nodes, required for validation */
        LYD_TREE_DFS_BEGIN(dup_src, next, elem) {
            elem->flags |= LYD_NEW;
            LYD_TREE_DFS_END(dup_src, next, elem);
        }

        lyd_insert_node(parent_trg, first_trg, dup_src);
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_merge(struct lyd_node **target, const struct lyd_node *source, int options)
{
    const struct lyd_node *sibling_src, *tmp;
    int first;

    LY_CHECK_ARG_RET(NULL, target, LY_EINVAL);

    if (!source) {
        /* nothing to merge */
        return LY_SUCCESS;
    }

    if (lysc_data_parent((*target)->schema) || lysc_data_parent(source->schema)) {
        LOGERR(LYD_NODE_CTX(source), LY_EINVAL, "Invalid arguments - can merge only 2 top-level subtrees (%s()).", __func__);
        return LY_EINVAL;
    }

    LY_LIST_FOR_SAFE(source, tmp, sibling_src) {
        first = sibling_src == source ? 1 : 0;
        LY_CHECK_RET(lyd_merge_sibling_r(target, NULL, &sibling_src, options));
        if (first && !sibling_src) {
            /* source was spent (unlinked), move to the next node */
            source = tmp;
        }

        if (options & LYD_MERGE_NOSIBLINGS) {
            break;
        }
    }

    if (options & LYD_MERGE_DESTRUCT) {
        /* free any leftover source data that were not merged */
        lyd_free_siblings((struct lyd_node *)source);
    }

    return LY_SUCCESS;
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

    for (key = lyd_node_children(node, 0); key && (key->schema->flags & LYS_KEY); key = key->next) {
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
    struct lyxp_expr *expr = NULL;
    uint16_t exp_idx = 0;
    struct ly_path_predicate *predicates = NULL;
    enum ly_path_pred_type pred_type = 0;
    struct lyd_value val = {0};
    LY_ARRAY_SIZE_TYPE u;

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
        /* parse keys */
        LY_CHECK_GOTO(rc = ly_path_parse_predicate(schema->module->ctx, key_or_value, val_len, LY_PATH_PREFIX_OPTIONAL,
                                                   LY_PATH_PRED_KEYS, &expr), cleanup);

        /* compile them */
        LY_CHECK_GOTO(rc = ly_path_compile_predicate(schema->module->ctx, NULL, schema, expr, &exp_idx, lydjson_resolve_prefix,
                                                     NULL, LYD_JSON, &predicates, &pred_type), cleanup);
    }

    /* find first matching value */
    LY_LIST_FOR(first, node) {
        if (node->schema != schema) {
            continue;
        }

        if ((schema->nodetype == LYS_LIST) && predicates) {
            /* compare all set keys */
            LY_ARRAY_FOR(predicates, u) {
                /* find key */
                rc = lyd_find_sibling_val(lyd_node_children(node, 0), predicates[u].key, NULL, 0, (struct lyd_node **)&term);
                if (rc == LY_ENOTFOUND) {
                    /* all keys must always exist */
                    LOGINT_RET(schema->module->ctx);
                }
                LY_CHECK_GOTO(rc, cleanup);

                /* compare values */
                if (!term->value.realtype->plugin->compare(&term->value, &predicates[u].value)) {
                    break;
                }
            }

            if (u < LY_ARRAY_SIZE(predicates)) {
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
    ly_path_predicates_free(schema->module->ctx, pred_type, NULL, predicates);
    lyxp_expr_free(schema->module->ctx, expr);
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

    if (!siblings || (lysc_data_parent(siblings->schema) != lysc_data_parent(target->schema))) {
        /* no data or schema mismatch */
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

    if (!siblings || (lysc_data_parent(siblings->schema) != lysc_data_parent(target->schema))) {
        /* no data or schema mismatch */
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
    if ((schema->nodetype == LYS_LIST) && (schema->flags & LYS_KEYLESS)) {
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

    if (!siblings || (lysc_data_parent(siblings->schema) != lysc_data_parent(schema))) {
        /* no data or schema mismatch */
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
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_LEAF:
        /* find it based on schema only, there cannot be more instances */
        rc = lyd_find_sibling_schema(siblings, schema, match);
        break;
    case LYS_LEAFLIST:
        /* target used attributes: schema, hash, value */
        rc = lyd_create_term(schema, key_or_value, val_len, NULL, lydjson_resolve_prefix, NULL, LYD_JSON, &target);
        LY_CHECK_RET(rc && (rc != LY_EINCOMPLETE), rc);
        rc = LY_SUCCESS;
        /* fallthrough */
    case LYS_LIST:
        if (schema->nodetype == LYS_LIST) {
            /* target used attributes: schema, hash, child (all keys) */
            LY_CHECK_RET(lyd_create_list2(schema, key_or_value, val_len, &target));
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

API LY_ERR
lyd_find_xpath(const struct lyd_node *ctx_node, const char *xpath, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set;
    struct lyxp_expr *exp;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx_node, xpath, set, LY_EINVAL);

    memset(&xp_set, 0, sizeof xp_set);

    /* compile expression */
    exp = lyxp_expr_parse((struct ly_ctx *)LYD_NODE_CTX(ctx_node), xpath, 0, 1);
    LY_CHECK_ERR_GOTO(!exp, ret = LY_EINVAL, cleanup);

    /* evaluate expression */
    ret = lyxp_eval(exp, LYD_JSON, ctx_node->schema->module, ctx_node, LYXP_NODE_ELEM, ctx_node, &xp_set, 0);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    *set = ly_set_new();
    LY_CHECK_ERR_GOTO(!*set, LOGMEM(LYD_NODE_CTX(ctx_node)); ret = LY_EMEM, cleanup);

    /* transform into ly_set */
    if (xp_set.type == LYXP_SET_NODE_SET) {
        /* allocate memory for all the elements once (even though not all items must be elements but most likely will be) */
        (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
        LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(LYD_NODE_CTX(ctx_node)); ret = LY_EMEM, cleanup);
        (*set)->size = xp_set.used;

        for (i = 0; i < xp_set.used; ++i) {
            if (xp_set.val.nodes[i].type == LYXP_NODE_ELEM) {
                ly_set_add(*set, xp_set.val.nodes[i].node, LY_SET_OPT_USEASLIST);
            }
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free((struct ly_ctx *)LYD_NODE_CTX(ctx_node), exp);
    return ret;
}
