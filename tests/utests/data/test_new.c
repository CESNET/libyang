/**
 * @file test_new.c
 * @author: Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for functions for creating data
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
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

#include "libyang.h"

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
            "leaf foo { type uint16;}"
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
test_top_level(void **state)
{
    *state = test_top_level;

    const struct lys_module *mod;
    struct lyd_node *node;

    /* we need the module first */
    mod = ly_ctx_get_module_implemented(ctx, "a");
    assert_non_null(mod);

    /* list */
    node = lyd_new_list(NULL, mod, "l1", "val_a", "val_b");
    assert_non_null(node);
    lyd_free_tree(node);

    node = lyd_new_list2(NULL, mod, "l1", "[]");
    assert_null(node);
    logbuf_assert("Unexpected XPath token ] (]).");

    node = lyd_new_list2(NULL, mod, "l1", "[key1='a'][key2='b']");
    assert_null(node);
    logbuf_assert("Not found node \"key1\" in path.");

    node = lyd_new_list2(NULL, mod, "l1", "[a='a'][b='b'][c='c']");
    assert_null(node);
    logbuf_assert("Key expected instead of leaf \"c\" in path. /a:l1/c");

    node = lyd_new_list2(NULL, mod, "c", "[a='a'][b='b']");
    assert_null(node);
    logbuf_assert("List node \"c\" not found.");

    node = lyd_new_list2(NULL, mod, "l1", "[a='a'][b='b']");
    assert_non_null(node);
    lyd_free_tree(node);

    node = lyd_new_list2(NULL, mod, "l1", "[a=''][b='']");
    assert_non_null(node);
    lyd_free_tree(node);

    node = lyd_new_list2(NULL, mod, "l1", "[a:a='a'][a:b='b']");
    assert_non_null(node);
    lyd_free_tree(node);

    node = lyd_new_list2(NULL, mod, "l1", "[a=   'a']\n[b  =\t'b']");
    assert_non_null(node);
    lyd_free_tree(node);

    /* leaf */
    node = lyd_new_term(NULL, mod, "foo", "[a='a'][b='b'][c='c']");
    assert_null(node);
    logbuf_assert("Invalid uint16 value \"[a='a'][b='b'][c='c']\". /a:foo");

    node = lyd_new_term(NULL, mod, "c", "value");
    assert_null(node);
    logbuf_assert("Term node \"c\" not found.");

    node = lyd_new_term(NULL, mod, "foo", "256");
    assert_non_null(node);
    lyd_free_tree(node);

    /* leaf-list */
    node = lyd_new_term(NULL, mod, "ll", "ahoy");
    assert_non_null(node);
    lyd_free_tree(node);

    /* container */
    node = lyd_new_inner(NULL, mod, "c");
    assert_non_null(node);
    lyd_free_tree(node);

    node = lyd_new_inner(NULL, mod, "l1");
    assert_null(node);
    logbuf_assert("Inner node (and not a list) \"l1\" not found.");

    node = lyd_new_inner(NULL, mod, "l2");
    assert_null(node);
    logbuf_assert("Inner node (and not a list) \"l2\" not found.");

    /* anydata */
    node = lyd_new_any(NULL, mod, "any", "some-value", LYD_ANYDATA_STRING);
    assert_non_null(node);
    lyd_free_tree(node);

    /* key-less list */
    node = lyd_new_list2(NULL, mod, "l2", "[a='a'][b='b']");
    assert_null(node);
    logbuf_assert("List predicate defined for keyless list \"l2\" in path.");

    node = lyd_new_list2(NULL, mod, "l2", "");
    assert_non_null(node);
    lyd_free_tree(node);

    node = lyd_new_list2(NULL, mod, "l2", NULL);
    assert_non_null(node);
    lyd_free_tree(node);

    node = lyd_new_list(NULL, mod, "l2");
    assert_non_null(node);
    lyd_free_tree(node);

    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_top_level, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
