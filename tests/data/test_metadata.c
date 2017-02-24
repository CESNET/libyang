/**
 * @file test_metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for ietf-yang-metadata (annotations extension).
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
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
    struct lyd_node *data;
    char *str;
};

static int
setup_f(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR"/schema/yang/ietf/");
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

    free(st->str);
    lyd_free_withsiblings(st->data);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

/*
 * leafref is not supported for annotations in libyang
 */
static void
test_leafref_type(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  import ietf-yang-metadata { prefix md; }"
                    "  md:annotation x { type leafref { path \"/x:a\"; } }"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_errno, LY_EEXT);
}

/*
 * attribute with no appropriate anotation specification cannot be loaded
 */
static void
test_unknown_metadata(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:x=\"urn:x\" x:attribute=\"not-defined\">a</a>";

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    /* parse input with strict - error */
    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INATTR);

    /* parse input without strict - passes, but the attribute is not present */
    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_CONFIG, NULL);
    assert_ptr_not_equal(st->data, NULL);
    assert_ptr_equal(st->data->attr, NULL);
}

/*
 * correctness of parsing and printing NETCONF's filter's attributes
 */
static void
test_nc_filter_subtree(void **state)
{
    struct state *st = (*state);
    const char *filter_subtree = "<get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
                    "<filter type=\"subtree\"><modules-state "
                    "xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\"><module-set-id/>"
                    "</modules-state></filter></get>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    st->data = lyd_parse_mem(st->ctx, filter_subtree, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, filter_subtree);
}

static void
test_nc_filter_xpath(void **state)
{
    struct state *st = (*state);
    const char *filter_xpath = "<get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
                    "<filter type=\"xpath\" "
                    "xmlns:yanglib=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\" "
                    "select=\"/yanglib:modules-state/yanglib:module-set-id\"/></get>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    st->data = lyd_parse_mem(st->ctx, filter_xpath, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, filter_xpath);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - invalid operation's value
 */
static void
test_nc_editconfig1(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                    "nc:operation=\"not-defined\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    /* operation attribute is valid, but its value is invalid so the parsing fails no matter if strict is used */
    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INVAL);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - too many operation attributes
 */
static void
test_nc_editconfig2(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                    "nc:operation=\"merge\" nc:operation=\"replace\" nc:operation=\"create\" "
                    "nc:operation=\"delete\" nc:operation=\"remove\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_TOOMANY);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - correct use
 */
static void
test_nc_editconfig3(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                    "nc:operation=\"create\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_leafref_type, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_unknown_metadata, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter_subtree, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter_xpath, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig3, setup_f, teardown_f),};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
