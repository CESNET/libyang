/**
 * @file hash_table.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang hash table
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_HASH_TABLE_H_
#define LY_HASH_TABLE_H_

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#include "compat.h"
#include "log.h"

/**
 * @brief Compute hash from (several) string(s).
 *
 * Usage:
 * - init hash to 0
 * - repeatedly call ::dict_hash_multi(), provide hash from the last call
 * - call ::dict_hash_multi() with key_part = NULL to finish the hash
 */
uint32_t dict_hash_multi(uint32_t hash, const char *key_part, size_t len);

/**
 * @brief Compute hash from a string.
 */
uint32_t dict_hash(const char *key, size_t len);

/**
 * @brief Callback for checking hash table values equivalence.
 *
 * @param[in] val1_p Pointer to the first value, the one being searched (inserted/removed).
 * @param[in] val2_p Pointer to the second value, the one stored in the hash table.
 * @param[in] mod Whether the operation modifies the hash table (insert or remove) or not (find).
 * @param[in] cb_data User callback data.
 * @return false (non-equal) or true (equal).
 */
typedef ly_bool (*lyht_value_equal_cb)(void *val1_p, void *val2_p, ly_bool mod, void *cb_data);

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

struct ly_dict_rec {
    char *value;
    uint32_t refcount;
};

/**
 * dictionary to store repeating strings
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

/**
 * @brief Create new hash table.
 *
 * @param[in] size Starting size of the hash table (capacity of values), must be power of 2.
 * @param[in] val_size Size in bytes of value (the stored hashed item).
 * @param[in] val_equal Callback for checking value equivalence.
 * @param[in] cb_data User data always passed to @p val_equal.
 * @param[in] resize Whether to resize the table on too few/too many records taken.
 * @return Empty hash table, NULL on error.
 */
struct ly_ht *lyht_new(uint32_t size, uint16_t val_size, lyht_value_equal_cb val_equal, void *cb_data, uint16_t resize);

/**
 * @brief Set hash table value equal callback.
 *
 * @param[in] ht Hash table to modify.
 * @param[in] new_val_equal New callback for checking value equivalence.
 * @return Previous callback for checking value equivalence.
 */
lyht_value_equal_cb lyht_set_cb(struct ly_ht *ht, lyht_value_equal_cb new_val_equal);

/**
 * @brief Set hash table value equal callback user data.
 *
 * @param[in] ht Hash table to modify.
 * @param[in] new_cb_data New data for values callback.
 * @return Previous data for values callback.
 */
void *lyht_set_cb_data(struct ly_ht *ht, void *new_cb_data);

/**
 * @brief Make a duplicate of an existing hash table.
 *
 * @param[in] orig Original hash table to duplicate.
 * @return Duplicated hash table @p orig, NULL on error.
 */
struct ly_ht *lyht_dup(const struct ly_ht *orig);

/**
 * @brief Free a hash table.
 *
 * @param[in] ht Hash table to be freed.
 * @param[in] val_free Optional callback for freeing allthe stored values, @p val_p is a pointer to a stored value.
 */
void lyht_free(struct ly_ht *ht, void (*val_free)(void *val_p));

/**
 * @brief Find a value in a hash table.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the value to find.
 * @param[in] hash Hash of the stored value.
 * @param[out] match_p Pointer to the matching value, optional.
 * @return LY_SUCCESS if value was found,
 * @return LY_ENOTFOUND if not found.
 */
LY_ERR lyht_find(struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p);

/**
 * @brief Find another equal value in the hash table.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the previously found value in @p ht.
 * @param[in] hash Hash of the previously found value.
 * @param[out] match_p Pointer to the matching value, optional.
 * @return LY_SUCCESS if value was found,
 * @return LY_ENOTFOUND if not found.
 */
LY_ERR lyht_find_next(struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p);

/**
 * @brief Find another equal value in the hash table. Same functionality as ::lyht_find_next()
 * but allows to specify a collision val equal callback to be used for checking for matching colliding values.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the previously found value in @p ht.
 * @param[in] hash Hash of the previously found value.
 * @param[in] collision_val_equal Val equal callback to use for checking collisions.
 * @param[out] match_p Pointer to the matching value, optional.
 * @return LY_SUCCESS if value was found,
 * @return LY_ENOTFOUND if not found.
 */
LY_ERR lyht_find_next_with_collision_cb(struct ly_ht *ht, void *val_p, uint32_t hash,
        lyht_value_equal_cb collision_val_equal, void **match_p);

/**
 * @brief Insert a value into a hash table.
 *
 * @param[in] ht Hash table to insert into.
 * @param[in] val_p Pointer to the value to insert. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @param[out] match_p Pointer to the stored value, optional
 * @return LY_SUCCESS on success,
 * @return LY_EEXIST in case the value is already present.
 * @return LY_EMEM in case of memory allocation failure.
 */
LY_ERR lyht_insert(struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p);

/**
 * @brief Insert a value into hash table. Same functionality as ::lyht_insert()
 * but allows to specify a temporary val equal callback to be used in case the hash table
 * will be resized after successful insertion.
 *
 * @param[in] ht Hash table to insert into.
 * @param[in] val_p Pointer to the value to insert. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @param[in] resize_val_equal Val equal callback to use for resizing.
 * @param[out] match_p Pointer to the stored value, optional
 * @return LY_SUCCESS on success,
 * @return LY_EEXIST in case the value is already present.
 * @return LY_EMEM in case of memory allocation failure.
 */
LY_ERR lyht_insert_with_resize_cb(struct ly_ht *ht, void *val_p, uint32_t hash, lyht_value_equal_cb resize_val_equal,
        void **match_p);

/**
 * @brief Remove a value from a hash table.
 *
 * @param[in] ht Hash table to remove from.
 * @param[in] val_p Pointer to value to be removed. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @return LY_SUCCESS on success,
 * @return LY_ENOTFOUND if value was not found.
 */
LY_ERR lyht_remove(struct ly_ht *ht, void *val_p, uint32_t hash);

/**
 * @brief Remove a value from a hash table. Same functionality as ::lyht_remove()
 * but allows to specify a temporary val equal callback to be used in case the hash table
 * will be resized after successful removal.
 *
 * @param[in] ht Hash table to remove from.
 * @param[in] val_p Pointer to value to be removed. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @param[in] resize_val_equal Val equal callback to use for resizing.
 * @return LY_SUCCESS on success,
 * @return LY_ENOTFOUND if value was not found.
 */
LY_ERR lyht_remove_with_resize_cb(struct ly_ht *ht, void *val_p, uint32_t hash, lyht_value_equal_cb resize_val_equal);

/**
 * @brief Get suitable size of a hash table for a fixed number of items.
 *
 * @param[in] item_count Number of stored items.
 * @return Hash table size.
 */
uint32_t lyht_get_fixed_size(uint32_t item_count);

#endif /* LY_HASH_TABLE_H_ */
