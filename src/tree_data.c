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
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "diff.h"
#include "hash_table.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "path.h"
#include "plugins_exts.h"
#include "plugins_exts_internal.h"
#include "plugins_exts_metadata.h"
#include "plugins_types.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"
#include "xml.h"
#include "xpath.h"

static LY_ERR lyd_find_sibling_schema(const struct lyd_node *siblings, const struct lysc_node *schema,
        struct lyd_node **match);

LY_ERR
lyd_value_store(const struct ly_ctx *ctx, struct lyd_value *val, const struct lysc_type *type, const char *value,
        size_t value_len, ly_bool *dynamic, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, ly_bool *incomplete, enum LY_VLOG_ELEM log_elem_type, const void *log_elem)
{
    LY_ERR ret;
    struct ly_err_item *err = NULL;
    uint32_t options = (dynamic && *dynamic ? LY_TYPE_STORE_DYNAMIC : 0);

    if (incomplete) {
        *incomplete = 0;
    }

    ret = type->plugin->store(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node, val, NULL, &err);
    if (ret == LY_EINCOMPLETE) {
        if (incomplete) {
            *incomplete = 1;
        }
    } else if (ret) {
        if (err) {
            LOGVAL(ctx, log_elem_type, log_elem, err->vecode, err->msg);
            ly_err_free(err);
        } else {
            LOGVAL(ctx, log_elem_type, log_elem, LYVE_OTHER, "Storing value \"%.*s\" failed.", (int)value_len, value);
        }
        return ret;
    }

    if (dynamic) {
        *dynamic = 0;
    }
    return LY_SUCCESS;
}

LY_ERR
lyd_value_validate_incomplete(const struct ly_ctx *ctx, const struct lysc_type *type, struct lyd_value *val,
        const struct lyd_node *ctx_node, const struct lyd_node *tree, enum LY_VLOG_ELEM log_elem_type, const void *log_elem)
{
    LY_ERR ret;
    struct ly_err_item *err = NULL;

    assert(type->plugin->validate);

    ret = type->plugin->validate(ctx, type, ctx_node, tree, val, &err);
    if (ret) {
        if (err) {
            LOGVAL(ctx, log_elem_type, log_elem, err->vecode, err->msg);
            ly_err_free(err);
        } else {
            LOGVAL(ctx, log_elem_type, log_elem, LYVE_OTHER, "Resolving value \"%s\" failed.", val->canonical);
        }
        return ret;
    }

    return LY_SUCCESS;
}

LY_ERR
_lys_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len,
        LY_PREFIX_FORMAT format, void *prefix_data)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct lyd_value storage;
    struct lysc_type *type;

    LY_CHECK_ARG_RET(ctx, node, value, LY_EINVAL);

    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGARG(ctx, node);
        return LY_EINVAL;
    }

    type = ((struct lysc_node_leaf *)node)->type;
    rc = type->plugin->store(ctx ? ctx : node->module->ctx, type, value, value_len, 0, format, prefix_data,
            LYD_HINT_SCHEMA, node, &storage, NULL, &err);
    if (rc == LY_EINCOMPLETE) {
        /* actually success since we do not provide the context tree and call validation with
         * LY_TYPE_OPTS_INCOMPLETE_DATA */
        rc = LY_SUCCESS;
    } else if (rc && err) {
        if (ctx) {
            /* log only in case the ctx was provided as input parameter */
            LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
        }
        ly_err_free(err);
    }

    if (!rc) {
        type->plugin->free(ctx ? ctx : node->module->ctx, &storage);
    }
    return rc;
}

API LY_ERR
lys_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len)
{
    return _lys_value_validate(ctx, node, value, value_len, LY_PREF_JSON, NULL);
}

API LY_ERR
lyd_value_validate(const struct ly_ctx *ctx, const struct lyd_node_term *node, const char *value, size_t value_len,
        const struct lyd_node *tree, const struct lysc_type **realtype)
{
    LY_ERR rc;
    struct ly_err_item *err = NULL;
    struct lysc_type *type;
    struct lyd_value val = {0};
    ly_bool stored = 0;

    LY_CHECK_ARG_RET(ctx, node, value, LY_EINVAL);

    type = ((struct lysc_node_leaf *)node->schema)->type;
    /* store */
    rc = type->plugin->store(ctx ? ctx : LYD_CTX(node), type, value, value_len, 0, LY_PREF_JSON, NULL,
            LYD_HINT_DATA, node->schema, &val, NULL, &err);
    if (rc == LY_EINCOMPLETE) {
        stored = 1;

        /* resolve */
        rc = type->plugin->validate(ctx ? ctx : LYD_CTX(node), type, (struct lyd_node *)node, tree, &val, &err);
    }

    if (rc) {
        if (err) {
            if (ctx) {
                LOGVAL(ctx, LY_VLOG_LYD, node, err->vecode, err->msg);
            }
            ly_err_free(err);
        }
        if (stored) {
            type->plugin->free(ctx ? ctx : LYD_CTX(node), &val);
        }
        return rc;
    }

    if (realtype) {
        if (val.realtype->basetype == LY_TYPE_UNION) {
            *realtype = val.subvalue->value.realtype;
        } else {
            *realtype = val.realtype;
        }
    }

    type->plugin->free(ctx ? ctx : LYD_CTX(node), &val);
    return LY_SUCCESS;
}

API LY_ERR
lyd_value_compare(const struct lyd_node_term *node, const char *value, size_t value_len)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    struct lyd_value val = {0};

    LY_CHECK_ARG_RET(node ? node->schema->module->ctx : NULL, node, value, LY_EINVAL);

    ctx = node->schema->module->ctx;
    type = ((struct lysc_node_leaf *)node->schema)->type;

    /* store the value */
    ret = lyd_value_store(ctx, &val, type, value, value_len, NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA, node->schema,
            NULL, LY_VLOG_LYSC, node->schema);
    LY_CHECK_RET(ret);

    /* compare values */
    ret = type->plugin->compare(&node->value, &val);

    type->plugin->free(ctx, &val);
    return ret;
}

API ly_bool
lyd_is_default(const struct lyd_node *node)
{
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lyd_node_term *term;
    LY_ARRAY_COUNT_TYPE u;

    assert(node->schema->nodetype & LYD_NODE_TERM);
    term = (const struct lyd_node_term *)node;

    if (node->schema->nodetype == LYS_LEAF) {
        leaf = (const struct lysc_node_leaf *)node->schema;
        if (!leaf->dflt) {
            return 0;
        }

        /* compare with the default value */
        if (leaf->type->plugin->compare(&term->value, leaf->dflt)) {
            return 0;
        }
    } else {
        llist = (const struct lysc_node_leaflist *)node->schema;
        if (!llist->dflts) {
            return 0;
        }

        LY_ARRAY_FOR(llist->dflts, u) {
            /* compare with each possible default value */
            if (llist->type->plugin->compare(&term->value, llist->dflts[u])) {
                return 0;
            }
        }
    }

    return 1;
}

static LYD_FORMAT
lyd_parse_get_format(const struct ly_in *in, LYD_FORMAT format)
{
    if (!format && (in->type == LY_IN_FILEPATH)) {
        /* unknown format - try to detect it from filename's suffix */
        const char *path = in->method.fpath.filepath;
        size_t len = strlen(path);

        /* ignore trailing whitespaces */
        for ( ; len > 0 && isspace(path[len - 1]); len--) {}

        if ((len >= LY_XML_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_XML_SUFFIX_LEN], LY_XML_SUFFIX, LY_XML_SUFFIX_LEN)) {
            format = LYD_XML;
        } else if ((len >= LY_JSON_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_JSON_SUFFIX_LEN], LY_JSON_SUFFIX, LY_JSON_SUFFIX_LEN)) {
            format = LYD_JSON;
        } else if ((len >= LY_LYB_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_LYB_SUFFIX_LEN], LY_LYB_SUFFIX, LY_LYB_SUFFIX_LEN)) {
            format = LYD_LYB;
        } /* else still unknown */
    }

    return format;
}

API LY_ERR
lyd_parse_data(const struct ly_ctx *ctx, struct ly_in *in, LYD_FORMAT format, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_ctx *lydctx = NULL;

    LY_CHECK_ARG_RET(ctx, ctx, in, tree, LY_EINVAL);
    LY_CHECK_ARG_RET(ctx, !(parse_options & ~LYD_PARSE_OPTS_MASK), LY_EINVAL);
    LY_CHECK_ARG_RET(ctx, !(validate_options & ~LYD_VALIDATE_OPTS_MASK), LY_EINVAL);

    format = lyd_parse_get_format(in, format);
    LY_CHECK_ARG_RET(ctx, format, LY_EINVAL);

    /* init */
    *tree = NULL;

    /* remember input position */
    in->func_start = in->current;

    switch (format) {
    case LYD_XML:
        LY_CHECK_RET(lyd_parse_xml_data(ctx, in, parse_options, validate_options, tree, &lydctx));
        break;
    case LYD_JSON:
        LY_CHECK_RET(lyd_parse_json_data(ctx, in, parse_options, validate_options, tree, &lydctx));
        break;
    case LYD_LYB:
        LY_CHECK_RET(lyd_parse_lyb_data(ctx, in, parse_options, validate_options, tree, &lydctx));
        break;
    case LYD_UNKNOWN:
        LOGINT_RET(ctx);
    }

    if (!(parse_options & LYD_PARSE_ONLY)) {
        uint32_t i = 0;
        const struct lys_module *mod;
        struct lyd_node *first, *next, **first2;

        next = *tree;
        while (1) {
            if (validate_options & LYD_VALIDATE_PRESENT) {
                mod = lyd_data_next_module(&next, &first);
            } else {
                mod = lyd_mod_next_module(next, NULL, ctx, &i, &first);
            }
            if (!mod) {
                break;
            }
            if (first == *tree) {
                /* make sure first2 changes are carried to tree */
                first2 = tree;
            } else {
                first2 = &first;
            }

            /* validate new top-level nodes, autodelete CANNOT occur, all nodes are new */
            LY_CHECK_GOTO(ret = lyd_validate_new(first2, NULL, mod, NULL), cleanup);

            /* add all top-level defaults for this module */
            ret = lyd_new_implicit_r(NULL, first2, NULL, mod, &lydctx->unres_node_type, &lydctx->when_check,
                    (validate_options & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);

            /* finish incompletely validated terminal values/attributes and when conditions */
            ret = lyd_validate_unres(tree, &lydctx->when_check, &lydctx->unres_node_type, &lydctx->unres_meta_type, NULL);
            LY_CHECK_GOTO(ret, cleanup);

            /* perform final validation that assumes the data tree is final */
            LY_CHECK_GOTO(ret = lyd_validate_final_r(*first2, NULL, NULL, mod, validate_options, 0), cleanup);
        }
    }

cleanup:
    lydctx->free(lydctx);
    if (ret) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    return ret;
}

API LY_ERR
lyd_parse_data_mem(const struct ly_ctx *ctx, const char *data, LYD_FORMAT format, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree)
{
    LY_ERR ret;
    struct ly_in *in;

    LY_CHECK_RET(ly_in_new_memory(data, &in));
    ret = lyd_parse_data(ctx, in, format, parse_options, validate_options, tree);

    ly_in_free(in, 0);
    return ret;
}

API LY_ERR
lyd_parse_data_fd(const struct ly_ctx *ctx, int fd, LYD_FORMAT format, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree)
{
    LY_ERR ret;
    struct ly_in *in;

    LY_CHECK_RET(ly_in_new_fd(fd, &in));
    ret = lyd_parse_data(ctx, in, format, parse_options, validate_options, tree);

    ly_in_free(in, 0);
    return ret;
}

API LY_ERR
lyd_parse_data_path(const struct ly_ctx *ctx, const char *path, LYD_FORMAT format, uint32_t parse_options,
        uint32_t validate_options, struct lyd_node **tree)
{
    LY_ERR ret;
    struct ly_in *in;

    LY_CHECK_RET(ly_in_new_filepath(path, 0, &in));
    ret = lyd_parse_data(ctx, in, format, parse_options, validate_options, tree);

    ly_in_free(in, 0);
    return ret;
}

API LY_ERR
lyd_parse_rpc(const struct ly_ctx *ctx, struct ly_in *in, LYD_FORMAT format, struct lyd_node **tree, struct lyd_node **op)
{
    LY_CHECK_ARG_RET(ctx, ctx, in, tree, LY_EINVAL);

    format = lyd_parse_get_format(in, format);
    LY_CHECK_ARG_RET(ctx, format, LY_EINVAL);

    /* init */
    *tree = NULL;
    if (op) {
        *op = NULL;
    }

    /* remember input position */
    in->func_start = in->current;

    switch (format) {
    case LYD_XML:
        return lyd_parse_xml_rpc(ctx, in, tree, op);
    case LYD_JSON:
        return lyd_parse_json_rpc(ctx, in, tree, op);
    case LYD_LYB:
        return lyd_parse_lyb_rpc(ctx, in, tree, op);
    case LYD_UNKNOWN:
        break;
    }

    LOGINT_RET(ctx);
}

API LY_ERR
lyd_parse_reply(const struct lyd_node *request, struct ly_in *in, LYD_FORMAT format, struct lyd_node **tree,
        struct lyd_node **op)
{
    LY_CHECK_ARG_RET(NULL, request, LY_EINVAL);
    LY_CHECK_ARG_RET(LYD_CTX(request), in, tree || op, LY_EINVAL);

    format = lyd_parse_get_format(in, format);
    LY_CHECK_ARG_RET(LYD_CTX(request), format, LY_EINVAL);

    /* init */
    if (tree) {
        *tree = NULL;
    }
    if (op) {
        *op = NULL;
    }

    /* remember input position */
    in->func_start = in->current;

    switch (format) {
    case LYD_XML:
        return lyd_parse_xml_reply(request, in, tree, op);
    case LYD_JSON:
        return lyd_parse_json_reply(request, in, tree, op);
    case LYD_LYB:
        return lyd_parse_lyb_reply(request, in, tree, op);
    case LYD_UNKNOWN:
        break;
    }

    LOGINT_RET(LYD_CTX(request));
}

API LY_ERR
lyd_parse_notif(const struct ly_ctx *ctx, struct ly_in *in, LYD_FORMAT format, struct lyd_node **tree, struct lyd_node **ntf)
{
    LY_CHECK_ARG_RET(ctx, ctx, in, tree || ntf, LY_EINVAL);

    format = lyd_parse_get_format(in, format);
    LY_CHECK_ARG_RET(ctx, format, LY_EINVAL);

    /* init */
    if (tree) {
        *tree = NULL;
    }
    if (ntf) {
        *ntf = NULL;
    }

    /* remember input position */
    in->func_start = in->current;

    switch (format) {
    case LYD_XML:
        return lyd_parse_xml_notif(ctx, in, tree, ntf);
    case LYD_JSON:
        return lyd_parse_json_notif(ctx, in, tree, ntf);
    case LYD_LYB:
        return lyd_parse_lyb_notif(ctx, in, tree, ntf);
    case LYD_UNKNOWN:
        break;
    }

    LOGINT_RET(ctx);
}

LY_ERR
lyd_create_term(const struct lysc_node *schema, const char *value, size_t value_len, ly_bool *dynamic,
        LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, ly_bool *incomplete, struct lyd_node **node)
{
    LY_ERR ret;
    struct lyd_node_term *term;

    assert(schema->nodetype & LYD_NODE_TERM);

    term = calloc(1, sizeof *term);
    LY_CHECK_ERR_RET(!term, LOGMEM(schema->module->ctx), LY_EMEM);

    term->schema = schema;
    term->prev = (struct lyd_node *)term;
    term->flags = LYD_NEW;

    ret = lyd_value_store(schema->module->ctx, &term->value, ((struct lysc_node_leaf *)term->schema)->type, value,
            value_len, dynamic, format, prefix_data, hints, schema, incomplete, LY_VLOG_LYSC, schema);
    LY_CHECK_ERR_RET(ret, free(term), ret);
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
    assert(val && val->canonical && val->realtype);

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
    if ((schema->nodetype == LYS_CONTAINER) && !(schema->flags & LYS_PRESENCE)) {
        in->flags |= LYD_DEFAULT;
    }

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
    LY_ARRAY_COUNT_TYPE u;

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
    LY_CHECK_GOTO(ret = ly_path_parse_predicate(schema->module->ctx, NULL, keys, keys_len, LY_PATH_PREFIX_OPTIONAL,
            LY_PATH_PRED_KEYS, &expr), cleanup);

    /* compile them */
    LY_CHECK_GOTO(ret = ly_path_compile_predicate(schema->module->ctx, NULL, NULL, schema, expr, &exp_idx, LY_PREF_JSON,
            NULL, &predicates, &pred_type), cleanup);

    /* create the list node */
    LY_CHECK_GOTO(ret = lyd_create_list(schema, predicates, node), cleanup);

cleanup:
    lyxp_expr_free(schema->module->ctx, expr);
    ly_path_predicates_free(schema->module->ctx, pred_type, predicates);
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

    /* TODO: convert XML/JSON strings into a opaq data tree */
    any->value.str = value;
    any->value_type = value_type;
    lyd_hash((struct lyd_node *)any);

    *node = (struct lyd_node *)any;
    return LY_SUCCESS;
}

LY_ERR
lyd_create_opaq(const struct ly_ctx *ctx, const char *name, size_t name_len, const char *prefix, size_t pref_len,
        const char *module_key, size_t module_key_len, const char *value, size_t value_len, ly_bool *dynamic,
        LY_PREFIX_FORMAT format, void *val_prefix_data, uint32_t hints, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node_opaq *opaq;

    assert(ctx && name && name_len && format);

    if (!value_len) {
        value = "";
    }

    opaq = calloc(1, sizeof *opaq);
    LY_CHECK_ERR_GOTO(!opaq, LOGMEM(ctx); ret = LY_EMEM, finish);

    opaq->prev = (struct lyd_node *)opaq;
    LY_CHECK_GOTO(ret = lydict_insert(ctx, name, name_len, &opaq->name.name), finish);

    if (pref_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, prefix, pref_len, &opaq->name.prefix), finish);
    }
    if (module_key_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, module_key, module_key_len, &opaq->name.module_ns), finish);
    }
    if (dynamic && *dynamic) {
        LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, (char *)value, &opaq->value), finish);
        *dynamic = 0;
    } else {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, value, value_len, &opaq->value), finish);
    }

    opaq->format = format;
    opaq->val_prefix_data = val_prefix_data;
    opaq->hints = hints;
    opaq->ctx = ctx;

finish:
    if (ret) {
        lyd_free_tree((struct lyd_node *)opaq);
        ly_free_prefix_data(format, val_prefix_data);
    } else {
        *node = (struct lyd_node *)opaq;
    }
    return ret;
}

API LY_ERR
lyd_new_inner(struct lyd_node *parent, const struct lys_module *module, const char *name, ly_bool output, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0,
            LYS_CONTAINER | LYS_NOTIF | LYS_RPC | LYS_ACTION, output ? LYS_GETNEXT_OUTPUT : 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Inner node (and not a list) \"%s\" not found.", name), LY_ENOTFOUND);

    LY_CHECK_RET(lyd_create_inner(schema, &ret));
    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

API LY_ERR
lyd_new_list(struct lyd_node *parent, const struct lys_module *module, const char *name, ly_bool output, struct lyd_node **node, ...)
{
    struct lyd_node *ret = NULL, *key;
    const struct lysc_node *schema, *key_s;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);
    va_list ap;
    const char *key_val;
    LY_ERR rc = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_LIST, output ? LYS_GETNEXT_OUTPUT : 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found.", name), LY_ENOTFOUND);

    /* create list inner node */
    LY_CHECK_RET(lyd_create_inner(schema, &ret));

    va_start(ap, node);

    /* create and insert all the keys */
    for (key_s = lysc_node_children(schema, 0); key_s && (key_s->flags & LYS_KEY); key_s = key_s->next) {
        key_val = va_arg(ap, const char *);

        rc = lyd_create_term(key_s, key_val, key_val ? strlen(key_val) : 0, NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA,
                NULL, &key);
        LY_CHECK_GOTO(rc, cleanup);
        lyd_insert_node(ret, NULL, key);
    }

    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }

cleanup:
    va_end(ap);
    if (rc) {
        lyd_free_tree(ret);
        ret = NULL;
    } else if (node) {
        *node = ret;
    }
    return rc;
}

API LY_ERR
lyd_new_list2(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *keys,
        ly_bool output, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }
    if (!keys) {
        keys = "";
    }

    /* find schema node */
    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_LIST, output ? LYS_GETNEXT_OUTPUT : 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found.", name), LY_ENOTFOUND);

    if ((schema->flags & LYS_KEYLESS) && !keys[0]) {
        /* key-less list */
        LY_CHECK_RET(lyd_create_inner(schema, &ret));
    } else {
        /* create the list node */
        LY_CHECK_RET(lyd_create_list2(schema, keys, strlen(keys), &ret));
    }
    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

API LY_ERR
lyd_new_term(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str,
        ly_bool output, struct lyd_node **node)
{
    LY_ERR rc;
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYD_NODE_TERM, output ? LYS_GETNEXT_OUTPUT : 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Term node \"%s\" not found.", name), LY_ENOTFOUND);

    rc = lyd_create_term(schema, val_str, val_str ? strlen(val_str) : 0, NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA, NULL,
            &ret);
    LY_CHECK_RET(rc);
    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

API LY_ERR
lyd_new_any(struct lyd_node *parent, const struct lys_module *module, const char *name, const void *value,
        LYD_ANYDATA_VALUETYPE value_type, ly_bool output, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = parent ? parent->schema->module->ctx : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYD_NODE_ANY, output ? LYS_GETNEXT_OUTPUT : 0);
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Any node \"%s\" not found.", name), LY_ENOTFOUND);

    LY_CHECK_RET(lyd_create_any(schema, value, value_type, &ret));
    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
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
        if (lyd_compare_single(node, new_any, 0)) {
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
        LOGINT(LYD_CTX(node));
        ret = LY_EINT;
        break;
    }

    return ret;
}

API LY_ERR
lyd_new_meta(const struct ly_ctx *ctx, struct lyd_node *parent, const struct lys_module *module, const char *name,
        const char *val_str, ly_bool clear_dflt, struct lyd_meta **meta)
{
    const char *prefix, *tmp;
    size_t pref_len, name_len;

    LY_CHECK_ARG_RET(NULL, ctx, name, module || strchr(name, ':'), parent || meta, LY_EINVAL);

    if (parent && !parent->schema) {
        LOGERR(ctx, LY_EINVAL, "Cannot add metadata to an opaque node \"%s\".", ((struct lyd_node_opaq *)parent)->name);
        return LY_EINVAL;
    }
    if (meta) {
        *meta = NULL;
    }

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Metadata name \"%s\" is not valid.", name);
        return LY_EINVAL;
    }

    /* find the module */
    if (prefix) {
        module = ly_ctx_get_module_implemented2(ctx, prefix, pref_len);
        LY_CHECK_ERR_RET(!module, LOGERR(ctx, LY_EINVAL, "Module \"%.*s\" not found.", pref_len, prefix), LY_ENOTFOUND);
    }

    /* set value if none */
    if (!val_str) {
        val_str = "";
    }

    return lyd_create_meta(parent, meta, module, name, name_len, val_str, strlen(val_str), NULL, LY_PREF_JSON,
            NULL, LYD_HINT_DATA, clear_dflt, NULL);
}

API LY_ERR
lyd_new_meta2(const struct ly_ctx *ctx, struct lyd_node *parent, ly_bool clear_dflt, const struct lyd_attr *attr,
        struct lyd_meta **meta)
{
    const struct lys_module *mod;

    LY_CHECK_ARG_RET(NULL, ctx, attr, parent || meta, LY_EINVAL);

    if (parent && !parent->schema) {
        LOGERR(ctx, LY_EINVAL, "Cannot add metadata to an opaque node \"%s\".", ((struct lyd_node_opaq *)parent)->name);
        return LY_EINVAL;
    }
    if (meta) {
        *meta = NULL;
    }

    switch (attr->format) {
    case LY_PREF_XML:
        mod = ly_ctx_get_module_implemented_ns(ctx, attr->name.module_ns);
        if (!mod) {
            LOGERR(ctx, LY_EINVAL, "Module with namespace \"%s\" not found.", attr->name.module_ns);
            return LY_ENOTFOUND;
        }
        break;
    case LY_PREF_JSON:
        mod = ly_ctx_get_module_implemented(ctx, attr->name.module_name);
        if (!mod) {
            LOGERR(ctx, LY_EINVAL, "Module \"%s\" not found.", attr->name.module_name);
            return LY_ENOTFOUND;
        }
        break;
    default:
        LOGINT_RET(ctx);
    }

    return lyd_create_meta(parent, meta, mod, attr->name.name, strlen(attr->name.name), attr->value, strlen(attr->value),
            NULL, attr->format, attr->val_prefix_data, attr->hints, clear_dflt, NULL);
}

API LY_ERR
lyd_new_opaq(struct lyd_node *parent, const struct ly_ctx *ctx, const char *name, const char *value,
        const char *module_name, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;

    LY_CHECK_ARG_RET(ctx, parent || ctx, parent || node, name, module_name, LY_EINVAL);

    if (!ctx) {
        ctx = LYD_CTX(parent);
    }
    if (!value) {
        value = "";
    }

    LY_CHECK_RET(lyd_create_opaq(ctx, name, strlen(name), NULL, 0, module_name, strlen(module_name), value,
            strlen(value), NULL, LY_PREF_JSON, NULL, 0, &ret));
    if (parent) {
        lyd_insert_node(parent, NULL, ret);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

API LY_ERR
lyd_new_attr(struct lyd_node *parent, const char *module_name, const char *name, const char *val_str,
        struct lyd_attr **attr)
{
    struct lyd_attr *ret = NULL;
    const struct ly_ctx *ctx;
    const char *prefix, *tmp;
    size_t pref_len, name_len, mod_len;

    LY_CHECK_ARG_RET(NULL, parent, !parent->schema, name, LY_EINVAL);

    ctx = LYD_CTX(parent);

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Metadata name \"%s\" is not valid.", name);
        return LY_EVALID;
    }

    /* get the module */
    if (module_name) {
        mod_len = strlen(module_name);
    } else {
        module_name = prefix;
        mod_len = pref_len;
    }

    /* set value if none */
    if (!val_str) {
        val_str = "";
    }

    LY_CHECK_RET(lyd_create_attr(parent, &ret, ctx, name, name_len, prefix, pref_len, module_name, mod_len, val_str,
            strlen(val_str), NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA));

    if (attr) {
        *attr = ret;
    }
    return LY_SUCCESS;
}

API LY_ERR
lyd_change_term(struct lyd_node *term, const char *val_str)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type *type;
    struct lyd_node_term *t;
    struct lyd_node *parent;
    struct lyd_value val = {0};
    ly_bool dflt_change, val_change;

    LY_CHECK_ARG_RET(NULL, term, term->schema, term->schema->nodetype & LYD_NODE_TERM, LY_EINVAL);

    if (!val_str) {
        val_str = "";
    }
    t = (struct lyd_node_term *)term;
    type = ((struct lysc_node_leaf *)term->schema)->type;

    /* parse the new value */
    ret = lyd_value_store(LYD_CTX(term), &val, type, val_str, strlen(val_str), NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA,
            term->schema, NULL, LY_VLOG_LYD, term);
    LY_CHECK_GOTO(ret, cleanup);

    /* compare original and new value */
    if (type->plugin->compare(&t->value, &val)) {
        /* values differ, switch them */
        type->plugin->free(LYD_CTX(term), &t->value);
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
    type->plugin->free(LYD_CTX(term), &val);
    return ret;
}

API LY_ERR
lyd_change_meta(struct lyd_meta *meta, const char *val_str)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *m2 = NULL;
    struct lyd_value val;
    ly_bool val_change;

    LY_CHECK_ARG_RET(NULL, meta, LY_EINVAL);

    if (!val_str) {
        val_str = "";
    }

    /* parse the new value into a new meta structure */
    LY_CHECK_GOTO(ret = lyd_create_meta(NULL, &m2, meta->annotation->module, meta->name, strlen(meta->name), val_str,
            strlen(val_str), NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA, 0, NULL), cleanup);

    /* compare original and new value */
    if (lyd_compare_meta(meta, m2)) {
        /* values differ, switch them */
        val = meta->value;
        meta->value = m2->value;
        m2->value = val;
        val_change = 1;
    } else {
        val_change = 0;
    }

    /* retrun value */
    if (!val_change) {
        /* no change */
        ret = LY_ENOT;
    } /* else value changed, LY_SUCCESS */

cleanup:
    lyd_free_meta_single(m2);
    return ret;
}

API LY_ERR
lyd_new_path(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const char *value, uint32_t options,
        struct lyd_node **node)
{
    return lyd_new_path2(parent, ctx, path, value, 0, options, node, NULL);
}

API LY_ERR
lyd_new_path2(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const void *value,
        LYD_ANYDATA_VALUETYPE value_type, uint32_t options, struct lyd_node **new_parent, struct lyd_node **new_node)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lyxp_expr *exp = NULL;
    struct ly_path *p = NULL;
    struct lyd_node *nparent = NULL, *nnode = NULL, *node = NULL, *cur_parent;
    const struct lysc_node *schema;
    LY_ARRAY_COUNT_TYPE path_idx = 0;
    struct ly_path_predicate *pred;

    LY_CHECK_ARG_RET(ctx, parent || ctx, path, (path[0] == '/') || parent, LY_EINVAL);

    if (!ctx) {
        ctx = LYD_CTX(parent);
    }

    /* parse path */
    LY_CHECK_GOTO(ret = ly_path_parse(ctx, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_FALSE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_SIMPLE, &exp), cleanup);

    /* compile path */
    LY_CHECK_GOTO(ret = ly_path_compile(ctx, NULL, parent ? parent->schema : NULL, exp, LY_PATH_LREF_FALSE,
            options & LYD_NEW_PATH_OUTPUT ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY, LY_PREF_JSON,
            NULL, NULL, &p), cleanup);

    schema = p[LY_ARRAY_COUNT(p) - 1].node;
    if ((schema->nodetype == LYS_LIST) && (p[LY_ARRAY_COUNT(p) - 1].pred_type == LY_PATH_PREDTYPE_NONE) &&
            !(options & LYD_NEW_PATH_OPAQ)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_XPATH, "Predicate missing for %s \"%s\" in path.",
                lys_nodetype2str(schema->nodetype), schema->name);
        ret = LY_EINVAL;
        goto cleanup;
    } else if ((schema->nodetype == LYS_LEAFLIST) && (p[LY_ARRAY_COUNT(p) - 1].pred_type == LY_PATH_PREDTYPE_NONE)) {
        /* parse leafref value into a predicate, if not defined in the path */
        if (!value) {
            value = "";
        }

        r = LY_SUCCESS;
        if (options & LYD_NEW_PATH_OPAQ) {
            r = lys_value_validate(NULL, schema, value, strlen(value));
        }
        if (!r) {
            /* store the new predicate */
            p[LY_ARRAY_COUNT(p) - 1].pred_type = LY_PATH_PREDTYPE_LEAFLIST;
            LY_ARRAY_NEW_GOTO(ctx, p[LY_ARRAY_COUNT(p) - 1].predicates, pred, ret, cleanup);

            ret = lyd_value_store(ctx, &pred->value, ((struct lysc_node_leaflist *)schema)->type, value, strlen(value),
                    NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA, schema, NULL, LY_VLOG_LYSC, schema);
            LY_CHECK_GOTO(ret, cleanup);
            ++((struct lysc_type *)pred->value.realtype)->refcount;
        } /* else we have opaq flag and the value is not valid, leavne no predicate and then create an opaque node */
    }

    /* try to find any existing nodes in the path */
    if (parent) {
        ret = ly_path_eval_partial(p, parent, &path_idx, &node);
        if (ret == LY_SUCCESS) {
            /* the node exists, are we supposed to update it or is it just a default? */
            if (!(options & LYD_NEW_PATH_UPDATE) && !(node->flags & LYD_DEFAULT)) {
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
            if (lysc_data_parent(p[0].node)) {
                node = parent;
            }
        } else {
            /* error */
            goto cleanup;
        }
    }

    /* create all the non-existing nodes in a loop */
    for ( ; path_idx < LY_ARRAY_COUNT(p); ++path_idx) {
        cur_parent = node;
        schema = p[path_idx].node;

        switch (schema->nodetype) {
        case LYS_LIST:
            if (!(schema->flags & LYS_KEYLESS)) {
                if ((options & LYD_NEW_PATH_OPAQ) && (p[path_idx].pred_type == LY_PATH_PREDTYPE_NONE)) {
                    /* creating opaque list without keys */
                    LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), NULL, 0,
                            schema->module->name, strlen(schema->module->name), NULL, 0, NULL, LY_PREF_JSON, NULL,
                            LYD_NODEHINT_LIST, &node), cleanup);
                } else {
                    assert(p[path_idx].pred_type == LY_PATH_PREDTYPE_LIST);
                    LY_CHECK_GOTO(ret = lyd_create_list(schema, p[path_idx].predicates, &node), cleanup);
                }
                break;
            }
        /* fall through */
        case LYS_CONTAINER:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            LY_CHECK_GOTO(ret = lyd_create_inner(schema, &node), cleanup);
            break;
        case LYS_LEAFLIST:
            if ((options & LYD_NEW_PATH_OPAQ) && (p[path_idx].pred_type == LY_PATH_PREDTYPE_NONE)) {
                /* creating opaque leaf-list without value */
                LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), NULL, 0,
                        schema->module->name, strlen(schema->module->name), NULL, 0, NULL, LY_PREF_JSON, NULL,
                        LYD_NODEHINT_LEAFLIST, &node), cleanup);
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
            if (options & LYD_NEW_PATH_OPAQ) {
                r = lys_value_validate(NULL, schema, value, strlen(value));
            }
            if (!r) {
                ret = lyd_create_term(schema, value, strlen(value), NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA, NULL, &node);
                LY_CHECK_GOTO(ret, cleanup);
            } else {
                /* creating opaque leaf without value */
                LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), NULL, 0, schema->module->name,
                        strlen(schema->module->name), NULL, 0, NULL, LY_PREF_JSON, NULL, 0, &node), cleanup);
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

LY_ERR
lyd_new_implicit_r(struct lyd_node *parent, struct lyd_node **first, const struct lysc_node *sparent,
        const struct lys_module *mod, struct ly_set *node_types, struct ly_set *node_when, uint32_t impl_opts,
        struct lyd_node **diff)
{
    LY_ERR ret;
    const struct lysc_node *iter = NULL;
    struct lyd_node *node = NULL;
    struct lyd_value **dflts;
    LY_ARRAY_COUNT_TYPE u;

    assert(first && (parent || sparent || mod));

    if (!sparent && parent) {
        sparent = parent->schema;
    }

    while ((iter = lys_getnext(iter, sparent, mod ? mod->compiled : NULL, LYS_GETNEXT_WITHCHOICE))) {
        if ((impl_opts & LYD_IMPLICIT_NO_STATE) && (iter->flags & LYS_CONFIG_R)) {
            continue;
        } else if ((impl_opts & LYD_IMPLICIT_NO_CONFIG) && (iter->flags & LYS_CONFIG_W)) {
            continue;
        }

        switch (iter->nodetype) {
        case LYS_CHOICE:
            node = lys_getnext_data(NULL, *first, NULL, iter, NULL);
            if (!node && ((struct lysc_node_choice *)iter)->dflt) {
                /* create default case data */
                LY_CHECK_RET(lyd_new_implicit_r(parent, first, (struct lysc_node *)((struct lysc_node_choice *)iter)->dflt,
                        NULL, node_types, node_when, impl_opts, diff));
            } else if (node) {
                /* create any default data in the existing case */
                assert(node->schema->parent->nodetype == LYS_CASE);
                LY_CHECK_RET(lyd_new_implicit_r(parent, first, node->schema->parent, NULL, node_types, node_when,
                        impl_opts, diff));
            }
            break;
        case LYS_CONTAINER:
            if (!(iter->flags & LYS_PRESENCE) && lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default NP container */
                LY_CHECK_RET(lyd_create_inner(iter, &node));
                node->flags = LYD_DEFAULT | (node->schema->when ? LYD_WHEN_TRUE : 0);
                lyd_insert_node(parent, first, node);

                /* cannot be a NP container with when */
                assert(!iter->when);

                if (diff) {
                    /* add into diff */
                    LY_CHECK_RET(lyd_val_diff_add(node, LYD_DIFF_OP_CREATE, diff));
                }

                /* create any default children */
                LY_CHECK_RET(lyd_new_implicit_r(node, lyd_node_children_p(node), NULL, NULL, node_types, node_when,
                        impl_opts, diff));
            }
            break;
        case LYS_LEAF:
            if (!(impl_opts & LYD_IMPLICIT_NO_DEFAULTS) && ((struct lysc_node_leaf *)iter)->dflt &&
                    lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default leaf */
                ret = lyd_create_term2(iter, ((struct lysc_node_leaf *)iter)->dflt, &node);
                if (ret == LY_EINCOMPLETE) {
                    if (node_types) {
                        /* remember to resolve type */
                        LY_CHECK_RET(ly_set_add(node_types, node, 1, NULL));
                    }
                } else if (ret) {
                    return ret;
                }
                node->flags = LYD_DEFAULT | (node->schema->when ? LYD_WHEN_TRUE : 0);
                lyd_insert_node(parent, first, node);

                if (iter->when && node_when) {
                    /* remember to resolve when */
                    LY_CHECK_RET(ly_set_add(node_when, node, 1, NULL));
                }
                if (diff) {
                    /* add into diff */
                    LY_CHECK_RET(lyd_val_diff_add(node, LYD_DIFF_OP_CREATE, diff));
                }
            }
            break;
        case LYS_LEAFLIST:
            if (!(impl_opts & LYD_IMPLICIT_NO_DEFAULTS) && ((struct lysc_node_leaflist *)iter)->dflts &&
                    lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create all default leaf-lists */
                dflts = ((struct lysc_node_leaflist *)iter)->dflts;
                LY_ARRAY_FOR(dflts, u) {
                    ret = lyd_create_term2(iter, dflts[u], &node);
                    if (ret == LY_EINCOMPLETE) {
                        if (node_types) {
                            /* remember to resolve type */
                            LY_CHECK_RET(ly_set_add(node_types, node, 1, NULL));
                        }
                    } else if (ret) {
                        return ret;
                    }
                    node->flags = LYD_DEFAULT | (node->schema->when ? LYD_WHEN_TRUE : 0);
                    lyd_insert_node(parent, first, node);

                    if (iter->when && node_when) {
                        /* remember to resolve when */
                        LY_CHECK_RET(ly_set_add(node_when, node, 1, NULL));
                    }
                    if (diff) {
                        /* add into diff */
                        LY_CHECK_RET(lyd_val_diff_add(node, LYD_DIFF_OP_CREATE, diff));
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

API LY_ERR
lyd_new_implicit_tree(struct lyd_node *tree, uint32_t implicit_options, struct lyd_node **diff)
{
    struct lyd_node *node;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, tree, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }

    LYD_TREE_DFS_BEGIN(tree, node) {
        /* skip added default nodes */
        if (((node->flags & (LYD_DEFAULT | LYD_NEW)) != (LYD_DEFAULT | LYD_NEW)) &&
                (node->schema->nodetype & LYD_NODE_INNER)) {
            LY_CHECK_GOTO(ret = lyd_new_implicit_r(node, lyd_node_children_p((struct lyd_node *)node), NULL, NULL, NULL,
                    NULL, implicit_options, diff), cleanup);
        }

        LYD_TREE_DFS_END(tree, node);
    }

cleanup:
    if (ret && diff) {
        lyd_free_all(*diff);
        *diff = NULL;
    }
    return ret;
}

API LY_ERR
lyd_new_implicit_all(struct lyd_node **tree, const struct ly_ctx *ctx, uint32_t implicit_options, struct lyd_node **diff)
{
    const struct lys_module *mod;
    struct lyd_node *d = NULL;
    uint32_t i = 0;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, tree, *tree || ctx, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }
    if (!ctx) {
        ctx = LYD_CTX(*tree);
    }

    /* add nodes for each module one-by-one */
    while ((mod = ly_ctx_get_module_iter(ctx, &i))) {
        if (!mod->implemented) {
            continue;
        }

        LY_CHECK_GOTO(ret = lyd_new_implicit_module(tree, mod, implicit_options, diff ? &d : NULL), cleanup);
        if (d) {
            /* merge into one diff */
            lyd_insert_sibling(*diff, d, diff);

            d = NULL;
        }
    }

cleanup:
    if (ret && diff) {
        lyd_free_all(*diff);
        *diff = NULL;
    }
    return ret;
}

API LY_ERR
lyd_new_implicit_module(struct lyd_node **tree, const struct lys_module *module, uint32_t implicit_options, struct lyd_node **diff)
{
    struct lyd_node *root, *d = NULL;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, tree, module, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }

    /* add all top-level defaults for this module */
    LY_CHECK_GOTO(ret = lyd_new_implicit_r(NULL, tree, NULL, module, NULL, NULL, implicit_options, diff), cleanup);

    /* process nested nodes */
    LY_LIST_FOR(*tree, root) {
        /* skip added default nodes */
        if ((root->flags & (LYD_DEFAULT | LYD_NEW)) != (LYD_DEFAULT | LYD_NEW)) {
            LY_CHECK_GOTO(ret = lyd_new_implicit_tree(root, implicit_options, diff ? &d : NULL), cleanup);

            if (d) {
                /* merge into one diff */
                lyd_insert_sibling(*diff, d, diff);

                d = NULL;
            }
        }
    }

cleanup:
    if (ret && diff) {
        lyd_free_all(*diff);
        *diff = NULL;
    }
    return ret;
}

struct lyd_node *
lyd_insert_get_next_anchor(const struct lyd_node *first_sibling, const struct lyd_node *new_node)
{
    const struct lysc_node *schema, *sparent;
    struct lyd_node *match = NULL;
    ly_bool found;

    assert(new_node);

    if (!first_sibling || !new_node->schema) {
        /* insert at the end, no next anchor */
        return NULL;
    }

    if (first_sibling->parent && first_sibling->parent->children_ht) {
        /* find the anchor using hashes */
        sparent = first_sibling->parent->schema;
        schema = lys_getnext(new_node->schema, sparent, NULL, 0);
        while (schema) {
            /* keep trying to find the first existing instance of the closest following schema sibling,
             * otherwise return NULL - inserting at the end */
            if (!lyd_find_sibling_schema(first_sibling, schema, &match)) {
                break;
            }

            schema = lys_getnext(schema, sparent, NULL, 0);
        }
    } else {
        /* find the anchor without hashes */
        match = (struct lyd_node *)first_sibling;
        if (!lysc_data_parent(new_node->schema)) {
            /* we are in top-level, skip all the data from preceding modules */
            LY_LIST_FOR(match, match) {
                if (!match->schema || (strcmp(lyd_owner_module(match)->name, lyd_owner_module(new_node)->name) >= 0)) {
                    break;
                }
            }
        }

        /* get the first schema sibling */
        sparent = lysc_data_parent(new_node->schema);
        schema = lys_getnext(NULL, sparent, new_node->schema->module->compiled, 0);

        found = 0;
        LY_LIST_FOR(match, match) {
            if (!match->schema || (lyd_owner_module(match) != lyd_owner_module(new_node))) {
                /* we have found an opaque node, which must be at the end, so use it OR
                 * modules do not match, so we must have traversed all the data from new_node module (if any),
                 * we have found the first node of the next module, that is what we want */
                break;
            }

            /* skip schema nodes until we find the instantiated one */
            while (!found) {
                if (new_node->schema == schema) {
                    /* we have found the schema of the new node, continue search to find the first
                     * data node with a different schema (after our schema) */
                    found = 1;
                    break;
                }
                if (match->schema == schema) {
                    /* current node (match) is a data node still before the new node, continue search in data */
                    break;
                }
                schema = lys_getnext(schema, sparent, new_node->schema->module->compiled, 0);
                assert(schema);
            }

            if (found && (match->schema != new_node->schema)) {
                /* find the next node after we have found our node schema data instance */
                break;
            }
        }
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
            for ( ; sibling->prev->next != node; sibling = sibling->prev) {}
        }
        sibling->prev = node;
    }
    node->parent = sibling->parent;

    for (par = node->parent; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
        if (par->schema && (par->schema->nodetype == LYS_LIST) && (par->schema->flags & LYS_KEYLESS)) {
            /* rehash key-less list */
            lyd_hash((struct lyd_node *)par);
        }
    }
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
        if (par->schema && (par->schema->nodetype == LYS_LIST) && (par->schema->flags & LYS_KEYLESS)) {
            /* rehash key-less list */
            lyd_hash((struct lyd_node *)par);
        }
    }
}

/**
 * @brief Insert node as the first and only child of a parent.
 *
 * Handles inserting into NP containers and key-less lists.
 *
 * @param[in] parent Parent to insert into.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_only_child(struct lyd_node *parent, struct lyd_node *node)
{
    struct lyd_node_inner *par;

    assert(parent && !lyd_child(parent) && !node->next && (node->prev == node));
    assert(!parent->schema || (parent->schema->nodetype & LYD_NODE_INNER));

    par = (struct lyd_node_inner *)parent;

    par->child = node;
    node->parent = par;

    for ( ; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
        if (par->schema && (par->schema->nodetype == LYS_LIST) && (par->schema->flags & LYS_KEYLESS)) {
            /* rehash key-less list */
            lyd_hash((struct lyd_node *)par);
        }
    }
}

/**
 * @brief Learn whether a list instance has all the keys.
 *
 * @param[in] list List instance to check.
 * @return non-zero if all the keys were found,
 * @return 0 otherwise.
 */
static int
lyd_insert_has_keys(const struct lyd_node *list)
{
    const struct lyd_node *key;
    const struct lysc_node *skey = NULL;

    assert(list->schema->nodetype == LYS_LIST);
    key = lyd_child(list);
    while ((skey = lys_getnext(skey, list->schema, NULL, 0)) && (skey->flags & LYS_KEY)) {
        if (!key || (key->schema != skey)) {
            /* key missing */
            return 0;
        }

        key = key->next;
    }

    /* all keys found */
    return 1;
}

void
lyd_insert_node(struct lyd_node *parent, struct lyd_node **first_sibling_p, struct lyd_node *node)
{
    struct lyd_node *anchor, *first_sibling;

    /* inserting list without its keys is not supported */
    assert((parent || first_sibling_p) && node && (node->hash || !node->schema));
    assert(!parent || !parent->schema ||
            (parent->schema->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_ACTION | LYS_NOTIF)));

    if (!parent && first_sibling_p && (*first_sibling_p) && (*first_sibling_p)->parent) {
        parent = (struct lyd_node *)(*first_sibling_p)->parent;
    }

    /* get first sibling */
    first_sibling = parent ? ((struct lyd_node_inner *)parent)->child : *first_sibling_p;

    /* find the anchor, our next node, so we can insert before it */
    anchor = lyd_insert_get_next_anchor(first_sibling, node);
    if (anchor) {
        lyd_insert_before_node(anchor, node);
        if (!parent && (*first_sibling_p == anchor)) {
            /* move first sibling */
            *first_sibling_p = node;
        }
    } else if (first_sibling) {
        lyd_insert_after_node(first_sibling->prev, node);
    } else if (parent) {
        lyd_insert_only_child(parent, node);
    } else {
        *first_sibling_p = node;
    }

    /* insert into parent HT */
    lyd_insert_hash(node);

    /* finish hashes for our parent, if needed and possible */
    if (node->schema && (node->schema->flags & LYS_KEY) && parent && lyd_insert_has_keys(parent)) {
        lyd_hash(parent);

        /* now we can insert even the list into its parent HT */
        lyd_insert_hash(parent);
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
            LOGERR(schema->module->ctx, LY_EINVAL, "Cannot insert, parent of \"%s\" is not \"%s\".", schema->name,
                    parent->name);
            return LY_EINVAL;
        }
    } else {
        /* top-level node */
        if (par2) {
            LOGERR(schema->module->ctx, LY_EINVAL, "Cannot insert, node \"%s\" is not top-level.", schema->name);
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_insert_child(struct lyd_node *parent, struct lyd_node *node)
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
lyd_insert_sibling(struct lyd_node *sibling, struct lyd_node *node, struct lyd_node **first)
{
    struct lyd_node *iter;

    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);

    if (sibling) {
        LY_CHECK_RET(lyd_insert_check_schema(lysc_data_parent(sibling->schema), node->schema));
    }

    if (node->parent || node->prev->next) {
        lyd_unlink_tree(node);
    }

    while (node) {
        if (node->schema->flags & LYS_KEY) {
            LOGERR(LYD_CTX(node), LY_EINVAL, "Cannot insert key \"%s\".", node->schema->name);
            return LY_EINVAL;
        }

        iter = node->next;
        lyd_unlink_tree(node);
        lyd_insert_node(NULL, &sibling, node);
        node = iter;
    }

    if (first) {
        /* find the first sibling */
        *first = sibling;
        while ((*first)->prev->next) {
            *first = (*first)->prev;
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

    if (!(node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(node->schema->flags & LYS_ORDBY_USER)) {
        LOGERR(LYD_CTX(sibling), LY_EINVAL, "Can be used only for user-ordered nodes.");
        return LY_EINVAL;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink_tree(node);
    }

    /* insert in reverse order to get the original order */
    node = node->prev;
    while (node) {
        iter = node->prev;
        lyd_unlink_tree(node);

        lyd_insert_before_node(sibling, node);
        lyd_insert_hash(node);

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

    if (!(node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(node->schema->flags & LYS_ORDBY_USER)) {
        LOGERR(LYD_CTX(sibling), LY_EINVAL, "Can be used only for user-ordered nodes.");
        return LY_EINVAL;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink_tree(node);
    }

    while (node) {
        iter = node->next;
        lyd_unlink_tree(node);

        lyd_insert_after_node(sibling, node);
        lyd_insert_hash(node);

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

    /* update hashes while still linked into the tree */
    lyd_unlink_hash(node);

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

        /* check for NP container whether its last non-default node is not being unlinked */
        if (node->parent->schema && (node->parent->schema->nodetype == LYS_CONTAINER) &&
                !(node->parent->flags & LYD_DEFAULT) && !(node->parent->schema->flags & LYS_PRESENCE)) {
            LY_LIST_FOR(node->parent->child, iter) {
                if ((iter != node) && !(iter->flags & LYD_DEFAULT)) {
                    break;
                }
            }
            if (!iter) {
                node->parent->flags |= LYD_DEFAULT;
            }
        }

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

void
lyd_insert_meta(struct lyd_node *parent, struct lyd_meta *meta, ly_bool clear_dflt)
{
    struct lyd_meta *last, *iter;

    assert(parent);

    if (!meta) {
        return;
    }

    for (iter = meta; iter; iter = iter->next) {
        iter->parent = parent;
    }

    /* insert as the last attribute */
    if (parent->meta) {
        for (last = parent->meta; last->next; last = last->next) {}
        last->next = meta;
    } else {
        parent->meta = meta;
    }

    /* remove default flags from NP containers */
    while (clear_dflt && parent && (parent->schema->nodetype == LYS_CONTAINER) && (parent->flags & LYD_DEFAULT)) {
        parent->flags &= ~LYD_DEFAULT;
        parent = (struct lyd_node *)parent->parent;
    }
}

LY_ERR
lyd_create_meta(struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod, const char *name,
        size_t name_len, const char *value, size_t value_len, ly_bool *dynamic, LY_PREFIX_FORMAT format,
        void *prefix_data, uint32_t hints, ly_bool clear_dflt, ly_bool *incomplete)
{
    LY_ERR rc;
    struct lysc_ext_instance *ant = NULL;
    struct lyd_meta *mt, *last;
    LY_ARRAY_COUNT_TYPE u;

    assert((parent || meta) && mod);

    LY_ARRAY_FOR(mod->compiled->exts, u) {
        if ((mod->compiled->exts[u].def->plugin == lyext_plugins_internal[LYEXT_PLUGIN_INTERNAL_ANNOTATION].plugin) &&
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
    rc = lyd_value_store(mod->ctx, &mt->value, ((struct lyext_metadata *)ant->data)->type, value, value_len, dynamic,
            format, prefix_data, hints, parent ? parent->schema : NULL, incomplete, LY_VLOG_NONE, NULL);
    LY_CHECK_ERR_RET(rc, free(mt), rc);
    rc = lydict_insert(mod->ctx, name, name_len, &mt->name);
    LY_CHECK_ERR_RET(rc, free(mt), rc);

    /* insert as the last attribute */
    if (parent) {
        lyd_insert_meta(parent, mt, clear_dflt);
    } else if (*meta) {
        for (last = *meta; last->next; last = last->next) {}
        last->next = mt;
    }

    if (meta) {
        *meta = mt;
    }
    return LY_SUCCESS;
}

void
lyd_insert_attr(struct lyd_node *parent, struct lyd_attr *attr)
{
    struct lyd_attr *last, *iter;
    struct lyd_node_opaq *opaq;

    assert(parent && !parent->schema);

    if (!attr) {
        return;
    }

    opaq = (struct lyd_node_opaq *)parent;
    for (iter = attr; iter; iter = iter->next) {
        iter->parent = opaq;
    }

    /* insert as the last attribute */
    if (opaq->attr) {
        for (last = opaq->attr; last->next; last = last->next) {}
        last->next = attr;
    } else {
        opaq->attr = attr;
    }
}

LY_ERR
lyd_create_attr(struct lyd_node *parent, struct lyd_attr **attr, const struct ly_ctx *ctx, const char *name, size_t name_len,
        const char *prefix, size_t prefix_len, const char *module_key, size_t module_key_len, const char *value,
        size_t value_len, ly_bool *dynamic, LY_PREFIX_FORMAT format, void *val_prefix_data, uint32_t hints)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_attr *at, *last;

    assert(ctx && (parent || attr) && (!parent || !parent->schema));
    assert(name && name_len && format);

    if (!value_len) {
        value = "";
    }

    at = calloc(1, sizeof *at);
    LY_CHECK_ERR_RET(!at, LOGMEM(ctx); ly_free_prefix_data(format, val_prefix_data), LY_EMEM);

    LY_CHECK_GOTO(ret = lydict_insert(ctx, name, name_len, &at->name.name), finish);
    if (prefix_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, prefix, prefix_len, &at->name.prefix), finish);
    }
    if (module_key_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, module_key, module_key_len, &at->name.module_ns), finish);
    }

    if (dynamic && *dynamic) {
        ret = lydict_insert_zc(ctx, (char *)value, &at->value);
        LY_CHECK_GOTO(ret, finish);
        *dynamic = 0;
    } else {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, value, value_len, &at->value), finish);
    }
    at->format = format;
    at->val_prefix_data = val_prefix_data;
    at->hints = hints;

    /* insert as the last attribute */
    if (parent) {
        lyd_insert_attr(parent, at);
    } else if (*attr) {
        for (last = *attr; last->next; last = last->next) {}
        last->next = at;
    }

finish:
    if (ret) {
        lyd_free_attr_single(ctx, at);
    } else if (attr) {
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
lyd_compare_single(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options)
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

    if ((LYD_CTX(node1) != LYD_CTX(node2)) || (node1->schema != node2->schema)) {
        return LY_ENOT;
    }

    if (node1->hash != node2->hash) {
        return LY_ENOT;
    }
    /* equal hashes do not mean equal nodes, they can be just in collision (or both be 0) so the nodes must be checked explicitly */

    if (!node1->schema) {
        opaq1 = (struct lyd_node_opaq *)node1;
        opaq2 = (struct lyd_node_opaq *)node2;
        if ((opaq1->name.name != opaq2->name.name) || (opaq1->format != opaq2->format) ||
                (opaq1->name.module_ns != opaq2->name.module_ns)) {
            return LY_ENOT;
        }
        switch (opaq1->format) {
        case LY_PREF_XML:
            if (lyxml_value_compare(LYD_CTX(node1), opaq1->value, opaq1->val_prefix_data, opaq2->value, opaq2->val_prefix_data)) {
                return LY_ENOT;
            }
            break;
        case LY_PREF_JSON:
            /* prefixes in JSON are unique, so it is not necessary to canonize the values */
            if (strcmp(opaq1->value, opaq2->value)) {
                return LY_ENOT;
            }
            break;
        default:
            /* not allowed */
            LOGINT(LYD_CTX(node1));
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
                iter1 = ((struct lyd_node_inner *)node1)->child;
                iter2 = ((struct lyd_node_inner *)node2)->child;
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
            iter1 = ((struct lyd_node_inner *)node1)->child;
            iter2 = ((struct lyd_node_inner *)node2)->child;

            if (!(node1->schema->flags & LYS_KEYLESS) && !(options & LYD_COMPARE_FULL_RECURSION)) {
                /* lists with keys, their equivalence is based on their keys */
                for (struct lysc_node *key = ((struct lysc_node_list *)node1->schema)->child;
                        key && (key->flags & LYS_KEY);
                        key = key->next) {
                    if (lyd_compare_single(iter1, iter2, options)) {
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

                for ( ; iter1 && iter2; iter1 = iter1->next, iter2 = iter2->next) {
                    if (lyd_compare_single(iter1, iter2, options | LYD_COMPARE_FULL_RECURSION)) {
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
            any1 = (struct lyd_node_any *)node1;
            any2 = (struct lyd_node_any *)node2;

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
                if ((len1 != len2) || strcmp(any1->value.str, any2->value.str)) {
                    return LY_ENOT;
                }
                return LY_SUCCESS;
            case LYD_ANYDATA_LYB:
                len1 = lyd_lyb_data_length(any1->value.mem);
                len2 = lyd_lyb_data_length(any2->value.mem);
                if ((len1 != len2) || memcmp(any1->value.mem, any2->value.mem, len1)) {
                    return LY_ENOT;
                }
                return LY_SUCCESS;
            }
        }
    }

    LOGINT(LYD_CTX(node1));
    return LY_EINT;
}

API LY_ERR
lyd_compare_siblings(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options)
{
    for ( ; node1 && node2; node1 = node1->next, node2 = node2->next) {
        LY_CHECK_RET(lyd_compare_single(node1, node2, options));
    }

    if (node1 == node2) {
        return LY_SUCCESS;
    }
    return LY_ENOT;
}

API LY_ERR
lyd_compare_meta(const struct lyd_meta *meta1, const struct lyd_meta *meta2)
{
    if (!meta1 || !meta2) {
        if (meta1 == meta2) {
            return LY_SUCCESS;
        } else {
            return LY_ENOT;
        }
    }

    if ((meta1->annotation->module->ctx != meta2->annotation->module->ctx) || (meta1->annotation != meta2->annotation)) {
        return LY_ENOT;
    }

    if (meta1->value.realtype != meta2->value.realtype) {
        return LY_ENOT;
    }

    return meta1->value.realtype->plugin->compare(&meta1->value, &meta2->value);
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
lyd_dup_r(const struct lyd_node *node, struct lyd_node *parent, struct lyd_node **first, uint32_t options,
        struct lyd_node **dup_p)
{
    LY_ERR ret;
    struct lyd_node *dup = NULL;
    struct lyd_meta *meta;
    struct lyd_node_any *any;

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
            LOGINT(LYD_CTX(node));
            ret = LY_EINT;
            goto error;
        }
    }
    LY_CHECK_ERR_GOTO(!dup, LOGMEM(LYD_CTX(node)); ret = LY_EMEM, error);

    if (options & LYD_DUP_WITH_FLAGS) {
        dup->flags = node->flags;
    } else {
        dup->flags = (node->flags & LYD_DEFAULT) | LYD_NEW;
    }
    dup->schema = node->schema;
    dup->prev = dup;

    /* duplicate metadata */
    if (!(options & LYD_DUP_NO_META)) {
        LY_LIST_FOR(node->meta, meta) {
            LY_CHECK_GOTO(ret = lyd_dup_meta_single(meta, dup, NULL), error);
        }
    }

    /* nodetype-specific work */
    if (!dup->schema) {
        struct lyd_node_opaq *opaq = (struct lyd_node_opaq *)dup;
        struct lyd_node_opaq *orig = (struct lyd_node_opaq *)node;
        struct lyd_node *child;

        if (options & LYD_DUP_RECURSIVE) {
            /* duplicate all the children */
            LY_LIST_FOR(orig->child, child) {
                LY_CHECK_GOTO(ret = lyd_dup_r(child, dup, NULL, options, NULL), error);
            }
        }
        LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), orig->name.name, 0, &opaq->name.name), error);
        opaq->format = orig->format;
        if (orig->name.prefix) {
            LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), orig->name.prefix, 0, &opaq->name.prefix), error);
        }
        LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), orig->name.module_ns, 0, &opaq->name.module_ns), error);
        if (orig->val_prefix_data) {
            ret = ly_dup_prefix_data(LYD_CTX(node), opaq->format, orig->val_prefix_data, &opaq->val_prefix_data);
            LY_CHECK_GOTO(ret, error);
        }
        LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), orig->value, 0, &opaq->value), error);
        opaq->ctx = orig->ctx;
    } else if (dup->schema->nodetype & LYD_NODE_TERM) {
        struct lyd_node_term *term = (struct lyd_node_term *)dup;
        struct lyd_node_term *orig = (struct lyd_node_term *)node;

        term->hash = orig->hash;
        LY_CHECK_ERR_GOTO(orig->value.realtype->plugin->duplicate(LYD_CTX(node), &orig->value, &term->value),
                LOGERR(LYD_CTX(node), LY_EINT, "Value duplication failed."); ret = LY_EINT, error);
    } else if (dup->schema->nodetype & LYD_NODE_INNER) {
        struct lyd_node_inner *orig = (struct lyd_node_inner *)node;
        struct lyd_node *child;

        if (options & LYD_DUP_RECURSIVE) {
            /* duplicate all the children */
            LY_LIST_FOR(orig->child, child) {
                LY_CHECK_GOTO(ret = lyd_dup_r(child, dup, NULL, options, NULL), error);
            }
        } else if ((dup->schema->nodetype == LYS_LIST) && !(dup->schema->flags & LYS_KEYLESS)) {
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
                LY_CHECK_GOTO(ret = lyd_dup_r(child, dup, NULL, options, NULL), error);
                child = child->next;
            }
        }
        lyd_hash(dup);
    } else if (dup->schema->nodetype & LYD_NODE_ANY) {
        dup->hash = node->hash;
        any = (struct lyd_node_any *)node;
        LY_CHECK_GOTO(ret = lyd_any_copy_value(dup, &any->value, any->value_type), error);
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

static LY_ERR
lyd_dup_get_local_parent(const struct lyd_node *node, const struct lyd_node_inner *parent, struct lyd_node **dup_parent,
        struct lyd_node_inner **local_parent)
{
    const struct lyd_node_inner *orig_parent, *iter;
    ly_bool repeat = 1;

    *dup_parent = NULL;
    *local_parent = NULL;

    for (orig_parent = node->parent; repeat && orig_parent; orig_parent = orig_parent->parent) {
        if (parent && (parent->schema == orig_parent->schema)) {
            /* stop creating parents, connect what we have into the provided parent */
            iter = parent;
            repeat = 0;
        } else {
            iter = NULL;
            LY_CHECK_RET(lyd_dup_r((struct lyd_node *)orig_parent, NULL, (struct lyd_node **)&iter, 0,
                    (struct lyd_node **)&iter));
        }
        if (!*local_parent) {
            *local_parent = (struct lyd_node_inner *)iter;
        }
        if (iter->child) {
            /* 1) list - add after keys
             * 2) provided parent with some children */
            iter->child->prev->next = *dup_parent;
            if (*dup_parent) {
                (*dup_parent)->prev = iter->child->prev;
                iter->child->prev = *dup_parent;
            }
        } else {
            ((struct lyd_node_inner *)iter)->child = *dup_parent;
        }
        if (*dup_parent) {
            (*dup_parent)->parent = (struct lyd_node_inner *)iter;
        }
        *dup_parent = (struct lyd_node *)iter;
    }

    if (repeat && parent) {
        /* given parent and created parents chain actually do not interconnect */
        LOGERR(LYD_CTX(node), LY_EINVAL,
                "Invalid argument parent (%s()) - does not interconnect with the created node's parents chain.", __func__);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

static LY_ERR
lyd_dup(const struct lyd_node *node, struct lyd_node_inner *parent, uint32_t options, ly_bool nosiblings, struct lyd_node **dup)
{
    LY_ERR rc;
    const struct lyd_node *orig;          /* original node to be duplicated */
    struct lyd_node *first = NULL;        /* the first duplicated node, this is returned */
    struct lyd_node *top = NULL;          /* the most higher created node */
    struct lyd_node_inner *local_parent = NULL; /* the direct parent node for the duplicated node(s) */

    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);

    if (options & LYD_DUP_WITH_PARENTS) {
        LY_CHECK_GOTO(rc = lyd_dup_get_local_parent(node, parent, &top, &local_parent), error);
    } else {
        local_parent = parent;
    }

    LY_LIST_FOR(node, orig) {
        /* if there is no local parent, it will be inserted into first */
        LY_CHECK_GOTO(rc = lyd_dup_r(orig, (struct lyd_node *)local_parent, &first, options, first ? NULL : &first), error);
        if (nosiblings) {
            break;
        }
    }

    /* rehash if needed */
    for ( ; local_parent; local_parent = local_parent->parent) {
        if ((local_parent->schema->nodetype == LYS_LIST) && (local_parent->schema->flags & LYS_KEYLESS)) {
            lyd_hash((struct lyd_node *)local_parent);
        }
    }

    if (dup) {
        *dup = first;
    }
    return LY_SUCCESS;

error:
    if (top) {
        lyd_free_tree(top);
    } else {
        lyd_free_siblings(first);
    }
    return rc;
}

API LY_ERR
lyd_dup_single(const struct lyd_node *node, struct lyd_node_inner *parent, uint32_t options, struct lyd_node **dup)
{
    return lyd_dup(node, parent, options, 1, dup);
}

API LY_ERR
lyd_dup_siblings(const struct lyd_node *node, struct lyd_node_inner *parent, uint32_t options, struct lyd_node **dup)
{
    return lyd_dup(node, parent, options, 0, dup);
}

API LY_ERR
lyd_dup_meta_single(const struct lyd_meta *meta, struct lyd_node *node, struct lyd_meta **dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *mt, *last;

    LY_CHECK_ARG_RET(NULL, meta, node, LY_EINVAL);

    /* create a copy */
    mt = calloc(1, sizeof *mt);
    LY_CHECK_ERR_RET(!mt, LOGMEM(LYD_CTX(node)), LY_EMEM);
    mt->annotation = meta->annotation;
    ret = meta->value.realtype->plugin->duplicate(LYD_CTX(node), &meta->value, &mt->value);
    LY_CHECK_ERR_GOTO(ret, LOGERR(LYD_CTX(node), LY_EINT, "Value duplication failed."), finish);
    LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), meta->name, 0, &mt->name), finish);

    /* insert as the last attribute */
    mt->parent = node;
    if (node->meta) {
        for (last = node->meta; last->next; last = last->next) {}
        last->next = mt;
    } else {
        node->meta = mt;
    }

finish:
    if (ret) {
        lyd_free_meta_single(mt);
    } else if (dup) {
        *dup = mt;
    }
    return LY_SUCCESS;
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
        uint16_t options)
{
    LY_ERR ret;
    const struct lyd_node *child_src, *tmp, *sibling_src;
    struct lyd_node *match_trg, *dup_src, *elem;
    struct lysc_type *type;

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
        if ((match_trg->schema->nodetype == LYS_LEAF) && lyd_compare_single(sibling_src, match_trg, LYD_COMPARE_DEFAULTS)) {
            /* since they are different, they cannot both be default */
            assert(!(sibling_src->flags & LYD_DEFAULT) || !(match_trg->flags & LYD_DEFAULT));

            /* update value (or only LYD_DEFAULT flag) only if flag set or the source node is not default */
            if ((options & LYD_MERGE_DEFAULTS) || !(sibling_src->flags & LYD_DEFAULT)) {
                type = ((struct lysc_node_leaf *)match_trg->schema)->type;
                type->plugin->free(LYD_CTX(match_trg), &((struct lyd_node_term *)match_trg)->value);
                LY_CHECK_RET(type->plugin->duplicate(LYD_CTX(match_trg), &((struct lyd_node_term *)sibling_src)->value,
                        &((struct lyd_node_term *)match_trg)->value));

                /* copy flags and add LYD_NEW */
                match_trg->flags = sibling_src->flags | LYD_NEW;
            }
        } else if ((match_trg->schema->nodetype & LYS_ANYDATA) && lyd_compare_single(sibling_src, match_trg, 0)) {
            /* update value */
            LY_CHECK_RET(lyd_any_copy_value(match_trg, &((struct lyd_node_any *)sibling_src)->value,
                    ((struct lyd_node_any *)sibling_src)->value_type));

            /* copy flags and add LYD_NEW */
            match_trg->flags = sibling_src->flags | LYD_NEW;
        } else {
            /* check descendants, recursively */
            LY_LIST_FOR_SAFE(lyd_child_no_keys(sibling_src), tmp, child_src) {
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
            LY_CHECK_RET(lyd_dup_single(sibling_src, NULL, LYD_DUP_RECURSIVE | LYD_DUP_WITH_FLAGS, &dup_src));
        }

        /* set LYD_NEW for all the new nodes, required for validation */
        LYD_TREE_DFS_BEGIN(dup_src, elem) {
            elem->flags |= LYD_NEW;
            LYD_TREE_DFS_END(dup_src, elem);
        }

        lyd_insert_node(parent_trg, first_trg, dup_src);
    }

    return LY_SUCCESS;
}

static LY_ERR
lyd_merge(struct lyd_node **target, const struct lyd_node *source, uint16_t options, ly_bool nosiblings)
{
    const struct lyd_node *sibling_src, *tmp;
    ly_bool first;

    LY_CHECK_ARG_RET(NULL, target, LY_EINVAL);

    if (!source) {
        /* nothing to merge */
        return LY_SUCCESS;
    }

    if ((*target && lysc_data_parent((*target)->schema)) || lysc_data_parent(source->schema)) {
        LOGERR(LYD_CTX(source), LY_EINVAL, "Invalid arguments - can merge only 2 top-level subtrees (%s()).", __func__);
        return LY_EINVAL;
    }

    LY_LIST_FOR_SAFE(source, tmp, sibling_src) {
        first = (sibling_src == source) ? 1 : 0;
        LY_CHECK_RET(lyd_merge_sibling_r(target, NULL, &sibling_src, options));
        if (first && !sibling_src) {
            /* source was spent (unlinked), move to the next node */
            source = tmp;
        }

        if (nosiblings) {
            break;
        }
    }

    if (options & LYD_MERGE_DESTRUCT) {
        /* free any leftover source data that were not merged */
        lyd_free_siblings((struct lyd_node *)source);
    }

    return LY_SUCCESS;
}

API LY_ERR
lyd_merge_tree(struct lyd_node **target, const struct lyd_node *source, uint16_t options)
{
    return lyd_merge(target, source, options, 1);
}

API LY_ERR
lyd_merge_siblings(struct lyd_node **target, const struct lyd_node *source, uint16_t options)
{
    return lyd_merge(target, source, options, 0);
}

static LY_ERR
lyd_path_str_enlarge(char **buffer, size_t *buflen, size_t reqlen, ly_bool is_static)
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

LY_ERR
lyd_path_list_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static)
{
    const struct lyd_node *key;
    size_t len;
    const char *val;
    char quot;

    for (key = lyd_child(node); key && (key->schema->flags & LYS_KEY); key = key->next) {
        val = LYD_CANON_VALUE(key);
        len = 1 + strlen(key->schema->name) + 2 + strlen(val) + 2;
        LY_CHECK_RET(lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static));

        quot = '\'';
        if (strchr(val, '\'')) {
            quot = '"';
        }
        *bufused += sprintf(*buffer + *bufused, "[%s=%c%s%c]", key->schema->name, quot, val, quot);
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
lyd_path_leaflist_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static)
{
    size_t len;
    const char *val;
    char quot;

    val = LYD_CANON_VALUE(node);
    len = 4 + strlen(val) + 2; /* "[.='" + val + "']" */
    LY_CHECK_RET(lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static));

    quot = '\'';
    if (strchr(val, '\'')) {
        quot = '"';
    }
    *bufused += sprintf(*buffer + *bufused, "[.=%c%s%c]", quot, val, quot);

    return LY_SUCCESS;
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
lyd_path_position_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static)
{
    const struct lyd_node *first, *iter;
    size_t len;
    uint64_t pos;
    char *val = NULL;
    LY_ERR rc;

    if (node->parent) {
        first = node->parent->child;
    } else {
        for (first = node; first->prev->next; first = first->prev) {}
    }
    pos = 1;
    for (iter = first; iter != node; iter = iter->next) {
        if (iter->schema == node->schema) {
            ++pos;
        }
    }
    if (asprintf(&val, "%" PRIu64, pos) == -1) {
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
    ly_bool is_static = 0;
    uint32_t i, depth;
    size_t bufused = 0, len;
    const struct lyd_node *iter;
    const struct lys_module *mod;
    LY_ERR rc = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, node, NULL);
    if (buffer) {
        LY_CHECK_ARG_RET(node->schema->module->ctx, buflen > 1, NULL);
        is_static = 1;
    } else {
        buflen = 0;
    }

    switch (pathtype) {
    case LYD_PATH_LOG:
    case LYD_PATH_LOG_NO_LAST_PRED:
        depth = 1;
        for (iter = node; iter->parent; iter = (const struct lyd_node *)iter->parent) {
            ++depth;
        }

        goto iter_print;
        while (depth) {
            /* find the right node */
            for (iter = node, i = 1; i < depth; iter = (const struct lyd_node *)iter->parent, ++i) {}
iter_print:
            /* print prefix and name */
            mod = NULL;
            if (iter->schema && (!iter->parent || (iter->schema->module != iter->parent->schema->module))) {
                mod = iter->schema->module;
            }

            /* realloc string */
            len = 1 + (mod ? strlen(mod->name) + 1 : 0) + (iter->schema ? strlen(iter->schema->name) :
                    strlen(((struct lyd_node_opaq *)iter)->name.name));
            rc = lyd_path_str_enlarge(&buffer, &buflen, bufused + len, is_static);
            if (rc != LY_SUCCESS) {
                break;
            }

            /* print next node */
            bufused += sprintf(buffer + bufused, "/%s%s%s", mod ? mod->name : "", mod ? ":" : "",
                    iter->schema ? iter->schema->name : ((struct lyd_node_opaq *)iter)->name.name);

            /* do not always print the last (first) predicate */
            if (iter->schema && ((depth > 1) || (pathtype == LYD_PATH_LOG))) {
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
                    break;
                }
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

API struct lyd_meta *
lyd_find_meta(const struct lyd_meta *first, const struct lys_module *module, const char *name)
{
    struct lyd_meta *ret = NULL;
    const struct ly_ctx *ctx;
    const char *prefix, *tmp;
    char *str;
    size_t pref_len, name_len;

    LY_CHECK_ARG_RET(NULL, module || strchr(name, ':'), name, NULL);

    if (!first) {
        return NULL;
    }

    ctx = first->annotation->module->ctx;

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Metadata name \"%s\" is not valid.", name);
        return NULL;
    }

    /* find the module */
    if (prefix) {
        str = strndup(prefix, pref_len);
        module = ly_ctx_get_module_latest(ctx, str);
        free(str);
        LY_CHECK_ERR_RET(!module, LOGERR(ctx, LY_EINVAL, "Module \"%.*s\" not found.", pref_len, prefix), NULL);
    }

    /* find the metadata */
    LY_LIST_FOR(first, first) {
        if ((first->annotation->module == module) && !strcmp(first->name, name)) {
            ret = (struct lyd_meta *)first;
            break;
        }
    }

    return ret;
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
            /* check even value when needed */
            if (!(target->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !lyd_compare_single(target, *match_p, 0)) {
                siblings = *match_p;
            } else {
                siblings = NULL;
            }
        } else {
            /* not found */
            siblings = NULL;
        }
    } else {
        /* no children hash table */
        for ( ; siblings; siblings = siblings->next) {
            if (!lyd_compare_single(siblings, target, 0)) {
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

/**
 * @brief Comparison callback to match schema node with a schema of a data node.
 *
 * @param[in] val1_p Pointer to the schema node
 * @param[in] val2_p Pointer to the data node
 * Implementation of ::values_equal_cb.
 */
static ly_bool
lyd_hash_table_schema_val_equal(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct lysc_node *val1;
    struct lyd_node *val2;

    val1 = *((struct lysc_node **)val1_p);
    val2 = *((struct lyd_node **)val2_p);

    if (val1 == val2->schema) {
        /* schema match is enough */
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Search in the given siblings (NOT recursively) for the first schema node data instance.
 * Uses hashes - should be used whenever possible for best performance.
 *
 * @param[in] siblings Siblings to search in including preceding and succeeding nodes.
 * @param[in] schema Target data node schema to find.
 * @param[out] match Can be NULL, otherwise the found data node.
 * @return LY_SUCCESS on success, @p match set.
 * @return LY_ENOTFOUND if not found, @p match set to NULL.
 * @return LY_ERR value if another error occurred.
 */
static LY_ERR
lyd_find_sibling_schema(const struct lyd_node *siblings, const struct lysc_node *schema, struct lyd_node **match)
{
    struct lyd_node **match_p;
    struct lyd_node_inner *parent;
    uint32_t hash;
    values_equal_cb ht_cb;

    assert(siblings && schema);

    parent = (struct lyd_node_inner *)siblings->parent;
    if (parent && parent->schema && parent->children_ht) {
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
        /* find first sibling */
        if (siblings->parent) {
            siblings = siblings->parent->child;
        } else {
            while (siblings->prev->next) {
                siblings = siblings->prev;
            }
        }

        /* search manually without hashes */
        for ( ; siblings; siblings = siblings->next) {
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

    LY_CHECK_ARG_RET(NULL, schema, !(schema->nodetype & (LYS_CHOICE | LYS_CASE)), LY_EINVAL);

    if (!siblings || (siblings->schema && (lysc_data_parent(siblings->schema) != lysc_data_parent(schema)))) {
        /* no data or schema mismatch */
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (key_or_value && !val_len) {
        val_len = strlen(key_or_value);
    }

    if ((schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) && key_or_value) {
        /* create a data node and find the instance */
        if (schema->nodetype == LYS_LEAFLIST) {
            /* target used attributes: schema, hash, value */
            rc = lyd_create_term(schema, key_or_value, val_len, NULL, LY_PREF_JSON, NULL, LYD_HINT_DATA, NULL, &target);
            LY_CHECK_RET(rc);
        } else {
            /* target used attributes: schema, hash, child (all keys) */
            LY_CHECK_RET(lyd_create_list2(schema, key_or_value, val_len, &target));
        }

        /* find it */
        rc = lyd_find_sibling_first(siblings, target, match);
    } else {
        /* find the first schema node instance */
        rc = lyd_find_sibling_schema(siblings, schema, match);
    }

    lyd_free_tree(target);
    return rc;
}

API LY_ERR
lyd_find_xpath(const struct lyd_node *ctx_node, const char *xpath, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set;
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx_node, xpath, set, LY_EINVAL);

    memset(&xp_set, 0, sizeof xp_set);

    /* compile expression */
    ret = lyxp_expr_parse((struct ly_ctx *)LYD_CTX(ctx_node), xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* evaluate expression */
    ret = lyxp_eval(exp, NULL, LY_PREF_JSON, NULL, ctx_node, ctx_node, &xp_set, LYXP_IGNORE_WHEN);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    ret = ly_set_new(set);
    LY_CHECK_GOTO(ret, cleanup);

    /* transform into ly_set */
    if (xp_set.type == LYXP_SET_NODE_SET) {
        /* allocate memory for all the elements once (even though not all items must be elements but most likely will be) */
        (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
        LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(LYD_CTX(ctx_node)); ret = LY_EMEM, cleanup);
        (*set)->size = xp_set.used;

        for (i = 0; i < xp_set.used; ++i) {
            if (xp_set.val.nodes[i].type == LYXP_NODE_ELEM) {
                ret = ly_set_add(*set, xp_set.val.nodes[i].node, 1, NULL);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free((struct ly_ctx *)LYD_CTX(ctx_node), exp);
    if (ret) {
        ly_set_free(*set, NULL);
        *set = NULL;
    }
    return ret;
}

API uint32_t
lyd_list_pos(const struct lyd_node *instance)
{
    const struct lyd_node *iter = NULL;
    uint32_t pos = 0;

    if (!instance || !(instance->schema->nodetype & (LYS_LIST | LYS_LEAFLIST))) {
        return 0;
    }

    /* data instances are ordered, so we can stop when we found instance of other schema node */
    for (iter = instance; iter->schema == instance->schema; iter = iter->prev) {
        if (pos && (iter->next == NULL)) {
            /* overrun to the end of the siblings list */
            break;
        }
        ++pos;
    }

    return pos;
}

API struct lyd_node *
lyd_first_sibling(const struct lyd_node *node)
{
    struct lyd_node *start;

    if (!node) {
        return NULL;
    }

    /* get the first sibling */
    if (node->parent) {
        start = node->parent->child;
    } else {
        for (start = (struct lyd_node *)node; start->prev->next; start = start->prev) {}
    }

    return start;
}
