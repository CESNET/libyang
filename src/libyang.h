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
const char *ly_ctx_get_searchdir(const struct ly_ctx *ctx);

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
const char **ly_ctx_get_module_names(const struct ly_ctx *ctx);

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
const char **ly_ctx_get_submodule_names(const struct ly_ctx *ctx, const char *module_name);

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
const struct lys_module *ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision);

/**
 * @brief Try to find the model in the searchpath of \p ctx and load it into it. If custom missing
 * module callback is set, it is used instead.
 *
 * @param[in] ctx Context to add to.
 * @param[in] name Name of the module to load.
 * @param[in] revision Optional revision date of the module. If not specified, it is
 * assumed that there is only one model revision in the searchpath (the first matching file
 * is parsed).
 * @return Pointer to the data model structure, NULL if not found or some error occured.
 */
const struct lys_module *ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision);

/**
 * @brief Callback for retrieving missing included or imported models in a custom way.
 *
 * @param[in] name Missing module name.
 * @param[in] revision Optional missing module revision.
 * @param[in] user_data User-supplied callback data.
 * @param[out] format Format of the returned module data.
 * @param[out] free_module_data Optional callback for freeing the returned module data. If not set, free() is used.
 * @return Requested module data or NULL on error.
 */
typedef char *(*ly_module_clb)(const char *name, const char *revision, void *user_data, LYS_INFORMAT *format,
                               void (**free_module_data)(char *model_data));

/**
 * @brief Set missing include or import model callback.
 *
 * @param[in] ctx Context that will use this callback.
 * @param[in] clb Callback responsible for returning a missing model.
 * @param[in] user_data Arbitrary data that will always be passed to the callback \p clb.
 */
void ly_ctx_set_module_clb(struct ly_ctx *ctx, ly_module_clb clb, void *user_data);

/**
 * @brief Get the custom callback for missing module retrieval.
 *
 * @param[in] ctx Context to read from.
 * @param[in] user_data Optional pointer for getting the user-supplied callbck data.
 * @return Custom user missing module callback or NULL if not set.
 */
ly_module_clb ly_ctx_get_module_clb(const struct ly_ctx *ctx, void **user_data);

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
const struct lys_module *ly_ctx_get_module_by_ns(const struct ly_ctx *ctx, const char *ns, const char *revision);

/**
 * @brief Get submodule from the context's search dir.
 *
 * @param[in] module Parent (belongs-to) module.
 * @param[in] name Name of the YANG submodule to get.
 * @param[in] revision Optional revision date of the YANG submodule to get. If
 * not specified, the newest revision is returned (TODO).
 * @return Pointer to the data model structure.
 */
const struct lys_submodule *ly_ctx_get_submodule(const struct lys_module *module, const char *name,
                                                 const char *revision);

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
const struct lys_module *lys_parse_data(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format);

/**
 * @brief Load a schema into the specified context from a file.
 *
 * LY_IN_YANG (YANG) format is not yet supported.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
const struct lys_module *lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format);

/**
 * @brief Read a schema from file descriptor into the specified context.
 *
 * LY_IN_YANG (YANG) format is not yet supported.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
const struct lys_module *lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format);

/**
 * @defgroup parseroptions Data parser options
 * @ingroup parsers
 *
 * Various options to change the data tree parsers behavior.
 *
 * Default behavior:
 * - in case of XML, parser reads all data from its input (file, memory, XML tree) including the case of not well-formed
 * XML document (multiple top-level elements) and if there is an unknown element, it is skipped including its subtree
 * (see the next point). This can be changed by the #LYD_OPT_NOSIBLINGS option which make parser to read only a single
 * tree (with a single root element) from its input.
 * - parser silently ignores the data without a matching node in schema trees. If the caller want to stop
 * parsing in case of presence of unknown data, the #LYD_OPT_STRICT can be used. The strict mode is useful for
 * NETCONF servers, since NETCONF clients should always send data according to the capabilities announced by the server.
 * On the other hand, the default non-strict mode is useful for clients receiving data from NETCONF server since
 * clients are not required to understand everything the server does. Of course, the optimal strategy for clients is
 * to use filtering to get only the required data. Having an unknown element of the known namespace is always an error.
 * The behavior can be changed by #LYD_OPT_STRICT option.
 * - using obsolete statements (status set to obsolete) just generates a warning, but the processing continues. The
 * behavior can be changed by #LYD_OPT_OBSOLETE option.
 * - parser expects that the provided data provides complete datastore content (both the configuration and state data)
 * and performs data validation according to all YANG rules. This can be a problem in case of representing NETCONF's
 * subtree filter data, edit-config's data or other type of data set - such data do not represent a complete data set
 * and some of the validation rules can fail. Therefore there are other options (within lower 8 bits) to make parser
 * to accept such a data.
 * @{
 */

#define LYD_OPT_DATA       0x00 /**< Default type of data - complete datastore content with configuration as well as
                                     state data. */
#define LYD_OPT_CONFIG     0x01 /**< A configuration datastore - complete datastore without state data.
                                     Validation modifications:
                                     - status data are not allowed */
#define LYD_OPT_GET        0x02 /**< Data content from a NETCONF reply message to the NETCONF \<get\> operation.
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - list's keys/unique nodes are not required (so duplication is not checked) */
#define LYD_OPT_GETCONFIG  0x04 /**< Data content from a NETCONF reply message to the NETCONF \<get-config\> operation
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - list's keys/unique nodes are not required (so duplication is not checked)
                                     - status data are not allowed */
#define LYD_OPT_EDIT       0x08 /**< Content of the NETCONF \<edit-config\>'s config element.
                                     Validation modifications:
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - status data are not allowed */
#define LYD_OPT_RPC        0x10 /**< Data represents RPC's input parameters. */
#define LYD_OPT_RPCREPLY   0x20 /**< Data represents RPC's output parameters (maps to NETCONF <rpc-reply> data). */
#define LYD_OPT_NOTIF      0x40 /**< Data represents an event notification data. */
#define LYD_OPT_FILTER     0x80 /**< Data represents NETCONF subtree filter. Validation modifications:
                                     - leafs/leaf-lists with no data are allowed (even not allowed e.g. by length restriction)
                                     - multiple instances of container/leaf/.. are allowed
                                     - list's keys/unique nodes are not required
                                     - mandatory nodes can be omitted
                                     - leafrefs and instance-identifier are not resolved
                                     - data from different choice's branches are allowed */
#define LYD_OPT_TYPEMASK   0xff /**< Mask to filter data type options. Always only a single data type option (only
                                     single bit from the lower 8 bits) can be set. */

#define LYD_OPT_STRICT     0x0100 /**< Instead of silent ignoring data without schema definition, raise an error. */
#define LYD_OPT_DESTRUCT   0x0200 /**< Free the provided XML tree during parsing the data. With this option, the
                                       provided XML tree is affected and all succesfully parsed data are freed.
                                       This option is applicable only to lyd_parse_xml() function. */
#define LYD_OPT_OBSOLETE   0x0400 /**< Raise an error when an obsolete statement (status set to obsolete) is used. */
#define LYD_OPT_NOSIBLINGS 0x0800 /**< Parse only a single XML tree from the input. This option applies only to
                                       XML input data. */

/**
 * @}
 */

/**
 * @brief Parse (and validate according to appropriate schema from the given context) data.
 *
 * In case of LY_XML format, the data string is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] data Serialized data in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_parse_data(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, ...);

/**
 * @brief Parse (and validate according to appropriate schema from the given context) XML tree.
 *
 * The output data tree is parsed from the given XML tree previously parsed by one of the
 * lyxml_read* functions.
 *
 * If there are some sibling elements of the \p root (data were read with #LYXML_READ_MULTIROOT option
 * or the provided root is a root element of a subtree), all the sibling nodes (previous as well as
 * following) are processed as well. The returned data node is a root of the first tree with other
 * trees connected via the next pointer. This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * When the function is used with #LYD_OPT_DESTRUCT, all the successfully parsed data including the
 * XML \p root and all its siblings (if #LYD_OPT_NOSIBLINGS is not used) are freed. Only with
 * #LYD_OPT_DESTRUCT option the \p root pointer is changed - if all the data are parsed, it is set
 * to NULL, otherwise it will hold the XML tree without the successfully parsed elements.
 *
 * The context must be the same as the context used to parse XML tree by lyxml_read* function.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in,out] root XML tree to parse (convert) to data tree. By default, parser do not change the XML tree. However,
 *            when #LYD_OPT_DESTRUCT is specified in \p options, parser frees all successfully parsed data.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_parse_xml(struct ly_ctx *ctx, struct lyxml_elem **root, int options,...);

/**
 * @brief Read data from the given file descriptor.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * In case of LY_XML format, the file content is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] fd The standard file descriptor of the file containing the data tree in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...);

/**
 * @brief Read data from the given file path.
 *
 * In case of LY_XML format, the file content is parsed completely. It means that when it contains
 * a non well-formed XML with multiple root elements, all those sibling XML trees are parsed. The
 * returned data node is a root of the first tree with other trees connected via the next pointer.
 * This behavior can be changed by #LYD_OPT_NOSIBLINGS option.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] path Path to the file containing the data tree in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @param[in] options Parser options, see @ref parseroptions.
 * @param[in] ... Additional argument must be supplied when #LYD_OPT_RPCREPLY value is specified in \p options. The
 *            argument is supposed to provide pointer to the RPC schema node for the reply's request
 *            (const struct ::lys_node* rpc).
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, ...);

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
 * @ingroup datatree
 * @ingroup schematree
 * @{
 *
 * @brief Structure to hold a set of (not necessary somehow connected) ::lyd_node or ::lys_node objects.
 * Caller is supposed to not mix the type of objects added to the set and according to its knowledge about
 * the set content, it is supposed to access the set via the sset, dset or set members of the structure.
 *
 * To free the structure, use lyd_set_free() function, to manipulate with the structure, use other
 * lyd_set_* functions.
 */
struct ly_set {
    unsigned int size;               /**< allocated size of the set array */
    unsigned int number;             /**< number of elements in (used size of) the set array */
    union {
        struct lys_node **sset;      /**< array of pointers to a ::lys_node objects */
        struct lyd_node **dset;      /**< array of pointers to a ::lyd_node objects */
        void **set;                   /**< dummy array for generic work */
    };
};

/**
 * @brief Create and initiate new ::lyd_set structure.
 *
 * @return Created ::lyd_set structure or NULL in case of error.
 */
struct ly_set *ly_set_new(void);

/**
 * @brief Add a ::lyd_node or ::lys_node object into the set
 *
 * @param[in] set Set where the \p node will be added.
 * @param[in] node The ::lyd_node or ::lys_node object to be added into the \p set;
 * @return 0 on success
 */
int ly_set_add(struct ly_set *set, void *node);

/**
 * @brief Remove a ::lyd_node or ::lys_node object from the set.
 *
 * Note that after removing a node from a set, indexes of other nodes in the set can change
 * (the last object is placed instead of the removed object).
 *
 * @param[in] set Set from which the \p node will be removed.
 * @param[in] node The ::lyd_node or ::lys_node object to be removed from the \p set;
 * @return 0 on success
 */
int ly_set_rm(struct ly_set *set, void *node);

/**
 * @brief Remove a ::lyd_node or ::lys_node object from the set index.
 *
 * Note that after removing a node from a set, indexes of other nodes in the set can change
 * (the last object is placed instead of the removed object).
 *
 * @param[in] set Set from which a node will be removed.
 * @param[in] index Index of the ::lyd_node or ::lys_node object in the \p set to be removed from the \p set;
 * @return 0 on success
 */
int ly_set_rm_index(struct ly_set *set, unsigned int index);

/**
 * @brief Free the ::lyd_set data. Frees only the set structure content, not the referred data.
 *
 * @param[in] set The set to be freed.
 */
void ly_set_free(struct ly_set *set);

/**@} sets */

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
int lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

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
int lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

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
int lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

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
int lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                  const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node);

/**
 * @defgroup printerflags Printer flags
 * @ingroup printers
 *
 * Validity flags for data nodes.
 *
 * @{
 */
#define LYP_WITHSIBLINGS 0x01 /**< Flag for printing also the (following) sibling nodes of the data node. */

/**
 * @}
 */

/**
 * @brief Print data tree in the specified format.
 *
 * To write data into a file descriptor, use lyd_print_fd().
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] f File stream where to print the data.
 * @param[in] format Data output format.
 * @param[in] options [printer flags](@ref printerflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * Same as lyd_print(), but output is written into the specified file descriptor.
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] fd File descriptor where to print the data.
 * @param[in] format Data output format.
 * @param[in] options [printer flags](@ref printerflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options);


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
 * @param[in] options [printer flags](@ref printerflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options);

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
 * @param[in] options [printer flags](@ref printerflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                  const struct lyd_node *root, LYD_FORMAT format, int options);

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
 * @brief Set logger callback.
 * @param[in] clb Logging callback.
 */
void ly_set_log_clb(void (*clb)(LY_LOG_LEVEL, const char *));

/**
 * @brief Get logger callback.
 * @return Logger callback (can be NULL).
 */
void (*ly_get_log_clb(void))(LY_LOG_LEVEL, const char *);

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
