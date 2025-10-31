/**
 * @file test_yang_semver.c
 * @author Joe Clarke <jclarke@cisco.com>
 * @brief unit tests for YANG Semantic Versioning extensions support
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
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-semver", "2025-09-29", NULL));

    return 0;
}

static void
test_version_valid(void **state)
{
    struct lys_module *mod;
    const char *data;

    /* Valid: basic semantic version */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:semver:a; prefix a;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);

    /* Valid: version with compatible modifier */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:semver:b; prefix b;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0_compatible\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);

    /* Valid: version with non_compatible modifier */
    data = "module c {yang-version 1.1; namespace urn:tests:extensions:semver:c; prefix c;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"2.1.3_non_compatible\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);

    /* Valid: version with pre-release */
    data = "module d {yang-version 1.1; namespace urn:tests:extensions:semver:d; prefix d;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0-alpha.1\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);

    /* Valid: version with metadata */
    data = "module e {yang-version 1.1; namespace urn:tests:extensions:semver:e; prefix e;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0+build.123\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);

    /* Valid: complex version */
    data = "module f {yang-version 1.1; namespace urn:tests:extensions:semver:f; prefix f;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.2.3_compatible-beta.2+build.456\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
}

static void
test_version_invalid(void **state)
{
    const char *data;

    /* Invalid: not in revision statement */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:semver:a; prefix a;"
            "import ietf-yang-semver { prefix ysv; }"
            "ysv:version \"1.0.0\";}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:version is allowed only in a revision statement, but it is placed in \"module\" statement.",
            "/a:{extension='ysv:version'}/1.0.0", 0);

    /* Invalid: bad format - missing patch */
    data = "module c {yang-version 1.1; namespace urn:tests:extensions:semver:c; prefix c;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:version has invalid YANG Semver version format: 1.0",
            "/c:{extension='ysv:version'}/1.0", 0);

    /* Invalid: bad format - too many dots */
    data = "module d {yang-version 1.1; namespace urn:tests:extensions:semver:d; prefix d;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0.1\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:version has invalid YANG Semver version format: 1.0.0.1",
            "/d:{extension='ysv:version'}/1.0.0.1", 0);

    /* Invalid: duplicate in same revision */
    data = "module e {yang-version 1.1; namespace urn:tests:extensions:semver:e; prefix e;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0\"; ysv:version \"1.0.1\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:version is instantiated multiple times in the same revision.",
            "/e:{extension='ysv:version'}/1.0.0", 0);
}

static void
test_version_uniqueness(void **state)
{
    struct lys_module *mod;
    const char *data;

    /* Invalid: same version in different revisions */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:semver:a; prefix a;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0\"; }"
            "revision 2025-02-01 { ysv:version \"1.0.0\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Version 1.0.0 is not unique - it appears in multiple revisions.",
            "/a:{extension='ysv:version'}/1.0.0", 0);

    /* Valid: different versions in different revisions */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:semver:b; prefix b;"
            "import ietf-yang-semver { prefix ysv; }"
            "revision 2025-01-01 { ysv:version \"1.0.0\"; }"
            "revision 2025-02-01 { ysv:version \"1.0.1\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
}

static void
test_recommended_min_version_valid(void **state)
{
    struct lys_module *mod;
    const char *data;

    /* Valid: basic MAJOR.MINOR.PATCH */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:semver:a; prefix a;"
            "import ietf-yang-semver { prefix ysv; }"
            "import ietf-yang-types { prefix yt; ysv:recommended-min-version \"1.0.0\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);

    /* Valid: different version format */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:semver:b; prefix b;"
            "import ietf-yang-semver { prefix ysv; }"
            "import ietf-yang-types { prefix yt; ysv:recommended-min-version \"2.15.3\"; }}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
}

static void
test_recommended_min_version_invalid(void **state)
{
    const char *data;

    /* Invalid: not in import statement */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:semver:a; prefix a;"
            "import ietf-yang-semver { prefix ysv; }"
            "ysv:recommended-min-version \"1.0.0\";}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:recommended-min-version is allowed only in an import statement, but it is placed in \"module\" statement.",
            "/a:{extension='ysv:recommended-min-version'}/1.0.0", 0);

    /* Invalid: has modifiers (not allowed) */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:semver:b; prefix b;"
            "import ietf-yang-semver { prefix ysv; }"
            "import ietf-yang-types { prefix yt; ysv:recommended-min-version \"1.0.0_compatible\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:recommended-min-version argument must be MAJOR.MINOR.PATCH format (no modifiers): 1.0.0_compatible",
            "/b:{extension='ysv:recommended-min-version'}/1.0.0_compatible", 0);

    /* Invalid: has pre-release (not allowed) */
    data = "module c {yang-version 1.1; namespace urn:tests:extensions:semver:c; prefix c;"
            "import ietf-yang-semver { prefix ysv; }"
            "import ietf-yang-types { prefix yt; ysv:recommended-min-version \"1.0.0-alpha\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:recommended-min-version argument must be MAJOR.MINOR.PATCH format (no modifiers): 1.0.0-alpha",
            "/c:{extension='ysv:recommended-min-version'}/1.0.0-alpha", 0);

    /* Invalid: duplicate in same import */
    data = "module d {yang-version 1.1; namespace urn:tests:extensions:semver:d; prefix d;"
            "import ietf-yang-semver { prefix ysv; }"
            "import ietf-yang-types { prefix yt; ysv:recommended-min-version \"1.0.0\"; ysv:recommended-min-version \"2.0.0\"; }}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 ietf-yang-semver v1\": "
            "Extension ysv:recommended-min-version is instantiated multiple times in the same import.",
            "/d:{extension='ysv:recommended-min-version'}/1.0.0", 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_version_valid, setup),
        UTEST(test_version_invalid, setup),
        UTEST(test_version_uniqueness, setup),
        UTEST(test_recommended_min_version_valid, setup),
        UTEST(test_recommended_min_version_invalid, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
