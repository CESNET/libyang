/**
 * @file libyang.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief The main libyang public header.
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

#ifndef LY_LIBYANG_H_
#define LY_LIBYANG_H_

#include <stdio.h>

#include "tree_schema.h"
#include "tree_data.h"
#include "xml.h"
#include "dict.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page howto How To ...
 *
 * - @subpage howtocontext
 * - @subpage howtoschemas
 * - @subpage howtodata
 * - @subpage howtologger
 */

/** @page howtocontext Context
 *
 * The context concept allows callers to work in environments with different sets of YANG schemas.
 *
 * The first step in libyang is to create a new context using ly_ctx_new(). It returns a handler
 * used in the following work.
 *
 * When creating a new context, search dir can be specified (NULL is accepted) to provide directory
 * where libyang will automatically search for schemas being imported or included. The search path
 * can be later changed via ly_ctx_set_searchdir() function. Before exploring the specified search
 * dir, libyang tries to get imported and included schemas from the current working directory first.
 *
 * Schemas are added into the context using [parser functions](@ref parsers) - lys_parse() or lys_read().
 * Note, that parser functions for schemas have \b lys_ prefix while instance data parser functions have
 * \b lyd_ prefix.
 *
 * Context can hold multiple revisons of the same schema.
 *
 * Context holds all modules and their submodules internally. The list of available module names is
 * provided via ly_ctx_get_module_names() functions. Similarly, caller can get also a list of submodules
 * names of a specific module using ly_ctx_get_submodule_names() function. The returned names can be
 * subsequently used to get the (sub)module structures using ly_ctx_get_module() and ly_ctx_get_submodule().
 *
 * Modules held by a context cannot be removed one after one. The only way how to \em change modules in the
 * context is to create a new context and remove the old one. To remove a context, there is ly_ctx_destroy()
 * function.
 *
 * \note API for this group of functions is available in the [context module](@ref context).
 *
 */

/**
 * @page howtoschemas Schemas
 *
 * Schema is an internal libyang's representation of a YANG data model. Each schema is connected with
 * its [context](@ref howtocontext) and loaded using [parser functions](@ref parsers). It means, that
 * the schema cannot be created (nor changed) programatically. In libyang, schemas are used only to
 * access data model definitions.
 *
 * \note There are many functions to access information from the schema trees. Details are available in
 * the [Schema Tree module](@ref schematree).
 *
 * YANG Features Manipulation
 * --------------------------
 *
 * The group of functions prefixed by \b lys_features_ are used to access and manipulate with the schema's
 * features.
 *
 * The first two functions are used to access information about the features in the schema.
 * lys_features_list() provides list of all features defined in the specific schema and its
 * submodules. Optionally, it can also provides information about the state of all features.
 * Alternatively, caller can use lys_features_state() function to get state of one specific
 * feature.
 *
 * The remaining two functions, lys_features_enable() and lys_features_disable(), are used
 * to enable and disable the specific feature. By default, when the module is loaded by libyang
 * parser, all features are disabled.
 *
 * Note, that feature's state can affect some of the output formats (e.g. Tree format).
 *
 */

/**
 * @page howtodata Data Instances
 *
 */

/**
 *
 * @page howtologger Logger
 *
 * There are 4 verbosity levels defined as ::LY_LOG_LEVEL. The level can be
 * changed by the ly_verb() function. By default, the verbosity level is
 * set to #LY_LLERR value.
 *
 * In case the logger has an error message (LY_LLERR) to print, also an error
 * code is recorded in extern ly_errno variable. Possible values are of type
 * ::LY_ERR.
 *
 * \note API for this group of functions is available in the [logger module](@ref logger).
 */

/**
 * @defgroup context Context
 * @{
 *
 * Structures and functions to manipulate with the libyang "containers". The \em context concept allows callers
 * to work in environments with different sets of YANG schemas. More detailed information can be found at
 * @ref howtocontext page.
 */

/**
 * @brief libyang context handler.
 */
struct ly_ctx;

/**
 * @brief Create libyang context
 *
 * Context is used to hold all information about schemas. Usually, the application is supposed
 * to work with a single context in which libyang is holding all schemas (and other internal
 * information) according to which the data trees will be processed and validated. So, the schema
 * trees are tightly connected with the specific context and they are held by the context internally
 * - caller does not need to keep pointers to the schemas returned by lys_parse(), context knows
 * about them. The data trees created with lyd_parse() are still connected with the specific context,
 * but they are not internally held by the context. The data tree just points and lean on some data
 * held by the context (schema tree, string dictionary, etc.). Therefore, in case of data trees, caller
 * is supposed to keep pointers returned by the lyd_parse() and manage the data tree on its own. This
 * also affects the number of instances of both tree types. While you can have only one instance of
 * specific schema connected with a single context, number of data tree instances is not connected.
 *
 * @param[in] search_dir Directory where libyang will search for the imported or included modules
 * and submodules. If no such directory is available, NULL is accepted.
 *
 * @return Pointer to the created libyang context, NULL in case of error.
 */
struct ly_ctx *ly_ctx_new(const char *search_dir);

/**
 * @brief Change the search path in libyang context
 *
 * @param[in] ctx Context to be modified.
 * @param[in] search_dir New search path to replace the current one in ctx.
 */
void ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir);

/**
 * @brief Get current value of the search path in libyang context
 *
 * @param[in] ctx Context to query.
 * @return Current value of the search path.
 */
const char *ly_ctx_get_searchdir(struct ly_ctx *ctx);

/**
 * @brief Get data of an internal ietf-yang-library module.
 *
 * @param[in] ctx Context with the modules.
 * @return Root data node corresponding to the model, NULL on error.
 * Caller is responsible for freeing the returned data tree using lyd_free().
 */
struct lyd_node *ly_ctx_info(struct ly_ctx *ctx);

/**
 * @brief Get the names of the loaded modules.
 *
 * @param[in] ctx Context with the modules.
 * @return NULL-terminated array of the module names,
 * NULL on error. The returned array must be freed by the caller, do not free
 * names in the array. Also remember that the names will be freed with freeing
 * the context.
 */
const char **ly_ctx_get_module_names(struct ly_ctx *ctx);

/**
 * @brief Get the names of the loaded submodules of the specified module.
 *
 * @param[in] ctx Context with the modules.
 * @param[in] module_name Name of the parent module.
 * @return NULL-terminated array of submodule names of the parent module,
 * NULL on error. The returned array must be freed by the caller, do not free
 * names in the array. Also remember that the names will be freed with freeing
 * the context.
 */
const char **ly_ctx_get_submodule_names(struct ly_ctx *ctx, const char *module_name);

/**
 * @brief Get pointer to the schema tree of the module of the specified name.
 *
 * @param[in] ctx Context to work in.
 * @param[in] name Name of the YANG module to get.
 * @param[in] revision Optional revision date of the YANG module to get. If not specified,
 * the schema in the newest revision is returned if any.
 * @return Pointer to the data model structure, NULL if no schema following the name and
 * revision requirements is present in the context.
 */
struct lys_module *ly_ctx_get_module(struct ly_ctx *ctx, const char *name, const char *revision);

/**
 * @brief Get pointer to the schema tree of the module of the specified namespace
 *
 * @param[in] ctx Context to work in.
 * @param[in] ns Namespace of the YANG module to get.
 * @param[in] revision Optional revision date of the YANG module to get. If not specified,
 * the schema in the newest revision is returned if any.
 * @return Pointer to the data model structure, NULL if no schema following the namespace and
 * revision requirements is present in the context.
 */
struct lys_module *ly_ctx_get_module_by_ns(struct ly_ctx *ctx, const char *ns, const char *revision);

/**
 * @brief Get submodule from the context's search dir.
 *
 * @param[in] module Parent (belongs-to) module.
 * @param[in] name Name of the YANG submodule to get.
 * @param[in] revision Optional revision date of the YANG submodule to get. If
 * not specified, the newest revision is returned (TODO).
 * @return Pointer to the data model structure.
 */
struct lys_submodule *ly_ctx_get_submodule(struct lys_module *module, const char *name, const char *revision);

/**
 * @brief Get schema node according to the given absolute schema node identifier.
 *
 * @param[in] ctx Context to work in.
 * @param[in] nodeid Absolute schema node identifier with module names used as
 * prefixes. Prefix (module name) must be used whenever the child node is from
 * other module (augments the parent node). The first node in the path must be
 * always specified with the prefix. Here are some examples:
 *
 * - /ietf-netconf-monitoring:get-schema/input/identifier
 * - /ietf-interfaces:interfaces/interface/ietf-ip:ipv4/address/ip
 */
struct lys_node *ly_ctx_get_node(struct ly_ctx *ctx, const char *nodeid);

/**
 * @brief Free all internal structures of the specified context.
 *
 * The function should be used before terminating the application to destroy
 * and free all structures internally used by libyang. If the caller uses
 * multiple contexts, the function should be called for each used context.
 *
 * All instance data are supposed to be freed before destroying the context.
 * Data models are destroyed automatically as part of ly_ctx_destroy() call.
 *
 * @param[in] ctx libyang context to destroy
 */
void ly_ctx_destroy(struct ly_ctx *ctx);

/**@} context */

/**
 * @defgroup parsers Parsers
 * @{
 *
 * Parsers allows to read schema and data trees from a specific format.
 *
 * For schemas, the following formats are supported:
 * - YANG
 *
 *   Basic YANG schemas format described in [RFC 6020](http://tools.ietf.org/html/rfc6020).
 *   Currently, only YANG 1.0 is supported.
 *
 *   \todo YANG input is not yet implemented
 *
 * - YIN
 *
 *   Alternative XML-based format to YANG. The details can be found in
 *   [RFC 6020](http://tools.ietf.org/html/rfc6020#section-11).
 *
 * For data instances, the following formats are supported:
 * - \todo TBD
 *
 */

/**
 * @brief Load a schema into the specified context.
 *
 * LY_IN_YANG (YANG) format is not yet supported.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_parse(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format);

/**
 * @brief Read a schema from file into the specified context.
 *
 * LY_IN_YANG (YANG) format is not yet supported.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd The standard file descriptor of the file containing the schema in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct lys_module *lys_read(struct ly_ctx *ctx, int fd, LYS_INFORMAT format);

/**
 * @defgroup parseroptions Data parser options
 * @ingroup parsers
 *
 * Various options to change the data tree parsers behavior.
 *
 * By default, parser silently ignores the data without a matching node in schema trees. If the caller want to stop
 * parsing in case of presence of unknown data, the #LYD_OPT_STRICT can be used. The strict mode is useful for
 * NETCONF servers, since NETCONF clients should always send data according to the capabilities announced by the server.
 * On the other hand, the default non-strict mode is useful for clients receiving data from NETCONF server since
 * clients are not required to understand everything the server does. Of course, the optimal strategy is to use
 * filtering to get only the required data.
 *
 * Parser also expects that the provided data are complete and performs data validation according to all
 * implemented YANG rules. This can be problem in case of representing NETCONF's subtree filter data or
 * edit-config's data which do not represent a complete data set. Therefore there are two options to make parser to
 * accept such a data: #LYD_OPT_FILTER and #LYD_OPT_EDIT. However, both these options are ignored when parsing
 * an RPC or a notification.
 *
 * @{
 */
#define LYD_OPT_STRICT   0x01  /**< instead of silent ignoring data without schema definition, raise an error.
                                    Having an unknown element of the known namespace is always an error. */
#define LYD_OPT_EDIT     0x02  /**< make validation to accept NETCONF edit-config's content:
                                    - mandatory nodes can be omitted
                                    - leafrefs and instance-identifier are not resolved
                                    - status data are not allowed */
#define LYD_OPT_FILTER   0x04  /**< make validation to accept NETCONF subtree filter data:
                                    - leafs/leaf-lists with no data are allowed (even not allowed e.g. by length restriction)
                                    - multiple instances of container/leaf/.. are allowed
                                    - list's keys are not required
                                    - mandatory nodes can be omitted
                                    - leafrefs and instance-identifier are not resolved
                                    - data from different choice's branches are allowed */
/**
 * @}
 */

/**
 * @brief Parse (and validate according to appropriate schema from the given context) data.
 *
 * In case of LY_XML format, the data string is expected to contain XML data under a single
 * XML element. The element is not parsed, but it is expected to keep XML data well formed in all
 * cases. There are no restrictions for the element name or its namespace.
 *
 * LY_JSON format is not yet supported.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] data Serialized data in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_parse(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options);

/**
 * @brief Parse (and validate according to appropriate schema from the given context) XML tree.
 *
 * The output data tree is parsed from the given XML tree previously parsed by one of the
 * lyxml_read* functions. Note, that the parser removes successfully parsed data from the
 * XML tree except the root element (see the note about XML format in lyd_parse()). When
 * the given XML tree is successfully parsed, the given \p root is kept but it has no children
 * which are returned as a top level nodes in the output data tree.
 *
 * The context must be the same as the context used to parse XML tree by lyxml_read* function.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] root XML tree to parse (convert) to data tree.
 * @param[in] options Parser options, see @ref parseroptions.
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_parse_xml(struct ly_ctx *ctx, struct lyxml_elem *root, int options);

/**
 * @brief Read data from the given file
 *
 * TODO not implemented
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] fd The standard file descriptor of the file containing the data tree in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_read(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options);

/**@} parsers */

/**
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 *
 * @}
 */

/**
 * @defgroup datatree Data Tree
 * @{
 *
 * Data structures and functions to manipulate and access instance data tree.
 *
 * @}
 */

/**
 * @defgroup printers Printers
 * @{
 *
 * Printers allows to serialize schema and data trees in a specific format.
 *
 * For schemas, the following formats are supported:
 * - YANG
 *
 *   Basic YANG schemas format described in [RFC 6020](http://tools.ietf.org/html/rfc6020).
 *   Currently, only YANG 1.0 is supported.
 *
 * - YIN
 *
 *   Alternative XML-based format to YANG. The details can be found in
 *   [RFC 6020](http://tools.ietf.org/html/rfc6020#section-11).
 *
 *   \todo YIN output is not yet implemented
 *
 * - Tree
 *
 *   Simple tree structure of the module.
 *
 * - Info
 *
 *   Detailed information about the specific node in the schema tree.
 *   The target can be more specific than the module itself:
 *
 *   - absolute-schema-nodeid&nbsp;&nbsp;&nbsp;&nbsp;\a /modules/module-set-id in \a ietf-yang-library
 *   - <b>typedef/</b>typedef-name&nbsp;&nbsp;&nbsp;&nbsp;\a typedef/revision-identifier in \a ietf-yang-library
 *   - <b>feature/</b>feature-name&nbsp;&nbsp;&nbsp;&nbsp;\a feature/ssh in \a ietf-netconf-server
 *   - <b>grouping/</b>grouping-name/descendant-schema-nodeid&nbsp;&nbsp;&nbsp;&nbsp;\a grouping/module or \a grouping/module/module/submodules
 *     in \a ietf-yang-library
 *   - <b>type/</b>leaf-or-leaflist&nbsp;&nbsp;&nbsp;&nbsp;\a type/modules/module-set-id in \a ietf-yang-library
 *
 * For data instances, the following formats are supported:
 * - \todo TBD
 *
 */

/**
 * @brief Print schema tree in the specified format.
 *
 * To write data into a file descriptor, use lys_print_fd().
 *
 * @param[in] module Schema tree to print.
 * @param[in] f File stream where to print the schema.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print(FILE *f, struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @brief Print schema tree in the specified format.
 *
 * Same as lys_print(), but output is written into the specified file descriptor.
 *
 * @param[in] module Schema tree to print.
 * @param[in] fd File descriptor where to print the data.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_fd(int fd, struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @brief Print schema tree in the specified format.
 *
 * Same as lys_print(),  but it allocates memory and store the data into it.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_mem(char **strp, struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @brief Print schema tree in the specified format.
 *
 * Same as lys_print(), but output is written via provided callback.
 *
 * @param[in] module Schema tree to print.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LYS_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @brief Print data tree in the specified format.
 *
 * To write data into a file descriptor, use lyd_print_fd().
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] f File stream where to print the data.
 * @param[in] format Data output format.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print(FILE *f, struct lyd_node *root, LYD_FORMAT format);

/**
 * @brief Print data tree in the specified format.
 *
 * Same as lyd_print(), but output is written into the specified file descriptor.
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] fd File descriptor where to print the data.
 * @param[in] format Data output format.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_fd(int fd, struct lyd_node *root, LYD_FORMAT format);


 /**
 * @brief Print data tree in the specified format.
 *
 * Same as lyd_print(), but it allocates memory and store the data into it.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] format Data output format.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_mem(char **strp, struct lyd_node *root, LYD_FORMAT format);

/**
 * @brief Print data tree in the specified format.
 *
 * Same as lyd_print(), but output is written via provided callback.
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Data output format.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, struct lyd_node *root, LYD_FORMAT format);

/**@} printers */

/**
 * @defgroup logger Logger
 * @{
 *
 * Publicly visible functions and values of the libyang logger. For more
 * information, see \ref howtologger.
 */

/**
 * @typedef LY_LOG_LEVEL
 * @brief Verbosity levels of the libyang logger.
 */
typedef enum {
    LY_LLERR,      /**< Print only error messages. */
    LY_LLWRN,      /**< Print error and warning messages. */
    LY_LLVRB,      /**< Besides errors and warnings, print some other verbose messages. */
    LY_LLDBG       /**< Print all messages including some development debug messages. */
} LY_LOG_LEVEL;

/**
 * @brief Set logger verbosity level.
 * @param[in] level Verbosity level.
 */
void ly_verb(LY_LOG_LEVEL level);

/**
 * @typedef LY_ERR
 * @brief libyang's error codes available via ly_errno extern variable.
 * @ingroup logger
 */
typedef enum {
    LY_SUCCESS,    /**< no error, not set by functions, included just to complete #LY_ERR enumeration */
    LY_EMEM,       /**< Memory allocation failure */
    LY_ESYS,       /**< System call failure */
    LY_EINVAL,     /**< Invalid value */
    LY_EINT,       /**< Internal error */
    LY_EVALID      /**< Validation failure */
} LY_ERR;
/**
 * @brief libyang specific errno.
 */
extern LY_ERR ly_errno;

/**@} logger */

#ifdef __cplusplus
}
#endif

#endif /* LY_LIBYANG_H_ */
