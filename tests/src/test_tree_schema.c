/*
 * @file test_tree_schema.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from tress_schema.c
 *
 * Copyright (c) 2018-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../../src/common.c"
#include "../../src/log.c"
#include "../../src/set.c"
#include "../../src/parser_yang.c"
#include "../../src/tree_schema.c"
#include "../../src/tree_schema_compile.c"
#include "../../src/tree_schema_free.c"
#include "../../src/tree_schema_helpers.c"
#include "../../src/hash_table.c"
#include "../../src/xpath.c"
#include "../../src/context.c"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

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
logger_setup(void **state)
{
    (void) state; /* unused */

    ly_set_log_clb(logger, 0);

    return 0;
}

static int
logger_teardown(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif
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
test_getnext(void **state)
{
    *state = test_getnext;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    const struct lysc_node *node = NULL, *four;
    const struct lysc_node_container *cont;
    const struct lysc_action *rpc;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1; namespace urn:a;prefix a;"
                                        "container a { container one {presence test;} leaf two {type string;} leaf-list three {type string;}"
                                        "  list four {config false;} choice x { leaf five {type string;} case y {leaf six {type string;}}}"
                                        "  anyxml seven; action eight {input {leaf eight-input {type string;}} output {leaf eight-output {type string;}}}"
                                        "  notification nine {leaf nine-data {type string;}}}"
                                        "leaf b {type string;} leaf-list c {type string;} list d {config false;}"
                                        "choice x { leaf e {type string;} case y {leaf f {type string;}}} anyxml g;"
                                        "rpc h {input {leaf h-input {type string;}} output {leaf h-output {type string;}}}"
                                        "rpc i;"
                                        "notification j {leaf i-data {type string;}}"
                                        "notification k;}", LYS_IN_YANG));
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("a", node->name);
    cont = (const struct lysc_node_container*)node;
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("b", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("c", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("d", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("e", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("f", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("g", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("h", node->name);
    rpc = (const struct lysc_action*)node;
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("i", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("j", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("k", node->name);
    assert_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    /* Inside container */
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("one", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("two", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("three", node->name);
    assert_non_null(node = four = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("four", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("five", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("six", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("seven", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("eight", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("nine", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    /* Inside RPC */
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)rpc, mod->compiled, 0));
    assert_string_equal("h-input", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node*)rpc, mod->compiled, 0));

    /* options */
    assert_non_null(node = lys_getnext(four, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCHOICE));
    assert_string_equal("x", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCHOICE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(four, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_NOCHOICE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(four, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("five", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("y", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, LYS_GETNEXT_INTONPCONT));
    assert_string_equal("one", node->name);

    assert_non_null(node = lys_getnext(NULL, (const struct lysc_node*)rpc, mod->compiled, LYS_GETNEXT_OUTPUT));
    assert_string_equal("h-output", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node*)rpc, mod->compiled, LYS_GETNEXT_OUTPUT));

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b; feature f;"
                                        "leaf a {type string; if-feature f;}"
                                        "leaf b {type string;}}", LYS_IN_YANG));
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, 0));
    assert_string_equal("b", node->name);
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, LYS_GETNEXT_NOSTATECHECK));
    assert_string_equal("a", node->name);


    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_getnext, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
