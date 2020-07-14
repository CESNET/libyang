/**
 * @file plugins_types.c
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

#define _GNU_SOURCE

#include "plugins_types.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "config.h"
#include "dict.h"
#include "path.h"
#include "set.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

/**
 * @brief Generic comparison callback checking the canonical value.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_canonical(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1 == val2 || !strcmp(val1->canonical_cache, val2->canonical_cache)) {
        return LY_SUCCESS;
    }

    return LY_ENOT;
}

/**
 * @brief Generic printer callback of the canonized value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_canonical(const struct lyd_value *value, LYD_FORMAT UNUSED(format),
                        ly_get_prefix_clb UNUSED(get_prefix), void *UNUSED(printer), int *dynamic)
{
    *dynamic = 0;
    return (char*)value->canonical_cache;
}

/**
 * @brief Generic duplication callback of the original value only.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_original(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    dup->canonical_cache = original->canonical_cache;
    dup->original = (void*)lydict_insert(ctx, original->original, strlen(original->original));
    if (dup->original) {
        return LY_SUCCESS;
    } else {
        return LY_EINT;
    }
}

/**
 * @brief Generic duplication callback of the canonized and original values only.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_canonical(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    ly_type_dup_original(ctx, original, dup);
    dup->canonical_cache = (void*)lydict_insert(ctx, original->canonical_cache, strlen(original->canonical_cache));
    if (dup->canonical_cache) {
        return LY_SUCCESS;
    } else {
        return LY_EINT;
    }
}

/**
 * @brief Free original value in lyd_value.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_original(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lydict_remove(ctx, value->original);
    value->original = NULL;
}

/**
 * @brief Free canonized value in lyd_value.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_canonical(const struct ly_ctx *ctx, struct lyd_value *value)
{
    ly_type_free_original(ctx, value);
    lydict_remove(ctx, value->canonical_cache);
    value->canonical_cache = NULL;
}

API LY_ERR
ly_type_parse_int(const char *datatype, int base, int64_t min, int64_t max, const char *value, size_t value_len,
                  int64_t *ret, struct ly_err_item **err)
{
    char *errmsg = NULL;
    int rc = 0;

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    /* consume leading whitespaces */
    for (;value_len && isspace(*value); ++value, --value_len);

    if (!value || !value[0] || !value_len) {
        rc = asprintf(&errmsg, "Invalid empty %s value.", datatype);
        goto error;
    }

    switch(ly_parse_int(value, value_len, min, max, base, ret)) {
    case LY_EDENIED:
        rc = asprintf(&errmsg, "Value is out of %s's min/max bounds.", datatype);
        goto error;
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        rc = asprintf(&errmsg, "Invalid %s value \"%.*s\".", datatype, (int)value_len, value);
        goto error;
    }

error:
    if (rc == -1) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        return LY_EMEM;
    } else {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        return LY_EVALID;
    }
}

API struct lyd_value_prefix *
ly_type_get_prefixes(const struct ly_ctx *ctx, const char *value, size_t value_len, ly_resolve_prefix_clb resolve_prefix,
                     void *parser)
{
    LY_ERR ret;
    unsigned int c;
    const char *start, *stop;
    struct lyd_value_prefix *prefixes = NULL;
    const struct lys_module *mod;
    LY_ARRAY_COUNT_TYPE u;

    for (stop = start = value; (size_t)(stop - value) < value_len; start = stop) {
        size_t bytes;
        ly_getutf8(&stop, &c, &bytes);
        if (is_xmlqnamestartchar(c)) {
            for (ly_getutf8(&stop, &c, &bytes);
                    is_xmlqnamechar(c) && (size_t)(stop - value) < value_len;
                    ly_getutf8(&stop, &c, &bytes));
            stop = stop - bytes;
            if (*stop == ':') {
                /* we have a possible prefix */
                struct lyd_value_prefix *p;
                size_t len = stop - start;
                mod = NULL;

                LY_ARRAY_FOR(prefixes, u) {
                    if (!ly_strncmp(prefixes[u].prefix, start, len)) {
                        mod = prefixes[u].mod;
                        break;
                    }
                }
                if (!mod) {
                    mod = resolve_prefix(ctx, start, len, parser);
                    if (mod) {
                        LY_ARRAY_NEW_GOTO(ctx, prefixes, p, ret, error);
                        p->mod = mod;
                        p->prefix = lydict_insert(ctx, start, len);
                    }
                } /* else the prefix already present */
            }
            stop = stop + bytes;
        }
    }

    return prefixes;

error:
    LY_ARRAY_FOR(prefixes, u) {
        lydict_remove(ctx, prefixes[u].prefix);
    }
    LY_ARRAY_FREE(prefixes);

    (void)ret;
    return NULL;
}

API LY_ERR
ly_type_parse_uint(const char *datatype, int base, uint64_t max, const char *value, size_t value_len, uint64_t *ret,
                   struct ly_err_item **err)
{
    char *errmsg = NULL;
    int rc = 0;

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    /* consume leading whitespaces */
    for (;value_len && isspace(*value); ++value, --value_len);

    if (!value || !value[0] || !value_len) {
        rc = asprintf(&errmsg, "Invalid empty %s value.", datatype);
        goto error;
    }

    *err = NULL;
    switch (ly_parse_uint(value, value_len, max, base, ret)) {
    case LY_EDENIED:
        rc = asprintf(&errmsg, "Value \"%.*s\" is out of %s's min/max bounds.", (int)value_len, value, datatype);
        goto error;
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        rc = asprintf(&errmsg, "Invalid %s value \"%.*s\".", datatype, (int)value_len, value);
        goto error;
    }

error:
    if (rc == -1) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        return LY_EMEM;
    } else {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        return LY_EVALID;
    }
}

API LY_ERR
ly_type_parse_dec64(uint8_t fraction_digits, const char *value, size_t value_len, int64_t *ret, struct ly_err_item **err)
{
    LY_ERR rc = LY_EVALID;
    char *errmsg = NULL;
    char *valcopy = NULL;
    size_t fraction = 0, size, len = 0, trailing_zeros;
    int64_t d;

    /* consume leading whitespaces */
    for (;value_len && isspace(*value); ++value, --value_len);

    /* parse value */
    if (!value_len) {
        errmsg = strdup("Invalid empty decimal64 value.");
        goto error;
    } else if (!isdigit(value[len]) && (value[len] != '-') && (value[len] != '+')) {
        if (asprintf(&errmsg, "Invalid %lu. character of decimal64 value \"%.*s\".",
                     len + 1, (int)value_len, value) == -1) {
            errmsg = NULL;
            rc = LY_EMEM;
        }
        goto error;
    }

    if ((value[len] == '-') || (value[len] == '+')) {
        ++len;
    }

    while (len < value_len && isdigit(value[len])) {
        ++len;
    }

    trailing_zeros = 0;
    if (len < value_len && ((value[len] != '.') || !isdigit(value[len + 1]))) {
        goto decimal;
    }
    fraction = len;
    ++len;
    while (len < value_len && isdigit(value[len])) {
        if (value[len] == '0') {
            ++trailing_zeros;
        } else {
            trailing_zeros = 0;
        }
        ++len;
    }
    len = len - trailing_zeros;

decimal:
    if (fraction && (len - 1 - fraction > fraction_digits)) {
        if (asprintf(&errmsg, "Value \"%.*s\" of decimal64 type exceeds defined number (%u) of fraction digits.", (int)len, value,
                     fraction_digits) == -1) {
            errmsg = NULL;
            rc = LY_EMEM;
        }
        goto error;
    }
    if (fraction) {
        size = len + (fraction_digits - (len - 1 - fraction));
    } else {
        size = len + fraction_digits + 1;
    }

    if (len + trailing_zeros < value_len) {
        /* consume trailing whitespaces to check that there is nothing after it */
        unsigned long int u;
        for (u = len + trailing_zeros; u < value_len && isspace(value[u]); ++u);
        if (u != value_len) {
            if (asprintf(&errmsg, "Invalid %lu. character of decimal64 value \"%.*s\".",
                         u + 1, (int)value_len, value) == -1) {
                errmsg = NULL;
                rc = LY_EMEM;
            }
            goto error;
        }
    }

    /* prepare value string without decimal point to easily parse using standard functions */
    valcopy = malloc(size * sizeof *valcopy);
    if (!valcopy) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        return LY_EMEM;
    }

    valcopy[size - 1] = '\0';
    if (fraction) {
        memcpy(&valcopy[0], &value[0], fraction);
        memcpy(&valcopy[fraction], &value[fraction + 1], len - 1 - (fraction));
        /* add trailing zero characters */
        memset(&valcopy[len - 1], '0', fraction_digits - (len - 1 - fraction));
    } else {
        memcpy(&valcopy[0], &value[0], len);
        /* add trailing zero characters */
        memset(&valcopy[len], '0', fraction_digits);
    }

    rc = ly_type_parse_int("decimal64", 10, INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807), valcopy, len, &d, err);
    if (!rc && ret) {
        *ret = d;
    }
    free(valcopy);

error:
    if (errmsg) {
        *err = ly_err_new(LY_LLERR, rc, LYVE_DATA, errmsg, NULL, NULL);
    }
    return rc;
}

API LY_ERR
ly_type_validate_patterns(struct lysc_pattern **patterns, const char *str, size_t str_len, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int rc;
    LY_ARRAY_COUNT_TYPE u;
    char *errmsg;
    pcre2_match_data *match_data = NULL;

    LY_CHECK_ARG_RET(NULL, str, err, LY_EINVAL);

    LY_ARRAY_FOR(patterns, u) {
        /* match_data needs to be allocated each time because of possible multi-threaded evaluation */
        match_data = pcre2_match_data_create_from_pattern(patterns[u]->code, NULL);
        if (!match_data) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
            return LY_EMEM;
        }

        rc = pcre2_match(patterns[u]->code, (PCRE2_SPTR)str, str_len, 0, PCRE2_ANCHORED | PCRE2_ENDANCHORED, match_data, NULL);
        if (rc == PCRE2_ERROR_NOMATCH) {
            if (asprintf(&errmsg, "String \"%.*s\" does not conform to the pattern \"%s\".", (int)str_len, str, patterns[u]->expr) == -1) {
                *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
                ret = LY_EMEM;
            } else {
                *err = ly_err_new(LY_LLERR, LY_ESYS, 0, errmsg, NULL, NULL);
                ret = LY_EVALID;
            }
            goto cleanup;
        } else if (rc < 0) {
            /* error */
            PCRE2_UCHAR pcre2_errmsg[256] = {0};
            pcre2_get_error_message(rc, pcre2_errmsg, 256);
            *err = ly_err_new(LY_LLERR, LY_ESYS, 0, strdup((const char*)pcre2_errmsg), NULL, NULL);
            ret = LY_ESYS;
            goto cleanup;
        }

    cleanup:
        pcre2_match_data_free(match_data);
        if (ret) {
            break;
        }
    }

    return ret;
}

API LY_ERR
ly_type_validate_range(LY_DATA_TYPE basetype, struct lysc_range *range, int64_t value, const char *strval,
                       struct ly_err_item **err)
{
    LY_ARRAY_COUNT_TYPE u;
    char *errmsg = NULL;
    int rc = 0;

    LY_ARRAY_FOR(range->parts, u) {
        if (basetype < LY_TYPE_DEC64) {
            /* unsigned */
            if ((uint64_t)value < range->parts[u].min_u64) {
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    rc = asprintf(&errmsg, "%s \"%s\" does not satisfy the %s constraint.",
                                  (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "Length" : "Value", strval,
                                  (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "length" : "range");
                }
                goto error;
            } else if ((uint64_t)value <= range->parts[u].max_u64) {
                /* inside the range */
                return LY_SUCCESS;
            } else if (u == LY_ARRAY_COUNT(range->parts) - 1) {
                /* we have the last range part, so the value is out of bounds */
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    rc = asprintf(&errmsg, "%s \"%s\" does not satisfy the %s constraint.",
                                  (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "Length" : "Value", strval,
                                  (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "length" : "range");
                }
                goto error;
            }
        } else {
            /* signed */
            if (value < range->parts[u].min_64) {
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    rc = asprintf(&errmsg, "Value \"%s\" does not satisfy the range constraint.", strval);
                }
                goto error;
            } else if (value < range->parts[u].max_64) {
                /* inside the range */
                return LY_SUCCESS;
            } else if (u == LY_ARRAY_COUNT(range->parts) - 1) {
                /* we have the last range part, so the value is out of bounds */
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    rc = asprintf(&errmsg, "Value \"%s\" does not satisfy the range constraint.", strval);
                }
                goto error;
            }
        }
    }

    return LY_SUCCESS;

error:
    if (rc == -1 || !errmsg) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        return LY_EMEM;
    } else {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, range->eapptag ? strdup(range->eapptag) : NULL);
        return LY_EVALID;
    }
}

static void
ly_type_store_strval(const struct ly_ctx *ctx, int options, const char *orig, const char *value,
                     struct lyd_value *storage, const char **canonized)
{
    if (options & LY_TYPE_OPTS_CANONIZE) {
        *canonized = value;
    }
    if (options & LY_TYPE_OPTS_STORE) {
        storage->original = orig;
        if (options & LY_TYPE_OPTS_CANONIZE) {
            /* already stored outside the storage in canonized, so we have to add instance in dictionary */
            storage->canonical_cache = (void*)lydict_insert(ctx, value, strlen(value));
        } else {
            storage->canonical_cache = (void*)value;
        }
    }
}
#if 0
static void
ly_type_store_canonized(const struct ly_ctx *ctx, int options, const char *value, struct lyd_value *storage, const char **canonized)
{
    if (options & LY_TYPE_OPTS_CANONIZE) {
        *canonized = value;
    }
    if (options & LY_TYPE_OPTS_STORE) {
        if (options & LY_TYPE_OPTS_CANONIZE) {
            /* already stored outside the storage in canonized, so we have to add instance in dictionary */
            storage->canonical_cache = (void*)lydict_insert(ctx, value, strlen(value));
        } else {
            storage->canonical_cache = (void*)value;
        }
    }
}
#endif
/**
 * @brief Validate, canonize and store value of the YANG built-in signed integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_int(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                  ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                  const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                  struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret;
    int64_t i;
    char *str;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    switch (type->basetype) {
    case LY_TYPE_INT8:
        LY_CHECK_RET(ly_type_parse_int("int8", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, INT64_C(-128), INT64_C(127), value, value_len, &i, err));
        break;
    case LY_TYPE_INT16:
        LY_CHECK_RET(ly_type_parse_int("int16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, INT64_C(-32768), INT64_C(32767), value, value_len, &i, err));
        break;
    case LY_TYPE_INT32:
        LY_CHECK_RET(ly_type_parse_int("int32", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10,
                                       INT64_C(-2147483648), INT64_C(2147483647), value, value_len, &i, err));
        break;
    case LY_TYPE_INT64:
        LY_CHECK_RET(ly_type_parse_int("int64", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10,
                                       INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807), value, value_len, &i, err));
        break;
    default:
        LOGINT_RET(NULL);
    }

    LY_CHECK_RET(asprintf(&str, "%"PRId64, i) == -1, LY_EMEM);

    /* range of the number */
    if (type_num->range) {
        LY_CHECK_ERR_RET(ret = ly_type_validate_range(type->basetype, type_num->range, i, str, err), free(str), ret);
    }

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        if (options & LY_TYPE_OPTS_STORE) {
            storage->int64 = i;
            ly_type_store_strval(ctx, options, lydict_insert(ctx, value_len ? value : "", value_len),
                                 lydict_insert_zc(ctx, str), storage, canonized);
        } else {
            ly_type_store_strval(ctx, options, NULL, lydict_insert_zc(ctx, str), storage, canonized);
        }
        str = NULL;
    }
    free(str);
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;
}

/* @brief Duplication callback of the signed integer values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_int(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    dup->int64 = original->int64;
    return ly_type_dup_canonical(ctx, original, dup);
}

/**
 * @brief Validate and canonize value of the YANG built-in unsigned integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_uint(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                   ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                   const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                   struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret;
    uint64_t u;
    struct lysc_type_num* type_num = (struct lysc_type_num*)type;
    char *str;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    switch (type->basetype) {
    case LY_TYPE_UINT8:
        LY_CHECK_RET(ly_type_parse_uint("uint8", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(255), value, value_len, &u, err));
        break;
    case LY_TYPE_UINT16:
        LY_CHECK_RET(ly_type_parse_uint("uint16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(65535), value, value_len, &u, err));
        break;
    case LY_TYPE_UINT32:
        LY_CHECK_RET(ly_type_parse_uint("uint32", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(4294967295), value, value_len, &u, err));
        break;
    case LY_TYPE_UINT64:
        LY_CHECK_RET(ly_type_parse_uint("uint64", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(18446744073709551615), value, value_len, &u, err));
        break;
    default:
        LOGINT_RET(NULL);
    }

    LY_CHECK_RET(asprintf(&str, "%"PRIu64, u) == -1, LY_EMEM);

    /* range of the number */
    if (type_num->range) {
        LY_CHECK_ERR_RET(ret = ly_type_validate_range(type->basetype, type_num->range, u, str, err), free(str), ret);
    }

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        if (options & LY_TYPE_OPTS_STORE) {
            storage->int64 = u;
            ly_type_store_strval(ctx, options, lydict_insert(ctx, value_len ? value : "", value_len),
                                 lydict_insert_zc(ctx, str), storage, canonized);
        } else {
            ly_type_store_strval(ctx, options, NULL, lydict_insert_zc(ctx, str), storage, canonized);
        }
        str = NULL;
    }
    free(str);
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;
}

/**
 * @brief Duplication callback of the unsigned integer values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_uint(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    dup->uint64 = original->uint64;
    return ly_type_dup_canonical(ctx, original, dup);
}

/**
 * @brief Validate, canonize and store value of the YANG built-in decimal64 types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_decimal64(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                        ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                        const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                        struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    int64_t d;
    struct lysc_type_dec* type_dec = (struct lysc_type_dec*)type;
    char buf[22];

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    if (!value || !value[0] || !value_len) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, strdup("Invalid empty decimal64 value."), NULL, NULL);
        return LY_EVALID;
    }

    LY_CHECK_RET(ly_type_parse_dec64(type_dec->fraction_digits, value, value_len, &d, err));
    /* prepare canonized value */
    if (d) {
        int count = sprintf(buf, "%"PRId64" ", d);
        if ( (d > 0 && (count - 1) <= type_dec->fraction_digits)
             || (count - 2) <= type_dec->fraction_digits ) {
            /* we have 0. value, print the value with the leading zeros
             * (one for 0. and also keep the correct with of num according
             * to fraction-digits value)
             * for (num<0) - extra character for '-' sign */
            count = sprintf(buf, "%0*"PRId64" ", (d > 0) ? (type_dec->fraction_digits + 1) : (type_dec->fraction_digits + 2), d);
        }
        for (int i = type_dec->fraction_digits, j = 1; i > 0 ; i--) {
            if (j && i > 1 && buf[count - 2] == '0') {
                /* we have trailing zero to skip */
                buf[count - 1] = '\0';
            } else {
                j = 0;
                buf[count - 1] = buf[count - 2];
            }
            count--;
        }
        buf[count - 1] = '.';
    } else {
        /* zero */
        sprintf(buf, "0.0");
    }

    /* range of the number */
    if (type_dec->range) {
        LY_CHECK_RET(ly_type_validate_range(type->basetype, type_dec->range, d, buf, err));
    }

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        if (options & LY_TYPE_OPTS_STORE) {
            storage->dec64 = d;
            ly_type_store_strval(ctx, options, lydict_insert(ctx, value, value_len),
                                 lydict_insert(ctx, buf, strlen(buf)), storage, canonized);
        } else {
            ly_type_store_strval(ctx, options, NULL, lydict_insert(ctx, buf, strlen(buf)), storage, canonized);
        }
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;
}

/* @brief Duplication callback of the decimal64 values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_decimal64(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
   dup->dec64 = original->dec64;
   return ly_type_dup_canonical(ctx, original, dup);
}

/**
 * @brief Validate, canonize and store value of the YANG built-in binary type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_binary(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                     ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                     const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                     struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    size_t start = 0, stop = 0, count = 0, u, termination = 0;
    struct lysc_type_bin *type_bin = (struct lysc_type_bin *)type;
    char *errmsg;
    int erc = 0;

    LY_CHECK_ARG_RET(ctx, value, LY_EINVAL);

    /* initiate */
    *err = NULL;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    /* validate characters and remember the number of octets for length validation */
    if (value_len) {
        /* silently skip leading/trailing whitespaces */
        for (start = 0; (start < value_len) && isspace(value[start]); start++);
        for (stop = value_len - 1; stop > start && isspace(value[stop]); stop--);
        if (start == stop) {
            /* empty string */
            goto finish;
        }

        for (count = 0, u = start; u <= stop; u++) {
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
                    if (u == stop - 1 && value[stop] == '=') {
                        termination = 2;
                        count++;
                        u++;
                    } else if (u == stop){
                        termination = 1;
                    }
                }
                if (!termination) {
                    /* error */
                    erc = asprintf(&errmsg, "Invalid Base64 character (%c).", value[u]);
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
        char buf[22];
        uint64_t len = ((count / 4) * 3) - termination;
        snprintf(buf, 22, "%"PRIu64, len);
        LY_CHECK_RET(ly_type_validate_range(LY_TYPE_BINARY, type_bin->length, len, buf, err));
    }

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        const char *c;
        if (start != 0 || (value_len && stop != value_len - 1)) {
            c = lydict_insert_zc(ctx, strndup(&value[start], stop + 1 - start));
        } else {
            c = lydict_insert(ctx, value_len ? value : "", value_len);
        }
        if (options & LY_TYPE_OPTS_STORE) {
            ly_type_store_strval(ctx, options, lydict_insert(ctx, value_len ? value : "", value_len), c, storage, canonized);
        } else {
            ly_type_store_strval(ctx, options, NULL, c, storage, canonized);
        }
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;

error:
    if (!*err) {
        if (erc == -1 || !errmsg) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        } else {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        }
    }
    return (*err)->no;
}

/**
 * @brief Validate and store value of the YANG built-in string type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_string(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                     ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                     const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                     struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    /* length restriction of the string */
    if (type_str->length) {
        char buf[22];
        size_t char_count = ly_utf8len(value, value_len);

        /* value_len is in bytes, but we need number of chaarcters here */
        snprintf(buf, 22, "%lu", char_count);
        LY_CHECK_RET(ly_type_validate_range(LY_TYPE_BINARY, type_str->length, char_count, buf, err));
    }

    /* pattern restrictions */
    LY_CHECK_RET(ly_type_validate_patterns(type_str->patterns, value, value_len, err));

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        /* NOTE: despite the pointer is used in storage twice (original and canonical_cache), it is stored in dictionary
         * just once. This works even without storage - the string is returned as canonized. In case both options are used,
         * ly_type_store_strval() increases reference count in dictionary for canonized. */
        const char *str = lydict_insert(ctx, value_len ? value : "", value_len);
        ly_type_store_strval(ctx, options, str, str, storage, canonized);
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_bits(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                   ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                   const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                   struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret = LY_EVALID;
    size_t item_len;
    const char *item;
    struct ly_set *items = NULL, *items_ordered = NULL;
    size_t buf_size = 0;
    char *buf = NULL;
    size_t index;
    LY_ARRAY_COUNT_TYPE u, v;
    char *errmsg = NULL;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits*)type;
    int iscanonical = 1;
    size_t ws_count;
    size_t lws_count; /* leading whitespace count */
    const char *can = NULL;
    int erc = 0;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    /* remember the present items for further work */
    items = ly_set_new();
    LY_CHECK_RET(!items, LY_EMEM);

    for (index = ws_count = lws_count = 0; index < value_len; index++, ws_count++) {
        if (isspace(value[index])) {
            continue;
        }
        if (index == ws_count) {
            lws_count = ws_count;
        } else if (ws_count > 1) {
            iscanonical = 0;
        }
        ws_count = 0;

        /* start of the item */
        item = &value[index];
        for (item_len = 0; index + item_len < value_len && !isspace(item[item_len]); item_len++);
        LY_ARRAY_FOR(type_bits->bits, u) {
            if (!ly_strncmp(type_bits->bits[u].name, item, item_len)) {
                /* we have the match */
                int inserted;

                /* check that the bit is not disabled */
                LY_ARRAY_FOR(type_bits->bits[u].iffeatures, v) {
                    if (lysc_iffeature_value(&type_bits->bits[u].iffeatures[v]) == LY_ENOT) {
                        erc = asprintf(&errmsg, "Bit \"%s\" is disabled by its %" LY_PRI_ARRAY_COUNT_TYPE ". if-feature condition.",
                                       type_bits->bits[u].name, v + 1);
                        goto error;
                    }
                }

                if (iscanonical &&items->count && type_bits->bits[u].position < ((struct lysc_type_bitenum_item*)items->objs[items->count - 1])->position) {
                    iscanonical = 0;
                }
                inserted = ly_set_add(items, &type_bits->bits[u], 0);
                LY_CHECK_ERR_GOTO(inserted == -1, ret = LY_EMEM, error);
                if ((unsigned int)inserted != items->count - 1) {
                    erc = asprintf(&errmsg, "Bit \"%s\" used multiple times.", type_bits->bits[u].name);
                    goto error;
                }
                goto next;
            }
        }
        /* item not found */
        erc = asprintf(&errmsg, "Invalid bit value \"%.*s\".", (int)item_len, item);
        goto error;
next:
        /* remember for canonized form: item + space/termination-byte */
        buf_size += item_len + 1;
        index += item_len;
    }
    /* validation done */

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        if (iscanonical) {
            /* items are already ordered */
            items_ordered = items;
            items = NULL;

            if (!ws_count && !lws_count && (options & LY_TYPE_OPTS_DYNAMIC)) {
                can = lydict_insert_zc(ctx, (char*)value);
                value = NULL;
            } else {
                can = lydict_insert(ctx, value_len ? &value[lws_count] : "", value_len - ws_count - lws_count);
            }
        } else {
            buf = malloc(buf_size * sizeof *buf);
            LY_CHECK_ERR_GOTO(!buf, LOGMEM(ctx); ret = LY_EMEM, error);
            index = 0;

            items_ordered = ly_set_dup(items, NULL);
            LY_CHECK_ERR_GOTO(!items_ordered, LOGMEM(ctx); ret = LY_EMEM, error);
            items_ordered->count = 0;

            /* generate ordered bits list */
            LY_ARRAY_FOR(type_bits->bits, u) {
                if (ly_set_contains(items, &type_bits->bits[u]) != -1) {
                    int c = sprintf(&buf[index], "%s%s", index ? " " : "", type_bits->bits[u].name);
                    LY_CHECK_ERR_GOTO(c < 0, LOGERR(ctx, LY_ESYS, "sprintf() failed."); ret = LY_ESYS, error);
                    index += c;
                    ly_set_add(items_ordered, &type_bits->bits[u], LY_SET_OPT_USEASLIST);
                }
            }
            assert(buf_size == index + 1);
            /* termination NULL-byte */
            buf[index] = '\0';

            can = lydict_insert_zc(ctx, buf);
            buf = NULL;
        }

        ly_type_store_strval(ctx, options, lydict_insert(ctx, value_len ? value : "", value_len), can, storage, canonized);

        if (options & LY_TYPE_OPTS_STORE) {
            /* store data */
            LY_ARRAY_CREATE_GOTO(ctx, storage->bits_items, items_ordered->count, ret, error);
            for (uint32_t x = 0; x < items_ordered->count; x++) {
                storage->bits_items[x] = items_ordered->objs[x];
                LY_ARRAY_INCREMENT(storage->bits_items);
            }
        }
        ly_set_free(items_ordered, NULL);
    }

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    ly_set_free(items, NULL);
    return LY_SUCCESS;
error:
    if (erc == -1) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        ret = LY_EMEM;
    } else if (errmsg) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
    }
    ly_set_free(items, NULL);
    ly_set_free(items_ordered, NULL);
    free(buf);
    lydict_remove(ctx, can);
    return ret;
}

/* @brief Duplication callback of the bits values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_bits(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_CREATE_RET(ctx, dup->bits_items, LY_ARRAY_COUNT(original->bits_items), LY_EMEM);
    LY_ARRAY_FOR(original->bits_items, u) {
        LY_ARRAY_INCREMENT(dup->bits_items);
        dup->bits_items[u] = original->bits_items[u];
    }

   return ly_type_dup_canonical(ctx, original, dup);
}

/**
 * @brief Free value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value)
{
    LY_ARRAY_FREE(value->bits_items);
    value->bits_items = NULL;

    ly_type_free_canonical(ctx, value);
}


/**
 * @brief Validate, canonize and store value of the YANG built-in enumeration type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_enum(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                   ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                   const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                   struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ARRAY_COUNT_TYPE u, v;
    char *errmsg = NULL;
    struct lysc_type_enum *type_enum = (struct lysc_type_enum*)type;
    int erc = 0;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    /* find the matching enumeration value item */
    LY_ARRAY_FOR(type_enum->enums, u) {
        if (!ly_strncmp(type_enum->enums[u].name, value, value_len)) {
            /* we have the match */

            /* check that the enumeration value is not disabled */
            LY_ARRAY_FOR(type_enum->enums[u].iffeatures, v) {
                if (lysc_iffeature_value(&type_enum->enums[u].iffeatures[v]) == LY_ENOT) {
                    erc = asprintf(&errmsg, "Enumeration \"%s\" is disabled by its %" LY_PRI_ARRAY_COUNT_TYPE ". if-feature condition.",
                                   type_enum->enums[u].name, v + 1);
                    goto error;
                }
            }
            goto match;
        }
    }
    /* enum not found */
    erc = asprintf(&errmsg, "Invalid enumeration value \"%.*s\".", (int)value_len, value);
    goto error;

match:
    /* validation done */

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        /* NOTE: despite the pointer is used in storage twice (original and canonical_cache), it is stored in dictionary
         * just once. This works even without storage - the string is returned as canonized. In case both options are used,
         * ly_type_store_strval() increases reference count in dictionary for canonized. */
        const char *str = lydict_insert(ctx, value_len ? value : "", value_len);
        if (options & LY_TYPE_OPTS_STORE) {
            storage->enum_item = &type_enum->enums[u];
        }
        ly_type_store_strval(ctx, options, str, str, storage, canonized);
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;

error:
    if (erc == -1) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        return LY_EMEM;
    } else {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        return LY_EVALID;
    }
}

/* @brief Duplication callback of the enumeration values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_enum(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    dup->enum_item = original->enum_item;
    return ly_type_dup_original(ctx, original, dup);
}

/**
 * @brief Validate and store value of the YANG built-in boolean type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_boolean(const struct ly_ctx *ctx, struct lysc_type *UNUSED(type), const char *value, size_t value_len, int options,
                      ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                      const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                      struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    int8_t i;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    if (value_len == 4 && !strncmp(value, "true", 4)) {
        i = 1;
    } else if (value_len == 5 && !strncmp(value, "false", 5)) {
        i = 0;
    } else {
        char *errmsg;
        if (asprintf(&errmsg, "Invalid boolean value \"%.*s\".", (int)value_len, value) == -1) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
            return LY_EMEM;
        } else {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
            return LY_EVALID;
        }
    }

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        /* NOTE: despite the pointer is used in storage twice (original and canonical_cache), it is stored in dictionary
         * just once. This works even without storage - the string is returned as canonized. In case both options are used,
         * ly_type_store_strval() increases reference count in dictionary for canonized. */
        const char *str = lydict_insert(ctx, value, value_len);
        if (options & LY_TYPE_OPTS_STORE) {
            storage->boolean = i;
        }
        ly_type_store_strval(ctx, options, str, str, storage, canonized);
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;
}

/* @brief Duplication callback of the boolean values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_boolean(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    dup->int64 = original->int64;
    return ly_type_dup_original(ctx, original, dup);
}

/**
 * @brief Validate and store value of the YANG built-in empty type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_empty(const struct ly_ctx *ctx, struct lysc_type *UNUSED(type), const char *value, size_t value_len, int options,
                    ly_resolve_prefix_clb UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                    const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                    struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    if (value_len) {
        char *errmsg;
        if (asprintf(&errmsg, "Invalid empty value \"%.*s\".", (int)value_len, value) == -1) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
            return LY_EMEM;
        } else {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
            return LY_EVALID;
        }
    }

    if (options & (LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_STORE)) {
        /* NOTE: despite the pointer is used in storage twice (original and canonical_cache), it is stored in dictionary
         * just once. This works even without storage - the string is returned as canonized. In case both options are used,
         * ly_type_store_strval() increases reference count in dictionary for canonized. */
        const char *str = lydict_insert(ctx, "", 0);
        ly_type_store_strval(ctx, options, str, str, storage, canonized);
    }

    return LY_SUCCESS;
}

/**
 * @brief Comparison callback for built-in empty type.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_empty(const struct lyd_value *UNUSED(val1), const struct lyd_value *UNUSED(val2))
{
    /* empty has just one value, so empty data must be always the same */
    return LY_SUCCESS;
}

API LY_ERR
ly_type_identity_isderived(struct lysc_ident *base, struct lysc_ident *der)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(base->derived, u) {
        if (der == base->derived[u]) {
            return LY_SUCCESS;
        }
        if (!ly_type_identity_isderived(base->derived[u], der)) {
            return LY_SUCCESS;
        }
    }
    return LY_ENOTFOUND;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in identiytref type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_identityref(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                          ly_resolve_prefix_clb resolve_prefix, void *parser, LYD_FORMAT UNUSED(format),
                          const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                          struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    struct lysc_type_identityref *type_ident = (struct lysc_type_identityref *)type;
    const char *id_name, *prefix = value;
    size_t id_len, prefix_len;
    char *errmsg = NULL;
    const struct lys_module *mod;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_ident *ident = NULL, *identities;
    int erc = 0;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    /* locate prefix if any */
    for (prefix_len = 0; prefix_len < value_len && value[prefix_len] != ':'; ++prefix_len);
    if (prefix_len < value_len) {
        id_name = &value[prefix_len + 1];
        id_len = value_len - (prefix_len + 1);
    } else {
        prefix_len = 0;
        id_name = value;
        id_len = value_len;
    }

    if (!id_len) {
        errmsg = strdup("Invalid empty identityref value.");
        goto error;
    }

    mod = resolve_prefix(ctx, prefix, prefix_len, parser);
    if (!mod) {
        erc = asprintf(&errmsg, "Invalid identityref \"%.*s\" value - unable to map prefix to YANG schema.", (int)value_len, value);
        goto error;
    }
    if (mod->compiled) {
        identities = mod->compiled->identities;
    } else {
        identities = mod->dis_identities;
    }
    LY_ARRAY_FOR(identities, u) {
        ident = &identities[u]; /* shortcut */
        if (!ly_strncmp(ident->name, id_name, id_len)) {
            /* we have match */
            break;
        }
    }
    if (u == LY_ARRAY_COUNT(identities)) {
        /* no match */
        erc = asprintf(&errmsg, "Invalid identityref \"%.*s\" value - identity not found.", (int)value_len, value);
        goto error;
    } else if (!mod->compiled) {
        /* non-implemented module */
        erc = asprintf(&errmsg, "Invalid identityref \"%.*s\" value - identity found in non-implemented module \"%s\".",
                 (int)value_len, value, mod->name);
        goto error;
    }

    /* check that the identity matches some of the type's base identities */
    LY_ARRAY_FOR(type_ident->bases, u) {
        if (!ly_type_identity_isderived(type_ident->bases[u], ident)) {
            /* we have match */
            break;
        }
    }
    if (u == LY_ARRAY_COUNT(type_ident->bases)) {
        /* no match */
        erc = asprintf(&errmsg, "Invalid identityref \"%.*s\" value - identity not accepted by the type specification.", (int)value_len, value);
        goto error;
    }

    if (options & LY_TYPE_OPTS_CANONIZE) {
        /* identityref does not have a canonical form - to make it clear, the canonized form is represented as NULL */
        *canonized = NULL;
    }

    if (options & LY_TYPE_OPTS_STORE) {
        storage->ident = ident;
        storage->canonical_cache = NULL;
        storage->original = lydict_insert(ctx, value, value_len);
    }

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;

error:
    if (erc == -1 || !errmsg) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
        return LY_EMEM;
    } else {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        return LY_EVALID;
    }
}

/**
 * @brief Comparison callback for built-in identityref type.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_identityref(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->ident == val2->ident) {
        return LY_SUCCESS;
    }
    return LY_ENOT;
}

/**
 * @brief Printer callback printing identityref value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_identityref(const struct lyd_value *value, LYD_FORMAT UNUSED(format), ly_get_prefix_clb get_prefix, void *printer, int *dynamic)
{
    char *result = NULL;

    *dynamic = 1;
    if (asprintf(&result, "%s:%s", get_prefix(value->ident->module, printer), value->ident->name) == -1) {
        return NULL;
    } else {
        return result;
    }
}

/* @brief Duplication callback of the identityref values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_identityref(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *original, struct lyd_value *dup)
{
    dup->ident = original->ident;
    return LY_SUCCESS;
}

/**
 * @brief Helper function for ly_type_store_instanceid_parse_predicate_value() to provide prefix mapping for the
 * validation callbacks for the values used in instance-identifier predicates.
 *
 * Implementation of the ly_resolve_prefix_clb.
 */
static const struct lys_module *
ly_type_stored_prefixes_clb(const struct ly_ctx *UNUSED(ctx), const char *prefix, size_t prefix_len, void *private)
{
    struct lyd_value_prefix *prefixes = (struct lyd_value_prefix*)private;
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(prefixes, u) {
        if (!ly_strncmp(prefixes[u].prefix, prefix, prefix_len)) {
            return prefixes[u].mod;
        }
    }
    return NULL;
}

/**
 * @brief Validate and store value of the YANG built-in instance-identifier type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_instanceid(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                         ly_resolve_prefix_clb resolve_prefix, void *parser, LYD_FORMAT format,
                         const void *context_node, const struct lyd_node *tree, struct lyd_value *storage,
                         const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret = LY_EVALID;
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)type;
    char *errmsg = NULL;
    struct lyd_value_prefix *prefixes = NULL;
    LY_ARRAY_COUNT_TYPE u;
    struct ly_path *path = NULL;
    struct ly_set predicates = {0};
    struct lyxp_expr *exp = NULL;
    const struct lysc_node *ctx_scnode;
    int erc = 0;
    int prefix_opt = 0;

    /* init */
    *err = NULL;
    ctx_scnode = (options & (LY_TYPE_OPTS_SCHEMA | LY_TYPE_OPTS_INCOMPLETE_DATA)) ?
                 (struct lysc_node *)context_node : ((struct lyd_node *)context_node)->schema;

    if ((options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_INCOMPLETE_DATA)) {
        /* we have incomplete schema tree, so we are actually just storing the original value for future validation */
        if (options & LY_TYPE_OPTS_STORE) {
            storage->original = lydict_insert(ctx, value_len ? value : "", value_len);
        }
        goto cleanup;
    }

    switch (format) {
    case LYD_SCHEMA:
    case LYD_XML:
        prefix_opt = LY_PATH_PREFIX_MANDATORY;
        break;
    case LYD_JSON:
    case LYD_LYB:
        prefix_opt = LY_PATH_PREFIX_STRICT_INHERIT;
        break;
    }

    if (!(options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_SECOND_CALL) && (options & LY_TYPE_OPTS_STORE)) {
        /* the second run in data tree, the first one ended with LY_EINCOMPLETE, but we have prepared the target structure */
        if (ly_path_eval(storage->target, tree, NULL)) {
            /* in error message we print the JSON format of the instance-identifier - in case of XML, it is not possible
             * to get the exactly same string as original, JSON is less demanding and still well readable/understandable. */
            int dynamic = 0;
            const char *id = storage->realtype->plugin->print(storage, LYD_JSON, json_print_get_prefix, NULL, &dynamic);
            erc = asprintf(&errmsg, "Invalid instance-identifier \"%s\" value - required instance not found.", id);
            if (dynamic) {
                free((char*)id);
            }
            /* we have to clean up the storage */
            type->plugin->free(ctx, storage);

            goto error;
        }
        return LY_SUCCESS;
    }

    /* get all used prefixes */
    prefixes = ly_type_get_prefixes(ctx, value, value_len, resolve_prefix, parser);

    /* parse the value */
    ret = ly_path_parse(ctx, ctx_scnode, value, value_len, LY_PATH_BEGIN_ABSOLUTE, LY_PATH_LREF_FALSE,
                        prefix_opt, LY_PATH_PRED_SIMPLE, &exp);
    if (ret) {
        erc = asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - syntax error.", (int)value_len, value);
        goto error;
    }

    /* resolve it on schema tree */
    ret = ly_path_compile(ctx, ctx_scnode->module, NULL, exp, LY_PATH_LREF_FALSE, lysc_is_output(ctx_scnode) ?
                          LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_SINGLE, ly_type_stored_prefixes_clb,
                          prefixes, format, &path);
    if (ret) {
        erc = asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - semantic error.", (int)value_len, value);
        goto error;
    }

    /* find it in data */
    if (!(options & LY_TYPE_OPTS_INCOMPLETE_DATA) && !(options & LY_TYPE_OPTS_SCHEMA) && type_inst->require_instance) {
        ret = ly_path_eval(path, tree, NULL);
        if (ret) {
            erc = asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - instance not found.", (int)value_len, value);
            goto error;
        }
    }

    /* store resolved schema path */
    if (options & LY_TYPE_OPTS_STORE) {
        storage->target = path;
        path = NULL;
        if (!storage->original) {
            /* it may be already present from the first call, in case this is the second */
            storage->original = lydict_insert(ctx, value_len ? value : "", value_len);
        }
    }

cleanup:
    LY_ARRAY_FOR(prefixes, u) {
        lydict_remove(ctx, prefixes[u].prefix);
    }
    LY_ARRAY_FREE(prefixes);
    ly_set_erase(&predicates, NULL);
    lyxp_expr_free(ctx, exp);
    ly_path_free(ctx, path);

    if (options & LY_TYPE_OPTS_CANONIZE) {
        /* instance-identifier does not have a canonical form (lexical representation in in XML and JSON are
         * even different) - to make it clear, the canonized form is represented as NULL */
        *canonized = NULL;
    }

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    if ((options & LY_TYPE_OPTS_INCOMPLETE_DATA) && ((options & LY_TYPE_OPTS_SCHEMA) || type_inst->require_instance)) {
        return LY_EINCOMPLETE;
    } else {
        return LY_SUCCESS;
    }

error:
    LY_ARRAY_FOR(prefixes, u) {
        lydict_remove(ctx, prefixes[u].prefix);
    }
    LY_ARRAY_FREE(prefixes);
    ly_set_erase(&predicates, NULL);
    lyxp_expr_free(ctx, exp);
    ly_path_free(ctx, path);

    if (!*err) {
        if (erc == -1) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
            ret = LY_EMEM;
        } else {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        }
    }
    return ret;
}

/**
 * @brief Comparison callback checking the instance-identifier value.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_instanceid(const struct lyd_value *val1, const struct lyd_value *val2)
{
    LY_ARRAY_COUNT_TYPE u, v;

    if (val1 == val2) {
        return LY_SUCCESS;
    } else if (!val1->target || !val2->target || LY_ARRAY_COUNT(val1->target) != LY_ARRAY_COUNT(val2->target)) {
        return LY_ENOT;
    }

    LY_ARRAY_FOR(val1->target, u) {
        struct ly_path *s1 = &val1->target[u];
        struct ly_path *s2 = &val2->target[u];

        if (s1->node != s2->node || (s1->pred_type != s2->pred_type) ||
                (s1->predicates && LY_ARRAY_COUNT(s1->predicates) != LY_ARRAY_COUNT(s2->predicates))) {
            return LY_ENOT;
        }
        if (s1->predicates) {
            LY_ARRAY_FOR(s1->predicates, v) {
                struct ly_path_predicate *pred1 = &s1->predicates[v];
                struct ly_path_predicate *pred2 = &s2->predicates[v];

                switch (s1->pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    if (pred1->position != pred2->position) {
                        return LY_ENOT;
                    }
                    break;
                case LY_PATH_PREDTYPE_LIST:
                    /* key-predicate */
                    if (pred1->key != pred2->key || ((struct lysc_node_leaf*)pred1->key)->type->plugin->compare(&pred1->value, &pred2->value)) {
                        return LY_ENOT;
                    }
                    break;
                case LY_PATH_PREDTYPE_LEAFLIST:
                    /* leaf-list predicate */
                    if (((struct lysc_node_leaflist *)s1->node)->type->plugin->compare(&pred1->value, &pred2->value)) {
                        return LY_ENOT;
                    }
                }
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Printer callback printing the instance-identifier value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_instanceid(const struct lyd_value *value, LYD_FORMAT format, ly_get_prefix_clb get_prefix, void *printer,
                         int *dynamic)
{
    LY_ARRAY_COUNT_TYPE u, v;
    char *result = NULL;

    if (!value->target && value->canonical_cache) {
        /* value was not fully processed, but we have the original value so return it's copy */
        *dynamic = 1;
        return strdup(value->canonical_cache);
    }

    if (format == LYD_XML) {
        /* everything is prefixed */
        LY_ARRAY_FOR(value->target, u) {
            ly_strcat(&result, "/%s:%s", get_prefix(value->target[u].node->module, printer), value->target[u].node->name);
            LY_ARRAY_FOR(value->target[u].predicates, v) {
                struct ly_path_predicate *pred = &value->target[u].predicates[v];
                switch (value->target[u].pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    ly_strcat(&result, "[%"PRIu64"]", pred->position);
                    break;
                case LY_PATH_PREDTYPE_LIST:
                {
                    /* key-predicate */
                    int d = 0;
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s:%s=%c%s%c]", get_prefix(pred->key->module, printer), pred->key->name, quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                    break;
                }
                case LY_PATH_PREDTYPE_LEAFLIST:
                {
                    /* leaf-list-predicate */
                    int d = 0;
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                    break;
                }
                }
            }
        }
    } else if (format == LYD_JSON) {
        /* only the first node or the node changing module is prefixed */
        struct lys_module *mod = NULL;
        LY_ARRAY_FOR(value->target, u) {
            if (mod != value->target[u].node->module) {
                mod = value->target[u].node->module;
                ly_strcat(&result, "/%s:%s", get_prefix(mod, printer), value->target[u].node->name);
            } else {
                ly_strcat(&result, "/%s", value->target[u].node->name);
            }
            LY_ARRAY_FOR(value->target[u].predicates, v) {
                struct ly_path_predicate *pred = &value->target[u].predicates[v];
                switch (value->target[u].pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    ly_strcat(&result, "[%"PRIu64"]", pred->position);
                    break;
                case LY_PATH_PREDTYPE_LIST:
                {
                    /* key-predicate */
                    int d = 0;
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s=%c%s%c]", pred->key->name, quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                    break;
                }
                case LY_PATH_PREDTYPE_LEAFLIST:
                {
                    /* leaf-list-predicate */
                    int d = 0;
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                    break;
                }
                }
            }
        }
    } else {
        /* not supported format */
        free(result);
        return NULL;
    }

    *dynamic = 1;
    return result;
}

/**
 * @brief Duplication callback of the instance-identifier values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_instanceid(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    return ly_path_dup(ctx, original->target, &dup->target);
}

/**
 * @brief Free value of the YANG built-in instance-identifier types.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_instanceid(const struct ly_ctx *ctx, struct lyd_value *value)
{
    ly_path_free(ctx, value->target);
    value->target = NULL;
    ly_type_free_original(ctx, value);
}

LY_ERR
ly_type_find_leafref(const struct lysc_type_leafref *lref, const struct lyd_node *node, struct lyd_value *value,
                     const struct lyd_node *tree, struct lyd_node **target, char **errmsg)
{
    LY_ERR ret;
    struct lyxp_set set = {0};
    const char *val_str;
    int dynamic;
    uint32_t i;

    /* find all target data instances */
    ret = lyxp_eval(lref->path, LYD_SCHEMA, lref->path_context, node, LYXP_NODE_ELEM, tree, &set, 0);
    if (ret) {
        ret = LY_ENOTFOUND;
        val_str = lref->plugin->print(value, LYD_JSON, json_print_get_prefix, NULL, &dynamic);
        if (asprintf(errmsg, "Invalid leafref value \"%s\" - XPath evaluation error.", val_str) == -1) {
            *errmsg = NULL;
            ret = LY_EMEM;
        }
        if (dynamic) {
            free((char *)val_str);
        }
        goto error;
    }

    /* check whether any matches */
    for (i = 0; i < set.used; ++i) {
        if (set.val.nodes[i].type != LYXP_NODE_ELEM) {
            continue;
        }

        if (!lref->plugin->compare(&((struct lyd_node_term *)set.val.nodes[i].node)->value, value)) {
            break;
        }
    }
    if (i == set.used) {
        ret = LY_ENOTFOUND;
        val_str = lref->plugin->print(value, LYD_JSON, json_print_get_prefix, NULL, &dynamic);
        if (asprintf(errmsg, "Invalid leafref value \"%s\" - no target instance \"%s\" with the same value.", val_str,
                     lref->path->expr) == -1) {
            *errmsg = NULL;
            ret = LY_EMEM;
        }
        if (dynamic) {
            free((char *)val_str);
        }
        goto error;
    }

    if (target) {
        *target = set.val.nodes[i].node;
    }

    lyxp_set_free_content(&set);
    return LY_SUCCESS;

error:
    lyxp_set_free_content(&set);
    return ret;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in leafref type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_leafref(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                      ly_resolve_prefix_clb resolve_prefix, void *parser, LYD_FORMAT format,
                      const void *context_node, const struct lyd_node *tree,
                      struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    char *errmsg = NULL;
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref*)type;
    int storage_dummy = 0;
    const char *orig = NULL;
    struct ly_path *p = NULL;
    struct ly_set *set = NULL;

    if (!type_lr->realtype) {
        if ((options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_INCOMPLETE_DATA)) {
            /* leafref's path was not yet resolved - in schema trees, path can be resolved when
             * the complete schema tree is present, in such a case we need to wait with validating
             * default values */

            /* keep the original value for the second call */
            if (options & LY_TYPE_OPTS_STORE) {
                storage->original = lydict_insert(ctx, value_len ? value : "", value_len);
            }
            return LY_EINCOMPLETE;
        } else {
            LOGINT(ctx);
            return LY_EINT;
        }
    }

    if (!(options & (LY_TYPE_OPTS_STORE | LY_TYPE_OPTS_INCOMPLETE_DATA)) && type_lr->require_instance) {
        /* if there is no storage, but we will check the instance presence in data tree(s),
         * we need some (dummy) storage for data comparison */
        storage = calloc(1, sizeof *storage);
        storage_dummy = 1;
    }
    /* rewrite leafref plugin stored in the storage by default */
    storage->realtype = type_lr->realtype;

    if ((options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_SECOND_CALL)) {
        /* second call after missing resolved path (target's type) for a default value */
        /* correct refcounts - leafref's realtype was set to itself in the first incomplete call,
         * since we are now changing realtype to point to the target's realtype, we have to decrese
         * leafref's refcount and increase realtype's refcount. */
        type->refcount--;
        storage->realtype->refcount++;

        /* remove temporarily stored original value and let the following store callback
         * to replace it via a target's type functions */
        orig = storage->original;
        storage->original = NULL;

        /* hide the LY_TYPE_OPTS_SECOND_CALL option from the target's store callback, the option is connected
         * only with the leafref's path, so it is not supposed to be used here. If the previous LY_EINCOMPLETE would
         * be caused by the target's type, the target type's callback would be used directly, not via leafref's callback */
        options &= ~LY_TYPE_OPTS_SECOND_CALL;
    }

    /* check value according to the real type of the leafref target */
    ret = type_lr->realtype->plugin->store(ctx, type_lr->realtype, value, value_len, options,
                                           resolve_prefix, parser, format, context_node, tree,
                                           storage, canonized, err);
    if (ret != LY_SUCCESS && ret != LY_EINCOMPLETE) {
        goto cleanup;
    }

    if (!(options & LY_TYPE_OPTS_SCHEMA) && type_lr->require_instance) {
        if (options & LY_TYPE_OPTS_INCOMPLETE_DATA) {
            ret = LY_EINCOMPLETE;
            goto cleanup;
        }

        /* check leafref target existence */
        ret = ly_type_find_leafref(type_lr, (struct lyd_node *)context_node, storage, tree, NULL, &errmsg);
        if (ret) {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

cleanup:
    if (storage_dummy) {
        storage->realtype->plugin->free(ctx, storage);
        free(storage);
    }
    lydict_remove(ctx, orig);
    ly_path_free(ctx, p);
    ly_set_free(set, NULL);
    return ret;
}

/**
 * @brief Comparison callback checking the leafref value.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_leafref(const struct lyd_value *val1, const struct lyd_value *val2)
{
    return val1->realtype->plugin->compare(val1, val2);
}

/**
 * @brief Printer callback printing the leafref value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_leafref(const struct lyd_value *value, LYD_FORMAT format, ly_get_prefix_clb get_prefix, void *printer, int *dynamic)
{
    return value->realtype->plugin->print(value, format, get_prefix, printer, dynamic);
}

/* @brief Duplication callback of the leafref values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_leafref(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    return original->realtype->plugin->duplicate(ctx, original, dup);
}

/**
 * @brief Free value of the YANG built-in leafref type.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_leafref(const struct ly_ctx *ctx, struct lyd_value *value)
{
    if (value->realtype->plugin != &ly_builtin_type_plugins[LY_TYPE_LEAFREF]) {
        /* leafref's realtype is again leafref only in case of incomplete store */
        value->realtype->plugin->free(ctx, value);
    } else {
        /* freeing incomplete type, there is original value to free */
        ly_type_free_original(ctx, value);
    }
}

/**
 * @brief Answer if the type is suitable for the parser's hit (if any) in the specified format
 */
LY_ERR
type_check_parser_hint(LYD_FORMAT format, int hint, LY_DATA_TYPE type)
{
    if (format == LYD_JSON && hint) {
        switch (type) {
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
            if (hint != LY_TYPE_OPTS_ISNUMBER) {
                return LY_ENOT;
            }
            break;
        case LY_TYPE_STRING:
        case LY_TYPE_UINT64:
        case LY_TYPE_INT64:
        case LY_TYPE_DEC64:
        case LY_TYPE_ENUM:
        case LY_TYPE_BITS:
        case LY_TYPE_BINARY:
        case LY_TYPE_IDENT:
        case LY_TYPE_INST:
            if (hint != LY_TYPE_OPTS_ISSTRING) {
                return LY_ENOT;
            }
            break;
        case LY_TYPE_BOOL:
            if (hint != LY_TYPE_OPTS_ISBOOLEAN) {
                return LY_ENOT;
            }
            break;
        case LY_TYPE_EMPTY:
            if (hint != LY_TYPE_OPTS_ISEMPTY) {
                return LY_ENOT;
            }
            break;
        default:
            LOGINT_RET(NULL);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in union type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_union(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                    ly_resolve_prefix_clb resolve_prefix, void *parser, LYD_FORMAT format,
                    const void *context_node, const struct lyd_node *tree,
                    struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_type_union *type_u = (struct lysc_type_union*)type;
    struct lyd_value_subvalue *subvalue;
    char *errmsg = NULL;
    int prev_lo;

    if ((options & LY_TYPE_OPTS_SECOND_CALL) && (options & LY_TYPE_OPTS_STORE)) {
        subvalue = storage->subvalue;

        /* call the callback second time */
        ret = subvalue->value->realtype->plugin->store(ctx, subvalue->value->realtype, value, value_len,
                                                       options & ~(LY_TYPE_OPTS_CANONIZE | LY_TYPE_OPTS_DYNAMIC),
                                                       ly_type_stored_prefixes_clb, subvalue->prefixes, format,
                                                       context_node, tree, subvalue->value, canonized, err);
        if (ret) {
            /* second call failed, we have to try another subtype of the union.
             * Unfortunately, since the realtype can change (e.g. in leafref), we are not able to detect
             * which of the subtype's were tried the last time, so we have to try all of them.
             * We also have to remove the LY_TYPE_OPTS_SECOND_CALL flag since the callbacks will be now
             * called for the first time.
             * In the second call we should have all the data instances, so the LY_EINCOMPLETE should not
             * happen again.
             */
            options = options & ~LY_TYPE_OPTS_SECOND_CALL;
            ly_err_free(*err);
            *err = NULL;
            goto search_subtype;
        }
    } else {
        /* prepare subvalue storage */
        subvalue = calloc(1, sizeof *subvalue);
        subvalue->value = calloc(1, sizeof *subvalue->value);

        /* store prefixes for later use */
        subvalue->prefixes = ly_type_get_prefixes(ctx, value, value_len, resolve_prefix, parser);

        /* remember the hint options */
        subvalue->parser_hint = options & LY_TYPE_PARSER_HINTS_MASK;

search_subtype:
        /* use the first usable sybtype to store the value */
        LY_ARRAY_FOR(type_u->types, u) {
            subvalue->value->realtype = type_u->types[u];

            if (type_check_parser_hint(format, subvalue->parser_hint, subvalue->value->realtype->basetype)) {
                /* not a suitable type */
                continue;
            }

            /* turn logging off */
            prev_lo = ly_log_options(0);
            ret = type_u->types[u]->plugin->store(ctx, type_u->types[u], value, value_len, options & ~LY_TYPE_OPTS_DYNAMIC,
                                                  ly_type_stored_prefixes_clb, subvalue->prefixes, format,
                                                  context_node, tree, subvalue->value, canonized, err);
            /* restore logging */
            ly_log_options(prev_lo);
            if (ret == LY_SUCCESS || ret == LY_EINCOMPLETE) {
                /* success (or not yet complete) */
                break;
            }
            ly_err_free(*err);
            *err = NULL;
        }

        if (u == LY_ARRAY_COUNT(type_u->types)) {
            if (asprintf(&errmsg, "Invalid union value \"%.*s\" - no matching subtype found.", (int)value_len, value) == -1) {
                *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
                ret = LY_EMEM;
            } else {
                *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
                ret = LY_EVALID;
            }
            LY_ARRAY_FOR(subvalue->prefixes, u) {
                lydict_remove(ctx, subvalue->prefixes[u].prefix);
            }
            LY_ARRAY_FREE(subvalue->prefixes);
            free(subvalue->value);
            free(subvalue);
            if ((options & LY_TYPE_OPTS_SECOND_CALL) && (options & LY_TYPE_OPTS_STORE)) {
                storage->subvalue = NULL;
            }
            return ret;
        }

        if ((options & LY_TYPE_OPTS_STORE) && !storage->original) {
            storage->original = lydict_insert(ctx, value_len ? value : "", value_len);
        }
    }
    /* success */

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    if (options & LY_TYPE_OPTS_STORE) {
        storage->subvalue = subvalue;
    } else {
        LY_ARRAY_FOR(subvalue->prefixes, u) {
            lydict_remove(ctx, subvalue->prefixes[u].prefix);
        }
        LY_ARRAY_FREE(subvalue->prefixes);
        free(subvalue->value);
        free(subvalue);
    }

    return ret;
}

/**
 * @brief Comparison callback checking the union value.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_union(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->subvalue->value->realtype != val2->subvalue->value->realtype) {
        return LY_ENOT;
    }
    return val1->subvalue->value->realtype->plugin->compare(val1->subvalue->value, val2->subvalue->value);
}

/**
 * @brief Printer callback printing the union value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_union(const struct lyd_value *value, LYD_FORMAT format, ly_get_prefix_clb get_prefix, void *printer, int *dynamic)
{
    return value->subvalue->value->realtype->plugin->print(value->subvalue->value, format, get_prefix, printer, dynamic);
}

/* @brief Duplication callback of the union values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ARRAY_COUNT_TYPE u;

    dup->subvalue = calloc(1, sizeof *dup->subvalue);
    LY_CHECK_ERR_RET(!dup->subvalue, LOGMEM(ctx), LY_EMEM);
    if (original->subvalue->prefixes) {
        LY_ARRAY_CREATE_RET(ctx, dup->subvalue->prefixes, LY_ARRAY_COUNT(original->subvalue->prefixes), LY_EMEM);
        LY_ARRAY_FOR(original->subvalue->prefixes, u) {
            LY_ARRAY_INCREMENT(dup->subvalue->prefixes);
            dup->subvalue->prefixes[u].mod = original->subvalue->prefixes[u].mod;
            dup->subvalue->prefixes[u].prefix = lydict_insert(ctx, original->subvalue->prefixes[u].prefix, 0);
        }
    }
    dup->subvalue->value = calloc(1, sizeof *dup->subvalue->value);
    LY_CHECK_ERR_RET(!dup->subvalue->value, LOGMEM(ctx), LY_EMEM);
    dup->subvalue->value->realtype = original->subvalue->value->realtype;
    dup->subvalue->value->realtype->plugin->duplicate(ctx, original->subvalue->value, dup->subvalue->value);

    return ly_type_dup_original(ctx, original, dup);
}

/**
 * @brief Free value of the YANG built-in union type.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value)
{
    LY_ARRAY_COUNT_TYPE u;

    if (value->subvalue) {
        if (value->subvalue->value) {
            value->subvalue->value->realtype->plugin->free(ctx, value->subvalue->value);
            LY_ARRAY_FOR(value->subvalue->prefixes, u) {
                lydict_remove(ctx, value->subvalue->prefixes[u].prefix);
            }
            LY_ARRAY_FREE(value->subvalue->prefixes);
            free(value->subvalue->value);
        }
        free(value->subvalue);
        value->subvalue = NULL;
    }
    ly_type_free_canonical(ctx, value);
}

/**
 * @brief Set of type plugins for YANG built-in types
 */
struct lysc_type_plugin ly_builtin_type_plugins[LY_DATA_TYPE_COUNT] = {
    {0}, /* LY_TYPE_UNKNOWN */
    {.type = LY_TYPE_BINARY, .store = ly_type_store_binary, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_canonical, .free = ly_type_free_canonical,
        .id = "libyang 2 - binary, version 1"},
    {.type = LY_TYPE_UINT8, .store = ly_type_store_uint, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_uint, .free = ly_type_free_canonical,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT16, .store = ly_type_store_uint, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_uint, .free = ly_type_free_canonical,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT32, .store = ly_type_store_uint, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_uint, .free = ly_type_free_canonical,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT64, .store = ly_type_store_uint, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_uint, .free = ly_type_free_canonical,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_STRING, .store = ly_type_store_string, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_original, .free = ly_type_free_original,
        .id = "libyang 2 - string, version 1"},
    {.type = LY_TYPE_BITS, .store = ly_type_store_bits, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_bits, .free = ly_type_free_bits,
        .id = "libyang 2 - bits, version 1"},
    {.type = LY_TYPE_BOOL, .store = ly_type_store_boolean, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_boolean, .free = ly_type_free_original,
        .id = "libyang 2 - boolean, version 1"},
    {.type = LY_TYPE_DEC64, .store = ly_type_store_decimal64, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_decimal64, .free = ly_type_free_canonical,
        .id = "libyang 2 - decimal64, version 1"},
    {.type = LY_TYPE_EMPTY, .store = ly_type_store_empty, .compare = ly_type_compare_empty,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_original, .free = ly_type_free_original,
        .id = "libyang 2 - empty, version 1"},
    {.type = LY_TYPE_ENUM, .store = ly_type_store_enum, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_enum, .free = ly_type_free_original,
        .id = "libyang 2 - enumeration, version 1"},
    {.type = LY_TYPE_IDENT, .store = ly_type_store_identityref, .compare = ly_type_compare_identityref,
        .print = ly_type_print_identityref, .duplicate = ly_type_dup_identityref, .free = ly_type_free_canonical,
        .id = "libyang 2 - identityref, version 1"},
    {.type = LY_TYPE_INST, .store = ly_type_store_instanceid, .compare = ly_type_compare_instanceid,
        .print = ly_type_print_instanceid, .duplicate = ly_type_dup_instanceid, .free = ly_type_free_instanceid,
        .id = "libyang 2 - instance-identifier, version 1"},
    {.type = LY_TYPE_LEAFREF, .store = ly_type_store_leafref, .compare = ly_type_compare_leafref,
        .print = ly_type_print_leafref, .duplicate = ly_type_dup_leafref, .free = ly_type_free_leafref,
        .id = "libyang 2 - leafref, version 1"},
    {.type = LY_TYPE_UNION, .store = ly_type_store_union, .compare = ly_type_compare_union,
        .print = ly_type_print_union, .duplicate = ly_type_dup_union, .free = ly_type_free_union,
        .id = "libyang 2 - union,version 1"},
    {.type = LY_TYPE_INT8, .store = ly_type_store_int, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_int, .free = ly_type_free_canonical,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT16, .store = ly_type_store_int, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_int, .free = ly_type_free_canonical,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT32, .store = ly_type_store_int, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_int, .free = ly_type_free_canonical,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT64, .store = ly_type_store_int, .compare = ly_type_compare_canonical,
        .print = ly_type_print_canonical, .duplicate = ly_type_dup_int, .free = ly_type_free_canonical,
        .id = "libyang 2 - integer, version 1"},
};
