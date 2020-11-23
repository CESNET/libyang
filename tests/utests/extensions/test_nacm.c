/*
 * @file test_nacm.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for NACM extensions support
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "utests.h"

#include "libyang.h"
#include "tests/config.h"

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

    s = calloc(1, sizeof *s);
    assert_non_null(s);

#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &s->ctx));
    assert_non_null(ly_ctx_load_module(s->ctx, "ietf-netconf-acm", "2018-02-14", NULL));

    *state = s;

    return 0;
}

static int
teardown(void **state)
{
    struct state_s *s = (struct state_s *)(*state);

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
test_deny_all(void **state)
{
    struct state_s *s = (struct state_s *)(*state);

    s->func = test_deny_all;

    const struct lys_module *mod;
    struct lysc_node_container *cont;
    struct lysc_node_leaf *leaf;
    struct lysc_ext_instance *e;

    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:nacm:a; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "container a { nacm:default-deny-all; leaf aa {type string;}}"
            "leaf b {type string;}}";

    /* valid data */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(s->ctx, data, LYS_IN_YANG, &mod));
    assert_non_null(cont = (struct lysc_node_container *)mod->compiled->data);
    assert_non_null(leaf = (struct lysc_node_leaf *)cont->child);
    assert_non_null(e = &cont->exts[0]);
    assert_int_equal(LY_ARRAY_COUNT(cont->exts), 1);
    assert_int_equal(LY_ARRAY_COUNT(leaf->exts), 1); /* NACM extensions inherit */
    assert_ptr_equal(e->def, leaf->exts[0].def);
    assert_int_equal(1, *((uint8_t *)e->data)); /* plugin's value for default-deny-all */
    assert_null(cont->next->exts);

    /* invalid */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:nacm:aa; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "nacm:default-deny-all;}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-all is allowed only in a data nodes, but it is placed in \"module\" statement.) /aa:{extension='nacm:default-deny-all'}");

    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:nacm:aa; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "leaf l { type string; nacm:default-deny-all; nacm:default-deny-write;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-write is mixed with nacm:default-deny-all.) /aa:l/{extension='nacm:default-deny-write'}");

    s->func = NULL;
}

static void
test_deny_write(void **state)
{
    struct state_s *s = (struct state_s *)(*state);

    s->func = test_deny_write;

    const struct lys_module *mod;
    struct lysc_node_container *cont;
    struct lysc_node_leaf *leaf;
    struct lysc_ext_instance *e;

    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:nacm:a; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "container a { nacm:default-deny-write; leaf aa {type string;}}"
            "leaf b {type string;}}";

    /* valid data */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(s->ctx, data, LYS_IN_YANG, &mod));
    assert_non_null(cont = (struct lysc_node_container *)mod->compiled->data);
    assert_non_null(leaf = (struct lysc_node_leaf *)cont->child);
    assert_non_null(e = &cont->exts[0]);
    assert_int_equal(LY_ARRAY_COUNT(cont->exts), 1);
    assert_int_equal(LY_ARRAY_COUNT(leaf->exts), 1); /* NACM extensions inherit */
    assert_ptr_equal(e->def, leaf->exts[0].def);
    assert_int_equal(2, *((uint8_t *)e->data)); /* plugin's value for default-deny-write */
    assert_null(cont->next->exts);

    /* invalid */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:nacm:aa; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "notification notif {nacm:default-deny-write;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-write is not allowed in notification statement.) /aa:notif/{extension='nacm:default-deny-write'}");

    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:nacm:aa; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "leaf l { type string; nacm:default-deny-write; nacm:default-deny-write;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-write is instantiated multiple times.) /aa:l/{extension='nacm:default-deny-write'}");

    s->func = NULL;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_deny_all, setup, teardown),
        cmocka_unit_test_setup_teardown(test_deny_write, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
