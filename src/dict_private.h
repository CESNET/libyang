/**
 * @file dict.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
