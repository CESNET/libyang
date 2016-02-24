/**
 * @file dict.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_DICT_PRIVATE_H_
#define LY_DICT_PRIVATE_H_

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
    char *value;
    uint32_t refcount;
    struct dict_rec *next;
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

#endif /* LY_DICT_PRIVATE_H_ */
