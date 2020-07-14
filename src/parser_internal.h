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

struct lyd_ctx;
/**
 * @brief Callback for lyd_ctx to free the structure
 *
 * @param[in] ctx Data parser context to free.
 */
typedef void (*lyd_ctx_free_clb)(struct lyd_ctx *ctx);

/**
 * @brief Internal (common) context for YANG data parsers.
 */
struct lyd_ctx {
    uint32_t parse_options;        /**< various @ref dataparseroptions. */
    uint32_t validate_options;     /**< various @ref datavalidationoptions. */
    uint32_t int_opts;             /**< internal data parser options */
    uint32_t path_len;             /**< used bytes in the path buffer */
#define LYD_PARSER_BUFSIZE 4078
    char path[LYD_PARSER_BUFSIZE]; /**< buffer for the generated path */
    struct ly_set unres_node_type; /**< set of nodes validated with LY_EINCOMPLETE result */
    struct ly_set unres_meta_type; /**< set of metadata validated with LY_EINCOMPLETE result */
    struct ly_set when_check;      /**< set of nodes with "when" conditions */
    struct lyd_node *op_node;      /**< if an RPC/action/notification is being parsed, store the pointer to it */

    /* callbacks */
    lyd_ctx_free_clb free;             /* destructor */
    ly_resolve_prefix_clb resolve_prefix;

    struct {
        const struct ly_ctx *ctx;
        uint64_t line;             /* current line */
        struct ly_in *in;          /* input structure */
    } *data_ctx;                   /* generic pointer supposed to map to and access (common part of) XML/JSON/... parser contexts */
};

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
 * @brief Common part of the lyd_ctx_free_t callbacks.
 */
void lyd_ctx_free(struct lyd_ctx *);

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

/**
 * @brief Check that a data node representing the @p snode is suitable based on options.
 *
 * @param[in] lydctx Common data parsers context.
 * @param[in] snode Schema node to check.
 * @return LY_SUCCESS or LY_EVALID
 */
LY_ERR lyd_parser_check_schema(struct lyd_ctx *lydctx, const struct lysc_node *snode);

/**
 * @brief Wrapper around lyd_create_term() for data parsers.
 *
 * @param[in] lydctx Data parser context.
 * @param[in] value_hints Data parser's hint for the value's type.
 */
LY_ERR lyd_parser_create_term(struct lyd_ctx *lydctx, const struct lysc_node *schema, const char *value, size_t value_len,
                              int *dynamic, int value_hints, ly_resolve_prefix_clb get_prefix, void *prefix_data,
                              LYD_FORMAT format, struct lyd_node **node);

/**
 * @brief Wrapper around lyd_create_meta() for data parsers.
 *
 * @param[in] lydctx Data parser context.
 * @param[in] value_hints [Value hint](@ref lydvalueparseopts) from the parser regarding the value type.
 */
LY_ERR lyd_parser_create_meta(struct lyd_ctx *lydctx, struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod,
                              const char *name, size_t name_len, const char *value, size_t value_len, int *dynamic, int value_hints,
                              ly_resolve_prefix_clb resolve_prefix, void *prefix_data, LYD_FORMAT format, const struct lysc_node *ctx_snode);

#endif /* LY_PARSER_INTERNAL_H_ */
