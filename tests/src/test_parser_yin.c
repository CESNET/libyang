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
void lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type);

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

int
setup_ly_ctx(void **state)
{
    struct state *st = NULL;

    /* allocate state variable */
    (*state) = st = calloc(1, sizeof(*st));
    if (!st) {
        fprintf(stderr, "Memmory allocation failed");
        return EXIT_FAILURE;
    }

    /* create new libyang context */
    ly_ctx_new(NULL, 0, &st->ctx);

    return EXIT_SUCCESS;
}

int
destroy_ly_ctx(void **state)
{
    struct state *st = *state;
    ly_ctx_destroy(st->ctx, NULL);
    free(st);

    return EXIT_SUCCESS;
}

static int
setup_f(void **state)
{
    struct state *st = *state;

#if ENABLE_LOGGER_CHECKING
    /* setup logger */
    ly_set_log_clb(logger, 1);
#endif

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
    free(st->yin_ctx);

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
    ret = yin_parse_extension_instance(st->yin_ctx, args, &data, name2fullname(name, prefix_len),
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
    ret = yin_parse_extension_instance(st->yin_ctx, args, &data, name, name_len, LYEXT_SUBSTMT_CONTACT, 0, &exts);
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
                            "<if-feature name=\"foo\"></if-feature>"
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
                            "<position value=\"25\"></position>"
                            "<value value=\"-5\"/>"
                            "<require-instance value=\"true\"></require-instance>"
                            "<range value=\"5..10\" />"
                            "<length value=\"baf\"/>"
                            "<pattern value='pattern'>"
                                "<modifier value='invert-match'/>"
                            "</pattern>"
                            "<enum name=\"yay\">"
                            "</enum>"
                        "</prefix>";
    struct lysp_ext_instance *exts = NULL;
    const char **if_features = NULL;
    struct yin_arg_record *attrs = NULL;
    const char *value, *err_msg, *app_tag, *units, *def;
    struct lysp_ext *ext_def = NULL;
    struct lysp_when *when_p = NULL;
    struct lysp_type_enum pos_enum = {}, val_enum = {};
    struct lysp_type req_type = {}, range_type = {}, len_type = {}, patter_type = {}, enum_type = {};
    uint8_t config = 0;

    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);

    struct yin_subelement subelems[17] = {
                                            {YANG_CONFIG, &config, 0},
                                            {YANG_DEFAULT, &def, 0},
                                            {YANG_ENUM, &enum_type, 0},
                                            {YANG_ERROR_APP_TAG, &app_tag, 0},
                                            {YANG_ERROR_MESSAGE, &err_msg, 0},
                                            {YANG_EXTENSION, &ext_def, 0},
                                            {YANG_IF_FEATURE, &if_features, 0},
                                            {YANG_LENGTH, &len_type, 0},
                                            {YANG_PATTERN, &patter_type, 0},
                                            {YANG_POSITION, &pos_enum, 0},
                                            {YANG_RANGE, &range_type, 0},
                                            {YANG_REQUIRE_INSTANCE, &req_type, 0},
                                            {YANG_UNITS, &units, 0},
                                            {YANG_VALUE, &val_enum, 0},
                                            {YANG_WHEN, &when_p, 0},
                                            {YANG_CUSTOM, NULL, 0},
                                            {YIN_TEXT, &value, 0}
                                         };
    ret = yin_parse_content(st->yin_ctx, subelems, 17, &data, YANG_PREFIX, NULL, &exts);
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
    assert_int_equal(pos_enum.value, 25);
    assert_true(pos_enum.flags | LYS_SET_VALUE);
    assert_int_equal(val_enum.value, -5);
    assert_true(val_enum.flags | LYS_SET_VALUE);
    assert_int_equal(req_type.require_instance, 1);
    assert_true(req_type.flags |= LYS_SET_REQINST);
    assert_string_equal(range_type.range->arg, "5..10");
    assert_true(range_type.flags | LYS_SET_RANGE);
    assert_string_equal(err_msg, "error-msg");
    assert_string_equal(app_tag, "err-app-tag");
    assert_string_equal(enum_type.enums->name, "yay");
    assert_string_equal(len_type.length->arg, "baf");
    assert_true(len_type.flags | LYS_SET_LENGTH);
    assert_string_equal(patter_type.patterns->arg, "\x015pattern");
    assert_true(patter_type.flags | LYS_SET_PATTERN);
    /* cleanup */
    lysp_ext_instance_free(st->ctx, exts);
    lysp_when_free(st->ctx, when_p);
    lysp_ext_free(st->ctx, ext_def);
    FREE_STRING(st->ctx, *if_features);
    FREE_STRING(st->ctx, err_msg);
    FREE_STRING(st->ctx, app_tag);
    FREE_STRING(st->ctx, units);
    FREE_STRING(st->ctx, patter_type.patterns->arg);
    FREE_STRING(st->ctx, def);
    FREE_STRING(st->ctx, range_type.range->arg);
    FREE_STRING(st->ctx, len_type.length->arg);
    FREE_STRING(st->ctx, enum_type.enums->name);
    FREE_STRING(st->ctx, value);
    LY_ARRAY_FREE(if_features);
    LY_ARRAY_FREE(exts);
    LY_ARRAY_FREE(ext_def);
    LY_ARRAY_FREE(attrs);
    LY_ARRAY_FREE(patter_type.patterns);
    LY_ARRAY_FREE(enum_type.enums);
    free(when_p);
    free(range_type.range);
    free(len_type.length);
    attrs = NULL;
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

static int
setup_element_test(void **state)
{
    struct state *st = *state;

#if ENABLE_LOGGER_CHECKING
    /* setup logger */
    ly_set_log_clb(logger, 1);
#endif

    /* reset logbuf */
    logbuf[0] = '\0';

    /* allocate parser context */
    st->yin_ctx = calloc(1, sizeof(*st->yin_ctx));
    st->yin_ctx->xml_ctx.ctx = st->ctx;
    st->yin_ctx->xml_ctx.line = 1;

    return EXIT_SUCCESS;
}

static int
teardown_element_test(void **state)
{
    struct state *st = *(struct state **)state;

#if ENABLE_LOGGER_CHECKING
    /* teardown logger */
    if (!st->finished_correctly && logbuf[0] != '\0') {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif

    lyxml_context_clear(&st->yin_ctx->xml_ctx);
    free(st->yin_ctx);

    return EXIT_SUCCESS;
}

#define ELEMENT_WRAPPER_START "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
#define ELEMENT_WRAPPER_END "</module>"

/* helper function to simplify unit test of each element using parse_content function */
LY_ERR
test_element_helper(struct state *st, const char **data, void *dest, const char **text,
                    struct lysp_ext_instance **exts, bool valid)
{
    struct yin_arg_record *attrs = NULL;
    struct sized_string name, prefix;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement subelems[71] = {
                                            {YANG_ACTION, dest, 0},
                                            {YANG_ANYDATA, dest, 0},
                                            {YANG_ANYXML, dest, 0},
                                            {YANG_ARGUMENT,dest, 0},
                                            {YANG_AUGMENT, dest, 0},
                                            {YANG_BASE, dest, 0},
                                            {YANG_BELONGS_TO, dest, 0},
                                            {YANG_BIT, dest, 0},
                                            {YANG_CASE, dest, 0},
                                            {YANG_CHOICE, dest, 0},
                                            {YANG_CONFIG, dest, 0},
                                            {YANG_CONTACT, dest, 0},
                                            {YANG_CONTAINER, dest, 0},
                                            {YANG_DEFAULT, dest, 0},
                                            {YANG_DESCRIPTION, dest, 0},
                                            {YANG_DEVIATE, dest, 0},
                                            {YANG_DEVIATION, dest, 0},
                                            {YANG_ENUM, dest, 0},
                                            {YANG_ERROR_APP_TAG, dest, 0},
                                            {YANG_ERROR_MESSAGE, dest, 0},
                                            {YANG_EXTENSION, dest, 0},
                                            {YANG_FEATURE, dest, 0},
                                            {YANG_FRACTION_DIGITS, dest, 0},
                                            {YANG_GROUPING, dest, 0},
                                            {YANG_IDENTITY, dest, 0},
                                            {YANG_IF_FEATURE, dest, 0},
                                            {YANG_IMPORT, dest, 0},
                                            {YANG_INCLUDE, dest, 0},
                                            {YANG_INPUT, dest, 0},
                                            {YANG_KEY, dest, 0},
                                            {YANG_LEAF, dest, 0},
                                            {YANG_LEAF_LIST, dest, 0},
                                            {YANG_LENGTH, dest, 0},
                                            {YANG_LIST, dest, 0},
                                            {YANG_MANDATORY, dest, 0},
                                            {YANG_MAX_ELEMENTS, dest, 0},
                                            {YANG_MIN_ELEMENTS, dest, 0},
                                            {YANG_MODIFIER, dest, 0},
                                            {YANG_MODULE, dest, 0},
                                            {YANG_MUST, dest, 0},
                                            {YANG_NAMESPACE, dest, 0},
                                            {YANG_NOTIFICATION, dest, 0},
                                            {YANG_ORDERED_BY, dest, 0},
                                            {YANG_ORGANIZATION, dest, 0},
                                            {YANG_OUTPUT, dest, 0},
                                            {YANG_PATH, dest, 0},
                                            {YANG_PATTERN, dest, 0},
                                            {YANG_POSITION, dest, 0},
                                            {YANG_PREFIX, dest, 0},
                                            {YANG_PRESENCE, dest, 0},
                                            {YANG_RANGE, dest, 0},
                                            {YANG_REFERENCE, dest, 0},
                                            {YANG_REFINE, dest, 0},
                                            {YANG_REQUIRE_INSTANCE, dest, 0},
                                            {YANG_REVISION, dest, 0},
                                            {YANG_REVISION_DATE, dest, 0},
                                            {YANG_RPC, dest, 0},
                                            {YANG_STATUS, dest, 0},
                                            {YANG_SUBMODULE, dest, 0},
                                            {YANG_TYPE, dest, 0},
                                            {YANG_TYPEDEF, dest, 0},
                                            {YANG_UNIQUE, dest, 0},
                                            {YANG_UNITS, dest, 0},
                                            {YANG_USES, dest, 0},
                                            {YANG_VALUE, dest, 0},
                                            {YANG_WHEN, dest, 0},
                                            {YANG_YANG_VERSION, dest, 0},
                                            {YANG_YIN_ELEMENT, dest, 0},
                                            {YANG_CUSTOM, dest, 0},
                                            {YIN_TEXT, dest, 0},
                                            {YIN_VALUE, dest, 0}
                                        };
    LY_CHECK_RET(lyxml_get_element(&st->yin_ctx->xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len));\
    LY_CHECK_RET(yin_load_attributes(st->yin_ctx, data, &attrs));\
    ret = yin_parse_content(st->yin_ctx, subelems, 71, data, yin_match_keyword(st->yin_ctx, name.value, name.len, prefix.value, prefix.len, YANG_NONE), text, exts);
    LY_ARRAY_FREE(attrs);
    if (valid) {
        assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    }
    /* reset status */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    return ret;
}

static void
test_enum_bit_elem(void **state)
{
    /* yin_parse_enum_bit is function that is being mainly tested by this test */
    struct state *st = *state;
    struct lysp_type type = {};
    const char *data;
    data = ELEMENT_WRAPPER_START
           "<enum name=\"enum-name\">"
                "<if-feature name=\"feature\" />"
                "<value value=\"55\" />"
                "<status value=\"deprecated\" />"
                "<description><text>desc...</text></description>"
                "<reference><text>ref...</text></reference>"
           "</enum>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(*type.enums->iffeatures, "feature");
    assert_int_equal(type.enums->value, 55);
    assert_true(type.enums->flags | LYS_STATUS_DEPRC && type.enums->flags | LYS_SET_VALUE);
    assert_string_equal(type.enums->dsc, "desc...");
    assert_string_equal(type.enums->ref, "ref...");
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof type);

    /* todo bit element test */
    st->finished_correctly = true;
}

static void
test_meta_elem(void **state)
{
    struct state *st = *state;
    char *value = NULL;
    const char *data;

    /* organization element */
    data = ELEMENT_WRAPPER_START
                "<organization><text>organization...</text></organization>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(value, "organization...");
    FREE_STRING(st->ctx, value);
    value = NULL;
    /* contact element */
    data = ELEMENT_WRAPPER_START
                "<contact><text>contact...</text></contact>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(value, "contact...");
    FREE_STRING(st->ctx, value);
    value = NULL;
    /* description element */
    data = ELEMENT_WRAPPER_START
                "<description><text>description...</text></description>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(value, "description...");
    FREE_STRING(st->ctx, value);
    value = NULL;
    /* reference element */
    data = ELEMENT_WRAPPER_START
                "<reference><text>reference...</text></reference>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(value, "reference...");
    FREE_STRING(st->ctx, value);
    value = NULL;

    /* missing text subelement */
    data = ELEMENT_WRAPPER_START
                "<reference>reference...</reference>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement text of reference element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_import_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lys_module *lys_mod = calloc(1, sizeof *lys_mod);
    struct lysp_module *lysp_mod = calloc(1, sizeof *lysp_mod);
    lys_mod->ctx = st->ctx;
    lysp_mod->mod = lys_mod;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<import module=\"a\">"
                    "<prefix value=\"a_mod\"/>"
                    "<revision-date date=\"2015-01-01\"></revision-date>"
                    "<description><text>import description</text></description>"
                    "<reference><text>import reference</text></reference>"
                "</import>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, lysp_mod, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(lysp_mod->imports->name, "a");
    assert_string_equal(lysp_mod->imports->prefix, "a_mod");
    assert_string_equal(lysp_mod->imports->rev, "2015-01-01");
    assert_string_equal(lysp_mod->imports->dsc, "import description");
    assert_string_equal(lysp_mod->imports->ref, "import reference");
    lysp_module_free(lysp_mod);
    lys_module_free(lys_mod, NULL);

    /* min subelems */
    lys_mod = calloc(1, sizeof *lys_mod);
    lysp_mod = calloc(1, sizeof *lysp_mod);
    lys_mod->ctx = st->ctx;
    lysp_mod->mod = lys_mod;
    data = ELEMENT_WRAPPER_START
                "<import module=\"a\">"
                    "<prefix value=\"a_mod\"/>"
                "</import>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, lysp_mod, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(lysp_mod->imports->prefix, "a_mod");
    lysp_module_free(lysp_mod);
    lys_module_free(lys_mod, NULL);

    /* invalid (missing prefix) */
    lys_mod = calloc(1, sizeof *lys_mod);
    lysp_mod = calloc(1, sizeof *lysp_mod);
    lys_mod->ctx = st->ctx;
    lysp_mod->mod = lys_mod;
    data = ELEMENT_WRAPPER_START "<import module=\"a\">""</import>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, lysp_mod, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement prefix of import element. Line number 1.");
    lysp_module_free(lysp_mod);
    lys_module_free(lys_mod, NULL);

    /* invalid reused prefix */
    lys_mod = calloc(1, sizeof *lys_mod);
    lysp_mod = calloc(1, sizeof *lysp_mod);
    lys_mod->ctx = st->ctx;
    lysp_mod->mod = lys_mod;
    data = ELEMENT_WRAPPER_START
                "<import module=\"a\">"
                    "<prefix value=\"a_mod\"/>"
                "</import>"
                "<import module=\"a\">"
                    "<prefix value=\"a_mod\"/>"
                "</import>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, lysp_mod, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Prefix \"a_mod\" already used to import \"a\" module. Line number 1.");
    lysp_module_free(lysp_mod);
    lys_module_free(lys_mod, NULL);

    st->finished_correctly = true;
}

static void
test_status_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint16_t flags = 0;

    /* test valid values */
    data = ELEMENT_WRAPPER_START "<status value=\"current\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags | LYS_STATUS_CURR);

    data = ELEMENT_WRAPPER_START "<status value=\"deprecated\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags | LYS_STATUS_DEPRC);

    data = ELEMENT_WRAPPER_START "<status value=\"obsolete\"></status>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags | LYS_STATUS_OBSLT);

    /* test invalid value */
    data = ELEMENT_WRAPPER_START "<status value=\"invalid\"></status>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"invalid\" of \"status\". Line number 1.");
    st->finished_correctly = true;
}

static void
test_ext_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_ext *ext = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
           "<extension name=\"ext_name\">"
                "<argument name=\"arg\"></argument>"
                "<status value=\"current\"/>"
                "<description><text>ext_desc</text></description>"
                "<reference><text>ext_ref</text></reference>"
           "</extension>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &ext, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(ext->name, "ext_name");
    assert_string_equal(ext->argument, "arg");
    assert_true(ext->flags | LYS_STATUS_CURR);
    assert_string_equal(ext->dsc, "ext_desc");
    assert_string_equal(ext->ref, "ext_ref");
    lysp_ext_free(st->ctx, ext);
    LY_ARRAY_FREE(ext);
    ext = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<extension name=\"ext_name\"></extension>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &ext, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(ext->name, "ext_name");
    lysp_ext_free(st->ctx, ext);
    LY_ARRAY_FREE(ext);
    ext = NULL;

    st->finished_correctly = true;
}

static void
test_yin_element_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint16_t flags = 0;

    data = ELEMENT_WRAPPER_START "<yin-element value=\"true\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags | LYS_YINELEM_TRUE);

    data = ELEMENT_WRAPPER_START "<yin-element value=\"false\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags | LYS_YINELEM_TRUE);

    data = ELEMENT_WRAPPER_START "<yin-element value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, false), LY_EVALID);
    assert_true(flags | LYS_YINELEM_TRUE);
    logbuf_assert("Invalid value \"invalid\" of \"yin-element\". Line number 1.");
    st->finished_correctly = true;
}

static void
test_yangversion_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint8_t version = 0;

    /* valid values */
    data = ELEMENT_WRAPPER_START "<yang-version value=\"1.0\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &version, NULL, NULL, true), LY_SUCCESS);
    assert_true(version | LYS_VERSION_1_0);
    assert_int_equal(st->yin_ctx->mod_version, LYS_VERSION_1_0);

    data = ELEMENT_WRAPPER_START "<yang-version value=\"1.1\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &version, NULL, NULL, true), LY_SUCCESS);
    assert_true(version | LYS_VERSION_1_1);
    assert_int_equal(st->yin_ctx->mod_version, LYS_VERSION_1_1);

    /* invalid value */
    data = ELEMENT_WRAPPER_START "<yang-version value=\"version\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &version, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"version\" of \"yang-version\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_mandatory_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint16_t man = 0;

    /* valid values */
    data = ELEMENT_WRAPPER_START "<mandatory value=\"true\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &man, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(man, LYS_MAND_TRUE);
    man = 0;

    data = ELEMENT_WRAPPER_START "<mandatory value=\"false\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &man, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(man, LYS_MAND_FALSE);

    data = ELEMENT_WRAPPER_START "<mandatory value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &man, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"invalid\" of \"mandatory\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_argument_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint16_t flags = 0;
    const char *arg;
    struct yin_argument_meta arg_meta = {&flags, &arg};
    /* max subelems */
    data = ELEMENT_WRAPPER_START
           "<argument name=\"arg-name\">"
                "<yin-element value=\"true\" />"
           "</argument>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &arg_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(arg, "arg-name");
    assert_true(flags | LYS_YINELEM_TRUE);
    flags = 0;
    FREE_STRING(st->ctx, arg);
    arg = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START
           "<argument name=\"arg\">"
           "</argument>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &arg_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(arg, "arg");
    assert_true(flags == 0);
    FREE_STRING(st->ctx, arg);

    st->finished_correctly = true;
}

static void
test_base_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char **bases = NULL;
    struct lysp_type type = {};

    /* as identity subelement */
    data = "<identity xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<base name=\"base-name\"/>"
           "</identity>";
    assert_int_equal(test_element_helper(st, &data, &bases, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(*bases, "base-name");
    FREE_STRING(st->ctx, *bases);
    LY_ARRAY_FREE(bases);

    /* as type subelement */
    data = "<type xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<base name=\"base-name\"/>"
           "</type>";
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(*type.bases, "base-name");
    assert_true(type.flags | LYS_SET_BASE);
    FREE_STRING(st->ctx, *type.bases);
    LY_ARRAY_FREE(type.bases);

    st->finished_correctly = true;
}

static void
test_belongsto_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_submodule submod;

    data = ELEMENT_WRAPPER_START
                "<belongs-to module=\"module-name\"><prefix value=\"pref\"/></belongs-to>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &submod, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(submod.belongsto, "module-name");
    assert_string_equal(submod.prefix, "pref");
    FREE_STRING(st->ctx, submod.belongsto);
    FREE_STRING(st->ctx, submod.prefix);

    data = ELEMENT_WRAPPER_START "<belongs-to module=\"module-name\"></belongs-to>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &submod, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement prefix of belongs-to element. Line number 1.");
    FREE_STRING(st->ctx, submod.belongsto);

    st->finished_correctly = true;
}

static void
test_config_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint16_t flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"true\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags | LYS_CONFIG_W);
    flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"false\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags | LYS_CONFIG_R);
    flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"invalid\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"invalid\" of \"config\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_default_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *val = NULL;

    data = ELEMENT_WRAPPER_START "<default value=\"defaul-value\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(val, "defaul-value");
    FREE_STRING(st->ctx, val);
    val = NULL;

    data = ELEMENT_WRAPPER_START "<default/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory attribute value of default element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_err_app_tag_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *val = NULL;

    data = ELEMENT_WRAPPER_START "<error-app-tag value=\"val\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(val, "val");
    FREE_STRING(st->ctx, val);
    val = NULL;

    data = ELEMENT_WRAPPER_START "<error-app-tag/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory attribute value of error-app-tag element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_err_msg_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *val = NULL;

    data = ELEMENT_WRAPPER_START "<error-message><value>val</value></error-message>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(val, "val");
    FREE_STRING(st->ctx, val);

    data = ELEMENT_WRAPPER_START "<error-message></error-message>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement value of error-message element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_fracdigits_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    /* valid value */
    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"10\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(type.fraction_digits, 10);
    assert_true(type.flags | LYS_SET_FRDIGITS);

    /* invalid values */
    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"-1\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"-1\" of \"fraction-digits\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"02\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"02\" of \"fraction-digits\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"1p\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"1p\" of \"fraction-digits\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"19\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"19\" of \"fraction-digits\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"999999999999999999\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"999999999999999999\" of \"fraction-digits\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_iffeature_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char **iffeatures = NULL;

    data = ELEMENT_WRAPPER_START "<if-feature name=\"local-storage\"></if-feature>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &iffeatures, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(*iffeatures, "local-storage");
    FREE_STRING(st->ctx, *iffeatures);
    LY_ARRAY_FREE(iffeatures);
    iffeatures = NULL;

    data = ELEMENT_WRAPPER_START "<if-feature/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &iffeatures, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory attribute name of if-feature element. Line number 1.");
    LY_ARRAY_FREE(iffeatures);
    iffeatures = NULL;

    st->finished_correctly = true;
}

static void
test_length_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<length value=\"length-str\">"
                    "<error-message><value>err-msg</value></error-message>"
                    "<error-app-tag value=\"err-app-tag\"/>"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                "</length>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.length->arg, "length-str");
    assert_string_equal(type.length->emsg, "err-msg");
    assert_string_equal(type.length->eapptag, "err-app-tag");
    assert_string_equal(type.length->dsc, "desc");
    assert_string_equal(type.length->ref, "ref");
    assert_true(type.flags | LYS_SET_LENGTH);
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START
                "<length value=\"length-str\">"
                "</length>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.length->arg, "length-str");
    lysp_type_free(st->ctx, &type);
    assert_true(type.flags | LYS_SET_LENGTH);
    memset(&type, 0, sizeof(type));

    data = ELEMENT_WRAPPER_START "<length></length>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory attribute value of length element. Line number 1.");
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    st->finished_correctly = true;
}

static void
test_modifier_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *pat = lydict_insert(st->ctx, "\006pattern", 8);

    data = ELEMENT_WRAPPER_START "<modifier value=\"invert-match\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &pat, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(pat, "\x015pattern");
    FREE_STRING(st->ctx, pat);

    pat = lydict_insert(st->ctx, "\006pattern", 8);
    data = ELEMENT_WRAPPER_START "<modifier value=\"invert\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &pat, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"invert\" of \"modifier\". Line number 1.");
    FREE_STRING(st->ctx, pat);

    st->finished_correctly = true;
}

static void
test_namespace_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *ns;

    data = ELEMENT_WRAPPER_START "<namespace uri=\"ns\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &ns, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(ns, "ns");
    FREE_STRING(st->ctx, ns);

    data = ELEMENT_WRAPPER_START "<namespace/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &ns, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory attribute uri of namespace element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_path_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    data = ELEMENT_WRAPPER_START "<path value=\"path-val\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal("path-val", type.path);
    assert_true(type.flags | LYS_SET_PATH);
    lysp_type_free(st->ctx, &type);

    st->finished_correctly = true;
}

static void
test_pattern_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<pattern value=\"super_pattern\">"
                    "<modifier value=\"invert-match\"/>"
                    "<error-message><value>err-msg-value</value></error-message>"
                    "<error-app-tag value=\"err-app-tag-value\"/>"
                    "<description><text>pattern-desc</text></description>"
                    "<reference><text>pattern-ref</text></reference>"
                "</pattern>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_true(type.flags | LYS_SET_PATTERN);
    assert_string_equal(type.patterns->arg, "\x015super_pattern");
    assert_string_equal(type.patterns->dsc, "pattern-desc");
    assert_string_equal(type.patterns->eapptag, "err-app-tag-value");
    assert_string_equal(type.patterns->emsg, "err-msg-value");
    assert_string_equal(type.patterns->dsc, "pattern-desc");
    assert_string_equal(type.patterns->ref, "pattern-ref");
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<pattern value=\"pattern\"> </pattern>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.patterns->arg, "\x006pattern");
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    st->finished_correctly = true;
}

static void
test_value_position_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type_enum en = {};

    /* valid values */
    data = ELEMENT_WRAPPER_START "<value value=\"55\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 55);
    assert_true(en.flags | LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"-55\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, -55);
    assert_true(en.flags | LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 0);
    assert_true(en.flags | LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"-0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 0);
    assert_true(en.flags | LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    /* valid positions */
    data = ELEMENT_WRAPPER_START "<position value=\"55\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 55);
    assert_true(en.flags | LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<position value=\"0\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 0);
    assert_true(en.flags | LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    /* invalid values */
    data = ELEMENT_WRAPPER_START "<value value=\"99999999999999999999999\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"99999999999999999999999\" of \"value\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<value value=\"1k\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"1k\" of \"value\". Line number 1.");

    /*invalid positions */
    data = ELEMENT_WRAPPER_START "<position value=\"-5\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"-5\" of \"position\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<position value=\"-0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"-0\" of \"position\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<position value=\"99999999999999999999\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"99999999999999999999\" of \"position\". Line number 1.");

    /* TODO empty position and value */
    st->finished_correctly = true;
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_yin_parse_module, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_match_keyword, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_element_generic, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_extension_instance, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_content, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_validate_value, setup_f, teardown_f),

        cmocka_unit_test(test_yin_match_argument_name),
        cmocka_unit_test_setup_teardown(test_enum_bit_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_meta_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_import_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_status_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_ext_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_yin_element_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_yangversion_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_mandatory_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_argument_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_base_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_belongsto_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_config_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_default_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_err_app_tag_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_err_msg_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_fracdigits_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_iffeature_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_length_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_modifier_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_namespace_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_path_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_pattern_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_value_position_elem, setup_element_test, teardown_element_test),

    };

    return cmocka_run_group_tests(tests, setup_ly_ctx, destroy_ly_ctx);
}
