/**
 * @file xpath1.0.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types xpath1.0 type plugin.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins_types.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/* internal header */
#include "xpath.h"

/**
 * @brief Stored value structure for xpath1.0
 */
struct lyd_value_xpath10 {
    struct lyxp_expr *exp;
    const struct ly_ctx *ctx;
    LY_VALUE_FORMAT format;
    void *prefix_data;
};

/**
 * @brief Print xpath1.0 token in the specific format.
 *
 * @param[in] token Token to transform.
 * @param[in] tok_len Lenghth of @p token.
 * @param[in] is_nametest Whether the token is a nametest, it then always requires a prefix in XML @p get_format.
 * @param[in] resolve_ctx Context to use for resolving prefixes.
 * @param[in] resolve_format Format of the resolved prefixes.
 * @param[in] resolve_prefix_data Resolved prefixes prefix data.
 * @param[in] get_format Format of the output prefixes.
 * @param[in] get_prefix_data Format-specific prefix data for the output.
 * @param[in,out] prev_prefix Prefix of a previous nametest.
 * @param[out] token_p Printed token.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
xpath10_print_token(const char *token, uint16_t tok_len, ly_bool is_nametest, const struct ly_ctx *resolve_ctx,
        LY_VALUE_FORMAT resolve_format, const void *resolve_prefix_data, LY_VALUE_FORMAT get_format,
        void *get_prefix_data, const char **prev_prefix, char **token_p, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *str_begin, *str_next, *prefix;
    ly_bool is_prefix;
    uint32_t len;
    char *str = NULL;
    void *mem;
    uint32_t str_len = 0;
    const struct lys_module *mod;

    str_begin = token;

    while ((len = ly_value_prefix_next(str_begin, token + tok_len, &is_prefix, &str_next))) {
        if (is_prefix) {
            /* resolve the module in the original format */
            mod = lyplg_type_identity_module(resolve_ctx, NULL, str_begin, len, resolve_format, resolve_prefix_data);
            if (!mod) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to resolve prefix \"%.*s\".", len, str_begin);
                goto cleanup;
            }

            /* get the prefix in the target format */
            prefix = lyplg_type_get_prefix(mod, get_format, get_prefix_data);
            if (!prefix) {
                ret = ly_err_new(err, LY_EINT, LYVE_DATA, NULL, NULL, "Internal error.");
                goto cleanup;
            }

            /* append the prefix */
            mem = realloc(str, str_len + strlen(prefix) + 2);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%s:", prefix);
            str_len += strlen(prefix) + 1;

            if (is_nametest) {
                /* remember prefix of a nametest */
                *prev_prefix = prefix;
            }
        } else if (is_nametest && (get_format == LY_VALUE_XML) && (len == tok_len) && *prev_prefix) {
            /* nametest without a prefix, we must add it */
            mem = realloc(str, str_len + strlen(*prev_prefix) + 1 + len + 1);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%s:%.*s", *prev_prefix, len, str_begin);
            str_len += strlen(*prev_prefix) + 1 + len;
        } else {
            /* just append the string */
            mem = realloc(str, str_len + len + 1);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%.*s", len, str_begin);
            str_len += len;
        }

        str_begin = str_next;
    }

cleanup:
    if (ret) {
        free(str);
    } else {
        *token_p = str;
    }
    return ret;
}

/**
 * @brief Print xpath1.0 value in the specific format.
 *
 * @param[in] xp_val xpath1.0 value structure.
 * @param[in] format Format to print in.
 * @param[in] prefix_data Format-specific prefix data.
 * @param[out] str_value Printed value.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
xpath10_print_value(const struct lyd_value_xpath10 *xp_val, LY_VALUE_FORMAT format, void *prefix_data,
        char **str_value, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    char *str = NULL, *token;
    void *mem;
    const char *str_exp_ptr = xp_val->exp->expr, *cur_exp_ptr, *prev_prefix = NULL;
    uint32_t str_len = 0, len;
    uint16_t idx;
    ly_bool is_nt;

    for (idx = 0; idx < xp_val->exp->used; ++idx) {
        cur_exp_ptr = xp_val->exp->expr + xp_val->exp->tok_pos[idx];

        /* only these tokens may include prefixes */
        if ((xp_val->exp->tokens[idx] == LYXP_TOKEN_NAMETEST) || (xp_val->exp->tokens[idx] == LYXP_TOKEN_LITERAL)) {
            /* append preceding expression */
            len = cur_exp_ptr - str_exp_ptr;
            mem = realloc(str, str_len + len + 1);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%.*s", len, str_exp_ptr);
            str_len += len;
            str_exp_ptr = cur_exp_ptr;

            /* get the token in the target format */
            is_nt = (xp_val->exp->tokens[idx] == LYXP_TOKEN_NAMETEST) ? 1 : 0;
            ret = xpath10_print_token(cur_exp_ptr, xp_val->exp->tok_len[idx], is_nt, xp_val->ctx, xp_val->format,
                    xp_val->prefix_data, format, prefix_data, &prev_prefix, &token, err);
            LY_CHECK_GOTO(ret, cleanup);

            /* append the converted token */
            mem = realloc(str, str_len + strlen(token) + 1);
            LY_CHECK_ERR_GOTO(!mem, free(token); ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%s", token);
            str_len += strlen(token);
            str_exp_ptr += xp_val->exp->tok_len[idx];
            free(token);
        }
    }

    /* append the rest of the expression */
    if (str_exp_ptr[0]) {
        mem = realloc(str, str_len + strlen(str_exp_ptr) + 1);
        LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
        str = mem;
        sprintf(str + str_len, "%s", str_exp_ptr);
    }

cleanup:
    if (ret) {
        free(str);
    } else {
        *str_value = str;
    }
    return ret;
}

API LY_ERR
lyplg_type_store_xpath10(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_xpath10 *xp_val;
    char *canonical;

    /* store as a string */
    ret = lyplg_type_store_string(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node,
            storage, unres, err);
    LY_CHECK_RET(ret);

    xp_val = calloc(1, sizeof *xp_val);
    LY_CHECK_ERR_GOTO(!xp_val, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    storage->ptr = xp_val;

    /* store format-specific data and context for later prefix resolution */
    ret = lyplg_type_prefix_data_new(ctx, storage->canonical, strlen(storage->canonical), format, prefix_data,
            &xp_val->format, &xp_val->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);
    xp_val->ctx = ctx;

    /* parse */
    ret = lyxp_expr_parse(ctx, storage->canonical, strlen(storage->canonical), 1, &xp_val->exp);
    LY_CHECK_GOTO(ret, cleanup);

    if (format != LY_VALUE_JSON) {
        /* generate canonical (JSON) value */
        ret = xpath10_print_value(xp_val, LY_VALUE_JSON, NULL, &canonical, err);
        LY_CHECK_GOTO(ret, cleanup);

        /* replace the canonical value */
        lydict_remove(ctx, storage->canonical);
        storage->canonical = NULL;
        LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, canonical, &storage->canonical), cleanup);
    }

cleanup:
    if (ret) {
        lyplg_type_free_xpath10(ctx, storage);
    }
    return ret;
}

API const char *
lyplg_type_print_xpath10(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, size_t *value_len)
{
    char *str_value;
    struct ly_err_item *err = NULL;

    if ((format == LY_VALUE_CANON) || (format == LY_VALUE_JSON)) {
        /* canonical */
        if (dynamic) {
            *dynamic = 0;
        }
        if (value_len) {
            *value_len = strlen(value->canonical);
        }
        return value->canonical;
    }

    if (xpath10_print_value(value->ptr, format, prefix_data, &str_value, &err)) {
        if (err) {
            LOGVAL(NULL, err->vecode, err->msg);
            ly_err_free(err);
        }
        *dynamic = 0;
        return NULL;
    }

    *dynamic = 1;
    return str_value;
}

API LY_ERR
lyplg_type_dup_xpath10(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_xpath10 *xp_val;
    const struct lyd_value_xpath10 *xp_val_orig = original->ptr;

    memset(dup, 0, sizeof *dup);

    ret = lydict_insert(ctx, original->canonical, 0, &dup->canonical);
    LY_CHECK_GOTO(ret, cleanup);

    xp_val = calloc(1, sizeof *xp_val);
    LY_CHECK_ERR_GOTO(!xp_val, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    xp_val->ctx = ctx;
    dup->ptr = xp_val;

    ret = lyxp_expr_dup(ctx, xp_val_orig->exp, &xp_val->exp);
    LY_CHECK_GOTO(ret, cleanup);

    xp_val->format = xp_val_orig->format;
    ret = lyplg_type_prefix_data_dup(ctx, xp_val_orig->format, xp_val_orig->prefix_data, &xp_val->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (ret) {
        lyplg_type_free_xpath10(ctx, dup);
    }
    return ret;
}

API void
lyplg_type_free_xpath10(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_xpath10 *xp_val = value->ptr;

    lydict_remove(ctx, value->canonical);
    if (xp_val) {
        lyxp_expr_free(ctx, xp_val->exp);
        lyplg_type_prefix_data_free(xp_val->format, xp_val->prefix_data);

        free(xp_val);
        value->ptr = NULL;
    }
}

/**
 * @brief Plugin information for xpath1.0 type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_xpath10[] = {
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "xpath1.0",

        .plugin.id = "libyang 2 - xpath1.0, version 1",
        .plugin.store = lyplg_type_store_xpath10,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_xpath10,
        .plugin.duplicate = lyplg_type_dup_xpath10,
        .plugin.free = lyplg_type_free_xpath10
    },
    {0}
};
