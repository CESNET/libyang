/**
 * @file schema_mount.h
 * @author Roman Janota <Roman.Janota@cesnet.cz>
 * @brief ietf-yang-schema-mount API
 *
 * Copyright (c) 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

 #ifndef LY_PLUGINS_EXTS_SCHEMA_MOUNT_H_
 #define LY_PLUGINS_EXTS_SCHEMA_MOUNT_H_

 #include "context.h"

 #ifdef __cplusplus
extern "C" {
 #endif

/**
 * @brief Create a shared schema mount context for a schema mount point.
 *
 * Does nothing if the mount point is not shared or if the context already exists.
 *
 * For printed contexts (read-only), all shared contexts must be created beforehand
 * by calling this function as they cannot be created once the context is printed.
 *
 * @param[in] ext Compiled extension instance of a schema mount point.
 * @param[in] ext_data ietf-yang-schema-mount and ietf-yang-library YANG data for the @p ext mount point.
 * @return LY_ENOT if @p ext mount point data was not found in @p ext_data, other LY_ERR value otherwise.
*/
LIBYANG_API_DECL LY_ERR lyplg_ext_schema_mount_create_shared_context(struct lysc_ext_instance *ext,
        const struct lyd_node *ext_data);

/**
 * @brief Destroy all the shared schema mount contexts for a given libyang context.
 *
 * For standard contexts this is done automatically when the context is destroyed.
 *
 * For printed contexts (read-only), all shared contexts must be destroyed
 * by calling this function as otherwise they will be lost once the original context is destroyed.
 *
 * @param[in] ext Compiled extension instance of a schema mount point. All the shared schema mount
 * contexts that belong to the same libyang context as @p ext will be destroyed.
 */
LIBYANG_API_DECL void lyplg_ext_schema_mount_destroy_shared_contexts(struct lysc_ext_instance *ext);

/**
 * @brief Destroy all the inline contexts for a given libyang context.
 *
 * For standard contexts this is done automatically when the context is destroyed.
 *
 * Inline contexts are not supported in printed contexts.
 *
 * @param[in] ext Compiled extension instance of a schema mount point. All the inline schema mount
 * contexts that belong to the same libyang context as @p ext will be destroyed.
 */
LIBYANG_API_DECL void lyplg_ext_schema_mount_destroy_inline_contexts(struct lysc_ext_instance *ext);

 #ifdef __cplusplus
}
 #endif

 #endif /* LY_PLUGINS_EXTS_SCHEMA_MOUNT_H_ */
