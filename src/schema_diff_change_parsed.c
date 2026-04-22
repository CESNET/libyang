/**
 * @file schema_diff_change_parsed.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema diff change parsed functionss
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
#include "schema_compile_node.h"
#include "tree_schema_internal.h"
#include "xpath.h"

static LY_ERR schema_diff_ptype_change(const struct lysp_type *type1, const struct lysp_node *ctx_node1,
        const struct lysp_type *type2, const struct lysp_node *ctx_node2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes, struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff);
static LY_ERR schema_diff_pnodes_change_r(const struct lysp_node *node1, const struct lysp_node *node2,
        struct lys_diff_s *diff);
static LY_ERR schema_diff_pext_insts_change(const struct lysp_ext_instance *exts1, const struct lysp_ext_instance *exts2,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff);

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
    pnc->ext_changes.changes = NULL;
    pnc->ext_changes.count = 0;

    *pnode_change = pnc;
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
    ic->ext_changes.changes = NULL;
    ic->ext_changes.count = 0;

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
    ic->ext_changes.changes = NULL;
    ic->ext_changes.count = 0;

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
    ec->ext_changes.changes = NULL;
    ec->ext_changes.count = 0;

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
    fc->ext_changes.changes = NULL;
    fc->ext_changes.count = 0;

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
    dc->ext_changes.changes = NULL;
    dc->ext_changes.count = 0;

    *dev_change = dc;
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
    rc->ext_changes.changes = NULL;
    rc->ext_changes.count = 0;

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
    tc->ext_changes.changes = NULL;
    tc->ext_changes.count = 0;

    *typedef_change = tc;
    return LY_SUCCESS;
}

/**
 * @brief Add a new parsed ext-instance change pair.
 *
 * @param[in] p_ext_old Old changed ext-instance.
 * @param[in] p_ext_new New changed ext-instance.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[out] ext_change Added ext-instance change structure.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_add_pext_change(const struct lysp_ext_instance *p_ext_old, const struct lysp_ext_instance *p_ext_new,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_pext_change_s **ext_change)
{
    void *mem;
    struct lys_diff_pext_change_s *ec;

    /* add new ext_change */
    mem = realloc(ext_changes->changes, (ext_changes->count + 1) * sizeof *ext_changes->changes);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    ext_changes->changes = mem;
    ec = &ext_changes->changes[ext_changes->count];
    ++ext_changes->count;

    /* fill new ext_change */
    ec->p_ext_old = p_ext_old;
    ec->p_ext_new = p_ext_new;
    ec->changes.changes = NULL;
    ec->changes.count = 0;

    *ext_change = ec;
    return LY_SUCCESS;
}

/**
 * @brief Check what conformance is signified by an extension.
 *
 * @param[in] ctx Context to use.
 * @param[in] ext Parsed ext instance to check.
 * @param[in] default_conform Conformance to return if @p ext is not a relevant extension.
 * @param[out] is_conform_ext Optional flag set if @p ext is a conformance ext instance.
 * @return Conformance of the extension.
 */
static enum lys_diff_conform_e
schema_diff_is_pext_conform(const struct ly_ctx *ctx, const struct lysp_ext_instance *ext,
        enum lys_diff_conform_e default_conform, ly_bool *is_conform_ext)
{
    const char *mod_name, *name;

    /* get the module and name */
    lysp_nodeid_find_module(ctx, ext->name, ext->format, ext->prefix_data, &mod_name, &name);

    if (is_conform_ext) {
        *is_conform_ext = 0;
    }

    /* check module name */
    if (strcmp(mod_name, "ietf-yang-schema-comparison")) {
        return default_conform;
    }

    /* check ext-inst name */
    if (!strcmp(name, "ed-change-at")) {
        if (is_conform_ext) {
            *is_conform_ext = 1;
        }
        return LYS_CONFORM_ED;
    } else if (!strcmp(name, "bc-change-at")) {
        if (is_conform_ext) {
            *is_conform_ext = 1;
        }
        return LYS_CONFORM_BC;
    } else if (!strcmp(name, "nbc-change-at")) {
        if (is_conform_ext) {
            *is_conform_ext = 1;
        }
        return LYS_CONFORM_NBC;
    } else {
        return default_conform;
    }
}

/**
 * @brief Check extensions for an explicit conformance extension and return it.
 *
 * @param[in] ctx Context to use.
 * @param[in] exts Sized-array of parsed extension-instances.
 * @param[in] parent_stmt Optional parent statement to only consider.
 * @param[in] default_conform Conformance to return if no @p exts are relevant extensions.
 * @param[out] has_conform_ext Optional flag set in case a relevant extension is found.
 * @return Found conformance.
 */
static enum lys_diff_conform_e
schema_diff_pext_conform(const struct ly_ctx *ctx, const struct lysp_ext_instance *exts, enum ly_stmt parent_stmt,
        enum lys_diff_conform_e default_conform, ly_bool *has_conform_ext)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool is_conform_ext;
    enum lys_diff_conform_e conform;

    LY_ARRAY_FOR(exts, u) {
        if (parent_stmt && (parent_stmt != exts[u].parent_stmt)) {
            continue;
        }

        conform = schema_diff_is_pext_conform(ctx, &exts[u], default_conform, &is_conform_ext);
        if (is_conform_ext) {
            if (has_conform_ext) {
                *has_conform_ext = 1;
            }
            return conform;
        }
    }

    if (has_conform_ext) {
        *has_conform_ext = 0;
    }
    return default_conform;
}

LY_ERR
schema_diff_iffeatures_change(const struct lysp_qname *iffs1, uint16_t flags1, const struct lysp_qname *iffs2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, *iff2_found = NULL;

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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_IF_FEATURE,
                    (flags1 & LYS_MAND_TRUE) ? LYS_CONFORM_NBC : LYS_CONFORM_BC, changes), cleanup);
        }
    }

    LY_ARRAY_FOR(iffs2, v) {
        if (iff2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_IF_FEATURE,
                LYS_CONFORM_NBC, changes), cleanup);
    }

cleanup:
    free(iff2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed text statement such as 'description' or 'reference'.
 *
 * @param[in] ctx Context to use.
 * @param[in] text1 First description.
 * @param[in] text2 Second description.
 * @param[in] exts2 Parsed extension instances of the second statement.
 * @param[in] stmt Changed statement.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode_text(const struct ly_ctx *ctx, const char *text1, const char *text2, const struct lysp_ext_instance *exts2,
        enum ly_stmt stmt, enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes)
{
    if (text1 && !text2) {
        /* removed, always NBC */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, schema_diff_stmt2changed(stmt),
                LYS_CONFORM_NBC, changes));
    } else if (!text1 && text2) {
        /* added */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, schema_diff_stmt2changed(stmt),
                LYS_CONFORM_BC, changes));
    } else if (text1 && text2 && strcmp(text1, text2)) {
        /* modified, check BC extension */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, schema_diff_stmt2changed(stmt),
                schema_diff_pext_conform(ctx, exts2, stmt, LYS_CONFORM_ED, NULL), changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a parsed restriction, which can be 'length', 'range', 'pattern', or 'must'.
 *
 * @param[in] restr1 First parsed restriction.
 * @param[in] range1 First compiled restriction, only 'length' or 'pattern'.
 * @param[in] restr2 Second parsed restriction.
 * @param[in] range2 Second compiled restriction, only 'length' or 'pattern'.
 * @param[in] changed Changed statement.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_restr_change(const struct lysp_restr *restr1, const struct lysc_range *range1,
        const struct lysp_restr *restr2, const struct lysc_range *range2, enum lys_diff_changed_e changed,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    enum lys_diff_conform_e conform;

    if (!restr1 && !restr2) {
        /* no changes */
        goto cleanup;
    } else if (!restr2) {
        /* removed */
        rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, LYS_CONFORM_BC, changes);
        goto cleanup;
    } else if (!restr1) {
        /* added */
        rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, LYS_CONFORM_NBC, changes);
        goto cleanup;
    }

    /* condition */
    switch (changed) {
    case LYS_CHANGED_PATTERN:
        if ((restr1->arg.str[0] != restr2->arg.str[0]) || strcmp(restr1->arg.str + 1, restr2->arg.str + 1)) {
            /* learn conformance */
            conform = schema_diff_pext_conform(diff->ctx, restr2->exts, LY_STMT_PATTERN, LYS_CONFORM_NBC, NULL);

            /* modified */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, conform, changes),
                    cleanup);
        }
        break;
    case LYS_CHANGED_MUST:
        if (strcmp(restr1->arg.str, restr2->arg.str)) {
            /* learn conformance */
            conform = schema_diff_pext_conform(diff->ctx, restr2->exts, LY_STMT_MUST, LYS_CONFORM_NBC, NULL);

            /* modified */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, conform, changes),
                    cleanup);
        }
        break;
    case LYS_CHANGED_LENGTH:
    case LYS_CHANGED_RANGE:
        if (strcmp(restr1->arg.str, restr2->arg.str)) {
            if (range1 && range2) {
                /* compare the compiled intervals */
                LY_CHECK_GOTO(rc = schema_diff_node_type_range_change(range1, range2, (changed == LYS_CHANGED_LENGTH) ? 0 : 1,
                        parent_changed, changes, NULL, NULL), cleanup);
            } else {
                /* in an ext instance, NBC unless the ext instance has 'backwards-compatible' */
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, changed, LYS_CONFORM_NBC,
                        changes), cleanup);
            }
        }
        break;
    default:
        LOGINT(diff->ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    /* description, reference, error-message, error-app-tag */
    LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, restr1->dsc, restr2->dsc, restr2->exts, LY_STMT_DESCRIPTION,
            changed, changes), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, restr1->ref, restr2->ref, restr2->exts, LY_STMT_REFERENCE,
            changed, changes), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(restr1->emsg, restr2->emsg, changed, LYS_CHANGED_ERR_MSG, changes), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(restr1->eapptag, restr2->eapptag, changed, LYS_CHANGED_ERR_APP_TAG, changes),
            cleanup);

    /* ext-instance */
    LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(restr1->exts, restr2->exts, ext_changes, diff), cleanup);

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
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_restrs_change(const struct lysp_restr *restrs1, const struct lysp_restr *restrs2,
        enum lys_diff_changed_e changed, enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, changed, LYS_CONFORM_BC,
                    changes), cleanup);
            continue;
        }

        /* restriction */
        LY_CHECK_GOTO(rc = schema_diff_parsed_restr_change(&restrs1[u], NULL, &restrs2[v], NULL, changed,
                parent_changed, changes, ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(restrs2, v) {
        if (restr2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, changed, LYS_CONFORM_NBC, changes),
                cleanup);
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
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptype_enums_change(const struct lysp_type_enum *enums1, const struct lysp_type_enum *enums2,
        enum lys_diff_changed_e changed, struct lys_diff_changes_s *changes, struct lys_diff_pext_changes_s *ext_changes,
        struct lys_diff_s *diff)
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
                found = 1;
                enum2_found[v] = 1;
                break;
            }
        }

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_TYPEDEF, changed, LYS_CONFORM_NBC,
                    changes), cleanup);
            continue;
        }

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, enums1[u].dsc, enums2[v].dsc, enums2[v].exts,
                LY_STMT_DESCRIPTION, changed, changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, enums1[u].ref, enums2[v].ref, enums2[v].exts,
                LY_STMT_REFERENCE, changed, changes), cleanup);

        /* value/position, does not matter */
        if (enums1[u].value != enums2[v].value) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_TYPEDEF, changed,
                    LYS_CONFORM_NBC, changes), cleanup);
        }

        /* if-features */
        LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(enums1[u].iffeatures, 0, enums2[v].iffeatures, changed, changes),
                cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(enums1[u].flags, enums2[v].flags, changed, changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(enums1[u].exts, enums2[v].exts, ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(enums2, v) {
        if (enum2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_TYPEDEF, changed, LYS_CONFORM_BC,
                changes), cleanup);
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_BASE,
                    LYS_CONFORM_BC, changes), cleanup);
        }
    }

    LY_ARRAY_FOR(bases2, v) {
        if (base2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_BASE,
                LYS_CONFORM_NBC, changes), cleanup);
    }

cleanup:
    free(base2_found);
    return rc;
}

/**
 * @brief Check changes of a parsed 'type' array.
 *
 * @param[in] types1 First type array.
 * @param[in] ctx_node1 Context node of @p types1, if any.
 * @param[in] types2 Second type array.
 * @param[in] ctx_node2 Context node of @p types2, if any.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptypes_change(const struct lysp_type *types1, const struct lysp_node *ctx_node1,
        const struct lysp_type *types2, const struct lysp_node *ctx_node2, struct lys_diff_changes_s *changes,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;

    if (LY_ARRAY_COUNT(types1) > LY_ARRAY_COUNT(types2)) {
        /* removed */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_TYPE,
                LYS_CONFORM_NBC, changes), cleanup);
    } else if (LY_ARRAY_COUNT(types1) < LY_ARRAY_COUNT(types2)) {
        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_TYPE,
                LYS_CONFORM_NBC, changes), cleanup);
    }

    LY_ARRAY_FOR(types1, u) {
        if (strcmp(types1[u].name, types2[u].name)) {
            /* modified */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_TYPEDEF, LYS_CHANGED_TYPE,
                    LYS_CONFORM_NBC, changes), cleanup);
        } else {
            /* type */
            LY_CHECK_GOTO(rc = schema_diff_ptype_change(&types1[u], ctx_node1, &types2[u], ctx_node2, LYS_CHANGED_TYPE,
                    changes, ext_changes, diff), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Check changes of a parsed 'type' 'length' or 'range'.
 *
 * @param[in] restr1 First length or range restriction.
 * @param[in] ctx_node1 Context node of @p type1, if any.
 * @param[in] type1 First type.
 * @param[in] restr2 Second length or range restriction.
 * @param[in] ctx_node2 Context node of @p type2, if any.
 * @param[in] type2 Second type.
 * @param[in] changed Changed statement.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptype_change_range_length(const struct lysp_restr *restr1, const struct lysp_node *ctx_node1,
        const struct lysp_type *type1, const struct lysp_restr *restr2, const struct lysp_node *ctx_node2,
        const struct lysp_type *type2, enum lys_diff_changed_e changed, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes, struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_range *range1 = NULL, *range2 = NULL;
    struct ly_set tpdf_chain = {0};
    struct lysc_ctx cctx = {0};
    LY_DATA_TYPE basetype;

    if (restr1 && restr2) {
        cctx.ctx = (struct ly_ctx *)diff->ctx;

        /* learn basetype1, ignore status (flags) */
        rc = lys_compile_type_find_tpdf(&cctx, ctx_node1, type1->flags, type1->name, type1, NULL, NULL, &tpdf_chain,
                &basetype);
        ly_set_erase(&tpdf_chain, NULL);
        if (rc == LY_ENOTFOUND) {
            /* must have been found before */
            LOGINT(diff->ctx);
            rc = LY_EINT;
            goto cleanup;
        } else if (rc) {
            goto cleanup;
        }

        /* compile range1 */
        LY_CHECK_GOTO(rc = lys_compile_type_range(&cctx, restr1, basetype, 0, type1->fraction_digits, NULL, &range1),
                cleanup);

        /* learn basetype2, ignore status (flags) */
        rc = lys_compile_type_find_tpdf(&cctx, ctx_node2, type2->flags, type2->name, type2, NULL, NULL, &tpdf_chain,
                &basetype);
        ly_set_erase(&tpdf_chain, NULL);
        if (rc == LY_ENOTFOUND) {
            LOGINT(diff->ctx);
            rc = LY_EINT;
            goto cleanup;
        } else if (rc) {
            goto cleanup;
        }

        /* compile range2 */
        LY_CHECK_GOTO(rc = lys_compile_type_range(&cctx, restr2, basetype, 0, type2->fraction_digits, NULL, &range2),
                cleanup);
    }

    /* length or range */
    LY_CHECK_GOTO(rc = schema_diff_parsed_restr_change(restr1, range1, restr2, range2, changed, parent_changed, changes,
            ext_changes, diff), cleanup);

cleanup:
    if (range1) {
        lysc_range_free(diff->ctx, range1);
        free(range1);
    }
    if (range2) {
        lysc_range_free(diff->ctx, range2);
        free(range2);
    }

    return rc;
}

/**
 * @brief Check changes of a parsed 'type'.
 *
 * @param[in] type1 First type.
 * @param[in] ctx_node1 Context node of @p type1, if any.
 * @param[in] type2 Second type.
 * @param[in] ctx_node2 Context node of @p type2, if any.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ptype_change(const struct lysp_type *type1, const struct lysp_node *ctx_node1, const struct lysp_type *type2,
        const struct lysp_node *ctx_node2, enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;

    /* basetype */
    if (strcmp(type1->name, type2->name)) {
        /* different types */
        return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_TYPE, LYS_CONFORM_NBC, changes);
    }

    /* range */
    LY_CHECK_GOTO(rc = schema_diff_ptype_change_range_length(type1->range, ctx_node1, type1, type2->range, ctx_node2,
            type2, LYS_CHANGED_RANGE, parent_changed, changes, ext_changes, diff), cleanup);

    /* length */
    LY_CHECK_GOTO(rc = schema_diff_ptype_change_range_length(type1->length, ctx_node1, type1, type2->length, ctx_node2,
            type2, LYS_CHANGED_LENGTH, parent_changed, changes, ext_changes, diff), cleanup);

    /* patterns */
    LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(type1->patterns, type2->patterns, LYS_CHANGED_PATTERN,
            parent_changed, changes, ext_changes, diff), cleanup);

    /* enums */
    LY_CHECK_GOTO(rc = schema_diff_ptype_enums_change(type1->enums, type2->enums, LYS_CHANGED_ENUM, changes,
            ext_changes, diff), cleanup);

    /* bits */
    LY_CHECK_GOTO(rc = schema_diff_ptype_enums_change(type1->bits, type2->bits, LYS_CHANGED_BIT, changes, ext_changes,
            diff), cleanup);

    /* path */
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(type1->path ? type1->path->expr : NULL,
            type2->path ? type2->path->expr : NULL, parent_changed, LYS_CHANGED_PATH, changes), cleanup);

    /* bases */
    LY_CHECK_GOTO(rc = schema_diff_ptype_bases_change(type1->bases, type2->bases, changes), cleanup);

    /* types */
    LY_CHECK_GOTO(rc = schema_diff_ptypes_change(type1->types, ctx_node1, type2->types, ctx_node2, changes, ext_changes,
            diff), cleanup);

    /* fraction-digits */
    if (type1->fraction_digits != type2->fraction_digits) {
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_FRAC_DIG,
                LYS_CONFORM_NBC, changes), cleanup);
    }

    /* require-instance */
    LY_CHECK_GOTO(rc = schema_diff_req_inst_change(type1->require_instance, type2->require_instance, parent_changed,
            changes), cleanup);

    /* ext-instance */
    LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(type1->exts, type2->exts, ext_changes, diff), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Check changes of a parsed 'typedef' array.
 *
 * @param[in] typedefs1 First typedef array.
 * @param[in] parent1 First typedef array parent node.
 * @param[in] typedefs2 Second typedef array.
 * @param[in] parent2 Second typedef array parent node.
 * @param[in] parent_changed Changed parent statement.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_typedefs_change(const struct lysp_tpdf *typedefs1, const struct lysp_node *parent1,
        const struct lysp_tpdf *typedefs2, const struct lysp_node *parent2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_s *diff)
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_TYPEDEF,
                    LYS_CONFORM_NBC, &typedef_change->changes), cleanup);
            continue;
        }

        /* units */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(typedefs1[u].units, typedefs2[v].units, LYS_CHANGED_TYPEDEF,
                LYS_CHANGED_UNITS, &typedef_change->changes), cleanup);

        /* default */
        LY_CHECK_GOTO(rc = schema_diff_text_bc_add(typedefs1[u].dflt.str, typedefs2[v].dflt.str, LYS_CHANGED_TYPEDEF,
                LYS_CHANGED_DEFAULT, &typedef_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, typedefs1[u].dsc, typedefs2[v].dsc, typedefs2[v].exts,
                LY_STMT_DESCRIPTION, LYS_CHANGED_TYPEDEF, &typedef_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, typedefs1[u].ref, typedefs2[v].ref, typedefs2[v].exts,
                LY_STMT_REFERENCE, LYS_CHANGED_TYPEDEF, &typedef_change->changes), cleanup);

        /* type */
        LY_CHECK_GOTO(rc = schema_diff_ptype_change(&typedefs1[u].type, parent1, &typedefs2[v].type, parent2,
                LYS_CHANGED_TYPEDEF, &typedef_change->changes, &typedef_change->ext_changes, diff), cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(typedefs1[u].flags, typedefs2[v].flags, LYS_CHANGED_TYPEDEF,
                &typedef_change->changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(typedefs1[u].exts, typedefs2[v].exts,
                &typedef_change->ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(typedefs2, v) {
        if (typedef2_found[v]) {
            continue;
        }

        /* add new typedef to changes */
        LY_CHECK_RET(schema_diff_add_typedef_change(NULL, &typedefs2[v], parent2, diff, &typedef_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_TYPEDEF, LYS_CONFORM_BC,
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
 * @param[in] parent_changed Changed parent statement.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode_when_change(const struct lysp_when *when1, const struct lysp_when *when2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    enum lys_diff_conform_e conform;

    if (!when1 && !when2) {
        /* no changes */
        goto cleanup;
    } else if (!when2) {
        /* removed, NBC for YANG 1.0 */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_WHEN,
                diff->is_yang10 ? LYS_CONFORM_NBC : LYS_CONFORM_BC, changes);
    } else if (!when1) {
        /* added, detect compatibility by the extension presence */
        if (diff->is_yang10) {
            conform = LYS_CONFORM_NBC;
        } else {
            conform = schema_diff_pext_conform(diff->ctx, when2->exts, LY_STMT_WHEN, LYS_CONFORM_NBC, NULL);
        }
        return schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_WHEN, conform, changes);
    }

    /* condition, detect compatibility by the extension presence */
    if (strcmp(when1->cond, when2->cond)) {
        if (diff->is_yang10) {
            conform = LYS_CONFORM_NBC;
        } else {
            conform = schema_diff_pext_conform(diff->ctx, when2->exts, LY_STMT_WHEN, LYS_CONFORM_NBC, NULL);
        }
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_WHEN, conform,
                changes), cleanup);
    }

    /* description */
    LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, when1->dsc, when2->dsc, when2->exts, LY_STMT_DESCRIPTION,
            LYS_CHANGED_WHEN, changes), cleanup);

    /* reference */
    LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, when1->ref, when2->ref, when2->exts, LY_STMT_REFERENCE,
            LYS_CHANGED_WHEN, changes), cleanup);

    /* ext-instance */
    LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(when1->exts, when2->exts, ext_changes, diff), cleanup);

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
        return schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_DEFAULT, LYS_CONFORM_NBC, changes);
    } else if (!dflts1) {
        /* added */
        return schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_DEFAULT, LYS_CONFORM_BC, changes);
    } else if (LY_ARRAY_COUNT(dflts1) != LY_ARRAY_COUNT(dflts2)) {
        /* modified */
        return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_DEFAULT, LYS_CONFORM_NBC, changes);
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
    enum lys_diff_conform_e conform;

    flags1 &= LYS_MAND_MASK;
    flags2 &= LYS_MAND_MASK;
    conform = ((flags1 & LYS_MAND_FALSE) || (flags2 & LYS_MAND_TRUE)) ? LYS_CONFORM_NBC : LYS_CONFORM_BC;

    if (flags1 && !flags2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_MANDATORY, parent_changed, conform, changes));
    } else if (!flags1 && flags2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_MANDATORY, parent_changed, conform, changes));
    } else if (flags1 && flags2 && (flags1 != flags2)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_MANDATORY, parent_changed, conform, changes));
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a parsed 'odrered-by' statement.
 *
 * @param[in] flags1 First flags.
 * @param[in] flags2 Second flags.
 * @param[in] parent_changed Changed parent statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_ordby_change(uint16_t flags1, uint16_t flags2, enum lys_diff_changed_e parent_changed,
        struct lys_diff_changes_s *changes)
{
    flags1 &= LYS_ORDBY_MASK;
    flags2 &= LYS_ORDBY_MASK;

    if (flags1 && !flags2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_ORDERED_BY, parent_changed, LYS_CONFORM_NBC,
                changes));
    } else if (!flags1 && flags2) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_ORDERED_BY, parent_changed, LYS_CONFORM_NBC,
                changes));
    } else if (flags1 && flags2 && (flags1 != flags2)) {
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_ORDERED_BY, parent_changed,
                LYS_CONFORM_NBC, changes));
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
 * @brief Determine the conformance of adding or removing a specific refine.
 *
 * @param[in] ctx Context to use.
 * @param[in] refine Refine to use.
 * @param[in] change Refine change, either added or removed.
 * @return Conformance of the change.
 */
static enum lys_diff_conform_e
schema_diff_parsed_refine_conform(const struct ly_ctx *ctx, const struct lysp_refine *refine, enum lys_diff_change_e change)
{
    enum lys_diff_conform_e conform, c;

    conform = LYS_CONFORM_ED;

    /* description */
    if (refine->dsc) {
        c = schema_diff_pext_conform(ctx, refine->exts, LY_STMT_DESCRIPTION, LYS_CONFORM_ED, NULL);
        if (c > conform) {
            conform = c;
        }
    }

    /* reference */
    if (refine->ref) {
        if (LYS_CONFORM_BC > conform) {
            conform = LYS_CONFORM_BC;
        }
    }

    /* if-features */
    if (refine->iffeatures) {
        c = (change == LYS_CHANGE_ADDED) ? LYS_CONFORM_NBC : LYS_CONFORM_BC;
        if (c > conform) {
            conform = c;
        }
    }

    /* musts */
    if (refine->musts) {
        c = (change == LYS_CHANGE_ADDED) ? LYS_CONFORM_NBC : LYS_CONFORM_BC;
        if (c > conform) {
            conform = c;
        }
    }

    /* presence */
    if (refine->presence) {
        c = schema_diff_pext_conform(ctx, refine->exts, LY_STMT_PRESENCE, LYS_CONFORM_ED, NULL);
        if (c > conform) {
            conform = c;
        }
    }

    /* default */
    if (refine->dflts) {
        /* only setting a default value for a leaf is BC, no way to check */
        return LYS_CONFORM_NBC;
    }

    /* min, max */
    if (refine->min || refine->max) {
        /* relaxed constraint is BC, no way to check */
        c = (change == LYS_CHANGE_ADDED) ? LYS_CONFORM_NBC : LYS_CONFORM_BC;
        if (c > conform) {
            conform = c;
        }
    }

    /* config */
    if (refine->flags & LYS_CONFIG_MASK) {
        /* state -> config BC if not mandatory, no way to check */
        return LYS_CONFORM_NBC;
    }

    /* mandatory */
    if (refine->flags & LYS_MAND_MASK) {
        c = LYS_CONFORM_BC;
        if ((refine->flags & LYS_MAND_TRUE) && (change == LYS_CHANGE_ADDED)) {
            c = LYS_CONFORM_NBC;
        } else if ((refine->flags & LYS_MAND_FALSE) && (change == LYS_CHANGE_REMOVED)) {
            c = LYS_CONFORM_NBC;
        }
        if (c > conform) {
            conform = c;
        }
    }

    return conform;
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
                    schema_diff_parsed_refine_conform(diff->ctx, &refines1[u], LYS_CHANGE_REMOVED), &refine_change->changes),
                    cleanup);
            continue;
        }

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, refines1[u].dsc, refines2[v].dsc, refines2[v].exts,
                LY_STMT_DESCRIPTION, LYS_CHANGED_REFINE, &refine_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, refines1[u].ref, refines2[v].ref, refines2[v].exts,
                LY_STMT_REFERENCE, LYS_CHANGED_REFINE, &refine_change->changes), cleanup);

        /* if-features */
        LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(refines1[u].iffeatures, 0, refines2[v].iffeatures,
                LYS_CHANGED_REFINE, &refine_change->changes), cleanup);

        /* musts */
        LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(refines1[u].musts, refines2[v].musts, LYS_CHANGED_MUST,
                LYS_CHANGED_REFINE, &refine_change->changes, &refine_change->ext_changes, diff), cleanup);

        /* presence */
        if (refines1[u].presence && !refines2[v].presence) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_REFINE, LYS_CHANGED_PRESENCE,
                    LYS_CONFORM_NBC, &refine_change->changes), cleanup);
        } else if (!refines1[u].presence && refines2[v].presence) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_REFINE, LYS_CHANGED_PRESENCE,
                    LYS_CONFORM_NBC, &refine_change->changes), cleanup);
        } else if (refines1[u].presence && refines2[v].presence && strcmp(refines1[u].presence, refines2[v].presence)) {
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_REFINE, LYS_CHANGED_PRESENCE,
                    schema_diff_pext_conform(diff->ctx, refines2[v].exts, LY_STMT_PRESENCE, LYS_CONFORM_ED, NULL),
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

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(refines1[u].exts, refines2[v].exts,
                &refine_change->ext_changes, diff), cleanup);
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
                schema_diff_parsed_refine_conform(diff->ctx, &refines2[v], LYS_CHANGE_ADDED), &refine_change->changes),
                cleanup);
    }

cleanup:
    free(refine2_found);
    free(buf1);
    free(buf2);
    return rc;
}

/**
 * @brief Determine the conformance of adding or removing a specific node.
 *
 * @param[in] node Node to use.
 * @param[in] change Node change, either added or removed.
 * @return Conformance of the change.
 */
static enum lys_diff_conform_e
schema_diff_parsed_node_conform(const struct lysp_node *node, enum lys_diff_change_e change)
{
    assert((change == LYS_CHANGE_ADDED) || (change == LYS_CHANGE_REMOVED));

    /* leave schema-only node changes up to the compiled schema tree */
    if (node->nodetype == LYS_USES) {
        return LYS_CONFORM_ED;
    } else if (node->nodetype == LYS_GROUPING) {
        return LYS_CONFORM_BC;
    }

    if (change == LYS_CHANGE_ADDED) {
        if (node->flags & LYS_MAND_TRUE) {
            /* adding mandatory nodes is NBC */
            return LYS_CONFORM_NBC;
        } else {
            return LYS_CONFORM_BC;
        }
    } else {
        if (node->flags & LYS_STATUS_OBSLT) {
            /* obsolete nodes can be removed */
            return LYS_CONFORM_BC;
        } else {
            return LYS_CONFORM_NBC;
        }
    }
}

/**
 * @brief Check changes of a parsed node pair.
 *
 * @param[in] node1 First node.
 * @param[in] node2 Second node.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode_change(const struct lysp_node *node1, const struct lysp_node *node2,
        struct lys_diff_changes_s *changes, struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
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
                schema_diff_parsed_node_conform(node2, LYS_CHANGE_ADDED), changes);
    } else if (!node2) {
        /* node removed change */
        return schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_NODE,
                schema_diff_parsed_node_conform(node1, LYS_CHANGE_REMOVED), changes);
    }

    /* description */
    LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, node1->dsc, node2->dsc, node2->exts, LY_STMT_DESCRIPTION,
            LYS_CHANGED_NODE, changes), cleanup);

    /* reference */
    LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, node1->ref, node2->ref, node2->exts, LY_STMT_REFERENCE,
            LYS_CHANGED_NODE, changes), cleanup);

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
    LY_CHECK_GOTO(rc = schema_diff_pnode_when_change(when1, when2, LYS_CHANGED_NODE, changes, ext_changes, diff), cleanup);

    /* musts */
    LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(musts1, musts2, LYS_CHANGED_MUST, LYS_CHANGED_NODE, changes,
            ext_changes, diff), cleanup);

    /* default */
    LY_CHECK_GOTO(rc = schema_diff_text_nbc(dflt1, dflt2, LYS_CHANGED_NODE, LYS_CHANGED_DEFAULT, changes), cleanup);

    /* refines, separate changes */
    LY_CHECK_GOTO(rc = schema_diff_parsed_refines_change(refines1, refines2, node2, diff), cleanup);

    /* ext-instance */
    LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(node1->exts, node2->exts, ext_changes, diff), cleanup);

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
    LY_CHECK_GOTO(rc = schema_diff_typedefs_change(lysp_node_typedefs(node1), node1, lysp_node_typedefs(node2), node2,
            LYS_CHANGED_NODE, diff), cleanup);

    if (LYS_DIFF_NODE_PARSED(nodetype)) {
        /* add new node to changes */
        LY_CHECK_GOTO(rc = schema_diff_add_pnode_change(node1, node2, diff, &pnode_change), cleanup);

        /* node changes (removed, modified) */
        LY_CHECK_GOTO(rc = schema_diff_pnode_change(node1, node2, &pnode_change->changes, &pnode_change->ext_changes,
                diff), cleanup);
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_IMPORT,
                    LYS_CONFORM_BC, &import_change->changes), cleanup);
            continue;
        }

        /* revision-date */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(imps1[u].rev[0] ? imps1[u].rev : NULL,
                imps2[v].rev[0] ? imps2[v].rev : NULL, LYS_CHANGED_IMPORT, LYS_CHANGED_REVISION_DATE,
                &import_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, imps1[u].dsc, imps2[v].dsc, imps2[v].exts,
                LY_STMT_DESCRIPTION, LYS_CHANGED_IMPORT, &import_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, imps1[u].ref, imps2[v].ref, imps2[v].exts,
                LY_STMT_REFERENCE, LYS_CHANGED_IMPORT, &import_change->changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(imps1[u].exts, imps2[v].exts, &import_change->ext_changes,
                diff), cleanup);
    }

    LY_ARRAY_FOR(imps2, v) {
        if (imp2_found[v]) {
            continue;
        }

        /* add new import to changes */
        LY_CHECK_RET(schema_diff_add_import_change(NULL, &imps2[v], diff, &import_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_IMPORT, LYS_CONFORM_BC,
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_INCLUDE,
                    LYS_CONFORM_BC, &include_change->changes), cleanup);
            continue;
        }

        /* revision-date */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(incs1[u].rev[0] ? incs1[u].rev : NULL,
                incs2[v].rev[0] ? incs2[v].rev : NULL, LYS_CHANGED_INCLUDE, LYS_CHANGED_REVISION_DATE,
                &include_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, incs1[u].dsc, incs2[v].dsc, incs2[v].exts,
                LY_STMT_DESCRIPTION, LYS_CHANGED_INCLUDE, &include_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, incs1[u].ref, incs2[v].ref, incs2[v].exts,
                LY_STMT_REFERENCE, LYS_CHANGED_INCLUDE, &include_change->changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(incs1[u].exts, incs2[v].exts, &include_change->ext_changes,
                diff), cleanup);
    }

    LY_ARRAY_FOR(incs2, v) {
        if (incs2[v].injected || inc2_found[v]) {
            continue;
        }

        /* add new include to changes */
        LY_CHECK_RET(schema_diff_add_include_change(NULL, &incs2[v], diff, &include_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_INCLUDE,
                LYS_CONFORM_BC, &include_change->changes), cleanup);
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_EXTENSION,
                    LYS_CONFORM_NBC, &extension_change->changes), cleanup);
            continue;
        }

        /* argument */
        LY_CHECK_GOTO(rc = schema_diff_text_nbc(extensions1[u].argname, extensions2[v].argname, LYS_CHANGED_EXTENSION,
                LYS_CHANGED_UNITS, &extension_change->changes), cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(extensions1[u].flags, extensions2[v].flags, LYS_CHANGED_EXTENSION,
                &extension_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, extensions1[u].dsc, extensions2[v].dsc,
                extensions2[v].exts, LY_STMT_DESCRIPTION, LYS_CHANGED_EXTENSION, &extension_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, extensions1[u].ref, extensions2[v].ref,
                extensions2[v].exts, LY_STMT_REFERENCE, LYS_CHANGED_EXTENSION, &extension_change->changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(extensions1[u].exts, extensions2[v].exts,
                &extension_change->ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(extensions2, v) {
        if (extension2_found[v]) {
            continue;
        }

        /* add new extension to changes */
        LY_CHECK_RET(schema_diff_add_extension_change(NULL, &extensions2[v], diff, &extension_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_EXTENSION,
                LYS_CONFORM_BC, &extension_change->changes), cleanup);
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_FEATURE,
                    LYS_CONFORM_NBC, &feat_change->changes), cleanup);
            continue;
        }

        /* if-features */
        LY_CHECK_GOTO(rc = schema_diff_iffeatures_change(features1[u].iffeatures, 0, features2[v].iffeatures,
                LYS_CHANGED_FEATURE, &feat_change->changes), cleanup);

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status_change(features1[u].flags, features2[v].flags, LYS_CHANGED_FEATURE,
                &feat_change->changes), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, features1[u].dsc, features2[v].dsc, features2[v].exts,
                LY_STMT_DESCRIPTION, LYS_CHANGED_FEATURE, &feat_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, features1[u].ref, features2[v].ref, features2[v].exts,
                LY_STMT_REFERENCE, LYS_CHANGED_FEATURE, &feat_change->changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(features1[u].exts, features2[v].exts,
                &feat_change->ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(features2, v) {
        if (feature2_found[v]) {
            continue;
        }

        /* add new feature to changes */
        LY_CHECK_RET(schema_diff_add_feat_change(NULL, &features2[v], diff, &feat_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_FEATURE,
                LYS_CONFORM_BC, &feat_change->changes), cleanup);
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
        return schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_UNIQUE, LYS_CONFORM_NBC, changes);
    } else if (!uniqs1) {
        /* added */
        return schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_UNIQUE, LYS_CONFORM_NBC, changes);
    } else if (LY_ARRAY_COUNT(uniqs1) != LY_ARRAY_COUNT(uniqs2)) {
        /* modified */
        return schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed, LYS_CHANGED_UNIQUE, LYS_CONFORM_NBC, changes);
    }

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a deviate pair.
 *
 * @param[in] dev1 First deviate.
 * @param[in] dev2 Second deviate.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_deviate_change(const struct lysp_deviate *dev1, const struct lysp_deviate *dev2,
        struct lys_diff_changes_s *changes, struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
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
                changes, ext_changes, diff));
    } else if (dev1->mod == LYS_DEV_DELETE) {
        LY_CHECK_RET(schema_diff_parsed_restrs_change(add2->musts, add1->musts, LYS_CHANGED_MUST, LYS_CHANGED_DEVIATE,
                changes, ext_changes, diff));
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
        LY_CHECK_RET(schema_diff_ptype_change(rpl1->type, NULL, rpl2->type, NULL, LYS_CHANGED_DEVIATE, changes,
                ext_changes, diff));
    }

    /* ext-instance */
    LY_CHECK_RET(schema_diff_pext_insts_change(dev1->exts, dev2->exts, ext_changes, diff));

    return LY_SUCCESS;
}

/**
 * @brief Check changes of a parsed 'deviate' array.
 *
 * @param[in] deviates1 First deviate array.
 * @param[in] deviates2 Second deviate array.
 * @param[in,out] changes Changes to add to.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_deviates_change(const struct lysp_deviate *deviates1, const struct lysp_deviate *deviates2,
        struct lys_diff_changes_s *changes, struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_DEVIATION, LYS_CHANGED_DEVIATE,
                    LYS_CONFORM_NBC, changes), cleanup);
            continue;
        }

        /* deviate */
        LY_CHECK_GOTO(rc = schema_diff_deviate_change(iter1, iter2, changes, ext_changes, diff), cleanup);
    }

    for (iter2 = deviates2, v = 0; iter2; iter2 = iter2->next, ++v) {
        if (deviate2_found[v]) {
            continue;
        }

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_DEVIATION, LYS_CHANGED_DEVIATE,
                LYS_CONFORM_NBC, changes), cleanup);
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
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_DEVIATION,
                    LYS_CONFORM_NBC, &dev_change->changes), cleanup);
            continue;
        }

        /* deviate */
        LY_CHECK_GOTO(rc = schema_diff_deviates_change(deviations1[u].deviates, deviations2[v].deviates,
                &dev_change->changes, &dev_change->ext_changes, diff), cleanup);

        /* description */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, deviations1[u].dsc, deviations2[v].dsc, deviations2[v].exts,
                LY_STMT_DESCRIPTION, LYS_CHANGED_DEVIATION, &dev_change->changes), cleanup);

        /* reference */
        LY_CHECK_GOTO(rc = schema_diff_pnode_text(diff->ctx, deviations1[u].ref, deviations2[v].ref, deviations2[v].exts,
                LY_STMT_REFERENCE, LYS_CHANGED_DEVIATION, &dev_change->changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(deviations1[u].exts, deviations2[v].exts,
                &dev_change->ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(deviations2, v) {
        if (deviation2_found[v]) {
            continue;
        }

        /* add new deviation to changes */
        LY_CHECK_RET(schema_diff_add_dev_change(NULL, &deviations2[v], diff, &dev_change));

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_DEVIATION,
                LYS_CONFORM_NBC, &dev_change->changes), cleanup);
    }

cleanup:
    free(deviation2_found);
    return rc;
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
schema_diff_pext_inst_substmts_change(const struct lysp_ext_substmt *substmts1, const struct lysp_ext_substmt *substmts2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysp_ext_substmt **substmts2_array = NULL;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found, siblings_checked = 0;

    /* collect all the compiled substatements2 to remove from */
    substmts2_array = calloc(LY_ARRAY_COUNT(substmts2), sizeof *substmts2_array);
    LY_CHECK_ERR_GOTO(!substmts2_array, LOGMEM(NULL); rc = LY_EMEM, cleanup);
    LY_ARRAY_FOR(substmts2, v) {
        if (substmts2[v].storage_p) {
            substmts2_array[v] = &substmts2[v];
        }
    }

    LY_ARRAY_FOR(substmts1, u) {
        if (!substmts1[u].storage_p) {
            /* not parsed */
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
            /* parsed statement not found in substmts2 */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed,
                    schema_diff_stmt2changed(substmts1[u].stmt), LYS_CONFORM_NBC, changes), cleanup);
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

            LY_CHECK_GOTO(rc = schema_diff_pnodes_change_r(*(substmts1[u].storage_p), *(substmts2[v].storage_p), diff),
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
            /* text ED */
            LY_CHECK_GOTO(rc = schema_diff_text_ed(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
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
            LY_CHECK_GOTO(rc = schema_diff_ptype_enums_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_CONFIG:
            /* config flag */
            LY_CHECK_GOTO(rc = schema_diff_config_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_FRACTION_DIGITS:
        case LY_STMT_REQUIRE_INSTANCE:
            /* uint8 number */
            if (*(uint8_t *)substmts1[u].storage_p != *(uint8_t *)substmts2[v].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_EXT_INST,
                        schema_diff_stmt2changed(substmts1[u].stmt), LYS_CONFORM_NBC, changes), cleanup);
            }
            break;
        case LY_STMT_IDENTITY:
            /* identity */
            LY_CHECK_GOTO(rc = schema_diff_module_identities_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_EXT_INST, diff), cleanup);
            break;
        case LY_STMT_LENGTH:
            /* length */
            LY_CHECK_GOTO(rc = schema_diff_parsed_restr_change(*(substmts1[u].storage_p), NULL, *(substmts2[v].storage_p),
                    NULL, LYS_CHANGED_LENGTH, LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_RANGE:
            /* range */
            LY_CHECK_GOTO(rc = schema_diff_parsed_restr_change(*(substmts1[u].storage_p), NULL, *(substmts2[v].storage_p),
                    NULL, LYS_CHANGED_RANGE, LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_MANDATORY:
            /* mandatory */
            LY_CHECK_GOTO(rc = schema_diff_parsed_mandatory_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_ORDERED_BY:
            /* odrered-by */
            LY_CHECK_GOTO(rc = schema_diff_parsed_ordby_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_MAX_ELEMENTS:
        case LY_STMT_MIN_ELEMENTS:
            /* uint32 number */
            if (*(uint32_t *)substmts1[u].storage_p != *(uint32_t *)substmts2[v].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_EXT_INST,
                        schema_diff_stmt2changed(substmts1[u].stmt), LYS_CONFORM_NBC, changes), cleanup);
            }
            break;
        case LY_STMT_MUST:
            /* must array */
            LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_MUST, LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_PATTERN:
            /* pattern array of arrays */
            LY_CHECK_GOTO(rc = schema_diff_parsed_restrs_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
                    LYS_CHANGED_PATTERN, LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_POSITION:
        case LY_STMT_VALUE:
            /* uint64/int64 number */
            if (*(uint64_t *)substmts1[u].storage_p != *(uint64_t *)substmts2[v].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_EXT_INST,
                        schema_diff_stmt2changed(substmts1[u].stmt), LYS_CONFORM_NBC, changes), cleanup);
            }
            break;
        case LY_STMT_STATUS:
            /* status flag */
            LY_CHECK_GOTO(rc = schema_diff_status_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_TYPE:
            /* type */
            LY_CHECK_GOTO(rc = schema_diff_ptype_change(*(substmts1[u].storage_p), NULL, *(substmts2[v].storage_p), NULL,
                    LYS_CHANGED_EXT_INST, changes, ext_changes, diff), cleanup);
            break;
        case LY_STMT_WHEN:
            /* when */
            LY_CHECK_GOTO(rc = schema_diff_pnode_when_change(*(substmts1[u].storage_p), *(substmts2[v].storage_p),
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
        case LY_STMT_EXTENSION_INSTANCE:
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
                schema_diff_stmt2changed(substmts2_array[v]->stmt), LYS_CONFORM_NBC, changes), cleanup);
    }

cleanup:
    free(substmts2_array);
    return rc;
}

/**
 * @brief Check whether 2 parse ext instances can be identified as matching.
 *
 * @param[in] ctx Context to use.
 * @param[in] ext1 First parsed ext instance.
 * @param[in] ext2 Second parsed ext instance.
 * @return Non-zero if the ext instances match;
 * @return 0 if the ext instances do not match.
 */
static ly_bool
schema_diff_pext_inst_match(const struct ly_ctx *ctx, const struct lysp_ext_instance *ext1,
        const struct lysp_ext_instance *ext2)
{
    const char *mod1, *mod2, *name1, *name2;

    /* get the modules and names */
    lysp_nodeid_find_module(ctx, ext1->name, ext1->format, ext1->prefix_data, &mod1, &name1);
    lysp_nodeid_find_module(ctx, ext2->name, ext2->format, ext2->prefix_data, &mod2, &name2);

    /* module and name */
    if (strcmp(mod1, mod2) || strcmp(name1, name2)) {
        return 0;
    }

    /* argument */
    if ((ext1->argument && !ext2->argument) || (!ext1->argument && ext2->argument) ||
            (ext1->argument && ext2->argument && strcmp(ext1->argument, ext2->argument))) {
        return 0;
    }

    return 1;
}

/**
 * @brief Check changes of parsed extension-instance arrays.
 *
 * @param[in] exts1 First ext-inst array.
 * @param[in] exts2 Second ext-inst array.
 * @param[in,out] ext_changes Ext-instance changes to add to.
 * @param[in,out] diff Diff to use.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pext_insts_change(const struct lysp_ext_instance *exts1, const struct lysp_ext_instance *exts2,
        struct lys_diff_pext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool *exts2_found, found, conform_ext;
    enum lys_diff_changed_e parent_changed;
    enum lys_diff_conform_e conform;
    struct lys_diff_pext_change_s *ext_change;
    LY_ARRAY_COUNT_TYPE u, v;
    uint32_t i;

    /* prepare array for marking found exts */
    exts2_found = calloc(LY_ARRAY_COUNT(exts2), sizeof *exts2_found);
    LY_CHECK_ERR_GOTO(!exts2_found, LOGMEM(NULL); rc = LY_SUCCESS, cleanup);

    LY_ARRAY_FOR(exts1, u) {
        schema_diff_is_pext_conform(diff->ctx, &exts1[u], 0, &conform_ext);
        if (conform_ext) {
            /* skip */
            continue;
        }

        found = 0;
        LY_ARRAY_FOR(exts2, v) {
            schema_diff_is_pext_conform(diff->ctx, &exts2[v], 0, &conform_ext);
            if (conform_ext) {
                /* skip */
                continue;
            }

            if (schema_diff_pext_inst_match(diff->ctx, &exts1[u], &exts2[v])) {
                found = 1;
                exts2_found[v] = 1;
                break;
            }
        }

        /* add new ext-instance to changes */
        LY_CHECK_RET(schema_diff_add_pext_change(&exts1[u], found ? &exts2[v] : NULL, ext_changes, &ext_change));
        parent_changed = schema_diff_stmt2changed(exts1[u].parent_stmt);

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_EXT_INST,
                    LYS_CONFORM_BC, &ext_change->changes), cleanup);
            continue;
        }

        /* substatements */
        LY_CHECK_GOTO(rc = schema_diff_pext_inst_substmts_change(exts1[u].substmts, exts2[v].substmts, parent_changed,
                &ext_change->changes, ext_changes, diff), cleanup);

        /* children not supported */

        /* learn conformance */
        conform = schema_diff_pext_conform(diff->ctx, exts1[u].exts, 0, LYS_CONFORM_BC, &conform_ext);

        if (conform_ext) {
            /* overwrite based on the extension */
            for (i = 0; i < ext_change->changes.count; ++i) {
                ext_change->changes.changes[i].conform = conform;
            }
        }

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_pext_insts_change(exts1[u].exts, exts2[v].exts, ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(exts2, v) {
        schema_diff_is_pext_conform(diff->ctx, &exts2[v], 0, &conform_ext);
        if (conform_ext || exts2_found[v]) {
            continue;
        }

        /* add new ext-instance to changes */
        LY_CHECK_RET(schema_diff_add_pext_change(NULL, &exts2[v], ext_changes, &ext_change));
        parent_changed = schema_diff_stmt2changed(exts2[v].parent_stmt);

        /* learn conformance */
        conform = schema_diff_pext_conform(diff->ctx, exts2[v].exts, 0, LYS_CONFORM_BC, NULL);

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_EXT_INST, conform,
                &ext_change->changes), cleanup);
    }

cleanup:
    free(exts2_found);
    return rc;
}

LY_ERR
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
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_NONE, LYS_CHANGED_PREFIX,
                LYS_CONFORM_ED, &diff->module_changes), cleanup);
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
    LY_CHECK_GOTO(rc = schema_diff_typedefs_change(mod1->typedefs, NULL, mod2->typedefs, NULL, LYS_CHANGED_NONE,
            diff), cleanup);

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
