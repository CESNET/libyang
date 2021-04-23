/**
 * @file ip_address.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ip-address type plugin.
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

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/**
 * @brief Canonize a valid IPv6 address.
 *
 * @param[in] ipv6_addr IPv6 address to canonize.
 * @param[out] canonical Canonical format of @p ipv6_addr.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
canonize_ipv6_addr(const char *ipv6_addr, char **canonical, struct ly_err_item **err)
{
    char buf[sizeof(struct in6_addr)], *str;

    *canonical = NULL;
    *err = NULL;

    str = malloc(INET6_ADDRSTRLEN);
    if (!str) {
        return LY_EMEM;
    }

    if (!inet_pton(AF_INET6, ipv6_addr, buf)) {
        free(str);
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address \"%s\".", ipv6_addr);
    }

    if (!inet_ntop(AF_INET6, buf, str, INET6_ADDRSTRLEN)) {
        free(str);
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address (%s).", strerror(errno));
    }

    *canonical = str;
    return LY_SUCCESS;
}

/**
 * @brief Validate, canonize and store value of the ietf-inet-types ip(v4/v6)-address type.
 * Implementation of the ::lyplg_type_store_clb.
 */
static LY_ERR
lyplg_type_store_ip_address(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *ptr;
    char *ipv6_addr, *result, *tmp;

    /* store as a string */
    ret = lyplg_type_store_string(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node,
            storage, unres, err);
    LY_CHECK_RET(ret);

    if (strchr(storage->canonical, ':')) {
        /* canonize IPv6 address */
        if ((ptr = strchr(storage->canonical, '%'))) {
            /* there is a zone index */
            ipv6_addr = strndup(storage->canonical, ptr - storage->canonical);
        } else {
            ipv6_addr = (char *)storage->canonical;
        }

        /* convert to canonical format */
        ret = canonize_ipv6_addr(ipv6_addr, &result, err);
        if (ptr) {
            free(ipv6_addr);
        }
        LY_CHECK_GOTO(ret, cleanup);

        if (strncmp(storage->canonical, result, strlen(result))) {
            /* some conversion took place */
            if (ptr) {
                /* concatenate the zone, if any */
                tmp = result;
                if (asprintf(&result, "%s%s", tmp, ptr) == -1) {
                    free(tmp);
                    ret = LY_EMEM;
                    goto cleanup;
                }
                free(tmp);
            }

            /* update the value */
            lydict_remove(ctx, storage->canonical);
            storage->canonical = NULL;
            LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, result, &storage->canonical), cleanup);
        } else {
            free(result);
        }
    }

cleanup:
    if (ret) {
        type->plugin->free(ctx, storage);
    }
    return ret;
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
        .plugin.store = lyplg_type_store_ip_address,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv6-address-no-zone",

        .plugin.id = "libyang 2 - ipv6-address-no-zone, version 1",
        .plugin.store = lyplg_type_store_ip_address,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
