/**
 * @file schema_diff_tree.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema diff tree functions
 *
 * Copyright (c) 2025 - 2026 CESNET, z.s.p.o.
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
static LY_ERR schema_diff_ext_inst(const struct lysc_ext_instance *ext, int is_cont, struct lyd_node *change_cont);

/**
 * @brief Get string from a changed statement.
 *
 * @param[in] ch Changed statement.
 * @return String form of the changed statement.
 */
static const char *
schema_diff_changed2str(enum lys_diff_changed_e ch)
{
    switch (ch) {
    case LYS_CHANGED_NONE:
        break;
    case LYS_CHANGED_BASE:
        return "base";
    case LYS_CHANGED_BELONGS_TO:
        return "belongs-to";
    case LYS_CHANGED_BIT:
        return "bit";
    case LYS_CHANGED_CONFIG:
        return "config";
    case LYS_CHANGED_CONTACT:
        return "contact";
    case LYS_CHANGED_DEFAULT:
        return "default";
    case LYS_CHANGED_DESCRIPTION:
        return "description";
    case LYS_CHANGED_DEVIATE:
        return "deviate";
    case LYS_CHANGED_DEVIATION:
        return "deviation";
    case LYS_CHANGED_ENUM:
        return "enum";
    case LYS_CHANGED_ERR_APP_TAG:
        return "error-app-tag";
    case LYS_CHANGED_ERR_MSG:
        return "error-message";
    case LYS_CHANGED_EXTENSION:
        return "extension";
    case LYS_CHANGED_EXT_INST:
        return "extension-instance";
    case LYS_CHANGED_FEATURE:
        return "feature";
    case LYS_CHANGED_FRAC_DIG:
        return "fraction-digits";
    case LYS_CHANGED_IDENT:
        return "identity";
    case LYS_CHANGED_IF_FEATURE:
        return "if-feature";
    case LYS_CHANGED_IMPORT:
        return "import";
    case LYS_CHANGED_INCLUDE:
        return "include";
    case LYS_CHANGED_KEY:
        return "key";
    case LYS_CHANGED_LENGTH:
        return "length";
    case LYS_CHANGED_MANDATORY:
        return "mandatory";
    case LYS_CHANGED_MAX_ELEM:
        return "max-elements";
    case LYS_CHANGED_MIN_ELEM:
        return "min-elements";
    case LYS_CHANGED_MODIFIER:
        return "modifier";
    case LYS_CHANGED_MODULE:
        return "module";
    case LYS_CHANGED_MUST:
        return "must";
    case LYS_CHANGED_NAMESPACE:
        return "namespace";
    case LYS_CHANGED_NODE:
        return "node";
    case LYS_CHANGED_ORDERED_BY:
        return "ordered-by";
    case LYS_CHANGED_ORGANIZATION:
        return "organization";
    case LYS_CHANGED_PATH:
        return "path";
    case LYS_CHANGED_PATTERN:
        return "pattern";
    case LYS_CHANGED_POSITION:
        return "position";
    case LYS_CHANGED_PREFIX:
        return "prefix";
    case LYS_CHANGED_PRESENCE:
        return "presence";
    case LYS_CHANGED_RANGE:
        return "range";
    case LYS_CHANGED_REFERENCE:
        return "reference";
    case LYS_CHANGED_REFINE:
        return "refine";
    case LYS_CHANGED_REQ_INSTANCE:
        return "require-instance";
    case LYS_CHANGED_REVISION:
        return "revision";
    case LYS_CHANGED_REVISION_DATE:
        return "revision-date";
    case LYS_CHANGED_STATUS:
        return "status";
    case LYS_CHANGED_SUBMODULE:
        return "submodue";
    case LYS_CHANGED_TYPE:
        return "type";
    case LYS_CHANGED_TYPEDEF:
        return "typedef";
    case LYS_CHANGED_UNITS:
        return "units";
    case LYS_CHANGED_VALUE:
        return "value";
    case LYS_CHANGED_UNIQUE:
        return "unique";
    case LYS_CHANGED_WHEN:
        return "when";
    case LYS_CHANGED_YANG_VERSION:
        return "yang-version";
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
schema_diff_change2str(enum lys_diff_change_e ch)
{
    switch (ch) {
    case LYS_CHANGE_MODIFIED:
        return "modified";
    case LYS_CHANGE_ADDED:
        return "added";
    case LYS_CHANGE_REMOVED:
        return "removed";
    case LYS_CHANGE_MOVED:
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
 * @brief Gnereate a path to a parsed node.
 *
 * @param[in] node Node to use.
 * @return Generated path.
 * @return NULL on error.
 */
static char *
schema_diff_pnode_path(const struct lysp_node *node)
{
    const struct lysp_node **nodes = NULL;
    char *path = NULL;
    const char *node_pre, *node_post;
    uint32_t node_count = 0, path_len = 0, i, new_len;

    if (!node) {
        /* top-level */
        if (asprintf(&path, "/") == -1) {
            LOGMEM(NULL);
            path = NULL;
            goto cleanup;
        }
        goto cleanup;
    }

    /* collect all the parent nodes */
    while (node) {
        nodes = ly_realloc(nodes, (node_count + 1) * sizeof *nodes);
        LY_CHECK_ERR_GOTO(!nodes, LOGMEM(NULL), cleanup);

        /* store the node */
        nodes[node_count] = node;
        ++node_count;

        node = node->parent;
    }

    i = node_count;
    do {
        --i;

        /* reset vars */
        node_pre = NULL;
        node_post = NULL;

        /* prepare node type */
        switch (nodes[i]->nodetype) {
        case LYS_CONTAINER:
        case LYS_CHOICE:
        case LYS_LIST:
        case LYS_CASE:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
        case LYS_INPUT:
        case LYS_OUTPUT:
            /* print jsut the node name */
            break;
        case LYS_USES:
            node_pre = "{uses=";
            node_post = "}";
            break;
        case LYS_GROUPING:
            node_pre = "{grp=";
            node_post = "}";
            break;
        case LYS_AUGMENT:
            node_pre = "{aug=";
            node_post = "}";
            break;
        default:
            LOGINT(NULL);
            free(path);
            path = NULL;
            goto cleanup;
        }

        /* append new node */
        new_len = path_len;
        if (node_pre && node_post) {
            new_len += strlen(node_pre) + strlen(node_post);
        }
        new_len += 1 + strlen(nodes[i]->name) + 1;
        path = ly_realloc(path, new_len + 1);
        LY_CHECK_ERR_GOTO(!path, LOGMEM(NULL), cleanup);

        sprintf(path + path_len, "/%s%s%s", node_pre ? node_pre : "", nodes[i]->name, node_post ? node_post : "");
        path_len = new_len;
    } while (i);

cleanup:
    free(nodes);
    return path;
}

/**
 * @brief Get string from a conformance.
 *
 * @param[in] conform Conformance to transform.
 * @return String conformance.
 */
static const char *
schema_diff_conform2str(enum lys_diff_conform_e conform)
{
    switch (conform) {
    case LYS_CONFORM_ED:
        return "editorial";
    case LYS_CONFORM_BC:
        return "backwards-compatible";
    case LYS_CONFORM_NBC:
        return "non-backwards-compatible";
    }

    return NULL;
}

/**
 * @brief Create cmp YANG data from common information about a change.
 *
 * @param[in] change Change to read from.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_change_info(const struct lys_diff_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *changed_list;

    /* changed stmt */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "changed", 0, &changed_list,
            schema_diff_changed2str(change->changed)), cleanup);

    /* parent-stmt */
    if (change->parent_changed && (rc = lyd_new_term(changed_list, NULL, "parent-stmt",
            schema_diff_changed2str(change->parent_changed), 0, NULL))) {
        goto cleanup;
    }

    /* change */
    LY_CHECK_GOTO(rc = lyd_new_term(changed_list, NULL, "change", schema_diff_change2str(change->change),
            0, NULL), cleanup);

    /* conformance */
    LY_CHECK_GOTO(rc = lyd_new_term(changed_list, NULL, "conformance", schema_diff_conform2str(change->conform), 0,
            NULL), cleanup);

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from common information about changes.
 *
 * @param[in] changes Changes to read from.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_changes_info(const struct lys_diff_changes_s *changes, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    uint32_t i;

    for (i = 0; i < changes->count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_change_info(&changes->changes[i], diff_list), cleanup);
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
 * @param[in] is_cont Whether the outer inner node is container or a list.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ext_inst(const struct lysc_ext_instance *ext, int is_cont, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *ext_par, *ext_child = NULL;

    /* inner node */
    if (is_cont) {
        LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "ext-instance", 0, &ext_par), cleanup);
    } else {
        LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "ext-instance", 0, &ext_par), cleanup);
    }

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
        LY_CHECK_GOTO(rc = lyd_new_any(ext_par, NULL, "substatements", ext_child, NULL, 0, LYD_NEW_ANY_USE_VALUE, NULL),
                cleanup);
        ext_child = NULL;
    }

cleanup:
    lyd_free_siblings(ext_child);
    return rc;
}

/**
 * @brief Create cmp YANG data from a parsed ext-inst substatements (children).
 *
 * @param[in] ctx Context to use.
 * @param[in] child Child statement to use, recursively.
 * @param[in,out] child_p Child to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pext_inst_children(const struct ly_ctx *ctx, const struct lysp_stmt *child, struct lyd_node **child_p)
{
    struct lyd_node_opaq *opaq;

    LY_LIST_FOR(child, child) {
        LY_CHECK_RET(schema_diff_ext_inst_substmts_child_add(ctx, child->stmt, child->arg, child_p));

        /* recurisively */
        opaq = (struct lyd_node_opaq *)*child_p;
        LY_CHECK_RET(schema_diff_pext_inst_children(ctx, child->child, &opaq->child));
    }

    return LY_SUCCESS;
}

/**
 * @brief Create cmp YANG data from a parsed ext-inst.
 *
 * @param[in] ext Ext-inst to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pext_inst(const struct lysp_ext_instance *ext, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *ext_par, *ext_child = NULL;
    const char *mod_name, *name;

    /* parse */
    lysp_nodeid_find_module(LYD_CTX(change_cont), ext->name, ext->format, ext->prefix_data, &mod_name, &name);

    /* inner node */
    LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "ext-instance", 0, &ext_par), cleanup);

    /* module */
    LY_CHECK_GOTO(rc = lyd_new_term(ext_par, NULL, "module", mod_name, 0, NULL), cleanup);

    /* name */
    LY_CHECK_GOTO(rc = lyd_new_term(ext_par, NULL, "name", name, 0, NULL), cleanup);

    /* argument */
    if (ext->argument) {
        LY_CHECK_GOTO(rc = lyd_new_term(ext_par, NULL, "argument", ext->argument, 0, NULL), cleanup);
    }

    /* substatements (children) */
    LY_CHECK_GOTO(rc = schema_diff_pext_inst_children(LYD_CTX(change_cont), ext->child, &ext_child), cleanup);
    if (ext_child) {
        LY_CHECK_GOTO(rc = lyd_new_any(ext_par, NULL, "substatements", ext_child, NULL, 0,
                LYD_NEW_ANY_USE_VALUE, NULL), cleanup);
        ext_child = NULL;
    }

cleanup:
    lyd_free_siblings(ext_child);
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
    const char *status = NULL;

    if (flags & LYS_STATUS_CURR) {
        status = "current";
    } else if (flags & LYS_STATUS_DEPRC) {
        status = "deprecated";
    } else if (flags & LYS_STATUS_OBSLT) {
        status = "obsolete";
    }

    if (status) {
        LY_CHECK_GOTO(rc = lyd_new_term(parent, NULL, "status", status, 0, NULL), cleanup);
    }

cleanup:
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
        if (asprintf(&keys, "[module='%s'][revision='%s']", imp->name, imp->revision) == -1) {
            LOGMEM(mod->ctx);
            rc = LY_EMEM;
            goto cleanup;
        }
        if (!lyd_find_sibling_val(lyd_child(diff_list), schema, keys, 0, NULL)) {
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
 * @brief Create cmp YANG data from direct 'module' substatement.
 *
 * @param[in] change Change to use.
 * @param[in] mod1 Old module.
 * @param[in] mod2 New module.
 * @param[in] with_parsed Whether 'parsed-schema' feature is enabled.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_substmt(const struct lys_diff_change_s *change, const struct lys_module *mod1,
        const struct lys_module *mod2, ly_bool with_parsed, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;
    const char *node_name, *text_old, *text_new;

    /* learn about the change */
    switch (change->changed) {
    case LYS_CHANGED_PREFIX:
        if (!with_parsed) {
            /* parsed-only */
            goto cleanup;
        }

        node_name = "prefix";
        text_old = mod1->prefix;
        text_new = mod2->prefix;
        break;
    case LYS_CHANGED_CONTACT:
        node_name = "contact";
        text_old = mod1->contact;
        text_new = mod2->contact;
        break;
    case LYS_CHANGED_DESCRIPTION:
        node_name = "description";
        text_old = mod1->dsc;
        text_new = mod2->dsc;
        break;
    case LYS_CHANGED_ORGANIZATION:
        node_name = "organization";
        text_old = mod1->org;
        text_new = mod2->org;
        break;
    case LYS_CHANGED_REFERENCE:
        node_name = "reference";
        text_old = mod1->ref;
        text_new = mod2->ref;
        break;
    case LYS_CHANGED_YANG_VERSION:
        node_name = "yang-version";
        break;
    default:
        LOGINT(mod1->ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_change_info(change, mod_cmp_list), cleanup);

    if (change->changed != LYS_CHANGED_YANG_VERSION) {
        /* text substatement */
        if (text_old) {
            /* old */
            LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
            LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, node_name, text_old, 0, NULL), cleanup);
        }

        if (text_new) {
            /* new */
            LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
            LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, node_name, text_new, 0, NULL), cleanup);
        }
    } else {
        /* yang-version substatement */
        if (mod1->version) {
            LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
            LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, node_name,
                    mod1->version == LYS_VERSION_1_1 ? "1.1" : "1", 0, NULL), cleanup);
        }

        if (mod2->version) {
            LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
            LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, node_name,
                    mod2->version == LYS_VERSION_1_1 ? "1.1" : "1", 0, NULL), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an identity.
 *
 * @param[in] ident Compiled identity to use.
 * @param[in] p_ident Parsed identity to use.
 * @param[in] with_parsed Whether 'parsed-schema' feature is enabled.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_ident(const struct lysc_ident *ident, const struct lysp_ident *p_ident, ly_bool with_parsed,
        struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *ident_cont;
    LY_ARRAY_COUNT_TYPE u;

    /* identity container */
    LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "identity", 0, &ident_cont), cleanup);

    /* name */
    LY_CHECK_GOTO(rc = lyd_new_term(ident_cont, NULL, "name", ident->name, 0, NULL), cleanup);

    if (with_parsed) {
        /* if-features */
        LY_ARRAY_FOR(p_ident->iffeatures, u) {
            LY_CHECK_GOTO(rc = lyd_new_term(ident_cont, NULL, "if-feature", p_ident->iffeatures[u].str, 0, NULL),
                    cleanup);
        }

        /* base */
        LY_ARRAY_FOR(p_ident->bases, u) {
            LY_CHECK_GOTO(rc = lyd_new_term(ident_cont, NULL, "base", p_ident->bases[u], 0, NULL), cleanup);
        }
    }

    /* ext-inst */
    LY_ARRAY_FOR(ident->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&ident->exts[u], 0, ident_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from identity changes of 'module'.
 *
 * @param[in] change Identity change to use.
 * @param[in] with_parsed Whether 'parsed-schema' feature is enabled.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_ident(const struct lys_diff_ident_change_s *change, ly_bool with_parsed, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this identity */
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, mod_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, mod_cmp_list), cleanup);
    }

    if (change->ident_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_ident(change->ident_old, change->p_ident_old, with_parsed, cont), cleanup);
    }

    if (change->ident_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_ident(change->ident_new, change->p_ident_new, with_parsed, cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a parsed when.
 *
 * @param[in] when Parsed when to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_when(const struct lysp_when *when, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *when_list;
    LY_ARRAY_COUNT_TYPE u;

    /* when */
    LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "when", 0, &when_list), cleanup);

    /* condition */
    LY_CHECK_GOTO(rc = lyd_new_term(when_list, NULL, "condition", when->cond, 0, NULL), cleanup);

    /* description */
    if (when->dsc && (rc = lyd_new_term(when_list, NULL, "description", when->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (when->ref && (rc = lyd_new_term(when_list, NULL, "reference", when->ref, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(when->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&when->exts[u], when_list), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data children from a parsed restriction.
 *
 * @param[in] restr Parsed restriction to use.
 * @param[in] leaf_name Name of the first restriction leaf to create.
 * @param[in,out] cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_restr_children(const struct lysp_restr *restr, const char *leaf_name, struct lyd_node *parent)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;

    /* check if pattern */
    if (!strcmp(leaf_name, "expression")) {
        /* expression */
        LY_CHECK_GOTO(rc = lyd_new_term(parent, NULL, leaf_name, restr->arg.str + 1, 0, NULL), cleanup);

        /* inverted */
        if ((restr->arg.str[0] == 0x15) && (rc = lyd_new_term(parent, NULL, "inverted", NULL, 0, NULL))) {
            goto cleanup;
        }
    } else {
        /* restriction */
        LY_CHECK_GOTO(rc = lyd_new_term(parent, NULL, leaf_name, restr->arg.str, 0, NULL), cleanup);
    }

    /* description, reference, error-message, error-app-tag */
    if (restr->dsc && (rc = lyd_new_term(parent, NULL, "description", restr->dsc, 0, NULL))) {
        goto cleanup;
    }
    if (restr->ref && (rc = lyd_new_term(parent, NULL, "reference", restr->ref, 0, NULL))) {
        goto cleanup;
    }
    if (restr->emsg && (rc = lyd_new_term(parent, NULL, "error-message", restr->emsg, 0, NULL))) {
        goto cleanup;
    }
    if (restr->eapptag && (rc = lyd_new_term(parent, NULL, "error-app-tag", restr->eapptag, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(restr->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&restr->exts[u], parent), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a parsed node.
 *
 * @param[in] pnode Parsed node to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_pnode(const struct lysp_node *pnode, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node *must_list;
    const struct lysp_node_choice *choic;
    const struct lysp_node_case *cas;
    const struct lysp_node_uses *uses;
    const struct lysp_node_action_inout *inout;
    const struct lysp_node_augment *aug;
    const struct lysp_when *when = NULL;
    const struct lysp_restr *musts = NULL;
    const char *dflt = NULL;

    /* description */
    if (pnode->dsc && (rc = lyd_new_term(change_cont, NULL, "description", pnode->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (pnode->ref && (rc = lyd_new_term(change_cont, NULL, "reference", pnode->ref, 0, NULL))) {
        goto cleanup;
    }

    /* status */
    LY_CHECK_GOTO(rc = schema_diff_status(pnode->flags, change_cont), cleanup);

    /* if-features */
    LY_ARRAY_FOR(pnode->iffeatures, u) {
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "if-feature", pnode->iffeatures[u].str, 0, NULL), cleanup);
    }

    switch (pnode->nodetype) {
    case LYS_CHOICE:
        /* when, dflt */
        choic = (struct lysp_node_choice *)pnode;
        when = choic->when;
        dflt = choic->dflt.str;

        break;
    case LYS_CASE:
        /* when */
        cas = (struct lysp_node_case *)pnode;
        when = cas->when;

        break;
    case LYS_USES:
        /* when */
        uses = (struct lysp_node_uses *)pnode;
        when = uses->when;

        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        /* musts */
        inout = (struct lysp_node_action_inout *)pnode;
        musts = inout->musts;

        break;
    case LYS_GROUPING:
        /* no special substatements */
        break;
    case LYS_AUGMENT:
        /* when */
        aug = (struct lysp_node_augment *)pnode;
        when = aug->when;

        break;
    default:
        LOGINT(NULL);
        rc = LY_EINT;
        goto cleanup;
    }

    /* when */
    if (when && (rc = schema_diff_parsed_when(when, change_cont))) {
        goto cleanup;
    }

    /* musts */
    LY_ARRAY_FOR(musts, u) {
        LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "must", 0, &must_list), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_parsed_restr_children(&musts[u], "condition", must_list), cleanup);
    }

    /* default */
    if (dflt && (rc = lyd_new_term(change_cont, NULL, "default", dflt, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(pnode->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&pnode->exts[u], change_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from all parsed node changes.
 *
 * @param[in] change Parsed node change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_pnode(const struct lys_diff_pnode_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *parsed_cmp_list, *cont;
    char *path = NULL;
    const struct lysp_node *pnode;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this parsed node */
        goto cleanup;
    }

    pnode = change->pnode_old ? change->pnode_old : change->pnode_new;

    /* parsed comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "parsed-comparison", 0, &parsed_cmp_list), cleanup);

    /* parent path */
    path = schema_diff_pnode_path(pnode->parent);
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "parent-path", path, 0, NULL), cleanup);

    /* indentifier */
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "identifier", pnode->name, 0, NULL), cleanup);

    /* stmt-type */
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "stmt-type", lys_nodetype2str(pnode->nodetype), 0, NULL), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, parsed_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, parsed_cmp_list), cleanup);
    }

    if (change->pnode_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(parsed_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_pnode(change->pnode_old, cont), cleanup);
    }

    if (change->pnode_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(parsed_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_pnode(change->pnode_new, cont), cleanup);
    }

cleanup:
    free(path);
    return rc;
}

/**
 * @brief Create cmp YANG data from a refine.
 *
 * @param[in] refine Parsed refine to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_refine(const struct lysp_refine *refine, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node *must_list;
    const char *config = NULL, *mandatory = NULL;
    char num_str[11];

    /* description */
    if (refine->dsc && (rc = lyd_new_term(change_cont, NULL, "description", refine->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (refine->ref && (rc = lyd_new_term(change_cont, NULL, "reference", refine->ref, 0, NULL))) {
        goto cleanup;
    }

    /* if-features */
    LY_ARRAY_FOR(refine->iffeatures, u) {
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "if-feature", refine->iffeatures[u].str, 0, NULL), cleanup);
    }

    /* musts */
    LY_ARRAY_FOR(refine->musts, u) {
        LY_CHECK_GOTO(rc = lyd_new_list(change_cont, NULL, "must", 0, &must_list), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_parsed_restr_children(&refine->musts[u], "condition", must_list), cleanup);
    }

    /* presence */
    if (refine->presence && (rc = lyd_new_term(change_cont, NULL, "presence", refine->presence, 0, NULL))) {
        goto cleanup;
    }

    /* defaults */
    LY_ARRAY_FOR(refine->dflts, u) {
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "default", refine->dflts[u].str, 0, NULL), cleanup);
    }

    /* min-elements */
    if (refine->flags & LYS_SET_MIN) {
        sprintf(num_str, "%" PRIu32, refine->min);
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "min-elements", num_str, 0, NULL), cleanup);
    }

    /* max-elements */
    if (refine->flags & LYS_SET_MAX) {
        sprintf(num_str, "%" PRIu32, refine->max);
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "max-elements", num_str, 0, NULL), cleanup);
    }

    /* config */
    if (refine->flags & LYS_CONFIG_W) {
        config = "true";
    } else if (refine->flags & LYS_CONFIG_R) {
        config = "false";
    }
    if (config && (rc = lyd_new_term(change_cont, NULL, "config", config, 0, NULL))) {
        goto cleanup;
    }

    /* mandatory */
    if (refine->flags & LYS_MAND_TRUE) {
        mandatory = "true";
    } else if (refine->flags & LYS_MAND_FALSE) {
        mandatory = "false";
    }
    if (mandatory && (rc = lyd_new_term(change_cont, NULL, "mandatory", mandatory, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(refine->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&refine->exts[u], change_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from all refine changes.
 *
 * @param[in] change Refine change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_refine(const struct lys_diff_refine_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *parsed_cmp_list, *cont;
    char *path = NULL;
    const char *nodeid;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this refine */
        goto cleanup;
    }

    /* parsed comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "parsed-comparison", 0, &parsed_cmp_list), cleanup);

    /* parent path */
    path = schema_diff_pnode_path(change->parent_new);
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "parent-path", path, 0, NULL), cleanup);

    /* indentifier */
    nodeid = change->refine_old ? change->refine_old->nodeid : change->refine_new->nodeid;
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "identifier", nodeid, 0, NULL), cleanup);

    /* stmt-type */
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "stmt-type", "refine", 0, NULL), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, parsed_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, parsed_cmp_list), cleanup);
    }

    if (change->refine_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(parsed_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_refine(change->refine_old, cont), cleanup);
    }

    if (change->refine_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(parsed_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_refine(change->refine_new, cont), cleanup);
    }

cleanup:
    free(path);
    return rc;
}

/**
 * @brief Create cmp YANG data from a parsed enum or bit.
 *
 * @param[in] enum Parsed enum or bit to use.
 * @param[in] is_pattern Whether @p enum is a bit, creates different nodes.
 * @param[in,out] cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_enum(const struct lysp_type_enum *enm, ly_bool is_bit, struct lyd_node *cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *enum_list;
    LY_ARRAY_COUNT_TYPE u;
    char str[22];

    /* list with name */
    LY_CHECK_GOTO(rc = lyd_new_list(cont, NULL, is_bit ? "bit" : "enum", 0, &enum_list, enm->name), cleanup);

    /* if-features */
    LY_ARRAY_FOR(enm->iffeatures, u) {
        LY_CHECK_GOTO(rc = lyd_new_term(enum_list, NULL, "if-feature", enm->iffeatures[u].str, 0, NULL), cleanup);
    }

    /* description */
    if (enm->dsc && (rc = lyd_new_term(enum_list, NULL, "description", enm->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (enm->ref && (rc = lyd_new_term(enum_list, NULL, "reference", enm->ref, 0, NULL))) {
        goto cleanup;
    }

    if (is_bit) {
        /* position */
        sprintf(str, "%" PRIu64, (uint64_t)enm->value);
        LY_CHECK_GOTO(rc = lyd_new_term(enum_list, NULL, "position", str, LYD_NEW_VAL_CANON, NULL), cleanup);
    } else {
        /* value */
        sprintf(str, "%" PRId64, enm->value);
        LY_CHECK_GOTO(rc = lyd_new_term(enum_list, NULL, "value", str, LYD_NEW_VAL_CANON, NULL), cleanup);
    }

    /* status */
    LY_CHECK_GOTO(rc = schema_diff_status(enm->flags, enum_list), cleanup);

    /* ext-instance */
    LY_ARRAY_FOR(enm->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&enm->exts[u], enum_list), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a parsed type.
 *
 * @param[in] type Parsed type to use.
 * @param[in,out] cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_type(const struct lysp_type *type, struct lyd_node *cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    char str[4];
    const char *req_inst_str;
    struct lyd_node *un_type_cont, *restr_parent;

    /* name */
    LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, "name", type->name, 0, NULL), cleanup);

    /* range */
    if (type->range) {
        LY_CHECK_GOTO(rc = lyd_new_inner(cont, NULL, "range", 0, &restr_parent), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_parsed_restr_children(type->range, "restriction", restr_parent), cleanup);
    }

    /* length */
    if (type->length) {
        LY_CHECK_GOTO(rc = lyd_new_inner(cont, NULL, "length", 0, &restr_parent), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_parsed_restr_children(type->length, "restriction", restr_parent), cleanup);
    }

    /* fraction-digits */
    if (type->flags & LYS_SET_FRDIGITS) {
        sprintf(str, "%" PRIu8, type->fraction_digits);
        LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, "fraction-digits", str, LYD_NEW_VAL_CANON, NULL), cleanup);
    }

    /* patterns */
    LY_ARRAY_FOR(type->patterns, u) {
        LY_CHECK_GOTO(rc = lyd_new_list(cont, NULL, "length", 0, &restr_parent), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_parsed_restr_children(&type->patterns[u], "expression", restr_parent), cleanup);
    }

    /* enums */
    LY_ARRAY_FOR(type->enums, u) {
        LY_CHECK_GOTO(rc = schema_diff_parsed_enum(&type->enums[u], 0, cont), cleanup);
    }

    /* bits */
    LY_ARRAY_FOR(type->bits, u) {
        LY_CHECK_GOTO(rc = schema_diff_parsed_enum(&type->bits[u], 1, cont), cleanup);
    }

    /* path */
    if (type->path && (rc = lyd_new_term(cont, NULL, "path", lyxp_get_expr(type->path), 0, NULL))) {
        goto cleanup;
    }

    /* require-instance */
    if (type->flags & LYS_SET_REQINST) {
        if (type->require_instance) {
            req_inst_str = "true";
        } else {
            req_inst_str = "false";
        }
    } else {
        req_inst_str = NULL;
    }
    if (req_inst_str && (rc = lyd_new_term(cont, NULL, "require-instance", req_inst_str, 0, NULL))) {
        goto cleanup;
    }

    /* bases */
    LY_ARRAY_FOR(type->bases, u) {
        LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, "base", type->bases[u], 0, NULL), cleanup);
    }

    /* types */
    LY_ARRAY_FOR(type->types, u) {
        LY_CHECK_GOTO(rc = lyd_new_inner(cont, NULL, "union-type", 0, &un_type_cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_parsed_type(&type->types[u], un_type_cont), cleanup)
    }

    /* ext-instance */
    LY_ARRAY_FOR(type->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&type->exts[u], cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a typedef.
 *
 * @param[in] typedef Parsed typedef to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_typedef(const struct lysp_tpdf *tpdf, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *type_cont;
    LY_ARRAY_COUNT_TYPE u;

    /* default */
    if (tpdf->dflt.str && (rc = lyd_new_term(change_cont, NULL, "default", tpdf->dflt.str, 0, NULL))) {
        goto cleanup;
    }

    /* description */
    if (tpdf->dsc && (rc = lyd_new_term(change_cont, NULL, "description", tpdf->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (tpdf->ref && (rc = lyd_new_term(change_cont, NULL, "reference", tpdf->ref, 0, NULL))) {
        goto cleanup;
    }

    /* status */
    LY_CHECK_GOTO(rc = schema_diff_status(tpdf->flags, change_cont), cleanup);

    /* type */
    LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "type", 0, &type_cont), cleanup);
    LY_CHECK_GOTO(rc = schema_diff_parsed_type(&tpdf->type, type_cont), cleanup);

    /* units */
    if (tpdf->units && (rc = lyd_new_term(change_cont, NULL, "units", tpdf->units, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(tpdf->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&tpdf->exts[u], change_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from all typedef changes.
 *
 * @param[in] change Typedef change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed_typedef(const struct lys_diff_typedef_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *parsed_cmp_list, *cont;
    char *path = NULL;
    const char *tpdf_name;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this typedef */
        goto cleanup;
    }

    /* parsed comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "parsed-comparison", 0, &parsed_cmp_list), cleanup);

    /* parent path */
    path = schema_diff_pnode_path(change->parent_new);
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "parent-path", path, 0, NULL), cleanup);

    /* indentifier */
    tpdf_name = change->typedef_old ? change->typedef_old->name : change->typedef_new->name;
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "identifier", tpdf_name, 0, NULL), cleanup);

    /* stmt-type */
    LY_CHECK_GOTO(rc = lyd_new_term(parsed_cmp_list, NULL, "stmt-type", "typedef", 0, NULL), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, parsed_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, parsed_cmp_list), cleanup);
    }

    if (change->typedef_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(parsed_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_typedef(change->typedef_old, cont), cleanup);
    }

    if (change->typedef_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(parsed_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_typedef(change->typedef_new, cont), cleanup);
    }

cleanup:
    free(path);
    return rc;
}

/**
 * @brief Create cmp YANG data from parsed-only statement changes.
 *
 * @param[in] diff Diff to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_parsed(const struct lys_diff_s *diff, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    uint32_t i;

    /* choice, case, input, output, uses, grouping, augment */
    for (i = 0; i < diff->pnode_change_count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_parsed_pnode(&diff->pnode_changes[i], diff_list), cleanup);
    }

    /* refine */
    for (i = 0; i < diff->refine_change_count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_parsed_refine(&diff->refine_changes[i], diff_list), cleanup);
    }

    /* typedefs */
    for (i = 0; i < diff->typedef_change_count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_parsed_typedef(&diff->typedef_changes[i], diff_list), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an import.
 *
 * @param[in] imp Parsed import to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_import(const struct lysp_import *imp, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *cont;
    LY_ARRAY_COUNT_TYPE u;

    /* import container */
    LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "import", 0, &cont), cleanup);

    /* module */
    LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, "module", imp->name, 0, NULL), cleanup);

    /* prefix */
    LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, "prefix", imp->prefix, 0, NULL), cleanup);

    /* revision-date */
    if (imp->rev[0] && (rc = lyd_new_term(cont, NULL, "revision-date", imp->rev, 0, NULL))) {
        goto cleanup;
    }

    /* description */
    if (imp->dsc && (rc = lyd_new_term(cont, NULL, "description", imp->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (imp->ref && (rc = lyd_new_term(cont, NULL, "reference", imp->ref, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(imp->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&imp->exts[u], cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from import changes of 'module'.
 *
 * @param[in] change Import change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_import(const struct lys_diff_import_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this import */
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, mod_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, mod_cmp_list), cleanup);
    }

    if (change->imp_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_import(change->imp_old, cont), cleanup);
    }

    if (change->imp_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_import(change->imp_new, cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an include.
 *
 * @param[in] inc Parsed include to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_include(const struct lysp_include *inc, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *cont;
    LY_ARRAY_COUNT_TYPE u;

    /* include container */
    LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "include", 0, &cont), cleanup);

    /* submodule */
    LY_CHECK_GOTO(rc = lyd_new_term(cont, NULL, "submodule", inc->name, 0, NULL), cleanup);

    /* revision-date */
    if (inc->rev[0] && (rc = lyd_new_term(cont, NULL, "revision-date", inc->rev, 0, NULL))) {
        goto cleanup;
    }

    /* description */
    if (inc->dsc && (rc = lyd_new_term(cont, NULL, "description", inc->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (inc->ref && (rc = lyd_new_term(cont, NULL, "reference", inc->ref, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(inc->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&inc->exts[u], cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from include changes of 'module'.
 *
 * @param[in] change Include change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_include(const struct lys_diff_include_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this include */
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, mod_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, mod_cmp_list), cleanup);
    }

    if (change->inc_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_include(change->inc_old, cont), cleanup);
    }

    if (change->inc_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_include(change->inc_new, cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from an extension.
 *
 * @param[in] ext Parsed extension to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_extension(const struct lysp_ext *ext, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *ext_cont;
    LY_ARRAY_COUNT_TYPE u;

    /* extension container */
    LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "extension", 0, &ext_cont), cleanup);

    /* name */
    LY_CHECK_GOTO(rc = lyd_new_term(ext_cont, NULL, "name", ext->name, 0, NULL), cleanup);

    /* argument */
    if (ext->argname && (rc = lyd_new_term(ext_cont, NULL, "argument", ext->argname, 0, NULL))) {
        goto cleanup;
    }

    /* status */
    LY_CHECK_GOTO(rc = schema_diff_status(ext->flags, ext_cont), cleanup);

    /* description */
    if (ext->dsc && (rc = lyd_new_term(ext_cont, NULL, "description", ext->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (ext->ref && (rc = lyd_new_term(ext_cont, NULL, "reference", ext->ref, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(ext->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&ext->exts[u], ext_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from extension changes of 'module'.
 *
 * @param[in] change Extension change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_extension(const struct lys_diff_extension_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this extension */
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, mod_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, mod_cmp_list), cleanup);
    }

    if (change->extension_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_extension(change->extension_old, cont), cleanup);
    }

    if (change->extension_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_extension(change->extension_new, cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a feature.
 *
 * @param[in] ext Parsed feature to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_feature(const struct lysp_feature *feat, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *feat_cont;
    LY_ARRAY_COUNT_TYPE u;

    /* feature container */
    LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "feature", 0, &feat_cont), cleanup);

    /* name */
    LY_CHECK_GOTO(rc = lyd_new_term(feat_cont, NULL, "name", feat->name, 0, NULL), cleanup);

    /* if-features */
    LY_ARRAY_FOR(feat->iffeatures, u) {
        LY_CHECK_GOTO(rc = lyd_new_term(feat_cont, NULL, "if-feature", feat->iffeatures[u].str, 0, NULL), cleanup);
    }

    /* status */
    LY_CHECK_GOTO(rc = schema_diff_status(feat->flags, feat_cont), cleanup);

    /* description */
    if (feat->dsc && (rc = lyd_new_term(feat_cont, NULL, "description", feat->dsc, 0, NULL))) {
        goto cleanup;
    }

    /* reference */
    if (feat->ref && (rc = lyd_new_term(feat_cont, NULL, "reference", feat->ref, 0, NULL))) {
        goto cleanup;
    }

    /* ext-instance */
    LY_ARRAY_FOR(feat->exts, u) {
        LY_CHECK_GOTO(rc = schema_diff_pext_inst(&feat->exts[u], feat_cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from feature changes of 'module'.
 *
 * @param[in] change Feature change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_feature(const struct lys_diff_feat_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this extension */
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, mod_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, mod_cmp_list), cleanup);
    }

    if (change->feat_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_feature(change->feat_old, cont), cleanup);
    }

    if (change->feat_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_feature(change->feat_new, cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a deviation.
 *
 * @param[in] dev Parsed deviation to use.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_deviation(const struct lysp_deviation *dev, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *dev_cont, *dev_list, *parent;
    LY_ARRAY_COUNT_TYPE u;
    const struct lysp_deviate *d;
    const struct lysp_deviate_add *d_add;
    const struct lysp_deviate_del *d_del;
    const struct lysp_deviate_rpl *d_rpl;
    const char *argument = NULL, *units = NULL, *dflt = NULL, *config = NULL, *mandatory = NULL;
    const struct lysp_restr *musts = NULL;
    const struct lysp_qname *uniques = NULL, *dflts = NULL;
    uint16_t flags = 0;
    uint32_t min = 0, max = 0;
    const struct lysp_type *type = NULL;
    char num_str[11];

    /* deviation container */
    LY_CHECK_GOTO(rc = lyd_new_inner(change_cont, NULL, "deviation", 0, &dev_cont), cleanup);

    /* target */
    LY_CHECK_GOTO(rc = lyd_new_term(dev_cont, NULL, "target", dev->nodeid, 0, NULL), cleanup);

    /* deviate */
    LY_LIST_FOR(dev->deviates, d) {
        /* collect all the substatements */
        switch (d->mod) {
        case LYS_DEV_NOT_SUPPORTED:
            argument = "not-supported";
            break;
        case LYS_DEV_ADD:
            argument = "add";

            d_add = (const struct lysp_deviate_add *)d;
            units = d_add->units;
            musts = d_add->musts;
            uniques = d_add->uniques;
            dflts = d_add->dflts;
            flags = d_add->flags;
            min = d_add->min;
            max = d_add->max;
            break;
        case LYS_DEV_DELETE:
            argument = "delete";

            d_del = (const struct lysp_deviate_del *)d;
            units = d_del->units;
            musts = d_del->musts;
            uniques = d_del->uniques;
            dflts = d_del->dflts;
            break;
        case LYS_DEV_REPLACE:
            argument = "replace";

            d_rpl = (const struct lysp_deviate_rpl *)d;
            type = d_rpl->type;
            units = d_rpl->units;
            dflt = d_rpl->dflt.str;
            flags = d_add->flags;
            min = d_add->min;
            max = d_add->max;
            break;
        }

        /* deviate list */
        LY_CHECK_GOTO(rc = lyd_new_list(dev_cont, NULL, "deviate", 0, &dev_list), cleanup);

        /* argument */
        LY_CHECK_GOTO(rc = lyd_new_term(dev_list, NULL, "argument", argument, 0, NULL), cleanup);

        /* musts */
        LY_ARRAY_FOR(musts, u) {
            LY_CHECK_GOTO(rc = lyd_new_list(dev_list, NULL, "must", 0, &parent), cleanup);
            LY_CHECK_GOTO(rc = schema_diff_parsed_restr_children(&musts[u], "condition", parent), cleanup);
        }

        /* default */
        if (dflt && (rc = lyd_new_term(dev_list, NULL, "default", dflt, 0, NULL))) {
            goto cleanup;
        } else {
            LY_ARRAY_FOR(dflts, u) {
                LY_CHECK_GOTO(rc = lyd_new_term(dev_list, NULL, "default", dflts[u].str, 0, NULL), cleanup);
            }
        }

        /* config */
        if (flags & LYS_CONFIG_W) {
            config = "true";
        } else if (flags & LYS_CONFIG_R) {
            config = "false";
        }
        if (config && (rc = lyd_new_term(dev_list, NULL, "config", config, 0, NULL))) {
            goto cleanup;
        }

        /* mandatory */
        if (flags & LYS_MAND_TRUE) {
            mandatory = "true";
        } else if (flags & LYS_MAND_FALSE) {
            mandatory = "false";
        }
        if (mandatory && (rc = lyd_new_term(dev_list, NULL, "mandatory", mandatory, 0, NULL))) {
            goto cleanup;
        }

        /* min-elements */
        if (flags & LYS_SET_MIN) {
            sprintf(num_str, "%" PRIu32, min);
            LY_CHECK_GOTO(rc = lyd_new_term(dev_list, NULL, "min-elements", num_str, 0, NULL), cleanup);
        }

        /* max-elements */
        if (flags & LYS_SET_MAX) {
            sprintf(num_str, "%" PRIu32, max);
            LY_CHECK_GOTO(rc = lyd_new_term(dev_list, NULL, "max-elements", num_str, 0, NULL), cleanup);
        }

        /* type */
        if (type) {
            LY_CHECK_GOTO(rc = lyd_new_inner(dev_list, NULL, "type", 0, &parent), cleanup);
            LY_CHECK_GOTO(rc = schema_diff_parsed_type(type, parent), cleanup);
        }

        /* units */
        if (units && (rc = lyd_new_term(dev_list, NULL, "units", units, 0, NULL))) {
            goto cleanup;
        }

        /* uniques */
        if (uniques) {
            LY_CHECK_GOTO(rc = lyd_new_list(dev_list, NULL, "unique", 0, &parent), cleanup);
            LY_ARRAY_FOR(uniques, u) {
                LY_CHECK_GOTO(rc = lyd_new_term(parent, NULL, "node", uniques[u].str, 0, NULL), cleanup);
            }
        }

        /* ext-instance */
        LY_ARRAY_FOR(dev->exts, u) {
            LY_CHECK_GOTO(rc = schema_diff_pext_inst(&dev->exts[u], dev_list), cleanup);
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from deviation changes of 'module'.
 *
 * @param[in] change Deviation change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_deviation(const struct lys_diff_dev_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;
    uint32_t i;

    if (!change->changes.count && !change->ext_changes.count) {
        /* no changes of this deviation */
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, mod_cmp_list), cleanup);
    for (i = 0; i < change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->ext_changes.changes[i].changes, mod_cmp_list), cleanup);
    }

    if (change->dev_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_deviation(change->dev_old, cont), cleanup);
    }

    if (change->dev_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_deviation(change->dev_new, cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from direct extension-instances of 'module'.
 *
 * @param[in] change Ext-instance change to use.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module_ext_inst(const struct lys_diff_ext_change_s *change, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *mod_cmp_list, *cont;

    if (!change->changes.count) {
        /* no changes of this ext-instance */
        goto cleanup;
    }

    /* module comparison */
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "module-comparison", 0, &mod_cmp_list), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&change->changes, mod_cmp_list), cleanup);

    if (change->ext_old) {
        /* old */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "old", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(change->ext_old, 1, cont), cleanup);
    }

    if (change->ext_new) {
        /* new */
        LY_CHECK_GOTO(rc = lyd_new_inner(mod_cmp_list, NULL, "new", 0, &cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(change->ext_new, 1, cont), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from 'module' changes.
 *
 * @param[in] diff Diff to use.
 * @param[in] mod1 First module.
 * @param[in] mod2 Second module.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_module(const struct lys_diff_s *diff, const struct lys_module *mod1, const struct lys_module *mod2,
        struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    uint32_t i;

    /* yang-version and text substmts */
    for (i = 0; i < diff->module_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_module_substmt(&diff->module_changes.changes[i], mod1, mod2, diff->with_parsed,
                diff_list), cleanup);
    }

    /* identities */
    for (i = 0; i < diff->ident_change_count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_module_ident(&diff->ident_changes[i], diff->with_parsed, diff_list), cleanup);
    }

    if (diff->with_parsed) {
        /* imports */
        for (i = 0; i < diff->import_change_count; ++i) {
            LY_CHECK_GOTO(rc = schema_diff_module_import(&diff->import_changes[i], diff_list), cleanup);
        }

        /* includes */
        for (i = 0; i < diff->include_change_count; ++i) {
            LY_CHECK_GOTO(rc = schema_diff_module_include(&diff->include_changes[i], diff_list), cleanup);
        }

        /* extensions */
        for (i = 0; i < diff->extension_change_count; ++i) {
            LY_CHECK_GOTO(rc = schema_diff_module_extension(&diff->extension_changes[i], diff_list), cleanup);
        }

        /* features */
        for (i = 0; i < diff->feat_change_count; ++i) {
            LY_CHECK_GOTO(rc = schema_diff_module_feature(&diff->feat_changes[i], diff_list), cleanup);
        }

        /* deviations */
        for (i = 0; i < diff->dev_change_count; ++i) {
            LY_CHECK_GOTO(rc = schema_diff_module_deviation(&diff->dev_changes[i], diff_list), cleanup);
        }
    }

    /* extension-instances */
    for (i = 0; i < diff->mod_ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_module_ext_inst(&diff->mod_ext_changes.changes[i], diff_list), cleanup);
    }

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
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&musts[u].exts[v], 0, must_list), cleanup);
        }
    }

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
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&whens[u]->exts[v], 0, when_list), cleanup);
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
    char num_str[22];

    LY_ARRAY_FOR(range->parts, u) {
        /* interval */
        LY_CHECK_GOTO(rc = lyd_new_list(parent, NULL, "interval", 0, &interval_list), cleanup);
        if (is_signed) {
            sprintf(num_str, "%" PRId64, range->parts[u].min_64);
            LY_CHECK_GOTO(rc = lyd_new_term(interval_list, NULL, "min", num_str, 0, NULL), cleanup);
            sprintf(num_str, "%" PRId64, range->parts[u].max_64);
            LY_CHECK_GOTO(rc = lyd_new_term(interval_list, NULL, "max", num_str, 0, NULL), cleanup);
        } else {
            sprintf(num_str, "%" PRIu64, range->parts[u].min_u64);
            LY_CHECK_GOTO(rc = lyd_new_term(interval_list, NULL, "min", num_str, 0, NULL), cleanup);
            sprintf(num_str, "%" PRIu64, range->parts[u].max_u64);
            LY_CHECK_GOTO(rc = lyd_new_term(interval_list, NULL, "max", num_str, 0, NULL), cleanup);
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
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&range->exts[u], 0, parent), cleanup);
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
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&patterns[u]->exts[v], 0, pat_list), cleanup);
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
    char num_str[22];

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
            sprintf(num_str, "%" PRId32, items[u].value);
            LY_CHECK_GOTO(rc = lyd_new_term(par_list, NULL, "value", num_str, 0, NULL), cleanup);
        } else {
            sprintf(num_str, "%" PRIu32, items[u].position);
            LY_CHECK_GOTO(rc = lyd_new_term(par_list, NULL, "position", num_str, 0, NULL), cleanup);
        }

        /* status */
        LY_CHECK_GOTO(rc = schema_diff_status(items[u].flags, par_list), cleanup);

        /* ext-instance */
        LY_ARRAY_FOR(items[u].exts, v) {
            LY_CHECK_GOTO(rc = schema_diff_ext_inst(&items[u].exts[v], 0, par_list), cleanup);
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
    char num_str[4];

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
        sprintf(num_str, "%" PRIu8, type_dec->fraction_digits);
        LY_CHECK_GOTO(rc = lyd_new_term(type_par, NULL, "fraction-digits", num_str, 0, NULL), cleanup);

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
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&type->exts[u], 0, type_par), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief Create cmp YANG data from a node.
 *
 * @param[in] node Node to use.
 * @param[in] with_priv_parsed Whether LY_CTX_SET_PRIV_PARSED ctx option is set.
 * @param[in,out] change_cont Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node_stmts(const struct lysc_node *node, ly_bool with_priv_parsed, struct lyd_node *change_cont)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    const char *config = NULL, *mandatory = NULL, *ordered_by = NULL;
    uint32_t min, max;
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lysc_node_list *list;
    const struct lysp_node *p_node;
    struct lyd_node *unique_list, *type_cont;
    char num_str[11];

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

    if (with_priv_parsed) {
        p_node = node->priv;

        /* if-features */
        LY_ARRAY_FOR(p_node->iffeatures, u) {
            LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "if-feature", p_node->iffeatures[u].str, 0, NULL), cleanup);
        }
    }

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
    if (min > 0) {
        sprintf(num_str, "%" PRIu32, min);
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "min-elements", num_str, 0, NULL), cleanup);
    }
    if (max < UINT32_MAX) {
        sprintf(num_str, "%" PRIu32, max);
        LY_CHECK_GOTO(rc = lyd_new_term(change_cont, NULL, "max-elements", num_str, 0, NULL), cleanup);
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
        LY_CHECK_GOTO(rc = schema_diff_ext_inst(&node->exts[u], 0, change_cont), cleanup);
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
 * @param[in] with_priv_parsed Whether LY_CTX_SET_PRIV_PARSED ctx option is set.
 * @param[in,out] diff_list Node to append to.
 * @return LY_ERR value.
 */
static LY_ERR
schema_diff_node(const struct lys_diff_node_change_s *node_change, ly_bool with_priv_parsed, struct lyd_node *diff_list)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *node_diff_list, *change_cont;
    const struct lysc_node *node;
    char *path = NULL;
    uint32_t i;

    assert(node_change->snode_old || node_change->snode_new);

    if (!node_change->changes.count && !node_change->ext_changes.count) {
        /* no changes */
        goto cleanup;
    }

    node = node_change->snode_old ? node_change->snode_old : node_change->snode_new;

    /* do not report schema-only nodes */
    if (node->nodetype & (LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)) {
        goto cleanup;
    }

    /* list instance with its key */
    path = lysc_path(node, LYSC_PATH_LOG, NULL, 0);
    if (!path) {
        rc = LY_EMEM;
        goto cleanup;
    }
    LY_CHECK_GOTO(rc = lyd_new_list(diff_list, NULL, "node-comparison", 0, &node_diff_list, path), cleanup);

    /* node-type */
    LY_CHECK_GOTO(rc = lyd_new_term(node_diff_list, NULL, "node-type", schema_diff_nodetype2enum(node->nodetype), 0,
            NULL), cleanup);

    /* change info */
    LY_CHECK_GOTO(rc = schema_diff_changes_info(&node_change->changes, node_diff_list), cleanup);
    for (i = 0; i < node_change->ext_changes.count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_changes_info(&node_change->ext_changes.changes[i].changes, node_diff_list), cleanup);
    }

    /* old */
    if (node_change->snode_old) {
        LY_CHECK_GOTO(rc = lyd_new_inner(node_diff_list, NULL, "old", 0, &change_cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_node_stmts(node_change->snode_old, with_priv_parsed, change_cont), cleanup);
    }

    /* new */
    if (node_change->snode_new) {
        LY_CHECK_GOTO(rc = lyd_new_inner(node_diff_list, NULL, "new", 0, &change_cont), cleanup);
        LY_CHECK_GOTO(rc = schema_diff_node_stmts(node_change->snode_new, with_priv_parsed, change_cont), cleanup);
    }

cleanup:
    free(path);
    return rc;
}

LY_ERR
lysc_diff_tree(const struct lys_module *mod1, const struct lys_module *mod2, const struct lys_diff_s *diff,
        const struct lys_module *cmp_mod, struct lyd_node **schema_diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *diff_cont = NULL, *diff_list, *mod_cont;
    const struct lysc_node *imp_schema;
    uint32_t i;

    /* structure extension */
    LY_CHECK_GOTO(rc = lyd_new_inner(NULL, cmp_mod, "schema-comparison", 0, &diff_cont), cleanup);
    LY_CHECK_GOTO(rc = lyd_new_list(diff_cont, NULL, "schema", 0, &diff_list), cleanup);

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
    LY_CHECK_GOTO(rc = lyd_new_term(diff_list, NULL, "conformance", schema_diff_conform2str(diff->conform), 0, NULL),
            cleanup);

    /* module comparison */
    LY_CHECK_GOTO(rc = schema_diff_module(diff, mod1, mod2, diff_list), cleanup);

    /* parsed comparison */
    if (diff->with_parsed) {
        LY_CHECK_GOTO(rc = schema_diff_parsed(diff, diff_list), cleanup);
    }

    /* node comparison */
    for (i = 0; i < diff->node_change_count; ++i) {
        LY_CHECK_GOTO(rc = schema_diff_node(&diff->node_changes[i], diff->with_priv_parsed, diff_list), cleanup);
    }

cleanup:
    if (rc) {
        lyd_free_tree(diff_cont);
    } else {
        *schema_diff = diff_cont;
    }
    return rc;
}
