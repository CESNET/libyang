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

    lyd_free_withsiblings(st->dt);
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
                      "</nacm><df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_status(void **state)
{
    struct state *st = (*state);
    const char *xml_min = "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
                          "<module-set-id>5</module-set-id>"
                          "</modules-state><nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">"
                            "<denied-operations>0</denied-operations>"
                            "<denied-data-writes>0</denied-data-writes>"
                            "<denied-notifications>0</denied-notifications>"
                          "</nacm>";

    const char *xml = "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
                        "<module-set-id>5</module-set-id>"
                      "</modules-state><nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">"
                        "<denied-operations>0</denied-operations>"
                        "<denied-data-writes>0</denied-data-writes>"
                        "<denied-notifications>0</denied-notifications>"
                      "</nacm><df xmlns=\"urn:libyang:tests:defaults\">"
                        "<b1_status>42</b1_status>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<papa>42</papa></hidden>";

    assert_ptr_not_equal((st->dt = lyd_parse_mem(st->ctx, xml_min, LYD_XML, LYD_OPT_DATA | LYD_WD_EXPLICIT)), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_DATA, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_empty_tag(void **state)
{
    struct state *st = (*state);
    const char *xml = "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<enable-nacm ncwd:default=\"true\">true</enable-nacm>"
                        "<read-default ncwd:default=\"true\">permit</read-default>"
                        "<write-default ncwd:default=\"true\">deny</write-default>"
                        "<exec-default ncwd:default=\"true\">permit</exec-default>"
                        "<enable-external-groups ncwd:default=\"true\">true</enable-external-groups>"
                      "</nacm><df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><b1_1 ncwd:default=\"true\">42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";

    lyd_wd_add(st->ctx, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL_TAG);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_df1(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<bar><ho>1</ho><hi>42</hi></bar>"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);
    /* presence container */
    assert_ptr_not_equal((node = lyd_new(st->dt, NULL, "bar")), NULL);
    assert_int_not_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);
    assert_string_equal(ly_errmsg(), "Missing required element \"ho\" in \"bar\".");

    /* manadatory node in bar */
    assert_ptr_not_equal(lyd_new_leaf(node, NULL, "ho", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);

    assert_int_equal(lyd_wd_add(NULL, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_df2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<list><name>a</name><value>42</value></list>"
                        "<list><name>b</name><value>1</value></list>"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);
    /* lists */
    assert_ptr_not_equal(lyd_new_path(st->dt, NULL, "/defaults:df/defaults:list[name='a']", NULL, 0), NULL);
    assert_ptr_not_equal((node = lyd_new_path(st->dt, NULL, "/defaults:df/defaults:list[name='b']", NULL, 0)), NULL);
    assert_ptr_not_equal(lyd_new_leaf(node, NULL, "value", "1"), NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);
    assert_int_equal(lyd_wd_add(NULL, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_df3(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<a1>1</a1>"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<c><x ncwd:default=\"true\">42</x></c>"
                        "<foo ncwd:default=\"true\">42</foo><b1_1 ncwd:default=\"true\">42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);

    /* select - c */
    assert_ptr_not_equal((node = lyd_new(st->dt, NULL, "c")), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);
    assert_int_equal(lyd_wd_add(NULL, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL_TAG), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml2);

    assert_int_equal(lyd_wd_cleanup(&(st->dt), 0), 0);
    free(st->xml);
    st->xml = NULL;

    /* select - a */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "a1", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);
    assert_int_equal(lyd_wd_add(NULL, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml1);
}

static void
test_df4(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<b1_2>x</b1_2><foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<b2>1</b2><foo ncwd:default=\"true\">42</foo>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<s2a>1</s2a><foo ncwd:default=\"true\">42</foo>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);

    /* select2 - s2a */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "s2a", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);
    assert_int_equal(lyd_wd_add(NULL, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL_TAG), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml3);

    assert_int_equal(lyd_wd_cleanup(&(st->dt), 0), 0);
    free(st->xml);
    st->xml = NULL;

    /* select2 - s2b - b2 */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "b2", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);
    assert_int_equal(lyd_wd_add(NULL, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL_TAG), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml2);

    assert_int_equal(lyd_wd_cleanup(&(st->dt), 0), 0);
    free(st->xml);
    st->xml = NULL;

    /* select2 - s2b - b1 */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "b1_2", "x"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG), 0);
    assert_int_equal(lyd_wd_add(NULL, &(st->dt), LYD_OPT_CONFIG | LYD_WD_ALL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml1);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_empty, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_empty_tag, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_status, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df3, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df4, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
