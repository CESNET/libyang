/**
 * @file test_defaults.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for processing default values.
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
    const struct lys_module *mod;
    struct lyd_node *dt;
    char *xml;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/defaults.yin";
    const char *ncwdfile = TESTS_DIR"/schema/yin/ietf/ietf-netconf-with-defaults.yin";
    const char *ietfdir = TESTS_DIR"/schema/yin/ietf/";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(ietfdir);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schemas */
    if (!lys_parse_path(st->ctx, ncwdfile, LYS_IN_YIN)) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", ncwdfile);
        goto error;
    }

    st->mod = lys_parse_path(st->ctx, schemafile, LYS_IN_YIN);
    if (!st->mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
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

    lyd_free(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_empty(void **state)
{
    struct state *st = (*state);
    const char *xml = "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">"
                        "<enable-nacm>true</enable-nacm>"
                        "<read-default>permit</read-default>"
                        "<write-default>deny</write-default>"
                        "<exec-default>permit</exec-default>"
                        "<enable-external-groups>true</enable-external-groups>"
                        "<denied-operations>0</denied-operations>"
                        "<denied-data-writes>0</denied-data-writes>"
                        "<denied-notifications>0</denied-notifications>"
                      "</nacm><df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    lyd_wd_add(st->ctx, &(st->dt), LYD_WD_ALL);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, xml);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_empty, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
