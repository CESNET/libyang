/**
 * \file test_extensions.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - extensions
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
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
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/files"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"


static int
setup_ctx_yin(void **state)
{
    struct ly_ctx *ctx;

    ctx = ly_ctx_new(SCHEMA_FOLDER_YIN);
    assert_non_null(ctx);

    *state = ctx;
    return 0;
}

static int
setup_ctx_yang(void **state)
{
    struct ly_ctx *ctx;

    ctx = ly_ctx_new(SCHEMA_FOLDER_YANG);
    assert_non_null(ctx);

    *state = ctx;
    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy(*state, NULL);

    return 0;
}

static void
test_fullset_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *mod;
    char *printed = NULL, *orig = NULL;
    int fd;
    struct stat s;

    mod = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/ext.yin", LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&printed, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(printed, NULL);

    fd = open(SCHEMA_FOLDER_YIN"/ext.yin", O_RDONLY);
    fstat(fd, &s);
    orig = malloc(s.st_size + 1);
    assert_ptr_not_equal(orig, NULL);
    assert_int_equal(read(fd, orig, s.st_size), s.st_size);
    orig[s.st_size] = '\0';

    assert_string_equal(printed, orig);
    free(printed);
    free(orig);
}

static void
test_fullset_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *mod;
    char *printed = NULL, *orig = NULL;
    int fd;
    struct stat s;

    mod = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/ext.yang", LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&printed, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(printed, NULL);

    fd = open(SCHEMA_FOLDER_YANG"/ext.yang", O_RDONLY);
    fstat(fd, &s);
    orig = malloc(s.st_size + 1);
    assert_ptr_not_equal(orig, NULL);
    assert_int_equal(read(fd, orig, s.st_size), s.st_size);
    orig[s.st_size] = '\0';

    assert_string_equal(printed, orig);
    free(printed);
    free(orig);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_fullset_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_fullset_yang, setup_ctx_yang, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
