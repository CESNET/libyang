/**
 * @file integer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in integer types plugin.
 *
 * Copyright (c) 2019 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include "plugins_types.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */
#include "tree_schema_internal.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesInteger (u)int(8/16/32/64) (built-in)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | variable | yes | pointer to the specific integer type | little-endian integer value |
 */

static LY_ERR
lyplg_type_store_int(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;
    uint32_t value_size;
    int64_t num = 0;
    int base = 1;
    char *canon = NULL;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size(lys_datatype2str(type->basetype), format, value_size_bits,
            LYPLG_LYB_SIZE_VARIABLE_BITS, 0, &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (format == LY_VALUE_LYB) {
        /* copy the integer and correct the byte order */
        memcpy(&num, value, value_size);
        num = le64toh(num);
    } else {
        /* check hints */
        ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, &base, err);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse the integer */
        switch (type->basetype) {
        case LY_TYPE_INT8:
            ret = lyplg_type_parse_int("int8", base, INT64_C(-128), INT64_C(127), value, value_size, &num, err);
            break;
        case LY_TYPE_INT16:
            ret = lyplg_type_parse_int("int16", base, INT64_C(-32768), INT64_C(32767), value, value_size, &num, err);
            break;
        case LY_TYPE_INT32:
            ret = lyplg_type_parse_int("int32", base, INT64_C(-2147483648), INT64_C(2147483647), value, value_size, &num, err);
            break;
        case LY_TYPE_INT64:
            ret = lyplg_type_parse_int("int64", base, INT64_C(-9223372036854775807) - INT64_C(1),
                    INT64_C(9223372036854775807), value, value_size, &num, err);
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
        }
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* set the value (matters for big-endian) and get the correct int64 number */
    switch (type->basetype) {
    case LY_TYPE_INT8:
        storage->int8 = num;
        num = storage->int8;
        break;
    case LY_TYPE_INT16:
        storage->int16 = num;
        num = storage->int16;
        break;
    case LY_TYPE_INT32:
        storage->int32 = num;
        num = storage->int32;
        break;
    case LY_TYPE_INT64:
        storage->int64 = num;
        num = storage->int64;
        break;
    default:
        break;
    }

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value, value_size, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        /* generate canonical value */
        switch (type->basetype) {
        case LY_TYPE_INT8:
            LY_CHECK_ERR_GOTO(asprintf(&canon, "%" PRId8, storage->int8) == -1, ret = LY_EMEM, cleanup);
            break;
        case LY_TYPE_INT16:
            LY_CHECK_ERR_GOTO(asprintf(&canon, "%" PRId16, storage->int16) == -1, ret = LY_EMEM, cleanup);
            break;
        case LY_TYPE_INT32:
            LY_CHECK_ERR_GOTO(asprintf(&canon, "%" PRId32, storage->int32) == -1, ret = LY_EMEM, cleanup);
            break;
        case LY_TYPE_INT64:
            LY_CHECK_ERR_GOTO(asprintf(&canon, "%" PRId64, storage->int64) == -1, ret = LY_EMEM, cleanup);
            break;
        default:
            break;
        }

        /* store it */
        ret = lydict_insert_zc(ctx, canon, (const char **)&storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (!(options & LYPLG_TYPE_STORE_ONLY)) {
        /* validate value */
        ret = lyplg_type_validate_int(ctx, type, NULL, NULL, storage, err);
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

static LY_ERR
lyplg_type_compare_int(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    switch (val1->realtype->basetype) {
    case LY_TYPE_INT8:
        if (val1->int8 != val2->int8) {
            return LY_ENOT;
        }
        break;
    case LY_TYPE_INT16:
        if (val1->int16 != val2->int16) {
            return LY_ENOT;
        }
        break;
    case LY_TYPE_INT32:
        if (val1->int32 != val2->int32) {
            return LY_ENOT;
        }
        break;
    case LY_TYPE_INT64:
        if (val1->int64 != val2->int64) {
            return LY_ENOT;
        }
        break;
    default:
        break;
    }
    return LY_SUCCESS;
}

static int
lyplg_type_sort_int(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    switch (val1->realtype->basetype) {
    case LY_TYPE_INT8:
        if (val1->int8 < val2->int8) {
            return -1;
        } else if (val1->int8 > val2->int8) {
            return 1;
        } else {
            return 0;
        }
        break;
    case LY_TYPE_INT16:
        if (val1->int16 < val2->int16) {
            return -1;
        } else if (val1->int16 > val2->int16) {
            return 1;
        } else {
            return 0;
        }
        break;
    case LY_TYPE_INT32:
        if (val1->int32 < val2->int32) {
            return -1;
        } else if (val1->int32 > val2->int32) {
            return 1;
        } else {
            return 0;
        }
        break;
    case LY_TYPE_INT64:
        if (val1->int64 < val2->int64) {
            return -1;
        } else if (val1->int64 > val2->int64) {
            return 1;
        } else {
            return 0;
        }
        break;
    default:
        break;
    }
    return 0;
}

static const void *
lyplg_type_print_int(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint32_t *value_size_bits)
{
    int64_t prev_num = 0, num = 0;
    void *buf;

    if (format == LY_VALUE_LYB) {
        switch (value->realtype->basetype) {
        case LY_TYPE_INT8:
            prev_num = num = value->int8;
            break;
        case LY_TYPE_INT16:
            prev_num = num = value->int16;
            break;
        case LY_TYPE_INT32:
            prev_num = num = value->int32;
            break;
        case LY_TYPE_INT64:
            prev_num = num = value->int64;
            break;
        default:
            break;
        }

        num = htole64(num);
        if (num == prev_num) {
            /* values are equal, little-endian or int8 */
            *dynamic = 0;
            if (value_size_bits) {
                /* the least amount of bits that can hold the number */
                *value_size_bits = lyplg_type_get_highest_set_bit_pos(num);
            }
            return &value->int64;
        } else {
            /* values differ, big-endian */
            buf = calloc(1, LYPLG_BITS2BYTES(lyplg_type_get_highest_set_bit_pos(num)));
            LY_CHECK_RET(!buf, NULL);

            *dynamic = 1;
            if (value_size_bits) {
                *value_size_bits = lyplg_type_get_highest_set_bit_pos(num);
            }
            memcpy(buf, &num, LYPLG_BITS2BYTES(lyplg_type_get_highest_set_bit_pos(num)));
            return buf;
        }
    }

    /* use the cached canonical value */
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_size_bits) {
        *value_size_bits = strlen(value->_canonical) * 8;
    }
    return value->_canonical;
}

static LY_ERR
lyplg_type_store_uint(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;
    uint32_t value_size;
    uint64_t num = 0;
    int base = 0;
    char *canon;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size(lys_datatype2str(type->basetype), format, value_size_bits,
            LYPLG_LYB_SIZE_VARIABLE_BITS, 0, &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (format == LY_VALUE_LYB) {
        /* copy the integer and correct the byte order */
        memcpy(&num, value, value_size);
        num = le64toh(num);
    } else {
        /* check hints */
        ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, &base, err);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse the integer */
        switch (type->basetype) {
        case LY_TYPE_UINT8:
            ret = lyplg_type_parse_uint("uint8", base, UINT64_C(255), value, value_size, &num, err);
            break;
        case LY_TYPE_UINT16:
            ret = lyplg_type_parse_uint("uint16", base, UINT64_C(65535), value, value_size, &num, err);
            break;
        case LY_TYPE_UINT32:
            ret = lyplg_type_parse_uint("uint32", base, UINT64_C(4294967295), value, value_size, &num, err);
            break;
        case LY_TYPE_UINT64:
            ret = lyplg_type_parse_uint("uint64", base, UINT64_C(18446744073709551615), value, value_size, &num, err);
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
        }
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* store value, matters for big-endian */
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

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value, value_size, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        /* generate canonical value */
        LY_CHECK_ERR_GOTO(asprintf(&canon, "%" PRIu64, num) == -1, ret = LY_EMEM, cleanup);

        /* store it */
        ret = lydict_insert_zc(ctx, canon, (const char **)&storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (!(options & LYPLG_TYPE_STORE_ONLY)) {
        /* validate value */
        ret = lyplg_type_validate_uint(ctx, type, NULL, NULL, storage, err);
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

static LY_ERR
lyplg_type_compare_uint(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    switch (val1->realtype->basetype) {
    case LY_TYPE_UINT8:
        if (val1->uint8 != val2->uint8) {
            return LY_ENOT;
        }
        break;
    case LY_TYPE_UINT16:
        if (val1->uint16 != val2->uint16) {
            return LY_ENOT;
        }
        break;
    case LY_TYPE_UINT32:
        if (val1->uint32 != val2->uint32) {
            return LY_ENOT;
        }
        break;
    case LY_TYPE_UINT64:
        if (val1->uint64 != val2->uint64) {
            return LY_ENOT;
        }
        break;
    default:
        break;
    }
    return LY_SUCCESS;
}

static int
lyplg_type_sort_uint(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    switch (val1->realtype->basetype) {
    case LY_TYPE_UINT8:
        if (val1->uint8 < val2->uint8) {
            return -1;
        } else if (val1->uint8 > val2->uint8) {
            return 1;
        } else {
            return 0;
        }
        break;
    case LY_TYPE_UINT16:
        if (val1->uint16 < val2->uint16) {
            return -1;
        } else if (val1->uint16 > val2->uint16) {
            return 1;
        } else {
            return 0;
        }
        break;
    case LY_TYPE_UINT32:
        if (val1->uint32 < val2->uint32) {
            return -1;
        } else if (val1->uint32 > val2->uint32) {
            return 1;
        } else {
            return 0;
        }
        break;
    case LY_TYPE_UINT64:
        if (val1->uint64 < val2->uint64) {
            return -1;
        } else if (val1->uint64 > val2->uint64) {
            return 1;
        } else {
            return 0;
        }
        break;
    default:
        break;
    }
    return 0;
}

static const void *
lyplg_type_print_uint(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint32_t *value_size_bits)
{
    uint64_t num = 0;
    void *buf;

    if (format == LY_VALUE_LYB) {
        switch (value->realtype->basetype) {
        case LY_TYPE_UINT8:
            num = value->uint8;
            break;
        case LY_TYPE_UINT16:
            num = value->uint16;
            break;
        case LY_TYPE_UINT32:
            num = value->uint32;
            break;
        case LY_TYPE_UINT64:
            num = value->uint64;
            break;
        default:
            break;
        }
        num = htole64(num);
        if (num == value->uint64) {
            /* values are equal, little-endian or uint8 */
            *dynamic = 0;
            if (value_size_bits) {
                /* the least amount of bits that can hold the number */
                *value_size_bits = lyplg_type_get_highest_set_bit_pos(num);
            }
            return &value->uint64;
        } else {
            /* values differ, big-endian */
            buf = calloc(1, LYPLG_BITS2BYTES(lyplg_type_get_highest_set_bit_pos(num)));
            LY_CHECK_RET(!buf, NULL);

            *dynamic = 1;
            if (value_size_bits) {
                *value_size_bits = lyplg_type_get_highest_set_bit_pos(num);
            }
            memcpy(buf, &num, LYPLG_BITS2BYTES(lyplg_type_get_highest_set_bit_pos(num)));
            return buf;
        }
    }

    /* use the cached canonical value */
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_size_bits) {
        *value_size_bits = strlen(value->_canonical) * 8;
    }
    return value->_canonical;
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

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = lyplg_type_validate_uint,
        .plugin.compare = lyplg_type_compare_uint,
        .plugin.sort = lyplg_type_sort_uint,
        .plugin.print = lyplg_type_print_uint,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UINT16_STR,

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = lyplg_type_validate_uint,
        .plugin.compare = lyplg_type_compare_uint,
        .plugin.sort = lyplg_type_sort_uint,
        .plugin.print = lyplg_type_print_uint,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UINT32_STR,

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = lyplg_type_validate_uint,
        .plugin.compare = lyplg_type_compare_uint,
        .plugin.sort = lyplg_type_sort_uint,
        .plugin.print = lyplg_type_print_uint,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UINT64_STR,

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_uint,
        .plugin.validate = lyplg_type_validate_uint,
        .plugin.compare = lyplg_type_compare_uint,
        .plugin.sort = lyplg_type_sort_uint,
        .plugin.print = lyplg_type_print_uint,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT8_STR,

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = lyplg_type_validate_int,
        .plugin.compare = lyplg_type_compare_int,
        .plugin.sort = lyplg_type_sort_int,
        .plugin.print = lyplg_type_print_int,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT16_STR,

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = lyplg_type_validate_int,
        .plugin.compare = lyplg_type_compare_int,
        .plugin.sort = lyplg_type_sort_int,
        .plugin.print = lyplg_type_print_int,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT32_STR,

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = lyplg_type_validate_int,
        .plugin.compare = lyplg_type_compare_int,
        .plugin.sort = lyplg_type_sort_int,
        .plugin.print = lyplg_type_print_int,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    }, {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INT64_STR,

        .plugin.id = "ly2 integers",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_int,
        .plugin.validate = lyplg_type_validate_int,
        .plugin.compare = lyplg_type_compare_int,
        .plugin.sort = lyplg_type_sort_int,
        .plugin.print = lyplg_type_print_int,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {0}
};
