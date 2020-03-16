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
#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plugins_types.h"
#include "dict.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "tree_data_internal.h"
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
                        ly_clb_get_prefix UNUSED(get_prefix), void *UNUSED(printer), int *dynamic)
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

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    /* consume leading whitespaces */
    for (;value_len && isspace(*value); ++value, --value_len);

    if (!value || !value[0] || !value_len) {
        asprintf(&errmsg, "Invalid empty %s value.", datatype);
        goto error;
    }

    switch(ly_parse_int(value, value_len, min, max, base, ret)) {
    case LY_EDENIED:
        asprintf(&errmsg, "Value is out of %s's min/max bounds.", datatype);
        goto error;
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        asprintf(&errmsg, "Invalid %s value \"%.*s\".", datatype, (int)value_len, value);
        goto error;
    }

error:
    *err = ly_err_new(LY_LLERR, LY_EINVAL, LYVE_DATA, errmsg, NULL, NULL);
    return LY_EVALID;
}

API struct lyd_value_prefix *
ly_type_get_prefixes(const struct ly_ctx *ctx, const char *value, size_t value_len, ly_clb_resolve_prefix resolve_prefix,
                     void *parser)
{
    LY_ERR ret;
    unsigned int c;
    const char *start, *stop;
    struct lyd_value_prefix *prefixes = NULL;
    const struct lys_module *mod;
    unsigned int u;

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

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    /* consume leading whitespaces */
    for (;value_len && isspace(*value); ++value, --value_len);

    if (!value || !value[0] || !value_len) {
        asprintf(&errmsg, "Invalid empty %s value.", datatype);
        goto error;
    }

    *err = NULL;
    switch (ly_parse_uint(value, value_len, max, base, ret)) {
    case LY_EDENIED:
        asprintf(&errmsg, "Value \"%.*s\" is out of %s's min/max bounds.", (int)value_len, value, datatype);
        goto error;
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        asprintf(&errmsg, "Invalid %s value \"%.*s\".", datatype, (int)value_len, value);
        goto error;
    }

error:
    *err = ly_err_new(LY_LLERR, LY_EINVAL, LYVE_DATA, errmsg, NULL, NULL);
    return LY_EVALID;
}

API LY_ERR
ly_type_parse_dec64(uint8_t fraction_digits, const char *value, size_t value_len, int64_t *ret, struct ly_err_item **err)
{
    LY_ERR rc = LY_EINVAL;
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
        asprintf(&errmsg, "Invalid %lu. character of decimal64 value \"%.*s\".",
                 len + 1, (int)value_len, value);
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
        asprintf(&errmsg, "Value \"%.*s\" of decimal64 type exceeds defined number (%u) of fraction digits.", (int)len, value,
                 fraction_digits);
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
            asprintf(&errmsg, "Invalid %lu. character of decimal64 value \"%.*s\".",
                     u + 1, (int)value_len, value);
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
        *err = ly_err_new(LY_LLERR, LY_EINVAL, LYVE_DATA, errmsg, NULL, NULL);
    }
    return rc;
}

API LY_ERR
ly_type_validate_patterns(struct lysc_pattern **patterns, const char *str, size_t str_len, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int rc;
    unsigned int u;
    char *errmsg;
    pcre2_match_data *match_data = NULL;

    LY_CHECK_ARG_RET(NULL, str, err, LY_EINVAL);

    LY_ARRAY_FOR(patterns, u) {
        match_data = pcre2_match_data_create_from_pattern(patterns[u]->code, NULL);
        if (!match_data) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, "Memory allocation failed.", NULL, NULL);
            return LY_EMEM;
        }

        rc = pcre2_match(patterns[u]->code, (PCRE2_SPTR)str, str_len, 0, PCRE2_ANCHORED | PCRE2_ENDANCHORED, match_data, NULL);
        if (rc == PCRE2_ERROR_NOMATCH) {
            asprintf(&errmsg, "String \"%.*s\" does not conform to the %u. pattern restriction of its type.",
                     (int)str_len, str, u + 1);
            *err = ly_err_new(LY_LLERR, LY_ESYS, 0, errmsg, NULL, NULL);
            ret = LY_EVALID;
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
    unsigned int u;
    char *errmsg = NULL;

    LY_ARRAY_FOR(range->parts, u) {
        if (basetype < LY_TYPE_DEC64) {
            /* unsigned */
            if ((uint64_t)value < range->parts[u].min_u64) {
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    asprintf(&errmsg, "%s \"%s\" does not satisfy the %s constraint.",
                           (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "Length" : "Value", strval,
                           (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "length" : "range");
                }
                goto error;
            } else if ((uint64_t)value <= range->parts[u].max_u64) {
                /* inside the range */
                return LY_SUCCESS;
            } else if (u == LY_ARRAY_SIZE(range->parts) - 1) {
                /* we have the last range part, so the value is out of bounds */
                if (range->emsg) {
                    errmsg = strdup(range->emsg);
                } else {
                    asprintf(&errmsg, "%s \"%s\" does not satisfy the %s constraint.",
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
                    asprintf(&errmsg, "Value \"%s\" does not satisfy the range constraint.", strval);
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
                    asprintf(&errmsg, "Value \"%s\" does not satisfy the range constraint.", strval);
                }
                goto error;
            }
        }
    }

    return LY_SUCCESS;

error:
    *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, range->eapptag ? strdup(range->eapptag) : NULL);
    return LY_EVALID;
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
                  ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
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
        LOGINT(NULL);
        return LY_EINVAL;
    }

    asprintf(&str, "%"PRId64, i);

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
                   ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
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
        LOGINT(NULL);
        return LY_EINVAL;
    }

    asprintf(&str, "%"PRIu64, u);

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
                        ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
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
        *err = ly_err_new(LY_LLERR, LY_EINVAL, LYVE_DATA, strdup("Invalid empty decimal64 value."), NULL, NULL);
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
                     ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                     const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                     struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    size_t start = 0, stop = 0, count = 0, u, termination = 0;
    struct lysc_type_bin *type_bin = (struct lysc_type_bin *)type;
    char *errmsg;

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
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
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
                     ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
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
                   ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
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
    unsigned int u, v;
    char *errmsg = NULL;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits*)type;
    int iscanonical = 1;
    size_t ws_count;
    size_t lws_count; /* leading whitespace count */
    const char *can = NULL;

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
                    if (!lysc_iffeature_value(&type_bits->bits[u].iffeatures[v])) {
                        asprintf(&errmsg, "Bit \"%s\" is disabled by its %u. if-feature condition.",
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
                    asprintf(&errmsg, "Bit \"%s\" used multiple times.", type_bits->bits[u].name);
                    goto error;
                }
                goto next;
            }
        }
        /* item not found */
        asprintf(&errmsg, "Invalid bit value \"%.*s\".", (int)item_len, item);
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
            for (u = 0; u < items_ordered->count; u++) {
                storage->bits_items[u] = items_ordered->objs[u];
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
    if (errmsg) {
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
    unsigned int u;

    LY_ARRAY_CREATE_RET(ctx, dup->bits_items, LY_ARRAY_SIZE(original->bits_items), LY_EMEM);
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
                   ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                   const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                   struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    unsigned int u, v;
    char *errmsg = NULL;
    struct lysc_type_enum *type_enum = (struct lysc_type_enum*)type;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    /* find the matching enumeration value item */
    LY_ARRAY_FOR(type_enum->enums, u) {
        if (!ly_strncmp(type_enum->enums[u].name, value, value_len)) {
            /* we have the match */

            /* check that the enumeration value is not disabled */
            LY_ARRAY_FOR(type_enum->enums[u].iffeatures, v) {
                if (!lysc_iffeature_value(&type_enum->enums[u].iffeatures[v])) {
                    asprintf(&errmsg, "Enumeration \"%s\" is disabled by its %u. if-feature condition.",
                             type_enum->enums[u].name, v + 1);
                    goto error;
                }
            }
            goto match;
        }
    }
    /* enum not found */
    asprintf(&errmsg, "Invalid enumeration value \"%.*s\".", (int)value_len, value);
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
    if (errmsg) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
    }
    return LY_EVALID;
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
                      ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
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
        asprintf(&errmsg, "Invalid boolean value \"%.*s\".", (int)value_len, value);
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        return LY_EVALID;
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
                    ly_clb_resolve_prefix UNUSED(resolve_prefix), void *UNUSED(parser), LYD_FORMAT UNUSED(format),
                    const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                    struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    if (value_len) {
        char *errmsg;
        asprintf(&errmsg, "Invalid empty value \"%.*s\".", (int)value_len, value);
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        return LY_EVALID;
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
    unsigned int u;

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
                          ly_clb_resolve_prefix resolve_prefix, void *parser, LYD_FORMAT UNUSED(format),
                          const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
                          struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    struct lysc_type_identityref *type_ident = (struct lysc_type_identityref *)type;
    const char *id_name, *prefix = value;
    size_t id_len, prefix_len;
    char *errmsg = NULL;
    const struct lys_module *mod;
    unsigned int u;
    struct lysc_ident *ident;

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
        asprintf(&errmsg, "Invalid identityref \"%.*s\" value - unable to map prefix to YANG schema.", (int)value_len, value);
        goto error;
    }
    LY_ARRAY_FOR(mod->compiled->identities, u) {
        ident = &mod->compiled->identities[u]; /* shortcut */
        if (!ly_strncmp(ident->name, id_name, id_len)) {
            /* we have match */
            break;
        }
    }
    if (u == LY_ARRAY_SIZE(mod->compiled->identities)) {
        /* no match */
        asprintf(&errmsg, "Invalid identityref \"%.*s\" value - identity not found.", (int)value_len, value);
        goto error;
    }

    /* check that the identity matches some of the type's base identities */
    LY_ARRAY_FOR(type_ident->bases, u) {
        if (!ly_type_identity_isderived(type_ident->bases[u], ident)) {
            /* we have match */
            break;
        }
    }
    if (u == LY_ARRAY_SIZE(type_ident->bases)) {
        /* no match */
        asprintf(&errmsg, "Invalid identityref \"%.*s\" value - identity not accepted by the type specification.", (int)value_len, value);
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
    *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
    return LY_EVALID;
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
ly_type_print_identityref(const struct lyd_value *value, LYD_FORMAT UNUSED(format), ly_clb_get_prefix get_prefix, void *printer, int *dynamic)
{
    char *result = NULL;

    *dynamic = 1;
    asprintf(&result, "%s:%s", get_prefix(value->ident->module, printer), value->ident->name);

    return result;
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
 * @brief Helper function for ly_type_store_instanceid() to check presence of the specific node in the (data/schema) tree.
 *
 * In the case the instance-identifier type does not require instance (@p require_instance is 0) or the data @p trees
 * are not complete (called in the middle of data parsing), the @p token is checked only to match schema tree.
 * Otherwise, the provided data @p trees are used to find instance of the node specified by the token and @p node_d as
 * its parent.
 *
 * @param[in] orig Complete instance-identifier expression for logging.
 * @param[in] orig_len Length of the @p orig string.
 * @param[in] options [Type plugin options ](@ref plugintypeopts) - only LY_TYPE_OPTS_INCOMPLETE_DATA is used.
 * @param[in] require_instance Flag if the instance-identifier requires presence of an instance in the data tree.
 * If the flag is zero, the data tree is not needed and the @p token is checked only by checking the schema tree.
 * @param[in,out] token Pointer to the specific position inside the @p orig string where the node-identifier starts.
 * The pointer is updated to point after the processed node-identifier.
 * @param[in] prefixes [Sized array](@ref sizedarrays) of known mappings between prefix used in the @p orig and modules from the context.
 * @param[in] format Format of the input YANG data, since XML and JSON format differs in case of instance-identifiers.
 * @param[in,out] node_s Parent schema node as input, resolved schema node as output. Alternative parameter for @p node_d
 * in case the instance is not available (@p trees are not yet complete) or required.
 * @param[in,out] node_d Parent data node as input, resolved data node instance as output. Alternative parameter for @p node_s
 * in case the instance is required and @p trees are complete.
 * @param[in] trees [Sized array](@ref sizedarrays)) of data trees where the data instance is supposed to be present.
 * @param[out] errmsg Error message in case of failure. Function does not log on its own, instead it creates error message. Caller is supposed to
 * free (or store somewhere) the returned message.
 * @return LY_SUCCESS when node found.
 * @return LY_EMEM or LY_EVALID in case of failure or when the node is not present in the schema/data tree.
 */
static LY_ERR
ly_type_store_instanceid_checknodeid(const char *orig, size_t orig_len, int options, int require_instance,
                                     const char **token, struct lyd_value_prefix *prefixes, LYD_FORMAT format,
                                     const struct lysc_node **node_s, const struct lyd_node **node_d,
                                     const struct lyd_node *tree, char **errmsg)
{
    const char *id, *prefix;
    size_t id_len, prefix_len;
    const struct lys_module *mod = NULL;
    unsigned int u;

    if (ly_parse_nodeid(token, &prefix, &prefix_len, &id, &id_len)) {
        asprintf(errmsg, "Invalid instance-identifier \"%.*s\" value at character %lu (%.*s).",
                 (int)orig_len, orig, *token - orig + 1, (int)(orig_len - (*token - orig)), *token);
        return LY_EVALID;
    }
    if (!prefix || !prefix_len) {
        if (format == LYD_XML || (!*node_s && !*node_d)) {
            asprintf(errmsg, "Invalid instance-identifier \"%.*s\" value - all node names (%.*s) MUST be qualified with explicit namespace prefix.",
                     (int)orig_len, orig, (int)id_len + 1, &id[-1]);
            return LY_EVALID;
        }

        /* non top-level node from JSON */
        if (*node_d) {
            mod = (*node_d)->schema->module;
        } else {
            mod = (*node_s)->module;
        }
    } else {
        /* map prefix to schema module */
        LY_ARRAY_FOR(prefixes, u) {
            if (!ly_strncmp(prefixes[u].prefix, prefix, prefix_len)) {
                mod = prefixes[u].mod;
                break;
            }
        }

        if (!mod) {
            asprintf(errmsg, "Invalid instance-identifier \"%.*s\" value - unable to map prefix \"%.*s\" to YANG schema.",
                     (int)orig_len, orig, (int)prefix_len, prefix);
            return LY_EVALID;
        }
    }

    if ((options & LY_TYPE_OPTS_INCOMPLETE_DATA) || (options & LY_TYPE_OPTS_SCHEMA) || !require_instance) {
        /* a) in schema tree */
        *node_s = lys_find_child(*node_s, mod, id, id_len, 0, 0);
        if (!(*node_s)) {
            asprintf(errmsg, "Invalid instance-identifier \"%.*s\" value - path \"%.*s\" does not exists in the YANG schema.",
                     (int)orig_len, orig, (int)(*token - orig), orig);
            return LY_EVALID;
        }
    } else {
        /* b) in data tree */
        if (*node_d) {
            /* inner node */
            lyd_find_sibling_next(lyd_node_children(*node_d), mod, id, id_len, NULL, 0, (struct lyd_node **)node_d);
            if (!*node_d) {
                asprintf(errmsg, "Invalid instance-identifier \"%.*s\" value - path \"%.*s\" does not exists in the data tree(s).",
                         (int)orig_len, orig, (int)(*token - orig), orig);
                return LY_EVALID;
            }
        } else {
            /* top-level node */
            lyd_find_sibling_next(tree, mod, id, id_len, NULL, 0, (struct lyd_node **)node_d);
            if (!(*node_d)) {
                /* node not found */
                asprintf(errmsg, "Invalid instance-identifier \"%.*s\" value - path \"%.*s\" does not exists in the data tree(s).",
                         (int)orig_len, orig, (int)(*token - orig), orig);
                return LY_EVALID;
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Helper function for ly_type_store_instanceid_parse_predicate_value() to provide prefix mapping for the
 * validation callbacks for the values used in instance-identifier predicates.
 *
 * Implementation of the ly_clb_resolve_prefix.
 */
static const struct lys_module *
ly_type_stored_prefixes_clb(const struct ly_ctx *UNUSED(ctx), const char *prefix, size_t prefix_len, void *private)
{
    struct lyd_value_prefix *prefixes = (struct lyd_value_prefix*)private;
    unsigned int u;

    LY_ARRAY_FOR(prefixes, u) {
        if (!ly_strncmp(prefixes[u].prefix, prefix, prefix_len)) {
            return prefixes[u].mod;
        }
    }
    return NULL;
}

/**
 * @brief Helper function for ly_type_store_instanceid() to prepare predicate's value structure into the lyd_value_path structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] key Schema key node of the predicate (list's key in case of key-predicate, leaf-list in case of leaf-list-predicate).
 * @param[in] val Value string of the predicate.
 * @param[in] val_len Length of the @p val.
 * @param[in] prefixes [Sized array](@ref sizedarrays) of the prefix mappings used in the instance-identifier value.
 * @param[in] format Input format of the data.
 * @param[in,out] pred Prepared predicate structure where the predicate information will be added.
 * @param[out] errmsg Error description in case the function fails. Caller is responsible to free the string.
 * @return LY_ERR value.
 */
static LY_ERR
ly_type_store_instanceid_parse_predicate_value(const struct ly_ctx *ctx, const struct lysc_node *key, const char *val,
                                               size_t val_len, struct lyd_value_prefix *prefixes, LYD_FORMAT format,
                                               struct lyd_value_path_predicate *pred, char **errmsg)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type *type;
    struct ly_err_item *err = NULL;
    int options = LY_TYPE_OPTS_STORE | LY_TYPE_OPTS_INCOMPLETE_DATA;

    pred->value = calloc(1, sizeof *pred->value);

    type = ((struct lysc_node_leaf*)key)->type;
    pred->value->realtype = type;
    ret = type->plugin->store(ctx, type, val, val_len, options, ly_type_stored_prefixes_clb, prefixes, format, key, NULL,
                              pred->value, NULL, &err);
    if (ret == LY_EINCOMPLETE) {
        /* actually expected without complete data */
        ret = LY_SUCCESS;
    } else if (ret) {
        if (err) {
            *errmsg = err->msg;
            err->msg = NULL;
            ly_err_free(err);
        } else {
            *errmsg = strdup("Type validation failed with unknown error.");
        }
        goto error;
    }

error:
    return ret;
}

/**
 * @brief Helper function for ly_type_store_instanceid() to correctly find the end of the predicate section.
 *
 * @param[in] predicate Start of the beginning section.
 * @return Pointer to the end of the predicate section.
 */
static const char *
ly_type_path_predicate_end(const char *predicate)
{
    size_t i = 0;
    while (predicate[i] != ']') {
        if (predicate[i] == '\'') {
            i++;
            while (predicate[i] != '\'') {
                i++;
            }
        }
        if (predicate[i] == '"') {
            i++;
            while (predicate[i] != '"') {
                if (predicate[i] == '\\') {
                    i += 2;
                    continue;
                }
                i++;
            }
        }
        i++;
    }
    return &predicate[i];
}

/**
 * @brief Validate and store value of the YANG built-in instance-identifier type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_instanceid(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                         ly_clb_resolve_prefix resolve_prefix, void *parser, LYD_FORMAT format,
                         const void *UNUSED(context_node), const struct lyd_node *tree,
                         struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret = LY_EVALID;
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)type;
    const char *id, *prefix, *val, *token, *node_start = NULL;
    size_t id_len, prefix_len, val_len;
    char *errmsg = NULL;
    const struct lysc_node *node_s = NULL;
    const struct lyd_node *node_d = NULL;
    struct lyd_value_prefix *prefixes = NULL;
    unsigned int u, c;
    struct lyd_value_path *target = NULL, *t;
    struct lyd_value_path_predicate *pred;
    struct ly_set predicates = {0};
    uint64_t pos;
    int i;
    const char *first_pred = NULL;

    /* init */
    *err = NULL;

    if ((options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_INCOMPLETE_DATA)) {
        /* we have incomplete schema tree, so we are actually just storing the original value for future validation */
        if (options & LY_TYPE_OPTS_STORE) {
            storage->original = lydict_insert(ctx, value_len ? value : "", value_len);
        }
        goto cleanup;
    }

    if (!(options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_SECOND_CALL) && (options & LY_TYPE_OPTS_STORE)) {
        /* the second run in data tree, the first one ended with LY_EINCOMPLETE, but we have prepared the target structure */
        if (!lyd_target(storage->target, tree)) {
            /* in error message we print the JSON format of the instance-identifier - in case of XML, it is not possible
             * to get the exactly same string as original, JSON is less demanding and still well readable/understandable. */
            int dynamic = 0;
            const char *id = storage->realtype->plugin->print(storage, LYD_JSON, json_print_get_prefix, NULL, &dynamic);
            asprintf(&errmsg, "Invalid instance-identifier \"%s\" value - required instance not found.", id);
            if (dynamic) {
                free((char*)id);
            }
            /* we have to clean up the storage */
            type->plugin->free(ctx, storage);

            goto error;
        }
        return LY_SUCCESS;
    } else {
        /* a) first run without prepared target, we will need all the prefixes used in the instance-identifier value
         * b) second run in schema tree - original value was stored as a canonical value and received now as value
         */
        prefixes = ly_type_get_prefixes(ctx, value, value_len, resolve_prefix, parser);
    }

    if (value[0] != '/') {
        asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - instance-identifier must starts with '/'.",
                 (int)value_len, value);
        goto error;
    }

    /* parse the value and try to resolve it in:
     * a) schema tree - instance is not required, just check that the path is instantiable
     * b) data tree - instance is required, so find it */
    for (token = value; (size_t)(token - value) < value_len;) {
        if (token[0] == '/') {
            /* node identifier */
            node_start = &token[1];

            /* clean them to correctly check predicates presence restrictions */
            pos = 0;
            ly_set_clean(&predicates, NULL);
            first_pred = NULL;

            token++;
            if (ly_type_store_instanceid_checknodeid(value, value_len, options, type_inst->require_instance,
                                                     &token, prefixes, format, &node_s, &node_d, tree, &errmsg)) {
                goto error;
            }

            if (node_d) {
                node_s = node_d->schema;
            }
            if (token[0] == '[') {
                /* predicate follows, this must be a list or leaf-list */
                if (node_s->nodetype != LYS_LIST && node_s->nodetype != LYS_LEAFLIST) {
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - predicate \"%.*s\" for %s is not accepted.",
                             (int)value_len, value, (int)(ly_type_path_predicate_end(token) - token) + 1, token,
                             lys_nodetype2str(node_s->nodetype));
                    goto error;
                }
            }

            if (options & LY_TYPE_OPTS_STORE) {
                /* prepare target path structure */
                LY_ARRAY_NEW_GOTO(ctx, target, t, ret, error);
                t->node = node_s;
            }
        } else if (token[0] == '[') {
            /* predicate */
            const char *pred_start = &token[0];
            const char *pred_errmsg = NULL;
            const struct lysc_node *key_s = node_s;
            const struct lyd_node *key_d = node_d;

            if (!first_pred) {
                first_pred = pred_start;
                c = predicates.count;
            }

check_predicates:
            if (ly_parse_instance_predicate(&token, value_len - (token - value), format, &prefix, &prefix_len, &id,
                    &id_len, &val, &val_len, &pred_errmsg)) {
                asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value's predicate \"%.*s\" (%s).", (int)value_len, value,
                         (int)(token - pred_start), pred_start, pred_errmsg);
                goto error;
            }

            if (options & LY_TYPE_OPTS_STORE) {
                /* update target path structure by adding predicate info */
                LY_ARRAY_NEW_GOTO(ctx, t->predicates, pred, ret, error);
            }

            if (prefix || (id && id[0] != '.')) {
                /* key-predicate */
                const char *start, *t;
                start = t = prefix ? prefix : id;

                /* check that the parent is a list */
                if (node_s->nodetype != LYS_LIST) {
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - key-predicate \"%.*s\" is accepted only for lists, not %s.",
                             (int)value_len, value, (int)(token - pred_start), pred_start, lys_nodetype2str(node_s->nodetype));
                    goto error;
                }

                /* resolve the key in predicate */
                if (ly_type_store_instanceid_checknodeid(value, value_len, options, type_inst->require_instance,
                                                            &t, prefixes, format, &key_s, &key_d, tree, &errmsg)) {
                    goto error;
                }
                if (key_d) {
                    key_s = key_d->schema;
                }

                /* check the key in predicate is really a key */
                if (!(key_s->flags & LYS_KEY)) {
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - node \"%s\" used in key-predicate \"%.*s\" must be a key.",
                            (int)value_len, value, key_s->name, (int)(token - pred_start), pred_start);
                    goto error;
                }

                if (node_d) {
                    while (node_d) {
                        if (!lyd_value_compare((const struct lyd_node_term*)key_d, val, val_len, resolve_prefix, parser, format, tree)) {
                            /* match */
                            break;
                        }
                        /* go to another instance */
                        t = start;
                        lyd_find_sibling_next(node_d->next, node_s->module, node_s->name, 0, NULL, 0, (struct lyd_node **)&node_d);
                        if (node_d) {
                            /* reset variables to the first predicate of this list to check it completely */
                            key_d = node_d;
                            pred_start = token = first_pred;
                            predicates.count = c;
                            goto check_predicates;
                        }
                    }
                    if (!node_d) {
                        const char *pathstr_end = token;
                        /* there maybe multiple predicates and we want to print all here, since them all identify the instance */
                        for (pathstr_end = ly_type_path_predicate_end(token);
                                pathstr_end[1] == '[';
                                pathstr_end = ly_type_path_predicate_end(pathstr_end + 1));
                        pathstr_end++;
                        asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - key-predicate \"%.*s\" does not match any \"%s\" instance.",
                                 (int)value_len, value, (int)(pathstr_end - first_pred), first_pred, node_s->name);
                        goto error;
                    }
                } else {
                    /* check value to the type */
                    if (lys_value_validate(NULL, key_s, val, val_len, resolve_prefix, parser, format)) {
                        asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - key-predicate \"%.*s\"'s key value is invalid.",
                                 (int)value_len, value, (int)(token - pred_start), pred_start);
                        goto error;
                    }
                }

                if (options & LY_TYPE_OPTS_STORE) {
                    /* update target path structure by adding predicate info */
                    pred->type = 1;
                    pred->key = key_s;
                    LY_CHECK_GOTO(ly_type_store_instanceid_parse_predicate_value(ctx, key_s, val, val_len, prefixes, format, pred, &errmsg), error);
                }
                i = predicates.count;
                if (i != ly_set_add(&predicates, (void*)key_s, 0)) {
                    /* the same key is used repeatedly */
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - key \"%s\" is referenced the second time in key-predicate \"%.*s\".",
                             (int)value_len, value, key_s->name, (int)(token - pred_start), pred_start);
                    goto error;
                }
                if (token[0] != '[') {
                    /* now we should have all the keys */
                    unsigned int keys_count = 0;
                    for (struct lysc_node *key = ((struct lysc_node_list*)node_s)->child;
                            key && key->nodetype == LYS_LEAF && (key->flags & LYS_KEY);
                            keys_count++, key = key->next);
                    if (keys_count != predicates.count) {
                        asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - missing %u key(s) for the list instance \"%.*s\".",
                                 (int)value_len, value, keys_count - predicates.count, (int)(token - node_start), node_start);
                        goto error;
                    }
                }
            } else if (id) {
                /* leaf-list-predicate */
                if (predicates.count) {
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - "
                             "leaf-list-predicate (\"%.*s\") cannot be used repeatedly for a single node.",
                             (int)value_len, value, (int)(token - pred_start), pred_start);
                    goto error;
                }

                /* check that the parent is a leaf-list */
                if (node_s->nodetype != LYS_LEAFLIST) {
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - leaf-list-predicate \"%.*s\" is accepted only for leaf-lists, not %s.",
                             (int)value_len, value, (int)(token - pred_start), pred_start, lys_nodetype2str(node_s->nodetype));
                    goto error;
                }

                if (key_d) {
                    while (key_d) {
                        if (!lyd_value_compare((const struct lyd_node_term*)key_d, val, val_len, resolve_prefix, parser, format, tree)) {
                            /* match */
                            break;
                        }
                        /* go to another instance */
                        lyd_find_sibling_next(key_d->next, node_s->module, node_s->name, 0, NULL, 0, (struct lyd_node **)&key_d);
                    }
                    if (!key_d) {
                        asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - leaf-list-predicate \"%.*s\" does not match any \"%s\" instance.",
                                 (int)value_len, value, (int)(token - pred_start), pred_start, node_s->name);
                        goto error;
                    }
                } else {
                    /* check value to the type */
                    if (lys_value_validate(NULL, key_s, val, val_len, resolve_prefix, parser, format)) {
                        asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - leaf-list-predicate \"%.*s\"'s value is invalid.",
                                 (int)value_len, value, (int)(token - pred_start), pred_start);
                        goto error;
                    }
                }

                if (options & LY_TYPE_OPTS_STORE) {
                    /* update target path structure by adding predicate info */
                    pred->type = 2;
                    pred->key = node_s;
                    LY_CHECK_GOTO(ly_type_store_instanceid_parse_predicate_value(ctx, node_s, val, val_len, prefixes, format, pred, &errmsg), error);
                }
                ly_set_add(&predicates, (void*)node_s, 0);
            } else {
                /* pos predicate */
                if (pos) {
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - "
                             "position predicate (\"%.*s\") cannot be used repeatedly for a single node.",
                             (int)value_len, value, (int)(token - pred_start), pred_start);
                    goto error;
                }

                /* check that the parent is a list without keys
                 * The check is actually commented, libyang does not want to be so strict here, since
                 * we see usecases even for lists with keys and leaf-lists
                if (node_s->nodetype != LYS_LIST || ((struct lysc_node_list*)node_s)->keys) {
                    asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - position predicate \"%.*s\" is accepted only for lists without keys.",
                             (int)value_len, value, (int)(token - pred_start), pred_start, lys_nodetype2str(node_s->nodetype));
                    goto error;
                }
                */

                if (ly_type_parse_uint("instance-identifier", 10, UINT64_MAX, val, val_len, &pos, err)) {
                    goto error;
                }
                if (node_d) {
                    /* get the correct instance */
                    for (uint64_t u = pos; u > 1; u--) {
                        lyd_find_sibling_next(node_d->next, node_s->module, node_s->name, 0, NULL, 0, (struct lyd_node **)&node_d);
                        if (!node_d) {
                            asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - "
                                     "position-predicate %"PRIu64" is bigger than number of instances in the data tree (%"PRIu64").",
                                     (int)value_len, value, pos, pos - u + 1);
                            goto error;
                        }
                    }
                } else {
                    /* check that there is not a limit below the position number */
                    if (node_s->nodetype == LYS_LIST) {
                        if (((struct lysc_node_list*)node_s)->max < pos) {
                            asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - position-predicate %"PRIu64" is bigger than allowed max-elements (%u).",
                                     (int)value_len, value, pos, ((struct lysc_node_list*)node_s)->max);
                            goto error;
                        }
                    } else {
                        if (((struct lysc_node_leaflist*)node_s)->max < pos) {
                            asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - position-predicate %"PRIu64" is bigger than allowed max-elements (%u).",
                                     (int)value_len, value, pos, ((struct lysc_node_leaflist*)node_s)->max);
                            goto error;
                        }
                    }
                }

                if (options & LY_TYPE_OPTS_STORE) {
                    /* update target path structure by adding predicate info */
                    pred->type = 0;
                    pred->position = pos;
                }
            }
        } else {
            asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value at character %lu (%.*s).",
                     (int)value_len, value, token - value + 1, (int)(value_len - (token - value)), token);
            goto error;
        }
    }

    if (options & LY_TYPE_OPTS_STORE) {
        storage->target = target;
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

    lyd_value_free_path(ctx, target);

    if (!*err) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
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
    unsigned int u, v;

    if (val1 == val2) {
        return LY_SUCCESS;
    } else if (!val1->target || !val2->target || LY_ARRAY_SIZE(val1->target) != LY_ARRAY_SIZE(val2->target)) {
        return LY_ENOT;
    }

    LY_ARRAY_FOR(val1->target, u) {
        struct lyd_value_path *s1 = &val1->target[u];
        struct lyd_value_path *s2 = &val2->target[u];

        if (s1->node != s2->node || (s1->predicates && !s2->predicates) || (!s1->predicates && s2->predicates) ||
                (s1->predicates && LY_ARRAY_SIZE(s1->predicates) != LY_ARRAY_SIZE(s2->predicates))) {
            return LY_ENOT;
        }
        if (s1->predicates) {
            LY_ARRAY_FOR(s1->predicates, v) {
                struct lyd_value_path_predicate *pred1 = &s1->predicates[v];
                struct lyd_value_path_predicate *pred2 = &s2->predicates[v];

                if (pred1->type != pred2->type) {
                    return LY_ENOT;
                }
                if (pred1->type == 0) {
                    /* position predicate */
                    if (pred1->position != pred2->position) {
                        return LY_ENOT;
                    }
                } else {
                    /* key-predicate or leaf-list-predicate */
                    if (pred1->key != pred2->key || ((struct lysc_node_leaf*)pred1->key)->type->plugin->compare(pred1->value, pred2->value)) {
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
ly_type_print_instanceid(const struct lyd_value *value, LYD_FORMAT format, ly_clb_get_prefix get_prefix, void *printer, int *dynamic)
{
    unsigned int u, v;
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
                struct lyd_value_path_predicate *pred = &value->target[u].predicates[v];
                if (pred->type == 0) {
                    /* position predicate */
                    ly_strcat(&result, "[%"PRIu64"]", pred->position);
                } else if (pred->type == 1) {
                    /* key-predicate */
                    int d = 0;
                    const char *value = pred->value->realtype->plugin->print(pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s:%s=%c%s%c]", get_prefix(pred->key->module, printer), pred->key->name, quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                } else if (pred->type == 2) {
                    /* leaf-list-predicate */
                    int d = 0;
                    const char *value = pred->value->realtype->plugin->print(pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                }
            }
        }
    } else { /* LYD_JSON */
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
                struct lyd_value_path_predicate *pred = &value->target[u].predicates[v];
                if (pred->type == 0) {
                    /* position predicate */
                    ly_strcat(&result, "[%"PRIu64"]", pred->position);
                } else if (pred->type == 1) {
                    /* key-predicate */
                    int d = 0;
                    const char *value = pred->value->realtype->plugin->print(pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s=%c%s%c]", pred->key->name, quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                } else if (pred->type == 2) {
                    /* leaf-list-predicate */
                    int d = 0;
                    const char *value = pred->value->realtype->plugin->print(pred->value, format, get_prefix, printer, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                }
            }
        }
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
    unsigned int u, v;

    if (!original->target) {
        return LY_SUCCESS;
    }

    LY_ARRAY_CREATE_RET(ctx, dup->target, LY_ARRAY_SIZE(original->target), LY_EMEM);
    LY_ARRAY_FOR(original->target, u) {
        LY_ARRAY_INCREMENT(dup->target);
        dup->target[u].node = original->target[u].node;
        if (original->target[u].predicates) {
            LY_ARRAY_CREATE_RET(ctx, dup->target[u].predicates, LY_ARRAY_SIZE(original->target[u].predicates), LY_EMEM);
            LY_ARRAY_FOR(original->target[u].predicates, v) {
                struct lyd_value_path_predicate *pred = &original->target[u].predicates[v];

                LY_ARRAY_INCREMENT(dup->target[u].predicates);
                dup->target[u].predicates[v].type = pred->type;
                if (pred->type) {
                    /* key-predicate or leaf-list-predicate */
                    dup->target[u].predicates[v].key = pred->key;
                    dup->target[u].predicates[v].value = calloc(1, sizeof *pred->value);
                    LY_CHECK_ERR_RET(!dup->target[u].predicates[v].value, LOGMEM(ctx), LY_EMEM);
                    dup->target[u].predicates[v].value->realtype = pred->value->realtype;
                    pred->value->realtype->plugin->duplicate(ctx, pred->value, dup->target[u].predicates[v].value);
                } else {
                    /* position-predicate */
                    dup->target[u].predicates[v].position = pred->position;
                }
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Free value of the YANG built-in instance-identifier types.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_instanceid(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lyd_value_free_path(ctx, value->target);
    value->target = NULL;
    ly_type_free_original(ctx, value);
}

/**
 * @brief Find leafref target in instance data.
 */
const struct lyd_node *
ly_type_find_leafref(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len,
                     const struct lyd_node *context_node, const struct lyd_node *tree, struct lyd_value *storage, char **errmsg)
{
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref*)type;
    const char *first_pred = NULL;
    const struct lyd_node *start_search;
    const char *prefix, *id;
    size_t prefix_len, id_len;
    const struct lys_module *mod_node = NULL;
    const char *token = type_lr->path;
    const struct lyd_node *node;
    struct lys_module *mod_context = context_node->schema->module;

    if (token[0] == '/') {
        /* absolute-path */
        node = NULL;
    } else {
        /*relative-path */
        node = context_node;
    }

    /* resolve leafref path */
    while (*token) {
        if (!strncmp(token, "../", 3)) {
            /* level up */
            token += 2;
            node = (struct lyd_node*)node->parent;
        } else if (!strncmp(token, "/../", 4)) {
            /* level up */
            token += 3;
            node = (struct lyd_node*)node->parent;
        } else if (*token == '/') {
            /* level down */

            /* reset predicates */
            first_pred = NULL;

            token++;
            ly_parse_nodeid(&token, &prefix, &prefix_len, &id, &id_len);
            mod_node = lys_module_find_prefix(mod_context, prefix, prefix_len);

            if (node) {
                /* inner node */
                start_search = lyd_node_children(node);
next_instance_inner:
                if (start_search) {
                    lyd_find_sibling_next(start_search, mod_node, id, id_len, NULL, 0, (struct lyd_node **)&node);
                } else {
                    node = NULL;
                }
            } else {
                /* top-level node */
                start_search = tree;
next_instance_toplevel:
                lyd_find_sibling_next(start_search, mod_node, id, id_len, NULL, 0, (struct lyd_node **)&node);
            }
            if (!node) {
                /* node not found */
                const char *pathstr_end = token;
                if (first_pred) {
                    /* some node instances actually exist, but they do not fit the predicate restrictions,
                     * here we want to find the end of the list's predicate(s) */
                    for (pathstr_end = ly_type_path_predicate_end(token);
                            pathstr_end[1] == '[';
                            pathstr_end = ly_type_path_predicate_end(pathstr_end + 1));
                    /* move after last ']' (after each predicate follows "/something" since path must points to
                     * a leaf/leaflist and predicates are allowed only for lists) */
                    pathstr_end++;
                }
                asprintf(errmsg, "Invalid leafref - required instance \"%.*s\" does not exists in the data tree(s).",
                         (int)(pathstr_end - type_lr->path), type_lr->path);
                return NULL;
            }
        } else if (*token == '[') {
            /* predicate */
            const char *pred_start = token;
            const struct lyd_node_term *key;
            const struct lyd_node *value;
            const struct lys_module *mod_pred;
            const char *pred_end = ly_type_path_predicate_end(token);
            const char *src_prefix, *src;
            size_t src_prefix_len, src_len;

            /* remember start of the first predicate to be able to return back when comparison fails
             * on a subsequent predicate in case of multiple predicates - on the next node instance
             * we have to start again with the first predicate */
            if (!first_pred) {
                first_pred = pred_start;
            }

            /* move after "[ *WSP" */
            token++;
            for (; isspace(*token); token++);

            /* parse node-identifier */
            ly_parse_nodeid(&token, &src_prefix, &src_prefix_len, &src, &src_len);
            mod_pred = lys_module_find_prefix(mod_context, src_prefix, src_prefix_len);

            lyd_find_sibling_next(lyd_node_children(node), mod_pred, src, src_len, NULL, 0, (struct lyd_node **)&key);
            if (!key) {
                asprintf(errmsg, "Internal error - missing expected list's key \"%.*s\" in module \"%s\" (%s:%d).",
                         (int)src_len, src, mod_pred->name, __FILE__, __LINE__);
                LOGINT(ctx);
                return NULL;
            }

            /* move after "*WSP = *WSP" */
            for (; *token != '='; token++);
            for (token++; isspace(*token); token++);
            /* move after "current() *WSP / *WSP 1*(.. *WSP / *WSP)" */
            token = strchr(token, ')') + 1;
            for (; *token != '/'; token++);
            for (; *token != '.'; token++);
            for (value = (const struct lyd_node*)context_node; *token == '.'; ) {
                value = (struct lyd_node*)value->parent; /* level up by .. */
                for (token += 2; *token != '/'; token++);
                for (token++; isspace(*token); token++);
            }

            /* parse "*(node-identifier *WSP / *WSP) node-identifier */
            do {
                /* parse node-identifier */
                ly_parse_nodeid(&token, &src_prefix, &src_prefix_len, &src, &src_len);
                mod_pred = lys_module_find_prefix(mod_context, src_prefix, src_prefix_len);

                if (!value) {
                    /* top-level search */
                    lyd_find_sibling_next(tree, mod_pred, src, src_len, NULL, 0, (struct lyd_node **)&value);
                } else {
                    /* inner node */
                    lyd_find_sibling_next(lyd_node_children(value), mod_pred, src, src_len, NULL, 0, (struct lyd_node **)&value);
                }
                if (!value) {
                    /* node not found - try another instance */
                    goto next_instance;
                }

                for (; isspace(*token); token++);
                if (*token == '/') {
                    /* - move after it and consume whitespaces */
                    for (token++; isspace(*token); token++);
                }
            } while (*token != ']');

            /* compare key and the value */
            if (key->value.realtype->plugin->compare(&key->value, &((struct lyd_node_term*)value)->value)) {
                /* nodes does not match, try another instance */
next_instance:
                token = first_pred;
                start_search = node->next;
                if (node->parent) {
                    goto next_instance_inner;
                } else {
                    goto next_instance_toplevel;
                }
            }
            /* match */

            /* move after predicate */
            assert(token == pred_end);
            token = pred_end + 1;
        }
    }

    /* check value */
    while (node && type_lr->realtype->plugin->compare(&((struct lyd_node_term*)node)->value, storage)) {
        /* values do not match, try another instance of the node */
        const struct lysc_node *schema = node->schema;
        LY_LIST_FOR(node->next, node) {
            if (node->schema == schema) {
                break;
            }
        }
    }
    if (!node) {
        /* node not found */
        asprintf(errmsg, "Invalid leafref value \"%.*s\" - required instance \"%.*s\" with this value does not exists"
                 " in the data tree(s).", (int)value_len, value, (int)(token - type_lr->path), type_lr->path);
        return NULL;
    }

    return node;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in leafref type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_leafref(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                      ly_clb_resolve_prefix resolve_prefix, void *parser, LYD_FORMAT format,
                      const void *context_node, const struct lyd_node *tree,
                      struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret;
    char *errmsg = NULL;
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref*)type;
    int storage_dummy = 0;

    if (!(options & (LY_TYPE_OPTS_STORE | LY_TYPE_OPTS_INCOMPLETE_DATA)) && type_lr->require_instance) {
        /* if there is no storage, but we will check the instance presence in data tree(s),
         * we need some (dummy) storage for data comparison */
        storage = calloc(1, sizeof *storage);
        storage_dummy = 1;
    }
    /* rewrite leafref plugin stored in the storage by default */
    storage->realtype = type_lr->realtype;

    /* check value according to the real type of the leafref target */
    ret = type_lr->realtype->plugin->store(ctx, type_lr->realtype, value, value_len, options,
                                           resolve_prefix, parser, format, context_node, tree,
                                           storage, canonized, err);
    if (ret != LY_SUCCESS && ret != LY_EINCOMPLETE) {
        return ret;
    }

    if (!(options & LY_TYPE_OPTS_SCHEMA) && type_lr->require_instance) {
        if (options & LY_TYPE_OPTS_INCOMPLETE_DATA) {
            return LY_EINCOMPLETE;
        }

        /* find corresponding data instance */
        if (!ly_type_find_leafref(ctx, type, value, value_len, (const struct lyd_node *)context_node, tree, storage, &errmsg)) {
            goto error;
        }
    }

    if (storage_dummy) {
        storage->realtype->plugin->free(ctx, storage);
        free(storage);
    }
    return ret;

error:
    if (!*err) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
    }
    if (storage_dummy) {
        storage->realtype->plugin->free(ctx, storage);
        free(storage);
    }
    return LY_EVALID;

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
ly_type_print_leafref(const struct lyd_value *value, LYD_FORMAT format, ly_clb_get_prefix get_prefix, void *printer, int *dynamic)
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
    value->realtype->plugin->free(ctx, value);
}

/**
 * @brief Validate, canonize and store value of the YANG built-in union type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_union(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                    ly_clb_resolve_prefix resolve_prefix, void *parser, LYD_FORMAT format,
                    const void *context_node, const struct lyd_node *tree,
                    struct lyd_value *storage, const char **canonized, struct ly_err_item **err)
{
    LY_ERR ret;
    unsigned int u;
    struct lysc_type_union *type_u = (struct lysc_type_union*)type;
    struct lyd_value_subvalue *subvalue;
    char *errmsg = NULL;

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

search_subtype:
        /* use the first usable sybtype to store the value */
        LY_ARRAY_FOR(type_u->types, u) {
            subvalue->value->realtype = type_u->types[u];
            ret = type_u->types[u]->plugin->store(ctx, type_u->types[u], value, value_len, options & ~LY_TYPE_OPTS_DYNAMIC,
                                                  ly_type_stored_prefixes_clb, subvalue->prefixes, format,
                                                  context_node, tree, subvalue->value, canonized, err);
            if (ret == LY_SUCCESS || ret == LY_EINCOMPLETE) {
                /* success (or not yet complete) */
                break;
            }
            ly_err_free(*err);
            *err = NULL;
        }

        if (u == LY_ARRAY_SIZE(type_u->types)) {
            asprintf(&errmsg, "Invalid union value \"%.*s\" - no matching subtype found.", (int)value_len, value);
            goto error;
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

error:

    if (!*err) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
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

    return LY_EVALID;
}

/**
 * @brief Comparison callback checking the union value.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_union(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }
    return val1->realtype->plugin->compare(val1->subvalue->value, val2->subvalue->value);
}

/**
 * @brief Printer callback printing the union value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_union(const struct lyd_value *value, LYD_FORMAT format, ly_clb_get_prefix get_prefix, void *printer, int *dynamic)
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
    unsigned int u;

    dup->subvalue = calloc(1, sizeof *dup->subvalue);
    LY_CHECK_ERR_RET(!dup->subvalue, LOGMEM(ctx), LY_EMEM);
    if (original->subvalue->prefixes) {
        LY_ARRAY_CREATE_RET(ctx, dup->subvalue->prefixes, LY_ARRAY_SIZE(original->subvalue->prefixes), LY_EMEM);
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
    unsigned int u;

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
