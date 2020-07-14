/**
 * @file test_validation.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from validation.c
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
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "context.h"
#include "parser.h"
#include "parser_data.h"
#include "printer.h"
#include "printer_data.h"
#include "tests/config.h"
#include "tree_schema.h"
#include "tree_data_internal.h"

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
    const char *schema_e =
        "module e {"
            "namespace urn:tests:e;"
            "prefix e;"
            "yang-version 1.1;"

            "choice choic {"
                "leaf a {"
                    "type string;"
                "}"
                "case b {"
                    "leaf-list l {"
                        "type string;"
                    "}"
                "}"
            "}"
            "list lt {"
                "key \"k\";"
                "leaf k {"
                    "type string;"
                "}"
            "}"
            "leaf d {"
                "type uint32;"
            "}"
            "leaf-list ll {"
                "type string;"
            "}"
            "container cont {"
                "list lt {"
                    "key \"k\";"
                    "leaf k {"
                        "type string;"
                    "}"
                "}"
                "leaf d {"
                    "type uint32;"
                "}"
                "leaf-list ll {"
                    "type string;"
                "}"
                "leaf-list ll2 {"
                    "type enumeration {"
                        "enum one;"
                        "enum two;"
                    "}"
                "}"
            "}"
        "}";
    const char *schema_f =
        "module f {"
            "namespace urn:tests:f;"
            "prefix f;"
            "yang-version 1.1;"

            "choice choic {"
                "default \"c\";"
                "leaf a {"
                    "type string;"
                "}"
                "case b {"
                    "leaf l {"
                        "type string;"
                    "}"
                "}"
                "case c {"
                    "leaf-list ll1 {"
                        "type string;"
                        "default \"def1\";"
                        "default \"def2\";"
                        "default \"def3\";"
                    "}"
                "}"
            "}"
            "leaf d {"
                "type uint32;"
                "default 15;"
            "}"
            "leaf-list ll2 {"
                "type string;"
                "default \"dflt1\";"
                "default \"dflt2\";"
            "}"
            "container cont {"
                "choice choic {"
                    "default \"c\";"
                    "leaf a {"
                        "type string;"
                    "}"
                    "case b {"
                        "leaf l {"
                            "type string;"
                        "}"
                    "}"
                    "case c {"
                        "leaf-list ll1 {"
                            "type string;"
                            "default \"def1\";"
                            "default \"def2\";"
                            "default \"def3\";"
                        "}"
                    "}"
                "}"
                "leaf d {"
                    "type uint32;"
                    "default 15;"
                "}"
                "leaf-list ll2 {"
                    "type string;"
                    "default \"dflt1\";"
                    "default \"dflt2\";"
                "}"
            "}"
        "}";
    const char *schema_g =
        "module g {"
            "namespace urn:tests:g;"
            "prefix g;"
            "yang-version 1.1;"

            "feature f1;"
            "feature f2;"
            "feature f3;"

            "container cont {"
                "if-feature \"f1\";"
                "choice choic {"
                    "if-feature \"f2 or f3\";"
                    "leaf a {"
                        "type string;"
                    "}"
                    "case b {"
                        "if-feature \"f2 and f1\";"
                        "leaf l {"
                            "type string;"
                        "}"
                    "}"
                "}"
                "leaf d {"
                    "type uint32;"
                "}"
                "container cont2 {"
                    "if-feature \"f2\";"
                    "leaf e {"
                        "type string;"
                    "}"
                "}"
            "}"
        "}";
    const char *schema_h =
        "module h {"
            "namespace urn:tests:h;"
            "prefix h;"
            "yang-version 1.1;"

            "container cont {"
                "container cont2 {"
                    "config false;"
                    "leaf l {"
                        "type string;"
                    "}"
                "}"
            "}"
        "}";
    const char *schema_i =
        "module i {"
            "namespace urn:tests:i;"
            "prefix i;"
            "yang-version 1.1;"

            "container cont {"
                "leaf l {"
                    "type string;"
                "}"
                "leaf l2 {"
                    "must \"../l = 'right'\";"
                    "type string;"
                "}"
            "}"
        "}";
    const char *schema_j =
        "module j {"
            "namespace urn:tests:j;"
            "prefix j;"
            "yang-version 1.1;"

            "feature feat1;"

            "container cont {"
                "must \"false()\";"
                "list l1 {"
                    "key \"k\";"
                    "leaf k {"
                        "type string;"
                    "}"
                    "action act {"
                        "if-feature feat1;"
                        "input {"
                            "must \"../../lf1 = 'true'\";"
                            "leaf lf2 {"
                                "type leafref {"
                                    "path /lf3;"
                                "}"
                            "}"
                        "}"
                        "output {"
                            "must \"../../lf1 = 'true2'\";"
                            "leaf lf2 {"
                                "type leafref {"
                                    "path /lf4;"
                                "}"
                            "}"
                        "}"
                    "}"
                "}"

                "leaf lf1 {"
                    "type string;"
                "}"
            "}"

            "leaf lf3 {"
                "type string;"
            "}"

            "leaf lf4 {"
                "type string;"
            "}"
        "}";

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &ctx));
    assert_non_null(ly_ctx_load_module(ctx, "ietf-netconf-with-defaults", "2011-06-01"));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_a, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_b, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_c, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_d, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_e, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_f, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_g, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_h, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_i, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_j, LYS_IN_YANG, NULL));

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
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("When condition \"/cont/b = 'val_b'\" not satisfied. /a:c");

    data = "<cont xmlns=\"urn:tests:a\"><b>val_b</b></cont><c xmlns=\"urn:tests:a\">hey</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_string_equal("c", tree->next->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);

    data = "<cont xmlns=\"urn:tests:a\"><a>val</a><b>val_b</b></cont><c xmlns=\"urn:tests:a\">val_c</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_string_equal("a", lyd_node_children(tree, 0)->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, lyd_node_children(tree, 0)->flags);
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
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"choic\" instance does not exist. /b:choic");

    data = "<l xmlns=\"urn:tests:b\">string</l><d xmlns=\"urn:tests:b\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a><c xmlns=\"urn:tests:b\">string2</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    *state = NULL;
}

static void
test_minmax(void **state)
{
    *state = test_minmax;

    const char *data;
    struct lyd_node *tree;

    data = "<d xmlns=\"urn:tests:c\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>"
    "<l xmlns=\"urn:tests:c\">val3</l>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
    "<l xmlns=\"urn:tests:c\">val1</l>"
    "<l xmlns=\"urn:tests:c\">val2</l>"
    "<l xmlns=\"urn:tests:c\">val3</l>"
    "<lt xmlns=\"urn:tests:c\"><k>val1</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val2</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val3</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val4</k></lt>"
    "<lt xmlns=\"urn:tests:c\"><k>val5</k></lt>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
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
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>same</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<l1>not-same</l1>"
    "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val1</k>"
        "<l1>same</l1>"
    "</lt>"
    "<lt xmlns=\"urn:tests:d\">"
        "<k>val2</k>"
        "<l1>same</l1>"
    "</lt>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
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
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

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
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

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
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
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
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

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
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
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
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
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
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l5 l6\" not satisfied in \"/d:lt2[k='val5']\" and \"/d:lt2[k='val3']\". /d:lt2[k='val3']");

    *state = NULL;
}

static void
test_dup(void **state)
{
    *state = test_dup;

    const char *data;
    struct lyd_node *tree;

    data = "<d xmlns=\"urn:tests:e\">25</d><d xmlns=\"urn:tests:e\">50</d>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"d\". /e:d");

    data = "<lt xmlns=\"urn:tests:e\"><k>A</k></lt><lt xmlns=\"urn:tests:e\"><k>B</k></lt><lt xmlns=\"urn:tests:e\"><k>A</k></lt>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"lt\". /e:lt[k='A']");

    data = "<ll xmlns=\"urn:tests:e\">A</ll><ll xmlns=\"urn:tests:e\">B</ll><ll xmlns=\"urn:tests:e\">B</ll>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"ll\". /e:ll[.='B']");

    data = "<cont xmlns=\"urn:tests:e\"></cont><cont xmlns=\"urn:tests:e\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"cont\". /e:cont");

    /* same tests again but using hashes */
    data = "<cont xmlns=\"urn:tests:e\"><d>25</d><d>50</d><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll></cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"d\". /e:cont/d");

    data = "<cont xmlns=\"urn:tests:e\"><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll>"
        "<lt><k>a</k></lt><lt><k>b</k></lt><lt><k>c</k></lt><lt><k>d</k></lt><lt><k>c</k></lt></cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"lt\". /e:cont/lt[k='c']");

    data = "<cont xmlns=\"urn:tests:e\"><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll>"
        "<ll>a</ll><ll>b</ll><ll>c</ll><ll>d</ll><ll>d</ll></cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"ll\". /e:cont/ll[.='d']");

    /* cases */
    data = "<l xmlns=\"urn:tests:e\">a</l><l xmlns=\"urn:tests:e\">b</l><l xmlns=\"urn:tests:e\">c</l><l xmlns=\"urn:tests:e\">b</l>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"l\". /e:l[.='b']");

    data = "<l xmlns=\"urn:tests:e\">a</l><l xmlns=\"urn:tests:e\">b</l><l xmlns=\"urn:tests:e\">c</l><a xmlns=\"urn:tests:e\">aa</a>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Data for both cases \"a\" and \"b\" exist. /e:choic");

    *state = NULL;
}

static void
test_defaults(void **state)
{
    *state = test_defaults;

    char *str;
    struct lyd_node *tree, *node, *diff;
    const struct lys_module *mod = ly_ctx_get_module_latest(ctx, "f");

    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    /* get defaults */
    tree = NULL;
    assert_int_equal(lyd_validate_module(&tree, mod, 0, &diff), LY_SUCCESS);
    assert_non_null(tree);
    assert_non_null(diff);

    /* check all defaults exist */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
        "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>"
        "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>"
        "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>"
        "<d xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>"
        "<cont xmlns=\"urn:tests:f\">"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>"
            "<d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>"
        "</cont>");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
        "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def1</ll1>"
        "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def2</ll1>"
        "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def3</ll1>"
        "<d xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">dflt1</ll2>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">dflt2</ll2>"
        "<cont xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<ll1 yang:operation=\"create\">def1</ll1>"
            "<ll1 yang:operation=\"create\">def2</ll1>"
            "<ll1 yang:operation=\"create\">def3</ll1>"
            "<d yang:operation=\"create\">15</d>"
            "<ll2 yang:operation=\"create\">dflt1</ll2>"
            "<ll2 yang:operation=\"create\">dflt2</ll2>"
        "</cont>"
        );
    ly_out_reset(out);
    lyd_free_siblings(diff);

    /* create another explicit case and validate */
    assert_int_equal(lyd_new_term(NULL, mod, "l", "value", &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
        "<l xmlns=\"urn:tests:f\">value</l>"
        "<d xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>"
        "<cont xmlns=\"urn:tests:f\">"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>"
            "<d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>"
        "</cont>");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
        "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def1</ll1>"
        "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def2</ll1>"
        "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def3</ll1>"
        );
    ly_out_reset(out);
    lyd_free_siblings(diff);

    /* create explicit leaf-list and leaf and validate */
    assert_int_equal(lyd_new_term(NULL, mod, "d", "15", &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll2", "dflt2", &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
        "<l xmlns=\"urn:tests:f\">value</l>"
        "<d xmlns=\"urn:tests:f\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>"
        "<cont xmlns=\"urn:tests:f\">"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>"
            "<d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>"
        "</cont>");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
        "<d xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">dflt1</ll2>"
        "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">dflt2</ll2>"
        );
    ly_out_reset(out);
    lyd_free_siblings(diff);

    /* create first explicit container, which should become implicit */
    assert_int_equal(lyd_new_inner(NULL, mod, "cont", &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
        "<l xmlns=\"urn:tests:f\">value</l>"
        "<d xmlns=\"urn:tests:f\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>"
        "<cont xmlns=\"urn:tests:f\">"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>"
            "<d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>"
        "</cont>");
    ly_out_reset(out);

    /* check diff */
    assert_null(diff);

    /* create second explicit container, which should become implicit, so the first tree node should be removed */
    assert_int_equal(lyd_new_inner(NULL, mod, "cont", &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
        "<l xmlns=\"urn:tests:f\">value</l>"
        "<d xmlns=\"urn:tests:f\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>"
        "<cont xmlns=\"urn:tests:f\">"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>"
            "<ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>"
            "<d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>"
            "<ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>"
        "</cont>");
    ly_out_reset(out);

    /* check diff */
    assert_null(diff);

    /* similar changes for nested defaults */
    assert_int_equal(lyd_new_term(tree->prev, NULL, "ll1", "def3", NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(tree->prev, NULL, "d", "5", NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(tree->prev, NULL, "ll2", "non-dflt", NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
        "<l xmlns=\"urn:tests:f\">value</l>"
        "<d xmlns=\"urn:tests:f\">15</d>"
        "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>"
        "<cont xmlns=\"urn:tests:f\">"
            "<ll1>def3</ll1>"
            "<d>5</d>"
            "<ll2>non-dflt</ll2>"
        "</cont>");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
        "<cont xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<ll1 yang:operation=\"delete\">def1</ll1>"
            "<ll1 yang:operation=\"delete\">def2</ll1>"
            "<ll1 yang:operation=\"delete\">def3</ll1>"
            "<d yang:operation=\"delete\">15</d>"
            "<ll2 yang:operation=\"delete\">dflt1</ll2>"
            "<ll2 yang:operation=\"delete\">dflt2</ll2>"
        "</cont>"
        );
    ly_out_reset(out);
    lyd_free_siblings(diff);

    lyd_free_siblings(tree);
    ly_out_free(out, NULL, 1);

    *state = NULL;
}

static void
test_iffeature(void **state)
{
    *state = test_iffeature;

    const char *data;
    struct lyd_node *tree;
    const struct lys_module *mod = ly_ctx_get_module_latest(ctx, "g");

    /* get empty data */
    tree = NULL;
    assert_int_equal(lyd_validate_module(&tree, mod, 0, NULL), LY_SUCCESS);
    assert_null(tree);

    /* disabled by f1 */
    data =
    "<cont xmlns=\"urn:tests:g\">"
        "<d>51</d>"
    "</cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Data are disabled by \"cont\" schema node if-feature. /g:cont");

    /* enable f1 */
    assert_int_equal(lys_feature_enable(mod, "f1"), LY_SUCCESS);

    /* get data with default container */
    assert_int_equal(lyd_validate_module(&tree, mod, 0, NULL), LY_SUCCESS);
    assert_non_null(tree);
    lyd_free_siblings(tree);

    /* disabled by f2 */
    data =
    "<cont xmlns=\"urn:tests:g\">"
        "<cont2>"
            "<e>val</e>"
        "</cont2>"
    "</cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Data are disabled by \"cont2\" schema node if-feature. /g:cont/cont2");

    data =
    "<cont xmlns=\"urn:tests:g\">"
        "<a>val</a>"
    "</cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Data are disabled by \"choic\" schema node if-feature. /g:cont/a");

    /* enable f3 */
    assert_int_equal(lys_feature_enable(mod, "f3"), LY_SUCCESS);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    /* disabled by f2 */
    data =
    "<cont xmlns=\"urn:tests:g\">"
        "<l>val</l>"
    "</cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Data are disabled by \"b\" schema node if-feature. /g:cont/l");

    /* enable f2 */
    assert_int_equal(lys_feature_enable(mod, "f2"), LY_SUCCESS);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    /* try separate validation */
    assert_int_equal(lys_feature_disable(mod, "f1"), LY_SUCCESS);
    assert_int_equal(lys_feature_disable(mod, "f2"), LY_SUCCESS);
    assert_int_equal(lys_feature_disable(mod, "f3"), LY_SUCCESS);

    data =
    "<cont xmlns=\"urn:tests:g\">"
        "<l>val</l>"
        "<d>51</d>"
        "<cont2>"
            "<e>val</e>"
        "</cont2>"
    "</cont>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_EVALID, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));
    logbuf_assert("Data are disabled by \"cont\" schema node if-feature. /g:cont");

    assert_int_equal(lys_feature_enable(mod, "f1"), LY_SUCCESS);

    assert_int_equal(LY_EVALID, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));
    logbuf_assert("Data are disabled by \"b\" schema node if-feature. /g:cont/l");

    assert_int_equal(lys_feature_enable(mod, "f2"), LY_SUCCESS);

    assert_int_equal(LY_SUCCESS, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));

    lyd_free_siblings(tree);

    *state = NULL;
}

static void
test_state(void **state)
{
    *state = test_state;

    const char *data;
    struct lyd_node *tree;

    data =
    "<cont xmlns=\"urn:tests:h\">"
        "<cont2>"
            "<l>val</l>"
        "</cont2>"
    "</cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_NO_STATE, 0, &tree));
    assert_null(tree);
    logbuf_assert("Invalid state data node \"cont2\" found. /h:cont/cont2");

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT | LYD_VALIDATE_NO_STATE, &tree));
    assert_null(tree);
    logbuf_assert("Invalid state data node \"cont2\" found. /h:cont/cont2");

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_EVALID, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT | LYD_VALIDATE_NO_STATE, NULL));
    logbuf_assert("Invalid state data node \"cont2\" found. /h:cont/cont2");

    lyd_free_siblings(tree);

    *state = NULL;
}

static void
test_must(void **state)
{
    *state = test_must;

    const char *data;
    struct lyd_node *tree;

    data =
    "<cont xmlns=\"urn:tests:i\">"
        "<l>wrong</l>"
        "<l2>val</l2>"
    "</cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Must condition \"../l = 'right'\" not satisfied. /i:cont/l2");

    data =
    "<cont xmlns=\"urn:tests:i\">"
        "<l>right</l>"
        "<l2>val</l2>"
    "</cont>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_tree(tree);

    *state = NULL;
}

static void
test_action(void **state)
{
    *state = test_action;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op_tree;
    const struct lys_module *mod;

    data =
    "<cont xmlns=\"urn:tests:j\">"
        "<l1>"
            "<k>val1</k>"
            "<act>"
                "<lf2>target</lf2>"
            "</act>"
        "</l1>"
    "</cont>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(ctx, in, LYD_XML, &op_tree, NULL));
    assert_non_null(op_tree);

    /* missing leafref */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, NULL, LYD_VALIDATE_OP_RPC, NULL));
    logbuf_assert("Invalid leafref value \"target\" - no target instance \"/lf3\" with the same value."
        " /j:cont/l1[k='val1']/act/lf2");
    ly_in_free(in, 0);

    data =
    "<cont xmlns=\"urn:tests:j\">"
        "<lf1>not true</lf1>"
    "</cont>"
    "<lf3 xmlns=\"urn:tests:j\">target</lf3>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_TRUSTED, 0, &tree));
    assert_non_null(tree);

    /* disabled if-feature */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_RPC, NULL));
    logbuf_assert("Data are disabled by \"act\" schema node if-feature. /j:cont/l1[k='val1']/act");

    mod = ly_ctx_get_module_latest(ctx, "j");
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "feat1"));

    /* input must false */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_RPC, NULL));
    logbuf_assert("Must condition \"../../lf1 = 'true'\" not satisfied. /j:cont/l1[k='val1']/act");

    lyd_free_siblings(tree);
    data =
    "<cont xmlns=\"urn:tests:j\">"
        "<lf1>true</lf1>"
    "</cont>"
    "<lf3 xmlns=\"urn:tests:j\">target</lf3>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_TRUSTED, 0, &tree));
    assert_non_null(tree);

    /* success */
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_RPC, NULL));

    lys_feature_disable(mod, "feat1");
    lyd_free_tree(op_tree);
    lyd_free_siblings(tree);

    *state = NULL;
}

static void
test_reply(void **state)
{
    *state = test_reply;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op_tree, *request;
    const struct lys_module *mod;

    data =
    "<cont xmlns=\"urn:tests:j\">"
        "<l1>"
            "<k>val1</k>"
            "<act>"
                "<lf2>target</lf2>"
            "</act>"
        "</l1>"
    "</cont>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(ctx, in, LYD_XML, &request, NULL));
    assert_non_null(request);
    ly_in_free(in, 0);

    data = "<lf2 xmlns=\"urn:tests:j\">target</lf2>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_reply(request, in, LYD_XML, &op_tree, NULL));
    lyd_free_all(request);
    assert_non_null(op_tree);
    ly_in_free(in, 0);

    /* missing leafref */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, NULL, LYD_VALIDATE_OP_REPLY, NULL));
    logbuf_assert("Invalid leafref value \"target\" - no target instance \"/lf4\" with the same value."
        " /j:cont/l1[k='val1']/act/lf2");

    data =
    "<cont xmlns=\"urn:tests:j\">"
        "<lf1>not true</lf1>"
    "</cont>"
    "<lf4 xmlns=\"urn:tests:j\">target</lf4>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_TRUSTED, 0, &tree));
    assert_non_null(tree);

    /* disabled if-feature */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_REPLY, NULL));
    logbuf_assert("Data are disabled by \"act\" schema node if-feature. /j:cont/l1[k='val1']/act");

    mod = ly_ctx_get_module_latest(ctx, "j");
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "feat1"));

    /* input must false */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_REPLY, NULL));
    logbuf_assert("Must condition \"../../lf1 = 'true2'\" not satisfied. /j:cont/l1[k='val1']/act");

    lyd_free_siblings(tree);
    data =
    "<cont xmlns=\"urn:tests:j\">"
        "<lf1>true2</lf1>"
    "</cont>"
    "<lf4 xmlns=\"urn:tests:j\">target</lf4>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_TRUSTED, 0, &tree));
    assert_non_null(tree);

    /* success */
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_REPLY, NULL));

    lys_feature_disable(mod, "feat1");
    lyd_free_tree(op_tree);
    lyd_free_siblings(tree);

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
        cmocka_unit_test_teardown(test_dup, teardown_s),
        cmocka_unit_test_teardown(test_defaults, teardown_s),
        cmocka_unit_test_teardown(test_iffeature, teardown_s),
        cmocka_unit_test_teardown(test_state, teardown_s),
        cmocka_unit_test_teardown(test_must, teardown_s),
        cmocka_unit_test_teardown(test_action, teardown_s),
        cmocka_unit_test_teardown(test_reply, teardown_s),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
