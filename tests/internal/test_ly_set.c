/**
 * @file test_ly_set.c
 * @author Olivier Matz <olivier.matz@6wind.com>
 * @brief Cmocka tests for ly_set.
 *
 * Copyright (c) 2018 6WIND S.A.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

static struct ly_set *set;

static int
setup_f(void **state)
{
    (void)state;

    set = ly_set_new();
    if (!set) {
        fprintf(stderr, "Failed to create ly_set.\n");
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void)state;

    ly_set_free(set);
    return 0;
}

static void
test_list(void **state)
{
    struct ly_set *set2;
    int ret;
    size_t i;

    (void)state;

    /* invalid params */
    ret = ly_set_contains(NULL, 0);
    assert_int_equal(ret, -1);
    set2 = ly_set_dup(NULL);
    assert_ptr_equal(set2, NULL);
    ret = ly_set_add(NULL, NULL, LY_SET_OPT_USEASLIST);
    assert_int_equal(ret, -1);
    ret = ly_set_merge(NULL, set, LY_SET_OPT_USEASLIST);
    assert_int_equal(ret, -1);
    ret = ly_set_rm(set, NULL);
    assert_int_equal(ret, EXIT_FAILURE);
    ret = ly_set_rm(NULL, (void *)1);
    assert_int_equal(ret, EXIT_FAILURE);
    ret = ly_set_clean(NULL);
    assert_int_equal(ret, EXIT_FAILURE);

    /* add 100 elements */
    for (i = 1; i <= 100; i++) {
        ret = ly_set_add(set, (void *)i, LY_SET_OPT_USEASLIST);
        assert_int_equal(ret, i - 1);
    }
    assert_int_equal(set->number, 100);

    /* create a new set with 100 elements, and merge them in first one */
    set2 = ly_set_new();
    assert_ptr_not_equal(set2, NULL);
    for (i = 101; i <= 200; i++) {
        ret = ly_set_add(set2, (void *)i, LY_SET_OPT_USEASLIST);
        assert_int_equal(ret, i - 101);
    }
    ret = ly_set_merge(set, set2, LY_SET_OPT_USEASLIST);
    assert_int_equal(ret, 100);
    assert_int_equal(set->number, 200);
    set2 = NULL; /* set2 is freed */

    /* merge NULL set */
    ret = ly_set_merge(set, NULL, LY_SET_OPT_USEASLIST);
    assert_int_equal(ret, 0);

    /* duplicate set */
    set2 = ly_set_dup(set);
    assert_ptr_not_equal(set2, NULL);
    ly_set_free(set);
    set = set2;
    set2 = NULL;

    /* check indexes */
    for (i = 1; i <= 200; i++) {
        ret = ly_set_contains(set, (void *)i);
        assert_int_equal(ret, i - 1);
    }

    /* unexistant */
    ret = ly_set_contains(set, (void *)201);
    assert_int_equal(ret, -1);
    ret = ly_set_rm(set, (void *)201);
    assert_int_equal(ret, EXIT_FAILURE);

    /* remove all even values */
    for (i = 1; i <= 200; i++) {
        if (i & 1)
            continue;
        ret = ly_set_rm(set, (void *)i);
        assert_int_equal(ret, 0);
    }
    assert_int_equal(set->number, 100);

    /* remove values from index */
    ret = ly_set_rm_index(set, 99);
    assert_int_equal(ret, 0);
    assert_int_equal(set->number, 99);
    ret = ly_set_rm_index(set, 50);
    assert_int_equal(ret, 0);
    assert_int_equal(set->number, 98);
    ret = ly_set_rm_index(set, 0);
    assert_int_equal(ret, 0);
    assert_int_equal(set->number, 97);

    /* invalid index */
    ret = ly_set_rm_index(set, 100);
    assert_int_equal(ret, EXIT_FAILURE);

    /* add same element twice */
    ret = ly_set_add(set, (void *)1337, LY_SET_OPT_USEASLIST);
    assert_int_equal(ret, 97);
    assert_int_equal(set->number, 98);
    ret = ly_set_add(set, (void *)1337, LY_SET_OPT_USEASLIST);
    assert_int_equal(ret, 98);
    assert_int_equal(set->number, 99);

    ly_set_clean(set);
    assert_int_equal(set->number, 0);

    /* add 100 elements after clean */
    for (i = 1; i <= 100; i++) {
        ret = ly_set_add(set, (void *)i, LY_SET_OPT_USEASLIST);
        assert_int_equal(ret, i - 1);
    }
    assert_int_equal(set->number, 100);

    /* check free(NULL) */
    ly_set_free(NULL);
}

static void
test_dict(void **state)
{
    struct ly_set *set2;
    int ret;
    size_t i;

    (void)state;

    /* invalid params */
    ret = ly_set_contains(NULL, 0);
    assert_int_equal(ret, -1);
    set2 = ly_set_dup(NULL);
    assert_ptr_equal(set2, NULL);
    ret = ly_set_add(NULL, NULL, 0);
    assert_int_equal(ret, -1);
    ret = ly_set_merge(NULL, set, 0);
    assert_int_equal(ret, -1);
    ret = ly_set_rm(set, NULL);
    assert_int_equal(ret, EXIT_FAILURE);
    ret = ly_set_rm(NULL, (void *)1);
    assert_int_equal(ret, EXIT_FAILURE);
    ret = ly_set_clean(NULL);
    assert_int_equal(ret, EXIT_FAILURE);

    /* add 100 elements */
    for (i = 1; i <= 100; i++) {
        ret = ly_set_add(set, (void *)i, 0);
        assert_int_equal(ret, i - 1);
    }
    assert_int_equal(set->number, 100);

    /* create a new set with 100 same elements, and merge them in first one */
    set2 = ly_set_new();
    assert_ptr_not_equal(set2, NULL);
    for (i = 1; i <= 100; i++) {
        ret = ly_set_add(set2, (void *)i, 0);
        assert_int_equal(ret, i - 1);
    }
    ret = ly_set_merge(set, set2, 0);
    assert_int_equal(ret, 0);
    assert_int_equal(set->number, 100);
    set2 = NULL; /* set2 is freed */

    /* merge NULL set */
    ret = ly_set_merge(set, NULL, 0);
    assert_int_equal(ret, 0);

    /* create a new set with 100 elements, and merge them in first one */
    set2 = ly_set_new();
    for (i = 101; i <= 200; i++) {
        ret = ly_set_add(set2, (void *)i, 0);
        assert_int_equal(ret, i - 101);
    }
    ly_set_merge(set, set2, 0);
    set2 = NULL; /* set2 is freed */
    assert_int_equal(set->number, 200);

    /* duplicate set */
    set2 = ly_set_dup(set);
    assert_ptr_not_equal(set2, NULL);
    ly_set_free(set);
    set = set2;
    set2 = NULL;

    /* check indexes */
    for (i = 1; i <= 200; i++) {
        ret = ly_set_contains(set, (void *)i);
        assert_int_equal(ret, i - 1);
    }

    /* unexistant */
    ret = ly_set_contains(set, (void *)201);
    assert_int_equal(ret, -1);
    ret = ly_set_rm(set, (void *)201);
    assert_int_equal(ret, EXIT_FAILURE);

    /* remove all even values */
    for (i = 1; i <= 200; i++) {
        if (i & 1)
            continue;
        ret = ly_set_rm(set, (void *)i);
        assert_int_equal(ret, 0);
    }
    assert_int_equal(set->number, 100);

    /* remove values from index */
    ret = ly_set_rm_index(set, 99);
    assert_int_equal(ret, 0);
    assert_int_equal(set->number, 99);
    ret = ly_set_rm_index(set, 50);
    assert_int_equal(ret, 0);
    assert_int_equal(set->number, 98);
    ret = ly_set_rm_index(set, 0);
    assert_int_equal(ret, 0);
    assert_int_equal(set->number, 97);

    /* invalid index */
    ret = ly_set_rm_index(set, 100);
    assert_int_equal(ret, EXIT_FAILURE);

    /* add same element twice */
    ret = ly_set_add(set, (void *)1337, 0);
    assert_int_equal(ret, 97);
    assert_int_equal(set->number, 98);
    ret = ly_set_add(set, (void *)1337, 0);
    assert_int_equal(ret, 97);
    assert_int_equal(set->number, 98);

    ly_set_clean(set);
    assert_int_equal(set->number, 0);

    /* add 100 elements after clean */
    for (i = 1; i <= 100; i++) {
        ret = ly_set_add(set, (void *)i, 0);
        assert_int_equal(ret, i - 1);
    }
    assert_int_equal(set->number, 100);

    /* check free(NULL) */
    ly_set_free(NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_list, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_dict, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
