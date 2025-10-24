/**
 * @file printer_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB printer for libyang data structure
 *
 * Copyright (c) 2020 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "lyb.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "context.h"
#include "hash_table.h"
#include "log.h"
#include "ly_common.h"
#include "out.h"
#include "out_internal.h"
#include "parser_internal.h"
#include "plugins_exts/metadata.h"
#include "plugins_internal.h"
#include "printer_data.h"
#include "printer_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"

static LY_ERR lyb_print_siblings(const struct lyd_node *node, ly_bool is_root, struct lyd_lyb_ctx *lybctx);

/**
 * @brief Free a LYB data printer context.
 *
 * @param[in] lydctx Printer context to free.
 */
static void
lyb_print_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_lyb_ctx *ctx = (struct lyd_lyb_ctx *)lydctx;
    LY_ARRAY_COUNT_TYPE u;

    if (!ctx) {
        return;
    }

    lyd_ctx_free(lydctx);

    LY_ARRAY_FOR(ctx->print_ctx->sib_hts, u) {
        lyht_free(ctx->print_ctx->sib_hts[u].ht, NULL);
    }
    LY_ARRAY_FREE(ctx->print_ctx->sib_hts);

    free(ctx->print_ctx);
    free(ctx);
}

/**
 * @brief Hash table equal callback for checking hash equality only.
 *
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lyb_hash_equal_cb(void *UNUSED(val1_p), void *UNUSED(val2_p), ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    /* for this purpose, if hash matches, the value does also, we do not want 2 values to have the same hash */
    return 1;
}

/**
 * @brief Hash table equal callback for checking value pointer equality only.
 *
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lyb_ptr_equal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct lysc_node *val1 = *(struct lysc_node **)val1_p;
    struct lysc_node *val2 = *(struct lysc_node **)val2_p;

    if (val1 == val2) {
        return 1;
    }
    return 0;
}

/**
 * @brief Hash table equal callback for checking collisions.
 *
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lyb_col_equal_cb(void *val1_p, void *val2_p, ly_bool mod, void *cb_data)
{
    /* for first value check use lyb_ptr_equal_cb, for collisions lyb_hash_equal_cb */
    return mod ? lyb_ptr_equal_cb(val1_p, val2_p, mod, cb_data) : lyb_hash_equal_cb(val1_p, val2_p, mod, cb_data);
}

/**
 * @brief Check that sibling collision hash is safe to insert into hash table.
 *
 * @param[in] ht Hash table.
 * @param[in] sibling Hashed sibling.
 * @param[in] ht_col_id Sibling hash collision ID.
 * @param[in] compare_col_id Last collision ID to compare with.
 * @return LY_SUCCESS when the whole hash sequence does not collide,
 * @return LY_EEXIST when the whole hash sequence sollides.
 */
static LY_ERR
lyb_hash_sequence_check(struct ly_ht *ht, struct lysc_node *sibling, LYB_HASH ht_col_id, LYB_HASH compare_col_id)
{
    struct lysc_node **col_node;

    /* get the first node inserted with last hash col ID ht_col_id */
    if (lyht_find(ht, &sibling, lyb_get_hash(sibling, ht_col_id), (void **)&col_node)) {
        /* there is none. valid situation */
        return LY_SUCCESS;
    }

    do {
        int64_t j;

        for (j = (int64_t)compare_col_id; j > -1; --j) {
            if (lyb_get_hash(sibling, j) != lyb_get_hash(*col_node, j)) {
                /* one non-colliding hash */
                break;
            }
        }
        if (j == -1) {
            /* all whole hash sequences of nodes inserted with last hash col ID compare_col_id collide */
            return LY_EEXIST;
        }

        /* get next node inserted with last hash col ID ht_col_id */
    } while (!lyht_find_next_with_collision_cb(ht, col_node, lyb_get_hash(*col_node, ht_col_id), lyb_col_equal_cb,
            (void **)&col_node));

    return LY_SUCCESS;
}

/**
 * @brief Hash all the siblings and add them also into a separate hash table.
 *
 * @param[in] sibling Any sibling in all the siblings on one level.
 * @param[out] ht_p Created hash table.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_hash_siblings(struct lysc_node *sibling, struct ly_ht **ht_p)
{
    struct ly_ht *ht;
    const struct lysc_node *parent;
    const struct lys_module *mod;
    LYB_HASH i;
    uint32_t getnext_opts;

    ht = lyht_new(1, sizeof(struct lysc_node *), lyb_hash_equal_cb, NULL, 1);
    LY_CHECK_ERR_RET(!ht, LOGMEM(sibling->module->ctx), LY_EMEM);

    getnext_opts = 0;
    if (sibling->flags & LYS_IS_OUTPUT) {
        getnext_opts = LYS_GETNEXT_OUTPUT;
    }

    parent = lysc_data_parent(sibling);
    mod = sibling->module;

    sibling = NULL;
    while ((sibling = (struct lysc_node *)lys_getnext(sibling, parent, mod->compiled, getnext_opts))) {
        /* find the first non-colliding hash (or specifically non-colliding hash sequence) */
        for (i = 0; i < LYB_HASH_BITS; ++i) {
            /* check that we are not colliding with nodes inserted with a lower collision ID than ours */
            int64_t j;

            for (j = (int64_t)i - 1; j > -1; --j) {
                if (lyb_hash_sequence_check(ht, sibling, (LYB_HASH)j, i)) {
                    break;
                }
            }
            if (j > -1) {
                /* some check failed, we must use a higher collision ID */
                continue;
            }

            /* try to insert node with the current collision ID */
            if (!lyht_insert_with_resize_cb(ht, &sibling, lyb_get_hash(sibling, i), lyb_ptr_equal_cb, NULL)) {
                /* success, no collision */
                break;
            }

            /* make sure we really cannot insert it with this hash col ID (meaning the whole hash sequence is colliding) */
            if (i && !lyb_hash_sequence_check(ht, sibling, i, i)) {
                /* it can be inserted after all, even though there is already a node with the same last collision ID */
                lyht_set_cb(ht, lyb_ptr_equal_cb);
                if (lyht_insert(ht, &sibling, lyb_get_hash(sibling, i), NULL)) {
                    LOGINT(sibling->module->ctx);
                    lyht_set_cb(ht, lyb_hash_equal_cb);
                    lyht_free(ht, NULL);
                    return LY_EINT;
                }
                lyht_set_cb(ht, lyb_hash_equal_cb);
                break;
            }
            /* there is still another colliding schema node with the same hash sequence, try higher collision ID */
        }

        if (i == LYB_HASH_BITS) {
            /* wow */
            LOGINT(sibling->module->ctx);
            lyht_free(ht, NULL);
            return LY_EINT;
        }
    }

    /* change val equal callback so that the HT is usable for finding value hashes */
    lyht_set_cb(ht, lyb_ptr_equal_cb);

    *ht_p = ht;
    return LY_SUCCESS;
}

/**
 * @brief Find node hash in a hash table.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] node Node to find.
 * @param[out] hash_p First non-colliding hash found.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_hash_find(struct ly_ht *ht, struct lysc_node *node, LYB_HASH *hash_p)
{
    LYB_HASH hash;
    uint32_t i;

    for (i = 0; i < LYB_HASH_BITS; ++i) {
        hash = lyb_get_hash(node, i);
        if (!hash) {
            LOGINT_RET(node->module->ctx);
        }

        if (!lyht_find(ht, &node, hash, NULL)) {
            /* success, no collision */
            break;
        }
    }
    /* cannot happen, we already calculated the hash */
    if (i == LYB_HASH_BITS) {
        LOGINT_RET(node->module->ctx);
    }

    *hash_p = hash;
    return LY_SUCCESS;
}

/**
 * @brief Store remaining bits in the printer ctx bit buffer.
 *
 * @param[in] lybctx Printer LYB context.
 * @param[in] buf Source buffer.
 * @param[in] count_bits Number of bits to write from @p buf.
 * @param[in] count_bits_remainder Number of last bits in @p buf to store in the buffer.
 */
static void
lyb_write_buffer_store(struct lylyb_print_ctx *lybctx, const void *buf, uint64_t count_bits,
        uint8_t count_bits_remainder)
{
    uint8_t *byte_p, byte, byte_bits, byte_bits_written;

    assert(!lybctx->buf_bits);

    /* get the (first) byte with the remaining bits */
    byte_p = &((uint8_t *)buf)[(count_bits - count_bits_remainder) / 8];

    /* move the byte so it is using rightmost bits */
    byte_bits_written = (count_bits - count_bits_remainder) % 8;
    byte_bits = 8 - byte_bits_written;
    byte = byte_p[0];
    byte >>= byte_bits_written;

    /* truncate invalid trailing bits */
    if (byte_bits > count_bits_remainder) {
        byte_bits = count_bits_remainder;
    }

    /* store (first) set of remaining bits */
    lybctx->buf = byte;
    lybctx->buf_bits = byte_bits;

    if (byte_bits < count_bits_remainder) {
        /* there are remainder bits in the next byte as well, store those */
        byte_bits = count_bits_remainder - byte_bits;
        byte = byte_p[1];
        byte <<= lybctx->buf_bits;

        lybctx->buf |= byte & lyb_left_bit_mask(8 - lybctx->buf_bits);
        lybctx->buf_bits += byte_bits;
    }
}

/**
 * @brief Write data to the output.
 *
 * @param[in] buf Source buffer.
 * @param[in] count_bits Number of bits to write from @p buf.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write(const void *buf, uint64_t count_bits, struct lylyb_print_ctx *lybctx)
{
    LY_ERR r;
    uint64_t count_bytes;
    uint8_t count_bits_remainder;
    void *buf2;

    if (!count_bits) {
        return LY_SUCCESS;
    }

    if (lybctx->buf_bits + count_bits < 8) {
        /* only buffering additional bits, append them (from left) */
        lybctx->buf |= ((*(uint8_t *)buf) << lybctx->buf_bits) & lyb_left_bit_mask(8 - lybctx->buf_bits);
        lybctx->buf_bits += count_bits;
        return LY_SUCCESS;
    }

    /* prepare local counts */
    count_bytes = count_bits / 8;
    count_bits_remainder = count_bits % 8;

    if (lybctx->buf_bits) {
        /* adjust the bytes to write and the remainder */
        count_bytes += (lybctx->buf_bits + count_bits_remainder) / 8;
        count_bits_remainder = (lybctx->buf_bits + count_bits_remainder) % 8;

        /* make a copy of the buffer */
        buf2 = malloc(count_bytes);
        LY_CHECK_ERR_RET(!buf2, LOGMEM(lybctx->ctx), LY_EMEM);
        memcpy(buf2, buf, count_bytes);

        /* prepend the buffered bits */
        lyb_prepend_bits(buf2, count_bytes, lybctx->buf, lybctx->buf_bits);

        /* write this buf2 */
        r = ly_write_(lybctx->out, buf2, count_bytes);
        free(buf2);
        LY_CHECK_RET(r);

        /* buffered bits spent */
        lybctx->buf_bits = 0;
        lybctx->buf = 0;
    } else {
        /* write full bytes from buf */
        LY_CHECK_RET(ly_write_(lybctx->out, buf, count_bytes));
    }

    if (count_bits_remainder) {
        /* store the remaining bits in the bit buffer */
        lyb_write_buffer_store(lybctx, buf, count_bits, count_bits_remainder);
    }

    return LY_SUCCESS;
}

/**
 * @brief Flush unwritten bits to the output.
 *
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_flush(struct lylyb_print_ctx *lybctx)
{
    if (!lybctx->buf_bits) {
        return LY_SUCCESS;
    }

    /* zero unused bits */
    lybctx->buf &= lyb_right_bit_mask(lybctx->buf_bits);

    /* write the last data byte */
    lybctx->buf_bits = 0;
    return ly_write_(lybctx->out, (char *)&lybctx->buf, 1);
}

/**
 * @brief Write a count.
 *
 * @param[in] count Count to write.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_count(uint32_t count, struct lylyb_print_ctx *lybctx)
{
    uint8_t prefix_b, num_b;
    uint32_t buf;

    /* prepare prefix in buf (in reverse, read bit-by-bit), set prefix length and number length in bits */
    if (count == 0) {
        /* 0 */
        buf = 0;
        prefix_b = 0;
        num_b = 1;
    } else if (count < 16) {
        /* 10 */
        buf = 0x1;
        prefix_b = 2;
        num_b = 4;
    } else if (count < 32) {
        /* 110 */
        buf = 0x3;
        prefix_b = 3;
        num_b = 5;
    } else if (count < 128) {
        /* 1110 */
        buf = 0x7;
        prefix_b = 4;
        num_b = 7;
    } else if (count < 2048) {
        /* 11110 */
        buf = 0xF;
        prefix_b = 5;
        num_b = 11;
    } else if (count < 67108864) {
        /* 111110 */
        buf = 0x1F;
        prefix_b = 6;
        num_b = 26;
    } else {
        LOGERR(lybctx->ctx, LY_EINT, "Cannot print count %" PRIu32 ", largest supported number is 67 108 863.", count);
        return LY_EINT;
    }

    /* correct byte order */
    count = htole32(count);

    /* copy count to buf */
    buf |= count << prefix_b;

    return lyb_write(&buf, prefix_b + num_b, lybctx);
}

/**
 * @brief Write a size in bits or bytes.
 *
 * @param[in] size Size to write.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_size(uint32_t size, struct lylyb_print_ctx *lybctx)
{
    uint8_t prefix_b, num_b;
    uint32_t buf;

    /* prepare prefix in buf (in reverse, read bit-by-bit), set prefix length and number length in bits */
    if (size < 16) {
        /* prefix 0, encoded on 5 b */
        buf = 0x0;
        prefix_b = 1;
        num_b = 4;
    } else if (size < 64) {
        /* prefix 10, encoded on 8 b */
        buf = 0x1;
        prefix_b = 2;
        num_b = 6;
    } else if (size < 4096) {
        /* prefix 110, encoded on 15 b */
        buf = 0x3;
        prefix_b = 3;
        num_b = 12;
    } else {
        /* prefix 1110, encoded on 36 b */
        buf = 0x7;
        prefix_b = 4;
        num_b = 32;
    }

    /* correct byte order */
    size = htole32(size);

    /* copy size to buf */
    buf |= size << prefix_b;

    return lyb_write(&buf, prefix_b + num_b, lybctx);
}

/**
 * @brief Write a string.
 *
 * @param[in] str String to write.
 * @param[in] str_len Length of @p str in bytes.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_string(const char *str, uint32_t str_len, struct lylyb_print_ctx *lybctx)
{
    if (!str) {
        str = "";
        LY_CHECK_ERR_RET(str_len, LOGINT(lybctx->ctx), LY_EINT);
    }

    if (!str_len) {
        str_len = strlen(str);
    }

    /* print the string length in bytes */
    LY_CHECK_RET(lyb_write_size(str_len, lybctx));

    if (str_len) {
        /* print the string */
        LY_CHECK_RET(lyb_write(str, str_len * 8, lybctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print YANG module info as seperate module name and its revision.
 *
 * @param[in] mod Module to print.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_module_idx(const struct lys_module *mod, struct lylyb_print_ctx *lybctx)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lys_module *m;
    uint32_t idx = 0;

    /* learn the index of the module */
    while ((m = ly_ctx_get_module_iter(mod->ctx, &idx))) {
        if (m == mod) {
            break;
        }
    }
    assert(m);

    /* adjust the index */
    --idx;

    /* write module index */
    LY_CHECK_GOTO(rc = lyb_write_count(idx, lybctx), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Print YANG module info as only the module index in the context.
 *
 * @param[in] mod Module to print.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_module(const struct lys_module *mod, struct lylyb_print_ctx *lybctx)
{
    LY_ERR rc = LY_SUCCESS;
    uint16_t revision;
    int r;

    /* module name length and module name */
    LY_CHECK_GOTO(rc = lyb_write_string(mod->name, 0, lybctx), cleanup);

    /* module revision as YYYY YYYM MMMD DDDD (2B):
     * Y - max 128 (offset from 2000)
     * M - max 16
     * D - max 32 */
    revision = 0;
    if (mod->revision) {
        r = atoi(mod->revision);
        r -= LYB_REV_YEAR_OFFSET;
        r <<= LYB_REV_YEAR_SHIFT;

        revision |= r;

        r = atoi(mod->revision + ly_strlen_const("YYYY-"));
        r <<= LYB_REV_MONTH_SHIFT;

        revision |= r;

        r = atoi(mod->revision + ly_strlen_const("YYYY-MM-"));

        revision |= r;
    }
    LY_CHECK_GOTO(rc = lyb_write(&revision, 2 * 8, lybctx), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Print LYB header.
 *
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_header(struct lylyb_print_ctx *lybctx)
{
    uint8_t byte = 0;
    uint32_t hash;

    /* version */
    byte = LYB_HEADER_VERSION_NUM;
    LY_CHECK_RET(lyb_write(&byte, LYB_HEADER_VERSION_BITS, lybctx));

    /* hash algorithm */
    byte = LYB_HEADER_HASH_ALG_NUM;
    LY_CHECK_RET(lyb_write(&byte, LYB_HEADER_HASH_ALG_BITS, lybctx));

    /* shrink */
    byte = (lybctx->shrink ? 1 : 0);
    LY_CHECK_RET(lyb_write(&byte, LYB_HEADER_SHRINK_FLAG_BITS, lybctx));

    /* context hash (is truncated), if not printing empty data */
    if (lybctx->ctx) {
        hash = lyb_truncate_hash_nonzero(ly_ctx_get_modules_hash(lybctx->ctx), LYB_HEADER_CTX_HASH_BITS);
    } else {
        hash = 0;
    }
    LY_CHECK_RET(lyb_write(&hash, LYB_HEADER_CTX_HASH_BITS, lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print prefix data.
 *
 * @param[in] format Value prefix format.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_prefix_data(LY_VALUE_FORMAT format, const void *prefix_data, struct lylyb_print_ctx *lybctx)
{
    const struct ly_set *set;
    const struct lyxml_ns *ns;
    uint32_t i;

    switch (format) {
    case LY_VALUE_XML:
        set = prefix_data;
        if (!set) {
            /* no prefix data */
            i = 0;
            LY_CHECK_RET(lyb_write_count(i, lybctx));
            break;
        }
        if (set->count > UINT8_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "Maximum supported number of prefixes is %u.", UINT8_MAX);
            return LY_EINT;
        }

        /* write number of prefixes */
        LY_CHECK_RET(lyb_write_count(set->count, lybctx));

        /* write all the prefixes */
        for (i = 0; i < set->count; ++i) {
            ns = set->objs[i];

            /* prefix */
            LY_CHECK_RET(lyb_write_string(ns->prefix, 0, lybctx));

            /* namespace */
            LY_CHECK_RET(lyb_write_string(ns->uri, 0, lybctx));
        }
        break;
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        /* nothing to print */
        break;
    default:
        LOGINT_RET(lybctx->ctx);
    }

    return LY_SUCCESS;
}

/**
 * @brief Print a value.
 *
 * @param[in] ctx Context to use for printing @p value.
 * @param[in] value Value to print.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_value(const struct ly_ctx *ctx, const struct lyd_value *value, struct lylyb_print_ctx *lybctx)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool dynamic = 0;
    void *val;
    uint32_t val_size_bits = 0;
    enum lyplg_lyb_size_type size_type;
    uint32_t fixed_size_bits;
    lyplg_type_print_clb print;
    struct lyplg_type *type_plg;

    assert(value->realtype && value->realtype->plugin_ref);
    type_plg = LYSC_GET_TYPE_PLG(value->realtype->plugin_ref);

    /* get size of LYB data to print */
    type_plg->lyb_size(value->realtype, &size_type, &fixed_size_bits);

    /* get value and also print its length only if size is not fixed */
    print = type_plg->print;
    if (size_type == LYPLG_LYB_SIZE_FIXED_BITS) {
        /* fixed-length data */

        /* get value from plugin */
        val = (void *)print(ctx, value, LY_VALUE_LYB, NULL, &dynamic, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* use the returned length */
        val_size_bits = fixed_size_bits;
    } else {
        /* variable-length data */

        /* get value and its length from plugin */
        val = (void *)print(ctx, value, LY_VALUE_LYB, NULL, &dynamic, &val_size_bits);
        LY_CHECK_ERR_GOTO(!val, ret = LY_EINT, cleanup);

        /* print the length of the data in bytes or bits */
        if (size_type == LYPLG_LYB_SIZE_VARIABLE_BYTES) {
            assert(!(val_size_bits % 8));
            ret = lyb_write_size(val_size_bits / 8, lybctx);
        } else {
            ret = lyb_write_size(val_size_bits, lybctx);
        }
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* print value */
    if (val_size_bits > 0) {
        ret = lyb_write(val, val_size_bits, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (dynamic) {
        free(val);
    }

    return ret;
}

/**
 * @brief Print YANG node metadata.
 *
 * @param[in] node Data node whose metadata to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_metadata(const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    uint32_t count = 0;
    const struct lys_module *df_mod = NULL;
    struct lyd_meta *iter;

    /* with-defaults */
    if (node->schema->nodetype & LYD_NODE_TERM) {
        if (((node->flags & LYD_DEFAULT) && (lybctx->print_options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
                ((lybctx->print_options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node))) {
            /* we have implicit OR explicit default node, print attribute only if context include with-defaults schema */
            if (ly_ctx_get_module_latest(LYD_CTX(node), "ietf-netconf-with-defaults")) {
                df_mod = ly_ctx_get_module_latest(LYD_CTX(node), "default");
            }
        }
    }

    /* count metadata */
    if (df_mod) {
        ++count;
    }
    LY_LIST_FOR(node->meta, iter) {
        if (!lyd_metadata_should_print(iter)) {
            continue;
        }
        if (count == LYB_METADATA_END - 1) {
            LOGERR(lybctx->print_ctx->ctx, LY_EINT, "Maximum supported number of data node metadata is %" PRIu8 ".",
                    LYB_METADATA_END - 1);
            return LY_EINT;
        }
        ++count;
    }

    /* write the number of metadata */
    LY_CHECK_RET(lyb_write_count(count, lybctx->print_ctx));

    if (df_mod) {
        /* write the "default" metadata */
        LY_CHECK_RET(lyb_print_module_idx(df_mod, lybctx->print_ctx));
        LY_CHECK_RET(lyb_write_string("default", 0, lybctx->print_ctx));
        LY_CHECK_RET(lyb_write_string("true", 0, lybctx->print_ctx));
    }

    /* write all the node metadata */
    LY_LIST_FOR(node->meta, iter) {
        if (!lyd_metadata_should_print(iter)) {
            continue;
        }

        /* module */
        LY_CHECK_RET(lyb_print_module_idx(iter->annotation->module, lybctx->print_ctx));

        /* annotation name with length */
        LY_CHECK_RET(lyb_write_string(iter->name, 0, lybctx->print_ctx));

        /* metadata value */
        LY_CHECK_RET(lyb_print_value(LYD_CTX(node), &iter->value, lybctx->print_ctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print opaque node attributes.
 *
 * @param[in] node Opaque node whose attributes to print.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_attributes(const struct lyd_node_opaq *node, struct lylyb_print_ctx *lybctx)
{
    uint32_t count = 0;
    struct lyd_attr *iter;
    uint8_t format;

    for (iter = node->attr; iter; iter = iter->next) {
        if (count == UINT32_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "Maximum supported number of data node attributes is %" PRIu32 ".", UINT32_MAX);
            return LY_EINT;
        }
        ++count;
    }

    /* write number of attributes */
    LY_CHECK_RET(lyb_write_count(count, lybctx));

    /* write all the attributes */
    LY_LIST_FOR(node->attr, iter) {
        /* prefix */
        LY_CHECK_RET(lyb_write_string(iter->name.prefix, 0, lybctx));

        /* namespace */
        LY_CHECK_RET(lyb_write_string(iter->name.module_name, 0, lybctx));

        /* name */
        LY_CHECK_RET(lyb_write_string(iter->name.name, 0, lybctx));

        /* format */
        if (iter->format == LY_VALUE_XML) {
            format = LYB_OPAQ_FORMAT_XML;
            LY_CHECK_RET(lyb_write(&format, LYB_OPAQ_FORMAT_BITS, lybctx));
        } else if (iter->format == LY_VALUE_JSON) {
            format = LYB_OPAQ_FORMAT_JSON;
            LY_CHECK_RET(lyb_write(&format, LYB_OPAQ_FORMAT_BITS, lybctx));
        } else {
            LOGERR(lybctx->ctx, LY_EINT, "Unexpected opaque attribute format.");
            return LY_EINT;
        }

        /* value prefixes */
        LY_CHECK_RET(lyb_print_prefix_data(iter->format, iter->val_prefix_data, lybctx));

        /* value */
        LY_CHECK_RET(lyb_write_string(iter->value, 0, lybctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print schema node hash.
 *
 * @param[in] schema Schema node whose hash to print.
 * @param[in,out] sibling_ht Cached hash table for these siblings, created if NULL.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_schema_hash(struct lysc_node *schema, struct ly_ht **sibling_ht, struct lylyb_print_ctx *lybctx)
{
    LY_ARRAY_COUNT_TYPE u;
    uint32_t i;
    LYB_HASH hash;
    struct lyd_lyb_sib_ht *sib_ht;
    struct lysc_node *first_sibling;

    if (!schema) {
        /* opaque node, write empty hash */
        hash = 0;
        LY_CHECK_RET(lyb_write(&hash, sizeof hash * 8, lybctx));
        return LY_SUCCESS;
    }

    /* create whole sibling HT if not already created and saved */
    if (!*sibling_ht) {
        /* get first schema data sibling */
        first_sibling = (struct lysc_node *)lys_getnext(NULL, lysc_data_parent(schema), schema->module->compiled,
                (schema->flags & LYS_IS_OUTPUT) ? LYS_GETNEXT_OUTPUT : 0);
        LY_ARRAY_FOR(lybctx->sib_hts, u) {
            if (lybctx->sib_hts[u].first_sibling == first_sibling) {
                /* we have already created a hash table for these siblings */
                *sibling_ht = lybctx->sib_hts[u].ht;
                break;
            }
        }

        if (!*sibling_ht) {
            /* we must create sibling hash table */
            LY_CHECK_RET(lyb_hash_siblings(first_sibling, sibling_ht));

            /* and save it */
            LY_ARRAY_NEW_RET(lybctx->ctx, lybctx->sib_hts, sib_ht, LY_EMEM);

            sib_ht->first_sibling = first_sibling;
            sib_ht->ht = *sibling_ht;
        }
    }

    /* get our hash */
    LY_CHECK_RET(lyb_hash_find(*sibling_ht, schema, &hash));

    /* write the hash */
    LY_CHECK_RET(lyb_write(&hash, sizeof hash * 8, lybctx));

    if (hash & LYB_HASH_COLLISION_ID) {
        /* no collision for this hash, we are done */
        return LY_SUCCESS;
    }

    /* written hash was a collision, write also all the preceding hashes */
    for (i = 0; !(hash & (LYB_HASH_COLLISION_ID >> i)); ++i) {}

    for ( ; i; --i) {
        hash = lyb_get_hash(schema, i - 1);
        if (!hash) {
            return LY_EINT;
        }
        assert(hash & (LYB_HASH_COLLISION_ID >> (i - 1)));

        LY_CHECK_RET(lyb_write(&hash, sizeof hash * 8, lybctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print header for non-opaq node.
 *
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_header(const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    /* write any metadata */
    LY_CHECK_RET(lyb_print_metadata(node, lybctx));

    if (!lybctx->print_ctx->shrink) {
        /* write node flags, fixed bits */
        LY_CHECK_RET(lyb_write(&node->flags, LYB_DATA_NODE_FLAG_BITS, lybctx->print_ctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print LYB node type.
 *
 * @param[in] node Current data node to print.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_lyb_type(const struct lyd_node *node, struct lylyb_print_ctx *lybctx)
{
    enum lylyb_node_type lyb_type;

    if (!node) {
        /* nodes end */
        lyb_type = LYB_NODE_END;
    } else if (!node->schema) {
        /* opaque node laways printed as a child */
        lyb_type = LYB_NODE_CHILD;
    } else if (node->flags & LYD_EXT) {
        /* nested extension node */
        lyb_type = LYB_NODE_EXT;
    } else if (!lysc_data_parent(node->schema)) {
        /* top-level node */
        lyb_type = LYB_NODE_TOP;
    } else {
        /* standard child node */
        lyb_type = LYB_NODE_CHILD;
    }

    LY_CHECK_RET(lyb_write(&lyb_type, LYB_NODE_TYPE_BITS, lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print inner node.
 *
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_inner(const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    /* write necessary basic data */
    LY_CHECK_RET(lyb_print_node_header(node, lybctx));

    /* recursively write all the descendants */
    LY_CHECK_RET(lyb_print_siblings(lyd_child(node), 0, lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print opaque node and its descendants.
 *
 * @param[in] opaq Node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_opaq(const struct lyd_node_opaq *opaq, struct lyd_lyb_ctx *lybctx)
{
    uint8_t format;

    /* write attributes */
    LY_CHECK_RET(lyb_print_attributes(opaq, lybctx->print_ctx));

    /* prefix */
    LY_CHECK_RET(lyb_write_string(opaq->name.prefix, 0, lybctx->print_ctx));

    /* module reference */
    LY_CHECK_RET(lyb_write_string(opaq->name.module_name, 0, lybctx->print_ctx));

    /* name */
    LY_CHECK_RET(lyb_write_string(opaq->name.name, 0, lybctx->print_ctx));

    /* value */
    LY_CHECK_RET(lyb_write_string(opaq->value, 0, lybctx->print_ctx));

    /* format */
    if (opaq->format == LY_VALUE_XML) {
        format = LYB_OPAQ_FORMAT_XML;
        LY_CHECK_RET(lyb_write(&format, LYB_OPAQ_FORMAT_BITS, lybctx->print_ctx));
    } else if (opaq->format == LY_VALUE_JSON) {
        format = LYB_OPAQ_FORMAT_JSON;
        LY_CHECK_RET(lyb_write(&format, LYB_OPAQ_FORMAT_BITS, lybctx->print_ctx));
    } else {
        LOGERR(lybctx->parse_ctx->ctx, LY_EINT, "Unexpected opaque node format.");
        return LY_EINT;
    }

    /* value prefixes */
    LY_CHECK_RET(lyb_print_prefix_data(opaq->format, opaq->val_prefix_data, lybctx->print_ctx));

    /* recursively write all the descendants */
    LY_CHECK_RET(lyb_print_siblings(opaq->child, 0, lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print anydata or anyxml node.
 *
 * @param[in] anydata Node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_any(struct lyd_node_any *anydata, struct lyd_lyb_ctx *lybctx)
{
    LY_ERR rc = LY_SUCCESS;

    if ((anydata->schema->nodetype == LYS_ANYDATA) && (anydata->value_type != LYD_ANYDATA_DATATREE)) {
        LOGINT_RET(lybctx->print_ctx->ctx);
    }

    /* write necessary basic data */
    LY_CHECK_RET(lyb_print_node_header((struct lyd_node *)anydata, lybctx));

    /* write anydata value type */
    LY_CHECK_GOTO(rc = lyb_write_count(anydata->value_type, lybctx->print_ctx), cleanup);

    switch (anydata->value_type) {
    case LYD_ANYDATA_DATATREE:
        /* print LYB siblings */
        LY_CHECK_GOTO(rc = lyb_print_siblings(anydata->value.tree, 0, lybctx), cleanup);
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        /* string value */
        LY_CHECK_GOTO(rc = lyb_write_string(anydata->value.str, 0, lybctx->print_ctx), cleanup);
        break;
    default:
        LOGINT(lybctx->print_ctx->ctx);
        rc = LY_EINT;
        goto cleanup;
    }

cleanup:
    return rc;
}

/**
 * @brief Print leaf node.
 *
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_leaf(const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    /* write necessary basic data */
    LY_CHECK_RET(lyb_print_node_header(node, lybctx));

    /* write term value */
    LY_CHECK_RET(lyb_print_value(LYD_CTX(node), &((struct lyd_node_term *)node)->value, lybctx->print_ctx));

    return LY_SUCCESS;
}

/**
 * @brief Print all leaflist nodes which belong to same schema.
 *
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @param[out] printed_node Last node that was printed by this function.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_leaflist(const struct lyd_node *node, struct lyd_lyb_ctx *lybctx, const struct lyd_node **printed_node)
{
    const struct lysc_node *schema;

    schema = node->schema;

    /* write all the siblings */
    LY_LIST_FOR(node, node) {
        if (schema != node->schema) {
            /* all leaflist nodes were printed */
            break;
        }

        /* write leaf data */
        LY_CHECK_RET(lyb_print_node_leaf(node, lybctx));
        *printed_node = node;
    }

    /* no more instances */
    LY_CHECK_RET(lyb_write_count(LYB_METADATA_END_COUNT, lybctx->print_ctx));

    return LY_SUCCESS;
}

/**
 * @brief Print all list nodes which belong to same schema.
 *
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @param[out] printed_node Last node that was printed by this function.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_list(const struct lyd_node *node, struct lyd_lyb_ctx *lybctx, const struct lyd_node **printed_node)
{
    const struct lysc_node *schema;

    schema = node->schema;

    LY_LIST_FOR(node, node) {
        if (schema != node->schema) {
            /* all list nodes was printed */
            break;
        }

        /* write necessary basic data */
        LY_CHECK_RET(lyb_print_node_header(node, lybctx));

        /* recursively write all the descendants */
        LY_CHECK_RET(lyb_print_siblings(lyd_child(node), 0, lybctx));

        *printed_node = node;
    }

    /* no more instances */
    LY_CHECK_RET(lyb_write_count(LYB_METADATA_END_COUNT, lybctx->print_ctx));

    return LY_SUCCESS;
}

/**
 * @brief Print node.
 *
 * @param[in,out] printed_node Current data node to print. Sets to the last printed node.
 * @param[in,out] sibling_ht Cached hash table for these siblings, created if NULL.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node(const struct lyd_node **printed_node, struct ly_ht **sibling_ht, struct lyd_lyb_ctx *lybctx)
{
    const struct lyd_node *node = *printed_node;

    if (lybctx->print_ctx->shrink && !lyd_node_should_print(node, LYD_PRINT_WD_TRIM)) {
        /* not printing default nodes */
        return LY_SUCCESS;
    }

    /* write node type */
    LY_CHECK_RET(lyb_print_lyb_type(node, lybctx->print_ctx));

    /* write module info first */
    if (node->flags & LYD_EXT) {
        LY_CHECK_RET(lyb_print_module(node->schema->module, lybctx->print_ctx));
    } else if (node->schema && !lysc_data_parent(node->schema)) {
        LY_CHECK_RET(lyb_print_module_idx(node->schema->module, lybctx->print_ctx));
    }

    if (node->flags & LYD_EXT) {
        /* extension context which may not have hashes generated */
        if (!(LYD_CTX(node)->opts & LY_CTX_LYB_HASHES)) {
            LY_CHECK_RET(ly_ctx_set_options((struct ly_ctx *)LYD_CTX(node), LY_CTX_LYB_HASHES));
        }

        /* write schema node name */
        LY_CHECK_RET(lyb_write_string(node->schema->name, 0, lybctx->print_ctx));
    } else {
        /* write schema hash */
        LY_CHECK_RET(lyb_print_schema_hash((struct lysc_node *)node->schema, sibling_ht, lybctx->print_ctx));
    }

    if (!node->schema) {
        LY_CHECK_RET(lyb_print_node_opaq((struct lyd_node_opaq *)node, lybctx));
    } else if (node->schema->nodetype == LYS_LEAFLIST) {
        LY_CHECK_RET(lyb_print_node_leaflist(node, lybctx, &node));
    } else if (node->schema->nodetype == LYS_LIST) {
        LY_CHECK_RET(lyb_print_node_list(node, lybctx, &node));
    } else if (node->schema->nodetype & LYD_NODE_ANY) {
        LY_CHECK_RET(lyb_print_node_any((struct lyd_node_any *)node, lybctx));
    } else if (node->schema->nodetype & LYD_NODE_INNER) {
        LY_CHECK_RET(lyb_print_node_inner(node, lybctx));
    } else {
        LY_CHECK_RET(lyb_print_node_leaf(node, lybctx));
    }

    *printed_node = node;

    return LY_SUCCESS;
}

/**
 * @brief Print siblings.
 *
 * @param[in] node Current data node to print.
 * @param[in] is_root Whether we are printing the root node(s).
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_siblings(const struct lyd_node *node, ly_bool is_root, struct lyd_lyb_ctx *lybctx)
{
    struct ly_ht *sibling_ht = NULL;
    const struct lys_module *prev_mod = NULL;

    /* write all the siblings */
    LY_LIST_FOR(node, node) {
        /* do not reuse top-level sibling hash tables from different modules */
        if (!node->schema || (!lysc_data_parent(node->schema) && (node->schema->module != prev_mod))) {
            sibling_ht = NULL;
            prev_mod = node->schema ? node->schema->module : NULL;
        }

        LY_CHECK_RET(lyb_print_node(&node, &sibling_ht, lybctx));

        if (is_root && !(lybctx->print_options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

    /* no more siblings */
    LY_CHECK_RET(lyb_print_lyb_type(NULL, lybctx->print_ctx));

    return LY_SUCCESS;
}

LY_ERR
lyb_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_lyb_ctx *lybctx;
    const struct ly_ctx *ctx = root ? LYD_CTX(root) : NULL;

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_GOTO(!lybctx, LOGMEM(ctx); rc = LY_EMEM, cleanup);
    lybctx->print_ctx = calloc(1, sizeof *lybctx->print_ctx);
    LY_CHECK_ERR_GOTO(!lybctx->print_ctx, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    lybctx->print_options = options;
    if (options & LYD_PRINT_SHRINK) {
        lybctx->print_ctx->shrink = 1;
    }

    if (root) {
        lybctx->print_ctx->ctx = ctx;
        assert(ctx->mod_hash);

        if (root->schema && lysc_data_parent(root->schema)) {
            LOGERR(ctx, LY_EINVAL, "LYB printer supports only printing top-level nodes.");
            rc = LY_EINVAL;
            goto cleanup;
        }

        if (!(ctx->opts & LY_CTX_LYB_HASHES)) {
            /* generate LYB hashes */
            LY_CHECK_GOTO(rc = ly_ctx_set_options((struct ly_ctx *)ctx, LY_CTX_LYB_HASHES), cleanup);
        }
    }
    lybctx->print_ctx->out = out;

    /* LYB header */
    LY_CHECK_GOTO(rc = lyb_print_header(lybctx->print_ctx), cleanup);

    /* all the top-level siblings, recursively */
    LY_CHECK_GOTO(rc = lyb_print_siblings(root, 1, lybctx), cleanup);

    /* flush any last remaining bits */
    LY_CHECK_GOTO(rc = lyb_write_flush(lybctx->print_ctx), cleanup);

cleanup:
    lyb_print_ctx_free((struct lyd_ctx *)lybctx);
    return rc;
}
