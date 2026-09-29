/**
 * @file test_json.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for a generic JSON parser
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "context.h"

static void
test_general(void **state)
{
    struct ly_in *in;
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " leaf l {"
            "    type boolean;"
            " }"
            " leaf emp {"
            "    type empty;"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, NULL));

    /* empty */
    str = "";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:l", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Empty JSON file.", NULL, 1);

    str = "  \n\t \n";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:l", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Empty JSON file.", NULL, 3);

    /* constant values */
    str = "true";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:l", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("true", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "false";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:l", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("false", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "[null]";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:emp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("", lyd_get_value(tree));

    lyd_free_all(tree);
    ly_in_free(in, 0);
}

static void
test_number(void **state)
{
    struct ly_in *in;
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " leaf num {"
            "    type union {"
            "       type int32;"
            "       type decimal64 {"
            "           fraction-digits 4;"
            "       }"
            "   }"
            " }"
            " leaf exp {"
            "   type string;"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, NULL));

    /* simple value */
    str = "11";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("11", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* fraction number */
    str = "\"37.7668\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("37.7668", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* negative number */
    str = "\"-122.3959\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("-122.3959", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* integer, positive exponent */
    str = "550E3";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("550000", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* integer, negative exponent */
    str = "-550E3";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("-550000", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* integer, negative exponent */
    str = "\"1E-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("1E-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"15E-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("15E-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"-15E-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("-15E-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"16E-2\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("16E-2", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"-16E-2\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("-16E-2", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"17E-3\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("17E-3", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"-17E-3\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("-17E-3", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "21000E-2";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("210", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"21000E-4\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("21000E-4", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"21000E-7\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("21000E-7", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* decimal number, positive exponent */
    str = "\"5.087E1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("5.087E1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"-5.087E1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("-5.087E1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"5.087E5\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("5.087E5", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "59.1e+1";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("591", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.005087E1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.005087E1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.005087E2\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.005087E2", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "0.005087E6";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("5087", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "0.05087E6";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("50870", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "0.005087E8";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("508700", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* decimal number, negative exponent */
    str = "\"35.94e-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("35.94e-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"-35.94e-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("-35.94e-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"35.94e-2\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("35.94e-2", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"35.94e-3\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("35.94e-3", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.3594e-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.3594e-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.03594e-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.03594e-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.003594e-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.003594e-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.3594e-2\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.3594e-2", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.03594e-2\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.03594e-2", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"0.003594e-2\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0.003594e-2", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* zero */
    str = "0";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "-0";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "94E0";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("94", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "0E2";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "-0E2";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("0", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "5.320e+2";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("532", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    str = "\"5.320e-1\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:exp", in, LYD_JSON, 0, 0, 0, &tree));
    assert_string_equal("5.320e-1", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* various invalid inputs */
    str = "-x";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Invalid character in JSON Number value (\"x\").", NULL, 1);

    str = "  -";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Unexpected end-of-input.", NULL, 1);

    str = "--1";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Invalid character in JSON Number value (\"-\").", NULL, 1);

    str = "+1";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"+1\", expected a JSON value.", NULL, 1);

    str = "  1.x ";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Invalid character in JSON Number value (\"x\").", NULL, 1);

    str = "1.";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Unexpected end-of-input.", NULL, 1);

    str = "  1eo ";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Invalid character in JSON Number value (\"o\").", NULL, 1);

    str = "1e";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Unexpected end-of-input.", NULL, 1);

    str = "1E1000";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Number encoded as a string exceeded the LY_NUMBER_MAXLEN limit.", NULL, 1);

    str = "1e9999999999999999999";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Exponent out-of-bounds in a JSON Number value (1e9999999999999999999).", NULL, 1);

    str = "1.1e66000";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Exponent out-of-bounds in a JSON Number value (1.1e66000).", NULL, 1);

    str = "1.1e-66000";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:num", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Exponent out-of-bounds in a JSON Number value (1.1e-66000).", NULL, 1);

    ly_in_free(in, 0);
}

/* now string is tested in file ./tests/utests/types/string.c */
static void
test_string(void **state)
{
    struct ly_in *in = NULL;
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " leaf l {"
            "   type string;"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, NULL));

    str = "";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));

    /* unterminated string */
    str = "\"unterminated string";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyd_parse_value_fragment(ctx, "/test:l", in, LYD_JSON, 0, 0, 0, &tree));
    CHECK_LOG_CTX("Missing quotation-mark at the end of a JSON string.", NULL, 1);
    CHECK_LOG_CTX("Unexpected end-of-input.", NULL, 1);

    ly_in_free(in, 0);
}

static void
test_object(void **state)
{
    struct ly_in *in;
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree, *search_node;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " leaf name {"
            "   type string;"
            " }"
            " leaf smart {"
            "   type boolean;"
            " }"
            " leaf handsom {"
            "   type boolean;"
            " }"
            " container person {"
            "   leaf name {"
            "     type string;"
            "   }"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, NULL));

    /* empty */
    str = "  { }  ";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_value_fragment(ctx, "/test:name", in, LYD_JSON, 0, 0, 0, &tree));
    assert_null(tree);

    /* simple value */
    str = "{\"test:name\" : \"Radek\"}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_string_equal("Radek", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* two values */
    str = "{\"test:smart\" : true,\"test:handsom\":false}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));

    assert_int_equal(LY_SUCCESS, lyd_find_path(tree, "/test:smart", 0, &search_node));
    assert_string_equal("true", lyd_get_value(search_node));
    assert_int_equal(LY_SUCCESS, lyd_find_path(tree, "/test:handsom", 0, &search_node));
    assert_string_equal("false", lyd_get_value(search_node));
    lyd_free_all(tree);
    tree = NULL;

    /* inherited objects */
    str = "{\"test:person\" : {\"name\":\"Radek\"}}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_int_equal(LY_SUCCESS, lyd_find_path(tree, "/test:person/name", 0, &search_node));
    assert_string_equal("Radek", lyd_get_value(search_node));
    lyd_free_all(tree);
    tree = NULL;

    /* unquoted string */
    str = "{ unquoted : \"data\"}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"unquoted : \"data\"}\", expected a JSON object name.", NULL, 1);

    ly_in_free(in, 0);
}

static void
test_array(void **state)
{
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;

    /* empty */
    str = "{\"mod:mixed-stuff\": [] }";
    assert_int_equal(LY_EINVAL, lyd_parse_data_mem(ctx, str, LYD_JSON,
            LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, &tree));
    CHECK_LOG_CTX("Unexpected input data array closed.", NULL, 1);

    /* simple value */
    str = "{\"mod:mixed-stuff\": [ null ]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_JSON,
            LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, &tree));
    lyd_free_all(tree);
    tree = NULL;

    /* two values */
    str = "{\"mod:mixed-stuff\": [{\"a\":null},\"x\"]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_JSON,
            LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, &tree));
    lyd_free_all(tree);
    tree = NULL;

    /* new line is allowed only as escaped character in JSON */
    str = "{\"mod:mixed-stuff\": [ , null]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_JSON,
            LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, &tree));
    CHECK_LOG_CTX("Invalid character sequence \", null]}\", expected a JSON value.", NULL, 1);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_general),
        UTEST(test_number),
        UTEST(test_string),
        UTEST(test_object),
        UTEST(test_array),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
