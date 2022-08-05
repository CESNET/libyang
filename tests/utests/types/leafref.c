/**
 * @file leafref.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief test for built-in enumeration type
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

/* INCLUDE UTEST HEADER */
#define  _UTEST_MAIN_
#include "../utests.h"

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

#define TEST_SUCCESS_XML2(XML1, MOD_NAME, NAMESPACES, NODE_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = XML1 "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\" " NAMESPACES ">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 1, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML2(XML1, MOD_NAME, NAMESPACES, NODE_NAME, DATA, RET) \
    {\
        struct lyd_node *tree; \
        const char *data = XML1 "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\" " NAMESPACES ">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, RET, tree); \
        assert_null(tree); \
    }

#define TEST_SUCCESS_LYB(MOD_NAME, NODE_NAME1, DATA1, NODE_NAME2, DATA2) \
    { \
        struct lyd_node *tree_1; \
        struct lyd_node *tree_2; \
        char *xml_out, *data; \
        data = "<" NODE_NAME1 " xmlns=\"urn:tests:" MOD_NAME "\"><name>" DATA1 "</name></" NODE_NAME1 ">" \
        "<" NODE_NAME2 " xmlns=\"urn:tests:" MOD_NAME "\">" DATA2 "</" NODE_NAME2 ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LY_SUCCESS, tree_1); \
        assert_int_equal(lyd_print_mem(&xml_out, tree_1, LYD_LYB, LYD_PRINT_WITHSIBLINGS), 0); \
        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml_out, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree_2)); \
        assert_non_null(tree_2); \
        CHECK_LYD(tree_1, tree_2); \
        free(xml_out); \
        lyd_free_all(tree_1); \
        lyd_free_all(tree_2); \
    }

static void
test_data_xml(void **state)
{
    const char *schema, *schema2, *data2;
    struct lyd_node *tree2;

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "leaf lref {type leafref {path /leaflisttarget; require-instance true;}}"
            "leaf lref2 {type leafref {path \"../list[id = current()/../str-norestr]/targets\"; require-instance true;}}"
            "leaf str-norestr {type string;}"
            "list list {key id; leaf id {type string;} leaf value {type string;} leaf-list targets {type string;}}"
            "container cont {leaf leaftarget {type empty;}"
            "    list listtarget {key id; max-elements 5;leaf id {type uint8;} leaf value {type string;}}"
            "    leaf-list leaflisttarget {type uint8; max-elements 5;}}"
            "leaf-list leaflisttarget {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    schema2 = MODULE_CREATE_YANG("leafrefs", "import defs {prefix t;}"
            "container c { container x {leaf x {type string;}} list l {"
            "  key \"id value\"; leaf id {type string;} leaf value {type string;}"
            "  leaf lr1 {type leafref {path \"../../../t:str-norestr\"; require-instance true;}}"
            "  leaf lr2 {type leafref {path \"../../l[id=current()/../../../t:str-norestr]\" +"
            "    \"[value=current()/../../../t:str-norestr]/value\"; require-instance true;}}"
            "  leaf lr3 {type leafref {path \"/t:list[t:id=current ( )/../../x/x]/t:targets\";}}"
            "}}");
    UTEST_ADD_MODULE(schema2, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML2("<leaflisttarget xmlns=\"urn:tests:defs\">x</leaflisttarget>"
            "<leaflisttarget xmlns=\"urn:tests:defs\">y</leaflisttarget>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "a:lref", "y", STRING, "y");

    TEST_SUCCESS_XML2("<list xmlns=\"urn:tests:defs\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:defs\"><id>y</id><targets>x</targets><targets>y</targets></list>"
            "<str-norestr xmlns=\"urn:tests:defs\">y</str-norestr>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "a:lref2", "y", STRING, "y");

    data2 = "<str-norestr xmlns=\"urn:tests:defs\">y</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>x</id><value>x</value><lr1>y</lr1></l></c>";
    CHECK_PARSE_LYD_PARAM(data2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree2);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)lyd_child(lyd_child(tree2->next->next)->next)->next->next,
            0, 0, 0, 1, 1, STRING, "y");
    lyd_free_all(tree2);

    data2 = "<list xmlns=\"urn:tests:defs\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:defs\"><id>y</id><targets>c</targets><targets>d</targets></list>"
            "<c xmlns=\"urn:tests:leafrefs\"><x><x>y</x></x>"
            "<l><id>x</id><value>x</value><lr3>c</lr3></l></c>";
    CHECK_PARSE_LYD_PARAM(data2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree2);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)lyd_child(lyd_child(tree2->next->next->next)->next)->next->next,
            0, 0, 0, 1, 1, STRING, "c");
    lyd_free_all(tree2);

    /* invalid value */
    TEST_ERROR_XML2("<leaflisttarget xmlns=\"urn:tests:defs\">x</leaflisttarget>",
            "defs", "", "lref", "y", LY_EVALID);
    CHECK_LOG_CTX_APPTAG("Invalid leafref value \"y\" - no target instance \"/leaflisttarget\" with the same value.",
            "Schema location \"/defs:lref\", data location \"/defs:lref\".", "instance-required");

    TEST_ERROR_XML2("<list xmlns=\"urn:tests:defs\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:defs\"><id>y</id><targets>x</targets><targets>y</targets></list>"
            "<str-norestr xmlns=\"urn:tests:defs\">y</str-norestr>",
            "defs", "", "lref2", "b", LY_EVALID);
    CHECK_LOG_CTX_APPTAG("Invalid leafref value \"b\" - "
            "no target instance \"../list[id = current()/../str-norestr]/targets\" with the same value.",
            "Schema location \"/defs:lref2\", data location \"/defs:lref2\".", "instance-required");

    TEST_ERROR_XML2("<list xmlns=\"urn:tests:defs\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:defs\"><id>y</id><targets>x</targets><targets>y</targets></list>",
            "defs", "", "lref2", "b", LY_EVALID);
    CHECK_LOG_CTX_APPTAG("Invalid leafref value \"b\" - "
            "no target instance \"../list[id = current()/../str-norestr]/targets\" with the same value.",
            "Schema location \"/defs:lref2\", data location \"/defs:lref2\".", "instance-required");

    TEST_ERROR_XML2("<str-norestr xmlns=\"urn:tests:defs\">y</str-norestr>",
            "defs", "", "lref2", "b", LY_EVALID);
    CHECK_LOG_CTX_APPTAG("Invalid leafref value \"b\" - "
            "no target instance \"../list[id = current()/../str-norestr]/targets\" with the same value.",
            "Schema location \"/defs:lref2\", data location \"/defs:lref2\".", "instance-required");

    TEST_ERROR_XML2("<str-norestr xmlns=\"urn:tests:defs\">y</str-norestr>",
            "leafrefs", "", "c", "<l><id>x</id><value>x</value><lr1>a</lr1></l>", LY_EVALID);
    CHECK_LOG_CTX_APPTAG("Invalid leafref value \"a\" - no target instance \"../../../t:str-norestr\" with the same value.",
            "Schema location \"/leafrefs:c/l/lr1\", data location \"/leafrefs:c/l[id='x'][value='x']/lr1\".", "instance-required");

    TEST_ERROR_XML2("<str-norestr xmlns=\"urn:tests:defs\">z</str-norestr>",
            "leafrefs", "", "c", "<l><id>y</id><value>y</value></l><l><id>x</id><value>x</value><lr2>z</lr2></l>", LY_EVALID);
    CHECK_LOG_CTX_APPTAG("Invalid leafref value \"z\" - no target instance \"../../l[id=current()/../../../t:str-norestr]"
            "[value=current()/../../../t:str-norestr]/value\" with the same value.",
            "Schema location \"/leafrefs:c/l/lr2\", data location \"/leafrefs:c/l[id='x'][value='x']/lr2\".", "instance-required");

    TEST_ERROR_XML2("",
            "defs", "", "lref", "%n", LY_EVALID);
    CHECK_LOG_CTX_APPTAG("Invalid leafref value \"%n\" - no target instance \"/leaflisttarget\" with the same value.",
            "Schema location \"/defs:lref\", data location \"/defs:lref\".", "instance-required");
}

static void
test_plugin_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb",
            "list lst {key \"name\"; leaf name {type string;}}"
            "leaf lref {type leafref {path \"../lst/name\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "lst", "key_str", "lref", "key_str");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
        UTEST(test_plugin_lyb),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
