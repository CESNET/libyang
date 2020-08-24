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
#include "dict.h"
#include "path.h"
#include "set.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

/**
 * @brief Find import prefix in imports.
 */
static const struct lys_module *
ly_schema_resolve_prefix(const struct ly_ctx *UNUSED(ctx), const char *prefix, size_t prefix_len, void *data)
{
    return lys_module_find_prefix((const struct lys_module *)data, prefix, prefix_len);
}

/**
 * @brief Find XML namespace prefix in XML namespaces, which are then mapped to modules.
 */
static const struct lys_module *
ly_xml_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *data)
{
    const struct lyxml_ns *ns;
    const struct ly_set *ns_set = data;

    ns = lyxml_ns_get(ns_set, prefix, prefix_len);
    if (!ns) {
        return NULL;
    }

    return ly_ctx_get_module_implemented_ns(ctx, ns->uri);
}

/**
 * @brief Find module name.
 */
static const struct lys_module *
ly_json_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *UNUSED(parser))
{
    return ly_ctx_get_module_implemented2(ctx, prefix, prefix_len);
}

const struct lys_module *
ly_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, LY_PREFIX_FORMAT format, void *prefix_data)
{
    const struct lys_module *mod;

    switch (format) {
    case LY_PREF_SCHEMA:
        mod = ly_schema_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    case LY_PREF_XML:
        mod = ly_xml_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    case LY_PREF_JSON:
        mod = ly_json_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    }

    return mod;
}

/**
 * @brief Find module in import prefixes.
 */
static const char *
ly_schema_get_prefix(const struct lys_module *mod, void *data)
{
    const struct lys_module *context_mod = data;
    LY_ARRAY_COUNT_TYPE u;

    if (context_mod == mod) {
        return context_mod->prefix;
    }
    LY_ARRAY_FOR(context_mod->parsed->imports, u) {
        if (context_mod->parsed->imports[u].module == mod) {
            /* match */
            return context_mod->parsed->imports[u].prefix;
        }
    }

    return NULL;
}

/**
 * @brief Simply return module local prefix. Also, store the module in a set.
 */
static const char *
ly_xml_get_prefix(const struct lys_module *mod, void *data)
{
    struct ly_set *ns_list = data;

    ly_set_add(ns_list, (void *)mod, 0);
    return mod->prefix;
}

/**
 * @brief Simply return module name.
 */
static const char *
ly_json_get_prefix(const struct lys_module *mod, void *UNUSED(data))
{
    return mod->name;
}

const char *
ly_get_prefix(const struct lys_module *mod, LY_PREFIX_FORMAT format, void *prefix_data)
{
    const char *prefix = NULL;

    switch (format) {
    case LY_PREF_SCHEMA:
        prefix = ly_schema_get_prefix(mod, prefix_data);
        break;
    case LY_PREF_XML:
        prefix = ly_xml_get_prefix(mod, prefix_data);
        break;
    case LY_PREF_JSON:
        prefix = ly_json_get_prefix(mod, prefix_data);
        break;
    }

    return prefix;
}

/**
 * @brief Generic comparison callback checking the canonical value.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_simple(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if ((val1->realtype == val2->realtype) && (val1->canonical == val2->canonical)) {
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
ly_type_print_simple(const struct lyd_value *value, LY_PREFIX_FORMAT UNUSED(format),
        void *UNUSED(prefix_data), int *dynamic)
{
    *dynamic = 0;
    return (char *)value->canonical;
}

/**
 * @brief Generic duplication callback of the canonized and original values only.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_simple(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    dup->canonical = lydict_insert(ctx, original->canonical, strlen(original->canonical));
    dup->ptr = original->ptr;
    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

/**
 * @brief Free canonized value in lyd_value.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_simple(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lydict_remove(ctx, value->canonical);
    value->canonical = NULL;
}

API LY_ERR
ly_type_parse_int(const char *datatype, int base, int64_t min, int64_t max, const char *value, size_t value_len,
        int64_t *ret, struct ly_err_item **err)
{
    char *errmsg = NULL;
    int rc = 0;

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    /* consume leading whitespaces */
    for (; value_len && isspace(*value); ++value, --value_len) {}

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

API LY_ERR
ly_type_parse_uint(const char *datatype, int base, uint64_t max, const char *value, size_t value_len, uint64_t *ret,
        struct ly_err_item **err)
{
    char *errmsg = NULL;
    int rc = 0;

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    /* consume leading whitespaces */
    for (; value_len && isspace(*value); ++value, --value_len) {}

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
    for (; value_len && isspace(*value); ++value, --value_len) {}

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
        for (u = len + trailing_zeros; u < value_len && isspace(value[u]); ++u) {}
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

/**
 * @brief Validate, canonize and store value of the YANG built-in signed integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_int(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret;
    int64_t num;
    char *str;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    switch (type->basetype) {
    case LY_TYPE_INT8:
        LY_CHECK_RET(ly_type_parse_int("int8", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, INT64_C(-128), INT64_C(127), value, value_len, &num, err));
        break;
    case LY_TYPE_INT16:
        LY_CHECK_RET(ly_type_parse_int("int16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, INT64_C(-32768), INT64_C(32767), value, value_len, &num, err));
        break;
    case LY_TYPE_INT32:
        LY_CHECK_RET(ly_type_parse_int("int32", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10,
                                       INT64_C(-2147483648), INT64_C(2147483647), value, value_len, &num, err));
        break;
    case LY_TYPE_INT64:
        LY_CHECK_RET(ly_type_parse_int("int64", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10,
                                       INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807), value, value_len, &num, err));
        break;
    default:
        LOGINT_RET(ctx);
    }

    LY_CHECK_RET(asprintf(&str, "%" PRId64, num) == -1, LY_EMEM);

    /* range of the number */
    if (type_num->range) {
        LY_CHECK_ERR_RET(ret = ly_type_validate_range(type->basetype, type_num->range, num, str, err), free(str), ret);
    }

    /* store everything */
    storage->canonical = lydict_insert_zc(ctx, str);
    storage->int64 = num;
    storage->realtype = type;

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
    }
    return LY_SUCCESS;
}

/**
 * @brief Validate and canonize value of the YANG built-in unsigned integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_uint(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret;
    uint64_t num;
    struct lysc_type_num* type_num = (struct lysc_type_num*)type;
    char *str;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    switch (type->basetype) {
    case LY_TYPE_UINT8:
        LY_CHECK_RET(ly_type_parse_uint("uint8", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(255), value, value_len, &num, err));
        break;
    case LY_TYPE_UINT16:
        LY_CHECK_RET(ly_type_parse_uint("uint16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(65535), value, value_len, &num, err));
        break;
    case LY_TYPE_UINT32:
        LY_CHECK_RET(ly_type_parse_uint("uint32", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(4294967295), value, value_len, &num, err));
        break;
    case LY_TYPE_UINT64:
        LY_CHECK_RET(ly_type_parse_uint("uint64", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, UINT64_C(18446744073709551615), value, value_len, &num, err));
        break;
    default:
        LOGINT_RET(ctx);
    }

    LY_CHECK_RET(asprintf(&str, "%" PRIu64, num) == -1, LY_EMEM);

    /* range of the number */
    if (type_num->range) {
        LY_CHECK_ERR_RET(ret = ly_type_validate_range(type->basetype, type_num->range, num, str, err), free(str), ret);
    }

    /* store everything */
    storage->canonical = lydict_insert_zc(ctx, str);
    storage->int64 = num;
    storage->realtype = type;

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
    }
    return LY_SUCCESS;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in decimal64 types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_decimal64(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
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
        int count = sprintf(buf, "%" PRId64 " ", d);
        if ((d > 0 && (count - 1) <= type_dec->fraction_digits)
                || (count - 2) <= type_dec->fraction_digits ) {
            /* we have 0. value, print the value with the leading zeros
             * (one for 0. and also keep the correct with of num according
             * to fraction-digits value)
             * for (num<0) - extra character for '-' sign */
            count = sprintf(buf, "%0*" PRId64 " ", (d > 0) ? (type_dec->fraction_digits + 1) : (type_dec->fraction_digits + 2), d);
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

    storage->canonical = lydict_insert(ctx, buf, strlen(buf));
    storage->dec64 = d;
    storage->realtype = type;

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
    }
    return LY_SUCCESS;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in binary type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_binary(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
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
        for (start = 0; (start < value_len) && isspace(value[start]); start++) {}
        for (stop = value_len - 1; stop > start && isspace(value[stop]); stop--) {}
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
                    } else if (u == stop) {
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
        snprintf(buf, 22, "%" PRIu64, len);
        LY_CHECK_RET(ly_type_validate_range(LY_TYPE_BINARY, type_bin->length, len, buf, err));
    }

    if (start != 0 || (value_len && stop != value_len - 1)) {
        storage->canonical = lydict_insert(ctx, &value[start], stop + 1 - start);
    } else {
        storage->canonical = lydict_insert(ctx, value_len ? value : "", value_len);
    }
    storage->ptr = NULL;
    storage->realtype = type;

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
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
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
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

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        storage->canonical = lydict_insert_zc(ctx, (char *)value);
    } else {
        storage->canonical = lydict_insert(ctx, value_len ? value : "", value_len);
    }
    storage->ptr = NULL;
    storage->realtype = type;

    return LY_SUCCESS;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_bits(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
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
        for (item_len = 0; index + item_len < value_len && !isspace(item[item_len]); item_len++) {}
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

                if (iscanonical && items->count && type_bits->bits[u].position < ((struct lysc_type_bitenum_item*)items->objs[items->count - 1])->position) {
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

    if (iscanonical) {
        /* items are already ordered */
        items_ordered = items;
        items = NULL;

        if (!ws_count && !lws_count && (options & LY_TYPE_OPTS_DYNAMIC)) {
            can = lydict_insert_zc(ctx, (char *)value);
            value = NULL;
            options &= ~LY_TYPE_OPTS_DYNAMIC;
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

    /* store all data */
    storage->canonical = can;
    can = NULL;
    LY_ARRAY_CREATE_GOTO(ctx, storage->bits_items, items_ordered->count, ret, error);
    for (uint32_t x = 0; x < items_ordered->count; x++) {
        storage->bits_items[x] = items_ordered->objs[x];
        LY_ARRAY_INCREMENT(storage->bits_items);
    }
    ly_set_free(items_ordered, NULL);
    storage->realtype = type;

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
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

    dup->canonical = lydict_insert(ctx, original->canonical, strlen(original->canonical));
    dup->realtype = original->realtype;
    return LY_SUCCESS;
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

    lydict_remove(ctx, value->canonical);
    value->canonical = NULL;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in enumeration type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_enum(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
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

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        storage->canonical = lydict_insert_zc(ctx, (char *)value);
    } else {
        storage->canonical = lydict_insert(ctx, value_len ? value : "", value_len);
    }
    storage->enum_item = &type_enum->enums[u];
    storage->realtype = type;

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

/**
 * @brief Validate and store value of the YANG built-in boolean type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_boolean(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len,
        int options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data),
        const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
        struct lyd_value *storage, struct ly_err_item **err)
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

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        storage->canonical = lydict_insert_zc(ctx, (char*)value);
    } else {
        storage->canonical = lydict_insert(ctx, value, value_len);
    }
    storage->boolean = i;
    storage->realtype = type;

    return LY_SUCCESS;
}

/**
 * @brief Validate and store value of the YANG built-in empty type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_empty(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data),
        const void *UNUSED(context_node), const struct lyd_node *UNUSED(tree),
        struct lyd_value *storage, struct ly_err_item **err)
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

    storage->canonical = lydict_insert(ctx, "", 0);
    storage->ptr = NULL;
    storage->realtype = type;

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

static const char *ly_type_print_identityref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        int *dynamic);

/**
 * @brief Validate, canonize and store value of the YANG built-in identiytref type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_identityref(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT format, void *prefix_data, const void *UNUSED(context_node),
        const struct lyd_node *UNUSED(tree),struct lyd_value *storage, struct ly_err_item **err)
{
    struct lysc_type_identityref *type_ident = (struct lysc_type_identityref *)type;
    const char *id_name, *prefix = value;
    size_t id_len, prefix_len;
    char *errmsg = NULL, *str;
    const struct lys_module *mod;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_ident *ident = NULL, *identities;
    int erc = 0, dyn;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        return LY_SUCCESS;
    }

    /* locate prefix if any */
    for (prefix_len = 0; prefix_len < value_len && value[prefix_len] != ':'; ++prefix_len) {}
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

    mod = ly_resolve_prefix(ctx, prefix, prefix_len, format, prefix_data);
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
        erc = asprintf(&errmsg, "Invalid identityref \"%.*s\" value - identity not accepted by the type specification.",
                        (int)value_len, value);
        goto error;
    }

    storage->ident = ident;

    /* get JSON form since there is no canonical */
    str = (char *)ly_type_print_identityref(storage, LY_PREF_JSON, NULL, &dyn);
    assert(str && dyn);
    storage->canonical = lydict_insert_zc(ctx, str);
    storage->realtype = type;

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
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
ly_type_print_identityref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, int *dynamic)
{
    char *result = NULL;

    *dynamic = 1;
    if (asprintf(&result, "%s:%s", ly_get_prefix(value->ident->module, format, prefix_data), value->ident->name) == -1) {
        return NULL;
    } else {
        return result;
    }
}

static const char *ly_type_print_instanceid(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        int *dynamic);

/**
 * @brief Validate and store value of the YANG built-in instance-identifier type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_instanceid(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT format, void *prefix_data, const void *context_node, const struct lyd_node *tree,
        struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret = LY_EVALID;
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)type;
    char *errmsg = NULL, *str;
    struct ly_path *path = NULL;
    struct ly_set predicates = {0};
    struct lyxp_expr *exp = NULL;
    const struct lysc_node *ctx_scnode;
    int erc = 0, prefix_opt = 0, dyn;

    /* init */
    *err = NULL;
    ctx_scnode = (options & (LY_TYPE_OPTS_SCHEMA | LY_TYPE_OPTS_INCOMPLETE_DATA)) ?
            (struct lysc_node *)context_node : ((struct lyd_node *)context_node)->schema;

    if ((options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_INCOMPLETE_DATA)) {
        /* we have incomplete schema tree */
        /* HACK temporary storing of the original value */
        storage->canonical = lydict_insert(ctx, value_len ? value : "", value_len);
        goto cleanup;
    }

    switch (format) {
    case LY_PREF_SCHEMA:
    case LY_PREF_XML:
        prefix_opt = LY_PATH_PREFIX_MANDATORY;
        break;
    case LY_PREF_JSON:
        prefix_opt = LY_PATH_PREFIX_STRICT_INHERIT;
        break;
    }

    if (!(options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_SECOND_CALL)) {
        /* the second run in data tree, the first one ended with LY_EINCOMPLETE, but we have prepared the target structure */
        if (ly_path_eval(storage->target, tree, NULL)) {
            /* in error message we print the JSON format of the instance-identifier - in case of XML, it is not possible
             * to get the exactly same string as original, JSON is less demanding and still well readable/understandable. */
            int dynamic = 0;
            const char *id = storage->realtype->plugin->print(storage, LY_PREF_JSON, NULL, &dynamic);
            erc = asprintf(&errmsg, "Invalid instance-identifier \"%s\" value - required instance not found.", id);
            if (dynamic) {
                free((char *)id);
            }
            /* we have to clean up the storage */
            type->plugin->free(ctx, storage);

            goto error;
        }
        return LY_SUCCESS;
    }

    /* parse the value */
    ret = ly_path_parse(ctx, ctx_scnode, value, value_len, LY_PATH_BEGIN_ABSOLUTE, LY_PATH_LREF_FALSE,
                        prefix_opt, LY_PATH_PRED_SIMPLE, &exp);
    if (ret) {
        erc = asprintf(&errmsg, "Invalid instance-identifier \"%.*s\" value - syntax error.", (int)value_len, value);
        goto error;
    }

    /* resolve it on schema tree */
    ret = ly_path_compile(ctx, ctx_scnode->module, NULL, exp, LY_PATH_LREF_FALSE, lysc_is_output(ctx_scnode) ?
                          LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_SINGLE, format, prefix_data, &path);
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

    /* HACK remove previously stored original value */
    lydict_remove(ctx, storage->canonical);

    /* store resolved schema path */
    storage->target = path;
    path = NULL;

    /* store JSON string value */
    str = (char *)ly_type_print_instanceid(storage, LY_PREF_JSON, NULL, &dyn);
    assert(str && dyn);
    storage->canonical = lydict_insert_zc(ctx, str);

    storage->realtype = type;

cleanup:
    ly_set_erase(&predicates, NULL);
    lyxp_expr_free(ctx, exp);
    ly_path_free(ctx, path);

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
    }

    if ((options & LY_TYPE_OPTS_INCOMPLETE_DATA) && ((options & LY_TYPE_OPTS_SCHEMA) || type_inst->require_instance)) {
        return LY_EINCOMPLETE;
    } else {
        return LY_SUCCESS;
    }

error:
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
ly_type_print_instanceid(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, int *dynamic)
{
    LY_ARRAY_COUNT_TYPE u, v;
    char *result = NULL;

    if (!value->target) {
        /* value was not fully processed */
        return NULL;
    }

    if ((format == LY_PREF_XML) || (format == LY_PREF_SCHEMA)) {
        /* everything is prefixed */
        LY_ARRAY_FOR(value->target, u) {
            ly_strcat(&result, "/%s:%s", ly_get_prefix(value->target[u].node->module, format, prefix_data),
                      value->target[u].node->name);
            LY_ARRAY_FOR(value->target[u].predicates, v) {
                struct ly_path_predicate *pred = &value->target[u].predicates[v];
                switch (value->target[u].pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    ly_strcat(&result, "[%" PRIu64 "]", pred->position);
                    break;
                case LY_PATH_PREDTYPE_LIST:
                {
                    /* key-predicate */
                    int d = 0;
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(value, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s:%s=%c%s%c]", ly_get_prefix(pred->key->module, format, prefix_data),
                              pred->key->name, quot, value, quot);
                    if (d) {
                        free((char*)value);
                    }
                    break;
                }
                case LY_PATH_PREDTYPE_LEAFLIST:
                {
                    /* leaf-list-predicate */
                    int d = 0;
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
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
    } else if (format == LY_PREF_JSON) {
        /* only the first node or the node changing module is prefixed */
        struct lys_module *mod = NULL;
        LY_ARRAY_FOR(value->target, u) {
            if (mod != value->target[u].node->module) {
                mod = value->target[u].node->module;
                ly_strcat(&result, "/%s:%s", ly_get_prefix(mod, format, prefix_data), value->target[u].node->name);
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
                    ly_strcat(&result, "[%" PRIu64 "]", pred->position);
                    break;
                case LY_PATH_PREDTYPE_LIST:
                {
                    /* key-predicate */
                    int d = 0;
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
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
                    const char *value = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
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
    dup->canonical = lydict_insert(ctx, original->canonical, strlen(original->canonical));
    dup->realtype = original->realtype;
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
    ly_type_free_simple(ctx, value);
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
    ret = lyxp_eval(lref->path, LY_PREF_SCHEMA, lref->path_context, node, LYXP_NODE_ELEM, tree, &set, 0);
    if (ret) {
        ret = LY_ENOTFOUND;
        val_str = lref->plugin->print(value, LY_PREF_JSON, NULL, &dynamic);
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
        val_str = lref->plugin->print(value, LY_PREF_JSON, NULL, &dynamic);
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
        LY_PREFIX_FORMAT format, void *prefix_data, const void *context_node, const struct lyd_node *tree,
        struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    char *errmsg = NULL;
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref *)type;
    struct ly_path *p = NULL;
    struct ly_set *set = NULL;

    if (!type_lr->realtype) {
        if ((options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_INCOMPLETE_DATA)) {
            /* leafref's path was not yet resolved - in schema trees, path can be resolved when
             * the complete schema tree is present, in such a case we need to wait with validating
             * default values */
            return LY_EINCOMPLETE;
        } else {
            LOGINT(ctx);
            return LY_EINT;
        }
    }

    if ((options & LY_TYPE_OPTS_SCHEMA) && (options & LY_TYPE_OPTS_SECOND_CALL)) {
        /* hide the LY_TYPE_OPTS_SECOND_CALL option from the target's store callback, the option is connected
         * only with the leafref's path, so it is not supposed to be used here. If the previous LY_EINCOMPLETE would
         * be caused by the target's type, the target type's callback would be used directly, not via leafref's callback */
        options &= ~LY_TYPE_OPTS_SECOND_CALL;
    }

    /* check value according to the real type of the leafref target */
    ret = type_lr->realtype->plugin->store(ctx, type_lr->realtype, value, value_len, options, format, prefix_data,
                                           context_node, tree, storage, err);
    if (ret != LY_SUCCESS && ret != LY_EINCOMPLETE) {
        goto cleanup;
    }

    /* rewrite leafref plugin stored in the storage by default */
    storage->realtype = type_lr->realtype;

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
ly_type_print_leafref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, int *dynamic)
{
    return value->realtype->plugin->print(value, format, prefix_data, dynamic);
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
    }
}

/**
 * @brief Answer if the type is suitable for the parser's hit (if any) in the specified format
 */
LY_ERR
type_check_parser_hint(LY_PREFIX_FORMAT format, int hint, LY_DATA_TYPE type)
{
    if (format == LY_PREF_JSON && hint) {
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
 * @brief Free stored prefix data of a union.
 *
 * @param[in] format Format of the prefixes.
 * @param[in] prefix_data Prefix data to free.
 */
static void
ly_type_union_free_prefix_data(LY_PREFIX_FORMAT format, void *prefix_data)
{
    struct ly_set *ns_list;
    uint32_t i;

    switch (format) {
    case LY_PREF_XML:
        ns_list = prefix_data;
        for (i = 0; i < ns_list->count; ++i) {
            free(((struct lyxml_ns *)ns_list->objs[i])->prefix);
            free(((struct lyxml_ns *)ns_list->objs[i])->uri);
        }
        ly_set_free(ns_list, free);
        break;
    case LY_PREF_SCHEMA:
    case LY_PREF_JSON:
        break;
    }
}

/**
 * @brief Store prefix data of a union.
 *
 * @param[in] ctx libyang context.
 * @param[in] value Value string to be parsed.
 * @param[in] value_len Length of the @p value string.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] prefix_data Parser's data for get prefix.
 * @return Created format-specific prefix data for prefix resolution callback.
 */
static void *
ly_type_union_store_prefix_data(const struct ly_ctx *ctx, const char *value, size_t value_len, LY_PREFIX_FORMAT format,
        void *prefix_data)
{
    unsigned int c;
    const char *start, *stop;
    const struct lys_module *mod;
    struct lyxml_ns *ns;
    struct ly_set *ns_list;

    switch (format) {
    case LY_PREF_SCHEMA:
        /* just remember the local module */
        return prefix_data;
    case LY_PREF_XML:
        /* copy only referenced namespaces from the ns_set */
        break;
    case LY_PREF_JSON:
        /* there should be no prefix data */
        assert(!prefix_data);
        return NULL;
    }

    /* XML only */
    ns_list = ly_set_new();
    LY_CHECK_ERR_RET(!ns_list, LOGMEM(ctx), NULL);

    /* add all used prefixes */
    for (stop = start = value; (size_t)(stop - value) < value_len; start = stop) {
        size_t bytes;
        ly_getutf8(&stop, &c, &bytes);
        if (is_xmlqnamestartchar(c)) {
            for (ly_getutf8(&stop, &c, &bytes);
                    is_xmlqnamechar(c) && (size_t)(stop - value) < value_len;
                    ly_getutf8(&stop, &c, &bytes)) {}
            stop = stop - bytes;
            if (*stop == ':') {
                /* we have a possible prefix */
                size_t len = stop - start;

                /* do we already have the prefix? */
                mod = ly_resolve_prefix(ctx, start, len, format, ns_list);
                if (!mod) {
                    /* is it even defined? */
                    mod = ly_resolve_prefix(ctx, start, len, format, prefix_data);
                    if (mod) {
                        /* store a new prefix - namespace pair */
                        ns = calloc(1, sizeof *ns);
                        LY_CHECK_ERR_GOTO(!ns, LOGMEM(ctx), error);
                        ly_set_add(ns_list, ns, LY_SET_OPT_USEASLIST);

                        ns->prefix = strndup(start, len);
                        LY_CHECK_ERR_GOTO(!ns->prefix, LOGMEM(ctx), error);
                        ns->uri = strdup(mod->ns);
                        LY_CHECK_ERR_GOTO(!ns->uri, LOGMEM(ctx), error);
                    }
                } /* else the prefix already present */
            }
            stop = stop + bytes;
        }
    }

    /* add default namespace */
    mod = ly_resolve_prefix(ctx, NULL, 0, format, prefix_data);
    if (mod) {
        ns = calloc(1, sizeof *ns);
        LY_CHECK_ERR_GOTO(!ns, LOGMEM(ctx), error);
        ly_set_add(ns_list, ns, LY_SET_OPT_USEASLIST);

        ns->uri = strdup(mod->ns);
        LY_CHECK_ERR_GOTO(!ns->uri, LOGMEM(ctx), error);
    }

    return ns_list;

error:
    ly_type_union_free_prefix_data(LY_PREF_XML, ns_list);
    return NULL;
}

/**
 * @brief Duplicate prefix data of a union.
 *
 * @param[in] ctx libyang context.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] prefix_data Prefix data to duplicate.
 * @return Duplicated prefix data.
 */
static void *
ly_type_union_dup_prefix_data(const struct ly_ctx *ctx, LY_PREFIX_FORMAT format, const void *prefix_data)
{
    struct lyxml_ns *ns;
    struct ly_set *ns_list, *orig;
    uint32_t i;

    switch (format) {
    case LY_PREF_SCHEMA:
        return (void *)prefix_data;
    case LY_PREF_XML:
        break;
    case LY_PREF_JSON:
        assert(!prefix_data);
        return NULL;
    }

    /* XML only */
    ns_list = ly_set_new();
    LY_CHECK_ERR_RET(!ns_list, LOGMEM(ctx), NULL);

    /* copy all the namespaces */
    orig = (struct ly_set *)prefix_data;
    for (i = 0; i < orig->count; ++i) {
        ns = calloc(1, sizeof *ns);
        LY_CHECK_ERR_GOTO(!ns, LOGMEM(ctx), error);
        ly_set_add(ns_list, ns, LY_SET_OPT_USEASLIST);

        if (((struct lyxml_ns *)orig->objs[i])->prefix) {
            ns->prefix = strdup(((struct lyxml_ns *)orig->objs[i])->prefix);
            LY_CHECK_ERR_GOTO(!ns->prefix, LOGMEM(ctx), error);
        }
        ns->uri = strdup(((struct lyxml_ns *)orig->objs[i])->uri);
        LY_CHECK_ERR_GOTO(!ns->uri, LOGMEM(ctx), error);
    }

    return ns_list;

error:
    ly_type_union_free_prefix_data(LY_PREF_XML, ns_list);
    return NULL;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in union type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_union(const struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
        LY_PREFIX_FORMAT format, void *prefix_data, const void *context_node, const struct lyd_node *tree,
        struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_type_union *type_u = (struct lysc_type_union *)type;
    struct lyd_value_subvalue *subvalue;
    char *errmsg = NULL;
    int prev_lo;

    if (options & LY_TYPE_OPTS_SECOND_CALL) {
        subvalue = storage->subvalue;

        /* call the callback second time */
        ret = subvalue->value->realtype->plugin->store(ctx, subvalue->value->realtype, subvalue->original,
                                                       strlen(subvalue->original), options & ~LY_TYPE_OPTS_DYNAMIC,
                                                       subvalue->format, subvalue->prefix_data, context_node, tree,
                                                       subvalue->value, err);
        if (ret == LY_SUCCESS) {
            /* storing successful */
            return LY_SUCCESS;
        }

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
    } else {
        /* prepare subvalue storage */
        subvalue = calloc(1, sizeof *subvalue);
        subvalue->value = calloc(1, sizeof *subvalue->value);

        /* remember the original value */
        subvalue->original = lydict_insert(ctx, value_len ? value : "", value_len);

        /* store format-specific data for later prefix resolution */
        subvalue->format = format;
        subvalue->prefix_data = ly_type_union_store_prefix_data(ctx, value, value_len, format, prefix_data);

        /* remember the hint options */
        subvalue->parser_hint = options & LY_TYPE_PARSER_HINTS_MASK;
    }

    /* use the first usable sybtype to store the value */
    LY_ARRAY_FOR(type_u->types, u) {
        if (type_check_parser_hint(format, subvalue->parser_hint, type_u->types[u]->basetype)) {
            /* not a suitable type */
            continue;
        }

        /* turn logging off */
        prev_lo = ly_log_options(0);
        ret = type_u->types[u]->plugin->store(ctx, type_u->types[u], subvalue->original, strlen(subvalue->original),
                                              options & ~LY_TYPE_OPTS_DYNAMIC, subvalue->format, subvalue->prefix_data,
                                              context_node, tree, subvalue->value, err);
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

        /* free any stored information */
        free(subvalue->value);
        lydict_remove(ctx, subvalue->original);
        ly_type_union_free_prefix_data(subvalue->format, subvalue->prefix_data);
        free(subvalue);
        if (options & LY_TYPE_OPTS_SECOND_CALL) {
            storage->subvalue = NULL;
        }
        return ret;
    }

    /* success */

    storage->canonical = lydict_insert(ctx, subvalue->value->canonical, strlen(subvalue->value->canonical));
    storage->subvalue = subvalue;
    storage->realtype = type;

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char *)value);
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
ly_type_print_union(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, int *dynamic)
{
    return value->subvalue->value->realtype->plugin->print(value->subvalue->value, format, prefix_data, dynamic);
}

/**
 * @brief Duplication callback of the union values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    dup->canonical = lydict_insert(ctx, original->canonical, strlen(original->canonical));

    dup->subvalue = calloc(1, sizeof *dup->subvalue);
    LY_CHECK_ERR_RET(!dup->subvalue, LOGMEM(ctx), LY_EMEM);
    dup->subvalue->value = calloc(1, sizeof *dup->subvalue->value);
    LY_CHECK_ERR_RET(!dup->subvalue->value, LOGMEM(ctx), LY_EMEM);
    original->subvalue->value->realtype->plugin->duplicate(ctx, original->subvalue->value, dup->subvalue->value);

    dup->subvalue->original = lydict_insert(ctx, original->subvalue->original, strlen(original->subvalue->original));
    dup->subvalue->format = original->subvalue->format;
    dup->subvalue->prefix_data = ly_type_union_dup_prefix_data(ctx, original->subvalue->format,
                                                               original->subvalue->prefix_data);

    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

/**
 * @brief Free value of the YANG built-in union type.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lydict_remove(ctx, value->canonical);
    if (value->subvalue) {
        if (value->subvalue->value) {
            value->subvalue->value->realtype->plugin->free(ctx, value->subvalue->value);
            free(value->subvalue->value);
            ly_type_union_free_prefix_data(value->subvalue->format, value->subvalue->prefix_data);
        }
        lydict_remove(ctx, value->subvalue->original);
        free(value->subvalue);
        value->subvalue = NULL;
    }
}

/**
 * @brief Set of type plugins for YANG built-in types
 */
struct lysc_type_plugin ly_builtin_type_plugins[LY_DATA_TYPE_COUNT] = {
    {0}, /* LY_TYPE_UNKNOWN */
    {.type = LY_TYPE_BINARY, .store = ly_type_store_binary, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - binary, version 1"},
    {.type = LY_TYPE_UINT8, .store = ly_type_store_uint, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT16, .store = ly_type_store_uint, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT32, .store = ly_type_store_uint, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT64, .store = ly_type_store_uint, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_STRING, .store = ly_type_store_string, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - string, version 1"},
    {.type = LY_TYPE_BITS, .store = ly_type_store_bits, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_bits, .free = ly_type_free_bits,
        .id = "libyang 2 - bits, version 1"},
    {.type = LY_TYPE_BOOL, .store = ly_type_store_boolean, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - boolean, version 1"},
    {.type = LY_TYPE_DEC64, .store = ly_type_store_decimal64, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - decimal64, version 1"},
    {.type = LY_TYPE_EMPTY, .store = ly_type_store_empty, .compare = ly_type_compare_empty,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - empty, version 1"},
    {.type = LY_TYPE_ENUM, .store = ly_type_store_enum, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - enumeration, version 1"},
    {.type = LY_TYPE_IDENT, .store = ly_type_store_identityref, .compare = ly_type_compare_identityref,
        .print = ly_type_print_identityref, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
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
    {.type = LY_TYPE_INT8, .store = ly_type_store_int, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT16, .store = ly_type_store_int, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT32, .store = ly_type_store_int, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT64, .store = ly_type_store_int, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
};
