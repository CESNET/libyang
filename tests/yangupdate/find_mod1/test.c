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
};

static int
setup(void **state)
{
    struct st *st;

    *state = st = calloc(1, sizeof *st);
    if (!st) {
        return 1;
    }

    /* modules */
    if (ly_ctx_new(TESTS_SRC "/yangupdate/find_mod1", 0, &st->ctx1)) {
        return 1;
    }

    /* ietf-yang-schema-comparison */
    if (ly_ctx_set_searchdir(st->ctx1, TESTS_SRC "/../modules")) {
        return 1;
    }

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
    mod1 = ly_ctx_load_module(st->ctx1, "find_mod1", "2025-01-01", NULL);
    assert_non_null(mod1);

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
    mod1 = ly_ctx_load_module(st->ctx1, "find_mod1", "2025-01-01", NULL);
    assert_non_null(mod1);

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
    mod1 = ly_ctx_load_module(st->ctx1, "find_mod1", "2025-01-01", NULL);
    assert_non_null(mod1);

    /* find the specific features revision */
    assert_int_equal(LY_SUCCESS, lyd_update_find_new(mod1, NULL, features, &st->ctx2, &mod2));

    assert_string_equal(mod2->revision, "2025-12-01");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_latest, setup, teardown),
        cmocka_unit_test_setup_teardown(test_revision, setup, teardown),
        cmocka_unit_test_setup_teardown(test_features, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
