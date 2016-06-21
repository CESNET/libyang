/**
 * @file test_sec6_2.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka test for RFC 6020 section 6.2. (also 6.1.2.) conformance.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include "../config.h"
#include "../../src/libyang.h"

#define TEST_DIR "sec6_2"
#define TEST_NAME test_sec6_2
#define TEST_SCHEMA_COUNT 4
#define TEST_SCHEMA_FORMAT "yang"
#define TEST_SCHEMA_LOAD_FAIL 0,1,1,1
#define TEST_DATA_FILE_COUNT 0
#define TEST_DATA_FILE_LOAD_FAIL 0

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *node;
};

static int
setup_f(void **state)
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
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free(st->node);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
TEST_NAME(void **state)
{
    struct state *st = (*state);
    const int schemas_fail[] = {TEST_SCHEMA_LOAD_FAIL};
    const int data_files_fail[] = {TEST_DATA_FILE_LOAD_FAIL};
    char buf[512];
    const struct lys_module *mod;
    int i;

    for (i = 0; i < TEST_SCHEMA_COUNT; ++i) {
        sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/mod%d.%s", i + 1, TEST_SCHEMA_FORMAT);
        mod = lys_parse_path(st->ctx, buf, (!strcmp(TEST_SCHEMA_FORMAT, "yang") ? LYS_IN_YANG : LYS_IN_YIN));
        if (schemas_fail[i]) {
            assert_ptr_equal(mod, NULL);
        } else {
            assert_ptr_not_equal(mod, NULL);
        }
    }

    for (i = 0; i < TEST_DATA_FILE_COUNT; ++i) {
        sprintf(buf, TESTS_DIR "/conformance/data%d.xml", i + 1);
        st->node = lyd_parse_path(st->ctx, buf, LYD_XML, LYD_OPT_CONFIG);
        if (data_files_fail[i]) {
            assert_ptr_not_equal(st->node, NULL);
        } else {
            assert_ptr_equal(st->node, NULL);
        }
    }
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(TEST_NAME, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

