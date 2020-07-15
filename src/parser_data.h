/**
 * @file parser_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data parsers for libyang
 *
 * Copyright (c) 2015-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_DATA_H_
#define LY_PARSER_DATA_H_

#include "tree_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_in;

/**
 * @addtogroup datatree
 * @{
 */

/**
 * @defgroup dataparseroptions Data parser options
 *
 * Various options to change the data tree parsers behavior.
 *
 * Default parser behavior:
 * - complete input file is always parsed. In case of XML, even not well-formed XML document (multiple top-level
 * elements) is parsed in its entirety,
 * - parser silently ignores data without matching schema node definition,
 * - list instances are checked whether they have all the keys, error is raised if not.
 *
 * Default parser validation behavior:
 * - the provided data are expected to provide complete datastore content (both the configuration and state data)
 * and performs data validation according to all YANG rules, specifics follow,
 * - list instances are expected to have all the keys (it is not checked),
 * - instantiated (status) obsolete data print a warning,
 * - all types are fully resolved (leafref/instance-identifier targets, unions) and must be valid (lists have
 * all the keys, leaf(-lists) correct values),
 * - when statements on existing nodes are evaluated, if not satisfied, a validation error is raised,
 * - if-feature statements are evaluated,
 * - invalid multiple data instances/data from several cases cause a validation error,
 * - default values are added.
 * @{
 */
/* note: keep the lower 16bits free for use by LYD_VALIDATE_ flags. They are not supposed to be combined together,
 * but since they are used (as a separate parameter) together in some functions, we want to keep them in a separated
 * range to be able detect that the caller put wrong flags into the parser/validate options parameter. */
#define LYD_PARSE_ONLY      0x010000        /**< Data will be only parsed and no validation will be performed. When statements
                                                 are kept unevaluated, union types may not be fully resolved, if-feature
                                                 statements are not checked, and default values are not added (only the ones
                                                 parsed are present). */
#define LYD_PARSE_TRUSTED   0x020000        /**< Data are considered trusted so they will be parsed as validated. If the parsed
                                                 data are not valid, using this flag may lead to some unexpected behavior!
                                                 This flag can be used only with #LYD_OPT_PARSE_ONLY. */
#define LYD_PARSE_STRICT    0x040000        /**< Instead of silently ignoring data without schema definition raise an error.
                                                 Do not combine with #LYD_OPT_OPAQ (except for ::LYD_LYB). */
#define LYD_PARSE_OPAQ      0x080000        /**< Instead of silently ignoring data without definition, parse them into
                                                 an opaq node. Do not combine with #LYD_OPT_STRICT (except for ::LYD_LYB). */
#define LYD_PARSE_NO_STATE  0x100000        /**< Forbid state data in the parsed data. */

#define LYD_PARSE_LYB_MOD_UPDATE  0x200000  /**< Only for LYB format, allow parsing data printed using a specific module
                                                 revision to be loaded even with a module with the same name but newer
                                                 revision. */
/** @} dataparseroptions */


/**
 * @defgroup datavalidationoptions Data validation options
 * @ingroup datatree
 *
 * Various options to change data validation behaviour, both for the parser and separate validation.
 *
 * Default separate validation behavior:
 * - the provided data are expected to provide complete datastore content (both the configuration and state data)
 * and performs data validation according to all YANG rules, specifics follow,
 * - instantiated (status) obsolete data print a warning,
 * - all types are fully resolved (leafref/instance-identifier targets, unions) and must be valid (lists have
 * all the keys, leaf(-lists) correct values),
 * - when statements on existing nodes are evaluated. Depending on the previous when state (from previous validation
 * or parsing), the node is silently auto-deleted if the state changed from true to false, otherwise a validation error
 * is raised if it evaluates to false,
 * - if-feature statements are evaluated,
 * - data from several cases behave based on their previous state (from previous validation or parsing). If there existed
 * already a case and another one was added, the previous one is silently auto-deleted. Otherwise (if data from 2 or
 * more cases were created) a validation error is raised,
 * - default values are added.
 *
 * @{
 */
#define LYD_VALIDATE_NO_STATE     0x0001    /**< Consider state data not allowed and raise an error if they are found. */
#define LYD_VALIDATE_PRESENT      0x0002    /**< Validate only modules whose data actually exist. */

/** @} datavalidationoptions */

/**
 * @defgroup datavalidateop Operation to validate
 * @ingroup datatree
 *
 * Operation provided to lyd_validate_op() to validate. The operation cannot be determined automatically since RPC/action and a reply to it
 * share the common top level node referencing the RPC/action schema node and may not have any input/output children to use for distinction.
 */
typedef enum {
    LYD_VALIDATE_OP_RPC = 1,   /**< Validate RPC/action request (input parameters). */
    LYD_VALIDATE_OP_REPLY,     /**< Validate RPC/action reply (output parameters). */
    LYD_VALIDATE_OP_NOTIF      /**< Validate Notification operation. */
} LYD_VALIDATE_OP;

/** @} datavalidateop */

/**
 * @brief Parse (and validate) data from the input handler as a YANG data tree.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] in The input handle to provide the dumped data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed. Can be 0 to try to detect format from the input handler.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Resulting data tree built from the input data. Note that NULL can be a valid result as a representation of an empty YANG data tree.
 * The returned data are expected to be freed using lyd_free_all().
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LY_ERR lyd_parse_data(const struct ly_ctx *ctx, struct ly_in *in, LYD_FORMAT format, int parse_options,
                      int validate_options, struct lyd_node **tree);

/**
 * @brief Parse (and validate) input data as a YANG data tree.
 *
 * Wrapper around lyd_parse_data() hiding work with the input handler.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] data The input data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed. Can be 0 to try to detect format from the input handler.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Resulting data tree built from the input data. Note that NULL can be a valid result as a representation of an empty YANG data tree.
 * The returned data are expected to be freed using lyd_free_all().
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LY_ERR lyd_parse_data_mem(const struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int parse_options,
                          int validate_options, struct lyd_node **tree);

/**
 * @brief Parse (and validate) input data as a YANG data tree.
 *
 * Wrapper around lyd_parse_data() hiding work with the input handler.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the input data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed. Can be 0 to try to detect format from the input handler.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Resulting data tree built from the input data. Note that NULL can be a valid result as a representation of an empty YANG data tree.
 * The returned data are expected to be freed using lyd_free_all().
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LY_ERR lyd_parse_data_fd(const struct ly_ctx *ctx, int fd, LYD_FORMAT format, int parse_options, int validate_options,
                         struct lyd_node **tree);

/**
 * @brief Parse (and validate) input data as a YANG data tree.
 *
 * Wrapper around lyd_parse_data() hiding work with the input handler.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] path Path to the file with the input data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed. Can be 0 to try to detect format from the input handler.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Resulting data tree built from the input data. Note that NULL can be a valid result as a representation of an empty YANG data tree.
 * The returned data are expected to be freed using lyd_free_all().
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LY_ERR lyd_parse_data_path(const struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int parse_options,
                           int validate_options, struct lyd_node **tree);

/**
 * @brief Parse (and validate) data from the input handler as a YANG RPC/action invocation.
 *
 * In case o LYD_XML @p format, the \<rpc\> envelope element is accepted if present. It is [checked](https://tools.ietf.org/html/rfc6241#section-4.1), an opaq
 * data node (lyd_node_opaq) is created and all its XML attributes are parsed and inserted into the node. As a content of the enveloper, an RPC data or
 * \<action\> envelope element is expected. The \<action\> envelope element is also [checked](https://tools.ietf.org/html/rfc7950#section-7.15.2) and parsed as
 * the \<rpc\> envelope. Inside the \<action\> envelope, only an action data are expected.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] in The input handle to provide the dumped data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed.
 * @param[out] tree Resulting full RPC/action tree built from the input data. The returned data are expected to be freed using lyd_free_all().
 * In contrast to YANG data tree, result of parsing RPC/action cannot be NULL until an error occurs.
 * @param[out] op Optional pointer to the actual operation node inside the full action @p tree, useful only for action.
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LY_ERR lyd_parse_rpc(const struct ly_ctx *ctx, struct ly_in *in, LYD_FORMAT format, struct lyd_node **tree,
                     struct lyd_node **op);

/**
 * @brief Parse (and validate) data from the input handler as a YANG RPC/action reply.
 *
 * In case o LYD_XML @p format, the \<rpc-reply\> envelope element is accepted if present. It is [checked](https://tools.ietf.org/html/rfc6241#section-4.2), an opaq
 * data node (lyd_node_opaq) is created and all its XML attributes are parsed and inserted into the node.
 *
 * The reply data are strictly expected to be related to the provided RPC/action @p request.
 *
 * @param[in] request The RPC/action tree (result of lyd_parse_rpc()) of the request for the reply being parsed.
 * @param[in] in The input handle to provide the dumped data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed.
 * @param[out] tree Resulting full RPC/action reply tree built from the input data. The returned data are expected to be freed using lyd_free_all().
 * The reply tree always includes duplicated operation node (and its parents) of the @p request, so in contrast to YANG data tree,
 * the result of parsing RPC/action reply cannot be NULL until an error occurs.
 * @param[out] op Optional pointer to the actual operation node inside the full action reply @p tree, useful only for action.
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the request's context using ly_err* functions.
 */
LY_ERR lyd_parse_reply(const struct lyd_node *request, struct ly_in *in, LYD_FORMAT format, struct lyd_node **tree,
                       struct lyd_node **op);

/**
 * @brief Parse XML string as YANG notification.
 *
 * In case o LYD_XML @p format, the \<notification\> envelope element in combination with the child \<eventTime\> element are accepted if present. They are
 * [checked](https://tools.ietf.org/html/rfc5277#page-25), opaq data nodes (lyd_node_opaq) are created and all their XML attributes are parsed and inserted into the nodes.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] in The input handle to provide the dumped data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed.
 * @param[out] tree Resulting full Notification tree built from the input data. The returned data are expected to be freed using lyd_free_all().
 * In contrast to YANG data tree, result of parsing Notification cannot be NULL until an error occurs.
 * @param[out] ntf Optional pointer to the actual notification node inside the full Notification @p tree, useful for nested notifications.
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LY_ERR lyd_parse_notif(const struct ly_ctx *ctx, struct ly_in *in, LYD_FORMAT format, struct lyd_node **tree,
                       struct lyd_node **ntf);

/**
 * @brief Fully validate a data tree.
 *
 * @param[in,out] tree Data tree to recursively validate. May be changed by validation.
 * @param[in] ctx libyang context. Can be NULL if @p tree is set.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LY_ERR lyd_validate_all(struct lyd_node **tree, const struct ly_ctx *ctx, int val_opts, struct lyd_node **diff);

/**
 * @brief Fully validate a data tree of a module.
 *
 * @param[in,out] tree Data tree to recursively validate. May be changed by validation.
 * @param[in] module Module whose data (and schema restrictions) to validate.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LY_ERR lyd_validate_module(struct lyd_node **tree, const struct lys_module *module, int val_opts, struct lyd_node **diff);

/**
 * @brief Validate an RPC/action, notification, or RPC/action reply.
 *
 * @param[in,out] op_tree Operation tree with any parents. It can point to the operation itself or any of
 * its parents, only the operation subtree is actually validated.
 * @param[in] tree Tree to be used for validating references from the operation subtree.
 * @param[in] op Operation to validate (@ref datavalidateop), the given @p op_tree must correspond to this value. Note that
 * it isn't possible to detect the operation simply from the @p op_tree since RPC/action and their reply share the same
 * RPC/action data node and in case one of the input and output do not define any data node children, it is not possible
 * to get know what is here given for validation and if it is really valid.
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LY_ERR lyd_validate_op(struct lyd_node *op_tree, const struct lyd_node *tree, LYD_VALIDATE_OP op, struct lyd_node **diff);

/** @} datatree */

#ifdef __cplusplus
}
#endif

#endif /* LY_PARSER_DATA_H_ */
