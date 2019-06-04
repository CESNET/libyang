/**
 * @file tree_data_helpers.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsing and validation helper functions for data trees
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
#include <stdlib.h>

#include "log.h"
#include "dict.h"
#include "plugins_types.h"
#include "tree_data.h"
#include "tree_schema.h"

struct lyd_node **
lyd_node_children_p(struct lyd_node *node)
{
    assert(node);
    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
        return &((struct lyd_node_inner*)node)->child;
    default:
        return NULL;
    }
}

API const struct lyd_node *
lyd_node_children(const struct lyd_node *node)
{
    struct lyd_node **children;

    if (!node) {
        return NULL;
    }

    children = lyd_node_children_p((struct lyd_node*)node);
    if (children) {
        return *children;
    } else {
        return NULL;
    }
}

LY_ERR
lyd_parse_check_options(struct ly_ctx *ctx, int options, const char *func)
{
    int x = options & LYD_OPT_TYPEMASK;

    /* LYD_OPT_WHENAUTODEL can be used only with LYD_OPT_DATA or LYD_OPT_CONFIG */
    if (options & LYD_OPT_WHENAUTODEL) {
        if ((x == LYD_OPT_EDIT) || (x == LYD_OPT_NOTIF_FILTER)) {
            LOGERR(ctx, LY_EINVAL, "%s: Invalid options 0x%x (LYD_OPT_DATA_WHENAUTODEL can be used only with LYD_OPT_DATA or LYD_OPT_CONFIG)",
                   func, options);
            return LY_EINVAL;
        }
    }

    if (options & (LYD_OPT_DATA_ADD_YANGLIB | LYD_OPT_DATA_NO_YANGLIB)) {
        if (x != LYD_OPT_DATA) {
            LOGERR(ctx, LY_EINVAL, "%s: Invalid options 0x%x (LYD_OPT_DATA_*_YANGLIB can be used only with LYD_OPT_DATA)",
                   func, options);
            return LY_EINVAL;
        }
    }

    /* "is power of 2" algorithm, with 0 exception */
    if (x && !(x && !(x & (x - 1)))) {
        LOGERR(ctx, LY_EINVAL, "%s: Invalid options 0x%x (multiple data type flags set).", func, options);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_value_parse(struct lyd_node_term *node, const char *value, size_t value_len, int dynamic,
                ly_type_resolve_prefix get_prefix, void *parser)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    void *priv = NULL;
    int options = LY_TYPE_OPTS_VALIDATE | LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE | (dynamic ? LY_TYPE_OPTS_DYNAMIC : 0);
    assert(node);

    ctx = node->schema->module->ctx;
    type = ((struct lysc_node_leaf*)node->schema)->type;
    if (type->plugin->validate) {
        ret = type->plugin->validate(ctx, type, value, value_len, options, get_prefix, parser, &node->value.canonized, &err, &priv);
        if (ret) {
            ly_err_print(err);
            LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
            ly_err_free(err);
            goto error;
        }
    } else if (dynamic) {
        node->value.canonized = lydict_insert_zc(ctx, (char*)value);
    } else {
        node->value.canonized = lydict_insert(ctx, value, value_len);
    }

    if (type->plugin->store) {
        ret = type->plugin->store(ctx, type, options, &node->value, &err, &priv);
        if (ret) {
            ly_err_print(err);
            LOGVAL(ctx, LY_VLOG_STR, err->path, err->vecode, err->msg);
            ly_err_free(err);
            goto error;
        }
    }

error:
    return ret;
}

