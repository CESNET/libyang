/**
 * @file plugins_exts.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang support for YANG extensions implementation.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_H_
#define LY_PLUGINS_EXTS_H_

#include "log.h"
#include "parser_data.h"
#include "plugins.h"
#include "tree_data.h"
#include "tree_edit.h"
#include "tree_schema.h"

#include "plugins_exts_compile.h"
#include "plugins_exts_print.h"

struct ly_ctx;
struct ly_in;
struct lyd_node;
struct lysc_ext_substmt;
struct lysp_ext_instance;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page howtoPluginsExtensions Extension Plugins
 *
 * Note that the part of the libyang API here is available only by including a separated `<libyang/plugins_exts.h>` header
 * file. Also note that the extension plugins API is versioned separately from libyang itself, so backward incompatible
 * changes can come even without changing libyang major version.
 *
 * YANG extensions are very complex. Usually only its description specifies how it is supposed to behave, what are the
 * allowed substatements, their cardinality or if the standard YANG statements placed inside the extension differs somehow
 * in their meaning or behavior. libyang provides the Extension plugins API to implement such extensions and add its support
 * into libyang itself. However we tried our best, the API is not (and it cannot be) so universal and complete to cover all
 * possibilities. There are definitely use cases which cannot be simply implemented only with this API.
 *
 * libyang implements 3 important extensions: [NACM](https://tools.ietf.org/html/rfc8341), [Metadata](@ref howtoDataMetadata)
 * and [yang-data](@ref howtoDataYangdata). Despite the core implementation in all three cases is done via extension plugin
 * API, also other parts of the libyang code had to be extended to cover complete scope of the extensions.
 *
 * We believe, that the API is capable to allow implementation of very wide range of YANG extensions. However, if you see
 * limitations for the particular YANG extension, don't hesitate to contact the project developers to discuss all the
 * options, including updating the API.
 *
 * The plugin's functionality is provided to libyang via a set of callbacks specified as an array of ::lyplg_ext_record
 * structures using the ::LYPLG_EXTENSIONS macro.
 *
 * The most important ::lyplg_ext.compile callback is responsible for processing the parsed extension instance. In this
 * phase, the callback must validate all the substatements, their values or placement of the extension instance itself.
 * If needed, the processed data can be stored in some form into the compiled schema representation of the extension
 * instance. To make the compilation process as easy as possible, libyang provides several
 * [helper functions](@ref pluginsExtensionsCompile) to handle the schema compilation context and to compile standard YANG
 * statements in the same way the libyang does it internally.
 *
 * The data validation callback ::lyplg_ext.validate is used for additional validation of a data nodes that contains the
 * connected extension instance directly (as a substatement) or indirectly in case of terminal nodes via their type (no
 * matter if the extension instance is placed directly in the leaf's/leaf-list's type or in the type of the referenced
 * typedef).
 *
 * The ::lyplg_ext.sprinter callback implement printing the compiled extension instance data when the schema (module) is
 * being printed in the ::LYS_OUT_YANG_COMPILED (info) format. As for compile callback, there are also
 * [helper functions](@ref pluginsExtensionsPrint) to access printer's context and to print standard YANG statements
 * placed in the extension instance by libyang itself.
 *
 * The last callback, ::lyplg_ext.free, is supposed to free all the data allocated by the ::lyplg_ext.compile callback.
 * To free the data created by helper function ::lys_compile_extension_instance(), the plugin can used
 * ::lyplg_ext_instance_substatements_free().
 *
 * The plugin information contains also the plugin identifier (::lyplg_type.id). This string can serve to identify the
 * specific plugin responsible to storing data value. In case the user can recognize the id string, it can access the
 * plugin specific data with the appropriate knowledge of its structure.
 *
 * Logging information from an extension plugin is possible via ::lyplg_ext_log() function
 */

/**
 * @defgroup pluginsExtensions Plugins: Extensions
 *
 * Structures and functions to for libyang plugins implementing specific YANG extensions defined in YANG modules. For more
 * information, see @ref howtoPluginsTypes.
 *
 * This part of libyang API is available by including `<libyang/plugins_ext.h>` header file.
 *
 * @{
 */

/**
 * @brief Extensions API version
 */
#define LYPLG_EXT_API_VERSION 4

/**
 * @brief Macro to define plugin information in external plugins
 *
 * Use as follows:
 * LYPLG_EXTENSIONS = {{<filled information of ::lyplg_ext_record>}, ..., {0}};
 */
#define LYPLG_EXTENSIONS \
    uint32_t plugins_extensions_apiver__ = LYPLG_EXT_API_VERSION; \
    const struct lyplg_ext_record plugins_extensions__[]

/**
 * @brief Free the extension instance's data compiled with ::lys_compile_extension_instance().
 *
 * @param[in] ctx libyang context
 * @param[in] substmts The sized array of extension instance's substatements. The whole array is freed except the storage
 * places which are expected to be covered by the extension plugin.
 */
LIBYANG_API_DECL void lyplg_ext_instance_substatements_free(struct ly_ctx *ctx, struct lysc_ext_substmt *substmts);

/**
 * @brief Callback to compile extension from the lysp_ext_instance to the lysc_ext_instance. The later structure is generally prepared
 * and only the extension specific data are supposed to be added (if any).
 *
 * The parsed generic statements can be processed by the callback on its own or the ::lys_compile_extension_instance
 * function can be used to let the compilation to libyang following the standard rules for processing the YANG statements.
 *
 * @param[in] cctx Current compile context.
 * @param[in] p_ext Parsed extension instance data.
 * @param[in,out] c_ext Prepared compiled extension instance structure where an addition, extension-specific, data are
 * supposed to be placed for later use (data validation or use of external tool).
 * @return LY_SUCCESS in case of success.
 * @return LY_EVALID in case of non-conforming parsed data.
 * @return LY_ENOT in case the extension instance is not supported and should be removed.
 */
typedef LY_ERR (*lyplg_ext_compile_clb)(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext,
        struct lysc_ext_instance *c_ext);

/**
 * @brief Callback to print the compiled extension instance's private data in the INFO format.
 *
 * @param[in] ctx YANG printer context to provide output handler and other information for printing.
 * @param[in] ext The compiled extension instance, mainly to access the extensions.
 * @param[in,out] flag Flag to be shared with the caller regarding the opening brackets - 0 if the '{' not yet printed,
 * 1 otherwise.
 * @return LY_SUCCESS when everything was fine, other LY_ERR values in case of failure
 */
typedef LY_ERR (*lyplg_ext_schema_printer_clb)(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag);

/**
 * @brief Callback to free the extension-specific data created by its compilation.
 *
 * @param[in] ctx libyang context.
 * @param[in,out] ext Compiled extension structure where the data to free are placed.
 */
typedef void (*lyplg_ext_free_clb)(struct ly_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Callback for getting a schema node for a new YANG instance data described by an extension instance.
 * Needed only if the extension instance supports some nested standard YANG data.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] parent Parsed parent data node. Set if @p sparent is NULL.
 * @param[in] sparent Schema parent node. Set if @p parent is NULL.
 * @param[in] prefix Element prefix, if any.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] format Format of @p prefix.
 * @param[in] prefix_data Format-specific prefix data.
 * @param[in] name Element name.
 * @param[in] name_len Length of @p name.
 * @param[out] snode Schema node to use for parsing the node.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the data are not described by @p ext.
 * @return LY_ERR on error.
 */
typedef LY_ERR (*lyplg_ext_data_snode_clb)(struct lysc_ext_instance *ext, const struct lyd_node *parent,
        const struct lysc_node *sparent, const char *prefix, size_t prefix_len, LY_VALUE_FORMAT format, void *prefix_data,
        const char *name, size_t name_len, const struct lysc_node **snode);

/**
 * @brief Callback for validating parsed YANG instance data described by an extension instance.
 *
 * This callback is used only for nested data definition (with a standard YANG schema parent).
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] sibling First sibling with schema node returned by ::lyplg_ext_data_snode_clb.
 * @param[in] dep_tree Tree to be used for validating references from the operation subtree, if operation.
 * @param[in] data_type Validated data type, can be ::LYD_TYPE_DATA_YANG, ::LYD_TYPE_RPC_YANG, ::LYD_TYPE_NOTIF_YANG,
 * or ::LYD_TYPE_REPLY_YANG.
 * @param[in] val_opts Validation options, see @ref datavalidationoptions.
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
typedef LY_ERR (*lyplg_ext_data_validate_clb)(struct lysc_ext_instance *ext, struct lyd_node *sibling,
        const struct lyd_node *dep_tree, enum lyd_type data_type, uint32_t val_opts, struct lyd_node **diff);

/**
 * @brief Extension plugin implementing various aspects of a YANG extension
 */
struct lyplg_ext {
    const char *id;                         /**< plugin identification (mainly for distinguish incompatible versions
                                                 of the plugins for external tools) */
    lyplg_ext_compile_clb compile;          /**< callback to compile extension instance from the parsed data */
    lyplg_ext_schema_printer_clb sprinter;  /**< callback to print the compiled content (info format) of the extension
                                                 instance */
    lyplg_ext_free_clb free;                /**< free the extension-specific data created by its compilation */

    lyplg_ext_data_snode_clb snode;         /**< callback to get schema node for nested YANG data */
    lyplg_ext_data_validate_clb validate;   /**< callback to validate parsed data instances according to the extension
                                                 definition */
};

struct lyplg_ext_record {
    /* plugin identification */
    const char *module;          /**< name of the module where the extension is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optimal approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin functions. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< YANG name of the extension */

    /* runtime data */
    struct lyplg_ext plugin;     /**< data to utilize plugin implementation */
};

/**
 * @brief Get specific run-time extension instance data from a callback set by ::ly_ctx_set_ext_data_clb().
 *
 * @param[in] ctx Context with the callback.
 * @param[in] ext Compiled extension instance.
 * @param[out] ext_data Provided extension instance data.
 * @param[out] ext_data_free Whether the extension instance should free @p ext_data or not.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_get_data(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, void **ext_data,
        ly_bool *ext_data_free);

/**
 * @brief Insert extension instance data into a parent.
 *
 * @param[in] parent Parent node to insert into.
 * @param[in] first First top-level sibling node to insert.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LIBYANG_API_DECL LY_ERR lyd_insert_ext(struct lyd_node *parent, struct lyd_node *first);

/**
 * @brief Provide a log message from an extension plugin.
 *
 * @param[in] ext Compiled extension structure providing generic information about the extension/plugin causing the message.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] err_no Error type code.
 * @param[in] path Path relevant to the message.
 * @param[in] format Format string to print.
 */
LIBYANG_API_DECL void lyplg_ext_log(const struct lysc_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err_no, const char *path,
        const char *format, ...);

/**
 * @brief Expand parent-reference xpath expressions
 *
 * @param ext[in] context allocated for extension
 * @param refs[out] set of lysc nodes matching parent-refernce xpaths
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_schema_mount_get_parent_ref(const struct lysc_ext_instance *ext, struct ly_set **refs);

/** @} pluginsExtensions */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_H_ */
