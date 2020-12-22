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
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"

/* common module for the tests */
const char *schema_a = "module a {\n"
        "  namespace urn:tests:a;\n"
        "  prefix a;yang-version 1.1;\n"
        "  list l1 {\n"
        "    key \"a b\";\n"
        "    leaf a {\n"
        "      type string;\n"
        "    }\n"
        "    leaf b {\n"
        "      type string;\n"
        "    }\n"
        "    leaf c {\n"
        "      type string;}\n"
        "    }\n"
        "    leaf foo {\n"
        "      type uint16;\n"
        "    }\n"
        "    leaf-list ll {\n"
        "      type string;\n"
        "    }\n"
        "    container c {\n"
        "      leaf-list x {\n"
        "    type string;\n"
        "    }\n"
        "  }"
        "  anydata any {\n"
        "    config false;\n"
        "  }"
        "  list l2 {\n"
        "    config false;\n"
        "    container c {\n"
        "      leaf x {\n"
        "        type string;\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "  rpc oper {\n"
        "    input {\n"
        "      leaf param {\n"
        "        type string;\n"
        "      }\n"
        "    }\n"
        "    output {\n"
        "      leaf param {\n"
        "        type int8;\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n";

static void
test_top_level(void **state)
{
    const struct lys_module *mod;
    struct lyd_node *node, *rpc;

    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, NULL, &mod);

    /* list */
    assert_int_equal(lyd_new_list(NULL, mod, "l1", 0, &node, "val_a", "val_b"), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[]", 0, &node), LY_EVALID);
    CHECK_LOG_CTX("Unexpected XPath token \"]\" (\"]\").", "Schema location /a:l1.");

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[key1='a'][key2='b']", 0, &node), LY_ENOTFOUND);
    CHECK_LOG_CTX("Not found node \"key1\" in path.", "Schema location /a:l1.");

    assert_int_equal(lyd_new_list2(NULL, mod, "l1", "[a='a'][b='b'][c='c']", 0, &node), LY_EVALID);
    CHECK_LOG_CTX("Key expected instead of leaf \"c\" in path.", "Schema location /a:l1.");

    assert_int_equal(lyd_new_list2(NULL, mod, "c", "[a='a'][b='b']", 0, &node), LY_ENOTFOUND);
    CHECK_LOG_CTX("List node \"c\" not found.", NULL);

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
    CHECK_LOG_CTX("Invalid uint16 value \"[a='a'][b='b'][c='c']\".", "Schema location /a:foo.");

    assert_int_equal(lyd_new_term(NULL, mod, "c", "value", 0, &node), LY_ENOTFOUND);
    CHECK_LOG_CTX("Term node \"c\" not found.", NULL);

    assert_int_equal(lyd_new_term(NULL, mod, "foo", "256", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* leaf-list */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "ahoy", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* container */
    assert_int_equal(lyd_new_inner(NULL, mod, "c", 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    assert_int_equal(lyd_new_inner(NULL, mod, "l1", 0, &node), LY_ENOTFOUND);
    CHECK_LOG_CTX("Inner node (and not a list) \"l1\" not found.", NULL);

    assert_int_equal(lyd_new_inner(NULL, mod, "l2", 0, &node), LY_ENOTFOUND);
    CHECK_LOG_CTX("Inner node (and not a list) \"l2\" not found.", NULL);

    /* anydata */
    assert_int_equal(lyd_new_any(NULL, mod, "any", "some-value", LYD_ANYDATA_STRING, 0, &node), LY_SUCCESS);
    lyd_free_tree(node);

    /* key-less list */
    assert_int_equal(lyd_new_list2(NULL, mod, "l2", "[a='a'][b='b']", 0, &node), LY_EVALID);
    CHECK_LOG_CTX("List predicate defined for keyless list \"l2\" in path.", "Schema location /a:l2.");

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
}

static void
test_opaq(void **state)
{
    struct lyd_node *root, *node;
    struct lyd_node_opaq *opq;

    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, NULL, NULL);

    assert_int_equal(lyd_new_opaq(NULL, UTEST_LYCTX, "node1", NULL, "my-module", &root), LY_SUCCESS);
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
}

static void
test_path(void **state)
{
    LY_ERR ret;
    struct lyd_node *root, *node, *parent;

    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, NULL, NULL);

    /* create 2 nodes */
    ret = lyd_new_path2(NULL, UTEST_LYCTX, "/a:c/x[.='val']", "vvv", 0, 0, &root, &node);
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
    ret = lyd_new_path2(NULL, UTEST_LYCTX, "/a:l1", NULL, 0, 0, NULL, NULL);
    assert_int_equal(ret, LY_EINVAL);
    CHECK_LOG_CTX("Predicate missing for list \"l1\" in path \"/a:l1\".", "Schema location /a:l1.");

    ret = lyd_new_path2(NULL, UTEST_LYCTX, "/a:l1", NULL, 0, LYD_NEW_PATH_OPAQ, NULL, &root);
    assert_int_equal(ret, LY_SUCCESS);
    assert_non_null(root);
    assert_null(root->schema);

    lyd_free_tree(root);

    ret = lyd_new_path2(NULL, UTEST_LYCTX, "/a:foo", NULL, 0, 0, NULL, NULL);
    assert_int_equal(ret, LY_EVALID);
    CHECK_LOG_CTX("Invalid empty uint16 value.", "Schema location /a:foo.");

    ret = lyd_new_path2(NULL, UTEST_LYCTX, "/a:foo", NULL, 0, LYD_NEW_PATH_OPAQ, NULL, &root);
    assert_int_equal(ret, LY_SUCCESS);
    assert_non_null(root);
    assert_null(root->schema);

    lyd_free_tree(root);

    /* try LYD_NEWOPT_UPDATE */
    ret = lyd_new_path2(NULL, UTEST_LYCTX, "/a:l2[1]/c/x", "val", 0, 0, &root, &node);
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
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_top_level),
        UTEST(test_opaq),
        UTEST(test_path),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
