/**
 * @file test_leafref_remove.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmoka tests for correct removing leafref nodes from data tree.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../config.h"
#include "../../src/libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *data;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/leafrefs.yin";
    const char *datafile = TESTS_DIR"/data/files/leafrefs.xml";

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

    /* schema */
    if (!lys_parse_path(st->ctx, schemafile, LYS_IN_YIN)) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
        return -1;
    }

    /* data */
    st->data = lyd_parse_path(st->ctx, datafile, LYD_XML, 0);
    if (!st->data) {
        fprintf(stderr, "Failed to load initial data file.\n");
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free(st->data);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_leafref_free(void **state)
{
    struct state *st = (*state);
    int r;

    lyd_free(st->data->child->child->prev);
    r = lyd_validate(st->data, 0);
    assert_int_not_equal(r, 0);

    lyd_new_leaf(st->data->child, NULL, "name", "jedna");
    r = lyd_validate(st->data, 0);
    assert_int_equal(r, 0);
}

static void
test_leafref_unlink(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    int r;

    node = st->data->child->child->next;
    lyd_unlink(node);
    r = lyd_validate(st->data, 0);
    assert_int_not_equal(r, 0);

    lyd_insert(st->data->child, node);
    r = lyd_validate(st->data, 0);
    assert_int_equal(r, 0);
}

static void
test_leafref_unlink2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    int r;

    node = st->data->child;
    lyd_unlink(node);
    r = lyd_validate(st->data, 0);
    assert_int_not_equal(r, 0);

    lyd_insert(st->data, node);
    r = lyd_validate(st->data, 0);
    assert_int_equal(r, 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_leafref_free, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_leafref_unlink, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_leafref_unlink2, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
