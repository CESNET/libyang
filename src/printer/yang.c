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

static void yang_print_mnode(FILE *f, int level, struct ly_mnode *mnode, int mask);

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

/*
 * Covers:
 * description, reference, status
 */
static void
yang_print_mnode_common(FILE *f, int level, struct ly_mnode *mnode)
{
    if (mnode->flags & LY_NODE_STATUS_CURR) {
        fprintf(f, "%*sstatus \"current\";\n", LEVEL, INDENT);
    } else if (mnode->flags & LY_NODE_STATUS_DEPRC) {
        fprintf(f, "%*sstatus \"deprecated\";\n", LEVEL, INDENT);
    } else if (mnode->flags & LY_NODE_STATUS_OBSLT) {
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
yang_print_mnode_common2(FILE *f, int level, struct ly_mnode *mnode)
{
    if (!mnode->parent || (mnode->parent->flags & LY_NODE_CONFIG_MASK) != (mnode->flags & LY_NODE_CONFIG_MASK)) {
        /* print config only when it differs from the parent or in root */
        if (mnode->flags & LY_NODE_CONFIG_W) {
            fprintf(f, "%*sconfig \"true\";\n", LEVEL, INDENT);
        } else if (mnode->flags & LY_NODE_CONFIG_R) {
            fprintf(f, "%*sconfig \"false\";\n", LEVEL, INDENT);
        }
    }

    if (mnode->flags & LY_NODE_MAND_TRUE) {
        fprintf(f, "%*smandatory \"true\";\n", LEVEL, INDENT);
    } else if (mnode->flags & LY_NODE_MAND_FALSE) {
        fprintf(f, "%*smandatory \"false\";\n", LEVEL, INDENT);
    }

    yang_print_mnode_common(f, level, mnode);
}

static void
yang_print_type(FILE *f, int level, struct ly_module *module, struct ly_type *type)
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
            fprintf(f, "%*slength \"%s\";\n", LEVEL, INDENT, type->info.binary.length);
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            fprintf(f, "%*sbit %s {\n", LEVEL, INDENT, type->info.bits.bit[i].value);
            level++;
            yang_print_mnode_common(f, level, (struct ly_mnode *)&type->info.bits.bit[i]);
            fprintf(f, "%*sposition %u;\n", LEVEL, INDENT, type->info.bits.bit[i].pos);
            level--;
            fprintf(f, "%*s}\n", LEVEL, INDENT);
        }
        break;
    case LY_TYPE_DEC64:
        fprintf(f, "%*sfraction-digits %d;\n", LEVEL, INDENT, type->info.dec64.dig);
        /* TODO range incomplete */
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            fprintf(f, "%*senum %s {\n", LEVEL, INDENT, type->info.enums.list[i].name);
            level++;
            yang_print_mnode_common(f, level, (struct ly_mnode *)&type->info.enums.list[i]);
            fprintf(f, "%*svalue %d;\n", LEVEL, INDENT, type->info.enums.list[i].value);
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
        /* TODO */
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        /* TODO wrong - incomplete */
        /*if (type->info.num.range != NULL) {
         * fprintf(f, "%*srange \"%s\";\n", LEVEL, INDENT, type->info.num.range);
         * } */
        break;
    case LY_TYPE_LEAFREF:
        fprintf(f, "%*spath \"%s\";\n", LEVEL, INDENT, type->info.lref.path);
        break;
    case LY_TYPE_STRING:
        /* TODO length, pattern (same as range) incomplete */
        break;
    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; ++i) {
            yang_print_type(f, level, module, &type->info.uni.type[i]);
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
yang_print_must(FILE *f, int level, struct ly_must *must)
{
    fprintf(f, "%*smust \"%s\" {\n", LEVEL, INDENT, must->cond);
    level++;

    if (must->eapptag != NULL) {
        fprintf(f, "%*serror-app-tag \"%s\";\n", LEVEL, INDENT, must->eapptag);
    }

    if (must->emsg != NULL) {
        yang_print_text(f, level, "error-message", must->emsg);
    }
    if (must->dsc != NULL) {
        yang_print_text(f, level, "description", must->dsc);
    }
    if (must->ref != NULL) {
        yang_print_text(f, level, "reference", must->ref);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_refine(FILE *f, int level, struct ly_refine *refine)
{
    int i;

    fprintf(f, "%*srefine \"%s\" {\n", LEVEL, INDENT, refine->target);
    level++;

    yang_print_mnode_common2(f, level, (struct ly_mnode *)refine);

    for (i = 0; i < refine->must_size; ++i) {
        yang_print_must(f, level, &refine->must[i]);
    }

    if (refine->target_type & (LY_NODE_LEAF | LY_NODE_CHOICE)) {
        if (refine->mod.dflt != NULL) {
            fprintf(f, "%*sdefault \"%s\";\n", LEVEL, INDENT, refine->mod.dflt);
        }
    } else if (refine->target_type == LY_NODE_CONTAINER) {
        if (refine->mod.presence != NULL) {
            yang_print_text(f, level, "presence", refine->mod.presence);
        }
    } else if (refine->target_type & (LY_NODE_LIST | LY_NODE_LEAFLIST)) {
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
yang_print_typedef(FILE *f, int level, struct ly_module *module, struct ly_tpdf *tpdf)
{
    fprintf(f, "%*stypedef %s {\n", LEVEL, INDENT, tpdf->name);
    level++;

    yang_print_mnode_common(f, level, (struct ly_mnode *)tpdf);
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
yang_print_identity(FILE *f, int level, struct ly_ident *ident)
{
    fprintf(f, "%*sidentity %s {\n", LEVEL, INDENT, ident->name);
    level++;

    yang_print_mnode_common(f, level, (struct ly_mnode *)ident);
    if (ident->base) {
        if (ident->base->module == ident->module) {
            fprintf(f, "%*sbase %s;\n", LEVEL, INDENT, ident->base->name);
        } else {
            fprintf(f, "%*sbase %s:%s;\n", LEVEL, INDENT, ident->base->module->prefix, ident->base->name);
        }
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);

}

static void
yang_print_container(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode *sub;
    struct ly_mnode_container *cont = (struct ly_mnode_container *)mnode;

    fprintf(f, "%*scontainer %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    if (cont->presence != NULL) {
        yang_print_text(f, level, "presence", cont->presence);
    }

    for (i = 0; i < cont->must_size; i++) {
        yang_print_must(f, level, &cont->must[i]);
    }

    yang_print_mnode_common2(f, level, mnode);

    for (i = 0; i < cont->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &cont->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
                         LY_NODE_USES | LY_NODE_GROUPING | LY_NODE_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_case(FILE *f, int level, struct ly_mnode *mnode)
{
    struct ly_mnode *sub;
    struct ly_mnode_case *cas = (struct ly_mnode_case *)mnode;

    fprintf(f, "%*scase %s {\n", LEVEL, INDENT, cas->name);
    level++;
    yang_print_mnode_common2(f, level, mnode);

    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
                         LY_NODE_USES | LY_NODE_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_choice(FILE *f, int level, struct ly_mnode *mnode)
{
    struct ly_mnode *sub;
    struct ly_mnode_choice *choice = (struct ly_mnode_choice *)mnode;

    fprintf(f, "%*schoice %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    if (choice->dflt != NULL) {
        fprintf(f, "%*sdefault \"%s\";\n", LEVEL, INDENT, choice->dflt->name);
    }

    yang_print_mnode_common2(f, level, mnode);
    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML | LY_NODE_CASE);
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_leaf(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode_leaf *leaf = (struct ly_mnode_leaf *)mnode;

    fprintf(f, "%*sleaf %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_mnode_common2(f, level, mnode);
    for (i = 0; i < leaf->must_size; i++) {
        yang_print_must(f, level, &leaf->must[i]);
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
yang_print_anyxml(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode_anyxml *anyxml = (struct ly_mnode_anyxml *)mnode;

    fprintf(f, "%*sanyxml %s {\n", LEVEL, INDENT, anyxml->name);
    level++;
    yang_print_mnode_common2(f, level, mnode);
    for (i = 0; i < anyxml->must_size; i++) {
        yang_print_must(f, level, &anyxml->must[i]);
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_leaflist(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode_leaflist *llist = (struct ly_mnode_leaflist *)mnode;

    fprintf(f, "%*sleaf-list %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_mnode_common2(f, level, mnode);
    if (llist->flags & LY_NODE_USERORDERED) {
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
    yang_print_type(f, level, mnode->module, &llist->type);
    if (llist->units != NULL) {
        fprintf(f, "%*sunits \"%s\";\n", LEVEL, INDENT, llist->units);
    }
    level--;

    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_list(FILE *f, int level, struct ly_mnode *mnode)
{
    int i, j;
    struct ly_mnode *sub;
    struct ly_unique *uniq;
    struct ly_mnode_list *list = (struct ly_mnode_list *)mnode;

    fprintf(f, "%*slist %s {\n", LEVEL, INDENT, mnode->name);
    level++;
    yang_print_mnode_common2(f, level, mnode);

    if (list->keys_size) {
        fprintf(f, "%*skey \"", LEVEL, INDENT);
        for (i = 0; i < list->keys_size; i++) {
            fprintf(f, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? " " : "");
        }
        fprintf(f, "\";\n");
    }

    for (i = 0; i < list->unique_size; i++) {
        uniq = &list->unique[i];
        fprintf(f, "%*sunique \"", LEVEL, INDENT);
        for (j = 0; j < uniq->leafs_size; j++) {
            fprintf(f, "%s%s", uniq->leafs[j]->name, j + 1 < uniq->leafs_size ? " " : "");
        }
        fprintf(f, "\";\n");
    }

    if (list->flags & LY_NODE_USERORDERED) {
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

    for (i = 0; i < list->tpdf_size; i++) {
        yang_print_typedef(f, level, list->module, &list->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
                         LY_NODE_USES | LY_NODE_GROUPING | LY_NODE_ANYXML);
    }
    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_grouping(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode *node;
    struct ly_mnode_grp *grp = (struct ly_mnode_grp *)mnode;

    fprintf(f, "%*sgrouping %s {\n", LEVEL, INDENT, mnode->name);
    level++;

    yang_print_mnode_common(f, level, mnode);

    for (i = 0; i < grp->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &grp->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, node) {
        yang_print_mnode(f, level, node,
                         LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
                         LY_NODE_USES | LY_NODE_GROUPING | LY_NODE_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_uses(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode_uses *uses = (struct ly_mnode_uses *)mnode;

    fprintf(f, "%*suses %s {\n", LEVEL, INDENT, uses->name);
    level++;

    yang_print_mnode_common(f, level, mnode);

    for (i = 0; i < uses->refine_size; i++) {
        yang_print_refine(f, level, &uses->refine[i]);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_input_output(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode *sub;
    struct ly_mnode_input_output *inout = (struct ly_mnode_input_output *)mnode;

    fprintf(f, "%*s%s {\n", LEVEL, INDENT, (inout->nodetype == LY_NODE_INPUT ? "input" : "output"));

    level++;
    for (i = 0; i < inout->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &inout->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
                         LY_NODE_USES | LY_NODE_GROUPING | LY_NODE_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_rpc(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode *sub;
    struct ly_mnode_rpc *rpc = (struct ly_mnode_rpc *)mnode;

    fprintf(f, "%*srpc %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_mnode_common(f, level, mnode);

    for (i = 0; i < rpc->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &rpc->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LY_NODE_GROUPING | LY_NODE_INPUT | LY_NODE_OUTPUT);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_notif(FILE *f, int level, struct ly_mnode *mnode)
{
    int i;
    struct ly_mnode *sub;
    struct ly_mnode_notif *notif = (struct ly_mnode_notif *)mnode;

    fprintf(f, "%*snotification %s {\n", LEVEL, INDENT, mnode->name);

    level++;
    yang_print_mnode_common(f, level, mnode);

    for (i = 0; i < notif->tpdf_size; i++) {
        yang_print_typedef(f, level, mnode->module, &notif->tpdf[i]);
    }

    LY_TREE_FOR(mnode->child, sub) {
        yang_print_mnode(f, level, sub,
                         LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
                         LY_NODE_USES | LY_NODE_GROUPING | LY_NODE_ANYXML);
    }

    level--;
    fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_mnode(FILE *f, int level, struct ly_mnode *mnode, int mask)
{
    switch (mnode->nodetype & mask) {
    case LY_NODE_CONTAINER:
        yang_print_container(f, level, mnode);
        break;
    case LY_NODE_CHOICE:
        yang_print_choice(f, level, mnode);
        break;
    case LY_NODE_LEAF:
        yang_print_leaf(f, level, mnode);
        break;
    case LY_NODE_LEAFLIST:
        yang_print_leaflist(f, level, mnode);
        break;
    case LY_NODE_LIST:
        yang_print_list(f, level, mnode);
        break;
    case LY_NODE_USES:
        yang_print_uses(f, level, mnode);
        break;
    case LY_NODE_GROUPING:
        yang_print_grouping(f, level, mnode);
        break;
    case LY_NODE_ANYXML:
        yang_print_anyxml(f, level, mnode);
        break;
    case LY_NODE_CASE:
        yang_print_case(f, level, mnode);
        break;
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
        yang_print_input_output(f, level, mnode);
        break;
    default:
        break;
    }
}

int
yang_print_model(FILE *f, struct ly_module *module)
{
    unsigned int i;
    int level = 0;
#define LEVEL (level*2)

    struct ly_mnode *mnode;

    fprintf(f, "module %s {\n", module->name);
    level++;

    fprintf(f, "%*snamespace \"%s\";\n", LEVEL, INDENT, module->ns);
    fprintf(f, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->prefix);

    if (module->version) {
        fprintf(f, "%*syang-version \"%s\";\n", LEVEL, INDENT, module->version == 1 ? "1.0" : "1.1");
    }

    for (i = 0; i < module->imp_size; i++) {
        fprintf(f, "%*simport \"%s\" {\n", LEVEL, INDENT, module->imp[i].module->name);
        level++;
        yang_print_text(f, level, "prefix", module->imp[i].prefix);
        if (module->imp[i].rev[0]) {
            yang_print_text(f, level, "revision-date", module->imp[i].rev);
        }
        level--;
        fprintf(f, "%*s}\n", LEVEL, INDENT);
    }

    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].rev[0]) {
            fprintf(f, "%*sinclude \"%s\" {\n", LEVEL, INDENT, module->inc[i].submodule->name);
            yang_print_text(f, level + 1, "revision-date", module->imp[i].rev);
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

    for (i = 0; i < module->ident_size; i++) {
        yang_print_identity(f, level, &module->ident[i]);
    }

    for (i = 0; i < module->tpdf_size; i++) {
        yang_print_typedef(f, level, module, &module->tpdf[i]);
    }

    LY_TREE_FOR(module->data, mnode) {
        yang_print_mnode(f, level, mnode,
                         LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
                         LY_NODE_USES | LY_NODE_GROUPING | LY_NODE_ANYXML);
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
