/**
 * @file hex_string.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types hex-string (and similar) type plugin.
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
 * @brief Validate, canonize and store value of the ietf-yang-types hex-string type.
 * Implementation of the ::lyplg_type_store_clb.
 */
static LY_ERR
lyplg_type_store_hex_string(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    char *str;
    uint32_t i, len;

    /* store as a string */
    ret = lyplg_type_store_string(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node,
            storage, unres, err);
    LY_CHECK_RET(ret);

    str = strdup(storage->_canonical);
    if (!str) {
        /* we can hardly allocate an error message */
        ret = LY_EMEM;
        goto cleanup;
    }

    len = strlen(str);
    for (i = 0; i < len; ++i) {
        if ((str[i] >= 'A') && (str[i] <= 'Z')) {
            /* make it lowercase (canonical format) */
            str[i] += 32;
        }
    }

    /* update the value correctly */
    lydict_remove(ctx, storage->_canonical);
    storage->_canonical = NULL;
    LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, str, &storage->_canonical), cleanup);

    /* validation succeeded and we do not want to change how it is stored */

cleanup:
    if (ret) {
        type->plugin->free(ctx, storage);
    }
    return ret;
}

/**
 * @brief Plugin information for hex-string type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_hex_string[] = {
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "phys-address",

        .plugin.id = "libyang 2 - phys-address, version 1",
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "mac-address",

        .plugin.id = "libyang 2 - mac-address, version 1",
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "hex-string",

        .plugin.id = "libyang 2 - hex-string, version 1",
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "uuid",

        .plugin.id = "libyang 2 - uuid, version 1",
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
