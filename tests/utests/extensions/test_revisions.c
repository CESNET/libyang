/**
 * @file test_revisions.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for revisions extensions support
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
test_nbc(void **state)
{
    struct lys_module *mod;
    const char *yang;

    /* valid yang */
    yang = "module rev1 {namespace urn:rev1; prefix r1;"
            "import ietf-yang-revisions {prefix rev;}"
            "revision 2026-01-01 {rev:non-backwards-compatible;}"
            "leaf a {type string;}"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, &mod);

    /* wrong statement */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-revisions {prefix rev;}"
            "revision 2026-01-01 {rev:non-backwards-compatible;}"
            "leaf a {type string {rev:non-backwards-compatible;}}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 revisions\": Extension rev:non-backwards-compatible is allowed only in "
            "a \"revision\" statement, but it is placed in \"type\" statement.",
            "/rev2:{type='string'}/{ext-inst='rev:non-backwards-compatible'}", 0);

    /* duplicate */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-revisions {prefix rev;}"
            "revision 2026-01-01 {rev:non-backwards-compatible; rev:non-backwards-compatible;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 revisions\": Extension rev:non-backwards-compatible is instantiated multiple times.",
            "/rev2:{revision='2026-01-01'}/{ext-inst='rev:non-backwards-compatible'}", 0);

    /* argument */
    yang = "module rev2 {namespace urn:rev2; prefix r2;"
            "import ietf-yang-revisions {prefix rev;}"
            "revision 2026-01-01 {rev:non-backwards-compatible nbc;}"
            "leaf a {type string;}"
            "}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 revisions\": Extension rev:non-backwards-compatible requires no argument but \"nbc\" used.",
            "/rev2:{revision='2026-01-01'}/{ext-inst='rev:non-backwards-compatible'}/nbc", 0);
}

static void
test_min_date(void **state)
{
    struct lys_module *mod;
    const char *yang;

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
    CHECK_LOG_CTX("Module \"ietf-yang-types@2025-12-22\" import recommended minimal date 2030-12-22.",
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
        UTEST(test_nbc),
        UTEST(test_min_date),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
