/**
 * @file schema_diff.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema comparison header.
 *
 * Copyright (c) 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_DIFF_H_
#define LY_SCHEMA_DIFF_H_

#include <stdint.h>

#include "log.h"
#include "ly_common.h"

/**
 * @brief Type of a schema change.
 */
enum lysc_diff_change_e {
    LYSC_CHANGE_MODIFIED,   /**< statement was modified in any way, used when none other apply */
    LYSC_CHANGE_ADDED,      /**< statement was added */
    LYSC_CHANGE_REMOVED,    /**< statement was removed */
    LYSC_CHANGE_MOVED       /**< statement was moved, if the order has meaning */
};

/**
 * @brief Changed statement of a schema change.
 */
enum lysc_diff_changed_e {
    LYSC_CHANGED_NONE = 0,
    LYSC_CHANGED_BASE,
    LYSC_CHANGED_BIT,
    LYSC_CHANGED_CONFIG,
    LYSC_CHANGED_CONTACT,
    LYSC_CHANGED_DEFAULT,
    LYSC_CHANGED_DESCRIPTION,
    LYSC_CHANGED_ENUM,
    LYSC_CHANGED_ERR_APP_TAG,
    LYSC_CHANGED_ERR_MSG,
    LYSC_CHANGED_EXT_INST,
    LYSC_CHANGED_FRAC_DIG,
    LYSC_CHANGED_IDENT,
    LYSC_CHANGED_LENGTH,
    LYSC_CHANGED_MANDATORY,
    LYSC_CHANGED_MAX_ELEM,
    LYSC_CHANGED_MIN_ELEM,
    LYSC_CHANGED_MUST,
    LYSC_CHANGED_NODE,
    LYSC_CHANGED_ORDERED_BY,
    LYSC_CHANGED_ORGANIZATION,
    LYSC_CHANGED_PATH,
    LYSC_CHANGED_PATTERN,
    LYSC_CHANGED_PRESENCE,
    LYSC_CHANGED_RANGE,
    LYSC_CHANGED_REFERENCE,
    LYSC_CHANGED_REQ_INSTANCE,
    LYSC_CHANGED_STATUS,
    LYSC_CHANGED_TYPE,
    LYSC_CHANGED_UNITS,
    LYSC_CHANGED_UNIQUE,
    LYSC_CHANGED_WHEN
};

/**
 * @brief Structure for a schema change.
 */
struct lysc_diff_change_s {
    enum lysc_diff_change_e change;             /**< type of change of the node */
    enum lysc_diff_changed_e parent_changed;    /**< type of the parent statement */
    enum lysc_diff_changed_e changed;           /**< type of the changed statement */
    ly_bool is_nbc;                             /**< flag to mark a non-backward-compatible change */
};

/**
 * @brief Structure for an array of changes.
 */
struct lysc_diff_changes_s {
    struct lysc_diff_change_s *changes; /**< array of changes */
    uint32_t count;                     /**< count of changes */
};

/**
 * @brief Structure for an array of extension-instance changes.
 */
struct lysc_diff_ext_changes_s {
    struct lysc_diff_ext_change_s *changes; /**< array of ext-instance changes */
    uint32_t count;                         /**< count of ext-instance changes */
};

/**
 * @brief Structure for an extension-instance change.
 */
struct lysc_diff_ext_change_s {
    const struct lysc_ext_instance *ext_old;    /**< old compiled extension-instance */
    const struct lysc_ext_instance *ext_new;    /**< new compiled extension-instance */
    struct lysc_diff_changes_s *changes;        /**< pointer to changes in the old and new extension-instance, may be empty */
};

/**
 * @brief Structure for a node schema change.
 */
struct lysc_diff_node_change_s {
    const struct lysc_node *snode_old;          /**< schema node from the old revision of the YANG module */
    const struct lysc_node *snode_new;          /**< schema node from the new revision of the YANG module */
    struct lysc_diff_changes_s changes;         /**< changes in the old and new schema node, may be empty */
    struct lysc_diff_ext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for an identity change.
 */
struct lysc_diff_ident_change_s {
    const struct lysc_ident *ident_old;         /**< old compiled identity */
    const struct lysp_ident *p_ident_old;       /**< old parsed identity */
    const struct lysc_ident *ident_new;         /**< new compiled identity */
    const struct lysp_ident *p_ident_new;       /**< new parsed identity */
    struct lysc_diff_changes_s changes;         /**< changes in the old and new identity, may be empty */
    struct lysc_diff_ext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for a full schema comparison.
 */
struct lysc_diff_s {
    struct lysc_diff_changes_s module_changes;      /**< module changes */
    struct lysc_diff_ident_change_s *ident_changes; /**< array of all the changed identities */
    uint32_t ident_change_count;                    /**< count of ident changes */
    struct lysc_diff_ext_changes_s mod_ext_changes; /**< module extension-instance changes */
    struct lysc_diff_node_change_s *node_changes;   /**< array of all the nodes and their changes */
    uint32_t node_change_count;                     /**< count of node changes */
    ly_bool is_yang10;                              /**< marks using YANG 1.0 update rules */
    ly_bool diff_parsed;                            /**< marks generating diff for parsed schema in addition to compiled */
    ly_bool is_nbc;                                 /**< flag to mark a non-backwards-compatible change */
};

/**
 * @brief Collect a diff of 2 modules.
 *
 * @param[in] mod1 Source module.
 * @param[in] mod2 Target module.
 * @param[in,out] diff Collected diff.
 * @return LY_ERR value.
 */
LY_ERR lysc_diff_changes(const struct lys_module *mod1, const struct lys_module *mod2, struct lysc_diff_s *diff);

/**
 * @brief Erase a diff structure.
 *
 * @param[in] diff Diff structure to erase.
 */
void lysc_diff_erase(struct lysc_diff_s *diff);

/**
 * @brief Create a schema-comparison data tree based on a collected diff.
 *
 * @param[in] mod1 Source module.
 * @param[in] mod2 Target module.
 * @param[in] diff Collected diff of @p mod1 and @p mod2.
 * @param[in] cmp_mod YANG module 'ietf-schema-comparison' to use.
 * @param[out] schema_diff Created data tree.
 * @return LY_ERR value.
 */
LY_ERR lysc_diff_tree(const struct lys_module *mod1, const struct lys_module *mod2, const struct lysc_diff_s *diff,
        const struct lys_module *cmp_mod, struct lyd_node **schema_diff);

#endif /* LY_SCHEMA_DIFF_H_ */
