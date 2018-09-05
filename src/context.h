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
#include "hash_table.h"
#include "tree_schema.h"

struct ly_modules_list {
    char **search_paths;
    int size;
    int used;
    struct lys_module **list;
    /* all (sub)modules that are currently being parsed */
    struct lys_module **parsing_sub_modules;
    /* all already parsed submodules of a module, which is before all its submodules (to mark submodule imports) */
    struct lys_module **parsed_submodules;
    uint8_t parsing_sub_modules_count;
    uint8_t parsed_submodules_count;
    uint16_t module_set_id;
    int flags; /* see @ref contextoptions. */
};

/**
 * @brief Context of the YANG schemas
 */
struct ly_ctx {
    struct dict_table dict;
    struct ly_modules_list models;
    pthread_key_t errlist_key;
};


#endif /* LY_CONTEXT_H_ */
