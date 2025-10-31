/**
 * @file test_yang_revisions.c
 * @author Joe Clarke <jclarke@cisco.com>
 * @brief unit tests for YANG Revisions extensions support
 *
 * Copyright (c) 2025 Cisco Systems, Inc.
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
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-revisions", "2025-01-28", NULL));

    return 0;
}

static void
test_non_backwards_compatible_valid(void **state)
{
    struct lys_module *mod;
    const char *data;

    /* Valid: no argument */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:revisions:a; prefix a;"
            "import ietf-yang-revisions { prefix rev; }"
            "revision 2025-01-01 { rev:non-backwards-compatible; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
}

static void
test_non_backwards_compatible_invalid(void **state)
{
    const char *data;

    /* Invalid: not in revision statement */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:revisions:a; prefix a;"
            "import ietf-yang-revisions { prefix rev; }"
            "rev:non-backwards-compatible;}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-revisions v1\": "
            "Extension rev:non-backwards-compatible is allowed only in a revision statement, but it is placed in \"module\" statement.",
            "/a:{extension='rev:non-backwards-compatible'}", 0);

    /* Invalid: duplicate in same revision */
    data = "module c {yang-version 1.1; namespace urn:tests:extensions:revisions:c; prefix c;"
            "import ietf-yang-revisions { prefix rev; }"
            "revision 2025-01-01 { rev:non-backwards-compatible; rev:non-backwards-compatible; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-revisions v1\": "
            "Extension rev:non-backwards-compatible is instantiated multiple times in the same revision.",
            "/c:{extension='rev:non-backwards-compatible'}", 0);
}

static void
test_recommended_min_date_valid(void **state)
{
    struct lys_module *mod;
    const char *data;

    /* Valid: proper date format */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:revisions:a; prefix a;"
            "import ietf-yang-revisions { prefix rev; }"
            "import ietf-yang-types { prefix yt; rev:recommended-min-date \"2013-07-15\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);

    /* Valid: different date */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:revisions:b; prefix b;"
            "import ietf-yang-revisions { prefix rev; }"
            "import ietf-yang-types { prefix yt; rev:recommended-min-date \"2020-12-31\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
}

static void
test_recommended_min_date_invalid(void **state)
{
    const char *data;

    /* Invalid: not in import statement */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:revisions:a; prefix a;"
            "import ietf-yang-revisions { prefix rev; }"
            "rev:recommended-min-date \"2013-07-15\";}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-revisions v1\": "
            "Extension rev:recommended-min-date is allowed only in an import statement, but it is placed in \"module\" statement.",
            "/a:{extension='rev:recommended-min-date'}/2013-07-15", 0);

    /* Invalid: bad date format - wrong separator */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:revisions:b; prefix b;"
            "import ietf-yang-revisions { prefix rev; }"
            "import ietf-yang-types { prefix yt; rev:recommended-min-date \"2013/07/15\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-revisions v1\": "
            "Extension rev:recommended-min-date has invalid revision-date format: 2013/07/15 (expected YYYY-MM-DD and a valid date)",
            "/b:{extension='rev:recommended-min-date'}/2013/07/15", 0);

    /* Invalid: bad date format - too short */
    data = "module c {yang-version 1.1; namespace urn:tests:extensions:revisions:c; prefix c;"
            "import ietf-yang-revisions { prefix rev; }"
            "import ietf-yang-types { prefix yt; rev:recommended-min-date \"2013-07\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-revisions v1\": "
            "Extension rev:recommended-min-date has invalid revision-date format: 2013-07 (expected YYYY-MM-DD and a valid date)",
            "/c:{extension='rev:recommended-min-date'}/2013-07", 0);

    /* Invalid: bad date - invalid month */
    data = "module d {yang-version 1.1; namespace urn:tests:extensions:revisions:d; prefix d;"
            "import ietf-yang-revisions { prefix rev; }"
            "import ietf-yang-types { prefix yt; rev:recommended-min-date \"2013-13-15\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-revisions v1\": "
            "Extension rev:recommended-min-date has invalid revision-date format: 2013-13-15 (expected YYYY-MM-DD and a valid date)",
            "/d:{extension='rev:recommended-min-date'}/2013-13-15", 0);

    /* Invalid: duplicate in same import */
    data = "module e {yang-version 1.1; namespace urn:tests:extensions:revisions:e; prefix e;"
            "import ietf-yang-revisions { prefix rev; }"
            "import ietf-yang-types { prefix yt; rev:recommended-min-date \"2013-07-15\"; rev:recommended-min-date \"2014-01-01\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-revisions v1\": "
            "Extension rev:recommended-min-date is instantiated multiple times in the same import.",
            "/e:{extension='rev:recommended-min-date'}/2013-07-15", 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_non_backwards_compatible_valid, setup),
        UTEST(test_non_backwards_compatible_invalid, setup),
        UTEST(test_recommended_min_date_valid, setup),
        UTEST(test_recommended_min_date_invalid, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
