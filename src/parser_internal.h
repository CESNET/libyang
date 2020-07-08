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
 * @brief Mask for checking LYD_PARSE_ options (@ref dataparseroptions)
 */
#define LYD_PARSE_OPTS_MASK    0xFFFF0000

/**
 * @brief Mask for checking LYD_VALIDATEP_ options (@ref datavalidationoptions)
 */
#define LYD_VALIDATE_OPTS_MASK 0x0000FFFF

/**
 * @brief Parser input structure specifying where the data are read.
 */
struct ly_in {
    LY_IN_TYPE type;        /**< type of the output to select the output method */
    const char *current;    /**< Current position in the input data */
    const char *func_start; /**< Input data position when the last parser function was executed */
    const char *start;      /**< Input data start */
    size_t length;          /**< mmap() length (if used) */
    union {
        int fd;             /**< file descriptor for LY_IN_FD type */
        FILE *f;            /**< file structure for LY_IN_FILE and LY_IN_FILEPATH types */
        struct {
            int fd;         /**< file descriptor for LY_IN_FILEPATH */
            char *filepath; /**< stored original filepath */
        } fpath;            /**< filepath structure for LY_IN_FILEPATH */
    } method;               /**< type-specific information about the output */
};

/**
 * @brief Read bytes from an input.
 *
 * @param[in] in Input structure.
 * @param[in] buf Destination buffer.
 * @param[in] count Number of bytes to read.
 * @return LY_SUCCESS on success,
 * @return LY_EDENIED on EOF.
 */
LY_ERR ly_in_read(struct ly_in *in, void *buf, size_t count);

/**
 * @brief Just skip bytes in an input.
 *
 * @param[in] in Input structure.
 * @param[in] count Number of bytes to skip.
 * @return LY_SUCCESS on success,
 * @return LY_EDENIED on EOF.
 */
LY_ERR ly_in_skip(struct ly_in *in, size_t count);

/**
 * @brief Parse submodule from YANG data.
 * @param[in,out] ctx Parser context.
 * @param[in] ly_ctx Context of YANG schemas.
 * @param[in] main_ctx Parser context of main module.
 * @param[in] in Input structure.
 * @param[out] submod Pointer to the parsed submodule structure.
 * @return LY_ERR value - LY_SUCCESS, LY_EINVAL or LY_EVALID.
 */
LY_ERR yang_parse_submodule(struct lys_yang_parser_ctx **context, struct ly_ctx *ly_ctx, struct lys_parser_ctx *main_ctx,
                            struct ly_in *in, struct lysp_submodule **submod);

/**
 * @brief Parse module from YANG data.
 * @param[in] ctx Parser context.
 * @param[in] in Input structure.
 * @param[in,out] mod Prepared module structure where the parsed information, including the parsed
 * module structure, will be filled in.
 * @return LY_ERR value - LY_SUCCESS, LY_EINVAL or LY_EVALID.
 */
LY_ERR yang_parse_module(struct lys_yang_parser_ctx **context, struct ly_in *in, struct lys_module *mod);

/**
 * @brief Parse module from YIN data.
 *
 * @param[in,out] yin_ctx Context created during parsing, is used to finalize lysp_model after it's completly parsed.
 * @param[in] in Input structure.
 * @param[in,out] mod Prepared module structure where the parsed information, including the parsed
 * module structure, will be filled in.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_module(struct lys_yin_parser_ctx **yin_ctx, struct ly_in *in, struct lys_module *mod);

/**
 * @brief Parse submodule from YIN data.
 *
 * @param[in,out] yin_ctx Context created during parsing, is used to finalize lysp_model after it's completly parsed.
 * @param[in] ctx Libyang context.
 * @param[in] main_ctx Parser context of main module.
 * @param[in] in Input structure.
 * @param[in,out] submod Submodule structure where the parsed information, will be filled in.
 * @return LY_ERR values.
 */
LY_ERR yin_parse_submodule(struct lys_yin_parser_ctx **yin_ctx, struct ly_ctx *ctx, struct lys_parser_ctx *main_ctx,
                           struct ly_in *in, struct lysp_submodule **submod);

#endif /* LY_PARSER_INTERNAL_H_ */
