/**
 * @file test_openconfig.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for openconfig extensions support
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

#include "printer_schema.h"

static void
test_schema(void **state)
{
    struct lys_module *mod;
    struct lysc_ext_instance *e;
    struct lysc_node *node;
    char *printed = NULL;
    const char *data, *yang, *info;

    /* load openconfig-extensions */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "openconfig-extensions", NULL, NULL));
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));

    /* valid data */
    data = "module oc-ext-pattern {namespace urn:oc-ext-pattern; prefix ocp;"
            "import openconfig-extensions {prefix oc-ext;}"
            "oc-ext:regexp-posix;"
            "container c {"
            "  leaf a {type string {pattern 'a';}}"
            "  leaf b {type string {oc-ext:posix-pattern '^a';}}"
            "}"
            "}";

    UTEST_ADD_MODULE(data, LYS_IN_YANG, NULL, &mod);
    assert_non_null(e = mod->compiled->exts);
    assert_int_equal(LYA_COUNT(mod->compiled->exts), 1);
    assert_non_null(node = lysc_node_child(mod->compiled->data)->next);
    assert_int_equal(LYA_COUNT(((struct lysc_node_leaf *)node)->type->exts), 1);

    /* yang print */
    yang = "module oc-ext-pattern {\n"
            "  namespace \"urn:oc-ext-pattern\";\n"
            "  prefix ocp;\n"
            "\n"
            "  import openconfig-extensions {\n"
            "    prefix oc-ext;\n"
            "  }\n"
            "\n"
            "  oc-ext:regexp-posix;\n"
            "\n"
            "  container c {\n"
            "    leaf a {\n"
            "      type string {\n"
            "        pattern 'a';\n"
            "      }\n"
            "    }\n"
            "    leaf b {\n"
            "      type string {\n"
            "        oc-ext:posix-pattern '^a';\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, yang);
    free(printed);

    /* yang compiled print BUG compiled pattern and extension pattern printed with " */
    info = "module oc-ext-pattern {\n"
            "  namespace \"urn:oc-ext-pattern\";\n"
            "  prefix ocp;\n"
            "\n"
            "  openconfig-extensions:regexp-posix;\n"
            "\n"
            "  container c {\n"
            "    config true;\n"
            "    status current;\n"
            "    leaf a {\n"
            "      type string {\n"
            "        pattern \"a\";\n"
            "      }\n"
            "      config true;\n"
            "      status current;\n"
            "    }\n"
            "    leaf b {\n"
            "      type string {\n"
            "        openconfig-extensions:posix-pattern \"^a\";\n"
            "      }\n"
            "      config true;\n"
            "      status current;\n"
            "    }\n"
            "  }\n"
            "}\n";

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);
}

static void
test_schema_invalid(void **state)
{
    const char *yang;

    /* load openconfig-extensions */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "openconfig-extensions", NULL, NULL));
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));

    yang = "module a {yang-version 1.1; namespace urn:tests:extensions:structure:a; prefix a;"
            "import openconfig-extensions {prefix oc-ext;}"
            "container c {oc-ext:regexp-posix;}}";
    UTEST_INVALID_MODULE(yang, LYS_IN_YANG, NULL, LY_EINVAL);
    CHECK_LOG_CTX("Ext plugin \"ly2 OpenConfig\": Extension \"oc-ext:regexp-posix\" is allowed only in 'module' or "
            "'submodule' statements, but it is placed in \"container\" statement.",
            "/a:c/{ext-inst='oc-ext:regexp-posix'}", 0);
}

static void
test_parse(void **state)
{
    struct lyd_node *tree = NULL;
    const char *yang, *xml;

    /* load openconfig-extensions */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "openconfig-extensions", NULL, NULL));
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));

    yang = "module oc-ext-pattern {namespace urn:oc-ext-pattern; prefix ocp;"
            "import openconfig-extensions {prefix oc-ext;}"
            "oc-ext:regexp-posix;"
            "container c {"
            "  leaf a {type string {pattern 'a';}}"
            "  leaf b {type string {oc-ext:posix-pattern '^a';}}"
            "}"
            "}";

    UTEST_ADD_MODULE(yang, LYS_IN_YANG, NULL, NULL);

    /* parse data */
    xml = "<c xmlns=\"urn:oc-ext-pattern\">"
            "<a>bab</a>"
            "<b>ab</b>"
            "</c>";
    CHECK_PARSE_LYD_PARAM(xml, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_SIBLINGS);
    lyd_free_all(tree);

    /* invalid data */
    xml = "<c xmlns=\"urn:oc-ext-pattern\">"
            "<a>bb</a>"
            "</c>";
    CHECK_PARSE_LYD_PARAM(xml, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Unsatisfied pattern - \"bb\" does not match \"a\".",
            "/oc-ext-pattern:c/a", 1);

    xml = "<c xmlns=\"urn:oc-ext-pattern\">"
            "<b>bba</b>"
            "</c>";
    CHECK_PARSE_LYD_PARAM(xml, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Ext plugin \"ly2 OpenConfig\": Unsatisfied pattern - \"bba\" does not match \"^a\".",
            "/oc-ext-pattern:c/b", 0);
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
