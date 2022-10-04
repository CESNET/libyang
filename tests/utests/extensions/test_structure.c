/**
 * @file test_structure.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for structure extensions support
 *
 * Copyright (c) 2022 CESNET, z.s.p.o.
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
    struct lysc_ext_instance *e;
    char *printed = NULL;
    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct {"
            "  must \"/n2/l\";"
            "  status deprecated;"
            "  description desc;"
            "  reference no-ref;"
            "  typedef my-type {type string;}"
            "  grouping my-grp {leaf gl {type my-type;}}"
            "  grouping my-grp2 {leaf gl-obs {type uint16;}}"
            "  container n1 {leaf l {config false; type uint32;}}"
            "  list n2 {leaf l {type leafref {path /n1/l;}}}"
            "  uses my-grp;"
            "  uses my-grp2 {status obsolete;}"
            "}}";
    const char *info = "module a {\n"
            "  namespace \"urn:tests:extensions:structure:a\";\n"
            "  prefix self;\n"
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
            "    leaf gl-obs {\n"
            "      type uint16;\n"
            "      status obsolete;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* valid data */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_non_null(e = mod->compiled->exts);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 1);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);

    /* no substatements */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:structure:b; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct;}";
    info = "module b {\n"
            "  namespace \"urn:tests:extensions:structure:b\";\n"
            "  prefix self;\n"
            "\n"
            "  ietf-yang-structure-ext:structure \"struct\";\n"
            "}\n";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_non_null(e = mod->compiled->exts);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 1);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);
}

static void
test_schema_invalid(void **state)
{
    const char *data;

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct {import yang;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid keyword \"import\" as a child of \"sx:structure struct\" extension instance.",
            "/a:{extension='sx:structure'}/struct");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "container b { sx:structure struct { container x { leaf x {type string;}}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - structure, version 1\": "
            "Extension sx:structure must not be used as a non top-level statement in \"container\" statement.",
            "/a:b/{extension='sx:structure'}/struct");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure { container x { leaf x {type string;}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension instance \"sx:structure\" misses argument element \"name\".",
            "/a:{extension='sx:structure'}");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct { container x { leaf x {type string;}}}"
            "sx:structure struct { container y { leaf y {type string;}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - structure, version 1\": "
            "Extension sx:structure is instantiated multiple times.",
            "/a:{extension='sx:structure'}/struct");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct { container x { leaf x {type string;}}}"
            "choice struct { container y { leaf y {type string;}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - structure, version 1\": "
            "Extension sx:structure collides with a choice with the same identifier.",
            "/a:{extension='sx:structure'}/struct");
}

static void
test_parse(void **state)
{
    struct lys_module *mod;
    struct lysc_ext_instance *e;
    struct lyd_node *tree = NULL;
    const char *schema = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix self;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "sx:structure struct { container x { leaf x { type string;}}}}";
    const char *xml = "<x xmlns=\"urn:tests:extensions:structure:a\"><x>test</x></x>";
    const char *json = "{\"a:x\":{\"x\":\"test\"}}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_non_null(e = mod->compiled->exts);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(xml, &UTEST_IN));
    assert_int_equal(LY_SUCCESS, lyd_parse_ext_data(e, NULL, UTEST_IN, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);
    lyd_free_all(tree);

    ly_in_memory(UTEST_IN, json);
    assert_int_equal(LY_SUCCESS, lyd_parse_ext_data(e, NULL, UTEST_IN, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, json, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_schema),
        UTEST(test_schema_invalid),
        UTEST(test_parse),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
