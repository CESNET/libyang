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
#include "../../src/xml.c"
#include "../../src/parser_yin.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"

static void
test_parse(void **state)
{
    (void)state; /* unused */

    struct ly_ctx *ctx;
    struct lys_module *mod;
    LY_ERR ret = LY_SUCCESS;

    ly_ctx_new(NULL, 0, &ctx);
    mod = calloc(1, sizeof(*mod));
    mod->ctx = ctx;

    ret = yin_parse_module(ctx, "<module name=\"example-foo\"\
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
                mod);

    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(mod->parsed->mod->name, "example-foo");
    assert_string_equal(mod->parsed->mod->prefix, "foo");

    lys_module_free(mod, NULL);
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

    assert_int_equal(match_keyword("anydatax", strlen("anydatax")), YANG_NONE);
    assert_int_equal(match_keyword("asdasd", strlen("asdasd")), YANG_NONE);
    assert_int_equal(match_keyword("", 0), YANG_NONE);
    assert_int_equal(match_keyword("anydata", strlen("anydata")), YANG_ANYDATA);
    assert_int_equal(match_keyword("anyxml", strlen("anyxml")), YANG_ANYXML);
    assert_int_equal(match_keyword("argument", strlen("argument")), YANG_ARGUMENT);
    assert_int_equal(match_keyword("augment", strlen("augment")), YANG_AUGMENT);
    assert_int_equal(match_keyword("base", strlen("base")), YANG_BASE);
    assert_int_equal(match_keyword("belongs-to", strlen("belongs-to")), YANG_BELONGS_TO);
    assert_int_equal(match_keyword("bit", strlen("bit")), YANG_BIT);
    assert_int_equal(match_keyword("case", strlen("case")), YANG_CASE);
    assert_int_equal(match_keyword("choice", strlen("choice")), YANG_CHOICE);
    assert_int_equal(match_keyword("config", strlen("config")), YANG_CONFIG);
    assert_int_equal(match_keyword("contact", strlen("contact")), YANG_CONTACT);
    assert_int_equal(match_keyword("container", strlen("container")), YANG_CONTAINER);
    assert_int_equal(match_keyword("default", strlen("default")), YANG_DEFAULT);
    assert_int_equal(match_keyword("description", strlen("description")), YANG_DESCRIPTION);
    assert_int_equal(match_keyword("deviate", strlen("deviate")), YANG_DEVIATE);
    assert_int_equal(match_keyword("deviation", strlen("deviation")), YANG_DEVIATION);
    assert_int_equal(match_keyword("enum", strlen("enum")), YANG_ENUM);
    assert_int_equal(match_keyword("error-app-tag", strlen("error-app-tag")), YANG_ERROR_APP_TAG);
    assert_int_equal(match_keyword("error-message", strlen("error-message")), YANG_ERROR_MESSAGE);
    assert_int_equal(match_keyword("extension", strlen("extension")), YANG_EXTENSION);
    assert_int_equal(match_keyword("feature", strlen("feature")), YANG_FEATURE);
    assert_int_equal(match_keyword("fraction-digits", strlen("fraction-digits")), YANG_FRACTION_DIGITS);
    assert_int_equal(match_keyword("grouping", strlen("grouping")), YANG_GROUPING);
    assert_int_equal(match_keyword("identity", strlen("identity")), YANG_IDENTITY);
    assert_int_equal(match_keyword("if-feature", strlen("if-feature")), YANG_IF_FEATURE);
    assert_int_equal(match_keyword("import", strlen("import")), YANG_IMPORT);
    assert_int_equal(match_keyword("include", strlen("include")), YANG_INCLUDE);
    assert_int_equal(match_keyword("input", strlen("input")), YANG_INPUT);
    assert_int_equal(match_keyword("key", strlen("key")), YANG_KEY);
    assert_int_equal(match_keyword("leaf", strlen("leaf")), YANG_LEAF);
    assert_int_equal(match_keyword("leaf-list", strlen("leaf-list")), YANG_LEAF_LIST);
    assert_int_equal(match_keyword("length", strlen("length")), YANG_LENGTH);
    assert_int_equal(match_keyword("list", strlen("list")), YANG_LIST);
    assert_int_equal(match_keyword("mandatory", strlen("mandatory")), YANG_MANDATORY);
    assert_int_equal(match_keyword("max-elements", strlen("max-elements")), YANG_MAX_ELEMENTS);
    assert_int_equal(match_keyword("min-elements", strlen("min-elements")), YANG_MIN_ELEMENTS);
    assert_int_equal(match_keyword("modifier", strlen("modifier")), YANG_MODIFIER);
    assert_int_equal(match_keyword("module", strlen("module")), YANG_MODULE);
    assert_int_equal(match_keyword("must", strlen("must")), YANG_MUST);
    assert_int_equal(match_keyword("namespace", strlen("namespace")), YANG_NAMESPACE);
    assert_int_equal(match_keyword("notification", strlen("notification")), YANG_NOTIFICATION);
    assert_int_equal(match_keyword("ordered-by", strlen("ordered-by")), YANG_ORDERED_BY);
    assert_int_equal(match_keyword("organization", strlen("organization")), YANG_ORGANIZATION);
    assert_int_equal(match_keyword("output", strlen("output")), YANG_OUTPUT);
    assert_int_equal(match_keyword("path", strlen("path")), YANG_PATH);
    assert_int_equal(match_keyword("pattern", strlen("pattern")), YANG_PATTERN);
    assert_int_equal(match_keyword("position", strlen("position")), YANG_POSITION);
    assert_int_equal(match_keyword("prefix", strlen("prefix")), YANG_PREFIX);
    assert_int_equal(match_keyword("presence", strlen("presence")), YANG_PRESENCE);
    assert_int_equal(match_keyword("range", strlen("range")), YANG_RANGE);
    assert_int_equal(match_keyword("reference", strlen("reference")), YANG_REFERENCE);
    assert_int_equal(match_keyword("refine", strlen("refine")), YANG_REFINE);
    assert_int_equal(match_keyword("require-instance", strlen("require-instance")), YANG_REQUIRE_INSTANCE);
    assert_int_equal(match_keyword("revision", strlen("revision")), YANG_REVISION);
    assert_int_equal(match_keyword("revision-date", strlen("revision-date")), YANG_REVISION_DATE);
    assert_int_equal(match_keyword("rpc", strlen("rpc")), YANG_RPC);
    assert_int_equal(match_keyword("status", strlen("status")), YANG_STATUS);
    assert_int_equal(match_keyword("submodule", strlen("submodule")), YANG_SUBMODULE);
    assert_int_equal(match_keyword("type", strlen("type")), YANG_TYPE);
    assert_int_equal(match_keyword("typedef", strlen("typedef")), YANG_TYPEDEF);
    assert_int_equal(match_keyword("unique", strlen("unique")), YANG_UNIQUE);
    assert_int_equal(match_keyword("units", strlen("units")), YANG_UNITS);
    assert_int_equal(match_keyword("uses", strlen("uses")), YANG_USES);
    assert_int_equal(match_keyword("value", strlen("value")), YANG_VALUE);
    assert_int_equal(match_keyword("when", strlen("when")), YANG_WHEN);
    assert_int_equal(match_keyword("yang-version", strlen("yang-version")), YANG_YANG_VERSION);
    assert_int_equal(match_keyword("yin-element", strlen("yin-element")), YANG_YIN_ELEMENT);
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
