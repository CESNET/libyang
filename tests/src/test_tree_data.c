/*
 * @file test_tree_schema.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from tress_data.c
 *
 * Copyright (c) 2018-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../../src/libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

struct ly_ctx *ctx; /* context for tests */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    if (store) {
        if (path && path[0]) {
            snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
        } else {
            strncpy(logbuf, msg, BUFSIZE - 1);
        }
        if (store > 0) {
            --store;
        }
    }
}
#endif

static int
setup(void **state)
{
    (void) state; /* unused */

    const char *schema_a = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;"
            "list l1 { key \"a b\"; leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
            "leaf foo { type string;}"
            "leaf-list ll { type string;}"
            "container c {leaf-list x {type string;}}"
            "anydata any {config false;}"
            "list l2 {config false; container c{leaf x {type string;}}}}";

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));
    assert_non_null(lys_parse_mem(ctx, schema_a, LYS_IN_YANG));

    return 0;
}

static int
teardown(void **state)
{
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
#else
    (void) state; /* unused */
#endif

    ly_ctx_destroy(ctx, NULL);
    ctx = NULL;

    return 0;
}

void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

static void
test_compare(void **state)
{
    *state = test_compare;

    struct lyd_node *tree1, *tree2;

    const char *data1 = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";
    const char *data2 = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>y</c></l1>";

    assert_int_equal(LY_SUCCESS, lyd_compare(NULL, NULL, 0));

    assert_non_null(tree1 = lyd_parse_mem(ctx, data1, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_parse_mem(ctx, data2, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    assert_int_equal(LY_ENOT, lyd_compare(((struct lyd_node_inner*)tree1)->child, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data1, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_parse_mem(ctx, data2, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_ENOT, lyd_compare(tree1, tree2, 0));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1->next, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<ll xmlns=\"urn:tests:a\">a</ll><ll xmlns=\"urn:tests:a\">b</ll>";
    data2 = "<ll xmlns=\"urn:tests:a\">b</ll>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data1, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_parse_mem(ctx, data2, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_ENOT, lyd_compare(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare(NULL, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare(tree1, NULL, 0));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1->next, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<c xmlns=\"urn:tests:a\"><x>x</x></c>";
    data2 = "<c xmlns=\"urn:tests:a\"><x>y</x></c>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data1, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_parse_mem(ctx, data2, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<c xmlns=\"urn:tests:a\"><x>x</x></c>";
    data2 = "<c xmlns=\"urn:tests:a\"><x>x</x><x>y</x></c>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data1, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_parse_mem(ctx, data2, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<any xmlns=\"urn:tests:a\"><x>x</x></any>";
    data2 = "<any xmlns=\"urn:tests:a\"><x>x</x><x>y</x></any>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data1, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_parse_mem(ctx, data2, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_ENOT, lyd_compare(tree1, tree2, 0));
    lyd_free_all(tree1);
    data1 = "<any xmlns=\"urn:tests:a\"><x>x</x><x>y</x></any>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data1, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    *state = NULL;
}

static void
test_dup(void **state)
{
    *state = test_dup;

    struct lyd_node *tree1, *tree2;
    const char *result;
    const char *data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";

    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_dup(tree1, NULL, LYD_DUP_RECURSIVE));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";
    result = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b></l1>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_dup(tree1, NULL, 0));
    lyd_free_all(tree1);
    assert_non_null(tree1 = lyd_parse_mem(ctx, result, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    result = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_dup(tree1, NULL, LYD_DUP_WITH_SIBLINGS | LYD_DUP_RECURSIVE));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree2);
    assert_non_null(tree2 = lyd_dup(tree1, NULL, LYD_DUP_RECURSIVE));
    lyd_free_all(tree1);
    assert_non_null(tree1 = lyd_parse_mem(ctx, result, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree2);

    assert_non_null(tree2 = lyd_dup(tree1, NULL, 0));
    lyd_free_all(tree1);
    result = "<l2 xmlns=\"urn:tests:a\"/>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, result, LYD_XML, LYD_OPT_PARSE_ONLY));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<any xmlns=\"urn:tests:a\"><c><a>a</a></c></any>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_dup(tree1, NULL, 0));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_dup(((struct lyd_node_inner*)((struct lyd_node_inner*)tree1)->child)->child, NULL, LYD_DUP_WITH_PARENTS));
    assert_string_equal("x", tree2->schema->name);
    assert_non_null(tree2->parent);
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, (struct lyd_node*)tree2->parent->parent, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>c</c></l1>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_dup(((struct lyd_node_inner*)tree1)->child->prev, NULL, LYD_DUP_WITH_PARENTS));
    assert_string_equal("c", tree2->schema->name);
    assert_non_null(tree2->parent);
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, (struct lyd_node*)tree2->parent, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_non_null(tree2 = lyd_dup(tree1, NULL, 0));
    assert_non_null(lyd_dup(((struct lyd_node_inner*)((struct lyd_node_inner*)tree1)->child)->child,
                            (struct lyd_node_inner*)tree2, LYD_DUP_WITH_PARENTS));
    assert_int_equal(LY_SUCCESS, lyd_compare(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* invalid */
    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>c</c></l1><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    assert_non_null(tree1 = lyd_parse_mem(ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_null(lyd_dup(((struct lyd_node_inner*)tree1)->child->prev, (struct lyd_node_inner*)tree1->next, LYD_DUP_WITH_PARENTS));
    lyd_free_all(tree1);

    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_compare, setup, teardown),
        cmocka_unit_test_setup_teardown(test_dup, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
