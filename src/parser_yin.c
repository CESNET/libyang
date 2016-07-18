/**
 * @file yin.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YIN parser for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "dict_private.h"
#include "xpath.h"
#include "parser.h"
#include "resolve.h"
#include "tree_internal.h"
#include "xml_internal.h"

#define GETVAL(value, node, arg)                                                 \
    value = lyxml_get_attr(node, arg, NULL);                                     \
    if (!value) {                                                                \
        LOGVAL(LYE_MISSARG, LY_VLOG_NONE, NULL, arg, node->name);  \
        goto error;                                                              \
    }

/* parser.c */
int dup_prefix_check(const char *prefix, struct lys_module *module);

#define OPT_IDENT   0x01
#define OPT_CONFIG  0x02
#define OPT_MODULE  0x04
#define OPT_INHERIT 0x08
#define OPT_NACMEXT 0x10
static int read_yin_common(struct lys_module *, struct lys_node *, struct lys_node *, struct lyxml_elem *, int);

static struct lys_node *read_yin_choice(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                        int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_case(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                      int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_anyxml(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                        int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_container(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                           int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_leaf(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                      int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_leaflist(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                          int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_list(struct lys_module *module,struct lys_node *parent, struct lyxml_elem *yin,
                                      int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_uses(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                      struct unres_schema *unres);
static struct lys_node *read_yin_grouping(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                          int resolve, struct unres_schema *unres);
static struct lys_when *read_yin_when(struct lys_module *module, struct lyxml_elem *yin);

/* logs directly */
static const char *
read_yin_subnode(struct ly_ctx *ctx, struct lyxml_elem *node, const char *name)
{
    int len;

    /* there should be <text> child */
    if (!node->child || !node->child->name || strcmp(node->child->name, name)) {
        LOGERR(LY_EVALID, "Expected \"%s\" element in \"%s\" element.", name, node->name);
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, name, node->name);
        return NULL;
    } else if (node->child->content) {
        len = strlen(node->child->content);
        return lydict_insert(ctx, node->child->content, len);
    } else {
        return lydict_insert(ctx, "", 0);
    }
}

/* logs directly */
static int
fill_yin_iffeature(struct lys_node *parent, struct lyxml_elem *yin, struct lys_feature **iffeat, struct unres_schema *unres)
{
    int r;
    const char *value;

    GETVAL(value, yin, "name");
    if (!(value = transform_schema2json(parent->module, value))) {
        return EXIT_FAILURE;
    }

    /* HACK - store pointer to the parent node for later status check */
    *iffeat = (struct lys_feature *)parent;
    r = unres_schema_add_str(parent->module, unres, iffeat, UNRES_IFFEAT, value);
    lydict_remove(parent->module->ctx, value);
    if (!r) {
        return EXIT_SUCCESS;
    }

error:
    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_identity(struct lys_module *module, struct lyxml_elem *yin, struct lys_ident *ident, struct unres_schema *unres)
{
    struct lyxml_elem *node;
    const char *value;
    int base_flag = 0;

    GETVAL(value, yin, "name");
    ident->name = value;

    if (read_yin_common(module, NULL, (struct lys_node *)ident, yin, OPT_IDENT | OPT_MODULE)) {
        return EXIT_FAILURE;
    }

    if (dup_identities_check(ident->name, module)) {
        return EXIT_FAILURE;
    }

    LY_TREE_FOR(yin->child, node) {
        if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(node->name, "base")) {
            if (base_flag) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                return EXIT_FAILURE;
            }
            base_flag = 1;

            GETVAL(value, node, "name");
            value = transform_schema2json(module, value);
            if (!value) {
                return EXIT_FAILURE;
            }

            if (unres_schema_add_str(module, unres, ident, UNRES_IDENT, value) == -1) {
                lydict_remove(module->ctx, value);
                return EXIT_FAILURE;
            }
            lydict_remove(module->ctx, value);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name, "identity");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

/* logs directly */
static int
read_restr_substmt(struct ly_ctx *ctx, struct lys_restr *restr, struct lyxml_elem *yin)
{
    struct lyxml_elem *child;
    const char *value;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (restr->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->dsc = read_yin_subnode(ctx, child, "text");
            if (!restr->dsc) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (restr->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->ref = read_yin_subnode(ctx, child, "text");
            if (!restr->ref) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "error-app-tag")) {
            if (restr->eapptag) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            GETVAL(value, child, "value");
            restr->eapptag = lydict_insert(ctx, value, 0);
        } else if (!strcmp(child->name, "error-message")) {
            if (restr->emsg) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->emsg = read_yin_subnode(ctx, child, "value");
            if (!restr->emsg) {
                return EXIT_FAILURE;
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

/* logs directly, returns EXIT_SUCCESS, EXIT_FAILURE, -1 */
int
fill_yin_type(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_type *type,
              int tpdftype, struct unres_schema *unres)
{
    const char *value, *name;
    struct lys_node *siter;
    struct lyxml_elem *next, *node;
    struct lys_restr **restr;
    struct lys_type_bit bit;
    int i, j, rc, val_set;
    int ret = -1;
    int64_t v, v_;
    int64_t p, p_;

    GETVAL(value, yin, "name");
    value = transform_schema2json(module, value);
    if (!value) {
        goto error;
    }

    i = parse_identifier(value);
    if (i < 1) {
        LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, value[-i], &value[-i]);
        lydict_remove(module->ctx, value);
        goto error;
    }
    /* module name */
    name = value;
    if (value[i]) {
        type->module_name = lydict_insert(module->ctx, value, i);
        name += i;
        if ((name[0] != ':') || (parse_identifier(name + 1) < 1)) {
            LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, name[0], name);
            lydict_remove(module->ctx, value);
            goto error;
        }
        /* name is in dictionary, but moved */
        ++name;
    }

    rc = resolve_superior_type(name, type->module_name, module, parent, &type->der);
    if (rc == -1) {
        LOGVAL(LYE_INMOD, LY_VLOG_NONE, NULL, type->module_name);
        lydict_remove(module->ctx, value);
        goto error;

    /* the type could not be resolved or it was resolved to an unresolved typedef */
    } else if (rc == EXIT_FAILURE) {
        LOGVAL(LYE_NORESOLV, LY_VLOG_NONE, NULL, "type", name);
        lydict_remove(module->ctx, value);
        ret = EXIT_FAILURE;
        goto error;
    }
    lydict_remove(module->ctx, value);
    type->base = type->der->type.base;

    /* check status */
    if (lyp_check_status(type->parent->flags, type->parent->module, type->parent->name,
                     type->der->flags, type->der->module, type->der->name,  parent)) {
        return -1;
    }

    switch (type->base) {
    case LY_TYPE_BITS:
        /* RFC 6020 9.7.4 - bit */

        /* get bit specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "bit")) {
                type->info.bits.count++;
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }
        if (!type->der->type.der && !type->info.bits.count) {
            /* type is derived directly from buit-in bits type and bit statement is required */
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "bit", "type");
            goto error;
        }
        if (type->der->type.der && type->info.bits.count) {
            /* type is not directly derived from buit-in bits type and bit statement is prohibited */
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "bit");
            goto error;
        }

        type->info.bits.bit = calloc(type->info.bits.count, sizeof *type->info.bits.bit);
        if (!type->info.bits.bit) {
            LOGMEM;
            goto error;
        }
        p = 0;
        i = -1;
        LY_TREE_FOR(yin->child, next) {
            i++;

            GETVAL(value, next, "name");
            if (lyp_check_identifier(value, LY_IDENT_SIMPLE, NULL, NULL)) {
                goto error;
            }

            type->info.bits.bit[i].name = lydict_insert(module->ctx, value, strlen(value));
            if (read_yin_common(module, NULL, (struct lys_node *)&type->info.bits.bit[i], next, 0)) {
                type->info.bits.count = i + 1;
                goto error;
            }

            /* check the name uniqueness */
            for (j = 0; j < i; j++) {
                if (!strcmp(type->info.bits.bit[j].name, type->info.bits.bit[i].name)) {
                    LOGVAL(LYE_BITS_DUPNAME, LY_VLOG_NONE, NULL, type->info.bits.bit[i].name);
                    type->info.bits.count = i + 1;
                    goto error;
                }
            }

            p_ = -1;
            LY_TREE_FOR(next->child, node) {
                if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                    /* garbage */
                    continue;
                }

                if (!strcmp(node->name, "position")) {
                    GETVAL(value, node, "value");
                    p_ = strtoll(value, NULL, 10);

                    /* range check */
                    if (p_ < 0 || p_ > UINT32_MAX) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "bit/position");
                        type->info.bits.count = i + 1;
                        goto error;
                    }
                    type->info.bits.bit[i].pos = (uint32_t)p_;

                    /* keep the highest enum value for automatic increment */
                    if (type->info.bits.bit[i].pos >= p) {
                        p = type->info.bits.bit[i].pos;
                        p++;
                    } else {
                        /* check that the value is unique */
                        for (j = 0; j < i; j++) {
                            if (type->info.bits.bit[j].pos == type->info.bits.bit[i].pos) {
                                LOGVAL(LYE_BITS_DUPVAL, LY_VLOG_NONE, NULL,
                                       type->info.bits.bit[i].pos, type->info.bits.bit[i].name);
                                type->info.bits.count = i + 1;
                                goto error;
                            }
                        }
                    }
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                    goto error;
                }
            }
            if (p_ == -1) {
                /* assign value automatically */
                if (p > UINT32_MAX) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "4294967295", "bit/position");
                    type->info.bits.count = i + 1;
                    goto error;
                }
                type->info.bits.bit[i].pos = (uint32_t)p;
                type->info.bits.bit[i].flags |= LYS_AUTOASSIGNED;
                p++;
            }

            /* keep them ordered by position */
            j = i;
            while (j && type->info.bits.bit[j - 1].pos > type->info.bits.bit[j].pos) {
                /* switch them */
                memcpy(&bit, &type->info.bits.bit[j], sizeof bit);
                memcpy(&type->info.bits.bit[j], &type->info.bits.bit[j - 1], sizeof bit);
                memcpy(&type->info.bits.bit[j - 1], &bit, sizeof bit);
                j--;
            }
        }
        break;

    case LY_TYPE_DEC64:
        /* RFC 6020 9.2.4 - range and 9.3.4 - fraction-digits */
        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, "range")) {
                if (type->info.dec64.range) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (lyp_check_length_range(value, type)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "range");
                    goto error;
                }
                type->info.dec64.range = calloc(1, sizeof *type->info.dec64.range);
                if (!type->info.dec64.range) {
                    LOGMEM;
                    goto error;
                }
                type->info.dec64.range->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module->ctx, type->info.dec64.range, node)) {
                    goto error;
                }
            } else if (!strcmp(node->name, "fraction-digits")) {
                if (type->info.dec64.dig) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }
                GETVAL(value, node, "value");
                v = strtol(value, NULL, 10);

                /* range check */
                if (v < 1 || v > 18) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, node->name);
                    goto error;
                }
                type->info.dec64.dig = (uint8_t)v;
                type->info.dec64.div = 10;
                for (i = 1; i < v; i++) {
                    type->info.dec64.div *= 10;
                }
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }

        /* mandatory sub-statement(s) check */
        if (!type->info.dec64.dig && !type->der->type.der) {
            /* decimal64 type directly derived from built-in type requires fraction-digits */
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "fraction-digits", "type");
            goto error;
        }
        if (type->info.dec64.dig && type->der->type.der) {
            /* type is not directly derived from buit-in type and fraction-digits statement is prohibited */
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "fraction-digits");
            goto error;
        }
        break;

    case LY_TYPE_ENUM:
        /* RFC 6020 9.6 - enum */

        /* get enum specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "enum")) {
                type->info.enums.count++;
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }
        if (!type->der->type.der && !type->info.enums.count) {
            /* type is derived directly from buit-in enumeartion type and enum statement is required */
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "enum", "type");
            goto error;
        }
        if (type->der->type.der && type->info.enums.count) {
            /* type is not directly derived from buit-in enumeration type and enum statement is prohibited */
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "enum");
            goto error;
        }

        type->info.enums.enm = calloc(type->info.enums.count, sizeof *type->info.enums.enm);
        if (!type->info.enums.enm) {
            LOGMEM;
            goto error;
        }

        val_set = v = 0;
        i = -1;
        LY_TREE_FOR(yin->child, next) {
            i++;

            GETVAL(value, next, "name");
            if (!value[0]) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "enum name");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Enum name must not be empty.");
                goto error;
            }
            type->info.enums.enm[i].name = lydict_insert(module->ctx, value, strlen(value));
            if (read_yin_common(module, NULL, (struct lys_node *)&type->info.enums.enm[i], next, 0)) {
                type->info.enums.count = i + 1;
                goto error;
            }

            /* the assigned name MUST NOT have any leading or trailing whitespace characters */
            value = type->info.enums.enm[i].name;
            if (isspace(value[0]) || isspace(value[strlen(value) - 1])) {
                LOGVAL(LYE_ENUM_WS, LY_VLOG_NONE, NULL, value);
                type->info.enums.count = i + 1;
                goto error;
            }

            /* check the name uniqueness */
            for (j = 0; j < i; j++) {
                if (!strcmp(type->info.enums.enm[j].name, type->info.enums.enm[i].name)) {
                    LOGVAL(LYE_ENUM_DUPNAME, LY_VLOG_NONE, NULL, type->info.enums.enm[i].name);
                    type->info.enums.count = i + 1;
                    goto error;
                }
            }

            LY_TREE_FOR(next->child, node) {
                if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                    /* garbage */
                    continue;
                }

                if (!strcmp(node->name, "value")) {
                    GETVAL(value, node, "value");
                    v_ = strtoll(value, NULL, 10);

                    /* range check */
                    if (v_ < INT32_MIN || v_ > INT32_MAX) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "enum/value");
                        type->info.enums.count = i + 1;
                        goto error;
                    }
                    type->info.enums.enm[i].value = v_;

                    /* keep the highest enum value for automatic increment */
                    if (!val_set || type->info.enums.enm[i].value > v) {
                        v = type->info.enums.enm[i].value;
                        v++;
                    } else {
                        /* check that the value is unique */
                        for (j = 0; j < i; j++) {
                            if (type->info.enums.enm[j].value == type->info.enums.enm[i].value) {
                                LOGVAL(LYE_ENUM_DUPVAL, LY_VLOG_NONE, NULL,
                                       type->info.enums.enm[i].value, type->info.enums.enm[i].name);
                                type->info.enums.count = i + 1;
                                goto error;
                            }
                        }
                    }
                    val_set = 1;
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                    goto error;
                }
            }
            if (!val_set) {
                /* assign value automatically */
                if (v > INT32_MAX) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "2147483648", "enum/value");
                    type->info.enums.count = i + 1;
                    goto error;
                }
                type->info.enums.enm[i].value = v;
                type->info.enums.enm[i].flags |= LYS_AUTOASSIGNED;
                v++;
            }
        }
        break;

    case LY_TYPE_IDENT:
        /* RFC 6020 9.10 - base */

        /* get base specification, exactly one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free(module->ctx, node);
                continue;
            }

            if (strcmp(node->name, "base")) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }

        if (!yin->child) {
            if (type->der->type.der) {
                /* this is just a derived type with no base specified/required */
                break;
            }
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "base", "type");
            goto error;
        }
        if (yin->child->next) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, yin->child->next->name, yin->name);
            goto error;
        }
        GETVAL(value, yin->child, "name");
        /* store in the JSON format */
        value = transform_schema2json(module, value);
        if (!value) {
            goto error;
        }
        rc = unres_schema_add_str(module, unres, type, UNRES_TYPE_IDENTREF, value);
        lydict_remove(module->ctx, value);

        if (rc == -1) {
            goto error;
        }
        break;

    case LY_TYPE_INST:
        /* RFC 6020 9.13.2 - require-instance */
        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, "require-instance")) {
                if (type->info.inst.req) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }
                GETVAL(value, node, "value");
                if (!strcmp(value, "true")) {
                    type->info.inst.req = 1;
                } else if (!strcmp(value, "false")) {
                    type->info.inst.req = -1;
                } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, node->name);
                    goto error;
                }
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }

        break;

    case LY_TYPE_BINARY:
        /* RFC 6020 9.8.1, 9.4.4 - length, number of octets it contains */
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        /* RFC 6020 9.2.4 - range */

        /* length and range are actually the same restriction, so process
         * them by this common code, we just need to differ the name and
         * structure where the information will be stored
         */
        if (type->base == LY_TYPE_BINARY) {
            restr = &type->info.binary.length;
            name = "length";
        } else {
            restr = &type->info.num.range;
            name = "range";
        }

        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, name)) {
                if (*restr) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (lyp_check_length_range(value, type)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, name);
                    goto error;
                }
                *restr = calloc(1, sizeof **restr);
                if (!(*restr)) {
                    LOGMEM;
                    goto error;
                }
                (*restr)->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module->ctx, *restr, node)) {
                    goto error;
                }
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }
        break;

    case LY_TYPE_LEAFREF:
        /* flag resolving for later use */
        if (!tpdftype) {
            for (siter = parent; siter && siter->nodetype != LYS_GROUPING; siter = lys_parent(siter));
            if (siter) {
                /* just a flag - do not resolve */
                tpdftype = 1;
            }
        }

        /* RFC 6020 9.9.2 - path */
        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, "path") && !type->der->type.der) {
                if (type->info.lref.path) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                /* store in the JSON format */
                type->info.lref.path = transform_schema2json(module, value);
                if (!type->info.lref.path) {
                    goto error;
                }

                /* try to resolve leafref path only when this is instantiated
                 * leaf, so it is not:
                 * - typedef's type,
                 * - in  grouping definition,
                 * - just instantiated in a grouping definition,
                 * because in those cases the nodes referenced in path might not be present
                 * and it is not a bug.  */
                if (!tpdftype && unres_schema_add_node(module, unres, type, UNRES_TYPE_LEAFREF, parent) == -1) {
                    goto error;
                }

            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }

        if (!type->info.lref.path) {
            if (!type->der->type.der) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "path", "type");
                goto error;
            } else {
                /* copy leafref definition into the derived type */
                type->info.lref.path = lydict_insert(module->ctx, type->der->type.info.lref.path, 0);
                /* and resolve the path at the place we are (if not in grouping/typedef) */
                if (!tpdftype && unres_schema_add_node(module, unres, type, UNRES_TYPE_LEAFREF, parent) == -1) {
                    goto error;
                }

                /* add pointer to leafref target, only on leaves (not in typedefs) */
                if (type->info.lref.target && lys_leaf_add_leafref_target(type->info.lref.target, (struct lys_node *)type->parent)) {
                    goto error;
                }
            }
        }

        break;

    case LY_TYPE_STRING:
        /* RFC 6020 9.4.4 - length */
        /* RFC 6020 9.4.6 - pattern */
        i = 0;
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "length")) {
                if (type->info.str.length) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (lyp_check_length_range(value, type)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "length");
                    goto error;
                }
                type->info.str.length = calloc(1, sizeof *type->info.str.length);
                if (!type->info.str.length) {
                    LOGMEM;
                    goto error;
                }
                type->info.str.length->expr = lydict_insert(module->ctx, value, 0);

                /* get possible sub-statements */
                if (read_restr_substmt(module->ctx, type->info.str.length, node)) {
                    goto error;
                }
                lyxml_free(module->ctx, node);
            } else if (!strcmp(node->name, "pattern")) {
                i++;
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }
        /* store patterns in array */
        if (i) {
            type->info.str.patterns = calloc(i, sizeof *type->info.str.patterns);
            if (!type->info.str.patterns) {
                LOGMEM;
                goto error;
            }
            LY_TREE_FOR(yin->child, node) {
                GETVAL(value, node, "value");

                if (lyp_check_pattern(value, NULL)) {
                    free(type->info.str.patterns);
                    type->info.str.patterns = NULL;
                    goto error;
                }

                type->info.str.patterns[type->info.str.pat_count].expr = lydict_insert(module->ctx, value, 0);

                /* get possible sub-statements */
                if (read_restr_substmt(module->ctx, &type->info.str.patterns[type->info.str.pat_count], node)) {
                    free(type->info.str.patterns);
                    goto error;
                }
                type->info.str.pat_count++;
            }
        }
        break;

    case LY_TYPE_UNION:
        /* RFC 6020 7.4 - type */
        /* count number of types in union */
        i = 0;
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "type")) {
                i++;
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }

        if (!i) {
            if (type->der->type.der) {
                /* this is just a derived type with no additional type specified/required */
                break;
            }
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "(union) type");
            goto error;
        }

        /* allocate array for union's types ... */
        type->info.uni.types = calloc(i, sizeof *type->info.uni.types);
        if (!type->info.uni.types) {
            LOGMEM;
            goto error;
        }
        /* ... and fill the structures */
        LY_TREE_FOR(yin->child, node) {
            type->info.uni.types[type->info.uni.count].parent = type->parent;
            rc = fill_yin_type(module, parent, node, &type->info.uni.types[type->info.uni.count], tpdftype, unres);
            if (!rc) {
                type->info.uni.count++;

                /* union's type cannot be empty or leafref */
                if (type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_EMPTY) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "empty", node->name);
                    rc = -1;
                } else if (type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_LEAFREF) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "leafref", node->name);
                    rc = -1;
                }
            }
            if (rc) {
                /* even if we got EXIT_FAILURE, throw it all away, too much trouble doing something else */
                for (i = 0; i < type->info.uni.count; ++i) {
                    lys_type_free(module->ctx, &type->info.uni.types[i]);
                }
                free(type->info.uni.types);
                type->info.uni.types = NULL;
                type->info.uni.count = 0;

                if (rc == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                    goto error;
                }
                goto error;
            }
        }
        break;

    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        /* no sub-statement allowed */
        LY_TREE_FOR(yin->child, node) {
            if (node->ns && !strcmp(node->ns->value, LY_NSYIN)) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }
        break;

    default:
        LOGINT;
        goto error;
    }

    return EXIT_SUCCESS;

error:
    if (type->module_name) {
        lydict_remove(module->ctx, type->module_name);
        type->module_name = NULL;
    }
    return ret;
}

/* logs directly */
static int
fill_yin_typedef(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_tpdf *tpdf, struct unres_schema *unres)
{
    const char *value;
    struct lyxml_elem *node, *next;
    int has_type = 0;

    GETVAL(value, yin, "name");
    if (lyp_check_identifier(value, LY_IDENT_TYPE, module, parent)) {
        goto error;
    }
    tpdf->name = lydict_insert(module->ctx, value, strlen(value));

    /* generic part - status, description, reference */
    if (read_yin_common(module, NULL, (struct lys_node *)tpdf, yin, OPT_MODULE)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, node) {
        if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(node->name, "type")) {
            if (has_type) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                goto error;
            }
            /* HACK for unres */
            tpdf->type.der = (struct lys_tpdf *)node;
            tpdf->type.parent = tpdf;
            if (unres_schema_add_node(module, unres, &tpdf->type, UNRES_TYPE_DER_TPDF, parent)) {
                goto error;
            }
            has_type = 1;
        } else if (!strcmp(node->name, "default")) {
            if (tpdf->dflt) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "value");
            tpdf->dflt = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(node->name, "units")) {
            if (tpdf->units) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "name");
            tpdf->units = lydict_insert(module->ctx, value, strlen(value));
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, value);
            goto error;
        }
    }

    /* check mandatory value */
    if (!has_type) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", yin->name);
        goto error;
    }

    /* check default value */
    if (tpdf->dflt) {
        if (unres_schema_add_str(module, unres, &tpdf->type, UNRES_TYPE_DFLT, tpdf->dflt) == -1) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_feature(struct lys_module *module, struct lyxml_elem *yin, struct lys_feature *f, struct unres_schema *unres)
{
    const char *value;
    struct lyxml_elem *child, *next;
    int c = 0, ret;

    GETVAL(value, yin, "name");
    if (lyp_check_identifier(value, LY_IDENT_FEATURE, module, NULL)) {
        goto error;
    }
    f->name = lydict_insert(module->ctx, value, strlen(value));
    f->module = module;

    if (read_yin_common(module, NULL, (struct lys_node *)f, yin, 0)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "if-feature")) {
            c++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    if (c) {
        f->features = calloc(c, sizeof *f->features);
        if (!f->features) {
            LOGMEM;
            goto error;
        }
    }
    LY_TREE_FOR(yin->child, child) {
        ret = fill_yin_iffeature((struct lys_node *)f, child, &f->features[f->features_size], unres);
        f->features_size++;
        if (ret) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_must(struct lys_module *module, struct lyxml_elem *yin, struct lys_restr *must)
{
    const char *value;

    GETVAL(value, yin, "condition");
    must->expr = transform_schema2json(module, value);
    if (!must->expr) {
        goto error;
    }
    if (lyxp_syntax_check(must->expr)) {
        goto error;
    }

    return read_restr_substmt(module->ctx, must, yin);

error:
    return EXIT_FAILURE;
}

static int
fill_yin_unique(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_unique *unique,
                struct unres_schema *unres)
{
    int i, j;
    const char *value, *vaux;

    /* get unique value (list of leafs supposed to be unique */
    GETVAL(value, yin, "tag");

    /* count the number of unique leafs in the value */
    vaux = value;
    while ((vaux = strpbrk(vaux, " \t\n"))) {
        unique->expr_size++;
        while (isspace(*vaux)) {
            vaux++;
        }
    }
    unique->expr_size++;
    unique->expr = calloc(unique->expr_size, sizeof *unique->expr);
    if (!unique->expr) {
        LOGMEM;
        goto error;
    }

    for (i = 0; i < unique->expr_size; i++) {
        vaux = strpbrk(value, " \t\n");
        if (!vaux) {
            /* the last token, lydict_insert() will count its size on its own */
            vaux = value;
        }

        /* store token into unique structure */
        unique->expr[i] = lydict_insert(module->ctx, value, vaux - value);

        /* check that the expression does not repeat */
        for (j = 0; j < i; j++) {
            if (ly_strequal(unique->expr[j], unique->expr[i], 1)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, unique->expr[i], "unique");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The identifier is not unique");
                goto error;
            }
        }

        /* try to resolve leaf */
        if (unres) {
            if (unres_schema_add_str(module, unres, parent, UNRES_LIST_UNIQ, unique->expr[i]) == -1){
                goto error;
            }
        } else {
            if (resolve_unique(parent, unique->expr[i])) {
                goto error;
            }
        }

        /* move to next token */
        value = vaux;
        while(isspace(*value)) {
            value++;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

/* logs directly
 *
 * type: 0 - min, 1 - max
 */
static int
deviate_minmax(struct lys_node *target, struct lyxml_elem *node, struct lys_deviate *d, int type)
{
    const char *value;
    char *endptr;
    unsigned long val;
    uint32_t *ui32val;

    /* del min/max is forbidden */
    if (d->mod == LY_DEVIATE_DEL) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, (type ? "max-elements" : "min-elements"), "deviate delete");
        goto error;
    }

    /* check target node type */
    if (target->nodetype == LYS_LEAFLIST) {
        if (type) {
            ui32val = &((struct lys_node_leaflist *)target)->max;
        } else {
            ui32val = &((struct lys_node_leaflist *)target)->min;
        }
    } else if (target->nodetype == LYS_LIST) {
        if (type) {
            ui32val = &((struct lys_node_list *)target)->max;
        } else {
            ui32val = &((struct lys_node_list *)target)->min;
        }
    } else {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", node->name);
        goto error;
    }

    GETVAL(value, node, "value");
    while (isspace(value[0])) {
        value++;
    }

    if (type && !strcmp(value, "unbounded")) {
        d->max = val = 0;
        d->max_set = 1;
    } else {
        /* convert it to uint32_t */
        errno = 0;
        endptr = NULL;
        val = strtoul(value, &endptr, 10);
        if (*endptr || value[0] == '-' || errno || val > UINT32_MAX) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, node->name);
            goto error;
        }
        if (type) {
            d->max = (uint32_t)val;
            d->max_set = 1;
        } else {
            d->min = (uint32_t)val;
            d->min_set = 1;
        }
    }

    if (d->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (*ui32val) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
            goto error;
        }
    } else if (d->mod == LY_DEVIATE_RPL) {
        /* unfortunately, there is no way to check reliably that there
         * was a value before, it could have been the default */
    }

    /* add (already checked) and replace */
    /* set new value specified in deviation */
    *ui32val = (uint32_t)val;

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_deviation(struct lys_module *module, struct lyxml_elem *yin, struct lys_deviation *dev,
                   struct unres_schema *unres)
{
    const char *value, **stritem;
    struct lyxml_elem *next, *child, *develem;
    int c_dev = 0, c_must, c_uniq;
    int f_min = 0, f_max = 0; /* flags */
    int i, j, rc;
    struct ly_ctx *ctx;
    struct lys_deviate *d = NULL;
    struct lys_node *node = NULL, *dev_target = NULL;
    struct lys_node_choice *choice = NULL;
    struct lys_node_leaf *leaf = NULL, **leaf_dflt_check = NULL;
    struct lys_node_list *list = NULL;
    struct lys_type *t = NULL;
    uint8_t *trg_must_size = NULL, leaf_dflt_check_count = 0;
    struct lys_restr **trg_must = NULL;
    struct unres_schema tmp_unres;

    ctx = module->ctx;

    GETVAL(value, yin, "target-node");
    dev->target_name = transform_schema2json(module, value);
    if (!dev->target_name) {
        goto error;
    }

    /* resolve target node */
    rc = resolve_augment_schema_nodeid(dev->target_name, NULL, module, (const struct lys_node **)&dev_target);
    if (rc || !dev_target) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, yin->name);
        goto error;
    }
    if (dev_target->module == lys_main_module(module)) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, yin->name);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Deviating own module is not allowed.");
        goto error;
    }
    lys_deviation_add_ext_imports(lys_node_module(dev_target), module);

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(ctx, child);
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (dev->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            dev->dsc = read_yin_subnode(ctx, child, "text");
            if (!dev->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (dev->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            dev->ref = read_yin_subnode(ctx, child, "text");
            if (!dev->ref) {
                goto error;
            }
        } else if (!strcmp(child->name, "deviate")) {
            c_dev++;

            /* skip lyxml_free() at the end of the loop, node will be
             * further processed later
             */
            continue;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }

        lyxml_free(ctx, child);
    }

    if (c_dev) {
        dev->deviate = calloc(c_dev, sizeof *dev->deviate);
        if (!dev->deviate) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, develem) {
        /* init */
        f_min = 0;
        f_max = 0;
        c_must = 0;
        c_uniq = 0;

        /* get deviation type */
        GETVAL(value, develem, "value");
        if (!strcmp(value, "not-supported")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_NO;
            /* no property expected in this case */
            if (develem->child) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, develem->child->name);
                goto error;
            }

            /* and neither any other deviate statement is expected,
             * not-supported deviation must be the only deviation of the target
             */
            if (dev->deviate_size || develem->next) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, develem->name);
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot be combined with any other deviation.");
                goto error;
            }

            /* you cannot remove a key leaf */
            if ((dev_target->nodetype == LYS_LEAF) && lys_parent(dev_target) && (lys_parent(dev_target)->nodetype == LYS_LIST)) {
                for (i = 0; i < ((struct lys_node_list *)lys_parent(dev_target))->keys_size; ++i) {
                    if (((struct lys_node_list *)lys_parent(dev_target))->keys[i] == (struct lys_node_leaf *)dev_target) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, develem->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot remove a list key.");
                        goto error;
                    }
                }
            }

            /* unlink and store the original node */
            lys_node_unlink(dev_target);
            dev->orig_node = dev_target;

            dev->deviate_size = 1;
            return EXIT_SUCCESS;
        } else if (!strcmp(value, "add")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_ADD;
        } else if (!strcmp(value, "replace")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_RPL;
        } else if (!strcmp(value, "delete")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_DEL;
        } else {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, develem->name);
            goto error;
        }
        d = &dev->deviate[dev->deviate_size];
        dev->deviate_size++;

        /* store a shallow copy of the original node */
        if (!dev->orig_node) {
            memset(&tmp_unres, 0, sizeof tmp_unres);
            dev->orig_node = lys_node_dup(dev_target->module, NULL, dev_target, 0, 0, &tmp_unres, 1);
            /* just to be safe */
            if (tmp_unres.count) {
                LOGINT;
                goto error;
            }
        }

        /* process deviation properties */
        LY_TREE_FOR_SAFE(develem->child, next, child) {
            if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free(ctx, child);
                continue;
            }

            if (!strcmp(child->name, "config")) {
                if (d->flags & LYS_CONFIG_MASK) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }

                /* for we deviate from RFC 6020 and allow config property even it is/is not
                 * specified in the target explicitly since config property inherits. So we expect
                 * that config is specified in every node. But for delete, we check that the value
                 * is the same as here in deviation
                 */
                GETVAL(value, child, "value");
                if (!strcmp(value, "false")) {
                    d->flags |= LYS_CONFIG_R;
                } else if (!strcmp(value, "true")) {
                    d->flags |= LYS_CONFIG_W;
                } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_DEL) {
                    /* del config is forbidden */
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "config", "deviate delete");
                    goto error;
                } else { /* add and replace are the same in this case */
                    /* remove current config value of the target ... */
                    dev_target->flags &= ~LYS_CONFIG_MASK;

                    /* ... and replace it with the value specified in deviation */
                    dev_target->flags |= d->flags & LYS_CONFIG_MASK;
                }
            } else if (!strcmp(child->name, "default")) {
                if (d->dflt) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }
                GETVAL(value, child, "value");
                d->dflt = lydict_insert(ctx, value, 0);

                if (dev_target->nodetype == LYS_CHOICE) {
                    choice = (struct lys_node_choice *)dev_target;

                    if (d->mod == LY_DEVIATE_ADD) {
                        /* check that there is no current value */
                        if (choice->dflt) {
                            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
                            goto error;
                        }
                        /* check collision with mandatory */
                        if (choice->flags & LYS_MAND_TRUE) {
                            LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, child->name, child->parent->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                                   "Adding the \"default\" statement is forbidden on choice with the \"mandatory\" statement.");
                            goto error;
                        }
                    } else if (d->mod == LY_DEVIATE_RPL) {
                        /* check that there was a value before */
                        if (!choice->dflt) {
                            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
                            goto error;
                        }
                    }

                    rc = resolve_choice_default_schema_nodeid(d->dflt, choice->child, (const struct lys_node **)&node);
                    if (rc || !node) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                        goto error;
                    }
                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!choice->dflt || (choice->dflt != node)) {
                            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                            goto error;
                        }
                    } else { /* add (already checked) and replace */
                        choice->dflt = node;
                        if (!choice->dflt) {
                            /* default branch not found */
                            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                            goto error;
                        }
                    }
                } else if (dev_target->nodetype == LYS_LEAF) {
                    leaf = (struct lys_node_leaf *)dev_target;

                    if (d->mod == LY_DEVIATE_ADD) {
                        /* check that there is no current value */
                        if (leaf->dflt) {
                            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
                            goto error;
                        }
                        /* check collision with mandatory */
                        if (leaf->flags & LYS_MAND_TRUE) {
                            LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, child->name, child->parent->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                                   "Adding the \"default\" statement is forbidden on leaf with the \"mandatory\" statement.");
                            goto error;
                        }
                    }

                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!leaf->dflt || !ly_strequal(leaf->dflt, d->dflt, 1)) {
                            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                            goto error;
                        }
                        /* remove value */
                        lydict_remove(ctx, leaf->dflt);
                        leaf->dflt = NULL;
                    } else { /* add (already checked) and replace */
                        /* remove value */
                        lydict_remove(ctx, leaf->dflt);

                        /* set new value */
                        leaf->dflt = lydict_insert(ctx, d->dflt, 0);

                        /* remember to check it later (it may not fit now, but the type can be deviated too) */
                        leaf_dflt_check = ly_realloc(leaf_dflt_check, ++leaf_dflt_check_count * sizeof *leaf_dflt_check);
                        if (!leaf_dflt_check) {
                            LOGMEM;
                            goto error;
                        }
                        leaf_dflt_check[leaf_dflt_check_count - 1] = leaf;
                    }
                } else {
                    /* invalid target for default value */
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }
            } else if (!strcmp(child->name, "mandatory")) {
                if (d->flags & LYS_MAND_MASK) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (!(dev_target->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYXML))) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                GETVAL(value, child, "value");
                if (!strcmp(value, "false")) {
                    d->flags |= LYS_MAND_FALSE;
                } else if (!strcmp(value, "true")) {
                    d->flags |= LYS_MAND_TRUE;
                } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_ADD) {
                    /* check that there is no current value */
                    if (dev_target->flags & LYS_MAND_MASK) {
                        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
                        goto error;
                    }
                    /* check collision with default-stmt */
                    if ((dev_target->nodetype == LYS_LEAF) && ((struct lys_node_leaf *)(dev_target))->dflt) {
                        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, child->name, child->parent->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                               "Adding the \"mandatory\" statement is forbidden on leaf with the \"default\" statement.");
                        goto error;
                    }

                    dev_target->flags |= d->flags & LYS_MAND_MASK;
                } else if (d->mod == LY_DEVIATE_RPL) {
                    /* check that there was a value before */
                    if (!(dev_target->flags & LYS_MAND_MASK)) {
                        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
                        goto error;
                    }

                    dev_target->flags &= ~LYS_MAND_MASK;
                    dev_target->flags |= d->flags & LYS_MAND_MASK;
                } else if (d->mod == LY_DEVIATE_DEL) {
                    /* del mandatory is forbidden */
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "deviate delete");
                    goto error;
                }
            } else if (!strcmp(child->name, "min-elements")) {
                if (f_min) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }
                f_min = 1;

                if (deviate_minmax(dev_target, child, d, 0)) {
                    goto error;
                }
            } else if (!strcmp(child->name, "max-elements")) {
                if (f_max) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }
                f_max = 1;

                if (deviate_minmax(dev_target, child, d, 1)) {
                    goto error;
                }
            } else if (!strcmp(child->name, "must")) {
                c_must++;
                /* skip lyxml_free() at the end of the loop, this node will be processed later */
                continue;
            } else if (!strcmp(child->name, "type")) {
                if (d->type) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }

                /* add, del type is forbidden */
                if (d->mod == LY_DEVIATE_ADD) {
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "type", "deviate add");
                    goto error;
                } else if (d->mod == LY_DEVIATE_DEL) {
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "type", "deviate delete");
                    goto error;
                }

                /* check target node type */
                if (dev_target->nodetype == LYS_LEAF) {
                    t = &((struct lys_node_leaf *)dev_target)->type;
                } else if (dev_target->nodetype == LYS_LEAFLIST) {
                    t = &((struct lys_node_leaflist *)dev_target)->type;
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                /* replace */
                lys_type_free(ctx, t);
                /* HACK for unres */
                t->der = (struct lys_tpdf *)child;
                if (unres_schema_add_node(module, unres, t, UNRES_TYPE_DER, dev_target)) {
                    goto error;
                }
                d->type = t;

                /* check leaf default later (type may not fit now, but default can be deviated later too) */
                if (dev_target->nodetype == LYS_LEAF) {
                    leaf_dflt_check = ly_realloc(leaf_dflt_check, ++leaf_dflt_check_count * sizeof *leaf_dflt_check);
                    if (!leaf_dflt_check) {
                        LOGMEM;
                        goto error;
                    }
                    leaf_dflt_check[leaf_dflt_check_count - 1] = (struct lys_node_leaf *)dev_target;
                }
            } else if (!strcmp(child->name, "unique")) {
                c_uniq++;
                /* skip lyxml_free() at the end of the loop, this node will be processed later */
                continue;
            } else if (!strcmp(child->name, "units")) {
                if (d->units) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (dev_target->nodetype == LYS_LEAFLIST) {
                    stritem = &((struct lys_node_leaflist *)dev_target)->units;
                } else if (dev_target->nodetype == LYS_LEAF) {
                    stritem = &((struct lys_node_leaf *)dev_target)->units;
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                /* get units value */
                GETVAL(value, child, "name");
                d->units = lydict_insert(ctx, value, 0);

                /* apply to target */
                if (d->mod == LY_DEVIATE_ADD) {
                    /* check that there is no current value */
                    if (*stritem) {
                        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
                        goto error;
                    }

                    *stritem = lydict_insert(ctx, value, 0);
                } else if (d->mod == LY_DEVIATE_RPL) {
                    /* check that there was a value before */
                    if (!*stritem) {
                        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
                        goto error;
                    }

                    lydict_remove(ctx, *stritem);
                    *stritem = lydict_insert(ctx, value, 0);
                } else if (d->mod == LY_DEVIATE_DEL) {
                    /* check values */
                    if (!ly_strequal(*stritem, d->units, 1)) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                        goto error;
                    }
                    /* remove current units value of the target */
                    lydict_remove(ctx, *stritem);
                }
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                goto error;
            }

            /* do not free sub, it could have been unlinked and stored in unres */
        }

        if (c_must) {
            /* check target node type */
            switch (dev_target->nodetype) {
            case LYS_LEAF:
                trg_must = &((struct lys_node_leaf *)dev_target)->must;
                trg_must_size = &((struct lys_node_leaf *)dev_target)->must_size;
                break;
            case LYS_CONTAINER:
                trg_must = &((struct lys_node_container *)dev_target)->must;
                trg_must_size = &((struct lys_node_container *)dev_target)->must_size;
                break;
            case LYS_LEAFLIST:
                trg_must = &((struct lys_node_leaflist *)dev_target)->must;
                trg_must_size = &((struct lys_node_leaflist *)dev_target)->must_size;
                break;
            case LYS_LIST:
                trg_must = &((struct lys_node_list *)dev_target)->must;
                trg_must_size = &((struct lys_node_list *)dev_target)->must_size;
                break;
            case LYS_ANYXML:
                trg_must = &((struct lys_node_anyxml *)dev_target)->must;
                trg_must_size = &((struct lys_node_anyxml *)dev_target)->must_size;
                break;
            default:
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", child->name);
                goto error;
            }

            if (d->mod == LY_DEVIATE_RPL) {
                /* replace must is forbidden */
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "must", "deviate replace");
                goto error;
            } else if (d->mod == LY_DEVIATE_ADD) {
                /* reallocate the must array of the target */
                d->must = ly_realloc(*trg_must, (c_must + *trg_must_size) * sizeof *d->must);
                if (!d->must) {
                    LOGMEM;
                    goto error;
                }
                *trg_must = d->must;
                d->must = &((*trg_must)[*trg_must_size]);
                d->must_size = c_must;
            } else { /* LY_DEVIATE_DEL */
                d->must = calloc(c_must, sizeof *d->must);
            }
            if (!d->must) {
                LOGMEM;
                goto error;
            }
        }
        if (c_uniq) {
            /* replace unique is forbidden */
            if (d->mod == LY_DEVIATE_RPL) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "unique", "deviate replace");
                goto error;
            }

            /* check target node type */
            if (dev_target->nodetype != LYS_LIST) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", child->name);
                goto error;
            }

            list = (struct lys_node_list *)dev_target;
            if (d->mod == LY_DEVIATE_ADD) {
                /* reallocate the unique array of the target */
                d->unique = ly_realloc(list->unique, (c_uniq + list->unique_size) * sizeof *d->unique);
                list->unique = d->unique;
                d->unique = &list->unique[list->unique_size];
                d->unique_size = c_uniq;
            } else { /* LY_DEVIATE_DEL */
                d->unique = calloc(c_uniq, sizeof *d->unique);
            }
            if (!d->unique) {
                LOGMEM;
                goto error;
            }
        }

        /* process deviation properties with 0..n cardinality */
        LY_TREE_FOR(develem->child, child) {
            if (!strcmp(child->name, "must")) {
                if (d->mod == LY_DEVIATE_DEL) {
                    if (fill_yin_must(module, child, &d->must[d->must_size])) {
                        goto error;
                    }

                    /* find must to delete, we are ok with just matching conditions */
                    for (i = 0; i < *trg_must_size; i++) {
                        if (ly_strequal(d->must[d->must_size].expr, (*trg_must)[i].expr, 1)) {
                            /* we have a match, free the must structure ... */
                            lys_restr_free(ctx, &((*trg_must)[i]));
                            /* ... and maintain the array */
                            (*trg_must_size)--;
                            if (i != *trg_must_size) {
                                (*trg_must)[i].expr = (*trg_must)[*trg_must_size].expr;
                                (*trg_must)[i].dsc = (*trg_must)[*trg_must_size].dsc;
                                (*trg_must)[i].ref = (*trg_must)[*trg_must_size].ref;
                                (*trg_must)[i].eapptag = (*trg_must)[*trg_must_size].eapptag;
                                (*trg_must)[i].emsg = (*trg_must)[*trg_must_size].emsg;
                            }
                            if (!(*trg_must_size)) {
                                free(*trg_must);
                                *trg_must = NULL;
                            } else {
                                (*trg_must)[*trg_must_size].expr = NULL;
                                (*trg_must)[*trg_must_size].dsc = NULL;
                                (*trg_must)[*trg_must_size].ref = NULL;
                                (*trg_must)[*trg_must_size].eapptag = NULL;
                                (*trg_must)[*trg_must_size].emsg = NULL;
                            }

                            i = -1; /* set match flag */
                            break;
                        }
                    }
                    d->must_size++;
                    if (i != -1) {
                        /* no match found */
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL,
                               d->must[d->must_size - 1].expr, child->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value does not match any must from the target.");
                        goto error;
                    }
                } else { /* replace or add */
                    memset(&((*trg_must)[*trg_must_size]), 0, sizeof **trg_must);
                    if (fill_yin_must(module, child, &((*trg_must)[*trg_must_size]))) {
                        goto error;
                    }
                    (*trg_must_size)++;
                }
            } else if (!strcmp(child->name, "unique")) {
                if (d->mod == LY_DEVIATE_DEL) {
                    memset(&d->unique[d->unique_size], 0, sizeof *d->unique);
                    if (fill_yin_unique(module, dev_target, child, &d->unique[d->unique_size], NULL)) {
                        d->unique_size++;
                        goto error;
                    }

                    /* find unique structures to delete */
                    for (i = 0; i < list->unique_size; i++) {
                        if (list->unique[i].expr_size != d->unique[d->unique_size].expr_size) {
                            continue;
                        }

                        for (j = 0; j < d->unique[d->unique_size].expr_size; j++) {
                            if (!ly_strequal(list->unique[i].expr[j], d->unique[d->unique_size].expr[j], 1)) {
                                break;
                            }
                        }

                        if (j == d->unique[d->unique_size].expr_size) {
                            /* we have a match, free the unique structure ... */
                            for (j = 0; j < list->unique[i].expr_size; j++) {
                                lydict_remove(ctx, list->unique[i].expr[j]);
                            }
                            free(list->unique[i].expr);
                            /* ... and maintain the array */
                            list->unique_size--;
                            if (i != list->unique_size) {
                                list->unique[i].expr_size = list->unique[list->unique_size].expr_size;
                                list->unique[i].expr = list->unique[list->unique_size].expr;
                            }

                            if (!list->unique_size) {
                                free(list->unique);
                                list->unique = NULL;
                            } else {
                                list->unique[list->unique_size].expr_size = 0;
                                list->unique[list->unique_size].expr = NULL;
                            }

                            i = -1; /* set match flag */
                            break;
                        }
                    }

                    d->unique_size++;
                    if (i != -1) {
                        /* no match found */
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, lyxml_get_attr(child, "tag", NULL), child->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                        goto error;
                    }
                } else { /* replace or add */
                    memset(&list->unique[list->unique_size], 0, sizeof *list->unique);
                    i = fill_yin_unique(module, dev_target, child, &list->unique[list->unique_size], NULL);
                    list->unique_size++;
                    if (i) {
                        goto error;
                    }
                }
            }
        }
    }

    /* now check whether default value, if any, matches the type */
    for (i = 0; i < leaf_dflt_check_count; ++i) {
        if (leaf_dflt_check[i]->dflt) {
            rc = unres_schema_add_str(module, unres, &leaf_dflt_check[i]->type, UNRES_TYPE_DFLT, leaf_dflt_check[i]->dflt);
            if (rc == -1) {
                goto error;
            } else if (rc == EXIT_FAILURE) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, leaf_dflt_check[i]->dflt, "default");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Leaf \"%s\" default value no longer matches its type.", dev->target_name);
                goto error;
            }
        }
    }
    free(leaf_dflt_check);

    return EXIT_SUCCESS;

error:
    free(leaf_dflt_check);
    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_augment(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_node_augment *aug,
                 struct unres_schema *unres)
{
    const char *value;
    struct lyxml_elem *child, *next;
    struct lys_node *node;
    int c = 0, ret;

    aug->nodetype = LYS_AUGMENT;
    GETVAL(value, yin, "target-node");
    aug->target_name = transform_schema2json(module, value);
    if (!aug->target_name) {
        goto error;
    }
    aug->parent = parent;

    if (read_yin_common(module, NULL, (struct lys_node *)aug, yin, OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "if-feature")) {
            c++;
            continue;
        } else if (!strcmp(child->name, "when")) {
            if (aug->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }

            aug->when = read_yin_when(module, child);
            if (!aug->when) {
                lyxml_free(module->ctx, child);
                goto error;
            }
            lyxml_free(module->ctx, child);
            continue;

        /* check allowed data sub-statements */
        } else if (!strcmp(child->name, "container")) {
            node = read_yin_container(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "leaf-list")) {
            node = read_yin_leaflist(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "leaf")) {
            node = read_yin_leaf(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "list")) {
            node = read_yin_list(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "uses")) {
            node = read_yin_uses(module, (struct lys_node *)aug, child, unres);
        } else if (!strcmp(child->name, "choice")) {
            node = read_yin_case(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "case")) {
            node = read_yin_case(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "anyxml")) {
            node = read_yin_anyxml(module, (struct lys_node *)aug, child, 0, unres);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }

        if (!node) {
            goto error;
        }

        node = NULL;
        lyxml_free(module->ctx, child);
    }

    if (c) {
        aug->features = calloc(c, sizeof *aug->features);
        if (!aug->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!strcmp(child->name, "if-feature")) {
            ret = fill_yin_iffeature((struct lys_node *)aug, child, &aug->features[aug->features_size], unres);
            aug->features_size++;
            if (ret) {
                goto error;
            }
            lyxml_free(module->ctx, child);
        }
    }

    /* aug->child points to the parsed nodes, they must now be
     * connected to the tree and adjusted (if possible right now).
     * However, if this is augment in a uses, it gets resolved
     * when the uses does and cannot be resolved now for sure
     * (the grouping was not yet copied into uses).
     */
    if (!parent || (parent->nodetype != LYS_USES)) {
        if (unres_schema_add_node(module, unres, aug, UNRES_AUGMENT, NULL) == -1) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_refine(struct lys_module *module, struct lyxml_elem *yin, struct lys_refine *rfn)
{
    struct lyxml_elem *sub, *next;
    const char *value;
    char *endptr;
    int f_mand = 0, f_min = 0, f_max = 0;
    int c_must = 0;
    int r;
    unsigned long int val;

    if (read_yin_common(module, NULL, (struct lys_node *)rfn, yin, OPT_CONFIG)) {
        goto error;
    }

    GETVAL(value, yin, "target-node");
    rfn->target_name = transform_schema2json(module, value);
    if (!rfn->target_name) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        /* limited applicability */
        if (!strcmp(sub->name, "default")) {
            /* leaf or choice */
            if (rfn->mod.dflt) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LEAF | LYS_CHOICE);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LEAF | LYS_CHOICE;
            }

            GETVAL(value, sub, "value");
            rfn->mod.dflt = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "mandatory")) {
            /* leaf, choice or anyxml */
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_ANYXML);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LEAF | LYS_CHOICE | LYS_ANYXML;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                rfn->flags |= LYS_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                rfn->flags |= LYS_MAND_FALSE;
            } else {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
        } else if (!strcmp(sub->name, "min-elements")) {
            /* list or leaf-list */
            if (f_min) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            f_min = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LIST | LYS_LEAFLIST);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LIST | LYS_LEAFLIST;
            }

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            endptr = NULL;
            val = strtoul(value, &endptr, 10);
            if (*endptr || value[0] == '-' || errno || val > UINT32_MAX) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
            rfn->mod.list.min = (uint32_t) val;
            rfn->flags |= LYS_RFN_MINSET;
        } else if (!strcmp(sub->name, "max-elements")) {
            /* list or leaf-list */
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LIST | LYS_LEAFLIST);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LIST | LYS_LEAFLIST;
            }

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            if (!strcmp(value, "unbounded")) {
                rfn->mod.list.max = 0;
            } else {
                /* convert it to uint32_t */
                errno = 0;
                endptr = NULL;
                val = strtoul(value, &endptr, 10);
                if (*endptr || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                    goto error;
                }
                rfn->mod.list.max = (uint32_t) val;
            }
            rfn->flags |= LYS_RFN_MAXSET;
        } else if (!strcmp(sub->name, "presence")) {
            /* container */
            if (rfn->mod.presence) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= LYS_CONTAINER;
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_CONTAINER;
            }

            GETVAL(value, sub, "value");
            rfn->mod.presence = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "must")) {
            /* leaf-list, list, container or anyxml */
            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
            }

            c_must++;
            continue;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    /* process nodes with cardinality of 0..n */
    if (c_must) {
        rfn->must = calloc(c_must, sizeof *rfn->must);
        if (!rfn->must) {
            LOGMEM;
            goto error;
        }
    }
    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_must(module, sub, &rfn->must[rfn->must_size]);
        rfn->must_size++;
        if (r) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_import(struct lys_module *module, struct lyxml_elem *yin, struct lys_import *imp)
{
    struct lyxml_elem *child;
    const char *value;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "prefix")) {
            GETVAL(value, child, "value");
            if (lyp_check_identifier(value, LY_IDENT_PREFIX, module, NULL)) {
                goto error;
            }
            imp->prefix = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(child->name, "revision-date")) {
            if (imp->rev[0]) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            } else if (!imp->prefix) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                       "The \"prefix\" statement is expected before the \"revision-date\".");
                goto error;
            }
            GETVAL(value, child, "date");
            if (lyp_check_date(value)) {
                goto error;
            }
            memcpy(imp->rev, value, LY_REV_SIZE - 1);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    /* check mandatory information */
    if (!imp->prefix) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", yin->name);
        goto error;
    }

    GETVAL(value, yin, "module");

    return lyp_check_import(module, value, imp);

error:

    return EXIT_FAILURE;
}

/* logs directly
 * returns:
 *  0 - inc successfully filled
 * -1 - error, inc is cleaned
 *  1 - duplication, ignore the inc structure, inc is cleaned
 */
static int
fill_yin_include(struct lys_module *module, struct lys_submodule *submodule, struct lyxml_elem *yin,
                 struct lys_include *inc, struct unres_schema *unres)
{
    struct lyxml_elem *child;
    const char *value;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }
        if (!strcmp(child->name, "revision-date")) {
            if (inc->rev[0]) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", yin->name);
                goto error;
            }
            GETVAL(value, child, "date");
            if (lyp_check_date(value)) {
                goto error;
            }
            memcpy(inc->rev, value, LY_REV_SIZE - 1);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    GETVAL(value, yin, "module");

    return lyp_check_include(module, submodule, value, inc, unres);

error:

    return -1;
}

/* logs directly
 *
 * Covers:
 * description, reference, status, optionaly config
 *
 */
static int
read_yin_common(struct lys_module *module, struct lys_node *parent,
                struct lys_node *node, struct lyxml_elem *xmlnode, int opt)
{
    const char *value;
    struct lyxml_elem *sub, *next;
    struct ly_ctx *const ctx = module->ctx;

    if (opt & OPT_MODULE) {
        node->module = module;
    }

    if (opt & OPT_IDENT) {
        GETVAL(value, xmlnode, "name");
        if (lyp_check_identifier(value, LY_IDENT_NAME, NULL, NULL)) {
            goto error;
        }
        node->name = lydict_insert(ctx, value, strlen(value));
    }

    /* inherit NACM flags */
    if ((opt & OPT_NACMEXT) && parent) {
        node->nacm = parent->nacm;
    }

    /* process local parameters */
    LY_TREE_FOR_SAFE(xmlnode->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
            lyxml_free(ctx, sub);
            continue;
        }
        if  (strcmp(sub->ns->value, LY_NSYIN)) {
            /* NACM extensions */
            if ((opt & OPT_NACMEXT) && !strcmp(sub->ns->value, LY_NSNACM)) {
                if (!strcmp(sub->name, "default-deny-write")) {
                    node->nacm |= LYS_NACM_DENYW;
                } else if (!strcmp(sub->name, "default-deny-all")) {
                    node->nacm |= LYS_NACM_DENYA;
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
                    goto error;
                }
            }

            /* else garbage */
            lyxml_free(ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "description")) {
            if (node->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, xmlnode->name);
                goto error;
            }
            node->dsc = read_yin_subnode(ctx, sub, "text");
            if (!node->dsc) {
                goto error;
            }
        } else if (!strcmp(sub->name, "reference")) {
            if (node->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, xmlnode->name);
                goto error;
            }
            node->ref = read_yin_subnode(ctx, sub, "text");
            if (!node->ref) {
                goto error;
            }
        } else if (!strcmp(sub->name, "status")) {
            if (node->flags & LYS_STATUS_MASK) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, xmlnode->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            if (!strcmp(value, "current")) {
                node->flags |= LYS_STATUS_CURR;
            } else if (!strcmp(value, "deprecated")) {
                node->flags |= LYS_STATUS_DEPRC;
            } else if (!strcmp(value, "obsolete")) {
                node->flags |= LYS_STATUS_OBSLT;
            } else {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
        } else if ((opt & OPT_CONFIG) && !strcmp(sub->name, "config")) {
            if (node->flags & LYS_CONFIG_MASK) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, xmlnode->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            if (!strcmp(value, "false")) {
                node->flags |= LYS_CONFIG_R;
            } else if (!strcmp(value, "true")) {
                node->flags |= LYS_CONFIG_W;
            } else {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
            node->flags |= LYS_CONFIG_SET;
        } else {
            /* skip the lyxml_free */
            continue;
        }
        lyxml_free(ctx, sub);
    }

    if ((opt & OPT_INHERIT) && !(node->flags & LYS_CONFIG_MASK)) {
        /* get config flag from parent */
        if (parent && (parent->flags & LYS_CONFIG_R)) {
            node->flags |= LYS_CONFIG_R;
        } else {
            /* default config is true */
            node->flags |= LYS_CONFIG_W;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static struct lys_when *
read_yin_when(struct lys_module *module, struct lyxml_elem *yin)
{
    struct lys_when *retval = NULL;
    struct lyxml_elem *child;
    const char *value;

    retval = calloc(1, sizeof *retval);
    if (!retval) {
        LOGMEM;
        return NULL;
    }

    GETVAL(value, yin, "condition");
    retval->cond = transform_schema2json(module, value);
    if (!retval->cond) {
        goto error;
    }
    if (lyxp_syntax_check(retval->cond)) {
        goto error;
    }

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (retval->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            retval->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!retval->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (retval->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            retval->ref = read_yin_subnode(module->ctx, child, "text");
            if (!retval->ref) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    return retval;

error:

    lys_when_free(module->ctx, retval);
    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_case(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
              struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node_case *cs;
    struct lys_node *retval, *node = NULL;
    int c_ftrs = 0, ret;

    /* init */
    memset(&root, 0, sizeof root);

    cs = calloc(1, sizeof *cs);
    if (!cs) {
        LOGMEM;
        return NULL;
    }
    cs->nodetype = LYS_CASE;
    cs->prev = (struct lys_node *)cs;
    retval = (struct lys_node *)cs;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_INHERIT | OPT_NACMEXT)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "anyxml")) {

            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            /* skip lyxml_free() at the end of the loop, sub is processed later */
            continue;
        } else if (!strcmp(sub->name, "when")) {
            if (cs->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            cs->when = read_yin_when(module, sub);
            if (!cs->when) {
                goto error;
            }

            lyxml_free(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    if (c_ftrs) {
        cs->features = calloc(c_ftrs, sizeof *cs->features);
        if (!cs->features) {
            LOGMEM;
            goto error;
        }
    }
    LY_TREE_FOR(yin->child, sub) {
        ret = fill_yin_iffeature(retval, sub, &cs->features[cs->features_size], unres);
        cs->features_size++;
        if (ret) {
            goto error;
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    return retval;

error:

    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }
    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_choice(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve, struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, *dflt = NULL;
    struct ly_ctx *const ctx = module->ctx;
    struct lys_node *retval, *node = NULL;
    struct lys_node_choice *choice;
    const char *value;
    int f_mand = 0, c_ftrs = 0, ret;

    choice = calloc(1, sizeof *choice);
    if (!choice) {
        LOGMEM;
        return NULL;
    }
    choice->nodetype = LYS_CHOICE;
    choice->prev = (struct lys_node *)choice;
    retval = (struct lys_node *)choice;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "container")) {
            if (!(node = read_yin_container(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "leaf-list")) {
            if (!(node = read_yin_leaflist(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "leaf")) {
            if (!(node = read_yin_leaf(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "list")) {
            if (!(node = read_yin_list(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "case")) {
            if (!(node = read_yin_case(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "anyxml")) {
            if (!(node = read_yin_anyxml(module, retval, sub, resolve, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "default")) {
            if (dflt) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            dflt = sub;
            lyxml_unlink_elem(ctx, dflt, 0);

            continue;
            /* skip lyxml_free() at the end of the loop, the sub node is processed later as dflt */

        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                choice->flags |= LYS_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                choice->flags |= LYS_MAND_FALSE;
            } else {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */
        } else if (!strcmp(sub->name, "when")) {
            if (choice->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            choice->when = read_yin_when(module, sub);
            if (!choice->when) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

            /* skip lyxml_free() at the end of the loop, the sub node is processed later */
            continue;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }

        node = NULL;
        lyxml_free(ctx, sub);
    }

    if (c_ftrs) {
        choice->features = calloc(c_ftrs, sizeof *choice->features);
        if (!choice->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        ret = fill_yin_iffeature(retval, sub, &choice->features[choice->features_size], unres);
        choice->features_size++;
        if (ret) {
            goto error;
        }
    }

    /* check - default is prohibited in combination with mandatory */
    if (dflt && (choice->flags & LYS_MAND_TRUE)) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", "choice");
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"default\" statement is forbidden on choices with \"mandatory\".");
        goto error;
    }

    /* link default with the case */
    if (dflt) {
        GETVAL(value, dflt, "value");
        if (unres_schema_add_str(module, unres, choice, UNRES_CHOICE_DFLT, value) == -1) {
            goto error;
        }
        lyxml_free(ctx, dflt);
    }

    return retval;

error:

    lyxml_free(ctx, dflt);
    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_anyxml(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
                struct unres_schema *unres)
{
    struct lys_node *retval;
    struct lys_node_leaf *anyxml;
    struct lyxml_elem *sub, *next;
    const char *value;
    int r;
    int f_mand = 0;
    int c_must = 0, c_ftrs = 0;

    anyxml = calloc(1, sizeof *anyxml);
    if (!anyxml) {
        LOGMEM;
        return NULL;
    }
    anyxml->nodetype = LYS_ANYXML;
    anyxml->prev = (struct lys_node *)anyxml;
    retval = (struct lys_node *)anyxml;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                anyxml->flags |= LYS_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                anyxml->flags |= LYS_MAND_FALSE;
            } else {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
            /* else false is the default value, so we can ignore it */
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (anyxml->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            anyxml->when = read_yin_when(module, sub);
            if (!anyxml->when) {
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        anyxml->must = calloc(c_must, sizeof *anyxml->must);
        if (!anyxml->must) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        anyxml->features = calloc(c_ftrs, sizeof *anyxml->features);
        if (!anyxml->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &anyxml->must[anyxml->must_size]);
            anyxml->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &anyxml->features[anyxml->features_size], unres);
            anyxml->features_size++;
            if (r) {
                goto error;
            }
        }
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_leaf(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
              struct unres_schema *unres)
{
    struct lys_node *retval;
    struct lys_node_leaf *leaf;
    struct lyxml_elem *sub, *next;
    const char *value;
    int r, has_type = 0;
    int c_must = 0, c_ftrs = 0, f_mand = 0;

    leaf = calloc(1, sizeof *leaf);
    if (!leaf) {
        LOGMEM;
        return NULL;
    }
    leaf->nodetype = LYS_LEAF;
    leaf->prev = (struct lys_node *)leaf;
    retval = (struct lys_node *)leaf;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "type")) {
            if (has_type) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* HACK for unres */
            leaf->type.der = (struct lys_tpdf *)sub;
            leaf->type.parent = (struct lys_tpdf *)leaf;
            if (unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DER, retval)) {
                leaf->type.der = NULL;
                goto error;
            }
            has_type = 1;
        } else if (!strcmp(sub->name, "default")) {
            if (leaf->dflt) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            leaf->dflt = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "units")) {
            if (leaf->units) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            leaf->units = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in leaf is not sufficient, we would allow
             * multiple mandatory statements with the "false" value
             */
            f_mand = 1;

            GETVAL(value, sub, "value");
            if (!strcmp(value, "true")) {
                leaf->flags |= LYS_MAND_TRUE;
            } else if (!strcmp(value, "false")) {
                leaf->flags |= LYS_MAND_FALSE;
            } else {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */
        } else if (!strcmp(sub->name, "when")) {
            if (leaf->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            leaf->when = read_yin_when(module, sub);
            if (!leaf->when) {
                goto error;
            }

        } else if (!strcmp(sub->name, "must")) {
            c_must++;
            continue;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            continue;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }

        /* do not free sub, it could have been unlinked and stored in unres */
    }

    /* check mandatory parameters */
    if (!has_type) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", yin->name);
        goto error;
    }
    if (leaf->dflt) {
        if (unres_schema_add_str(module, unres, &leaf->type, UNRES_TYPE_DFLT, leaf->dflt) == -1) {
            goto error;
        }
        if (leaf->flags & LYS_MAND_TRUE) {
            LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "leaf");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                   "The \"mandatory\" statement is forbidden on leaf with the \"default\" statement.");
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        leaf->must = calloc(c_must, sizeof *leaf->must);
        if (!leaf->must) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        leaf->features = calloc(c_ftrs, sizeof *leaf->features);
        if (!leaf->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &leaf->must[leaf->must_size]);
            leaf->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &leaf->features[leaf->features_size], unres);
            leaf->features_size++;
            if (r) {
                goto error;
            }
        }
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_leaflist(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
                  struct unres_schema *unres)
{
    struct lys_node *retval;
    struct lys_node_leaflist *llist;
    struct lyxml_elem *sub, *next;
    const char *value;
    char *endptr;
    unsigned long val;
    int r, has_type = 0;
    int c_must = 0, c_ftrs = 0;
    int f_ordr = 0, f_min = 0, f_max = 0;

    llist = calloc(1, sizeof *llist);
    if (!llist) {
        LOGMEM;
        return NULL;
    }
    llist->nodetype = LYS_LEAFLIST;
    llist->prev = (struct lys_node *)llist;
    retval = (struct lys_node *)llist;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "type")) {
            if (has_type) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* HACK for unres */
            llist->type.der = (struct lys_tpdf *)sub;
            llist->type.parent = (struct lys_tpdf *)llist;
            if (unres_schema_add_node(module, unres, &llist->type, UNRES_TYPE_DER, retval)) {
                llist->type.der = NULL;
                goto error;
            }
            has_type = 1;
        } else if (!strcmp(sub->name, "units")) {
            if (llist->units) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            llist->units = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "ordered-by")) {
            if (f_ordr) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in llist is not sufficient, we would
             * allow multiple ordered-by statements with the "system" value
             */
            f_ordr = 1;

            if (llist->flags & LYS_CONFIG_R) {
                /* RFC 6020, 7.7.5 - ignore ordering when the list represents
                 * state data
                 */
                lyxml_free(module->ctx, sub);
                continue;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "user")) {
                llist->flags |= LYS_USERORDERED;
            } else if (strcmp(value, "system")) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            } /* else system is the default value, so we can ignore it */

        } else if (!strcmp(sub->name, "must")) {
            c_must++;
            continue;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            continue;

        } else if (!strcmp(sub->name, "min-elements")) {
            if (f_min) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            f_min = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            endptr = NULL;
            val = strtoul(value, &endptr, 10);
            if (*endptr || value[0] == '-' || errno || val > UINT32_MAX) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
            llist->min = (uint32_t) val;
            if (llist->max && (llist->min > llist->max)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                goto error;
            }
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            if (!strcmp(value, "unbounded")) {
                llist->max = 0;
            } else {
                /* convert it to uint32_t */
                errno = 0;
                endptr = NULL;
                val = strtoul(value, &endptr, 10);
                if (*endptr || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                    goto error;
                }
                llist->max = (uint32_t) val;
                if (llist->min > llist->max) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                    goto error;
                }
            }
        } else if (!strcmp(sub->name, "when")) {
            if (llist->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            llist->when = read_yin_when(module, sub);
            if (!llist->when) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }

        /* do not free sub, it could have been unlinked and stored in unres */
    }

    /* check constraints */
    if (!has_type) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", yin->name);
        goto error;
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        llist->must = calloc(c_must, sizeof *llist->must);
        if (!llist->must) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        llist->features = calloc(c_ftrs, sizeof *llist->features);
        if (!llist->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &llist->must[llist->must_size]);
            llist->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &llist->features[llist->features_size], unres);
            llist->features_size++;
            if (r) {
                goto error;
            }
        }
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_list(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
              struct unres_schema *unres)
{
    struct lys_node *retval, *node;
    struct lys_node_list *list;
    struct lyxml_elem *sub, *next, root, uniq;
    int r;
    int c_tpdf = 0, c_must = 0, c_uniq = 0, c_ftrs = 0;
    int f_ordr = 0, f_max = 0, f_min = 0;
    const char *key_str = NULL, *value;
    char *auxs;
    unsigned long val;

    /* init */
    memset(&root, 0, sizeof root);
    memset(&uniq, 0, sizeof uniq);

    list = calloc(1, sizeof *list);
    if (!list) {
        LOGMEM;
        return NULL;
    }
    list->nodetype = LYS_LIST;
    list->prev = (struct lys_node *)list;
    retval = (struct lys_node *)list;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process list's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        /* data statements */
        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "key")) {
            /* check cardinality 0..1 */
            if (list->keys_size) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, list->name);
                goto error;
            }

            /* count the number of keys */
            GETVAL(value, sub, "value");
            key_str = value;
            while ((value = strpbrk(value, " \t\n"))) {
                list->keys_size++;
                while (isspace(*value)) {
                    value++;
                }
            }
            list->keys_size++;
            list->keys = calloc(list->keys_size, sizeof *list->keys);
            if (!list->keys) {
                LOGMEM;
                goto error;
            }
        } else if (!strcmp(sub->name, "unique")) {
            c_uniq++;
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &uniq, sub);
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

            /* optional stetments */
        } else if (!strcmp(sub->name, "ordered-by")) {
            if (f_ordr) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            /* just checking the flags in llist is not sufficient, we would
             * allow multiple ordered-by statements with the "system" value
             */
            f_ordr = 1;

            if (list->flags & LYS_CONFIG_R) {
                /* RFC 6020, 7.7.5 - ignore ordering when the list represents
                 * state data
                 */
                lyxml_free(module->ctx, sub);
                continue;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "user")) {
                list->flags |= LYS_USERORDERED;
            } else if (strcmp(value, "system")) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
            /* else system is the default value, so we can ignore it */
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "min-elements")) {
            if (f_min) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            f_min = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            auxs = NULL;
            val = strtoul(value, &auxs, 10);
            if (*auxs || value[0] == '-' || errno || val > UINT32_MAX) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
            list->min = (uint32_t) val;
            if (list->max && (list->min > list->max)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            if (!strcmp(value, "unbounded")) {
                list->max = 0;;
            } else {
                /* convert it to uint32_t */
                errno = 0;
                auxs = NULL;
                val = strtoul(value, &auxs, 10);
                if (*auxs || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                    goto error;
                }
                list->max = (uint32_t) val;
                if (list->min > list->max) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                    goto error;
                }
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (list->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            list->when = read_yin_when(module, sub);
            if (!list->when) {
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* check - if list is configuration, key statement is mandatory */
    if ((list->flags & LYS_CONFIG_W) && !key_str) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "key", "list");
        goto error;
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        list->tpdf = calloc(c_tpdf, sizeof *list->tpdf);
        if (!list->tpdf) {
            LOGMEM;
            goto error;
        }
    }
    if (c_must) {
        list->must = calloc(c_must, sizeof *list->must);
        if (!list->must) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        list->features = calloc(c_ftrs, sizeof *list->features);
        if (!list->features) {
            LOGMEM;
            goto error;
        }
    }
    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &list->tpdf[list->tpdf_size], unres);
            list->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &list->features[list->features_size], unres);
            list->features_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &list->must[list->must_size]);
            list->must_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        } else {
            LOGINT;
            goto error;
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    if (key_str) {
        if (unres_schema_add_str(module, unres, list, UNRES_LIST_KEYS, key_str) == -1) {
            goto error;
        }
    } /* else config false list without a key, key_str presence in case of config true is checked earlier */

    /* process unique statements */
    if (c_uniq) {
        list->unique = calloc(c_uniq, sizeof *list->unique);
        if (!list->unique) {
            LOGMEM;
            goto error;
        }

        LY_TREE_FOR_SAFE(uniq.child, next, sub) {
            r = fill_yin_unique(module, retval, sub, &list->unique[list->unique_size], unres);
            list->unique_size++;
            if (r) {
                goto error;
            }

            lyxml_free(module->ctx, sub);
        }
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);
    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }
    while (uniq.child) {
        lyxml_free(module->ctx, uniq.child);
    }

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_container(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
                   struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_container *cont;
    const char *value;
    int r;
    int c_tpdf = 0, c_must = 0, c_ftrs = 0;

    /* init */
    memset(&root, 0, sizeof root);

    cont = calloc(1, sizeof *cont);
    if (!cont) {
        LOGMEM;
        return NULL;
    }
    cont->nodetype = LYS_CONTAINER;
    cont->prev = (struct lys_node *)cont;
    retval = (struct lys_node *)cont;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process container's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "presence")) {
            if (cont->presence) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            cont->presence = lydict_insert(module->ctx, value, strlen(value));

            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (cont->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            cont->when = read_yin_when(module, sub);
            if (!cont->when) {
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);

            /* data statements */
        } else if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        cont->tpdf = calloc(c_tpdf, sizeof *cont->tpdf);
        if (!cont->tpdf) {
            LOGMEM;
            goto error;
        }
    }
    if (c_must) {
        cont->must = calloc(c_must, sizeof *cont->must);
        if (!cont->must) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        cont->features = calloc(c_ftrs, sizeof *cont->features);
        if (!cont->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &cont->tpdf[cont->tpdf_size], unres);
            cont->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &cont->must[cont->must_size]);
            cont->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &cont->features[cont->features_size], unres);
            cont->features_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);
    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_grouping(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
                  struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_grp *grp;
    int r;
    int c_tpdf = 0;

    /* init */
    memset(&root, 0, sizeof root);

    grp = calloc(1, sizeof *grp);
    if (!grp) {
        LOGMEM;
        return NULL;
    }
    grp->nodetype = LYS_GROUPING;
    grp->prev = (struct lys_node *)grp;
    retval = (struct lys_node *)grp;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        /* data statements */
        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        grp->tpdf = calloc(c_tpdf, sizeof *grp->tpdf);
        if (!grp->tpdf) {
            LOGMEM;
            goto error;
        }
    }
    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_typedef(module, retval, sub, &grp->tpdf[grp->tpdf_size], unres);
        grp->tpdf_size++;
        if (r) {
            goto error;
        }
    }

    /* last part - process data nodes */
    if (!root.child) {
        LOGWRN("Grouping \"%s\" without children.", retval->name);
    }
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);
    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_input_output(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
                      struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval = NULL;
    struct lys_node_rpc_inout *inout;
    int r;
    int c_tpdf = 0;

    /* init */
    memset(&root, 0, sizeof root);

    inout = calloc(1, sizeof *inout);
    if (!inout) {
        LOGMEM;
        return NULL;
    }
    inout->prev = (struct lys_node *)inout;

    if (!strcmp(yin->name, "input")) {
        inout->nodetype = LYS_INPUT;
        inout->name = lydict_insert(module->ctx, "input", 0);
    } else if (!strcmp(yin->name, "output")) {
        inout->nodetype = LYS_OUTPUT;
        inout->name = lydict_insert(module->ctx, "output", 0);
    } else {
        LOGINT;
        free(inout);
        goto error;
    }

    retval = (struct lys_node *)inout;

    if (read_yin_common(module, parent, retval, yin, OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* data statements */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        inout->tpdf = calloc(c_tpdf, sizeof *inout->tpdf);
        if (!inout->tpdf) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_typedef(module, retval, sub, &inout->tpdf[inout->tpdf_size], unres);
        inout->tpdf_size++;
        if (r) {
            goto error;
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);
    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_notif(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
               struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_notif *notif;
    int r;
    int c_tpdf = 0, c_ftrs = 0;

    memset(&root, 0, sizeof root);

    notif = calloc(1, sizeof *notif);
    if (!notif) {
        LOGMEM;
        return NULL;
    }
    notif->nodetype = LYS_NOTIF;
    notif->prev = (struct lys_node *)notif;
    retval = (struct lys_node *)notif;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        /* data statements */
        if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        notif->tpdf = calloc(c_tpdf, sizeof *notif->tpdf);
        if (!notif->tpdf) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        notif->features = calloc(c_ftrs, sizeof *notif->features);
        if (!notif->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &notif->tpdf[notif->tpdf_size], unres);
            notif->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &notif->features[notif->features_size], unres);
            notif->features_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);
    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_rpc(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
             struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_rpc *rpc;
    int r;
    int c_tpdf = 0, c_ftrs = 0;

    /* init */
    memset(&root, 0, sizeof root);

    rpc = calloc(1, sizeof *rpc);
    if (!rpc) {
        LOGMEM;
        return NULL;
    }
    rpc->nodetype = LYS_RPC;
    rpc->prev = (struct lys_node *)rpc;
    retval = (struct lys_node *)rpc;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "input")) {
            if (rpc->child
                && (rpc->child->nodetype == LYS_INPUT
                    || (rpc->child->next && rpc->child->next->nodetype == LYS_INPUT))) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);
        } else if (!strcmp(sub->name, "output")) {
            if (rpc->child
                && (rpc->child->nodetype == LYS_INPUT
                    || (rpc->child->next && rpc->child->next->nodetype == LYS_INPUT))) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* data statements */
        } else if (!strcmp(sub->name, "grouping")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        rpc->tpdf = calloc(c_tpdf, sizeof *rpc->tpdf);
        if (!rpc->tpdf) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        rpc->features = calloc(c_ftrs, sizeof *rpc->features);
        if (!rpc->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &rpc->tpdf[rpc->tpdf_size], unres);
            rpc->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &rpc->features[rpc->features_size], unres);
            rpc->features_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "input")) {
            node = read_yin_input_output(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "output")) {
            node = read_yin_input_output(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);
    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }

    return NULL;
}

/* logs directly
 *
 * resolve - referenced grouping should be bounded to the namespace (resolved)
 * only when uses does not appear in grouping. In a case of grouping's uses,
 * we just get information but we do not apply augment or refine to it.
 */
static struct lys_node *
read_yin_uses(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next;
    struct lys_node *retval;
    struct lys_node_uses *uses;
    const char *value;
    int c_ref = 0, c_aug = 0, c_ftrs = 0;
    int r;

    uses = calloc(1, sizeof *uses);
    if (!uses) {
        LOGMEM;
        return NULL;
    }
    uses->nodetype = LYS_USES;
    uses->prev = (struct lys_node *)uses;
    retval = (struct lys_node *)uses;

    GETVAL(value, yin, "name");
    uses->name = lydict_insert(module->ctx, value, 0);

    if (read_yin_common(module, parent, retval, yin, OPT_MODULE
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* get other properties of uses */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "refine")) {
            c_ref++;
        } else if (!strcmp(sub->name, "augment")) {
            c_aug++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else if (!strcmp(sub->name, "when")) {
            if (uses->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            uses->when = read_yin_when(module, sub);
            if (!uses->when) {
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }
    }

    /* process properties with cardinality 0..n */
    if (c_ref) {
        uses->refine = calloc(c_ref, sizeof *uses->refine);
        if (!uses->refine) {
            LOGMEM;
            goto error;
        }
    }
    if (c_aug) {
        uses->augment = calloc(c_aug, sizeof *uses->augment);
        if (!uses->augment) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        uses->features = calloc(c_ftrs, sizeof *uses->features);
        if (!uses->features) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "refine")) {
            r = fill_yin_refine(module, sub, &uses->refine[uses->refine_size]);
            uses->refine_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "augment")) {
            r = fill_yin_augment(module, retval, sub, &uses->augment[uses->augment_size], unres);
            uses->augment_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, sub, &uses->features[uses->features_size], unres);
            uses->features_size++;
            if (r) {
                goto error;
            }
        }
    }

    if (unres_schema_add_node(module, unres, uses, UNRES_USES, NULL) == -1) {
        goto error;
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly
 *
 * common code for yin_read_module() and yin_read_submodule()
 */
static int
read_sub_module(struct lys_module *module, struct lys_submodule *submodule, struct lyxml_elem *yin,
                struct unres_schema *unres)
{
    struct ly_ctx *ctx = module->ctx;
    struct lyxml_elem *next, *child, *child2, root, grps, augs;
    struct lys_node *node = NULL;
    struct lys_module *trg;
    struct lys_include inc;
    const char *value;
    int i, r;
    size_t size;
    int version_flag = 0;
    /* counters */
    int c_imp = 0, c_rev = 0, c_tpdf = 0, c_ident = 0, c_inc = 0, c_aug = 0, c_ftrs = 0, c_dev = 0;

    /* to simplify code, store the module/submodule being processed as trg */
    trg = submodule ? (struct lys_module *)submodule : module;

    /* init */
    memset(&root, 0, sizeof root);
    memset(&grps, 0, sizeof grps);
    memset(&augs, 0, sizeof augs);

    /*
     * in the first run, we process elements with cardinality of 1 or 0..1 and
     * count elements with cardinality 0..n. Data elements (choices, containers,
     * leafs, lists, leaf-lists) are moved aside to be processed last, since we
     * need have all top-level and groupings already prepared at that time. In
     * the middle loop, we process other elements with carinality of 0..n since
     * we need to allocate arrays to store them.
     */
    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free(ctx, child);
            continue;
        }

        if (!submodule && !strcmp(child->name, "namespace")) {
            if (module->ns) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "uri");
            module->ns = lydict_insert(ctx, value, strlen(value));
            lyxml_free(ctx, child);
        } else if (!submodule && !strcmp(child->name, "prefix")) {
            if (module->prefix) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "value");
            if (lyp_check_identifier(value, LY_IDENT_PREFIX, module, NULL)) {
                goto error;
            }
            module->prefix = lydict_insert(ctx, value, strlen(value));
            lyxml_free(ctx, child);
        } else if (submodule && !strcmp(child->name, "belongs-to")) {
            if (submodule->prefix) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "module");
            if (!ly_strequal(value, submodule->belongsto->name, 1)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                goto error;
            }

            /* get the prefix substatement, start with checks */
            if (!child->child) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", child->name);
                goto error;
            } else if (strcmp(child->child->name, "prefix")) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->child->name);
                goto error;
            } else if (child->child->next) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->child->next->name);
                goto error;
            }
            /* and now finally get the value */
            GETVAL(value, child->child, "value");
            /* check here differs from a generic prefix check, since this prefix
             * don't have to be unique
             */
            if (lyp_check_identifier(value, LY_IDENT_NAME, NULL, NULL)) {
                goto error;
            }
            submodule->prefix = lydict_insert(ctx, value, strlen(value));

            /* we are done with belongs-to */
            lyxml_free(ctx, child);

            /* counters (statements with n..1 cardinality) */
        } else if (!strcmp(child->name, "import")) {
            c_imp++;
        } else if (!strcmp(child->name, "revision")) {
            c_rev++;
        } else if (!strcmp(child->name, "typedef")) {
            c_tpdf++;
        } else if (!strcmp(child->name, "identity")) {
            c_ident++;
        } else if (!strcmp(child->name, "include")) {
            c_inc++;
        } else if (!strcmp(child->name, "augment")) {
            c_aug++;
            /* keep augments separated, processed last */
            lyxml_unlink_elem(ctx, child, 2);
            lyxml_add_child(ctx, &augs, child);

        } else if (!strcmp(child->name, "feature")) {
            c_ftrs++;
        } else if (!strcmp(child->name, "deviation")) {
            c_dev++;

            /* data statements */
        } else if (!strcmp(child->name, "container") ||
                !strcmp(child->name, "leaf-list") ||
                !strcmp(child->name, "leaf") ||
                !strcmp(child->name, "list") ||
                !strcmp(child->name, "choice") ||
                !strcmp(child->name, "uses") ||
                !strcmp(child->name, "anyxml") ||
                !strcmp(child->name, "rpc") ||
                !strcmp(child->name, "notification")) {
            lyxml_unlink_elem(ctx, child, 2);
            lyxml_add_child(ctx, &root, child);

        } else if (!strcmp(child->name, "grouping")) {
            /* keep groupings separated and process them before other data statements */
            lyxml_unlink_elem(ctx, child, 2);
            lyxml_add_child(ctx, &grps, child);

            /* optional statements */
        } else if (!strcmp(child->name, "description")) {
            if (trg->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            trg->dsc = read_yin_subnode(ctx, child, "text");
            lyxml_free(ctx, child);
            if (!trg->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (trg->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            trg->ref = read_yin_subnode(ctx, child, "text");
            lyxml_free(ctx, child);
            if (!trg->ref) {
                goto error;
            }
        } else if (!strcmp(child->name, "organization")) {
            if (trg->org) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            trg->org = read_yin_subnode(ctx, child, "text");
            lyxml_free(ctx, child);
            if (!trg->org) {
                goto error;
            }
        } else if (!strcmp(child->name, "contact")) {
            if (trg->contact) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            trg->contact = read_yin_subnode(ctx, child, "text");
            lyxml_free(ctx, child);
            if (!trg->contact) {
                goto error;
            }
        } else if (!strcmp(child->name, "yang-version")) {
            /* TODO: support YANG 1.1 ? */
            if (version_flag) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "value");
            if (strcmp(value, "1")) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "yang-version");
                goto error;
            }
            version_flag = 1;
            if (!submodule) {
                module->version = 1;
            } /* TODO else check for the submodule's same version as in main module, waits for YANG 1.1 support */
            lyxml_free(ctx, child);

        } else if (!strcmp(child->name, "extension")) {
            GETVAL(value, child, "name");

            /* we have the following supported (hardcoded) extensions: */
            /* ietf-netconf's get-filter-element-attributes */
            if (!strcmp(module->ns, LY_NSNC) &&
                    !strcmp(value, "get-filter-element-attributes")) {
                LOGDBG("NETCONF filter extension found");
            /* NACM's default-deny-write and default-deny-all */
            } else if (!strcmp(module->ns, LY_NSNACM) &&
                    (!strcmp(value, "default-deny-write") || !strcmp(value, "default-deny-all"))) {
                LOGDBG("NACM extension found");
            /* other extensions are not supported, so inform about such an extension */
            } else {
                LOGWRN("Not supported \"%s\" extension statement found, ignoring.", value);
                lyxml_free(ctx, child);
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    /* check for mandatory statements */
    if (submodule && !submodule->prefix) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
        goto error;
    } else if (!submodule) {
        if (!module->ns) {
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "namespace", "module");
            goto error;
        }
        if (!module->prefix) {
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "module");
            goto error;
        }
    }

    /* allocate arrays for elements with cardinality of 0..n */
    if (c_imp) {
        size = (c_imp * sizeof *trg->imp) + sizeof(void*);
        trg->imp = calloc(1, size);
        if (!trg->imp) {
            LOGMEM;
            goto error;
        }
        /* set stop block for possible realloc */
        trg->imp[c_imp].module = (void*)0x1;
    }
    if (c_rev) {
        trg->rev = calloc(c_rev, sizeof *trg->rev);
        if (!trg->rev) {
            LOGMEM;
            goto error;
        }
    }
    if (c_tpdf) {
        trg->tpdf = calloc(c_tpdf, sizeof *trg->tpdf);
        if (!trg->tpdf) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ident) {
        trg->ident = calloc(c_ident, sizeof *trg->ident);
        if (!trg->ident) {
            LOGMEM;
            goto error;
        }
    }
    if (c_inc) {
        size = (c_inc * sizeof *trg->inc) + sizeof(void*);
        trg->inc = calloc(1, size);
        if (!trg->inc) {
            LOGMEM;
            goto error;
        }
        /* set stop block for possible realloc */
        trg->inc[c_inc].submodule = (void*)0x1;
    }
    if (c_aug) {
        trg->augment = calloc(c_aug, sizeof *trg->augment);
        if (!trg->augment) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        trg->features = calloc(c_ftrs, sizeof *trg->features);
        if (!trg->features) {
            LOGMEM;
            goto error;
        }
    }
    if (c_dev) {
        trg->deviation = calloc(c_dev, sizeof *trg->deviation);
        if (!trg->deviation) {
            LOGMEM;
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes and augments */
    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!strcmp(child->name, "import")) {
            r = fill_yin_import(trg, child, &trg->imp[trg->imp_size]);
            trg->imp_size++;
            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "include")) {
            memset(&inc, 0, sizeof inc);
            /* 1) pass module, not trg, since we want to pass the main module
             * 2) we cannot pass directly the structure in the array since
             * submodule parser can realloc our array of includes */
            r = fill_yin_include(module, submodule, child, &inc, unres);
            if (!r) {
                /* success, copy the filled data into the final array */
                memcpy(&trg->inc[trg->inc_size], &inc, sizeof inc);
                trg->inc_size++;
            } else if (r == -1) {
                goto error;
            }

        } else if (!strcmp(child->name, "revision")) {
            GETVAL(value, child, "date");
            if (lyp_check_date(value)) {
                goto error;
            }
            memcpy(trg->rev[trg->rev_size].date, value, LY_REV_SIZE - 1);
            /* check uniqueness of the revision date - not required by RFC */
            for (i = 0; i < trg->rev_size; i++) {
                if (!strcmp(value, trg->rev[i].date)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, child->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Revision is not unique.");
                }
            }

            LY_TREE_FOR(child->child, child2) {
                if (!strcmp(child2->name, "description")) {
                    if (trg->rev[trg->rev_size].dsc) {
                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child2->name, child->name);
                        goto error;
                    }
                    trg->rev[trg->rev_size].dsc = read_yin_subnode(ctx, child2, "text");
                    if (!trg->rev[trg->rev_size].dsc) {
                        goto error;
                    }
                } else if (!strcmp(child2->name, "reference")) {
                    if (trg->rev[trg->rev_size].ref) {
                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child2->name, child->name);
                        goto error;
                    }
                    trg->rev[trg->rev_size].ref = read_yin_subnode(ctx, child2, "text");
                    if (!trg->rev[trg->rev_size].ref) {
                        goto error;
                    }
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child2->name);
                    goto error;
                }
            }

            /* keep the latest revision at position 0 */
            if (trg->rev_size && strcmp(trg->rev[trg->rev_size].date, trg->rev[0].date) > 0) {
                /* switch their position */
                value = strdup(trg->rev[0].date);
                if (!value) {
                    LOGMEM;
                    goto error;
                }
                memcpy(trg->rev[0].date, trg->rev[trg->rev_size].date, LY_REV_SIZE - 1);
                memcpy(trg->rev[trg->rev_size].date, value, LY_REV_SIZE - 1);
                free((char *)value);

                if (!ly_strequal(trg->rev[0].dsc, trg->rev[trg->rev_size].dsc, 1)) {
                    value = trg->rev[0].dsc;
                    trg->rev[0].dsc = trg->rev[trg->rev_size].dsc;
                    trg->rev[trg->rev_size].dsc = value;
                }

                if (!ly_strequal(trg->rev[0].ref, trg->rev[trg->rev_size].ref, 1)) {
                    value = trg->rev[0].ref;
                    trg->rev[0].ref = trg->rev[trg->rev_size].ref;
                    trg->rev[trg->rev_size].ref = value;
                }
            }

            trg->rev_size++;

        } else if (!strcmp(child->name, "typedef")) {
            r = fill_yin_typedef(trg, NULL, child, &trg->tpdf[trg->tpdf_size], unres);
            trg->tpdf_size++;
            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "identity")) {
            r = fill_yin_identity(trg, child, &trg->ident[trg->ident_size], unres);
            trg->ident_size++;
            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "feature")) {
            r = fill_yin_feature(trg, child, &trg->features[trg->features_size], unres);
            trg->features_size++;
            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "deviation")) {
            r = fill_yin_deviation(trg, child, &trg->deviation[trg->deviation_size], unres);
            trg->deviation_size++;
            if (r) {
                goto error;
            }
            /* module with deviation - must be implemented (description of /ietf-yang-library:modules-state/module/deviation) */
            module->implemented = 1;

        }
    }

    if (!submodule) {
        /* update the size of the arrays, they can be smaller due to possible duplicities
         * found in submodules */
        if (module->inc_size) {
            module->inc = ly_realloc(module->inc, module->inc_size * sizeof *module->inc);
            if (!module->inc) {
                LOGMEM;
                goto error;
            }
        }
        if (module->imp_size) {
            module->imp = ly_realloc(module->imp, module->imp_size * sizeof *module->imp);
            if (!module->imp) {
                LOGMEM;
                goto error;
            }
        }
    }

    /* process data nodes. Start with groupings to allow uses
     * refer to them. Submodule's data nodes are stored in the
     * main module data tree.
     */
    LY_TREE_FOR_SAFE(grps.child, next, child) {
        node = read_yin_grouping(trg, NULL, child, 0, unres);
        if (!node) {
            goto error;
        }

        lyxml_free(ctx, child);
    }

    /* parse data nodes, ... */
    LY_TREE_FOR_SAFE(root.child, next, child) {

        if (!strcmp(child->name, "container")) {
            node = read_yin_container(trg, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "leaf-list")) {
            node = read_yin_leaflist(trg, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "leaf")) {
            node = read_yin_leaf(trg, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "list")) {
            node = read_yin_list(trg, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "choice")) {
            node = read_yin_choice(trg, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "uses")) {
            node = read_yin_uses(trg, NULL, child, unres);
        } else if (!strcmp(child->name, "anyxml")) {
            node = read_yin_anyxml(trg, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "rpc")) {
            node = read_yin_rpc(trg, NULL, child, 0, unres);
        } else if (!strcmp(child->name, "notification")) {
            node = read_yin_notif(trg, NULL, child, 0, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(ctx, child);
    }

    /* ... and finally augments (last, so we can augment our data, for instance) */
    LY_TREE_FOR_SAFE(augs.child, next, child) {
        r = fill_yin_augment(trg, NULL, child, &trg->augment[trg->augment_size], unres);
        trg->augment_size++;

        if (r) {
            goto error;
        }
        lyxml_free(ctx, child);
    }

    return EXIT_SUCCESS;

error:
    /* cleanup */
    while (root.child) {
        lyxml_free(module->ctx, root.child);
    }
    while (grps.child) {
        lyxml_free(module->ctx, grps.child);
    }
    while (augs.child) {
        lyxml_free(module->ctx, augs.child);
    }

    return EXIT_FAILURE;
}

/* logs directly */
struct lys_submodule *
yin_read_submodule(struct lys_module *module, const char *data, struct unres_schema *unres)
{
    struct lyxml_elem *yin;
    struct lys_submodule *submodule = NULL;
    const char *value;

    assert(module->ctx);

    yin = lyxml_parse_mem(module->ctx, data, 0);
    if (!yin) {
        return NULL;
    }

    /* check root element */
    if (!yin->name || strcmp(yin->name, "submodule")) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yin->name);
        goto error;
    }

    GETVAL(value, yin, "name");
    if (lyp_check_identifier(value, LY_IDENT_NAME, NULL, NULL)) {
        goto error;
    }

    submodule = calloc(1, sizeof *submodule);
    if (!submodule) {
        LOGMEM;
        goto error;
    }

    submodule->ctx = module->ctx;
    submodule->name = lydict_insert(submodule->ctx, value, strlen(value));
    submodule->type = 1;
    submodule->belongsto = module;

    LOGVRB("Reading submodule \"%s\".", submodule->name);
    if (read_sub_module(module, submodule, yin, unres)) {
        goto error;
    }

    /* cleanup */
    lyxml_free(module->ctx, yin);

    LOGVRB("Submodule \"%s\" successfully parsed.", submodule->name);
    return submodule;

error:
    /* cleanup */
    unres_schema_free((struct lys_module *)submodule, &unres);
    lyxml_free(module->ctx, yin);

    if (!submodule) {
        LOGERR(ly_errno, "Submodule parsing failed.");
        return NULL;
    }

    LOGERR(ly_errno, "Submodule \"%s\" parsing failed.", submodule->name);

    lys_sub_module_remove_devs_augs((struct lys_module *)submodule);
    lys_submodule_module_data_free(submodule);
    lys_submodule_free(submodule, NULL);
    return NULL;
}

/* logs directly */
struct lys_module *
yin_read_module(struct ly_ctx *ctx, const char *data, const char *revision, int implement)
{
    struct lyxml_elem *yin;
    struct lys_module *module = NULL;
    struct unres_schema *unres;
    const char *value;
    int i;

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        return NULL;
    }

    yin = lyxml_parse_mem(ctx, data, 0);
    if (!yin) {
       goto error;
    }

    /* check root element */
    if (!yin->name || strcmp(yin->name, "module")) {
        LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yin->name);
        goto error;
    }

    GETVAL(value, yin, "name");
    if (lyp_check_identifier(value, LY_IDENT_NAME, NULL, NULL)) {
        goto error;
    }

    module = calloc(1, sizeof *module);
    if (!module) {
        LOGMEM;
        goto error;
    }

    module->ctx = ctx;
    module->name = lydict_insert(ctx, value, strlen(value));
    module->type = 0;
    module->implemented = (implement ? 1 : 0);

    LOGVRB("Reading module \"%s\".", module->name);
    if (read_sub_module(module, NULL, yin, unres)) {
        goto error;
    }

    /* resolve rest of unres items */
    if (unres->count && resolve_unres_schema(module, unres)) {
        goto error;
    }

    if (revision) {
        /* check revision of the parsed model */
        if (!module->rev_size || strcmp(revision, module->rev[0].date)) {
            LOGVRB("Module \"%s\" parsed with the wrong revision (\"%s\" instead \"%s\").",
                   module->name, module->rev[0].date, revision);
            goto error;
        }
    }

    if (lyp_ctx_add_module(&module)) {
        goto error;
    }

    if (module->augment_size || module->deviation_size) {
        if (!module->implemented) {
            LOGVRB("Module \"%s\" includes augments or deviations, changing conformance to \"implement\".", module->name);
        }
        if (lys_module_set_implement(module)) {
            goto error;
        }

        if (lys_sub_module_set_dev_aug_target_implement(module)) {
            goto error;
        }
        for (i = 0; i < module->inc_size; ++i) {
            if (!module->inc[i].submodule) {
                continue;
            }
            if (lys_sub_module_set_dev_aug_target_implement((struct lys_module *)module->inc[i].submodule)) {
                goto error;
            }
        }
    }

    lyxml_free(ctx, yin);
    unres_schema_free(NULL, &unres);
    LOGVRB("Module \"%s\" successfully parsed.", module->name);
    return module;

error:
    /* cleanup */
    lyxml_free(ctx, yin);
    unres_schema_free(module, &unres);

    if (!module) {
        LOGERR(ly_errno, "Module parsing failed.");
        return NULL;
    }

    LOGERR(ly_errno, "Module \"%s\" parsing failed.", module->name);

    lys_sub_module_remove_devs_augs(module);
    lys_free(module, NULL, 1);
    return NULL;
}
