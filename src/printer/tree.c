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

#define LEVEL (2*2)
#define INDENT ""

static void tree_print_mnode(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
                             int mask);

static char *create_indent(int level, const char *old_indent, const struct ly_mnode *mnode) {
	char *new_indent = malloc((level*4+1)*sizeof(char));
	strcpy(new_indent, old_indent);
	if (mnode->next) {
		strcat(new_indent, "|  ");
	} else {
		strcat(new_indent, "   ");
	}

	return new_indent;
}

static void tree_print_text(FILE *f, const char *name,
                            const char *text)
{
	const char *s, *t;

	fprintf(f, "%*s%s\n", LEVEL, INDENT, name);

	fprintf(f, "%*s\"", LEVEL, INDENT);
	t = text;
	while((s = strchr(t, '\n'))) {
		fwrite(t, sizeof *t, s - t + 1, f);
		t = s + 1;
		fprintf(f, "%*s", LEVEL, INDENT);
	}

	fprintf(f, "%s\";\n\n", t);
}

/*
 * Covers:
 * description, reference, status
 */
static void tree_print_mnode_common(FILE *f, struct ly_mnode *mnode)
{
	if (mnode->flags & LY_NODE_STATUS_CURR) {
		fprintf(f, "%*sstatus \"current\";\n", LEVEL, INDENT);
	} else if (mnode->flags & LY_NODE_STATUS_DEPRC) {
		fprintf(f, "%*sstatus \"deprecated\";\n", LEVEL, INDENT);
	} else if (mnode->flags & LY_NODE_STATUS_OBSLT) {
		fprintf(f, "%*sstatus \"obsolete\";\n", LEVEL, INDENT);
	}

	if (mnode->dsc) {
		tree_print_text(f, "description", mnode->dsc);
	}
	if (mnode->ref) {
		tree_print_text(f, "reference", mnode->ref);
	}
}

static void tree_print_type(FILE *f, char *indent, unsigned int max_name_len, struct ly_module *module, struct ly_type *type)
{
	int i;

	if (type->prefix) {
		fprintf(f, "%*stype %s:%s {\n", LEVEL, INDENT, type->prefix, type->der->name);
	} else {
		fprintf(f, "%*stype %s {\n", LEVEL, INDENT, type->der->name);
	}
	switch (type->base) {
	case LY_TYPE_ENUM:
		for (i = 0; i < type->info.enums.count; i++) {
			fprintf(f, "%*senum %s {\n", LEVEL, INDENT, type->info.enums.list[i].name);
			tree_print_mnode_common(f, (struct ly_mnode *)&type->info.enums.list[i]);
			fprintf(f, "%*svalue %d;\n", LEVEL, INDENT, type->info.enums.list[i].value);
			fprintf(f, "%*s}\n", LEVEL, INDENT);
		}
		break;
	case LY_TYPE_IDENT:
		if (module == type->info.ident.ref->module) {
			fprintf(f, "%*sbase %s;\n", LEVEL, INDENT, type->info.ident.ref->name);
		} else {
			fprintf(f, "%*sbase %s:%s;\n", LEVEL, INDENT, type->info.ident.ref->module->prefix, type->info.ident.ref->name);
		}
		break;
	default:
		/* TODO other cases */
		break;
	}
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void tree_print_container(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	char *new_indent;
	struct ly_mnode *sub;

	fprintf(f, "%s+--%s %s\n", indent, (mnode->flags & LY_NODE_CONFIG_W ? "rw" : "ro"), mnode->name);

	level++;
	new_indent = create_indent(level, indent, mnode);

	LY_TREE_FOR(mnode->child, sub) {
		tree_print_mnode(f, level, new_indent, max_name_len, sub, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_USES);
	}

	free(new_indent);
}

static void tree_print_choice(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	struct ly_mnode *sub;

	fprintf(f, "%*schoice %s {\n", LEVEL, INDENT, mnode->name);
	level++;
	tree_print_mnode_common(f, mnode);
	LY_TREE_FOR(mnode->child, sub) {
		tree_print_mnode(f, level, indent, max_name_len, sub,
		                 LY_NODE_CONTAINER | LY_NODE_LEAF |
		                 LY_NODE_LEAFLIST | LY_NODE_LIST);
	}
	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void tree_print_leaf(FILE *f, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	fprintf(f, "%s+--%s %s%s\n", indent, (mnode->flags & LY_NODE_CONFIG_W ? "rw" : "ro"), mnode->name, (mnode->flags & LY_NODE_MANDATORY ? "" : "?"));
}

static void tree_print_leaflist(FILE *f, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	fprintf(f, "%s+--%s %s*\n", indent, (mnode->flags & LY_NODE_CONFIG_W ? "rw" : "ro"), mnode->name);
}

static void tree_print_list(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	int i;
	char *new_indent;
	struct ly_mnode *sub;
	struct ly_mnode_list *list = (struct ly_mnode_list *)mnode;

	fprintf(f, "%s+--%s %s* [", indent, (mnode->flags & LY_NODE_CONFIG_W ? "rw" : "ro"), mnode->name);

	for (i = 0; i < list->keys_size; i++) {
		fprintf(f, "%s%s", list->keys[i]->name, i + 1 < list->keys_size ? "," : "");
	}
	fprintf(f, "]\n");

	level++;
	new_indent = create_indent(level, indent, mnode);

	LY_TREE_FOR(mnode->child, sub) {
		tree_print_mnode(f, level, new_indent, max_name_len, sub, LY_NODE_CHOICE | LY_NODE_CONTAINER |
		                 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_USES);
	}

	free(new_indent);
}

static void tree_print_grouping(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	struct ly_mnode *node;
	struct ly_mnode_grp *grp = (struct ly_mnode_grp *)mnode;

	level++;

	LY_TREE_FOR(mnode->child, node) {
		tree_print_mnode(f, level, indent, max_name_len, node, LY_NODE_CHOICE | LY_NODE_CONTAINER |
		                 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_USES | LY_NODE_GROUPING);
	}
}

static void tree_print_uses(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode)
{
	struct ly_mnode_uses *uses = (struct ly_mnode_uses *)mnode;

	tree_print_grouping(f, level, indent, max_name_len, (struct ly_mnode *)uses->grp);
}

static void tree_print_mnode(FILE *f, int level, char *indent, unsigned int max_name_len, struct ly_mnode *mnode,
                             int mask)
{
	switch(mnode->nodetype & mask) {
	case LY_NODE_CONTAINER:
		tree_print_container(f, level, indent, max_name_len, mnode);
		break;
	case LY_NODE_CHOICE:
		tree_print_choice(f, level, indent, max_name_len, mnode);
		break;
	case LY_NODE_LEAF:
		tree_print_leaf(f, indent, max_name_len, mnode);
		break;
	case LY_NODE_LEAFLIST:
		tree_print_leaflist(f, indent, max_name_len, mnode);
		break;
	case LY_NODE_LIST:
		tree_print_list(f, level, indent, max_name_len, mnode);
		break;
	case LY_NODE_USES:
		tree_print_uses(f, level, indent, max_name_len, mnode);
		break;
	default: break;
	}
}

int tree_print_model(FILE *f, struct ly_module *module)
{
	unsigned int max_child_name_len = 0;
	int level = 1;
	char *indent = malloc((level*4+1)*sizeof(char));
	strcpy(indent, "   ");

	struct ly_mnode *mnode;

	LY_TREE_FOR(module->data, mnode) {
		if (strlen(mnode->name) > max_child_name_len) {
			max_child_name_len = strlen(mnode->name);
		}
	}

	fprintf(f, "module: %s \n", module->name);
	level++;

	/*if (module->version) {
		fprintf(f, "%*syang-version \"%s\";\n", LEVEL, INDENT, module->version == 1 ? "1.0" : "1.1");
	}*/

	LY_TREE_FOR(module->data, mnode) {
		tree_print_mnode(f, level, indent, max_child_name_len, mnode, LY_NODE_CHOICE | LY_NODE_CONTAINER |
						 LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST |
						 LY_NODE_USES);
	}

	free(indent);
	return EXIT_SUCCESS;
#undef LEVEL
}
