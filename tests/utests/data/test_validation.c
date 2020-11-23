/**
 * @file test_validation.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from validation.c
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "context.h"
#include "in.h"
#include "out.h"
#include "parser_data.h"
#include "printer_data.h"
#include "tests/config.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "utests.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

struct ly_ctx *ctx; /* context for tests */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    if (store) {
        if (path && path[0]) {
            snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
        } else {
            strncpy(logbuf, msg, BUFSIZE - 1);
        }
        if (store > 0) {
            --store;
        }
    }
}

#endif

static int
setup(void **state)
{
    (void) state; /* unused */

    const char *schema_a =
            "module a {\n"
            "    namespace urn:tests:a;\n"
            "    prefix a;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    container cont {\n"
            "        leaf a {\n"
            "            when \"../../c = 'val_c'\";\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf c {\n"
            "        when \"/cont/b = 'val_b'\";\n"
            "        type string;\n"
            "    }\n"
            "}";
    const char *schema_b =
            "module b {\n"
            "    namespace urn:tests:b;\n"
            "    prefix b;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        mandatory true;\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    leaf c {\n"
            "        mandatory true;\n"
            "        type string;\n"
            "    }\n"
            "    leaf d {\n"
            "        type empty;\n"
            "    }\n"
            "}";
    const char *schema_c =
            "module c {\n"
            "    namespace urn:tests:c;\n"
            "    prefix c;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf-list l {\n"
            "                min-elements 3;\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    list lt {\n"
            "        max-elements 4;\n"
            "        key \"k\";\n"
            "        leaf k {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf d {\n"
            "        type empty;\n"
            "    }\n"
            "}";
    const char *schema_d =
            "module d {\n"
            "    namespace urn:tests:d;\n"
            "    prefix d;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    list lt {\n"
            "        key \"k\";\n"
            "        unique \"l1\";\n"
            "        leaf k {\n"
            "            type string;\n"
            "        }\n"
            "        leaf l1 {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    list lt2 {\n"
            "        key \"k\";\n"
            "        unique \"cont/l2 l4\";\n"
            "        unique \"l5 l6\";\n"
            "        leaf k {\n"
            "            type string;\n"
            "        }\n"
            "        container cont {\n"
            "            leaf l2 {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "        leaf l4 {\n"
            "            type string;\n"
            "        }\n"
            "        leaf l5 {\n"
            "            type string;\n"
            "        }\n"
            "        leaf l6 {\n"
            "            type string;\n"
            "        }\n"
            "        list lt3 {\n"
            "            key \"kk\";\n"
            "            unique \"l3\";\n"
            "            leaf kk {\n"
            "                type string;\n"
            "            }\n"
            "            leaf l3 {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    const char *schema_e =
            "module e {\n"
            "    namespace urn:tests:e;\n"
            "    prefix e;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf-list l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    list lt {\n"
            "        key \"k\";\n"
            "        leaf k {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf d {\n"
            "        type uint32;\n"
            "    }\n"
            "    leaf-list ll {\n"
            "        type string;\n"
            "    }\n"
            "    container cont {\n"
            "        list lt {\n"
            "            key \"k\";\n"
            "            leaf k {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "        leaf d {\n"
            "            type uint32;\n"
            "        }\n"
            "        leaf-list ll {\n"
            "            type string;\n"
            "        }\n"
            "        leaf-list ll2 {\n"
            "            type enumeration {\n"
            "                enum one;\n"
            "                enum two;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    const char *schema_f =
            "module f {\n"
            "    namespace urn:tests:f;\n"
            "    prefix f;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        default \"c\";\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "        case c {\n"
            "            leaf-list ll1 {\n"
            "                type string;\n"
            "                default \"def1\";\n"
            "                default \"def2\";\n"
            "                default \"def3\";\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    leaf d {\n"
            "        type uint32;\n"
            "        default 15;\n"
            "    }\n"
            "    leaf-list ll2 {\n"
            "        type string;\n"
            "        default \"dflt1\";\n"
            "        default \"dflt2\";\n"
            "    }\n"
            "    container cont {\n"
            "        choice choic {\n"
            "            default \"c\";\n"
            "            leaf a {\n"
            "                type string;\n"
            "            }\n"
            "            case b {\n"
            "                leaf l {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "            case c {\n"
            "                leaf-list ll1 {\n"
            "                    type string;\n"
            "                    default \"def1\";\n"
            "                    default \"def2\";\n"
            "                    default \"def3\";\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "        leaf d {\n"
            "            type uint32;\n"
            "            default 15;\n"
            "        }\n"
            "        leaf-list ll2 {\n"
            "            type string;\n"
            "            default \"dflt1\";\n"
            "            default \"dflt2\";\n"
            "        }\n"
            "    }\n"
            "}";
    const char *schema_g =
            "module g {\n"
            "    namespace urn:tests:g;\n"
            "    prefix g;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    feature f1;\n"
            "    feature f2;\n"
            "    feature f3;\n"
            "\n"
            "    container cont {\n"
            "        if-feature \"f1\";\n"
            "        choice choic {\n"
            "            if-feature \"f2 or f3\";\n"
            "            leaf a {\n"
            "                type string;\n"
            "            }\n"
            "            case b {\n"
            "                if-feature \"f2 and f1\";\n"
            "                leaf l {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "        leaf d {\n"
            "            type uint32;\n"
            "        }\n"
            "        container cont2 {\n"
            "            if-feature \"f2\";\n"
            "            leaf e {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    const char *schema_h =
            "module h {\n"
            "    namespace urn:tests:h;\n"
            "    prefix h;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    container cont {\n"
            "        container cont2 {\n"
            "            config false;\n"
            "            leaf l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    const char *schema_i =
            "module i {\n"
            "    namespace urn:tests:i;\n"
            "    prefix i;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    container cont {\n"
            "        leaf l {\n"
            "            type string;\n"
            "        }\n"
            "        leaf l2 {\n"
            "            must \"../l = 'right'\";\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "}";
    const char *schema_j =
            "module j {\n"
            "    namespace urn:tests:j;\n"
            "    prefix j;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    feature feat1;\n"
            "\n"
            "    container cont {\n"
            "        must \"false()\";\n"
            "        list l1 {\n"
            "            key \"k\";\n"
            "            leaf k {\n"
            "                type string;\n"
            "            }\n"
            "            action act {\n"
            "                if-feature feat1;\n"
            "                input {\n"
            "                    must \"../../lf1 = 'true'\";\n"
            "                    leaf lf2 {\n"
            "                        type leafref {\n"
            "                            path /lf3;\n"
            "                        }\n"
            "                    }\n"
            "                }\n"
            "                output {\n"
            "                    must \"../../lf1 = 'true2'\";\n"
            "                    leaf lf2 {\n"
            "                        type leafref {\n"
            "                            path /lf4;\n"
            "                        }\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "\n"
            "        leaf lf1 {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "\n"
            "    leaf lf3 {\n"
            "        type string;\n"
            "    }\n"
            "\n"
            "    leaf lf4 {\n"
            "        type string;\n"
            "    }\n"
            "}";
    struct ly_in *in;
    const char *feats[] = {"feat1", NULL};

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &ctx));
    assert_non_null(ly_ctx_load_module(ctx, "ietf-netconf-with-defaults", "2011-06-01", NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_a, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_b, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_c, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_d, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_e, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_f, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_g, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_h, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_i, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(schema_j, &in));
    assert_int_equal(LY_SUCCESS, lys_parse(ctx, in, LYS_IN_YANG, feats, NULL));
    ly_in_free(in, 0);

    return 0;
}

static int
teardown(void **state)
{
    (void)state;
    ly_ctx_destroy(ctx, NULL);
    ctx = NULL;

    return 0;
}

static int
teardown_s(void **state)
{
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
#else
    (void) state; /* unused */
#endif

    return 0;
}

void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

static void
test_when(void **state)
{
    *state = test_when;

    const char *data;
    struct lyd_node *tree;

    data = "<c xmlns=\"urn:tests:a\">hey</c>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("When condition \"/cont/b = 'val_b'\" not satisfied. /a:c");

    data = "<cont xmlns=\"urn:tests:a\"><b>val_b</b></cont><c xmlns=\"urn:tests:a\">hey</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_string_equal("c", tree->next->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);

    data = "<cont xmlns=\"urn:tests:a\"><a>val</a><b>val_b</b></cont><c xmlns=\"urn:tests:a\">val_c</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_string_equal("a", lyd_child(tree)->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, lyd_child(tree)->flags);
    assert_string_equal("c", tree->next->schema->name);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);

    *state = NULL;
}

static void
test_mandatory(void **state)
{
    *state = test_mandatory;

    const char *data;
    struct lyd_node *tree;

    data = "<d xmlns=\"urn:tests:b\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"choic\" instance does not exist. /b:choic");

    data = "<l xmlns=\"urn:tests:b\">string</l><d xmlns=\"urn:tests:b\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Mandatory node \"c\" instance does not exist. /b:c");

    data = "<a xmlns=\"urn:tests:b\">string</a><c xmlns=\"urn:tests:b\">string2</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    *state = NULL;
}

static void
test_minmax(void **state)
{
    *state = test_minmax;

    const char *data;
    struct lyd_node *tree;

    data = "<d xmlns=\"urn:tests:c\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
            "<l xmlns=\"urn:tests:c\">val1</l>"
            "<l xmlns=\"urn:tests:c\">val2</l>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Too few \"l\" instances. /c:choic/b/l");

    data =
            "<l xmlns=\"urn:tests:c\">val1</l>"
            "<l xmlns=\"urn:tests:c\">val2</l>"
            "<l xmlns=\"urn:tests:c\">val3</l>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
            "<l xmlns=\"urn:tests:c\">val1</l>"
            "<l xmlns=\"urn:tests:c\">val2</l>"
            "<l xmlns=\"urn:tests:c\">val3</l>"
            "<lt xmlns=\"urn:tests:c\"><k>val1</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val2</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val3</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val4</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val5</k></lt>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Too many \"lt\" instances. /c:lt");

    *state = NULL;
}

static void
test_unique(void **state)
{
    *state = test_unique;

    const char *data;
    struct lyd_node *tree;

    data =
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>same</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>same</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>not-same</l1>\n"
            "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>same</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>same</l1>\n"
            "</lt>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l1\" not satisfied in \"/d:lt[k='val1']\" and \"/d:lt[k='val2']\". /d:lt[k='val2']");

    /* now try with more instances */
    data =
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>1</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <l1>3</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <l1>4</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <l1>5</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val6</k>\n"
            "    <l1>6</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val7</k>\n"
            "    <l1>7</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val8</k>\n"
            "    <l1>8</l1>\n"
            "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>1</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <l1>3</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <l1>5</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val6</k>\n"
            "    <l1>6</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val7</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val8</k>\n"
            "</lt>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>1</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <l1>4</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val6</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val7</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val8</k>\n"
            "    <l1>8</l1>\n"
            "</lt>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l1\" not satisfied in \"/d:lt[k='val7']\" and \"/d:lt[k='val2']\". /d:lt[k='val2']");

    *state = NULL;
}

static void
test_unique_nested(void **state)
{
    *state = test_unique_nested;

    const char *data;
    struct lyd_node *tree;

    /* nested list uniquest are compared only with instances in the same parent list instance */
    data =
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>2</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>1</l3>\n"
            "    </lt3>\n"
            "    <lt3>\n"
            "        <kk>val2</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>3</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>4</l2>\n"
            "    </cont>\n"
            "    <l4>4</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <l4>5</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_siblings(tree);

    data =
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>1</l3>\n"
            "    </lt3>\n"
            "    <lt3>\n"
            "        <kk>val2</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "    <lt3>\n"
            "        <kk>val3</kk>\n"
            "        <l3>1</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>4</l2>\n"
            "    </cont>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l3\" not satisfied in \"/d:lt2[k='val2']/lt3[kk='val3']\" and"
            " \"/d:lt2[k='val2']/lt3[kk='val1']\". /d:lt2[k='val2']/lt3[kk='val1']");

    data =
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>2</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>3</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>2</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <l4>5</l4>\n"
            "</lt2>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"cont/l2 l4\" not satisfied in \"/d:lt2[k='val4']\" and \"/d:lt2[k='val2']\". /d:lt2[k='val2']");

    data =
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>1</l5>\n"
            "    <l6>1</l6>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>1</l5>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>3</l5>\n"
            "    <l6>3</l6>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>4</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l6>1</l6>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>3</l5>\n"
            "    <l6>3</l6>\n"
            "</lt2>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Unique data leaf(s) \"l5 l6\" not satisfied in \"/d:lt2[k='val5']\" and \"/d:lt2[k='val3']\". /d:lt2[k='val3']");

    *state = NULL;
}

static void
test_dup(void **state)
{
    *state = test_dup;

    const char *data;
    struct lyd_node *tree;

    data = "<d xmlns=\"urn:tests:e\">25</d><d xmlns=\"urn:tests:e\">50</d>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"d\". /e:d");

    data = "<lt xmlns=\"urn:tests:e\"><k>A</k></lt><lt xmlns=\"urn:tests:e\"><k>B</k></lt><lt xmlns=\"urn:tests:e\"><k>A</k></lt>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"lt\". /e:lt[k='A']");

    data = "<ll xmlns=\"urn:tests:e\">A</ll><ll xmlns=\"urn:tests:e\">B</ll><ll xmlns=\"urn:tests:e\">B</ll>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"ll\". /e:ll[.='B']");

    data = "<cont xmlns=\"urn:tests:e\"></cont><cont xmlns=\"urn:tests:e\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"cont\". /e:cont");

    /* same tests again but using hashes */
    data = "<cont xmlns=\"urn:tests:e\"><d>25</d><d>50</d><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll></cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"d\". /e:cont/d");

    data = "<cont xmlns=\"urn:tests:e\"><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll>"
            "<lt><k>a</k></lt><lt><k>b</k></lt><lt><k>c</k></lt><lt><k>d</k></lt><lt><k>c</k></lt></cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"lt\". /e:cont/lt[k='c']");

    data = "<cont xmlns=\"urn:tests:e\"><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll>"
            "<ll>a</ll><ll>b</ll><ll>c</ll><ll>d</ll><ll>d</ll></cont>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"ll\". /e:cont/ll[.='d']");

    /* cases */
    data = "<l xmlns=\"urn:tests:e\">a</l><l xmlns=\"urn:tests:e\">b</l><l xmlns=\"urn:tests:e\">c</l><l xmlns=\"urn:tests:e\">b</l>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Duplicate instance of \"l\". /e:l[.='b']");

    data = "<l xmlns=\"urn:tests:e\">a</l><l xmlns=\"urn:tests:e\">b</l><l xmlns=\"urn:tests:e\">c</l><a xmlns=\"urn:tests:e\">aa</a>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Data for both cases \"a\" and \"b\" exist. /e:choic");

    *state = NULL;
}

static void
test_defaults(void **state)
{
    *state = test_defaults;

    char *str;
    struct lyd_node *tree, *node, *diff;
    const struct lys_module *mod = ly_ctx_get_module_latest(ctx, "f");

    struct ly_out *out;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    /* get defaults */
    tree = NULL;
    assert_int_equal(lyd_validate_module(&tree, mod, 0, &diff), LY_SUCCESS);
    assert_non_null(tree);
    assert_non_null(diff);

    /* check all defaults exist */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
            "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "<d xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def1</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def2</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def3</ll1>\n"
            "<d xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <ll1 yang:operation=\"create\">def1</ll1>\n"
            "  <ll1 yang:operation=\"create\">def2</ll1>\n"
            "  <ll1 yang:operation=\"create\">def3</ll1>\n"
            "  <d yang:operation=\"create\">15</d>\n"
            "  <ll2 yang:operation=\"create\">dflt1</ll2>\n"
            "  <ll2 yang:operation=\"create\">dflt2</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);
    lyd_free_siblings(diff);

    /* create another explicit case and validate */
    assert_int_equal(lyd_new_term(NULL, mod, "l", "value", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def1</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def2</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def3</ll1>\n");
    ly_out_reset(out);
    lyd_free_siblings(diff);

    /* create explicit leaf-list and leaf and validate */
    assert_int_equal(lyd_new_term(NULL, mod, "d", "15", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll2", "dflt2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
            "<d xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">dflt2</ll2>\n");
    ly_out_reset(out);
    lyd_free_siblings(diff);

    /* create first explicit container, which should become implicit */
    assert_int_equal(lyd_new_inner(NULL, mod, "cont", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);

    /* check diff */
    assert_null(diff);

    /* create second explicit container, which should become implicit, so the first tree node should be removed */
    assert_int_equal(lyd_new_inner(NULL, mod, "cont", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);

    /* check diff */
    assert_null(diff);

    /* similar changes for nested defaults */
    assert_int_equal(lyd_new_term(tree->prev, NULL, "ll1", "def3", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(tree->prev, NULL, "d", "5", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(tree->prev, NULL, "ll2", "non-dflt", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, ctx, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG);
    assert_string_equal(str,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1>def3</ll1>\n"
            "  <d>5</d>\n"
            "  <ll2>non-dflt</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);

    /* check diff */
    lyd_print_all(out, diff, LYD_XML, LYD_PRINT_WD_ALL);
    assert_string_equal(str,
            "<cont xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ll1 yang:operation=\"delete\">def1</ll1>\n"
            "  <ll1 yang:operation=\"delete\">def2</ll1>\n"
            "  <ll1 yang:operation=\"delete\">def3</ll1>\n"
            "  <d yang:operation=\"delete\">15</d>\n"
            "  <ll2 yang:operation=\"delete\">dflt1</ll2>\n"
            "  <ll2 yang:operation=\"delete\">dflt2</ll2>\n"
            "</cont>\n");
    ly_out_reset(out);
    lyd_free_siblings(diff);

    lyd_free_siblings(tree);
    ly_out_free(out, NULL, 1);

    *state = NULL;
}

static void
test_state(void **state)
{
    *state = test_state;

    const char *data;
    struct lyd_node *tree;

    data =
            "<cont xmlns=\"urn:tests:h\">\n"
            "  <cont2>\n"
            "    <l>val</l>\n"
            "  </cont2>\n"
            "</cont>\n";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_NO_STATE, 0, &tree));
    assert_null(tree);
    logbuf_assert("Invalid state data node \"cont2\" found. /h:cont/cont2");

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT | LYD_VALIDATE_NO_STATE, &tree));
    assert_null(tree);
    logbuf_assert("Invalid state data node \"cont2\" found. /h:cont/cont2");

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_EVALID, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT | LYD_VALIDATE_NO_STATE, NULL));
    logbuf_assert("Invalid state data node \"cont2\" found. /h:cont/cont2");

    lyd_free_siblings(tree);

    *state = NULL;
}

static void
test_must(void **state)
{
    *state = test_must;

    const char *data;
    struct lyd_node *tree;

    data =
            "<cont xmlns=\"urn:tests:i\">\n"
            "  <l>wrong</l>\n"
            "  <l2>val</l2>\n"
            "</cont>\n";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    logbuf_assert("Must condition \"../l = 'right'\" not satisfied. /i:cont/l2");

    data =
            "<cont xmlns=\"urn:tests:i\">\n"
            "  <l>right</l>\n"
            "  <l2>val</l2>\n"
            "</cont>\n";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_tree(tree);

    *state = NULL;
}

static void
test_action(void **state)
{
    *state = test_action;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op_tree;

    data =
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <l1>\n"
            "    <k>val1</k>\n"
            "    <act>\n"
            "      <lf2>target</lf2>\n"
            "    </act>\n"
            "  </l1>\n"
            "</cont>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(ctx, in, LYD_XML, &op_tree, NULL));
    assert_non_null(op_tree);

    /* missing leafref */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, NULL, LYD_VALIDATE_OP_RPC, NULL));
    logbuf_assert("Invalid leafref value \"target\" - no target instance \"/lf3\" with the same value."
            " /j:cont/l1[k='val1']/act/lf2");
    ly_in_free(in, 0);

    data =
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>not true</lf1>\n"
            "</cont>\n"
            "<lf3 xmlns=\"urn:tests:j\">target</lf3>\n";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    /* input must false */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_RPC, NULL));
    logbuf_assert("Must condition \"../../lf1 = 'true'\" not satisfied. /j:cont/l1[k='val1']/act");

    lyd_free_siblings(tree);
    data =
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>true</lf1>\n"
            "</cont>\n"
            "<lf3 xmlns=\"urn:tests:j\">target</lf3>\n";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    /* success */
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_RPC, NULL));

    lyd_free_tree(op_tree);
    lyd_free_siblings(tree);

    *state = NULL;
}

static void
test_reply(void **state)
{
    *state = test_reply;

    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op_tree, *request;

    data =
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <l1>\n"
            "    <k>val1</k>\n"
            "    <act>\n"
            "      <lf2>target</lf2>\n"
            "    </act>\n"
            "  </l1>\n"
            "</cont>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(ctx, in, LYD_XML, &request, NULL));
    assert_non_null(request);
    ly_in_free(in, 0);

    data = "<lf2 xmlns=\"urn:tests:j\">target</lf2>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_reply(request, in, LYD_XML, &op_tree, NULL));
    lyd_free_all(request);
    assert_non_null(op_tree);
    ly_in_free(in, 0);

    /* missing leafref */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, NULL, LYD_VALIDATE_OP_REPLY, NULL));
    logbuf_assert("Invalid leafref value \"target\" - no target instance \"/lf4\" with the same value."
            " /j:cont/l1[k='val1']/act/lf2");

    data =
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>not true</lf1>\n"
            "</cont>\n"
            "<lf4 xmlns=\"urn:tests:j\">target</lf4>\n";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    /* input must false */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_REPLY, NULL));
    logbuf_assert("Must condition \"../../lf1 = 'true2'\" not satisfied. /j:cont/l1[k='val1']/act");

    lyd_free_siblings(tree);
    data =
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>true2</lf1>\n"
            "</cont>\n"
            "<lf4 xmlns=\"urn:tests:j\">target</lf4>\n";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    /* success */
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op_tree, tree, LYD_VALIDATE_OP_REPLY, NULL));

    lyd_free_tree(op_tree);
    lyd_free_siblings(tree);

    *state = NULL;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_teardown(test_when, teardown_s),
        cmocka_unit_test_teardown(test_mandatory, teardown_s),
        cmocka_unit_test_teardown(test_minmax, teardown_s),
        cmocka_unit_test_teardown(test_unique, teardown_s),
        cmocka_unit_test_teardown(test_unique_nested, teardown_s),
        cmocka_unit_test_teardown(test_dup, teardown_s),
        cmocka_unit_test_teardown(test_defaults, teardown_s),
        cmocka_unit_test_teardown(test_state, teardown_s),
        cmocka_unit_test_teardown(test_must, teardown_s),
        cmocka_unit_test_teardown(test_action, teardown_s),
        cmocka_unit_test_teardown(test_reply, teardown_s),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
