/**
 * @file date.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types date and date-no-zone type plugin.
 *
 * Copyright (c) 2019 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* strdup */
#define _XOPEN_SOURCE /* strptime */

#include "plugins_types.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libyang.h"

#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesDate date and date-no-zone (ietf-yang-types)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 64       | yes | `time_t *` | UNIX timestamp |
 * | 1        | no | `uint8_t *` | flag whether the value is in the special Z/-00:00 unknown timezone or not |
 */

static void lyplg_type_free_date(const struct ly_ctx *ctx, struct lyd_value *value);

static void
lyplg_type_lyb_size_date_nz(const struct lysc_type *UNUSED(type), enum lyplg_lyb_size_type *size_type,
        uint64_t *fixed_size_bits)
{
    *size_type = LYPLG_LYB_SIZE_FIXED_BITS;
    *fixed_size_bits = 64;
}

/**
 * @brief Implementation of ::lyplg_type_store_clb for ietf-yang-types date and date-no-zone type.
 */
static LY_ERR
lyplg_type_store_date(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint64_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_date *val = NULL;
    struct lyd_value_date_nz *val_nz = NULL;
    struct tm tm = {0};
    char *ptr;
    uint32_t value_size;
    char *str = NULL;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    if (!strcmp(type->name, "date")) {
        LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    } else {
        LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val_nz);
    }
    LY_CHECK_ERR_GOTO(!val && !val_nz, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (val) {
            if ((value_size_bits != 64) && (value_size_bits != 65)) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB %s value size %" PRIu64
                        " b (expected 64 or 65 b).", type->name, value_size_bits);
                goto cleanup;
            }
        } else {
            if (value_size_bits != 64) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB %s value size %" PRIu64
                        " b (expected 64 b).", type->name, value_size_bits);
                goto cleanup;
            }
        }
        value_size = LYPLG_BITS2BYTES(value_size_bits);

        /* store timestamp */
        if (val) {
            memcpy(&val->time, value, sizeof val->time);
        } else {
            memcpy(&val_nz->time, value, sizeof val_nz->time);
        }

        /* store unknown timezone */
        if (val && (value_size > 8)) {
            val->unknown_tz = *(((uint8_t *)value) + 8) ? 1 : 0;
        }

        /* success */
        goto cleanup;
    }

    /* get value byte length */
    if (val) {
        ret = lyplg_type_check_value_size(type->name, format, value_size_bits, LYPLG_LYB_SIZE_VARIABLE_BITS, 0,
                &value_size, err);
    } else {
        ret = lyplg_type_check_value_size(type->name, format, value_size_bits, LYPLG_LYB_SIZE_FIXED_BITS, 64,
                &value_size, err);
    }
    LY_CHECK_GOTO(ret, cleanup);

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(options & LYPLG_TYPE_STORE_ONLY)) {
        /* validate value */
        ret = lyplg_type_validate_patterns(ctx, ((struct lysc_type_str *)type)->patterns, value, value_size, err);
        LY_CHECK_RET(ret);
    }

    if (val) {
        /* create date-and-time value */
        if (asprintf(&str, "%.*sT00:00:00%.*s", 10, (char *)value, (int)(value_size - 10), (char *)value + 10) == -1) {
            ret = LY_EMEM;
            goto cleanup;
        }

        /* convert to UNIX time */
        ret = ly_time_str2time(str, &val->time, NULL);
        if (ret) {
            ret = ly_err_new(err, ret, 0, NULL, NULL, "%s", ly_last_logmsg());
            goto cleanup;
        }
    } else {
        /* fill tm */
        ptr = strptime(value, "%Y-%m-%d", &tm);
        if (!ptr || ((uint32_t)(ptr - (char *)value) != value_size)) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to parse %s value \"%.*s\".", type->name,
                    (int)value_size, (char *)value);
            goto cleanup;
        }

        /* convert to UNIX time */
        val_nz->time = timegm(&tm);
    }

    if (val && (((char *)value)[value_size - 1] == 'Z')) {
        /* unknown timezone */
        val->unknown_tz = 1;
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
    }

cleanup:
    free(str);
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_date(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for ietf-yang-types date and date-no-zone type.
 */
static LY_ERR
lyplg_type_compare_date(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    struct lyd_value_date *v1, *v2;
    struct lyd_value_date_nz *vn1, *vn2;

    if (!strcmp(val1->realtype->name, "date")) {
        LYD_VALUE_GET(val1, v1);
        LYD_VALUE_GET(val2, v2);

        /* compare timestamp and unknown tz */
        if ((v1->time != v2->time) || (v1->unknown_tz != v2->unknown_tz)) {
            return LY_ENOT;
        }
    } else {
        LYD_VALUE_GET(val1, vn1);
        LYD_VALUE_GET(val2, vn2);

        /* compare timestamp */
        if (vn1->time != vn2->time) {
            return LY_ENOT;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Implementation of ::lyplg_type_sort_clb for ietf-yang-types date and date-no-zone type.
 */
static int
lyplg_type_sort_date(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_date *v1, *v2;
    struct lyd_value_date_nz *vn1, *vn2;

    if (!strcmp(val1->realtype->name, "date")) {
        LYD_VALUE_GET(val1, v1);
        LYD_VALUE_GET(val2, v2);

        /* compare timestamps */
        return difftime(v1->time, v2->time);
    } else {
        LYD_VALUE_GET(val1, vn1);
        LYD_VALUE_GET(val2, vn2);

        /* compare timestamps */
        return difftime(vn1->time, vn2->time);
    }
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for ietf-yang-types date and date-no-zone type.
 */
static const void *
lyplg_type_print_date(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint64_t *value_size_bits)
{
    struct lyd_value_date *val = NULL;
    struct lyd_value_date_nz *val_nz = NULL;
    struct tm tm;
    char *ret;

    if (!strcmp(value->realtype->name, "date")) {
        LYD_VALUE_GET(value, val);
    } else {
        LYD_VALUE_GET(value, val_nz);
    }

    if (format == LY_VALUE_LYB) {
        if (val && val->unknown_tz) {
            ret = malloc(8 + 1);
            LY_CHECK_ERR_RET(!ret, LOGMEM(ctx), NULL);

            *dynamic = 1;
            if (value_size_bits) {
                *value_size_bits = 64 + 8;
            }
            memcpy(ret, &val->time, sizeof val->time);
            memcpy(ret + 8, &val->unknown_tz, sizeof val->unknown_tz);
        } else {
            *dynamic = 0;
            if (value_size_bits) {
                *value_size_bits = 64;
            }
            ret = val ? (char *)&val->time : (char *)&val_nz->time;
        }
        return ret;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        if (val_nz || (val && val->unknown_tz)) {
            /* ly_time_time2str but always using GMT */
            if (!gmtime_r(val_nz ? &val_nz->time : &val->time, &tm)) {
                return NULL;
            }

            if (asprintf(&ret, "%04d-%02d-%02d%s", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, val ? "Z" : "") == -1) {
                return NULL;
            }
        } else {
            if (ly_time_time2str(val->time, NULL, &ret)) {
                return NULL;
            }

            /* truncate the time segment */
            assert(ret[10] == 'T');
            memmove(ret + 10, ret + 19, strlen(ret + 19) + 1);
        }

        /* store it */
        if (lydict_insert(ctx, ret, 0, (const char **)&value->_canonical)) {
            free(ret);
            LOGMEM(ctx);
            return NULL;
        }
        free(ret);
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
 * @brief Implementation of ::lyplg_type_dup_clb for ietf-yang-types date and date-no-zone type.
 */
static LY_ERR
lyplg_type_dup_date(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_date *orig_val, *dup_val;
    struct lyd_value_date *orig_val_nz, *dup_val_nz;

    memset(dup, 0, sizeof *dup);

    /* optional canonical value */
    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    /* allocate value */
    if (!strcmp(original->realtype->name, "date")) {
        LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
        LY_CHECK_ERR_GOTO(!dup_val, ret = LY_EMEM, error);

        LYD_VALUE_GET(original, orig_val);

        /* copy timestamp and unknown tz */
        dup_val->time = orig_val->time;
        dup_val->unknown_tz = orig_val->unknown_tz;
    } else {
        LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val_nz);
        LY_CHECK_ERR_GOTO(!dup_val_nz, ret = LY_EMEM, error);

        LYD_VALUE_GET(original, orig_val_nz);

        /* copy timestamp */
        dup_val_nz->time = orig_val_nz->time;
    }

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_date(ctx, dup);
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_free_clb for ietf-yang-types date and date-no-zone type.
 */
static void
lyplg_type_free_date(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_date *val, *val_nz;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;

    if (!strcmp(value->realtype->name, "date")) {
        LYD_VALUE_GET(value, val);
        if (val) {
            LYPLG_TYPE_VAL_INLINE_DESTROY(val);
        }
    } else {
        LYD_VALUE_GET(value, val_nz);
        if (val_nz) {
            LYPLG_TYPE_VAL_INLINE_DESTROY(val_nz);
        }
    }
}

/**
 * @brief Plugin information for date and date-no-zone type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_date[] = {
    {
        .module = "ietf-yang-types",
        .revision = NULL,
        .name = "date",

        .plugin.id = "ly2 date",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_date,
        .plugin.validate_value = lyplg_type_validate_value_string,
        .plugin.validate_tree = NULL,
        .plugin.compare = lyplg_type_compare_date,
        .plugin.sort = lyplg_type_sort_date,
        .plugin.print = lyplg_type_print_date,
        .plugin.duplicate = lyplg_type_dup_date,
        .plugin.free = lyplg_type_free_date,
    },
    {
        .module = "ietf-yang-types",
        .revision = NULL,
        .name = "date-no-zone",

        .plugin.id = "ly2 date",
        .plugin.lyb_size = lyplg_type_lyb_size_date_nz,
        .plugin.store = lyplg_type_store_date,
        .plugin.validate_value = lyplg_type_validate_value_string,
        .plugin.validate_tree = NULL,
        .plugin.compare = lyplg_type_compare_date,
        .plugin.sort = lyplg_type_sort_date,
        .plugin.print = lyplg_type_print_date,
        .plugin.duplicate = lyplg_type_dup_date,
        .plugin.free = lyplg_type_free_date,
    },
    {0}
};
