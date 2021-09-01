/**
 * @file union.c
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
#include "path.h"

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
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 1, 0, 1, TYPE, ## __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML2(XML1, MOD_NAME, NAMESPACES, NODE_NAME, DATA, RET) \
    {\
        struct lyd_node *tree; \
        const char *data = XML1 "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\" " NAMESPACES ">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, RET, tree); \
        assert_null(tree); \
    }

#define TEST_SUCCESS_LYB(MOD_NAME, NODE_NAME, DATA) \
    { \
        struct lyd_node *tree_1; \
        struct lyd_node *tree_2; \
        char *xml_out, *data; \
        data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
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
    const char *schema;
    const enum ly_path_pred_type val1[] = {LY_PATH_PREDTYPE_LEAFLIST};

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "identity ident1; identity ident2 {base ident1;}"
            "leaf un1 {type union {"
            "    type leafref {path /int8; require-instance true;}"
            "    type union { type identityref {base ident1;} type instance-identifier {require-instance true;} }"
            "    type string {length 1..20;}}}"
            "leaf int8 {type int8 {range 10..20;}}"
            "leaf-list llist {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML2("<int8 xmlns=\"urn:tests:defs\">12</int8>",
            "defs", "", "un1", "12", UNION, "12", INT8, "12", 12);

    TEST_SUCCESS_XML2("<int8 xmlns=\"urn:tests:defs\">12</int8>",
            "defs", "", "un1", "2", UNION, "2", STRING, "2");

    TEST_SUCCESS_XML2("<int8 xmlns=\"urn:tests:defs\">10</int8>",
            "defs", "xmlns:x=\"urn:tests:defs\"", "un1", "x:ident2", UNION, "defs:ident2", IDENT, "defs:ident2", "ident2");

    TEST_SUCCESS_XML2("<int8 xmlns=\"urn:tests:defs\">10</int8>",
            "defs", "xmlns:x=\"urn:tests:defs\"", "un1", "x:ident55", UNION, "x:ident55", STRING, "x:ident55");

    TEST_SUCCESS_XML2("<llist xmlns=\"urn:tests:defs\">x</llist>"
            "<llist xmlns=\"urn:tests:defs\">y</llist>",
            "defs", "xmlns:x=\"urn:tests:defs\"", "un1", "/x:llist[.='y']", UNION, "/defs:llist[.='y']",
            INST, "/defs:llist[.='y']", val1);

    TEST_SUCCESS_XML2("<llist xmlns=\"urn:tests:defs\">x</llist>"
            "<llist xmlns=\"urn:tests:defs\">y</llist>",
            "defs", "xmlns:x=\"urn:tests:defs\"", "un1", "/x:llist[3]", UNION, "/x:llist[3]",
            STRING, "/x:llist[3]");

    /* invalid value */
    TEST_ERROR_XML2("",
            "defs", "", "un1", "123456789012345678901", LY_EVALID);
    CHECK_LOG_CTX("Invalid union value \"123456789012345678901\" - no matching subtype found.",
            "Schema location /defs:un1, line number 1.");
}

static void
test_plugin_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb",
            "leaf int8 {type int8 {range 10..20;}}"
            "leaf un1 {type union {"
            "    type leafref {path /int8; require-instance true;}"
            "    type string;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "un1", "12");
    TEST_SUCCESS_LYB("lyb", "un1", "some_string");
    TEST_SUCCESS_LYB("lyb", "un1", "");
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
