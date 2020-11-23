/**
 * @file test_merge.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief tests for complex data merges.
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
#include "utests.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *source;
    struct lyd_node *target;
    struct lyd_node *result;
};

static int
setup_dflt(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    /* libyang context */
    if (ly_ctx_new(NULL, 0, &st->ctx)) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_dflt(void **state)
{
    struct state *st = (*state);

    lyd_free_siblings(st->target);
    lyd_free_siblings(st->source);
    lyd_free_siblings(st->result);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_batch(void **state)
{
    struct state *st = (*state);
    uint32_t i;
    char *str;

    const char *start =
            "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
            "  <module>\n"
            "    <name>yang</name>\n"
            "    <revision>2016-02-11</revision>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "</modules-state>\n";
    const char *data[] = {
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-yang-library</name>\n"
        "    <revision>2016-02-01</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-acm</name>\n"
        "    <revision>2012-02-22</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-monitoring</name>\n"
        "    <revision>2010-10-04</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-with-defaults</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>yang</name>\n"
        "    <revision>2016-02-11</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-yang-library</name>\n"
        "    <revision>2016-02-01</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-acm</name>\n"
        "    <revision>2012-02-22</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
        "    <feature>writable-running</feature>\n"
        "    <feature>candidate</feature>\n"
        "    <feature>rollback-on-error</feature>\n"
        "    <feature>validate</feature>\n"
        "    <feature>startup</feature>\n"
        "    <feature>xpath</feature>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-monitoring</name>\n"
        "    <revision>2010-10-04</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-with-defaults</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n"
    };
    const char *output_template =
            "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
            "  <module>\n"
            "    <name>yang</name>\n"
            "    <revision>2016-02-11</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-yang-library</name>\n"
            "    <revision>2016-02-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-acm</name>\n"
            "    <revision>2012-02-22</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf</name>\n"
            "    <revision>2011-06-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
            "    <feature>writable-running</feature>\n"
            "    <feature>candidate</feature>\n"
            "    <feature>rollback-on-error</feature>\n"
            "    <feature>validate</feature>\n"
            "    <feature>startup</feature>\n"
            "    <feature>xpath</feature>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-monitoring</name>\n"
            "    <revision>2010-10-04</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-with-defaults</name>\n"
            "    <revision>2011-06-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "</modules-state>\n";
    struct ly_in *in = NULL;

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(start, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_data(st->ctx, in, LYD_XML, LYD_PARSE_ONLY, 0, &st->target));
    assert_non_null(st->target);

    for (i = 0; i < 11; ++i) {
        ly_in_memory(in, data[i]);
        assert_int_equal(LY_SUCCESS, lyd_parse_data(st->ctx, in, LYD_XML, LYD_PARSE_ONLY, 0, &st->source));
        assert_non_null(st->source);

        assert_int_equal(LY_SUCCESS, lyd_merge_siblings(&st->target, st->source, LYD_MERGE_DESTRUCT));
        st->source = NULL;
    }

    lyd_print_mem(&str, st->target, LYD_XML, 0);
    assert_string_equal(str, output_template);

    ly_in_free(in, 0);
    free(str);
}

static void
test_leaf(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
            "  namespace urn:x;"
            "  prefix x;"
            "    container A {"
            "      leaf f1 {type string;}"
            "      container B {"
            "        leaf f2 {type string;}"
            "      }"
            "    }"
            "  }";
    const char *trg = "<A xmlns=\"urn:x\"> <f1>block</f1> </A>";
    const char *src = "<A xmlns=\"urn:x\"> <f1>aa</f1> <B> <f2>bb</f2> </B> </A>";
    const char *result = "<A xmlns=\"urn:x\"><f1>aa</f1><B><f2>bb</f2></B></A>";
    char *printed = NULL;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, src, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->source));
    assert_non_null(st->source);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, trg, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->target));
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&st->target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_container(void **state)
{
    struct state *st = (*state);
    const char *sch =
            "module A {\n"
            "    namespace \"aa:A\";\n"
            "    prefix A;\n"
            "    container A {\n"
            "        leaf f1 {type string;}\n"
            "        container B {\n"
            "            leaf f2 {type string;}\n"
            "        }\n"
            "        container C {\n"
            "            leaf f3 {type string;}\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg = "<A xmlns=\"aa:A\"> <B> <f2>aaa</f2> </B> </A>";
    const char *src = "<A xmlns=\"aa:A\"> <C> <f3>bbb</f3> </C> </A>";
    const char *result = "<A xmlns=\"aa:A\"><B><f2>aaa</f2></B><C><f3>bbb</f3></C></A>";
    char *printed = NULL;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, src, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->source));
    assert_non_null(st->source);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, trg, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->target));
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&st->target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_list(void **state)
{
    struct state *st = (*state);
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "\n"
            "    container inner1 {\n"
            "        list b-list1 {\n"
            "            key p1;\n"
            "            leaf p1 {\n"
            "                type uint8;\n"
            "            }\n"
            "            leaf p2 {\n"
            "                type string;\n"
            "            }\n"
            "            leaf p3 {\n"
            "                type boolean;\n"
            "                default false;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>a</p2>\n"
            "    <p3>true</p3>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *src =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *result =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "    <p3>true</p3>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    char *printed = NULL;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, src, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->source));
    assert_non_null(st->source);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, trg, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->target));
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&st->target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_list2(void **state)
{
    struct state *st = (*state);
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "\n"
            "    container inner1 {\n"
            "        list b-list1 {\n"
            "            key p1;\n"
            "            leaf p1 {\n"
            "                type uint8;\n"
            "            }\n"
            "            leaf p2 {\n"
            "                type string;\n"
            "            }\n"
            "            container inner2 {\n"
            "                leaf p3 {\n"
            "                    type boolean;\n"
            "                    default false;\n"
            "                }\n"
            "                leaf p4 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>a</p2>\n"
            "    <inner2>\n"
            "      <p4>val</p4>\n"
            "    </inner2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *src =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *result =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "    <inner2>\n"
            "      <p4>val</p4>\n"
            "    </inner2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    char *printed = NULL;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, src, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->source));
    assert_non_null(st->source);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, trg, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->target));
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&st->target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_case(void **state)
{
    struct state *st = (*state);
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "    container cont {\n"
            "        choice ch {\n"
            "            container inner {\n"
            "                leaf p1 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "            case c2 {\n"
            "                leaf p1 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<cont xmlns=\"http://test/merge\">\n"
            "  <inner>\n"
            "    <p1>1</p1>\n"
            "  </inner>\n"
            "</cont>\n";
    const char *src =
            "<cont xmlns=\"http://test/merge\">\n"
            "  <p1>1</p1>\n"
            "</cont>\n";
    const char *result =
            "<cont xmlns=\"http://test/merge\">\n"
            "  <p1>1</p1>\n"
            "</cont>\n";
    char *printed = NULL;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, src, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->source));
    assert_non_null(st->source);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, trg, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->target));
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&st->target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_dflt(void **state)
{
    struct state *st = (*state);
    const char *sch =
            "module merge-dflt {\n"
            "    namespace \"urn:merge-dflt\";\n"
            "    prefix md;\n"
            "    container top {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            type string;\n"
            "        }\n"
            "        leaf c {\n"
            "            type string;\n"
            "            default \"c_dflt\";\n"
            "        }\n"
            "    }\n"
            "}\n";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(lyd_new_path(NULL, st->ctx, "/merge-dflt:top/c", "c_dflt", 0, &st->target), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&(st->target), NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_new_path(NULL, st->ctx, "/merge-dflt:top/a", "a_val", 0, &st->source), LY_SUCCESS);
    assert_int_equal(lyd_new_path(st->source, st->ctx, "/merge-dflt:top/b", "b_val", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&(st->source), NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_merge_siblings(&st->target, st->source, LYD_MERGE_DESTRUCT | LYD_MERGE_DEFAULTS), LY_SUCCESS);
    st->source = NULL;

    /* c should be replaced and now be default */
    assert_string_equal(lyd_child(st->target)->prev->schema->name, "c");
    assert_true(lyd_child(st->target)->prev->flags & LYD_DEFAULT);
}

static void
test_dflt2(void **state)
{
    struct state *st = (*state);
    const char *sch =
            "module merge-dflt {\n"
            "    namespace \"urn:merge-dflt\";\n"
            "    prefix md;\n"
            "    container top {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            type string;\n"
            "        }\n"
            "        leaf c {\n"
            "            type string;\n"
            "            default \"c_dflt\";\n"
            "        }\n"
            "    }\n"
            "}\n";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(lyd_new_path(NULL, st->ctx, "/merge-dflt:top/c", "c_dflt", 0, &st->target), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&(st->target), NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_new_path(NULL, st->ctx, "/merge-dflt:top/a", "a_val", 0, &st->source), LY_SUCCESS);
    assert_int_equal(lyd_new_path(st->source, st->ctx, "/merge-dflt:top/b", "b_val", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&(st->source), NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_merge_siblings(&st->target, st->source, 0), LY_SUCCESS);

    /* c should not be replaced, so c remains not default */
    assert_false(lyd_child(st->target)->flags & LYD_DEFAULT);
}

static void
test_leafrefs(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
            "  namespace urn:x;"
            "  prefix x;"
            "  list l {"
            "    key n;"
            "    leaf n { type string; }"
            "    leaf t { type string; }"
            "    leaf r { type leafref { path '/l/n'; } }}}";
    const char *trg = "<l xmlns=\"urn:x\"><n>a</n></l>"
            "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>";
    const char *src = "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>"
            "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>";
    const char *res = "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>"
            "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>"
            "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>";
    char *prt = NULL;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, sch, LYS_IN_YANG, NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, src, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->source));
    assert_non_null(st->source);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, trg, LYD_XML, 0, LYD_VALIDATE_PRESENT, &st->target));
    assert_non_null(st->target);

    assert_int_equal(lyd_merge_siblings(&st->target, st->source, 0), LY_SUCCESS);

    lyd_print_mem(&prt, st->target, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_string_equal(prt, res);
    free(prt);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_batch, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_leaf, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_container, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_list, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_list2, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_case, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_dflt, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_dflt2, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_leafrefs, setup_dflt, teardown_dflt),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
