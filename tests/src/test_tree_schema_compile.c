/*
 * @file test_parser_yang.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from parser_yang.c
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
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"
#include "../../src/parser_yang.c"
#include "../../src/tree_schema.c"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */

    if (path) {
        snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
    } else {
        strncpy(logbuf, msg, BUFSIZE - 1);
    }
}
#endif

static int
logger_setup(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
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
test_module(void **state)
{
    (void) state; /* unused */

    const char *str;
    struct ly_ctx *ctx;
    struct lys_module mod = {0};

    str = "module test {namespace urn:test; prefix t;"
          "feature f1;}";
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    assert_int_equal(LY_EINVAL, lys_compile(NULL, 0, NULL));
    logbuf_assert("Invalid argument sc (lys_compile()).");
    assert_int_equal(LY_EINVAL, lys_compile(NULL, 0, &mod.compiled));
    logbuf_assert("Invalid argument sp (lys_compile()).");
    assert_int_equal(LY_SUCCESS, yang_parse(ctx, str, &mod.parsed));
    assert_int_equal(LY_SUCCESS, lys_compile(mod.parsed, 0, &mod.compiled));
    assert_non_null(mod.compiled);
    assert_ptr_equal(mod.parsed->name, mod.compiled->name);
    assert_ptr_equal(mod.parsed->ns, mod.compiled->ns);

    lysc_module_free(mod.compiled);

    assert_int_equal(LY_SUCCESS, lys_compile(mod.parsed, LYSC_OPT_FREE_SP, &mod.compiled));
    assert_non_null(mod.compiled);
    assert_string_equal("test", mod.compiled->name);
    assert_string_equal("urn:test", mod.compiled->ns);

    lysc_module_free(mod.compiled);
    ly_ctx_destroy(ctx, NULL);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_module, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
