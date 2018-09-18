/*
 * @file set.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from set.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

#include "libyang.h"
#include "../../src/set.c"

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
test_basics(void **state)
{
    (void) state; /* unused */

    struct ly_set *set;
    char *str;
    unsigned int u;
    void *ptr;

    /* creation - everything is empty */
    set = ly_set_new();
    assert_non_null(set);
    assert_int_equal(0, set->count);
    assert_int_equal(0, set->size);
    assert_null(set->objs);

    /* add a testing object */
    str = strdup("test string");
    assert_non_null(str);

    ly_set_add(set, str, 0);
    assert_int_not_equal(0, set->size);
    assert_int_equal(1, set->count);
    assert_non_null(set->objs);
    assert_non_null(set->objs[0]);

    /* check the presence of the testing data */
    assert_int_equal(0, ly_set_contains(set, str));
    assert_int_equal(-1, ly_set_contains(set, str - 1));

    /* remove data, but keep the set */
    u = set->size;
    ptr = set->objs;
    ly_set_clean(set, free);
    assert_int_equal(0, set->count);
    assert_int_equal(u, set->size);
    assert_ptr_equal(ptr, set->objs);

    /* remove buffer, but keep the set object */
    ly_set_erase(set, NULL);
    assert_int_equal(0, set->count);
    assert_int_equal(0, set->size);
    assert_ptr_equal(NULL, set->objs);

    /* final cleanup */
    ly_set_free(set, NULL);
}

static void
test_inval(void **state)
{
    struct ly_set set;
    memset(&set, 0, sizeof set);

    ly_set_clean(NULL, NULL);
    assert_string_equal(logbuf, "Invalid argument set (ly_set_clean()).");

    ly_set_erase(NULL, NULL);
    assert_string_equal(logbuf, "Invalid argument set (ly_set_erase()).");

    ly_set_free(NULL, NULL);
    assert_string_equal(logbuf, "Invalid argument set (ly_set_free()).");

    assert_null(ly_set_dup(NULL));
    assert_string_equal(logbuf, "Invalid argument set (ly_set_dup()).");

    assert_int_equal(-1, ly_set_add(NULL, NULL, 0));
    assert_string_equal(logbuf, "Invalid argument set (ly_set_add()).");
    assert_int_equal(-1, ly_set_add(&set, NULL, 0));
    assert_string_equal(logbuf, "Invalid argument object (ly_set_add()).");

    assert_int_equal(-1, ly_set_merge(NULL, NULL, 0));
    assert_string_equal(logbuf, "Invalid argument trg (ly_set_merge()).");
    assert_int_equal(0, ly_set_merge(&set, NULL, 0));
    assert_string_equal(logbuf, "Invalid argument src (ly_set_merge()).");

    assert_int_equal(LY_EINVAL, ly_set_rm_index(NULL, 0));
    assert_string_equal(logbuf, "Invalid argument set (ly_set_rm_index()).");
    assert_int_equal(LY_EINVAL, ly_set_rm_index(&set, 1));
    assert_string_equal(logbuf, "Invalid argument index (ly_set_rm_index()).");

    assert_int_equal(LY_EINVAL, ly_set_rm(NULL, NULL));
    assert_string_equal(logbuf, "Invalid argument set (ly_set_rm()).");
    assert_int_equal(LY_EINVAL, ly_set_rm(&set, NULL));
    assert_string_equal(logbuf, "Invalid argument object (ly_set_rm()).");
    assert_int_equal(LY_EINVAL, ly_set_rm(&set, &state));
    assert_string_equal(logbuf, "Invalid argument object (ly_set_rm()).");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_basics),
        cmocka_unit_test_setup(test_inval, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
