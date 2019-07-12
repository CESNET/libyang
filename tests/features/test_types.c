/*
 * @file test_parser_xml.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from parser_xml.c
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "../../src/libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

struct state_s {
    void *func;
    struct ly_ctx *ctx;
};

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
    struct state_s *s;
    const char *schema_a = "module defs {namespace urn:tests:defs;prefix d;yang-version 1.1;"
            "identity crypto-alg; identity interface-type; identity ethernet {base interface-type;} identity fast-ethernet {base ethernet;}}";
    const char *schema_b = "module types {namespace urn:tests:types;prefix t;yang-version 1.1; import defs {prefix defs;}"
            "feature f; identity gigabit-ethernet { base defs:ethernet;}"
            "container cont {leaf leaftarget {type empty;}"
                            "list listtarget {key id; max-elements 5;leaf id {type uint8;} leaf value {type string;}}"
                            "leaf-list leaflisttarget {type uint8; max-elements 5;}}"
            "list list {key id; leaf id {type string;} leaf value {type string;} leaf-list targets {type string;}}"
            "list list2 {key \"id value\"; leaf id {type string;} leaf value {type string;}}"
            "list list_inst {key id; leaf id {type instance-identifier {require-instance true;}} leaf value {type string;}}"
            "list list_ident {key id; leaf id {type identityref {base defs:interface-type;}} leaf value {type string;}}"
            "leaf-list leaflisttarget {type string;}"
            "leaf binary {type binary {length 5 {error-message \"This base64 value must be of length 5.\";}}}"
            "leaf binary-norestr {type binary;}"
            "leaf int8 {type int8 {range 10..20;}}"
            "leaf uint8 {type uint8 {range 150..200;}}"
            "leaf int16 {type int16 {range -20..-10;}}"
            "leaf uint16 {type uint16 {range 150..200;}}"
            "leaf int32 {type int32;}"
            "leaf uint32 {type uint32;}"
            "leaf int64 {type int64;}"
            "leaf uint64 {type uint64;}"
            "leaf bits {type bits {bit zero; bit one {if-feature f;} bit two;}}"
            "leaf enums {type enumeration {enum white; enum yellow {if-feature f;}}}"
            "leaf dec64 {type decimal64 {fraction-digits 1; range 1.5..10;}}"
            "leaf dec64-norestr {type decimal64 {fraction-digits 18;}}"
            "leaf str {type string {length 8..10; pattern '[a-z ]*';}}"
            "leaf str-norestr {type string;}"
            "leaf bool {type boolean;}"
            "leaf empty {type empty;}"
            "leaf ident {type identityref {base defs:interface-type;}}"
            "leaf inst {type instance-identifier {require-instance true;}}"
            "leaf inst-noreq {type instance-identifier {require-instance false;}}"
            "leaf lref {type leafref {path /leaflisttarget; require-instance true;}}"
            "leaf lref2 {type leafref {path \"../list[id = current()/../str-norestr]/targets\"; require-instance true;}}}";

    s = calloc(1, sizeof *s);
    assert_non_null(s);

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &s->ctx));
    assert_non_null(lys_parse_mem(s->ctx, schema_a, LYS_IN_YANG));
    assert_non_null(lys_parse_mem(s->ctx, schema_b, LYS_IN_YANG));

    *state = s;

    return 0;
}

static int
teardown(void **state)
{
    struct state_s *s = (struct state_s*)(*state);

#if ENABLE_LOGGER_CHECKING
    if (s->func) {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif

    ly_ctx_destroy(s->ctx, NULL);
    free(s);

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
test_int(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_int;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<int8 xmlns=\"urn:tests:types\">\n 15 \t\n  </int8>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("int8", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("15", leaf->value.canonized);
    assert_int_equal(15, leaf->value.int8);
    lyd_free_all(tree);

    /* invalid range */
    data = "<int8 xmlns=\"urn:tests:types\">1</int8>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"1\" does not satisfy the range constraint. /");

    data = "<int16 xmlns=\"urn:tests:types\">100</int16>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"100\" does not satisfy the range constraint. /");

    /* invalid value */
    data = "<int32 xmlns=\"urn:tests:types\">0x01</int32>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid int32 value \"0x01\". /");

    data = "<int64 xmlns=\"urn:tests:types\"></int64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty int64 value. /");

    data = "<int64 xmlns=\"urn:tests:types\">   </int64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty int64 value. /");

    data = "<int64 xmlns=\"urn:tests:types\">-10  xxx</int64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid int64 value \"-10  xxx\". /");

    s->func = NULL;
}

static void
test_uint(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_uint;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<uint8 xmlns=\"urn:tests:types\">\n 150 \t\n  </uint8>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("uint8", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("150", leaf->value.canonized);
    assert_int_equal(150, leaf->value.uint8);
    lyd_free_all(tree);

    /* invalid range */
    data = "<uint8 xmlns=\"urn:tests:types\">\n 15 \t\n  </uint8>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"15\" does not satisfy the range constraint. /");

    data = "<uint16 xmlns=\"urn:tests:types\">\n 1500 \t\n  </uint16>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"1500\" does not satisfy the range constraint. /");

    /* invalid value */
    data = "<uint32 xmlns=\"urn:tests:types\">-10</uint32>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"-10\" is out of uint32's min/max bounds. /");

    data = "<uint64 xmlns=\"urn:tests:types\"/>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty uint64 value. /");

    data = "<uint64 xmlns=\"urn:tests:types\">   </uint64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty uint64 value. /");

    data = "<uint64 xmlns=\"urn:tests:types\">10  xxx</uint64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid uint64 value \"10  xxx\". /");

    s->func = NULL;
}

static void
test_dec64(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_dec64;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<dec64 xmlns=\"urn:tests:types\">\n +8 \t\n  </dec64>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("8.0", leaf->value.canonized);
    assert_int_equal(80, leaf->value.dec64);
    lyd_free_all(tree);

    data = "<dec64 xmlns=\"urn:tests:types\">8.00</dec64>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("8.0", leaf->value.canonized);
    assert_int_equal(80, leaf->value.dec64);
    lyd_free_all(tree);

    data = "<dec64-norestr xmlns=\"urn:tests:types\">-9.223372036854775808</dec64-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("-9.223372036854775808", leaf->value.canonized);
    assert_int_equal(INT64_C(-9223372036854775807) - INT64_C(1), leaf->value.dec64);
    lyd_free_all(tree);

    data = "<dec64-norestr xmlns=\"urn:tests:types\">9.223372036854775807</dec64-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("9.223372036854775807", leaf->value.canonized);
    assert_int_equal(INT64_C(9223372036854775807), leaf->value.dec64);
    lyd_free_all(tree);

    /* invalid range */
    data = "<dec64 xmlns=\"urn:tests:types\">\n 15 \t\n  </dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"15.0\" does not satisfy the range constraint. /");

    data = "<dec64 xmlns=\"urn:tests:types\">\n 0 \t\n  </dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"0.0\" does not satisfy the range constraint. /");

    /* invalid value */
    data = "<dec64 xmlns=\"urn:tests:types\">xxx</dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid 1. character of decimal64 value \"xxx\". /");

    data = "<dec64 xmlns=\"urn:tests:types\"/>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty decimal64 value. /");

    data = "<dec64 xmlns=\"urn:tests:types\">   </dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty decimal64 value. /");

    data = "<dec64 xmlns=\"urn:tests:types\">8.5  xxx</dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid 6. character of decimal64 value \"8.5  xxx\". /");

    data = "<dec64 xmlns=\"urn:tests:types\">8.55  xxx</dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Value \"8.55\" of decimal64 type exceeds defined number (1) of fraction digits. /");

    s->func = NULL;
}

static void
test_string(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_string;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<str xmlns=\"urn:tests:types\">teststring</str>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("str", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("teststring", leaf->value.canonized);
    lyd_free_all(tree);

    /* invalid length */
    data = "<str xmlns=\"urn:tests:types\">short</str>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Length \"5\" does not satisfy the length constraint. /");

    data = "<str xmlns=\"urn:tests:types\">tooooo long</str>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Length \"11\" does not satisfy the length constraint. /");

    /* invalid pattern */
    data = "<str xmlns=\"urn:tests:types\">string15</str>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("String \"string15\" does not conforms to the 1. pattern restriction of its type. /");

    s->func = NULL;
}

static void
test_bits(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_bits;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<bits xmlns=\"urn:tests:types\">\n two    \t\nzero\n  </bits>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bits", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("zero two", leaf->value.canonized);
    lyd_free_all(tree);

    data = "<bits xmlns=\"urn:tests:types\">zero  two</bits>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bits", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("zero two", leaf->value.canonized);
    lyd_free_all(tree);

    /* disabled feature */
    data = "<bits xmlns=\"urn:tests:types\"> \t one \n\t </bits>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Bit \"one\" is disabled by its 1. if-feature condition. /");

    /* enable that feature */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(ly_ctx_get_module(s->ctx, "types", NULL), "f"));
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bits", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("one", leaf->value.canonized);
    lyd_free_all(tree);

    /* multiple instances of the bit */
    data = "<bits xmlns=\"urn:tests:types\">one zero one</bits>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Bit \"one\" used multiple times. /");

    /* invalid bit value */
    data = "<bits xmlns=\"urn:tests:types\">one xero one</bits>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid bit value \"xero\". /");

    s->func = NULL;
}

static void
test_enums(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_enums;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<enums xmlns=\"urn:tests:types\">white</enums>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("enums", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("white", leaf->value.canonized);
    lyd_free_all(tree);

    /* disabled feature */
    data = "<enums xmlns=\"urn:tests:types\">yellow</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Enumeration \"yellow\" is disabled by its 1. if-feature condition. /");

    /* enable that feature */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(ly_ctx_get_module(s->ctx, "types", NULL), "f"));
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("enums", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("yellow", leaf->value.canonized);
    lyd_free_all(tree);

    /* leading/trailing whitespaces are not valid */
    data = "<enums xmlns=\"urn:tests:types\"> white</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid enumeration value \" white\". /");
    data = "<enums xmlns=\"urn:tests:types\">white\n</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid enumeration value \"white\n\". /");

    /* invalid enumeration value */
    data = "<enums xmlns=\"urn:tests:types\">black</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid enumeration value \"black\". /");

    s->func = NULL;
}

static void
test_binary(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_binary;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<binary xmlns=\"urn:tests:types\">\n   aGVs\nbG8=  \t\n  </binary>"
                       "<binary-norestr xmlns=\"urn:tests:types\">TQ==</binary-norestr>";

    /* valid data (hello) */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("aGVs\nbG8=", leaf->value.canonized);
    assert_non_null(tree = tree->next);
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("TQ==", leaf->value.canonized);
    lyd_free_all(tree);

    /* no data */
    data = "<binary-norestr xmlns=\"urn:tests:types\">\n    \t\n  </binary-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonized);
    lyd_free_all(tree);
    data = "<binary-norestr xmlns=\"urn:tests:types\"></binary-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonized);
    lyd_free_all(tree);
    data = "<binary-norestr xmlns=\"urn:tests:types\"/>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonized);
    lyd_free_all(tree);

    /* invalid base64 character */
    data = "<binary-norestr xmlns=\"urn:tests:types\">a@bcd=</binary-norestr>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid Base64 character (@). /");

    /* missing data */
    data = "<binary-norestr xmlns=\"urn:tests:types\">aGVsbG8</binary-norestr>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Base64 encoded value length must be divisible by 4. /");
    data = "<binary-norestr xmlns=\"urn:tests:types\">VsbG8=</binary-norestr>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Base64 encoded value length must be divisible by 4. /");

    /* invalid binary length */
    data = "<binary xmlns=\"urn:tests:types\">aGVsbG93b3JsZA==</binary>"; /* helloworld */
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("This base64 value must be of length 5. /");
    data = "<binary xmlns=\"urn:tests:types\">TQ==</binary>"; /* M */
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("This base64 value must be of length 5. /");

    s->func = NULL;
}

static void
test_boolean(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_boolean;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<bool xmlns=\"urn:tests:types\">true</bool>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bool", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("true", leaf->value.canonized);
    assert_int_equal(1, leaf->value.boolean);
    lyd_free_all(tree);

    data = "<bool xmlns=\"urn:tests:types\">false</bool>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bool", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("false", leaf->value.canonized);
    assert_int_equal(0, leaf->value.boolean);
    lyd_free_all(tree);

    /* invalid value */
    data = "<bool xmlns=\"urn:tests:types\">unsure</bool>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid boolean value \"unsure\". /");

    data = "<bool xmlns=\"urn:tests:types\"> true</bool>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid boolean value \" true\". /");

    s->func = NULL;
}

static void
test_empty(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_empty;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<empty xmlns=\"urn:tests:types\"></empty>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("empty", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonized);
    lyd_free_all(tree);

    data = "<empty xmlns=\"urn:tests:types\"/>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("empty", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonized);
    lyd_free_all(tree);

    /* invalid value */
    data = "<empty xmlns=\"urn:tests:types\">x</empty>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty value \"x\". /");

    data = "<empty xmlns=\"urn:tests:types\"> </empty>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid empty value \" \". /");

    s->func = NULL;
}

static void
test_identityref(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_identityref;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<ident xmlns=\"urn:tests:types\">gigabit-ethernet</ident>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("ident", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("gigabit-ethernet", leaf->value.canonized);
    lyd_free_all(tree);

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:fast-ethernet</ident>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("ident", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("fast-ethernet", leaf->value.canonized);
    lyd_free_all(tree);

    /* invalid value */
    data = "<ident xmlns=\"urn:tests:types\">fast-ethernet</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid identityref \"fast-ethernet\" value - identity not found. /");

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:slow-ethernet</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid identityref \"x:slow-ethernet\" value - identity not found. /");

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:crypto-alg</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid identityref \"x:crypto-alg\" value - identity not accepted by the type specification. /");

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:unknown\">x:fast-ethernet</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid identityref \"x:fast-ethernet\" value - unable to map prefix to YANG schema. /");

    s->func = NULL;
}

/* dummy get_prefix callback for test_instanceid() */
const struct lys_module *
test_instanceid_getprefix(struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *private)
{
    (void)ctx;
    (void)prefix;
    (void)prefix_len;

    return private;
}

static void
test_instanceid(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_instanceid;

    struct lyd_node *tree;
    const struct lyd_node **trees;
    const struct lyd_node_term *leaf;

    const char *data = "<cont xmlns=\"urn:tests:types\"><leaftarget/></cont>"
            "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:cont/xdf:leaftarget</xdf:inst>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>a</id></list><list xmlns=\"urn:tests:types\"><id>b</id></list>"
           "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[xdf:id='b']/xdf:id</xdf:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<leaflisttarget xmlns=\"urn:tests:types\">1</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">2</leaflisttarget>"
           "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:leaflisttarget[.='1']</xdf:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>a</id></list><list xmlns=\"urn:tests:types\"><id>b</id><value>x</value></list>"
           "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[2]/xdf:value</xdf:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='a']</id><value>x</value></list_inst>"
           "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='b']</id><value>y</value></list_inst>"
           "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list_inst[a:id=\"/a:leaflisttarget[.='b']\"]/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[1]</id><value>x</value></list_inst>"
           "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[2]</id><value>y</value></list_inst>"
           "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list_inst[a:id=\"/a:leaflisttarget[2]\"]/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list_ident xmlns=\"urn:tests:types\"><id xmlns:dfs=\"urn:tests:defs\">dfs:ethernet</id><value>x</value></list_ident>"
           "<list_ident xmlns=\"urn:tests:types\"><id xmlns:dfs=\"urn:tests:defs\">dfs:fast-ethernet</id><value>y</value></list_ident>"
           "<a:inst xmlns:a=\"urn:tests:types\" xmlns:d=\"urn:tests:defs\">/a:list_ident[a:id='d:fast-ethernet']/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list2 xmlns=\"urn:tests:types\"><id>types:xxx</id><value>x</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>a:xxx</id><value>y</value></list2>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a:xxx'][a:value='y']/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>types:xxx</id><value>x</value></list>"
           "<list xmlns=\"urn:tests:types\"><id>a:xxx</id><value>y</value></list>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list[2]/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>a</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>c</id><value>b</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>b</value></list2>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a'][a:value='b']/a:id</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonized);
    trees = lyd_trees_new(1, tree);
    assert_non_null(leaf = lyd_target(leaf->value.target, trees));
    assert_string_equal("a", leaf->value.canonized);
    assert_string_equal("b", ((struct lyd_node_term*)leaf->next)->value.canonized);
    lyd_trees_free(trees, 1);

    /* invalid value */
    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:1leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:1leaftarget\" value at character 11 (1leaftarget). /");

    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont:t:1leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont:t:1leaftarget\" value at character 8 (:t:1leaftarget). /");

    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:invalid/t:path</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:invalid/t:path\" value - path \"/t:cont/t:invalid\" does not exists in the YANG schema. /");

    data =  "<inst xmlns=\"urn:tests:types\" xmlns:t=\"urn:tests:invalid\">/t:cont/t:leaftarget</inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaftarget\" value - unable to map prefix \"t\" to YANG schema. /");

    data =  "<inst xmlns=\"urn:tests:types\">/cont/leaftarget</inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/cont/leaftarget\" value - all node names (/cont) MUST be qualified with explicit namespace prefix. /");

    data =  "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"TODO\" value - required instance not found. /");

    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"TODO\" value - required instance not found. /");

    data =  "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><t:inst xmlns:t=\"urn:tests:types\">/t:leaflisttarget[1</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:leaflisttarget[1\" value's predicate \"[1\" (Predicate (pos) is not terminated by ']' character.). /");

    data =  "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">/t:cont[1]</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont[1]\" value - predicate \"[1]\" for container is not accepted. /");

    data =  "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">[1]</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"[1]\" value - instance-identifier must starts with '/'. /");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont><t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[id='1']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[id='1']\" value's predicate \"[id=\" (Missing prefix of a node name.). /");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont><t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[t:id='1']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[t:id='1']\" value - key-predicate \"[t:id='1']\" is accepted only for lists, not leaf-list. /");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget><leaflisttarget>2</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[4]</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"TODO\" value - required instance not found. /");

    data =  "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[6]</t:inst-noreq>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[6]\" value - "
            "position-predicate 6 is bigger than allowed max-elements (5). /");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:value='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:value='x']\" value - "
            "node \"value\" used in key-predicate \"[t:value='x']\" must be a key. /");
    logbuf_clean();
    data =  "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:value='x']</t:inst-noreq>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:value='x']\" value - "
            "node \"value\" used in key-predicate \"[t:value='x']\" must be a key. /");
    data =  "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:x='x']</t:inst-noreq>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:x='x']\" value - "
            "path \"/t:cont/t:listtarget[t:x\" does not exists in the YANG schema. /");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[.='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[.='x']\" value - "
            "leaf-list-predicate \"[.='x']\" is accepted only for leaf-lists, not list. /");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[.='2']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"TODO\" value - required instance not found. /");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[.='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[.='x']\" value - "
            "leaf-list-predicate \"[.='x']\"'s value is invalid. /");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:id='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:id='x']\" value - "
            "key-predicate \"[t:id='x']\"'s key value is invalid. /");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:id='2']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"TODO\" value - required instance not found. /");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
           "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:leaflisttarget[1][2]</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/a:leaflisttarget[1][2]\" value - "
            "position predicate (\"[2]\") cannot be used repeatedly for a single node. /");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
           "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:leaflisttarget[.='a'][.='b']</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/a:leaflisttarget[.='a'][.='b']\" value - "
            "leaf-list-predicate (\"[.='b']\") cannot be used repeatedly for a single node. /");

    data = "<list xmlns=\"urn:tests:types\"><id>a</id><value>x</value></list>"
           "<list xmlns=\"urn:tests:types\"><id>b</id><value>y</value></list>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list[a:id='a'][a:id='b']/a:value</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/a:list[a:id='a'][a:id='b']/a:value\" value - "
                  "key \"id\" is referenced the second time in key-predicate \"[a:id='b']\". /");

    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>x</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>b</id><value>y</value></list2>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a']/a:value</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid instance-identifier \"/a:list2[a:id='a']/a:value\" value - "
                  "missing 1 key(s) for the list instance \"a:list2[a:id='a']\". /");

    /* check for validting instance-identifier with a complete data tree */
    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>a</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>c</id><value>b</value></list2>"
           "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
           "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a'][a:value='a']/a:id</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    trees = lyd_trees_new(1, tree);
    /* key-predicate */
    data = "/a:list2[a:id='a'][a:value='b']/a:id";
    assert_int_equal(LY_EVALID, lyd_value_validate(s->ctx, (const struct lyd_node_term*)tree->prev, data, strlen(data),
                                                   test_instanceid_getprefix, tree->schema->module, LYD_XML, trees));
    logbuf_assert("Invalid instance-identifier \"/a:list2[a:id='a'][a:value='b']/a:id\" value - "
                  "key-predicate \"[a:id='a'][a:value='b']\" does not match any \"list2\" instance. /");
    /* leaf-list-predicate */
    data = "/a:leaflisttarget[.='c']";
    assert_int_equal(LY_EVALID, lyd_value_validate(s->ctx, (const struct lyd_node_term*)tree->prev, data, strlen(data),
                                                   test_instanceid_getprefix, tree->schema->module, LYD_XML, trees));
    logbuf_assert("Invalid instance-identifier \"/a:leaflisttarget[.='c']\" value - "
                  "leaf-list-predicate \"[.='c']\" does not match any \"leaflisttarget\" instance. /");
    /* position predicate */
    data = "/a:list2[4]";
    assert_int_equal(LY_EVALID, lyd_value_validate(s->ctx, (const struct lyd_node_term*)tree->prev, data, strlen(data),
                                                   test_instanceid_getprefix, tree->schema->module, LYD_XML, trees));
    logbuf_assert("Invalid instance-identifier \"/a:list2[4]\" value - "
                  "position-predicate 4 is bigger than number of instances in the data tree (2). /");
    lyd_trees_free(trees, 1);

    s->func = NULL;
}


static void
test_leafref(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_leafref;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    /* types:lref: /leaflisttarget */
    /* types:lref2: ../list[id = current()/../str-norestr]/targets */

    const char *schema = "module leafrefs {yang-version 1.1; namespace urn:tests:leafrefs; prefix lr; import types {prefix t;}"
            "container c { container x {leaf x {type string;}} list l {key \"id value\"; leaf id {type string;} leaf value {type string;}"
                "leaf lr1 {type leafref {path \"../../../t:str-norestr\"; require-instance true;}}"
                "leaf lr2 {type leafref {path \"../../l[id=current()/../../../t:str-norestr][value=current()/../../../t:str-norestr]/value\"; require-instance true;}}"
                "leaf lr3 {type leafref {path \"/t:list[t:id=current ( )/../../x/x]/t:targets\";}}"
            "}}}";

    const char *data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">y</leaflisttarget>"
            "<lref xmlns=\"urn:tests:types\">y</lref>";

    /* additional schema */
    assert_non_null(lys_parse_mem(s->ctx, schema, LYS_IN_YANG));

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("lref", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("y", leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
           "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">y</lref2>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("lref2", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("y", leaf->value.canonized);
    lyd_free_all(tree);

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
           "<c xmlns=\"urn:tests:leafrefs\"><l><id>x</id><value>x</value><lr1>y</lr1></l></c>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    leaf = (struct lyd_node_term*)(lyd_node_children(lyd_node_children(tree))->prev);
    assert_int_equal(LYS_LEAF, leaf->schema->nodetype);
    assert_string_equal("lr1", leaf->schema->name);
    assert_string_equal("y", leaf->value.canonized);
    lyd_free_all(tree);

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
           "<c xmlns=\"urn:tests:leafrefs\"><l><id>y</id><value>y</value></l>"
              "<l><id>x</id><value>x</value><lr2>y</lr2></l></c>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    leaf = (struct lyd_node_term*)(lyd_node_children(lyd_node_children(tree)->prev)->prev);
    assert_int_equal(LYS_LEAF, leaf->schema->nodetype);
    assert_string_equal("lr2", leaf->schema->name);
    assert_string_equal("y", leaf->value.canonized);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>c</targets><targets>d</targets></list>"
           "<c xmlns=\"urn:tests:leafrefs\"><x><x>y</x></x>"
              "<l><id>x</id><value>x</value><lr3>c</lr3></l></c>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    tree = tree->prev;
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    leaf = (struct lyd_node_term*)(lyd_node_children(lyd_node_children(tree)->prev)->prev);
    assert_int_equal(LYS_LEAF, leaf->schema->nodetype);
    assert_string_equal("lr3", leaf->schema->name);
    assert_string_equal("c", leaf->value.canonized);
    lyd_free_all(tree);

    /* invalid value */
    data =  "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget>"
            "<lref xmlns=\"urn:tests:types\">y</lref>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid leafref value \"y\" - required instance \"/leaflisttarget\" with this value does not exists in the data tree(s). /");

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
           "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">b</lref2>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid leafref value \"b\" - required instance \"../list[id = current()/../str-norestr]/targets\" with this value does not exists in the data tree(s). /");

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
           "<lref2 xmlns=\"urn:tests:types\">b</lref2>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid leafref - required instance \"../list[id = current()/../str-norestr]\" does not exists in the data tree(s). /");

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">b</lref2>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid leafref - required instance \"../list\" does not exists in the data tree(s). /");

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>x</id><value>x</value><lr1>a</lr1></l></c>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid leafref value \"a\" - required instance \"../../../t:str-norestr\" with this value does not exists in the data tree(s). /");

    data = "<str-norestr xmlns=\"urn:tests:types\">z</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>y</id><value>y</value></l>"
              "<l><id>x</id><value>x</value><lr2>z</lr2></l></c>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    logbuf_assert("Invalid leafref - required instance \"../../l[id=current()/../../../t:str-norestr][value=current()/../../../t:str-norestr]\" "
                  "does not exists in the data tree(s). /");

    s->func = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_int, setup, teardown),
        cmocka_unit_test_setup_teardown(test_uint, setup, teardown),
        cmocka_unit_test_setup_teardown(test_dec64, setup, teardown),
        cmocka_unit_test_setup_teardown(test_string, setup, teardown),
        cmocka_unit_test_setup_teardown(test_bits, setup, teardown),
        cmocka_unit_test_setup_teardown(test_enums, setup, teardown),
        cmocka_unit_test_setup_teardown(test_binary, setup, teardown),
        cmocka_unit_test_setup_teardown(test_boolean, setup, teardown),
        cmocka_unit_test_setup_teardown(test_empty, setup, teardown),
        cmocka_unit_test_setup_teardown(test_identityref, setup, teardown),
        cmocka_unit_test_setup_teardown(test_instanceid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_leafref, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
