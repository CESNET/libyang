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
test_utf8(void **state)
{
    (void) state; /* unused */

    char buf[5] = {0};
    const char *str = buf;
    unsigned int c;
    size_t len;

    /* test invalid UTF-8 characters in lyxml_getutf8
     * - https://en.wikipedia.org/wiki/UTF-8 */
    buf[0] = 0x04;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));
    buf[0] = 0x80;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));

    buf[0] = 0xc0;
    buf[1] = 0x00;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));
    buf[1] = 0x80;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));

    buf[0] = 0xe0;
    buf[1] = 0x00;
    buf[2] = 0x80;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));
    buf[1] = 0x80;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));

    buf[0] = 0xf0;
    buf[1] = 0x00;
    buf[2] = 0x80;
    buf[3] = 0x80;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));
    buf[1] = 0x80;
    assert_int_equal(LY_EINVAL, lyxml_getutf8(&str, &c, &len));
}

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
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_true(str[0] == '\0');

    /* no element */
    logbuf_clean();
    str = p = "no data present";
    assert_int_equal(LY_EINVAL, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("");

    /* not supported DOCTYPE */
    str = p = "<!DOCTYPE greeting SYSTEM \"hello.dtd\"><greeting/>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("Document Type Declaration not supported. Line number 1.");

    /* unqualified element */
    str = "  <  element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_string_equal("/>", str);

    str = "<?xml version=\"1.0\"?>  <!-- comment --> <![CDATA[<greeting>Hello, world!</greeting>]]> <?TEST xxx?> <element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_string_equal("/>", str);

    str = "<element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_string_equal(" xmlns=\"urn\"></element>", str);

    /* qualified element */
    str = "  <  yin:element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("yin", prefix, prefix_len));
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(3, prefix_len);
    assert_int_equal(7, name_len);
    assert_string_equal("/>", str);

    str = "<yin:element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("yin", prefix, prefix_len));
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(3, prefix_len);
    assert_int_equal(7, name_len);
    assert_string_equal(" xmlns=\"urn\"></element>", str);

    /* UTF8 characters */
    str = "<𠜎€𠜎Øn:𠜎€𠜎Øn/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("𠜎€𠜎Øn", prefix, prefix_len));
    assert_false(strncmp("𠜎€𠜎Øn", name, name_len));
    assert_int_equal(14, prefix_len);
    assert_int_equal(14, name_len);
    assert_string_equal("/>", str);

    /* invalid UTF-8 character */
    str = "<¢:element>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    logbuf_assert("Identifier \"¢:element>\" starts with invalid character. Line number 1.");
    str = "<yin:c⁐element>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    logbuf_assert("Invalid character sequence \"⁐element>\", expected whitespace or element tag termination ('>' or '/>'. Line number 1.");
}

static void
test_attribute(void **state)
{
    (void) state; /* unused */

    size_t name_len, prefix_len;
    const char *name, *prefix;
    const char *str, *p;

    struct lyxml_context ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.line = 1;

    /* empty - without element tag termination */
    str = "";
    assert_int_equal(LY_EINVAL, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));

    /* empty - without element tag termination */
    str = "   />";
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_true(str[0] == '/');
    str = ">";
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_true(str[0] == '>');

    /* not an attribute */
    str = p = "unknown/>";
    assert_int_equal(LY_EVALID, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("Invalid character sequence \"/>\", expected whitespace or '='. Line number 1.");
    str = p = "unknown />";
    assert_int_equal(LY_EVALID, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("Invalid character sequence \"/>\", expected '='. Line number 1.");
    str = p = "xxx=/>";
    assert_int_equal(LY_EVALID, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("Invalid character sequence \"/>\", expected either single or double quotation mark. Line number 1.");
    str = p = "xxx\n = yyy/>";
    assert_int_equal(LY_EVALID, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_ptr_equal(p, str); /* input data not eaten */
    logbuf_assert("Invalid character sequence \"yyy/>\", expected either single or double quotation mark. Line number 2.");

    /* valid attribute */
    str = "xmlns=\"urn\">";
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_non_null(name);
    assert_null(prefix);
    assert_int_equal(5, name_len);
    assert_int_equal(0, prefix_len);
    assert_false(strncmp("xmlns", name, name_len));
    assert_string_equal("\"urn\">", str);

    str = "xmlns:nc\n = \'urn\'>";
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_non_null(name);
    assert_non_null(prefix);
    assert_int_equal(2, name_len);
    assert_int_equal(5, prefix_len);
    assert_int_equal(3, ctx.line);
    assert_false(strncmp("xmlns", prefix, prefix_len));
    assert_false(strncmp("nc", name, name_len));
    assert_string_equal("\'urn\'>", str);
}

static void
test_text(void **state)
{
    (void) state; /* unused */

    size_t out_len;
    const char *str, *p;
    char *out = NULL;

    struct lyxml_context ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.line = 1;

    /* empty attribute value */
    str = "\"\"";
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &out, &out_len));
    assert_non_null(out);
    assert_int_equal(1, out_len);
    assert_true(str[0] == '\0'); /* everything eaten */
    assert_true(out[0] == '\0'); /* empty string */
    str = "\'\'";
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &out, &out_len));
    assert_non_null(out);
    assert_int_equal(1, out_len);
    assert_true(str[0] == '\0'); /* everything eaten */
    assert_true(out[0] == '\0'); /* empty string */

    /* empty element content - only formating before defining child */
    str = "\n  <";
    assert_int_equal(LY_EINVAL, lyxml_get_string(&ctx, &str, &out, &out_len));
    assert_string_equal("<", str);

    /* empty element content is invalid - missing content terminating character < */
    str = "";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &out, &out_len));
    logbuf_assert("Unexpected end-of-file. Line number 1.");
    str = p = "xxx";

    free(out);
    out = NULL;

    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &out, &out_len));
    logbuf_assert("Unexpected end-of-file. Line number 1.");
    assert_ptr_equal(p, str); /* input data not eaten */

    free(out);
    out = NULL;

    /* valid strings */
    str = "€𠜎Øn \n&lt;&amp;&quot;&apos;&gt; &#82;&#x4f;&#x4B;<";
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &out, &out_len));
    assert_int_equal(22, out_len);
    assert_string_equal("€𠜎Øn \n<&\"\'> ROK", out);
    assert_string_equal("<", str);

    /* invalid characters in string */
    str = p = "\'&#x52\'";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &out, &out_len));
    logbuf_assert("Invalid character sequence \"'\", expected ;. Line number 2.");
    assert_ptr_equal(p, str); /* input data not eaten */
    str = p = "\"&#82\"";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &out, &out_len));
    logbuf_assert("Invalid character sequence \"\"\", expected ;. Line number 2.");
    assert_ptr_equal(p, str); /* input data not eaten */
    str = p = "\"&nonsence;\"";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &out, &out_len));
    logbuf_assert("Entity reference \"&nonsence;\" not supported, only predefined references allowed. Line number 2.");
    assert_ptr_equal(p, str); /* input data not eaten */
    str = p = "&#o122;";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &out, &out_len));
    logbuf_assert("Invalid character reference \"&#o122;\". Line number 2.");
    assert_ptr_equal(p, str); /* input data not eaten */

    free(out);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_utf8, logger_setup),
        cmocka_unit_test_setup(test_element, logger_setup),
        cmocka_unit_test_setup(test_attribute, logger_setup),
        cmocka_unit_test_setup(test_text, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
