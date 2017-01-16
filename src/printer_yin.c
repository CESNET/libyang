/**
 * @file printer_yin.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YIN printer for libyang data model structure
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"
#include "xml_internal.h"

#define INDENT ""
#define LEVEL (level*2)

static void yin_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask);

/* endflag :
 * -1: />  - empty element
 *  0:     - no end
 *  1: >   - element with children
 */
static void
yin_print_open(struct lyout *out, int level, const char *elem_prefix, const char *elem_name,
               const char *attr_name, const char *attr_value, int endflag)
{
    if (elem_prefix) {
        ly_print(out, "%*s<%s:%s", LEVEL, INDENT, elem_prefix, elem_name);
    } else {
        ly_print(out, "%*s<%s", LEVEL, INDENT, elem_name);
    }

    if (attr_name) {
        ly_print(out, " %s=\"", attr_name);
        lyxml_dump_text(out, attr_value);
        ly_print(out, "\"%s", endflag == -1 ? "/>\n" : endflag == 1 ? ">\n" : "");
    } else if (endflag) {
        ly_print(out, endflag == -1 ? "/>\n" : ">\n");
    }
}

/*
 * endflag:
 * 0: />           - closing empty element
 * 1: </elem_name> - closing element with children
 */
static void
yin_print_close(struct lyout *out, int level, const char *elem_prefix, const char *elem_name, int endflag)
{
    if (endflag) {
        if (elem_prefix) {
            ly_print(out, "%*s</%s:%s>\n", LEVEL, INDENT, elem_prefix, elem_name);
        } else {
            ly_print(out, "%*s</%s>\n", LEVEL, INDENT, elem_name);
        }
    } else {
        ly_print(out, "/>\n");
    }
}

/*
 * par_close_flag
 * 0 - parent not yet closed, printing >\n, setting flag to 1
 * 1 or NULL - parent already closed, do nothing
 */
static void
yin_print_close_parent(struct lyout *out, int *par_close_flag)
{
    if (par_close_flag && !(*par_close_flag)) {
        (*par_close_flag) = 1;
        ly_print(out, ">\n");
    }
}

static void
yin_print_arg(struct lyout *out, int level, const char *arg, const char *text)
{
    ly_print(out, "%*s<%s>", LEVEL, INDENT, arg);
    lyxml_dump_text(out, text);
    ly_print(out, "</%s>\n", arg);
}

static void
yin_print_extension_instances(struct lyout *out, int level, const struct lys_module *module,
                              LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                              struct lys_ext_instance **ext, unsigned int count)
{
    unsigned int u, x;
    struct lys_module *mod;
    const char *prefix = NULL;
    int content;

    for (u = 0; u < count; u++) {
        if (ext[u]->flags & LYEXT_OPT_INHERIT) {
            /* ignore the inherited extensions which were not explicitely instantiated in the module */
            continue;
        } else if (ext[u]->substmt != substmt || ext[u]->substmt_index != substmt_index) {
            /* do not print the other substatement than the required */
            continue;
        }

        mod = lys_main_module(ext[u]->def->module);
        if (mod == lys_main_module(module)) {
            prefix = module->prefix;
        } else {
            for (x = 0; x < module->imp_size; x++) {
                if (mod == module->imp[x].module) {
                    prefix = module->imp[x].prefix;
                    break;
                }
            }
        }
        assert(prefix);

        content = 0;
        if (ext[u]->arg_value) {
            if (ext[u]->def->flags & LYS_YINELEM) {
                /* argument as element */
                content = 1;
                yin_print_open(out, level, prefix, ext[u]->def->name, NULL, NULL, content);
                level++;
                ly_print(out, "%*s<%s:%s>", LEVEL, INDENT, prefix, ext[u]->def->argument);
                lyxml_dump_text(out, ext[u]->arg_value);
                ly_print(out, "</%s:%s>\n", prefix, ext[u]->def->argument);
                level--;
            } else {
                /* argument as attribute */
                yin_print_open(out, level, prefix, ext[u]->def->name,
                               ext[u]->def->argument, ext[u]->arg_value, content);
            }
        } else {
            yin_print_open(out, level, prefix, ext[u]->def->name, NULL, NULL, content);
        }

        /* extension - type-specific part */
        switch (lys_ext_instance_type(ext[u])) {
        case LYEXT_FLAG:
            /* flag extension - nothing special */
            break;
        case LYEXT_ERR:
            LOGINT;
            break;
        }

        /* extensions */
        if (ext[u]->ext_size) {
            yin_print_close_parent(out, &content);
            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_SELF, 0,
                                          ext[u]->ext, ext[u]->ext_size);
        }

        /* close extension */
        yin_print_close(out, level, prefix, ext[u]->def->name, content);
    }
}

static void
yin_print_substmt(struct lyout *out, int level, LYEXT_SUBSTMT substmt, uint8_t substmt_index, const char *text,
                   const struct lys_module *module, struct lys_ext_instance **ext, unsigned int ext_size)
{
    int i, content = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) {
        content = 1;
        yin_print_open(out, level, NULL, ext_substmt_info[substmt].name,
                       NULL, NULL, content);
    } else {
        yin_print_open(out, level, NULL, ext_substmt_info[substmt].name,
                       ext_substmt_info[substmt].arg, text, content);
    }
    /* extensions */
    i = -1;
    do {
        i = ly_print_ext_iter(ext, ext_size, i + 1, substmt);
    } while (i != -1 && ext[i]->substmt_index != substmt_index);
    if (i != -1) {
        yin_print_close_parent(out, &content);
        do {
            yin_print_extension_instances(out, level + 1, module, substmt, substmt_index, &ext[i], 1);
            do {
                i = ly_print_ext_iter(ext, ext_size, i + 1, substmt);
            } while (i != -1 && ext[i]->substmt_index != substmt_index);
        } while (i != -1);
    }

    /* argument as yin-element */
    if (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) {
        yin_print_arg(out, level + 1, ext_substmt_info[substmt].arg, text);
    }

    yin_print_close(out, level, NULL, ext_substmt_info[substmt].name, content);
}

static void
yin_print_iffeature(struct lyout *out, int level, const struct lys_module *module, struct lys_iffeature *iffeature)
{
    ly_print(out, "%*s<if-feature name=\"", LEVEL, INDENT);
    ly_print_iffeature(out, module, iffeature);
    ly_print(out, "\"/>\n");
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
yin_print_snode_common(struct lyout *out, int level, const struct lys_node *node, const struct lys_module *module,
                       int *par_close_flag, int mask)
{
    int i;
    const char *status = NULL;

    /* extensions */
    if ((mask & SNODE_COMMON_EXT) && node->ext_size) {
        yin_print_close_parent(out, par_close_flag);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, node->ext, node->ext_size);
    }

    /* if-features */
    if (mask & SNODE_COMMON_IFF) {
        for (i = 0; i < node->iffeature_size; ++i) {
            yin_print_close_parent(out, par_close_flag);
            yin_print_iffeature(out, level, module, &node->iffeature[i]);
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
                    yin_print_close_parent(out, par_close_flag);
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true",
                                      module, node->ext, node->ext_size);
                } else if (node->flags & LYS_CONFIG_R) {
                    yin_print_close_parent(out, par_close_flag);
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false",
                                      module, node->ext, node->ext_size);
                }
            }
        } else if (node->flags & LYS_CONFIG_R) {
            /* ... or it's a top-level state node */
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false",
                              module, node->ext, node->ext_size);
        } else if (i != -1) {
            /* the config has an extension, so we have to print it */
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true",
                              module, node->ext, node->ext_size);
        }
    }

    /* mandatory */
    if ((mask & SNODE_COMMON_MAND) && (node->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA))) {
        if (node->flags & LYS_MAND_TRUE) {
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true",
                              module, node->ext, node->ext_size);
        } else if (node->flags & LYS_MAND_FALSE) {
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false",
                              module, node->ext, node->ext_size);
        }
    }

    /* status */
    if (mask & SNODE_COMMON_STATUS) {
        if (node->flags & LYS_STATUS_CURR) {
            yin_print_close_parent(out, par_close_flag);
            status = "current";
        } else if (node->flags & LYS_STATUS_DEPRC) {
            yin_print_close_parent(out, par_close_flag);
            status = "deprecated";
        } else if (node->flags & LYS_STATUS_OBSLT) {
            yin_print_close_parent(out, par_close_flag);
            status = "obsolete";
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_STATUS, 0, status, module, node->ext, node->ext_size);
    }

    /* description */
    if ((mask & SNODE_COMMON_DSC) && node->dsc) {
        yin_print_close_parent(out, par_close_flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, node->dsc,
                           module, node->ext, node->ext_size);
    }

    /* reference */
    if ((mask & SNODE_COMMON_REF) && node->ref) {
        yin_print_close_parent(out, par_close_flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, node->ref,
                          module, node->ext, node->ext_size);
    }
}

static void
yin_print_extension(struct lyout *out, int level, const struct lys_ext *ext)
{
    int close = 0, close2 = 0, i;

    yin_print_open(out, level, NULL, "extension", "name", ext->name, close);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)ext, ext->module, &close,
                           SNODE_COMMON_EXT);

    if (ext->argument) {
        yin_print_close_parent(out, &close);
        yin_print_open(out, level, NULL, "argument", "name", ext->argument, close2);
        i = -1;
        while ((i = ly_print_ext_iter(ext->ext, ext->ext_size, i + 1, LYEXT_SUBSTMT_ARGUMENT)) != -1) {
            yin_print_close_parent(out, &close2);
            yin_print_extension_instances(out, level + 1, ext->module, LYEXT_SUBSTMT_ARGUMENT, 0, &ext->ext[i], 1);
        }
        if (ext->flags & LYS_YINELEM) {
            yin_print_close_parent(out, &close2);
            yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_YINELEM, 0, "true", ext->module, ext->ext, ext->ext_size);
        }
        yin_print_close(out, level, NULL, "argument", close2);
    }

    yin_print_snode_common(out, level, (struct lys_node *)ext, ext->module, &close,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "extension", close);
}

static void
yin_print_restr(struct lyout *out, int level, const struct lys_module *module, const struct lys_restr *restr,
                    int *flag)
{
    if (restr->ext_size) {
        yin_print_close_parent(out, flag);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                      restr->ext, restr->ext_size);
    }
    if (restr->expr[0] == 0x15) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match", module,
                          restr->ext, restr->ext_size);
    }
    if (restr->emsg != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ERRMSG, 0, restr->emsg,
                          module, restr->ext, restr->ext_size);
    }
    if (restr->eapptag != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ERRTAG, 0, restr->eapptag,
                          module, restr->ext, restr->ext_size);
    }
    if (restr->dsc != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, restr->dsc,
                          module, restr->ext, restr->ext_size);
    }
    if (restr->ref != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, restr->ref,
                          module, restr->ext, restr->ext_size);
    }
}

/* covers length, range, pattern*/
static void
yin_print_typerestr(struct lyout *out, int level, const char *elem_name,
                    const struct lys_module *module, const struct lys_restr *restr)
{
    int content = 0;
    int pattern = 0;

    if (restr->expr[0] == 0x06 || restr->expr[0] == 0x15) {
        pattern = 1;
    }

    yin_print_open(out, level, NULL, elem_name, "value", pattern ? &restr->expr[1] : restr->expr , content);
    yin_print_restr(out, level + 1, module, restr, &content);
    yin_print_close(out, level, NULL, elem_name, content);
}

static void
yin_print_feature(struct lyout *out, int level, const struct lys_feature *feat)
{
    int close = 0;

    yin_print_open(out, level, NULL, "feature", "name", feat->name, close);
    yin_print_snode_common(out, level + 1, (struct lys_node *)feat, feat->module, &close, SNODE_COMMON_EXT |
                            SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    yin_print_close(out, level, NULL, "feature", close);
}

static void
yin_print_when(struct lyout *out, int level, const struct lys_module *module, const struct lys_when *when)
{
    int flag = 0;
    const char *str;

    str = transform_json2schema(module, when->cond);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*s<when condition=\"", LEVEL, INDENT);
    lyxml_dump_text(out, str);
    ly_print(out, "\"");
    lydict_remove(module->ctx, str);

    level++;

    if (when->ext_size) {
        /* extension is stored in lys_when incompatible with lys_node, so we cannot use yang_print_snode_common() */
        yin_print_close_parent(out, &flag);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, when->ext, when->ext_size);
    }
    yin_print_snode_common(out, level, (struct lys_node *)when, module, &flag, SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "when", flag);
}

static void
yin_print_unsigned(struct lyout *out, int level, LYEXT_SUBSTMT substmt, const struct lys_module *module,
                   struct lys_ext_instance **ext, unsigned int ext_size, unsigned int attr_value)
{
    char *str;

    asprintf(&str, "%u", attr_value);
    yin_print_substmt(out, level, substmt, 0, str, module, ext, ext_size);
    free(str);
}

static void
yin_print_signed(struct lyout *out, int level, LYEXT_SUBSTMT substmt, const struct lys_module *module,
                 struct lys_ext_instance **ext, unsigned int ext_size, signed int attr_value)
{
    char *str;

    asprintf(&str, "%d", attr_value);
    yin_print_substmt(out, level, substmt, 0, str, module, ext, ext_size);
    free(str);
}


static void
yin_print_type(struct lyout *out, int level, const struct lys_module *module, const struct lys_type *type)
{
    int i, content = 0, content2 = 0;
    const char *str;
    char *s;
    struct lys_module *mod;

    if (type->module_name) {
        ly_print(out, "%*s<type name=\"%s:%s\"", LEVEL, INDENT,
                 transform_module_name2import_prefix(module, type->module_name), type->der->name);
    } else {
        yin_print_open(out, level, NULL, "type", "name", type->der->name, content);
    }
    level++;

    /* extensions */
    if (type->ext_size) {
        yin_print_close_parent(out, &content);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, type->ext, type->ext_size);
    }


    switch (type->base) {
    case LY_TYPE_BINARY:
        if (type->info.binary.length) {
            yin_print_typerestr(out, level, "length", module, type->info.binary.length);
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            content2 = 0;
            yin_print_close_parent(out, &content);
            yin_print_open(out, level, NULL, "bit", "name", type->info.bits.bit[i].name, content2);
            yin_print_snode_common(out, level + 1, (struct lys_node *)&type->info.bits.bit[i], module, &content2,
                                   SNODE_COMMON_EXT | SNODE_COMMON_IFF);
            if (!(type->info.bits.bit[i].flags & LYS_AUTOASSIGNED)) {
                yin_print_close_parent(out, &content2);
                yin_print_unsigned(out, level + 1, LYEXT_SUBSTMT_POSITION, module,
                                   type->info.bits.bit[i].ext, type->info.bits.bit[i].ext_size,
                                   type->info.bits.bit[i].pos);
            }
            yin_print_snode_common(out, level, (struct lys_node *)&type->info.bits.bit[i], module, &content2,
                                   SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
            yin_print_close(out, level, NULL, "bit", content2);
        }
        break;
    case LY_TYPE_DEC64:
        if (!type->der->type.der) {
            yin_print_close_parent(out, &content);
            yin_print_unsigned(out, level, LYEXT_SUBSTMT_DIGITS, module,
                               type->ext, type->ext_size, type->info.dec64.dig);
        }
        if (type->info.dec64.range) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, "range", module, type->info.dec64.range);
        }
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            content2 = 0;
            yin_print_close_parent(out, &content);
            yin_print_open(out, level, NULL, "enum", "name", type->info.enums.enm[i].name, content2);
            yin_print_snode_common(out, level + 1, (struct lys_node *)&type->info.enums.enm[i], module, &content2,
                                   SNODE_COMMON_EXT | SNODE_COMMON_IFF);
            if (!(type->info.enums.enm[i].flags & LYS_AUTOASSIGNED)) {
                yin_print_close_parent(out, &content2);
                yin_print_signed(out, level + 1, LYEXT_SUBSTMT_VALUE, module,
                                 type->info.enums.enm[i].ext, type->info.enums.enm[i].ext_size,
                                 type->info.enums.enm[i].value);
            }
            yin_print_snode_common(out, level, (struct lys_node *)&type->info.enums.enm[i], module, &content2,
                                   SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
            yin_print_close(out, level, NULL, "enum", content2);
        }
        break;
    case LY_TYPE_IDENT:
        if (type->info.ident.count) {
            yin_print_close_parent(out, &content);
            for (i = 0; i < type->info.ident.count; ++i) {
                mod = lys_main_module(type->info.ident.ref[i]->module);
                if (lys_main_module(module) == mod) {
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, type->info.ident.ref[i]->name,
                                      module, type->info.ident.ref[i]->ext, type->info.ident.ref[i]->ext_size);
                } else {
                    asprintf(&s, "%s:%s", transform_module_name2import_prefix(module, mod->name),
                             type->info.ident.ref[i]->name);
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, s,
                                      module, type->info.ident.ref[i]->ext, type->info.ident.ref[i]->ext_size);
                    free(s);
                }
            }
        }
        break;
    case LY_TYPE_INST:
        if (type->info.inst.req == 1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "true", module, type->ext, type->ext_size);
        } else if (type->info.inst.req == -1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "false", module, type->ext, type->ext_size);
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
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, "range", module, type->info.num.range);
        }
        break;
    case LY_TYPE_LEAFREF:
        if (ly_strequal(type->der->name, "leafref", 0)) {
            yin_print_close_parent(out, &content);
            str = transform_json2schema(module, type->info.lref.path);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_PATH, 0, str, module, type->ext, type->ext_size);
            lydict_remove(module->ctx, str);
        }
        if (type->info.lref.req == 1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "true", module, type->ext, type->ext_size);
        } else if (type->info.lref.req == -1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINST, 0, "false", module, type->ext, type->ext_size);
        }
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, "length", module, type->info.str.length);
        }
        for (i = 0; i < type->info.str.pat_count; i++) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, "pattern", module, &type->info.str.patterns[i]);
        }
        break;
    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; ++i) {
            yin_print_close_parent(out, &content);
            yin_print_type(out, level, module, &type->info.uni.types[i]);
        }
        break;
    default:
        /* other types do not have substatements */
        break;
    }

    level--;
    yin_print_close(out, level, NULL, "type", content);
}

static void
yin_print_must(struct lyout *out, int level, const struct lys_module *module, const struct lys_restr *must)
{
    const char *str;
    int content = 0;

    str = transform_json2schema(module, must->expr);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*s<must condition=\"", LEVEL, INDENT);
    lyxml_dump_text(out, str);
    ly_print(out, "\"");
    lydict_remove(module->ctx, str);

    yin_print_restr(out, level + 1, module, must, &content);
    yin_print_close(out, level, NULL, "must", content);
}

static void
yin_print_unique(struct lyout *out, int level, const struct lys_unique *uniq)
{
    int i;

    ly_print(out, "%*s<unique tag=\"", LEVEL, INDENT);
    for (i = 0; i < uniq->expr_size; i++) {
        ly_print(out, "%s%s", uniq->expr[i], i + 1 < uniq->expr_size ? " " : "");
    }
    ly_print(out, "\"");
}

static void
yin_print_refine(struct lyout *out, int level, const struct lys_module *module, const struct lys_refine *refine)
{
    int i, content = 0;
    const char *str;

    str = transform_json2xml(module, refine->target_name, NULL, NULL, NULL);
    yin_print_open(out, level, NULL, "refine", "target-node", str, content);
    lydict_remove(module->ctx, str);

    level++;
    yin_print_snode_common(out, level, (struct lys_node *)refine, module, &content,
                           SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < refine->must_size; ++i) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, module, &refine->must[i]);
    }
    if (refine->target_type == LYS_CONTAINER && refine->mod.presence) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PRESENCE, 0, refine->mod.presence,
                          module, refine->ext, refine->ext_size);
    }
    for (i = 0; i < refine->dflt_size; ++i) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, i, refine->dflt[i],
                          module, refine->ext, refine->ext_size);
    }
    if (refine->flags & LYS_CONFIG_W) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true", module, refine->ext, refine->ext_size);
    } else if (refine->flags & LYS_CONFIG_R) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false", module, refine->ext, refine->ext_size);
    }
    if (refine->flags & LYS_MAND_TRUE) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true", module, refine->ext, refine->ext_size);
    } else if (refine->flags & LYS_MAND_FALSE) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false", module, refine->ext, refine->ext_size);
    }
    if (refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
        if (refine->flags & LYS_RFN_MINSET) {
            yin_print_close_parent(out, &content);
            yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, module, refine->ext, refine->ext_size,
                               refine->mod.list.min);
        }
        if (refine->flags & LYS_RFN_MAXSET) {
            yin_print_close_parent(out, &content);
            if (refine->mod.list.max) {
                yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, module, refine->ext, refine->ext_size,
                                   refine->mod.list.min);
            } else {
                yin_print_substmt(out, level, LYEXT_SUBSTMT_MAX, 0, "unbounded", module,
                                  refine->ext, refine->ext_size);
            }
        }
    }
    yin_print_snode_common(out, level, (struct lys_node *)refine, module, &content,
                           SNODE_COMMON_DSC | SNODE_COMMON_REF);
    level--;

    yin_print_close(out, level, NULL, "refine", content);
}

static void
yin_print_deviation(struct lyout *out, int level, const struct lys_module *module,
                    const struct lys_deviation *deviation)
{
    int i, j, p, content;
    const char *str;

    str = transform_json2schema(module, deviation->target_name);
    yin_print_open(out, level, NULL, "deviation", "target-node", str, 1);
    lydict_remove(module->ctx, str);

    level++;

    if (deviation->ext_size) {
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, deviation->ext, deviation->ext_size);
    }
    yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, deviation->dsc,
                      module, deviation->ext, deviation->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, deviation->ref,
                      module, deviation->ext, deviation->ext_size);

    for (i = 0; i < deviation->deviate_size; ++i) {
        ly_print(out, "%*s<deviate value=", LEVEL, INDENT);
        if (deviation->deviate[i].mod == LY_DEVIATE_NO) {
            ly_print(out, "\"not-supported\"/>\n");
            continue;
        } else if (deviation->deviate[i].mod == LY_DEVIATE_ADD) {
            ly_print(out, "\"add\">\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_RPL) {
            ly_print(out, "\"replace\">\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_DEL) {
            ly_print(out, "\"delete\">\n");
        }
        level++;

        /* extensions */
        if (deviation->deviate[i].ext_size) {
            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_SELF, 0,
                                          deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* type */
        if (deviation->deviate[i].type) {
            yin_print_type(out, level, module, deviation->deviate[i].type);
        }

        /* units */
        yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, deviation->deviate[i].units, module,
                          deviation->deviate[i].ext, deviation->deviate[i].ext_size);

        /* must */
        for (j = 0; j < deviation->deviate[i].must_size; ++j) {
            yin_print_must(out, level, module, &deviation->deviate[i].must[j]);
        }

        /* unique */
        for (j = 0; j < deviation->deviate[i].unique_size; ++j) {
            yin_print_unique(out, level, &deviation->deviate[i].unique[j]);
            content = 0;
            /* unique's extensions */
            p = -1;
            do {
                p = ly_print_ext_iter(deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                      p + 1, LYEXT_SUBSTMT_UNIQUE);
            } while (p != -1 && deviation->deviate[i].ext[p]->substmt_index != j);
            if (p != -1) {
                yin_print_close_parent(out, &content);
                do {
                    yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_UNIQUE, j,
                                                  &deviation->deviate[i].ext[p], 1);
                    do {
                        p = ly_print_ext_iter(deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                              p + 1, LYEXT_SUBSTMT_UNIQUE);
                    } while (p != -1 && deviation->deviate[i].ext[p]->substmt_index != j);
                } while (p != -1);
            }
            yin_print_close(out, level, NULL, "unique", content);
        }

        /* default */
        for (j = 0; j < deviation->deviate[i].dflt_size; ++j) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, j, deviation->deviate[i].dflt[j], module,
                              deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* config */
        if (deviation->deviate[i].flags & LYS_CONFIG_W) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true", module,
                              deviation->deviate->ext, deviation->deviate[i].ext_size);
        } else if (deviation->deviate[i].flags & LYS_CONFIG_R) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false", module,
                               deviation->deviate->ext, deviation->deviate[i].ext_size);
        }

        /* mandatory */
        if (deviation->deviate[i].flags & LYS_MAND_TRUE) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true", module,
                              deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        } else if (deviation->deviate[i].flags & LYS_MAND_FALSE) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false", module,
                              deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* min-elements */
        if (deviation->deviate[i].min_set) {
            yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, module,
                               deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                               deviation->deviate[i].min);
        }

        /* max-elements */
        if (deviation->deviate[i].max_set) {
            if (deviation->deviate[i].max) {
                yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, module,
                                   deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                   deviation->deviate[i].max);
            } else {
                yin_print_substmt(out, level, LYEXT_SUBSTMT_MAX, 0, "unbounded", module,
                                  deviation->deviate[i].ext, deviation->deviate[i].ext_size);
            }
        }
        level--;

        yin_print_close(out, level, NULL, "deviate", 1);
    }

    level--;
    yin_print_close(out, level, NULL, "deviation", 1);
}

static void
yin_print_augment(struct lyout *out, int level, const struct lys_module *module,
                  const struct lys_node_augment *augment)
{
    struct lys_node *sub;
    const char *str;

    str = transform_json2schema(module, augment->target_name);
    yin_print_open(out, level, NULL, "augment", "target-node", str, 1);
    lydict_remove(module->ctx, str);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)augment, augment->module, NULL, SNODE_COMMON_EXT);
    if (augment->when) {
        yin_print_when(out, level, module, augment->when);
    }
    yin_print_snode_common(out, level, (struct lys_node *)augment, augment->module, NULL,
                           SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);


    LY_TREE_FOR(augment->child, sub) {
        /* only our augment */
        if (sub->parent != (struct lys_node *)augment) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA | LYS_CASE | LYS_ACTION | LYS_NOTIF);
    }
    level--;

    yin_print_close(out, level, NULL, "augment", 1);
}

static void
yin_print_typedef(struct lyout *out, int level, const struct lys_module *module, const struct lys_tpdf *tpdf)
{
    const char *dflt;

    yin_print_open(out, level, NULL, "typedef", "name", tpdf->name, 1);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)tpdf, module, NULL, SNODE_COMMON_EXT);
    yin_print_type(out, level, module, &tpdf->type);
    if (tpdf->units) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, tpdf->units, module, tpdf->ext, tpdf->ext_size);
    }
    if (tpdf->dflt) {
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
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, dflt, module, tpdf->ext, tpdf->ext_size);
        if (tpdf->flags & LYS_DFLTJSON) {
            lydict_remove(module->ctx, dflt);
        }
    }
    yin_print_snode_common(out, level, (struct lys_node *)tpdf, module, NULL,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "typedef", 1);
}

static void
yin_print_identity(struct lyout *out, int level, const struct lys_ident *ident)
{
    int content = 0, i;
    struct lys_module *mod;
    char *str;

    yin_print_open(out, level, NULL, "identity", "name", ident->name, content);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)ident, ident->module, &content,
                           SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < ident->base_size; i++) {
        yin_print_close_parent(out, &content);
        mod = lys_main_module(ident->base[i]->module);
        if (lys_main_module(ident->module) == mod) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, ident->base[i]->name,
                              ident->module, ident->ext, ident->ext_size);
        } else {
            asprintf(&str, "%s:%s", transform_module_name2import_prefix(ident->module, mod->name), ident->base[i]->name);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, str,
                              ident->module, ident->ext, ident->ext_size);
            free(str);
        }
    }
    yin_print_snode_common(out, level, (struct lys_node *)ident, ident->module, &content,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "identity", content);
}

static void
yin_print_container(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_container *cont = (struct lys_node_container *)node;

    yin_print_open(out, level, NULL, "container", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (cont->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, cont->when);
    }

    for (i = 0; i < cont->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &cont->iffeature[i]);
    }

    for (i = 0; i < cont->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, node->module, &cont->must[i]);
    }

    if (cont->presence) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PRESENCE, 0, cont->presence,
                          node->module, node->ext, node->ext_size);
    }
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_CONFIG |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < cont->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &cont->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA );
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_ACTION);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yin_print_close(out, level, NULL, "container", content);
}

static void
yin_print_case(struct lyout *out, int level, const struct lys_node *node)
{
    int content = 0;
    struct lys_node *sub;
    struct lys_node_case *cas = (struct lys_node_case *)node;

    yin_print_open(out, level, NULL, "case", "name", cas->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (cas->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, cas->when);
    }
    yin_print_snode_common(out, level, node, node->module, &content,
                           SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }

    level--;
    yin_print_close(out, level, NULL, "case", content);
}

static void
yin_print_choice(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    yin_print_open(out, level, NULL, "choice", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (choice->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, choice->when);
    }
    for (i = 0; i < choice->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &choice->iffeature[i]);
    }
    if (choice->dflt) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt->name,
                          node->module, node->ext, node->ext_size);
    }

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub,
                        LYS_CHOICE |LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_CASE);
    }

    level--;
    yin_print_close(out, level, NULL, "choice", content);
}

static void
yin_print_leaf(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;
    const char *dflt;

    yin_print_open(out, level, NULL, "leaf", "name", node->name, 1);
    level++;

    yin_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_EXT);
    if (leaf->when) {
        yin_print_when(out, level, node->module, leaf->when);
    }
    for (i = 0; i < leaf->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &leaf->iffeature[i]);
    }
    yin_print_type(out, level, node->module, &leaf->type);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, leaf->units,
                      node->module, node->ext, node->ext_size);
    for (i = 0; i < leaf->must_size; i++) {
        yin_print_must(out, level, node->module, &leaf->must[i]);
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
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, dflt,
                          node->module, node->ext, node->ext_size);
        if (leaf->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    yin_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "leaf", 1);
}

static void
yin_print_anydata(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node_anydata *any = (struct lys_node_anydata *)node;
    const char *name;

    name = any->nodetype == LYS_ANYXML ? "anyxml" : "anydata";
    yin_print_open(out, level, NULL, name, "name", any->name, content);

    level++;
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (any->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, any->when);
    }
    for (i = 0; i < any->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &any->iffeature[i]);
    }
    for (i = 0; i < any->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, node->module, &any->must[i]);
    }
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, name, content);
}

static void
yin_print_leaflist(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;
    const char *dflt;

    yin_print_open(out, level, NULL, "leaf-list", "name", node->name, 1);
    level++;

    yin_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_EXT);
    if (llist->when) {
        yin_print_when(out, level, llist->module, llist->when);
    }
    for (i = 0; i < llist->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &llist->iffeature[i]);
    }
    yin_print_type(out, level, node->module, &llist->type);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, llist->units,
                      node->module, node->ext, node->ext_size);
    for (i = 0; i < llist->must_size; i++) {
        yin_print_must(out, level, node->module, &llist->must[i]);
    }
    for (i = 0; i < llist->dflt_size; i++) {
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
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, i, dflt,
                          node->module, node->ext, node->ext_size);
        if (llist->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    if (llist->min > 0) {
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, node->module, node->ext, node->ext_size, llist->min);
    }
    if (llist->max > 0) {
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, node->module, node->ext, node->ext_size, llist->max);
    }
    if (llist->flags & LYS_USERORDERED) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "user",
                          node->module, node->ext, node->ext_size);
    } else if (ly_print_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_ORDEREDBY) != -1) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "system",
                          node->module, node->ext, node->ext_size);
    }
    yin_print_snode_common(out, level, node, node->module, NULL,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "leaf-list", 1);
}

static void
yin_print_list(struct lyout *out, int level, const struct lys_node *node)
{
    int i, p, content = 0, content2;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;

    yin_print_open(out, level, NULL, "list", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (list->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, list->module, list->when);
    }
    for (i = 0; i < list->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &list->iffeature[i]);
    }
    for (i = 0; i < list->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, list->module, &list->must[i]);
    }
    if (list->keys_size) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_KEY, 0, list->keys_str,
                          node->module, node->ext, node->ext_size);
    }
    for (i = 0; i < list->unique_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_unique(out, level, &list->unique[i]);
        content2 = 0;
        /* unique's extensions */
        p = -1;
        do {
            p = ly_print_ext_iter(list->ext, list->ext_size, p + 1, LYEXT_SUBSTMT_UNIQUE);
        } while (p != -1 && list->ext[p]->substmt_index != i);
        if (p != -1) {
            yin_print_close_parent(out, &content2);
            do {
                yin_print_extension_instances(out, level + 1, list->module, LYEXT_SUBSTMT_UNIQUE, i, &list->ext[p], 1);
                do {
                    p = ly_print_ext_iter(list->ext, list->ext_size, p + 1, LYEXT_SUBSTMT_UNIQUE);
                } while (p != -1 && list->ext[p]->substmt_index != i);
            } while (p != -1);
        }
        yin_print_close(out, level, NULL, "unique", content2);
    }
    if (list->min > 0) {
        yin_print_close_parent(out, &content);
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, node->module, node->ext, node->ext_size, list->min);
    }
    if (list->max > 0) {
        yin_print_close_parent(out, &content);
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, node->module, node->ext, node->ext_size, list->max);
    }
    if (list->flags & LYS_USERORDERED) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "user",
                          node->module, node->ext, node->ext_size);
    } else if (ly_print_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_ORDEREDBY) != -1) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "system",
                          node->module, node->ext, node->ext_size);
    }
    yin_print_snode_common(out, level, node, node->module, NULL,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < list->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, list->module, &list->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_GROUPING | LYS_ANYDATA);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_ACTION);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yin_print_close(out, level, NULL, "list", content);
}

static void
yin_print_grouping(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_grp *grp = (struct lys_node_grp *)node;

    yin_print_open(out, level, NULL, "grouping", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT | SNODE_COMMON_STATUS |
                           SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < grp->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &grp->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_ACTION);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yin_print_close(out, level, NULL, "grouping", content);
}

static void
yin_print_uses(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node_uses *uses = (struct lys_node_uses *)node;
    struct lys_module *mod;

    ly_print(out, "%*s<uses name=\"", LEVEL, INDENT);
    if (node->child) {
        mod = lys_node_module(node->child);
        if (lys_node_module(node) != mod) {
            ly_print(out, "%s:", transform_module_name2import_prefix(node->module, mod->name));
        }
    }
    ly_print(out, "%s\"", uses->name);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (uses->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, uses->when);
    }
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_IFF |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < uses->refine_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_refine(out, level, node->module, &uses->refine[i]);
    }
    for (i = 0; i < uses->augment_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_augment(out, level, node->module, &uses->augment[i]);
    }

    level--;
    yin_print_close(out, level, NULL, "uses", content);
}

static void
yin_print_input_output(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_inout *inout = (struct lys_node_inout *)node;

    if (node->flags & LYS_IMPLICIT) {
        /* implicit input/output which is not a part of the schema */
        return;
    }

    yin_print_open(out, level, NULL, inout->nodetype == LYS_INPUT ? "input" : "output", NULL, NULL, 1);
    level++;

    if (inout->ext_size) {
        yin_print_extension_instances(out, level, node->module, LYEXT_SUBSTMT_SELF, 0, inout->ext, inout->ext_size);
    }
    for (i = 0; i < inout->must_size; i++) {
        yin_print_must(out, level, node->module, &inout->must[i]);
    }
    for (i = 0; i < inout->tpdf_size; i++) {
        yin_print_typedef(out, level, node->module, &inout->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }
    level--;

    yin_print_close(out, level, NULL, (inout->nodetype == LYS_INPUT ? "input" : "output"), 1);
}

static void
yin_print_rpc_action(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_rpc_action *rpc = (struct lys_node_rpc_action *)node;

    yin_print_open(out, level, NULL, (node->nodetype == LYS_RPC ? "rpc" : "action"), "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT | SNODE_COMMON_IFF |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < rpc->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &rpc->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_INPUT);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_OUTPUT);
    }

    level--;
    yin_print_close(out, level, NULL, (node->nodetype == LYS_RPC ? "rpc" : "action"), content);
}

static void
yin_print_notif(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_notif *notif = (struct lys_node_notif *)node;

    yin_print_open(out, level, NULL, "notification", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < notif->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, node->module, &notif->must[i]);
    }
    yin_print_snode_common(out, level, node, node->module, &content,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < notif->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &notif->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    level--;
    yin_print_close(out, level, NULL, "notification", content);
}

static void
yin_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask)
{
    switch (node->nodetype & mask) {
    case LYS_CONTAINER:
        yin_print_container(out, level, node);
        break;
    case LYS_CHOICE:
        yin_print_choice(out, level, node);
        break;
    case LYS_LEAF:
        yin_print_leaf(out, level, node);
        break;
    case LYS_LEAFLIST:
        yin_print_leaflist(out, level, node);
        break;
    case LYS_LIST:
        yin_print_list(out, level, node);
        break;
    case LYS_USES:
        yin_print_uses(out, level, node);
        break;
    case LYS_GROUPING:
        yin_print_grouping(out, level, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        yin_print_anydata(out, level, node);
        break;
    case LYS_CASE:
        yin_print_case(out, level, node);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        yin_print_rpc_action(out, level, node);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        yin_print_input_output(out, level, node);
        break;
    case LYS_NOTIF:
        yin_print_notif(out, level, node);
        break;
    default:
        break;
    }
}

static void
yin_print_xmlns(struct lyout *out, const struct lys_module *module)
{
    unsigned int i, lvl;

    if (module->type) {
        lvl = 11;
    } else {
        lvl = 8;
    }

    ly_print(out, "%*sxmlns=\"%s\"", lvl, INDENT, LY_NSYIN);
    if (!module->type) {
        ly_print(out, "\n%*sxmlns:%s=\"%s\"", lvl, INDENT, module->prefix, module->ns);
    }
    for (i = 0; i < module->imp_size; ++i) {
        ly_print(out, "\n%*sxmlns:%s=\"%s\"", lvl, INDENT, module->imp[i].prefix, module->imp[i].module->ns);
    }
}

int
yin_print_model(struct lyout *out, const struct lys_module *module)
{
    unsigned int i;
    int level = 0, p;
    struct lys_node *node;

    ly_print(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

    if (module->deviated == 1) {
        ly_print(out, "<!-- DEVIATED -->\n");
    }

    /* (sub)module-header-stmts */
    if (module->type) {
        ly_print(out, "<submodule name=\"%s\"\n", module->name);
        yin_print_xmlns(out, module);
        ly_print(out, ">\n");

        level++;
        if (module->version) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_VERSION, 0,
                              ((struct lys_submodule *)module)->belongsto->version == 2 ? "1.1" : "1",
                              module, module->ext, module->ext_size);
        }
        yin_print_open(out, level, NULL, "belongs-to", "module", ((struct lys_submodule *)module)->belongsto->name, 1);
        p = -1;
        while ((p = ly_print_ext_iter(module->ext, module->ext_size, p + 1, LYEXT_SUBSTMT_BELONGSTO)) != -1) {
            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_BELONGSTO, 0, &module->ext[p], 1);
        }
        yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_PREFIX, 0, module->prefix,
                          module, module->ext, module->ext_size);
        yin_print_close(out, level, NULL, "belongs-to", 1);
    } else {
        ly_print(out, "<module name=\"%s\"\n", module->name);
        yin_print_xmlns(out, module);
        ly_print(out, ">\n");

        level++;
        if (module->version) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_VERSION, 0, module->version == 2 ? "1.1" : "1",
                              module, module->ext, module->ext_size);
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns,
                          module, module->ext, module->ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PREFIX, 0, module->prefix,
                           module, module->ext, module->ext_size);
    }

    /* linkage-stmts */
    for (i = 0; i < module->imp_size; i++) {
        yin_print_open(out, level, NULL, "import", "module", module->imp[i].module->name, 1);
        level++;

        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                       module->imp[i].ext, module->imp[i].ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PREFIX, 0, module->imp[i].prefix,
                          module, module->imp[i].ext, module->imp[i].ext_size);
        if (module->imp[i].rev[0]) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REVISIONDATE, 0, module->imp[i].rev,
                              module, module->imp[i].ext, module->imp[i].ext_size);
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->imp[i].dsc,
                          module, module->imp[i].ext, module->imp[i].ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->imp[i].ref,
                          module, module->imp[i].ext, module->imp[i].ext_size);

        level--;
        yin_print_close(out, level, NULL, "import", 1);
    }
    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].external) {
            continue;
        }

        if (module->inc[i].rev[0] || module->inc[i].dsc || module->inc[i].ref || module->inc[i].ext_size) {
            yin_print_open(out, level, NULL, "include", "module", module->inc[i].submodule->name, 1);
            level++;

            yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                          module->inc[i].ext, module->inc[i].ext_size);
            if (module->inc[i].rev[0]) {
                yin_print_substmt(out, level, LYEXT_SUBSTMT_REVISIONDATE, 0, module->inc[i].rev,
                                  module, module->inc[i].ext, module->inc[i].ext_size);
            }
            yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->inc[i].dsc,
                               module, module->inc[i].ext, module->inc[i].ext_size);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->inc[i].ref,
                              module, module->inc[i].ext, module->inc[i].ext_size);

            level--;
            yin_print_close(out, level, NULL, "include", 1);
        } else {
            yin_print_open(out, level, NULL, "include", "module", module->inc[i].submodule->name, -1);
        }
    }

    /* meta-stmts */
    yin_print_substmt(out, level, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org,
                      module, module->ext, module->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_CONTACT, 0, module->contact,
                      module, module->ext, module->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc,
                      module, module->ext, module->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->ref,
                      module, module->ext, module->ext_size);

    /* revision-stmts */
    for (i = 0; i < module->rev_size; i++) {
        if (module->rev[i].dsc || module->rev[i].ref || module->ext_size) {
            yin_print_open(out, level, NULL, "revision", "date", module->rev[i].date, 1);
            level++;
            yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                          module->rev[i].ext, module->rev[i].ext_size);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->rev[i].dsc,
                              module, module->rev[i].ext, module->rev[i].ext_size);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->rev[i].ref,
                               module, module->rev[i].ext, module->rev[i].ext_size);
            level--;
            yin_print_close(out, level, NULL, "revision", 1);
        } else {
            yin_print_open(out, level, NULL, "revision", "date", module->rev[i].date, -1);
        }
    }

    /* body-stmts */
    for (i = 0; i < module->extensions_size; ++i) {
        yin_print_extension(out, level, &module->extensions[i]);
    }
    if (module->ext_size) {
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, module->ext, module->ext_size);
    }

    for (i = 0; i < module->features_size; i++) {
        yin_print_feature(out, level, &module->features[i]);
    }

    for (i = 0; i < module->ident_size; i++) {
        yin_print_identity(out, level, &module->ident[i]);
    }

    for (i = 0; i < module->tpdf_size; i++) {
        yin_print_typedef(out, level, module, &module->tpdf[i]);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_GROUPING);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }

    for (i = 0; i < module->augment_size; i++) {
        yin_print_augment(out, level, module, &module->augment[i]);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_RPC | LYS_ACTION);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_NOTIF);
    }

    for (i = 0; i < module->deviation_size; ++i) {
        yin_print_deviation(out, level, module, &module->deviation[i]);
    }

    if (module->type) {
        ly_print(out, "</submodule>\n");
    } else {
        ly_print(out, "</module>\n");
    }
    ly_print_flush(out);

    return EXIT_SUCCESS;
#undef LEVEL
}

