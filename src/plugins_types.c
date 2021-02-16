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
#define _POSIX_C_SOURCE 200809L /* strdup */

#include "plugins_types.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "path.h"
#include "schema_compile.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"


/*
 * @brief function create and fill error structure. Error message is created from parameter err_msg and folowing parameters simulary to function printf.
 *
 * @param [in,out] err_code return code. if thre is not enough memory for allocating error message then this code change to LY_EMEM.
 * @param [in] err_vecode Validity error code in case of LY_EVALID error code.
 * @param [in] err_msg    error foramting message. More info printf
 * @return NULL in case of memory allocation faliture
 * @return Created error information structure that can be freed using ::ly_err_free().
 */
static struct ly_err_item *
ly_err_msg_create(LY_ERR *err_code, LY_VECODE err_vecode, const char *err_msg, ...){
    struct ly_err_item *ret;
    char *msg;
    int  print_ret;
    va_list(print_args);

    if (err_msg != NULL){
        va_start(print_args, err_msg);
        print_ret = vasprintf(&msg, err_msg, print_args);
    }

    if (print_ret < 0 || err_msg == NULL) {
        ret = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
        *err_code = LY_EMEM;
    } else {
        ret = ly_err_new(LY_LLERR, *err_code, err_vecode, msg, NULL, NULL);
    }

    return ret;
}


/**
 * @brief Find import prefix in imports.
 */
static const struct lys_module *
ly_schema_resolve_prefix(const struct ly_ctx *UNUSED(ctx), const char *prefix, size_t prefix_len, void *prefix_data)
{
    const struct lysp_module *prefix_mod = prefix_data;
    struct lys_module *m = NULL;
    LY_ARRAY_COUNT_TYPE u;
    const char *local_prefix;

    local_prefix = prefix_mod->is_submod ? ((struct lysp_submodule *)prefix_mod)->prefix : prefix_mod->mod->prefix;
    if (!prefix_len || !ly_strncmp(local_prefix, prefix, prefix_len)) {
        /* it is the prefix of the module itself */
        m = prefix_mod->mod;
    }

    /* search in imports */
    if (!m) {
        LY_ARRAY_FOR(prefix_mod->imports, u) {
            if (!ly_strncmp(prefix_mod->imports[u].prefix, prefix, prefix_len)) {
                m = prefix_mod->imports[u].module;
                break;
            }
        }
    }

    return m;
}

/**
 * @brief Find resolved module for a prefix in prefix - module pairs.
 */
static const struct lys_module *
ly_schema_resolved_resolve_prefix(const struct ly_ctx *UNUSED(ctx), const char *prefix, size_t prefix_len,
        void *prefix_data)
{
    struct lysc_prefix *prefixes = prefix_data;
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(prefixes, u) {
        if (!ly_strncmp(prefixes[u].prefix, prefix, prefix_len)) {
            return prefixes[u].mod;
        }
    }

    return NULL;
}

/**
 * @brief Find XML namespace prefix in XML namespaces, which are then mapped to modules.
 */
static const struct lys_module *
ly_xml_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *prefix_data)
{
    const struct lyxml_ns *ns;
    const struct ly_set *ns_set = prefix_data;

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
ly_json_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *UNUSED(prefix_data))
{
    return ly_ctx_get_module_implemented2(ctx, prefix, prefix_len);
}

const struct lys_module *
ly_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, LY_PREFIX_FORMAT format, void *prefix_data)
{
    const struct lys_module *mod = NULL;

    LY_CHECK_ARG_RET(ctx, prefix, prefix_len, NULL);

    switch (format) {
    case LY_PREF_SCHEMA:
        mod = ly_schema_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    case LY_PREF_SCHEMA_RESOLVED:
        mod = ly_schema_resolved_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
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

API const struct lys_module *
ly_type_store_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len,
        LY_PREFIX_FORMAT format, void *prefix_data)
{
    return ly_resolve_prefix(ctx, prefix, prefix_len, format, prefix_data);
}

/**
 * @brief Find module in import prefixes.
 */
static const char *
ly_schema_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    const struct lysp_module *pmod = prefix_data;
    LY_ARRAY_COUNT_TYPE u;

    if (pmod->mod == mod) {
        if (pmod->is_submod) {
            return ((struct lysp_submodule *)pmod)->prefix;
        } else {
            return pmod->mod->prefix;
        }
    }

    LY_ARRAY_FOR(pmod->imports, u) {
        if (pmod->imports[u].module == mod) {
            /* match */
            return pmod->imports[u].prefix;
        }
    }

    return NULL;
}

/**
 * @brief Find prefix in prefix - module pairs.
 */
static const char *
ly_schema_resolved_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    struct lysc_prefix *prefixes = prefix_data;
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(prefixes, u) {
        if (prefixes[u].mod == mod) {
            return prefixes[u].prefix;
        }
    }

    return NULL;
}

/**
 * @brief Simply return module local prefix. Also, store the module in a set.
 */
static const char *
ly_xml_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    struct ly_set *ns_list = prefix_data;

    LY_CHECK_RET(ly_set_add(ns_list, (void *)mod, 0, NULL), NULL);
    return mod->prefix;
}

/**
 * @brief Simply return module name.
 */
static const char *
ly_json_get_prefix(const struct lys_module *mod, void *UNUSED(prefix_data))
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
    case LY_PREF_SCHEMA_RESOLVED:
        prefix = ly_schema_resolved_get_prefix(mod, prefix_data);
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

API const char *
ly_type_print_get_prefix(const struct lys_module *mod, LY_PREFIX_FORMAT format, void *prefix_data)
{
    return ly_get_prefix(mod, format, prefix_data);
}

/**
 * @brief Generic comparison callback checking the canonical value.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_simple(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1->canonical == val2->canonical) {
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
        void *UNUSED(prefix_data), ly_bool *dynamic)
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
    LY_CHECK_RET(lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical));
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
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

    if (!value || !value[0] || !value_len) {
        rc = asprintf(&errmsg, "Invalid empty %s value.", datatype);
        goto error;
    }

    switch (ly_parse_int(value, value_len, min, max, base, ret)) {
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
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
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
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

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
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
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
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

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
    if ((len < value_len) && ((value[len] != '.') || !isdigit(value[len + 1]))) {
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
        uint64_t u;
        for (u = len + trailing_zeros; u < value_len && isspace(value[u]); ++u) {}
        if (u != value_len) {
            if (asprintf(&errmsg, "Invalid %" PRIu64 ". character of decimal64 value \"%.*s\".",
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
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
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

    rc = ly_type_parse_int("decimal64", LY_BASE_DEC, INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807),
            valcopy, len, &d, err);
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
    int rc;
    LY_ARRAY_COUNT_TYPE u;
    char *errmsg;
    pcre2_match_data *match_data = NULL;

    LY_CHECK_ARG_RET(NULL, str, err, LY_EINVAL);

    LY_ARRAY_FOR(patterns, u) {
        /* match_data needs to be allocated each time because of possible multi-threaded evaluation */
        match_data = pcre2_match_data_create_from_pattern(patterns[u]->code, NULL);
        if (!match_data) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
            return LY_EMEM;
        }

        rc = pcre2_match(patterns[u]->code, (PCRE2_SPTR)str, str_len, 0, PCRE2_ANCHORED | PCRE2_ENDANCHORED, match_data, NULL);
        pcre2_match_data_free(match_data);

        if ((rc != PCRE2_ERROR_NOMATCH) && (rc < 0)) {
            PCRE2_UCHAR pcre2_errmsg[LY_PCRE2_MSG_LIMIT] = {0};
            pcre2_get_error_message(rc, pcre2_errmsg, LY_PCRE2_MSG_LIMIT);

            *err = ly_err_new(LY_LLERR, LY_ESYS, 0, strdup((const char *)pcre2_errmsg), NULL, NULL);
            return LY_ESYS;
        } else if (((rc == PCRE2_ERROR_NOMATCH) && (patterns[u]->inverted == 0)) ||
                ((rc != PCRE2_ERROR_NOMATCH) && (patterns[u]->inverted == 1))) {
            LY_ERR ret = 0;
            const char *inverted = patterns[u]->inverted == 0 ? " " : " inverted ";
            if (asprintf(&errmsg, "String \"%.*s\" does not conform to the%spattern \"%s\".",
                    (int)str_len, str, inverted, patterns[u]->expr) == -1) {
                *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
                ret = LY_EMEM;
            } else {
                *err = ly_err_new(LY_LLERR, LY_ESYS, 0, errmsg, NULL, NULL);
                ret = LY_EVALID;
            }
            return ret;
        }
    }
    return LY_SUCCESS;
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
            } else if (value <= range->parts[u].max_64) {
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
    if ((rc == -1) || !errmsg) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
        return LY_EMEM;
    } else {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, range->eapptag ? strdup(range->eapptag) : NULL);
        return LY_EVALID;
    }
}

API LY_ERR
lysc_prefixes_compile(const char *str, size_t str_len, const struct lysp_module *prefix_mod, struct lysc_prefix **prefixes)
{
    LY_ERR ret;
    LY_PREFIX_FORMAT format = LY_PREF_SCHEMA_RESOLVED;

    LY_CHECK_ARG_RET(NULL, prefix_mod, prefixes, LY_EINVAL);

    ret = ly_store_prefix_data(prefix_mod->mod->ctx, str, str_len, LY_PREF_SCHEMA, (void *)prefix_mod, &format,
            (void **)prefixes);
    assert(format == LY_PREF_SCHEMA_RESOLVED);
    return ret;
}

API LY_ERR
lysc_prefixes_dup(const struct lysc_prefix *orig, struct lysc_prefix **dup)
{
    LY_CHECK_ARG_RET(NULL, dup, LY_EINVAL);

    *dup = NULL;

    if (!orig) {
        return LY_SUCCESS;
    }

    return ly_dup_prefix_data(NULL, LY_PREF_SCHEMA_RESOLVED, orig, (void **)dup);
}

API void
lysc_prefixes_free(struct lysc_prefix *prefixes)
{
    ly_free_prefix_data(LY_PREF_SCHEMA_RESOLVED, prefixes);
}

static int
type_get_hints_base(uint32_t hints)
{
    /* set allowed base */
    switch (hints & (LYD_VALHINT_DECNUM | LYD_VALHINT_OCTNUM | LYD_VALHINT_HEXNUM)) {
    case LYD_VALHINT_DECNUM:
        return LY_BASE_DEC;
    case LYD_VALHINT_OCTNUM:
        return LY_BASE_OCT;
    case LYD_VALHINT_HEXNUM:
        return LY_BASE_HEX;
    default:
        break;
    }

    return 0;
}

/**
 * @brief Answer if the type is suitable for the parser's hit (if any) in the specified format
 */
static LY_ERR
type_check_hints(uint32_t hints, const char *value, size_t value_len, LY_DATA_TYPE type, int *base, struct ly_err_item **err)
{
    int rc;
    char *msg;

    switch (type) {
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
        if (!(hints & (LYD_VALHINT_DECNUM | LYD_VALHINT_OCTNUM | LYD_VALHINT_HEXNUM))) {
            rc = asprintf(&msg, "Invalid non-number-encoded %s value \"%.*s\".", lys_datatype2str(type), (int)value_len, value);
            goto error;
        }
        *base = type_get_hints_base(hints);
        break;
    case LY_TYPE_UINT64:
    case LY_TYPE_INT64:
        if (!(hints & LYD_VALHINT_NUM64)) {
            rc = asprintf(&msg, "Invalid non-num64-encoded %s value \"%.*s\".", lys_datatype2str(type), (int)value_len, value);
            goto error;
        }
        *base = type_get_hints_base(hints);
        break;
    case LY_TYPE_STRING:
    case LY_TYPE_DEC64:
    case LY_TYPE_ENUM:
    case LY_TYPE_BITS:
    case LY_TYPE_BINARY:
    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
        if (!(hints & LYD_VALHINT_STRING)) {
            rc = asprintf(&msg, "Invalid non-string-encoded %s value \"%.*s\".", lys_datatype2str(type), (int)value_len, value);
            goto error;
        }
        break;
    case LY_TYPE_BOOL:
        if (!(hints & LYD_VALHINT_BOOLEAN)) {
            rc = asprintf(&msg, "Invalid non-boolean-encoded %s value \"%.*s\".", lys_datatype2str(type), (int)value_len, value);
            goto error;
        }
        break;
    case LY_TYPE_EMPTY:
        if (!(hints & LYD_VALHINT_EMPTY)) {
            rc = asprintf(&msg, "Invalid non-empty-encoded %s value \"%.*s\".", lys_datatype2str(type), (int)value_len, value);
            goto error;
        }
        break;
    case LY_TYPE_UNKNOWN:
    case LY_TYPE_LEAFREF:
    case LY_TYPE_UNION:
        LOGINT_RET(NULL);
    }

    return LY_SUCCESS;

error:
    if ((rc == -1) || !msg) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
        return LY_EMEM;
    } else {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, msg, NULL, NULL);
        return LY_EVALID;
    }
}

/**
 * @brief Validate, canonize and store value of the YANG built-in signed integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_int(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int64_t num;
    int base;
    char *str;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, &base, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    switch (type->basetype) {
    case LY_TYPE_INT8:
        ret = ly_type_parse_int("int8", base, INT64_C(-128), INT64_C(127), value, value_len, &num, err);
        break;
    case LY_TYPE_INT16:
        ret = ly_type_parse_int("int16", base, INT64_C(-32768), INT64_C(32767), value, value_len, &num, err);
        break;
    case LY_TYPE_INT32:
        ret = ly_type_parse_int("int32", base, INT64_C(-2147483648), INT64_C(2147483647), value, value_len, &num, err);
        break;
    case LY_TYPE_INT64:
        ret = ly_type_parse_int("int64", base, INT64_C(-9223372036854775807) - INT64_C(1),
                INT64_C(9223372036854775807), value, value_len, &num, err);
        break;
    default:
        LOGINT(ctx);
        ret = LY_EINT;
    }

    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    LY_CHECK_ERR_GOTO(asprintf(&str, "%" PRId64, num) == -1, ret = LY_EMEM, cleanup);

    /* range of the number */
    if (type_num->range) {
        ret = ly_type_validate_range(type->basetype, type_num->range, num, str, err);
        LY_CHECK_ERR_GOTO(ret != LY_SUCCESS, free(str), cleanup);
    }

    /* store everything */
    ret = lydict_insert_zc(ctx, str, &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->int64 = num;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

/**
 * @brief Validate and canonize value of the YANG built-in unsigned integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_uint(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint64_t num;
    int base = 0;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;
    char *str;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, &base, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    switch (type->basetype) {
    case LY_TYPE_UINT8:
        ret = ly_type_parse_uint("uint8", base, UINT64_C(255), value, value_len, &num, err);
        break;
    case LY_TYPE_UINT16:
        ret = ly_type_parse_uint("uint16", base, UINT64_C(65535), value, value_len, &num, err);
        break;
    case LY_TYPE_UINT32:
        ret = ly_type_parse_uint("uint32", base, UINT64_C(4294967295), value, value_len, &num, err);
        break;
    case LY_TYPE_UINT64:
        ret = ly_type_parse_uint("uint64", base, UINT64_C(18446744073709551615), value, value_len, &num, err);
        break;
    default:
        LOGINT(ctx);
        ret = LY_EINT;
    }

    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    LY_CHECK_ERR_GOTO(asprintf(&str, "%" PRIu64, num) == -1, ret = LY_EMEM, cleanup);

    /* range of the number */
    if (type_num->range) {
        ret = ly_type_validate_range(type->basetype, type_num->range, num, str, err);
        LY_CHECK_ERR_GOTO(ret != LY_SUCCESS, free(str), cleanup);
    }

    /* store everything */
    ret = lydict_insert_zc(ctx, str, &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->int64 = num;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in decimal64 types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_decimal64(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int64_t d;
    struct lysc_type_dec *type_dec = (struct lysc_type_dec *)type;
    char buf[LY_NUMBER_MAXLEN];

    if (!value || !value[0] || !value_len) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, strdup("Invalid empty decimal64 value."), NULL, NULL);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    ret = ly_type_parse_dec64(type_dec->fraction_digits, value, value_len, &d, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    /* prepare canonized value */
    if (d) {
        int count = sprintf(buf, "%" PRId64 " ", d);
        if (((d > 0) && ((count - 1) <= type_dec->fraction_digits)) ||
                ((count - 2) <= type_dec->fraction_digits)) {
            /* we have 0. value, print the value with the leading zeros
             * (one for 0. and also keep the correct with of num according
             * to fraction-digits value)
             * for (num<0) - extra character for '-' sign */
            count = sprintf(buf, "%0*" PRId64 " ", (d > 0) ? (type_dec->fraction_digits + 1) : (type_dec->fraction_digits + 2), d);
        }
        for (uint8_t i = type_dec->fraction_digits, j = 1; i > 0; i--) {
            if (j && (i > 1) && (buf[count - 2] == '0')) {
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
        ret = ly_type_validate_range(type->basetype, type_dec->range, d, buf, err);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }

    ret = lydict_insert(ctx, buf, strlen(buf), &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->dec64 = d;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in binary type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_binary(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    size_t base64_start, base64_end, base64_count, base64_terminated, value_end;
    struct lysc_type_bin *type_bin = (struct lysc_type_bin *)type;

    LY_CHECK_ARG_RET(ctx, value, LY_EINVAL);

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* validate characters and remember the number of octets for length validation */
    /* silently skip leading whitespaces */
    base64_start = 0;
    while (base64_start < value_len && isspace(value[base64_start])) {
        base64_start++;
    }
    /* silently skip trailing whitespace */
    value_end = value_len;
    while (base64_start < value_end && isspace(value[value_end - 1])) {
        value_end--;
    }

    /* find end of base64 value */
    base64_end = base64_start;
    base64_count = 0;
    while ((base64_end < value_len) &&
            /* check correct character in base64 */
            ((('A' <= value[base64_end]) && (value[base64_end] <= 'Z')) ||
            (('a' <= value[base64_end]) && (value[base64_end] <= 'z')) ||
            (('0' <= value[base64_end]) && (value[base64_end] <= '9')) ||
            ('+' == value[base64_end]) ||
            ('/' == value[base64_end]) ||
            ('\n' == value[base64_end]))) {

        if ('\n' != value[base64_end]) {
            base64_count++;
        }
        base64_end++;
    }

    /* find end of padding */
    base64_terminated = 0;
    while (((base64_end < value_len) && (base64_terminated < 2)) &&
            /* check padding on end of string */
            (('=' == value[base64_end]) ||
            ('\n' == value[base64_end]))) {

        if ('\n' != value[base64_end]) {
            base64_terminated++;
        }
        base64_end++;
    }

    /* check if value is valid base64 value */
    if (value_end != base64_end) {
        ret = LY_EVALID;
        *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid Base64 character (%c).", value[base64_end]);
        goto cleanup;
    }

    if ((base64_count + base64_terminated) & 3) {
        /* base64 length must be multiple of 4 chars */
        ret = LY_EVALID;
        *err = ly_err_msg_create(&ret, LYVE_DATA, "Base64 encoded value length must be divisible by 4.");
        goto cleanup;
    }

    /* check if value meets the type requirments */
    if (type_bin->length) {
        const uint32_t value_length = ((base64_count + base64_terminated) / 4) * 3 - base64_terminated;
        ret = ly_type_validate_range(LY_TYPE_BINARY, type_bin->length, value_length, value, err);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }

    if (base64_count != 0) {
        ret = lydict_insert(ctx, &value[base64_start], base64_end - base64_start, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, "", 0, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->ptr = NULL;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

/**
 * @brief Validate and store value of the YANG built-in string type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_string(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* length restriction of the string */
    if (type_str->length) {
        char buf[LY_NUMBER_MAXLEN];
        size_t char_count = ly_utf8len(value, value_len);

        /* value_len is in bytes, but we need number of chaarcters here */
        snprintf(buf, LY_NUMBER_MAXLEN, "%zu", char_count);
        ret = ly_type_validate_range(LY_TYPE_BINARY, type_str->length, char_count, buf, err);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }

    /* pattern restrictions */
    ret = ly_type_validate_patterns(type_str->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->canonical);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->ptr = NULL;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

/**
 * @brief Validate, canonize and store value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_bits(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    size_t item_len;
    const char *item;
    struct ly_set *items = NULL, *items_ordered = NULL;
    size_t buf_size = 0;
    char *buf = NULL;
    size_t index;
    LY_ARRAY_COUNT_TYPE u;
    char *errmsg = NULL;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)type;
    ly_bool iscanonical = 1;
    size_t ws_count;
    size_t lws_count; /* leading whitespace count */
    const char *can = NULL;
    int rc = 0;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup_value);

    /* remember the present items for further work */
    ret = ly_set_new(&items);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup_value);

    ws_count = lws_count = 0;
    for (index = 0; index < value_len; ++index) {
        if (isspace(value[index])) {
            ++ws_count;
            continue;
        }
        if (index == ws_count) {
            lws_count = ws_count;
        } else if (ws_count) {
            iscanonical = 0;
        }
        ws_count = 0;

        /* start of the item */
        item = &value[index];
        for (item_len = 0; index + item_len < value_len && !isspace(item[item_len]); item_len++) {}
        LY_ARRAY_FOR(type_bits->bits, u) {
            if (!ly_strncmp(type_bits->bits[u].name, item, item_len)) {
                /* we have the match */
                uint32_t inserted;

                if (iscanonical && items->count && (type_bits->bits[u].position < ((struct lysc_type_bitenum_item *)items->objs[items->count - 1])->position)) {
                    iscanonical = 0;
                }
                ret = ly_set_add(items, &type_bits->bits[u], 0, &inserted);
                LY_CHECK_GOTO(ret, cleanup);
                if (inserted != items->count - 1) {
                    rc = asprintf(&errmsg, "Bit \"%s\" used multiple times.", type_bits->bits[u].name);
                    goto cleanup;
                }
                goto next;
            }
        }
        /* item not found */
        rc = asprintf(&errmsg, "Invalid bit value \"%.*s\".", (int)item_len, item);
        goto cleanup;
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

        if (!ws_count && !lws_count && (options & LY_TYPE_STORE_DYNAMIC)) {
            ret = lydict_insert_zc(ctx, (char *)value, &can);
            LY_CHECK_GOTO(ret, cleanup);
            value = NULL;
            options &= ~LY_TYPE_STORE_DYNAMIC;
        } else {
            ret = lydict_insert(ctx, value_len ? &value[lws_count] : "", value_len - ws_count - lws_count, &can);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        buf = malloc(buf_size * sizeof *buf);
        LY_CHECK_ERR_GOTO(!buf, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        index = 0;

        ret = ly_set_dup(items, NULL, &items_ordered);
        LY_CHECK_GOTO(ret, cleanup);
        items_ordered->count = 0;

        /* generate ordered bits list */
        LY_ARRAY_FOR(type_bits->bits, u) {
            if (ly_set_contains(items, &type_bits->bits[u], NULL)) {
                int c = sprintf(&buf[index], "%s%s", index ? " " : "", type_bits->bits[u].name);
                LY_CHECK_ERR_GOTO(c < 0, LOGERR(ctx, LY_ESYS, "sprintf() failed."); ret = LY_ESYS, cleanup);
                index += c;
                ret = ly_set_add(items_ordered, &type_bits->bits[u], 1, NULL);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }
        assert(buf_size == index + 1);
        /* termination NULL-byte */
        buf[index] = '\0';

        ret = lydict_insert_zc(ctx, buf, &can);
        buf = NULL;
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* store all data */
    storage->canonical = can;
    can = NULL;
    LY_ARRAY_CREATE_GOTO(ctx, storage->bits_items, items_ordered->count, ret, cleanup);
    for (uint32_t x = 0; x < items_ordered->count; x++) {
        storage->bits_items[x] = items_ordered->objs[x];
        LY_ARRAY_INCREMENT(storage->bits_items);
    }
    storage->realtype = type;

cleanup:
    if (rc == -1) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
        ret = LY_EMEM;
    } else if (errmsg) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        ret = LY_EVALID;
    }
    ly_set_free(items, NULL);
    ly_set_free(items_ordered, NULL);
    free(buf);
    lydict_remove(ctx, can);
cleanup_value:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

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

    LY_CHECK_RET(lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical));
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
ly_type_store_enum(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_type_enum *type_enum = (struct lysc_type_enum *)type;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    /* find the matching enumeration value item */
    LY_ARRAY_FOR(type_enum->enums, u) {
        if (!ly_strncmp(type_enum->enums[u].name, value, value_len)) {
            /* we have a match */
            goto match;
        }
    }
    /* enum not found */
    ret  = LY_EVALID;
    *err = ly_err_msg_create(&ret, LYVE_DATA,
            "Invalid enumeration value \"%.*s\".", (int)value_len, value);
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

/**
 * @brief Validate and store value of the YANG built-in boolean type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_boolean(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int8_t i;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    if ((value_len == ly_strlen_const("true")) && !strncmp(value, "true", ly_strlen_const("true"))) {
        i = 1;
    } else if ((value_len == ly_strlen_const("false")) && !strncmp(value, "false", ly_strlen_const("false"))) {
        i = 0;
    } else {
        char *errmsg;
        if (asprintf(&errmsg, "Invalid boolean value \"%.*s\".", (int)value_len, value) == -1) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
            ret = LY_EMEM;
        } else {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
            ret = LY_EVALID;
        }
        goto cleanup;
    }

    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->canonical);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, value, value_len, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->boolean = i;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

/**
 * @brief Validate and store value of the YANG built-in empty type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_empty(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

    if (value_len) {
        char *errmsg;
        if (asprintf(&errmsg, "Invalid empty value \"%.*s\".", (int)value_len, value) == -1) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
            ret = LY_EMEM;
        } else {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
            ret = LY_EVALID;
        }
        goto cleanup;
    }

    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->canonical);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    } else {
        ret = lydict_insert(ctx, "", value_len, &storage->canonical);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    }
    storage->ptr = NULL;
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }
    return ret;
}

/**
 * @brief Comparison callback for built-in empty type.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_empty(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

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
        ly_bool *dynamic);

/**
 * @brief Validate, canonize and store value of the YANG built-in identiytref type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_identityref(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
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

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
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
        ret = LY_EVALID;
        *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid empty identityref value.");
        goto cleanup;
    }

    if (prefix_len) {
        mod = ly_type_store_resolve_prefix(ctx, prefix, prefix_len, format, prefix_data);
    } else {
        switch (format) {
        case LY_PREF_SCHEMA:
        case LY_PREF_SCHEMA_RESOLVED:
            /* use context node module, handles augments */
            mod = ctx_node->module;
            break;
        case LY_PREF_JSON:
            /* use context node module (as specified) */
            mod = ctx_node->module;
            break;
        case LY_PREF_XML:
            /* use the default namespace */
            mod = ly_xml_resolve_prefix(ctx, NULL, 0, prefix_data);
            break;
        }
    }
    if (!mod) {
        ret = LY_EVALID;
        *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid identityref \"%.*s\" value - unable to map prefix to YANG schema.",
                (int)value_len, value);
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
        ret = LY_EVALID;
        *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid identityref \"%.*s\" value - identity not found in module \"%s\".",
                (int)value_len, value, mod->name);
        goto cleanup;
    } else if (!mod->implemented) {
        /* non-implemented module */
        if (options & LY_TYPE_STORE_IMPLEMENT) {
            ret = lys_set_implemented_r((struct lys_module *)mod, NULL, unres);
            LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
        } else {
            ret = LY_EVALID;
            *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid identityref \"%.*s\" value - identity found in non-implemented module \"%s\".",
                    (int)value_len, value, mod->name);
            goto cleanup;
        }
    }

    /* check that the identity matches some of the type's base identities */
    LY_ARRAY_FOR(type_ident->bases, u) {
        if (!ly_type_identity_isderived(type_ident->bases[u], ident)) {
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
        ret = LY_EVALID;
        if (u == 1) {
            *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid identityref \"%.*s\" value - identity not derived from the base %s.",
                    (int)value_len, value, str);
        } else {
            *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid identityref \"%.*s\" value - identity not derived from all the bases %s.",
                    (int)value_len, value, str);
        }
        free(str);
        goto cleanup;
    }

    storage->ident = ident;

    /* get JSON form since there is no canonical */
    str = (char *)ly_type_print_identityref(storage, LY_PREF_JSON, NULL, &dyn);
    assert(str && dyn);
    ret = lydict_insert_zc(ctx, str, &storage->canonical);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    storage->realtype = type;

cleanup:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

/**
 * @brief Comparison callback for built-in identityref type.
 *
 * Implementation of the ly_type_compare_clb.
 */
static LY_ERR
ly_type_compare_identityref(const struct lyd_value *val1, const struct lyd_value *val2)
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
 * @brief Printer callback printing identityref value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_identityref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic)
{
    char *result = NULL;

    *dynamic = 1;
    if (asprintf(&result, "%s:%s", ly_type_print_get_prefix(value->ident->module, format, prefix_data), value->ident->name) == -1) {
        return NULL;
    } else {
        return result;
    }
}

static const char *ly_type_print_instanceid(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data,
        ly_bool *dynamic);

/**
 * @brief Validate and store value of the YANG built-in instance-identifier type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)type;
    char *str;
    struct ly_path *path = NULL;
    struct lyxp_expr *exp = NULL;
    uint32_t prefix_opt = 0;
    ly_bool dyn;

    /* init */
    *err = NULL;

    /* check hints */
    ret = type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS,  cleanup_value);

    switch (format) {
    case LY_PREF_SCHEMA:
    case LY_PREF_SCHEMA_RESOLVED:
    case LY_PREF_XML:
        prefix_opt = LY_PATH_PREFIX_MANDATORY;
        break;
    case LY_PREF_JSON:
        prefix_opt = LY_PATH_PREFIX_STRICT_INHERIT;
        break;
    }

    /* parse the value */
    ret = ly_path_parse(ctx, ctx_node, value, value_len, LY_PATH_BEGIN_ABSOLUTE, LY_PATH_LREF_FALSE,
            prefix_opt, LY_PATH_PRED_SIMPLE, &exp);
    if (ret) {
        ret = LY_EVALID;
        *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid instance-identifier \"%.*s\" value - syntax error.", (int)value_len, value);
        goto cleanup;
    }

    if (options & LY_TYPE_STORE_IMPLEMENT) {
        /* implement all prefixes */
        LY_CHECK_GOTO(ret = lys_compile_expr_implement(ctx, exp, format, prefix_data, 1, unres, NULL), cleanup);
    }

    /* resolve it on schema tree */
    ret = ly_path_compile(ctx, NULL, ctx_node, exp, LY_PATH_LREF_FALSE, (ctx_node->flags & LYS_IS_OUTPUT) ?
            LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_SINGLE, format, prefix_data, unres, &path);
    if (ret) {
        *err = ly_err_msg_create(&ret, LYVE_DATA, "Invalid instance-identifier \"%.*s\" value - semantic error.", (int)value_len, value);
        goto cleanup;
    }

    /* store resolved schema path */
    storage->target = path;
    path = NULL;

    /* store JSON string value */
    str = (char *)ly_type_print_instanceid(storage, LY_PREF_JSON, NULL, &dyn);
    assert(str && dyn);
    LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, str, &storage->canonical), cleanup);
    storage->realtype = type;

    /* cleanup */
cleanup:
    lyxp_expr_free(ctx, exp);
    ly_path_free(ctx, path);

cleanup_value:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    if ((ret == LY_SUCCESS) && type_inst->require_instance) {
        /* needs to be resolved */
        return LY_EINCOMPLETE;
    } else {
        return ret;
    }
}

/**
 * @brief Validate value of the YANG built-in instance-identifier type.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_instanceid(const struct ly_ctx *UNUSED(ctx), const struct lysc_type *UNUSED(type),
        const struct lyd_node *UNUSED(ctx_node), const struct lyd_node *tree, struct lyd_value *storage,
        struct ly_err_item **err)
{
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)storage->realtype;
    LY_ERR ret;
    char *errmsg;

    *err = NULL;

    if (!type_inst->require_instance) {
        /* redundant to resolve */
        return LY_SUCCESS;
    }

    /* find the target in data */
    ret = ly_path_eval(storage->target, tree, NULL);
    if (ret) {
        if (asprintf(&errmsg, "Invalid instance-identifier \"%s\" value - required instance not found.", storage->canonical) == -1) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
            ret = LY_EMEM;
        } else {
            *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        }
        return ret;
    }

    return LY_SUCCESS;
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

    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1 == val2) {
        return LY_SUCCESS;
    } else if (!val1->target || !val2->target || (LY_ARRAY_COUNT(val1->target) != LY_ARRAY_COUNT(val2->target))) {
        return LY_ENOT;
    }

    LY_ARRAY_FOR(val1->target, u) {
        struct ly_path *s1 = &val1->target[u];
        struct ly_path *s2 = &val2->target[u];

        if ((s1->node != s2->node) || (s1->pred_type != s2->pred_type) ||
                (s1->predicates && (LY_ARRAY_COUNT(s1->predicates) != LY_ARRAY_COUNT(s2->predicates)))) {
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
                    if ((pred1->key != pred2->key) || ((struct lysc_node_leaf *)pred1->key)->type->plugin->compare(&pred1->value, &pred2->value)) {
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
ly_type_print_instanceid(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic)
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
            ly_strcat(&result, "/%s:%s", ly_type_print_get_prefix(value->target[u].node->module, format, prefix_data),
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
                case LY_PATH_PREDTYPE_LIST: {
                    /* key-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s:%s=%c%s%c]", ly_type_print_get_prefix(pred->key->module, format, prefix_data),
                            pred->key->name, quot, str, quot);
                    if (d) {
                        free((char *)str);
                    }
                    break;
                }
                case LY_PATH_PREDTYPE_LEAFLIST: {
                    /* leaf-list-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, str, quot);
                    if (d) {
                        free((char *)str);
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
                ly_strcat(&result, "/%s:%s", ly_type_print_get_prefix(mod, format, prefix_data), value->target[u].node->name);
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
                case LY_PATH_PREDTYPE_LIST: {
                    /* key-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s=%c%s%c]", pred->key->name, quot, str, quot);
                    if (d) {
                        free((char *)str);
                    }
                    break;
                }
                case LY_PATH_PREDTYPE_LEAFLIST: {
                    /* leaf-list-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, str, quot);
                    if (d) {
                        free((char *)str);
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
    LY_CHECK_RET(lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical));
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
    ly_bool dynamic;
    uint32_t i;
    int rc;

    /* find all target data instances */
    ret = lyxp_eval(lref->cur_mod->ctx, lref->path, lref->cur_mod, LY_PREF_SCHEMA_RESOLVED, lref->prefixes, node, tree,
            &set, 0);
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
        if (set.used) {
            rc = asprintf(errmsg, "Invalid leafref value \"%s\" - no target instance \"%s\" with the same value.",
                    val_str, lref->path->expr);
        } else {
            rc = asprintf(errmsg, "Invalid leafref value \"%s\" - no existing target instance \"%s\".",
                    val_str, lref->path->expr);
        }
        if (rc == -1) {
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
 * @brief Store and canonize value of the YANG built-in leafref type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_leafref(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref *)type;

    assert(type_lr->realtype);

    /* store the value as the real type of the leafref target */
    ret = type_lr->realtype->plugin->store(ctx, type_lr->realtype, value, value_len, options, format, prefix_data,
            hints, ctx_node, storage, unres, err);
    if (ret == LY_EINCOMPLETE) {
        /* it is irrelevant whether the target type needs some resolving */
        ret = LY_SUCCESS;
    }
    LY_CHECK_RET(ret);

    if (type_lr->require_instance) {
        /* needs to be resolved */
        return LY_EINCOMPLETE;
    } else {
        return LY_SUCCESS;
    }
}

/**
 * @brief Validate value of the YANG built-in leafref type.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_leafref(const struct ly_ctx *UNUSED(ctx), const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err)
{
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref *)type;
    char *errmsg = NULL;

    *err = NULL;

    if (!type_lr->require_instance) {
        /* redundant to resolve */
        return LY_SUCCESS;
    }

    /* check leafref target existence */
    if (ly_type_find_leafref(type_lr, ctx_node, storage, tree, NULL, &errmsg)) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        return LY_EVALID;
    }

    return LY_SUCCESS;
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
ly_type_print_leafref(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic)
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

static LY_ERR
ly_type_union_store_type(const struct ly_ctx *ctx, struct lysc_type **types, struct lyd_value_subvalue *subvalue,
        ly_bool resolve, const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lys_glob_unres *unres,
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    char *errmsg = NULL;
    uint32_t prev_lo;

    if (!types || !LY_ARRAY_COUNT(types)) {
        return LY_EINVAL;
    }

    /* turn logging off */
    prev_lo = ly_log_options(0);

    /* use the first usable subtype to store the value */
    for (u = 0; u < LY_ARRAY_COUNT(types); ++u) {
        ret = types[u]->plugin->store(ctx, types[u], subvalue->original, strlen(subvalue->original), 0, subvalue->format,
                subvalue->prefix_data, subvalue->hints, subvalue->ctx_node, &subvalue->value, unres, err);
        if ((ret == LY_SUCCESS) || (ret == LY_EINCOMPLETE)) {
            if (resolve && (ret == LY_EINCOMPLETE)) {
                /* we need the value resolved */
                ret = subvalue->value.realtype->plugin->validate(ctx, types[u], ctx_node, tree, &subvalue->value, err);
                if (!ret) {
                    /* store and resolve successful */
                    break;
                }

                /* resolve failed, we need to free the stored value */
                types[u]->plugin->free(ctx, &subvalue->value);
            } else {
                /* store successful */
                break;
            }
        }
        ly_err_free(*err);
        *err = NULL;
    }

    if (u == LY_ARRAY_COUNT(types)) {
        if (asprintf(&errmsg, "Invalid union value \"%s\" - no matching subtype found.", subvalue->original) == -1) {
            *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
            ret = LY_EMEM;
            goto cleanup;
        }
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_DATA, errmsg, NULL, NULL);
        ret = LY_EVALID;
        goto cleanup;
    }

cleanup:
    /* restore logging */
    ly_log_options(prev_lo);
    return ret;
}

/**
 * @brief Store and canonize value of the YANG built-in union type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_union(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)type;
    struct lyd_value_subvalue *subvalue = NULL;

    /* prepare subvalue storage */
    subvalue = calloc(1, sizeof *subvalue);
    if (!subvalue) {
        *err = ly_err_new(LY_LLERR, LY_EMEM, 0, LY_EMEM_MSG, NULL, NULL);
        ret = LY_EMEM;
        goto cleanup_value;
    }

    /* remember the original value */
    if (options & LY_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &subvalue->original);
        options &= ~LY_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &subvalue->original);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* store format-specific data for later prefix resolution */
    ret = ly_store_prefix_data(ctx, subvalue->original, value_len, format, prefix_data, &subvalue->format,
            &subvalue->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);
    subvalue->hints = hints;
    subvalue->ctx_node = ctx_node;

    /* use the first usable subtype to store the value */
    ret = ly_type_union_store_type(ctx, type_u->types, subvalue, 0, NULL, NULL, unres, err);
    LY_CHECK_GOTO((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE), cleanup);

cleanup:
    if ((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE)) {
        lydict_remove(ctx, subvalue->original);
        ly_free_prefix_data(subvalue->format, subvalue->prefix_data);
        free(subvalue);
    } else {
        /* store it as union, the specific type is in the subvalue, but canonical value is the specific type value */
        ret = lydict_insert(ctx, subvalue->value.canonical, 0, &storage->canonical);
        LY_CHECK_GOTO(ret, cleanup);
        storage->subvalue = subvalue;
        storage->realtype = type;
    }

cleanup_value:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    return ret;
}

/**
 * @brief Validate value of the YANG built-in union type.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_union(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)storage->realtype;
    struct lyd_value_subvalue *subvalue = storage->subvalue;

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

    /* store and resolve the value */
    ret = ly_type_union_store_type(ctx, type_u->types, subvalue, 1, ctx_node, tree, NULL, err);
    LY_CHECK_RET(ret);

    /* success, update the canonical value */
    lydict_remove(ctx, storage->canonical);
    LY_CHECK_RET(lydict_insert(ctx, subvalue->value.canonical, 0, &storage->canonical));
    return LY_SUCCESS;
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

    if (val1->subvalue->value.realtype != val2->subvalue->value.realtype) {
        return LY_ENOT;
    }
    return val1->subvalue->value.realtype->plugin->compare(&val1->subvalue->value, &val2->subvalue->value);
}

/**
 * @brief Printer callback printing the union value.
 *
 * Implementation of the ly_type_print_clb.
 */
static const char *
ly_type_print_union(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic)
{
    return value->subvalue->value.realtype->plugin->print(&value->subvalue->value, format, prefix_data, dynamic);
}

/**
 * @brief Duplication callback of the union values.
 *
 * Implementation of the ly_type_dup_clb.
 */
static LY_ERR
ly_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_CHECK_RET(lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical));

    dup->subvalue = calloc(1, sizeof *dup->subvalue);
    LY_CHECK_ERR_RET(!dup->subvalue, LOGMEM(ctx), LY_EMEM);
    LY_CHECK_RET(original->subvalue->value.realtype->plugin->duplicate(ctx, &original->subvalue->value, &dup->subvalue->value));

    LY_CHECK_RET(lydict_insert(ctx, original->subvalue->original, strlen(original->subvalue->original),
            &dup->subvalue->original));
    dup->subvalue->format = original->subvalue->format;
    LY_CHECK_RET(ly_dup_prefix_data(ctx, original->subvalue->format, original->subvalue->prefix_data,
            &dup->subvalue->prefix_data));

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
        if (value->subvalue->value.realtype) {
            value->subvalue->value.realtype->plugin->free(ctx, &value->subvalue->value);
        }
        ly_free_prefix_data(value->subvalue->format, value->subvalue->prefix_data);
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
    {.type = LY_TYPE_BINARY, .store = ly_type_store_binary, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - binary, version 1"},
    {.type = LY_TYPE_UINT8, .store = ly_type_store_uint, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT16, .store = ly_type_store_uint, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT32, .store = ly_type_store_uint, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_UINT64, .store = ly_type_store_uint, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - unsigned integer, version 1"},
    {.type = LY_TYPE_STRING, .store = ly_type_store_string, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - string, version 1"},
    {.type = LY_TYPE_BITS, .store = ly_type_store_bits, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_bits, .free = ly_type_free_bits,
        .id = "libyang 2 - bits, version 1"},
    {.type = LY_TYPE_BOOL, .store = ly_type_store_boolean, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - boolean, version 1"},
    {.type = LY_TYPE_DEC64, .store = ly_type_store_decimal64, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - decimal64, version 1"},
    {.type = LY_TYPE_EMPTY, .store = ly_type_store_empty, .validate = NULL, .compare = ly_type_compare_empty,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - empty, version 1"},
    {.type = LY_TYPE_ENUM, .store = ly_type_store_enum, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - enumeration, version 1"},
    {.type = LY_TYPE_IDENT, .store = ly_type_store_identityref, .validate = NULL, .compare = ly_type_compare_identityref,
        .print = ly_type_print_identityref, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - identityref, version 1"},
    {.type = LY_TYPE_INST, .store = ly_type_store_instanceid, .validate = ly_type_validate_instanceid,
        .compare = ly_type_compare_instanceid, .print = ly_type_print_instanceid, .duplicate = ly_type_dup_instanceid,
        .free = ly_type_free_instanceid, .id = "libyang 2 - instance-identifier, version 1"},
    {.type = LY_TYPE_LEAFREF, .store = ly_type_store_leafref, .validate = ly_type_validate_leafref,
        .compare = ly_type_compare_leafref, .print = ly_type_print_leafref, .duplicate = ly_type_dup_leafref,
        .free = ly_type_free_leafref, .id = "libyang 2 - leafref, version 1"},
    {.type = LY_TYPE_UNION, .store = ly_type_store_union, .validate = ly_type_validate_union, .compare = ly_type_compare_union,
        .print = ly_type_print_union, .duplicate = ly_type_dup_union, .free = ly_type_free_union,
        .id = "libyang 2 - union,version 1"},
    {.type = LY_TYPE_INT8, .store = ly_type_store_int, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT16, .store = ly_type_store_int, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT32, .store = ly_type_store_int, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
    {.type = LY_TYPE_INT64, .store = ly_type_store_int, .validate = NULL, .compare = ly_type_compare_simple,
        .print = ly_type_print_simple, .duplicate = ly_type_dup_simple, .free = ly_type_free_simple,
        .id = "libyang 2 - integer, version 1"},
};
