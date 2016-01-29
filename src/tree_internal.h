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

#include "tree_schema.h"
#include "tree_data.h"
#include "resolve.h"

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
 * @brief YANG namespace
 */
#define LY_NSYANG "urn:ietf:params:xml:ns:yang:1"

/**
 * @brief YIN namespace
 */
#define LY_NSYIN "urn:ietf:params:xml:ns:yang:yin:1"

/**
 * @brief NETCONF namespace
 */
#define LY_NSNC "urn:ietf:params:xml:ns:netconf:base:1.0"

/**
 * @brief NACM namespace
 */
#define LY_NSNACM "urn:ietf:params:xml:ns:yang:ietf-netconf-acm"

/**
 * @brief Internal list of built-in types
 */
struct ly_types {
    LY_DATA_TYPE type;
    struct lys_tpdf *def;
};
extern struct ly_types ly_types[LY_DATA_TYPE_COUNT];

/**
 * @brief Create submodule structure by reading data from memory.
 *
 * @param[in] module Schema tree where to connect the submodule, belongs-to value must match.
 * @param[in] data String containing the submodule specification in the given \p format.
 * @param[in] format Format of the data to read.
 * @param[in] unres TODO provide description
 * @return Created submodule structure or NULL in case of error.
 */
struct lys_submodule *lys_submodule_parse(struct lys_module *module, const char *data, LYS_INFORMAT format,
                                          struct unres_schema *unres);

/**
 * @brief Create submodule structure by reading data from file descriptor.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] module Schema tree where to connect the submodule, belongs-to value must match.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the submodule
 *            specification in the given \p format.
 * @param[in] format Format of the data to read.
 * @param[in] unres TODO provide description
 * @return Created submodule structure or NULL in case of error.
 */
struct lys_submodule *lys_submodule_read(struct lys_module *module, int fd, LYS_INFORMAT format,
                                         struct unres_schema *unres);

/**
 * @brief Free the submodule structure
 *
 * @param[in] submodule The structure to free. Do not use the pointer after calling this function.
 * @param[in] free_int_mods Whether to remove internal modules or not.
 */
void lys_submodule_free(struct lys_submodule *submodule, int free_int_mods);

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
 * @brief Find a valid grouping definition relative to a node.
 *
 * Valid definition means a sibling of \p start or a sibling of any of \p start 's parents.
 *
 * @param[in] name Name of the searched grouping.
 * @param[in] start Definition must be valid (visible) for this node.
 * @return Matching valid grouping or NULL.
 */
struct lys_node_grp *lys_find_grouping_up(const char *name, struct lys_node *start);

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
int lys_check_id(struct lys_node *node, struct lys_node *parent, struct lys_module *module);

/**
 * @brief Create a copy of the specified schema tree \p node
 *
 * @param[in] module Target module for the duplicated node.
 * @param[in] parent Schema tree node where the node is being connected, NULL in case of top level \p node.
 * @param[in] node Schema tree node to be duplicated.
 * @param[in] flags Config flag to be inherited in case the origin node does not specify config flag
 * @param[in] nacm NACM flags to be inherited from the parent
 * @param[in] unres TODO provide description
 * @return Created copy of the provided schema \p node.
 */
struct lys_node *lys_node_dup(struct lys_module *module, struct lys_node *parent, const struct lys_node *node,
                              uint8_t flags, uint8_t nacm, struct unres_schema *unres);

/**
 * @brief Return main module of the schema tree node.
 *
 * In case of regular YANG module, it returns ::lys_node#module pointer,
 * but in case of submodule, it returns pointer to the main module.
 *
 * @param[in] node Schema tree node to be examined
 * @return pointer to the main module (schema structure), NULL in case of error.
 */
struct lys_module *lys_mainmodule(const struct lys_node *node);

/**
 * @brief Free a schema when condition
 *
 * @param[in] libyang context where the schema of the ondition is used.
 * @param[in] w When structure to free.
 */
void lys_when_free(struct ly_ctx *ctx, struct lys_when *w);

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
 * @param[in] free_int_mods Whether to remove internal modules or not.
 */
void lys_free(struct lys_module *module, int free_int_mods);

/**
 * @brief Check presence of all the mandatory elements in the given data tree subtree
 *
 * Besides the mandatory statements, also min-elements and max-elements constraints in
 * lists and leaf-list are checked.
 *
 * @param[in] data Root node for the searching subtree. Expecting that all child instances
 * are already resolved. Note that the \p start node itself is not checked since it must be present.
 * @param[in] schema To check mandatory elements in empty data tree (\p data is NULL), we need
 * the first schema node in a schema to be checked.
 * @return The first mandatory element definition not present in the data, NULL if
 * there is no such element in the \p starts's subtree.
 */
const struct lys_node *ly_check_mandatory(const struct lyd_node *data, const struct lys_node *schema);

/**
 * @brief Find the parent node of an attribute.
 *
 * @param[in] root Root element of the data tree with the attribute.
 * @param[in] attr Attribute to find.
 *
 * @return Parent of \p attr, NULL if not found.
 */
struct lyd_node *lyd_attr_parent(struct lyd_node *root, struct lyd_attr *attr);

/**
 * @brief Find an import from \p module with matching \p prefix, \p name, or both,
 * \p module itself is also compared.
 *
 * @param[in] module Module with imports.
 * @param[in] prefix Module prefix to search for.
 * @param[in] pref_len Module \p prefix length. If 0, the whole prefix is used, if not NULL.
 * @param[in] name Module name to search for.
 * @param[in] name_len Module \p name length. If 0, the whole name is used, if not NULL.
 *
 * @return Matching module, NULL if not found.
 */
const struct lys_module *lys_get_import_module(const struct lys_module *module, const char *prefix, int pref_len,
                                               const char *name, int name_len);

/**
 * @brief Find a specific sibling. Does not log.
 *
 * Includes module comparison (handles augments if \p type & LYS_AUGMENT).
 * Module is adjusted based on the \p mod_name. Includes are also searched
 * if \p siblings are top-level nodes.
 *
 * @param[in] siblings Siblings to consider. They are first adjusted to
 *                     point to the first sibling.
 * @param[in] mod_name Module name, mandatory.
 * @param[in] mod_name_len Module name length.
 * @param[in] name Node name, mandatory.
 * @param[in] nam_len Node name length.
 * @param[in] type ORed desired type of the node. 0 means any type.
 *                 Does not return groupings, uses, and augments (but can return augment nodes).
 * @param[out] ret Pointer to the node of the desired type. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int lys_get_sibling(const struct lys_node *siblings, const char *mod_name, int mod_name_len, const char *name,
                    int nam_len, LYS_NODE type, const struct lys_node **ret);

/**
 * @brief Find a specific sibling that can appear in the data. Does not log.
 *
 * Includes are also searched if siblings are
 * top-level nodes.
 *
 * @param[in] mod Main module with the node.
 * @param[in] siblings Siblings to consider. They are first adjusted to
 *                     point to the first sibling.
 * @param[in] name Node name.
 * @param[in] type ORed desired type of the node. 0 means any (data node) type.
 * @param[out] ret Pointer to the node of the desired type. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on fail.
 */
int lys_get_data_sibling(const struct lys_module *mod, const struct lys_node *siblings, const char *name, LYS_NODE type,
                         const struct lys_node **ret);

/**
 * @brief Compare 2 data nodes if they are the same from the YANG point of view.
 *
 * - containers are the same if they are defined by the same schema tree node
 * - anyxmls are the same if they are defined by the same schema tree node
 * - leafs are the same if they are defined by the same schema tree node
 * - leaf-lists are the same if they are defined by the same schema tree node and they have the same value
 * - lists are the same if they are defined by the same schema tree node and all their keys have identical values
 *
 * @param[in] first The first data node to compare
 * @param[in] second The second node to compare
 * @param[in] unique If the given nodes are lists, value 1 here forces to check their leafs defined as unique.
 * If they are the same, the return value is 0 despite the values of the key. For all other node type, this
 * parameter is ignored.
 * @return 0 if both the nodes are the same from the YANG point of view.
 */
int lyd_compare(struct lyd_node *first, struct lyd_node *second, int unique);

#endif /* LY_TREE_INTERNAL_H_ */
