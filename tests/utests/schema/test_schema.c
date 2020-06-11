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

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

#include "../../../src/common.h"
#include "../../../src/context.h"
#include "../../../src/log.h"
#include "../../../src/parser_yin.h"
#include "../../../src/tree_schema.h"
#include "../../../src/tree_schema_internal.h"
#include "../../../src/xml.h"


#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    if (store) {
        if (path && path[0]) {
            snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
        } else {
            strncpy(logbuf, msg, BUFSIZE - 1);
        }
        if (store > 0) {
            --store;
        }
    }
}
#endif

static int
logger_setup(void **state)
{
    (void) state; /* unused */

#if ENABLE_LOGGER_CHECKING
    /* setup logger */
    ly_set_log_clb(logger, 1);
#endif

    return 0;
}

static int
logger_teardown(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif
    return 0;
}

void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

/**
 * INCLUDE OTHER SCHEMA TESTS
 */
#include "test_schema_common.c"
#include "test_schema_stmts.c"

int main(void)
{
    const struct CMUnitTest tests[] = {
        /** test_schema_common.c */
        cmocka_unit_test_setup_teardown(test_getnext, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_date, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_revisions, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_typedef, logger_setup, logger_teardown),

        /** test_schema_stmts.c */
        cmocka_unit_test_setup_teardown(test_identity, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_feature, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
