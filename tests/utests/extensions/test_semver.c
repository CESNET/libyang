/**
 * @file test_semver.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for semver extensions support
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
test_version(void **state)
{
    struct lys_module *mod;
    const char *yang;

    /* valid yang */
    yang = "module rev1 {namespace urn:rev1; prefix r1;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-01-01 {ysv:version 1.0.0;}"
            "leaf a {type string;}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);

    /* wrong statement */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-01-01 {ysv:version 1.0.0;}"
            "leaf a {type string {ysv:version 1.0.0;}}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Extension ysv:version is allowed only in a \"revision\" statement, "
            "but it is placed in \"type\" statement.",
            "/rev2:{type='string'}/{ext-inst='ysv:version'}/1.0.0", 0);

    /* duplicate */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-01-01 {ysv:version 1.0.0; ysv:version 1.0.0;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Extension ysv:version is instantiated multiple times.",
            "/rev2:{revision='2026-01-01'}/{ext-inst='ysv:version'}/1.0.0", 0);

    /* argument */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-01-01 {ysv:version 1.0.0_compat;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Extension ysv:version argument yang-semantic-version \"1.0.0_compat\" invalid.",
            "/rev2:{revision='2026-01-01'}/{ext-inst='ysv:version'}/1.0.0_compat", 0);

    /* uniqueness ignoring COMPAT */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-01-05 {ysv:version 1.2.0;}"
            "revision 2026-01-03 {ysv:version 1.1.0_non_compatible;}"
            "revision 2026-01-04 {ysv:version 1.0.1;}"
            "revision 2026-01-02 {ysv:version 1.1.0;}"
            "revision 2026-01-01 {ysv:version 1.0.0;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Duplicate version \"1.1.0\" revision 2026-01-02 and "
            "\"1.1.0_non_compatible\" revision 2026-01-03.",
            "/rev2:{revision='2026-01-02'}/{ext-inst='ysv:version'}/1.1.0", 0);
    CHECK_LOG_CTX("Older revision 2026-01-03 found after a newer revision 2026-01-04 in module \"rev2\".", NULL, 0);

    /* not a higher patch version */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-01-05 {ysv:version 1.0.2;}"
            "revision 2026-01-04 {ysv:version 1.1.1;}"
            "revision 2026-01-03 {ysv:version 1.1.0;}"
            "revision 2026-01-02 {ysv:version 1.0.4;}"
            "revision 2026-01-01 {ysv:version 1.0.0;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Invalid higher version \"1.0.4\" revision 2026-01-02 compared to "
            "\"1.0.2\" revision 2026-01-05.",
            "/rev2:{revision='2026-01-02'}/{ext-inst='ysv:version'}/1.0.4", 0);

    /* sticky COMPAT modifier #1 */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-05-01 {ysv:version 1.2.0;}"
            "revision 2026-04-01 {ysv:version 1.0.2;}"
            "revision 2026-03-01 {ysv:version 1.1.0;}"
            "revision 2026-02-01 {ysv:version 1.0.1_compatible;}"
            "revision 2026-01-01 {ysv:version 1.0.0;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Invalid compat change in version \"1.0.2\" revision 2026-04-01 "
            "compared to \"1.0.1_compatible\" revision 2026-02-01.",
            "/rev2:{revision='2026-02-01'}/{ext-inst='ysv:version'}/1.0.1_compatible", 0);

    /* sticky COMPAT modifier #2 */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "revision 2026-05-01 {ysv:version 1.2.0;}"
            "revision 2026-03-01 {ysv:version 1.1.0;}"
            "revision 2026-02-01 {ysv:version 1.0.1_non_compatible;}"
            "revision 2026-04-01 {ysv:version 1.0.2_compatible;}"
            "revision 2026-01-01 {ysv:version 1.0.0;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Invalid compat change in version \"1.0.2_compatible\" revision 2026-04-01 "
            "compared to \"1.0.1_non_compatible\" revision 2026-02-01.",
            "/rev2:{revision='2026-04-01'}/{ext-inst='ysv:version'}/1.0.2_compatible", 0);
    CHECK_LOG_CTX("Older revision 2026-02-01 found after a newer revision 2026-04-01 in module \"rev2\".", NULL, 0);

    /* NBC ext missing MAJOR version increase #1 */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "import ietf-yang-revisions {prefix rev;}"
            "revision 2026-05-01 {ysv:version 1.2.0;}"
            "revision 2026-04-01 {ysv:version 1.1.1; rev:non-backwards-compatible;}"
            "revision 2026-03-01 {ysv:version 1.1.0;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Missing new major version with NBC changes in \"1.1.1\" revision 2026-04-01 "
            "compared to \"1.1.0\" revision 2026-03-01.",
            "/rev2:{revision='2026-03-01'}/{ext-inst='ysv:version'}/1.1.0", 0);

    /* NBC ext missing MAJOR version increase #2 */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-semver {prefix ysv;}"
            "import ietf-yang-revisions {prefix rev;}"
            "revision 2026-05-01 {ysv:version 1.2.0;}"
            "revision 2026-03-01 {ysv:version 1.1.0;}"
            "revision 2026-04-01 {ysv:version 1.1.1; rev:non-backwards-compatible;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Missing new major version with NBC changes in \"1.1.1\" revision 2026-04-01 "
            "compared to \"1.1.0\" revision 2026-03-01.",
            "/rev2:{revision='2026-04-01'}/{ext-inst='ysv:version'}/1.1.1", 0);
    CHECK_LOG_CTX("Older revision 2026-03-01 found after a newer revision 2026-04-01 in module \"rev2\".", NULL, 0);
}

static void
test_min_version(void **state)
{
    struct lys_module *mod;
    const char *yang;

    ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC "/modules/min_ver");

    /* valid yang */
    yang = "module ver1 {namespace urn:ver1; prefix v1;"
            "import ietf-yang-semver {prefix ysv;}"
            "import imp-min-ver1 {prefix imv1; ysv:recommended-min-version 1.0.0;}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);

    /* older version import */
    yang = "module ver2 {namespace urn:ver2; prefix v2;"
            "import ietf-yang-semver {prefix ysv;}"
            "import imp-min-ver1 {prefix imv1; ysv:recommended-min-version 1.5.0;}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);
    CHECK_LOG_CTX("Module \"imp-min-ver1@2026-01-01\" with version 1.0.0 but import recommended minimal version is 1.5.0.", NULL, 0);

    /* wrong statement */
    yang = "module ver3 {namespace urn:ver3; prefix v3;"
            "import ietf-yang-semver {prefix ysv;}"
            "import imp-min-ver1 {prefix imv1; ysv:recommended-min-version 1.0.0;}"
            "revision 2026-01-01 {ysv:recommended-min-version 1.0.0;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Extension ysv:recommended-min-version is allowed only in an \"import\" "
            "statement, but it is placed in \"revision\" statement.",
            "/ver3:{revision='2026-01-01'}/{ext-inst='ysv:recommended-min-version'}/1.0.0", 0);

    /* duplicate */
    yang = "module ver3 {namespace urn:ver3; prefix v3;"
            "import ietf-yang-semver {prefix ysv;}"
            "import imp-min-ver1 {prefix imv1; ysv:recommended-min-version 1.0.0; ysv:recommended-min-version 1.0.5;}"
            "revision 2026-01-01;"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Extension ysv:recommended-min-version is instantiated multiple times.",
            "/ver3:{import='imp-min-ver1'}/{ext-inst='ysv:recommended-min-version'}/1.0.0", 0);

    /* argument */
    yang = "module ver3 {namespace urn:ver3; prefix v3;"
            "import ietf-yang-semver {prefix ysv;}"
            "import imp-min-ver1 {prefix imv1; ysv:recommended-min-version;}"
            "revision 2026-01-01;"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Extension instance \"ysv:recommended-min-version\" missing argument \"yang-semantic-version\".",
            "/ver3:{import='imp-min-ver1'}/{ext-inst='ysv:recommended-min-version'}", 0);

    yang = "module ver3 {namespace urn:ver3; prefix v3;"
            "import ietf-yang-semver {prefix ysv;}"
            "import imp-min-ver1 {prefix imv1; ysv:recommended-min-version 1.0;}"
            "revision 2026-01-01;"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 semver\": Extension ysv:recommended-min-version argument yang-semantic-version \"1.0\" invalid.",
            "/ver3:{import='imp-min-ver1'}/{ext-inst='ysv:recommended-min-version'}/1.0", 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_version),
        UTEST(test_min_version),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
