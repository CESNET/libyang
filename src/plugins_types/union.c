/**
 * @file union.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in union type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesUnion union (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 4 | yes | `uint32_t *` | index of the resolved type in ::lysc_type_union.types |
 * | exact same format as the resolved type ||||
 *
 * Note that loading union value in this format prevents it from changing its real (resolved) type.
 */

/**
 * @brief Size in bytes of the index in the LYB Binary Format.
 */
#define IDX_SIZE 4

/**
 * @brief Assign a value to the union subvalue.
 *
 * @param[in] value Value for assignment.
 * @param[in] value_len Length of the @p value.
 * @param[out] original Destination item of the subvalue.
 * @param[out] orig_len Length of the @p original.
 * @param[in,out] options Flag containing LYPLG_TYPE_STORE_DYNAMIC.
 * @return LY_ERR value.
 */
static LY_ERR
union_subvalue_assignment(const void *value, size_t value_len,
        void **original, size_t *orig_len, uint32_t *options)
{
    LY_ERR ret = LY_SUCCESS;

    if (*options & LYPLG_TYPE_STORE_DYNAMIC) {
        /* The allocated value is stored and spend. */
        *original = (void *)value;
        *options &= ~LYPLG_TYPE_STORE_DYNAMIC;
    } else if (value_len) {
        /* Make copy of the value. */
        *original = calloc(1, value_len);
        LY_CHECK_ERR_RET(!*original, ret = LY_EMEM, ret);
        memcpy(*original, value, value_len);
    } else {
        /* Empty value. */
        *original = strdup("");
        LY_CHECK_ERR_RET(!*original, ret = LY_EMEM, ret);
    }
    *orig_len = value_len;

    return ret;
}

/**
 * @brief Validate LYB Binary Format.
 *
 * @param[in] lyb_data Source of LYB data to parse.
 * @param[in] lyb_data_len Length of @p lyb_data.
 * @param[in] type_u Compiled type of union.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_union_validate(const void *lyb_data, size_t lyb_data_len,
        const struct lysc_type_union *type_u, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t type_idx;

    /* Basic validation. */
    if (lyb_data_len < IDX_SIZE) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid LYB union value size %zu (expected at least 4).",
                lyb_data_len);
        return ret;
    }

    /* Get index. */
    type_idx = *(uint32_t *)lyb_data;
    if (type_idx >= LY_ARRAY_COUNT(type_u->types)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid LYB union type index %" PRIu32
                " (type count %" LY_PRI_ARRAY_COUNT_TYPE ").",
                type_idx, LY_ARRAY_COUNT(type_u->types));
        return ret;
    }

    return ret;
}

/**
 * @brief Parse index and lyb_value from LYB Binary Format.
 *
 * @param[in] lyb_data Source of LYB data to parse.
 * @param[in] lyb_data_len Length of @p lyb_data.
 * @param[out] type_idx Index of the union type.
 * @param[out] lyb_value Value after index number. If there is no value
 * after the index, it is set to empty string ("").
 * @param[out] lyb_value_len Length of @p lyb_value.
 */
static void
lyb_parse_union(const void *lyb_data, size_t lyb_data_len,
        uint32_t *type_idx, const void **lyb_value, size_t *lyb_value_len)
{
    assert(lyb_data && !(lyb_value && !lyb_value_len));

    if (type_idx) {
        *type_idx = *(uint32_t *)lyb_data;
    }

    if (lyb_value && lyb_value_len && lyb_data_len) {
        /* Get lyb_value and its length. */
        if (lyb_data_len == IDX_SIZE) {
            *lyb_value_len = 0;
            *lyb_value = "";
        } else {
            *lyb_value_len = lyb_data_len - IDX_SIZE;
            *lyb_value = lyb_data + IDX_SIZE;
        }
    }
}

/**
 * @brief Store subvalue as a specific type.
 *
 * @param[in] ctx libyang context.
 * @param[in] type Specific union type to use for storing.
 * @param[in] subvalue Union subvalue structure.
 * @param[in] resolve Whether the value needs to be resolved (validated by a callback).
 * @param[in] ctx_node Context node for prefix resolution.
 * @param[in] tree Data tree for resolving (validation).
 * @param[in,out] unres Global unres structure.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
union_store_type(const struct ly_ctx *ctx, struct lysc_type *type, struct lyd_value_union *subvalue,
        ly_bool resolve, const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lys_glob_unres *unres,
        struct ly_err_item **err)
{
    LY_ERR ret;
    const void *value = NULL;
    size_t value_len = 0;

    if (subvalue->format == LY_VALUE_LYB) {
        lyb_parse_union(subvalue->original, subvalue->orig_len, NULL, &value, &value_len);
    } else {
        value = subvalue->original;
        value_len = subvalue->orig_len;
    }

    ret = type->plugin->store(ctx, type, value, value_len, 0, subvalue->format, subvalue->prefix_data, subvalue->hints,
            subvalue->ctx_node, &subvalue->value, unres, err);
    if ((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE)) {
        /* clear any leftover/freed garbage */
        memset(&subvalue->value, 0, sizeof subvalue->value);
        return ret;
    }

    if (resolve && (ret == LY_EINCOMPLETE)) {
        /* we need the value resolved */
        ret = subvalue->value.realtype->plugin->validate(ctx, type, ctx_node, tree, &subvalue->value, err);
        if (ret) {
            /* resolve failed, we need to free the stored value */
            type->plugin->free(ctx, &subvalue->value);
        }
    }

    return ret;
}

/**
 * @brief Find the first valid type for a union value.
 *
 * @param[in] ctx libyang context.
 * @param[in] types Sized array of union types.
 * @param[in] subvalue Union subvalue structure.
 * @param[in] resolve Whether the value needs to be resolved (validated by a callback).
 * @param[in] ctx_node Context node for prefix resolution.
 * @param[in] tree Data tree for resolving (validation).
 * @param[out] type_idx Index of the type in which the value was stored.
 * @param[in,out] unres Global unres structure.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
union_find_type(const struct ly_ctx *ctx, struct lysc_type **types, struct lyd_value_union *subvalue,
        ly_bool resolve, const struct lyd_node *ctx_node, const struct lyd_node *tree, uint32_t *type_idx,
        struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    uint32_t prev_lo;

    if (!types || !LY_ARRAY_COUNT(types)) {
        return LY_EINVAL;
    }

    *err = NULL;

    /* turn logging off */
    prev_lo = ly_log_options(0);

    /* use the first usable subtype to store the value */
    for (u = 0; u < LY_ARRAY_COUNT(types); ++u) {
        ret = union_store_type(ctx, types[u], subvalue, resolve, ctx_node, tree, unres, err);
        if ((ret == LY_SUCCESS) || (ret == LY_EINCOMPLETE)) {
            break;
        }

        ly_err_free(*err);
        *err = NULL;
    }

    if (u == LY_ARRAY_COUNT(types)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid union value \"%.*s\" - no matching subtype found.",
                (int)subvalue->orig_len, (char *)subvalue->original);
    } else if (type_idx) {
        *type_idx = u;
    }

    /* restore logging */
    ly_log_options(prev_lo);
    return ret;
}

/**
 * @brief Fill union subvalue items: original, origin_len, format
 * prefix_data and call 'store' function for value.
 *
 * @param[in] ctx libyang context.
 * @param[in] type_u Compiled type of union.
 * @param[in] lyb_data Input LYB data consisting of index followed by
 * value (lyb_value).
 * @param[in] lyb_data_len Length of @p lyb_data.
 * @param[in] prefix_data Format-specific data for resolving any
 * prefixes (see ly_resolve_prefix()).
 * @param[in,out] subvalue Union subvalue to be filled.
 * @param[in,out] options Option containing LYPLG_TYPE_STORE_DYNAMIC.
 * @param[in,out] unres Global unres structure for newly implemented
 * modules.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_fill_subvalue(const struct ly_ctx *ctx, struct lysc_type_union *type_u,
        const void *lyb_data, size_t lyb_data_len, void *prefix_data,
        struct lyd_value_union *subvalue, uint32_t *options,
        struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret;
    uint32_t type_idx;
    const void *lyb_value = NULL;
    size_t lyb_value_len = 0;

    ret = lyb_union_validate(lyb_data, lyb_data_len, type_u, err);
    LY_CHECK_RET(ret);

    /* Parse lyb_data and set the lyb_value and lyb_value_len. */
    lyb_parse_union(lyb_data, lyb_data_len, &type_idx, &lyb_value, &lyb_value_len);
    LY_CHECK_RET(ret);

    /* Store lyb_data to subvalue. */
    ret = union_subvalue_assignment(lyb_data, lyb_data_len,
            &subvalue->original, &subvalue->orig_len, options);
    LY_CHECK_RET(ret);

    if (lyb_value) {
        /* Resolve prefix_data and set format. */
        ret = lyplg_type_prefix_data_new(ctx, lyb_value, lyb_value_len,
                LY_VALUE_LYB, prefix_data, &subvalue->format, &subvalue->prefix_data);
        LY_CHECK_RET(ret);
        assert(subvalue->format == LY_VALUE_LYB);
    } else {
        /* The lyb_parse_union() did not find lyb_value.
         * Just set format.
         */
        subvalue->format = LY_VALUE_LYB;
    }

    /* Use the specific type to store the value. */
    ret = union_store_type(ctx, type_u->types[type_idx], subvalue, 0, NULL, NULL, unres, err);

    return ret;
}

API LY_ERR
lyplg_type_store_union(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)type;
    struct lyd_value_union *subvalue;

    *err = NULL;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, subvalue);
    LY_CHECK_ERR_GOTO(!subvalue, ret = LY_EMEM, cleanup);
    storage->realtype = type;
    subvalue->hints = hints;
    subvalue->ctx_node = ctx_node;

    if (format == LY_VALUE_LYB) {
        ret = lyb_fill_subvalue(ctx, type_u, value, value_len,
                prefix_data, subvalue, &options, unres, err);
        LY_CHECK_GOTO((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE), cleanup);
    } else {
        /* Store @p value to subvalue. */
        ret = union_subvalue_assignment(value, value_len,
                &subvalue->original, &subvalue->orig_len, &options);
        LY_CHECK_GOTO(ret, cleanup);

        /* store format-specific data for later prefix resolution */
        ret = lyplg_type_prefix_data_new(ctx, value, value_len, format, prefix_data, &subvalue->format,
                &subvalue->prefix_data);
        LY_CHECK_GOTO(ret, cleanup);

        /* use the first usable subtype to store the value */
        ret = union_find_type(ctx, type_u->types, subvalue, 0, NULL, NULL, NULL, unres, err);
        LY_CHECK_GOTO((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE), cleanup);
    }

    /* store canonical value, if any (use the specific type value) */
    ret = lydict_insert(ctx, subvalue->value._canonical, 0, &storage->_canonical);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if ((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE)) {
        lyplg_type_free_union(ctx, storage);
    }
    return ret;
}

API LY_ERR
lyplg_type_validate_union(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)storage->realtype;
    struct lyd_value_union *subvalue = storage->subvalue;
    uint32_t type_idx;

    *err = NULL;

    if (!subvalue->value.realtype->plugin->validate) {
        /* nothing to resolve */
        return LY_SUCCESS;
    }

    /* resolve the stored value */
    if (!subvalue->value.realtype->plugin->validate(ctx, type, ctx_node, tree, &subvalue->value, err)) {
        /* resolve successful */
        return LY_SUCCESS;
    }

    /* Resolve failed, we have to try another subtype of the union.
     * Unfortunately, since the realtype can change (e.g. in leafref), we are not able to detect
     * which of the subtype's were tried the last time, so we have to try all of them again.
     */
    ly_err_free(*err);
    *err = NULL;

    if (subvalue->format == LY_VALUE_LYB) {
        /* use the specific type to store the value */
        lyb_parse_union(subvalue->original, 0, &type_idx, NULL, NULL);
        ret = union_store_type(ctx, type_u->types[type_idx], subvalue, 1, ctx_node, tree, NULL, err);
        LY_CHECK_RET(ret);
    } else {
        /* use the first usable subtype to store the value */
        ret = union_find_type(ctx, type_u->types, subvalue, 1, ctx_node, tree, NULL, NULL, err);
        LY_CHECK_RET(ret);
    }

    /* store and resolve the value */
    ret = union_find_type(ctx, type_u->types, subvalue, 1, ctx_node, tree, NULL, NULL, err);
    LY_CHECK_RET(ret);

    /* success, update the canonical value, if any generated */
    lydict_remove(ctx, storage->_canonical);
    LY_CHECK_RET(lydict_insert(ctx, subvalue->value._canonical, 0, &storage->_canonical));
    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_compare_union(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1->subvalue->value.realtype != val2->subvalue->value.realtype) {
        return LY_ENOT;
    }
    return val1->subvalue->value.realtype->plugin->compare(&val1->subvalue->value, &val2->subvalue->value);
}

/**
 * @brief Create LYB data for printing.
 *
 * @param[in] ctx libyang context.
 * @param[in] type_u Compiled type of union.
 * @param[in] subvalue Union value.
 * @param[in] prefix_data Format-specific data for resolving any
 * prefixes (see ly_resolve_prefix()).
 * @param[out] value_len Length of returned data.
 * @return Pointer to created LYB data. Caller must release.
 * @return NULL in case of error.
 */
static const void *
lyb_union_print(const struct ly_ctx *ctx, struct lysc_type_union *type_u,
        struct lyd_value_union *subvalue, void *prefix_data, size_t *value_len)
{
    void *ret = NULL;
    LY_ERR retval;
    struct ly_err_item *err;
    uint32_t type_idx;
    ly_bool dynamic;
    size_t pval_len;
    void *pval;

    /* Find out the index number (type_idx). The call should succeed
     * because the union_find_type() has already been called in the
     * lyplg_type_store_union().
     */
    if (!ctx) {
        assert(subvalue->ctx_node);
        ctx = subvalue->ctx_node->module->ctx;
    }
    subvalue->value.realtype->plugin->free(ctx, &subvalue->value);
    retval = union_find_type(ctx, type_u->types, subvalue,
            0, NULL, NULL, &type_idx, NULL, &err);
    LY_CHECK_RET((retval != LY_SUCCESS) && (retval != LY_EINCOMPLETE), NULL);

    /* Print subvalue in LYB format. */
    pval = (void *)subvalue->value.realtype->plugin->print(NULL,
            &subvalue->value, LY_VALUE_LYB, prefix_data, &dynamic,
            &pval_len);
    LY_CHECK_RET(!pval, NULL);

    /* Create LYB data. */
    *value_len = IDX_SIZE + pval_len;
    ret = malloc(*value_len);
    LY_CHECK_RET(!ret, NULL);
    memcpy(ret, &type_idx, IDX_SIZE);
    memcpy(ret + IDX_SIZE, pval, pval_len);

    if (dynamic) {
        free(pval);
    }

    return ret;
}

API const void *
lyplg_type_print_union(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, size_t *value_len)
{
    const void *ret;
    struct lyd_value_union *subvalue = value->subvalue;
    struct lysc_type_union *type_u = (struct lysc_type_union *)value->realtype;
    size_t lyb_data_len = 0;

    if ((format == LY_VALUE_LYB) && (subvalue->format == LY_VALUE_LYB)) {
        /* The return value is already ready. */
        *dynamic = 0;
        if (value_len) {
            *value_len = subvalue->orig_len;
        }
        return subvalue->original;
    } else if ((format == LY_VALUE_LYB) && (subvalue->format != LY_VALUE_LYB)) {
        /* The return LYB data must be created. */
        *dynamic = 1;
        ret = lyb_union_print(ctx, type_u, subvalue, prefix_data, &lyb_data_len);
        if (value_len) {
            *value_len = lyb_data_len;
        }
        return ret;
    }

    assert(format != LY_VALUE_LYB);
    ret = (void *)subvalue->value.realtype->plugin->print(ctx,
            &subvalue->value, format, prefix_data, dynamic, value_len);
    if (!value->_canonical && (format == LY_VALUE_CANON)) {
        /* the canonical value is supposed to be stored now */
        lydict_insert(ctx, subvalue->value._canonical, 0, (const char **)&value->_canonical);
    }

    return ret;
}

API LY_ERR
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

    ret = orig_val->value.realtype->plugin->duplicate(ctx, &orig_val->value, &dup_val->value);
    LY_CHECK_GOTO(ret, cleanup);

    if (orig_val->orig_len) {
        dup_val->original = calloc(1, orig_val->orig_len);
        LY_CHECK_ERR_GOTO(!dup_val->original, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        memcpy(dup_val->original, orig_val->original, orig_val->orig_len);
    } else {
        dup_val->original = strdup("");
        LY_CHECK_ERR_GOTO(!dup_val->original, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    }
    dup_val->orig_len = orig_val->orig_len;

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

API void
lyplg_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_union *val;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
    LYD_VALUE_GET(value, val);
    if (val) {
        if (val->value.realtype) {
            val->value.realtype->plugin->free(ctx, &val->value);
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

        .plugin.id = "libyang 2 - union,version 1",
        .plugin.store = lyplg_type_store_union,
        .plugin.validate = lyplg_type_validate_union,
        .plugin.compare = lyplg_type_compare_union,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_union,
        .plugin.duplicate = lyplg_type_dup_union,
        .plugin.free = lyplg_type_free_union,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
