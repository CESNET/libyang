/**
 * @file hex_string.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in hex-string and associated types plugin.
 *
 * Copyright (c) 2023 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins_types.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesHexString phys-address, mac-address, hex-string, uuid (ietf-yang-types)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | variable, full bytes | yes | `char *` | string itself |
 */

LIBYANG_API_DEF LY_ERR
lyplg_type_store_hex_string(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    uint32_t value_size, i;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size("hex-string", format, value_size_bits, LYPLG_LYB_SIZE_VARIABLE_BYTES, 0,
            &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* length restriction of the string */
    if (type_str->length) {
        /* value_len is in bytes, but we need number of characters here */
        ret = lyplg_type_validate_range(LY_TYPE_STRING, type_str->length, ly_utf8len(value, value_len), value, value_len, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* pattern restrictions */
    ret = lyplg_type_validate_patterns(type_str->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* make a copy, it is needed for canonization */
    if ((format != LY_VALUE_CANON) && !(options & LYPLG_TYPE_STORE_DYNAMIC)) {
        value = strndup(value, value_size);
        LY_CHECK_ERR_GOTO(!value, ret = LY_EMEM, cleanup);
        options |= LYPLG_TYPE_STORE_DYNAMIC;
    }

    /* store canonical value */
    if (format != LY_VALUE_CANON) {
        /* make lowercase and store, the value must be dynamic */
        for (i = 0; i < value_size; ++i) {
            ((char *)value)[i] = tolower(((char *)value)[i]);
        }

        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);

        /* value may have been freed */
        value = storage->_canonical;
    } else {
        /* store directly */
        ret = lydict_insert(ctx, value_size ? value : "", value_size, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (!(options & LYPLG_TYPE_STORE_ONLY)) {
        /* validate length restriction of the string */
        if (type_str->length) {
            /* value_size is in bytes, but we need number of characters here */
            ret = lyplg_type_validate_range(LY_TYPE_STRING, type_str->length, ly_utf8len(value, value_size), value,
                    value_size, err);
            LY_CHECK_GOTO(ret, cleanup);
        }

        /* validate pattern restrictions */
        ret = lyplg_type_validate_patterns(ctx, type_str->patterns, value, value_size, err);
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
 * @brief Plugin information for string type implementation.
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

        .plugin.id = "ly2 hex-string",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "mac-address",

        .plugin.id = "ly2 hex-string",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "hex-string",

        .plugin.id = "ly2 hex-string",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "uuid",

        .plugin.id = "ly2 hex-string",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_hex_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {0}
};
