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
    const char *schema_d =
        "module d {"
            "namespace urn:tests:d;"
            "prefix d;"
            "yang-version 1.1;"

            "list lt {"
                "key \"k\";"
                "unique \"l1\";"
                "leaf k {"
                    "type string;"
                "}"
                "leaf l1 {"
                    "type string;"
                "}"
            "}"
            "list lt2 {"
                "key \"k\";"
                "unique \"cont/l2 l4\";"
                "unique \"l5 l6\";"
                "leaf k {"
                    "type string;"
                "}"
                "container cont {"
                    "leaf l2 {"
                        "type string;"
                    "}"
                "}"
                "leaf l4 {"
                    "type string;"
                "}"
                "leaf l5 {"
                    "type string;"
                "}"
                "leaf l6 {"
                    "type string;"
                "}"
                "list lt3 {"
                    "key \"kk\";"
                    "unique \"l3\";"
                    "leaf kk {"
                        "type string;"
                    "}"
                    "leaf l3 {"
                        "type string;"
                    "}"
                "}"
            "}"
        "}";

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));
    assert_non_null(lys_parse_mem(ctx, schema_a, LYS_IN_YANG));
    assert_non_null(lys_parse_mem(ctx, schema_b, LYS_IN_YANG));
    assert_non_null(lys_parse_mem(ctx, schema_c, LYS_IN_YANG));
    assert_non_null(lys_parse_mem(ctx, schema_d, LYS_IN_YANG));

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
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("When condition \"/cont/b = 'val_b'\" not satisfied. /a:c");

    data = "<cont xmlns=\"urn:tests:a\"><b>val_b</b></cont><c xmlns=\"urn:tests:a\">hey</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_non_null(tree);
    assert_string_equal("c", tree->next->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);

    data = "<cont xmlns=\"urn:tests:a\"><a>val</a><b>val_b</b></cont><c xmlns=\"urn:tests:a\">val_c</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
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
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"choic\" instance does not exist. /b:choic");

    data = "<l xmlns=\"urn:tests:b\">string</l><d xmlns=\"urn:tests:b\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a><c xmlns=\"urn:tests:b\">string2</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
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
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>"
    "<l xmlns=\"urn:tests:c\">val3</l>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
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
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Too many \"lt\" instances. /c:lt");

    *state = NULL;
}

static void
test_unique(void **state)
{
    *state = test_unique;

    const char *data;
    struct lyd_node *tree;

    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>same</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
    "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_non_null(tree);
    lyd_free_withsiblings(tree);

    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>same</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<l1>not-same</l1>"
    "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_non_null(tree);
    lyd_free_withsiblings(tree);

    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>same</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<l1>same</l1>"
    "</lt>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l1\" not satisfied in \"/d:lt[k='val1']\" and \"/d:lt[k='val2']\". /d:lt[k='val2']");

    /* now try with more instances */
    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>1</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<l1>2</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val3</k>"
        "<l1>3</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val4</k>"
        "<l1>4</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val5</k>"
        "<l1>5</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val6</k>"
        "<l1>6</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val7</k>"
        "<l1>7</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val8</k>"
        "<l1>8</l1>"
    "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_non_null(tree);
    lyd_free_withsiblings(tree);

    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>1</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<l1>2</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val3</k>"
        "<l1>3</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val4</k>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val5</k>"
        "<l1>5</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val6</k>"
        "<l1>6</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val7</k>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val8</k>"
    "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_non_null(tree);
    lyd_free_withsiblings(tree);

    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>1</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<l1>2</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val3</k>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val4</k>"
        "<l1>4</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val5</k>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val6</k>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val7</k>"
        "<l1>2</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val8</k>"
        "<l1>8</l1>"
    "</lt>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l1\" not satisfied in \"/d:lt[k='val7']\" and \"/d:lt[k='val2']\". /d:lt[k='val2']");

    *state = NULL;
}

static void
test_unique_nested(void **state)
{
    *state = test_unique_nested;

    const char *data;
    struct lyd_node *tree;

    /* nested list uniquest are compared only with instances in the same parent list instance */
    data =
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<cont>"
            "<l2>1</l2>"
        "</cont>"
        "<l4>1</l4>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<cont>"
            "<l2>2</l2>"
        "</cont>"
        "<l4>2</l4>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>1</l3>"
        "</lt3>"
        "<lt3>"
            "<kk>val2</kk>"
            "<l3>2</l3>"
        "</lt3>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val3</k>"
        "<cont>"
            "<l2>3</l2>"
        "</cont>"
        "<l4>3</l4>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>2</l3>"
        "</lt3>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val4</k>"
        "<cont>"
            "<l2>4</l2>"
        "</cont>"
        "<l4>4</l4>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>3</l3>"
        "</lt3>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val5</k>"
        "<cont>"
            "<l2>5</l2>"
        "</cont>"
        "<l4>5</l4>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>3</l3>"
        "</lt3>"
    "</lt2>";
    assert_int_equal(LY_SUCCESS, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY | LYD_OPT_STRICT, &tree));
    assert_non_null(tree);
    lyd_free_withsiblings(tree);

    data =
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<cont>"
            "<l2>1</l2>"
        "</cont>"
        "<l4>1</l4>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<cont>"
            "<l2>2</l2>"
        "</cont>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>1</l3>"
        "</lt3>"
        "<lt3>"
            "<kk>val2</kk>"
            "<l3>2</l3>"
        "</lt3>"
        "<lt3>"
            "<kk>val3</kk>"
            "<l3>1</l3>"
        "</lt3>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val3</k>"
        "<cont>"
            "<l2>3</l2>"
        "</cont>"
        "<l4>1</l4>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>2</l3>"
        "</lt3>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val4</k>"
        "<cont>"
            "<l2>4</l2>"
        "</cont>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>3</l3>"
        "</lt3>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val5</k>"
        "<cont>"
            "<l2>5</l2>"
        "</cont>"
        "<lt3>"
            "<kk>val1</kk>"
            "<l3>3</l3>"
        "</lt3>"
    "</lt2>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l3\" not satisfied in \"/d:lt2[k='val2']/lt3[kk='val3']\" and"
                  " \"/d:lt2[k='val2']/lt3[kk='val1']\". /d:lt2[k='val2']/lt3[kk='val1']");

    data =
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<cont>"
            "<l2>1</l2>"
        "</cont>"
        "<l4>1</l4>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<cont>"
            "<l2>2</l2>"
        "</cont>"
        "<l4>2</l4>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val3</k>"
        "<cont>"
            "<l2>3</l2>"
        "</cont>"
        "<l4>3</l4>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val4</k>"
        "<cont>"
            "<l2>2</l2>"
        "</cont>"
        "<l4>2</l4>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val5</k>"
        "<cont>"
            "<l2>5</l2>"
        "</cont>"
        "<l4>5</l4>"
    "</lt2>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"cont/l2 l4\" not satisfied in \"/d:lt2[k='val4']\" and \"/d:lt2[k='val2']\". /d:lt2[k='val2']");

    data =
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<cont>"
            "<l2>1</l2>"
        "</cont>"
        "<l4>1</l4>"
        "<l5>1</l5>"
        "<l6>1</l6>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<cont>"
            "<l2>2</l2>"
        "</cont>"
        "<l4>1</l4>"
        "<l5>1</l5>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val3</k>"
        "<cont>"
            "<l2>3</l2>"
        "</cont>"
        "<l4>1</l4>"
        "<l5>3</l5>"
        "<l6>3</l6>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val4</k>"
        "<cont>"
            "<l2>4</l2>"
        "</cont>"
        "<l4>1</l4>"
        "<l6>1</l6>"
    "</lt2>"
    "<lt2 xmlns=\"urn:tests:d\">"
        "<k>val5</k>"
        "<cont>"
            "<l2>5</l2>"
        "</cont>"
        "<l4>1</l4>"
        "<l5>3</l5>"
        "<l6>3</l6>"
    "</lt2>";
    assert_int_equal(LY_EVALID, lyd_parse_xml(ctx, data, LYD_VALOPT_DATA_ONLY, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l5 l6\" not satisfied in \"/d:lt2[k='val5']\" and \"/d:lt2[k='val3']\". /d:lt2[k='val3']");

    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_teardown(test_when, teardown_s),
        cmocka_unit_test_teardown(test_mandatory, teardown_s),
        cmocka_unit_test_teardown(test_minmax, teardown_s),
        cmocka_unit_test_teardown(test_unique, teardown_s),
        cmocka_unit_test_teardown(test_unique_nested, teardown_s),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
