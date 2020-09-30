/*
 * @file test_metadata.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for Metadata extension (annotation) support
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

#include "libyang.h"
#include "plugins_exts_metadata.h"

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

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &s->ctx));
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
test_yang(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_yang;

    const struct lys_module *mod;
    struct lysc_ext_instance *e;
    struct lyext_metadata *ant;

    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:metadata:a; prefix a;"
            "import ietf-yang-metadata {prefix md;}"
            "feature f;"
            "md:annotation x {"
            "  description \"test\";"
            "  if-feature f;"
            "  reference \"test\";"
            "  status \"current\";"
            "  type uint8;"
            "  units meters;"
            "}}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(s->ctx, data, LYS_IN_YANG, &mod));
    assert_int_equal(1, LY_ARRAY_COUNT(mod->compiled->exts));
    e = &mod->compiled->exts[0];
    assert_non_null(ant = (struct lyext_metadata*)e->data);
    assert_string_equal("meters", ant->units);

    /* invalid */
    /* missing mandatory type substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa;}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Missing mandatory keyword \"type\" as a child of \"md:annotation aa\". /aa:{extension='md:annotation'}/aa");

    /* not allowed substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {default x;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Invalid keyword \"default\" as a child of \"md:annotation aa\" extension instance. /aa:{extension='md:annotation'}/aa");

    /* invalid cardinality of units substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string; units x; units y;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Duplicate keyword \"units\". /aa:{extension='md:annotation'}/aa");

    /* invalid cardinality of status substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string; status current; status obsolete;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Duplicate keyword \"status\". /aa:{extension='md:annotation'}/aa");

    /* invalid cardinality of status substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string; type uint8;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Duplicate keyword \"type\". /aa:{extension='md:annotation'}/aa");

    /* duplication of the same annotation */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string;} md:annotation aa {type uint8;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YANG, NULL));
    logbuf_assert("Extension plugin \"libyang 2 - metadata, version 1\": "
            "Extension md:annotation is instantiated multiple times.) /aa:{extension='md:annotation'}/aa");

    s->func = NULL;
}

static void
test_yin(void **state)
{
    struct state_s *s = (struct state_s*)(*state);
    s->func = test_yin;

    const struct lys_module *mod;
    struct lysc_ext_instance *e;
    struct lyext_metadata *ant;
    const char *data;

    /* TODO yin-extension-prefix-compilation-bug
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"a\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:a\"/><prefix value=\"a\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<feature name=\"f\"/>\n"
            "<md:annotation name=\"x\">\n"
            "  <description><text>test</text></description>\n"
            "  <reference><text>test</text></reference>\n"
            "  <if-feature name=\"f\"/>\n"
            "  <status value=\"current\"/>\n"
            "  <type name=\"uint8\"/>\n"
            "  <units name=\"meters\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(s->ctx, data, LYS_IN_YIN, &mod));
    assert_int_equal(1, LY_ARRAY_COUNT(mod->compiled->exts));
    e = &mod->compiled->exts[0];
    assert_non_null(ant = (struct lyext_metadata*)e->data);
    assert_string_equal("meters", ant->units);*/

    /* invalid */
    /* missing mandatory type substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\"/>\n"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YIN, NULL));
    logbuf_assert("Missing mandatory keyword \"type\" as a child of \"md:annotation aa\". /aa:{extension='md:annotation'}/aa");

    /* not allowed substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <default value=\"x\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YIN, NULL));
    logbuf_assert("Invalid keyword \"default\" as a child of \"md:annotation aa\" extension instance. /aa:{extension='md:annotation'}/aa");

    /* invalid cardinality of units substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "  <units name=\"x\"/>\n"
            "  <units name=\"y\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YIN, NULL));
    logbuf_assert("Duplicate keyword \"units\". /aa:{extension='md:annotation'}/aa");

    /* invalid cardinality of status substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "  <status value=\"current\"/>\n"
            "  <status value=\"obsolete\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YIN, NULL));
    logbuf_assert("Duplicate keyword \"status\". /aa:{extension='md:annotation'}/aa");

    /* invalid cardinality of status substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "  <type name=\"uint8\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YIN, NULL));
    logbuf_assert("Duplicate keyword \"type\". /aa:{extension='md:annotation'}/aa");

    /* duplication of the same annotation */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "</md:annotation><md:annotation name=\"aa\">\n"
            "  <type name=\"uint8\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(s->ctx, data, LYS_IN_YIN, NULL));
    logbuf_assert("Extension plugin \"libyang 2 - metadata, version 1\": "
            "Extension md:annotation is instantiated multiple times.) /aa:{extension='md:annotation'}/aa");
    s->func = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_yang, setup, teardown),
        cmocka_unit_test_setup_teardown(test_yin, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
