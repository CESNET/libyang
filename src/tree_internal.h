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
 */

#ifndef LY_TREE_INTERNAL_H_
#define LY_TREE_INTERNAL_H_

#include "tree.h"

#define LY_INTERNAL_MODULE_COUNT 3

/**
 * @brief Internal list of internal modules that are a part
 *        of every context and must never be freed. Structure
 *        instance defined in "tree.c".
 */
struct internal_modules {
    const struct {
        const char *name;
        const char *revision;
    } modules[LY_INTERNAL_MODULE_COUNT];
    const uint8_t count;
};

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
    void **item;            /* array of pointers, each is determined by the type (one of lys_* structures) */
    enum UNRES_ITEM *type;  /* array of unres types */
    void **str_snode;       /* array of pointers, each is determined by the type (a string, a lys_node *, or NULL) */
#ifndef NDEBUG
    uint32_t *line;         /* array of lines for each unres item */
#endif
    uint32_t count;         /* count of unres items */
};

/**
 * @brief Create submodule structure by reading data from memory.
 *
 * @param[in] module Schema tree where to connect the submodule, belongs-to value must match.
 * @param[in] data String containing the submodule specification in the given \p format.
 * @param[in] format Format of the data to read.
 * @param[in] implement Flag to distinguish implemented and just imported (sub)modules.
 * @return Created submodule structure or NULL in case of error.
 */
struct lys_submodule *lys_submodule_parse(struct lys_module *module, const char *data, LYS_INFORMAT format, int implement);

/**
 * @brief Create submodule structure by reading data from file descriptor.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] module Schema tree where to connect the submodule, belongs-to value must match.
 * @param[in] fd Standard file descriptor of the file containing the submodule specification in the given \p format.
 * @param[in] format Format of the data to read.
 * @param[in] implement Flag to distinguish implemented and just imported (sub)modules.
 * @return Created submodule structure or NULL in case of error.
 */
struct lys_submodule *lys_submodule_read(struct lys_module *module, int fd, LYS_INFORMAT format, int implement);

/**
 * @brief Free the submodule structure
 *
 * @param[in] submodule The structure to free. Do not use the pointer after calling this function.
 */
void lys_submodule_free(struct lys_submodule *submodule);

/**
 * @brief Add child schema tree node at the end of the parent's child list.
 *
 * If the child is connected somewhere (has a parent), it is completely
 * unlinked and none of the following conditions applies.
 * If the child has prev sibling(s), they are ignored (child is added at the
 * end of the child list).
 * If the child has next sibling(s), all of them are connected with the parent.
 *
 * @param[in] parent Parent node where the \p child will be added.
 * @param[in] module Module where the \p child will be added if the \p parent
 * parameter is NULL (case of top-level elements). The parameter does not change
 * the module of the \p child element. If the \p parent parameter is present,
 * the \p module parameter is ignored.
 * @param[in] child The schema tree node to be added.
 * @return 0 on success, nonzero else
 */
int lys_node_addchild(struct lys_node *parent, struct lys_module *module, struct lys_node *child);

/**
 * @brief Check that the \p node being connected into the \p parent has a unique name (identifier).
 *
 * Function is performed also as part of lys_node_addchild().
 *
 * @param[in] node The schema tree node to be checked.
 * @param[in] parent Parent node where the \p child is supposed to be added.
 * @param[in] module Module where the \p child is supposed to be added if the \p parent
 * parameter is NULL (case of top-level elements). The parameter does not change
 * the module of the \p child element. If the \p parent parameter is present,
 * the \p module parameter is ignored.
 * @return 0 on success, nonzero else
 */
int
lys_check_id(struct lys_node *node, struct lys_node *parent, struct lys_module *module);

/**
 * @brief Create a copy of the specified schema tree \p node
 *
 * @param[in] module Target module for the duplicated node.
 * @param[in] node Schema tree node to be duplicated.
 * @param[in] flags Config flag to be inherited in case the origin node does not specify config flag
 * @param[in] nacm NACM flags to be inherited from the parent
 * @param[in] recursive 1 if all children are supposed to be also duplicated.
 * @param[in] unres TODO provide description
 * @return Created copy of the provided schema \p node.
 */
struct lys_node *lys_node_dup(struct lys_module *module, struct lys_node *node, uint8_t flags, uint8_t nacm,
                              int recursive, struct unres_schema *unres);

/**
 * @brief Free the schema tree restriction (must, ...) structure content
 *
 * @param[in] ctx libyang context where the schema of the restriction is used.
 * @param[in] restr The restriction structure to free. The function actually frees only
 * the content of the structure, so after using this function, caller is supposed to
 * use free(restr). It is done to free the content of structures being allocated as
 * part of array, in that case the free() is used on the whole array.
 */
void lys_restr_free(struct ly_ctx *ctx, struct lys_restr *restr);

/**
 * @brief Free the schema tree type structure content
 *
 * @param[in] ctx libyang context where the schema of the type is used.
 * @param[in] restr The type structure to free. The function actually frees only
 * the content of the structure, so after using this function, caller is supposed to
 * use free(type). It is done to free the content of structures being allocated as
 * part of array, in that case the free() is used on the whole array.
 */
void lys_type_free(struct ly_ctx *ctx, struct lys_type *type);

/**
 * @brief Unlink the schema node from the tree.
 *
 * @param[in] node Schema tree node to unlink.
 */
void lys_node_unlink(struct lys_node *node);

/**
 * @brief Free the schema node structure, includes unlinking it from the tree
 *
 * @param[in] node Schema tree node to free. Do not use the pointer after calling this function.
 */
void lys_node_free(struct lys_node *node);

/**
 * @brief Free (and unlink it from the context) the specified schema.
 *
 * It is dangerous to call this function on schemas already placed into the context's
 * list of modules - there can be many references from other modules and data instances.
 *
 * @param[in] module Data model to free.
 */
void lys_free(struct lys_module *module);

/**
 * @brief Search for a mandatory element in the given schema tree subtree
 *
 * @param[in] start Root node for the searching subtree. Expecting that in a data tree
 * instance, the start is the first node that does not have its instance (the start's
 * parent has its instance in the data tree).
 * @return The first mandatory element definition, NULL if there is no mandatory element
 * in the subtree. TODO
 */
struct lys_node *ly_check_mandatory(struct lyd_node *start);

#endif /* LY_TREE_INTERNAL_H_ */
