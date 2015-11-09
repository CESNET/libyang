/**
 * @file yin.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YIN parser for libyang
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <pcre.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "dict_private.h"
#include "xpath.h"
#include "parser.h"
#include "resolve.h"
#include "tree_internal.h"
#include "xml_internal.h"

enum LY_IDENT {
    LY_IDENT_SIMPLE,   /* only syntax rules */
    LY_IDENT_FEATURE,
    LY_IDENT_IDENTITY,
    LY_IDENT_TYPE,
    LY_IDENT_NODE,
    LY_IDENT_NAME,     /* uniqueness across the siblings */
    LY_IDENT_PREFIX
};

#define GETVAL(value, node, arg)                                    \
	value = lyxml_get_attr(node, arg, NULL);                        \
	if (!value) {                                                   \
		LOGVAL(LYE_MISSARG, LOGLINE(node), arg, node->name);         \
		goto error;                                                 \
	}

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
                                      int resolve, struct unres_schema *unres);
static struct lys_node *read_yin_grouping(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                          int resolve, struct unres_schema *unres);
static struct lys_when *read_yin_when(struct lys_module *module, struct lyxml_elem *yin);

/* does not log */
static int
dup_typedef_check(const char *type, struct lys_tpdf *tpdf, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (!strcmp(type, tpdf[i].name)) {
            /* name collision */
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log */
static int
dup_feature_check(const char *id, struct lys_module *module)
{
    int i;

    for (i = 0; i < module->features_size; i++) {
        if (!strcmp(id, module->features[i].name)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log */
static int
dup_prefix_check(const char *prefix, struct lys_module *module)
{
    int i;

    if (!module->type && module->prefix && !strcmp(module->prefix, prefix)) {
        return EXIT_FAILURE;
    }
    for (i = 0; i < module->imp_size; i++) {
        if (!strcmp(module->imp[i].prefix, prefix)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
check_identifier(const char *id, enum LY_IDENT type, unsigned int line,
                 struct lys_module *module, struct lys_node *parent)
{
    int i;
    int size;
    struct lys_tpdf *tpdf;
    struct lys_node *node;

    assert(id);

    /* check id syntax */
    if (!(id[0] >= 'A' && id[0] <= 'Z') && !(id[0] >= 'a' && id[0] <= 'z') && id[0] != '_') {
        LOGVAL(LYE_INID, line, id, "invalid start character");
        return EXIT_FAILURE;
    }
    for (i = 1; id[i]; i++) {
        if (!(id[i] >= 'A' && id[i] <= 'Z') && !(id[i] >= 'a' && id[i] <= 'z')
                && !(id[i] >= '0' && id[i] <= '9') && id[i] != '_' && id[i] != '-' && id[i] != '.') {
            LOGVAL(LYE_INID, line, id, "invalid character");
            return EXIT_FAILURE;
        }
    }

    if (i > 64) {
        LOGWRN("Identifier \"%s\" is long, you should use something shorter.", id);
    }

    switch (type) {
    case LY_IDENT_NAME:
        /* check uniqueness of the node within its siblings */
        if (!parent) {
            break;
        }

        LY_TREE_FOR(parent->child, node) {
            if (node->name == id) {
                LOGVAL(LYE_INID, line, id, "name duplication");
                return EXIT_FAILURE;
            }
        }
        break;
    case LY_IDENT_TYPE:
        assert(module);

        /* check collision with the built-in types */
        if (!strcmp(id, "binary") || !strcmp(id, "bits") ||
                !strcmp(id, "boolean") || !strcmp(id, "decimal64") ||
                !strcmp(id, "empty") || !strcmp(id, "enumeration") ||
                !strcmp(id, "identityref") || !strcmp(id, "instance-identifier") ||
                !strcmp(id, "int8") || !strcmp(id, "int16") ||
                !strcmp(id, "int32") || !strcmp(id, "int64") ||
                !strcmp(id, "leafref") || !strcmp(id, "string") ||
                !strcmp(id, "uint8") || !strcmp(id, "uint16") ||
                !strcmp(id, "uint32") || !strcmp(id, "uint64") || !strcmp(id, "union")) {
            LOGVAL(LYE_SPEC, line, "Typedef name duplicates built-in type.");
            return EXIT_FAILURE;
        }

        /* check locally scoped typedefs (avoid name shadowing) */
        for (; parent; parent = parent->parent) {
            switch (parent->nodetype) {
            case LYS_CONTAINER:
                size = ((struct lys_node_container *)parent)->tpdf_size;
                tpdf = ((struct lys_node_container *)parent)->tpdf;
                break;
            case LYS_LIST:
                size = ((struct lys_node_list *)parent)->tpdf_size;
                tpdf = ((struct lys_node_list *)parent)->tpdf;
                break;
            case LYS_GROUPING:
                size = ((struct lys_node_grp *)parent)->tpdf_size;
                tpdf = ((struct lys_node_grp *)parent)->tpdf;
                break;
            default:
                continue;
            }

            if (dup_typedef_check(id, tpdf, size)) {
                LOGVAL(LYE_DUPID, line, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        /* check top-level names */
        if (dup_typedef_check(id, module->tpdf, module->tpdf_size)) {
            LOGVAL(LYE_DUPID, line, "typedef", id);
            return EXIT_FAILURE;
        }

        /* check submodule's top-level names */
        for (i = 0; i < module->inc_size; i++) {
            if (dup_typedef_check(id, module->inc[i].submodule->tpdf, module->inc[i].submodule->tpdf_size)) {
                LOGVAL(LYE_DUPID, line, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        /* check top-level names in the main module */
        if (module->type) {
            if (dup_typedef_check(id, ((struct lys_submodule *)module)->belongsto->tpdf,
                                  ((struct lys_submodule *)module)->belongsto->tpdf_size)) {
                LOGVAL(LYE_DUPID, line, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        break;
    case LY_IDENT_PREFIX:
        assert(module);

        if (module->type) {
            /* go to the main module */
            module = ((struct lys_submodule *)module)->belongsto;
        }

        /* check the main module itself */
        if (dup_prefix_check(id, module)) {
            LOGVAL(LYE_DUPID, line, "prefix", id);
            return EXIT_FAILURE;
        }

        /* and all its submodules */
        for (i = 0; i < module->inc_size; i++) {
            if (dup_prefix_check(id, (struct lys_module *)module->inc[i].submodule)) {
                LOGVAL(LYE_DUPID, line, "prefix", id);
                return EXIT_FAILURE;
            }
        }
        break;
    case LY_IDENT_FEATURE:
        assert(module);

        /* check feature name uniqness*/
        /* check features in the current module */
        if (dup_feature_check(id, module)) {
            LOGVAL(LYE_DUPID, line, "feature", id);
            return EXIT_FAILURE;
        }

        /* and all its submodules */
        for (i = 0; i < module->inc_size; i++) {
            if (dup_feature_check(id, (struct lys_module *)module->inc[i].submodule)) {
                LOGVAL(LYE_DUPID, line, "feature", id);
                return EXIT_FAILURE;
            }
        }
        break;

    default:
        /* no check required */
        break;
    }

    return EXIT_SUCCESS;
}

/* does not log */
static int
check_mandatory(struct lys_node *node)
{
    struct lys_node *child;

    assert(node);

    if (node->flags & LYS_MAND_TRUE) {
        return EXIT_FAILURE;
    }

    if (node->nodetype == LYS_CASE || node->nodetype == LYS_CHOICE) {
        LY_TREE_FOR(node->child, child) {
            if (check_mandatory(child)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
check_date(const char *date, unsigned int line)
{
    int i;

    assert(date);

    if (strlen(date) != LY_REV_SIZE - 1) {
        goto error;
    }

    for (i = 0; i < LY_REV_SIZE - 1; i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') {
                goto error;
            }
        } else if (!isdigit(date[i])) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    LOGVAL(LYE_INDATE, line, date);
    return EXIT_FAILURE;
}

/* logs directly */
static const char *
read_yin_subnode(struct ly_ctx *ctx, struct lyxml_elem *node, const char *name)
{
    int len;

    /* there should be <text> child */
    if (!node->child || !node->child->name || strcmp(node->child->name, name)) {
        LOGWRN("Expected \"%s\" element in \"%s\" element.", name, node->name);
    } else if (node->child->content) {
        len = strlen(node->child->content);
        return lydict_insert(ctx, node->child->content, len);
    }

    LOGVAL(LYE_INARG, LOGLINE(node), name, node->name);
    return NULL;
}

/* logs directly */
static int
fill_yin_identity(struct lys_module *module, struct lyxml_elem *yin, struct lys_ident *ident, struct unres_schema *unres)
{
    struct lyxml_elem *node;
    const char *value;

    GETVAL(value, yin, "name");
    ident->name = value;

    if (read_yin_common(module, NULL, (struct lys_node *)ident, yin, OPT_IDENT | OPT_MODULE)) {
        return EXIT_FAILURE;
    }

    LY_TREE_FOR(yin->child, node) {
        if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(node->name, "base")) {
            if (ident->base) {
                LOGVAL(LYE_TOOMANY, LOGLINE(node), "base", "identity");
                return EXIT_FAILURE;
            }
            GETVAL(value, node, "name");
            value = transform_schema2json(module, value, LOGLINE(node));
            if (!value) {
                return EXIT_FAILURE;
            }

            if (unres_schema_add_str(module, unres, ident, UNRES_IDENT, value, LOGLINE(node)) == -1) {
                lydict_remove(module->ctx, value);
                return EXIT_FAILURE;
            }
            lydict_remove(module->ctx, value);
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(node), node->name, "identity");
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
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->dsc = read_yin_subnode(ctx, child, "text");
            if (!restr->dsc) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (restr->ref) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->ref = read_yin_subnode(ctx, child, "text");
            if (!restr->ref) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "error-app-tag")) {
            if (restr->eapptag) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            GETVAL(value, child, "value");
            restr->eapptag = lydict_insert(ctx, value, 0);
        } else if (!strcmp(child->name, "error-message")) {
            if (restr->emsg) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                return EXIT_FAILURE;
            }
            restr->emsg = read_yin_subnode(ctx, child, "value");
            if (!restr->emsg) {
                return EXIT_FAILURE;
            }
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_type(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_type *type,
              struct unres_schema *unres)
{
#define REGEX_ERR_LEN 128
    const char *value, *name, *err_ptr;
    struct lyxml_elem *next, *node;
    struct lys_restr **restr;
    struct lys_type_bit bit;
    pcre *precomp;
    int i, j, rc, err_offset;
    int64_t v, v_;
    int64_t p, p_;

    GETVAL(value, yin, "name");
    value = transform_schema2json(module, value, LOGLINE(yin));
    if (!value) {
        goto error;
    }

    i = parse_identifier(value);
    if (i < 1) {
        LOGVAL(LYE_INCHAR, LOGLINE(yin), value[-i], &value[-i]);
        goto error;
    }
    /* module name */
    if (value[i]) {
        type->module_name = lydict_insert(module->ctx, value, i);
        value += i;
        if ((value[0] != ':') || (parse_identifier(value + 1) < 1)) {
            LOGVAL(LYE_INCHAR, LOGLINE(yin), value[0], value);
            goto error;
        }
        ++value;
    }

    rc = resolve_superior_type(value, type->module_name, module, parent, &type->der);
    if (rc == -1) {
        LOGVAL(LYE_INMOD, LOGLINE(yin), type->module_name);
        goto error;
    } else if (rc == EXIT_FAILURE) {
        /* HACK for unres */
        type->der = (struct lys_tpdf *)parent;
        rc = unres_schema_add_str(module, unres, type, UNRES_TYPE_DER, value, LOGLINE(yin));
        if (rc == -1) {
            goto error;
        } else {
            return EXIT_SUCCESS;
        }
    }
    type->base = type->der->type.base;

    switch (type->base) {
    case LY_TYPE_BITS:
        /* RFC 6020 9.7.4 - bit */

        /* get bit specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "bit")) {
                type->info.bits.count++;
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }
        if (!type->der->type.der && !type->info.bits.count) {
            /* type is derived directly from buit-in bits type and bit statement is required */
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "bit", "type");
            goto error;
        }
        if (type->der->type.der && type->info.bits.count) {
            /* type is not directly derived from buit-in bits type and bit statement is prohibited */
            LOGVAL(LYE_INSTMT, LOGLINE(yin), "bit");
            goto error;
        }

        type->info.bits.bit = calloc(type->info.bits.count, sizeof *type->info.bits.bit);
        p = 0;
        i = -1;
        LY_TREE_FOR(yin->child, next) {
            i++;

            GETVAL(value, next, "name");
            if (check_identifier(value, LY_IDENT_SIMPLE, LOGLINE(next), NULL, NULL)) {
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
                    LOGVAL(LYE_BITS_DUPNAME, LOGLINE(next), type->info.bits.bit[i].name);
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
                        LOGVAL(LYE_INARG, LOGLINE(node), value, "bit/position");
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
                                LOGVAL(LYE_BITS_DUPVAL, LOGLINE(node), type->info.bits.bit[i].pos, type->info.bits.bit[i].name);
                                type->info.bits.count = i + 1;
                                goto error;
                            }
                        }
                    }
                } else {
                    LOGVAL(LYE_INSTMT, LOGLINE(next), next->name);
                    goto error;
                }
            }
            if (p_ == -1) {
                /* assign value automatically */
                if (p > UINT32_MAX) {
                    LOGVAL(LYE_INARG, LOGLINE(next), "4294967295", "bit/position");
                    type->info.bits.count = i + 1;
                    goto error;
                }
                type->info.bits.bit[i].pos = (uint32_t)p;
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
                    LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (lyp_check_length_range(value, type)) {
                    LOGVAL(LYE_INARG, LOGLINE(node), value, "range");
                    goto error;
                }
                type->info.dec64.range = calloc(1, sizeof *type->info.dec64.range);
                type->info.dec64.range->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module->ctx, type->info.dec64.range, node)) {
                    goto error;
                }
            } else if (!strcmp(node->name, "fraction-digits")) {
                if (type->info.dec64.dig) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }
                GETVAL(value, node, "value");
                v = strtol(value, NULL, 10);

                /* range check */
                if (v < 1 || v > 18) {
                    LOGVAL(LYE_INARG, LOGLINE(node), value, node->name);
                    goto error;
                }
                type->info.dec64.dig = (uint8_t)v;
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }

        /* mandatory sub-statement(s) check */
        if (!type->info.dec64.dig && !type->der->type.der) {
            /* decimal64 type directly derived from built-in type requires fraction-digits */
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "fraction-digits", "type");
            goto error;
        }
        if (type->info.dec64.dig && type->der->type.der) {
            /* type is not directly derived from buit-in type and fraction-digits statement is prohibited */
            LOGVAL(LYE_INSTMT, LOGLINE(yin), "fraction-digits");
            goto error;
        }
        break;

    case LY_TYPE_ENUM:
        /* RFC 6020 9.6 - enum */

        /* get enum specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "enum")) {
                type->info.enums.count++;
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }
        if (!type->der->type.der && !type->info.enums.count) {
            /* type is derived directly from buit-in enumeartion type and enum statement is required */
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "enum", "type");
            goto error;
        }
        if (type->der->type.der && type->info.enums.count) {
            /* type is not directly derived from buit-in enumeration type and enum statement is prohibited */
            LOGVAL(LYE_INSTMT, LOGLINE(yin), "enum");
            goto error;
        }

        type->info.enums.enm = calloc(type->info.enums.count, sizeof *type->info.enums.enm);
        v = 0;
        i = -1;
        LY_TREE_FOR(yin->child, next) {
            i++;

            GETVAL(value, next, "name");
            if (check_identifier(value, LY_IDENT_SIMPLE, LOGLINE(next), NULL, NULL)) {
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
                LOGVAL(LYE_ENUM_WS, LOGLINE(next), value);
                type->info.enums.count = i + 1;
                goto error;
            }

            /* check the name uniqueness */
            for (j = 0; j < i; j++) {
                if (!strcmp(type->info.enums.enm[j].name, type->info.enums.enm[i].name)) {
                    LOGVAL(LYE_ENUM_DUPNAME, LOGLINE(next), type->info.enums.enm[i].name);
                    type->info.enums.count = i + 1;
                    goto error;
                }
            }

            v_ = -1;
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
                        LOGVAL(LYE_INARG, LOGLINE(node), value, "enum/value");
                        type->info.enums.count = i + 1;
                        goto error;
                    }
                    type->info.enums.enm[i].value = v_;

                    /* keep the highest enum value for automatic increment */
                    if (type->info.enums.enm[i].value > v) {
                        v = type->info.enums.enm[i].value;
                        v++;
                    } else {
                        /* check that the value is unique */
                        for (j = 0; j < i; j++) {
                            if (type->info.enums.enm[j].value == type->info.enums.enm[i].value) {
                                LOGVAL(LYE_ENUM_DUPVAL, LOGLINE(node), type->info.enums.enm[i].value,
                                       type->info.enums.enm[i].name);
                                type->info.enums.count = i + 1;
                                goto error;
                            }
                        }
                    }
                } else {
                    LOGVAL(LYE_INSTMT, LOGLINE(next), next->name);
                    goto error;
                }
            }
            if (v_ == -1) {
                /* assign value automatically */
                if (v > INT32_MAX) {
                    LOGVAL(LYE_INARG, LOGLINE(next), "2147483648", "enum/value");
                    type->info.enums.count = i + 1;
                    goto error;
                }
                type->info.enums.enm[i].value = v;
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
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (strcmp(yin->child->name, "base")) {
                LOGVAL(LYE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }

        if (!yin->child) {
            if (type->der->type.der) {
                /* this is just a derived type with no base specified/required */
                break;
            }
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "base", "type");
            goto error;
        }
        if (yin->child->next) {
            LOGVAL(LYE_TOOMANY, LOGLINE(yin->child->next), yin->child->next->name, yin->name);
            goto error;
        }
        GETVAL(value, yin->child, "name");
        if (unres_schema_add_str(module, unres, type, UNRES_TYPE_IDENTREF, value, LOGLINE(yin->child)) == -1) {
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
                    LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }
                GETVAL(value, node, "value");
                if (!strcmp(value, "true")) {
                    type->info.inst.req = 1;
                } else if (!strcmp(value, "false")) {
                    type->info.inst.req = -1;
                } else {
                    LOGVAL(LYE_INARG, LOGLINE(node), value, node->name);
                    goto error;
                }
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
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
                    LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (lyp_check_length_range(value, type)) {
                    LOGVAL(LYE_INARG, LOGLINE(node), value, name);
                    goto error;
                }
                *restr = calloc(1, sizeof **restr);
                (*restr)->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module->ctx, *restr, node)) {
                    goto error;
                }
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }
        break;

    case LY_TYPE_LEAFREF:
        /* RFC 6020 9.9.2 - path */
        LY_TREE_FOR(yin->child, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                continue;
            }

            if (!strcmp(node->name, "path")) {
                if (type->info.lref.path) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                /* store in the JSON format */
                type->info.lref.path = transform_schema2json(module, value, LOGLINE(node));
                if (!type->info.lref.path) {
                    goto error;
                }
                if (unres_schema_add_node(module, unres, type, UNRES_TYPE_LEAFREF, parent, LOGLINE(yin)) == -1) {
                    goto error;
                }

            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }

        if (!type->info.lref.path) {
            if (type->der->type.der) {
                /* this is just a derived type with no path specified/required */
                break;
            }
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "path", "type");
            goto error;
        }
        break;

    case LY_TYPE_STRING:
        /* RFC 6020 9.4.4 - length */
        /* RFC 6020 9.4.6 - pattern */
        i = 0;
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "length")) {
                if (type->info.str.length) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (lyp_check_length_range(value, type)) {
                    LOGVAL(LYE_INARG, LOGLINE(node), value, "length");
                    goto error;
                }
                type->info.str.length = calloc(1, sizeof *type->info.str.length);
                type->info.str.length->expr = lydict_insert(module->ctx, value, 0);

                /* get possible sub-statements */
                if (read_restr_substmt(module->ctx, type->info.str.length, node)) {
                    goto error;
                }
                lyxml_free_elem(module->ctx, node);
            } else if (!strcmp(node->name, "pattern")) {
                i++;
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }
        /* store patterns in array */
        if (i) {
            type->info.str.patterns = calloc(i, sizeof *type->info.str.patterns);
            LY_TREE_FOR(yin->child, node) {
                GETVAL(value, yin->child, "value");

                /* check that the regex is valid */
                precomp = pcre_compile(value, PCRE_NO_AUTO_CAPTURE, &err_ptr, &err_offset, NULL);
                if (!precomp) {
                    LOGVAL(LYE_INREGEX, LOGLINE(node), value, err_ptr);
                    free(type->info.str.patterns);
                    goto error;
                }
                free(precomp);

                type->info.str.patterns[type->info.str.pat_count].expr = lydict_insert(module->ctx, value, 0);

                /* get possible sub-statements */
                if (read_restr_substmt(module->ctx, &type->info.str.patterns[type->info.str.pat_count], yin->child)) {
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
                lyxml_free_elem(module->ctx, node);
                continue;
            }

            if (!strcmp(node->name, "type")) {
                i++;
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
                goto error;
            }
        }

        if (!i) {
            if (type->der->type.der) {
                /* this is just a derived type with no base specified/required */
                break;
            }
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "type", "(union) type");
            goto error;
        }

        /* allocate array for union's types ... */
        type->info.uni.types = calloc(i, sizeof *type->info.uni.types);
        /* ... and fill the structures */
        LY_TREE_FOR(yin->child, node) {
            if (fill_yin_type(module, parent, node, &type->info.uni.types[type->info.uni.count], unres)) {
                goto error;
            }
            type->info.uni.count++;

            /* union's type cannot be empty or leafref */
            if (type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_EMPTY) {
                LOGVAL(LYE_INARG, LOGLINE(node), "empty", node->name);
                goto error;
            } else if (type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_LEAFREF) {
                LOGVAL(LYE_INARG, LOGLINE(node), "leafref", node->name);
                goto error;
            }
        }
        break;

    default:
        /* no sub-statement allowed in:
         * LY_TYPE_BOOL, LY_TYPE_EMPTY
         */
        LY_TREE_FOR(yin->child, node) {
            if (node->ns && !strcmp(node->ns->value, LY_NSYIN)) {
                LOGVAL(LYE_INSTMT, LOGLINE(yin->child), yin->child->name);
                goto error;
            }
        }
        break;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_typedef(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_tpdf *tpdf, struct unres_schema *unres)
{
    const char *value;
    struct lyxml_elem *node;
    int has_type = 0, dflt_line;

    GETVAL(value, yin, "name");
    if (check_identifier(value, LY_IDENT_TYPE, LOGLINE(yin), module, parent)) {
        goto error;
    }
    tpdf->name = lydict_insert(module->ctx, value, strlen(value));

    /* generic part - status, description, reference */
    if (read_yin_common(module, NULL, (struct lys_node *)tpdf, yin, OPT_IDENT | OPT_MODULE)) {
        goto error;
    }

    LY_TREE_FOR(yin->child, node) {
        if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(node->name, "type")) {
            if (tpdf->type.der || has_type) {
                LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            if (fill_yin_type(module, parent, node, &tpdf->type, unres)) {
                goto error;
            }
            has_type = 1;
        } else if (!strcmp(node->name, "default")) {
            if (tpdf->dflt) {
                LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "value");
            tpdf->dflt = lydict_insert(module->ctx, value, strlen(value));
            dflt_line = LOGLINE(node);
        } else if (!strcmp(node->name, "units")) {
            if (tpdf->units) {
                LOGVAL(LYE_TOOMANY, LOGLINE(node), node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "name");
            tpdf->units = lydict_insert(module->ctx, value, strlen(value));
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(node), value);
            goto error;
        }
    }

    /* check mandatory value */
    if (!has_type) {
        LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
        goto error;
    }

    /* check default value */
    if (tpdf->dflt) {
        if (unres_schema_add_str(module, unres, &tpdf->type, UNRES_TYPE_DFLT, tpdf->dflt, dflt_line) == -1) {
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
    if (check_identifier(value, LY_IDENT_FEATURE, LOGLINE(yin), module, NULL)) {
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
            lyxml_free_elem(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "if-feature")) {
            c++;
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    if (c) {
        f->features = calloc(c, sizeof *f->features);
    }
    LY_TREE_FOR(yin->child, child) {
        GETVAL(value, child, "name");
        if (!(value = transform_schema2json(module, value, LOGLINE(child)))) {
            goto error;
        }
        ret = unres_schema_add_str(module, unres, &f->features[f->features_size++], UNRES_IFFEAT, value,
                                   LOGLINE(child));
        lydict_remove(module->ctx, value);
        if (ret == -1) {
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
    must->expr = transform_schema2json(module, value, LOGLINE(yin));
    if (!must->expr) {
        goto error;
    }

    return read_restr_substmt(module->ctx, must, yin);

error: /* GETVAL requires this label */

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
        LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
        LOGVAL(LYE_SPEC, 0, "Target node does not allow \"%s\" property.", node->name);
        goto error;
    }

    GETVAL(value, node, "value");
    while (isspace(value[0])) {
        value++;
    }

    /* convert it to uint32_t */
    errno = 0;
    endptr = NULL;
    val = strtoul(value, &endptr, 10);
    if (*endptr || value[0] == '-' || errno || val > UINT32_MAX) {
        LOGVAL(LYE_INARG, LOGLINE(node), value, node->name);
        goto error;
    }
    if (type) {
        d->max = (uint32_t)val;
    } else {
        d->min = (uint32_t)val;
    }

    if (d->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (*ui32val) {
            LOGVAL(LYE_INSTMT, LOGLINE(node), node->name);
            LOGVAL(LYE_SPEC, 0, "Adding property that already exists.");
            goto error;
        }
    }

    if (d->mod == LY_DEVIATE_DEL) {
        /* check values */
        if ((uint32_t)val != *ui32val) {
            LOGVAL(LYE_INARG, LOGLINE(node), value, node->name);
            LOGVAL(LYE_SPEC, 0, "Value differs from the target being deleted.");
            goto error;
        }
        /* remove current min-elements value of the target */
        *ui32val = 0;
    } else { /* add (already checked) and replace */
        /* set new value specified in deviation */
        *ui32val = (uint32_t)val;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_deviation(struct lys_module *module, struct lyxml_elem *yin, struct lys_deviation *dev)
{
    const char *value, **stritem;
    struct lyxml_elem *next, *child, *develem;
    int c_dev = 0, c_must, c_uniq;
    int f_min = 0; /* flags */
    int i, j, rc;
    struct lys_deviate *d = NULL;
    struct lys_node *node = NULL;
    struct lys_node_choice *choice = NULL;
    struct lys_node_leaf *leaf = NULL;
    struct lys_node_list *list = NULL;
    struct lys_type *t = NULL;
    uint8_t *trg_must_size = NULL;
    struct lys_restr **trg_must = NULL;

    GETVAL(value, yin, "target-node");
    dev->target_name = transform_schema2json(module, value, LOGLINE(yin));
    if (!dev->target_name) {
        goto error;
    }

    /* resolve target node */
    rc = resolve_schema_nodeid(dev->target_name, NULL, module, LYS_AUGMENT, &dev->target);
    if (rc) {
        LOGVAL(LYE_INARG, LOGLINE(yin), dev->target_name, yin->name);
        goto error;
    }
    if (dev->target->module == module) {
        LOGVAL(LYE_SPEC, LOGLINE(yin), "Deviating own module is not allowed.");
        goto error;
    }
    /* mark the target module as deviated */
    dev->target->module->deviated = 1;

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (dev->dsc) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            dev->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!dev->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (dev->ref) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            dev->ref = read_yin_subnode(module->ctx, child, "text");
            if (!dev->ref) {
                goto error;
            }
        } else if (!strcmp(child->name, "deviate")) {
            c_dev++;

            /* skip lyxml_free_elem() at the end of the loop, node will be
             * further processed later
             */
            continue;

        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, child);
    }

    if (c_dev) {
        dev->deviate = calloc(c_dev, sizeof *dev->deviate);
    }

    LY_TREE_FOR(yin->child, develem) {
        /* init */
        f_min = 0;
        c_must = 0;
        c_uniq = 0;

        /* get deviation type */
        GETVAL(value, develem, "value");
        if (!strcmp(value, "not-supported")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_NO;
            /* no property expected in this case */
            if (develem->child) {
                LOGVAL(LYE_INSTMT, LOGLINE(develem->child), develem->child->name);
                goto error;
            }

            /* and neither any other deviate statement is expected,
             * not-supported deviation must be the only deviation of the target
             */
            if (dev->deviate_size || develem->next) {
                LOGVAL(LYE_INARG, LOGLINE(develem), value, develem->name);
                LOGVAL(LYE_SPEC, 0, "\"not-supported\" deviation cannot be combined with any other deviation.");
                goto error;
            }


            /* remove target node */
            lys_node_free(dev->target);
            dev->target = NULL;

            dev->deviate_size = 1;
            return EXIT_SUCCESS;
        } else if (!strcmp(value, "add")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_ADD;
        } else if (!strcmp(value, "replace")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_RPL;
        } else if (!strcmp(value, "delete")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_DEL;
        } else {
            LOGVAL(LYE_INARG, LOGLINE(develem), value, develem->name);
            goto error;
        }
        d = &dev->deviate[dev->deviate_size];

        /* process deviation properties */
        LY_TREE_FOR_SAFE(develem->child, next, child) {
            if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
                /* garbage */
                lyxml_free_elem(module->ctx, child);
                continue;
            }

            if (!strcmp(child->name, "config")) {
                if (d->flags & LYS_CONFIG_MASK) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
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
                    LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_DEL) {
                    /* check values */
                    if ((d->flags & LYS_CONFIG_MASK) != (dev->target->flags & LYS_CONFIG_MASK)) {
                        LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                        LOGVAL(LYE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                    /* remove current config value of the target ... */
                    dev->target->flags &= ~LYS_CONFIG_MASK;

                    /* ... and inherit config value from the target's parent */
                    if (dev->target->parent) {
                        dev->target->flags |= dev->target->parent->flags & LYS_CONFIG_MASK;
                    } else {
                        /* default config is true */
                        dev->target->flags |= LYS_CONFIG_W;
                    }
                } else { /* add and replace are the same in this case */
                    /* remove current config value of the target ... */
                    dev->target->flags &= ~LYS_CONFIG_MASK;

                    /* ... and replace it with the value specified in deviation */
                    dev->target->flags |= d->flags & LYS_CONFIG_MASK;
                }
            } else if (!strcmp(child->name, "default")) {
                if (d->dflt) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }
                GETVAL(value, child, "value");
                d->dflt = lydict_insert(module->ctx, value, 0);

                if (dev->target->nodetype == LYS_CHOICE) {
                    choice = (struct lys_node_choice *)dev->target;

                    if (d->mod == LY_DEVIATE_ADD) {
                        /* check that there is no current value */
                        if (choice->dflt) {
                            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                            LOGVAL(LYE_SPEC, 0, "Adding property that already exists.");
                            goto error;
                        }
                    }

                    rc = resolve_schema_nodeid(d->dflt, choice->child, choice->module, LYS_CHOICE, &node);
                    if (rc) {
                        LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                        goto error;
                    }
                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!choice->dflt || choice->dflt != node) {
                            LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                            LOGVAL(LYE_SPEC, 0, "Value differs from the target being deleted.");
                            goto error;
                        }
                    } else { /* add (already checked) and replace */
                        choice->dflt = node;
                        if (!choice->dflt) {
                            /* default branch not found */
                            LOGVAL(LYE_INARG, LOGLINE(yin), value, "default");
                            goto error;
                        }
                    }
                } else if (dev->target->nodetype == LYS_LEAF) {
                    leaf = (struct lys_node_leaf *)dev->target;

                    if (d->mod == LY_DEVIATE_ADD) {
                        /* check that there is no current value */
                        if (leaf->dflt) {
                            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                            LOGVAL(LYE_SPEC, 0, "Adding property that already exists.");
                            goto error;
                        }
                    }

                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!leaf->dflt || leaf->dflt != d->dflt) {
                            LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                            LOGVAL(LYE_SPEC, 0, "Value differs from the target being deleted.");
                            goto error;
                        }
                        /* remove value */
                        lydict_remove(leaf->module->ctx, leaf->dflt);
                        leaf->dflt = NULL;
                    } else { /* add (already checked) and replace */
                        /* remove value */
                        lydict_remove(leaf->module->ctx, leaf->dflt);

                        /* set new value */
                        leaf->dflt = lydict_insert(leaf->module->ctx, d->dflt, 0);
                    }
                } else {
                    /* invalid target for default value */
                    LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(LYE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }
            } else if (!strcmp(child->name, "mandatory")) {
                if (d->flags & LYS_MAND_MASK) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (!(dev->target->nodetype &= (LYS_LEAF | LYS_CHOICE | LYS_ANYXML))) {
                    LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(LYE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                GETVAL(value, child, "value");
                if (!strcmp(value, "false")) {
                    d->flags |= LYS_MAND_FALSE;
                } else if (!strcmp(value, "true")) {
                    d->flags |= LYS_MAND_TRUE;
                } else {
                    LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_ADD) {
                    /* check that there is no current value */
                    if (dev->target->flags & LYS_MAND_MASK) {
                        LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                        LOGVAL(LYE_SPEC, 0, "Adding property that already exists.");
                        goto error;
                    }
                }

                if (d->mod == LY_DEVIATE_DEL) {
                    /* check values */
                    if ((d->flags & LYS_MAND_MASK) != (dev->target->flags & LYS_MAND_MASK)) {
                        LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                        LOGVAL(LYE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                    /* remove current mandatory value of the target */
                    dev->target->flags &= ~LYS_MAND_MASK;
                } else { /* add (already checked) and replace */
                    /* remove current mandatory value of the target ... */
                    dev->target->flags &= ~LYS_MAND_MASK;

                    /* ... and replace it with the value specified in deviation */
                    dev->target->flags |= d->flags & LYS_MAND_MASK;
                }
            } else if (!strcmp(child->name, "min-elements")) {
                if (f_min) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }
                f_min = 1;

                if (deviate_minmax(dev->target, child, d, 0)) {
                    goto error;
                }
            } else if (!strcmp(child->name, "max-elements")) {
                if (d->max) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                if (deviate_minmax(dev->target, child, d, 1)) {
                    goto error;
                }
            } else if (!strcmp(child->name, "must")) {
                c_must++;
                /* skip lyxml_free_elem() at the end of the loop, this node will be processed later */
                continue;
            } else if (!strcmp(child->name, "type")) {
                if (d->type) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (dev->target->nodetype == LYS_LEAF) {
                    t = &((struct lys_node_leaf *)dev->target)->type;
                } else if (dev->target->nodetype == LYS_LEAFLIST) {
                    t = &((struct lys_node_leaflist *)dev->target)->type;
                } else {
                    LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(LYE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                if (d->mod == LY_DEVIATE_ADD) {
                    /* not allowed, type is always present at the target */
                    LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(LYE_SPEC, 0, "Adding property that already exists.");
                    goto error;
                } else if (d->mod == LY_DEVIATE_DEL) {
                    /* not allowed, type cannot be deleted from the target */
                    LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                    LOGVAL(LYE_SPEC, 0, "Deleteing type from the target is not allowed.");
                    goto error;
                }

                /* replace */
                /* remove current units value of the target ... */
                lys_type_free(dev->target->module->ctx, t);

                /* ... and replace it with the value specified in deviation */
                if (fill_yin_type(module, dev->target, child, t, NULL)) {
                    goto error;
                }
                d->type = t;
            } else if (!strcmp(child->name, "unique")) {
                c_uniq++;
                /* skip lyxml_free_elem() at the end of the loop, this node will be processed later */
                continue;
            } else if (!strcmp(child->name, "units")) {
                if (d->units) {
                    LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (dev->target->nodetype == LYS_LEAFLIST) {
                    stritem = &((struct lys_node_leaflist *)dev->target)->units;
                } else if (dev->target->nodetype == LYS_LEAF) {
                    stritem = &((struct lys_node_leaf *)dev->target)->units;
                } else {
                    LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                    LOGVAL(LYE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                /* get units value */
                GETVAL(value, child, "name");
                d->units = lydict_insert(module->ctx, value, 0);

                /* apply to target */
                if (d->mod == LY_DEVIATE_ADD) {
                    /* check that there is no current value */
                    if (*stritem) {
                        LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                        LOGVAL(LYE_SPEC, 0, "Adding property that already exists.");
                        goto error;
                    }
                }

                if (d->mod == LY_DEVIATE_DEL) {
                    /* check values */
                    if (*stritem != d->units) {
                        LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                        LOGVAL(LYE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                    /* remove current units value of the target */
                    lydict_remove(dev->target->module->ctx, *stritem);
                } else { /* add (already checked) and replace */
                    /* remove current units value of the target ... */
                    lydict_remove(dev->target->module->ctx, *stritem);

                    /* ... and replace it with the value specified in deviation */
                    *stritem = lydict_insert(module->ctx, value, 0);
                }
            } else {
                LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                goto error;
            }

            lyxml_free_elem(module->ctx, child);
        }

        if (c_must) {
            /* check target node type */
            switch (dev->target->nodetype) {
            case LYS_LEAF:
                trg_must = &((struct lys_node_leaf *)dev->target)->must;
                trg_must_size = &((struct lys_node_leaf *)dev->target)->must_size;
                break;
            case LYS_CONTAINER:
                trg_must = &((struct lys_node_container *)dev->target)->must;
                trg_must_size = &((struct lys_node_container *)dev->target)->must_size;
                break;
            case LYS_LEAFLIST:
                trg_must = &((struct lys_node_leaflist *)dev->target)->must;
                trg_must_size = &((struct lys_node_leaflist *)dev->target)->must_size;
                break;
            case LYS_LIST:
                trg_must = &((struct lys_node_list *)dev->target)->must;
                trg_must_size = &((struct lys_node_list *)dev->target)->must_size;
                break;
            case LYS_ANYXML:
                trg_must = &((struct lys_node_anyxml *)dev->target)->must;
                trg_must_size = &((struct lys_node_anyxml *)dev->target)->must_size;
                break;
            default:
                LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                LOGVAL(LYE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                goto error;
            }

            if (d->mod == LY_DEVIATE_RPL) {
                /* remove target's musts and allocate new array for it */
                if (!*trg_must) {
                    LOGVAL(LYE_INARG, LOGLINE(develem), "replace", "deviate");
                    LOGVAL(LYE_SPEC, 0, "Property \"must\" to replace does not exists in target.");
                    goto error;
                }

                for (i = 0; i < list->must_size; i++) {
                    lys_restr_free(dev->target->module->ctx, &(*trg_must[i]));
                }
                free(*trg_must);
                *trg_must = d->must = calloc(c_must, sizeof *d->must);
                d->must_size = c_must;
                *trg_must_size = 0;
            } else if (d->mod == LY_DEVIATE_ADD) {
                /* reallocate the must array of the target */
                d->must = realloc(*trg_must, (c_must + *trg_must_size) * sizeof *d->must);
                *trg_must = d->must;
                d->must = &((*trg_must)[*trg_must_size]);
                d->must_size = c_must;
            } else { /* LY_DEVIATE_DEL */
                d->must = calloc(c_must, sizeof *d->must);
            }
        }
        if (c_uniq) {
            /* check target node type */
            if (dev->target->nodetype != LYS_LIST) {
                LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
                LOGVAL(LYE_SPEC, 0, "Target node does not allow \"%s\" property.", child->name);
                goto error;
            }

            list = (struct lys_node_list *)dev->target;
            if (d->mod == LY_DEVIATE_RPL) {
                /* remove target's unique and allocate new array for it */
                if (!list->unique) {
                    LOGVAL(LYE_INARG, LOGLINE(develem), "replace", "deviate");
                    LOGVAL(LYE_SPEC, 0, "Property \"unique\" to replace does not exists in target.");
                    goto error;
                }

                for (i = 0; i < list->unique_size; i++) {
                    free(list->unique[i].leafs);
                }
                free(list->unique);
                list->unique = d->unique = calloc(c_uniq, sizeof *d->unique);
                d->unique_size = c_uniq;
                list->unique_size = 0;
            } else if (d->mod == LY_DEVIATE_ADD) {
                /* reallocate the unique array of the target */
                d->unique = realloc(list->unique, (c_uniq + list->unique_size) * sizeof *d->unique);
                list->unique = d->unique;
                d->unique = &list->unique[list->unique_size];
                d->unique_size = c_uniq;
            } else { /* LY_DEVIATE_DEL */
                d->unique = calloc(c_uniq, sizeof *d->unique);
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
                        if (d->must[d->must_size].expr == (*trg_must)[i].expr) {
                            /* we have a match, free the must structure ... */
                            lys_restr_free(dev->target->module->ctx, &((*trg_must)[i]));
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
                        LOGVAL(LYE_INARG, LOGLINE(child), d->must[d->must_size - 1].expr, child->name);
                        LOGVAL(LYE_SPEC, 0, "Value does not match any must from the target.");
                        goto error;
                    }
                } else { /* replace or add */
                    if (fill_yin_must(dev->target->module, child, &((*trg_must)[*trg_must_size]))) {
                        goto error;
                    }
                    (*trg_must_size)++;
                }
            } else if (!strcmp(child->name, "unique")) {
                if (d->mod == LY_DEVIATE_DEL) {
                    GETVAL(value, child, "tag");
                    if (resolve_unique(dev->target, value, &d->unique[d->unique_size], 0, LOGLINE(child))) {
                        goto error;
                    }

                    /* find unique structures to delete */
                    for (i = 0; i < list->unique_size; i++) {
                        if (list->unique[i].leafs_size != d->unique[d->unique_size].leafs_size) {
                            continue;
                        }

                        for (j = 0; j < d->unique[d->unique_size].leafs_size; j++) {
                            if (list->unique[i].leafs[j] != d->unique[d->unique_size].leafs[j]) {
                                break;
                            }
                        }

                        if (j == d->unique[d->unique_size].leafs_size) {
                            /* we have a match, free the unique structure ... */
                            free(list->unique[i].leafs);
                            /* ... and maintain the array */
                            list->unique_size--;
                            if (i != list->unique_size) {
                                list->unique[i].leafs_size = list->unique[list->unique_size].leafs_size;
                                list->unique[i].leafs = list->unique[list->unique_size].leafs;
                            }

                            if (!list->unique_size) {
                                free(list->unique);
                                list->unique = NULL;
                            } else {
                                list->unique[list->unique_size].leafs_size = 0;
                                list->unique[list->unique_size].leafs = NULL;
                            }

                            i = -1; /* set match flag */
                            break;
                        }
                    }

                    d->unique_size++;
                    if (i != -1) {
                        /* no match found */
                        LOGVAL(LYE_INARG, LOGLINE(child), lyxml_get_attr(child, "tag", NULL), child->name);
                        LOGVAL(LYE_SPEC, 0, "Value differs from the target being deleted.");
                        goto error;
                    }
                } else { /* replace or add */
                    GETVAL(value, child, "tag");
                    if (resolve_unique(dev->target, value, &list->unique[list->unique_size], 0, LOGLINE(child))) {
                        goto error;
                    }
                    list->unique_size++;
                }
            }
        }

        dev->deviate_size++;
    }

    return EXIT_SUCCESS;

error:

    if (dev->deviate) {
        for (i = 0; i < dev->deviate_size; i++) {
            lydict_remove(module->ctx, dev->deviate[i].dflt);
            lydict_remove(module->ctx, dev->deviate[i].units);

            if (dev->deviate[i].mod == LY_DEVIATE_DEL) {
                for (j = 0; j < dev->deviate[i].must_size; j++) {
                    lys_restr_free(module->ctx, &dev->deviate[i].must[j]);
                }
                free(dev->deviate[i].must);

                for (j = 0; j < dev->deviate[i].unique_size; j++) {
                    free(dev->deviate[i].unique[j].leafs);
                }
                free(dev->deviate[i].unique);
            }
        }
        free(dev->deviate);
    }

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
    aug->target_name = transform_schema2json(module, value, LOGLINE(yin));
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
            lyxml_free_elem(module->ctx, child);
            continue;
        }

        if (!strcmp(child->name, "if-feature")) {
            c++;
            continue;
        } else if (!strcmp(child->name, "when")) {
            if (aug->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }

            aug->when = read_yin_when(module, child);
            if (!aug->when) {
                lyxml_free_elem(module->ctx, child);
                goto error;
            }
            if (lyxp_syntax_check(aug->when->cond, LOGLINE(child))) {
                goto error;
            }
            lyxml_free_elem(module->ctx, child);
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
            node = read_yin_uses(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "choice")) {
            node = read_yin_case(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "case")) {
            node = read_yin_case(module, (struct lys_node *)aug, child, 0, unres);
        } else if (!strcmp(child->name, "anyxml")) {
            node = read_yin_anyxml(module, (struct lys_node *)aug, child, 0, unres);
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }

        if (!node) {
            goto error;
        }

        /* check for mandatory nodes - if the target node is in another module
         * the added nodes cannot be mandatory
         */
        if ((!parent || (parent->nodetype != LYS_USES)) && check_mandatory(node)) {
            LOGVAL(LYE_SPEC, LOGLINE(child), "When augmenting data in another module, mandatory statement is not allowed.");
            goto error;
        }

        node = NULL;
        lyxml_free_elem(module->ctx, child);
    }

    if (c) {
        aug->features = calloc(c, sizeof *aug->features);
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!strcmp(child->name, "if-feature")) {
            GETVAL(value, child, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(child)))) {
                goto error;
            }
            ret = unres_schema_add_str(module, unres, &aug->features[aug->features_size++], UNRES_IFFEAT, value,
                                       LOGLINE(child));
            lydict_remove(module->ctx, value);
            if (ret == -1) {
                goto error;
            }
            lyxml_free_elem(module->ctx, child);
        }
    }

    /* aug->child points to the parsed nodes, they must now be
     * connected to the tree and adjusted (if possible right now).
     * However, if this is augment in a uses, it gets resolved
     * when the uses does and cannot be resolved now for sure
     * (the grouping was not yet copied into uses).
     */
    if (!parent || (parent->nodetype != LYS_USES)) {
        if (resolve_augment(aug, aug->child)) {
            LOGVAL(LYE_INRESOLV, LOGLINE(yin), "augment", aug->target_name);
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
    rfn->target_name = transform_schema2json(module, value, LOGLINE(yin));
    if (!rfn->target_name) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        /* limited applicability */
        if (!strcmp(sub->name, "default")) {
            /* leaf or choice */
            if (rfn->mod.dflt) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LEAF | LYS_CHOICE);
                if (!rfn->target_type) {
                    LOGVAL(LYE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
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
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                    LOGVAL(LYE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
        } else if (!strcmp(sub->name, "min-elements")) {
            /* list or leaf-list */
            if (f_min) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_min = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LIST | LYS_LEAFLIST);
                if (!rfn->target_type) {
                    LOGVAL(LYE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            rfn->mod.list.min = (uint32_t) val;

            /* magic - bit 3 in flags means min set */
            rfn->flags |= 0x04;
        } else if (!strcmp(sub->name, "max-elements")) {
            /* list or leaf-list */
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LIST | LYS_LEAFLIST);
                if (!rfn->target_type) {
                    LOGVAL(LYE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
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
            if (*endptr || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            rfn->mod.list.max = (uint32_t) val;

            /* magic - bit 4 in flags means min set */
            rfn->flags |= 0x08;
        } else if (!strcmp(sub->name, "presence")) {
            /* container */
            if (rfn->mod.presence) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= LYS_CONTAINER;
                if (!rfn->target_type) {
                    LOGVAL(LYE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
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
                rfn->target_type &= (LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML);
                if (!rfn->target_type) {
                    LOGVAL(LYE_SPEC, LOGLINE(sub), "invalid combination of refine substatements");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
            }

            c_must++;
            continue;

        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    /* process nodes with cardinality of 0..n */
    if (c_must) {
        rfn->must = calloc(c_must, sizeof *rfn->must);
    }
    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_must(module, sub, &rfn->must[rfn->must_size++]);
        if (r) {
            goto error;
        }
        if (lyxp_syntax_check(rfn->must[rfn->must_size-1].expr, LOGLINE(sub))) {
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
    int count;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "prefix")) {
            GETVAL(value, child, "value");
            if (check_identifier(value, LY_IDENT_PREFIX, LOGLINE(child), module, NULL)) {
                goto error;
            }
            imp->prefix = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(child->name, "revision-date")) {
            if (imp->rev[0]) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), "revision-date", yin->name);
                goto error;
            }
            GETVAL(value, child, "date");
            if (check_date(value, LOGLINE(child))) {
                goto error;
            }
            memcpy(imp->rev, value, LY_REV_SIZE - 1);
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    /* check mandatory information */
    if (!imp->prefix) {
        LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "prefix", yin->name);
        goto error;
    }

    GETVAL(value, yin, "module");

    /* check for circular import, store it if passed */
    if (!module->ctx->models.parsing) {
        count = 0;
    } else {
        for (count = 0; module->ctx->models.parsing[count]; ++count) {
            if (value == module->ctx->models.parsing[count]) {
                LOGERR(LY_EVALID, "Circular import dependency on the module \"%s\".", value);
                goto error;
            }
        }
    }
    ++count;
    module->ctx->models.parsing =
        realloc(module->ctx->models.parsing, (count + 1) * sizeof *module->ctx->models.parsing);
    module->ctx->models.parsing[count - 1] = value;
    module->ctx->models.parsing[count] = NULL;

    /* try to load the module */
    imp->module = ly_ctx_get_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
    if (!imp->module) {
        imp->module = lyp_search_file(module->ctx, NULL, value, imp->rev[0] ? imp->rev : NULL);
    }

    /* remove the new module name now that its parsing is finished (even if failed) */
    if (module->ctx->models.parsing[count] || (module->ctx->models.parsing[count - 1] != value)) {
        LOGINT;
    }
    --count;
    if (count) {
        module->ctx->models.parsing[count] = NULL;
    } else {
        free(module->ctx->models.parsing);
        module->ctx->models.parsing = NULL;
    }

    /* check the result */
    if (!imp->module) {
        LOGVAL(LYE_INARG, LOGLINE(yin), value, yin->name);
        LOGERR(LY_EVALID, "Importing \"%s\" module into \"%s\" failed.", value, module->name);
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_include(struct lys_module *module, struct lyxml_elem *yin, struct lys_include *inc)
{
    struct lyxml_elem *child;
    const char *value;
    int count;

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }
        if (!strcmp(child->name, "revision-date")) {
            if (inc->rev[0]) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), "revision-date", yin->name);
                goto error;
            }
            GETVAL(value, child, "date");
            if (check_date(value, LOGLINE(child))) {
                goto error;
            }
            memcpy(inc->rev, value, LY_REV_SIZE - 1);
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    GETVAL(value, yin, "module");

    /* check for circular include, store it if passed */
    if (!module->ctx->models.parsing) {
        count = 0;
    } else {
        for (count = 0; module->ctx->models.parsing[count]; ++count) {
            if (value == module->ctx->models.parsing[count]) {
                LOGERR(LY_EVALID, "Circular include dependency on the submodule \"%s\".", value);
                goto error;
            }
        }
    }
    ++count;
    module->ctx->models.parsing =
        realloc(module->ctx->models.parsing, (count + 1) * sizeof *module->ctx->models.parsing);
    module->ctx->models.parsing[count - 1] = value;
    module->ctx->models.parsing[count] = NULL;

    /* try to load the submodule */
    inc->submodule = ly_ctx_get_submodule(module, value, inc->rev[0] ? inc->rev : NULL);
    if (!inc->submodule) {
        inc->submodule = (struct lys_submodule *)lyp_search_file(module->ctx, module, value, inc->rev[0] ? inc->rev : NULL);
    }

    /* remove the new submodule name now that its parsing is finished (even if failed) */
    if (module->ctx->models.parsing[count] || (module->ctx->models.parsing[count - 1] != value)) {
        LOGINT;
    }
    --count;
    if (count) {
        module->ctx->models.parsing[count] = NULL;
    } else {
        free(module->ctx->models.parsing);
        module->ctx->models.parsing = NULL;
    }

    /* check the result */
    if (!inc->submodule) {
        LOGVAL(LYE_INARG, LOGLINE(yin), value, yin->name);
        LOGERR(LY_EVALID, "Including \"%s\" module into \"%s\" failed.", value, module->name);
        goto error;
    }

    /* check that belongs-to corresponds */
    if (module->type) {
        module = ((struct lys_submodule *)module)->belongsto;
    }
    if (inc->submodule->belongsto != module) {
        LOGVAL(LYE_INARG, LOGLINE(yin), value, yin->name);
        LOGERR(LY_EVALID, "The included module does not belongs-to the \"%s\" module", module->name);
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
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
        if (check_identifier(value, LY_IDENT_NAME, LOGLINE(xmlnode), NULL, NULL)) {
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
            lyxml_free_elem(ctx, sub);
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
                    LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
                    goto error;
                }
            }

            /* else garbage */
            lyxml_free_elem(ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "description")) {
            if (node->dsc) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
                goto error;
            }
            node->dsc = read_yin_subnode(ctx, sub, "text");
            if (!node->dsc) {
                goto error;
            }
        } else if (!strcmp(sub->name, "reference")) {
            if (node->ref) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
                goto error;
            }
            node->ref = read_yin_subnode(ctx, sub, "text");
            if (!node->ref) {
                goto error;
            }
        } else if (!strcmp(sub->name, "status")) {
            if (node->flags & LYS_STATUS_MASK) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
        } else if ((opt & OPT_CONFIG) && !strcmp(sub->name, "config")) {
            if (node->flags & LYS_CONFIG_MASK) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, xmlnode->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            if (!strcmp(value, "false")) {
                node->flags |= LYS_CONFIG_R;
            } else if (!strcmp(value, "true")) {
                node->flags |= LYS_CONFIG_W;
            } else {
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
        } else {
            /* skip the lyxml_free_elem */
            continue;
        }
        lyxml_free_elem(ctx, sub);
    }

    if ((opt & OPT_INHERIT) && !(node->flags & LYS_CONFIG_MASK)) {
        /* get config flag from parent */
        if (parent) {
            node->flags |= parent->flags & LYS_CONFIG_MASK;
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

    GETVAL(value, yin, "condition");
    retval->cond = transform_schema2json(module, value, LOGLINE(yin));
    if (!retval->cond) {
        goto error;
    }

    LY_TREE_FOR(yin->child, child) {
        if (!child->ns || strcmp(child->ns->value, LY_NSYIN)) {
            /* garbage */
            continue;
        }

        if (!strcmp(child->name, "description")) {
            if (retval->dsc) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            retval->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!retval->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (retval->ref) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            retval->ref = read_yin_subnode(module->ctx, child, "text");
            if (!retval->ref) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
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
    const char *value;

    /* init */
    memset(&root, 0, sizeof root);

    cs = calloc(1, sizeof *cs);
    cs->nodetype = LYS_CASE;
    cs->prev = (struct lys_node *)cs;
    retval = (struct lys_node *)cs;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_INHERIT | OPT_NACMEXT)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, module, retval)) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
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
            /* skip lyxml_free_elem() at the end of the loop, sub is processed later */
            continue;
        } else if (!strcmp(sub->name, "when")) {
            if (cs->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            cs->when = read_yin_when(module, sub);
            if (!cs->when) {
                goto error;
            }
            if (lyxp_syntax_check(cs->when->cond, LOGLINE(sub))) {
                goto error;
            }

            lyxml_free_elem(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    if (c_ftrs) {
        cs->features = calloc(c_ftrs, sizeof *cs->features);
    }
    LY_TREE_FOR(yin->child, sub) {
        GETVAL(value, sub, "name");
        if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
            goto error;
        }
        ret = unres_schema_add_str(module, unres, &cs->features[cs->features_size++], UNRES_IFFEAT, value,
                                   LOGLINE(sub));
        lydict_remove(module->ctx, value);
        if (ret == -1) {
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
            node = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }
    lys_node_free(retval);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_choice(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve, struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next;
    struct ly_ctx *const ctx = module->ctx;
    struct lys_node *retval, *node = NULL;
    struct lys_node_choice *choice;
    const char *value, *dflt_str = NULL;
    int f_mand = 0, c_ftrs = 0, ret;

    choice = calloc(1, sizeof *choice);
    choice->nodetype = LYS_CHOICE;
    choice->prev = (struct lys_node *)choice;
    retval = (struct lys_node *)choice;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, module, retval)) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
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
            if (dflt_str) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(dflt_str, sub, "value");
        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */
        } else if (!strcmp(sub->name, "when")) {
            if (choice->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            choice->when = read_yin_when(module, sub);
            if (!choice->when) {
                goto error;
            }
            if (lyxp_syntax_check(choice->when->cond, LOGLINE(sub))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

            /* skip lyxml_free_elem() at the end of the loop, the sub node is processed later */
            continue;
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        node = NULL;
        lyxml_free_elem(ctx, sub);
    }

    if (c_ftrs) {
        choice->features = calloc(c_ftrs, sizeof *choice->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        GETVAL(value, sub, "name");
        if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
            goto error;
        }
        ret = unres_schema_add_str(module, unres, &choice->features[choice->features_size++], UNRES_IFFEAT, value,
                                   LOGLINE(sub));
        lydict_remove(module->ctx, value);
        if (ret == -1) {
            goto error;
        }
    }

    /* check - default is prohibited in combination with mandatory */
    if (dflt_str && (choice->flags & LYS_MAND_TRUE)) {
        LOGVAL(LYE_SPEC, LOGLINE(yin),
               "The \"default\" statement MUST NOT be present on choices where \"mandatory\" is true.");
        goto error;
    }

    /* link default with the case */
    if (dflt_str) {
        if (unres_schema_add_str(module, unres, choice, UNRES_CHOICE_DFLT, dflt_str, LOGLINE(yin)) == -1) {
            goto error;
        }
    }

    return retval;

error:

    lys_node_free(retval);

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
    anyxml->nodetype = LYS_ANYXML;
    anyxml->prev = (struct lys_node *)anyxml;
    retval = (struct lys_node *)anyxml;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            /* else false is the default value, so we can ignore it */
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (anyxml->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            anyxml->when = read_yin_when(module, sub);
            if (!anyxml->when) {
                lyxml_free_elem(module->ctx, sub);
                goto error;
            }
            if (lyxp_syntax_check(anyxml->when->cond, LOGLINE(sub))) {
                goto error;
            }
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        anyxml->must = calloc(c_must, sizeof *anyxml->must);
    }
    if (c_ftrs) {
        anyxml->features = calloc(c_ftrs, sizeof *anyxml->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &anyxml->must[anyxml->must_size++]);
            if (r) {
                goto error;
            }
            if (lyxp_syntax_check(anyxml->must[anyxml->must_size-1].expr, LOGLINE(sub))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &anyxml->features[anyxml->features_size++], UNRES_IFFEAT, value,
                                     LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        }
    }

    return retval;

error:

    lys_node_free(retval);

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
    int r, has_type = 0, dflt_line;
    int c_must = 0, c_ftrs = 0, f_mand = 0;

    leaf = calloc(1, sizeof *leaf);
    leaf->nodetype = LYS_LEAF;
    leaf->prev = (struct lys_node *)leaf;
    retval = (struct lys_node *)leaf;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "type")) {
            if (leaf->type.der || has_type) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            if (fill_yin_type(module, parent, sub, &leaf->type, unres)) {
                goto error;
            }
            has_type = 1;
        } else if (!strcmp(sub->name, "default")) {
            if (leaf->dflt) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            leaf->dflt = lydict_insert(module->ctx, value, strlen(value));
            dflt_line = LOGLINE(sub);
        } else if (!strcmp(sub->name, "units")) {
            if (leaf->units) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            leaf->units = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */
        } else if (!strcmp(sub->name, "when")) {
            if (leaf->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            leaf->when = read_yin_when(module, sub);
            if (!leaf->when) {
                goto error;
            }
            if (lyxp_syntax_check(leaf->when->cond, LOGLINE(sub))) {
                goto error;
            }

        } else if (!strcmp(sub->name, "must")) {
            c_must++;
            continue;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            continue;

        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    /* check mandatory parameters */
    if (!has_type) {
        LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
        goto error;
    }
    if (leaf->dflt) {
        if (unres_schema_add_str(module, unres, &leaf->type, UNRES_TYPE_DFLT, leaf->dflt, dflt_line) == -1) {
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        leaf->must = calloc(c_must, sizeof *leaf->must);
    }
    if (c_ftrs) {
        leaf->features = calloc(c_ftrs, sizeof *leaf->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &leaf->must[leaf->must_size++]);
            if (r) {
                goto error;
            }
            if (lyxp_syntax_check(leaf->must[leaf->must_size-1].expr, LOGLINE(sub))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &leaf->features[leaf->features_size++], UNRES_IFFEAT, value,
                                     LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        }
    }

    return retval;

error:

    lys_node_free(retval);

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
    llist->nodetype = LYS_LEAFLIST;
    llist->prev = (struct lys_node *)llist;
    retval = (struct lys_node *)llist;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "type")) {
            if (llist->type.der || has_type) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            if (fill_yin_type(module, parent, sub, &llist->type, unres)) {
                goto error;
            }
            has_type = 1;
        } else if (!strcmp(sub->name, "units")) {
            if (llist->units) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            llist->units = lydict_insert(module->ctx, value, strlen(value));
        } else if (!strcmp(sub->name, "ordered-by")) {
            if (f_ordr) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                lyxml_free_elem(module->ctx, sub);
                continue;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "user")) {
                llist->flags |= LYS_USERORDERED;
            } else if (strcmp(value, "system")) {
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
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
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            llist->min = (uint32_t) val;
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            endptr = NULL;
            val = strtoul(value, &endptr, 10);
            if (*endptr || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            llist->max = (uint32_t) val;
        } else if (!strcmp(sub->name, "when")) {
            if (llist->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            llist->when = read_yin_when(module, sub);
            if (!llist->when) {
                goto error;
            }
            if (lyxp_syntax_check(llist->when->cond, LOGLINE(sub))) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    /* check constraints */
    if (!has_type) {
        LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "type", yin->name);
        goto error;
    }
    if (llist->max && llist->min > llist->max) {
        LOGVAL(LYE_SPEC, LOGLINE(yin), "\"min-elements\" is bigger than \"max-elements\".");
        goto error;
    }

    /* middle part - process nodes with cardinality of 0..n */
    if (c_must) {
        llist->must = calloc(c_must, sizeof *llist->must);
    }
    if (c_ftrs) {
        llist->features = calloc(c_ftrs, sizeof *llist->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &llist->must[llist->must_size++]);
            if (r) {
                goto error;
            }
            if (lyxp_syntax_check(llist->must[llist->must_size-1].expr, LOGLINE(sub))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &llist->features[llist->features_size++], UNRES_IFFEAT, value,
                                     LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        }
    }

    return retval;

error:

    lys_node_free(retval);

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
    int r, key_line;
    int c_tpdf = 0, c_must = 0, c_uniq = 0, c_ftrs = 0;
    int f_ordr = 0, f_max = 0, f_min = 0;
    const char *key_str = NULL, *value;
    char *auxs;
    unsigned long val;

    /* init */
    memset(&root, 0, sizeof root);
    memset(&uniq, 0, sizeof uniq);

    list = calloc(1, sizeof *list);
    list->nodetype = LYS_LIST;
    list->prev = (struct lys_node *)list;
    retval = (struct lys_node *)list;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* process list's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
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
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, list->name);
                goto error;
            }

            /* count the number of keys */
            GETVAL(value, sub, "value");
            key_str = value;
            key_line = LOGLINE(sub);
            while ((value = strpbrk(value, " \t\n"))) {
                list->keys_size++;
                while (isspace(*value)) {
                    value++;
                }
            }
            list->keys_size++;
            list->keys = calloc(list->keys_size, sizeof *list->keys);
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
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                lyxml_free_elem(module->ctx, sub);
                continue;
            }

            GETVAL(value, sub, "value");
            if (!strcmp(value, "user")) {
                list->flags |= LYS_USERORDERED;
            } else if (strcmp(value, "system")) {
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            /* else system is the default value, so we can ignore it */
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "min-elements")) {
            if (f_min) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            list->min = (uint32_t) val;
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            f_max = 1;

            GETVAL(value, sub, "value");
            while (isspace(value[0])) {
                value++;
            }

            /* convert it to uint32_t */
            errno = 0;
            auxs = NULL;
            val = strtoul(value, &auxs, 10);
            if (*auxs || value[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                LOGVAL(LYE_INARG, LOGLINE(sub), value, sub->name);
                goto error;
            }
            list->max = (uint32_t) val;
            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (list->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            list->when = read_yin_when(module, sub);
            if (!list->when) {
                lyxml_free_elem(module->ctx, sub);
                goto error;
            }
            if (lyxp_syntax_check(list->when->cond, LOGLINE(sub))) {
                goto error;
            }
            lyxml_free_elem(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* check - if list is configuration, key statement is mandatory */
    if ((list->flags & LYS_CONFIG_W) && !key_str) {
        LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "key", "list");
        goto error;
    }
    if (list->max && list->min > list->max) {
        LOGVAL(LYE_SPEC, LOGLINE(yin), "\"min-elements\" is bigger than \"max-elements\".");
        goto error;
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        list->tpdf = calloc(c_tpdf, sizeof *list->tpdf);
    }
    if (c_must) {
        list->must = calloc(c_must, sizeof *list->must);
    }
    if (c_ftrs) {
        list->features = calloc(c_ftrs, sizeof *list->features);
    }
    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &list->tpdf[list->tpdf_size++], unres);

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &list->features[list->features_size++], UNRES_IFFEAT, value,
                                       LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &list->must[list->must_size++]);
            if (r) {
                goto error;
            }
            if (lyxp_syntax_check(list->must[list->must_size-1].expr, LOGLINE(sub))) {
                goto error;
            }
        }
    }

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
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
            node = read_yin_uses(module, retval, sub, resolve, unres);
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

        lyxml_free_elem(module->ctx, sub);
    }

    if (!key_str) {
        /* config false list without a key */
        return retval;
    }
    if (unres_schema_add_str(module, unres, list, UNRES_LIST_KEYS, key_str, key_line) == -1) {
        goto error;
    }

    /* process unique statements */
    if (c_uniq) {
        list->unique = calloc(c_uniq, sizeof *list->unique);
    }
    LY_TREE_FOR_SAFE(uniq.child, next, sub) {
        /* HACK for unres */
        list->unique[list->unique_size++].leafs = (struct lys_node_leaf **)list;
        GETVAL(value, sub, "tag");
        if (unres_schema_add_str(module, unres, &list->unique[list->unique_size-1], UNRES_LIST_UNIQ, value,
                          LOGLINE(sub)) == -1) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }
    while (uniq.child) {
        lyxml_free_elem(module->ctx, uniq.child);
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
    cont->nodetype = LYS_CONTAINER;
    cont->prev = (struct lys_node *)cont;
    retval = (struct lys_node *)cont;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_CONFIG
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* process container's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "presence")) {
            if (cont->presence) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            cont->presence = lydict_insert(module->ctx, value, strlen(value));

            lyxml_free_elem(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (cont->when) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            cont->when = read_yin_when(module, sub);
            if (!cont->when) {
                lyxml_free_elem(module->ctx, sub);
                goto error;
            }
            if (lyxp_syntax_check(cont->when->cond, LOGLINE(sub))) {
                goto error;
            }
            lyxml_free_elem(module->ctx, sub);

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
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        cont->tpdf = calloc(c_tpdf, sizeof *cont->tpdf);
    }
    if (c_must) {
        cont->must = calloc(c_must, sizeof *cont->must);
    }
    if (c_ftrs) {
        cont->features = calloc(c_ftrs, sizeof *cont->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &cont->tpdf[cont->tpdf_size++], unres);
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &cont->must[cont->must_size++]);
            if (r) {
                goto error;
            }
            if (lyxp_syntax_check(cont->must[cont->must_size-1].expr, LOGLINE(sub))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &cont->features[cont->features_size++], UNRES_IFFEAT, value,
                                       LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        }
    }

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
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
            node = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
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
    grp->nodetype = LYS_GROUPING;
    grp->prev = (struct lys_node *)grp;
    retval = (struct lys_node *)grp;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
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
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        grp->tpdf = calloc(c_tpdf, sizeof *grp->tpdf);
    }
    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_typedef(module, retval, sub, &grp->tpdf[grp->tpdf_size++], unres);

        if (r) {
            goto error;
        }
    }

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
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
            node = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
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
    inout->prev = (struct lys_node *)inout;

    if (!strcmp(yin->name, "input")) {
        inout->nodetype = LYS_INPUT;
    } else if (!strcmp(yin->name, "output")) {
        inout->nodetype = LYS_OUTPUT;
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

    /* data statements */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
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
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        inout->tpdf = calloc(c_tpdf, sizeof *inout->tpdf);
    }

    LY_TREE_FOR(yin->child, sub) {
        r = fill_yin_typedef(module, retval, sub, &inout->tpdf[inout->tpdf_size++], unres);

        if (r) {
            goto error;
        }
    }

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
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
            node = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
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
    const char *value;
    int r;
    int c_tpdf = 0, c_ftrs = 0;

    memset(&root, 0, sizeof root);

    notif = calloc(1, sizeof *notif);
    notif->nodetype = LYS_NOTIF;
    notif->prev = (struct lys_node *)notif;
    retval = (struct lys_node *)notif;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
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
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        notif->tpdf = calloc(c_tpdf, sizeof *notif->tpdf);
    }
    if (c_ftrs) {
        notif->features = calloc(c_ftrs, sizeof *notif->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &notif->tpdf[notif->tpdf_size++], unres);

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-features")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &notif->features[notif->features_size++], UNRES_IFFEAT, value,
                                       LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        }
    }

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
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
            node = read_yin_uses(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, resolve, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anyxml(module, retval, sub, resolve, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
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
    const char *value;
    int r;
    int c_tpdf = 0, c_ftrs = 0;

    /* init */
    memset(&root, 0, sizeof root);

    rpc = calloc(1, sizeof *rpc);
    rpc->nodetype = LYS_RPC;
    rpc->prev = (struct lys_node *)rpc;
    retval = (struct lys_node *)rpc;

    if (read_yin_common(module, parent, retval, yin, OPT_IDENT | OPT_MODULE | OPT_NACMEXT)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
            continue;
        }

        if (!strcmp(sub->name, "input")) {
            if (rpc->child
                && (rpc->child->nodetype == LYS_INPUT
                    || (rpc->child->next && rpc->child->next->nodetype == LYS_INPUT))) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);
        } else if (!strcmp(sub->name, "output")) {
            if (rpc->child
                && (rpc->child->nodetype == LYS_INPUT
                    || (rpc->child->next && rpc->child->next->nodetype == LYS_INPUT))) {
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
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
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes */
    if (c_tpdf) {
        rpc->tpdf = calloc(c_tpdf, sizeof *rpc->tpdf);
    }
    if (c_ftrs) {
        rpc->features = calloc(c_ftrs, sizeof *rpc->features);
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &rpc->tpdf[rpc->tpdf_size++], unres);

            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &rpc->features[rpc->features_size++], UNRES_IFFEAT, value,
                                       LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        }
    }

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
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

        lyxml_free_elem(module->ctx, sub);
    }

    return retval;

error:

    lys_node_free(retval);
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
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
read_yin_uses(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int resolve,
              struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next;
    struct lys_node *retval;
    struct lys_node_uses *uses;
    const char *value;
    int c_ref = 0, c_aug = 0, c_ftrs = 0;
    int r;

    uses = calloc(1, sizeof *uses);
    uses->nodetype = LYS_USES;
    uses->prev = (struct lys_node *)uses;
    retval = (struct lys_node *)uses;

    GETVAL(value, yin, "name");
    uses->name = lydict_insert(module->ctx, value, 0);

    if (read_yin_common(module, parent, retval, yin, OPT_MODULE
            | (parent && (parent->nodetype == LYS_GROUPING) ? 0 : OPT_NACMEXT) | (resolve ? OPT_INHERIT : 0))) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* get other properties of uses */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns || strcmp(sub->ns->value, LY_NSYIN)) {
            /* garbage */
            lyxml_free_elem(module->ctx, sub);
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
                LOGVAL(LYE_TOOMANY, LOGLINE(sub), sub->name, yin->name);
                goto error;
            }

            uses->when = read_yin_when(module, sub);
            if (!uses->when) {
                lyxml_free_elem(module->ctx, sub);
                goto error;
            }
            if (lyxp_syntax_check(uses->when->cond, LOGLINE(sub))) {
                goto error;
            }
            lyxml_free_elem(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(sub), sub->name);
            goto error;
        }
    }

    /* process properties with cardinality 0..n */
    if (c_ref) {
        uses->refine = calloc(c_ref, sizeof *uses->refine);
    }
    if (c_aug) {
        uses->augment = calloc(c_aug, sizeof *uses->augment);
    }
    if (c_ftrs) {
        uses->features = calloc(c_ftrs, sizeof *uses->features);
    }

    if (lys_node_addchild(parent, module, retval)) {
        goto error;
    }

    LY_TREE_FOR(yin->child, sub) {
        if (!strcmp(sub->name, "refine")) {
            r = fill_yin_refine(module, sub, &uses->refine[uses->refine_size++]);
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "augment")) {
            r = fill_yin_augment(module, retval, sub, &uses->augment[uses->augment_size++], unres);
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            GETVAL(value, sub, "name");
            if (!(value = transform_schema2json(module, value, LOGLINE(sub)))) {
                goto error;
            }
            r = unres_schema_add_str(module, unres, &uses->features[uses->features_size++], UNRES_IFFEAT, value,
                                     LOGLINE(sub));
            lydict_remove(module->ctx, value);
            if (r == -1) {
                goto error;
            }
        }
    }

    if (unres_schema_add_node(module, unres, uses, UNRES_USES, NULL, LOGLINE(yin)) == -1) {
        goto error;
    }

    if (resolve) {
        /* inherit config flag */
        if (parent) {
            retval->flags |= parent->flags & LYS_CONFIG_MASK;
        } else {
            /* default config is true */
            retval->flags |= LYS_CONFIG_W;
        }
    }

    return retval;

error:

    lys_node_free(retval);

    return NULL;
}

/* logs directly
 *
 * common code for yin_read_module() and yin_read_submodule()
 */
static int
read_sub_module(struct lys_module *module, struct lyxml_elem *yin, struct unres_schema *unres)
{
    struct ly_ctx *ctx = module->ctx;
    struct lys_submodule *submodule = (struct lys_submodule *)module;
    struct lyxml_elem *next, *child, *child2, root, grps, augs;
    struct lys_node *node = NULL;
    const char *value;
    int i, r;
    int belongsto_flag = 0;
    /* counters */
    int c_imp = 0, c_rev = 0, c_tpdf = 0, c_ident = 0, c_inc = 0, c_aug = 0, c_ftrs = 0, c_dev = 0;

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
            lyxml_free_elem(ctx, child);
            continue;
        }

        if (!module->type && !strcmp(child->name, "namespace")) {
            if (module->ns) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "uri");
            module->ns = lydict_insert(ctx, value, strlen(value));
            lyxml_free_elem(ctx, child);
        } else if (!module->type && !strcmp(child->name, "prefix")) {
            if (module->prefix) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "value");
            if (check_identifier(value, LY_IDENT_PREFIX, LOGLINE(child), module, NULL)) {
                goto error;
            }
            module->prefix = lydict_insert(ctx, value, strlen(value));
            lyxml_free_elem(ctx, child);
        } else if (module->type && !strcmp(child->name, "belongs-to")) {
            if (belongsto_flag) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            belongsto_flag = 1;
            GETVAL(value, child, "module");
            while (submodule->belongsto->type) {
                submodule->belongsto = ((struct lys_submodule *)submodule->belongsto)->belongsto;
            }
            if (value != submodule->belongsto->name) {
                LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                goto error;
            }

            /* get the prefix substatement, start with checks */
            if (!child->child) {
                LOGVAL(LYE_MISSSTMT2, LOGLINE(child), "prefix", child->name);
                goto error;
            } else if (strcmp(child->child->name, "prefix")) {
                LOGVAL(LYE_INSTMT, LOGLINE(child->child), child->child->name);
                goto error;
            } else if (child->child->next) {
                LOGVAL(LYE_INSTMT, LOGLINE(child->child->next), child->child->next->name);
                goto error;
            }
            /* and now finally get the value */
            GETVAL(value, child->child, "value");
            /* check here differs from a generic prefix check, since this prefix
             * don't have to be unique
             */
            if (check_identifier(value, LY_IDENT_NAME, LOGLINE(child->child), NULL, NULL)) {
                goto error;
            }
            module->prefix = lydict_insert(ctx, value, strlen(value));

            /* we are done with belongs-to */
            lyxml_free_elem(ctx, child);

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
            lyxml_unlink_elem(module->ctx, child, 2);
            lyxml_add_child(module->ctx, &augs, child);

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
            lyxml_unlink_elem(module->ctx, child, 2);
            lyxml_add_child(module->ctx, &root, child);

        } else if (!strcmp(child->name, "grouping")) {
            /* keep groupings separated and process them before other data statements */
            lyxml_unlink_elem(module->ctx, child, 2);
            lyxml_add_child(module->ctx, &grps, child);

            /* optional statements */
        } else if (!strcmp(child->name, "description")) {
            if (module->dsc) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            module->dsc = read_yin_subnode(ctx, child, "text");
            lyxml_free_elem(ctx, child);
            if (!module->dsc) {
                goto error;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (module->ref) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            module->ref = read_yin_subnode(ctx, child, "text");
            lyxml_free_elem(ctx, child);
            if (!module->ref) {
                goto error;
            }
        } else if (!strcmp(child->name, "organization")) {
            if (module->org) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            module->org = read_yin_subnode(ctx, child, "text");
            lyxml_free_elem(ctx, child);
            if (!module->org) {
                goto error;
            }
        } else if (!strcmp(child->name, "contact")) {
            if (module->contact) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            module->contact = read_yin_subnode(ctx, child, "text");
            lyxml_free_elem(ctx, child);
            if (!module->contact) {
                goto error;
            }
        } else if (!strcmp(child->name, "yang-version")) {
            /* TODO: support YANG 1.1 ? */
            if (module->version) {
                LOGVAL(LYE_TOOMANY, LOGLINE(child), child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "value");
            if (strcmp(value, "1")) {
                LOGVAL(LYE_INARG, LOGLINE(child), value, "yang-version");
                goto error;
            }
            module->version = 1;
            lyxml_free_elem(ctx, child);

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
                lyxml_free_elem(ctx, child);
            }
        } else {
            LOGVAL(LYE_INSTMT, LOGLINE(child), child->name);
            goto error;
        }
    }

    if (!submodule) {
        /* check for mandatory statements */
        if (!module->ns) {
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "namespace", "module");
            goto error;
        }
        if (!module->prefix) {
            LOGVAL(LYE_MISSSTMT2, LOGLINE(yin), "prefix", "module");
            goto error;
        }
    }

    /* allocate arrays for elements with cardinality of 0..n */
    if (c_imp) {
        module->imp = calloc(c_imp, sizeof *module->imp);
    }
    if (c_rev) {
        module->rev = calloc(c_rev, sizeof *module->rev);
    }
    if (c_tpdf) {
        module->tpdf = calloc(c_tpdf, sizeof *module->tpdf);
    }
    if (c_ident) {
        module->ident = calloc(c_ident, sizeof *module->ident);
    }
    if (c_inc) {
        module->inc = calloc(c_inc, sizeof *module->inc);
    }
    if (c_aug) {
        module->augment = calloc(c_aug, sizeof *module->augment);
    }
    if (c_ftrs) {
        module->features = calloc(c_ftrs, sizeof *module->features);
    }
    if (c_dev) {
        module->deviation = calloc(c_dev, sizeof *module->deviation);
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes and augments */
    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!strcmp(child->name, "import")) {
            r = fill_yin_import(module, child, &module->imp[module->imp_size]);
            module->imp_size++;
            if (r) {
                goto error;
            }

            /* check duplicities in imported modules */
            for (i = 0; i < module->imp_size - 1; i++) {
                if (!strcmp(module->imp[i].module->name, module->imp[module->imp_size - 1].module->name)) {
                    LOGVAL(LYE_SPEC, LOGLINE(child), "Importing module \"%s\" repeatedly.", module->imp[i].module->name);
                    goto error;
                }
            }

        } else if (!strcmp(child->name, "include")) {
            r = fill_yin_include(module, child, &module->inc[module->inc_size]);
            module->inc_size++;
            if (r) {
                goto error;
            }

            /* check duplications in include submodules */
            for (i = 0; i < module->inc_size - 1; i++) {
                if (!strcmp(module->inc[i].submodule->name, module->inc[module->inc_size - 1].submodule->name)) {
                    LOGVAL(LYE_SPEC, LOGLINE(child), "Importing module \"%s\" repeatedly.",
                           module->inc[i].submodule->name);
                    goto error;
                }
            }

        } else if (!strcmp(child->name, "revision")) {
            GETVAL(value, child, "date");
            if (check_date(value, LOGLINE(child))) {
                goto error;
            }
            memcpy(module->rev[module->rev_size].date, value, LY_REV_SIZE - 1);
            /* check uniqueness of the revision date - not required by RFC */
            for (i = 0; i < module->rev_size; i++) {
                if (!strcmp(value, module->rev[i].date)) {
                    LOGVAL(LYE_INARG, LOGLINE(child), value, child->name);
                    LOGVAL(LYE_SPEC, 0, "Revision is not unique.");
                }
            }

            LY_TREE_FOR(child->child, child2) {
                if (!strcmp(child2->name, "description")) {
                    if (module->rev[module->rev_size].dsc) {
                        LOGVAL(LYE_TOOMANY, LOGLINE(child), child2->name, child->name);
                        goto error;
                    }
                    module->rev[module->rev_size].dsc = read_yin_subnode(ctx, child2, "text");
                    if (!module->rev[module->rev_size].dsc) {
                        goto error;
                    }
                } else if (!strcmp(child2->name, "reference")) {
                    if (module->rev[module->rev_size].ref) {
                        LOGVAL(LYE_TOOMANY, LOGLINE(child), child2->name, child->name);
                        goto error;
                    }
                    module->rev[module->rev_size].ref = read_yin_subnode(ctx, child2, "text");
                    if (!module->rev[module->rev_size].ref) {
                        goto error;
                    }
                } else {
                    LOGVAL(LYE_INSTMT, LOGLINE(child2), child2->name);
                    goto error;
                }
            }

            /* keep the latest revision at position 0 */
            if (module->rev_size && strcmp(module->rev[module->rev_size].date, module->rev[0].date) > 0) {
                /* switch their position */
                value = strdup(module->rev[0].date);
                memcpy(module->rev[0].date, module->rev[module->rev_size].date, LY_REV_SIZE - 1);
                memcpy(module->rev[module->rev_size].date, value, LY_REV_SIZE - 1);
                free((char *)value);

                if (module->rev[0].dsc != module->rev[module->rev_size].dsc) {
                    value = module->rev[0].dsc;
                    module->rev[0].dsc = module->rev[module->rev_size].dsc;
                    module->rev[module->rev_size].dsc = value;
                }

                if (module->rev[0].ref != module->rev[module->rev_size].ref) {
                    value = module->rev[0].ref;
                    module->rev[0].ref = module->rev[module->rev_size].ref;
                    module->rev[module->rev_size].ref = value;
                }
            }

            module->rev_size++;

        } else if (!strcmp(child->name, "typedef")) {
            r = fill_yin_typedef(module, NULL, child, &module->tpdf[module->tpdf_size], unres);
            module->tpdf_size++;

            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "identity")) {
            r = fill_yin_identity(module, child, &module->ident[module->ident_size], unres);
            module->ident_size++;

            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "feature")) {
            r = fill_yin_feature(module, child, &module->features[module->features_size], unres);
            module->features_size++;

            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "deviation")) {
            r = fill_yin_deviation(module, child, &module->deviation[module->deviation_size]);
            module->deviation_size++;

            if (r) {
                goto error;
            }

        }
    }

    /* process data nodes. Start with groupings to allow uses
     * refer to them
     */
    LY_TREE_FOR_SAFE(grps.child, next, child) {
        node = read_yin_grouping(module, NULL, child, 0, unres);
        if (!node) {
            goto error;
        }

        lyxml_free_elem(ctx, child);
    }

    /* parse data nodes, ... */
    LY_TREE_FOR_SAFE(root.child, next, child) {

        if (!strcmp(child->name, "container")) {
            node = read_yin_container(module, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "leaf-list")) {
            node = read_yin_leaflist(module, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "leaf")) {
            node = read_yin_leaf(module, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "list")) {
            node = read_yin_list(module, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "choice")) {
            node = read_yin_choice(module, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "uses")) {
            node = read_yin_uses(module, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "anyxml")) {
            node = read_yin_anyxml(module, NULL, child, 1, unres);
        } else if (!strcmp(child->name, "rpc")) {
            node = read_yin_rpc(module, NULL, child, 0, unres);
        } else if (!strcmp(child->name, "notification")) {
            node = read_yin_notif(module, NULL, child, 0, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free_elem(ctx, child);
    }

    /* ... and finally augments (last, so we can augment our data, for instance) */
    LY_TREE_FOR_SAFE(augs.child, next, child) {
        r = fill_yin_augment(module, NULL, child, &module->augment[module->augment_size], unres);
        module->augment_size++;

        if (r) {
            goto error;
        }
        lyxml_free_elem(ctx, child);
    }

    return EXIT_SUCCESS;

error:
    /* cleanup */
    while (root.child) {
        lyxml_free_elem(module->ctx, root.child);
    }
    while (grps.child) {
        lyxml_free_elem(module->ctx, grps.child);
    }
    while (augs.child) {
        lyxml_free_elem(module->ctx, augs.child);
    }

    free(unres->item);
    unres->item = NULL;
    free(unres->type);
    unres->type = NULL;
    free(unres->str_snode);
    unres->str_snode = NULL;
#ifndef NDEBUG
    free(unres->line);
    unres->line = NULL;
#endif
    unres->count = 0;

    return EXIT_FAILURE;
}

/* logs directly */
struct lys_submodule *
yin_read_submodule(struct lys_module *module, const char *data, int implement, struct unres_schema *unres)
{
    struct lyxml_elem *yin;
    struct lys_submodule *submodule = NULL;
    const char *value;

    assert(module->ctx);

    yin = lyxml_read(module->ctx, data, 0);
    if (!yin) {
        return NULL;
    }

    /* check root element */
    if (!yin->name || strcmp(yin->name, "submodule")) {
        LOGVAL(LYE_INSTMT, LOGLINE(yin), yin->name);
        goto error;
    }

    GETVAL(value, yin, "name");
    if (check_identifier(value, LY_IDENT_NAME, LOGLINE(yin), NULL, NULL)) {
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
    submodule->implemented = (implement ? 1 : 0);

    LOGVRB("Reading submodule %s", submodule->name);
    if (read_sub_module((struct lys_module *)submodule, yin, unres)) {
        goto error;
    }

    /* cleanup */
    lyxml_free_elem(module->ctx, yin);

    LOGVRB("Submodule %s successfully parsed", submodule->name);

    return submodule;

error:
    /* cleanup */
    lyxml_free_elem(module->ctx, yin);
    lys_submodule_free(submodule, 0);

    return NULL;
}

/* logs directly */
struct lys_module *
yin_read_module(struct ly_ctx *ctx, const char *data, int implement, struct unres_schema *unres)
{
    struct lyxml_elem *yin;
    struct lys_module *module = NULL, **newlist = NULL;
    const char *value;
    int i;

    yin = lyxml_read(ctx, data, 0);
    if (!yin) {
        return NULL;
    }

    /* check root element */
    if (!yin->name || strcmp(yin->name, "module")) {
        LOGVAL(LYE_INSTMT, LOGLINE(yin), yin->name);
        goto error;
    }

    GETVAL(value, yin, "name");
    if (check_identifier(value, LY_IDENT_NAME, LOGLINE(yin), NULL, NULL)) {
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

    LOGVRB("Reading module %s", module->name);
    if (read_sub_module(module, yin, unres)) {
        goto error;
    }

    /* add to the context's list of modules */
    if (ctx->models.used == ctx->models.size) {
        newlist = realloc(ctx->models.list, (2 * ctx->models.size) * sizeof *newlist);
        if (!newlist) {
            LOGMEM;
            goto error;
        }
        for (i = ctx->models.size; i < ctx->models.size * 2; i++) {
            newlist[i] = NULL;
        }
        ctx->models.size *= 2;
        ctx->models.list = newlist;
    }
    for (i = 0; ctx->models.list[i]; i++) {
        /* check name (name/revision) and namespace uniqueness */
        if (!strcmp(ctx->models.list[i]->name, module->name)) {
            if (ctx->models.list[i]->rev_size == module->rev_size) {
                /* both have the same number of revisions */
                if (!module->rev_size || !strcmp(ctx->models.list[i]->rev[0].date, module->rev[0].date)) {
                    /* both have the same revision -> we already have the same module */
                    /* so free the new one and update the old one's implement flag if needed */
                    lyxml_free_elem(ctx, yin);
                    lys_free(module, 0);
                    unres->count = 0;

                    LOGVRB("Module %s already in context", ctx->models.list[i]->name);

                    if (implement && !ctx->models.list[i]->implemented) {
                        lyp_set_implemented(ctx->models.list[i]);
                    }
                    return ctx->models.list[i];
                }
            }
            /* else (both elses) keep searching, for now the caller is just adding
             * another revision of an already present schema
             */
        } else if (!strcmp(ctx->models.list[i]->ns, module->ns)) {
            LOGERR(LY_EINVAL, "Two different modules (\"%s\" and \"%s\") have the same namespace \"%s\"",
                   ctx->models.list[i]->name, module->name, module->ns);
            goto error;
        }
    }
    ctx->models.list[i] = module;
    ctx->models.used++;
    ctx->models.module_set_id++;

    /* cleanup */
    lyxml_free_elem(ctx, yin);

    LOGVRB("Module %s successfully parsed", module->name);

    return module;

error:
    /* cleanup */
    lyxml_free_elem(ctx, yin);
    lys_free(module, 0);

    return NULL;
}
