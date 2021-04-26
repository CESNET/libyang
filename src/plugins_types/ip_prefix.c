/**
 * @file ip_prefix.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ip-prefix type plugin.
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
 * @brief Canonize an ipv4-prefix value.
 *
 * @param[in] ipv4_prefix Prefix to canonize.
 * @param[out] canonical Canonical format of @p ipv4_prefix.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
canonize_ipv4_prefix(const char *ipv4_prefix, char **canonical, struct ly_err_item **err)
{
    LY_ERR ret;
    const char *pref_str;
    char *ptr, *result = NULL;
    uint32_t pref, addr_bin, i, mask;

    *canonical = NULL;
    *err = NULL;

    pref_str = strchr(ipv4_prefix, '/');
    if (!pref_str) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid IPv4 prefix \"%s\".", ipv4_prefix);
        goto error;
    }

    /* learn prefix */
    pref = strtoul(pref_str + 1, &ptr, 10);
    if (ptr[0] || (pref > 32)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid IPv4 prefix \"%s\".", ipv4_prefix);
        goto error;
    }

    result = malloc(INET_ADDRSTRLEN + 3);
    if (!result) {
        ret = LY_EMEM;
        goto error;
    }

    /* copy just the network prefix */
    strncpy(result, ipv4_prefix, pref_str - ipv4_prefix);
    result[pref_str - ipv4_prefix] = '\0';

    /* convert it to binary form */
    if (inet_pton(AF_INET, result, (void *)&addr_bin) != 1) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv4 address \"%s\".", result);
        goto error;
    }

    /* zero host bits */
    mask = 0;
    for (i = 0; i < 32; ++i) {
        mask <<= 1;
        if (pref > i) {
            mask |= 1;
        }
    }
    mask = htonl(mask);
    addr_bin &= mask;

    /* convert back to string */
    if (!inet_ntop(AF_INET, (void *)&addr_bin, result, INET_ADDRSTRLEN)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv4 address (%s).", strerror(errno));
        goto error;
    }

    /* add the prefix */
    strcat(result, pref_str);

    *canonical = result;
    return LY_SUCCESS;

error:
    free(result);
    return ret;
}

/**
 * @brief Canonize an ipv6-prefix value.
 *
 * @param[in] ipv6_prefix Prefix to canonize.
 * @param[out] canonical Canonical format of @p ipv6_prefix.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
canonize_ipv6_prefix(const char *ipv4_prefix, char **canonical, struct ly_err_item **err)
{
    LY_ERR ret;
    const char *pref_str;
    char *ptr, *result = NULL;
    unsigned long int pref, i, j;

    union {
        struct in6_addr s;
        uint32_t a[4];
    } addr_bin;
    uint32_t mask;

    *canonical = NULL;
    *err = NULL;

    pref_str = strchr(ipv4_prefix, '/');
    if (!pref_str) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid IPv6 prefix \"%s\".", ipv4_prefix);
        goto error;
    }

    /* learn prefix */
    pref = strtoul(pref_str + 1, &ptr, 10);
    if (ptr[0] || (pref > 128)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid IPv6 prefix \"%s\".", ipv4_prefix);
        goto error;
    }

    result = malloc(INET6_ADDRSTRLEN + 4);
    if (!result) {
        ret = LY_EMEM;
        goto error;
    }

    /* copy just the network prefix */
    strncpy(result, ipv4_prefix, pref_str - ipv4_prefix);
    result[pref_str - ipv4_prefix] = '\0';

    /* convert it to binary form */
    if (inet_pton(AF_INET6, result, (void *)&addr_bin.s) != 1) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address \"%s\".", result);
        goto error;
    }

    /* zero host bits */
    for (i = 0; i < 4; ++i) {
        mask = 0;
        for (j = 0; j < 32; ++j) {
            mask <<= 1;
            if (pref > (i * 32) + j) {
                mask |= 1;
            }
        }
        mask = htonl(mask);
        addr_bin.a[i] &= mask;
    }

    /* convert back to string */
    if (!inet_ntop(AF_INET6, (void *)&addr_bin.s, result, INET6_ADDRSTRLEN)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address (%s).", strerror(errno));
        goto error;
    }

    /* add the prefix */
    strcat(result, pref_str);

    *canonical = result;
    return LY_SUCCESS;

error:
    free(result);
    return ret;
}

/**
 * @brief Validate, canonize and store value of the ietf-inet-types ipv4-prefix type.
 * Implementation of the ::lyplg_type_store_clb.
 */
static LY_ERR
lyplg_type_store_ipv4_prefix(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    char *canonical;

    /* store as a string */
    ret = lyplg_type_store_string(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node,
            storage, unres, err);
    LY_CHECK_RET(ret);

    /* canonize */
    ret = canonize_ipv4_prefix(storage->_canonical, &canonical, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (strcmp(canonical, storage->_canonical)) {
        /* some conversion took place, update the value */
        lydict_remove(ctx, storage->_canonical);
        storage->_canonical = NULL;
        LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, canonical, &storage->_canonical), cleanup);
    } else {
        free(canonical);
    }

cleanup:
    if (ret) {
        type->plugin->free(ctx, storage);
    }
    return ret;
}

/**
 * @brief Validate, canonize and store value of the ietf-inet-types ipv6-prefix type.
 * Implementation of the ::lyplg_type_store_clb.
 */
static LY_ERR
lyplg_type_store_ipv6_prefix(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    char *canonical;

    /* store as a string */
    ret = lyplg_type_store_string(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node,
            storage, unres, err);
    LY_CHECK_RET(ret);

    /* canonize */
    ret = canonize_ipv6_prefix(storage->_canonical, &canonical, err);
    LY_CHECK_GOTO(ret, cleanup);

    if (strcmp(canonical, storage->_canonical)) {
        /* some conversion took place, update the value */
        lydict_remove(ctx, storage->_canonical);
        storage->_canonical = NULL;
        LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, canonical, &storage->_canonical), cleanup);
    } else {
        free(canonical);
    }

cleanup:
    if (ret) {
        type->plugin->free(ctx, storage);
    }
    return ret;
}

/**
 * @brief Plugin information for ip-prefix type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_ip_prefix[] = {
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv4-prefix",

        .plugin.id = "libyang 2 - ipv4-prefix, version 1",
        .plugin.store = lyplg_type_store_ipv4_prefix,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv6-prefix",

        .plugin.id = "libyang 2 - ipv6-prefix, version 1",
        .plugin.store = lyplg_type_store_ipv6_prefix,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.hash = lyplg_type_hash_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
