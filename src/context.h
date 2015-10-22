/**
 * @file context.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal context structures and functions
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

#ifndef LY_CONTEXT_H_
#define LY_CONTEXT_H_

#include "dict.h"
#include "tree_schema.h"

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
};

#endif /* LY_CONTEXT_H_ */
