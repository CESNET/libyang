/**
 * @file time_period.c
 * @author Roman Janota <janota@cesnet.cz>
 * @brief libnetconf2-netconf-server time-period type plugin.
 *
 * Copyright (c) 2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_types.h"

#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesTimePeriod time-period (libnetconf2-netconf-server)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | string length | yes | `char *` | time in either months, weeks, days, or hours |
 */

/**
 * @brief Implementation of ::lyplg_type_sort_clb for libnetconf2-netconf-server time-period type.
 *
 * The values are sorted in descending order, i.e. the longest expiration time comes first.
 */
static int
lyplg_type_sort_time_period(const struct ly_ctx *ctx, const struct lyd_value *val1, const struct lyd_value *val2)
{
    const char *value1, *value2;
    char unit1, unit2;
    long v1, v2;

    value1 = lyd_value_get_canonical(ctx, val1);
    value2 = lyd_value_get_canonical(ctx, val2);

    /* get the units (last character) and the values (all characters except the last one) */
    unit1 = value1[strlen(value1) - 1];
    unit2 = value2[strlen(value2) - 1];
    v1 = strtol(value1, NULL, 10);
    v2 = strtol(value2, NULL, 10);

    /* descending order */
    if (unit1 == unit2) {
        if (v1 > v2) {
            return -1;
        } else if (v1 == v2) {
            return 0;
        } else {
            return 1;
        }
    } else if (unit1 == 'm') {
        return -1;
    } else if ((unit1 == 'w') && (unit2 != 'm')) {
        return -1;
    } else if ((unit1 == 'd') && (unit2 == 'h')) {
        return -1;
    } else {
        return 1;
    }
}

/**
 * @brief Plugin information for time-period type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_time_period[] = {
    {
        .module = "libnetconf2-netconf-server",
        .revision = "2024-07-09",
        .name = "time-period",

        .plugin.id = "libyang 2 - time-period, version 1",
        .plugin.store = lyplg_type_store_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_time_period,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
