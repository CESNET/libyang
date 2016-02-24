/**
 * @file printer/info.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief INFO printer for libyang data model structure
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"
#include "resolve.h"

#define INDENT_LEN 11

static void
info_print_text(struct lyout *out, const char *text, const char *label)
{
    const char *ptr1, *ptr2;
    int first = 1;

    ly_print(out, "%-*s", INDENT_LEN, label);

    if (text) {
        ptr1 = text;
        while (1) {
            ptr2 = strchr(ptr1, '\n');
            if (!ptr2) {
                if (first) {
                    ly_print(out, "%s\n", ptr1);
                    first = 0;
                } else {
                    ly_print(out, "%*s%s\n", INDENT_LEN, "", ptr1);
                }
                break;
            }
            ++ptr2;
            if (first) {
                ly_print(out, "%.*s", (int)(ptr2-ptr1), ptr1);
                first = 0;
            } else {
                ly_print(out, "%*s%.*s", INDENT_LEN, "", (int)(ptr2-ptr1), ptr1);
            }
            ptr1 = ptr2;
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_snode(struct lyout *out, const struct lys_node *parent, const struct lys_node *node, const char *label)
{
    assert(strlen(label) < INDENT_LEN-1);

    ly_print(out, "%-*s", INDENT_LEN, label);

    if (node) {
        if (node->name) {
            ly_print(out, "%s \"", strnodetype(node->nodetype));
            if (parent != node->parent) {
                ly_print(out, "%s:", node->module->prefix);
            }
            ly_print(out, "%s\"\n", node->name);
        } else {
            ly_print(out, "%s\n", (node->nodetype == LYS_INPUT ? "input" : "output"));
        }
        node = node->next;
        for (; node; node = node->next) {
            if (node->name) {
                ly_print(out, "%*s%s \"", INDENT_LEN, "", strnodetype(node->nodetype));
                if (parent != node->parent) {
                    ly_print(out, "%s:", node->module->prefix);
                }
                ly_print(out, "%s\"\n", node->name);
            } else {
                ly_print(out, "%*s%s\n", INDENT_LEN, "", (node->nodetype == LYS_INPUT ? "input" : "output"));
            }
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_flags(struct lyout *out, uint8_t flags, uint8_t mask, int is_list)
{
    if (mask & LYS_CONFIG_MASK) {
        ly_print(out, "%-*s", INDENT_LEN, "Config: ");
        if (flags & LYS_CONFIG_R) {
            ly_print(out, "read-only\n");
        } else {
            ly_print(out, "read-write\n");
        }
    }

    if (mask & LYS_STATUS_MASK) {
        ly_print(out, "%-*s", INDENT_LEN, "Status: ");

        if (flags & LYS_STATUS_DEPRC) {
            ly_print(out, "deprecated\n");
        } else if (flags & LYS_STATUS_OBSLT) {
            ly_print(out, "obsolete\n");
        } else {
            ly_print(out, "current\n");
        }
    }

    if (mask & LYS_MAND_MASK) {
        ly_print(out, "%-*s", INDENT_LEN, "Mandatory: ");

        if (flags & LYS_MAND_TRUE) {
            ly_print(out, "yes\n");
        } else {
            ly_print(out, "no\n");
        }
    }

    if (is_list && (mask & LYS_USERORDERED)) {
        ly_print(out, "%-*s", INDENT_LEN, "Order: ");

        if (flags & LYS_USERORDERED) {
            ly_print(out, "user-ordered\n");
        } else {
            ly_print(out, "system-ordered\n");
        }
    }

    if (!is_list && (mask & LYS_FENABLED)) {
        ly_print(out, "%-*s", INDENT_LEN, "Enabled: ");

        if (flags & LYS_FENABLED) {
            ly_print(out, "yes\n");
        } else {
            ly_print(out, "no\n");
        }
    }
}

static void
info_print_if_feature(struct lyout *out, struct lys_feature ** const features, uint8_t features_size)
{
    int i;

    ly_print(out, "%-*s", INDENT_LEN, "If-feats: ");

    if (features_size) {
        ly_print(out, "%s\n", features[0]->name);
        for (i = 1; i < features_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", features[i]->name);
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_when(struct lyout *out, const struct lys_when *when)
{
    ly_print(out, "%-*s", INDENT_LEN, "When: ");
    if (when) {
        ly_print(out, "%s\n", when->cond);
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_must(struct lyout *out, const struct lys_restr *must, uint8_t must_size)
{
    int i;

    ly_print(out, "%-*s", INDENT_LEN, "Must: ");

    if (must_size) {
        ly_print(out, "%s\n", must[0].expr);
        for (i = 1; i < must_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", must[i].expr);
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_typedef(struct lyout *out, const struct lys_tpdf *tpdf, uint8_t tpdf_size)
{
    int i;

    ly_print(out, "%-*s", INDENT_LEN, "Typedefs: ");

    if (tpdf_size) {
        ly_print(out, "%s\n", tpdf[0].name);
        for (i = 1; i < tpdf_size; ++i) {
            ly_print(out, "%*s%s", INDENT_LEN, "", tpdf[i].name);
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_typedef_with_include(struct lyout *out, const struct lys_module *mod)
{
    int i, j, first = 1;

    ly_print(out, "%-*s", INDENT_LEN, "Typedefs: ");

    if (mod->tpdf_size) {
        ly_print(out, "%s\n", mod->tpdf[0].name);
        first = 0;

        for (i = 1; i < mod->tpdf_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", mod->tpdf[i].name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->tpdf_size) {
            if (first) {
                ly_print(out, "%s\n", mod->inc[i].submodule->tpdf[0].name);
                j = 1;
            } else {
                j = 0;
            }
            first = 0;

            for (; j < mod->inc[i].submodule->tpdf_size; ++j) {
                ly_print(out, "%*s%s\n", INDENT_LEN, "", mod->inc[i].submodule->tpdf[j].name);
            }
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_type_detail(struct lyout *out, const struct lys_type *type, int uni)
{
    int i;

    if (uni) {
        ly_print(out, "  ");
    }

    switch (type->base) {
    case LY_TYPE_DER:
        /* unused, but what the hack */
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "derived");
        break;
    case LY_TYPE_BINARY:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "binary");
        if (!uni) {
            info_print_text(out, (type->info.binary.length ? type->info.binary.length->expr : NULL), "Length: ");
        }
        break;
    case LY_TYPE_BITS:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "bits");

        assert(type->info.bits.count);
        if (!uni) {
            ly_print(out, "%-*s%u %s\n", INDENT_LEN, "Bits: ", type->info.bits.bit[0].pos, type->info.bits.bit[0].name);
            for (i = 1; i < type->info.bits.count; ++i) {
                ly_print(out, "%*s%u %s\n", INDENT_LEN, "", type->info.bits.bit[i].pos, type->info.bits.bit[i].name);
            }
        }

        break;
    case LY_TYPE_BOOL:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "bool");
        break;
    case LY_TYPE_DEC64:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "decimal64");
        if (!uni) {
            info_print_text(out, (type->info.dec64.range ? type->info.dec64.range->expr : NULL), "Range: ");
            assert(type->info.dec64.dig);
            ly_print(out, "%-*s%u\n", INDENT_LEN, "Frac dig: ", type->info.dec64.dig);
        }
        break;
    case LY_TYPE_EMPTY:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "empty");
        break;
    case LY_TYPE_ENUM:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "enum");

        assert(type->info.enums.count);
        if (!uni) {
            ly_print(out, "%-*s%s\n", INDENT_LEN, "Values: ", type->info.enums.enm[0].name);
            for (i = 1; i < type->info.enums.count; ++i) {
                ly_print(out, "%*s%s\n", INDENT_LEN, "", type->info.enums.enm[i].name);
            }
        }

        break;
    case LY_TYPE_IDENT:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "identityref");
        assert(type->info.ident.ref);
        if (!uni) {
            info_print_text(out, type->info.ident.ref->name, "Identity: ");
        }
        break;
    case LY_TYPE_INST:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "instance-identifier");
        if (!uni) {
            ly_print(out, "%-*s%s\n", INDENT_LEN, "Required: ", (type->info.inst.req < 1 ? "no" : "yes"));
        }
        break;
    case LY_TYPE_INT8:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "int8");
        goto int_range;
    case LY_TYPE_INT16:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "int16");
        goto int_range;
    case LY_TYPE_INT32:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "int32");
        goto int_range;
    case LY_TYPE_INT64:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "int64");
        goto int_range;
    case LY_TYPE_UINT8:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint8");
        goto int_range;
    case LY_TYPE_UINT16:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint16");
        goto int_range;
    case LY_TYPE_UINT32:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint32");
        goto int_range;
    case LY_TYPE_UINT64:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint64");

int_range:
        if (!uni) {
            info_print_text(out, (type->info.num.range ? type->info.num.range->expr : NULL), "Range: ");
        }
        break;
    case LY_TYPE_LEAFREF:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "leafref");
        if (!uni) {
            info_print_text(out, type->info.lref.path, "Path: ");
        }
        break;
    case LY_TYPE_STRING:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "string");
        if (!uni) {
            info_print_text(out, (type->info.str.length ? type->info.str.length->expr : NULL), "Length: ");

            ly_print(out, "%-*s", INDENT_LEN, "Pattern: ");
            if (type->info.str.pat_count) {
                ly_print(out, "%s\n", type->info.str.patterns[0].expr);
                for (i = 1; i < type->info.str.pat_count; ++i) {
                    ly_print(out, "%*s%s\n", INDENT_LEN, "", type->info.str.patterns[i].expr);
                }
            } else {
                ly_print(out, "\n");
            }
        }

        break;
    case LY_TYPE_UNION:
        ly_print(out, "%-*s%s\n", INDENT_LEN, "Base type: ", "union");

        if (!uni) {
            for (i = 0; i < type->info.uni.count; ++i) {
                info_print_type_detail(out, &type->info.uni.types[i], 1);
            }
        }
        break;
    }

    if (uni) {
        ly_print(out, "  ");
    }
    ly_print(out, "%-*s", INDENT_LEN, "Superior: ");
    if (type->der) {
        if (type->module_name) {
            ly_print(out, "%s:", type->module_name);
        }
        ly_print(out, "%s\n", type->der->name);
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_list_constr(struct lyout *out, uint32_t min, uint32_t max)
{
    ly_print(out, "%-*s%u..", INDENT_LEN, "Elements: ", min);
    if (max) {
        ly_print(out, "%u\n", max);
    } else {
        ly_print(out, "unbounded\n");
    }
}

static void
info_print_keys(struct lyout *out, struct lys_node_leaf ** const keys, uint8_t keys_size)
{
    int i;

    ly_print(out, "%-*s", INDENT_LEN, "Keys: ");

    if (keys_size) {
        ly_print(out, "%s\n", keys[0]->name);
        for (i = 1; i < keys_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", keys[i]->name);
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_unique(struct lyout *out, const struct lys_unique *unique, uint8_t unique_size)
{
    int i, j;

    ly_print(out, "%-*s", INDENT_LEN, "Unique: ");

    if (unique_size) {
        ly_print(out, "%s\n", unique[0].expr[0]);
        for (i = 0; i < unique_size; ++i) {
            for (j = (!i ? 1 : 0); j < unique[i].expr_size; ++j) {
                ly_print(out, "%*s%s\n", INDENT_LEN, "", unique[i].expr[j]);
            }
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_nacmext(struct lyout *out, uint8_t nacm)
{
    ly_print(out, "%-*s", INDENT_LEN, "NACM: ");

    if (nacm) {
        if (nacm & LYS_NACM_DENYW) {
            ly_print(out, "default-deny-write\n");
        } else if (nacm & LYS_NACM_DENYA) {
            ly_print(out, "default-deny-all\n");
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_revision(struct lyout *out, const struct lys_revision *rev, uint8_t rev_size)
{
    int i;

    ly_print(out, "%-*s", INDENT_LEN, "Revisions: ");

    if (rev_size) {
        ly_print(out, "%s\n", rev[0].date);
        for (i = 1; i < rev_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", rev[i].date);
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_import_with_include(struct lyout *out, const struct lys_module *mod)
{
    int first = 1, i, j;

    ly_print(out, "%-*s", INDENT_LEN, "Imports: ");
    if (mod->imp_size) {
        ly_print(out, "%s:%s\n", mod->imp[0].prefix, mod->imp[0].module->name);
        i = 1;
        first = 0;

        for (; i < mod->imp_size; ++i) {
            ly_print(out, "%*s%s:%s\n", INDENT_LEN, "", mod->imp[i].prefix, mod->imp[i].module->name);
        }
    }

    for (j = 0; j < mod->inc_size; ++j) {
        if (mod->inc[j].submodule->imp_size) {
            if (first) {
                ly_print(out, "%s:%s\n",
                        mod->inc[j].submodule->imp[0].prefix, mod->inc[j].submodule->imp[0].module->name);
                i = 1;
            } else {
                i = 0;
            }
            first = 0;

            for (; i < mod->inc[j].submodule->imp_size; ++i) {
                ly_print(out, "%*s%s:%s\n", INDENT_LEN, "",
                        mod->inc[j].submodule->imp[i].prefix, mod->inc[j].submodule->imp[i].module->name);
            }
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_include(struct lyout *out, const struct lys_module *mod)
{
    int first = 1, i;

    ly_print(out, "%-*s", INDENT_LEN, "Includes: ");
    if (mod->inc_size) {
        ly_print(out, "%s\n", mod->inc[0].submodule->name);
        i = 1;
        first = 0;

        for (; i < mod->inc_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", mod->inc[i].submodule->name);
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_augment(struct lyout *out, const struct lys_module *mod)
{
    int first = 1, i;

    ly_print(out, "%-*s", INDENT_LEN, "Augments: ");
    if (mod->augment_size) {
        ly_print(out, "\"%s\"\n", mod->augment[0].target_name);
        i = 1;
        first = 0;

        for (; i < mod->augment_size; ++i) {
            ly_print(out, "%*s\"%s\"\n", INDENT_LEN, "", mod->augment[i].target_name);
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_deviation(struct lyout *out, const struct lys_module *mod)
{
    int first = 1, i;

    ly_print(out, "%-*s", INDENT_LEN, "Deviation: ");
    if (mod->deviation_size) {
        ly_print(out, "\"%s\"\n", mod->deviation[0].target_name);
        i = 1;
        first = 0;

        for (; i < mod->deviation_size; ++i) {
            ly_print(out, "%*s\"%s\"\n", INDENT_LEN, "", mod->deviation[i].target_name);
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_ident_with_include(struct lyout *out, const struct lys_module *mod)
{
    int first = 1, i, j;

    ly_print(out, "%-*s", INDENT_LEN, "Idents: ");
    if (mod->ident_size) {
        ly_print(out, "%s\n", mod->ident[0].name);
        i = 1;
        first = 0;

        for (; i < (signed)mod->ident_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", mod->ident[i].name);
        }
    }

    for (j = 0; j < mod->inc_size; ++j) {
        if (mod->inc[j].submodule->ident_size) {
            if (first) {
                ly_print(out, "%s\n", mod->inc[j].submodule->ident[0].name);
                i = 1;
            } else {
                i = 0;
            }
            first = 0;

            for (; i < (signed)mod->inc[j].submodule->ident_size; ++i) {
                ly_print(out, "%*s%s\n", INDENT_LEN, "", mod->inc[j].submodule->ident[i].name);
            }
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_features_with_include(struct lyout *out, const struct lys_module *mod)
{
    int first = 1, i, j;

    ly_print(out, "%-*s", INDENT_LEN, "Features: ");
    if (mod->features_size) {
        ly_print(out, "%s\n", mod->features[0].name);
        i = 1;
        first = 0;

        for (; i < mod->features_size; ++i) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", mod->features[i].name);
        }
    }

    for (j = 0; j < mod->inc_size; ++j) {
        if (mod->inc[j].submodule->features_size) {
            if (first) {
                ly_print(out, "%s\n", mod->inc[j].submodule->features[0].name);
                i = 1;
            } else {
                i = 0;
            }
            first = 0;

            for (; i < mod->inc[j].submodule->features_size; ++i) {
                ly_print(out, "%*s%s\n", INDENT_LEN, "", mod->inc[j].submodule->features[i].name);
            }
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_data_with_include(struct lyout *out, const struct lys_module *mod)

{
    int first = 1;
    struct lys_node *node;

    ly_print(out, "%-*s", INDENT_LEN, "Data: ");

    if (mod->data) {
        ly_print(out, "%s \"%s\"\n", strnodetype(mod->data->nodetype), mod->data->name);
        node = mod->data->next;
        first = 0;

        for (; node; node = node->next) {
            ly_print(out, "%*s%s \"%s\"\n", INDENT_LEN, "", strnodetype(node->nodetype), node->name);
        }
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
info_print_typedef_detail(struct lyout *outf, const struct lys_tpdf *tpdf)
{
    ly_print(outf, "%-*s%s\n", INDENT_LEN, "Typedef: ", tpdf->name);
    ly_print(outf, "%-*s%s\n", INDENT_LEN, "Module: ", tpdf->module->name);
    info_print_text(outf, tpdf->dsc, "Desc: ");
    info_print_text(outf, tpdf->ref, "Reference: ");
    info_print_flags(outf, tpdf->flags, LYS_STATUS_MASK, 0);
    info_print_type_detail(outf, &tpdf->type, 0);
    info_print_text(outf, tpdf->units, "Units: ");
    info_print_text(outf, tpdf->dflt, "Default: ");
}

static void
info_print_ident_detail(struct lyout *out, const struct lys_ident *ident)
{
    struct lys_ident_der *der;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Identity: ", ident->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", ident->module->name);
    info_print_text(out, ident->dsc, "Desc: ");
    info_print_text(out, ident->ref, "Reference: ");
    info_print_flags(out, ident->flags, LYS_STATUS_MASK, 0);
    info_print_text(out, (ident->base ? ident->base->name : NULL), "Base: ");

    ly_print(out, "%-*s", INDENT_LEN, "Derived: ");
    if (ident->der) {
        der = ident->der;
        ly_print(out, "%s\n", der->ident->name);
        for (der = der->next; der; der = der->next) {
            ly_print(out, "%*s%s\n", INDENT_LEN, "", der->ident->name);
        }
    } else {
        ly_print(out, "\n");
    }
}

static void
info_print_feature_detail(struct lyout *out, const struct lys_feature *feat)
{
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Feature: ", feat->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", feat->module->name);
    info_print_text(out, feat->dsc, "Desc: ");
    info_print_text(out, feat->ref, "Reference: ");
    info_print_flags(out, feat->flags, LYS_STATUS_MASK | LYS_FENABLED, 0);
    info_print_if_feature(out, feat->features, feat->features_size);
}

static void
info_print_module(struct lyout *out, const struct lys_module *module)
{
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", module->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Namespace: ", module->ns);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Prefix: ", module->prefix);
    info_print_text(out, module->dsc, "Desc: ");
    info_print_text(out, module->ref, "Reference: ");
    info_print_text(out, module->org, "Org: ");
    info_print_text(out, module->contact, "Contact: ");
    ly_print(out, "%-*s%s\n", INDENT_LEN, "YANG ver: ", (module->version == 2 ? "1.1" : "1.0"));
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Deviated: ", (module->deviated ? "yes" : "no"));
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Implement: ", (module->implemented ? "yes" : "no"));
    info_print_text(out, module->uri, "URI: ");

    info_print_revision(out, module->rev, module->rev_size);
    info_print_include(out, module);
    info_print_import_with_include(out, module);
    info_print_typedef_with_include(out, module);
    info_print_ident_with_include(out, module);
    info_print_features_with_include(out, module);
    info_print_augment(out, module);
    info_print_deviation(out, module);

    info_print_data_with_include(out, module);
}

static void
info_print_submodule(struct lyout *out, const struct lys_submodule *module)
{
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Submodule: ", module->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Parent: ", module->belongsto->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Prefix: ", module->prefix);
    info_print_text(out, module->dsc, "Desc: ");
    info_print_text(out, module->ref, "Reference: ");
    info_print_text(out, module->org, "Org: ");
    info_print_text(out, module->contact, "Contact: ");

    /* inherited from main module */
    ly_print(out, "%-*s%s\n", INDENT_LEN, "YANG ver: ", (module->belongsto->version == 2 ? "1.1" : "1.0"));
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Deviated: ", (module->belongsto->deviated ? "yes" : "no"));
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Implement: ", (module->belongsto->implemented ? "yes" : "no"));

    info_print_text(out, module->uri, "URI: ");

    info_print_revision(out, module->rev, module->rev_size);
    info_print_include(out, (struct lys_module *)module);
    info_print_import_with_include(out, (struct lys_module *)module);
    info_print_typedef_with_include(out, (struct lys_module *)module);
    info_print_ident_with_include(out, (struct lys_module *)module);
    info_print_features_with_include(out, (struct lys_module *)module);
    info_print_augment(out, (struct lys_module *)module);
    info_print_deviation(out, (struct lys_module *)module);

    info_print_data_with_include(out, (struct lys_module *)module);
}

static void
info_print_container(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_container *cont = (struct lys_node_container *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Container: ", cont->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", cont->module->name);
    info_print_text(out, cont->dsc, "Desc: ");
    info_print_text(out, cont->ref, "Reference: ");
    info_print_flags(out, cont->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_text(out, cont->presence, "Presence: ");
    info_print_if_feature(out, cont->features, cont->features_size);
    info_print_when(out, cont->when);
    info_print_must(out, cont->must, cont->must_size);
    info_print_typedef(out, cont->tpdf, cont->tpdf_size);
    info_print_nacmext(out, cont->nacm);

    info_print_snode(out, (struct lys_node *)cont, cont->child, "Children:");
}

static void
info_print_choice(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Choice: ", choice->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", choice->module->name);
    info_print_text(out, choice->dsc, "Desc: ");
    info_print_text(out, choice->ref, "Reference: ");
    info_print_flags(out, choice->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    ly_print(out, "%-*s", INDENT_LEN, "Default: ");
    if (choice->dflt) {
        ly_print(out, "%s\n", choice->dflt->name);
    } else {
        ly_print(out, "\n");
    }
    info_print_if_feature(out, choice->features, choice->features_size);
    info_print_when(out, choice->when);
    info_print_nacmext(out, choice->nacm);

    info_print_snode(out, (struct lys_node *)choice, choice->child, "Cases:");
}

static void
info_print_leaf(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Leaf: ", leaf->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", leaf->module->name);
    info_print_text(out, leaf->dsc, "Desc: ");
    info_print_text(out, leaf->ref, "Reference: ");
    info_print_flags(out, leaf->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_text(out, leaf->type.der->name, "Type: ");
    info_print_text(out, leaf->units, "Units: ");
    info_print_text(out, leaf->dflt, "Default: ");
    info_print_if_feature(out, leaf->features, leaf->features_size);
    info_print_when(out, leaf->when);
    info_print_must(out, leaf->must, leaf->must_size);
    info_print_nacmext(out, leaf->nacm);
}

static void
info_print_leaflist(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Leaflist: ", llist->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", llist->module->name);
    info_print_text(out, llist->dsc, "Desc: ");
    info_print_text(out, llist->ref, "Reference: ");
    info_print_flags(out, llist->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK | LYS_USERORDERED, 1);
    info_print_text(out, llist->type.der->name, "Type: ");
    info_print_text(out, llist->units, "Units: ");
    info_print_list_constr(out, llist->min, llist->max);
    info_print_if_feature(out, llist->features, llist->features_size);
    info_print_when(out, llist->when);
    info_print_must(out, llist->must, llist->must_size);
    info_print_nacmext(out, llist->nacm);
}

static void
info_print_list(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_list *list = (struct lys_node_list *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "List: ", list->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", list->module->name);
    info_print_text(out, list->dsc, "Desc: ");
    info_print_text(out, list->ref, "Reference: ");
    info_print_flags(out, list->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK | LYS_USERORDERED, 1);
    info_print_list_constr(out, list->min, list->max);
    info_print_if_feature(out, list->features, list->features_size);
    info_print_when(out, list->when);
    info_print_must(out, list->must, list->must_size);
    info_print_keys(out, list->keys, list->keys_size);
    info_print_unique(out, list->unique, list->unique_size);
    info_print_typedef(out, list->tpdf, list->tpdf_size);
    info_print_nacmext(out, list->nacm);

    info_print_snode(out, (struct lys_node *)list, list->child, "Children:");
}

static void
info_print_anyxml(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_anyxml *axml = (struct lys_node_anyxml *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Anyxml: ", axml->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", axml->module->name);
    info_print_text(out, axml->dsc, "Desc: ");
    info_print_text(out, axml->ref, "Reference: ");
    info_print_flags(out, axml->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_if_feature(out, axml->features, axml->features_size);
    info_print_when(out, axml->when);
    info_print_must(out, axml->must, axml->must_size);
    info_print_nacmext(out, axml->nacm);
}

static void
info_print_grouping(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_grp *group = (struct lys_node_grp *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Grouping: ", group->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", group->module->name);
    info_print_text(out, group->dsc, "Desc: ");
    info_print_text(out, group->ref, "Reference: ");
    info_print_flags(out, group->flags, LYS_STATUS_MASK, 0);
    info_print_typedef(out, group->tpdf, group->tpdf_size);
    info_print_nacmext(out, group->nacm);

    info_print_snode(out, (struct lys_node *)group, group->child, "Children:");
}

static void
info_print_case(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_case *cas = (struct lys_node_case *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Case: ", cas->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", cas->module->name);
    info_print_text(out, cas->dsc, "Desc: ");
    info_print_text(out, cas->ref, "Reference: ");
    info_print_flags(out, cas->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_if_feature(out, cas->features, cas->features_size);
    info_print_when(out, cas->when);
    info_print_nacmext(out, cas->nacm);

    info_print_snode(out, (struct lys_node *)cas, cas->child, "Children:");
}

static void
info_print_input(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_rpc_inout *input = (struct lys_node_rpc_inout *)node;

    assert(input->parent && input->parent->nodetype == LYS_RPC);

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Input of: ", input->parent->name);
    info_print_typedef(out, input->tpdf, input->tpdf_size);

    info_print_snode(out, (struct lys_node *)input, input->child, "Children:");
}

static void
info_print_output(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_rpc_inout *output = (struct lys_node_rpc_inout *)node;

    assert(output->parent && output->parent->nodetype == LYS_RPC);

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Output of: ", output->parent->name);
    info_print_typedef(out, output->tpdf, output->tpdf_size);

    info_print_snode(out, (struct lys_node *)output, output->child, "Children:");
}

static void
info_print_notif(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_notif *ntf = (struct lys_node_notif *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "Notif: ", ntf->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", ntf->module->name);
    info_print_text(out, ntf->dsc, "Desc: ");
    info_print_text(out, ntf->ref, "Reference: ");
    info_print_flags(out, ntf->flags, LYS_STATUS_MASK, 0);
    info_print_if_feature(out, ntf->features, ntf->features_size);
    info_print_typedef(out, ntf->tpdf, ntf->tpdf_size);
    info_print_nacmext(out, ntf->nacm);

    info_print_snode(out, (struct lys_node *)ntf, ntf->child, "Params:");
}

static void
info_print_rpc(struct lyout *out, const struct lys_node *node)
{
    struct lys_node_rpc *rpc = (struct lys_node_rpc *)node;

    ly_print(out, "%-*s%s\n", INDENT_LEN, "RPC: ", rpc->name);
    ly_print(out, "%-*s%s\n", INDENT_LEN, "Module: ", rpc->module->name);
    info_print_text(out, rpc->dsc, "Desc: ");
    info_print_text(out, rpc->ref, "Reference: ");
    info_print_flags(out, rpc->flags, LYS_STATUS_MASK, 0);
    info_print_if_feature(out, rpc->features, rpc->features_size);
    info_print_typedef(out, rpc->tpdf, rpc->tpdf_size);
    info_print_nacmext(out, rpc->nacm);

    info_print_snode(out, (struct lys_node *)rpc, rpc->child, "Data:");
}

int
info_print_model(struct lyout *out, const struct lys_module *module, const char *target_node)
{
    int i, rc;
    char *grouping_target = NULL;
    struct lys_node *target = NULL;

    if (!target_node) {
        if (module->type == 0) {
            info_print_module(out, module);
        } else {
            info_print_submodule(out, (struct lys_submodule *)module);
        }
    } else {
        if ((target_node[0] == '/') || !strncmp(target_node, "type/", 5)) {
            rc = resolve_augment_schema_nodeid((target_node[0] == '/' ? target_node : target_node + 4), NULL, module,
                                               (const struct lys_node **)&target);
            if (rc || !target) {
                ly_print(out, "Target %s could not be resolved.\n", (target_node[0] == '/' ? target_node : target_node + 4));
                return EXIT_FAILURE;
            }
        } else if (!strncmp(target_node, "grouping/", 9)) {
            /* cut the data part off */
            if (strchr(target_node + 9, '/')) {
                /* HACK only temporary */
                *strchr(target_node + 9, '/') = '\0';
                grouping_target = (char *)(target_node + strlen(target_node) + 1);
            }
            rc = resolve_absolute_schema_nodeid(target_node + 9, module, LYS_GROUPING, (const struct lys_node **)&target);
            if (rc || !target) {
                ly_print(out, "Grouping %s not found.\n", target_node+9);
                return EXIT_FAILURE;
            }
        } else if (!strncmp(target_node, "typedef/", 8)) {
            target_node += 8;
            for (i = 0; i < module->tpdf_size; ++i) {
                if (!strcmp(module->tpdf[i].name, target_node)) {
                    break;
                }
            }
            if (i == module->tpdf_size) {
                ly_print(out, "Typedef %s not found.\n", target_node);
                return EXIT_FAILURE;
            }

            info_print_typedef_detail(out, &module->tpdf[i]);
            return EXIT_SUCCESS;

        } else if (!strncmp(target_node, "identity/", 9)) {
            target_node += 9;
            for (i = 0; i < (signed)module->ident_size; ++i) {
                if (!strcmp(module->ident[i].name, target_node)) {
                    break;
                }
            }
            if (i == (signed)module->ident_size) {
                ly_print(out, "Identity %s not found.\n", target_node);
                return EXIT_FAILURE;
            }

            info_print_ident_detail(out, &module->ident[i]);
            return EXIT_SUCCESS;

        } else if (!strncmp(target_node, "feature/", 8)) {
            target_node += 8;
            for (i = 0; i < module->features_size; ++i) {
                if (!strcmp(module->features[i].name, target_node)) {
                    break;
                }
            }
            if (i == module->features_size) {
                ly_print(out, "Feature %s not found.\n", target_node);
                return EXIT_FAILURE;
            }

            info_print_feature_detail(out, &module->features[i]);
            return EXIT_SUCCESS;
        } else {
            ly_print(out, "Target could not be resolved.\n");
            return EXIT_FAILURE;
        }

        if (!strncmp(target_node, "type/", 5)) {
            if (!(target->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                ly_print(out, "Target is not a leaf or a leaf-list.\n");
                return EXIT_FAILURE;
            }
            info_print_type_detail(out, &((struct lys_node_leaf *)target)->type, 0);
            return EXIT_SUCCESS;
        } else if (!strncmp(target_node, "grouping/", 9) && !grouping_target) {
            info_print_grouping(out, target);
            return EXIT_SUCCESS;
        }

        /* find the node in the grouping */
        if (grouping_target) {
            rc = resolve_descendant_schema_nodeid(grouping_target, target->child, LYS_NO_RPC_NOTIF_NODE,
                                                  (const struct lys_node **)&target);
            if (rc || !target) {
                ly_print(out, "Grouping %s child \"%s\" not found.\n", target_node + 9, grouping_target);
                return EXIT_FAILURE;
            }
            /* HACK return previous hack */
            --grouping_target;
            grouping_target[0] = '/';
        }

        switch (target->nodetype) {
        case LYS_CONTAINER:
            info_print_container(out, target);
            break;
        case LYS_CHOICE:
            info_print_choice(out, target);
            break;
        case LYS_LEAF:
            info_print_leaf(out, target);
            break;
        case LYS_LEAFLIST:
            info_print_leaflist(out, target);
            break;
        case LYS_LIST:
            info_print_list(out, target);
            break;
        case LYS_ANYXML:
            info_print_anyxml(out, target);
            break;
        case LYS_CASE:
            info_print_case(out, target);
            break;
        case LYS_NOTIF:
            info_print_notif(out, target);
            break;
        case LYS_RPC:
            info_print_rpc(out, target);
            break;
        case LYS_INPUT:
            info_print_input(out, target);
            break;
        case LYS_OUTPUT:
            info_print_output(out, target);
            break;
        default:
            ly_print(out, "Nodetype %s not supported.\n", strnodetype(target->nodetype));
            break;
        }
    }

    return EXIT_SUCCESS;
}
