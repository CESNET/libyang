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

#define INDENT_LEN 11

struct ly_mnode *resolve_schema_nodeid(const char *id, struct ly_mnode *start, struct ly_module *mod, LY_NODE_TYPE node_type);

static void
info_print_mnodes(FILE *f, struct ly_mnode *mnode, const char *label)
{
    assert(strlen(label) < INDENT_LEN-1);

    fprintf(f, "%-*s", INDENT_LEN, label);

    if (mnode) {
        if (mnode->name) {
            fprintf(f, "%s \"%s\"\n", strnodetype(mnode->nodetype), mnode->name);
        } else {
            fprintf(f, "%s\n", (mnode->nodetype == LY_NODE_INPUT ? "input" : "output"));
        }
        mnode = mnode->next;
        for (; mnode; mnode = mnode->next) {
            if (mnode->name) {
                fprintf(f, "%*s%s \"%s\"\n", INDENT_LEN, "", strnodetype(mnode->nodetype), mnode->name);
            } else {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", (mnode->nodetype == LY_NODE_INPUT ? "input" : "output"));
            }
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_flags(FILE *f, uint8_t flags, uint8_t mask)
{
    int first = 1;

    fprintf(f, "%-*s", INDENT_LEN, "Flags: ");

    if (mask & LY_NODE_CONFIG_MASK) {
        if (flags & LY_NODE_CONFIG_R) {
            fprintf(f, "read-only\n");
        } else {
            fprintf(f, "read-write\n");
        }
        first = 0;
    }

    if (mask & LY_NODE_STATUS_MASK) {
        if (!first) {
            fprintf(f, "%-*s", INDENT_LEN, " ");
        }

        if (flags & LY_NODE_STATUS_DEPRC) {
            fprintf(f, "deprecated\n");
        } else if (flags & LY_NODE_STATUS_OBSLT) {
            fprintf(f, "obsolete\n");
        } else {
            fprintf(f, "current\n");
        }
        first = 0;
    }

    if (mask & LY_NODE_MAND_MASK) {
        if (!first) {
            fprintf(f, "%-*s", INDENT_LEN, " ");
        }

        if (flags & LY_NODE_MAND_TRUE) {
            fprintf(f, "mandatory\n");
        } else {
            fprintf(f, "non-mandatory\n");
        }
        first = 0;
    }

    if (mask & LY_NODE_USERORDERED) {
        if (!first) {
            fprintf(f, "%-*s", INDENT_LEN, " ");
        }

        if (flags & LY_NODE_USERORDERED) {
            fprintf(f, "user-ordered\n");
        } else {
            fprintf(f, "system-ordered\n");
        }
        first = 0;
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_if_feature(FILE *f, struct ly_feature **features, uint8_t features_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "If-feats: ");

    if (features_size) {
        fprintf(f, "%s\n", features[0]->name);
        for (i = 1; i < features_size-1; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", features[i]->name);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_when(FILE *f, struct ly_when *when)
{
    fprintf(f, "%-*s", INDENT_LEN, "When: ");
    if (when) {
        fprintf(f, "%s\n", when->cond);
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_must(FILE *f, struct ly_restr *must, uint8_t must_size)
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
info_print_typedef(FILE *f, struct ly_tpdf *tpdf, uint8_t tpdf_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "Typedefs: ");

    if (tpdf_size) {
        fprintf(f, "%s\n", tpdf[0].name);
        for (i = 1; i < tpdf_size-1; ++i) {
            fprintf(f, "%*s%s", INDENT_LEN, "", tpdf[i].name);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_typedef_recursive(FILE *f, struct ly_module *mod, int *first)
{
    int i;

    if (mod->tpdf_size) {
        if (*first) {
            fprintf(f, "%s\n", mod->tpdf[0].name);
            i = 1;
        } else {
            i = 0;
        }
        *first = 0;

        for (; i < mod->tpdf_size-1; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->tpdf[i].name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        info_print_typedef_recursive(f, (struct ly_module *)mod->inc[i].submodule, first);
    }
}

static void
info_print_typedef_all(FILE *f, struct ly_module *mod)
{
    int first = 1;

    fprintf(f, "%-*s", INDENT_LEN, "Typedefs: ");

    info_print_typedef_recursive(f, mod, &first);

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_type(FILE *f, struct ly_type *type, const char *units, const char *dflt)
{
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Type: ", type->der->name);
    fprintf(f, "%-*s", INDENT_LEN, "Units: ");
    if (units) {
        fprintf(f, "%s\n", units);
    } else {
        fprintf(f, "\n");
    }

    fprintf(f, "%-*s", INDENT_LEN, "Default: ");
    if (dflt) {
        fprintf(f, "%s\n", dflt);
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_list_constr(FILE *f, uint32_t min, uint32_t max)
{
    fprintf(f, "%-*s%u .. ", INDENT_LEN, "Elements: ", min);
    if (max) {
        fprintf(f, "%u\n", max);
    } else {
        fprintf(f, "unbounded\n");
    }
}

static void
info_print_keys(FILE *f, struct ly_mnode_leaf **keys, uint8_t keys_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "Keys: ");

    if (keys_size) {
        fprintf(f, "%s\n", keys[0]->name);
        for (i = 1; i < keys_size-1; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", keys[i]->name);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_unique(FILE *f, struct ly_unique *unique, uint8_t unique_size)
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
info_print_revision(FILE *f, struct ly_revision *rev, uint8_t rev_size)
{
    int i;

    fprintf(f, "%-*s", INDENT_LEN, "Revisions: ");

    if (rev_size) {
        fprintf(f, "%s\n", rev[0].date);
        for (i = 1; i < rev_size-1; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", rev[i].date);
        }
    } else {
        fprintf(f, "\n");
    }
}

static void
info_print_import_all(FILE *f, struct ly_module *mod)
{
    int first = 1, i, j;

    fprintf(f, "%-*s", INDENT_LEN, "Imports: ");
    if (mod->imp_size) {
        fprintf(f, "%s\n", mod->imp[0].module->name);
        i = 1;
        first = 0;

        for (; i < mod->imp_size; ++i) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->imp[i].module->name);
        }
    }

    for (j = 0; j < mod->inc_size; ++j) {
        if (mod->inc[j].submodule->imp_size) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[j].submodule->imp[0].module->name);
                i = 1;
            } else {
                i = 0;
            }
            first = 0;

            for (; i < mod->inc[j].submodule->imp_size; ++i) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", mod->inc[j].submodule->imp[i].module->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_include(FILE *f, struct ly_module *mod)
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
info_print_ident_all(FILE *f, struct ly_module *mod)
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
info_print_features_all(FILE *f, struct ly_module *mod)
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
info_print_rpc_all(FILE *f, struct ly_module *mod)
{
    int first = 1, i;
    struct ly_mnode *mnode;

    fprintf(f, "%-*s", INDENT_LEN, "RPCs: ");

    if (mod->rpc) {
        fprintf(f, "%s\n", mod->rpc->name);
        mnode = mod->rpc->next;
        first = 0;

        for (; mnode; mnode = mnode->next) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mnode->name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->rpc) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[i].submodule->rpc->name);
                mnode = mod->inc[i].submodule->rpc->next;
            } else {
                mnode = mod->inc[i].submodule->rpc;
            }
            first = 0;

            for (; mnode; mnode = mnode->next) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", mnode->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_notif_all(FILE *f, struct ly_module *mod)
{
    int first = 1, i;
    struct ly_mnode *mnode;

    fprintf(f, "%-*s", INDENT_LEN, "Notifs: ");

    if (mod->notif) {
        fprintf(f, "%s\n", mod->notif->name);
        mnode = mod->notif->next;
        first = 0;

        for (; mnode; mnode = mnode->next) {
            fprintf(f, "%*s%s\n", INDENT_LEN, "", mnode->name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->notif) {
            if (first) {
                fprintf(f, "%s\n", mod->inc[i].submodule->notif->name);
                mnode = mod->inc[i].submodule->notif->next;
            } else {
                mnode = mod->inc[i].submodule->notif;
            }
            first = 0;

            for (; mnode; mnode = mnode->next) {
                fprintf(f, "%*s%s\n", INDENT_LEN, "", mnode->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_mnodes_all(FILE *f, struct ly_module *mod)
{
    int first = 1, i;
    struct ly_mnode *mnode;

    fprintf(f, "%-*s", INDENT_LEN, "Data: ");

    if (mod->data) {
        fprintf(f, "%s \"%s\"\n", strnodetype(mod->data->nodetype), mod->data->name);
        mnode = mod->data->next;
        first = 0;

        for (; mnode; mnode = mnode->next) {
            fprintf(f, "%*s%s \"%s\"\n", INDENT_LEN, "", strnodetype(mnode->nodetype), mnode->name);
        }
    }

    for (i = 0; i < mod->inc_size; ++i) {
        if (mod->inc[i].submodule->data) {
            if (first) {
                fprintf(f, "%s \"%s\"\n", strnodetype(mod->inc[i].submodule->data->nodetype), mod->inc[i].submodule->data->name);
                mnode = mod->inc[i].submodule->data->next;
            } else {
                mnode = mod->inc[i].submodule->data;
            }
            first = 0;

            for (; mnode; mnode = mnode->next) {
                fprintf(f, "%*s%s \"%s\"\n", INDENT_LEN, "", strnodetype(mnode->nodetype), mnode->name);
            }
        }
    }

    if (first) {
        fprintf(f, "\n");
    }
}

static void
info_print_module(FILE *f, struct ly_module *module)
{
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", module->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Namespace: ", module->ns);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Prefix: ", module->prefix);
    fprintf(f, "%-*s", INDENT_LEN, "Org: ");
    if (module->org) {
        fprintf(f, "%s\n", module->org);
    } else {
        fprintf(f, "\n");
    }
    fprintf(f, "%-*s%s\n", INDENT_LEN, "YANG ver: ", (module->version == 2 ? "1.1" : "1.0"));
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Deviated: ", (module->deviated ? "yes" : "no"));

    info_print_revision(f, module->rev, module->rev_size);
    info_print_include(f, module);
    info_print_import_all(f, module);
    info_print_typedef_all(f, module);
    info_print_ident_all(f, module);
    info_print_features_all(f, module);

    info_print_rpc_all(f, module);
    info_print_notif_all(f, module);
    info_print_mnodes_all(f, module);
}

static void
info_print_submodule(FILE *f, struct ly_submodule *module)
{
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Submodule: ", module->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Parent: ", module->belongsto->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Prefix: ", module->prefix);
    fprintf(f, "%-*s", INDENT_LEN, "Org: ");
    if (module->org) {
        fprintf(f, "%s\n", module->org);
    } else {
        fprintf(f, "\n");
    }
    fprintf(f, "%-*s%s\n", INDENT_LEN, "YANG ver: ", (module->version == 2 ? "1.1" : "1.0"));
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Deviated: ", (module->deviated ? "yes" : "no"));

    info_print_revision(f, module->rev, module->rev_size);
    info_print_include(f, (struct ly_module *)module);
    info_print_import_all(f, (struct ly_module *)module);
    info_print_typedef_all(f, (struct ly_module *)module);
    info_print_ident_all(f, (struct ly_module *)module);
    info_print_features_all(f, (struct ly_module *)module);

    info_print_rpc_all(f, (struct ly_module *)module);
    info_print_notif_all(f, (struct ly_module *)module);
    info_print_mnodes_all(f, (struct ly_module *)module);
}

static void
info_print_container(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_container *cont = (struct ly_mnode_container *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Container: ", cont->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", cont->module->name);
    info_print_flags(f, cont->flags, LY_NODE_CONFIG_MASK | LY_NODE_STATUS_MASK | LY_NODE_MAND_MASK);
    fprintf(f, "%-*s", INDENT_LEN, "Presence: ");
    if (cont->presence) {
        fprintf(f, "\"%s\"\n", cont->presence);
    } else {
        fprintf(f, "\n");
    }
    info_print_if_feature(f, cont->features, cont->features_size);
    info_print_when(f, cont->when);
    info_print_must(f, cont->must, cont->must_size);
    info_print_typedef(f, cont->tpdf, cont->tpdf_size);

    info_print_mnodes(f, cont->child, "Children:");
}

static void
info_print_choice(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_choice *choice = (struct ly_mnode_choice *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Choice: ", choice->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", choice->module->name);
    info_print_flags(f, choice->flags, LY_NODE_CONFIG_MASK | LY_NODE_STATUS_MASK | LY_NODE_MAND_MASK);
    fprintf(f, "%-*s", INDENT_LEN, "Default: ");
    if (choice->dflt) {
        fprintf(f, "%s\n", choice->dflt->name);
    } else {
        fprintf(f, "\n");
    }
    info_print_if_feature(f, choice->features, choice->features_size);
    info_print_when(f, choice->when);

    info_print_mnodes(f, choice->child, "Cases:");
}

static void
info_print_leaf(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_leaf *leaf = (struct ly_mnode_leaf *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Leaf: ", leaf->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", leaf->module->name);
    info_print_flags(f, leaf->flags, LY_NODE_CONFIG_MASK | LY_NODE_STATUS_MASK | LY_NODE_MAND_MASK);
    info_print_type(f, &leaf->type, leaf->units, leaf->dflt);
    info_print_if_feature(f, leaf->features, leaf->features_size);
    info_print_when(f, leaf->when);
    info_print_must(f, leaf->must, leaf->must_size);
}

static void
info_print_leaflist(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_leaflist *llist = (struct ly_mnode_leaflist *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Leaflist: ", llist->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", llist->module->name);
    info_print_flags(f, llist->flags, LY_NODE_CONFIG_MASK | LY_NODE_STATUS_MASK | LY_NODE_MAND_MASK | LY_NODE_USERORDERED);
    info_print_type(f, &llist->type, llist->units, NULL);
    info_print_list_constr(f, llist->min, llist->max);
    info_print_if_feature(f, llist->features, llist->features_size);
    info_print_when(f, llist->when);
    info_print_must(f, llist->must, llist->must_size);
}

static void
info_print_list(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_list *list = (struct ly_mnode_list *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "List: ", list->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", list->module->name);
    info_print_flags(f, list->flags, LY_NODE_CONFIG_MASK | LY_NODE_STATUS_MASK | LY_NODE_MAND_MASK | LY_NODE_USERORDERED);
    info_print_list_constr(f, list->min, list->max);
    info_print_if_feature(f, list->features, list->features_size);
    info_print_when(f, list->when);
    info_print_must(f, list->must, list->must_size);
    info_print_keys(f, list->keys, list->keys_size);
    info_print_unique(f, list->unique, list->unique_size);
    info_print_typedef(f, list->tpdf, list->tpdf_size);

    info_print_mnodes(f, list->child, "Children:");
}

static void
info_print_anyxml(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_anyxml *axml = (struct ly_mnode_anyxml *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Anyxml: ", axml->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", axml->module->name);
    info_print_flags(f, axml->flags, LY_NODE_CONFIG_MASK | LY_NODE_STATUS_MASK | LY_NODE_MAND_MASK);
    info_print_if_feature(f, axml->features, axml->features_size);
    info_print_when(f, axml->when);
    info_print_must(f, axml->must, axml->must_size);
}

static void
info_print_grouping(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_grp *group = (struct ly_mnode_grp *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Grouping: ", group->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", group->module->name);
    info_print_flags(f, group->flags, LY_NODE_STATUS_MASK);
    info_print_typedef(f, group->tpdf, group->tpdf_size);

    info_print_mnodes(f, group->child, "Children:");
}

static void
info_print_case(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_case *cas = (struct ly_mnode_case *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Case: ", cas->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", cas->module->name);
    info_print_flags(f, cas->flags, LY_NODE_CONFIG_MASK | LY_NODE_STATUS_MASK | LY_NODE_MAND_MASK);
    info_print_if_feature(f, cas->features, cas->features_size);
    info_print_when(f, cas->when);

    info_print_mnodes(f, cas->child, "Children:");
}

static void
info_print_input(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_input_output *input = (struct ly_mnode_input_output *)mnode;

    assert(input->parent && input->parent->nodetype == LY_NODE_RPC);

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Input of: ", input->parent->name);
    info_print_typedef(f, input->tpdf, input->tpdf_size);

    info_print_mnodes(f, input->child, "Children:");
}

static void
info_print_output(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_input_output *output = (struct ly_mnode_input_output *)mnode;

    assert(output->parent && output->parent->nodetype == LY_NODE_RPC);

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Output of: ", output->parent->name);
    info_print_typedef(f, output->tpdf, output->tpdf_size);

    info_print_mnodes(f, output->child, "Children:");
}

static void
info_print_notif(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_notif *ntf = (struct ly_mnode_notif *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "Notif: ", ntf->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", ntf->module->name);
    info_print_flags(f, ntf->flags, LY_NODE_STATUS_MASK);
    info_print_if_feature(f, ntf->features, ntf->features_size);
    info_print_typedef(f, ntf->tpdf, ntf->tpdf_size);

    info_print_mnodes(f, ntf->child, "Params:");
}

static void
info_print_rpc(FILE *f, struct ly_mnode *mnode)
{
    struct ly_mnode_rpc *rpc = (struct ly_mnode_rpc *)mnode;

    fprintf(f, "%-*s%s\n", INDENT_LEN, "RPC: ", rpc->name);
    fprintf(f, "%-*s%s\n", INDENT_LEN, "Module: ", rpc->module->name);
    info_print_flags(f, rpc->flags, LY_NODE_STATUS_MASK);
    info_print_if_feature(f, rpc->features, rpc->features_size);
    info_print_typedef(f, rpc->tpdf, rpc->tpdf_size);

    info_print_mnodes(f, rpc->child, "Data:");
}

int
info_print_model(FILE *f, struct ly_module *module, const char *target_node)
{
    struct ly_mnode *target;

    if (!target_node) {
        if (f == stdout) {
            fprintf(f, "\n");
        }
        if (module->type == 0) {
            info_print_module(f, module);
        } else {
            info_print_submodule(f, (struct ly_submodule *)module);
        }
    } else {
        if (target_node[0] != '/') {
            fprintf(f, "Target node is not an absolute schema node.\n");
            return EXIT_FAILURE;
        }
        target = resolve_schema_nodeid(target_node, module->data, module, LY_NODE_AUGMENT);
        if (!target) {
            fprintf(f, "Target %s could not be resolved.\n", target_node);
            return EXIT_FAILURE;
        }

        switch (target->nodetype) {
        case LY_NODE_CONTAINER:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_container(f, target);
            break;
        case LY_NODE_CHOICE:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_choice(f, target);
            break;
        case LY_NODE_LEAF:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_leaf(f, target);
            break;
        case LY_NODE_LEAFLIST:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_leaflist(f, target);
            break;
        case LY_NODE_LIST:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_list(f, target);
            break;
        case LY_NODE_ANYXML:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_anyxml(f, target);
            break;
        case LY_NODE_GROUPING:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_grouping(f, target);
            break;
        case LY_NODE_CASE:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_case(f, target);
            break;
        case LY_NODE_NOTIF:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_notif(f, target);
            break;
        case LY_NODE_RPC:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_rpc(f, target);
            break;
        case LY_NODE_INPUT:
            if (f == stdout) {
                fprintf(f, "\n");
            }
            info_print_input(f, target);
            break;
        case LY_NODE_OUTPUT:
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
