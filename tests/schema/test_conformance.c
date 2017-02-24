/**
 * \file test_conformance.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - setting up modules as imported or implemented
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
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/conformance"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/conformance"

static int
setup_ctx(void **state)
{
    *state = ly_ctx_new(NULL);
    if (!*state) {
        return -1;
    }
    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy(*state, NULL);
    return 0;
}

static void
test_implemented1_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b, *b2, *c, *c2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    /* loads a.yin (impl), b@2015-01-01.yin (impl by augment) and c@2015-03-03.yin (imp) */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/a.yin", LYS_IN_YIN);
    assert_ptr_not_equal(a, NULL);
    assert_int_equal(a->implemented, 1);

    b = ly_ctx_get_module(ctx, "b", NULL);
    assert_ptr_not_equal(b, NULL);
    assert_int_equal(b->implemented, 1);

    c = ly_ctx_get_module(ctx, "c", NULL);
    assert_ptr_not_equal(c, NULL);
    assert_int_equal(c->implemented, 0);

    /* another b cannot be loaded, since it is already implemented */
    b2 = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/b@2015-04-04.yin", LYS_IN_YIN);
    assert_ptr_equal(b2, NULL);
    assert_int_equal(ly_errno, LY_EINVAL);
    assert_string_equal(ly_errmsg(), "Module \"b\" parsing failed.");

    /* older c can be loaded and it will be marked as implemented */
    c2 = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/c@2015-01-01.yin", LYS_IN_YIN);
    assert_ptr_not_equal(c2, NULL);
    assert_int_equal(c2->implemented, 1);
    assert_ptr_not_equal(c, c2);
}

static void
test_implemented1_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b, *b2, *c, *c2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    /* loads a.yang (impl), b@2015-01-01.yang (impl by augment) and c@2015-03-03.yang (imp) */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/a.yang", LYS_IN_YANG);
    assert_ptr_not_equal(a, NULL);
    assert_int_equal(a->implemented, 1);

    b = ly_ctx_get_module(ctx, "b", NULL);
    assert_ptr_not_equal(b, NULL);
    assert_int_equal(b->implemented, 1);

    c = ly_ctx_get_module(ctx, "c", NULL);
    assert_ptr_not_equal(c, NULL);
    assert_int_equal(c->implemented, 0);

    /* another b cannot be loaded, since it is already implemented */
    b2 = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/b@2015-04-04.yang", LYS_IN_YANG);
    assert_ptr_equal(b2, NULL);
    assert_int_equal(ly_errno, LY_EINVAL);
    assert_string_equal(ly_errmsg(), "Module \"b\" parsing failed.");

    /* older c can be loaded and it will be marked as implemented */
    c2 = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/c@2015-01-01.yang", LYS_IN_YANG);
    assert_ptr_not_equal(c2, NULL);
    assert_int_equal(c2->implemented, 1);
    assert_ptr_not_equal(c, c2);
}

static void
test_implemented2_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b, *b2, *c, *c2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    /* load the newest b first, it is implemented */
    b2 = ly_ctx_load_module(ctx, "b", "2015-04-04");
    assert_ptr_not_equal(b2, NULL);
    assert_int_equal(b2->implemented, 1);

    /* loads a.yin (impl), b@2015-01-01.yin (imp) and c@2015-01-01.yin (imp)
     * b@2015-04-04 is augmented by a */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/a.yin", LYS_IN_YIN);
    assert_ptr_not_equal(a, NULL);
    assert_int_equal(a->implemented, 1);

    c = ly_ctx_get_module(ctx, "c", NULL);
    assert_ptr_not_equal(c, NULL);
    assert_int_equal(c->implemented, 0);

    b = ly_ctx_get_module(ctx, "b", "2015-01-01");
    assert_ptr_not_equal(b, NULL);
    assert_int_equal(b->implemented, 0);
    assert_ptr_equal(b2->data->child->next, a->augment[0].child);

    /* we load the newest c, which is already loaded and now it is going to be marked as implemented */
    c2 = ly_ctx_load_module(ctx, "c", NULL);
    assert_ptr_not_equal(c2, NULL);
    assert_int_equal(c2->implemented, 1);
    assert_ptr_equal(c, c2);
}

static void
test_implemented2_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b, *b2, *c, *c2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    /* load the newest b first, it is implemented */
    b2 = ly_ctx_load_module(ctx, "b", "2015-04-04");
    assert_ptr_not_equal(b2, NULL);
    assert_int_equal(b2->implemented, 1);

    /* loads a.yang (impl), b@2015-01-01.yang (imp) and c@2015-01-01.yang (imp)
     * b@2015-04-04 is augmented by a */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/a.yang", LYS_IN_YANG);
    assert_ptr_not_equal(a, NULL);
    assert_int_equal(a->implemented, 1);

    c = ly_ctx_get_module(ctx, "c", NULL);
    assert_ptr_not_equal(c, NULL);
    assert_int_equal(c->implemented, 0);

    b = ly_ctx_get_module(ctx, "b", "2015-01-01");
    assert_ptr_not_equal(b, NULL);
    assert_int_equal(b->implemented, 0);
    assert_ptr_equal(b2->data->child->next, a->augment[0].child);

    /* we load the newest c, which is already loaded and now it is going to be marked as implemented */
    c2 = ly_ctx_load_module(ctx, "c", NULL);
    assert_ptr_not_equal(c2, NULL);
    assert_int_equal(c2->implemented, 1);
    assert_ptr_equal(c, c2);
}

static void
test_implemented_info_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    struct lyd_node *info;
    const struct lys_module *a;
    char *data;
    const char *template = "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
"  <module>\n"
"    <name>ietf-yang-metadata</name>\n"
"    <revision>2016-08-05</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>yang</name>\n"
"    <revision>2017-02-20</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-inet-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-library</name>\n"
"    <revision>2016-06-21</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>b</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YIN"/b@2015-01-01.yin</schema>\n"
"    <namespace>urn:example:b</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>c</name>\n"
"    <revision>2015-03-03</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YIN"/c@2015-03-03.yin</schema>\n"
"    <namespace>urn:example:c</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>a</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YIN"/a.yin</schema>\n"
"    <namespace>urn:example:a</namespace>\n"
"    <feature>foo</feature>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module-set-id>9</module-set-id>\n"
"</modules-state>\n";

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    /* loads a.yin (impl), b@2015-01-01.yin (impl by augment) and c@2015-03-03.yin (imp) */
    assert_ptr_not_equal((a = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/a.yin", LYS_IN_YIN)), NULL);
    assert_int_equal(lys_features_enable(a, "foo"), 0);

    /* get yang-library data */
    info = ly_ctx_info(ctx);
    assert_ptr_not_equal(info, NULL);

    lyd_print_mem(&data, info, LYD_XML, LYP_FORMAT);
    lyd_free_withsiblings(info);
    assert_string_equal(data, template);
    free(data);
}

static void
test_implemented_info_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    struct lyd_node *info;
    const struct lys_module *a;
    char *data;
    const char *template = "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
"  <module>\n"
"    <name>ietf-yang-metadata</name>\n"
"    <revision>2016-08-05</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>yang</name>\n"
"    <revision>2017-02-20</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-inet-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-library</name>\n"
"    <revision>2016-06-21</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>b</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YANG"/b@2015-01-01.yang</schema>\n"
"    <namespace>urn:example:b</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>c</name>\n"
"    <revision>2015-03-03</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YANG"/c@2015-03-03.yang</schema>\n"
"    <namespace>urn:example:c</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>a</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YANG"/a.yang</schema>\n"
"    <namespace>urn:example:a</namespace>\n"
"    <feature>foo</feature>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module-set-id>9</module-set-id>\n"
"</modules-state>\n";

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    /* loads a.yang (impl), b@2015-01-01.yang (impl by augment) and c@2015-03-03.yang (imp) */
    assert_ptr_not_equal((a = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/a.yang", LYS_IN_YANG)), NULL);
    assert_int_equal(lys_features_enable(a, "foo"), 0);

    /* get yang-library data */
    info = ly_ctx_info(ctx);
    assert_ptr_not_equal(info, NULL);

    lyd_print_mem(&data, info, LYD_XML, LYP_FORMAT);
    lyd_free_withsiblings(info);
    assert_string_equal(data, template);
    free(data);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_implemented1_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented1_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented2_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented2_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented_info_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented_info_yang, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
