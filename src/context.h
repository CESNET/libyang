/**
 * @file context.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal context structures and functions
 *
 * Copyright (c) 2015 - 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_CONTEXT_H_
#define LY_CONTEXT_H_

#include <pthread.h>

#include "common.h"
#include "set.h"
#include "hash_table.h"
#include "tree_schema.h"

/**
 * @brief Context of the YANG schemas
 */
struct ly_ctx {
    struct dict_table dict;           /**< dictionary to effectively store strings used in the context related structures */
    struct ly_set search_paths;       /**< set of directories where to search for schema's imports/includes */
    struct ly_set list;               /**< set of YANG schemas */
    uint16_t module_set_id;           /**< ID of the current set of schemas */
    uint16_t flags;                   /**< context settings, see @ref contextoptions. */
    pthread_key_t errlist_key;        /**< key for the thread-specific list of errors related to the context */
};

#endif /* LY_CONTEXT_H_ */
