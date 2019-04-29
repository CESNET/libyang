/*
 * @file test_printer_yang.c
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

#include "../../src/common.c"
#include "../../src/set.c"
#include "../../src/log.c"
#include "../../src/hash_table.c"
#include "../../src/xpath.c"
#include "../../src/parser_yang.c"
#include "../../src/context.c"
#include "../../src/tree_schema_helpers.c"
#include "../../src/tree_schema_free.c"
#include "../../src/tree_schema_compile.c"
#include "../../src/tree_schema.c"
#include "../../src/printer_yang.c"
#include "../../src/printer.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

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
logger_setup(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif
    return 0;
}

static int
logger_teardown(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
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
    *state = test_module;

    struct ly_ctx *ctx = {0};
    const struct lys_module *mod;
    const char *orig = "module a {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:test:a\";\n"
            "  prefix a;\n\n"
            "  import ietf-yang-types {\n"
            "    prefix yt;\n"
            "    revision-date 2013-07-15;\n"
            "    description\n"
            "      \"YANG types\";\n"
            "    reference\n"
            "      \"RFC reference\";\n"
            "  }\n\n"
            "  organization\n"
            "    \"ORG\";\n"
            "  contact\n"
            "    \"Radek Krejci.\";\n"
            "  description\n"
            "    \"Long multiline\n"
            "      description.\";\n"
            "  reference\n"
            "    \"some reference\";\n"
            "}\n";
    char *printed;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, orig, LYS_IN_YANG));
    assert_int_equal(strlen(orig), lys_print_mem(&printed, mod, LYS_OUT_YANG, 0, 0));
    assert_string_equal(printed, orig);
    free(printed);

    orig = "module b {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:test:b\";\n"
            "  prefix b;\n\n"
            "  revision 2019-04-16 {\n"
            "    description\n"
            "      \"text\";\n"
            "    reference\n"
            "      \"text\";\n"
            "  }\n"
            "  revision 2019-04-15 {\n"
            "    description\n"
            "      \"initial revision\";\n"
            "  }\n\n"
            "  feature f1 {\n"
            "    status current;\n"
            "    description\n"
            "      \"text\";\n"
            "    reference\n"
            "      \"text\";\n"
            "  }\n\n"
            "  feature f2 {\n"
            "    if-feature \"not f1\";\n"
            "  }\n"
            "}\n";
    assert_non_null(mod = lys_parse_mem(ctx, orig, LYS_IN_YANG));
    assert_int_equal(strlen(orig), lys_print_mem(&printed, mod, LYS_OUT_YANG, 0, 0));
    assert_string_equal(printed, orig);
    free(printed);

    orig = "module c {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:test:c\";\n"
            "  prefix c;\n\n"
            "  feature f1;\n\n"
            "  identity i1 {\n"
            "    if-feature \"f1\";\n"
            "    description\n"
            "      \"text\";\n"
            "    reference\n"
            "      \"text32\";\n"
            "  }\n\n"
            "  identity i2 {\n"
            "    base i1;\n"
            "    status obsolete;\n"
            "  }\n"
            "}\n";
    assert_non_null(mod = lys_parse_mem(ctx, orig, LYS_IN_YANG));
    assert_int_equal(strlen(orig), lys_print_mem(&printed, mod, LYS_OUT_YANG, 0, 0));
    assert_string_equal(printed, orig);
    free(printed);

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

/* TODO: include */

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_module, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
