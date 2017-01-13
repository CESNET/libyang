/**
 * @file printer/yang.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YANG printer for libyang data model structure
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

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
yang_print_text(struct lyout *out, int level, const char *name, const char *text, int singleline, int closed)
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

    yang_encode(out, t, strlen(t));
    if (closed) {
        ly_print(out, "\";\n");
    } else {
        ly_print(out, "\"");
    }
    level--;

}

static void
yang_print_extension_instances(struct lyout *out, int level, const struct lys_module *module,
                               LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                               struct lys_ext_instance **ext, unsigned int count)
{
    unsigned int u, x;
    struct lys_module *mod;
    const char *prefix = NULL;
    int flag;

    for (u = 0; u < count; u++) {
        if (ext[u]->flags & LYEXT_OPT_INHERIT) {
            /* ignore the inherited extensions which were not explicitely instantiated in the module */
            continue;
        } else if (ext[u]->substmt != substmt || ext[u]->substmt_index != substmt_index) {
            /* do not print the other substatement than the required */
            continue;
        }

        mod = lys_main_module(ext[u]->def->module);
        if (mod == module) {
            prefix = module->prefix;
        } else {
            for (x = 0; x < module->imp_size; x++) {
                if (mod == module->imp[x].module) {
                    prefix = module->imp[x].prefix;
                    break;
                }
            }
        }

        flag = 0;
        ly_print(out, "%*s%s:%s", LEVEL, INDENT, prefix, ext[u]->def->name);
        /* extension - generic part */
        if (ext[u]->arg_value) {
            ly_print(out, " \"%s\"", ext[u]->arg_value);
        }

        /* extension - type-specific part */
        switch(lys_ext_instance_type(ext[u])) {
        case LYEXT_FLAG:
            /* flag extension - nothing special */
            break;
        case LYEXT_ERR:
            LOGINT;
            break;
        }

        /* extensions in extension instance */
        if (ext[u]->ext_size) {
            yang_print_open(out, &flag);
            yang_print_extension_instances(out, level + 1, module, substmt, 0, ext[u]->ext, ext[u]->ext_size);
        }

        /* close extension */
        yang_print_close(out, level, flag);
    }
}

static void
yang_print_substmt(struct lyout *out, int level, LYEXT_SUBSTMT substmt, uint8_t substmt_index, const char *text,
                   const struct lys_module *module, struct lys_ext_instance **ext, unsigned int ext_size)
{
    int i = -1;

    if (!text) {
        /* nothing to print */
        return;
    }

    do {
        i = ly_print_ext_iter(ext, ext_size, i + 1, substmt);
    } while (i != -1 && ext[i]->substmt_index != substmt_index);

    if (ext_substmt_info[substmt].flags & SUBST_FLAG_ID) {
        ly_print(out, "%*s%s %s%s", LEVEL, INDENT, ext_substmt_info[substmt].name, text, i == -1 ? ";\n" : "");
    } else {
        yang_print_text(out, level, ext_substmt_info[substmt].name, text,
                        (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) ? 0 : 1, i == -1 ? 1 : 0);
    }

    if (i != -1) {
        ly_print(out, " {\n");
        do {
            yang_print_extension_instances(out, level + 1, module, substmt, substmt_index, &ext[i], 1);
            do {
                i = ly_print_ext_iter(ext, ext_size, i + 1, substmt);
            } while (i != -1 && ext[i]->substmt_index != substmt_index);
        } while (i != -1);
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    }
}

static void
yang_print_iffeature(struct lyout *out, int level, const struct lys_module *module, struct lys_iffeature *iffeature)
{
    ly_print(out, "%*sif-feature \"", LEVEL, INDENT);
    ly_print_iffeature(out, module, iffeature);

    /* extensions */
    if (iffeature->ext_size) {
        ly_print(out, "\" {\n");
        yang_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_SELF, 0, iffeature->ext, iffeature->ext_size);
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    } else {
        ly_print(out, "\";\n");
    }
}

/*
 * Covers:
 * extension (instances), if-features, config, mandatory, status, description, reference
 */
#define SNODE_COMMON_EXT    0x01
#define SNODE_COMMON_IFF    0x02
#define SNODE_COMMON_CONFIG 0x04
#define SNODE_COMMON_MAND   0x08
#define SNODE_COMMON_STATUS 0x10
#define SNODE_COMMON_DSC    0x20
#define SNODE_COMMON_REF    0x40
static void
yang_print_snode_common(struct lyout *out, int level, const struct lys_node *node, const struct lys_module *module,
                        int *flag, int mask)
{
    int i;
    const char *status = NULL;

    /* extensions */
    if ((mask & SNODE_COMMON_EXT) && node->ext_size) {
        yang_print_open(out, flag);
        yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, node->ext, node->ext_size);
    }

    /* if-features */
    if (mask & SNODE_COMMON_IFF) {
        for (i = 0; i < node->iffeature_size; ++i) {
            yang_print_open(out, flag);
            yang_print_iffeature(out, level, module, &node->iffeature[i]);
        }
    }

    /* config */
    if (mask & SNODE_COMMON_CONFIG) {
        /* get info if there is an extension for the config statement */
        i = ly_print_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_CONFIG);

        if (lys_parent(node)) {
            if ((node->flags & LYS_CONFIG_SET) || i != -1) {
                /* print config when it differs from the parent or if it has an extension instance ... */
                if (node->flags & LYS_CONFIG_W) {
                    yang_print_open(out, flag);
                    yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true",
                                       module, node->ext, node->ext_size);
                } else if (node->flags & LYS_CONFIG_R) {
                    yang_print_open(out, flag);
                    yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false",
                                       module, node->ext, node->ext_size);
                }
            }
        } else if (node->flags & LYS_CONFIG_R) {
            /* ... or it's a top-level state node */
            yang_print_open(out, flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false",
                               module, node->ext, node->ext_size);
        } else if (i != -1) {
            /* the config has an extension, so we have to print it */
            yang_print_open(out, flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true",
                               module, node->ext, node->ext_size);
        }
    }

    /* mandatory */
    if ((mask & SNODE_COMMON_MAND) && (node->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA))) {
        if (node->flags & LYS_MAND_TRUE) {
            yang_print_open(out, flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true",
                               module, node->ext, node->ext_size);
        } else if (node->flags & LYS_MAND_FALSE) {
            yang_print_open(out, flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false",
                               module, node->ext, node->ext_size);
        }
    }

    /* status */
    if (mask & SNODE_COMMON_STATUS) {
        if (node->flags & LYS_STATUS_CURR) {
            yang_print_open(out, flag);
            status = "current";
        } else if (node->flags & LYS_STATUS_DEPRC) {
            yang_print_open(out, flag);
            status = "deprecated";
        } else if (node->flags & LYS_STATUS_OBSLT) {
            yang_print_open(out, flag);
            status = "obsolete";
        }
        yang_print_substmt(out, level, LYEXT_SUBSTMT_STATUS, 0, status, module, node->ext, node->ext_size);
    }

    /* description */
    if ((mask & SNODE_COMMON_DSC) && node->dsc) {
        yang_print_open(out, flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, node->dsc,
                           module, node->ext, node->ext_size);
    }

    /* reference */
    if ((mask & SNODE_COMMON_REF) && node->ref) {
        yang_print_open(out, flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, node->ref,
                           module, node->ext, node->ext_size);
    }
}

static void
yang_print_feature(struct lyout *out, int level, const struct lys_feature *feat)
{
    int flag = 0;

    ly_print(out, "%*sfeature %s", LEVEL, INDENT, feat->name);
    yang_print_snode_common(out, level + 1, (struct lys_node *)feat, feat->module, &flag, SNODE_COMMON_EXT |
                            SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    yang_print_close(out, level, flag);
}

static void
yang_print_extension(struct lyout *out, int level, const struct lys_ext *ext)
{
    int flag = 0, flag2 = 0, i;

    ly_print(out, "%*sextension %s", LEVEL, INDENT, ext->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)ext, ext->module, &flag,
                            SNODE_COMMON_EXT);

    if (ext->argument) {
        yang_print_open(out, &flag);

        ly_print(out, "%*sargument %s", LEVEL, INDENT, ext->argument);
        i = -1;
        while ((i = ly_print_ext_iter(ext->ext, ext->ext_size, i + 1, LYEXT_SUBSTMT_ARGUMENT)) != -1) {
            yang_print_open(out, &flag2);
            yang_print_extension_instances(out, level + 1, ext->module, LYEXT_SUBSTMT_ARGUMENT, 0, &ext->ext[i], 1);
        }
        if (ext->flags & LYS_YINELEM) {
            yang_print_open(out, &flag2);
            yang_print_substmt(out, level + 1, LYEXT_SUBSTMT_YINELEM, 0, "true", ext->module, ext->ext, ext->ext_size);
        }
        yang_print_close(out, level, flag2);
    }

    yang_print_snode_common(out, level, (struct lys_node *)ext, ext->module, &flag,
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_restr(struct lyout *out, int level, const struct lys_module *module, const struct lys_restr *restr, int *flag)
{
    if (restr->ext_size) {
        yang_print_open(out, flag);
        yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, restr->ext, restr->ext_size);
    }
    if (restr->emsg != NULL) {
        yang_print_open(out, flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_ERRMSG, 0, restr->emsg,
                           module, restr->ext, restr->ext_size);
    }
    if (restr->eapptag != NULL) {
        yang_print_open(out, flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_ERRTAG, 0, restr->eapptag,
                           module, restr->ext, restr->ext_size);
    }
    if (restr->dsc != NULL) {
        yang_print_open(out, flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, restr->dsc,
                           module, restr->ext, restr->ext_size);
    }
    if (restr->ref != NULL) {
        yang_print_open(out, flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, restr->ref,
                           module, restr->ext, restr->ext_size);
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

    if (when->ext_size) {
        /* extension is stored in lys_when incompatible with lys_node, so we cannot use yang_print_snode_common() */
        yang_print_open(out, &flag);
        yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, when->ext, when->ext_size);
    }
    yang_print_snode_common(out, level, (struct lys_node *)when, module, &flag, SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_type(struct lyout *out, int level, const struct lys_module *module, const struct lys_type *type)
{
    int i;
    int flag = 0, flag2;
    const char *str;
    char *s;
    struct lys_module *mod;

    if (type->module_name) {
        ly_print(out, "%*stype %s:%s", LEVEL, INDENT,
                 transform_module_name2import_prefix(module, type->module_name), type->der->name);
    } else {
        ly_print(out, "%*stype %s", LEVEL, INDENT, type->der->name);
    }
    level++;

    /* extensions */
    if (type->ext_size) {
        yang_print_open(out, &flag);
        yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, type->ext, type->ext_size);
    }

    switch (type->base) {
    case LY_TYPE_BINARY:
        if (type->info.binary.length) {
            yang_print_open(out, &flag);
            ly_print(out, "%*slength \"", LEVEL, INDENT);
            yang_encode(out, type->info.binary.length->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, module, type->info.binary.length, &flag2);
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            yang_print_open(out, &flag);
            ly_print(out, "%*sbit %s", LEVEL, INDENT, type->info.bits.bit[i].name);
            flag2 = 0;
            level++;
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.bits.bit[i], module, &flag2,
                                    SNODE_COMMON_EXT | SNODE_COMMON_IFF);
            if (!(type->info.bits.bit[i].flags & LYS_AUTOASSIGNED)) {
                yang_print_open(out, &flag2);
                asprintf(&s, "%u", type->info.bits.bit[i].pos);
                yang_print_substmt(out, level, LYEXT_SUBSTMT_POSITION, 0, s,
                                   module, type->info.bits.bit[i].ext, type->info.bits.bit[i].ext_size);
                free(s);
            }
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.bits.bit[i], module, &flag2,
                                    SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
            level--;
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_DEC64:
        if (!type->der->type.der) {
            yang_print_open(out, &flag);
            asprintf(&s, "%d", type->info.dec64.dig);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_DIGITS, 0, s, module, type->ext, type->ext_size);
            free(s);
        }
        if (type->info.dec64.range != NULL) {
            yang_print_open(out, &flag);
            ly_print(out, "%*srange \"", LEVEL, INDENT);
            yang_encode(out, type->info.dec64.range->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, module, type->info.dec64.range, &flag2);
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            yang_print_open(out, &flag);
            ly_print(out, "%*senum \"%s\"", LEVEL, INDENT, type->info.enums.enm[i].name);
            flag2 = 0;
            level++;
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.enums.enm[i], module, &flag2,
                                    SNODE_COMMON_EXT | SNODE_COMMON_IFF);
            if (!(type->info.enums.enm[i].flags & LYS_AUTOASSIGNED)) {
                yang_print_open(out, &flag2);
                asprintf(&s, "%d", type->info.enums.enm[i].value);
                yang_print_substmt(out, level, LYEXT_SUBSTMT_VALUE, 0, s,
                                   module, type->info.enums.enm[i].ext, type->info.enums.enm[i].ext_size);
                free(s);
            }
            yang_print_snode_common(out, level, (struct lys_node *)&type->info.enums.enm[i], module, &flag2,
                                    SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
            level--;
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_IDENT:
        if (type->info.ident.count) {
            yang_print_open(out, &flag);
            for (i = 0; i < type->info.ident.count; ++i) {
                mod = lys_main_module(type->info.ident.ref[i]->module);
                if (lys_main_module(module) == mod) {
                    yang_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, type->info.ident.ref[i]->name,
                                       module, type->info.ident.ref[i]->ext, type->info.ident.ref[i]->ext_size);
                } else {
                    asprintf(&s, "%s:%s", transform_module_name2import_prefix(module, mod->name),
                             type->info.ident.ref[i]->name);
                    yang_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, s,
                                       module, type->info.ident.ref[i]->ext, type->info.ident.ref[i]->ext_size);
                    free(s);
                }
            }
        }
        break;
    case LY_TYPE_INST:
        if (type->info.inst.req == 1) {
            yang_print_open(out, &flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "true", module, type->ext, type->ext_size);
        } else if (type->info.inst.req == -1) {
            yang_print_open(out, &flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "false", module, type->ext, type->ext_size);
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
        if (type->info.num.range) {
            yang_print_open(out, &flag);
            ly_print(out, "%*srange \"", LEVEL, INDENT);
            yang_encode(out, type->info.num.range->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, module, type->info.num.range, &flag2);
            yang_print_close(out, level, flag2);
        }
        break;
    case LY_TYPE_LEAFREF:
        if (ly_strequal(type->der->name, "leafref", 0)) {
            yang_print_open(out, &flag);
            str = transform_json2schema(module, type->info.lref.path);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_PATH, 0, str, module, type->ext, type->ext_size);
            lydict_remove(module->ctx, str);
        }
        if (type->info.lref.req == 1) {
            yang_print_open(out, &flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "true", module, type->ext, type->ext_size);
        } else if (type->info.lref.req == -1) {
            yang_print_open(out, &flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "false", module, type->ext, type->ext_size);
        }
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length) {
            yang_print_open(out, &flag);
            ly_print(out, "%*slength \"", LEVEL, INDENT);
            yang_encode(out, type->info.str.length->expr, -1);
            ly_print(out, "\"");
            flag2 = 0;
            yang_print_restr(out, level + 1, module, type->info.str.length, &flag2);
            yang_print_close(out, level, flag2);
        }
        for (i = 0; i < type->info.str.pat_count; i++) {
            yang_print_open(out, &flag);
            ly_print(out, "%*spattern \"", LEVEL, INDENT);
            yang_encode(out, &type->info.str.patterns[i].expr[1], -1);
            ly_print(out, "\"");
            flag2 = 0;
            if (type->info.str.patterns[i].expr[0] == 0x15) {
                yang_print_open(out, &flag2);
                yang_print_substmt(out, level + 1, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match",
                                   module, type->info.str.patterns[i].ext, type->info.str.patterns[i].ext_size);
            }
            yang_print_restr(out, level + 1, module, &type->info.str.patterns[i], &flag2);
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

    yang_print_restr(out, level + 1, module, must, &flag);
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
    ly_print(out, "\"");
}

static void
yang_print_refine(struct lyout *out, int level, const struct lys_module *module, const struct lys_refine *refine)
{
    int i, flag = 0;
    const char *str;
    char *s;

    str = transform_json2schema(module, refine->target_name);
    ly_print(out, "%*srefine \"%s\"", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)refine, module, &flag, SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < refine->must_size; ++i) {
        yang_print_open(out, &flag);
        yang_print_must(out, level, module, &refine->must[i]);
    }
    if (refine->target_type == LYS_CONTAINER) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_PRESENCE, 0, refine->mod.presence,
                           module, refine->ext, refine->ext_size);
    }
    for (i = 0; i < refine->dflt_size; ++i) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, i, refine->dflt[i], module, refine->ext, refine->ext_size);
    }
    if (refine->flags & LYS_CONFIG_W) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true", module, refine->ext, refine->ext_size);
    } else if (refine->flags & LYS_CONFIG_R) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false", module, refine->ext, refine->ext_size);
    }
    if (refine->flags & LYS_MAND_TRUE) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true", module, refine->ext, refine->ext_size);
    } else if (refine->flags & LYS_MAND_FALSE) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false", module, refine->ext, refine->ext_size);
    }
    if (refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
        if (refine->flags & LYS_RFN_MINSET) {
            asprintf(&s, "%u", refine->mod.list.min);
            yang_print_open(out, &flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MIN, 0, s, module, refine->ext, refine->ext_size);
            free(s);
        }
        if (refine->flags & LYS_RFN_MAXSET) {
            if (refine->mod.list.max) {
                asprintf(&s, "%u", refine->mod.list.max);
            } else {
                s = NULL;
            }
            yang_print_open(out, &flag);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MIN, 0, s ? s : "unbounded", module, refine->ext, refine->ext_size);
            free(s);
        }
    }
    yang_print_snode_common(out, level, (struct lys_node *)refine, module, &flag, SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_deviation(struct lyout *out, int level, const struct lys_module *module,
                     const struct lys_deviation *deviation)
{
    int i, j, k, p;
    const char *str;
    char *s;

    str = transform_json2schema(module, deviation->target_name);
    ly_print(out, "%*sdeviation \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(module->ctx, str);
    level++;

    if (deviation->ext_size) {
        yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, deviation->ext, deviation->ext_size);
    }
    yang_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, deviation->dsc,
                       module, deviation->ext, deviation->ext_size);
    yang_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, deviation->ref,
                       module, deviation->ext, deviation->ext_size);

    for (i = 0; i < deviation->deviate_size; ++i) {
        ly_print(out, "%*sdeviate ", LEVEL, INDENT);
        if (deviation->deviate[i].mod == LY_DEVIATE_NO) {
            ly_print(out, "not-supported;\n");
            continue;
        } else if (deviation->deviate[i].mod == LY_DEVIATE_ADD) {
            ly_print(out, "add {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_RPL) {
            ly_print(out, "replace {\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_DEL) {
            ly_print(out, "delete {\n");
        }
        level++;

        /* extensions */
        if (deviation->deviate[i].ext_size) {
            yang_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_SELF, 0,
                                           deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* type */
        if (deviation->deviate[i].type) {
            yang_print_type(out, level, module, deviation->deviate[i].type);
        }

        /* units */
        yang_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, deviation->deviate[i].units, module,
                           deviation->deviate[i].ext, deviation->deviate[i].ext_size);

        /* must */
        for (j = 0; j < deviation->deviate[i].must_size; ++j) {
            yang_print_must(out, level, module, &deviation->deviate[i].must[j]);
        }

        /* unique */

        for (j = 0; j < deviation->deviate[i].unique_size; ++j) {
            yang_print_unique(out, level, &deviation->deviate[i].unique[j]);
            /* unique's extensions */
            p = -1;
            do {
                p = ly_print_ext_iter(deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                      p + 1, LYEXT_SUBSTMT_UNIQUE);
            } while (p != -1 && deviation->deviate[i].ext[p]->substmt_index != j);
            if (p != -1) {
                ly_print(out, " {\n");
                do {
                    yang_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_UNIQUE, j,
                                                   &deviation->deviate[i].ext[p], 1);
                    do {
                        p = ly_print_ext_iter(deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                              p + 1, LYEXT_SUBSTMT_UNIQUE);
                    } while (p != -1 && deviation->deviate[i].ext[p]->substmt_index != j);
                } while (p != -1);
                ly_print(out, "%*s}\n", LEVEL, INDENT);
            } else {
                ly_print(out, ";\n");
            }
        }

        /* default */
        for (k = 0; k < deviation->deviate[i].dflt_size; ++k) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, k, deviation->deviate[i].dflt[k], module,
                               deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* config */
        if (deviation->deviate[i].flags & LYS_CONFIG_W) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true", module,
                               deviation->deviate->ext, deviation->deviate[i].ext_size);
        } else if (deviation->deviate[i].flags & LYS_CONFIG_R) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false", module,
                               deviation->deviate->ext, deviation->deviate[i].ext_size);
        }

        /* mandatory */
        if (deviation->deviate[i].flags & LYS_MAND_TRUE) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true", module,
                               deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        } else if (deviation->deviate[i].flags & LYS_MAND_FALSE) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false", module,
                               deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* min-elements */
        if (deviation->deviate[i].min_set) {
            asprintf(&s, "%u", deviation->deviate[i].min);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MIN, 0, s, module,
                               deviation->deviate[i].ext, deviation->deviate[i].ext_size);
            free(s);
        }

        /* max-elements */
        if (deviation->deviate[i].max_set) {
            if (deviation->deviate[i].max) {
                asprintf(&s, "%u", deviation->deviate[i].max);
            } else {
                s = NULL;
            }
            yang_print_substmt(out, level, LYEXT_SUBSTMT_MAX, 0, s ? s : "unbounded", module,
                               deviation->deviate[i].ext, deviation->deviate[i].ext_size);
            free(s);
        }

        level--;
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_augment(struct lyout *out, int level, const struct lys_node_augment *augment)
{
    struct lys_node *sub;
    const char *str;

    str = transform_json2schema(augment->module, augment->target_name);
    ly_print(out, "%*saugment \"%s\" {\n", LEVEL, INDENT, str);
    lydict_remove(augment->module->ctx, str);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)augment, augment->module, NULL, SNODE_COMMON_EXT);
    if (augment->when) {
        yang_print_when(out, level, augment->module, augment->when);
    }
    yang_print_snode_common(out, level, (struct lys_node *)augment, augment->module, NULL,
                            SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    LY_TREE_FOR(augment->child, sub) {
        /* only our augment */
        if (sub->parent != (struct lys_node *)augment) {
            continue;
        }
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA | LYS_CASE | LYS_ACTION | LYS_NOTIF);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_typedef(struct lyout *out, int level, const struct lys_module *module, const struct lys_tpdf *tpdf)
{
    ly_print(out, "%*stypedef %s {\n", LEVEL, INDENT, tpdf->name);
    level++;
    const char *dflt;

    yang_print_snode_common(out, level, (struct lys_node *)tpdf, module, NULL, SNODE_COMMON_EXT);
    yang_print_type(out, level, module, &tpdf->type);
    if (tpdf->units != NULL) {
        yang_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, tpdf->units, module, tpdf->ext, tpdf->ext_size);
    }
    if (tpdf->dflt != NULL) {
        if (tpdf->flags & LYS_DFLTJSON) {
            assert(strchr(tpdf->dflt, ':'));
            if (!strncmp(tpdf->dflt, module->name, strchr(tpdf->dflt, ':') - tpdf->dflt)) {
                /* local module */
                dflt = lydict_insert(module->ctx, strchr(tpdf->dflt, ':') + 1, 0);
            } else {
                dflt = transform_json2schema(module, tpdf->dflt);
            }
        } else {
            dflt = tpdf->dflt;
        }
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, dflt, module, tpdf->ext, tpdf->ext_size);
        if (tpdf->flags & LYS_DFLTJSON) {
            lydict_remove(module->ctx, dflt);
        }
    }
    yang_print_snode_common(out, level, (struct lys_node *)tpdf, module, NULL,
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_identity(struct lyout *out, int level, const struct lys_ident *ident)
{
    int flag = 0, i;
    struct lys_module *mod;
    char *str;

    ly_print(out, "%*sidentity %s", LEVEL, INDENT, ident->name);
    level++;

    yang_print_snode_common(out, level, (struct lys_node *)ident, ident->module, &flag,
                            SNODE_COMMON_EXT | SNODE_COMMON_IFF);

    for (i = 0; i < ident->base_size; i++) {
        yang_print_open(out, &flag);
        mod = lys_main_module(ident->base[i]->module);
        if (lys_main_module(ident->module) == mod) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, ident->base[i]->name,
                               ident->module, ident->ext, ident->ext_size);
        } else {
            asprintf(&str, "%s:%s", transform_module_name2import_prefix(ident->module, mod->name), ident->base[i]->name);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, str,
                               ident->module, ident->ext, ident->ext_size);
            free(str);
        }
    }

    yang_print_snode_common(out, level, (struct lys_node *)ident, ident->module, &flag,
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

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

    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT);
    if (cont->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, cont->when);
    }
    for (i = 0; i < cont->iffeature_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, &cont->iffeature[i]);
    }
    for (i = 0; i < cont->must_size; i++) {
        yang_print_open(out, &flag);
        yang_print_must(out, level, node->module, &cont->must[i]);
    }
    if (cont->presence != NULL) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_PRESENCE, 0, cont->presence,
                           node->module, node->ext, node->ext_size);
    }
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_CONFIG |
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < cont->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, node->module, &cont->tpdf[i]);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA );
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_ACTION);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub,LYS_NOTIF);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_case(struct lyout *out, int level, const struct lys_node *node)
{
    int flag = 0;
    struct lys_node *sub;
    struct lys_node_case *cas = (struct lys_node_case *)node;

    ly_print(out, "%*scase %s", LEVEL, INDENT, cas->name);
    level++;

    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT);
    if (cas->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, cas->when);
    }
    yang_print_snode_common(out, level, node, node->module, &flag,
                            SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_choice(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node *sub;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    ly_print(out, "%*schoice %s", LEVEL, INDENT, node->name);
    level++;

    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT);
    if (choice->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, choice->when);
    }
    for (i = 0; i < choice->iffeature_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, &choice->iffeature[i]);
    }
    if (choice->dflt != NULL) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt->name,
                           node->module, node->ext, node->ext_size);
    }
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_CASE);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_leaf(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;
    const char *dflt;

    ly_print(out, "%*sleaf %s {\n", LEVEL, INDENT, node->name);
    level++;

    yang_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_EXT);
    if (leaf->when) {
        yang_print_when(out, level, node->module, leaf->when);
    }
    for (i = 0; i < leaf->iffeature_size; i++) {
        yang_print_iffeature(out, level, node->module, &leaf->iffeature[i]);
    }
    yang_print_type(out, level, node->module, &leaf->type);
    yang_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, leaf->units,
                       node->module, node->ext, node->ext_size);
    for (i = 0; i < leaf->must_size; i++) {
        yang_print_must(out, level, node->module, &leaf->must[i]);
    }
    if (leaf->dflt) {
        if (leaf->flags & LYS_DFLTJSON) {
            assert(strchr(leaf->dflt, ':'));
            if (!strncmp(leaf->dflt, lys_node_module(node)->name, strchr(leaf->dflt, ':') - leaf->dflt)) {
                /* local module */
                dflt = lydict_insert(node->module->ctx, strchr(leaf->dflt, ':') + 1, 0);
            } else {
                dflt = transform_json2schema(node->module, leaf->dflt);
            }
        } else {
            dflt = leaf->dflt;
        }
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, dflt,
                           node->module, node->ext, node->ext_size);
        if (leaf->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    yang_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_anydata(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node_anydata *any = (struct lys_node_anydata *)node;

    ly_print(out, "%*s%s %s", LEVEL, INDENT, any->nodetype == LYS_ANYXML ? "anyxml" : "anydata", any->name);
    level++;

    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT);
    if (any->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, any->when);
    }
    for (i = 0; i < any->iffeature_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, &any->iffeature[i]);
    }
    for (i = 0; i < any->must_size; i++) {
        yang_print_open(out, &flag);
        yang_print_must(out, level, node->module, &any->must[i]);
    }
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_leaflist(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;
    const char *dflt;
    char *str;

    ly_print(out, "%*sleaf-list %s {\n", LEVEL, INDENT, node->name);
    level++;
    yang_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_EXT);
    if (llist->when) {
        yang_print_when(out, level, llist->module, llist->when);
    }
    for (i = 0; i < llist->iffeature_size; i++) {
        yang_print_iffeature(out, level, node->module, &llist->iffeature[i]);
    }
    yang_print_type(out, level, node->module, &llist->type);
    yang_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, llist->units,
                       node->module, node->ext, node->ext_size);
    for (i = 0; i < llist->must_size; i++) {
        yang_print_must(out, level, node->module, &llist->must[i]);
    }
    for (i = 0; i < llist->dflt_size; ++i) {
        if (llist->flags & LYS_DFLTJSON) {
            assert(strchr(llist->dflt[i], ':'));
            if (!strncmp(llist->dflt[i], lys_node_module(node)->name, strchr(llist->dflt[i], ':') - llist->dflt[i])) {
                /* local module */
                dflt = lydict_insert(node->module->ctx, strchr(llist->dflt[i], ':') + 1, 0);
            } else {
                dflt = transform_json2schema(node->module, llist->dflt[i]);
            }
        } else {
            dflt = llist->dflt[i];
        }
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, i, dflt,
                           node->module, node->ext, node->ext_size);
        if (llist->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    if (llist->min > 0) {
        asprintf(&str, "%u", llist->min);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_MIN, 0, str,
                           node->module, node->ext, node->ext_size);
        free(str);
    }
    if (llist->max > 0) {
        asprintf(&str, "%u", llist->max);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_MAX, 0, str,
                           node->module, node->ext, node->ext_size);
        free(str);
    }
    if (llist->flags & LYS_USERORDERED) {
        yang_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "user",
                           node->module, node->ext, node->ext_size);
    } else if (ly_print_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_ORDEREDBY) != -1) {
        yang_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "system",
                           node->module, node->ext, node->ext_size);
    }
    yang_print_snode_common(out, level, node, node->module, NULL,
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_list(struct lyout *out, int level, const struct lys_node *node)
{
    int i, p, flag = 0;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;
    char *str;

    ly_print(out, "%*slist %s", LEVEL, INDENT, node->name);
    level++;
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT);
    if (list->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, list->module, list->when);
    }
    for (i = 0; i < list->iffeature_size; i++) {
        yang_print_open(out, &flag);
        yang_print_iffeature(out, level, node->module, &list->iffeature[i]);
    }
    for (i = 0; i < list->must_size; i++) {
        yang_print_open(out, &flag);
        yang_print_must(out, level, list->module, &list->must[i]);
    }
    if (list->keys_size) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_KEY, 0, list->keys_str,
                           node->module, node->ext, node->ext_size);
    }
    for (i = 0; i < list->unique_size; i++) {
        yang_print_open(out, &flag);
        yang_print_unique(out, level, &list->unique[i]);
        /* unique's extensions */
        p = -1;
        do {
            p = ly_print_ext_iter(list->ext, list->ext_size, p + 1, LYEXT_SUBSTMT_UNIQUE);
        } while (p != -1 && list->ext[p]->substmt_index != i);
        if (p != -1) {
            ly_print(out, " {\n");
            do {
                yang_print_extension_instances(out, level + 1, list->module, LYEXT_SUBSTMT_UNIQUE, i, &list->ext[p], 1);
                do {
                    p = ly_print_ext_iter(list->ext, list->ext_size, p + 1, LYEXT_SUBSTMT_UNIQUE);
                } while (p != -1 && list->ext[p]->substmt_index != i);
            } while (p != -1);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        } else {
            ly_print(out, ";\n");
        }
    }
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_CONFIG);
    if (list->min > 0) {
        yang_print_open(out, &flag);
        asprintf(&str, "%u", list->min);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_MIN, 0, str,
                           node->module, node->ext, node->ext_size);
        free(str);
    }
    if (list->max > 0) {
        yang_print_open(out, &flag);
        asprintf(&str, "%u", list->max);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_MAX, 0, str,
                           node->module, node->ext, node->ext_size);
        free(str);
    }
    if (list->flags & LYS_USERORDERED) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "user",
                           node->module, node->ext, node->ext_size);
    } else if (ly_print_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_ORDEREDBY) != -1) {
        yang_print_open(out, &flag);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "system",
                           node->module, node->ext, node->ext_size);
    }
    yang_print_snode_common(out, level, node, node->module, &flag,
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < list->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, list->module, &list->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYDATA);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_ACTION);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_grouping(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node *sub;
    struct lys_node_grp *grp = (struct lys_node_grp *)node;

    ly_print(out, "%*sgrouping %s", LEVEL, INDENT, node->name);
    level++;

    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT | SNODE_COMMON_STATUS |
                            SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < grp->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, node->module, &grp->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    LY_TREE_FOR(node->child, sub) {
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_ACTION);
    }

    LY_TREE_FOR(node->child, sub) {
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_uses(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node_uses *uses = (struct lys_node_uses *)node;
    struct lys_module *mod;

    ly_print(out, "%*suses ", LEVEL, INDENT);
    if (node->child) {
        mod = lys_node_module(node->child);
        if (lys_node_module(node) != mod) {
            ly_print(out, "%s:", transform_module_name2import_prefix(node->module, mod->name));
        }
    }
    ly_print(out, "%s", uses->name);
    level++;

    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT);
    if (uses->when) {
        yang_print_open(out, &flag);
        yang_print_when(out, level, node->module, uses->when);
    }
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_IFF |
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < uses->refine_size; i++) {
        yang_print_open(out, &flag);
        yang_print_refine(out, level, node->module, &uses->refine[i]);
    }
    for (i = 0; i < uses->augment_size; i++) {
        yang_print_open(out, &flag);
        yang_print_augment(out, level, &uses->augment[i]);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_input_output(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_inout *inout = (struct lys_node_inout *)node;

    ly_print(out, "%*s%s {\n", LEVEL, INDENT, (inout->nodetype == LYS_INPUT ? "input" : "output"));
    level++;

    if (node->ext_size) {
        yang_print_extension_instances(out, level, node->module, LYEXT_SUBSTMT_SELF, 0, node->ext, node->ext_size);
    }
    for (i = 0; i < inout->must_size; i++) {
        yang_print_must(out, level, node->module, &inout->must[i]);
    }
    for (i = 0; i < inout->tpdf_size; i++) {
        yang_print_typedef(out, level, node->module, &inout->tpdf[i]);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    level--;
    ly_print(out, "%*s}\n", LEVEL, INDENT);
}

static void
yang_print_rpc_action(struct lyout *out, int level, const struct lys_node *node)
{
    int i, flag = 0;
    struct lys_node *sub;
    struct lys_node_rpc_action *rpc = (struct lys_node_rpc_action *)node;

    ly_print(out, "%*s%s %s", LEVEL, INDENT, (node->nodetype == LYS_RPC ? "rpc" : "action"), node->name);

    level++;
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT | SNODE_COMMON_IFF |
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < rpc->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, node->module, &rpc->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_INPUT);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_OUTPUT);
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
    yang_print_snode_common(out, level, node, node->module, &flag, SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < notif->must_size; i++) {
        yang_print_open(out, &flag);
        yang_print_must(out, level, node->module, &notif->must[i]);
    }
    yang_print_snode_common(out, level, node, node->module, &flag,
                            SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < notif->tpdf_size; i++) {
        yang_print_open(out, &flag);
        yang_print_typedef(out, level, node->module, &notif->tpdf[i]);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yang_print_open(out, &flag);
        yang_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    level--;
    yang_print_close(out, level, flag);
}

static void
yang_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask)
{
    if (node->nodetype & mask) {
        if ((node->nodetype & (LYS_INPUT | LYS_OUTPUT)) && (node->flags & LYS_IMPLICIT)) {
            /* implicit input/output node is not supposed to be printed */
            return;
        } else if (!node->parent ||
                (node->parent->nodetype == LYS_AUGMENT && node != node->parent->child) ||
                (node->parent->nodetype != LYS_AUGMENT && node->prev->next)) {
            /* do not print the blank line before the first data-def node */
            ly_print(out, "\n");
        }
    }

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
    case LYS_ANYDATA:
        yang_print_anydata(out, level, node);
        break;
    case LYS_CASE:
        yang_print_case(out, level, node);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        yang_print_rpc_action(out, level, node);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        yang_print_input_output(out, level, node);
        break;
    case LYS_NOTIF:
        yang_print_notif(out, level, node);
        break;
    default:
        break;
    }
}

int
yang_print_model(struct lyout *out, const struct lys_module *module)
{
    unsigned int i;
    int level = 0, p;
#define LEVEL (level*2)

    struct lys_node *node;

    /* (sub)module-header-stmts */
    if (module->type) {
        ly_print(out, "submodule %s {%s\n", module->name, (module->deviated == 1 ? " // DEVIATED" : ""));
        level++;
        if (module->version) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_VERSION, 0,
                               ((struct lys_submodule *)module)->belongsto->version == 2 ? "1.1" : "1",
                               module, module->ext, module->ext_size);
        }
        ly_print(out, "%*sbelongs-to %s {\n", LEVEL, INDENT, ((struct lys_submodule *)module)->belongsto->name);
        p = -1;
        while ((p = ly_print_ext_iter(module->ext, module->ext_size, p + 1, LYEXT_SUBSTMT_BELONGSTO)) != -1) {
            yang_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_BELONGSTO, 0, &module->ext[p], 1);
        }
        yang_print_substmt(out, level + 1, LYEXT_SUBSTMT_PREFIX, 0, module->prefix,
                           module, module->ext, module->ext_size);
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    } else {
        ly_print(out, "module %s {%s\n", module->name, (module->deviated == 1 ? " // DEVIATED" : ""));
        level++;
        if (module->version) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_VERSION, 0, module->version == 2 ? "1.1" : "1",
                               module, module->ext, module->ext_size);
        }
        yang_print_substmt(out, level, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns,
                           module, module->ext, module->ext_size);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_PREFIX, 0, module->prefix,
                           module, module->ext, module->ext_size);
    }

    /* linkage-stmts */
    for (i = 0; i < module->imp_size; i++) {
        ly_print(out, "\n%*simport %s {\n", LEVEL, INDENT, module->imp[i].module->name);
        level++;
        yang_print_substmt(out, level, LYEXT_SUBSTMT_PREFIX, 0, module->imp[i].prefix,
                           module, module->imp[i].ext, module->imp[i].ext_size);
        yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                       module->imp[i].ext, module->imp[i].ext_size);
        if (module->imp[i].rev[0]) {
            yang_print_substmt(out, level, LYEXT_SUBSTMT_REVISIONDATE, 0, module->imp[i].rev,
                               module, module->imp[i].ext, module->imp[i].ext_size);
        }
        yang_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->imp[i].dsc,
                           module, module->imp[i].ext, module->imp[i].ext_size);
        yang_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->imp[i].ref,
                           module, module->imp[i].ext, module->imp[i].ext_size);
        level--;
        ly_print(out, "%*s}\n", LEVEL, INDENT);
    }
    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].external) {
            continue;
        }
        if (module->inc[i].rev[0] || module->inc[i].dsc || module->inc[i].ref || module->inc[i].ext_size) {
            ly_print(out, "\n%*sinclude %s {\n", LEVEL, INDENT, module->inc[i].submodule->name);
            level++;
            yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                           module->inc[i].ext, module->inc[i].ext_size);
            if (module->inc[i].rev[0]) {
                yang_print_substmt(out, level, LYEXT_SUBSTMT_REVISIONDATE, 0, module->inc[i].rev,
                                   module, module->inc[i].ext, module->inc[i].ext_size);
            }
            yang_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->inc[i].dsc,
                               module, module->inc[i].ext, module->inc[i].ext_size);
            yang_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->inc[i].ref,
                               module, module->inc[i].ext, module->inc[i].ext_size);
            level--;
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        } else {
            ly_print(out, "\n%*sinclude \"%s\";\n", LEVEL, INDENT, module->inc[i].submodule->name);
        }
    }

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print(out, "\n");
    }
    yang_print_substmt(out, level, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org,
                       module, module->ext, module->ext_size);
    yang_print_substmt(out, level, LYEXT_SUBSTMT_CONTACT, 0, module->contact,
                       module, module->ext, module->ext_size);
    yang_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc,
                       module, module->ext, module->ext_size);
    yang_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->ref,
                       module, module->ext, module->ext_size);

    /* revision-stmts */
    if (module->rev_size) {
        ly_print(out, "\n");
    }
    for (i = 0; i < module->rev_size; i++) {
        if (module->rev[i].dsc || module->rev[i].ref || module->rev[i].ext_size) {
            ly_print(out, "%*srevision \"%s\" {\n", LEVEL, INDENT, module->rev[i].date);
            yang_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_SELF, 0,
                                           module->rev[i].ext, module->rev[i].ext_size);
            yang_print_substmt(out, level + 1, LYEXT_SUBSTMT_DESCRIPTION, 0, module->rev[i].dsc,
                               module, module->rev[i].ext, module->rev[i].ext_size);
            yang_print_substmt(out, level + 1, LYEXT_SUBSTMT_REFERENCE, 0, module->rev[i].ref,
                               module, module->rev[i].ext, module->rev[i].ext_size);
            ly_print(out, "%*s}\n", LEVEL, INDENT);
        } else {
            ly_print(out, "%*srevision %s;\n", LEVEL, INDENT, module->rev[i].date);
        }
    }

    /* body-stmts */
    for (i = 0; i < module->extensions_size; i++) {
        ly_print(out, "\n");
        yang_print_extension(out, level, &module->extensions[i]);
    }
    if (module->ext_size) {
        ly_print(out, "\n");
        yang_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, module->ext, module->ext_size);
    }

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

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yang_print_snode(out, level, node, LYS_GROUPING);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yang_print_snode(out, level, node, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                             LYS_USES | LYS_ANYDATA);
    }

    for (i = 0; i < module->augment_size; i++) {
        ly_print(out, "\n");
        yang_print_augment(out, level, &module->augment[i]);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yang_print_snode(out, level, node, LYS_RPC | LYS_ACTION);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yang_print_snode(out, level, node, LYS_NOTIF);
    }

    for (i = 0; i < module->deviation_size; ++i) {
        ly_print(out, "\n");
        yang_print_deviation(out, level, module, &module->deviation[i]);
    }

    ly_print(out, "}\n");
    ly_print_flush(out);

    return EXIT_SUCCESS;
#undef LEVEL
}
