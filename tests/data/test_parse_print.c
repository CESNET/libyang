/**
 * @file test_parse_print.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for parsing and printing both schema and data.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include "../config.h"
#include "../../src/libyang.h"

struct state {
    struct ly_ctx *ctx;
    const struct lys_module *mod;
    struct lyd_node *dt;
    int fd;
    char *str1;
    char *str2;
};

int
setup_f(void **state)
{
    struct state *st;
    const char *schema = TESTS_DIR"/data/files/all-dev.yin";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR"/data/files");
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schema */
    st->mod = lys_parse_path(st->ctx, schema, LYS_IN_YIN);
    if (!st->mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schema);
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    if (st->fd > 0) {
        close(st->fd);
    }
    free(st->str1);
    free(st->str2);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_parse_print_yin(void **state)
{
    struct state *st = (*state);
    struct stat s;
    int fd;

    *state = st = calloc(1, sizeof *st);
    assert_ptr_not_equal(st, NULL);

    st->ctx = ly_ctx_new(TESTS_DIR"/data/files");
    assert_ptr_not_equal(st->ctx, NULL);

    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/all.yin", LYS_IN_YIN);
    assert_ptr_not_equal(st->mod, NULL);

    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/all-dev.yin", LYS_IN_YIN);
    assert_ptr_not_equal(st->mod, NULL);

    fd = open(TESTS_DIR"/data/files/all-dev.yin", O_RDONLY);
    fstat(fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), st->mod, LYS_OUT_YIN, NULL);

    assert_string_equal(st->str1, st->str2);

    close(fd);
    fd = -1;
    free(st->str1);
    st->str1 = NULL;
    free(st->str2);
    st->str2 = NULL;

    st->mod = ly_ctx_get_module(st->ctx, "all", NULL);
    assert_ptr_not_equal(st->mod, NULL);

    fd = open(TESTS_DIR"/data/files/all.yin", O_RDONLY);
    fstat(fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), st->mod, LYS_OUT_YIN, NULL);

    assert_string_equal(st->str1, st->str2);
}

void
test_parse_print_yang(void **state)
{
    struct state *st = (*state);
    struct stat s;
    int fd;

    *state = st = calloc(1, sizeof *st);
    assert_ptr_not_equal(st, NULL);

    st->ctx = ly_ctx_new(TESTS_DIR"/data/files");
    assert_ptr_not_equal(st->ctx, NULL);

    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/all.yang", LYS_IN_YANG);
    assert_ptr_not_equal(st->mod, NULL);

    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/all-dev.yang", LYS_IN_YANG);
    assert_ptr_not_equal(st->mod, NULL);

    fd = open(TESTS_DIR"/data/files/all-dev.yang", O_RDONLY);
    fstat(fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), st->mod, LYS_OUT_YANG, NULL);

    assert_string_equal(st->str1, st->str2);

    close(fd);
    fd = -1;
    free(st->str1);
    st->str1 = NULL;
    free(st->str2);
    st->str2 = NULL;

    st->mod = ly_ctx_get_module(st->ctx, "all", NULL);
    assert_ptr_not_equal(st->mod, NULL);

    fd = open(TESTS_DIR"/data/files/all.yang", O_RDONLY);
    fstat(fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), st->mod, LYS_OUT_YANG, NULL);

    if (strcmp(st->str1, st->str2)) {
        printf("\"%s\"\n", st->str2);
        fail();
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_teardown(test_parse_print_yin, teardown_f),
                    //cmocka_unit_test_teardown(test_parse_print_yang, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
