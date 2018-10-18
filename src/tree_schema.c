/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "tree_schema_internal.h"

#define FREE_ARRAY(CTX, ARRAY, FUNC) {uint64_t c__; LY_ARRAY_FOR(ARRAY, c__){FUNC(CTX, LY_ARRAY_INDEX(ARRAY, c__), dict);}free(ARRAY);}
#define FREE_MEMBER(CTX, MEMBER, FUNC) if (MEMBER) {FUNC(CTX, MEMBER, dict);free(MEMBER);}
#define FREE_STRING(CTX, STRING) if (dict && STRING) {lydict_remove(CTX, STRING);}

static void lysp_grp_free(struct ly_ctx *ctx, struct lysp_grp *grp, int dict);
static void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node, int dict);

#define LYSC_CTX_BUFSIZE 4086
struct lysc_ctx {
    struct lysc_module *mod;
    uint16_t path_len;
    char path[LYSC_CTX_BUFSIZE];
};

static void
lysp_stmt_free(struct ly_ctx *ctx, struct lysp_stmt *stmt, int dict)
{
    struct lysp_stmt *child, *next;

    FREE_STRING(ctx, stmt->stmt);
    FREE_STRING(ctx, stmt->arg);

    LY_LIST_FOR_SAFE(stmt->child, next, child) {
        lysp_stmt_free(ctx, child, dict);
    }

    free(stmt);
}

static void
lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext, int dict)
{
    struct lysp_stmt *stmt, *next;

    FREE_STRING(ctx, ext->name);
    FREE_STRING(ctx, ext->argument);

    LY_LIST_FOR_SAFE(ext->child, next, stmt) {
        lysp_stmt_free(ctx, stmt, dict);
    }
}

static void
lysp_import_free(struct ly_ctx *ctx, struct lysp_import *import, int dict)
{
    FREE_STRING(ctx, import->name);
    FREE_STRING(ctx, import->prefix);
    FREE_STRING(ctx, import->dsc);
    FREE_STRING(ctx, import->ref);
    FREE_ARRAY(ctx, import->exts, lysp_ext_instance_free);
    {
        void *p__;
        int64_t c__;
        for(p__ = ((void*)((uint32_t*)((import->exts) + 0) + 1)), c__ = 0;
                (import->exts) && c__ < (*(uint32_t*)(import->exts));
                p__ = ((void*)((uint32_t*)((import->exts) + c__) + 1))) {
            lysp_ext_instance_free(ctx, p__, dict);
        }
        free(import->exts);
    }
}

static void
lysp_include_free(struct ly_ctx *ctx, struct lysp_include *include, int dict)
{
    FREE_STRING(ctx, include->name);
    FREE_STRING(ctx, include->dsc);
    FREE_STRING(ctx, include->ref);
    FREE_ARRAY(ctx, include->exts, lysp_ext_instance_free);
}

static void
lysp_revision_free(struct ly_ctx *ctx, struct lysp_revision *rev, int dict)
{
    FREE_STRING(ctx, rev->dsc);
    FREE_STRING(ctx, rev->ref);
    FREE_ARRAY(ctx, rev->exts, lysp_ext_instance_free);
}

static void
lysp_ext_free(struct ly_ctx *ctx, struct lysp_ext *ext, int dict)
{
    FREE_STRING(ctx, ext->name);
    FREE_STRING(ctx, ext->argument);
    FREE_STRING(ctx, ext->dsc);
    FREE_STRING(ctx, ext->ref);
    FREE_ARRAY(ctx, ext->exts, lysp_ext_instance_free);
}

static void
lysp_feature_free(struct ly_ctx *ctx, struct lysp_feature *feat, int dict)
{
    unsigned int u;
    FREE_STRING(ctx, feat->name);
    for (u = 0; feat->iffeatures && feat->iffeatures[u]; ++u) {
        FREE_STRING(ctx, feat->iffeatures[u]);
    }
    free(feat->iffeatures);
    FREE_STRING(ctx, feat->dsc);
    FREE_STRING(ctx, feat->ref);
    FREE_ARRAY(ctx, feat->exts, lysp_ext_instance_free);
}

static void
lysp_ident_free(struct ly_ctx *ctx, struct lysp_ident *ident, int dict)
{
    unsigned int u;
    FREE_STRING(ctx, ident->name);
    for (u = 0; ident->iffeatures && ident->iffeatures[u]; ++u) {
        FREE_STRING(ctx, ident->iffeatures[u]);
    }
    free(ident->iffeatures);
    for (u = 0; ident->bases && ident->bases[u]; ++u) {
        FREE_STRING(ctx, ident->bases[u]);
    }
    free(ident->bases);
    FREE_STRING(ctx, ident->dsc);
    FREE_STRING(ctx, ident->ref);
    FREE_ARRAY(ctx, ident->exts, lysp_ext_instance_free);
}

static void
lysp_restr_free(struct ly_ctx *ctx, struct lysp_restr *restr, int dict)
{
    FREE_STRING(ctx, restr->arg);
    FREE_STRING(ctx, restr->emsg);
    FREE_STRING(ctx, restr->eapptag);
    FREE_STRING(ctx, restr->dsc);
    FREE_STRING(ctx, restr->ref);
    FREE_ARRAY(ctx, restr->exts, lysp_ext_instance_free);
}

static void
lysp_type_enum_free(struct ly_ctx *ctx, struct lysp_type_enum *item, int dict)
{
    unsigned int u;
    FREE_STRING(ctx, item->name);
    FREE_STRING(ctx, item->dsc);
    FREE_STRING(ctx, item->ref);
    for (u = 0; item->iffeatures && item->iffeatures[u]; ++u) {
        FREE_STRING(ctx, item->iffeatures[u]);
    }
    free(item->iffeatures);
    FREE_ARRAY(ctx, item->exts, lysp_ext_instance_free);
}

static void
lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type, int dict)
{
    unsigned int u;
    FREE_STRING(ctx, type->name);
    FREE_MEMBER(ctx, type->range, lysp_restr_free);
    FREE_MEMBER(ctx, type->length, lysp_restr_free);
    FREE_ARRAY(ctx, type->patterns, lysp_restr_free);
    FREE_ARRAY(ctx, type->enums, lysp_type_enum_free);
    FREE_ARRAY(ctx, type->bits, lysp_type_enum_free);
    FREE_STRING(ctx, type->path);
    for (u = 0; type->bases && type->bases[u]; ++u) {
        FREE_STRING(ctx, type->bases[u]);
    }
    free(type->bases);
    FREE_ARRAY(ctx, type->types, lysp_type_free);
    FREE_ARRAY(ctx, type->exts, lysp_ext_instance_free);
}

static void
lysp_tpdf_free(struct ly_ctx *ctx, struct lysp_tpdf *tpdf, int dict)
{
    FREE_STRING(ctx, tpdf->name);
    FREE_STRING(ctx, tpdf->units);
    FREE_STRING(ctx, tpdf->dflt);
    FREE_STRING(ctx, tpdf->dsc);
    FREE_STRING(ctx, tpdf->ref);
    FREE_ARRAY(ctx, tpdf->exts, lysp_ext_instance_free);
    lysp_type_free(ctx, &tpdf->type, dict);
}

static void
lysp_action_inout_free(struct ly_ctx *ctx, struct lysp_action_inout *inout, int dict)
{
    struct lysp_node *node, *next;

    FREE_ARRAY(ctx, inout->musts, lysp_restr_free);
    FREE_ARRAY(ctx, inout->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, inout->groupings, lysp_grp_free);
    LY_LIST_FOR_SAFE(inout->data, next, node) {
        lysp_node_free(ctx, node, dict);
    }
    FREE_ARRAY(ctx, inout->exts, lysp_ext_instance_free);

}

static void
lysp_action_free(struct ly_ctx *ctx, struct lysp_action *action, int dict)
{
    unsigned int u;
    FREE_STRING(ctx, action->name);
    FREE_STRING(ctx, action->dsc);
    FREE_STRING(ctx, action->ref);
    for (u = 0; action->iffeatures && action->iffeatures[u]; ++u) {
        FREE_STRING(ctx, action->iffeatures[u]);
    }
    free(action->iffeatures);
    FREE_ARRAY(ctx, action->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, action->groupings, lysp_grp_free);
    FREE_MEMBER(ctx, action->input, lysp_action_inout_free);
    FREE_MEMBER(ctx, action->output, lysp_action_inout_free);
    FREE_ARRAY(ctx, action->exts, lysp_ext_instance_free);
}

static void
lysp_notif_free(struct ly_ctx *ctx, struct lysp_notif *notif, int dict)
{
    unsigned int u;
    struct lysp_node *node, *next;

    FREE_STRING(ctx, notif->name);
    FREE_STRING(ctx, notif->dsc);
    FREE_STRING(ctx, notif->ref);
    for (u = 0; notif->iffeatures && notif->iffeatures[u]; ++u) {
        FREE_STRING(ctx, notif->iffeatures[u]);
    }
    free(notif->iffeatures);
    FREE_ARRAY(ctx, notif->musts, lysp_restr_free);
    FREE_ARRAY(ctx, notif->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, notif->groupings, lysp_grp_free);
    LY_LIST_FOR_SAFE(notif->data, next, node) {
        lysp_node_free(ctx, node, dict);
    }
    FREE_ARRAY(ctx, notif->exts, lysp_ext_instance_free);
}

static void
lysp_grp_free(struct ly_ctx *ctx, struct lysp_grp *grp, int dict)
{
    struct lysp_node *node, *next;

    FREE_STRING(ctx, grp->name);
    FREE_STRING(ctx, grp->dsc);
    FREE_STRING(ctx, grp->ref);
    FREE_ARRAY(ctx, grp->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, grp->groupings, lysp_grp_free);
    LY_LIST_FOR_SAFE(grp->data, next, node) {
        lysp_node_free(ctx, node, dict);
    }
    FREE_ARRAY(ctx, grp->actions, lysp_action_free);
    FREE_ARRAY(ctx, grp->notifs, lysp_notif_free);
    FREE_ARRAY(ctx, grp->exts, lysp_ext_instance_free);
}

static void
lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when, int dict)
{
    FREE_STRING(ctx, when->cond);
    FREE_STRING(ctx, when->dsc);
    FREE_STRING(ctx, when->ref);
    FREE_ARRAY(ctx, when->exts, lysp_ext_instance_free);
}

static void
lysp_augment_free(struct ly_ctx *ctx, struct lysp_augment *augment, int dict)
{
    unsigned int u;
    struct lysp_node *node, *next;

    FREE_STRING(ctx, augment->nodeid);
    FREE_STRING(ctx, augment->dsc);
    FREE_STRING(ctx, augment->ref);
    FREE_MEMBER(ctx, augment->when, lysp_when_free);
    for (u = 0; augment->iffeatures && augment->iffeatures[u]; ++u) {
        FREE_STRING(ctx, augment->iffeatures[u]);
    }
    free(augment->iffeatures);
    LY_LIST_FOR_SAFE(augment->child, next, node) {
        lysp_node_free(ctx, node, dict);
    }
    FREE_ARRAY(ctx, augment->actions, lysp_action_free);
    FREE_ARRAY(ctx, augment->notifs, lysp_notif_free);
    FREE_ARRAY(ctx, augment->exts, lysp_ext_instance_free);
}

static void
lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d, int dict)
{
    unsigned int u;
    struct lysp_deviate_add *add = (struct lysp_deviate_add*)d;
    struct lysp_deviate_rpl *rpl = (struct lysp_deviate_rpl*)d;

    FREE_ARRAY(ctx, d->exts, lysp_ext_instance_free);
    switch(d->mod) {
    case LYS_DEV_NOT_SUPPORTED:
        /* nothing to do */
        break;
    case LYS_DEV_ADD:
    case LYS_DEV_DELETE: /* compatible for dynamically allocated data */
        FREE_STRING(ctx, add->units);
        FREE_ARRAY(ctx, add->musts, lysp_restr_free);
        for (u = 0; add->uniques && add->uniques[u]; ++u) {
            FREE_STRING(ctx, add->uniques[u]);
        }
        free(add->uniques);
        for (u = 0; add->dflts && add->dflts[u]; ++u) {
            FREE_STRING(ctx, add->dflts[u]);
        }
        free(add->dflts);
        break;
    case LYS_DEV_REPLACE:
        FREE_MEMBER(ctx, rpl->type, lysp_type_free);
        FREE_STRING(ctx, rpl->units);
        FREE_STRING(ctx, rpl->dflt);
        break;
    default:
        LOGINT(ctx);
        break;
    }
}

static void
lysp_deviation_free(struct ly_ctx *ctx, struct lysp_deviation *dev, int dict)
{
    struct lysp_deviate *next, *iter;

    FREE_STRING(ctx, dev->nodeid);
    FREE_STRING(ctx, dev->dsc);
    FREE_STRING(ctx, dev->ref);
    LY_LIST_FOR_SAFE(dev->deviates, next, iter) {
        lysp_deviate_free(ctx, iter, dict);
        free(iter);
    }
    FREE_ARRAY(ctx, dev->exts, lysp_ext_instance_free);
}

static void
lysp_refine_free(struct ly_ctx *ctx, struct lysp_refine *ref, int dict)
{
    unsigned int u;
    FREE_STRING(ctx, ref->nodeid);
    FREE_STRING(ctx, ref->dsc);
    FREE_STRING(ctx, ref->ref);
    for (u = 0; ref->iffeatures && ref->iffeatures[u]; ++u) {
        FREE_STRING(ctx, ref->iffeatures[u]);
    }
    free(ref->iffeatures);
    FREE_ARRAY(ctx, ref->musts, lysp_restr_free);
    FREE_STRING(ctx, ref->presence);
    for (u = 0; ref->dflts && ref->dflts[u]; ++u) {
        FREE_STRING(ctx, ref->dflts[u]);
    }
    free(ref->dflts);
    FREE_ARRAY(ctx, ref->exts, lysp_ext_instance_free);
}

static void
lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node, int dict)
{
    unsigned int u;
    struct lysp_node *child, *next;

    FREE_STRING(ctx, node->name);
    FREE_STRING(ctx, node->dsc);
    FREE_STRING(ctx, node->ref);
    FREE_MEMBER(ctx, node->when, lysp_when_free);
    for (u = 0; node->iffeatures && node->iffeatures[u]; ++u) {
        FREE_STRING(ctx, node->iffeatures[u]);
    }
    free(node->iffeatures);
    FREE_ARRAY(ctx, node->exts, lysp_ext_instance_free);

    switch(node->nodetype) {
    case LYS_CONTAINER:
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->musts, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_container*)node)->presence);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->typedefs, lysp_tpdf_free);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->groupings, lysp_grp_free);
        LY_LIST_FOR_SAFE(((struct lysp_node_container*)node)->child, next, child) {
            lysp_node_free(ctx, child, dict);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->actions, lysp_action_free);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->notifs, lysp_notif_free);
        break;
    case LYS_LEAF:
        FREE_ARRAY(ctx, ((struct lysp_node_leaf*)node)->musts, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaf*)node)->type, dict);
        FREE_STRING(ctx, ((struct lysp_node_leaf*)node)->units);
        FREE_STRING(ctx, ((struct lysp_node_leaf*)node)->dflt);
        break;
    case LYS_LEAFLIST:
        FREE_ARRAY(ctx, ((struct lysp_node_leaflist*)node)->musts, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaflist*)node)->type, dict);
        FREE_STRING(ctx, ((struct lysp_node_leaflist*)node)->units);
        for (u = 0; ((struct lysp_node_leaflist*)node)->dflts && ((struct lysp_node_leaflist*)node)->dflts[u]; ++u) {
            FREE_STRING(ctx, ((struct lysp_node_leaflist*)node)->dflts[u]);
        }
        free(((struct lysp_node_leaflist*)node)->dflts);
        break;
    case LYS_LIST:
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->musts, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_list*)node)->key);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->typedefs, lysp_tpdf_free);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->groupings,  lysp_grp_free);
        LY_LIST_FOR_SAFE(((struct lysp_node_list*)node)->child, next, child) {
            lysp_node_free(ctx, child, dict);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->actions, lysp_action_free);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->notifs, lysp_notif_free);
        for (u = 0; ((struct lysp_node_list*)node)->uniques && ((struct lysp_node_list*)node)->uniques[u]; ++u) {
            FREE_STRING(ctx, ((struct lysp_node_list*)node)->uniques[u]);
        }
        free(((struct lysp_node_list*)node)->uniques);
        break;
    case LYS_CHOICE:
        LY_LIST_FOR_SAFE(((struct lysp_node_choice*)node)->child, next, child) {
            lysp_node_free(ctx, child, dict);
        }
        FREE_STRING(ctx, ((struct lysp_node_choice*)node)->dflt);
        break;
    case LYS_CASE:
        LY_LIST_FOR_SAFE(((struct lysp_node_case*)node)->child, next, child) {
            lysp_node_free(ctx, child, dict);
        }
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        FREE_ARRAY(ctx, ((struct lysp_node_anydata*)node)->musts, lysp_restr_free);
        break;
    case LYS_USES:
        FREE_ARRAY(ctx, ((struct lysp_node_uses*)node)->refines, lysp_refine_free);
        FREE_ARRAY(ctx, ((struct lysp_node_uses*)node)->augments, lysp_augment_free);
        break;
    default:
        LOGINT(ctx);
    }

    free(node);
}

static void
lysp_module_free_(struct lysp_module *module, int dict)
{
    struct ly_ctx *ctx;
    struct lysp_node *node, *next;

    LY_CHECK_ARG_RET(NULL, module,);
    ctx = module->ctx;

    FREE_STRING(ctx, module->name);
    FREE_STRING(ctx, module->filepath);
    FREE_STRING(ctx, module->ns);  /* or belongs-to */
    FREE_STRING(ctx, module->prefix);

    FREE_ARRAY(ctx, module->imports, lysp_import_free);
    FREE_ARRAY(ctx, module->includes, lysp_include_free);

    FREE_STRING(ctx, module->org);
    FREE_STRING(ctx, module->contact);
    FREE_STRING(ctx, module->dsc);
    FREE_STRING(ctx, module->ref);

    FREE_ARRAY(ctx, module->revs, lysp_revision_free);
    FREE_ARRAY(ctx, module->extensions, lysp_ext_free);
    FREE_ARRAY(ctx, module->features, lysp_feature_free);
    FREE_ARRAY(ctx, module->identities, lysp_ident_free);
    FREE_ARRAY(ctx, module->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, module->groupings, lysp_grp_free);
    LY_LIST_FOR_SAFE(module->data, next, node) {
        lysp_node_free(ctx, node, dict);
    }
    FREE_ARRAY(ctx, module->augments, lysp_augment_free);
    FREE_ARRAY(ctx, module->rpcs, lysp_action_free);
    FREE_ARRAY(ctx, module->notifs, lysp_notif_free);
    FREE_ARRAY(ctx, module->deviations, lysp_deviation_free);
    FREE_ARRAY(ctx, module->exts, lysp_ext_instance_free);

    free(module);
}

API void
lysp_module_free(struct lysp_module *module)
{
    lysp_module_free_(module, 1);
}

static void
lysc_feature_free(struct ly_ctx *ctx, struct lysc_feature *feat, int dict)
{
    FREE_STRING(ctx, feat->name);

}

static void
lysc_module_free_(struct lysc_module *module, int dict)
{
    struct ly_ctx *ctx;

    LY_CHECK_ARG_RET(NULL, module,);
    ctx = module->ctx;

    FREE_STRING(ctx, module->name);
    FREE_STRING(ctx, module->ns);
    FREE_STRING(ctx, module->prefix);


    FREE_ARRAY(ctx, module->features, lysc_feature_free);


    free(module);
}

API void
lysc_module_free(struct lysc_module *module, void (*private_destructor)(const struct lysc_node *node, void *priv))
{
    lysc_module_free_(module, 1);
}

void
lys_module_free(struct lys_module *module, void (*private_destructor)(const struct lysc_node *node, void *priv))
{
    if (!module) {
        return;
    }

    lysc_module_free(module->compiled, private_destructor);
    lysp_module_free(module->parsed);
    free(module);
}

static LY_ERR
lys_compile_iffeature(struct lysc_ctx *ctx, const char *iff_p, struct lysc_iffeature **iffeatures)
{
    struct lysc_iffeature *iff;

    if ((ctx->mod->version != 2) && ((iff_p[0] == '(') || strchr(iff_p, ' '))) {
        LOGVAL(ctx->mod->ctx, LY_VLOG_STR, ctx->path,LY_VCODE_INVAL, strlen(iff_p), iff_p, "if-feature");
        return LY_EVALID;
    }

    LYSP_ARRAY_NEW_RET(ctx->mod->ctx, *iffeatures, iff, LY_EMEM);

    return LY_SUCCESS;
}

static LY_ERR
lys_compile_feature(struct lysc_ctx *ctx, struct lysp_feature *feature_p, int options, struct lysc_feature **features)
{
    struct lysc_feature *feature;
    unsigned int u;
    LY_ERR ret;

    LYSP_ARRAY_NEW_RET(ctx->mod->ctx, *features, feature, LY_EMEM);

    if (options & LYSC_OPT_FREE_SP) {
        /* just switch the pointers */
        feature->name = feature_p->name;
    } else {
        /* keep refcounts correct for lysp_module_free() */
        feature->name = lydict_insert(ctx->mod->ctx, feature_p->name, 0);
    }
    feature->flags = feature_p->flags;

    for (u = 0; feature_p->iffeatures && feature_p->iffeatures[u]; ++u) {
        ret = lys_compile_iffeature(ctx, feature_p->iffeatures[u], &feature->iffeatures);
        LY_CHECK_RET(ret);
    }

    return LY_SUCCESS;
}

LY_ERR
lys_compile(struct lysp_module *sp, int options, struct lysc_module **sc)
{
    struct lysc_ctx ctx = {0};
    struct lysc_module *mod_c;
    void *p;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, sc, sp, sp->ctx, LY_EINVAL);

    if (sp->submodule) {
        LOGERR(sp->ctx, LY_EINVAL, "Submodules (%s) are not supposed to be compiled, compile only the main modules.", sp->name);
        return LY_EINVAL;
    }

    ctx.mod = mod_c = calloc(1, sizeof *mod_c);
    LY_CHECK_ERR_RET(!mod_c, LOGMEM(sp->ctx), LY_EMEM);
    mod_c->ctx = sp->ctx;
    mod_c->version = sp->version;

    if (options & LYSC_OPT_FREE_SP) {
        /* just switch the pointers */
        mod_c->name = sp->name;
        mod_c->ns = sp->ns;
        mod_c->prefix = sp->prefix;
    } else {
        /* keep refcounts correct for lysp_module_free() */
        mod_c->name = lydict_insert(sp->ctx, sp->name, 0);
        mod_c->ns = lydict_insert(sp->ctx, sp->ns, 0);
        mod_c->prefix = lydict_insert(sp->ctx, sp->prefix, 0);
    }

    if (sp->features) {
        LY_ARRAY_FOR(sp->features, struct lysp_feature, p) {
            ret = lys_compile_feature(&ctx, p, options, &mod_c->features);
            LY_CHECK_GOTO(ret != LY_SUCCESS, error);
        }
    }

    if (options & LYSC_OPT_FREE_SP) {
        lysp_module_free_(sp, 0);
    }

    (*sc) = mod_c;
    return LY_SUCCESS;

error:

    if (options & LYSC_OPT_FREE_SP) {
        lysc_module_free_(mod_c, 0);
    } else {
        lysc_module_free_(mod_c, 1);
    }
    return ret;
}

const struct lys_module *
lys_parse_mem_(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, const char *revision, int implement)
{
    struct lys_module *mod = NULL;
    LY_ERR ret;

    LY_CHECK_ARG_RET(ctx, ctx, data, NULL);

    mod = calloc(1, sizeof *mod);
    LY_CHECK_ERR_RET(!mod, LOGMEM(ctx), NULL);

    switch (format) {
    case LYS_IN_YIN:
        /* TODO not yet supported
        mod = yin_read_module(ctx, data, revision, implement);
        */
        break;
    case LYS_IN_YANG:
        ret = yang_parse(ctx, data, &mod->parsed);
        LY_CHECK_RET(ret, NULL);
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        break;
    }

    if (implement) {
        mod->parsed->implemented = 1;
    }

    if (revision) {
        /* check revision of the parsed model */
        if (!mod->parsed->revs || strcmp(revision, LY_ARRAY_INDEX(mod->parsed->revs, 0, struct lysp_revision)->rev)) {
            LOGERR(ctx, LY_EINVAL, "Module \"%s\" parsed with the wrong revision (\"%s\" instead \"%s\").",
                   mod->parsed->name, LY_ARRAY_INDEX(mod->parsed->revs, 0, struct lysp_revision)->rev, revision);
            lysp_module_free(mod->parsed);
            free(mod);
            return NULL;
        }
    }

    /* check for duplicity in the context */

    /* add into context */
    ly_set_add(&ctx->list, mod, LY_SET_OPT_USEASLIST);

#if 0
    /* hack for NETCONF's edit-config's operation attribute. It is not defined in the schema, but since libyang
     * implements YANG metadata (annotations), we need its definition. Because the ietf-netconf schema is not the
     * internal part of libyang, we cannot add the annotation into the schema source, but we do it here to have
     * the anotation definitions available in the internal schema structure. There is another hack in schema
     * printers to do not print this internally added annotation. */
    if (mod && ly_strequal(mod->name, "ietf-netconf", 0)) {
        if (lyp_add_ietf_netconf_annotations(mod)) {
            lys_free(mod, NULL, 1, 1);
            return NULL;
        }
    }
#endif

    return mod;
}

API const struct lys_module *
lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format)
{
    return lys_parse_mem_(ctx, data, format, NULL, 1);
}

static void
lys_parse_set_filename(struct ly_ctx *ctx, const char **filename, int fd)
{
#ifdef __APPLE__
    char path[MAXPATHLEN];
#else
    int len;
    char path[PATH_MAX], proc_path[32];
#endif

#ifdef __APPLE__
    if (fcntl(fd, F_GETPATH, path) != -1) {
        *filename = lydict_insert(ctx, path, 0);
    }
#else
    /* get URI if there is /proc */
    sprintf(proc_path, "/proc/self/fd/%d", fd);
    if ((len = readlink(proc_path, path, PATH_MAX - 1)) > 0) {
        *filename = lydict_insert(ctx, path, len);
    }
#endif
}

const struct lys_module *
lys_parse_fd_(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const char *revision, int implement)
{
    const struct lys_module *mod;
    size_t length;
    char *addr;

    LY_CHECK_ARG_RET(ctx, ctx, NULL);
    if (fd < 0) {
        LOGARG(ctx, fd);
        return NULL;
    }

    LY_CHECK_RET(ly_mmap(ctx, fd, &length, (void **)&addr), NULL);
    if (!addr) {
        LOGERR(ctx, LY_EINVAL, "Empty schema file.");
        return NULL;
    }

    mod = lys_parse_mem_(ctx, addr, format, revision, implement);
    ly_munmap(addr, length);

    if (mod && !mod->parsed->filepath) {
        lys_parse_set_filename(ctx, &mod->parsed->filepath, fd);
    }

    return mod;
}

API const struct lys_module *
lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    return lys_parse_fd_(ctx, fd, format, NULL, 1);
}

API const struct lys_module *
lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format)
{
    int fd;
    const struct lys_module *mod;
    const char *rev, *dot, *filename;
    size_t len;

    LY_CHECK_ARG_RET(ctx, ctx, path, NULL);

    fd = open(path, O_RDONLY);
    LY_CHECK_ERR_RET(fd == -1, LOGERR(ctx, LY_ESYS, "Opening file \"%s\" failed (%s).", path, strerror(errno)), NULL);

    mod = lys_parse_fd(ctx, fd, format);
    close(fd);
    LY_CHECK_RET(!mod, NULL);

    /* check that name and revision match filename */
    filename = strrchr(path, '/');
    if (!filename) {
        filename = path;
    } else {
        filename++;
    }
    rev = strchr(filename, '@');
    dot = strrchr(filename, '.');

    /* name */
    len = strlen(mod->parsed->name);
    if (strncmp(filename, mod->parsed->name, len) ||
            ((rev && rev != &filename[len]) || (!rev && dot != &filename[len]))) {
        LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, mod->parsed->name);
    }
    if (rev) {
        len = dot - ++rev;
        if (!mod->parsed->revs || len != 10 || strncmp(LY_ARRAY_INDEX(mod->parsed->revs, 0, struct lysp_revision)->rev, rev, len)) {
            LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                   mod->parsed->revs ? LY_ARRAY_INDEX(mod->parsed->revs, 0, struct lysp_revision)->rev : "none");
        }
    }

    if (!mod->parsed->filepath) {
        /* store URI */
        char rpath[PATH_MAX];
        if (realpath(path, rpath) != NULL) {
            mod->parsed->filepath = lydict_insert(ctx, rpath, 0);
        } else {
            mod->parsed->filepath = lydict_insert(ctx, path, 0);
        }
    }

    return mod;
}

