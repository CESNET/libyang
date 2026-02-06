/**
 * @file schema_diff_change.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema diff change functionss
 *
 * Copyright (c) 2025 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "schema_diff.h"

#include <assert.h>
#include <stdlib.h>

#include "ly_common.h"
#include "xpath.h"

static LY_ERR schema_diff_ptype_change(const struct lysp_type *type1, const struct lysp_type *type2,
        struct lys_diff_changes_s *changes);
static LY_ERR schema_diff_pnodes_change_r(const struct lysp_node *node1, const struct lysp_node *node2,
        struct lys_diff_s *diff);
static LY_ERR schema_diff_node_type_change(const struct lysc_type *type1, const struct lysc_type *type2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff);
static LY_ERR schema_diff_ext_insts_change(const struct lysc_ext_instance *exts1, const struct lysc_ext_instance *exts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff);
static LY_ERR schema_diff_nodes_change_r(const struct lysc_node *node1, const struct lysc_node *node2, struct lys_diff_s *diff);

/**
 * @brief Add a new schema node change pair.
 *
 * @param[in] snode_old Old changed schema node.
 * @param[in] snode_new New changed schema node.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] node_change Added node change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_node_change(const struct lysc_node *snode_old, const struct lysc_node *snode_new, struct lys_diff_s *diff,
        struct lys_diff_node_change_s **node_change)
{
    void *mem;
    struct lys_diff_node_change_s *nc;

    /* add new node_change */
    mem = realloc(diff->node_changes, (diff->node_change_count + 1) * sizeof *diff->node_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->node_changes = mem;
    nc = &diff->node_changes[diff->node_change_count];
    ++diff->node_change_count;

    /* fill new node_change */
    nc->snode_old = snode_old;
    nc->snode_new = snode_new;
    nc->changes.changes = NULL;
    nc->changes.count = 0;
    nc->ext_changes.changes = NULL;
    nc->ext_changes.count = 0;

    *node_change = nc;
    return LY_SUCCESS;
}

/**
 * @brief Add a new parsed schema node change pair.
 *
 * @param[in] pnode_old Old changed parsed schema node.
 * @param[in] pnode_new New changed parsed schema node.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] pnode_change Added pnode change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_pnode_change(const struct lysp_node *pnode_old, const struct lysp_node *pnode_new,
        struct lys_diff_s *diff, struct lys_diff_pnode_change_s **pnode_change)
{
    void *mem;
    struct lys_diff_pnode_change_s *pnc;

    /* add new pnode_change */
    mem = realloc(diff->pnode_changes, (diff->pnode_change_count + 1) * sizeof *diff->pnode_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->pnode_changes = mem;
    pnc = &diff->pnode_changes[diff->pnode_change_count];
    ++diff->pnode_change_count;

    /* fill new pnode_change */
    pnc->pnode_old = pnode_old;
    pnc->pnode_new = pnode_new;
    pnc->changes.changes = NULL;
    pnc->changes.count = 0;

    *pnode_change = pnc;
    return LY_SUCCESS;
}

/**
 * @brief Find parsed identity of a compiled identity.
 *
 * @param[in] ident Compiled identity.
 * @param[out] p_ident Found parsed identity.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_find_parsed_ident(const struct lysc_ident *ident, const struct lysp_ident **p_ident)
{
    const struct lysp_ident *idents;
    LY_ARRAY_COUNT_TYPE u, v;

    *p_ident = NULL;

    if (!ident) {
        /* no compiled nor parsed identity */
        return LY_SUCCESS;
    }

    /* find the parsed identity in the module */
    idents = ident->module->parsed->identities;
    LY_ARRAY_FOR(idents, u) {
        if (idents[u].name == ident->name) {
            *p_ident = &idents[u];
            break;
        }
    }

    if (!*p_ident) {
        /* find the parsed identity in submodules */
        LY_ARRAY_FOR(ident->module->parsed->includes, v) {
            idents = ident->module->parsed->includes[v].submodule->identities;
            LY_ARRAY_FOR(idents, u) {
                if (idents[u].name == ident->name) {
                    *p_ident = &idents[u];
                    break;
                }
            }

            if (*p_ident) {
                break;
            }
        }
    }

    if (!*p_ident) {
        LOGERR(ident->module->ctx, LY_EINT, "Failed to find parsed identity \"%s:%s\".", ident->module->name, ident->name);
        return LY_EINT;
    }

    return LY_SUCCESS;
}

/**
 * @brief Add a new identity change pair.
 *
 * @param[in] ident_old Old changed identity.
 * @param[in] ident_new New changed identity.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] ident_change Added identity change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_ident_change(const struct lysc_ident *ident_old, const struct lysc_ident *ident_new,
        struct lys_diff_s *diff, struct lys_diff_ident_change_s **ident_change)
{
    void *mem;
    struct lys_diff_ident_change_s *ic;

    /* add new ident_change */
    mem = realloc(diff->ident_changes, (diff->ident_change_count + 1) * sizeof *diff->ident_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->ident_changes = mem;
    ic = &diff->ident_changes[diff->ident_change_count];
    ++diff->ident_change_count;

    /* fill new ident_change */
    ic->ident_old = ident_old;
    LY_CHECK_RET(schema_diff_find_parsed_ident(ident_old, &ic->p_ident_old));
    ic->ident_new = ident_new;
    LY_CHECK_RET(schema_diff_find_parsed_ident(ident_new, &ic->p_ident_new));
    ic->changes.changes = NULL;
    ic->changes.count = 0;
    ic->ext_changes.changes = NULL;
    ic->ext_changes.count = 0;

    *ident_change = ic;
    return LY_SUCCESS;
}

/**
 * @brief Add a new import change pair.
 *
 * @param[in] imp_old Old changed import.
 * @param[in] imp_new New changed import.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] import_change Added import change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_import_change(const struct lysp_import *imp_old, const struct lysp_import *imp_new,
        struct lys_diff_s *diff, struct lys_diff_import_change_s **import_change)
{
    void *mem;
    struct lys_diff_import_change_s *ic;

    /* add new import_change */
    mem = realloc(diff->import_changes, (diff->import_change_count + 1) * sizeof *diff->import_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->import_changes = mem;
    ic = &diff->import_changes[diff->import_change_count];
    ++diff->import_change_count;

    /* fill new import_change */
    ic->imp_old = imp_old;
    ic->imp_new = imp_new;
    ic->changes.changes = NULL;
    ic->changes.count = 0;

    *import_change = ic;
    return LY_SUCCESS;
}

/**
 * @brief Add a new include change pair.
 *
 * @param[in] inc_old Old changed include.
 * @param[in] inc_new New changed include.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] include_change Added include change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_include_change(const struct lysp_include *inc_old, const struct lysp_include *inc_new,
        struct lys_diff_s *diff, struct lys_diff_include_change_s **include_change)
{
    void *mem;
    struct lys_diff_include_change_s *ic;

    /* add new include_change */
    mem = realloc(diff->include_changes, (diff->include_change_count + 1) * sizeof *diff->include_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->include_changes = mem;
    ic = &diff->include_changes[diff->include_change_count];
    ++diff->include_change_count;

    /* fill new include_change */
    ic->inc_old = inc_old;
    ic->inc_new = inc_new;
    ic->changes.changes = NULL;
    ic->changes.count = 0;

    *include_change = ic;
    return LY_SUCCESS;
}

/**
 * @brief Add a new extension change pair.
 *
 * @param[in] extension_old Old changed extension.
 * @param[in] extension_new New changed extension.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] extension_change Added extension change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_extension_change(const struct lysp_ext *extension_old, const struct lysp_ext *extension_new,
        struct lys_diff_s *diff, struct lys_diff_extension_change_s **extension_change)
{
    void *mem;
    struct lys_diff_extension_change_s *ec;

    /* add new extension_change */
    mem = realloc(diff->extension_changes, (diff->extension_change_count + 1) * sizeof *diff->extension_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->extension_changes = mem;
    ec = &diff->extension_changes[diff->extension_change_count];
    ++diff->extension_change_count;

    /* fill new extension_change */
    ec->extension_old = extension_old;
    ec->extension_new = extension_new;
    ec->changes.changes = NULL;
    ec->changes.count = 0;

    *extension_change = ec;
    return LY_SUCCESS;
}

/**
 * @brief Add a new feature change pair.
 *
 * @param[in] feat_old Old changed identity.
 * @param[in] feat_new New changed identity.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] feat_change Added identity change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_feat_change(const struct lysp_feature *feat_old, const struct lysp_feature *feat_new,
        struct lys_diff_s *diff, struct lys_diff_feat_change_s **feat_change)
{
    void *mem;
    struct lys_diff_feat_change_s *fc;

    /* add new feat_change */
    mem = realloc(diff->feat_changes, (diff->feat_change_count + 1) * sizeof *diff->feat_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->feat_changes = mem;
    fc = &diff->feat_changes[diff->feat_change_count];
    ++diff->feat_change_count;

    /* fill new feat_change */
    fc->feat_old = feat_old;
    fc->feat_new = feat_new;
    fc->changes.changes = NULL;
    fc->changes.count = 0;

    *feat_change = fc;
    return LY_SUCCESS;
}

/**
 * @brief Add a new devation change pair.
 *
 * @param[in] dev_old Old changed deviation.
 * @param[in] dev_new New changed devation.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] ident_change Added identity change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_dev_change(const struct lysp_deviation *dev_old, const struct lysp_deviation *dev_new,
        struct lys_diff_s *diff, struct lys_diff_dev_change_s **dev_change)
{
    void *mem;
    struct lys_diff_dev_change_s *dc;

    /* add new ident_change */
    mem = realloc(diff->dev_changes, (diff->dev_change_count + 1) * sizeof *diff->dev_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->dev_changes = mem;
    dc = &diff->dev_changes[diff->dev_change_count];
    ++diff->dev_change_count;

    /* fill new ident_change */
    dc->dev_old = dev_old;
    dc->dev_new = dev_new;
    dc->changes.changes = NULL;
    dc->changes.count = 0;

    *dev_change = dc;
    return LY_SUCCESS;
}

/**
 * @brief Add a new ext-instance change pair.
 *
 * @param[in] ext_old Old changed ext-instance.
 * @param[in] ext_new New changed ext-instance.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[out] ext_change Added ext-instance change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_ext_change(const struct lysc_ext_instance *ext_old, const struct lysc_ext_instance *ext_new,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_ext_change_s **ext_change)
{
    void *mem;
    struct lys_diff_ext_change_s *ec;

    /* add new ext_change */
    mem = realloc(ext_changes->changes, (ext_changes->count + 1) * sizeof *ext_changes->changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    ext_changes->changes = mem;
    ec = &ext_changes->changes[ext_changes->count];
    ++ext_changes->count;

    /* fill new ext_change */
    ec->ext_old = ext_old;
    ec->ext_new = ext_new;
    ec->changes = calloc(1, sizeof *ec->changes);
    LY_CHECK_ERR_RET(!ec->changes, LOGMEM(NULL), LY_EMEM);

    *ext_change = ec;
    return LY_SUCCESS;
}

/**
 * @brief Add a new refine change pair.
 *
 * @param[in] refine_old Old refine.
 * @param[in] refine_new New refine.
 * @param[in] parent_new New refine parent node.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] refine_change Added refine change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_refine_change(const struct lysp_refine *refine_old, const struct lysp_refine *refine_new,
        const struct lysp_node *parent_new, struct lys_diff_s *diff, struct lys_diff_refine_change_s **refine_change)
{
    void *mem;
    struct lys_diff_refine_change_s *rc;

    /* add new typedef_change */
    mem = realloc(diff->refine_changes, (diff->refine_change_count + 1) * sizeof *diff->refine_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->refine_changes = mem;
    rc = &diff->refine_changes[diff->refine_change_count];
    ++diff->refine_change_count;

    /* fill new refine_change */
    rc->refine_old = refine_old;
    rc->refine_new = refine_new;
    rc->parent_new = parent_new;
    rc->changes.changes = NULL;
    rc->changes.count = 0;

    *refine_change = rc;
    return LY_SUCCESS;
}

/**
 * @brief Add a new typedef change pair.
 *
 * @param[in] typedef_old Old typedef.
 * @param[in] typedef_new New typedef.
 * @param[in] parent_new New typedef parent node.
 * @param[in,out] diff Diff to use and add to.
 * @param[out] typedef_change Added typedef change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_typedef_change(const struct lysp_tpdf *typedef_old, const struct lysp_tpdf *typedef_new,
        const struct lysp_node *parent_new, struct lys_diff_s *diff, struct lys_diff_typedef_change_s **typedef_change)
{
    void *mem;
    struct lys_diff_typedef_change_s *tc;

    /* add new typedef_change */
    mem = realloc(diff->typedef_changes, (diff->typedef_change_count + 1) * sizeof *diff->typedef_changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    diff->typedef_changes = mem;
    tc = &diff->typedef_changes[diff->typedef_change_count];
    ++diff->typedef_change_count;

    /* fill new typedef_change */
    tc->typedef_old = typedef_old;
    tc->typedef_new = typedef_new;
    tc->parent_new = parent_new;
    tc->changes.changes = NULL;
    tc->changes.count = 0;

    *typedef_change = tc;
    return LY_SUCCESS;
}

/**
 * @brief Add a new change into a schema node change pair.
 *
 * @param[in] change Type of change.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in] is_nbc Set if the change is non-backwards-compatible.
 * @param[in,out] changes Changes to add the change to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_change(enum lys_diff_change_e change, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, ly_bool is_nbc, struct lys_diff_changes_s *changes)
{
    void *mem;
    struct lys_diff_change_s *c;

    /* add new change */
    mem = realloc(changes->changes, (changes->count + 1) * sizeof *changes->changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    changes->changes = mem;
    c = &changes->changes[changes->count];
    ++changes->count;

    /* fill new change */
    c->change = change;
    c->parent_changed = parent_changed;
    c->changed = changed;
    c->is_nbc = is_nbc;

    return LY_SUCCESS;
}

/**
 * @brief Check whether any change is NBC, mark it in the diff if so.
 *
 * @param[in] changes Changes to check.
 * @param[in,out] diff Diff to update.
 */
static void
schema_diff_check_node_change_nbc(const struct lys_diff_changes_s *changes, struct lys_diff_s *diff)
{
    uint32_t i;

    if (diff->is_nbc) {
        /* nothing to do anymore */
        return;
    }

    for (i = 0; i < changes->count; ++i) {
        if (changes->changes[i].is_nbc) {
            diff->is_nbc = 1;
            break;
        }
    }
}

/**
 * @brief Check whether there is 'backwards-compatible' extension in an array of ext-insts.
 *
 * @param[in] exts Sized-array of extension-instances.
 * @return 1 if the extension-instance is found;
 * @return 0 otherwise.
 */
static ly_bool
schema_diff_has_bc_ext(const struct lysc_ext_instance *exts)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(exts, u) {
        if (!strcmp(exts[u].def->module->name, "ietf-yang-schema-comparison") &&
                !strcmp(exts[u].def->name, "backwards-compatible")) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Check whether there is 'backwards-compatible' extension in an array of parsed ext-insts.
 *
 * @param[in] exts Sized-array of parsed extension-instances.
 * @return 1 if the extension-instance is found;
 * @return 0 otherwise.
 */
static ly_bool
schema_diff_parsed_has_bc_ext(const struct lysp_ext_instance *exts)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(exts, u) {
        if ((exts[u].parent_stmt == LY_STMT_DESCRIPTION) && strstr(exts[u].name, ":backwards-compatible") &&
                !exts[u].argument) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Get the changed statement from a parser statement.
 *
 * @param[in] stmt Parser statement.
 * @return Changed statement.
 */
static enum lys_diff_changed_e
schema_diff_stmt2changed(enum ly_stmt stmt)
{
    switch (stmt) {
    case LY_STMT_NONE:
        return LYS_CHANGED_NONE;
    case LY_STMT_ACTION:
    case LY_STMT_ANYDATA:
    case LY_STMT_ANYXML:
    case LY_STMT_CONTAINER:
    case LY_STMT_LEAF:
    case LY_STMT_LEAF_LIST:
    case LY_STMT_LIST:
    case LY_STMT_NOTIFICATION:
    case LY_STMT_RPC:
        return LYS_CHANGED_NODE;
    case LY_STMT_ARGUMENT:
    case LY_STMT_ARG_TEXT:
    case LY_STMT_ARG_VALUE:
    case LY_STMT_AUGMENT:
    case LY_STMT_BELONGS_TO:
    case LY_STMT_CASE:
    case LY_STMT_CHOICE:
    case LY_STMT_DEVIATE:
    case LY_STMT_DEVIATION:
    case LY_STMT_EXTENSION:
    case LY_STMT_FEATURE:
    case LY_STMT_GROUPING:
    case LY_STMT_IF_FEATURE:
    case LY_STMT_IMPORT:
    case LY_STMT_INCLUDE:
    case LY_STMT_INPUT:
    case LY_STMT_KEY:
    case LY_STMT_MODIFIER:
    case LY_STMT_MODULE:
    case LY_STMT_NAMESPACE:
    case LY_STMT_OUTPUT:
    case LY_STMT_POSITION:
    case LY_STMT_PREFIX:
    case LY_STMT_REFINE:
    case LY_STMT_REVISION:
    case LY_STMT_REVISION_DATE:
    case LY_STMT_SUBMODULE:
    case LY_STMT_SYNTAX_LEFT_BRACE:
    case LY_STMT_SYNTAX_RIGHT_BRACE:
    case LY_STMT_SYNTAX_SEMICOLON:
    case LY_STMT_TYPEDEF:
    case LY_STMT_USES:
    case LY_STMT_VALUE:
    case LY_STMT_YANG_VERSION:
    case LY_STMT_YIN_ELEMENT:
        /* invalid */
        break;
    case LY_STMT_BASE:
        return LYS_CHANGED_BASE;
    case LY_STMT_BIT:
        return LYS_CHANGED_BIT;
    case LY_STMT_CONFIG:
        return LYS_CHANGED_CONFIG;
    case LY_STMT_CONTACT:
        return LYS_CHANGED_CONTACT;
    case LY_STMT_DEFAULT:
        return LYS_CHANGED_DEFAULT;
    case LY_STMT_DESCRIPTION:
        return LYS_CHANGED_DESCRIPTION;
    case LY_STMT_ENUM:
        return LYS_CHANGED_ENUM;
    case LY_STMT_ERROR_APP_TAG:
        return LYS_CHANGED_ERR_APP_TAG;
    case LY_STMT_ERROR_MESSAGE:
        return LYS_CHANGED_ERR_MSG;
    case LY_STMT_EXTENSION_INSTANCE:
        return LYS_CHANGED_EXT_INST;
    case LY_STMT_FRACTION_DIGITS:
        return LYS_CHANGED_FRAC_DIG;
    case LY_STMT_IDENTITY:
        return LYS_CHANGED_IDENT;
    case LY_STMT_LENGTH:
        return LYS_CHANGED_LENGTH;
    case LY_STMT_MANDATORY:
        return LYS_CHANGED_MANDATORY;
    case LY_STMT_MAX_ELEMENTS:
        return LYS_CHANGED_MAX_ELEM;
    case LY_STMT_MIN_ELEMENTS:
        return LYS_CHANGED_MIN_ELEM;
    case LY_STMT_MUST:
        return LYS_CHANGED_MUST;
    case LY_STMT_ORDERED_BY:
        return LYS_CHANGED_ORDERED_BY;
    case LY_STMT_ORGANIZATION:
        return LYS_CHANGED_ORGANIZATION;
    case LY_STMT_PATH:
        return LYS_CHANGED_PATH;
    case LY_STMT_PATTERN:
        return LYS_CHANGED_PATTERN;
    case LY_STMT_PRESENCE:
        return LYS_CHANGED_PRESENCE;
    case LY_STMT_RANGE:
        return LYS_CHANGED_RANGE;
    case LY_STMT_REFERENCE:
        return LYS_CHANGED_REFERENCE;
    case LY_STMT_REQUIRE_INSTANCE:
        return LYS_CHANGED_REQ_INSTANCE;
    case LY_STMT_STATUS:
        return LYS_CHANGED_STATUS;
    case LY_STMT_TYPE:
        return LYS_CHANGED_TYPE;
    case LY_STMT_UNIQUE:
        return LYS_CHANGED_UNIQUE;
    case LY_STMT_UNITS:
        return LYS_CHANGED_UNITS;
    case LY_STMT_WHEN:
        return LYS_CHANGED_WHEN;
    }

    return LYS_CHANGED_NONE;
}

/**
 * @brief Check changes of a text whose change is considered BC.
 *
 * @param[in] text1 First text.
 * @param[in] text2 Second text.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in,out] changes Changes to add the change to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_text_bc(const char *text1, const char *text2, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes)
{
    if (text1 && !text2) {
        /* removed, always NBC */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, 1, changes));
    } else if (!text1 && text2) {
        /* added */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, 0, changes));
    } else if (text1 && text2 && strcmp(text1, text2)) {
        /* modified */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 0, changes));
    }

    return LY_SUCCESS;
}

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
static LY_ERR
schema_diff_text_bc_add(const char *text1, const char *text2, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes)
{
    if (!text1 && text2) {
        /* added, BC */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, 0, changes));
    } else if (text1 && !text2) {
        /* removed */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, 1, changes));
    } else if (text1 && text2 && strcmp(text1, text2)) {
        /* modified */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 1, changes));
    }

    return LY_SUCCESS;
}

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
static LY_ERR
schema_diff_text_nbc(const char *text1, const char *text2, enum lys_diff_changed_e parent_changed,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes)
{
    if (text1 && !text2) {
        /* removed */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, 1, changes));
    } else if (!text1 && text2) {
        /* added */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, 1, changes));
    } else if (text1 && text2 && strcmp(text1, text2)) {
        /* modified */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 1, changes));
    }

    return LY_SUCCESS;
}

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
static LY_ERR
schema_diff_iffeatures_change(const struct lysp_qname *iffs1, uint16_t flags1, const struct lysp_qname *iffs2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *iff2_found = NULL, is_nbc;

    /* prepare array for marking found if-features */
    iff2_found = calloc(LY_ARRAY_COUNT(iffs2), sizeof *iff2_found);
    LY_CHECK_ERR_GOTO(!iff2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)

    LY_ARRAY_FOR(iffs1, u) {
        found = 0;
        LY_ARRAY_FOR(iffs2, v) {
            if (iff2_found[v]) {
                continue;
            }

            /* text value */
            if (!strcmp(iffs1[u].str, iffs2[v].str)) {
                found = 1;
                iff2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            is_nbc = (flags1 & LYS_MAND_TRUE) ? 1 : 0;
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_IF_FEATURE, is_nbc,
                    changes), cleanup);
        }
    }

    LY_ARRAY_FOR(iffs2, v) {
        if (iff2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_IF_FEATURE, 1, changes),
                cleanup);
    }

cleanup:
    free(iff2_found);
    return rc;
}

/**
 * @brief Check changes of a 'status'.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_status_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes)
{
    flags1 &= LYS_STATUS_MASK;
    flags2 &= LYS_STATUS_MASK;

    if ((flags1 == LYS_STATUS_CURR) && (flags2 == LYS_STATUS_DEPRC)) {
        /* current -> deprecated */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_STATUS, 0, changes));
    } else if (flags1 != flags2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_STATUS, 1, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a 'config'.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_config_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes)
{
    if ((flags1 & LYS_CONFIG_R) && (flags2 & LYS_CONFIG_W) && !(flags2 & LYS_MAND_TRUE)) {
        /* state -> configuration, not mandatory */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_CONFIG, 0, changes));
    } else if ((flags1 & LYS_CONFIG_MASK) != (flags2 & LYS_CONFIG_MASK)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_CONFIG, 1, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a 'require-instance'.
 *
 * @param[in] req_inst1 First require-instance value.
 * @param[in] req_inst2 Second require-instance value.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_req_inst_change(uint8_t req_inst1, uint8_t req_inst2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes)
{
    if (req_inst1 > req_inst2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_REQ_INSTANCE, 0,
                changes));
    } else if (req_inst1 != req_inst2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_REQ_INSTANCE, 1,
                changes));
    }

    return LY_SUCCESS;
}

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
static LY_ERR
schema_diff_elem_limit_change(uint32_t num1, int num1_set, uint32_t num2, int num2_set,
        enum lys_diff_changed_e parent_changed, enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes)
{
    if (!num1_set && !num2_set) {
        /* nothing set */
        return LY_SUCCESS;
    } else if (!num1_set) {
        /* added */
        return schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, 1, changes);
    } else if (!num2_set) {
        /* removed */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, 0, changes);
    }

    /* modified */
    if ((changed == LYS_CHANGED_MIN_ELEM) && (num1 > num2)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 0, changes));
    } else if ((changed == LYS_CHANGED_MAX_ELEM) && (num1 < num2)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 0, changes));
    } else if (num1 != num2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 1, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a 'description'.
 *
 * @param[in] dsc1 First description.
 * @param[in] dsc2 Second description.
 * @param[in] exts2 Extension instances of the second statement.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_description(const char *dsc1, const char *dsc2, const struct lysc_ext_instance *exts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool is_nbc;

    if (dsc1 && !dsc2) {
        /* removed, always NBC */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_DESCRIPTION, 1, changes));
    } else if (!dsc1 && dsc2) {
        /* added */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_DESCRIPTION, 0, changes));
    } else if (dsc1 && dsc2 && strcmp(dsc1, dsc2)) {
        /* modified, look for BC extension */
        is_nbc = 1;
        LY_ARRAY_FOR(exts2, u) {
            if ((exts2[u].parent_stmt == LY_STMT_DESCRIPTION) &&
                    !strcmp(exts2[u].def->module->name, "ietf-yang-schema-comparison") &&
                    !strcmp(exts2[u].def->name, "backwards-compatible")) {
                is_nbc = 0;
                break;
            }
        }
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_DESCRIPTION, is_nbc,
                changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a parsed 'description'.
 *
 * @param[in] dsc1 First description.
 * @param[in] dsc2 Second description.
 * @param[in] exts2 Parsed extension instances of the second statement.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode_description(const char *dsc1, const char *dsc2, const struct lysp_ext_instance *exts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    ly_bool is_nbc;

    if (dsc1 && !dsc2) {
        /* removed, always NBC */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_DESCRIPTION, 1, changes));
    } else if (!dsc1 && dsc2) {
        /* added */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_DESCRIPTION, 0, changes));
    } else if (dsc1 && dsc2 && strcmp(dsc1, dsc2)) {
        /* modified, check BC extension */
        is_nbc = !schema_diff_parsed_has_bc_ext(exts2);
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_DESCRIPTION, is_nbc,
                changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a parsed restriction, which can be 'length', 'range', 'pattern', or 'must'.
 *
 * @param[in] restr1 First restriction.
 * @param[in] restr2 Second restriction.
 * @param[in] changed Changed statement.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_restr_change(const struct lysp_restr *restr1, const struct lysp_restr *restr2,
        enum lys_diff_changed_e changed, enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;

    if (!restr1 && !restr2) {
        /* no changes */
        goto cleanup;
    } else if (!restr2) {
        /* removed */
        rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, 0, changes);
        goto cleanup;
    } else if (!restr1) {
        rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, 1, changes);
        goto cleanup;
    }

    /* condition */
    if (changed == LYS_CHANGED_PATTERN) {
        if ((restr1->arg.str[0] != restr2->arg.str[0]) || strcmp(restr1->arg.str + 1, restr2->arg.str + 1)) {
            /* different restrictions */
            return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 1, changes);
        }
    } else {
        if (strcmp(restr1->arg.str, restr2->arg.str)) {
            /* different restrictions */
            return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 1, changes);
        }
    }

    /* description, reference, error-message, error-app-tag */
    LY_CHECK_GOTO(rc = schema_diff_pnode_description(restr1->dsc, restr2->dsc, restr2->exts, changed, changes), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_text_bc(restr1->ref, restr2->ref, changed, LYS_CHANGED_REFERENCE, changes), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(restr1->emsg, restr2->emsg, changed, LYS_CHANGED_ERR_MSG, changes), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(restr1->eapptag, restr2->eapptag, changed, LYS_CHANGED_ERR_APP_TAG, changes),
            cleanup);

cleanup:
    return rc;
}

/**
 * @brief Check changes of a parsed array of restrictions which can be 'length', 'range', 'pattern', or 'must'.
 *
 * @param[in] restrs1 First restriction array.
 * @param[in] restrs2 Second restriction array.
 * @param[in] changed Changed statement.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_restrs_change(const struct lysp_restr *restrs1, const struct lysp_restr *restrs2,
        enum lys_diff_changed_e changed, enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *restr2_found = NULL;

    if (restrs2) {
        /* prepare array for marking found bases */
        restr2_found = calloc(LY_ARRAY_COUNT(restrs2), sizeof *restr2_found);
        LY_CHECK_ERR_GOTO(!restr2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)
    }

    LY_ARRAY_FOR(restrs1, u) {
        found = 0;
        LY_ARRAY_FOR(restrs2, v) {
            if (restr2_found[v]) {
                continue;
            }

            /* condition */
            if (changed == LYS_CHANGED_PATTERN) {
                if ((restrs1[u].arg.str[0] == restrs2[v].arg.str[0]) &&
                        !strcmp(restrs1[u].arg.str + 1, restrs2[v].arg.str + 1)) {
                    found = 1;
                }
            } else {
                if (!strcmp(restrs1[u].arg.str, restrs2[v].arg.str)) {
                    found = 1;
                }
            }
            if (found) {
                /* found */
                restr2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, 0, changes),
                    cleanup);
            continue;
        }

        /* restriction */
        LY_CHECK_GOTO(rc = schema_diff_parsed_restr_change(&restrs1[u], &restrs2[v], changed, parent_changed, changes),
                cleanup);
    }

    LY_ARRAY_FOR(restrs2, v) {
        if (restr2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, 1, changes), cleanup);
    }

cleanup:
    free(restr2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'enum' or 'bit' array.
 *
 * @param[in] enums1 First array.
 * @param[in] enums2 Second array.
 * @param[in] changed Changed statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptype_enums_change(const struct lysp_type_enum *enums1, const struct lysp_type_enum *enums2,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *enum2_found = NULL;

    /* prepare array for marking found bases */
    enum2_found = calloc(LY_ARRAY_COUNT(enums2), sizeof *enum2_found);
    LY_CHECK_ERR_GOTO(!enum2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)

    LY_ARRAY_FOR(enums1, u) {
        found = 0;
        LY_ARRAY_FOR(enums2, v) {
            if (enum2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(enums1[u].name, enums2[v].name)) {
                /* found */
                found = 1;
                enum2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_TYPEDEF, changed, 1, changes),
                    cleanup);
            continue;
        }

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(enums1[u].dsc, enums2[v].dsc, enums2[v].exts, changed, changes),
                cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(enums1[u].ref, enums2[v].ref, changed, LYS_CHANGED_REFERENCE, changes),
                cleanup);

        /* value/position, does not matter */
        if (enums1[u].value != enums2[v].value) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_TYPEDEF, changed, 1, changes),
                    cleanup);
        }

        /* if-features */
        LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(enums1[u].iffeatures, 0, enums2[v].iffeatures, changed, changes),
                cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(enums1[u].flags, enums2[v].flags, changed, changes), cleanup);
    }

    LY_ARRAY_FOR(enums2, v) {
        if (enum2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_TYPEDEF, changed, 0, changes), cleanup);
    }

cleanup:
    free(enum2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'base' array.
 *
 * @param[in] bases1 First base array.
 * @param[in] bases2 Second base array.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptype_bases_change(const char **bases1, const char **bases2, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *base2_found = NULL;

    /* prepare array for marking found bases */
    base2_found = calloc(LY_ARRAY_COUNT(bases2), sizeof *base2_found);
    LY_CHECK_ERR_GOTO(!base2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)

    LY_ARRAY_FOR(bases1, u) {
        found = 0;
        LY_ARRAY_FOR(bases2, v) {
            if (base2_found[v]) {
                continue;
            }

            /* condition */
            if (!strcmp(bases1[u], bases2[v])) {
                /* found */
                found = 1;
                base2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_BASE, 0,
                    changes), cleanup);
        }
    }

    LY_ARRAY_FOR(bases2, v) {
        if (base2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_BASE, 1,
                changes), cleanup);
    }

cleanup:
    free(base2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'type' array.
 *
 * @param[in] types1 First type array.
 * @param[in] types2 Second type array.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptypes_change(const struct lysp_type *types1, const struct lysp_type *types2,
        struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;

    if (LY_ARRAY_COUNT(types1) > LY_ARRAY_COUNT(types2)) {
        /* removed */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_TYPE, 1,
                changes), cleanup);
    } else if (LY_ARRAY_COUNT(types1) < LY_ARRAY_COUNT(types2)) {
        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_TYPE, 1,
                changes), cleanup);
    }

    LY_ARRAY_FOR(types1, u) {
        if (strcmp(types1[u].name, types2[u].name)) {
            /* modified */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_TYPE, 1,
                    changes), cleanup);
        } else {
            /* type */
            LY_CHECK_GOTO(rc = schema_diff_ptype_change(&types1[u], &types2[u], changes), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Check changes of a parsed 'type'.
 *
 * @param[in] type1 First type.
 * @param[in] type2 Second type.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptype_change(const struct lysp_type *type1, const struct lysp_type *type2, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;

    /* basetype */
    if (strcmp(type1->name, type2->name)) {
        /* different types */
        return schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_TYPE, 1, changes);
    }

    /* range */
    LY_CHECK_GOTO(rc = schema_diff_parsed_restr_change(type1->range, type2->range, LYS_CHANGED_RANGE,
            LYS_CHANGED_TYPEDEF, changes), cleanup);

    /* length */
    LY_CHECK_GOTO(rc = schema_diff_parsed_restr_change(type1->length, type2->length, LYS_CHANGED_LENGTH,
            LYS_CHANGED_TYPEDEF, changes), cleanup);

    /* patterns */
    LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(type1->patterns, type2->patterns, LYS_CHANGED_PATTERN,
            LYS_CHANGED_TYPEDEF, changes), cleanup);

    /* enums */
    LY_CHECK_GOTO(rc = schema_diff_ptype_enums_change(type1->enums, type2->enums, LYS_CHANGED_ENUM, changes), cleanup);

    /* bits */
    LY_CHECK_GOTO(rc = schema_diff_ptype_enums_change(type1->bits, type2->bits, LYS_CHANGED_BIT, changes), cleanup);

    /* path */
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(type1->path ? type1->path->expr : NULL,
            type2->path ? type2->path->expr : NULL, LYS_CHANGED_TYPEDEF, LYS_CHANGED_PATH, changes), cleanup);

    /* bases */
    LY_CHECK_GOTO(rc = schema_diff_ptype_bases_change(type1->bases, type2->bases, changes), cleanup);

    /* types */
    LY_CHECK_GOTO(rc = schema_diff_ptypes_change(type1->types, type2->types, changes), cleanup);

    /* fraction-digits */
    if (type1->fraction_digits != type2->fraction_digits) {
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_FRAC_DIG, 1,
                changes), cleanup);
    }

    /* require-instance */
    LY_CHECK_GOTO(rc = schema_diff_req_inst_change(type1->require_instance, type2->require_instance, LYS_CHANGED_TYPEDEF,
            changes), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Check changes of a parsed 'typedef' array.
 *
 * @param[in] typedefs1 First typedef array.
 * @param[in] typedefs2 Second typedef array.
 * @param[in] parent2 Second typedef array parent node.
 * @param[in] parent_changed Changed parent statement.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_typedefs_change(const struct lysp_tpdf *typedefs1, const struct lysp_tpdf *typedefs2,
        const struct lysp_node *parent2, enum lys_diff_changed_e parent_changed, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_typedef_change_s *typedef_change;
    ly_bool *typedef2_found = NULL, found;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found typedefs */
    typedef2_found = calloc(LY_ARRAY_COUNT(typedefs2), sizeof *typedef2_found);
    LY_CHECK_ERR_GOTO(!typedef2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(typedefs1, u) {
        found = 0;
        LY_ARRAY_FOR(typedefs2, v) {
            if (typedef2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(typedefs1[u].name, typedefs2[v].name)) {
                found = 1;
                typedef2_found[v] = 1;
                break;
            }
        }

        /* add new typedef to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_typedef_change(&typedefs1[u], found ? &typedefs2[v] : NULL, parent2, diff,
                &typedef_change), cleanup);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_TYPEDEF, 1,
                    &typedef_change->changes), cleanup);

            /* NBC */
            diff->is_nbc = 1;
            continue;
        }

        /* units */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(typedefs1[u].units, typedefs2[v].units, LYS_CHANGED_TYPEDEF,
                LYS_CHANGED_UNITS, &typedef_change->changes), cleanup);

        /* default */
        LY_CHECK_GOTO(rc = schema_diff_text_bc_add(typedefs1[u].dflt.str, typedefs2[v].dflt.str, LYS_CHANGED_TYPEDEF,
                LYS_CHANGED_DEFAULT, &typedef_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(typedefs1[u].dsc, typedefs2[v].dsc, typedefs2[v].exts,
                LYS_CHANGED_TYPEDEF, &typedef_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(typedefs1[u].ref, typedefs2[v].ref, LYS_CHANGED_TYPEDEF,
                LYS_CHANGED_REFERENCE, &typedef_change->changes), cleanup);

        /* type */
        LY_CHECK_GOTO(rc = schema_diff_ptype_change(&typedefs1[u].type, &typedefs2[v].type, &typedef_change->changes),
                cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(typedefs1[u].flags, typedefs2[v].flags, LYS_CHANGED_TYPEDEF,
                &typedef_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&typedef_change->changes, diff);
    }

    LY_ARRAY_FOR(typedefs2, v) {
        if (typedef2_found[v]) {
            continue;
        }

        /* add new typedef to changes */
        LY_CHECK_RET(schema_diff_add_typedef_change(NULL, &typedefs2[v], parent2, diff, &typedef_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_TYPEDEF, 0,
                &typedef_change->changes), cleanup);
    }

cleanup:
    free(typedef2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'when' statement.
 *
 * @param[in] when1 First when.
 * @param[in] when2 Second when.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode_when_change(const struct lysp_when *when1, const struct lysp_when *when2,
        struct lys_diff_changes_s *changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;

    if (!when1 && !when2) {
        /* no changes */
        goto cleanup;
    } else if (!when2) {
        /* removed, NBC for YANG 1.0 */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NODE, LYS_CHANGED_WHEN, diff->is_yang10, changes);
    } else if (!when1) {
        /* added, detect compatibility by the extension presence */
        if (!diff->is_yang10 && schema_diff_parsed_has_bc_ext(when2->exts)) {
            return schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NODE, LYS_CHANGED_WHEN, 0, changes);
        } else {
            return schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NODE, LYS_CHANGED_WHEN, 1, changes);
        }
    }

    /* condition, detect compatibility by the extension presence */
    if (strcmp(when1->cond, when2->cond)) {
        if (!diff->is_yang10 && schema_diff_parsed_has_bc_ext(when2->exts)) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_NODE, LYS_CHANGED_WHEN, 0,
                    changes), cleanup);
        } else {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_NODE, LYS_CHANGED_WHEN, 1,
                    changes), cleanup);
        }
    }

    /* description */
    LY_CHECK_GOTO(rc = schema_diff_text_bc(when1->dsc, when2->dsc, LYS_CHANGED_WHEN, LYS_CHANGED_DESCRIPTION, changes),
            cleanup);

    /* reference */
    LY_CHECK_GOTO(rc = schema_diff_text_bc(when1->ref, when2->ref, LYS_CHANGED_WHEN, LYS_CHANGED_REFERENCE, changes),
            cleanup);

cleanup:
    return rc;
}

/**
 * @brief Check changes of a parsed 'default' array.
 *
 * @param[in] dflts1 First default array.
 * @param[in] dflts2 Second default array.
 * @param[in] parent_changed Changed parent statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_defaults_change(const struct lysp_qname *dflts1, const struct lysp_qname *dflts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    if (!dflts1 && !dflts2) {
        /* no changes */
        return LY_SUCCESS;
    } else if (!dflts2) {
        /* removed */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_DEFAULT, 1, changes);
    } else if (!dflts1) {
        /* added */
        return schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_DEFAULT, 0, changes);
    } else if (LY_ARRAY_COUNT(dflts1) != LY_ARRAY_COUNT(dflts2)) {
        /* modified */
        return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_DEFAULT, 1, changes);
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a parsed 'mandatory' statement.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Changed parent statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_mandatory_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes)
{
    ly_bool is_nbc;

    flags1 &= LYS_MAND_MASK;
    flags2 &= LYS_MAND_MASK;
    is_nbc = ((flags1 & LYS_MAND_FALSE) || (flags2 & LYS_MAND_TRUE)) ? 1 : 0;

    if (flags1 && !flags2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_MANDATORY, parent_changed, is_nbc, changes));
    } else if (!flags1 && flags2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_MANDATORY, parent_changed, is_nbc, changes));
    } else if (flags1 && flags2 && (flags1 != flags2)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_MANDATORY, parent_changed, is_nbc, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Generate a canonical name (without local prefixes) of a node.
 *
 * @param[in] name Node name.
 * @param[in] nodetype Node type.
 * @param[in] prefix Local prefix to remove.
 * @param[in,out] buf Buffer to use, may already be allocated.
 * @return Canonical name of @p node.
 */
static const char *
schema_diff_parsed_name_canon(const char *name, uint16_t nodetype, const char *prefix, char **buf)
{
    uint32_t pref_len = strlen(prefix);
    const char *ptr;
    char *buf_ptr;

    if (!(nodetype & (LYS_USES | LYS_AUGMENT))) {
        /* no prefixes */
        return name;
    }

    if (nodetype == LYS_USES) {
        /* may start with the local prefix */
        if (!strncmp(name, prefix, pref_len) && (name[pref_len] == ':')) {
            return name + pref_len + 1;
        }

        return name;
    }

    /* may include repeated local prefixes, relative or absolute path */
    if (!strstr(name, prefix)) {
        /* no local prefix usage */
        return name;
    }

    free(*buf);
    *buf = malloc(strlen(name) + 1);
    buf_ptr = *buf;

    for (ptr = name; ptr[0]; ++ptr) {
        /* make sure it is a prefix */
        if (((ptr == name) || (ptr[-1] == '/')) && !strncmp(ptr, prefix, pref_len) && (ptr[pref_len] == ':')) {
            /* skip the prefix */
            ptr += pref_len + 1;
        }

        buf_ptr[0] = ptr[0];
        ++buf_ptr;
    }

    buf_ptr[0] = '\0';
    return *buf;
}

/**
 * @brief Determine whether adding or removing a specific refine is a BC or NBC change.
 *
 * @param[in] refine Refine to use.
 * @param[in] change Refine change, either added or removed.
 * @return 1 if @p refine change is NBC.
 * @return 0 if @p refine change is BC.
 */
static ly_bool
schema_diff_parsed_refine_is_nbc(const struct lysp_refine *refine, enum lys_diff_change_e change)
{
    /* description */
    if (refine->dsc) {
        if (!schema_diff_parsed_has_bc_ext(refine->exts)) {
            return 1;
        }
    }

    /* reference */
    /* always BC */

    /* if-features */
    if (refine->iffeatures) {
        if (change == LYS_CHANGE_ADDED) {
            return 1;
        }
    }

    /* musts */
    if (refine->musts) {
        if (change == LYS_CHANGE_ADDED) {
            return 1;
        }
    }

    /* presence */
    if (refine->presence) {
        return 1;
    }

    /* default */
    if (refine->dflts) {
        /* only setting a default value for a leaf is BC, no way to check */
        return 1;
    }

    /* min, max */
    if (refine->min || refine->max) {
        /* relaxed constraint is BC, no way to check */
        if (change == LYS_CHANGE_ADDED) {
            return 1;
        }
    }

    /* config */
    if (refine->flags & LYS_CONFIG_MASK) {
        /* state -> config BC if not mandatory, no way to check */
        return 1;
    }

    /* mandatory */
    if ((refine->flags & LYS_MAND_TRUE) && (change == LYS_CHANGE_ADDED)) {
        return 1;
    } else if ((refine->flags & LYS_MAND_FALSE) && (change == LYS_CHANGE_REMOVED)) {
        return 1;
    }

    return 0;
}

/**
 * @brief Check changes of a parsed 'refine' array.
 *
 * @param[in] refines1 First refine array.
 * @param[in] refines2 Second refine array.
 * @param[in] parent2 Second refine array parent node.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_refines_change(const struct lysp_refine *refines1, const struct lysp_refine *refines2,
        const struct lysp_node *parent2, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_refine_change_s *refine_change;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *refine2_found = NULL;
    const char *nodeid1, *nodeid2;
    char *buf1 = NULL, *buf2 = NULL;

    if (refines2) {
        /* prepare array for marking found refines */
        refine2_found = calloc(LY_ARRAY_COUNT(refines2), sizeof *refine2_found);
        LY_CHECK_ERR_GOTO(!refine2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)
    }

    LY_ARRAY_FOR(refines1, u) {
        nodeid1 = schema_diff_parsed_name_canon(refines1[u].nodeid, LYS_AUGMENT, diff->old_prefix, &buf1);

        found = 0;
        LY_ARRAY_FOR(refines2, v) {
            if (refine2_found[v]) {
                continue;
            }

            nodeid2 = schema_diff_parsed_name_canon(refines2[v].nodeid, LYS_AUGMENT, diff->new_prefix, &buf2);

            /* node-id */
            if (!strcmp(nodeid1, nodeid2)) {
                /* found */
                found = 1;
                refine2_found[v] = 1;
                break;
            }
        }

        /* add new refine to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_refine_change(&refines1[u], &refines2[v], parent2, diff, &refine_change),
                cleanup);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NODE, LYS_CHANGED_REFINE,
                    schema_diff_parsed_refine_is_nbc(&refines1[u], LYS_CHANGE_REMOVED), &refine_change->changes), cleanup);

            /* check whether any of the changes were NBC */
            schema_diff_check_node_change_nbc(&refine_change->changes, diff);
            continue;
        }

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(refines1[u].dsc, refines2[v].dsc, refines2[v].exts,
                LYS_CHANGED_REFINE, &refine_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(refines1[u].ref, refines2[v].ref, LYS_CHANGED_REFINE,
                LYS_CHANGED_REFERENCE, &refine_change->changes), cleanup);

        /* if-features */
        LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(refines1[u].iffeatures, 0, refines2[v].iffeatures,
                LYS_CHANGED_REFINE, &refine_change->changes), cleanup);

        /* musts */
        LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(refines1[u].musts, refines2[v].musts, LYS_CHANGED_MUST,
                LYS_CHANGED_REFINE, &refine_change->changes), cleanup);

        /* presence */
        if (refines1[u].presence && !refines2[v].presence) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_REFINE, LYS_CHANGED_PRESENCE, 1,
                    &refine_change->changes), cleanup);
        } else if (!refines1[u].presence && refines2[v].presence) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_REFINE, LYS_CHANGED_PRESENCE, 1,
                    &refine_change->changes), cleanup);
        } else if (refines1[u].presence && refines2[v].presence && strcmp(refines1[u].presence, refines2[v].presence)) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_REFINE, LYS_CHANGED_PRESENCE, 0,
                    &refine_change->changes), cleanup);
        }

        /* defaults */
        LY_CHECK_GOTO(rc = schema_diff_parsed_defaults_change(refines1[u].dflts, refines2[v].dflts, LYS_CHANGED_REFINE,
                &refine_change->changes), cleanup);

        /* min-elements */
        LY_CHECK_GOTO(rc = schema_diff_elem_limit_change(refines1[u].min, refines1[u].flags & LYS_SET_MIN,
                refines2[v].min, refines2[v].flags & LYS_SET_MIN, LYS_CHANGED_REFINE, LYS_CHANGED_MIN_ELEM,
                &refine_change->changes), cleanup);

        /* max-elements */
        LY_CHECK_GOTO(rc = schema_diff_elem_limit_change(refines1[u].max, refines1[u].flags & LYS_SET_MAX,
                refines2[v].max, refines2[v].flags & LYS_SET_MAX, LYS_CHANGED_REFINE, LYS_CHANGED_MAX_ELEM,
                &refine_change->changes), cleanup);

        /* config */
        LY_CHECK_GOTO(rc = schema_diff_config_change(refines1[u].flags, refines2[v].flags, LYS_CHANGED_REFINE,
                &refine_change->changes), cleanup);

        /* mandatory */
        LY_CHECK_GOTO(rc = schema_diff_parsed_mandatory_change(refines1[u].flags, refines2[v].flags, LYS_CHANGED_REFINE,
                &refine_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&refine_change->changes, diff);
    }

    LY_ARRAY_FOR(refines2, v) {
        if (refine2_found[v]) {
            continue;
        }

        /* add new refine to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_refine_change(&refines1[u], &refines2[v], parent2, diff, &refine_change),
                cleanup);

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NODE, LYS_CHANGED_REFINE,
                schema_diff_parsed_refine_is_nbc(&refines2[v], LYS_CHANGE_ADDED), &refine_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&refine_change->changes, diff);
    }

cleanup:
    free(refine2_found);
    free(buf1);
    free(buf2);
    return rc;
}

/**
 * @brief Check changes of a parsed node pair.
 *
 * @param[in] node1 First node.
 * @param[in] node2 Second node.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode_change(const struct lysp_node *node1, const struct lysp_node *node2,
        struct lys_diff_changes_s *changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysp_node_choice *choic;
    const struct lysp_node_case *cas;
    const struct lysp_node_uses *uses;
    const struct lysp_node_action_inout *inout;
    const struct lysp_node_augment *aug;
    const struct lysp_when *when1 = NULL, *when2 = NULL;
    const struct lysp_restr *musts1 = NULL, *musts2 = NULL;
    const char *dflt1 = NULL, *dflt2 = NULL;
    const struct lysp_refine *refines1 = NULL, *refines2 = NULL;

    if (!node1) {
        /* node added change */
        assert(node2);
        return schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_NODE,
                (node2->flags & LYS_MAND_TRUE) ? 1 : 0, changes);
    } else if (!node2) {
        /* node removed change */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_NODE,
                (node1->flags & LYS_STATUS_OBSLT) ? 0 : 1, changes);
    }

    /* description */
    LY_CHECK_GOTO(rc = schema_diff_pnode_description(node1->dsc, node2->dsc, node2->exts, LYS_CHANGED_NODE, changes),
            cleanup);

    /* reference */
    LY_CHECK_GOTO(rc = schema_diff_text_bc(node1->ref, node2->ref, LYS_CHANGED_NODE, LYS_CHANGED_REFERENCE, changes),
            cleanup);

    /* if-features */
    LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(node1->iffeatures, node1->flags, node2->iffeatures,
            LYS_CHANGED_NODE, changes), cleanup);

    /* status */
    LY_CHECK_GOTO(rc = schema_diff_status_change(node1->flags, node2->flags, LYS_CHANGED_NODE, changes), cleanup);

    switch (node1->nodetype) {
    case LYS_CHOICE:
        /* when, dflt */
        choic = (struct lysp_node_choice *)node1;
        when1 = choic->when;
        dflt1 = choic->dflt.str;

        choic = (struct lysp_node_choice *)node2;
        when2 = choic->when;
        dflt2 = choic->dflt.str;
        break;
    case LYS_CASE:
        /* when */
        cas = (struct lysp_node_case *)node1;
        when1 = cas->when;

        cas = (struct lysp_node_case *)node2;
        when2 = cas->when;
        break;
    case LYS_USES:
        /* refines, when */
        uses = (struct lysp_node_uses *)node1;
        refines1 = uses->refines;
        when1 = uses->when;

        uses = (struct lysp_node_uses *)node2;
        refines2 = uses->refines;
        when2 = uses->when;
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        /* musts */
        inout = (struct lysp_node_action_inout *)node1;
        musts1 = inout->musts;

        inout = (struct lysp_node_action_inout *)node2;
        musts2 = inout->musts;
        break;
    case LYS_GROUPING:
        /* no special substatements */
        break;
    case LYS_AUGMENT:
        /* when */
        aug = (struct lysp_node_augment *)node1;
        when1 = aug->when;

        aug = (struct lysp_node_augment *)node2;
        when2 = aug->when;
        break;
    default:
        LOGINT(NULL);
        rc = LY_EINT;
        goto cleanup;
    }

    /* when */
    LY_CHECK_GOTO(rc = schema_diff_pnode_when_change(when1, when2, changes, diff), cleanup);

    /* musts */
    LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(musts1, musts2, LYS_CHANGED_MUST, LYS_CHANGED_NODE, changes),
            cleanup);

    /* default */
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(dflt1, dflt2, LYS_CHANGED_NODE, LYS_CHANGED_DEFAULT, changes), cleanup);

    /* refines, separate changes */
    LY_CHECK_GOTO(rc = schema_diff_parsed_refines_change(refines1, refines2, node2, diff), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Check changes of a parsed node pair, recursively.
 *
 * @param[in] node1 First node.
 * @param[in] node2 Second node.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode_change_r(const struct lysp_node *node1, const struct lysp_node *node2,
        struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysp_node_action *act1, *act2;
    const struct lysp_node_uses *uses1, *uses2;
    struct lys_diff_pnode_change_s *pnode_change;
    uint16_t nodetype;

    assert(node1 || node2);

    nodetype = node1 ? node1->nodetype : node2->nodetype;

    /* TODO nodes are equal but their names do not have to match exactly, prefixes may differ (uses, augment), report it? */

    /* typedefs */
    LY_CHECK_GOTO(rc = schema_diff_typedefs_change(lysp_node_typedefs(node1), lysp_node_typedefs(node2),
            node2, LYS_CHANGED_NODE, diff), cleanup);

    if (LYS_DIFF_NODE_PARSED(nodetype)) {
        /* add new node to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_pnode_change(node1, node2, diff, &pnode_change), cleanup);

        /* node changes (removed, modified) */
        LY_CHECK_GOTO(rc = schema_diff_pnode_change(node1, node2, &pnode_change->changes, diff), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&pnode_change->changes, diff);
    }

    /* recursive changes */
    if (nodetype & (LYS_RPC | LYS_ACTION)) {
        act1 = (struct lysp_node_action *)node1;
        act2 = (struct lysp_node_action *)node2;

        LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r(act1 ? &act1->input.node : NULL, act2 ? &act2->input.node : NULL,
                diff), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r(act1 ? &act1->output.node : NULL, act2 ? &act2->output.node : NULL,
                diff), cleanup);
    } else if (nodetype == LYS_USES) {
        /* do not report descendant changes in removed/added uses */
        if (node1 && node2) {
            uses1 = (struct lysp_node_uses *)node1;
            uses2 = (struct lysp_node_uses *)node2;

            LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)uses1->augments,
                    (struct lysp_node *)uses2->augments, diff), cleanup);
        }
    } else {
        LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r(lysp_node_child(node1), lysp_node_child(node2), diff), cleanup);
    }

    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)lysp_node_actions(node1),
            (struct lysp_node *)lysp_node_actions(node2), diff), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)lysp_node_notifs(node1),
            (struct lysp_node *)lysp_node_notifs(node2), diff), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)lysp_node_groupings(node1),
            (struct lysp_node *)lysp_node_groupings(node2), diff), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Check changes of parsed node siblings, recursively.
 *
 * @param[in] node1 First node siblings.
 * @param[in] node2 Second node siblings.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnodes_change_r(const struct lysp_node *node1, const struct lysp_node *node2, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysp_node **node2_array = NULL, *iter;
    uint32_t node2_count = 0, i;
    const char *name1, *name2;
    char *buf1 = NULL, *buf2 = NULL;

    /* prepare array for node2 siblings */
    LY_LIST_FOR(node2, iter) {
        node2_array = ly_realloc(node2_array, (node2_count + 1) * sizeof *node2_array);
        LY_CHECK_ERR_GOTO(!node2_array, LOGMEM(NULL); rc = LY_SUCCESS, cleanup);
        node2_array[node2_count] = iter;
        ++node2_count;
    }

    LY_LIST_FOR(node1, node1) {
        /* get node1 canonical name */
        name1 = schema_diff_parsed_name_canon(node1->name, node1->nodetype, diff->old_prefix, &buf1);

        node2 = NULL;
        for (i = 0; i < node2_count; ++i) {
            if (!node2_array[i]) {
                continue;
            }

            /* get node2 canonical name */
            name2 = schema_diff_parsed_name_canon(node2_array[i]->name, node2_array[i]->nodetype, diff->new_prefix, &buf2);

            if (!strcmp(name1, name2)) {
                /* matching nodes */
                node2 = node2_array[i];
                break;
            }
        }

        /* process nodes */
        LY_CHECK_GOTO(rc = schema_diff_pnode_change_r(node1, node2, diff), cleanup);

        if (node2) {
            /* match found */
            node2_array[i] = NULL;
        }
    }

    for (i = 0; i < node2_count; ++i) {
        if (!node2_array[i]) {
            continue;
        }

        /* process nodes */
        LY_CHECK_GOTO(rc = schema_diff_pnode_change_r(NULL, node2_array[i], diff), cleanup);
    }

cleanup:
    free(node2_array);
    free(buf1);
    free(buf2);
    return rc;
}

/**
 * @brief Check changes of a parsed 'import' array.
 *
 * @param[in] imps1 First import array.
 * @param[in] imps2 Second import array.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_imports_change(const struct lysp_import *imps1, const struct lysp_import *imps2, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_import_change_s *import_change;
    ly_bool *imp2_found = NULL, found;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found identities */
    imp2_found = calloc(LY_ARRAY_COUNT(imps2), sizeof *imp2_found);
    LY_CHECK_ERR_GOTO(!imp2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(imps1, u) {
        found = 0;
        LY_ARRAY_FOR(imps2, v) {
            if (imp2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(imps1[u].name, imps2[v].name)) {
                found = 1;
                imp2_found[v] = 1;
                break;
            }
        }

        /* add new import to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_import_change(&imps1[u], found ? &imps2[v] : NULL, diff,
                &import_change), cleanup);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_IMPORT, 0,
                    &import_change->changes), cleanup);
            continue;
        }

        /* revision-date */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(imps1[u].rev[0] ? imps1[u].rev : NULL,
                imps2[v].rev[0] ? imps2[v].rev : NULL, LYS_CHANGED_IMPORT, LYS_CHANGED_REVISION_DATE,
                &import_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(imps1[u].dsc, imps2[v].dsc, imps2[v].exts, LYS_CHANGED_IMPORT,
                &import_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(imps1[u].ref, imps2[v].ref, LYS_CHANGED_IMPORT, LYS_CHANGED_REFERENCE,
                &import_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&import_change->changes, diff);
    }

    LY_ARRAY_FOR(imps2, v) {
        if (imp2_found[v]) {
            continue;
        }

        /* add new import to changes */
        LY_CHECK_RET(schema_diff_add_import_change(NULL, &imps2[v], diff, &import_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_IMPORT, 0,
                &import_change->changes), cleanup);
    }

cleanup:
    free(imp2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'include' array.
 *
 * @param[in] incs1 First include array.
 * @param[in] incs2 Second include array.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_includes_change(const struct lysp_include *incs1, const struct lysp_include *incs2, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_include_change_s *include_change;
    ly_bool *inc2_found = NULL, found;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found identities */
    inc2_found = calloc(LY_ARRAY_COUNT(incs2), sizeof *inc2_found);
    LY_CHECK_ERR_GOTO(!inc2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(incs1, u) {
        if (incs1[u].injected) {
            continue;
        }

        found = 0;
        LY_ARRAY_FOR(incs2, v) {
            if (incs2[v].injected || inc2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(incs1[u].name, incs2[v].name)) {
                found = 1;
                inc2_found[v] = 1;
                break;
            }
        }

        /* add new include to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_include_change(&incs1[u], found ? &incs2[v] : NULL, diff, &include_change),
                cleanup);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_INCLUDE, 0,
                    &include_change->changes), cleanup);
            continue;
        }

        /* revision-date */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(incs1[u].rev[0] ? incs1[u].rev : NULL,
                incs2[v].rev[0] ? incs2[v].rev : NULL, LYS_CHANGED_INCLUDE, LYS_CHANGED_REVISION_DATE,
                &include_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(incs1[u].dsc, incs2[v].dsc, incs2[v].exts,
                LYS_CHANGED_INCLUDE, &include_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(incs1[u].ref, incs2[v].ref, LYS_CHANGED_INCLUDE,
                LYS_CHANGED_REFERENCE, &include_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&include_change->changes, diff);
    }

    LY_ARRAY_FOR(incs2, v) {
        if (incs2[v].injected || inc2_found[v]) {
            continue;
        }

        /* add new include to changes */
        LY_CHECK_RET(schema_diff_add_include_change(NULL, &incs2[v], diff, &include_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_INCLUDE, 0,
                &include_change->changes), cleanup);
    }

cleanup:
    free(inc2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'extension' array.
 *
 * @param[in] extensions1 First extension array.
 * @param[in] extensions2 Second extension array.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_extensions_change(const struct lysp_ext *extensions1, const struct lysp_ext *extensions2,
        struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_extension_change_s *extension_change;
    ly_bool *extension2_found = NULL, found;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found identities */
    extension2_found = calloc(LY_ARRAY_COUNT(extensions2), sizeof *extension2_found);
    LY_CHECK_ERR_GOTO(!extension2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(extensions1, u) {
        found = 0;
        LY_ARRAY_FOR(extensions2, v) {
            if (extension2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(extensions1[u].name, extensions2[v].name)) {
                found = 1;
                extension2_found[v] = 1;
                break;
            }
        }

        /* add new extension to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_extension_change(&extensions1[u], found ? &extensions2[v] : NULL, diff,
                &extension_change), cleanup);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_EXTENSION, 1,
                    &extension_change->changes), cleanup);

            /* NBC */
            diff->is_nbc = 1;
            continue;
        }

        /* argument */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(extensions1[u].argname, extensions2[v].argname, LYS_CHANGED_EXTENSION,
                LYS_CHANGED_UNITS, &extension_change->changes), cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(extensions1[u].flags, extensions2[v].flags, LYS_CHANGED_EXTENSION,
                &extension_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(extensions1[u].dsc, extensions2[v].dsc, extensions2[v].exts,
                LYS_CHANGED_EXTENSION, &extension_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(extensions1[u].ref, extensions2[v].ref, LYS_CHANGED_EXTENSION,
                LYS_CHANGED_REFERENCE, &extension_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&extension_change->changes, diff);
    }

    LY_ARRAY_FOR(extensions2, v) {
        if (extension2_found[v]) {
            continue;
        }

        /* add new extension to changes */
        LY_CHECK_RET(schema_diff_add_extension_change(NULL, &extensions2[v], diff, &extension_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_EXTENSION, 0,
                &extension_change->changes), cleanup);
    }

cleanup:
    free(extension2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'feature' array.
 *
 * @param[in] features1 First feature array.
 * @param[in] features2 Second feature array.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_features_change(const struct lysp_feature *features1, const struct lysp_feature *features2,
        struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_feat_change_s *feat_change;
    ly_bool *feature2_found = NULL, found;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found identities */
    feature2_found = calloc(LY_ARRAY_COUNT(features2), sizeof *feature2_found);
    LY_CHECK_ERR_GOTO(!feature2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(features1, u) {
        found = 0;
        LY_ARRAY_FOR(features2, v) {
            if (feature2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(features1[u].name, features2[v].name)) {
                found = 1;
                feature2_found[v] = 1;
                break;
            }
        }

        /* add new feature to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_feat_change(&features1[u], found ? &features2[v] : NULL, diff, &feat_change),
                cleanup);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_FEATURE, 1,
                    &feat_change->changes), cleanup);

            /* NBC */
            diff->is_nbc = 1;
            continue;
        }

        /* if-features */
        LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(features1[u].iffeatures, 0, features2[v].iffeatures,
                LYS_CHANGED_FEATURE, &feat_change->changes), cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(features1[u].flags, features2[v].flags, LYS_CHANGED_FEATURE,
                &feat_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(features1[u].dsc, features2[v].dsc, features2[v].exts,
                LYS_CHANGED_FEATURE, &feat_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(features1[u].ref, features2[v].ref, LYS_CHANGED_FEATURE,
                LYS_CHANGED_REFERENCE, &feat_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&feat_change->changes, diff);
    }

    LY_ARRAY_FOR(features2, v) {
        if (feature2_found[v]) {
            continue;
        }

        /* add new feature to changes */
        LY_CHECK_RET(schema_diff_add_feat_change(NULL, &features2[v], diff, &feat_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_FEATURE, 0,
                &feat_change->changes), cleanup);
    }

cleanup:
    free(feature2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'unique' array.
 *
 * @param[in] uniqs1 First unique array.
 * @param[in] uniqs2 Second unique array.
 * @param[in] parent_changed Changed parent statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_uniques_change(const struct lysp_qname *uniqs1, const struct lysp_qname *uniqs2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    if (!uniqs1 && !uniqs2) {
        /* no changes */
        return LY_SUCCESS;
    } else if (!uniqs2) {
        /* removed */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_UNIQUE, 1, changes);
    } else if (!uniqs1) {
        /* added */
        return schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_UNIQUE, 1, changes);
    } else if (LY_ARRAY_COUNT(uniqs1) != LY_ARRAY_COUNT(uniqs2)) {
        /* modified */
        return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_UNIQUE, 1, changes);
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a deviate pair.
 *
 * @param[in] dev1 First deviate.
 * @param[in] dev2 Second deviate.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_deviate_change(const struct lysp_deviate *dev1, const struct lysp_deviate *dev2,
        struct lys_diff_changes_s *changes)
{
    const struct lysp_deviate_add *add1, *add2;
    const struct lysp_deviate_rpl *rpl1, *rpl2;

    assert(dev1->mod == dev2->mod);

    add1 = (const struct lysp_deviate_add *)dev1;
    add2 = (const struct lysp_deviate_add *)dev2;
    rpl1 = (const struct lysp_deviate_rpl *)dev1;
    rpl2 = (const struct lysp_deviate_rpl *)dev2;

    /* units */
    if (dev1->mod == LYS_DEV_ADD) {
        LY_CHECK_RET(schema_diff_text_bc_add(add1->units, add2->units, LYS_CHANGED_DEVIATE, LYS_CHANGED_UNITS,
                changes));
    } else if (dev1->mod == LYS_DEV_DELETE) {
        LY_CHECK_RET(schema_diff_text_bc_add(add2->units, add1->units, LYS_CHANGED_DEVIATE, LYS_CHANGED_UNITS,
                changes));
    }

    /* musts */
    if (dev1->mod == LYS_DEV_ADD) {
        LY_CHECK_RET(schema_diff_parsed_restrs_change(add1->musts, add2->musts, LYS_CHANGED_MUST, LYS_CHANGED_DEVIATE,
                changes));
    } else if (dev1->mod == LYS_DEV_DELETE) {
        LY_CHECK_RET(schema_diff_parsed_restrs_change(add2->musts, add1->musts, LYS_CHANGED_MUST, LYS_CHANGED_DEVIATE,
                changes));
    }

    /* unique */
    if ((dev1->mod == LYS_DEV_ADD) || (dev1->mod == LYS_DEV_DELETE)) {
        /* any change is NBC */
        LY_CHECK_RET(schema_diff_parsed_uniques_change(add1->uniques, add2->uniques, LYS_CHANGED_DEVIATE, changes));
    }

    /* default */
    if (dev1->mod == LYS_DEV_ADD) {
        LY_CHECK_RET(schema_diff_parsed_defaults_change(add1->dflts, add2->dflts, LYS_CHANGED_DEVIATE, changes));
    } else if (dev1->mod == LYS_DEV_DELETE) {
        LY_CHECK_RET(schema_diff_parsed_defaults_change(add2->dflts, add1->dflts, LYS_CHANGED_DEVIATE, changes));
    } else if (dev1->mod == LYS_DEV_REPLACE) {
        LY_CHECK_RET(schema_diff_text_bc_add(rpl1->dflt.str, rpl2->dflt.str, LYS_CHANGED_DEVIATE, LYS_CHANGED_DEFAULT,
                changes));
    }

    /* config */
    if ((dev1->mod == LYS_DEV_ADD) || (dev1->mod == LYS_DEV_REPLACE)) {
        LY_CHECK_RET(schema_diff_config_change(add1->flags, add2->flags, LYS_CHANGED_DEVIATE, changes));
    }

    /* mandatory */
    if ((dev1->mod == LYS_DEV_ADD) || (dev1->mod == LYS_DEV_REPLACE)) {
        LY_CHECK_RET(schema_diff_parsed_mandatory_change(add1->flags, add2->flags, LYS_CHANGED_DEVIATE, changes));
    }

    /* min-elements */
    if ((dev1->mod == LYS_DEV_ADD) || (dev1->mod == LYS_DEV_REPLACE)) {
        LY_CHECK_RET(schema_diff_elem_limit_change(add1->min, add1->flags & LYS_SET_MIN, add2->min,
                add2->flags & LYS_SET_MIN, LYS_CHANGED_DEVIATE, LYS_CHANGED_MIN_ELEM, changes));
    }

    /* max-elements */
    if ((dev1->mod == LYS_DEV_ADD) || (dev1->mod == LYS_DEV_REPLACE)) {
        LY_CHECK_RET(schema_diff_elem_limit_change(add1->max, add1->flags & LYS_SET_MAX, add2->max,
                add2->flags & LYS_SET_MAX, LYS_CHANGED_DEVIATE, LYS_CHANGED_MAX_ELEM, changes));
    }

    /* type */
    if (dev1->mod == LYS_DEV_REPLACE) {
        LY_CHECK_RET(schema_diff_ptype_change(rpl1->type, rpl2->type, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a parsed 'deviate' array.
 *
 * @param[in] deviates1 First deviate array.
 * @param[in] deviates2 Second deviate array.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_deviates_change(const struct lysp_deviate *deviates1, const struct lysp_deviate *deviates2,
        struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE v;
    ly_bool found, *deviate2_found = NULL;
    const struct lysp_deviate *iter1, *iter2;

    if (deviates2) {
        /* prepare array for marking found deviates */
        deviate2_found = calloc(LY_ARRAY_COUNT(deviates2), sizeof *deviate2_found);
        LY_CHECK_ERR_GOTO(!deviate2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)
    }

    LY_LIST_FOR(deviates1, iter1) {
        found = 0;
        for (iter2 = deviates2, v = 0; iter2; iter2 = iter2->next, ++v) {
            if (deviate2_found[v]) {
                continue;
            }

            /* argument (type) */
            if (iter1->mod == iter2->mod) {
                found = 1;
                deviate2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_DEVIATION, LYS_CHANGED_DEVIATE, 1,
                    changes), cleanup);
            continue;
        }

        /* deviate */
        LY_CHECK_GOTO(rc = schema_diff_deviate_change(iter1, iter2, changes), cleanup);
    }

    for (iter2 = deviates2, v = 0; iter2; iter2 = iter2->next, ++v) {
        if (deviate2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_DEVIATION, LYS_CHANGED_DEVIATE, 1,
                changes), cleanup);
    }

cleanup:
    free(deviate2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'deviation' array.
 *
 * @param[in] deviations1 First deviation array.
 * @param[in] deviations2 Second deviation array.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_deviations_change(const struct lysp_deviation *deviations1, const struct lysp_deviation *deviations2,
        struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_dev_change_s *dev_change;
    ly_bool *deviation2_found = NULL, found;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found identities */
    deviation2_found = calloc(LY_ARRAY_COUNT(deviations2), sizeof *deviation2_found);
    LY_CHECK_ERR_GOTO(!deviation2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(deviations1, u) {
        found = 0;
        LY_ARRAY_FOR(deviations2, v) {
            if (deviation2_found[v]) {
                continue;
            }

            /* nodeid */
            if (!strcmp(deviations1[u].nodeid, deviations2[v].nodeid)) {
                found = 1;
                deviation2_found[v] = 1;
                break;
            }
        }

        /* add new deviation to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_dev_change(&deviations1[u], found ? &deviations2[v] : NULL, diff, &dev_change),
                cleanup);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_DEVIATION, 1,
                    &dev_change->changes), cleanup);

            /* NBC */
            diff->is_nbc = 1;
            continue;
        }

        /* deviate */
        LY_CHECK_GOTO(rc = schema_diff_deviates_change(deviations1[u].deviates, deviations2[v].deviates,
                &dev_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_description(deviations1[u].dsc, deviations2[v].dsc, deviations2[v].exts,
                LYS_CHANGED_DEVIATION, &dev_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(deviations1[u].ref, deviations2[v].ref, LYS_CHANGED_DEVIATION,
                LYS_CHANGED_REFERENCE, &dev_change->changes), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&dev_change->changes, diff);
    }

    LY_ARRAY_FOR(deviations2, v) {
        if (deviation2_found[v]) {
            continue;
        }

        /* add new deviation to changes */
        LY_CHECK_RET(schema_diff_add_dev_change(NULL, &deviations2[v], diff, &dev_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_DEVIATION, 1,
                &dev_change->changes), cleanup);

        /* NBC */
        diff->is_nbc = 1;
    }

cleanup:
    free(deviation2_found);
    return rc;
}

/**
 * @brief Check changes of parsed modules.
 *
 * @param[in] mod1 First parsed module.
 * @param[in] mod2 Second parsed module.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pmodule_change(const struct lysp_module *mod1, const struct lysp_module *mod2, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;

    if (!mod1 || !mod2) {
        LOGERR(NULL, LY_EINVAL, "Cannot generate parsed schema changes without parsed modules.");
        return LY_EINVAL;
    }

    /*
     * module parsed substatements
     */

    /* prefix */
    if (strcmp(mod1->mod->prefix, mod2->mod->prefix)) {
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_NONE, LYS_CHANGED_PREFIX, 0,
                &diff->module_changes), cleanup);
    }

    /* imports */
    LY_CHECK_GOTO(rc = schema_diff_imports_change(mod1->imports, mod2->imports, diff), cleanup);

    /* includes */
    LY_CHECK_GOTO(rc = schema_diff_includes_change(mod1->includes, mod2->includes, diff), cleanup);

    /* extensions */
    LY_CHECK_GOTO(rc = schema_diff_extensions_change(mod1->extensions, mod2->extensions, diff), cleanup);

    /* features */
    LY_CHECK_GOTO(rc = schema_diff_features_change(mod1->features, mod2->features, diff), cleanup);

    /* deviations */
    LY_CHECK_GOTO(rc = schema_diff_deviations_change(mod1->deviations, mod2->deviations, diff), cleanup);

    /* typedefs */
    LY_CHECK_GOTO(rc = schema_diff_typedefs_change(mod1->typedefs, mod2->typedefs, NULL, LYS_CHANGED_NONE, diff), cleanup);

    /*
     * node parsed substatements
     */
    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r(mod1->data, mod2->data, diff), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)mod1->rpcs, (struct lysp_node *)mod2->rpcs, diff),
            cleanup);
    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)mod1->notifs, (struct lysp_node *)mod2->notifs,
            diff), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)mod1->groupings,
            (struct lysp_node *)mod2->groupings, diff), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r((struct lysp_node *)mod1->augments,
            (struct lysp_node *)mod2->augments, diff), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Check changes of 'identity' 'base' arrays.
 *
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] ident_change Ident change structure to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_identity_bases_change(enum lys_diff_changed_e parent_changed,
        struct lys_diff_ident_change_s *ident_change)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    int found, added = 0, removed = 0;

    /* compare old ident bases to new ones */
    LY_ARRAY_FOR(ident_change->p_ident_old->bases, u) {
        found = 0;
        LY_ARRAY_FOR(ident_change->p_ident_new->bases, v) {
            if (!strcmp(ident_change->p_ident_old->bases[u], ident_change->p_ident_new->bases[v])) {
                found = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            ++removed;
        }
    }

    if (removed) {
        /* bases NBC modified */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_BASE, 1,
                &ident_change->changes), cleanup);
    }

    /* compare new ident bases to old ones */
    LY_ARRAY_FOR(ident_change->p_ident_new->bases, v) {
        found = 0;
        LY_ARRAY_FOR(ident_change->p_ident_old->bases, u) {
            if (!strcmp(ident_change->p_ident_old->bases[u], ident_change->p_ident_new->bases[v])) {
                found = 1;
                break;
            }
        }

        if (!found) {
            /* added */
            ++added;
        }
    }

    if (added) {
        /* bases BC modified */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_BASE, 0,
                &ident_change->changes), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Check changes of 'identity' arrays.
 *
 * @param[in] idents1 First identity array.
 * @param[in] idents2 Second identity array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_identities_change(const struct lysc_ident *idents1, const struct lysc_ident *idents2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_ident_change_s *ident_change;
    ly_bool *ident2_found = NULL, found;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found identities */
    ident2_found = calloc(LY_ARRAY_COUNT(idents2), sizeof *ident2_found);
    LY_CHECK_ERR_GOTO(!ident2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(idents1, u) {
        found = 0;
        LY_ARRAY_FOR(idents2, v) {
            if (ident2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(idents1[u].name, idents2[v].name)) {
                found = 1;
                ident2_found[v] = 1;
                break;
            }
        }

        /* add new identity to changes */
        LY_CHECK_RET(schema_diff_add_ident_change(&idents1[u], found ? &idents2[v] : NULL, diff, &ident_change));

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_IDENT, 1,
                    &ident_change->changes), cleanup);

            /* NBC */
            diff->is_nbc = 1;
            continue;
        }

        if (diff->with_parsed) {
            /* if-features */
            LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(ident_change->p_ident_old->iffeatures, 0,
                    ident_change->p_ident_new->iffeatures, LYS_CHANGED_IDENT, &ident_change->changes), cleanup);

            /* bases */
            LY_CHECK_GOTO(rc = schema_diff_module_identity_bases_change(parent_changed, ident_change), cleanup);
        }

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(idents1[u].exts, idents2[v].exts, LYS_CHANGED_IDENT,
                &ident_change->ext_changes, diff), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&ident_change->changes, diff);
    }

    LY_ARRAY_FOR(idents2, v) {
        if (ident2_found[v]) {
            continue;
        }

        /* add new identity to changes */
        LY_CHECK_RET(schema_diff_add_ident_change(NULL, &idents2[v], diff, &ident_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_IDENT, 0,
                &ident_change->changes), cleanup);
    }

cleanup:
    free(ident2_found);
    return rc;
}

/**
 * @brief Check changes of direct non-data-definition substatements of a 'module'.
 *
 * @param[in] mod1 First module.
 * @param[in] mod2 Second module.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_change(const struct lys_module *mod1, const struct lys_module *mod2, struct lys_diff_s *diff)
{
    /* organization */
    LY_CHECK_RET(schema_diff_text_bc(mod1->org, mod2->org, LYS_CHANGED_NONE, LYS_CHANGED_ORGANIZATION,
            &diff->module_changes));

    /* contact */
    LY_CHECK_RET(schema_diff_text_bc(mod1->contact, mod2->contact, LYS_CHANGED_NONE, LYS_CHANGED_CONTACT,
            &diff->module_changes));

    /* description */
    LY_CHECK_RET(schema_diff_text_bc(mod1->dsc, mod2->dsc, LYS_CHANGED_NONE, LYS_CHANGED_DESCRIPTION,
            &diff->module_changes));

    /* reference */
    LY_CHECK_RET(schema_diff_text_bc(mod1->ref, mod2->ref, LYS_CHANGED_NONE, LYS_CHANGED_REFERENCE,
            &diff->module_changes));

    /* check whether any of the changes were NBC */
    schema_diff_check_node_change_nbc(&diff->module_changes, diff);

    /* identity */
    LY_CHECK_RET(schema_diff_module_identities_change(mod1->identities, mod2->identities, LYS_CHANGED_NONE, diff));

    /* ext-instance */
    LY_CHECK_RET(schema_diff_ext_insts_change(mod1->compiled->exts, mod2->compiled->exts, LYS_CHANGED_NONE,
            &diff->mod_ext_changes, diff));

    return LY_SUCCESS;
}

/**
 * @brief Check changes of 'must' arrays.
 *
 * @param[in] musts1 First must array.
 * @param[in] musts2 Second must array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_musts_change(const struct lysc_must *musts1, const struct lysc_must *musts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *must2_found = NULL;

    /* prepare array for marking found musts */
    must2_found = calloc(LY_ARRAY_COUNT(musts2), sizeof *must2_found);
    LY_CHECK_ERR_GOTO(!must2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)

    LY_ARRAY_FOR(musts1, u) {
        found = 0;
        LY_ARRAY_FOR(musts2, v) {
            if (must2_found[v]) {
                continue;
            }

            /* condition */
            if (!strcmp(lyxp_get_expr(musts1[u].cond), lyxp_get_expr(musts2[v].cond))) {
                /* found */
                found = 1;
                must2_found[v] = 1;
                break;
            }
        }
        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_MUST, 0,
                    changes), cleanup);
            continue;
        }

        /* description, reference, error-message, error-app-tag */
        LY_CHECK_GOTO(rc = schema_diff_node_description(musts1[u].dsc, musts2[v].dsc, musts2[v].exts, LYS_CHANGED_MUST,
                changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(musts1[u].ref, musts2[v].ref, LYS_CHANGED_MUST, LYS_CHANGED_REFERENCE,
                changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(musts1[u].emsg, musts2[v].emsg, LYS_CHANGED_MUST, LYS_CHANGED_ERR_MSG,
                changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(musts1[u].eapptag, musts2[v].eapptag, LYS_CHANGED_MUST,
                LYS_CHANGED_ERR_APP_TAG, changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(musts1[u].exts, musts2[v].exts, LYS_CHANGED_MUST,
                ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(musts2, v) {
        if (must2_found[v]) {
            continue;
        }

        /* added, detect compatibility by the extension presence */
        if (schema_diff_has_bc_ext(musts2[v].exts)) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_MUST, 0,
                    changes), cleanup);
        } else {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_MUST, 1,
                    changes), cleanup);
        }
    }

cleanup:
    free(must2_found);
    return rc;
}

/**
 * @brief Check changes of a 'presence'.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_presence_change(uint16_t flags1, uint16_t flags2, struct lys_diff_changes_s *changes)
{
    if ((flags1 & LYS_PRESENCE) && !(flags2 & LYS_PRESENCE)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_PRESENCE, 1, changes));
    } else if (!(flags1 & LYS_PRESENCE) && (flags2 & LYS_PRESENCE)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_PRESENCE, 1, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a 'mandatory'.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_mandatory_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes)
{
    if ((flags1 & LYS_MAND_TRUE) && !(flags2 & LYS_MAND_TRUE)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_MANDATORY, 0, changes));
    } else if (!(flags1 & LYS_MAND_TRUE) && (flags2 & LYS_MAND_TRUE)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_MANDATORY, 1, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a 'odrered-by'.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_ordby_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes)
{
    if ((flags1 & LYS_ORDBY_MASK) != (flags2 & LYS_ORDBY_MASK)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_ORDERED_BY, 1, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of leaf-list 'default' arrays.
 *
 * @param[in] llist1 First leaf-list.
 * @param[in] llist2 Second leaf-list.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_defaults_change(const struct lysc_node_leaflist *llist1, const struct lysc_node_leaflist *llist2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *dflt2_found = NULL;

    /* prepare array for marking found defaults */
    dflt2_found = calloc(LY_ARRAY_COUNT(llist2->dflts), sizeof *dflt2_found);
    LY_CHECK_ERR_GOTO(!dflt2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(llist1->dflts, u) {
        found = 0;
        LY_ARRAY_FOR(llist2->dflts, v) {
            if (dflt2_found[v]) {
                continue;
            }

            /* value */
            if (!strcmp(llist1->dflts[u].str, llist2->dflts[v].str)) {
                found = 1;
                dflt2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_DEFAULT, 1,
                    changes), cleanup);
            continue;
        }
    }

    LY_ARRAY_FOR(llist2->dflts, v) {
        if (dflt2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_DEFAULT, 1,
                changes), cleanup);
    }

cleanup:
    free(dflt2_found);
    return rc;
}

/**
 * @brief Check changes of 'when' arrays.
 *
 * @param[in] whens1 First when array.
 * @param[in] whens2 Second when array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_whens_change(struct lysc_when **whens1, struct lysc_when **whens2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes, struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *when2_found = NULL;

    /* prepare array for marking found whens */
    when2_found = calloc(LY_ARRAY_COUNT(whens2), sizeof *when2_found);
    LY_CHECK_ERR_GOTO(!when2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(whens1, u) {
        found = 0;
        LY_ARRAY_FOR(whens2, v) {
            if (when2_found[v]) {
                continue;
            }

            /* condition */
            if (!strcmp(lyxp_get_expr(whens1[u]->cond), lyxp_get_expr(whens2[v]->cond))) {
                /* found */
                found = 1;
                when2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed, NBC for YANG 1.0 */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_WHEN,
                    diff->is_yang10, changes), cleanup);
            continue;
        }

        /* description, reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(whens1[u]->dsc, whens2[v]->dsc, LYS_CHANGED_WHEN,
                LYS_CHANGED_DESCRIPTION, changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(whens1[u]->ref, whens2[v]->ref, LYS_CHANGED_WHEN,
                LYS_CHANGED_REFERENCE, changes), cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(whens1[u]->flags, whens2[v]->flags, LYS_CHANGED_WHEN, changes),
                cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(whens1[u]->exts, whens2[v]->exts, LYS_CHANGED_WHEN,
                ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(whens2, v) {
        if (when2_found[v]) {
            continue;
        }

        /* added, detect compatibility by the extension presence */
        if (!diff->is_yang10 && schema_diff_has_bc_ext(whens2[v]->exts)) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_WHEN, 0,
                    changes), cleanup);
        } else {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_WHEN, 1,
                    changes), cleanup);
        }
    }

cleanup:
    free(when2_found);
    return rc;
}

/**
 * @brief Check changes of 'unique' arrays.
 *
 * @param[in] uniques1 First unique array.
 * @param[in] uniques2 Second unique array.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_uniques_change(struct lysc_node_leaf ***uniques1, struct lysc_node_leaf ***uniques2,
        struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE i, u, v;
    ly_bool found, *unique2_found = NULL;
    const struct lysc_node_leaf *term1, *term2;

    LY_ARRAY_FOR(uniques1, i) {
        if (LY_ARRAY_COUNT(uniques2) == i) {
            break;
        }

        /* prepare array for marking found uniques */
        unique2_found = calloc(LY_ARRAY_COUNT(uniques2[i]), sizeof *unique2_found);
        LY_CHECK_ERR_GOTO(!unique2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

        LY_ARRAY_FOR(uniques1[i], u) {
            found = 0;
            LY_ARRAY_FOR(uniques2[i], v) {
                if (unique2_found[v]) {
                    continue;
                }

                term1 = uniques1[i][u];
                term2 = uniques2[i][v];

                /* leaf node */
                if (!strcmp(term1->module->name, term2->module->name) && !strcmp(term1->name, term2->name)) {
                    /* found */
                    found = 1;
                    unique2_found[v] = 1;
                    break;
                }
            }

            if (!found) {
                /* removed */
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_UNIQUE,
                        1, changes), cleanup);
            }
        }

        LY_ARRAY_FOR(uniques2[i], v) {
            if (unique2_found[v]) {
                continue;
            }

            /* added */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_UNIQUE, 1,
                    changes), cleanup);
        }

        free(unique2_found);
        unique2_found = NULL;
    }

    if (i == LY_ARRAY_COUNT(uniques1)) {
        /* added */
        while (i < LY_ARRAY_COUNT(uniques2)) {
            LY_ARRAY_FOR(uniques2[i], v) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_UNIQUE, 1,
                        changes), cleanup);
            }

            ++i;
        }
    } else {
        /* removed */
        assert(i == LY_ARRAY_COUNT(uniques2));
        while (i < LY_ARRAY_COUNT(uniques1)) {
            LY_ARRAY_FOR(uniques1[i], u) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_UNIQUE,
                        1, changes), cleanup);
            }
        }
    }

cleanup:
    free(unique2_found);
    return rc;
}

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
static LY_ERR
schema_diff_node_type_range_change(const struct lysc_range *range1, const struct lysc_range *range2, ly_bool sign,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    int match, part_match;
    LY_ARRAY_COUNT_TYPE u, v;
    int64_t min1_s, max1_s, min2_s, max2_s;
    uint64_t min1_u, max1_u, min2_u, max2_u;

    if (!range1 && !range2) {
        return LY_SUCCESS;
    } else if (!range1) {
        /* added */
        return schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed,
                sign ? LYS_CHANGED_RANGE : LYS_CHANGED_LENGTH, 1, changes);
    } else if (!range2) {
        /* removed */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed,
                sign ? LYS_CHANGED_RANGE : LYS_CHANGED_LENGTH, 0, changes);
    }

    /* parts (intervals) */
    match = 0;
    LY_ARRAY_FOR(range1->parts, u) {
        if (sign) {
            min1_s = range1->parts[u].min_64;
            max1_s = range1->parts[u].max_64;
        } else {
            min1_u = range1->parts[u].min_u64;
            max1_u = range1->parts[u].max_u64;
        }

        /* find an interval with this min and max */
        part_match = 0;
        LY_ARRAY_FOR(range2->parts, v) {
            if (sign) {
                min2_s = range2->parts[v].min_64;
                max2_s = range2->parts[v].max_64;

                if ((min2_s == min1_s) && (max2_s == max1_s)) {
                    /* exact match */
                    part_match = 2;
                    break;
                } else if ((min2_s <= min1_s) && (max2_s >= max1_s)) {
                    /* larger interval */
                    part_match = 1;
                    break;
                }
            } else {
                min2_u = range2->parts[v].min_u64;
                max2_u = range2->parts[v].max_u64;

                if ((min2_u == min1_u) && (max2_u == max1_u)) {
                    /* exact match */
                    part_match = 2;
                    break;
                } else if ((min2_u <= min1_u) && (max2_u >= max1_u)) {
                    /* larger interval */
                    part_match = 1;
                    break;
                }
            }
        }

        /* store the partial result */
        if ((match == 0) || (part_match < 2)) {
            match = part_match;
        }

        if (!match) {
            /* final result */
            break;
        }
    }

    if (match == 0) {
        /* NBC change */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed,
                sign ? LYS_CHANGED_RANGE : LYS_CHANGED_LENGTH, 1, changes));
    } else if (match == 1) {
        /* BC change */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed,
                sign ? LYS_CHANGED_RANGE : LYS_CHANGED_LENGTH, 0, changes));
    }

    /* description, reference, error-message, error-app-tag */
    LY_CHECK_RET(schema_diff_text_bc(range1->dsc, range2->dsc, LYS_CHANGED_TYPE, LYS_CHANGED_DESCRIPTION, changes));
    LY_CHECK_RET(schema_diff_text_bc(range1->ref, range2->ref, LYS_CHANGED_TYPE, LYS_CHANGED_REFERENCE, changes));
    LY_CHECK_RET(schema_diff_text_nbc(range1->emsg, range2->emsg, LYS_CHANGED_TYPE, LYS_CHANGED_ERR_MSG, changes));
    LY_CHECK_RET(schema_diff_text_nbc(range1->eapptag, range2->eapptag, LYS_CHANGED_TYPE, LYS_CHANGED_ERR_APP_TAG,
            changes));

    /* ext-instance */
    LY_CHECK_RET(schema_diff_ext_insts_change(range1->exts, range2->exts, LYS_CHANGED_TYPE, ext_changes, diff));

    return LY_SUCCESS;
}

/**
 * @brief Check changes of 'pattern' arrays.
 *
 * @param[in] patterns1 First pattern array.
 * @param[in] patterns2 Second pattern array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_patterns_change(struct lysc_pattern **patterns1, struct lysc_pattern **patterns2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *pattern2_found = NULL;

    /* prepare array for marking found patterns */
    pattern2_found = calloc(LY_ARRAY_COUNT(patterns2), sizeof *pattern2_found);
    LY_CHECK_ERR_GOTO(!pattern2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(patterns1, u) {
        found = 0;
        LY_ARRAY_FOR(patterns2, v) {
            if (pattern2_found[v]) {
                continue;
            }

            /* pattern */
            if (!strcmp(patterns1[u]->expr, patterns2[v]->expr)) {
                /* found */
                found = 1;
                pattern2_found[v] = 1;
                break;
            }
        }
        if (!found) {
            /* removed, always BC */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_PATTERN, 0,
                    changes), cleanup);
            continue;
        }

        /* description, reference, error-message, error-app-tag */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->dsc, patterns2[u]->dsc, LYS_CHANGED_PATTERN,
                LYS_CHANGED_DESCRIPTION, changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->ref, patterns2[u]->ref, LYS_CHANGED_PATTERN,
                LYS_CHANGED_REFERENCE, changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->emsg, patterns2[u]->emsg, LYS_CHANGED_PATTERN,
                LYS_CHANGED_ERR_MSG, changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->eapptag, patterns2[u]->eapptag, LYS_CHANGED_PATTERN,
                LYS_CHANGED_ERR_APP_TAG, changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(patterns1[u]->exts, patterns2[u]->exts, LYS_CHANGED_PATTERN,
                ext_changes, diff), cleanup);

        /* inverted */
        if (patterns1[u]->inverted != patterns2[u]->inverted) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_PATTERN, 1,
                    changes), cleanup);
        }
    }

    LY_ARRAY_FOR(patterns2, v) {
        if (pattern2_found[v]) {
            continue;
        }

        /* added, detect compatibility by the extension presence */
        if (schema_diff_has_bc_ext(patterns2[v]->exts)) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_PATTERN, 0,
                    changes), cleanup);
        } else {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_PATTERN, 1,
                    changes), cleanup);
        }
        break;
    }

cleanup:
    free(pattern2_found);
    return rc;
}

/**
 * @brief Check changes of 'bit' or 'enum' arrays.
 *
 * @param[in] bitenums1 First bit/enum array.
 * @param[in] bitenums2 Second bit/enum array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_bitenum_change(const struct lysc_type_bitenum_item *bitenums1,
        const struct lysc_type_bitenum_item *bitenums2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes, struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *bitenum2_found = NULL;
    enum lys_diff_changed_e changed = LYS_CHANGED_NONE;

    /* enum or bit */
    assert(bitenums1 && bitenums2);
    if (bitenums1[0].flags & LYS_IS_ENUM) {
        changed = LYS_CHANGED_ENUM;
    } else {
        changed = LYS_CHANGED_BIT;
    }

    /* prepare array for marking found bitenums */
    bitenum2_found = calloc(LY_ARRAY_COUNT(bitenums2), sizeof *bitenum2_found);
    LY_CHECK_ERR_GOTO(!bitenum2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    LY_ARRAY_FOR(bitenums1, u) {
        found = 0;
        LY_ARRAY_FOR(bitenums2, v) {
            if (bitenum2_found[v]) {
                continue;
            }

            /* name */
            if (!strcmp(bitenums1[u].name, bitenums2[v].name)) {
                /* found */
                found = 1;
                bitenum2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, 1, changes), cleanup);
            continue;
        }

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(bitenums1[u].dsc, bitenums2[v].dsc, changed, LYS_CHANGED_DESCRIPTION,
                changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_text_bc(bitenums1[u].ref, bitenums2[v].ref, changed, LYS_CHANGED_REFERENCE,
                changes), cleanup);

        /* if-features not supported */

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(bitenums1[u].exts, bitenums2[v].exts,
                parent_changed ? parent_changed : changed, ext_changes, diff), cleanup);

        /* value/position, does not matter */
        if (bitenums1[u].value != bitenums2[v].value) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, 1,
                    changes), cleanup);
        }

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(bitenums1[u].flags, bitenums2[v].flags, changed, changes),
                cleanup);
    }

    LY_ARRAY_FOR(bitenums2, v) {
        if (bitenum2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, 0, changes), cleanup);
    }

cleanup:
    free(bitenum2_found);
    return rc;
}

/**
 * @brief Check changes of 'base' arrays.
 *
 * @param[in] bases1 First base array.
 * @param[in] bases2 Second base array.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_bases_change(struct lysc_ident **bases1, struct lysc_ident **bases2,
        struct lys_diff_changes_s *changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *base2_found = NULL;

    /* prepare array for marking found bases */
    base2_found = calloc(LY_ARRAY_COUNT(bases2), sizeof *base2_found);
    LY_CHECK_ERR_GOTO(!base2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)

    LY_ARRAY_FOR(bases1, u) {
        found = 0;
        LY_ARRAY_FOR(bases2, v) {
            if (base2_found[v]) {
                continue;
            }

            /* base */
            if (!strcmp(bases1[u]->name, bases2[v]->name)) {
                /* found */
                found = 1;
                base2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed, NBC for YANG 1.0 */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_BASE,
                    diff->is_yang10, changes), cleanup);
        }
    }

    LY_ARRAY_FOR(bases2, v) {
        if (base2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_BASE, 1,
                changes), cleanup);
    }

cleanup:
    free(base2_found);
    return rc;
}

/**
 * @brief Check changes of 'union' 'type' arrays.
 *
 * @param[in] types1 First type array.
 * @param[in] types2 Second type array.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_union_change(struct lysc_type **types1, struct lysc_type **types2,
        struct lys_diff_changes_s *changes, struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *type2_found = NULL;

    /* prepare array for marking found types */
    type2_found = calloc(LY_ARRAY_COUNT(types2), sizeof *type2_found);
    LY_CHECK_ERR_GOTO(!type2_found, LOGMEM(NULL); rc = LY_EMEM, cleanup)

    LY_ARRAY_FOR(types1, u) {
        found = 0;
        LY_ARRAY_FOR(types2, v) {
            if (type2_found[v]) {
                continue;
            }

            /* basetype */
            if (types1[u]->basetype == types2[v]->basetype) {
                /* found */
                found = 1;
                type2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_TYPE, 1,
                    changes), cleanup);
            continue;
        }

        /* type */
        LY_CHECK_GOTO(rc = schema_diff_node_type_change(types1[u], types2[u], LYS_CHANGED_TYPE, changes, ext_changes,
                diff), cleanup);
    }

    LY_ARRAY_FOR(types2, v) {
        if (type2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_TYPE, 0,
                changes), cleanup);
    }

cleanup:
    free(type2_found);
    return rc;
}

/**
 * @brief Check changes of a 'type'.
 *
 * @param[in] type1 First type.
 * @param[in] type2 Second type.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_change(const struct lysc_type *type1, const struct lysc_type *type2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    const struct lysc_type_num *type1_num, *type2_num;
    const struct lysc_type_dec *type1_dec, *type2_dec;
    const struct lysc_type_str *type1_str, *type2_str;
    const struct lysc_type_enum *type1_enum, *type2_enum;
    const struct lysc_type_bits *type1_bits, *type2_bits;
    const struct lysc_type_leafref *type1_lref, *type2_lref;
    const struct lysc_type_identityref *type1_identref, *type2_identref;
    const struct lysc_type_instanceid *type1_instid, *type2_instid;
    const struct lysc_type_union *type1_union, *type2_union;
    const struct lysc_type_bin *type1_bin, *type2_bin;

    /* basetype */
    if (type1->basetype != type2->basetype) {
        /* we cannot compare different types */
        return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_TYPE, 1, changes);
    }

    switch (type1->basetype) {
    case LY_TYPE_BINARY:
        type1_bin = (const struct lysc_type_bin *)type1;
        type2_bin = (const struct lysc_type_bin *)type2;

        /* range */
        LY_CHECK_RET(schema_diff_node_type_range_change(type1_bin->length, type2_bin->length, 1, parent_changed,
                changes, ext_changes, diff));
        break;
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
        type1_num = (const struct lysc_type_num *)type1;
        type2_num = (const struct lysc_type_num *)type2;

        /* range */
        LY_CHECK_RET(schema_diff_node_type_range_change(type1_num->range, type2_num->range, 1, parent_changed,
                changes, ext_changes, diff));
        break;
    case LY_TYPE_STRING:
        type1_str = (const struct lysc_type_str *)type1;
        type2_str = (const struct lysc_type_str *)type2;

        /* length */
        LY_CHECK_RET(schema_diff_node_type_range_change(type1_str->length, type2_str->length, 0, parent_changed,
                changes, ext_changes, diff));

        /* pattern */
        LY_CHECK_RET(schema_diff_node_type_patterns_change(type1_str->patterns, type2_str->patterns, parent_changed,
                changes, ext_changes, diff));
        break;
    case LY_TYPE_BITS:
        type1_bits = (const struct lysc_type_bits *)type1;
        type2_bits = (const struct lysc_type_bits *)type2;

        /* bit */
        LY_CHECK_RET(schema_diff_node_type_bitenum_change(type1_bits->bits, type2_bits->bits, parent_changed,
                changes, ext_changes, diff));
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        break;
    case LY_TYPE_DEC64:
        type1_dec = (const struct lysc_type_dec *)type1;
        type2_dec = (const struct lysc_type_dec *)type2;

        /* fraction-digits */
        if (type1_dec->fraction_digits != type2_dec->fraction_digits) {
            LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_FRAC_DIG, 1,
                    changes));
        }

        /* range */
        LY_CHECK_RET(schema_diff_node_type_range_change(type1_dec->range, type2_dec->range, 1, parent_changed,
                changes, ext_changes, diff));
        break;
    case LY_TYPE_ENUM:
        type1_enum = (const struct lysc_type_enum *)type1;
        type2_enum = (const struct lysc_type_enum *)type2;

        /* enum */
        LY_CHECK_RET(schema_diff_node_type_bitenum_change(type1_enum->enums, type2_enum->enums, parent_changed,
                changes, ext_changes, diff));
        break;
    case LY_TYPE_IDENT:
        type1_identref = (const struct lysc_type_identityref *)type1;
        type2_identref = (const struct lysc_type_identityref *)type2;

        /* base */
        LY_CHECK_RET(schema_diff_node_type_bases_change(type1_identref->bases, type2_identref->bases, changes, diff));
        break;
    case LY_TYPE_INST:
        type1_instid = (const struct lysc_type_instanceid *)type1;
        type2_instid = (const struct lysc_type_instanceid *)type2;

        /* require-instance */
        LY_CHECK_RET(schema_diff_req_inst_change(type1_instid->require_instance, type2_instid->require_instance,
                parent_changed, changes));
        break;
    case LY_TYPE_LEAFREF:
        type1_lref = (const struct lysc_type_leafref *)type1;
        type2_lref = (const struct lysc_type_leafref *)type2;

        /* path */
        if (strcmp(lyxp_get_expr(type1_lref->path), lyxp_get_expr(type2_lref->path))) {
            LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_PATH, 1, changes));
        }

        /* require-instance */
        LY_CHECK_RET(schema_diff_req_inst_change(type1_lref->require_instance, type2_lref->require_instance,
                parent_changed, changes));
        break;
    case LY_TYPE_UNION:
        type1_union = (const struct lysc_type_union *)type1;
        type2_union = (const struct lysc_type_union *)type2;

        /* type */
        LY_CHECK_RET(schema_diff_node_type_union_change(type1_union->types, type2_union->types, changes, ext_changes, diff));
        break;
    case LY_TYPE_UNKNOWN:
        /* invalid */
        assert(0);
        LOGINT_RET(NULL);
    }

    /* ext-instance */
    LY_CHECK_RET(schema_diff_ext_insts_change(type1->exts, type2->exts, LYS_CHANGED_TYPE, ext_changes, diff));

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a node.
 *
 * @param[in] node1 First node.
 * @param[in] node2 Second node.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_change(const struct lysc_node *node1, const struct lysc_node *node2, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    const struct lysc_node_leaf *term1, *term2;
    const struct lysc_node_leaflist *llist1, *llist2;
    const struct lysc_node_list *list1, *list2;
    const struct lysc_node_choice *choic1, *choic2;

    if (!node1) {
        /* node added change */
        assert(node2);
        return schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_NODE,
                (node2->flags & LYS_MAND_TRUE) ? 1 : 0, changes);
    } else if (!node2) {
        /* node removed change */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_NODE,
                (node1->flags & LYS_STATUS_OBSLT) ? 0 : 1, changes);
    }

    /* config */
    LY_CHECK_RET(schema_diff_config_change(node1->flags, node2->flags, LYS_CHANGED_NONE, changes));

    /* description */
    LY_CHECK_RET(schema_diff_node_description(node1->dsc, node2->dsc, node2->exts, LYS_CHANGED_NONE, changes));

    /* must */
    LY_CHECK_RET(schema_diff_node_musts_change(lysc_node_musts(node1), lysc_node_musts(node2), LYS_CHANGED_NONE,
            changes, ext_changes, diff));

    /* presence */
    if (node1->nodetype == LYS_CONTAINER) {
        LY_CHECK_RET(schema_diff_node_presence_change(node1->flags, node2->flags, changes));
    }

    /* reference */
    LY_CHECK_RET(schema_diff_text_bc(node1->ref, node2->ref, LYS_CHANGED_NONE, LYS_CHANGED_REFERENCE, changes));

    if (diff->with_parsed && diff->with_priv_parsed) {
        /* if-features */
        LY_CHECK_RET(schema_diff_iffeatures_change(((struct lysp_node *)node1->priv)->iffeatures, node1->flags,
                ((struct lysp_node *)node2->priv)->iffeatures, LYS_CHANGED_NODE, changes));
    }

    /* status */
    LY_CHECK_RET(schema_diff_status_change(node1->flags, node2->flags, LYS_CHANGED_NONE, changes));

    /* mandatory */
    if (node1->nodetype & (LYS_LEAF | LYS_CHOICE | LYD_NODE_ANY)) {
        LY_CHECK_RET(schema_diff_node_mandatory_change(node1->flags, node2->flags, LYS_CHANGED_NONE, changes));
    }

    /* when */
    LY_CHECK_RET(schema_diff_node_whens_change(lysc_node_when(node1), lysc_node_when(node2), LYS_CHANGED_NONE,
            changes, ext_changes, diff));

    /* type */
    if (node1->nodetype & LYD_NODE_TERM) {
        term1 = (const struct lysc_node_leaf *)node1;
        term2 = (const struct lysc_node_leaf *)node2;

        LY_CHECK_RET(schema_diff_node_type_change(term1->type, term2->type, LYS_CHANGED_NONE, changes, ext_changes, diff));
    }

    /* units */
    if (node1->nodetype & LYD_NODE_TERM) {
        term1 = (const struct lysc_node_leaf *)node1;
        term2 = (const struct lysc_node_leaf *)node2;

        LY_CHECK_RET(schema_diff_text_bc_add(term1->units, term2->units, LYS_CHANGED_NONE, LYS_CHANGED_UNITS,
                changes));
    }

    /* ordered-by */
    if (node1->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        LY_CHECK_RET(schema_diff_node_ordby_change(node1->flags, node2->flags, LYS_CHANGED_NONE, changes));
    }

    /* default */
    if (node1->nodetype == LYS_LEAF) {
        term1 = (const struct lysc_node_leaf *)node1;
        term2 = (const struct lysc_node_leaf *)node2;

        LY_CHECK_RET(schema_diff_text_bc_add(term1->dflt.str, term2->dflt.str, LYS_CHANGED_NONE, LYS_CHANGED_DEFAULT,
                changes));
    } else if (node1->nodetype == LYS_LEAFLIST) {
        llist1 = (const struct lysc_node_leaflist *)node1;
        llist2 = (const struct lysc_node_leaflist *)node2;

        LY_CHECK_RET(schema_diff_node_defaults_change(llist1, llist2, LYS_CHANGED_NONE, changes));
    } else if (node1->nodetype == LYS_CHOICE) {
        choic1 = (const struct lysc_node_choice *)node1;
        choic2 = (const struct lysc_node_choice *)node2;

        LY_CHECK_RET(schema_diff_text_nbc(choic1->dflt ? choic1->dflt->name : NULL,
                choic2->dflt ? choic2->dflt->name : NULL, LYS_CHANGED_NONE, LYS_CHANGED_DEFAULT, changes));
    }

    /* min-elements, max-elements */
    if (node1->nodetype == LYS_LEAFLIST) {
        llist1 = (const struct lysc_node_leaflist *)node1;
        llist2 = (const struct lysc_node_leaflist *)node2;

        LY_CHECK_RET(schema_diff_elem_limit_change(llist1->min, 1, llist2->min, 1, LYS_CHANGED_NONE,
                LYS_CHANGED_MIN_ELEM, changes));
        LY_CHECK_RET(schema_diff_elem_limit_change(llist1->max, 1, llist2->max, 1, LYS_CHANGED_NONE,
                LYS_CHANGED_MAX_ELEM, changes));
    } else if (node1->nodetype == LYS_LIST) {
        list1 = (const struct lysc_node_list *)node1;
        list2 = (const struct lysc_node_list *)node2;

        LY_CHECK_RET(schema_diff_elem_limit_change(list1->min, 1, list2->min, 1, LYS_CHANGED_NONE, LYS_CHANGED_MIN_ELEM,
                changes));
        LY_CHECK_RET(schema_diff_elem_limit_change(list1->max, 1, list2->max, 1, LYS_CHANGED_NONE, LYS_CHANGED_MAX_ELEM,
                changes));
    }

    /* unique */
    if (node1->nodetype == LYS_LIST) {
        list1 = (const struct lysc_node_list *)node1;
        list2 = (const struct lysc_node_list *)node2;

        LY_CHECK_RET(schema_diff_node_uniques_change(list1->uniques, list2->uniques, changes));
    }

    /* ext-instance */
    LY_CHECK_RET(schema_diff_ext_insts_change(node1->exts, node2->exts, LYS_CHANGED_NONE, ext_changes, diff));

    return LY_SUCCESS;
}

/**
 * @brief Check changes of extension-instance substatement arrays.
 *
 * @param[in] substmts1 First substatement array.
 * @param[in] substmts2 Second substatement array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Nested ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_change(const struct lysc_ext_substmt *substmts1, const struct lysc_ext_substmt *substmts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysc_ext_substmt **substmts2_array = NULL;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, siblings_checked = 0;

    /* collect all the compiled substatements2 to remove from */
    substmts2_array = calloc(LY_ARRAY_COUNT(substmts2), sizeof *substmts2_array);
    LY_CHECK_ERR_GOTO(!substmts2_array, LOGMEM(NULL); rc = LY_EMEM, cleanup);
    LY_ARRAY_FOR(substmts2, u) {
        if (substmts2[u].storage_p) {
            substmts2_array[u] = &substmts2[u];
        }
    }

    LY_ARRAY_FOR(substmts1, u) {
        if (!substmts1[u].storage_p) {
            /* not compiled */
            continue;
        }

        found = 0;
        for (v = 0; v < LY_ARRAY_COUNT(substmts2); ++v) {
            if (!substmts2_array[v]) {
                continue;
            }

            if (substmts1[u].stmt == substmts2_array[v]->stmt) {
                /* found */
                found = 1;
                substmts2_array[v] = NULL;
                break;
            }
        }
        if (!found) {
            /* compiled statement not found in substmts2 */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed,
                    schema_diff_stmt2changed(substmts1[u].stmt), 1, changes), cleanup);
            continue;
        }

        switch (substmts1[u].stmt) {
        case LY_STMT_ACTION:
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_CASE:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_INPUT:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_NOTIFICATION:
        case LY_STMT_OUTPUT:
        case LY_STMT_RPC:
        case LY_STMT_USES:
            /* all the nodes are connected into a sibling list, do not repeat the check */
            if (siblings_checked) {
                break;
            }
            siblings_checked = 1;

            LY_CHECK_GOTO(rc = schema_diff_nodes_change_r(*(substmts1[u].storage_p), *(substmts2[v].storage_p), diff),
                    cleanup);
            break;
        case LY_STMT_ARGUMENT:
        case LY_STMT_ERROR_APP_TAG:
        case LY_STMT_ERROR_MESSAGE:
        case LY_STMT_KEY:
        case LY_STMT_MODIFIER:
        case LY_STMT_NAMESPACE:
            /* text NBC */
            LY_CHECK_GOTO(rc = schema_diff_text_nbc(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, schema_diff_stmt2changed(substmts1[u].stmt), changes), cleanup);
            break;
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_ORGANIZATION:
        case LY_STMT_PRESENCE:
        case LY_STMT_REFERENCE:
            /* text BC */
            LY_CHECK_GOTO(rc = schema_diff_text_bc(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, schema_diff_stmt2changed(substmts1[u].stmt), changes), cleanup);
            break;
        case LY_STMT_UNITS:
            /* text BC add */
            LY_CHECK_GOTO(rc = schema_diff_text_bc_add(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, schema_diff_stmt2changed(substmts1[u].stmt), changes), cleanup);
            break;
        case LY_STMT_BIT:
        case LY_STMT_ENUM:
            /* bitenum struct array */
            LY_CHECK_GOTO(rc = schema_diff_node_type_bitenum_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_CONFIG:
            /* config flag */
            LY_CHECK_GOTO(rc = schema_diff_config_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[u].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            /* ext-instance */
            LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, ext_changes, diff), cleanup);
            break;
        case LY_STMT_FRACTION_DIGITS:
        case LY_STMT_REQUIRE_INSTANCE:
            /* uint8 number */
            if (*(uint8_t *)substmts1[u].storage_p != *(uint8_t *)substmts2[u].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_EXT_INST,
                        schema_diff_stmt2changed(substmts1[u].stmt), 1, changes), cleanup);
            }
            break;
        case LY_STMT_IDENTITY:
            /* identity */
            LY_CHECK_GOTO(rc = schema_diff_module_identities_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, diff), cleanup);
            break;
        case LY_STMT_LENGTH:
            /* length */
            LY_CHECK_GOTO(rc = schema_diff_node_type_range_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    0, LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_RANGE:
            /* range */
            LY_CHECK_GOTO(rc = schema_diff_node_type_range_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    1, LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_MANDATORY:
            /* mandatory */
            LY_CHECK_GOTO(rc = schema_diff_node_mandatory_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[u].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_ORDERED_BY:
            /* odrered-by */
            LY_CHECK_GOTO(rc = schema_diff_node_ordby_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[u].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_MAX_ELEMENTS:
        case LY_STMT_MIN_ELEMENTS:
            /* uint32 number */
            if (*(uint32_t *)substmts1[u].storage_p != *(uint32_t *)substmts2[u].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_EXT_INST,
                        schema_diff_stmt2changed(substmts1[u].stmt), 1, changes), cleanup);
            }
            break;
        case LY_STMT_MUST:
            /* must array */
            LY_CHECK_GOTO(rc = schema_diff_node_musts_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_PATTERN:
            /* pattern array of arrays */
            LY_CHECK_GOTO(rc = schema_diff_node_type_patterns_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_POSITION:
        case LY_STMT_VALUE:
            /* uint64/int64 number */
            if (*(uint64_t *)substmts1[u].storage_p != *(uint64_t *)substmts2[u].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_EXT_INST,
                        schema_diff_stmt2changed(substmts1[u].stmt), 1, changes), cleanup);
            }
            break;
        case LY_STMT_STATUS:
            /* status flag */
            LY_CHECK_GOTO(rc = schema_diff_status_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[u].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_TYPE:
            /* type */
            LY_CHECK_GOTO(rc = schema_diff_node_type_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_WHEN:
            /* when array */
            LY_CHECK_GOTO(rc = schema_diff_node_whens_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_ARG_TEXT:
        case LY_STMT_ARG_VALUE:
        case LY_STMT_AUGMENT:
        case LY_STMT_BASE:
        case LY_STMT_BELONGS_TO:
        case LY_STMT_DEFAULT:
        case LY_STMT_DEVIATE:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_IF_FEATURE:
        case LY_STMT_GROUPING:
        case LY_STMT_IMPORT:
        case LY_STMT_INCLUDE:
        case LY_STMT_MODULE:
        case LY_STMT_PATH:
        case LY_STMT_PREFIX:
        case LY_STMT_REFINE:
        case LY_STMT_REVISION:
        case LY_STMT_REVISION_DATE:
        case LY_STMT_SUBMODULE:
        case LY_STMT_SYNTAX_LEFT_BRACE:
        case LY_STMT_SYNTAX_RIGHT_BRACE:
        case LY_STMT_SYNTAX_SEMICOLON:
        case LY_STMT_TYPEDEF:
        case LY_STMT_UNIQUE:
        case LY_STMT_YANG_VERSION:
        case LY_STMT_YIN_ELEMENT:
        case LY_STMT_NONE:
            /* not compiled/invalid */
            LOGINT_RET(NULL);
        }
    }

    for (v = 0; v < LY_ARRAY_COUNT(substmts2); ++v) {
        if (!substmts2_array[v]) {
            continue;
        }

        /* compiled statement not found in substmts1 */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_EXT_INST,
                schema_diff_stmt2changed(substmts2_array[v]->stmt), 1, changes), cleanup);
    }

cleanup:
    free(substmts2_array);
    return rc;
}

/**
 * @brief Check changes of extension-instance arrays.
 *
 * @param[in] exts1 First ext-inst array.
 * @param[in] exts2 Second ext-inst array.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_insts_change(const struct lysc_ext_instance *exts1, const struct lysc_ext_instance *exts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool *exts2_found, found;
    enum lys_diff_changed_e ext_parent_changed;
    struct lys_diff_ext_change_s *ext_change;
    LY_ARRAY_COUNT_TYPE u, v;

    /* prepare array for marking found exts */
    exts2_found = calloc(LY_ARRAY_COUNT(exts2), sizeof *exts2_found);
    LY_CHECK_ERR_GOTO(!exts2_found, LOGMEM(NULL); rc = LY_SUCCESS, cleanup);

    LY_ARRAY_FOR(exts1, u) {
        found = 0;
        LY_ARRAY_FOR(exts2, v) {
            if (!strcmp(exts1[u].def->name, exts2[v].def->name) && !strcmp(exts1[u].argument, exts2[v].argument)) {
                found = 1;
                exts2_found[v] = 1;
                break;
            }
        }

        /* add new ext-instance to changes */
        LY_CHECK_RET(schema_diff_add_ext_change(&exts1[u], found ? &exts2[v] : NULL, ext_changes, &ext_change));

        if (!found) {
            /* removed */
            if (schema_diff_has_bc_ext(exts1[u].exts)) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_EXT_INST, 0,
                        ext_change->changes), cleanup);
            } else {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_EXT_INST, 1,
                        ext_change->changes), cleanup);

                /* NBC */
                diff->is_nbc = 1;
            }
            continue;
        }

        /* substatements */
        ext_parent_changed = (parent_changed == LYS_CHANGED_NONE) ? LYS_CHANGED_EXT_INST : parent_changed;
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_change(exts1[u].substmts, exts2[v].substmts, ext_parent_changed,
                ext_change->changes, ext_changes, diff), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(ext_change->changes, diff);
    }

    LY_ARRAY_FOR(exts2, v) {
        if (exts2_found[v]) {
            continue;
        }

        /* add new ext-instance to changes */
        LY_CHECK_RET(schema_diff_add_ext_change(NULL, &exts2[v], ext_changes, &ext_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_EXT_INST, 0,
                ext_change->changes), cleanup);
    }

cleanup:
    free(exts2_found);
    return rc;
}

/**
 * @brief Check changes of nodes, recursively.
 *
 * @param[in] node1 First node.
 * @param[in] node2 Second node.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_nodes_change_r(const struct lysc_node *node1, const struct lysc_node *node2, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_diff_node_change_s *node_change;
    const struct lysc_node **node2_array = NULL, *iter;
    uint32_t node2_count = 0, i, prev_i = 0;

    /* prepare array for node2 siblings */
    LY_LIST_FOR(node2, iter) {
        node2_array = ly_realloc(node2_array, (node2_count + 1) * sizeof *node2_array);
        LY_CHECK_ERR_GOTO(!node2_array, LOGMEM(NULL); rc = LY_SUCCESS, cleanup);
        node2_array[node2_count] = iter;
        ++node2_count;
    }

    LY_LIST_FOR(node1, node1) {
        node2 = NULL;
        for (i = 0; i < node2_count; ++i) {
            if (!node2_array[i]) {
                continue;
            }

            if (!strcmp(node1->module->name, node2_array[i]->module->name) && !strcmp(node1->name, node2_array[i]->name)) {
                /* matching nodes */
                node2 = node2_array[i];
                break;
            }
        }

        /* add new node to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_node_change(node1, node2, diff, &node_change), cleanup);

        if (node2 && (i < prev_i)) {
            /* moved */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MOVED, LYS_CHANGED_NONE, LYS_CHANGED_NODE, 1,
                    &node_change->changes), cleanup);
        }

        /* node changes (removed, moved, modified) */
        LY_CHECK_GOTO(rc = schema_diff_node_change(node1, node2, &node_change->changes, &node_change->ext_changes, diff),
                cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&node_change->changes, diff);

        /* recursive changes */
        LY_CHECK_GOTO(rc = schema_diff_nodes_change_r(lysc_node_child(node1), lysc_node_child(node2), diff), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_nodes_change_r((const struct lysc_node *)lysc_node_actions(node1),
                (const struct lysc_node *)lysc_node_actions(node2), diff), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_nodes_change_r((const struct lysc_node *)lysc_node_notifs(node1),
                (const struct lysc_node *)lysc_node_notifs(node2), diff), cleanup);

        if (node2) {
            /* match found */
            prev_i = i;
            node2_array[i] = NULL;
        }
    }

    for (i = 0; i < node2_count; ++i) {
        if (!node2_array[i]) {
            continue;
        }

        /* add new node to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_node_change(NULL, node2_array[i], diff, &node_change), cleanup);

        /* node changes (added) */
        LY_CHECK_GOTO(rc = schema_diff_node_change(NULL, node2_array[i], &node_change->changes,
                &node_change->ext_changes, diff), cleanup);

        /* check whether any of the changes were NBC */
        schema_diff_check_node_change_nbc(&node_change->changes, diff);

        /* recursive changes */
        LY_CHECK_GOTO(rc = schema_diff_nodes_change_r(NULL, lysc_node_child(node2_array[i]), diff), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_nodes_change_r(NULL, (const struct lysc_node *)lysc_node_actions(node2_array[i]),
                diff), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_nodes_change_r(NULL, (const struct lysc_node *)lysc_node_notifs(node2_array[i]),
                diff), cleanup);
    }

cleanup:
    free(node2_array);
    return rc;
}

LY_ERR
lysc_diff_changes(const struct lys_module *mod1, const struct lys_module *mod2, struct lys_diff_s *diff)
{
    if (diff->with_parsed) {
        /* parsed module changes */
        LY_CHECK_RET(schema_diff_pmodule_change(mod1->parsed, mod2->parsed, diff));
    }

    /* compiled module changes */
    LY_CHECK_RET(schema_diff_module_change(mod1, mod2, diff));

    /* compiled node changes */
    LY_CHECK_RET(schema_diff_nodes_change_r(mod1->compiled->data, mod2->compiled->data, diff));
    LY_CHECK_RET(schema_diff_nodes_change_r((const struct lysc_node *)mod1->compiled->rpcs,
            (const struct lysc_node *)mod2->compiled->rpcs, diff));
    LY_CHECK_RET(schema_diff_nodes_change_r((const struct lysc_node *)mod1->compiled->notifs,
            (const struct lysc_node *)mod2->compiled->notifs, diff));

    return LY_SUCCESS;
}

/**
 * @brief Erase ext-instance changes array.
 *
 * @param[in] ext_changes Ext-instance changes structure to erase.
 */
static void
lysc_diff_erase_ext_changes(struct lys_diff_ext_changes_s *ext_changes)
{
    uint32_t i;

    for (i = 0; i < ext_changes->count; ++i) {
        free(ext_changes->changes[i].changes->changes);
        free(ext_changes->changes[i].changes);
    }
    free(ext_changes->changes);
}

void
lysc_diff_erase(struct lys_diff_s *diff)
{
    uint32_t i;

    /* module */
    free(diff->module_changes.changes);

    for (i = 0; i < diff->ident_change_count; ++i) {
        free(diff->ident_changes[i].changes.changes);
        lysc_diff_erase_ext_changes(&diff->ident_changes[i].ext_changes);
    }
    free(diff->ident_changes);

    /* parsed module */
    for (i = 0; i < diff->import_change_count; ++i) {
        free(diff->import_changes[i].changes.changes);
    }
    free(diff->import_changes);

    for (i = 0; i < diff->include_change_count; ++i) {
        free(diff->include_changes[i].changes.changes);
    }
    free(diff->include_changes);

    for (i = 0; i < diff->extension_change_count; ++i) {
        free(diff->extension_changes[i].changes.changes);
    }
    free(diff->extension_changes);

    for (i = 0; i < diff->feat_change_count; ++i) {
        free(diff->feat_changes[i].changes.changes);
    }
    free(diff->feat_changes);

    for (i = 0; i < diff->dev_change_count; ++i) {
        free(diff->dev_changes[i].changes.changes);
    }
    free(diff->dev_changes);

    lysc_diff_erase_ext_changes(&diff->mod_ext_changes);

    /* parsed node */
    for (i = 0; i < diff->pnode_change_count; ++i) {
        free(diff->pnode_changes[i].changes.changes);
    }
    free(diff->pnode_changes);

    for (i = 0; i < diff->refine_change_count; ++i) {
        free(diff->refine_changes[i].changes.changes);
    }
    free(diff->refine_changes);

    for (i = 0; i < diff->typedef_change_count; ++i) {
        free(diff->typedef_changes[i].changes.changes);
    }
    free(diff->typedef_changes);

    /* compiled node */
    for (i = 0; i < diff->node_change_count; ++i) {
        free(diff->node_changes[i].changes.changes);
        lysc_diff_erase_ext_changes(&diff->node_changes[i].ext_changes);
    }
    free(diff->node_changes);
}
