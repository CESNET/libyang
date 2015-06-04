/**
 * @file tree.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang data structures
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

#include <assert.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "parser.h"
#include "tree_internal.h"

void ly_submodule_free(struct ly_submodule *submodule);

struct ly_mnode_leaf *find_leaf(struct ly_mnode *parent, const char *name, int len)
{
	struct ly_mnode *child;
	struct ly_mnode_leaf *result;

	if (!len) {
		len = strlen(name);
	}

	LY_TREE_FOR(parent->child, child) {
		switch (child->nodetype) {
		case LY_NODE_LEAF:
			/* direct check */
			if (child->name == name || !strcmp(child->name, name)) {
				return (struct ly_mnode_leaf *)child;
			}
			break;
		case LY_NODE_USES:
			/* search recursively */
			result = find_leaf(child, name, len);
			if (result) {
				return result;
			}
			break;
		default:
			/* ignore */
			break;
		}
	}

	return NULL;
}

void ly_mnode_unlink(struct ly_mnode *node)
{
	struct ly_mnode *parent, *first;

	if (!node) {
		return;
	}

	/* unlink from data model if necessary */
	if (node->module) {
		if (node->module->data == node) {
			node->module->data = node->next;
		}
	}

	/* store pointers to important nodes */
	parent = node->parent;

	/* unlink from parent */
	if (parent) {
		if (parent->child == node) {
			parent->child = node->next;
		}
		node->parent = NULL;
	}

	/* unlink from siblings */
	if (node->prev == node) {
		/* there are no more siblings */
		return;
	}
	if (node->next) {
		node->next->prev = node->prev;
	} else {
		/* unlinking the last element */
		if (parent) {
			first = parent->child;
		} else {
			first = node;
			while (node->prev->next) {
				first = node->prev;
			}
		}
		first->prev = node->prev;
	}
	if (node->prev->next) {
		node->prev->next = node->next;
	}

	/* clean up the unlinked element */
	node->next = NULL;
	node->prev = node;
}

/*
 * Add child model node at the end of the parent's child list.
 * If the child is connected somewhere (has a parent), it is completely
 * unlinked and none of the following conditions applies.
 * If the child has prev sibling(s), they are ignored (child is added at the
 * end of the child list).
 * If the child has next sibling(s), all of them are connected with the parent.
 */
int ly_mnode_addchild(struct ly_mnode *parent, struct ly_mnode *child)
{
	struct ly_mnode *last;

	assert(parent);
	assert(child);

	if (child->parent) {
		ly_mnode_unlink(child);
	}

	if (!parent->child) {
		/* the only/first child of the parent */
		parent->child = child;
		child->parent = parent;
		return EXIT_SUCCESS;
	}

	/* add a new child at the end of parent's child list */
	last = parent->child->prev;
	last->next = child;
	child->prev = last;
	while (last->next) {
		last = last->next;
		last->parent = parent;
	}
	parent->child->prev = last;

	return EXIT_SUCCESS;
}

int resolve_uses(struct ly_mnode_uses *uses)
{
	struct ly_mnode *mnode = NULL, *mnode_aux;

	/* copy the data nodes from grouping into the uses context */
	LY_TREE_FOR(uses->grp->child, mnode) {
		mnode_aux = ly_mnode_dup(uses->module, mnode, uses->flags, 1);
		if (!mnode_aux) {
			return EXIT_FAILURE;
		}
		ly_mnode_addchild((struct ly_mnode *)uses, mnode_aux);

		if (mnode_aux->nodetype == LY_NODE_USES) {
			if (resolve_uses((struct ly_mnode_uses *)mnode_aux)) {
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}

API struct ly_module *ly_module_read(struct ly_ctx *ctx, const char *data,
                                    LY_MINFORMAT format)
{
	if (!ctx || !data) {
		LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
		return NULL;
	}

	switch (format) {
	case LY_IN_YIN:
		return yin_read_module(ctx, data);
	case LY_IN_YANG:
	default:
		/* TODO */
		return NULL;
	}

	return NULL;
}

struct ly_submodule *ly_submodule_read(struct ly_module *module,
                                       const char *data, LY_MINFORMAT format)
{
	assert(module);
	assert(data);

	switch (format) {
	case LY_IN_YIN:
		return yin_read_submodule(module, data);
	case LY_IN_YANG:
	default:
		/* TODO */
		return NULL;
	}

	return NULL;
}

API struct ly_module *ly_module_read_fd(struct ly_ctx *ctx, int fd,
                                       LY_MINFORMAT format)
{
	struct ly_module *module;
	struct stat sb;
	char *addr;

	if (!ctx || fd < 0) {
		LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
		return NULL;
	}

	/*
	 * TODO
	 * This is just a temporary solution to make working automatic search for
	 * imported modules. This doesn't work e.g. for streams (stdin)
	 */
	fstat(fd, &sb);
	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	module = ly_module_read(ctx, addr, format);
	munmap(addr, sb.st_size);

	return module;
}

struct ly_submodule *ly_submodule_read_fd(struct ly_module *module, int fd,
                                          LY_MINFORMAT format)
{
	struct ly_submodule *submodule;
	struct stat sb;
	char *addr;

	assert(module);
	assert(fd >= 0);

	/*
	 * TODO
	 * This is just a temporary solution to make working automatic search for
	 * imported modules. This doesn't work e.g. for streams (stdin)
	 */
	fstat(fd, &sb);
	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	/* TODO addr error check */
	submodule = ly_submodule_read(module, addr, format);
	munmap(addr, sb.st_size);

	return submodule;

}

void ly_type_dup(struct ly_ctx *ctx, struct ly_type *new, struct ly_type *old)
{
	int i;

	new->prefix = lydict_insert(ctx, old->prefix, 0);
	new->base = old->base;
	new->der = old->der;

	switch(new->base) {
	case LY_TYPE_ENUM:
		new->info.enums.count = old->info.enums.count;
		if (new->info.enums.count) {
			new->info.enums.list = calloc(new->info.enums.count, sizeof *new->info.enums.list);
			for (i = 0; i < new->info.enums.count; i++) {
				new->info.enums.list[i].name = lydict_insert(ctx, old->info.enums.list[i].name, 0);
				new->info.enums.list[i].dsc = lydict_insert(ctx, old->info.enums.list[i].dsc, 0);
				new->info.enums.list[i].ref = lydict_insert(ctx, old->info.enums.list[i].ref, 0);
			}
		}
		break;
	default:
		/* TODO */
		break;
	}
}

void ly_type_free(struct ly_ctx *ctx, struct ly_type *type)
{
	int i;

	assert(ctx);
	if (!type) {
		return;
	}

	lydict_remove(ctx, type->prefix);

	switch(type->base) {
	case LY_TYPE_ENUM:
		for (i = 0; i < type->info.enums.count; i++) {
			lydict_remove(ctx, type->info.enums.list[i].name);
			lydict_remove(ctx, type->info.enums.list[i].dsc);
			lydict_remove(ctx, type->info.enums.list[i].ref);
		}
		free(type->info.enums.list);
		break;
	default:
		/* TODO */
		break;
	}
}

struct ly_tpdf *ly_tpdf_dup(struct ly_ctx *ctx, struct ly_tpdf *old, int size)
{
	struct ly_tpdf *result;
	int i;

	if (!size) {
		return NULL;
	}

	result = calloc(size, sizeof *result);
	for (i = 0; i < size; i++) {
		result[i].name = lydict_insert(ctx, old[i].name, 0);
		result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
		result[i].ref = lydict_insert(ctx, old[i].ref, 0);
		result[i].flags = old[i].flags;
		result[i].module = old[i].module;

		ly_type_dup(ctx, &(result[i].type), &(old[i].type));

		result[i].dflt = lydict_insert(ctx, old[i].dflt, 0);
		result[i].units = lydict_insert(ctx, old[i].units, 0);
	}

	return result;
}

void ly_tpdf_free(struct ly_ctx *ctx, struct ly_tpdf *tpdf)
{
	assert(ctx);
	if (!tpdf) {
		return;
	}

	lydict_remove(ctx, tpdf->name);
	lydict_remove(ctx, tpdf->dsc);
	lydict_remove(ctx, tpdf->ref);

	ly_type_free(ctx, &tpdf->type);

	lydict_remove(ctx, tpdf->units);
	lydict_remove(ctx, tpdf->dflt);
}

struct ly_must *ly_must_dup(struct ly_ctx *ctx, struct ly_must *old, int size)
{
	struct ly_must *result;
	int i;

	if (!size) {
		return NULL;
	}

	result = calloc(size, sizeof *result);
	for (i = 0; i < size; i++) {
		result[i].cond = lydict_insert(ctx, old[i].cond, 0);
		result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
		result[i].ref = lydict_insert(ctx, old[i].ref, 0);
		result[i].eapptag = lydict_insert(ctx, old[i].eapptag, 0);
		result[i].emsg = lydict_insert(ctx, old[i].emsg, 0);
	}

	return result;
}

void ly_must_free(struct ly_ctx *ctx, struct ly_must *must)
{
	assert(ctx);
	if (!must) {
		return;
	}

	lydict_remove(ctx, must->cond);
	lydict_remove(ctx, must->dsc);
	lydict_remove(ctx, must->ref);
	lydict_remove(ctx, must->eapptag);
	lydict_remove(ctx, must->emsg);
}

void ly_ident_free(struct ly_ctx *ctx, struct ly_ident *ident)
{
	struct ly_ident_der *der;

	assert(ctx);
	if (!ident) {
		return;
	}

	/*
	 * TODO
	 * if caller free only a single data model which is used (its identity is
	 * reference from identity in another module), this silly freeing can lead
	 * to segmentation fault. But without noting if the module is used by some
	 * other, it cannot be solved.
	 */
	while(ident->der) {
		der = ident->der;
		ident->der = der->next;
		free(der);
	}

	lydict_remove(ctx, ident->name);
	lydict_remove(ctx, ident->dsc);
	lydict_remove(ctx, ident->ref);

}

void ly_grp_free(struct ly_ctx *ctx, struct ly_mnode_grp *grp)
{
	int i;

	/* handle only specific parts for LY_NODE_GROUPING */
	for (i = 0; i < grp->tpdf_size; i++) {
		ly_tpdf_free(ctx, &grp->tpdf[i]);
	}
	free(grp->tpdf);
}

void ly_anyxml_free(struct ly_ctx *ctx, struct ly_mnode_anyxml *anyxml)
{
	int i;

	for (i = 0; i < anyxml->must_size; i++) {
		ly_must_free(ctx, &anyxml->must[i]);
	}
	free(anyxml->must);
}

void ly_leaf_free(struct ly_ctx *ctx, struct ly_mnode_leaf *leaf)
{
	int i;

	for (i = 0; i < leaf->must_size; i++) {
		ly_must_free(ctx, &leaf->must[i]);
	}
	free(leaf->must);

	ly_type_free(ctx, &leaf->type);
	lydict_remove(ctx, leaf->units);
	lydict_remove(ctx, leaf->dflt);
}

void ly_leaflist_free(struct ly_ctx *ctx, struct ly_mnode_leaflist *llist)
{
	int i;

	for (i = 0; i < llist->must_size; i++) {
		ly_must_free(ctx, &llist->must[i]);
	}
	free(llist->must);

	ly_type_free(ctx, &llist->type);
	lydict_remove(ctx, llist->units);
}

void ly_list_free(struct ly_ctx *ctx, struct ly_mnode_list *list)
{
	int i;

	/* handle only specific parts for LY_NODE_LIST */
	for (i = 0; i < list->tpdf_size; i++) {
		ly_tpdf_free(ctx, &list->tpdf[i]);
	}
	free(list->tpdf);

	for (i = 0; i < list->must_size; i++) {
		ly_must_free(ctx, &list->must[i]);
	}
	free(list->must);

	for (i = 0; i < list->unique_size; i++) {
		free(list->unique[i].leafs);
	}
	free(list->unique);

	free(list->keys);
}

void ly_container_free(struct ly_ctx *ctx, struct ly_mnode_container *cont)
{
	int i;

	/* handle only specific parts for LY_NODE_CONTAINER */
	lydict_remove(ctx, cont->presence);

	for (i = 0; i < cont->tpdf_size; i++) {
		ly_tpdf_free(ctx, &cont->tpdf[i]);
	}
	free(cont->tpdf);

	for (i = 0; i < cont->must_size; i++) {
		ly_must_free(ctx, &cont->must[i]);
	}
	free(cont->must);
}

void ly_mnode_free(struct ly_mnode *node)
{
	struct ly_ctx *ctx;
	struct ly_mnode *sub, *next;

	if (!node) {
		return;
	}

	assert(node->module);
	assert(node->module->ctx);

	ctx = node->module->ctx;

	/* common part */
	LY_TREE_FOR_SAFE(node->child, next, sub) {
		ly_mnode_free(sub);
	}

	lydict_remove(ctx, node->name);
	lydict_remove(ctx, node->dsc);
	lydict_remove(ctx, node->ref);

	/* specific part */
	switch(node->nodetype) {
	case LY_NODE_CONTAINER:
		ly_container_free(ctx, (struct ly_mnode_container *)node);
		break;
	case LY_NODE_CHOICE:
		break;
	case LY_NODE_LEAF:
		ly_leaf_free(ctx, (struct ly_mnode_leaf *)node);
		break;
	case LY_NODE_LEAFLIST:
		ly_leaflist_free(ctx, (struct ly_mnode_leaflist *)node);
		break;
	case LY_NODE_LIST:
		ly_list_free(ctx, (struct ly_mnode_list *)node);
		break;
	case LY_NODE_ANYXML:
		ly_anyxml_free(ctx, (struct ly_mnode_anyxml *)node);
		break;
	case LY_NODE_USES:
		break;
	case LY_NODE_GROUPING:
		ly_grp_free(ctx, (struct ly_mnode_grp *)node);
		break;
	case LY_NODE_CASE:
		break;
	}

	/* again common part */
	ly_mnode_unlink(node);
	free(node);
}

static void module_free_common(struct ly_module *module)
{
	struct ly_ctx *ctx;
	struct ly_mnode *mnode;
	unsigned int i;

	assert(module->ctx);
	ctx = module->ctx;

	while (module->data) {
		mnode = module->data;
		module->data = mnode;
		ly_mnode_free(mnode);
	}

	lydict_remove(ctx, module->dsc);
	lydict_remove(ctx, module->ref);
	lydict_remove(ctx, module->org);
	lydict_remove(ctx, module->contact);

	for (i = 0; i < module->rev_size; i++) {
		lydict_remove(ctx, module->rev[i].dsc);
		lydict_remove(ctx, module->rev[i].ref);
	}
	if (module->rev) {
		free(module->rev);
	}

	for (i = 0; i < module->ident_size; i++) {
		ly_ident_free(ctx, &module->ident[i]);
	}
	module->ident_size = 0;
	if (module->ident) {
		free(module->ident);
	}

	for (i = 0; i < module->tpdf_size; i++) {
		ly_tpdf_free(ctx, &module->tpdf[i]);
	}
	if (module->tpdf) {
		free(module->tpdf);
	}

	if (module->imp) {
		free(module->imp);
	}

	for (i = 0; i < module->inc_size; i++) {
		ly_submodule_free(module->inc[i].submodule);
	}
	if (module->inc) {
		free(module->inc);
	}

	lydict_remove(ctx, module->name);
}

void ly_submodule_free(struct ly_submodule *submodule)
{
	if (!submodule) {
		return;
	}

	/* common part with struct ly_module */
	module_free_common((struct ly_module *)submodule);

	/* no specific items to free */

	free(submodule);
}

struct ly_mnode *ly_mnode_dup(struct ly_module *module, struct ly_mnode *mnode, uint8_t flags, int recursive)
{
	struct ly_mnode *retval = NULL, *aux, *child;
	struct ly_ctx *ctx = module->ctx;
	int i, j;

	struct ly_mnode_container *cont;
	struct ly_mnode_container *cont_orig = (struct ly_mnode_container *)mnode;
	struct ly_mnode_choice *choice;
	struct ly_mnode_choice *choice_orig = (struct ly_mnode_choice *)mnode;
	struct ly_mnode_leaf *leaf;
	struct ly_mnode_leaf *leaf_orig = (struct ly_mnode_leaf *)mnode;
	struct ly_mnode_leaflist *llist;
	struct ly_mnode_leaflist *llist_orig = (struct ly_mnode_leaflist *)mnode;
	struct ly_mnode_list *list;
	struct ly_mnode_list *list_orig = (struct ly_mnode_list *)mnode;
	struct ly_mnode_anyxml *anyxml;
	struct ly_mnode_anyxml *anyxml_orig = (struct ly_mnode_anyxml *)mnode;
	struct ly_mnode_uses *uses;
	struct ly_mnode_uses *uses_orig = (struct ly_mnode_uses *)mnode;
	struct ly_mnode_grp *grp;
	struct ly_mnode_grp *grp_orig = (struct ly_mnode_grp *)mnode;
	struct ly_mnode_case *cs;

	/* we cannot just duplicate memory since the strings are stored in
	 * dictionary and we need to update dictionary counters.
	 */

	switch(mnode->nodetype) {
	case LY_NODE_CONTAINER:
		cont = calloc(1, sizeof *cont);
		retval = (struct ly_mnode *)cont;
		break;
	case LY_NODE_CHOICE:
		choice = calloc(1, sizeof *choice);
		retval = (struct ly_mnode *)choice;
		break;
	case LY_NODE_LEAF:
		leaf = calloc(1, sizeof *leaf);
		retval = (struct ly_mnode *)leaf;
		break;
	case LY_NODE_LEAFLIST:
		llist = calloc(1, sizeof *llist);
		retval = (struct ly_mnode *)llist;
		break;
	case LY_NODE_LIST:
		list = calloc(1, sizeof *list);
		retval = (struct ly_mnode *)list;
		break;
	case LY_NODE_ANYXML:
		anyxml = calloc(1, sizeof *anyxml);
		retval = (struct ly_mnode *)anyxml;
		break;
	case LY_NODE_USES:
		uses = calloc(1, sizeof *uses);
		retval = (struct ly_mnode *)uses;
		break;
	case LY_NODE_GROUPING:
		grp = calloc(1, sizeof *grp);
		retval = (struct ly_mnode *)grp;
		break;
	case LY_NODE_CASE:
		cs = calloc(1, sizeof *cs);
		retval = (struct ly_mnode *)cs;
		break;
	}

	/*
	 * duplicate generic part of the structure
	 */
	retval->name = lydict_insert(ctx, mnode->name, 0);
	retval->dsc = lydict_insert(ctx, mnode->dsc, 0);
	retval->ref = lydict_insert(ctx, mnode->ref, 0);
	retval->flags = mnode->flags;
	if (!(retval->flags & LY_NODE_CONFIG_MASK)) {
		/* set parent's config flag */
		retval->flags |= flags & LY_NODE_CONFIG_MASK;
	}

	retval->module = module;
	retval->nodetype = mnode->nodetype;

	retval->prev = retval;

	retval->feature = NULL; /* TODO */
	retval->when = NULL; /* TODO */

	if (recursive) {
		/* go recursively */
		LY_TREE_FOR(mnode->child, child) {
			aux = ly_mnode_dup(module, child, retval->flags, 1);
			if (!aux) {
				goto error;
			}
			ly_mnode_addchild(retval, aux);
		}
	}

	/*
	 * duplicate specific part of the structure
	 */
	switch(mnode->nodetype) {
	case LY_NODE_CONTAINER:
		cont->presence = lydict_insert(ctx, cont_orig->presence, 0);

		cont->must_size = cont_orig->must_size;
		cont->tpdf_size = cont_orig->tpdf_size;

		cont->must = ly_must_dup(ctx, cont_orig->must, cont->must_size);
		cont->tpdf = ly_tpdf_dup(ctx, cont_orig->tpdf, cont->tpdf_size);
		break;
	case LY_NODE_CHOICE:
		if (choice->dflt) {
			LY_TREE_FOR(choice->child, child) {
				if (child->name == choice_orig->dflt->name) {
					choice->dflt = child;
					break;
				}
			}
		}
		break;
	case LY_NODE_LEAF:
		ly_type_dup(ctx, &(leaf->type), &(leaf_orig->type));
		leaf->units = lydict_insert(ctx, leaf_orig->units, 0);
		leaf->dflt = lydict_insert(ctx, leaf_orig->dflt, 0);

		leaf->must_size = leaf_orig->must_size;
		leaf->must = ly_must_dup(ctx, leaf_orig->must, leaf->must_size);
		break;
	case LY_NODE_LEAFLIST:

		ly_type_dup(ctx, &(llist->type), &(llist_orig->type));
		llist->units = lydict_insert(ctx, llist_orig->units, 0);

		llist->min = llist_orig->min;
		llist->max = llist_orig->max;

		llist->must_size = llist_orig->must_size;
		llist->must = ly_must_dup(ctx, llist_orig->must, llist->must_size);
		break;
	case LY_NODE_LIST:
		list->min = list_orig->min;
		list->max = list_orig->max;

		list->must_size = list_orig->must_size;
		list->tpdf_size = list_orig->tpdf_size;
		list->keys_size = list_orig->keys_size;
		list->unique_size = list_orig->unique_size;

		list->must = ly_must_dup(ctx, list_orig->must, list->must_size);
		list->tpdf = ly_tpdf_dup(ctx, list_orig->tpdf, list->tpdf_size);

		if (list->keys_size) {
			list->keys = calloc(list->keys_size, sizeof *list->keys);
			for (i = 0; i < list->keys_size; i++) {
				list->keys[i] = find_leaf(retval, list_orig->keys[i]->name, 0);
			}
		}
		if (list->unique_size) {
			list->unique = calloc(list->unique_size, sizeof *list->unique);
			for (i = 0; i < list->unique_size; i++) {
				list->unique[i].leafs = calloc(list->unique[i].leafs_size, sizeof *list->unique[i].leafs);
				for (j = 0; j < list->unique[i].leafs_size; j++) {
					list->unique[i].leafs[j] = find_leaf(retval, list_orig->unique[i].leafs[j]->name, 0);
				}
			}
		}
		break;
	case LY_NODE_ANYXML:
		anyxml->must_size = anyxml_orig->must_size;
		anyxml->must = ly_must_dup(ctx, anyxml_orig->must, anyxml->must_size);
		break;
	case LY_NODE_USES:
		uses->grp = uses_orig->grp;
		if (resolve_uses(uses)) {
			goto error;
		}
		break;
	case LY_NODE_GROUPING:
		grp->tpdf_size = grp_orig->tpdf_size;
		grp->tpdf = ly_tpdf_dup(ctx, grp_orig->tpdf, grp->tpdf_size);
		break;
	case LY_NODE_CASE:
		/* nothing to do */
		break;
	}


	return retval;

error:
	LOGDBG("error: %s:%d", __FILE__, __LINE__);

	ly_mnode_free(retval);
	return NULL;
}

API void ly_module_free(struct ly_module *module)
{
	if (!module) {
		return;
	}

	/* common part with struct ly_submodule */
	module_free_common(module);

	/* specific items to free */
	lydict_remove(module->ctx, module->ns);
	lydict_remove(module->ctx, module->prefix);

	free(module);
}


