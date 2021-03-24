/**
 * @file plugins.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Plugins manipulation.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_H_
#define LY_PLUGINS_H_

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup plugins Plugins
 * @{
 *
 */

/**
 * @brief Identifiers of the plugin type.
 */
enum LYPLG {
    LYPLG_TYPE,      /**< Specific type (typedef) */
    LYPLG_EXTENSION  /**< YANG extension */
};

/**
 * @brief Manually load a plugin file.
 *
 * Note, that a plugin can be loaded only if there is at least one context. The loaded plugins are connected with the
 * existence of a context. When all the contexts are destroyed, all the plugins are unloaded.
 *
 * @param[in] pathname Path to the plugin file. It can contain types or extensions plugins, both are accepted and correctly
 * loaded.
 *
 * @return LY_SUCCESS if the file contains valid plugin compatible with the library version.
 * @return LY_EDENIED in case there is no context and the plugin cannot be loaded.
 * @return LY_EINVAL when pathname is NULL or the plugin contains invalid content for this libyang version.
 * @return LY_ESYS when the plugin file cannot be loaded.
 */
LY_ERR lyplg_add(const char *pathname);

/** @} plugins */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_H_ */
