/*
 * @file test_lys_find_grouping_up.c
 * @author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 * @brief unit tests for functions from tree_internal.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "../../src/libyang.h"
#include "../../src/tree_internal.h"


struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

static int
setup_f(void **state)
{
    (void) state; /* unused */
    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (ctx) {
        ly_ctx_destroy(ctx, NULL);
    }
    return 0;
}

/**
 * @brief Test for function lys_find_grouping_up() function.
 */
static void
test_lys_find_grouping_up(void **state)
{
    (void) state; /* unused */
    struct lys_node *node_yes, *node_no;
    struct lys_node_grp *group, *found_grp;
    struct lys_module *module;

    /* Filler module, all structures need notion of module. */
    module = calloc(1, sizeof(struct lys_module));
    module->ctx = ctx;
    module->name = "module";
    module->prefix = "m";

    /* Group node for function to find or fail trying. */
    group = calloc(1, sizeof(struct lys_node_grp));
    group->name = "G";
    group->module = module;
    group->nodetype = LYS_GROUPING;
    group->parent = NULL;

    /* case: trivial success
     * Simple two-node tree is created where 'group' node is previous sibling
     * of ordinary node. Property is checked by comparing pointers of created
     * group node and the one function found. */
    node_yes = calloc(1, sizeof(struct lys_node));
    group->next = node_yes;
    node_yes->name = "Ny";
    node_yes->nodetype = LYS_LEAF;
    node_yes->module = module;
    node_yes->parent = NULL;
    node_yes->prev = (struct lys_node*)group;
    found_grp = lys_find_grouping_up("G", node_yes);

    assert_ptr_equal(found_grp, group);

    free(node_yes);
    free(group);

    /* case: no group sibling
     * The simplest failure test. Node has no parent or siblings so function is
     * expected to return NULL pointer. */
    node_no = calloc(1, sizeof(struct lys_node));
    node_no->nodetype = LYS_LEAF;
    node_no->module = module;
    node_no->name = "Nn";
    node_no->prev = node_no->parent = NULL;
    found_grp = lys_find_grouping_up("G", node_no);

    assert_null(found_grp);

    /* case: top-level augment
     * A special case in which augment is at the module top-level. Function is
     * expected to return NULL if there is top-level augment and there is no
     * specific module data to handle it.  */
    struct lys_node_augment *parent;
    parent = calloc(1,sizeof(struct lys_node_augment));
    node_no->parent = (struct lys_node*)parent;
    parent->nodetype = LYS_AUGMENT;
    parent->module = module;
    module->data = NULL;
    found_grp = lys_find_grouping_up("G", node_no);

    assert_null(found_grp);

    free(node_no);
    free(parent);
    free(module);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_lys_find_grouping_up, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
