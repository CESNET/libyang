/**
 * @file printer/yang.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YANG printer for libyang data model structure
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"

#define INDENT ""
#define LEVEL (level*2)

static void yang_print_snode(struct lyout *out, int level, struct lys_node *node, int mask);

static const char*
get_module_import_prefix(struct lys_module *main_mod, struct lys_module *imp_mod)
{
    int i, j;

    for (i = 0; i < main_mod->imp_size; ++i) {
        if (main_mod->imp[i].module == imp_mod) {
            return main_mod->imp[i].prefix;
        }
    }

    for (j = 0; j < main_mod->inc_size; ++j) {
        for (i = 0; i < main_mod->inc[j].submodule->imp_size; ++i) {
            if (main_mod->inc[j].submodule->imp[i].module == imp_mod) {
                return main_mod->inc[j].submodule->imp[i].prefix;
            }
        }
    }

    return NULL;
}

static void
yang_print_text(struct lyout *out, int level, const char *name, const char *text)
{
    const char *s, *t;

    ly_print(out, "%*s%s\n", LEVEL, INDENT, name);
    level++;

    ly_print(out, "%*s\"", LEVEL, INDENT);
    t = text;
    while ((s = strchr(t, '\n'))) {
        ly_write(out, t, (s - t) + 1);
        t = s + 1;
        ly_print(out, "%*s", LEVEL, INDENT);
    }

    ly_print(out, "%s\";\n\n", t);
    level--;

}

static void
yang_print_nacmext(struct lyout *out, int level, struct lys_node *node, struct lys_module *module)
{
    int i, j;
    const char *prefix = NULL;

    if (node->nacm && (!node->parent || node->parent->nacm != node->nacm)) {
        /* locate ietf-netconf-acm module in imports */
        if (!strcmp(module->name, "ietf-netconf-acm")) {
            prefix = module->prefix;
        } else {
            /* search in imports */
            for (i = 0; i < module->imp_size; i++) {
                if (!strcmp(module->imp[i].module->name, "ietf-netconf-acm")) {
                    prefix = module->imp[i].prefix;
                    break;
                }
            }
            /* and in imports of includes */
            if (!prefix) {
                for (j = 0; j < module->inc_size; j++) {
                    for (i = 0; i < module->inc[j].submodule->imp_size; i++) {
                        if (!strcmp(module->inc[j].submodule->imp[i].module->name, "ietf-netconf-acm")) {
                            prefix = module->inc[j].submodule->imp[i].prefix;
                            break;
                        }
                    }
                }
            }
        }

        if ((node->nacm & LYS_NACM_DENYW) && (!node->parent || !(node->parent->nacm & LYS_NACM_DENYW))) {
            ly_print(out, "%*s%s:default-deny-write;\n", LEVEL, INDENT, prefix);
        }
        if ((node->nacm & LYS_NACM_DENYA) && (!node->parent || !(node->parent->nacm & LYS_NACM_DENYA))) {
            ly_print(out, "%*s%s:default-deny-all;\n", LEVEL, INDENT, prefix);
        }
    }
}

/*
 * Covers:
 * description, reference, status
 */
static void
yang_print_snode_common(struct lyout *out, int level, struct lys_node *node)
{
    if (node->flags & LYS_STATUS_CURR) {
        ly_print(out, "%*sstatus \"current\";\n", LEVEL, INDENT);
    } else if (node->flags & LYS_STATUS_DEPRC) {
        ly_print(out, "%*sstatus \"deprecated\";\n", LEVEL, INDENT);
    } else if (node->flags & LYS_STATUS_OBSLT) {
        ly_print(out, "%*sstatus \"obsolete\";\n", LEVEL, INDENT);
    }

    if (node->dsc) {
        yang_print_text(out, level, "description", node->dsc);
    }
    if (node->ref) {
        yang_print_text(out, level, "reference", node->ref);
    }
}

/*
 * Covers:
 * config, mandatory
 * description, reference, status
 */
static void
yang_print_snode_common2(struct lyout *out, int level, struct lys_node *node)
{
    if (!node->parent || (node->parent->flags & LYS_CONFIG_MASK) != (node->flags & LYS_CONFIG_MASK)) {
        /* print config only when it differs from the parent or in root */
        if (node->flags & LYS_CONFIG_W) {
            ly_print(out, "%*sconfig \"true\";\n", LEVEL, INDENT);
        } else if (node->flags & LYS_CONFIG_R) {
            ly_print(out, "%*sconfig \"false\";\n", LEVEL, INDENT);
        }
    }

    if (node->flags & LYS_MAND_TRUE) {
        ly_print(out, "%*smandatory \"true\";\n", LEVEL, INDENT);
    } else if (node->flags & LYS_MAND_FALSE) {
        ly_print(out, "%*smandatory \"false\";\n", LEVEL, INDENT);
    }

    yang_print_snode_common(out, level, node);
}

static void
yang_print_iffeature(struct lyout *out, int level, struct lys_module *module, struct lys_feature *feat)
{
    ly_print(out, "%*sif-feature ", LEVEL, INDENT);
    if ((feat->module != module) && !feat->module->type) {
        ly_print(out, "%s:", get_module_import_prefix(module, feat->module));
    }
    ly_print(out, "%s;\n", feat->name);
}

static void
yang_print_feature(struct lyout *out, int level, struct lys_feature *feat)
{
    int i;

    ly_print(out, "%*sfeature %s {\n", LEVEL, INDENT, feat->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)feat);
    for (i = 0; i < feat->features_size; ++i) {
        yang_print_iffeature(out, level, feat->module, feat->features[i]);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_restr(struct lyout *out, int level, struct lys_restr *restr)
{
    if (restr->dsc != NULL) {
        yang_print_text(out, level, "description", restr->dsc);
    }
    if (restr->ref != NULL) {
        yang_print_text(out, level, "reference", restr->ref);
    }
    if (restr->eapptag != NULL) {
        ly_print(out, "%*serror-app-tag \"%s\";\n", LEVEL, INDENT, restr->eapptag);
    }
    if (restr->emsg != NULL) {
        yang_print_text(out, level, "error-message", restr->emsg);
    }
}

static void
yang_print_when(struct lyout *out, int level, struct lys_module *module, struct lys_when *when)
{
    const char *xml_expr;

    xml_expr = transform_expr_json2xml(module, when->cond, NULL, NULL, NULL);
    if (!xml_expr) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*swhen \"%s\" {\n", LEVEL, INDENT, xml_expr);
    lydict_remove(module->ctx, xml_expr);

    level++;
    if (when->dsc) {
        yang_print_text(out, level, "description", when->dsc);
    }
    if (when->ref) {
        yang_print_text(out, level, "reference", when->ref);
    }
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_type(struct lyout *out, int level, struct lys_module *module, struct lys_type *type)
{
    int i;
    const char *str;

    if (type->module_name) {
        str = transform_expr_json2xml(module, type->module_name, NULL, NULL, NULL);
        ly_print(out, "%*stype %s:%s {\n", LEVEL, INDENT, str, type->der->name);
        lydict_remove(module->ctx, str);
    } else {
        ly_print(out, "%*stype %s {\n", LEVEL, INDENT, type->der->name);
    }
    level++;
    switch (type->base) {
    case LY_TYPE_BINARY:
        if (type->info.binary.length != NULL) {
            ly_print(out, "%*slength \"%s\" {\n", LEVEL, INDENT, type->info.binary.length->expr);
            yang_print_restr(out, level + 1, type->info.binary.length);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            ly_print(out, "%*sbit %s {\n", LEVEL, INDENT, type->info.bits.bit[i].name);
            level++;
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.bits.bit[i]);
            ly_print(out, "%*sposition %u;\n", LEVEL, INDENT, type->info.bits.bit[i].pos);
            level--;
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_DEC64:
        ly_print(out, "%*sfraction-digits %d;\n", LEVEL, INDENT, type->info.dec64.dig);
        if (type->info.dec64.range != NULL) {
            ly_print(out, "%*srange \"%s\" {\n", LEVEL, INDENT, type->info.dec64.range->expr);
            yang_print_restr(out, level + 1, type->info.dec64.range);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            ly_print(out, "%*senum %s {\n", LEVEL, INDENT, type->info.enums.enm[i].name);
            level++;
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.enums.enm[i]);
            ly_print(out, "%*svalue %d;\n", LEVEL, INDENT, type->info.enums.enm[i].value);
            level--;
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_IDENT:
        if (module == type->info.ident.ref->module) {
            ly_print(out, "%*sbase %s;\n", LEVEL, INDENT, type->info.ident.ref->name);
        } else {
            ly_print(out, "%*sbase %s:%s;\n", LEVEL, INDENT, type->info.ident.ref->module->prefix,
                    type->info.ident.ref->name);
        }
        break;
    case LY_TYPE_INST:
        if (type->info.inst.req == 1) {
            ly_print(out, "%*srequire-instance \"true\";\n", LEVEL, INDENT);
        } else if (type->info.inst.req == -1) {
            ly_print(out, "%*srequire-instance \"false\";\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        if (type->info.num.range != NULL) {
            ly_print(out, "%*srange \"%s\" {\n", LEVEL, INDENT, type->info.num.range->expr);
            yang_print_restr(out, level + 1, type->info.num.range);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_LEAFREF:
        str = transform_expr_json2xml(module, type->info.lref.path, NULL, NULL, NULL);
        ly_print(out, "%*spath \"%s\";\n", LEVEL, INDENT, str);
        lydict_remove(module->ctx, str);
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length) {
            ly_print(out, "%*slength \"%s\" {\n", LEVEL, INDENT, type->info.str.length->expr);
            yang_print_restr(out, level + 1, type->info.str.length);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        }
        for (i = 0; i < type->info.str.pat_count; i++) {
            ly_print(out, "%*spattern \"%s\" {\n", LEVEL, INDENT, type->info.str.patterns[i].expr);
            yang_print_restr(out, level + 1, &type->info.str.patterns[i]);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; ++i) {
            yang_print_type(out, level, module, &type->info.uni.types[i]);
        }
        break;
    default:
        /* other types do not have substatements */
        break;
    }
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_must(struct lyout *out, int level, struct lys_module *module, struct lys_restr *must)
{
    const char *xml_expr;

    xml_expr = transform_expr_json2xml(module, must->expr, NULL, NULL, NULL);
    if (!xml_expr) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*smust \"%s\" {\n", LEVEL, INDENT, xml_expr);
    lydict_remove(module->ctx, xml_expr);

    yang_print_restr(out, level + 1, must);
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_unique(struct lyout *out, int level, struct lys_unique *uniq)
{
    int i;

    ly_print(out, "%*sunique \"", LEVEL, INDENT);
    for (i = 0; i < uniq->leafs_size; i++) {
        ly_print(out, "%s%s", uniq->leafs[i]->name, i + 1 < uniq->leafs_size ? " " : "");
    }
    ly_print(out, "\";\n");
}

static void
yang_print_refine(struct lyout *out, int level, struct lys_module *module, struct lys_refine *refine)
{
    int i;
    const char *str;

    str = transform_expr_json2xml(module, refine->target_name, NULL, NULL, NULL);
    ly_print(out, "%*srefine \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    if (refine->flags & LYS_CONFIG_W) {
        ly_print(out, "%*sconfig \"true\";\n", LEVEL, INDENT);
    } else if (refine->flags & LYS_CONFIG_R) {
        ly_print(out, "%*sconfig \"false\";\n", LEVEL, INDENT);
    }

    if (refine->flags & LYS_MAND_TRUE) {
        ly_print(out, "%*smandatory \"true\";\n", LEVEL, INDENT);
    } else if (refine->flags & LYS_MAND_FALSE) {
        ly_print(out, "%*smandatory \"false\";\n", LEVEL, INDENT);
    }

    yang_print_snode_common(out, level, (struct lys_node *)refine);

    for (i = 0; i < refine->must_size; ++i) {
        yang_print_must(out, level, module, &refine->must[i]);
    }

    if (refine->target_type & (LYS_LEAF | LYS_CHOICE)) {
        if (refine->mod.dflt != NULL) {
            ly_print(out, "%*sdefault \"%s\";\n", LEVEL, INDENT, refine->mod.dflt);
        }
    } else if (refine->target_type == LYS_CONTAINER) {
        if (refine->mod.presence != NULL) {
            yang_print_text(out, level, "presence", refine->mod.presence);
        }
    } else if (refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
        if (refine->mod.list.min > 0) {
            ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, refine->mod.list.min);
        }
        if (refine->mod.list.max > 0) {
            ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, refine->mod.list.max);
        }
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_deviation(struct lyout *out, int level, struct lys_module *module, struct lys_deviation *deviation)
{
    int i, j;
    const char *str;

    str = transform_expr_json2xml(module, deviation->target_name, NULL, NULL, NULL);
    ly_print(out, "%*sdeviation \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    if (deviation->dsc) {
        yang_print_text(out, level, "description", deviation->dsc);
    }
    if (deviation->ref) {
        yang_print_text(out, level, "reference", deviation->ref);
    }

    for (i = 0; i < deviation->deviate_size; ++i) {
        ly_print(out, "%*sdeviate ", LEVEL, INDENT);
        if (deviation->deviate[i].mod == LY_DEVIATE_NO) {
            ly_print(out, "not-supported {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_ADD) {
            ly_print(out, "add {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_RPL) {
            ly_print(out, "replace {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_DEL) {
            ly_print(out, "delete {\n");
        }
        level++;

        if (deviation->deviate[i].flags & LYS_CONFIG_W) {
            ly_print(out, "%*sconfig \"true\";\n", LEVEL, INDENT);
        } else if (deviation->deviate[i].flags & LYS_CONFIG_R) {
            ly_print(out, "%*sconfig \"false\";\n", LEVEL, INDENT);
        }

        if (deviation->deviate[i].flags & LYS_MAND_TRUE) {
            ly_print(out, "%*smandatory \"true\";\n", LEVEL, INDENT);
        } else if (deviation->deviate[i].flags & LYS_MAND_FALSE) {
            ly_print(out, "%*smandatory \"false\";\n", LEVEL, INDENT);
        }

        if (deviation->deviate[i].dflt) {
            ly_print(out, "%*sdefault %s;\n", LEVEL, INDENT, deviation->deviate[i].dflt);
        }

        if (deviation->deviate[i].min) {
            ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, deviation->deviate[i].min);
        }
        if (deviation->deviate[i].max) {
            ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, deviation->deviate[i].max);
        }

        for (j = 0; j < deviation->deviate[i].must_size; ++j) {
            yang_print_must(out, level, module, &deviation->deviate[i].must[j]);
        }

        for (j = 0; j < deviation->deviate[i].unique_size; ++j) {
            yang_print_unique(out, level, &deviation->deviate[i].unique[j]);
        }

        if (deviation->deviate[i].type) {
            yang_print_type(out, level, module, deviation->deviate[i].type);
        }

        if (deviation->deviate[i].units) {
            ly_print(out, "%*sunits %s;\n", LEVEL, INDENT, deviation->deviate[i].units);
        }

        level--;
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_augment(struct lyout *out, int level, struct lys_module *module, struct lys_node_augment *augment)
{
    int i;
    struct lys_node *sub;
    const char *str;

    str = transform_expr_json2xml(module, augment->target_name, NULL, NULL, NULL);
    ly_print(out, "%*saugment \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    yang_print_nacmext(out, level, (struct lys_node *)augment, module);
    yang_print_snode_common(out, level, (struct lys_node *)augment);

    for (i = 0; i < augment->features_size; i++) {
        yang_print_iffeature(out, level, module, augment->features[i]);
    }

    if (augment->when) {
        yang_print_when(out, level, module, augment->when);
    }

    LY_TREE_FOR(augment->child, sub) {
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYXML | LYS_CASE);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_typedef(struct lyout *out, int level, struct lys_module *module, struct lys_tpdf *tpdf)
{
    ly_print(out, "%*stypedef %s {\n", LEVEL, INDENT, tpdf->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)tpdf);
    yang_print_type(out, level, module, &tpdf->type);
    if (tpdf->units != NULL) {
        ly_print(out, "%*sunits \"%s\";\n", LEVEL, INDENT, tpdf->units);
    }
    if (tpdf->dflt != NULL) {
        ly_print(out, "%*sdefault \"%s\";\n", LEVEL, INDENT, tpdf->dflt);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_identity(struct lyout *out, int level, struct lys_ident *ident)
{
    ly_print(out, "%*sidentity %s {\n", LEVEL, INDENT, ident->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)ident);
    if (ident->base) {
        ly_print(out, "%*sbase ", LEVEL, INDENT);
        if ((ident->module != ident->base->module) && !ident->base->module->type) {
            ly_print(out, "%s:", get_module_import_prefix(ident->module, ident->base->module));
        }
        ly_print(out, "%s;\n", ident->base->name);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);

}

static void
yang_print_container(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_container *cont = (struct lys_node_container *)node;

    ly_print(out, "%*scontainer %s {\n", LEVEL, INDENT, node->name);

    level++;

    yang_print_nacmext(out, level, node, node->module);

    if (cont->presence != NULL) {
        yang_print_text(out, level, "presence", cont->presence);
    }

    for (i = 0; i < cont->must_size; i++) {
        yang_print_must(out, level, node->module, &cont->must[i]);
    }

    yang_print_snode_common2(out, level, node);

    for (i = 0; i < cont->features_size; i++) {
        yang_print_iffeature(out, level, node->module, cont->features[i]);
    }

    for (i = 0; i < cont->tpdf_size; i++) {
        yang_print_typedef(out, level, node->module, &cont->tpdf[i]);
    }

    if (cont->when) {
        yang_print_when(out, level, node->module, cont->when);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_case(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_case *cas = (struct lys_node_case *)node;

    ly_print(out, "%*scase %s {\n", LEVEL, INDENT, cas->name);
    level++;
    yang_print_nacmext(out, level, node, node->module);
    yang_print_snode_common2(out, level, node);

    for (i = 0; i < cas->features_size; i++) {
        yang_print_iffeature(out, level, node->module, cas->features[i]);
    }

    if (cas->when) {
        yang_print_when(out, level, node->module, cas->when);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYXML);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_choice(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    ly_print(out, "%*schoice %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_nacmext(out, level, node, node->module);
    if (choice->dflt != NULL) {
        ly_print(out, "%*sdefault \"%s\";\n", LEVEL, INDENT, choice->dflt->name);
    }

    yang_print_snode_common2(out, level, node);

    for (i = 0; i < choice->features_size; i++) {
        yang_print_iffeature(out, level, node->module, choice->features[i]);
    }

    if (choice->when) {
        yang_print_when(out, level, node->module, choice->when);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_CASE);
    }
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_leaf(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;

    ly_print(out, "%*sleaf %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_nacmext(out, level, node, node->module);
    yang_print_snode_common2(out, level, node);
    for (i = 0; i < leaf->features_size; i++) {
        yang_print_iffeature(out, level, node->module, leaf->features[i]);
    }
    for (i = 0; i < leaf->must_size; i++) {
        yang_print_must(out, level, node->module, &leaf->must[i]);
    }
    if (leaf->when) {
        yang_print_when(out, level, node->module, leaf->when);
    }
    yang_print_type(out, level, node->module, &leaf->type);
    if (leaf->units != NULL) {
        ly_print(out, "%*sunits \"%s\";\n", LEVEL, INDENT, leaf->units);
    }
    if (leaf->dflt != NULL) {
        ly_print(out, "%*sdefault \"%s\";\n", LEVEL, INDENT, leaf->dflt);
    }
    level--;

    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_anyxml(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node_anyxml *anyxml = (struct lys_node_anyxml *)node;

    ly_print(out, "%*sanyxml %s {\n", LEVEL, INDENT, anyxml->name);
    level++;
    yang_print_nacmext(out, level, node, node->module);
    yang_print_snode_common2(out, level, node);
    for (i = 0; i < anyxml->features_size; i++) {
        yang_print_iffeature(out, level, node->module, anyxml->features[i]);
    }
    for (i = 0; i < anyxml->must_size; i++) {
        yang_print_must(out, level, node->module, &anyxml->must[i]);
    }
    if (anyxml->when) {
        yang_print_when(out, level, node->module, anyxml->when);
    }
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_leaflist(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;

    ly_print(out, "%*sleaf-list %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_nacmext(out, level, node, node->module);
    yang_print_snode_common2(out, level, node);
    for (i = 0; i < llist->features_size; i++) {
        yang_print_iffeature(out, level, node->module, llist->features[i]);
    }
    if (llist->flags & LYS_USERORDERED) {
        ly_print(out, "%*sordered-by user;\n", LEVEL, INDENT);
    }
    if (llist->min > 0) {
        ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, llist->min);
    }
    if (llist->max > 0) {
        ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, llist->max);
    }
    for (i = 0; i < llist->must_size; i++) {
        yang_print_must(out, level, node->module, &llist->must[i]);
    }
    if (llist->when) {
        yang_print_when(out, level, llist->module, llist->when);
    }
    yang_print_type(out, level, node->module, &llist->type);
    if (llist->units != NULL) {
        ly_print(out, "%*sunits \"%s\";\n", LEVEL, INDENT, llist->units);
    }
    level--;

    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_list(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;

    ly_print(out, "%*slist %s {\n", LEVEL, INDENT, node->name);
    level++;
    yang_print_nacmext(out, level, node, node->module);
    yang_print_snode_common2(out, level, node);

    for (i = 0; i < list->features_size; i++) {
        yang_print_iffeature(out, level, node->module, list->features[i]);
    }

    if (list->keys_size) {
        ly_print(out, "%*skey \"", LEVEL, INDENT);
        for (i = 0; i < list->keys_size; i++) {
            ly_print(out, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? " " : "");
        }
        ly_print(out, "\";\n");
    }

    for (i = 0; i < list->unique_size; i++) {
        yang_print_unique(out, level, &list->unique[i]);
    }

    if (list->flags & LYS_USERORDERED) {
        ly_print(out, "%*sordered-by user;\n", LEVEL, INDENT);
    }
    if (list->min > 0) {
        ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, list->min);
    }
    if (list->max > 0) {
        ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, list->max);
    }
    for (i = 0; i < list->must_size; i++) {
        yang_print_must(out, level, list->module, &list->must[i]);
    }
    if (list->when) {
        yang_print_when(out, level, list->module, list->when);
    }

    for (i = 0; i < list->tpdf_size; i++) {
        yang_print_typedef(out, level, list->module, &list->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_grouping(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *child;
    struct lys_node_grp *grp = (struct lys_node_grp *)node;

    ly_print(out, "%*sgrouping %s {\n", LEVEL, INDENT, node->name);
    level++;

    yang_print_snode_common(out, level, node);

    for (i = 0; i < grp->tpdf_size; i++) {
        yang_print_typedef(out, level, node->module, &grp->tpdf[i]);
    }

    LY_TREE_FOR(node->child, child) {
        yang_print_snode(out, level, child,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_uses(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node_uses *uses = (struct lys_node_uses *)node;

    ly_print(out, "%*suses ", LEVEL, INDENT);
    if (node->child && (node->module != node->child->module) && !node->child->module->type) {
        ly_print(out, "%s:", get_module_import_prefix(node->module, node->child->module));
    }
    ly_print(out, "%s {\n",uses->name);
    level++;

    yang_print_nacmext(out, level, node, node->module);
    yang_print_snode_common(out, level, node);
    for (i = 0; i < uses->features_size; i++) {
        yang_print_iffeature(out, level, node->module, uses->features[i]);
    }
    if (uses->when) {
        yang_print_when(out, level, node->module, uses->when);
    }

    for (i = 0; i < uses->refine_size; i++) {
        yang_print_refine(out, level, node->module, &uses->refine[i]);
    }

    for (i = 0; i < uses->augment_size; i++) {
        yang_print_augment(out, level, node->module, &uses->augment[i]);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_input_output(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_rpc_inout *inout = (struct lys_node_rpc_inout *)node;

    ly_print(out, "%*s%s {\n", LEVEL, INDENT, (inout->nodetype == LYS_INPUT ? "input" : "output"));

    level++;
    for (i = 0; i < inout->tpdf_size; i++) {
        yang_print_typedef(out, level, node->module, &inout->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_rpc(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_rpc *rpc = (struct lys_node_rpc *)node;

    ly_print(out, "%*srpc %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_snode_common(out, level, node);

    for (i = 0; i < rpc->features_size; i++) {
        yang_print_iffeature(out, level, node->module, rpc->features[i]);
    }

    for (i = 0; i < rpc->tpdf_size; i++) {
        yang_print_typedef(out, level, node->module, &rpc->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        yang_print_snode(out, level, sub,
                         LYS_GROUPING | LYS_INPUT | LYS_OUTPUT);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_notif(struct lyout *out, int level, struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_notif *notif = (struct lys_node_notif *)node;

    ly_print(out, "%*snotification %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_snode_common(out, level, node);

    for (i = 0; i < notif->features_size; i++) {
        yang_print_iffeature(out, level, node->module, notif->features[i]);
    }

    for (i = 0; i < notif->tpdf_size; i++) {
        yang_print_typedef(out, level, node->module, &notif->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_snode(struct lyout *out, int level, struct lys_node *node, int mask)
{
    switch (node->nodetype & mask) {
    case LYS_CONTAINER:
        yang_print_container(out, level, node);
        break;
    case LYS_CHOICE:
        yang_print_choice(out, level, node);
        break;
    case LYS_LEAF:
        yang_print_leaf(out, level, node);
        break;
    case LYS_LEAFLIST:
        yang_print_leaflist(out, level, node);
        break;
    case LYS_LIST:
        yang_print_list(out, level, node);
        break;
    case LYS_USES:
        yang_print_uses(out, level, node);
        break;
    case LYS_GROUPING:
        yang_print_grouping(out, level, node);
        break;
    case LYS_ANYXML:
        yang_print_anyxml(out, level, node);
        break;
    case LYS_CASE:
        yang_print_case(out, level, node);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        yang_print_input_output(out, level, node);
        break;
    default:
        break;
    }
}

int
yang_print_model(struct lyout *out, struct lys_module *module)
{
    unsigned int i;
    int level = 0;
#define LEVEL (level*2)

    struct lys_node *node;

    if (module->type) {
        ly_print(out, "submodule %s {%s\n", module->name, (module->deviated ? " // DEVIATED" : ""));
        level++;
        ly_print(out, "%*sbelongs-to %s {\n", LEVEL, INDENT, ((struct lys_submodule *)module)->belongsto->name);
        level++;
        ly_print(out, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->prefix);
        level--;
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    } else {
        ly_print(out, "module %s {%s\n", module->name, (module->deviated ? " // DEVIATED" : ""));
        level++;
        ly_print(out, "%*snamespace \"%s\";\n", LEVEL, INDENT, module->ns);
        ly_print(out, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->prefix);
    }

    if (module->version) {
        ly_print(out, "%*syang-version %s;\n", LEVEL, INDENT, module->version == 1 ? "1" : "1.1");
    }

    for (i = 0; i < module->imp_size; i++) {
        ly_print(out, "%*simport \"%s\" {\n", LEVEL, INDENT, module->imp[i].module->name);
        level++;
        ly_print(out, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->imp[i].prefix);
        if (module->imp[i].rev[0]) {
            yang_print_text(out, level, "revision-date", module->imp[i].rev);
        }
        level--;
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    }

    for (i = 0; i < module->deviation_size; ++i) {
        yang_print_deviation(out, level, module, &module->deviation[i]);
    }

    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].rev[0]) {
            ly_print(out, "%*sinclude \"%s\" {\n", LEVEL, INDENT, module->inc[i].submodule->name);
            yang_print_text(out, level + 1, "revision-date", module->inc[i].rev);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        } else {
            ly_print(out, "%*sinclude \"%s\";\n", LEVEL, INDENT, module->inc[i].submodule->name);
        }
    }

    if (module->org) {
        yang_print_text(out, level, "organization", module->org);
    }
    if (module->contact) {
        yang_print_text(out, level, "contact", module->contact);
    }
    if (module->dsc) {
        yang_print_text(out, level, "description", module->dsc);
    }
    if (module->ref) {
        yang_print_text(out, level, "reference", module->ref);
    }
    for (i = 0; i < module->rev_size; i++) {
        if (module->rev[i].dsc || module->rev[i].ref) {
            ly_print(out, "%*srevision \"%s\" {\n", LEVEL, INDENT, module->rev[i].date);
            level++;
            if (module->rev[i].dsc) {
                yang_print_text(out, level, "description", module->rev[i].dsc);
            }
            if (module->rev[i].ref) {
                yang_print_text(out, level, "reference", module->rev[i].ref);
            }
            level--;
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        } else {
            yang_print_text(out, level, "revision", module->rev[i].date);
        }
    }

    for (i = 0; i < module->features_size; i++) {
        yang_print_feature(out, level, &module->features[i]);
    }

    for (i = 0; i < module->ident_size; i++) {
        yang_print_identity(out, level, &module->ident[i]);
    }

    for (i = 0; i < module->tpdf_size; i++) {
        yang_print_typedef(out, level, module, &module->tpdf[i]);
    }

    LY_TREE_FOR(module->data, node) {
        switch(node->nodetype) {
        case LYS_RPC:
            yang_print_rpc(out, level, node);
            break;
        case LYS_NOTIF:
            yang_print_notif(out, level, node);
            break;
        default:
            yang_print_snode(out, level, node,
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                             LYS_USES | LYS_GROUPING | LYS_ANYXML);
            break;
        }
    }

    for (i = 0; i < module->augment_size; i++) {
        yang_print_augment(out, level, module, &module->augment[i]);
    }

    ly_print(out, "}\n");

    return EXIT_SUCCESS;
#undef LEVEL
}
