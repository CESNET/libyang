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

/**
 * @brief Internal list of built-in types
 */
struct ly_types {
    LY_DATA_TYPE type;
    struct lys_tpdf *def;
};
extern struct ly_types ly_types[LY_DATA_TYPE_COUNT];

/**
 * @brief Unresolved leafref or instance-identifier in DATA
 */
struct unres_data {
    uint8_t is_leafref;
    struct lyd_node *dnode;
    struct unres_data *next;
#ifndef NDEBUG
    uint32_t line;
#endif
};

/**
 * @brief Type of an unresolved item in a SCHEMA
 */
enum UNRES_ITEM {
    UNRES_RESOLVED,      /* a resolved item */
    UNRES_IDENT,         /* unresolved derived identities */
    UNRES_TYPE_IDENTREF, /* check identityref value */
    UNRES_TYPE_LEAFREF,  /* check leafref value */
    UNRES_TYPE_DER,      /* unresolved derived type */
    UNRES_AUGMENT,       /* unresolved augments (standalone only) */
    UNRES_IFFEAT,        /* unresolved if-feature */
    UNRES_USES,          /* unresolved uses grouping (refines and augments in it are resolved as well) */
    UNRES_TYPE_DFLT,     /* validate default type value */
    UNRES_CHOICE_DFLT,   /* check choice default case */
    UNRES_LIST_KEYS,     /* list keys */
    UNRES_LIST_UNIQ,     /* list uniques */
    UNRES_WHEN,          /* check when */
    UNRES_MUST           /* check must */
};

/**
 * @brief Unresolved items in a SCHEMA
 */
struct unres_schema {
    void **item;
    enum UNRES_ITEM *type;
    void **str_mnode;
#ifndef NDEBUG
    uint32_t *line;
#endif
    uint32_t count;
};

/*
 * Unlink data model tree node from the tree.
 */
void ly_mnode_unlink(struct lys_node *node);

/*
 * Free data model tree node structure, includes unlinking from the tree
 */
void ly_mnode_free(struct lys_node *node);

void ly_restr_free(struct ly_ctx *ctx, struct lys_restr *restr);
void ly_type_free(struct ly_ctx *ctx, struct lys_type *type);
void ly_deviation_free(struct ly_ctx *ctx, struct lys_deviation *dev);
void ly_submodule_free(struct lys_submodule *submodule);

struct lys_submodule *ly_submodule_read(struct lys_module *module, const char *data, LYS_INFORMAT format, int implement);
struct lys_submodule *ly_submodule_read_fd(struct lys_module *module, int fd, LYS_INFORMAT format, int implement);

/*
 * Add child model node at the end of the parent's child list.
 * If the child is connected somewhere (has a parent), it is completely
 * unlinked and none of the following conditions applies.
 * If the child has prev sibling(s), they are ignored (child is added at the
 * end of the child list).
 * If the child has next sibling(s), all of them are connected with the parent.
 */
int ly_mnode_addchild(struct lys_node *parent, struct lys_node *child);

struct lys_module *lys_read_import(struct ly_ctx *ctx, int fd, LYS_INFORMAT format);

/**
 * @brief Free (and unlink it from the context) the specified schema.
 *
 * It is dangerous to call this function on schemas already placed into the context's
 * list of modules - there can be many references from other modules and data instances.
 *
 * @param[in] module Data model to free.
 */
void lys_free(struct lys_module *module);

struct lys_node *ly_mnode_dup(struct lys_module *module, struct lys_node *mnode, uint8_t flags, int recursive,
                              struct unres_schema *unres);

#endif /* LY_TREE_INTERNAL_H_ */
