/**
 * @file schema_diff.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema diff functionss
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

void
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

LY_ERR
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

enum lys_diff_changed_e
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
    case LY_STMT_AUGMENT:
    case LY_STMT_CASE:
    case LY_STMT_CHOICE:
    case LY_STMT_GROUPING:
    case LY_STMT_INPUT:
    case LY_STMT_OUTPUT:
    case LY_STMT_USES:
        return LYS_CHANGED_NODE;
    case LY_STMT_ARGUMENT:
    case LY_STMT_ARG_TEXT:
    case LY_STMT_ARG_VALUE:
    case LY_STMT_SYNTAX_LEFT_BRACE:
    case LY_STMT_SYNTAX_RIGHT_BRACE:
    case LY_STMT_SYNTAX_SEMICOLON:
    case LY_STMT_YIN_ELEMENT:
        /* invalid */
        LOGINT(NULL);
        break;
    case LY_STMT_BASE:
        return LYS_CHANGED_BASE;
    case LY_STMT_BELONGS_TO:
        return LYS_CHANGED_BELONGS_TO;
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
    case LY_STMT_DEVIATE:
        return LYS_CHANGED_DEVIATE;
    case LY_STMT_DEVIATION:
        return LYS_CHANGED_DEVIATION;
    case LY_STMT_ENUM:
        return LYS_CHANGED_ENUM;
    case LY_STMT_ERROR_APP_TAG:
        return LYS_CHANGED_ERR_APP_TAG;
    case LY_STMT_ERROR_MESSAGE:
        return LYS_CHANGED_ERR_MSG;
    case LY_STMT_EXTENSION:
        return LYS_CHANGED_EXTENSION;
    case LY_STMT_EXTENSION_INSTANCE:
        return LYS_CHANGED_EXT_INST;
    case LY_STMT_FEATURE:
        return LYS_CHANGED_FEATURE;
    case LY_STMT_FRACTION_DIGITS:
        return LYS_CHANGED_FRAC_DIG;
    case LY_STMT_IDENTITY:
        return LYS_CHANGED_IDENT;
    case LY_STMT_IF_FEATURE:
        return LYS_CHANGED_IF_FEATURE;
    case LY_STMT_IMPORT:
        return LYS_CHANGED_IMPORT;
    case LY_STMT_INCLUDE:
        return LYS_CHANGED_INCLUDE;
    case LY_STMT_KEY:
        return LYS_CHANGED_KEY;
    case LY_STMT_LENGTH:
        return LYS_CHANGED_LENGTH;
    case LY_STMT_MANDATORY:
        return LYS_CHANGED_MANDATORY;
    case LY_STMT_MAX_ELEMENTS:
        return LYS_CHANGED_MAX_ELEM;
    case LY_STMT_MIN_ELEMENTS:
        return LYS_CHANGED_MIN_ELEM;
    case LY_STMT_MODIFIER:
        return LYS_CHANGED_MODIFIER;
    case LY_STMT_MODULE:
        return LYS_CHANGED_MODULE;
    case LY_STMT_MUST:
        return LYS_CHANGED_MUST;
    case LY_STMT_NAMESPACE:
        return LYS_CHANGED_NAMESPACE;
    case LY_STMT_ORDERED_BY:
        return LYS_CHANGED_ORDERED_BY;
    case LY_STMT_ORGANIZATION:
        return LYS_CHANGED_ORGANIZATION;
    case LY_STMT_PATH:
        return LYS_CHANGED_PATH;
    case LY_STMT_PATTERN:
        return LYS_CHANGED_PATTERN;
    case LY_STMT_POSITION:
        return LYS_CHANGED_POSITION;
    case LY_STMT_PREFIX:
        return LYS_CHANGED_PREFIX;
    case LY_STMT_PRESENCE:
        return LYS_CHANGED_PRESENCE;
    case LY_STMT_RANGE:
        return LYS_CHANGED_RANGE;
    case LY_STMT_REFERENCE:
        return LYS_CHANGED_REFERENCE;
    case LY_STMT_REFINE:
        return LYS_CHANGED_REFINE;
    case LY_STMT_REQUIRE_INSTANCE:
        return LYS_CHANGED_REQ_INSTANCE;
    case LY_STMT_REVISION:
        return LYS_CHANGED_REVISION;
    case LY_STMT_REVISION_DATE:
        return LYS_CHANGED_REVISION_DATE;
    case LY_STMT_STATUS:
        return LYS_CHANGED_STATUS;
    case LY_STMT_SUBMODULE:
        return LYS_CHANGED_SUBMODULE;
    case LY_STMT_TYPE:
        return LYS_CHANGED_TYPE;
    case LY_STMT_TYPEDEF:
        return LYS_CHANGED_TYPEDEF;
    case LY_STMT_UNIQUE:
        return LYS_CHANGED_UNIQUE;
    case LY_STMT_UNITS:
        return LYS_CHANGED_UNITS;
    case LY_STMT_VALUE:
        return LYS_CHANGED_VALUE;
    case LY_STMT_WHEN:
        return LYS_CHANGED_WHEN;
    case LY_STMT_YANG_VERSION:
        return LYS_CHANGED_YANG_VERSION;
    }

    return LYS_CHANGED_NONE;
}

void
schema_diff_find_module(const struct ly_ctx *ctx, const char *nodeid, LY_VALUE_FORMAT format, void *prefix_data,
        const char **mod_name, const char **name)
{
    const struct lys_module *mod;
    const char *prefix, *nam;
    uint32_t prefix_len, nam_len;

    /* parse the prefix */
    ly_parse_nodeid(&nodeid, &prefix, &prefix_len, &nam, &nam_len);
    assert(nam[nam_len] == '\0');

    /* find the module */
    mod = lys_find_module(ctx, NULL, prefix, prefix_len, format, prefix_data);
    assert(mod);

    *mod_name = mod->name;
    *name = nam;
}

/**
 * @brief Check changes of a 'yang-version'.
 *
 * @param[in] yvsn1 First yang-version.
 * @param[in] yvsn2 Second yang-version.
 * @param[in] parent_changed Parent statement of the change.
 * @param[in] changed Changed statement.
 * @param[in,out] changes Changes to add to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_yangversion_change(uint8_t yvsn1, uint8_t yvsn2,
        enum lys_diff_changed_e parent_changed, enum lys_diff_changed_e changed,
        struct lys_diff_changes_s *changes)
{
    if (yvsn1 != yvsn2) {
        /* modified */
        LY_CHECK_RET(schema_diff_add_change(LYS_CHANGE_MODIFIED, parent_changed,
                     changed, 1, changes));
    }

    return LY_SUCCESS;
}

LY_ERR
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

LY_ERR
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

LY_ERR
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

LY_ERR
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

LY_ERR
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

LY_ERR
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

LY_ERR
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

LY_ERR
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
        LY_CHECK_GOTO(rc = schema_diff_ext_insts_change(idents1[u].exts, idents2[v].exts, &ident_change->ext_changes,
                diff), cleanup);

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
    /* yang-version */
    LY_CHECK_RET(schema_diff_yangversion_change(mod1->version, mod2->version,
            LYS_CHANGED_NONE, LYS_CHANGED_YANG_VERSION, &diff->module_changes));

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
    LY_CHECK_RET(schema_diff_ext_insts_change(mod1->compiled->exts, mod2->compiled->exts, &diff->mod_ext_changes, diff));

    return LY_SUCCESS;
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
        free(ext_changes->changes[i].changes.changes);
    }
    free(ext_changes->changes);
}

/**
 * @brief Erase parsed ext-instance changes array.
 *
 * @param[in] ext_changes Ext-instance changes structure to erase.
 */
static void
lysc_diff_erase_pext_changes(struct lys_diff_pext_changes_s *ext_changes)
{
    uint32_t i;

    for (i = 0; i < ext_changes->count; ++i) {
        free(ext_changes->changes[i].changes.changes);
    }
    free(ext_changes->changes);
}

void
lysc_diff_erase(struct lys_diff_s *diff)
{
    uint32_t i;

    /* module */
    free(diff->module_changes.changes);
    lysc_diff_erase_ext_changes(&diff->mod_ext_changes);

    for (i = 0; i < diff->ident_change_count; ++i) {
        free(diff->ident_changes[i].changes.changes);
        lysc_diff_erase_ext_changes(&diff->ident_changes[i].ext_changes);
    }
    free(diff->ident_changes);

    /* parsed module */
    for (i = 0; i < diff->import_change_count; ++i) {
        free(diff->import_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->import_changes[i].ext_changes);
    }
    free(diff->import_changes);

    for (i = 0; i < diff->include_change_count; ++i) {
        free(diff->include_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->include_changes[i].ext_changes);
    }
    free(diff->include_changes);

    for (i = 0; i < diff->extension_change_count; ++i) {
        free(diff->extension_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->extension_changes[i].ext_changes);
    }
    free(diff->extension_changes);

    for (i = 0; i < diff->feat_change_count; ++i) {
        free(diff->feat_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->feat_changes[i].ext_changes);
    }
    free(diff->feat_changes);

    for (i = 0; i < diff->dev_change_count; ++i) {
        free(diff->dev_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->dev_changes[i].ext_changes);
    }
    free(diff->dev_changes);

    /* parsed node */
    for (i = 0; i < diff->pnode_change_count; ++i) {
        free(diff->pnode_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->pnode_changes[i].ext_changes);
    }
    free(diff->pnode_changes);

    for (i = 0; i < diff->refine_change_count; ++i) {
        free(diff->refine_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->refine_changes[i].ext_changes);
    }
    free(diff->refine_changes);

    for (i = 0; i < diff->typedef_change_count; ++i) {
        free(diff->typedef_changes[i].changes.changes);
        lysc_diff_erase_pext_changes(&diff->typedef_changes[i].ext_changes);
    }
    free(diff->typedef_changes);

    /* compiled node */
    for (i = 0; i < diff->node_change_count; ++i) {
        free(diff->node_changes[i].changes.changes);
        lysc_diff_erase_ext_changes(&diff->node_changes[i].ext_changes);
    }
    free(diff->node_changes);
}
