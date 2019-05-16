/*
 * @file test_parser_xml.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from parser_xml.c
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../../src/common.c"
#include "../../src/compat.c"
#include "../../src/set.c"
#include "../../src/log.c"
#include "../../src/hash_table.c"
#include "../../src/xpath.c"
#include "../../src/parser_yang.c"
#include "../../src/context.c"
#include "../../src/tree_schema_helpers.c"
#include "../../src/tree_schema_free.c"
#include "../../src/tree_schema_compile.c"
#include "../../src/tree_schema.c"
#include "../../src/plugins_types.c"
#include "../../src/tree_data_free.c"
#include "../../src/tree_data_helpers.c"
#include "../../src/tree_data_hash.c"
#include "../../src/printer.c"
#include "../../src/xml.c"
#include "../../src/parser_xml.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

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
            "leaf foo { type string;}}";

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

#define TEST_DUP_GENERIC(PREFIX, MEMBER, VALUE1, VALUE2, FUNC, RESULT, LINE, CLEANUP) \
    str = PREFIX MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, FUNC(&ctx, &str, RESULT)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    CLEANUP

static void
test_leaf(void **state)
{
    *state = test_leaf;

    const char *data = "<foo xmlns=\"urn:tests:a\">foo value</foo>";
    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, 0, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("foo", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("foo value", leaf->value.canonized);

    lyd_free_all(tree);
    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_leaf, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
