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

#include "common.h"
#include "../../src/set.c"
#include "../../src/xml.c"
#include "../../src/common.c"
#include "../../src/log.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"

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
    size_t buf_len, len;
    const char *name, *prefix;
    char *buf = NULL, *out = NULL;
    const char *str, *p;
    int dynamic;

    struct lyxml_context ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.line = 1;

    /* empty */
    str = "";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(name);
    assert_int_equal(LYXML_END, ctx.status);
    assert_true(str[0] == '\0');

    /* end element */
    str = "</element>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    logbuf_assert("Opening and closing elements tag missmatch (\"element>\"). Line number 1.");


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
    assert_int_equal(LYXML_ELEMENT, ctx.status);
    assert_string_equal("", str);
    assert_int_equal(0, ctx.elements.count);

    str = "  <  element attr=\'x\'/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_int_equal(LYXML_ATTRIBUTE, ctx.status);
    assert_string_equal("attr=\'x\'/>", str);
    assert_int_equal(1, ctx.elements.count);
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_int_equal(LYXML_ATTR_CONTENT, ctx.status);
    assert_string_equal("\'x\'/>", str);
    assert_int_equal(1, ctx.elements.count);
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_int_equal(LYXML_ELEMENT, ctx.status);
    assert_string_equal("", str);
    assert_int_equal(0, ctx.elements.count);

    str = "<?xml version=\"1.0\"?>  <!-- comment --> <![CDATA[<greeting>Hello, world!</greeting>]]> <?TEST xxx?> <element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_int_equal(LYXML_ELEMENT, ctx.status);
    assert_string_equal("", str);

    str = "<element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(7, name_len);
    assert_int_equal(LYXML_ATTRIBUTE, ctx.status);
    assert_string_equal("xmlns=\"urn\"></element>", str);
    /* cleean context by getting closing tag */
    str += 12;
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));

    /* qualified element */
    str = "  <  yin:element/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("yin", prefix, prefix_len));
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(3, prefix_len);
    assert_int_equal(7, name_len);
    assert_int_equal(LYXML_ELEMENT, ctx.status);
    assert_string_equal("", str);

    str = "<yin:element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("yin", prefix, prefix_len));
    assert_false(strncmp("element", name, name_len));
    assert_int_equal(3, prefix_len);
    assert_int_equal(7, name_len);
    assert_int_equal(LYXML_ATTRIBUTE, ctx.status);
    assert_string_equal("xmlns=\"urn\"></element>", str);
    /* cleean context by getting closing tag */
    str += 12;
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    logbuf_assert("Opening and closing elements tag missmatch (\"element>\"). Line number 1.");
    str = "</yin:element/>";
    assert_int_equal(LY_EVALID, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    logbuf_assert("Unexpected data \"/>\" in closing element tag. Line number 1.");
    lyxml_context_clear(&ctx);

    /* UTF8 characters */
    str = "<𠜎€𠜎Øn:𠜎€𠜎Øn/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_false(strncmp("𠜎€𠜎Øn", prefix, prefix_len));
    assert_false(strncmp("𠜎€𠜎Øn", name, name_len));
    assert_int_equal(14, prefix_len);
    assert_int_equal(14, name_len);
    assert_int_equal(LYXML_ELEMENT, ctx.status);
    assert_string_equal("", str);

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
    assert_int_equal(LYXML_ATTR_CONTENT, ctx.status);

    str = "xmlns:nc\n = \'urn\'/>";
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, &str, &prefix, &prefix_len, &name, &name_len));
    assert_non_null(name);
    assert_non_null(prefix);
    assert_int_equal(2, name_len);
    assert_int_equal(5, prefix_len);
    assert_int_equal(3, ctx.line);
    assert_false(strncmp("xmlns", prefix, prefix_len));
    assert_false(strncmp("nc", name, name_len));
    assert_string_equal("\'urn\'/>", str);
    assert_int_equal(LYXML_ATTR_CONTENT, ctx.status);

}

static void
test_text(void **state)
{
    (void) state; /* unused */

    size_t buf_len, len;
    int dynamic;
    const char *str, *p;
    char *buf = NULL, *out = NULL;

    struct lyxml_context ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.line = 1;

    /* empty attribute value */
    ctx.status = LYXML_ATTR_CONTENT;
    str = "\"\"";
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_null(buf);
    assert_ptr_equal(&str[-1], out);
    assert_int_equal(0, dynamic);
    assert_int_equal(0, len);
    assert_true(str[0] == '\0'); /* everything eaten */
    assert_int_equal(LYXML_ATTRIBUTE, ctx.status);

    ctx.status = LYXML_ATTR_CONTENT;
    str = "\'\'";
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_null(buf);
    assert_ptr_equal(&str[-1], out);
    assert_int_equal(0, dynamic);
    assert_int_equal(0, len);
    assert_true(str[0] == '\0'); /* everything eaten */
    assert_int_equal(LYXML_ATTRIBUTE, ctx.status);

    /* empty element content - only formating before defining child */
    ctx.status = LYXML_ELEM_CONTENT;
    str = "\n  <";
    assert_int_equal(LY_EINVAL, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_null(buf);
    assert_string_equal("<", str);

    /* empty element content is invalid - missing content terminating character < */
    ctx.status = LYXML_ELEM_CONTENT;
    str = "";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_null(buf);
    logbuf_assert("Unexpected end-of-file. Line number 2.");

    ctx.status = LYXML_ELEM_CONTENT;
    str = p = "xxx";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_null(buf);
    logbuf_assert("Unexpected end-of-file. Line number 2.");
    assert_ptr_equal(p, str); /* input data not eaten */

    /* valid strings */
    ctx.status = LYXML_ELEM_CONTENT;
    str = "€𠜎Øn \n&lt;&amp;&quot;&apos;&gt; &#82;&#x4f;&#x4B;<";
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_int_not_equal(0, dynamic);
    assert_non_null(buf);
    assert_ptr_equal(out, buf);
    assert_int_equal(22, buf_len);
    assert_int_equal(21, len);
    assert_string_equal("€𠜎Øn \n<&\"\'> ROK", buf);
    assert_string_equal("<", str);
    assert_int_equal(LYXML_ELEMENT, ctx.status);

    /* test using n-bytes UTF8 hexadecimal code points */
    ctx.status = LYXML_ATTR_CONTENT;
    str = "\'&#x0024;&#x00A2;&#x20ac;&#x10348;\'";
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    assert_int_not_equal(0, dynamic);
    assert_non_null(buf);
    assert_ptr_equal(out, buf);
    assert_int_equal(22, buf_len);
    assert_int_equal(10, len);
    assert_string_equal("$¢€𐍈", buf);
    assert_int_equal(LYXML_ATTRIBUTE, ctx.status);

    free(buf);
    buf = NULL;

    /* invalid characters in string */
    ctx.status = LYXML_ATTR_CONTENT;
    str = p = "\'&#x52\'";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    logbuf_assert("Invalid character sequence \"'\", expected ;. Line number 3.");
    assert_null(buf);
    assert_ptr_equal(p, str); /* input data not eaten */
    ctx.status = LYXML_ATTR_CONTENT;
    str = p = "\"&#82\"";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    logbuf_assert("Invalid character sequence \"\"\", expected ;. Line number 3.");
    assert_null(buf);
    assert_ptr_equal(p, str); /* input data not eaten */
    ctx.status = LYXML_ATTR_CONTENT;
    str = p = "\"&nonsence;\"";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    logbuf_assert("Entity reference \"&nonsence;\" not supported, only predefined references allowed. Line number 3.");
    assert_null(buf);
    assert_ptr_equal(p, str); /* input data not eaten */
    ctx.status = LYXML_ELEM_CONTENT;
    str = p = "&#o122;";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    logbuf_assert("Invalid character reference \"&#o122;\". Line number 3.");
    assert_null(buf);
    assert_ptr_equal(p, str); /* input data not eaten */

    ctx.status = LYXML_ATTR_CONTENT;
    str = p = "\'&#x06;\'";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    logbuf_assert("Invalid character reference \"&#x06;\'\" (0x00000006). Line number 3.");
    assert_null(buf);
    assert_ptr_equal(p, str); /* input data not eaten */
    ctx.status = LYXML_ATTR_CONTENT;
    str = p = "\'&#xfdd0;\'";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    logbuf_assert("Invalid character reference \"&#xfdd0;\'\" (0x0000fdd0). Line number 3.");
    assert_null(buf);
    assert_ptr_equal(p, str); /* input data not eaten */
    ctx.status = LYXML_ATTR_CONTENT;
    str = p = "\'&#xffff;\'";
    assert_int_equal(LY_EVALID, lyxml_get_string(&ctx, &str, &buf, &buf_len, &out, &len, &dynamic));
    logbuf_assert("Invalid character reference \"&#xffff;\'\" (0x0000ffff). Line number 3.");
    assert_null(buf);
    assert_ptr_equal(p, str); /* input data not eaten */
}

static void
test_ns(void **state)
{
    (void) state; /* unused */

    const char *e1, *e2;
    const struct lyxml_ns *ns;

    struct lyxml_context ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.line = 1;

    e1 = "element1";
    e2 = "element2";
    assert_int_equal(LY_SUCCESS, lyxml_ns_add(&ctx, e1, NULL, 0, strdup("urn:default")));
    assert_int_equal(LY_SUCCESS, lyxml_ns_add(&ctx, e1, "nc", 2, strdup("urn:nc1")));
    assert_int_equal(LY_SUCCESS, lyxml_ns_add(&ctx, e2, "nc", 2, strdup("urn:nc2")));
    assert_int_equal(3, (&ctx)->ns.count);
    assert_int_not_equal(0, (&ctx)->ns.size);

    ns = lyxml_ns_get(&ctx, NULL, 0);
    assert_non_null(ns);
    assert_null(ns->prefix);
    assert_string_equal("urn:default", ns->uri);

    ns = lyxml_ns_get(&ctx, "nc", 2);
    assert_non_null(ns);
    assert_string_equal("nc", ns->prefix);
    assert_string_equal("urn:nc2", ns->uri);

    assert_int_equal(LY_SUCCESS, lyxml_ns_rm(&ctx, e2));
    assert_int_equal(2, (&ctx)->ns.count);

    ns = lyxml_ns_get(&ctx, "nc", 2);
    assert_non_null(ns);
    assert_string_equal("nc", ns->prefix);
    assert_string_equal("urn:nc1", ns->uri);

    assert_int_equal(LY_SUCCESS, lyxml_ns_rm(&ctx, e1));
    assert_int_equal(0, (&ctx)->ns.count);

    assert_null(lyxml_ns_get(&ctx, "nc", 2));
    assert_null(lyxml_ns_get(&ctx, NULL, 0));
}

static void
test_simple_xml(void **state)
{
    (void)state; /* unused */
    size_t name_len, prefix_len;
    const char *prefix, *name;
    char *test_in = NULL;
    struct lyxml_context ctx;

    char *buf = NULL, *output = NULL;
    size_t buf_size, length;
    int dynamic;

    memset(&ctx, 0, sizeof ctx);
    ctx.line = 1;
    test_in = malloc(100);
    /* test input */
    strcpy(test_in, "<elem1 attr1=\"value\">      <elem2 attr2=\"value\"/> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, (const char **)&test_in, &prefix, &prefix_len, &name, &name_len));     /* <elem1 */
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, (const char **)&test_in, &prefix, &prefix_len, &name, &name_len));   /* attr1= */
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, (const char **)&test_in, &buf, &buf_size, &output, &length, &dynamic)); /* "value" */
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, (const char **)&test_in, &prefix, &prefix_len, &name, &name_len));   /* > */
    /* try to get string content of elem1 whitespace is removed and EINVAL is expected in this case */
    assert_int_equal(LY_EINVAL, lyxml_get_string(&ctx, (const char **)&test_in, &buf, &buf_size, &output, &length, &dynamic));
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, (const char **)&test_in, &prefix, &prefix_len, &name, &name_len));     /* <elem2 */
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, (const char **)&test_in, &prefix, &prefix_len, &name, &name_len));   /* attr2= */
    assert_int_equal(LY_SUCCESS, lyxml_get_string(&ctx, (const char **)&test_in, &buf, &buf_size, &output, &length, &dynamic)); /* "value" */
    assert_int_equal(LY_SUCCESS, lyxml_get_attribute(&ctx, (const char **)&test_in, &prefix, &prefix_len, &name, &name_len));   /* /> */
    assert_int_equal(LY_SUCCESS, lyxml_get_element(&ctx, (const char **)&test_in, &prefix, &prefix_len, &name, &name_len));     /* </elem1> */
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_element, logger_setup),
        cmocka_unit_test_setup(test_attribute, logger_setup),
        cmocka_unit_test_setup(test_text, logger_setup),
        cmocka_unit_test_setup(test_ns, logger_setup),
        cmocka_unit_test(test_simple_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
