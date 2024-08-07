/**
 * @file enumeration.c
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

#define TEST_SUCCESS_XML(MOD_NAME, NODE_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML(MOD_NAME, NODE_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
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

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "feature f; leaf l1 {type enumeration {enum white; enum yellow {if-feature f;}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML("defs", "l1", "white", ENUM, "white", "white");

    /* disabled feature */
    TEST_ERROR_XML("defs", "l1", "yellow");
    CHECK_LOG_CTX("Invalid enumeration value \"yellow\".", "/defs:l1", 1);

    /* leading/trailing whitespaces */
    TEST_ERROR_XML("defs", "l1", " white");
    CHECK_LOG_CTX("Invalid enumeration value \" white\".", "/defs:l1", 1);

    TEST_ERROR_XML("defs", "l1", "white\n");
    CHECK_LOG_CTX("Invalid enumeration value \"white\n\".", "/defs:l1", 2);

    /* invalid value */
    TEST_ERROR_XML("defs", "l1", "black");
    CHECK_LOG_CTX("Invalid enumeration value \"black\".", "/defs:l1", 1);
}

static void
test_plugin_sort(void **state)
{
    const char *v1, *v2;
    const char *schema;
    struct lys_module *mod;
    struct lyd_value val1 = {0}, val2 = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_ENUM]);
    struct lysc_type *lysc_type;
    struct ly_err_item *err = NULL;

    schema = MODULE_CREATE_YANG("sort", "leaf l1 {type enumeration {enum white; enum yellow; enum black;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    v1 = "white";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "black";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 < type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 > type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);
}

static void
test_plugin_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb", "leaf l1 {type enumeration {enum white; enum yellow; enum black;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "l1", "white");
    TEST_SUCCESS_LYB("lyb", "l1", "black");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
        UTEST(test_data_xml),
        UTEST(test_plugin_sort),
        UTEST(test_plugin_lyb),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
