/**
 * @file union.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko
 * @brief Built-in union type plugin.
 *
 * Copyright (c) 2019 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* strdup */

#include "plugins_types.h"

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "dict.h"
#include "ly_array.h"
#include "ly_common.h"
#include "lyb.h"
#include "plugins_internal.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesUnion union (built-in)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 8 | yes | `uint32_t *` | little-endian index of the resolved type in ::lysc_type_union.types |
 * | exact same format as the resolved type ||||
 *
 * Note that loading union value in this format prevents it from changing its real (resolved) type.
 */

static void lyplg_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Size in bytes of the used type index in the LYB Binary Format.
 */
#define LYPLG_UNION_TYPE_IDX_SIZE 1

/**
 * @brief Validate LYB union value.
 *
 * @param[in] lyb_data Source of LYB data to parse.
 * @param[in] lyb_data_size_bits Size of @p lyb_data in bits.
 * @param[in] type_u Compiled type of union.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
union_validate_value_lyb(const void *lyb_data, uint64_t lyb_data_size_bits, const struct lysc_type_union *type_u,
        struct ly_err_item **err)
{
    uint32_t type_idx = 0;

    /* basic validation */
    if (lyb_data_size_bits < LYPLG_UNION_TYPE_IDX_SIZE * 8) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB union value size %" PRIu64
                " b (expected at least %" PRIu8 " b).", lyb_data_size_bits, LYPLG_UNION_TYPE_IDX_SIZE * 8);
    }

    /* get index in correct byte order */
    memcpy(&type_idx, lyb_data, LYPLG_UNION_TYPE_IDX_SIZE);
    type_idx = le32toh(type_idx);
    if (type_idx >= LYA_COUNT(type_u->types)) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid LYB union type index %" PRIu32 " (type count %" LYA_PRI_COUNT_T ").",
                type_idx, LYA_COUNT(type_u->types));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse index and specific value from LYB union value.
 *
 * @param[in] lyb_data Source of LYB data to parse.
 * @param[in] lyb_data_size_bits Size of @p lyb_data in bits.
 * @param[out] type_idx Index of the union type.
 * @param[out] lyb_value Value after index number. If there is no value
 * after the index, it is set to empty string ("").
 * @param[out] lyb_value_size_bits Size of @p lyb_value in bits.
 */
static void
union_parse_value_lyb(const void *lyb_data, uint64_t lyb_data_size_bits, uint32_t *type_idx, const void **lyb_value,
        uint64_t *lyb_value_size_bits)
{
    uint32_t num = 0;

    assert(lyb_data);

    if (type_idx) {
        memcpy(&num, lyb_data, LYPLG_UNION_TYPE_IDX_SIZE);
        num = le32toh(num);

        *type_idx = num;
    }

    if (lyb_value && lyb_value_size_bits && lyb_data_size_bits) {
        /* get lyb_value and its size */
        if (lyb_data_size_bits == LYPLG_UNION_TYPE_IDX_SIZE * 8) {
            *lyb_value_size_bits = 0;
            *lyb_value = "";
        } else {
            *lyb_value_size_bits = lyb_data_size_bits - LYPLG_UNION_TYPE_IDX_SIZE * 8;
            *lyb_value = (char *)lyb_data + LYPLG_UNION_TYPE_IDX_SIZE;
        }
    }
}

/**
 * @brief Create LYB binary union data for a value of a given member type: the pinned member type
 * index followed by the binary member type value.
 *
 * @param[in] ctx libyang context.
 * @param[in] type_idx Index of the member type of @p member_value, is pinned in the binary data.
 * @param[in] value Stored member type value.
 * @param[out] lyb_value Printed union LYB value.
 * @param[out] lyb_size_bits Optional, size of @p lyb_value.
 * @return LY_ERR value.
 */
static LY_ERR
union_create_value_lyb(const struct ly_ctx *ctx, uint32_t type_idx, const struct lyd_value *value, void **lyb_value,
        uint64_t *lyb_size_bits)
{
    LY_ERR rc = LY_SUCCESS;
    void *lyb = NULL;
    const void *pval = NULL;
    uint64_t pval_size_bits, lyb_bits = 0;
    uint32_t num;
    ly_bool dynamic = 0;

    /* print the value in LYB format */
    pval = LYSC_GET_TYPE_PLG(value->realtype->plugin_ref)->print(ctx, value, LY_VALUE_LYB, NULL, &dynamic, &pval_size_bits);
    if (!pval) {
        rc = LY_EMEM;
        goto cleanup;
    }

    /* create the binary data of the pinned member type index followed by its binary value
     * (the size may not be an exact number of bytes, e.g. for integers) */
    lyb_bits = LYPLG_UNION_TYPE_IDX_SIZE * 8 + pval_size_bits;
    lyb = malloc(LYPLG_BITS2BYTES(lyb_bits));
    if (!lyb) {
        lyb_bits = 0;
        rc = LY_EMEM;
        goto cleanup;
    }

    num = htole32(type_idx);
    memcpy(lyb, &num, LYPLG_UNION_TYPE_IDX_SIZE);
    memcpy((char *)lyb + LYPLG_UNION_TYPE_IDX_SIZE, pval, LYPLG_BITS2BYTES(pval_size_bits));

cleanup:
    if (dynamic) {
        free((void *)pval);
    }
    *lyb_value = lyb;
    if (lyb_size_bits) {
        *lyb_size_bits = lyb_bits;
    }
    return rc;
}

/**
 * @brief For leafref failures, ensure the appropriate error is propagated, not a type validation failure.
 *
 * RFC7950 Section 15.5 defines the appropriate error app tag of "require-instance".
 *
 * @param[in,out] err Error record to be updated.
 * @param[in] type Leafref type used to extract target path.
 * @param[in] value Value attempted to be stored.
 * @param[in] value_size_bits Size of @p value in bits.
 * @return LY_ERR value. Only possible errors are LY_SUCCESS and LY_EMEM.
 */
static LY_ERR
union_update_lref_err(struct ly_err_item *err, const struct lysc_type *type, const void *value, uint64_t value_size_bits)
{
    const struct lysc_type_leafref *lref;
    char *valstr = NULL;
    int r;

    if (!err || (type->basetype != LY_TYPE_LEAFREF)) {
        /* nothing to do */
        return LY_SUCCESS;
    }

    lref = (const struct lysc_type_leafref *)type;

    /* update error-app-tag */
    free(err->apptag);
    err->apptag = strdup("instance-required");
    LY_CHECK_ERR_RET(!err->apptag, LOGMEM(NULL), LY_EMEM);

    valstr = strndup((const char *)value, value_size_bits / 8);
    LY_CHECK_ERR_RET(!valstr, LOGMEM(NULL), LY_EMEM);

    /* update error-message */
    free(err->msg);
    r = asprintf(&err->msg, LY_ERRMSG_NOLREF_VAL, valstr, lyxp_get_expr(lref->path));
    free(valstr);
    LY_CHECK_ERR_RET(r == -1, LOGMEM(NULL), LY_EMEM);

    return LY_SUCCESS;
}

/**
 * @brief Get the input value suitable for a type from the original value in a subvalue.
 *
 * @param[in] ctx Context to use.
 * @param[in] type_u Union type to use.
 * @param[in] type_idx Index of type in @p type_u of the requested input value.
 * @param[in] subvalue Union subvalue to use.
 * @param[out] value Input value.
 * @param[out] value_size_bits Size of @p value in bits.
 * @param[out] dynamic Set if @p value is dynamic.
 * @param[out] format Format of @p value.
 * @param[out] prefix_data Prefix data of @p value.
 * @param[out] err Error information on error.
 * @return LY_SUCCESS on success;
 * @return LY_EINCOMPLETE when the value was stored but needs validation;
 * @return LY_ERR value on error.
 */
static LY_ERR
union_subvalue_get_input_value(const struct ly_ctx *ctx, const struct lysc_type_union *type_u, uint32_t type_idx,
        const struct lyd_value_union *subvalue, const void **value, uint64_t *value_size_bits, ly_bool *dynamic,
        LY_VALUE_FORMAT *format, void **prefix_data, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_value storage = {0};
    uint32_t ti;
    struct lyplg_type *type_plg;

    *value = NULL;
    *value_size_bits = 0;
    *dynamic = 0;
    *format = 0;
    *prefix_data = NULL;
    *err = NULL;

    if (subvalue->format == LY_VALUE_LYB) {
        union_parse_value_lyb(subvalue->original, subvalue->orig_size_bits, &ti, value, value_size_bits);
        if (ti != type_idx) {
            /* value of another type, store the value */
            type_plg = LYSC_GET_TYPE_PLG(type_u->types[ti]->plugin_ref);
            rc = type_plg->store(ctx, type_u->types[ti], *value, *value_size_bits, LYPLG_TYPE_STORE_ONLY,
                    subvalue->format, subvalue->prefix_data, subvalue->hints, subvalue->ctx_node, &storage, NULL, err);
            if (rc && (rc != LY_EINCOMPLETE)) {
                /* if this is a leafref, lets make sure we propagate the appropriate error, and not a type validation failure */
                union_update_lref_err(*err, type_u->types[ti], *value, *value_size_bits);
                goto cleanup;
            }

            /* print the value in canonical format */
            assert(storage.realtype);
            *value = LYSC_GET_TYPE_PLG(storage.realtype->plugin_ref)->print(ctx, &storage, LY_VALUE_CANON, NULL, dynamic,
                    value_size_bits);
            assert(!(*value_size_bits % 8));

            /* free storage, but we need the value, which may be stored there */
            if (!*dynamic) {
                *value = strndup(*value, *value_size_bits / 8);
                *dynamic = 1;
            }
            type_plg->free(ctx, &storage);

            *format = LY_VALUE_CANON;
            *prefix_data = NULL;
        } else {
            *format = subvalue->format;
            *prefix_data = subvalue->prefix_data;
        }
    } else {
        *value = subvalue->original;
        *value_size_bits = subvalue->orig_size_bits;
        *format = subvalue->format;
        *prefix_data = subvalue->prefix_data;
    }

cleanup:
    return rc;
}

/**
 * @brief Store (and validate) a value of a specific type.
 *
 * @param[in] ctx Context to use.
 * @param[in] type Type to use.
 * @param[in] value Value to store.
 * @param[in] value_size_bits Size of @p value in bits.
 * @param[in] options Store options.
 * @param[in] format Format of @p value.
 * @param[in] prefix_data Prefix data of @p value.
 * @param[in] hints Hints of @p value.
 * @param[in] ctx_snode Schema context node for prefix resolution.
 * @param[in] ctx_node Data context node for the validation.
 * @param[in] tree Optional data tree for validation. If NULL, validation is not performed.
 * @param[out] storage Value storage to use.
 * @param[in,out] unres Global unres structure.
 * @param[out] err Error information on error.
 * @return LY_SUCCESS on success;
 * @return LY_EINCOMPLETE when the value was stored but needs validation;
 * @return LY_ERR value on error.
 */
static LY_ERR
union_store_type(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint64_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_snode,
        const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lyd_value *storage,
        struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyplg_type *type_plg;
    ly_bool dynamic = 0;

    *err = NULL;

    type_plg = LYSC_GET_TYPE_PLG(type->plugin_ref);

    /* do not free the value (yet) */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        dynamic = 1;
    }

    rc = type_plg->store(ctx, type, value, value_size_bits, options, format, prefix_data, hints, ctx_snode, storage,
            unres, err);
    if (rc && (rc != LY_EINCOMPLETE)) {
        /* if this is a leafref, lets make sure we propagate the appropriate error, and not a type validation failure */
        union_update_lref_err(*err, type, value, value_size_bits);
        goto cleanup;
    }

    if (tree && type_plg->validate_tree) {
        /* we need the value validated in the data tree */
        rc = type_plg->validate_tree(ctx, type, ctx_node, tree, storage, err);
        if (rc) {
            /* validate failed, we need to free the stored value */
            type_plg->free(ctx, storage);
            goto cleanup;
        }
    }

cleanup:
    if (dynamic) {
        free((void *)value);
    }
    if (rc && (rc != LY_EINCOMPLETE)) {
        /* clear any leftover/freed garbage */
        memset(storage, 0, sizeof *storage);
    }
    return rc;
}

/**
 * @brief Read the original value from a union value and store it as a value of a specific union type.
 *
 * @param[in] ctx Context to use.
 * @param[in] type_u Union type to use.
 * @param[in] type_idx Index of type in @p type_u of the requested input value.
 * @param[in] subvalue Union subvalue to use.
 * @param[in] options Store options.
 * @param[in] ctx_node Data context node for the validation.
 * @param[in] tree Optional data tree for validation. If NULL, validation is not performed.
 * @param[in,out] unres Global unres structure.
 * @param[out] err Error information on error.
 * @return LY_SUCCESS on success;
 * @return LY_EINCOMPLETE when the value was stored but needs validation;
 * @return LY_ERR value on error.
 */
static LY_ERR
union_read_store_type(const struct ly_ctx *ctx, const struct lysc_type_union *type_u, uint32_t type_idx,
        struct lyd_value_union *subvalue, uint32_t options, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS, r;
    const void *value;
    uint64_t value_size_bits;
    ly_bool dynamic;
    LY_VALUE_FORMAT format;
    void *prefix_data;

    /* get the input value in a suitable format from the original value */
    r = union_subvalue_get_input_value(ctx, type_u, type_idx, subvalue, &value, &value_size_bits, &dynamic, &format,
            &prefix_data, err);
    if (r && (r != LY_EINCOMPLETE)) {
        rc = r;
        goto cleanup;
    }

    if (dynamic) {
        options |= LYPLG_TYPE_STORE_DYNAMIC;
    } else {
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
    }

    /* store the value as the specific type */
    rc = union_store_type(ctx, type_u->types[type_idx], value, value_size_bits, options, format, prefix_data,
            subvalue->hints, subvalue->ctx_node, ctx_node, tree, &subvalue->value, unres, err);

cleanup:
    return rc;
}

/**
 * @brief Find the first suitable type for a union value.
 *
 * @param[in] ctx libyang context.
 * @param[in] type_u Union type.
 * @param[in] subvalue Union subvalue structure.
 * @param[in] options The store options.
 * @param[in] ctx_node Data context node for the validation.
 * @param[in] tree Optional data tree for validation. If NULL, validation is not performed.
 * @param[out] type_idx Index of the type in which the value was stored.
 * @param[in,out] unres Global unres structure.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
union_find_type(const struct ly_ctx *ctx, struct lysc_type_union *type_u, struct lyd_value_union *subvalue,
        uint32_t options, const struct lyd_node *ctx_node, const struct lyd_node *tree, uint32_t *type_idx,
        struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS;
    LYA_COUNT_T u;
    struct ly_err_item **errs = NULL, *e;
    uint32_t *prev_lo, temp_lo = 0;
    char *msg = NULL, *err_app_tag = NULL;
    int msg_len = 0;
    ly_bool use_err_app_tag = 0;
    struct lyplg_type *type_plg;

    *err = NULL;

    /* alloc errors */
    errs = calloc(LYA_COUNT(type_u->types), sizeof *errs);
    LY_CHECK_RET(!errs, LY_EMEM);

    /* turn logging temporarily off */
    prev_lo = ly_temp_log_options(&temp_lo);

    /* use the first usable subtype to store the value */
    LYA_FOR(type_u->types, u) {
        rc = union_read_store_type(ctx, type_u, u, subvalue, options, ctx_node, tree, unres, &e);
        if (!rc || (rc == LY_EINCOMPLETE)) {
            /* success */
            break;
        }

        errs[u] = e;
    }

    if (u == LYA_COUNT(type_u->types)) {
        /* create the full error */
        if (subvalue->format == LY_VALUE_LYB) {
            msg_len = asprintf(&msg, "Invalid LYB union value - no matching subtype found:\n");
        } else {
            msg_len = asprintf(&msg, "Invalid union value \"%.*s\" - no matching subtype found:\n",
                    (int)subvalue->orig_size_bits / 8, (char *)subvalue->original);
        }
        if (msg_len == -1) {
            LY_CHECK_ERR_GOTO(!errs, rc = LY_EMEM, cleanup);
            /* for further actions in function msg_len is just 0 */
            msg_len = 0;
        }
        LYA_FOR(type_u->types, u) {
            if (!errs[u]) {
                /* no error for some reason */
                continue;
            }

            /* use an app-tag if all the types set it or set none */
            if (errs[u]->apptag) {
                if (!err_app_tag) {
                    err_app_tag = strdup(errs[u]->apptag);
                    LY_CHECK_ERR_GOTO(!err_app_tag, rc = LY_EMEM, cleanup);
                    use_err_app_tag = 1;
                } else if (strcmp(errs[u]->apptag, err_app_tag)) {
                    use_err_app_tag = 0;
                }
            }

            type_plg = LYSC_GET_TYPE_PLG(type_u->types[u]->plugin_ref);

            msg = ly_realloc(msg, msg_len + 4 + strlen(type_plg->id) + 2 + strlen(errs[u]->msg) + 2);
            LY_CHECK_ERR_GOTO(!msg, rc = LY_EMEM, cleanup);
            msg_len += sprintf(msg + msg_len, "    %s: %s\n", type_plg->id, errs[u]->msg);
        }

        if (!use_err_app_tag) {
            free(err_app_tag);
            err_app_tag = NULL;
        }
        rc = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, err_app_tag, "%s", msg);
        err_app_tag = NULL;
    } else if (type_idx) {
        *type_idx = u;
    }

cleanup:
    LYA_FOR(type_u->types, u) {
        ly_err_free(errs[u]);
    }
    free(errs);
    free(msg);
    free(err_app_tag);
    ly_temp_log_options(prev_lo);
    return rc;
}

/**
 * @brief Prepare a union value storage for storing a value: initialize the storage, assign the caller
 * value as the union subvalue original and derive its format-specific data.
 *
 * @param[in] ctx libyang context.
 * @param[in] type The union schema type.
 * @param[in] value Value to store, is always consumed.
 * @param[in] value_size_bits Size of @p value in bits.
 * @param[in,out] options Store options; may be updated by the member store.
 * @param[in] format Format of @p value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ly_resolve_prefix()).
 * @param[in] hints Bitmap of value hints.
 * @param[in] ctx_node Context node for prefix resolution.
 * @param[in,out] storage Value storage to fill.
 * @param[out] subvalue Prepared union subvalue.
 * @return LY_ERR value.
 */
static LY_ERR
union_prepare_subvalue(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        uint64_t value_size_bits, uint32_t *options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lyd_value_union **subvalue)
{
    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, *subvalue);
    LY_CHECK_RET(!*subvalue, LY_EMEM);

    storage->realtype = type;
    (*subvalue)->hints = hints;
    (*subvalue)->ctx_node = ctx_node;

    /* store the caller value as the subvalue original, it is needed for the member type store */
    if (*options & LYPLG_TYPE_STORE_DYNAMIC) {
        (*subvalue)->original = (void *)value;
        *options &= ~LYPLG_TYPE_STORE_DYNAMIC;
    } else if (value_size_bits) {
        (*subvalue)->original = calloc(1, LYPLG_BITS2BYTES(value_size_bits));
        LY_CHECK_RET(!(*subvalue)->original, LY_EMEM);
        memcpy((*subvalue)->original, value, LYPLG_BITS2BYTES(value_size_bits));
    } else {
        (*subvalue)->original = strdup("");
        LY_CHECK_RET(!(*subvalue)->original, LY_EMEM);
    }
    (*subvalue)->orig_size_bits = value_size_bits;

    /* derive format-specific data of the value, needed for the member type store;
     * the values in the LYB format must have no prefix data stored (see ly_dup_prefix_data()) */
    return lyplg_type_prefix_data_new(ctx, value, LYPLG_BITS2BYTES(value_size_bits), format,
            (format == LY_VALUE_LYB) ? NULL : prefix_data, &(*subvalue)->format, &(*subvalue)->prefix_data);
}

static LY_ERR
lyplg_type_store_union(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, uint64_t value_size_bits,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS, r;
    struct lysc_type_union *type_u;
    struct lyd_value_union *subvalue;
    uint32_t type_idx = 0;

    *err = NULL;

    type_u = (struct lysc_type_union *)type;

    /* initialize the storage and assign the caller value to the subvalue */
    rc = union_prepare_subvalue(ctx, type, value, value_size_bits, &options, format, prefix_data, hints,
            ctx_node, storage, &subvalue);
    LY_CHECK_GOTO(rc, cleanup);

    if (format == LY_VALUE_LYB) {
        /* the binary value pins the member type to use, validate it and learn its index */
        rc = union_validate_value_lyb(value, value_size_bits, type_u, err);
        LY_CHECK_GOTO(rc, cleanup);
        union_parse_value_lyb(value, 0, &type_idx, NULL, NULL);

        /* use the pinned member type to store the value, its acceptance errors are propagated */
        rc = union_read_store_type(ctx, type_u, type_idx, subvalue, options, NULL, NULL, unres, err);
        LY_CHECK_GOTO(rc && (rc != LY_EINCOMPLETE), cleanup);
    } else {
        /* use the first usable and valid subtype to store the value */
        rc = union_find_type(ctx, type_u, subvalue, options & ~LYPLG_TYPE_STORE_ONLY, NULL, NULL, NULL, unres, err);
        if (rc && (rc != LY_EINCOMPLETE) && (options & LYPLG_TYPE_STORE_ONLY)) {
            /* we tried to find the actual type by validating the value but no type matched, so try to find any type */
            ly_err_free(*err);
            *err = NULL;
            rc = union_find_type(ctx, type_u, subvalue, options, NULL, NULL, NULL, unres, err);
        }
        LY_CHECK_GOTO(rc && (rc != LY_EINCOMPLETE), cleanup);
    }

    /* store canonical value, if any (use the specific type value) */
    r = lydict_dup(ctx, subvalue->value._canonical, &storage->_canonical);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (rc && (rc != LY_EINCOMPLETE)) {
        lyplg_type_free_union(ctx, storage);
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_store_union_idx(const struct ly_ctx *ctx, const struct lysc_type *type, uint32_t type_idx,
        const void *value, uint64_t value_size_bits, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data,
        uint32_t hints, const struct lysc_node *ctx_node, struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS, r;
    struct lysc_type_union *type_u = (struct lysc_type_union *)type;
    struct lyd_value_union *subvalue;
    void *lyb;
    uint64_t lyb_size_bits;

    LY_CHECK_ARG_RET(ctx, ctx, type, type->basetype == LY_TYPE_UNION, type_idx < LYA_COUNT(type_u->types),
            !(options & LYPLG_TYPE_STORE_IMPLEMENT), storage, err, LY_EINVAL);

    memset(storage, 0, sizeof *storage);
    *err = NULL;

    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, subvalue);
    LY_CHECK_ERR_GOTO(!subvalue, rc = LY_EMEM, cleanup);

    /* store the value of the specific type */
    rc = union_store_type(ctx, type_u->types[type_idx], value, value_size_bits, options & ~LYPLG_TYPE_STORE_DYNAMIC,
            format, prefix_data, hints, ctx_node, NULL, NULL, &subvalue->value, NULL, err);
    LY_CHECK_GOTO(rc && (rc != LY_EINCOMPLETE), cleanup);

    /* create the union LYB value with the resolved type index */
    r = union_create_value_lyb(ctx, type_idx, &subvalue->value, &lyb, &lyb_size_bits);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    /* store it as the original value (hints, while not relevant for the LYB format, may be used by other formats) */
    subvalue->original = lyb;
    subvalue->orig_size_bits = lyb_size_bits;
    subvalue->hints = hints;
    subvalue->format = LY_VALUE_LYB;
    subvalue->ctx_node = ctx_node;

    /* store canonical value (use the specific type value) and realtype */
    r = lydict_dup(ctx, subvalue->value._canonical, &storage->_canonical);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    storage->realtype = type;

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (rc && (rc != LY_EINCOMPLETE)) {
        lyplg_type_free_union(ctx, storage);
        memset(storage, 0, sizeof *storage);
    }
    return rc;
}

static LY_ERR
lyplg_type_validate_tree_union(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)type;
    struct lyd_value_union *subvalue = storage->subvalue;
    struct lyd_value orig = {0};
    uint32_t type_idx;
    ly_bool validated = 0;
    struct lyplg_type *realtype_plg;

    *err = NULL;

    /* because of types that do not store their own type as realtype (leafref), we are not able to call their
     * validate callback (there is no way to get the type) but even if possible, the value may be invalid
     * for the type, so we may have to perform union value storing again from scratch, but keep a value backup */
    realtype_plg = LYSC_GET_TYPE_PLG(subvalue->value.realtype->plugin_ref);
    LY_CHECK_RET(realtype_plg->duplicate(ctx, &subvalue->value, &orig));
    realtype_plg->free(ctx, &subvalue->value);

    if (subvalue->format == LY_VALUE_LYB) {
        /* use the specific type to store and validate the value */
        union_parse_value_lyb(subvalue->original, 0, &type_idx, NULL, NULL);

        if (union_read_store_type(ctx, type_u, type_idx, subvalue, 0, ctx_node, tree, NULL, err)) {
            /* validation failed, we need to try storing the value again */
            ly_err_free(*err);
            *err = NULL;
        } else {
            validated = 1;
        }
    }

    if (!validated) {
        /* use the first usable subtype to store and validate the value */
        rc = union_find_type(ctx, type_u, subvalue, 0, ctx_node, tree, NULL, NULL, err);
        LY_CHECK_GOTO(rc, cleanup);
    }

    /* update the canonical value, if any generated */
    lydict_remove(ctx, storage->_canonical);
    storage->_canonical = NULL;
    LY_CHECK_GOTO(rc = lydict_dup(ctx, subvalue->value._canonical, &storage->_canonical), cleanup);

cleanup:
    if (rc) {
        /* restore the previous value */
        subvalue->value = orig;
    } else {
        /* free the backup value */
        LYSC_GET_TYPE_PLG(orig.realtype->plugin_ref)->free(ctx, &orig);
    }
    return rc;
}

static LY_ERR
lyplg_type_compare_union(const struct ly_ctx *ctx, const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->subvalue->value.realtype != val2->subvalue->value.realtype) {
        return LY_ENOT;
    }
    return LYSC_GET_TYPE_PLG(val1->subvalue->value.realtype->plugin_ref)->compare(ctx,
            &val1->subvalue->value, &val2->subvalue->value);
}

static int
lyplg_type_sort_union(const struct ly_ctx *ctx, const struct lyd_value *val1, const struct lyd_value *val2)
{
    int rc;
    LYA_COUNT_T u;
    struct lysc_type **types, *type;

    if (val1->subvalue->value.realtype == val2->subvalue->value.realtype) {
        return LYSC_GET_TYPE_PLG(val1->subvalue->value.realtype->plugin_ref)->sort(ctx,
                &val1->subvalue->value, &val2->subvalue->value);
    }

    /* compare according to the order of types */
    rc = 0;
    types = ((struct lysc_type_union *)val1->realtype)->types;
    LYA_FOR(types, u) {
        if (types[u]->basetype == LY_TYPE_LEAFREF) {
            type = ((struct lysc_type_leafref *)types[u])->realtype;
        } else {
            type = types[u];
        }

        if (type == val1->subvalue->value.realtype) {
            rc = 1;
            break;
        } else if (type == val2->subvalue->value.realtype) {
            rc = -1;
            break;
        }
    }
    assert(rc);

    return rc;
}

/**
 * @brief Create LYB data for printing.
 *
 * @param[in] ctx libyang context.
 * @param[in] type_u Compiled type of union.
 * @param[in] subvalue Union value.
 * @param[out] value_size_bits Size of returned data in bits.
 * @return Created LYB data.
 * @return NULL in case of error.
 */
static const void *
union_print_value_lyb(const struct ly_ctx *ctx, struct lysc_type_union *type_u, struct lyd_value_union *subvalue,
        uint64_t *value_size_bits)
{
    void *ret = NULL;
    LY_ERR r;
    struct ly_err_item *err;
    uint32_t type_idx = 0;
    struct lyd_value orig = {0};
    struct lyplg_type *realtype_plg;

    if (!ctx) {
        assert(subvalue->ctx_node);
        ctx = subvalue->ctx_node->module->ctx;
    }

    /* because of types that do not store their own type as realtype (leafref), we are not able to call their
     * validate callback (there is no way to get the type) but even if possible, the value may be invalid
     * for the type, so we may have to perform union value storing again from scratch, but keep a value backup */
    realtype_plg = LYSC_GET_TYPE_PLG(subvalue->value.realtype->plugin_ref);
    LY_CHECK_RET(realtype_plg->duplicate(ctx, &subvalue->value, &orig), NULL);
    realtype_plg->free(ctx, &subvalue->value);

    /* learn the type index, may fail if LYPLG_TYPE_STORE_ONLY has been used before */
    r = union_find_type(ctx, type_u, subvalue, 0, NULL, NULL, &type_idx, NULL, &err);
    ly_err_free(err);
    if (r && (r != LY_EINCOMPLETE)) {
        /* use LYPLG_TYPE_STORE_ONLY to manage to print the value */
        r = union_find_type(ctx, type_u, subvalue, LYPLG_TYPE_STORE_ONLY, NULL, NULL, &type_idx, NULL, &err);
        ly_err_free(err);
    }
    LY_CHECK_ERR_RET(r && (r != LY_EINCOMPLETE), subvalue->value = orig, NULL);

    /* create the LYB data of the resolved member type value */
    r = union_create_value_lyb(NULL, type_idx, &subvalue->value, &ret, value_size_bits);
    LY_CHECK_ERR_RET(r, subvalue->value = orig, NULL);

    /* restore backup (original) value */
    realtype_plg = LYSC_GET_TYPE_PLG(subvalue->value.realtype->plugin_ref);
    realtype_plg->free(ctx, &subvalue->value);
    subvalue->value = orig;
    return ret;
}

static const void *
lyplg_type_print_union(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, uint64_t *value_size_bits)
{
    const void *ret;
    struct lyd_value_union *subvalue = value->subvalue;
    struct lysc_type_union *type_u = (struct lysc_type_union *)value->realtype;
    uint64_t lyb_data_size_bits = 0;

    if ((format == LY_VALUE_LYB) && (subvalue->format == LY_VALUE_LYB)) {
        /* return value is ready */
        *dynamic = 0;
        if (value_size_bits) {
            *value_size_bits = subvalue->orig_size_bits;
        }
        return subvalue->original;
    } else if ((format == LY_VALUE_LYB) && (subvalue->format != LY_VALUE_LYB)) {
        /* return LYB data value must be created */
        *dynamic = 1;
        ret = union_print_value_lyb(ctx, type_u, subvalue, &lyb_data_size_bits);
        if (value_size_bits) {
            *value_size_bits = lyb_data_size_bits;
        }
        return ret;
    }

    assert(format != LY_VALUE_LYB);
    ret = (void *)LYSC_GET_TYPE_PLG(subvalue->value.realtype->plugin_ref)->print(ctx, &subvalue->value,
            format, prefix_data, dynamic, value_size_bits);
    if (!value->_canonical && (format == LY_VALUE_CANON)) {
        /* the canonical value is supposed to be stored now */
        lydict_dup(ctx, subvalue->value._canonical, (const char **)&value->_canonical);
    }

    return ret;
}

static LY_ERR
lyplg_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_union *orig_val = original->subvalue, *dup_val;

    /* init dup value */
    memset(dup, 0, sizeof *dup);
    dup->realtype = original->realtype;

    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, cleanup);

    dup_val = calloc(1, sizeof *dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    dup->subvalue = dup_val;

    ret = LYSC_GET_TYPE_PLG(orig_val->value.realtype->plugin_ref)->duplicate(ctx, &orig_val->value, &dup_val->value);
    LY_CHECK_GOTO(ret, cleanup);

    if (orig_val->orig_size_bits) {
        dup_val->original = calloc(1, LYPLG_BITS2BYTES(orig_val->orig_size_bits));
        LY_CHECK_ERR_GOTO(!dup_val->original, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        memcpy(dup_val->original, orig_val->original, LYPLG_BITS2BYTES(orig_val->orig_size_bits));
    } else {
        dup_val->original = strdup("");
        LY_CHECK_ERR_GOTO(!dup_val->original, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    }
    dup_val->orig_size_bits = orig_val->orig_size_bits;

    dup_val->format = orig_val->format;
    dup_val->ctx_node = orig_val->ctx_node;
    dup_val->hints = orig_val->hints;
    ret = lyplg_type_prefix_data_dup(ctx, orig_val->format, orig_val->prefix_data, &dup_val->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (ret) {
        lyplg_type_free_union(ctx, dup);
    }
    return ret;
}

static void
lyplg_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_union *val;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
    LYD_VALUE_GET(value, val);
    if (val) {
        if (val->value.realtype) {
            LYSC_GET_TYPE_PLG(val->value.realtype->plugin_ref)->free(ctx, &val->value);
        }
        lyplg_type_prefix_data_free(val->format, val->prefix_data);
        free(val->original);

        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for union type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_union[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UNION_STR,

        .plugin.id = "ly2 union",
        .plugin.lyb_size = lyplg_type_lyb_size_variable_bits,
        .plugin.store = lyplg_type_store_union,
        .plugin.validate_value = NULL,
        .plugin.validate_tree = lyplg_type_validate_tree_union,
        .plugin.compare = lyplg_type_compare_union,
        .plugin.sort = lyplg_type_sort_union,
        .plugin.print = lyplg_type_print_union,
        .plugin.duplicate = lyplg_type_dup_union,
        .plugin.free = lyplg_type_free_union,
    },
    {0}
};
