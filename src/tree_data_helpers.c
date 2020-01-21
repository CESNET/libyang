/**
 * @file tree_data_helpers.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsing and validation helper functions for data trees
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "common.h"

#include <assert.h>
#include <stdlib.h>

#include "log.h"
#include "dict.h"
#include "plugins_types.h"
#include "tree_data.h"
#include "tree_schema.h"

struct lyd_node **
lyd_node_children_p(struct lyd_node *node)
{
    assert(node);
    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_ACTION:
    case LYS_NOTIF:
        return &((struct lyd_node_inner*)node)->child;
    default:
        return NULL;
    }
}

API const struct lyd_node *
lyd_node_children(const struct lyd_node *node)
{
    struct lyd_node **children;

    if (!node) {
        return NULL;
    }

    children = lyd_node_children_p((struct lyd_node*)node);
    if (children) {
        return *children;
    } else {
        return NULL;
    }
}
