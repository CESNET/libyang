/**
 * @file test_parser_yin.c
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @brief unit tests for functions from parser_yin.c
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
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
#include <stdbool.h>

#include "../../src/common.h"
#include "../../src/tree_schema.h"
#include "../../src/tree_schema_internal.h"
#include "../../src/parser_yin.h"
#include "../../src/xml.h"

struct state {
    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lyxml_context *xml_ctx;
    bool finished_correctly;
};

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

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

#define TEST_DUP_GENERIC(PREFIX, MEMBER, VALUE1, VALUE2, FUNC, RESULT, LINE, CLEANUP) \
    str = PREFIX MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, FUNC(&ctx, &str, RESULT)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    CLEANUP


static int
setup_f(void **state)
{
    struct state *st = NULL;

#if ENABLE_LOGGER_CHECKING
    /* setup logger */
    ly_set_log_clb(logger, 1);
#endif

    /* allocate state variable */
    (*state) = st = calloc(1, sizeof(*st));
    if (!st) {
        fprintf(stderr, "Memmory allocation failed");
        return EXIT_FAILURE;
    }

    /* create new libyang context */
    ly_ctx_new(NULL, 0, &st->ctx);

    /* allocate new module */
    st->mod = calloc(1, sizeof(*st->mod));
    st->mod->ctx = st->ctx;

    st->xml_ctx = calloc(1, sizeof(struct lys_parser_ctx));
    st->xml_ctx->ctx = st->ctx;
    st->xml_ctx->line = 1;

    return EXIT_SUCCESS;
}

static int
teardown_f(void **state)
{
    struct state *st = *(struct state **)state;

#if ENABLE_LOGGER_CHECKING
    /* teardown logger */
    if (!st->finished_correctly && logbuf[0] != '\0') {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif

    lyxml_context_clear(st->xml_ctx);
    lys_module_free(st->mod, NULL);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml_ctx);
    free(st);

    return EXIT_SUCCESS;
}

static struct state*
reset_state(void **state)
{
    ((struct state *)*state)->finished_correctly = true;
    teardown_f(state);
    setup_f(state);

    return *state;
}

void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

static void
test_yin_parse_module(void **state)
{
    LY_ERR ret = LY_SUCCESS;
    struct state *st = *state;

    ret = yin_parse_module(st->ctx,
                    "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\
                        name=\"example-foo\"\
                        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\
                        xmlns:foo=\"urn:example:foo\"\
                        xmlns:myext=\"urn:example:extensions\">\
                        <namespace uri=\"urn:example:foo\" xmlns:myext=\"urn:example:extensions\"/>\
                        <prefix xmlns:myxt=\"urn:emple:extensions\" value=\"foo\" xmlns:myext=\"urn:example:extensions\"/>\
                     </module>",
                st->mod);

    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(st->mod->parsed->mod->name, "example-foo");
    assert_string_equal(st->mod->parsed->mod->prefix, "foo");
    assert_string_equal(st->mod->parsed->mod->ns, "urn:example:foo");

    st = reset_state(state);
    ret = yin_parse_module(st->ctx,
                           "<module name=\"example-foo\">\
                                <invalid-tag uri=\"urn:example:foo\"\"/>\
                            </module>",
                            st->mod);
    assert_int_equal(ret, LY_EVALID);

    st = reset_state(state);
    ret = yin_parse_module(st->ctx,
                           "<module>\
                            </module>",
                           st->mod);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Missing argument name of a module Line number 1.");

    st = reset_state(state);
    ret = yin_parse_module(st->ctx,
                    "",
                st->mod);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Invalid keyword \"(null)\", expected \"module\" or \"submodule\". Line number 1.");
    st->finished_correctly = true;
}

static void
test_yin_match_keyword(void **state)
{
    (void)state; /* unused */

    assert_int_equal(yin_match_keyword("anydatax", strlen("anydatax")), YANG_NONE);
    assert_int_equal(yin_match_keyword("asdasd", strlen("asdasd")), YANG_NONE);
    assert_int_equal(yin_match_keyword("", 0), YANG_NONE);
    assert_int_equal(yin_match_keyword("anydata", strlen("anydata")), YANG_ANYDATA);
    assert_int_equal(yin_match_keyword("anyxml", strlen("anyxml")), YANG_ANYXML);
    assert_int_equal(yin_match_keyword("argument", strlen("argument")), YANG_ARGUMENT);
    assert_int_equal(yin_match_keyword("augment", strlen("augment")), YANG_AUGMENT);
    assert_int_equal(yin_match_keyword("base", strlen("base")), YANG_BASE);
    assert_int_equal(yin_match_keyword("belongs-to", strlen("belongs-to")), YANG_BELONGS_TO);
    assert_int_equal(yin_match_keyword("bit", strlen("bit")), YANG_BIT);
    assert_int_equal(yin_match_keyword("case", strlen("case")), YANG_CASE);
    assert_int_equal(yin_match_keyword("choice", strlen("choice")), YANG_CHOICE);
    assert_int_equal(yin_match_keyword("config", strlen("config")), YANG_CONFIG);
    assert_int_equal(yin_match_keyword("contact", strlen("contact")), YANG_CONTACT);
    assert_int_equal(yin_match_keyword("container", strlen("container")), YANG_CONTAINER);
    assert_int_equal(yin_match_keyword("default", strlen("default")), YANG_DEFAULT);
    assert_int_equal(yin_match_keyword("description", strlen("description")), YANG_DESCRIPTION);
    assert_int_equal(yin_match_keyword("deviate", strlen("deviate")), YANG_DEVIATE);
    assert_int_equal(yin_match_keyword("deviation", strlen("deviation")), YANG_DEVIATION);
    assert_int_equal(yin_match_keyword("enum", strlen("enum")), YANG_ENUM);
    assert_int_equal(yin_match_keyword("error-app-tag", strlen("error-app-tag")), YANG_ERROR_APP_TAG);
    assert_int_equal(yin_match_keyword("error-message", strlen("error-message")), YANG_ERROR_MESSAGE);
    assert_int_equal(yin_match_keyword("extension", strlen("extension")), YANG_EXTENSION);
    assert_int_equal(yin_match_keyword("feature", strlen("feature")), YANG_FEATURE);
    assert_int_equal(yin_match_keyword("fraction-digits", strlen("fraction-digits")), YANG_FRACTION_DIGITS);
    assert_int_equal(yin_match_keyword("grouping", strlen("grouping")), YANG_GROUPING);
    assert_int_equal(yin_match_keyword("identity", strlen("identity")), YANG_IDENTITY);
    assert_int_equal(yin_match_keyword("if-feature", strlen("if-feature")), YANG_IF_FEATURE);
    assert_int_equal(yin_match_keyword("import", strlen("import")), YANG_IMPORT);
    assert_int_equal(yin_match_keyword("include", strlen("include")), YANG_INCLUDE);
    assert_int_equal(yin_match_keyword("input", strlen("input")), YANG_INPUT);
    assert_int_equal(yin_match_keyword("key", strlen("key")), YANG_KEY);
    assert_int_equal(yin_match_keyword("leaf", strlen("leaf")), YANG_LEAF);
    assert_int_equal(yin_match_keyword("leaf-list", strlen("leaf-list")), YANG_LEAF_LIST);
    assert_int_equal(yin_match_keyword("length", strlen("length")), YANG_LENGTH);
    assert_int_equal(yin_match_keyword("list", strlen("list")), YANG_LIST);
    assert_int_equal(yin_match_keyword("mandatory", strlen("mandatory")), YANG_MANDATORY);
    assert_int_equal(yin_match_keyword("max-elements", strlen("max-elements")), YANG_MAX_ELEMENTS);
    assert_int_equal(yin_match_keyword("min-elements", strlen("min-elements")), YANG_MIN_ELEMENTS);
    assert_int_equal(yin_match_keyword("modifier", strlen("modifier")), YANG_MODIFIER);
    assert_int_equal(yin_match_keyword("module", strlen("module")), YANG_MODULE);
    assert_int_equal(yin_match_keyword("must", strlen("must")), YANG_MUST);
    assert_int_equal(yin_match_keyword("namespace", strlen("namespace")), YANG_NAMESPACE);
    assert_int_equal(yin_match_keyword("notification", strlen("notification")), YANG_NOTIFICATION);
    assert_int_equal(yin_match_keyword("ordered-by", strlen("ordered-by")), YANG_ORDERED_BY);
    assert_int_equal(yin_match_keyword("organization", strlen("organization")), YANG_ORGANIZATION);
    assert_int_equal(yin_match_keyword("output", strlen("output")), YANG_OUTPUT);
    assert_int_equal(yin_match_keyword("path", strlen("path")), YANG_PATH);
    assert_int_equal(yin_match_keyword("pattern", strlen("pattern")), YANG_PATTERN);
    assert_int_equal(yin_match_keyword("position", strlen("position")), YANG_POSITION);
    assert_int_equal(yin_match_keyword("prefix", strlen("prefix")), YANG_PREFIX);
    assert_int_equal(yin_match_keyword("presence", strlen("presence")), YANG_PRESENCE);
    assert_int_equal(yin_match_keyword("range", strlen("range")), YANG_RANGE);
    assert_int_equal(yin_match_keyword("reference", strlen("reference")), YANG_REFERENCE);
    assert_int_equal(yin_match_keyword("refine", strlen("refine")), YANG_REFINE);
    assert_int_equal(yin_match_keyword("require-instance", strlen("require-instance")), YANG_REQUIRE_INSTANCE);
    assert_int_equal(yin_match_keyword("revision", strlen("revision")), YANG_REVISION);
    assert_int_equal(yin_match_keyword("revision-date", strlen("revision-date")), YANG_REVISION_DATE);
    assert_int_equal(yin_match_keyword("rpc", strlen("rpc")), YANG_RPC);
    assert_int_equal(yin_match_keyword("status", strlen("status")), YANG_STATUS);
    assert_int_equal(yin_match_keyword("submodule", strlen("submodule")), YANG_SUBMODULE);
    assert_int_equal(yin_match_keyword("type", strlen("type")), YANG_TYPE);
    assert_int_equal(yin_match_keyword("typedef", strlen("typedef")), YANG_TYPEDEF);
    assert_int_equal(yin_match_keyword("unique", strlen("unique")), YANG_UNIQUE);
    assert_int_equal(yin_match_keyword("units", strlen("units")), YANG_UNITS);
    assert_int_equal(yin_match_keyword("uses", strlen("uses")), YANG_USES);
    assert_int_equal(yin_match_keyword("value", strlen("value")), YANG_VALUE);
    assert_int_equal(yin_match_keyword("when", strlen("when")), YANG_WHEN);
    assert_int_equal(yin_match_keyword("yang-version", strlen("yang-version")), YANG_YANG_VERSION);
    assert_int_equal(yin_match_keyword("yin-element", strlen("yin-element")), YANG_YIN_ELEMENT);
}

static void
test_yin_match_argument_name(void **state)
{
    (void)state; /* unused */

    assert_int_equal(yin_match_argument_name("", 5), YIN_ARG_UNKNOWN);
    assert_int_equal(yin_match_argument_name("qwertyasd", 5), YIN_ARG_UNKNOWN);
    assert_int_equal(yin_match_argument_name("conditionasd", 8), YIN_ARG_UNKNOWN);
    assert_int_equal(yin_match_argument_name("condition", 9), YIN_ARG_CONDITION);
    assert_int_equal(yin_match_argument_name("date", 4), YIN_ARG_DATE);
    assert_int_equal(yin_match_argument_name("module", 6), YIN_ARG_MODULE);
    assert_int_equal(yin_match_argument_name("name", 4), YIN_ARG_NAME);
    assert_int_equal(yin_match_argument_name("tag", 3), YIN_ARG_TAG);
    assert_int_equal(yin_match_argument_name("target-node", 11), YIN_ARG_TARGET_NODE);
    assert_int_equal(yin_match_argument_name("text", 4), YIN_ARG_TEXT);
    assert_int_equal(yin_match_argument_name("uri", 3), YIN_ARG_URI);
    assert_int_equal(yin_match_argument_name("value", 5), YIN_ARG_VALUE);
}

static void
test_meta(void **state)
{
    LY_ERR ret = LY_SUCCESS;
    struct state *st = *state;

    ret = yin_parse_module(st->ctx,"<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\
                                        name=\"example-foo\">\
                                        <organization xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><text>organization...</text></organization>\
                                        <contact><text>contact...</text></contact>\
                                        <description><text>description...</text></description>\
                                        <reference><text>reference...</text></reference>\
                                    </module>", st->mod);

    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(st->mod->parsed->mod->org, "organization...");
    assert_string_equal(st->mod->parsed->mod->contact, "contact...");
    assert_string_equal(st->mod->parsed->mod->dsc, "description...");
    assert_string_equal(st->mod->parsed->mod->ref, "reference...");

    st = reset_state(state);
    ret = yin_parse_module(st->ctx,"<module name=\"example-foo\">\
                                        <organization test=\"invalid-argument\">organization...</organization>\
                                    </module>", st->mod);
    assert_int_equal(ret, LY_EVALID);

    st->finished_correctly = true;
}

static void
test_yin_parse_text_element(void **state)
{
    struct state *st = *state;
    const char *res = NULL, *prefix = NULL, *name = NULL;
    size_t prefix_len = 0, name_len = 0;
    LY_ERR ret = LY_SUCCESS;

    const char *data = "<elem>content</elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_parse_text_element(st->xml_ctx, &data, &res);
    assert_string_equal(res, "content");
    lydict_remove(st->ctx, "content");
    st = reset_state(state);

    data = "<elem xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">another-content</elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_parse_text_element(st->xml_ctx, &data, &res);
    assert_string_equal(res, "another-content");
    lydict_remove(st->ctx, "another-content");
    st = reset_state(state);

    data = "<elem invalid=\"invalid\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">text</elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_text_element(st->xml_ctx, &data, &res);
    assert_int_equal(ret, LY_EVALID);

    st->finished_correctly = true;
}

static void
test_yin_parse_import(void **state)
{
    struct state *st = *state;
    const char *prefix = NULL, *name = NULL;
    size_t prefix_len = 0, name_len = 0;
    LY_ERR ret = LY_SUCCESS;
    struct lysp_import *imports = NULL;

    const char *data = "<import xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" module=\"a\">\
                            <prefix value=\"a_mod\"/>\
                            <revision-date date=\"2015-01-01\"/>\
                            <description><text>import description</text></description>\
                            <reference><text>import reference</text></reference>\
                        </import>\
                        \
                        <import xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" module=\"a\">\
                            <prefix value=\"a_mod\"/>\
                            <revision-date date=\"2015-01-01\"/>\
                        </import>";
    /* first import */
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_import(st->xml_ctx, "b-mod", &data, &imports);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(imports->name, "a");
    assert_string_equal(imports->prefix, "a_mod");
    assert_string_equal(imports->rev, "2015-01-01");
    assert_string_equal(imports->dsc, "import description");
    assert_string_equal(imports->ref, "import reference");
    lydict_remove(st->ctx, imports->name);
    lydict_remove(st->ctx, imports->prefix);
    lydict_remove(st->ctx, imports->dsc);
    lydict_remove(st->ctx, imports->ref);
    LY_ARRAY_FREE(imports);
    imports = NULL;

    /* second invalid import */
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_import(st->xml_ctx, "a_mod", &data, &imports);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Prefix \"a_mod\" already used as module prefix. Line number 1.");
    /* cleanup is supposed to be done by caller function */
    lydict_remove(st->ctx, imports->name);
    lydict_remove(st->ctx, imports->prefix);
    LY_ARRAY_FREE(imports);

    imports = NULL;
    st = reset_state(state);
    /* import with unknown child element */
    data = "<import xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" module=\"a\">\
                <what value=\"a_mod\"/>\
            </import>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_import(st->xml_ctx, "invalid_mod", &data, &imports);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Unexpected child element \"what\" of import element.");
    /* cleanup is supposed to be done by caller function */
    lydict_remove(st->ctx, imports->name);
    LY_ARRAY_FREE(imports);

    st->finished_correctly = true;
}

static void
test_yin_parse_status(void **state)
{
    struct state *st = *state;
    const char *prefix = NULL, *name = NULL;
    size_t prefix_len = 0, name_len = 0;
    LY_ERR ret = LY_SUCCESS;
    uint16_t flags = 0;
    struct lysp_ext_instance *exts;

    /* try all valid values */
    const char *data = "<status value=\"current\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_status(st->xml_ctx, &data, &flags, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_CURR);

    st = reset_state(state);
    flags = 0;
    data = "<status value=\"deprecated\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_status(st->xml_ctx, &data, &flags, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_DEPRC);
    st->finished_correctly = true;

    st = reset_state(state);
    flags = 0;
    data = "<status value=\"obsolete\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_status(st->xml_ctx, &data, &flags, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_OBSLT);
    st->finished_correctly = true;

    /* duplicit definition (no reset_state() call) */
    data = "<status value=\"deprecated\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_status(st->xml_ctx, &data, &flags, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Duplicate element \"status\". Line number 1.");

    /* invalid status value */
    st = reset_state(state);
    flags = 0;
    data = "<status value=\"dunno\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_status(st->xml_ctx, &data, &flags, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Invalid value \"dunno\" of \"status\". Line number 1.");
    st->finished_correctly = true;
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_yin_parse_module, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_meta, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_text_element, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_import, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_status, setup_f, teardown_f),
        cmocka_unit_test(test_yin_match_keyword),
        cmocka_unit_test(test_yin_match_argument_name),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
