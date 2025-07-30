/**
 * @file empty.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in empty type plugin.
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
 * @subsection howtoDataLYBTypesEmpty empty (built-in)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 0        | yes | `void` | none |
 */

static void
lyplg_type_lyb_size_empty(const struct lysc_type *UNUSED(type), enum lyplg_lyb_size_type *size_type,
        uint32_t *fixed_size_bits)
{
    *size_type = LYPLG_LYB_SIZE_FIXED_BITS;
    *fixed_size_bits = 0;
}

static LY_ERR
lyplg_type_store_empty(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), const struct lysc_ext_instance *UNUSED(top_ext),
        struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres), struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t value_size;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* validation */
    if (value_size_bits) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid empty value size %" PRIu32 " b.", value_size_bits);
        goto cleanup;
    }
    value_size = 0;

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* store canonical value, it always is */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lydict_insert(ctx, "", value_size, &storage->_canonical);
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

        .plugin.id = "ly2 empty",
        .plugin.lyb_size = lyplg_type_lyb_size_empty,
        .plugin.store = lyplg_type_store_empty,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {0}
};
