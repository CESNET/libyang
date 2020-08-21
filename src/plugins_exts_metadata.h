/**
 * @file plugins_exts_metadata.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief ietf-yang-metadata API
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_METADATA_H_
#define LY_PLUGINS_EXTS_METADATA_H_

#include "tree_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Representation of the compiled metadata substatements as provided by libyang 2 metadata extension plugin.
 */
struct lyext_metadata {
    struct lysc_type *type;            /**< type of the metadata (mandatory) */
    const char *units;                 /**< units of the leaf's type */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    uint16_t flags;                    /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
};

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_METADATA_H_ */
