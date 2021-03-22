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

/** @} plugins */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_H_ */
