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
#include "plugins_exts_compile.h"
#include "plugins_exts_print.h"

#include <stdint.h>
#include <string.h>

extern struct lyext_plugin metadata_plugin; /* plugins_exts_metadata.c */
extern struct lyext_plugin nacm_plugin;     /* plugins_exts_nacm.c */
extern struct lyext_plugin yangdata_plugin; /* plugins_exts_yangdata.c */

/* internal libyang headers - do not make them accessible to the extension plugins in plugins_exts_*.c */
#include "common.h"
#include "plugins_exts_internal.h"
#include "printer_internal.h"
#include "schema_compile.h"

/**
 * @brief list of all extension plugins implemented internally
 */
struct lyext_plugin_record lyext_plugins_internal[] = {
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

API struct ly_ctx *
lysc_ctx_get_ctx(const struct lysc_ctx *ctx)
{
    return ctx->ctx;
}

API uint32_t *
lysc_ctx_get_options(const struct lysc_ctx *ctx)
{
    return &((struct lysc_ctx *)ctx)->options;
}

API const char *
lysc_ctx_get_path(const struct lysc_ctx *ctx)
{
    return ctx->path;
}

API struct ly_out **
lys_ypr_ctx_get_out(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->out;
}

API uint32_t *
lys_ypr_ctx_get_options(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->options;
}

API uint16_t *
lys_ypr_ctx_get_level(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->level;
}
