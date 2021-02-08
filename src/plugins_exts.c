/**
 * @file plugins_exts.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Internally implemented YANG extensions.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_exts.h"

#include <stdint.h>
#include <string.h>

#include "common.h"
#include "plugins_exts_metadata.c"
#include "plugins_exts_nacm.c"
#include "plugins_exts_yangdata.c"

/**
 * @brief list of all extension plugins implemented internally
 */
struct lyext_plugins_list lyext_plugins_internal[] = {
    {"ietf-netconf-acm", "2012-02-22", "default-deny-write", &nacm_plugin},
    {"ietf-netconf-acm", "2018-02-14", "default-deny-write", &nacm_plugin},
    {"ietf-netconf-acm", "2012-02-22", "default-deny-all", &nacm_plugin},
    {"ietf-netconf-acm", "2018-02-14", "default-deny-all", &nacm_plugin},
    {"ietf-yang-metadata", "2016-08-05", "annotation", &metadata_plugin},
    {"ietf-restconf", "2017-01-26", "yang-data", &yangdata_plugin},
    {NULL, NULL, NULL, NULL} /* terminating item */
};

/* TODO support for external extension plugins */

struct lyext_plugin *
lyext_get_plugin(struct lysc_ext *ext)
{
    for (uint8_t u = 0; lyext_plugins_internal[u].module; ++u) {
        if (!strcmp(ext->name, lyext_plugins_internal[u].name) &&
                !strcmp(ext->module->name, lyext_plugins_internal[u].module) &&
                (!lyext_plugins_internal[u].revision || !strcmp(ext->module->revision, lyext_plugins_internal[u].revision))) {
            /* we have the match */
            return lyext_plugins_internal[u].plugin;
        }
    }

    return NULL;
}

API const char *
lyext_parent2str(LYEXT_PARENT type)
{
    switch (type) {
    case LYEXT_PAR_MODULE:
        return "module";
    case LYEXT_PAR_NODE:
        return "data node";
    case LYEXT_PAR_INPUT:
        return "input";
    case LYEXT_PAR_OUTPUT:
        return "output";
    case LYEXT_PAR_TYPE:
        return "type";
    case LYEXT_PAR_TYPE_BIT:
        return "bit";
    case LYEXT_PAR_TYPE_ENUM:
        return "enum";
    case LYEXT_PAR_MUST:
        return "must";
    case LYEXT_PAR_PATTERN:
        return "pattern";
    case LYEXT_PAR_LENGTH:
        return "length";
    case LYEXT_PAR_RANGE:
        return "range";
    case LYEXT_PAR_WHEN:
        return "when";
    case LYEXT_PAR_IDENT:
        return "identity";
    case LYEXT_PAR_EXT:
        return "extension instance";
    case LYEXT_PAR_IMPORT:
        return "import";
    /* YANG allows extension instances inside the following statements,
     * but they do not have any meaning in current libyang
        case LYEXT_PAR_TPDF:
            return "typedef";
        case LYEXT_PAR_EXTINST:
            return "extension";
        case LYEXT_PAR_REFINE:
            return "refine";
        case LYEXT_PAR_DEVIATION:
            return "deviation";
        case LYEXT_PAR_DEVIATE:
            return "deviate";
        case LYEXT_PAR_INCLUDE:
            return "include";
        case LYEXT_PAR_REVISION:
            return "revision";
     */
    default:
        return "unknown";
    }
}
