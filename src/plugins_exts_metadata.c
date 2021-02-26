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

/**
 * @brief Representation of the compiled metadata substatements - simplify storage for the items available via
 * ::lysc_ext_substmt.
 */
struct lyext_metadata {
    struct lysc_type *type;            /**< type of the metadata (mandatory) */
    const char *units;                 /**< units of the leaf's type */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    const char *dsc;                   /**< description */
    const char *ref;                   /**< reference */
    uint16_t flags;                    /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
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
    LY_ARRAY_CREATE_RET(cctx->ctx, c_ext->substmts, 6, LY_EMEM);
    c_ext->data = annotation = calloc(1, sizeof *annotation);
    if (!annotation) {
        LOGMEM(cctx->ctx);
        LY_ARRAY_FREE(c_ext->substmts);
        c_ext->substmts = NULL;
        return LY_EMEM;
    }

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_IFF].stmt = LY_STMT_IF_FEATURE;
    c_ext->substmts[ANNOTATION_SUBSTMT_IFF].cardinality = LY_STMT_CARD_ANY;
    c_ext->substmts[ANNOTATION_SUBSTMT_IFF].storage = &annotation->iffeatures;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_UNITS].stmt = LY_STMT_UNITS;
    c_ext->substmts[ANNOTATION_SUBSTMT_UNITS].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_UNITS].storage = &annotation->units;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_STATUS].stmt = LY_STMT_STATUS;
    c_ext->substmts[ANNOTATION_SUBSTMT_STATUS].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_STATUS].storage = &annotation->flags;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_TYPE].stmt = LY_STMT_TYPE;
    c_ext->substmts[ANNOTATION_SUBSTMT_TYPE].cardinality = LY_STMT_CARD_MAND;
    c_ext->substmts[ANNOTATION_SUBSTMT_TYPE].storage = &annotation->type;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_DSC].stmt = LY_STMT_DESCRIPTION;
    c_ext->substmts[ANNOTATION_SUBSTMT_DSC].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_DSC].storage = &annotation->dsc;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_REF].stmt = LY_STMT_REFERENCE;
    c_ext->substmts[ANNOTATION_SUBSTMT_REF].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_REF].storage = &annotation->ref;

    LY_CHECK_RET(lys_compile_extension_instance(cctx, p_ext, c_ext->substmts));

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
    if (!ext->substmts) {
        return;
    }

    lysc_extension_instance_substatements_free(ctx, ext->substmts);
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
