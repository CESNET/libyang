/**
 * @file boolean.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in boolean type plugin.
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
 * @subsection howtoDataLYBTypesBoolean boolean (built-in)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 1 | yes | `int8_t *` | 0 for false, otherwise true |
 */

static void
lyplg_type_lyb_size_boolean(const struct lysc_type *UNUSED(type), enum lyplg_lyb_size_type *size_type,
        uint32_t *fixed_size_bits)
{
    *size_type = LYPLG_LYB_SIZE_FIXED_BITS;
    *fixed_size_bits = 1;
}

static LY_ERR
lyplg_type_store_boolean(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), const struct lysc_ext_instance *UNUSED(top_ext),
        struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres), struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t value_size;
    uint8_t i;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size("boolean", format, value_size_bits, LYPLG_LYB_SIZE_FIXED_BITS, 1, &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (format == LY_VALUE_LYB) {
        /* store value */
        i = *(uint8_t *)value;
        storage->boolean = i ? 1 : 0;

        /* store canonical value, it always is */
        ret = lydict_insert(ctx, i ? "true" : "false", 0, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* validate and store the value */
    if ((value_size == ly_strlen_const("true")) && !strncmp(value, "true", ly_strlen_const("true"))) {
        i = 1;
    } else if ((value_size == ly_strlen_const("false")) && !strncmp(value, "false", ly_strlen_const("false"))) {
        i = 0;
    } else {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid boolean value \"%.*s\".", (int)value_size,
                (char *)value);
        goto cleanup;
    }
    storage->boolean = i;

    /* store canonical value, it always is */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lydict_insert(ctx, value, value_size, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

static LY_ERR
lyplg_type_compare_boolean(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->boolean != val2->boolean) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

static int
lyplg_type_sort_boolean(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->boolean > val2->boolean) {
        return 1;
    } else if (val1->boolean < val2->boolean) {
        return -1;
    } else {
        return 0;
    }
}

static const void *
lyplg_type_print_boolean(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint32_t *value_size_bits)
{
    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_size_bits) {
            *value_size_bits = 1;
        }
        return &value->boolean;
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
 * @brief Plugin information for boolean type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_boolean[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_BOOL_STR,

        .plugin.id = "ly2 boolean",
        .plugin.lyb_size = lyplg_type_lyb_size_boolean,
        .plugin.store = lyplg_type_store_boolean,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_boolean,
        .plugin.sort = lyplg_type_sort_boolean,
        .plugin.print = lyplg_type_print_boolean,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {0}
};
