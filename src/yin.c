/**
 * @file yin.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YIN parser for libyang
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
#include <string.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "dict.h"
#include "yin.h"

#include "tree_internal.h"
#include "xml.h"

static struct ly_mnode *read_yin_choice(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_container(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_leaf(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_leaflist(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_list(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_grouping(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);

static char *read_yin_text(struct ly_ctx *ctx, struct lyxml_elem *node, const char *name)
{
	char *value;

	/* there should be <text> child */
	if (!node->child || !node->child->name
	        || strcmp(node->child->name, "text")) {
		LY_WRN("Expected \"text\" element in \"%s\" element.", name);
	} else {
		value = node->child->content;
		if (value) {
			return lydict_insert(ctx, value, strlen(value));
		}
	}
	return NULL;
}

static struct ly_tpdf *find_superior_type(const char *name,
                                          struct ly_module *module,
                                          struct ly_mnode *parent)
{
	int i, found = 0;
	int prefix_len = 0;
	const char *qname;

	qname = strchr(name, ':');

	if (!qname) {
		/* no prefix, try built-in types */
		for (i = 1; i < LY_DATA_TYPE_COUNT; i++) {
			if (!strcmp(ly_types[i].def->name, name)) {
				return ly_types[i].def;
			}
		}
		qname = name;
	} else {
		/* set qname to correct position after colon */
		prefix_len = qname - name;
		qname++;

		if (!strncmp(name, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
			/* prefix refers to the current module, ignore it */
			prefix_len = 0;
		}
	}

	if (!prefix_len && parent) {
		/* TODO - search in local typedefs */
	} else if (prefix_len) {
		/* get module where to search */
		for (i = 0; i < module->imp_size; i++) {
			if (!strncmp(module->imp[i].prefix, name, prefix_len) && !module->imp[i].prefix[prefix_len]) {
				module = module->imp[i].module;
				found = 1;
				break;
			}
		}
		if (!found) {
			/* TODO - syntax error */
			return NULL;
		}
	}

	/* search in top level typedefs */
	for (i = 0; i < module->tpdf_size; i++) {
		if (!strcmp(module->tpdf[i].name, qname)) {
			return &module->tpdf[i];
		}
	}

	return NULL;
}

static int fill_yin_type(struct ly_module *module, struct ly_mnode *parent,
                         struct lyxml_elem *yin, struct ly_type *type)
{
	const char *value;

	value = lyxml_get_attr(yin, "name", NULL);
	type->name = lydict_insert(module->ctx, value, strlen(value));
	type->der = find_superior_type(value, module, parent);

	return EXIT_SUCCESS;
}

static int fill_yin_typedef(struct ly_module *module, struct ly_mnode *parent,
                            struct lyxml_elem *yin, struct ly_tpdf *tpdf)
{
	const char *value;
	struct lyxml_elem *node, *next;

	value = lyxml_get_attr(yin, "name", NULL);
	tpdf->name = lydict_insert(module->ctx, value, strlen(value));

	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!strcmp(node->name, "type")) {
			fill_yin_type(module, parent, node, &tpdf->type);
			tpdf->base = tpdf->type.der->base;
		/* optional statements */
		} else if (!strcmp(node->name, "description")) {
			tpdf->dsc = read_yin_text(module->ctx, node, "description");
			lyxml_free_elem(module->ctx, node);
		} else if (!strcmp(node->name, "reference")) {
			tpdf->ref = read_yin_text(module->ctx, node, "reference");
			lyxml_free_elem(module->ctx, node);
		} else if (!strcmp(node->name, "status")) {
			value = lyxml_get_attr(node, "value", NULL);
			if (!strcmp(value, "current")) {
				tpdf->flags |= LY_NODE_STATUS_CURR;
			} else if (!strcmp(value, "deprecated")) {
				tpdf->flags |= LY_NODE_STATUS_DEPRC;
			} else if (!strcmp(value, "obsolete")) {
				tpdf->flags |= LY_NODE_STATUS_OBSLT;
			}
		}
	}

	return EXIT_SUCCESS;
}

/*
 * Covers:
 * description, reference, config, status,
 */
static int read_yin_common(struct ly_module *module, struct ly_mnode *parent,
		                   struct ly_mnode *mnode, struct lyxml_elem *xmlnode)
{
	const char *value;
	struct lyxml_elem *sub, *next;
	struct ly_ctx * const ctx = module->ctx;

	mnode->module = module;

	value = lyxml_get_attr(xmlnode, "name", NULL);
	mnode->name = lydict_insert(ctx, value, strlen(value));

	/* process local parameters */
	LY_TREE_FOR_SAFE(xmlnode->child, next, sub) {
		if (!strcmp(sub->name, "description")) {
			mnode->dsc = read_yin_text(ctx, sub, "description");
		} else if (!strcmp(sub->name, "reference")) {
			mnode->ref = read_yin_text(ctx, sub, "reference");
		} else if (!strcmp(sub->name, "config")) {
			value = lyxml_get_attr(sub, "value", NULL);
			if (!strcmp(value, "false")) {
				mnode->flags |= LY_NODE_CONFIG_R;
			} else if (!strcmp(value, "false")) {
				mnode->flags |= LY_NODE_CONFIG_W;
			}
		} else if (!strcmp(sub->name, "status")) {
			value = lyxml_get_attr(sub, "value", NULL);
			if (!strcmp(value, "current")) {
				mnode->flags |= LY_NODE_STATUS_CURR;
			} else if (!strcmp(value, "deprecated")) {
				mnode->flags |= LY_NODE_STATUS_DEPRC;
			} else if (!strcmp(value, "obsolete")) {
				mnode->flags |= LY_NODE_STATUS_OBSLT;
			}
		} else {
			/* skip the lyxml_free_elem */
			continue;
		}
		lyxml_free_elem(ctx, sub);
	}

	if (mnode->nodetype != LY_NODE_GROUPING && !(mnode->flags & LY_NODE_CONFIG_MASK)) {
		/* get config flag from parent */
		if (parent) {
			mnode->flags |= parent->flags & LY_NODE_CONFIG_MASK;
		} else {
			/* default config is true */
			mnode->flags |= LY_NODE_CONFIG_W;
		}
	}

	return EXIT_SUCCESS;
}

static struct ly_mnode *read_yin_choice(struct ly_module *module,
                                        struct ly_mnode *parent,
                                        struct lyxml_elem *node)
{
	struct lyxml_elem *sub, *next;
	struct ly_ctx * const ctx = module->ctx;
	struct ly_mnode *retval;
	struct ly_mnode_choice *choice;

	if (!module || !node) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	choice = calloc(1, sizeof *choice);
	choice->nodetype = LY_NODE_CHOICE;
	choice->module = module;
	choice->prev = (struct ly_mnode *)choice;
	retval = (struct ly_mnode *)choice;

	if (read_yin_common(module, parent, retval, node)) {
		goto error;
	}

	/* process choice's specific children */
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			read_yin_list(module, retval, sub);
		}
		lyxml_free_elem(ctx, sub);
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_leaf(struct ly_module *module,
                                      struct ly_mnode *parent,
                                      struct lyxml_elem *node)
{
	struct ly_mnode *retval;
	struct ly_mnode_leaf *leaf;
	struct lyxml_elem *sub, *next;

	if (!module || !node) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	leaf = calloc(1, sizeof *leaf);
	leaf->nodetype = LY_NODE_LEAF;
	leaf->prev = (struct ly_mnode *)leaf;
	retval = (struct ly_mnode *)leaf;

	if (read_yin_common(module, parent, retval, node)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "type")) {
			fill_yin_type(module, parent, sub, &leaf->type);
		}
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_leaflist(struct ly_module *module,
                                          struct ly_mnode *parent,
                                          struct lyxml_elem *node)
{
	struct ly_mnode *retval;
	struct ly_mnode_leaflist *llist;
	struct lyxml_elem *sub, *next;

	if (!module || !node) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	llist = calloc(1, sizeof *llist);
	llist->nodetype = LY_NODE_LEAFLIST;
	llist->prev = (struct ly_mnode *)llist;
	retval = (struct ly_mnode *)llist;

	if (read_yin_common(module, parent, retval, node)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "type")) {
			fill_yin_type(module, parent, sub, &llist->type);
		}
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_list(struct ly_module *module,
                                      struct ly_mnode *parent,
                                      struct lyxml_elem *node)
{
	struct ly_mnode *retval, *mnode;
	struct ly_mnode_list *list;
	struct lyxml_elem *sub, *next, root = {0};
	int c_tpdf = 0;

	if (!module || !node) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	list = calloc(1, sizeof *list);
	list->nodetype = LY_NODE_LIST;
	list->prev = (struct ly_mnode *)list;
	retval = (struct ly_mnode *)list;

	if (read_yin_common(module, parent, retval, node)) {
		goto error;
	}

	/* process list's specific children */
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		/* data statements */
		if (!strcmp(sub->name, "container") ||
				!strcmp(sub->name, "leaf-list") ||
				!strcmp(sub->name, "leaf") ||
				!strcmp(sub->name, "list") ||
				!strcmp(sub->name, "choice") ||
				!strcmp(sub->name, "grouping")) {
			lyxml_unlink_elem(sub);
			lyxml_add_child(&root, sub);

		/* array counters */
		} else if (!strcmp(sub->name, "typedef")) {
			c_tpdf++;
		}
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	if (c_tpdf) {
		list->tpdf_size = c_tpdf;
		list->tpdf = calloc(c_tpdf, sizeof *list->tpdf);
		c_tpdf = 0;
	}
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "typedef")) {
			fill_yin_typedef(module, retval, sub, &list->tpdf[c_tpdf]);
			c_tpdf++;
		}

		lyxml_free_elem(module->ctx, sub);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			mnode = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			mnode = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			mnode = read_yin_list(module, retval, sub);
		} else if (!strcmp(sub->name, "choice")) {
			mnode = read_yin_choice(module, retval, sub);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (mnode) {
			/* add child */
			ly_mnode_addchild(retval, mnode);
			mnode = NULL;
		}
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_container(struct ly_module *module,
                                           struct ly_mnode *parent,
                                           struct lyxml_elem *node)
{
	struct lyxml_elem *sub, *next, root = {0};
	struct ly_mnode *mnode = NULL;
	struct ly_mnode *retval;
	struct ly_mnode_container *cont;
	int c_tpdf = 0;

	if (!module || !node) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	cont = calloc(1, sizeof *cont);
	cont->nodetype = LY_NODE_CONTAINER;
	cont->prev = (struct ly_mnode *)cont;
	retval = (struct ly_mnode *)cont;

	if (read_yin_common(module, parent, retval, node)) {
		goto error;
	}

	/* process container's specific children */
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		/* data statements */
		if (!strcmp(sub->name, "container") ||
				!strcmp(sub->name, "leaf-list") ||
				!strcmp(sub->name, "leaf") ||
				!strcmp(sub->name, "list") ||
				!strcmp(sub->name, "choice") ||
				!strcmp(sub->name, "grouping")) {
			lyxml_unlink_elem(sub);
			lyxml_add_child(&root, sub);

		/* array counters */
		} else if (!strcmp(sub->name, "typedef")) {
			c_tpdf++;
		}
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	if (c_tpdf) {
		cont->tpdf_size = c_tpdf;
		cont->tpdf = calloc(c_tpdf, sizeof *cont->tpdf);
		c_tpdf = 0;
	}
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "typedef")) {
			fill_yin_typedef(module, retval, sub, &cont->tpdf[c_tpdf]);
			c_tpdf++;
		}

		lyxml_free_elem(module->ctx, sub);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			mnode = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			mnode = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			mnode = read_yin_list(module, retval, sub);
		} else if (!strcmp(sub->name, "choice")) {
			mnode = read_yin_choice(module, retval, sub);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (mnode) {
			/* add child */
			ly_mnode_addchild(retval, mnode);
			mnode = NULL;
		}
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

static struct ly_mnode *read_yin_grouping(struct ly_module *module,
                                          struct ly_mnode *parent,
                                          struct lyxml_elem *node)
{
	struct lyxml_elem *sub, *next, root = {0};
	struct ly_mnode *mnode = NULL;
	struct ly_mnode *retval;
	struct ly_mnode_grp *grp;
	int c_tpdf = 0;

	grp = calloc(1, sizeof *grp);
	grp->nodetype = LY_NODE_GROUPING;
	grp->module = module;
	grp->prev = (struct ly_mnode *)grp;
	retval = (struct ly_mnode *)grp;

	if (read_yin_common(module, parent, retval, node)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(node->child, next, sub) {
		/* data statements */
		if (!strcmp(sub->name, "container") ||
				!strcmp(sub->name, "leaf-list") ||
				!strcmp(sub->name, "leaf") ||
				!strcmp(sub->name, "list") ||
				!strcmp(sub->name, "choice") ||
				!strcmp(sub->name, "grouping")) {
			lyxml_unlink_elem(sub);
			lyxml_add_child(&root, sub);

		/* array counters */
		} else if (!strcmp(sub->name, "typedef")) {
			c_tpdf++;
		}
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	if (c_tpdf) {
		grp->tpdf_size = c_tpdf;
		grp->tpdf = calloc(c_tpdf, sizeof *grp->tpdf);
		c_tpdf = 0;
	}
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "typedef")) {
			fill_yin_typedef(module, retval, sub, &grp->tpdf[c_tpdf]);
			c_tpdf++;
		}

		lyxml_free_elem(module->ctx, sub);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			mnode = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			mnode = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			mnode = read_yin_list(module, retval, sub);
		} else if (!strcmp(sub->name, "choice")) {
			mnode = read_yin_choice(module, retval, sub);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (mnode) {
			/* add child */
			ly_mnode_addchild(retval, mnode);
			mnode = NULL;
		}
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

struct ly_module *ly_read_yin(struct ly_ctx *ctx, const char *data)
{
	struct lyxml_elem *yin, *node, *next, *child, root = {0};
	struct ly_module *module = NULL, **newlist = NULL, *imp;
	struct ly_mnode *ynode = NULL;
	const char *value;
	int i;
	/* counters */
	int c_imp = 0, c_rev = 0, c_tpdf = 0;

	yin = lyxml_read(ctx, data, 0);
	if (!yin) {
		return NULL;
	}

	/* check root element */
	if (!yin->name || strcmp(yin->name, "module")) {
		/* TODO: support submodules */
		LY_ERR(LY_EVALID, "Expected \"module\" element, but have \"%s\".");
		goto error;
	}

	/* check its namespace */
	if (!yin->ns || !yin->ns->value || strcmp(yin->ns->value, LY_NSYIN)) {
		LY_ERR(LY_EVALID,
		       "Invalid namespace of the \"module\" element, \"" LY_NSYIN
		       "\" expected.");
		goto error;
	}

	value = lyxml_get_attr(yin, "name", NULL);
	if (!value) {
		LY_ERR(LY_EVALID, "Missing \"name\" attribute of the \"module\".");
		goto error;
	}

	module = calloc(1, sizeof *module);
	if (!module) {
		ly_errno = LY_EFATAL;
		goto error;
	}

	module->ctx = ctx;
	module->name = lydict_insert(ctx, value, strlen(value));

	LY_VRB("reading module %s", module->name);

	/*
	 * in the first run, we process elements with cardinality of 1 or 0..1 and
	 * count elements with cardinality 0..n. Data elements (choices, containers,
	 * leafs, lists, leaf-lists) are moved aside to be processed last, since we
	 * need have all top-level and groupings already prepared at that time. In
	 * the middle loop, we process other elements with carinality of 0..n since
	 * we need to allocate arrays to store them.
	 */
	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!node->ns || strcmp(node->ns->value, LY_NSYIN)) {
			lyxml_free_elem(ctx, node);
			continue;
		}

		if (!strcmp(node->name, "namespace")) {
			value = lyxml_get_attr(node, "uri", NULL);
			if (!value) {
				LY_ERR(LY_EVALID,
				       "%s: Missing \"uri\" attribute in \"namespace\" element.", module->name);
				goto error;
			}
			module->ns = lydict_insert(ctx, value, strlen(value));
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "prefix")) {
			value = lyxml_get_attr(node, "value", NULL);
			if (!value) {
				LY_ERR(LY_EVALID,
				       "%s: Missing \"value\" attribute in \"prefix\" element.", module->name);
				goto error;
			}
			module->prefix = lydict_insert(ctx, value, strlen(value));
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "import")) {
			c_imp++;
		} else if (!strcmp(node->name, "revision")) {
			c_rev++;
		} else if (!strcmp(node->name, "typedef")) {
			c_tpdf++;

		/* data statements */
		} else if (!strcmp(node->name, "container") ||
				!strcmp(node->name, "leaf-list") ||
				!strcmp(node->name, "leaf") ||
				!strcmp(node->name, "list") ||
				!strcmp(node->name, "choice") ||
				!strcmp(node->name, "grouping")) {
			lyxml_unlink_elem(node);
			lyxml_add_child(&root, node);

		/* optional statements */
		} else if (!strcmp(node->name, "description")) {
			module->dsc = read_yin_text(ctx, node, "description");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "reference")) {
			module->ref = read_yin_text(ctx, node, "reference");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "organization")) {
			module->org = read_yin_text(ctx, node, "organization");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "contact")) {
			module->contact = read_yin_text(ctx, node, "contact");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "yang-version")) {
			/* TODO: support YANG 1.1 */
			value = lyxml_get_attr(node, "value", NULL);
			if (strcmp(value, "1")) {
				LY_ERR(LY_EVALID, "%s: Invalid \"yang-version\" value.", module->name);
				goto error;
			}
			module->version = 0;
			lyxml_free_elem(ctx, node);
		}
	}

	/* allocate arrays for elements with cardinality of 0..n */
	if (c_imp) {
		module->imp_size = c_imp;
		module->imp = calloc(c_imp, sizeof *module->imp);
		c_imp = 0;
	}
	if (c_rev) {
		module->rev_size = c_rev;
		module->rev = calloc(c_rev, sizeof *module->rev);
		c_rev = 0;
	}
	if (c_tpdf) {
		module->tpdf_size = c_tpdf;
		module->tpdf = calloc(c_tpdf, sizeof *module->tpdf);
		c_tpdf = 0;
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!strcmp(node->name, "import")) {
			LY_TREE_FOR(node->child, child) {
				if (!strcmp(child->name, "prefix")) {
					value = lyxml_get_attr(child, "value", NULL);
					module->imp[c_imp].prefix = lydict_insert(ctx, value, strlen(value));
				} else if (!strcmp(child->name, "revision-date")) {
					value = lyxml_get_attr(child, "date", NULL);
					memcpy(module->imp[c_imp].rev,
					       lyxml_get_attr(child, "date", NULL),
					       LY_REV_SIZE - 1);
				}
			}
			value = lyxml_get_attr(node, "module", NULL);
			imp = ly_ctx_get_model(ctx, value, module->imp[c_imp].rev[0] ? module->imp[c_imp].rev : NULL);
			if (!imp) {
				LY_ERR(LY_EVALID, "Importing \"%s\" module into \"%s\" failed.",
				       value, module->name);
				goto error;
			}
			module->imp[c_imp].module = imp;
			c_imp++;
		} else if (!strcmp(node->name, "revision")) {
			memcpy(module->rev[c_rev].date,
			       lyxml_get_attr(node, "date", NULL), LY_REV_SIZE - 1);
			LY_TREE_FOR(node->child, child) {
				if (!strcmp(child->name, "description")) {
					module->rev[c_rev].dsc = read_yin_text(ctx, child, "description");
				} else if (!strcmp(child->name, "reference")) {
					module->rev[c_rev].ref = read_yin_text(ctx, child, "reference");
				}
			}
			c_rev++;
		} else if (!strcmp(node->name, "typedef")) {
			fill_yin_typedef(module, NULL, node, &module->tpdf[c_tpdf]);
			c_tpdf++;
		}

		lyxml_free_elem(ctx, node);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, node) {

		if (!strcmp(node->name, "container")) {
			ynode = read_yin_container(module, NULL, node);
		} else if (!strcmp(node->name, "leaf-list")) {
			ynode = read_yin_leaflist(module, NULL, node);
		} else if (!strcmp(node->name, "leaf")) {
			ynode = read_yin_leaf(module, NULL, node);
		} else if (!strcmp(node->name, "list")) {
			ynode = read_yin_list(module, NULL, node);
		} else if (!strcmp(node->name, "choice")) {
			ynode = read_yin_choice(module, NULL, node);
		} else if (!strcmp(node->name, "grouping")) {
			ynode = read_yin_grouping(module, NULL, node);
		} else {
			continue;
		}
		lyxml_free_elem(ctx, node);

		if (ynode) {
			/* include data element */
			if (module->data) {
				module->data->prev->next = ynode;
				ynode->prev = module->data->prev;
				module->data->prev = ynode;
			} else {
				module->data = ynode;
			}
			ynode = NULL;
		}
	}

	/* add to the context's list of modules */
	if (ctx->models.used == ctx->models.size) {
		newlist = realloc(ctx->models.list, ctx->models.size * 2);
		if (!newlist) {
			LY_ERR(LY_EFATAL, NULL);
			goto error;
		}
		for (i = ctx->models.size; i < ctx->models.size * 2; i++) {
			newlist[i] = NULL;
		}
		ctx->models.size *= 2;
		ctx->models.list = newlist;
	}
	for (i = 0; ctx->models.list[i]; i++);
	ctx->models.list[i] = module;
	ctx->models.used++;

	/* cleanup */
	lyxml_free_elem(ctx, yin);

	LY_VRB("module %s successfully parsed", module->name);

	return module;

error:
	/* cleanup */
	lyxml_free_elem(ctx, yin);
	ly_model_free(module);

	return NULL;
}
