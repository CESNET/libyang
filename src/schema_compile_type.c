/**
 * @file schema_compile_type.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema compilation of type.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* strndup */

#include "schema_compile_type.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "dict.h"
#include "log.h"
#include "ly_array.h"
#include "ly_common.h"
#include "plugins.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "schema_compile_node.h"
#include "schema_features.h"
#include "set.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "utils.h"
#include "xpath.h"

/**
 * @brief Item for storing typedef chain item.
 */
struct lys_type_item {
    const struct lysp_tpdf *tpdf;
    struct lysp_node *node;
};

/**
 * @brief Duplicate the compiled pattern structure.
 *
 * Instead of duplicating memory, the reference counter in the @p orig is increased.
 *
 * @param[in] orig The pattern structure to duplicate.
 * @return The duplicated structure to use.
 */
static struct lysc_pattern *
lysc_pattern_dup(struct lysc_pattern *orig)
{
    ++orig->refcount;
    return orig;
}

/**
 * @brief Duplicate the array of compiled patterns.
 *
 * The sized array itself is duplicated, but the pattern structures are just shadowed by increasing their reference counter.
 *
 * @param[in] ctx Libyang context for logging.
 * @param[in] orig The patterns sized array to duplicate.
 * @return New sized array as a copy of @p orig.
 * @return NULL in case of memory allocation error.
 */
static struct lysc_pattern **
lysc_patterns_dup(struct ly_ctx *ctx, struct lysc_pattern **orig)
{
    struct lysc_pattern **dup = NULL;
    LYA_COUNT_T u;

    assert(orig);

    LYA_PREALLOC(dup, LYA_COUNT(orig), LOGMEM(ctx); return NULL);
    LYA_FOR(orig, u) {
        dup[u] = lysc_pattern_dup(orig[u]);
        LYA_INCREMENT(dup);
    }

    return dup;
}

/**
 * @brief Duplicate compiled range structure.
 *
 * @param[in] ctx Compile context.
 * @param[in] orig The range structure to be duplicated.
 * @param[in] tpdf_chain Chain of the used typedefs, traversed backwards.
 * @param[in] tpdf_chain_last Index of the last (backwards) typedef in @p tpdf_chain to use.
 * @return New compiled range structure as a copy of @p orig.
 * @return NULL in case of memory allocation error.
 */
static struct lysc_range *
lysc_range_dup(struct lysc_ctx *ctx, const struct lysc_range *orig, struct ly_set *tpdf_chain, uint32_t tpdf_chain_last)
{
    struct lysc_range *dup;
    LY_ERR ret;
    struct lys_type_item *tpdf_item;
    uint32_t i;

    assert(orig);

    dup = calloc(1, sizeof *dup);
    LY_CHECK_ERR_RET(!dup, LOGMEM(ctx->ctx), NULL);
    if (orig->parts) {
        LYA_PREALLOC(dup->parts, LYA_COUNT(orig->parts), LOGMEM(ctx->ctx); ret = LY_EMEM; goto error);
        (*((LYA_COUNT_T *)(dup->parts) - 1)) = LYA_COUNT(orig->parts);
        memcpy(dup->parts, orig->parts, LYA_COUNT(dup->parts) * sizeof *dup->parts);
    }
    DUP_STRING_GOTO(ctx->ctx, orig->eapptag, dup->eapptag, ret, error);
    DUP_STRING_GOTO(ctx->ctx, orig->emsg, dup->emsg, ret, error);

    /* collect all range extensions */
    if (tpdf_chain->count > tpdf_chain_last) {
        i = tpdf_chain->count;
        do {
            --i;
            tpdf_item = tpdf_chain->objs[i];
            if (!tpdf_item->tpdf->type.range) {
                continue;
            }
            COMPILE_EXTS_GOTO(ctx, tpdf_item->tpdf->type.range->exts, dup->exts, dup, ret, error);
        } while (i > tpdf_chain_last);
    }

    return dup;

error:
    if (dup) {
        lysc_range_free(ctx->ctx, dup);
        free(dup);
    }
    return NULL;
}

/**
 * @brief Validate and normalize numeric value from a range definition.
 *
 * @param[in] ctx Compile context.
 * @param[in] basetype Base YANG built-in type of the node connected with the range restriction. Actually only LY_TYPE_DEC64 is important to
 * allow processing of the fractions. The fraction point is extracted from the value which is then normalize according to given frdigits into
 * valcopy to allow easy parsing and storing of the value. libyang stores decimal number without the decimal point which is always recovered from
 * the known fraction-digits value. So, with fraction-digits 2, number 3.14 is stored as 314 and number 1 is stored as 100.
 * @param[in] frdigits The fraction-digits of the type in case of LY_TYPE_DEC64.
 * @param[in] value String value of the range boundary.
 * @param[out] len Number of the processed bytes from the value. Processing stops on the first character which is not part of the number boundary.
 * @param[out] valcopy NULL-terminated string with the numeric value to parse and store.
 * @return LY_SUCCESS on success;
 * @return LY_EMEM on memory allocation failure;
 * @return LY_EVALID if not a number;
 * @return LY_EINVAL if decimal64 not matching fraction-digits value.
 */
static LY_ERR
range_part_check_value_syntax(struct lysc_ctx *ctx, LY_DATA_TYPE basetype, uint8_t frdigits, const char *value,
        size_t *len, char **valcopy)
{
    size_t fraction = 0, size;

    *len = 0;

    assert(value);
    /* parse value */
    if (!isdigit(value[*len]) && (value[*len] != '-') && (value[*len] != '+')) {
        return LY_EVALID;
    }

    if ((value[*len] == '-') || (value[*len] == '+')) {
        ++(*len);
    }

    while (isdigit(value[*len])) {
        ++(*len);
    }

    if ((basetype != LY_TYPE_DEC64) || (value[*len] != '.') || !isdigit(value[*len + 1])) {
        if (basetype == LY_TYPE_DEC64) {
            goto decimal;
        } else {
            *valcopy = strndup(value, *len);
            return LY_SUCCESS;
        }
    }
    fraction = *len;

    ++(*len);
    while (isdigit(value[*len])) {
        ++(*len);
    }

    if (basetype == LY_TYPE_DEC64) {
decimal:
        assert(frdigits);
        if (fraction && (*len - 1 - fraction > frdigits)) {
            LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                    "Range boundary \"%.*s\" of decimal64 type exceeds defined number (%u) of fraction digits.",
                    (int)(*len), value, frdigits);
            return LY_EINVAL;
        }
        if (fraction) {
            size = (*len) + (frdigits - ((*len) - 1 - fraction));
        } else {
            size = (*len) + frdigits + 1;
        }
        *valcopy = malloc(size * sizeof **valcopy);
        LY_CHECK_ERR_RET(!(*valcopy), LOGMEM(ctx->ctx), LY_EMEM);

        (*valcopy)[size - 1] = '\0';
        if (fraction) {
            memcpy(&(*valcopy)[0], &value[0], fraction);
            memcpy(&(*valcopy)[fraction], &value[fraction + 1], (*len) - 1 - (fraction));
            memset(&(*valcopy)[(*len) - 1], '0', frdigits - ((*len) - 1 - fraction));
        } else {
            memcpy(&(*valcopy)[0], &value[0], *len);
            memset(&(*valcopy)[*len], '0', frdigits);
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Check that values in range are in ascendant order.
 *
 * @param[in] unsigned_value Flag to note that we are working with unsigned values.
 * @param[in] max Flag to distinguish if checking min or max value. min value must be strictly higher than previous,
 * max can be also equal.
 * @param[in] value Current value to check.
 * @param[in] prev_value Last seen value.
 * @return LY_SUCCESS on success;
 * @return LY_EEXIST for invalid order.
 */
static LY_ERR
range_part_check(ly_bool unsigned_value, ly_bool max, int64_t value, int64_t prev_value)
{
    if (unsigned_value) {
        if ((max && ((uint64_t)prev_value > (uint64_t)value)) || (!max && ((uint64_t)prev_value >= (uint64_t)value))) {
            return LY_EEXIST;
        }
    } else {
        if ((max && (prev_value > value)) || (!max && (prev_value >= value))) {
            return LY_EEXIST;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Set min/max value of the range part.
 *
 * @param[in] ctx Compile context.
 * @param[in] part Range part structure to fill.
 * @param[in] max Flag to distinguish if storing min or max value.
 * @param[in] prev The last seen value to check that all values in range are specified in ascendant order.
 * @param[in] basetype Type of the value to get know implicit min/max values and other checking rules.
 * @param[in] first Flag for the first value of the range to avoid ascendancy order.
 * @param[in] length_restr Flag to distinguish between range and length restrictions. Only for logging.
 * @param[in] frdigits The fraction-digits value in case of LY_TYPE_DEC64 basetype.
 * @param[in] base_range Range from the type from which the current type is derived (if not built-in) to get type's min and max values.
 * @param[in,out] value Numeric range value to be stored, if not provided the type's min/max value is set.
 * @return LY_SUCCESS - success;
 * @return LY_EDENIED - value broken type's boundaries;
 * @return LY_EVALID - not a number;
 * @return LY_EEXIST - value is smaller than the previous one;
 * @return LY_EINVAL - decimal64 value does not corresponds with the fraction-digits value;
 * @return LY_EMEM - memory allocation failed.
 */
static LY_ERR
range_part_minmax(struct lysc_ctx *ctx, struct lysc_range_part *part, ly_bool max, int64_t prev, LY_DATA_TYPE basetype,
        ly_bool first, ly_bool length_restr, uint8_t frdigits, struct lysc_range *base_range, const char **value)
{
    LY_ERR ret = LY_SUCCESS;
    char *valcopy = NULL;
    size_t len = 0;

    if (value) {
        ret = range_part_check_value_syntax(ctx, basetype, frdigits, *value, &len, &valcopy);
        LY_CHECK_GOTO(ret, finalize);
    }
    if (!valcopy && base_range) {
        if (max) {
            part->max_64 = base_range->parts[LYA_COUNT(base_range->parts) - 1].max_64;
        } else {
            part->min_64 = base_range->parts[0].min_64;
        }
        if (!first) {
            ret = range_part_check(basetype <= LY_TYPE_STRING ? 1 : 0, max, max ? part->max_64 : part->min_64, prev);
        }
        goto finalize;
    }

    switch (basetype) {
    case LY_TYPE_INT8: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-128), INT64_C(127), LY_BASE_DEC,
                    max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(127);
        } else {
            part->min_64 = INT64_C(-128);
        }
        if (!ret && !first) {
            ret = range_part_check(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT16: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-32768), INT64_C(32767), LY_BASE_DEC,
                    max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(32767);
        } else {
            part->min_64 = INT64_C(-32768);
        }
        if (!ret && !first) {
            ret = range_part_check(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT32: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-2147483648), INT64_C(2147483647), LY_BASE_DEC,
                    max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(2147483647);
        } else {
            part->min_64 = INT64_C(-2147483648);
        }
        if (!ret && !first) {
            ret = range_part_check(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT64: /* range */
    case LY_TYPE_DEC64: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807),
                    LY_BASE_DEC, max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(9223372036854775807);
        } else {
            part->min_64 = INT64_C(-9223372036854775807) - INT64_C(1);
        }
        if (!ret && !first) {
            ret = range_part_check(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT8: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(255), LY_BASE_DEC, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(255);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT16: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(65535), LY_BASE_DEC, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(65535);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT32: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(4294967295), LY_BASE_DEC,
                    max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(4294967295);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT64: /* range */
    case LY_TYPE_STRING: /* length */
    case LY_TYPE_BINARY: /* length */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(18446744073709551615), LY_BASE_DEC,
                    max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(18446744073709551615);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    default:
        LOGINT(ctx->ctx);
        ret = LY_EINT;
    }

finalize:
    if (ret == LY_EDENIED) {
        LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                "Invalid %s restriction - value \"%s\" does not fit the type limitations.",
                length_restr ? "length" : "range", valcopy ? valcopy : *value);
    } else if (ret == LY_EVALID) {
        LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "Invalid %s restriction - invalid value \"%s\".",
                length_restr ? "length" : "range", valcopy ? valcopy : *value);
    } else if (ret == LY_EEXIST) {
        LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "Invalid %s restriction - values are not in ascending order (%s).",
                length_restr ? "length" : "range",
                (valcopy && basetype != LY_TYPE_DEC64) ? valcopy : value ? *value : max ? "max" : "min");
    } else if (!ret && value) {
        *value = *value + len;
    }
    free(valcopy);
    return ret;
}

LY_ERR
lys_compile_type_range(struct lysc_ctx *ctx, const struct lysp_restr *range_p, LY_DATA_TYPE basetype, ly_bool length_restr,
        uint8_t frdigits, struct lysc_range *base_range, struct lysc_range **range)
{
    LY_ERR ret = LY_SUCCESS;
    const char *expr;
    struct lysc_range_part *parts = NULL, *part;
    ly_bool range_expected = 0, uns;
    LYA_COUNT_T parts_done = 0, u, v;

    assert(range);
    assert(range_p);

    expr = range_p->arg.str;
    while (1) {
        if (isspace(*expr)) {
            ++expr;
        } else if (*expr == '\0') {
            if (range_expected) {
                LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected end of the expression after \"..\" (%s).",
                        length_restr ? "length" : "range", range_p->arg.str);
                ret = LY_EVALID;
                goto cleanup;
            } else if (!parts || (parts_done == LYA_COUNT(parts))) {
                LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected end of the expression (%s).",
                        length_restr ? "length" : "range", range_p->arg.str);
                ret = LY_EVALID;
                goto cleanup;
            }
            parts_done++;
            break;
        } else if (!strncmp(expr, "min", ly_strlen_const("min"))) {
            if (parts) {
                /* min cannot be used elsewhere than in the first part */
                LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected data before min keyword (%.*s).", length_restr ? "length" : "range",
                        (int)(expr - range_p->arg.str), range_p->arg.str);
                ret = LY_EVALID;
                goto cleanup;
            }
            expr += ly_strlen_const("min");

            LYA_ADD_ITEM(parts, part, LOGMEM(ctx->ctx); ret = LY_EMEM; goto cleanup);
            LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 0, 0, basetype, 1, length_restr, frdigits, base_range, NULL), cleanup);
            part->max_64 = part->min_64;
        } else if (*expr == '|') {
            if (!parts || range_expected) {
                LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected beginning of the expression (%s).",
                        length_restr ? "length" : "range", expr);
                ret = LY_EVALID;
                goto cleanup;
            }
            expr++;
            parts_done++;
            /* process next part of the expression */
        } else if (!strncmp(expr, "..", 2)) {
            expr += 2;
            while (isspace(*expr)) {
                expr++;
            }
            if (!parts || (LYA_COUNT(parts) == parts_done)) {
                LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected \"..\" without a lower bound.", length_restr ? "length" : "range");
                ret = LY_EVALID;
                goto cleanup;
            }
            /* continue expecting the upper boundary */
            range_expected = 1;
        } else if (isdigit(*expr) || (*expr == '-') || (*expr == '+')) {
            /* number */
            if (range_expected) {
                part = &parts[LYA_COUNT(parts) - 1];
                LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 1, part->min_64, basetype, 0, length_restr, frdigits, NULL, &expr), cleanup);
                range_expected = 0;
            } else {
                LYA_ADD_ITEM(parts, part, LOGMEM(ctx->ctx); ret = LY_EMEM; goto cleanup);
                LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 0, parts_done ? parts[LYA_COUNT(parts) - 2].max_64 : 0,
                        basetype, parts_done ? 0 : 1, length_restr, frdigits, NULL, &expr), cleanup);
                part->max_64 = part->min_64;
            }

            /* continue with possible another expression part */
        } else if (!strncmp(expr, "max", ly_strlen_const("max"))) {
            expr += ly_strlen_const("max");
            while (isspace(*expr)) {
                expr++;
            }
            if (*expr != '\0') {
                LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "Invalid %s restriction - unexpected data after max keyword (%s).",
                        length_restr ? "length" : "range", expr);
                ret = LY_EVALID;
                goto cleanup;
            }
            if (range_expected) {
                part = &parts[LYA_COUNT(parts) - 1];
                ret = range_part_minmax(ctx, part, 1, part->min_64, basetype, 0, length_restr, frdigits, base_range, NULL);
                LY_CHECK_GOTO(ret, cleanup);
                range_expected = 0;
            } else {
                LYA_ADD_ITEM(parts, part, LOGMEM(ctx->ctx); ret = LY_EMEM; goto cleanup);
                LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 1, parts_done ? parts[LYA_COUNT(parts) - 2].max_64 : 0,
                        basetype, parts_done ? 0 : 1, length_restr, frdigits, base_range, NULL), cleanup);
                part->min_64 = part->max_64;
            }
        } else {
            LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "Invalid %s restriction - unexpected data (%s).",
                    length_restr ? "length" : "range", expr);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    /* check with the previous range/length restriction */
    if (base_range) {
        switch (basetype) {
        case LY_TYPE_BINARY:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
        case LY_TYPE_STRING:
            uns = 1;
            break;
        case LY_TYPE_DEC64:
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
            uns = 0;
            break;
        default:
            LOGINT(ctx->ctx);
            ret = LY_EINT;
            goto cleanup;
        }
        for (u = v = 0; u < parts_done && v < LYA_COUNT(base_range->parts); ++u) {
            if ((uns && (parts[u].min_u64 < base_range->parts[v].min_u64)) ||
                    (!uns && (parts[u].min_64 < base_range->parts[v].min_64))) {
                goto baseerror;
            }
            /* current lower bound is not lower than the base */
            if (base_range->parts[v].min_64 == base_range->parts[v].max_64) {
                /* base has single value */
                if (base_range->parts[v].min_64 == parts[u].min_64) {
                    /* both lower bounds are the same */
                    if (parts[u].min_64 != parts[u].max_64) {
                        /* current continues with a range */
                        goto baseerror;
                    } else {
                        /* equal single values, move both forward */
                        ++v;
                        continue;
                    }
                } else {
                    /* base is single value lower than current range, so the
                     * value from base range is removed in the current,
                     * move only base and repeat checking */
                    ++v;
                    --u;
                    continue;
                }
            } else {
                /* base is the range */
                if (parts[u].min_64 == parts[u].max_64) {
                    /* current is a single value */
                    if ((uns && (parts[u].max_u64 > base_range->parts[v].max_u64)) || (!uns && (parts[u].max_64 > base_range->parts[v].max_64))) {
                        /* current is behind the base range, so base range is omitted,
                         * move the base and keep the current for further check */
                        ++v;
                        --u;
                    } /* else it is within the base range, so move the current, but keep the base */
                    continue;
                } else {
                    /* both are ranges - check the higher bound, the lower was already checked */
                    if ((uns && (parts[u].max_u64 > base_range->parts[v].max_u64)) ||
                            (!uns && (parts[u].max_64 > base_range->parts[v].max_64))) {
                        /* higher bound is higher than the current higher bound */
                        if ((uns && (parts[u].min_u64 > base_range->parts[v].max_u64)) ||
                                (!uns && (parts[u].min_64 > base_range->parts[v].max_64))) {
                            /* but the current lower bound is also higher, so the base range is omitted,
                             * continue with the same current, but move the base */
                            --u;
                            ++v;
                            continue;
                        }
                        /* current range starts within the base range but end behind it */
                        goto baseerror;
                    } else {
                        /* current range is smaller than the base,
                         * move current, but stay with the base */
                        continue;
                    }
                }
            }
        }
        if (u != parts_done) {
baseerror:
            LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                    "Invalid %s restriction - the derived restriction (%s) is not equally or more limiting.",
                    length_restr ? "length" : "range", range_p->arg.str);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    if (!(*range)) {
        *range = calloc(1, sizeof **range);
        LY_CHECK_ERR_RET(!(*range), LOGMEM(ctx->ctx), LY_EMEM);
    }

    /* we rewrite the following values as the types chain is being processed */
    if (range_p->eapptag) {
        lysdict_remove(ctx->ctx, (*range)->eapptag);
        LY_CHECK_GOTO(ret = lysdict_insert(ctx->ctx, range_p->eapptag, 0, &(*range)->eapptag), cleanup);
    }
    if (range_p->emsg) {
        lysdict_remove(ctx->ctx, (*range)->emsg);
        LY_CHECK_GOTO(ret = lysdict_insert(ctx->ctx, range_p->emsg, 0, &(*range)->emsg), cleanup);
    }
    if (range_p->dsc) {
        lysdict_remove(ctx->ctx, (*range)->dsc);
        LY_CHECK_GOTO(ret = lysdict_insert(ctx->ctx, range_p->dsc, 0, &(*range)->dsc), cleanup);
    }
    if (range_p->ref) {
        lysdict_remove(ctx->ctx, (*range)->ref);
        LY_CHECK_GOTO(ret = lysdict_insert(ctx->ctx, range_p->ref, 0, &(*range)->ref), cleanup);
    }
    /* extensions are taken only from the last range by the caller */

    (*range)->parts = parts;
    parts = NULL;
cleanup:
    LYA_FREE(parts);

    return ret;
}

/**
 * @brief Check whether a parsed moudle includes the 'openconfig-extensions:regexp-posix' extension.
 *
 * @param[in] pmod Parsed module to examine.
 * @return 1 if the extension has been found;
 * @return 0 otherwise.
 */
static ly_bool
lys_compile_type_patterns_has_oc_posix_ext(const struct lysp_module *pmod)
{
    const struct lysp_ext_instance *ext;
    LYA_COUNT_T u;
    const struct lys_module *mod;
    const char *tmp, *name, *prefix;
    uint32_t pref_len, name_len;

    LYA_FOR(pmod->exts, u) {
        ext = &pmod->exts[u];

        /* parse the prefix, the nodeid was previously already parsed and checked */
        tmp = ext->name;
        ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len);

        /* get the module */
        mod = ly_resolve_prefix(pmod->mod->ctx, prefix, pref_len, ext->format, ext->prefix_data);

        /* compare */
        if ((name_len != 12) || strncmp(name, "regexp-posix", name_len)) {
            continue;
        }
        if (!mod || strcmp(mod->name, "openconfig-extensions")) {
            continue;
        }

        return 1;
    }

    return 0;
}

LY_ERR
lys_compile_type_patterns(struct lysc_ctx *ctx, const struct lysp_restr *patterns_p, const struct lysp_module *type_pmod,
        struct lysc_pattern **base_patterns, struct lysc_pattern ***patterns)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_pattern **pattern;
    LYA_COUNT_T u;
    ly_bool format;

    /* first check the format of the patterns based on the type's local parsed module extensions (not compiled yet) */
    format = lys_compile_type_patterns_has_oc_posix_ext(type_pmod);

    /* copy the patterns from the base type */
    if (base_patterns) {
        *patterns = lysc_patterns_dup(ctx->ctx, base_patterns);
        LY_CHECK_ERR_RET(!(*patterns), LOGMEM(ctx->ctx), LY_EMEM);
    }

    LYA_FOR(patterns_p, u) {
        LYA_ADD_ITEM((*patterns), pattern, LOGMEM(ctx->ctx); return LY_EMEM);
        *pattern = calloc(1, sizeof **pattern);
        (*pattern)->refcount = 1;

        (*pattern)->format = format;
        if (patterns_p[u].arg.str[0] == LYSP_RESTR_PATTERN_NACK) {
            (*pattern)->inverted = 1;
        }
        LY_CHECK_GOTO(rc = lysdict_insert(ctx->ctx, &patterns_p[u].arg.str[1], 0, &(*pattern)->expr), cleanup);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].eapptag, (*pattern)->eapptag, rc, cleanup);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].emsg, (*pattern)->emsg, rc, cleanup);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].dsc, (*pattern)->dsc, rc, cleanup);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].ref, (*pattern)->ref, rc, cleanup);
        COMPILE_EXTS_GOTO(ctx, patterns_p[u].exts, (*pattern)->exts, (*pattern), rc, cleanup);

        /* compile the pattern, if it wasnt already compiled (e.g. same expression but different module)
         * we do this now to verify the pattern right away while compiling the type
         * and so we can store it in the cache so we dont have to recompile it again later
         */
        LY_CHECK_GOTO(rc = ly_ctx_shared_data_pattern_get(ctx->ctx, (*pattern)->expr, (*pattern)->format, NULL), cleanup);
    }

cleanup:
    return rc;
}

/**
 * @brief map of the possible restrictions combination for the specific built-in type.
 */
static uint16_t type_substmt_map[LY_DATA_TYPE_COUNT] = {
    0 /* LY_TYPE_UNKNOWN */,
    LYS_SET_LENGTH /* LY_TYPE_BINARY */,
    LYS_SET_RANGE /* LY_TYPE_UINT8 */,
    LYS_SET_RANGE /* LY_TYPE_UINT16 */,
    LYS_SET_RANGE /* LY_TYPE_UINT32 */,
    LYS_SET_RANGE /* LY_TYPE_UINT64 */,
    LYS_SET_LENGTH | LYS_SET_PATTERN /* LY_TYPE_STRING */,
    LYS_SET_BIT /* LY_TYPE_BITS */,
    0 /* LY_TYPE_BOOL */,
    LYS_SET_FRDIGITS | LYS_SET_RANGE /* LY_TYPE_DEC64 */,
    0 /* LY_TYPE_EMPTY */,
    LYS_SET_ENUM /* LY_TYPE_ENUM */,
    LYS_SET_BASE /* LY_TYPE_IDENT */,
    LYS_SET_REQINST /* LY_TYPE_INST */,
    LYS_SET_REQINST | LYS_SET_PATH /* LY_TYPE_LEAFREF */,
    LYS_SET_TYPE /* LY_TYPE_UNION */,
    LYS_SET_RANGE /* LY_TYPE_INT8 */,
    LYS_SET_RANGE /* LY_TYPE_INT16 */,
    LYS_SET_RANGE /* LY_TYPE_INT32 */,
    LYS_SET_RANGE /* LY_TYPE_INT64 */
};

/**
 * @brief stringification of the YANG built-in data types
 */
const char *ly_data_type2str[LY_DATA_TYPE_COUNT] = {
    LY_TYPE_UNKNOWN_STR,
    LY_TYPE_BINARY_STR,
    LY_TYPE_UINT8_STR,
    LY_TYPE_UINT16_STR,
    LY_TYPE_UINT32_STR,
    LY_TYPE_UINT64_STR,
    LY_TYPE_STRING_STR,
    LY_TYPE_BITS_STR,
    LY_TYPE_BOOL_STR,
    LY_TYPE_DEC64_STR,
    LY_TYPE_EMPTY_STR,
    LY_TYPE_ENUM_STR,
    LY_TYPE_IDENT_STR,
    LY_TYPE_INST_STR,
    LY_TYPE_LEAFREF_STR,
    LY_TYPE_UNION_STR,
    LY_TYPE_INT8_STR,
    LY_TYPE_INT16_STR,
    LY_TYPE_INT32_STR,
    LY_TYPE_INT64_STR
};

LY_ERR
lys_compile_type_enums(struct lysc_ctx *ctx, const struct lysp_type_enum *enums_p, LY_DATA_TYPE basetype,
        struct lysc_type_bitenum_item *base_enums, struct lysc_type_bitenum_item **bitenums)
{
    LY_ERR ret = LY_SUCCESS;
    LYA_COUNT_T u, v, match = 0;
    int32_t highest_value = INT32_MIN, cur_val = INT32_MIN;
    uint32_t highest_position = 0, cur_pos = 0;
    struct lysc_type_bitenum_item *e, storage;
    ly_bool enabled;

    if (base_enums && (ctx->pmod->version < LYS_VERSION_1_1)) {
        LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "%s type can be subtyped only in YANG 1.1 modules.",
                basetype == LY_TYPE_ENUM ? "Enumeration" : "Bits");
        return LY_EVALID;
    }

    LYA_FOR(enums_p, u) {
        /* perform all checks */
        if (base_enums) {
            /* check the enum/bit presence in the base type - the set of enums/bits in the derived type must be a subset */
            LYA_FOR(base_enums, v) {
                if (!strcmp(enums_p[u].name, base_enums[v].name)) {
                    break;
                }
            }
            if (v == LYA_COUNT(base_enums)) {
                LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "Invalid %s - derived type adds new item \"%s\".",
                        basetype == LY_TYPE_ENUM ? "enumeration" : "bits", enums_p[u].name);
                return LY_EVALID;
            }
            match = v;
        }

        if (basetype == LY_TYPE_ENUM) {
            if (enums_p[u].flags & LYS_SET_VALUE) {
                /* value assigned by module */
                cur_val = (int32_t)enums_p[u].value;
                /* check collision with other values */
                LYA_FOR(*bitenums, v) {
                    if (cur_val == (*bitenums)[v].value) {
                        LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                                "Invalid enumeration - value %" PRId32 " collide in items \"%s\" and \"%s\".",
                                cur_val, enums_p[u].name, (*bitenums)[v].name);
                        return LY_EVALID;
                    }
                }
            } else if (base_enums) {
                /* inherit the assigned value */
                cur_val = base_enums[match].value;
            } else {
                /* assign value automatically */
                if (u == 0) {
                    cur_val = 0;
                } else if (highest_value == INT32_MAX) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid enumeration - it is not possible to auto-assign enum value for "
                            "\"%s\" since the highest value is already 2147483647.", enums_p[u].name);
                    return LY_EVALID;
                } else {
                    cur_val = highest_value + 1;
                }
            }

            /* save highest value for auto assign */
            if (highest_value < cur_val) {
                highest_value = cur_val;
            }
        } else { /* LY_TYPE_BITS */
            if (enums_p[u].flags & LYS_SET_VALUE) {
                /* value assigned by module */
                cur_pos = (uint32_t)enums_p[u].value;
                /* check collision with other values */
                LYA_FOR(*bitenums, v) {
                    if (cur_pos == (*bitenums)[v].position) {
                        LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                                "Invalid bits - position %" PRIu32 " collide in items \"%s\" and \"%s\".",
                                cur_pos, enums_p[u].name, (*bitenums)[v].name);
                        return LY_EVALID;
                    }
                }
            } else if (base_enums) {
                /* inherit the assigned value */
                cur_pos = base_enums[match].position;
            } else {
                /* assign value automatically */
                if (u == 0) {
                    cur_pos = 0;
                } else if (highest_position == UINT32_MAX) {
                    /* counter overflow */
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid bits - it is not possible to auto-assign bit position for "
                            "\"%s\" since the highest value is already 4294967295.", enums_p[u].name);
                    return LY_EVALID;
                } else {
                    cur_pos = highest_position + 1;
                }
            }

            /* save highest position for auto assign */
            if (highest_position < cur_pos) {
                highest_position = cur_pos;
            }
        }

        /* the assigned values must not change from the derived type */
        if (base_enums) {
            if (basetype == LY_TYPE_ENUM) {
                if (cur_val != base_enums[match].value) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid enumeration - value of the item \"%s\" has changed from %" PRId32 " to %" PRId32
                            " in the derived type.", enums_p[u].name, base_enums[match].value, cur_val);
                    return LY_EVALID;
                }
            } else {
                if (cur_pos != base_enums[match].position) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid bits - position of the item \"%s\" has changed from %" PRIu32 " to %" PRIu32
                            " in the derived type.", enums_p[u].name, base_enums[match].position, cur_pos);
                    return LY_EVALID;
                }
            }
        }

        /* add new enum/bit */
        LYA_ADD_ITEM(*bitenums, e, LOGMEM(ctx->ctx); return LY_EMEM);
        DUP_STRING_GOTO(ctx->ctx, enums_p[u].name, e->name, ret, done);
        DUP_STRING_GOTO(ctx->ctx, enums_p[u].dsc, e->dsc, ret, done);
        DUP_STRING_GOTO(ctx->ctx, enums_p[u].ref, e->ref, ret, done);

        /* copy flags except for status */
        e->flags = (enums_p[u].flags & LYS_FLAGS_COMPILED_MASK) & ~LYS_STATUS_MASK;

        /* compile status */
        LY_CHECK_RET(lys_compile_status(ctx, enums_p[u].flags, 0, 0, NULL, (basetype == LY_TYPE_ENUM) ? "enum" : "bit",
                &e->flags));

        /* enum/bit flag */
        e->flags |= (basetype == LY_TYPE_ENUM) ? LYS_IS_ENUM : 0;

        if (basetype == LY_TYPE_ENUM) {
            e->value = cur_val;
        } else {
            e->position = cur_pos;
        }
        COMPILE_EXTS_GOTO(ctx, enums_p[u].exts, e->exts, e, ret, done);

        /* evaluate if-ffeatures */
        LY_CHECK_RET(lys_eval_iffeatures(ctx->ctx, enums_p[u].iffeatures, &enabled));
        if (!enabled) {
            /* set only flag, later resolved and removed */
            e->flags |= LYS_DISABLED;
        }

        if (basetype == LY_TYPE_BITS) {
            /* keep bits ordered by position */
            for (v = u; v && (*bitenums)[v - 1].position > e->position; --v) {}
            if (v != u) {
                memcpy(&storage, e, sizeof *e);
                memmove(&(*bitenums)[v + 1], &(*bitenums)[v], (u - v) * sizeof **bitenums);
                memcpy(&(*bitenums)[v], &storage, sizeof storage);
            }
        }
    }

    /* revalidate the backward parent pointers from extensions now that the array is final */
    if (basetype == LY_TYPE_ENUM) {
        LYSP_ARRAY_EXT_PARENTS_UPDATE(*bitenums, exts, LY_STMT_ENUM);
        LYSP_ARRAY_EXT_PARENTS_UPDATE(*bitenums, exts, LY_STMT_VALUE);
    } else {
        LYSP_ARRAY_EXT_PARENTS_UPDATE(*bitenums, exts, LY_STMT_BIT);
        LYSP_ARRAY_EXT_PARENTS_UPDATE(*bitenums, exts, LY_STMT_POSITION);
    }

done:
    return ret;
}

/**
 * @brief Compile union type.
 *
 * @param[in] ctx Compile context.
 * @param[in] ptypes Parsed union types.
 * @param[in] context_pnode Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef to correctly check status
 * of referencing and referenced objects.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in,out] utypes_p Array of compiled union types.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_type_union(struct lysc_ctx *ctx, struct lysp_type *ptypes, struct lysp_node *context_pnode, uint16_t context_flags,
        const char *context_name, struct lysc_type ***utypes_p)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_type **utypes = *utypes_p;
    struct lysc_type_union *un_aux = NULL;
    LYA_COUNT_T u, v, additional;
    ly_bool free_types = 0;

    LYA_PREALLOC(utypes, LYA_COUNT(ptypes), LOGMEM(ctx->ctx); rc = LY_EMEM; goto cleanup);

    additional = 0;
    LYA_FOR(ptypes, u) {
        rc = lys_compile_type(ctx, context_pnode, context_flags, context_name, &ptypes[u], &utypes[u + additional],
                NULL, NULL);
        LY_CHECK_GOTO(rc, cleanup);
        if (!utypes[u + additional]) {
            /* foreign typedef in local-only module */
            free_types = 1;
            goto cleanup;
        }
        LY_ATOMIC_INC_BARRIER(utypes[u + additional]->refcount);

        if (utypes[u + additional]->basetype == LY_TYPE_UNION) {
            /* add space for additional types from the union subtype */
            un_aux = (struct lysc_type_union *)utypes[u + additional];
            LYA_PREALLOC(utypes, LYA_COUNT(ptypes) + additional + LYA_COUNT(un_aux->types), LOGMEM(ctx->ctx);
                    rc = LY_EMEM; goto cleanup);

            /* copy subtypes of the subtype union */
            LYA_FOR(un_aux->types, v) {
                utypes[u + additional] = un_aux->types[v];
                LY_ATOMIC_INC_BARRIER(un_aux->types[v]->refcount);
                ++additional;
                LYA_INCREMENT(utypes);
            }

            /* compensate u increment in main loop */
            assert(additional);
            --additional;

            /* free the replaced union subtype */
            lysc_type_free(ctx->ctx, (struct lysc_type *)un_aux);
            un_aux = NULL;
        } else {
            LYA_INCREMENT(utypes);
        }
    }

cleanup:
    if (un_aux) {
        lysc_type_free(ctx->ctx, (struct lysc_type *)un_aux);
    }
    if (!rc && !free_types) {
        *utypes_p = utypes;
    } else {
        LYA_FOR(utypes, u) {
            lysc_type_free(ctx->ctx, utypes[u]);
        }
        LYA_FREE(utypes);
    }
    return rc;
}

/**
 * @brief Allocate a new specific type structure according to the basetype.
 *
 * @param[in] ctx Context to use.
 * @param[in] basetype Base type of the new type.
 * @param[in] tpdf_name Optional referenced typedef name, NULL for built-in types.
 * @param[out] type Specific type structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_new_type(const struct ly_ctx *ctx, LY_DATA_TYPE basetype, const char *tpdf_name, struct lysc_type **type)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_type *t = NULL;

    *type = NULL;

    switch (basetype) {
    case LY_TYPE_BINARY:
        t = calloc(1, sizeof(struct lysc_type_bin));
        break;
    case LY_TYPE_BITS:
        t = calloc(1, sizeof(struct lysc_type_bits));
        break;
    case LY_TYPE_DEC64:
        t = calloc(1, sizeof(struct lysc_type_dec));
        break;
    case LY_TYPE_STRING:
        t = calloc(1, sizeof(struct lysc_type_str));
        break;
    case LY_TYPE_ENUM:
        t = calloc(1, sizeof(struct lysc_type_enum));
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        t = calloc(1, sizeof(struct lysc_type_num));
        break;
    case LY_TYPE_IDENT:
        t = calloc(1, sizeof(struct lysc_type_identityref));
        break;
    case LY_TYPE_LEAFREF:
        t = calloc(1, sizeof(struct lysc_type_leafref));
        break;
    case LY_TYPE_INST:
        t = calloc(1, sizeof(struct lysc_type_instanceid));
        break;
    case LY_TYPE_UNION:
        t = calloc(1, sizeof(struct lysc_type_union));
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        t = calloc(1, sizeof(struct lysc_type));
        break;
    case LY_TYPE_UNKNOWN:
        break;
    }
    LY_CHECK_ERR_GOTO(!t, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    if (tpdf_name) {
        rc = lysdict_insert(ctx, tpdf_name, 0, &t->name);
        LY_CHECK_GOTO(rc, cleanup);
    }

cleanup:
    if (rc) {
        free(t);
    } else {
        *type = t;
    }
    return rc;
}

/**
 * @brief Check that a compiled type can be reused and shared.
 *
 * @param[in] type_p Parsed type.
 * @param[in] type_c Compiled type.
 * @return 1 if @p type_c can be shared;
 * @return 0 otherwise.
 */
static ly_bool
lys_compile_type_share_compiled(const struct lysp_type *type_p, const struct lysc_type *type_c)
{
    LYA_COUNT_T u;
    struct lysc_type_union *type_un;

    if (!type_c) {
        /* no compiled type to share */
        return 0;
    }

    if (type_p && (type_p->flags || type_p->exts)) {
        /* special type flags or extensions to be compiled */
        return 0;
    }

    /* learn whether the type has a leafref, in which case it cannot be shared because it may resolve to a different
     * real type for every instantiation */
    if (type_c->basetype == LY_TYPE_LEAFREF) {
        /* leafref type */
        return 0;
    } else if (type_c->basetype == LY_TYPE_UNION) {
        /* union with a leafref */
        type_un = (struct lysc_type_union *)type_c;
        LYA_FOR(type_un->types, u) {
            if (type_un->types[u]->basetype == LY_TYPE_LEAFREF) {
                return 0;
            }
        }
    }

    return 1;
}

/**
 * @brief Find a specific parsed type in the typedef chain.
 *
 * @param[in] tpdf_chain Set with the typedef chain.
 * @param[in] tpdf_chain_last Index of @p tpdf_chain not to cross in descending order.
 * @param[in] with_enum Set if looking for the first type with enums.
 * @param[in] with_bits Set if looking for the first type with bits.
 * @return Found parsed type.
 */
static const struct lysp_type *
lys_compile_type_find_parsed(const struct ly_set *tpdf_chain, uint32_t tpdf_chain_last, ly_bool with_enum, ly_bool with_bits)
{
    const struct lysp_type *parsed = NULL;
    const struct lys_type_item *tpdf_item;
    uint32_t i;

    assert(tpdf_chain->count > tpdf_chain_last);
    assert((with_enum && !with_bits) || (!with_enum && with_bits));

    i = tpdf_chain->count;
    do {
        --i;
        tpdf_item = tpdf_chain->objs[i];

        if (with_enum && tpdf_item->tpdf->type.enums) {
            /* enums */
            parsed = &tpdf_item->tpdf->type;
        } else if (with_bits && tpdf_item->tpdf->type.bits) {
            /* bits */
            parsed = &tpdf_item->tpdf->type;
        }
    } while (!parsed && (i > tpdf_chain_last));

    assert(parsed);
    return parsed;
}

/**
 * @brief The core of the lys_compile_type() - compile information about the given type (from typedef or leaf/leaf-list).
 *
 * @param[in] ctx Compile context.
 * @param[in] context_pnode Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef to correctly check status of
 * referencing and referenced objects.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in] type_p Parsed type to compile.
 * @param[in] basetype Base YANG built-in type of the type to compile.
 * @param[in] tpdfname Name of the type's typedef, serves as a flag - if it is leaf/leaf-list's type, it is NULL.
 * @param[in] base Latest base (compiled) type from which the current type is being derived.
 * @param[in] base_p Parsed type of the @p base type.
 * @param[in] plugin_ref Reference to the type plugin to use.
 * @param[in] tpdf_chain Chain of the used typedefs, traversed backwards.
 * @param[in] tpdf_chain_last Index of the last (backwards) typedef in @p tpdf_chain to use.
 * @param[out] type Compiled type.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_type_(struct lysc_ctx *ctx, struct lysp_node *context_pnode, uint16_t context_flags, const char *context_name,
        const struct lysp_type *type_p, LY_DATA_TYPE basetype, const char *tpdfname, const struct lysc_type *base,
        const struct lysp_type *base_p, uintptr_t plugin_ref, struct ly_set *tpdf_chain, uint32_t tpdf_chain_last,
        struct lysc_type **type)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_type_bin *bin;
    struct lysc_type_num *num;
    struct lysc_type_str *str;
    struct lysc_type_bits *bits;
    struct lysc_type_enum *enumeration;
    struct lysc_type_dec *dec;
    struct lysc_type_identityref *idref;
    struct lysc_type_leafref *lref;
    struct lysc_type_instanceid *instid;
    struct lysc_type_union *un;
    struct lys_type_item *tpdf_item;
    const struct lysp_type *base_type_p;
    struct lysc_prefix *prefixes;
    uint32_t i;
    LYA_COUNT_T u;
    ly_bool share_base;

    /* alloc and init */
    rc = lys_new_type(ctx->ctx, basetype, tpdfname, type);
    LY_CHECK_GOTO(rc, cleanup);

    (*type)->basetype = basetype;
    (*type)->plugin_ref = plugin_ref;

    switch (basetype) {
    case LY_TYPE_BINARY:
        bin = (struct lysc_type_bin *)*type;

        /* RFC 7950 9.8.1, 9.4.4 - length, number of octets it contains */
        if (type_p->length) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->length, basetype, 1, 0,
                    base ? ((struct lysc_type_bin *)base)->length : NULL, &bin->length), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->length->exts, bin->length->exts, bin->length, rc, cleanup);
            }
        } else if (base && ((struct lysc_type_bin *)base)->length) {
            bin->length = lysc_range_dup(ctx, ((struct lysc_type_bin *)base)->length, tpdf_chain, tpdf_chain_last);
        }
        break;
    case LY_TYPE_BITS:
        bits = (struct lysc_type_bits *)*type;

        /* RFC 7950 9.7 - bits */
        if (type_p->bits) {
            /* compile bits from this type */
            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, type_p->bits, basetype,
                    base ? (struct lysc_type_bitenum_item *)((struct lysc_type_bits *)base)->bits : NULL,
                    (struct lysc_type_bitenum_item **)&bits->bits), cleanup);
        } else if (base) {
            /* recompile bits from the first superior type with bits */
            base_type_p = lys_compile_type_find_parsed(tpdf_chain, tpdf_chain_last, 0, 1);
            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, base_type_p->bits, basetype, NULL,
                    (struct lysc_type_bitenum_item **)&bits->bits), cleanup);
        } else {
            /* type derived from bits built-in type must contain at least one bit */
            if (tpdfname) {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "bit", "bits type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "bit", "bits type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_DEC64:
        dec = (struct lysc_type_dec *)*type;

        /* RFC 7950 9.3.4 - fraction-digits */
        if (!base) {
            if (!type_p->fraction_digits) {
                if (tpdfname) {
                    LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "fraction-digits", "decimal64 type ", tpdfname);
                } else {
                    LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "fraction-digits", "decimal64 type", "");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            dec->fraction_digits = type_p->fraction_digits;
        } else {
            if (type_p->fraction_digits) {
                /* fraction digits is prohibited in types not directly derived from built-in decimal64 */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid fraction-digits substatement for type \"%s\" not directly derived from decimal64 built-in type.",
                            tpdfname);
                } else {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid fraction-digits substatement for type not directly derived from decimal64 built-in type.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            dec->fraction_digits = ((struct lysc_type_dec *)base)->fraction_digits;
        }

        /* RFC 7950 9.2.4 - range */
        if (type_p->range) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->range, basetype, 0, dec->fraction_digits,
                    base ? ((struct lysc_type_dec *)base)->range : NULL, &dec->range), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->range->exts, dec->range->exts, dec->range, rc, cleanup);
            }
        } else if (base && ((struct lysc_type_dec *)base)->range) {
            dec->range = lysc_range_dup(ctx, ((struct lysc_type_dec *)base)->range, tpdf_chain, tpdf_chain_last);
        }
        break;
    case LY_TYPE_STRING:
        str = (struct lysc_type_str *)*type;

        /* RFC 7950 9.4.4 - length */
        if (type_p->length) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->length, basetype, 1, 0,
                    base ? ((struct lysc_type_str *)base)->length : NULL, &str->length), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->length->exts, str->length->exts, str->length, rc, cleanup);
            }
        } else if (base && ((struct lysc_type_str *)base)->length) {
            str->length = lysc_range_dup(ctx, ((struct lysc_type_str *)base)->length, tpdf_chain, tpdf_chain_last);
        }

        /* RFC 7950 9.4.5 - pattern */
        if (type_p->patterns) {
            LY_CHECK_GOTO(rc = lys_compile_type_patterns(ctx, type_p->patterns, type_p->pmod,
                    base ? ((struct lysc_type_str *)base)->patterns : NULL, &str->patterns), cleanup);
        } else if (base && ((struct lysc_type_str *)base)->patterns) {
            str->patterns = lysc_patterns_dup(ctx->ctx, ((struct lysc_type_str *)base)->patterns);
        }
        break;
    case LY_TYPE_ENUM:
        enumeration = (struct lysc_type_enum *)*type;

        /* RFC 7950 9.6 - enum */
        if (type_p->enums) {
            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, type_p->enums, basetype,
                    base ? ((struct lysc_type_enum *)base)->enums : NULL, &enumeration->enums), cleanup);
        } else if (base) {
            /* recompile enums from the first superior type with enums */
            base_type_p = lys_compile_type_find_parsed(tpdf_chain, tpdf_chain_last, 1, 0);
            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, base_type_p->enums, basetype, NULL, &enumeration->enums), cleanup);
        } else {
            /* type derived from enumerations built-in type must contain at least one enum */
            if (tpdfname) {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "enum", "enumeration type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "enum", "enumeration type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        num = (struct lysc_type_num *)*type;

        /* RFC 6020 9.2.4 - range */
        if (type_p->range) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->range, basetype, 0, 0,
                    base ? ((struct lysc_type_num *)base)->range : NULL, &num->range), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->range->exts, num->range->exts, num->range, rc, cleanup);
            }
        } else if (base && ((struct lysc_type_num *)base)->range) {
            num->range = lysc_range_dup(ctx, ((struct lysc_type_num *)base)->range, tpdf_chain, tpdf_chain_last);
        }
        break;
    case LY_TYPE_IDENT:
        idref = (struct lysc_type_identityref *)*type;

        /* RFC 7950 9.10.2 - base */
        if (type_p->bases) {
            if (base) {
                /* only the directly derived identityrefs can contain base specification */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid base substatement for the type \"%s\" not directly derived from identityref built-in type.",
                            tpdfname);
                } else {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid base substatement for the type not directly derived from identityref built-in type.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            LY_CHECK_GOTO(rc = lys_compile_identity_bases(ctx, type_p->pmod, type_p->bases, NULL, &idref->bases), cleanup);
        } else if (base) {
            /* copy all the bases */
            const struct lysc_type_identityref *idref_base = (struct lysc_type_identityref *)base;
            LYA_COUNT_T u;

            LYA_PREALLOC(idref->bases, LYA_COUNT(idref_base->bases), LOGMEM(ctx->ctx); rc = LY_EMEM; goto cleanup);
            LYA_FOR(idref_base->bases, u) {
                idref->bases[u] = idref_base->bases[u];
                LYA_INCREMENT(idref->bases);
            }
        } else {
            /* type derived from identityref built-in type must contain at least one base */
            if (tpdfname) {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "base", "identityref type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "base", "identityref type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_LEAFREF:
        lref = (struct lysc_type_leafref *)*type;

        /* RFC 7950 9.9.3 - require-instance */
        if (type_p->flags & LYS_SET_REQINST) {
            if (type_p->pmod->version < LYS_VERSION_1_1) {
                if (tpdfname) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                            "Leafref type \"%s\" can be restricted by require-instance statement only in YANG 1.1 modules.", tpdfname);
                } else {
                    LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS,
                            "Leafref type can be restricted by require-instance statement only in YANG 1.1 modules.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            lref->require_instance = type_p->require_instance;
        } else if (base) {
            /* inherit */
            lref->require_instance = ((struct lysc_type_leafref *)base)->require_instance;
        } else {
            /* default is true */
            lref->require_instance = 1;
        }

        /* RFC 7950 9.9.2 - path */
        if (type_p->path) {
            LY_VALUE_FORMAT format;

            LY_CHECK_GOTO(rc = lyxp_expr_dup(ctx->ctx, type_p->path, 0, 0, &lref->path), cleanup);
            LY_CHECK_GOTO(lyplg_type_prefix_data_new(ctx->ctx, type_p->path->expr, strlen(type_p->path->expr),
                    LY_VALUE_SCHEMA, type_p->pmod, &format, (void **)&lref->prefixes), cleanup);

            /* non-prefixed nodes in path are supposed to be from the module where the leafref type is instantiated */
            assert(format == LY_VALUE_SCHEMA_RESOLVED);
            prefixes = lref->prefixes;
            assert(!prefixes[0].prefix);
            prefixes[0].mod = ctx->cur_mod;
        } else if (base) {
            /* make a copy */
            LY_CHECK_GOTO(rc = lyxp_expr_dup(ctx->ctx, ((struct lysc_type_leafref *)base)->path, 0, 0, &lref->path), cleanup);
            LY_CHECK_GOTO(rc = lyplg_type_prefix_data_dup(ctx->ctx, LY_VALUE_SCHEMA_RESOLVED,
                    ((struct lysc_type_leafref *)base)->prefixes, (void **)&lref->prefixes), cleanup);
        } else {
            /* type derived from leafref built-in type must contain path */
            if (tpdfname) {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "path", "leafref type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "path", "leafref type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_INST:
        instid = (struct lysc_type_instanceid *)*type;

        /* RFC 7950 9.9.3 - require-instance */
        if (type_p->flags & LYS_SET_REQINST) {
            instid->require_instance = type_p->require_instance;
        } else if (base) {
            /* inherit */
            instid->require_instance = ((struct lysc_type_instanceid *)base)->require_instance;
        } else {
            /* default is true */
            instid->require_instance = 1;
        }
        break;
    case LY_TYPE_UNION:
        un = (struct lysc_type_union *)*type;

        /* RFC 7950 7.4 - type */
        if (type_p->types) {
            if (base) {
                /* only the directly derived union can contain types specification */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid type substatement for the type \"%s\" not directly derived from union built-in type.",
                            tpdfname);
                } else {
                    LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG,
                            "Invalid type substatement for the type not directly derived from union built-in type.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            /* compile the type */
            LY_CHECK_GOTO(rc = lys_compile_type_union(ctx, type_p->types, context_pnode, context_flags, context_name,
                    &un->types), cleanup);
            if (!un->types) {
                /* foreign typedef in local-only module */
                lydict_remove(ctx->ctx, (*type)->name);
                free(*type);
                *type = NULL;
                goto cleanup;
            }
        } else if (base) {
            /* copy all the types if possible */
            const struct lysc_type_union *un_base = (struct lysc_type_union *)base;

            share_base = 1;
            LYA_FOR(un_base->types, u) {
                if (un_base->types[u]->exts || !lys_compile_type_share_compiled(type_p, un_base->types[u])) {
                    /* cannot be shared */
                    share_base = 0;
                    break;
                }
            }

            if (share_base) {
                LYA_PREALLOC(un->types, LYA_COUNT(un_base->types), LOGMEM(ctx->ctx); rc = LY_EMEM; goto cleanup);
                LYA_FOR(un_base->types, u) {
                    un->types[u] = un_base->types[u];
                    LY_ATOMIC_INC_BARRIER(un->types[u]->refcount);
                    LYA_INCREMENT(un->types);
                }
            } else {
                /* free the prepared type */
                lydict_remove(ctx->ctx, (*type)->name);
                free(*type);
                *type = NULL;

                /* recompile the union */
                rc = lys_compile_type(ctx, context_pnode, context_flags, context_name, base_p, type, NULL, NULL);
                LY_CHECK_GOTO(rc, cleanup);
                assert(*type);
            }
        } else {
            /* type derived from union built-in type must contain at least one type */
            if (tpdfname) {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "type", "union type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, NULL, LY_VCODE_MISSCHILDSTMT, "type", "union type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        /* nothing to do */
        break;
    case LY_TYPE_UNKNOWN:
        LOGINT(ctx->ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    if (tpdf_chain->count > tpdf_chain_last) {
        i = tpdf_chain->count;
        do {
            --i;
            tpdf_item = tpdf_chain->objs[i];

            /* compile previous typedefs extensions */
            COMPILE_EXTS_GOTO(ctx, tpdf_item->tpdf->type.exts, (*type)->exts, *type, rc, cleanup);
        } while (i > tpdf_chain_last);
    }

    /* compile new parsed extensions */
    COMPILE_EXTS_GOTO(ctx, type_p->exts, (*type)->exts, *type, rc, cleanup);

cleanup:
    if (rc) {
        LY_ATOMIC_INC_BARRIER((*type)->refcount);
        lysc_type_free(ctx->ctx, *type);
        *type = NULL;
    }
    return rc;
}

LY_ERR
lys_compile_type_find_tpdf(struct lysc_ctx *ctx, const struct lysp_node *context_pnode, uint16_t context_flags,
        const char *context_name, const struct lysp_type *type_p, const char **units, struct lysp_qname **dflt,
        struct ly_set *tpdf_chain, LY_DATA_TYPE *basetype)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool dummyloops = 0;
    struct lys_type_item *tctx, *tctx_prev = NULL, *tctx_iter;
    uint32_t i;

    assert(!tpdf_chain->count);

    if (units) {
        *units = NULL;
    }
    if (dflt) {
        *dflt = NULL;
    }
    *basetype = LY_TYPE_UNKNOWN;

    tctx = calloc(1, sizeof *tctx);
    LY_CHECK_ERR_RET(!tctx, LOGMEM(ctx->ctx), LY_EMEM);

    for (ret = lysp_type_find(type_p->name, context_pnode, type_p->pmod, ctx->ext, basetype, &tctx->tpdf, &tctx->node);
            ret == LY_SUCCESS;
            ret = lysp_type_find(tctx_prev->tpdf->type.name, tctx_prev->node, tctx_prev->tpdf->type.pmod, ctx->ext,
                    basetype, &tctx->tpdf, &tctx->node)) {
        if (*basetype) {
            break;
        }

        /* check status */
        ret = lysc_check_status(ctx, NULL, context_flags, (void *)type_p->pmod, context_name, tctx->tpdf->flags,
                (void *)tctx->tpdf->type.pmod, tctx->node ? tctx->node->name : tctx->tpdf->name);
        LY_CHECK_GOTO(ret, cleanup);

        if (units && !*units) {
            /* inherit units */
            DUP_STRING(ctx->ctx, tctx->tpdf->units, *units, ret);
            LY_CHECK_GOTO(ret, cleanup);
        }
        if (dflt && !*dflt && tctx->tpdf->dflt.str) {
            /* inherit default */
            *dflt = (struct lysp_qname *)&tctx->tpdf->dflt;
        }
        if (dummyloops && (!units || *units) && dflt && *dflt) {
            *basetype = ((struct lys_type_item *)tpdf_chain->objs[tpdf_chain->count - 1])->tpdf->type.compiled->basetype;
            break;
        }

        if (tctx->tpdf->type.compiled && (tctx->tpdf->type.compiled->refcount == 1)) {
            /* context recompilation - everything was freed previously (the only reference is from the parsed type itself)
             * and we need now recompile the type again in the updated context. */
            lysc_type_free(ctx->ctx, tctx->tpdf->type.compiled);
            ((struct lysp_tpdf *)tctx->tpdf)->type.compiled = NULL;
        }

        if (tctx->tpdf->type.compiled) {
            /* it is not necessary to continue, the rest of the chain was already compiled,
             * but we still may need to inherit default and units values, so start dummy loops */
            *basetype = tctx->tpdf->type.compiled->basetype;
            ret = ly_set_add(tpdf_chain, tctx, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);

            if ((units && !*units) || (dflt && !*dflt)) {
                dummyloops = 1;
                goto next_iter;
            } else {
                tctx = NULL;
                break;
            }
        }

        /* circular typedef reference detection */
        for (i = 0; i < tpdf_chain->count; ++i) {
            /* local part */
            tctx_iter = tpdf_chain->objs[i];
            if (tctx_iter->tpdf == tctx->tpdf) {
                LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Invalid \"%s\" type reference - circular chain of types detected.",
                        tctx->tpdf->name);
                ret = LY_EVALID;
                goto cleanup;
            }
        }
        for (i = 0; i < ctx->tpdf_chain.count; ++i) {
            /* global part for unions corner case */
            tctx_iter = (struct lys_type_item *)ctx->tpdf_chain.objs[i];
            if (tctx_iter->tpdf == tctx->tpdf) {
                LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Invalid \"%s\" type reference - circular chain of types detected.",
                        tctx->tpdf->name);
                ret = LY_EVALID;
                goto cleanup;
            }
        }

        /* store information for the following processing */
        ret = ly_set_add(tpdf_chain, tctx, 1, NULL);
        LY_CHECK_GOTO(ret, cleanup);

next_iter:
        /* prepare next loop */
        tctx_prev = tctx;
        tctx = calloc(1, sizeof *tctx);
        LY_CHECK_ERR_RET(!tctx, LOGMEM(ctx->ctx), LY_EMEM);
    }

cleanup:
    free(tctx);
    return ret;
}

LY_ERR
lys_compile_type(struct lysc_ctx *ctx, struct lysp_node *context_pnode, uint16_t context_flags, const char *context_name,
        const struct lysp_type *type_p, struct lysc_type **type, const char **units, struct lysp_qname **dflt)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_type_item *tctx;
    LY_DATA_TYPE basetype = LY_TYPE_UNKNOWN;
    struct lysc_type *base = NULL;
    const struct lysp_type *base_p = NULL;
    uint32_t i;
    struct ly_set tpdf_chain = {0};
    uintptr_t plugin_ref = 0;

    *type = NULL;

    /* collect the typedef chain and learn the basetype */
    ret = lys_compile_type_find_tpdf(ctx, context_pnode, context_flags, context_name, type_p, units, dflt, &tpdf_chain,
            &basetype);
    if (ret == LY_ENOTFOUND) {
        tctx = tpdf_chain.count ? tpdf_chain.objs[tpdf_chain.count - 1] : NULL;
        LOGVAL(ctx->ctx, NULL, LYVE_REFERENCE, "Referenced type \"%s\" not found.",
                tctx ? tctx->tpdf->type.name : type_p->name);
        ret = LY_EVALID;
        goto cleanup;
    } else if (ret) {
        goto cleanup;
    }

    /* basic checks */
    if (~type_substmt_map[basetype] & type_p->flags) {
        LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "Invalid type restrictions for %s type.", ly_data_type2str[basetype]);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* get restrictions from the referred typedefs */
    for (i = tpdf_chain.count - 1; i + 1 > 0; --i) {
        tctx = tpdf_chain.objs[i];

        /* remember the typedef context for circular check */
        ret = ly_set_add(&ctx->tpdf_chain, tctx, 1, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        if (tctx->tpdf->type.compiled) {
            /* already compiled, reuse */
            base = tctx->tpdf->type.compiled;
            base_p = &tctx->tpdf->type;
            continue;
        }

        /* try to find loaded user type plugins */
        plugin_ref = lyplg_type_plugin_find(ctx->ctx, tctx->tpdf->type.pmod->mod->name, tctx->tpdf->type.pmod->mod->revision,
                tctx->tpdf->name);
        if (!plugin_ref && base) {
            plugin_ref = base->plugin_ref;
        }
        if (!plugin_ref) {
            /* use the internal built-in type implementation */
            plugin_ref = lyplg_type_plugin_find(ctx->ctx, "", NULL, ly_data_type2str[basetype]);
        }
        assert(plugin_ref);

        if (lys_compile_type_share_compiled(&tctx->tpdf->type, base) && (i != tpdf_chain.count - 1) &&
                (plugin_ref == base->plugin_ref)) {
            /* no change, reuse the compiled base */
            ((struct lysp_tpdf *)tctx->tpdf)->type.compiled = base;
            LY_ATOMIC_INC_BARRIER(base->refcount);
            continue;
        }

        if (~type_substmt_map[basetype] & tctx->tpdf->type.flags) {
            LOGVAL(ctx->ctx, NULL, LYVE_SYNTAX_YANG, "Invalid type \"%s\" restriction(s) for %s type.",
                    tctx->tpdf->name, ly_data_type2str[basetype]);
            ret = LY_EVALID;
            goto cleanup;
        } else if ((basetype == LY_TYPE_EMPTY) && tctx->tpdf->dflt.str) {
            LOGVAL(ctx->ctx, NULL, LYVE_SEMANTICS, "Invalid type \"%s\" - \"empty\" type must not have a default value (%s).",
                    tctx->tpdf->name, tctx->tpdf->dflt.str);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* compile the typedef type */
        ret = lys_compile_type_(ctx, tctx->node, tctx->tpdf->flags, tctx->tpdf->name, &tctx->tpdf->type, basetype,
                tctx->tpdf->name, base, base_p, plugin_ref, &tpdf_chain, i + 1, &base);
        LY_CHECK_GOTO(ret, cleanup);
        base_p = &tctx->tpdf->type;

        /* store separately compiled typedef type to be reused */
        ((struct lysp_tpdf *)tctx->tpdf)->type.compiled = base;
        LY_ATOMIC_INC_BARRIER(base->refcount);
    }

    /* remove the processed typedef contexts from the stack for circular check */
    ctx->tpdf_chain.count = ctx->tpdf_chain.count - tpdf_chain.count;

    if ((ctx->compile_opts & LYS_COMPILE_LOCAL_ONLY) && tpdf_chain.count &&
            (((struct lys_type_item *)tpdf_chain.objs[tpdf_chain.count - 1])->tpdf->type.pmod->mod != ctx->pmod->mod)) {
        /* last typedef is foreign, do not resolve */
        goto cleanup;
    }

    /* process the type definition in leaf */
    if (!lys_compile_type_share_compiled(type_p, base)) {
        /* leaf type has changes that need to be compiled into the type */
        if (base) {
            plugin_ref = base->plugin_ref;
        } else {
            plugin_ref = lyplg_type_plugin_find(ctx->ctx, "", NULL, ly_data_type2str[basetype]);
        }
        ret = lys_compile_type_(ctx, context_pnode, context_flags, context_name, (struct lysp_type *)type_p, basetype,
                NULL, base, base_p, plugin_ref, &tpdf_chain, 0, type);
        LY_CHECK_GOTO(ret, cleanup);
        if (!*type) {
            /* foreign typedef in local-only module */
            goto cleanup;
        }

        /* always fill the type name */
        if (tpdf_chain.count) {
            ret = lysdict_insert(ctx->ctx, ((struct lys_type_item *)tpdf_chain.objs[0])->tpdf->name, 0, &(*type)->name);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        /* no changes of the type in the leaf, just use the base compiled type */
        *type = base;
    }

cleanup:
    ly_set_erase(&tpdf_chain, free);
    return ret;
}
