/*
 * @file set.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from context.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "tests/config.h"
#include "../../src/context.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#include <stdio.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    (void) path; /* unused */

    strncpy(logbuf, msg, BUFSIZE - 1);
}

static int
logger_setup(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 0);
#endif
    return 0;
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

int __real_ly_set_add(struct ly_set *set, void *object, int options);
int __wrap_ly_set_add(struct ly_set *set, void *object, int options)
{
    int wrap = mock_type(int);

    if (wrap) {
        /* error */
        return -1;
    } else {
        return __real_ly_set_add(set, object, options);
    }
}

static void
test_searchdirs(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;
    const char * const *list;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    /* invalid arguments */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(NULL, NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_set_searchdir()).");
    assert_null(ly_ctx_get_searchdirs(NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_get_searchdirs()).");
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdirs(NULL, 0));
    logbuf_assert("Invalid argument ctx (ly_ctx_unset_searchdirs()).");
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdirs(ctx, 0));
    logbuf_assert("Invalid argument index (ly_ctx_unset_searchdirs()).");

    /* readable and executable, but not a directory */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(ctx, TESTS_BIN"/src_context"));
    logbuf_assert("Given search directory \""TESTS_BIN"/src_context\" is not a directory.");
    /* not executable */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(ctx, TESTS_SRC"/src/context.c"));
    logbuf_assert("Unable to use search directory \""TESTS_SRC"/src/context.c\" (Permission denied)");
    /* not existing */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(ctx, "/nonexistingfile"));
    logbuf_assert("Unable to use search directory \"/nonexistingfile\" (No such file or directory)");

    /* ly_set_add() fails */
    will_return(__wrap_ly_set_add, 1);
    assert_int_equal(LY_EMEM, ly_ctx_set_searchdir(ctx, TESTS_BIN"/src"));

    /* no change */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, NULL));

    /* correct path */
    will_return_always(__wrap_ly_set_add, 0);
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_BIN"/src"));
    assert_int_equal(1, ctx->search_paths.count);
    assert_string_equal(TESTS_BIN"/src", ctx->search_paths.objs[0]);

    /* duplicated paths - function succeeds, but context still contains just a single path */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_BIN"/src"));
    assert_int_equal(1, ctx->search_paths.count);
    assert_string_equal(TESTS_BIN"/src", ctx->search_paths.objs[0]);

    /* another paths - add 8 to fill the initial buffer of the searchpaths list */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_BIN"/CMakeFiles"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC"/../src"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC"/../CMakeModules"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC"/../doc"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_BIN));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, "/tmp"));
    assert_int_equal(8, ctx->search_paths.count);

    /* get searchpaths */
    list = ly_ctx_get_searchdirs(ctx);
    assert_non_null(list);
    assert_string_equal(TESTS_BIN"/src", list[0]);
    assert_string_equal(TESTS_BIN"/CMakeFiles", list[1]);
    assert_string_equal(TESTS_SRC, list[5]);
    assert_string_equal(TESTS_BIN, list[6]);
    assert_string_equal("/tmp", list[7]);
    assert_null(list[8]);

    /* removing searchpaths */
    /* first */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, 0));
    assert_string_not_equal(TESTS_BIN"/src", list[0]);
    assert_int_equal(7, ctx->search_paths.count);
    /* middle */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, 4));
    assert_int_equal(6, ctx->search_paths.count);
    /* last */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, 5));
    assert_int_equal(5, ctx->search_paths.count);
    /* all */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, -5));
    assert_int_equal(0, ctx->search_paths.count);

    ly_ctx_destroy(ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_searchdirs, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
