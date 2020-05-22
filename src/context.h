/**
 * @file context.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal context structures and functions
 *
 * Copyright (c) 2015 - 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_CONTEXT_H_
#define LY_CONTEXT_H_

#include <stdint.h>

#include "log.h"
#include "parser_schema.h"
#include "tree_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page howtocontext Context
 *
 * The context concept allows callers to work in environments with different sets of YANG schemas.
 *
 * The first step in libyang is to create a new context using ly_ctx_new(). It returns a handler
 * used in the following work.
 *
 * When creating a new context, search dir can be specified (NULL is accepted) to provide directory
 * where libyang will automatically search for schemas being imported or included. The search path
 * can be later changed via ly_ctx_set_searchdir() and ly_ctx_unset_searchdir() functions. Before the search dirs,
 * also the current working directory is (non-recursively) searched. For the case of the explicitly set search
 * dirs, also all their subdirectories (and symlinks) are taken into account. Searching in the current working
 * directory can be avoided with the context's #LY_CTX_DISABLE_SEARCHDIR_CWD option (or via ly_ctx_set_options()).
 * Searching in all the context's search dirs (without removing them) can be avoided with the context's
 * #LY_CTX_DISABLE_SEARCHDIRS option (or via ly_ctx_set_options()). This automatic searching can be preceded
 * by a custom  module searching callback (#ly_module_imp_clb) set via ly_ctx_set_module_imp_clb(). The algorithm of
 * searching in search dirs is also available via API as lys_search_localfile() function.
 *
 * Schemas are added into the context using [parser functions](@ref howtoschemasparsers) - \b lys_parse_*().
 * Alternatively, also ly_ctx_load_module() can be used - in that case the #ly_module_imp_clb or automatic
 * search in search dir and in the current working directory is used. YANG submodules cannot be loaded or even validated
 * directly, they are loaded always only as includes of YANG modules.
 *
 * YANG schemas are loaded in two steps. First, the input YANG/YIN data are parsed into \b lysp_* structures that reflect
 * the structure of the input schema. Mostly just syntax checks are done, no reference or type checking is performed in
 * this step. If the module is supposed to be implemented, not just imported by another module, the second step is to compile
 * it. The compiled schema may significantly differ in structure from the source schema structure. All the references
 * are resolved, groupings are instantiated, types are resolved (and compiled by grouping all the relevant restrictions
 * when derived from another types) and many other syntactical checks are done.
 *
 * Similarly, data trees can be parsed by \b lyd_parse_*() functions. Note, that functions for schemas have \b lys_
 * prefix (or \b lysp_ for the parsed and \b lysc_ for the compiled schema) while functions for instance data have
 * \b lyd_ prefix. It can happen during data parsing that a schema is required and __not found__ in the context or
 * the schema is found, but is __only imported__, not implemented (so the data cannot actually be instantiated).
 * In these cases, a callback is called, which should add this schema into the context or change its conformance
 * to implemented. You can set the callback using ly_ctx_set_module_data_clb() (more in @ref howtodataparsers
 * and @ref howtodatavalidation).
 *
 * Context can hold multiple revisions of the same schema, but only one of them can be implemented. The schema is not
 * implemented in case it is automatically loaded as import for another module and it is not referenced in such
 * a module (and no other) as target of leafref, augment or deviation. All modules with deviation definition are always
 * marked as implemented. The imported (not implemented) module can be set implemented by lys_set_implemented(). But
 * the implemented module cannot be changed back to just imported module. The imported modules are used only as a
 * source of definitions for types and groupings for uses statements. The data in such modules are ignored - caller
 * is not allowed to create the data (including instantiating identities) defined in the model via data parsers,
 * the default nodes are not added into any data tree and mandatory nodes are not checked in the data trees. This
 * can be changed by ly_ctx_new()'s #LY_CTX_ALLIMPLEMENTED option (or via ly_ctx_set_options()), which causes that
 * all the imported modules are automatically set to be implemented.
 *
 * When loading/importing a module without revision, the latest revision of the required module is supposed to load.
 * For a context, the first time the latest revision of a module is requested, it is properly searched for and loaded.
 * However, when this module is requested (without revision) the second time, the one found previously is returned.
 * This has the advantage of not searching for the module repeatedly but the drawback that if a later revision
 * of the module is later made available, this context will not use it. However, to force libyang to re-search the
 * latest revision, ly_ctx_reset_latests() can be used (not that it applies to all the schemas in the context).
 *
 * Context holds all schema modules internally. To get a specific module, use ly_ctx_get_module() (or ly_ctx_get_module_ns())
 * The returned structure includes both, parsed and compiled, schema variants. If you need to do something with all the modules
 * in the context, it is advised to iterate over them using ly_ctx_get_module_iter(), it is the most efficient way.
 * Alternatively, the ly_ctx_info() function can be used to get complex information about the schemas in the context
 * in the form of data tree defined by <a href="https://tools.ietf.org/html/rfc7895">ietf-yang-library</a> schema.
 * To get a specific node defined in a module in the context, ly_ctx_find_path() or ly_ctx_get_node() can be used.
 *
 * Modules cannot be removed from their context. If you need to change the set of the schema modules in the context
 * (use only a subset), a new context must be created. To remove the context, there is ly_ctx_destroy() function.
 *
 * - @subpage howtocontextdict
 *
 * \note API for this group of functions is available in the [context module](@ref context).
 *
 * Functions List
 * --------------
 * - ::ly_ctx_new()
 * - ::ly_ctx_set_searchdir()
 * - ::ly_ctx_unset_searchdir()
 * - ::ly_ctx_unset_searchdirs()
 * - ::ly_ctx_get_searchdirs()
 * - ::ly_ctx_set_module_imp_clb()
 * - ::ly_ctx_get_module_imp_clb()
 * - ::ly_ctx_set_module_data_clb()
 * - ::ly_ctx_get_module_data_clb()
 * - ::ly_ctx_set_options()
 * - ::ly_ctx_unset_options()
 * - ::ly_ctx_get_options()
 * - ::ly_ctx_load_module()
 * - ::ly_ctx_info()
 * - ::ly_ctx_get_module_iter()
 * - ::ly_ctx_get_module()
 * - ::ly_ctx_get_module_ns()
 * - ::ly_ctx_get_module_implemented()
 * - ::ly_ctx_get_module_implemented_ns()
 * - ::ly_ctx_get_module_latest()
 * - ::ly_ctx_get_module_latest_ns()
 * - ::ly_ctx_reset_latests()
 * - ::ly_ctx_get_module_set_id()
 * - ::ly_ctx_get_node()
 * - ::ly_ctx_find_path()
 * - ::ly_ctx_destroy()
 * - ::lys_set_implemented()
 * - ::lys_search_localfile()
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
 * @struct ly_ctx
 * @brief libyang context handler.
 */
struct ly_ctx;

/**
 * @defgroup contextoptions Context options
 * @ingroup context
 *
 * Options to change context behavior.
 *
 * Note that the flags 0xFF00 are reserved for internal use.
 *
 * @{
 */

#define LY_CTX_ALLIMPLEMENTED 0x01 /**< All the imports of the schema being parsed are treated implemented. */
#define LY_CTX_TRUSTED        0x02 /**< Handle the schema being parsed as trusted and skip its validation
                                        tests. Note that while this option improves performance, it can
                                        lead to an undefined behavior if the schema is not correct. */
#define LY_CTX_NOYANGLIBRARY  0x04 /**< Do not internally implement ietf-yang-library module. The option
                                        causes that function ly_ctx_info() does not work (returns NULL) until
                                        the ietf-yang-library module is loaded manually. While any revision
                                        of this schema can be loaded with this option, note that the only
                                        revisions implemented by ly_ctx_info() are 2016-04-09 and 2018-01-17.
                                        This option cannot be changed on existing context. */
#define LY_CTX_DISABLE_SEARCHDIRS 0x08  /**< Do not search for schemas in context's searchdirs neither in current
                                        working directory. It is entirely skipped and the only way to get
                                        schema data for imports or for ly_ctx_load_module() is to use the
                                        callbacks provided by caller via ly_ctx_set_module_imp_clb() */
#define LY_CTX_DISABLE_SEARCHDIR_CWD 0x10 /**< Do not automatically search for schemas in current working
                                        directory, which is by default searched automatically (despite not
                                        recursively). */
#define LY_CTX_PREFER_SEARCHDIRS 0x20 /**< When searching for schema, prefer searchdirs instead of user callback. */

/** @} contextoptions */

/**
 * @brief Create libyang context.
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
 * @param[in] options Context options, see @ref contextoptions.
 * @param[out] new_ctx Pointer to the created libyang context if LY_SUCCESS returned.
 * @return LY_ERR return value.
 */
LY_ERR ly_ctx_new(const char *search_dir, int options, struct ly_ctx **new_ctx);

/**
 * @brief Add the search path into libyang context
 *
 * To reset search paths set in the context, use ly_ctx_unset_searchdirs() and then
 * set search paths again.
 *
 * @param[in] ctx Context to be modified.
 * @param[in] search_dir New search path to add to the current paths previously set in ctx.
 * @return LY_ERR return value.
 */
LY_ERR ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir);

/**
 * @brief Clean the search path(s) from the libyang context
 *
 * To remove the search path by its index, use ly_ctx_unset_searchdir().
 *
 * @param[in] ctx Context to be modified.
 * @param[in] value Searchdir to be removed, use NULL to remove them all.
 * @return LY_ERR return value
 */
LY_ERR ly_ctx_unset_searchdirs(struct ly_ctx *ctx, const char *value);

/**
 * @brief Remove the specific search path from the libyang context.
 *
 * To remove the search path by its value, use ly_ctx_unset_searchdirs().
 *
 * @param[in] ctx Context to be modified.
 * @param[in] index Index of the searchdir to be removed.
 * @return LY_ERR return value
 */
LY_ERR ly_ctx_unset_searchdir(struct ly_ctx *ctx, unsigned int index);

/**
 * @brief Get the NULL-terminated list of the search paths in libyang context. Do not modify the result!
 *
 * @param[in] ctx Context to query.
 * @return NULL-terminated list (array) of the search paths, NULL if no searchpath was set.
 * Do not modify the provided data in any way!
 */
const char * const *ly_ctx_get_searchdirs(const struct ly_ctx *ctx);

/**
 * @brief Get the currently set context's options.
 *
 * @param[in] ctx Context to query.
 * @return Combination of all the currently set context's options, see @ref contextoptions.
 */
int ly_ctx_get_options(const struct ly_ctx *ctx);

/**
 * @brief Set some of the context's options, see @ref contextoptions.
 * @param[in] ctx Context to be modified.
 * @param[in] option Combination of the context's options to be set, see @ref contextoptions.
 * @return LY_ERR value.
 */
LY_ERR ly_ctx_set_options(struct ly_ctx *ctx, int option);

/**
 * @brief Unset some of the context's options, see @ref contextoptions.
 * @param[in] ctx Context to be modified.
 * @param[in] option Combination of the context's options to be unset, see @ref contextoptions.
 * @return LY_ERR value.
 */
LY_ERR ly_ctx_unset_options(struct ly_ctx *ctx, int option);

/**
 * @brief Get current ID of the modules set. The value is available also
 * as module-set-id in ly_ctx_info() result.
 *
 * @param[in] ctx Context to be examined.
 * @return Numeric identifier of the current context's modules set.
 */
uint16_t ly_ctx_get_module_set_id(const struct ly_ctx *ctx);

/**
 * @brief Callback for retrieving missing included or imported models in a custom way.
 *
 * When submod_name is provided, the submodule is requested instead of the module (in this case only
 * the module name without its revision is provided).
 *
 * If an @arg free_module_data callback is provided, it will be used later to free the allegedly const data
 * which were returned by this callback.
 *
 * @param[in] mod_name Missing module name.
 * @param[in] mod_rev Optional missing module revision. If NULL and submod_name is not provided, the latest revision is
 * requested, the parsed module is then marked by the latest_revision flag.
 * @param[in] submod_name Optional missing submodule name.
 * @param[in] submod_rev Optional missing submodule revision. If NULL and submod_name is provided, the latest revision is
 * requested, the parsed submodule is then marked by the latest_revision flag.
 * @param[in] user_data User-supplied callback data.
 * @param[out] format Format of the returned module data.
 * @param[out] module_data Requested module data.
 * @param[out] free_module_data Callback for freeing the returned module data. If not set, the data will be left untouched.
 * @return LY_ERR value. If the returned value differs from LY_SUCCESS, libyang continue in trying to get the module data
 * according to the settings of its mechanism to search for the imported/included schemas.
 */
typedef LY_ERR (*ly_module_imp_clb)(const char *mod_name, const char *mod_rev, const char *submod_name, const char *sub_rev,
                                    void *user_data, LYS_INFORMAT *format, const char **module_data,
                                    void (**free_module_data)(void *model_data, void *user_data));

/**
 * @brief Get the custom callback for missing import/include module retrieval.
 *
 * @param[in] ctx Context to read from.
 * @param[in] user_data Optional pointer for getting the user-supplied callback data.
 * @return Callback or NULL if not set.
 */
ly_module_imp_clb ly_ctx_get_module_imp_clb(const struct ly_ctx *ctx, void **user_data);

/**
 * @brief Set missing include or import module callback. It is meant to be used when the models
 * are not locally available (such as when downloading modules from a NETCONF server), it should
 * not be required in other cases.
 *
 * @param[in] ctx Context that will use this callback.
 * @param[in] clb Callback responsible for returning the missing model.
 * @param[in] user_data Arbitrary data that will always be passed to the callback \p clb.
 */
void ly_ctx_set_module_imp_clb(struct ly_ctx *ctx, ly_module_imp_clb clb, void *user_data);

/**
 * @brief Get YANG module of the given name and revision.
 *
 * @param[in] ctx Context to work in.
 * @param[in] name Name of the YANG module to get.
 * @param[in] revision Requested revision date of the YANG module to get. If not specified,
 * the schema with no revision is returned, if it is present in the context.
 * @return Pointer to the YANG module, NULL if no schema in the context follows the name and revision requirements.
 */
struct lys_module *ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision);

/**
 * @brief Get the latest revision of the YANG module specified by its name.
 *
 * YANG modules with no revision are supposed to be the oldest one.
 *
 * @param[in] ctx Context where to search.
 * @param[in] name Name of the YANG module to get.
 * @return The latest revision of the specified YANG module in the given context, NULL if no YANG module of the
 * given name is present in the context.
 */
struct lys_module *ly_ctx_get_module_latest(const struct ly_ctx *ctx, const char *name);

/**
 * @brief Get the (only) implemented YANG module specified by its name.
 *
 * @param[in] ctx Context where to search.
 * @param[in] name Name of the YANG module to get.
 * @return The only implemented YANG module revision of the given name in the given context. NULL if there is no
 * implemented module of the given name.
 */
struct lys_module *ly_ctx_get_module_implemented(const struct ly_ctx *ctx, const char *name);

/**
 * @brief Iterate over all modules in the given context.
 *
 * @param[in] ctx Context with the modules.
 * @param[in,out] index Index of the next module to get. Value of 0 starts from the beginning.
 * The value is updated with each call, so to iterate over all modules the same variable is supposed
 * to be used in all calls starting with value 0.
 * @return Next context module, NULL if the last was already returned.
 */
const struct lys_module *ly_ctx_get_module_iter(const struct ly_ctx *ctx, unsigned int *index);

/**
 * @brief Get YANG module of the given namespace and revision.
 *
 * @param[in] ctx Context to work in.
 * @param[in] ns Namespace of the YANG module to get.
 * @param[in] revision Requested revision date of the YANG module to get. If not specified,
 * the schema with no revision is returned, if it is present in the context.
 * @return Pointer to the YANG module, NULL if no schema in the context follows the namespace and revision requirements.
 */
struct lys_module *ly_ctx_get_module_ns(const struct ly_ctx *ctx, const char *ns, const char *revision);

/**
 * @brief Get the latest revision of the YANG module specified by its namespace.
 *
 * YANG modules with no revision are supposed to be the oldest one.
 *
 * @param[in] ctx Context where to search.
 * @param[in] ns Namespace of the YANG module to get.
 * @return The latest revision of the specified YANG module in the given context, NULL if no YANG module of the
 * given namespace is present in the context.
 */
struct lys_module *ly_ctx_get_module_latest_ns(const struct ly_ctx *ctx, const char *ns);

/**
 * @brief Get the (only) implemented YANG module specified by its namespace.
 *
 * @param[in] ctx Context where to search.
 * @param[in] ns Namespace of the YANG module to get.
 * @return The only implemented YANG module revision of the given namespace in the given context. NULL if there is no
 * implemented module of the given namespace.
 */
struct lys_module *ly_ctx_get_module_implemented_ns(const struct ly_ctx *ctx, const char *ns);

/**
 * @brief Reset cached latest revision information of the schemas in the context.
 *
 * When a (sub)module is imported/included without revision, the latest revision is
 * searched. libyang searches for the latest revision in searchdirs and/or via provided
 * import callback ly_module_imp_clb() just once. Then it is expected that the content
 * of searchdirs or data returned by the callback does not change. So when it changes,
 * it is necessary to force searching for the latest revision in case of loading another
 * module, which what this function does.
 *
 * The latest revision information is also reset when the searchdirs set changes via
 * ly_ctx_set_searchdir().
 *
 * @param[in] ctx libyang context where the latest revision information is going to be reset.
 */
void ly_ctx_reset_latests(struct ly_ctx *ctx);

/**
 * @brief Try to find the model in the searchpaths of \p ctx and load it into it. If custom missing
 * module callback is set, it is used instead.
 *
 * The context itself is searched for the requested module first. If \p revision is not specified
 * (the module of the latest revision is requested) and there is implemented revision of the requested
 * module in the context, this implemented revision is returned despite there might be a newer revision.
 * This behavior is cause by the fact that it is not possible to have multiple implemented revisions of
 * the same module in the context.
 *
 * @param[in] ctx Context to add to.
 * @param[in] name Name of the module to load.
 * @param[in] revision Optional revision date of the module. If not specified, the latest revision is loaded.
 * @return Pointer to the data model structure, NULL if not found or some error occurred.
 */
const struct lys_module *ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision);

/**
 * @brief Get current ID of the modules set. The value is available also
 * as module-set-id in ::ly_ctx_get_yanglib_data() result.
 *
 * @param[in] ctx Context to be examined.
 * @return Numeric identifier of the current context's modules set.
 */
uint16_t ly_ctx_get_yanglib_id(const struct ly_ctx *ctx);

/**
 * @brief Get data of the internal ietf-yang-library module with information about all the loaded modules.
 * ietf-yang-library module must be loaded.
 *
 * @param[in] ctx Context with the modules.
 * @return Generated data, must be freed,
 * @return NULL on error.
 */
struct lyd_node *ly_ctx_get_yanglib_data(const struct ly_ctx *ctx);

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
 * @param[in] private_destructor Optional destructor function for private objects assigned
 * to the nodes via lys_set_private(). If NULL, the private objects are not freed by libyang.
 * Remember the differences between the structures derived from ::lysc_node and always check
 * ::lysc_node#nodetype.
 */
void ly_ctx_destroy(struct ly_ctx *ctx, void (*private_destructor)(const struct lysc_node *node, void *priv));

/** @} context */

#ifdef __cplusplus
}
#endif

#endif /* LY_CONTEXT_H_ */
