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

#include "../common.h"
#include "../tree.h"

static void tree_print_mnode_choice(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
							 int mask);
static void tree_print_mnode(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
                             int mask);

static int uses_has_valid_child(struct ly_mnode *mnode) {
	struct ly_mnode *cur;

	if (mnode == NULL) {
		return 0;
	}

	for (cur = mnode; cur != NULL; cur = cur->next) {
		if (cur->nodetype & (LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML | LY_NODE_CHOICE)) {
			return 1;
		}

		if (cur->nodetype == LY_NODE_USES && uses_has_valid_child(cur->child)) {
			return 1;
		}
	}

	return 0;
}

static char *create_indent(int level, const char *old_indent, const struct ly_mnode *mnode, int shorthand)
{
	struct ly_mnode *cur;
	int next_is_case = 0, sibling_in_submodule = 0, sibling_outside_uses = 0;
	int is_case = 0;
	char *new_indent = malloc((level*4+1)*sizeof(char));

	strcpy(new_indent, old_indent);

	/* this is the indent of a case (standard or shorthand) */
	if (mnode->nodetype == LY_NODE_CASE || shorthand) {
		is_case = 1;
	}

	/* this is the direct child of a case */
	if (!is_case && mnode->parent != NULL && mnode->parent->nodetype & (LY_NODE_CASE | LY_NODE_CHOICE)) {
		/* it is not the only child */
		if (mnode->next != NULL && mnode->next->parent != NULL && mnode->next->parent->nodetype == LY_NODE_CHOICE) {
			next_is_case = 1;
		}
	}

	/* we are in a submodule and we don't have a parent */
	if (mnode->module->type == 1 && mnode->parent == NULL) {
		struct ly_submodule *submod = (struct ly_submodule *)mnode->module;

		/* this submodule is not the last submodule of the module */
		if (strcmp(mnode->module->name, submod->belongsto->inc[submod->belongsto->inc_size-1].submodule->name) != 0) {
			sibling_in_submodule = 1;
		}
	}

	/* there is no standard sibling and this is a uses */
	if (mnode->next == NULL && mnode->parent != NULL && mnode->parent->nodetype == LY_NODE_USES) {
		/* there is a sibling, it contains a valid node */
		for (cur = mnode->parent->next; cur != NULL; cur = cur->next) {
			if (cur->nodetype & (LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML | LY_NODE_CHOICE)) {
				sibling_outside_uses = 1;
				break;
			}

			/* we must check uses recursively for a valid node (it can contain only typedef, for instance) */
			if (cur->nodetype == LY_NODE_USES && uses_has_valid_child(cur)) {
				sibling_outside_uses = 1;
				break;
			}
		}
	}

	if ((mnode->next && !next_is_case) || sibling_in_submodule || sibling_outside_uses) {
		strcat(new_indent, "|  ");
	} else {
		strcat(new_indent, "   ");
	}

	return new_indent;
}

static unsigned int get_max_name_len(struct ly_mnode *mnode)
{
	struct ly_mnode *sub;
	unsigned int max_name_len = 0, uses_max_name_len;

	LY_TREE_FOR(mnode, sub) {
		if (sub->nodetype == LY_NODE_USES) {
			uses_max_name_len = get_max_name_len(sub->child);
			if (uses_max_name_len > max_name_len) {
				max_name_len = uses_max_name_len;
			}
		} else if (sub->nodetype & (LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF |
				   LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML)) {
			if (strlen(sub->name) > max_name_len) {
				max_name_len = strlen(sub->name);
			}
		}
	}

	return max_name_len;
}

static void tree_print_type(FILE *f, struct ly_type *type)
{
	if (type->base == LY_TYPE_LEAFREF) {
		fprintf(f, "-> %s", type->info.lref.path);
	} else if (type->prefix) {
		fprintf(f, "%s:%s", type->prefix, type->der->name);
	} else {
		fprintf(f, "%s", type->der->name);
	}
}

static void tree_print_container(FILE *f, int level, char *indent, struct ly_mnode *mnode)
{
	unsigned int max_child_len;
	char *new_indent;
	struct ly_mnode_container *cont = (struct ly_mnode_container *)mnode;
	struct ly_mnode *sub;

	fprintf(f, "%s+--%s %s%s\n", indent, (cont->flags & LY_NODE_CONFIG_W ? "rw" : "ro"), cont->name, (cont->presence ? "!" : ""));

	level++;
	new_indent = create_indent(level, indent, mnode, 0);

	max_child_len = get_max_name_len(mnode->child);

	LY_TREE_FOR(cont->child, sub) {
		tree_print_mnode(f, level, new_indent, max_child_len, sub, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML |
						 LY_NODE_USES);
	}

	free(new_indent);
}

static void tree_print_choice(FILE *f, int level, char *indent, struct ly_mnode *mnode)
{
	unsigned int max_child_len;
	char *new_indent;
	struct ly_mnode_choice *choice = (struct ly_mnode_choice *)mnode;
	struct ly_mnode *sub;

	fprintf(f, "%s+--%s (%s)%s", indent, (choice->flags & LY_NODE_CONFIG_W ? "rw" : "ro"), choice->name, (choice->flags & LY_NODE_MAND_TRUE ? "" : "?"));
	if (choice->dflt != NULL) {
		fprintf(f, " <%s>", choice->dflt->name);
	}
	fprintf(f, "\n");

	level++;
	new_indent = create_indent(level, indent, mnode, 0);

	max_child_len = get_max_name_len(mnode->child);

	LY_TREE_FOR(choice->child, sub) {
		tree_print_mnode_choice(f, level, new_indent, max_child_len, sub,
								LY_NODE_CASE | LY_NODE_CONTAINER |
								LY_NODE_LEAF | LY_NODE_LEAFLIST |
								LY_NODE_LIST | LY_NODE_ANYXML);
	}

	free(new_indent);
}

static void tree_print_case(FILE *f, int level, char *indent, unsigned int max_name_len, int shorthand, struct ly_mnode *mnode)
{
	char *new_indent;
	//struct ly_mnode_case *cas = (struct ly_mnode_case *)mnode;
	struct ly_mnode *sub;

	fprintf(f, "%s+--:(%s)\n", indent, mnode->name);

	level++;
	new_indent = create_indent(level, indent, mnode, shorthand);

	if (shorthand) {
		tree_print_mnode(f, level, new_indent, max_name_len, mnode,
						 LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_ANYXML | LY_NODE_USES);
	} else {
		LY_TREE_FOR(mnode->child, sub) {
			tree_print_mnode(f, level, new_indent, max_name_len, sub,
							LY_NODE_CHOICE | LY_NODE_CONTAINER |
							LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
							LY_NODE_ANYXML | LY_NODE_USES);
		}
	}

	free(new_indent);
}

static void tree_print_anyxml(FILE *f, char *indent, struct ly_mnode *mnode)
{
	struct ly_mnode_anyxml *anyxml = (struct ly_mnode_anyxml *)mnode;

	fprintf(f, "%s+--%s %s\n", indent, (anyxml->flags & LY_NODE_CONFIG_W ? "rw" : "ro"),
			anyxml->name);
}

static void tree_print_leaf(FILE *f, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	struct ly_mnode_leaf *leaf = (struct ly_mnode_leaf *)mnode;
	struct ly_mnode_list *list;
	int i, is_key = 0;

	if (leaf->parent->nodetype == LY_NODE_LIST) {
		list = (struct ly_mnode_list *)leaf->parent;
		for (i = 0; i < list->keys_size; i++) {
			if (strcmp(list->keys[i]->name, leaf->name) == 0) {
				is_key = 1;
				break;
			}
		}
	}

	fprintf(f, "%s+--%s %s%s%*s", indent, (leaf->flags & LY_NODE_CONFIG_W ? "rw" : "ro"),
			leaf->name, (leaf->flags & LY_NODE_MAND_TRUE || is_key ? " " : "?"), 3+(int)(max_name_len-strlen(leaf->name)), "   ");
	tree_print_type(f, &leaf->type);
	if (leaf->dflt != NULL) {
		fprintf(f, " <%s>", leaf->dflt);
	}
	fprintf(f, "\n");
}

static void tree_print_leaflist(FILE *f, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	struct ly_mnode_leaflist *leaflist = (struct ly_mnode_leaflist *)mnode;

	fprintf(f, "%s+--%s %s*%*s", indent, (leaflist->flags & LY_NODE_CONFIG_W ? "rw" : "ro"),
			leaflist->name, 3+(int)(max_name_len-strlen(leaflist->name)), "   ");
	tree_print_type(f, &leaflist->type);
	fprintf(f, "\n");
}

static void tree_print_list(FILE *f, int level, char *indent, struct ly_mnode *mnode)
{
	int i;
	unsigned int max_child_len;
	char *new_indent;
	struct ly_mnode *sub;
	struct ly_mnode_list *list = (struct ly_mnode_list *)mnode;

	fprintf(f, "%s+--%s %s*", indent, (mnode->flags & LY_NODE_CONFIG_W ? "rw" : "ro"), mnode->name);

	for (i = 0; i < list->keys_size; i++) {
		if (i == 0) {
			fprintf(f, " [");
		}
		fprintf(f, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? "," : "]");
	}
	fprintf(f, "\n");

	level++;
	new_indent = create_indent(level, indent, mnode, 0);

	max_child_len = get_max_name_len(mnode->child);

	LY_TREE_FOR(mnode->child, sub) {
		tree_print_mnode(f, level, new_indent, max_child_len, sub, LY_NODE_CHOICE | LY_NODE_CONTAINER |
		                 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_USES);
	}

	free(new_indent);
}

static void tree_print_uses(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	struct ly_mnode *node;
	struct ly_mnode_uses *uses = (struct ly_mnode_uses *)mnode;;

	LY_TREE_FOR(uses->child, node) {
		tree_print_mnode(f, level, indent, max_name_len, node, LY_NODE_CHOICE | LY_NODE_CONTAINER |
		                 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_USES | LY_NODE_ANYXML);
	}
}

static void tree_print_mnode_choice(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
							 int mask)
{
	if (mnode->nodetype & mask) {
		if (mnode->nodetype == LY_NODE_CASE) {
			tree_print_case(f, level, indent, max_name_len, 0, mnode);
		} else {
			tree_print_case(f, level, indent, max_name_len, 1, mnode);
		}
	}
}

static void tree_print_mnode(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
							 int mask)
{
	switch(mnode->nodetype & mask) {
	case LY_NODE_CONTAINER:
		tree_print_container(f, level, indent, mnode);
		break;
	case LY_NODE_CHOICE:
		tree_print_choice(f, level, indent, mnode);
		break;
	case LY_NODE_LEAF:
		tree_print_leaf(f, indent, max_name_len, mnode);
		break;
	case LY_NODE_LEAFLIST:
		tree_print_leaflist(f, indent, max_name_len, mnode);
		break;
	case LY_NODE_LIST:
		tree_print_list(f, level, indent, mnode);
		break;
	case LY_NODE_ANYXML:
		tree_print_anyxml(f, indent, mnode);
		break;
	case LY_NODE_USES:
		tree_print_uses(f, level, indent, max_name_len, mnode);
		break;
	default: break;
	}
}

int tree_print_model(FILE *f, struct ly_module *module)
{
	struct ly_mnode *mnode;
	unsigned int max_child_len;
	int level = 1, i;
	char *indent = malloc((level*4+1)*sizeof(char));
	strcpy(indent, "   ");

	fprintf(f, "module: %s\n", module->name);

	/* included submodules */
	for (i = 0; i < module->inc_size; ++i) {
		max_child_len = get_max_name_len(module->inc[i].submodule->data);

		LY_TREE_FOR(module->inc[i].submodule->data, mnode) {
			tree_print_mnode(f, level, indent, max_child_len, mnode, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML |
						 LY_NODE_USES);
		}
	}

	/* module */
	max_child_len = get_max_name_len(module->data);
	level++;

	/*if (module->version) {
		fprintf(f, "%*syang-version \"%s\";\n", LEVEL, INDENT, module->version == 1 ? "1.0" : "1.1");
	}*/

	LY_TREE_FOR(module->data, mnode) {
		tree_print_mnode(f, level, indent, max_child_len, mnode, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_ANYXML |
						 LY_NODE_USES);
	}

	/* TODO RPCs */

	/* TODO notifications */

	free(indent);
	return EXIT_SUCCESS;
#undef LEVEL
}
