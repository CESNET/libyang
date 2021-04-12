/**
 * @file plugins_types_empty.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in empty type plugin.
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

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

API LY_ERR
lyplg_type_store_empty(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;

    *err = NULL;

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    if (value_len) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid empty value \"%.*s\".", (int)value_len, value);
        goto cleanup;
    }

    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, "", value_len, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->ptr = NULL;
    storage->realtype = type;

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

API LY_ERR
lyplg_type_compare_empty(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    /* empty has just one value, so empty data must be always the same */
    return LY_SUCCESS;
}

/**
 * @brief Plugin information for empty type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_empty[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_EMPTY_STR,

        .plugin.id = "libyang 2 - empty, version 1",
        .plugin.type = LY_TYPE_EMPTY,
        .plugin.store = lyplg_type_store_empty,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_empty,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
