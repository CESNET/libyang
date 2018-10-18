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

static void
test_date(void **state)
{
    (void) state; /* unused */

    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, NULL, 0, "date"));
    assert_string_equal(logbuf, "Invalid argument date (lysp_check_date()).");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "x", 1, "date"));
    assert_string_equal(logbuf, "Invalid argument date_len (lysp_check_date()).");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "nonsencexx", 10, "date"));
    assert_string_equal(logbuf, "Invalid value \"nonsencexx\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "123x-11-11", 10, "date"));
    assert_string_equal(logbuf, "Invalid value \"123x-11-11\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-13-11", 10, "date"));
    assert_string_equal(logbuf, "Invalid value \"2018-13-11\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-11-41", 10, "date"));
    assert_string_equal(logbuf, "Invalid value \"2018-11-41\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02-29", 10, "date"));
    assert_string_equal(logbuf, "Invalid value \"2018-02-29\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018.02-28", 10, "date"));
    assert_string_equal(logbuf, "Invalid value \"2018.02-28\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02.28", 10, "date"));
    assert_string_equal(logbuf, "Invalid value \"2018-02.28\" of \"date\".");

    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-11-11", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-02-28", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2016-02-29", 10, "date"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_date, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
