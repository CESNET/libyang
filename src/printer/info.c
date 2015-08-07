/**
 * @file printer/info.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief INFO printer for libyang data model structure
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
#include <assert.h>

#include "../common.h"
#include "../tree.h"
#include "../resolve.h"

#define INDENT_LEN 11

static void
info_print_text(FILE *f, const char *text, const char *label)
{
    const char *ptr1, *ptr2;
    int first = 1;

    fprintf(f, "%-*s", INDENT_LEN, label);

    if (text) {
        ptr1 = text;
        while (1) {
            ptr2 = strchr(ptr1, '\n');
            if (!ptr2) {
                if (first) {
                    fprintf(f, "%s\n", ptr1);
                    first = 0;
                } else {
                    fprintf(f, "%*s%s\n", INDENT_LEN, "", ptr1);
                }
                break;
            }
            ++ptr2;
            if (first) {
                fprintf(f, "%.*s", (int)(ptr2-ptr1), ptr1);
                first = 0;
            } else {
                fprintf(f, "%*s%.*s", INDENT_LEN, "", (int)(ptr2-ptr1), ptr1);
            }
            ptr1 = ptr2;
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_snode(FILE *f, struct lys_node *parent, struct lys_node *node, const char *label)
{
    assert(strlen(label) < INDENT_LEN-1);

    fprintf(f, "%-*s", INDENT_LEN, label);

    if (node) {
        if (node->name) {
            fprintf(f, "%s \"", strnodetype(node->nodetype));
            if (parent != node->parent) {
                fprintf(f, "%s:", node->module->prefix);
            }
            fprintf(f, "%s\"\n", node->name);
        } else {
            fprintf(f, "%s\n", (node->nodetype == LYS_INPUT ? "input" : "output"));
        }
        node = node->next;
        for (; node; node = node->next) {
            if (node->name) {
                fprintf(f, "%*s%s \"", INDENT_LEN, "", strnodetype(node->nodetype));
                if (parent != node->parent) {
                    fprintf(f, "%s:", node->module->prefix);
                }
                fprintf(f, "%s\"\n", node->name);
            } else {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", (node->nodetype == LYS_INPUT ? "input" : "output"));
            }
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_flags(FILE *f, uint8_t flags, uint8_t mask, int is_list)
{
    if (mask & LYS_CONFIG_MASK) {
        fprintf(f, "%-*s", INDENT_LEN, "Config: ");
        if (flags & LYS_CONFIG_R) {
            fprintf(f, "read-only\n");
        } else {
            fprintf(f, "read-write\n");
        }
    }

    if (mask & LYS_STATUS_MASK) {
        fprintf(f, "%-*s", INDENT_LEN, "Status: ");

        if (flags & LYS_STATUS_DEPRC) {
            fprintf(f, "deprecated\n");
        } else if (flags & LYS_STATUS_OBSLT) {
            fprintf(f, "obsolete\n");
        } else {
            fprintf(f, "current\n");
        }
    }

    if (mask & LYS_MAND_MASK) {
        fprintf(f, "%-*s", INDENT_LEN, "Mandatory: ");

        if (flags & LYS_MAND_TRUE) {
            fprintf(f, "yes\n");
        } else {
            fprintf(f, "no\n");
        }
    }

    if (is_list && (mask & LYS_USERORDERED)) {
        fprintf(f, "%-*s", INDENT_LEN, "Order: ");

        if (flags & LYS_USERORDERED) {
            fprintf(f, "user-ordered\n");
        } else {
            fprintf(f, "system-ordered\n");
        }
    }

    if (!is_list && (mask & LYS_FENABLED)) {
        fprintf(f, "%-*s", INDENT_LEN, "Enabled: ");

        if (flags & LYS_FENABLED) {
            fprintf(f, "yes\n");
        } else {
            fprintf(f, "no\n");
        }
    }
}

static void
info_print_if_feature(FILE *f, struct lys_feature **features, uint8_t features_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "If-feats: ");

    if (features_size) {
        fprintf(f, "%s\n", features[0]->name);
        for (i = 1; i < features_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", features[i]->name);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_when(FILE *f, struct lys_when *when)
{
    fprintf(f, "%-*s", INDENT_LEN, "When: ");
    if (when) {
        fprintf(f, "%s\n", when->cond);
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_must(FILE *f, struct lys_restr *must, uint8_t must_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "Must: ");

    if (must_size) {
        fprintf(f, "%s\n", must[0].expr);
        for (i = 1; i < must_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", must[i].expr);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_typedef(FILE *f, struct lys_tpdf *tpdf, uint8_t tpdf_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "Typedefs: ");

    if (tpdf_size) {
        fprintf(f, "%s\n", tpdf[0].name);
        for (i = 1; i < tpdf_size; ++i) {
            fprintf(f, "%*s%s", INDENT_LEN, "", tpdf[i].name);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_typedef_with_include(FILE *f, struct lys_module *mod)
{
    int i, j, first = 1;

    fprintf(f, "%-*s", INDENT_LEN, "Typedefs: ");

    if (mod->tpdf_size) {
        if (first) {
            fprintf(f, "%s\n", mod->tpdf[0].name);
            i = 1;
        } else {
            i = 0;
        }
        first = 0;

        for (; i < mod->tpdf_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->tpdf[i].name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->tpdf_size) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[i].submodule->tpdf[0].name);
                j = 1;
            } else {
                j = 0;
            }
            first = 0;

            for (; j < mod->inc[i].submodule->tpdf_size; ++j) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->inc[i].submodule->tpdf[j].name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_type_detail(FILE *f, struct lys_type *type, int uni)
{
    int i;

    if (uni) {
        fprintf(f, "  ");
    }

    switch (type->base) {
    case LY_TYPE_DER:
        /* unused, but what the heck */
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "derived");
        break;
    case LY_TYPE_BINARY:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "binary");
        if (!uni) {
            info_print_text(f, (type->info.binary.length ? type->info.binary.length->expr : NULL), "Length: ");
        }
        break;
    case LY_TYPE_BITS:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "bits");

        assert(type->info.bits.count);
        if (!uni) {
            fprintf(f, "%-*s%u %s\n", INDENT_LEN, "Bits: ", type->info.bits.bit[0].pos, type->info.bits.bit[0].name);
            for (i = 1; i < type->info.bits.count; ++i) {
                fprintf(f, "%*s%u %s\n", INDENT_LEN, "", type->info.bits.bit[i].pos, type->info.bits.bit[i].name);
            }
        }

        break;
    case LY_TYPE_BOOL:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "bool");
        break;
    case LY_TYPE_DEC64:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "decimal64");
        if (!uni) {
            info_print_text(f, (type->info.dec64.range ? type->info.dec64.range->expr : NULL), "Range: ");
            assert(type->info.dec64.dig);
            fprintf(f, "%-*s%u\n", INDENT_LEN, "Frac dig: ", type->info.dec64.dig);
        }
        break;
    case LY_TYPE_EMPTY:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "empty");
        break;
    case LY_TYPE_ENUM:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "enum");

        assert(type->info.enums.count);
        if (!uni) {
            fprintf(f, "%-*s%s\n", INDENT_LEN, "Values: ", type->info.enums.enm[0].name);
            for (i = 1; i < type->info.enums.count; ++i) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", type->info.enums.enm[i].name);
            }
        }

        break;
    case LY_TYPE_IDENT:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "identityref");
        assert(type->info.ident.ref);
        if (!uni) {
            info_print_text(f, type->info.ident.ref->name, "Identity: ");
        }
        break;
    case LY_TYPE_INST:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "instance-identifier");
        if (!uni) {
            fprintf(f, "%-*s%s\n", INDENT_LEN, "Required: ", (type->info.inst.req < 1 ? "no" : "yes"));
        }
        break;
    case LY_TYPE_INT8:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "int8");
        goto int_range;
    case LY_TYPE_INT16:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "int16");
        goto int_range;
    case LY_TYPE_INT32:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "int32");
        goto int_range;
    case LY_TYPE_INT64:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "int64");
        goto int_range;
    case LY_TYPE_UINT8:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint8");
        goto int_range;
    case LY_TYPE_UINT16:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint16");
        goto int_range;
    case LY_TYPE_UINT32:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint32");
        goto int_range;
    case LY_TYPE_UINT64:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "uint64");

int_range:
        if (!uni) {
            info_print_text(f, (type->info.num.range ? type->info.num.range->expr : NULL), "Range: ");
        }
        break;
    case LY_TYPE_LEAFREF:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "leafref");
        if (!uni) {
            info_print_text(f, type->info.lref.path, "Path: ");
        }
        break;
    case LY_TYPE_STRING:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "string");
        if (!uni) {
            info_print_text(f, (type->info.str.length ? type->info.str.length->expr : NULL), "Length: ");

            fprintf(f, "%-*s", INDENT_LEN, "Pattern: ");
            if (type->info.str.pat_count) {
                fprintf(f, "%s\n", type->info.str.patterns[0].expr);
                for (i = 1; i < type->info.str.pat_count; ++i) {
                    fprintf(f, "%*s%s\n", INDENT_LEN, "", type->info.str.patterns[i].expr);
                }
            } else {
                fprintf(f, "\n");
            }
        }

        break;
    case LY_TYPE_UNION:
        fprintf(f, "%-*s%s\n", INDENT_LEN, "Base type: ", "union");

        if (!uni) {
            for (i = 0; i < type->info.uni.count; ++i) {
                info_print_type_detail(f, &type->info.uni.types[i], 1);
            }
        }
        break;
    }

    if (uni) {
        fprintf(f, "  ");
    }
    fprintf(f, "%-*s", INDENT_LEN, "Superior: ");
    if (type->der) {
        if (type->prefix) {
            fprintf(f, "%s:", type->prefix);
        }
        fprintf(f, "%s\n", type->der->name);
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_list_constr(FILE *f, uint32_t min, uint32_t max)
{
    fprintf(f, "%-*s%u..", INDENT_LEN, "Elements: ", min);
    if (max) {
        fprintf(f, "%u\n", max);
    } else {
        fprintf(f, "unbounded\n");
    }
}

static void
info_print_keys(FILE *f, struct lys_node_leaf **keys, uint8_t keys_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "Keys: ");

    if (keys_size) {
        fprintf(f, "%s\n", keys[0]->name);
        for (i = 1; i < keys_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", keys[i]->name);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_unique(FILE *f, struct lys_unique *unique, uint8_t unique_size)
{
    int i, j;

    fprintf(f, "%-*s", INDENT_LEN, "Unique: ");

    if (unique_size) {
        fprintf(f, "%s\n", unique[0].leafs[0]->name);
        for (i = 0; i < unique_size; ++i) {
            for (j = (!i ? 1 : 0); j < unique[i].leafs_size; ++j) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", unique[i].leafs[j]->name);
            }
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_nacmext(FILE *f, uint8_t nacm)
{
    fprintf(f, "%-*s", INDENT_LEN, "NACM: ");

    if (nacm) {
        if (nacm & LYS_NACM_DENYW) {
            fprintf(f, "default-deny-write\n");
        } else if (nacm & LYS_NACM_DENYA) {
            fprintf(f, "default-deny-all\n");
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_revision(FILE *f, struct lys_revision *rev, uint8_t rev_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "Revisions: ");

    if (rev_size) {
        fprintf(f, "%s\n", rev[0].date);
        for (i = 1; i < rev_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", rev[i].date);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_import_with_include(FILE *f, struct lys_module *mod)
{
    int first = 1, i, j;

    fprintf(f, "%-*s", INDENT_LEN, "Imports: ");
    if (mod->imp_size) {
        fprintf(f, "%s:%s\n", mod->imp[0].prefix, mod->imp[0].module->name);
        i = 1;
        first = 0;

        for (; i < mod->imp_size; ++i) {
            fprintf(f, "%*s%s:%s\n", INDENT_LEN, "", mod->imp[i].prefix, mod->imp[i].module->name);
        }
    }

    for (j = 0; j < mod->inc_size; ++j) {
        if (mod->inc[j].submodule->imp_size) {
            if (first) {
                fprintf(f, "%s:%s\n",
                        mod->inc[j].submodule->imp[0].prefix, mod->inc[j].submodule->imp[0].module->name);
                i = 1;
            } else {
                i = 0;
            }
            first = 0;

            for (; i < mod->inc[j].submodule->imp_size; ++i) {
                fprintf(f, "%*s%s:%s\n", INDENT_LEN, "",
                        mod->inc[j].submodule->imp[i].prefix, mod->inc[j].submodule->imp[i].module->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_include(FILE *f, struct lys_module *mod)
{
    int first = 1, i;

    fprintf(f, "%-*s", INDENT_LEN, "Includes: ");
    if (mod->inc_size) {
        fprintf(f, "%s\n", mod->inc[0].submodule->name);
        i = 1;
        first = 0;

        for (; i < mod->inc_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->inc[i].submodule->name);
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_augment(FILE *f, struct lys_module *mod)
{
    int first = 1, i;

    fprintf(f, "%-*s", INDENT_LEN, "Augments: ");
    if (mod->augment_size) {
        fprintf(f, "\"%s\"\n", mod->augment[0].target_name);
        i = 1;
        first = 0;

        for (; i < mod->augment_size; ++i) {
            fprintf(f, "%*s\"%s\"\n", INDENT_LEN, "", mod->augment[i].target_name);
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_deviation(FILE *f, struct lys_module *mod)
{
    int first = 1, i;

    fprintf(f, "%-*s", INDENT_LEN, "Deviation: ");
    if (mod->deviation_size) {
        fprintf(f, "\"%s\"\n", mod->deviation[0].target_name);
        i = 1;
        first = 0;

        for (; i < mod->deviation_size; ++i) {
            fprintf(f, "%*s\"%s\"\n", INDENT_LEN, "", mod->deviation[i].target_name);
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_ident_with_include(FILE *f, struct lys_module *mod)
{
    int first = 1, i, j;

    fprintf(f, "%-*s", INDENT_LEN, "Idents: ");
    if (mod->ident_size) {
        fprintf(f, "%s\n", mod->ident[0].name);
        i = 1;
        first = 0;

        for (; i < (signed)mod->ident_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->ident[i].name);
        }
    }

    for (j = 0; j < mod->inc_size; ++j) {
        if (mod->inc[j].submodule->ident_size) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[j].submodule->ident[0].name);
                i = 1;
            } else {
                i = 0;
            }
            first = 0;

            for (; i < (signed)mod->inc[j].submodule->ident_size; ++i) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->inc[j].submodule->ident[i].name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_features_with_include(FILE *f, struct lys_module *mod)
{
    int first = 1, i, j;

    fprintf(f, "%-*s", INDENT_LEN, "Features: ");
    if (mod->features_size) {
        fprintf(f, "%s\n", mod->features[0].name);
        i = 1;
        first = 0;

        for (; i < mod->features_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->features[i].name);
        }
    }

    for (j = 0; j < mod->inc_size; ++j) {
        if (mod->inc[j].submodule->features_size) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[j].submodule->features[0].name);
                i = 1;
            } else {
                i = 0;
            }
            first = 0;

            for (; i < mod->inc[j].submodule->features_size; ++i) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->inc[j].submodule->features[i].name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_rpc_with_include(FILE *f, struct lys_module *mod)
{
    int first = 1, i;
    struct lys_node *node;

    fprintf(f, "%-*s", INDENT_LEN, "RPCs: ");

    if (mod->rpc) {
        fprintf(f, "%s\n", mod->rpc->name);
        node = mod->rpc->next;
        first = 0;

        for (; node; node = node->next) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", node->name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->rpc) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[i].submodule->rpc->name);
                node = mod->inc[i].submodule->rpc->next;
            } else {
                node = mod->inc[i].submodule->rpc;
            }
            first = 0;

            for (; node; node = node->next) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", node->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_notif_with_include(FILE *f, struct lys_module *mod)
{
    int first = 1, i;
    struct lys_node *node;

    fprintf(f, "%-*s", INDENT_LEN, "Notifs: ");

    if (mod->notif) {
        fprintf(f, "%s\n", mod->notif->name);
        node = mod->notif->next;
        first = 0;

        for (; node; node = node->next) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", node->name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->notif) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[i].submodule->notif->name);
                node = mod->inc[i].submodule->notif->next;
            } else {
                node = mod->inc[i].submodule->notif;
            }
            first = 0;

            for (; node; node = node->next) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", node->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_snode_with_include(FILE *f, struct lys_module *mod)
{
    int first = 1, i;
    struct lys_node *node;

    fprintf(f, "%-*s", INDENT_LEN, "Data: ");

    if (mod->data) {
        fprintf(f, "%s \"%s\"\n", strnodetype(mod->data->nodetype), mod->data->name);
        node = mod->data->next;
        first = 0;

        for (; node; node = node->next) {
            fprintf(f, "%*s%s \"%s\"\n", INDENT_LEN, "", strnodetype(node->nodetype), node->name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->data) {
            if (first) {
                fprintf(f, "%s \"%s\"\n", strnodetype(mod->inc[i].submodule->data->nodetype), mod->inc[i].submodule->data->name);
                node = mod->inc[i].submodule->data->next;
            } else {
                node = mod->inc[i].submodule->data;
            }
            first = 0;

            for (; node; node = node->next) {
                fprintf(f, "%*s%s \"%s\"\n", INDENT_LEN, "", strnodetype(node->nodetype), node->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_typedef_detail(FILE *f, struct lys_tpdf *tpdf)
{
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Typedef: ", tpdf->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", tpdf->module->name);
    info_print_text(f, tpdf->dsc, "Desc: ");
    info_print_text(f, tpdf->ref, "Reference: ");
    info_print_flags(f, tpdf->flags, LYS_STATUS_MASK, 0);
    info_print_type_detail(f, &tpdf->type, 0);
    info_print_text(f, tpdf->units, "Units: ");
    info_print_text(f, tpdf->dflt, "Default: ");
}

static void
info_print_ident_detail(FILE *f, struct lys_ident *ident)
{
    struct lys_ident_der *der;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Identity: ", ident->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", ident->module->name);
    info_print_text(f, ident->dsc, "Desc: ");
    info_print_text(f, ident->ref, "Reference: ");
    info_print_flags(f, ident->flags, LYS_STATUS_MASK, 0);
    info_print_text(f, (ident->base ? ident->base->name : NULL), "Base: ");

    fprintf(f, "%-*s", INDENT_LEN, "Derived: ");
    if (ident->der) {
        der = ident->der;
        fprintf(f, "%s\n", der->ident->name);
        for (der = der->next; der; der = der->next) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", der->ident->name);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_feature_detail(FILE *f, struct lys_feature *feat)
{
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Feature: ", feat->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", feat->module->name);
    info_print_text(f, feat->dsc, "Desc: ");
    info_print_text(f, feat->ref, "Reference: ");
    info_print_flags(f, feat->flags, LYS_STATUS_MASK | LYS_FENABLED, 0);
    info_print_if_feature(f, feat->features, feat->features_size);
}

static void
info_print_module(FILE *f, struct lys_module *module)
{
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", module->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Namespace: ", module->ns);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Prefix: ", module->prefix);
    info_print_text(f, module->dsc, "Desc: ");
    info_print_text(f, module->ref, "Reference: ");
    info_print_text(f, module->org, "Org: ");
    info_print_text(f, module->contact, "Contact: ");
    fprintf(f, "%-*s%s\n", INDENT_LEN, "YANG ver: ", (module->version == 2 ? "1.1" : "1.0"));
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Deviated: ", (module->deviated ? "yes" : "no"));
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Implement: ", (module->implemented ? "yes" : "no"));
    info_print_text(f, module->uri, "URI: ");

    info_print_revision(f, module->rev, module->rev_size);
    info_print_include(f, module);
    info_print_import_with_include(f, module);
    info_print_typedef_with_include(f, module);
    info_print_ident_with_include(f, module);
    info_print_features_with_include(f, module);
    info_print_augment(f, module);
    info_print_deviation(f, module);

    info_print_rpc_with_include(f, module);
    info_print_notif_with_include(f, module);
    info_print_snode_with_include(f, module);
}

static void
info_print_submodule(FILE *f, struct lys_submodule *module)
{
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Submodule: ", module->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Parent: ", module->belongsto->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Prefix: ", module->prefix);
    info_print_text(f, module->dsc, "Desc: ");
    info_print_text(f, module->ref, "Reference: ");
    info_print_text(f, module->org, "Org: ");
    info_print_text(f, module->contact, "Contact: ");
    fprintf(f, "%-*s%s\n", INDENT_LEN, "YANG ver: ", (module->version == 2 ? "1.1" : "1.0"));
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Deviated: ", (module->deviated ? "yes" : "no"));
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Implement: ", (module->implemented ? "yes" : "no"));
    info_print_text(f, module->uri, "URI: ");

    info_print_revision(f, module->rev, module->rev_size);
    info_print_include(f, (struct lys_module *)module);
    info_print_import_with_include(f, (struct lys_module *)module);
    info_print_typedef_with_include(f, (struct lys_module *)module);
    info_print_ident_with_include(f, (struct lys_module *)module);
    info_print_features_with_include(f, (struct lys_module *)module);
    info_print_augment(f, (struct lys_module *)module);
    info_print_deviation(f, (struct lys_module *)module);

    info_print_rpc_with_include(f, (struct lys_module *)module);
    info_print_notif_with_include(f, (struct lys_module *)module);
    info_print_snode_with_include(f, (struct lys_module *)module);
}

static void
info_print_container(FILE *f, struct lys_node *node)
{
    struct lys_node_container *cont = (struct lys_node_container *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Container: ", cont->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", cont->module->name);
    info_print_text(f, cont->dsc, "Desc: ");
    info_print_text(f, cont->ref, "Reference: ");
    info_print_flags(f, cont->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_text(f, cont->presence, "Presence: ");
    info_print_if_feature(f, cont->features, cont->features_size);
    info_print_when(f, cont->when);
    info_print_must(f, cont->must, cont->must_size);
    info_print_typedef(f, cont->tpdf, cont->tpdf_size);
    info_print_nacmext(f, cont->nacm);

    info_print_snode(f, (struct lys_node *)cont, cont->child, "Children:");
}

static void
info_print_choice(FILE *f, struct lys_node *node)
{
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Choice: ", choice->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", choice->module->name);
    info_print_text(f, choice->dsc, "Desc: ");
    info_print_text(f, choice->ref, "Reference: ");
    info_print_flags(f, choice->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    fprintf(f, "%-*s", INDENT_LEN, "Default: ");
    if (choice->dflt) {
        fprintf(f, "%s\n", choice->dflt->name);
    } else {
        fprintf(f, "\n");
    }
    info_print_if_feature(f, choice->features, choice->features_size);
    info_print_when(f, choice->when);
    info_print_nacmext(f, choice->nacm);

    info_print_snode(f, (struct lys_node *)choice, choice->child, "Cases:");
}

static void
info_print_leaf(FILE *f, struct lys_node *node)
{
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Leaf: ", leaf->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", leaf->module->name);
    info_print_text(f, leaf->dsc, "Desc: ");
    info_print_text(f, leaf->ref, "Reference: ");
    info_print_flags(f, leaf->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_text(f, leaf->type.der->name, "Type: ");
    info_print_text(f, leaf->units, "Units: ");
    info_print_text(f, leaf->dflt, "Default: ");
    info_print_if_feature(f, leaf->features, leaf->features_size);
    info_print_when(f, leaf->when);
    info_print_must(f, leaf->must, leaf->must_size);
    info_print_nacmext(f, leaf->nacm);
}

static void
info_print_leaflist(FILE *f, struct lys_node *node)
{
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Leaflist: ", llist->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", llist->module->name);
    info_print_text(f, llist->dsc, "Desc: ");
    info_print_text(f, llist->ref, "Reference: ");
    info_print_flags(f, llist->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK | LYS_USERORDERED, 1);
    info_print_text(f, llist->type.der->name, "Type: ");
    info_print_text(f, llist->units, "Units: ");
    info_print_list_constr(f, llist->min, llist->max);
    info_print_if_feature(f, llist->features, llist->features_size);
    info_print_when(f, llist->when);
    info_print_must(f, llist->must, llist->must_size);
    info_print_nacmext(f, llist->nacm);
}

static void
info_print_list(FILE *f, struct lys_node *node)
{
    struct lys_node_list *list = (struct lys_node_list *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "List: ", list->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", list->module->name);
    info_print_text(f, list->dsc, "Desc: ");
    info_print_text(f, list->ref, "Reference: ");
    info_print_flags(f, list->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK | LYS_USERORDERED, 1);
    info_print_list_constr(f, list->min, list->max);
    info_print_if_feature(f, list->features, list->features_size);
    info_print_when(f, list->when);
    info_print_must(f, list->must, list->must_size);
    info_print_keys(f, list->keys, list->keys_size);
    info_print_unique(f, list->unique, list->unique_size);
    info_print_typedef(f, list->tpdf, list->tpdf_size);
    info_print_nacmext(f, list->nacm);

    info_print_snode(f, (struct lys_node *)list, list->child, "Children:");
}

static void
info_print_anyxml(FILE *f, struct lys_node *node)
{
    struct lys_node_anyxml *axml = (struct lys_node_anyxml *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Anyxml: ", axml->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", axml->module->name);
    info_print_text(f, axml->dsc, "Desc: ");
    info_print_text(f, axml->ref, "Reference: ");
    info_print_flags(f, axml->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_if_feature(f, axml->features, axml->features_size);
    info_print_when(f, axml->when);
    info_print_must(f, axml->must, axml->must_size);
    info_print_nacmext(f, axml->nacm);
}

static void
info_print_grouping(FILE *f, struct lys_node *node)
{
    struct lys_node_grp *group = (struct lys_node_grp *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Grouping: ", group->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", group->module->name);
    info_print_text(f, group->dsc, "Desc: ");
    info_print_text(f, group->ref, "Reference: ");
    info_print_flags(f, group->flags, LYS_STATUS_MASK, 0);
    info_print_typedef(f, group->tpdf, group->tpdf_size);
    info_print_nacmext(f, group->nacm);

    info_print_snode(f, (struct lys_node *)group, group->child, "Children:");
}

static void
info_print_case(FILE *f, struct lys_node *node)
{
    struct lys_node_case *cas = (struct lys_node_case *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Case: ", cas->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", cas->module->name);
    info_print_text(f, cas->dsc, "Desc: ");
    info_print_text(f, cas->ref, "Reference: ");
    info_print_flags(f, cas->flags, LYS_CONFIG_MASK | LYS_STATUS_MASK | LYS_MAND_MASK, 0);
    info_print_if_feature(f, cas->features, cas->features_size);
    info_print_when(f, cas->when);
    info_print_nacmext(f, cas->nacm);

    info_print_snode(f, (struct lys_node *)cas, cas->child, "Children:");
}

static void
info_print_input(FILE *f, struct lys_node *node)
{
    struct lys_node_rpc_inout *input = (struct lys_node_rpc_inout *)node;

    assert(input->parent && input->parent->nodetype == LYS_RPC);

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Input of: ", input->parent->name);
    info_print_typedef(f, input->tpdf, input->tpdf_size);

    info_print_snode(f, (struct lys_node *)input, input->child, "Children:");
}

static void
info_print_output(FILE *f, struct lys_node *node)
{
    struct lys_node_rpc_inout *output = (struct lys_node_rpc_inout *)node;

    assert(output->parent && output->parent->nodetype == LYS_RPC);

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Output of: ", output->parent->name);
    info_print_typedef(f, output->tpdf, output->tpdf_size);

    info_print_snode(f, (struct lys_node *)output, output->child, "Children:");
}

static void
info_print_notif(FILE *f, struct lys_node *node)
{
    struct lys_node_notif *ntf = (struct lys_node_notif *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Notif: ", ntf->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", ntf->module->name);
    info_print_text(f, ntf->dsc, "Desc: ");
    info_print_text(f, ntf->ref, "Reference: ");
    info_print_flags(f, ntf->flags, LYS_STATUS_MASK, 0);
    info_print_if_feature(f, ntf->features, ntf->features_size);
    info_print_typedef(f, ntf->tpdf, ntf->tpdf_size);
    info_print_nacmext(f, ntf->nacm);

    info_print_snode(f, (struct lys_node *)ntf, ntf->child, "Params:");
}

static void
info_print_rpc(FILE *f, struct lys_node *node)
{
    struct lys_node_rpc *rpc = (struct lys_node_rpc *)node;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "RPC: ", rpc->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", rpc->module->name);
    info_print_text(f, rpc->dsc, "Desc: ");
    info_print_text(f, rpc->ref, "Reference: ");
    info_print_flags(f, rpc->flags, LYS_STATUS_MASK, 0);
    info_print_if_feature(f, rpc->features, rpc->features_size);
    info_print_typedef(f, rpc->tpdf, rpc->tpdf_size);
    info_print_nacmext(f, rpc->nacm);

    info_print_snode(f, (struct lys_node *)rpc, rpc->child, "Data:");
}

int
info_print_model(FILE *f, struct lys_module *module, const char *target_node)
{
    int i;
    char *grouping_target = NULL;
    struct lys_node *target;

    if (!target_node) {
        if (f == stdout) {
            fprintf(f, "\n");
        }
        if (module->type == 0) {
            info_print_module(f, module);
        } else {
            info_print_submodule(f, (struct lys_submodule *)module);
        }
    } else {
        if ((target_node[0] == '/') || !strncmp(target_node, "type/", 5)) {
            target = resolve_schema_nodeid((target_node[0] == '/' ? target_node : target_node+4), module->data, module, LYS_AUGMENT);
            if (!target) {
                fprintf(f, "Target %s could not be resolved.\n", (target_node[0] == '/' ? target_node : target_node+4));
                return EXIT_FAILURE;
            }
        } else if (!strncmp(target_node, "grouping/", 9)) {
            /* cut the data part off */
            if (strchr(target_node+9, '/')) {
                /* HACK only temporary */
                *strchr(target_node+9, '/') = '\0';
                grouping_target = (char *)(target_node+strlen(target_node)+1);
            }
            target = resolve_schema_nodeid(target_node+9, module->data, module, LYS_USES);
            if (!target) {
                fprintf(f, "Grouping %s not found.\n", target_node+9);
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
                fprintf(f, "Typedef %s not found.\n", target_node);
                return EXIT_FAILURE;
            }

            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_typedef_detail(f, &module->tpdf[i]);
            return EXIT_SUCCESS;

        } else if (!strncmp(target_node, "identity/", 9)) {
            target_node += 9;
            for (i = 0; i < (signed)module->ident_size; ++i) {
                if (!strcmp(module->ident[i].name, target_node)) {
                    break;
                }
            }
            if (i == (signed)module->ident_size) {
                fprintf(f, "Identity %s not found.\n", target_node);
                return EXIT_FAILURE;
            }

            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_ident_detail(f, &module->ident[i]);
            return EXIT_SUCCESS;

        } else if (!strncmp(target_node, "feature/", 8)) {
            target_node += 8;
            for (i = 0; i < module->features_size; ++i) {
                if (!strcmp(module->features[i].name, target_node)) {
                    break;
                }
            }
            if (i == module->features_size) {
                fprintf(f, "Feature %s not found.\n", target_node);
                return EXIT_FAILURE;
            }

            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_feature_detail(f, &module->features[i]);
            return EXIT_SUCCESS;
        } else {
            fprintf(f, "Target could not be resolved.\n");
            return EXIT_FAILURE;
        }

        if (!strncmp(target_node, "type/", 5)) {
            if (!(target->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                fprintf(f, "Target is not a leaf or a leaf-list.\n");
                return EXIT_FAILURE;
            }
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_type_detail(f, &((struct lys_node_leaf *)target)->type, 0);
            return EXIT_SUCCESS;
        } else if (!strncmp(target_node, "grouping/", 9) && !grouping_target) {
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_grouping(f, target);
            return EXIT_SUCCESS;
        }

        /* find the node in the grouping */
        if (grouping_target) {
            target = resolve_schema_nodeid(grouping_target, target->child, module, LYS_LEAF);
            if (!target) {
                fprintf(f, "Grouping %s child \"%s\" not found.\n", target_node+9, grouping_target);
                return EXIT_FAILURE;
            }
            /* HACK return previous hack */
            --grouping_target;
            grouping_target[0] = '/';
        }

        switch (target->nodetype) {
        case LYS_CONTAINER:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_container(f, target);
            break;
        case LYS_CHOICE:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_choice(f, target);
            break;
        case LYS_LEAF:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_leaf(f, target);
            break;
        case LYS_LEAFLIST:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_leaflist(f, target);
            break;
        case LYS_LIST:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_list(f, target);
            break;
        case LYS_ANYXML:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_anyxml(f, target);
            break;
        case LYS_CASE:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_case(f, target);
            break;
        case LYS_NOTIF:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_notif(f, target);
            break;
        case LYS_RPC:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_rpc(f, target);
            break;
        case LYS_INPUT:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_input(f, target);
            break;
        case LYS_OUTPUT:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_output(f, target);
            break;
        default:
            fprintf(f, "Nodetype %s not supported.\n", strnodetype(target->nodetype));
            break;
        }
    }

    return EXIT_SUCCESS;
}
