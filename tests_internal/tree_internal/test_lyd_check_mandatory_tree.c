/*
 * @file test_lyd_check_mandatory_tree.c
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
#include "../config.h"


struct ly_ctx *ctx = NULL;
struct lys_module *module;
struct lyd_node *root;
int rc;

static void
test_general_mandatory_leaf()
{
    ctx = ly_ctx_new(TESTS_DIR"/tree_internal/");
    module = (struct lys_module *) lys_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories.yang", LYS_IN_YANG);
    root = lyd_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories_general.xml", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_null(root);
    assert_true(ly_errno != 0);
    ly_ctx_destroy(ctx, NULL);
}

static void
test_toplevel_mandatory_leaf()
{
    ctx = ly_ctx_new(TESTS_DIR"/tree_internal/");
    module = (struct lys_module *) lys_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories_toplevel_fail.yang", LYS_IN_YANG);
    root = lyd_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories_example.xml", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_null(root);
    assert_true(ly_errno != 0);
    ly_ctx_destroy(ctx, NULL);
}

static void
test_listleaf_max()
{
    ctx = ly_ctx_new(TESTS_DIR"/tree_internal/");
    module = (struct lys_module *) lys_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories.yang", LYS_IN_YANG);
    root = lyd_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories_max.xml", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_null(root);
    assert_true(ly_errno != 0);
    ly_ctx_destroy(ctx, NULL);
}

static void
test_listleaf_min()
{

    ctx = ly_ctx_new(TESTS_DIR"/tree_internal/");
    module = (struct lys_module *) lys_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories.yang", LYS_IN_YANG);
    root = lyd_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories_min.xml", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_null(root);
    assert_true(ly_errno != 0);
    ly_ctx_destroy(ctx, NULL);
}

static void
test_listleaf_succ()
{
    ctx = ly_ctx_new(TESTS_DIR"/tree_internal/");
    module = (struct lys_module *) lys_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories.yang", LYS_IN_YANG);
    root = lyd_parse_path(ctx, TESTS_DIR"/tree_internal/files/mandatories_example.xml", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_non_null(root);
    assert_true(ly_errno == LY_SUCCESS);
    lyd_free(root);
    ly_ctx_destroy(ctx, NULL);
}

/**
 * @brief Test for function lys_get_sibling()
 *
 * Function lys_get_sibling() validates trees mandatory conditions.
 */
static void
test_lyd_check_mandatory_tree(void **state)
{
    (void) state; /* unused */

    /* Mandatory leaf is not given. */
    test_general_mandatory_leaf();

    /* There is a top level mandatory leaf. */
    test_toplevel_mandatory_leaf();

    /* Minimal number of leaf-list elements is not given. */
    test_listleaf_min();

    /* Maximal number of leaf-list elements is not given. */
    test_listleaf_max();

    /* Ordinary case of success. */
    test_listleaf_succ();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_lyd_check_mandatory_tree),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
