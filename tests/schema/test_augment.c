/**
 * \file test_augment.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - augment targets
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
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/files"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"

#define MOD_COUNT 7
#define YANG_MOD_IDX(idx) (idx)
#define YIN_MOD_IDX(idx) (MOD_COUNT + idx)

struct ly_ctx *ctx;
char *yang_modules[2 * MOD_COUNT] = {0};
char *yin_modules[2 * MOD_COUNT] = {0};


static int
setup_ctx_yin(void **state)
{
    *state = malloc(strlen(TESTS_DIR) + 40);
    assert_non_null(*state);
    memcpy(*state, SCHEMA_FOLDER_YIN, strlen(SCHEMA_FOLDER_YIN) + 1);

    ctx = ly_ctx_new(NULL);
    if (!ctx) {
        return -1;
    }

    return 0;
}

static int
setup_ctx_yang(void **state)
{
    *state = malloc(strlen(TESTS_DIR) + 40);
    assert_non_null(*state);
    memcpy(*state, SCHEMA_FOLDER_YANG, strlen(SCHEMA_FOLDER_YANG) + 1);

    ctx = ly_ctx_new(NULL);
    if (!ctx) {
        return -1;
    }

    return 0;
}

static int
teardown_ctx(void **state)
{
    free(*state);
    ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_target_include_submodule(void **state)
{
    int length;
    char *path = *state;
    const struct lys_module *module;

    ly_ctx_set_searchdir(ctx, path);
    length = strlen(path);
    if (!strcmp(path, SCHEMA_FOLDER_YIN)) {
        strcpy(path + length, "/a.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        lys_print_mem(&yin_modules[YANG_MOD_IDX(0)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[YIN_MOD_IDX(0)], module, LYS_OUT_YIN, NULL);
    } else {
        strcpy(path + length, "/a.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        lys_print_mem(&yang_modules[YANG_MOD_IDX(0)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yang_modules[YIN_MOD_IDX(0)], module, LYS_OUT_YIN, NULL);
    }
}

static void
test_leafref(void **state)
{
    int length;
    char *path = *state;
    const struct lys_module *module;

    ly_ctx_set_searchdir(ctx, path);
    length = strlen(path);
    if (!strcmp(path, SCHEMA_FOLDER_YIN)) {
        strcpy(path + length, "/b1.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        lys_print_mem(&yin_modules[YANG_MOD_IDX(1)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[YIN_MOD_IDX(1)], module, LYS_OUT_YIN, NULL);

        strcpy(path + length, "/b2.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        lys_print_mem(&yin_modules[YANG_MOD_IDX(2)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[YIN_MOD_IDX(2)], module, LYS_OUT_YIN, NULL);
    } else {
        strcpy(path + length, "/b1.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        lys_print_mem(&yang_modules[YANG_MOD_IDX(1)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yang_modules[YIN_MOD_IDX(1)], module, LYS_OUT_YIN, NULL);

        strcpy(path + length, "/b2.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        lys_print_mem(&yang_modules[YANG_MOD_IDX(2)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yang_modules[YIN_MOD_IDX(2)], module, LYS_OUT_YIN, NULL);
    }
}

static void
test_target_augment(void **state)
{
    int length;
    char *path = *state;
    const struct lys_module *module;

    ly_ctx_set_searchdir(ctx, path);
    length = strlen(path);
    if (!strcmp(path, SCHEMA_FOLDER_YIN)) {
        strcpy(path + length, "/c1.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        lys_print_mem(&yin_modules[YANG_MOD_IDX(3)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[YIN_MOD_IDX(3)], module, LYS_OUT_YIN, NULL);

        strcpy(path + length, "/c2.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        lys_print_mem(&yin_modules[YANG_MOD_IDX(4)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[YIN_MOD_IDX(4)], module, LYS_OUT_YIN, NULL);

        strcpy(path + length, "/c3.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        lys_print_mem(&yin_modules[YANG_MOD_IDX(5)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[YIN_MOD_IDX(5)], module, LYS_OUT_YIN, NULL);
    } else {
        strcpy(path + length, "/c1.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        lys_print_mem(&yang_modules[YANG_MOD_IDX(3)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yang_modules[YIN_MOD_IDX(3)], module, LYS_OUT_YIN, NULL);

        strcpy(path + length, "/c2.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        lys_print_mem(&yang_modules[YANG_MOD_IDX(4)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yang_modules[YIN_MOD_IDX(4)], module, LYS_OUT_YIN, NULL);

        strcpy(path + length, "/c3.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        lys_print_mem(&yang_modules[YANG_MOD_IDX(5)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yang_modules[YIN_MOD_IDX(5)], module, LYS_OUT_YIN, NULL);
    }
}

static void
test_unres_augment(void **state)
{
    int length;
    char *path = *state;
    const struct lys_module *module;

    ly_ctx_set_searchdir(ctx, path);
    length = strlen(path);
    if (!strcmp(path, SCHEMA_FOLDER_YIN)) {
        strcpy(path + length, "/emod.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        lys_print_mem(&yin_modules[YANG_MOD_IDX(6)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[YIN_MOD_IDX(6)], module, LYS_OUT_YIN, NULL);
    } else {
        strcpy(path + length, "/emod.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        lys_print_mem(&yang_modules[YANG_MOD_IDX(6)], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yang_modules[YIN_MOD_IDX(6)], module, LYS_OUT_YIN, NULL);
    }
}

static void
test_import_augment_target(void **state)
{
    int length;
    char *path = *state;
    const struct lys_module *module;

    ly_ctx_set_searchdir(ctx, path);
    length = strlen(path);
    if (!strcmp(path, SCHEMA_FOLDER_YIN)) {
        strcpy(path + length, "/g1.yin");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YIN))) {
            fail();
        }
        /*lys_print_mem(&yin_modules[7], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[14], module, LYS_OUT_YIN, NULL);*/
    } else {
        strcpy(path + length, "/g1.yang");
        if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
            fail();
        }
        /*lys_print_mem(&yin_modules[7], module, LYS_OUT_YANG, NULL);
        lys_print_mem(&yin_modules[14], module, LYS_OUT_YIN, NULL);*/
    }
}

static void
compare_output(void **state)
{
    int i;
    char *name[] = {"a", "b1", "b2", "c1", "c2", "c3", "emod"};

    (void) state; /* unused state*/
    for (i = 0; i < 14; ++i) {
        fprintf(stdout, "Compare \"%s\" modules print by %s ... ", name[i % 7], (i / 7) ? "yin_print" : "yang_print");
        assert_non_null(yang_modules[i]);
        assert_non_null(yin_modules[i]);
        if (strcmp(yang_modules[i], yin_modules[i])) {
            fprintf(stderr, "failed.\n");
            fail();
        }
        fprintf(stdout, "ok.\n");
    }
}

static int
teardown_output(void **state)
{
    int i;

    (void)state; /* unused state*/
    for (i = 0; i < 14; ++i) {
        free(yang_modules[i]);
        free(yin_modules[i]);
    }
    return 0;
}

int
main(void)
{
    ly_verb(LY_LLWRN);
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_target_include_submodule, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leafref, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_target_augment, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_unres_augment, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_import_augment_target, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_target_include_submodule, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leafref, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_target_augment, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_unres_augment, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_import_augment_target, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_teardown(compare_output, teardown_output),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
