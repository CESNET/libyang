/**
 * @file bits.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in bits type plugin.
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
 * @subsection howtoDataLYBTypesBits bits (built-in)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | fixed for a specific type | yes | pointer to integer type of the specific size, if size more than 8 use `char *` | bitmap of the set bits |
 */

static void lyplg_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Get the position of the last bit.
 */
#define BITS_LAST_BIT_POSITION(type_bits) (type_bits->bits[LY_ARRAY_COUNT(type_bits->bits) - 1].position)

/**
 * @brief Get a specific byte in a bitmap.
 */
#ifdef IS_BIG_ENDIAN
# define BITS_BITMAP_BYTE(bitmap, size, idx) (bitmap + (size - 1) - idx)
#else
# define BITS_BITMAP_BYTE(bitmap, size, idx) (bitmap + idx)
#endif

LIBYANG_API_DEF ly_bool
lyplg_type_bits_is_bit_set(const char *bitmap, uint32_t size_bits, uint32_t bit_position)
{
    char bitmask;
    uint32_t size;

    /* get size in bytes */
    size = size_bits / 8 + (size_bits % 8) ? 1 : 0;

    /* find the byte with our bit */
    (void)size;
    bitmap = BITS_BITMAP_BYTE(bitmap, size, bit_position / 8);
    bit_position %= 8;

    /* generate bitmask */
    bitmask = 1;
    bitmask <<= bit_position;

    /* check if bit set */
    if (*bitmap & bitmask) {
        return 1;
    }
    return 0;
}

/**
 * @brief Set bit at a specific position.
 *
 * @param[in,out] bitmap Bitmap to modify.
 * @param[in] size_bits Size of @p bitmap in bits.
 * @param[in] bit_position Bit position to set.
 */
static void
bits_bit_set(char *bitmap, uint32_t size_bits, uint32_t bit_position)
{
    char bitmask;
    uint32_t size;

    /* get size in bytes */
    size = size_bits / 8 + (size_bits % 8) ? 1 : 0;

    /* find the byte with our bit */
    (void)size;
    bitmap = BITS_BITMAP_BYTE(bitmap, size, bit_position / 8);
    bit_position %= 8;

    /* generate bitmask */
    bitmask = 1;
    bitmask <<= bit_position;

    /* set the bit */
    *bitmap |= bitmask;
}

static int32_t
lyplg_type_lyb_size_bits(const struct lysc_type *type)
{
    const struct lysc_type_bits *type_bits = (struct lysc_type_bits *)type;

    /* position of the last bit, positions start at 0, add 1 */
    return BITS_LAST_BIT_POSITION(type_bits) + 1;
}

/**
 * @brief Convert a list of bit names separated by whitespaces to a bitmap.
 *
 * @param[in] value Value to convert.
 * @param[in] value_len Length of @p value.
 * @param[in] type Type of the value.
 * @param[in,out] bitmap Zeroed bitmap, is filled (set).
 * @param[out] err Error information.
 * @return LY_ERR value.
 */
static LY_ERR
bits_str2bitmap(const char *value, uint32_t value_len, struct lysc_type_bits *type, char *bitmap, struct ly_err_item **err)
{
    const struct lysc_type *t = (struct lysc_type *)type;
    uint32_t idx_start, idx_end;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool found;

    idx_start = idx_end = 0;
    while (idx_end < value_len) {
        /* skip whitespaces */
        while ((idx_end < value_len) && isspace(value[idx_end])) {
            ++idx_end;
        }
        if (idx_end == value_len) {
            break;
        }

        /* parse bit name */
        idx_start = idx_end;
        while ((idx_end < value_len) && !isspace(value[idx_end])) {
            ++idx_end;
        }

        /* find the bit */
        found = 0;
        LY_ARRAY_FOR(type->bits, u) {
            if (!ly_strncmp(type->bits[u].name, value + idx_start, idx_end - idx_start)) {
                found = 1;
                break;
            }
        }

        /* check if name exists */
        if (!found) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid bit \"%.*s\".", (int)(idx_end - idx_start),
                    value + idx_start);
        }

        /* check for duplication */
        if (lyplg_type_bits_is_bit_set(bitmap, lyplg_type_lyb_size_bits(t), type->bits[u].position)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Duplicate bit \"%s\".", type->bits[u].name);
        }

        /* set the bit */
        bits_bit_set(bitmap, lyplg_type_lyb_size_bits(t), type->bits[u].position);
    }

    return LY_SUCCESS;
}

/**
 * @brief Add a bit item into an array.
 *
 * @param[in] position Bit position to add.
 * @param[in] type Bitis type to read the bit positions and names from.
 * @param[in,out] items Array of bit item pointers to add to.
 */
static void
bits_add_item(uint32_t position, struct lysc_type_bits *type, struct lysc_type_bitenum_item **items)
{
    LY_ARRAY_COUNT_TYPE u;

    /* find the bit item */
    LY_ARRAY_FOR(type->bits, u) {
        if (type->bits[u].position == position) {
            break;
        }
    }

    /* add it at the end */
    items[LY_ARRAY_COUNT(items)] = &type->bits[u];
    LY_ARRAY_INCREMENT(items);
}

/**
 * @brief Convert a bitmap to a sized array of pointers to their bit definitions.
 *
 * @param[in] bitmap Bitmap to read from.
 * @param[in] type Bits type.
 * @param[in,out] items Allocated sized array to fill with the set bits.
 */
static void
bits_bitmap2items(const char *bitmap, struct lysc_type_bits *type, struct lysc_type_bitenum_item **items)
{
    uint32_t bit_pos, i, bitmap_size;
    uint8_t bitmask;
    const uint8_t *byte;

    bitmap_size = LYPLG_BITS2BYTES(lyplg_type_lyb_size_bits((struct lysc_type *)type));

    bit_pos = 0;
    for (i = 0; i < bitmap_size; ++i) {
        /* check this byte (but not necessarily all bits in the last byte) */
        byte = (uint8_t *)BITS_BITMAP_BYTE(bitmap, bitmap_size, i);
        for (bitmask = 1; bitmask; bitmask <<= 1) {
            if (*byte & bitmask) {
                /* add this bit */
                bits_add_item(bit_pos, type, items);
            }

            if (bit_pos == BITS_LAST_BIT_POSITION(type)) {
                /* we have checked the last valid bit */
                break;
            }

            ++bit_pos;
        }
    }
}

/**
 * @brief Generate canonical value from ordered array of set bit items.
 *
 * @param[in] items Sized array of set bit items.
 * @param[out] canonical Canonical string value.
 * @return LY_ERR value.
 */
static LY_ERR
bits_items2canon(struct lysc_type_bitenum_item **items, char **canonical)
{
    char *ret;
    uint32_t ret_len;
    LY_ARRAY_COUNT_TYPE u;

    *canonical = NULL;

    /* init value */
    ret = strdup("");
    LY_CHECK_RET(!ret, LY_EMEM);
    ret_len = 0;

    LY_ARRAY_FOR(items, u) {
        if (!ret_len) {
            ret = ly_realloc(ret, strlen(items[u]->name) + 1);
            LY_CHECK_RET(!ret, LY_EMEM);
            strcpy(ret, items[u]->name);

            ret_len = strlen(ret);
        } else {
            ret = ly_realloc(ret, ret_len + 1 + strlen(items[u]->name) + 1);
            LY_CHECK_RET(!ret, LY_EMEM);
            sprintf(ret + ret_len, " %s", items[u]->name);

            ret_len += 1 + strlen(items[u]->name);
        }
    }

    *canonical = ret;
    return LY_SUCCESS;
}

static LY_ERR
lyplg_type_store_bits(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint32_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)type;
    struct lyd_value_bits *val;
    uint32_t value_size;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size("bits", format, value_size_bits, lyplg_type_lyb_size_bits(type), &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (format == LY_VALUE_LYB) {
        /* store value (bitmap) */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            val->bitmap = (char *)value;
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        } else {
            val->bitmap = malloc(value_size);
            LY_CHECK_ERR_GOTO(!val->bitmap, ret = LY_EMEM, cleanup);
            memcpy(val->bitmap, value, value_size);
        }

        /* allocate and fill the bit item array */
        LY_ARRAY_CREATE_GOTO(ctx, val->items, LY_ARRAY_COUNT(type_bits->bits), ret, cleanup);
        bits_bitmap2items(val->bitmap, type_bits, val->items);

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate the bitmap */
    val->bitmap = calloc(1, LYPLG_BITS2BYTES(lyplg_type_lyb_size_bits(type)));
    LY_CHECK_ERR_GOTO(!val->bitmap, ret = LY_EMEM, cleanup);

    /* fill the bitmap */
    ret = bits_str2bitmap(value, value_size, type_bits, val->bitmap, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate and fill the bit item array */
    LY_ARRAY_CREATE_GOTO(ctx, val->items, LY_ARRAY_COUNT(type_bits->bits), ret, cleanup);
    bits_bitmap2items(val->bitmap, type_bits, val->items);

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value_size ? value : "", value_size, &storage->_canonical);
            LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
        }
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_bits(ctx, storage);
    }
    return ret;
}

static LY_ERR
lyplg_type_compare_bits(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_bits *v1, *v2;
    uint32_t bitmap_size;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    bitmap_size = LYPLG_BITS2BYTES(lyplg_type_lyb_size_bits(val1->realtype));

    if (memcmp(v1->bitmap, v2->bitmap, bitmap_size)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

static int
lyplg_type_sort_bits(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_binary *v1, *v2;
    uint32_t bitmap_size;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    bitmap_size = LYPLG_BITS2BYTES(lyplg_type_lyb_size_bits(val1->realtype));

    return memcmp(v1->data, v2->data, bitmap_size);
}

static const void *
lyplg_type_print_bits(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint32_t *value_len_bits)
{
    struct lyd_value_bits *val;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len_bits) {
            *value_len_bits = lyplg_type_lyb_size_bits(value->realtype);
        }
        return val->bitmap;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        /* get the canonical value */
        if (bits_items2canon(val->items, &ret)) {
            return NULL;
        }

        /* store it */
        if (lydict_insert_zc(ctx, ret, (const char **)&value->_canonical)) {
            LOGMEM(ctx);
            return NULL;
        }
    }

    /* use the cached canonical value */
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len_bits) {
        *value_len_bits = strlen(value->_canonical) * 8;
    }
    return value->_canonical;
}

static LY_ERR
lyplg_type_dup_bits(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_value_bits *orig_val, *dup_val;
    uint32_t bitmap_size;

    memset(dup, 0, sizeof *dup);

    bitmap_size = LYPLG_BITS2BYTES(lyplg_type_lyb_size_bits(original->realtype));

    /* optional canonical value */
    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    /* allocate value */
    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, ret = LY_EMEM, error);

    LYD_VALUE_GET(original, orig_val);

    /* duplicate bitmap */
    dup_val->bitmap = malloc(bitmap_size);
    LY_CHECK_ERR_GOTO(!dup_val->bitmap, ret = LY_EMEM, error);
    memcpy(dup_val->bitmap, orig_val->bitmap, bitmap_size);

    /* duplicate bit item pointers */
    LY_ARRAY_CREATE_GOTO(ctx, dup_val->items, LY_ARRAY_COUNT(orig_val->items), ret, error);
    LY_ARRAY_FOR(orig_val->items, u) {
        LY_ARRAY_INCREMENT(dup_val->items);
        dup_val->items[u] = orig_val->items[u];
    }

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_bits(ctx, dup);
    return ret;
}

static void
lyplg_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_bits *val;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
    LYD_VALUE_GET(value, val);
    if (val) {
        free(val->bitmap);
        LY_ARRAY_FREE(val->items);
        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for bits type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_bits[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_BITS_STR,

        .plugin.id = "ly2 bits",
        .plugin.lyb_size = lyplg_type_lyb_size_bits,
        .plugin.store = lyplg_type_store_bits,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_bits,
        .plugin.sort = lyplg_type_sort_bits,
        .plugin.print = lyplg_type_print_bits,
        .plugin.duplicate = lyplg_type_dup_bits,
        .plugin.free = lyplg_type_free_bits,
    },
    {0}
};
