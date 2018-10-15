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

#include "libyang.h"
#include "common.h"
#include "tree_schema_internal.h"

#define FREE_ARRAY(CTX, ARRAY, ITER, FUNC) LY_ARRAY_FOR(ARRAY, ITER){FUNC(CTX, &ARRAY[ITER]);}free(ARRAY);
#define FREE_MEMBER(CTX, MEMBER, FUNC) if (MEMBER) {FUNC(CTX, MEMBER);free(MEMBER);}
#define FREE_STRING(CTX, STRING) if (STRING) {lydict_remove(CTX, STRING);}

static void lysp_grp_free(struct ly_ctx *ctx, struct lysp_grp *grp);
static void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);

static void
lysp_stmt_free(struct ly_ctx *ctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *child, *next;

    FREE_STRING(ctx, stmt->stmt);
    FREE_STRING(ctx, stmt->arg);

    LY_LIST_FOR_SAFE(stmt->child, next, child) {
        lysp_stmt_free(ctx, child);
    }

    free(stmt);
}

static void
lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext)
{
    struct lysp_stmt *stmt, *next;

    FREE_STRING(ctx, ext->name);
    FREE_STRING(ctx, ext->argument);

    LY_LIST_FOR_SAFE(ext->child, next, stmt) {
        lysp_stmt_free(ctx, stmt);
    }
}

static void
lysp_import_free(struct ly_ctx *ctx, struct lysp_import *import)
{
    unsigned int u;

    FREE_STRING(ctx, import->name);
    FREE_STRING(ctx, import->prefix);
    FREE_STRING(ctx, import->dsc);
    FREE_STRING(ctx, import->ref);
    FREE_ARRAY(ctx, import->exts, u, lysp_ext_instance_free);
}

static void
lysp_include_free(struct ly_ctx *ctx, struct lysp_include *include)
{
    unsigned int u;
    FREE_STRING(ctx, include->name);
    FREE_STRING(ctx, include->dsc);
    FREE_STRING(ctx, include->ref);
    FREE_ARRAY(ctx, include->exts, u, lysp_ext_instance_free);
}

static void
lysp_revision_free(struct ly_ctx *ctx, struct lysp_revision *rev)
{
    unsigned int u;
    FREE_STRING(ctx, rev->dsc);
    FREE_STRING(ctx, rev->ref);
    FREE_ARRAY(ctx, rev->exts, u, lysp_ext_instance_free);
}

static void
lysp_ext_free(struct ly_ctx *ctx, struct lysp_ext *ext)
{
    unsigned int u;
    FREE_STRING(ctx, ext->name);
    FREE_STRING(ctx, ext->argument);
    FREE_STRING(ctx, ext->dsc);
    FREE_STRING(ctx, ext->ref);
    FREE_ARRAY(ctx, ext->exts, u, lysp_ext_instance_free);
}

static void
lysp_feature_free(struct ly_ctx *ctx, struct lysp_feature *feat)
{
    unsigned int u;
    FREE_STRING(ctx, feat->name);
    for (u = 0; feat->iffeatures && feat->iffeatures[u]; ++u) {
        FREE_STRING(ctx, feat->iffeatures[u]);
    }
    free(feat->iffeatures);
    FREE_STRING(ctx, feat->dsc);
    FREE_STRING(ctx, feat->ref);
    FREE_ARRAY(ctx, feat->exts, u, lysp_ext_instance_free);
}

static void
lysp_ident_free(struct ly_ctx *ctx, struct lysp_ident *ident)
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
    FREE_ARRAY(ctx, ident->exts, u, lysp_ext_instance_free);
}

static void
lysp_restr_free(struct ly_ctx *ctx, struct lysp_restr *restr)
{
    unsigned int u;
    FREE_STRING(ctx, restr->arg);
    FREE_STRING(ctx, restr->emsg);
    FREE_STRING(ctx, restr->eapptag);
    FREE_STRING(ctx, restr->dsc);
    FREE_STRING(ctx, restr->ref);
    FREE_ARRAY(ctx, restr->exts, u, lysp_ext_instance_free);
}

static void
lysp_type_enum_free(struct ly_ctx *ctx, struct lysp_type_enum *item)
{
    unsigned int u;
    FREE_STRING(ctx, item->name);
    FREE_STRING(ctx, item->dsc);
    FREE_STRING(ctx, item->ref);
    for (u = 0; item->iffeatures && item->iffeatures[u]; ++u) {
        FREE_STRING(ctx, item->iffeatures[u]);
    }
    free(item->iffeatures);
    FREE_ARRAY(ctx, item->exts, u, lysp_ext_instance_free);
}

static void
lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type)
{
    unsigned int u;
    FREE_STRING(ctx, type->name);
    FREE_MEMBER(ctx, type->range, lysp_restr_free);
    FREE_MEMBER(ctx, type->length, lysp_restr_free);
    FREE_ARRAY(ctx, type->patterns, u, lysp_restr_free);
    FREE_ARRAY(ctx, type->enums, u, lysp_type_enum_free);
    FREE_ARRAY(ctx, type->bits, u, lysp_type_enum_free);
    FREE_STRING(ctx, type->path);
    for (u = 0; type->bases && type->bases[u]; ++u) {
        FREE_STRING(ctx, type->bases[u]);
    }
    free(type->bases);
    FREE_ARRAY(ctx, type->types, u, lysp_type_free);
    FREE_ARRAY(ctx, type->exts, u, lysp_ext_instance_free);
}

static void
lysp_tpdf_free(struct ly_ctx *ctx, struct lysp_tpdf *tpdf)
{
    unsigned int u;
    FREE_STRING(ctx, tpdf->name);
    FREE_STRING(ctx, tpdf->units);
    FREE_STRING(ctx, tpdf->dflt);
    FREE_STRING(ctx, tpdf->dsc);
    FREE_STRING(ctx, tpdf->ref);
    FREE_ARRAY(ctx, tpdf->exts, u, lysp_ext_instance_free);
    lysp_type_free(ctx, &tpdf->type);
}

static void
lysp_action_inout_free(struct ly_ctx *ctx, struct lysp_action_inout *inout)
{
    unsigned int u;
    struct lysp_node *node, *next;

    FREE_ARRAY(ctx, inout->musts, u, lysp_restr_free);
    FREE_ARRAY(ctx, inout->typedefs, u, lysp_tpdf_free);
    FREE_ARRAY(ctx, inout->groupings, u, lysp_grp_free);
    LY_LIST_FOR_SAFE(inout->data, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, inout->exts, u, lysp_ext_instance_free);

}

static void
lysp_action_free(struct ly_ctx *ctx, struct lysp_action *action)
{
    unsigned int u;
    FREE_STRING(ctx, action->name);
    FREE_STRING(ctx, action->dsc);
    FREE_STRING(ctx, action->ref);
    for (u = 0; action->iffeatures && action->iffeatures[u]; ++u) {
        FREE_STRING(ctx, action->iffeatures[u]);
    }
    free(action->iffeatures);
    FREE_ARRAY(ctx, action->typedefs, u, lysp_tpdf_free);
    FREE_ARRAY(ctx, action->groupings, u, lysp_grp_free);
    FREE_MEMBER(ctx, action->input, lysp_action_inout_free);
    FREE_MEMBER(ctx, action->output, lysp_action_inout_free);
    FREE_ARRAY(ctx, action->exts, u, lysp_ext_instance_free);
}

static void
lysp_notif_free(struct ly_ctx *ctx, struct lysp_notif *notif)
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
    FREE_ARRAY(ctx, notif->musts, u, lysp_restr_free);
    FREE_ARRAY(ctx, notif->typedefs, u, lysp_tpdf_free);
    FREE_ARRAY(ctx, notif->groupings, u, lysp_grp_free);
    LY_LIST_FOR_SAFE(notif->data, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, notif->exts, u, lysp_ext_instance_free);
}

static void
lysp_grp_free(struct ly_ctx *ctx, struct lysp_grp *grp)
{
    unsigned int u;
    struct lysp_node *node, *next;

    FREE_STRING(ctx, grp->name);
    FREE_STRING(ctx, grp->dsc);
    FREE_STRING(ctx, grp->ref);
    FREE_ARRAY(ctx, grp->typedefs, u, lysp_tpdf_free);
    FREE_ARRAY(ctx, grp->groupings, u, lysp_grp_free);
    LY_LIST_FOR_SAFE(grp->data, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, grp->actions, u, lysp_action_free);
    FREE_ARRAY(ctx, grp->notifs, u, lysp_notif_free);
    FREE_ARRAY(ctx, grp->exts, u, lysp_ext_instance_free);
}

static void
lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when)
{
    unsigned int u;
    FREE_STRING(ctx, when->cond);
    FREE_STRING(ctx, when->dsc);
    FREE_STRING(ctx, when->ref);
    FREE_ARRAY(ctx, when->exts, u, lysp_ext_instance_free);
}

static void
lysp_augment_free(struct ly_ctx *ctx, struct lysp_augment *augment)
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
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, augment->actions, u, lysp_action_free);
    FREE_ARRAY(ctx, augment->notifs, u, lysp_notif_free);
    FREE_ARRAY(ctx, augment->exts, u, lysp_ext_instance_free);
}

static void
lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d)
{
    unsigned int u;
    struct lysp_deviate_add *add = (struct lysp_deviate_add*)d;
    struct lysp_deviate_rpl *rpl = (struct lysp_deviate_rpl*)d;

    FREE_ARRAY(ctx, d->exts, u, lysp_ext_instance_free);
    switch(d->mod) {
    case LYS_DEV_NOT_SUPPORTED:
        /* nothing to do */
        break;
    case LYS_DEV_ADD:
    case LYS_DEV_DELETE: /* compatible for dynamically allocated data */
        FREE_STRING(ctx, add->units);
        FREE_ARRAY(ctx, add->musts, u, lysp_restr_free);
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
lysp_deviation_free(struct ly_ctx *ctx, struct lysp_deviation *dev)
{
    unsigned int u;
    struct lysp_deviate *next, *iter;

    FREE_STRING(ctx, dev->nodeid);
    FREE_STRING(ctx, dev->dsc);
    FREE_STRING(ctx, dev->ref);
    LY_LIST_FOR_SAFE(dev->deviates, next, iter) {
        lysp_deviate_free(ctx, iter);
        free(iter);
    }
    FREE_ARRAY(ctx, dev->exts, u, lysp_ext_instance_free);
}

static void
lysp_refine_free(struct ly_ctx *ctx, struct lysp_refine *ref)
{
    unsigned int u;
    FREE_STRING(ctx, ref->nodeid);
    FREE_STRING(ctx, ref->dsc);
    FREE_STRING(ctx, ref->ref);
    for (u = 0; ref->iffeatures && ref->iffeatures[u]; ++u) {
        FREE_STRING(ctx, ref->iffeatures[u]);
    }
    free(ref->iffeatures);
    FREE_ARRAY(ctx, ref->musts, u, lysp_restr_free);
    FREE_STRING(ctx, ref->presence);
    for (u = 0; ref->dflts && ref->dflts[u]; ++u) {
        FREE_STRING(ctx, ref->dflts[u]);
    }
    free(ref->dflts);
    FREE_ARRAY(ctx, ref->exts, u, lysp_ext_instance_free);
}

static void
lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node)
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
    FREE_ARRAY(ctx, node->exts, u, lysp_ext_instance_free);

    switch(node->nodetype) {
    case LYS_CONTAINER:
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->musts, u, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_container*)node)->presence);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->typedefs, u, lysp_tpdf_free);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->groupings, u, lysp_grp_free);
        LY_LIST_FOR_SAFE(((struct lysp_node_container*)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->actions, u, lysp_action_free);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->notifs, u, lysp_notif_free);
        break;
    case LYS_LEAF:
        FREE_ARRAY(ctx, ((struct lysp_node_leaf*)node)->musts, u, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaf*)node)->type);
        FREE_STRING(ctx, ((struct lysp_node_leaf*)node)->units);
        FREE_STRING(ctx, ((struct lysp_node_leaf*)node)->dflt);
        break;
    case LYS_LEAFLIST:
        FREE_ARRAY(ctx, ((struct lysp_node_leaflist*)node)->musts, u, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaflist*)node)->type);
        FREE_STRING(ctx, ((struct lysp_node_leaflist*)node)->units);
        for (u = 0; ((struct lysp_node_leaflist*)node)->dflts && ((struct lysp_node_leaflist*)node)->dflts[u]; ++u) {
            FREE_STRING(ctx, ((struct lysp_node_leaflist*)node)->dflts[u]);
        }
        free(((struct lysp_node_leaflist*)node)->dflts);
        break;
    case LYS_LIST:
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->musts, u, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_list*)node)->key);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->typedefs, u, lysp_tpdf_free);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->groupings, u, lysp_grp_free);
        LY_LIST_FOR_SAFE(((struct lysp_node_list*)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->actions, u, lysp_action_free);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->notifs, u, lysp_notif_free);
        for (u = 0; ((struct lysp_node_list*)node)->uniques && ((struct lysp_node_list*)node)->uniques[u]; ++u) {
            FREE_STRING(ctx, ((struct lysp_node_list*)node)->uniques[u]);
        }
        free(((struct lysp_node_list*)node)->uniques);
        break;
    case LYS_CHOICE:
        LY_LIST_FOR_SAFE(((struct lysp_node_choice*)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_STRING(ctx, ((struct lysp_node_choice*)node)->dflt);
        break;
    case LYS_CASE:
        LY_LIST_FOR_SAFE(((struct lysp_node_case*)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        FREE_ARRAY(ctx, ((struct lysp_node_anydata*)node)->musts, u, lysp_restr_free);
        break;
    case LYS_USES:
        FREE_ARRAY(ctx, ((struct lysp_node_uses*)node)->refines, u, lysp_refine_free);
        FREE_ARRAY(ctx, ((struct lysp_node_uses*)node)->augments, u, lysp_augment_free);
        break;
    default:
        LOGINT(ctx);
    }

    free(node);
}

API void
lysp_module_free(struct lysp_module *module)
{
    struct ly_ctx *ctx;
    unsigned int u;
    struct lysp_node *node, *next;

    LY_CHECK_ARG_RET(NULL, module,);
    ctx = module->ctx;

    FREE_STRING(ctx, module->name);
    FREE_STRING(ctx, module->filepath);
    FREE_STRING(ctx, module->ns);  /* or belongs-to */
    FREE_STRING(ctx, module->prefix);

    FREE_ARRAY(ctx, module->imports, u, lysp_import_free);
    FREE_ARRAY(ctx, module->includes, u, lysp_include_free);

    FREE_STRING(ctx, module->org);
    FREE_STRING(ctx, module->contact);
    FREE_STRING(ctx, module->dsc);
    FREE_STRING(ctx, module->ref);

    FREE_ARRAY(ctx, module->revs, u, lysp_revision_free);
    FREE_ARRAY(ctx, module->extensions, u, lysp_ext_free);
    FREE_ARRAY(ctx, module->features, u, lysp_feature_free);
    FREE_ARRAY(ctx, module->identities, u, lysp_ident_free);
    FREE_ARRAY(ctx, module->typedefs, u, lysp_tpdf_free);
    FREE_ARRAY(ctx, module->groupings, u, lysp_grp_free);
    LY_LIST_FOR_SAFE(module->data, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, module->augments, u, lysp_augment_free);
    FREE_ARRAY(ctx, module->rpcs, u, lysp_action_free);
    FREE_ARRAY(ctx, module->notifs, u, lysp_notif_free);
    FREE_ARRAY(ctx, module->deviations, u, lysp_deviation_free);
    FREE_ARRAY(ctx, module->exts, u, lysp_ext_instance_free);


    free(module);
}

LY_ERR
lysp_check_prefix(struct ly_parser_ctx *ctx, struct lysp_module *module, const char **value)
{
    unsigned int u;

    if (module->prefix && &module->prefix != value && !strcmp(module->prefix, *value)) {
        LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE,
               "Prefix \"%s\" already used as module prefix.", *value);
        return LY_EEXIST;
    }
    if (module->imports) {
        LY_ARRAY_FOR(module->imports, u) {
            if (module->imports[u].prefix && &module->imports[u].prefix != value && !strcmp(module->imports[u].prefix, *value)) {
                LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE,
                       "Prefix \"%s\" already used to import \"%s\" module.", *value, module->imports[u].name);
                return LY_EEXIST;
            }
        }
    }
    return LY_SUCCESS;
}
