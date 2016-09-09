/*
 * @file test_lyd_list_equal.c
 * @author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 * @brief unit tests for functions from tree_internal.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
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
 * @brief Helper function for creating schema.

 * Optionally, schema can be added an unique element with expression string.
 *
 * @param schema, allocated.
 * @param type type of schema.
 * @param is_unique determines whether to create schema with unique elements.
 * @param expr_str is schema unique expression.
*/
static int
create_schema(struct lys_node_list *schema, const LYS_NODE type, const int is_unique, const char *expr_str)
{
    schema->nodetype = type;

    if (is_unique && expr_str) {
        struct lys_unique *uniq = calloc(1, sizeof(*uniq));
        const char *expr = expr_str;
        uniq->expr = calloc(1, sizeof(expr));
        *uniq->expr = expr;
        uniq->expr_size = 1;
        schema->unique = uniq;
        schema->unique_size = 1;
    }

    return 0;
}

/**
 * Test case: When two lists have different schemes, they are considered different.
*/
static void
test_lyd_list_equal_diff_schema(struct lyd_node *xs, struct lyd_node *ys)
{
    int rc;

    struct lys_node_list *different_schema;

    different_schema = calloc(1, sizeof(*different_schema));
    different_schema->nodetype = LYS_LIST;

    ys->schema = (struct lys_node*)different_schema;
    rc = lyd_list_equal(xs, ys, 0, 0);
    assert_true(rc == 0);

    ys->schema = xs->schema;

    free(different_schema);
}

/**
 * Test case: Trivial case when action is 0, no unique check is done.
*/
static void
test_lyd_list_equal_no_action(struct lyd_node *xs, struct lyd_node *ys)
{
    int rc;

    /* no action, no keys */
    rc = lyd_list_equal(xs, ys, 0, 0);
    assert_true(rc == 0);
}

/**
 * Test case: check two element lists for equality.
*/
static void
test_lyd_list_equal_compare_keys(struct lys_node_list *schema, struct lyd_node *xs, struct lyd_node *ys)
{
    int rc;
    struct lys_node_leaf *x_key, *y_key;
    struct lyd_node_leaf_list *x_child, *y_child;

    /* Initialize keys for schema */
    x_key = calloc(1, sizeof(*x_key));
    y_key = calloc(1, sizeof(*y_key));

    schema->keys = calloc(1, sizeof(*x_key));
    *schema->keys = (struct lys_node_leaf*)schema;
    schema->keys_size = 1;

    /* Initialize children */
    x_child = calloc(1, sizeof(*x_child));
    x_child->parent = xs;
    x_child->schema = (struct lys_node*)schema;
    xs->child = (struct lyd_node*)x_child;

    y_child = calloc(1, sizeof(*y_child));
    y_child->parent = ys;
    y_child->schema = (struct lys_node*)schema;
    ys->child = (struct lyd_node*)y_child;

    /* case: equal lists */
    x_child->value_str = "xstr";
    y_child->value_str = "xstr";
    rc = lyd_list_equal(xs, ys, 0, 0);
    assert_int_equal(rc, 1);

    /* case: different lists */
    y_child->value_str = "ystr";
    rc = lyd_list_equal(xs, ys, 0, 0);
    assert_int_equal(rc, 0);

    free(y_key);
    free(x_key);
    free(y_child);
    free(x_child);
}

/**
 * @brief test case for lyd_list equality for cases when type is LYS_LIST.
 *
 * It uses following helper function:
 * test_lyd_list_equall_diff_schema() for case when lists are different because of different schemes.
 * test_lyd_list_equal_no_action() for case when flag action is false, no unique checks.
 * test_lyd_list_equal_compare_keys() for case of comparing two-element lists.
 */
static void
test_lyd_list_equal(void **state)
{
    (void) state; /* unused */

    struct lyd_node *xs, *ys;
    struct lys_node_list *schema;

    /* Initialize schema and two nodes.*/
    schema = calloc(1, sizeof(*schema));
    create_schema(schema, LYS_LIST, 0, NULL);

    xs = calloc(1, sizeof(*xs));
    xs->schema = (struct lys_node*)schema;

    ys = calloc(1, sizeof(*ys));
    ys->schema = (struct lys_node*)schema;

    test_lyd_list_equal_diff_schema(xs, ys);
    test_lyd_list_equal_no_action(xs, ys);
    test_lyd_list_equal_compare_keys(schema, xs, ys);

    free(ys);
    free(xs);
    free(schema->keys);
    free(schema);
};

/**
   @brief Test for function lyd_list_equal()

   This is a test for equality of leaf lists.

   Two singleton leaf lists are created and tested for two simple cases:

   case: different
   case: equal
 */
static void
test_lyd_list_equal_leaf_list(void **state)
{
    (void) state; /* unused */
    struct lyd_node_leaf_list *xs;
    struct lyd_node_leaf_list *ys;
    struct lys_node_list *schema;
    int rc;

    /* Initialization. */
    schema = calloc(1, sizeof(*schema));
    create_schema(schema, LYS_LEAFLIST, 0, NULL);

    xs = calloc(1, sizeof(*xs));
    xs->schema = (struct lys_node*)schema;

    ys = calloc(1, sizeof(*ys));
    ys->schema = (struct lys_node*)schema;

    /* case: failure
     * Two nodes with different value strings are given.
     * Expected result is 0, which indicates that they are different. */
    xs->value_str = "x";
    ys->value_str = "y";
    rc = lyd_list_equal((struct lyd_node*)xs, (struct lyd_node*)ys, 0, 0);
    assert_int_equal(rc, 0);

    /* case: failure
     * Two nodes with different value strings are given.
     * Expected result is 1, which indicates equality of leafs. */
    ys->value_str = "x";
    rc = lyd_list_equal((struct lyd_node*)xs, (struct lyd_node*)ys, 0, 0);
    assert_int_equal(rc, 1);

    /* End */
    free(ys);
    free(xs);
    free(schema);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_lyd_list_equal, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_list_equal_leaf_list, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
