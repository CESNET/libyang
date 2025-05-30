/**
 * @file lyb.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for LYB format printer & parser
 *
 * Copyright (c) 2020 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_LYB_H_
#define LY_LYB_H_

#include <stddef.h>
#include <stdint.h>

#include "ly_common.h"

struct ly_ctx;
struct lysc_node;

/*
 * LYB format
 *
 * Unlike XML or JSON, it is binary format so most data are represented in similar way but in binary.
 * Some notable differences:
 *
 * - schema nodes are identified based on their hash instead of their string name. In case of collisions
 * an array of hashes is created with each next hash one bit shorter until a unique sequence of all these
 * hashes is found and then all of them are stored.
 *
 * - tree structure is represented as individual strictly bounded "siblings". Each "siblings" begins
 * with its metadata, which consist of 1) the whole "sibling" length in bytes and 2) number
 * of included metadata chunks of nested "siblings".
 *
 * - since length of a "sibling" is not known before it is printed, holes are first written and
 * after the "sibling" is printed, they are filled with actual valid metadata. As a consequence,
 * LYB data cannot be directly printed into streams!
 *
 * - data are preceded with information about the used context. The exact same context must be used for
 * parsing the data to guarantee that all the schema nodes get the same hash.
 *
 * This is a short summary of the format:
 * @verbatim

 sb          = siblings_start
 se          = siblings_end
 siblings    = zero-LYB_SIZE_BYTES | (sb instance+ se)
 instance    = node_type module hash node
 module      = 16bit_zero | (module_name_length module_name revision)
 node        = opaq | leaflist | list | any | inner | leaf
 opaq        = opaq_data siblings
 leaflist    = sb leaf+ se
 list        = sb (node_header siblings)+ se
 any         = node_header anydata_data
 inner       = node_header siblings
 leaf        = node_header term_value
 node_header = metadata node_flags

 @endverbatim
 */

/**
 * @brief LYB data node type
 */
enum lylyb_node_type {
    LYB_NODE_END = 0,   /**< no more nodes, marks the end of a list of nodes */
    LYB_NODE_TOP,       /**< top-level node */
    LYB_NODE_CHILD,     /**< child (or opaque) node with a parent */
    LYB_NODE_EXT        /**< nested extension node */
};

/**< number of required bits required for all node types */
#define LYB_NODE_TYPE_BITS 2

/**< number of required data node flag bits, fixed LYB size */
#define LYB_DATA_NODE_FLAG_BITS 4

/**
 * @brief LYB format printer context
 */
struct lylyb_print_ctx {
    const struct ly_ctx *ctx;

    struct lyd_lyb_sib_ht {
        struct lysc_node *first_sibling;
        struct ly_ht *ht;
    } *sib_hts;                 /**< sibling hash tables */
    ly_bool empty_hash;         /**< mark empty context hash */

    struct ly_out *out;         /**< output structure */
    uint8_t buf;                /**< not yet written rightmost bits, unused bits are zeroed */
    uint8_t buf_bits;           /**< cached buf bit count */
};

/**
 * @brief LYB format parser context
 */
struct lylyb_parse_ctx {
    const struct ly_ctx *ctx;

    uint64_t line;              /**< current line */
    ly_bool empty_hash;         /**< mark empty context hash */

    struct ly_in *in;           /**< input structure */
    uint8_t buf;                /**< read leftover rightmost bits from in */
    uint8_t buf_bits;           /**< cached buf bit count */
};

/**< current LYB format version */
#define LYB_HEADER_VERSION_NUM 0x07

/**< LYB format version mask of the header byte */
#define LYB_HEADER_VERSION_MASK 0x0F

/**< current LYB hash function */
#ifdef LY_XXHASH_SUPPORT
# define LYB_HEADER_HASH_ALG 0x20  /**< xxhash */
#else
# define LYB_HEADER_HASH_ALG 0x10  /**< one-at-a-time hash */
#endif

/**< LYB hash algorithm mask of the header byte */
#define LYB_HEADER_HASH_MASK 0x30

/**< reserved number of metadata instances used for the last instance of (leaf-)list */
#define LYB_METADATA_END_NUM 15

/**< reserved encoded number of metadata instances */
#define LYB_METADATA_END 0x3D

/**< number of required bits for reserved metadata instance count */
#define LYB_METADATA_END_BITS 6

/**< opaque node format LY_VALUE_XML */
#define LYB_OPAQ_FORMAT_XML 0x0

/**< opaque node format LY_VALUE_JSON */
#define LYB_OPAQ_FORMAT_JSON 0x1

/**< number of required bits for all LYB opaque formats */
#define LYB_OPAQ_FORMAT_BITS 1

/**
 * LYB schema hash constants
 *
 * Hash is divided to collision ID and hash itself.
 *
 * @anchor collisionid
 *
 * First bits are collision ID until 1 is found. The rest is truncated 32b hash.
 * 1xxx xxxx - collision ID 0 (no collisions)
 * 01xx xxxx - collision ID 1 (collision ID 0 hash collided)
 * 001x xxxx - collision ID 2 ...
 *
 * When finding a match for a unique schema (siblings) hash (sequence of hashes with increasing collision ID), the highest
 * collision ID can be read from the last hash (LYB parser).
 *
 * To learn what is the highest collision ID of a hash that must be included in a unique schema (siblings) hash,
 * collisions with all the preceding sibling schema hashes must be checked (LYB printer).
 */

/**< number of bits the whole hash will take (including hash collision ID) */
#define LYB_HASH_BITS 8

/**< masking 32b hash (collision ID 0) */
#define LYB_HASH_MASK 0x7f

/**< type for storing the whole hash (used only internally, publicly defined directly) */
#define LYB_HASH uint8_t

/**< need to move this first >> collision number (from 0) to get collision ID hash part */
#define LYB_HASH_COLLISION_ID 0x80

/**< module revision as XXXX XXXX XXXX XXXX (2B) (year is offset from 2000)
 *                      YYYY YYYM MMMD DDDD */
#define LYB_REV_YEAR_OFFSET 2000
#define LYB_REV_YEAR_MASK   0xfe00U
#define LYB_REV_YEAR_SHIFT  9
#define LYB_REV_MONTH_MASK  0x01E0U
#define LYB_REV_MONTH_SHIFT 5
#define LYB_REV_DAY_MASK    0x001fU

/**
 * @brief Get single hash for a schema node to be used for LYB data. Read from cache, if possible.
 *
 * @param[in] node Node to hash.
 * @param[in] collision_id Collision ID of the hash to generate, see @ref collisionid.
 * @return Generated hash.
 */
LYB_HASH lyb_get_hash(const struct lysc_node *node, uint8_t collision_id);

/**
 * @brief Module DFS callback filling all cached hashes of a schema node.
 */
LY_ERR lyb_cache_node_hash_cb(struct lysc_node *node, void *data, ly_bool *dfs_continue);

/**
 * @brief Get a mask with specific number of rightmost bits set.
 *
 * @param[in] bit_count Number of bits to set in the mask.
 * @return Bit mask.
 */
uint8_t lyb_right_bit_mask(uint8_t bit_count);

/**
 * @brief Get a mask with specific number of leftmost bits set.
 *
 * @param[in] bit_count Number of bits to set in the mask.
 * @return Bit mask.
 */
uint8_t lyb_left_bit_mask(uint8_t bit_count);

/**
 * @brief Prepend bits to an array of bytes.
 *
 * @p byte_bits number of bits are lost in the last byte.
 *
 * @param[in] buf Buffer with bytes to prepend to.
 * @param[in] count_bytes Count of bytes in buf.
 * @param[in] byte Byte with the bits to prepend.
 * @param[in] byte_bits Number of @p byte bits to prepend.
 */
void lyb_prepend_bits(void *buf, uint64_t count_bytes, uint8_t byte, uint8_t byte_bits);

#endif /* LY_LYB_H_ */
