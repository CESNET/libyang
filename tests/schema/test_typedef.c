/**
 * \file test_typedef.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - typedefs and their resolution
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
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

struct state {
    struct ly_ctx *ctx;
    int fd;
    char *str1;
    char *str2;
};

static int
setup_ctx(void **state)
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
teardown_ctx(void **state)
{
    struct state *st = (*state);

    ly_ctx_destroy(st->ctx, NULL);
    if (st->fd > 0) {
        close(st->fd);
    }
    free(st->str1);
    free(st->str2);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_typedef_yin(void **state)
{
    const char *schema = TESTS_DIR"/schema/yin/files/f.yin";
    struct state *st = (*state);
    struct stat s;
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, schema, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    st->fd = open(schema, O_RDONLY);
    fstat(st->fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(st->fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), mod, LYS_OUT_YIN, NULL);

    assert_string_equal(st->str1, st->str2);
}

static void
test_typedef_yang(void **state)
{
    const char *schema = TESTS_DIR"/schema/yang/files/f.yang";
    struct state *st = (*state);
    struct stat s;
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, schema, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->fd = open(schema, O_RDONLY);
    fstat(st->fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(st->fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), mod, LYS_OUT_YANG, NULL);

    assert_string_equal(st->str1, st->str2);
}

static void
test_typedef_11in10(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin_enums = "<module name=\"x1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x1\"/><prefix value=\"x1\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"one\"/>"
"  </type></leaf>"
"</module>";

    const char *yang_enums = "module x2 {"
"  namespace \"urn:x2\"; prefix x2;"
"  typedef e1 { type enumeration { enum one; enum two; } default one; }"
"  leaf l { type e1 { enum one; } } }";

    const char *yin_bits = "<module name=\"y1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:y1\"/><prefix value=\"y1\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"one\"/>"
"  </type></leaf>"
"</module>";

    const char *yang_bits = "module y2 {"
"  namespace \"urn:y2\"; prefix y2;"
"  typedef b1 { type bits { bit one; bit two; } default one; }"
"  leaf l { type b1 { bit one; } } }";

    mod = lys_parse_mem(st->ctx, yin_enums, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yin_bits, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yang_enums, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yang_bits, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_INSTMT);
}

static void
test_typedef_11_enums_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *enums1 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"two\"/>"
"  </type></leaf>"
"</module>";

    const char *enums2 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"one\"><value value=\"1\"/></enum>"
"  </type></leaf>"
"</module>";

    const char *enums3 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"three\"/>"
"  </type></leaf>"
"</module>";

    const char *enums4 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"one\"/>"
"  </type></leaf>"
"</module>";

    mod = lys_parse_mem(st->ctx, enums1, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    mod = lys_parse_mem(st->ctx, enums2, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_ENUM_INVAL);

    mod = lys_parse_mem(st->ctx, enums3, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_ENUM_INNAME);

    mod = lys_parse_mem(st->ctx, enums4, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);
}

static void
test_typedef_11_bits_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *bits1 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"two\"/>"
"  </type></leaf>"
"</module>";

    const char *bits2 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"one\"><position value=\"1\"/></bit>"
"  </type></leaf>"
"</module>";

    const char *bits3 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"three\"/>"
"  </type></leaf>"
"</module>";

    const char *bits4 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"one\"/>"
"  </type></leaf>"
"</module>";

    mod = lys_parse_mem(st->ctx, bits1, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    mod = lys_parse_mem(st->ctx, bits2, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_BITS_INVAL);

    mod = lys_parse_mem(st->ctx, bits3, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode, LYVE_BITS_INNAME);

    mod = lys_parse_mem(st->ctx, bits4, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);
}

static void
test_typedef_11_iff_ident_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/><feature name=\"x\"/>"
"  <identity name=\"ibase\"/>"
"  <identity name=\"one\"><base name=\"ibase\"/><if-feature name=\"x\"/></identity>"
"  <identity name=\"two\"><base name=\"ibase\"/></identity>"
"  <leaf name=\"l\"><type name=\"identityref\"><base name=\"ibase\"/></type></leaf></module>";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG), 0);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_iff_enums_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/><feature name=\"x\"/>"
"  <typedef name=\"myenum\"><type name=\"enumeration\">"
"    <enum name=\"one\"><if-feature name=\"x\"/></enum><enum name=\"two\"/>"
"  </type></typedef>"
"  <leaf name=\"l\"><type name=\"myenum\"/></leaf></module>";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG), 0);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_iff_bits_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/><feature name=\"x\"/>"
"  <typedef name=\"mybits\"><type name=\"bits\">"
"    <bit name=\"one\"><if-feature name=\"x\"/></bit><bit name=\"two\"/>"
"  </type></typedef>"
"  <leaf name=\"l\"><type name=\"mybits\"/></leaf></module>";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG), 0);
    assert_int_equal(ly_vecode, LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_pattern_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *modstr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<module name=\"x\"\n"
"        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
"        xmlns:x=\"urn:x\">\n"
"  <yang-version value=\"1.1\"/>\n"
"  <namespace uri=\"urn:x\"/>\n  <prefix value=\"x\"/>\n"
"  <leaf name=\"l\">\n    <type name=\"string\">\n"
"      <pattern value=\"[a-zA-Z_][a-zA-Z0-9\\-_.]*\"/>\n"
"      <pattern value=\"[nN][oO][tT].*\">\n        <modifier value=\"invert-match\"/>\n      </pattern>\n"
"    </type>\n  </leaf>\n</module>\n";
    char *printed;

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">enabled</l>"; /* legal */
    const char *data2 = "<l xmlns=\"urn:x\">10</l>";      /* ilegal, starts with number */
    const char *data3 = "<l xmlns=\"urn:x\">notoric</l>"; /* ilegal, starts with not */

    mod = lys_parse_mem(st->ctx, modstr, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&printed, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(printed, NULL);
    assert_string_equal(printed, modstr);
    free(printed);

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode, LYVE_NOCONSTR);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode, LYVE_NOCONSTR);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_typedef_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11in10, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_enums_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_bits_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_ident_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_enums_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_bits_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_pattern_yin, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
