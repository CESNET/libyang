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
#include "tree_edit.h"
#include "tree_schema.h"

#include "plugins_exts_compile.h"
#include "plugins_exts_print.h"

struct ly_ctx;
struct lyd_node;

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
#define LYEXT_API_VERSION 1

/**
 * @brief Macro to store version of extension plugins API in the plugins.
 * It is matched when the plugin is being loaded by libyang.
 */
#define LYEXT_VERSION_CHECK uint32_t lyext_api_version = LYEXT_API_VERSION;

/**
 * @brief Possible cardinalities of the YANG statements.
 *
 * Used in extensions plugins to define cardinalities of the extension instance substatements.
 */
enum ly_stmt_cardinality {
    LY_STMT_CARD_OPT,    /* 0..1 */
    LY_STMT_CARD_MAND,   /* 1 */
    LY_STMT_CARD_SOME,   /* 1..n */
    LY_STMT_CARD_ANY     /* 0..n */
};

/**
 * @brief Description of the extension instance substatements.
 *
 * Provided by extensions plugins to libyang to be able to correctly compile the content of extension instances.
 * Note that order of the defined records matters - just follow the values of ::ly_stmt and order the records from lower to higher values.
 */
struct lysc_ext_substmt {
    enum ly_stmt stmt;                     /**< allowed substatement */
    enum ly_stmt_cardinality cardinality;  /**< cardinality of the substatement */
    void *storage;                         /**< pointer to the storage of the compiled statement according to the specific
                                                lysc_ext_substmt::stmt and lysc_ext_substmt::cardinality */
};

/**
 * @brief Free the extension instance's data compiled with ::lys_compile_extension_instance().
 *
 * @param[in] libyang context
 * @param[in] substmts The sized array of extension instance's substatements. The whole array is freed except the storage
 * places which are expected to be covered by the extension plugin.
 */
void lysc_extension_instance_substatements_free(struct ly_ctx *ctx, struct lysc_ext_substmt *substmts);

/**
 * @brief Get pointer to the storage of the specified substatement in the given extension instance.
 *
 * The function simplifies access into the ::lysc_ext_instance.substmts sized array.
 *
 * @param[in] ext Compiled extension instance to process.
 * @param[in] substmt Extension substatement to search for.
 * @param[out] instance_p Pointer where the storage of the @p substmt will be provided. The specific type returned depends
 * on the @p substmt and can be found in the documentation of each ::ly_stmt value. Also note that some of the substatements
 * (::lysc_node based or flags) can share the storage with other substatements. In case the pointer is NULL, still the return
 * code can be used to at least know if the substatement is allowed for the extension.
 * @param[out] cardinality_p Pointer to provide allowed cardinality of the substatements in the extension. Note that in some
 * cases, the type of the storage depends also on the cardinality of the substatement.
 * @return LY_SUCCESS if the @p substmt found.
 * @return LY_ENOT in case the @p ext is not able to store (does not allow) the specified @p substmt.
 */
LY_ERR lysc_ext_substmt(const struct lysc_ext_instance *ext, enum ly_stmt substmt,
        void **instance_p, enum ly_stmt_cardinality *cardinality_p);

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
struct lyext_plugin {
    const char *id;                     /**< Plugin identification (mainly for distinguish incompatible versions of the plugins for external tools) */
    lyext_clb_compile compile;          /**< Callback to compile extension instance from the parsed data */
    lyext_clb_data_validation validate; /**< Callback to decide if data instance is valid according to the schema. */
    lyext_clb_schema_printer sprinter;  /**< Callback to print the compiled content (info format) of the extension instance */
    /* lyext_clb_data_printer dprinter; ? */
    lyext_clb_free free;                /**< Free the extension instance specific data created by ::lyext_plugin.compile callback */
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
