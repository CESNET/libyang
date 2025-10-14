/**
 * @file test_schema_comparison.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for YANG schema comparison
 *
 * Copyright (c) 2025 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE

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

#define TEST_SC_BC_DIR TESTS_SRC "/utests/schema_comparison/bc"
#define TEST_SC_NBC_DIR TESTS_SRC "/utests/schema_comparison/nbc"

#define TEST_SC_OLD_REV "2000-01-01"
#define TEST_SC_NEW_REV "2000-01-02"

struct sc_state {
    struct ly_ctx *ctx1;
    struct ly_ctx *ctx2;
    struct lyd_node *sc_data;
    char *str;
    char *exp;
    FILE *f;
};

static int
setup(void **state)
{
    struct sc_state *st;

    st = calloc(1, sizeof *st);
    *state = st;

    /* create contexts */
    if (ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIR_CWD, &st->ctx1)) {
        return 1;
    }
    if (ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIR_CWD, &st->ctx2)) {
        return 1;
    }

    /* load ietf-schema-comparison into both contexts, the module is imported */
    if (lys_parse_path(st->ctx1, TESTS_SRC "/../modules/ietf-yang-schema-comparison@2025-10-13.yang", LYS_IN_YANG, NULL)) {
        return 1;
    }
    if (lys_parse_path(st->ctx2, TESTS_SRC "/../modules/ietf-yang-schema-comparison@2025-10-13.yang", LYS_IN_YANG, NULL)) {
        return 1;
    }

    return 0;
}

static int
teardown(void **state)
{
    struct sc_state *st = *state;

    if (!st) {
        return 0;
    }

    /* test cleanup */
    lyd_free_tree(st->sc_data);
    free(st->str);
    free(st->exp);
    if (st->f) {
        fclose(st->f);
    }

    /* destroy contexts */
    ly_ctx_destroy(st->ctx1);
    ly_ctx_destroy(st->ctx2);

    free(st);

    return 0;
}

static void
schema_comparison(struct sc_state *st, const char *module_name)
{
    struct lys_module *src_mod, *trg_mod;
    char *path;
    size_t size;

    print_message("[          ] - %s\n", module_name);

    /* load modules */
    src_mod = ly_ctx_load_module(st->ctx1, module_name, TEST_SC_OLD_REV, NULL);
    assert_non_null(src_mod);
    trg_mod = ly_ctx_load_module(st->ctx2, module_name, TEST_SC_NEW_REV, NULL);
    assert_non_null(trg_mod);

    /* get and print the comparison data */
    assert_int_equal(LY_SUCCESS, lysc_compare(st->ctx1, src_mod, trg_mod, &st->sc_data));
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&st->str, st->sc_data, LYD_JSON, 0));

    /* open file with the expected output */
    path = strdup(src_mod->filepath);
    assert_non_null(path);
    sprintf(strrchr(path, '@'), "_cmp.json");
    st->f = fopen(path, "r");
    free(path);
    assert_non_null(st->f);

    /* load expected output from file */
    fseek(st->f, 0, SEEK_END);
    size = ftell(st->f);
    fseek(st->f, 0, SEEK_SET);
    st->exp = malloc(size + 1);
    assert_non_null(st->exp);
    assert_int_equal(size, fread(st->exp, 1, size, st->f));
    st->exp[size] = '\0';

    /* compare the output */
    assert_string_equal(st->str, st->exp);

    /* cleanup */
    lyd_free_tree(st->sc_data);
    st->sc_data = NULL;
    free(st->str);
    st->str = NULL;
    free(st->exp);
    st->exp = NULL;
    fclose(st->f);
    st->f = NULL;
}

static void
test_backwards_compatible(void **state)
{
    struct sc_state *st = *state;

    /* set up contexts */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(st->ctx1, TEST_SC_BC_DIR));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(st->ctx2, TEST_SC_BC_DIR));

    /* test all backward-compatible modules */
    schema_comparison(st, "enumeration");
    schema_comparison(st, "bits");
    schema_comparison(st, "length");
    schema_comparison(st, "pattern");
    schema_comparison(st, "range");
    schema_comparison(st, "dflt");
    schema_comparison(st, "units");
    schema_comparison(st, "reference");
    schema_comparison(st, "must");
    schema_comparison(st, "when");
    schema_comparison(st, "mandatory");
    schema_comparison(st, "min-elements");
    schema_comparison(st, "max-elements");
    schema_comparison(st, "description");
    schema_comparison(st, "base");
    schema_comparison(st, "new-stmt");
    schema_comparison(st, "new-data-def");
    schema_comparison(st, "case");
    schema_comparison(st, "config");
    schema_comparison(st, "status");
    schema_comparison(st, "type");
    schema_comparison(st, "uses");
    schema_comparison(st, "submod1");
    schema_comparison(st, "prefix");
}

static void
test_non_backwards_compatible(void **state)
{
    struct sc_state *st = *state;

    /* set up contexts */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(st->ctx1, TEST_SC_NBC_DIR));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(st->ctx2, TEST_SC_NBC_DIR));

    /* test all non-backward-compatible modules */
    schema_comparison(st, "enumeration");
    schema_comparison(st, "bits");
    schema_comparison(st, "length");
    schema_comparison(st, "pattern");
    schema_comparison(st, "range");
    schema_comparison(st, "dflt");
    schema_comparison(st, "units");
    schema_comparison(st, "reference");
    schema_comparison(st, "must");
    schema_comparison(st, "when");
    schema_comparison(st, "mandatory");
    schema_comparison(st, "min-elements");
    schema_comparison(st, "max-elements");
    schema_comparison(st, "description");
    schema_comparison(st, "base");
    schema_comparison(st, "new-data-def");
    schema_comparison(st, "config");
    schema_comparison(st, "status");
    schema_comparison(st, "data-def-order");
    schema_comparison(st, "presence");
    schema_comparison(st, "union");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_backwards_compatible),
        cmocka_unit_test(test_non_backwards_compatible),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
