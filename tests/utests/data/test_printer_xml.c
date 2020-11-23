/*
 * @file test_printer_xml.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from printer_yang.c
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <string.h>

#include "context.h"
#include "out.h"
#include "parser_data.h"
#include "printer_data.h"
#include "tests/config.h"
#include "tree_schema.h"
#include "utests.h"

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
            "    list listtarget {key id; max-elements 5;leaf id {type uint8;} leaf value {type string;}"
            "        action test {input {leaf a {type string;}} output {leaf b {type string;}}}}"
            "    leaf-list leaflisttarget {type uint8; max-elements 5;}}"
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
            "leaf lref2 {type leafref {path \"../list[id = current()/../str-norestr]/targets\"; require-instance true;}}"
            "leaf un1 {type union {"
            "    type leafref {path /int8; require-instance true;}"
            "    type union { type identityref {base defs:interface-type;} type instance-identifier {require-instance true;} }"
            "    type string {length 1..20;}}}"
            "anydata any;"
            "rpc sum {input {leaf x {type uint8;} leaf y {type uint8;}} output {leaf result {type uint16;}}}}";
    const char *schema_c =
            "module defaults {\n"
            "    namespace \"urn:defaults\";\n"
            "    prefix d;\n"
            "    leaf a {\n"
            "        type union {\n"
            "            type instance-identifier;\n"
            "            type string;\n"
            "        }\n"
            "        default \"/d:b\";\n"
            "    }\n"
            "    leaf b {\n"
            "        type string;\n"
            "    }\n"
            "    leaf c {\n"
            "        type string;\n"
            "    }\n"
            "}";

    s = calloc(1, sizeof *s);
    assert_non_null(s);

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &s->ctx));
    assert_non_null(ly_ctx_load_module(s->ctx, "ietf-netconf-with-defaults", "2011-06-01", NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(s->ctx, schema_a, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(s->ctx, schema_b, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(s->ctx, schema_c, LYS_IN_YANG, NULL));

    *state = s;

    return 0;
}

static int
teardown(void **state)
{
    struct state_s *s = (struct state_s *)(*state);

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
test_leaf(void **state)
{
    struct state_s *s = (struct state_s *)(*state);
    struct lyd_node *tree;
    const char *data;
    const char *result;
    char *printed;
    struct ly_out *out;

    s->func = test_leaf;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    data = "<int8 xmlns=\"urn:tests:types\">\n 15 \t\n  </int8>";
    result = "<int8 xmlns=\"urn:tests:types\">15</int8>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(s->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_int_equal(LY_SUCCESS, lyd_print_tree(out, tree->next, LYD_XML, LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    assert_string_equal(printed, result);
    lyd_free_all(tree);

    ly_out_free(out, NULL, 1);
    s->func = NULL;
}

static void
test_anydata(void **state)
{
    struct state_s *s = (struct state_s *)(*state);
    struct lyd_node *tree;
    const char *data;
    char *printed;
    struct ly_out *out;

    s->func = test_anydata;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    data = "<any xmlns=\"urn:tests:types\"><somexml xmlns:x=\"url:x\" xmlns=\"example.com\"><x:x/></somexml></any>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(s->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_int_equal(LY_SUCCESS, lyd_print_tree(out, tree->next, LYD_XML, LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    /* canonized */
    data = "<any xmlns=\"urn:tests:types\"><somexml xmlns=\"example.com\"><x xmlns=\"url:x\"/></somexml></any>";
    assert_string_equal(printed, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    data = "<any xmlns=\"urn:tests:types\"/>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(s->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_int_equal(LY_SUCCESS, lyd_print_tree(out, tree->next, LYD_XML, LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    assert_string_equal(printed, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    data =
            "<any xmlns=\"urn:tests:types\">\n"
            "  <cont>\n"
            "    <defs:elem1 xmlns:defs=\"urn:tests:defs\">\n"
            "      <elem2 xmlns:defaults=\"urn:defaults\" defs:attr1=\"defaults:val\" attr2=\"/defaults:node/defs:node2\">\n"
            "      </elem2>\n"
            "    </defs:elem1>\n"
            "  </cont>\n"
            "</any>\n";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(s->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    /* cont should be normally parsed */
    tree = tree->next;
    assert_string_equal(tree->schema->name, "any");
    assert_int_equal(((struct lyd_node_any *)tree)->value_type, LYD_ANYDATA_DATATREE);
    assert_string_equal(((struct lyd_node_any *)tree)->value.tree->schema->name, "cont");
    /* but its children not */
    assert_null(((struct lyd_node_inner *)(((struct lyd_node_any *)tree)->value.tree))->child->schema);
    assert_int_equal(LY_SUCCESS, lyd_print_tree(out, tree, LYD_XML, 0));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    /* canonized */
    data =
            "<any xmlns=\"urn:tests:types\">\n"
            "  <cont>\n"
            "    <elem1 xmlns=\"urn:tests:defs\">\n"
            "      <elem2 xmlns=\"urn:tests:types\" xmlns:defs=\"urn:tests:defs\" xmlns:defaults=\"urn:defaults\" "
            "defs:attr1=\"defaults:val\" attr2=\"/defaults:node/defs:node2\"/>\n"
            "    </elem1>\n"
            "  </cont>\n"
            "</any>\n";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    lyd_free_all(tree);

    ly_out_free(out, NULL, 1);
    s->func = NULL;
}

static void
test_defaults(void **state)
{
    struct state_s *s = (struct state_s *)(*state);
    struct lyd_node *tree;
    const char *data;
    char *printed;
    struct ly_out *out;

    s->func = test_defaults;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    /* standard default value */
    data = "<c xmlns=\"urn:defaults\">aa</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(s->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_TRIM | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    data = "<a xmlns=\"urn:defaults\" xmlns:d=\"urn:defaults\">/d:b</a><c xmlns=\"urn:defaults\">aa</c>";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_ALL_TAG | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    data = "<a xmlns=\"urn:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\""
            " ncwd:default=\"true\" xmlns:d=\"urn:defaults\">/d:b</a>"
            "<c xmlns=\"urn:defaults\">aa</c>";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    data = "<a xmlns=\"urn:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\""
            " ncwd:default=\"true\" xmlns:d=\"urn:defaults\">/d:b</a>"
            "<c xmlns=\"urn:defaults\">aa</c>";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    lyd_free_all(tree);

    /* string value equal to the default but default is an unresolved instance-identifier, so they are not considered equal */
    data = "<a xmlns=\"urn:defaults\">/d:b</a><c xmlns=\"urn:defaults\">aa</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(s->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_TRIM | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_ALL_TAG | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    assert_string_equal(printed, data);
    ly_out_reset(out);

    lyd_free_all(tree);

    /* instance-identifier value equal to the default, should be considered equal */
    data = "<a xmlns=\"urn:defaults\" xmlns:d=\"urn:defaults\">/d:b</a><b xmlns=\"urn:defaults\">val</b><c xmlns=\"urn:defaults\">aa</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(s->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_TRIM | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    data = "<b xmlns=\"urn:defaults\">val</b><c xmlns=\"urn:defaults\">aa</c>";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    data = "<a xmlns=\"urn:defaults\" xmlns:d=\"urn:defaults\">/d:b</a><b xmlns=\"urn:defaults\">val</b><c xmlns=\"urn:defaults\">aa</c>";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_ALL_TAG | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    data = "<a xmlns=\"urn:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\""
            " ncwd:default=\"true\" xmlns:d=\"urn:defaults\">/d:b</a>"
            "<b xmlns=\"urn:defaults\">val</b>"
            "<c xmlns=\"urn:defaults\">aa</c>";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_SHRINK));
    assert_int_equal(strlen(printed), ly_out_printed(out));
    data = "<a xmlns=\"urn:defaults\" xmlns:d=\"urn:defaults\">/d:b</a><b xmlns=\"urn:defaults\">val</b><c xmlns=\"urn:defaults\">aa</c>";
    assert_string_equal(printed, data);
    ly_out_reset(out);

    lyd_free_all(tree);
    ly_out_free(out, NULL, 1);

    s->func = NULL;
}

#if 0

static void
test_rpc(void **state)
{
    struct state_s *s = (struct state_s *)(*state);
    struct lyd_node *tree1;
    struct lyd_node *tree2;
    const struct lyd_node **trees;
    const char *request;
    const char *reply, *result;
    char *printed;
    ssize_t len;
    struct ly_out *out;

    s->func = test_rpc;
    assert_non_null(out = ly_out_new_memory(&printed, 0));

    request = "<sum xmlns=\"urn:tests:types\"><x>10</x><y>20</y></sum>";
    reply = "<result xmlns=\"urn:tests:types\">30</result>";
    result = "<sum xmlns=\"urn:tests:types\"><result>30</result></sum>";
    assert_non_null(tree1 = lyd_parse_mem(s->ctx, request, LYD_XML, LYD_OPT_RPC, NULL));
    assert_true((len = lyd_print_tree(out, tree1, LYD_XML, LYD_PRINT_SHRINK)) >= 0);
    assert_int_equal(len, strlen(printed));
    assert_string_equal(printed, request);
    ly_out_reset(out);
    assert_non_null(trees = lyd_trees_new(1, tree1));
    assert_non_null(tree2 = lyd_parse_mem(s->ctx, reply, LYD_XML, LYD_OPT_RPCREPLY, trees));
    assert_true((len = lyd_print_tree(out, tree2, LYD_XML, LYD_PRINT_SHRINK)) >= 0);
    assert_int_equal(len, strlen(printed));
    assert_string_equal(printed, result);
    ly_out_reset(out);
    lyd_trees_free(trees, 0);
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* no arguments */
    request = "<sum xmlns=\"urn:tests:types\"/>";
    reply = "";
    result = "<sum xmlns=\"urn:tests:types\"/>";
    assert_non_null(tree1 = lyd_parse_mem(s->ctx, request, LYD_XML, LYD_OPT_RPC, NULL));
    assert_true((len = lyd_print_tree(out, tree1, LYD_XML, LYD_PRINT_SHRINK)) >= 0);
    assert_int_equal(len, strlen(printed));
    assert_string_equal(printed, request);
    ly_out_reset(out);
    assert_non_null(trees = lyd_trees_new(1, tree1));
    assert_non_null(tree2 = lyd_parse_mem(s->ctx, reply, LYD_XML, LYD_OPT_RPCREPLY, trees));
    assert_true((len = lyd_print_tree(out, tree2, LYD_XML, LYD_PRINT_SHRINK)) >= 0);
    assert_int_equal(len, strlen(printed));
    assert_string_equal(printed, result);
    ly_out_reset(out);
    lyd_trees_free(trees, 0);
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* action
     * "container cont {leaf leaftarget {type empty;}"
                            "list listtarget {key id; max-elements 5;leaf id {type uint8;} leaf value {type string;}"
                                             "action test {input {leaf a {type string;}} output {leaf b {type string;}}}}"
                            "leaf-list leaflisttarget {type uint8; max-elements 5;}}"
                            */
    request = "<cont xmlns=\"urn:tests:types\"><listtarget><id>10</id><test><a>test</a></test></listtarget></cont>";
    reply = "<b xmlns=\"urn:tests:types\">test-reply</b>";
    result = "<cont xmlns=\"urn:tests:types\"><listtarget><id>10</id><test><b>test-reply</b></test></listtarget></cont>";
    assert_non_null(tree1 = lyd_parse_mem(s->ctx, request, LYD_XML, LYD_OPT_RPC, NULL));
    assert_true((len = lyd_print_tree(out, tree1, LYD_XML, LYD_PRINT_SHRINK)) >= 0);
    assert_int_equal(len, strlen(printed));
    assert_string_equal(printed, request);
    ly_out_reset(out);
    assert_non_null(trees = lyd_trees_new(1, tree1));
    assert_non_null(tree2 = lyd_parse_mem(s->ctx, reply, LYD_XML, LYD_OPT_RPCREPLY, trees));
    assert_true((len = lyd_print_tree(out, tree2, LYD_XML, LYD_PRINT_SHRINK)) >= 0);
    assert_int_equal(len, strlen(printed));
    assert_string_equal(printed, result);
    ly_out_reset(out);
    lyd_trees_free(trees, 0);
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    ly_out_free(out, NULL, 1);
    s->func = NULL;
}

#endif

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_leaf, setup, teardown),
        cmocka_unit_test_setup_teardown(test_anydata, setup, teardown),
        cmocka_unit_test_setup_teardown(test_defaults, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
