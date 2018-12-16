#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"
#include "tree_schema_internal.h"
#include "../../src/parser_yin.c"

static void
test_parse(void **state)
{
    (void)state; /* unused */

    struct ly_ctx *ctx;
    struct lysp_module *mod;

    ly_ctx_new(NULL, 0, &ctx);
    mod = calloc(1, sizeof(*mod));
    mod->ctx = ctx;

    yin_parse(ctx, "<module name=\"example-foo\"\
                    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\
                    xmlns:foo=\"urn:example:foo\"\
                    xmlns:myext=\"urn:example:extensions\">\
                    <namespace uri=\"urn:example:foo\"/>\
                    <prefix value=\"foo\"/>\
                    \
                    <import module=\"example-extensions\">\
                        <prefix value=\"myext\"/>\
                    </import>\
                    \
                    <list name=\"interface\">\
                        <key value=\"name\"/>\
                        <leaf name=\"name\">\
                        <type name=\"string\"/>\
                        </leaf>\
                        <leaf name=\"mtu\">\
                        <type name=\"uint32\"/>\
                        <description>\
                            <text>The MTU of the interface.</text>\
                        </description>\
                        <myext:c-define name=\"MY_MTU\"/>\
                        </leaf>\
                    </list>\
                    </module>",
                &mod);

    assert_string_equal(mod->name, "example-foo");
    assert_string_equal(mod->prefix, "foo");

    lysp_module_free(mod);
    ly_ctx_destroy(ctx, NULL);
}

static int
test_setup(void **state) {
    (void)state; /* unused */
    return 0;
}

static void
test_match_keyword(void **state)
{
    (void)state; /* unused */

    assert_int_equal(match_keyword("anydata"), YANG_ANYDATA);
    assert_int_equal(match_keyword("asdasd"), YANG_NONE);
    assert_int_equal(match_keyword(""), YANG_NONE);
    assert_int_equal(match_keyword("anydata"), YANG_ANYDATA);
    assert_int_equal(match_keyword("anyxml"), YANG_ANYXML);
    assert_int_equal(match_keyword("argument"), YANG_ARGUMENT);
    assert_int_equal(match_keyword("augment"), YANG_AUGMENT);
    assert_int_equal(match_keyword("base"), YANG_BASE);
    assert_int_equal(match_keyword("belongs-to"), YANG_BELONGS_TO);
    assert_int_equal(match_keyword("bit"), YANG_BIT);
    assert_int_equal(match_keyword("case"), YANG_CASE);
    assert_int_equal(match_keyword("choice"), YANG_CHOICE);
    assert_int_equal(match_keyword("config"), YANG_CONFIG);
    assert_int_equal(match_keyword("contact"), YANG_CONTACT);
    assert_int_equal(match_keyword("container"), YANG_CONTAINER);
    assert_int_equal(match_keyword("default"), YANG_DEFAULT);
    assert_int_equal(match_keyword("description"), YANG_DESCRIPTION);
    assert_int_equal(match_keyword("deviate"), YANG_DEVIATE);
    assert_int_equal(match_keyword("deviation"), YANG_DEVIATION);
    assert_int_equal(match_keyword("enum"), YANG_ENUM);
    assert_int_equal(match_keyword("error-app-tag"), YANG_ERROR_APP_TAG);
    assert_int_equal(match_keyword("error-message"), YANG_ERROR_MESSAGE);
    assert_int_equal(match_keyword("extension"), YANG_EXTENSION);
    assert_int_equal(match_keyword("feature"), YANG_FEATURE);
    assert_int_equal(match_keyword("fraction-digits"), YANG_FRACTION_DIGITS);
    assert_int_equal(match_keyword("grouping"), YANG_GROUPING);
    assert_int_equal(match_keyword("identity"), YANG_IDENTITY);
    assert_int_equal(match_keyword("if-feature"), YANG_IF_FEATURE);
    assert_int_equal(match_keyword("import"), YANG_IMPORT);
    assert_int_equal(match_keyword("include"), YANG_INCLUDE);
    assert_int_equal(match_keyword("input"), YANG_INPUT);
    assert_int_equal(match_keyword("key"), YANG_KEY);
    assert_int_equal(match_keyword("leaf"), YANG_LEAF);
    assert_int_equal(match_keyword("leaf-list"), YANG_LEAF_LIST);
    assert_int_equal(match_keyword("length"), YANG_LENGTH);
    assert_int_equal(match_keyword("list"), YANG_LIST);
    assert_int_equal(match_keyword("mandatory"), YANG_MANDATORY);
    assert_int_equal(match_keyword("max-elements"), YANG_MAX_ELEMENTS);
    assert_int_equal(match_keyword("min-elements"), YANG_MIN_ELEMENTS);
    assert_int_equal(match_keyword("modifier"), YANG_MODIFIER);
    assert_int_equal(match_keyword("module"), YANG_MODULE);
    assert_int_equal(match_keyword("must"), YANG_MUST);
    assert_int_equal(match_keyword("namespace"), YANG_NAMESPACE);
    assert_int_equal(match_keyword("notification"), YANG_NOTIFICATION);
    assert_int_equal(match_keyword("ordered-by"), YANG_ORDERED_BY);
    assert_int_equal(match_keyword("organization"), YANG_ORGANIZATION);
    assert_int_equal(match_keyword("output"), YANG_OUTPUT);
    assert_int_equal(match_keyword("path"), YANG_PATH);
    assert_int_equal(match_keyword("pattern"), YANG_PATTERN);
    assert_int_equal(match_keyword("position"), YANG_POSITION);
    assert_int_equal(match_keyword("prefix"), YANG_PREFIX);
    assert_int_equal(match_keyword("presence"), YANG_PRESENCE);
    assert_int_equal(match_keyword("range"), YANG_RANGE);
    assert_int_equal(match_keyword("reference"), YANG_REFERENCE);
    assert_int_equal(match_keyword("refine"), YANG_REFINE);
    assert_int_equal(match_keyword("require-instance"), YANG_REQUIRE_INSTANCE);
    assert_int_equal(match_keyword("revision"), YANG_REVISION);
    assert_int_equal(match_keyword("revision-date"), YANG_REVISION_DATE);
    assert_int_equal(match_keyword("rpc"), YANG_RPC);
    assert_int_equal(match_keyword("status"), YANG_STATUS);
    assert_int_equal(match_keyword("submodule"), YANG_SUBMODULE);
    assert_int_equal(match_keyword("type"), YANG_TYPE);
    assert_int_equal(match_keyword("typedef"), YANG_TYPEDEF);
    assert_int_equal(match_keyword("unique"), YANG_UNIQUE);
    assert_int_equal(match_keyword("units"), YANG_UNITS);
    assert_int_equal(match_keyword("uses"), YANG_USES);
    assert_int_equal(match_keyword("value"), YANG_VALUE);
    assert_int_equal(match_keyword("when"), YANG_WHEN);
    assert_int_equal(match_keyword("yang-version"), YANG_YANG_VERSION);
    assert_int_equal(match_keyword("yin-element"), YANG_YIN_ELEMENT);
}

static void
test_match_argument(void **state)
{
    (void)state;

    assert_int_equal(match_argument_name("", 5), YIN_ARG_NONE);
    assert_int_equal(match_argument_name("qwertyasd", 5), YIN_ARG_NONE);
    assert_int_equal(match_argument_name("conditionasd", 8), YIN_ARG_NONE);
    assert_int_equal(match_argument_name("condition", 9), YIN_ARG_CONDITION);
    assert_int_equal(match_argument_name("date", 4), YIN_ARG_DATE);
    assert_int_equal(match_argument_name("module", 6), YIN_ARG_MODULE);
    assert_int_equal(match_argument_name("name", 4), YIN_ARG_NAME);
    assert_int_equal(match_argument_name("tag", 3), YIN_ARG_TAG);
    assert_int_equal(match_argument_name("target-node", 11), YIN_ARG_TARGET_NODE);
    assert_int_equal(match_argument_name("text", 4), YIN_ARG_TEXT);
    assert_int_equal(match_argument_name("uri", 3), YIN_ARG_URI);
    assert_int_equal(match_argument_name("value", 5), YIN_ARG_VALUE);
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_parse, test_setup),
        cmocka_unit_test(test_match_keyword),
        cmocka_unit_test(test_match_argument),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
