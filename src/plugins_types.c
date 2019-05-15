/**
 * @file plugin_types.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in types plugins and interface for user types plugins.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "common.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plugins_types.h"
#include "dict.h"
#include "tree_schema.h"

API LY_ERR
ly_type_validate_range(LY_DATA_TYPE basetype, struct lysc_range *range, int64_t value, struct ly_err_item **err)
{
    unsigned int u;
    char *errmsg = NULL;

    LY_ARRAY_FOR(range->parts, u) {
        if (basetype < LY_TYPE_DEC64) {
            /* unsigned */
            if ((uint64_t)value < range->parts[u].min_u64) {
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    asprintf(&errmsg, "%s \"%"PRIu64"\" does not satisfy the %s constraint.",
                           (basetype & (LY_TYPE_BINARY | LY_TYPE_STRING)) ? "Length" : "Value", (uint64_t)value,
                           (basetype & (LY_TYPE_BINARY | LY_TYPE_STRING)) ? "length" : "range");
                }
                goto error;
            } else if ((uint64_t)value < range->parts[u].max_u64) {
                /* inside the range */
                return LY_SUCCESS;
            } else if (u == LY_ARRAY_SIZE(range->parts) - 1) {
                /* we have the last range part, so the value is out of bounds */
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    asprintf(&errmsg, "%s \"%"PRIu64"\" does not satisfy the %s constraint.",
                           (basetype & (LY_TYPE_BINARY | LY_TYPE_STRING)) ? "Length" : "Value", (uint64_t)value,
                           (basetype & (LY_TYPE_BINARY | LY_TYPE_STRING)) ? "length" : "range");
                }
                goto error;
            }
        } else {
            /* signed */
            if (value < range->parts[u].min_64) {
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    asprintf(&errmsg, "Value \"%"PRId64"\" does not satisfy the range constraint.", value);
                }
                goto error;
            } else if (value < range->parts[u].max_64) {
                /* inside the range */
                return LY_SUCCESS;
            } else if (u == LY_ARRAY_SIZE(range->parts) - 1) {
                /* we have the last range part, so the value is out of bounds */
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    asprintf(&errmsg, "Value \"%"PRId64"\" does not satisfy the range constraint.", value);
                }
                goto error;
            }
        }
    }

    return LY_SUCCESS;

error:
    *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_RESTRICTION, errmsg, NULL, range->eapptag ? strdup(range->eapptag) : NULL);
    return LY_EVALID;
}

/**
 * @brief Validate and canonize value of the YANG built-in binary type.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_binary(struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                        const char **canonized, struct ly_err_item **err)
{
    size_t start, stop, count = 0, u, termination = 0;
    struct lysc_type_bin *type_bin = (struct lysc_type_bin *)type;
    char *errmsg;

    /* initiate */
    *err = NULL;

    /* validate characters and remember the number of octets for length validation */
    if (value) {
        /* silently skip leading/trailing whitespaces */
        for (start = 0; (start < value_len) && isspace(value[start]); start++);
        for (stop = value_len - 1; stop > start && isspace(value[stop]); stop--);
        if (start == stop) {
            /* empty string */
            goto finish;
        }

        for (count = 0, u = start; u < stop; u++) {
            if (value[u] == '\n') {
                /* newline formatting */
                continue;
            }
            count++;

            if ((value[u] < '/' && value[u] != '+') ||
                    (value[u] > '9' && value[u] < 'A') ||
                    (value[u] > 'Z' && value[u] < 'a') || value[u] > 'z') {
                /* non-encoding characters */
                if (value[u] == '=') {
                    /* padding */
                    if (u == stop - 1 && value[u + 1] == '=') {
                        termination = 2;
                        stop = u + 1;
                    } else if (u == stop){
                        termination = 1;
                        stop = u;
                    }
                }
                if (!termination) {
                    /* error */
                    asprintf(&errmsg, "Invalid Base64 character (%c).", value[u]);
                    goto error;
                }
            }
        }
    }

finish:
    if (count & 3) {
        /* base64 length must be multiple of 4 chars */
        errmsg = strdup("Base64 encoded value length must be divisible by 4.");
        goto error;
    }

    /* length of the encoded string */
    if (type_bin->length) {
        uint64_t len = ((count / 4) * 3) - termination;
        LY_CHECK_RET(ly_type_validate_range(LY_TYPE_BINARY, type_bin->length, len, err));
    }

    if (options & LY_TYPE_VALIDATE_CANONIZE) {
        if (start != 0 || stop != value_len - 1) {
            *canonized = lydict_insert_zc(ctx, strndup(&value[start], stop - start));
        } else if (options & LY_TYPE_VALIDATE_DYNAMIC) {
            *canonized = lydict_insert_zc(ctx, (char*)value);
            value = NULL;
        } else {
            *canonized = lydict_insert(ctx, value, value_len);
        }
    }
    if (options & LY_TYPE_VALIDATE_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;

error:
    if (!*err) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_RESTRICTION, errmsg, NULL, NULL);
    }
    return (*err)->no;
}


struct lysc_type_plugin ly_builtin_type_plugins[LY_DATA_TYPE_COUNT] = {
    {0}, /* LY_TYPE_UNKNOWN */
    {.type = LY_TYPE_BINARY, .validate = ly_type_validate_binary, .store = NULL},
    {0}, /* TODO LY_TYPE_UINT8 */
    {0}, /* TODO LY_TYPE_UINT16 */
    {0}, /* TODO LY_TYPE_UINT32 */
    {0}, /* TODO LY_TYPE_UINT64 */
    {0}, /* TODO LY_TYPE_STRING */
    {0}, /* TODO LY_TYPE_BITS */
    {0}, /* TODO LY_TYPE_BOOL */
    {0}, /* TODO LY_TYPE_DEC64 */
    {0}, /* TODO LY_TYPE_EMPTY */
    {0}, /* TODO LY_TYPE_ENUM */
    {0}, /* TODO LY_TYPE_IDENT */
    {0}, /* TODO LY_TYPE_INST */
    {0}, /* TODO LY_TYPE_LEAFREF */
    {0}, /* TODO LY_TYPE_UNION */
    {0}, /* TODO LY_TYPE_INT8 */
    {0}, /* TODO LY_TYPE_INT16 */
    {0}, /* TODO LY_TYPE_INT32 */
    {0}  /* TODO LY_TYPE_INT64 */
};

