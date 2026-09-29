/**
 * @file semver.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-semver API
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_SEMVER_H_
#define LY_PLUGINS_EXTS_SEMVER_H_

#include <stdint.h>

#include "plugins_exts.h"
#include "tree_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief COMPAT modifier of semver.
 */
enum lys_ext_instance_semver_compat {
    LYS_EXT_SEMVER_COMPAT_NONE = 0,         /**< No COMPAT modifier. */
    LYS_EXT_SEMVER_COMPAT_COMPATIBLE,       /**< COMPAT modifier "compatible". */
    LYS_EXT_SEMVER_COMPAT_NON_COMPATIBLE    /**< COMPAT modifier "non_compatible". */
};

/**
 * @brief Structure with parsed semver version.
 *
 * MAJOR.MINOR.PATCH(_COMPAT)(-PRE_RELEASE)(+BUILD)
 */
struct lys_ext_instance_semver {
    int32_t major;                              /**< MAJOR version number. */
    int32_t minor;                              /**< MINOR version number. */
    int32_t patch;                              /**< PATCH version number. */
    enum lys_ext_instance_semver_compat compat; /**< Optional COMPAT version modifier. */
    char *pre_release_meta;                     /**< Optional PRE_RELEASE matadata. */
    char *build_meta;                           /**< Optional BUILD metadata. */
};

/**
 * @brief Get semantic version of a module.
 *
 * To get version of a submodule, use ::lysp_semver_get().
 *
 * @param[in] mod Module to use.
 * @param[out] semver_str Optional string semantic version if defined for @p mod at its current revision. NULL otherwise.
 * @return Semantic version structure;
 * @return NULL if no semantic version is defined for @p mod at its current revision.
 */
LIBYANG_API_DECL const struct lys_ext_instance_semver *lys_semver_get(const struct lys_module *mod,
        const char **semver_str);

/**
 * @brief Get semantic version of a parsed (sub)module.
 *
 * @param[in] pmod Parsed (sub)module to use.
 * @param[out] semver_str Optional string semantic version if defined for @p mod at its current revision. NULL otherwise.
 * @return Semantic version structure;
 * @return NULL if no semantic version is defined for @p pmod at its current revision.
 */
LIBYANG_API_DECL const struct lys_ext_instance_semver *lysp_semver_get(const struct lysp_module *pmod,
        const char **semver_str);

/**
 * @brief Compare semantic versions ignoring metadata.
 *
 * @param[in] ver1 First semantic version.
 * @param[in] ver2 Second semantic version.
 * @return -1 if ver1 < ver2,
 * @return 0 if ver1 == ver2,
 * @return 1 if ver1 > ver2;
 */
LIBYANG_API_DECL int lys_semver_cmp(const struct lys_ext_instance_semver *ver1,
        const struct lys_ext_instance_semver *ver2);

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_SEMVER_H_ */
