/*
 * @file test_schema_extensions.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for YANG (YIN) extension statements and their instances in schemas
 *
 * Copyright (c) 2018-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "test_schema.h"

#include <string.h>

#include "context.h"
#include "log.h"
#include "plugins_exts.h"
#include "tree_schema.h"

void
test_extension_argument(void **state)
{
    struct lys_module *mod;
    const char *mod_def_yang = "module a {\n"
            "  namespace \"urn:a\";\n"
            "  prefix a;\n\n"
            "  extension e {\n"
            "    argument name;\n"
            "  }\n\n"
            "  a:e \"aaa\";\n"
            "}\n";
    const char *mod_def_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a\"/>\n\n"
            "  <extension name=\"e\">\n"
            "    <argument name=\"name\"/>\n"
            "  </extension>\n\n"
            "  <a:e name=\"aaa\"/>\n"
            "</module>\n";
    const char *mod_test_yin, *mod_test_yang;
    char *printed;

    mod_test_yang = "module b {\n"
            "  namespace \"urn:b\";\n"
            "  prefix b;\n\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n\n"
            "  a:e \"xxx\";\n"
            "}\n";
    mod_test_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"b\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:b=\"urn:b\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:b\"/>\n"
            "  <prefix value=\"b\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e name=\"xxx\"/>\n"
            "</module>\n";

    /* from YANG */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* context reset */
    ly_ctx_destroy(UTEST_LYCTX);
    ly_ctx_new(NULL, 0, &UTEST_LYCTX);

    /* from YIN */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yin);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);
}

void
test_extension_argument_element(void **state)
{
    struct lys_module *mod;
    const char *mod_def_yang = "module a {\n"
            "  namespace \"urn:a\";\n"
            "  prefix a;\n\n"
            "  extension e {\n"
            "    argument name {\n"
            "      yin-element true;\n"
            "    }\n"
            "  }\n\n"
            "  a:e \"aaa\";\n"
            "}\n";
    const char *mod_def_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a\"/>\n\n"
            "  <extension name=\"e\">\n"
            "    <argument name=\"name\">\n"
            "      <yin-element value=\"true\"/>\n"
            "    </argument>\n"
            "  </extension>\n\n"
            "  <a:e>\n"
            "    <a:name>aaa</a:name>\n"
            "  </a:e>\n"
            "</module>\n";
    const char *mod_test_yin, *mod_test_yang;
    char *printed;

    mod_test_yang = "module b {\n"
            "  namespace \"urn:b\";\n"
            "  prefix b;\n\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n\n"
            "  a:e \"xxx\";\n"
            "}\n";
    mod_test_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"b\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:b=\"urn:b\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:b\"/>\n"
            "  <prefix value=\"b\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <a:name>xxx</a:name>\n"
            "  </a:e>\n"
            "</module>\n";

    /* from YANG */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* context reset */
    ly_ctx_destroy(UTEST_LYCTX);
    ly_ctx_new(NULL, 0, &UTEST_LYCTX);

    /* from YIN */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yin);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* invalid */
    mod_test_yang = "module x { namespace \"urn:x\"; prefix x; import a { prefix a; } a:e; }";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" misses argument element \"name\".", "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e/>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" misses argument element \"name\".", "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e name=\"xxx\"/>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" misses argument element \"name\".", "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <x:name>xxx</x:name>\n"
            "  </a:e>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" element and its argument element \"name\" are expected in the same namespace, but they differ.",
            "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <a:value>xxx</a:value>\n"
            "  </a:e>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" expects argument element \"name\" as its first XML child, but \"value\" element found.",
            "/x:{extension='a:e'}");

}

void
test_extension_compile(void **state)
{
    struct lys_module *mod;
    struct lysc_ctx cctx = {0};
    struct lysp_ext_instance ext_p = {0};
    struct lysp_stmt child = {0};
    struct lysc_ext_instance ext_c = {0};
    struct lysc_ext_substmt *substmt;
    LY_ERR rc = LY_SUCCESS;

    /* current module, whatever */
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "yang");
    assert_true(mod);

    /* compile context */
    cctx.ctx = UTEST_LYCTX;
    cctx.cur_mod = mod;
    cctx.pmod = mod->parsed;
    cctx.path_len = 1;
    cctx.path[0] = '/';

    /* parsed ext instance */
    lydict_insert(UTEST_LYCTX, "pref:my-ext", 0, &ext_p.name);
    ext_p.format = LY_VALUE_JSON;
    ext_p.parent_stmt = LY_STMT_MODULE;

    /* compiled ext instance */
    ext_c.parent_stmt = ext_p.parent_stmt;
    // ext_c.parent =
    LY_ARRAY_NEW_GOTO(UTEST_LYCTX, ext_c.substmts, substmt, rc, cleanup);

    /*
     * error-message
     */
    ext_p.child = &child;
    lydict_insert(UTEST_LYCTX, "error-message", 0, &child.stmt);
    lydict_insert(UTEST_LYCTX, "my error", 0, &child.arg);
    child.format = LY_VALUE_JSON;
    child.kw = LY_STMT_ERROR_MESSAGE;

    substmt->stmt = LY_STMT_ERROR_MESSAGE;
    substmt->cardinality = LY_STMT_CARD_OPT;
    substmt->storage = &ext_c.data;

    /* compile */
    assert_int_equal(LY_SUCCESS, lys_compile_extension_instance(&cctx, &ext_p, &ext_c));

    /* check */
    assert_string_equal(ext_c.data, "my error");

cleanup:
    lydict_remove(UTEST_LYCTX, ext_p.name);
    lydict_remove(UTEST_LYCTX, child.stmt);
    lydict_remove(UTEST_LYCTX, child.arg);
    LY_ARRAY_FREE(ext_c.substmts);

    lydict_remove(UTEST_LYCTX, ext_c.data);
    if (rc) {
        fail();
    }
}
