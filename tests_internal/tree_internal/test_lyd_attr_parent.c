/*
 * @file test_lyd_attr_parent.c
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
 * @brief Test for function lyd_attr_parent()
 *
 * Trivial case of success is implemented.
 */
static void
test_lyd_attr_parent(void **state)
{
    (void)state; /* unused */

    struct lyd_node *root;
    const struct lyd_node *parent;
    struct lyd_attr *attr;

    attr = calloc(1, sizeof(*attr));
    attr->name = "test_attr";
    attr->value = "test_val";

    root = calloc(1, sizeof(*root));
    root->attr = attr;

    parent = lyd_attr_parent(root, attr);

    assert_ptr_not_equal(parent, NULL);
    assert_ptr_equal(parent->attr, attr);
    assert_string_equal(parent->attr->name, "test_attr");

    free(attr);
    free(root);
}

int main(void)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test_setup_teardown(test_lyd_attr_parent, setup_f, teardown_f),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
