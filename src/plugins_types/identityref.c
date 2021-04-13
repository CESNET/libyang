/**
 * @file identityref.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in identityref type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins_types.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

API const char *
lyplg_type_print_identityref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic)
{
    char *result = NULL;

    *dynamic = 1;
    if (asprintf(&result, "%s:%s", lyplg_type_get_prefix(value->ident->module, format, prefix_data), value->ident->name) == -1) {
        return NULL;
    } else {
        return result;
    }
}

API LY_ERR
lyplg_type_store_identityref(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_identityref *type_ident = (struct lysc_type_identityref *)type;
    const char *id_name, *prefix = value;
    size_t id_len, prefix_len, str_len;
    char *str;
    const struct lys_module *mod = NULL;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_ident *ident = NULL, *identities, *base;
    ly_bool dyn;

    *err = NULL;

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* locate prefix if any */
    for (prefix_len = 0; (prefix_len < value_len) && (value[prefix_len] != ':'); ++prefix_len) {}
    if (prefix_len < value_len) {
        id_name = &value[prefix_len + 1];
        id_len = value_len - (prefix_len + 1);
    } else {
        prefix_len = 0;
        id_name = value;
        id_len = value_len;
    }

    if (!id_len) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid empty identityref value.");
        goto cleanup;
    }

    mod = lyplg_type_identity_module(ctx, ctx_node, prefix, prefix_len, format, prefix_data);
    if (!mod) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid identityref \"%.*s\" value - unable to map prefix to YANG schema.", (int)value_len, value);
        goto cleanup;
    }

    identities = mod->identities;
    LY_ARRAY_FOR(identities, u) {
        ident = &identities[u]; /* shortcut */
        if (!ly_strncmp(ident->name, id_name, id_len)) {
            /* we have match */
            break;
        }
    }
    if (!identities || (u == LY_ARRAY_COUNT(identities))) {
        /* no match */
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid identityref \"%.*s\" value - identity not found in module \"%s\".",
                (int)value_len, value, mod->name);
        goto cleanup;
    } else if (!mod->implemented) {
        /* non-implemented module */
        if (options & LYPLG_TYPE_STORE_IMPLEMENT) {
            ret = lyplg_type_make_implemented((struct lys_module *)mod, NULL, unres);
            LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
        } else {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                    "Invalid identityref \"%.*s\" value - identity found in non-implemented module \"%s\".",
                    (int)value_len, value, mod->name);
            goto cleanup;
        }
    }

    /* check that the identity matches some of the type's base identities */
    LY_ARRAY_FOR(type_ident->bases, u) {
        if (!lyplg_type_identity_isderived(type_ident->bases[u], ident)) {
            /* we have match */
            break;
        }
    }
    if (u == LY_ARRAY_COUNT(type_ident->bases)) {
        str = NULL;
        str_len = 1;
        LY_ARRAY_FOR(type_ident->bases, u) {
            base = type_ident->bases[u];
            str_len += (u ? 2 : 0) + 1 + strlen(base->module->name) + 1 + strlen(base->name) + 1;
            str = ly_realloc(str, str_len);
            sprintf(str + (u ? strlen(str) : 0), "%s\"%s:%s\"", u ? ", " : "", base->module->name, base->name);
        }

        /* no match */
        if (u == 1) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                    "Invalid identityref \"%.*s\" value - identity not derived from the base %s.",
                    (int)value_len, value, str);
        } else {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                    "Invalid identityref \"%.*s\" value - identity not derived from all the bases %s.",
                    (int)value_len, value, str);
        }
        free(str);
        goto cleanup;
    }

    storage->ident = ident;

    /* get JSON form since there is no canonical */
    str = (char *)lyplg_type_print_identityref(storage, LY_PREF_JSON, NULL, &dyn);
    assert(str && dyn);
    ret = lydict_insert_zc(ctx, str, &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->realtype = type;

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

API LY_ERR
lyplg_type_compare_identityref(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1->ident == val2->ident) {
        return LY_SUCCESS;
    }
    return LY_ENOT;
}

/**
 * @brief Plugin information for identityref type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_identityref[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_IDENT_STR,

        .plugin.id = "libyang 2 - identityref, version 1",
        .plugin.type = LY_TYPE_IDENT,
        .plugin.store = lyplg_type_store_identityref,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_identityref,
        .plugin.print = lyplg_type_print_identityref,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
