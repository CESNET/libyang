/**
 * @file extensions.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang support for YANG extension implementations.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_EXTENSIONS_H_
#define LY_EXTENSIONS_H_

#include "tree_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup extensions
 * @{
 */

/**
 * @brief Callback to check that the extension can be instantiated inside the provided node
 *
 * @param[in] parent The parent of the instantiated extension.
 * @param[in] parent_type The type of the structure provided as \p parent.
 * @param[in] substmt_type libyang does not store all the extension instances in the structures where they are
 *                         instantiated in the module. In some cases (see #LYEXT_SUBSTMT) they are stored in parent
 *                         structure and marked with flag to know in which substatement of the parent the extension
 *                         was originally instantiated.
 * @return 0 - yes
 *         1 - no
 *         2 - ignore / skip without an error
 */
typedef int (*lyext_check_position_clb)(const void *parent, LYEXT_PAR parent_type, LYEXT_SUBSTMT substmt_type);

/**
 * @brief Callback to check that the extension instance is correct - have
 * the valid argument, all the mandatory substatements, etc.
 *
 * @param[in] ext Extension instance to be checked.
 * @return 0 - ok
 *         1 - error
 */
typedef int (*lyext_check_result_clb)(struct lys_ext_instance *ext);

/**
 * @brief Callback to decide whether the extension will be inherited into the provided schema node. The extension
 * instance is always from some of the node's parents.
 *
 * @param[in] ext Extension instance to be inherited.
 * @param[in] node Schema node where the node is supposed to be inherited.
 * @return 0 - yes
 *         1 - no (do not process the node's children)
 *         2 - no, but continue with children
 */
typedef int (*lyext_check_inherit_clb)(struct lys_ext_instance *ext, struct lys_node *node);

struct lyext_plugin {
    LYEXT_TYPE type;                          /**< type of the extension, according to it the structure will be casted */
    uint16_t flags;                           /**< [extension flags](@ref extflags) */

    lyext_check_position_clb check_position;  /**< callbcak for testing that the extension can be instantiated
                                                   under the provided parent. Mandatory callback. */
    lyext_check_result_clb check_result;      /**< callback for testing if the argument value of the extension instance
                                                   is valid. Mandatory if the extension has the argument. */
    lyext_check_inherit_clb check_inherit;    /**< callback to decide if the extension is supposed to be inherited into
                                                   the provided node */
};

struct lyext_plugin_complex {
    LYEXT_TYPE type;                          /**< type of the extension, according to it the structure will be casted */
    uint16_t flags;                           /**< [extension flags](@ref extflags) */

    lyext_check_position_clb check_position;  /**< callbcak for testing that the extension can be instantiated
                                                   under the provided parent. Mandatory callback. */
    lyext_check_result_clb check_result;      /**< callback for testing if the argument value of the extension instance
                                                   is valid. Mandatory if the extension has the argument. */
    lyext_check_inherit_clb check_inherit;    /**< callback to decide if the extension is supposed to be inherited into
                                                   the provided node */
    struct lyext_substmt *substmt;            /**< NULL-terminated array of allowed substatements and restrictions
                                                   to their instantiation inside the extension instance */
    size_t instance_size;                     /**< size of the instance structure to allocate, the structure is
                                                   is provided as ::lys_ext_instance_complex, but the content array
                                                   is accessed according to the substmt specification provided by
                                                   plugin */
};

struct lyext_plugin_list {
    const char *module;          /**< name of the module where the extension is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optional approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin extension. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< name of the extension */
    struct lyext_plugin *plugin; /**< plugin for the extension */
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LY_EXTENSIONS_H_ */
