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
                           "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\
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
    struct state *st = *state;

    const char *prefix, *name;
    struct yin_arg_record *args = NULL;
    size_t prefix_len, name_len;
    /* create mock yin namespace in xml context */
    const char *data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" />";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    LY_ARRAY_FREE(args);

    assert_int_equal(yin_match_keyword(st->xml_ctx, "anydatax", strlen("anydatax"), prefix, prefix_len), YANG_NONE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "asdasd", strlen("asdasd"), prefix, prefix_len), YANG_NONE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "", 0, prefix, prefix_len), YANG_NONE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "anydata", strlen("anydata"), prefix, prefix_len), YANG_ANYDATA);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "anyxml", strlen("anyxml"), prefix, prefix_len), YANG_ANYXML);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "argument", strlen("argument"), prefix, prefix_len), YANG_ARGUMENT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "augment", strlen("augment"), prefix, prefix_len), YANG_AUGMENT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "base", strlen("base"), prefix, prefix_len), YANG_BASE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "belongs-to", strlen("belongs-to"), prefix, prefix_len), YANG_BELONGS_TO);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "bit", strlen("bit"), prefix, prefix_len), YANG_BIT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "case", strlen("case"), prefix, prefix_len), YANG_CASE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "choice", strlen("choice"), prefix, prefix_len), YANG_CHOICE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "config", strlen("config"), prefix, prefix_len), YANG_CONFIG);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "contact", strlen("contact"), prefix, prefix_len), YANG_CONTACT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "container", strlen("container"), prefix, prefix_len), YANG_CONTAINER);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "default", strlen("default"), prefix, prefix_len), YANG_DEFAULT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "description", strlen("description"), prefix, prefix_len), YANG_DESCRIPTION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "deviate", strlen("deviate"), prefix, prefix_len), YANG_DEVIATE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "deviation", strlen("deviation"), prefix, prefix_len), YANG_DEVIATION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "enum", strlen("enum"), prefix, prefix_len), YANG_ENUM);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "error-app-tag", strlen("error-app-tag"), prefix, prefix_len), YANG_ERROR_APP_TAG);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "error-message", strlen("error-message"), prefix, prefix_len), YANG_ERROR_MESSAGE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "extension", strlen("extension"), prefix, prefix_len), YANG_EXTENSION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "feature", strlen("feature"), prefix, prefix_len), YANG_FEATURE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "fraction-digits", strlen("fraction-digits"), prefix,  prefix_len), YANG_FRACTION_DIGITS);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "grouping", strlen("grouping"), prefix, prefix_len), YANG_GROUPING);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "identity", strlen("identity"), prefix, prefix_len), YANG_IDENTITY);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "if-feature", strlen("if-feature"), prefix, prefix_len), YANG_IF_FEATURE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "import", strlen("import"), prefix, prefix_len), YANG_IMPORT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "include", strlen("include"), prefix, prefix_len), YANG_INCLUDE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "input", strlen("input"), prefix, prefix_len), YANG_INPUT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "key", strlen("key"), prefix, prefix_len), YANG_KEY);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "leaf", strlen("leaf"), prefix, prefix_len), YANG_LEAF);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "leaf-list", strlen("leaf-list"), prefix, prefix_len), YANG_LEAF_LIST);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "length", strlen("length"), prefix, prefix_len), YANG_LENGTH);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "list", strlen("list"), prefix, prefix_len), YANG_LIST);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "mandatory", strlen("mandatory"), prefix, prefix_len), YANG_MANDATORY);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "max-elements", strlen("max-elements"), prefix, prefix_len), YANG_MAX_ELEMENTS);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "min-elements", strlen("min-elements"), prefix, prefix_len), YANG_MIN_ELEMENTS);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "modifier", strlen("modifier"), prefix, prefix_len), YANG_MODIFIER);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "module", strlen("module"), prefix, prefix_len), YANG_MODULE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "must", strlen("must"), prefix, prefix_len), YANG_MUST);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "namespace", strlen("namespace"), prefix, prefix_len), YANG_NAMESPACE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "notification", strlen("notification"), prefix, prefix_len), YANG_NOTIFICATION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "ordered-by", strlen("ordered-by"), prefix, prefix_len), YANG_ORDERED_BY);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "organization", strlen("organization"), prefix, prefix_len), YANG_ORGANIZATION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "output", strlen("output"), prefix, prefix_len), YANG_OUTPUT);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "path", strlen("path"), prefix, prefix_len), YANG_PATH);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "pattern", strlen("pattern"), prefix, prefix_len), YANG_PATTERN);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "position", strlen("position"), prefix, prefix_len), YANG_POSITION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "prefix", strlen("prefix"), prefix, prefix_len), YANG_PREFIX);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "presence", strlen("presence"), prefix, prefix_len), YANG_PRESENCE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "range", strlen("range"), prefix, prefix_len), YANG_RANGE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "reference", strlen("reference"), prefix, prefix_len), YANG_REFERENCE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "refine", strlen("refine"), prefix, prefix_len), YANG_REFINE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "require-instance", strlen("require-instance"), prefix, prefix_len), YANG_REQUIRE_INSTANCE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "revision", strlen("revision"), prefix, prefix_len), YANG_REVISION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "revision-date", strlen("revision-date"), prefix, prefix_len), YANG_REVISION_DATE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "rpc", strlen("rpc"), prefix, prefix_len), YANG_RPC);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "status", strlen("status"), prefix, prefix_len), YANG_STATUS);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "submodule", strlen("submodule"), prefix, prefix_len), YANG_SUBMODULE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "type", strlen("type"), prefix, prefix_len), YANG_TYPE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "typedef", strlen("typedef"), prefix, prefix_len), YANG_TYPEDEF);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "unique", strlen("unique"), prefix, prefix_len), YANG_UNIQUE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "units", strlen("units"), prefix, prefix_len), YANG_UNITS);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "uses", strlen("uses"), prefix, prefix_len), YANG_USES);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "value", strlen("value"), prefix, prefix_len), YANG_VALUE);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "when", strlen("when"), prefix, prefix_len), YANG_WHEN);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "yang-version", strlen("yang-version"), prefix, prefix_len), YANG_YANG_VERSION);
    assert_int_equal(yin_match_keyword(st->xml_ctx, "yin-element", strlen("yin-element"), prefix, prefix_len), YANG_YIN_ELEMENT);

    st->finished_correctly = true;
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
    struct yin_arg_record *args = NULL;

    const char *data = "<elem>content</elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_text_element(st->xml_ctx, &args, &data, &res);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(res, "content");
    lydict_remove(st->ctx, "content");
    st = reset_state(state);
    LY_ARRAY_FREE(args);
    args = NULL;

    data = "<elem xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">another-content</elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_text_element(st->xml_ctx, &args, &data, &res);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(res, "another-content");
    lydict_remove(st->ctx, "another-content");
    st = reset_state(state);
    LY_ARRAY_FREE(args);
    args = NULL;

    data = "<elem unknown=\"unknown\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">text</elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_text_element(st->xml_ctx, &args, &data, &res);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(res, "text");
    lydict_remove(st->ctx, "text");
    LY_ARRAY_FREE(args);
    args = NULL;

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
    struct yin_arg_record *args = NULL;

    const char *data = "<import xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" module=\"a\">\
                            <prefix value=\"a_mod\"/>\
                            <revision-date date=\"2015-01-01\"></revision-date>\
                            <description><text>import description</text></description>\
                            <reference><text>import reference</text></reference>\
                        </import>\
                        \
                        <import xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" module=\"a\">\
                            <prefix value=\"a_mod\"/>\
                            <revision-date date=\"2015-01-01\" />\
                        </import>";
    /* first import */
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_import(st->xml_ctx, &args, "b-mod", &data, &imports);
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
    LY_ARRAY_FREE(args);
    imports = NULL;
    args = NULL;

    /* second invalid import */
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_import(st->xml_ctx,  &args, "a_mod", &data, &imports);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Prefix \"a_mod\" already used as module prefix. Line number 1.");
    /* cleanup is supposed to be done by caller function */
    lydict_remove(st->ctx, imports->name);
    lydict_remove(st->ctx, imports->prefix);
    LY_ARRAY_FREE(imports);
    LY_ARRAY_FREE(args);
    imports = NULL;
    args = NULL;

    st = reset_state(state);
    /* import with unknown child element */
    data = "<import xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" module=\"a\">\
                <what value=\"a_mod\"/>\
            </import>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_import(st->xml_ctx, &args, "invalid_mod", &data, &imports);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Unexpected child element \"what\" of import element.");
    /* cleanup is supposed to be done by caller function */
    lydict_remove(st->ctx, imports->name);
    LY_ARRAY_FREE(imports);
    LY_ARRAY_FREE(args);

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
    struct yin_arg_record *args = NULL;

    /* try all valid values */
    const char *data = "<status value=\"current\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_status(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_CURR);
    LY_ARRAY_FREE(args);
    args = NULL;

    st = reset_state(state);
    flags = 0;
    data = "<status value=\"deprecated\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_status(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_DEPRC);
    LY_ARRAY_FREE(args);
    args = NULL;

    st = reset_state(state);
    flags = 0;
    data = "<status value=\"obsolete\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_status(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_OBSLT);
    LY_ARRAY_FREE(args);
    args = NULL;

    /* duplicit definition (no reset_state() call) */
    data = "<status value=\"deprecated\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_status(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Duplicate element \"status\". Line number 1.");
    LY_ARRAY_FREE(args);
    args = NULL;

    /* invalid status value */
    st = reset_state(state);
    flags = 0;
    data = "<status value=\"dunno\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_status(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Invalid value \"dunno\" of \"status\". Line number 1.");
    LY_ARRAY_FREE(args);

    st->finished_correctly = true;
}

static void
test_yin_parse_extension(void **state)
{
    struct state *st = *state;
    const char *prefix = NULL, *name = NULL;
    size_t prefix_len = 0, name_len = 0;
    LY_ERR ret = LY_SUCCESS;
    struct yin_arg_record *args = NULL;
    struct lysp_ext *exts = NULL, *iter = NULL;

    const char *data = "<extension name=\"b\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\
                            <argument name=\"argname\"></argument>\
                            <description><text>desc</text></description>\
                            <reference><text>ref</text></reference>\
                            <status value=\"deprecated\"></status>\
                        </extension>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_extension(st->xml_ctx, &args, &data, &exts);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    LY_ARRAY_FOR_ITER(exts, struct lysp_ext, iter) {
        assert_string_equal(iter->name, "b");
        assert_string_equal(iter->dsc, "desc");
        assert_string_equal(iter->ref, "ref");
        assert_string_equal(iter->argument, "argname");
        assert_true(iter->flags & LYS_STATUS_DEPRC);
    }
    lydict_remove(st->ctx, "b");
    lydict_remove(st->ctx, "desc");
    lydict_remove(st->ctx, "ref");
    lydict_remove(st->ctx, "argname");
    LY_ARRAY_FREE(args);
    LY_ARRAY_FREE(exts);
    st->finished_correctly = true;
}

static void
test_yin_parse_yin_element_element(void **state)
{
    struct state *st = *state;
    const char *prefix = NULL, *name = NULL;
    size_t prefix_len = 0, name_len = 0;
    LY_ERR ret = LY_SUCCESS;
    uint16_t flags = 0;
    struct lysp_ext *exts;
    struct yin_arg_record *args = NULL;

    /* try all valid values */
    const char *data = "<yin-element value=\"true\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_yin_element_element(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_TRUE);
    LY_ARRAY_FREE(args);
    args = NULL;

    st = reset_state(state);
    flags = 0;
    data = "<yin-element value=\"false\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_yin_element_element(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_FALSE);
    LY_ARRAY_FREE(args);
    args = NULL;

    /* invalid value */
    st = reset_state(state);
    flags = 0;
    data = "<yin-element value=\"invalid\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_yin_element_element(st->xml_ctx, &args, &data, &flags, &exts);
    assert_int_equal(ret, LY_EVALID);
    LY_ARRAY_FREE(args);
    args = NULL;

    st->finished_correctly = true;
}

static void
test_yin_parse_element_generic(void **state)
{
    const char *prefix, *name;
    struct state *st = *state;
    struct lysp_ext_instance exts;
    size_t prefix_len, name_len;
    LY_ERR ret;

    memset(&exts, 0, sizeof(exts));

    const char *data = "<elem attr=\"value\">text_value</elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_element_generic(st->xml_ctx, name, name_len, prefix, prefix_len, &data, &exts.child);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(exts.child->stmt, "elem");
    assert_string_equal(exts.child->arg, "text_value");
    assert_string_equal(exts.child->child->stmt, "attr");
    assert_string_equal(exts.child->child->arg, "value");
    assert_true(exts.child->child->flags & LYS_YIN_ATTR);
    lysp_ext_instance_free(st->ctx, &exts);
    st = reset_state(state);

    data = "<elem></elem>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_element_generic(st->xml_ctx, name, name_len, prefix, prefix_len, &data, &exts.child);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(exts.child->stmt, "elem");
    assert_null(exts.child->child);
    assert_null(exts.child->arg);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    lysp_ext_instance_free(st->ctx, &exts);

    st->finished_correctly = true;
}

static void
test_yin_parse_extension_instance(void **state)
{
    LY_ERR ret;
    struct state *st = *state;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct yin_arg_record *args = NULL;
    struct lysp_ext_instance *exts = NULL;

    const char *data = "<ext value1=\"test\" value=\"test2\"><subelem>text</subelem></ext>";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_extension_instance(st->xml_ctx, &args, &data, nameprefix2fullname(name, prefix_len),
                                       namelen2fulllen(name_len, prefix_len), LYEXT_SUBSTMT_CONTACT, 0, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(exts->name, "ext");
    assert_int_equal(exts->insubstmt_index, 0);
    assert_true(exts->insubstmt == LYEXT_SUBSTMT_CONTACT);
    assert_true(exts->yin & LYS_YIN);
    assert_string_equal(exts->child->stmt, "value1");
    assert_string_equal(exts->child->arg, "test");
    assert_null(exts->child->child);
    assert_true(exts->child->flags & LYS_YIN_ATTR);
    assert_string_equal(exts->child->next->stmt, "value");
    assert_string_equal(exts->child->next->arg, "test2");
    assert_null(exts->child->next->child);
    assert_true(exts->child->next->flags & LYS_YIN_ATTR);

    assert_string_equal(exts->child->next->next->stmt, "subelem");
    assert_string_equal(exts->child->next->next->arg, "text");
    assert_null(exts->child->next->next->child);
    assert_null(exts->child->next->next->next);
    assert_false(exts->child->next->next->flags & LYS_YIN_ATTR);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    LY_ARRAY_FREE(args);
    lysp_ext_instance_free(st->ctx, exts);
    LY_ARRAY_FREE(exts);
    exts = NULL;
    args = NULL;
    st = reset_state(state);

    data = "<extension-elem />";
    lyxml_get_element(st->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->xml_ctx, &data, &args);
    ret = yin_parse_extension_instance(st->xml_ctx, &args, &data, name, name_len, LYEXT_SUBSTMT_CONTACT, 0, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(exts->name, "extension-elem");
    assert_null(exts->argument);
    assert_null(exts->child);
    assert_int_equal(exts->insubstmt, LYEXT_SUBSTMT_CONTACT);
    assert_int_equal(exts->insubstmt_index, 0);
    assert_true(exts->yin & LYS_YIN);
    assert_int_equal(st->xml_ctx->status, LYXML_END);
    LY_ARRAY_FREE(args);
    lysp_ext_instance_free(st->ctx, exts);
    LY_ARRAY_FREE(exts);
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
        cmocka_unit_test_setup_teardown(test_yin_match_keyword, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_extension, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_yin_element_element, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_element_generic, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_extension_instance, setup_f, teardown_f),
        cmocka_unit_test(test_yin_match_argument_name),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
