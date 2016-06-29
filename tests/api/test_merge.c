/**
 * @file test_merge.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for lyd_merge().
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../config.h"
#include "../../src/libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *node, *root;
    char *path, *output;
};

static int
setup_f(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error.\n");
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
teardown_f(void **state)
{
    struct state *st = (*state);

    free(st->path);
    free(st->output);
    lyd_free_withsiblings(st->root);
    lyd_free_withsiblings(st->node);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_merge(void **state)
{
    struct state *st = (*state);
    uint32_t i;
    const char *output_template =
"<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
  "<module>"
    "<name>yang</name>"
    "<revision>2016-02-11</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:1</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-yang-library</name>"
    "<revision>2016-02-01</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-netconf-acm</name>"
    "<revision>2012-02-22</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-netconf</name>"
    "<revision>2011-06-01</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>"
    "<feature>writable-running</feature>"
    "<feature>candidate</feature>"
    "<feature>rollback-on-error</feature>"
    "<feature>validate</feature>"
    "<feature>startup</feature>"
    "<feature>xpath</feature>"
  "</module>"
  "<module>"
    "<name>ietf-netconf-monitoring</name>"
    "<revision>2010-10-04</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-netconf-with-defaults</name>"
    "<revision>2011-06-01</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>"
  "</module>"
"</modules-state>";

    st->root = lyd_parse_path(st->ctx, TESTS_DIR "/api/files/merge_start.xml", LYD_XML, LYD_OPT_GET);
    assert_ptr_not_equal(st->root, NULL);

    asprintf(&st->path, TESTS_DIR "/api/files/mergeXX.xml");
    for (i = 1; i < 12; ++i) {
        sprintf(st->path + (strlen(st->path) - 6), "%02u.xml", i);
        st->node = lyd_parse_path(st->ctx, st->path, LYD_XML, LYD_OPT_GET);
        assert_ptr_not_equal(st->node, NULL);

        assert_int_equal(lyd_merge(st->root, st->node, LYD_OPT_DESTRUCT), 0);
        st->node = NULL;
    }

    lyd_print_mem(&st->output, st->root, LYD_XML, 0);
    assert_string_equal(st->output, output_template);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_merge, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
