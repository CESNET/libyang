/**
 * @file printer/tree.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief TREE printer for libyang data model structure
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

/* spec_config = 0 (no special config status), 1 (read-only - rpc output, notification), 2 (write-only - rpc input) */
static void tree_print_mnode_choice(FILE *f, struct lys_module* module, int level, char *indent, unsigned int max_name_len,
                                    struct lys_node *mnode, int mask, int spec_config, struct lys_submodule *main_submod);
static void tree_print_mnode(FILE *f, struct lys_module *module, int level, char *indent, unsigned int max_name_len, struct lys_node *mnode,
                             int mask, int spec_config, struct lys_submodule *main_submod);

static int
is_enabled(struct lys_node *mnode)
{
    int i;

    for (i = 0; i < mnode->features_size; i++) {
        if (!(mnode->features[i]->flags & LYS_FENABLED)) {
            return 0;
        }
    }

    if (mnode->parent && (mnode->parent->nodetype == LYS_AUGMENT)) {
        for (i = 0; i < mnode->parent->features_size; i++) {
            if (!(mnode->parent->features[i]->flags & LYS_FENABLED)) {
                return 0;
            }
        }
    }

    return 1;
}

static int
sibling_is_valid_child(const struct lys_node *mnode, int including)
{
    struct lys_node *cur;

    if (mnode == NULL) {
        return 0;
    }

    /* has a following printed child */
    LY_TREE_FOR((struct lys_node *)(including ? mnode : mnode->next), cur) {
        if (is_enabled(cur) && (cur->nodetype &
                (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_CHOICE |
                 LYS_RPC | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF | LYS_CASE))) {
            return 1;
        }
    }

    /* if in uses, the following printed child can actually be in the parent node :-/ */
    if (mnode->parent && mnode->parent->nodetype == LYS_USES) {
        return sibling_is_valid_child(mnode->parent, 0);
    }

    return 0;
}

static char *
create_indent(int level, const char *old_indent, const struct lys_node *mnode, int shorthand, struct lys_submodule *main_submod)
{
    int next_is_case = 0, is_case = 0, has_next = 0, i, found;
    char *new_indent = malloc((level * 4 + 1) * sizeof (char));

    strcpy(new_indent, old_indent);

    /* this is the indent of a case (standard or shorthand) */
    if ((mnode->nodetype == LYS_CASE) || shorthand) {
        is_case = 1;
    }

    /* this is the direct child of a case */
    if (!is_case && mnode->parent && (mnode->parent->nodetype & (LYS_CASE | LYS_CHOICE))) {
        /* it is not the only child */
        if (mnode->next && mnode->next->parent && (mnode->next->parent->nodetype == LYS_CHOICE)) {
            next_is_case = 1;
        }
    }

    /* next is a node that will actually be printed */
    has_next = sibling_is_valid_child(mnode, 0);

    /* there is no next, but we are in top-level of a submodule */
    if (!has_next && (mnode->module->type == 1) && !mnode->parent) {
        struct lys_submodule *submod = (struct lys_submodule *)mnode->module;
        struct lys_module *mod = submod->belongsto;

        /* a special case when we check the includes of a submodule */
        if (main_submod) {
            if (submod != main_submod) {
                found = 0;
                for (i = 0; i < main_submod->inc_size; i++) {
                    if (found) {
                        if (mnode->nodetype == LYS_RPC) {
                            has_next = sibling_is_valid_child(main_submod->inc[i].submodule->rpc, 1);
                        } else if (mnode->nodetype == LYS_NOTIF) {
                            has_next = sibling_is_valid_child(main_submod->inc[i].submodule->notif, 1);
                        } else {
                            has_next = sibling_is_valid_child(main_submod->inc[i].submodule->data, 1);
                        }
                        if (has_next) {
                            break;
                        }
                    }
                    if (!found && (submod == main_submod->inc[i].submodule)) {
                        found = 1;
                    }
                }

                if (!has_next) {
                    if (mnode->nodetype == LYS_RPC) {
                        has_next = sibling_is_valid_child(main_submod->rpc, 1);
                    } else if (mnode->nodetype == LYS_NOTIF) {
                        has_next = sibling_is_valid_child(main_submod->notif, 1);
                    } else {
                        has_next = sibling_is_valid_child(main_submod->data, 1);
                    }
                }
            }

            goto strcat_indent;
        }

        /* find this submodule, check all the next ones for valid printed nodes */
        found = 0;
        for (i = 0; i < mod->inc_size; i++) {
            /* we found ours, check all the following submodules and the module */
            if (found) {
                if (mnode->nodetype == LYS_RPC) {
                    has_next = sibling_is_valid_child(mod->inc[i].submodule->rpc, 1);
                } else if (mnode->nodetype == LYS_NOTIF) {
                    has_next = sibling_is_valid_child(mod->inc[i].submodule->notif, 1);
                } else {
                    has_next = sibling_is_valid_child(mod->inc[i].submodule->data, 1);
                }
                if (has_next) {
                    break;
                }
            }
            if (!found && (submod == mod->inc[i].submodule)) {
                found = 1;
            }
        }

        /* there is nothing in submodules, check module */
        if (!has_next) {
            if (mnode->nodetype == LYS_RPC) {
                has_next = sibling_is_valid_child(mod->rpc, 1);
            } else if (mnode->nodetype == LYS_NOTIF) {
                has_next = sibling_is_valid_child(mod->notif, 1);
            } else {
                has_next = sibling_is_valid_child(mod->data, 1);
            }
        }
    }

strcat_indent:
    if (has_next && !next_is_case) {
        strcat(new_indent, "|  ");
    } else {
        strcat(new_indent, "   ");
    }

    return new_indent;
}

static unsigned int
get_max_name_len(struct lys_module *module, struct lys_node *mnode)
{
    struct lys_node *sub;
    unsigned int max_name_len = 0, uses_max_name_len, name_len;

    LY_TREE_FOR(mnode, sub) {
        if (sub->nodetype == LYS_USES) {
            uses_max_name_len = get_max_name_len(module, sub->child);
            if (uses_max_name_len > max_name_len) {
                max_name_len = uses_max_name_len;
            }
        } else if (sub->nodetype &
                (LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST
                | LYS_ANYXML | LYS_CASE)) {
            name_len = strlen(sub->name) + (module == sub->module ? 0 : strlen(sub->module->prefix)+1);
            if (name_len > max_name_len) {
                max_name_len = name_len;
            }
        }
    }

    return max_name_len;
}

static void
tree_print_type(FILE *f, struct lys_type *type)
{
    if (type->base == LY_TYPE_LEAFREF) {
        fprintf(f, "-> %s", type->info.lref.path);
    } else if (type->prefix) {
        fprintf(f, "%s:%s", type->prefix, type->der->name);
    } else {
        fprintf(f, "%s", type->der->name);
    }
}

static void
tree_print_features(FILE *f, const struct lys_feature **features, uint8_t features_size)
{
    int i;

    if (!features_size) {
        return;
    }

    fprintf(f, " {");
    for (i = 0; i < features_size; i++) {
        if (i > 0) {
            fprintf(f, ",");
        }
        fprintf(f, "%s", features[i]->name);
    }
    fprintf(f, "}?");
}

static void
tree_print_input_output(FILE *f, struct lys_module *module, int level, char *indent, struct lys_node *mnode, int spec_config, struct lys_submodule *main_submod)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node *sub;

    assert(spec_config);

    fprintf(f, "%s+--%s %s\n", indent, (spec_config == 1 ? "-w" : "ro"), (spec_config == 1 ? "input" : "output"));

    level++;
    new_indent = create_indent(level, indent, mnode, 0, main_submod);

    max_child_len = get_max_name_len(module, mnode->child);

    LY_TREE_FOR(mnode->child, sub) {
        tree_print_mnode(f, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                         spec_config, main_submod);
    }

    free(new_indent);
}

static void
tree_print_container(FILE *f, struct lys_module *module, int level, char *indent, struct lys_node *mnode, int spec_config, struct lys_submodule *main_submod)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node_container *cont = (struct lys_node_container *)mnode;
    struct lys_node *sub;

    assert(spec_config >= 0 && spec_config <= 2);

    fprintf(f, "%s%s--", indent,
            (cont->flags & LYS_STATUS_DEPRC ? "x" : (cont->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (cont->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    if (module != cont->module) {
        fprintf(f, "%s:", cont->module->prefix);
    }

    fprintf(f, "%s%s", cont->name, (cont->presence ? "!" : ""));

    tree_print_features(f, (const struct lys_feature **)cont->features, cont->features_size);

    fprintf(f, "\n");

    level++;
    new_indent = create_indent(level, indent, mnode, 0, main_submod);

    max_child_len = get_max_name_len(module, mnode->child);

    LY_TREE_FOR(cont->child, sub) {
        tree_print_mnode(f, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                         spec_config, main_submod);
    }

    free(new_indent);
}

static void
tree_print_choice(FILE *f, struct lys_module *module, int level, char *indent, struct lys_node *mnode, int spec_config, struct lys_submodule *main_submod)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node_choice *choice = (struct lys_node_choice *)mnode;
    struct lys_node *sub;

    assert(spec_config >= 0 && spec_config <= 2);

    fprintf(f, "%s%s--", indent,
            (choice->flags & LYS_STATUS_DEPRC ? "x" : (choice->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (choice->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    fprintf(f, "(");

    if (module != choice->module) {
        fprintf(f, "%s:", choice->module->prefix);
    }

    fprintf(f, "%s)%s", choice->name, (choice->flags & LYS_MAND_TRUE ? "" : "?"));

    if (choice->dflt != NULL) {
        fprintf(f, " <%s>", choice->dflt->name);
    }

    tree_print_features(f, (const struct lys_feature **)choice->features, choice->features_size);

    fprintf(f, "\n");

    level++;
    new_indent = create_indent(level, indent, mnode, 0, main_submod);

    max_child_len = get_max_name_len(module, mnode->child);

    LY_TREE_FOR(choice->child, sub) {
        tree_print_mnode_choice(f, module, level, new_indent, max_child_len, sub,
                                LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML,
                                spec_config, main_submod);
    }

    free(new_indent);
}

static void
tree_print_case(FILE *f, struct lys_module *module, int level, char *indent, unsigned int max_name_len, struct lys_node *mnode, int shorthand, int spec_config, struct lys_submodule *main_submod)
{
    char *new_indent;
    struct lys_node_case *cas = (struct lys_node_case *)mnode;
    struct lys_node *sub;

    fprintf(f, "%s%s--:(", indent,
            (cas->flags & LYS_STATUS_DEPRC ? "x" : (cas->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (module != cas->module) {
        fprintf(f, "%s:", cas->module->prefix);
    }

    fprintf(f, "%s)", cas->name);

    tree_print_features(f, (const struct lys_feature **)cas->features, cas->features_size);

    fprintf(f, "\n");

    level++;
    new_indent = create_indent(level, indent, mnode, shorthand, main_submod);

    if (shorthand) {
        tree_print_mnode(f, module, level, new_indent, max_name_len, mnode,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                         spec_config, main_submod);
    } else {
        LY_TREE_FOR(mnode->child, sub) {
            tree_print_mnode(f, module, level, new_indent, max_name_len, sub,
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                             spec_config, main_submod);
        }
    }

    free(new_indent);
}

static void
tree_print_anyxml(FILE *f, struct lys_module *module, char *indent, unsigned int max_name_len, struct lys_node *mnode, int spec_config)
{
    uint8_t prefix_len;
    struct lys_node_anyxml *anyxml = (struct lys_node_anyxml *)mnode;

    assert(spec_config >= 0 && spec_config <= 2);

    fprintf(f, "%s%s--", indent,
            (anyxml->flags & LYS_STATUS_DEPRC ? "x" : (anyxml->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (anyxml->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    prefix_len = 0;
    if (module != anyxml->module) {
        fprintf(f, "%s:", anyxml->module->prefix);
        prefix_len = strlen(anyxml->module->prefix)+1;
    }

    fprintf(f, "%s%s%*sanyxml", anyxml->name, (anyxml->flags & LYS_MAND_TRUE ? " " : "?"),
            3 + (int)((max_name_len - strlen(anyxml->name)) - prefix_len), "   ");

    tree_print_features(f, (const struct lys_feature **)anyxml->features, anyxml->features_size);

    fprintf(f, "\n");
}

static void
tree_print_leaf(FILE *f, struct lys_module *module, char *indent, unsigned int max_name_len, struct lys_node *mnode, int spec_config)
{
    uint8_t prefix_len;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)mnode;
    struct lys_node *parent;
    struct lys_node_list *list;
    int i, is_key = 0;

    assert(spec_config >= 0 && spec_config <= 2);

    for (parent = leaf->parent; parent && parent->nodetype == LYS_USES; parent = parent->parent);
    if (parent->nodetype == LYS_LIST) {
        list = (struct lys_node_list *)parent;
        for (i = 0; i < list->keys_size; i++) {
            if (list->keys[i] == leaf) {
                is_key = 1;
                break;
            }
        }
    }

    fprintf(f, "%s%s--", indent,
            (leaf->flags & LYS_STATUS_DEPRC ? "x" : (leaf->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (leaf->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    prefix_len = 0;
    if (module != leaf->module) {
        fprintf(f, "%s:", leaf->module->prefix);
        prefix_len = strlen(leaf->module->prefix)+1;
    }

    fprintf(f, "%s%s%*s", leaf->name, ((leaf->flags & LYS_MAND_TRUE) || is_key ? " " : "?"),
            3 + (int)((max_name_len - strlen(leaf->name)) - prefix_len), "   ");

    tree_print_type(f, &leaf->type);

    if (leaf->dflt != NULL) {
        fprintf(f, " <%s>", leaf->dflt);
    }

    tree_print_features(f, (const struct lys_feature **)leaf->features, leaf->features_size);

    fprintf(f, "\n");
}

static void
tree_print_leaflist(FILE *f, struct lys_module *module, char *indent, unsigned int max_name_len, struct lys_node *mnode, int spec_config)
{
    struct lys_node_leaflist *leaflist = (struct lys_node_leaflist *)mnode;

    assert(spec_config >= 0 && spec_config <= 2);

    fprintf(f, "%s%s--", indent,
            (leaflist->flags & LYS_STATUS_DEPRC ? "x" : (leaflist->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (leaflist->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    if (module != leaflist->module) {
        fprintf(f, "%s:", leaflist->module->prefix);
    }

    fprintf(f, "%s*%*s", leaflist->name, 3 + (int)(max_name_len - strlen(leaflist->name)), "   ");

    tree_print_type(f, &leaflist->type);

    tree_print_features(f, (const struct lys_feature **)leaflist->features, leaflist->features_size);

    fprintf(f, "\n");
}

static void
tree_print_list(FILE *f, struct lys_module *module, int level, char *indent, struct lys_node *mnode, int spec_config, struct lys_submodule *main_submod)
{
    int i;
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)mnode;

    fprintf(f, "%s%s--", indent,
            (list->flags & LYS_STATUS_DEPRC ? "x" : (list->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        fprintf(f, "%s ", (list->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        fprintf(f, "-w ");
    } else if (spec_config == 2) {
        fprintf(f, "ro ");
    }

    if (module != list->module) {
        fprintf(f, "%s:", list->module->prefix);
    }

    fprintf(f, "%s*", list->name);

    for (i = 0; i < list->keys_size; i++) {
        if (i == 0) {
            fprintf(f, " [");
        }
        fprintf(f, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? "," : "]");
    }

    tree_print_features(f, (const struct lys_feature **)list->features, list->features_size);

    fprintf(f, "\n");

    level++;
    new_indent = create_indent(level, indent, mnode, 0, main_submod);

    max_child_len = get_max_name_len(module, mnode->child);

    LY_TREE_FOR(mnode->child, sub) {
        tree_print_mnode(f, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ANYXML,
                         spec_config, main_submod);
    }

    free(new_indent);
}

static void
tree_print_uses(FILE *f, struct lys_module *module, int level, char *indent, unsigned int max_name_len, struct lys_node *mnode, int spec_config, struct lys_submodule *main_submod)
{
    struct lys_node *node;
    struct lys_node_uses *uses = (struct lys_node_uses *)mnode;

    LY_TREE_FOR(uses->child, node) {
        tree_print_mnode(f, module, level, indent, max_name_len, node,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ANYXML,
                         spec_config, main_submod);
    }
}

static void
tree_print_rpc(FILE *f, struct lys_module *module, int level, char *indent, struct lys_node *mnode, struct lys_submodule *main_submod)
{
    char *new_indent;
    struct lys_node *node;
    struct lys_node_rpc *rpc = (struct lys_node_rpc *)mnode;

    if (!is_enabled(mnode)) {
        return;
    }

    fprintf(f, "%s%s---x %s", indent,
            (rpc->flags & LYS_STATUS_DEPRC ? "x" : (rpc->flags & LYS_STATUS_OBSLT ? "o" : "+")), rpc->name);

    tree_print_features(f, (const struct lys_feature **)rpc->features, rpc->features_size);

    fprintf(f, "\n");

    level++;
    new_indent = create_indent(level, indent, mnode, 0, main_submod);

    LY_TREE_FOR(rpc->child, node) {
        if (node->nodetype == LYS_INPUT) {
            tree_print_input_output(f, module, level, new_indent, node, 1, main_submod);
        } else if (node->nodetype == LYS_OUTPUT) {
            tree_print_input_output(f, module, level, new_indent, node, 2, main_submod);
        }
    }

    free(new_indent);
}

static void
tree_print_notif(FILE *f, struct lys_module *module, int level, char *indent, struct lys_node *mnode, struct lys_submodule *main_submod)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node *node;
    struct lys_node_notif *notif = (struct lys_node_notif *)mnode;

    if (!is_enabled(mnode)) {
        return;
    }

    fprintf(f, "%s%s---n %s", indent,
            (notif->flags & LYS_STATUS_DEPRC ? "x" : (notif->flags & LYS_STATUS_OBSLT ? "o" : "+")),
            notif->name);

    tree_print_features(f, (const struct lys_feature **)notif->features, notif->features_size);

    fprintf(f, "\n");

    level++;
    new_indent = create_indent(level, indent, mnode, 0, main_submod);

    max_child_len = get_max_name_len(module, mnode->child);

    LY_TREE_FOR(notif->child, node) {
        tree_print_mnode(f, module, level, new_indent, max_child_len, node,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                         2, main_submod);
    }

    free(new_indent);
}

static void
tree_print_mnode_choice(FILE *f, struct lys_module *module, int level, char *indent, unsigned int max_name_len, struct lys_node *mnode, int mask,
                        int spec_config, struct lys_submodule *main_submod)
{
    if (!is_enabled(mnode)) {
        return;
    }

    if (mnode->nodetype & mask) {
        if (mnode->nodetype == LYS_CASE) {
            tree_print_case(f, module, level, indent, max_name_len, mnode, 0, spec_config, main_submod);
        } else {
            tree_print_case(f, module, level, indent, max_name_len, mnode, 1, spec_config, main_submod);
        }
    }
}

static void
tree_print_mnode(FILE *f, struct lys_module *module, int level, char *indent, unsigned int max_name_len, struct lys_node *mnode, int mask,
                 int spec_config, struct lys_submodule *main_submod)
{
    if (!is_enabled(mnode)) {
        return;
    }

    switch (mnode->nodetype & mask) {
    case LYS_CONTAINER:
        tree_print_container(f, module, level, indent, mnode, spec_config, main_submod);
        break;
    case LYS_CHOICE:
        tree_print_choice(f, module, level, indent, mnode, spec_config, main_submod);
        break;
    case LYS_LEAF:
        tree_print_leaf(f, module, indent, max_name_len, mnode, spec_config);
        break;
    case LYS_LEAFLIST:
        tree_print_leaflist(f, module, indent, max_name_len, mnode, spec_config);
        break;
    case LYS_LIST:
        tree_print_list(f, module, level, indent, mnode, spec_config, main_submod);
        break;
    case LYS_ANYXML:
        tree_print_anyxml(f, module, indent, max_name_len, mnode, spec_config);
        break;
    case LYS_USES:
        tree_print_uses(f, module, level, indent, max_name_len, mnode, spec_config, main_submod);
        break;
    default:
        break;
    }
}

int
tree_print_model(FILE *f, struct lys_module *module)
{
    struct lys_node *mnode;
    struct lys_submodule *submod;
    unsigned int max_child_len;
    int level = 1, i, have_rpcs = 0, have_notifs = 0;
    char *indent = malloc((level * 4 + 1) * sizeof (char));
    strcpy(indent, "   ");

    if (module->type) {
        submod = (struct lys_submodule *)module;
        fprintf(f, "submodule: %s (belongs-to %s)\n", submod->name, submod->belongsto->name);
    } else {
        submod = NULL;
        fprintf(f, "module: %s\n", module->name);
    }

    /* included submodules */
    for (i = 0; i < module->inc_size; i++) {
        max_child_len = get_max_name_len((struct lys_module *)module->inc[i].submodule, module->inc[i].submodule->data);

        LY_TREE_FOR(module->inc[i].submodule->data, mnode) {
            tree_print_mnode(f, (struct lys_module *)module->inc[i].submodule, level, indent, max_child_len, mnode,
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST
                             | LYS_ANYXML | LYS_USES, 0, submod);
        }
    }

    /* module */
    max_child_len = get_max_name_len(module, module->data);
    level++;

    LY_TREE_FOR(module->data, mnode) {
        tree_print_mnode(f, module, level, indent, max_child_len, mnode,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST
                         | LYS_ANYXML | LYS_USES, 0, submod);
    }

    /* rpc */
    if (module->rpc) {
        have_rpcs = 1;
    } else {
        for (i = 0; i < module->inc_size; i++) {
            if (module->inc[i].submodule->rpc) {
                have_rpcs = 1;
                break;
            }
        }
    }
    if (have_rpcs) {
        fprintf(f, "rpcs:\n");
        for (i = 0; i < module->inc_size; i++) {
            LY_TREE_FOR(module->inc[i].submodule->rpc, mnode) {
                tree_print_rpc(f, (struct lys_module *)module->inc[i].submodule, level, indent, mnode, submod);
            }
        }
        LY_TREE_FOR(module->rpc, mnode) {
            tree_print_rpc(f, module, level, indent, mnode, submod);
        }
    }

    /* notification */
    if (module->notif) {
        have_notifs = 1;
    } else {
        for (i = 0; i < module->inc_size; i++) {
            if (module->inc[i].submodule->notif) {
                have_notifs = 1;
                break;
            }
        }
    }
    if (have_notifs) {
        fprintf(f, "notifications:\n");
        for (i = 0; i < module->inc_size; i++) {
            LY_TREE_FOR(module->inc[i].submodule->notif, mnode) {
                tree_print_notif(f, (struct lys_module *)module->inc[i].submodule, level, indent, mnode, submod);
            }
        }
        LY_TREE_FOR(module->notif, mnode) {
            tree_print_notif(f, module, level, indent, mnode, submod);
        }
    }

    free(indent);
    return EXIT_SUCCESS;
}
