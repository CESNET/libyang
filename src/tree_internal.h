/**
 * @file tree_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang internal functions for manipulating with the data model and
 * data trees.
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
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is, and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#ifndef LY_TREE_INTERNAL_H_
#define LY_TREE_INTERNAL_H_

#include "tree.h"

/*
 * Unlink data model tree node from the tree.
 */
void ly_mnode_unlink(struct ly_mnode *node);

/*
 * Free data model tree node structure, includes unlinking from the tree
 */
void ly_mnode_free(struct ly_mnode *node);

void ly_restr_free(struct ly_ctx *ctx, struct ly_restr *restr);
void ly_type_free(struct ly_ctx *ctx, struct ly_type *type);
void ly_deviation_free(struct ly_ctx *ctx, struct ly_deviation *dev);
void ly_submodule_free(struct ly_submodule *submodule);

/*
 * Add child model node at the end of the parent's child list.
 * If the child is connected somewhere (has a parent), it is completely
 * unlinked and none of the following conditions applies.
 * If the child has prev sibling(s), they are ignored (child is added at the
 * end of the child list).
 * If the child has next sibling(s), all of them are connected with the parent.
 */
int ly_mnode_addchild(struct ly_mnode *parent, struct ly_mnode *child);

struct ly_mnode *ly_mnode_dup(struct ly_module *module, struct ly_mnode *mnode, uint8_t flags, int recursive, unsigned int line);

int resolve_uses(struct ly_mnode_uses *uses, unsigned int line);
struct ly_mnode *resolve_schema_nodeid(const char *id, struct ly_mnode *start, struct ly_module *mod, LY_NODE_TYPE node_type);
int resolve_path(struct lyd_node *data, const char *path, struct leafref_instid **ret);
int resolve_instid(struct lyd_node *data, const char *path, int path_len, struct leafref_instid **ret);
struct ly_mnode_leaf *find_leaf(struct ly_mnode *parent, const char *name, int len);
struct ly_ident *find_base_ident(struct ly_module *module, struct ly_ident *ident, const char *basename, int line, const char* parent);
struct ly_ident *find_identityref(struct ly_ident *base, const char *name, const char *ns);
struct ly_module *lys_read_import(struct ly_ctx *ctx, int fd, LY_MINFORMAT format);

/**
 * @brief Free (and unlink it from the context) the specified schema.
 *
 * It is dangerous to call this function on schemas already placed into the context's
 * list of modules - there can be many references from other modules and data instances.
 *
 * @param[in] module Data model to free.
 */
void lys_free(struct ly_module *module);

#endif /* LY_TREE_INTERNAL_H_ */
