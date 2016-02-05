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

#include "common.h"
#include "printer.h"
#include "tree_schema.h"

/* spec_config = 0 (no special config status), 1 (read-only - rpc output, notification), 2 (write-only - rpc input) */
static void tree_print_choice_content(struct lyout *out, const struct lys_module* module, int level, char *indent,
                                      unsigned int max_name_len, const struct lys_node *node, int mask,
                                      int spec_config);
static void tree_print_snode(struct lyout *out, const struct lys_module *module, int level, char *indent,
                             unsigned int max_name_len, const struct lys_node *node, int mask, int spec_config);

static int
sibling_is_valid_child(const struct lys_node *node, int including)
{
    struct lys_node *cur;

    if (node == NULL) {
        return 0;
    }

    /* has a following printed child */
    LY_TREE_FOR((struct lys_node *)(including ? node : node->next), cur) {
        if (!lys_is_disabled(cur, 0)) {
            if (cur->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_CHOICE |
                    LYS_INPUT | LYS_OUTPUT | LYS_CASE)) {
                return 1;
            }
            if ((cur->nodetype == LYS_USES) && sibling_is_valid_child(cur->child, 1)) {
                return 1;
            }
        }
    }

    /* if in uses, the following printed child can actually be in the parent node :-/ */
    if (node->parent && node->parent->nodetype == LYS_USES) {
        return sibling_is_valid_child(node->parent, 0);
    }

    return 0;
}

static char *
create_indent(int level, const char *old_indent, const struct lys_node *node, int shorthand)
{
    int next_is_case = 0, is_case = 0, has_next = 0;
    char *new_indent = malloc((level * 4 + 1) * sizeof (char));

    if (!new_indent) {
        LOGMEM;
        return NULL;
    }

    strcpy(new_indent, old_indent);

    /* this is the indent of a case (standard or shorthand) */
    if ((node->nodetype == LYS_CASE) || shorthand) {
        is_case = 1;
    }

    /* this is the direct child of a case */
    if (!is_case && node->parent && (node->parent->nodetype & (LYS_CASE | LYS_CHOICE))) {
        /* it is not the only child */
        if (node->next && node->next->parent && (node->next->parent->nodetype == LYS_CHOICE)) {
            next_is_case = 1;
        }
    }

    /* next is a node that will actually be printed */
    has_next = sibling_is_valid_child(node, 0);

    if (has_next && !next_is_case) {
        strcat(new_indent, "|  ");
    } else {
        strcat(new_indent, "   ");
    }

    return new_indent;
}

static unsigned int
get_max_name_len(const struct lys_module *module, const struct lys_node *node)
{
    const struct lys_node *sub;
    struct lys_module *mod;
    unsigned int max_name_len = 0, uses_max_name_len, name_len;

    LY_TREE_FOR(node, sub) {
        if (sub->nodetype == LYS_USES) {
            uses_max_name_len = get_max_name_len(module, sub->child);
            if (uses_max_name_len > max_name_len) {
                max_name_len = uses_max_name_len;
            }
        } else if (sub->nodetype &
                (LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST
                | LYS_ANYXML | LYS_CASE)) {
            mod = lys_mainmodule(sub);
            name_len = strlen(sub->name) + (module == mod ? 0 : strlen(mod->prefix)+1);
            if (name_len > max_name_len) {
                max_name_len = name_len;
            }
        }
    }

    return max_name_len;
}

static void
tree_print_type(struct lyout *out, const struct lys_type *type)
{
    if ((type->base == LY_TYPE_LEAFREF) && !type->der->module) {
        ly_print(out, "-> %s", type->info.lref.path);
    } else if (type->module_name) {
        ly_print(out, "%s:%s", type->module_name, type->der->name);
    } else {
        ly_print(out, "%s", type->der->name);
    }
}

static void
tree_print_features(struct lyout *out, const struct lys_feature **features, uint8_t features_size)
{
    int i;

    if (!features_size) {
        return;
    }

    ly_print(out, " {");
    for (i = 0; i < features_size; i++) {
        if (i > 0) {
            ly_print(out, ",");
        }
        ly_print(out, "%s", features[i]->name);
    }
    ly_print(out, "}?");
}

static void
tree_print_inout(struct lyout *out, const struct lys_module *module, int level, char *indent,
                 const struct lys_node *node, int spec_config)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node *sub;

    assert(spec_config);

    ly_print(out, "%s+--%s %s\n", indent, (spec_config == 1 ? "-w" : "ro"), (spec_config == 1 ? "input" : "output"));

    level++;
    new_indent = create_indent(level, indent, node, 0);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(node->child, sub) {
        tree_print_snode(out, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                         spec_config);
    }

    free(new_indent);
}

static void
tree_print_container(struct lyout *out, const struct lys_module *module, int level, char *indent,
                     const struct lys_node *node, int spec_config)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node_container *cont = (struct lys_node_container *)node;
    struct lys_node *sub;
    struct lys_module *nodemod;

    assert(spec_config >= 0 && spec_config <= 2);

    ly_print(out, "%s%s--", indent,
            (cont->flags & LYS_STATUS_DEPRC ? "x" : (cont->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (cont->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    nodemod = lys_mainmodule(node);
    if (module != nodemod) {
        ly_print(out, "%s:", nodemod->prefix);
    }

    ly_print(out, "%s%s", cont->name, (cont->presence ? "!" : ""));

    tree_print_features(out, (const struct lys_feature **)cont->features, cont->features_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(cont->child, sub) {
        tree_print_snode(out, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                         spec_config);
    }

    free(new_indent);
}

static void
tree_print_choice(struct lyout *out, const struct lys_module *module, int level, char *indent,
                  const struct lys_node *node, int spec_config)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;
    struct lys_node *sub;
    struct lys_module *nodemod;

    assert(spec_config >= 0 && spec_config <= 2);

    ly_print(out, "%s%s--", indent,
            (choice->flags & LYS_STATUS_DEPRC ? "x" : (choice->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (choice->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    ly_print(out, "(");

    nodemod = lys_mainmodule(node);
    if (module != nodemod) {
        ly_print(out, "%s:", nodemod->prefix);
    }

    ly_print(out, "%s)%s", choice->name, (choice->flags & LYS_MAND_TRUE ? "" : "?"));

    if (choice->dflt != NULL) {
        ly_print(out, " <%s>", choice->dflt->name);
    }

    tree_print_features(out, (const struct lys_feature **)choice->features, choice->features_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(choice->child, sub) {
        tree_print_choice_content(out, module, level, new_indent, max_child_len, sub,
                                  LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML,
                                  spec_config);
    }

    free(new_indent);
}

static void
tree_print_case(struct lyout *out, const struct lys_module *module, int level, char *indent, unsigned int max_name_len,
                const struct lys_node *node, int shorthand, int spec_config)
{
    char *new_indent;
    struct lys_node_case *cas = (struct lys_node_case *)node;
    struct lys_node *sub;
    struct lys_module *nodemod;

    ly_print(out, "%s%s--:(", indent,
            (cas->flags & LYS_STATUS_DEPRC ? "x" : (cas->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    nodemod = lys_mainmodule(node);
    if (module != nodemod) {
        ly_print(out, "%s:", nodemod->prefix);
    }

    ly_print(out, "%s)", cas->name);

    tree_print_features(out, (const struct lys_feature **)cas->features, cas->features_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, shorthand);

    if (shorthand) {
        tree_print_snode(out, module, level, new_indent, max_name_len, node,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                         spec_config);
    } else {
        LY_TREE_FOR(node->child, sub) {
            tree_print_snode(out, module, level, new_indent, max_name_len, sub,
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES,
                             spec_config);
        }
    }

    free(new_indent);
}

static void
tree_print_anyxml(struct lyout *out, const struct lys_module *module, char *indent, unsigned int max_name_len,
                  const struct lys_node *node, int spec_config)
{
    uint8_t prefix_len;
    struct lys_module *nodemod;
    struct lys_node_anyxml *anyxml = (struct lys_node_anyxml *)node;

    assert(spec_config >= 0 && spec_config <= 2);

    ly_print(out, "%s%s--", indent,
            (anyxml->flags & LYS_STATUS_DEPRC ? "x" : (anyxml->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (anyxml->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    prefix_len = 0;
    nodemod = lys_mainmodule(node);
    if (module != nodemod) {
        ly_print(out, "%s:", nodemod->prefix);
        prefix_len = strlen(nodemod->prefix)+1;
    }

    ly_print(out, "%s%s%*sanyxml", anyxml->name, (anyxml->flags & LYS_MAND_TRUE ? " " : "?"),
            3 + (int)((max_name_len - strlen(anyxml->name)) - prefix_len), "   ");

    tree_print_features(out, (const struct lys_feature **)anyxml->features, anyxml->features_size);

    ly_print(out, "\n");
}

static void
tree_print_leaf(struct lyout *out, const struct lys_module *module, char *indent, unsigned int max_name_len,
                const struct lys_node *node, int spec_config)
{
    uint8_t prefix_len;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;
    struct lys_node *parent;
    struct lys_node_list *list;
    struct lys_module *nodemod;
    int i, is_key = 0;

    assert(spec_config >= 0 && spec_config <= 2);

    /* get know if the leaf is a key in a list, in that case it is
     * mandatory by default */
    for (parent = leaf->parent; parent && parent->nodetype == LYS_USES; parent = parent->parent);
    if (parent && parent->nodetype == LYS_LIST) {
        list = (struct lys_node_list *)parent;
        for (i = 0; i < list->keys_size; i++) {
            if (list->keys[i] == leaf) {
                is_key = 1;
                break;
            }
        }
    }

    ly_print(out, "%s%s--", indent,
            (leaf->flags & LYS_STATUS_DEPRC ? "x" : (leaf->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (leaf->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    prefix_len = 0;
    nodemod = lys_mainmodule(node);
    if (module != nodemod) {
        ly_print(out, "%s:", nodemod->prefix);
        prefix_len = strlen(nodemod->prefix)+1;
    }

    ly_print(out, "%s%s%*s", leaf->name, ((leaf->flags & LYS_MAND_TRUE) || is_key ? " " : "?"),
            3 + (int)((max_name_len - strlen(leaf->name)) - prefix_len), "   ");

    tree_print_type(out, &leaf->type);

    if (leaf->dflt != NULL) {
        ly_print(out, " <%s>", leaf->dflt);
    }

    tree_print_features(out, (const struct lys_feature **)leaf->features, leaf->features_size);

    ly_print(out, "\n");
}

static void
tree_print_leaflist(struct lyout *out, const struct lys_module *module, char *indent, unsigned int max_name_len,
                    const struct lys_node *node, int spec_config)
{
    struct lys_node_leaflist *leaflist = (struct lys_node_leaflist *)node;
    struct lys_module *nodemod;

    assert(spec_config >= 0 && spec_config <= 2);

    ly_print(out, "%s%s--", indent,
            (leaflist->flags & LYS_STATUS_DEPRC ? "x" : (leaflist->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (leaflist->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    nodemod = lys_mainmodule(node);
    if (module != nodemod) {
        ly_print(out, "%s:", nodemod->prefix);
    }

    ly_print(out, "%s*%*s", leaflist->name, 3 + (int)(max_name_len - strlen(leaflist->name)), "   ");

    tree_print_type(out, &leaflist->type);

    tree_print_features(out, (const struct lys_feature **)leaflist->features, leaflist->features_size);

    ly_print(out, "\n");
}

static void
tree_print_list(struct lyout *out, const struct lys_module *module, int level, char *indent,
                const struct lys_node *node, int spec_config)
{
    int i;
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;
    struct lys_module *nodemod;

    ly_print(out, "%s%s--", indent,
            (list->flags & LYS_STATUS_DEPRC ? "x" : (list->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (list->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    nodemod = lys_mainmodule(node);
    if (module != nodemod) {
        ly_print(out, "%s:", nodemod->prefix);
    }

    ly_print(out, "%s*", list->name);

    for (i = 0; i < list->keys_size; i++) {
        if (i == 0) {
            ly_print(out, " [");
        }
        ly_print(out, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? " " : "]");
    }

    tree_print_features(out, (const struct lys_feature **)list->features, list->features_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(node->child, sub) {
        tree_print_snode(out, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ANYXML,
                         spec_config);
    }

    free(new_indent);
}

static void
tree_print_uses(struct lyout *out, const struct lys_module *module, int level, char *indent, unsigned int max_name_len,
                const struct lys_node *node, int spec_config)
{
    struct lys_node *child;
    struct lys_node_uses *uses = (struct lys_node_uses *)node;

    LY_TREE_FOR(uses->child, child) {
        tree_print_snode(out, module, level, indent, max_name_len, child,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ANYXML,
                         spec_config);
    }
}

static void
tree_print_rpc(struct lyout *out, const struct lys_module *module, int level, char *indent,
               const struct lys_node *node)
{
    char *new_indent;
    struct lys_node *child;
    struct lys_node_rpc *rpc = (struct lys_node_rpc *)node;

    if (lys_is_disabled(node, 0)) {
        return;
    }

    ly_print(out, "%s%s---x %s", indent,
            (rpc->flags & LYS_STATUS_DEPRC ? "x" : (rpc->flags & LYS_STATUS_OBSLT ? "o" : "+")), rpc->name);

    tree_print_features(out, (const struct lys_feature **)rpc->features, rpc->features_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0);

    LY_TREE_FOR(rpc->child, child) {
        if (child->nodetype == LYS_INPUT) {
            tree_print_inout(out, module, level, new_indent, child, 1);
        } else if (child->nodetype == LYS_OUTPUT) {
            tree_print_inout(out, module, level, new_indent, child, 2);
        }
    }

    free(new_indent);
}

static void
tree_print_notif(struct lyout *out, const struct lys_module *module, int level, char *indent,
                 const struct lys_node *node)
{
    unsigned int max_child_len;
    char *new_indent;
    struct lys_node *child;
    struct lys_node_notif *notif = (struct lys_node_notif *)node;

    if (lys_is_disabled(node, 0)) {
        return;
    }

    ly_print(out, "%s%s---n %s", indent,
            (notif->flags & LYS_STATUS_DEPRC ? "x" : (notif->flags & LYS_STATUS_OBSLT ? "o" : "+")),
            notif->name);

    tree_print_features(out, (const struct lys_feature **)notif->features, notif->features_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(notif->child, child) {
        tree_print_snode(out, module, level, new_indent, max_child_len, child,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_USES, 2);
    }

    free(new_indent);
}

static void
tree_print_choice_content(struct lyout *out, const struct lys_module *module, int level, char *indent,
                          unsigned int max_name_len, const struct lys_node *node, int mask, int spec_config)
{
    if (lys_is_disabled(node, 0)) {
        return;
    }

    if (node->nodetype & mask) {
        if (node->nodetype == LYS_CASE) {
            tree_print_case(out, module, level, indent, max_name_len, node, 0, spec_config);
        } else {
            tree_print_case(out, module, level, indent, max_name_len, node, 1, spec_config);
        }
    }
}

/* spec_config = 0 (no special config status), 1 (read-only - rpc output, notification), 2 (write-only - rpc input) */
static void
tree_print_snode(struct lyout *out, const struct lys_module *module, int level, char *indent,
                 unsigned int max_name_len, const struct lys_node *node, int mask, int spec_config)
{
    if (lys_is_disabled(node, 0)) {
        return;
    }

    switch (node->nodetype & mask) {
    case LYS_CONTAINER:
        tree_print_container(out, module, level, indent, node, spec_config);
        break;
    case LYS_CHOICE:
        tree_print_choice(out, module, level, indent, node, spec_config);
        break;
    case LYS_LEAF:
        tree_print_leaf(out, module, indent, max_name_len, node, spec_config);
        break;
    case LYS_LEAFLIST:
        tree_print_leaflist(out, module, indent, max_name_len, node, spec_config);
        break;
    case LYS_LIST:
        tree_print_list(out, module, level, indent, node, spec_config);
        break;
    case LYS_ANYXML:
        tree_print_anyxml(out, module, indent, max_name_len, node, spec_config);
        break;
    case LYS_USES:
        tree_print_uses(out, module, level, indent, max_name_len, node, spec_config);
        break;
    default:
        break;
    }
}

int
tree_print_model(struct lyout *out, const struct lys_module *module)
{
    struct lys_node *node;
    unsigned int max_child_len;
    int level = 1, have_rpcs = 0, have_notifs = 0;
    char *indent = malloc((level * 4 + 1) * sizeof (char));

    if (!indent) {
        LOGMEM;
        return EXIT_FAILURE;
    }
    strcpy(indent, "   ");

    if (module->type) {
        ly_print(out, "submodule: %s (belongs-to %s)\n", module->name,
                 ((struct lys_submodule *)module)->belongsto->name);
    } else {
        ly_print(out, "module: %s\n", module->name);
    }

    /* module */
    max_child_len = get_max_name_len(module, module->data);
    level++;

    LY_TREE_FOR(module->data, node) {
        switch(node->nodetype) {
        case LYS_RPC:
            if (!lys_is_disabled(node, 0)) {
                have_rpcs++;
            }
            break;
        case LYS_NOTIF:
            if (!lys_is_disabled(node, 0)) {
                have_notifs++;
            }
            break;
        default:
            tree_print_snode(out, module, level, indent, max_child_len, node,
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST
                             | LYS_ANYXML | LYS_USES, 0);
            break;
        }
    }

    /* rpc */
    if (have_rpcs) {
        ly_print(out, "rpcs:\n");
        LY_TREE_FOR(module->data, node) {
            if (!have_rpcs) {
                break;
            }
            if (node->nodetype == LYS_RPC) {
                tree_print_rpc(out, module, level, indent, node);
                have_rpcs--;
            }
        }
    }

    /* notification */
    if (have_notifs) {
        ly_print(out, "notifications:\n");
        LY_TREE_FOR(module->data, node) {
            if (!have_notifs) {
                break;
            }
            if (node->nodetype == LYS_NOTIF) {
                tree_print_notif(out, module, level, indent, node);
                have_notifs--;
            }
        }
    }

    free(indent);
    return EXIT_SUCCESS;
}
