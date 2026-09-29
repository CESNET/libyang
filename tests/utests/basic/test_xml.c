/**
 * @file test_xml.c
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
#define _UTEST_MAIN_
#include "utests.h"

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L /* strdup */
#endif

#include <string.h>

#include "context.h"

static void
test_element(void **state)
{
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " leaf element {"
            "   type string;"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, NULL));

    /* empty */
    str = "";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));

    /* end element */
    str = "</element>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Stray closing element tag (\"element\").", NULL, 1);

    /* no element */
    str = "no data present";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"no data present\", expected element tag start ('<').", NULL, 1);

    /* not supported DOCTYPE */
    str = "<!DOCTYPE greeting SYSTEM \"hello.dtd\"><greeting/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Document Type Declaration not supported.", NULL, 1);

    /* invalid XML */
    str = "<!NONSENSE/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Unknown XML section \"<!NONSENSE/>\".", NULL, 1);

    /* namespace ambiguity */
    str = "<element xmlns=\"urn1\" xmlns=\"urn2\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Duplicate default XML namespaces \"urn1\" and \"urn2\".", NULL, 1);

    /* prefix duplicate */
    str = "<element xmlns:a=\"urn1\" xmlns:a=\"urn2\"/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Duplicate XML NS prefix \"a\" used for namespaces \"urn1\" and \"urn2\".", NULL, 1);

    /* unqualified element */
    str = "  <  element/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    UTEST_LOG_CTX_CLEAN;

    /* element with attribute */
    str = "  <  element attr=\'x\'/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    UTEST_LOG_CTX_CLEAN;

    /* headers and comments */
    str = "<?xml version=\"1.0\"?>  <!-- comment --> <?TEST xxx?> <element/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    UTEST_LOG_CTX_CLEAN;

    /* separate opening and closing tags, neamespaced parsed internally */
    str = "<element xmlns=\"urn:test\"></element>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_string_equal("element", tree->schema->name);
    assert_string_equal("", lyd_get_value(tree));
    lyd_free_all(tree);
    tree = NULL;

    /* qualified element */
    str = "  <  yin:element/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));

    /* non-matching closing tag */
    str = "<t:element xmlns:t=\"urn:test\"></element>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Opening (\"t:element\") and closing (\"element\") elements tag mismatch.", NULL, 1);

    /* invalid closing tag */
    str = "<t:element xmlns:t=\"urn:test\"></t:element/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"/>\", expected element tag termination ('>').", NULL, 1);
    UTEST_LOG_CTX_CLEAN;

    /* UTF8 characters */
    str = "<𠜎€𠜎Øn:𠜎€𠜎Øn xmlns:𠜎€𠜎Øn=\"urn:test\"/>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);

    /* invalid UTF-8 characters */
    str = "<¢:element>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Identifier \"¢:element>\" starts with an invalid character.", NULL, 1);

    str = "<test:c⁐element>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"⁐element>\", expected element tag end ('>' or '/>') or an attribute.", NULL, 1);

    /* mixed content */
    str = "<a>text <b>x</b></a>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    UTEST_LOG_CTX_CLEAN;

    /* tag mismatch */
    str = "<a xmlns=\"urn:test\">text</b>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Opening (\"a\") and closing (\"b\") elements tag mismatch.", NULL, 1);

}

static void
test_attribute(void **state)
{
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " anyxml e;"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, NULL));

    /* not an attribute */
    str = "<e unknown/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"/>\", expected '='.", NULL, 1);

    str = "<e xxx=/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"/>\", expected either single or double quotation mark.", NULL, 1);

    str = "<e xxx\n = yyy/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"yyy/>\", expected either single or double quotation mark.", NULL, 2);

    /* valid attribute */
    str = "<e xmlns=\"urn:test\" attr=\"val\"/>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_string_equal("e", tree->schema->name);
    lyd_free_all(tree);
    tree = NULL;

    /* valid namespace with prefix */
    str = "<e xmlns=\"urn:test\" xmlns:nc\n = \'urn\'/>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_string_equal("e", tree->schema->name);
    lyd_free_all(tree);
}

static void
test_text(void **state)
{
    const char *str;
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " anyxml e;"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, NULL));
    /* empty attribute value */
    str = "<e xmlns=\"urn:test\" attr=\"\"/>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_all(tree);
    tree = NULL;

    /* empty value but in single quotes */
    str = "<e xmlns=\"urn:test\" attr=\'\'/>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_all(tree);
    tree = NULL;

    /* empty element content - only formating before defining child */
    str = ">\n  <e>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));

    /* empty element content is invalid - missing content terminating character < */
    str = "<e xmlns=\"urn:test\"";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Unexpected end-of-input.", NULL, 1);
    str = "xxx";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"xxx\", expected element tag start ('<').", NULL, 1);
    UTEST_LOG_CTX_CLEAN;

    /* valid strings */
    str = "<e xmlns=\"urn:test\">€𠜎Øn \n&lt;&amp;&quot;&apos;&gt; &#82;&#x4f;&#x4B;</e>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_all(tree);
    tree = NULL;

    /* test using n-bytes UTF8 hexadecimal code points */
    str = "<e xmlns=\"urn:test\" attr=\'&#x0024;&#x00A2;&#x20ac;&#x10348;\'/>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_all(tree);
    tree = NULL;

    /* CDATA value */
    str = "<e xmlns=\"urn:test\">   <![CDATA[    special non-escaped chars <>&\"'  ]]>  </e>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_all(tree);
    tree = NULL;

    /* invalid characters in string */
    str = "<e xmlns=\"urn:test\" attr=\'&#x52\'";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"'\", expected ;.", NULL, 1);

    str = "<e xmlns=\"urn:test\" attr=\"&#82\"";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character sequence \"\"\", expected ;.", NULL, 1);

    str = "<e xmlns=\"urn:test\" attr=\'&nonsense;\'/>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Entity reference \"&nonsense;\" not supported, only predefined references allowed.", NULL, 1);

    str = "<e xmlns=\"urn:test\">&#o122;";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character reference \"&#o122;\".", NULL, 1);

    str = "<e xmlns=\"urn:test\" attr=\'&#x06;\'";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character reference \"&#x06;\'\" (0x00000006).", NULL, 1);

    str = "<e xmlns=\"urn:test\" attr=\'&#xfdd0;\'";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character reference \"&#xfdd0;\'\" (0x0000fdd0).", NULL, 1);

    str = "<e xmlns=\"urn:test\" attr=\'&#xffff;\'";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, str, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Invalid character reference \"&#xffff;\'\" (0x0000ffff).", NULL, 1);
}

static void
test_ns(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    struct lyd_node_opaq *opaq1, *opaq2;
    struct lyd_attr *attr;
    const char *xml =
            "<element1 xmlns=\"urn:default\" xmlns:nc=\"urn:nc1\" nc:attr1=\"value\">\n"
            "  <element2 xmlns:nc=\"urn:nc2\" nc:attr2=\"value\"/>\n"
            "</element1>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, xml, LYD_XML,
            LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    opaq1 = (struct lyd_node_opaq *)tree;
    assert_string_equal("element1", opaq1->name.name);
    assert_string_equal("urn:default", opaq1->name.module_ns);

    attr = opaq1->attr;
    assert_non_null(attr);
    assert_string_equal("nc", attr->name.prefix);
    assert_string_equal("urn:nc1", attr->name.module_ns);

    assert_non_null(lyd_child(tree));
    opaq2 = (struct lyd_node_opaq *)lyd_child(tree);
    assert_string_equal("element2", opaq2->name.name);
    assert_string_equal("urn:default", opaq2->name.module_ns);

    attr = opaq2->attr;
    assert_non_null(attr);
    assert_string_equal("nc", attr->name.prefix);
    assert_string_equal("urn:nc2", attr->name.module_ns);

    lyd_free_all(tree);
}

static void
test_ns2(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    struct lyd_node_opaq *parent, *child;
    const char *xml =
            "<element1 xmlns=\"urn:default\">\n"
            "  <element1/>\n"
            "</element1>\n";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, xml, LYD_XML,
            LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, &tree));
    assert_non_null(tree);

    parent = (struct lyd_node_opaq *)tree;
    assert_string_equal("element1", parent->name.name);
    assert_string_equal("urn:default", parent->name.module_ns);

    assert_non_null(lyd_child(tree));
    child = (struct lyd_node_opaq *)lyd_child(tree);
    assert_string_equal("element1", child->name.name);

    assert_string_equal("urn:default", child->name.module_ns);

    lyd_free_all(tree);
}

static void
test_simple_xml(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lyd_node *tree = NULL;
    const char *test_input = "<elem1 xmlns=\"urn:test\" attr1=\"value\"> <elem2>value</elem2> </elem1>";
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " container elem1 {"
            "   leaf elem2 {"
            "     type string;"
            "   }"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yang, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, test_input, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_string_equal("elem1", tree->schema->name);
    struct lyd_node *child = lyd_child(tree);

    assert_non_null(child);
    assert_string_equal("elem2", child->schema->name);
    assert_string_equal("value", lyd_get_value(child));
    lyd_free_all(tree);
    tree = NULL;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_element),
        UTEST(test_attribute),
        UTEST(test_text),
        UTEST(test_ns),
        UTEST(test_ns2),
        UTEST(test_simple_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
