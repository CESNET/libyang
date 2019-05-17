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

#include "../../src/libyang.h"

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
    const char *schema_a = "module types {namespace urn:tests:types;prefix t;yang-version 1.1;"
            "leaf binary {type binary {length 5;}}"
            "leaf binary-norestr {type binary;}}";

    s = calloc(1, sizeof *s);
    assert_non_null(s);

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &s->ctx));
    assert_non_null(lys_parse_mem(s->ctx, schema_a, LYS_IN_YANG));

    *state = s;

    return 0;
}

static int
teardown(void **state)
{
    struct state_s *s = (struct state_s*)(*state);

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
test_binary(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_binary;

    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    const char *data = "<binary xmlns=\"urn:tests:types\">\n   aGVsbG8=  \t\n  </binary>";

    /* valid data (hello) */
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("aGVsbG8=", leaf->value.canonized);
    lyd_free_all(tree);

    /* no data */
    data = "<binary-norestr xmlns=\"urn:tests:types\">\n    \t\n  </binary-norestr>";
    assert_non_null(tree = lyd_parse_mem(s->ctx, data, LYD_XML, 0));
    assert_int_equal(LYS_LEAF, tree->schema->nodetype);
    assert_string_equal("binary-norestr", tree->schema->name);
    leaf = (struct lyd_node_term*)tree;
    assert_string_equal("", leaf->value.canonized);
    lyd_free_all(tree);

    s->func = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_binary, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
