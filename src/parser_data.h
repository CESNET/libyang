/**
 * @file parser_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Data parsers for libyang
 *
 * Copyright (c) 2015-2023 CESNET, z.s.p.o.
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
 * @page howtoDataParsers Parsing Data
 *
 * Data parser allows to read instances from a specific format. libyang supports the following data formats:
 *
 * - XML
 *
 *   Original data format used in NETCONF protocol. XML mapping is part of the YANG specification
 *   ([RFC 6020](http://tools.ietf.org/html/rfc6020)).
 *
 * - JSON
 *
 *   The alternative data format available in RESTCONF protocol. Specification of JSON encoding of data modeled by YANG
 *   can be found in [RFC 7951](http://tools.ietf.org/html/rfc7951). The specification does not cover RPCs, actions and
 *   Notifications, so the representation of these data trees is proprietary and corresponds to the representation of these
 *   trees in XML.
 *
 * While the parsers themselves process the input data only syntactically, all the parser functions actually incorporate
 * the [common validator](@ref howtoDataValidation) checking the input data semantically. Therefore, the parser functions
 * accepts two groups of options - @ref dataparseroptions and @ref datavalidationoptions.
 *
 * In contrast to the schema parser, data parser also accepts empty input data if such an empty data tree is valid
 * according to the schemas in the libyang context (i.e. there are no top level mandatory nodes).
 *
 * There are individual functions to process different types of the data instances trees:
 * - ::lyd_parse_data() is intended for standard configuration data trees. According to the given
 *   [parser options](@ref dataparseroptions), the caller can further specify which kind of data tree is expected:
 *   - *complete :running datastore*: this is the default case, possibly with the use of (some of) the
 *     ::LYD_PARSE_STRICT, ::LYD_PARSE_OPAQ or ::LYD_VALIDATE_PRESENT options.
 *   - *complete configuration-only datastore* (such as :startup): in this case it is necessary to except all state data
 *     using ::LYD_PARSE_NO_STATE option.
 *   - *incomplete datastore*: there are situation when the data tree is incomplete or invalid by specification. For
 *     example the *:operational* datastore is not necessarily valid and results of the NETCONF's \<get\> or \<get-config\>
 *     oprations used with filters will be incomplete (and thus invalid). This can be allowed using ::LYD_PARSE_ONLY,
 *     the ::LYD_PARSE_NO_STATE should be used for the data returned by \<get-config\> operation.
 * - ::lyd_parse_ext_data() is used for parsing configuration data trees defined inside extension instances, such as
 *   instances of yang-data extension specified in [RFC 8040](http://tools.ietf.org/html/rfc8040).
 * - ::lyd_parse_op() is used for parsing RPCs/actions, replies, and notifications. Even NETCONF rpc, rpc-reply, and
 *   notification messages are supported.
 * - ::lyd_parse_ext_op() is used for parsing RPCs/actions, replies, and notifications defined inside extension instances.
 *
 * Further information regarding the processing input instance data can be found on the following pages.
 * - @subpage howtoDataValidation
 * - @subpage howtoDataWD
 *
 * Functions List
 * --------------
 * - ::lyd_parse_data()
 * - ::lyd_parse_data_mem()
 * - ::lyd_parse_data_fd()
 * - ::lyd_parse_data_path()
 * - ::lyd_parse_ext_data()
 * - ::lyd_parse_op()
 * - ::lyd_parse_ext_op()
 */

/**
 * @page howtoDataValidation Validating Data
 *
 * Data validation is performed implicitly to the input data processed by the [parser](@ref howtoDataParsers) and
 * on demand via the lyd_validate_*() functions. The explicit validation process is supposed to be used when a (complex or
 * simple) change is done on the data tree (via [data manipulation](@ref howtoDataManipulation) functions) and the data
 * tree is expected to be valid (it doesn't make sense to validate modified result of filtered \<get\> operation).
 *
 * Similarly to the [data parser](@ref howtoDataParsers), there are individual functions to validate standard data tree
 * (::lyd_validate_all()) and RPC, Action and Notification (::lyd_validate_op()). For the standard data trees, it is possible
 * to modify the validation process by @ref datavalidationoptions. This way the state data can be prohibited
 * (::LYD_VALIDATE_NO_STATE) and checking for mandatory nodes can be limited to the YANG modules with already present data
 * instances (::LYD_VALIDATE_PRESENT). Validation of the standard data tree can be also limited with ::lyd_validate_module()
 * function, which scopes only to a specified single YANG module.
 *
 * Since the operation data trees (RPCs, Actions or Notifications) can reference (leafref, instance-identifier, when/must
 * expressions) data from a datastore tree, ::lyd_validate_op() may require additional data tree to be provided. This is a
 * difference in contrast to the parsing process, when the data are loaded from an external source and invalid reference
 * outside the operation tree is acceptable.
 *
 * Functions List
 * --------------
 * - ::lyd_validate_all()
 * - ::lyd_validate_module()
 * - ::lyd_validate_op()
 */

/**
 * @addtogroup datatree
 * @{
 */

/**
 * @ingroup datatree
 * @defgroup dataparseroptions Data parser options
 *
 * Various options to change the data tree parsers behavior.
 *
 * Default parser behavior:
 * - complete input file is always parsed. In case of XML, even not well-formed XML document (multiple top-level
 * elements) is parsed in its entirety,
 * - parser silently ignores data without matching schema node definition (for LYB format an error),
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
 * - invalid multiple data instances/data from several cases cause a validation error,
 * - implicit nodes (NP containers and default values) are added.
 * @{
 */
/* note: keep the lower 16bits free for use by LYD_VALIDATE_ flags. They are not supposed to be combined together,
 * but since they are used (as a separate parameter) together in some functions, we want to keep them in a separated
 * range to be able detect that the caller put wrong flags into the parser/validate options parameter. */
#define LYD_PARSE_ONLY      0x010000        /**< Data will be only parsed and no data validation will be performed but
                                                 type value restrictions will be checked (unlike ::LYD_PARSE_STORE_ONLY).
                                                 When statements are kept unevaluated, union types may not be fully
                                                 resolved, and default values are not added (only the ones parsed are
                                                 present). */
#define LYD_PARSE_STRICT    0x020000        /**< Instead of silently ignoring data without schema definition raise an error.
                                                 Do not combine with ::LYD_PARSE_OPAQ (except for ::LYD_LYB). */
#define LYD_PARSE_OPAQ      0x040000        /**< Instead of silently ignoring data without definition, parse them into
                                                 an opaq node. Do not combine with ::LYD_PARSE_STRICT (except for ::LYD_LYB). */
#define LYD_PARSE_NO_STATE  0x080000        /**< Forbid state data in the parsed data. Usually used with ::LYD_VALIDATE_NO_STATE. */
#define LYD_PARSE_LYB_MOD_UPDATE  0x100000  /**< Only for LYB format, allow parsing data printed using a specific module
                                                 revision to be loaded even with a module with the same name but newer
                                                 revision. */
#define LYD_PARSE_ORDERED 0x200000          /**< Do not search for the correct place of each node but instead expect
                                                 that the nodes are being parsed in the correct schema-based order,
                                                 which is always true if the data were printed by libyang and not
                                                 modified manually. If this flag is used incorrectly (for unordered data),
                                                 the behavior is undefined and most functions executed with these
                                                 data will not work correctly. */
#define LYD_PARSE_SUBTREE 0x400000          /**< Parse only the first child item along with any descendants, but no
                                                 siblings. This flag is not required when parsing data which do not
                                                 start at the schema root; for that purpose, use lyd_parse_data's parent
                                                 argument.
                                                 Also, a new return value ::LY_ENOT is returned if there is a sibling
                                                 subtree following in the input data. Note that if validation is requested,
                                                 only the newly parsed subtree is validated. This might result in
                                                 an invalid datastore content. */
#define LYD_PARSE_WHEN_TRUE 0x800000        /**< Mark all the parsed nodes dependend on a when condition with the flag
                                                 that means the condition was satisifed before. This allows for
                                                 auto-deletion of these nodes during validation. */
#define LYD_PARSE_NO_NEW 0x1000000          /**< Do not set ::LYD_NEW (non-validated node flag) for any nodes. Use
                                                 when parsing validated data to skip some validation tasks and modify
                                                 some validation behavior (auto-deletion of cases). */
#define LYD_PARSE_STORE_ONLY 0x2010000      /**< Similar to ::LYD_PARSE_ONLY but even type value restrictions will not
                                                 be checked (length, range, pattern, ...) and if a value can be stored,
                                                 it is. Calling separate validation on these data always checks all the
                                                 restrictions as well. */
#define LYD_PARSE_JSON_NULL 0x4000000       /**< Allow using JSON empty value 'null' within JSON input, such nodes are
                                                 silently skipped and treated as non-existent. By default, such values
                                                 are invalid. */
#define LYD_PARSE_JSON_STRING_DATATYPES 0x8000000 /**< By default, JSON data values are expected to be in the correct
                                                       format according to RFC 7951 based on their type. Using this
                                                       option the validation can be softened to accept boolean and
                                                       number type values enclosed in quotes. */
#define LYD_PARSE_OPTS_MASK 0xFFFF0000      /**< Mask for all the LYD_PARSE_ options. */

/** @} dataparseroptions */

/**
 * @ingroup datatree
 * @defgroup datavalidationoptions Data validation options
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
#define LYD_VALIDATE_NO_STATE   0x0001      /**< Consider state data not allowed and raise an error if they are found.
                                                 Also, no implicit state data are added. */
#define LYD_VALIDATE_PRESENT    0x0002      /**< Validate only modules whose data actually exist. */
#define LYD_VALIDATE_MULTI_ERROR 0x0004     /**< Do not stop validation on the first error but generate all the detected errors. */
#define LYD_VALIDATE_OPERATIONAL 0x0008     /**< Semantic constraint violations are reported only as warnings instead of
                                                 errors (see [RFC 8342 sec. 5.3](https://datatracker.ietf.org/doc/html/rfc8342#section-5.3)). */
#define LYD_VALIDATE_NO_DEFAULTS 0x0010     /**< Do not add any default nodes during validation, other implicit nodes
                                                 (such as NP containers) are still added. Validation will fail if a
                                                 default node is required for it to pass. */
#define LYD_VALIDATE_NOT_FINAL 0x0020       /**< Skip final validation tasks that require for all the data nodes to
                                                 either exist or not, based on the YANG constraints. Once the data
                                                 satisfy this requirement, the final validation should be performed. */

#define LYD_VALIDATE_OPTS_MASK  0x0000FFFF  /**< Mask for all the LYD_VALIDATE_* options. */

/** @} datavalidationoptions */

/**
 * @brief Parse (and validate) data from the input handler as a YANG data tree.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] parent Optional parent to connect the parsed nodes to. If provided, the data are expected to describe
 * a subtree of the YANG model instead of starting at the schema root.
 * @param[in] in The input handle to provide the dumped data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed. Can be 0 to try to detect format from the input handler.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Full parsed data tree, note that NULL can be a valid tree. If @p parent is set, the first parsed child.
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 *
 * When parsing subtrees (i.e., when @p parent is non-NULL), validation is only performed on the newly parsed data.
 * This might result in allowing invalid datastore content when the schema contains cross-branch constraints,
 * complicated `must` statements, etc. When a full-datastore validation is desirable, parse all subtrees
 * first, and then request validation of the complete datastore content.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_data(const struct ly_ctx *ctx, struct lyd_node *parent, struct ly_in *in, LYD_FORMAT format,
        uint32_t parse_options, uint32_t validate_options, struct lyd_node **tree);

/**
 * @brief Parse (and validate) input data as a YANG data tree.
 *
 * Wrapper around ::lyd_parse_data() hiding work with the input handler and some obscure options.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] data The input data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Full parsed data tree, note that NULL can be a valid tree
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_data_mem(const struct ly_ctx *ctx, const char *data, LYD_FORMAT format, uint32_t parse_options,
        uint32_t validate_options, struct lyd_node **tree);

/**
 * @brief Parse (and validate) input data as a YANG data tree.
 *
 * Wrapper around ::lyd_parse_data() hiding work with the input handler and some obscure options.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the input data in the
 * specified @p format to parse.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Full parsed data tree, note that NULL can be a valid tree
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_data_fd(const struct ly_ctx *ctx, int fd, LYD_FORMAT format, uint32_t parse_options,
        uint32_t validate_options, struct lyd_node **tree);

/**
 * @brief Parse (and validate) input data as a YANG data tree.
 *
 * Wrapper around ::lyd_parse_data() hiding work with the input handler and some obscure options.
 *
 * @param[in] ctx Context to connect with the tree being built here.
 * @param[in] path Path to the file with the input data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed. Can be 0 to try to detect format from @p path extension.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Full parsed data tree, note that NULL can be a valid tree
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_data_path(const struct ly_ctx *ctx, const char *path, LYD_FORMAT format,
        uint32_t parse_options, uint32_t validate_options, struct lyd_node **tree);

/**
 * @brief Parse (and validate) data from the input handler as an extension data tree following the schema tree of the given
 * extension instance.
 *
 * Note that the data being parsed are limited only to the schema tree specified by the given extension, it does not allow
 * to mix them with the standard data from any module.
 *
 * Directly applicable to data defined as [yang-data](@ref howtoDataYangdata).
 *
 * @param[in] ext Extension instance providing the specific schema tree to match with the data being parsed.
 * @param[in] parent Optional parent to connect the parsed nodes to.
 * @param[in] in The input handle to provide the dumped data in the specified @p format to parse (and validate).
 * @param[in] format Format of the input data to be parsed. Can be 0 to try to detect format from the input handler.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] tree Full parsed data tree, note that NULL can be a valid tree. If @p parent is set, set to NULL.
 * @return LY_SUCCESS in case of successful parsing (and validation).
 * @return LY_ERR value in case of error. Additional error information can be obtained from the context using ly_err* functions.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_ext_data(const struct lysc_ext_instance *ext, struct lyd_node *parent, struct ly_in *in,
        LYD_FORMAT format, uint32_t parse_options, uint32_t validate_options, struct lyd_node **tree);

/**
 * @ingroup datatree
 * @defgroup datatype Data operation type
 *
 * Operation provided to ::lyd_validate_op() to validate.
 *
 * The operation cannot be determined automatically since RPC/action and a reply to it share the common top level node
 * referencing the RPC/action schema node and may not have any input/output children to use for distinction.
 *
 * @{
 */
enum lyd_type {
    LYD_TYPE_DATA_YANG = 0,     /**< generic YANG instance data */
    LYD_TYPE_RPC_YANG,          /**< instance of a YANG RPC/action request with only "input" data children,
                                     including all parents and optional top-level "action" element in case of an action */
    LYD_TYPE_NOTIF_YANG,        /**< instance of a YANG notification, including all parents in case of a nested one */
    LYD_TYPE_REPLY_YANG,        /**< instance of a YANG RPC/action reply with only "output" data children,
                                     including all parents in case of an action */

    LYD_TYPE_RPC_NETCONF,       /**< complete NETCONF RPC invocation as defined for
                                     [RPC](https://tools.ietf.org/html/rfc7950#section-7.14.4) and
                                     [action](https://tools.ietf.org/html/rfc7950#section-7.15.2) */
    LYD_TYPE_NOTIF_NETCONF,     /**< complete NETCONF notification message as defined for
                                     [notification](https://tools.ietf.org/html/rfc7950#section-7.16.2) */
    LYD_TYPE_REPLY_NETCONF,     /**< complete NETCONF RPC reply as defined for
                                     [RPC](https://tools.ietf.org/html/rfc7950#section-7.14.4) and
                                     [action](https://tools.ietf.org/html/rfc7950#section-7.15.2) */

    LYD_TYPE_RPC_RESTCONF,      /**< message-body of a RESTCONF operation input parameters
                                     ([ref](https://www.rfc-editor.org/rfc/rfc8040.html#section-3.6.1)) */
    LYD_TYPE_NOTIF_RESTCONF,    /**< RESTCONF JSON notification data
                                     ([ref](https://www.rfc-editor.org/rfc/rfc8040.html#section-6.4)), to parse
                                     a notification in XML, use ::LYD_TYPE_NOTIF_NETCONF */
    LYD_TYPE_REPLY_RESTCONF     /**< message-body of a RESTCONF operation output parameters
                                     ([ref](https://www.rfc-editor.org/rfc/rfc8040.html#section-3.6.2)) */
};
/** @} datatype */

/**
 * @brief Parse YANG data into an operation data tree. Specific parsing flags ::LYD_PARSE_ONLY, ::LYD_PARSE_STRICT and
 * no validation flags are used.
 *
 * At least one of @p parent, @p tree, or @p op must always be set.
 *
 * Specific @p data_type values have different parameter meaning as follows:
 * - ::LYD_TYPE_RPC_NETCONF:
 *   - @p parent - must be NULL, the whole RPC is expected;
 *   - @p format - must be ::LYD_XML, NETCONF supports only this format;
 *   - @p tree - must be provided, all the NETCONF-specific XML envelopes will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be provided, the RPC/action data tree itself will be returned here, pointing to the operation;
 *
 * - ::LYD_TYPE_NOTIF_NETCONF:
 *   - @p parent - must be NULL, the whole notification is expected;
 *   - @p format - must be ::LYD_XML, NETCONF supports only this format;
 *   - @p tree - must be provided, all the NETCONF-specific XML envelopes will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be provided, the notification data tree itself will be returned here, pointing to the operation;
 *
 * - ::LYD_TYPE_REPLY_NETCONF:
 *   - @p parent - must be set, pointing to the invoked RPC operation (RPC or action) node;
 *   - @p format - must be ::LYD_XML, NETCONF supports only this format;
 *   - @p tree - must be provided, all the NETCONF-specific XML envelopes will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be NULL, the reply is appended to the RPC;
 *   Note that there are 3 kinds of NETCONF replies - ok, error, and data. Only data reply appends any nodes to the RPC.
 *
 * - ::LYD_TYPE_RPC_RESTCONF:
 *   - @p parent - must be set, pointing to the invoked RPC operation (RPC or action) node;
 *   - @p format - can be both ::LYD_JSON and ::LYD_XML;
 *   - @p tree - must be provided, all the RESTCONF-specific JSON objects will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be NULL, @p parent points to the operation;
 *
 * - ::LYD_TYPE_NOTIF_RESTCONF:
 *   - @p parent - must be NULL, the whole notification is expected;
 *   - @p format - must be ::LYD_JSON, XML-formatted notifications are parsed using ::LYD_TYPE_NOTIF_NETCONF;
 *   - @p tree - must be provided, all the RESTCONF-specific JSON objects will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be provided, the notification data tree itself will be returned here, pointing to the operation;
 *
 * - ::LYD_TYPE_REPLY_RESTCONF:
 *   - @p parent - must be set, pointing to the invoked RPC operation (RPC or action) node;
 *   - @p format - can be both ::LYD_JSON and ::LYD_XML;
 *   - @p tree - must be provided, all the RESTCONF-specific JSON objects will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be NULL, @p parent points to the operation;
 *   Note that error reply should be parsed as 'yang-data' extension data.
 *
 * - ::LYD_TYPE_RPC_YANG:
 * - ::LYD_TYPE_NOTIF_YANG:
 * - ::LYD_TYPE_REPLY_YANG:
 *   - all the parameters have their default meaning.
 *
 * - :: LYD_TYPE_DATA_YANG:
 *   - not accepted by the function.
 *
 * @param[in] ctx libyang context.
 * @param[in] parent Optional parent to connect the parsed nodes to.
 * @param[in] in Input handle to read the input from.
 * @param[in] format Expected format of the data in @p in.
 * @param[in] data_type Expected operation to parse (@ref datatype).
 * @param[out] tree Optional full parsed data tree. If @p parent is set, set to NULL.
 * @param[out] op Optional pointer to the operation (action/RPC/notification) node.
 * @return LY_ERR value.
 * @return LY_ENOT if @p data_type is a NETCONF message and the root XML element is not the expected one.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_op(const struct ly_ctx *ctx, struct lyd_node *parent, struct ly_in *in, LYD_FORMAT format,
        enum lyd_type data_type, struct lyd_node **tree, struct lyd_node **op);

/**
 * @brief Parse extension data into an operation data tree following only the specification from the given extension instance.
 *
 * Directly applicable to data defined as [yang-data](@ref howtoDataYangdata).
 *
 * At least one of @p parent, @p tree, or @p op must always be set.
 *
 * Specific @p data_type values have different parameter meaning as follows:
 * - ::LYD_TYPE_RPC_NETCONF:
 *   - @p parent - must be NULL, the whole RPC is expected;
 *   - @p format - must be ::LYD_XML, NETCONF supports only this format;
 *   - @p tree - must be provided, all the NETCONF-specific XML envelopes will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be provided, the RPC/action data tree itself will be returned here, pointing to the operation;
 *
 * - ::LYD_TYPE_NOTIF_NETCONF:
 *   - @p parent - must be NULL, the whole notification is expected;
 *   - @p format - must be ::LYD_XML, NETCONF supports only this format;
 *   - @p tree - must be provided, all the NETCONF-specific XML envelopes will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be provided, the notification data tree itself will be returned here, pointing to the operation;
 *
 * - ::LYD_TYPE_REPLY_NETCONF:
 *   - @p parent - must be set, pointing to the invoked RPC operation (RPC or action) node;
 *   - @p format - must be ::LYD_XML, NETCONF supports only this format;
 *   - @p tree - must be provided, all the NETCONF-specific XML envelopes will be returned here as
 *               a separate opaque data tree, even if the function fails, this may be returned;
 *   - @p op - must be NULL, the reply is appended to the RPC;
 *   Note that there are 3 kinds of NETCONF replies - ok, error, and data. Only data reply appends any nodes to the RPC.
 *
 * @param[in] ext Extension instance providing the specific schema tree to match with the data being parsed.
 * @param[in] parent Optional parent to connect the parsed nodes to.
 * @param[in] in Input handle to read the input from.
 * @param[in] format Expected format of the data in @p in.
 * @param[in] data_type Expected operation to parse (@ref datatype).
 * @param[out] tree Optional full parsed data tree. If @p parent is set, set to NULL.
 * @param[out] op Optional pointer to the operation (action/RPC) node.
 * @return LY_ERR value.
 * @return LY_ENOT if @p data_type is a NETCONF message and the root XML element is not the expected one.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_ext_op(const struct lysc_ext_instance *ext, struct lyd_node *parent, struct ly_in *in,
        LYD_FORMAT format, enum lyd_type data_type, struct lyd_node **tree, struct lyd_node **op);

/**
 * @brief Fully validate a data tree.
 *
 * The data tree is modified in-place. As a result of the validation, some data might be removed
 * from the tree. In that case, the removed items are freed, not just unlinked.
 *
 * @param[in,out] tree Data tree to recursively validate. May be changed by validation, might become NULL.
 * @param[in] ctx libyang context. Can be NULL if @p tree is set.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LIBYANG_API_DECL LY_ERR lyd_validate_all(struct lyd_node **tree, const struct ly_ctx *ctx, uint32_t val_opts,
        struct lyd_node **diff);

/**
 * @brief Fully validate a data tree of a module.
 *
 * The data tree is modified in-place. As a result of the validation, some data might be removed
 * from the tree. In that case, the removed items are freed, not just unlinked.
 *
 * If several modules need to be validated, the flag ::LYD_VALIDATE_NOT_FINAL should be used first for validation
 * of each module and then ::lyd_validate_module_final() should be called also for each module. Otherwise,
 * false-positive validation errors for foreign dependencies may occur.
 *
 * @param[in,out] tree Data tree to recursively validate. May be changed by validation, might become NULL.
 * @param[in] module Module whose data (and schema restrictions) to validate.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LIBYANG_API_DECL LY_ERR lyd_validate_module(struct lyd_node **tree, const struct lys_module *module, uint32_t val_opts,
        struct lyd_node **diff);

/**
 * @brief Finish validation of a module data that have previously been validated with ::LYD_VALIDATE_NOT_FINAL flag.
 *
 * This final validation will not add or remove any nodes.
 *
 * @param[in] tree Data tree to recursively validate.
 * @param[in] module Module whose data (and schema restrictions) to validate.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LIBYANG_API_DECL LY_ERR lyd_validate_module_final(struct lyd_node *tree, const struct lys_module *module,
        uint32_t val_opts);

/**
 * @brief Validate an RPC/action request, reply, or notification. Only the operation data tree (input/output/notif)
 * is validate, any parents are ignored.
 *
 * @param[in,out] op_tree Operation tree with any parents. It can point to the operation itself or any of
 * its parents, only the operation subtree is actually validated.
 * @param[in] dep_tree Tree to be used for validating references from the operation subtree.
 * @param[in] data_type Operation type to validate (only YANG operations are accepted, @ref datatype).
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LIBYANG_API_DECL LY_ERR lyd_validate_op(struct lyd_node *op_tree, const struct lyd_node *dep_tree, enum lyd_type data_type,
        struct lyd_node **diff);

/** @} datatree */

#ifdef __cplusplus
}
#endif

#endif /* LY_PARSER_DATA_H_ */
