/**
 * @file hash_table.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang hash table
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_HASH_TABLE_H_
#define LY_HASH_TABLE_H_

#include <stdint.h>
#include <pthread.h>

#include "dict.h"

/**
 * size of the dictionary for each context
 */
#define DICT_SIZE 1024

/**
 * record of the dictionary
 * TODO: save the next pointer by different collision strategy, will need to
 * make dictionary size dynamic
 */
struct dict_rec {
    struct dict_rec *next;
    char *value;
    uint32_t refcount:22;
    uint32_t len:10;
#define DICT_REC_MAXCOUNT 0x003fffff
#define DICT_REC_MAXLEN   0x000003ff
};

/**
 * dictionary to store repeating strings
 * TODO: make it variable size
 */
struct dict_table {
    struct dict_rec recs[DICT_SIZE];
    int hash_mask;
    uint32_t used;
    pthread_mutex_t lock;
};

/**
 * @brief Initiate content (non-zero values) of the dictionary
 *
 * @param[in] dict Dictionary table to initiate
 */
void lydict_init(struct dict_table *dict);

/**
 * @brief Cleanup the dictionary content
 *
 * @param[in] dict Dictionary table to cleanup
 */
void lydict_clean(struct dict_table *dict);

/**
 * @brief compute hash from (several) string(s)
 *
 * Usage:
 * - init hash to 0
 * - repeatedly call dict_hash_multi(), provide hash from the last call
 * - call dict_hash_multi() with key_part = NULL to finish the hash
 */
uint32_t dict_hash_multi(uint32_t hash, const char *key_part, size_t len);

/**
 * @brief Callback for checking hash table values equivalence.
 *
 * @param[in] value1 First value.
 * @param[in] value2 Second value.
 * @param[in] cb_data User callback data.
 * @return 0 on non-equal, non-zero on equal.
 */
typedef int (*values_equal_cb)(void *value1, void *value2, void *cb_data);

/**
 * @brief Generic hash table.
 */
struct hash_table {
    struct ht_rec {
        void *value;    /* arbitrary value */
        uint32_t hash;  /* hash of value */
        uint32_t hits;  /* collision/overflow count */
    } *recs;
    uint32_t used;      /* number of values stored in the hash table */
    uint32_t size;      /* always holds 2^x = size (is power of 2) */
    values_equal_cb val_equal; /* callback for testing value equivalence */
    void *cb_data;      /* user data callback arbitrary value */
};

/**
 * @brief Create new hash table.
 *
 * @param[in] size Starting size of the hash table, must be power of 2.
 * @param[in] val_equal Callback for checking value equivalence.
 * @param[in] cb_data User data always passed to \p val_equal.
 * @return Empty hash table, NULL on error.
 */
struct hash_table *lyht_new(uint32_t size, values_equal_cb val_equal, void *cb_data);

/**
 * @brief Free a hash table.
 *
 * @param[in] ht Hash table to be freed.
 */
void lyht_free(struct hash_table *ht);

/**
 * @brief Insert a value into a hash table.
 *
 * @param[in] ht Hash table to insert into.
 * @param[in] value Value to insert.
 * @param[in] hash Hash of \p value.
 * @return 0 on success, 1 if already inserted.
 */
int lyht_insert(struct hash_table *ht, void *value, uint32_t hash);

/**
 * @brief Remove a value from a hash table.
 *
 * @param[in] ht Hash table to remove from.
 * @param[in] value Value to be removed.
 * @param[in] hash Hash of \p value.
 * @return 0 on success, 1 if value was not found.
 */
int lyht_remove(struct hash_table *ht, void *value, uint32_t hash);

#endif /* LY_HASH_TABLE_H_ */
