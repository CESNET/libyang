/**
 * @file integer.c
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

#define _GNU_SOURCE /* asprintf, strdup */
#include <sys/cdefs.h>

#include "plugins_types.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @brief LYB value size of each integer type.
 */
static size_t integer_lyb_size[] = {
    [LY_TYPE_INT8] = 1, [LY_TYPE_INT16] = 2, [LY_TYPE_INT32] = 4, [LY_TYPE_INT64] = 8,
    [LY_TYPE_UINT8] = 1, [LY_TYPE_UINT16] = 2, [LY_TYPE_UINT32] = 4, [LY_TYPE_UINT64] = 8
};

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesInteger (u)int(8/16/32/64) (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 1/2/4/8 | yes | pointer to the specific integer type | integer value |
 */

API LY_ERR
lyplg_type_store_int(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int64_t num;
    int base = 1;
    char *canon;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;

    /* clear storage */
    memset(storage, 0, sizeof *storage);

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != integer_lyb_size[type->basetype]) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB signed integer value size %zu (expected %zu).",
                    value_len, integer_lyb_size[type->basetype]);
            goto cleanup;
        }

        /* get the integer */
        switch (type->basetype) {
        case LY_TYPE_INT8:
            num = *(int8_t *)value;
            break;
        case LY_TYPE_INT16:
            num = *(int16_t *)value;
            break;
        case LY_TYPE_INT32:
            num = *(int32_t *)value;
            break;
        case LY_TYPE_INT64:
            num = *(int64_t *)value;
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
            goto cleanup;
        }
    } else {
        /* check hints */
        ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, &base, err);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse the integer */
        switch (type->basetype) {
        case LY_TYPE_INT8:
            ret = lyplg_type_parse_int("int8", base, INT64_C(-128), INT64_C(127), value, value_len, &num, err);
            break;
        case LY_TYPE_INT16:
            ret = lyplg_type_parse_int("int16", base, INT64_C(-32768), INT64_C(32767), value, value_len, &num, err);
            break;
        case LY_TYPE_INT32:
            ret = lyplg_type_parse_int("int32", base, INT64_C(-2147483648), INT64_C(2147483647), value, value_len, &num, err);
            break;
        case LY_TYPE_INT64:
            ret = lyplg_type_parse_int("int64", base, INT64_C(-9223372036854775807) - INT64_C(1),
                    INT64_C(9223372036854775807), value, value_len, &num, err);
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
        }
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* init storage */
    storage->_canonical = NULL;
    /* matters for big-endian */
    switch (type->basetype) {
    case LY_TYPE_INT8:
        storage->int8 = num;
        break;
    case LY_TYPE_INT16:
        storage->int16 = num;
        break;
    case LY_TYPE_INT32:
        storage->int32 = num;
        break;
    case LY_TYPE_INT64:
        storage->int64 = num;
        break;
    default:
        break;
    }
    storage->realtype = type;

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value, value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        /* generate canonical value */
        LY_CHECK_ERR_GOTO(asprintf(&canon, "%" PRId64, num) == -1, ret = LY_EMEM, cleanup);

        /* store it */
        ret = lydict_insert_zc(ctx, canon, (const char **)&storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* validate range of the number */
    if (type_num->range) {
        ret = lyplg_type_validate_range(type->basetype, type_num->range, num, storage->_canonical,
                strlen(storage->_canonical), err);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

API LY_ERR
lyplg_type_store_uint(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint64_t num;
    int base = 0;
    char *canon;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;

    /* clear storage */
    memset(storage, 0, sizeof *storage);

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != integer_lyb_size[type->basetype]) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB unsigned integer value size %zu (expected %zu).",
                    value_len, integer_lyb_size[type->basetype]);
            goto cleanup;
        }

        /* get the integer */
        switch (type->basetype) {
        case LY_TYPE_UINT8:
            num = *(uint8_t *)value;
            break;
        case LY_TYPE_UINT16:
            num = *(uint16_t *)value;
            break;
        case LY_TYPE_UINT32:
            num = *(uint32_t *)value;
            break;
        case LY_TYPE_UINT64:
            num = *(uint64_t *)value;
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
            goto cleanup;
        }
    } else {
        /* check hints */
        ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, &base, err);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse the integer */
        switch (type->basetype) {
        case LY_TYPE_UINT8:
            ret = lyplg_type_parse_uint("uint8", base, UINT64_C(255), value, value_len, &num, err);
            break;
        case LY_TYPE_UINT16:
            ret = lyplg_type_parse_uint("uint16", base, UINT64_C(65535), value, value_len, &num, err);
            break;
        case LY_TYPE_UINT32:
            ret = lyplg_type_parse_uint("uint32", base, UINT64_C(4294967295), value, value_len, &num, err);
            break;
        case LY_TYPE_UINT64:
            ret = lyplg_type_parse_uint("uint64", base, UINT64_C(18446744073709551615), value, value_len, &num, err);
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
        }
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* init storage */
    storage->_canonical = NULL;
    /* matters for big-endian */
    switch (type->basetype) {
    case LY_TYPE_UINT8:
        storage->uint8 = num;
        break;
    case LY_TYPE_UINT16:
        storage->uint16 = num;
        break;
    case LY_TYPE_UINT32:
        storage->uint32 = num;
        break;
    case LY_TYPE_UINT64:
        storage->uint64 = num;
        break;
    default:
        break;
    }
    storage->realtype = type;

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value, value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        /* generate canonical value */
        LY_CHECK_ERR_GOTO(asprintf(&canon, "%" PRIu64, num) == -1, ret = LY_EMEM, cleanup);

        /* store it */
        ret = lydict_insert_zc(ctx, canon, (const char **)&storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* validate range of the number */
    if (type_num->range) {
        ret = lyplg_type_validate_range(type->basetype, type_num->range, num, storage->_canonical,
                strlen(storage->_canonical), err);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

/**
 * @brief Plugin information for integer types implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_integer[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UINT8_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UINT16_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UINT32_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UINT64_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT8_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT16_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT32_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT64_STR,

        .plugin.id = "libyang 2 - integers, version 1",
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
