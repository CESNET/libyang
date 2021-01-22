/**
 * @file tree_schema_free.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Freeing functions for schema tree structures.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>

#include "common.h"
#include "compat.h"
#include "log.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

void lysp_grp_free(struct ly_ctx *ctx, struct lysp_node_grp *grp);
void lysc_extension_free(struct ly_ctx *ctx, struct lysc_ext **ext);
static void lysc_node_free_(struct ly_ctx *ctx, struct lysc_node *node);

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

void
lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext)
{
    struct lysp_stmt *stmt, *next;

    FREE_STRING(ctx, ext->name);
    FREE_STRING(ctx, ext->argument);

    LY_LIST_FOR_SAFE(ext->child, next, stmt) {
        lysp_stmt_free(ctx, stmt);
    }
}

void
lysp_import_free(struct ly_ctx *ctx, struct lysp_import *import)
{
    /* imported module is freed directly from the context's list */
    FREE_STRING(ctx, import->name);
    FREE_STRING(ctx, import->prefix);
    FREE_STRING(ctx, import->dsc);
    FREE_STRING(ctx, import->ref);
    FREE_ARRAY(ctx, import->exts, lysp_ext_instance_free);
}

/**
 * @brief Common function to erase include record in main module and submodule.
 *
 * There is a difference since the main module is expected to have the complete list if the included submodules and
 * the parsed submodule is shared with any include in a submodule. Therefore, the referenced submodules in the include
 * record are freed only from main module's records.
 *
 * @param[in] ctx libyang context
 * @param[in] include The include record to be erased, the record itself is not freed.
 * @param[in] main_module Flag to get know if the include record is placed in main module so also the referenced submodule
 * is supposed to be freed.
 */
static void
lysp_include_free_(struct ly_ctx *ctx, struct lysp_include *include, ly_bool main_module)
{
    if (main_module && include->submodule) {
        lysp_module_free((struct lysp_module *)include->submodule);
    }
    FREE_STRING(ctx, include->name);
    FREE_STRING(ctx, include->dsc);
    FREE_STRING(ctx, include->ref);
    FREE_ARRAY(ctx, include->exts, lysp_ext_instance_free);
}

void
lysp_include_free_submodule(struct ly_ctx *ctx, struct lysp_include *include)
{
    return lysp_include_free_(ctx, include, 0);
}

void
lysp_include_free(struct ly_ctx *ctx, struct lysp_include *include)
{
    return lysp_include_free_(ctx, include, 1);
}

void
lysp_revision_free(struct ly_ctx *ctx, struct lysp_revision *rev)
{
    FREE_STRING(ctx, rev->dsc);
    FREE_STRING(ctx, rev->ref);
    FREE_ARRAY(ctx, rev->exts, lysp_ext_instance_free);
}

void
lysp_ext_free(struct ly_ctx *ctx, struct lysp_ext *ext)
{
    FREE_STRING(ctx, ext->name);
    FREE_STRING(ctx, ext->argument);
    FREE_STRING(ctx, ext->dsc);
    FREE_STRING(ctx, ext->ref);
    FREE_ARRAY(ctx, ext->exts, lysp_ext_instance_free);
    if (ext->compiled) {
        lysc_extension_free(ctx, &ext->compiled);
    }
}

void
lysp_feature_free(struct ly_ctx *ctx, struct lysp_feature *feat)
{
    FREE_STRING(ctx, feat->name);
    FREE_ARRAY(ctx, feat->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, feat->iffeatures_c, lysc_iffeature_free);
    LY_ARRAY_FREE(feat->depfeatures);
    FREE_STRING(ctx, feat->dsc);
    FREE_STRING(ctx, feat->ref);
    FREE_ARRAY(ctx, feat->exts, lysp_ext_instance_free);
}

void
lysp_ident_free(struct ly_ctx *ctx, struct lysp_ident *ident)
{
    FREE_STRING(ctx, ident->name);
    FREE_ARRAY(ctx, ident->iffeatures, lysp_qname_free);
    FREE_STRINGS(ctx, ident->bases);
    FREE_STRING(ctx, ident->dsc);
    FREE_STRING(ctx, ident->ref);
    FREE_ARRAY(ctx, ident->exts, lysp_ext_instance_free);
}

void
lysp_restr_free(struct ly_ctx *ctx, struct lysp_restr *restr)
{
    FREE_STRING(ctx, restr->arg.str);
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
    FREE_ARRAY(ctx, item->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, item->exts, lysp_ext_instance_free);
}

void lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type);

void
lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type)
{
    FREE_STRING(ctx, type->name);
    FREE_MEMBER(ctx, type->range, lysp_restr_free);
    FREE_MEMBER(ctx, type->length, lysp_restr_free);
    FREE_ARRAY(ctx, type->patterns, lysp_restr_free);
    FREE_ARRAY(ctx, type->enums, lysp_type_enum_free);
    FREE_ARRAY(ctx, type->bits, lysp_type_enum_free);
    lyxp_expr_free(ctx, type->path);
    FREE_STRINGS(ctx, type->bases);
    FREE_ARRAY(ctx, type->types, lysp_type_free);
    FREE_ARRAY(ctx, type->exts, lysp_ext_instance_free);
    if (type->compiled) {
        lysc_type_free(ctx, type->compiled);
    }
}

void
lysp_tpdf_free(struct ly_ctx *ctx, struct lysp_tpdf *tpdf)
{
    FREE_STRING(ctx, tpdf->name);
    FREE_STRING(ctx, tpdf->units);
    FREE_STRING(ctx, tpdf->dflt.str);
    FREE_STRING(ctx, tpdf->dsc);
    FREE_STRING(ctx, tpdf->ref);
    FREE_ARRAY(ctx, tpdf->exts, lysp_ext_instance_free);

    lysp_type_free(ctx, &tpdf->type);

}

void
lysp_grp_free(struct ly_ctx *ctx, struct lysp_node_grp *grp)
{
    struct lysp_node *node, *next;

    FREE_ARRAY(ctx, grp->typedefs, lysp_tpdf_free);
    LY_LIST_FOR_SAFE((struct lysp_node *)grp->groupings, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE(grp->data, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)grp->actions, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)grp->notifs, next, node) {
        lysp_node_free(ctx, node);
    }
}

void
lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when)
{
    FREE_STRING(ctx, when->cond);
    FREE_STRING(ctx, when->dsc);
    FREE_STRING(ctx, when->ref);
    FREE_ARRAY(ctx, when->exts, lysp_ext_instance_free);
}

void
lysp_augment_free(struct ly_ctx *ctx, struct lysp_node_augment *augment)
{
    struct lysp_node *node, *next;

    LY_LIST_FOR_SAFE(augment->child, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)augment->actions, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)augment->notifs, next, node) {
        lysp_node_free(ctx, node);
    }
}

void
lysp_qname_free(struct ly_ctx *ctx, struct lysp_qname *qname)
{
    if (qname) {
        FREE_STRING(ctx, qname->str);
    }
}

void
lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d)
{
    struct lysp_deviate_add *add = (struct lysp_deviate_add *)d;
    struct lysp_deviate_rpl *rpl = (struct lysp_deviate_rpl *)d;

    FREE_ARRAY(ctx, d->exts, lysp_ext_instance_free);
    switch (d->mod) {
    case LYS_DEV_NOT_SUPPORTED:
        /* nothing to do */
        break;
    case LYS_DEV_ADD:
    case LYS_DEV_DELETE: /* compatible for dynamically allocated data */
        FREE_STRING(ctx, add->units);
        FREE_ARRAY(ctx, add->musts, lysp_restr_free);
        FREE_ARRAY(ctx, add->uniques, lysp_qname_free);
        FREE_ARRAY(ctx, add->dflts, lysp_qname_free);
        break;
    case LYS_DEV_REPLACE:
        FREE_MEMBER(ctx, rpl->type, lysp_type_free);
        FREE_STRING(ctx, rpl->units);
        lysp_qname_free(ctx, &rpl->dflt);
        break;
    default:
        LOGINT(ctx);
        break;
    }
}

void
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

void
lysp_refine_free(struct ly_ctx *ctx, struct lysp_refine *ref)
{
    FREE_STRING(ctx, ref->nodeid);
    FREE_STRING(ctx, ref->dsc);
    FREE_STRING(ctx, ref->ref);
    FREE_ARRAY(ctx, ref->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, ref->musts, lysp_restr_free);
    FREE_STRING(ctx, ref->presence);
    FREE_ARRAY(ctx, ref->dflts, lysp_qname_free);
    FREE_ARRAY(ctx, ref->exts, lysp_ext_instance_free);
}

void
lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node)
{
    struct lysp_node *child, *next;

    FREE_STRING(ctx, node->name);
    FREE_STRING(ctx, node->dsc);
    FREE_STRING(ctx, node->ref);
    FREE_MEMBER(ctx, node->when, lysp_when_free);
    FREE_ARRAY(ctx, node->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, node->exts, lysp_ext_instance_free);

    switch (node->nodetype) {
    case LYS_CONTAINER:
        FREE_ARRAY(ctx, ((struct lysp_node_container *)node)->musts, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_container *)node)->presence);
        FREE_ARRAY(ctx, ((struct lysp_node_container *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_container *)node)->groupings, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_container *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_container *)node)->actions, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_container *)node)->notifs, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_LEAF:
        FREE_ARRAY(ctx, ((struct lysp_node_leaf *)node)->musts, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaf *)node)->type);
        FREE_STRING(ctx, ((struct lysp_node_leaf *)node)->units);
        FREE_STRING(ctx, ((struct lysp_node_leaf *)node)->dflt.str);
        break;
    case LYS_LEAFLIST:
        FREE_ARRAY(ctx, ((struct lysp_node_leaflist *)node)->musts, lysp_restr_free);
        lysp_type_free(ctx, &((struct lysp_node_leaflist *)node)->type);
        FREE_STRING(ctx, ((struct lysp_node_leaflist *)node)->units);
        FREE_ARRAY(ctx, ((struct lysp_node_leaflist *)node)->dflts, lysp_qname_free);
        break;
    case LYS_LIST:
        FREE_ARRAY(ctx, ((struct lysp_node_list *)node)->musts, lysp_restr_free);
        FREE_STRING(ctx, ((struct lysp_node_list *)node)->key);
        FREE_ARRAY(ctx, ((struct lysp_node_list *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_list *)node)->groupings, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_list *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_list *)node)->actions, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_list *)node)->notifs, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_list *)node)->uniques, lysp_qname_free);
        break;
    case LYS_CHOICE:
        LY_LIST_FOR_SAFE(((struct lysp_node_choice *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_STRING(ctx, ((struct lysp_node_choice *)node)->dflt.str);
        break;
    case LYS_CASE:
        LY_LIST_FOR_SAFE(((struct lysp_node_case *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        FREE_ARRAY(ctx, ((struct lysp_node_anydata *)node)->musts, lysp_restr_free);
        break;
    case LYS_USES:
        FREE_ARRAY(ctx, ((struct lysp_node_uses *)node)->refines, lysp_refine_free);
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_uses *)node)->augments, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_RPC:
    case LYS_ACTION:
        FREE_ARRAY(ctx, ((struct lysp_node_action *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_action *)node)->groupings, next, child) {
            lysp_node_free(ctx, child);
        }
        if (((struct lysp_node_action *)node)->input.nodetype) {
            lysp_node_free(ctx, (struct lysp_node *)&((struct lysp_node_action *)node)->input);
        }
        if (((struct lysp_node_action *)node)->output.nodetype) {
            lysp_node_free(ctx, (struct lysp_node *)&((struct lysp_node_action *)node)->output);
        }
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        FREE_ARRAY(ctx, ((struct lysp_node_action_inout *)node)->musts, lysp_restr_free);
        FREE_ARRAY(ctx, ((struct lysp_node_action_inout *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_action_inout *)node)->groupings, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_action_inout *)node)->data, next, child) {
            lysp_node_free(ctx, child);
        }
        /* do not free the node, it is never standalone but part of the action node */
        return;
    case LYS_NOTIF:
        FREE_ARRAY(ctx, ((struct lysp_node_notif *)node)->musts, lysp_restr_free);
        FREE_ARRAY(ctx, ((struct lysp_node_notif *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE((struct lysp_node *)((struct lysp_node_notif *)node)->groupings, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_notif *)node)->data, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_GROUPING:
        lysp_grp_free(ctx, (struct lysp_node_grp *)node);
        break;
    case LYS_AUGMENT:
        lysp_augment_free(ctx, (struct lysp_node_augment *)node);
        break;
    default:
        LOGINT(ctx);
    }

    free(node);
}

void
lysp_module_free(struct lysp_module *module)
{
    struct ly_ctx *ctx;
    struct lysp_node *node, *next;

    if (!module) {
        return;
    }
    ctx = module->mod->ctx;

    FREE_ARRAY(ctx, module->imports, lysp_import_free);
    FREE_ARRAY(ctx, module->includes, module->is_submod ? lysp_include_free_submodule : lysp_include_free);

    FREE_ARRAY(ctx, module->revs, lysp_revision_free);
    FREE_ARRAY(ctx, module->extensions, lysp_ext_free);
    FREE_ARRAY(ctx, module->features, lysp_feature_free);
    FREE_ARRAY(ctx, module->identities, lysp_ident_free);
    FREE_ARRAY(ctx, module->typedefs, lysp_tpdf_free);
    LY_LIST_FOR_SAFE((struct lysp_node *)module->groupings, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE(module->data, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)module->augments, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)module->rpcs, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)module->notifs, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, module->deviations, lysp_deviation_free);
    FREE_ARRAY(ctx, module->exts, lysp_ext_instance_free);

    if (module->is_submod) {
        struct lysp_submodule *submod = (struct lysp_submodule *)module;

        FREE_STRING(ctx, submod->name);
        FREE_STRING(ctx, submod->filepath);
        FREE_STRING(ctx, submod->prefix);
        FREE_STRING(ctx, submod->org);
        FREE_STRING(ctx, submod->contact);
        FREE_STRING(ctx, submod->dsc);
        FREE_STRING(ctx, submod->ref);
    }

    free(module);
}

void
lysc_extension_free(struct ly_ctx *ctx, struct lysc_ext **ext)
{
    if (--(*ext)->refcount) {
        return;
    }
    FREE_STRING(ctx, (*ext)->name);
    FREE_STRING(ctx, (*ext)->argument);
    FREE_ARRAY(ctx, (*ext)->exts, lysc_ext_instance_free);
    free(*ext);
}

void
lysc_ext_instance_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    if (ext->def && ext->def->plugin && ext->def->plugin->free) {
        ext->def->plugin->free(ctx, ext);
    }
    if (ext->def) {
        lysc_extension_free(ctx, &ext->def);
    }
    FREE_STRING(ctx, ext->argument);
    FREE_ARRAY(ctx, ext->exts, lysc_ext_instance_free);
}

void
lysc_iffeature_free(struct ly_ctx *UNUSED(ctx), struct lysc_iffeature *iff)
{
    LY_ARRAY_FREE(iff->features);
    free(iff->expr);
}

static void
lysc_when_free(struct ly_ctx *ctx, struct lysc_when **w)
{
    if (--(*w)->refcount) {
        return;
    }
    lyxp_expr_free(ctx, (*w)->cond);
    lysc_prefixes_free((*w)->prefixes);
    FREE_STRING(ctx, (*w)->dsc);
    FREE_STRING(ctx, (*w)->ref);
    FREE_ARRAY(ctx, (*w)->exts, lysc_ext_instance_free);
    free(*w);
}

void
lysc_must_free(struct ly_ctx *ctx, struct lysc_must *must)
{
    lyxp_expr_free(ctx, must->cond);
    lysc_prefixes_free(must->prefixes);
    FREE_STRING(ctx, must->emsg);
    FREE_STRING(ctx, must->eapptag);
    FREE_STRING(ctx, must->dsc);
    FREE_STRING(ctx, must->ref);
    FREE_ARRAY(ctx, must->exts, lysc_ext_instance_free);
}

void
lysc_ident_free(struct ly_ctx *ctx, struct lysc_ident *ident)
{
    FREE_STRING(ctx, ident->name);
    FREE_STRING(ctx, ident->dsc);
    FREE_STRING(ctx, ident->ref);
    LY_ARRAY_FREE(ident->derived);
    FREE_ARRAY(ctx, ident->exts, lysc_ext_instance_free);
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
    pcre2_code_free((*pattern)->code);
    FREE_STRING(ctx, (*pattern)->expr);
    FREE_STRING(ctx, (*pattern)->eapptag);
    FREE_STRING(ctx, (*pattern)->emsg);
    FREE_STRING(ctx, (*pattern)->dsc);
    FREE_STRING(ctx, (*pattern)->ref);
    FREE_ARRAY(ctx, (*pattern)->exts, lysc_ext_instance_free);
    free(*pattern);
}

static void
lysc_enum_item_free(struct ly_ctx *ctx, struct lysc_type_bitenum_item *item)
{
    FREE_STRING(ctx, item->name);
    FREE_STRING(ctx, item->dsc);
    FREE_STRING(ctx, item->ref);
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

    switch (type->basetype) {
    case LY_TYPE_BINARY:
        FREE_MEMBER(ctx, ((struct lysc_type_bin *)type)->length, lysc_range_free);
        break;
    case LY_TYPE_BITS:
        FREE_ARRAY(ctx, (struct lysc_type_bitenum_item *)((struct lysc_type_bits *)type)->bits, lysc_enum_item_free);
        break;
    case LY_TYPE_DEC64:
        FREE_MEMBER(ctx, ((struct lysc_type_dec *)type)->range, lysc_range_free);
        break;
    case LY_TYPE_STRING:
        FREE_MEMBER(ctx, ((struct lysc_type_str *)type)->length, lysc_range_free);
        FREE_ARRAY(ctx, ((struct lysc_type_str *)type)->patterns, lysc_pattern_free);
        break;
    case LY_TYPE_ENUM:
        FREE_ARRAY(ctx, ((struct lysc_type_enum *)type)->enums, lysc_enum_item_free);
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        FREE_MEMBER(ctx, ((struct lysc_type_num *)type)->range, lysc_range_free);
        break;
    case LY_TYPE_IDENT:
        LY_ARRAY_FREE(((struct lysc_type_identityref *)type)->bases);
        break;
    case LY_TYPE_UNION:
        FREE_ARRAY(ctx, ((struct lysc_type_union *)type)->types, lysc_type2_free);
        break;
    case LY_TYPE_LEAFREF:
        lyxp_expr_free(ctx, ((struct lysc_type_leafref *)type)->path);
        lysc_prefixes_free(((struct lysc_type_leafref *)type)->prefixes);
        break;
    case LY_TYPE_INST:
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
    case LY_TYPE_UNKNOWN:
        /* nothing to do */
        break;
    }

    FREE_ARRAY(ctx, type->exts, lysc_ext_instance_free);
    free(type);
}

void
lysc_node_action_inout_free(struct ly_ctx *ctx, struct lysc_node_action_inout *inout)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, inout->musts, lysc_must_free);
    LY_LIST_FOR_SAFE(inout->data, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

void
lysc_node_action_free(struct ly_ctx *ctx, struct lysc_node_action *action)
{
    if (action->input.nodetype) {
        lysc_node_free_(ctx, (struct lysc_node *)&action->input);
    }
    if (action->output.nodetype) {
        lysc_node_free_(ctx, (struct lysc_node *)&action->output);
    }
}

void
lysc_node_notif_free(struct ly_ctx *ctx, struct lysc_node_notif *notif)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, notif->musts, lysc_must_free);
    LY_LIST_FOR_SAFE(notif->data, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

void
lysc_node_container_free(struct ly_ctx *ctx, struct lysc_node_container *node)
{
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)node->actions, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)node->notifs, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
}

static void
lysc_node_leaf_free(struct ly_ctx *ctx, struct lysc_node_leaf *node)
{
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    FREE_STRING(ctx, node->units);
    if (node->dflt) {
        node->dflt->realtype->plugin->free(ctx, node->dflt);
        lysc_type_free(ctx, (struct lysc_type *)node->dflt->realtype);
        free(node->dflt);
    }
}

static void
lysc_node_leaflist_free(struct ly_ctx *ctx, struct lysc_node_leaflist *node)
{
    LY_ARRAY_COUNT_TYPE u;

    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    FREE_STRING(ctx, node->units);
    LY_ARRAY_FOR(node->dflts, u) {
        node->dflts[u]->realtype->plugin->free(ctx, node->dflts[u]);
        lysc_type_free(ctx, (struct lysc_type *)node->dflts[u]->realtype);
        free(node->dflts[u]);
    }
    LY_ARRAY_FREE(node->dflts);
}

static void
lysc_node_list_free(struct ly_ctx *ctx, struct lysc_node_list *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    FREE_ARRAY(ctx, node->musts, lysc_must_free);

    LY_ARRAY_FOR(node->uniques, u) {
        LY_ARRAY_FREE(node->uniques[u]);
    }
    LY_ARRAY_FREE(node->uniques);

    LY_LIST_FOR_SAFE((struct lysc_node *)node->actions, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)node->notifs, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

static void
lysc_node_choice_free(struct ly_ctx *ctx, struct lysc_node_choice *node)
{
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE((struct lysc_node *)node->cases, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

static void
lysc_node_case_free(struct ly_ctx *ctx, struct lysc_node_case *node)
{
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

static void
lysc_node_anydata_free(struct ly_ctx *ctx, struct lysc_node_anydata *node)
{
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
}

static void
lysc_node_free_(struct ly_ctx *ctx, struct lysc_node *node)
{
    ly_bool inout = 0;

    /* common part */
    FREE_STRING(ctx, node->name);
    FREE_STRING(ctx, node->dsc);
    FREE_STRING(ctx, node->ref);

    /* nodetype-specific part */
    switch (node->nodetype) {
    case LYS_CONTAINER:
        lysc_node_container_free(ctx, (struct lysc_node_container *)node);
        break;
    case LYS_LEAF:
        lysc_node_leaf_free(ctx, (struct lysc_node_leaf *)node);
        break;
    case LYS_LEAFLIST:
        lysc_node_leaflist_free(ctx, (struct lysc_node_leaflist *)node);
        break;
    case LYS_LIST:
        lysc_node_list_free(ctx, (struct lysc_node_list *)node);
        break;
    case LYS_CHOICE:
        lysc_node_choice_free(ctx, (struct lysc_node_choice *)node);
        break;
    case LYS_CASE:
        lysc_node_case_free(ctx, (struct lysc_node_case *)node);
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        lysc_node_anydata_free(ctx, (struct lysc_node_anydata *)node);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        lysc_node_action_free(ctx, (struct lysc_node_action *)node);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        lysc_node_action_inout_free(ctx, (struct lysc_node_action_inout *)node);
        inout = 1;
        break;
    case LYS_NOTIF:
        lysc_node_notif_free(ctx, (struct lysc_node_notif *)node);
        break;
    default:
        LOGINT(ctx);
    }

    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->exts, lysc_ext_instance_free);

    if (!inout) {
        free(node);
    }
}

void
lysc_node_free(struct ly_ctx *ctx, struct lysc_node *node, ly_bool unlink)
{
    struct lysc_node *iter, **child_p;

    if (node->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        /* nothing to do - inouts are part of actions and cannot be unlinked/freed separately */
        return;
    }

    if (unlink) {
        /* unlink from siblings */
        if (node->prev->next) {
            node->prev->next = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            /* unlinking the last node */
            if (node->parent) {
                if (node->nodetype == LYS_ACTION) {
                    iter = (struct lysc_node *)lysc_node_actions(node->parent);
                } else if (node->nodetype == LYS_NOTIF) {
                    iter = (struct lysc_node *)lysc_node_notifs(node->parent);
                } else {
                    iter = (struct lysc_node *)lysc_node_children(node->parent, node->flags & LYS_CONFIG_MASK);
                }
                LY_CHECK_ERR_RET(!iter, LOGINT(ctx), );
            } else if (node->nodetype == LYS_RPC) {
                iter = (struct lysc_node *)node->module->compiled->rpcs;
            } else if (node->nodetype == LYS_NOTIF) {
                iter = (struct lysc_node *)node->module->compiled->notifs;
            } else {
                iter = node->module->compiled->data;
            }
            /* update the "last" pointer from the first node */
            iter->prev = node->prev;
        }

        /* unlink from parent */
        if (node->parent) {
            if (node->nodetype == LYS_ACTION) {
                child_p = (struct lysc_node **)lysc_node_actions_p(node->parent);
            } else if (node->nodetype == LYS_NOTIF) {
                child_p = (struct lysc_node **)lysc_node_notifs_p(node->parent);
            } else {
                child_p = lysc_node_children_p(node->parent, node->flags & LYS_CONFIG_MASK);
            }
        } else if (node->nodetype == LYS_RPC) {
            child_p = (struct lysc_node **)&node->module->compiled->rpcs;
        } else if (node->nodetype == LYS_NOTIF) {
            child_p = (struct lysc_node **)&node->module->compiled->notifs;
        } else {
            child_p = &node->module->compiled->data;
        }
        if (child_p && (*child_p == node)) {
            /* the node is the first child */
            *child_p = node->next;
        }
    }

    lysc_node_free_(ctx, node);
}

static void
lysc_module_free_(struct lysc_module *module)
{
    struct ly_ctx *ctx;
    struct lysc_node *node, *node_next;

    LY_CHECK_ARG_RET(NULL, module, );
    ctx = module->mod->ctx;

    LY_LIST_FOR_SAFE(module->data, node_next, node) {
        lysc_node_free_(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)module->rpcs, node_next, node) {
        lysc_node_free_(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)module->notifs, node_next, node) {
        lysc_node_free_(ctx, node);
    }
    FREE_ARRAY(ctx, module->exts, lysc_ext_instance_free);

    free(module);
}

void
lysc_module_free(struct lysc_module *module, void (*private_destructor)(const struct lysc_node *node, void *priv))
{
    /* TODO use the destructor, this just suppress warning about unused parameter */
    (void) private_destructor;

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
    FREE_ARRAY(module->ctx, module->identities, lysc_ident_free);
    lysp_module_free(module->parsed);

    LY_ARRAY_FREE(module->augmented_by);
    LY_ARRAY_FREE(module->deviated_by);

    FREE_STRING(module->ctx, module->name);
    FREE_STRING(module->ctx, module->revision);
    FREE_STRING(module->ctx, module->ns);
    FREE_STRING(module->ctx, module->prefix);
    FREE_STRING(module->ctx, module->filepath);
    FREE_STRING(module->ctx, module->org);
    FREE_STRING(module->ctx, module->contact);
    FREE_STRING(module->ctx, module->dsc);
    FREE_STRING(module->ctx, module->ref);

    free(module);
}

API void
lysc_extension_instance_free(struct ly_ctx *ctx, struct lysc_ext_substmt *substmts)
{
    for (LY_ARRAY_COUNT_TYPE u = 0; substmts[u].stmt; ++u) {
        if (!substmts[u].storage) {
            continue;
        }

        switch (substmts[u].stmt) {
        case LY_STMT_TYPE:
            if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                /* single item */
                struct lysc_type *type = *((struct lysc_type **)substmts[u].storage);
                if (!type) {
                    break;
                }
                lysc_type_free(ctx, type);
            } else {
                /* multiple items */
                struct lysc_type **types = *((struct lysc_type ***)substmts[u].storage);
                if (!types) {
                    break;
                }
                FREE_ARRAY(ctx, types, lysc_type2_free);
            }
            break;
        case LY_STMT_UNITS:
            if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                /* single item */
                const char *str = *((const char **)substmts[u].storage);
                if (!str) {
                    break;
                }
                FREE_STRING(ctx, str);
            } else {
                /* multiple items */
                const char **strs = *((const char ***)substmts[u].storage);
                if (!strs) {
                    break;
                }
                FREE_STRINGS(ctx, strs);
            }
            break;
        case LY_STMT_STATUS:
        case LY_STMT_CONFIG:
            /* nothing to do */
            break;
        case LY_STMT_IF_FEATURE: {
            struct lysc_iffeature *iff = *((struct lysc_iffeature **)substmts[u].storage);
            if (!iff) {
                break;
            }
            if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                /* single item */
                lysc_iffeature_free(ctx, iff);
                free(iff);
            } else {
                /* multiple items */
                FREE_ARRAY(ctx, iff, lysc_iffeature_free);
            }
            break;
        }

        /* TODO other statements */
        default:
            LOGINT(ctx);
        }
    }
}

void
yang_parser_ctx_free(struct lys_yang_parser_ctx *ctx)
{
    if (ctx) {
        free(ctx);
    }
}

void
yin_parser_ctx_free(struct lys_yin_parser_ctx *ctx)
{
    if (ctx) {
        lyxml_ctx_free(ctx->xmlctx);
        free(ctx);
    }
}
