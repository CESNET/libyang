/**
 * @file test_yang.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for YANG module parser and printer
 *
 * Copyright (c) 2018 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include <stdio.h>
#include <string.h>

#include "tree_edit.h"
#include "tree_schema.h"

#define TEST_MOD(CTX, YANG_STR, EXPECTED_VALUE) \
    do { \
        assert_int_equal((EXPECTED_VALUE), lys_parse_mem((CTX), (YANG_STR), LYS_IN_YANG, NULL)); \
    } while(0)

#define TEST_SUBMOD(CTX, SUBMOD_STR, EXPECTED_VALUE) \
do { \
    ly_ctx_set_module_imp_clb((CTX), test_imp_clb, (SUBMOD_STR)); \
    const char *parent_str = "module parent { yang-version 1.1; namespace \"urn:parent\"; prefix p; include subname; }"; \
    assert_int_equal((EXPECTED_VALUE), lys_parse_mem((CTX), parent_str, LYS_IN_YANG, NULL)); \
} while(0)

static void
test_helpers(void **state)
{
    /* invalid first characters */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module 2invalid { namespace urn:test; prefix t; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid identifier first character '2' (0x0032).", NULL, 1);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module .invalid { namespace urn:test; prefix t; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid identifier first character '.' (0x002e).", NULL, 1);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module -invalid { namespace urn:test; prefix t; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid identifier first character '-' (0x002d).", NULL, 1);
    /* invalid following characters */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module inv!alid { namespace urn:test; prefix t; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid identifier character '!' (0x0021).", NULL, 1);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module inv:alid { namespace urn:test; prefix t; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", NULL, 1);

    /* valid colon for prefixed identifiers */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module valid { namespace urn:test; prefix t; typedef my-string { type string; } leaf my-leaf { type t:my-string; } }", LYS_IN_YANG, NULL));

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module invalid { namespace urn:test; prefix t; typedef my-string { type string; } leaf my-leaf { type t::my-string; } }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"invalid\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier first character ':' (0x003a).", NULL, 1);
}

static void
test_comments(void **state)
{
    struct lys_module *mod;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test1 { // this is a text of / one * line */ comment\nnamespace urn:test1; prefix t1; }", LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test2 {/* this is a \n * text // of / block * comment */namespace \"urn:t\" + \"es\" \n + \n \"t2\"; prefix t2;}", LYS_IN_YANG, &mod));
    assert_string_equal("urn:test2", mod->ns);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test3 { namespace urn:test3; prefix t3; } // this is one line comment on last line", LYS_IN_YANG, NULL));

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test4 { namespace urn:test4; prefix t4; /* this is a not terminated comment x ", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test4\" failed.", NULL, 0);
    CHECK_LOG_CTX("Unexpected end-of-input, non-terminated comment.", NULL, 1);
}

static void
test_arg(void **state)
{
    struct lys_module *mod;
    const char *schema;

    /* missing argument */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test { namespace urn:test; prefix ; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid character sequence \";\", expected an argument.", NULL, 1);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test { namespace urn:test; prefix {} }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid character sequence \"{\", expected an argument.", NULL, 1);

    /* invalid escape sequence */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test { namespace urn:test; prefix t; description \"\\s\"; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Double-quoted string unknown special character \'\\s\'.", NULL, 1);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test { namespace urn:test; prefix t; description \'\\s\'; }", LYS_IN_YANG, NULL));

    /* invalid character after the argument */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test1 { namespace hello\"; prefix t;}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid character sequence \"\"\", expected unquoted string character, optsep, semicolon or opening brace.", NULL, 1);

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test1 { namespace hello}; prefix t;}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid character sequence \"}\", expected unquoted string character, optsep, semicolon or opening brace.", NULL, 1);

    /* invalid identifier-ref-arg-str */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test1 { namespace urn:test; prefix t; typedef my-string { type string; } leaf my-leaf { type t:t:my-string; } }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", NULL, 1);

    /* empty identifier is not allowed */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test1 { namespace urn:test; prefix \"\"; }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Statement argument is required.", NULL, 1);

    /* empty reference identifier is not allowed */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test1 { namespace urn:test; prefix t; typedef my-string { type string; } leaf my-leaf { type \"\"; } }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Statement argument is required.", NULL, 1);

    /* slash is not an invalid character */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test1 { namespace urn:test1; prefix t; description \"hello/x\"; }", LYS_IN_YANG, &mod));
    assert_string_equal("hello/x", mod->dsc);

    /* different quoting */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test2 { namespace urn:test2; prefix t; description \"hello/x\t\"; }", LYS_IN_YANG, &mod));
    assert_string_equal("hello/x\t", mod->dsc);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test3 { namespace urn:test3; prefix t; description \"hello \"; }", LYS_IN_YANG, &mod));
    assert_string_equal("hello ", mod->dsc);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module test4 { namespace urn:test4; prefix t; description \"hello\\n\\t\\\"\\\\\"; }", LYS_IN_YANG, &mod));
    assert_string_equal("hello\n\t\"\\", mod->dsc);

    schema = "module test5 {\n"
            "  namespace urn:test5;\n"
            "  prefix t;\n"
            "  description \"hello \t\n\t\t world!\";\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello\n  world!", mod->dsc);

    /* In contrast to previous, the backslash-escaped tabs are expanded after trimming, so they are preserved */
    schema = "module test6 {\n"
            "  namespace urn:test6;\n"
            "  prefix t;\n"
            "  description \"hello \\t\n\t\\t world!\";\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello \t\n\t world!", mod->dsc);

    /* Do not handle whitespaces after backslash-escaped newline as indentation */
    schema = "module test7 {\n"
            "  namespace urn:test7;\n"
            "  prefix t;\n"
            "  description \"hello\\n\t\t world!\";\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello\n\t\t world!", mod->dsc);

    schema = "module test8 {\n"
            "  namespace urn:test8;\n"
            "  prefix t;\n"
            "  description \"hello\n \tworld!\";\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello\nworld!", mod->dsc);

    schema = "module test9 {\n"
            "  namespace urn:test9;\n"
            "  prefix t;\n"
            "  description \'hello\';\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello", mod->dsc);

    schema = "module test10 {\n"
            "  namespace urn:test10;\n"
            "  prefix t;\n"
            "  description \"hel\"  +\t\n\"lo\";\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello", mod->dsc);

    schema = "module test11 {\n"
            "  namespace urn:test11;\n"
            "  prefix t;\n"
            "  description \'he\'\t\n+ \"llo\";\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello", mod->dsc);

    schema = "module test12 {\n"
            "  namespace urn:test12;\n"
            "  prefix t;\n"
            "  description \t\n\"he\"+\'llo\';\n"
            "}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_string_equal("hello", mod->dsc);

    schema = "module test13 {\n"
            "  namespace urn:test13;\n"
            "  prefix t;\n"
            "  description \"hel\"  +\t\nlo;\n"
            "}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Parsing module \"test13\" failed.", NULL, 0);
    CHECK_LOG_CTX("Both string parts divided by '+' must be quoted.", NULL, 5);
}

#define TEST_STMS_SUCCESS(CTX, MOD_NAME, STMT_TEXT) \
    do { \
        struct lys_module *mod; \
        char buffer[1024]; \
        snprintf(buffer, sizeof(buffer), \
                 "module %s { yang-version 1.1; namespace \"urn:test:%s\"; prefix t; %s }", \
                 (MOD_NAME), (MOD_NAME), (STMT_TEXT)); \
        assert_int_equal(LY_SUCCESS, lys_parse_mem((CTX), buffer, LYS_IN_YANG, &mod)); \
    } while(0)

static void
test_stmts(void **state)
{
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test {namespace \"urn:test:err\"; prefix e;  /input { }", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier first character '/'.", NULL, 1);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module test { namespace \"urn:test:err2\"; prefix e2; not-a-statement-nor-extension {}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid character sequence \"not-a-statement-nor-extension\", expected a keyword.", NULL, 1);

    TEST_STMS_SUCCESS(UTEST_LYCTX, "t-input-fmt", "rpc my-rpc { \n// comment\n\tinput\t{ leaf in-leaf { type string; } } }");
    TEST_STMS_SUCCESS(UTEST_LYCTX, "t-output-fmt", "rpc my-rpc { \t /* comment */\t output\n\t{ leaf out-leaf { type string; } } }");
}

static void
test_minmax(void **state)
{
    const char *yang =
            "module test {"
            "  yang-version 1.1;"
            "  namespace \"urn:test\";"
            "  prefix t;"
            "  leaf-list ll {"
            "    type string;"
            "    min-elements %s;"
            "    max-elements %s;"
            "  }"
            "}";
    char buf[256];

    snprintf(buf, sizeof buf, yang, "1invalid", "...");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"1invalid\" of \"min-elements\".", NULL, 1);

    snprintf(buf, sizeof buf, yang, "-1", "...");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"min-elements\".", NULL, 1);

    /* implementation limit */
    snprintf(buf, sizeof buf, yang, "4294967296", "...");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Value \"4294967296\" is out of \"min-elements\" bounds.", NULL, 1);

    snprintf(buf, sizeof buf, yang, "1 {config true;}", "...");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"min-elements\".", NULL, 1);

    snprintf(buf, sizeof buf, yang, "2", "1invalid");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"1invalid\" of \"max-elements\".", NULL, 1);

    snprintf(buf, sizeof buf, yang, "2", "-1");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"max-elements\".", NULL, 1);

    snprintf(buf, sizeof buf, yang, "2", "4294967296");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Value \"4294967296\" is out of \"max-elements\" bounds.", NULL, 1);

    snprintf(buf, sizeof buf, yang, "2", "1 {config true;}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buf, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"max-elements\".", NULL, 1);
}

static void
test_valid_module(void **state)
{
    struct lys_module *mod;
    char *printed;
    const char *links_yang =
            "module links {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:module2\";\n"
            "  prefix mod2;\n"
            "\n"
            "  identity just-another-identity;\n"
            "\n"
            "  leaf one-leaf {\n"
            "    type string;\n"
            "  }\n"
            "\n"
            "  list list-for-augment {\n"
            "    key keyleaf;\n"
            "\n"
            "    leaf keyleaf {\n"
            "      type string;\n"
            "    }\n"
            "\n"
            "    leaf just-leaf {\n"
            "      type int32;\n"
            "    }\n"
            "  }\n"
            "\n"
            "  leaf rleaf {\n"
            "    type string;\n"
            "  }\n"
            "\n"
            "  leaf-list llist {\n"
            "    type string;\n"
            "    min-elements 0;\n"
            "    max-elements 100;\n"
            "    ordered-by user;\n"
            "  }\n"
            "\n"
            "  grouping rgroup {\n"
            "    leaf rg1 {\n"
            "      type string;\n"
            "    }\n"
            "\n"
            "    leaf rg2 {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n";
    const char *statements_yang =
            "module statements {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:module\";\n"
            "  prefix mod;\n"
            "\n"
            "  import links {\n"
            "    prefix mod2;\n"
            "  }\n"
            "\n"
            "  extension ext;\n"
            "\n"
            "  identity random-identity {\n"
            "    base mod2:just-another-identity;\n"
            "    base another-identity;\n"
            "  }\n"
            "\n"
            "  identity another-identity {\n"
            "    base mod2:just-another-identity;\n"
            "  }\n"
            "\n"
            "  typedef percent {\n"
            "    type uint8 {\n"
            "      range \"0 .. 100\";\n"
            "    }\n"
            "    units \"percent\";\n"
            "  }\n"
            "\n"
            "  list list1 {\n"
            "    key \"a\";\n"
            "    leaf a {\n"
            "      type string;\n"
            "    }\n"
            "    leaf x {\n"
            "      type string;\n"
            "    }\n"
            "    leaf y {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "  container ice-cream-shop {\n"
            "    container employees {\n"
            "      when \"/list1/x\";\n"
            "      list employee {\n"
            "        key \"id\";\n"
            "        unique \"name\";\n"
            "        config true;\n"
            "        min-elements 0 {\n"
            "          mod:ext;\n"
            "        }\n"
            "        max-elements unbounded;\n"
            "        leaf id {\n"
            "          type uint64;\n"
            "          mandatory true;\n"
            "        }\n"
            "        leaf name {\n"
            "          type string;\n"
            "        }\n"
            "        leaf age {\n"
            "          type uint32;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  container random {\n"
            "    grouping group {\n"
            "      leaf g1 {\n"
            "        type percent;\n"
            "        mandatory false;\n"
            "      }\n"
            "      leaf g2 {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    choice switch {\n"
            "      case a {\n"
            "        leaf aleaf {\n"
            "          type string;\n"
            "          default \"aaa\";\n"
            "        }\n"
            "      }\n"
            "      case c {\n"
            "        leaf cleaf {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "    anyxml xml-data;\n"
            "    anydata any-data;\n"
            "    leaf-list leaflist {\n"
            "      type string;\n"
            "      min-elements 0;\n"
            "      max-elements 20;\n"
            "    }\n"
            "    uses group;\n"
            "    uses mod2:rgroup;\n"
            "    leaf lref {\n"
            "      type leafref {\n"
            "        path \"/mod2:one-leaf\";\n"
            "      }\n"
            "    }\n"
            "    leaf iref {\n"
            "      type identityref {\n"
            "        base mod2:just-another-identity;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "\n"
            "  augment \"/random\" {\n"
            "    leaf aug-leaf {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "\n"
            "  notification notif;\n"
            "\n"
            "  deviation \"/mod:ice-cream-shop/mod:employees/mod:employee/mod:age\" {\n"
            "    deviate not-supported {\n"
            "      mod:ext;\n"
            "    }\n"
            "  }\n"
            "  deviation \"/mod:list1\" {\n"
            "    deviate add {\n"
            "      mod:ext;\n"
            "      must \"1\";\n"
            "      must \"2\";\n"
            "      unique \"x\";\n"
            "      unique \"y\";\n"
            "      config true;\n"
            "      min-elements 1;\n"
            "      max-elements 2;\n"
            "    }\n"
            "  }\n"
            "  deviation \"/mod:ice-cream-shop/mod:employees/mod:employee\" {\n"
            "    deviate delete {\n"
            "      unique \"name\";\n"
            "    }\n"
            "  }\n"
            "  deviation \"/mod:random/mod:leaflist\" {\n"
            "    deviate replace {\n"
            "      type uint32;\n"
            "      min-elements 10;\n"
            "      max-elements 15;\n"
            "    }\n"
            "  }\n"
            "}\n";

    UTEST_ADD_MODULE(links_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(statements_yang, LYS_IN_YANG, NULL, &mod);
    lys_print_mem(&printed, mod, LYS_OUT_YANG, 0);
    assert_string_equal(printed, statements_yang);
    free(printed);
}

static LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YANG;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
test_module(void **state)
{
    /* Missing mandatory substatements */
    TEST_MOD(UTEST_LYCTX, "module name {}", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"name\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory keyword \"namespace\" as a child of \"module\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module name { namespace urn:name; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"name\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory keyword \"prefix\" as a child of \"module\".", NULL, 1);

    /* Successful minimal module */
    TEST_MOD(UTEST_LYCTX, "module name { namespace urn:name; prefix n; }", LY_SUCCESS);

    /* Duplicated headers */
    TEST_MOD(UTEST_LYCTX, "module dup { namespace urn:x; prefix x; namespace urn:y; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"dup\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"namespace\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module dup { namespace urn:x; prefix x; prefix y; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"dup\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"prefix\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module dup { namespace urn:x; prefix x; contact a; contact b; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"dup\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"contact\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module dup { namespace urn:x; prefix x; description a; description b; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"dup\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"description\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module dup { namespace urn:x; prefix x; organization a; organization b; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"dup\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"organization\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module dup { namespace urn:x; prefix x; reference a; reference b; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"dup\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"reference\".", NULL, 1);

    /* Invalid Keywords in Module (Submodule-specific or illegal) */
    TEST_MOD(UTEST_LYCTX, "module inv { namespace urn:x; prefix x; belongs-to master { prefix m; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"inv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"belongs-to\" as a child of \"module\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module inv { namespace urn:x; prefix x; must false; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"inv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"must\" as a child of \"module\".", NULL, 1);

    /* Yang-version parsing and validation */
    TEST_MOD(UTEST_LYCTX, "module yv { yang-version 10; namespace urn:x; prefix x; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"yv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"10\" of \"yang-version\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module yv { yang-version 1.1; yang-version 1.1; namespace urn:x; prefix x; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"yv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"yang-version\".", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "module yv { yang-version 1; namespace urn:x; prefix x; }", LY_SUCCESS);
    TEST_MOD(UTEST_LYCTX, "module yv { yang-version \"1.1\"; namespace \"urn:yv4\"; prefix \"yv4\";}", LY_SUCCESS);

    /* Trailing Garbage and Base Syntax */
    TEST_MOD(UTEST_LYCTX, "module tg1 { namespace urn:x; prefix x; } module q { namespace urn:q; prefix q; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"tg1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Trailing garbage \"module q { name...\" after module, expected end-of-input.", NULL, 1);
    TEST_MOD(UTEST_LYCTX, "prefix name { }", LY_EVALID);
    CHECK_LOG_CTX("Invalid keyword \"prefix\", expected \"module\" or \"submodule\".", NULL, 1);

    /* Submodules */
    TEST_SUBMOD(UTEST_LYCTX, "submodule subname {}", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"parent\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"subname\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory keyword \"belongs-to\" as a child of \"submodule\".", NULL, 1);
    TEST_SUBMOD(UTEST_LYCTX, "submodule subname { belongs-to parent { prefix p; } belongs-to parent2; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"parent\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"subname\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"belongs-to\".", NULL, 1);
    TEST_SUBMOD(UTEST_LYCTX, "submodule subname { belongs-to parent { prefix p; } namespace \"urn:z\"; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"parent\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"subname\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"namespace\" as a child of \"submodule\".", NULL, 1);
    TEST_SUBMOD(UTEST_LYCTX, "submodule subname { belongs-to parent { prefix p; } prefix m; }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"parent\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"subname\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"prefix\" as a child of \"submodule\".", NULL, 1);
    TEST_SUBMOD(UTEST_LYCTX, "submodule subname { belongs-to parent { prefix p; } } module q {}", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"parent\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"subname\" failed.", NULL, 0);
    CHECK_LOG_CTX("Trailing garbage \"module q {}\" after submodule, expected end-of-input.", NULL, 1);
    TEST_SUBMOD(UTEST_LYCTX, "prefix subname { belongs-to parent { prefix p; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"parent\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"subname\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"prefix\", expected \"module\" or \"submodule\".", NULL, 1);

    TEST_SUBMOD(UTEST_LYCTX, "submodule subname { belongs-to parent { prefix p; } }", LY_SUCCESS);

    /* Pre-load modules required for the Import tests */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module zzz { yang-version 1.1; namespace \"urn:zzz\"; prefix z; }", LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module yyy { yang-version 1.1; namespace \"urn:yyy\"; prefix y; }", LYS_IN_YANG, NULL));

    /* Imports */
    TEST_MOD(UTEST_LYCTX, "module imp-success { namespace urn:s1; prefix s1; import zzz { prefix my-z; } }", LY_SUCCESS);
    TEST_MOD(UTEST_LYCTX, "module imp-multi { namespace urn:s2; prefix s2; import zzz { prefix z; } import yyy { prefix y; } }", LY_SUCCESS);
    TEST_MOD(UTEST_LYCTX, "module imp { namespace urn:e4; prefix e4; import zzz { prefix a; } import zzz { prefix b; } }", LY_SUCCESS);
    CHECK_LOG_CTX("Single revision of the module \"zzz\" imported twice.", NULL, 0);
    TEST_MOD(UTEST_LYCTX, "module err { namespace urn:e1; prefix e1; import missing-mod { prefix m; } }", LY_EINVAL);
    CHECK_LOG_CTX("Parsing module \"err\" failed.", NULL, 0);
    CHECK_LOG_CTX("Input data contains submodule which cannot be parsed directly without its main module.", NULL, 0);
    TEST_MOD(UTEST_LYCTX, "module err { namespace urn:e2; prefix e2; import zzz { prefix x; } import yyy { prefix x; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"err\" failed.", NULL, 0);
    CHECK_LOG_CTX("Prefix \"x\" already used to import \"zzz\" module.", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module err { namespace urn:e3; prefix z; import zzz { prefix z; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"err\" failed.", NULL, 0);
    CHECK_LOG_CTX("Prefix \"z\" already used as module prefix.", NULL, 1);
}

static void
test_deviation(void **state)
{
    /* invalid cardinality */
    TEST_MOD(UTEST_LYCTX, "module d1 { namespace urn:d1; prefix d; deviation /d:target { deviate not-supported; description \"a\"; description \"b\"; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"d1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"description\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module d2 { namespace urn:d2; prefix d; deviation /d:target { deviate not-supported; reference \"a\"; reference \"b\"; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"d2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"reference\".", NULL, 1);

    /* missing mandatory substatement */
    TEST_MOD(UTEST_LYCTX, "module d3 { namespace urn:d3; prefix d; deviation /d:target { description \"text\"; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"d3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory keyword \"deviate\" as a child of \"deviation\".", NULL, 1);

    /* invalid substatement */
    TEST_MOD(UTEST_LYCTX, "module d4 { namespace urn:d4; prefix d; deviation /d:target { deviate not-supported; status obsolete; } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"d4\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"status\" as a child of \"deviation\".", NULL, 1);
}

static void
test_deviate(void **state)
{
#define TEST_DEVIATE_ERR(MEMBER, ERR_MSG) \
    TEST_MOD(UTEST_LYCTX, "module dev { yang-version 1.1; namespace urn:cont; prefix c; deviation /c:t { " MEMBER " } }", LY_EVALID); \
    CHECK_LOG_CTX("Parsing module \"dev\" failed.", NULL, 0);\
    CHECK_LOG_CTX(ERR_MSG, NULL, 1)

    /* invalid cardinality */
    TEST_DEVIATE_ERR("deviate add { config true; config false; }", "Duplicate keyword \"config\".");
    TEST_DEVIATE_ERR("deviate add { mandatory true; mandatory false; }", "Duplicate keyword \"mandatory\".");
    TEST_DEVIATE_ERR("deviate add { max-elements 1; max-elements 2; }", "Duplicate keyword \"max-elements\".");
    TEST_DEVIATE_ERR("deviate add { min-elements 1; min-elements 2; }", "Duplicate keyword \"min-elements\".");
    TEST_DEVIATE_ERR("deviate add { units \"kilometers\"; units \"miles\"; }", "Duplicate keyword \"units\".");

#define TEST_NOT_SUP(DEV, STMT, VALUE) \
    TEST_DEVIATE_ERR("deviate "DEV" {"STMT" "VALUE";}", "Deviate \""DEV"\" does not support keyword \""STMT"\".");

    /* invalid substatements */
    TEST_NOT_SUP("not-supported", "units", "meters");
    TEST_NOT_SUP("not-supported", "must", "1");
    TEST_NOT_SUP("not-supported", "unique", "x");
    TEST_NOT_SUP("not-supported", "default", "a");
    TEST_NOT_SUP("not-supported", "config", "true");
    TEST_NOT_SUP("not-supported", "mandatory", "true");
    TEST_NOT_SUP("not-supported", "min-elements", "1");
    TEST_NOT_SUP("not-supported", "max-elements", "2");
    TEST_NOT_SUP("not-supported", "type", "string");
    TEST_NOT_SUP("add", "type", "string");
    TEST_NOT_SUP("delete", "config", "true");
    TEST_NOT_SUP("delete", "mandatory", "true");
    TEST_NOT_SUP("delete", "min-elements", "1");
    TEST_NOT_SUP("delete", "max-elements", "2");
    TEST_NOT_SUP("delete", "type", "string");
    TEST_NOT_SUP("replace", "must", "1");
    TEST_NOT_SUP("replace", "unique", "a");
#undef TEST_DEVIATE_ERR
}

static void
test_container(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module cont { yang-version 1.1; namespace urn:cont; prefix c; container test {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} }", LY_EVALID); \
    CHECK_LOG_CTX("Parsing module \"cont\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("presence", "true", "false");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module cont {"
            "  yang-version 1.1;"
            "  namespace \"urn:cont\";"
            "  prefix c;"
            "  feature f;"
            "  extension ext;"
            "  container test {"
            "    action x;"
            "    anydata any;"
            "    anyxml anyxml;"
            "    choice ch;"
            "    config false;"
            "    container c;"
            "    description test;"
            "    grouping g;"
            "    if-feature f;"
            "    leaf l {type string;}"
            "    leaf-list ll {type string;}"
            "    list li;"
            "    must 'expr';"
            "    notification not;"
            "    presence true;"
            "    reference test;"
            "    status current;"
            "    typedef t {type int8;}"
            "    uses g;"
            "    when 'true()';"
            "    c:ext;"
            "  }"
            "}",
            LY_SUCCESS);

    /* invalid */
    TEST_MOD(UTEST_LYCTX, "module cont2 { yang-version 1.1; namespace urn:cont2; prefix c2; container test {augment /root;} }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"cont2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"augment\" as a child of \"container\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module cont2 { yang-version 1.1; namespace urn:cont2; prefix c2; container test {nonsence true;} }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"cont2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid character sequence \"nonsence\", expected a keyword.", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module cont2 { namespace urn:cont2; prefix c2; container test {action x;} }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"cont2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"action\" as a child of \"container\" - "
            "the statement is allowed only in YANG 1.1 modules.", NULL, 1);
}

static void
test_leaf(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module leaf1 { namespace urn:leaf1; prefix l1; leaf test {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} }", LY_EVALID); \
    CHECK_LOG_CTX("Parsing module \"leaf1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("config", "true", "false");
    TEST_DUP("default", "x", "y");
    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("type", "int8", "uint8");
    TEST_DUP("units", "text1", "text2");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content - without mandatory which is mutual exclusive with default */
    TEST_MOD(UTEST_LYCTX,
            "module leaf1 {"
            "  namespace \"urn:leaf1\";"
            "  prefix l1;"
            "  feature f;"
            "  extension ext;"
            "  leaf test {"
            "    config false;"
            "    default \"xxx\";"
            "    description test;"
            "    if-feature f;"
            "    must 'expr';"
            "    reference test;"
            "    status current;"
            "    type string;"
            "    units yyy;"
            "    when 'true()';"
            "    l1:ext;"
            "  }"
            "}",
            LY_SUCCESS);

    /* full content - now with mandatory */
    TEST_MOD(UTEST_LYCTX,
            "module leaf2 {"
            "  namespace \"urn:leaf2\";"
            "  prefix l2;"
            "  leaf test {"
            "    mandatory true;"
            "    type string;"
            "  }"
            "}",
            LY_SUCCESS);

    /* invalid */
    TEST_MOD(UTEST_LYCTX, "module leaf3 { namespace urn:leaf3; prefix l3; leaf test {description \"missing type\";} }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"leaf3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory keyword \"type\" as a child of \"leaf\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module leaf3 { namespace urn:leaf3; prefix l3; leaf test { type iid { path qpud wrong {} }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"leaf3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid character sequence \"wrong\", expected a keyword.", NULL, 1);
}

static void
test_leaflist(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module leaflist1 { yang-version 1.1; namespace urn:leaflist1; prefix ll1; leaf-list test {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} }", LY_EVALID); \
    CHECK_LOG_CTX("Parsing module \"leaflist1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("max-elements", "10", "20");
    TEST_DUP("min-elements", "10", "20");
    TEST_DUP("ordered-by", "user", "system");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("type", "int8", "uint8");
    TEST_DUP("units", "text1", "text2");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content - without min-elements which is mutual exclusive with default */
    TEST_MOD(UTEST_LYCTX,
            "module leaflist1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:leaflist1\";"
            "  prefix ll1;"
            "  feature f;"
            "  extension ext;"
            "  leaf-list test {"
            "    config false;"
            "    default \"xxx\";"
            "    description test;"
            "    if-feature f;"
            "    max-elements 10;"
            "    must 'expr';"
            "    ordered-by user;"
            "    reference test;"
            "    status current;"
            "    type string;"
            "    units zzz;"
            "    when 'true()';"
            "    ll1:ext;"
            "  }"
            "}",
            LY_SUCCESS);

    /* full content - now with min-elements */
    TEST_MOD(UTEST_LYCTX,
            "module leaflist2 {"
            "  yang-version 1.1;"
            "  namespace \"urn:leaflist2\";"
            "  prefix ll2;"
            "  leaf-list test {"
            "    type string;"
            "  }"
            "}",
            LY_SUCCESS);

    /* invalid */
    TEST_MOD(UTEST_LYCTX, "module leaflist3 { yang-version 1.1; namespace urn:leaflist3; prefix ll3; leaf-list test {description \"missing type\";} }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"leaflist3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory keyword \"type\" as a child of \"leaf-list\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module leaflist3 { namespace urn:leaflist3; prefix ll3; leaf-list test {default xx; type string;}}", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"leaflist3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"default\" as a child of \"leaf-list\" - the statement is allowed only in YANG 1.1 modules.", NULL, 1);
}

static void
test_list(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module list1 { yang-version 1.1; namespace urn:list1; prefix l1; list test {" MEMBER" "VALUE1";"MEMBER" "VALUE2"; key \"name\"; leaf name {type string;}} }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"list1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("key", "one", "two");
    TEST_DUP("max-elements", "10", "20");
    TEST_DUP("min-elements", "10", "20");
    TEST_DUP("ordered-by", "user", "system");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module list1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:list1\";"
            "  prefix l1;"
            "  feature f;"
            "  extension ext;"
            "  list test {"
            "    action x;"
            "    anydata any;"
            "    anyxml anyxml;"
            "    choice ch;"
            "    config false;"
            "    container c;"
            "    description test;"
            "    grouping g;"
            "    if-feature f;"
            "    key l;"
            "    leaf l {type string;}"
            "    leaf xxx {type string;} "
            "    leaf yyy {type string;} "
            "    leaf-list ll {type string;}"
            "    list li;"
            "    max-elements 10;"
            "    min-elements 1;"
            "    must 'expr';"
            "    notification not;"
            "    ordered-by system;"
            "    reference test;"
            "    status current;"
            "    typedef t {type int8;} unique xxx;"
            "    unique yyy;"
            "    uses g;"
            "    when 'true()';"
            "    l1:ext;"
            "  }"
            "}",
            LY_SUCCESS);

    /* invalid content */
    TEST_MOD(UTEST_LYCTX, "module list2 { namespace \"urn:list2\"; prefix l2; list test {action x; key \"name\"; leaf name {type string} } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"list2\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Invalid keyword \"action\" as a child of \"list\" - the statement is allowed only in YANG 1.1 modules.", NULL, 1);
}

static void
test_choice(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module choice1 { yang-version 1.1; namespace \"urn:choice1\"; prefix c1; choice test {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"choice1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("config", "true", "false");
    TEST_DUP("default", "a", "b");
    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content - without default due to a collision with mandatory */
    TEST_MOD(UTEST_LYCTX,
            "module choice1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:choice1\";"
            "  prefix c1;"
            "  feature f;"
            "  extension ext;"
            "  choice test {"
            "    anydata any;"
            "    anyxml anyxml;"
            "    case c;"
            "    choice ch;"
            "    config false;"
            "    container cont;"
            "    description test;"
            "    if-feature f;"
            "    leaf l {type string;}"
            "    leaf-list ll {type string;}"
            "    list li;"
            "    mandatory true;"
            "    reference test;"
            "    status current;"
            "    when 'true()';"
            "    c1:ext;"
            "  }"
            "}",
            LY_SUCCESS);

    /* full content - the default missing from the previous node */
    TEST_MOD(UTEST_LYCTX,
            "module choice2 {"
            "  yang-version 1.1;"
            "  namespace \"urn:choice2\";"
            "  prefix c2;"
            "  choice test {"
            "    default c;"
            "    case c;"
            "  }"
            "}",
            LY_SUCCESS);
}

static void
test_case(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module case1 { yang-version 1.1; namespace \"urn:case1\"; prefix c1; choice ch {case test {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} } }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"case1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module case1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:case1\";"
            "  prefix c1;"
            "  feature f;"
            "  extension ext;"
            "  grouping grp;"
            "  choice ch {"
            "   case test {"
            "    anydata any;"
            "    anyxml anyxml;"
            "    choice cho;"
            "    container cont;"
            "    description test;"
            "    if-feature f;"
            "    leaf l {type string;}"
            "    leaf-list ll {type string;}"
            "    list li { config false; }"
            "    reference test;"
            "    status current;"
            "    uses grp;"
            "    when 'true()';"
            "    c1:ext;"
            "    }"
            "  }"
            "}",
            LY_SUCCESS);

    /* invalid content */
    TEST_MOD(UTEST_LYCTX,
            "module case1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:case1\";"
            "  prefix c1;"
            "  feature f;"
            "  extension ext;"
            "  choice ch {"
            "   case test {"
            "    config true;"
            "    }"
            "  }"
            "}",
            LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"case1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"case\".", NULL, 1);
}

static void
test_anydata(void **state)
{
    #define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module anydata1 { yang-version 1.1; namespace \"urn:anydata1\"; prefix a1; anydata test {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"anydata1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module anydata1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:anydata1\";"
            "  prefix a1;"
            "  feature f;"
            "  extension ext;"
            "  anydata test {"
            "    config true;"
            "    description test;"
            "    if-feature f;"
            "    mandatory true;"
            "    must 'expr';"
            "    reference test;"
            "    status current;"
            "    when 'true()';"
            "    a1:ext;"
            "  }"
            "}",
            LY_SUCCESS);
}

static void
test_anyxml(void **state)
{
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module anyxml1 { yang-version 1.1; namespace \"urn:anyxml1\"; prefix a1; anyxml test {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"anyxml1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module anyxml1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:anyxml1\";"
            "  prefix a1;"
            "  feature f;"
            "  extension ext;"
            "  anyxml test {"
            "    config true;"
            "    description test;"
            "    if-feature f;"
            "    mandatory true;"
            "    must 'expr';"
            "    reference test;"
            "    status current;"
            "    when 'true()';"
            "    a1:ext;"
            "  }"
            "}",
            LY_SUCCESS);
}

static void
test_grouping(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module grouping1 { yang-version 1.1; namespace \"urn:group1\"; prefix g1; grouping test {" MEMBER" "VALUE1";"MEMBER" "VALUE2"; leaf name {type string} } }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"grouping1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module grouping1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:grouping1\";"
            "  prefix g1;"
            "  feature f;"
            "  extension ext;"
            "  grouping test {"
            "    anydata any;"
            "    anyxml anyxml;"
            "    choice ch;"
            "    container cont;"
            "    description test;"
            "    grouping g;"
            "    leaf l {type string;}"
            "    leaf-list ll {type string;}"
            "    list li { config false; }"
            "    notification not;"
            "    reference test;"
            "    status current;"
            "    typedef t {type int8;}uses g;"
            "    g1:ext;"
            "    }"
            "}",
            LY_SUCCESS);

    /* invalid content */
    TEST_MOD(UTEST_LYCTX,
            "module grouping2 {"
            "  yang-version 1.1;"
            "  namespace \"urn:grouping2\";"
            "  prefix g2;"
            "  grouping test {"
            "    config true"
            "    }"
            "}",
            LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"grouping2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"grouping\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX,
            "module grouping2 {"
            "  yang-version 1.1;"
            "  namespace \"urn:grouping2\";"
            "  prefix g2;"
            "  grouping test {"
            "    must 'expr'"
            "    }"
            "}",
            LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"grouping2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"must\" as a child of \"grouping\".", NULL, 1);
}

static void
test_action(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module act1 { yang-version 1.1; namespace \"urn:act1\"; prefix a; container top { action test { " MEMBER " " VALUE1 "; " MEMBER " " VALUE2 "; } } }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"act1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    TEST_MOD(UTEST_LYCTX, "module act1 { yang-version 1.1; namespace \"urn:act-in\"; prefix a; container top { action func { input { leaf l1 {type empty;} } input { leaf l2 {type empty;} } } } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"act1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"input\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module act1 { yang-version 1.1; namespace \"urn:act-out\"; prefix a; container top { action func { output { leaf l1 {type empty;} } output { leaf l2 {type empty;} } } } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"act1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"output\".", NULL, 1);

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module act1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:act1\";"
            "  prefix a;"
            "  feature f;"
            "  extension ext;"
            "  container top {"
            "    action test {"
            "      description test;"
            "      grouping grp {}"
            "      if-feature f;"
            "      reference test;"
            "      status current;"
            "      typedef mytype {type int8;}"
            "      a:ext;"
            "      input {"
            "        anydata a1;"
            "        anyxml a2;"
            "        choice ch { leaf x { type empty; } }"
            "        container c;"
            "        grouping grp-in {}"
            "        leaf l {type int8;}"
            "        leaf-list ll {type int8;}"
            "        list li { key k; leaf k {type empty;} }"
            "        must 'true()';"
            "        typedef mytypei {type int8;}"
            "        uses grp-in;"
            "        a:ext;"
            "      }"
            "      output {"
            "        anydata a1;"
            "        anyxml a2;"
            "        choice ch { leaf x { type empty; } }"
            "        container c;"
            "        grouping grp-out {}"
            "        leaf l {type int8;}"
            "        leaf-list ll {type int8;}"
            "        list li { key k; leaf k {type empty;} }"
            "        must 'true()';"
            "        typedef mytypeo {type int8;}"
            "        uses grp-out;"
            "        a:ext;"
            "      }"
            "    }"
            "  }"
            "}",
            LY_SUCCESS);

    /* invalid content */
    TEST_MOD(UTEST_LYCTX,
            "module act2 {"
            "  yang-version 1.1;"
            "  namespace \"urn:act2\";"
            "  prefix a;"
            "  container top {"
            "    action func { config true; }"
            "  }"
            "}",
            LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"act2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"action\".", NULL, 1);
}

static void
test_notification(void **state)
{
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module notif1 { yang-version 1.1; namespace \"urn:notif1\"; prefix n; notification ntf { " MEMBER " " VALUE1 "; " MEMBER " " VALUE2 "; } }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"notif1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module notif1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:notif1\";"
            "  prefix n;"
            "  feature f;"
            "  extension ext;"
            "  container top {"
            "    notification ntf {"
            "      anydata a1;"
            "      anyxml a2;"
            "      choice ch { leaf x { type empty; } }"
            "      container c;"
            "      description test;"
            "      grouping grp {}"
            "      if-feature f;"
            "      leaf l {type int8;}"
            "      leaf-list ll {type int8;}"
            "      list li { key k; leaf k {type empty;} }"
            "      must 'true()';"
            "      reference test;"
            "      status current;"
            "      typedef mytype {type int8;}"
            "      uses grp;"
            "      n:ext;"
            "    }"
            "  }"
            "}",
            LY_SUCCESS);

    /* invalid content */
    TEST_MOD(UTEST_LYCTX,
            "module notif2 {"
            "  yang-version 1.1;"
            "  namespace \"urn:notif2\";"
            "  prefix n;"
            "  notification ntf { config true; }" /* Notifications cannot have config true */
            "}",
            LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"notif2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"notification\".", NULL, 1);
}

static void
test_uses(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module uses1 { yang-version 1.1; namespace \"urn:uses1\"; prefix u; grouping g; container c { uses g { " MEMBER " " VALUE1 "; " MEMBER " " VALUE2 "; } } }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"uses1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module uses1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:uses1\";"
            "  prefix u;"
            "  feature f;"
            "  extension ext;"
            "  "
            "  /* We MUST build the tree inside the grouping so augment/refine can find their targets! */"
            "  grouping grpref {"
            "    container some {"
            "      container node;"
            "      container other {"
            "        leaf node { type string; }"
            "      }"
            "    }"
            "  }"
            "  "
            "  container top {"
            "    uses grpref {"
            "      augment \"some/node\" { leaf new-leaf { type empty; } }"
            "      description test;"
            "      if-feature f;"
            "      reference test;"
            "      refine \"some/other/node\" { description \"refined\"; }"
            "      status current;"
            "      when 'true()';"
            "      u:ext;"
            "    }"
            "  }"
            "}",
            LY_SUCCESS);
}

static void
test_augment(void **state)
{
    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_MOD(UTEST_LYCTX, "module aug1 { yang-version 1.1; namespace \"urn:aug1\"; prefix a; container target { container nodeid; } augment \"/a:target/a:nodeid\" { " MEMBER " " VALUE1 "; " MEMBER " " VALUE2 "; } }", LY_EVALID);\
    CHECK_LOG_CTX("Parsing module \"aug1\" failed.", NULL, 0);\
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", NULL, 1);

    TEST_DUP("description", "\"text1\"", "\"text2\"");
    TEST_DUP("reference", "\"1\"", "\"2\"");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "'true()'", "'false()'");
#undef TEST_DUP

    /* full content */
    TEST_MOD(UTEST_LYCTX,
            "module aug1 {"
            "  yang-version 1.1;"
            "  namespace \"urn:aug1\";"
            "  prefix a;"
            "  feature f;"
            "  extension ext;"
            "  grouping g;"
            "  "
            "  container target {"
            "    container nodeid;"
            "    choice ch;"
            "  }"
            "  "
            "  /* Augmenting a container */"
            "  augment \"/a:target/a:nodeid\" {"
            "    action x;"
            "    anydata any;"
            "    anyxml anyxml;"
            "    choice ch { leaf ch-l { type empty; } }"
            "    container c;"
            "    description test;"
            "    if-feature f;"
            "    leaf l {type string;}"
            "    leaf-list ll {type string;}"
            "    list li { key k; leaf k {type empty;} }"
            "    notification not;"
            "    reference test;"
            "    status current;"
            "    uses g;"
            "    when 'true()';"
            "    a:ext;"
            "  }"
            "  "
            "  augment \"/a:target/a:ch\" {"
            "    case cs { leaf cs-l { type empty; } }"
            "  }"
            "}",
            LY_SUCCESS);
}

static void
test_when(void **state)
{
    TEST_MOD(UTEST_LYCTX, "module when1 { yang-version 1.1; namespace \"urn:when1\"; prefix w; leaf l { type empty; when 'true()' { description text1; description text2; } } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"when1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"description\".", NULL, 1);

    TEST_MOD(UTEST_LYCTX, "module when1 { yang-version 1.1; namespace \"urn:when1\"; prefix w; leaf l { type empty; when 'true()' { reference 1; reference 2; } } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"when1\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate keyword \"reference\".", NULL, 1);
}

static void
test_value(void **state)
{
    TEST_MOD(UTEST_LYCTX, "module val1 { yang-version 1.1; namespace \"urn:val1\"; prefix v; leaf l { type enumeration { enum a { value -0; } } } }", LY_SUCCESS);

    TEST_MOD(UTEST_LYCTX, "module val2 { yang-version 1.1; namespace \"urn:val2\"; prefix v; leaf l { type bits { bit a { position -0; } } } }", LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"val2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-0\" of \"position\".", NULL, 1);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_helpers),
        UTEST(test_comments),
        UTEST(test_arg),
        UTEST(test_stmts),
        UTEST(test_minmax),
        UTEST(test_valid_module),
        UTEST(test_module),
        UTEST(test_deviation),
        UTEST(test_deviate),
        UTEST(test_container),
        UTEST(test_leaf),
        UTEST(test_leaflist),
        UTEST(test_list),
        UTEST(test_choice),
        UTEST(test_case),
        UTEST(test_anydata),
        UTEST(test_anyxml),
        UTEST(test_action),
        UTEST(test_notification),
        UTEST(test_grouping),
        UTEST(test_uses),
        UTEST(test_augment),
        UTEST(test_when),
        UTEST(test_value),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
