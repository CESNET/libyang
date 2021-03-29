/**
 * @file plugins_exts.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang support for YANG extensions implementation.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
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
#include "plugins.h"
#include "tree_edit.h"
#include "tree_schema.h"

#include "plugins_exts_compile.h"
#include "plugins_exts_print.h"

struct ly_ctx;
struct lyd_node;
struct lysc_ctx;
struct lyspr_ctx;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup extensions YANG Extensions
 *
 * @{
 */

/**
 * @brief Extensions API version
 */
#define LYPLG_EXT_API_VERSION 1

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
 * @param[in] libyang context
 * @param[in] substmts The sized array of extension instance's substatements. The whole array is freed except the storage
 * places which are expected to be covered by the extension plugin.
 */
void lysc_extension_instance_substatements_free(struct ly_ctx *ctx, struct lysc_ext_substmt *substmts);

/**
 * @brief Callback to compile extension from the lysp_ext_instance to the lysc_ext_instance. The later structure is generally prepared
 * and only the extension specific data are supposed to be added (if any).
 *
 * The parsed generic statements can be processed by the callback on its own or the ::lys_compile_extension_instance
 * function can be used to let the compilation to libyang following the standard rules for processing the YANG statements.
 *
 * @param[in] cctx Current compile context.
 * @param[in] p_ext Parsed extension instance data.
 * @param[in,out] c_ext Prepared compiled extension instance structure where an addition, extension-specific, data are supposed to be placed
 * for later use (data validation or use of external tool).
 * @return LY_SUCCESS in case of success.
 * @return LY_EVALID in case of non-conforming parsed data.
 * @return LY_ENOT in case the extension instance is not supported and should be removed.
 */
typedef LY_ERR (*lyext_clb_compile)(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext);

/**
 * @brief Callback to free the extension specific data created by the ::lyext_clb_compile callback of the same extension plugin.
 *
 * @param[in] ctx libyang context.
 * @param[in,out] ext Compiled extension structure where the data to free are placed.
 */
typedef void (*lyext_clb_free)(struct ly_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Callback to decide if data instance is valid according to the schema.
 *
 * The callback is used only for the extension instances placed in the following parent statements
 * (which is specified as ::lysc_ext_instance.parent_type):
 *     - LYEXT_PAR_NODE - @p node is instance of the schema node where the extension instance was specified.
 *     - LYEXT_PAR_TPDF - @p node is instance of the schema node with the value of the typedef's type where the extension instance was specified.
 *     - LYEXT_PAR_TYPE - @p node is instance of the schema node with the value of the type where the extension instance was specified.
 *     - LYEXT_PAR_TYPE_BIT - @p node is instance of the schema node with the value of the bit where the extension instance was specified.
 *     - LYEXT_PAR_TYPE_ENUM - @p node is instance of the schema node with the value of the enum where the extension instance was specified.
 *
 * @param[in] ext Extension instance to be checked.
 * @param[in] node Data node, where the extension data are supposed to be placed.
 *
 * @return LY_SUCCESS on data validation success.
 * @return LY_EVALID in case the validation fails.
 */
typedef LY_ERR (*lyext_clb_data_validation)(struct lysc_ext_instance *ext, struct lyd_node *node);

/**
 * @brief Callback to print the compiled extension instance's private data in the INFO format.
 *
 * @param[in] ctx YANG printer context to provide output handler and other information for printing.
 * @param[in] ext The compiled extension instance, mainly to access the extensions.
 * @param[in, out] flag Flag to be shared with the caller regarding the opening brackets - 0 if the '{' not yet printed,
 * 1 otherwise.
 *
 * @return LY_SUCCESS when everything was fine, other LY_ERR values in case of failure
 */
typedef LY_ERR (*lyext_clb_schema_printer)(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag);

/**
 * @brief Extension plugin implementing various aspects of a YANG extension
 */
struct lyplg_ext {
    const char *id;                     /**< Plugin identification (mainly for distinguish incompatible versions of the plugins for external tools) */
    lyext_clb_compile compile;          /**< Callback to compile extension instance from the parsed data */
    lyext_clb_data_validation validate; /**< Callback to decide if data instance is valid according to the schema. */
    lyext_clb_schema_printer sprinter;  /**< Callback to print the compiled content (info format) of the extension instance */
    /* lyext_clb_data_printer dprinter; ? */
    lyext_clb_free free;                /**< Free the extension instance specific data created by ::lyplg_ext.compile callback */
};

struct lyplg_ext_record {
    /* plugin identification */
    const char *module;          /**< name of the module where the extension is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optimal approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin functions. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< name of the extension */

    /* runtime data */
    struct lyplg_ext plugin;     /**< data to utilize plugin implementation */
};

/**
 * @brief Provide a log message from an extension plugin.
 *
 * @param[in] ext Compiled extension structure providing generic information about the extension/plugin causing the message.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] err_no Error type code.
 * @param[in] path Path relevant to the message.
 * @param[in] format Format string to print.
 */
void lyext_log(const struct lysc_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err_no, const char *path, const char *format, ...);

/** @} extensions */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_H_ */
