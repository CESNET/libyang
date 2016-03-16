/**
 * @file test_autodel.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for auto-delete of choice's case when a node from another case comes.
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

#include "../config.h"
#include "../../src/libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt;
    char *xml;
};
static int
setup_f(void **state)
{
    struct state *st;
    const struct lys_module *mod;
    const char *schemafile = TESTS_DIR"/data/files/autodel.yin";

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

    /* schema */
    mod = lys_parse_path(st->ctx, schemafile, LYS_IN_YIN);
    if (!mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
        goto error;
    }

    /* root */
    st->dt = lyd_new(NULL, mod, "c");
    if (!st->dt) {
        fprintf(stderr, "Failed to create data root.\n");
        goto error;
    }

    return 0;

error:
    lyd_free(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_atob(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;

    node = lyd_new_leaf(st->dt, NULL, "a", "x");
    assert_ptr_not_equal(node, NULL);
    assert_ptr_equal(st->dt->child, node);
    assert_int_equal(lyd_validate(&(st->dt), 0), 0);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, "<c xmlns=\"urn:autodel\"><a>x</a></c>");

    node = lyd_new(st->dt, NULL, "b1");
    assert_ptr_not_equal(node, NULL);
    assert_int_equal(lyd_validate(&(st->dt), 0), 0);

    free(st->xml);
    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, "<c xmlns=\"urn:autodel\"><b1/></c>");
}

static void
test_btoa(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;

    node = lyd_new(st->dt, NULL, "b1");
    assert_ptr_not_equal(node, NULL);
    node = lyd_new_leaf(st->dt, NULL, "b2", "z");
    assert_ptr_not_equal(node, NULL);
    assert_int_equal(lyd_validate(&(st->dt), 0), 0);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, "<c xmlns=\"urn:autodel\"><b1/><b2>z</b2></c>");

    node = lyd_new_leaf(st->dt, NULL, "a", "x");
    assert_ptr_not_equal(node, NULL);
    assert_int_equal(lyd_validate(&(st->dt), 0), 0);

    free(st->xml);
    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, "<c xmlns=\"urn:autodel\"><a>x</a></c>");
}

static void
test_insert_fail(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    struct lyd_node *root;

    node = lyd_new_leaf(st->dt, NULL, "a", "x");
    assert_ptr_not_equal(node, NULL);
    assert_ptr_equal(st->dt->child, node);

    root = lyd_new(NULL, st->dt->schema->module, "c");
    assert_ptr_not_equal(root, NULL);
    node = lyd_new(root, NULL, "b1");
    assert_ptr_not_equal(node, NULL);
    assert_int_equal(lyd_validate(&root, 0), 0);

    assert_int_not_equal(lyd_insert_after(st->dt->child, node), 0);
    assert_string_equal(ly_errmsg(), "Insert request refers node (/autodel:c/a) that is going to be auto-deleted.");

    lyd_free(root);
}

static void
test_insert_correct(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node, *node2;
    struct lyd_node *root;

    node = lyd_new_leaf(st->dt, NULL, "a", "x");
    assert_ptr_not_equal(node, NULL);
    assert_ptr_equal(st->dt->child, node);
    node2 = lyd_new_leaf(st->dt, NULL, "x", "node");
    assert_ptr_not_equal(node, NULL);

    root = lyd_new(NULL, st->dt->schema->module, "c");
    assert_ptr_not_equal(root, NULL);
    node = lyd_new(root, NULL, "b1");
    assert_ptr_not_equal(node, NULL);
    assert_int_equal(lyd_validate(&root, 0), 0);

    assert_int_equal(lyd_insert_after(node2, node), 0);
    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, "<c xmlns=\"urn:autodel\"><x>node</x><b1/></c>");
    assert_int_equal(lyd_validate(&(st->dt), 0), 0);

    lyd_free(root);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_atob, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_btoa, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_insert_fail, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_insert_correct, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
