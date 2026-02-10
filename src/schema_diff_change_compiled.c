/**
 * @file schema_diff_change_compiled.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema diff change compiled functionss
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

static LY_ERR schema_diff_node_type_change(const struct lysc_type *type1, const struct lysc_type *type2,
        enum lys_diff_changed_e parent_changed, struct lys_diff_changes_s *changes,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff);

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
    ec->changes.changes = NULL;
    ec->changes.count = 0;

    *ext_change = ec;
    return LY_SUCCESS;
}

/**
 * @brief Check whether an ext-instance is 'backwards-compatible'.
 *
 * @param[in] ext Ext instance to check.
 * @return 1 if it is;
 * @return 0 otherwise.
 */
static ly_bool
schema_diff_is_bc_ext(const struct lysc_ext_instance *ext)
{
    if (!strcmp(ext->def->module->name, "ietf-yang-schema-comparison") &&
            !strcmp(ext->def->name, "backwards-compatible")) {
        return 1;
    }

    return 0;
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
        if (schema_diff_is_bc_ext(&exts[u])) {
            return 1;
        }
    }

    return 0;
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
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(musts1[u].exts, musts2[v].exts, ext_changes, diff), cleanup);
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
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(whens1[u]->exts, whens2[v]->exts, ext_changes, diff), cleanup);
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
    LY_CHECK_RET(schema_diff_ext_insts_change(range1->exts, range2->exts, ext_changes, diff));

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
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->dsc, patterns2[v]->dsc, LYS_CHANGED_PATTERN,
                LYS_CHANGED_DESCRIPTION, changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->ref, patterns2[v]->ref, LYS_CHANGED_PATTERN,
                LYS_CHANGED_REFERENCE, changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->emsg, patterns2[v]->emsg, LYS_CHANGED_PATTERN,
                LYS_CHANGED_ERR_MSG, changes), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_text_bc(patterns1[u]->eapptag, patterns2[v]->eapptag, LYS_CHANGED_PATTERN,
                LYS_CHANGED_ERR_APP_TAG, changes), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(patterns1[u]->exts, patterns2[v]->exts, ext_changes, diff), cleanup);

        /* inverted */
        if (patterns1[u]->inverted != patterns2[v]->inverted) {
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
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(bitenums1[u].exts, bitenums2[v].exts, ext_changes, diff), cleanup);

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
    LY_ARRAY_COUNT_TYPE u;

    /* order matters */
    LY_ARRAY_FOR(types1, u) {
        if (u >= LY_ARRAY_COUNT(types2)) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, LYS_CHANGED_NONE, LYS_CHANGED_TYPE, 1,
                    changes), cleanup);
            continue;
        }

        /* type */
        LY_CHECK_GOTO(rc = schema_diff_node_type_change(types1[u], types2[u], LYS_CHANGED_TYPE, changes, ext_changes,
                diff), cleanup);
    }

    for ( ; u < LY_ARRAY_COUNT(types2); ++u) {
        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, LYS_CHANGED_NONE, LYS_CHANGED_TYPE, 0,
                changes), cleanup);
    }

cleanup:
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
    LY_CHECK_RET(schema_diff_ext_insts_change(type1->exts, type2->exts, ext_changes, diff));

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
    LY_CHECK_RET(schema_diff_ext_insts_change(node1->exts, node2->exts, ext_changes, diff));

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
    LY_ARRAY_FOR(substmts2, v) {
        if (substmts2[v].storage_p) {
            substmts2_array[v] = &substmts2[v];
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
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_FRACTION_DIGITS:
        case LY_STMT_REQUIRE_INSTANCE:
            /* uint8 number */
            if (*(uint8_t *)substmts1[u].storage_p != *(uint8_t *)substmts2[v].storage_p) {
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
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_ORDERED_BY:
            /* odrered-by */
            LY_CHECK_GOTO(rc = schema_diff_node_ordby_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
            break;
        case LY_STMT_MAX_ELEMENTS:
        case LY_STMT_MIN_ELEMENTS:
            /* uint32 number */
            if (*(uint32_t *)substmts1[u].storage_p != *(uint32_t *)substmts2[v].storage_p) {
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
            if (*(uint64_t *)substmts1[u].storage_p != *(uint64_t *)substmts2[v].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_MODIFIED, LYS_CHANGED_EXT_INST,
                        schema_diff_stmt2changed(substmts1[u].stmt), 1, changes), cleanup);
            }
            break;
        case LY_STMT_STATUS:
            /* status flag */
            LY_CHECK_GOTO(rc = schema_diff_status_change(*(uint16_t *)substmts1[u].storage_p,
                    *(uint16_t *)substmts2[v].storage_p, LYS_CHANGED_EXT_INST, changes), cleanup);
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
                schema_diff_stmt2changed(substmts2_array[v]->stmt), 1, changes), cleanup);
    }

cleanup:
    free(substmts2_array);
    return rc;
}

LY_ERR
schema_diff_ext_insts_change(const struct lysc_ext_instance *exts1, const struct lysc_ext_instance *exts2,
        struct lys_diff_ext_changes_s *ext_changes, struct lys_diff_s *diff)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool *exts2_found, found, is_nbc;
    enum lys_diff_changed_e parent_changed;
    struct lys_diff_ext_change_s *ext_change;
    LY_ARRAY_COUNT_TYPE u, v;
    uint32_t i;

    /* prepare array for marking found exts */
    exts2_found = calloc(LY_ARRAY_COUNT(exts2), sizeof *exts2_found);
    LY_CHECK_ERR_GOTO(!exts2_found, LOGMEM(NULL); rc = LY_SUCCESS, cleanup);

    LY_ARRAY_FOR(exts1, u) {
        if (schema_diff_is_bc_ext(&exts1[u])) {
            /* skip */
            continue;
        }

        found = 0;
        LY_ARRAY_FOR(exts2, v) {
            if (schema_diff_is_bc_ext(&exts2[v])) {
                /* skip */
                continue;
            }

            if (!strcmp(exts1[u].def->name, exts2[v].def->name) && !strcmp(exts1[u].argument, exts2[v].argument)) {
                found = 1;
                exts2_found[v] = 1;
                break;
            }
        }

        /* add new ext-instance to changes */
        LY_CHECK_RET(schema_diff_add_ext_change(&exts1[u], found ? &exts2[v] : NULL, ext_changes, &ext_change));
        parent_changed = schema_diff_stmt2changed(exts1[u].parent_stmt);

        /* learn BC/NBC */
        is_nbc = schema_diff_has_bc_ext(exts1[u].exts) ? 0 : 1;

        if (!found) {
            /* removed */
            LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_REMOVED, parent_changed, LYS_CHANGED_EXT_INST, is_nbc,
                    &ext_change->changes), cleanup);

            if (is_nbc) {
                /* NBC */
                diff->is_nbc = 1;
            }
            continue;
        }

        /* substatements */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_change(exts1[u].substmts, exts2[v].substmts, parent_changed,
                &ext_change->changes, ext_changes, diff), cleanup);

        if (!is_nbc) {
            /* no changes are NBC */
            for (i = 0; i < ext_change->changes.count; ++i) {
                ext_change->changes.changes[i].is_nbc = 0;
            }
        } else {
            /* check whether any of the changes were NBC */
            schema_diff_check_node_change_nbc(&ext_change->changes, diff);
        }

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(exts1[u].exts, exts2[v].exts, ext_changes, diff), cleanup);
    }

    LY_ARRAY_FOR(exts2, v) {
        if (schema_diff_is_bc_ext(&exts2[v]) || exts2_found[v]) {
            continue;
        }

        /* add new ext-instance to changes */
        LY_CHECK_RET(schema_diff_add_ext_change(NULL, &exts2[v], ext_changes, &ext_change));
        parent_changed = schema_diff_stmt2changed(exts2[v].parent_stmt);

        /* learn BC/NBC */
        is_nbc = schema_diff_has_bc_ext(exts2[v].exts) ? 0 : 1;

        /* added */
        LY_CHECK_GOTO(rc = schema_diff_add_change(LYS_CHANGE_ADDED, parent_changed, LYS_CHANGED_EXT_INST, is_nbc,
                &ext_change->changes), cleanup);
    }

cleanup:
    free(exts2_found);
    return rc;
}

LY_ERR
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
