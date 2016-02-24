/**
 * @file context.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal context structures and functions
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_CONTEXT_H_
#define LY_CONTEXT_H_

#include "dict_private.h"
#include "tree_schema.h"
#include "libyang.h"

struct ly_modules_list {
    char *search_path;
    int size;
    int used;
    struct lys_module **list;
    const char **parsing;
    uint16_t module_set_id;
};

struct ly_ctx {
    struct dict_table dict;
    struct ly_modules_list models;
    ly_module_clb module_clb;
    void *module_clb_data;
};

#endif /* LY_CONTEXT_H_ */
