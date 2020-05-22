/**
 * @file parser_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Internal structures and functions for libyang parsers
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_INTERNAL_H_
#define LY_PARSER_INTERNAL_H_

#include "parser.h"
#include "tree_schema_internal.h"

/**
 * @brief Parser input structure specifying where the data are read.
 */
struct ly_in {
    LY_IN_TYPE type;     /**< type of the output to select the output method */
    const char *current;
    const char *start;
    size_t length;
    union {
        int fd;          /**< file descriptor for LY_IN_FD type */
        FILE *f;         /**< file structure for LY_IN_FILE and LY_IN_FILEPATH types */
        struct {
            int fd;      /**< file descriptor for LY_IN_FILEPATH */
            char *filepath;   /**< stored original filepath */
        } fpath;         /**< filepath structure for LY_IN_FILEPATH */
    } method;            /**< type-specific information about the output */
};

/**
 * @brief Parse submodule from YANG data.
 * @param[in,out] ctx Parser context.
 * @param[in] ly_ctx Context of YANG schemas.
 * @param[in] main_ctx Parser context of main module.
 * @param[in] data Input data to be parsed.
 * @param[out] submod Pointer to the parsed submodule structure.
 * @return LY_ERR value - LY_SUCCESS, LY_EINVAL or LY_EVALID.
 */
LY_ERR yang_parse_submodule(struct lys_yang_parser_ctx **context, struct ly_ctx *ly_ctx, struct lys_parser_ctx *main_ctx,
                            const char *data, struct lysp_submodule **submod);

/**
 * @brief Parse module from YANG data.
 * @param[in] ctx Parser context.
 * @param[in] data Input data to be parsed.
 * @param[in, out] mod Prepared module structure where the parsed information, including the parsed
 * module structure, will be filled in.
 * @return LY_ERR value - LY_SUCCESS, LY_EINVAL or LY_EVALID.
 */
LY_ERR yang_parse_module(struct lys_yang_parser_ctx **context, const char *data, struct lys_module *mod);

/**
 * @brief Parse module from YIN data.
 *
 * @param[in,out] yin_ctx Context created during parsing, is used to finalize lysp_model after it's completly parsed.
 * @param[in] data Input data to be parsed.
 * @param[in,out] mod Prepared module structure where the parsed information, including the parsed
 * module structure, will be filled in.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_module(struct lys_yin_parser_ctx **yin_ctx, const char *data, struct lys_module *mod);

/**
 * @brief Parse submodule from YIN data.
 *
 * @param[in,out] yin_ctx Context created during parsing, is used to finalize lysp_model after it's completly parsed.
 * @param[in] ctx Libyang context.
 * @param[in] main_ctx Parser context of main module.
 * @param[in,out] data Input data to be parsed.
 * @param[in,out] submod Submodule structure where the parsed information, will be filled in.
 * @return LY_ERR values.
 */
LY_ERR yin_parse_submodule(struct lys_yin_parser_ctx **yin_ctx, struct ly_ctx *ctx, struct lys_parser_ctx *main_ctx,
                           const char *data, struct lysp_submodule **submod);

#endif /* LY_PARSER_INTERNAL_H_ */
