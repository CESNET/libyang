/*
 * @file json.c
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

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json.h"
#include "context.h"
#include "parser_internal.h"

void *testfunc = NULL;

static int
setup(void **state)
{
    if (ly_ctx_new(NULL, 0, (struct ly_ctx **)state)) {
        return 1;
    }

    return 0;
}

static int
teardown(void **state)
{
    ly_ctx_destroy(*state, NULL);
    return 0;
}

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */

    if (path) {
        snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
    } else {
        strncpy(logbuf, msg, BUFSIZE - 1);
    }
}

static int
logger_setup(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif
    return 0;
}

static int
logger_teardown(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    if (testfunc) {
        fprintf(stderr, "%s\n", logbuf);
    }
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
test_general(void **state)
{
    struct lyjson_ctx *jsonctx;
    struct ly_in *in;
    const char *str;

    testfunc = test_general;

    /* empty */
    str = "";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    str = "  \n\t \n";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* constant values */
    str = "true";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_TRUE, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    str = "false";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_FALSE, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    str = "null";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NULL, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    ly_in_free(in, 0);
    testfunc = NULL;
}

static void
test_number(void **state)
{
    struct lyjson_ctx *jsonctx;
    struct ly_in *in;
    const char *str;

    testfunc = test_number;

    /* simple value */
    str = "11";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("11", jsonctx->value);
    assert_int_equal(2, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* fraction number */
    str = "37.7668";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("37.7668", jsonctx->value);
    assert_int_equal(7, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* negative number */
    str = "-122.3959";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("-122.3959", jsonctx->value);
    assert_int_equal(9, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* exp number */
    str = "1E10";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("10000000000", jsonctx->value);
    assert_int_equal(11, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    str = "15E-1";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("1.5", jsonctx->value);
    assert_int_equal(3, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    str = "15E-3";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("0.015", jsonctx->value);
    assert_int_equal(5, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* exp fraction number */
    str = "1.1e3";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("1100", jsonctx->value);
    assert_int_equal(4, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* negative exp fraction number */
    str = "1.1e-3";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("0.0011", jsonctx->value);
    assert_int_equal(6, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* exp negative fraction number */
    str = "-0.11e3";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("-110", jsonctx->value);
    assert_int_equal(4, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* negative exp negative fraction number */
    str = "-3.14e-3";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_NUMBER, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("-0.00314", jsonctx->value);
    assert_int_equal(8, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* various invalid inputs */
    str = "-x";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character in JSON Number value (\"x\"). Line number 1.");

    str = "  -";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Unexpected end-of-input. Line number 1.");

    str = "--1";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character in JSON Number value (\"-\"). Line number 1.");

    str = "+1";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character sequence \"+1\", expected a JSON value. Line number 1.");

    str = "  1.x ";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character in JSON Number value (\"x\"). Line number 1.");

    str = "1.";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Unexpected end-of-input. Line number 1.");

    str = "  1eo ";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character in JSON Number value (\"o\"). Line number 1.");

    str = "1e";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Unexpected end-of-input. Line number 1.");

    ly_in_free(in, 0);
    testfunc = NULL;
}

static void
test_string(void **state)
{
    struct lyjson_ctx *jsonctx;
    struct ly_in *in;
    const char *str;

    testfunc = test_string;

    /* simple string */
    str = "\"hello\"";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_STRING, lyjson_ctx_status(jsonctx, 0));
    assert_ptr_equal(&str[1], jsonctx->value);
    assert_int_equal(5, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* 4-byte utf8 character */
    str = "\"\\t𠜎\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_STRING, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("\t𠜎", jsonctx->value);
    assert_int_equal(5, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* valid escape sequences - note that here it mixes valid JSON string characters (RFC 7159, sec. 7) and
     * valid characters in YANG string type (RFC 7950, sec. 9.4). Since the latter is a subset of JSON string,
     * the YANG string type's restrictions apply to the JSON escape sequences */
    str = "\"\\\" \\\\ \\r \\/ \\n \\t \\u20ac\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_STRING, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("\" \\ \r / \n \t €", jsonctx->value);
    assert_int_equal(15, jsonctx->value_len);
    assert_int_equal(1, jsonctx->dynamic);
    lyjson_ctx_free(jsonctx);

    /* backspace and form feed are valid JSON escape sequences, but the control characters they represents are not allowed values for YANG string type */
    str = "\"\\b\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character reference \"\\b\" (0x00000008). Line number 1.");

    str = "\"\\f\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character reference \"\\f\" (0x0000000c). Line number 1.");

    /* unterminated string */
    str = "\"unterminated string";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Missing quotation-mark at the end of a JSON string. Line number 1.");

    /* invalid escape sequence */
    str = "\"char \\x \"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character escape sequence \\x. Line number 1.");

    /* new line is allowed only as escaped character in JSON */
    str = "\"\n\"";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character in JSON string \"\n\" (0x0000000a). Line number 1.");

    ly_in_free(in, 0);
    testfunc = NULL;
}

static void
test_object(void **state)
{
    struct lyjson_ctx *jsonctx;
    struct ly_in *in;
    const char *str;

    testfunc = test_object;

    /* empty */
    str = "  { }  ";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_OBJECT_EMPTY, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* simple value */
    str = "{\"name\" : \"Radek\"}";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_OBJECT, lyjson_ctx_status(jsonctx, 0));
    assert_ptr_equal(&str[2], jsonctx->value);
    assert_int_equal(4, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("\"Radek\"}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_STRING, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("Radek\"}", jsonctx->value);
    assert_int_equal(5, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT_CLOSED, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* two values */
    str = "{\"smart\" : true,\"handsom\":false}";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_OBJECT, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("smart\" : true,\"handsom\":false}", jsonctx->value);
    assert_int_equal(5, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("true,\"handsom\":false}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_TRUE, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal(",\"handsom\":false}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("handsom\":false}", jsonctx->value);
    assert_int_equal(7, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("false}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_FALSE, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT_CLOSED, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* inherited objects */
    str = "{\"person\" : {\"name\":\"Radek\"}}";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_OBJECT, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("person\" : {\"name\":\"Radek\"}}", jsonctx->value);
    assert_int_equal(6, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("{\"name\":\"Radek\"}}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("name\":\"Radek\"}}", jsonctx->value);
    assert_int_equal(4, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("\"Radek\"}}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_STRING, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("Radek\"}}", jsonctx->value);
    assert_int_equal(5, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("}}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT_CLOSED, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("}", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT_CLOSED, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* new line is allowed only as escaped character in JSON */
    str = "{ unquoted : \"data\"}";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_EVALID, lyjson_ctx_new(*state, in, &jsonctx));
    logbuf_assert("Invalid character sequence \"unquoted : \"data\"}\", expected a JSON object's member. Line number 1.");

    ly_in_free(in, 0);
    testfunc = NULL;
}

static void
test_array(void **state)
{
    struct lyjson_ctx *jsonctx;
    struct ly_in *in;
    const char *str;

    testfunc = test_array;

    /* empty */
    str = "  [  ]  ";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_ARRAY_EMPTY, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* simple value */
    str = "[ null]";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_ARRAY, lyjson_ctx_status(jsonctx, 0));
    assert_null(jsonctx->value);
    assert_int_equal(0, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("null]", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_NULL, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("]", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_ARRAY_CLOSED, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* two values */
    str = "[{\"a\":null},\"x\"]";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LYJSON_ARRAY, lyjson_ctx_status(jsonctx, 0));
    assert_null(jsonctx->value);
    assert_int_equal(0, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("{\"a\":null},\"x\"]", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("a\":null},\"x\"]", jsonctx->value);
    assert_int_equal(1, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("null},\"x\"]", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_NULL, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("},\"x\"]", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_OBJECT_CLOSED, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal(",\"x\"]", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_STRING, lyjson_ctx_status(jsonctx, 0));
    assert_string_equal("x\"]", jsonctx->value);
    assert_int_equal(1, jsonctx->value_len);
    assert_int_equal(0, jsonctx->dynamic);
    assert_string_equal("]", jsonctx->in->current);
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_ARRAY_CLOSED, lyjson_ctx_status(jsonctx, 0));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_next(jsonctx, NULL));
    assert_int_equal(LYJSON_END, lyjson_ctx_status(jsonctx, 0));
    lyjson_ctx_free(jsonctx);

    /* new line is allowed only as escaped character in JSON */
    str = "[ , null]";
    assert_non_null(ly_in_memory(in, str));
    assert_int_equal(LY_SUCCESS, lyjson_ctx_new(*state, in, &jsonctx));
    assert_int_equal(LY_EVALID, lyjson_ctx_next(jsonctx, NULL));
    logbuf_assert("Invalid character sequence \", null]\", expected a JSON value. Line number 1.");
    lyjson_ctx_free(jsonctx);

    ly_in_free(in, 0);
    testfunc = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_general, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_number, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_string, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_object, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_array, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
