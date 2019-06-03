/**
 * @file tree_data_hash.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Functions to manipulate with the data node's hashes.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "common.h"

#include "hash_table.h"
#include "tree_data.h"

void
lyd_unlink_hash(struct lyd_node *node)
{
    if (node->parent && node->parent->children_ht) {
        lyht_remove(node->parent->children_ht, &node, node->hash);
    }
}
