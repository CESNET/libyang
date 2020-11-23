/*
 * @file test_xpath.c
 * @author: Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for XPath evaluation
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <string.h>

#include "context.h"
#include "parser_data.h"
#include "set.h"
#include "tests/config.h"
#include "tree_data.h"
#include "tree_schema.h"
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

    const char *schema_a =
            "module a {\n"
            "    namespace urn:tests:a;\n"
            "    prefix a;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    list l1 {\n"
            "        key \"a b\";\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            type string;\n"
            "        }\n"
            "        leaf c {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf foo {\n"
            "        type string;\n"
            "    }\n"
            "    leaf foo2 {\n"
            "        type uint8;\n"
            "    }\n"
            "    container c {\n"
            "        leaf x {\n"
            "            type string;\n"
            "        }\n"
            "        list ll {\n"
            "            key \"a\";\n"
            "            leaf a {\n"
            "                type string;\n"
            "            }\n"
            "            list ll {\n"
            "                key \"a\";\n"
            "                leaf a {\n"
            "                    type string;\n"
            "                }\n"
            "                leaf b {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "        leaf-list ll2 {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "}";
    const char *schema_b =
            "module b {\n"
            "    namespace urn:tests:b;\n"
            "    prefix b;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    list l2 {\n"
            "        key \"a\";\n"
            "        leaf a {\n"
            "            type uint16;\n"
            "        }\n"
            "        leaf b {\n"
            "            type uint16;\n"
            "        }\n"
            "    }\n"
            "}";

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &ctx));
    assert_int_equal(lys_parse_mem(ctx, schema_a, LYS_IN_YANG, NULL), LY_SUCCESS);
    assert_int_equal(lys_parse_mem(ctx, schema_b, LYS_IN_YANG, NULL), LY_SUCCESS);

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
test_hash(void **state)
{
    *state = test_hash;

    const char *data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a3</a>\n"
            "    <b>b3</b>\n"
            "    <c>c3</c>\n"
            "</l1>\n"
            "<foo xmlns=\"urn:tests:a\">foo value</foo>\n"
            "<c xmlns=\"urn:tests:a\">\n"
            "    <x>val</x>\n"
            "    <ll>\n"
            "        <a>val_a</a>\n"
            "        <ll>\n"
            "            <a>val_a</a>\n"
            "            <b>val</b>\n"
            "        </ll>\n"
            "        <ll>\n"
            "            <a>val_b</a>\n"
            "        </ll>\n"
            "    </ll>\n"
            "    <ll>\n"
            "        <a>val_b</a>\n"
            "        <ll>\n"
            "            <a>val_a</a>\n"
            "        </ll>\n"
            "        <ll>\n"
            "            <a>val_b</a>\n"
            "            <b>val</b>\n"
            "        </ll>\n"
            "    </ll>\n"
            "    <ll>\n"
            "        <a>val_c</a>\n"
            "        <ll>\n"
            "            <a>val_a</a>\n"
            "        </ll>\n"
            "        <ll>\n"
            "            <a>val_b</a>\n"
            "        </ll>\n"
            "    </ll>\n"
            "    <ll2>one</ll2>\n"
            "    <ll2>two</ll2>\n"
            "    <ll2>three</ll2>\n"
            "    <ll2>four</ll2>\n"
            "</c>";
    struct lyd_node *tree, *node;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* top-level, so hash table is not ultimately used but instances can be compared based on hashes */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:l1[a='a3'][b='b3']", &set));
    assert_int_equal(1, set->count);

    node = set->objs[0];
    assert_string_equal(node->schema->name, "l1");
    node = lyd_child(node);
    assert_string_equal(node->schema->name, "a");
    assert_string_equal(LYD_CANON_VALUE(node), "a3");

    ly_set_free(set, NULL);

    /* hashes should be used for both searches (well, there are not enough nested ll instances, so technically not true) */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[a='val_b']/ll[a='val_b']", &set));
    assert_int_equal(1, set->count);

    node = set->objs[0];
    assert_string_equal(node->schema->name, "ll");
    node = lyd_child(node);
    assert_string_equal(node->schema->name, "a");
    assert_string_equal(LYD_CANON_VALUE(node), "val_b");
    node = node->next;
    assert_string_equal(node->schema->name, "b");
    assert_null(node->next);

    ly_set_free(set, NULL);

    /* hashes are not used */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c//ll[a='val_b']", &set));
    assert_int_equal(4, set->count);

    ly_set_free(set, NULL);

    /* hashes used even for leaf-lists */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll2[. = 'three']", &set));
    assert_int_equal(1, set->count);

    node = set->objs[0];
    assert_string_equal(node->schema->name, "ll2");
    assert_string_equal(LYD_CANON_VALUE(node), "three");

    ly_set_free(set, NULL);

    /* not found using hashes */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[a='val_d']", &set));
    assert_int_equal(0, set->count);

    ly_set_free(set, NULL);

    /* white-spaces are also ok */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[ \na = 'val_c' ]", &set));
    assert_int_equal(1, set->count);

    ly_set_free(set, NULL);

    lyd_free_all(tree);
    *state = NULL;
}

static void
test_toplevel(void **state)
{
    *state = test_toplevel;

    const char *data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a3</a>\n"
            "    <b>b3</b>\n"
            "    <c>c3</c>\n"
            "</l1>\n"
            "<foo xmlns=\"urn:tests:a\">foo value</foo>\n"
            "<l2 xmlns=\"urn:tests:b\">\n"
            "    <a>1</a>\n"
            "    <b>1</b>\n"
            "</l2>\n"
            "<l2 xmlns=\"urn:tests:b\">\n"
            "    <a>2</a>\n"
            "    <b>1</b>\n"
            "</l2>\n"
            "<l2 xmlns=\"urn:tests:b\">\n"
            "    <a>3</a>\n"
            "    <b>1</b>\n"
            "</l2>";
    struct lyd_node *tree;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* all top-level nodes from one module (default container as well) */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:*", &set));
    assert_int_equal(5, set->count);

    ly_set_free(set, NULL);

    /* all top-level nodes from all modules */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/*", &set));
    assert_int_equal(8, set->count);

    ly_set_free(set, NULL);

    /* all nodes from one module */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//a:*", &set));
    assert_int_equal(13, set->count);

    ly_set_free(set, NULL);

    /* all nodes from all modules */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//*", &set));
    assert_int_equal(22, set->count);

    ly_set_free(set, NULL);

    /* all nodes from all modules #2 */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//.", &set));
    assert_int_equal(22, set->count);

    ly_set_free(set, NULL);

    lyd_free_all(tree);
    *state = NULL;
}

static void
test_atomize(void **state)
{
    *state = test_atomize;

    struct ly_set *set;
    const struct lys_module *mod;

    mod = ly_ctx_get_module_latest(ctx, "a");

    /* some random paths just making sure the API function works */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(mod->compiled->data, "/a:*", 0, &set));
    assert_int_equal(4, set->count);

    ly_set_free(set, NULL);

    /* all nodes from all modules (including internal, which can change easily, so check just the test modules) */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(mod->compiled->data, "//.", 0, &set));
    assert_in_range(set->count, 16, UINT32_MAX);

    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(mod->compiled->data->next->next, "/a:c/ll[a='val1']/ll[a='val2']/b",
            0, &set));
    assert_int_equal(7, set->count);

    ly_set_free(set, NULL);
}

static void
test_canonize(void **state)
{
    *state = test_canonize;

    const char *data =
            "<foo2 xmlns=\"urn:tests:a\">50</foo2>";
    struct lyd_node *tree;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:foo2[.='050']", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    /* TODO more use-cases once there are some type plugins that have canonical values */

    lyd_free_all(tree);
    *state = NULL;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_hash, setup, teardown),
        cmocka_unit_test_setup_teardown(test_toplevel, setup, teardown),
        cmocka_unit_test_setup_teardown(test_atomize, setup, teardown),
        cmocka_unit_test_setup_teardown(test_canonize, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
