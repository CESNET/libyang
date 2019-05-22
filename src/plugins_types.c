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

API LY_ERR
parse_int(const char *datatype, int base, int64_t min, int64_t max, const char *value, size_t value_len, int64_t *ret, struct ly_err_item **err)
{
    char *errmsg = NULL;
    char *strptr = NULL;
    int64_t i;

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);
    if (!value || !value[0] || !value_len) {
        asprintf(&errmsg, "Invalid empty %s value.", datatype);
        goto error;
    }

    /* convert to 64-bit integer, all the redundant characters are handled */
    errno = 0;

    /* parse the value */
    i = strtoll(value, &strptr, base);
    if (errno || (i < min) || (i > max)) {
        goto error;
    } else if (strptr && *strptr && strptr < value + value_len) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr && strptr < value + value_len) {
            goto error;
        }
    }

    if (ret) {
        *ret = i;
    }
    return LY_SUCCESS;

error:
    if (!errmsg) {
        asprintf(&errmsg, "Invalid %s value %.*s.", datatype, (int)value_len, value);
    }
    *err = ly_err_new(LY_LLERR, LY_EINVAL, LYVE_RESTRICTION, errmsg, NULL, NULL);
    return LY_EVALID;;
}

API LY_ERR
parse_uint(const char *datatype, int base, uint64_t min, uint64_t max, const char *value, size_t value_len, uint64_t *ret, struct ly_err_item **err)
{
    char *errmsg = NULL;
    char *strptr = NULL;
    uint64_t u;

    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);
    if (!value || !value[0] || !value_len) {
        asprintf(&errmsg, "Invalid empty %s value.", datatype);
        goto error;
    }

    *err = NULL;
    errno = 0;
    u = strtoull(value, &strptr, base);
    if (errno || (u < min) || (u > max)) {
        goto error;
    } else if (strptr && *strptr && strptr < value + value_len) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr && strptr < value + value_len) {
            goto error;
        }
    } else if (u != 0 && value[0] == '-') {
        goto error;
    }

    if (ret) {
        *ret = u;
    }
    return LY_SUCCESS;

error:
    if (!errmsg) {
        asprintf(&errmsg, "Invalid %s value %.*s.", datatype, (int)value_len, value);
    }
    *err = ly_err_new(LY_LLERR, LY_EINVAL, LYVE_RESTRICTION, errmsg, NULL, NULL);
    return LY_EVALID;;
}

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
                           (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "Length" : "Value", (uint64_t)value,
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
                    asprintf(&errmsg, "%s \"%"PRIu64"\" does not satisfy the %s constraint.",
                           (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? "Length" : "Value", (uint64_t)value,
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

static LY_ERR
ly_type_parse_int_builtin(LY_DATA_TYPE basetype, const char *value, size_t value_len, int options, int64_t *val, struct ly_err_item **err)
{
    switch (basetype) {
    case LY_TYPE_INT8:
        return parse_int("int16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, INT64_C(-128), INT64_C(127), value, value_len, val, err);
    case LY_TYPE_INT16:
        return parse_int("int16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, INT64_C(-32768), INT64_C(32767), value, value_len, val, err);
    case LY_TYPE_INT32:
        return parse_int("int32", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10,
                         INT64_C(-2147483648), INT64_C(2147483647), value, value_len, val, err);
    case LY_TYPE_INT64:
        return parse_int("int64", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10,
                         INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807), value, value_len, val, err);
    default:
        LOGINT(NULL);
        return LY_EINVAL;
    }
}
/**
 * @brief Validate and canonize value of the YANG built-in signed integer types.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_int(struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                     const char **canonized, struct ly_err_item **err, void **priv)
{
    int64_t i;
    struct lysc_type_num *type_num = (struct lysc_type_num *)type;

    LY_CHECK_RET(ly_type_parse_int_builtin(type->basetype, value, value_len, options, &i, err));

    /* range of the number */
    if (type_num->range) {
        LY_CHECK_RET(ly_type_validate_range(type->basetype, type_num->range, i, err));
    }

    if (options & LY_TYPE_OPTS_CANONIZE) {
        char *str;
        asprintf(&str, "%"PRId64, i);
        *canonized = lydict_insert_zc(ctx, str);
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }
    if (options & LY_TYPE_OPTS_STORE) {
        /* save for the store callback */
        *priv = malloc(sizeof i);
        *(int64_t*)(*priv) = i;
    }

    return LY_SUCCESS;
}

/**
 * @brief Store value of the YANG built-in signed integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_int(struct ly_ctx *UNUSED(ctx), struct lysc_type *type, int options,
                  struct lyd_value *value, struct ly_err_item **err, void **priv)
{
    int64_t i;

    if (options & LY_TYPE_OPTS_VALIDATE) {
        /* the value was prepared by ly_type_validate_int() */
        i = *(int64_t*)(*priv);
        free(*priv);
    } else {
        LY_CHECK_RET(ly_type_parse_int_builtin(type->basetype, value->canonized, strlen(value->canonized), options, &i, err));
    }

    /* store the result */
    value->int64 = i;

    return LY_SUCCESS;
}

static LY_ERR
ly_type_parse_uint_builtin(LY_DATA_TYPE basetype, const char *value, size_t value_len, int options, uint64_t *val, struct ly_err_item **err)
{
    switch (basetype) {
    case LY_TYPE_UINT8:
        return parse_uint("uint16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, 0, UINT64_C(255), value, value_len, val, err);
    case LY_TYPE_UINT16:
        return parse_uint("uint16", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, 0, UINT64_C(65535), value, value_len, val, err);
    case LY_TYPE_UINT32:
        return parse_uint("uint32", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, 0, UINT64_C(4294967295), value, value_len, val, err);
    case LY_TYPE_UINT64:
        return parse_uint("uint64", (options & LY_TYPE_OPTS_SCHEMA) ? 0 : 10, 0, UINT64_C(18446744073709551615), value, value_len, val, err);
    default:
        LOGINT(NULL);
        return LY_EINVAL;
    }
}
/**
 * @brief Validate and canonize value of the YANG built-in unsigned integer types.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_uint(struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                      const char **canonized, struct ly_err_item **err, void **priv)
{
    uint64_t u;
    struct lysc_type_num* type_num = (struct lysc_type_num*)type;

    LY_CHECK_RET(ly_type_parse_uint_builtin(type->basetype, value, value_len, options, &u, err));

    /* range of the number */
    if (type_num->range) {
        LY_CHECK_RET(ly_type_validate_range(type->basetype, type_num->range, u, err));
    }

    if (options & LY_TYPE_OPTS_CANONIZE) {
        char *str;
        asprintf(&str, "%"PRIu64, u);
        *canonized = lydict_insert_zc(ctx, str);
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }
    if (options & LY_TYPE_OPTS_STORE) {
        /* save for the store callback */
        *priv = malloc(sizeof u);
        *(uint64_t*)(*priv) = u;
    }

    return LY_SUCCESS;
}

/**
 * @brief Store value of the YANG built-in unsigned integer types.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_uint(struct ly_ctx *UNUSED(ctx), struct lysc_type *type, int options,
                   struct lyd_value *value, struct ly_err_item **err, void **priv)
{
    uint64_t u;

    if (options & LY_TYPE_OPTS_VALIDATE) {
        /* the value was prepared by ly_type_validate_uint() */
        u = *(uint64_t*)(*priv);
        free(*priv);
    } else {
        LY_CHECK_RET(ly_type_parse_uint_builtin(type->basetype, value->canonized, strlen(value->canonized), options, &u, err));
    }

    /* store the result */
    value->uint64 = u;

    return LY_SUCCESS;
}

/**
 * @brief Validate and canonize value of the YANG built-in binary type.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_binary(struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                        const char **canonized, struct ly_err_item **err, void **UNUSED(priv))
{
    size_t start = 0, stop = 0, count = 0, u, termination = 0;
    struct lysc_type_bin *type_bin = (struct lysc_type_bin *)type;
    char *errmsg;

    /* initiate */
    *err = NULL;

    /* validate characters and remember the number of octets for length validation */
    if (value && value_len) {
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
        uint64_t len = ((count / 4) * 3) - termination;
        LY_CHECK_RET(ly_type_validate_range(LY_TYPE_BINARY, type_bin->length, len, err));
    }

    if (options & LY_TYPE_OPTS_CANONIZE) {
        if (start != 0 || stop != value_len) {
            *canonized = lydict_insert_zc(ctx, strndup(&value[start], stop + 1 - start));
        } else if (options & LY_TYPE_OPTS_DYNAMIC) {
            *canonized = lydict_insert_zc(ctx, (char*)value);
            value = NULL;
        } else {
            *canonized = lydict_insert(ctx, value_len ? value : "", value_len);
        }
    }
    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    return LY_SUCCESS;

error:
    if (!*err) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_RESTRICTION, errmsg, NULL, NULL);
    }
    return (*err)->no;
}

/**
 * @brief Validate and canonize value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_validate_clb.
 */
static LY_ERR
ly_type_validate_bits(struct ly_ctx *ctx, struct lysc_type *type, const char *value, size_t value_len, int options,
                       const char **canonized, struct ly_err_item **err, void **priv)
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
    size_t ws_count = 0;
    size_t lws_count = 0; /* leading whitespace count */

    /* remember the present items for further work */
    items = ly_set_new();
    LY_CHECK_RET(!items, LY_EMEM);

    for (index = 0; index < value_len; index++) {
        if (isspace(value[index])) {
            if (iscanonical && (ws_count || !index || value[index] != ' ')) {
                /* this whitespace breaks canonical format of the value */
                iscanonical = 0;
            }
            ws_count++;
            continue;
        }
        if (index == ws_count) {
            lws_count = ws_count;
        }
        ws_count = 0;

        /* start of the item */
        item = &value[index];
        for (item_len = 0; index + item_len < value_len && !isspace(item[item_len]); item_len++);
        LY_ARRAY_FOR(type_bits->bits, u) {
            if (!strncmp(type_bits->bits[u].name, item, item_len) && type_bits->bits[u].name[item_len] == '\0') {
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

    if (options & LY_TYPE_OPTS_CANONIZE) {
        if (options & LY_TYPE_OPTS_STORE) {
            if (iscanonical) {
                items_ordered = items;
                items = NULL;
            } else {
                items_ordered = ly_set_dup(items, NULL);
                LY_CHECK_ERR_GOTO(!items_ordered, LOGMEM(ctx); ret = LY_EMEM, error);
                items_ordered->count = 0;
            }
        }
        if (iscanonical) {
            if (!ws_count && !lws_count && (options & LY_TYPE_OPTS_DYNAMIC)) {
                *canonized = lydict_insert_zc(ctx, (char*)value);
                value = NULL;
            } else {
                *canonized = lydict_insert(ctx, value_len ? &value[lws_count] : "", value_len - ws_count - lws_count);
            }
        } else {
            buf = malloc(buf_size * sizeof *buf);
            LY_CHECK_ERR_GOTO(!buf, LOGMEM(ctx); ret = LY_EMEM, error);
            index = 0;

            /* generate ordered bits list */
            LY_ARRAY_FOR(type_bits->bits, u) {
                int i = ly_set_contains(items, &type_bits->bits[u]);
                if (i != -1) {
                    int c = sprintf(&buf[index], "%s%s", index ? " " : "", type_bits->bits[u].name);
                    LY_CHECK_ERR_GOTO(c < 0, LOGERR(ctx, LY_ESYS, "sprintf() failed."); ret = LY_ESYS, error);
                    index += c;
                    if (items_ordered) {
                        ly_set_add(items_ordered, &type_bits->bits[u], LY_SET_OPT_USEASLIST);
                    }
                }
            }
            assert(buf_size == index + 1);
            /* termination NULL-byte */
            buf[index] = '\0';

            *canonized = lydict_insert_zc(ctx, buf);
            buf = NULL;
        }
    }

    if (options & LY_TYPE_OPTS_STORE) {
        /* remember the set to store */
        if (items_ordered) {
            *priv = items_ordered;
        } else {
            *priv = items;
            items = NULL;
        }
    }

    if (options & LY_TYPE_OPTS_DYNAMIC) {
        free((char*)value);
    }

    ly_set_free(items, NULL);
    return LY_SUCCESS;
error:
    if (errmsg) {
        *err = ly_err_new(LY_LLERR, LY_EVALID, LYVE_RESTRICTION, errmsg, NULL, NULL);
    }
    ly_set_free(items, NULL);
    ly_set_free(items_ordered, NULL);
    free(buf);
    return ret;
}

/**
 * @brief Store value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_store_clb.
 */
static LY_ERR
ly_type_store_bits(struct ly_ctx *ctx, struct lysc_type *UNUSED(type), int options,
                   struct lyd_value *value, struct ly_err_item **UNUSED(err), void **priv)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_set *items = NULL;
    unsigned int u;

    if (options & LY_TYPE_OPTS_VALIDATE) {
        /* the value was prepared by ly_type_validate_bits() */
        items = (struct ly_set *)(*priv);
        LY_ARRAY_CREATE_GOTO(ctx, value->bits_items, items->count, ret, cleanup);
        for (u = 0; u < items->count; u++) {
            value->bits_items[u] = items->objs[u];
            LY_ARRAY_INCREMENT(value->bits_items);
        }
    } else {
        /* TODO if there is usecase for store without validate */
    }

cleanup:
    ly_set_free(items, NULL);
    return ret;
}

/**
 * @brief Free value of the YANG built-in bits type.
 *
 * Implementation of the ly_type_free_clb.
 */
static void
ly_type_free_bits(struct ly_ctx *UNUSED(ctx), struct lysc_type *UNUSED(type), struct lyd_value *value)
{
    LY_ARRAY_FREE(value->bits_items);
    value->bits_items = NULL;
}

struct lysc_type_plugin ly_builtin_type_plugins[LY_DATA_TYPE_COUNT] = {
    {0}, /* LY_TYPE_UNKNOWN */
    {.type = LY_TYPE_BINARY, .validate = ly_type_validate_binary, .store = NULL, .free = NULL},
    {.type = LY_TYPE_UINT8, .validate = ly_type_validate_uint, .store = ly_type_store_uint, .free = NULL},
    {.type = LY_TYPE_UINT16, .validate = ly_type_validate_uint, .store = ly_type_store_uint, .free = NULL},
    {.type = LY_TYPE_UINT32, .validate = ly_type_validate_uint, .store = ly_type_store_uint, .free = NULL},
    {.type = LY_TYPE_UINT64, .validate = ly_type_validate_uint, .store = ly_type_store_uint, .free = NULL},
    {0}, /* TODO LY_TYPE_STRING */
    {.type = LY_TYPE_BITS, .validate = ly_type_validate_bits, .store = ly_type_store_bits, .free = ly_type_free_bits},
    {0}, /* TODO LY_TYPE_BOOL */
    {0}, /* TODO LY_TYPE_DEC64 */
    {0}, /* TODO LY_TYPE_EMPTY */
    {0}, /* TODO LY_TYPE_ENUM */
    {0}, /* TODO LY_TYPE_IDENT */
    {0}, /* TODO LY_TYPE_INST */
    {0}, /* TODO LY_TYPE_LEAFREF */
    {0}, /* TODO LY_TYPE_UNION */
    {.type = LY_TYPE_INT8, .validate = ly_type_validate_int, .store = ly_type_store_int, .free = NULL},
    {.type = LY_TYPE_INT16, .validate = ly_type_validate_int, .store = ly_type_store_int, .free = NULL},
    {.type = LY_TYPE_INT32, .validate = ly_type_validate_int, .store = ly_type_store_int, .free = NULL},
    {.type = LY_TYPE_INT64, .validate = ly_type_validate_int, .store = ly_type_store_int, .free = NULL},
};

