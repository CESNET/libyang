/**
 * @file enumeration.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in enumeration type plugin.
 *
 * Copyright (c) 2019 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* strdup */

#include "plugins_types.h"

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
 * @subsection howtoDataLYBTypesEnumeration enumeration (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | fixed for a specific type | yes | `int32 *` | assigned little-endian value of the enum |
 */

#define ENUM_FIXED_SIZE_BITS(type) \
        lyplg_type_get_highest_set_bit_pos(\
        ((struct lysc_type_enum *)type)->enums[LY_ARRAY_COUNT(((struct lysc_type_enum *)type)->enums) - 1].value)

static void
lyplg_type_lyb_size_enum(const struct lysc_type *type, enum lyplg_lyb_size_type *size_type, uint32_t *fixed_size_bits)
{
    const struct lysc_type_enum *type_enum = (struct lysc_type_enum *)type;
    uint32_t max_value;

    *size_type = LYPLG_LYB_SIZE_FIXED_BITS;

    if (type_enum->enums[0].value < 0) {
        /* we need the full 4 bytes */
        *fixed_size_bits = 32;
        return;
    }

    /* value of the last enum, sorted */
    max_value = type_enum->enums[LY_ARRAY_COUNT(type_enum->enums) - 1].value;

    /* learn the position of the highest set bit, the least amount of bits that can hold the number */
    *fixed_size_bits = lyplg_type_get_highest_set_bit_pos(max_value);
}

static LY_ERR
lyplg_type_store_enum(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), const struct lysc_ext_instance *UNUSED(top_ext),
        struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres), struct ly_err_item **err)
{
    struct lysc_type_enum *type_enum = (struct lysc_type_enum *)type;
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool found = 0;
    uint32_t value_size;
    int64_t num = 0;
    int32_t num_val;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size("enumeration", format, value_size_bits, LYPLG_LYB_SIZE_FIXED_BITS,
            ENUM_FIXED_SIZE_BITS(type), &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (format == LY_VALUE_LYB) {
        /* convert the value to host byte order */
        memcpy(&num, value, value_size);
        num = le64toh(num);
        num_val = num;

        /* find the matching enumeration value item */
        LY_ARRAY_FOR(type_enum->enums, u) {
            if (type_enum->enums[u].value == num_val) {
                found = 1;
                break;
            }
        }

        if (!found) {
            /* value not found */
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid enumeration value % " PRIi32 ".", num_val);
            goto cleanup;
        }

        /* store value */
        storage->enum_item = &type_enum->enums[u];

        /* canonical settings via dictionary due to free callback */
        ret = lydict_insert(ctx, type_enum->enums[u].name, 0, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* find the matching enumeration value item */
    LY_ARRAY_FOR(type_enum->enums, u) {
        if (!ly_strncmp(type_enum->enums[u].name, value, value_size)) {
            found = 1;
            break;
        }
    }

    if (!found) {
        /* enum not found */
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid enumeration value \"%.*s\".", (int)value_size,
                (char *)value);
        goto cleanup;
    }

    /* store value */
    storage->enum_item = &type_enum->enums[u];

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
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

static int
lyplg_type_sort_enum(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    if (val1->enum_item->value > val2->enum_item->value) {
        return -1;
    } else if (val1->enum_item->value < val2->enum_item->value) {
        return 1;
    } else {
        return 0;
    }
}

static const void *
lyplg_type_print_enum(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint32_t *value_size_bits)
{
    int64_t prev_num = 0, num = 0;
    void *buf;

    if (format == LY_VALUE_LYB) {
        prev_num = num = value->enum_item->value;
        num = htole64(num);
        if (num == prev_num) {
            /* values are equal, little-endian */
            *dynamic = 0;
            if (value_size_bits) {
                *value_size_bits = ENUM_FIXED_SIZE_BITS(value->realtype);
            }
            return &value->enum_item->value;
        } else {
            /* values differ, big-endian */
            buf = calloc(1, 4);
            LY_CHECK_RET(!buf, NULL);

            *dynamic = 1;
            if (value_size_bits) {
                *value_size_bits = ENUM_FIXED_SIZE_BITS(value->realtype);
            }
            memcpy(buf, &num, 4);
            return buf;
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
 * @brief Plugin information for enumeration type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_enumeration[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_ENUM_STR,

        .plugin.id = "ly2 enumeration",
        .plugin.lyb_size = lyplg_type_lyb_size_enum,
        .plugin.store = lyplg_type_store_enum,
        .plugin.validate_value = NULL,
        .plugin.validate_tree = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_enum,
        .plugin.print = lyplg_type_print_enum,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {0}
};
