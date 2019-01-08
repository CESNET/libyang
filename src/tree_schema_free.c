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

#include "common.h"

#include "libyang.h"
#include "context.h"
#include "tree_schema_internal.h"
#include "xpath.h"

#define FREE_ARRAY(CTX, ARRAY, FUNC) {uint64_t c__; LY_ARRAY_FOR(ARRAY, c__){FUNC(CTX, &(ARRAY)[c__]);}LY_ARRAY_FREE(ARRAY);}
#define FREE_MEMBER(CTX, MEMBER, FUNC) if (MEMBER) {FUNC(CTX, MEMBER);free(MEMBER);}
#define FREE_STRINGS(CTX, ARRAY) {uint64_t c__; LY_ARRAY_FOR(ARRAY, c__){FREE_STRING(CTX, ARRAY[c__]);}LY_ARRAY_FREE(ARRAY);}

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
    /* imported module is freed directly from the context's list */
    FREE_STRING(ctx, import->name);
    FREE_STRING(ctx, import->prefix);
    FREE_STRING(ctx, import->dsc);
    FREE_STRING(ctx, import->ref);
    FREE_ARRAY(ctx, import->exts, lysp_ext_instance_free);
}

static void
lysp_include_free(struct ly_ctx *ctx, struct lysp_include *include)
{
    if (include->submodule) {
        lysp_module_free(include->submodule);
    }
    FREE_STRING(ctx, include->name);
    FREE_STRING(ctx, include->dsc);
    FREE_STRING(ctx, include->ref);
    FREE_ARRAY(ctx, include->exts, lysp_ext_instance_free);
}

static void
lysp_revision_free(struct ly_ctx *ctx, struct lysp_revision *rev)
{
    FREE_STRING(ctx, rev->dsc);
    FREE_STRING(ctx, rev->ref);
    FREE_ARRAY(ctx, rev->exts, lysp_ext_instance_free);
}

static void
lysp_ext_free(struct ly_ctx *ctx, struct lysp_ext *ext)
{
    FREE_STRING(ctx, ext->name);
    FREE_STRING(ctx, ext->argument);
    FREE_STRING(ctx, ext->dsc);
    FREE_STRING(ctx, ext->ref);
    FREE_ARRAY(ctx, ext->exts, lysp_ext_instance_free);
}

static void
lysp_feature_free(struct ly_ctx *ctx, struct lysp_feature *feat)
{
    FREE_STRING(ctx, feat->name);
    FREE_STRINGS(ctx, feat->iffeatures);
    FREE_STRING(ctx, feat->dsc);
    FREE_STRING(ctx, feat->ref);
    FREE_ARRAY(ctx, feat->exts, lysp_ext_instance_free);
}

static void
lysp_ident_free(struct ly_ctx *ctx, struct lysp_ident *ident)
{
    FREE_STRING(ctx, ident->name);
    FREE_STRINGS(ctx, ident->iffeatures);
    FREE_STRINGS(ctx, ident->bases);
    FREE_STRING(ctx, ident->dsc);
    FREE_STRING(ctx, ident->ref);
    FREE_ARRAY(ctx, ident->exts, lysp_ext_instance_free);
}

static void
lysp_restr_free(struct ly_ctx *ctx, struct lysp_restr *restr)
{
    FREE_STRING(ctx, restr->arg);
    FREE_STRING(ctx, restr->emsg);
    FREE_STRING(ctx, restr->eapptag);
    FREE_STRING(ctx, restr->dsc);
    FREE_STRING(ctx, restr->ref);
    FREE_ARRAY(ctx, restr->exts, lysp_ext_instance_free);
}

static void
lysp_type_enum_free(struct ly_ctx *ctx, struct lysp_type_enum *item)
{
    FREE_STRING(ctx, item->name);
    FREE_STRING(ctx, item->dsc);
    FREE_STRING(ctx, item->ref);
    FREE_STRINGS(ctx, item->iffeatures);
    FREE_ARRAY(ctx, item->exts, lysp_ext_instance_free);
}

void lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type);
static void
lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type)
{
    FREE_STRING(ctx, type->name);
    FREE_MEMBER(ctx, type->range, lysp_restr_free);
    FREE_MEMBER(ctx, type->length, lysp_restr_free);
    FREE_ARRAY(ctx, type->patterns, lysp_restr_free);
    FREE_ARRAY(ctx, type->enums, lysp_type_enum_free);
    FREE_ARRAY(ctx, type->bits, lysp_type_enum_free);
    FREE_STRING(ctx, type->path);
    FREE_STRINGS(ctx, type->bases);
    FREE_ARRAY(ctx, type->types, lysp_type_free);
    FREE_ARRAY(ctx, type->exts, lysp_ext_instance_free);
    if (type->compiled) {
        lysc_type_free(ctx, type->compiled);
    }
}

static void
lysp_tpdf_free(struct ly_ctx *ctx, struct lysp_tpdf *tpdf)
{
    FREE_STRING(ctx, tpdf->name);
    FREE_STRING(ctx, tpdf->units);
    FREE_STRING(ctx, tpdf->dflt);
    FREE_STRING(ctx, tpdf->dsc);
    FREE_STRING(ctx, tpdf->ref);
    FREE_ARRAY(ctx, tpdf->exts, lysp_ext_instance_free);

    lysp_type_free(ctx, &tpdf->type);

}

static void
lysp_action_inout_free(struct ly_ctx *ctx, struct lysp_action_inout *inout)
{
    struct lysp_node *node, *next;

    FREE_ARRAY(ctx, inout->musts, lysp_restr_free);
    FREE_ARRAY(ctx, inout->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, inout->groupings, lysp_grp_free);
    LY_LIST_FOR_SAFE(inout->data, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, inout->exts, lysp_ext_instance_free);

}

static void
lysp_action_free(struct ly_ctx *ctx, struct lysp_action *action)
{
    FREE_STRING(ctx, action->name);
    FREE_STRING(ctx, action->dsc);
    FREE_STRING(ctx, action->ref);
    FREE_STRINGS(ctx, action->iffeatures);
    FREE_ARRAY(ctx, action->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, action->groupings, lysp_grp_free);
    FREE_MEMBER(ctx, action->input, lysp_action_inout_free);
    FREE_MEMBER(ctx, action->output, lysp_action_inout_free);
    FREE_ARRAY(ctx, action->exts, lysp_ext_instance_free);
}

static void
lysp_notif_free(struct ly_ctx *ctx, struct lysp_notif *notif)
{
    struct lysp_node *node, *next;

    FREE_STRING(ctx, notif->name);
    FREE_STRING(ctx, notif->dsc);
    FREE_STRING(ctx, notif->ref);
    FREE_STRINGS(ctx, notif->iffeatures);
    FREE_ARRAY(ctx, notif->musts, lysp_restr_free);
    FREE_ARRAY(ctx, notif->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, notif->groupings, lysp_grp_free);
    LY_LIST_FOR_SAFE(notif->data, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, notif->exts, lysp_ext_instance_free);
}

static void
lysp_grp_free(struct ly_ctx *ctx, struct lysp_grp *grp)
{
    struct lysp_node *node, *next;

    FREE_STRING(ctx, grp->name);
    FREE_STRING(ctx, grp->dsc);
    FREE_STRING(ctx, grp->ref);
    FREE_ARRAY(ctx, grp->typedefs, lysp_tpdf_free);
    FREE_ARRAY(ctx, grp->groupings, lysp_grp_free);
    LY_LIST_FOR_SAFE(grp->data, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, grp->actions, lysp_action_free);
    FREE_ARRAY(ctx, grp->notifs, lysp_notif_free);
    FREE_ARRAY(ctx, grp->exts, lysp_ext_instance_free);
}

static void
lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when)
{
    FREE_STRING(ctx, when->cond);
    FREE_STRING(ctx, when->dsc);
    FREE_STRING(ctx, when->ref);
    FREE_ARRAY(ctx, when->exts, lysp_ext_instance_free);
}

static void
lysp_augment_free(struct ly_ctx *ctx, struct lysp_augment *augment)
{
    struct lysp_node *node, *next;

    FREE_STRING(ctx, augment->nodeid);
    FREE_STRING(ctx, augment->dsc);
    FREE_STRING(ctx, augment->ref);
    FREE_MEMBER(ctx, augment->when, lysp_when_free);
    FREE_STRINGS(ctx, augment->iffeatures);
    LY_LIST_FOR_SAFE(augment->child, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, augment->actions, lysp_action_free);
    FREE_ARRAY(ctx, augment->notifs, lysp_notif_free);
    FREE_ARRAY(ctx, augment->exts, lysp_ext_instance_free);
}

static void
lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d)
{
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
        FREE_STRINGS(ctx, add->uniques);
        FREE_STRINGS(ctx, add->dflts);
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
    struct lysp_deviate *next, *iter;

    FREE_STRING(ctx, dev->nodeid);
    FREE_STRING(ctx, dev->dsc);
    FREE_STRING(ctx, dev->ref);
    LY_LIST_FOR_SAFE(dev->deviates, next, iter) {
        lysp_deviate_free(ctx, iter);
        free(iter);
    }
    FREE_ARRAY(ctx, dev->exts, lysp_ext_instance_free);
}

static void
lysp_refine_free(struct ly_ctx *ctx, struct lysp_refine *ref)
{
    FREE_STRING(ctx, ref->nodeid);
    FREE_STRING(ctx, ref->dsc);
    FREE_STRING(ctx, ref->ref);
    FREE_STRINGS(ctx, ref->iffeatures);
    FREE_ARRAY(ctx, ref->musts, lysp_restr_free);
    FREE_STRING(ctx, ref->presence);
    FREE_STRINGS(ctx, ref->dflts);
    FREE_ARRAY(ctx, ref->exts, lysp_ext_instance_free);
}

static void
lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node)
{
    struct lysp_node *child, *next;

    FREE_STRING(ctx, node->name);
    FREE_STRING(ctx, node->dsc);
    FREE_STRING(ctx, node->ref);
    FREE_MEMBER(ctx, node->when, lysp_when_free);
    FREE_STRINGS(ctx, node->iffeatures);
    FREE_ARRAY(ctx, node->exts, lysp_ext_instance_free);

    switch(node->nodetype) {
    case LYS_CONTAINER:
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->musts, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_container*)node)->presence);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->typedefs, lysp_tpdf_free);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->groupings, lysp_grp_free);
        LY_LIST_FOR_SAFE(((struct lysp_node_container*)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->actions, lysp_action_free);
        FREE_ARRAY(ctx, ((struct lysp_node_container*)node)->notifs, lysp_notif_free);
        break;
    case LYS_LEAF:
        FREE_ARRAY(ctx, ((struct lysp_node_leaf*)node)->musts, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaf*)node)->type);
        FREE_STRING(ctx, ((struct lysp_node_leaf*)node)->units);
        FREE_STRING(ctx, ((struct lysp_node_leaf*)node)->dflt);
        break;
    case LYS_LEAFLIST:
        FREE_ARRAY(ctx, ((struct lysp_node_leaflist*)node)->musts, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaflist*)node)->type);
        FREE_STRING(ctx, ((struct lysp_node_leaflist*)node)->units);
        FREE_STRINGS(ctx, ((struct lysp_node_leaflist*)node)->dflts);
        break;
    case LYS_LIST:
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->musts, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_list*)node)->key);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->typedefs, lysp_tpdf_free);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->groupings,  lysp_grp_free);
        LY_LIST_FOR_SAFE(((struct lysp_node_list*)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->actions, lysp_action_free);
        FREE_ARRAY(ctx, ((struct lysp_node_list*)node)->notifs, lysp_notif_free);
        FREE_STRINGS(ctx, ((struct lysp_node_list*)node)->uniques);
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

API void
lysp_module_free(struct lysp_module *module)
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
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, module->augments, lysp_augment_free);
    FREE_ARRAY(ctx, module->rpcs, lysp_action_free);
    FREE_ARRAY(ctx, module->notifs, lysp_notif_free);
    FREE_ARRAY(ctx, module->deviations, lysp_deviation_free);
    FREE_ARRAY(ctx, module->exts, lysp_ext_instance_free);

    free(module);
}

static void
lysc_ext_instance_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    FREE_STRING(ctx, ext->argument);
    FREE_ARRAY(ctx, ext->exts, lysc_ext_instance_free);
}

static void
lysc_iffeature_free(struct ly_ctx *UNUSED(ctx), struct lysc_iffeature *iff)
{
    LY_ARRAY_FREE(iff->features);
    free(iff->expr);
}

static void
lysc_when_free(struct ly_ctx *ctx, struct lysc_when *w)
{
    lyxp_expr_free(ctx, w->cond);
    FREE_STRING(ctx, w->dsc);
    FREE_STRING(ctx, w->ref);
    FREE_ARRAY(ctx, w->exts, lysc_ext_instance_free);
}

static void
lysc_must_free(struct ly_ctx *ctx, struct lysc_must *must)
{
    lyxp_expr_free(ctx, must->cond);
    FREE_STRING(ctx, must->emsg);
    FREE_STRING(ctx, must->eapptag);
    FREE_STRING(ctx, must->dsc);
    FREE_STRING(ctx, must->ref);
    FREE_ARRAY(ctx, must->exts, lysc_ext_instance_free);
}

static void
lysc_import_free(struct ly_ctx *ctx, struct lysc_import *import)
{
    /* imported module is freed directly from the context's list */
    FREE_STRING(ctx, import->prefix);
    FREE_ARRAY(ctx, import->exts, lysc_ext_instance_free);
}

static void
lysc_ident_free(struct ly_ctx *ctx, struct lysc_ident *ident)
{
    FREE_STRING(ctx, ident->name);
    FREE_STRING(ctx, ident->dsc);
    FREE_STRING(ctx, ident->ref);
    FREE_ARRAY(ctx, ident->iffeatures, lysc_iffeature_free);
    LY_ARRAY_FREE(ident->derived);
    FREE_ARRAY(ctx, ident->exts, lysc_ext_instance_free);
}

static void
lysc_feature_free(struct ly_ctx *ctx, struct lysc_feature *feat)
{
    FREE_STRING(ctx, feat->name);
    FREE_STRING(ctx, feat->dsc);
    FREE_STRING(ctx, feat->ref);
    FREE_ARRAY(ctx, feat->iffeatures, lysc_iffeature_free);
    LY_ARRAY_FREE(feat->depfeatures);
    FREE_ARRAY(ctx, feat->exts, lysc_ext_instance_free);
}

static void
lysc_range_free(struct ly_ctx *ctx, struct lysc_range *range)
{
    LY_ARRAY_FREE(range->parts);
    FREE_STRING(ctx, range->eapptag);
    FREE_STRING(ctx, range->emsg);
    FREE_STRING(ctx, range->dsc);
    FREE_STRING(ctx, range->ref);
    FREE_ARRAY(ctx, range->exts, lysc_ext_instance_free);
}

static void
lysc_pattern_free(struct ly_ctx *ctx, struct lysc_pattern **pattern)
{
    if (--(*pattern)->refcount) {
        return;
    }
    pcre_free((*pattern)->expr);
    pcre_free_study((*pattern)->expr_extra);
    FREE_STRING(ctx, (*pattern)->eapptag);
    FREE_STRING(ctx, (*pattern)->emsg);
    FREE_STRING(ctx, (*pattern)->dsc);
    FREE_STRING(ctx, (*pattern)->ref);
    FREE_ARRAY(ctx, (*pattern)->exts, lysc_ext_instance_free);
    free(*pattern);
}

static void
lysc_enum_item_free(struct ly_ctx *ctx, struct lysc_type_enum_item *item)
{
    FREE_STRING(ctx, item->name);
    FREE_STRING(ctx, item->dsc);
    FREE_STRING(ctx, item->ref);
    FREE_ARRAY(ctx, item->iffeatures, lysc_iffeature_free);
    FREE_ARRAY(ctx, item->exts, lysc_ext_instance_free);
}

static void
lysc_type2_free(struct ly_ctx *ctx, struct lysc_type **type)
{
    lysc_type_free(ctx, *type);
}
void
lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type)
{
    if (--type->refcount) {
        return;
    }
    switch(type->basetype) {
    case LY_TYPE_BINARY:
        FREE_MEMBER(ctx, ((struct lysc_type_bin*)type)->length, lysc_range_free);
        break;
    case LY_TYPE_BITS:
        FREE_ARRAY(ctx, (struct lysc_type_enum_item*)((struct lysc_type_bits*)type)->bits, lysc_enum_item_free);
        break;
    case LY_TYPE_DEC64:
        FREE_MEMBER(ctx, ((struct lysc_type_dec*)type)->range, lysc_range_free);
        break;
    case LY_TYPE_STRING:
        FREE_MEMBER(ctx, ((struct lysc_type_str*)type)->length, lysc_range_free);
        FREE_ARRAY(ctx, ((struct lysc_type_str*)type)->patterns, lysc_pattern_free);
        break;
    case LY_TYPE_ENUM:
        FREE_ARRAY(ctx, ((struct lysc_type_enum*)type)->enums, lysc_enum_item_free);
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        FREE_MEMBER(ctx, ((struct lysc_type_num*)type)->range, lysc_range_free);
        break;
    case LY_TYPE_IDENT:
        LY_ARRAY_FREE(((struct lysc_type_identityref*)type)->bases);
        break;
    case LY_TYPE_UNION:
        FREE_ARRAY(ctx, ((struct lysc_type_union*)type)->types, lysc_type2_free);
        break;
    case LY_TYPE_LEAFREF:
        FREE_STRING(ctx, ((struct lysc_type_leafref*)type)->path);
        break;
    case LY_TYPE_INST:
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
    case LY_TYPE_UNKNOWN:
        /* nothing to do */
        break;
    }
    FREE_ARRAY(ctx, type->exts, lysc_ext_instance_free);
    FREE_STRING(ctx, type->dflt);

    free(type);
}

static void
lysc_node_container_free(struct ly_ctx *ctx, struct lysc_node_container *node)
{
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free(ctx, child);
    }
    FREE_ARRAY(ctx, node->musts, lysc_must_free);

    /* TODO actions, notifs */
}

static void
lysc_node_leaf_free(struct ly_ctx *ctx, struct lysc_node_leaf *node)
{
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    FREE_STRING(ctx, node->units);
    FREE_STRING(ctx, node->dflt);
}

static void
lysc_node_leaflist_free(struct ly_ctx *ctx, struct lysc_node_leaflist *node)
{
    unsigned int u;

    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    FREE_STRING(ctx, node->units);
    LY_ARRAY_FOR(node->dflts, u) {
        lydict_remove(ctx, node->dflts[u]);
    }
    LY_ARRAY_FREE(node->dflts);
}

static void
lysc_node_list_free(struct ly_ctx *ctx, struct lysc_node_list *node)
{
    unsigned int u;
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free(ctx, child);
    }
    FREE_ARRAY(ctx, node->musts, lysc_must_free);

    LY_ARRAY_FREE(node->keys);
    LY_ARRAY_FOR(node->uniques, u) {
        LY_ARRAY_FREE(node->uniques[u]);
    }
    LY_ARRAY_FREE(node->uniques);

    /* TODO actions, notifs */
}

static void
lysc_node_choice_free(struct ly_ctx *ctx, struct lysc_node_choice *node)
{
    struct lysc_node *child, *child_next;

    if (node->cases) {
        LY_LIST_FOR_SAFE(node->cases->child, child_next, child) {
            lysc_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE((struct lysc_node*)node->cases, child_next, child) {
            lysc_node_free(ctx, child);
        }
    }
}

static void
lysc_node_anydata_free(struct ly_ctx *ctx, struct lysc_node_anydata *node)
{
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
}

void
lysc_node_free(struct ly_ctx *ctx, struct lysc_node *node)
{
    /* common part */
    FREE_STRING(ctx, node->name);
    FREE_STRING(ctx, node->dsc);
    FREE_STRING(ctx, node->ref);

    /* nodetype-specific part */
    switch(node->nodetype) {
    case LYS_CONTAINER:
        lysc_node_container_free(ctx, (struct lysc_node_container*)node);
        break;
    case LYS_LEAF:
        lysc_node_leaf_free(ctx, (struct lysc_node_leaf*)node);
        break;
    case LYS_LEAFLIST:
        lysc_node_leaflist_free(ctx, (struct lysc_node_leaflist*)node);
        break;
    case LYS_LIST:
        lysc_node_list_free(ctx, (struct lysc_node_list*)node);
        break;
    case LYS_CHOICE:
        lysc_node_choice_free(ctx, (struct lysc_node_choice*)node);
        break;
    case LYS_CASE:
        /* nothing specific */
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        lysc_node_anydata_free(ctx, (struct lysc_node_anydata*)node);
        break;
    default:
        LOGINT(ctx);
    }

    FREE_MEMBER(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->iffeatures, lysc_iffeature_free);
    FREE_ARRAY(ctx, node->exts, lysc_ext_instance_free);
    free(node);
}

static void
lysc_module_free_(struct lysc_module *module)
{
    struct ly_ctx *ctx;
    struct lysc_node *node, *node_next;

    LY_CHECK_ARG_RET(NULL, module,);
    ctx = module->ctx;

    FREE_STRING(ctx, module->name);
    FREE_STRING(ctx, module->ns);
    FREE_STRING(ctx, module->prefix);
    FREE_STRING(ctx, module->revision);
    FREE_STRING(ctx, module->org);
    FREE_STRING(ctx, module->contact);
    FREE_STRING(ctx, module->dsc);
    FREE_STRING(ctx, module->ref);

    FREE_ARRAY(ctx, module->imports, lysc_import_free);
    FREE_ARRAY(ctx, module->features, lysc_feature_free);
    FREE_ARRAY(ctx, module->identities, lysc_ident_free);

    LY_LIST_FOR_SAFE(module->data, node_next, node) {
        lysc_node_free(ctx, node);
    }

    FREE_ARRAY(ctx, module->exts, lysc_ext_instance_free);

    free(module);
}

void
lysc_module_free(struct lysc_module *module, void (*private_destructor)(const struct lysc_node *node, void *priv))
{
    if (module) {
        lysc_module_free_(module);
    }
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
