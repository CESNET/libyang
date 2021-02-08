/**
 * @file plugins_exts_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions to support extension plugins.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_INTERNAL_H_
#define LY_PLUGINS_EXTS_INTERNAL_H_

#include "plugins_exts.h"
#include "tree_schema.h"

#include "plugins_exts_metadata.h"

/**
 * @brief List of internally implemented extension plugins.
 */
extern struct lyext_plugins_list lyext_plugins_internal[];

/**
 * @brief Index of Metadata's annotation extension plugin in lyext_plugins_internal
 */
#define LYEXT_PLUGIN_INTERNAL_ANNOTATION 4

/**
 * @brief Index of yang-data extension plugin in lyext_plugins_internal
 */
#define LYEXT_PLUGIN_INTERNAL_YANGDATA 5

/**
 * @brief Find the extension plugin for the specified extension instance.
 *
 * @param[in] mod YANG module where the
 */
struct lyext_plugin *lyext_get_plugin(struct lysc_ext *ext);

#endif /* LY_PLUGINS_EXTS_INTERNAL_H_ */
