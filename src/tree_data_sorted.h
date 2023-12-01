/**
 * @file tree_data_sorted.h
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Binary search tree (BST) for sorting data nodes.
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef _LYDS_TREE_H_
#define _LYDS_TREE_H_

#include "log.h"

struct lyd_node;
struct rb_node;
struct lyd_meta;

/* This functionality applies to list and leaf-list with the "ordered-by system" statement,
 * which is implicit. The BST is implemented using a Red-black tree and is used for sorting nodes.
 * For example, a list of valid users would typically be sorted alphabetically. This tree is saved
 * in the first instance of the leaf-list/list in the metadata named lyds_tree. Thanks to the tree,
 * it is possible to insert a sibling data node in such a way that the order of the nodes is preserved.
 * The decision whether the value is greater or less takes place in the callback function ::lyplg_type_sort_clb
 * in the corresponding type.
 *
 * Parameters always must be from the same context.
 */

/**
 * @brief Check that ordering is supported for the @p node.
 *
 * If the function returns 0 for a given node, other lyds_* or rb_* functions must not be called for this node.
 *
 * @param[in] node Node to check. Expected (leaf-)list or list with key(s).
 * @return 1 if @p node can be sorted.
 */
ly_bool lyds_is_supported(struct lyd_node *node);

/**
 * @brief Create the 'lyds_tree' metadata.
 *
 * @param[in] leader First instance of the (leaf-)list. If the node already contains the metadata,
 * then nothing happens. The BST is unchanged or empty.
 * @param[out] meta Newly created 'lyds_tree' metadata.
 * @return LY_ERR value.
 */
LY_ERR lyds_create_metadata(struct lyd_node *leader, struct lyd_meta **meta);

/**
 * @brief Create new BST node.
 *
 * @param[in] node Data node to link with new red-black node.
 * @param[out] rbn Created red-black node.
 * @return LY_SUCCESS on success.
 */
LY_ERR lyds_create_node(struct lyd_node *node, struct rb_node **rbn);

/**
 * @brief Insert the @p node into BST and into @p leader's siblings.
 *
 * Sibling data nodes of the @p leader are also modified for sorting to take place.
 * The function automatically take care of lyds_create_metadata() and lyds_create_tree() calls.
 * Hash for data nodes is not added.
 *
 * @param[in,out] leader First instance of the (leaf-)list. After executing the function,
 * @p leader does not have to be be first if @p node was inserted before @p leader.
 * @param[in] node A single (without siblings) node or tree to be inserted. It must be unlinked.
 * @return LY_ERR value.
 */
LY_ERR lyds_insert(struct lyd_node **leader, struct lyd_node *node);

/**
 * @brief Unlink (remove) the specified data node from BST.
 *
 * Pointers in sibling data nodes (lyd_node) are NOT modified. This means that the data node is NOT unlinked.
 * Even if the BST will remain empty, lyds_tree metadata will remain.
 * Hash for data nodes is not removed.
 *
 * @param[in,out] leader First instance of (leaf-)list. If it is NULL, nothing happens.
 * @param[in] node Some instance of (leaf-)list to be unlinked.
 */
void lyds_unlink(struct lyd_node **leader, struct lyd_node *node);

/**
 * @brief Release the metadata including BST.
 *
 * No more nodes can be inserted after the function is executed.
 *
 * @param[in] node Data node of the type (leaf-)list that may contain metadata and BST.
 */
void lyds_free_metadata(struct lyd_node *node);

/**
 * @brief Release all BST nodes including the root.
 *
 * @param[in] rbt Root of the Red-black tree.
 */
void lyds_free_tree(struct rb_node *rbt);

/**
 * @brief Compare (sort) 2 data nodes.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @return Negative number if val1 < val2,
 * @return Zero if val1 == val2,
 * @return Positive number if val1 > val2.
 */
int lyds_compare_single(struct lyd_node *node1, struct lyd_node *node2);

#endif /* _LYDS_TREE_H_ */
