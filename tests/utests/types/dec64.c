/**
 * @file dec64.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief test for decimal64 values
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

/* GLOBAL INCLUDE HEADERS */
#include <ctype.h>

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"
#include "path.h"
#include "plugins_internal.h"

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
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, ## __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML(MOD_NAME, NODE_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

static void
test_data_xml(void **state)
{
    const char *schema;

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "leaf l1 {type decimal64 {fraction-digits 1; range 1.5..10;}}"
            "leaf l2 {type decimal64 {fraction-digits 18;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML("defs", "l1", "\n +8 \t\n  ", DEC64, "8.0", 80);
    TEST_SUCCESS_XML("defs", "l1", "8.00", DEC64, "8.0", 80);

    TEST_SUCCESS_XML("defs", "l2", "-9.223372036854775808", DEC64, "-9.223372036854775808",
            INT64_C(-9223372036854775807) - INT64_C(1));
    TEST_SUCCESS_XML("defs", "l2", "9.223372036854775807", DEC64, "9.223372036854775807", INT64_C(9223372036854775807));

    TEST_ERROR_XML("defs", "l1", "\n 15 \t\n  ");
    CHECK_LOG_CTX("Unsatisfied range - value \"15.0\" is out of the allowed range.",
            "Schema location /defs:l1, line number 3.");

    TEST_ERROR_XML("defs", "l1", "\n 0 \t\n  ");
    CHECK_LOG_CTX("Unsatisfied range - value \"0.0\" is out of the allowed range.",
            "Schema location /defs:l1, line number 3.");

    TEST_ERROR_XML("defs", "l1", "xxx");
    CHECK_LOG_CTX("Invalid 1. character of decimal64 value \"xxx\".",
            "Schema location /defs:l1, line number 1.");

    TEST_ERROR_XML("defs", "l1", "");
    CHECK_LOG_CTX("Invalid empty decimal64 value.",
            "Schema location /defs:l1, line number 1.");

    TEST_ERROR_XML("defs", "l1", "8.5  xxx");
    CHECK_LOG_CTX("Invalid 6. character of decimal64 value \"8.5  xxx\".",
            "Schema location /defs:l1, line number 1.");

    TEST_ERROR_XML("defs", "l1", "8.55  xxx");
    CHECK_LOG_CTX("Value \"8.55\" of decimal64 type exceeds defined number (1) of fraction digits.",
            "Schema location /defs:l1, line number 1.");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
