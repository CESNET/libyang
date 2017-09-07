/**
 * \file test_typedef.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - typedefs and their resolution
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt;
};

static int
setup_ctx(void **state)
{
    struct state *st;
    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(NULL);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_ctx(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}
static void
test_status_yin(void **state)
{
    struct state *st = (*state);
    const char *yin = "<module name=\"status\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
        "<namespace uri=\"urn:status\"/>"
        "<prefix value=\"st\"/>"
        "<grouping name=\"g\">"
        "  <leaf name=\"gl\">"
        "    <type name=\"string\"/>"
        "    <mandatory value=\"true\"/>"
        "  </leaf>"
        "</grouping>"
        "<container name=\"a\">"
        "  <status value=\"deprecated\"/>"
        "  <leaf name=\"l\">"
        "    <type name=\"string\"/>"
        "    <mandatory value=\"true\"/>"
        "  </leaf>"
        "  <uses name=\"g\"/>"
        "</container>"
        "<container name=\"b\">"
        "  <status value=\"deprecated\"/>"
        "  <leaf name=\"l\">"
        "    <status value=\"obsolete\"/>"
        "    <mandatory value=\"true\"/>"
        "    <type name=\"string\"/>"
        "  </leaf>"
        "</container>"
        "<container name=\"c\">"
        "  <status value=\"obsolete\"/>"
        "  <leaf name=\"l\">"
        "    <mandatory value=\"true\"/>"
        "    <type name=\"string\"/>"
        "  </leaf>"
        "</container>"
        "</module>";

    const char *yin_fail1 = "<module name=\"status\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
        "<namespace uri=\"urn:status\"/>"
        "<prefix value=\"st\"/>"
        "<container name=\"c\">"
        "  <status value=\"deprecated\"/>"
        "  <leaf name=\"l\">"
        "    <status value=\"current\"/>"
        "    <type name=\"string\"/>"
        "  </leaf>"
        "</container>"
        "</module>";
    const char *yin_fail2 = "<module name=\"status\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
        "<namespace uri=\"urn:status\"/>"
        "<prefix value=\"st\"/>"
        "<container name=\"c\">"
        "  <status value=\"obsolete\"/>"
        "  <leaf name=\"l\">"
        "    <status value=\"current\"/>"
        "    <type name=\"string\"/>"
        "  </leaf>"
        "</container>"
        "</module>";
    const char *yin_fail3 = "<module name=\"status\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
        "<namespace uri=\"urn:status\"/>"
        "<prefix value=\"st\"/>"
        "<container name=\"c\">"
        "  <status value=\"obsolete\"/>"
        "  <leaf name=\"l\">"
        "    <status value=\"deprecated\"/>"
        "    <type name=\"string\"/>"
        "  </leaf>"
        "</container>"
        "</module>";

    /* deprecated nodes cannot be in obsolete data (obsolete is stronger) */
    assert_ptr_equal(NULL, lys_parse_mem(st->ctx, yin_fail3, LYS_IN_YIN));
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INSTATUS);

    /* current nodes cannot be in obsolete data (obsolete is stronger) */
    assert_ptr_equal(NULL, lys_parse_mem(st->ctx, yin_fail2, LYS_IN_YIN));
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INSTATUS);

    /* current nodes cannot be in deprecated data (deprecated is stronger) */
    assert_ptr_equal(NULL, lys_parse_mem(st->ctx, yin_fail1, LYS_IN_YIN));
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INSTATUS);

    /* status is inherited so all the mandatory statements should be ignored and empty data tree is fine */
    assert_ptr_not_equal(NULL, lys_parse_mem(st->ctx, yin, LYS_IN_YIN));
    assert_int_equal(0, lyd_validate(&st->dt, LYD_OPT_CONFIG, st->ctx));
}

static void
test_status_yang(void **state)
{
    struct state *st = (*state);
    const char *yang = "module status {"
        "  namespace urn:status;"
        "  prefix st;"
        "  grouping g {"
        "    leaf gl {"
        "      type string;"
        "      mandatory true;"
        "    }"
        "  }"
        "  container a {"
        "    status deprecated;"
        "    leaf l {"
        "      type string;"
        "      mandatory true;"
        "    }"
        "    uses g;"
        "  }"
        "  container b {"
        "    status deprecated;"
        "    leaf l {"
        "      status obsolete;"
        "      type string;"
        "      mandatory true;"
        "    }"
        "  }"
        "  container c {"
        "    status obsolete;"
        "    leaf l {"
        "      type string;"
        "      mandatory true;"
        "    }"
        "  }"
        "}";
    const char *yang_fail1 = "module status {"
        "  namespace urn:status;"
        "  prefix st;"
        "  container c {"
        "    status deprecated;"
        "    leaf l {"
        "      status current;"
        "      type string;"
        "    }"
        "  }"
        "}";
    const char *yang_fail2 = "module status {"
        "  namespace urn:status;"
        "  prefix st;"
        "  container c {"
        "    status obsolete;"
        "    leaf l {"
        "      status current;"
        "      type string;"
        "    }"
        "  }"
        "}";
    const char *yang_fail3 = "module status {"
        "  namespace urn:status;"
        "  prefix st;"
        "  container c {"
        "    status obsolete;"
        "    leaf l {"
        "      status deprecated;"
        "      type string;"
        "    }"
        "  }"
        "}";

    /* deprecated nodes cannot be in obsolete data (obsolete is stronger) */
    assert_ptr_equal(NULL, lys_parse_mem(st->ctx, yang_fail3, LYS_IN_YANG));
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INSTATUS);

    /* current nodes cannot be in obsolete data (obsolete is stronger) */
    assert_ptr_equal(NULL, lys_parse_mem(st->ctx, yang_fail2, LYS_IN_YANG));
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INSTATUS);

    /* current nodes cannot be in deprecated data (deprecated is stronger) */
    assert_ptr_equal(NULL, lys_parse_mem(st->ctx, yang_fail1, LYS_IN_YANG));
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INSTATUS);

    /* status is inherited so all the mandatory statements should be ignored and empty data tree is fine */
    assert_ptr_not_equal(NULL, lys_parse_mem(st->ctx, yang, LYS_IN_YANG));
    assert_int_equal(0, lyd_validate(&st->dt, LYD_OPT_CONFIG, st->ctx));
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_status_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_status_yang, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
