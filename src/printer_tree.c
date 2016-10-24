/**
 * @file printer/tree.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief TREE printer for libyang data model structure
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

/* spec_config = 0 (no special config status), 1 (read-only - rpc output, notification), 2 (write-only - rpc input) */
static void tree_print_choice_content(struct lyout *out, const struct lys_module* module, int level, uint64_t indent,
                                      unsigned int max_name_len, const struct lys_node *node, int mask,
                                      int spec_config);
static void tree_print_snode(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                             unsigned int max_name_len, const struct lys_node *node, int mask, int spec_config);

static void
print_indent(struct lyout *out, uint64_t indent, int level)
{
    int i;

    for (i = 1; i < level; ++i) {
        if (indent & (1 << i)) {
            ly_print(out, "|  ");
        } else {
            ly_print(out, "   ");
        }
    }
}

static int
sibling_is_valid_child(const struct lys_node *node, int including, const struct lys_module *module)
{
    struct lys_node *cur;

    if (!node) {
        return 0;
    }

    /* has a following printed child */
    LY_TREE_FOR((struct lys_node *)(including ? node : node->next), cur) {
        if (module != lys_node_module(cur)) {
            continue;
        }

        if (!lys_is_disabled(cur, 0)) {
            if (cur->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_CHOICE |
                    LYS_INPUT | LYS_OUTPUT | LYS_CASE)) {
                return 1;
            }
            if ((cur->nodetype == LYS_USES) && sibling_is_valid_child(cur->child, 1, module)) {
                return 1;
            }
        }
    }

    /* if in uses, the following printed child can actually be in the parent node :-/ */
    if (lys_parent(node) && (lys_parent(node)->nodetype == LYS_USES)) {
        return sibling_is_valid_child(lys_parent(node), 0, module);
    }

    return 0;
}

uint64_t
create_indent(int level, uint64_t old_indent, const struct lys_node *node, int shorthand, const struct lys_module *module)
{
    uint64_t new_indent;
    int next_is_case = 0, is_case = 0, has_next = 0;

    if (level > 64) {
        LOGINT;
        return 0;
    }

    new_indent = old_indent;

    /* this is the indent of a case (standard or shorthand) */
    if ((node->nodetype == LYS_CASE) || shorthand) {
        is_case = 1;
    }

    /* this is the direct child of a case */
    if (!is_case && lys_parent(node) && (lys_parent(node)->nodetype & (LYS_CASE | LYS_CHOICE))) {
        /* it is not the only child */
        if (node->next && lys_parent(node->next) && (lys_parent(node->next)->nodetype == LYS_CHOICE)) {
            next_is_case = 1;
        }
    }

    /* next is a node that will actually be printed */
    has_next = sibling_is_valid_child(node, 0, lys_main_module(module));

    if (has_next && !next_is_case) {
        new_indent |= 1 << (level - 1);
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
        if (module->type && (sub->module != module)) {
            /* when printing submodule, we are only concerned with its own data (they are in the module data) */
            continue;
        }

        if (sub->nodetype == LYS_USES) {
            uses_max_name_len = get_max_name_len(module, sub->child);
            if (uses_max_name_len > max_name_len) {
                max_name_len = uses_max_name_len;
            }
        } else if (sub->nodetype &
                (LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_CASE)) {
            mod = lys_node_module(sub);
            name_len = strlen(sub->name) + (module == mod ? 0 : strlen(mod->name) + 1);
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
tree_print_features(struct lyout *out, const struct lys_module *module,
                    struct lys_iffeature *iffeature, uint8_t iffeature_size)
{
    int i;

    if (!iffeature_size) {
        return;
    }

    ly_print(out, " {");
    for (i = 0; i < iffeature_size; i++) {
        if (i > 0) {
            ly_print(out, ",");
        }
        ly_print_iffeature(out, module, &iffeature[i]);
    }
    ly_print(out, "}?");
}

static void
tree_print_inout(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                 const struct lys_node *node, int spec_config)
{
    unsigned int max_child_len;
    uint64_t new_indent;
    struct lys_node *sub;

    assert(spec_config);

    if (node->flags & LYS_IMPLICIT) {
        /* implicit input/output which is not a part of the schema */
        return;
    }

    print_indent(out, indent, level);
    ly_print(out, "+--%s %s\n", (spec_config == 1 ? "-w" : "ro"), (spec_config == 1 ? "input" : "output"));

    level++;
    new_indent = create_indent(level, indent, node, 0, module);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(node->child, sub) {
        /* submodule, foreign augments */
        if (module->type && (sub->parent != node) && (sub->module != module)) {
            continue;
        }
        tree_print_snode(out, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES,
                         spec_config);
    }
}

static void
tree_print_container(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                     const struct lys_node *node, int spec_config)
{
    unsigned int max_child_len;
    uint64_t new_indent;
    struct lys_node_container *cont = (struct lys_node_container *)node;
    struct lys_node *sub;
    struct lys_module *nodemod;

    assert(spec_config >= 0 && spec_config <= 2);

    print_indent(out, indent, level);
    ly_print(out, "%s--", (cont->flags & LYS_STATUS_DEPRC ? "x" : (cont->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (cont->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    nodemod = lys_node_module(node);
    if (lys_main_module(module) != nodemod) {
        ly_print(out, "%s:", nodemod->name);
    }

    ly_print(out, "%s%s", cont->name, (cont->presence ? "!" : ""));

    tree_print_features(out, module, cont->iffeature, cont->iffeature_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0, module);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(node->child, sub) {
        /* submodule, foreign augments */
        if (module->type && (sub->parent != node) && (sub->module != module)) {
            continue;
        }
        tree_print_snode(out, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES | LYS_ACTION,
                         spec_config);
    }
}

static void
tree_print_choice(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                  const struct lys_node *node, int spec_config)
{
    unsigned int max_child_len;
    uint64_t new_indent;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;
    struct lys_node *sub;
    struct lys_module *nodemod;

    assert(spec_config >= 0 && spec_config <= 2);

    print_indent(out, indent, level);
    ly_print(out, "%s--", (choice->flags & LYS_STATUS_DEPRC ? "x" : (choice->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (choice->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    ly_print(out, "(");

    nodemod = lys_node_module(node);
    if (lys_main_module(module) != nodemod) {
        ly_print(out, "%s:", nodemod->name);
    }

    ly_print(out, "%s)%s", choice->name, (choice->flags & LYS_MAND_TRUE ? "" : "?"));

    if (choice->dflt != NULL) {
        ly_print(out, " <%s>", choice->dflt->name);
    }

    tree_print_features(out, module, choice->iffeature, choice->iffeature_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0, module);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(node->child, sub) {
        /* submodule, foreign augments */
        if (module->type && (sub->parent != node) && (sub->module != module)) {
            continue;
        }
        tree_print_choice_content(out, module, level, new_indent, max_child_len, sub,
                                  LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA,
                                  spec_config);
    }
}

static void
tree_print_case(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                unsigned int max_name_len, const struct lys_node *node, int shorthand, int spec_config)
{
    uint64_t new_indent;
    struct lys_node_case *cas = (struct lys_node_case *)node;
    struct lys_node *sub;
    struct lys_module *nodemod;

    print_indent(out, indent, level);
    ly_print(out, "%s--:(", (cas->flags & LYS_STATUS_DEPRC ? "x" : (cas->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    nodemod = lys_node_module(node);
    if (lys_main_module(module) != nodemod) {
        ly_print(out, "%s:", nodemod->name);
    }

    ly_print(out, "%s)", cas->name);

    tree_print_features(out, module, cas->iffeature, cas->iffeature_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, shorthand, module);

    if (shorthand) {
        tree_print_snode(out, module, level, new_indent, max_name_len, node,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES,
                         spec_config);
    } else {
        LY_TREE_FOR(node->child, sub) {
            /* submodule, foreign augments */
            if (module->type && (sub->parent != node) && (sub->module != module)) {
                continue;
            }
            tree_print_snode(out, module, level, new_indent, max_name_len, sub,
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES,
                             spec_config);
        }
    }
}

static void
tree_print_anydata(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                   unsigned int max_name_len, const struct lys_node *node, int spec_config)
{
    uint8_t prefix_len;
    struct lys_module *nodemod;
    struct lys_node_anydata *any = (struct lys_node_anydata *)node;

    assert(spec_config >= 0 && spec_config <= 2);

    print_indent(out, indent, level);
    ly_print(out, "%s--", (any->flags & LYS_STATUS_DEPRC ? "x" : (any->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (any->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    prefix_len = 0;
    nodemod = lys_node_module(node);
    if (lys_main_module(module) != nodemod) {
        ly_print(out, "%s:", nodemod->name);
        prefix_len = strlen(nodemod->name)+1;
    }

    ly_print(out, "%s%s%*s%s", any->name, (any->flags & LYS_MAND_TRUE ? " " : "?"),
            3 + (int)((max_name_len - strlen(any->name)) - prefix_len), "   ",
            any->nodetype == LYS_ANYXML ? "anyxml" : "anydata");

    tree_print_features(out, module, any->iffeature, any->iffeature_size);

    ly_print(out, "\n");
}

static void
tree_print_leaf(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                unsigned int max_name_len, const struct lys_node *node, int spec_config)
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
    for (parent = lys_parent(node); parent && parent->nodetype == LYS_USES; parent = lys_parent(parent));
    if (parent && parent->nodetype == LYS_LIST) {
        list = (struct lys_node_list *)parent;
        for (i = 0; i < list->keys_size; i++) {
            if (list->keys[i] == leaf) {
                is_key = 1;
                break;
            }
        }
    }

    print_indent(out, indent, level);
    ly_print(out, "%s--", (leaf->flags & LYS_STATUS_DEPRC ? "x" : (leaf->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (leaf->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    prefix_len = 0;
    nodemod = lys_node_module(node);
    if (lys_main_module(module) != nodemod) {
        ly_print(out, "%s:", nodemod->name);
        prefix_len = strlen(nodemod->name)+1;
    }

    ly_print(out, "%s%s%*s", leaf->name, ((leaf->flags & LYS_MAND_TRUE) || is_key ? " " : "?"),
            3 + (int)((max_name_len - strlen(leaf->name)) - prefix_len), "   ");

    tree_print_type(out, &leaf->type);

    if (leaf->dflt) {
        ly_print(out, " <%s>", leaf->dflt);
    }

    tree_print_features(out, module, leaf->iffeature, leaf->iffeature_size);

    ly_print(out, "\n");
}

static void
tree_print_leaflist(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                    unsigned int max_name_len, const struct lys_node *node, int spec_config)
{
    struct lys_node_leaflist *leaflist = (struct lys_node_leaflist *)node;
    struct lys_module *nodemod;

    assert(spec_config >= 0 && spec_config <= 2);

    print_indent(out, indent, level);
    ly_print(out, "%s--", (leaflist->flags & LYS_STATUS_DEPRC ? "x" : (leaflist->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (leaflist->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    nodemod = lys_node_module(node);
    if (lys_main_module(module) != nodemod) {
        ly_print(out, "%s:", nodemod->name);
    }

    ly_print(out, "%s*%*s", leaflist->name, 3 + (int)(max_name_len - strlen(leaflist->name)), "   ");

    tree_print_type(out, &leaflist->type);

    tree_print_features(out, module, leaflist->iffeature, leaflist->iffeature_size);

    ly_print(out, "\n");
}

static void
tree_print_list(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                const struct lys_node *node, int spec_config)
{
    int i;
    unsigned int max_child_len;
    uint64_t new_indent;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;
    struct lys_module *nodemod;

    print_indent(out, indent, level);
    ly_print(out, "%s--", (list->flags & LYS_STATUS_DEPRC ? "x" : (list->flags & LYS_STATUS_OBSLT ? "o" : "+")));

    if (spec_config == 0) {
        ly_print(out, "%s ", (list->flags & LYS_CONFIG_W ? "rw" : "ro"));
    } else if (spec_config == 1) {
        ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ly_print(out, "ro ");
    }

    nodemod = lys_node_module(node);
    if (lys_main_module(module) != nodemod) {
        ly_print(out, "%s:", nodemod->name);
    }

    ly_print(out, "%s*", list->name);

    for (i = 0; i < list->keys_size; i++) {
        if (i == 0) {
            ly_print(out, " [");
        }
        ly_print(out, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? " " : "]");
    }

    tree_print_features(out, module, list->iffeature, list->iffeature_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0, module);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(node->child, sub) {
        /* submodule, foreign augments */
        if (module->type && (sub->parent != node) && (sub->module != module)) {
            continue;
        }
        tree_print_snode(out, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ANYDATA | LYS_ACTION,
                         spec_config);
    }
}

static void
tree_print_uses(struct lyout *out, const struct lys_module *module, int level, uint64_t indent, unsigned int max_name_len,
                const struct lys_node *node, int spec_config)
{
    struct lys_node *sub;

    LY_TREE_FOR(node->child, sub) {
        tree_print_snode(out, module, level, indent, max_name_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ANYDATA,
                         spec_config);
    }
}

static void
tree_print_rpc_action(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                      const struct lys_node *node)
{
    uint64_t new_indent;
    struct lys_node *sub;
    struct lys_node_rpc_action *rpc = (struct lys_node_rpc_action *)node;

    if (lys_is_disabled(node, 0)) {
        return;
    }

    print_indent(out, indent, level);
    ly_print(out, "%s---x %s", (rpc->flags & LYS_STATUS_DEPRC ? "x" : (rpc->flags & LYS_STATUS_OBSLT ? "o" : "+")), rpc->name);

    tree_print_features(out, module, rpc->iffeature, rpc->iffeature_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0, module);

    LY_TREE_FOR(node->child, sub) {
        /* submodule, foreign augments */
        if (module->type && (sub->parent != node) && (sub->module != module)) {
            continue;
        }
        if (sub->nodetype == LYS_INPUT) {
            tree_print_inout(out, module, level, new_indent, sub, 1);
        } else if (sub->nodetype == LYS_OUTPUT) {
            tree_print_inout(out, module, level, new_indent, sub, 2);
        }
    }
}

static void
tree_print_notif(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                 const struct lys_node *node)
{
    unsigned int max_child_len;
    uint64_t new_indent;
    struct lys_node *sub;
    struct lys_node_notif *notif = (struct lys_node_notif *)node;

    if (lys_is_disabled(node, 0)) {
        return;
    }

    print_indent(out, indent, level);
    ly_print(out, "%s---n %s", (notif->flags & LYS_STATUS_DEPRC ? "x" : (notif->flags & LYS_STATUS_OBSLT ? "o" : "+")),
            notif->name);

    tree_print_features(out, module, notif->iffeature, notif->iffeature_size);

    ly_print(out, "\n");

    level++;
    new_indent = create_indent(level, indent, node, 0, module);

    max_child_len = get_max_name_len(module, node->child);

    LY_TREE_FOR(node->child, sub) {
        /* submodule, foreign augments */
        if (module->type && (sub->parent != node) && (sub->module != module)) {
            continue;
        }
        tree_print_snode(out, module, level, new_indent, max_child_len, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES, 2);
    }
}

static void
tree_print_choice_content(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
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
tree_print_snode(struct lyout *out, const struct lys_module *module, int level, uint64_t indent,
                 unsigned int max_name_len, const struct lys_node *node, int mask, int spec_config)
{
    if (lys_is_disabled(node, (node->parent && node->parent->nodetype == LYS_AUGMENT) ? 1 : 0)) {
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
        tree_print_leaf(out, module, level, indent, max_name_len, node, spec_config);
        break;
    case LYS_LEAFLIST:
        tree_print_leaflist(out, module, level, indent, max_name_len, node, spec_config);
        break;
    case LYS_LIST:
        tree_print_list(out, module, level, indent, node, spec_config);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        tree_print_anydata(out, module, level, indent, max_name_len, node, spec_config);
        break;
    case LYS_USES:
        tree_print_uses(out, module, level, indent, max_name_len, node, spec_config);
        break;
    case LYS_ACTION:
        tree_print_rpc_action(out, module, level, indent, node);
        break;
    case LYS_CASE:
        /* a very special case of cases in an augment */
        tree_print_case(out, module, level, indent, max_name_len, node, 0, spec_config);
        break;
    default:
        break;
    }
}

int
tree_print_model(struct lyout *out, const struct lys_module *module)
{
    struct lys_node *node, *data;
    unsigned int max_child_len;
    int level = 1, have_rpcs = 0, have_notifs = 0;
    uint64_t indent = 0;
    int i;

    if (module->type) {
        ly_print(out, "submodule: %s (belongs-to %s)\n", module->name,
                 ((struct lys_submodule *)module)->belongsto->name);
        data = ((struct lys_submodule *)module)->belongsto->data;
    } else {
        ly_print(out, "module: %s\n", module->name);
        data = module->data;
    }

    /* module */
    max_child_len = get_max_name_len(module, data);
    level++;

    LY_TREE_FOR(data, node) {
        if (module->type && (node->module != module)) {
            /* we're printing the submodule only */
            continue;
        }

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
                             LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES, 0);
            break;
        }
    }

    /* augment */
    for (i = 0; i < module->augment_size; i++) {
        if ((module->type && (module->augment[i].target->module == module))
                || (!module->type && (lys_node_module(module->augment[i].target) == module))) {
            /* submodule, target is our submodule or module, target is in our module or any submodules */
            continue;
        }

        ly_print(out, "augment %s:\n", module->augment[i].target_name);
        LY_TREE_FOR(module->augment[i].child, node) {
            /* submodule, foreign augments */
            if (module->type && (node->module != module)) {
                continue;
            }
            tree_print_snode(out, module, level, indent, max_child_len, node,
                             LYS_CHOICE | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES, 0);
        }
    }

    /* rpc */
    if (have_rpcs) {
        ly_print(out, "rpcs:\n");
        LY_TREE_FOR(data, node) {
            if (!have_rpcs) {
                break;
            }
            if (node->nodetype == LYS_RPC) {
                tree_print_rpc_action(out, module, level, indent, node);
                have_rpcs--;
            }
        }
    }

    /* notification */
    if (have_notifs) {
        ly_print(out, "notifications:\n");
        LY_TREE_FOR(data, node) {
            if (!have_notifs) {
                break;
            }
            if (node->nodetype == LYS_NOTIF) {
                tree_print_notif(out, module, level, indent, node);
                have_notifs--;
            }
        }
    }
    ly_print_flush(out);

    return EXIT_SUCCESS;
}
