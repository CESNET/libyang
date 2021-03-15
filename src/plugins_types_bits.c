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

/**
 * @brief Validate, canonize and store value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_store_clb.
 */
LY_ERR
ly_type_store_bits(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    size_t item_len;
    const char *item;
    struct ly_set *items = NULL, *items_ordered = NULL;
    size_t buf_size = 0;
    char *buf = NULL;
    size_t index;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)type;
    ly_bool iscanonical = 1;
    size_t ws_count;
    size_t lws_count; /* leading whitespace count */
    const char *can = NULL;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup_value);

    /* remember the present items for further work */
    ret = ly_set_new(&items);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup_value);

    ws_count = lws_count = 0;
    for (index = 0; index < value_len; ++index) {
        if (isspace(value[index])) {
            ++ws_count;
            continue;
        }
        if (index == ws_count) {
            lws_count = ws_count;
        } else if (ws_count) {
            iscanonical = 0;
        }
        ws_count = 0;

        /* start of the item */
        item = &value[index];
        for (item_len = 0; index + item_len < value_len && !isspace(item[item_len]); item_len++) {}
        LY_ARRAY_FOR(type_bits->bits, u) {
            if (!ly_strncmp(type_bits->bits[u].name, item, item_len)) {
                /* we have the match */
                uint32_t inserted;

                if (iscanonical && items->count && (type_bits->bits[u].position < ((struct lysc_type_bitenum_item *)items->objs[items->count - 1])->position)) {
                    iscanonical = 0;
                }
                ret = ly_set_add(items, &type_bits->bits[u], 0, &inserted);
                LY_CHECK_GOTO(ret, cleanup);
                if (inserted != items->count - 1) {
                    ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                            "Bit \"%s\" used multiple times.", type_bits->bits[u].name);
                    goto cleanup;
                }
                goto next;
            }
        }
        /* item not found */
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid bit value \"%.*s\".", (int)item_len, item);
        goto cleanup;
next:
        /* remember for canonized form: item + space/termination-byte */
        buf_size += item_len + 1;
        index += item_len;
    }
    /* validation done */

    if (iscanonical) {
        /* items are already ordered */
        items_ordered = items;
        items = NULL;

        if (!ws_count && !lws_count && (options & LY_TYPE_STORE_DYNAMIC)) {
            ret = lydict_insert_zc(ctx, (char *)value, &can);
            value = NULL;
            options &= ~LY_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value_len ? &value[lws_count] : "", value_len - ws_count - lws_count, &can);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        buf = malloc(buf_size * sizeof *buf);
        if (!buf) {
            ret = ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
            goto cleanup;
        }
        index = 0;

        ret = ly_set_dup(items, NULL, &items_ordered);
        LY_CHECK_GOTO(ret, cleanup);
        items_ordered->count = 0;

        /* generate ordered bits list */
        LY_ARRAY_FOR(type_bits->bits, u) {
            if (ly_set_contains(items, &type_bits->bits[u], NULL)) {
                int c = sprintf(&buf[index], "%s%s", index ? " " : "", type_bits->bits[u].name);
                if (c < 0) {
                    ret = ly_err_new(err, LY_ESYS, 0, NULL, NULL, "sprintf() failed.");
                    goto cleanup;
                }
                index += c;
                ret = ly_set_add(items_ordered, &type_bits->bits[u], 1, NULL);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }
        assert(buf_size == index + 1);
        /* termination NULL-byte */
        buf[index] = '\0';

        ret = lydict_insert_zc(ctx, buf, &can);
        buf = NULL;
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* store all data */
    storage->canonical = can;
    can = NULL;
    LY_ARRAY_CREATE_GOTO(ctx, storage->bits_items, items_ordered->count, ret, cleanup);
    for (uint32_t x = 0; x < items_ordered->count; x++) {
        storage->bits_items[x] = items_ordered->objs[x];
        LY_ARRAY_INCREMENT(storage->bits_items);
    }
    storage->realtype = type;

cleanup:
    ly_set_free(items, NULL);
    ly_set_free(items_ordered, NULL);
    free(buf);
    lydict_remove(ctx, can);
cleanup_value:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

/* @brief Duplication callback of the bits values.
 *
 * Implementation of the ly_type_dup_clb.
 */
LY_ERR
ly_type_dup_bits(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_CREATE_RET(ctx, dup->bits_items, LY_ARRAY_COUNT(original->bits_items), LY_EMEM);
    LY_ARRAY_FOR(original->bits_items, u) {
        LY_ARRAY_INCREMENT(dup->bits_items);
        dup->bits_items[u] = original->bits_items[u];
    }

    LY_CHECK_RET(lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical));
    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

/**
 * @brief Free value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_free_clb.
 */
void
ly_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value)
{
    LY_ARRAY_FREE(value->bits_items);
    value->bits_items = NULL;

    lydict_remove(ctx, value->canonical);
    value->canonical = NULL;
}
