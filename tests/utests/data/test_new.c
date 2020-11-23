/**
 * @file test_new.c
 * @author: Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for functions for creating data
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "libyang.h"
#include "utests.h"

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

    const char *schema_a = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;"
            "list l1 { key \"a b\"; leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
            "leaf foo { type uint16;}"
            "leaf-list ll { type string;}"
            "container c {leaf-list x {type string;}}"
            "anydata any {config false;}"
            "list l2 {config false; container c{leaf x {type string;}}}"
            "rpc oper {input {leaf param {type string;}} output {leaf param {type int8;}}}}";

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));
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
test_top_level(void **state)
{
    *state = test_top_level;

    const struct lys_module *mod;
    struct lyd_node *node, *rpc;

    /* we need the module first */
    mod = ly_ctx_get_module_implemented(ctx, "a");
    assert_non_null(mod);

    /* list */
    assert_int_equal(lyd_new_list(NULL, mod, "l1", 0, &node, "val_a", "val_b"), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[]", 0, &node), LY_EVALID);
    logbuf_assert("Unexpected XPath token \"]\" (\"]\").");

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[key1='a'][key2='b']", 0, &node), LY_ENOTFOUND);
    logbuf_assert("Not found node \"key1\" in path.");

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[a='a'][b='b'][c='c']", 0, &node), LY_EVALID);
    logbuf_assert("Key expected instead of leaf \"c\" in path. /a:l1/c");

    assert_int_equal(lyd_new_list2(NULL, mod, "c", "[a='a'][b='b']", 0, &node), LY_ENOTFOUND);
    logbuf_assert("List node \"c\" not found.");

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[a='a'][b='b']", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[a=''][b='']", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[a:a='a'][a:b='b']", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[a=   'a']\n[b  =\t'b']", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* leaf */
    assert_int_equal(lyd_new_term(NULL, mod, "foo", "[a='a'][b='b'][c='c']", 0, &node), LY_EVALID);
    logbuf_assert("Invalid uint16 value \"[a='a'][b='b'][c='c']\". /a:foo");

    assert_int_equal(lyd_new_term(NULL, mod, "c", "value", 0, &node), LY_ENOTFOUND);
    logbuf_assert("Term node \"c\" not found.");

    assert_int_equal(lyd_new_term(NULL, mod, "foo", "256", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* leaf-list */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "ahoy", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* container */
    assert_int_equal(lyd_new_inner(NULL, mod, "c", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_inner(NULL, mod, "l1", 0, &node), LY_ENOTFOUND);
    logbuf_assert("Inner node (and not a list) \"l1\" not found.");

    assert_int_equal(lyd_new_inner(NULL, mod, "l2", 0, &node), LY_ENOTFOUND);
    logbuf_assert("Inner node (and not a list) \"l2\" not found.");

    /* anydata */
    assert_int_equal(lyd_new_any(NULL, mod, "any", "some-value", LYD_ANYDATA_STRING, 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* key-less list */
    assert_int_equal(lyd_new_list2(NULL, mod, "l2", "[a='a'][b='b']", 0, &node), LY_EVALID);
    logbuf_assert("List predicate defined for keyless list \"l2\" in path.");

    assert_int_equal(lyd_new_list2(NULL, mod, "l2", "", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_list2(NULL, mod, "l2", NULL, 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_list(NULL, mod, "l2", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* RPC */
    assert_int_equal(lyd_new_inner(NULL, mod, "oper", 0, &rpc), LY_SUCCESS);
    assert_int_equal(lyd_new_term(rpc, mod, "param", "22", 0, &node), LY_SUCCESS);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_node_leaf *)node->schema)->type->basetype);
    assert_int_equal(lyd_new_term(rpc, mod, "param", "22", 1, &node), LY_SUCCESS);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_node_leaf *)node->schema)->type->basetype);
    lyd_free_tree(rpc);

    *state = NULL;
}

static void
test_opaq(void **state)
{
    *state = test_opaq;

    struct lyd_node *root, *node;
    struct lyd_node_opaq *opq;

    assert_int_equal(lyd_new_opaq(NULL, ctx, "node1", NULL, "my-module", &root), LY_SUCCESS);
    assert_null(root->schema);
    opq = (struct lyd_node_opaq *)root;
    assert_string_equal(opq->name.name, "node1");
    assert_string_equal(opq->name.module_name, "my-module");
    assert_string_equal(opq->value, "");

    assert_int_equal(lyd_new_opaq(root, NULL, "node2", "value", "my-module2", &node), LY_SUCCESS);
    assert_null(node->schema);
    opq = (struct lyd_node_opaq *)node;
    assert_string_equal(opq->name.name, "node2");
    assert_string_equal(opq->name.module_name, "my-module2");
    assert_string_equal(opq->value, "value");
    assert_ptr_equal(opq->parent, root);

    lyd_free_tree(root);

    *state = NULL;
}

static void
test_path(void **state)
{
    *state = test_path;

    LY_ERR ret;
    struct lyd_node *root, *node, *parent;

    /* create 2 nodes */
    ret = lyd_new_path2(NULL, ctx, "/a:c/x[.='val']", "vvv", 0, 0, &root, &node);
    assert_int_equal(ret, LY_SUCCESS);
    assert_non_null(root);
    assert_string_equal(root->schema->name, "c");
    assert_non_null(node);
    assert_string_equal(node->schema->name, "x");
    assert_string_equal("val", LYD_CANON_VALUE(node));

    /* append another */
    ret = lyd_new_path2(root, NULL, "/a:c/x", "val2", 0, 0, &parent, &node);
    assert_int_equal(ret, LY_SUCCESS);
    assert_ptr_equal(parent, node);
    assert_string_equal(node->schema->name, "x");
    assert_string_equal("val2", LYD_CANON_VALUE(node));

    /* and a last one */
    ret = lyd_new_path2(root, NULL, "x", "val3", 0, 0, &parent, &node);
    assert_int_equal(ret, LY_SUCCESS);
    assert_ptr_equal(parent, node);
    assert_string_equal(node->schema->name, "x");
    assert_string_equal("val3", LYD_CANON_VALUE(node));

    lyd_free_tree(root);

    /* try LYD_NEWOPT_OPAQ */
    ret = lyd_new_path2(NULL, ctx, "/a:l1", NULL, 0, 0, NULL, NULL);
    assert_int_equal(ret, LY_EINVAL);
    logbuf_assert("Predicate missing for list \"l1\" in path.");

    ret = lyd_new_path2(NULL, ctx, "/a:l1", NULL, 0, LYD_NEW_PATH_OPAQ, NULL, &root);
    assert_int_equal(ret, LY_SUCCESS);
    assert_non_null(root);
    assert_null(root->schema);

    lyd_free_tree(root);

    ret = lyd_new_path2(NULL, ctx, "/a:foo", NULL, 0, 0, NULL, NULL);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Invalid empty uint16 value. /a:foo");

    ret = lyd_new_path2(NULL, ctx, "/a:foo", NULL, 0, LYD_NEW_PATH_OPAQ, NULL, &root);
    assert_int_equal(ret, LY_SUCCESS);
    assert_non_null(root);
    assert_null(root->schema);

    lyd_free_tree(root);

    /* try LYD_NEWOPT_UPDATE */
    ret = lyd_new_path2(NULL, ctx, "/a:l2[1]/c/x", "val", 0, 0, &root, &node);
    assert_int_equal(ret, LY_SUCCESS);
    assert_non_null(root);
    assert_string_equal(node->schema->name, "x");
    assert_string_equal("val", LYD_CANON_VALUE(node));

    ret = lyd_new_path2(root, NULL, "/a:l2[1]/c/x", "val", 0, 0, NULL, &node);
    assert_int_equal(ret, LY_EEXIST);

    ret = lyd_new_path2(root, NULL, "/a:l2[1]/c/x", "val", 0, LYD_NEW_PATH_UPDATE, NULL, &node);
    assert_int_equal(ret, LY_SUCCESS);
    assert_null(node);

    ret = lyd_new_path2(root, NULL, "/a:l2[1]/c/x", "val2", 0, LYD_NEW_PATH_UPDATE, NULL, &node);
    assert_int_equal(ret, LY_SUCCESS);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "x");
    assert_string_equal("val2", LYD_CANON_VALUE(node));

    lyd_free_tree(root);

    *state = NULL;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_top_level, setup, teardown),
        cmocka_unit_test_setup_teardown(test_opaq, setup, teardown),
        cmocka_unit_test_setup_teardown(test_path, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
