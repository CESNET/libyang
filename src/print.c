/**
 * @file print.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Printers for libyang data structures
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

#include "common.h"
#include "tree.h"

#define INDENT ""
#define LEVEL (level*2)

static void yang_print_mnode(FILE *f, int level, struct ly_mnode *mnode,
                             int mask);

static void yang_print_text(FILE *f, int level, const char *name,
                            const char *text)
{
	const char *s, *t;

	fprintf(f, "%*s%s\n", LEVEL, INDENT, name);
	level++;

	fprintf(f, "%*s\"", LEVEL, INDENT);
	t = text;
	while((s = strchr(t, '\n'))) {
		fwrite(t, sizeof *t, s - t + 1, f);
		t = s + 1;
		fprintf(f, "%*s", LEVEL, INDENT);
	}

	fprintf(f, "%s\";\n\n", t);
	level--;

}

/*
 * Covers:
 * description, reference, status
 */
static void yang_print_mnode_common(FILE *f, int level, struct ly_mnode *mnode)
{
	if (mnode->flags & LY_NODE_STATUS_CURR) {
		fprintf(f, "%*sstatus \"current\";\n", LEVEL, INDENT);
	} else if (mnode->flags & LY_NODE_STATUS_DEPRC) {
		fprintf(f, "%*sstatus \"deprecated\";\n", LEVEL, INDENT);
	} else if (mnode->flags & LY_NODE_STATUS_OBSLT) {
		fprintf(f, "%*sstatus \"obsolete\";\n", LEVEL, INDENT);
	}

	if (mnode->dsc) {
		yang_print_text(f, level, "description", mnode->dsc);
	}
	if (mnode->ref) {
		yang_print_text(f, level, "reference", mnode->ref);
	}
}

/*
 * Covers:
 * config
 * description, reference, status
 */
static void yang_print_mnode_common2(FILE *f, int level, struct ly_mnode *mnode)
{
	if (!mnode->parent || (mnode->parent->flags & LY_NODE_CONFIG_MASK) != (mnode->flags & LY_NODE_CONFIG_MASK)) {
		/* print config only when it differs from the parent or in root */
		if (mnode->flags & LY_NODE_CONFIG_W) {
			fprintf(f, "%*sconfig \"true\";\n", LEVEL, INDENT);
		} else if (mnode->flags & LY_NODE_CONFIG_R) {
			fprintf(f, "%*sconfig \"false\";\n", LEVEL, INDENT);
		}
	}

	yang_print_mnode_common(f, level, mnode);
}

static void yang_print_type(FILE *f, int level, struct ly_type *type)
{
	fprintf(f, "%*stype %s {\n", LEVEL, INDENT, type->name);
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_typedef(FILE *f, int level, struct ly_tpdf *tpdf)
{
	fprintf(f, "%*stypedef %s {\n", LEVEL, INDENT, tpdf->name);
	level++;

	yang_print_mnode_common(f, level, (struct ly_mnode *)tpdf);
	yang_print_type(f, level, &tpdf->type);

	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_container(FILE *f, int level, struct ly_mnode *mnode)
{
	int i;
	struct ly_mnode *sub;
	struct ly_mnode_container *cont = (struct ly_mnode_container *)mnode;

	fprintf(f, "%*scontainer %s {\n", LEVEL, INDENT, mnode->name);
	level++;
	yang_print_mnode_common2(f, level, mnode);

	for (i = 0; i < cont->tpdf_size; i++) {
		yang_print_typedef(f, level, &cont->tpdf[i]);
	}

	LY_TREE_FOR(mnode->child, sub) {
		yang_print_mnode(f, level, sub, LY_NODE_CHOICE |LY_NODE_CONTAINER |
		                 LY_NODE_LEAF |LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_GROUPING);
	}

	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_choice(FILE *f, int level, struct ly_mnode *mnode)
{
	struct ly_mnode *sub;

	fprintf(f, "%*schoice %s {\n", LEVEL, INDENT, mnode->name);
	level++;
	yang_print_mnode_common2(f, level, mnode);
	LY_TREE_FOR(mnode->child, sub) {
		yang_print_mnode(f, level, sub,
		                 LY_NODE_CONTAINER | LY_NODE_LEAF |
		                 LY_NODE_LEAFLIST | LY_NODE_LIST);
	}
	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_leaf(FILE *f, int level, struct ly_mnode *mnode)
{
	struct ly_mnode_leaf *leaf = (struct ly_mnode_leaf *)mnode;

	fprintf(f, "%*sleaf %s {\n", LEVEL, INDENT, mnode->name);
	level++;
	yang_print_mnode_common2(f, level, mnode);
	yang_print_type(f, level, &leaf->type);
	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_leaflist(FILE *f, int level, struct ly_mnode *mnode)
{
	struct ly_mnode_leaflist *llist = (struct ly_mnode_leaflist *)mnode;

	fprintf(f, "%*sleaf-list %s {\n", LEVEL, INDENT, mnode->name);
	level++;
	yang_print_mnode_common2(f, level, mnode);
	yang_print_type(f, level, &llist->type);
	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_list(FILE *f, int level, struct ly_mnode *mnode)
{
	int i;
	struct ly_mnode *sub;
	struct ly_mnode_list *list = (struct ly_mnode_list *)mnode;

	fprintf(f, "%*slist %s {\n", LEVEL, INDENT, mnode->name);
	level++;
	yang_print_mnode_common2(f, level, mnode);

	for (i = 0; i < list->tpdf_size; i++) {
		yang_print_typedef(f, level, &list->tpdf[i]);
	}

	LY_TREE_FOR(mnode->child, sub) {
		yang_print_mnode(f, level, sub, LY_NODE_CHOICE |LY_NODE_CONTAINER |
		                 LY_NODE_LEAF |LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_GROUPING);
	}
	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_grouping(FILE *f, int level, struct ly_mnode *mnode)
{
	int i;
	struct ly_mnode *node;
	struct ly_mnode_grp *grp = (struct ly_mnode_grp *)mnode;

	fprintf(f, "%*sgrouping %s {\n", LEVEL, INDENT, mnode->name);
	level++;

	yang_print_mnode_common(f, level, mnode);

	for (i = 0; i < grp->tpdf_size; i++) {
		yang_print_typedef(f, level, &grp->tpdf[i]);
	}

	LY_TREE_FOR(mnode->child, node) {
		yang_print_mnode(f, level, node, LY_NODE_CHOICE |LY_NODE_CONTAINER |
		                 LY_NODE_LEAF |LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_GROUPING);
	}

	level--;
	fprintf(f, "%*s}\n", LEVEL, INDENT);
}

static void yang_print_mnode(FILE *f, int level, struct ly_mnode *mnode,
                             int mask)
{
	switch(mnode->nodetype & mask) {
	case LY_NODE_CONTAINER:
		yang_print_container(f, level, mnode);
		break;
	case LY_NODE_CHOICE:
		yang_print_choice(f, level, mnode);
		break;
	case LY_NODE_LEAF:
		yang_print_leaf(f, level, mnode);
		break;
	case LY_NODE_LEAFLIST:
		yang_print_leaflist(f, level, mnode);
		break;
	case LY_NODE_LIST:
		yang_print_list(f, level, mnode);
		break;
	case LY_NODE_GROUPING:
		yang_print_grouping(f, level, mnode);
		break;
	default: break;
	}
}

API int ly_model_print(FILE *f, struct ly_module *module, LY_MFORMAT format)
{
	int i;
	int level = 0;
#define LEVEL (level*2)

	struct ly_mnode *mnode;

	if (!f || !module) {
		ly_errno = LY_EINVAL;
		return EXIT_FAILURE;
	}

	if (format != LY_YANG) {
		return EXIT_FAILURE;
	}

	fprintf(f, "module %s {\n", module->name);
	level++;
	/*
	fprintf(f, "%*snamespace \"%s\";\n", LEVEL, INDENT, module->ns);
	fprintf(f, "%*sprefix \"%s\";\n", LEVEL, INDENT, module->prefix);
	*/
	yang_print_text(f, level, "namespace", module->ns);
	yang_print_text(f, level, "prefix", module->prefix);

	for (i = 0; i < module->imp_size; i++) {
		fprintf(f, "%*simport \"%s\" {\n", LEVEL, INDENT,
				module->imp[i].module->name);
		level++;
		yang_print_text(f, level, "prefix", module->imp[i].prefix);
		if (module->imp[i].rev[0]) {
			yang_print_text(f, level, "revision-date", module->imp[i].rev);
		}
		level--;
		fprintf(f, "%*s}\n", LEVEL, INDENT);
	}

	if (module->org) {
		yang_print_text(f, level, "organization", module->org);
	}
	if (module->contact) {
		yang_print_text(f, level, "contact", module->contact);
	}
	if (module->dsc) {
		yang_print_text(f, level, "description", module->dsc);
	}
	if (module->ref) {
		yang_print_text(f, level, "reference", module->ref);
	}
	for (i = 0; i < module->rev_size; i++) {
		if (module->rev[i].dsc || module->rev[i].ref) {
			fprintf(f, "%*srevision \"%s\" {\n", LEVEL, INDENT,
					module->rev[i].date);
			level++;
			if (module->rev[i].dsc) {
				yang_print_text(f, level, "description", module->rev[i].dsc);
			}
			if (module->rev[i].ref) {
				yang_print_text(f, level, "reference", module->rev[i].ref);
			}
			level--;
			fprintf(f, "%*s}\n", LEVEL, INDENT);
		} else {
			yang_print_text(f, level, "revision", module->rev[i].date);
		}
	}

	for (i = 0; i < module->tpdf_size; i++) {
		yang_print_typedef(f, level, &module->tpdf[i]);
	}

	LY_TREE_FOR(module->data, mnode) {
		yang_print_mnode(f, level, mnode, LY_NODE_CHOICE |LY_NODE_CONTAINER |
                         LY_NODE_LEAF |LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_GROUPING);
	}

	fprintf(f, "}\n");

	return EXIT_SUCCESS;
#undef LEVEL
}
