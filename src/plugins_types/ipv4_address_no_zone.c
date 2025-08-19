/**
 * @file ipv4_address_no_zone.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ipv4-address-no-zone type plugin.
 *
 * Copyright (c) 2019 - 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* strndup */

#include "plugins_internal.h"
#include "plugins_types.h"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
#    include <netinet/in.h>
#    include <sys/socket.h>
#  endif
#endif
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "compat.h"
#include "ly_common.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesIPv4AddressNoZone ipv4-address-no-zone (ietf-inet-types)
 *
 * | Size (b) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 32       | yes       | `struct in_addr *` | IPv4 address in network-byte order |
 */

static void
lyplg_type_lyb_size_ipv4_address_no_zone(const struct lysc_type *UNUSED(type), enum lyplg_lyb_size_type *size_type,
        uint32_t *fixed_size_bits)
{
    *size_type = LYPLG_LYB_SIZE_FIXED_BITS;
    *fixed_size_bits = 32;
}

/**
 * @brief Implementation of ::lyplg_type_store_clb for the ipv4-address-no-zone ietf-inet-types type.
 */
static LY_ERR
lyplg_type_store_ipv4_address_no_zone(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        uint32_t value_size_bits, uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), const struct lysc_ext_instance *UNUSED(top_ext),
        struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres), struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_ipv4_address_no_zone *val;
    uint32_t value_size;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    /* check value length */
    ret = lyplg_type_check_value_size("ipv4-address-no-zone", format, value_size_bits, LYPLG_LYB_SIZE_FIXED_BITS, 32,
            &value_size, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (format == LY_VALUE_LYB) {
        /* store IP address */
        memcpy(&val->addr, value, 4);

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_size, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* we always need a dynamic value */
    if (!(options & LYPLG_TYPE_STORE_DYNAMIC)) {
        value = strndup(value, value_size);
        LY_CHECK_ERR_GOTO(!value, ret = LY_EMEM, cleanup);

        options |= LYPLG_TYPE_STORE_DYNAMIC;
    }

    /* get the network-byte order address */
    if (!inet_pton(AF_INET, value, &val->addr)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to store IPv4 address \"%s\".", (char *)value);
        goto cleanup;
    }

    /* store canonical value */
    ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
    options &= ~LYPLG_TYPE_STORE_DYNAMIC;
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the ipv4-address-no-zone ietf-inet-types type.
 */
static LY_ERR
lyplg_type_compare_ipv4_address_no_zone(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    struct lyd_value_ipv4_address_no_zone *v1, *v2;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    if (memcmp(&v1->addr, &v2->addr, sizeof v1->addr)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

/**
 * @brief Implementation of ::lyplg_type_sort_clb for the ipv4-address-no-zone ietf-inet-types type.
 */
static int
lyplg_type_sort_ipv4_address_no_zone(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    struct lyd_value_ipv4_address_no_zone *v1, *v2;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    return memcmp(&v1->addr, &v2->addr, sizeof v1->addr);
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for the ipv4-address-no-zone ietf-inet-types type.
 */
static const void *
lyplg_type_print_ipv4_address_no_zone(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, uint32_t *value_size_bits)
{
    struct lyd_value_ipv4_address_no_zone *val;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_size_bits) {
            *value_size_bits = 32;
        }
        return &val->addr;
    }

    /* generate canonical value if not already (loaded from LYB) */
    if (!value->_canonical) {
        ret = malloc(INET_ADDRSTRLEN);
        LY_CHECK_RET(!ret, NULL);

        /* get the address in string */
        if (!inet_ntop(AF_INET, &val->addr, ret, INET_ADDRSTRLEN)) {
            free(ret);
            LOGERR(ctx, LY_ESYS, "Failed to get IPv4 address in string (%s).", strerror(errno));
            return NULL;
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
    if (value_size_bits) {
        *value_size_bits = strlen(value->_canonical) * 8;
    }
    return value->_canonical;
}

/**
 * @brief Plugin information for ipv4-address-no-zone type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_ipv4_address_no_zone[] = {
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv4-address-no-zone",

        .plugin.id = "ly2 ipv4-address-no-zone",
        .plugin.lyb_size = lyplg_type_lyb_size_ipv4_address_no_zone,
        .plugin.store = lyplg_type_store_ipv4_address_no_zone,
        .plugin.validate_value = NULL,
        .plugin.validate_tree = NULL,
        .plugin.compare = lyplg_type_compare_ipv4_address_no_zone,
        .plugin.sort = lyplg_type_sort_ipv4_address_no_zone,
        .plugin.print = lyplg_type_print_ipv4_address_no_zone,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
    },
    {0}
};
