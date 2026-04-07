/**
 * @file test.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for yangupdate
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* strdup, setenv, tzset */

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

#include "libyang.h"
#include "tests_config.h"
#include "yang_update.h"

struct st {
    struct ly_ctx *ctx1;
    struct ly_ctx *ctx2;
    const char **search_dirs;
};

const char *glob_search_dirs[] = {TESTS_SRC "/yangupdate/find_mod1", TESTS_SRC "/../modules", NULL};

static int
setup(void **state)
{
    struct st *st;

    *state = st = calloc(1, sizeof *st);
    if (!st) {
        return 1;
    }

    /* search dirs */
    st->search_dirs = glob_search_dirs;

    return 0;
}

static int
teardown(void **state)
{
    struct st *st = *state;

    if (!st) {
        return 0;
    }

    ly_ctx_destroy(st->ctx1);
    ly_ctx_destroy(st->ctx2);
    free(st);

    return 0;
}

static void
test_latest(void **state)
{
    struct st *st = *state;
    struct lys_module *mod1, *mod2;

    /* load old module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_old("find_mod1", "2025-01-01", NULL, st->search_dirs, &st->ctx1, &mod1));

    /* find the latest module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_new(mod1, NULL, NULL, &st->ctx2, &mod2));
    assert_string_equal(mod2->revision, "2025-12-01");
}

static void
test_revision(void **state)
{
    struct st *st = *state;
    struct lys_module *mod1, *mod2;

    /* load old module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_old("find_mod1", "2025-01-01", NULL, st->search_dirs, &st->ctx1, &mod1));

    /* find the specific revision */
    assert_int_equal(LY_SUCCESS, lyd_update_find_new(mod1, "2025-09-01", NULL, &st->ctx2, &mod2));

    assert_string_equal(mod2->revision, "2025-09-01");
}

static void
test_features(void **state)
{
    struct st *st = *state;
    struct lys_module *mod1, *mod2;
    const char *features[] = {"feat1", "feat2", NULL};

    /* load old module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_old("find_mod1", "2025-01-01", NULL, st->search_dirs, &st->ctx1, &mod1));

    /* find the specific features revision */
    assert_int_equal(LY_SUCCESS, lyd_update_find_new(mod1, NULL, features, &st->ctx2, &mod2));

    assert_string_equal(mod2->revision, "2025-12-01");
}

static void
test_invalid_old(void **state)
{
    struct st *st = *state;
    struct lys_module *mod1;
    const char *features[] = {"feat1", "feattt2", NULL};

    /* non-existing plugin module name */
    assert_int_equal(LY_ENOTFOUND, lyd_update_find_old("findd_mod1", "2025-10-01", NULL, st->search_dirs, &st->ctx1, &mod1));

    /* non-existing plugin revision */
    assert_int_equal(LY_ENOTFOUND, lyd_update_find_old("find_mod1", "2025-10-02", NULL, st->search_dirs, &st->ctx1, &mod1));

    /* non-existing plugin features */
    assert_int_equal(LY_ENOTFOUND, lyd_update_find_old("find_mod1", "2025-12-01", features, st->search_dirs, &st->ctx1, &mod1));
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_latest, setup, teardown),
        cmocka_unit_test_setup_teardown(test_revision, setup, teardown),
        cmocka_unit_test_setup_teardown(test_features, setup, teardown),
        cmocka_unit_test_setup_teardown(test_invalid_old, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
