/**
 * @file hash_table_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang hash table internal header
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_HASH_TABLE_INTERNAL_H_
#define LY_HASH_TABLE_INTERNAL_H_

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#include "compat.h"
#include "hash_table.h"

/** reference value for 100% */
#define LYHT_HUNDRED_PERCENTAGE 100

/** when the table is at least this much percent full, it is enlarged (double the size) */
#define LYHT_ENLARGE_PERCENTAGE 75

/** only once the table is this much percent full, enable shrinking */
#define LYHT_FIRST_SHRINK_PERCENTAGE 50

/** when the table is less than this much percent full, it is shrunk (half the size) */
#define LYHT_SHRINK_PERCENTAGE 25

/** when the table has less than this much percent empty records, it is rehashed to get rid of all the invalid records */
#define LYHT_REHASH_PERCENTAGE 2

/** never shrink beyond this size */
#define LYHT_MIN_SIZE 8

/**
 * @brief Generic hash table record.
 */
struct ly_ht_rec {
    uint32_t hash;        /* hash of the value */
    int32_t hits;         /* collision/overflow value count - 1 (a filled entry has 1 hit,
                           * special value -1 means a deleted record) */
    unsigned char val[1]; /* arbitrary-size value */
} _PACKED;

/**
 * @brief (Very) generic hash table.
 *
 * Hash table with open addressing collision resolution and
 * linear probing of interval 1 (next free record is used).
 * Removal is lazy (removed records are only marked), but
 * if possible, they are fully emptied.
 */
struct ly_ht {
    uint32_t used;        /* number of values stored in the hash table (filled records) */
    uint32_t size;        /* always holds 2^x == size (is power of 2), actually number of records allocated */
    uint32_t invalid;     /* number of invalid records (deleted) */
    lyht_value_equal_cb val_equal; /* callback for testing value equivalence */
    void *cb_data;        /* user data callback arbitrary value */
    uint16_t resize;      /* 0 - resizing is disabled, *
                           * 1 - enlarging is enabled, *
                           * 2 - both shrinking and enlarging is enabled */
    uint16_t rec_size;    /* real size (in bytes) of one record for accessing recs array */
    unsigned char *recs;  /* pointer to the hash table itself (array of struct ht_rec) */
};

/**
 * @brief Dictionary hash table record.
 */
struct ly_dict_rec {
    char *value;        /**< stored string */
    uint32_t refcount;  /**< reference count of the string */
};

/**
 * @brief Dictionary for storing repeated strings.
 */
struct ly_dict {
    struct ly_ht *hash_tab;
    pthread_mutex_t lock;
};

/**
 * @brief Initiate content (non-zero values) of the dictionary
 *
 * @param[in] dict Dictionary table to initiate
 */
void lydict_init(struct ly_dict *dict);

/**
 * @brief Cleanup the dictionary content
 *
 * @param[in] dict Dictionary table to cleanup
 */
void lydict_clean(struct ly_dict *dict);

#endif /* LY_HASH_TABLE_INTERNAL_H_ */
