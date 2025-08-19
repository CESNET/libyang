/**
 * @file string.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in string type plugin.
 *
 * Copyright (c) 2019 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_types.h"

#include <stdint.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesString string (built-in)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | variable, full bytes | yes | `char *` | string itself |
 */

/**
 * @brief Check string value for invalid characters.
 *
 * @param[in] value String to check.
 * @param[in] value_size Size of @p value.
 * @param[out] err Generated error on error.
 * @return LY_ERR value.
 */
static LY_ERR
string_check_chars(const char *value, uint32_t value_size, struct ly_err_item **err)
{
    uint32_t len, parsed = 0;

    while (value_size - parsed) {
        if (ly_checkutf8(value + parsed, value_size - parsed, &len)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid character 0x%hhx.", value[parsed]);
        }
        parsed += len;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_store_string(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), const struct lysc_ext_instance *UNUSED(top_ext),
        struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres), struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t value_size;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size("string", format, value_size_bits, LYPLG_LYB_SIZE_VARIABLE_BYTES, 0, &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(options & LYPLG_TYPE_STORE_IS_UTF8)) {
        /* check the UTF-8 encoding */
        ret = string_check_chars(value, value_size, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* store canonical value */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);

        /* value may have been freed */
        value = storage->_canonical;
    } else {
        ret = lydict_insert(ctx, value_size ? value : "", value_size, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (!(options & LYPLG_TYPE_STORE_ONLY)) {
        /* validate value */
        ret = lyplg_type_validate_value_string(ctx, type, storage, err);
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

LIBYANG_API_DEF LY_ERR
lyplg_type_validate_value_string(const struct ly_ctx *ctx, const struct lysc_type *type, struct lyd_value *storage,
        struct ly_err_item **err)
{
    LY_ERR ret;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    const void *value;
    size_t value_len;

    LY_CHECK_ARG_RET(NULL, type, storage, err, LY_EINVAL);
    value = lyd_value_get_canonical(ctx, storage);
    value_len = strlen(value);
    *err = NULL;

    /* length restriction of the string */
    if (type_str->length) {
        /* value_len is in bytes, but we need number of characters here */
        ret = lyplg_type_validate_range(LY_TYPE_STRING, type_str->length, ly_utf8len(value, value_len), value, value_len, err);
        LY_CHECK_RET(ret);
    }

    /* pattern restrictions */
    ret = lyplg_type_validate_patterns(ctx, type_str->patterns, value, value_len, err);
    LY_CHECK_RET(ret);

    return LY_SUCCESS;
}

/**
 * @brief Plugin information for string type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_string[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_STRING_STR,

        .plugin.id = "ly2 string",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bytes,
        .plugin.store = lyplg_type_store_string,
        .plugin.validate_value = lyplg_type_validate_value_string,
        .plugin.validate_tree = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {0}
};
