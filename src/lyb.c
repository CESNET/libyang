/**
 * @file lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB format common functionality.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "lyb.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "ly_common.h"
#include "tree_schema.h"

/**
 * @brief Generate single hash for a schema node to be used for LYB data.
 *
 * @param[in] node Node to hash.
 * @param[in] collision_id Collision ID of the hash to generate.
 * @return Generated hash.
 */
static LYB_HASH
lyb_generate_hash(const struct lysc_node *node, uint8_t collision_id)
{
    const struct lys_module *mod = node->module;
    uint32_t full_hash;
    LYB_HASH hash;

    /* generate full hash */
    full_hash = lyht_hash_multi(0, mod->name, strlen(mod->name));
    full_hash = lyht_hash_multi(full_hash, node->name, strlen(node->name));
    if (collision_id) {
        size_t ext_len;

        if (collision_id > strlen(mod->name)) {
            /* fine, we will not hash more bytes, just use more bits from the hash than previously */
            ext_len = strlen(mod->name);
        } else {
            /* use one more byte from the module name than before */
            ext_len = collision_id;
        }
        full_hash = lyht_hash_multi(full_hash, mod->name, ext_len);
    }
    full_hash = lyht_hash_multi(full_hash, NULL, 0);

    /* use the shortened hash */
    hash = full_hash & (LYB_HASH_MASK >> collision_id);
    /* add collision identificator */
    hash |= LYB_HASH_COLLISION_ID >> collision_id;

    return hash;
}

LYB_HASH
lyb_get_hash(const struct lysc_node *node, uint8_t collision_id)
{
    /* hashes must be cached */
    assert(node->hash[0]);

    if (collision_id < LYS_NODE_HASH_COUNT) {
        /* read from cache */
        return node->hash[collision_id];
    }

    /* generate */
    return lyb_generate_hash(node, collision_id);
}

LY_ERR
lyb_cache_node_hash_cb(struct lysc_node *node, void *UNUSED(data), ly_bool *UNUSED(dfs_continue))
{
    if (node->hash[0]) {
        /* already cached, stop the DFS */
        return LY_EEXIST;
    }

    for (uint8_t i = 0; i < LYS_NODE_HASH_COUNT; ++i) {
        /* store the hash in the cache */
        node->hash[i] = lyb_generate_hash(node, i);
    }

    return LY_SUCCESS;
}

uint8_t
lyb_right_bit_mask(uint8_t bit_count)
{
    uint8_t mask = 0;

    switch (bit_count) {
    case 1:
        mask = 0x01;
        break;
    case 2:
        mask = 0x03;
        break;
    case 3:
        mask = 0x07;
        break;
    case 4:
        mask = 0x0F;
        break;
    case 5:
        mask = 0x1F;
        break;
    case 6:
        mask = 0x3F;
        break;
    case 7:
        mask = 0x7F;
        break;
    case 8:
        mask = 0xFF;
        break;
    default:
        LOGINT(NULL);
        break;
    }

    return mask;
}

uint8_t
lyb_left_bit_mask(uint8_t bit_count)
{
    uint8_t mask = 0;

    switch (bit_count) {
    case 1:
        mask = 0x80;
        break;
    case 2:
        mask = 0xC0;
        break;
    case 3:
        mask = 0xE0;
        break;
    case 4:
        mask = 0xF0;
        break;
    case 5:
        mask = 0xF8;
        break;
    case 6:
        mask = 0xFC;
        break;
    case 7:
        mask = 0xFE;
        break;
    case 8:
        mask = 0xFF;
        break;
    default:
        LOGINT(NULL);
        break;
    }

    return mask;
}

void
lyb_right_shift(void *buf, uint32_t count_bytes, uint8_t shift)
{
    uint32_t i;
    uint8_t bits;

    if (!shift) {
        return;
    }

    for (i = 0; i < count_bytes; ++i) {
        /* shift the byte */
        ((uint8_t *)buf)[i] >>= shift;

        if (i < count_bytes - 1) {
            /* copy the bits from the next byte */
            bits = ((uint8_t *)buf)[i + 1] & lyb_right_bit_mask(shift);
            bits <<= 8 - shift;
            ((uint8_t *)buf)[i] |= bits;
        }
    }
}

void
lyb_prepend_bits(void *buf, uint32_t count_bytes, uint8_t byte, uint8_t byte_bits)
{
    uint8_t bits;

    if (!byte_bits) {
        return;
    }

    while (count_bytes) {
        /* shift the byte */
        ((uint8_t *)buf)[count_bytes - 1] <<= byte_bits;

        if (count_bytes - 1) {
            /* copy the bits from the previous byte */
            bits = ((uint8_t *)buf)[count_bytes - 2] & lyb_left_bit_mask(byte_bits);
            bits >>= 8 - byte_bits;
            ((uint8_t *)buf)[count_bytes - 1] |= bits;
        }

        --count_bytes;
    }

    /* prepend the bits from the byte */
    ((uint8_t *)buf)[0] |= byte & lyb_right_bit_mask(byte_bits);
}

uint32_t
lyb_truncate_hash_nonzero(uint32_t hash, uint8_t hash_bits)
{
    uint32_t orig_hash = hash;
    uint8_t offset;

    assert(hash && !(hash_bits % 8));

    offset = 0;
    do {
        hash = orig_hash;

        /* truncate hash */
        hash <<= (32 - offset) - hash_bits;
        hash >>= 32 - hash_bits;

        /* next offset */
        offset += 8;
    } while (!hash);

    return hash;
}
