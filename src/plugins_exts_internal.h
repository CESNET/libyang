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
 * @brief Record describing an implemented extension.
 */
struct lyext_plugin_record {
    const char *module;          /**< name of the module where the extension is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optimal approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin extension. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< name of the extension */
    struct lyext_plugin *plugin; /**< plugin for the extension */
};

/**
 * @brief List of internally implemented extension plugins.
 */
extern struct lyext_plugin_record lyext_plugins_internal[];

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
