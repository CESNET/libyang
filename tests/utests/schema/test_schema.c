/*
 * @file test_schema.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for schema related functions
 *
 * Copyright (c) 2018-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "test_schema.h"

#include <string.h>

#include "log.h"
#include "parser_schema.h"
#include "set.h"
#include "tree_schema.h"

LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    if ((*module_data)[0] == '<') {
        *format = LYS_IN_YIN;
    } else {
        *format = LYS_IN_YANG;
    }
    *free_module_data = NULL;
    return LY_SUCCESS;
}

/**
 * DECLARE OTHER SCHEMA TESTS
 */
/* test_schema_common.c */
void test_getnext(void **state);
void test_date(void **state);
void test_revisions(void **state);
void test_collision_typedef(void **state);
void test_collision_grouping(void **state);
void test_collision_identity(void **state);
void test_collision_feature(void **state);
void test_accessible_tree(void **state);
void test_includes(void **state);
void test_key_order(void **state);
void test_disabled_enum(void **state);

/* test_schema_stmts.c */
void test_identity(void **state);
void test_feature(void **state);

/* test_schema_extensions.c */
void test_extension_argument(void **state);
void test_extension_argument_element(void **state);
void test_extension_compile(void **state);

int
main(void)
{
    const struct CMUnitTest tests[] = {
        /** test_schema_common.c */
        UTEST(test_getnext),
        UTEST(test_date),
        UTEST(test_revisions),
        UTEST(test_collision_typedef),
        UTEST(test_collision_grouping),
        UTEST(test_collision_identity),
        UTEST(test_collision_feature),
        UTEST(test_accessible_tree),
        UTEST(test_includes),
        UTEST(test_key_order),
        UTEST(test_disabled_enum),

        /** test_schema_stmts.c */
        UTEST(test_identity),
        UTEST(test_feature),

        /** test_schema_extensions.c */
        UTEST(test_extension_argument),
        UTEST(test_extension_argument_element),
        UTEST(test_extension_compile),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
