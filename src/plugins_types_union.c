/**
 * @file plugins_types_union.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in union type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins_types.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"

static LY_ERR
ly_type_union_store_type(const struct ly_ctx *ctx, struct lysc_type **types, struct lyd_value_subvalue *subvalue,
        ly_bool resolve, const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lys_glob_unres *unres,
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    uint32_t prev_lo;

    if (!types || !LY_ARRAY_COUNT(types)) {
        return LY_EINVAL;
    }

    *err = NULL;

    /* turn logging off */
    prev_lo = ly_log_options(0);

    /* use the first usable subtype to store the value */
    for (u = 0; u < LY_ARRAY_COUNT(types); ++u) {
        ret = types[u]->plugin->store(ctx, types[u], subvalue->original, strlen(subvalue->original), 0, subvalue->format,
                subvalue->prefix_data, subvalue->hints, subvalue->ctx_node, &subvalue->value, unres, err);
        if ((ret == LY_SUCCESS) || (ret == LY_EINCOMPLETE)) {
            if (resolve && (ret == LY_EINCOMPLETE)) {
                /* we need the value resolved */
                ret = subvalue->value.realtype->plugin->validate(ctx, types[u], ctx_node, tree, &subvalue->value, err);
                if (!ret) {
                    /* store and resolve successful */
                    break;
                }

                /* resolve failed, we need to free the stored value */
                types[u]->plugin->free(ctx, &subvalue->value);
            } else {
                /* store successful */
                break;
            }
        }
        ly_err_free(*err);
        *err = NULL;
    }

    if (u == LY_ARRAY_COUNT(types)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid union value \"%s\" - no matching subtype found.", subvalue->original);
    }

    /* restore logging */
    ly_log_options(prev_lo);
    return ret;
}

/**
 * @brief Store and canonize value of the YANG built-in union type.
 *
 * Implementation of the ly_type_store_clb.
 */
LY_ERR
ly_type_store_union(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)type;
    struct lyd_value_subvalue *subvalue = NULL;

    *err = NULL;

    /* prepare subvalue storage */
    subvalue = calloc(1, sizeof *subvalue);
    if (!subvalue) {
        ret = ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
        goto cleanup_value;
    }

    /* remember the original value */
    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &subvalue->original);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &subvalue->original);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* store format-specific data for later prefix resolution */
    ret = ly_type_prefix_data_new(ctx, subvalue->original, value_len, format, prefix_data, &subvalue->format,
            &subvalue->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);
    subvalue->hints = hints;
    subvalue->ctx_node = ctx_node;

    /* use the first usable subtype to store the value */
    ret = ly_type_union_store_type(ctx, type_u->types, subvalue, 0, NULL, NULL, unres, err);
    LY_CHECK_GOTO((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE), cleanup);

cleanup:
    if ((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE)) {
        lydict_remove(ctx, subvalue->original);
        ly_type_prefix_data_free(subvalue->format, subvalue->prefix_data);
        free(subvalue);
    } else {
        /* store it as union, the specific type is in the subvalue, but canonical value is the specific type value */
        ret = lydict_insert(ctx, subvalue->value.canonical, 0, &storage->canonical);
        LY_CHECK_GOTO(ret, cleanup);
        storage->subvalue = subvalue;
        storage->realtype = type;
    }

cleanup_value:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

/**
 * @brief Validate value of the YANG built-in union type.
 *
 * Implementation of the ly_type_validate_clb.
 */
LY_ERR
ly_type_validate_union(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)storage->realtype;
    struct lyd_value_subvalue *subvalue = storage->subvalue;

    *err = NULL;

    if (!subvalue->value.realtype->plugin->validate) {
        /* nothing to resolve */
        return LY_SUCCESS;
    }

    /* resolve the stored value */
    if (!subvalue->value.realtype->plugin->validate(ctx, type, ctx_node, tree, &subvalue->value, err)) {
        /* resolve successful */
        return LY_SUCCESS;
    }

    /* Resolve failed, we have to try another subtype of the union.
     * Unfortunately, since the realtype can change (e.g. in leafref), we are not able to detect
     * which of the subtype's were tried the last time, so we have to try all of them again.
     */
    ly_err_free(*err);
    *err = NULL;

    /* store and resolve the value */
    ret = ly_type_union_store_type(ctx, type_u->types, subvalue, 1, ctx_node, tree, NULL, err);
    LY_CHECK_RET(ret);

    /* success, update the canonical value */
    lydict_remove(ctx, storage->canonical);
    LY_CHECK_RET(lydict_insert(ctx, subvalue->value.canonical, 0, &storage->canonical));
    return LY_SUCCESS;
}

/**
 * @brief Comparison callback checking the union value.
 *
 * Implementation of the ly_type_compare_clb.
 */
LY_ERR
ly_type_compare_union(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1->subvalue->value.realtype != val2->subvalue->value.realtype) {
        return LY_ENOT;
    }
    return val1->subvalue->value.realtype->plugin->compare(&val1->subvalue->value, &val2->subvalue->value);
}

/**
 * @brief Printer callback printing the union value.
 *
 * Implementation of the ly_type_print_clb.
 */
const char *
ly_type_print_union(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic)
{
    return value->subvalue->value.realtype->plugin->print(&value->subvalue->value, format, prefix_data, dynamic);
}

/**
 * @brief Duplication callback of the union values.
 *
 * Implementation of the ly_type_dup_clb.
 */
LY_ERR
ly_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_CHECK_RET(lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical));

    dup->subvalue = calloc(1, sizeof *dup->subvalue);
    LY_CHECK_ERR_RET(!dup->subvalue, LOGMEM(ctx), LY_EMEM);
    LY_CHECK_RET(original->subvalue->value.realtype->plugin->duplicate(ctx, &original->subvalue->value, &dup->subvalue->value));

    LY_CHECK_RET(lydict_insert(ctx, original->subvalue->original, strlen(original->subvalue->original),
            &dup->subvalue->original));
    dup->subvalue->format = original->subvalue->format;
    LY_CHECK_RET(ly_type_prefix_data_dup(ctx, original->subvalue->format, original->subvalue->prefix_data,
            &dup->subvalue->prefix_data));

    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

/**
 * @brief Free value of the YANG built-in union type.
 *
 * Implementation of the ly_type_free_clb.
 */
void
ly_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lydict_remove(ctx, value->canonical);
    if (value->subvalue) {
        if (value->subvalue->value.realtype) {
            value->subvalue->value.realtype->plugin->free(ctx, &value->subvalue->value);
        }
        ly_type_prefix_data_free(value->subvalue->format, value->subvalue->prefix_data);
        lydict_remove(ctx, value->subvalue->original);
        free(value->subvalue);
        value->subvalue = NULL;
    }
}
