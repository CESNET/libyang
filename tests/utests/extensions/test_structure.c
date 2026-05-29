/**
 * @file test_structure.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for structure extensions support
 *
 * Copyright (c) 2022 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"

static void
test_schema(void **state)
{
    struct lys_module *mod;
    char *printed = NULL;
    const char *data, *info;

    /* valid data */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix a;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct {"
            "  must \"/n2/l\";"
            "  status deprecated;"
            "  description desc;"
            "  reference no-ref;"
            "  typedef my-type {type string;}"
            "  grouping my-grp {leaf gl {type my-type;}}"
            "  container n1 {leaf l {config false; type uint32;}}"
            "  list n2 {leaf l {type leafref {path /n1/l;}}}"
            "  uses my-grp;"
            "}}";

    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 1);

    /* valid augment data */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:structure:b; prefix b;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "import a {prefix a;}"
            "sx:augment-structure \"/a:struct/a:n1\" {"
            "  status obsolete;"
            "  reference none;"
            "  leaf aug-leaf {type string;}"
            "}"
            "sx:augment-structure \"/a:struct\" {"
            "  leaf l {type uint32;}"
            "}}";

    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 2);

    /* yang compiled print */
    info = "module a {\n"
            "  namespace \"urn:tests:extensions:structure:a\";\n"
            "  prefix a;\n"
            "\n"
            "  ietf-yang-structure-ext:structure \"struct\" {\n"
            "    must \"/n2/l\";\n"
            "    status deprecated;\n"
            "    description\n"
            "      \"desc\";\n"
            "    reference\n"
            "      \"no-ref\";\n"
            "    container n1 {\n"
            "      status deprecated;\n"
            "      leaf l {\n"
            "        type uint32;\n"
            "        status deprecated;\n"
            "      }\n"
            "      leaf aug-leaf {\n"
            "        type string;\n"
            "        status obsolete;\n"
            "      }\n"
            "    }\n"
            "    list n2 {\n"
            "      min-elements 0;\n"
            "      max-elements 4294967295;\n"
            "      ordered-by user;\n"
            "      status deprecated;\n"
            "      leaf l {\n"
            "        type leafref {\n"
            "          path \"/n1/l\";\n"
            "          require-instance true;\n"
            "          type uint32;\n"
            "        }\n"
            "        status deprecated;\n"
            "      }\n"
            "    }\n"
            "    leaf gl {\n"
            "      type string;\n"
            "      status deprecated;\n"
            "    }\n"
            "    leaf l {\n"
            "      type uint32;\n"
            "      config true;\n"
            "      status deprecated;\n"
            "    }\n"
            "  }\n"
            "}\n";

    assert_non_null(mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "a"));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);

    info = "module b {\n"
            "  namespace \"urn:tests:extensions:structure:b\";\n"
            "  prefix b;\n"
            "\n"
            "  ietf-yang-structure-ext:augment-structure \"/a:struct/a:n1\";\n"
            "  ietf-yang-structure-ext:augment-structure \"/a:struct\";\n"
            "}\n";

    assert_non_null(mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "b"));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);

    /* no substatements */
    data = "module c {yang-version 1.1; namespace urn:tests:extensions:structure:c; prefix c;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct;}";
    info = "module c {\n"
            "  namespace \"urn:tests:extensions:structure:c\";\n"
            "  prefix c;\n"
            "\n"
            "  ietf-yang-structure-ext:structure \"struct\";\n"
            "}\n";

    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 1);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);
}

static void
test_schema_invalid(void **state)
{
    const char *data;

    /* structure */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct {import yang;}}";
    UTEST_INVALID_MODULE(data, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid keyword \"import\" as a child of \"sx:structure struct\" extension instance.",
            "/a:{ext-inst='sx:structure'}/struct", 0);

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "container b { sx:structure struct { container x { leaf x {type string;}}}}}";
    UTEST_INVALID_MODULE(data, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 structure\": "
            "Extension sx:structure must not be used as a non top-level statement in \"container\" statement.",
            "/a:b/{ext-inst='sx:structure'}/struct", 0);

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure { container x { leaf x {type string;}}}}";
    UTEST_INVALID_MODULE(data, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Extension instance \"sx:structure\" missing argument element \"name\".", "/a:{ext-inst='sx:structure'}", 0);

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct { container x { leaf x {type string;}}}"
            "sx:structure struct { container y { leaf y {type string;}}}}";
    UTEST_INVALID_MODULE(data, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 structure\": Extension sx:structure is instantiated multiple times.",
            "/a:{ext-inst='sx:structure'}/struct", 0);

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct { container x { leaf x {type string;}}}"
            "choice struct { container y { leaf y {type string;}}}}";
    UTEST_INVALID_MODULE(data, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Ext plugin \"ly2 structure\": Extension sx:structure collides with a choice with the same identifier.",
            "/a:{ext-inst='sx:structure'}/struct", 0);

    /* augment-structure */
    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix a;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct {"
            "  container n1 {leaf l {config false; type uint32;}}"
            "  list n2 {leaf l {type string;}}"
            "}"
            "container n1 {leaf l2 {type uint8;}}}";
    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, NULL);

    data = "module b {yang-version 1.1; namespace urn:tests:extensions:structure:b; prefix b;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "import a {prefix a;}"
            "sx:augment-structure \"/a:n1\" {"
            "  leaf aug-leaf {type string;}"
            "}}";
    UTEST_INVALID_MODULE(data, LYS_IN_YANG, NULL, LY_ENOTFOUND);
    CHECK_LOG_CTX("Augment ext-inst target node \"/a:n1\" from module \"b\" was not found.",
            "/b:{ext-inst='sx:augment-structure'}/{augment='/a:n1'}", 0);
}

static void
test_parse(void **state)
{
    struct lyd_node *tree = NULL, *node;
    const char *yang, *xml, *json;
    char *lyb;

    yang = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix a;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct { container x { leaf x { type leafref {path \"/x/y\"; }}"
            "anydata any;"
            "leaf y { type string; must \"/x/y = 'val'\";} leaf z { type instance-identifier;}}}}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, NULL);

    yang = "module b {yang-version 1.1; namespace urn:tests:extensions:structure:b; prefix b;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "import a {prefix a;}"
            "sx:augment-structure \"/a:struct/a:x\" {"
            "  leaf x2 {type uint32;}"
            "}}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, NULL);

    /* structure data, in all the formats */
    xml = "<struct xmlns=\"urn:tests:extensions:structure:a\">"
            "<x>"
            "<x>val</x>"
            "<y>val</y>"
            "<z xmlns:a=\"urn:tests:extensions:structure:a\">/a:x/a:x</z>"
            "<x2 xmlns=\"urn:tests:extensions:structure:b\">25</x2>"
            "</x>"
            "</struct>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(xml, &UTEST_IN));
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS);
    lyd_free_all(tree);

    json = "{\"a:struct\":{\"x\":{\"x\":\"val\",\"y\":\"val\",\"z\":\"/a:x/x\",\"b:x2\":25}}}";
    ly_in_memory(UTEST_IN, json);
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_JSON, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, json, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS);

    ly_out_new_memory(&lyb, 0, &UTEST_OUT);
    assert_int_equal(LY_SUCCESS, lyd_print_tree(UTEST_OUT, tree, LYD_LYB, 0));
    ly_out_free(current_utest_context->out, NULL, 0);
    lyd_free_all(tree);
    ly_in_memory(UTEST_IN, lyb);
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_LYB, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    free(lyb);
    lyd_free_all(tree);

    /* nested structure */
    xml = "<struct xmlns=\"urn:tests:extensions:structure:a\">"
            "<x>"
            "<struct>"
            "<x>"
            "<x>val</x>"
            "<y>val</y>"
            "<z xmlns:a=\"urn:tests:extensions:structure:a\">/a:x/a:y</z>"
            "<x2 xmlns=\"urn:tests:extensions:structure:b\">25</x2>"
            "</x>"
            "</struct>"
            "</x>"
            "</struct>";
    ly_in_memory(UTEST_IN, xml);
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS);
    lyd_free_all(tree);

    json = "{\"a:struct\":{\"x\":{\"struct\":{\"x\":{\"x\":\"val\",\"y\":\"val\",\"z\":\"/a:x/y\",\"b:x2\":25}}}}}";
    ly_in_memory(UTEST_IN, json);
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_JSON, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, json, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS);

    ly_out_new_memory(&lyb, 0, &UTEST_OUT);
    assert_int_equal(LY_SUCCESS, lyd_print_tree(UTEST_OUT, tree, LYD_LYB, 0));
    ly_out_free(current_utest_context->out, NULL, 0);
    lyd_free_all(tree);
    ly_in_memory(UTEST_IN, lyb);
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_LYB, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    free(lyb);

    /* invalid data */
    node = lyd_child_no_keys(lyd_child_no_keys(lyd_child_no_keys(lyd_child_no_keys(tree))));
    assert_string_equal(LYD_NAME(node->next), "y");
    lyd_free_tree(node->next);
    assert_string_equal(LYD_NAME(node), "x");
    lyd_free_tree(node);
    assert_int_equal(LY_ENOTFOUND, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));
    CHECK_LOG_CTX("Invalid instance-identifier \"/a:x/y\" value - required instance not found.",
            "/a:struct/x/struct/x/z", 0);
    lyd_free_all(tree);

    /* anydata strict */
    yang = "module c {yang-version 1.1; namespace urn:tests:c; prefix c;"
            "container cont {"
            "  leaf l {type string;}"
            "}}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, NULL);

    xml = "<struct xmlns=\"urn:tests:extensions:structure:a\">"
            "<x>"
            "<struct>"
            "<x>"
            "<any>"
            "<cont xmlns=\"urn:tests:c\"><l>val</l></cont>"
            "</any>"
            "</x>"
            "</struct>"
            "</x>"
            "</struct>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(xml, &UTEST_IN));
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_XML,
            LYD_PARSE_STRICT | LYD_PARSE_ANYDATA_STRICT, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS);
    lyd_free_all(tree);

    xml = "<struct xmlns=\"urn:tests:extensions:structure:a\">"
            "<x>"
            "<struct>"
            "<x>"
            "<any>"
            "<cont xmlns=\"urn:tests:c\"><l2>val</l2></cont>"
            "</any>"
            "</x>"
            "</struct>"
            "</x>"
            "</struct>";
    ly_in_memory(UTEST_IN, xml);
    assert_int_equal(LY_EVALID, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_XML,
            LYD_PARSE_STRICT | LYD_PARSE_ANYDATA_STRICT, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Node \"l2\" not found as a child of \"cont\" node.",
            "/a:struct/x/struct/x/any/c:cont", 1);

    /* patch */
    ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG);
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-patch", NULL, NULL));
    yang = "module example {yang-version 1.1;namespace \"http://example.tld/example\";prefix ex;"
            "  container a {"
            "    container b {"
            "      container c {"
            "        leaf enabled {"
            "          type boolean;"
            "          default true;"
            "        }"
            "        leaf blower {"
            "          type string;"
            "        }"
            "      }"
            "    }"
            "    container b1 { }"
            "    leaf something { type string; }"
            "  }"
            "  container two-leafs {"
            "    leaf a { type string; }"
            "    leaf b { type string; }"
            "  }"
            "}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, NULL);

    json =
            "{\n"
            "  \"ietf-yang-patch:yang-patch\": {\n"
            "    \"patch-id\": \"patch\",\n"
            "    \"edit\": [\n"
            "      {\n"
            "        \"edit-id\": \"edit\",\n"
            "        \"operation\": \"replace\",\n"
            "        \"target\": \"/example:a\",\n"
            "        \"value\": {\n"
            "          \"example:a\": \"aaa\"\n"
            "        }\n"
            "      },\n"
            "      {\n"
            "        \"edit-id\": \"edit\",\n"
            "        \"operation\": \"replace\",\n"
            "        \"target\": \"/example:b\",\n"
            "        \"value\": {\n"
            "          \"example:b\": \"bbb\"\n"
            "        }\n"
            "      }\n"
            "    ]\n"
            "  }\n"
            "}\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(json, &UTEST_IN));
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_JSON, LYD_PARSE_STRICT | LYD_PARSE_ONLY,
            0, &tree));
    CHECK_LYD_STRING_PARAM(tree, json, LYD_JSON, LYD_PRINT_SIBLINGS);
    lyd_free_all(tree);
}

static void
test_xpath(void **state)
{
    struct lyd_node *tree = NULL;
    struct ly_set *set;
    const char *yang, *xml;

    yang = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix a;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct { container x { leaf x { type leafref {path \"/x/y\"; }}"
            "anydata any;"
            "leaf y { type string; must \"/x/y = 'val'\";} leaf z { type instance-identifier;}}}}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, NULL);

    yang = "module c {yang-version 1.1; namespace urn:tests:c; prefix c;"
            "container cont {"
            "  leaf l {type string;}"
            "}}";
    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, NULL);

    xml = "<struct xmlns=\"urn:tests:extensions:structure:a\">"
            "<x>"
            "<struct>"
            "<x>"
            "<any>"
            "<cont xmlns=\"urn:tests:c\"><l>val</l></cont>"
            "</any>"
            "</x>"
            "</struct>"
            "</x>"
            "</struct>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(xml, &UTEST_IN));
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, NULL, UTEST_IN, LYD_XML,
            LYD_PARSE_STRICT | LYD_PARSE_ANYDATA_STRICT, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS);

    /* find xpath */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath3(NULL, tree, "/a:struct/x/struct/x/any/c:cont/l", LY_VALUE_JSON, NULL,
            NULL, &set));
    assert_int_equal(set->count, 1);
    ly_set_free(set, NULL);

    /* find path */
    assert_int_equal(LY_SUCCESS, lyd_find_path(tree, "/a:struct/x/struct/x/any/c:cont/l", 0, NULL));

    lyd_free_all(tree);

    /* create path */
    assert_int_equal(LY_SUCCESS, lyd_new_path(NULL, UTEST_LYCTX, "/a:struct/x/struct/x/any/c:cont/l", "val", 0, &tree));
    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_schema),
        UTEST(test_schema_invalid),
        UTEST(test_parse),
        UTEST(test_xpath),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
