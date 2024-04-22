/*
 * @file test_printer_json.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from printer_yang.c
 *
 * Copyright (c) 2019-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

static int
setup(void **state)
{
    const char *schema1 = "module schema1 {namespace urn:tests:schema1;prefix schema1;yang-version 1.1;"
            "revision 2014-05-08;"
            "anydata data;"
            "}";

    UTEST_SETUP;
    UTEST_ADD_MODULE(schema1, LYS_IN_YANG, NULL, NULL);
    return 0;
}

static void
test_container_presence(void **state)
{
    struct lyd_node *tree;
    char *buffer = NULL;
    const char *data = "{\"schema1:data\":{\"cont1\":{}}}";

    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&buffer, tree, LYD_JSON, LYD_PRINT_SHRINK));
    CHECK_STRING(buffer, data);
    free(buffer);
    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_container_presence, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
