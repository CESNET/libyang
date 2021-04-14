/**
 * @file date_and_time.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types date-and-time type plugin.
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

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/**
 * @brief Convert string to a number.
 *
 * @param[in,out] str String to convert, the parsed number are skipped.
 * @param[in] len Expected length of the number. 0 to parse at least 1 character.
 * @param[in] full_str Full string for error generation.
 * @param[out] num Converted number.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
convert_number(const char **str, int len, const char *full_str, int *num, struct ly_err_item **err)
{
    char *ptr;

    *num = strtoul(*str, &ptr, 10);
    if ((len && (ptr - *str != len)) || (!len && (ptr == *str))) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid character '%c' in date-and-time value \"%s\", "
                "a digit expected.", ptr[0], full_str);
    }

    *str = ptr;
    return LY_SUCCESS;
}

/**
 * @brief Check a single character.
 *
 * @param[in,out] str String to check, the parsed character is skipped.
 * @param[in] c Array of possible characters.
 * @param[in] full_str Full string for error generation.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
check_char(const char **str, char c[], const char *full_str, struct ly_err_item **err)
{
    LY_ERR ret;
    uint32_t i;
    char *exp_str;

    for (i = 0; c[i]; ++i) {
        if ((*str)[0] == c[i]) {
            break;
        }
    }
    if (!c[i]) {
        /* "'c'" + (" or 'c'")* + \0 */
        exp_str = malloc(3 + (i - 1) * 7 + 1);
        sprintf(exp_str, "'%c'", c[0]);
        for (i = 1; c[i]; ++i) {
            sprintf(exp_str + strlen(exp_str), " or '%c'", c[i]);
        }
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid character '%c' in date-and-time value \"%s\", "
                "%s expected.", (*str)[0], full_str, exp_str);
        free(exp_str);
        return ret;
    }

    ++(*str);
    return LY_SUCCESS;
}

/**
 * @brief Validate, canonize and store value of the ietf-yang-types date-and-time type.
 * Implementation of the ::lyplg_type_store_clb.
 */
static LY_ERR
lyplg_type_store_date_and_time(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct tm tm = {0}, tm2;
    int num;
    const char *val_str;

    /* store as a string */
    ret = lyplg_type_store_string(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node,
            storage, unres, err);
    LY_CHECK_RET(ret);

    /* canonize */
    /* \d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[\+\-]\d{2}:\d{2})
     * 2018-03-21T09:11:05(.55785...)(Z|+02:00) */
    val_str = storage->canonical;

    /* year */
    ret = convert_number(&val_str, 4, storage->canonical, &tm.tm_year, err);
    LY_CHECK_GOTO(ret, cleanup);
    tm.tm_year -= 1900;

    LY_CHECK_GOTO(ret = check_char(&val_str, "-", storage->canonical, err), cleanup);

    /* month */
    ret = convert_number(&val_str, 2, storage->canonical, &tm.tm_mon, err);
    LY_CHECK_GOTO(ret, cleanup);
    tm.tm_mon -= 1;

    LY_CHECK_GOTO(ret = check_char(&val_str, "-", storage->canonical, err), cleanup);

    /* day */
    ret = convert_number(&val_str, 2, storage->canonical, &tm.tm_mday, err);
    LY_CHECK_GOTO(ret, cleanup);

    LY_CHECK_GOTO(ret = check_char(&val_str, "T", storage->canonical, err), cleanup);

    /* hours */
    ret = convert_number(&val_str, 2, storage->canonical, &tm.tm_hour, err);
    LY_CHECK_GOTO(ret, cleanup);

    LY_CHECK_GOTO(ret = check_char(&val_str, ":", storage->canonical, err), cleanup);

    /* minutes */
    ret = convert_number(&val_str, 2, storage->canonical, &tm.tm_min, err);
    LY_CHECK_GOTO(ret, cleanup);

    LY_CHECK_GOTO(ret = check_char(&val_str, ":", storage->canonical, err), cleanup);

    /* seconds */
    ret = convert_number(&val_str, 2, storage->canonical, &tm.tm_sec, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* do not move the pointer */
    LY_CHECK_GOTO(ret = check_char(&val_str, ".Z+-", storage->canonical, err), cleanup);
    --val_str;

    /* validate using mktime() */
    tm2 = tm;
    errno = 0;
    mktime(&tm);
    /* ENOENT is set when "/etc/localtime" is missing but the function suceeeds */
    if (errno && (errno != ENOENT)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Checking date-and-time value \"%s\" failed (%s).",
                storage->canonical, strerror(errno));
        goto cleanup;
    }
    /* we now have correctly filled the remaining values, use them */
    memcpy(((char *)&tm2) + (6 * sizeof(int)), ((char *)&tm) + (6 * sizeof(int)), sizeof(struct tm) - (6 * sizeof(int)));
    /* back it up again */
    tm = tm2;
    /* let mktime() correct date & time with having the other values correct now */
    errno = 0;
    mktime(&tm);
    if (errno && (errno != ENOENT)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Checking date-and-time value \"%s\" failed (%s).",
                storage->canonical, strerror(errno));
        goto cleanup;
    }
    /* detect changes in the filled values */
    if (memcmp(&tm, &tm2, 6 * sizeof(int))) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Checking date-and-time value \"%s\" failed, "
                "canonical date and time is \"%04d-%02d-%02dT%02d:%02d:%02d\".", storage->canonical,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        goto cleanup;
    }

    /* tenth of a second */
    if (val_str[0] == '.') {
        ++val_str;
        ret = convert_number(&val_str, 0, storage->canonical, &num, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    switch (val_str[0]) {
    case 'Z':
        /* done */
        ++val_str;
        break;
    case '+':
    case '-':
        /* timezone shift */
        if ((val_str[1] < '0') || (val_str[1] > '2')) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".",
                    val_str, storage->canonical);
            goto cleanup;
        }
        if ((val_str[2] < '0') || ((val_str[1] == '2') && (val_str[2] > '3')) || (val_str[2] > '9')) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".",
                    val_str, storage->canonical);
            goto cleanup;
        }

        if (val_str[3] != ':') {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".",
                    val_str, storage->canonical);
            goto cleanup;
        }

        if ((val_str[4] < '0') || (val_str[4] > '5')) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".",
                    val_str, storage->canonical);
            goto cleanup;
        }
        if ((val_str[5] < '0') || (val_str[5] > '9')) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".",
                    val_str, storage->canonical);
            goto cleanup;
        }

        val_str += 6;
        break;
    default:
        LY_CHECK_GOTO(ret = check_char(&val_str, "Z+-", storage->canonical, err), cleanup);
    }

    /* no other characters expected */
    if (val_str[0]) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid character '%c' in date-and-time value \"%s\", "
                "no characters expected.", val_str[0], storage->canonical);
        goto cleanup;
    }

    /* validation succeeded and we do not want to change how it is stored */

cleanup:
    if (ret) {
        type->plugin->free(ctx, storage);
    }
    return ret;
}

/**
 * @brief Plugin information for date-and-time type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_date_and_time[] = {
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "date-and-time",

        .plugin.id = "libyang 2 - date-and-time, version 1",
        .plugin.store = lyplg_type_store_date_and_time,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
