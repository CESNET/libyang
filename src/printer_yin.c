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

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"
#include "xml_internal.h"

#define INDENT ""
#define LEVEL (level*2)

static void yin_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask);

static void
yin_print_open(struct lyout *out, int level, const char *elem_name, const char *attr_name, const char *attr_value,
               int close)
{
    ly_print(out, "%*s<%s %s=\"%s\"%s>\n", LEVEL, INDENT, elem_name, attr_name, attr_value, (close ? "/" : ""));
}

static void
yin_print_close(struct lyout *out, int level, const char *elem_name)
{
    ly_print(out, "%*s</%s>\n", LEVEL, INDENT, elem_name);
}

static void
yin_print_unsigned(struct lyout *out, int level, const char *elem_name, const char *attr_name, unsigned int attr_value)
{
    ly_print(out, "%*s<%s %s=\"%u\"/>\n", LEVEL, INDENT, elem_name, attr_name, attr_value);
}

static void
yin_print_text(struct lyout *out, int level, const char *elem_name, const char *text)
{
    ly_print(out, "%*s<%s>\n", LEVEL, INDENT, elem_name);

    level++;
    ly_print(out, "%*s<text>", LEVEL, INDENT);
    lyxml_dump_text(out, text);
    ly_print(out, "</text>\n");
    level--;

    ly_print(out, "%*s</%s>\n", LEVEL, INDENT, elem_name);
}

static void
yin_print_restr_sub(struct lyout *out, int level, const struct lys_restr *restr)
{
    if (restr->dsc) {
        yin_print_text(out, level, "description", restr->dsc);
    }
    if (restr->ref) {
        yin_print_text(out, level, "reference", restr->ref);
    }
    if (restr->eapptag) {
        yin_print_open(out, level, "error-app-tag", "value", restr->eapptag, 1);
    }
    if (restr->emsg) {
        ly_print(out, "%*s<error-message>\n", LEVEL, INDENT);

        level++;
        ly_print(out, "%*s<value>", LEVEL, INDENT, restr->emsg);
        lyxml_dump_text(out, restr->emsg);
        ly_print(out, "</value>\n");
        level--;

        yin_print_close(out, level, "error-message");
    }
}

static void
yin_print_restr(struct lyout *out, int level, const char *elem_name, const struct lys_restr *restr)
{
    int close;
    int pattern = 0;

    if (restr->expr[0] == 0x06 || restr->expr[0] == 0x15) {
        pattern = 1;
    }
    close = (restr->dsc || restr->ref || restr->eapptag || restr->emsg || restr->expr[0] == 0x15 ? 0 : 1);

    yin_print_open(out, level, elem_name, "value", pattern ? &restr->expr[1] : restr->expr , close);
    if (!close) {
        if (restr->expr[0] == 0x15) {
            yin_print_open(out, level + 1, "modifier", "value", "invert-match", 1);
        }
        yin_print_restr_sub(out, level + 1, restr);

        yin_print_close(out, level, elem_name);
    }
}

static int
yin_has_nacmext(const struct lys_node *node)
{
    if (node->nacm && (!lys_parent(node) || lys_parent(node)->nacm != node->nacm)) {
        return 1;
    }
    return 0;
}

static void
yin_print_nacmext(struct lyout *out, int level, const struct lys_node *node, const struct lys_module *module)
{
    int i, j;
    const char *prefix = NULL;

    if (node->nacm && (!lys_parent(node) || lys_parent(node)->nacm != node->nacm)) {
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

        if ((node->nacm & LYS_NACM_DENYW) && (!lys_parent(node) || !(lys_parent(node)->nacm & LYS_NACM_DENYW))) {
            ly_print(out, "%*s<%s:default-deny-write/>\n", LEVEL, INDENT, prefix);
        }
        if ((node->nacm & LYS_NACM_DENYA) && (!lys_parent(node) || !(lys_parent(node)->nacm & LYS_NACM_DENYA))) {
            ly_print(out, "%*s<%s:default-deny-all/>\n", LEVEL, INDENT, prefix);
        }
    }
}

static int
yin_has_snode_common(const struct lys_node *node)
{
    if ((node->flags & LYS_STATUS_MASK) || node->dsc || node->ref || node->ext_size) {
        return 1;
    }
    return 0;
}

static void
yin_print_extension_instances(struct lyout *out, int level, const struct lys_module *module,
                              struct lys_ext_instance **ext, unsigned int count)
{
    unsigned int u, x;
    struct lys_module *mod;
    const char *prefix = NULL;
    int content;

    for (u = 0; u < count; u++) {
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

        content = 0;
        ly_print(out, "%*s<%s:%s", LEVEL, INDENT, prefix, ext[u]->def->name);

        if (!ext[u]->def->plugin || ext[u]->def->plugin->type == LY_EXT_FLAG) {
            /* flag extension */
            if (((struct lys_ext_instance_flag *)ext[u])->arg_value) {
                if (ext[u]->def->flags & LYS_YINELEM) {
                    content = 1;
                    level++;
                    ly_print(out, ">\n%*s<%s:%s>%s</%s:%s>\n", LEVEL, INDENT, prefix, ext[u]->def->argument,
                             ((struct lys_ext_instance_flag *)ext[u])->arg_value, prefix, ext[u]->def->argument);
                    level--;
                } else {
                    ly_print(out, " %s=\"%s\"", ext[u]->def->argument,
                             ((struct lys_ext_instance_flag *)ext[u])->arg_value);
                }
            }
        }

        /* extensions */
        if (ext[u]->ext_size) {
            if (!content) {
                content = 1;
                ly_print(out, ">\n");
            }

            yin_print_extension_instances(out, level + 1, module, ext[u]->ext, ext[u]->ext_size);
        }

        /* close extension */
        if (content) {
            ly_print(out, "%*s</%s:%s>\n", LEVEL, INDENT, prefix, ext[u]->def->name);
        } else {
            ly_print(out, "/>\n");
        }
    }
}

/*
 * Covers:
 * description, reference, status
 */
static void
yin_print_snode_common(struct lyout *out, int level, const struct lys_node *node)
{
    if (node->flags & LYS_STATUS_CURR) {
        yin_print_open(out, level, "status", "value", "current", 1);
    } else if (node->flags & LYS_STATUS_DEPRC) {
        yin_print_open(out, level, "status", "value", "deprecated", 1);
    } else if (node->flags & LYS_STATUS_OBSLT) {
        yin_print_open(out, level, "status", "value", "obsolete", 1);
    }

    if (node->dsc) {
        yin_print_text(out, level, "description", node->dsc);
    }
    if (node->ref) {
        yin_print_text(out, level, "reference", node->ref);
    }
    if (node->ext_size) {
        yin_print_extension_instances(out, level, node->module, node->ext, node->ext_size);
    }
}

static int
yin_has_snode_common2(const struct lys_node *node)
{
    if ((lys_parent(node) && (lys_parent(node)->flags & LYS_CONFIG_MASK) != (node->flags & LYS_CONFIG_MASK))
            || (!lys_parent(node) && (node->flags & LYS_CONFIG_R)) || (node->flags & LYS_MAND_MASK)) {
        return 1;
    }
    return yin_has_snode_common(node);
}

/*
 * Covers:
 * config, mandatory
 * description, reference, status
 */
static void
yin_print_snode_common2(struct lyout *out, int level, const struct lys_node *node)
{
    if (lys_parent(node)) {
        if (node->flags & LYS_CONFIG_SET) {
            /* print config when it differs from the parent ... */
            if (node->flags & LYS_CONFIG_W) {
                yin_print_open(out, level, "config", "value", "true", 1);
            } else if (node->flags & LYS_CONFIG_R) {
                yin_print_open(out, level, "config", "value", "false", 1);
            }
        }
    } else if (node->flags & LYS_CONFIG_R) {
        /* ... or is a top-level state node */
        yin_print_open(out, level, "config", "value", "false", 1);
    }

    if (node->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA)) {
        if (node->flags & LYS_MAND_TRUE) {
            yin_print_open(out, level, "mandatory", "value", "true", 1);
        } else if (node->flags & LYS_MAND_FALSE) {
            yin_print_open(out, level, "mandatory", "value", "false", 1);
        }
    }

    yin_print_snode_common(out, level, node);
}

static void
yin_print_iffeature(struct lyout *out, int level, const struct lys_module *module, struct lys_iffeature *iffeature)
{
    ly_print(out, "%*s<if-feature name=\"", LEVEL, INDENT);
    ly_print_iffeature(out, module, iffeature);
    ly_print(out, "\"/>\n");
}

static void
yin_print_feature(struct lyout *out, int level, const struct lys_feature *feat)
{
    int i, close;

    close = (yin_has_snode_common((struct lys_node *)feat) || feat->iffeature_size ? 0 : 1);

    yin_print_open(out, level, "feature", "name", feat->name, close);

    if (!close) {
        level++;
        yin_print_snode_common(out, level, (struct lys_node *)feat);
        for (i = 0; i < feat->iffeature_size; ++i) {
            yin_print_iffeature(out, level, feat->module, &feat->iffeature[i]);
        }
        level--;

        yin_print_close(out, level, "feature");
    }
}

static void
yin_print_extension(struct lyout *out, int level, const struct lys_ext *ext)
{
    int close, close2;

    close = (yin_has_snode_common((struct lys_node *)ext) || ext->argument ? 0 : 1);

    yin_print_open(out, level, "extension", "name", ext->name, close);
    if (!close) {
        level++;
        yin_print_snode_common(out, level, (struct lys_node *)ext);
        if (ext->argument) {
            close2 = ext->flags & LYS_YINELEM ? 0 : 1;
            yin_print_open(out, level, "argument", "name", ext->argument, close2);
            if (!close2) {
                yin_print_open(out, level + 1, "yin-element", "value", "true", 1);
                yin_print_close(out, level, "argument");
            }
        }
        level--;

        yin_print_close(out, level, "extension");
    }
}

static void
yin_print_when(struct lyout *out, int level, const struct lys_module *module, const struct lys_when *when)
{
    int close;
    const char *str;

    close = (when->dsc || when->ref ? 0 : 1);

    str = transform_json2schema(module, when->cond);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*s<when condition=\"", LEVEL, INDENT);
    lyxml_dump_text(out, str);
    ly_print(out, "\"%s>\n", (close ? "/" : ""));

    lydict_remove(module->ctx, str);

    if (!close) {
        level++;
        if (when->dsc) {
            yin_print_text(out, level, "description", when->dsc);
        }
        if (when->ref) {
            yin_print_text(out, level, "reference", when->ref);
        }
        level--;

        yin_print_close(out, level, "when");
    }
}

static void
yin_print_type(struct lyout *out, int level, const struct lys_module *module, const struct lys_type *type)
{
    int i, content, close2;
    const char *str;
    struct lys_module *mod;

    /* decide whether the type will have any substatements */
    content = 0;
    switch (type->base) {
    case LY_TYPE_BINARY:
        if (type->info.binary.length) {
            content = 0x01;
        }
        break;
    case LY_TYPE_DEC64:
        if (type->info.dec64.dig || type->info.dec64.range) {
            content = 0x01;
        }
        break;
    case LY_TYPE_ENUM:
        if (type->info.enums.count) {
            content = 0x01;
        }
        break;
    case LY_TYPE_IDENT:
        if (type->info.ident.ref) {
            content = 0x01;
        }
        break;
    case LY_TYPE_BITS:
        if (type->info.bits.count) {
            content = 0x01;
        }
        break;
    case LY_TYPE_UNION:
        if (type->info.uni.count) {
            content = 0x01;
        }
        break;
    case LY_TYPE_LEAFREF:
        if (ly_strequal(type->der->name, "leafref", 0)) {
            content = 0x01;
        }
        break;
    case LY_TYPE_INST:
        if (type->info.inst.req) {
            content = 0x01;
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
            content = 0x01;
        }
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length || type->info.str.pat_count) {
            content = 0x01;
        }
        break;
    default:
        break;
    }

    if (type->ext_size) {
        content |= 0x02;
    }

    if (type->module_name) {
        ly_print(out, "%*s<type name=\"%s:%s\"%s>\n", LEVEL, INDENT,
                 transform_module_name2import_prefix(module, type->module_name), type->der->name, (content ? "" : "/"));
    } else {
        yin_print_open(out, level, "type", "name", type->der->name, content ? 0 : 1);
    }

    if (content & 0x01) {
        level++;
        switch (type->base) {
        case LY_TYPE_BINARY:
            if (type->info.binary.length) {
                yin_print_restr(out, level, "length", type->info.binary.length);
            }
            break;
        case LY_TYPE_BITS:
            for (i = 0; i < type->info.bits.count; ++i) {
                close2 = !yin_has_snode_common((struct lys_node *)&type->info.bits.bit[i])
                    && (type->info.bits.bit[i].flags & LYS_AUTOASSIGNED);

                yin_print_open(out, level, "bit", "name", type->info.bits.bit[i].name, close2);

                if (!close2) {
                    level++;
                    yin_print_snode_common(out, level, (struct lys_node *)&type->info.bits.bit[i]);
                    if (!(type->info.bits.bit[i].flags & LYS_AUTOASSIGNED)) {
                        yin_print_unsigned(out, level, "position", "value", type->info.bits.bit[i].pos);
                    }
                    level--;

                    yin_print_close(out, level, "bit");
                }
            }
            break;
        case LY_TYPE_DEC64:
            if (!type->der->type.der) {
                yin_print_unsigned(out, level, "fraction-digits", "value", type->info.dec64.dig);
            }
            if (type->info.dec64.range) {
                yin_print_restr(out, level, "range", type->info.dec64.range);
            }
            break;
        case LY_TYPE_ENUM:
            for (i = 0; i < type->info.enums.count; i++) {
                close2 = !yin_has_snode_common((struct lys_node *)&type->info.enums.enm[i])
                    && (type->info.enums.enm[i].flags & LYS_AUTOASSIGNED);

                yin_print_open(out, level, "enum", "name", type->info.enums.enm[i].name, close2);

                if (!close2) {
                    level++;
                    yin_print_snode_common(out, level, (struct lys_node *)&type->info.enums.enm[i]);
                    if (!(type->info.enums.enm[i].flags & LYS_AUTOASSIGNED)) {
                        ly_print(out, "%*s<value value=\"%d\"/>\n", LEVEL, INDENT, type->info.enums.enm[i].value);
                    }
                    level--;

                    yin_print_close(out, level, "enum");
                }
            }
            break;
        case LY_TYPE_IDENT:
            if (type->info.ident.count) {
                for (i = 0; i < type->info.ident.count; ++i) {
                    mod = lys_main_module(type->info.ident.ref[i]->module);
                    if (lys_main_module(module) == mod) {
                        ly_print(out, "%*s<base name=\"%s\"/>\n", LEVEL, INDENT, type->info.ident.ref[i]->name);
                    } else {
                        ly_print(out, "%*s<base name=\"%s:%s\"/>\n", LEVEL, INDENT,
                                transform_module_name2import_prefix(module, mod->name), type->info.ident.ref[i]->name);
                    }
                }
            }
            break;
        case LY_TYPE_INST:
            if (type->info.inst.req == 1) {
                yin_print_open(out, level, "require-instance", "value", "true", 1);
            } else if (type->info.inst.req == -1) {
                yin_print_open(out, level, "require-instance", "value", "false", 1);
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
                yin_print_restr(out, level, "range", type->info.num.range);
            }
            break;
        case LY_TYPE_LEAFREF:
            if (type->info.lref.path) {
                str = transform_json2schema(module, type->info.lref.path);
                yin_print_open(out, level, "path", "value", str, 1);
                lydict_remove(module->ctx, str);
            }
            break;
        case LY_TYPE_STRING:
            if (type->info.str.length) {
                yin_print_restr(out, level, "length", type->info.str.length);
            }
            for (i = 0; i < type->info.str.pat_count; i++) {
                yin_print_restr(out, level, "pattern", &type->info.str.patterns[i]);
            }
            break;
        case LY_TYPE_UNION:
            for (i = 0; i < type->info.uni.count; ++i) {
                yin_print_type(out, level, module, &type->info.uni.types[i]);
            }
            break;
        default:
            /* other types do not have substatements */
            break;
        }
        level--;
    }

    if (content & 0x02) {
        yin_print_extension_instances(out, level + 1, module, type->ext, type->ext_size);
    }

    if (content) {
        yin_print_close(out, level, "type");
    }
}

static void
yin_print_must(struct lyout *out, int level, const struct lys_module *module, const struct lys_restr *must)
{
    const char *str;
    int close;

    close = (must->dsc || must->ref || must->eapptag || must->emsg ? 0 : 1);

    str = transform_json2schema(module, must->expr);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*s<must condition=\"", LEVEL, INDENT);
    lyxml_dump_text(out, str);
    ly_print(out, "\"%s>\n", (close ? "/" : ""));

    lydict_remove(module->ctx, str);

    if (!close) {
        yin_print_restr_sub(out, level + 1, must);
        yin_print_close(out, level, "must");
    }
}

static void
yin_print_unique(struct lyout *out, int level, const struct lys_unique *uniq)
{
    int i;

    ly_print(out, "%*s<unique tag=\"", LEVEL, INDENT);
    for (i = 0; i < uniq->expr_size; i++) {
        ly_print(out, "%s%s", uniq->expr[i], i + 1 < uniq->expr_size ? " " : "");
    }
    ly_print(out, "\"/>\n");
}

static void
yin_print_refine(struct lyout *out, int level, const struct lys_module *module, const struct lys_refine *refine)
{
    int i;
    const char *str;

    str = transform_json2xml(module, refine->target_name, NULL, NULL, NULL);
    yin_print_open(out, level, "refine", "target-node", str, 0);
    lydict_remove(module->ctx, str);

    level++;
    if (refine->flags & LYS_CONFIG_W) {
        yin_print_open(out, level, "config", "value", "true", 1);
    } else if (refine->flags & LYS_CONFIG_R) {
        yin_print_open(out, level, "config", "value", "false", 1);
    }

    if (refine->flags & LYS_MAND_TRUE) {
        yin_print_open(out, level, "mandatory", "value", "true", 1);
    } else if (refine->flags & LYS_MAND_FALSE) {
        yin_print_open(out, level, "mandatory", "value", "false", 1);
    }

    yin_print_snode_common(out, level, (struct lys_node *)refine);

    for (i = 0; i < refine->must_size; ++i) {
        yin_print_must(out, level, module, &refine->must[i]);
    }
    for (i = 0; i < refine->iffeature_size; i++) {
        yin_print_iffeature(out, level, module, &refine->iffeature[i]);
    }
    for (i = 0; i < refine->dflt_size; ++i) {
        yin_print_open(out, level, "default", "value", refine->dflt[i], 1);
    }

    if (refine->target_type == LYS_CONTAINER) {
        if (refine->mod.presence) {
            yin_print_open(out, level, "presence", "value", refine->mod.presence, 1);
        }
    } else if (refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
        if (refine->flags & LYS_RFN_MINSET) {
            yin_print_unsigned(out, level, "min-elements", "value", refine->mod.list.min);
        }
        if (refine->flags & LYS_RFN_MAXSET) {
            if (refine->mod.list.max) {
                yin_print_unsigned(out, level, "max-elements", "value", refine->mod.list.max);
            } else {
                yin_print_open(out, level, "max-elements", "value", "unbounded", 1);
            }
        }
    }
    level--;

    yin_print_close(out, level, "refine");
}

static void
yin_print_deviation(struct lyout *out, int level, const struct lys_module *module,
                    const struct lys_deviation *deviation)
{
    int i, j;
    const char *str;

    str = transform_json2schema(module, deviation->target_name);
    yin_print_open(out, level, "deviation", "target-node", str, 0);
    lydict_remove(module->ctx, str);

    level++;
    if (deviation->dsc) {
        yin_print_text(out, level, "description", deviation->dsc);
    }
    if (deviation->ref) {
        yin_print_text(out, level, "reference", deviation->ref);
    }
    if (deviation->ext_size) {
        yin_print_extension_instances(out, level, module, deviation->ext, deviation->ext_size);
    }

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
        if (deviation->deviate[i].flags & LYS_CONFIG_W) {
            yin_print_open(out, level, "config", "value", "true", 1);
        } else if (deviation->deviate[i].flags & LYS_CONFIG_R) {
            yin_print_open(out, level, "config", "value", "false", 1);
        }

        if (deviation->deviate[i].flags & LYS_MAND_TRUE) {
            yin_print_open(out, level, "mandatory", "value", "true", 1);
        } else if (deviation->deviate[i].flags & LYS_MAND_FALSE) {
            yin_print_open(out, level, "mandatory", "value", "false", 1);
        }

        for (j = 0; j < deviation->deviate[i].dflt_size; j++) {
            yin_print_open(out, level, "default", "value", deviation->deviate[i].dflt[j], 1);
        }

        if (deviation->deviate[i].min_set) {
            yin_print_unsigned(out, level, "min-elements", "value", deviation->deviate[i].min);
        }
        if (deviation->deviate[i].max_set) {
            if (deviation->deviate[i].max) {
                yin_print_unsigned(out, level, "max-elements", "value", deviation->deviate[i].max);
            } else {
                yin_print_open(out, level, "max-elements", "value", "unbounded", 1);
            }
        }

        for (j = 0; j < deviation->deviate[i].must_size; ++j) {
            yin_print_must(out, level, module, &deviation->deviate[i].must[j]);
        }

        for (j = 0; j < deviation->deviate[i].unique_size; ++j) {
            yin_print_unique(out, level, &deviation->deviate[i].unique[j]);
        }

        if (deviation->deviate[i].type) {
            yin_print_type(out, level, module, deviation->deviate[i].type);
        }

        if (deviation->deviate[i].units) {
            yin_print_open(out, level, "units", "name", deviation->deviate[i].units, 1);
        }
        level--;

        yin_print_close(out, level, "deviate");
    }
    level--;

    yin_print_close(out, level, "deviation");
}

static void
yin_print_augment(struct lyout *out, int level, const struct lys_module *module,
                  const struct lys_node_augment *augment)
{
    int i;
    struct lys_node *sub;
    const char *str;

    str = transform_json2schema(module, augment->target_name);
    yin_print_open(out, level, "augment", "target-node", str, 0);
    lydict_remove(module->ctx, str);

    level++;
    yin_print_nacmext(out, level, (struct lys_node *)augment, module);
    yin_print_snode_common(out, level, (struct lys_node *)augment);

    for (i = 0; i < augment->iffeature_size; i++) {
        yin_print_iffeature(out, level, module, &augment->iffeature[i]);
    }

    if (augment->when) {
        yin_print_when(out, level, module, augment->when);
    }

    LY_TREE_FOR(augment->child, sub) {
        /* only our augment */
        if (sub->parent != (struct lys_node *)augment) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA | LYS_CASE | LYS_ACTION);
    }
    level--;

    yin_print_close(out, level, "augment");
}

static void
yin_print_typedef(struct lyout *out, int level, const struct lys_module *module, const struct lys_tpdf *tpdf)
{
    yin_print_open(out, level, "typedef", "name", tpdf->name, 0);
    const char *dflt;

    level++;
    yin_print_snode_common(out, level, (struct lys_node *)tpdf);
    yin_print_type(out, level, module, &tpdf->type);
    if (tpdf->units) {
        yin_print_open(out, level, "units", "name", tpdf->units, 1);
    }
    if (tpdf->dflt) {
        if (tpdf->flags & LYS_DFLTJSON) {
            dflt = transform_json2schema(module, tpdf->dflt);
        } else {
            dflt = tpdf->dflt;
        }
        yin_print_open(out, level, "default", "value", dflt, 1);
        if (tpdf->flags & LYS_DFLTJSON) {
            lydict_remove(module->ctx, dflt);
        }
    }
    level--;

    yin_print_close(out, level, "typedef");
}

static void
yin_print_identity(struct lyout *out, int level, const struct lys_ident *ident)
{
    int close, i;
    struct lys_module *mod;

    close = (yin_has_snode_common((struct lys_node *)ident) || ident->base ? 0 : 1);

    yin_print_open(out, level, "identity", "name", ident->name, close);

    if (!close) {
        level++;
        yin_print_snode_common(out, level, (struct lys_node *)ident);
        for (i = 0; i < ident->base_size; i++) {
            ly_print(out, "%*s<base name=\"", LEVEL, INDENT);
            mod = lys_main_module(ident->base[i]->module);
            if (lys_main_module(ident->module) != mod) {
                ly_print(out, "%s:", transform_module_name2import_prefix(ident->module, mod->name));
            }
            ly_print(out, "%s\"/>\n", ident->base[i]->name);
        }
        level--;

        yin_print_close(out, level, "identity");
    }
}

static void
yin_print_container(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_container *cont = (struct lys_node_container *)node;

    yin_print_open(out, level, "container", "name", node->name, 0);

    level++;
    yin_print_nacmext(out, level, node, node->module);

    if (cont->when) {
        yin_print_when(out, level, node->module, cont->when);
    }

    for (i = 0; i < cont->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &cont->iffeature[i]);
    }

    for (i = 0; i < cont->must_size; i++) {
        yin_print_must(out, level, node->module, &cont->must[i]);
    }

    if (cont->presence) {
        yin_print_open(out, level, "presence", "value", cont->presence, 1);
    }

    yin_print_snode_common2(out, level, node);

    for (i = 0; i < cont->tpdf_size; i++) {
        yin_print_typedef(out, level, node->module, &cont->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_GROUPING | LYS_ANYDATA | LYS_ACTION | LYS_NOTIF);
    }
    level--;

    yin_print_close(out, level, "container");
}

static void
yin_print_case(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_case *cas = (struct lys_node_case *)node;

    yin_print_open(out, level, "case", "name", cas->name, 0);

    level++;
    yin_print_nacmext(out, level, node, node->module);
    yin_print_snode_common2(out, level, node);

    for (i = 0; i < cas->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &cas->iffeature[i]);
    }

    if (cas->when) {
        yin_print_when(out, level, node->module, cas->when);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }
    level--;

    yin_print_close(out, level, "case");
}

static void
yin_print_choice(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    yin_print_open(out, level, "choice", "name", node->name, 0);

    level++;
    yin_print_nacmext(out, level, node, node->module);
    if (choice->dflt) {
        yin_print_open(out, level, "default", "value", choice->dflt->name, 1);
    }

    yin_print_snode_common2(out, level, node);

    for (i = 0; i < choice->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &choice->iffeature[i]);
    }

    if (choice->when) {
        yin_print_when(out, level, node->module, choice->when);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_CASE);
    }
    level--;

    yin_print_close(out, level, "choice");
}

static void
yin_print_leaf(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;
    const char *dflt;

    yin_print_open(out, level, "leaf", "name", node->name, 0);

    level++;
    yin_print_nacmext(out, level, node, node->module);
    if (leaf->when) {
        yin_print_when(out, level, node->module, leaf->when);
    }
    for (i = 0; i < leaf->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &leaf->iffeature[i]);
    }
    for (i = 0; i < leaf->must_size; i++) {
        yin_print_must(out, level, node->module, &leaf->must[i]);
    }
    yin_print_snode_common2(out, level, node);
    yin_print_type(out, level, node->module, &leaf->type);
    if (leaf->units) {
        yin_print_open(out, level, "units", "name", leaf->units, 1);
    }
    if (leaf->dflt) {
        if (leaf->flags & LYS_DFLTJSON) {
            dflt = transform_json2schema(node->module, leaf->dflt);
        } else {
            dflt = leaf->dflt;
        }
        yin_print_open(out, level, "default", "value", dflt, 1);
        if (leaf->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    level--;

    yin_print_close(out, level, "leaf");
}

static void
yin_print_anydata(struct lyout *out, int level, const struct lys_node *node)
{
    int i, close;
    struct lys_node_anydata *any = (struct lys_node_anydata *)node;
    const char *name;

    close = (yin_has_nacmext(node) || yin_has_snode_common2(node) || any->iffeature_size || any->must_size
            || any->when ? 0 : 1);
    name = any->nodetype == LYS_ANYXML ? "anyxml" : "anydata";
    yin_print_open(out, level, name, "name", any->name, close);

    if (!close) {
        level++;
        yin_print_nacmext(out, level, node, node->module);
        yin_print_snode_common2(out, level, node);
        for (i = 0; i < any->iffeature_size; i++) {
            yin_print_iffeature(out, level, node->module, &any->iffeature[i]);
        }
        for (i = 0; i < any->must_size; i++) {
            yin_print_must(out, level, node->module, &any->must[i]);
        }
        if (any->when) {
            yin_print_when(out, level, node->module, any->when);
        }
        level--;

        yin_print_close(out, level, name);
    }
}

static void
yin_print_leaflist(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;
    const char *dflt;

    yin_print_open(out, level, "leaf-list", "name", node->name, 0);

    level++;
    yin_print_nacmext(out, level, node, node->module);
    if (llist->when) {
        yin_print_when(out, level, llist->module, llist->when);
    }
    for (i = 0; i < llist->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &llist->iffeature[i]);
    }
    for (i = 0; i < llist->must_size; i++) {
        yin_print_must(out, level, node->module, &llist->must[i]);
    }
    yin_print_snode_common2(out, level, node);
    yin_print_type(out, level, node->module, &llist->type);
    if (llist->units) {
        yin_print_open(out, level, "units", "name", llist->units, 1);
    }
    for (i = 0; i < llist->dflt_size; i++) {
        if (llist->flags & LYS_DFLTJSON) {
            dflt = transform_json2schema(node->module, llist->dflt[i]);
        } else {
            dflt = llist->dflt[i];
        }
        yin_print_open(out, level, "default", "value", dflt, 1);
        if (llist->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    if (llist->min > 0) {
        yin_print_unsigned(out, level, "min-elements", "value", llist->min);
    }
    if (llist->max > 0) {
        yin_print_unsigned(out, level, "max-elements", "value", llist->max);
    }
    if (llist->flags & LYS_USERORDERED) {
        yin_print_open(out, level, "ordered-by", "value", "user", 1);
    }
    level--;

    yin_print_close(out, level, "leaf-list");
}

static void
yin_print_list(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;

    yin_print_open(out, level, "list", "name", node->name, 0);

    level++;
    yin_print_nacmext(out, level, node, node->module);
    if (list->when) {
        yin_print_when(out, level, list->module, list->when);
    }
    for (i = 0; i < list->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &list->iffeature[i]);
    }
    for (i = 0; i < list->must_size; i++) {
        yin_print_must(out, level, list->module, &list->must[i]);
    }
    if (list->keys_size) {
        ly_print(out, "%*s<key value=\"%s\"/>\n", LEVEL, INDENT, list->keys_str);
    }
    for (i = 0; i < list->unique_size; i++) {
        yin_print_unique(out, level, &list->unique[i]);
    }
    yin_print_snode_common2(out, level, node);
    if (list->min > 0) {
        yin_print_unsigned(out, level, "min-elements", "value", list->min);
    }
    if (list->max > 0) {
        yin_print_unsigned(out, level, "max-elements", "value", list->max);
    }
    if (list->flags & LYS_USERORDERED) {
        yin_print_open(out, level, "ordered-by", "value", "user", 1);
    }

    for (i = 0; i < list->tpdf_size; i++) {
        yin_print_typedef(out, level, list->module, &list->tpdf[i]);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_GROUPING | LYS_ANYDATA | LYS_ACTION | LYS_NOTIF);
    }
    level--;

    yin_print_close(out, level, "list");
}

static void
yin_print_grouping(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_grp *grp = (struct lys_node_grp *)node;

    yin_print_open(out, level, "grouping", "name", node->name, 0);

    level++;
    yin_print_snode_common(out, level, node);

    for (i = 0; i < grp->tpdf_size; i++) {
        yin_print_typedef(out, level, node->module, &grp->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_GROUPING | LYS_ANYDATA | LYS_ACTION);
    }
    level--;

    yin_print_close(out, level, "grouping");
}

static void
yin_print_uses(struct lyout *out, int level, const struct lys_node *node)
{
    int i, close;
    struct lys_node_uses *uses = (struct lys_node_uses *)node;
    struct lys_module *mod;

    close = (yin_has_nacmext(node) || yin_has_snode_common(node) || uses->iffeature_size || uses->when
            || uses->refine_size || uses->augment_size ? 0 : 1);

    ly_print(out, "%*s<uses name=\"", LEVEL, INDENT);
    if (node->child) {
        mod = lys_node_module(node->child);
        if (lys_node_module(node) != mod) {
            ly_print(out, "%s:", transform_module_name2import_prefix(node->module, mod->name));
        }
    }
    ly_print(out, "%s\"%s>\n", uses->name, (close ? "/" : ""));

    if (!close) {
        level++;
        yin_print_nacmext(out, level, node, node->module);
        yin_print_snode_common(out, level, node);
        for (i = 0; i < uses->iffeature_size; i++) {
            yin_print_iffeature(out, level, node->module, &uses->iffeature[i]);
        }
        if (uses->when) {
            yin_print_when(out, level, node->module, uses->when);
        }

        for (i = 0; i < uses->refine_size; i++) {
            yin_print_refine(out, level, node->module, &uses->refine[i]);
        }

        for (i = 0; i < uses->augment_size; i++) {
            yin_print_augment(out, level, node->module, &uses->augment[i]);
        }
        level--;

        yin_print_close(out, level, "uses");
    }
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

    ly_print(out, "%*s<%s>\n", LEVEL, INDENT, (inout->nodetype == LYS_INPUT ? "input" : "output"));

    level++;
    for (i = 0; i < inout->must_size; i++) {
        yin_print_must(out, level, node->module, &inout->must[i]);
    }
    if (inout->ext_size) {
        yin_print_extension_instances(out, level, node->module, inout->ext, inout->ext_size);
    }
    for (i = 0; i < inout->tpdf_size; i++) {
        yin_print_typedef(out, level, node->module, &inout->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_GROUPING | LYS_ANYDATA);
    }
    level--;

    yin_print_close(out, level, (inout->nodetype == LYS_INPUT ? "input" : "output"));
}

static void
yin_print_rpc_action(struct lyout *out, int level, const struct lys_node *node)
{
    int i, close;
    struct lys_node *sub;
    struct lys_node_rpc_action *rpc = (struct lys_node_rpc_action *)node;

    close = (yin_has_snode_common(node) || rpc->iffeature_size || rpc->tpdf_size || node->child ? 0 : 1);

    yin_print_open(out, level, (node->nodetype == LYS_RPC ? "rpc" : "action"), "name", node->name, close);

    if (!close) {
        level++;
        yin_print_snode_common(out, level, node);

        for (i = 0; i < rpc->iffeature_size; i++) {
            yin_print_iffeature(out, level, node->module, &rpc->iffeature[i]);
        }

        for (i = 0; i < rpc->tpdf_size; i++) {
            yin_print_typedef(out, level, node->module, &rpc->tpdf[i]);
        }

        LY_TREE_FOR(node->child, sub) {
            /* augments */
            if (sub->parent != node) {
                continue;
            }
            yin_print_snode(out, level, sub, LYS_GROUPING | LYS_INPUT | LYS_OUTPUT);
        }
        level--;

        yin_print_close(out, level, (node->nodetype == LYS_RPC ? "rpc" : "action"));
    }
}

static void
yin_print_notif(struct lyout *out, int level, const struct lys_node *node)
{
    int i, close;
    struct lys_node *sub;
    struct lys_node_notif *notif = (struct lys_node_notif *)node;

    close = (yin_has_snode_common(node) || notif->iffeature_size || notif->tpdf_size || node->child ? 0 : 1);

    yin_print_open(out, level, "notification", "name", node->name, close);

    if (!close) {
        level++;
        yin_print_snode_common(out, level, node);

        for (i = 0; i < notif->iffeature_size; i++) {
            yin_print_iffeature(out, level, node->module, &notif->iffeature[i]);
        }

        for (i = 0; i < notif->tpdf_size; i++) {
            yin_print_typedef(out, level, node->module, &notif->tpdf[i]);
        }

        for (i = 0; i < notif->must_size; i++) {
            yin_print_must(out, level, node->module, &notif->must[i]);
        }

        LY_TREE_FOR(node->child, sub) {
            /* augments */
            if (sub->parent != node) {
                continue;
            }
            yin_print_snode(out, level, sub,
                            LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                            LYS_USES | LYS_GROUPING | LYS_ANYDATA);
        }
        level--;

        yin_print_close(out, level, "notification");
    }
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
yin_print_namespaces(struct lyout *out, const struct lys_module *module)
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
    int level = 0, close;
#define LEVEL (level*2)

    struct lys_node *node;

    ly_print(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

    if (module->deviated == 1) {
        ly_print(out, "<!-- DEVIATED -->\n");
    }

    /* (sub)module-header-stmts */
    if (module->type) {
        ly_print(out, "<submodule name=\"%s\"\n", module->name);
        yin_print_namespaces(out, module);
        ly_print(out, ">\n");

        level++;
        if (module->version) {
            yin_print_open(out, level, "yang-version", "value",
                           ((struct lys_submodule *)module)->belongsto->version == 2 ? "1.1" : "1", 1);
        }
        yin_print_open(out, level, "belongs-to", "module", ((struct lys_submodule *)module)->belongsto->name, 0);

        level++;
        yin_print_open(out, level, "prefix", "value", module->prefix, 1);
        level--;

        yin_print_close(out, level, "belongs-to");
    } else {
        ly_print(out, "<module name=\"%s\"\n", module->name);
        yin_print_namespaces(out, module);
        ly_print(out, ">\n");

        level++;
        if (module->version) {
            yin_print_open(out, level, "yang-version", "value", module->version == 2 ? "1.1" : "1", 1);
        }
        yin_print_open(out, level, "namespace", "uri", module->ns, 1);
        yin_print_open(out, level, "prefix", "value", module->prefix, 1);
    }

    /* linkage-stmts */
    for (i = 0; i < module->imp_size; i++) {
        yin_print_open(out, level, "import", "module", module->imp[i].module->name, 0);

        level++;
        yin_print_open(out, level, "prefix", "value", module->imp[i].prefix, 1);
        if (module->imp[i].rev[0]) {
            yin_print_open(out, level, "revision-date", "date", module->imp[i].rev, 1);
        }
        if (module->imp[i].dsc) {
            yin_print_text(out, level, "description", module->imp[i].dsc);
        }
        if (module->imp[i].ref) {
            yin_print_text(out, level, "reference", module->imp[i].ref);
        }
        if (module->imp[i].ext_size) {
            yin_print_extension_instances(out, level, module, module->imp[i].ext, module->imp[i].ext_size);
        }
        level--;

        yin_print_close(out, level, "import");
    }
    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].external) {
            continue;
        }

        close = ((module->inc[i].rev[0] || module->inc[i].dsc || module->inc[i].ref || module->inc[i].ext_size) ? 0 : 1);
        yin_print_open(out, level, "include", "module", module->inc[i].submodule->name, close);

        if (!close) {
            level++;
            if (module->inc[i].rev[0]) {
                yin_print_open(out, level, "revision-date", "date", module->inc[i].rev, 1);
            }
            if (module->inc[i].dsc) {
                yin_print_text(out, level, "description", module->inc[i].dsc);
            }
            if (module->inc[i].ref) {
                yin_print_text(out, level, "reference", module->inc[i].ref);
            }
            if (module->inc[i].ext_size) {
                yin_print_extension_instances(out, level, module, module->inc[i].ext, module->inc[i].ext_size);
            }
            level--;

            yin_print_close(out, level, "include");
        }
    }

    /* meta-stmts */
    if (module->org) {
        yin_print_text(out, level, "organization", module->org);
    }
    if (module->contact) {
        yin_print_text(out, level, "contact", module->contact);
    }
    if (module->dsc) {
        yin_print_text(out, level, "description", module->dsc);
    }
    if (module->ref) {
        yin_print_text(out, level, "reference", module->ref);
    }

    /* revision-stmts */
    for (i = 0; i < module->rev_size; i++) {
        close = (module->rev[i].dsc || module->rev[i].ref ? 0 : 1);
        yin_print_open(out, level, "revision", "date", module->rev[i].date, close);

        if (!close) {
            level++;
            if (module->rev[i].dsc) {
                yin_print_text(out, level, "description", module->rev[i].dsc);
            }
            if (module->rev[i].ref) {
                yin_print_text(out, level, "reference", module->rev[i].ref);
            }
            level--;

            yin_print_close(out, level, "revision");
        }
    }

    /* body-stmts */
    for (i = 0; i < module->extensions_size; ++i) {
        yin_print_extension(out, level, &module->extensions[i]);
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

    if (module->ext_size) {
        yin_print_extension_instances(out, level, module, module->ext, module->ext_size);
    }

    for (i = 0; i < module->deviation_size; ++i) {
        yin_print_deviation(out, level, module, &module->deviation[i]);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }

        switch (node->nodetype) {
        case LYS_RPC:
            yin_print_rpc_action(out, level, node);
            break;
        case LYS_NOTIF:
            yin_print_notif(out, level, node);
            break;
        default:
            yin_print_snode(out, level, node,
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                             LYS_USES | LYS_GROUPING | LYS_ANYDATA);
            break;
        }
    }

    for (i = 0; i < module->augment_size; i++) {
        yin_print_augment(out, level, module, &module->augment[i]);
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

