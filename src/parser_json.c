/**
 * @file parser_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON data parser for libyang
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "config.h"
#include "context.h"

/**
 * @brief JSON-parser's implementation of ly_type_resolve_prefix() callback to provide mapping between prefixes used
 * in the values to the schema via context module names.
 */
const struct lys_module *
lydjson_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *UNUSED(parser))
{
    const struct lys_module *mod;
    char *name;

    name = strndup(prefix, prefix_len);
    if (!name) {
        return NULL;
    }

    mod = ly_ctx_get_module_implemented(ctx, name);
    free(name);
    return mod;
}
