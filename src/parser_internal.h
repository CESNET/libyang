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

#include "plugins_types.h"
#include "tree_schema_internal.h"

struct lyd_ctx;
struct ly_in;

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
    lyd_ctx_free_clb free;         /**< destructor */

    struct {
        const struct ly_ctx *ctx;  /**< libyang context */
        uint64_t line;             /**< current line */
        struct ly_in *in;          /**< input structure */
    } *data_ctx;                   /**< generic pointer supposed to map to and access (common part of) XML/JSON/... parser contexts */
};

/**
 * @brief Common part of the lyd_ctx_free_t callbacks.
 */
void lyd_ctx_free(struct lyd_ctx *);

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
 * @brief Parse XML string as YANG data tree.
 *
 * @param[in] ctx libyang context
 * @param[in] in Input structure.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree_p Parsed data tree. Note that NULL can be a valid result.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_data(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree_p, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse XML string as YANG RPC/action invocation.
 *
 * Optional \<rpc\> envelope element is accepted if present. It is [checked](https://tools.ietf.org/html/rfc6241#section-4.1) and all
 * its XML attributes are parsed. As a content of the envelope, an RPC data or \<action\> envelope element is expected. The \<action\> envelope element is
 * also [checked](https://tools.ietf.org/html/rfc7950#section-7.15.2) and then an action data is expected as a content of this envelope.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full RPC/action tree.
 * @param[out] op_p Optional pointer to the actual operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p);

/**
 * @brief Parse XML string as YANG notification.
 *
 * Optional \<notification\> envelope element, if present, is [checked](https://tools.ietf.org/html/rfc5277#page-25)
 * and parsed. Specifically, its namespace and the child \<eventTime\> element and its value.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full notification tree.
 * @param[out] op_p Optional pointer to the actual notification. Useful mainly for nested notifications.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **ntf_p);

/**
 * @brief Parse XML string as YANG RPC/action reply.
 *
 * Optional \<rpc-reply\> envelope element, if present, is [checked](https://tools.ietf.org/html/rfc6241#section-4.2)
 * and all its XML attributes parsed.
 *
 * @param[in] request Data tree of the RPC/action request.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full reply tree. It always includes duplicated operation and parents of the @p request.
 * @param[out] op_p Optional pointer to the reply operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_reply(const struct lyd_node *request, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p);

/**
 * @brief Parse JSON string as YANG data tree.
 *
 * @param[in] ctx libyang context
 * @param[in] in Input structure.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree_p Parsed data tree. Note that NULL can be a valid result.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_json_data(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree_p, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse JSON string as YANG notification.
 *
 * Optional top-level "notification" envelope object, if present, is [checked](https://tools.ietf.org/html/rfc5277#page-25)
 * and parsed. Specifically the child "eventTime" member and its value.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full notification tree.
 * @param[out] ntf_p Optional pointer to the actual notification. Useful mainly for nested notifications.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_json_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **ntf_p);

/**
 * @brief Parse JSON string as YANG RPC/action invocation.
 *
 * Optional top-level "rpc" envelope object, if present is is [checked](https://tools.ietf.org/html/rfc6241#section-4.1) and the parser
 * goes inside for the content, which is an RPC data or "action" envelope objects. The "action" envelope object is
 * also [checked](https://tools.ietf.org/html/rfc7950#section-7.15.2) and then an action data is expected as a content of this envelope.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full RPC/action tree.
 * @param[out] op_p Optional pointer to the actual operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_json_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p);

/**
 * @brief Parse JSON string as YANG RPC/action reply.
 *
 * Optional "rpc-reply" envelope object, if present, is [checked](https://tools.ietf.org/html/rfc6241#section-4.2).
 *
 * @param[in] request Data tree of the RPC/action request.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full reply tree. It always includes duplicated operation and parents of the @p request.
 * @param[out] op_p Optional pointer to the reply operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_json_reply(const struct lyd_node *request, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p);

/**
 * @brief Parse binary data as YANG data tree.
 *
 * @param[in] ctx libyang context
 * @param[in] in Input structure.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree_p Parsed data tree. Note that NULL can be a valid result.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_data(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree_p, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse binary data as YANG RPC/action invocation.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full RPC/action tree.
 * @param[out] op_p Optional pointer to the actual operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p);

/**
 * @brief Parse binary data as YANG notification.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full notification tree.
 * @param[out] ntf_p Optional pointer to the actual notification. Useful mainly for nested notifications.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **ntf_p);

/**
 * @brief Parse binary data as YANG RPC/action reply.
 *
 * @param[in] request Data tree of the RPC/action request.
 * @param[in] in Input structure.
 * @param[out] tree_p Parsed full reply tree. It always includes duplicated operation and parents of the @p request.
 * @param[out] op_p Optional pointer to the reply operation. Useful mainly for action.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb_reply(const struct lyd_node *request, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p);

/**
 * @brief Check that a data node representing the @p snode is suitable based on options.
 *
 * @param[in] lydctx Common data parsers context.
 * @param[in] snode Schema node to check.
 * @return LY_SUCCESS or LY_EVALID
 */
LY_ERR lyd_parser_check_schema(struct lyd_ctx *lydctx, const struct lysc_node *snode);

/**
 * @brief Wrapper around ::lyd_create_term() for data parsers.
 *
 * @param[in] lydctx Data parser context.
 * @param[in] hints Data parser's hint for the value's type.
 */
LY_ERR lyd_parser_create_term(struct lyd_ctx *lydctx, const struct lysc_node *schema, const char *value, size_t value_len,
        ly_bool *dynamic, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, struct lyd_node **node);

/**
 * @brief Wrapper around ::lyd_create_meta() for data parsers.
 *
 * @param[in] lydctx Data parser context.
 * @param[in] hints [Value hint](@ref lydvalhints) from the parser regarding the value type.
 */
LY_ERR lyd_parser_create_meta(struct lyd_ctx *lydctx, struct lyd_node *parent, struct lyd_meta **meta,
        const struct lys_module *mod, const char *name, size_t name_len, const char *value,
        size_t value_len, ly_bool *dynamic, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints);

#endif /* LY_PARSER_INTERNAL_H_ */
