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

#include "test_schema.h"

#include <string.h>

#include "log.h"
#include "parser_schema.h"
#include "tests/config.h"
#include "tree_schema.h"
#include "utests.h"

#if ENABLE_LOGGER_CHECKING

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

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
#if ENABLE_LOGGER_CHECKING
    logbuf[0] = '\0';
#endif
}

LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YANG;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

/**
 * DECLARE OTHER SCHEMA TESTS
 */
/* test_schema_common.c */
void test_getnext(void **state);
void test_date(void **state);
void test_revisions(void **state);
void test_typedef(void **state);
void test_accessible_tree(void **state);

/* test_schema_stmts.c */
void test_identity(void **state);
void test_feature(void **state);

int
main(void)
{
    const struct CMUnitTest tests[] = {
        /** test_schema_common.c */
        cmocka_unit_test_setup_teardown(test_getnext, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_date, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_revisions, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_typedef, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_accessible_tree, logger_setup, logger_teardown),

        /** test_schema_stmts.c */
        cmocka_unit_test_setup_teardown(test_identity, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_feature, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
