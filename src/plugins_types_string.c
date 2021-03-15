/**
 * @file plugins_types_string.c
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

/**
 * @brief Validate and store value of the YANG built-in string type.
 *
 * Implementation of the ly_type_store_clb.
 */
LY_ERR
ly_type_store_string(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* length restriction of the string */
    if (type_str->length) {
        char buf[LY_NUMBER_MAXLEN];
        size_t char_count = ly_utf8len(value, value_len);

        /* value_len is in bytes, but we need number of chaarcters here */
        snprintf(buf, LY_NUMBER_MAXLEN, "%zu", char_count);
        ret = ly_type_validate_range(LY_TYPE_BINARY, type_str->length, char_count, buf, err);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }

    /* pattern restrictions */
    ret = ly_type_validate_patterns(type_str->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->canonical);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->ptr = NULL;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}
