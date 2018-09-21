/*
 * @file xml.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from xml.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"
#include "../../src/xml.c"

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
test_element(void **state)
{
    (void) state; /* unused */

    size_t name_len, prefix_len;
    const char *name, *prefix;
    const char *str, *p;

    struct lyxml_context ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.line = 1;

    /* empty */
    str = "";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_true(str[0] == '\0');

    /* no element */
    logbuf_clean();
    str = p = "no data present";
    assert_int_equal(LY_EINVAL, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("");

    /* not supported DOCTYPE */
    str = p = "<!DOCTYPE greeting SYSTEM \"hello.dtd\"><greeting/>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("Document Type Declaration not supported. Line number 1.");

    /* unqualified element */
    str = "  <  element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_string_equal("/>", str);

    str = "<?xml version=\"1.0\"?>  <!-- comment --> <?TEST xxx?> <element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_string_equal("/>", str);

    str = "<element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_string_equal(" xmlns=\"urn\"></element>", str);

    /* qualified element */
    str = "  <  yin:element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("yin", prefix, prefix_len));
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(3, prefix_len);
    assert_int_equal(7, name_len);
    assert_string_equal("/>", str);

    str = "<yin:element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("yin", prefix, prefix_len));
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(3, prefix_len);
    assert_int_equal(7, name_len);
    assert_string_equal(" xmlns=\"urn\"></element>", str);

    /* UTF8 characters */
    str = "<𠜎€𠜎Øn:𠜎€𠜎Øn/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("𠜎€𠜎Øn", prefix, prefix_len));
    assert_false(strncmp("𠜎€𠜎Øn", name, name_len));
    assert_int_equal(14, prefix_len);
    assert_int_equal(14, name_len);
    assert_string_equal("/>", str);

    /* invalid UTF-8 character */
    str = "<¢:element>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    logbuf_assert("Identifier \"¢:element>\" starts with invalid character. Line number 1.");
    str = "<yin:c⁐element>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, 0, &prefix, &prefix_len, &name, &name_len));
    logbuf_assert("Invalid character 0x9081e2. Line number 1.");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_element, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
