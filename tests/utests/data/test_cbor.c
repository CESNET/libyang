/**
 * @file test_cbor.c
 * @author Juraj Budai <budai@cesnet.cz>
 * @brief Cmocka tests for CBOR data format.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "in_internal.h"
#include "parser_data.h"
#include "printer_data.h"

void **glob_state;

static int
setup(void **state)
{
    UTEST_SETUP;
    glob_state = state;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "cbor-test", NULL, NULL));

    return 0;
}

struct test_case {
    const char *name;
    const char *json;
};

static void
test_node(void **state)
{
    struct lyd_node *tree;
    char *buffer, *json;
    struct ly_out *out;
    size_t i;

    const struct test_case tests[] = {
        {"lf-str",      "{\"cbor-test:lf-str\":\"example string\"}"},
        {"lf-bool",     "{\"cbor-test:lf-bool\":true}"},
        {"lf-empty",    "{\"cbor-test:lf-empty\":[null]}"},
        {"lf-bin",      "{\"cbor-test:lf-bin\":\"dGVzdA==\"}"},
        {"lf-dec64",    "{\"cbor-test:lf-dec64\":\"12.34\"}"},

        {"lf-intg8",    "{\"cbor-test:lf-intg8\":-128}"},
        {"lf-intg16",   "{\"cbor-test:lf-intg16\":-32768}"},
        {"lf-intg32",   "{\"cbor-test:lf-intg32\":-2147483648}"},
        {"lf-intg64",   "{\"cbor-test:lf-intg64\":\"-9223372036854775808\"}"},

        {"lf-uintg8",   "{\"cbor-test:lf-uintg8\":255}"},
        {"lf-uintg16",  "{\"cbor-test:lf-uintg16\":65535}"},
        {"lf-uintg32",  "{\"cbor-test:lf-uintg32\":4294967295}"},
        {"lf-uintg64",  "{\"cbor-test:lf-uintg64\":\"18446744073709551615\"}"},

        {"lf-enum",     "{\"cbor-test:lf-enum\":\"green\"}"},
        {"lf-bits",     "{\"cbor-test:lf-bits\":\"up running\"}"},
        {"lf-idenref",  "{\"cbor-test:lf-idenref\":\"ethernet\"}"},
        {
            "lf-inst-id",
            "{"
            "\"cbor-test:cont-root\":{\"lf-root-str\":\"example string\"},"
            "\"cbor-test:lf-inst-id\":\"/cbor-test:cont-root/lf-root-str\""
            "}"
        },
        {"lf-union-s",  "{\"cbor-test:lf-union\":\"text\"}"},
        {"lf-union-b",  "{\"cbor-test:lf-union\":false}"},
        {
            "lf-lfref",
            "{"
            "\"cbor-test:lf-str\":\"referenced string\","
            "\"cbor-test:lf-lfref\":\"referenced string\""
            "}"
        },
        {
            "augment",
            "{"
            "\"cbor-test:cont-root\":{"
            "\"lf-root-str\":\"example string\","
            "\"lf-augmented-str\":\"augmented string\""
            "}"
            "}"
        },
        {"anydata",  "{\"cbor-test:anydata-payload\":{\"nested-key\":\"value\"}}"},
        {"anyxml",   "{\"cbor-test:anyxml-payload\":{\"xml-element\":\"text\"}}"},
        {"ll-str",      "{\"cbor-test:ll-str\":[\"val1\",\"val2\",\"val3\",\"val4\"]}"},
        {"ll-intg32",   "{\"cbor-test:ll-intg32\":[-30,10,20]}"},
        {"list-one",    "{\"cbor-test:list-user\":[{\"key-id\":1,\"lf-name\":\"Alice\"}]}"},
        {"list-multi",  "{\"cbor-test:list-user\":[{\"key-id\":1,\"lf-name\":\"Alice\"},{\"key-id\":2,\"lf-name\":\"Bob\"}]}"},
        {"choice-tcp",   "{\"cbor-test:lf-tcp-port\":8080}"},
        {"choice-unix",  "{\"cbor-test:lf-unix-socket\":\"/var/run/test.sock\"}"},
    };

    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        CHECK_PARSE_LYD_PARAM(tests[i].json, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);

        assert_int_equal(LY_SUCCESS, ly_out_new_memory(&buffer, 0, &out));
        assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_CBOR, 0));

        lyd_free_all(tree);

        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem_len(((struct utest_context *)*state)->ctx, buffer,
                (uint32_t)ly_out_printed(out), LYD_CBOR, LYD_PARSE_ONLY, 0, &tree));

        ly_out_free(out, NULL, 0);
        free(buffer);

        assert_int_equal(LY_SUCCESS, lyd_print_mem(&json, tree, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS));
        assert_string_equal(json, tests[i].json);

        free(json);
        lyd_free_all(tree);
    }
}

static void
test_operations(void **state)
{
    struct lyd_node *tree;
    char *buffer, *json;
    struct ly_out *out;
    struct ly_in *in;
    size_t i;

    const struct test_case tests[] = {
        {"rpc",          "{\"cbor-test:rpc-reset\":{\"lf1\":true}}"},
        {"notification", "{\"cbor-test:notif-alarm\":{\"lf-severity\":\"warning\",\"lf-message\":\"Something\"}}"},
    };

    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        enum lyd_type type = (strcmp(tests[i].name, "rpc") == 0) ? LYD_TYPE_RPC_YANG : LYD_TYPE_NOTIF_YANG;

        assert_int_equal(LY_SUCCESS, ly_in_new_memory(tests[i].json, &in));
        assert_int_equal(LY_SUCCESS, lyd_parse_op(((struct utest_context *)*state)->ctx, NULL, in, LYD_JSON, type, 0, &tree, NULL));
        ly_in_free(in, 0);

        assert_int_equal(LY_SUCCESS, ly_out_new_memory(&buffer, 0, &out));
        assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_CBOR, 0));

        lyd_free_all(tree);

        assert_int_equal(LY_SUCCESS, ly_in_new_memory(buffer, &in));
        in->length = ly_out_printed(out);
        assert_int_equal(LY_SUCCESS, lyd_parse_op(((struct utest_context *)*state)->ctx, NULL, in, LYD_CBOR, type, 0, &tree, NULL));
        ly_in_free(in, 0);

        ly_out_free(out, NULL, 0);
        free(buffer);

        assert_int_equal(LY_SUCCESS, lyd_print_mem(&json, tree, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS));
        assert_string_equal(json, tests[i].json);

        free(json);
        lyd_free_all(tree);
    }
}

static LY_ERR
test_ext_data_clb(const struct lysc_ext_instance *ext, const struct lyd_node *UNUSED(parent), void *user_data,
        void **ext_data, ly_bool *ext_data_free)
{
    void **state = glob_state;
    struct lyd_node *data = NULL;
    const struct lys_module *sm_mod;

    (void)ext;

    if (user_data) {
        ly_ctx_set_ext_data_clb(UTEST_LYCTX, test_ext_data_clb, NULL);
        CHECK_PARSE_LYD_PARAM(user_data, LYD_XML, LYD_PARSE_STRICT | LYD_PARSE_ONLY, 0, LY_SUCCESS, data);
        sm_mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-yang-schema-mount");
        assert_int_equal(LY_SUCCESS, lyd_validate_module(&data, sm_mod, 0, NULL));
        ly_ctx_set_ext_data_clb(UTEST_LYCTX, test_ext_data_clb, user_data);
    }

    *ext_data = data;
    *ext_data_free = 1;
    return LY_SUCCESS;
}

static void
test_schema_mount(void **state)
{
    struct lyd_node *tree = NULL;
    char *cbor_buffer = NULL, *xml_buffer = NULL;
    const char *ext_data, *input_xml;
    struct ly_out *out_cbor = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "base-mod", NULL, NULL));

    ext_data =
            "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\" "
            "    xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores\">"
            "  <module-set>"
            "    <name>mount-set</name>"
            "    <module>"
            "      <name>base-mod</name>"
            "      <namespace>urn:example:base-mod</namespace>"
            "    </module>"
            "  </module-set>"
            "  <schema>"
            "    <name>mount-schema</name>"
            "    <module-set>mount-set</module-set>"
            "  </schema>"
            "  <datastore>"
            "    <name>ds:running</name>"
            "    <schema>mount-schema</schema>"
            "  </datastore>"
            "  <content-id>1</content-id>"
            "</yang-library>"
            "<schema-mounts xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount\">"
            "  <mount-point>"
            "    <module>cbor-test</module>"
            "    <label>services</label>"
            "    <inline/>"
            "  </mount-point>"
            "</schema-mounts>";

    input_xml =
            "<cont-root xmlns=\"urn:cbor:test\">\n"
            "  <lf-root-str>Hello</lf-root-str>\n"
            "  <services>\n"
            "    <val xmlns=\"urn:example:base-mod\">Mounted value</val>\n"
            "  </services>\n"
            "</cont-root>\n";

    ly_ctx_set_ext_data_clb(UTEST_LYCTX, test_ext_data_clb, (void *)ext_data);

    CHECK_PARSE_LYD_PARAM(input_xml, LYD_XML, LYD_PARSE_STRICT | LYD_PARSE_ONLY, 0, LY_SUCCESS, tree);

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&cbor_buffer, 0, &out_cbor));
    assert_int_equal(LY_SUCCESS, lyd_print_all(out_cbor, tree, LYD_CBOR, 0));

    lyd_free_all(tree);
    tree = NULL;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem_len(UTEST_LYCTX, cbor_buffer,
            (uint32_t)ly_out_printed(out_cbor), LYD_CBOR, LYD_PARSE_STRICT | LYD_PARSE_ONLY, 0, &tree));

    ly_out_free(out_cbor, NULL, 0);
    free(cbor_buffer);

    assert_int_equal(LY_SUCCESS, lyd_print_mem(&xml_buffer, tree, LYD_XML, LYD_PRINT_SIBLINGS));
    assert_string_equal(input_xml, xml_buffer);

    free(xml_buffer);
    lyd_free_all(tree);
}

static void
test_opaque(void **state)
{
    struct lyd_node *tree;
    char *buffer, *json;
    const char *input_json;
    struct ly_out *out;

    input_json =
            "{"
            "\"cbor-test:cont-root\":{"
            "\"lf-root-str\":\"Something\","
            "\"nonexisting-element\":\"Random value\""
            "}"
            "}";

    CHECK_PARSE_LYD_PARAM(input_json, LYD_JSON, LYD_PARSE_ONLY | LYD_PARSE_OPAQ, 0, LY_SUCCESS, tree);

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&buffer, 0, &out));
    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_CBOR, 0));

    lyd_free_all(tree);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem_len(((struct utest_context *)*state)->ctx, buffer,
            (uint32_t)ly_out_printed(out), LYD_CBOR, LYD_PARSE_ONLY | LYD_PARSE_OPAQ, 0, &tree));

    ly_out_free(out, NULL, 0);
    free(buffer);

    assert_int_equal(LY_SUCCESS, lyd_print_mem(&json, tree, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS));
    assert_string_equal(json, input_json);

    free(json);
    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_node, setup),
        UTEST(test_operations, setup),
        UTEST(test_schema_mount, setup),
        UTEST(test_opaque, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
