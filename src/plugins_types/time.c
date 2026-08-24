/**
 * @file time.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types time and time-no-zone type plugin.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
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
 * @subsection howtoDataLYBTypesTime time and time-no-zone (ietf-yang-types)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 32       | yes | `uint32_t *` | timestamp converted into seconds |
 * | 8        | yes (only for time) | `uint8_t *` | flag whether the value is in the special Z/-00:00 unknown timezone or not |
 * | string length | no | `char *` | string with the fraction digits of a second |
 */

static void lyplg_type_free_time(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Implementation of ::lyplg_type_store_clb for ietf-yang-types time and time-no-zone type.
 */
static LY_ERR
lyplg_type_store_time(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint64_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_time *val = NULL;
    struct lyd_value_time_nz *val_nz = NULL;
    struct tm tm = {0};
    uint32_t i, value_size;
    char c, *ptr, *str = NULL;
    time_t t;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    if (!strcmp(type->name, "time")) {
        LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    } else {
        LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val_nz);
    }
    LY_CHECK_ERR_GOTO(!val && !val_nz, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (val && (value_size_bits < 40)) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB %s value size %" PRIu64
                    " b (expected at least 40 b).", type->name, value_size_bits);
            goto cleanup;
        } else if (val_nz && (value_size_bits < 32)) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB %s value size %" PRIu64
                    " b (expected at least 32 b).", type->name, value_size_bits);
            goto cleanup;
        }
        value_size = LYPLG_BITS2BYTES(value_size_bits);
        for (i = val ? 5 : 4; i < value_size; ++i) {
            c = ((char *)value)[i];
            if (!isdigit(c)) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB %s character '%c' "
                        "(expected a digit).", type->name,  c);
                goto cleanup;
            }
        }

        /* store timestamp */
        if (val) {
            memcpy(&val->seconds, value, sizeof val->seconds);
        } else {
            memcpy(&val_nz->seconds, value, sizeof val_nz->seconds);
        }

        /* store unknown timezone */
        if (val) {
            val->unknown_tz = *(((uint8_t *)value) + 4) ? 1 : 0;
        }

        /* store fractions of a second */
        if (val && (value_size > 5)) {
            val->fractions_s = strndup(((char *)value) + 5, value_size - 5);
            LY_CHECK_ERR_GOTO(!val->fractions_s, ret = LY_EMEM, cleanup);
        } else if (val_nz && (value_size > 4)) {
            val_nz->fractions_s = strndup(((char *)value) + 4, value_size - 4);
            LY_CHECK_ERR_GOTO(!val_nz->fractions_s, ret = LY_EMEM, cleanup);
        }

        /* success */
        goto cleanup;
    }

    /* get value byte length */
    if (val) {
        ret = lyplg_type_check_value_size(type->name, format, value_size_bits, LYPLG_LYB_SIZE_VARIABLE_BITS, 0,
                &value_size, err);
    } else {
        ret = lyplg_type_check_value_size(type->name, format, value_size_bits, LYPLG_LYB_SIZE_FIXED_BITS, 32,
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
        if (asprintf(&str, "1970-01-01T%.*s", (int)value_size, (char *)value) == -1) {
            ret = LY_EMEM;
            goto cleanup;
        }

        /* convert to UNIX time and fractions of a second */
        ret = ly_time_str2time(str, (time_t *)&t, &val->fractions_s);
        if (ret) {
            ret = ly_err_new(err, ret, 0, NULL, NULL, "%s", ly_last_logmsg());
            goto cleanup;
        }
        val->seconds = t;
    } else {
        /* fill tm */
        ptr = strptime(value, "%H:%M:%S", &tm);
        if (!ptr || (ptr - (char *)value != 8)) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to parse %s value \"%.*s\".", type->name,
                    (int)value_size, (char *)value);
            goto cleanup;
        }

        /* set the Epoch */
        tm.tm_mday = 1;
        tm.tm_mon = 0;
        tm.tm_year = 70;
        tm.tm_isdst = 0;

        /* convert to UNIX time */
        val_nz->seconds = timegm(&tm);

        /* store fractions of a second */
        if (ptr[0] == '.') {
            ++ptr;
            val_nz->fractions_s = strndup(ptr, value_size - (ptr - (char *)value));
            if (!val_nz->fractions_s) {
                ret = LY_EMEM;
                goto cleanup;
            }
        }
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
        lyplg_type_free_time(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for ietf-yang-types time and time-no-zone type.
 */
static LY_ERR
lyplg_type_compare_time(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    struct lyd_value_time *v1, *v2;
    struct lyd_value_time_nz *vn1, *vn2;
    const char *fr1, *fr2;

    if (!strcmp(val1->realtype->name, "time")) {
        LYD_VALUE_GET(val1, v1);
        LYD_VALUE_GET(val2, v2);

        /* compare seconds and unknown tz */
        if ((v1->seconds != v2->seconds) || (v1->unknown_tz != v2->unknown_tz)) {
            return LY_ENOT;
        }

        fr1 = v1->fractions_s;
        fr2 = v2->fractions_s;
    } else {
        LYD_VALUE_GET(val1, vn1);
        LYD_VALUE_GET(val2, vn2);

        /* compare seconds */
        if (vn1->seconds != vn2->seconds) {
            return LY_ENOT;
        }

        fr1 = vn1->fractions_s;
        fr2 = vn2->fractions_s;
    }

    /* compare second fractions */
    if ((!fr1 && !fr2) || (fr1 && fr2 && !strcmp(fr1, fr2))) {
        return LY_SUCCESS;
    }
    return LY_ENOT;
}

/**
 * @brief Decide if @p frac can be represented as zero.
 *
 * @param[in] frac Fractions of a second.
 * @return 1 if @p frac can be represented as zero.
 * @return 0 @p frac is not zero.
 */
static ly_bool
lyplg_type_fractions_is_zero(char *frac)
{
    char *iter;

    if (!frac) {
        return 1;
    }

    for (iter = frac; *iter; iter++) {
        if (*iter != '0') {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Compare @p f1 and @p f2 for sorting.
 *
 * @param[in] f1 First fractions of a second.
 * @param[in] f2 Second fractions of a second.
 * @return 1 if @p f1 > @p f2.
 * @return 0 if @p f1 == @p f2.
 * @return -1 if @p f1 < @p f2.
 */
static int
lyplg_type_sort_by_fractions(char *f1, char *f2)
{
    ly_bool f1_is_zero, f2_is_zero;
    int df;

    f1_is_zero = lyplg_type_fractions_is_zero(f1);
    f2_is_zero = lyplg_type_fractions_is_zero(f2);

    if (f1_is_zero && !f2_is_zero) {
        return -1;
    } else if (!f1_is_zero && f2_is_zero) {
        return 1;
    } else if (f1_is_zero && f2_is_zero) {
        return 0;
    }

    /* both f1 and f2 have some non-zero number */
    assert(!f1_is_zero && !f2_is_zero && f1 && f2);
    df = strcmp(f1, f2);
    if (df > 0) {
        return 1;
    } else if (df < 0) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * @brief Implementation of ::lyplg_type_sort_clb for ietf-yang-types time and time-no-zone type.
 */
static int
lyplg_type_sort_time(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_time *v1 = NULL, *v2;
    struct lyd_value_time_nz *vn1 = NULL, *vn2;
    double dt;

    if (!strcmp(val1->realtype->name, "time")) {
        LYD_VALUE_GET(val1, v1);
        LYD_VALUE_GET(val2, v2);

        /* compare seconds */
        dt = difftime(v1->seconds, v2->seconds);
    } else {
        LYD_VALUE_GET(val1, vn1);
        LYD_VALUE_GET(val2, vn2);

        /* compare seconds */
        dt = difftime(vn1->seconds, vn2->seconds);
    }
    if (dt != 0) {
        return dt;
    }

    /* compare second fractions */
    if (v1) {
        return lyplg_type_sort_by_fractions(v1->fractions_s, v2->fractions_s);
    } else {
        return lyplg_type_sort_by_fractions(vn1->fractions_s, vn2->fractions_s);
    }
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for ietf-yang-types time and time-no-zone type.
 */
static const void *
lyplg_type_print_time(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint64_t *value_size_bits)
{
    struct lyd_value_time *val = NULL;
    struct lyd_value_time_nz *val_nz = NULL;
    struct tm tm;
    uint32_t seconds;
    time_t t;
    ly_bool unknown_tz;
    char *ret, *fractions_s;

    if (!strcmp(value->realtype->name, "time")) {
        LYD_VALUE_GET(value, val);
    } else {
        LYD_VALUE_GET(value, val_nz);
    }

    if (format == LY_VALUE_LYB) {
        if (val_nz && !val_nz->fractions_s) {
            /* static value */
            *dynamic = 0;
            if (value_size_bits) {
                *value_size_bits = 32;
            }
            return (char *)&val_nz->seconds;
        }

        /* dynamic value */
        if (val) {
            seconds = val->seconds;
            unknown_tz = val->unknown_tz;
            fractions_s = val->fractions_s;
        } else {
            seconds = val_nz->seconds;
            fractions_s = val_nz->fractions_s;
        }

        ret = malloc(4 + (val ? 1 : 0) + (fractions_s ? strlen(fractions_s) : 0));
        LY_CHECK_ERR_RET(!ret, LOGMEM(ctx), NULL);

        *dynamic = 1;
        if (value_size_bits) {
            *value_size_bits = 32 + (val ? 8 : 0) + (fractions_s ? strlen(fractions_s) * 8 : 0);
        }
        memcpy(ret, &seconds, sizeof seconds);
        if (val) {
            memcpy(ret + 4, &unknown_tz, sizeof unknown_tz);
        }
        if (fractions_s) {
            memcpy(ret + (val ? 5 : 4), fractions_s, strlen(fractions_s));
        }
        return ret;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        if (val_nz || (val && val->unknown_tz)) {
            if (val) {
                t = val->seconds;
                fractions_s = val->fractions_s;
            } else {
                t = val_nz->seconds;
                fractions_s = val_nz->fractions_s;
            }

            /* ly_time_time2str but always using GMT */
            if (!gmtime_r(&t, &tm)) {
                return NULL;
            }

            if (asprintf(&ret, "%02d:%02d:%02d%s%s%s", tm.tm_hour, tm.tm_min, tm.tm_sec,
                    fractions_s ? "." : "", fractions_s ? fractions_s : "", val ? "Z" : "") == -1) {
                return NULL;
            }

            /* store it */
            if (lydict_insert_zc(ctx, ret, (const char **)&value->_canonical)) {
                LOGMEM(ctx);
                return NULL;
            }
        } else {
            if (ly_time_time2str(val->seconds, val->fractions_s, &ret)) {
                return NULL;
            }

            /* truncate the date segment */
            assert(ret[10] == 'T');
            memmove(ret, ret + 11, strlen(ret + 11) + 1);

            /* store it */
            if (lydict_insert(ctx, ret, 0, (const char **)&value->_canonical)) {
                free(ret);
                LOGMEM(ctx);
                return NULL;
            }
            free(ret);
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
 * @brief Implementation of ::lyplg_type_dup_clb for ietf-yang-types time and time-no-zone type.
 */
static LY_ERR
lyplg_type_dup_time(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_time *orig_val, *dup_val;
    struct lyd_value_time *orig_val_nz, *dup_val_nz;

    memset(dup, 0, sizeof *dup);

    /* optional canonical value */
    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    /* allocate value */
    if (!strcmp(original->realtype->name, "time")) {
        LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
        LY_CHECK_ERR_GOTO(!dup_val, ret = LY_EMEM, error);

        LYD_VALUE_GET(original, orig_val);

        /* copy seconds and unknown tz */
        dup_val->seconds = orig_val->seconds;
        dup_val->unknown_tz = orig_val->unknown_tz;

        /* duplicate second fractions */
        if (orig_val->fractions_s) {
            dup_val->fractions_s = strdup(orig_val->fractions_s);
            LY_CHECK_ERR_GOTO(!dup_val->fractions_s, ret = LY_EMEM, error);
        }
    } else {
        LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val_nz);
        LY_CHECK_ERR_GOTO(!dup_val_nz, ret = LY_EMEM, error);

        LYD_VALUE_GET(original, orig_val_nz);

        /* copy seconds */
        dup_val_nz->seconds = orig_val_nz->seconds;

        /* duplicate second fractions */
        if (orig_val_nz->fractions_s) {
            dup_val_nz->fractions_s = strdup(orig_val_nz->fractions_s);
            LY_CHECK_ERR_GOTO(!dup_val_nz->fractions_s, ret = LY_EMEM, error);
        }
    }

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_time(ctx, dup);
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_free_clb for ietf-yang-types time and time-no-zone type.
 */
static void
lyplg_type_free_time(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_time *val, *val_nz;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;

    if (!strcmp(value->realtype->name, "time")) {
        LYD_VALUE_GET(value, val);
        if (val) {
            free(val->fractions_s);
            LYPLG_TYPE_VAL_INLINE_DESTROY(val);
        }
    } else {
        LYD_VALUE_GET(value, val_nz);
        if (val_nz) {
            free(val_nz->fractions_s);
            LYPLG_TYPE_VAL_INLINE_DESTROY(val_nz);
        }
    }
}

/**
 * @brief Plugin information for time and time-no-zone type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_time[] = {
    {
        .module = "ietf-yang-types",
        .revision = NULL,
        .name = "time",

        .plugin.id = "ly2 time",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_time,
        .plugin.validate_value = lyplg_type_validate_value_string,
        .plugin.validate_tree = NULL,
        .plugin.compare = lyplg_type_compare_time,
        .plugin.sort = lyplg_type_sort_time,
        .plugin.print = lyplg_type_print_time,
        .plugin.duplicate = lyplg_type_dup_time,
        .plugin.free = lyplg_type_free_time,
    },
    {
        .module = "ietf-yang-types",
        .revision = NULL,
        .name = "time-no-zone",

        .plugin.id = "ly2 time",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_time,
        .plugin.validate_value = lyplg_type_validate_value_string,
        .plugin.validate_tree = NULL,
        .plugin.compare = lyplg_type_compare_time,
        .plugin.sort = lyplg_type_sort_time,
        .plugin.print = lyplg_type_print_time,
        .plugin.duplicate = lyplg_type_dup_time,
        .plugin.free = lyplg_type_free_time,
    },
    {0}
};
