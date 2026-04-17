/**
 * @file schema_diff.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema comparison header.
 *
 * Copyright (c) 2025 - 2026 CESNET, z.s.p.o.
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
    LYS_CHANGED_NODE,
    LYS_CHANGED_BASE,
    LYS_CHANGED_BELONGS_TO,
    LYS_CHANGED_BIT,
    LYS_CHANGED_CONFIG,
    LYS_CHANGED_CONTACT,
    LYS_CHANGED_DEFAULT,
    LYS_CHANGED_DESCRIPTION,
    LYS_CHANGED_DEVIATE,
    LYS_CHANGED_DEVIATION,
    LYS_CHANGED_ENUM,
    LYS_CHANGED_ERR_APP_TAG,
    LYS_CHANGED_ERR_MSG,
    LYS_CHANGED_EXTENSION,
    LYS_CHANGED_EXT_INST,
    LYS_CHANGED_FEATURE,
    LYS_CHANGED_FRAC_DIG,
    LYS_CHANGED_IDENT,
    LYS_CHANGED_IF_FEATURE,
    LYS_CHANGED_IMPORT,
    LYS_CHANGED_INCLUDE,
    LYS_CHANGED_KEY,
    LYS_CHANGED_LENGTH,
    LYS_CHANGED_MANDATORY,
    LYS_CHANGED_MAX_ELEM,
    LYS_CHANGED_MIN_ELEM,
    LYS_CHANGED_MODIFIER,
    LYS_CHANGED_MODULE,
    LYS_CHANGED_MUST,
    LYS_CHANGED_NAMESPACE,
    LYS_CHANGED_ORDERED_BY,
    LYS_CHANGED_ORGANIZATION,
    LYS_CHANGED_PATH,
    LYS_CHANGED_PATTERN,
    LYS_CHANGED_POSITION,
    LYS_CHANGED_PREFIX,
    LYS_CHANGED_PRESENCE,
    LYS_CHANGED_RANGE,
    LYS_CHANGED_REFERENCE,
    LYS_CHANGED_REFINE,
    LYS_CHANGED_REQ_INSTANCE,
    LYS_CHANGED_REVISION,
    LYS_CHANGED_REVISION_DATE,
    LYS_CHANGED_STATUS,
    LYS_CHANGED_SUBMODULE,
    LYS_CHANGED_TYPE,
    LYS_CHANGED_TYPEDEF,
    LYS_CHANGED_UNIQUE,
    LYS_CHANGED_UNITS,
    LYS_CHANGED_VALUE,
    LYS_CHANGED_WHEN,
    LYS_CHANGED_YANG_VERSION
};

enum lys_diff_conform_e {
    LYS_CONFORM_ED,     /**< editorial change */
    LYS_CONFORM_BC,     /**< backwards-compatible change */
    LYS_CONFORM_NBC     /**< non-backwards-compatible change */
};

/**
 * @brief Structure for a schema change.
 */
struct lys_diff_change_s {
    enum lys_diff_change_e change;              /**< type of change of the node */
    enum lys_diff_changed_e parent_changed;     /**< type of the parent statement */
    enum lys_diff_changed_e changed;            /**< type of the changed statement */
    enum lys_diff_conform_e conform;            /**< conformance of the change */
};

/**
 * @brief Structure for an array of changes.
 */
struct lys_diff_changes_s {
    struct lys_diff_change_s *changes;  /**< array of changes */
    uint32_t count;                     /**< count of changes */
};

/**
 * @brief Structure for a compiled extension-instance change.
 */
struct lys_diff_ext_change_s {
    const struct lysc_ext_instance *ext_old;    /**< old compiled extension-instance */
    const struct lysc_ext_instance *ext_new;    /**< new compiled extension-instance */
    struct lys_diff_changes_s changes;          /**< changes in the old and new extension-instance, may be empty */
};

/**
 * @brief Structure for an array of compiled extension-instance changes.
 */
struct lys_diff_ext_changes_s {
    struct lys_diff_ext_change_s *changes;  /**< array of ext-instance changes */
    uint32_t count;                         /**< count of ext-instance changes */
};

/**
 * @brief Structure for a parsed extension-instance change.
 */
struct lys_diff_pext_change_s {
    const struct lysp_ext_instance *p_ext_old;  /**< old parsed extension-instance */
    const struct lysp_ext_instance *p_ext_new;  /**< new parsed extension-instance */
    struct lys_diff_changes_s changes;          /**< changes in the old and new extension-instance, may be empty */
};

/**
 * @brief Structure for an array of parsed extension-instance changes.
 */
struct lys_diff_pext_changes_s {
    struct lys_diff_pext_change_s *changes; /**< array of ext-instance changes */
    uint32_t count;                         /**< count of ext-instance changes */
};

/**
 * @brief Structure for an import change.
 */
struct lys_diff_import_change_s {
    const struct lysp_import *imp_old;          /**< old parsed import */
    const struct lysp_import *imp_new;          /**< new parsed import */
    struct lys_diff_changes_s changes;          /**< changes in the old and new import, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for an include change.
 */
struct lys_diff_include_change_s {
    const struct lysp_include *inc_old;         /**< old parsed include */
    const struct lysp_include *inc_new;         /**< new parsed include */
    struct lys_diff_changes_s changes;          /**< changes in the old and new include, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
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
 * @brief Structure for an extension change.
 */
struct lys_diff_extension_change_s {
    const struct lysp_ext *extension_old;       /**< old parsed extension */
    const struct lysp_ext *extension_new;       /**< new parsed extension */
    struct lys_diff_changes_s changes;          /**< changes in the old and new extension, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for a feature change.
 */
struct lys_diff_feat_change_s {
    const struct lysp_feature *feat_old;        /**< old parsed feature */
    const struct lysp_feature *feat_new;        /**< new parsed feature */
    struct lys_diff_changes_s changes;          /**< changes in the old and new feature, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for a deviation change.
 */
struct lys_diff_dev_change_s {
    const struct lysp_deviation *dev_old;       /**< old parsed deviation */
    const struct lysp_deviation *dev_new;       /**< new parsed deviation */
    struct lys_diff_changes_s changes;          /**< changes in the old and new deviation, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for a refine change.
 */
struct lys_diff_refine_change_s {
    const struct lysp_refine *refine_old;       /**< old parsed refine */
    const struct lysp_refine *refine_new;       /**< new parsed refine */
    const struct lysp_node *parent_new;         /**< new parent parsed node of the refine */
    struct lys_diff_changes_s changes;          /**< changes in the old and new refine, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for a typedef change.
 */
struct lys_diff_typedef_change_s {
    const struct lysp_tpdf *typedef_old;        /**< old parsed typedef */
    const struct lysp_tpdf *typedef_new;        /**< new parsed typedef */
    const struct lysp_node *parent_new;         /**< new parent parsed node of the typedef */
    struct lys_diff_changes_s changes;          /**< changes in the old and new typedef, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
};

/**
 * @brief Structure for a parsed node schema change.
 */
struct lys_diff_pnode_change_s {
    const struct lysp_node *pnode_old;          /**< parsed schema node from the old revision of the YANG module */
    const struct lysp_node *pnode_new;          /**< parsed schema node from the new revision of the YANG module */
    struct lys_diff_changes_s changes;          /**< changes in the old and new schema node, may be empty */
    struct lys_diff_pext_changes_s ext_changes; /**< extension-instance changes */
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
    struct lys_diff_ext_changes_s mod_ext_changes;  /**< module extension-instance changes */
    struct lys_diff_import_change_s *import_changes;    /**< array of all the changed imports */
    uint32_t import_change_count;                   /**< count of import changes */
    struct lys_diff_include_change_s *include_changes;  /**< array of all the changed includes */
    uint32_t include_change_count;                  /**< count of include changes */
    struct lys_diff_ident_change_s *ident_changes;  /**< array of all the changed identities */
    uint32_t ident_change_count;                    /**< count of ident changes */
    struct lys_diff_extension_change_s *extension_changes;  /**< array of all the changed extensions */
    uint32_t extension_change_count;                /**< count of extension changes */
    struct lys_diff_feat_change_s *feat_changes;    /**< array of all the changed features */
    uint32_t feat_change_count;                     /**< count of feat changes */
    struct lys_diff_dev_change_s *dev_changes;      /**< array of all the changed deviations */
    uint32_t dev_change_count;                      /**< count of dev changes */
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
    ly_bool with_priv_parsed;                       /**< marks compiled nodes having references to parsed nodes */
    enum lys_diff_conform_e conform;                /**< conformance of the whole diff */
    const struct ly_ctx *ctx;                       /**< context to use */
};

/**
 * @brief Add a new change into a schema node change pair.
 *
 * @param[in] change Type of change.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in] conform Conformance of the change.
 * @param[in,out] changes Changes to add the change to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_add_change(enum lys_diff_change_e change, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, enum lys_diff_conform_e conform, struct lys_diff_changes_s *changes);

/**
 * @brief Get the changed statement from a parser statement.
 *
 * @param[in] stmt Parser statement.
 * @return Changed statement.
 */
enum lys_diff_changed_e schema_diff_stmt2changed(enum ly_stmt stmt);

/**
 * @brief Get the module name from a node ID with a prefix.
 *
 * @param[in] ctx Contex to use.
 * @param[in] nodeid Node ID to parse.
 * @param[in] format Prefix format in @p nodeid.
 * @param[in] prefix_data Prefix data to use.
 * @param[out] mod_name Found module name.
 * @param[out] name Local name.
 */
void schema_diff_find_module(const struct ly_ctx *ctx, const char *nodeid, LY_VALUE_FORMAT format, void *prefix_data,
        const char **mod_name, const char **name);

/**
 * @brief Check changes of a text whose change is always considered ED.
 *
 * @param[in] text1 First text.
 * @param[in] text2 Second text.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in,out] changes Changes to add the change to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_text_ed(const char *text1, const char *text2, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of a text whose addition is considered BC.
 *
 * @param[in] text1 First text.
 * @param[in] text2 Second text.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_text_bc_add(const char *text1, const char *text2, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of a text whose any changes are considered NBC.
 *
 * @param[in] text1 First text.
 * @param[in] text2 Second text.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_text_nbc(const char *text1, const char *text2, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of a 'status'.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_status_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of a 'config'.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_config_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of a 'require-instance'.
 *
 * @param[in] req_inst1 First require-instance value.
 * @param[in] req_inst2 Second require-instance value.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_req_inst_change(uint8_t req_inst1, uint8_t req_inst2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of a 'min-elements' or 'max-elements' statements.
 *
 * @param[in] num1 First elements value.
 * @param[in] num1_set Whether @p num1 is explicitly set or not.
 * @param[in] num2 Second elements value.
 * @param[in] num2_set Whether @p num2 is explicitly set or not.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_elem_limit_change(uint32_t num1, int num1_set, uint32_t num2, int num2_set,
        enum lys_diff_changed_e parent_changed, enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of a 'range' or 'length'.
 *
 * @param[in] range1 First range/length.
 * @param[in] range2 Second range/length.
 * @param[in] sign If signed, is a range, otherwise length.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_node_type_range_change(const struct lysc_range *range1, const struct lysc_range *range2, ly_bool sign,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff);

/**
 * @brief Check changes of extension-instance arrays.
 *
 * @param[in] exts1 First ext-inst array.
 * @param[in] exts2 Second ext-inst array.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_ext_insts_change(const struct lysc_ext_instance *exts1, const struct lysc_ext_instance *exts2,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff);

/**
 * @brief Check changes of nodes, recursively.
 *
 * @param[in] node1 First node.
 * @param[in] node2 Second node.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_nodes_change_r(const struct lysc_node *node1, const struct lysc_node *node2, struct lys_diff_s *diff);

/**
 * @brief Check changes of an 'if-feature' array.
 *
 * @param[in] iffs1 First if-feature array.
 * @param[in] flags1 First node flags, if applicable.
 * @param[in] iffs2 Second if-feature array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_iffeatures_change(const struct lysp_qname *iffs1, uint16_t flags1, const struct lysp_qname *iffs2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes);

/**
 * @brief Check changes of parsed modules.
 *
 * @param[in] mod1 First parsed module.
 * @param[in] mod2 Second parsed module.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_pmodule_change(const struct lysp_module *mod1, const struct lysp_module *mod2, struct lys_diff_s *diff);

/**
 * @brief Check changes of 'identity' arrays.
 *
 * @param[in] idents1 First identity array.
 * @param[in] idents2 Second identity array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
LY_ERR schema_diff_module_identities_change(const struct lysc_ident *idents1, const struct lysc_ident *idents2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_s *diff);

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
