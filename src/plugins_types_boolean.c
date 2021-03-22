/**
 * @file plugins_types_boolean.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in boolean type plugin.
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
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

API LY_ERR
ly_type_store_boolean(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int8_t i;

    *err = NULL;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    if ((value_len == ly_strlen_const("true")) && !strncmp(value, "true", ly_strlen_const("true"))) {
        i = 1;
    } else if ((value_len == ly_strlen_const("false")) && !strncmp(value, "false", ly_strlen_const("false"))) {
        i = 0;
    } else {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid boolean value \"%.*s\".", (int)value_len, value);
        goto cleanup;
    }

    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->canonical);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, value, value_len, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->boolean = i;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

const struct lyplg_type_record plugins_boolean[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_BOOL_STR,

        .plugin.id = "libyang 2 - boolean, version 1",
        .plugin.type = LY_TYPE_BOOL,
        .plugin.store = ly_type_store_boolean,
        .plugin.validate = NULL,
        .plugin.compare = ly_type_compare_simple,
        .plugin.print = ly_type_print_simple,
        .plugin.duplicate = ly_type_dup_simple,
        .plugin.free = ly_type_free_simple
    },
    {0}
};
