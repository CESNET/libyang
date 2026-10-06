/**
 * @file test_schema_comparison.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for schema-comparison extensions support
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
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

static void
test_change_at(void **state)
{
    struct lys_module *mod;
    const char *yang;

    /* load ietf-yang-schema-comaprison */
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-schema-comparison", NULL, NULL));

    /* valid yang */
    yang = "module a {namespace urn:a; prefix a;"
            "import ietf-yang-schema-comparison {prefix yscmp;}"
            "leaf a {type string {pattern '[a-z]+' {yscmp:bc-change-at 1.1.0;}}}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);

    /* wrong statement */
    yang = "module b {namespace urn:b; prefix b;"
            "import ietf-yang-schema-comparison {prefix yscmp;}"
            "leaf a {type string {yscmp:bc-change-at 1.1.0;}}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);
    CHECK_LOG_CTX("Ext plugin \"ly2 schema-comparison\": Extension yscmp:bc-change-at is not allowed in a \"type\" statement.",
            "/b:{type='string'}/{ext-inst='yscmp:bc-change-at'}/1.1.0", 0);

    /* duplicate */
    yang = "module c {namespace urn:c; prefix c;"
            "import ietf-yang-schema-comparison {prefix yscmp;}"
            "leaf a {type string {pattern 'a' {yscmp:bc-change-at 1.1.0; yscmp:ed-change-at 1.1.0;}}}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 schema-comparison\": Extension yscmp:bc-change-at semver \"1.1.0\" "
            "is a duplicate of yscmp:ed-change-at semver \"1.1.0\".",
            "/c:{pattern='a'}/{ext-inst='yscmp:bc-change-at'}/1.1.0", 0);

    /* invalid argument */
    yang = "module c {namespace urn:c; prefix c;"
            "import ietf-yang-schema-comparison {prefix yscmp;}"
            "leaf a {type string; description \"eh\" {yscmp:bc-change-at 1.1.1a;}}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 schema-comparison\": Extension yscmp:bc-change-at argument semver \"1.1.1a\" invalid.",
            "/c:{description}/{ext-inst='yscmp:bc-change-at'}/1.1.1a", 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_change_at),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
