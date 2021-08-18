/**
 * @file instanceid.c
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

#define LYB_CHECK_START \
    struct lyd_node *tree_1; \
    struct lyd_node *tree_2; \
    char *xml_out, *data;

#define LYB_CHECK_END \
    { \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LY_SUCCESS, tree_1); \
        assert_int_equal(lyd_print_mem(&xml_out, tree_1, LYD_LYB, LYD_PRINT_WITHSIBLINGS), 0); \
        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml_out, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree_2)); \
        assert_non_null(tree_2); \
        CHECK_LYD(tree_1, tree_2); \
        free(xml_out); \
        lyd_free_all(tree_1); \
        lyd_free_all(tree_2); \
    }

#define TEST_SUCCESS_LYB(MOD_NAME, NODE_NAME1, DATA1, NODE_NAME2, DATA2) \
    LYB_CHECK_START \
    data = "<" NODE_NAME1 " xmlns=\"urn:tests:" MOD_NAME "\">" DATA1 "</" NODE_NAME1 ">" \
    "<xdf:" NODE_NAME2 " xmlns:xdf=\"urn:tests:" MOD_NAME "\">/xdf:" DATA2 "</xdf:" NODE_NAME2 ">"; \
    LYB_CHECK_END \

#define TEST_SUCCESS_LYB2(MOD_NAME, NODE_NAME, DATA) \
    { \
        LYB_CHECK_START \
        data = "<" NODE_NAME " xmlns:aa=\"urn:tests:lyb2\" xmlns=\"urn:tests:" MOD_NAME "\">/aa:" DATA "</" NODE_NAME ">"; \
        LYB_CHECK_END \
    }

static void
test_plugin_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb",
            "leaf-list leaflisttarget {type string;}"
            "leaf inst {type instance-identifier {require-instance true;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "leaflisttarget", "1", "inst", "leaflisttarget[.='1']");

    /* ietf-netconf-acm node-instance-identifier type */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    schema = MODULE_CREATE_YANG("lyb2",
            "import ietf-netconf-acm {prefix acm;}"
            "leaf-list ll {type string;}"
            "leaf nii {type acm:node-instance-identifier;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB2("lyb2", "nii", "ll[. = 'some_string']");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_plugin_lyb),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
