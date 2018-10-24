/*
 * @file set.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from common.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../../src/tree_schema_helpers.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    (void) path; /* unused */

    strncpy(logbuf, msg, BUFSIZE - 1);
}

static int
logger_setup(void **state)
{
    (void) state; /* unused */

    ly_set_log_clb(logger, 0);

    return 0;
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

static void
test_date(void **state)
{
    (void) state; /* unused */

    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, NULL, 0, "date"));
    logbuf_assert("Invalid argument date (lysp_check_date()).");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "x", 1, "date"));
    logbuf_assert("Invalid argument date_len (lysp_check_date()).");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "nonsencexx", 10, "date"));
    logbuf_assert("Invalid value \"nonsencexx\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "123x-11-11", 10, "date"));
    logbuf_assert("Invalid value \"123x-11-11\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-13-11", 10, "date"));
    logbuf_assert("Invalid value \"2018-13-11\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-11-41", 10, "date"));
    logbuf_assert("Invalid value \"2018-11-41\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02-29", 10, "date"));
    logbuf_assert("Invalid value \"2018-02-29\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018.02-28", 10, "date"));
    logbuf_assert("Invalid value \"2018.02-28\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02.28", 10, "date"));
    logbuf_assert("Invalid value \"2018-02.28\" of \"date\".");

    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-11-11", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-02-28", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2016-02-29", 10, "date"));
}

static void
test_revisions(void **state)
{
    (void) state; /* unused */

    struct lysp_revision *revs = NULL, *rev;

    /* no error, it just does nothing */
    lysp_sort_revisions(NULL);
    logbuf_assert("");

    /* revisions are stored in wrong order - the newest is the last */
    LY_ARRAY_NEW_RET(NULL, revs, rev,);
    strcpy(rev->date, "2018-01-01");
    LY_ARRAY_NEW_RET(NULL, revs, rev,);
    strcpy(rev->date, "2018-12-31");

    assert_int_equal(2, LY_ARRAY_SIZE(revs));
    assert_string_equal("2018-01-01", &revs[0]);
    assert_string_equal("2018-12-31", &revs[1]);
    /* the order should be fixed, so the newest revision will be the first in the array */
    lysp_sort_revisions(revs);
    assert_string_equal("2018-12-31", &revs[0]);
    assert_string_equal("2018-01-01", &revs[1]);

    LY_ARRAY_FREE(revs);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_date, logger_setup),
        cmocka_unit_test_setup(test_revisions, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
