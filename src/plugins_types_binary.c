/**
 * @file plugins_types_binary.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in binary type plugin.
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

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"

/**
 * @brief Validate, canonize and store value of the YANG built-in binary type.
 *
 * Implementation of the ly_type_store_clb.
 */
LY_ERR
ly_type_store_binary(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    size_t base64_start, base64_end, base64_count, base64_terminated, value_end;
    struct lysc_type_bin *type_bin = (struct lysc_type_bin *)type;

    LY_CHECK_ARG_RET(ctx, value, LY_EINVAL);

    *err = NULL;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* validate characters and remember the number of octets for length validation */
    /* silently skip leading whitespaces */
    base64_start = 0;
    while (base64_start < value_len && isspace(value[base64_start])) {
        base64_start++;
    }
    /* silently skip trailing whitespace */
    value_end = value_len;
    while (base64_start < value_end && isspace(value[value_end - 1])) {
        value_end--;
    }

    /* find end of base64 value */
    base64_end = base64_start;
    base64_count = 0;
    while ((base64_end < value_len) &&
            /* check correct character in base64 */
            ((('A' <= value[base64_end]) && (value[base64_end] <= 'Z')) ||
            (('a' <= value[base64_end]) && (value[base64_end] <= 'z')) ||
            (('0' <= value[base64_end]) && (value[base64_end] <= '9')) ||
            ('+' == value[base64_end]) ||
            ('/' == value[base64_end]) ||
            ('\n' == value[base64_end]))) {

        if ('\n' != value[base64_end]) {
            base64_count++;
        }
        base64_end++;
    }

    /* find end of padding */
    base64_terminated = 0;
    while (((base64_end < value_len) && (base64_terminated < 2)) &&
            /* check padding on end of string */
            (('=' == value[base64_end]) ||
            ('\n' == value[base64_end]))) {

        if ('\n' != value[base64_end]) {
            base64_terminated++;
        }
        base64_end++;
    }

    /* check if value is valid base64 value */
    if (value_end != base64_end) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid Base64 character (%c).", value[base64_end]);
        goto cleanup;
    }

    if ((base64_count + base64_terminated) & 3) {
        /* base64 length must be multiple of 4 chars */
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Base64 encoded value length must be divisible by 4.");
        goto cleanup;
    }

    /* check if value meets the type requirments */
    if (type_bin->length) {
        const uint32_t value_length = ((base64_count + base64_terminated) / 4) * 3 - base64_terminated;
        ret = ly_type_validate_range(LY_TYPE_BINARY, type_bin->length, value_length, value, err);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }

    if (base64_count != 0) {
        ret = lydict_insert(ctx, &value[base64_start], base64_end - base64_start, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, "", 0, &storage->canonical);
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
