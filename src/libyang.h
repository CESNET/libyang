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

#include "tree.h"

/**
 * @page howto How To ...
 *
 * - @subpage howtocontext
 * - @subpage howtologger
 */

/** @page howtocontext Context
 *
 * The \em context concept allows callers to work in environments with different sets of YANG schemas.
 *
 * The first step in libyang is to create a new context using ly_ctx_new(). It returns a handler
 * used in the following work.
 *
 * When creating a new context, search dir can be specified (NULL is accepted) to provide directory
 * where libyang will automatically search for schemas being imported or included. The search path
 * can be later changed via ly_ctx_set_searchdir() function. Before exploring the specified search
 * dir, libyang tries to get imported and included schemas from the current working directory first.
 *
 * Note that it is prohibited to have two different revisions of the same schema in a single context.
 *
 * API for this group of functions is available in the [context module](@ref context).
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
 * API for this group of functions is available in the [logger module](@ref logger).
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
 * to work with a single context in which libyang is holding all data models and other internal
 * information according to which the data will be processed and validated. Therefore, both schema
 * and data trees are connected with a specific context.
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
 * @brief Get pointer to the data model structure of the specified name.
 *
 * If the module is not yet loaded in the context, libyang tries to find it in
 * the search directory specified when the context was created by ly_ctx_new().
 *
 * @param[in] ctx Context to work in.
 * @param[in] name Name of the YANG module to get.
 * @param[in] revision Optional revision date of the YANG module to get. If not
 * specified, the newest revision is returned (TODO).
 * @return Pointer to the data model structure.
 */
struct ly_module *ly_ctx_get_module(struct ly_ctx *ctx, const char *name, const char *revision);

/**
 * @brief Get submodule from the context's search dir.
 *
 * @param[in] module Parent (belongs-to) module.
 * @param[in] name Name of the YANG submodule to get.
 * @param[in] revision Optional revision date of the YANG submodule to get. If
 * not specified, the newest revision is returned (TODO).
 * @return Pointer to the data model structure.
 */
struct ly_submodule *ly_ctx_get_submodule(struct ly_module *module, const char *name, const char *revision);

/**
 * @brief Get the names of the loaded modules.
 *
 * @param[in] ctx Context with the modules.
 * @return NULL-terminated array of the module names,
 * NULL on error. The result must be freed by the caller.
 */
char **ly_ctx_get_module_names(struct ly_ctx *ctx);

/**
 * @brief Get the names of the loaded submodules of a loaded module.
 *
 * @param[in] ctx Context with the modules.
 * @param[in] name Name of the parent module.
 * @return NULL-terminated array of submodule names of the parent module,
 * NULL on error. The result must be freed by the caller.
 */
char **ly_ctx_get_submodule_names(struct ly_ctx *ctx, const char *name);

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
 */

/**
 * @brief Load a data model into the specified context.
 *
 * LY_IN_YANG (YANG) format is not yet supported.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
struct ly_module *lys_parse(struct ly_ctx *ctx, const char *data, LY_MINFORMAT format);

struct ly_module *lys_read(struct ly_ctx *ctx, int fd, LY_MINFORMAT format);

/**
 * @brief Free (and unlink it from the context) the specified schema.
 *
 * It is up to the caller that there is no instance data using the schema being freed.
 *
 * @param[in] module Data model to free.
 */
void lys_free(struct ly_module *module);

/**
 * @brief Parse (and validate according to appropriate schema from the given context) data.
 *
 * In case of LY_XML format, the data string is expected to contain XML data under the single
 * \<config\> or \<data\> element in the "urn:ietf:params:xml:ns:netconf:base:1.0" namespace.
 *
 * LY_JSON format is not yet supported.
 *
 * @param[in] ctx Context to connect with the data tree being built here.
 * @param[in] data Serialized data in the specified format.
 * @param[in] format Format of the input data to be parsed.
 * @return Pointer to the built data tree. To free the returned structure, use lyd_free().
 */
struct lyd_node *lyd_parse(struct ly_ctx *ctx, const char *data, LY_DFORMAT format);

/**
 * @brief Free (and unlink) the specified data (sub)tree.
 *
 * @param[in] node Root of the (sub)tree to be freed.
 */
void lyd_free(struct lyd_node *node);

/**@} parsers */


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
 *
 *   \todo describe target_node syntax
 */

/**
 * @brief Print schema tree in the specified format.
 *
 * @param[in] module Schema tree to print.
 * @param[in] f File stream where to print the schema.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter for ::LY_OUT_INFO format. It specifies which particular
 * node in the module will be printed.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print(FILE *f, struct ly_module *module, LY_MOUTFORMAT format, const char *target_node);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] f File stream where to print the data.
 * @param[in] format Data output format.
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lyd_print(FILE *f, struct lyd_node *root, LY_DFORMAT format);

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

#endif /* LY_LIBYANG_H_ */
