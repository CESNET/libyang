/**
 * @file plugins_exts_nacm.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - Metadata (RFC 7952)
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

#include <stdlib.h>

#include "plugins_exts.h"
#include "plugins_exts_metadata.h"
#include "schema_compile.h"
#include "tree_schema.h"

/**
 * @brief Storage for ID used to check plugin API version compatibility.
 * Ignored here in the internal plugin.
LYEXT_VERSION_CHECK
 */

#define ANNOTATION_SUBSTMT_IFF     0
#define ANNOTATION_SUBSTMT_UNITS   1
#define ANNOTATION_SUBSTMT_STATUS  2
#define ANNOTATION_SUBSTMT_TYPE    3
#define ANNOTATION_SUBSTMT_DSC     4
#define ANNOTATION_SUBSTMT_REF     5

struct lysc_ext_substmt annotation_substmt[] = {
    {LY_STMT_IF_FEATURE, LY_STMT_CARD_ANY, NULL},
    {LY_STMT_UNITS, LY_STMT_CARD_OPT, NULL},
    {LY_STMT_STATUS, LY_STMT_CARD_OPT, NULL},
    {LY_STMT_TYPE, LY_STMT_CARD_MAND, NULL},
    {LY_STMT_DESCRIPTION, LY_STMT_CARD_OPT, NULL},
    {LY_STMT_REFERENCE, LY_STMT_CARD_OPT, NULL},
    {0, 0, 0} /* terminating item */
};

/**
 * @brief Compile annotation extension instances.
 *
 * Implementation of lyext_clb_compile callback set as lyext_plugin::compile.
 */
LY_ERR
annotation_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    struct lyext_metadata *annotation;
    struct lysc_module *mod_c;
    LY_ARRAY_COUNT_TYPE u;

    /* annotations can appear only at the top level of a YANG module or submodule */
    if (c_ext->parent_type != LYEXT_PAR_MODULE) {
        lyext_log(c_ext, LY_LLERR, LY_EVALID, cctx->path, "Extension %s is allowed only at the top level of a YANG module or submodule, but it is placed in \"%s\" statement.",
                p_ext->name, lyext_parent2str(c_ext->parent_type));
        return LY_EVALID;
    }
    /* check mandatory argument */
    if (!c_ext->argument) {
        lyext_log(c_ext, LY_LLERR, LY_EVALID, cctx->path, "Extension %s is instantiated without mandatory argument representing metadata name.",
                p_ext->name);
        return LY_EVALID;
    }

    mod_c = (struct lysc_module *)c_ext->parent;

    /* check for duplication */
    LY_ARRAY_FOR(mod_c->exts, u) {
        if ((&mod_c->exts[u] != c_ext) && (mod_c->exts[u].def == c_ext->def) && !strcmp(mod_c->exts[u].argument, c_ext->argument)) {
            /* duplication of the same annotation extension in a single module */
            lyext_log(c_ext, LY_LLERR, LY_EVALID, cctx->path, "Extension %s is instantiated multiple times.", p_ext->name);
            return LY_EVALID;
        }
    }

    /* compile annotation substatements */
    c_ext->data = annotation = calloc(1, sizeof *annotation);
    LY_CHECK_ERR_RET(!annotation, LOGMEM(cctx->ctx), LY_EMEM);
    annotation_substmt[ANNOTATION_SUBSTMT_IFF].storage = &annotation->iffeatures;
    annotation_substmt[ANNOTATION_SUBSTMT_UNITS].storage = &annotation->units;
    annotation_substmt[ANNOTATION_SUBSTMT_STATUS].storage = &annotation->flags;
    annotation_substmt[ANNOTATION_SUBSTMT_TYPE].storage = &annotation->type;
    /* description and reference are allowed, but not compiled */

    LY_CHECK_RET(lys_compile_extension_instance(cctx, p_ext, annotation_substmt));

    return LY_SUCCESS;
}

/**
 * @brief Free annotation extension instances' data.
 *
 * Implementation of lyext_clb_free callback set as lyext_plugin::free.
 */
void
annotation_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    if (!ext->data) {
        return;
    }

    struct lyext_metadata *annotation = (struct lyext_metadata *)ext->data;

    annotation_substmt[ANNOTATION_SUBSTMT_IFF].storage = &annotation->iffeatures;
    annotation_substmt[ANNOTATION_SUBSTMT_UNITS].storage = &annotation->units;
    annotation_substmt[ANNOTATION_SUBSTMT_STATUS].storage = &annotation->flags;
    annotation_substmt[ANNOTATION_SUBSTMT_TYPE].storage = &annotation->type;

    lysc_extension_instance_free(ctx, annotation_substmt);
    free(ext->data);
}

/**
 * @brief Plugin for the Metadata's annotation extension
 */
struct lyext_plugin metadata_plugin = {
    .id = "libyang 2 - metadata, version 1",
    .compile = &annotation_compile,
    .validate = NULL,
    .free = annotation_free
};
