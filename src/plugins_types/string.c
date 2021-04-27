/**
 * @file string.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in string type plugin.
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

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

API LY_ERR
lyplg_type_store_string(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* length restriction of the string */
    if (type_str->length) {
        char buf[LY_NUMBER_MAXLEN];
        size_t char_count = ly_utf8len(value, value_len);

        /* value_len is in bytes, but we need number of characters here */
        snprintf(buf, LY_NUMBER_MAXLEN, "%zu", char_count);
        ret = lyplg_type_validate_range(LY_TYPE_STRING, type_str->length, char_count, buf, strlen(buf), err);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }

    /* pattern restrictions */
    ret = lyplg_type_validate_patterns(type_str->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->_canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->ptr = NULL;
    storage->realtype = type;

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

/**
 * @brief Plugin information for string type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_string[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_STRING_STR,

        .plugin.id = "libyang 2 - string, version 1",
        .plugin.store = lyplg_type_store_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
