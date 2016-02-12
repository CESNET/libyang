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
#include <stdint.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"

#define INDENT ""
#define LEVEL (level*2)

static void yang_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask);

static void
yang_encode(struct lyout *out, const char *text, int len)
{
    int i, start_len;
    const char *start;
    char special = 0;

    if (!len) {
        return;
    }

    if (len < 0) {
        len = strlen(text);
    }

    start = text;
    start_len = 0;
    for (i = 0; i < len; ++i) {
        switch (text[i]) {
        case '\n':
        case '\t':
        case '\"':
        case '\\':
            special = text[i];
            break;
        default:
            ++start_len;
            break;
        }

        if (special) {
            ly_write(out, start, start_len);
            switch (special) {
            case '\n':
                ly_write(out, "\\n", 2);
                break;
            case '\t':
                ly_write(out, "\\t", 2);
                break;
            case '\"':
                ly_write(out, "\\\"", 2);
                break;
            case '\\':
                ly_write(out, "\\\\", 2);
                break;
            }

            start += start_len + 1;
            start_len = 0;

            special = 0;
        }
    }

    ly_write(out, start, start_len);
}

static void
yang_print_open(struct lyout *out, int *flag)
{
    if (flag && !*flag) {
        *flag = 1;
        ly_print(out, " {\n");
    }
}

static void
yang_print_close(struct lyout *out, int level, int flag)
{
    if (flag) {
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    } else {
        ly_print(out, ";\n");
    }
}

static void
yang_print_text(struct lyout *out, int level, const char *name, const char *text, int singleline)
{
    const char *s, *t;

    if (singleline) {
        ly_print(out, "%*s%s \"", LEVEL, INDENT, name);
    } else {
        ly_print(out, "%*s%s\n", LEVEL, INDENT, name);
        level++;

        ly_print(out, "%*s\"", LEVEL, INDENT);
    }
    t = text;
    while ((s = strchr(t, '\n'))) {
        yang_encode(out, t, s - t);
        ly_print(out, "\n");
        t = s + 1;
        ly_print(out, "%*s ", LEVEL, INDENT);
    }

    ly_print(out, "%s\";\n", t);
    level--;

}

static void
yang_print_nacmext(struct lyout *out, int level, const struct lys_node *node, const struct lys_module *module, int *flag)
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
            yang_print_open(out, flag);
            ly_print(out, "%*s%s:default-deny-write;\n", LEVEL, INDENT, prefix);
        }
        if ((node->nacm & LYS_NACM_DENYA) && (!node->parent || !(node->parent->nacm & LYS_NACM_DENYA))) {
            yang_print_open(out, flag);
            ly_print(out, "%*s%s:default-deny-all;\n", LEVEL, INDENT, prefix);
        }
    }
}

/*
 * Covers:
 * description, reference, status
 */
static void
yang_print_snode_common(struct lyout *out, int level, const struct lys_node *node, int *flag)
{
    if (node->flags & LYS_STATUS_CURR) {
        yang_print_open(out, flag);
        ly_print(out, "%*sstatus \"current\";\n", LEVEL, INDENT);
    } else if (node->flags & LYS_STATUS_DEPRC) {
        yang_print_open(out, flag);
        ly_print(out, "%*sstatus \"deprecated\";\n", LEVEL, INDENT);
    } else if (node->flags & LYS_STATUS_OBSLT) {
        yang_print_open(out, flag);
        ly_print(out, "%*sstatus \"obsolete\";\n", LEVEL, INDENT);
    }

    if (node->dsc) {
        yang_print_open(out, flag);
        yang_print_text(out, level, "description", node->dsc, 0);
    }
    if (node->ref) {
        yang_print_open(out, flag);
        yang_print_text(out, level, "reference", node->ref, 0);
    }
}

/*
 * Covers:
 * config, mandatory
 * description, reference, status
 */
static void
yang_print_snode_common2(struct lyout *out, int level, const struct lys_node *node, int *flag)
{
    if (node->parent) {
        if ((node->parent->flags & LYS_CONFIG_MASK) != (node->flags & LYS_CONFIG_MASK)) {
            /* print config when it differs from the parent ... */
            if (node->flags & LYS_CONFIG_W) {
                yang_print_open(out, flag);
                ly_print(out, "%*sconfig true;\n", LEVEL, INDENT);
            } else if (node->flags & LYS_CONFIG_R) {
                yang_print_open(out, flag);
                ly_print(out, "%*sconfig false;\n", LEVEL, INDENT);
            }
        }
    } else if (node->flags & LYS_CONFIG_R) {
        /* ... or it's a top-level state node */
        yang_print_open(out, flag);
        ly_print(out, "%*sconfig false;\n", LEVEL, INDENT);
    }

    if (node->flags & LYS_MAND_TRUE) {
        yang_print_open(out, flag);
        ly_print(out, "%*smandatory true;\n", LEVEL, INDENT);
    } else if (node->flags & LYS_MAND_FALSE) {
        yang_print_open(out, flag);
        ly_print(out, "%*smandatory false;\n", LEVEL, INDENT);
    }

    yang_print_snode_common(out, level, node, flag);
}

static void
yang_print_iffeature(struct lyout *out, int level, const struct lys_module *module, const struct lys_feature *feat)
{
    struct lys_module *mod;

    ly_print(out, "%*sif-feature ", LEVEL, INDENT);
    mod = (feat->module->type ? ((struct lys_submodule *)feat->module)->belongsto : feat->module);
    if (module != mod) {
        ly_print(out, "%s:", transform_module_name2import_prefix(module, mod->name));
    }
    ly_print(out, "%s;\n", feat->name);
}

static void
yang_print_feature(struct lyout *out, int level, const struct lys_feature *feat)
{
    int i, flag = 0;

    ly_print(out, "%*sfeature %s", LEVEL, INDENT, feat->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)feat, &flag);
    for (i = 0; i < feat->features_size; ++i) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, feat->module, feat->features[i]);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_restr(struct lyout *out, int level, const struct lys_restr *restr, int *flag)
{
    if (restr->dsc != NULL) {
        yang_print_open(out, flag);
        yang_print_text(out, level, "description", restr->dsc, 0);
    }
    if (restr->ref != NULL) {
        yang_print_open(out, flag);
        yang_print_text(out, level, "reference", restr->ref, 0);
    }
    if (restr->eapptag != NULL) {
        yang_print_open(out, flag);
        ly_print(out, "%*serror-app-tag \"%s\";\n", LEVEL, INDENT, restr->eapptag);
    }
    if (restr->emsg != NULL) {
        yang_print_open(out, flag);
        yang_print_text(out, level, "error-message", restr->emsg, 0);
    }
}

static void
yang_print_when(struct lyout *out, int level, const struct lys_module *module, const struct lys_when *when)
{
    int flag = 0;
    const char *str;

    str = transform_json2schema(module, when->cond);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*swhen \"", LEVEL, INDENT);
    yang_encode(out, str, -1);
    ly_print(out, "\"");
    lydict_remove(module->ctx, str);

    level++;
    if (when->dsc) {
        yang_print_open(out, &flag);
        yang_print_text(out, level, "description", when->dsc, 0);
    }
    if (when->ref) {
        yang_print_open(out, &flag);
        yang_print_text(out, level, "reference", when->ref, 0);
    }
    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_type(struct lyout *out, int level, const struct lys_module *module, const struct lys_type *type)
{
    int i;
    int flag = 0, flag2;
    const char *str;
    struct lys_module *mod;

    if (type->module_name) {
        ly_print(out, "%*stype %s:%s", LEVEL, INDENT,
                 transform_module_name2import_prefix(module, type->module_name), type->der->name);
    } else {
        ly_print(out, "%*stype %s", LEVEL, INDENT, type->der->name);
    }
    level++;
    switch (type->base) {
    case LY_TYPE_BINARY:
        if (type->info.binary.length != NULL) {
            yang_print_open(out, &flag);
            ly_print(out, "%*slength \"", LEVEL, INDENT);
            yang_encode(out, type->info.binary.length->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, type->info.binary.length, &flag2);
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            yang_print_open(out, &flag);
            ly_print(out, "%*sbit %s", LEVEL, INDENT, type->info.bits.bit[i].name);
            flag2 = 0;
            level++;
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.bits.bit[i], &flag2);
            if (!(type->info.bits.bit[i].flags & LYS_AUTOASSIGNED)) {
                yang_print_open(out, &flag2);
                ly_print(out, "%*sposition %u;\n", LEVEL, INDENT, type->info.bits.bit[i].pos);
            }
            level--;
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_DEC64:
        yang_print_open(out, &flag);
        ly_print(out, "%*sfraction-digits %d;\n", LEVEL, INDENT, type->info.dec64.dig);
        if (type->info.dec64.range != NULL) {
            ly_print(out, "%*srange \"", LEVEL, INDENT);
            yang_encode(out, type->info.dec64.range->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, type->info.dec64.range, &flag2);
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            yang_print_open(out, &flag);
            ly_print(out, "%*senum \"%s\"", LEVEL, INDENT, type->info.enums.enm[i].name);
            flag2 = 0;
            level++;
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.enums.enm[i], &flag2);
            if (!(type->info.enums.enm[i].flags & LYS_AUTOASSIGNED)) {
                yang_print_open(out, &flag2);
                ly_print(out, "%*svalue %d;\n", LEVEL, INDENT, type->info.enums.enm[i].value);
            }
            level--;
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_IDENT:
        yang_print_open(out, &flag);
        mod = type->info.ident.ref->module->type ?
                        ((struct lys_submodule *)type->info.ident.ref->module)->belongsto :
                        type->info.ident.ref->module;
        if (module == mod) {
            ly_print(out, "%*sbase %s;\n", LEVEL, INDENT, type->info.ident.ref->name);
        } else {
            ly_print(out, "%*sbase %s:%s;\n", LEVEL, INDENT, transform_module_name2import_prefix(module, mod->name),
                     type->info.ident.ref->name);
        }
        break;
    case LY_TYPE_INST:
        if (type->info.inst.req == 1) {
            yang_print_open(out, &flag);
            ly_print(out, "%*srequire-instance true;\n", LEVEL, INDENT);
        } else if (type->info.inst.req == -1) {
            yang_print_open(out, &flag);
            ly_print(out, "%*srequire-instance false;\n", LEVEL, INDENT);
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
            yang_print_open(out, &flag);
            ly_print(out, "%*srange \"", LEVEL, INDENT);
            yang_encode(out, type->info.num.range->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, type->info.num.range, &flag2);
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_LEAFREF:
        yang_print_open(out, &flag);
        str = transform_json2schema(module, type->info.lref.path);
        ly_print(out, "%*spath \"%s\";\n", LEVEL, INDENT, str);
        lydict_remove(module->ctx, str);
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length) {
            yang_print_open(out, &flag);
            ly_print(out, "%*slength \"", LEVEL, INDENT);
            yang_encode(out, type->info.str.length->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, type->info.str.length, &flag2);
            yang_print_close(out, level, flag2);
        }
        for (i = 0; i < type->info.str.pat_count; i++) {
            yang_print_open(out, &flag);
            ly_print(out, "%*spattern \"", LEVEL, INDENT);
            yang_encode(out, type->info.str.patterns[i].expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, &type->info.str.patterns[i], &flag2);
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; ++i) {
            yang_print_open(out, &flag);
            yang_print_type(out, level, module, &type->info.uni.types[i]);
        }
        break;
    default:
        /* other types do not have substatements */
        break;
    }
    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_must(struct lyout *out, int level, const struct lys_module *module, const struct lys_restr *must)
{
    int flag = 0;
    const char *str;

    str = transform_json2schema(module, must->expr);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*smust \"", LEVEL, INDENT);
    yang_encode(out, str, -1);
    ly_print(out, "\"");
    lydict_remove(module->ctx, str);

    yang_print_restr(out, level + 1, must, &flag);
    yang_print_close(out, level, flag);
}

static void
yang_print_unique(struct lyout *out, int level, const struct lys_unique *uniq)
{
    int i;

    ly_print(out, "%*sunique \"", LEVEL, INDENT);
    for (i = 0; i < uniq->expr_size; i++) {
        ly_print(out, "%s%s", uniq->expr[i], i + 1 < uniq->expr_size ? " " : "");
    }
    ly_print(out, "\";\n");
}

static void
yang_print_refine(struct lyout *out, int level, const struct lys_module *module, const struct lys_refine *refine)
{
    int i;
    const char *str;

    str = transform_json2schema(module, refine->target_name);
    ly_print(out, "%*srefine \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    if (refine->flags & LYS_CONFIG_W) {
        ly_print(out, "%*sconfig true;\n", LEVEL, INDENT);
    } else if (refine->flags & LYS_CONFIG_R) {
        ly_print(out, "%*sconfig false;\n", LEVEL, INDENT);
    }

    if (refine->flags & LYS_MAND_TRUE) {
        ly_print(out, "%*smandatory true;\n", LEVEL, INDENT);
    } else if (refine->flags & LYS_MAND_FALSE) {
        ly_print(out, "%*smandatory false;\n", LEVEL, INDENT);
    }

    yang_print_snode_common(out, level, (struct lys_node *)refine, NULL);

    for (i = 0; i < refine->must_size; ++i) {
        yang_print_must(out, level, module, &refine->must[i]);
    }

    if (refine->target_type & (LYS_LEAF | LYS_CHOICE)) {
        if (refine->mod.dflt != NULL) {
            ly_print(out, "%*sdefault \"%s\";\n", LEVEL, INDENT, refine->mod.dflt);
        }
    } else if (refine->target_type == LYS_CONTAINER) {
        if (refine->mod.presence != NULL) {
            yang_print_text(out, level, "presence", refine->mod.presence, 1);
        }
    } else if (refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
        /* magic - bit 3 in flags means min set, bit 4 says max set */
        if (refine->flags & 0x04) {
            ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, refine->mod.list.min);
        }
        if (refine->flags & 0x08) {
            if (refine->mod.list.max) {
                ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, refine->mod.list.max);
            } else {
                ly_print(out, "%*smax-elements \"unbounded\";\n", LEVEL, INDENT);
            }
        }
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_deviation(struct lyout *out, int level, const struct lys_module *module,
                     const struct lys_deviation *deviation)
{
    int i, j;
    const char *str;

    str = transform_json2schema(module, deviation->target_name);
    ly_print(out, "%*sdeviation \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    if (deviation->dsc) {
        yang_print_text(out, level, "description", deviation->dsc, 0);
    }
    if (deviation->ref) {
        yang_print_text(out, level, "reference", deviation->ref, 0);
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
            ly_print(out, "%*sconfig true;\n", LEVEL, INDENT);
        } else if (deviation->deviate[i].flags & LYS_CONFIG_R) {
            ly_print(out, "%*sconfig false;\n", LEVEL, INDENT);
        }

        if (deviation->deviate[i].flags & LYS_MAND_TRUE) {
            ly_print(out, "%*smandatory true;\n", LEVEL, INDENT);
        } else if (deviation->deviate[i].flags & LYS_MAND_FALSE) {
            ly_print(out, "%*smandatory false;\n", LEVEL, INDENT);
        }

        if (deviation->deviate[i].dflt) {
            ly_print(out, "%*sdefault %s;\n", LEVEL, INDENT, deviation->deviate[i].dflt);
        }

        if (deviation->deviate[i].min_set) {
            ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, deviation->deviate[i].min);
        }
        if (deviation->deviate[i].max_set) {
            if (deviation->deviate[i].max) {
                ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, deviation->deviate[i].max);
            } else {
                ly_print(out, "%*smax-elements \"unbounded\";\n", LEVEL, INDENT);
            }
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
yang_print_augment(struct lyout *out, int level, const struct lys_module *module,
                   const struct lys_node_augment *augment)
{
    int i;
    struct lys_node *sub;
    const char *str;

    str = transform_json2schema(module, augment->target_name);
    ly_print(out, "%*saugment \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    yang_print_nacmext(out, level, (struct lys_node *)augment, module, NULL);
    yang_print_snode_common(out, level, (struct lys_node *)augment, NULL);

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
yang_print_typedef(struct lyout *out, int level, const struct lys_module *module, const struct lys_tpdf *tpdf)
{
    ly_print(out, "%*stypedef %s {\n", LEVEL, INDENT, tpdf->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)tpdf, NULL);
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
yang_print_identity(struct lyout *out, int level, const struct lys_ident *ident)
{
    int flag = 0;
    struct lys_module *mod;

    ly_print(out, "%*sidentity %s", LEVEL, INDENT, ident->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)ident, &flag);
    if (ident->base) {
        yang_print_open(out, &flag);
        ly_print(out, "%*sbase ", LEVEL, INDENT);
        mod = (ident->base->module->type ? ((struct lys_submodule *)ident->base->module)->belongsto : ident->base->module);
        if (ident->module != mod) {
            ly_print(out, "%s:", transform_module_name2import_prefix(ident->module, mod->name));
        }
        ly_print(out, "%s;\n", ident->base->name);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_container(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node *sub;
    struct lys_node_container *cont = (struct lys_node_container *)node;

    ly_print(out, "%*scontainer %s", LEVEL, INDENT, node->name);

    level++;

    yang_print_nacmext(out, level, node, node->module, &flag);

    if (cont->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, cont->when);
    }

    for (i = 0; i < cont->features_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, cont->features[i]);
    }

    for (i = 0; i < cont->must_size; i++) {
        yang_print_open(out, &flag);
        yang_print_must(out, level, node->module, &cont->must[i]);
    }

    if (cont->presence != NULL) {
        yang_print_open(out, &flag);
        yang_print_text(out, level, "presence", cont->presence, 1);
    }

    yang_print_snode_common2(out, level, node, &flag);

    for (i = 0; i < cont->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, node->module, &cont->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment and data from submodules */
        if (sub->module != node->module) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_case(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_case *cas = (struct lys_node_case *)node;

    ly_print(out, "%*scase %s {\n", LEVEL, INDENT, cas->name);
    level++;
    yang_print_nacmext(out, level, node, node->module, NULL);
    yang_print_snode_common2(out, level, node, NULL);

    for (i = 0; i < cas->features_size; i++) {
        yang_print_iffeature(out, level, node->module, cas->features[i]);
    }

    if (cas->when) {
        yang_print_when(out, level, node->module, cas->when);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment and data from submodules */
        if (sub->module != node->module) {
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
yang_print_choice(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    ly_print(out, "%*schoice %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_nacmext(out, level, node, node->module, NULL);
    if (choice->dflt != NULL) {
        ly_print(out, "%*sdefault \"%s\";\n", LEVEL, INDENT, choice->dflt->name);
    }

    yang_print_snode_common2(out, level, node, NULL);

    for (i = 0; i < choice->features_size; i++) {
        yang_print_iffeature(out, level, node->module, choice->features[i]);
    }

    if (choice->when) {
        yang_print_when(out, level, node->module, choice->when);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment and data from submodules */
        if (sub->module != node->module) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_CASE);
    }
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_leaf(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;

    ly_print(out, "%*sleaf %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_nacmext(out, level, node, node->module, NULL);
    if (leaf->when) {
        yang_print_when(out, level, node->module, leaf->when);
    }
    for (i = 0; i < leaf->features_size; i++) {
        yang_print_iffeature(out, level, node->module, leaf->features[i]);
    }
    for (i = 0; i < leaf->must_size; i++) {
        yang_print_must(out, level, node->module, &leaf->must[i]);
    }
    yang_print_snode_common2(out, level, node, NULL);
    yang_print_type(out, level, node->module, &leaf->type);
    if (leaf->units != NULL) {
        ly_print(out, "%*sunits \"%s\";\n", LEVEL, INDENT, leaf->units);
    }
    if (leaf->dflt != NULL) {
        switch (leaf->type.base) {
        case LY_TYPE_STRING:
        case LY_TYPE_LEAFREF:
        case LY_TYPE_IDENT:
        case LY_TYPE_UNION:
            ly_print(out, "%*sdefault \"%s\";\n", LEVEL, INDENT, leaf->dflt);
            break;
        default:
            ly_print(out, "%*sdefault %s;\n", LEVEL, INDENT, leaf->dflt);
            break;
        }
    }
    level--;

    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_anyxml(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node_anyxml *anyxml = (struct lys_node_anyxml *)node;

    ly_print(out, "%*sanyxml %s", LEVEL, INDENT, anyxml->name);
    level++;
    yang_print_nacmext(out, level, node, node->module, &flag);
    yang_print_snode_common2(out, level, node, &flag);
    for (i = 0; i < anyxml->features_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, anyxml->features[i]);
    }
    for (i = 0; i < anyxml->must_size; i++) {
        yang_print_open(out, &flag);
        yang_print_must(out, level, node->module, &anyxml->must[i]);
    }
    if (anyxml->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, anyxml->when);
    }
    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_leaflist(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;

    ly_print(out, "%*sleaf-list %s {\n", LEVEL, INDENT, node->name);

    level++;
    yang_print_nacmext(out, level, node, node->module, NULL);
    if (llist->when) {
        yang_print_when(out, level, llist->module, llist->when);
    }
    for (i = 0; i < llist->features_size; i++) {
        yang_print_iffeature(out, level, node->module, llist->features[i]);
    }
    for (i = 0; i < llist->must_size; i++) {
        yang_print_must(out, level, node->module, &llist->must[i]);
    }
    yang_print_snode_common2(out, level, node, NULL);
    yang_print_type(out, level, node->module, &llist->type);
    if (llist->units != NULL) {
        ly_print(out, "%*sunits \"%s\";\n", LEVEL, INDENT, llist->units);
    }
    if (llist->min > 0) {
        ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, llist->min);
    }
    if (llist->max > 0) {
        ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, llist->max);
    }
    if (llist->flags & LYS_USERORDERED) {
        ly_print(out, "%*sordered-by user;\n", LEVEL, INDENT);
    }
    level--;

    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_list(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;

    ly_print(out, "%*slist %s {\n", LEVEL, INDENT, node->name);
    level++;
    yang_print_nacmext(out, level, node, node->module, NULL);
    if (list->when) {
        yang_print_when(out, level, list->module, list->when);
    }
    for (i = 0; i < list->features_size; i++) {
        yang_print_iffeature(out, level, node->module, list->features[i]);
    }
    for (i = 0; i < list->must_size; i++) {
        yang_print_must(out, level, list->module, &list->must[i]);
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
    yang_print_snode_common2(out, level, node, NULL);
    if (list->min > 0) {
        ly_print(out, "%*smin-elements %u;\n", LEVEL, INDENT, list->min);
    }
    if (list->max > 0) {
        ly_print(out, "%*smax-elements %u;\n", LEVEL, INDENT, list->max);
    }
    if (list->flags & LYS_USERORDERED) {
        ly_print(out, "%*sordered-by user;\n", LEVEL, INDENT);
    }

    for (i = 0; i < list->tpdf_size; i++) {
        yang_print_typedef(out, level, list->module, &list->tpdf[i]);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augment and data from submodules */
        if (sub->module != node->module) {
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
yang_print_grouping(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *child;
    struct lys_node_grp *grp = (struct lys_node_grp *)node;

    ly_print(out, "%*sgrouping %s {\n", LEVEL, INDENT, node->name);
    level++;

    yang_print_snode_common(out, level, node, NULL);

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
yang_print_uses(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node_uses *uses = (struct lys_node_uses *)node;
    struct lys_module *mod;

    ly_print(out, "%*suses ", LEVEL, INDENT);
    if (node->child) {
        mod = (node->child->module->type ? ((struct lys_submodule *)node->child->module)->belongsto
              : node->child->module);
        if (node->module != mod) {
            ly_print(out, "%s:", transform_module_name2import_prefix(node->module, mod->name));
        }
    }
    ly_print(out, "%s", uses->name);
    level++;

    yang_print_nacmext(out, level, node, node->module, &flag);
    yang_print_snode_common(out, level, node, &flag);
    for (i = 0; i < uses->features_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, uses->features[i]);
    }
    if (uses->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, uses->when);
    }

    for (i = 0; i < uses->refine_size; i++) {
        yang_print_open(out, &flag);
        yang_print_refine(out, level, node->module, &uses->refine[i]);
    }

    for (i = 0; i < uses->augment_size; i++) {
        yang_print_open(out, &flag);
        yang_print_augment(out, level, node->module, &uses->augment[i]);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_input_output(struct lyout *out, int level, const struct lys_node *node)
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
        /* augment and data from submodules */
        if (sub->module != node->module) {
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
yang_print_rpc(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node *sub;
    struct lys_node_rpc *rpc = (struct lys_node_rpc *)node;

    ly_print(out, "%*srpc %s", LEVEL, INDENT, node->name);

    level++;
    yang_print_snode_common(out, level, node, &flag);

    for (i = 0; i < rpc->features_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, rpc->features[i]);
    }

    for (i = 0; i < rpc->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, node->module, &rpc->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment and data from submodules */
        if (sub->module != node->module) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_GROUPING | LYS_INPUT | LYS_OUTPUT);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_notif(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node *sub;
    struct lys_node_notif *notif = (struct lys_node_notif *)node;

    ly_print(out, "%*snotification %s", LEVEL, INDENT, node->name);

    level++;
    yang_print_snode_common(out, level, node, &flag);

    for (i = 0; i < notif->features_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, notif->features[i]);
    }

    for (i = 0; i < notif->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, node->module, &notif->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augment and data from submodules */
        if (sub->module != node->module) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYXML);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask)
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
yang_print_model(struct lyout *out, const struct lys_module *module)
{
    unsigned int i;
    int level = 0;
#define LEVEL (level*2)

    struct lys_node *node;

    /* (sub)module-header-stmts */
    if (module->type) {
        ly_print(out, "submodule %s {%s\n", module->name, (module->deviated ? " // DEVIATED" : ""));
        level++;
        if (module->version) {
            ly_print(out, "%*syang-version %s;\n", LEVEL, INDENT,
                     ((struct lys_submodule *)module)->belongsto->version == 2 ? "1.1" : "1");
        }
        ly_print(out, "%*sbelongs-to %s {\n", LEVEL, INDENT, ((struct lys_submodule *)module)->belongsto->name);
        level++;
        ly_print(out, "%*sprefix %s;\n", LEVEL, INDENT, module->prefix);
        level--;
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    } else {
        ly_print(out, "module %s {%s\n", module->name, (module->deviated ? " // DEVIATED" : ""));
        level++;
        if (module->version) {
            ly_print(out, "%*syang-version %s;\n", LEVEL, INDENT, module->version == 2 ? "1.1" : "1");
        }
        ly_print(out, "%*snamespace \"%s\";\n", LEVEL, INDENT, module->ns);
        ly_print(out, "%*sprefix %s;\n", LEVEL, INDENT, module->prefix);
    }

    /* linkage-stmts */
    if (module->imp_size || module->inc_size) {
        ly_print(out, "\n");
    }
    for (i = 0; i < module->imp_size; i++) {
        if (module->imp[i].external) {
            continue;
        }
        ly_print(out, "%*simport %s {\n", LEVEL, INDENT, module->imp[i].module->name);
        level++;
        ly_print(out, "%*sprefix %s;\n", LEVEL, INDENT, module->imp[i].prefix);
        if (module->imp[i].rev[0]) {
            ly_print(out, "%*srevision-date %s;", LEVEL, INDENT, module->imp[i].rev);
        }
        level--;
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    }
    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].external) {
            continue;
        }
        if (module->inc[i].rev[0]) {
            ly_print(out, "%*sinclude \"%s\" {\n", LEVEL, INDENT, module->inc[i].submodule->name);
            level++;
            ly_print(out, "%*srevision-date %s;", LEVEL, INDENT, module->inc[i].rev);
            level--;
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        } else {
            ly_print(out, "%*sinclude \"%s\";\n", LEVEL, INDENT, module->inc[i].submodule->name);
        }
    }

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print(out, "\n");
    }
    if (module->org) {
        yang_print_text(out, level, "organization", module->org, 0);
    }
    if (module->contact) {
        yang_print_text(out, level, "contact", module->contact, 0);
    }
    if (module->dsc) {
        yang_print_text(out, level, "description", module->dsc, 0);
    }
    if (module->ref) {
        yang_print_text(out, level, "reference", module->ref, 0);
    }

    /* revision-stmts */
    if (module->rev_size) {
        ly_print(out, "\n");
    }
    for (i = 0; i < module->rev_size; i++) {
        if (module->rev[i].dsc || module->rev[i].ref) {
            ly_print(out, "%*srevision \"%s\" {\n", LEVEL, INDENT, module->rev[i].date);
            level++;
            if (module->rev[i].dsc) {
                yang_print_text(out, level, "description", module->rev[i].dsc, 0);
            }
            if (module->rev[i].ref) {
                yang_print_text(out, level, "reference", module->rev[i].ref, 0);
            }
            level--;
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        } else {
            ly_print(out, "%*srevision %s;", LEVEL, INDENT, module->rev[i].date);
        }
    }

    /* body-stmts */
    for (i = 0; i < module->features_size; i++) {
        ly_print(out, "\n");
        yang_print_feature(out, level, &module->features[i]);
    }

    for (i = 0; i < module->ident_size; i++) {
        ly_print(out, "\n");
        yang_print_identity(out, level, &module->ident[i]);
    }

    for (i = 0; i < module->tpdf_size; i++) {
        ly_print(out, "\n");
        yang_print_typedef(out, level, module, &module->tpdf[i]);
    }

    for (i = 0; i < module->deviation_size; ++i) {
        ly_print(out, "\n");
        yang_print_deviation(out, level, module, &module->deviation[i]);
    }

    LY_TREE_FOR(module->data, node) {
        if (node->module != module) {
            continue;
        }
        ly_print(out, "\n");
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
        ly_print(out, "\n");
        yang_print_augment(out, level, module, &module->augment[i]);
    }

    ly_print(out, "}\n");

    return EXIT_SUCCESS;
#undef LEVEL
}
