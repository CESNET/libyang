/**
 * @file test_yanglib.c
 * @author: Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for ietf-yang-library data
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include <cmocka.h>

#include "context.h"
#include "log.h"
#include "set.h"
#include "tests/config.h"
#include "tree_data.h"
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

static LY_ERR
test_imp_clb(const char *mod_name, const char *mod_rev, const char *submod_name, const char *sub_rev, void *user_data,
             LYS_INFORMAT *format, const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    const char *schema_a_sub =
    "submodule a_sub {"
        "belongs-to a {"
            "prefix a;"
        "}"
        "yang-version 1.1;"

        "feature feat1;"

        "list l3 {"
            "key \"a\";"
            "leaf a {"
                "type uint16;"
            "}"
            "leaf b {"
                "type uint16;"
            "}"
        "}"
    "}";

    assert_string_equal(mod_name, "a");
    assert_null(mod_rev);
    if (!submod_name) {
        return LY_ENOTFOUND;
    }
    assert_string_equal(submod_name, "a_sub");
    assert_null(sub_rev);
    assert_null(user_data);

    *format = LYS_IN_YANG;
    *module_data = schema_a_sub;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static int
setup(void **state)
{
    (void) state; /* unused */

    const char *schema_a =
    "module a {"
        "namespace urn:tests:a;"
        "prefix a;"
        "yang-version 1.1;"

        "include a_sub;"

        "list l2 {"
            "key \"a\";"
            "leaf a {"
                "type uint16;"
            "}"
            "leaf b {"
                "type uint16;"
            "}"
        "}"
    "}";
    const char *schema_b =
    "module b {"
        "namespace urn:tests:b;"
        "prefix b;"
        "yang-version 1.1;"

        "import a {"
            "prefix a;"
        "}"

        "deviation /a:l2 {"
            "deviate add {"
                "max-elements 40;"
            "}"
        "}"

        "leaf foo {"
            "type string;"
        "}"
    "}";
    const struct lys_module *mod;

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &ctx));
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, NULL);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_a, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "feat1"));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, schema_b, LYS_IN_YANG, NULL));

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
test_yanglib(void **state)
{
    *state = test_yanglib;

    struct lyd_node *tree;
    struct ly_set *set;
    LY_ERR ret;

    assert_int_equal(LY_SUCCESS, ly_ctx_get_yanglib_data(ctx, &tree));

    /* make sure there is "a" with a submodule and deviation */
    ret = lyd_find_xpath(tree, "/ietf-yang-library:yang-library/module-set/module[name='a'][submodule/name='a_sub']"
                         "[feature='feat1'][deviation='b']", &set);
    assert_int_equal(ret, LY_SUCCESS);

    assert_int_equal(set->count, 1);
    ly_set_free(set, NULL);

    lyd_free_all(tree);
    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_yanglib, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
