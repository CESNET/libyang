/**
 * \file test_typedef.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - typedefs and their resolution
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

struct state {
    struct ly_ctx *ctx;
    int fd;
    char *str1;
    char *str2;
};

static int
setup_ctx(void **state)
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
teardown_ctx(void **state)
{
    struct state *st = (*state);

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
test_typedef_yin(void **state)
{
    const char *schema = TESTS_DIR"/schema/yin/files/f.yin";
    struct state *st = (*state);
    struct stat s;
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, schema, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    st->fd = open(schema, O_RDONLY);
    fstat(st->fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(st->fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), mod, LYS_OUT_YIN, NULL);

    assert_string_equal(st->str1, st->str2);
}

static void
test_typedef_yang(void **state)
{
    const char *schema = TESTS_DIR"/schema/yang/files/f.yang";
    struct state *st = (*state);
    struct stat s;
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, schema, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->fd = open(schema, O_RDONLY);
    fstat(st->fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(st->fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), mod, LYS_OUT_YANG, NULL);

    assert_string_equal(st->str1, st->str2);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_typedef_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_yang, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
