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

#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "common.h"
#include "context.h"

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
		node->parent = NULL;
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
	node->parent = NULL;
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

	if (!parent || !child) {
		ly_errno = LY_EINVAL;
		return EXIT_FAILURE;
	}

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

API struct ly_module *ly_model_read(struct ly_ctx *ctx, const char *data,
                                    LY_MFORMAT format)
{
	if (!ctx || !data) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	switch (format) {
	case LY_YIN:
		return ly_read_yin(ctx, data);
	case LY_YANG:
	default:
		/* TODO */
		return NULL;
	}

	return NULL;
}

API struct ly_module *ly_model_read_fd(struct ly_ctx *ctx, int fd,
                                       LY_MFORMAT format)
{
	struct ly_module *module;
	struct stat sb;
	char *addr;

	if (!ctx || fd < 0) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	/*
	 * TODO
	 * This is just a temporary solution to make working automatic search for
	 * imported modules. This doesn't work e.g. for streams (stdin)
	 */
	fstat(fd, &sb);
	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	module = ly_model_read(ctx, addr, format);
	munmap(addr, sb.st_size);

	return module;
}

void ly_type_free(struct ly_ctx *ctx, struct ly_type *type)
{
	int i;

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

void ly_tpdf_free(struct ly_ctx *ctx, struct ly_tpdf *tpdf)
{
	lydict_remove(ctx, tpdf->name);
	lydict_remove(ctx, tpdf->dsc);
	lydict_remove(ctx, tpdf->ref);

	ly_type_free(ctx, &tpdf->type);
}

void ly_grp_free(struct ly_ctx *ctx, struct ly_mnode_grp *grp)
{
	int i;

	/* handle only specific parts for LY_NODE_GROUPING */
	if (grp->tpdf_size) {
		for (i = 0; i < grp->tpdf_size; i++) {
			ly_tpdf_free(ctx, &grp->tpdf[i]);
		}
		free(grp->tpdf);
	}
}

void ly_leaf_free(struct ly_ctx *ctx, struct ly_mnode_leaf *leaf)
{
	ly_type_free(ctx, &leaf->type);
}

void ly_leaflist_free(struct ly_ctx *ctx, struct ly_mnode_leaflist *llist)
{
	ly_type_free(ctx, &llist->type);
}

void ly_list_free(struct ly_ctx *ctx, struct ly_mnode_list *list)
{
	int i;

	/* handle only specific parts for LY_NODE_LIST */
	if (list->tpdf_size) {
		for (i = 0; i < list->tpdf_size; i++) {
			ly_tpdf_free(ctx, &list->tpdf[i]);
		}
		free(list->tpdf);
	}
}

void ly_container_free(struct ly_ctx *ctx, struct ly_mnode_container *cont)
{
	int i;

	/* handle only specific parts for LY_NODE_CONTAINER */
	if (cont->tpdf_size) {
		for (i = 0; i < cont->tpdf_size; i++) {
			ly_tpdf_free(ctx, &cont->tpdf[i]);
		}
		free(cont->tpdf);
	}
}

void ly_mnode_free(struct ly_mnode *node)
{
	struct ly_ctx *ctx;
	struct ly_mnode *sub, *next;

	if (!node) {
		return;
	}

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
		break;
	case LY_NODE_USES:
		break;
	case LY_NODE_GROUPING:
		ly_grp_free(ctx, (struct ly_mnode_grp *)node);
		break;
	}

	/* again common part */
	ly_mnode_unlink(node);
	free(node);
}

API void ly_model_free(struct ly_module *module)
{
	struct ly_ctx *ctx;
	struct ly_mnode *mnode;
	int i;

	if (!module) {
		return;
	}

	ctx = module->ctx;

	while(module->data) {
		mnode = module->data;
		module->data = mnode;
		ly_mnode_free(mnode);
	}

	lydict_remove(ctx, module->name);
	lydict_remove(ctx, module->ns);
	lydict_remove(ctx, module->prefix);
	lydict_remove(ctx, module->dsc);
	lydict_remove(ctx, module->ref);
	lydict_remove(ctx, module->org);
	lydict_remove(ctx, module->contact);

	if (module->rev_size) {
		for (i = 0; i < module->rev_size; i++) {
			lydict_remove(ctx, module->rev[i].dsc);
			lydict_remove(ctx, module->rev[i].ref);
		}
		free(module->rev);
	}

	if (module->imp_size) {
		free(module->imp);
	}

	if (module->tpdf_size) {
		for (i = 0; i < module->tpdf_size; i++) {
			ly_tpdf_free(ctx, &module->tpdf[i]);
		}
		free(module->tpdf);
	}

	free(module);
}


