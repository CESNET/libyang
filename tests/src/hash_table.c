/*
 * @file hash_table.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from hash_table.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include "tests/config.h"
#include "../../src/hash_table.c"

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
test_invalid_arguments(void **state)
{
    (void) state; /* unused */
    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    assert_null(lydict_insert(NULL, NULL, 0));
    logbuf_assert("Invalid argument ctx (lydict_insert()).");

    assert_null(lydict_insert_zc(NULL, NULL));
    logbuf_assert("Invalid argument ctx (lydict_insert_zc()).");
    logbuf_clean();
    assert_null(lydict_insert_zc(ctx, NULL));
    logbuf_assert("");

    ly_ctx_destroy(ctx, NULL);
}

static void
test_dict_hit(void **state)
{
    (void) state; /* unused */

    const char *str1, *str2;
    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    /* insert 2 strings, one of them repeatedly */
    str1 = lydict_insert(ctx, "test1", 0);
    assert_non_null(str1);
    /* via zerocopy we have to get the same pointer as provided */
    assert_non_null(str2 = strdup("test2"));
    assert_true(str2 == lydict_insert_zc(ctx, (char *)str2));
    /* here we get the same pointer as in case the string was inserted first time */
    str2 = lydict_insert(ctx, "test1", 0);
    assert_non_null(str2);
    assert_ptr_equal(str1, str2);

    /* remove strings, but the repeatedly inserted only once */
    lydict_remove(ctx, "test1");
    lydict_remove(ctx, "test2");

    /* destroy dictionary - should raise warning about data presence */
    ly_ctx_destroy(ctx, NULL);
    logbuf_assert("String \"test1\" not freed from the dictionary, refcount 1");

#ifndef NDEBUG
    /* cleanup */
    free((char*)str1);
#endif
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_invalid_arguments, logger_setup),
        cmocka_unit_test_setup(test_dict_hit, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
