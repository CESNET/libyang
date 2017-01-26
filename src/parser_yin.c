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

#define OPT_IDENT       0x01
#define OPT_CFG_PARSE   0x02
#define OPT_CFG_INHERIT 0x04
#define OPT_CFG_IGNORE  0x08
#define OPT_MODULE      0x10
static int read_yin_common(struct lys_module *, struct lys_node *, void *, LYEXT_PAR, struct lyxml_elem *, int,
                           struct unres_schema *);

static struct lys_node *read_yin_choice(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                        int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_case(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                      int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_anydata(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                         LYS_NODE type, int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_container(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                           int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_leaf(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                      int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_leaflist(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                          int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_list(struct lys_module *module,struct lys_node *parent, struct lyxml_elem *yin,
                                      int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_uses(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                      struct unres_schema *unres);
static struct lys_node *read_yin_grouping(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                          int valid_config, struct unres_schema *unres);
static struct lys_node *read_yin_rpc_action(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                            struct unres_schema *unres);
static struct lys_node *read_yin_notif(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                                       struct unres_schema *unres);
static struct lys_when *read_yin_when(struct lys_module *module, struct lyxml_elem *yin, struct unres_schema *unres);

/*
 * yin - the provided XML subtree is unlinked
 * ext - pointer to the storage in the parent structure to be able to update its location after realloc
 */
int
lyp_yin_fill_ext(void *parent, LYEXT_PAR parent_type, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
             struct lys_module *module, struct lyxml_elem *yin, struct lys_ext_instance ***ext,
             uint8_t ext_index, struct unres_schema *unres)
{
    struct unres_ext *info;

    info = malloc(sizeof *info);
    lyxml_unlink(module->ctx, yin);
    info->data.yin = yin;
    info->datatype = LYS_IN_YIN;
    info->parent = parent;
    info->mod = module;
    info->parent_type = parent_type;
    info->substmt = substmt;
    info->substmt_index = substmt_index;
    info->ext_index = ext_index;

    if (unres_schema_add_node(module, unres, ext, UNRES_EXT, (struct lys_node *)info) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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

static int
read_yin_subnode_ext(struct lys_module *mod, void *elem, LYEXT_PAR elem_type,
                     struct lyxml_elem *yin, LYEXT_SUBSTMT type, uint8_t i, struct unres_schema *unres)
{
    void *reallocated;
    struct lyxml_elem *next, *child;
    int r;
    struct lys_ext_instance ***ext;
    uint8_t *ext_size;

    switch (elem_type) {
    case LYEXT_PAR_MODULE:
        ext_size = &((struct lys_module *)elem)->ext_size;
        ext = &((struct lys_module *)elem)->ext;
        break;
    case LYEXT_PAR_IMPORT:
        ext_size = &((struct lys_import *)elem)->ext_size;
        ext = &((struct lys_import *)elem)->ext;
        break;
    case LYEXT_PAR_INCLUDE:
        ext_size = &((struct lys_include *)elem)->ext_size;
        ext = &((struct lys_include *)elem)->ext;
        break;
    case LYEXT_PAR_REVISION:
        ext_size = &((struct lys_revision *)elem)->ext_size;
        ext = &((struct lys_revision *)elem)->ext;
        break;
    case LYEXT_PAR_NODE:
        ext_size = &((struct lys_node *)elem)->ext_size;
        ext = &((struct lys_node *)elem)->ext;
        break;
    case LYEXT_PAR_IDENT:
        ext_size = &((struct lys_ident *)elem)->ext_size;
        ext = &((struct lys_ident *)elem)->ext;
        break;
    case LYEXT_PAR_TYPE:
        ext_size = &((struct lys_type *)elem)->ext_size;
        ext = &((struct lys_type *)elem)->ext;
        break;
    case LYEXT_PAR_TYPE_BIT:
        ext_size = &((struct lys_type_bit *)elem)->ext_size;
        ext = &((struct lys_type_bit *)elem)->ext;
        break;
    case LYEXT_PAR_TYPE_ENUM:
        ext_size = &((struct lys_type_enum *)elem)->ext_size;
        ext = &((struct lys_type_enum *)elem)->ext;
        break;
    case LYEXT_PAR_TPDF:
        ext_size = &((struct lys_tpdf *)elem)->ext_size;
        ext = &((struct lys_tpdf *)elem)->ext;
        break;
    case LYEXT_PAR_EXT:
        ext_size = &((struct lys_ext *)elem)->ext_size;
        ext = &((struct lys_ext *)elem)->ext;
        break;
    case LYEXT_PAR_FEATURE:
        ext_size = &((struct lys_feature *)elem)->ext_size;
        ext = &((struct lys_feature *)elem)->ext;
        break;
    case LYEXT_PAR_REFINE:
        ext_size = &((struct lys_refine *)elem)->ext_size;
        ext = &((struct lys_refine *)elem)->ext;
        break;
    case LYEXT_PAR_RESTR:
        ext_size = &((struct lys_restr *)elem)->ext_size;
        ext = &((struct lys_restr *)elem)->ext;
        break;
    case LYEXT_PAR_WHEN:
        ext_size = &((struct lys_when *)elem)->ext_size;
        ext = &((struct lys_when *)elem)->ext;
        break;
    case LYEXT_PAR_DEVIATE:
        ext_size = &((struct lys_deviate *)elem)->ext_size;
        ext = &((struct lys_deviate *)elem)->ext;
        break;
    case LYEXT_PAR_DEVIATION:
        ext_size = &((struct lys_deviation *)elem)->ext_size;
        ext = &((struct lys_deviation *)elem)->ext;
        break;
    default:
        LOGERR(LY_EINT, "parent type %d", elem_type);
        return EXIT_FAILURE;
    }

    if (type == LYEXT_SUBSTMT_SELF) {
        /* parse for the statement self, not for the substatement */
        child = yin;
        next = NULL;
        goto parseext;
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!strcmp(child->ns->value, LY_NSYIN)) {
            /* skip the regular YIN nodes */
            continue;
        }

        /* parse it as extension */
parseext:

        /* first, allocate a space for the extension instance in the parent elem */
        reallocated = realloc(*ext, (1 + (*ext_size)) * sizeof **ext);
        if (!reallocated) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        (*ext) = reallocated;

        /* init memory */
        (*ext)[(*ext_size)] = NULL;

        /* parse YIN data */
        r = lyp_yin_fill_ext(elem, elem_type, type, i, mod, child, &(*ext), (*ext_size), unres);
        (*ext_size)++;
        if (r) {
            return EXIT_FAILURE;
        }

        /* done - do not free the child, it is unlinked in lyp_yin_fill_ext */
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
fill_yin_iffeature(struct lys_node *parent, int parent_is_feature, struct lyxml_elem *yin, struct lys_iffeature *iffeat,
                   struct unres_schema *unres)
{
    int r, c_ext = 0;
    const char *value;
    struct lyxml_elem *node, *next;

    GETVAL(value, yin, "name");

    if ((lys_node_module(parent)->version != 2) && ((value[0] == '(') || strchr(value, ' '))) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
error:
        return EXIT_FAILURE;
    }

    if (!(value = transform_iffeat_schema2json(parent->module, value))) {
        return EXIT_FAILURE;
    }

    r = resolve_iffeature_compile(iffeat, value, parent, parent_is_feature, unres);
    lydict_remove(parent->module->ctx, value);
    if (r) {
        return EXIT_FAILURE;
    }

    LY_TREE_FOR_SAFE(yin->child, next, node) {
        if (!node->ns) {
            /* garbage */
            lyxml_free(parent->module->ctx, node);
        } else if (strcmp(node->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name, "if-feature");
            return EXIT_FAILURE;
        }
    }
    if (c_ext) {
        iffeat->ext = calloc(c_ext, sizeof *iffeat->ext);
        if (!iffeat->ext) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            /* extensions */
            r = lyp_yin_fill_ext(iffeat, LYEXT_PAR_IDENT, 0, 0, parent->module, node,
                                 &iffeat->ext, iffeat->ext_size, unres);
            iffeat->ext_size++;
            if (r) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
fill_yin_identity(struct lys_module *module, struct lyxml_elem *yin, struct lys_ident *ident, struct unres_schema *unres)
{
    struct lyxml_elem *node, *next;
    const char *value;
    int rc;
    int c_ftrs = 0, c_base = 0, c_ext = 0;
    void *reallocated;

    GETVAL(value, yin, "name");
    ident->name = value;

    if (read_yin_common(module, NULL, ident, LYEXT_PAR_IDENT, yin, OPT_IDENT | OPT_MODULE, unres)) {
        goto error;
    }

    if (dup_identities_check(ident->name, module)) {
        goto error;
    }

    LY_TREE_FOR(yin->child, node) {
        if (strcmp(node->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
        } else if (!strcmp(node->name, "base")) {
            if (c_base && (module->version < 2)) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                goto error;
            }
            if (read_yin_subnode_ext(module, ident, LYEXT_PAR_IDENT, node, LYEXT_SUBSTMT_BASE, c_base, unres)) {
                goto error;
            }
            c_base++;

        } else if ((module->version >= 2) && !strcmp(node->name, "if-feature")) {
            c_ftrs++;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name, "identity");
            goto error;
        }
    }

    if (c_base) {
        ident->base_size = 0;
        ident->base = calloc(c_base, sizeof *ident->base);
        if (!ident->base) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ftrs) {
        ident->iffeature = calloc(c_ftrs, sizeof *ident->iffeature);
        if (!ident->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(ident->ext, (c_ext + ident->ext_size) * sizeof *ident->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        ident->ext = reallocated;

        /* init memory */
        memset(&ident->ext[ident->ext_size], 0, c_ext * sizeof *ident->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, node) {
        if (strcmp(node->ns->value, LY_NSYIN)) {
            /* extension */
            rc = lyp_yin_fill_ext(ident, LYEXT_PAR_IDENT, 0, 0, module, node, &ident->ext, ident->ext_size, unres);
            ident->ext_size++;
            if (rc) {
                goto error;
            }
        } else if (!strcmp(node->name, "base")) {
            GETVAL(value, node, "name");
            value = transform_schema2json(module, value);
            if (!value) {
                goto error;
            }

            if (unres_schema_add_str(module, unres, ident, UNRES_IDENT, value) == -1) {
                lydict_remove(module->ctx, value);
                goto error;
            }
            lydict_remove(module->ctx, value);
        } else if (!strcmp(node->name, "if-feature")) {
            rc = fill_yin_iffeature((struct lys_node *)ident, 0, node, &ident->iffeature[ident->iffeature_size], unres);
            ident->iffeature_size++;
            if (rc) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

/* logs directly */
static int
read_restr_substmt(struct lys_module *module, LYEXT_PAR restr_type, struct lys_restr *restr, struct lyxml_elem *yin,
                   struct unres_schema *unres)
{
    struct lyxml_elem *child, *next;
    const char *value;

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns) {
            /* garbage */
            continue;
        } else if (strcmp(child->ns->value, LY_NSYIN)) {
            /* extension */
            if (read_yin_subnode_ext(module, restr, restr_type, child, LYEXT_SUBSTMT_SELF, 0, unres)) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "description")) {
            if (restr->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            if (read_yin_subnode_ext(module, restr, restr_type, child, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
                return EXIT_FAILURE;
            }
            restr->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!restr->dsc) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "reference")) {
            if (restr->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            if (read_yin_subnode_ext(module, restr, restr_type, child, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
                return EXIT_FAILURE;
            }
            restr->ref = read_yin_subnode(module->ctx, child, "text");
            if (!restr->ref) {
                return EXIT_FAILURE;
            }
        } else if (!strcmp(child->name, "error-app-tag")) {
            if (restr->eapptag) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            if (read_yin_subnode_ext(module, restr, restr_type, child, LYEXT_SUBSTMT_ERRTAG, 0, unres)) {
                return EXIT_FAILURE;
            }
            GETVAL(value, child, "value");
            restr->eapptag = lydict_insert(module->ctx, value, 0);
        } else if (!strcmp(child->name, "error-message")) {
            if (restr->emsg) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                return EXIT_FAILURE;
            }
            if (read_yin_subnode_ext(module, restr, restr_type, child, LYEXT_SUBSTMT_ERRMSG, 0, unres)) {
                return EXIT_FAILURE;
            }
            restr->emsg = read_yin_subnode(module->ctx, child, "value");
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
    struct lyxml_elem *next, *next2, *node, *child, exts;
    struct lys_restr **restrs, *restr;
    struct lys_type_bit bit, *bits_sc = NULL;
    struct lys_type_enum *enms_sc = NULL; /* shortcut */
    struct lys_type *dertype;
    int i, j, rc, val_set, c_ftrs, c_ext = 0;
    int ret = -1;
    int64_t v, v_;
    int64_t p, p_;
    size_t len;
    char *buf, modifier;

    /* init */
    memset(&exts, 0, sizeof exts);

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

    if (type->base == LY_TYPE_ERR) {
        /* resolved type in grouping, decrease the grouping's nacm number to indicate that one less
         * unresolved item left inside the grouping, LY_TYPE_ERR used as a flag for types inside a grouping. */
        for (siter = parent; siter && (siter->nodetype != LYS_GROUPING); siter = lys_parent(siter));
        if (siter) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            if (!((uint8_t*)&((struct lys_node_grp *)siter)->flags)[1]) {
                LOGINT;
                goto error;
            }
            ((uint8_t*)&((struct lys_node_grp *)siter)->flags)[1]--;
#else
            if (!((uint8_t*)&((struct lys_node_grp *)siter)->flags)[0]) {
                LOGINT;
                goto error;
            }
            ((uint8_t*)&((struct lys_node_grp *)siter)->flags)[0]--;
#endif
        } else {
            LOGINT;
            goto error;
        }
    }
    type->base = type->der->type.base;

    /* check status */
    if (lyp_check_status(type->parent->flags, type->parent->module, type->parent->name,
                     type->der->flags, type->der->module, type->der->name,  parent)) {
        return -1;
    }

    /* parse extension instances */
    LY_TREE_FOR_SAFE(yin->child, next, node) {
        if (!node->ns) {
            /* garbage */
            lyxml_free(module->ctx, node);
            continue;
        } else if (!strcmp(node->ns->value, LY_NSYIN)) {
            /* YANG (YIN) statements - process later */
            continue;
        }

        lyxml_unlink_elem(module->ctx, node, 2);
        lyxml_add_child(module->ctx, &exts, node);
        c_ext++;
    }
    if (c_ext) {
        type->ext = calloc(c_ext, sizeof *type->ext);
        if (!type->ext) {
            LOGMEM;
            goto error;
        }
        LY_TREE_FOR_SAFE(exts.child, next, node) {
            rc = lyp_yin_fill_ext(type, LYEXT_PAR_TYPE, 0, 0, module, node, &type->ext, type->ext_size, unres);
            type->ext_size++;
            if (rc) {
                goto error;
            }
        }
    }

    switch (type->base) {
    case LY_TYPE_BITS:
        /* RFC 6020 9.7.4 - bit */

        /* get bit specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            if (!strcmp(node->name, "bit")) {
                type->info.bits.count++;
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                type->info.bits.count = 0;
                goto error;
            }
        }
        dertype = &type->der->type;
        if (!dertype->der) {
            if (!type->info.bits.count) {
                /* type is derived directly from buit-in bits type and bit statement is required */
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "bit", "type");
                goto error;
            }
        } else {
            for (; !dertype->info.enums.count; dertype = &dertype->der->type);
            if (module->version < 2 && type->info.bits.count) {
                /* type is not directly derived from buit-in bits type and bit statement is prohibited,
                 * since YANG 1.1 the bit statements can be used to restrict the base bits type */
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "bit");
                type->info.bits.count = 0;
                goto error;
            }
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
            c_ftrs = 0;

            GETVAL(value, next, "name");
            if (lyp_check_identifier(value, LY_IDENT_SIMPLE, NULL, NULL)) {
                goto error;
            }

            type->info.bits.bit[i].name = lydict_insert(module->ctx, value, strlen(value));
            if (read_yin_common(module, NULL, &type->info.bits.bit[i], LYEXT_PAR_TYPE_BIT, next, 0, unres)) {
                type->info.bits.count = i + 1;
                goto error;
            }

            if (!dertype->der) { /* directly derived type from bits built-in type */
                /* check the name uniqueness */
                for (j = 0; j < i; j++) {
                    if (!strcmp(type->info.bits.bit[j].name, type->info.bits.bit[i].name)) {
                        LOGVAL(LYE_BITS_DUPNAME, LY_VLOG_NONE, NULL, type->info.bits.bit[i].name);
                        type->info.bits.count = i + 1;
                        goto error;
                    }
                }
            } else {
                /* restricted bits type - the name MUST be used in the base type */
                bits_sc = dertype->info.bits.bit;
                for (j = 0; j < dertype->info.bits.count; j++) {
                    if (ly_strequal(bits_sc[j].name, value, 1)) {
                        break;
                    }
                }
                if (j == dertype->info.bits.count) {
                    LOGVAL(LYE_BITS_INNAME, LY_VLOG_NONE, NULL, value);
                    type->info.bits.count = i + 1;
                    goto error;
                }
            }


            p_ = -1;
            LY_TREE_FOR_SAFE(next->child, next2, node) {
                if (!node->ns) {
                    /* garbage */
                    continue;
                } else if (strcmp(node->ns->value, LY_NSYIN)) {
                    /* extension */
                    if (read_yin_subnode_ext(module, &type->info.bits.bit[i], LYEXT_PAR_TYPE_BIT, node,
                                             LYEXT_SUBSTMT_SELF, 0, unres)) {
                        goto error;
                    }
                } else if (!strcmp(node->name, "position")) {
                    if (p_ != -1) {
                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, next->name);
                        type->info.bits.count = i + 1;
                        goto error;
                    }

                    GETVAL(value, node, "value");
                    p_ = strtoll(value, NULL, 10);

                    /* range check */
                    if (p_ < 0 || p_ > UINT32_MAX) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "bit/position");
                        type->info.bits.count = i + 1;
                        goto error;
                    }
                    type->info.bits.bit[i].pos = (uint32_t)p_;

                    if (!dertype->der) { /* directly derived type from bits built-in type */
                        /* keep the highest enum value for automatic increment */
                        if (type->info.bits.bit[i].pos >= p) {
                            p = type->info.bits.bit[i].pos;
                            p++;
                        } else {
                            /* check that the value is unique */
                            for (j = 0; j < i; j++) {
                                if (type->info.bits.bit[j].pos == type->info.bits.bit[i].pos) {
                                    LOGVAL(LYE_BITS_DUPVAL, LY_VLOG_NONE, NULL,
                                           type->info.bits.bit[i].pos, type->info.bits.bit[i].name,
                                           type->info.bits.bit[j].name);
                                    type->info.bits.count = i + 1;
                                    goto error;
                                }
                            }
                        }
                    }

                    if (read_yin_subnode_ext(module, &type->info.bits.bit[i], LYEXT_PAR_TYPE_BIT, node,
                                             LYEXT_SUBSTMT_POSITION, 0, unres)) {
                        goto error;
                    }
                } else if ((module->version >= 2) && !strcmp(node->name, "if-feature")) {
                    c_ftrs++;
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                    goto error;
                }
            }

            if (!dertype->der) { /* directly derived type from bits built-in type */
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
            } else { /* restricted bits type */
                if (p_ == -1) {
                    /* automatically assign position from base type */
                    type->info.bits.bit[i].pos = bits_sc[j].pos;
                    type->info.bits.bit[i].flags |= LYS_AUTOASSIGNED;
                } else {
                    /* check that the assigned position corresponds to the original
                     * position of the bit in the base type */
                    if (p_ != bits_sc[j].pos) {
                        /* p_ - assigned position in restricted bits
                         * bits_sc[j].pos - position assigned to the corresponding bit (detected above) in base type */
                        LOGVAL(LYE_BITS_INVAL, LY_VLOG_NONE, NULL, type->info.bits.bit[i].pos,
                               type->info.bits.bit[i].name, bits_sc[j].pos);
                        type->info.bits.count = i + 1;
                        goto error;
                    }
                }
            }

            /* if-features */
            if (c_ftrs) {
                bits_sc = &type->info.bits.bit[i];
                bits_sc->iffeature = calloc(c_ftrs, sizeof *bits_sc->iffeature);
                if (!bits_sc->iffeature) {
                    LOGMEM;
                    type->info.bits.count = i + 1;
                    goto error;
                }

                LY_TREE_FOR(next->child, node) {
                    if (!strcmp(node->name, "if-feature")) {
                        rc = fill_yin_iffeature((struct lys_node *)type->parent, 0, node,
                                                &bits_sc->iffeature[bits_sc->iffeature_size], unres);
                        bits_sc->iffeature_size++;
                        if (rc) {
                            type->info.bits.count = i + 1;
                            goto error;
                        }
                    }
                }
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

            if (!strcmp(node->name, "range")) {
                if (type->info.dec64.range) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                type->info.dec64.range = calloc(1, sizeof *type->info.dec64.range);
                if (!type->info.dec64.range) {
                    LOGMEM;
                    goto error;
                }
                type->info.dec64.range->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module, LYEXT_PAR_RESTR, type->info.dec64.range, node, unres)) {
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

                /* extensions */
                if (read_yin_subnode_ext(module, type, LYEXT_PAR_TYPE, node, LYEXT_SUBSTMT_DIGITS, 0, unres)) {
                    goto error;
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

        /* copy fraction-digits specification from parent type for easier internal use */
        if (type->der->type.der) {
            type->info.dec64.dig = type->der->type.info.dec64.dig;
            type->info.dec64.div = type->der->type.info.dec64.div;
        }

        if (type->info.dec64.range && lyp_check_length_range(type->info.dec64.range->expr, type)) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "range");
            goto error;
        }
        break;

    case LY_TYPE_ENUM:
        /* RFC 6020 9.6 - enum */

        /* get enum specifications, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {

            if (!strcmp(node->name, "enum")) {
                type->info.enums.count++;
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                type->info.enums.count = 0;
                goto error;
            }
        }
        dertype = &type->der->type;
        if (!dertype->der) {
            if (!type->info.enums.count) {
                /* type is derived directly from buit-in enumeartion type and enum statement is required */
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "enum", "type");
                goto error;
            }
        } else {
            for (; !dertype->info.enums.count; dertype = &dertype->der->type);
            if (module->version < 2 && type->info.enums.count) {
                /* type is not directly derived from built-in enumeration type and enum statement is prohibited
                 * in YANG 1.0, since YANG 1.1 enum statements can be used to restrict the base enumeration type */
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "enum");
                type->info.enums.count = 0;
                goto error;
            }
        }

        type->info.enums.enm = calloc(type->info.enums.count, sizeof *type->info.enums.enm);
        if (!type->info.enums.enm) {
            LOGMEM;
            goto error;
        }

        v = 0;
        i = -1;
        LY_TREE_FOR(yin->child, next) {
            i++;
            c_ftrs = 0;

            GETVAL(value, next, "name");
            if (!value[0]) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "enum name");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Enum name must not be empty.");
                goto error;
            }
            type->info.enums.enm[i].name = lydict_insert(module->ctx, value, strlen(value));
            if (read_yin_common(module, NULL, &type->info.enums.enm[i], LYEXT_PAR_TYPE_ENUM, next, 0, unres)) {
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

            if (!dertype->der) { /* directly derived type from enumeration built-in type */
                /* check the name uniqueness */
                for (j = 0; j < i; j++) {
                    if (ly_strequal(type->info.enums.enm[j].name, value, 1)) {
                        LOGVAL(LYE_ENUM_DUPNAME, LY_VLOG_NONE, NULL, value);
                        type->info.enums.count = i + 1;
                        goto error;
                    }
                }
            } else {
                /* restricted enumeration type - the name MUST be used in the base type */
                enms_sc = dertype->info.enums.enm;
                for (j = 0; j < dertype->info.enums.count; j++) {
                    if (ly_strequal(enms_sc[j].name, value, 1)) {
                        break;
                    }
                }
                if (j == dertype->info.enums.count) {
                    LOGVAL(LYE_ENUM_INNAME, LY_VLOG_NONE, NULL, value);
                    type->info.enums.count = i + 1;
                    goto error;
                }
            }

            val_set = 0;
            LY_TREE_FOR_SAFE(next->child, next2, node) {
                if (!node->ns) {
                    /* garbage */
                    continue;
                } else if (strcmp(node->ns->value, LY_NSYIN)) {
                    /* extensions */
                    if (read_yin_subnode_ext(module, &type->info.enums.enm[i], LYEXT_PAR_TYPE_ENUM, node,
                                             LYEXT_SUBSTMT_SELF, 0, unres)) {
                        goto error;
                    }
                } else if (!strcmp(node->name, "value")) {
                    if (val_set) {
                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, next->name);
                        type->info.enums.count = i + 1;
                        goto error;
                    }

                    GETVAL(value, node, "value");
                    v_ = strtoll(value, NULL, 10);

                    /* range check */
                    if (v_ < INT32_MIN || v_ > INT32_MAX) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "enum/value");
                        type->info.enums.count = i + 1;
                        goto error;
                    }
                    type->info.enums.enm[i].value = v_;

                    if (!dertype->der) { /* directly derived type from enumeration built-in type */
                        if (!i) {
                            /* change value, which is assigned automatically, if first enum has value. */
                            v = type->info.enums.enm[i].value;
                            v++;
                        } else {
                            /* keep the highest enum value for automatic increment */
                            if (type->info.enums.enm[i].value >= v) {
                                v = type->info.enums.enm[i].value;
                                v++;
                            } else {
                                /* check that the value is unique */
                                for (j = 0; j < i; j++) {
                                    if (type->info.enums.enm[j].value == type->info.enums.enm[i].value) {
                                        LOGVAL(LYE_ENUM_DUPVAL, LY_VLOG_NONE, NULL,
                                               type->info.enums.enm[i].value, type->info.enums.enm[i].name,
                                               type->info.enums.enm[j].name);
                                        type->info.enums.count = i + 1;
                                        goto error;
                                    }
                                }
                            }
                        }
                    }
                    val_set = 1;

                    if (read_yin_subnode_ext(module, &type->info.enums.enm[i], LYEXT_PAR_TYPE_ENUM, node,
                                             LYEXT_SUBSTMT_VALUE, 0, unres)) {
                        goto error;
                    }
                } else if ((module->version >= 2) && !strcmp(node->name, "if-feature")) {
                    c_ftrs++;

                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                    goto error;
                }
            }

            if (!dertype->der) { /* directly derived type from enumeration */
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
            } else { /* restricted enum type */
                if (!val_set) {
                    /* automatically assign value from base type */
                    type->info.enums.enm[i].value = enms_sc[j].value;
                    type->info.enums.enm[i].flags |= LYS_AUTOASSIGNED;
                } else {
                    /* check that the assigned value corresponds to the original
                     * value of the enum in the base type */
                    if (v_ != enms_sc[j].value) {
                        /* v_ - assigned value in restricted enum
                         * enms_sc[j].value - value assigned to the corresponding enum (detected above) in base type */
                        LOGVAL(LYE_ENUM_INVAL, LY_VLOG_NONE, NULL,
                               type->info.enums.enm[i].value, type->info.enums.enm[i].name, enms_sc[j].value);
                        type->info.enums.count = i + 1;
                        goto error;
                    }
                }
            }

            /* if-features */
            if (c_ftrs) {
                enms_sc = &type->info.enums.enm[i];
                enms_sc->iffeature = calloc(c_ftrs, sizeof *enms_sc->iffeature);
                if (!enms_sc->iffeature) {
                    LOGMEM;
                    type->info.enums.count = i + 1;
                    goto error;
                }

                LY_TREE_FOR(next->child, node) {
                    if (!strcmp(node->name, "if-feature")) {
                        rc = fill_yin_iffeature((struct lys_node *)type->parent, 0, node,
                                                &enms_sc->iffeature[enms_sc->iffeature_size], unres);
                        enms_sc->iffeature_size++;
                        if (rc) {
                            type->info.enums.count = i + 1;
                            goto error;
                        }
                    }
                }
            }

        }
        break;

    case LY_TYPE_IDENT:
        /* RFC 6020 9.10 - base */

        /* get base specification, at least one must be present */
        LY_TREE_FOR_SAFE(yin->child, next, node) {

            if (strcmp(node->name, "base")) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
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

            if (read_yin_subnode_ext(module, type, LYEXT_PAR_TYPE, node, LYEXT_SUBSTMT_BASE, 0, unres)) {
                goto error;
            }
        }

        if (!yin->child) {
            if (type->der->type.der) {
                /* this is just a derived type with no base required */
                break;
            }
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "base", "type");
            goto error;
        } else {
            if (type->der->type.der) {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "base");
                goto error;
            }
        }
        if (yin->child->next) {
            LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, yin->child->next->name, yin->name);
            goto error;
        }
        break;

    case LY_TYPE_INST:
        /* RFC 6020 9.13.2 - require-instance */
        LY_TREE_FOR(yin->child, node) {

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

                /* extensions */
                if (read_yin_subnode_ext(module, type, LYEXT_PAR_TYPE, node, LYEXT_SUBSTMT_REQINST, 0, unres)) {
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
            restrs = &type->info.binary.length;
            name = "length";
        } else {
            restrs = &type->info.num.range;
            name = "range";
        }

        LY_TREE_FOR(yin->child, node) {

            if (!strcmp(node->name, name)) {
                if (*restrs) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }

                GETVAL(value, node, "value");
                if (lyp_check_length_range(value, type)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, name);
                    goto error;
                }
                *restrs = calloc(1, sizeof **restrs);
                if (!(*restrs)) {
                    LOGMEM;
                    goto error;
                }
                (*restrs)->expr = lydict_insert(module->ctx, value, 0);

                /* get possible substatements */
                if (read_restr_substmt(module, LYEXT_PAR_RESTR, *restrs, node, unres)) {
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

                /* extensions */
                if (read_yin_subnode_ext(module, type, LYEXT_PAR_TYPE, node, LYEXT_SUBSTMT_PATH, 0, unres)) {
                    goto error;
                }
            } else if (module->version >= 2 && !strcmp(node->name, "require-instance")) {
                if (type->info.lref.req) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                    goto error;
                }
                GETVAL(value, node, "value");
                if (!strcmp(value, "true")) {
                    type->info.lref.req = 1;
                } else if (!strcmp(value, "false")) {
                    type->info.lref.req = -1;
                } else {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, node->name);
                    goto error;
                }

                /* extensions */
                if (read_yin_subnode_ext(module, type, LYEXT_PAR_TYPE, node, LYEXT_SUBSTMT_REQINST, 0, unres)) {
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
                if (read_restr_substmt(module, LYEXT_PAR_RESTR, type->info.str.length, node, unres)) {
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
                restr = &type->info.str.patterns[type->info.str.pat_count]; /* shortcut */

                modifier = 0x06; /* ACK */
                name = NULL;
                if (module->version >= 2) {
                    LY_TREE_FOR_SAFE(node->child, next2, child) {
                        if (child->ns && !strcmp(child->ns->value, LY_NSYIN) && !strcmp(child->name, "modifier")) {
                            if (name) {
                                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "modifier", node->name);
                                goto error;
                            }

                            GETVAL(name, child, "value");
                            if (!strcmp(name, "invert-match")) {
                                modifier = 0x15; /* NACK */
                            } else {
                                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, name, "modifier");
                                goto error;
                            }
                            /* get extensions of the modifier */
                            if (read_yin_subnode_ext(module, restr, LYEXT_PAR_RESTR, child,
                                                     LYEXT_SUBSTMT_MODIFIER, 0, unres)) {
                                return EXIT_FAILURE;
                            }

                            lyxml_free(module->ctx, child);
                        }
                    }
                }

                len = strlen(value);
                buf = malloc((len + 2) * sizeof *buf); /* modifier byte + value + terminating NULL byte */
                buf[0] = modifier;
                strcpy(&buf[1], value);

                restr->expr = lydict_insert_zc(module->ctx, buf);

                /* get possible sub-statements */
                if (read_restr_substmt(module, LYEXT_PAR_RESTR, restr, node, unres)) {
                    free(type->info.str.patterns);
                    type->info.str.patterns = NULL;
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

            if (!strcmp(node->name, "type")) {
                if (type->der->type.der) {
                    /* type can be a substatement only in "union" type, not in derived types */
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "type", "derived type");
                    goto error;
                }
                i++;
            } else {
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, node->name);
                goto error;
            }
        }

        if (!i && !type->der->type.der) {
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "(union) type");
            goto error;
        }

        /* inherit instid presence information */
        if ((type->der->type.base == LY_TYPE_UNION) && type->der->type.info.uni.has_ptr_type) {
            type->info.uni.has_ptr_type = 1;
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

                if (module->version < 2) {
                    /* union's type cannot be empty or leafref */
                    if (type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_EMPTY) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "empty", node->name);
                        rc = -1;
                    } else if (type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_LEAFREF) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, "leafref", node->name);
                        rc = -1;
                    }
                }

                if ((type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_INST)
                        || (type->info.uni.types[type->info.uni.count - 1].base == LY_TYPE_LEAFREF)) {
                    type->info.uni.has_ptr_type = 1;
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
                type->info.uni.has_ptr_type = 0;
                type->der = NULL;
                type->base = LY_TYPE_DER;

                if (rc == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                }
                goto error;
            }
        }
        break;

    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        /* no sub-statement allowed */
        if (yin->child) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yin->child->name);
            goto error;
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
    lyxml_free_withsiblings(module->ctx, exts.child);

    return ret;
}

/* logs directly */
static int
fill_yin_typedef(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_tpdf *tpdf,
                 struct unres_schema *unres)
{
    const char *value;
    struct lyxml_elem *node, *next;
    int rc, has_type = 0, c_ext = 0;
    void *reallocated;

    GETVAL(value, yin, "name");
    if (lyp_check_identifier(value, LY_IDENT_TYPE, module, parent)) {
        goto error;
    }
    tpdf->name = lydict_insert(module->ctx, value, strlen(value));

    /* generic part - status, description, reference */
    if (read_yin_common(module, NULL, tpdf, LYEXT_PAR_TPDF, yin, OPT_MODULE, unres)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, node) {
        if (strcmp(node->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;
        } else if (!strcmp(node->name, "type")) {
            if (has_type) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                goto error;
            }
            /* HACK for unres */
            tpdf->type.der = (struct lys_tpdf *)node;
            tpdf->type.parent = tpdf;
            if (unres_schema_add_node(module, unres, &tpdf->type, UNRES_TYPE_DER_TPDF, parent) == -1) {
                goto error;
            }
            has_type = 1;

            /* skip lyxml_free() at the end of the loop, node was freed or at least unlinked in unres processing */
            continue;
        } else if (!strcmp(node->name, "default")) {
            if (tpdf->dflt) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "value");
            tpdf->dflt = lydict_insert(module->ctx, value, strlen(value));

            if (read_yin_subnode_ext(module, tpdf, LYEXT_PAR_TPDF, node, LYEXT_SUBSTMT_DEFAULT, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(node->name, "units")) {
            if (tpdf->units) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, node->name, yin->name);
                goto error;
            }
            GETVAL(value, node, "name");
            tpdf->units = lydict_insert(module->ctx, value, strlen(value));

            if (read_yin_subnode_ext(module, tpdf, LYEXT_PAR_TPDF, node, LYEXT_SUBSTMT_UNITS, 0, unres)) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, value);
            goto error;
        }

        lyxml_free(module->ctx, node);
    }

    /* check mandatory value */
    if (!has_type) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", yin->name);
        goto error;
    }

    /* check default value (if not defined, there still could be some restrictions
     * that need to be checked against a default value from a derived type) */
    if (unres_schema_add_node(module, unres, &tpdf->type, UNRES_TYPE_DFLT, (struct lys_node *)(&tpdf->dflt)) == -1) {
        goto error;
    }

    /* finish extensions parsing */
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(tpdf->ext, (c_ext + tpdf->ext_size) * sizeof *tpdf->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        tpdf->ext = reallocated;

        /* init memory */
        memset(&tpdf->ext[tpdf->ext_size], 0, c_ext * sizeof *tpdf->ext);

        LY_TREE_FOR_SAFE(yin->child, next, node) {
            rc = lyp_yin_fill_ext(tpdf, LYEXT_PAR_TYPE, 0, 0, module, node, &tpdf->ext, tpdf->ext_size, unres);
            tpdf->ext_size++;
            if (rc) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static int
fill_yin_extension(struct lys_module *module, struct lyxml_elem *yin, struct lys_ext *ext, struct unres_schema *unres)
{
    const char *value;
    struct lyxml_elem *child, *node, *next, *next2;
    int c_ext = 0, rc;
    void *reallocated;

    GETVAL(value, yin, "name");

    if (lyp_check_identifier(value, LY_IDENT_EXTENSION, module, NULL)) {
        goto error;
    }
    ext->name = lydict_insert(module->ctx, value, strlen(value));

    if (read_yin_common(module, NULL, ext, LYEXT_PAR_EXT, yin, OPT_MODULE, unres)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, node) {
        if (strcmp(node->ns->value, LY_NSYIN)) {
            /* possible extension instance */
            c_ext++;
        } else if (!strcmp(node->name, "argument")) {
            /* argument */
            GETVAL(value, node, "name");
            ext->argument = lydict_insert(module->ctx, value, strlen(value));
            if (read_yin_subnode_ext(module, ext, LYEXT_PAR_EXT, node, LYEXT_SUBSTMT_ARGUMENT, 0, unres)) {
                goto error;
            }

            /* yin-element */
            LY_TREE_FOR_SAFE(node->child, next2, child) {
                if (child->ns == node->ns && !strcmp(child->name, "yin-element")) {
                    GETVAL(value, child, "value");
                    if (ly_strequal(value, "true", 0)) {
                        ext->flags |= LYS_YINELEM;
                    }

                    if (read_yin_subnode_ext(module, ext, LYEXT_PAR_EXT, child, LYEXT_SUBSTMT_YINELEM, 0, unres)) {
                        goto error;
                    }
                } else if (child->ns) {
                    /* unexpected YANG statement */
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, child->name, child->name);
                    goto error;
                } /* else garbage, but save resource needed for unlinking */
            }

            lyxml_free(module->ctx, node);
        } else {
            /* unexpected YANG statement */
            LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, node->name, node->name);
            goto error;
        }
    }

    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(ext->ext, (c_ext + ext->ext_size) * sizeof *ext->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        ext->ext = reallocated;

        /* init memory */
        memset(&ext->ext[ext->ext_size], 0, c_ext * sizeof *ext->ext);

        /* process the extension instances of the extension itself */
        LY_TREE_FOR_SAFE(yin->child, next, node) {
            rc = lyp_yin_fill_ext(ext, LYEXT_PAR_EXT, 0, 0, module, node, &ext->ext, ext->ext_size, unres);
            ext->ext_size++;
            if (rc) {
                goto error;
            }
        }
    }

    /* search for plugin */
    ext->plugin = ext_get_plugin(ext->name, ext->module->name, ext->module->rev ? ext->module->rev[0].date : NULL);

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
    int c_ftrs = 0, c_ext = 0, ret;
    void *reallocated;

    GETVAL(value, yin, "name");
    if (lyp_check_identifier(value, LY_IDENT_FEATURE, module, NULL)) {
        goto error;
    }
    f->name = lydict_insert(module->ctx, value, strlen(value));
    f->module = module;

    if (read_yin_common(module, NULL, f, LYEXT_PAR_FEATURE, yin, 0, unres)) {
        goto error;
    }

    LY_TREE_FOR(yin->child, child) {
        if (strcmp(child->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
        } else if (!strcmp(child->name, "if-feature")) {
            c_ftrs++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    if (c_ftrs) {
        f->iffeature = calloc(c_ftrs, sizeof *f->iffeature);
        if (!f->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(f->ext, (c_ext + f->ext_size) * sizeof *f->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        f->ext = reallocated;

        /* init memory */
        memset(&f->ext[f->ext_size], 0, c_ext * sizeof *f->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (strcmp(child->ns->value, LY_NSYIN)) {
            /* extension */
            ret = lyp_yin_fill_ext(f, LYEXT_PAR_FEATURE, 0, 0, module, child, &f->ext, f->ext_size, unres);
            f->ext_size++;
            if (ret) {
                goto error;
            }
        } else { /* if-feature */
            ret = fill_yin_iffeature((struct lys_node *)f, 1, child, &f->iffeature[f->iffeature_size], unres);
            f->iffeature_size++;
            if (ret) {
                goto error;
            }
        }
    }

    /* check for circular dependencies */
    if (f->iffeature_size) {
        if (unres_schema_add_node(module, unres, f, UNRES_FEATURE, NULL) == -1) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_must(struct lys_module *module, struct lyxml_elem *yin, struct lys_restr *must, struct unres_schema *unres)
{
    const char *value;

    GETVAL(value, yin, "condition");
    must->expr = transform_schema2json(module, value);
    if (!must->expr) {
        goto error;
    }

    return read_restr_substmt(module, LYEXT_PAR_RESTR, must, yin, unres);

error:
    return EXIT_FAILURE;
}

static int
fill_yin_unique(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_unique *unique,
                struct unres_schema *unres)
{
    int i, j;
    const char *value, *vaux;
    struct unres_list_uniq *unique_info;

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
            unique_info = malloc(sizeof *unique_info);
            unique_info->list = parent;
            unique_info->expr = unique->expr[i];
            unique_info->trg_type = &unique->trg_type;
            if (unres_schema_add_node(module, unres, unique_info, UNRES_LIST_UNIQ, NULL) == -1){
                goto error;
            }
        } else {
            if (resolve_unique(parent, unique->expr[i], &unique->trg_type)) {
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
    uint32_t *ui32val, *min, *max;

    /* del min/max is forbidden */
    if (d->mod == LY_DEVIATE_DEL) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, (type ? "max-elements" : "min-elements"), "deviate delete");
        goto error;
    }

    /* check target node type */
    if (target->nodetype == LYS_LEAFLIST) {
        max = &((struct lys_node_leaflist *)target)->max;
        min = &((struct lys_node_leaflist *)target)->min;
    } else if (target->nodetype == LYS_LIST) {
        max = &((struct lys_node_list *)target)->max;
        min = &((struct lys_node_list *)target)->min;
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
        ui32val = max;
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
            ui32val = max;
        } else {
            d->min = (uint32_t)val;
            d->min_set = 1;
            ui32val = min;
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

    /* check min-elements is smaller than max-elements */
    if (*max && *min > *max) {
        if (type) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "max-elements");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
        } else {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "min-elements");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
        }
        goto error;
    }

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
    struct lyxml_elem *next, *next2, *child, *develem;
    int c_dev = 0, c_must, c_uniq, c_dflt, c_ext = 0;
    int f_min = 0, f_max = 0; /* flags */
    int i, j, rc;
    unsigned int u;
    struct ly_ctx *ctx;
    struct lys_deviate *d = NULL;
    struct lys_node *node = NULL, *parent, *dev_target = NULL;
    struct lys_node_choice *choice = NULL;
    struct lys_node_leaf *leaf = NULL;
    struct ly_set *dflt_check = ly_set_new();
    struct lys_node_list *list = NULL;
    struct lys_node_leaflist *llist = NULL;
    struct lys_type *t = NULL;
    uint8_t *trg_must_size = NULL;
    struct lys_restr **trg_must = NULL;
    struct unres_schema tmp_unres;
    struct lys_module *mod;
    void *reallocated;

    ctx = module->ctx;

    GETVAL(value, yin, "target-node");
    dev->target_name = transform_schema2json(module, value);
    if (!dev->target_name) {
        goto error;
    }

    /* resolve target node */
    rc = resolve_augment_schema_nodeid(dev->target_name, NULL, module, 1, (const struct lys_node **)&dev_target);
    if (rc || !dev_target) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, yin->name);
        goto error;
    }
    if (dev_target->module == lys_main_module(module)) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, yin->name);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Deviating own module is not allowed.");
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns ) {
            /* garbage */
            lyxml_free(ctx, child);
            continue;
        } else if (strcmp(child->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;
        } else if (!strcmp(child->name, "description")) {
            if (dev->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            if (read_yin_subnode_ext(module, dev, LYEXT_PAR_DEVIATION, child, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
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
            if (read_yin_subnode_ext(module, dev, LYEXT_PAR_DEVIATION, child, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
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
    } else {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
        goto error;
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(dev->ext, (c_ext + dev->ext_size) * sizeof *dev->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        dev->ext = reallocated;

        /* init memory */
        memset(&dev->ext[dev->ext_size], 0, c_ext * sizeof *dev->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, develem) {
        if (strcmp(develem->ns->value, LY_NSYIN)) {
            /* deviation's extension */
            rc = lyp_yin_fill_ext(dev, LYEXT_PAR_DEVIATION, 0, 0, module, develem, &dev->ext, dev->ext_size, unres);
            dev->ext_size++;
            if (rc) {
                goto error;
            }
            continue;
        }

        /* deviate */
        /* init */
        f_min = 0;
        f_max = 0;
        c_must = 0;
        c_uniq = 0;
        c_dflt = 0;
        c_ext = 0;

        /* get deviation type */
        GETVAL(value, develem, "value");
        if (!strcmp(value, "not-supported")) {
            dev->deviate[dev->deviate_size].mod = LY_DEVIATE_NO;
            /* no other deviate statement is expected,
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
            dev->orig_node = lys_node_dup(dev_target->module, NULL, dev_target, &tmp_unres, 1);
            /* just to be safe */
            if (tmp_unres.count) {
                LOGINT;
                goto error;
            }
        }

        /* process deviation properties */
        LY_TREE_FOR_SAFE(develem->child, next2, child) {
            if (!child->ns) {
                /* garbage */
                lyxml_free(ctx, child);
                continue;
            } else if  (strcmp(child->ns->value, LY_NSYIN)) {
                /* extensions */
                c_ext++;
            } else if (d->mod == LY_DEVIATE_NO) {
                /* no YIN substatement expected in this case */
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                goto error;
            } else if (!strcmp(child->name, "config")) {
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

                if (read_yin_subnode_ext(module, d, LYEXT_PAR_DEVIATE, child, LYEXT_SUBSTMT_CONFIG, 0, unres)) {
                    goto error;
                }
            } else if (!strcmp(child->name, "default")) {
                if (read_yin_subnode_ext(module, d, LYEXT_PAR_DEVIATE, child, LYEXT_SUBSTMT_DEFAULT, c_dflt, unres)) {
                    goto error;
                }
                c_dflt++;

                /* check target node type */
                if (module->version < 2 && dev_target->nodetype == LYS_LEAFLIST) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"default\" property.");
                    goto error;
                } else if (c_dflt > 1 && dev_target->nodetype != LYS_LEAFLIST) { /* from YANG 1.1 */
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow multiple \"default\" properties.");
                    goto error;
                } else if (c_dflt == 1 && (!(dev_target->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE)))) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"default\" property.");
                    goto error;
                }

                /* skip lyxml_free() at the end of the loop, this node will be processed later */
                continue;

            } else if (!strcmp(child->name, "mandatory")) {
                if (d->flags & LYS_MAND_MASK) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                    goto error;
                }

                /* check target node type */
                if (!(dev_target->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA))) {
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
                    if (d->flags & LYS_MAND_TRUE) {
                        if (dev_target->nodetype == LYS_CHOICE) {
                            if (((struct lys_node_choice *)(dev_target))->dflt) {
                                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, child->name, child->parent->name);
                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                                       "Adding the \"mandatory\" statement is forbidden on choice with the \"default\" statement.");
                                goto error;
                            }
                        } else if (dev_target->nodetype == LYS_LEAF) {
                            if (((struct lys_node_leaf *)(dev_target))->dflt) {
                                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, child->name, child->parent->name);
                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                                       "Adding the \"mandatory\" statement is forbidden on leaf with the \"default\" statement.");
                                goto error;
                            }
                        }
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

                /* check for mandatory node in default case, first find the closest parent choice to the changed node */
                for (parent = dev_target->parent;
                     parent && !(parent->nodetype & (LYS_CHOICE | LYS_GROUPING | LYS_ACTION));
                     parent = parent->parent) {
                    if (parent->nodetype == LYS_CONTAINER && ((struct lys_node_container *)parent)->presence) {
                        /* stop also on presence containers */
                        break;
                    }
                }
                /* and if it is a choice with the default case, check it for presence of a mandatory node in it */
                if (parent && parent->nodetype == LYS_CHOICE && ((struct lys_node_choice *)parent)->dflt) {
                    if (lyp_check_mandatory_choice(parent)) {
                        goto error;
                    }
                }

                if (read_yin_subnode_ext(module, d, LYEXT_PAR_DEVIATE, child, LYEXT_SUBSTMT_MANDATORY, 0, unres)) {
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
                if (read_yin_subnode_ext(module, d, LYEXT_PAR_DEVIATE, child, LYEXT_SUBSTMT_MIN, 0, unres)) {
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
                if (read_yin_subnode_ext(module, d, LYEXT_PAR_DEVIATE, child, LYEXT_SUBSTMT_MAX, 0, unres)) {
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
                    if (((struct lys_node_leaf *)dev_target)->dflt) {
                        ly_set_add(dflt_check, dev_target, 0);
                    }
                } else if (dev_target->nodetype == LYS_LEAFLIST) {
                    t = &((struct lys_node_leaflist *)dev_target)->type;
                    if (((struct lys_node_leaflist *)dev_target)->dflt) {
                        ly_set_add(dflt_check, dev_target, 0);
                    }
                } else {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", child->name);
                    goto error;
                }

                /* replace */
                lys_type_free(ctx, t);
                /* HACK for unres */
                t->der = (struct lys_tpdf *)child;
                if (unres_schema_add_node(module, unres, t, UNRES_TYPE_DER, dev_target) == -1) {
                    goto error;
                }
                d->type = t;
            } else if (!strcmp(child->name, "unique")) {
                if (read_yin_subnode_ext(module, d, LYEXT_PAR_DEVIATE, child, LYEXT_SUBSTMT_UNIQUE, c_uniq, unres)) {
                    goto error;
                }
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
                    (*stritem) = NULL;
                }

                if (read_yin_subnode_ext(module, d, LYEXT_PAR_DEVIATE, child, LYEXT_SUBSTMT_UNITS, 0, unres)) {
                    goto error;
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
            case LYS_ANYDATA:
                trg_must = &((struct lys_node_anydata *)dev_target)->must;
                trg_must_size = &((struct lys_node_anydata *)dev_target)->must_size;
                break;
            default:
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "must");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"must\" property.");
                goto error;
            }

            dev_target->flags &= ~LYS_XPATH_DEP;

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
                LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "unique");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"unique\" property.");
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
        if (c_dflt) {
            if (d->mod == LY_DEVIATE_ADD) {
                /* check that there is no current value */
                if ((dev_target->nodetype == LYS_LEAF && ((struct lys_node_leaf *)dev_target)->dflt) ||
                        (dev_target->nodetype == LYS_CHOICE && ((struct lys_node_choice *)dev_target)->dflt)) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
                    goto error;
                }

                /* check collision with mandatory/min-elements */
                if ((dev_target->flags & LYS_MAND_TRUE) ||
                        (dev_target->nodetype == LYS_LEAFLIST && ((struct lys_node_leaflist *)dev_target)->min)) {
                    LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, child->name, child->parent->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                           "Adding the \"default\" statement is forbidden on %s statement.",
                           (dev_target->flags & LYS_MAND_TRUE) ? "nodes with the \"mandatory\"" : "leaflists with non-zero \"min-elements\"");
                    goto error;
                }
            } else if (d->mod == LY_DEVIATE_RPL) {
                /* check that there was a value before */
                if (((dev_target->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && !((struct lys_node_leaf *)dev_target)->dflt) ||
                        (dev_target->nodetype == LYS_CHOICE && !((struct lys_node_choice *)dev_target)->dflt)) {
                    LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
                    goto error;
                }
            }

            if (dev_target->nodetype == LYS_LEAFLIST) {
                /* reallocate default list in the target */
                llist = (struct lys_node_leaflist *)dev_target;
                if (d->mod == LY_DEVIATE_ADD) {
                    /* reallocate (enlarge) the unique array of the target */
                    llist->dflt = ly_realloc(llist->dflt, (c_dflt + llist->dflt_size) * sizeof *d->dflt);
                } else if (d->mod == LY_DEVIATE_RPL) {
                    /* reallocate (replace) the unique array of the target */
                    for (i = 0; i < llist->dflt_size; i++) {
                        lydict_remove(llist->module->ctx, llist->dflt[i]);
                    }
                    llist->dflt = ly_realloc(llist->dflt, c_dflt * sizeof *d->dflt);
                    llist->dflt_size = 0;
                }
            }
            d->dflt = calloc(c_dflt, sizeof *d->dflt);
            if (!d->dflt) {
                LOGMEM;
                goto error;
            }
        }
        if (c_ext) {
            /* some extensions may be already present from the substatements */
            reallocated = realloc(d->ext, (c_ext + d->ext_size) * sizeof *d->ext);
            if (!reallocated) {
                LOGMEM;
                goto error;
            }
            d->ext = reallocated;

            /* init memory */
            memset(&d->ext[d->ext_size], 0, c_ext * sizeof *d->ext);
        }

        /* process deviation properties with 0..n cardinality */
        LY_TREE_FOR_SAFE(develem->child, next2, child) {
            if (strcmp(child->ns->value, LY_NSYIN)) {
                /* extension */
                if (lyp_yin_fill_ext(d, LYEXT_PAR_DEVIATE, 0, 0, module, child, &d->ext, d->ext_size, unres)) {
                    goto error;
                }
                d->ext_size++;
            } else if (!strcmp(child->name, "must")) {
                if (d->mod == LY_DEVIATE_DEL) {
                    if (fill_yin_must(module, child, &d->must[d->must_size], unres)) {
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
                    if (fill_yin_must(module, child, &((*trg_must)[*trg_must_size]), unres)) {
                        goto error;
                    }
                    (*trg_must_size)++;
                }

                /* check XPath dependencies again */
                if (*trg_must_size && unres_schema_add_node(module, unres, dev_target, UNRES_XPATH, NULL)) {
                    goto error;
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
            } else if (!strcmp(child->name, "default")) {
                GETVAL(value, child, "value");
                u = strlen(value);
                d->dflt[d->dflt_size++] = lydict_insert(module->ctx, value, u);

                if (dev_target->nodetype == LYS_CHOICE) {
                    choice = (struct lys_node_choice *)dev_target;
                    rc = resolve_choice_default_schema_nodeid(value, choice->child, (const struct lys_node **)&node);
                    if (rc || !node) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                        goto error;
                    }
                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!choice->dflt || (choice->dflt != node)) {
                            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                            goto error;
                        }
                    } else { /* add or replace */
                        choice->dflt = node;
                        if (!choice->dflt) {
                            /* default branch not found */
                            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                            goto error;
                        }
                    }
                } else if (dev_target->nodetype == LYS_LEAF) {
                    leaf = (struct lys_node_leaf *)dev_target;
                    if (d->mod == LY_DEVIATE_DEL) {
                        if (!leaf->dflt || !ly_strequal(leaf->dflt, value, 1)) {
                            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                            goto error;
                        }
                        /* remove value */
                        lydict_remove(ctx, leaf->dflt);
                        leaf->dflt = NULL;
                        leaf->flags &= ~LYS_DFLTJSON;
                    } else { /* add (already checked) and replace */
                        /* remove value */
                        lydict_remove(ctx, leaf->dflt);
                        leaf->flags &= ~LYS_DFLTJSON;

                        /* set new value */
                        leaf->dflt = lydict_insert(ctx, value, u);

                        /* remember to check it later (it may not fit now, because the type can be deviated too) */
                        ly_set_add(dflt_check, dev_target, 0);
                    }
                } else { /* LYS_LEAFLIST */
                    llist = (struct lys_node_leaflist *)dev_target;
                    if (d->mod == LY_DEVIATE_DEL) {
                        /* find and remove the value in target list */
                        for (i = 0; i < llist->dflt_size; i++) {
                            if (llist->dflt[i] && ly_strequal(llist->dflt[i], value, 1)) {
                                /* match, remove the value */
                                lydict_remove(llist->module->ctx, llist->dflt[i]);
                                llist->dflt[i] = NULL;
                                break;
                            }
                        }
                        if (i == llist->dflt_size) {
                            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The default value to delete not found in the target node.");
                            goto error;
                        }
                    } else {
                        /* add or replace, anyway we place items into the deviate's list
                           which propagates to the target */
                        /* we just want to check that the value isn't already in the list */
                        for (i = 0; i < llist->dflt_size; i++) {
                            if (ly_strequal(llist->dflt[i], value, 1)) {
                                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", value);
                                goto error;
                            }
                        }
                        /* store it in target node */
                        llist->dflt[llist->dflt_size++] = lydict_insert(module->ctx, value, u);

                        /* remember to check it later (it may not fit now, but the type can be deviated too) */
                        ly_set_add(dflt_check, dev_target, 0);
                        llist->flags &= ~LYS_DFLTJSON;
                    }
                }
            }
        }

        if (c_dflt && dev_target->nodetype == LYS_LEAFLIST && d->mod == LY_DEVIATE_DEL) {
            /* consolidate the final list in the target after removing items from it */
            llist = (struct lys_node_leaflist *)dev_target;
            for (i = j = 0; j < llist->dflt_size; j++) {
                llist->dflt[i] = llist->dflt[j];
                if (llist->dflt[i]) {
                    i++;
                }
            }
            llist->dflt_size = i + 1;
        }
    }

    /* now check whether default value, if any, matches the type */
    for (u = 0; u < dflt_check->number; ++u) {
        value = NULL;
        rc = EXIT_SUCCESS;
        if (dflt_check->set.s[u]->nodetype == LYS_LEAF) {
            leaf = (struct lys_node_leaf *)dflt_check->set.s[u];
            value = leaf->dflt;
            rc = unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DFLT, (struct lys_node *)(&leaf->dflt));
        } else { /* LYS_LEAFLIST */
            llist = (struct lys_node_leaflist *)dflt_check->set.s[u];
            for (j = 0; j < llist->dflt_size; j++) {
                rc = unres_schema_add_node(module, unres, &llist->type, UNRES_TYPE_DFLT,
                                           (struct lys_node *)(&llist->dflt[j]));
                if (rc == -1) {
                    value = llist->dflt[j];
                    break;
                }
            }

        }
        if (rc == -1) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                   "The default value \"%s\" of the deviated node \"%s\"no longer matches its type.",
                   dev->target_name);
            goto error;
        }
    }

    /* mark all the affected modules as deviated and implemented */
    for(parent = dev_target; parent; parent = lys_parent(parent)) {
        mod = lys_node_module(parent);
        if (module != mod) {
            mod->deviated = 1;
            lys_set_implemented(mod);
        }
    }

    ly_set_free(dflt_check);
    return EXIT_SUCCESS;

error:
    ly_set_free(dflt_check);
    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_augment(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_node_augment *aug,
                 struct unres_schema *unres)
{
    const char *value;
    struct lyxml_elem *sub, *next;
    struct lys_node *node;
    int ret, c_ftrs = 0, c_ext = 0;
    void *reallocated;

    aug->nodetype = LYS_AUGMENT;
    GETVAL(value, yin, "target-node");
    aug->target_name = transform_schema2json(module, value);
    if (!aug->target_name) {
        goto error;
    }
    aug->parent = parent;

    if (read_yin_common(module, NULL, aug, LYEXT_PAR_NODE, yin, OPT_MODULE, unres)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            continue;
        } else if (!strcmp(sub->name, "when")) {
            if (aug->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            aug->when = read_yin_when(module, sub, unres);
            if (!aug->when) {
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);
            continue;

        /* check allowed data sub-statements */
        } else if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, (struct lys_node *)aug, sub, 1, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, (struct lys_node *)aug, sub, 1, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, (struct lys_node *)aug, sub, 1, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, (struct lys_node *)aug, sub, 1, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, (struct lys_node *)aug, sub, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, (struct lys_node *)aug, sub, 1, unres);
        } else if (!strcmp(sub->name, "case")) {
            node = read_yin_case(module, (struct lys_node *)aug, sub, 1, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anydata(module, (struct lys_node *)aug, sub, LYS_ANYXML, 1, unres);
        } else if (!strcmp(sub->name, "anydata")) {
            node = read_yin_anydata(module, (struct lys_node *)aug, sub, LYS_ANYDATA, 1, unres);
        } else if (!strcmp(sub->name, "action")) {
            node = read_yin_rpc_action(module, (struct lys_node *)aug, sub, unres);
        } else if (!strcmp(sub->name, "notification")) {
            node = read_yin_notif(module, (struct lys_node *)aug, sub, unres);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, sub->name);
            goto error;
        }

        if (!node) {
            goto error;
        }

        node = NULL;
        lyxml_free(module->ctx, sub);
    }

    if (c_ftrs) {
        aug->iffeature = calloc(c_ftrs, sizeof *aug->iffeature);
        if (!aug->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(aug->ext, (c_ext + aug->ext_size) * sizeof *aug->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        aug->ext = reallocated;

        /* init memory */
        memset(&aug->ext[aug->ext_size], 0, c_ext * sizeof *aug->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            ret = lyp_yin_fill_ext(aug, LYEXT_PAR_NODE, 0, 0, module, sub, &aug->ext, aug->ext_size, unres);
            aug->ext_size++;
            if (ret) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            ret = fill_yin_iffeature((struct lys_node *)aug, 0, sub, &aug->iffeature[aug->iffeature_size], unres);
            aug->iffeature_size++;
            if (ret) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
        }
    }

    /* aug->child points to the parsed nodes, they must now be
     * connected to the tree and adjusted (if possible right now).
     * However, if this is augment in a uses (parent is NULL), it gets resolved
     * when the uses does and cannot be resolved now for sure
     * (the grouping was not yet copied into uses).
     */
    if (!parent) {
        if (unres_schema_add_node(module, unres, aug, UNRES_AUGMENT, NULL) == -1) {
            goto error;
        }
    }

    /* check XPath dependencies */
    if (aug->when && (unres_schema_add_node(module, unres, (struct lys_node *)aug, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_refine(struct lys_node *uses, struct lyxml_elem *yin, struct lys_refine *rfn, struct unres_schema *unres)
{
    struct lys_module *module;
    struct lyxml_elem *sub, *next;
    const char *value;
    char *endptr;
    int f_mand = 0, f_min = 0, f_max = 0;
    int c_must = 0, c_ftrs = 0, c_dflt = 0, c_ext = 0;
    int r;
    unsigned long int val;
    void *reallocated;

    assert(uses);
    module = uses->module; /* shorthand */

    GETVAL(value, yin, "target-node");
    rfn->target_name = transform_schema2json(module, value);
    if (!rfn->target_name) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
        } else if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;

        } else if (!strcmp(sub->name, "description")) {
            if (rfn->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
                goto error;
            }

            rfn->dsc = read_yin_subnode(module->ctx, sub, "text");
            if (!rfn->dsc) {
                goto error;
            }
        } else if (!strcmp(sub->name, "reference")) {
            if (rfn->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }

            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
                goto error;
            }

            rfn->ref = read_yin_subnode(module->ctx, sub, "text");
            if (!rfn->ref) {
                goto error;
            }
        } else if (!strcmp(sub->name, "config")) {
            if (rfn->flags & LYS_CONFIG_MASK) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            if (!strcmp(value, "false")) {
                rfn->flags |= LYS_CONFIG_R;
            } else if (!strcmp(value, "true")) {
                rfn->flags |= LYS_CONFIG_W;
            } else {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, sub->name);
                goto error;
            }
            rfn->flags |= LYS_CONFIG_SET;

            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_CONFIG, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "default")) {
            /* leaf, leaf-list or choice */

            /* check possibility of statements combination */
            if (rfn->target_type) {
                if (c_dflt) {
                    /* multiple defaults are allowed only in leaf-list */
                    if (module->version < 2) {
                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, yin->name);
                        goto error;
                    }
                    rfn->target_type &= LYS_LEAFLIST;
                } else {
                    if (module->version < 2) {
                        rfn->target_type &= (LYS_LEAF | LYS_CHOICE);
                    } else {
                        /* YANG 1.1 */
                        rfn->target_type &= (LYS_LEAFLIST | LYS_LEAF | LYS_CHOICE);
                    }
                }
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                if (module->version < 2) {
                    rfn->target_type = LYS_LEAF | LYS_CHOICE;
                } else {
                    /* YANG 1.1 */
                    rfn->target_type = LYS_LEAFLIST | LYS_LEAF | LYS_CHOICE;
                }
            }

            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_DEFAULT, c_dflt, unres)) {
                goto error;
            }
            c_dflt++;
            continue;
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
                rfn->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LEAF | LYS_CHOICE | LYS_ANYDATA;
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
            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_MANDATORY, 0, unres)) {
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

            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_MIN, 0, unres)) {
                goto error;
            }
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

            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_MAX, 0, unres)) {
                goto error;
            }
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

            if (read_yin_subnode_ext(module, rfn, LYEXT_PAR_REFINE, sub, LYEXT_SUBSTMT_PRESENCE, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            /* leafm leaf-list, list, container or anyxml */
            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA;
            }

            c_must++;
            continue;

        } else if ((module->version >= 2) && !strcmp(sub->name, "if-feature")) {
            /* leaf, leaf-list, list, container or anyxml */
            /* check possibility of statements combination */
            if (rfn->target_type) {
                rfn->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA);
                if (!rfn->target_type) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, sub->name, yin->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                    goto error;
                }
            } else {
                rfn->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA;
            }

            c_ftrs++;
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
    if (c_ftrs) {
        rfn->iffeature = calloc(c_ftrs, sizeof *rfn->iffeature);
        if (!rfn->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_dflt) {
        rfn->dflt = calloc(c_dflt, sizeof *rfn->dflt);
        if (!rfn->dflt) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(rfn->ext, (c_ext + rfn->ext_size) * sizeof *rfn->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        rfn->ext = reallocated;

        /* init memory */
        memset(&rfn->ext[rfn->ext_size], 0, c_ext * sizeof *rfn->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(rfn, LYEXT_PAR_REFINE, 0, 0, module, sub, &rfn->ext, rfn->ext_size, unres);
            rfn->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(uses, 0, sub, &rfn->iffeature[rfn->iffeature_size], unres);
            rfn->iffeature_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &rfn->must[rfn->must_size], unres);
            rfn->must_size++;
            if (r) {
                goto error;
            }
        } else { /* default */
            GETVAL(value, sub, "value");

            /* check for duplicity */
            for (r = 0; r < rfn->dflt_size; r++) {
                if (ly_strequal(rfn->dflt[r], value, 1)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", value);
                    goto error;
                }
            }
            rfn->dflt[rfn->dflt_size++] = lydict_insert(module->ctx, value, strlen(value));
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* logs directly */
static int
fill_yin_import(struct lys_module *module, struct lyxml_elem *yin, struct lys_import *imp, struct unres_schema *unres)
{
    struct lyxml_elem *child, *next, exts;
    const char *value;
    int r, c_ext = 0;
    void *reallocated;

    /* init */
    memset(&exts, 0, sizeof exts);

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns) {
            /* garbage */
            continue;
        } else if (strcmp(child->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            lyxml_unlink_elem(module->ctx, child, 2);
            lyxml_add_child(module->ctx, &exts, child);
        } else if (!strcmp(child->name, "prefix")) {
            GETVAL(value, child, "value");
            if (lyp_check_identifier(value, LY_IDENT_PREFIX, module, NULL)) {
                goto error;
            }
            imp->prefix = lydict_insert(module->ctx, value, strlen(value));

            if (read_yin_subnode_ext(module, imp, LYEXT_PAR_IMPORT, child, LYEXT_SUBSTMT_PREFIX, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(child->name, "revision-date")) {
            if (imp->rev[0]) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "date");
            if (lyp_check_date(value)) {
                goto error;
            }
            memcpy(imp->rev, value, LY_REV_SIZE - 1);

            if (read_yin_subnode_ext(module, imp, LYEXT_PAR_IMPORT, child, LYEXT_SUBSTMT_REVISIONDATE, 0, unres)) {
                goto error;
            }
        } else if ((module->version >= 2) && !strcmp(child->name, "description")) {
            if (imp->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            if (read_yin_subnode_ext(module, imp, LYEXT_PAR_IMPORT, child, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
                goto error;
            }
            imp->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!imp->dsc) {
                goto error;
            }
        } else if ((module->version >= 2) && !strcmp(child->name, "reference")) {
            if (imp->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            if (read_yin_subnode_ext(module, imp, LYEXT_PAR_IMPORT, child, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
                goto error;
            }
            imp->ref = read_yin_subnode(module->ctx, child, "text");
            if (!imp->ref) {
                goto error;
            }
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

    /* process extensions */
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(imp->ext, (c_ext + imp->ext_size) * sizeof *imp->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        imp->ext = reallocated;

        /* init memory */
        memset(&imp->ext[imp->ext_size], 0, c_ext * sizeof *imp->ext);

        LY_TREE_FOR_SAFE(exts.child, next, child) {
            /* extension */
            r = lyp_yin_fill_ext(imp, LYEXT_PAR_IMPORT, 0, 0, module, child, &imp->ext, imp->ext_size, unres);
            imp->ext_size++;
            if (r) {
                goto error;
            }
        }
    }

    GETVAL(value, yin, "module");
    return lyp_check_import(module, value, imp);

error:

    while (exts.child) {
        lyxml_free(module->ctx, exts.child);
    }
    return EXIT_FAILURE;
}

/* logs directly
 * returns:
 *  0 - inc successfully filled
 * -1 - error
 */
static int
fill_yin_include(struct lys_module *module, struct lys_submodule *submodule, struct lyxml_elem *yin,
                 struct lys_include *inc, struct unres_schema *unres)
{
    struct lyxml_elem *child, *next, exts;
    const char *value;
    int r, c_ext = 0;
    void *reallocated;

    /* init */
    memset(&exts, 0, sizeof exts);

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns) {
            /* garbage */
            continue;
        } else if (strcmp(child->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            lyxml_unlink_elem(module->ctx, child, 2);
            lyxml_add_child(module->ctx, &exts, child);
        } else if (!strcmp(child->name, "revision-date")) {
            if (inc->rev[0]) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", yin->name);
                goto error;
            }
            GETVAL(value, child, "date");
            if (lyp_check_date(value)) {
                goto error;
            }
            memcpy(inc->rev, value, LY_REV_SIZE - 1);

            if (read_yin_subnode_ext(module, inc, LYEXT_PAR_INCLUDE, child, LYEXT_SUBSTMT_REVISIONDATE, 0, unres)) {
                goto error;
            }
        } else if ((module->version >= 2) && !strcmp(child->name, "description")) {
            if (inc->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            if (read_yin_subnode_ext(module, inc, LYEXT_PAR_INCLUDE, child, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
                goto error;
            }
            inc->dsc = read_yin_subnode(module->ctx, child, "text");
            if (!inc->dsc) {
                goto error;
            }
        } else if ((module->version >= 2) && !strcmp(child->name, "reference")) {
            if (inc->ref) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            if (read_yin_subnode_ext(module, inc, LYEXT_PAR_INCLUDE, child, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
                goto error;
            }
            inc->ref = read_yin_subnode(module->ctx, child, "text");
            if (!inc->ref) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    /* process extensions */
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(inc->ext, (c_ext + inc->ext_size) * sizeof *inc->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        inc->ext = reallocated;

        /* init memory */
        memset(&inc->ext[inc->ext_size], 0, c_ext * sizeof *inc->ext);

        LY_TREE_FOR_SAFE(exts.child, next, child) {
            /* extension */
            r = lyp_yin_fill_ext(inc, LYEXT_PAR_INCLUDE, 0, 0, module, child, &inc->ext, inc->ext_size, unres);
            inc->ext_size++;
            if (r) {
                goto error;
            }
        }
    }

    GETVAL(value, yin, "module");
    return lyp_check_include(submodule ? (struct lys_module *)submodule : module, value, inc, unres);

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
read_yin_common(struct lys_module *module, struct lys_node *parent, void *stmt, LYEXT_PAR stmt_type,
                struct lyxml_elem *xmlnode, int opt, struct unres_schema *unres)
{
    struct lys_node *node = stmt;
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

    /* process local parameters */
    LY_TREE_FOR_SAFE(xmlnode->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
            lyxml_free(ctx, sub);
            continue;
        }
        if  (strcmp(sub->ns->value, LY_NSYIN)) {
            /* possibly an extension, keep the node for later processing, so skipping lyxml_free() */
            continue;
        }

        if (!strcmp(sub->name, "description")) {
            if (node->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, sub->name, xmlnode->name);
                goto error;
            }

            if (read_yin_subnode_ext(module, stmt, stmt_type, sub, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
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

            if (read_yin_subnode_ext(module, stmt, stmt_type, sub, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
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

            if (read_yin_subnode_ext(module, stmt, stmt_type, sub, LYEXT_SUBSTMT_STATUS, 0, unres)) {
                goto error;
            }
        } else if ((opt & (OPT_CFG_PARSE | OPT_CFG_IGNORE)) && !strcmp(sub->name, "config")) {
            if (opt & OPT_CFG_PARSE) {
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

                if (read_yin_subnode_ext(module, stmt, stmt_type, sub, LYEXT_SUBSTMT_CONFIG, 0, unres)) {
                    goto error;
                }
            }
        } else {
            /* skip the lyxml_free */
            continue;
        }
        lyxml_free(ctx, sub);
    }

    if ((opt & OPT_CFG_INHERIT) && !(node->flags & LYS_CONFIG_MASK)) {
        /* get config flag from parent */
        if (parent) {
            node->flags |= parent->flags & LYS_CONFIG_MASK;
        } else if (!parent) {
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
read_yin_when(struct lys_module *module, struct lyxml_elem *yin, struct unres_schema *unres)
{
    struct lys_when *retval = NULL;
    struct lyxml_elem *child, *next;
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

    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns) {
            /* garbage */
            continue;
        } else if (strcmp(child->ns->value, LY_NSYIN)) {
            /* extensions */
            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_WHEN, child, LYEXT_SUBSTMT_SELF, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(child->name, "description")) {
            if (retval->dsc) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_WHEN, child, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
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

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_WHEN, child, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
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
read_yin_case(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int valid_config,
              struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node_case *cs;
    struct lys_node *retval, *node = NULL;
    int c_ftrs = 0, c_ext = 0, ret;
    void *reallocated;

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

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin,
            OPT_IDENT | OPT_MODULE | (valid_config ? OPT_CFG_INHERIT : 0), unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
        } else if (!strcmp(sub->name, "container") ||
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
        } else if (!strcmp(sub->name, "when")) {
            if (cs->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            cs->when = read_yin_when(module, sub, unres);
            if (!cs->when) {
                goto error;
            }

            lyxml_free(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
            goto error;
        }
    }

    if (c_ftrs) {
        cs->iffeature = calloc(c_ftrs, sizeof *cs->iffeature);
        if (!cs->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            ret = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (ret) {
                goto error;
            }
        } else {
            /* if-feature */
            ret = fill_yin_iffeature(retval, 0, sub, &cs->iffeature[cs->iffeature_size], unres);
            cs->iffeature_size++;
            if (ret) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYXML, valid_config, unres);
        } else if (!strcmp(sub->name, "anydata")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYDATA, valid_config, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    /* check XPath dependencies */
    if (cs->when && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
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
read_yin_choice(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int valid_config,
                struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, *dflt = NULL;
    struct ly_ctx *const ctx = module->ctx;
    struct lys_node *retval, *node = NULL;
    struct lys_node_choice *choice;
    const char *value;
    int f_mand = 0, c_ftrs = 0, c_ext = 0, ret;
    void *reallocated;

    choice = calloc(1, sizeof *choice);
    if (!choice) {
        LOGMEM;
        return NULL;
    }
    choice->nodetype = LYS_CHOICE;
    choice->prev = (struct lys_node *)choice;
    retval = (struct lys_node *)choice;

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin,
            OPT_IDENT | OPT_MODULE | (valid_config ? OPT_CFG_PARSE | OPT_CFG_INHERIT : OPT_CFG_IGNORE), unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process choice's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            /* keep it for later processing, skip lyxml_free() */
            continue;
        } else if (!strcmp(sub->name, "container")) {
            if (!(node = read_yin_container(module, retval, sub, valid_config, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "leaf-list")) {
            if (!(node = read_yin_leaflist(module, retval, sub, valid_config, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "leaf")) {
            if (!(node = read_yin_leaf(module, retval, sub, valid_config, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "list")) {
            if (!(node = read_yin_list(module, retval, sub, valid_config, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "case")) {
            if (!(node = read_yin_case(module, retval, sub, valid_config, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "anyxml")) {
            if (!(node = read_yin_anydata(module, retval, sub, LYS_ANYXML, valid_config, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "anydata")) {
            if (!(node = read_yin_anydata(module, retval, sub, LYS_ANYDATA, valid_config, unres))) {
                goto error;
            }
        } else if (!strcmp(sub->name, "default")) {
            if (dflt) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_DEFAULT, 0, unres)) {
                goto error;
            }

            dflt = sub;
            lyxml_unlink_elem(ctx, dflt, 0);
            continue;
            /* skip lyxml_free() at the end of the loop, the sub node is processed later as dflt */

        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_MANDATORY, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "when")) {
            if (choice->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            choice->when = read_yin_when(module, sub, unres);
            if (!choice->when) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;

            /* skip lyxml_free() at the end of the loop, the sub node is processed later */
            continue;
        } else if (module->version >= 2 && !strcmp(sub->name, "choice")) {
            if (!(node = read_yin_choice(module, retval, sub, valid_config, unres))) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
            goto error;
        }

        node = NULL;
        lyxml_free(ctx, sub);
    }

    if (c_ftrs) {
        choice->iffeature = calloc(c_ftrs, sizeof *choice->iffeature);
        if (!choice->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            ret = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (ret) {
                goto error;
            }
        } else {
            ret = fill_yin_iffeature(retval, 0, sub, &choice->iffeature[choice->iffeature_size], unres);
            choice->iffeature_size++;
            if (ret) {
                goto error;
            }
        }
    }

    /* check - default is prohibited in combination with mandatory */
    if (dflt && (choice->flags & LYS_MAND_TRUE)) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, retval, "default", "choice");
        LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "The \"default\" statement is forbidden on choices with \"mandatory\".");
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

    /* check XPath dependencies */
    if (choice->when && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return retval;

error:

    lyxml_free(ctx, dflt);
    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_anydata(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, LYS_NODE type,
                 int valid_config, struct unres_schema *unres)
{
    struct lys_node *retval;
    struct lys_node_anydata *anyxml;
    struct lyxml_elem *sub, *next;
    const char *value;
    int r;
    int f_mand = 0;
    int c_must = 0, c_ftrs = 0, c_ext = 0;
    void *reallocated;

    anyxml = calloc(1, sizeof *anyxml);
    if (!anyxml) {
        LOGMEM;
        return NULL;
    }
    anyxml->nodetype = type;
    anyxml->prev = (struct lys_node *)anyxml;
    retval = (struct lys_node *)anyxml;

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin,
            OPT_IDENT | OPT_MODULE | (valid_config ? OPT_CFG_PARSE | OPT_CFG_INHERIT : OPT_CFG_IGNORE), unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                goto error;
            }
            /* else false is the default value, so we can ignore it */

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_MANDATORY, 0, unres)) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (anyxml->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            anyxml->when = read_yin_when(module, sub, unres);
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
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
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
        anyxml->iffeature = calloc(c_ftrs, sizeof *anyxml->iffeature);
        if (!anyxml->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &anyxml->must[anyxml->must_size], unres);
            anyxml->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, 0, sub, &anyxml->iffeature[anyxml->iffeature_size], unres);
            anyxml->iffeature_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* check XPath dependencies */
    if ((anyxml->when || anyxml->must_size) && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_leaf(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int valid_config,
              struct unres_schema *unres)
{
    struct lys_node *retval;
    struct lys_node_leaf *leaf;
    struct lyxml_elem *sub, *next;
    const char *value;
    int r, has_type = 0;
    int c_must = 0, c_ftrs = 0, f_mand = 0, c_ext = 0;
    void *reallocated;

    leaf = calloc(1, sizeof *leaf);
    if (!leaf) {
        LOGMEM;
        return NULL;
    }
    leaf->nodetype = LYS_LEAF;
    leaf->prev = (struct lys_node *)leaf;
    retval = (struct lys_node *)leaf;

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin,
            OPT_IDENT | OPT_MODULE | (valid_config ? OPT_CFG_PARSE | OPT_CFG_INHERIT : OPT_CFG_IGNORE), unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;
        } else if (!strcmp(sub->name, "type")) {
            if (has_type) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            /* HACK for unres */
            leaf->type.der = (struct lys_tpdf *)sub;
            leaf->type.parent = (struct lys_tpdf *)leaf;
            /* postpone type resolution when if-feature parsing is done since we need
             * if-feature for check_leafref_features() */
            has_type = 1;
        } else if (!strcmp(sub->name, "default")) {
            if (leaf->dflt) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            leaf->dflt = lydict_insert(module->ctx, value, strlen(value));

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_DEFAULT, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "units")) {
            if (leaf->units) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            leaf->units = lydict_insert(module->ctx, value, strlen(value));

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_UNITS, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "mandatory")) {
            if (f_mand) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                goto error;
            }                   /* else false is the default value, so we can ignore it */

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_MANDATORY, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "when")) {
            if (leaf->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            leaf->when = read_yin_when(module, sub, unres);
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
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
            goto error;
        }

        /* do not free sub, it could have been unlinked and stored in unres */
    }

    /* check mandatory parameters */
    if (!has_type) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, retval, "type", yin->name);
        goto error;
    }
    if (leaf->dflt && (leaf->flags & LYS_MAND_TRUE)) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, retval, "mandatory", "leaf");
        LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL,
               "The \"mandatory\" statement is forbidden on leaf with the \"default\" statement.");
        goto error;
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
        leaf->iffeature = calloc(c_ftrs, sizeof *leaf->iffeature);
        if (!leaf->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &leaf->must[leaf->must_size], unres);
            leaf->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, 0, sub, &leaf->iffeature[leaf->iffeature_size], unres);
            leaf->iffeature_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* finalize type parsing */
    if (unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DER, retval) == -1) {
        leaf->type.der = NULL;
        goto error;
    }

    /* check default value (if not defined, there still could be some restrictions
     * that need to be checked against a default value from a derived type) */
    if (unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DFLT, (struct lys_node *)(&leaf->dflt)) == -1) {
        goto error;
    }

    /* check XPath dependencies */
    if ((leaf->when || leaf->must_size) && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_leaflist(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int valid_config,
                  struct unres_schema *unres)
{
    struct lys_node *retval;
    struct lys_node_leaflist *llist;
    struct lyxml_elem *sub, *next;
    const char *value;
    char *endptr;
    unsigned long val;
    int r, has_type = 0;
    int c_must = 0, c_ftrs = 0, c_dflt = 0, c_ext = 0;
    int f_ordr = 0, f_min = 0, f_max = 0;
    void *reallocated;

    llist = calloc(1, sizeof *llist);
    if (!llist) {
        LOGMEM;
        return NULL;
    }
    llist->nodetype = LYS_LEAFLIST;
    llist->prev = (struct lys_node *)llist;
    retval = (struct lys_node *)llist;

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin,
            OPT_IDENT | OPT_MODULE | (valid_config ? OPT_CFG_PARSE | OPT_CFG_INHERIT : OPT_CFG_IGNORE), unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;
        } else if (!strcmp(sub->name, "type")) {
            if (has_type) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            /* HACK for unres */
            llist->type.der = (struct lys_tpdf *)sub;
            llist->type.parent = (struct lys_tpdf *)llist;
            /* postpone type resolution when if-feature parsing is done since we need
             * if-feature for check_leafref_features() */
            has_type = 1;
        } else if (!strcmp(sub->name, "units")) {
            if (llist->units) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "name");
            llist->units = lydict_insert(module->ctx, value, strlen(value));

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_UNITS, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "ordered-by")) {
            if (f_ordr) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                goto error;
            } /* else system is the default value, so we can ignore it */

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_ORDEREDBY, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            c_must++;
            continue;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
            continue;
        } else if ((module->version >= 2) && !strcmp(sub->name, "default")) {
            /* read the default's extension instances */
            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_DEFAULT, c_dflt, unres)) {
                goto error;
            }

            c_dflt++;
            continue;

        } else if (!strcmp(sub->name, "min-elements")) {
            if (f_min) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                goto error;
            }
            llist->min = (uint32_t) val;
            if (llist->max && (llist->min > llist->max)) {
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                goto error;
            }

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_MIN, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                    LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                    goto error;
                }
                llist->max = (uint32_t) val;
                if (llist->min > llist->max) {
                    LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                    goto error;
                }
            }

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_MAX, 0, unres)) {
                goto error;
            }
        } else if (!strcmp(sub->name, "when")) {
            if (llist->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            llist->when = read_yin_when(module, sub, unres);
            if (!llist->when) {
                goto error;
            }
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
            goto error;
        }

        /* do not free sub, it could have been unlinked and stored in unres */
    }

    /* check constraints */
    if (!has_type) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, retval, "type", yin->name);
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
        llist->iffeature = calloc(c_ftrs, sizeof *llist->iffeature);
        if (!llist->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_dflt) {
        llist->dflt = calloc(c_dflt, sizeof *llist->dflt);
        if (!llist->dflt) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &llist->must[llist->must_size], unres);
            llist->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, 0, sub, &llist->iffeature[llist->iffeature_size], unres);
            llist->iffeature_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "default")) {
            GETVAL(value, sub, "value");

            /* check for duplicity in case of configuration data,
             * in case of status data duplicities are allowed */
            if (llist->flags & LYS_CONFIG_W) {
                for (r = 0; r < llist->dflt_size; r++) {
                    if (ly_strequal(llist->dflt[r], value, 1)) {
                        LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, "default");
                        LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "Duplicated default value \"%s\".", value);
                        goto error;
                    }
                }
            }
            llist->dflt[llist->dflt_size++] = lydict_insert(module->ctx, value, strlen(value));
        }
    }

    /* finalize type parsing */
    if (unres_schema_add_node(module, unres, &llist->type, UNRES_TYPE_DER, retval) == -1) {
        llist->type.der = NULL;
        goto error;
    }

    if (llist->dflt_size && llist->min) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, retval, "min-elements", "leaf-list");
        LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL,
               "The \"min-elements\" statement with non-zero value is forbidden on leaf-lists with the \"default\" statement.");
        goto error;
    }

    /* check default value (if not defined, there still could be some restrictions
     * that need to be checked against a default value from a derived type) */
    for (r = 0; r < llist->dflt_size; r++) {
        if (unres_schema_add_node(module, unres, &llist->type, UNRES_TYPE_DFLT,
                                  (struct lys_node *)(&llist->dflt[r])) == -1) {
            goto error;
        }
    }

    /* check XPath dependencies */
    if ((llist->when || llist->must_size) && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return retval;

error:

    lys_node_free(retval, NULL, 0);

    return NULL;
}

/* logs directly */
static struct lys_node *
read_yin_list(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int valid_config,
              struct unres_schema *unres)
{
    struct lys_node *retval, *node;
    struct lys_node_list *list;
    struct lyxml_elem *sub, *next, root, uniq;
    int r;
    int c_tpdf = 0, c_must = 0, c_uniq = 0, c_ftrs = 0, c_ext = 0;
    int f_ordr = 0, f_max = 0, f_min = 0;
    const char *value;
    char *auxs;
    unsigned long val;
    void *reallocated;

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

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin,
            OPT_IDENT | OPT_MODULE | (valid_config ? OPT_CFG_PARSE | OPT_CFG_INHERIT : OPT_CFG_IGNORE), unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process list's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;

        /* data statements */
        } else if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml") ||
                !strcmp(sub->name, "action") ||
                !strcmp(sub->name, "notification")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "key")) {
            /* check cardinality 0..1 */
            if (list->keys_size) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, list->name);
                goto error;
            }

            /* count the number of keys */
            GETVAL(value, sub, "value");
            list->keys_str = lydict_insert(module->ctx, value, 0);
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

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_KEY, 0, unres)) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
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
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                goto error;
            } /* else system is the default value, so we can ignore it */

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_ORDEREDBY, 0, unres)) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "min-elements")) {
            if (f_min) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                goto error;
            }
            list->min = (uint32_t) val;
            if (list->max && (list->min > list->max)) {
                LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                lyxml_free(module->ctx, sub);
                goto error;
            }
            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_MIN, 0, unres)) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "max-elements")) {
            if (f_max) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
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
                    LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                    goto error;
                }
                list->max = (uint32_t) val;
                if (list->min > list->max) {
                    LOGVAL(LYE_INARG, LY_VLOG_LYS, retval, value, sub->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "\"max-elements\" is smaller than \"min-elements\".");
                    goto error;
                }
            }
            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_MAX, 0, unres)) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (list->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            list->when = read_yin_when(module, sub, unres);
            if (!list->when) {
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
            goto error;
        }
    }

    /* check - if list is configuration, key statement is mandatory
     * (but only if we are not in a grouping or augment, then the check is deferred) */
    for (node = retval; node && !(node->nodetype & (LYS_GROUPING | LYS_AUGMENT)); node = node->parent);
    if (!node && (list->flags & LYS_CONFIG_W) && !list->keys_str) {
        LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, retval, "key", "list");
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
        list->iffeature = calloc(c_ftrs, sizeof *list->iffeature);
        if (!list->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &list->tpdf[list->tpdf_size], unres);
            list->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, 0, sub, &list->iffeature[list->iffeature_size], unres);
            list->iffeature_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &list->must[list->must_size], unres);
            list->must_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYXML, valid_config, unres);
        } else if (!strcmp(sub->name, "anydata")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYDATA, valid_config, unres);
        } else if (!strcmp(sub->name, "action")) {
            node = read_yin_rpc_action(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "notification")) {
            node = read_yin_notif(module, retval, sub, unres);
        } else {
            LOGINT;
            goto error;
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    if (list->keys_str) {
        /* check that we are not in grouping */
        for (node = parent; node && node->nodetype != LYS_GROUPING; node = lys_parent(node));
        if (!node && unres_schema_add_node(module, unres, list, UNRES_LIST_KEYS, NULL) == -1) {
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

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub,
                                     LYEXT_SUBSTMT_UNIQUE, list->unique_size - 1, unres)) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
        }
    }

    /* check XPath dependencies */
    if ((list->when || list->must_size) && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
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
read_yin_container(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int valid_config,
                   struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_container *cont;
    const char *value;
    void *reallocated;
    int r;
    int c_tpdf = 0, c_must = 0, c_ftrs = 0, c_ext = 0;

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

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin,
            OPT_IDENT | OPT_MODULE | (valid_config ? OPT_CFG_PARSE | OPT_CFG_INHERIT : OPT_CFG_IGNORE), unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process container's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
        } else if (!strcmp(sub->name, "presence")) {
            if (cont->presence) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            GETVAL(value, sub, "value");
            cont->presence = lydict_insert(module->ctx, value, strlen(value));

            if (read_yin_subnode_ext(module, retval, LYEXT_PAR_NODE, sub, LYEXT_SUBSTMT_PRESENCE, 0, unres)) {
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else if (!strcmp(sub->name, "when")) {
            if (cont->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            cont->when = read_yin_when(module, sub, unres);
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
                !strcmp(sub->name, "anyxml") ||
                !strcmp(sub->name, "action") ||
                !strcmp(sub->name, "notification")) {
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
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
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
        cont->iffeature = calloc(c_ftrs, sizeof *cont->iffeature);
        if (!cont->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &cont->tpdf[cont->tpdf_size], unres);
            cont->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &cont->must[cont->must_size], unres);
            cont->must_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, 0, sub, &cont->iffeature[cont->iffeature_size], unres);
            cont->iffeature_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYXML, valid_config, unres);
        } else if (!strcmp(sub->name, "anydata")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYDATA, valid_config, unres);
        } else if (!strcmp(sub->name, "action")) {
            node = read_yin_rpc_action(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "notification")) {
            node = read_yin_notif(module, retval, sub, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    /* check XPath dependencies */
    if ((cont->when || cont->must_size) && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
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
read_yin_grouping(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, int valid_config,
                  struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_grp *grp;
    int r;
    int c_tpdf = 0, c_ext = 0;
    void *reallocated;

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

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin, OPT_IDENT | OPT_MODULE , unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;

        /* data statements */
        } else if (!strcmp(sub->name, "container") ||
                !strcmp(sub->name, "leaf-list") ||
                !strcmp(sub->name, "leaf") ||
                !strcmp(sub->name, "list") ||
                !strcmp(sub->name, "choice") ||
                !strcmp(sub->name, "uses") ||
                !strcmp(sub->name, "grouping") ||
                !strcmp(sub->name, "anyxml") ||
                !strcmp(sub->name, "action") ||
                !strcmp(sub->name, "notification")) {
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);

            /* array counters */
        } else if (!strcmp(sub->name, "typedef")) {
            c_tpdf++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
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
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else {
            /* typedef */
            r = fill_yin_typedef(module, retval, sub, &grp->tpdf[grp->tpdf_size], unres);
            grp->tpdf_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    if (!root.child) {
        LOGWRN("Grouping \"%s\" without children.", retval->name);
    }
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, valid_config, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYXML, valid_config, unres);
        } else if (!strcmp(sub->name, "anydata")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYDATA, valid_config, unres);
        } else if (!strcmp(sub->name, "action")) {
            node = read_yin_rpc_action(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "notification")) {
            node = read_yin_notif(module, retval, sub, unres);
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
read_yin_input_output(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                      struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval = NULL;
    struct lys_node_inout *inout;
    int r;
    int c_tpdf = 0, c_must = 0, c_ext = 0;

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
    retval->module = module;

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* data statements */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (!sub->ns) {
            /* garbage */
            lyxml_free(module->ctx, sub);
        } else if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
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

        } else if ((module->version >= 2) && !strcmp(sub->name, "must")) {
            c_must++;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
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
    if (c_must) {
        inout->must = calloc(c_must, sizeof *inout->must);
        if (!inout->must) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        inout->ext = calloc(c_ext, sizeof *inout->ext);
        if (!inout->ext) {
            LOGMEM;
            goto error;
        }
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &inout->must[inout->must_size], unres);
            inout->must_size++;
            if (r) {
                goto error;
            }
        } else { /* typedef */
            r = fill_yin_typedef(module, retval, sub, &inout->tpdf[inout->tpdf_size], unres);
            inout->tpdf_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYXML, 0, unres);
        } else if (!strcmp(sub->name, "anydata")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYDATA, 0, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    /* check XPath dependencies */
    if (inout->must_size && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
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
read_yin_notif(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
               struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_notif *notif;
    int r;
    int c_tpdf = 0, c_ftrs = 0, c_must = 0, c_ext = 0;
    void *reallocated;

    if (parent && (module->version < 2)) {
        LOGVAL(LYE_INSTMT, LY_VLOG_LYS, parent, "notification");
        return NULL;
    }

    memset(&root, 0, sizeof root);

    notif = calloc(1, sizeof *notif);
    if (!notif) {
        LOGMEM;
        return NULL;
    }
    notif->nodetype = LYS_NOTIF;
    notif->prev = (struct lys_node *)notif;
    retval = (struct lys_node *)notif;

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin, OPT_IDENT | OPT_MODULE, unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;

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
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else if ((module->version >= 2) && !strcmp(sub->name, "must")) {
            c_must++;
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
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
        notif->iffeature = calloc(c_ftrs, sizeof *notif->iffeature);
        if (!notif->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_must) {
        notif->must = calloc(c_must, sizeof *notif->must);
        if (!notif->must) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &notif->tpdf[notif->tpdf_size], unres);
            notif->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, 0, sub, &notif->iffeature[notif->iffeature_size], unres);
            notif->iffeature_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "must")) {
            r = fill_yin_must(module, sub, &notif->must[notif->must_size], unres);
            notif->must_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "container")) {
            node = read_yin_container(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "leaf-list")) {
            node = read_yin_leaflist(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "leaf")) {
            node = read_yin_leaf(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "list")) {
            node = read_yin_list(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "choice")) {
            node = read_yin_choice(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "uses")) {
            node = read_yin_uses(module, retval, sub, unres);
        } else if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "anyxml")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYXML, 0, unres);
        } else if (!strcmp(sub->name, "anydata")) {
            node = read_yin_anydata(module, retval, sub, LYS_ANYDATA, 0, unres);
        }
        if (!node) {
            goto error;
        }

        lyxml_free(module->ctx, sub);
    }

    /* check XPath dependencies */
    if (notif->must_size && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
        goto error;
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
read_yin_rpc_action(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin,
                    struct unres_schema *unres)
{
    struct lyxml_elem *sub, *next, root;
    struct lys_node *node = NULL;
    struct lys_node *retval;
    struct lys_node_rpc_action *rpc;
    int r;
    int c_tpdf = 0, c_ftrs = 0, c_input = 0, c_output = 0, c_ext = 0;
    void *reallocated;

    if (!strcmp(yin->name, "action")) {
        if (module->version < 2) {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, parent, "action");
            return NULL;
        }
        for (node = parent; node; node = lys_parent(node)) {
            if ((node->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))
                    || ((node->nodetype == LYS_LIST) && !((struct lys_node_list *)node)->keys_size)) {
                LOGVAL(LYE_INPAR, LY_VLOG_LYS, parent, strnodetype(node->nodetype), "action");
                return NULL;
            }
        }
    }

    /* init */
    memset(&root, 0, sizeof root);

    rpc = calloc(1, sizeof *rpc);
    if (!rpc) {
        LOGMEM;
        return NULL;
    }
    rpc->nodetype = (!strcmp(yin->name, "rpc") ? LYS_RPC : LYS_ACTION);
    rpc->prev = (struct lys_node *)rpc;
    retval = (struct lys_node *)rpc;

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin, OPT_IDENT | OPT_MODULE, unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* process rpc's specific children */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;
        } else if (!strcmp(sub->name, "input")) {
            if (c_input) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            c_input++;
            lyxml_unlink_elem(module->ctx, sub, 2);
            lyxml_add_child(module->ctx, &root, sub);
        } else if (!strcmp(sub->name, "output")) {
            if (c_output) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }
            c_output++;
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
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
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
        rpc->iffeature = calloc(c_ftrs, sizeof *rpc->iffeature);
        if (!rpc->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "typedef")) {
            r = fill_yin_typedef(module, retval, sub, &rpc->tpdf[rpc->tpdf_size], unres);
            rpc->tpdf_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "if-feature")) {
            r = fill_yin_iffeature(retval, 0, sub, &rpc->iffeature[rpc->iffeature_size], unres);
            rpc->iffeature_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* last part - process data nodes */
    LY_TREE_FOR_SAFE(root.child, next, sub) {
        if (!strcmp(sub->name, "grouping")) {
            node = read_yin_grouping(module, retval, sub, 0, unres);
        } else if (!strcmp(sub->name, "input") || !strcmp(sub->name, "output")) {
            node = read_yin_input_output(module, retval, sub, unres);
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
    int c_ref = 0, c_aug = 0, c_ftrs = 0, c_ext = 0;
    int r;
    void *reallocated;

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

    if (read_yin_common(module, parent, retval, LYEXT_PAR_NODE, yin, OPT_MODULE, unres)) {
        goto error;
    }

    LOGDBG("YIN: parsing %s statement \"%s\"", yin->name, retval->name);

    /* insert the node into the schema tree */
    if (lys_node_addchild(parent, lys_main_module(module), retval)) {
        goto error;
    }

    /* get other properties of uses */
    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            c_ext++;
            continue;
        } else if (!strcmp(sub->name, "refine")) {
            c_ref++;
        } else if (!strcmp(sub->name, "augment")) {
            c_aug++;
        } else if (!strcmp(sub->name, "if-feature")) {
            c_ftrs++;
        } else if (!strcmp(sub->name, "when")) {
            if (uses->when) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_LYS, retval, sub->name, yin->name);
                goto error;
            }

            uses->when = read_yin_when(module, sub, unres);
            if (!uses->when) {
                lyxml_free(module->ctx, sub);
                goto error;
            }
            lyxml_free(module->ctx, sub);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_LYS, retval, sub->name);
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
        uses->iffeature = calloc(c_ftrs, sizeof *uses->iffeature);
        if (!uses->iffeature) {
            LOGMEM;
            goto error;
        }
    }
    if (c_ext) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(retval->ext, (c_ext + retval->ext_size) * sizeof *retval->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        retval->ext = reallocated;

        /* init memory */
        memset(&retval->ext[retval->ext_size], 0, c_ext * sizeof *retval->ext);
    }

    LY_TREE_FOR_SAFE(yin->child, next, sub) {
        if (strcmp(sub->ns->value, LY_NSYIN)) {
            /* extension */
            r = lyp_yin_fill_ext(retval, LYEXT_PAR_NODE, 0, 0, module, sub, &retval->ext, retval->ext_size, unres);
            retval->ext_size++;
            if (r) {
                goto error;
            }
        } else if (!strcmp(sub->name, "refine")) {
            r = fill_yin_refine(retval, sub, &uses->refine[uses->refine_size], unres);
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
            r = fill_yin_iffeature(retval, 0, sub, &uses->iffeature[uses->iffeature_size], unres);
            uses->iffeature_size++;
            if (r) {
                goto error;
            }
        }
    }

    if (unres_schema_add_node(module, unres, uses, UNRES_USES, NULL) == -1) {
        goto error;
    }

    /* check XPath dependencies */
    if (uses->when && (unres_schema_add_node(module, unres, retval, UNRES_XPATH, NULL) == -1)) {
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
    struct lyxml_elem *next, *child, *next2, *child2, root, grps, augs, exts;
    struct lys_node *node = NULL;
    struct lys_module *trg;
    const char *value;
    int i, r;
    int version_flag = 0;
    /* counters */
    int c_imp = 0, c_rev = 0, c_tpdf = 0, c_ident = 0, c_inc = 0, c_aug = 0, c_ftrs = 0, c_dev = 0;
    int c_ext = 0, c_extinst = 0;
    void *reallocated;

    /* to simplify code, store the module/submodule being processed as trg */
    trg = submodule ? (struct lys_module *)submodule : module;

    /* init */
    memset(&root, 0, sizeof root);
    memset(&grps, 0, sizeof grps);
    memset(&augs, 0, sizeof augs);
    memset(&exts, 0, sizeof exts);

    /*
     * in the first run, we process elements with cardinality of 1 or 0..1 and
     * count elements with cardinality 0..n. Data elements (choices, containers,
     * leafs, lists, leaf-lists) are moved aside to be processed last, since we
     * need have all top-level and groupings already prepared at that time. In
     * the middle loop, we process other elements with carinality of 0..n since
     * we need to allocate arrays to store them.
     */
    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!child->ns) {
            /* garbage */
            lyxml_free(ctx, child);
            continue;
        } else if  (strcmp(child->ns->value, LY_NSYIN)) {
            /* possible extension instance */
            lyxml_unlink_elem(module->ctx, child, 2);
            lyxml_add_child(module->ctx, &exts, child);
            c_extinst++;
        } else if (!submodule && !strcmp(child->name, "namespace")) {
            if (module->ns) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "uri");
            module->ns = lydict_insert(ctx, value, strlen(value));

            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_NAMESPACE, 0, unres)) {
                goto error;
            }
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

            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_PREFIX, 0, unres)) {
                goto error;
            }
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

            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_BELONGSTO, 0, unres)) {
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

            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child->child, LYEXT_SUBSTMT_PREFIX, 0, unres)) {
                goto error;
            }

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
            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
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
            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
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
            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_ORGANIZATION, 0, unres)) {
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
            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_CONTACT, 0, unres)) {
                goto error;
            }
            trg->contact = read_yin_subnode(ctx, child, "text");
            lyxml_free(ctx, child);
            if (!trg->contact) {
                goto error;
            }
        } else if (!strcmp(child->name, "yang-version")) {
            if (version_flag) {
                LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child->name, yin->name);
                goto error;
            }
            GETVAL(value, child, "value");
            if (strcmp(value, "1") && strcmp(value, "1.1")) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "yang-version");
                goto error;
            }
            version_flag = 1;
            if (!strcmp(value, "1")) {
                if (submodule) {
                    if (module->version > 1) {
                        LOGVAL(LYE_INVER, LY_VLOG_NONE, NULL);
                        goto error;
                    }
                } else {
                    module->version = 1;
                }
            } else {
                if (submodule) {
                    if (module->version < 2) {
                        LOGVAL(LYE_INVER, LY_VLOG_NONE, NULL);
                        goto error;
                    }
                } else {
                    module->version = 2;
                }
            }

            if (read_yin_subnode_ext(trg, trg, LYEXT_PAR_MODULE, child, LYEXT_SUBSTMT_VERSION, 0, unres)) {
                goto error;
            }
            lyxml_free(ctx, child);

        } else if (!strcmp(child->name, "extension")) {
            c_ext++;

        } else if (!strcmp(child->name, "deviation")) {
            c_dev++;

        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, child->name);
            goto error;
        }
    }

    /* check for mandatory statements */
    if (submodule) {
        if (!submodule->prefix) {
            LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
            goto error;
        }
        if (!version_flag) {
            /* check version compatibility with the main module */
            if (module->version > 1) {
                LOGVAL(LYE_INVER, LY_VLOG_NONE, NULL);
                goto error;
            }
        }
    } else {
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
        trg->imp = calloc(c_imp, sizeof *trg->imp);
        if (!trg->imp) {
            LOGMEM;
            goto error;
        }
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
        trg->inc = calloc(c_inc, sizeof *trg->inc);
        if (!trg->inc) {
            LOGMEM;
            goto error;
        }
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
    if (c_ext) {
        trg->extensions = calloc(c_ext, sizeof *trg->extensions);
        if (!trg->extensions) {
            LOGMEM;
            goto error;
        }
    }
    if (c_extinst) {
        /* some extensions may be already present from the substatements */
        reallocated = realloc(trg->ext, (c_extinst + trg->ext_size) * sizeof *trg->ext);
        if (!reallocated) {
            LOGMEM;
            goto error;
        }
        trg->ext = reallocated;

        /* init memory */
        memset(&trg->ext[trg->ext_size], 0, c_extinst * sizeof *trg->ext);
    }

    /* middle part - process nodes with cardinality of 0..n except the data nodes and augments */
    LY_TREE_FOR_SAFE(yin->child, next, child) {
        if (!strcmp(child->name, "import")) {
            r = fill_yin_import(trg, child, &trg->imp[trg->imp_size], unres);
            trg->imp_size++;
            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "include")) {
            r = fill_yin_include(trg, submodule, child, &trg->inc[trg->inc_size], unres);
            trg->inc_size++;
            if (r) {
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

            LY_TREE_FOR_SAFE(child->child, next2, child2) {
                if (!child2->ns) {
                    /* garbage */
                    continue;
                } else if (strcmp(child2->ns->value, LY_NSYIN)) {
                    /* possible extension instance */
                    if (read_yin_subnode_ext(trg, &trg->rev[trg->rev_size], LYEXT_PAR_REVISION,
                                             child2, LYEXT_SUBSTMT_SELF, 0, unres)) {
                        goto error;
                    }
                } else if (!strcmp(child2->name, "description")) {
                    if (trg->rev[trg->rev_size].dsc) {
                        LOGVAL(LYE_TOOMANY, LY_VLOG_NONE, NULL, child2->name, child->name);
                        goto error;
                    }
                    if (read_yin_subnode_ext(trg, &trg->rev[trg->rev_size], LYEXT_PAR_REVISION,
                                             child2, LYEXT_SUBSTMT_DESCRIPTION, 0, unres)) {
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
                    if (read_yin_subnode_ext(trg, &trg->rev[trg->rev_size], LYEXT_PAR_REVISION,
                                             child2, LYEXT_SUBSTMT_REFERENCE, 0, unres)) {
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

        } else if (!strcmp(child->name, "extension")) {
            r = fill_yin_extension(trg, child, &trg->extensions[trg->extensions_size], unres);
            trg->extensions_size++;
            if (r) {
                goto error;
            }

        } else if (!strcmp(child->name, "deviation")) {
            r = fill_yin_deviation(trg, child, &trg->deviation[trg->deviation_size], unres);
            trg->deviation_size++;
            if (r) {
                goto error;
            }
        }
    }

    /* process extension instances */
    LY_TREE_FOR_SAFE(exts.child, next, child) {
        r = lyp_yin_fill_ext(trg, LYEXT_PAR_MODULE, 0, 0, trg, child, &trg->ext, trg->ext_size, unres);
        trg->ext_size++;
        if (r) {
            goto error;
        }
    }

    /* process data nodes. Start with groupings to allow uses
     * refer to them. Submodule's data nodes are stored in the
     * main module data tree.
     */
    LY_TREE_FOR_SAFE(grps.child, next, child) {
        node = read_yin_grouping(trg, NULL, child, 1, unres);
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
            node = read_yin_anydata(trg, NULL, child, LYS_ANYXML, 1, unres);
        } else if (!strcmp(child->name, "anydata")) {
            node = read_yin_anydata(trg, NULL, child, LYS_ANYDATA, 1, unres);
        } else if (!strcmp(child->name, "rpc")) {
            node = read_yin_rpc_action(trg, NULL, child, unres);
        } else if (!strcmp(child->name, "notification")) {
            node = read_yin_notif(trg, NULL, child, unres);
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
    while (exts.child) {
        lyxml_free(module->ctx, exts.child);
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

    lyp_sort_revisions((struct lys_module *)submodule);

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
        if (ly_strequal("submodule", yin->name, 0)) {
            LOGVAL(LYE_SUBMODULE, LY_VLOG_NONE, NULL);
        } else {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, yin->name);
        }
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

    lyp_sort_revisions(module);

    if (revision) {
        /* check revision of the parsed model */
        if (!module->rev_size || strcmp(revision, module->rev[0].date)) {
            LOGVRB("Module \"%s\" parsed with the wrong revision (\"%s\" instead \"%s\").",
                   module->name, module->rev[0].date, revision);
            goto error;
        }
    }

    /* check correctness of includes */
    if (lyp_check_include_missing(module)) {
        goto error;
    }

    if (lyp_ctx_add_module(&module)) {
        goto error;
    }

    if (module->deviation_size && !module->implemented) {
        LOGVRB("Module \"%s\" includes deviations, changing its conformance to \"implement\".", module->name);
        /* deviations always causes target to be made implemented,
         * but augents and leafrefs not, so we have to apply them now */
        if (lys_set_implemented(module)) {
            goto error;
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
        if (ly_vecode != LYVE_SUBMODULE) {
            LOGERR(ly_errno, "Module parsing failed.");
        }
        return NULL;
    }

    LOGERR(ly_errno, "Module \"%s\" parsing failed.", module->name);

    lys_sub_module_remove_devs_augs(module);
    lys_free(module, NULL, 1);
    return NULL;
}
