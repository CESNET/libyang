/**
 * @file plugins_types_bits.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in bits type plugin.
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

#include <assert.h>
#include <ctype.h>
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
ly_type_store_bits(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)type;
    struct lysc_type_bitenum_item **bits_items = NULL;
    struct ly_set *items = NULL;

    uint32_t index_start;   /* start index of bit name */
    uint32_t index_end = 0; /* end index of bit name */
    uint32_t item_len;      /* length of bit name */
    uint32_t buf_len = 0;
    uint32_t item_pos;
    uint32_t item_pos_expected;
    char *buf = NULL;
    const char *item = NULL;

    ly_bool item_present;
    LY_ARRAY_COUNT_TYPE item_present_index;
    LY_ARRAY_COUNT_TYPE it;

    *err = NULL;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup_value);

    /* remember the present items for further work */
    ret = ly_set_new(&items);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup_value);

    /* get all values */
    while (index_end < value_len) {
        /* skip leading spaces */
        index_start = index_end;
        while ((index_start < value_len) && isspace(value[index_start])) {
            index_start++;
        }

        index_end = index_start;
        /* find end of word */
        while ((index_end < value_len) && !isspace(value[index_end])) {
            index_end++;
        }

        /* check if name of bit is valid */
        item = &value[index_start];
        item_len = index_end - index_start;
        if (item_len == 0) {
            /* loop read all bits names*/
            break;
        }

        /* looking for correct name */
        item_present = 0;
        LY_ARRAY_FOR(type_bits->bits, it) {
            if (!ly_strncmp(type_bits->bits[it].name, item, item_len)) {
                item_present = 1;
                item_present_index = it;
            }
        }

        /* check if name exists */
        if (!item_present) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                    "Invalid bit value \"%.*s\".", (int)item_len, item);
            goto cleanup;
        }

        /* add item to set */
        item_pos_expected = items->count;
        ret = ly_set_add(items, &type_bits->bits[item_present_index], 0, &item_pos);
        LY_CHECK_GOTO(ret, cleanup);
        if (item_pos != item_pos_expected) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                    "Bit \"%s\" used multiple times.", type_bits->bits[item_present_index].name);
            goto cleanup;
        }
        /* count require buff size */
        buf_len += item_len + 1;
    }

    /* creating buffer for cannonical value */
    if (buf_len != 0) {
        uint32_t buf_index = 0;
        LY_ARRAY_COUNT_TYPE it;

        /* create space for cannonical value and array for bits*/
        buf = malloc(buf_len * sizeof *buf);
        if (buf == NULL) {
            ret = LY_EMEM;
            LOGMEM(ctx);
            goto cleanup;
        }
        LY_ARRAY_CREATE_GOTO(ctx, bits_items, items->count, ret, cleanup);

        /* generate ordered bits list and cannonical value*/
        LY_ARRAY_FOR(type_bits->bits, it) {
            if (ly_set_contains(items, &type_bits->bits[it], NULL)) {
                uint32_t name_index = 0;

                /* write space */
                if (buf_index != 0) {
                    buf[buf_index] = ' ';
                    buf_index++;
                }

                /* write bit name*/
                while (type_bits->bits[it].name[name_index]) {
                    buf[buf_index] = type_bits->bits[it].name[name_index];
                    buf_index++;
                    name_index++;
                }

                bits_items[LY_ARRAY_COUNT(bits_items)] = &type_bits->bits[it];
                LY_ARRAY_INCREMENT(bits_items);
            }
        }
        buf[buf_index] = 0;

        ret = lydict_insert_zc(ctx, buf, &storage->canonical);
        buf = NULL;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        bits_items = NULL;
        ret = lydict_insert(ctx, "", 0, &storage->canonical);
        buf = NULL;
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* store value */
    storage->bits_items = bits_items;
    storage->realtype = type;

    /* RETURN LY_SUCCESS */
    ly_set_free(items, NULL);
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return LY_SUCCESS;

    /* ERROR HANDLING */
cleanup:
    LY_ARRAY_FREE(bits_items);
    free(buf);
    ly_set_free(items, NULL);
cleanup_value:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

API LY_ERR
ly_type_dup_bits(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_type_bitenum_item **bits_items = NULL;

    LY_ARRAY_CREATE_RET(ctx, bits_items, LY_ARRAY_COUNT(original->bits_items), LY_EMEM);
    LY_ARRAY_FOR(original->bits_items, u) {
        LY_ARRAY_INCREMENT(bits_items);
        bits_items[u] = original->bits_items[u];
    }

    ret = lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    dup->bits_items = bits_items;
    dup->realtype = original->realtype;
    return LY_SUCCESS;

    /* ERROR HANDLING */
cleanup:
    LY_ARRAY_FREE(bits_items);
    return ret;
}

API void
ly_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value)
{
    LY_ARRAY_FREE(value->bits_items);
    value->bits_items = NULL;

    lydict_remove(ctx, value->canonical);
    value->canonical = NULL;
}

const struct lyplg_type_record plugins_bits[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_BITS_STR,

        .plugin.id = "libyang 2 - bits, version 1",
        .plugin.type = LY_TYPE_BITS,
        .plugin.store = ly_type_store_bits,
        .plugin.validate = NULL,
        .plugin.compare = ly_type_compare_simple,
        .plugin.print = ly_type_print_simple,
        .plugin.duplicate = ly_type_dup_bits,
        .plugin.free = ly_type_free_bits
    },
    {0}
};
