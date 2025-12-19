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
 * @brief Check a node type for a prased-only node.
 *
 * @param[in] nodetype Node type to check.
 */
#define LYS_DIFF_NODE_PARSED(nodetype) ((nodetype) & (LYS_CHOICE | LYS_CASE | LYS_USES | LYS_INPUT | LYS_OUTPUT | LYS_GROUPING | LYS_AUGMENT))

/**
 * @brief Type of a schema change.
 */
enum lys_diff_change_e {
    LYS_CHANGE_MODIFIED,    /**< statement was modified in any way, used when none other apply */
    LYS_CHANGE_ADDED,       /**< statement was added */
    LYS_CHANGE_REMOVED,     /**< statement was removed */
    LYS_CHANGE_MOVED        /**< statement was moved, if the order has meaning */
};

/**
 * @brief Changed statement of a schema change.
 */
enum lys_diff_changed_e {
    LYS_CHANGED_NONE = 0,
    LYS_CHANGED_BASE,
    LYS_CHANGED_BIT,
    LYS_CHANGED_CONFIG,
    LYS_CHANGED_CONTACT,
    LYS_CHANGED_DEFAULT,
    LYS_CHANGED_DESCRIPTION,
    LYS_CHANGED_ENUM,
    LYS_CHANGED_ERR_APP_TAG,
    LYS_CHANGED_ERR_MSG,
    LYS_CHANGED_EXT_INST,
    LYS_CHANGED_FRAC_DIG,
    LYS_CHANGED_IDENT,
    LYS_CHANGED_LENGTH,
    LYS_CHANGED_MANDATORY,
    LYS_CHANGED_MAX_ELEM,
    LYS_CHANGED_MIN_ELEM,
    LYS_CHANGED_MUST,
    LYS_CHANGED_NODE,
    LYS_CHANGED_ORDERED_BY,
    LYS_CHANGED_ORGANIZATION,
    LYS_CHANGED_PATH,
    LYS_CHANGED_PATTERN,
    LYS_CHANGED_PRESENCE,
    LYS_CHANGED_RANGE,
    LYS_CHANGED_REFERENCE,
    LYS_CHANGED_REQ_INSTANCE,
    LYS_CHANGED_STATUS,
    LYS_CHANGED_TYPE,
    LYS_CHANGED_UNITS,
    LYS_CHANGED_UNIQUE,
    LYS_CHANGED_WHEN,

    /* parsed-schema */
    LYS_CHANGED_PREFIX,
    LYS_CHANGED_REFINE,
    LYS_CHANGED_TYPEDEF
};

/**
 * @brief Structure for a schema change.
 */
struct lys_diff_change_s {
    enum lys_diff_change_e change;              /**< type of change of the node */
    enum lys_diff_changed_e parent_changed;     /**< type of the parent statement */
    enum lys_diff_changed_e changed;            /**< type of the changed statement */
    ly_bool is_nbc;                             /**< flag to mark a non-backward-compatible change */
};

/**
 * @brief Structure for an array of changes.
 */
struct lys_diff_changes_s {
    struct lys_diff_change_s *changes;  /**< array of changes */
    uint32_t count;                     /**< count of changes */
};

/**
 * @brief Structure for an array of extension-instance changes.
 */
struct lys_diff_ext_changes_s {
    struct lys_diff_ext_change_s *changes;  /**< array of ext-instance changes */
    uint32_t count;                         /**< count of ext-instance changes */
};

/**
 * @brief Structure for an extension-instance change.
 */
struct lys_diff_ext_change_s {
    const struct lysc_ext_instance *ext_old;    /**< old compiled extension-instance */
    const struct lysc_ext_instance *ext_new;    /**< new compiled extension-instance */
    struct lys_diff_changes_s *changes;         /**< pointer to changes in the old and new extension-instance, may be empty */
};

/**
 * @brief Structure for an identity change.
 */
struct lys_diff_ident_change_s {
    const struct lysc_ident *ident_old;         /**< old compiled identity */
    const struct lysp_ident *p_ident_old;       /**< old parsed identity */
    const struct lysc_ident *ident_new;         /**< new compiled identity */
    const struct lysp_ident *p_ident_new;       /**< new parsed identity */
    struct lys_diff_changes_s changes;          /**< changes in the old and new identity, may be empty */
    struct lys_diff_ext_changes_s ext_changes;  /**< extension-instance changes */
};

/**
 * @brief Structure for a refine change.
 */
struct lys_diff_refine_change_s {
    const struct lysp_refine *refine_old;       /**< old parsed refine */
    const struct lysp_refine *refine_new;       /**< new parsed refine */
    const struct lysp_node *parent_new;         /**< new parent parsed node of the refine */
    struct lys_diff_changes_s changes;          /**< changes in the old and new refine, may be empty */
};

/**
 * @brief Structure for a typedef change.
 */
struct lys_diff_typedef_change_s {
    const struct lysp_tpdf *typedef_old;        /**< old parsed typedef */
    const struct lysp_tpdf *typedef_new;        /**< new parsed typedef */
    const struct lysp_node *parent_new;         /**< new parent parsed node of the typedef */
    struct lys_diff_changes_s changes;          /**< changes in the old and new typedef, may be empty */
};

/**
 * @brief Structure for a parsed node schema change.
 */
struct lys_diff_pnode_change_s {
    const struct lysp_node *pnode_old;          /**< parsed schema node from the old revision of the YANG module */
    const struct lysp_node *pnode_new;          /**< parsed schema node from the new revision of the YANG module */
    struct lys_diff_changes_s changes;          /**< changes in the old and new schema node, may be empty */
};

/**
 * @brief Structure for a node schema change.
 */
struct lys_diff_node_change_s {
    const struct lysc_node *snode_old;          /**< schema node from the old revision of the YANG module */
    const struct lysc_node *snode_new;          /**< schema node from the new revision of the YANG module */
    struct lys_diff_changes_s changes;          /**< changes in the old and new schema node, may be empty */
    struct lys_diff_ext_changes_s ext_changes;  /**< extension-instance changes */
};

/**
 * @brief Structure for a full schema comparison.
 */
struct lys_diff_s {
    struct lys_diff_changes_s module_changes;       /**< module changes */
    struct lys_diff_ident_change_s *ident_changes;  /**< array of all the changed identities */
    uint32_t ident_change_count;                    /**< count of ident changes */
    struct lys_diff_ext_changes_s mod_ext_changes;  /**< module extension-instance changes */
    struct lys_diff_pnode_change_s *pnode_changes;  /**< array of all the parsed-only nodes and their changes */
    uint32_t pnode_change_count;                    /**< count of pnode changes */
    struct lys_diff_refine_change_s *refine_changes;    /**< array of all the changed refines */
    uint32_t refine_change_count;                   /**< count of refine changes */
    struct lys_diff_typedef_change_s *typedef_changes;  /**< array of all the changed typedefs */
    uint32_t typedef_change_count;                  /**< count of typedef changes */
    struct lys_diff_node_change_s *node_changes;    /**< array of all the nodes and their changes */
    uint32_t node_change_count;                     /**< count of node changes */
    const char *old_prefix;                         /**< old module local prefix */
    const char *new_prefix;                         /**< new module local prefix */
    ly_bool is_yang10;                              /**< marks using YANG 1.0 update rules */
    ly_bool with_parsed;                            /**< marks generating diff for parsed schema in addition to compiled */
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
LY_ERR lysc_diff_changes(const struct lys_module *mod1, const struct lys_module *mod2, struct lys_diff_s *diff);

/**
 * @brief Erase a diff structure.
 *
 * @param[in] diff Diff structure to erase.
 */
void lysc_diff_erase(struct lys_diff_s *diff);

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
LY_ERR lysc_diff_tree(const struct lys_module *mod1, const struct lys_module *mod2, const struct lys_diff_s *diff,
        const struct lys_module *cmp_mod, struct lyd_node **schema_diff);

#endif /* LY_SCHEMA_DIFF_H_ */
