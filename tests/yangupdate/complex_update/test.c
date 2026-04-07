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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

#include "libyang.h"
#include "tests_config.h"
#include "yang_update.h"

struct st {
    struct ly_ctx *ctx1;
    struct ly_ctx *ctx2;
    struct lyd_node *data1;
    struct lyd_node *data2;
    char *str1;
    char *str2;
    FILE *f;
};

static int
setup(void **state)
{
    struct st *st;

    *state = st = calloc(1, sizeof *st);
    if (!st) {
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

    free(st->str1);
    free(st->str2);
    if (st->f) {
        fclose(st->f);
    }
    lyd_free_siblings(st->data1);
    lyd_free_siblings(st->data2);
    ly_ctx_destroy(st->ctx1);
    ly_ctx_destroy(st->ctx2);
    free(st);

    return 0;
}

static void
test_update_01_01_to_01_20(void **state)
{
    struct st *st = *state;
    struct lys_module *mod1, *mod2;
    const char *search_dirs[] = {TESTS_SRC "/yangupdate/complex_update", TESTS_SRC "/../modules", NULL};
    long size;

    /* load old module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_old("ydct", "2026-01-01", NULL, search_dirs, &st->ctx1, &mod1));

    /* load old data */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_path(st->ctx1,
            TESTS_SRC "/yangupdate/complex_update/data-2026-01-01.json", LYD_JSON, 0, LYD_VALIDATE_PRESENT, &st->data1));

    /* find the latest module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_new(mod1, NULL, NULL, &st->ctx2, &mod2));
    assert_string_equal(mod2->revision, "2026-01-20");

    /* update data */
    assert_int_equal(LY_SUCCESS, lyd_update(mod1, st->data1, mod2, &st->data2));

    /* check the updated data */
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&st->str2, st->data2, LYD_JSON, LYD_PRINT_SIBLINGS));
    st->f = fopen(TESTS_SRC "/yangupdate/complex_update/data-2026-01-20-from-2026-01-01.json", "r");
    assert_non_null(st->f);
    fseek(st->f, 0, SEEK_END);
    size = ftell(st->f);
    rewind(st->f);
    st->str1 = malloc(size + 1);
    assert_non_null(st->str1);
    fread(st->str1, 1, size, st->f);
    st->str1[size] = '\0';
    assert_string_equal(st->str2, st->str1);
}

static void
test_update_01_10_to_01_15(void **state)
{
    struct st *st = *state;
    struct lys_module *mod1, *mod2;
    const char *search_dirs[] = {TESTS_SRC "/yangupdate/complex_update", TESTS_SRC "/../modules", NULL};
    long size;

    /* load old module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_old("ydct", "2026-01-10", NULL, search_dirs, &st->ctx1, &mod1));

    /* load old data */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_path(st->ctx1,
            TESTS_SRC "/yangupdate/complex_update/data-2026-01-10.json", LYD_JSON, 0, LYD_VALIDATE_PRESENT, &st->data1));

    /* find the latest module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_new(mod1, "2026-01-15", NULL, &st->ctx2, &mod2));
    assert_string_equal(mod2->revision, "2026-01-15");

    /* update data */
    assert_int_equal(LY_SUCCESS, lyd_update(mod1, st->data1, mod2, &st->data2));

    /* check the updated data */
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&st->str2, st->data2, LYD_JSON, LYD_PRINT_SIBLINGS));
    st->f = fopen(TESTS_SRC "/yangupdate/complex_update/data-2026-01-15-from-2026-01-10.json", "r");
    assert_non_null(st->f);
    fseek(st->f, 0, SEEK_END);
    size = ftell(st->f);
    rewind(st->f);
    st->str1 = malloc(size + 1);
    assert_non_null(st->str1);
    fread(st->str1, 1, size, st->f);
    st->str1[size] = '\0';
    assert_string_equal(st->str2, st->str1);
}

static void
test_update_01_15_to_01_20(void **state)
{
    struct st *st = *state;
    struct lys_module *mod1, *mod2;
    const char *search_dirs[] = {TESTS_SRC "/yangupdate/complex_update", TESTS_SRC "/../modules", NULL};
    const char *features_old[] = {"feature-02", NULL};
    long size;

    /* load old module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_old("ydct", "2026-01-15", features_old, search_dirs, &st->ctx1, &mod1));

    /* load old data */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_path(st->ctx1,
            TESTS_SRC "/yangupdate/complex_update/data-2026-01-15.json", LYD_JSON, 0, LYD_VALIDATE_PRESENT, &st->data1));

    /* find the latest module */
    assert_int_equal(LY_SUCCESS, lyd_update_find_new(mod1, "2026-01-20", NULL, &st->ctx2, &mod2));
    assert_string_equal(mod2->revision, "2026-01-20");

    /* update data */
    assert_int_equal(LY_SUCCESS, lyd_update(mod1, st->data1, mod2, &st->data2));

    /* check the updated data */
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&st->str2, st->data2, LYD_JSON, LYD_PRINT_SIBLINGS));
    st->f = fopen(TESTS_SRC "/yangupdate/complex_update/data-2026-01-20-from-2026-01-15.json", "r");
    assert_non_null(st->f);
    fseek(st->f, 0, SEEK_END);
    size = ftell(st->f);
    rewind(st->f);
    st->str1 = malloc(size + 1);
    assert_non_null(st->str1);
    fread(st->str1, 1, size, st->f);
    st->str1[size] = '\0';
    assert_string_equal(st->str2, st->str1);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_update_01_01_to_01_20, setup, teardown),
        cmocka_unit_test_setup_teardown(test_update_01_10_to_01_15, setup, teardown),
        cmocka_unit_test_setup_teardown(test_update_01_15_to_01_20, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
