/**
 * @file test_data_initialization.c
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Cmocka data test initialization.
 *
 * Copyright (c) 2015 Sartura d.o.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "../config.h"
#include "../../src/libyang.h"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

int
generic_init(char *config_file, char *yang_file, char *yang_folder)
{
    LYS_INFORMAT yang_format;
    LYD_FORMAT in_format;
    char *schema = NULL;
    char *config = NULL;
    struct stat sb_schema, sb_config;
    int fd = -1;

    if (!config_file || !yang_file || !yang_folder) {
        goto error;
    }

    yang_format = LYS_IN_YIN;
    in_format = LYD_XML;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    fd = open(yang_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_schema) == -1 || !S_ISREG(sb_schema.st_mode)) {
        goto error;
    }

    schema = mmap(NULL, sb_schema.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    fd = open(config_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_config) == -1 || !S_ISREG(sb_config.st_mode)) {
        goto error;
    }

    config = mmap(NULL, sb_config.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    fd = -1;

    if (!lys_parse_mem(ctx, schema, yang_format)) {
        goto error;
    }

    root = lyd_parse_mem(ctx, config, in_format, LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    /* cleanup */
    munmap(config, sb_config.st_size);
    munmap(schema, sb_schema.st_size);

    return 0;

error:
    if (schema) {
        munmap(schema, sb_schema.st_size);
    }
    if (config) {
        munmap(config, sb_config.st_size);
    }
    if (fd != -1) {
        close(fd);
    }

    return -1;
}

static int
setup_f(void **state)
{
    (void) state; /* unused */
    char *config_file = TESTS_DIR"/api/files/a.xml";
    char *yang_file = TESTS_DIR"/api/files/a.yin";
    char *yang_folder = TESTS_DIR"/api/files";
    int rc;

    rc = generic_init(config_file, yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_ly_ctx_new(void **state)
{
    char *yang_folder = TESTS_DIR"/data/files";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_new_invalid(void **state)
{
    char *yang_folder = "INVALID_PATH";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (ctx) {
        fail();
    }
}

static void
test_ly_ctx_get_searchdir(void **state)
{
    const char *result;
    char *yang_folder = TESTS_DIR"/data/files";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    result = ly_ctx_get_searchdir(ctx);
    if (!result) {
        fail();
    }
    assert_string_equal(yang_folder, result);

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_set_searchdir(void **state)
{
    const char *result;
    char *yang_folder = TESTS_DIR"/data/files";
    char *new_yang_folder = TESTS_DIR"/schema/yin";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    ly_ctx_set_searchdir(ctx, new_yang_folder);
    result = ly_ctx_get_searchdir(ctx);
    if (!result) {
        fail();
    }

    assert_string_equal(new_yang_folder, result);

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_set_searchdir_invalid(void **state)
{
    const char *result;
    char *yang_folder = TESTS_DIR"/data/files";
    char *new_yang_folder = "INVALID_PATH";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    ly_ctx_set_searchdir(ctx, new_yang_folder);
    result = ly_ctx_get_searchdir(ctx);
    if (!result) {
        fail();
    }

    assert_string_equal(yang_folder, result);

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_info(void **state)
{
    struct lyd_node *node;
    char *yang_folder = TESTS_DIR"/data/files";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    node = ly_ctx_info(ctx);

    assert_int_equal(LYD_VAL_OK, node->validity);

    lyd_free(node);
}

static void
test_ly_ctx_get_module_name(void **state)
{
    (void) state; /* unused */
    const char **result;

    result = ly_ctx_get_module_names(ctx);
    if (!result) {
        fail();
    }

    assert_string_equal("yang", *result);

    free(result);
}

static void
test_ly_ctx_get_submodule_name(void **state)
{
    (void) state; /* unused */
    const char **result;
    const char *module_name = "a";

    result = ly_ctx_get_submodule_names(ctx, module_name);

    assert_string_equal("asub", *result);

    free(result);
}

static void
test_ly_ctx_get_module(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    const char *name = "a";
    const char *revision = NULL;

    module = ly_ctx_get_module(ctx, name, revision);
    if (!module) {
        fail();
    }

    assert_string_equal("a", module->name);
}

static void
test_ly_ctx_get_module_by_ns(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    const char *ns = "urn:a";
    const char *revision = NULL;

    module = ly_ctx_get_module_by_ns(ctx, ns, revision);
    if (!module) {
        fail();
    }

    assert_string_equal("a", module->name);
}

static void
test_ly_ctx_get_submodule(void **state)
{
    (void) state; /* unused */
    const struct lys_submodule *submodule;
    const char *mod_name = "a";
    const char *sub_name = "asub";
    const char *revision = NULL;

    submodule = ly_ctx_get_submodule(ctx, mod_name, revision, sub_name);
    if (!submodule) {
        fail();
    }

    assert_string_equal("asub", submodule->name);
}

static void
test_ly_set_new(void **state)
{
    (void) state; /* unused */
    struct ly_set *set;

    set = ly_set_new();
    if (!set) {
        fail();
    }

    free(set);
}

static void
test_ly_set_free(void **state)
{
    (void) state; /* unused */
    struct ly_set *set;

    set = ly_set_new();
    if (!set) {
        fail();
    }

    ly_set_free(set);

    if (!set) {
        fail();
    }
}

static void
test_ly_errno_location(void **state)
{
    (void) state; /* unused */
    char *yang_folder = "INVALID_PATH";

    LY_ERR *error;

    error = ly_errno_location();

    assert_int_equal(LY_SUCCESS, *error);

    ctx = ly_ctx_new(yang_folder);
    if (ctx) {
        fail();
    }

    error = ly_errno_location();

    assert_int_equal(LY_ESYS, *error);
}

static void
test_ly_errmsg(void **state)
{
    (void) state; /* unused */
    const char *msg;
    char *yang_folder = "INVALID_PATH";
    char *compare = "Unable to use search directory \"INVALID_PATH\" (No such file or directory)";

    ctx = ly_ctx_new(yang_folder);
    if (ctx) {
        fail();
    }

    msg = ly_errmsg();

    assert_string_equal(compare, msg);
}

static void
test_ly_errpath(void **state)
{
    (void) state; /* unused */
    const char *path;
    char *compare = "";

    path = ly_errpath();

    assert_string_equal(compare, path);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test(test_ly_ctx_new),
                    cmocka_unit_test(test_ly_ctx_new_invalid),
                    cmocka_unit_test(test_ly_ctx_get_searchdir),
		    cmocka_unit_test(test_ly_ctx_set_searchdir),
		    cmocka_unit_test(test_ly_ctx_set_searchdir_invalid),
                    cmocka_unit_test_teardown(test_ly_ctx_info, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_ctx_get_module_name, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_ctx_get_submodule_name, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_ctx_get_module, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_ctx_get_module_by_ns, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_ctx_get_submodule, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_set_new, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_set_free, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_ly_errno_location, setup_f, teardown_f),
                    cmocka_unit_test(test_ly_errmsg),
                    cmocka_unit_test_setup_teardown(test_ly_errpath, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
