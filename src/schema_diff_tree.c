/**
 * @file schema_diff_tree.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema diff tree functions
 *
 * Copyright (c) 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf */

#include "schema_diff.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "ly_common.h"
#include "tree_schema_internal.h"

static LY_ERR schema_diff_ext_inst_substmts(const struct ly_ctx *ctx, const struct lysc_ext_substmt *substmts,
        struct lyd_node **child_p);
static LY_ERR schema_diff_ext_inst_substmts_ext_insts(const struct ly_ctx *ctx, const struct lysc_ext_instance *exts,
        struct lyd_node **child_p);
static LY_ERR schema_diff_ext_inst(const struct lysc_ext_instance *ext, struct lyd_node *change_cont);

/**
 * @brief Get string from a changed statement.
 *
 * @param[in] ch Changed statement.
 * @return String form of the changed statement.
 */
static const char *
schema_diff_changed2str(enum lysc_diff_changed_e ch)
{
    switch (ch) {
    case LYSC_CHANGED_NONE:
        break;
    case LYSC_CHANGED_BASE:
        return "base";
    case LYSC_CHANGED_BIT:
        return "bit";
    case LYSC_CHANGED_CONFIG:
        return "config";
    case LYSC_CHANGED_CONTACT:
        return "contact";
    case LYSC_CHANGED_DEFAULT:
        return "default";
    case LYSC_CHANGED_DESCRIPTION:
        return "description";
    case LYSC_CHANGED_ENUM:
        return "enum";
    case LYSC_CHANGED_ERR_APP_TAG:
        return "error-app-tag";
    case LYSC_CHANGED_ERR_MSG:
        return "error-message";
    case LYSC_CHANGED_EXT_INST:
        return "extension-instance";
    case LYSC_CHANGED_FRAC_DIG:
        return "fraction-digits";
    case LYSC_CHANGED_IDENT:
        return "identity";
    case LYSC_CHANGED_LENGTH:
        return "length";
    case LYSC_CHANGED_MANDATORY:
        return "mandatory";
    case LYSC_CHANGED_MAX_ELEM:
        return "max-elements";
    case LYSC_CHANGED_MIN_ELEM:
        return "min-elements";
    case LYSC_CHANGED_MUST:
        return "must";
    case LYSC_CHANGED_NODE:
        return "node";
    case LYSC_CHANGED_ORDERED_BY:
        return "ordered-by";
    case LYSC_CHANGED_ORGANIZATION:
        return "organization";
    case LYSC_CHANGED_PATH:
        return "path";
    case LYSC_CHANGED_PATTERN:
        return "pattern";
    case LYSC_CHANGED_PRESENCE:
        return "presence";
    case LYSC_CHANGED_RANGE:
        return "range";
    case LYSC_CHANGED_REFERENCE:
        return "reference";
    case LYSC_CHANGED_REQ_INSTANCE:
        return "require-instance";
    case LYSC_CHANGED_STATUS:
        return "status";
    case LYSC_CHANGED_TYPE:
        return "type";
    case LYSC_CHANGED_UNITS:
        return "units";
    case LYSC_CHANGED_UNIQUE:
        return "unique";
    case LYSC_CHANGED_WHEN:
        return "when";
    }

    return NULL;
}

/**
 * @brief Get string from a change type.
 *
 * @param[in] ch Change type.
 * @return String form of the change type.
 */
static const char *
schema_diff_change2str(enum lysc_diff_change_e ch)
{
    switch (ch) {
    case LYSC_CHANGE_MODIFIED:
        return "modified";
    case LYSC_CHANGE_ADDED:
        return "added";
    case LYSC_CHANGE_REMOVED:
        return "removed";
    case LYSC_CHANGE_MOVED:
        return "moved";
    }

    return NULL;
}

/**
 * @brief Get string from a base type.
 *
 * @param[in] basetype Base type.
 * @return String form of the base type.
 */
static const char *
schema_diff_type2str(LY_DATA_TYPE basetype)
{
    switch (basetype) {
    case LY_TYPE_BINARY:
        return "binary";
    case LY_TYPE_UINT8:
        return "uint8";
    case LY_TYPE_UINT16:
        return "uint16";
    case LY_TYPE_UINT32:
        return "uint32";
    case LY_TYPE_UINT64:
        return "uint64";
    case LY_TYPE_INT8:
        return "int8";
    case LY_TYPE_INT16:
        return "int16";
    case LY_TYPE_INT32:
        return "int32";
    case LY_TYPE_INT64:
        return "int64";
    case LY_TYPE_STRING:
        return "string";
    case LY_TYPE_BITS:
        return "bits";
    case LY_TYPE_BOOL:
        return "boolean";
    case LY_TYPE_EMPTY:
        return "empty";
    case LY_TYPE_DEC64:
        return "decimal64";
    case LY_TYPE_ENUM:
        return "enumeration";
    case LY_TYPE_IDENT:
        return "identityref";
    case LY_TYPE_INST:
        return "instance-identifier";
    case LY_TYPE_LEAFREF:
        return "leafref";
    case LY_TYPE_UNION:
        return "union";
    default:
        return "unknown";
    }
}

/**
 * @brief Create cmp YANG data from common information about a change.
 *
 * @param[in] changes Changes to read from.
 * @param[in] change_count Count of @p changes.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_change_info(const struct lysc_diff_change_s *changes, uint32_t change_count, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *changed_list;
    uint32_t i;

    for (i = 0; i < change_count; ++i) {
        /* changed stmt */
        LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "changed", 0, &changed_list,
                schema_diff_changed2str(changes[i].changed)), cleanup);

        /* parent-stmt */
        if (changes[i].parent_changed && (rc = lyd_new_term(changed_list, NULL, "parent-stmt",
                schema_diff_changed2str(changes[i].parent_changed), 0, NULL))) {
            goto cleanup;
        }

        /* change */
        LY_CHECK_GOTO(rc = lyd_new_term(changed_list, NULL, "change", schema_diff_change2str(changes[i].change), 0, NULL),
                cleanup);

        /* conformance */
        LY_CHECK_GOTO(rc = lyd_new_term(changed_list, NULL, "conformance",
                changes[i].is_nbc ? "non-backwards-compatible" : "backwards-compatible", 0, NULL), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement generic child.
 *
 * @param[in] ctx Context to use.
 * @param[in] name Child name.
 * @param[in] value Child value.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_child_add(const struct ly_ctx *ctx, const char *name, const char *value,
        struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *node;

    LY_CHECK_GOTO(rc = lyd_new_opaq(NULL, ctx, name, value, NULL, "yang", &node), cleanup);
    LY_CHECK_GOTO(rc = lyd_insert_sibling(*child_p, node, child_p), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'status'.
 *
 * @param[in] ctx Context to use.
 * @param[in] flags Flags to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_status(const struct ly_ctx *ctx, uint16_t flags, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;

    if (flags & LYS_STATUS_CURR) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "status", "current", child_p), cleanup);
    } else if (flags & LYS_STATUS_OBSLT) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "status", "obsolete", child_p), cleanup);
    } else if (flags & LYS_STATUS_DEPRC) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "status", "deprecated", child_p), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'bit' or 'enum' arrays.
 *
 * @param[in] ctx Context to use.
 * @param[in] items Bit/enum array to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_bitenums(const struct ly_ctx *ctx, const struct lysc_type_bitenum_item *items,
        struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node_opaq *bitenum_node;
    char num_str[12];

    LY_ARRAY_FOR(items, u) {
        /* enum/bit */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, (items[u].flags & LYS_IS_ENUM) ? "enum" : "bit",
                NULL, child_p), cleanup);
        bitenum_node = (struct lyd_node_opaq *)(*child_p)->prev;

        /* name */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "name", items[u].name, &bitenum_node->child),
                cleanup);

        /* description, reference */
        if (items[u].dsc && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "description", items[u].dsc,
                &bitenum_node->child))) {
            goto cleanup;
        }
        if (items[u].ref && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "reference", items[u].ref,
                &bitenum_node->child))) {
            goto cleanup;
        }

        /* value/position */
        if (items[u].flags & LYS_IS_ENUM) {
            sprintf(num_str, "%" PRId32, items[u].value);
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "value", num_str, &bitenum_node->child),
                    cleanup);
        } else {
            sprintf(num_str, "%" PRIu32, items[u].position);
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "position", num_str, &bitenum_node->child),
                    cleanup);
        }

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_status(ctx, items[u].flags, &bitenum_node->child), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, items[u].exts, &bitenum_node->child), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'identity'.
 *
 * @param[in] ctx Context to use.
 * @param[in] ident Identity to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_identity(const struct ly_ctx *ctx, const struct lysc_ident *ident, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node_opaq *ident_node;

    /* identity */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "identity", NULL, child_p), cleanup);
    ident_node = (struct lyd_node_opaq *)(*child_p)->prev;

    /* name */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "name", ident->name, &ident_node->child), cleanup);

    /* ext-instance */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, ident->exts, &ident_node->child), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'pattern' array.
 *
 * @param[in] ctx Context to use.
 * @param[in] fpatterns Pattern array to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_patterns(const struct ly_ctx *ctx, struct lysc_pattern **patterns, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node_opaq *pat_node;

    LY_ARRAY_FOR(patterns, u) {
        /* pattern */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "pattern", NULL, child_p), cleanup);
        pat_node = (struct lyd_node_opaq *)(*child_p)->prev;

        /* expression */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "expression", patterns[u]->expr,
                &pat_node->child), cleanup);

        /* description, reference, error-message, error-app-tag */
        if (patterns[u]->dsc && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "description", patterns[u]->dsc,
                &pat_node->child))) {
            goto cleanup;
        }
        if (patterns[u]->ref && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "reference", patterns[u]->ref,
                &pat_node->child))) {
            goto cleanup;
        }
        if (patterns[u]->emsg && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "error-message", patterns[u]->emsg,
                &pat_node->child))) {
            goto cleanup;
        }
        if (patterns[u]->eapptag && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "error-app-tag",
                patterns[u]->eapptag, &pat_node->child))) {
            goto cleanup;
        }

        /* inverted */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "inverted",
                patterns[u]->inverted ? "true" : "false", &pat_node->child), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, patterns[u]->exts, &pat_node->child), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'range' or 'length'.
 *
 * @param[in] ctx Context to use.
 * @param[in] range Range/length to use.
 * @param[in] is_length Set if @p range is length.
 * @param[in] is_signed Set if the parts are using signed numbers.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_range(const struct ly_ctx *ctx, const struct lysc_range *range, ly_bool is_length,
        ly_bool is_signed, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node_opaq *par_node, *int_node;
    char num_min_str[22], num_max_str[22];

    /* length/range */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, is_length ? "length" : "range", NULL, child_p), cleanup);
    par_node = (struct lyd_node_opaq *)(*child_p)->prev;

    LY_ARRAY_FOR(range->parts, u) {
        /* interval */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "interval", NULL, &par_node->child), cleanup);
        int_node = (struct lyd_node_opaq *)par_node->child->prev;

        if (is_signed) {
            sprintf(num_min_str, "%" PRId64, range->parts[u].min_64);
            sprintf(num_max_str, "%" PRId64, range->parts[u].max_64);
        } else {
            sprintf(num_min_str, "%" PRIu64, range->parts[u].min_u64);
            sprintf(num_max_str, "%" PRIu64, range->parts[u].max_u64);
        }

        /* min */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "min", num_min_str, &int_node->child), cleanup);

        /* max */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "max", num_max_str, &int_node->child), cleanup);
    }

    /* description, reference, error-message, error-app-tag */
    if (range->dsc && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "description", range->dsc, &par_node->child))) {
        goto cleanup;
    }
    if (range->ref && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "reference", range->ref, &par_node->child))) {
        goto cleanup;
    }
    if (range->emsg && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "error-message", range->emsg, &par_node->child))) {
        goto cleanup;
    }
    if (range->eapptag && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "error-app-tag", range->eapptag,
            &par_node->child))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, range->exts, &par_node->child), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'must' array.
 *
 * @param[in] ctx Context to use.
 * @param[in] musts Must array to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_musts(const struct ly_ctx *ctx, const struct lysc_must *musts, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node_opaq *must_node;

    LY_ARRAY_FOR(musts, u) {
        /* must */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "must", NULL, child_p), cleanup);
        must_node = (struct lyd_node_opaq *)(*child_p)->prev;

        /* condition */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "condition", lyxp_get_expr(musts[u].cond),
                &must_node->child), cleanup);

        /* description */
        if (musts[u].dsc && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "description", musts[u].dsc,
                &must_node->child))) {
            goto cleanup;
        }

        /* reference */
        if (musts[u].ref && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "reference", musts[u].ref,
                &must_node->child))) {
            goto cleanup;
        }

        /* error-message */
        if (musts[u].emsg && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "error-message", musts[u].emsg,
                &must_node->child))) {
            goto cleanup;
        }

        /* error-app-tag */
        if (musts[u].eapptag && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "error-app-tag", musts[u].eapptag,
                &must_node->child))) {
            goto cleanup;
        }

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, musts[u].exts, &must_node->child), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'when' array.
 *
 * @param[in] ctx Context to use.
 * @param[in] whens When array to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_whens(const struct ly_ctx *ctx, struct lysc_when **whens, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node_opaq *when_node;

    LY_ARRAY_FOR(whens, u) {
        /* when */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "when", NULL, child_p), cleanup);
        when_node = (struct lyd_node_opaq *)(*child_p)->prev;

        /* condition */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "condition", lyxp_get_expr(whens[u]->cond),
                &when_node->child), cleanup);

        /* description */
        if (whens[u]->dsc && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "description", whens[u]->dsc,
                &when_node->child))) {
            goto cleanup;
        }

        /* reference */
        if (whens[u]->ref && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "reference", whens[u]->ref,
                &when_node->child))) {
            goto cleanup;
        }

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_status(ctx, whens[u]->flags, &when_node->child), cleanup);

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, whens[u]->exts, &when_node->child), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement 'type'.
 *
 * @param[in] ctx Context to use.
 * @param[in] type Type to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_type(const struct ly_ctx *ctx, const struct lysc_type *type, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    const struct lysc_type_num *type_num;
    const struct lysc_type_dec *type_dec;
    const struct lysc_type_str *type_str;
    const struct lysc_type_enum *type_enum;
    const struct lysc_type_bits *type_bits;
    const struct lysc_type_leafref *type_lref;
    const struct lysc_type_identityref *type_identref;
    const struct lysc_type_instanceid *type_instid;
    const struct lysc_type_union *type_union;
    const struct lysc_type_bin *type_bin;
    ly_bool is_signed = 1;

    /* base-type */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "base-type", schema_diff_type2str(type->basetype),
            child_p), cleanup);

    switch (type->basetype) {
    case LY_TYPE_BINARY:
        type_bin = (const struct lysc_type_bin *)type;

        /* length */
        if (type_bin->length) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_range(ctx, type_bin->length, 1, 0, child_p), cleanup);
        }
        break;
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        is_signed = 0;
    /* fallthrough */
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
        type_num = (const struct lysc_type_num *)type;

        /* range */
        if (type_num->range) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_range(ctx, type_num->range, 0, is_signed, child_p), cleanup);
        }
        break;
    case LY_TYPE_STRING:
        type_str = (const struct lysc_type_str *)type;

        /* length */
        if (type_str->length) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_range(ctx, type_str->length, 1, 0, child_p), cleanup);
        }

        /* pattern */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_patterns(ctx, type_str->patterns, child_p), cleanup);
        break;
    case LY_TYPE_BITS:
        type_bits = (const struct lysc_type_bits *)type;

        /* bit */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_bitenums(ctx, type_bits->bits, child_p), cleanup);
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        break;
    case LY_TYPE_DEC64: {
        char num_str[4];

        type_dec = (const struct lysc_type_dec *)type;

        /* fraction-digits */
        sprintf(num_str, "%" PRIu8, type_dec->fraction_digits);
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "fraction-digits", num_str, child_p), cleanup);

        /* range */
        if (type_dec->range) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_range(ctx, type_dec->range, 0, 1, child_p), cleanup);
        }
        break;
    }
    case LY_TYPE_ENUM:
        type_enum = (const struct lysc_type_enum *)type;

        /* enum */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_bitenums(ctx, type_enum->enums, child_p), cleanup);
        break;
    case LY_TYPE_IDENT:
        type_identref = (const struct lysc_type_identityref *)type;

        /* base */
        LY_ARRAY_FOR(type_identref->bases, u) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "base", type_identref->bases[u]->name,
                    child_p), cleanup);
        }
        break;
    case LY_TYPE_INST:
        type_instid = (const struct lysc_type_instanceid *)type;

        /* require-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "require-instance",
                type_instid->require_instance ? "true" : "false", child_p), cleanup);
        break;
    case LY_TYPE_LEAFREF:
        type_lref = (const struct lysc_type_leafref *)type;

        /* path */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "path", lyxp_get_expr(type_lref->path),
                child_p), cleanup);

        /* require-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "require-instance",
                type_lref->require_instance ? "true" : "false", child_p), cleanup);
        break;
    case LY_TYPE_UNION: {
        struct lyd_node_opaq *un_type_node;

        type_union = (const struct lysc_type_union *)type;

        /* union-type */
        LY_ARRAY_FOR(type_union->types, u) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "union-type", NULL, child_p), cleanup);
            un_type_node = (struct lyd_node_opaq *)(*child_p)->prev;

            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_type(ctx, type_union->types[u], &un_type_node->child),
                    cleanup);
        }
        break;
    }
    case LY_TYPE_UNKNOWN:
        /* invalid */
        LOGINT(ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    /* ext-instance */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, type->exts, child_p), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement nested ext-inst array.
 *
 * @param[in] ctx Context to use.
 * @param[in] exts Ext-inst array to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_ext_insts(const struct ly_ctx *ctx, const struct lysc_ext_instance *exts,
        struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node *node;
    struct lyd_node_opaq *ext_node;

    LY_ARRAY_FOR(exts, u) {
        /* ext-instance */
        LY_CHECK_GOTO(rc = lyd_new_opaq(NULL, ctx, exts[u].def->name, NULL, exts[u].def->module->prefix,
                exts[u].def->module->name, &node), cleanup);
        LY_CHECK_GOTO(rc = lyd_insert_sibling(*child_p, node, child_p), cleanup);
        ext_node = (struct lyd_node_opaq *)node;

        /* argument */
        if (exts[u].argument) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "argument", exts[u].argument,
                    &ext_node->child), cleanup);
        }

        /* substmts */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts(ctx, exts[u].substmts, &ext_node->child), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatement nodes, recursively.
 *
 * @param[in] sibling First sibling to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts_nodes_r(const struct lysc_node *sibling, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    const struct ly_ctx *ctx = sibling->module->ctx;
    LY_ARRAY_COUNT_TYPE u, v;
    const char *config;
    char num_str[11];
    uint32_t min, max;
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lysc_node_list *list;
    struct lyd_node_opaq *node, *uniq_node;

    LY_LIST_FOR(sibling, sibling) {
        /* node-type node */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, lys_nodetype2str(sibling->nodetype), NULL,
                child_p), cleanup);
        node = (struct lyd_node_opaq *)(*child_p)->prev;

        /* name */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "name", sibling->name, &node->child), cleanup);

        /* config */
        config = NULL;
        if (sibling->flags & LYS_CONFIG_W) {
            config = "true";
        } else if (sibling->flags & LYS_CONFIG_R) {
            config = "false";
        }
        if (config && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "config", config, &node->child))) {
            goto cleanup;
        }

        /* description */
        if (sibling->dsc && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "description", sibling->dsc, &node->child))) {
            goto cleanup;
        }

        /* must */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_musts(ctx, lysc_node_musts(sibling), &node->child), cleanup);

        /* presence */
        if (sibling->nodetype == LYS_CONTAINER) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "presence",
                    (sibling->flags & LYS_PRESENCE) ? "true" : "false", &node->child), cleanup);
        }

        /* reference */
        if (sibling->ref && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "reference", sibling->ref, &node->child))) {
            goto cleanup;
        }

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_status(ctx, sibling->flags, &node->child), cleanup);

        /* when */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_whens(ctx, lysc_node_when(sibling), &node->child), cleanup);

        /* type */
        if (sibling->nodetype & LYD_NODE_TERM) {
            leaf = (const struct lysc_node_leaf *)sibling;

            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_type(ctx, leaf->type, &node->child), cleanup);
        }

        /* units */
        if (sibling->nodetype & LYD_NODE_TERM) {
            leaf = (const struct lysc_node_leaf *)sibling;

            if (leaf->units && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "units", leaf->units, &node->child))) {
                goto cleanup;
            }
        }

        /* default */
        if (sibling->nodetype == LYS_LEAF) {
            leaf = (const struct lysc_node_leaf *)sibling;

            if (leaf->dflt.str && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "default", leaf->dflt.str, &node->child))) {
                goto cleanup;
            }
        } else if (sibling->nodetype == LYS_LEAFLIST) {
            llist = (const struct lysc_node_leaflist *)sibling;

            LY_ARRAY_FOR(llist->dflts, u) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "default", llist->dflts[u].str,
                        &node->child), cleanup);
            }
        }

        /* min-elements, max-elements */
        min = 0;
        max = UINT32_MAX;
        if (sibling->nodetype == LYS_LIST) {
            list = (const struct lysc_node_list *)sibling;

            min = list->min;
            max = list->max;
        } else if (sibling->nodetype == LYS_LEAFLIST) {
            llist = (const struct lysc_node_leaflist *)sibling;

            min = llist->min;
            max = llist->max;
        }
        if (min > 0) {
            sprintf(num_str, "%" PRIu32, min);
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "min-elements", num_str, &node->child),
                    cleanup);
        }
        if (max < UINT32_MAX) {
            sprintf(num_str, "%" PRIu32, max);
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "max-elements", num_str, &node->child),
                    cleanup);
        }

        /* unique */
        if (sibling->nodetype == LYS_LIST) {
            list = (const struct lysc_node_list *)sibling;

            LY_ARRAY_FOR(list->uniques, u) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "unique", NULL, &node->child), cleanup);
                uniq_node = (struct lyd_node_opaq *)(*child_p)->prev;

                LY_ARRAY_FOR(list->uniques[u], v) {
                    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "node", list->uniques[u][v]->name,
                            &uniq_node->child), cleanup);
                }
            }
        }

        /* ext-instance */
        LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, sibling->exts, &node->child), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst substatements.
 *
 * @param[in] ctx Context to use.
 * @param[in] substmts Substatement array to use.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst_substmts(const struct ly_ctx *ctx, const struct lysc_ext_substmt *substmts, struct lyd_node **child_p)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool siblings_processed = 0;
    const char *config;

    LY_ARRAY_FOR(substmts, u) {
        if (!substmts[u].storage_p) {
            /* not compiled */
            continue;
        }

        switch (substmts[u].stmt) {
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
            /* all the nodes are connected into a sibling list, report only once */
            if (siblings_processed) {
                break;
            }
            siblings_processed = 1;

            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_nodes_r(*(substmts[u].storage_p), child_p), cleanup);
            break;
        case LY_STMT_ARGUMENT:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_ERROR_APP_TAG:
        case LY_STMT_ERROR_MESSAGE:
        case LY_STMT_KEY:
        case LY_STMT_MODIFIER:
        case LY_STMT_NAMESPACE:
        case LY_STMT_ORGANIZATION:
        case LY_STMT_PRESENCE:
        case LY_STMT_REFERENCE:
        case LY_STMT_UNITS:
            /* text */
            if (*(substmts[u].storage_p)) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, lys_stmt_str(substmts[u].stmt),
                        *(substmts[u].storage_p), child_p), cleanup);
            }
            break;
        case LY_STMT_BIT:
        case LY_STMT_ENUM:
            /* bitenum struct array */
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_bitenums(ctx, *(substmts[u].storage_p), child_p), cleanup);
            break;
        case LY_STMT_CONFIG:
            /* config flag */
            config = NULL;
            if (*(uint16_t *)substmts[u].storage_p & LYS_CONFIG_W) {
                config = "true";
            } else if (*(uint16_t *)substmts[u].storage_p & LYS_CONFIG_R) {
                config = "false";
            }
            if (config && (rc = schema_diff_ext_inst_substmts_child_add(ctx, "config", config, child_p))) {
                goto cleanup;
            }
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            /* nexted ext-instance */
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_ext_insts(ctx, *(substmts[u].storage_p), child_p), cleanup);
            break;
        case LY_STMT_FRACTION_DIGITS: {
            char num_str[4];

            /* uint8 number */
            if (*(substmts[u].storage_p)) {
                sprintf(num_str, "%" PRIu8, *(uint8_t *)substmts[u].storage_p);
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, lys_stmt_str(substmts[u].stmt), num_str,
                        child_p), cleanup);
            }
            break;
        }
        case LY_STMT_IDENTITY:
            /* identity */
            if (*(substmts[u].storage_p)) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_identity(ctx, *(substmts[u].storage_p), child_p), cleanup);
            }
            break;
        case LY_STMT_LENGTH:
            /* length */
            if (*(substmts[u].storage_p)) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_range(ctx, *(substmts[u].storage_p), 1, 0, child_p),
                        cleanup);
            }
            break;
        case LY_STMT_MANDATORY:
            /* mandatory flag */
            if (*(uint16_t *)substmts[u].storage_p & LYS_MAND_TRUE) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "mandatory", "true", child_p), cleanup);
            } else {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "mandatory", "false", child_p), cleanup);
            }
            break;
        case LY_STMT_MAX_ELEMENTS: {
            char num_str[11];

            /* uint32 number */
            if (*(uint32_t *)substmts[u].storage_p < UINT32_MAX) {
                sprintf(num_str, "%" PRIu32, *(uint32_t *)substmts[u].storage_p);
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "max-elements", num_str, child_p),
                        cleanup);
            }
            break;
        }
        case LY_STMT_MIN_ELEMENTS: {
            char num_str[11];

            /* uint32 number */
            if (*(uint32_t *)substmts[u].storage_p > 0) {
                sprintf(num_str, "%" PRIu32, *(uint32_t *)substmts[u].storage_p);
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "min-elements", num_str, child_p),
                        cleanup);
            }
            break;
        }
        case LY_STMT_MUST:
            /* must array */
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_musts(ctx, *(substmts[u].storage_p), child_p), cleanup);
            break;
        case LY_STMT_ORDERED_BY:
            /* ordered-by flag */
            if (*(uint16_t *)substmts[u].storage_p & LYS_ORDBY_SYSTEM) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "ordered-by", "system", child_p), cleanup);
            } else if (*(uint16_t *)substmts[u].storage_p & LYS_ORDBY_USER) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "ordered-by", "user", child_p), cleanup);
            }
            break;
        case LY_STMT_PATTERN:
            /* pattern array of arrays */
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_patterns(ctx, *(substmts[u].storage_p), child_p), cleanup);
            break;
        case LY_STMT_POSITION: {
            char num_str[21];

            /* uint64 number */
            sprintf(num_str, "%" PRIu64, *(uint64_t *)substmts[u].storage_p);
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "position", num_str, child_p), cleanup);
            break;
        }
        case LY_STMT_RANGE:
            /* range */
            if (*(substmts[u].storage_p)) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_range(ctx, *(substmts[u].storage_p), 0, 1, child_p),
                        cleanup);
            }
            break;
        case LY_STMT_REQUIRE_INSTANCE:
            /* bool flag */
            if (*(uint8_t *)substmts[u].storage_p) {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "require-instance", "true", child_p),
                        cleanup);
            } else {
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "require-instance", "false", child_p),
                        cleanup);
            }
            break;
        case LY_STMT_STATUS:
            /* status flag */
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_status(ctx, *(uint16_t *)substmts[u].storage_p, child_p),
                    cleanup);
            break;
        case LY_STMT_TYPE: {
            struct lyd_node_opaq *type_node;

            if (*(substmts[u].storage_p)) {
                /* type */
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "type", NULL, child_p), cleanup);
                type_node = (struct lyd_node_opaq *)(*child_p)->prev;

                /* substatements */
                LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_type(ctx, *(substmts[u].storage_p), &type_node->child),
                        cleanup);
            }
            break;
        }
        case LY_STMT_VALUE: {
            char num_str[21];

            /* int64 number */
            sprintf(num_str, "%" PRId64, *(int64_t *)substmts[u].storage_p);
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_child_add(ctx, "value", num_str, child_p), cleanup);
            break;
        }
        case LY_STMT_WHEN:
            /* when array */
            LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts_whens(ctx, *(substmts[u].storage_p), child_p), cleanup);
            break;
        case LY_STMT_AUGMENT:
        case LY_STMT_ARG_TEXT:
        case LY_STMT_ARG_VALUE:
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
            LOGINT(ctx);
            rc = LY_EINT;
            goto cleanup;
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an ext-inst.
 *
 * @param[in] ext Ext-inst to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst(const struct lysc_ext_instance *ext, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *ext_par, *ext_child = NULL;

    /* list instance */
    LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "ext-instance", 0, &ext_par), cleanup);

    /* module */
    LY_CHECK_GOTO(rc = lyd_new_term(ext_par, NULL, "module", ext->def->module->name, 0, NULL), cleanup);

    /* name */
    LY_CHECK_GOTO(rc = lyd_new_term(ext_par, NULL, "name", ext->def->name, 0, NULL), cleanup);

    /* argument */
    if (ext->argument) {
        LY_CHECK_GOTO(rc = lyd_new_term(ext_par, NULL, "argument", ext->argument, 0, NULL), cleanup);
    }

    /* substatements */
    LY_CHECK_GOTO(rc = schema_diff_ext_inst_substmts(LYD_CTX(change_cont), ext->substmts, &ext_child), cleanup);
    if (ext_child) {
        LY_CHECK_GOTO(rc = lyd_new_any(ext_par, NULL, "substatements", ext_child, LYD_ANYDATA_DATATREE,
                LYD_NEW_ANY_USE_VALUE, NULL), cleanup);
        ext_child = NULL;
    }

cleanup:
    lyd_free_siblings(ext_child);
    return rc;
}

/**
 * @brief Create leaf-list of enabled features as part of cmp YANG data.
 *
 * @param[in] mod Module to use.
 * @param[in,out] parent Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_enabled_features(const struct lys_module *mod, struct lyd_node *parent)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!mod->implemented) {
        return LY_SUCCESS;
    }

    LY_ARRAY_FOR(mod->compiled->features, u) {
        LY_CHECK_RET(lyd_new_term(parent, NULL, "enabled-feature", mod->compiled->features[u], 0, NULL));
    }

    return LY_SUCCESS;
}

/**
 * @brief Create list of submodules of the module as part of cmp YANG data.
 *
 * @param[in] mod Module to use.
 * @param[in,out] parent Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_submodules(const struct lys_module *mod, struct lyd_node *parent)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    const struct lysp_submodule *submod;
    const char *revision;

    LY_ARRAY_FOR(mod->parsed->includes, u) {
        submod = mod->parsed->includes[u].submodule;

        /* add the includes */
        revision = submod->revs ? submod->revs[0].date : NULL;
        LY_CHECK_GOTO(rc = lyd_new_list(parent, NULL, "submodule", 0, NULL, submod->name, revision), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from module imports.
 *
 * @param[in] mod Module to use.
 * @param[in] schema YANG data schema node to create.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_imports(const struct lys_module *mod, const struct lysc_node *schema, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    char *keys = NULL;
    const struct lys_module *imp;
    struct lyd_node *mod_list;

    LY_ARRAY_FOR(mod->parsed->imports, u) {
        imp = mod->parsed->imports[u].module;

        /* check that the import has not yet been added */
        free(keys);
        if (asprintf(&keys, "[name='%s'][revision='%s']", imp->name, imp->revision) == -1) {
            LOGMEM(mod->ctx);
            rc = LY_EMEM;
            goto cleanup;
        }
        if (!lyd_find_sibling_val(diff_list, schema, keys, 0, NULL)) {
            continue;
        }

        /* add the imports, recursively */
        LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, schema->name, 0, &mod_list, imp->name, imp->revision), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_enabled_features(imp, mod_list), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_submodules(imp, mod_list), cleanup);

        LY_CHECK_GOTO(rc = schema_diff_imports(imp, schema, diff_list), cleanup);
    }

cleanup:
    free(keys);
    return rc;
}

/**
 * @brief Create cmp YANG data from direct substatements of 'module'.
 *
 * @param[in] mod Module to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_stmts(const struct lys_module *mod, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lyd_node *ident_list;
    const struct lysc_ident *ident;

    /* organization */
    if (mod->org && (rc = lyd_new_term(change_cont, NULL, "organization", mod->org, 0, NULL))) {
        goto cleanup;
    }

    /* contact */
    if (mod->contact && (rc = lyd_new_term(change_cont, NULL, "contact", mod->contact, 0, NULL))) {
        goto cleanup;
    }

    /* description */
    if (mod->dsc && (rc = lyd_new_term(change_cont, NULL, "description", mod->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (mod->ref && (rc = lyd_new_term(change_cont, NULL, "reference", mod->ref, 0, NULL))) {
        goto cleanup;
    }

    /* identity */
    LY_ARRAY_FOR(mod->identities, u) {
        ident = &mod->identities[u];

        /* identity name */
        LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "identity", 0, &ident_list, ident->name), cleanup);

        /* ext-instance */
        LY_ARRAY_FOR(ident->exts, v) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&ident->exts[v], ident_list), cleanup);
        }
    }

    /* ext-instance */
    LY_ARRAY_FOR(mod->compiled->exts, v) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&mod->compiled->exts[v], change_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from 'module' changes.
 *
 * @param[in] node_change Node change to use, is actually a module change.
 * @param[in] mod1 First module.
 * @param[in] mod2 Second module.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module(const struct lysc_diff_node_change_s *node_change, const struct lys_module *mod1,
        const struct lys_module *mod2, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_diff_cont, *cont;

    if (!node_change->change_count) {
        /* no changes */
        goto cleanup;
    }

    /* module diff */
    LY_CHECK_GOTO(rc = lyd_new_inner(diff_list, NULL, "module-diff", 0, &mod_diff_cont), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_change_info(node_change->changes, node_change->change_count, mod_diff_cont), cleanup);

    /* old */
    LY_CHECK_GOTO(rc = lyd_new_inner(mod_diff_cont, NULL, "old", 0, &cont), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_module_stmts(mod1, cont), cleanup);

    /* new */
    LY_CHECK_GOTO(rc = lyd_new_inner(mod_diff_cont, NULL, "new", 0, &cont), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_module_stmts(mod2, cont), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a 'must' array.
 *
 * @param[in] musts Must array to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_musts(const struct lysc_must *musts, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lyd_node *must_list;

    LY_ARRAY_FOR(musts, u) {
        LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "must", 0, &must_list), cleanup);

        /* condition */
        LY_CHECK_GOTO(rc = lyd_new_term(must_list, NULL, "condition", lyxp_get_expr(musts[u].cond), 0, NULL), cleanup);

        /* description */
        if (musts[u].dsc && (rc = lyd_new_term(must_list, NULL, "description", musts[u].dsc, 0, NULL))) {
            goto cleanup;
        }

        /* reference */
        if (musts[u].ref && (rc = lyd_new_term(must_list, NULL, "reference", musts[u].ref, 0, NULL))) {
            goto cleanup;
        }

        /* error-message */
        if (musts[u].emsg && (rc = lyd_new_term(must_list, NULL, "error-message", musts[u].emsg, 0, NULL))) {
            goto cleanup;
        }

        /* error-app-tag */
        if (musts[u].eapptag && (rc = lyd_new_term(must_list, NULL, "error-app-tag", musts[u].eapptag, 0, NULL))) {
            goto cleanup;
        }

        /* ext-instance */
        LY_ARRAY_FOR(musts[u].exts, v) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&musts[u].exts[v], must_list), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a 'status'.
 *
 * @param[in] flags Flags to use.
 * @param[in,out] parent Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_status(uint16_t flags, struct lyd_node *parent)
{
    LY_ERR rc = LY_SUCCESS;
    const char *status;

    if (flags & LYS_STATUS_CURR) {
        status = "current";
    } else if (flags & LYS_STATUS_DEPRC) {
        status = "deprecated";
    } else {
        assert(flags & LYS_STATUS_OBSLT);
        status = "obsolete";
    }
    LY_CHECK_GOTO(rc = lyd_new_term(parent, NULL, "status", status, 0, NULL), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a 'when' array.
 *
 * @param[in] whens When array to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_whens(struct lysc_when **whens, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lyd_node *when_list;

    LY_ARRAY_FOR(whens, u) {
        LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "when", 0, &when_list), cleanup);

        /* condition */
        LY_CHECK_GOTO(rc = lyd_new_term(when_list, NULL, "condition", lyxp_get_expr(whens[u]->cond), 0, NULL), cleanup);

        /* description */
        if (whens[u]->dsc && (rc = lyd_new_term(when_list, NULL, "description", whens[u]->dsc, 0, NULL))) {
            goto cleanup;
        }

        /* reference */
        if (whens[u]->ref && (rc = lyd_new_term(when_list, NULL, "reference", whens[u]->ref, 0, NULL))) {
            goto cleanup;
        }

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status(whens[u]->flags, when_list), cleanup);

        /* ext-instance */
        LY_ARRAY_FOR(whens[u]->exts, v) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&whens[u]->exts[v], when_list), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a 'range' or 'length'.
 *
 * @param[in] range Range/lengthto use.
 * @param[in] is_signed Set if parts (intervals) of the range/length are signed.
 * @param[in,out] parent Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_range(const struct lysc_range *range, ly_bool is_signed, struct lyd_node *parent)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node *interval_list;

    LY_ARRAY_FOR(range->parts, u) {
        /* interval */
        LY_CHECK_GOTO(rc = lyd_new_list(parent, NULL, "interval", 0, &interval_list), cleanup);
        if (is_signed) {
            LY_CHECK_GOTO(rc = lyd_new_term_bin(interval_list, NULL, "min", &range->parts[u].min_64,
                    sizeof range->parts[u].min_64, 0, NULL), cleanup);
            LY_CHECK_GOTO(rc = lyd_new_term_bin(interval_list, NULL, "max", &range->parts[u].max_64,
                    sizeof range->parts[u].max_64, 0, NULL), cleanup);
        } else {
            LY_CHECK_GOTO(rc = lyd_new_term_bin(interval_list, NULL, "min", &range->parts[u].min_u64,
                    sizeof range->parts[u].min_u64, 0, NULL), cleanup);
            LY_CHECK_GOTO(rc = lyd_new_term_bin(interval_list, NULL, "max", &range->parts[u].max_u64,
                    sizeof range->parts[u].max_u64, 0, NULL), cleanup);
        }
    }

    /* description, reference, error-message, error-app-tag */
    if (range->dsc && (rc = lyd_new_term(parent, NULL, "description", range->dsc, 0, NULL))) {
        goto cleanup;
    }
    if (range->ref && (rc = lyd_new_term(parent, NULL, "reference", range->ref, 0, NULL))) {
        goto cleanup;
    }
    if (range->emsg && (rc = lyd_new_term(parent, NULL, "error-message", range->emsg, 0, NULL))) {
        goto cleanup;
    }
    if (range->eapptag && (rc = lyd_new_term(parent, NULL, "error-app-tag", range->eapptag, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(range->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&range->exts[u], parent), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a 'pattern' array.
 *
 * @param[in] patterns Pattern array to use.
 * @param[in,out] type_par Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_patterns(struct lysc_pattern **patterns, struct lyd_node *type_par)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lyd_node *pat_list;

    LY_ARRAY_FOR(patterns, u) {
        LY_CHECK_GOTO(rc = lyd_new_list(type_par, NULL, "pattern", 0, &pat_list), cleanup);

        /* expression */
        LY_CHECK_GOTO(rc = lyd_new_term(pat_list, NULL, "expression", patterns[u]->expr, 0, NULL), cleanup);

        /* description, reference, error-message, error-app-tag */
        if (patterns[u]->dsc && (rc = lyd_new_term(pat_list, NULL, "description", patterns[u]->dsc, 0, NULL))) {
            goto cleanup;
        }
        if (patterns[u]->ref && (rc = lyd_new_term(pat_list, NULL, "reference", patterns[u]->ref, 0, NULL))) {
            goto cleanup;
        }
        if (patterns[u]->emsg && (rc = lyd_new_term(pat_list, NULL, "error-message", patterns[u]->emsg, 0, NULL))) {
            goto cleanup;
        }
        if (patterns[u]->eapptag && (rc = lyd_new_term(pat_list, NULL, "error-app-tag", patterns[u]->eapptag, 0, NULL))) {
            goto cleanup;
        }

        /* inverted */
        LY_CHECK_GOTO(rc = lyd_new_term(pat_list, NULL, "inverted", patterns[u]->inverted ? "true" : "false", 0, NULL),
                cleanup);

        /* ext-instance */
        LY_ARRAY_FOR(patterns[u]->exts, v) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&patterns[u]->exts[v], pat_list), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a 'bit' or 'enum' array.
 *
 * @param[in] items Bit/enum array to use.
 * @param[in,out] type_par Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type_bitenums(const struct lysc_type_bitenum_item *items, struct lyd_node *type_par)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lyd_node *par_list;

    LY_ARRAY_FOR(items, u) {
        /* list with the key */
        LY_CHECK_GOTO(rc = lyd_new_list(type_par, NULL, (items[u].flags & LYS_IS_ENUM) ? "enum" : "bit", 0, &par_list,
                items[u].name), cleanup);

        /* description, reference */
        if (items[u].dsc && (rc = lyd_new_term(par_list, NULL, "description", items[u].dsc, 0, NULL))) {
            goto cleanup;
        }
        if (items[u].ref && (rc = lyd_new_term(par_list, NULL, "reference", items[u].ref, 0, NULL))) {
            goto cleanup;
        }

        /* value/position */
        if (items[u].flags & LYS_IS_ENUM) {
            LY_CHECK_GOTO(rc = lyd_new_term_bin(par_list, NULL, "value", &items[u].value, sizeof items[u].value, 0,
                    NULL), cleanup);
        } else {
            LY_CHECK_GOTO(rc = lyd_new_term_bin(par_list, NULL, "position", &items[u].position, sizeof items[u].position,
                    0, NULL), cleanup);
        }

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status(items[u].flags, par_list), cleanup);

        /* ext-instance */
        LY_ARRAY_FOR(items[u].exts, v) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&items[u].exts[v], par_list), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a 'type'.
 *
 * @param[in] type Type to use.
 * @param[in,out] type_par Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_type(const struct lysc_type *type, struct lyd_node *type_par)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    const struct lysc_type_num *type_num;
    const struct lysc_type_dec *type_dec;
    const struct lysc_type_str *type_str;
    const struct lysc_type_enum *type_enum;
    const struct lysc_type_bits *type_bits;
    const struct lysc_type_leafref *type_lref;
    const struct lysc_type_identityref *type_identref;
    const struct lysc_type_instanceid *type_instid;
    const struct lysc_type_union *type_union;
    const struct lysc_type_bin *type_bin;
    struct lyd_node *parent;
    ly_bool is_signed = 1;

    /* base-type */
    LY_CHECK_GOTO(rc = lyd_new_term(type_par, NULL, "base-type", schema_diff_type2str(type->basetype), 0, NULL), cleanup);

    switch (type->basetype) {
    case LY_TYPE_BINARY:
        type_bin = (const struct lysc_type_bin *)type;

        /* length */
        if (type_bin->length) {
            LY_CHECK_GOTO(rc = lyd_new_inner(type_par, NULL, "length", 0, &parent), cleanup);
            LY_CHECK_GOTO(rc = schema_diff_node_type_range(type_bin->length, 0, parent), cleanup);
        }
        break;
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        is_signed = 0;
    /* fallthrough */
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
        type_num = (const struct lysc_type_num *)type;

        /* range */
        if (type_num->range) {
            LY_CHECK_GOTO(rc = lyd_new_inner(type_par, NULL, "range", 0, &parent), cleanup);
            LY_CHECK_GOTO(rc = schema_diff_node_type_range(type_num->range, is_signed, parent), cleanup);
        }
        break;
    case LY_TYPE_STRING:
        type_str = (const struct lysc_type_str *)type;

        /* length */
        if (type_str->length) {
            LY_CHECK_GOTO(rc = lyd_new_inner(type_par, NULL, "length", 0, &parent), cleanup);
            LY_CHECK_GOTO(rc = schema_diff_node_type_range(type_str->length, 0, parent), cleanup);
        }

        /* pattern */
        LY_CHECK_GOTO(rc = schema_diff_node_type_patterns(type_str->patterns, type_par), cleanup);
        break;
    case LY_TYPE_BITS:
        type_bits = (const struct lysc_type_bits *)type;

        /* bit */
        LY_CHECK_GOTO(rc = schema_diff_node_type_bitenums(type_bits->bits, type_par), cleanup);
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        break;
    case LY_TYPE_DEC64:
        type_dec = (const struct lysc_type_dec *)type;

        /* fraction-digits */
        LY_CHECK_GOTO(rc = lyd_new_term_bin(type_par, NULL, "fraction-digits", &type_dec->fraction_digits,
                sizeof type_dec->fraction_digits, 0, NULL), cleanup);

        /* range */
        if (type_dec->range) {
            LY_CHECK_GOTO(rc = lyd_new_inner(type_par, NULL, "range", 0, &parent), cleanup);
            LY_CHECK_GOTO(rc = schema_diff_node_type_range(type_dec->range, 1, parent), cleanup);
        }
        break;
    case LY_TYPE_ENUM:
        type_enum = (const struct lysc_type_enum *)type;

        /* enum */
        LY_CHECK_GOTO(rc = schema_diff_node_type_bitenums(type_enum->enums, type_par), cleanup);
        break;
    case LY_TYPE_IDENT:
        type_identref = (const struct lysc_type_identityref *)type;

        /* base */
        LY_ARRAY_FOR(type_identref->bases, u) {
            LY_CHECK_GOTO(rc = lyd_new_term(type_par, NULL, "base", type_identref->bases[u]->name, 0, NULL), cleanup);
        }
        break;
    case LY_TYPE_INST:
        type_instid = (const struct lysc_type_instanceid *)type;

        /* require-instance */
        LY_CHECK_GOTO(rc = lyd_new_term(type_par, NULL, "require-instance",
                type_instid->require_instance ? "true" : "false", 0, NULL), cleanup);
        break;
    case LY_TYPE_LEAFREF:
        type_lref = (const struct lysc_type_leafref *)type;

        /* path */
        LY_CHECK_GOTO(rc = lyd_new_term(type_par, NULL, "path", lyxp_get_expr(type_lref->path), 0, NULL), cleanup);

        /* require-instance */
        LY_CHECK_GOTO(rc = lyd_new_term(type_par, NULL, "require-instance",
                type_lref->require_instance ? "true" : "false", 0, NULL), cleanup);
        break;
    case LY_TYPE_UNION:
        type_union = (const struct lysc_type_union *)type;

        /* union-type */
        LY_ARRAY_FOR(type_union->types, u) {
            LY_CHECK_GOTO(rc = lyd_new_list(type_par, NULL, "union-type", 0, &parent), cleanup);
            LY_CHECK_GOTO(rc = schema_diff_node_type(type_union->types[u], parent), cleanup);
        }
        break;
    case LY_TYPE_UNKNOWN:
        /* invalid */
        LOGINT(NULL);
        rc = LY_EINT;
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(type->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&type->exts[u], type_par), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a node.
 *
 * @param[in] node Node to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_stmts(const struct lysc_node *node, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    const char *config = NULL, *mandatory = NULL, *ordered_by = NULL;
    uint32_t min, max;
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lysc_node_list *list;
    struct lyd_node *unique_list, *type_cont;

    /* config */
    if (node->flags & LYS_CONFIG_W) {
        config = "true";
    } else if (node->flags & LYS_CONFIG_R) {
        config = "false";
    }
    if (config && (rc = lyd_new_term(change_cont, NULL, "config", config, 0, NULL))) {
        goto cleanup;
    }

    /* description */
    if (node->dsc && (rc = lyd_new_term(change_cont, NULL, "description", node->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* mandatory */
    if (node->nodetype & (LYS_LEAF | LYS_CHOICE | LYD_NODE_ANY)) {
        mandatory = (node->flags & LYS_MAND_TRUE) ? "true" : "false";
    }
    if (mandatory && (rc = lyd_new_term(change_cont, NULL, "mandatory", mandatory, 0, NULL))) {
        goto cleanup;
    }

    /* must */
    LY_CHECK_GOTO(rc = schema_diff_node_musts(lysc_node_musts(node), change_cont), cleanup);

    /* presence */
    if (node->nodetype == LYS_CONTAINER) {
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "presence", (node->flags & LYS_PRESENCE) ? "true" : "false",
                0, NULL), cleanup);
    }

    /* reference */
    if (node->ref && (rc = lyd_new_term(change_cont, NULL, "reference", node->ref, 0, NULL))) {
        goto cleanup;
    }

    /* status */
    LY_CHECK_GOTO(rc = schema_diff_status(node->flags, change_cont), cleanup);

    /* when */
    LY_CHECK_GOTO(rc = schema_diff_node_whens(lysc_node_when(node), change_cont), cleanup);

    /* type */
    if (node->nodetype & LYD_NODE_TERM) {
        leaf = (const struct lysc_node_leaf *)node;

        LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "type", 0, &type_cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_node_type(leaf->type, type_cont), cleanup);
    }

    /* units */
    if (node->nodetype & LYD_NODE_TERM) {
        leaf = (const struct lysc_node_leaf *)node;

        if (leaf->units && (rc = lyd_new_term(change_cont, NULL, "units", leaf->units, 0, NULL))) {
            goto cleanup;
        }
    }

    /* ordered-by */
    if (node->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        ordered_by = (node->flags & LYS_ORDBY_USER) ? "user" : "system";
    }
    if (ordered_by && (rc = lyd_new_term(change_cont, NULL, "ordered-by", ordered_by, 0, NULL))) {
        rc = 1;
        goto cleanup;
    }

    /* default */
    if (node->nodetype == LYS_LEAF) {
        leaf = (const struct lysc_node_leaf *)node;

        if (leaf->dflt.str && (rc = lyd_new_term(change_cont, NULL, "default", leaf->dflt.str, 0, NULL))) {
            goto cleanup;
        }
    } else if (node->nodetype == LYS_LEAFLIST) {
        llist = (const struct lysc_node_leaflist *)node;

        LY_ARRAY_FOR(llist->dflts, u) {
            LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "default", llist->dflts[u].str, 0, NULL), cleanup);
        }
    }

    /* min-elements, max-elements */
    min = 0;
    max = UINT32_MAX;
    if (node->nodetype == LYS_LIST) {
        list = (const struct lysc_node_list *)node;

        min = list->min;
        max = list->max;
    } else if (node->nodetype == LYS_LEAFLIST) {
        llist = (const struct lysc_node_leaflist *)node;

        min = llist->min;
        max = llist->max;
    }
    if ((min > 0) && (rc = lyd_new_term_bin(change_cont, NULL, "min-elements", &min, sizeof min, 0, NULL))) {
        goto cleanup;
    }
    if ((max < UINT32_MAX) && (rc = lyd_new_term_bin(change_cont, NULL, "max-elements", &max, sizeof max, 0, NULL))) {
        goto cleanup;
    }

    /* unique */
    if (node->nodetype == LYS_LIST) {
        list = (const struct lysc_node_list *)node;

        LY_ARRAY_FOR(list->uniques, u) {
            LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "unique", 0, &unique_list), cleanup);

            LY_ARRAY_FOR(list->uniques[u], v) {
                LY_CHECK_GOTO(rc = lyd_new_term(unique_list, NULL, "node", list->uniques[u][v]->name, 0, NULL), cleanup);
            }
        }
    }

    /* ext-instance */
    LY_ARRAY_FOR(node->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&node->exts[u], change_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Get enum string from a node type.
 *
 * @param[in] nodetype Node type.
 * @return String enum value form of the node type.
 */
static const char *
schema_diff_nodetype2enum(uint16_t nodetype)
{
    switch (nodetype) {
    case LYS_CONTAINER:
        return "container";
    case LYS_LEAF:
        return "leaf";
    case LYS_LEAFLIST:
        return "leaf-list";
    case LYS_LIST:
        return "list";
    case LYS_ANYDATA:
        return "anydata";
    case LYS_ANYXML:
        return "anyxml";
    case LYS_RPC:
        return "rpc";
    case LYS_ACTION:
        return "action";
    case LYS_NOTIF:
        return "notification";
    default:
        return "unkown-node-type";
    }
}

/**
 * @brief Create cmp YANG data from a node change.
 *
 * @param[in] node_change Node change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node(const struct lysc_diff_node_change_s *node_change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *node_diff_list, *change_cont;
    const struct lysc_node *node;
    char *path = NULL;

    assert(node_change->snode_old || node_change->snode_new);

    if (!node_change->change_count) {
        /* no changes */
        goto cleanup;
    }

    node = node_change->snode_old ? node_change->snode_old : node_change->snode_new;

    /* do not report schema-only nodes */
    if (node->nodetype & (LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)) {
        goto cleanup;
    }

    /* list instance with its key */
    path = lysc_path(node, LYSC_PATH_DATA, NULL, 0);
    if (!path) {
        rc = LY_EMEM;
        goto cleanup;
    }
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "node-diff", 0, &node_diff_list, path), cleanup);

    /* node-type */
    LY_CHECK_GOTO(rc = lyd_new_term(node_diff_list, NULL, "node-type", schema_diff_nodetype2enum(node->nodetype), 0,
            NULL), cleanup);

    /* input/output */
    if (node->flags & LYS_IS_INPUT) {
        LY_CHECK_GOTO(rc = lyd_new_term(node_diff_list, NULL, "in-rpc-action", "input", 0, NULL), cleanup);
    } else if (node->flags & LYS_IS_OUTPUT) {
        LY_CHECK_GOTO(rc = lyd_new_term(node_diff_list, NULL, "in-rpc-action", "output", 0, NULL), cleanup);
    }

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_change_info(node_change->changes, node_change->change_count, node_diff_list), cleanup);

    /* old */
    if (node_change->snode_old) {
        LY_CHECK_GOTO(rc = lyd_new_inner(node_diff_list, NULL, "old", 0, &change_cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_node_stmts(node_change->snode_old, change_cont), cleanup);
    }

    /* new */
    if (node_change->snode_new) {
        LY_CHECK_GOTO(rc = lyd_new_inner(node_diff_list, NULL, "new", 0, &change_cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_node_stmts(node_change->snode_new, change_cont), cleanup);
    }

cleanup:
    free(path);
    return rc;
}

LY_ERR
lysc_diff_tree(const struct lys_module *mod1, const struct lys_module *mod2, const struct lysc_diff_s *diff,
        const struct lys_module *cmp_mod, struct lyd_node **schema_diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *diff_cont = NULL, *diff_list, *mod_cont;
    const struct lysc_node *imp_schema;
    uint32_t i;

    /* structure extension */
    assert(LY_ARRAY_COUNT(cmp_mod->compiled->exts) == 1);
    LY_CHECK_GOTO(rc = lyd_new_ext_inner(&cmp_mod->compiled->exts[0], "schema-comparison", &diff_cont), cleanup);
    LY_CHECK_GOTO(rc = lyd_new_list(diff_cont, NULL, "compiled-diff", 0, &diff_list), cleanup);

    /* source module info */
    LY_CHECK_GOTO(rc = lyd_new_inner(diff_list, NULL, "source", 0, &mod_cont), cleanup);
    LY_CHECK_GOTO(rc = lyd_new_term(mod_cont, NULL, "module", mod1->name, 0, NULL), cleanup);
    LY_CHECK_GOTO(rc = lyd_new_term(mod_cont, NULL, "revision", mod1->revision, 0, NULL), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_enabled_features(mod1, mod_cont), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_submodules(mod1, mod_cont), cleanup);

    imp_schema = lys_find_path(NULL, diff_list->schema, "source-import", 0);
    LY_CHECK_GOTO(rc = schema_diff_imports(mod1, imp_schema, diff_list), cleanup);

    /* target module info */
    LY_CHECK_GOTO(rc = lyd_new_inner(diff_list, NULL, "target", 0, &mod_cont), cleanup);
    LY_CHECK_GOTO(rc = lyd_new_term(mod_cont, NULL, "module", mod2->name, 0, NULL), cleanup);
    LY_CHECK_GOTO(rc = lyd_new_term(mod_cont, NULL, "revision", mod2->revision, 0, NULL), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_enabled_features(mod2, mod_cont), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_submodules(mod2, mod_cont), cleanup);

    imp_schema = lys_find_path(NULL, diff_list->schema, "target-import", 0);
    LY_CHECK_GOTO(rc = schema_diff_imports(mod2, imp_schema, diff_list), cleanup);

    /* overall conformance */
    LY_CHECK_GOTO(rc = lyd_new_term(diff_list, NULL, "conformance",
            diff->is_nbc ? "non-backwards-compatible" : "backwards-compatible", 0, NULL), cleanup);

    /* module diff */
    assert(!diff->node_changes[0].snode_old && !diff->node_changes[0].snode_new);
    LY_CHECK_GOTO(rc = schema_diff_module(&diff->node_changes[0], mod1, mod2, diff_list), cleanup);

    /* node diff */
    for (i = 1; i < diff->node_change_count; ++i) {
        /* node diff */
        LY_CHECK_GOTO(rc = schema_diff_node(&diff->node_changes[i], diff_list), cleanup);
    }

cleanup:
    if (rc) {
        lyd_free_tree(diff_cont);
    } else {
        *schema_diff = diff_cont;
    }
    return rc;
}
