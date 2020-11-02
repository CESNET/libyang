/**
 * @file test_diff.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief tests for lyd_diff()
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
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "libyang.h"
#include "tests/config.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *first;
    struct lyd_node *second;
    struct lyd_node *third;
    struct lyd_node *diff1;
    struct lyd_node *diff2;
    char *xml;
    char *xml1;
    char *xml2;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schema =
    "module defaults {"
        "yang-version 1.1;"
        "namespace \"urn:libyang:tests:defaults\";"
        "prefix df;"

        "feature unhide;"

        "typedef defint32 {"
            "type int32;"
            "default \"42\";"
        "}"

        "leaf hiddenleaf {"
            "if-feature \"unhide\";"
            "type int32;"
            "default \"42\";"
        "}"

        "container df {"
            "leaf foo {"
                "type defint32;"
            "}"

            "leaf hiddenleaf {"
                "if-feature \"unhide\";"
                "type int32;"
                "default \"42\";"
            "}"

            "container bar {"
                "presence \"\";"
                "leaf hi {"
                    "type int32;"
                    "default \"42\";"
                "}"

                "leaf ho {"
                    "type int32;"
                    "mandatory true;"
                "}"
            "}"

            "leaf-list llist {"
                "type defint32;"
                "ordered-by user;"
            "}"

            "leaf-list dllist {"
                "type uint8;"
                "default \"1\";"
                "default \"2\";"
                "default \"3\";"
            "}"

            "list list {"
                "key \"name\";"
                "leaf name {"
                    "type string;"
                "}"

                "leaf value {"
                    "type int32;"
                    "default \"42\";"
                "}"
            "}"

            "choice select {"
                "default \"a\";"
                "case a {"
                    "choice a {"
                        "leaf a1 {"
                            "type int32;"
                            "default \"42\";"
                        "}"

                        "leaf a2 {"
                            "type int32;"
                            "default \"24\";"
                        "}"
                    "}"
                "}"

                "leaf b {"
                    "type string;"
                "}"

                "container c {"
                    "presence \"\";"
                    "leaf x {"
                        "type int32;"
                        "default \"42\";"
                    "}"
                "}"
            "}"

            "choice select2 {"
                "default \"s2b\";"
                "leaf s2a {"
                    "type int32;"
                    "default \"42\";"
                "}"

                "case s2b {"
                    "choice s2b {"
                        "default \"b1\";"
                        "case b1 {"
                            "leaf b1_1 {"
                                "type int32;"
                                "default \"42\";"
                            "}"

                            "leaf b1_2 {"
                                "type string;"
                            "}"

                            "leaf b1_status {"
                                "type int32;"
                                "default \"42\";"
                                "config false;"
                            "}"
                        "}"

                        "leaf b2 {"
                            "type int32;"
                            "default \"42\";"
                        "}"
                    "}"
                "}"
            "}"
        "}"

        "container hidden {"
            "leaf foo {"
                "type int32;"
                "default \"42\";"
            "}"

            "leaf baz {"
                "type int32;"
                "default \"42\";"
            "}"

            "leaf papa {"
                "type int32;"
                "default \"42\";"
                "config false;"
            "}"
        "}"

        "rpc rpc1 {"
            "input {"
                "leaf inleaf1 {"
                    "type string;"
                "}"

                "leaf inleaf2 {"
                    "type string;"
                    "default \"def1\";"
                "}"
            "}"

            "output {"
                "leaf outleaf1 {"
                    "type string;"
                    "default \"def2\";"
                "}"

                "leaf outleaf2 {"
                    "type string;"
                "}"
            "}"
        "}"

        "notification notif {"
            "leaf ntfleaf1 {"
                "type string;"
                "default \"def3\";"
            "}"

            "leaf ntfleaf2 {"
                "type string;"
            "}"
        "}"
    "}";

    (*state) = st = calloc(1, sizeof *st);
    assert_non_null(st);
    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &st->ctx));
    assert_non_null(ly_ctx_load_module(st->ctx, "ietf-netconf-acm", "2018-02-14", NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, schema, LYS_IN_YANG, NULL));

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_siblings(st->first);
    lyd_free_siblings(st->second);
    lyd_free_siblings(st->third);
    lyd_free_siblings(st->diff1);
    lyd_free_siblings(st->diff2);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st->xml1);
    free(st->xml2);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_invalid(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\"><foo>42</foo></df>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    st->second = NULL;

    assert_int_equal(lyd_diff_siblings(st->first, lyd_child(st->first), 0, &st->diff1), LY_EINVAL);

    assert_int_equal(lyd_diff_siblings(NULL, NULL, 0, NULL), LY_EINVAL);
}

static void
test_same(void **state)
{
    struct state *st = (*state);
    const char *xml = "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">"
                        "<enable-nacm>true</enable-nacm>"
                        "<read-default>permit</read-default>"
                        "<write-default>deny</write-default>"
                        "<exec-default>permit</exec-default>"
                        "<enable-external-groups>true</enable-external-groups>"
                      "</nacm><df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);

    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);
    assert_null(st->diff1);

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);
}

static void
test_empty1(void **state)
{
    struct state *st = (*state);
    const char *xml =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo><b1_1>42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>";

    st->first = NULL;
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);

    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">"
            "<foo>42</foo><b1_1>42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);
}

static void
test_empty2(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    st->second = NULL;

    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">"
            "<foo>42</foo><b1_1>42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    assert_ptr_equal(st->first, st->second);
}

static void
test_empty_nested(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\"><foo>42</foo></df>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    st->second = NULL;

    assert_int_equal(lyd_diff_siblings(NULL, NULL, 0, &st->diff1), LY_SUCCESS);
    assert_null(st->diff1);

    assert_int_equal(lyd_diff_siblings(NULL, lyd_child(st->first), 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"create\">42</foo>"
        "</df>"
    );

    free(st->xml);
    assert_int_equal(lyd_diff_siblings(lyd_child(st->first), NULL, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"delete\">42</foo>"
        "</df>"
    );
}

static void
test_leaf(void **state)
{
    struct state *st = (*state);
    const char *xml1 =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>42</foo><baz>42</baz>"
        "</hidden>";
    const char *xml2 =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>41</foo><b1_1>42</b1_1>"
        "</df>";
    const char *xml3 =
        "<df xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>40</foo>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\">"
            "<foo>40</foo>"
        "</hidden>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"42\">41</foo>"
            "<b1_1 yang:operation=\"create\">42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">"
            "<foo>42</foo>"
            "<baz>42</baz>"
        "</hidden>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"41\">40</foo>"
            "<b1_1 yang:operation=\"delete\">42</b1_1>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">"
            "<foo>40</foo>"
        "</hidden>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"42\">40</foo>"
        "</df>"
        "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"42\">40</foo>"
            "<baz yang:operation=\"delete\">42</baz>"
        "</hidden>"
    );
}

static void
test_list(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>a</name><value>1</value></list>"
                         "<list><name>b</name><value>2</value></list>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>b</name><value>-2</value></list>"
                         "<list><name>c</name><value>3</value></list>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>b</name><value>-2</value></list>"
                         "<list><name>a</name><value>2</value></list>"
                       "</df>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<list yang:operation=\"delete\">"
                "<name>a</name>"
                "<value>1</value>"
            "</list>"
            "<list yang:operation=\"none\">"
                "<name>b</name>"
                "<value yang:operation=\"replace\" yang:orig-value=\"2\">-2</value>"
            "</list>"
            "<list yang:operation=\"create\">"
                "<name>c</name>"
                "<value>3</value>"
            "</list>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<list yang:operation=\"delete\">"
                "<name>c</name>"
                "<value>3</value>"
            "</list>"
            "<list yang:operation=\"create\">"
                "<name>a</name>"
                "<value>2</value>"
            "</list>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<list yang:operation=\"none\">"
                "<name>a</name>"
                "<value yang:operation=\"replace\" yang:orig-value=\"1\">2</value>"
            "</list>"
            "<list yang:operation=\"none\">"
                "<name>b</name>"
                "<value yang:operation=\"replace\" yang:orig-value=\"2\">-2</value>"
            "</list>"
        "</df>"
    );
}

static void
test_userord_llist(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                         "<llist>4</llist>"
                         "<llist>5</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                         "<llist>2</llist>"
                         "<llist>5</llist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>5</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                         "<llist>2</llist>"
                       "</df>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"\">1</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"2\" yang:value=\"\">5</llist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>"
            "<llist yang:orig-value=\"\" yang:operation=\"delete\">1</llist>"
            "<llist yang:orig-value=\"2\" yang:value=\"\" yang:operation=\"replace\">5</llist>"
        "</df>"
    );
}

static void
test_userord_llist2(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                         "<llist>4</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>2</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>4</llist>"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>3</llist>"
                       "</df>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"2\">4</llist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"1\" yang:value=\"\">4</llist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"\">4</llist>"
            "<llist yang:orig-value=\"1\" yang:operation=\"delete\">2</llist>"
        "</df>"
    );
}

static void
test_userord_mix(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>3</llist>"
                         "<llist>1</llist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                       "</df>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"1\" yang:value=\"\">3</llist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"replace\" yang:orig-value=\"3\" yang:value=\"\">1</llist>"
            "<llist yang:operation=\"create\" yang:value=\"1\">4</llist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>"
            "<llist yang:operation=\"replace\" yang:orig-value=\"1\" yang:value=\"\">3</llist>"
            "<llist yang:orig-value=\"3\" yang:value=\"\" yang:operation=\"replace\">1</llist>"
            "<llist yang:value=\"1\" yang:operation=\"create\">4</llist>"
        "</df>"
    );
}

static void
test_wd(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<foo>41</foo><dllist>4</dllist>"
                       "</df>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<foo>42</foo><dllist>4</dllist><dllist>1</dllist>"
                       "</df>";

    mod = ly_ctx_get_module_implemented(st->ctx, "defaults");
    assert_non_null(mod);

    st->first = NULL;
    assert_int_equal(lyd_validate_module(&st->first, mod, 0, NULL), LY_SUCCESS);
    assert_ptr_not_equal(st->first, NULL);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, LYD_DIFF_DEFAULTS, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-default=\"true\" yang:orig-value=\"42\">41</foo>"
            "<dllist yang:operation=\"delete\">1</dllist>"
            "<dllist yang:operation=\"delete\">2</dllist>"
            "<dllist yang:operation=\"delete\">3</dllist>"
            "<dllist yang:operation=\"create\">4</dllist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, LYD_DIFF_DEFAULTS, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:operation=\"replace\" yang:orig-value=\"41\">42</foo>"
            "<dllist yang:operation=\"create\">1</dllist>"
        "</df>"
    );

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK);
    assert_string_equal(st->xml,
        "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">"
            "<foo yang:orig-default=\"true\" yang:operation=\"none\">42</foo>"
            "<dllist yang:operation=\"none\" yang:orig-default=\"true\">1</dllist>"
            "<dllist yang:operation=\"delete\">2</dllist>"
            "<dllist yang:operation=\"delete\">3</dllist>"
            "<dllist yang:operation=\"create\">4</dllist>"
        "</df>"
    );
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_invalid, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_same, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_empty1, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_empty2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_empty_nested, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_leaf, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_list, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_userord_llist, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_userord_llist2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_userord_mix, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_wd, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
