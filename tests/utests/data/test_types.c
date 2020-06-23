/*
 * @file test_types.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for support of YANG data types
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

#include "libyang.h"
#include "path.h"

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
            "identity crypto-alg; identity interface-type; identity ethernet {base interface-type;} identity fast-ethernet {base ethernet;}"
            "typedef iref {type identityref {base interface-type;}}}";
    const char *schema_b = "module types {namespace urn:tests:types;prefix t;yang-version 1.1; import defs {prefix defs;}"
            "feature f; identity gigabit-ethernet { base defs:ethernet;}"
            "typedef tboolean {type boolean;}"
            "typedef tempty {type empty;}"
            "container cont {leaf leaftarget {type empty;}"
                            "list listtarget {key id; max-elements 5;leaf id {type uint8;} leaf value {type string;}}"
                            "leaf-list leaflisttarget {type uint8; max-elements 5;}}"
            "list list {key id; leaf id {type string;} leaf value {type string;} leaf-list targets {type string;}}"
            "list list2 {key \"id value\"; leaf id {type string;} leaf value {type string;}}"
            "list list_inst {key id; leaf id {type instance-identifier {require-instance true;}} leaf value {type string;}}"
            "list list_ident {key id; leaf id {type identityref {base defs:interface-type;}} leaf value {type string;}}"
            "list list_keyless {config \"false\"; leaf id {type string;} leaf value {type string;}}"
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
            "leaf str-utf8 {type string{length 2..5; pattern '€*';}}"
            "leaf bool {type boolean;}"
            "leaf tbool {type tboolean;}"
            "leaf empty {type empty;}"
            "leaf tempty {type tempty;}"
            "leaf ident {type identityref {base defs:interface-type;}}"
            "leaf iref {type defs:iref;}"
            "leaf inst {type instance-identifier {require-instance true;}}"
            "leaf inst-noreq {type instance-identifier {require-instance false;}}"
            "leaf lref {type leafref {path /leaflisttarget; require-instance true;}}"
            "leaf lref2 {type leafref {path \"../list[id = current()/../str-norestr]/targets\"; require-instance true;}}"
            "leaf un1 {type union {"
              "type leafref {path /int8; require-instance true;}"
              "type union { type identityref {base defs:interface-type;} type instance-identifier {require-instance true;} }"
              "type string {length 1..20;}}}}";

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
    struct lyd_value value = {0};

    const char *data = "<int8 xmlns=\"urn:tests:types\">\n 15 \t\n  </int8>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("int8", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("15", leaf->value.canonical_cache);
    assert_int_equal(15, leaf->value.int8);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal(leaf->value.canonical_cache, value.canonical_cache);
    assert_int_equal(15, value.int8);
    value.realtype->plugin->free(s->ctx, &value);
    memset(&value, 0, sizeof value);
    lyd_free_all(tree);

    /* invalid range */
    data = "<int8 xmlns=\"urn:tests:types\">1</int8>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"1\" does not satisfy the range constraint. /types:int8");

    data = "<int16 xmlns=\"urn:tests:types\">100</int16>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"100\" does not satisfy the range constraint. /types:int16");

    /* invalid value */
    data = "<int32 xmlns=\"urn:tests:types\">0x01</int32>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid int32 value \"0x01\". /types:int32");

    data = "<int64 xmlns=\"urn:tests:types\"></int64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty int64 value. /types:int64");

    data = "<int64 xmlns=\"urn:tests:types\">   </int64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty int64 value. /types:int64");

    data = "<int64 xmlns=\"urn:tests:types\">-10  xxx</int64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid int64 value \"-10  xxx\". /types:int64");

    s->func = NULL;
}

static void
test_uint(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_uint;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    const char *data = "<uint8 xmlns=\"urn:tests:types\">\n 150 \t\n  </uint8>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("uint8", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("150", leaf->value.canonical_cache);
    assert_int_equal(150, leaf->value.uint8);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal(leaf->value.canonical_cache, value.canonical_cache);
    assert_int_equal(150, value.uint8);
    value.realtype->plugin->free(s->ctx, &value);
    memset(&value, 0, sizeof value);
    lyd_free_all(tree);

    /* invalid range */
    data = "<uint8 xmlns=\"urn:tests:types\">\n 15 \t\n  </uint8>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"15\" does not satisfy the range constraint. /types:uint8");

    data = "<uint16 xmlns=\"urn:tests:types\">\n 1500 \t\n  </uint16>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"1500\" does not satisfy the range constraint. /types:uint16");

    /* invalid value */
    data = "<uint32 xmlns=\"urn:tests:types\">-10</uint32>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"-10\" is out of uint32's min/max bounds. /types:uint32");

    data = "<uint64 xmlns=\"urn:tests:types\"/>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty uint64 value. /types:uint64");

    data = "<uint64 xmlns=\"urn:tests:types\">   </uint64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty uint64 value. /types:uint64");

    data = "<uint64 xmlns=\"urn:tests:types\">10  xxx</uint64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid uint64 value \"10  xxx\". /types:uint64");

    s->func = NULL;
}

static void
test_dec64(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_dec64;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    const char *data = "<dec64 xmlns=\"urn:tests:types\">\n +8 \t\n  </dec64>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("8.0", leaf->value.canonical_cache);
    assert_int_equal(80, leaf->value.dec64);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal(leaf->value.canonical_cache, value.canonical_cache);
    assert_int_equal(80, value.dec64);
    value.realtype->plugin->free(s->ctx, &value);
    memset(&value, 0, sizeof value);
    lyd_free_all(tree);

    data = "<dec64 xmlns=\"urn:tests:types\">8.00</dec64>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("8.0", leaf->value.canonical_cache);
    assert_int_equal(80, leaf->value.dec64);
    lyd_free_all(tree);

    data = "<dec64-norestr xmlns=\"urn:tests:types\">-9.223372036854775808</dec64-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("-9.223372036854775808", leaf->value.canonical_cache);
    assert_int_equal(INT64_C(-9223372036854775807) - INT64_C(1), leaf->value.dec64);
    lyd_free_all(tree);

    data = "<dec64-norestr xmlns=\"urn:tests:types\">9.223372036854775807</dec64-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("dec64-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("9.223372036854775807", leaf->value.canonical_cache);
    assert_int_equal(INT64_C(9223372036854775807), leaf->value.dec64);
    lyd_free_all(tree);

    /* invalid range */
    data = "<dec64 xmlns=\"urn:tests:types\">\n 15 \t\n  </dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"15.0\" does not satisfy the range constraint. /types:dec64");

    data = "<dec64 xmlns=\"urn:tests:types\">\n 0 \t\n  </dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"0.0\" does not satisfy the range constraint. /types:dec64");

    /* invalid value */
    data = "<dec64 xmlns=\"urn:tests:types\">xxx</dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid 1. character of decimal64 value \"xxx\". /types:dec64");

    data = "<dec64 xmlns=\"urn:tests:types\"/>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty decimal64 value. /types:dec64");

    data = "<dec64 xmlns=\"urn:tests:types\">   </dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty decimal64 value. /types:dec64");

    data = "<dec64 xmlns=\"urn:tests:types\">8.5  xxx</dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid 6. character of decimal64 value \"8.5  xxx\". /types:dec64");

    data = "<dec64 xmlns=\"urn:tests:types\">8.55  xxx</dec64>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Value \"8.55\" of decimal64 type exceeds defined number (1) of fraction digits. /types:dec64");

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
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("str", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("teststring", leaf->value.canonical_cache);
    lyd_free_all(tree);

    /* multibyte characters (€ encodes as 3-byte UTF8 character, length restriction is 2-5) */
    data = "<str-utf8 xmlns=\"urn:tests:types\">€€</str-utf8>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("str-utf8", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("€€", leaf->value.canonical_cache);
    lyd_free_all(tree);
    data = "<str-utf8 xmlns=\"urn:tests:types\">€</str-utf8>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Length \"1\" does not satisfy the length constraint. /types:str-utf8");
    data = "<str-utf8 xmlns=\"urn:tests:types\">€€€€€€</str-utf8>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Length \"6\" does not satisfy the length constraint. /types:str-utf8");
    data = "<str-utf8 xmlns=\"urn:tests:types\">€€x</str-utf8>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("String \"€€x\" does not conform to the pattern \"€*\". /types:str-utf8");

    /* invalid length */
    data = "<str xmlns=\"urn:tests:types\">short</str>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Length \"5\" does not satisfy the length constraint. /types:str");

    data = "<str xmlns=\"urn:tests:types\">tooooo long</str>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Length \"11\" does not satisfy the length constraint. /types:str");

    /* invalid pattern */
    data = "<str xmlns=\"urn:tests:types\">string15</str>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("String \"string15\" does not conform to the pattern \"[a-z ]*\". /types:str");

    s->func = NULL;
}

static void
test_bits(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_bits;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    const char *data = "<bits xmlns=\"urn:tests:types\">\n two    \t\nzero\n  </bits>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bits", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("zero two", leaf->value.canonical_cache);
    assert_int_equal(2, LY_ARRAY_SIZE(leaf->value.bits_items));
    assert_string_equal("zero", leaf->value.bits_items[0]->name);
    assert_string_equal("two", leaf->value.bits_items[1]->name);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal(leaf->value.canonical_cache, value.canonical_cache);
    assert_int_equal(2, LY_ARRAY_SIZE(value.bits_items));
    assert_string_equal("zero", value.bits_items[0]->name);
    assert_string_equal("two", value.bits_items[1]->name);
    value.realtype->plugin->free(s->ctx, &value);
    memset(&value, 0, sizeof value);
    lyd_free_all(tree);

    data = "<bits xmlns=\"urn:tests:types\">zero  two</bits>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bits", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("zero two", leaf->value.canonical_cache);
    lyd_free_all(tree);

    /* disabled feature */
    data = "<bits xmlns=\"urn:tests:types\"> \t one \n\t </bits>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Bit \"one\" is disabled by its 1. if-feature condition. /types:bits");

    /* enable that feature */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(ly_ctx_get_module(s->ctx, "types", NULL), "f"));
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bits", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("one", leaf->value.canonical_cache);
    assert_int_equal(1, LY_ARRAY_SIZE(leaf->value.bits_items));
    assert_string_equal("one", leaf->value.bits_items[0]->name);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal(leaf->value.canonical_cache, value.canonical_cache);
    assert_int_equal(1, LY_ARRAY_SIZE(value.bits_items));
    assert_string_equal("one", value.bits_items[0]->name);
    value.realtype->plugin->free(s->ctx, &value);
    lyd_free_all(tree);

    /* multiple instances of the bit */
    data = "<bits xmlns=\"urn:tests:types\">one zero one</bits>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Bit \"one\" used multiple times. /types:bits");

    /* invalid bit value */
    data = "<bits xmlns=\"urn:tests:types\">one xero one</bits>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid bit value \"xero\". /types:bits");

    s->func = NULL;
}

static void
test_enums(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_enums;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    const char *data = "<enums xmlns=\"urn:tests:types\">white</enums>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("enums", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("white", leaf->value.canonical_cache);
    assert_string_equal("white", leaf->value.enum_item->name);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal(leaf->value.canonical_cache, value.canonical_cache);
    assert_string_equal("white", value.enum_item->name);
    value.realtype->plugin->free(s->ctx, &value);
    lyd_free_all(tree);

    /* disabled feature */
    data = "<enums xmlns=\"urn:tests:types\">yellow</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Enumeration \"yellow\" is disabled by its 1. if-feature condition. /types:enums");

    /* enable that feature */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(ly_ctx_get_module(s->ctx, "types", NULL), "f"));
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("enums", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("yellow", leaf->value.canonical_cache);
    lyd_free_all(tree);

    /* leading/trailing whitespaces are not valid */
    data = "<enums xmlns=\"urn:tests:types\"> white</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid enumeration value \" white\". /types:enums");
    data = "<enums xmlns=\"urn:tests:types\">white\n</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid enumeration value \"white\n\". /types:enums");

    /* invalid enumeration value */
    data = "<enums xmlns=\"urn:tests:types\">black</enums>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid enumeration value \"black\". /types:enums");

    s->func = NULL;
}

static void
test_binary(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_binary;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    const char *data = "<binary xmlns=\"urn:tests:types\">\n   aGVs\nbG8=  \t\n  </binary>"
                       "<binary-norestr xmlns=\"urn:tests:types\">TQ==</binary-norestr>";

    /* valid data (hello) */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("aGVs\nbG8=", leaf->value.canonical_cache);
    assert_non_null(tree = tree->next);
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("TQ==", leaf->value.canonical_cache);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal(leaf->value.canonical_cache, value.canonical_cache);
    value.realtype->plugin->free(s->ctx, &value);
    memset(&value, 0, sizeof value);
    lyd_free_all(tree);

    /* no data */
    data = "<binary-norestr xmlns=\"urn:tests:types\">\n    \t\n  </binary-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonical_cache);
    lyd_free_all(tree);
    data = "<binary-norestr xmlns=\"urn:tests:types\"></binary-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonical_cache);
    lyd_free_all(tree);
    data = "<binary-norestr xmlns=\"urn:tests:types\"/>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonical_cache);
    lyd_free_all(tree);

    /* invalid base64 character */
    data = "<binary-norestr xmlns=\"urn:tests:types\">a@bcd=</binary-norestr>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid Base64 character (@). /types:binary-norestr");

    /* missing data */
    data = "<binary-norestr xmlns=\"urn:tests:types\">aGVsbG8</binary-norestr>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Base64 encoded value length must be divisible by 4. /types:binary-norestr");
    data = "<binary-norestr xmlns=\"urn:tests:types\">VsbG8=</binary-norestr>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Base64 encoded value length must be divisible by 4. /types:binary-norestr");

    /* invalid binary length */
    data = "<binary xmlns=\"urn:tests:types\">aGVsbG93b3JsZA==</binary>"; /* helloworld */
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("This base64 value must be of length 5. /types:binary");
    data = "<binary xmlns=\"urn:tests:types\">TQ==</binary>"; /* M */
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("This base64 value must be of length 5. /types:binary");

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
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bool", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("true", leaf->value.canonical_cache);
    assert_int_equal(1, leaf->value.boolean);
    lyd_free_all(tree);

    data = "<bool xmlns=\"urn:tests:types\">false</bool>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("bool", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("false", leaf->value.canonical_cache);
    assert_int_equal(0, leaf->value.boolean);
    lyd_free_all(tree);

    data = "<tbool xmlns=\"urn:tests:types\">false</tbool>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("tbool", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("false", leaf->value.canonical_cache);
    assert_int_equal(0, leaf->value.boolean);
    lyd_free_all(tree);

    /* invalid value */
    data = "<bool xmlns=\"urn:tests:types\">unsure</bool>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid boolean value \"unsure\". /types:bool");

    data = "<bool xmlns=\"urn:tests:types\"> true</bool>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid boolean value \" true\". /types:bool");

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
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("empty", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<empty xmlns=\"urn:tests:types\"/>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("empty", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<tempty xmlns=\"urn:tests:types\"/>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("tempty", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonical_cache);
    lyd_free_all(tree);

    /* invalid value */
    data = "<empty xmlns=\"urn:tests:types\">x</empty>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty value \"x\". /types:empty");

    data = "<empty xmlns=\"urn:tests:types\"> </empty>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid empty value \" \". /types:empty");

    s->func = NULL;
}

static const char *
test_get_prefix(const struct lys_module *mod, void *private)
{
    (void)mod;
    return (const char *)private;
}

static void
test_printed_value(const struct lyd_value *value, const char *expected_value, LYD_FORMAT format, const char *prefix)
{
    const char *str;
    int dynamic;

    assert_non_null(str = value->realtype->plugin->print(value, format, test_get_prefix, (void*)prefix, &dynamic));
    assert_string_equal(expected_value, str);
    if (dynamic) {
        free((char*)str);
    }
}

static void
test_identityref(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_identityref;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    const char *data = "<ident xmlns=\"urn:tests:types\">gigabit-ethernet</ident>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("ident", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    assert_string_equal("gigabit-ethernet", leaf->value.ident->name);
    test_printed_value(&leaf->value, "t:gigabit-ethernet", LYD_XML, "t");

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_null(value.canonical_cache);
    assert_string_equal("gigabit-ethernet", value.ident->name);
    value.realtype->plugin->free(s->ctx, &value);
    lyd_free_all(tree);

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:fast-ethernet</ident>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("ident", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    test_printed_value(&leaf->value, "d:fast-ethernet", LYD_XML, "d");
    lyd_free_all(tree);

    /* invalid value */
    data = "<ident xmlns=\"urn:tests:types\">fast-ethernet</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid identityref \"fast-ethernet\" value - identity not found. /types:ident");

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:slow-ethernet</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid identityref \"x:slow-ethernet\" value - identity not found. /types:ident");

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:crypto-alg</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid identityref \"x:crypto-alg\" value - identity not accepted by the type specification. /types:ident");

    data = "<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:unknown\">x:fast-ethernet</ident>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid identityref \"x:fast-ethernet\" value - unable to map prefix to YANG schema. /types:ident");

    s->func = NULL;
}

/* dummy get_prefix callback for test_instanceid() */
const struct lys_module *
test_instanceid_getprefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *private)
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
    const struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    const char *data = "<cont xmlns=\"urn:tests:types\"><leaftarget/></cont>"
            "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:cont/xdf:leaftarget</xdf:inst>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    assert_int_equal(2, LY_ARRAY_SIZE(leaf->value.target));
    assert_string_equal("cont", leaf->value.target[0].node->name);
    assert_null(leaf->value.target[0].predicates);
    assert_string_equal("leaftarget", leaf->value.target[1].node->name);
    assert_null(leaf->value.target[1].predicates);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_null(value.canonical_cache);
    assert_true(LY_ARRAY_SIZE(leaf->value.target) == LY_ARRAY_SIZE(value.target));
    assert_true(leaf->value.target[0].node == value.target[0].node);
    assert_true(leaf->value.target[0].predicates == value.target[0].predicates); /* NULL */
    assert_true(leaf->value.target[1].node == value.target[1].node);
    assert_true(leaf->value.target[1].predicates == value.target[1].predicates); /* NULL */
    value.realtype->plugin->free(s->ctx, &value);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>a</id></list><list xmlns=\"urn:tests:types\"><id>b</id></list>"
           "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[xdf:id='b']/xdf:id</xdf:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<leaflisttarget xmlns=\"urn:tests:types\">1</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">2</leaflisttarget>"
           "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:leaflisttarget[.='1']</xdf:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='a']</id><value>x</value></list_inst>"
           "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='b']</id><value>y</value></list_inst>"
           "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list_inst[a:id=\"/a:leaflisttarget[.='b']\"]/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    assert_int_equal(2, LY_ARRAY_SIZE(leaf->value.target));
    assert_string_equal("list_inst", leaf->value.target[0].node->name);
    assert_int_equal(1, LY_ARRAY_SIZE(leaf->value.target[0].predicates));
    assert_string_equal("value", leaf->value.target[1].node->name);
    assert_null(leaf->value.target[1].predicates);

    test_printed_value(&leaf->value, "/t:list_inst[t:id=\"/t:leaflisttarget[.='b']\"]/t:value", LYD_XML, "t");
    test_printed_value(&leaf->value, "/types:list_inst[id=\"/types:leaflisttarget[.='b']\"]/value", LYD_JSON, "types");

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_null(value.canonical_cache);
    assert_true(LY_ARRAY_SIZE(leaf->value.target) == LY_ARRAY_SIZE(value.target));
    assert_true(leaf->value.target[0].node == value.target[0].node);
    assert_true(LY_ARRAY_SIZE(leaf->value.target[0].predicates) == LY_ARRAY_SIZE(value.target[0].predicates));
    assert_true(leaf->value.target[1].node == value.target[1].node);
    assert_true(leaf->value.target[1].predicates == value.target[1].predicates); /* NULL */
    value.realtype->plugin->free(s->ctx, &value);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>a</id></list><list xmlns=\"urn:tests:types\"><id>b</id><value>x</value></list>"
           "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[xdf:id='b']/xdf:value</xdf:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='a']</id><value>x</value></list_inst>"
           "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='b']</id><value>y</value></list_inst>"
           "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list_inst[a:id=\"/a:leaflisttarget[.='a']\"]/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<list_ident xmlns=\"urn:tests:types\"><id xmlns:dfs=\"urn:tests:defs\">dfs:ethernet</id><value>x</value></list_ident>"
           "<list_ident xmlns=\"urn:tests:types\"><id xmlns:dfs=\"urn:tests:defs\">dfs:fast-ethernet</id><value>y</value></list_ident>"
           "<a:inst xmlns:a=\"urn:tests:types\" xmlns:d=\"urn:tests:defs\">/a:list_ident[a:id='d:fast-ethernet']/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<list2 xmlns=\"urn:tests:types\"><id>types:xxx</id><value>x</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>a:xxx</id><value>y</value></list2>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a:xxx'][a:value='y']/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>types:xxx</id><value>x</value></list>"
           "<list xmlns=\"urn:tests:types\"><id>a:xxx</id><value>y</value></list>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list[a:id='a:xxx']/a:value</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("inst", tree->schema->name);
    leaf = (const struct lyd_node_term*)tree;
    assert_null(leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>a</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>c</id><value>b</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>b</value></list2>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a'][a:value='b']/a:id</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    leaf = (const struct lyd_node_term*)tree->prev->prev;
    assert_int_equal(LYS_LEAF, leaf->schema->nodetype);
    assert_string_equal("inst", leaf->schema->name);
    assert_null(leaf->value.canonical_cache);
    assert_non_null(leaf = lyd_target(leaf->value.target, tree));
    assert_string_equal("a", leaf->value.canonical_cache);
    assert_string_equal("b", ((struct lyd_node_term*)leaf->next)->value.canonical_cache);
    lyd_free_all(tree);

    /* invalid value */
    data = "<list xmlns=\"urn:tests:types\"><id>a</id></list><list xmlns=\"urn:tests:types\"><id>b</id><value>x</value></list>"
           "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[2]/xdf:value</xdf:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/xdf:list[2]/xdf:value\" value - semantic error. /types:inst");

    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:1leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:1leaftarget\" value - syntax error. /types:inst");

    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont:t:1leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont:t:1leaftarget\" value - syntax error. /types:inst");

    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:invalid/t:path</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:invalid/t:path\" value - semantic error. /types:inst");

    data =  "<inst xmlns=\"urn:tests:types\" xmlns:t=\"urn:tests:invalid\">/t:cont/t:leaftarget</inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaftarget\" value - semantic error. /types:inst");

    data =  "<inst xmlns=\"urn:tests:types\">/cont/leaftarget</inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/cont/leaftarget\" value - syntax error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    /* instance-identifier is here in JSON format because it is already in internal representation without original prefixes */
    logbuf_assert("Invalid instance-identifier \"/types:cont/leaftarget\" value - required instance not found. /types:inst");

    data =  "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaftarget</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    /* instance-identifier is here in JSON format because it is already in internal representation without original prefixes */
    logbuf_assert("Invalid instance-identifier \"/types:cont/leaftarget\" value - required instance not found. /types:inst");

    data =  "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><t:inst xmlns:t=\"urn:tests:types\">/t:leaflisttarget[1</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:leaflisttarget[1\" value - syntax error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">/t:cont[1]</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont[1]\" value - semantic error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">[1]</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"[1]\" value - syntax error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont><t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[id='1']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[id='1']\" value - syntax error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
        "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[t:id='1']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[t:id='1']\" value - semantic error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget><leaflisttarget>2</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[4]</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[4]\" value - semantic error. /types:inst");

    data =  "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[6]</t:inst-noreq>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[6]\" value - semantic error. /types:inst-noreq");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:value='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:value='x']\" value - semantic error. /types:inst");
    logbuf_clean();
    data =  "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:value='x']</t:inst-noreq>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:value='x']\" value - semantic error. /types:inst-noreq");
    data =  "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:x='x']</t:inst-noreq>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:x='x']\" value - semantic error. /types:inst-noreq");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[.='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[.='x']\" value - semantic error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[.='2']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    /* instance-identifier is here in JSON format because it is already in internal representation without original prefixes */
    logbuf_assert("Invalid instance-identifier \"/types:cont/leaflisttarget[.='2']\" value - required instance not found. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[.='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:leaflisttarget[.='x']\" value - semantic error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:id='x']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/t:cont/t:listtarget[t:id='x']\" value - semantic error. /types:inst");

    data =  "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:id='2']</t:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    /* instance-identifier is here in JSON format because it is already in internal representation without original prefixes */
    logbuf_assert("Invalid instance-identifier \"/types:cont/listtarget[id='2']\" value - required instance not found. /types:inst");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
           "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:leaflisttarget[1][2]</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/a:leaflisttarget[1][2]\" value - syntax error. /types:inst");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
           "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:leaflisttarget[.='a'][.='b']</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/a:leaflisttarget[.='a'][.='b']\" value - syntax error. /types:inst");

    data = "<list xmlns=\"urn:tests:types\"><id>a</id><value>x</value></list>"
           "<list xmlns=\"urn:tests:types\"><id>b</id><value>y</value></list>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list[a:id='a'][a:id='b']/a:value</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/a:list[a:id='a'][a:id='b']/a:value\" value - syntax error. /types:inst");

    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>x</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>b</id><value>y</value></list2>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a']/a:value</a:inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/a:list2[a:id='a']/a:value\" value - semantic error. /types:inst");

    /* check for validting instance-identifier with a complete data tree */
    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>a</value></list2>"
           "<list2 xmlns=\"urn:tests:types\"><id>c</id><value>b</value></list2>"
           "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
           "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a'][a:value='a']/a:id</a:inst>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    /* key-predicate */
    data = "/a:list2[a:id='a'][a:value='b']/a:id";
    assert_int_equal(LY_ENOTFOUND, lyd_value_validate(s->ctx, (const struct lyd_node_term*)tree->prev->prev, data, strlen(data),
                                                   test_instanceid_getprefix, tree->schema->module, LYD_XML, tree));
    logbuf_assert("Invalid instance-identifier \"/a:list2[a:id='a'][a:value='b']/a:id\" value - instance not found. /");
    /* leaf-list-predicate */
    data = "/a:leaflisttarget[.='c']";
    assert_int_equal(LY_ENOTFOUND, lyd_value_validate(s->ctx, (const struct lyd_node_term*)tree->prev->prev, data, strlen(data),
                                                   test_instanceid_getprefix, tree->schema->module, LYD_XML, tree));
    logbuf_assert("Invalid instance-identifier \"/a:leaflisttarget[.='c']\" value - instance not found. /");
    /* position predicate */
    data = "/a:list_keyless[4]";
    assert_int_equal(LY_ENOTFOUND, lyd_value_validate(s->ctx, (const struct lyd_node_term*)tree->prev->prev, data, strlen(data),
                                                   test_instanceid_getprefix, tree->schema->module, LYD_XML, tree));
    logbuf_assert("Invalid instance-identifier \"/a:list_keyless[4]\" value - instance not found. /");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
           "<inst xmlns=\"urn:tests:types\">/a:leaflisttarget[1]</inst>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid instance-identifier \"/a:leaflisttarget[1]\" value - semantic error. /types:inst");
    lyd_free_siblings(tree);

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
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("lref", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("y", leaf->value.canonical_cache);
    assert_int_equal(LY_TYPE_STRING, leaf->value.realtype->plugin->type);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
           "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">y</lref2>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("lref2", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("y", leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
           "<c xmlns=\"urn:tests:leafrefs\"><l><id>x</id><value>x</value><lr1>y</lr1></l></c>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev;
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    leaf = (struct lyd_node_term*)(lyd_node_children(lyd_node_children(tree, 0), 0)->prev);
    assert_int_equal(LYS_LEAF, leaf->schema->nodetype);
    assert_string_equal("lr1", leaf->schema->name);
    assert_string_equal("y", leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
           "<c xmlns=\"urn:tests:leafrefs\"><l><id>y</id><value>y</value></l>"
              "<l><id>x</id><value>x</value><lr2>y</lr2></l></c>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev;
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    leaf = (struct lyd_node_term*)(lyd_node_children(lyd_node_children(tree, 0)->prev->prev, 0)->prev);
    assert_int_equal(LYS_LEAF, leaf->schema->nodetype);
    assert_string_equal("lr2", leaf->schema->name);
    assert_string_equal("y", leaf->value.canonical_cache);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>c</targets><targets>d</targets></list>"
           "<c xmlns=\"urn:tests:leafrefs\"><x><x>y</x></x>"
              "<l><id>x</id><value>x</value><lr3>c</lr3></l></c>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev;
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    leaf = (struct lyd_node_term*)(lyd_node_children(lyd_node_children(tree, 0)->prev, 0)->prev);
    assert_int_equal(LYS_LEAF, leaf->schema->nodetype);
    assert_string_equal("lr3", leaf->schema->name);
    assert_string_equal("c", leaf->value.canonical_cache);
    lyd_free_all(tree);

    /* invalid value */
    data =  "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget>"
            "<lref xmlns=\"urn:tests:types\">y</lref>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid leafref value \"y\" - no target instance \"/leaflisttarget\" with the same value. /types:lref");

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
           "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">b</lref2>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid leafref value \"b\" - no target instance \"../list[id = current()/../str-norestr]/targets\" with"
        " the same value. /types:lref2");

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
           "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
           "<lref2 xmlns=\"urn:tests:types\">b</lref2>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid leafref value \"b\" - no target instance \"../list[id = current()/../str-norestr]/targets\""
        " with the same value. /types:lref2");

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">b</lref2>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid leafref value \"b\" - no target instance \"../list[id = current()/../str-norestr]/targets\""
        " with the same value. /types:lref2");

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>x</id><value>x</value><lr1>a</lr1></l></c>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid leafref value \"a\" - no target instance \"../../../t:str-norestr\" with the same value."
        " /leafrefs:c/l[id='x'][value='x']/lr1");

    data = "<str-norestr xmlns=\"urn:tests:types\">z</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>y</id><value>y</value></l>"
              "<l><id>x</id><value>x</value><lr2>z</lr2></l></c>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid leafref value \"z\" - no target instance \"../../l[id=current()/../../../t:str-norestr]"
        "[value=current()/../../../t:str-norestr]/value\" with the same value. /leafrefs:c/l[id='x'][value='x']/lr2");

    s->func = NULL;
}

static void
test_union(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_union;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    struct lyd_value value = {0};

    /*
     * leaf un1 {type union {
     *             type leafref {path /int8; require-instance true;}
     *             type union {
     *               type identityref {base defs:interface-type;}
     *               type instance-identifier {require-instance true;}
     *             }
     *             type string {range 1..20;};
     *           }
     * }
     */

    const char *data = "<int8 xmlns=\"urn:tests:types\">12</int8>"
            "<un1 xmlns=\"urn:tests:types\">12</un1>";

    /* valid data */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->next;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("un1", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("12", leaf->value.original);
    assert_null(leaf->value.canonical_cache);
    assert_null(leaf->value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_UNION, leaf->value.realtype->basetype);
    assert_int_equal(LY_TYPE_INT8, leaf->value.subvalue->value->realtype->basetype);
    assert_string_equal("12", leaf->value.subvalue->value->canonical_cache);
    assert_int_equal(12, leaf->value.subvalue->value->int8);

    test_printed_value(&leaf->value, "12", LYD_XML, NULL);

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal("12", value.original);
    assert_null(value.canonical_cache);
    assert_null(value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_INT8, value.subvalue->value->realtype->basetype);
    assert_string_equal("12", value.subvalue->value->canonical_cache);
    assert_int_equal(12, leaf->value.subvalue->value->int8);
    value.realtype->plugin->free(s->ctx, &value);
    lyd_free_all(tree);

    data = "<int8 xmlns=\"urn:tests:types\">12</int8>"
           "<un1 xmlns=\"urn:tests:types\">2</un1>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->next;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("un1", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("2", leaf->value.original);
    assert_null(leaf->value.canonical_cache);
    assert_null(leaf->value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_UNION, leaf->value.realtype->basetype);
    assert_int_equal(LY_TYPE_STRING, leaf->value.subvalue->value->realtype->basetype);
    assert_string_equal("2", leaf->value.subvalue->value->canonical_cache);
    lyd_free_all(tree);

    data = "<un1 xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:fast-ethernet</un1>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("un1", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("x:fast-ethernet", leaf->value.original);
    assert_null(leaf->value.canonical_cache);
    assert_non_null(leaf->value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_UNION, leaf->value.realtype->basetype);
    assert_int_equal(LY_TYPE_IDENT, leaf->value.subvalue->value->realtype->basetype);
    assert_null(leaf->value.subvalue->value->canonical_cache); /* identityref does not have canonical form */

    test_printed_value(&leaf->value, "x:fast-ethernet", LYD_XML, "x");
    test_printed_value(leaf->value.subvalue->value, "d:fast-ethernet", LYD_XML, "d");

    value.realtype = leaf->value.realtype;
    assert_int_equal(LY_SUCCESS, value.realtype->plugin->duplicate(s->ctx, &leaf->value, &value));
    assert_string_equal("x:fast-ethernet", value.original);
    assert_null(value.canonical_cache);
    assert_null(value.subvalue->value->canonical_cache);
    assert_non_null(value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_IDENT, value.subvalue->value->realtype->basetype);
    assert_string_equal("fast-ethernet", value.subvalue->value->ident->name);
    value.realtype->plugin->free(s->ctx, &value);
    lyd_free_all(tree);

    data = "<un1 xmlns=\"urn:tests:types\" xmlns:d=\"urn:tests:defs\">d:superfast-ethernet</un1>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("un1", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("d:superfast-ethernet", leaf->value.original);
    assert_null(leaf->value.canonical_cache);
    assert_non_null(leaf->value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_UNION, leaf->value.realtype->basetype);
    assert_int_equal(LY_TYPE_STRING, leaf->value.subvalue->value->realtype->basetype);
    assert_string_equal("d:superfast-ethernet", leaf->value.subvalue->value->canonical_cache);
    lyd_free_all(tree);

    data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">y</leaflisttarget>"
           "<un1 xmlns=\"urn:tests:types\" xmlns:a=\"urn:tests:types\">/a:leaflisttarget[.='y']</un1>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("un1", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("/a:leaflisttarget[.='y']", leaf->value.original);
    assert_null(leaf->value.canonical_cache);
    assert_non_null(leaf->value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_UNION, leaf->value.realtype->basetype);
    assert_int_equal(LY_TYPE_INST, leaf->value.subvalue->value->realtype->basetype);
    assert_null(leaf->value.subvalue->value->canonical_cache); /* instance-identifier does not have canonical form */
    lyd_free_all(tree);

    data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">y</leaflisttarget>"
           "<un1 xmlns=\"urn:tests:types\" xmlns:a=\"urn:tests:types\">/a:leaflisttarget[3]</un1>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    tree = tree->prev->prev;
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("un1", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("/a:leaflisttarget[3]", leaf->value.original);
    assert_null(leaf->value.canonical_cache);
    assert_non_null(leaf->value.subvalue->prefixes);
    assert_int_equal(LY_TYPE_UNION, leaf->value.realtype->basetype);
    assert_int_equal(LY_TYPE_STRING, leaf->value.subvalue->value->realtype->basetype);
    assert_string_equal("/a:leaflisttarget[3]", leaf->value.subvalue->value->canonical_cache);
    lyd_free_all(tree);

    data = "<un1 xmlns=\"urn:tests:types\">123456789012345678901</un1>";
    assert_null(lyd_parse_mem(s->ctx, data, LYD_XML, LYD_VALOPT_DATA_ONLY));
    logbuf_assert("Invalid union value \"123456789012345678901\" - no matching subtype found. /types:un1");

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
        cmocka_unit_test_setup_teardown(test_union, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
