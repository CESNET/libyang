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

#include "context.h"
#include "parser.h"
#include "parser_data.h"
#include "printer.h"
#include "printer_data.h"
#include "tests/config.h"
#include "tree_data_internal.h"
#include "tree_schema.h"

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

    const char *schema_a = "module a {namespace urn:tests:a;prefix a;yang-version 1.1; import ietf-yang-metadata {prefix md;}"
            "md:annotation hint { type int8;}"
            "list l1 { key \"a b c\"; leaf a {type string;} leaf b {type string;} leaf c {type int16;} leaf d {type string;}}"
            "leaf foo { type string;}"
            "container c {"
                "leaf x {type string;}"
                "action act { input { leaf al {type string;} } output { leaf al {type uint8;} } }"
                "notification n1 { leaf nl {type string;} }"
            "}"
            "container cp {presence \"container switch\"; leaf y {type string;} leaf z {type int8;}}"
            "anydata any {config false;}"
            "leaf-list ll1 { type uint8; }"
            "leaf foo2 { type string; default \"default-val\"; }"
            "leaf foo3 { type uint32; }"
            "notification n2;}";
    const struct lys_module *mod;

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &ctx));
    assert_non_null(ly_ctx_load_module(ctx, "ietf-netconf-with-defaults", "2011-06-01"));
    assert_non_null((mod = ly_ctx_load_module(ctx, "ietf-netconf", "2011-06-01")));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "writable-running"));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_a, LYS_IN_YANG, NULL));

    return 0;
}

static int
teardown(void **state)
{
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
#else
    (void) state; /* unused */
#endif

    ly_ctx_destroy(ctx, NULL);
    ctx = NULL;

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
    *state = test_leaf;

    const char *data = "{\"a:foo\":\"foo value\"}";
    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    char *printed;
    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("foo", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("foo value", leaf->value.canonical);

    assert_int_equal(LYS_LEAF, tree->next->next->schema->nodetype);
    assert_string_equal("foo2", tree->next->next->schema->name);
    leaf = (struct lyd_node_term*)tree->next->next;
    assert_string_equal("default-val", leaf->value.canonical);
    assert_true(leaf->flags & LYD_DEFAULT);

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* make foo2 explicit */
    data = "{\"a:foo2\":\"default-val\"}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("foo2", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("default-val", leaf->value.canonical);
    assert_false(leaf->flags & LYD_DEFAULT);

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* parse foo2 but make it implicit */
    data = "{\"a:foo2\":\"default-val\",\"@a:foo2\":{\"ietf-netconf-with-defaults:default\":true}}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("foo2", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("default-val", leaf->value.canonical);
    assert_true(leaf->flags & LYD_DEFAULT);

    /* TODO default values
    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_reset(out);
    */
    lyd_free_all(tree);

    /* multiple meatadata hint and unknown metadata xxx supposed to be skipped since it is from missing schema */
    data = "{\"@a:foo\":{\"a:hint\":1,\"a:hint\":2,\"x:xxx\":{\"value\":\"/x:no/x:yes\"}},\"a:foo\":\"xxx\"}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("foo", tree->schema->name);
    assert_non_null(tree->meta);
    assert_string_equal("hint", tree->meta->name);
    assert_int_equal(LY_TYPE_INT8, tree->meta->value.realtype->basetype);
    assert_string_equal("1", tree->meta->value.canonical);
    assert_int_equal(1, tree->meta->value.int8);
    assert_ptr_equal(tree, tree->meta->parent);
    assert_non_null(tree->meta->next);
    assert_string_equal("hint", tree->meta->next->name);
    assert_int_equal(LY_TYPE_INT8, tree->meta->next->value.realtype->basetype);
    assert_string_equal("2", tree->meta->next->value.canonical);
    assert_int_equal(2, tree->meta->next->value.int8);
    assert_ptr_equal(tree, tree->meta->next->parent);
    assert_null(tree->meta->next->next);

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, "{\"a:foo\":\"xxx\",\"@a:foo\":{\"a:hint\":1,\"a:hint\":2}}");
    ly_out_free(out, NULL, 1);
    lyd_free_all(tree);

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Unknown (or not implemented) YANG module \"x\" for metadata \"x:xxx\". /a:foo");

    /* missing referenced metadata node */
    data = "{\"@a:foo\" : { \"a:hint\" : 1 }}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Missing JSON data instance to be coupled with @a:foo metadata. /");

    /* missing namespace for meatadata*/
    data = "{\"a:foo\" : \"value\", \"@a:foo\" : { \"hint\" : 1 }}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Metadata in JSON must be namespace-qualified, missing prefix for \"hint\". /a:foo");

    *state = NULL;
}

static void
test_leaflist(void **state)
{
    *state = test_leaflist;

    const char *data = "{\"a:ll1\":[10,11]}";
    struct lyd_node *tree;
    struct lyd_node_term *ll;

    char *printed;
    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAFLIST, tree->schema->nodetype);
    assert_string_equal("ll1", tree->schema->name);
    ll = (struct lyd_node_term*)tree;
    assert_string_equal("10", ll->value.canonical);

    assert_non_null(tree->next);
    assert_int_equal(LYS_LEAFLIST, tree->next->schema->nodetype);
    assert_string_equal("ll1", tree->next->schema->name);
    ll = (struct lyd_node_term*)tree->next;
    assert_string_equal("11", ll->value.canonical);

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* simple metadata */
    data = "{\"a:ll1\":[10,11],\"@a:ll1\":[null,{\"a:hint\":2}]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAFLIST, tree->schema->nodetype);
    assert_string_equal("ll1", tree->schema->name);
    ll = (struct lyd_node_term*)tree;
    assert_string_equal("10", ll->value.canonical);
    assert_null(ll->meta);

    assert_non_null(tree->next);
    assert_int_equal(LYS_LEAFLIST, tree->next->schema->nodetype);
    assert_string_equal("ll1", tree->next->schema->name);
    ll = (struct lyd_node_term*)tree->next;
    assert_string_equal("11", ll->value.canonical);
    assert_non_null(ll->meta);
    assert_string_equal("2", ll->meta->value.canonical);
    assert_null(ll->meta->next);

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_reset(out);
    lyd_free_all(tree);

        /* multiple meatadata hint and unknown metadata xxx supposed to be skipped since it is from missing schema */
    data = "{\"@a:ll1\" : [{\"a:hint\" : 1, \"x:xxx\" :  { \"value\" : \"/x:no/x:yes\" }, \"a:hint\" : 10},null,{\"a:hint\" : 3}], \"a:ll1\" : [1,2,3]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LEAFLIST, tree->schema->nodetype);
    assert_string_equal("ll1", tree->schema->name);
    ll = (struct lyd_node_term*)tree;
    assert_string_equal("1", ll->value.canonical);
    assert_non_null(ll->meta);
    assert_string_equal("hint", ll->meta->name);
    assert_int_equal(LY_TYPE_INT8, ll->meta->value.realtype->basetype);
    assert_string_equal("1", ll->meta->value.canonical);
    assert_int_equal(1, ll->meta->value.int8);
    assert_ptr_equal(ll, ll->meta->parent);
    assert_non_null(ll->meta->next);
    assert_string_equal("hint", ll->meta->next->name);
    assert_int_equal(LY_TYPE_INT8, ll->meta->next->value.realtype->basetype);
    assert_string_equal("10", ll->meta->next->value.canonical);
    assert_int_equal(10, ll->meta->next->value.int8);
    assert_ptr_equal(ll, ll->meta->next->parent);
    assert_null(ll->meta->next->next);

    assert_non_null(tree->next);
    assert_int_equal(LYS_LEAFLIST, tree->next->schema->nodetype);
    assert_string_equal("ll1", tree->next->schema->name);
    ll = (struct lyd_node_term*)tree->next;
    assert_string_equal("2", ll->value.canonical);
    assert_null(ll->meta);

    assert_non_null(tree->next->next);
    assert_int_equal(LYS_LEAFLIST, tree->next->next->schema->nodetype);
    assert_string_equal("ll1", tree->next->next->schema->name);
    ll = (struct lyd_node_term*)tree->next->next;
    assert_string_equal("3", ll->value.canonical);
    assert_non_null(ll->meta);
    assert_string_equal("hint", ll->meta->name);
    assert_int_equal(LY_TYPE_INT8, ll->meta->value.realtype->basetype);
    assert_string_equal("3", ll->meta->value.canonical);
    assert_int_equal(3, ll->meta->value.int8);
    assert_ptr_equal(ll, ll->meta->parent);
    assert_null(ll->meta->next);

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, "{\"a:ll1\":[1,2,3],\"@a:ll1\":[{\"a:hint\":1,\"a:hint\":10},null,{\"a:hint\":3}]}");
    ly_out_free(out, NULL, 1);
    lyd_free_all(tree);

    /* missing referenced metadata node */
    data = "{\"@a:ll1\":[{\"a:hint\":1}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Missing JSON data instance to be coupled with @a:ll1 metadata. /");

    data = "{\"a:ll1\":[1],\"@a:ll1\":[{\"a:hint\":1},{\"a:hint\":2}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Missing JSON data instance no. 2 of a:ll1 to be coupled with metadata. /");

    data = "{\"@a:ll1\":[{\"a:hint\":1},{\"a:hint\":2},{\"a:hint\":3}],\"a:ll1\" : [1, 2]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Missing 3rd JSON data instance to be coupled with @a:ll1 metadata. /");

    *state = NULL;
}

static void
test_anydata(void **state)
{
    *state = test_anydata;

    const char *data;
    char *str;
    struct lyd_node *tree;

    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    data = "{\"a:any\":{\"x:element1\":{\"element2\":\"/a:some/a:path\",\"list\":[{},{\"key\":\"a\"}]}}}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_ANYDATA, tree->schema->nodetype);
    assert_string_equal("any", tree->schema->name);

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);

    lyd_free_all(tree);
    ly_out_free(out, NULL, 1);

    *state = NULL;
}

static void
test_list(void **state)
{
    *state = test_list;

    const char *data = "{\"a:l1\":[{\"a\":\"one\",\"b\":\"one\",\"c\":1}]}";
    struct lyd_node *tree, *iter;
    struct lyd_node_inner *list;
    struct lyd_node_term *leaf;

    char *printed;
    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    /* check hashes */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LIST, tree->schema->nodetype);
    assert_string_equal("l1", tree->schema->name);
    list = (struct lyd_node_inner*)tree;
    LY_LIST_FOR(list->child, iter) {
        assert_int_not_equal(0, iter->hash);
    }

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* missing keys */
    data = "{ \"a:l1\": [ {\"c\" : 1, \"b\" : \"b\"}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("List instance is missing its key \"a\". /a:l1[b='b'][c='1']");

    data = "{ \"a:l1\": [ {\"a\" : \"a\"}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("List instance is missing its key \"b\". /a:l1[a='a']");

    data = "{ \"a:l1\": [ {\"b\" : \"b\", \"a\" : \"a\"}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("List instance is missing its key \"c\". /a:l1[a='a'][b='b']");

    /* key duplicate */
    data = "{ \"a:l1\": [ {\"c\" : 1, \"b\" : \"b\", \"a\" : \"a\", \"c\" : 1}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Duplicate instance of \"c\". /a:l1[a='a'][b='b'][c='1'][c='1']/c");

    /* keys order, in contrast to XML, JSON accepts keys in any order even in strict mode */
    logbuf_clean();
    data = "{ \"a:l1\": [ {\"d\" : \"d\", \"a\" : \"a\", \"c\" : 1, \"b\" : \"b\"}]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LIST, tree->schema->nodetype);
    assert_string_equal("l1", tree->schema->name);
    list = (struct lyd_node_inner*)tree;
    assert_non_null(leaf = (struct lyd_node_term*)list->child);
    assert_string_equal("a", leaf->schema->name);
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    assert_string_equal("b", leaf->schema->name);
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    assert_string_equal("c", leaf->schema->name);
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    assert_string_equal("d", leaf->schema->name);
    logbuf_assert("");

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, "{\"a:l1\":[{\"a\":\"a\",\"b\":\"b\",\"c\":1,\"d\":\"d\"}]}");
    ly_out_reset(out);
    lyd_free_all(tree);

    /*  */
    data = "{\"a:l1\":[{\"c\":1,\"b\":\"b\",\"a\":\"a\"}]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_LIST, tree->schema->nodetype);
    assert_string_equal("l1", tree->schema->name);
    list = (struct lyd_node_inner*)tree;
    assert_non_null(leaf = (struct lyd_node_term*)list->child);
    assert_string_equal("a", leaf->schema->name);
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    assert_string_equal("b", leaf->schema->name);
    assert_non_null(leaf = (struct lyd_node_term*)leaf->next);
    assert_string_equal("c", leaf->schema->name);
    logbuf_assert("");

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, "{\"a:l1\":[{\"a\":\"a\",\"b\":\"b\",\"c\":1}]}");
    ly_out_reset(out);
    lyd_free_all(tree);

    data = "{\"a:cp\":{\"@\":{\"a:hint\":1}}}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    assert_string_equal("cp", tree->schema->name);
    assert_non_null(tree->meta);
    assert_string_equal("hint", tree->meta->name);
    assert_string_equal("1", tree->meta->value.canonical);
    assert_ptr_equal(tree, tree->meta->parent);
    assert_null(tree->meta->next);

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_free(out, NULL, 1);
    lyd_free_all(tree);

    *state = NULL;
}

static void
test_container(void **state)
{
    *state = test_container;

    const char *data = "{\"a:c\":{}}";
    struct lyd_node *tree;
    struct lyd_node_inner *cont;

    char *printed;
    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    assert_string_equal("c", tree->schema->name);
    cont = (struct lyd_node_inner*)tree;
    assert_true(cont->flags & LYD_DEFAULT);

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, "{}");
    ly_out_reset(out);
    lyd_free_all(tree);

    data = "{\"a:cp\":{}}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_int_equal(LYS_CONTAINER, tree->schema->nodetype);
    assert_string_equal("cp", tree->schema->name);
    cont = (struct lyd_node_inner*)tree;
    assert_false(cont->flags & LYD_DEFAULT);

    lyd_print_all(out, tree, LYD_JSON, 0);
    assert_string_equal(printed, data);
    ly_out_free(out, NULL, 1);
    lyd_free_all(tree);

    *state = NULL;
}

static void
test_opaq(void **state)
{
    *state = test_opaq;

    const char *data;
    char *str;
    struct lyd_node *tree;

    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    /* invalid value, no flags */
    data = "{\"a:foo3\":[null]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Invalid empty uint32 value. /a:foo3");
    assert_null(tree);

    /* opaq flag */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, LYD_PARSE_OPAQ, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "foo3");
    assert_string_equal(((struct lyd_node_opaq *)tree)->value, "");

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* missing key, no flags */
    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"d\":\"val_d\"}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("List instance is missing its key \"c\". /a:l1[a='val_a'][b='val_b']");
    assert_null(tree);

    /* opaq flag */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, LYD_PARSE_OPAQ, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "l1");
    assert_string_equal(((struct lyd_node_opaq *)tree)->value, "");

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* invalid key, no flags */
    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    logbuf_assert("Invalid int16 value \"val_c\". /a:l1/c");
    assert_null(tree);

    /* opaq flag */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, LYD_PARSE_OPAQ, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "l1");
    assert_string_equal(((struct lyd_node_opaq *)tree)->value, "");

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":{\"val\":\"val_c\"}}]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, LYD_PARSE_OPAQ, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "l1");
    assert_string_equal(((struct lyd_node_opaq *)tree)->value, "");

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\"}]}";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_JSON, LYD_PARSE_OPAQ, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "l1");
    assert_string_equal(((struct lyd_node_opaq *)tree)->value, "");

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    ly_out_free(out, NULL, 1);

    *state = NULL;
}

static void
test_rpc(void **state)
{
    *state = test_rpc;

    const char *data;
    struct ly_in *in;
    char *str;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;

    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    data = "{\"ietf-netconf:rpc\":{\"edit-config\":{"
              "\"target\":{\"running\":[null]},"
              "\"config\":{\"a:cp\":{\"z\":[null],\"@z\":{\"ietf-netconf:operation\":\"replace\"}},"
                          "\"a:l1\":[{\"@\":{\"ietf-netconf:operation\":\"replace\"},\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}"
            "}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(ctx, in, LYD_JSON, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    assert_string_equal(op->schema->name, "edit-config");

    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "rpc");
    /* TODO support generic attributes in JSON ?
    assert_non_null(((struct lyd_node_opaq *)tree)->attr);
    */
    node = lyd_node_children(tree, 0);
    assert_string_equal(node->schema->name, "edit-config");
    node = lyd_node_children(node, 0)->next;
    assert_string_equal(node->schema->name, "config");

    node = ((struct lyd_node_any *)node)->value.tree;
    assert_non_null(node->schema);
    assert_string_equal(node->schema->name, "cp");
    node = lyd_node_children(node, 0);
    /* z has no value */
    assert_null(node->schema);
    assert_string_equal(((struct lyd_node_opaq *)node)->name, "z");
    node = node->parent->next;
    /* l1 key c has invalid value so it is at the end */
    assert_null(node->schema);
    assert_string_equal(((struct lyd_node_opaq *)node)->name, "l1");

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */

    ly_out_free(out, NULL, 1);

    *state = NULL;
}

static void
test_action(void **state)
{
    *state = test_action;

    const char *data;
    struct ly_in *in;
    char *str;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;

    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    data = "{\"ietf-netconf:rpc\":{\"yang:action\":{\"a:c\":{\"act\":{\"al\":\"value\"}}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(ctx, in, LYD_JSON, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    assert_string_equal(op->schema->name, "act");

    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "rpc");
    assert_null(((struct lyd_node_opaq *)tree)->attr);
    node = lyd_node_children(tree, 0);
    assert_null(node->schema);
    assert_string_equal(((struct lyd_node_opaq *)node)->name, "action");
    assert_null(((struct lyd_node_opaq *)node)->attr);

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */

    ly_out_free(out, NULL, 1);

    *state = NULL;
}

static void
test_notification(void **state)
{
    *state = test_notification;

    const char *data;
    struct ly_in *in;
    char *str;
    struct lyd_node *tree, *ntf;
    const struct lyd_node *node;

    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    data = "{\"ietf-restconf:notification\":{\"eventTime\":\"2037-07-08T00:01:00Z\",\"a:c\":{\"n1\":{\"nl\":\"value\"}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_notif(ctx, in, LYD_JSON, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    assert_string_equal(ntf->schema->name, "n1");

    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "notification");
    assert_null(((struct lyd_node_opaq *)tree)->attr);
    node = lyd_node_children(tree, 0);
    assert_null(node->schema);
    assert_string_equal(((struct lyd_node_opaq *)node)->name, "eventTime");
    assert_string_equal(((struct lyd_node_opaq *)node)->value, "2037-07-08T00:01:00Z");
    assert_null(((struct lyd_node_opaq *)node)->attr);
    node = node->next;
    assert_non_null(node->schema);
    assert_string_equal(node->schema->name, "c");

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* top-level notif without envelope */
    data = "{\"a:n2\":{}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_notif(ctx, in, LYD_JSON, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    assert_string_equal(ntf->schema->name, "n2");

    assert_non_null(tree);
    assert_ptr_equal(ntf, tree);

    lyd_print_tree(out, tree, LYD_JSON, 0);
    assert_string_equal(str, data);
    ly_out_reset(out);
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */

    ly_out_free(out, NULL, 1);

    *state = NULL;
}

static void
test_reply(void **state)
{
    *state = test_reply;

    const char *data;
    struct ly_in *in;
    char *str;
    struct lyd_node *request, *tree, *op;
    const struct lyd_node *node;

    struct ly_out *out;
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&str, 0, &out));

    data = "{\"a:c\":{\"act\":{\"al\":\"value\"}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_rpc(ctx, in, LYD_JSON, &request, NULL));
    ly_in_free(in, 0);

    data = "{\"ietf-netconf:rpc-reply\":{\"a:al\":25}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_reply(request, in, LYD_JSON, &tree, &op));
    ly_in_free(in, 0);
    lyd_free_all(request);

    assert_non_null(op);
    assert_string_equal(op->schema->name, "act");
    node = lyd_node_children(op, 0);
    assert_non_null(node->schema);
    assert_string_equal(node->schema->name, "al");
    assert_true(node->schema->flags & LYS_CONFIG_R);

    assert_non_null(tree);
    assert_null(tree->schema);
    assert_string_equal(((struct lyd_node_opaq *)tree)->name, "rpc-reply");
    node = lyd_node_children(tree, 0);
    assert_non_null(node->schema);
    assert_string_equal(node->schema->name, "c");

    /* TODO print only rpc-reply node and then output subtree */
    lyd_print_tree(out, lyd_node_children(op, 0), LYD_JSON, 0);
    assert_string_equal(str, "{\"a:al\":25}");
    ly_out_reset(out);
    lyd_print_tree(out, lyd_node_children(tree, 0), LYD_JSON, 0);
    assert_string_equal(str, "{\"a:c\":{\"act\":{\"al\":25}}}");
    ly_out_reset(out);
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */

    ly_out_free(out, NULL, 1);

    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_leaf, setup, teardown),
        cmocka_unit_test_setup_teardown(test_leaflist, setup, teardown),
        cmocka_unit_test_setup_teardown(test_anydata, setup, teardown),
        cmocka_unit_test_setup_teardown(test_list, setup, teardown),
        cmocka_unit_test_setup_teardown(test_container, setup, teardown),
        cmocka_unit_test_setup_teardown(test_opaq, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rpc, setup, teardown),
        cmocka_unit_test_setup_teardown(test_action, setup, teardown),
        cmocka_unit_test_setup_teardown(test_notification, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reply, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
