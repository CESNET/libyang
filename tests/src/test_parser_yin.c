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

/* prototypes of static functions */
void lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext);
void lysp_ext_free(struct ly_ctx *ctx, struct lysp_ext *ext);
void lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when);

struct state {
    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysp_module *lysp_mod;
    struct yin_parser_ctx *yin_ctx;
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

    /* allocate new parsed module */
    st->lysp_mod = calloc(1, sizeof(*st->lysp_mod));
    st->lysp_mod->mod = calloc(1, sizeof(*st->lysp_mod->mod));
    st->lysp_mod->mod->ctx = st->ctx;

    /* allocate parser context */
    st->yin_ctx = calloc(1, sizeof(*st->yin_ctx));
    st->yin_ctx->xml_ctx.ctx = st->ctx;
    st->yin_ctx->xml_ctx.line = 1;

    return EXIT_SUCCESS;
}

static int
teardown_f(void **state)
{
    struct state *st = *(struct state **)state;
    struct lys_module *temp;

#if ENABLE_LOGGER_CHECKING
    /* teardown logger */
    if (!st->finished_correctly && logbuf[0] != '\0') {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif

    temp = st->lysp_mod->mod;

    lyxml_context_clear(&st->yin_ctx->xml_ctx);
    lys_module_free(st->mod, NULL);
    lysp_module_free(st->lysp_mod);
    lys_module_free(temp, NULL);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->yin_ctx);
    free(st);

    return EXIT_SUCCESS;
}

static struct state*
reset_state(void **state)
{
    ((struct state *)*state)->finished_correctly = true;
    logbuf[0] = '\0';
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
    logbuf_assert("Missing mandatory attribute name of module element. Line number 1.");

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
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    LY_ARRAY_FREE(args);

    assert_int_equal(yin_match_keyword(st->yin_ctx, "anydatax", strlen("anydatax"), prefix, prefix_len, YANG_NONE), YANG_NONE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "asdasd", strlen("asdasd"), prefix, prefix_len, YANG_NONE), YANG_NONE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "", 0, prefix, prefix_len, YANG_NONE), YANG_NONE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "anydata", strlen("anydata"), prefix, prefix_len, YANG_NONE), YANG_ANYDATA);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "anyxml", strlen("anyxml"), prefix, prefix_len, YANG_NONE), YANG_ANYXML);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "argument", strlen("argument"), prefix, prefix_len, YANG_NONE), YANG_ARGUMENT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "augment", strlen("augment"), prefix, prefix_len, YANG_NONE), YANG_AUGMENT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "base", strlen("base"), prefix, prefix_len, YANG_NONE), YANG_BASE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "belongs-to", strlen("belongs-to"), prefix, prefix_len, YANG_NONE), YANG_BELONGS_TO);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "bit", strlen("bit"), prefix, prefix_len, YANG_NONE), YANG_BIT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "case", strlen("case"), prefix, prefix_len, YANG_NONE), YANG_CASE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "choice", strlen("choice"), prefix, prefix_len, YANG_NONE), YANG_CHOICE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "config", strlen("config"), prefix, prefix_len, YANG_NONE), YANG_CONFIG);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "contact", strlen("contact"), prefix, prefix_len, YANG_NONE), YANG_CONTACT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "container", strlen("container"), prefix, prefix_len, YANG_NONE), YANG_CONTAINER);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "default", strlen("default"), prefix, prefix_len, YANG_NONE), YANG_DEFAULT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "description", strlen("description"), prefix, prefix_len, YANG_NONE), YANG_DESCRIPTION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "deviate", strlen("deviate"), prefix, prefix_len, YANG_NONE), YANG_DEVIATE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "deviation", strlen("deviation"), prefix, prefix_len, YANG_NONE), YANG_DEVIATION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "enum", strlen("enum"), prefix, prefix_len, YANG_NONE), YANG_ENUM);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "error-app-tag", strlen("error-app-tag"), prefix, prefix_len, YANG_NONE), YANG_ERROR_APP_TAG);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "error-message", strlen("error-message"), prefix, prefix_len, YANG_NONE), YANG_ERROR_MESSAGE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "extension", strlen("extension"), prefix, prefix_len, YANG_NONE), YANG_EXTENSION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "feature", strlen("feature"), prefix, prefix_len, YANG_NONE), YANG_FEATURE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "fraction-digits", strlen("fraction-digits"), prefix,  prefix_len, YANG_NONE), YANG_FRACTION_DIGITS);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "grouping", strlen("grouping"), prefix, prefix_len, YANG_NONE), YANG_GROUPING);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "identity", strlen("identity"), prefix, prefix_len, YANG_NONE), YANG_IDENTITY);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "if-feature", strlen("if-feature"), prefix, prefix_len, YANG_NONE), YANG_IF_FEATURE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "import", strlen("import"), prefix, prefix_len, YANG_NONE), YANG_IMPORT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "include", strlen("include"), prefix, prefix_len, YANG_NONE), YANG_INCLUDE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "input", strlen("input"), prefix, prefix_len, YANG_NONE), YANG_INPUT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "key", strlen("key"), prefix, prefix_len, YANG_NONE), YANG_KEY);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "leaf", strlen("leaf"), prefix, prefix_len, YANG_NONE), YANG_LEAF);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "leaf-list", strlen("leaf-list"), prefix, prefix_len, YANG_NONE), YANG_LEAF_LIST);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "length", strlen("length"), prefix, prefix_len, YANG_NONE), YANG_LENGTH);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "list", strlen("list"), prefix, prefix_len, YANG_NONE), YANG_LIST);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "mandatory", strlen("mandatory"), prefix, prefix_len, YANG_NONE), YANG_MANDATORY);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "max-elements", strlen("max-elements"), prefix, prefix_len, YANG_NONE), YANG_MAX_ELEMENTS);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "min-elements", strlen("min-elements"), prefix, prefix_len, YANG_NONE), YANG_MIN_ELEMENTS);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "modifier", strlen("modifier"), prefix, prefix_len, YANG_NONE), YANG_MODIFIER);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "module", strlen("module"), prefix, prefix_len, YANG_NONE), YANG_MODULE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "must", strlen("must"), prefix, prefix_len, YANG_NONE), YANG_MUST);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "namespace", strlen("namespace"), prefix, prefix_len, YANG_NONE), YANG_NAMESPACE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "notification", strlen("notification"), prefix, prefix_len, YANG_NONE), YANG_NOTIFICATION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "ordered-by", strlen("ordered-by"), prefix, prefix_len, YANG_NONE), YANG_ORDERED_BY);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "organization", strlen("organization"), prefix, prefix_len, YANG_NONE), YANG_ORGANIZATION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "output", strlen("output"), prefix, prefix_len, YANG_NONE), YANG_OUTPUT);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "path", strlen("path"), prefix, prefix_len, YANG_NONE), YANG_PATH);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "pattern", strlen("pattern"), prefix, prefix_len, YANG_NONE), YANG_PATTERN);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "position", strlen("position"), prefix, prefix_len, YANG_NONE), YANG_POSITION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "prefix", strlen("prefix"), prefix, prefix_len, YANG_NONE), YANG_PREFIX);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "presence", strlen("presence"), prefix, prefix_len, YANG_NONE), YANG_PRESENCE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "range", strlen("range"), prefix, prefix_len, YANG_NONE), YANG_RANGE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "reference", strlen("reference"), prefix, prefix_len, YANG_NONE), YANG_REFERENCE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "refine", strlen("refine"), prefix, prefix_len, YANG_NONE), YANG_REFINE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "require-instance", strlen("require-instance"), prefix, prefix_len, YANG_NONE), YANG_REQUIRE_INSTANCE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "revision", strlen("revision"), prefix, prefix_len, YANG_NONE), YANG_REVISION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "revision-date", strlen("revision-date"), prefix, prefix_len, YANG_NONE), YANG_REVISION_DATE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "rpc", strlen("rpc"), prefix, prefix_len, YANG_NONE), YANG_RPC);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "status", strlen("status"), prefix, prefix_len, YANG_NONE), YANG_STATUS);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "submodule", strlen("submodule"), prefix, prefix_len, YANG_NONE), YANG_SUBMODULE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "type", strlen("type"), prefix, prefix_len, YANG_NONE), YANG_TYPE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "typedef", strlen("typedef"), prefix, prefix_len, YANG_NONE), YANG_TYPEDEF);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "unique", strlen("unique"), prefix, prefix_len, YANG_NONE), YANG_UNIQUE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "units", strlen("units"), prefix, prefix_len, YANG_NONE), YANG_UNITS);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "uses", strlen("uses"), prefix, prefix_len, YANG_NONE), YANG_USES);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "value", strlen("value"), prefix, prefix_len, YANG_NONE), YANG_VALUE);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "when", strlen("when"), prefix, prefix_len, YANG_NONE), YANG_WHEN);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "yang-version", strlen("yang-version"), prefix, prefix_len, YANG_NONE), YANG_YANG_VERSION);
    assert_int_equal(yin_match_keyword(st->yin_ctx, "yin-element", strlen("yin-element"), prefix, prefix_len, YANG_NONE), YANG_YIN_ELEMENT);

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
                                        <prefix value=\"foo\">ignored</prefix>\
                                        <namespace uri=\"urn:example:foo\" xmlns:myext=\"urn:example:extensions\"/>\
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
test_yin_parse_import(void **state)
{
    struct state *st = *state;
    const char *prefix = NULL, *name = NULL;
    size_t prefix_len = 0, name_len = 0;
    LY_ERR ret = LY_SUCCESS;
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
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    st->lysp_mod->mod->prefix = "b-mod";
    ret = yin_parse_import(st->yin_ctx, &args, &data, st->lysp_mod);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(st->lysp_mod->imports->name, "a");
    assert_string_equal(st->lysp_mod->imports->prefix, "a_mod");
    assert_string_equal(st->lysp_mod->imports->rev, "2015-01-01");
    assert_string_equal(st->lysp_mod->imports->dsc, "import description");
    assert_string_equal(st->lysp_mod->imports->ref, "import reference");
    LY_ARRAY_FREE(args);
    args = NULL;
    st = reset_state(state);

    /* second invalid import */
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    st->lysp_mod->mod->prefix = "a_mod";
    ret = yin_parse_import(st->yin_ctx, &args, &data, st->lysp_mod);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Prefix \"a_mod\" already used as module prefix. Line number 1.");
    LY_ARRAY_FREE(args);
    args = NULL;

    st = reset_state(state);
    /* import with unknown child element */
    data = "<import xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" module=\"a\">\
                <what value=\"a_mod\"/>\
            </import>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    st->lysp_mod->mod->prefix = "invalid_mod";
    ret = yin_parse_import(st->yin_ctx, &args, &data, st->lysp_mod);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Unexpected child element \"what\" of import element. Line number 1.");
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
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_status(st->yin_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_CURR);
    LY_ARRAY_FREE(args);
    args = NULL;

    st = reset_state(state);
    flags = 0;
    data = "<status value=\"deprecated\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_status(st->yin_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_DEPRC);
    LY_ARRAY_FREE(args);
    args = NULL;

    st = reset_state(state);
    flags = 0;
    data = "<status value=\"obsolete\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_status(st->yin_ctx, &args, &data, &flags, &exts);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_STATUS_OBSLT);
    LY_ARRAY_FREE(args);
    args = NULL;

    /* invalid status value */
    st = reset_state(state);
    flags = 0;
    data = "<status value=\"dunno\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_status(st->yin_ctx, &args, &data, &flags, &exts);
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
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_extension(st->yin_ctx, &args, &data, &exts);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
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
    struct lysp_ext_instance *exts;
    struct yin_arg_record *args = NULL;

    /* try all valid values */
    const char *data = "<yin-element value=\"true\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_yin_element_element(st->yin_ctx, args, &data, &flags, &exts);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_TRUE);
    LY_ARRAY_FREE(args);
    args = NULL;

    st = reset_state(state);
    flags = 0;
    data = "<yin-element value=\"false\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_yin_element_element(st->yin_ctx, args, &data, &flags, &exts);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_int_equal(ret, LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_FALSE);
    LY_ARRAY_FREE(args);
    args = NULL;

    /* invalid value */
    st = reset_state(state);
    flags = 0;
    data = "<yin-element value=\"invalid\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"/>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_yin_element_element(st->yin_ctx, args, &data, &flags, &exts);
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
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_element_generic(st->yin_ctx, name, name_len, prefix, prefix_len, &data, &exts.child);
    assert_int_equal(ret, LY_SUCCESS);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_string_equal(exts.child->stmt, "elem");
    assert_string_equal(exts.child->arg, "text_value");
    assert_string_equal(exts.child->child->stmt, "attr");
    assert_string_equal(exts.child->child->arg, "value");
    assert_true(exts.child->child->flags & LYS_YIN_ATTR);
    lysp_ext_instance_free(st->ctx, &exts);
    st = reset_state(state);

    data = "<elem></elem>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    ret = yin_parse_element_generic(st->yin_ctx, name, name_len, prefix, prefix_len, &data, &exts.child);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(exts.child->stmt, "elem");
    assert_null(exts.child->child);
    assert_null(exts.child->arg);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
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
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_extension_instance(st->yin_ctx, &args, &data, name2fullname(name, prefix_len),
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
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    LY_ARRAY_FREE(args);
    lysp_ext_instance_free(st->ctx, exts);
    LY_ARRAY_FREE(exts);
    exts = NULL;
    args = NULL;
    st = reset_state(state);

    data = "<extension-elem />";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    yin_load_attributes(st->yin_ctx, &data, &args);
    ret = yin_parse_extension_instance(st->yin_ctx, &args, &data, name, name_len, LYEXT_SUBSTMT_CONTACT, 0, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(exts->name, "extension-elem");
    assert_null(exts->argument);
    assert_null(exts->child);
    assert_int_equal(exts->insubstmt, LYEXT_SUBSTMT_CONTACT);
    assert_int_equal(exts->insubstmt_index, 0);
    assert_true(exts->yin & LYS_YIN);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    LY_ARRAY_FREE(args);
    lysp_ext_instance_free(st->ctx, exts);
    LY_ARRAY_FREE(exts);
    st->finished_correctly = true;
}

static void
test_yin_parse_content(void **state)
{
    struct state *st = *state;
    LY_ERR ret = LY_SUCCESS;
    struct sized_string name, prefix;
    const char *data = "<prefix value=\"a_mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                            "<custom xmlns=\"my-ext\">"
                                "totally amazing extension"
                            "</custom>"
                            "<extension name=\"ext\">"
                                "<argument name=\"argname\"></argument>"
                                "<description><text>desc</text></description>"
                                "<reference><text>ref</text></reference>"
                                "<status value=\"deprecated\"></status>"
                            "</extension>"
                            "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
                            "<if-feature value=\"foo\"></if-feature>"
                            "<when condition=\"condition...\">"
                                "<reference><text>when_ref</text></reference>"
                                "<description><text>when_desc</text></description>"
                            "</when>"
                            "<config value=\"true\"/>"
                            "<error-message>"
                                "<value>error-msg</value>"
                            "</error-message>"
                            "<error-app-tag value=\"err-app-tag\"/>"
                            "<units name=\"radians\"></units>"
                            "<default value=\"default-value\"/>"
                        "</prefix>";
    struct lysp_ext_instance *exts = NULL;
    const char **if_features = NULL;
    struct yin_arg_record *attrs = NULL;
    const char *value, *err_msg, *app_tag, *units, *def;
    struct lysp_ext *ext_def = NULL;
    struct lysp_when *when_p = NULL;
    uint8_t config = 0;

    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);

    struct yin_subelement subelems[10] = {{YANG_CONFIG, &config, 0},
                                         {YANG_DEFAULT, &def, 0},
                                         {YANG_ERROR_APP_TAG, &app_tag, 0},
                                         {YANG_ERROR_MESSAGE, &err_msg, 0},
                                         {YANG_EXTENSION, &ext_def, 0},
                                         {YANG_IF_FEATURE, &if_features, 0},
                                         {YANG_UNITS, &units, 0},
                                         {YANG_WHEN, &when_p, 0},
                                         {YANG_CUSTOM, NULL, 0},
                                         {YIN_TEXT, &value, 0}};
    ret = yin_parse_content(st->yin_ctx, subelems, 10, &data, YANG_PREFIX, NULL, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    /* check parsed values */
    assert_string_equal(def, "default-value");
    assert_string_equal(exts->name, "custom");
    assert_string_equal(exts->argument, "totally amazing extension");
    assert_string_equal(value, "wsefsdf");
    assert_string_equal(units, "radians");
    assert_string_equal(when_p->cond, "condition...");
    assert_string_equal(when_p->dsc, "when_desc");
    assert_string_equal(when_p->ref, "when_ref");
    assert_int_equal(config, LYS_CONFIG_W);
    assert_string_equal(err_msg, "error-msg");
    assert_string_equal(app_tag, "err-app-tag");
    /* cleanup */
    lysp_ext_instance_free(st->ctx, exts);
    lysp_when_free(st->ctx, when_p);
    lysp_ext_free(st->ctx, ext_def);
    FREE_STRING(st->ctx, *if_features);
    FREE_STRING(st->ctx, err_msg);
    FREE_STRING(st->ctx, app_tag);
    FREE_STRING(st->ctx, units);
    FREE_STRING(st->ctx, def);
    LY_ARRAY_FREE(if_features);
    LY_ARRAY_FREE(exts);
    LY_ARRAY_FREE(ext_def);
    LY_ARRAY_FREE(attrs);
    free(when_p);
    attrs = NULL;
    lydict_remove(st->ctx, value);
    st = reset_state(state);

    /* test unique subelem */
    const char *prefix_value;
    struct yin_subelement subelems2[2] = {{YANG_PREFIX, &prefix_value, 0},
                                         {YIN_TEXT, &value, YIN_SUBELEM_UNIQUE}};
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<prefix value=\"inv_mod\" />"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
           "</module>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_content(st->yin_ctx, subelems2, 2, &data, YANG_MODULE, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Redefinition of text element in module element. Line number 1.");
    lydict_remove(st->ctx, prefix_value);
    lydict_remove(st->ctx, value);
    st = reset_state(state);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;

    /* test first subelem */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<prefix value=\"inv_mod\" />"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
           "</module>";
    struct yin_subelement subelems3[2] = {{YANG_PREFIX, &prefix_value, 0},
                                         {YIN_TEXT, &value, YIN_SUBELEM_FIRST}};
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_content(st->yin_ctx, subelems3, 2, &data, YANG_MODULE, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Subelement text of module element must be defined as first subelement. Line number 1.");
    lydict_remove(st->ctx, prefix_value);
    st = reset_state(state);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;

    /* test mandatory subelem */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
           "</module>";
    struct yin_subelement subelems4[1] = {{YANG_PREFIX, &prefix_value, YIN_SUBELEM_MANDATORY}};
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_content(st->yin_ctx, subelems4, 1, &data, YANG_MODULE, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Missing mandatory subelement prefix of module element. Line number 1.");
    LY_ARRAY_FREE(attrs);

    st->finished_correctly = true;
}

static void
test_yin_parse_yangversion(void **state)
{
    struct state *st = *state;
    LY_ERR ret = LY_SUCCESS;
    struct sized_string name, prefix;
    struct yin_arg_record *attrs = NULL;
    uint8_t version;

    const char *data = "<yang-version xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" value=\"1.0\">\n"
                       "</yang-version>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_yangversion(st->yin_ctx, attrs, &data, &version, NULL);
    assert_int_equal(LY_SUCCESS, ret);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_true(version == LYS_VERSION_1_0);
    assert_true(st->yin_ctx->mod_version == LYS_VERSION_1_0);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;
    st = reset_state(state);

    data = "<yang-version xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" value=\"1.1\">\n"
           "</yang-version>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_yangversion(st->yin_ctx, attrs, &data, &version, NULL);
    assert_int_equal(LY_SUCCESS, ret);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_true(version == LYS_VERSION_1_1);
    assert_true(st->yin_ctx->mod_version == LYS_VERSION_1_1);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;
    st = reset_state(state);

    data = "<yang-version xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" value=\"randomvalue\">\n"
           "</yang-version>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_yangversion(st->yin_ctx, attrs, &data, &version, NULL);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Invalid value \"randomvalue\" of \"yang-version\". Line number 1.");
    LY_ARRAY_FREE(attrs);
    attrs = NULL;
    st = reset_state(state);

    data = "<yang-version xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
           "</yang-version>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_yangversion(st->yin_ctx, attrs, &data, &version, NULL);
    assert_int_equal(ret, LY_EVALID);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;
    logbuf_assert("Missing mandatory attribute value of yang-version element. Line number 1.");
    st->finished_correctly = true;
}

static void
test_yin_parse_mandatory(void **state)
{
    struct state *st = *state;
    LY_ERR ret = LY_SUCCESS;
    struct sized_string name, prefix;
    struct yin_arg_record *attrs = NULL;
    uint16_t man = 0;

    const char *data = "<mandatory xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" value=\"true\">\n"
                       "</mandatory>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_mandatory(st->yin_ctx, attrs, &data, &man, NULL);
    assert_int_equal(LY_SUCCESS, ret);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_true(man == LYS_MAND_TRUE);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;
    man = 0;
    st = reset_state(state);

    data = "<mandatory xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" value=\"false\" />";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_mandatory(st->yin_ctx, attrs, &data, &man, NULL);
    assert_int_equal(LY_SUCCESS, ret);
    assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    assert_true(man == LYS_MAND_FALSE);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;
    man = 0;
    st = reset_state(state);

    data = "<mandatory xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" value=\"randomvalue\">\n"
           "</mandatory>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_mandatory(st->yin_ctx, attrs, &data, &man, NULL);
    assert_int_equal(ret, LY_EVALID);
    LY_ARRAY_FREE(attrs);
    logbuf_assert("Invalid value \"randomvalue\" of \"mandatory\". Line number 1.");
    attrs = NULL;
    man = 0;
    st = reset_state(state);

    data = "<mandatory xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
           "</mandatory>";
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_mandatory(st->yin_ctx, attrs, &data, &man, NULL);
    assert_int_equal(ret, LY_EVALID);
    LY_ARRAY_FREE(attrs);
    logbuf_assert("Missing mandatory attribute value of mandatory element. Line number 1.");
    st->finished_correctly = true;
}

static void
test_validate_value(void **state)
{
    struct state *st = *state;
    assert_int_equal(yin_validate_value(st->yin_ctx, Y_IDENTIF_ARG, "#invalid", 8), LY_EVALID);
    logbuf_assert("Invalid identifier character '#'. Line number 1.");
    assert_int_equal(yin_validate_value(st->yin_ctx, Y_STR_ARG, "", 0), LY_SUCCESS);
    assert_int_equal(yin_validate_value(st->yin_ctx, Y_IDENTIF_ARG, "pre:b", 5), LY_EVALID);
    assert_int_equal(yin_validate_value(st->yin_ctx, Y_PREF_IDENTIF_ARG, "pre:b", 5), LY_SUCCESS);
    assert_int_equal(yin_validate_value(st->yin_ctx, Y_PREF_IDENTIF_ARG, "pre:pre:b", 9), LY_EVALID);

    st->finished_correctly = true;
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_yin_parse_module, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_meta, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_import, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_status, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_match_keyword, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_extension, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_yin_element_element, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_element_generic, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_extension_instance, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_content, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_yangversion, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_mandatory, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_validate_value, setup_f, teardown_f),
        cmocka_unit_test(test_yin_match_argument_name),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
