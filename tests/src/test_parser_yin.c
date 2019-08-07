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
void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);
void lysp_tpdf_free(struct ly_ctx *ctx, struct lysp_tpdf *tpdf);
void lysp_refine_free(struct ly_ctx *ctx, struct lysp_refine *ref);
void lysp_revision_free(struct ly_ctx *ctx, struct lysp_revision *rev);
void lysp_include_free(struct ly_ctx *ctx, struct lysp_include *include);
void lysp_feature_free(struct ly_ctx *ctx, struct lysp_feature *feat);
void lysp_ident_free(struct ly_ctx *ctx, struct lysp_ident *ident);
void lysp_notif_free(struct ly_ctx *ctx, struct lysp_notif *notif);
void lysp_grp_free(struct ly_ctx *ctx, struct lysp_grp *grp);
void lysp_action_inout_free(struct ly_ctx *ctx, struct lysp_action_inout *inout);
void lysp_action_free(struct ly_ctx *ctx, struct lysp_action *action);
void lysp_augment_free(struct ly_ctx *ctx, struct lysp_augment *augment);
void lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d);
void lysp_deviation_free(struct ly_ctx *ctx, struct lysp_deviation *dev);
void lysp_submodule_free(struct ly_ctx *ctx, struct lysp_submodule *submod);
void lysp_import_free(struct ly_ctx *ctx, struct lysp_import *import);

/* wrapping element used for mocking has nothing to do with real module structure */
#define ELEMENT_WRAPPER_START "<status xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
#define ELEMENT_WRAPPER_END "</status>"

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

static int
setup_logger(void **state)
{
    (void)state; /* unused */
#if ENABLE_LOGGER_CHECKING
    /* setup logger */
    ly_set_log_clb(logger, 1);
#endif

    logbuf[0] = '\0';

    return EXIT_SUCCESS;
}

static int
teardown_logger(void **state)
{
    struct state *st = *state;

#if ENABLE_LOGGER_CHECKING
    /* teardown logger */
    if (!st->finished_correctly && logbuf[0] != '\0') {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif

    return EXIT_SUCCESS;
}

static int
setup_element_test(void **state)
{
    setup_logger(state);
    struct state *st = *state;

    st->yin_ctx = calloc(1, sizeof(*st->yin_ctx));

    /* allocate parser context */
    st->yin_ctx->xml_ctx.ctx = st->ctx;
    st->yin_ctx->xml_ctx.line = 1;

    return EXIT_SUCCESS;
}

static int
teardown_element_test(void **state)
{
    struct state *st = *(struct state **)state;

    lyxml_context_clear(&st->yin_ctx->xml_ctx);
    free(st->yin_ctx);

    teardown_logger(state);

    return EXIT_SUCCESS;
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
    ret = yin_parse_element_generic(st->yin_ctx, name, name_len, &data, &exts.child);
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
    ret = yin_parse_element_generic(st->yin_ctx, name, name_len, &data, &exts.child);
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
                                            {YANG_DEFAULT, &def, YIN_SUBELEM_UNIQUE},
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
    assert_true(pos_enum.flags & LYS_SET_VALUE);
    assert_int_equal(val_enum.value, -5);
    assert_true(val_enum.flags & LYS_SET_VALUE);
    assert_int_equal(req_type.require_instance, 1);
    assert_true(req_type.flags &= LYS_SET_REQINST);
    assert_string_equal(range_type.range->arg, "5..10");
    assert_true(range_type.flags & LYS_SET_RANGE);
    assert_string_equal(err_msg, "error-msg");
    assert_string_equal(app_tag, "err-app-tag");
    assert_string_equal(enum_type.enums->name, "yay");
    assert_string_equal(len_type.length->arg, "baf");
    assert_true(len_type.flags & LYS_SET_LENGTH);
    assert_string_equal(patter_type.patterns->arg, "\x015pattern");
    assert_true(patter_type.flags & LYS_SET_PATTERN);
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
    data = ELEMENT_WRAPPER_START
                "<prefix value=\"inv_mod\" />"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
           ELEMENT_WRAPPER_END;
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_content(st->yin_ctx, subelems2, 2, &data, YANG_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Redefinition of text element in status element. Line number 1.");
    lydict_remove(st->ctx, prefix_value);
    lydict_remove(st->ctx, value);
    st = reset_state(state);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;

    /* test first subelem */
    data = ELEMENT_WRAPPER_START
                "<prefix value=\"inv_mod\" />"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
                "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
           ELEMENT_WRAPPER_END;
    struct yin_subelement subelems3[2] = {{YANG_PREFIX, &prefix_value, 0},
                                         {YIN_TEXT, &value, YIN_SUBELEM_FIRST}};
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_content(st->yin_ctx, subelems3, 2, &data, YANG_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Subelement text of status element must be defined as first subelement. Line number 1.");
    lydict_remove(st->ctx, prefix_value);
    st = reset_state(state);
    LY_ARRAY_FREE(attrs);
    attrs = NULL;

    /* test mandatory subelem */
    data = ELEMENT_WRAPPER_START ELEMENT_WRAPPER_END;
    struct yin_subelement subelems4[1] = {{YANG_PREFIX, &prefix_value, YIN_SUBELEM_MANDATORY}};
    lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len);
    yin_load_attributes(st->yin_ctx, &data, &attrs);
    ret = yin_parse_content(st->yin_ctx, subelems4, 1, &data, YANG_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    logbuf_assert("Missing mandatory subelement prefix of status element. Line number 1.");
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
                                            {YANG_DEFAULT, dest, YIN_SUBELEM_UNIQUE},
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
    LY_CHECK_RET(lyxml_get_element(&st->yin_ctx->xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len));
    LY_CHECK_RET(yin_load_attributes(st->yin_ctx, data, &attrs));
    ret = yin_parse_content(st->yin_ctx, subelems, 71, data, yin_match_keyword(st->yin_ctx, name.value, name.len, prefix.value, prefix.len, YANG_NONE), text, exts);
    LY_ARRAY_FREE(attrs);
    if (valid) {
        assert_int_equal(st->yin_ctx->xml_ctx.status, LYXML_END);
    }
    /* reset status */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    return ret;
}

#define EXT_SUBELEM "<myext:c-define name=\"MY_MTU\" xmlns:myext=\"urn:example:extensions\"/>"

static void
test_enum_elem(void **state)
{
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
                EXT_SUBELEM
           "</enum>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.enums->name, "enum-name");
    assert_string_equal(*type.enums->iffeatures, "feature");
    assert_int_equal(type.enums->value, 55);
    assert_true((type.enums->flags & LYS_STATUS_DEPRC) && (type.enums->flags & LYS_SET_VALUE));
    assert_string_equal(type.enums->dsc, "desc...");
    assert_string_equal(type.enums->ref, "ref...");
    assert_string_equal(type.enums->exts->name, "myext:c-define");
    assert_int_equal(type.enums->exts->insubstmt_index, 0);
    assert_int_equal(type.enums->exts->insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof type);

    data = ELEMENT_WRAPPER_START
           "<enum name=\"enum-name\"></enum>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.enums->name, "enum-name");
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof type);

    st->finished_correctly = true;
}

static void
test_bit_elem(void **state)
{
    struct state *st = *state;
    struct lysp_type type = {};
    const char *data;
    data = ELEMENT_WRAPPER_START
           "<bit name=\"bit-name\">"
                "<if-feature name=\"feature\" />"
                "<position value=\"55\" />"
                "<status value=\"deprecated\" />"
                "<description><text>desc...</text></description>"
                "<reference><text>ref...</text></reference>"
                EXT_SUBELEM
           "</bit>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.bits->name, "bit-name");
    assert_string_equal(*type.bits->iffeatures, "feature");
    assert_int_equal(type.bits->value, 55);
    assert_true((type.bits->flags & LYS_STATUS_DEPRC) && (type.bits->flags & LYS_SET_VALUE));
    assert_string_equal(type.bits->dsc, "desc...");
    assert_string_equal(type.bits->ref, "ref...");
    assert_string_equal(type.bits->exts->name, "myext:c-define");
    assert_int_equal(type.bits->exts->insubstmt_index, 0);
    assert_int_equal(type.bits->exts->insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof type);

    data = ELEMENT_WRAPPER_START
           "<bit name=\"bit-name\"> </bit>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.bits->name, "bit-name");
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof type);

    st->finished_correctly = true;
}

static void
test_meta_elem(void **state)
{
    struct state *st = *state;
    char *value = NULL;
    const char *data;
    struct lysp_ext_instance *exts = NULL;

    /* organization element */
    data = ELEMENT_WRAPPER_START
                "<organization><text>organization...</text>" EXT_SUBELEM EXT_SUBELEM "</organization>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_ORGANIZATION);
    assert_string_equal(exts[1].name, "myext:c-define");
    assert_int_equal(exts[1].insubstmt_index, 0);
    assert_int_equal(exts[1].insubstmt, LYEXT_SUBSTMT_ORGANIZATION);
    assert_string_equal(value, "organization...");
    FREE_STRING(st->ctx, value);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    value = NULL;
    exts = NULL;

    /* contact element */
    data = ELEMENT_WRAPPER_START
                "<contact><text>contact...</text>" EXT_SUBELEM "</contact>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_CONTACT);
    assert_string_equal(value, "contact...");
    FREE_STRING(st->ctx, value);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
    value = NULL;

    /* description element */
    data = ELEMENT_WRAPPER_START
                "<description><text>description...</text>" EXT_SUBELEM "</description>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(value, "description...");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_DESCRIPTION);
    FREE_STRING(st->ctx, value);
    value = NULL;
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;

    /* reference element */
    data = ELEMENT_WRAPPER_START
                "<reference><text>reference...</text>" EXT_SUBELEM "</reference>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(value, "reference...");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_REFERENCE);
    FREE_STRING(st->ctx, value);
    value = NULL;
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;

    /* reference element */
    data = ELEMENT_WRAPPER_START
                "<reference invalid=\"text\"><text>reference...</text>""</reference>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, false), LY_EVALID);
    logbuf_assert("Unexpected attribute \"invalid\" of reference element. Line number 1.");
    FREE_STRING(st->ctx, value);
    value = NULL;
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;

    /* missing text subelement */
    data = ELEMENT_WRAPPER_START
                "<reference>reference...</reference>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement text of reference element. Line number 1.");

    /* reference element */
    data = ELEMENT_WRAPPER_START
                "<reference>" EXT_SUBELEM "<text>reference...</text></reference>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, false), LY_EVALID);
    logbuf_assert("Subelement text of reference element must be defined as first subelement. Line number 1.");
    FREE_STRING(st->ctx, value);
    value = NULL;
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;

    st->finished_correctly = true;
}

static void
test_import_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_import *imports = NULL;
    struct import_meta imp_meta = {"prefix", &imports};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<import module=\"a\">"
                    EXT_SUBELEM
                    "<prefix value=\"a_mod\"/>"
                    "<revision-date date=\"2015-01-01\"></revision-date>"
                    "<description><text>import description</text></description>"
                    "<reference><text>import reference</text></reference>"
                "</import>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &imp_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(imports->name, "a");
    assert_string_equal(imports->prefix, "a_mod");
    assert_string_equal(imports->rev, "2015-01-01");
    assert_string_equal(imports->dsc, "import description");
    assert_string_equal(imports->ref, "import reference");
    assert_string_equal(imports->exts->name, "myext:c-define");
    assert_int_equal(imports->exts->insubstmt, LYEXT_SUBSTMT_SELF);
    assert_int_equal(imports->exts->insubstmt_index, 0);
    FREE_ARRAY(st->ctx, imports, lysp_import_free);
    imports = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START
                "<import module=\"a\">"
                    "<prefix value=\"a_mod\"/>"
                "</import>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &imp_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(imports->prefix, "a_mod");
    FREE_ARRAY(st->ctx, imports, lysp_import_free);
    imports = NULL;

    /* invalid (missing prefix) */
    data = ELEMENT_WRAPPER_START "<import module=\"a\"></import>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &imp_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement prefix of import element. Line number 1.");
    FREE_ARRAY(st->ctx, imports, lysp_import_free);
    imports = NULL;

    /* invalid reused prefix */
    data = ELEMENT_WRAPPER_START
                "<import module=\"a\">"
                    "<prefix value=\"prefix\"/>"
                "</import>"
                "<import module=\"a\">"
                    "<prefix value=\"prefix\"/>"
                "</import>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &imp_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Prefix \"prefix\" already used as module prefix. Line number 1.");
    FREE_ARRAY(st->ctx, imports, lysp_import_free);
    imports = NULL;

    st->finished_correctly = true;
}

static void
test_status_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint16_t flags = 0;
    struct lysp_ext_instance *exts = NULL;

    /* test valid values */
    data = ELEMENT_WRAPPER_START "<status value=\"current\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags & LYS_STATUS_CURR);

    data = ELEMENT_WRAPPER_START "<status value=\"deprecated\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags & LYS_STATUS_DEPRC);

    data = ELEMENT_WRAPPER_START "<status value=\"obsolete\">"EXT_SUBELEM"</status>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, &exts, true), LY_SUCCESS);
    assert_true(flags & LYS_STATUS_OBSLT);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_STATUS);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;

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
                EXT_SUBELEM
           "</extension>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &ext, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(ext->name, "ext_name");
    assert_string_equal(ext->argument, "arg");
    assert_true(ext->flags & LYS_STATUS_CURR);
    assert_string_equal(ext->dsc, "ext_desc");
    assert_string_equal(ext->ref, "ext_ref");
    assert_string_equal(ext->exts[0].name, "myext:c-define");
    assert_int_equal(ext->exts[0].insubstmt_index, 0);
    assert_int_equal(ext->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<yin-element value=\"true\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_TRUE);

    data = ELEMENT_WRAPPER_START "<yin-element value=\"false\">" EXT_SUBELEM "</yin-element>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, &exts, true), LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_TRUE);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_YINELEM);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);

    data = ELEMENT_WRAPPER_START "<yin-element value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, false), LY_EVALID);
    assert_true(flags & LYS_YINELEM_TRUE);
    logbuf_assert("Invalid value \"invalid\" of \"yin-element\". Line number 1.");
    st->finished_correctly = true;
}

static void
test_yangversion_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint8_t version = 0;
    struct lysp_ext_instance *exts = NULL;

    /* valid values */
    data = ELEMENT_WRAPPER_START "<yang-version value=\"1.0\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &version, NULL, NULL, true), LY_SUCCESS);
    assert_true(version & LYS_VERSION_1_0);
    assert_int_equal(st->yin_ctx->mod_version, LYS_VERSION_1_0);

    data = ELEMENT_WRAPPER_START "<yang-version value=\"1.1\">" EXT_SUBELEM "</yang-version>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &version, NULL, &exts, true), LY_SUCCESS);
    assert_true(version & LYS_VERSION_1_1);
    assert_int_equal(st->yin_ctx->mod_version, LYS_VERSION_1_1);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_VERSION);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);

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
    struct lysp_ext_instance *exts = NULL;

    /* valid values */
    data = ELEMENT_WRAPPER_START "<mandatory value=\"true\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &man, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(man, LYS_MAND_TRUE);
    man = 0;

    data = ELEMENT_WRAPPER_START "<mandatory value=\"false\">" EXT_SUBELEM "</mandatory>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &man, NULL, &exts, true), LY_SUCCESS);
    assert_int_equal(man, LYS_MAND_FALSE);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_MANDATORY);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);

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
    struct lysp_ext_instance *exts = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
           "<argument name=\"arg-name\">"
                "<yin-element value=\"true\" />"
                EXT_SUBELEM
           "</argument>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &arg_meta, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(arg, "arg-name");
    assert_true(flags & LYS_YINELEM_TRUE);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_ARGUMENT);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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
    struct lysp_ext_instance *exts = NULL;
    struct lysp_type type = {};

    /* as identity subelement */
    data = "<identity xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<base name=\"base-name\">"
                    EXT_SUBELEM
                "</base>"
           "</identity>";
    assert_int_equal(test_element_helper(st, &data, &bases, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(*bases, "base-name");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_BASE);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
    FREE_STRING(st->ctx, *bases);
    LY_ARRAY_FREE(bases);

    /* as type subelement */
    data = "<type xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<base name=\"base-name\">"
                    EXT_SUBELEM
                "</base>"
           "</type>";
    assert_int_equal(test_element_helper(st, &data, &type, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(*type.bases, "base-name");
    assert_true(type.flags & LYS_SET_BASE);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_BASE);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START
                "<belongs-to module=\"module-name\"><prefix value=\"pref\"/>"EXT_SUBELEM"</belongs-to>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &submod, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(submod.belongsto, "module-name");
    assert_string_equal(submod.prefix, "pref");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_BELONGSTO);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<config value=\"true\">" EXT_SUBELEM "</config>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, &exts, true), LY_SUCCESS);
    assert_true(flags & LYS_CONFIG_W);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_CONFIG);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
    flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"false\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags & LYS_CONFIG_R);
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<default value=\"defaul-value\">"EXT_SUBELEM"</default>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(val, "defaul-value");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_DEFAULT);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<error-app-tag value=\"val\">"EXT_SUBELEM"</error-app-tag>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(val, "val");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_ERRTAG);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<error-message><value>val</value>"EXT_SUBELEM"</error-message>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(val, "val");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_ERRMSG);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
    FREE_STRING(st->ctx, val);

    data = ELEMENT_WRAPPER_START "<error-message></error-message>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement value of error-message element. Line number 1.");

    data = ELEMENT_WRAPPER_START "<error-message invalid=\"text\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Unexpected attribute \"invalid\" of error-message element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_fracdigits_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    /* valid value */
    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"10\">"EXT_SUBELEM"</fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.exts[0].name, "myext:c-define");
    assert_int_equal(type.exts[0].insubstmt_index, 0);
    assert_int_equal(type.exts[0].insubstmt, LYEXT_SUBSTMT_FRACDIGITS);
    assert_int_equal(type.fraction_digits, 10);
    assert_true(type.flags & LYS_SET_FRDIGITS);
    FREE_ARRAY(st->ctx, type.exts, lysp_ext_instance_free);

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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<if-feature name=\"local-storage\">"EXT_SUBELEM"</if-feature>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &iffeatures, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(*iffeatures, "local-storage");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_IFFEATURE);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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
                    EXT_SUBELEM
                "</length>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.length->arg, "length-str");
    assert_string_equal(type.length->emsg, "err-msg");
    assert_string_equal(type.length->eapptag, "err-app-tag");
    assert_string_equal(type.length->dsc, "desc");
    assert_string_equal(type.length->ref, "ref");
    assert_true(type.flags & LYS_SET_LENGTH);
    assert_string_equal(type.length->exts[0].name, "myext:c-define");
    assert_int_equal(type.length->exts[0].insubstmt_index, 0);
    assert_int_equal(type.length->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
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
    assert_true(type.flags & LYS_SET_LENGTH);
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<modifier value=\"invert-match\">" EXT_SUBELEM "</modifier>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &pat, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(pat, "\x015pattern");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_MODIFIER);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<namespace uri=\"ns\">" EXT_SUBELEM "</namespace>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &ns, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(ns, "ns");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_NAMESPACE);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
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

    data = ELEMENT_WRAPPER_START "<path value=\"path-val\">" EXT_SUBELEM "</path>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal("path-val", type.path);
    assert_true(type.flags & LYS_SET_PATH);
    assert_string_equal(type.exts[0].name, "myext:c-define");
    assert_int_equal(type.exts[0].insubstmt_index, 0);
    assert_int_equal(type.exts[0].insubstmt, LYEXT_SUBSTMT_PATH);
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
                    EXT_SUBELEM
                "</pattern>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_true(type.flags & LYS_SET_PATTERN);
    assert_string_equal(type.patterns->arg, "\x015super_pattern");
    assert_string_equal(type.patterns->dsc, "pattern-desc");
    assert_string_equal(type.patterns->eapptag, "err-app-tag-value");
    assert_string_equal(type.patterns->emsg, "err-msg-value");
    assert_string_equal(type.patterns->dsc, "pattern-desc");
    assert_string_equal(type.patterns->ref, "pattern-ref");
    assert_string_equal(type.patterns->exts[0].name, "myext:c-define");
    assert_int_equal(type.patterns->exts[0].insubstmt_index, 0);
    assert_int_equal(type.patterns->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
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
    data = ELEMENT_WRAPPER_START "<value value=\"55\">" EXT_SUBELEM "</value>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 55);
    assert_true(en.flags & LYS_SET_VALUE);
    assert_string_equal(en.exts[0].name, "myext:c-define");
    assert_int_equal(en.exts[0].insubstmt_index, 0);
    assert_int_equal(en.exts[0].insubstmt, LYEXT_SUBSTMT_VALUE);
    FREE_ARRAY(st->ctx, en.exts, lysp_ext_instance_free);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"-55\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, -55);
    assert_true(en.flags & LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 0);
    assert_true(en.flags & LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"-0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 0);
    assert_true(en.flags & LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    /* valid positions */
    data = ELEMENT_WRAPPER_START "<position value=\"55\">" EXT_SUBELEM "</position>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 55);
    assert_true(en.flags & LYS_SET_VALUE);
    assert_string_equal(en.exts[0].name, "myext:c-define");
    assert_int_equal(en.exts[0].insubstmt_index, 0);
    assert_int_equal(en.exts[0].insubstmt, LYEXT_SUBSTMT_POSITION);
    FREE_ARRAY(st->ctx, en.exts, lysp_ext_instance_free);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<position value=\"0\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(en.value, 0);
    assert_true(en.flags & LYS_SET_VALUE);
    memset(&en, 0, sizeof(en));

    /* invalid values */
    data = ELEMENT_WRAPPER_START "<value value=\"99999999999999999999999\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"99999999999999999999999\" of \"value\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<value value=\"1k\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"1k\" of \"value\". Line number 1.");

    data = ELEMENT_WRAPPER_START "<value value=\"\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"\" of \"value\". Line number 1.");

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

    data = ELEMENT_WRAPPER_START "<position value=\"\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &en, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"\" of \"position\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_prefix_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *value = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<prefix value=\"pref\">" EXT_SUBELEM "</prefix>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(value, "pref");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_PREFIX);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    exts = NULL;
    FREE_STRING(st->ctx, value);

    data = ELEMENT_WRAPPER_START "<prefix value=\"pref\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &value, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(value, "pref");
    FREE_STRING(st->ctx, value);

    st->finished_correctly = true;
}

static void
test_range_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<range value=\"range-str\">"
                    "<error-message><value>err-msg</value></error-message>"
                    "<error-app-tag value=\"err-app-tag\" />"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    EXT_SUBELEM
                "</range>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.range->arg, "range-str");
    assert_string_equal(type.range->dsc, "desc");
    assert_string_equal(type.range->eapptag, "err-app-tag");
    assert_string_equal(type.range->emsg, "err-msg");
    assert_string_equal(type.range->ref, "ref");
    assert_true(type.flags & LYS_SET_RANGE);
    assert_string_equal(type.range->exts[0].name, "myext:c-define");
    assert_int_equal(type.range->exts[0].insubstmt_index, 0);
    assert_int_equal(type.range->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<range value=\"range-str\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.range->arg, "range-str");
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    st->finished_correctly = true;
}

static void
test_reqinstance_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    data = ELEMENT_WRAPPER_START "<require-instance value=\"true\">" EXT_SUBELEM "</require-instance>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(type.require_instance, 1);
    assert_true(type.flags & LYS_SET_REQINST);
    assert_string_equal(type.exts[0].name, "myext:c-define");
    assert_int_equal(type.exts[0].insubstmt_index, 0);
    assert_int_equal(type.exts[0].insubstmt, LYEXT_SUBSTMT_REQINSTANCE);
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    data = ELEMENT_WRAPPER_START "<require-instance value=\"false\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(type.require_instance, 0);
    assert_true(type.flags & LYS_SET_REQINST);
    memset(&type, 0, sizeof(type));

    data = ELEMENT_WRAPPER_START "<require-instance value=\"invalid\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, false), LY_EVALID);
    memset(&type, 0, sizeof(type));
    logbuf_assert("Invalid value \"invalid\" of \"require-instance\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_revision_date_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    char rev[LY_REV_SIZE];
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-01-01\">"EXT_SUBELEM"</revision-date>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, rev, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(rev, "2000-01-01");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_REVISIONDATE);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-01-01\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, rev, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(rev, "2000-01-01");

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-50-05\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, rev, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"2000-50-05\" of \"revision-date\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_unique_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char **values = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<unique tag=\"tag\">"EXT_SUBELEM"</unique>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &values, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(*values, "tag");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_UNIQUE);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    FREE_STRING(st->ctx, *values);
    LY_ARRAY_FREE(values);
    values = NULL;

    data = ELEMENT_WRAPPER_START "<unique tag=\"tag\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &values, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(*values, "tag");
    FREE_STRING(st->ctx, *values);
    LY_ARRAY_FREE(values);
    values = NULL;

    st->finished_correctly = true;
}

static void
test_units_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *values = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<units name=\"name\">"EXT_SUBELEM"</units>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &values, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(values, "name");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_UNITS);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    FREE_STRING(st->ctx, values);
    values = NULL;

    data = ELEMENT_WRAPPER_START "<units name=\"name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &values, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(values, "name");
    FREE_STRING(st->ctx, values);
    values = NULL;

    st->finished_correctly = true;
}

static void
test_when_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_when *when = NULL;

    data = ELEMENT_WRAPPER_START
                "<when condition=\"cond\">"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    EXT_SUBELEM
                "</when>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &when, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(when->cond, "cond");
    assert_string_equal(when->dsc, "desc");
    assert_string_equal(when->ref, "ref");
    assert_string_equal(when->exts[0].name, "myext:c-define");
    assert_int_equal(when->exts[0].insubstmt_index, 0);
    assert_int_equal(when->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_when_free(st->ctx, when);
    free(when);
    when = NULL;

    data = ELEMENT_WRAPPER_START "<when condition=\"cond\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &when, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(when->cond, "cond");
    lysp_when_free(st->ctx, when);
    free(when);
    when = NULL;

    st->finished_correctly = true;
}

static void
test_yin_text_value_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *val;

    data = ELEMENT_WRAPPER_START "<text>text</text>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(val, "text");
    FREE_STRING(st->ctx, val);

    data = "<error-message xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <value>text</value> </error-message>";
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(val, "text");
    FREE_STRING(st->ctx, val);

    data = ELEMENT_WRAPPER_START "<text></text>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal("", val);
    FREE_STRING(st->ctx, val);

    st->finished_correctly = true;
}

static void
test_type_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<type name=\"type-name\">"
                    "<base name=\"base-name\"/>"
                    "<bit name=\"bit\"/>"
                    "<enum name=\"enum\"/>"
                    "<fraction-digits value=\"2\"/>"
                    "<length value=\"length\"/>"
                    "<path value=\"path\"/>"
                    "<pattern value=\"pattern\"/>"
                    "<range value=\"range\" />"
                    "<require-instance value=\"true\"/>"
                    "<type name=\"sub-type-name\"/>"
                    EXT_SUBELEM
                "</type>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(type.name, "type-name");
    assert_string_equal(*type.bases, "base-name");
    assert_string_equal(type.bits->name,  "bit");
    assert_string_equal(type.enums->name,  "enum");
    assert_int_equal(type.fraction_digits, 2);
    assert_string_equal(type.length->arg, "length");
    assert_string_equal(type.path, "path");
    assert_string_equal(type.patterns->arg, "\006pattern");
    assert_string_equal(type.range->arg, "range");
    assert_int_equal(type.require_instance, 1);
    assert_string_equal(type.types->name, "sub-type-name");
    assert_string_equal(type.exts[0].name, "myext:c-define");
    assert_int_equal(type.exts[0].insubstmt_index, 0);
    assert_int_equal(type.exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    assert_true(type.flags & LYS_SET_BASE);
    assert_true(type.flags & LYS_SET_BIT);
    assert_true(type.flags & LYS_SET_ENUM);
    assert_true(type.flags & LYS_SET_FRDIGITS);
    assert_true(type.flags & LYS_SET_LENGTH);
    assert_true(type.flags & LYS_SET_PATH);
    assert_true(type.flags & LYS_SET_PATTERN);
    assert_true(type.flags & LYS_SET_RANGE);
    assert_true(type.flags & LYS_SET_REQINST);
    assert_true(type.flags & LYS_SET_TYPE);
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<type name=\"type-name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &type, NULL, NULL, true), LY_SUCCESS);
    lysp_type_free(st->ctx, &type);
    memset(&type, 0, sizeof(type));

    st->finished_correctly = true;
}

static void
test_max_elems_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node_list list = {};
    struct lysp_node_leaflist llist = {};
    struct lysp_refine refine = {};

    data = "<refine xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"unbounded\">"EXT_SUBELEM"</max-elements> </refine>";
    assert_int_equal(test_element_helper(st, &data, &refine, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(refine.max, 0);
    assert_true(refine.flags & LYS_SET_MAX);
    assert_string_equal(refine.exts[0].name, "myext:c-define");
    assert_int_equal(refine.exts[0].insubstmt_index, 0);
    assert_int_equal(refine.exts[0].insubstmt, LYEXT_SUBSTMT_MAX);
    FREE_ARRAY(st->ctx, refine.exts, lysp_ext_instance_free);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"5\">"EXT_SUBELEM"</max-elements> </list>";
    assert_int_equal(test_element_helper(st, &data, &list, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(list.max, 5);
    assert_true(list.flags & LYS_SET_MAX);
    assert_string_equal(list.exts[0].name, "myext:c-define");
    assert_int_equal(list.exts[0].insubstmt_index, 0);
    assert_int_equal(list.exts[0].insubstmt, LYEXT_SUBSTMT_MAX);
    FREE_ARRAY(st->ctx, list.exts, lysp_ext_instance_free);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"85\">"EXT_SUBELEM"</max-elements> </leaf-list>";
    assert_int_equal(test_element_helper(st, &data, &llist, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(llist.max, 85);
    assert_true(llist.flags & LYS_SET_MAX);
    assert_string_equal(llist.exts[0].name, "myext:c-define");
    assert_int_equal(llist.exts[0].insubstmt_index, 0);
    assert_int_equal(llist.exts[0].insubstmt, LYEXT_SUBSTMT_MAX);
    FREE_ARRAY(st->ctx, llist.exts, lysp_ext_instance_free);

    data = "<refine xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"10\"/> </refine>";
    assert_int_equal(test_element_helper(st, &data, &refine, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(refine.max, 10);
    assert_true(refine.flags & LYS_SET_MAX);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"0\"/> </list>";
    assert_int_equal(test_element_helper(st, &data, &list, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"0\" of \"max-elements\". Line number 1.");

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"-10\"/> </list>";
    assert_int_equal(test_element_helper(st, &data, &list, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"-10\" of \"max-elements\". Line number 1.");

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"k\"/> </list>";
    assert_int_equal(test_element_helper(st, &data, &list, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"k\" of \"max-elements\". Line number 1.");

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"u12\"/> </list>";
    assert_int_equal(test_element_helper(st, &data, &list, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"u12\" of \"max-elements\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_min_elems_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node_list list = {};
    struct lysp_node_leaflist llist = {};
    struct lysp_refine refine = {};

    data = "<refine xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"0\">"EXT_SUBELEM"</min-elements> </refine>";
    assert_int_equal(test_element_helper(st, &data, &refine, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(refine.min, 0);
    assert_true(refine.flags & LYS_SET_MIN);
    assert_string_equal(refine.exts[0].name, "myext:c-define");
    assert_int_equal(refine.exts[0].insubstmt_index, 0);
    assert_int_equal(refine.exts[0].insubstmt, LYEXT_SUBSTMT_MIN);
    FREE_ARRAY(st->ctx, refine.exts, lysp_ext_instance_free);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"41\">"EXT_SUBELEM"</min-elements> </list>";
    assert_int_equal(test_element_helper(st, &data, &list, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(list.min, 41);
    assert_true(list.flags & LYS_SET_MIN);
    assert_string_equal(list.exts[0].name, "myext:c-define");
    assert_int_equal(list.exts[0].insubstmt_index, 0);
    assert_int_equal(list.exts[0].insubstmt, LYEXT_SUBSTMT_MIN);
    FREE_ARRAY(st->ctx, list.exts, lysp_ext_instance_free);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"50\">"EXT_SUBELEM"</min-elements> </leaf-list>";
    assert_int_equal(test_element_helper(st, &data, &llist, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(llist.min, 50);
    assert_true(llist.flags & LYS_SET_MIN);
    assert_string_equal(llist.exts[0].name, "myext:c-define");
    assert_int_equal(llist.exts[0].insubstmt_index, 0);
    assert_int_equal(llist.exts[0].insubstmt, LYEXT_SUBSTMT_MIN);
    FREE_ARRAY(st->ctx, llist.exts, lysp_ext_instance_free);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"-5\"/> </leaf-list>";
    assert_int_equal(test_element_helper(st, &data, &llist, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Value \"-5\" is out of \"min-elements\" bounds. Line number 1.");

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"99999999999999999\"/> </leaf-list>";
    assert_int_equal(test_element_helper(st, &data, &llist, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Value \"99999999999999999\" is out of \"min-elements\" bounds. Line number 1.");

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"5k\"/> </leaf-list>";
    assert_int_equal(test_element_helper(st, &data, &llist, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"5k\" of \"min-elements\". Line number 1.");

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"05\"/> </leaf-list>";
    assert_int_equal(test_element_helper(st, &data, &llist, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"05\" of \"min-elements\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_ordby_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    uint16_t flags = 0;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"system\">"EXT_SUBELEM"</ordered-by>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, &exts, true), LY_SUCCESS);
    assert_true(flags & LYS_ORDBY_SYSTEM);
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_ORDEREDBY);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"user\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, true), LY_SUCCESS);
    assert_true(flags & LYS_ORDBY_USER);

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"inv\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &flags, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"inv\" of \"ordered-by\". Line number 1.");

    st->finished_correctly = true;
}

static void
test_any_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {.parent = NULL, .siblings = &siblings};
    struct lysp_node_anydata *parsed = NULL;

    /* anyxml max subelems */
    data = ELEMENT_WRAPPER_START
                "<anyxml name=\"any-name\">"
                    "<config value=\"true\" />"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"feature\" />"
                    "<mandatory value=\"true\" />"
                    "<must condition=\"must-cond\" />"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"deprecated\"/>"
                    "<when condition=\"when-cond\"/>"
                    EXT_SUBELEM
                "</anyxml>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_anydata *)siblings;
    assert_null(parsed->parent);
    assert_int_equal(parsed->nodetype, LYS_ANYXML);
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_true(parsed->flags & LYS_MAND_TRUE);
    assert_true(parsed->flags & LYS_STATUS_DEPRC);
    assert_null(parsed->next);
    assert_string_equal(parsed->name, "any-name");
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_string_equal(*parsed->iffeatures, "feature");
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    /* anydata max subelems */
    data = ELEMENT_WRAPPER_START
                "<anydata name=\"any-name\">"
                    "<config value=\"true\" />"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"feature\" />"
                    "<mandatory value=\"true\" />"
                    "<must condition=\"must-cond\" />"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"deprecated\"/>"
                    "<when condition=\"when-cond\"/>"
                    EXT_SUBELEM
                "</anydata>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_anydata *)siblings;
    assert_null(parsed->parent);
    assert_int_equal(parsed->nodetype, LYS_ANYDATA);
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_true(parsed->flags & LYS_MAND_TRUE);
    assert_true(parsed->flags & LYS_STATUS_DEPRC);
    assert_null(parsed->next);
    assert_string_equal(parsed->name, "any-name");
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_string_equal(*parsed->iffeatures, "feature");
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    /* min subelems */
    node_meta.parent = (void *)0x10;
    data = ELEMENT_WRAPPER_START "<anydata name=\"any-name\"> </anydata>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_anydata *)siblings;
    assert_ptr_equal(parsed->parent, node_meta.parent);
    assert_int_equal(parsed->nodetype, LYS_ANYDATA);
    assert_null(parsed->next);
    assert_null(parsed->exts);
    lysp_node_free(st->ctx, siblings);

    st->finished_correctly = true;
}

static void
test_leaf_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {.parent = NULL, .siblings = &siblings};
    struct lysp_node_leaf *parsed = NULL;

    /* max elements */
    data = ELEMENT_WRAPPER_START
                "<leaf name=\"leaf\">"
                    "<config value=\"true\" />"
                    "<default value=\"def-val\"/>"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"feature\" />"
                    "<mandatory value=\"true\" />"
                    "<must condition=\"must-cond\" />"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"deprecated\"/>"
                    "<type name=\"type\"/>"
                    "<units name=\"uni\"/>"
                    "<when condition=\"when-cond\"/>"
                    EXT_SUBELEM
                "</leaf>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_leaf *)siblings;
    assert_null(parsed->parent);
    assert_int_equal(parsed->nodetype, LYS_LEAF);
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_true(parsed->flags & LYS_MAND_TRUE);
    assert_true(parsed->flags & LYS_STATUS_DEPRC);
    assert_null(parsed->next);
    assert_string_equal(parsed->name, "leaf");
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_string_equal(*parsed->iffeatures, "feature");
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    assert_string_equal(parsed->musts->arg, "must-cond");
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    assert_string_equal(parsed->dflt, "def-val");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    /* min elements */
    data = ELEMENT_WRAPPER_START "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_leaf *)siblings;
    assert_string_equal(parsed->name, "leaf");
    assert_string_equal(parsed->type.name, "type");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    st->finished_correctly = true;
}

static void
test_leaf_list_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {.parent = NULL, .siblings = &siblings};
    struct lysp_node_leaflist *parsed = NULL;

    data = ELEMENT_WRAPPER_START
                "<leaf-list name=\"llist\">"
                    "<config value=\"true\" />"
                    "<default value=\"def-val0\"/>"
                    "<default value=\"def-val1\"/>"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"feature\"/>"
                    "<max-elements value=\"5\"/>"
                    "<must condition=\"must-cond\"/>"
                    "<ordered-by value=\"user\" />"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<type name=\"type\"/>"
                    "<units name=\"uni\"/>"
                    "<when condition=\"when-cond\"/>"
                    EXT_SUBELEM
                "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    assert_string_equal(parsed->dflts[0], "def-val0");
    assert_string_equal(parsed->dflts[1], "def-val1");
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(*parsed->iffeatures, "feature");
    assert_int_equal(parsed->max, 5);
    assert_string_equal(parsed->musts->arg, "must-cond");
    assert_string_equal(parsed->name, "llist");
    assert_null(parsed->next);
    assert_int_equal(parsed->nodetype, LYS_LEAFLIST);
    assert_null(parsed->parent);
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_true(parsed->flags & LYS_ORDBY_USER);
    assert_true(parsed->flags & LYS_STATUS_CURR);
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
                "<leaf-list name=\"llist\">"
                    "<config value=\"true\" />"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"feature\"/>"
                    "<min-elements value=\"5\"/>"
                    "<must condition=\"must-cond\"/>"
                    "<ordered-by value=\"user\" />"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<type name=\"type\"/>"
                    "<units name=\"uni\"/>"
                    "<when condition=\"when-cond\"/>"
                    EXT_SUBELEM
                "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(*parsed->iffeatures, "feature");
    assert_int_equal(parsed->min, 5);
    assert_string_equal(parsed->musts->arg, "must-cond");
    assert_string_equal(parsed->name, "llist");
    assert_null(parsed->next);
    assert_int_equal(parsed->nodetype, LYS_LEAFLIST);
    assert_null(parsed->parent);
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_true(parsed->flags & LYS_ORDBY_USER);
    assert_true(parsed->flags & LYS_STATUS_CURR);
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
                "<leaf-list name=\"llist\">"
                    "<config value=\"true\" />"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"feature\"/>"
                    "<max-elements value=\"15\"/>"
                    "<min-elements value=\"5\"/>"
                    "<must condition=\"must-cond\"/>"
                    "<ordered-by value=\"user\" />"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<type name=\"type\"/>"
                    "<units name=\"uni\"/>"
                    "<when condition=\"when-cond\"/>"
                "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(*parsed->iffeatures, "feature");
    assert_int_equal(parsed->min, 5);
    assert_int_equal(parsed->max, 15);
    assert_string_equal(parsed->musts->arg, "must-cond");
    assert_string_equal(parsed->name, "llist");
    assert_null(parsed->next);
    assert_int_equal(parsed->nodetype, LYS_LEAFLIST);
    assert_null(parsed->parent);
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_true(parsed->flags & LYS_ORDBY_USER);
    assert_true(parsed->flags & LYS_STATUS_CURR);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
                "<leaf-list name=\"llist\">"
                    "<type name=\"type\"/>"
                "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    assert_string_equal(parsed->name, "llist");
    assert_string_equal(parsed->type.name, "type");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    /* invalid combinations */
    data = ELEMENT_WRAPPER_START
                "<leaf-list name=\"llist\">"
                    "<max-elements value=\"5\"/>"
                    "<min-elements value=\"15\"/>"
                    "<type name=\"type\"/>"
                "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid combination of min-elements and max-elements: min value 15 is bigger than the max value 5. Line number 1.");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
                "<leaf-list name=\"llist\">"
                    "<default value=\"def-val1\"/>"
                    "<min-elements value=\"15\"/>"
                    "<type name=\"type\"/>"
                "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid combination of keywords \"min-elements\" and \"default\" as substatements of \"leaf-list\". Line number 1.");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
                "<leaf-list name=\"llist\">"
                "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory subelement type of leaf-list element. Line number 1.");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    st->finished_correctly = true;
}

static void
test_presence_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *val;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<presence value=\"presence-val\">"EXT_SUBELEM"</presence>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(val, "presence-val");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_PRESENCE);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    FREE_STRING(st->ctx, val);

    data = ELEMENT_WRAPPER_START "<presence value=\"presence-val\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(val, "presence-val");
    FREE_STRING(st->ctx, val);

    data = ELEMENT_WRAPPER_START "<presence/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory attribute value of presence element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_key_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    const char *val;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<key value=\"key-value\">"EXT_SUBELEM"</key>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, &exts, true), LY_SUCCESS);
    assert_string_equal(val, "key-value");
    assert_string_equal(exts[0].name, "myext:c-define");
    assert_int_equal(exts[0].insubstmt_index, 0);
    assert_int_equal(exts[0].insubstmt, LYEXT_SUBSTMT_KEY);
    FREE_ARRAY(st->ctx, exts, lysp_ext_instance_free);
    FREE_STRING(st->ctx, val);

    data = ELEMENT_WRAPPER_START "<key value=\"key-value\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(val, "key-value");
    FREE_STRING(st->ctx, val);

    data = ELEMENT_WRAPPER_START "<key/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &val, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Missing mandatory attribute value of key element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_typedef_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_tpdf *tpdfs = NULL;
    struct typedef_meta typdef_meta = {NULL, &tpdfs};

    data = ELEMENT_WRAPPER_START
                "<typedef name=\"tpdf-name\">"
                    "<default value=\"def-val\"/>"
                    "<description><text>desc-text</text></description>"
                    "<reference><text>ref-text</text></reference>"
                    "<status value=\"current\"/>"
                    "<type name=\"type\"/>"
                    "<units name=\"uni\"/>"
                    EXT_SUBELEM
                "</typedef>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &typdef_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(tpdfs[0].dflt, "def-val");
    assert_string_equal(tpdfs[0].dsc, "desc-text");
    assert_string_equal(tpdfs[0].name, "tpdf-name");
    assert_string_equal(tpdfs[0].ref, "ref-text");
    assert_string_equal(tpdfs[0].type.name, "type");
    assert_string_equal(tpdfs[0].units, "uni");
    assert_true(tpdfs[0].flags & LYS_STATUS_CURR);
    assert_string_equal(tpdfs[0].exts[0].name, "myext:c-define");
    assert_int_equal(tpdfs[0].exts[0].insubstmt_index, 0);
    assert_int_equal(tpdfs[0].exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, tpdfs, lysp_tpdf_free);
    tpdfs = NULL;

    data = ELEMENT_WRAPPER_START
                "<typedef name=\"tpdf-name\">"
                    "<type name=\"type\"/>"
                "</typedef>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &typdef_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(tpdfs[0].name, "tpdf-name");
    assert_string_equal(tpdfs[0].type.name, "type");
    FREE_ARRAY(st->ctx, tpdfs, lysp_tpdf_free);
    tpdfs = NULL;

    st->finished_correctly = true;
}

static void
test_refine_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_refine *refines = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<refine target-node=\"target\">"
                    "<if-feature name=\"feature\" />"
                    "<must condition=\"cond\" />"
                    "<presence value=\"presence\" />"
                    "<default value=\"def\" />"
                    "<config value=\"true\" />"
                    "<mandatory value=\"true\" />"
                    "<min-elements value=\"10\" />"
                    "<max-elements value=\"20\" />"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    EXT_SUBELEM
                "</refine>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &refines, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(refines->nodeid, "target");
    assert_string_equal(*refines->dflts, "def");
    assert_string_equal(refines->dsc, "desc");
    assert_true(refines->flags & LYS_CONFIG_W);
    assert_true(refines->flags & LYS_MAND_TRUE);
    assert_string_equal(*refines->iffeatures, "feature");
    assert_int_equal(refines->max, 20);
    assert_int_equal(refines->min, 10);
    assert_string_equal(refines->musts->arg, "cond");
    assert_string_equal(refines->presence, "presence");
    assert_string_equal(refines->ref, "ref");
    assert_string_equal(refines->exts[0].name, "myext:c-define");
    assert_int_equal(refines->exts[0].insubstmt_index, 0);
    assert_int_equal(refines->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, refines, lysp_refine_free);
    refines = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<refine target-node=\"target\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &refines, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(refines->nodeid, "target");
    FREE_ARRAY(st->ctx, refines, lysp_refine_free);
    refines = NULL;

    st->finished_correctly = true;
}

static void
test_uses_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_uses *parsed = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<uses name=\"uses-name\">"
                    "<when condition=\"cond\" />"
                    "<if-feature name=\"feature\" />"
                    "<status value=\"obsolete\" />"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    "<refine target-node=\"target\"/>"
                    "<augment target-node=\"target\" />"
                    EXT_SUBELEM
                "</uses>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_uses *)&siblings[0];
    assert_string_equal(parsed->name, "uses-name");
    assert_string_equal(parsed->dsc, "desc");
    assert_true(parsed->flags & LYS_STATUS_OBSLT);
    assert_string_equal(*parsed->iffeatures, "feature");
    assert_null(parsed->next);
    assert_int_equal(parsed->nodetype, LYS_USES);
    assert_null(parsed->parent);
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->refines->nodeid, "target");
    assert_string_equal(parsed->when->cond, "cond");
    assert_string_equal(parsed->augments->nodeid, "target");
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<uses name=\"uses-name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(siblings[0].name, "uses-name");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    st->finished_correctly = true;
}

static void
test_revision_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_revision *revs = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<revision date=\"2018-12-25\">"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    EXT_SUBELEM
                "</revision>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &revs, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(revs->date, "2018-12-25");
    assert_string_equal(revs->dsc, "desc");
    assert_string_equal(revs->ref, "ref");
    assert_string_equal(revs->exts[0].name, "myext:c-define");
    assert_int_equal(revs->exts[0].insubstmt_index, 0);
    assert_int_equal(revs->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, revs, lysp_revision_free);
    revs = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<revision date=\"2005-05-05\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &revs, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(revs->date, "2005-05-05");
    FREE_ARRAY(st->ctx, revs, lysp_revision_free);
    revs = NULL;

    /* invalid value */
    data = ELEMENT_WRAPPER_START "<revision date=\"05-05-2005\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &revs, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"05-05-2005\" of \"revision\". Line number 1.");
    FREE_ARRAY(st->ctx, revs, lysp_revision_free);
    revs = NULL;

    st->finished_correctly = true;
}

static void
test_include_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_include *includes = NULL;
    struct include_meta inc_meta = {"module-name", &includes};

    /* max subelems */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<include module=\"mod\">"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    "<revision-date date=\"1999-09-09\"/>"
                    EXT_SUBELEM
                "</include>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inc_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(includes->name, "mod");
    assert_string_equal(includes->dsc, "desc");
    assert_string_equal(includes->ref, "ref");
    assert_string_equal(includes->rev, "1999-09-09");
    assert_string_equal(includes->exts[0].name, "myext:c-define");
    assert_int_equal(includes->exts[0].insubstmt_index, 0);
    assert_int_equal(includes->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, includes, lysp_include_free);
    includes = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<include module=\"mod\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inc_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(includes->name, "mod");
    FREE_ARRAY(st->ctx, includes, lysp_include_free);
    includes = NULL;

    /* invalid combinations */
    st->yin_ctx->mod_version = LYS_VERSION_1_0;
    data = ELEMENT_WRAPPER_START
                "<include module=\"mod\">"
                    "<description><text>desc</text></description>"
                    "<revision-date date=\"1999-09-09\"/>"
                "</include>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inc_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid sub-elemnt \"description\" of \"include\" element - this sub-element is allowed only in modules with version 1.1 or newer. Line number 1.");
    FREE_ARRAY(st->ctx, includes, lysp_include_free);
    includes = NULL;

    st->yin_ctx->mod_version = LYS_VERSION_1_0;
    data = ELEMENT_WRAPPER_START
                "<include module=\"mod\">"
                    "<reference><text>ref</text></reference>"
                    "<revision-date date=\"1999-09-09\"/>"
                "</include>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inc_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid sub-elemnt \"reference\" of \"include\" element - this sub-element is allowed only in modules with version 1.1 or newer. Line number 1.");
    FREE_ARRAY(st->ctx, includes, lysp_include_free);
    includes = NULL;

    st->finished_correctly = true;
}

static void
test_feature_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_feature *features = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<feature name=\"feature-name\">"
                    "<if-feature name=\"iff\"/>"
                    "<status value=\"deprecated\"/>"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    EXT_SUBELEM
                "</feature>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &features, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(features->name, "feature-name");
    assert_string_equal(features->dsc, "desc");
    assert_true(features->flags & LYS_STATUS_DEPRC);
    assert_string_equal(*features->iffeatures, "iff");
    assert_string_equal(features->ref, "ref");
    assert_string_equal(features->exts[0].name, "myext:c-define");
    assert_int_equal(features->exts[0].insubstmt_index, 0);
    assert_int_equal(features->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, features, lysp_feature_free);
    features = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<feature name=\"feature-name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &features, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(features->name, "feature-name");
    FREE_ARRAY(st->ctx, features, lysp_feature_free);
    features = NULL;

    st->finished_correctly = true;
}

static void
test_identity_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_ident *identities = NULL;

    /* max subelems */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<identity name=\"ident-name\">"
                    "<if-feature name=\"iff\"/>"
                    "<base name=\"base-name\"/>"
                    "<status value=\"deprecated\"/>"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    EXT_SUBELEM
                "</identity>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &identities, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(identities->name, "ident-name");
    assert_string_equal(*identities->bases, "base-name");
    assert_string_equal(*identities->iffeatures, "iff");
    assert_string_equal(identities->dsc, "desc");
    assert_string_equal(identities->ref, "ref");
    assert_true(identities->flags & LYS_STATUS_DEPRC);
    assert_string_equal(identities->exts[0].name, "myext:c-define");
    assert_int_equal(identities->exts[0].insubstmt_index, 0);
    assert_int_equal(identities->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, identities, lysp_ident_free);
    identities = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<identity name=\"ident-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &identities, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(identities->name, "ident-name");
    FREE_ARRAY(st->ctx, identities, lysp_ident_free);
    identities = NULL;

    /* invalid */
    st->yin_ctx->mod_version = LYS_VERSION_1_0;
    data = ELEMENT_WRAPPER_START
                "<identity name=\"ident-name\">"
                    "<if-feature name=\"iff\"/>"
                "</identity>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &identities, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid sub-elemnt \"if-feature\" of \"identity\" element - this sub-element is allowed only in modules with version 1.1 or newer. Line number 1.");
    FREE_ARRAY(st->ctx, identities, lysp_ident_free);
    identities = NULL;

    st->finished_correctly = true;
}

static void
test_list_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_list *parsed = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<list name=\"list-name\">"
                    "<when condition=\"when\"/>"
                    "<if-feature name=\"iff\"/>"
                    "<must condition=\"must-cond\"/>"
                    "<key value=\"key\"/>"
                    "<unique tag=\"utag\"/>"
                    "<config value=\"true\"/>"
                    "<min-elements value=\"10\"/>"
                    "<ordered-by value=\"user\"/>"
                    "<status value=\"deprecated\"/>"
                    "<description><text>desc</text></description>"
                    "<reference><text>ref</text></reference>"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<container name=\"cont\"/>"
                    "<choice name=\"choice\"/>"
                    "<action name=\"action\"/>"
                    "<grouping name=\"grp\"/>"
                    "<notification name=\"notf\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"sub-list\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    "<uses name=\"uses-name\"/>"
                    EXT_SUBELEM
                "</list>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_list *)&siblings[0];
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "cont");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(parsed->child->next->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "sub-list");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    assert_string_equal(parsed->groupings->name, "grp");
    assert_string_equal(parsed->actions->name, "action");
    assert_int_equal(parsed->groupings->nodetype, LYS_GROUPING);
    assert_string_equal(parsed->notifs->name, "notf");
    assert_true(parsed->flags & LYS_ORDBY_USER);
    assert_true(parsed->flags & LYS_STATUS_DEPRC);
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_string_equal(*parsed->iffeatures, "iff");
    assert_string_equal(parsed->key, "key");
    assert_int_equal(parsed->min, 10);
    assert_string_equal(parsed->musts->arg, "must-cond");
    assert_string_equal(parsed->name, "list-name");
    assert_null(parsed->next);
    assert_int_equal(parsed->nodetype, LYS_LIST);
    assert_null(parsed->parent);
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->typedefs->name, "tpdf");
    assert_string_equal(*parsed->uniques, "utag");
    assert_string_equal(parsed->when->cond, "when");
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    ly_set_erase(&st->yin_ctx->tpdfs_nodes, NULL);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<list name=\"list-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_list *)&siblings[0];
    assert_string_equal(parsed->name, "list-name");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    st->finished_correctly = true;
}

static void
test_notification_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_notif *notifs = NULL;
    struct notif_meta notif_meta = {NULL, &notifs};

    /* max subelems */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<notification name=\"notif-name\">"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"iff\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"sub-list\"/>"
                    "<must condition=\"cond\"/>"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"deprecated\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    "<uses name=\"uses-name\"/>"
                    "<container name=\"cont\"/>"
                    "<choice name=\"choice\"/>"
                    "<grouping name=\"grp\"/>"
                    EXT_SUBELEM
                "</notification>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &notif_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(notifs->name, "notif-name");
    assert_string_equal(notifs->data->name, "anyd");
    assert_int_equal(notifs->data->nodetype, LYS_ANYDATA);
    assert_string_equal(notifs->data->next->name, "anyx");
    assert_int_equal(notifs->data->next->nodetype, LYS_ANYXML);
    assert_string_equal(notifs->data->next->next->name, "leaf");
    assert_int_equal(notifs->data->next->next->nodetype, LYS_LEAF);
    assert_string_equal(notifs->data->next->next->next->name, "llist");
    assert_int_equal(notifs->data->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(notifs->data->next->next->next->next->name, "sub-list");
    assert_int_equal(notifs->data->next->next->next->next->nodetype, LYS_LIST);
    assert_true(notifs->flags & LYS_STATUS_DEPRC);
    assert_string_equal(notifs->groupings->name, "grp");
    assert_int_equal(notifs->groupings->nodetype, LYS_GROUPING);
    assert_string_equal(notifs->data->next->next->next->next->next->name, "uses-name");
    assert_int_equal(notifs->data->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(notifs->data->next->next->next->next->next->next->name, "cont");
    assert_int_equal(notifs->data->next->next->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_int_equal(notifs->data->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(notifs->data->next->next->next->next->next->next->next->name, "choice");
    assert_null(notifs->data->next->next->next->next->next->next->next->next);
    assert_string_equal(*notifs->iffeatures, "iff");
    assert_string_equal(notifs->musts->arg, "cond");
    assert_int_equal(notifs->nodetype, LYS_NOTIF);
    assert_null(notifs->parent);
    assert_string_equal(notifs->ref, "ref");
    assert_string_equal(notifs->typedefs->name, "tpdf");
    assert_string_equal(notifs->exts[0].name, "myext:c-define");
    assert_int_equal(notifs->exts[0].insubstmt_index, 0);
    assert_int_equal(notifs->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, notifs, lysp_notif_free);
    notifs = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<notification name=\"notif-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &notif_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(notifs->name, "notif-name");
    FREE_ARRAY(st->ctx, notifs, lysp_notif_free);
    notifs = NULL;

    st->finished_correctly = true;
}

static void
test_grouping_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_grp *grps = NULL;
    struct grouping_meta grp_meta = {NULL, &grps};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<grouping name=\"grp-name\">"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<description><text>desc</text></description>"
                    "<grouping name=\"sub-grp\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"list\"/>"
                    "<notification name=\"notf\"/>"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    "<uses name=\"uses-name\"/>"
                    "<action name=\"act\"/>"
                    "<container name=\"cont\"/>"
                    "<choice name=\"choice\"/>"
                    EXT_SUBELEM
                "</grouping>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &grp_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(grps->name, "grp-name");
    assert_string_equal(grps->data->name, "anyd");
    assert_string_equal(grps->data->next->name, "anyx");
    assert_string_equal(grps->data->next->next->name, "leaf");
    assert_string_equal(grps->data->next->next->next->name, "llist");
    assert_string_equal(grps->data->next->next->next->next->name, "list");
    assert_string_equal(grps->dsc, "desc");
    assert_true(grps->flags & LYS_STATUS_CURR);
    assert_string_equal(grps->groupings->name, "sub-grp");
    assert_int_equal(grps->nodetype, LYS_GROUPING);
    assert_string_equal(grps->notifs->name, "notf");
    assert_null(grps->parent);
    assert_string_equal(grps->ref, "ref");
    assert_string_equal(grps->typedefs->name, "tpdf");
    assert_string_equal(grps->actions->name, "act");
    assert_string_equal(grps->data->next->next->next->next->next->name, "uses-name");
    assert_int_equal(grps->data->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(grps->data->next->next->next->next->next->next->name, "cont");
    assert_int_equal(grps->data->next->next->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(grps->data->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(grps->data->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(grps->exts[0].name, "myext:c-define");
    assert_int_equal(grps->exts[0].insubstmt_index, 0);
    assert_int_equal(grps->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, grps, lysp_grp_free);
    grps = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<grouping name=\"grp-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &grp_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(grps->name, "grp-name");
    FREE_ARRAY(st->ctx, grps, lysp_grp_free);
    grps = NULL;

    st->finished_correctly = true;
}

static void
test_container_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_container *parsed = NULL;

    /* max subelems */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<container name=\"cont-name\">"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<config value=\"true\"/>"
                    "<container name=\"subcont\"/>"
                    "<description><text>desc</text></description>"
                    "<grouping name=\"sub-grp\"/>"
                    "<if-feature name=\"iff\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"list\"/>"
                    "<must condition=\"cond\"/>"
                    "<notification name=\"notf\"/>"
                    "<presence value=\"presence\"/>"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    "<uses name=\"uses-name\"/>"
                    "<when condition=\"when-cond\"/>"
                    "<action name=\"act\"/>"
                    "<choice name=\"choice\"/>"
                    EXT_SUBELEM
                "</container>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_container *)siblings;
    assert_string_equal(parsed->name, "cont-name");
    assert_null(parsed->parent);
    assert_int_equal(parsed->nodetype, LYS_CONTAINER);
    assert_true(parsed->flags & LYS_CONFIG_W);
    assert_true(parsed->flags & LYS_STATUS_CURR);
    assert_null(parsed->next);
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_string_equal(*parsed->iffeatures, "iff");
    assert_string_equal(parsed->musts->arg, "cond");
    assert_string_equal(parsed->presence, "presence");
    assert_string_equal(parsed->typedefs->name, "tpdf");
    assert_string_equal(parsed->groupings->name, "sub-grp");
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "subcont");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "list");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    assert_string_equal(parsed->notifs->name, "notf");
    assert_string_equal(parsed->actions->name, "act");
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    ly_set_erase(&st->yin_ctx->tpdfs_nodes, NULL);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<container name=\"cont-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_container *)siblings;
    assert_string_equal(parsed->name, "cont-name");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    st->finished_correctly = true;
}

static void
test_case_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_case *parsed = NULL;

    /* max subelems */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<case name=\"case-name\">"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<container name=\"subcont\"/>"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"iff\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"list\"/>"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<uses name=\"uses-name\"/>"
                    "<when condition=\"when-cond\"/>"
                    "<choice name=\"choice\"/>"
                    EXT_SUBELEM
                "</case>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_case *)siblings;
    assert_string_equal(parsed->name, "case-name");
    assert_null(parsed->parent);
    assert_int_equal(parsed->nodetype, LYS_CASE);
    assert_true(parsed->flags & LYS_STATUS_CURR);
    assert_null(parsed->next);
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_string_equal(*parsed->iffeatures, "iff");
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "subcont");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "list");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<case name=\"case-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_case *)siblings;
    assert_string_equal(parsed->name, "case-name");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    st->finished_correctly = true;
}

static void
test_choice_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_choice *parsed = NULL;

    /* max subelems */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<choice name=\"choice-name\">"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<case name=\"sub-case\"/>"
                    "<choice name=\"choice\"/>"
                    "<config value=\"true\"/>"
                    "<container name=\"subcont\"/>"
                    "<default value=\"def\"/>"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"iff\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"list\"/>"
                    "<mandatory value=\"true\" />"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<when condition=\"when-cond\"/>"
                    EXT_SUBELEM
                "</choice>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_choice *)siblings;
    assert_string_equal(parsed->name, "choice-name");
    assert_null(parsed->parent);
    assert_int_equal(parsed->nodetype, LYS_CHOICE);
    assert_true(parsed->flags & LYS_CONFIG_W && parsed->flags & LYS_MAND_TRUE  && parsed->flags & LYS_STATUS_CURR);
    assert_null(parsed->next);
    assert_string_equal(parsed->dsc, "desc");
    assert_string_equal(parsed->ref, "ref");
    assert_string_equal(parsed->when->cond, "when-cond");
    assert_string_equal(*parsed->iffeatures, "iff");
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "sub-case");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CASE);
    assert_string_equal(parsed->child->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(parsed->child->next->next->next->next->name, "subcont");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "list");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    assert_string_equal(parsed->exts[0].name, "myext:c-define");
    assert_int_equal(parsed->exts[0].insubstmt_index, 0);
    assert_int_equal(parsed->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<choice name=\"choice-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &node_meta, NULL, NULL, true), LY_SUCCESS);
    parsed = (struct lysp_node_choice *)siblings;
    assert_string_equal(parsed->name, "choice-name");
    lysp_node_free(st->ctx, siblings);
    siblings = NULL;

    st->finished_correctly = true;
}

static void
test_inout_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_action_inout inout = {};
    struct inout_meta inout_meta = {NULL, &inout};

    /* max subelements */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<input>"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<choice name=\"choice\"/>"
                    "<container name=\"subcont\"/>"
                    "<grouping name=\"sub-grp\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"list\"/>"
                    "<must condition=\"cond\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    "<uses name=\"uses-name\"/>"
                    EXT_SUBELEM
                "</input>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inout_meta, NULL, NULL, true), LY_SUCCESS);
    assert_null(inout.parent);
    assert_int_equal(inout.nodetype, LYS_INPUT);
    assert_string_equal(inout.musts->arg, "cond");
    assert_string_equal(inout.typedefs->name, "tpdf");
    assert_string_equal(inout.groupings->name, "sub-grp");
    assert_string_equal(inout.data->name, "anyd");
    assert_int_equal(inout.data->nodetype, LYS_ANYDATA);
    assert_string_equal(inout.data->next->name, "anyx");
    assert_int_equal(inout.data->next->nodetype, LYS_ANYXML);
    assert_string_equal(inout.data->next->next->name, "choice");
    assert_int_equal(inout.data->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(inout.data->next->next->next->name, "subcont");
    assert_int_equal(inout.data->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(inout.data->next->next->next->next->name, "leaf");
    assert_int_equal(inout.data->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(inout.data->next->next->next->next->next->name, "llist");
    assert_int_equal(inout.data->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(inout.data->next->next->next->next->next->next->name, "list");
    assert_int_equal(inout.data->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(inout.data->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(inout.data->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(inout.data->next->next->next->next->next->next->next->next);
    assert_string_equal(inout.exts[0].name, "myext:c-define");
    assert_int_equal(inout.exts[0].insubstmt_index, 0);
    assert_int_equal(inout.exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_action_inout_free(st->ctx, &inout);
    memset(&inout, 0, sizeof inout);

    /* max subelements */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<output>"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<choice name=\"choice\"/>"
                    "<container name=\"subcont\"/>"
                    "<grouping name=\"sub-grp\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"list\"/>"
                    "<must condition=\"cond\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    "<uses name=\"uses-name\"/>"
                    EXT_SUBELEM
                "</output>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inout_meta, NULL, NULL, true), LY_SUCCESS);
    assert_null(inout.parent);
    assert_int_equal(inout.nodetype, LYS_OUTPUT);
    assert_string_equal(inout.musts->arg, "cond");
    assert_string_equal(inout.typedefs->name, "tpdf");
    assert_string_equal(inout.groupings->name, "sub-grp");
    assert_string_equal(inout.data->name, "anyd");
    assert_int_equal(inout.data->nodetype, LYS_ANYDATA);
    assert_string_equal(inout.data->next->name, "anyx");
    assert_int_equal(inout.data->next->nodetype, LYS_ANYXML);
    assert_string_equal(inout.data->next->next->name, "choice");
    assert_int_equal(inout.data->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(inout.data->next->next->next->name, "subcont");
    assert_int_equal(inout.data->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(inout.data->next->next->next->next->name, "leaf");
    assert_int_equal(inout.data->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(inout.data->next->next->next->next->next->name, "llist");
    assert_int_equal(inout.data->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(inout.data->next->next->next->next->next->next->name, "list");
    assert_int_equal(inout.data->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(inout.data->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(inout.data->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(inout.data->next->next->next->next->next->next->next->next);
    assert_string_equal(inout.exts[0].name, "myext:c-define");
    assert_int_equal(inout.exts[0].insubstmt_index, 0);
    assert_int_equal(inout.exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_action_inout_free(st->ctx, &inout);
    memset(&inout, 0, sizeof inout);

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<input />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inout_meta, NULL, NULL, true), LY_SUCCESS);
    memset(&inout, 0, sizeof inout);

    data = ELEMENT_WRAPPER_START "<output />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inout_meta, NULL, NULL, true), LY_SUCCESS);
    memset(&inout, 0, sizeof inout);

    /* invalid combinations */
    data = ELEMENT_WRAPPER_START "<input name=\"test\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &inout_meta, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Unexpected attribute \"name\" of input element. Line number 1.");
    memset(&inout, 0, sizeof inout);

    st->finished_correctly = true;
}

static void
test_action_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_action *actions = NULL;
    struct action_meta act_meta = {NULL, &actions};

    /* max subelems */
    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<action name=\"act\">"
                    "<description><text>desc</text></description>"
                    "<grouping name=\"grouping\"/>"
                    "<if-feature name=\"iff\"/>"
                    "<input><uses name=\"uses-name\"/></input>"
                    "<output><must condition=\"cond\"/></output>"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"deprecated\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    EXT_SUBELEM
                "</action>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &act_meta, NULL, NULL, true), LY_SUCCESS);
    assert_null(actions->parent);
    assert_int_equal(actions->nodetype, LYS_ACTION);
    assert_true(actions->flags & LYS_STATUS_DEPRC);
    assert_string_equal(actions->name, "act");
    assert_string_equal(actions->dsc, "desc");
    assert_string_equal(actions->ref, "ref");
    assert_string_equal(*actions->iffeatures, "iff");
    assert_string_equal(actions->typedefs->name, "tpdf");
    assert_string_equal(actions->groupings->name, "grouping");
    assert_string_equal(actions->input.data->name, "uses-name");
    assert_string_equal(actions->output.musts->arg, "cond");
    assert_string_equal(actions->exts[0].name, "myext:c-define");
    assert_int_equal(actions->exts[0].insubstmt_index, 0);
    assert_int_equal(actions->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, actions, lysp_action_free)
    actions = NULL;

    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<rpc name=\"act\">"
                    "<description><text>desc</text></description>"
                    "<grouping name=\"grouping\"/>"
                    "<if-feature name=\"iff\"/>"
                    "<input><uses name=\"uses-name\"/></input>"
                    "<output><must condition=\"cond\"/></output>"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"deprecated\"/>"
                    "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>"
                    EXT_SUBELEM
                "</rpc>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &act_meta, NULL, NULL, true), LY_SUCCESS);
    assert_null(actions->parent);
    assert_int_equal(actions->nodetype, LYS_ACTION);
    assert_true(actions->flags & LYS_STATUS_DEPRC);
    assert_string_equal(actions->name, "act");
    assert_string_equal(actions->dsc, "desc");
    assert_string_equal(actions->ref, "ref");
    assert_string_equal(*actions->iffeatures, "iff");
    assert_string_equal(actions->typedefs->name, "tpdf");
    assert_string_equal(actions->groupings->name, "grouping");
    assert_string_equal(actions->input.data->name, "uses-name");
    assert_string_equal(actions->output.musts->arg, "cond");
    assert_string_equal(actions->exts[0].name, "myext:c-define");
    assert_int_equal(actions->exts[0].insubstmt_index, 0);
    assert_int_equal(actions->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, actions, lysp_action_free)
    actions = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<action name=\"act\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &act_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(actions->name, "act");
    FREE_ARRAY(st->ctx, actions, lysp_action_free)
    actions = NULL;

    st->finished_correctly = true;
}

static void
test_augment_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_augment *augments = NULL;
    struct augment_meta aug_meta = {NULL, &augments};

    st->yin_ctx->mod_version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
                "<augment target-node=\"target\">"
                    "<action name=\"action\"/>"
                    "<anydata name=\"anyd\"/>"
                    "<anyxml name=\"anyx\"/>"
                    "<case name=\"case\"/>"
                    "<choice name=\"choice\"/>"
                    "<container name=\"subcont\"/>"
                    "<description><text>desc</text></description>"
                    "<if-feature name=\"iff\"/>"
                    "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>"
                    "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>"
                    "<list name=\"list\"/>"
                    "<notification name=\"notif\"/>"
                    "<reference><text>ref</text></reference>"
                    "<status value=\"current\"/>"
                    "<uses name=\"uses\"/>"
                    "<when condition=\"when-cond\"/>"
                    EXT_SUBELEM
                "</augment>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &aug_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(augments->nodeid, "target");
    assert_null(augments->parent);
    assert_int_equal(augments->nodetype, LYS_AUGMENT);
    assert_true(augments->flags & LYS_STATUS_CURR);
    assert_string_equal(augments->dsc, "desc");
    assert_string_equal(augments->ref, "ref");
    assert_string_equal(augments->when->cond, "when-cond");
    assert_string_equal(*augments->iffeatures, "iff");
    assert_string_equal(augments->child->name, "anyd");
    assert_int_equal(augments->child->nodetype, LYS_ANYDATA);
    assert_string_equal(augments->child->next->name, "anyx");
    assert_int_equal(augments->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(augments->child->next->next->name, "case");
    assert_int_equal(augments->child->next->next->nodetype, LYS_CASE);
    assert_string_equal(augments->child->next->next->next->name, "choice");
    assert_int_equal(augments->child->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(augments->child->next->next->next->next->name, "subcont");
    assert_int_equal(augments->child->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(augments->child->next->next->next->next->next->name, "leaf");
    assert_int_equal(augments->child->next->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(augments->child->next->next->next->next->next->next->name, "llist");
    assert_int_equal(augments->child->next->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(augments->child->next->next->next->next->next->next->next->name, "list");
    assert_int_equal(augments->child->next->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(augments->child->next->next->next->next->next->next->next->next->name, "uses");
    assert_int_equal(augments->child->next->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(augments->child->next->next->next->next->next->next->next->next->next);
    assert_string_equal(augments->actions->name, "action");
    assert_string_equal(augments->notifs->name, "notif");
    assert_string_equal(augments->exts[0].name, "myext:c-define");
    assert_int_equal(augments->exts[0].insubstmt_index, 0);
    assert_int_equal(augments->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, augments, lysp_augment_free)
    augments = NULL;

    data = ELEMENT_WRAPPER_START "<augment target-node=\"target\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &aug_meta, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(augments->nodeid, "target");
    FREE_ARRAY(st->ctx, augments, lysp_augment_free)
    augments = NULL;

    st->finished_correctly = true;
}

static void
test_deviate_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_deviate *deviates = NULL;
    struct lysp_deviate_add *d_add;
    struct lysp_deviate_rpl *d_rpl;
    struct lysp_deviate_del *d_del;

    /* all valid arguments with min subelems */
    data = ELEMENT_WRAPPER_START "<deviate value=\"not-supported\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_NOT_SUPPORTED);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"add\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_ADD);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"replace\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_REPLACE);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"delete\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_DELETE);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    /* max subelems and valid arguments */
    data = ELEMENT_WRAPPER_START
                "<deviate value=\"not-supported\">"
                    EXT_SUBELEM
                "</deviate>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_NOT_SUPPORTED);
    assert_string_equal(deviates->exts[0].name, "myext:c-define");
    assert_int_equal(deviates->exts[0].insubstmt_index, 0);
    assert_int_equal(deviates->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
                "<deviate value=\"add\">"
                    "<units name=\"units\"/>"
                    "<must condition=\"cond\"/>"
                    "<unique tag=\"utag\"/>"
                    "<default value=\"def\"/>"
                    "<config value=\"true\"/>"
                    "<mandatory value=\"true\"/>"
                    "<min-elements value=\"5\"/>"
                    "<max-elements value=\"15\"/>"
                    EXT_SUBELEM
                "</deviate>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    d_add = (struct lysp_deviate_add *)deviates;
    assert_int_equal(d_add->mod, LYS_DEV_ADD);
    assert_null(d_add->next);
    assert_string_equal(d_add->units, "units");
    assert_string_equal(d_add->musts->arg, "cond");
    assert_string_equal(*d_add->uniques, "utag");
    assert_string_equal(*d_add->dflts, "def");
    assert_true(d_add->flags & LYS_MAND_TRUE && d_add->flags & LYS_CONFIG_W);
    assert_int_equal(d_add->min, 5);
    assert_int_equal(d_add->max, 15);
    assert_string_equal(deviates->exts[0].name, "myext:c-define");
    assert_int_equal(deviates->exts[0].insubstmt_index, 0);
    assert_int_equal(deviates->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
                "<deviate value=\"replace\">"
                    "<type name=\"newtype\"/>"
                    "<units name=\"uni\"/>"
                    "<default value=\"def\"/>"
                    "<config value=\"true\"/>"
                    "<mandatory value=\"true\"/>"
                    "<min-elements value=\"5\"/>"
                    "<max-elements value=\"15\"/>"
                    EXT_SUBELEM
                "</deviate>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    d_rpl = (struct lysp_deviate_rpl *)deviates;
    assert_int_equal(d_rpl->mod, LYS_DEV_REPLACE);
    assert_null(d_rpl->next);
    assert_string_equal(d_rpl->type->name, "newtype");
    assert_string_equal(d_rpl->units, "uni");
    assert_string_equal(d_rpl->dflt, "def");
    assert_true(d_rpl->flags & LYS_MAND_TRUE && d_rpl->flags & LYS_CONFIG_W);
    assert_int_equal(d_rpl->min, 5);
    assert_int_equal(d_rpl->max, 15);
    assert_string_equal(deviates->exts[0].name, "myext:c-define");
    assert_int_equal(deviates->exts[0].insubstmt_index, 0);
    assert_int_equal(deviates->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
                "<deviate value=\"delete\">"
                    "<units name=\"u\"/>"
                    "<must condition=\"c\"/>"
                    "<unique tag=\"tag\"/>"
                    "<default value=\"default\"/>"
                    EXT_SUBELEM
                "</deviate>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, true), LY_SUCCESS);
    d_del = (struct lysp_deviate_del *)deviates;
    assert_int_equal(d_del->mod, LYS_DEV_DELETE);
    assert_null(d_del->next);
    assert_string_equal(d_del->units, "u");
    assert_string_equal(d_del->musts->arg, "c");
    assert_string_equal(*d_del->uniques, "tag");
    assert_string_equal(*d_del->dflts, "default");
    assert_string_equal(deviates->exts[0].name, "myext:c-define");
    assert_int_equal(deviates->exts[0].insubstmt_index, 0);
    assert_int_equal(deviates->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_deviate_free(st->ctx, deviates);
    free(deviates);
    deviates = NULL;

    /* invalid arguments */
    data = ELEMENT_WRAPPER_START "<deviate value=\"\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"\" of \"deviate\". Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"invalid\" of \"deviate\". Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"ad\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"ad\" of \"deviate\". Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"adds\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Invalid value \"adds\" of \"deviate\". Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
                "<deviate value=\"not-supported\">"
                    "<must condition=\"c\"/>"
                "</deviate>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviates, NULL, NULL, false), LY_EVALID);
    logbuf_assert("Deviate of this type doesn't allow \"must\" as it's sub-element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_deviation_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lysp_deviation *deviations = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START
                "<deviation target-node=\"target\">"
                    "<deviate value=\"not-supported\"/>"
                "</deviation>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviations, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(deviations->nodeid, "target");
    assert_int_equal(deviations->deviates->mod, LYS_DEV_NOT_SUPPORTED);
    FREE_ARRAY(st->ctx, deviations, lysp_deviation_free);
    deviations = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
                "<deviation target-node=\"target\">"
                    "<reference><text>ref</text></reference>"
                    "<description><text>desc</text></description>"
                    "<deviate value=\"add\"/>"
                    EXT_SUBELEM
                "</deviation>"
           ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviations, NULL, NULL, true), LY_SUCCESS);
    assert_string_equal(deviations->nodeid, "target");
    assert_int_equal(deviations->deviates->mod, LYS_DEV_ADD);
    assert_string_equal(deviations->ref, "ref");
    assert_string_equal(deviations->dsc, "desc");
    assert_string_equal(deviations->exts[0].name, "myext:c-define");
    assert_int_equal(deviations->exts[0].insubstmt_index, 0);
    assert_int_equal(deviations->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    FREE_ARRAY(st->ctx, deviations, lysp_deviation_free);
    deviations = NULL;

    /* invalid */
    data = ELEMENT_WRAPPER_START "<deviation target-node=\"target\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(st, &data, &deviations, NULL, NULL, false), LY_EVALID);
    FREE_ARRAY(st->ctx, deviations, lysp_deviation_free);
    deviations = NULL;
    logbuf_assert("Missing mandatory subelement deviate of deviate element. Line number 1.");

    st->finished_correctly = true;
}

static void
test_module_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct yin_arg_record *attrs = NULL;
    struct sized_string name, prefix;
    struct lys_module *lys_mod = NULL;
    struct lysp_module *lysp_mod = NULL;

    /* max subelems */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    lys_mod = calloc(1, sizeof *lys_mod);
    lysp_mod = calloc(1, sizeof *lysp_mod);
    lys_mod->ctx = st->ctx;
    lysp_mod->mod = lys_mod;
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">\n"
                "<yang-version value=\"1.1\"/>\n"
                "<namespace uri=\"ns\"/>\n"
                "<prefix value=\"pref\"/>\n"
                "<include module=\"b-mod\"/>\n"
                "<import module=\"a-mod\"><prefix value=\"imp-pref\"/></import>\n"
                "<organization><text>org</text></organization>\n"
                "<contact><text>contact</text></contact>\n"
                "<description><text>desc</text></description>"
                "<reference><text>ref</text></reference>\n"
                "<revision date=\"2019-02-02\"/>\n"
                "<anydata name=\"anyd\"/>\n"
                "<anyxml name=\"anyx\"/>\n"
                "<choice name=\"choice\"/>\n"
                "<container name=\"cont\"/>\n"
                "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
                "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
                "<list name=\"sub-list\"/>\n"
                "<uses name=\"uses-name\"/>\n"
                "<augment target-node=\"target\"/>\n"
                "<deviation target-node=\"target\">""<deviate value=\"not-supported\"/>""</deviation>\n"
                "<extension name=\"ext\"/>\n"
                "<feature name=\"feature\"/>\n"
                "<grouping name=\"grp\"/>\n"
                "<identity name=\"ident-name\"/>\n"
                "<notification name=\"notf\"/>\n"
                "<rpc name=\"rpc-name\"/>\n"
                "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
                EXT_SUBELEM"\n"
           "</module>\n";
    assert_int_equal(lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len), LY_SUCCESS);
    assert_int_equal(yin_load_attributes(st->yin_ctx, &data, &attrs), LY_SUCCESS);
    assert_int_equal(yin_parse_mod(st->yin_ctx, attrs, &data, lysp_mod), LY_SUCCESS);
    assert_string_equal(lysp_mod->mod->name, "mod");
    assert_string_equal(lysp_mod->revs, "2019-02-02");
    assert_string_equal(lysp_mod->mod->ns, "ns");
    assert_string_equal(lysp_mod->mod->prefix, "pref");
    assert_null(lysp_mod->mod->filepath);
    assert_string_equal(lysp_mod->mod->org, "org");
    assert_string_equal(lysp_mod->mod->contact, "contact");
    assert_string_equal(lysp_mod->mod->dsc, "desc");
    assert_string_equal(lysp_mod->mod->ref, "ref");
    assert_int_equal(lysp_mod->mod->version, LYS_VERSION_1_1);
    assert_string_equal(lysp_mod->imports->name, "a-mod");
    assert_string_equal(lysp_mod->includes->name, "b-mod");
    assert_string_equal(lysp_mod->extensions->name, "ext");
    assert_string_equal(lysp_mod->features->name, "feature");
    assert_string_equal(lysp_mod->identities->name, "ident-name");
    assert_string_equal(lysp_mod->typedefs->name, "tpdf");
    assert_string_equal(lysp_mod->groupings->name, "grp");
    assert_string_equal(lysp_mod->data->name, "anyd");
    assert_int_equal(lysp_mod->data->nodetype, LYS_ANYDATA);
    assert_string_equal(lysp_mod->data->next->name, "anyx");
    assert_int_equal(lysp_mod->data->next->nodetype, LYS_ANYXML);
    assert_string_equal(lysp_mod->data->next->next->name, "choice");
    assert_int_equal(lysp_mod->data->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(lysp_mod->data->next->next->next->name, "cont");
    assert_int_equal(lysp_mod->data->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(lysp_mod->data->next->next->next->next->name, "leaf");
    assert_int_equal(lysp_mod->data->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(lysp_mod->data->next->next->next->next->next->name, "llist");
    assert_int_equal(lysp_mod->data->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(lysp_mod->data->next->next->next->next->next->next->name, "sub-list");
    assert_int_equal(lysp_mod->data->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(lysp_mod->data->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(lysp_mod->data->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(lysp_mod->data->next->next->next->next->next->next->next->next);
    assert_string_equal(lysp_mod->augments->nodeid, "target");
    assert_string_equal(lysp_mod->rpcs->name, "rpc-name");
    assert_string_equal(lysp_mod->notifs->name, "notf");
    assert_string_equal(lysp_mod->deviations->nodeid, "target");
    assert_string_equal(lysp_mod->exts[0].name, "myext:c-define");
    assert_int_equal(lysp_mod->exts[0].insubstmt_index, 0);
    assert_int_equal(lysp_mod->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);
    lysp_module_free(lysp_mod);
    lys_module_free(lys_mod, NULL);
    FREE_ARRAY(st->yin_ctx, attrs, free_arg_rec);
    attrs = NULL;

    /* min subelems */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    lys_mod = calloc(1, sizeof *lys_mod);
    lysp_mod = calloc(1, sizeof *lysp_mod);
    lys_mod->ctx = st->ctx;
    lysp_mod->mod = lys_mod;
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">"
                "<namespace uri=\"ns\"/>"
                "<prefix value=\"pref\"/>"
                "<yang-version value=\"1.1\"/>"
           "</module>";
    assert_int_equal(lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len), LY_SUCCESS);
    assert_int_equal(yin_load_attributes(st->yin_ctx, &data, &attrs), LY_SUCCESS);
    assert_int_equal(yin_parse_mod(st->yin_ctx, attrs, &data, lysp_mod), LY_SUCCESS);
    assert_string_equal(lysp_mod->mod->name, "mod");
    lysp_module_free(lysp_mod);
    lys_module_free(lys_mod, NULL);
    FREE_ARRAY(st->yin_ctx, attrs, free_arg_rec);
    attrs = NULL;

    /* incorrect subelem order */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    lys_mod = calloc(1, sizeof *lys_mod);
    lysp_mod = calloc(1, sizeof *lysp_mod);
    lys_mod->ctx = st->ctx;
    lysp_mod->mod = lys_mod;
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">"
                "<feature name=\"feature\"/>\n"
                "<namespace uri=\"ns\"/>"
                "<prefix value=\"pref\"/>"
                "<yang-version value=\"1.1\"/>"
           "</module>";
    assert_int_equal(lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len), LY_SUCCESS);
    assert_int_equal(yin_load_attributes(st->yin_ctx, &data, &attrs), LY_SUCCESS);
    assert_int_equal(yin_parse_mod(st->yin_ctx, attrs, &data, lysp_mod), LY_EVALID);
    logbuf_assert("Invalid order of module\'s subelements \"namespace\" can\'t appear after \"feature\". Line number 30.");
    lysp_module_free(lysp_mod);
    lys_module_free(lys_mod, NULL);
    FREE_ARRAY(st->yin_ctx, attrs, free_arg_rec);
    attrs = NULL;

    st->finished_correctly = true;
}

static void
test_submodule_elem(void **state)
{
    struct state *st = *state;
    const char *data;
    struct yin_arg_record *attrs = NULL;
    struct sized_string name, prefix;
    struct lysp_submodule *lysp_submod = NULL;

    /* max subelements */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    lysp_submod = calloc(1, sizeof *lysp_submod);
    data = "<submodule xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">\n"
                "<yang-version value=\"1.1\"/>\n"
                "<belongs-to module=\"mod-name\"><prefix value=\"pref\"/></belongs-to>"
                "<include module=\"b-mod\"/>\n"
                "<import module=\"a-mod\"><prefix value=\"imp-pref\"/></import>\n"
                "<organization><text>org</text></organization>\n"
                "<contact><text>contact</text></contact>\n"
                "<description><text>desc</text></description>"
                "<reference><text>ref</text></reference>\n"
                "<revision date=\"2019-02-02\"/>\n"
                "<anydata name=\"anyd\"/>\n"
                "<anyxml name=\"anyx\"/>\n"
                "<choice name=\"choice\"/>\n"
                "<container name=\"cont\"/>\n"
                "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
                "<leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
                "<list name=\"sub-list\"/>\n"
                "<uses name=\"uses-name\"/>\n"
                "<augment target-node=\"target\"/>\n"
                "<deviation target-node=\"target\">""<deviate value=\"not-supported\"/>""</deviation>\n"
                "<extension name=\"ext\"/>\n"
                "<feature name=\"feature\"/>\n"
                "<grouping name=\"grp\"/>\n"
                "<identity name=\"ident-name\"/>\n"
                "<notification name=\"notf\"/>\n"
                "<rpc name=\"rpc-name\"/>\n"
                "<typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
                EXT_SUBELEM"\n"
           "</submodule>\n";
    assert_int_equal(lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len), LY_SUCCESS);
    assert_int_equal(yin_load_attributes(st->yin_ctx, &data, &attrs), LY_SUCCESS);
    assert_int_equal(yin_parse_submod(st->yin_ctx, attrs, &data, lysp_submod), LY_SUCCESS);

    assert_string_equal(lysp_submod->name, "mod");
    assert_string_equal(lysp_submod->revs, "2019-02-02");
    assert_string_equal(lysp_submod->prefix, "pref");
    assert_null(lysp_submod->filepath);
    assert_string_equal(lysp_submod->org, "org");
    assert_string_equal(lysp_submod->contact, "contact");
    assert_string_equal(lysp_submod->dsc, "desc");
    assert_string_equal(lysp_submod->ref, "ref");
    assert_int_equal(lysp_submod->version, LYS_VERSION_1_1);
    assert_string_equal(lysp_submod->imports->name, "a-mod");
    assert_string_equal(lysp_submod->includes->name, "b-mod");
    assert_string_equal(lysp_submod->extensions->name, "ext");
    assert_string_equal(lysp_submod->features->name, "feature");
    assert_string_equal(lysp_submod->identities->name, "ident-name");
    assert_string_equal(lysp_submod->typedefs->name, "tpdf");
    assert_string_equal(lysp_submod->groupings->name, "grp");
    assert_string_equal(lysp_submod->data->name, "anyd");
    assert_int_equal(lysp_submod->data->nodetype, LYS_ANYDATA);
    assert_string_equal(lysp_submod->data->next->name, "anyx");
    assert_int_equal(lysp_submod->data->next->nodetype, LYS_ANYXML);
    assert_string_equal(lysp_submod->data->next->next->name, "choice");
    assert_int_equal(lysp_submod->data->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(lysp_submod->data->next->next->next->name, "cont");
    assert_int_equal(lysp_submod->data->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(lysp_submod->data->next->next->next->next->name, "leaf");
    assert_int_equal(lysp_submod->data->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(lysp_submod->data->next->next->next->next->next->name, "llist");
    assert_int_equal(lysp_submod->data->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(lysp_submod->data->next->next->next->next->next->next->name, "sub-list");
    assert_int_equal(lysp_submod->data->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(lysp_submod->data->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(lysp_submod->data->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(lysp_submod->data->next->next->next->next->next->next->next->next);
    assert_string_equal(lysp_submod->augments->nodeid, "target");
    assert_string_equal(lysp_submod->rpcs->name, "rpc-name");
    assert_string_equal(lysp_submod->notifs->name, "notf");
    assert_string_equal(lysp_submod->deviations->nodeid, "target");
    assert_string_equal(lysp_submod->exts[0].name, "myext:c-define");
    assert_int_equal(lysp_submod->exts[0].insubstmt_index, 0);
    assert_int_equal(lysp_submod->exts[0].insubstmt, LYEXT_SUBSTMT_SELF);

    lysp_submodule_free(st->ctx, lysp_submod);
    FREE_ARRAY(st->yin_ctx, attrs, free_arg_rec);
    attrs = NULL;

    /* min subelemnts */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    lysp_submod = calloc(1, sizeof *lysp_submod);
    data = "<submodule xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"submod\">"
                "<yang-version value=\"1.0\"/>"
                "<belongs-to module=\"mod-name\"><prefix value=\"pref\"/></belongs-to>"
           "</submodule>";
    assert_int_equal(lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len), LY_SUCCESS);
    assert_int_equal(yin_load_attributes(st->yin_ctx, &data, &attrs), LY_SUCCESS);
    assert_int_equal(yin_parse_submod(st->yin_ctx, attrs, &data, lysp_submod), LY_SUCCESS);
    assert_string_equal(lysp_submod->prefix, "pref");
    assert_string_equal(lysp_submod->belongsto, "mod-name");
    assert_int_equal(lysp_submod->version, LYS_VERSION_1_0);
    lysp_submodule_free(st->ctx, lysp_submod);
    FREE_ARRAY(st->yin_ctx, attrs, free_arg_rec);
    attrs = NULL;

    /* incorrect subelem order */
    st->yin_ctx->xml_ctx.status = LYXML_ELEMENT;
    lysp_submod = calloc(1, sizeof *lysp_submod);
    data = "<submodule xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"submod\">"
                "<yang-version value=\"1.0\"/>"
                "<reference><text>ref</text></reference>\n"
                "<belongs-to module=\"mod-name\"><prefix value=\"pref\"/></belongs-to>"
           "</submodule>";
    assert_int_equal(lyxml_get_element(&st->yin_ctx->xml_ctx, &data, &prefix.value, &prefix.len, &name.value, &name.len), LY_SUCCESS);
    assert_int_equal(yin_load_attributes(st->yin_ctx, &data, &attrs), LY_SUCCESS);
    assert_int_equal(yin_parse_submod(st->yin_ctx, attrs, &data, lysp_submod), LY_EVALID);
    logbuf_assert("Invalid order of submodule's subelements \"belongs-to\" can't appear after \"reference\". Line number 28.");
    lysp_submodule_free(st->ctx, lysp_submod);
    FREE_ARRAY(st->yin_ctx, attrs, free_arg_rec);
    attrs = NULL;

    st->finished_correctly = true;
}

static void
test_yin_parse_module(void **state)
{
    struct state *st = *state;
    const char *data;
    struct lys_module *mod;
    struct yin_parser_ctx *yin_ctx = NULL;

    mod = calloc(1, sizeof *mod);
    mod->ctx = st->ctx;
    data =  "<module name=\"example-foo\""
             "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
             "xmlns:foo=\"urn:example:foo\""
             "xmlns:myext=\"urn:example:extensions\">\n"

                "<yang-version value=\"1.0\"/>\n"

                "<namespace uri=\"urn:example:foo\"/>\n"
                "<prefix value=\"foo\"/>\n"

                "<import module=\"example-extensions\">\n"
                    "<prefix value=\"myext\"/>\n"
                "</import>\n"

                "<list name=\"interface\">\n"
                    "<key value=\"name\"/>\n"
                    "<leaf name=\"name\">\n"
                        "<type name=\"string\"/>\n"
                    "</leaf>\n"
                    "<leaf name=\"mtu\">\n"
                        "<type name=\"uint32\"/>\n"
                        "<description>\n"
                            "<text>The MTU of the interface.</text>\n"
                        "</description>\n"
                        "<myext:c-define name=\"MY_MTU\"/>\n"
                    "</leaf>\n"
                "</list>\n"
            "</module>\n";
    assert_int_equal(yin_parse_module(&yin_ctx, data, mod), LY_SUCCESS);
    lys_module_free(mod, NULL);
    yin_parser_ctx_free(yin_ctx);
    mod = NULL;
    yin_ctx = NULL;

    mod = calloc(1, sizeof *mod);
    mod->ctx = st->ctx;
    data =  "<module name=\"example-foo\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
                "<yang-version value=\"1.0\"/>\n"
                "<namespace uri=\"urn:example:foo\"/>\n"
                "<prefix value=\"foo\"/>\n"
            "</module>\n";
    assert_int_equal(yin_parse_module(&yin_ctx, data, mod), LY_SUCCESS);
    lys_module_free(mod, NULL);
    yin_parser_ctx_free(yin_ctx);
    mod = NULL;
    yin_ctx = NULL;


    mod = calloc(1, sizeof *mod);
    mod->ctx = st->ctx;
    data =  "<submodule name=\"example-foo\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "</submodule>\n";
    assert_int_equal(yin_parse_module(&yin_ctx, data, mod), LY_EINVAL);
    logbuf_assert("Input data contains submodule which cannot be parsed directly without its main module.");
    lys_module_free(mod, NULL);
    yin_parser_ctx_free(yin_ctx);

    mod = calloc(1, sizeof *mod);
    mod->ctx = st->ctx;
    data =  "<module name=\"example-foo\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
                "<yang-version value=\"1.0\"/>\n"
                "<namespace uri=\"urn:example:foo\"/>\n"
                "<prefix value=\"foo\"/>\n"
            "</module>"
            "<module>";
    assert_int_equal(yin_parse_module(&yin_ctx, data, mod), LY_EVALID);
    logbuf_assert("Trailing garbage \"<module>\" after module, expected end-of-input. Line number 5.");
    lys_module_free(mod, NULL);
    yin_parser_ctx_free(yin_ctx);
    mod = NULL;
    yin_ctx = NULL;

    st->finished_correctly = true;
}

static void
test_yin_parse_submodule(void **state)
{
    struct state *st = *state;
    const char *data;
    struct yin_parser_ctx *yin_ctx = NULL;
    struct lysp_submodule *submod = NULL;
    struct lys_parser_ctx main_ctx = {};

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<submodule name=\"asub\""
              "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
              "xmlns:a=\"urn:a\">"
                "<yang-version value=\"1.0\"/>\n"
                "<belongs-to module=\"a\">"
                    "<prefix value=\"a_pref\"/>"
                "</belongs-to>"
                "<include module=\"atop\"/>"
                "<feature name=\"fox\"/>"
                "<notification name=\"bar-notif\">"
                    "<if-feature name=\"bar\"/>"
                "</notification>"
                "<notification name=\"fox-notif\">"
                    "<if-feature name=\"fox\"/>"
                "</notification>"
                "<augment target-node=\"/a_pref:top\">"
                    "<if-feature name=\"bar\"/>"
                    "<container name=\"bar-sub\"/>"
                "</augment>"
                "<augment target-node=\"/top\">"
                    "<container name=\"bar-sub2\"/>"
                "</augment>"
            "</submodule>";
    assert_int_equal(yin_parse_submodule(&yin_ctx, st->ctx, &main_ctx, data, &submod), LY_SUCCESS);
    lysp_submodule_free(st->ctx, submod);
    yin_parser_ctx_free(yin_ctx);
    yin_ctx = NULL;
    submod = NULL;

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<yang-version value=\"1.0\"/>\n"
                "<belongs-to module=\"a\">"
                    "<prefix value=\"a_pref\"/>"
                "</belongs-to>"
            "</submodule>";
    assert_int_equal(yin_parse_submodule(&yin_ctx, st->ctx, &main_ctx, data, &submod), LY_SUCCESS);
    lysp_submodule_free(st->ctx, submod);
    yin_parser_ctx_free(yin_ctx);
    yin_ctx = NULL;
    submod = NULL;

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<module name=\"inval\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "</module>";
    assert_int_equal(yin_parse_submodule(&yin_ctx, st->ctx, &main_ctx, data, &submod), LY_EINVAL);
    logbuf_assert("Input data contains module in situation when a submodule is expected.");
    lysp_submodule_free(st->ctx, submod);
    yin_parser_ctx_free(yin_ctx);
    yin_ctx = NULL;
    submod = NULL;

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<yang-version value=\"1.0\"/>\n"
                "<belongs-to module=\"a\">"
                    "<prefix value=\"a_pref\"/>"
                "</belongs-to>"
            "</submodule>"
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
                "<yang-version value=\"1.0\"/>\n"
                "<belongs-to module=\"a\">"
                    "<prefix value=\"a_pref\"/>"
                "</belongs-to>"
            "</submodule>";
    assert_int_equal(yin_parse_submodule(&yin_ctx, st->ctx, &main_ctx, data, &submod), LY_EVALID);
    logbuf_assert("Trailing garbage \"<submodule name...\" after submodule, expected end-of-input. Line number 2.");
    lysp_submodule_free(st->ctx, submod);
    yin_parser_ctx_free(yin_ctx);
    yin_ctx = NULL;
    submod = NULL;

    st->finished_correctly = true;
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_yin_match_keyword, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_element_generic, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_extension_instance, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_yin_parse_content, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_validate_value, setup_f, teardown_f),

        cmocka_unit_test(test_yin_match_argument_name),
        cmocka_unit_test_setup_teardown(test_enum_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_bit_elem, setup_element_test, teardown_element_test),
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
        cmocka_unit_test_setup_teardown(test_prefix_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_range_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_reqinstance_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_revision_date_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_unique_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_units_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_when_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_yin_text_value_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_type_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_max_elems_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_min_elems_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_ordby_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_any_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_leaf_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_leaf_list_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_presence_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_key_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_typedef_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_refine_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_uses_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_revision_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_include_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_feature_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_identity_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_list_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_notification_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_grouping_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_container_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_case_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_choice_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_inout_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_action_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_augment_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_deviate_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_deviation_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_module_elem, setup_element_test, teardown_element_test),
        cmocka_unit_test_setup_teardown(test_submodule_elem, setup_element_test, teardown_element_test),

        cmocka_unit_test_setup_teardown(test_yin_parse_module, setup_logger, teardown_logger),
        cmocka_unit_test_setup_teardown(test_yin_parse_submodule, setup_logger, teardown_logger),
    };

    return cmocka_run_group_tests(tests, setup_ly_ctx, destroy_ly_ctx);
}
