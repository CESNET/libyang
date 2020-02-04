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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "../../src/context.h"
#include "../../src/tree_data_internal.h"

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

    const char *schema_a =
        "module a {"
            "namespace urn:tests:a;"
            "prefix a;"
            "yang-version 1.1;"

            "container cont {"
                "leaf a {"
                    "when \"../../c = 'val_c'\";"
                    "type string;"
                "}"
                "leaf b {"
                    "type string;"
                "}"
            "}"
            "leaf c {"
                "when \"/cont/b = 'val_b'\";"
                "type string;"
            "}"
        "}";
    const char *schema_b =
        "module b {"
            "namespace urn:tests:b;"
            "prefix b;"
            "yang-version 1.1;"

            "choice choic {"
                "mandatory true;"
                "leaf a {"
                    "type string;"
                "}"
                "case b {"
                    "leaf l {"
                        "type string;"
                    "}"
                "}"
            "}"
            "leaf c {"
                "mandatory true;"
                "type string;"
            "}"
            "leaf d {"
                "type empty;"
            "}"
        "}";
    const char *schema_c =
        "module c {"
            "namespace urn:tests:c;"
            "prefix c;"
            "yang-version 1.1;"

            "choice choic {"
                "leaf a {"
                    "type string;"
                "}"
                "case b {"
                    "leaf-list l {"
                        "min-elements 3;"
                        "type string;"
                    "}"
                "}"
            "}"
            "list lt {"
                "max-elements 4;"
                "key \"k\";"
                "leaf k {"
                    "type string;"
                "}"
            "}"
            "leaf d {"
                "type empty;"
            "}"
        "}";

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));
    assert_non_null(lys_parse_mem(ctx, schema_a, LYS_IN_YANG));
    assert_non_null(lys_parse_mem(ctx, schema_b, LYS_IN_YANG));
    assert_non_null(lys_parse_mem(ctx, schema_c, LYS_IN_YANG));

    return 0;
}

static int
teardown(void **state)
{
    (void)state;
    ly_ctx_destroy(ctx, NULL);
    ctx = NULL;

    return 0;
}

static int
teardown_s(void **state)
{
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
#else
    (void) state; /* unused */
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
test_when(void **state)
{
    *state = test_when;

    const char *data;
    struct lyd_node *tree;

    data = "<c xmlns=\"urn:tests:a\">hey</c>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("When condition \"/cont/b = 'val_b'\" not satisfied.");

    data = "<cont xmlns=\"urn:tests:a\"><b>val_b</b></cont><c xmlns=\"urn:tests:a\">hey</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_non_null(tree);
    assert_string_equal("c", tree->next->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);

    data = "<cont xmlns=\"urn:tests:a\"><a>val</a><b>val_b</b></cont><c xmlns=\"urn:tests:a\">val_c</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_non_null(tree);
    assert_string_equal("a", lyd_node_children(tree)->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, lyd_node_children(tree)->flags);
    assert_string_equal("c", tree->next->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);

    *state = NULL;
}

static void
test_mandatory(void **state)
{
    *state = test_mandatory;

    const char *data;
    struct lyd_node *tree;

    data = "<d xmlns=\"urn:tests:b\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"choic\" instance does not exist. /b:choic");

    data = "<l xmlns=\"urn:tests:b\">string</l><d xmlns=\"urn:tests:b\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a><c xmlns=\"urn:tests:b\">string2</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_non_null(tree);
    lyd_free_withsiblings(tree);

    *state = NULL;
}

static void
test_minmax(void **state)
{
    *state = test_minmax;

    const char *data;
    struct lyd_node *tree;

    data = "<d xmlns=\"urn:tests:c\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>"
    "<l xmlns=\"urn:tests:c\">val3</l>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_non_null(tree);
    lyd_free_withsiblings(tree);

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>"
    "<l xmlns=\"urn:tests:c\">val3</l>"
    "<lt xmlns=\"urn:tests:c\"><k>val1</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val2</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val3</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val4</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val5</k></lt>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_OPT_VAL_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Too many \"lt\" instances. /c:lt");

    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_teardown(test_when, teardown_s),
        cmocka_unit_test_teardown(test_mandatory, teardown_s),
        cmocka_unit_test_teardown(test_minmax, teardown_s),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
