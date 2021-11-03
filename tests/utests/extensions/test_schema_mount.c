/*
 * @file test_schema_mount.c
 * @author: Tadeas Vintrlik <xvintr04@stud.fit.vutbr.cz>
 * @brief unit tests for Schema Mount extension support
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"

static int
setup(void **state)
{
    UTEST_SETUP;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-schema-mount", "2019-01-14", NULL));

    return 0;
}

static void
test_compile(void **state)
{
    struct lys_module *mod;
    const char *data = "module test-parent {yang-version 1.1;namespace \"urn:test-parent\";"
            "prefix \"tp\"; import ietf-yang-schema-mount {prefix yangmnt;} container root {"
            "yangmnt:mount-point \"root\" {}}}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
}

static void
test_parse_no_yanglib(void **state)
{
    struct ly_ctx *new;
    const char *data = "<root/>";

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &new));
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_compile, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
