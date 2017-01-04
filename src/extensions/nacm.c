/**
 * @file nacm.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - NACM (RFC 6536)
 *
 * Copyright (c) 2016-2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#include "../extensions.h"

/**
 * @brief Callback to check that the NACM extension can be instantiated inside the provided node
 *
 * @param[in] parent The parent of the instantiated extension.
 * @param[in] parent_type The type of the structure provided as \p parent.
 * @param[in] substmt_type libyang does not store all the extension instances in the structures where they are
 *                         instantiated in the module. In some cases (see #LYEXT_SUBSTMT) they are stored in parent
 *                         structure and marked with flag to know in which substatement of the parent the extension
 *                         was originally instantiated.
 * @return 0 - ok
 *         1 - error
 */
int nacm_position(const void *parent, LYEXT_PAR parent_type, LYEXT_SUBSTMT UNUSED(substmt_type))
{
    if (parent_type != LYEXT_PAR_NODE) {
        return 2;
    }

    switch(((struct lys_node*)parent)->nodetype) {
    case LYS_CONTAINER:
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_LIST:
    case LYS_CHOICE:
    case LYS_ANYXML:
    case LYS_AUGMENT:
    case LYS_CASE:
    case LYS_USES:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
        return 0;
    default:
        return 2;
    }
}

/**
 * @brief Plugin for the NACM's default-deny-write extension
 */
struct lyext_plugin nacm_deny_write = {
    .type = LYEXT_FLAG,
    .check_position = &nacm_position,
    .check_result = NULL,
};

/**
 * @brief Plugin for the NACM's default-deny-all extension
 */
struct lyext_plugin nacm_deny_all = {
    .type = LYEXT_FLAG,
    .check_position = &nacm_position,
    .check_result = NULL,
};

/**
 * @brief list of all extension plugins implemented here
 *
 * MANDATORY object for all libyang extension plugins, the name must match the <name>.so
 */
struct lyext_plugin_list nacm[] = {
    {"ietf-netconf-acm", "2012-02-22", "default-deny-write", &nacm_deny_write},
    {"ietf-netconf-acm", "2012-02-22", "default-deny-all", &nacm_deny_all},
    {NULL, NULL, NULL, NULL} /* terminating item */
};
