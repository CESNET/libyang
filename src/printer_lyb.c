/**
 * @file printer_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB printer for libyang data structure
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "resolve.h"
#include "tree_internal.h"

/* writing function handles writing size information */
static int
lyb_write(struct lyout *out, const uint8_t *buf, size_t count, struct lyb_state *lybs)
{
    int ret, i, full_chunk_i;
    size_t r, to_write;

    assert(out && lybs);

    ret = 0;
    while (count) {
        /* check for full data chunks */
        to_write = count;
        full_chunk_i = -1;
        for (i = 0; i < lybs->used; ++i) {
            /* we want the innermost chunks resolved first, so replace previous full chunks */
            if (lybs->written[i] + to_write >= LYB_SIZE_MAX) {
                /* full chunk, do not write more than allowed */
                to_write = LYB_SIZE_MAX - lybs->written[i];
                full_chunk_i = i;
            }
        }

        r = ly_write(out, (char *)buf, to_write);
        if (r < to_write) {
            return -1;
        }

        for (i = 0; i < lybs->used; ++i) {
            /* increase all written counters */
            lybs->written[i] += r;
            assert(lybs->written[i] <= LYB_SIZE_MAX);
        }
        /* decrease count/buf */
        count -= r;
        buf += r;

        ret += r;

        if (full_chunk_i > -1) {
            /* write the chunk size */
            r = ly_write_skipped(out, lybs->position[full_chunk_i], (char *)&lybs->written[full_chunk_i], LYB_SIZE_BYTES);
            if (r < LYB_SIZE_BYTES) {
                return -1;
            }

            /* zero written */
            lybs->written[full_chunk_i] = 0;

            /* skip space for another chunk size */
            r = ly_write_skip(out, LYB_SIZE_BYTES, &lybs->position[full_chunk_i]);
            if (r < LYB_SIZE_BYTES) {
                return -1;
            }

            ret += r;
        }
    }

    return ret;
}

static int
lyb_write_stop_subtree(struct lyout *out, struct lyb_state *lybs)
{
    int r;

    /* write the chunk size */
    r = ly_write_skipped(out, lybs->position[lybs->used - 1], (char *)&lybs->written[lybs->used - 1], LYB_SIZE_BYTES);
    if (r < LYB_SIZE_BYTES) {
        return -1;
    }

    --lybs->used;
    return r;
}

static int
lyb_write_start_subtree(struct lyout *out, struct lyb_state *lybs)
{
    if (lybs->used == lybs->size) {
        lybs->size += LYB_STATE_STEP;
        lybs->written = ly_realloc(lybs->written, lybs->size * sizeof *lybs->written);
        lybs->position = ly_realloc(lybs->position, lybs->size * sizeof *lybs->position);
        LY_CHECK_ERR_RETURN(!lybs->written || !lybs->position, LOGMEM(NULL), -1);
    }

    ++lybs->used;
    lybs->written[lybs->used - 1] = 0;
    return ly_write_skip(out, LYB_SIZE_BYTES, &lybs->position[lybs->used - 1]);
}

static int
lyb_write_number(uint64_t num, uint64_t max_num, struct lyout *out, struct lyb_state *lybs)
{
    int max_bits, max_bytes, i, ret = 0;
    uint8_t byte;

    for (max_bits = 0; max_num; max_num >>= 1, ++max_bits);
    max_bytes = max_bits / 8 + (max_bits % 8 ? 1 : 0);

    for (i = 0; i < max_bytes; ++i) {
        byte = *(((uint8_t *)&num) + i);
        ret += lyb_write(out, &byte, 1, lybs);
    }

    return ret;
}

static int
lyb_write_string(const char *str, struct lyout *out, struct lyb_state *lybs)
{
    return lyb_write(out, (const uint8_t *)str, strlen(str), lybs);
}

static int
lyb_print_model(struct lyout *out, const struct lys_module *mod, struct lyb_state *lybs)
{
    int r, ret = 0;
    size_t len;
    uint16_t revision;

    /* model name length and model name */
    len = strlen(mod->name);
    if (len > UINT16_MAX) {
        LOGINT(mod->ctx);
        return -1;
    }
    ret += (r = lyb_write(out, (uint8_t *)&len, sizeof(uint16_t), lybs));
    if (r < 0) {
        return -1;
    }
    ret += (r = lyb_write(out, (uint8_t *)mod->name, len, lybs));
    if (r < 0) {
        return -1;
    }

    /* model revision as XXXX XXXX XXXX XXXX (2B) (year is offset from 2000)
     *                   YYYY YYYM MMMD DDDD */
    revision = 0;
    if (mod->rev_size) {
        r = atoi(mod->rev[0].date);
        r -= 2000;
        r <<= 9;

        revision |= r;

        r = atoi(mod->rev[0].date + 5);
        r <<= 5;

        revision |= r;

        r = atoi(mod->rev[0].date + 8);

        revision |= r;
    }

    ret += (r = lyb_write(out, (uint8_t *)&revision, sizeof revision, lybs));
    if (r < 0) {
        return -1;
    }

    return ret;
}

static int
is_added_model(const struct lys_module **models, size_t mod_count, const struct lys_module *mod)
{
    size_t i;

    for (i = 0; i < mod_count; ++i) {
        if (models[i] == mod) {
            return 1;
        }
    }

    return 0;
}

static void
add_model(const struct lys_module ***models, size_t *mod_count, const struct lys_module *mod)
{
    if (is_added_model(*models, *mod_count, mod)) {
        return;
    }

    *models = ly_realloc(*models, ++(*mod_count) * sizeof **models);
    (*models)[*mod_count - 1] = mod;
}

static int
lyb_print_data_models(struct lyout *out, const struct lyd_node *root, struct lyb_state *lybs)
{
    int ret = 0;
    const struct lys_module **models = NULL, *mod;
    const struct lyd_node *node;
    size_t mod_count = 0;
    uint32_t idx = 0, i;

    /* first, collect all data node modules */
    LY_TREE_FOR(root, node) {
        mod = lyd_node_module(node);
        add_model(&models, &mod_count, mod);
    }

    /* then add all models augmenting or deviating the used models */
    idx = ly_ctx_internal_modules_count(root->schema->module->ctx);
    while ((mod = ly_ctx_get_module_iter(root->schema->module->ctx, &idx))) {
        if (!mod->implemented) {
next_mod:
            continue;
        }

        for (i = 0; i < mod->deviation_size; ++i) {
            if (mod->deviation[i].orig_node && is_added_model(models, mod_count, lys_node_module(mod->deviation[i].orig_node))) {
                add_model(&models, &mod_count, mod);
                goto next_mod;
            }
        }
        for (i = 0; i < mod->augment_size; ++i) {
            if (is_added_model(models, mod_count, lys_node_module(mod->augment[i].target))) {
                add_model(&models, &mod_count, mod);
                goto next_mod;
            }
        }
    }

    /* now write module count on 2 bytes */
    ret += lyb_write(out, (uint8_t *)&mod_count, 2, lybs);

    /* and all the used models */
    for (i = 0; i < mod_count; ++i) {
        ret += lyb_print_model(out, models[i], lybs);
    }

    free(models);
    return ret;
}

static int
lyb_print_header(struct lyout *out, int options)
{
    int ret = 0;
    uint8_t byte = 0;

    /* TODO version, some other flags? */
    ret += ly_write(out, (char *)&byte, sizeof byte);

    return ret;
}

static int
lyb_print_anydata(struct lyd_node_anydata *anydata, struct lyout *out, struct lyb_state *lybs)
{
    int ret = 0;
    char *buf;
    LYD_ANYDATA_VALUETYPE type;

    switch (anydata->value_type) {
    case LYD_ANYDATA_XML:
        /* transform XML into CONSTSTRING */
        lyxml_print_mem(&buf, anydata->value.xml, LYXML_PRINT_SIBLINGS);
        lyxml_free(anydata->schema->module->ctx, anydata->value.xml);

        anydata->value_type = LYD_ANYDATA_CONSTSTRING;
        anydata->value.str = lydict_insert_zc(anydata->schema->module->ctx, buf);
        /* fallthrough */
    case LYD_ANYDATA_DATATREE:
    case LYD_ANYDATA_JSON:
    case LYD_ANYDATA_SXML:
    case LYD_ANYDATA_CONSTSTRING:
        type = anydata->value_type;
        break;
    default:
        return -1;
    }

    /* first byte is type */
    ret += lyb_write(out, (uint8_t *)&type, sizeof type, lybs);

    /* followed by the content */
    if (type == LYD_ANYDATA_DATATREE) {
        ret += lyb_print_data(out, anydata->value.tree, 0);
    } else {
        ret += lyb_write_string(anydata->value.str, out, lybs);
    }

    return ret;
}

static int
lyb_print_leaf(const struct lyd_node_leaf_list *leaf, struct lyout *out, struct lyb_state *lybs)
{
    int ret = 0;
    uint8_t byte = 0;
    size_t count, i, bits_i;
    LY_DATA_TYPE dtype;
    const struct lys_type *type = &((struct lys_node_leaf *)leaf->schema)->type;

    /* value type byte - ABCD DDDD
     *
     * A - dflt flag
     * B - user type flag
     * C - unres flag
     * D (5b) - data type value
     */
    if (leaf->dflt) {
        byte |= 0x80;
    }
    if (leaf->value_flags & LY_VALUE_USER) {
        byte |= 0x40;
    }
    if (leaf->value_flags & LY_VALUE_UNRES) {
        byte |= 0x20;
    }

store_value_type:
    /* we have only 5b available, must be enough */
    assert((type->base & 0x1f) == type->base);

    if (!(leaf->value_flags & LY_VALUE_USER)) {
        switch (type->base) {
        case LY_TYPE_DER:
            /* error */
            return 0;

        case LY_TYPE_LEAFREF:
            assert(!(leaf->value_flags & LY_VALUE_UNRES));
            /* find the leafref target */
            type = lyd_leaf_type(leaf);
            if (!type) {
                /* error */
                return 0;
            }
            goto store_value_type;

        default:
            /* just store the value type */
            byte |= type->base & 0x1f;
            break;
        }
    }

    /* write value type byte */
    ret += lyb_write(out, &byte, sizeof byte, lybs);

    /* print value itself */
    if (leaf->value_flags & LY_VALUE_USER) {
        dtype = LY_TYPE_STRING;
    } else {
        dtype = type->base;
    }
    switch (dtype) {
    case LY_TYPE_BINARY:
    case LY_TYPE_INST:
    case LY_TYPE_STRING:
    case LY_TYPE_UNION:
    case LY_TYPE_IDENT:
    case LY_TYPE_UNKNOWN:
        /* store string */
        ret += lyb_write_string(leaf->value_str, out, lybs);
        break;
    case LY_TYPE_BITS:
        /* find the correct structure */
        for (; !type->info.bits.count; type = &type->der->type);

        /* store a bitfield */
        bits_i = 0;

        for (count = type->info.bits.count / 8; count; --count) {
            /* will be a full byte */
            for (byte = 0, i = 0; i < 8; ++i) {
                if (leaf->value.bit[bits_i + i]) {
                    byte |= 0x80;
                }
                byte >>= 1;
            }
            ret += lyb_write(out, &byte, sizeof byte, lybs);
            bits_i += 8;
        }

        /* store the remainder */
        for (byte = 0, i = 0; i < type->info.bits.count % 8; ++i) {
            if (leaf->value.bit[bits_i + i]) {
                byte |= 0x80;
            }
            byte >>= 1;
        }
        ret += lyb_write(out, &byte, sizeof byte, lybs);
        break;
    case LY_TYPE_BOOL:
        /* store only 1b */
        byte = 0;
        if (leaf->value.bln) {
            byte |= 0x01;
        }
        ret += lyb_write(out, &byte, sizeof byte, lybs);
        break;
    case LY_TYPE_EMPTY:
        /* nothing to store */
        break;
    case LY_TYPE_ENUM:
        /* find the correct structure */
        for (; !type->info.enums.count; type = &type->der->type);

        /* store the enum index (save bytes if possible) */
        i = (leaf->value.enm - type->info.enums.enm) / sizeof *leaf->value.enm;
        ret += lyb_write_number(i, type->info.enums.enm[type->info.enums.count - 1].value, out, lybs);
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
        ret += lyb_write_number(leaf->value.uint8, UINT8_MAX, out, lybs);
        break;
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
        ret += lyb_write_number(leaf->value.uint16, UINT16_MAX, out, lybs);
        break;
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
        ret += lyb_write_number(leaf->value.uint32, UINT32_MAX, out, lybs);
        break;
    case LY_TYPE_DEC64:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        ret += lyb_write_number(leaf->value.uint64, UINT64_MAX, out, lybs);
        break;
    default:
        return 0;
    }

    return ret;
}

static LYB_HASH
lyb_hash_find(struct hash_table *ht, struct lys_node *node)
{
    LYB_HASH hash;
    uint32_t i;

    for (i = 0; i < LYB_HASH_BITS; ++i) {
        hash = lyb_hash(node, i);
        if (!hash) {
            return 0;
        }

        if (!lyht_find(ht, &node, hash, NULL)) {
            /* success, no collision */
            break;
        }
    }
    /* cannot happen, we already calculated the hash */
    if (i > LYB_HASH_BITS) {
        return 0;
    }

    return hash;
}

static int
lyb_print_subtree(struct lyout *out, const struct lyd_node *node, struct hash_table **sibling_ht, struct lyb_state *lybs,
                  int options, int top_level)
{
    int r, ret = 0;
    LYB_HASH hash = 0;
    struct hash_table *children_ht = NULL;

    /* create whole sibling HT if not already and get our hash */
    if (!*sibling_ht) {
        *sibling_ht = lyb_hash_siblings(node->schema, NULL, 0);
        if (!*sibling_ht) {
            return -1;
        }
    }
    hash = lyb_hash_find(*sibling_ht, node->schema);
    if (!hash) {
        return -1;
    }

    /* register a new subtree */
    ret += (r = lyb_write_start_subtree(out, lybs));
    if (r < 0) {
        return -1;
    }

    /*
     * write the node information
     */

    if (top_level) {
        /* write model info first */
        ret += (r = lyb_print_model(out, lyd_node_module(node), lybs));
        if (r < 0) {
            return -1;
        }
    }

    /* write schema node hash */
    ret += (r = lyb_write(out, &hash, sizeof hash, lybs));
    if (r < 0) {
        return -1;
    }

    /* TODO write attributes */

    /* write node content */
    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
        /* nothing to write */
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        ret += (r = lyb_print_leaf((const struct lyd_node_leaf_list *)node, out, lybs));
        if (r < 0) {
            return -1;
        }
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        ret += (r = lyb_print_anydata((struct lyd_node_anydata *)node, out, lybs));
        if (r < 0) {
            return -1;
        }
        break;
    default:
        return -1;
    }

    /* recursively write all the descendants */
    r = 0;
    if (node->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) {
        LY_TREE_FOR(node->child, node) {
            ret += (r = lyb_print_subtree(out, node, &children_ht, lybs, options, 0));
            if (r < 0) {
                break;
            }
        }
    }
    if (children_ht) {
        lyht_free(children_ht);
    }
    if (r < 0) {
        return -1;
    }

    /* finish this subtree */
    ret += (r = lyb_write_stop_subtree(out, lybs));
    if (r < 0) {
        return -1;
    }

    return ret;
}

int
lyb_print_data(struct lyout *out, const struct lyd_node *root, int options)
{
    int r, ret = 0;
    uint8_t zero = 0;
    struct hash_table *top_sibling_ht = NULL;
    struct lyb_state lybs;

    lybs.written = malloc(LYB_STATE_STEP * sizeof *lybs.written);
    lybs.position = malloc(LYB_STATE_STEP * sizeof *lybs.position);
    LY_CHECK_ERR_GOTO(!lybs.written || !lybs.position, LOGMEM(root->schema->module->ctx), finish);
    lybs.used = 0;
    lybs.size = LYB_STATE_STEP;

    /* LYB header */
    ret += (r = lyb_print_header(out, options));
    if (r < 0) {
        ret = -1;
        goto finish;
    }

    /* all used models */
    ret += (r = lyb_print_data_models(out, root, &lybs));
    if (r < 0) {
        ret = -1;
        goto finish;
    }

    LY_TREE_FOR(root, root) {
        ret += (r = lyb_print_subtree(out, root, &top_sibling_ht, &lybs, options, 1));
        if (r < 0) {
            ret = -1;
            goto finish;
        }

        if (!(options & LYP_WITHSIBLINGS)) {
            break;
        }
    }

    /* ending zero byte */
    ret += (r = lyb_write(out, &zero, sizeof zero, &lybs));
    if (r < 0) {
        ret = -1;
    }

finish:
    if (top_sibling_ht) {
        lyht_free(top_sibling_ht);
    }
    free(lybs.written);
    free(lybs.position);

    return ret;
}
