/**
 * @file decimal64.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in decimal64 type plugin.
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

API LY_ERR
lyplg_type_store_decimal64(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int64_t d;
    struct lysc_type_dec *type_dec = (struct lysc_type_dec *)type;
    char buf[LY_NUMBER_MAXLEN];

    *err = NULL;

    if (!value || !value[0] || !value_len) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL,  NULL, "Invalid empty decimal64 value.");
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    ret = lyplg_type_parse_dec64(type_dec->fraction_digits, value, value_len, &d, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    /* prepare canonized value */
    if (d) {
        int count = sprintf(buf, "%" PRId64 " ", d);
        if (((d > 0) && ((count - 1) <= type_dec->fraction_digits)) ||
                ((count - 2) <= type_dec->fraction_digits)) {
            /* we have 0. value, print the value with the leading zeros
             * (one for 0. and also keep the correct with of num according
             * to fraction-digits value)
             * for (num<0) - extra character for '-' sign */
            count = sprintf(buf, "%0*" PRId64 " ", (d > 0) ? (type_dec->fraction_digits + 1) : (type_dec->fraction_digits + 2), d);
        }
        for (uint8_t i = type_dec->fraction_digits, j = 1; i > 0; i--) {
            if (j && (i > 1) && (buf[count - 2] == '0')) {
                /* we have trailing zero to skip */
                buf[count - 1] = '\0';
            } else {
                j = 0;
                buf[count - 1] = buf[count - 2];
            }
            count--;
        }
        buf[count - 1] = '.';
    } else {
        /* zero */
        sprintf(buf, "0.0");
    }

    /* range of the number */
    if (type_dec->range) {
        ret = lyplg_type_validate_range(type->basetype, type_dec->range, d, buf, err);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }

    ret = lydict_insert(ctx, buf, strlen(buf), &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->dec64 = d;
    storage->realtype = type;

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

/**
 * @brief Plugin information for decimal64 type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_decimal64[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_DEC64_STR,

        .plugin.id = "libyang 2 - decimal64, version 1",
        .plugin.store = lyplg_type_store_decimal64,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
