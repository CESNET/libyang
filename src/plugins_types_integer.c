/**
 * @file plugins_types_integer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in integer types plugin.
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

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"

API LY_ERR
ly_type_store_int(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int64_t num;
    int base = 1;
    char *str;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, &base, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    switch (type->basetype) {
    case LY_TYPE_INT8:
        ret = ly_type_parse_int("int8", base, INT64_C(-128), INT64_C(127), value, value_len, &num, err);
        break;
    case LY_TYPE_INT16:
        ret = ly_type_parse_int("int16", base, INT64_C(-32768), INT64_C(32767), value, value_len, &num, err);
        break;
    case LY_TYPE_INT32:
        ret = ly_type_parse_int("int32", base, INT64_C(-2147483648), INT64_C(2147483647), value, value_len, &num, err);
        break;
    case LY_TYPE_INT64:
        ret = ly_type_parse_int("int64", base, INT64_C(-9223372036854775807) - INT64_C(1),
                INT64_C(9223372036854775807), value, value_len, &num, err);
        break;
    default:
        LOGINT(ctx);
        ret = LY_EINT;
    }

    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    LY_CHECK_ERR_GOTO(asprintf(&str, "%" PRId64, num) == -1, ret = LY_EMEM, cleanup);

    /* range of the number */
    if (type_num->range) {
        ret = ly_type_validate_range(type->basetype, type_num->range, num, str, err);
        LY_CHECK_ERR_GOTO(ret != LY_SUCCESS, free(str), cleanup);
    }

    /* store everything */
    ret = lydict_insert_zc(ctx, str, &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->int64 = num;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

API LY_ERR
ly_type_store_uint(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint64_t num;
    int base = 0;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;
    char *str;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, &base, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    switch (type->basetype) {
    case LY_TYPE_UINT8:
        ret = ly_type_parse_uint("uint8", base, UINT64_C(255), value, value_len, &num, err);
        break;
    case LY_TYPE_UINT16:
        ret = ly_type_parse_uint("uint16", base, UINT64_C(65535), value, value_len, &num, err);
        break;
    case LY_TYPE_UINT32:
        ret = ly_type_parse_uint("uint32", base, UINT64_C(4294967295), value, value_len, &num, err);
        break;
    case LY_TYPE_UINT64:
        ret = ly_type_parse_uint("uint64", base, UINT64_C(18446744073709551615), value, value_len, &num, err);
        break;
    default:
        LOGINT(ctx);
        ret = LY_EINT;
    }

    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    LY_CHECK_ERR_GOTO(asprintf(&str, "%" PRIu64, num) == -1, ret = LY_EMEM, cleanup);

    /* range of the number */
    if (type_num->range) {
        ret = ly_type_validate_range(type->basetype, type_num->range, num, str, err);
        LY_CHECK_ERR_GOTO(ret != LY_SUCCESS, free(str), cleanup);
    }

    /* store everything */
    ret = lydict_insert_zc(ctx, str, &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->int64 = num;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}
