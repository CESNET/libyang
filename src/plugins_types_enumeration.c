/**
 * @file plugins_types_enumeration.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in enumeration type plugin.
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"

/**
 * @brief Validate, canonize and store value of the YANG built-in enumeration type.
 *
 * Implementation of the ly_type_store_clb.
 */
LY_ERR
ly_type_store_enum(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_type_enum *type_enum = (struct lysc_type_enum *)type;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* find the matching enumeration value item */
    LY_ARRAY_FOR(type_enum->enums, u) {
        if (!ly_strncmp(type_enum->enums[u].name, value, value_len)) {
            /* we have a match */
            goto match;
        }
    }
    /* enum not found */
    ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid enumeration value \"%.*s\".", (int)value_len, value);
    goto cleanup;

match:
    /* validation done */

    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->canonical);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->enum_item = &type_enum->enums[u];
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *) value);
    }

    return ret;
}
