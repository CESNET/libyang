/**
 * @file ipv6_address.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ipv6-address type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */
#include <sys/cdefs.h>

#include "plugins_types.h"

#include <arpa/inet.h>
#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesIPv6Address ietf-inet-types LYB ipv6-address(-no-zone)
 *
 * | Size (B) | Mandatory | Meaning |
 * | :------  | :-------: | :-----: |
 * | 16       | yes       | IPv6 address in network-byte order |
 * | variable | no        | IPv6 address zone string |
 */

/**
 * @brief Stored value structure for ipv6-address
 */
struct lyd_value_ipv6_address {
    struct in6_addr addr;
    const char *zone;
};

/**
 * @brief Free an ipv6-address value.
 * Implementation of ::lyplg_type_free_clb.
 */
static void
lyplg_type_free_ipv6_address(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_ipv6_address *val = value->ptr;

    lydict_remove(ctx, value->_canonical);
    if (val) {
        lydict_remove(ctx, val->zone);
        free(val);
    }
}

/**
 * @brief Validate and store value of the ietf-inet-types ipv6-address type.
 * Implementation of ::lyplg_type_store_clb.
 */
static LY_ERR
lyplg_type_store_ipv6_address(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *addr_no_zone, *value_str = value;
    char *zone_ptr = NULL, *addr_dyn = NULL;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    struct lyd_value_ipv6_address *val;
    size_t i, zone_len;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (!strcmp(type->plugin->id, "libyang 2 - ipv6-address-no-zone, version 1")) {
            if (value_len != 16) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-address-no-zone value size %zu "
                        "(expected 16).", value_len);
                goto cleanup;
            }
        } else {
            if (value_len < 16) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-address value size %zu "
                        "(expected at least 16).", value_len);
                goto cleanup;
            }
            for (i = 16; i < value_len; ++i) {
                if (!isalnum(value_str[i])) {
                    ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-address zone character 0x%x.",
                            value_str[i]);
                    goto cleanup;
                }
            }
        }

        /* allocate the value */
        val = malloc(sizeof *val);
        LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);

        /* init storage */
        storage->_canonical = NULL;
        storage->ptr = val;
        storage->realtype = type;

        /* store IP address */
        memcpy(&val->addr, value, sizeof val->addr);

        /* store zone, if any */
        if (value_len > 16) {
            ret = lydict_insert(ctx, value + 16, value_len - 16, &val->zone);
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            val->zone = NULL;
        }

        /* success */
        goto cleanup;
    }

    /* zero storage so we can always free it */
    memset(storage, 0, sizeof *storage);

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* length restriction of the string */
    if (type_str->length) {
        char buf[LY_NUMBER_MAXLEN];
        size_t char_count = ly_utf8len(value, value_len);

        /* value_len is in bytes, but we need number of characters here */
        snprintf(buf, LY_NUMBER_MAXLEN, "%zu", char_count);
        ret = lyplg_type_validate_range(LY_TYPE_STRING, type_str->length, char_count, buf, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* pattern restrictions */
    ret = lyplg_type_validate_patterns(type_str->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate the value */
    val = calloc(1, sizeof *val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);

    /* init storage */
    storage->_canonical = NULL;
    storage->ptr = val;
    storage->realtype = type;

    /* store zone and get the string IPv6 address without it */
    if ((zone_ptr = ly_strnchr(value_str, '%', value_len))) {
        /* there is a zone index */
        zone_len = value_len - (zone_ptr - value_str) - 1;
        ret = lydict_insert(ctx, zone_ptr + 1, zone_len, &val->zone);
        LY_CHECK_GOTO(ret, cleanup);

        /* get the IP without it */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            *zone_ptr = '\0';
            addr_no_zone = value_str;
        } else {
            addr_dyn = strndup(value_str, zone_ptr - value_str);
            addr_no_zone = addr_dyn;
        }
    } else {
        /* no zone */
        val->zone = NULL;

        /* get the IP terminated with zero */
        if (value_str[value_len] != '\0') {
            assert(!(options & LYPLG_TYPE_STORE_DYNAMIC));
            addr_dyn = strndup(value_str, value_len);
            addr_no_zone = addr_dyn;
        } else {
            /* we can use the value directly */
            addr_no_zone = value_str;
        }
    }

    /* store the IPv6 address in network-byte order */
    if (!inet_pton(AF_INET6, addr_no_zone, &val->addr)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address \"%s\".", addr_no_zone);
        goto cleanup;
    }

    /* restore the value */
    if ((options & LYPLG_TYPE_STORE_DYNAMIC) && zone_ptr) {
        *zone_ptr = '%';
    }

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    free(addr_dyn);
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_ipv6_address(ctx, storage);
    }
    return ret;
}

/**
 * @brief Compare 2 values of the ietf-inet-types ipv6-address type.
 * Implementation of ::lyplg_type_compare_clb.
 */
static LY_ERR
lyplg_type_compare_ipv6_address(const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_ipv6_address *v1 = val1->ptr, *v2 = val2->ptr;

    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    /* zones are NULL or in the dictionary */
    if (memcmp(&v1->addr, &v2->addr, sizeof v1->addr) || (v1->zone != v2->zone)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

/**
 * @brief Print a value of the ietf-inet-types ipv6-address type.
 * Implementation of ::lyplg_type_print_clb.
 */
static const void *
lyplg_type_print_ipv6_address(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_ipv6_address *val = value->ptr;
    size_t zone_len;
    char *ret;

    if (format == LY_VALUE_LYB) {
        /* binary format is not using cache and will always be dynamic */
        zone_len = val->zone ? strlen(val->zone) : 0;
        ret = malloc(sizeof val->addr + zone_len);
        LY_CHECK_RET(!ret, NULL);

        memcpy(ret, &val->addr, sizeof val->addr);
        if (zone_len) {
            memcpy(ret + sizeof val->addr, val->zone, zone_len);
        }
        *dynamic = 1;
        if (value_len) {
            *value_len = sizeof val->addr + zone_len;
        }
        return ret;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        /* '%' + zone */
        zone_len = val->zone ? strlen(val->zone) + 1 : 0;
        ret = malloc(INET6_ADDRSTRLEN + zone_len);
        LY_CHECK_RET(!ret, NULL);

        /* get the address in string */
        if (!inet_ntop(AF_INET6, &val->addr, ret, INET6_ADDRSTRLEN)) {
            free(ret);
            LOGERR(ctx, LY_EVALID, "Failed to get IPv6 address in string (%s).", strerror(errno));
            return NULL;
        }

        /* add zone */
        if (zone_len) {
            sprintf(ret + strlen(ret), "%%%s", val->zone);
        }

        /* store it */
        if (lydict_insert_zc(ctx, ret, (const char **)&value->_canonical)) {
            LOGMEM(ctx);
            return NULL;
        }
    }

    /* use the cached canonical value */
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len) {
        *value_len = strlen(value->_canonical);
    }
    return value->_canonical;
}

/**
 * @brief Get hash key of a value of the ietf-inet-types ipv6-address type.
 * Implementation of ::lyplg_type_hash_clb.
 */
static const void *
lyplg_type_hash_ipv6_address(const struct lyd_value *value, ly_bool *dynamic, size_t *key_len)
{
    struct lyd_value_ipv6_address *val = value->ptr;

    if (!val->zone) {
        /* we can use the IP directly */
        *dynamic = 0;
        *key_len = sizeof val->addr;
        return &val->addr;
    }

    /* simply use the (dynamic) LYB value */
    return lyplg_type_print_ipv6_address(NULL, value, LY_VALUE_LYB, NULL, dynamic, key_len);
}

/**
 * @brief Duplicate a value of the ietf-inet-types ipv6-address type.
 * Implementation of ::lyplg_type_dup_clb.
 */
static LY_ERR
lyplg_type_dup_ipv6_address(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_ipv6_address *orig_val = original->ptr, *dup_val;

    ret = lydict_insert(ctx, original->_canonical, ly_strlen(original->_canonical), &dup->_canonical);
    LY_CHECK_RET(ret);

    dup_val = malloc(sizeof *dup_val);
    if (!dup_val) {
        lydict_remove(ctx, dup->_canonical);
        return LY_EMEM;
    }
    memcpy(&dup_val->addr, &orig_val->addr, sizeof orig_val->addr);
    ret = lydict_insert(ctx, orig_val->zone, 0, &dup_val->zone);
    if (ret) {
        lydict_remove(ctx, dup->_canonical);
        free(dup_val);
        return ret;
    }

    dup->ptr = dup_val;
    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

/**
 * @brief Plugin information for ip-address type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_ip_address[] = {
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv6-address",

        .plugin.id = "libyang 2 - ipv6-address, version 1",
        .plugin.store = lyplg_type_store_ipv6_address,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_ipv6_address,
        .plugin.print = lyplg_type_print_ipv6_address,
        .plugin.hash = lyplg_type_hash_ipv6_address,
        .plugin.duplicate = lyplg_type_dup_ipv6_address,
        .plugin.free = lyplg_type_free_ipv6_address
    },
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv6-address-no-zone",

        .plugin.id = "libyang 2 - ipv6-address-no-zone, version 1",
        .plugin.store = lyplg_type_store_ipv6_address,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_ipv6_address,
        .plugin.print = lyplg_type_print_ipv6_address,
        .plugin.hash = lyplg_type_hash_ipv6_address,
        .plugin.duplicate = lyplg_type_dup_ipv6_address,
        .plugin.free = lyplg_type_free_ipv6_address
    },
    {0}
};
