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
#include <stdio.h>
#include <string.h>

#include "../common.h"
#include "../tree.h"

#define INDENT ""
#define LEVEL (level*2)

static void yang_print_mnode(FILE *f, int level, struct lys_node *mnode, int mask);

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
yang_print_text(FILE *f, int level, const char *name, const char *text)
{
    const char *s, *t;

    fprintf(f, "%*s%s\n", LEVEL, INDENT, name);
    level++;

    fprintf(f, "%*s\"", LEVEL, INDENT);
    t = text;
    while ((s = strchr(t, '\n'))) {
        fwrite(t, sizeof *t, (s - t) + 1, f);
        t = s + 1;
        fprintf(f, "%*s", LEVEL, INDENT);
    }

    fprintf(f, "%s\";\n\n", t);
    level--;

}

static void
yang_print_nacmext(FILE *f, int level, struct lys_node *mnode, struct lys_module *module)
{
    int i, j;
    const char *prefix = NULL;

    if (mnode->nacm && (!mnode->parent || mnode->parent->nacm != mnode->nacm)) {
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

        if ((mnode->nacm & LYS_NACM_DENYW) && (!mnode->parent || !(mnode->parent->nacm & LYS_NACM_DENYW))) {
            fprintf(f, "%*s%s:default-deny-write;\n", LEVEL, INDENT, prefix);
        }
        if ((mnode->nacm & LYS_NACM_DENYA) && (!mnode->parent || !(mnode->parent->nacm & LYS_NACM_DENYA))) {
            fprintf(f, "%*s%s:default-deny-all;\n", LEVEL, INDENT, prefix);
        }
    }
}

/*
 * Covers:
 * description, reference, status
 */
static void
yang_print_mnode_common(FILE *f, int level, struct lys_node *mnode)
{
    if (mnode->flags & LYS_STATUS_CURR) {
        fprintf(f, "%*sstatus \"current\";\n", LEVEL, INDENT);
    } else if (mnode->flags & LYS_STATUS_DEPRC) {
        fprintf(f, "%*sstatus \"deprecated\";\n", LEVEL, INDENT);
    } else if (mnode->flags & LYS_STATUS_OBSLT) {
        fprintf(f, "%*sstatus \"obsolete\";\n", LEVEL, INDENT);
    }

    if (mnode->dsc) {
        yang_print_text(f, level, "description", mnode->dsc);
    }
    if (mnode->ref) {
        yang_print_text(f, level, "reference", mnode->ref);
    }
}

/*
 * Covers:
 * config, mandatory
 * description, reference, status
 */
static void
yang_print_mnode_common2(FILE *f, int level, struct lys_node *mnode)
{
    if (!mnode->parent || (mnode->parent->flags & LYS_CONFIG_MASK) != (mnode->flags & LYS_CONFIG_MASK)) {
        /* print config only when it differs from the parent or in root */
        if (mnode->flags & LYS_CONFIG_W) {
            fprintf(f, "%*sconfig \"true\";\n", LEVEL, INDENT);
        } else if (mnode->flags & LYS_CONFIG_R) {
            fprintf(f, "%*sconfig \"false\";\n", LEVEL, INDENT);
        }
    }

    if (mnode->flags & LYS_MAND_TRUE) {
        fprintf(f, "%*smandatory \"true\";\n", LEVEL, INDENT);
    } else if (mnode->flags & LYS_MAND_FALSE) {
        fprintf(f, "%*smandatory \"false\";\n", LEVEL, INDENT);
    }

    yang_print_mnode_common(f, level, mnode);
}

static void
yang_print_iffeature(FILE *f, int level, struct lys_module *module, struct lys_feature *feat)
{
    fprintf(f, "%*sif-feature ", LEVEL, INDENT);
    if ((feat->module != module) && !feat->module->type) {
        fprintf(f, "%s:", get_module_import_prefix(module, feat->module));
    }
    fprintf(f, "%s;\n", feat->name);
}

static void
yang_print_feature(FILE *f, int level, struct lys_feature *feat)
{
    int i;

    fprintf(f, "%*sfeature %s {\n", LEVEL, INDENT, feat->name);
    level++;

    yang_print_mnode_common(f, level, (struct lys_node *)feat);
    for (i = 0; i < feat->features_size; ++i) {
        yang_print_iffeature(f, level, feat->module, feat->features[i]);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_restr(FILE *f, int level, struct lys_restr *restr)
{
    if (restr->dsc != NULL) {
        yang_print_text(f, level, "description", restr->dsc);
    }
    if (restr->ref != NULL) {
        yang_print_text(f, level, "reference", restr->ref);
    }
    if (restr->eapptag != NULL) {
        fprintf(f, "%*serror-app-tag \"%s\";\n", LEVEL, INDENT, restr->eapptag);
    }
    if (restr->emsg != NULL) {
        yang_print_text(f, level, "error-message", restr->emsg);
    }
}

static void
yang_print_when(FILE *f, int level, struct lys_when *when)
{
    fprintf(f, "%*swhen \"%s\" {\n", LEVEL, INDENT, when->cond);
    level++;
    if (when->dsc) {
        yang_print_text(f, level, "description", when->dsc);
    }
    if (when->ref) {
        yang_print_text(f, level, "reference", when->ref);
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_type(FILE *f, int level, struct lys_module *module, struct lys_type *type)
{
    int i;

    if (type->prefix) {
        fprintf(f, "%*stype %s:%s {\n", LEVEL, INDENT, type->prefix, type->der->name);
    } else {
        fprintf(f, "%*stype %s {\n", LEVEL, INDENT, type->der->name);
    }
    level++;
    switch (type->base) {
    case LY_TYPE_BINARY:
        if (type->info.binary.length != NULL) {
            fprintf(f, "%*slength \"%s\" {\n", LEVEL, INDENT, type->info.binary.length->expr);
            yang_print_restr(f, level + 1, type->info.binary.length);
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            fprintf(f, "%*sbit %s {\n", LEVEL, INDENT, type->info.bits.bit[i].name);
            level++;
            yang_print_mnode_common(f, level, (struct lys_node *)&type->info.bits.bit[i]);
            fprintf(f, "%*sposition %u;\n", LEVEL, INDENT, type->info.bits.bit[i].pos);
            level--;
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_DEC64:
        fprintf(f, "%*sfraction-digits %d;\n", LEVEL, INDENT, type->info.dec64.dig);
        if (type->info.dec64.range != NULL) {
            fprintf(f, "%*srange \"%s\" {\n", LEVEL, INDENT, type->info.dec64.range->expr);
            yang_print_restr(f, level + 1, type->info.dec64.range);
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            fprintf(f, "%*senum %s {\n", LEVEL, INDENT, type->info.enums.enm[i].name);
            level++;
            yang_print_mnode_common(f, level, (struct lys_node *)&type->info.enums.enm[i]);
            fprintf(f, "%*svalue %d;\n", LEVEL, INDENT, type->info.enums.enm[i].value);
            level--;
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_IDENT:
        if (module == type->info.ident.ref->module) {
            fprintf(f, "%*sbase %s;\n", LEVEL, INDENT, type->info.ident.ref->name);
        } else {
            fprintf(f, "%*sbase %s:%s;\n", LEVEL, INDENT, type->info.ident.ref->module->prefix,
                    type->info.ident.ref->name);
        }
        break;
    case LY_TYPE_INST:
        if (type->info.inst.req == 1) {
            fprintf(f, "%*srequire-instance \"true\";\n", LEVEL, INDENT);
        } else if (type->info.inst.req == -1) {
            fprintf(f, "%*srequire-instance \"false\";\n", LEVEL, INDENT);
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
            fprintf(f, "%*srange \"%s\" {\n", LEVEL, INDENT, type->info.num.range->expr);
            yang_print_restr(f, level + 1, type->info.num.range);
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_LEAFREF:
        fprintf(f, "%*spath \"%s\";\n", LEVEL, INDENT, type->info.lref.path);
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length) {
            fprintf(f, "%*slength \"%s\" {\n", LEVEL, INDENT, type->info.str.length->expr);
            yang_print_restr(f, level + 1, type->info.str.length);
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        for (i = 0; i < type->info.str.pat_count; i++) {
            fprintf(f, "%*spattern \"%s\" {\n", LEVEL, INDENT, type->info.str.patterns[i].expr);
            yang_print_restr(f, level + 1, &type->info.str.patterns[i]);
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; ++i) {
            yang_print_type(f, level, module, &type->info.uni.types[i]);
        }
        break;
    default:
        /* other types do not have substatements */
        break;
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_must(FILE *f, int level, struct lys_restr *must)
{
    fprintf(f, "%*smust \"%s\" {\n", LEVEL, INDENT, must->expr);
    yang_print_restr(f, level + 1, must);
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_unique(FILE *f, int level, struct lys_unique *uniq)
{
    int i;

    fprintf(f, "%*sunique \"", LEVEL, INDENT);
    for (i = 0; i < uniq->leafs_size; i++) {
        fprintf(f, "%s%s", uniq->leafs[i]->name, i + 1 < uniq->leafs_size ? " " : "");
    }
    fprintf(f, "\";\n");
}

static void
yang_print_refine(FILE *f, int level, struct lys_refine *refine)
{
    int i;

    fprintf(f, "%*srefine \"%s\" {\n", LEVEL, INDENT, refine->target_name);
    level++;

    if (refine->flags & LYS_CONFIG_W) {
        fprintf(f, "%*sconfig \"true\";\n", LEVEL, INDENT);
    } else if (refine->flags & LYS_CONFIG_R) {
        fprintf(f, "%*sconfig \"false\";\n", LEVEL, INDENT);
    }

    if (refine->flags & LYS_MAND_TRUE) {
        fprintf(f, "%*smandatory \"true\";\n", LEVEL, INDENT);
    } else if (refine->flags & LYS_MAND_FALSE) {
        fprintf(f, "%*smandatory \"false\";\n", LEVEL, INDENT);
    }

    yang_print_mnode_common(f, level, (struct lys_node *)refine);

    for (i = 0; i < refine->must_size; ++i) {
        yang_print_must(f, level, &refine->must[i]);
    }

    if (refine->target_type & (LYS_LEAF | LYS_CHOICE)) {
        if (refine->mod.dflt != NULL) {
            fprintf(f, "%*sdefault \"%s\";\n", LEVEL, INDENT, refine->mod.dflt);
        }
    } else if (refine->target_type == LYS_CONTAINER) {
        if (refine->mod.presence != NULL) {
            yang_print_text(f, level, "presence", refine->mod.presence);
        }
    } else if (refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
        if (refine->mod.list.min > 0) {
            fprintf(f, "%*smin-elements %u;\n", LEVEL, INDENT, refine->mod.list.min);
        }
        if (refine->mod.list.max > 0) {
            fprintf(f, "%*smax-elements %u;\n", LEVEL, INDENT, refine->mod.list.max);
        }
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_deviation(FILE *f, int level, struct lys_module *module, struct lys_deviation *deviation)
{
    int i, j;

    fprintf(f, "%*sdeviation \"%s\" {\n", LEVEL, INDENT, deviation->target_name);
    level++;

    if (deviation->dsc) {
        yang_print_text(f, level, "description", deviation->dsc);
    }
    if (deviation->ref) {
        yang_print_text(f, level, "reference", deviation->ref);
    }

    for (i = 0; i < deviation->deviate_size; ++i) {
        fprintf(f, "%*sdeviate ", LEVEL, INDENT);
        if (deviation->deviate[i].mod == LY_DEVIATE_NO) {
            fprintf(f, "not-supported {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_ADD) {
            fprintf(f, "add {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_RPL) {
            fprintf(f, "replace {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_DEL) {
            fprintf(f, "delete {\n");
        }
        level++;

        if (deviation->deviate[i].flags & LYS_CONFIG_W) {
            fprintf(f, "%*sconfig \"true\";\n", LEVEL, INDENT);
        } else if (deviation->deviate[i].flags & LYS_CONFIG_R) {
            fprintf(f, "%*sconfig \"false\";\n", LEVEL, INDENT);
        }

        if (deviation->deviate[i].flags & LYS_MAND_TRUE) {
            fprintf(f, "%*smandatory \"true\";\n", LEVEL, INDENT);
        } else if (deviation->deviate[i].flags & LYS_MAND_FALSE) {
            fprintf(f, "%*smandatory \"false\";\n", LEVEL, INDENT);
        }

        if (deviation->deviate[i].dflt) {
            fprintf(f, "%*sdefault %s;\n", LEVEL, INDENT, deviation->deviate[i].dflt);
        }

        if (deviation->deviate[i].min) {
            fprintf(f, "%*smin-elements %u;\n", LEVEL, INDENT, deviation->deviate[i].min);
        }
        if (deviation->deviate[i].max) {
            fprintf(f, "%*smax-elements %u;\n", LEVEL, INDENT, deviation->deviate[i].max);
        }

        for (j = 0; j < deviation->deviate[i].must_size; ++j) {
            yang_print_must(f, level, &deviation->deviate[i].must[j]);
        }

        for (j = 0; j < deviation->deviate[i].unique_size; ++j) {
            yang_print_unique(f, level, &deviation->deviate[i].unique[j]);
        }

        if (deviation->deviate[i].type) {
            yang_print_type(f, level, module, deviation->deviate[i].type);
        }

        if (deviation->deviate[i].units) {
            fprintf(f, "%*sunits %s;\n", LEVEL, INDENT, deviation->deviate[i].units);
        }

        level--;
        fprintf(f, "%*s}\n", LEVEL, INDENT);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_augment(FILE *f, int level, struct lys_module *module, struct lys_node_augment *augment)
{
    int i;
    struct lys_node *sub;

    fprintf(f, "%*saugment \"%s\" {\n", LEVEL, INDENT, augment->target_name);
    level++;

    yang_print_nacmext(f, level, (struct lys_node *)augment, module);
    yang_print_mnode_common(f, level, (struct lys_node *)augment);

    for (i = 0; i < augment->features_size; i++) {
        yang_print_iffeature(f, level, module, augment->features[i]);
    }

    if (augment->when) {
        yang_print_when(f, level, augment->when);
    }

    LY_TREE_FOR(augment->child, sub) {
        yang_print_mnode(f, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYXML | LYS_CASE);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_typedef(FILE *f, int level, struct lys_module *module, struct lys_tpdf *tpdf)
{
    fprintf(f, "%*stypedef %s {\n", LEVEL, INDENT, tpdf->name);
    level++;

    yang_print_mnode_common(f, level, (struct lys_node *)tpdf);
    yang_print_type(f, level, module, &tpdf->type);
    if (tpdf->units != NULL) {
        fprintf(f, "%*sunits \"%s\";\n", LEVEL, INDENT, tpdf->units);
    }
    if (tpdf->dflt != NULL) {
        fprintf(f, "%*sdefault \"%s\";\n", LEVEL, INDENT, tpdf->dflt);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_identity(FILE *f, int level, struct lys_ident *ident)
{
    fprintf(f, "%*sidentity %s {\n", LEVEL, INDENT, ident->name);
    level++;

    yang_print_mnode_common(f, level, (struct lys_node *)ident);
    if (ident->base) {
        fprintf(f, "%*sbase ", LEVEL, INDENT);
        if ((ident->module != ident->base->module) && !ident->base->module->type) {
            fprintf(f, "%s:", get_module_import_prefix(ident->module, ident->base->module));
        }
        fprintf(f, "%s;\n", ident->base->name);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);

}

static void
yang_print_container(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *sub;
    struct lys_node_container *cont = (struct lys_node_container *)mnode;

    fprintf(f, "%*scontainer %s {\n", LEVEL, INDENT, mnode->name);

    level++;

    yang_print_nacmext(f, level, mnode, mnode->module);

    if (cont->presence != NULL) {
        yang_print_text(f, level, "presence", cont->presence);
    }

    for (i = 0; i < cont->must_size; i++) {
        yang_print_must(f, level, &cont->must[i]);
    }

    yang_print_mnode_common2(f, level, mnode);

    for (i = 0; i < cont->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, cont->features[i]);
    }

    for (i = 0; i < cont->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &cont->tpdf[i]);
    }

    if (cont->when) {
        yang_print_when(f, level, cont->when);
    }

    LY_TREE_FOR(mnode->child, sub) {
        /* augment */
        if (sub->parent != mnode) {
            continue;
        }
        yang_print_mnode(f, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_case(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *sub;
    struct lys_node_case *cas = (struct lys_node_case *)mnode;

    fprintf(f, "%*scase %s {\n", LEVEL, INDENT, cas->name);
    level++;
    yang_print_nacmext(f, level, mnode, mnode->module);
    yang_print_mnode_common2(f, level, mnode);

    for (i = 0; i < cas->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, cas->features[i]);
    }

    if (cas->when) {
        yang_print_when(f, level, cas->when);
    }

    LY_TREE_FOR(mnode->child, sub) {
        /* augment */
        if (sub->parent != mnode) {
            continue;
        }
        yang_print_mnode(f, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_choice(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *sub;
    struct lys_node_choice *choice = (struct lys_node_choice *)mnode;

    fprintf(f, "%*schoice %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_nacmext(f, level, mnode, mnode->module);
    if (choice->dflt != NULL) {
        fprintf(f, "%*sdefault \"%s\";\n", LEVEL, INDENT, choice->dflt->name);
    }

    yang_print_mnode_common2(f, level, mnode);

    for (i = 0; i < choice->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, choice->features[i]);
    }

    if (choice->when) {
        yang_print_when(f, level, choice->when);
    }

    LY_TREE_FOR(mnode->child, sub) {
        /* augment */
        if (sub->parent != mnode) {
            continue;
        }
        yang_print_mnode(f, level, sub,
                         LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_CASE);
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_leaf(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)mnode;

    fprintf(f, "%*sleaf %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_nacmext(f, level, mnode, mnode->module);
    yang_print_mnode_common2(f, level, mnode);
    for (i = 0; i < leaf->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, leaf->features[i]);
    }
    for (i = 0; i < leaf->must_size; i++) {
        yang_print_must(f, level, &leaf->must[i]);
    }
    if (leaf->when) {
        yang_print_when(f, level, leaf->when);
    }
    yang_print_type(f, level, mnode->module, &leaf->type);
    if (leaf->units != NULL) {
        fprintf(f, "%*sunits \"%s\";\n", LEVEL, INDENT, leaf->units);
    }
    if (leaf->dflt != NULL) {
        fprintf(f, "%*sdefault \"%s\";\n", LEVEL, INDENT, leaf->dflt);
    }
    level--;

    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_anyxml(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node_anyxml *anyxml = (struct lys_node_anyxml *)mnode;

    fprintf(f, "%*sanyxml %s {\n", LEVEL, INDENT, anyxml->name);
    level++;
    yang_print_nacmext(f, level, mnode, mnode->module);
    yang_print_mnode_common2(f, level, mnode);
    for (i = 0; i < anyxml->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, anyxml->features[i]);
    }
    for (i = 0; i < anyxml->must_size; i++) {
        yang_print_must(f, level, &anyxml->must[i]);
    }
    if (anyxml->when) {
        yang_print_when(f, level, anyxml->when);
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_leaflist(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)mnode;

    fprintf(f, "%*sleaf-list %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_nacmext(f, level, mnode, mnode->module);
    yang_print_mnode_common2(f, level, mnode);
    for (i = 0; i < llist->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, llist->features[i]);
    }
    if (llist->flags & LYS_USERORDERED) {
        fprintf(f, "%*sordered-by user;\n", LEVEL, INDENT);
    }
    if (llist->min > 0) {
        fprintf(f, "%*smin-elements %u;\n", LEVEL, INDENT, llist->min);
    }
    if (llist->max > 0) {
        fprintf(f, "%*smax-elements %u;\n", LEVEL, INDENT, llist->max);
    }
    for (i = 0; i < llist->must_size; i++) {
        yang_print_must(f, level, &llist->must[i]);
    }
    if (llist->when) {
        yang_print_when(f, level, llist->when);
    }
    yang_print_type(f, level, mnode->module, &llist->type);
    if (llist->units != NULL) {
        fprintf(f, "%*sunits \"%s\";\n", LEVEL, INDENT, llist->units);
    }
    level--;

    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_list(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)mnode;

    fprintf(f, "%*slist %s {\n", LEVEL, INDENT, mnode->name);
    level++;
    yang_print_nacmext(f, level, mnode, mnode->module);
    yang_print_mnode_common2(f, level, mnode);

    for (i = 0; i < list->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, list->features[i]);
    }

    if (list->keys_size) {
        fprintf(f, "%*skey \"", LEVEL, INDENT);
        for (i = 0; i < list->keys_size; i++) {
            fprintf(f, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? " " : "");
        }
        fprintf(f, "\";\n");
    }

    for (i = 0; i < list->unique_size; i++) {
        yang_print_unique(f, level, &list->unique[i]);
    }

    if (list->flags & LYS_USERORDERED) {
        fprintf(f, "%*sordered-by user;\n", LEVEL, INDENT);
    }
    if (list->min > 0) {
        fprintf(f, "%*smin-elements %u;\n", LEVEL, INDENT, list->min);
    }
    if (list->max > 0) {
        fprintf(f, "%*smax-elements %u;\n", LEVEL, INDENT, list->max);
    }
    for (i = 0; i < list->must_size; i++) {
        yang_print_must(f, level, &list->must[i]);
    }
    if (list->when) {
        yang_print_when(f, level, list->when);
    }

    for (i = 0; i < list->tpdf_size; i++) {
        yang_print_typedef(f, level, list->module, &list->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        /* augment */
        if (sub->parent != mnode) {
            continue;
        }
        yang_print_mnode(f, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_grouping(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *node;
    struct lys_node_grp *grp = (struct lys_node_grp *)mnode;

    fprintf(f, "%*sgrouping %s {\n", LEVEL, INDENT, mnode->name);
    level++;

    yang_print_mnode_common(f, level, mnode);

    for (i = 0; i < grp->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &grp->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, node) {
        yang_print_mnode(f, level, node,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_uses(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node_uses *uses = (struct lys_node_uses *)mnode;

    fprintf(f, "%*suses ", LEVEL, INDENT);
    if (mnode->child && (mnode->module != mnode->child->module) && !mnode->child->module->type) {
        fprintf(f, "%s:", get_module_import_prefix(mnode->module, mnode->child->module));
    }
    fprintf(f, "%s {\n",uses->name);
    level++;

    yang_print_nacmext(f, level, mnode, mnode->module);
    yang_print_mnode_common(f, level, mnode);
    for (i = 0; i < uses->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, uses->features[i]);
    }
    if (uses->when) {
        yang_print_when(f, level, uses->when);
    }

    for (i = 0; i < uses->refine_size; i++) {
        yang_print_refine(f, level, &uses->refine[i]);
    }

    for (i = 0; i < uses->augment_size; i++) {
        yang_print_augment(f, level, mnode->module, &uses->augment[i]);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_input_output(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *sub;
    struct lys_node_rpc_inout *inout = (struct lys_node_rpc_inout *)mnode;

    fprintf(f, "%*s%s {\n", LEVEL, INDENT, (inout->nodetype == LYS_INPUT ? "input" : "output"));

    level++;
    for (i = 0; i < inout->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &inout->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        /* augment */
        if (sub->parent != mnode) {
            continue;
        }
        yang_print_mnode(f, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_rpc(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *sub;
    struct lys_node_rpc *rpc = (struct lys_node_rpc *)mnode;

    fprintf(f, "%*srpc %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_mnode_common(f, level, mnode);

    for (i = 0; i < rpc->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, rpc->features[i]);
    }

    for (i = 0; i < rpc->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &rpc->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LYS_GROUPING | LYS_INPUT | LYS_OUTPUT);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_notif(FILE *f, int level, struct lys_node *mnode)
{
    int i;
    struct lys_node *sub;
    struct lys_node_notif *notif = (struct lys_node_notif *)mnode;

    fprintf(f, "%*snotification %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_mnode_common(f, level, mnode);

    for (i = 0; i < notif->features_size; i++) {
        yang_print_iffeature(f, level, mnode->module, notif->features[i]);
    }

    for (i = 0; i < notif->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &notif->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        /* augment */
        if (sub->parent != mnode) {
            continue;
        }
        yang_print_mnode(f, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_mnode(FILE *f, int level, struct lys_node *mnode, int mask)
{
    switch (mnode->nodetype & mask) {
    case LYS_CONTAINER:
        yang_print_container(f, level, mnode);
        break;
    case LYS_CHOICE:
        yang_print_choice(f, level, mnode);
        break;
    case LYS_LEAF:
        yang_print_leaf(f, level, mnode);
        break;
    case LYS_LEAFLIST:
        yang_print_leaflist(f, level, mnode);
        break;
    case LYS_LIST:
        yang_print_list(f, level, mnode);
        break;
    case LYS_USES:
        yang_print_uses(f, level, mnode);
        break;
    case LYS_GROUPING:
        yang_print_grouping(f, level, mnode);
        break;
    case LYS_ANYXML:
        yang_print_anyxml(f, level, mnode);
        break;
    case LYS_CASE:
        yang_print_case(f, level, mnode);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        yang_print_input_output(f, level, mnode);
        break;
    default:
        break;
    }
}

int
yang_print_model(FILE *f, struct lys_module *module)
{
    unsigned int i;
    int level = 0;
#define LEVEL (level*2)

    struct lys_node *mnode;

    if (module->type) {
        fprintf(f, "submodule %s {%s\n", module->name, (module->deviated ? " // DEVIATED" : ""));
        level++;
        fprintf(f, "%*sbelongs-to %s {\n", LEVEL, INDENT, ((struct lys_submodule *)module)->belongsto->name);
        level++;
        fprintf(f, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->prefix);
        level--;
        fprintf(f, "%*s}\n", LEVEL, INDENT);
    } else {
        fprintf(f, "module %s {%s\n", module->name, (module->deviated ? " // DEVIATED" : ""));
        level++;
        fprintf(f, "%*snamespace \"%s\";\n", LEVEL, INDENT, module->ns);
        fprintf(f, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->prefix);
    }

    if (module->version) {
        fprintf(f, "%*syang-version %s;\n", LEVEL, INDENT, module->version == 1 ? "1" : "1.1");
    }

    for (i = 0; i < module->imp_size; i++) {
        fprintf(f, "%*simport \"%s\" {\n", LEVEL, INDENT, module->imp[i].module->name);
        level++;
        fprintf(f, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->imp[i].prefix);
        if (module->imp[i].rev[0]) {
            yang_print_text(f, level, "revision-date", module->imp[i].rev);
        }
        level--;
        fprintf(f, "%*s}\n", LEVEL, INDENT);
    }

    for (i = 0; i < module->deviation_size; ++i) {
        yang_print_deviation(f, level, module, &module->deviation[i]);
    }

    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].rev[0]) {
            fprintf(f, "%*sinclude \"%s\" {\n", LEVEL, INDENT, module->inc[i].submodule->name);
            yang_print_text(f, level + 1, "revision-date", module->inc[i].rev);
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        } else {
            fprintf(f, "%*sinclude \"%s\";\n", LEVEL, INDENT, module->inc[i].submodule->name);
        }
    }

    if (module->org) {
        yang_print_text(f, level, "organization", module->org);
    }
    if (module->contact) {
        yang_print_text(f, level, "contact", module->contact);
    }
    if (module->dsc) {
        yang_print_text(f, level, "description", module->dsc);
    }
    if (module->ref) {
        yang_print_text(f, level, "reference", module->ref);
    }
    for (i = 0; i < module->rev_size; i++) {
        if (module->rev[i].dsc || module->rev[i].ref) {
            fprintf(f, "%*srevision \"%s\" {\n", LEVEL, INDENT, module->rev[i].date);
            level++;
            if (module->rev[i].dsc) {
                yang_print_text(f, level, "description", module->rev[i].dsc);
            }
            if (module->rev[i].ref) {
                yang_print_text(f, level, "reference", module->rev[i].ref);
            }
            level--;
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        } else {
            yang_print_text(f, level, "revision", module->rev[i].date);
        }
    }

    for (i = 0; i < module->features_size; i++) {
        yang_print_feature(f, level, &module->features[i]);
    }

    for (i = 0; i < module->ident_size; i++) {
        yang_print_identity(f, level, &module->ident[i]);
    }

    for (i = 0; i < module->tpdf_size; i++) {
        yang_print_typedef(f, level, module, &module->tpdf[i]);
    }

    LY_TREE_FOR(module->data, mnode) {
        yang_print_mnode(f, level, mnode,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    for (i = 0; i < module->augment_size; i++) {
        yang_print_augment(f, level, module, &module->augment[i]);
    }

    LY_TREE_FOR(module->rpc, mnode) {
        yang_print_rpc(f, level, mnode);
    }

    LY_TREE_FOR(module->notif, mnode) {
        yang_print_notif(f, level, mnode);
    }

    fprintf(f, "}\n");

    return EXIT_SUCCESS;
#undef LEVEL
}
