/**
 * @file test_values.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for correct resolving of types and values of the data nodes.
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
    char *data;
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

    lyd_free_withsiblings(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->data);
    free(st);
    (*state) = NULL;

    return 0;
}

/*
 * default values of integer types are allowed to be specified in decimal, octal and hexadecimal forms
 */
static void
test_default_int(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type int8; default 10; }"  // decimal (10)
                    "  leaf b { type int8; default 012; }" // octal (10)
                    "  leaf c { type int8; default 0xa; }" // hexadecimal (10)
                    "}";
    const char *xml1 = "<a xmlns=\"urn:x\">012</a>"; // value is supposed to be 12, not 10 as in case of octal value
    const char *xml2 = "<a xmlns=\"urn:x\">0xa</a>"; // error
    const struct lys_module *mod;

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);
    assert_string_equal(mod->data->name, "a");
    assert_string_equal(((struct lys_node_leaf *)mod->data)->dflt, "10");
    assert_string_equal(mod->data->next->name, "b");
    assert_string_equal(((struct lys_node_leaf *)mod->data->next)->dflt, "10");
    assert_string_equal(mod->data->prev->name, "c");
    assert_string_equal(((struct lys_node_leaf *)mod->data->prev)->dflt, "10");

    /* in contrast, octal and hexadecimal values are not allowed in data */
    st->dt = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode, LYVE_INVAL);
    assert_string_equal(ly_errmsg(), "Invalid value \"0xa\" in \"a\" element.");

    st->dt = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "12");
}

/*
 * identityref and instance-identifiers values in XML are supposed to be converted into a JSON form where the prefixes
 * are the names of the modules, not a generic prefixes as in XML
 */
static void
test_xmltojson_identityref(void **state)
{
    struct state *st = (*state);
    const char *yang1 = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  identity vehicle;"
                    "  identity car { base vehicle; }"
                    "}";
    const char *yang2 = "module y {"
                    "  namespace urn:y;"
                    "  prefix y;"
                    "  import x { prefix xpref; }"
                    "  leaf y1 { type identityref { base xpref:vehicle; } }"
                    "  leaf y2 { type leafref { path \"../y1\"; } }"
                    "}";
    const char *xml = "<y1 xmlns=\"urn:y\" xmlns:z=\"urn:x\">z:car</y1>"
                    "<y2 xmlns=\"urn:y\" xmlns:z=\"urn:x\">z:car</y2>";
    const char *result = "<y1 xmlns=\"urn:y\" xmlns:x=\"urn:x\">x:car</y1>"
                    "<y2 xmlns=\"urn:y\" xmlns:x=\"urn:x\">x:car</y2>";

    assert_ptr_not_equal(lys_parse_mem(st->ctx, yang1, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(st->ctx, yang2, LYS_IN_YANG), NULL);

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&st->data, st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_ptr_not_equal(st->data, NULL);
    assert_string_equal(st->data, result);
}

static void
test_xmltojson_identityref2(void **state)
{
    struct state *st = (*state);
    const char *yang = "module y {"
                    "  namespace urn:y;"
                    "  prefix y;"
                    "  identity vehicle;"
                    "  identity car { base vehicle; }"
                    "  leaf y { type identityref { base y:vehicle; } default y:car; }"
                    "}";
    const char *result = "<y xmlns=\"urn:y\">car</y>";

    assert_ptr_not_equal(lys_parse_mem(st->ctx, yang, LYS_IN_YANG), NULL);

    st->dt = NULL;
    lyd_validate(&st->dt, LYD_OPT_CONFIG, st->ctx);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&st->data, st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL);
    assert_ptr_not_equal(st->data, NULL);
    assert_string_equal(st->data, result);
}

static void
test_xmltojson_instanceid(void **state)
{
    struct state *st = (*state);
    const char *yang1 = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf x { type string; }"
                    "}";
    const char *yang2 = "module y {"
                    "  namespace urn:y;"
                    "  prefix y;"
                    "  leaf y1 { type instance-identifier; }"
                    "  leaf y2 { type leafref { path \"../y1\"; } }"
                    "}";
    const char *xml = "<x xmlns=\"urn:x\">test</x>"
                    "<y1 xmlns=\"urn:y\" xmlns:z=\"urn:x\">/z:x</y1>"
                    "<y2 xmlns=\"urn:y\" xmlns:z=\"urn:x\">/z:x</y2>";
    const char *result =  "<x xmlns=\"urn:x\">test</x>"
                    "<y1 xmlns=\"urn:y\" xmlns:x=\"urn:x\">/x:x</y1>"
                    "<y2 xmlns=\"urn:y\" xmlns:x=\"urn:x\">/x:x</y2>";

    assert_ptr_not_equal(lys_parse_mem(st->ctx, yang1, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(st->ctx, yang2, LYS_IN_YANG), NULL);

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&st->data, st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_ptr_not_equal(st->data, NULL);
    assert_string_equal(st->data, result);
}

static void
test_canonical(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  container x {"
                    "    leaf-list a { type int8; }"
                    "    leaf-list b { type int16; }"
                    "    leaf-list c { type int32; }"
                    "    leaf-list d { type int64; }"
                    "    leaf-list e { type uint8; }"
                    "    leaf-list f { type uint16; }"
                    "    leaf-list g { type uint32; }"
                    "    leaf-list h { type uint64; }"
                    "    leaf-list i { type decimal64 { fraction-digits 2; } }"
                    "    leaf-list j { type bits {"
                    "        bit one { position 1; }"
                    "        bit three { position 3; }"
                    "        bit two { position 2; }"
                    "    }  }"
                    "    leaf-list alr { type leafref { path ../a; } }"
                    "    leaf-list blr { type leafref { path ../b; } }"
                    "    leaf-list clr { type leafref { path ../c; } }"
                    "    leaf-list dlr { type leafref { path ../d; } }"
                    "    leaf-list elr { type leafref { path ../e; } }"
                    "    leaf-list flr { type leafref { path ../f; } }"
                    "    leaf-list glr { type leafref { path ../g; } }"
                    "    leaf-list hlr { type leafref { path ../h; } }"
                    "    leaf-list ilr { type leafref { path ../i; } }"
                    "    leaf-list jlr { type leafref { path ../j; } }"
                    "} }";
    const char *input = "<x xmlns=\"urn:x\">"
                    "<a>+1</a><a>+0</a>"
                    "<b>+300</b><b>+0</b>"
                    "<c>+66000</c><c>+0</c>"
                    "<d>+4300000000</d><d>+0</d>"
                    "<e>+1</e><e>-0</e>"
                    "<f>+300</f><f>-0</f>"
                    "<g>+66000</g><g>-0</g>"
                    "<h>+4300000000</h><h>-0</h>"
                    "<i>1</i><i>+2</i><i>0</i><i>3.000000</i><i>0000004</i><i>4.1</i>"
                    "<j> three      two</j><j>one two</j><j> two   three one</j>"
                    "<alr>+1</alr><alr>+0</alr>"
                    "<blr>+300</blr><blr>+0</blr>"
                    "<clr>+66000</clr><clr>+0</clr>"
                    "<dlr>+4300000000</dlr><dlr>+0</dlr>"
                    "<elr>+1</elr><elr>-0</elr>"
                    "<flr>+300</flr><flr>-0</flr>"
                    "<glr>+66000</glr><glr>-0</glr>"
                    "<hlr>+4300000000</hlr><hlr>-0</hlr>"
                    "<ilr>1</ilr><ilr>+2</ilr><ilr>0</ilr><ilr>3.000000</ilr><ilr>0000004</ilr><ilr>4.1</ilr>"
                    "<jlr> three      two</jlr><jlr>one two</jlr><jlr> two   three one</jlr>"
                    "</x>";
    const char *result = "<x xmlns=\"urn:x\">"
                    "<a>1</a><a>0</a>"
                    "<b>300</b><b>0</b>"
                    "<c>66000</c><c>0</c>"
                    "<d>4300000000</d><d>0</d>"
                    "<e>1</e><e>0</e>"
                    "<f>300</f><f>0</f>"
                    "<g>66000</g><g>0</g>"
                    "<h>4300000000</h><h>0</h>"
                    "<i>1.0</i><i>2.0</i><i>0.0</i><i>3.0</i><i>4.0</i><i>4.1</i>"
                    "<j>two three</j><j>one two</j><j>one two three</j>"
                    "<alr>1</alr><alr>0</alr>"
                    "<blr>300</blr><blr>0</blr>"
                    "<clr>66000</clr><clr>0</clr>"
                    "<dlr>4300000000</dlr><dlr>0</dlr>"
                    "<elr>1</elr><elr>0</elr>"
                    "<flr>300</flr><flr>0</flr>"
                    "<glr>66000</glr><glr>0</glr>"
                    "<hlr>4300000000</hlr><hlr>0</hlr>"
                    "<ilr>1.0</ilr><ilr>2.0</ilr><ilr>0.0</ilr><ilr>3.0</ilr><ilr>4.0</ilr><ilr>4.1</ilr>"
                    "<jlr>two three</jlr><jlr>one two</jlr><jlr>one two three</jlr>"
                    "</x>";

    assert_ptr_not_equal(lys_parse_mem(st->ctx, yang, LYS_IN_YANG), NULL);
    st->dt = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&st->data, st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_ptr_not_equal(st->data, NULL);
    assert_string_equal(st->data, result);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_default_int, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_xmltojson_identityref, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_xmltojson_identityref2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_xmltojson_instanceid, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_canonical, setup_f, teardown_f),};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
