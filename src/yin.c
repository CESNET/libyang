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

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "dict.h"
#include "yin.h"

#include "tree_internal.h"
#include "xml.h"

static int read_yin_common(struct ly_module *, struct ly_mnode *, struct ly_mnode *, struct lyxml_elem *, int );
static struct ly_mnode *read_yin_choice(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_container(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_leaf(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_leaflist(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_list(struct ly_module *, struct ly_mnode *, struct lyxml_elem *);
static struct ly_mnode *read_yin_uses(struct ly_module *, struct ly_mnode *, struct lyxml_elem *, int);
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
	struct ly_tpdf *tpdf;
	int tpdf_size;

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
		/* search in local typedefs */
		while (parent) {
			switch (parent->nodetype) {
			case LY_NODE_CONTAINER:
				tpdf_size = ((struct ly_mnode_container *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_container *)parent)->tpdf;
				break;
			case LY_NODE_LIST:
				tpdf_size = ((struct ly_mnode_list *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_list *)parent)->tpdf;
				break;
			case LY_NODE_GROUPING:
				tpdf_size = ((struct ly_mnode_grp *)parent)->tpdf_size;
				tpdf = ((struct ly_mnode_grp *)parent)->tpdf;
				break;
			default:
				parent = parent->parent;
				continue;
			}

			for (i = 0; i < tpdf_size; i++) {
				if (!strcmp(tpdf[i].name, qname)) {
					return &tpdf[i];
				}
			}

			parent = parent->parent;
		}
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

static struct ly_ident *find_base_ident(struct ly_module *module, struct ly_ident *ident, const char *basename)
{
	const char *name;
	int prefix_len = 0;
	int i, found = 0;
	struct ly_ident *base_iter;
	struct ly_ident_der *der;

	if (!basename) {
		ly_verr(LY_VERR_MISS_ARG, "name", "base");
		return NULL;
	}

	/* search for the base identity */
	name = strchr(basename, ':');
	if (name) {
		/* set name to correct position after colon */
		prefix_len = name - basename;
		name++;

		if (!strncmp(basename, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
			/* prefix refers to the current module, ignore it */
			prefix_len = 0;
		}
	} else {
		name = basename;
	}

	if (prefix_len) {
		/* get module where to search */
		for (i = 0; i < module->imp_size; i++) {
			if (!strncmp(module->imp[i].prefix, basename, prefix_len)
					&& !module->imp[i].prefix[prefix_len]) {
				module = module->imp[i].module;
				found = 1;
				break;
			}
		}
		if (!found) {
			/* identity refers unknown data model */
			ly_verr(LY_VERR_UNEXP_PREFIX, basename);
			return NULL;
		}
	}

	/* search in the identified module */
	/* TODO what about submodules? */
	for (i = 0; i < module->ident_size; i++) {
		if (!strcmp(name, module->ident[i].name)) {
			/* we are done */

			if (!ident) {
				/* just search for type, so do not modify anything, just return
				 * the base identity pointer
				 */
				return &module->ident[i];
			}

			/* we are resolving identity definition, so now update structures */
			ident->base = base_iter = &module->ident[i];

			while (base_iter) {
				for (der = base_iter->der; der && der->next; der = der->next);
				if (der) {
					der->next = malloc(sizeof *der);
					der = der->next;
				} else {
					ident->base->der = der = malloc(sizeof *der);
				}
				der->next = NULL;
				der->ident = ident;

				base_iter = base_iter->base;
			}
			return ident->base;
		}
	}

	ly_verr(LY_VERR_UNEXP_VAL, basename, ident ? "identity" : "type");
	return NULL;
}

static int fill_yin_identity(struct ly_module *module, struct lyxml_elem *yin, struct ly_ident *ident)
{
	struct lyxml_elem *node, *next;

	if (read_yin_common(module, NULL, (struct ly_mnode *)ident, yin, 0)) {
		return EXIT_FAILURE;
	}
	ident->module = module;

	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!strcmp(node->name, "base")) {
			if (ident->base) {
				ly_verr(LY_VERR_TOOMANY, "base", "identity");
				return EXIT_FAILURE;
			}
			if (!find_base_ident(module, ident, lyxml_get_attr(node, "name", NULL))) {
				return EXIT_FAILURE;
			}
		} else {
			ly_verr(LY_VERR_UNEXP_STMT, node->name, "identity");
			return EXIT_FAILURE;
		}

		lyxml_free_elem(module->ctx, node);
	}

	return EXIT_SUCCESS;
}

static int fill_yin_type(struct ly_module *module, struct ly_mnode *parent,
                         struct lyxml_elem *yin, struct ly_type *type)
{
	const char *value, *delim;
	struct lyxml_elem *next, *node, root = {0};
	int i, j, r;
	int64_t v, v_;

	value = lyxml_get_attr(yin, "name", NULL);
	if (!value) {
		ly_verr(LY_VERR_MISS_ARG, "name", yin->name);
		return EXIT_FAILURE;
	}

	delim = strchr(value, ':');
	if (delim) {
		type->prefix = lydict_insert(module->ctx, value, delim - value);
	}

	type->der = find_superior_type(value, module, parent);
	/* TODO error */
	type->base = type->der->type.base;

	switch (type->base) {
	case LY_TYPE_BINARY:
		/* length, 9.4.4
		 * - optional, 0..1, rekurzivni - omezuje, string (podobne jako range),
		 * hodnoty se musi vejit do 64b, podelementy
		 */
		break;
	case LY_TYPE_BITS:
		/* bit, 9.7.4
		 * 1..n, nerekurzivni, stringy s podelementy */
		break;
	case LY_TYPE_DEC64:
		/* fraction-digits, 9.3.4
		 * - MUST, 1, nerekurzivni, hodnota 1-18 */
		/* range, 9.2.4
		 * - optional, 0..1, rekurzivne - omezuje, string,  podelementy*/
		break;
	case LY_TYPE_ENUM:
		/* RFC 6020 9.6 */

		/* get enum specification, at least one must be present */
		LY_TREE_FOR_SAFE(yin->child, next, node) {
			if (!strcmp(node->name, "enum")) {
				lyxml_unlink_elem(node);
				lyxml_add_child(&root, node);
				type->info.enums.count++;
			}
		}
		if (yin->child) {
			ly_verr(LY_VERR_UNEXP_STMT, yin->child->name);
			goto error;
		}
		if (!type->info.enums.count) {
			if (type->der->type.der) {
				/* this is just a derived type with no enum specified */
				break;
			}
			ly_verr(LY_VERR_MISS_STMT2, "enum", "type");
			goto error;
		}

		type->info.enums.list = calloc(type->info.enums.count, sizeof *type->info.enums.list);
		for (i = v = 0; root.child; i++) {
			r = read_yin_common(module, NULL, (struct ly_mnode *)&type->info.enums.list[i], root.child, 0);
			if (r) {
				type->info.enums.count = i + 1;
				goto error;
			}
			/* the assigned name MUST NOT have any leading or trailing whitespace characters */
			value = type->info.enums.list[i].name;
			if (isspace(value[0]) || isspace(value[strlen(value) - 1])) {
				ly_verr(LY_VERR_ENUM_WS, value);
				type->info.enums.count = i + 1;
				goto error;
			}

			/* check the name uniqueness */
			for (j = 0; j < i; j++) {
				if (!strcmp(type->info.enums.list[j].name, type->info.enums.list[i].name)) {
					ly_verr(LY_VERR_ENUM_DUP_NAME, type->info.enums.list[i].name);
					type->info.enums.count = i + 1;
					goto error;
				}
			}

			node = root.child->child;
			if (node && !strcmp(node->name, "value")) {
				value = lyxml_get_attr(node, "value", NULL);
				v_ = strtol(value, NULL, 10);

				/* range check */
				if (v_ < INT32_MIN || v_ > INT32_MAX) {
					ly_verr(LY_VERR_UNEXP_VAL, value, "enum/value");
					type->info.enums.count = i + 1;
					goto error;
				}
				type->info.enums.list[i].value = v_;

				/* keep the highest enum value for automatic increment */
				if (type->info.enums.list[i].value > v) {
					v = type->info.enums.list[i].value;
					v++;
				} else {
					/* check that the value is unique */
					for (j = 0; j < i; j++) {
						if (type->info.enums.list[j].value == type->info.enums.list[i].value) {
							ly_verr(LY_VERR_ENUM_DUP_VAL, type->info.enums.list[i].value, type->info.enums.list[i].name);
							type->info.enums.count = i + 1;
							goto error;
						}
					}
				}
			} else {
				/* assign value automatically */
				if (v > INT32_MAX) {
					ly_verr(LY_VERR_UNEXP_VAL, "2147483648", "enum/value");
					type->info.enums.count = i + 1;
					goto error;
				}
				type->info.enums.list[i].value = v;
				v++;
			}
			lyxml_free_elem(module->ctx, root.child);
		}
		break;
	case LY_TYPE_IDENT:
		/* RFC 6020 9.10 */

		/* get base specification, exactly one must be present */
		if (!yin->child) {
			ly_verr(LY_VERR_MISS_STMT2, "base", "type");
			goto error;
		}
		if (strcmp(yin->child->name, "base")) {
			ly_verr(LY_VERR_UNEXP_STMT, yin->child->name);
			goto error;
		}
		if (yin->child->next) {
			ly_verr(LY_VERR_UNEXP_STMT, yin->child->next->name);
			goto error;
		}
		type->info.ident.ref = find_base_ident(module, NULL, lyxml_get_attr(yin->child, "name", NULL));
		if (!type->info.ident.ref) {
			return EXIT_FAILURE;
		}
		break;
	case LY_TYPE_INST:
		/* require-instance, 9.13.2
		 * - 0..1, true/false */
		break;
	case LY_TYPE_INT8:
	case LY_TYPE_INT16:
	case LY_TYPE_INT32:
	case LY_TYPE_INT64:
	case LY_TYPE_UINT8:
	case LY_TYPE_UINT16:
	case LY_TYPE_UINT32:
	case LY_TYPE_UINT64:
		/* range, 9.2.4
		 * - optional, 0..1, i rekurzivne - omezuje, string, podelementy*/
		break;
	case LY_TYPE_LEAFREF:
		/* path, 9.9.2
		 * - 1, nerekurzivni, string */
		break;
	case LY_TYPE_STRING:
		/* length, 9.4.4
		 * - optional, 0..1, rekurzivni - omezuje, string (podobne jako range), hodnoty se musi vejit do 64b, podelementy
		 * pattern, 9.4.6
		 * - optional, 0..n, rekurzivni - rozsiruje, string, podelementy */
		break;
	case LY_TYPE_UNION:
		/* type, 7.4
		 * - 1..n, nerekurzivni, resp rekurzivni pro union ale bez vazby na predky, nesmi byt empty nebo leafref */
		break;
	default:
		/* nothing needed :
		 * LY_TYPE_BOOL, LY_TYPE_EMPTY
		 */
		break;
	}

	return EXIT_SUCCESS;

error:

	while(root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

	return EXIT_FAILURE;
}

static int fill_yin_typedef(struct ly_module *module, struct ly_mnode *parent,
                            struct lyxml_elem *yin, struct ly_tpdf *tpdf)
{
	const char *value;
	struct lyxml_elem *node, *next;
	int r;

	value = lyxml_get_attr(yin, "name", NULL);
	tpdf->name = lydict_insert(module->ctx, value, strlen(value));

	LY_TREE_FOR_SAFE(yin->child, next, node) {
		if (!strcmp(node->name, "type")) {
			r = fill_yin_type(module, parent, node, &tpdf->type);

		/* optional statements */
		} else if (!strcmp(node->name, "description")) {
			tpdf->dsc = read_yin_text(module->ctx, node, "description");
			if (!tpdf->dsc) {
				r = 1;
			}
		} else if (!strcmp(node->name, "reference")) {
			tpdf->ref = read_yin_text(module->ctx, node, "reference");
			if (!tpdf->dsc) {
				r = 1;
			}
		} else if (!strcmp(node->name, "status")) {
			value = lyxml_get_attr(node, "value", NULL);
			if (!strcmp(value, "current")) {
				tpdf->flags |= LY_NODE_STATUS_CURR;
			} else if (!strcmp(value, "deprecated")) {
				tpdf->flags |= LY_NODE_STATUS_DEPRC;
			} else if (!strcmp(value, "obsolete")) {
				tpdf->flags |= LY_NODE_STATUS_OBSLT;
			} else {
				ly_verr(LY_VERR_UNEXP_VAL, value, "status");
				r = 1;
			}
		}
		lyxml_free_elem(module->ctx, node);
		if (r) {
			return EXIT_FAILURE;
		}
	}

	if (!tpdf->type.der) {
		ly_verr(LY_VERR_MISS_STMT2, "type", "typedef");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
 * Covers:
 * description, reference, status, optionaly config
 */
static int read_yin_common(struct ly_module *module, struct ly_mnode *parent,
		                   struct ly_mnode *mnode, struct lyxml_elem *xmlnode, int ext)
{
	const char *value;
	struct lyxml_elem *sub, *next;
	struct ly_ctx * const ctx = module->ctx;

	if (ext) {
		mnode->module = module;
	}

	value = lyxml_get_attr(xmlnode, "name", NULL);
	mnode->name = lydict_insert(ctx, value, strlen(value));
	if (!mnode->name || !mnode->name[0]) {
		ly_verr(LY_VERR_MISS_ARG, "name", xmlnode->name);
		return EXIT_FAILURE;
	}

	/* process local parameters */
	LY_TREE_FOR_SAFE(xmlnode->child, next, sub) {
		if (!strcmp(sub->name, "description")) {
			mnode->dsc = read_yin_text(ctx, sub, "description");
		} else if (!strcmp(sub->name, "reference")) {
			mnode->ref = read_yin_text(ctx, sub, "reference");
		} else if (!strcmp(sub->name, "status")) {
			value = lyxml_get_attr(sub, "value", NULL);
			if (!strcmp(value, "current")) {
				mnode->flags |= LY_NODE_STATUS_CURR;
			} else if (!strcmp(value, "deprecated")) {
				mnode->flags |= LY_NODE_STATUS_DEPRC;
			} else if (!strcmp(value, "obsolete")) {
				mnode->flags |= LY_NODE_STATUS_OBSLT;
			}
		} else if (ext && !strcmp(sub->name, "config")) {
			value = lyxml_get_attr(sub, "value", NULL);
			if (!strcmp(value, "false")) {
				mnode->flags |= LY_NODE_CONFIG_R;
			} else if (!strcmp(value, "false")) {
				mnode->flags |= LY_NODE_CONFIG_W;
			}
		} else {
			/* skip the lyxml_free_elem */
			continue;
		}
		lyxml_free_elem(ctx, sub);
	}

	if (ext && !(mnode->flags & LY_NODE_CONFIG_MASK)) {
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
	struct ly_mnode *retval, *r;
	struct ly_mnode_choice *choice;

	choice = calloc(1, sizeof *choice);
	choice->nodetype = LY_NODE_CHOICE;
	choice->module = module;
	choice->prev = (struct ly_mnode *)choice;
	retval = (struct ly_mnode *)choice;

	if (read_yin_common(module, parent, retval, node, 1)) {
		goto error;
	}

	/* process choice's specific children */
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "container")) {
			r = read_yin_container(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf-list")) {
			r = read_yin_leaflist(module, retval, sub);
		} else if (!strcmp(sub->name, "leaf")) {
			r = read_yin_leaf(module, retval, sub);
		} else if (!strcmp(sub->name, "list")) {
			r = read_yin_list(module, retval, sub);
		} else {
			continue;
		}
		lyxml_free_elem(ctx, sub);
		if (!r) {
			goto error;
		}
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
	int r;

	leaf = calloc(1, sizeof *leaf);
	leaf->nodetype = LY_NODE_LEAF;
	leaf->prev = (struct ly_mnode *)leaf;
	retval = (struct ly_mnode *)leaf;

	if (read_yin_common(module, parent, retval, node, 1)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "type")) {
			r = fill_yin_type(module, parent, sub, &leaf->type);
			if (r) {
				goto error;
			}
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
	int r;

	llist = calloc(1, sizeof *llist);
	llist->nodetype = LY_NODE_LEAFLIST;
	llist->prev = (struct ly_mnode *)llist;
	retval = (struct ly_mnode *)llist;

	if (read_yin_common(module, parent, retval, node, 1)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "type")) {
			r = fill_yin_type(module, parent, sub, &llist->type);
			if (r) {
				goto error;
			}
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
	struct ly_mnode_leaf *key;
	struct lyxml_elem *sub, *next, root = {0};
	int i, j, r;
	size_t len;
	int c_tpdf = 0;
	const char *key_str = NULL, *s;
	char *dup;

	list = calloc(1, sizeof *list);
	list->nodetype = LY_NODE_LIST;
	list->prev = (struct ly_mnode *)list;
	retval = (struct ly_mnode *)list;

	if (read_yin_common(module, parent, retval, node, 1)) {
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
				!strcmp(sub->name, "uses") ||
				!strcmp(sub->name, "grouping")) {
			lyxml_unlink_elem(sub);
			lyxml_add_child(&root, sub);

		/* array counters */
		} else if (!strcmp(sub->name, "key")) {
			/* check cardinality 0..1 */
			if (list->keys_size) {
				ly_verr(LY_VERR_TOOMANY, "key", list->name);
				goto error;
			}

			/* count the number of keys */
			key_str = s = lyxml_get_attr(sub, "value", NULL);
			if (!s) {
				ly_verr(LY_VERR_MISS_ARG, "value", "key");
				goto error;
			}
			while((s = strpbrk(s, " \t\n"))) {
				list->keys_size++;
				while(isspace(*s)) {
					s++;
				}
			}
			list->keys_size++;

			list->keys = calloc(list->keys_size, sizeof *list->keys);

		} else if (!strcmp(sub->name, "typedef")) {
			c_tpdf++;
		}
	}

	/* check - if list is configuration, key statement is mandatory */
	if ((list->flags & LY_NODE_CONFIG_W) && !list->keys_size) {
		ly_verr(LY_VERR_MISS_STMT2, "key", "list");
		goto error;
	}

	/* middle part - process nodes with cardinality of 0..n except the data nodes */
	if (c_tpdf) {
		list->tpdf_size = c_tpdf;
		list->tpdf = calloc(c_tpdf, sizeof *list->tpdf);
		c_tpdf = 0;
	}
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		if (!strcmp(sub->name, "typedef")) {
			r = fill_yin_typedef(module, retval, sub, &list->tpdf[c_tpdf]);
			c_tpdf++;

			if (r) {
				list->tpdf_size = c_tpdf;
				goto error;
			}
			lyxml_free_elem(module->ctx, sub);
		}
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
		} else if (!strcmp(sub->name, "uses")) {
			mnode = read_yin_uses(module, retval, sub, 1);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			/* TODO error */
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (!mnode) {
			goto error;
		}
	}

	/* link key leafs into the list structure and check all constraints  */
	/* TODO - include searching in uses/grouping */
	for (i = 0; i < list->keys_size; i++) {
		/* get the key name */
		if ((s = strpbrk(key_str, " \t\n"))) {
			len = s - key_str;
		} else {
			len = strlen(key_str);
		}
		LY_TREE_FOR(list->child, mnode) {
			if (!strncmp(mnode->name, key_str, len) && !mnode->name[len]) {
				list->keys[i] = mnode;
				break;
			}
		}
		key = (struct ly_mnode_leaf *)list->keys[i];

		/* existence */
		if (!key) {
			if ((s = strpbrk(key_str, " \t\n"))) {
				len = s - key_str;
				dup = strdup(key_str);
				dup[len] = '\0';
				key_str = dup;
			}
			ly_verr(LY_VERR_KEY_MISS, key_str);
			if (s) {
				free(dup);
			}
			goto error;
		}

		/* uniquness */
		for (j = i - 1; j >= 0; j--) {
			if (list->keys[i] == list->keys[j]) {
				ly_verr(LY_VERR_KEY_DUP, key->name, list->name);
				goto error;
			}
		}

		/* key is a leaf */
		if (key->nodetype != LY_NODE_LEAF) {
			ly_verr(LY_VERR_KEY_NLEAF, key->name, list->name);
			goto error;
		}

		/* type of the leaf is not built-in empty */
		if (key->type.base == LY_TYPE_EMPTY) {
			ly_verr(LY_VERR_KEY_TYPE, key->name, list->name);
			goto error;
		}

		/* config attribute is the same as of the list */
		if ((list->flags & LY_NODE_CONFIG_MASK) != (key->flags & LY_NODE_CONFIG_MASK)) {
			ly_verr(LY_VERR_KEY_CONFIG, key->name, list->name);
			goto error;
		}

		/* prepare for next iteration */
		while (s && isspace(*s)) {
			s++;
		}
		key_str = s;
	}


	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);
	while(root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

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
	int r;
	int c_tpdf = 0;

	cont = calloc(1, sizeof *cont);
	cont->nodetype = LY_NODE_CONTAINER;
	cont->prev = (struct ly_mnode *)cont;
	retval = (struct ly_mnode *)cont;

	if (read_yin_common(module, parent, retval, node, 1)) {
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
				!strcmp(sub->name, "uses") ||
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
			r = fill_yin_typedef(module, retval, sub, &cont->tpdf[c_tpdf]);
			c_tpdf++;

			if (r) {
				cont->tpdf_size = c_tpdf;
				goto error;
			}
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
		} else if (!strcmp(sub->name, "uses")) {
			mnode = read_yin_uses(module, retval, sub, 1);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			/* TODO error */
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (!mnode) {
			goto error;
		}
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);
	while (root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

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
	int r;
	int c_tpdf = 0;

	grp = calloc(1, sizeof *grp);
	grp->nodetype = LY_NODE_GROUPING;
	grp->module = module;
	grp->prev = (struct ly_mnode *)grp;
	retval = (struct ly_mnode *)grp;

	if (read_yin_common(module, parent, retval, node, 0)) {
		goto error;
	}

	LY_TREE_FOR_SAFE(node->child, next, sub) {
		/* data statements */
		if (!strcmp(sub->name, "container") ||
				!strcmp(sub->name, "leaf-list") ||
				!strcmp(sub->name, "leaf") ||
				!strcmp(sub->name, "list") ||
				!strcmp(sub->name, "choice") ||
				!strcmp(sub->name, "uses") ||
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
			r = fill_yin_typedef(module, retval, sub, &grp->tpdf[c_tpdf]);
			c_tpdf++;

			if (r) {
				grp->tpdf_size = c_tpdf;
				goto error;
			}
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
		} else if (!strcmp(sub->name, "uses")) {
			mnode = read_yin_uses(module, retval, sub, 0);
		} else if (!strcmp(sub->name, "grouping")) {
			mnode = read_yin_grouping(module, retval, sub);
		} else {
			/* TODO error */
			continue;
		}
		lyxml_free_elem(module->ctx, sub);

		if (!mnode) {
			goto error;
		}
	}

	ly_mnode_addchild(parent, retval);

	return retval;

error:

	ly_mnode_free(retval);
	while (root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}

	return NULL;
}

/*
 * resolve - referenced grouping should be bounded to the namespace (resolved)
 * only when uses does not appear in grouping. In a case of grouping's uses,
 * we just get information but we do not apply augment or refine to it.
 */
static struct ly_mnode *read_yin_uses(struct ly_module *module,
                                      struct ly_mnode *parent,
                                      struct lyxml_elem *node, int resolve)
{
	struct ly_mnode *retval;
	struct ly_mnode *mnode = NULL, *par;
	struct ly_mnode_uses *uses;
	struct ly_module *searchmod = NULL;
	const char *name;
	int prefix_len = 0;
	int i;

	uses = calloc(1, sizeof *uses);
	uses->nodetype = LY_NODE_USES;
	uses->module = module;
	uses->prev = (struct ly_mnode *)uses;
	retval = (struct ly_mnode *)uses;

	if (read_yin_common(module, parent, retval, node, 0)) {
		goto error;
	}

	/* get referenced grouping */
	name = strchr(uses->name, ':');
	if (!name) {
		/* no prefix, search in local tree */
		name = uses->name;
	} else {
		/* there is some prefix, check if it refer the same data model */

		/* set name to correct position after colon */
		prefix_len = name - uses->name;
		name++;

		if (!strncmp(uses->name, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
			/* prefix refers to the current module, ignore it */
			prefix_len = 0;
		}
	}

	/* search */
	if (prefix_len) {
		/* in top-level groupings of some other module */
		for (i = 0; i < module->imp_size; i++) {
			if (!strncmp(module->imp[i].prefix, uses->name, prefix_len)
					&& !module->imp[i].prefix[prefix_len]) {
				searchmod = module->imp[i].module;
				break;
			}
		}
		if (!searchmod) {
			/* uses refers unknown data model */
			ly_verr(LY_VERR_UNEXP_PREFIX, name);
			goto error;
		}

		LY_TREE_FOR(module->data, mnode) {
			if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
				uses->grp = (struct ly_mnode_grp *)mnode;
				break;
			}
		}

		if (!uses->grp) {
			ly_verr(LY_VERR_UNEXP_VAL, uses->name, "uses");
			goto error;
		}

	} else {
		/* in local tree hierarchy */
		for(par = parent; par; par = par->parent) {
			LY_TREE_FOR(parent->child, mnode) {
				if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
					uses->grp = (struct ly_mnode_grp *)mnode;
					break;
				}
			}
		}

		/* search in top level of the current module */
		LY_TREE_FOR(module->data, mnode) {
			if (mnode->nodetype == LY_NODE_GROUPING && !strcmp(mnode->name, name)) {
				uses->grp = (struct ly_mnode_grp *)mnode;
				break;
			}
		}
	}

	ly_mnode_addchild(parent, retval);

	if (!resolve) {
		return retval;
	}

	/* TODO */

	return retval;

error:

	ly_mnode_free(retval);

	return NULL;
}

struct ly_module *ly_read_yin(struct ly_ctx *ctx, const char *data)
{
	struct lyxml_elem *yin, *node, *next, *child, root = {0};
	struct ly_module *module = NULL, **newlist = NULL, *imp;
	struct ly_mnode *mnode = NULL;
	const char *value;
	int r;
	int i;
	/* counters */
	int c_imp = 0, c_rev = 0, c_tpdf = 0, c_ident = 0;

	yin = lyxml_read(ctx, data, 0);
	if (!yin) {
		return NULL;
	}

	/* check root element */
	if (!yin->name || strcmp(yin->name, "module")) {
		/* TODO: support submodules */
		ly_verr(LY_VERR_UNEXP_STMT, yin->name);
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
		} else if (!strcmp(node->name, "identity")) {
			c_ident++;

		/* data statements */
		} else if (!strcmp(node->name, "container") ||
				!strcmp(node->name, "leaf-list") ||
				!strcmp(node->name, "leaf") ||
				!strcmp(node->name, "list") ||
				!strcmp(node->name, "choice") ||
				!strcmp(node->name, "uses") ||
				!strcmp(node->name, "grouping")) {
			lyxml_unlink_elem(node);
			lyxml_add_child(&root, node);

		/* optional statements */
		} else if (!strcmp(node->name, "description")) {
			if (module->dsc) {
				ly_verr(LY_VERR_TOOMANY, "description", "module");
				goto error;
			}
			module->dsc = read_yin_text(ctx, node, "description");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "reference")) {
			if (module->ref) {
				ly_verr(LY_VERR_TOOMANY, "reference", "module");
				goto error;
			}
			module->ref = read_yin_text(ctx, node, "reference");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "organization")) {
			if (module->org) {
				ly_verr(LY_VERR_TOOMANY, "organization", "module");
				goto error;
			}
			module->org = read_yin_text(ctx, node, "organization");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "contact")) {
			if (module->contact) {
				ly_verr(LY_VERR_TOOMANY, "contact", "module");
				goto error;
			}
			module->contact = read_yin_text(ctx, node, "contact");
			lyxml_free_elem(ctx, node);
		} else if (!strcmp(node->name, "yang-version")) {
			/* TODO: support YANG 1.1 */
			if (module->version) {
				ly_verr(LY_VERR_TOOMANY, "yang-version", "module");
				goto error;
			}
			value = lyxml_get_attr(node, "value", NULL);
			if (strcmp(value, "1")) {
				ly_verr(LY_VERR_UNEXP_VAL, value, "yang-version");
				goto error;
			}
			module->version = 1;
			lyxml_free_elem(ctx, node);
		}
	}

	/* check for mandatory statements */
	if (!module->ns) {
		ly_verr(LY_VERR_MISS_STMT2, "namespace", "module");
		goto error;
	}
	if (!module->prefix) {
		ly_verr(LY_VERR_MISS_STMT2, "prefix", "module");
		goto error;
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
	if (c_ident) {
		module->ident_size = c_ident;
		module->ident = calloc(c_ident, sizeof *module->ident);
		c_ident = 0;
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
			r = fill_yin_typedef(module, NULL, node, &module->tpdf[c_tpdf]);
			c_tpdf++;

			if (r) {
				module->tpdf_size = c_tpdf;
				goto error;
			}
		} else if (!strcmp(node->name, "identity")) {
			r = fill_yin_identity(module, node, &module->ident[c_ident]);
			c_ident++;

			if (r) {
				module->ident_size = c_ident;
				goto error;
			}

		}

		lyxml_free_elem(ctx, node);
	}

	/* last part - process data nodes */
	LY_TREE_FOR_SAFE(root.child, next, node) {

		if (!strcmp(node->name, "container")) {
			mnode = read_yin_container(module, NULL, node);
		} else if (!strcmp(node->name, "leaf-list")) {
			mnode = read_yin_leaflist(module, NULL, node);
		} else if (!strcmp(node->name, "leaf")) {
			mnode = read_yin_leaf(module, NULL, node);
		} else if (!strcmp(node->name, "list")) {
			mnode = read_yin_list(module, NULL, node);
		} else if (!strcmp(node->name, "choice")) {
			mnode = read_yin_choice(module, NULL, node);
		} else if (!strcmp(node->name, "grouping")) {
			mnode = read_yin_grouping(module, NULL, node);
		} else if (!strcmp(node->name, "uses")) {
			mnode = read_yin_uses(module, NULL, node, 1);
		} else {
			/* TODO error */
			continue;
		}
		lyxml_free_elem(ctx, node);

		if (!mnode) {
			goto error;
		}

		/* include data element */
		if (module->data) {
			module->data->prev->next = mnode;
			mnode->prev = module->data->prev;
			module->data->prev = mnode;
		} else {
			module->data = mnode;
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
	while (root.child) {
		lyxml_free_elem(module->ctx, root.child);
	}
	lyxml_free_elem(ctx, yin);
	ly_model_free(module);

	return NULL;
}
