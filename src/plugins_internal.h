/**
 * @file plugins_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief internal functions to support extension and type plugins.
 *
 * Copyright (c) 2019-2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_INTERNAL_H_
#define LY_PLUGINS_INTERNAL_H_

#include <stdint.h>

#include "plugins.h"
#include "plugins_exts.h"
#include "plugins_types.h"

#define LY_TYPE_UNKNOWN_STR "unknown"               /**< text representation of ::LY_TYPE_UNKNOWN */
#define LY_TYPE_BINARY_STR "binary"                 /**< text representation of ::LY_TYPE_BINARY */
#define LY_TYPE_UINT8_STR "8bit unsigned integer"   /**< text representation of ::LY_TYPE_UINT8 */
#define LY_TYPE_UINT16_STR "16bit unsigned integer" /**< text representation of ::LY_TYPE_UINT16 */
#define LY_TYPE_UINT32_STR "32bit unsigned integer" /**< text representation of ::LY_TYPE_UINT32 */
#define LY_TYPE_UINT64_STR "64bit unsigned integer" /**< text representation of ::LY_TYPE_UINT64 */
#define LY_TYPE_STRING_STR "string"                 /**< text representation of ::LY_TYPE_STRING */
#define LY_TYPE_BITS_STR "bits"                     /**< text representation of ::LY_TYPE_BITS */
#define LY_TYPE_BOOL_STR "boolean"                  /**< text representation of ::LY_TYPE_BOOL */
#define LY_TYPE_DEC64_STR "decimal64"               /**< text representation of ::LY_TYPE_DEC64 */
#define LY_TYPE_EMPTY_STR "empty"                   /**< text representation of ::LY_TYPE_EMPTY */
#define LY_TYPE_ENUM_STR "enumeration"              /**< text representation of ::LY_TYPE_ENUM */
#define LY_TYPE_IDENT_STR "identityref"             /**< text representation of ::LY_TYPE_IDENT */
#define LY_TYPE_INST_STR "instance-identifier"      /**< text representation of ::LY_TYPE_INST */
#define LY_TYPE_LEAFREF_STR "leafref"               /**< text representation of ::LY_TYPE_LEAFREF */
#define LY_TYPE_UNION_STR "union"                   /**< text representation of ::LY_TYPE_UNION */
#define LY_TYPE_INT8_STR "8bit integer"             /**< text representation of ::LY_TYPE_INT8 */
#define LY_TYPE_INT16_STR "16bit integer"           /**< text representation of ::LY_TYPE_INT16 */
#define LY_TYPE_INT32_STR "32bit integer"           /**< text representation of ::LY_TYPE_INT32 */
#define LY_TYPE_INT64_STR "64bit integer"           /**< text representation of ::LY_TYPE_INT64 */

/**
 * @brief Shared (= not context specific) type plugins set.
 */
extern struct ly_set ly_plugins_types;

/**
 * @brief Shared (= not context specific) extension plugins set.
 */
extern struct ly_set ly_plugins_extensions;

/**
 * @brief Count of static type plugins.
 */
extern uint32_t ly_static_type_plugins_count;

/**
 * @brief Count of static extension plugins.
 */
extern uint32_t ly_static_ext_plugins_count;

/**
 * @brief Get a type plugin.
 *
 * @param[in] PLUGIN_REF Reference to a type plugin. Either an index of a static plugin (offset by +1)
 * or a pointer to an external plugin.
 * @return Type plugin.
 */
#define LYSC_GET_TYPE_PLG(PLUGIN_REF) \
    (((uintptr_t)(PLUGIN_REF) <= (uintptr_t)ly_static_type_plugins_count) ? \
    (struct lyplg_type *)&((struct lyplg_type_record *)ly_plugins_types.objs[(PLUGIN_REF) - 1])->plugin : \
    (struct lyplg_type *)(PLUGIN_REF))

/**
 * @brief Get an extension plugin.
 *
 * @param[in] PLUGIN_REF Reference to an extension plugin. Either an index of a static plugin (offset by +1)
 * or a pointer to an external plugin.
 * @return Extension plugin.
 */
#define LYSC_GET_EXT_PLG(PLUGIN_REF) \
    (((uintptr_t)(PLUGIN_REF) <= (uintptr_t)ly_static_ext_plugins_count) ? \
    (struct lyplg_ext *)&((struct lyplg_ext_record *)ly_plugins_extensions.objs[(PLUGIN_REF) - 1])->plugin : \
    (struct lyplg_ext *)(PLUGIN_REF))

/**
 * @brief Initiate libyang plugins.
 *
 * Covers both the types and extensions plugins.
 *
 * @param[in] builtin_type_plugins_only Whether to load only built-in YANG type plugins and no included extension plugins.
 * @param[in] static_plugins_only Whether to load only static (libyang built-in) plugins, meaning no external directories with plugins.
 * @return LY_SUCCESS in case of success
 * @return LY_EINT in case of internal error
 * @return LY_EMEM in case of memory allocation failure.
 */
LY_ERR lyplg_init(ly_bool builtin_type_plugins_only, ly_bool static_plugins_only);

/**
 * @brief Remove (unload) all the plugins currently available.
 */
void lyplg_clean(void);

/**
 * @brief Find a type plugin.
 *
 * @param[in] ctx Optional context for which the plugin should be find. If NULL, only shared plugins will be searched
 * @param[in] module Name of the module where the type is defined. Must not be NULL, in case of plugins for
 * built-in types, the module is "".
 * @param[in] revision Revision of the module for which the plugin is implemented. NULL is not a wildcard, it matches
 * only the plugins with NULL revision specified.
 * @param[in] name Name of the type which the plugin implements.
 * @return ID of the found type plugin, 0 if none found. The type plugin can be obtained
 * by passing the returned ID to ::lysc_get_type_plugin() or ::LYSC_GET_TYPE_PLG.
 */
uintptr_t lyplg_type_plugin_find(const struct ly_ctx *ctx, const char *module, const char *revision, const char *name);

/**
 * @brief Find an extension plugin.
 *
 * @param[in] ctx Optional context for which the plugin should be find. If NULL, only shared plugins will be searched
 * @param[in] module Name of the module where the extension is defined.
 * @param[in] revision Revision of the module for which the plugin is implemented. NULL is not a wildcard, it matches
 * only the plugins with NULL revision specified.
 * @param[in] name Name of the extension which the plugin implements.
 * @return ID of the found extension plugin, 0 if none found. The extension plugin can be obtained
 * by passing the returned ID to ::lysc_get_ext_plugin() or ::LYSC_GET_EXT_PLG.
 */
uintptr_t lyplg_ext_plugin_find(const struct ly_ctx *ctx, const char *module, const char *revision, const char *name);

#endif /* LY_PLUGINS_INTERNAL_H_ */
