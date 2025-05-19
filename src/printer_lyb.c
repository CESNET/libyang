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

    LY_ARRAY_FOR(ctx->print_ctx.sib_hts, u) {
        lyht_free(ctx->print_ctx.sib_hts[u].ht, NULL);
    }
    LY_ARRAY_FREE(ctx->print_ctx.sib_hts);

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
 * @brief Write LYB data.
 *
 * @param[in] lybctx Printer LYB context.
 * @param[in] buf Source buffer.
 * @param[in] count Number of bytes to write.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write(struct lylyb_print_ctx *lybctx, const void *buf, size_t count)
{
    if (!count) {
        return LY_SUCCESS;
    }

    return ly_write_(lybctx->out, (char *)buf, count);
}

/**
 * @brief Write a number.
 *
 * @param[in] num Number to write.
 * @param[in] bytes Valid bytes of @p num, only those are written.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_number(uint64_t num, size_t bytes, struct lylyb_print_ctx *lybctx)
{
    /* correct byte order */
    num = htole64(num);

    return lyb_write(lybctx, (uint8_t *)&num, bytes);
}

/**
 * @brief Write a string.
 *
 * @param[in] str String to write.
 * @param[in] str_len Length of @p str.
 * @param[in] len_size Size of @p str_len in bytes.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_string(const char *str, size_t str_len, uint8_t len_size, struct lylyb_print_ctx *lybctx)
{
    ly_bool error;

    if (!str) {
        str = "";
        LY_CHECK_ERR_RET(str_len, LOGINT(lybctx->ctx), LY_EINT);
    }

    if (!str_len) {
        str_len = strlen(str);
    }

    switch (len_size) {
    case sizeof(uint8_t):
        error = str_len > UINT8_MAX;
        break;
    case sizeof(uint16_t):
        error = str_len > UINT16_MAX;
        break;
    case sizeof(uint32_t):
        error = str_len > UINT32_MAX;
        break;
    case sizeof(uint64_t):
        error = 0;
        break;
    default:
        error = 1;
    }
    if (error) {
        LOGINT(lybctx->ctx);
        return LY_EINT;
    }

    LY_CHECK_RET(lyb_write_number(str_len, len_size, lybctx));

    LY_CHECK_RET(lyb_write(lybctx, (const uint8_t *)str, str_len));

    return LY_SUCCESS;
}

/**
 * @brief Print YANG module info.
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
    LY_CHECK_GOTO(rc = lyb_write_string(mod->name, 0, sizeof(uint16_t), lybctx), cleanup);

    /* module revision as XXXX XXXX XXXX XXXX (2B) (year is offset from 2000)
     *                    YYYY YYYM MMMD DDDD */
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
    LY_CHECK_GOTO(rc = lyb_write_number(revision, sizeof revision, lybctx), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Print LYB magic number.
 *
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_magic_number(struct lylyb_print_ctx *lybctx)
{
    /* 'l', 'y', 'b' - 0x6c7962 */
    const uint8_t magic_number[] = {'l', 'y', 'b'};

    LY_CHECK_RET(lyb_write(lybctx, magic_number, 3));

    return LY_SUCCESS;
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

    /* version, hash algorithm (flags) */
    byte |= LYB_HEADER_VERSION_NUM;
    byte |= LYB_HEADER_HASH_ALG;

    LY_CHECK_RET(lyb_write(lybctx, &byte, sizeof byte));

    /* context hash, if not printing empty data */
    if (lybctx->ctx) {
        hash = ly_ctx_get_modules_hash(lybctx->ctx);
    } else {
        hash = 0;
    }
    LY_CHECK_RET(lyb_write(lybctx, &hash, sizeof hash));

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
            LY_CHECK_RET(lyb_write(lybctx, (uint8_t *)&i, 1));
            break;
        }
        if (set->count > UINT8_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "Maximum supported number of prefixes is %u.", UINT8_MAX);
            return LY_EINT;
        }

        /* write number of prefixes on 1 byte */
        LY_CHECK_RET(lyb_write_number(set->count, 1, lybctx));

        /* write all the prefixes */
        for (i = 0; i < set->count; ++i) {
            ns = set->objs[i];

            /* prefix */
            LY_CHECK_RET(lyb_write_string(ns->prefix, 0, sizeof(uint16_t), lybctx));

            /* namespace */
            LY_CHECK_RET(lyb_write_string(ns->uri, 0, sizeof(uint16_t), lybctx));
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
 * @brief Print term node.
 *
 * @param[in] term Node to print.
 * @param[in] lybctx Printer LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_term_value(struct lyd_node_term *term, struct lylyb_print_ctx *lybctx)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool dynamic = 0;
    void *value;
    size_t value_len = 0;
    int32_t lyb_data_len;
    lyplg_type_print_clb print;
    struct lyplg_type *type_plg;

    assert(term->value.realtype && term->value.realtype->plugin_ref &&
            LYSC_GET_TYPE_PLG(term->value.realtype->plugin_ref)->print && term->schema);

    type_plg = LYSC_GET_TYPE_PLG(term->value.realtype->plugin_ref);

    /* get length of LYB data to print */
    lyb_data_len = term->value.realtype->plugin->lyb_data_len;

    /* get value and also print its length only if size is not fixed */
    print = type_plg->print;
    if (lyb_data_len < 0) {
        /* variable-length data */

        /* get value and its length from plugin */
        value = (void *)print(term->schema->module->ctx, &term->value, LY_VALUE_LYB, NULL, &dynamic, &value_len);
        LY_CHECK_ERR_GOTO(!value, ret = LY_EINT, cleanup);

        if (value_len > UINT32_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "Maximum length of a LYB data value must not exceed %" PRIu32 ".", UINT32_MAX);
            ret = LY_EINT;
            goto cleanup;
        }

        /* print the length of the data as 64-bit unsigned integer */
        ret = lyb_write_number(value_len, sizeof(uint64_t), lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        /* fixed-length data */

        /* get value from plugin */
        value = (void *)print(term->schema->module->ctx, &term->value, LY_VALUE_LYB, NULL, &dynamic, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* copy the length from the compiled node */
        value_len = lyb_data_len;
    }

    /* print value */
    if (value_len > 0) {
        ret = lyb_write(lybctx, value, value_len);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (dynamic) {
        free(value);
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
    uint8_t count = 0;
    const struct lys_module *wd_mod = NULL;
    struct lyd_meta *iter;

    /* with-defaults */
    if (node->schema->nodetype & LYD_NODE_TERM) {
        if (((node->flags & LYD_DEFAULT) && (lybctx->print_options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
                ((lybctx->print_options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node))) {
            /* we have implicit OR explicit default node, print attribute only if context include with-defaults schema */
            wd_mod = ly_ctx_get_module_latest(node->schema->module->ctx, "ietf-netconf-with-defaults");
        }
    }

    /* count metadata */
    if (wd_mod) {
        ++count;
    }
    LY_LIST_FOR(node->meta, iter) {
        if (!lyd_metadata_should_print(iter)) {
            continue;
        }
        if (count == LYB_METADATA_END - 1) {
            LOGERR(lybctx->print_ctx.ctx, LY_EINT, "Maximum supported number of data node metadata is %" PRIu8 ".",
                    LYB_METADATA_END - 1);
            return LY_EINT;
        }
        ++count;
    }

    /* write number of metadata on 1 byte */
    LY_CHECK_RET(lyb_write(&lybctx->print_ctx, &count, 1));

    if (wd_mod) {
        /* write the "default" metadata */
        LY_CHECK_RET(lyb_print_module(wd_mod, &lybctx->print_ctx));
        LY_CHECK_RET(lyb_write_string("default", 0, sizeof(uint16_t), &lybctx->print_ctx));
        LY_CHECK_RET(lyb_write_string("true", 0, sizeof(uint16_t), &lybctx->print_ctx));
    }

    /* write all the node metadata */
    LY_LIST_FOR(node->meta, iter) {
        if (!lyd_metadata_should_print(iter)) {
            continue;
        }

        /* module */
        LY_CHECK_RET(lyb_print_module(iter->annotation->module, &lybctx->print_ctx));

        /* annotation name with length */
        LY_CHECK_RET(lyb_write_string(iter->name, 0, sizeof(uint16_t), &lybctx->print_ctx));

        /* metadata value */
        LY_CHECK_RET(lyb_write_string(lyd_get_meta_value(iter), 0, sizeof(uint64_t), &lybctx->print_ctx));
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
    uint8_t count = 0;
    struct lyd_attr *iter;

    for (iter = node->attr; iter; iter = iter->next) {
        if (count == UINT8_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "Maximum supported number of data node attributes is %u.", UINT8_MAX);
            return LY_EINT;
        }
        ++count;
    }

    /* write number of attributes on 1 byte */
    LY_CHECK_RET(lyb_write(lybctx, &count, 1));

    /* write all the attributes */
    LY_LIST_FOR(node->attr, iter) {
        /* prefix */
        LY_CHECK_RET(lyb_write_string(iter->name.prefix, 0, sizeof(uint16_t), lybctx));

        /* namespace */
        LY_CHECK_RET(lyb_write_string(iter->name.module_name, 0, sizeof(uint16_t), lybctx));

        /* name */
        LY_CHECK_RET(lyb_write_string(iter->name.name, 0, sizeof(uint16_t), lybctx));

        /* format */
        LY_CHECK_RET(lyb_write_number(iter->format, 1, lybctx));

        /* value prefixes */
        LY_CHECK_RET(lyb_print_prefix_data(iter->format, iter->val_prefix_data, lybctx));

        /* value */
        LY_CHECK_RET(lyb_write_string(iter->value, 0, sizeof(uint64_t), lybctx));
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
        LY_CHECK_RET(lyb_write(lybctx, &hash, sizeof hash));
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
    LY_CHECK_RET(lyb_write(lybctx, &hash, sizeof hash));

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

        LY_CHECK_RET(lyb_write(lybctx, &hash, sizeof hash));
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

    /* write node flags */
    LY_CHECK_RET(lyb_write_number(node->flags, sizeof node->flags, &lybctx->print_ctx));

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
        /* opaque node */
        lyb_type = LYB_NODE_OPAQ;
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

    LY_CHECK_RET(lyb_write_number(lyb_type, 1, lybctx));

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
    /* write attributes */
    LY_CHECK_RET(lyb_print_attributes(opaq, &lybctx->print_ctx));

    /* write node flags */
    LY_CHECK_RET(lyb_write_number(opaq->flags, sizeof opaq->flags, &lybctx->print_ctx));

    /* prefix */
    LY_CHECK_RET(lyb_write_string(opaq->name.prefix, 0, sizeof(uint16_t), &lybctx->print_ctx));

    /* module reference */
    LY_CHECK_RET(lyb_write_string(opaq->name.module_name, 0, sizeof(uint16_t), &lybctx->print_ctx));

    /* name */
    LY_CHECK_RET(lyb_write_string(opaq->name.name, 0, sizeof(uint16_t), &lybctx->print_ctx));

    /* value */
    LY_CHECK_RET(lyb_write_string(opaq->value, 0, sizeof(uint64_t), &lybctx->print_ctx));

    /* format */
    LY_CHECK_RET(lyb_write_number(opaq->format, 1, &lybctx->print_ctx));

    /* value prefixes */
    LY_CHECK_RET(lyb_print_prefix_data(opaq->format, opaq->val_prefix_data, &lybctx->print_ctx));

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
        LOGINT_RET(lybctx->print_ctx.ctx);
    }

    /* write necessary basic data */
    LY_CHECK_RET(lyb_print_node_header((struct lyd_node *)anydata, lybctx));

    /* first byte is type */
    LY_CHECK_GOTO(rc = lyb_write_number(anydata->value_type, sizeof anydata->value_type, &lybctx->print_ctx), cleanup);

    if (anydata->value_type == LYD_ANYDATA_DATATREE) {
        /* print LYB siblings */
        LY_CHECK_GOTO(rc = lyb_print_siblings(anydata->value.tree, 0, lybctx), cleanup);
    } else {
        /* string value */
        LY_CHECK_GOTO(rc = lyb_write_string(anydata->value.str, (size_t)strlen(anydata->value.str), sizeof(uint64_t),
                &lybctx->print_ctx), cleanup);
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
    LY_CHECK_RET(lyb_print_term_value((struct lyd_node_term *)node, &lybctx->print_ctx));

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
    uint8_t byte = LYB_METADATA_END;

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
    LY_CHECK_RET(lyb_write(&lybctx->print_ctx, &byte, sizeof byte));

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
    uint8_t byte = LYB_METADATA_END;

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
    LY_CHECK_RET(lyb_write(&lybctx->print_ctx, &byte, sizeof byte));

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

    /* write node type */
    LY_CHECK_RET(lyb_print_lyb_type(node, &lybctx->print_ctx));

    /* write module info first */
    if (node->schema && ((node->flags & LYD_EXT) || !lysc_data_parent(node->schema))) {
        LY_CHECK_RET(lyb_print_module(node->schema->module, &lybctx->print_ctx));
    }

    if (node->flags & LYD_EXT) {
        /* extension context which may not have hashes generated */
        if (!(LYD_CTX(node)->opts & LY_CTX_LYB_HASHES)) {
            LY_CHECK_RET(ly_ctx_set_options((struct ly_ctx *)LYD_CTX(node), LY_CTX_LYB_HASHES));
        }

        /* write schema node name */
        LY_CHECK_RET(lyb_write_string(node->schema->name, 0, sizeof(uint16_t), &lybctx->print_ctx));
    } else {
        /* write schema hash */
        LY_CHECK_RET(lyb_print_schema_hash((struct lysc_node *)node->schema, sibling_ht, &lybctx->print_ctx));
    }

    if (!node->schema) {
        LY_CHECK_RET(lyb_print_node_opaq((struct lyd_node_opaq *)node, lybctx));
    } else if (node->schema->nodetype & LYS_LEAFLIST) {
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
    LY_CHECK_RET(lyb_print_lyb_type(NULL, &lybctx->print_ctx));

    return LY_SUCCESS;
}

LY_ERR
lyb_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_lyb_ctx *lybctx;
    const struct ly_ctx *ctx = root ? LYD_CTX(root) : NULL;

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_GOTO(!lybctx, LOGMEM(ctx); ret = LY_EMEM, cleanup);

    lybctx->print_options = options;
    if (root) {
        lybctx->print_ctx.ctx = ctx;
        assert(ctx->mod_hash);

        if (root->schema && lysc_data_parent(root->schema)) {
            LOGERR(lybctx->print_ctx.ctx, LY_EINVAL, "LYB printer supports only printing top-level nodes.");
            ret = LY_EINVAL;
            goto cleanup;
        }

        if (!(ctx->opts & LY_CTX_LYB_HASHES)) {
            /* generate LYB hashes */
            LY_CHECK_GOTO(ret = ly_ctx_set_options((struct ly_ctx *)ctx, LY_CTX_LYB_HASHES), cleanup);
        }
    }
    lybctx->print_ctx.out = out;

    /* LYB magic number */
    LY_CHECK_GOTO(ret = lyb_print_magic_number(&lybctx->print_ctx), cleanup);

    /* LYB header */
    LY_CHECK_GOTO(ret = lyb_print_header(&lybctx->print_ctx), cleanup);

    /* all the top-level siblings, recursively */
    LY_CHECK_GOTO(ret = lyb_print_siblings(root, 1, lybctx), cleanup);

cleanup:
    lyb_print_ctx_free((struct lyd_ctx *)lybctx);
    return ret;
}
