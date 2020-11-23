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

#include "libyang.h"
#include "tests/config.h"
#include "utests.h"

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
            "module defaults {\n"
            "    yang-version 1.1;\n"
            "    namespace \"urn:libyang:tests:defaults\";\n"
            "    prefix df;\n"
            "\n"
            "    feature unhide;\n"
            "\n"
            "    typedef defint32 {\n"
            "        type int32;\n"
            "        default \"42\";\n"
            "    }\n"
            "\n"
            "    leaf hiddenleaf {\n"
            "        if-feature \"unhide\";\n"
            "        type int32;\n"
            "        default \"42\";\n"
            "    }\n"
            "\n"
            "    container df {\n"
            "        leaf foo {\n"
            "            type defint32;\n"
            "        }\n"
            "\n"
            "        leaf hiddenleaf {\n"
            "            if-feature \"unhide\";\n"
            "            type int32;\n"
            "            default \"42\";\n"
            "        }\n"
            "\n"
            "        container bar {\n"
            "            presence \"\";\n"
            "            leaf hi {\n"
            "                type int32;\n"
            "                default \"42\";\n"
            "            }\n"
            "\n"
            "            leaf ho {\n"
            "                type int32;\n"
            "                mandatory true;\n"
            "            }\n"
            "        }\n"
            "\n"
            "        leaf-list llist {\n"
            "            type defint32;\n"
            "            ordered-by user;\n"
            "        }\n"
            "\n"
            "        leaf-list dllist {\n"
            "            type uint8;\n"
            "            default \"1\";\n"
            "            default \"2\";\n"
            "            default \"3\";\n"
            "        }\n"
            "\n"
            "        list list {\n"
            "            key \"name\";\n"
            "            leaf name {\n"
            "                type string;\n"
            "            }\n"
            "\n"
            "            leaf value {\n"
            "                type int32;\n"
            "                default \"42\";\n"
            "            }\n"
            "        }\n"
            "\n"
            "        choice select {\n"
            "            default \"a\";\n"
            "            case a {\n"
            "                choice a {\n"
            "                    leaf a1 {\n"
            "                        type int32;\n"
            "                        default \"42\";\n"
            "                    }\n"
            "\n"
            "                    leaf a2 {\n"
            "                        type int32;\n"
            "                        default \"24\";\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "\n"
            "            leaf b {\n"
            "                type string;\n"
            "            }\n"
            "\n"
            "            container c {\n"
            "                presence \"\";\n"
            "                leaf x {\n"
            "                    type int32;\n"
            "                    default \"42\";\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "\n"
            "        choice select2 {\n"
            "            default \"s2b\";\n"
            "            leaf s2a {\n"
            "                type int32;\n"
            "                default \"42\";\n"
            "            }\n"
            "\n"
            "            case s2b {\n"
            "                choice s2b {\n"
            "                    default \"b1\";\n"
            "                    case b1 {\n"
            "                        leaf b1_1 {\n"
            "                            type int32;\n"
            "                            default \"42\";\n"
            "                        }\n"
            "\n"
            "                        leaf b1_2 {\n"
            "                            type string;\n"
            "                        }\n"
            "\n"
            "                        leaf b1_status {\n"
            "                            type int32;\n"
            "                            default \"42\";\n"
            "                            config false;\n"
            "                        }\n"
            "                    }\n"
            "\n"
            "                    leaf b2 {\n"
            "                        type int32;\n"
            "                        default \"42\";\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "\n"
            "    container hidden {\n"
            "        leaf foo {\n"
            "            type int32;\n"
            "            default \"42\";\n"
            "        }\n"
            "\n"
            "        leaf baz {\n"
            "            type int32;\n"
            "            default \"42\";\n"
            "        }\n"
            "\n"
            "        leaf papa {\n"
            "            type int32;\n"
            "            default \"42\";\n"
            "            config false;\n"
            "        }\n"
            "    }\n"
            "\n"
            "    rpc rpc1 {\n"
            "        input {\n"
            "            leaf inleaf1 {\n"
            "                type string;\n"
            "            }\n"
            "\n"
            "            leaf inleaf2 {\n"
            "                type string;\n"
            "                default \"def1\";\n"
            "            }\n"
            "        }\n"
            "\n"
            "        output {\n"
            "            leaf outleaf1 {\n"
            "                type string;\n"
            "                default \"def2\";\n"
            "            }\n"
            "\n"
            "            leaf outleaf2 {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "\n"
            "    notification notif {\n"
            "        leaf ntfleaf1 {\n"
            "            type string;\n"
            "            default \"def3\";\n"
            "        }\n"
            "\n"
            "        leaf ntfleaf2 {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "}\n";

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
    const char *xml =
            "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">\n"
            "  <enable-nacm>true</enable-nacm>\n"
            "  <read-default>permit</read-default>\n"
            "  <write-default>deny</write-default>\n"
            "  <exec-default>permit</exec-default>\n"
            "  <enable-external-groups>true</enable-external-groups>\n"
            "</nacm><df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo><b1_1>42</b1_1>\n"
            "</df><hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo><baz>42</baz></hidden>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);

    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);
    assert_null(st->diff1);

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);
}

static void
test_empty1(void **state)
{
    struct state *st = (*state);
    const char *xml =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n";

    st->first = NULL;
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);

    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n");
    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);
}

static void
test_empty2(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df><hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    st->second = NULL;

    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n");

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
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"create\">42</foo>\n"
            "</df>\n");

    free(st->xml);
    assert_int_equal(lyd_diff_siblings(lyd_child(st->first), NULL, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"delete\">42</foo>\n"
            "</df>\n");
}

static void
test_leaf(void **state)
{
    struct state *st = (*state);
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>41</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>40</foo>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>40</foo>\n"
            "</hidden>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"42\">41</foo>\n"
            "  <b1_1 yang:operation=\"create\">42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n");

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"41\">40</foo>\n"
            "  <b1_1 yang:operation=\"delete\">42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <foo>40</foo>\n"
            "</hidden>\n");

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2, 0), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"42\">40</foo>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-value=\"42\" yang:orig-default=\"false\">40</foo>\n"
            "  <baz yang:operation=\"delete\">42</baz>\n"
            "</hidden>\n");
}

static void
test_list(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <list>\n"
            "    <name>a</name>\n"
            "    <value>1</value>\n"
            "  </list>\n"
            "  <list>\n"
            "    <name>b</name>\n"
            "    <value>2</value>\n"
            "  </list>\n"
            "</df>\n";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <list>\n"
            "    <name>b</name>\n"
            "    <value>-2</value>\n"
            "  </list>\n"
            "  <list>\n"
            "    <name>c</name>\n"
            "    <value>3</value>\n"
            "  </list>\n"
            "</df>\n";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <list>\n"
            "    <name>b</name>\n"
            "    <value>-2</value>\n"
            "  </list>\n"
            "  <list>\n"
            "    <name>a</name>\n"
            "    <value>2</value>\n"
            "  </list>\n"
            "</df>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>a</name>\n"
            "    <value>1</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"none\">\n"
            "    <name>b</name>\n"
            "    <value yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\">-2</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"create\">\n"
            "    <name>c</name>\n"
            "    <value>3</value>\n"
            "  </list>\n"
            "</df>\n");
    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>c</name>\n"
            "    <value>3</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"create\">\n"
            "    <name>a</name>\n"
            "    <value>2</value>\n"
            "  </list>\n"
            "</df>\n");
    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2, 0), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"none\">\n"
            "    <name>a</name>\n"
            "    <value yang:operation=\"replace\" yang:orig-value=\"1\" yang:orig-default=\"false\">2</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"none\">\n"
            "    <name>b</name>\n"
            "    <value yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\">-2</value>\n"
            "  </list>\n"
            "</df>\n");
}

static void
test_userord_llist(void **state)
{
    struct state *st = (*state);
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>2</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>5</llist>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>2</llist>\n"
            "  <llist>5</llist>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>5</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>2</llist>\n"
            "</df>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"\">1</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"\">5</llist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2, 0), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>\n"
            "  <llist yang:orig-value=\"\" yang:operation=\"delete\">1</llist>\n"
            "  <llist yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"\" yang:operation=\"replace\">5</llist>\n"
            "</df>\n");
}

static void
test_userord_llist2(void **state)
{
    struct state *st = (*state);
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <list><name>a</name><value>1</value></list>\n"
            "  <llist>2</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>4</llist>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <list><name>a</name><value>1</value></list>\n"
            "  <llist>2</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>4</llist>\n"
            "  <llist>1</llist>\n"
            "  <list><name>a</name><value>1</value></list>\n"
            "  <llist>3</llist>\n"
            "</df>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"2\">4</llist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"1\" yang:value=\"\">4</llist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2, 0), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"\">4</llist>\n"
            "  <llist yang:orig-value=\"1\" yang:operation=\"delete\">2</llist>\n"
            "</df>\n");
}

static void
test_userord_mix(void **state)
{
    struct state *st = (*state);
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>2</llist>\n"
            "  <llist>3</llist>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>3</llist>\n"
            "  <llist>1</llist>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "</df>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml1, LYD_XML, LYD_PARSE_ONLY, 0, &st->first));
    assert_non_null(st->first);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml2, LYD_XML, LYD_PARSE_ONLY, 0, &st->second));
    assert_non_null(st->second);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, xml3, LYD_XML, LYD_PARSE_ONLY, 0, &st->third));
    assert_non_null(st->third);

    /* diff1 */
    assert_int_equal(lyd_diff_siblings(st->first, st->second, 0, &st->diff1), LY_SUCCESS);

    assert_non_null(st->diff1);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"1\" yang:value=\"\">3</llist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, 0, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"\">1</llist>\n"
            "  <llist yang:operation=\"create\" yang:value=\"1\">4</llist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2, 0), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"1\" yang:value=\"\">3</llist>\n"
            "  <llist yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"\" yang:operation=\"replace\">1</llist>\n"
            "  <llist yang:value=\"1\" yang:operation=\"create\">4</llist>\n"
            "</df>\n");
}

static void
test_wd(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>41</foo>\n"
            "  <dllist>4</dllist>\n"
            "</df>\n";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <dllist>4</dllist>\n"
            "  <dllist>1</dllist>\n"
            "</df>\n";

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
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"true\" yang:orig-value=\"42\">41</foo>\n"
            "  <dllist yang:operation=\"delete\">1</dllist>\n"
            "  <dllist yang:operation=\"delete\">2</dllist>\n"
            "  <dllist yang:operation=\"delete\">3</dllist>\n"
            "  <dllist yang:operation=\"create\">4</dllist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->first, st->diff1), LY_SUCCESS);
    lyd_print_mem(&st->xml1, st->first, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    lyd_print_mem(&st->xml2, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* diff2 */
    assert_int_equal(lyd_diff_siblings(st->second, st->third, LYD_DIFF_DEFAULTS, &st->diff2), LY_SUCCESS);

    assert_non_null(st->diff2);
    free(st->xml);
    lyd_print_mem(&st->xml, st->diff2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"41\">42</foo>\n"
            "  <dllist yang:operation=\"create\">1</dllist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&st->second, st->diff2), LY_SUCCESS);
    free(st->xml1);
    lyd_print_mem(&st->xml1, st->second, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    free(st->xml2);
    lyd_print_mem(&st->xml2, st->third, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(st->xml1, st->xml2);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&st->diff1, st->diff2, 0), LY_SUCCESS);

    free(st->xml);
    lyd_print_mem(&st->xml, st->diff1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL);
    assert_string_equal(st->xml,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:orig-default=\"true\" yang:operation=\"none\">42</foo>\n"
            "  <dllist yang:operation=\"none\" yang:orig-default=\"true\">1</dllist>\n"
            "  <dllist yang:operation=\"delete\">2</dllist>\n"
            "  <dllist yang:operation=\"delete\">3</dllist>\n"
            "  <dllist yang:operation=\"create\">4</dllist>\n"
            "</df>\n");
}

int
main(void)
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
