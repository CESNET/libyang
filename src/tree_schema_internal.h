/**
 * @file tree_schema_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_INTERNAL_H_
#define LY_TREE_SCHEMA_INTERNAL_H_

/**
 * @brief internal context for schema parsers
 */
struct ly_parser_ctx {
    struct ly_ctx *ctx;
    uint64_t line;      /* line number */
    uint64_t indent;    /* current position on the line for YANG indentation */
};

/**
 * @brief Check the currently present prefixes in the module for collision with the new one.
 *
 * @param[in] ctx yang parser context.
 * @param[in] module Schema tree to check.
 * @param[in] value Newly added prefix value (including its location to distinguish collision with itself).
 * @return LY_EEXIST when prefix is already used in the module, LY_SUCCESS otherwise
 */
LY_ERR lysp_check_prefix(struct ly_parser_ctx *ctx, struct lysp_module *module, const char **value);

#endif /* LY_TREE_SCHEMA_INTERNAL_H_ */
