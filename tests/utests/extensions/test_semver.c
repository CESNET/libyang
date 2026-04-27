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

    /* TODO */

    /* valid yang */
    yang = "module rev1 {namespace urn:rev1; prefix r1;"
            "import ietf-yang-revisions {prefix rev;}"
            "import ietf-yang-types {prefix yang; rev:recommended-min-date 2025-12-22;}"
            "leaf a {type yang:hex-string;}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);

    /* unsatisfied min-date */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-revisions {prefix rev;}"
            "import ietf-yang-types {prefix yang; rev:recommended-min-date 2030-12-22;}"
            "leaf a {type yang:hex-string;}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);
    CHECK_LOG_CTX("Module \"rev2\" recommended minimal date of import \"ietf-yang-types\" is 2030-12-22 "
            "but the imported module revision is 2025-12-22.",
            NULL, 0);

    /* wrong statement */
    yang = "module rev3 {namespace urn:rev3; prefix r3;"
            "import ietf-yang-revisions {prefix rev;}"
            "import ietf-yang-types {prefix yang; rev:recommended-min-date 2025-12-22;}"
            "revision 2026-01-01 {rev:recommended-min-date 2025-12-22;}"
            "leaf a {type yang:hex-string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 revisions\": Extension rev:recommended-min-date is allowed only in an \"import\" "
            "statement, but it is placed in \"revision\" statement.",
            "/rev3:{revision='2026-01-01'}/{ext-inst='rev:recommended-min-date'}/2025-12-22", 0);

    /* duplicate */
    yang = "module rev3 {namespace urn:rev3; prefix r3;"
            "import ietf-yang-revisions {prefix rev;}"
            "import ietf-yang-types {prefix yang; rev:recommended-min-date 2025-12-22; rev:recommended-min-date 2022-12-22;}"
            "leaf a {type yang:hex-string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 revisions\": Extension rev:recommended-min-date is instantiated multiple times.",
            "/rev3:{import='ietf-yang-types'}/{ext-inst='rev:recommended-min-date'}/2025-12-22", 0);

    /* argument */
    yang = "module rev3 {namespace urn:rev3; prefix r3;"
            "import ietf-yang-revisions {prefix rev;}"
            "import ietf-yang-types {prefix yang; rev:recommended-min-date;}"
            "leaf a {type yang:hex-string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Extension instance \"rev:recommended-min-date\" missing argument \"revision-date\".",
            "/rev3:{import='ietf-yang-types'}/{ext-inst='rev:recommended-min-date'}", 0);

    yang = "module rev3 {namespace urn:rev3; prefix r3;"
            "import ietf-yang-revisions {prefix rev;}"
            "import ietf-yang-types {prefix yang; rev:recommended-min-date 1999-02-30;}"
            "leaf a {type yang:hex-string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 revisions\": Extension rev:recommended-min-date argument revision-date \"1999-02-30\" invalid.",
            "/rev3:{import='ietf-yang-types'}/{ext-inst='rev:recommended-min-date'}/1999-02-30", 0);
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
