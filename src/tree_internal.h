/**
 * @file tree_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang internal functions for manipulating with the data model and
 * data trees.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_INTERNAL_H_
#define LY_TREE_INTERNAL_H_

#include "tree_schema.h"
#include "tree_data.h"
#include "resolve.h"

/* this is used to distinguish lyxml_elem * from a YANG temporary parsing structure, the first byte is compared */
#define LY_YANG_STRUCTURE_FLAG 0x80

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
 * @brief internal parser flag for actions and inline notifications
 */
#define LYD_OPT_ACT_NOTIF 0x80

/**
 * @brief Internal list of built-in types
 */
struct ly_types {
    LY_DATA_TYPE type;
    struct lys_tpdf *def;
};
extern struct ly_types ly_types[LY_DATA_TYPE_COUNT];

/**
 * @brief Internal structure for data node sorting.
 */
struct lyd_node_pos {
    struct lyd_node *node;
    uint32_t pos;
};

/**
 * Macros to work with ::lyd_node#when_status
 * +--- bit 1 - some when-stmt connected with the node (resolve_applies_when() is true)
 * |+-- bit 2 - when-stmt's condition is resolved and it is true
 * ||+- bit 3 - when-stmt's condition is resolved and it is false
 * XXX
 *
 * bit 1 is set when the node is created
 * if none of bits 2 and 3 is set, the when condition is not yet resolved
 */
#define LYD_WHEN       0x04
#define LYD_WHEN_TRUE  0x02
#define LYD_WHEN_FALSE 0x01
#define LYD_WHEN_DONE(status) (!((status) & LYD_WHEN) || ((status) & (LYD_WHEN_TRUE | LYD_WHEN_FALSE)))

/**
 * @brief Create submodule structure by reading data from memory.
 *
 * @param[in] module Schema tree where to connect the submodule, belongs-to value must match.
 * @param[in] data String containing the submodule specification in the given \p format.
 * @param[in] format Format of the data to read.
 * @param[in] unres list of unresolved items
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
 * @param[in] unres list of unresolved items
 * @return Created submodule structure or NULL in case of error.
 */
struct lys_submodule *lys_submodule_read(struct lys_module *module, int fd, LYS_INFORMAT format,
                                         struct unres_schema *unres);

/**
 * @brief Free the submodule structure
 *
 * @param[in] submodule The structure to free. Do not use the pointer after calling this function.
 * @param[in] private_destructor Optional destructor function for private objects assigned
 * to the nodes via lys_set_private(). If NULL, the private objects are not freed by libyang.
 */
void lys_submodule_free(struct lys_submodule *submodule, void (*private_destructor)(const struct lys_node *node, void *priv));

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
 * @brief Check all XPath expressions of a node (when and must), set LYS_XPATH_DEP flag if required.
 *
 * @param[in] node Node to examine.
 * @param[in] check_place Check where the node is placed to get know if the check is supposed to be performed
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int lys_check_xpath(struct lys_node *node, int check_place);

/**
 * @brief Get know if the node contains must or when with XPath expression
 *
 * @param[in] node Node to examine.
 * @return 1 if contains, 0 otherwise
 */
int lys_has_xpath(const struct lys_node *node);

/**
 * @brief Create a copy of the specified schema tree \p node
 *
 * @param[in] module Target module for the duplicated node.
 * @param[in] parent Schema tree node where the node is being connected, NULL in case of top level \p node.
 * @param[in] node Schema tree node to be duplicated.
 * @param[in] nacm NACM flags to be inherited from the parent
 * @param[in] unres list of unresolved items
 * @param[in] shallow Whether to copy children and connect to parent/module too.
 * @return Created copy of the provided schema \p node.
 */
struct lys_node *lys_node_dup(struct lys_module *module, struct lys_node *parent, const struct lys_node *node,
                              uint8_t nacm, struct unres_schema *unres, int shallow);

/**
 * @brief Switch two same schema nodes. \p src must be a shallow copy
 * of \p dst.
 *
 * @param[in] dst Destination node that will be replaced with \p src.
 * @param[in] src Source node that will replace \p dst.
 */
void lys_node_switch(struct lys_node *dst, struct lys_node *src);

/**
 * @brief Add pointer to \p leafref to \p leafref_target children so that it knows there
 * are some leafrefs referring it.
 *
 * @param[in] leafref_target Leaf that is \p leafref's target.
 * @param[in] leafref Leaf or leaflist of type #LY_TYPE_LEAFREF referring \p leafref_target.
 * @return 0 on success, -1 on error.
 */
int lys_leaf_add_leafref_target(struct lys_node_leaf *leafref_target, struct lys_node *leafref);

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
 * @param[in] private_destructor Optional destructor function for private objects assigned
 * to the nodes via lys_set_private(). If NULL, the private objects are not freed by libyang.
 * @param[in] shallow Whether to do a shallow free only (on a shallow copy of a node).
 */
void lys_node_free(struct lys_node *node, void (*private_destructor)(const struct lys_node *node, void *priv), int shallow);

/**
 * @brief Free (and unlink it from the context) the specified schema.
 *
 * It is dangerous to call this function on schemas already placed into the context's
 * list of modules - there can be many references from other modules and data instances.
 *
 * @param[in] module Data model to free.
 * @param[in] private_destructor Optional destructor function for private objects assigned
 * to the nodes via lys_set_private(). If NULL, the private objects are not freed by libyang.
 * @param[in] remove_from_ctx Whether to remove this model from context. Always use 1 except
 * when removing all the models (in ly_ctx_destroy()).
 */
void lys_free(struct lys_module *module, void (*private_destructor)(const struct lys_node *node, void *priv), int remove_from_ctx);

/**
 * @brief Create a data container knowing it's schema node.
 *
 * @param[in] parent Data parent of the new node.
 * @param[in] schema Schema node of the new node.
 * @param[in] dflt Set dflt flag in the created data nodes
 * @return New node, NULL on error.
 */
struct lyd_node *_lyd_new(struct lyd_node *parent, const struct lys_node *schema, int dflt);

/**
 * @brief Create a dummy node for XPath evaluation. After done using, it should be removed.
 *
 * The function must be used very carefully:
 * - there must not be a list node to create
 *
 * @param[in] data Any data node of the tree where the dummy node will be created
 * @param[in] parent To optimize searching in data tree (and to avoid issues with lists), caller can specify a
 *                   parent node that exists in the data tree.
 * @param[in] schema Schema node of the dummy node to create, must be of nodetype that
 * appears also in data tree.
 * @param[in] value Optional value to be set in the dummy node
 * @param[in] dflt Set dflt flag in the created data nodes
 *
 * @return The first created node needed for the dummy node in the given tree.
 */
struct lyd_node *lyd_new_dummy(struct lyd_node *data, struct lyd_node *parent, const struct lys_node *schema,
                               const char *value, int dflt);

/**
 * @brief Find the parent node of an attribute.
 *
 * @param[in] root Root element of the data tree with the attribute.
 * @param[in] attr Attribute to find.
 *
 * @return Parent of \p attr, NULL if not found.
 */
const struct lyd_node *lyd_attr_parent(const struct lyd_node *root, struct lyd_attr *attr);

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
 * Since \p mod_name is mandatory, augments are handled.
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
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference.
 */
int lys_get_sibling(const struct lys_node *siblings, const char *mod_name, int mod_name_len, const char *name,
                    int nam_len, LYS_NODE type, const struct lys_node **ret);

/**
 * @brief Find a specific sibling that can appear in the data. Does not log.
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
 * @brief Compare 2 list or leaf-list data nodes if they are the same from the YANG point of view. Logs directly.
 *
 * - leaf-lists are the same if they are defined by the same schema tree node and they have the same value
 * - lists are the same if they are defined by the same schema tree node, all their keys have identical values,
 *   and all unique sets have the same values
 *
 * @param[in] first First data node to compare.
 * @param[in] second Second node to compare.
 * @param[in] action Option to specify what will be checked:
 *            -1 - compare keys and all uniques
 *             0 - compare only keys
 *             n - compare n-th unique
 * @param[in] printval Flag for printing validation errors, useful for internal (non-validation) use of this function
 * @return 1 if both the nodes are the same from the YANG point of view,
 *         0 if they differ,
 *         -1 on error.
 */
int lyd_list_equal(struct lyd_node *first, struct lyd_node *second, int action, int printval);

const char *lyd_get_unique_default(const char* unique_expr, struct lyd_node *list);

/**
 * @brief Check for (validate) mandatory nodes of a data tree. Checks recursively whole data tree. Requires all when
 * statement to be solved.
 *
 * @param[in] root Data tree to validate.
 * @param[in] ctx libyang context (for the case when the data tree is empty - i.e. root == NULL).
 * @param[in] options Standard @ref parseroptions.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int lyd_check_mandatory_tree(struct lyd_node *root, struct ly_ctx *ctx, int options);

/**
 * @brief Add default values, \p resolve unres, and finally
 * remove any redundant default values based on \p options.
 *
 * @param[in] root Data tree root. With empty data tree, new default nodes can be created so the root pointer
 *            will contain/return the newly created data tree.
 * @param[in] options Options for the inserting data to the target data tree options, see @ref parseroptions.
 * @param[in] ctx Optional parameter. If provided, default nodes from all modules in the context will be added.
 *            If NULL, only the modules explicitly mentioned in data tree are taken into account.
 * @param[in] data_tree Additional data tree to be traversed when evaluating when or must expressions in \p root
 *            tree.
 * @param[in] act_notif Action/notification itself in case \p root is actually an action/notification.
 * @param[in] unres Valid unres structure, on function successful exit they are all resolved.
 * @return 0 on success, nonzero on failure.
 */
int lyd_defaults_add_unres(struct lyd_node **root, int options, struct ly_ctx *ctx, const struct lyd_node *data_tree,
                           struct lyd_node *act_notif, struct unres_data *unres);

void lys_switch_deviations(struct lys_module *module);

void lys_sub_module_remove_devs_augs(struct lys_module *module);

void lys_submodule_module_data_free(struct lys_submodule *submodule);

/**
 * @brief Get know if the \p leaf is a key of the \p list
 * @return 0 for false, position of the key otherwise
 */
int lys_is_key(struct lys_node_list *list, struct lys_node_leaf *leaf);

#endif /* LY_TREE_INTERNAL_H_ */
