/**
 * @file libyang.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief The main libyang public header.
 *
 * Copyright (c) 2015-2016 CESNET, z.s.p.o.
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
 * @mainpage About
 *
 * libyang is a library implementing processing of the YANG schemas and data modeled by the YANG language. The
 * library is implemented in C for GNU/Linux and provides C API.
 *
 * @section about-features Main Features
 *
 * - Parsing (and validating) schemas in YIN format.
 * - Parsing, validating and printing instance data in XML format.
 * - Parsing, validating and printing instance data in JSON format.
 * - Manipulation with the instance data.
 *
 * - \todo Parsing (and validating) schemas in YANG format.
 *
 * @subsection about-features-others Extra (side-effect) Features
 *
 * - XML parser.
 * - Optimized string storage (dictionary).
 *
 * @section about-license License
 *
 * Copyright (c) 2015-2016 CESNET, z.s.p.o.
 *
 * (The BSD 3-Clause License)
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

/**
 * @page howto How To ...
 *
 * - @subpage howtocontext
 * - @subpage howtoschemas
 * - @subpage howtodata
 * - @subpage howtoxml
 * - @subpage howtothreads
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
 * This automatic searching can be completely avoided when the caller sets module searching callback
 * (#ly_module_clb) via ly_ctx_set_module_clb().
 *
 * Schemas are added into the context using [parser functions](@ref howtoschemasparsers) - \b lys_parse_*() or \b lyd_parse_*().
 * In case of schemas, also ly_ctx_load_module() can be used - in that case the #ly_module_clb or automatic
 * search in working directory and in the searchpath is used. Note, that functions for schemas have \b lys_
 * prefix while functions for instance data have \b lyd_ prefix.
 *
 * Context can hold multiple revisons of the same schema.
 *
 * Context holds all modules and their submodules internally. The list of available module names is
 * provided via ly_ctx_get_module_names() functions. Similarly, caller can get also a list of submodules
 * names of a specific module using ly_ctx_get_submodule_names() function. The returned names can be
 * subsequently used to get the (sub)module structures using ly_ctx_get_module() and ly_ctx_get_submodule().
 * Alternatively, the ly_ctx_info() function can be used to get complex information about the schemas in the context
 * in the form of data tree defined by
 * <a href="https://tools.ietf.org/html/draft-ietf-netconf-yang-library-04">ietf-yang-library</a> schema.
 *
 * Modules held by a context cannot be removed one after one. The only way how to \em change modules in the
 * context is to create a new context and remove the old one. To remove a context, there is ly_ctx_destroy()
 * function.
 *
 * - @subpage howtocontextdict
 *
 * \note API for this group of functions is available in the [context module](@ref context).
 *
 * Functions List
 * --------------
 * - ly_ctx_new()
 * - ly_ctx_set_searchdir()
 * - ly_ctx_get_searchdir()
 * - ly_ctx_set_module_clb()
 * - ly_ctx_get_module_clb()
 * - ly_ctx_load_module()
 * - ly_ctx_info()
 * - ly_ctx_get_module_names()
 * - ly_ctx_get_module()
 * - ly_ctx_get_module_by_ns()
 * - ly_ctx_get_submodule_names()
 * - ly_ctx_get_submodule()
 * - ly_ctx_destroy()
 */

/**
 * @page howtocontextdict Context Dictionary
 *
 * Context includes dictionary to store strings more effectively. The most of strings repeats quite often in schema
 * as well as data trees. Therefore, instead of allocating those strings each time they appear, libyang stores them
 * as records in the dictionary. The basic API to the context dictionary is public, so even a caller application can
 * use the dictionary.
 *
 * To insert a string into the dictionary, caller can use lydict_insert() (adding a constant string) or
 * lydict_insert_zc() (for dynamically allocated strings that won't be used by the caller after its insertion into
 * the dictionary). Both functions return the pointer to the inserted string in the dictionary record.
 *
 * To remove (reference of the) string from the context dictionary, lydict_remove() is supposed to be used.
 *
 * \note Incorrect usage of the dictionary can break libyang functionality.
 *
 * \note API for this group of functions is described in the [XML Parser module](@ref dict).
 *
 * Functions List
 * --------------
 * - lydict_insert()
 * - lydict_insert_zc()
 * - lydict_remove()
 */

/**
 * @page howtoschemas Schemas
 *
 *
 * Schema is an internal libyang's representation of a YANG data model. Each schema is connected with
 * its [context](@ref howtocontext) and loaded using [parser functions](@ref howtoschemasparsers). It means, that
 * the schema cannot be created (nor changed) programmatically. In libyang, schemas are used only to
 * access data model definitions.
 *
 * Schema tree nodes are able to hold private objects (via a pointer to a structure, function, variable, ...) used by
 * a caller application. Such an object can be assigned to a specific node using lys_set_private() function.
 * Note that the object is not freed by libyang when the context is being destroyed. So the caller is responsible
 * for freeing the provided structure after the context is destroyed or the private pointer is set to NULL in
 * appropriate schema nodes where the object was previously set. On the other hand, freeing the object while the schema
 * tree is still used can lead to a segmentation fault.
 *
 * - @subpage howtoschemasparsers
 * - @subpage howtoschemasfeatures
 * - @subpage howtoschemasprinters
 *
 * \note There are many functions to access information from the schema trees. Details are available in
 * the [Schema Tree module](@ref schematree).
 *
 * Functions List (not assigned to above subsections)
 * --------------------------------------------------
 * - lys_get_node()
 * - lys_get_next()
 * - lys_parent()
 * - lys_set_private()
 */

/**
 * @page howtoschemasparsers Parsing Schemas
 *
 * Schema parser allows to read schema from a specific format. libyang supports the following schema formats:
 *
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
 * When the [context](@ref howtocontext) is created, it already contains the following three schemas, which
 * are implemented internally by libyang: *
 * - ietf-inet-types@2013-07-15
 * - ietf-yang-types@2013-07-15
 * - ietf-yang-library@2015-07-03
 *
 * Other schemas can be added to the context manually as described in [context page](@ref howtocontext) by the functions
 * listed below. Besides the schema parser functions, it is also possible to use ly_ctx_load_module() which tries to
 * find the required schema automatically - using #ly_module_clb or automatic search in working directory and in the
 * context's searchpath.
 *
 * Functions List
 * --------------
 * - lys_parse_mem()
 * - lys_parse_fd()
 * - lys_parse_path()
 * - ly_ctx_set_module_clb()
 * - ly_ctx_load_module()
 */

/**
 * @page howtoschemasfeatures YANG Features Manipulation
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
 * to enable and disable the specific feature (or all via \b "*"). By default, when the module
 * is loaded by libyang parser, all features are disabled.
 *
 * To get know, if a specific schema node is currently disabled or enable, the lys_is_disabled() function can be used.
 *
 * Note, that the feature's state can affect some of the output formats (e.g. Tree format).
 *
 * Functions List
 * --------------
 * - lys_features_list()
 * - lys_features_enable()
 * - lys_features_disable()
 * - lys_features_state()
 * - lys_is_disabled()
 */

/**
 * @page howtoschemasprinters Printing Schemas
 *
 * Schema printers allows to serialize internal representation of a schema module in a specific format. libyang
 * supports the following schema formats for printing:
 *
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
 *   It allows to print information not only about a specific module, but also about its specific part:
 *
 *   - absolute-schema-nodeid
 *
 *     e.g. \a `/modules/module-set-id`  in \a `ietf-yang-library` module
 *
 *   - <b>typedef/</b>typedef-name
 *
 *     e.g. \a `typedef/revision-identifier` in \a `ietf-yang-library` module
 *
 *   - <b>feature/</b>feature-name
 *
 *     e.g. \a `feature/ssh` in \a `ietf-netconf-server` module
 *
 *   - <b>grouping/</b>grouping-name/descendant-schema-nodeid
 *
 *     e.g. \a `grouping/module` or \a `grouping/module/module/submodules` in \a `ietf-yang-library` module
 *
 *   - <b>type/</b>leaf-or-leaflist
 *
 *     e.g. \a `type/modules/module-set-id` in \a `ietf-yang-library` module
 *
 * Printer functions allow to print to the different outputs including a callback function which allows caller
 * to have a full control of the output data - libyang passes to the callback a private argument (some internal
 * data provided by a caller of lys_print_clb()), string buffer and number of characters to print. Note that the
 * callback is supposed to be called multiple times during the lys_print_clb() execution.
 *
 * Functions List
 * --------------
 * - lys_print_mem()
 * - lys_print_fd()
 * - lys_print_file()
 * - lys_print_clb()
 */

/**
 * @page howtodata Data Instances
 *
 * All data nodes in data trees are connected with their schema node - libyang is not able to represent data of an
 * unknown schema.
 *
 * By default, the represented data are supposed to represent a full YANG datastore content. So if a schema declares
 * some mandatory nodes, despite configuration or status, the data are supposed to be present in the data tree being
 * loaded or validated. However, it is possible to specify other kinds of data (see @ref parseroptions) allowing some
 * exceptions to the validation process.
 *
 * Data validation is performed implicitly to the input data processed by the parser (\b lyd_parse_*() functions) and
 * on demand via the lyd_validate() function. The lyd_validate() is supposed to be used when a (complex or simple)
 * change is done on the data tree (via a combination of \b lyd_change_*(), \b lyd_insert*(), \b lyd_new*(),
 * lyd_unlink() and lyd_free() functions).
 *
 * - @subpage howtodataparsers
 * - @subpage howtodatamanipulators
 * - @subpage howtodataprinters
 *
 * \note API for this group of functions is described in the [Data Instances module](@ref datatree).
 *
 * Functions List (not assigned to above subsections)
 * --------------------------------------------------
 * - lyd_get_node()
 */

/**
 * @page howtodataparsers Parsing Data
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
 *   can be found in [this draft](https://tools.ietf.org/html/draft-ietf-netmod-yang-json-05).
 *
 * Besides the format of input data, the parser functions accepts additional [options](@ref parseroptions) to specify
 * how the input data should be processed.
 *
 * In contrast to the schema parser, data parser also accepts empty input data if such an empty data tree is valid
 * according to the schemas in the libyang context.
 *
 * In case of XML input data, there is one additional way to parse input data. Besides parsing the data from a string
 * in memory or a file, caller is able to build an XML tree using [libyang XML parser](@ref howtoxml) and then use
 * this tree (or a part of it) as input to the lyd_parse_xml() function.
 *
 * Functions List
 * --------------
 * - lyd_parse_mem()
 * - lyd_parse_fd()
 * - lyd_parse_path()
 * - lyd_parse_xml()
 */

/**
 * @page howtodatamanipulators Manipulating Data
 *
 * There are many functions to create or modify an existing data tree. You can add new nodes, reconnect nodes from
 * one tree to another (or e.g. from one list instance to another) or remove nodes. The functions doesn't allow you
 * to put a node to a wrong place (by checking the module), but not all validation checks can be made directly
 * (or you have to make a valid change by multiple tree modifications) when the tree is being changed. Therefore,
 * there is lyd_validate() function supposed to be called to make sure that the current data tree is valid. Note,
 * that not calling this function after the performed changes can cause failure of various libyang functions later.
 *
 * Also remember, that when you are creating/inserting a node, all the objects in that operation must belong to the
 * same context.
 *
 * Modifying the single data tree in multiple threads is not safe.
 *
 * Functions List
 * --------------
 * - lyd_dup()
 * - lyd_change_leaf()
 * - lyd_insert()
 * - lyd_insert_before()
 * - lyd_insert_after()
 * - lyd_insert_attr()
 * - lyd_new()
 * - lyd_new_anyxml()
 * - lyd_new_leaf()
 * - lyd_output_new()
 * - lyd_output_new_anyxml()
 * - lyd_output_new_leaf()
 * - lyd_unlink()
 * - lyd_free()
 * - lyd_free_attr()
 * - lyd_free_withsiblings()
 * - lyd_validate()
 */

/**
 * @page howtodataprinters Printing Data
 *
 * Schema printers allows to serialize internal representation of a schema module in a specific format. libyang
 * supports the following schema formats for printing:
 *
 * - XML
 *
 *   Basic format as specified in rules of mapping YANG modeled data to XML in
 *   [RFC 6020](http://tools.ietf.org/html/rfc6020). It is possible to specify if
 *   the indentation will be used.
 *
 * - JSON
 *
 *   The alternative data format available in RESTCONF protocol. Specification of JSON encoding of data modeled by YANG
 *   can be found in [this draft](https://tools.ietf.org/html/draft-ietf-netmod-yang-json-05).
 *
 * Printer functions allow to print to the different outputs including a callback function which allows caller
 * to have a full control of the output data - libyang passes to the callback a private argument (some internal
 * data provided by a caller of lyd_print_clb()), string buffer and number of characters to print. Note that the
 * callback is supposed to be called multiple times during the lyd_print_clb() execution.
 *
 * Functions List
 * --------------
 * - lyd_print_mem()
 * - lyd_print_fd()
 * - lyd_print_file()
 * - lyd_print_clb()
 */

/**
 * @page howtoxml libyang XML Support
 *
 * libyang XML parser is able to parse XML documents used to represent data modeled by YANG. Therefore, there are
 * some limitations in comparison to a full-featured XML parsers:
 * - comments are ignored
 * - Doctype declaration is ignored
 * - CData sections are ignored
 * - Process Instructions (PI) are ignored
 *
 * The API is designed to almost only read-only access. You can simply load XML document, go through the tree as
 * you wish and dump the tree to an output. The only "write" functions are lyxml_free() and lyxml_unlink() to remove
 * part of the tree or to unlink (separate) a subtree.
 *
 * XML parser is also used internally by libyang for parsing YIN schemas and data instances in XML format.
 *
 * \note API for this group of functions is described in the [XML Parser module](@ref xmlparser).
 *
 * Functions List
 * --------------
 * - lyxml_parse_mem()
 * - lyxml_parse_path()
 * - lyxml_get_attr()
 * - lyxml_get_ns()
 * - lyxml_print_mem()
 * - lyxml_print_fd()
 * - lyxml_print_file()
 * - lyxml_print_clb()
 * - lyxml_unlink()
 * - lyxml_free()
 */

/**
 * @page howtothreads libyang in Threads
 *
 * libyang can be used in multithreaded application keeping in mind the following rules:
 * - libyang context manipulation (adding new schemas) is not thread safe and it is supposed to be done in a main
 *   thread before any other work with context, schemas or data instances. And destroying the context is supposed to
 *   be done when no other thread accesses context, schemas nor data trees
 * - Data parser (\b lyd_parse*() functions) can be used simultaneously in multiple threads (also the returned
 *   #ly_errno is thread safe).
 * - Modifying (lyd_new(), lyd_insert(), lyd_unlink(), lyd_free() and many other functions) a single data tree is not
 *   thread safe.
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
 * \note API for this group of functions is described in the [logger module](@ref logger).
 *
 * Functions List
 * --------------
 * - ly_verb()
 * - ly_set_log_clb()
 * - ly_get_log_clb()
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
 * @defgroup nodeset Tree nodes set
 * @ingroup datatree
 * @ingroup schematree
 * @{
 *
 * Structure and functions to hold and manipulate with sets of nodes from schema or data trees.
 */

/**
 * @brief Structure to hold a set of (not necessary somehow connected) ::lyd_node or ::lys_node objects.
 * Caller is supposed to not mix the type of objects added to the set and according to its knowledge about
 * the set content, it is supposed to access the set via the sset, dset or set members of the structure.
 *
 * To free the structure, use ly_set_free() function, to manipulate with the structure, use other
 * ly_set_* functions.
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
 * @brief Create and initiate new ::ly_set structure.
 *
 * @return Created ::ly_set structure or NULL in case of error.
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
 * @brief Free the ::ly_set data. Frees only the set structure content, not the referred data.
 *
 * @param[in] set The set to be freed.
 */
void ly_set_free(struct ly_set *set);

/**@} nodeset */

/**
 * @defgroup printerflags Printer flags
 * @ingroup datatree
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
 * @cond INTERNAL
 * Function to get address of global `ly_errno' variable.
 */
LY_ERR *ly_errno_location(void);

/**
 * @endcond INTERNAL
 * @brief libyang specific (thread-safe) errno (see #LY_ERR for the list of possible values and their meaning).
 */
#define ly_errno (*ly_errno_location())

/**@} logger */

#ifdef __cplusplus
}
#endif

#endif /* LY_LIBYANG_H_ */
