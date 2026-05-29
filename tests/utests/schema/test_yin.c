/**
 * @file test_yin.c
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for YIN parser and printer
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "in.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema.h"

#define TEST_1_CHECK_LYSP_EXT_INSTANCE(NODE, INSUBSTMT, ARGUMENT)\
    CHECK_LYSP_EXT_INSTANCE((NODE), (ARGUMENT), 1, INSUBSTMT, 0, "myext:c-define", LY_VALUE_XML)

static void
test_yin_match_keyword(void **state)
{
    const char *yin_data;

    yin_data =
            "<module name=\"test\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t1\"/>"
            "  <prefix value=\"t1\"/>"
            "  <anydatax/>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Unexpected sub-element \"anydatax\" of \"module\" element.", NULL, 1);

    yin_data =
            "<module name=\"test\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t1\"/>"
            "  <prefix value=\"t1\"/>"
            "  <asdasd/>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Unexpected sub-element \"asdasd\" of \"module\" element.", NULL, 1);

    yin_data =
            "<module name=\"test\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t1\"/>"
            "  <prefix value=\"t1\"/>"
            "  </>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Identifier \"></module>\" starts with an invalid character.", NULL, 1);
}

static void
test_yin_parse_content(void **state)
{
    const char *yin_data;

    yin_data =
            "<module name=\"test\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:err1\"/>"
            "  <prefix value=\"e1\"/>"
            "  <description>"
            "    <text>First text</text>"
            "    <text>Second text (invalid duplication)</text>"
            "  </description>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Redefinition of \"text\" sub-element in \"description\" element.", NULL, 1);

    yin_data =
            "<module name=\"test\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:err1\"/>"
            "  <prefix value=\"e1\"/>"
            "  <extension name=\"ext\"/>"
            "  <description>"
            "    <e1:ext/>"
            "    <text>Desc</text>"
            "  </description>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid order of module's sub-elements \"description\" can't appear after \"extension\".", NULL, 1);

    yin_data =
            "<module name=\"test\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:err1\"/>"
            "  <prefix value=\"e1\"/>"
            "  <description>"
            "  </description>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"test\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory sub-element \"text\" of \"description\" element.", NULL, 1);

}

static void
test_validate_value(void **state)
{
    char buffer[512];
    const char *yin_ident =
            "<module name=\"%s\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t1\"/>"
            "  <prefix value=\"t1\"/>"
            "</module>";
    const char *yin_str_empty =
            "<module name=\"t2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t2\"/>"
            "  <prefix value=\"t2\"/>"
            "  <description><text></text></description>"
            "</module>";
    const char *yin_pref =
            "<module name=\"t3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t3\"/>"
            "  <prefix value=\"pre\"/>"
            "  <identity name=\"b\"/>"
            "  <identity name=\"test-id\">"
            "    <base name=\"%s\"/>"
            "  </identity>"
            "</module>";

    snprintf(buffer, sizeof(buffer), yin_ident, "#invalid");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Invalid identifier first character '#' (0x0023).", NULL, 1);

    snprintf(buffer, sizeof(buffer), yin_ident, "pre:b");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", NULL, 1);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_str_empty, LYS_IN_YIN, NULL));

    snprintf(buffer, sizeof(buffer), yin_pref, "pre:pre:b");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"t3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", NULL, 1);

    snprintf(buffer, sizeof(buffer), yin_pref, "pre:b");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
}

static void
test_valid_module(void **state)
{
    struct lys_module *mod;
    char *printed;
    const char *links_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"links\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:mod2=\"urn:module2\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:module2\"/>\n"
            "  <prefix value=\"mod2\"/>\n"
            "  <identity name=\"just-another-identity\"/>\n"
            "  <grouping name=\"rgroup\">\n"
            "    <leaf name=\"rg1\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"rg2\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </grouping>\n"
            "  <leaf name=\"one-leaf\">\n"
            "    <type name=\"string\"/>\n"
            "  </leaf>\n"
            "  <list name=\"list-for-augment\">\n"
            "    <key value=\"keyleaf\"/>\n"
            "    <leaf name=\"keyleaf\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"just-leaf\">\n"
            "      <type name=\"int32\"/>\n"
            "    </leaf>\n"
            "  </list>\n"
            "  <leaf name=\"rleaf\">\n"
            "    <type name=\"string\"/>\n"
            "  </leaf>\n"
            "  <leaf-list name=\"llist\">\n"
            "    <type name=\"string\"/>\n"
            "    <min-elements value=\"0\"/>\n"
            "    <max-elements value=\"100\"/>\n"
            "    <ordered-by value=\"user\"/>\n"
            "  </leaf-list>\n"
            "</module>\n";
    const char *statements_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"statements\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:mod=\"urn:module\"\n"
            "        xmlns:mod2=\"urn:module2\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:module\"/>\n"
            "  <prefix value=\"mod\"/>\n"
            "  <import module=\"links\">\n"
            "    <prefix value=\"mod2\"/>\n"
            "  </import>\n"
            "  <extension name=\"ext\"/>\n"
            "  <identity name=\"random-identity\">\n"
            "    <base name=\"mod2:just-another-identity\"/>\n"
            "    <base name=\"another-identity\"/>\n"
            "  </identity>\n"
            "  <identity name=\"another-identity\">\n"
            "    <base name=\"mod2:just-another-identity\"/>\n"
            "  </identity>\n"
            "  <typedef name=\"percent\">\n"
            "    <type name=\"uint8\">\n"
            "      <range value=\"0 .. 100\"/>\n"
            "    </type>\n"
            "    <units name=\"percent\"/>\n"
            "  </typedef>\n"
            "  <list name=\"list1\">\n"
            "    <key value=\"a\"/>\n"
            "    <leaf name=\"a\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"x\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"y\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </list>\n"
            "  <container name=\"ice-cream-shop\">\n"
            "    <container name=\"employees\">\n"
            "      <when condition=\"/list1/x\"/>\n"
            "      <list name=\"employee\">\n"
            "        <key value=\"id\"/>\n"
            "        <unique tag=\"name\"/>\n"
            "        <config value=\"true\"/>\n"
            "        <min-elements value=\"0\">\n"
            "          <mod:ext/>\n"
            "        </min-elements>\n"
            "        <max-elements value=\"unbounded\"/>\n"
            "        <leaf name=\"id\">\n"
            "          <type name=\"uint64\"/>\n"
            "          <mandatory value=\"true\"/>\n"
            "        </leaf>\n"
            "        <leaf name=\"name\">\n"
            "          <type name=\"string\"/>\n"
            "        </leaf>\n"
            "        <leaf name=\"age\">\n"
            "          <type name=\"uint32\"/>\n"
            "        </leaf>\n"
            "      </list>\n"
            "    </container>\n"
            "  </container>\n"
            "  <container name=\"random\">\n"
            "    <grouping name=\"group\">\n"
            "      <leaf name=\"g1\">\n"
            "        <type name=\"percent\"/>\n"
            "        <mandatory value=\"false\"/>\n"
            "      </leaf>\n"
            "      <leaf name=\"g2\">\n"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "    </grouping>\n"
            "    <choice name=\"switch\">\n"
            "      <case name=\"a\">\n"
            "        <leaf name=\"aleaf\">\n"
            "          <type name=\"string\"/>\n"
            "          <default value=\"aaa\"/>\n"
            "        </leaf>\n"
            "      </case>\n"
            "      <case name=\"c\">\n"
            "        <leaf name=\"cleaf\">\n"
            "          <type name=\"string\"/>\n"
            "        </leaf>\n"
            "      </case>\n"
            "    </choice>\n"
            "    <anyxml name=\"xml-data\"/>\n"
            "    <anydata name=\"any-data\"/>\n"
            "    <leaf-list name=\"leaflist\">\n"
            "      <type name=\"string\"/>\n"
            "      <min-elements value=\"0\"/>\n"
            "      <max-elements value=\"20\"/>\n"
            "    </leaf-list>\n"
            "    <uses name=\"group\"/>\n"
            "    <uses name=\"mod2:rgroup\"/>\n"
            "    <leaf name=\"lref\">\n"
            "      <type name=\"leafref\">\n"
            "        <path value=\"/mod2:one-leaf\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"iref\">\n"
            "      <type name=\"identityref\">\n"
            "        <base name=\"mod2:just-another-identity\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "  </container>\n"
            "  <augment target-node=\"/random\">\n"
            "    <leaf name=\"aug-leaf\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </augment>\n"
            "  <notification name=\"notif\"/>\n"
            "  <deviation target-node=\"/mod:ice-cream-shop/mod:employees/mod:employee/mod:age\">\n"
            "    <deviate value=\"not-supported\">\n"
            "      <mod:ext/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "  <deviation target-node=\"/mod:list1\">\n"
            "    <deviate value=\"add\">\n"
            "      <mod:ext/>\n"
            "      <must condition=\"1\"/>\n"
            "      <must condition=\"2\"/>\n"
            "      <unique tag=\"x\"/>\n"
            "      <unique tag=\"y\"/>\n"
            "      <config value=\"true\"/>\n"
            "      <min-elements value=\"1\"/>\n"
            "      <max-elements value=\"2\"/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "  <deviation target-node=\"/mod:ice-cream-shop/mod:employees/mod:employee\">\n"
            "    <deviate value=\"delete\">\n"
            "      <unique tag=\"name\"/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "  <deviation target-node=\"/mod:random/mod:leaflist\">\n"
            "    <deviate value=\"replace\">\n"
            "      <type name=\"uint32\"/>\n"
            "      <min-elements value=\"10\"/>\n"
            "      <max-elements value=\"15\"/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "</module>\n";

    UTEST_ADD_MODULE(links_yin, LYS_IN_YIN, NULL, NULL);
    UTEST_ADD_MODULE(statements_yin, LYS_IN_YIN, NULL, &mod);
    lys_print_mem(&printed, mod, LYS_OUT_YIN, 0);
    assert_string_equal(printed, statements_yin);
    free(printed);
}

static void
test_print_module(void **state)
{
    struct lys_module *mod;

    char *orig = malloc(8096);

    strcpy(orig,
            "module all {\n"
            "    yang-version 1.1;\n"
            "    namespace \"urn:all\";\n"
            "    prefix all_mod;\n\n"
            "    import ietf-yang-types {\n"
            "        prefix yt;\n"
            "        revision-date 2025-12-22;\n"
            "        description\n"
            "            \"YANG types\";\n"
            "        reference\n"
            "            \"RFC reference\";\n"
            "    }\n\n"
            "    feature feat1 {\n"
            "        if-feature \"feat2\";\n"
            "        status obsolete;\n"
            "    }\n\n"
            "    feature feat2;\n"
            "    feature feat3;\n\n"
            "    identity ident2 {\n"
            "        base ident1;\n"
            "    }\n\n"
            "    identity ident1;\n\n"
            "    typedef tdef1 {\n"
            "        type tdef2 {\n"
            "            length \"3..9 | 30..40\";\n"
            "            pattern \"[ac]*\";\n"
            "        }\n"
            "        units \"none\";\n"
            "        default \"aaa\";\n"
            "    }\n\n"
            "    typedef tdef2 {\n"
            "        type string {\n"
            "            length \"2..10 | 20..50\";\n"
            "            pattern \"[ab]*\";\n"
            "        }\n"
            "    }\n\n"
            "    grouping group1 {\n"
            "        leaf leaf1 {\n"
            "            type int8;\n"
            "        }\n"
            "    }\n\n"
            "    container cont1 {\n"
            "        leaf leaf2 {\n"
            "            if-feature \"feat1\";\n"
            "            type int16;\n"
            "            status obsolete;\n"
            "        }\n\n"
            "        uses group1 {\n"
            "            if-feature \"feat2\";\n"
            "            refine \"leaf1\" {\n"
            "                if-feature \"feat3\";\n"
            "                must \"24 - 4 = number('20')\";\n"
            "                default \"25\";\n"
            "                config true;\n"
            "                mandatory false;\n"
            "                description\n"
            "                    \"dsc\";\n"
            "                reference\n"
            "                    \"none\";\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf3 {\n"
            "            type int32;\n"
            "        }\n\n"
            "        leaf leaf4 {\n"
            "            type int64 {\n"
            "                range \"1000 .. 50000\" {\n"
            "                    error-message\n"
            "                        \"Special error message.\";\n"
            "                    error-app-tag \"special-tag\";\n"
            "                }\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf5 {\n"
            "            type uint8;\n"
            "        }\n\n"
            "        leaf leaf6 {\n"
            "            type uint16;\n"
            "        }\n\n"
            "        leaf leaf7 {\n"
            "            type uint32;\n"
            "        }\n\n"
            "        leaf leaf8 {\n"
            "            type uint64;\n"
            "        }\n\n"
            "        choice choic1 {\n"
            "            default \"leaf9b\";\n"
            "            leaf leaf9a {\n"
            "                type decimal64 {\n"
            "                    fraction-digits 9;\n"
            "                }\n"
            "            }\n\n"
            "            leaf leaf9b {\n"
            "                type boolean;\n"
            "                default \"false\";\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf10 {\n"
            "            type boolean;\n"
            "        }\n\n");
    strcpy(orig + strlen(orig),
            "        leaf leaf11 {\n"
            "            type enumeration {\n"
            "                enum \"one\";\n"
            "                enum \"two\";\n"
            "                enum \"five\" {\n"
            "                    value 5;\n"
            "                }\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf12 {\n"
            "            type bits {\n"
            "                bit flag0 {\n"
            "                    position 0;\n"
            "                }\n"
            "                bit flag1;\n"
            "                bit flag2 {\n"
            "                    position 2;\n"
            "                }\n"
            "                bit flag3 {\n"
            "                    position 3;\n"
            "                }\n"
            "            }\n"
            "            default \"flag0 flag3\";\n"
            "        }\n\n"
            "        leaf leaf13 {\n"
            "            type binary;\n"
            "        }\n\n"
            "        leaf leaf14 {\n"
            "            type leafref {\n"
            "                path \"/cont1/leaf17\";\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf15 {\n"
            "            type empty;\n"
            "        }\n\n"
            "        leaf leaf16 {\n"
            "            type union {\n"
            "                type instance-identifier {\n"
            "                    require-instance true;\n"
            "                }\n"
            "                type int8;\n"
            "            }\n"
            "        }\n\n"
            "        list list1 {\n"
            "            key \"leaf18\";\n"
            "            unique \"leaf19\";\n"
            "            min-elements 1;\n"
            "            max-elements 20;\n"
            "            leaf leaf18 {\n"
            "                type string;\n"
            "            }\n\n"
            "            leaf leaf19 {\n"
            "                type uint32;\n"
            "            }\n\n"
            "            anyxml axml1;\n"
            "            anydata adata1;\n\n"
            "            action act1 {\n"
            "                input {\n"
            "                    leaf leaf24 {\n"
            "                        type string;\n"
            "                    }\n"
            "                }\n\n"
            "                output {\n"
            "                    leaf leaf25 {\n"
            "                        type string;\n"
            "                    }\n"
            "                }\n"
            "            }\n\n"
            "            notification notif1 {\n"
            "                leaf leaf26 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n\n"
            "        leaf-list llist1 {\n"
            "            type tdef1;\n"
            "            ordered-by user;\n"
            "        }\n\n"
            "        list list2 {\n"
            "            key \"leaf27 leaf28\";\n"
            "            leaf leaf27 {\n"
            "                type uint8;\n"
            "            }\n\n"
            "            leaf leaf28 {\n"
            "                type uint8;\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf29 {\n"
            "            type instance-identifier;\n"
            "        }\n\n"
            "        container must-deviations-container {\n"
            "            presence \"Allows deviations on the leaf\";\n"
            "            leaf leaf30 {\n"
            "                type string;\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf23 {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n\n"
            "    augment \"/cont1\" {\n"
            "        leaf leaf17 {\n"
            "            type string;\n"
            "        }\n"
            "    }\n\n"
            "    rpc rpc1 {\n"
            "        input {\n"
            "            leaf leaf20 {\n"
            "                type tdef1;\n"
            "            }\n"
            "        }\n\n"
            "        output {\n"
            "            container cont2 {\n"
            "                leaf leaf21 {\n"
            "                    type empty;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n\n"
            "    container test-when {\n"
            "        leaf when-check {\n"
            "            type boolean;\n"
            "        }\n\n"
            "        leaf gated-data {\n"
            "            when \"../when-check = 'true'\";\n"
            "            type uint16;\n"
            "        }\n"
            "    }\n\n"
            "    extension c-define {\n"
            "        description\n"
            "            \"Takes as an argument a name string.\n"
            "            Makes the code generator use the given name\n"
            "            in the #define.\";\n"
            "        argument \"name\";\n"
            "    }\n"
            "}\n");

    char *ori_res = malloc(8096);

    strcpy(ori_res,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"all\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:all_mod=\"urn:all\"\n"
            "        xmlns:yt=\"urn:ietf:params:xml:ns:yang:ietf-yang-types\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:all\"/>\n"
            "  <prefix value=\"all_mod\"/>\n"
            "  <import module=\"ietf-yang-types\">\n"
            "    <prefix value=\"yt\"/>\n"
            "    <revision-date date=\"2025-12-22\"/>\n"
            "    <description>\n"
            "      <text>YANG types</text>\n"
            "    </description>\n"
            "    <reference>\n"
            "      <text>RFC reference</text>\n"
            "    </reference>\n"
            "  </import>\n"
            "  <extension name=\"c-define\">\n"
            "    <argument name=\"name\"/>\n"
            "    <description>\n"
            "      <text>Takes as an argument a name string.\n"
            "Makes the code generator use the given name\n"
            "in the #define.</text>\n"
            "    </description>\n"
            "  </extension>\n"
            "  <feature name=\"feat1\">\n"
            "    <if-feature name=\"feat2\"/>\n"
            "    <status value=\"obsolete\"/>\n"
            "  </feature>\n"
            "  <feature name=\"feat2\"/>\n"
            "  <feature name=\"feat3\"/>\n"
            "  <identity name=\"ident2\">\n"
            "    <base name=\"ident1\"/>\n"
            "  </identity>\n"
            "  <identity name=\"ident1\"/>\n"
            "  <typedef name=\"tdef1\">\n"
            "    <type name=\"tdef2\">\n"
            "      <length value=\"3..9 | 30..40\"/>\n"
            "      <pattern value=\"[ac]*\"/>\n"
            "    </type>\n"
            "    <units name=\"none\"/>\n"
            "    <default value=\"aaa\"/>\n"
            "  </typedef>\n"
            "  <typedef name=\"tdef2\">\n"
            "    <type name=\"string\">\n"
            "      <length value=\"2..10 | 20..50\"/>\n"
            "      <pattern value=\"[ab]*\"/>\n"
            "    </type>\n"
            "  </typedef>\n"
            "  <grouping name=\"group1\">\n"
            "    <leaf name=\"leaf1\">\n"
            "      <type name=\"int8\"/>\n"
            "    </leaf>\n"
            "  </grouping>\n"
            "  <container name=\"cont1\">\n"
            "    <leaf name=\"leaf2\">\n"
            "      <if-feature name=\"feat1\"/>\n"
            "      <type name=\"int16\"/>\n"
            "      <status value=\"obsolete\"/>\n"
            "    </leaf>\n"
            "    <uses name=\"group1\">\n"
            "      <if-feature name=\"feat2\"/>\n"
            "      <refine target-node=\"leaf1\">\n"
            "        <if-feature name=\"feat3\"/>\n"
            "        <must condition=\"24 - 4 = number('20')\"/>\n"
            "        <default value=\"25\"/>\n"
            "        <config value=\"true\"/>\n"
            "        <mandatory value=\"false\"/>\n"
            "        <description>\n"
            "          <text>dsc</text>\n"
            "        </description>\n"
            "        <reference>\n"
            "          <text>none</text>\n"
            "        </reference>\n"
            "      </refine>\n"
            "    </uses>\n"
            "    <leaf name=\"leaf3\">\n"
            "      <type name=\"int32\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf4\">\n"
            "      <type name=\"int64\">\n"
            "        <range value=\"1000 .. 50000\">\n"
            "          <error-message>\n"
            "            <value>Special error message.</value>\n"
            "          </error-message>\n"
            "          <error-app-tag value=\"special-tag\"/>\n"
            "        </range>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf5\">\n"
            "      <type name=\"uint8\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf6\">\n"
            "      <type name=\"uint16\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf7\">\n"
            "      <type name=\"uint32\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf8\">\n"
            "      <type name=\"uint64\"/>\n"
            "    </leaf>\n"
            "    <choice name=\"choic1\">\n"
            "      <default value=\"leaf9b\"/>\n"
            "      <leaf name=\"leaf9a\">\n"
            "        <type name=\"decimal64\">\n"
            "          <fraction-digits value=\"9\"/>\n"
            "        </type>\n"
            "      </leaf>\n"
            "      <leaf name=\"leaf9b\">\n"
            "        <type name=\"boolean\"/>\n"
            "        <default value=\"false\"/>\n"
            "      </leaf>\n"
            "    </choice>\n"
            "    <leaf name=\"leaf10\">\n"
            "      <type name=\"boolean\"/>\n"
            "    </leaf>\n");
    strcpy(ori_res + strlen(ori_res),
            "    <leaf name=\"leaf11\">\n"
            "      <type name=\"enumeration\">\n"
            "        <enum name=\"one\"/>\n"
            "        <enum name=\"two\"/>\n"
            "        <enum name=\"five\">\n"
            "          <value value=\"5\"/>\n"
            "        </enum>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf12\">\n"
            "      <type name=\"bits\">\n"
            "        <bit name=\"flag0\">\n"
            "          <position value=\"0\"/>\n"
            "        </bit>\n"
            "        <bit name=\"flag1\"/>\n"
            "        <bit name=\"flag2\">\n"
            "          <position value=\"2\"/>\n"
            "        </bit>\n"
            "        <bit name=\"flag3\">\n"
            "          <position value=\"3\"/>\n"
            "        </bit>\n"
            "      </type>\n"
            "      <default value=\"flag0 flag3\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf13\">\n"
            "      <type name=\"binary\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf14\">\n"
            "      <type name=\"leafref\">\n"
            "        <path value=\"/cont1/leaf17\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf15\">\n"
            "      <type name=\"empty\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf16\">\n"
            "      <type name=\"union\">\n"
            "        <type name=\"instance-identifier\">\n"
            "          <require-instance value=\"true\"/>\n"
            "        </type>\n"
            "        <type name=\"int8\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <list name=\"list1\">\n"
            "      <key value=\"leaf18\"/>\n"
            "      <unique tag=\"leaf19\"/>\n"
            "      <min-elements value=\"1\"/>\n"
            "      <max-elements value=\"20\"/>\n"
            "      <leaf name=\"leaf18\">\n"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "      <leaf name=\"leaf19\">\n"
            "        <type name=\"uint32\"/>\n"
            "      </leaf>\n"
            "      <anyxml name=\"axml1\"/>\n"
            "      <anydata name=\"adata1\"/>\n"
            "      <action name=\"act1\">\n"
            "        <input>\n"
            "          <leaf name=\"leaf24\">\n"
            "            <type name=\"string\"/>\n"
            "          </leaf>\n"
            "        </input>\n"
            "        <output>\n"
            "          <leaf name=\"leaf25\">\n"
            "            <type name=\"string\"/>\n"
            "          </leaf>\n"
            "        </output>\n"
            "      </action>\n"
            "      <notification name=\"notif1\">\n"
            "        <leaf name=\"leaf26\">\n"
            "          <type name=\"string\"/>\n"
            "        </leaf>\n"
            "      </notification>\n"
            "    </list>\n"
            "    <leaf-list name=\"llist1\">\n"
            "      <type name=\"tdef1\"/>\n"
            "      <ordered-by value=\"user\"/>\n"
            "    </leaf-list>\n"
            "    <list name=\"list2\">\n"
            "      <key value=\"leaf27 leaf28\"/>\n"
            "      <leaf name=\"leaf27\">\n"
            "        <type name=\"uint8\"/>\n"
            "      </leaf>\n"
            "      <leaf name=\"leaf28\">\n"
            "        <type name=\"uint8\"/>\n"
            "      </leaf>\n"
            "    </list>\n"
            "    <leaf name=\"leaf29\">\n"
            "      <type name=\"instance-identifier\"/>\n"
            "    </leaf>\n"
            "    <container name=\"must-deviations-container\">\n"
            "      <presence value=\"Allows deviations on the leaf\"/>\n"
            "      <leaf name=\"leaf30\">\n"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "    </container>\n"
            "    <leaf name=\"leaf23\">\n"
            "      <type name=\"empty\"/>\n"
            "    </leaf>\n"
            "  </container>\n"
            "  <container name=\"test-when\">\n"
            "    <leaf name=\"when-check\">\n"
            "      <type name=\"boolean\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"gated-data\">\n"
            "      <when condition=\"../when-check = 'true'\"/>\n"
            "      <type name=\"uint16\"/>\n"
            "    </leaf>\n"
            "  </container>\n"
            "  <augment target-node=\"/cont1\">\n"
            "    <leaf name=\"leaf17\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </augment>\n"
            "  <rpc name=\"rpc1\">\n"
            "    <input>\n"
            "      <leaf name=\"leaf20\">\n"
            "        <type name=\"tdef1\"/>\n"
            "      </leaf>\n"
            "    </input>\n"
            "    <output>\n"
            "      <container name=\"cont2\">\n"
            "        <leaf name=\"leaf21\">\n"
            "          <type name=\"empty\"/>\n"
            "        </leaf>\n"
            "      </container>\n"
            "    </output>\n"
            "  </rpc>\n"
            "</module>\n");

    char *printed;
    struct ly_out *out;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YIN, 0, 0));
    assert_int_equal(strlen(ori_res), ly_out_printed(out));
    assert_string_equal(printed, ori_res);

    ly_out_free(out, NULL, 1);
    free(orig);
    free(ori_res);
}

static LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YIN;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
test_print_submodule(void **state)
{
    struct lys_module *mod;

    const char *mod_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a_mod=\"urn:a\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a_mod\"/>\n"
            "  <include module=\"a-sub\"/>\n"
            "</module>\n";

    char *submod_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<submodule name=\"a-sub\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a_mod=\"urn:a\"\n"
            "        xmlns:yt=\"urn:ietf:params:xml:ns:yang:ietf-yang-types\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <belongs-to module=\"a\">\n"
            "    <prefix value=\"a_mod\"/>\n"
            "  </belongs-to>\n"
            "  <import module=\"ietf-yang-types\">\n"
            "    <prefix value=\"yt\"/>\n"
            "    <revision-date date=\"2025-12-22\"/>\n"
            "  </import>\n\n"
            "  <description>\n"
            "    <text>YANG types</text>\n"
            "  </description>\n"
            "  <reference>\n"
            "    <text>RFC reference</text>\n"
            "  </reference>\n"
            "</submodule>\n";

    char *printed;
    struct ly_out *out;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod_yin);

    UTEST_ADD_MODULE(mod_yin, LYS_IN_YIN, NULL, &mod);
    assert_int_equal(LY_SUCCESS, lys_print_submodule(out, mod->parsed->includes[0].submodule, LYS_OUT_YIN, 0, 0));
    assert_int_equal(strlen(submod_yin), ly_out_printed(out));
    assert_string_equal(printed, submod_yin);

    ly_out_free(out, NULL, 1);
}

#define EXT_SUBELEM "<myext:c-define name=\"MY_MTU\"/>"

static void
test_enum_elem(void **state)
{
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;
    struct lysp_type *type;
    uint16_t flags = LYS_STATUS_DEPRC | LYS_SET_VALUE;
    const char *yin_data =
            "<module name=\"t-enum\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:t-enum\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t-enum\"/>"
            "  <prefix value=\"te\"/>"
            "  <extension name=\"c-define\">"
            "       <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"feature\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"enumeration\">"
            "      <enum name=\"enum-name\">"
            "        <if-feature name=\"feature\" />"
            "        <value value=\"55\" />"
            "        <status value=\"deprecated\" />"
            "        <description><text>desc...</text></description>"
            "        <reference><text>ref...</text></reference>"
            EXT_SUBELEM
            "      </enum>"
            "      <enum name=\"fallback\">"
            "        <value value=\"1\" />"
            "      </enum>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_non_null(mod);
    assert_non_null(mod->parsed);
    assert_non_null(mod->parsed->data);
    assert_int_equal(LYS_LEAF, mod->parsed->data->nodetype);
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    CHECK_LYSP_TYPE_ENUM(type->enums, "desc...", 1, flags, 1, "enum-name", "ref...", 55);
    assert_string_equal(type->enums->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(type->enums->exts, LY_STMT_ENUM, "MY_MTU");

    yin_data =
            "<module name=\"t-enum2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:t-enum2\"/>"
            "  <prefix value=\"te2\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"enumeration\">"
            "      <enum name=\"enum-name\"/>"
            "    </type>"
            "  </leaf>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    assert_string_equal(leaf->type.enums[0].name, "enum-name");
}

static void
test_bit_elem(void **state)
{
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;
    struct lysp_type *type;
    uint16_t flags = LYS_STATUS_DEPRC | LYS_SET_VALUE;
    const char *yin_data =
            "<module name=\"t-bit\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:t-bit\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t-bit\"/>"
            "  <prefix value=\"tb\"/>"
            "  <extension name=\"c-define\">"
            "     <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"feature\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"bits\">"
            "      <bit name=\"bit-name\">"
            "        <if-feature name=\"feature\" />"
            "        <position value=\"55\" />"
            "        <status value=\"deprecated\" />"
            "        <description><text>desc...</text></description>"
            "        <reference><text>ref...</text></reference>"
            EXT_SUBELEM
            "      </bit>"
            "      <bit name=\"fallback\">"
            "        <position value=\"1\" />"
            "      </bit>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_non_null(mod);
    assert_non_null(mod->parsed);
    assert_non_null(mod->parsed->data);
    assert_int_equal(LYS_LEAF, mod->parsed->data->nodetype);
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    CHECK_LYSP_TYPE_ENUM(type->bits, "desc...", 1, flags, 1, "bit-name", "ref...", 55);
    assert_string_equal(type->bits->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(type->bits->exts, LY_STMT_BIT, "MY_MTU");

    yin_data =
            "<module name=\"t-bit2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:t-bit2\"/>"
            "  <prefix value=\"tb2\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"bits\">"
            "      <bit name=\"bit-name\"/>"
            "    </type>"
            "  </leaf>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    CHECK_LYSP_TYPE_ENUM(leaf->type.bits, NULL, 0, 0, 0, "bit-name", NULL, 0);
}

static void
test_status_elem(void **state)
{
    struct lys_module *mod = NULL;
    const char *yin_data =
            "<module name=\"t-status\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:t-status\"/>"
            "  <prefix value=\"st\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\" />"
            "    <status value=\"invalid\" />"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    CHECK_LOG_CTX("Parsing module \"t-status\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"status\" element. "
            "Valid values are \"current\", \"deprecated\" and \"obsolete\".", NULL, 1);
}

static void
test_yin_element_elem(void **state)
{
    struct lys_module *mod = NULL;
    const char *yin_data =
            "<module name=\"t-yin-elem\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:t-bit\"/>"
            "  <prefix value=\"myext\"/>"
            "  <extension name=\"c-define\">"
            "       <argument name=\"name\">"
            "          <yin-element value=\"invalid\"/>"
            "       </argument>"
            "  </extension>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    CHECK_LOG_CTX("Parsing module \"t-yin-elem\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"yin-element\" element. "
            "Valid values are \"true\" and \"false\".", NULL, 1);
}

static void
test_yangversion_elem(void **state)
{
    struct lys_module *mod = NULL;
    const char *yin_data =
            "<module name=\"t-ver\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"invalid\"/>"
            "  <namespace uri=\"urn:t-ver\"/>"
            "  <prefix value=\"v\"/>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    CHECK_LOG_CTX("Parsing module \"t-ver\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"yang-version\" element. Valid values are \"1\" and \"1.1\".", NULL, 1);
}

static void
test_argument_elem(void **state)
{
    struct lys_module *mod = NULL;
    const char *yin_data =
            "<module name=\"t-arg\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:t-bit\"/>"
            "  <prefix value=\"myext\"/>"
            "  <extension name=\"c-define\">"
            "     <argument name=\"arg\"/>"
            "  </extension>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_string_equal(mod->parsed->extensions->argname, "arg");
    assert_true(mod->parsed->extensions->flags == 0);
}

static void
test_belongsto_elem(void **state)
{
    struct lys_module *mod = NULL;
    const char *parent_yin =
            "<module name=\"parent\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:parent\"/>"
            "  <prefix value=\"mn\"/>"
            "  <include module=\"sub\"/>"
            "</module>";
    char *submod_in =
            "<submodule name=\"sub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <belongs-to module=\"parent\">"
            "    <prefix value=\"mn\"/>"
            "  </belongs-to>"
            "</submodule>";
    char *submod_fail =
            "<submodule name=\"sub-fail\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <belongs-to module=\"parent\">"
            "  </belongs-to>"
            "</submodule>";

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod_fail);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, parent_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"parent\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"sub\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory sub-element \"prefix\" of \"belongs-to\" element.", NULL, 1);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod_in);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, parent_yin, LYS_IN_YIN, &mod));
}

static void
test_config_elem(void **state)
{
    struct lys_module *mod = NULL;
    const char *yin_data =
            "<module name=\"config\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:minimal-config\"/>"
            "  <prefix value=\"min\"/>"
            "  <leaf name=\"my-setting\">"
            "    <type name=\"string\"/>"
            "    <config value=\"%s\"/>"
            "  </leaf>"
            "</module>";
    char buffer[256];

    snprintf(buffer, sizeof(buffer), yin_data, "false");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, &mod));

    snprintf(buffer, sizeof(buffer), yin_data, "invalid");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, &mod));
    CHECK_LOG_CTX("Parsing module \"config\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"config\" element. "
            "Valid values are \"true\" and \"false\".", NULL, 1);
}

static void
test_default_elem(void **state)
{
    const char *yin_data =
            "<module name=\"def\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:mod\"/>"
            "  <prefix value=\"m\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\"/>"
            "    <default/>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"def\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory attribute value of default element.", NULL, 1);
}

static void
test_err_app_tag_elem(void **state)
{
    const char *yin_data =
            "<module name=\"err-app-tag\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:mod\"/>"
            "  <prefix value=\"m\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\"/>"
            "    <must condition=\"true\">"
            "      <error-app-tag/>"
            "    </must>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"err-app-tag\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory attribute value of error-app-tag element.", NULL, 1);
}

static void
test_err_msg_elem(void **state)
{
    const char *yin_data =
            "<module name=\"err-msg\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:mod\"/>"
            "  <prefix value=\"m\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\"/>"
            "    <must condition=\"true\">"
            "      <error-message></error-message>"
            "    </must>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"err-msg\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory sub-element \"value\" of \"error-message\" element.", NULL, 1);

    yin_data = "<module name=\"err-msg\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:mod\"/>"
            "  <prefix value=\"m\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\"/>"
            "    <must condition=\"true\">"
            "      <error-message invalid=\"text\"/>"
            "    </must>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"err-msg\" failed.", NULL, 0);
    CHECK_LOG_CTX("Unexpected attribute \"invalid\" of \"error-message\" element.", NULL, 1);
}

static void
test_fracdigits_elem(void **state)
{
    char buffer[256];
    const char *yin_data =
            "<module name=\"frac-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:frac1\"/>"
            "  <prefix value=\"f1\"/>"
            "  <leaf name=\"l\"><type name=\"decimal64\"><fraction-digits value=\"%s\"/></type></leaf>"
            "</module>";

    /* invalid values */
    snprintf(buffer, sizeof(buffer), yin_data, "-1");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"frac-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    snprintf(buffer, sizeof(buffer), yin_data, "02");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"frac-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"02\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    snprintf(buffer, sizeof(buffer), yin_data, "1p");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"frac-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"1p\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    snprintf(buffer, sizeof(buffer), yin_data, "19");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"frac-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"19\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    snprintf(buffer, sizeof(buffer), yin_data, "999999999999999999");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, buffer, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"frac-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"999999999999999999\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

}

static void
test_iffeature_elem(void **state)
{
    const char *yin_data =
            "<module name=\"feat\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:feat\"/>"
            "  <prefix value=\"ft\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\"/>"
            "    <if-feature/>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"feat\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory attribute name of if-feature element.", NULL, 1);
}

static void
test_length_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;
    struct lysp_type *type;

    /* max subelems */
    yin_data =
            "<module name=\"len-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:len1\">"
            "  <namespace uri=\"urn:len1\"/>"
            "  <prefix value=\"l1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\">"
            "      <length value=\"1..10\">"
            "        <error-message><value>err-msg</value></error-message>"
            "        <error-app-tag value=\"err-app-tag\"/>"
            "        <description><text>desc</text></description>"
            "        <reference><text>ref</text></reference>"
            EXT_SUBELEM
            "      </length>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;

    CHECK_LYSP_RESTR(type->length, "1..10", "desc", "err-app-tag", "err-msg", 1, "ref");
    assert_true(type->flags & LYS_SET_LENGTH);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type->length->exts[0]), LY_STMT_LENGTH, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"len-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:len2\"/>"
            "  <prefix value=\"l2\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\">"
            "      <length value=\"1..10\"></length>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;

    CHECK_LYSP_RESTR(type->length, "1..10", NULL, NULL, NULL, 0, NULL);
    assert_true(type->flags & LYS_SET_LENGTH);

    yin_data =
            "<module name=\"len-fail\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:lenf\"/>"
            "  <prefix value=\"lf\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\">"
            "      <length></length>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"len-fail\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory attribute value of length element.", NULL, 1);
}

static void
test_modifier_elem(void **state)
{
    const char *yin_data =
            "<module name=\"modif\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:mod\"/>"
            "  <prefix value=\"mm\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\">"
            "      <pattern value=\"a.*\">"
            "        <modifier value=\"invert\"/>"
            "      </pattern>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"modif\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"invert\" of \"value\" attribute in \"modifier\" element. "
            "Only valid value is \"invert-match\".", NULL, 1);
}

static void
test_namespace_elem(void **state)
{
    const char *yin_data =
            "<module name=\"ns\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace/>"
            "  <prefix value=\"nt\"/>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"ns\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory attribute uri of namespace element.", NULL, 1);
}

static void
test_pattern_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;
    struct lysp_type *type;

    /* max subelems */
    yin_data =
            "<module name=\"pat-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:pat1\">"
            "  <namespace uri=\"urn:pat1\"/>"
            "  <prefix value=\"p1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\">"
            "      <pattern value=\"super_pattern\">"
            "        <modifier value=\"invert-match\"/>"
            "        <error-message><value>err-msg-value</value></error-message>"
            "        <error-app-tag value=\"err-app-tag-value\"/>"
            "        <description><text>&quot;pattern-desc&quot;</text></description>"
            "        <reference><text>pattern-ref</text></reference>"
            EXT_SUBELEM
            "      </pattern>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    assert_true(type->flags & LYS_SET_PATTERN);
    CHECK_LYSP_RESTR(type->patterns, "\x015super_pattern", "\"pattern-desc\"",
            "err-app-tag-value", "err-msg-value", 1, "pattern-ref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type->patterns->exts[0]), LY_STMT_PATTERN, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"pat-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:pat2\"/>"
            "  <prefix value=\"p2\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\">"
            "      <pattern value=\"pattern\"> </pattern>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    CHECK_LYSP_RESTR(type->patterns, "\x006pattern", NULL, NULL, NULL, 0, NULL);
}

static void
test_value_position_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;

    /* valid values */
    yin_data = "<module name=\"vp1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp1\"/><prefix value=\"vp1\"/>"
            "<leaf name=\"l\"><type name=\"enumeration\"><enum name=\"e\"><value value=\"-55\"/></enum></type></leaf></module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    CHECK_LYSP_TYPE_ENUM(&(leaf->type.enums[0]), NULL, 0, LYS_SET_VALUE, 0, "e", NULL, -55);

    yin_data = "<module name=\"vp2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp2\"/><prefix value=\"vp2\"/>"
            "<leaf name=\"l\"><type name=\"enumeration\"><enum name=\"e\"><value value=\"0\"/></enum></type></leaf></module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    CHECK_LYSP_TYPE_ENUM(&(leaf->type.enums[0]), NULL, 0, LYS_SET_VALUE, 0, "e", NULL, 0);

    yin_data = "<module name=\"vp3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp3\"/><prefix value=\"vp3\"/>"
            "<leaf name=\"l\"><type name=\"enumeration\"><enum name=\"e\"><value value=\"-0\"/></enum></type></leaf></module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    CHECK_LYSP_TYPE_ENUM(&(leaf->type.enums[0]), NULL, 0, LYS_SET_VALUE, 0, "e", NULL, 0);

    /* valid positions */
    yin_data = "<module name=\"vp4\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp4\"/><prefix value=\"vp4\"/>"
            "<leaf name=\"l\"><type name=\"bits\"><bit name=\"b\"><position value=\"0\"/></bit></type></leaf></module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    CHECK_LYSP_TYPE_ENUM(&(leaf->type.bits[0]), NULL, 0, LYS_SET_VALUE, 0, "b", NULL, 0);

    /* invalid values */
    yin_data = "<module name=\"vp5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp5\"/><prefix value=\"vp5\"/>"
            "<leaf name=\"l\"><type name=\"enumeration\"><enum name=\"e\"><value value=\"99999999999999999999999\"/></enum></type></leaf></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"vp5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"99999999999999999999999\" of \"value\" attribute in \"value\" element.", NULL, 1);

    yin_data = "<module name=\"vp5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp6\"/><prefix value=\"vp6\"/>"
            "<leaf name=\"l\"><type name=\"enumeration\"><enum name=\"e\"><value value=\"1k\"/></enum></type></leaf></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"vp5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"1k\" of \"value\" attribute in \"value\" element.", NULL, 1);

    yin_data = "<module name=\"vp5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp7\"/><prefix value=\"vp7\"/>"
            "<leaf name=\"l\"><type name=\"enumeration\"><enum name=\"e\"><value value=\"\"/></enum></type></leaf></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"vp5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"value\" element.", NULL, 1);

    /*invalid positions */
    yin_data = "<module name=\"vp5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp8\"/><prefix value=\"vp8\"/>"
            "<leaf name=\"l\"><type name=\"bits\"><bit name=\"b\"><position value=\"-5\"/></bit></type></leaf></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"vp5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-5\" of \"value\" attribute in \"position\" element.", NULL, 1);

    yin_data = "<module name=\"vp5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp9\"/><prefix value=\"vp9\"/>"
            "<leaf name=\"l\"><type name=\"bits\"><bit name=\"b\"><position value=\"-0\"/></bit></type></leaf></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"vp5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-0\" of \"value\" attribute in \"position\" element.", NULL, 1);

    yin_data = "<module name=\"vp5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp10\"/><prefix value=\"vp10\"/>"
            "<leaf name=\"l\"><type name=\"bits\"><bit name=\"b\"><position value=\"99999999999999999999\"/></bit></type></leaf></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"vp5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"99999999999999999999\" of \"value\" attribute in \"position\" element.", NULL, 1);

    yin_data = "<module name=\"vp5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:vp11\"/><prefix value=\"vp11\"/>"
            "<leaf name=\"l\"><type name=\"bits\"><bit name=\"b\"><position value=\"\"/></bit></type></leaf></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"vp5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"position\" element.", NULL, 1);
}

static void
test_prefix_elem(void **state)
{
    struct lys_module *mod = NULL;
    const char *yin_data =
            "<module name=\"pref-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:pref\"/>"
            "  <prefix value=\"pref\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_string_equal(mod->prefix, "pref");
}

static void
test_range_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;
    struct lysp_type *type;

    /* max subelems */
    yin_data =
            "<module name=\"rng-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:rng1\">"
            "  <namespace uri=\"urn:rng1\"/>"
            "  <prefix value=\"r1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <leaf name=\"l\">"
            "    <type name=\"uint8\">"
            "      <range value=\"1..10\">"
            "        <error-message><value>err-msg</value></error-message>"
            "        <error-app-tag value=\"err-app-tag\" />"
            "        <description><text>desc</text></description>"
            "        <reference><text>ref</text></reference>"
            EXT_SUBELEM
            "      </range>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    assert_true(type->flags & LYS_SET_RANGE);
    CHECK_LYSP_RESTR(type->range, "1..10", "desc",
            "err-app-tag", "err-msg", 1, "ref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type->range->exts[0]), LY_STMT_RANGE, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"rng-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:rng2\"/>"
            "  <prefix value=\"r2\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"uint8\">"
            "      <range value=\"1..10\"/>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    CHECK_LYSP_RESTR(type->range, "1..10", NULL, NULL, NULL, 0, NULL);
}

static void
test_reqinstance_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;
    struct lysp_type *type;

    yin_data =
            "<module name=\"req-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:req1\">"
            "  <namespace uri=\"urn:req1\"/>"
            "  <prefix value=\"r1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <leaf name=\"l\">"
            "    <type name=\"instance-identifier\">"
            "      <require-instance value=\"true\">"
            EXT_SUBELEM
            "      </require-instance>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    assert_int_equal(type->require_instance, 1);
    assert_true(type->flags & LYS_SET_REQINST);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type->exts[0]), LY_STMT_REQUIRE_INSTANCE, "MY_MTU");

    yin_data =
            "<module name=\"req-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:req2\"/>"
            "  <prefix value=\"r2\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"instance-identifier\">"
            "      <require-instance value=\"false\"/>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    type = &leaf->type;
    assert_int_equal(type->require_instance, 0);
    assert_true(type->flags & LYS_SET_REQINST);

    /* invalid */
    yin_data =
            "<module name=\"req-mod3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:req3\"/>"
            "  <prefix value=\"r3\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"instance-identifier\">"
            "      <require-instance value=\"invalid\"/>"
            "    </type>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"req-mod3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"require-instance\" element. "
            "Valid values are \"true\" and \"false\".", NULL, 1);
}

static void
test_revision_date_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lys_module *dummy = NULL;

    const char *dummy_mod =
            "<module name=\"dummy\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:dummy\"/>"
            "  <prefix value=\"dum\"/>"
            "  <revision date=\"2000-01-01\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, dummy_mod, LYS_IN_YIN, &dummy));

    yin_data =
            "<module name=\"rev-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:rev1\"/>"
            "  <prefix value=\"r1\"/>"
            "  <import module=\"dummy\">"
            "    <prefix value=\"d\"/>"
            "    <revision-date date=\"2000-01-01\"/>"
            "  </import>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_string_equal(mod->parsed->imports[0].rev, "2000-01-01");

    yin_data =
            "<module name=\"rev-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:rev2\"/>"
            "  <prefix value=\"r2\"/>"
            "  <import module=\"dummy\">"
            "    <prefix value=\"d\"/>"
            "    <revision-date date=\"2000-50-05\"/>"
            "  </import>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"rev-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"2000-50-05\" of \"revision-date\".", NULL, 1);
}

static void
test_unique_elem(void **state)
{
    struct lys_module *mod = NULL;
    struct lysp_node_list *list;
    const char *yin_data =
            "<module name=\"uniq-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:uniq\"/>"
            "  <prefix value=\"u\"/>"
            "  <list name=\"l\">"
            "    <config value=\"false\"/>"
            "    <leaf name=\"tag\"><type name=\"string\"/></leaf>"
            "    <unique tag=\"tag\"/>"
            "  </list>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    list = (struct lysp_node_list *)mod->parsed->data;
    assert_string_equal(list->uniques[0].str, "tag");
}

static void
test_units_elem(void **state)
{
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;
    const char *yin_data =
            "<module name=\"units-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:units\"/>"
            "  <prefix value=\"u\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\"/>"
            "    <units name=\"name\"/>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    assert_string_equal(leaf->units, "name");
}

static void
test_yin_text_value_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *leaf;

    yin_data =
            "<module name=\"txt-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:txt1\"/>"
            "  <prefix value=\"t1\"/>"
            "  <description><text>text</text></description>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_string_equal(mod->dsc, "text");

    yin_data =
            "<module name=\"txt-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:txt2\"/>"
            "  <prefix value=\"t2\"/>"
            "  <leaf name=\"l\">"
            "    <type name=\"string\"/>"
            "    <must condition=\"'true()'\">"
            "      <error-message><value>text</value></error-message>"
            "    </must>"
            "  </leaf>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    leaf = (struct lysp_node_leaf *)mod->parsed->data;
    assert_string_equal(leaf->musts->emsg, "text");

    yin_data =
            "<module name=\"txt-mod3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:txt3\"/>"
            "  <prefix value=\"t3\"/>"
            "  <description><text></text></description>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_string_equal(mod->dsc, "");
}

static void
test_max_elems_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_uses *uses;

    yin_data =
            "<module name=\"max-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:max1\"/>"
            "  <prefix value=\"m1\"/>"
            "  <grouping name=\"g\">"
            "    <list name=\"l\"><config value=\"false\"/></list>"
            "  </grouping>"
            "  <uses name=\"g\">"
            "    <refine target-node=\"l\">"
            "      <max-elements value=\"10\"/>"
            "    </refine>"
            "  </uses>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    uses = (struct lysp_node_uses *)mod->parsed->data;
    assert_int_equal(uses->refines->max, 10);
    assert_true(uses->refines->flags & LYS_SET_MAX);

    yin_data = "<module name=\"max-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:max2\"/><prefix value=\"m2\"/>"
            "<list name=\"l\"><config value=\"false\"/><max-elements value=\"0\"/></list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"max-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"0\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);

    yin_data = "<module name=\"max-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:max3\"/><prefix value=\"m3\"/>"
            "<list name=\"l\"><config value=\"false\"/><max-elements value=\"-10\"/></list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"max-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-10\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);

    yin_data = "<module name=\"max-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:max4\"/><prefix value=\"m4\"/>"
            "<list name=\"l\"><config value=\"false\"/><max-elements value=\"k\"/></list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"max-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"k\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);

    yin_data = "<module name=\"max-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:max5\"/><prefix value=\"m5\"/>"
            "<list name=\"l\"><config value=\"false\"/><max-elements value=\"u12\"/></list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"max-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"u12\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);
}

static void
test_min_elems_elem(void **state)
{
    const char *yin_data;

    yin_data = "<module name=\"min-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:min1\"/><prefix value=\"m1\"/>"
            "<leaf-list name=\"ll\"><type name=\"string\"/><min-elements value=\"-5\"/></leaf-list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"min-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Value \"-5\" of \"value\" attribute in \"min-elements\" element is out of bounds.", NULL, 1);

    yin_data = "<module name=\"min-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:min2\"/><prefix value=\"m2\"/>"
            "<leaf-list name=\"ll\"><type name=\"string\"/><min-elements value=\"99999999999999999\"/></leaf-list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"min-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Value \"99999999999999999\" of \"value\" attribute in \"min-elements\" element is out of bounds.", NULL, 1);

    yin_data = "<module name=\"min-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:min3\"/><prefix value=\"m3\"/>"
            "<leaf-list name=\"ll\"><type name=\"string\"/><min-elements value=\"5k\"/></leaf-list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"min-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"5k\" of \"value\" attribute in \"min-elements\" element.", NULL, 1);

    yin_data = "<module name=\"min-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"><namespace uri=\"urn:min4\"/><prefix value=\"m4\"/>"
            "<leaf-list name=\"ll\"><type name=\"string\"/><min-elements value=\"05\"/></leaf-list></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"min-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"05\" of \"value\" attribute in \"min-elements\" element.", NULL, 1);
}

static void
test_ordby_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_list *list;

    yin_data =
            "<module name=\"ord-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:ord1\"/>"
            "  <prefix value=\"o1\"/>"
            "  <list name=\"l\">"
            "    <key value=\"k\"/>"
            "    <leaf name=\"k\"><type name=\"string\"/></leaf>"
            "    <ordered-by value=\"user\"/>"
            "  </list>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    list = (struct lysp_node_list *)mod->parsed->data;
    assert_true(list->flags & LYS_ORDBY_USER);

    yin_data =
            "<module name=\"ord-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:ord2\"/>"
            "  <prefix value=\"o2\"/>"
            "  <list name=\"l\">"
            "    <key value=\"k\"/>"
            "    <leaf name=\"k\"><type name=\"string\"/></leaf>"
            "    <ordered-by value=\"inv\"/>"
            "  </list>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"ord-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"inv\" of \"value\" attribute in \"ordered-by\" element. "
            "Valid values are \"system\" and \"user\".", NULL, 1);
}

static void
test_any_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_anydata *parsed = NULL;
    uint16_t flags = LYS_CONFIG_W | LYS_MAND_TRUE | LYS_STATUS_DEPRC;

    /* anyxml max subelems */
    yin_data =
            "<module name=\"any-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:any1\">"
            "  <namespace uri=\"urn:any1\"/>"
            "  <prefix value=\"a1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"feature\"/>"
            "  <anyxml name=\"any-name\">"
            "    <config value=\"true\" />"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"feature\" />"
            "    <mandatory value=\"true\" />"
            "    <must condition=\"must-cond\" />"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"deprecated\"/>"
            "    <when condition=\"when-cond\"/>"
            EXT_SUBELEM
            "  </anyxml>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_anydata *)mod->parsed->data;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "any-name", 0, LYS_ANYXML, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_ANYXML, "MY_MTU");

    /* anydata max subelems */
    yin_data =
            "<module name=\"any-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:any2\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:any2\"/>"
            "  <prefix value=\"a2\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"feature\"/>"
            "  <anydata name=\"any-name\">"
            "    <config value=\"true\" />"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"feature\" />"
            "    <mandatory value=\"true\" />"
            "    <must condition=\"must-cond\" />"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"deprecated\"/>"
            "    <when condition=\"when-cond\"/>"
            EXT_SUBELEM
            "  </anydata>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_anydata *)mod->parsed->data;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "any-name", 0, LYS_ANYDATA, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_ANYDATA, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"any-mod3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:any3\"/>"
            "  <prefix value=\"a3\"/>"
            "  <anydata name=\"any-name\"> </anydata>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_anydata *)mod->parsed->data;
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "any-name", 0, LYS_ANYDATA, 0, NULL);
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 0);
}

static void
test_leaf_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaf *parsed = NULL;
    uint16_t flags;

    /* max elements */
    yin_data =
            "<module name=\"leaf-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:leaf1\">"
            "  <namespace uri=\"urn:leaf1\"/>"
            "  <yang-version value=\"1.1\"/>"
            "  <prefix value=\"l1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"feature\"/>"
            "  <leaf name=\"leaf1\">"
            "    <config value=\"true\" />"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"feature\" />"
            "    <mandatory value=\"true\" />"
            "    <must condition=\"must-cond\" />"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"deprecated\"/>"
            "    <type name=\"string\"/>"
            "    <units name=\"uni\"/>"
            "    <when condition=\"when-cond\"/>"
            EXT_SUBELEM
            "  </leaf>"
            "  <leaf name=\"leaf2\">"
            "    <type name=\"string\"/>"
            "    <default value=\"def-val\"/>"
            "  </leaf>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_leaf *)mod->parsed->data;
    flags = LYS_CONFIG_W | LYS_MAND_TRUE | LYS_STATUS_DEPRC;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "leaf1", 1, LYS_LEAF, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LEAF, "MY_MTU");
    assert_string_equal(parsed->musts->arg.str, "must-cond");
    assert_string_equal(parsed->type.name, "string");
    assert_string_equal(parsed->units, "uni");

    parsed = (struct lysp_node_leaf *)parsed->next;
    assert_string_equal(parsed->name, "leaf2");
    assert_string_equal(parsed->dflt.str, "def-val");

    /* min elements */
    yin_data = "<module name=\"leaf-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:leaf2\"/>"
            "  <prefix value=\"l2\"/>"
            "  <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_leaf *)mod->parsed->data;
    assert_string_equal(parsed->name, "leaf");
    assert_string_equal(parsed->type.name, "string");
}

static void
test_leaf_list_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_leaflist *parsed = NULL;
    uint16_t flags;

    yin_data =
            "<module name=\"ll-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:ll1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:ll1\"/>"
            "  <prefix value=\"ll1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"feature\"/>"
            "  <leaf-list name=\"llist\">"
            "    <config value=\"true\" />"
            "    <default value=\"def-val0\"/>"
            "    <default value=\"def-val1\"/>"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"feature\"/>"
            "    <max-elements value=\"5\"/>"
            "    <must condition=\"must-cond\"/>"
            "    <ordered-by value=\"user\" />"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"current\"/>"
            "    <type name=\"string\"/>"
            "    <units name=\"uni\"/>"
            "    <when condition=\"when-cond\"/>"
            EXT_SUBELEM
            "  </leaf-list>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_leaflist *)mod->parsed->data;
    flags = LYS_CONFIG_W | LYS_ORDBY_USER | LYS_STATUS_CURR | LYS_SET_MAX;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "llist", 0, LYS_LEAFLIST, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    assert_string_equal(parsed->dflts[0].str, "def-val0");
    assert_string_equal(parsed->dflts[1].str, "def-val1");
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_int_equal(parsed->max, 5);
    assert_string_equal(parsed->type.name, "string");
    assert_string_equal(parsed->units, "uni");
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LEAF_LIST, "MY_MTU");

    yin_data =
            "<module name=\"ll-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:ll2\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:ll2\"/>"
            "  <prefix value=\"ll2\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"feature\"/>"
            "  <leaf-list name=\"llist\">"
            "    <config value=\"true\" />"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"feature\"/>"
            "    <min-elements value=\"5\"/>"
            "    <must condition=\"must-cond\"/>"
            "    <ordered-by value=\"user\" />"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"current\"/>"
            "    <type name=\"string\"/>"
            "    <units name=\"uni\"/>"
            "    <when condition=\"when-cond\"/>"
            EXT_SUBELEM
            "  </leaf-list>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_leaflist *)mod->parsed->data;
    flags = LYS_CONFIG_W | LYS_ORDBY_USER | LYS_STATUS_CURR | LYS_SET_MIN;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "llist", 0, LYS_LEAFLIST, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_int_equal(parsed->min, 5);
    assert_string_equal(parsed->type.name, "string");
    assert_string_equal(parsed->units, "uni");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LEAF_LIST, "MY_MTU");

    yin_data =
            "<module name=\"ll-mod3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:ll3\"/>"
            "  <prefix value=\"ll3\"/>"
            "  <feature name=\"feature\"/>"
            "  <leaf-list name=\"llist\">"
            "    <config value=\"true\" />"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"feature\"/>"
            "    <max-elements value=\"15\"/>"
            "    <min-elements value=\"5\"/>"
            "    <must condition=\"must-cond\"/>"
            "    <ordered-by value=\"user\" />"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"current\"/>"
            "    <type name=\"string\"/>"
            "    <units name=\"uni\"/>"
            "    <when condition=\"when-cond\"/>"
            "  </leaf-list>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_leaflist *)mod->parsed->data;
    flags = LYS_CONFIG_W | LYS_ORDBY_USER | LYS_STATUS_CURR | LYS_SET_MIN | LYS_SET_MAX;
    CHECK_LYSP_NODE(parsed, "desc", 0, flags, 1,
            "llist", 0, LYS_LEAFLIST, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_int_equal(parsed->min, 5);
    assert_int_equal(parsed->max, 15);
    assert_string_equal(parsed->type.name, "string");
    assert_string_equal(parsed->units, "uni");

    yin_data =
            "<module name=\"ll-mod4\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:ll4\"/>"
            "  <prefix value=\"ll4\"/>"
            "  <leaf-list name=\"llist\">"
            "    <type name=\"string\"/>"
            "  </leaf-list>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_leaflist *)mod->parsed->data;
    assert_string_equal(parsed->name, "llist");
    assert_string_equal(parsed->type.name, "string");

    /* invalid combinations */
    yin_data =
            "<module name=\"ll-inv\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:inv1\"/>"
            "  <prefix value=\"i1\"/>"
            "  <leaf-list name=\"llist\">"
            "    <max-elements value=\"5\"/>"
            "    <min-elements value=\"15\"/>"
            "    <type name=\"string\"/>"
            "  </leaf-list>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"ll-inv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid combination of min-elements and max-elements: min value 15 is bigger than the max value 5.",
            NULL, 1);
    yin_data =
            "<module name=\"ll-inv\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:inv2\"/>"
            "  <prefix value=\"i2\"/>"
            "  <leaf-list name=\"llist\">"
            "    <default value=\"def-val1\"/>"
            "    <min-elements value=\"15\"/>"
            "    <type name=\"string\"/>"
            "  </leaf-list>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"ll-inv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid combination of sub-elemnts \"min-elements\" and \"default\" in \"leaf-list\" element.", NULL, 1);

    yin_data =
            "<module name=\"ll-inv\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:inv3\"/>"
            "  <prefix value=\"i3\"/>"
            "  <leaf-list name=\"llist\">"
            "  </leaf-list>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"ll-inv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory sub-element \"type\" of \"leaf-list\" element.", NULL, 1);
}

static void
test_presence_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_container *cont;

    yin_data =
            "<module name=\"pres-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:pres1\"/>"
            "  <prefix value=\"p1\"/>"
            "  <container name=\"c\">"
            "    <presence value=\"presence-val\"/>"
            "  </container>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    cont = (struct lysp_node_container *)mod->parsed->data;
    assert_string_equal(cont->presence, "presence-val");

    yin_data =
            "<module name=\"pres-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:pres2\"/>"
            "  <prefix value=\"p2\"/>"
            "  <container name=\"c\">"
            "    <presence/>"
            "  </container>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"pres-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory attribute value of presence element.", NULL, 1);
}

static void
test_key_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_list *list;

    yin_data =
            "<module name=\"key-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:key1\"/>"
            "  <prefix value=\"k1\"/>"
            "  <list name=\"l\">"
            "    <key value=\"key-value\"/>"
            "    <leaf name=\"key-value\"><type name=\"string\"/></leaf>"
            "  </list>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    list = (struct lysp_node_list *)mod->parsed->data;
    assert_string_equal(list->key, "key-value");

    yin_data =
            "<module name=\"key-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:key2\"/>"
            "  <prefix value=\"k2\"/>"
            "  <list name=\"l\">"
            "    <key/>"
            "  </list>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"key-mod2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory attribute value of key element.", NULL, 1);
}

static void
test_uses_elem(void **state)
{
    struct lys_module *mod;
    struct lysp_node_uses *parsed;
    const char *yin_data;

    /* max subelems */
    yin_data =
            "<module name=\"uses1\""
            "    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1.1\"/>"
            "    <namespace uri=\"urn:uses1\"/>"
            "    <prefix value=\"u1\"/>"
            "    <feature name=\"feature\"/>"
            "    <grouping name=\"uses-name\">"
            "        <container name=\"target\"/>"
            "    </grouping>"
            "    <uses name=\"uses-name\">"
            "        <when condition=\"cond\"/>"
            "        <if-feature name=\"feature\"/>"
            "        <status value=\"obsolete\"/>"
            "        <description><text>desc</text></description>"
            "        <reference><text>ref</text></reference>"
            "        <refine target-node=\"target\"/>"
            "        <augment target-node=\"target\">"
            "            <container name=\"aug-cont\"/>"
            "        </augment>"
            "    </uses>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_uses *)mod->parsed->data;
    CHECK_LYSP_NODE(parsed, "desc", 0, LYS_STATUS_OBSLT, 1,
            "uses-name", 0, LYS_USES, 0, "ref");
    CHECK_LYSP_WHEN(parsed->when, "cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_string_equal(parsed->refines->nodeid, "target");
    assert_string_equal(parsed->augments->nodeid, "target");

    /* min subelems */
    yin_data =
            "<module name=\"uses2\""
            "    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1.1\"/>"
            "    <namespace uri=\"urn:uses2\"/>"
            "    <prefix value=\"u2\"/>"
            "    <grouping name=\"uses-name\"/>"
            "    <uses name=\"uses-name\"/>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_uses *)mod->parsed->data;
    assert_string_equal(parsed->name, "uses-name");
}

static void
test_list_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_list *parsed = NULL;
    uint16_t flags = LYS_ORDBY_USER | LYS_STATUS_DEPRC | LYS_CONFIG_W | LYS_SET_MIN;

    /* max subelems */
    yin_data =
            "<module name=\"list-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:list1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:list1\"/>"
            "  <prefix value=\"l1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <list name=\"list-name\">"
            "    <when condition=\"when\"/>"
            "    <if-feature name=\"iff\"/>"
            "    <must condition=\"must-cond\"/>"
            "    <key value=\"key\"/>"
            "    <unique tag=\"utag\"/>"
            "    <config value=\"true\"/>"
            "    <min-elements value=\"10\"/>"
            "    <ordered-by value=\"user\"/>"
            "    <status value=\"deprecated\"/>"
            "    <description><text>desc</text></description>"
            "    <reference><text>ref</text></reference>"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <container name=\"cont\"/>"
            "    <choice name=\"choice\"/>"
            "    <action name=\"action\"/>"
            "    <grouping name=\"grp\"/>"
            "    <notification name=\"notf\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"sub-list\"><config value=\"false\"/></list>"
            "    <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "    <uses name=\"grp\"/>"
            EXT_SUBELEM
            "    <leaf name=\"key\"><type name=\"string\"/></leaf>"
            "    <leaf name=\"utag\"><type name=\"string\"/></leaf>"
            "  </list>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_list *)mod->parsed->data;

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
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "grp");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->next->name, "key");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->next->next->name, "utag");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->next->next->nodetype, LYS_LEAF);
    assert_null(parsed->child->next->next->next->next->next->next->next->next->next->next);

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1, "list-name", 0, LYS_LIST, 0, "ref");
    CHECK_POINTER(parsed->when, 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when", NULL, 0, NULL);
    assert_string_equal(parsed->groupings->name, "grp");
    assert_string_equal(parsed->actions->name, "action");
    assert_int_equal(parsed->groupings->nodetype, LYS_GROUPING);
    assert_string_equal(parsed->notifs->name, "notf");
    assert_string_equal(parsed->iffeatures[0].str, "iff");
    assert_string_equal(parsed->key, "key");
    assert_int_equal(parsed->min, 10);
    assert_string_equal(parsed->typedefs->name, "tpdf");
    assert_string_equal(parsed->uniques->str, "utag");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LIST, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"list-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:list2\"/>"
            "  <prefix value=\"l2\"/>"
            "  <list name=\"list-name\">"
            "    <config value=\"false\"/>"
            "  </list>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_list *)mod->parsed->data;
    CHECK_LYSP_NODE(parsed, NULL, 0, LYS_CONFIG_R, 0, "list-name", 0, LYS_LIST, 0, NULL);
    CHECK_POINTER(parsed->when, 0);
}

static void
test_notification_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_notif *notifs = NULL;

    /* max subelems */
    yin_data =
            "<module name=\"notif-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:notif1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:notif1\"/>"
            "  <prefix value=\"n1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <grouping name=\"uses-name\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <notification name=\"notif-name\">"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"iff\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"sub-list\"><config value=\"false\"/></list>"
            "    <must condition=\"cond\"/>"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"deprecated\"/>"
            "    <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "    <uses name=\"uses-name\"/>"
            "    <container name=\"cont\"/>"
            "    <choice name=\"choice\"/>"
            "    <grouping name=\"grp\"/>"
            EXT_SUBELEM
            "  </notification>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    notifs = mod->parsed->notifs;

    assert_string_equal(notifs->name, "notif-name");
    assert_string_equal(notifs->child->name, "anyd");
    assert_int_equal(notifs->child->nodetype, LYS_ANYDATA);
    assert_string_equal(notifs->child->next->name, "anyx");
    assert_int_equal(notifs->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(notifs->child->next->next->name, "leaf");
    assert_int_equal(notifs->child->next->next->nodetype, LYS_LEAF);
    assert_string_equal(notifs->child->next->next->next->name, "llist");
    assert_int_equal(notifs->child->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(notifs->child->next->next->next->next->name, "sub-list");
    assert_int_equal(notifs->child->next->next->next->next->nodetype, LYS_LIST);
    assert_true(notifs->flags & LYS_STATUS_DEPRC);
    assert_string_equal(notifs->groupings->name, "grp");
    assert_int_equal(notifs->groupings->nodetype, LYS_GROUPING);
    assert_string_equal(notifs->child->next->next->next->next->next->name, "uses-name");
    assert_int_equal(notifs->child->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(notifs->child->next->next->next->next->next->next->name, "cont");
    assert_int_equal(notifs->child->next->next->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_int_equal(notifs->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(notifs->child->next->next->next->next->next->next->next->name, "choice");
    assert_null(notifs->child->next->next->next->next->next->next->next->next);
    assert_string_equal(notifs->iffeatures[0].str, "iff");
    assert_string_equal(notifs->musts->arg.str, "cond");
    assert_int_equal(notifs->nodetype, LYS_NOTIF);
    assert_null(notifs->parent);
    assert_string_equal(notifs->ref, "ref");
    assert_string_equal(notifs->typedefs->name, "tpdf");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(notifs->exts[0]), LY_STMT_NOTIFICATION, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"notif-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:notif2\"/>"
            "  <prefix value=\"n2\"/>"
            "  <notification name=\"notif-name\" />"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    notifs = mod->parsed->notifs;
    assert_string_equal(notifs->name, "notif-name");
}

static void
test_grouping_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_grp *grps = NULL;

    /* max subelems */
    yin_data =
            "<module name=\"grp-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:grp1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:grp1\"/>"
            "  <prefix value=\"g1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <grouping name=\"grp-name\">"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <description><text>desc</text></description>"
            "    <grouping name=\"sub-grp\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"list\"><config value=\"false\"/></list>"
            "    <notification name=\"notf\"/>"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"current\"/>"
            "    <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "    <uses name=\"uses-name\"/>"
            "    <action name=\"act\"/>"
            "    <container name=\"cont\"/>"
            "    <choice name=\"choice\"/>"
            EXT_SUBELEM
            "  </grouping>"
            "  <grouping name=\"uses-name\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    grps = &mod->parsed->groupings[0];

    assert_string_equal(grps->name, "grp-name");
    assert_string_equal(grps->child->name, "anyd");
    assert_string_equal(grps->child->next->name, "anyx");
    assert_string_equal(grps->child->next->next->name, "leaf");
    assert_string_equal(grps->child->next->next->next->name, "llist");
    assert_string_equal(grps->child->next->next->next->next->name, "list");
    assert_string_equal(grps->dsc, "desc");
    assert_true(grps->flags & LYS_STATUS_CURR);
    assert_string_equal(grps->groupings->name, "sub-grp");
    assert_int_equal(grps->nodetype, LYS_GROUPING);
    assert_string_equal(grps->notifs->name, "notf");
    assert_null(grps->parent);
    assert_string_equal(grps->ref, "ref");
    assert_string_equal(grps->typedefs->name, "tpdf");
    assert_string_equal(grps->actions->name, "act");
    assert_string_equal(grps->child->next->next->next->next->next->name, "uses-name");
    assert_int_equal(grps->child->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(grps->child->next->next->next->next->next->next->name, "cont");
    assert_int_equal(grps->child->next->next->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(grps->child->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(grps->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(grps->exts[0]), LY_STMT_GROUPING, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"grp-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:grp2\"/>"
            "  <prefix value=\"g2\"/>"
            "  <grouping name=\"grp-name\" />"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    grps = mod->parsed->groupings;
    assert_string_equal(grps->name, "grp-name");
}

static void
test_container_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_container *parsed = NULL;
    uint16_t flags = LYS_CONFIG_W | LYS_STATUS_CURR;

    /* max subelems */
    yin_data =
            "<module name=\"cont-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:cont1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:cont1\"/>"
            "  <prefix value=\"c1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <leaf name=\"when-cond\"><type name=\"string\"/></leaf>"
            "  <container name=\"cont-name\">"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <config value=\"true\"/>"
            "    <container name=\"subcont\"/>"
            "    <description><text>desc</text></description>"
            "    <grouping name=\"grp\"/>"
            "    <if-feature name=\"iff\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"list\"><config value=\"false\"/></list>"
            "    <must condition=\"cond\"/>"
            "    <notification name=\"notf\"/>"
            "    <presence value=\"presence\"/>"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"current\"/>"
            "    <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "    <uses name=\"grp\"/>"
            "    <when condition=\"when-cond\"/>"
            "    <action name=\"act\"/>"
            "    <choice name=\"choice\"/>"
            EXT_SUBELEM
            "  </container>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    parsed = (struct lysp_node_container *)mod->parsed->data;
    while (parsed && strcmp(parsed->name, "cont-name") != 0) {
        parsed = (struct lysp_node_container *)parsed->next;
    }

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "cont-name", 0, LYS_CONTAINER, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_RESTR(parsed->musts, "cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);

    assert_string_equal(parsed->iffeatures[0].str, "iff");
    assert_string_equal(parsed->presence, "presence");
    assert_string_equal(parsed->typedefs->name, "tpdf");
    assert_string_equal(parsed->groupings->name, "grp");
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
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "grp");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    assert_string_equal(parsed->notifs->name, "notf");
    assert_string_equal(parsed->actions->name, "act");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_CONTAINER, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"cont-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:cont2\"/>"
            "  <prefix value=\"c2\"/>"
            "  <container name=\"cont-name\" />"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_container *)mod->parsed->data;
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "cont-name", 0, LYS_CONTAINER, 0, NULL);
    CHECK_POINTER(parsed->when, 0);
}

static void
test_case_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node *node = NULL;
    struct lysp_node_case *parsed = NULL;
    uint16_t flags = LYS_STATUS_CURR;

    /* max subelems */
    yin_data =
            "<module name=\"case-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:case1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:case1\"/>"
            "  <prefix value=\"c1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <grouping name=\"uses-name\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <choice name=\"ch\">"
            "    <case name=\"case-name\">"
            "      <anydata name=\"anyd\"/>"
            "      <anyxml name=\"anyx\"/>"
            "      <container name=\"subcont\"/>"
            "      <description><text>desc</text></description>"
            "      <if-feature name=\"iff\"/>"
            "      <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "      <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "      <list name=\"list\"><config value=\"false\"/></list>"
            "      <reference><text>ref</text></reference>"
            "      <status value=\"current\"/>"
            "      <uses name=\"uses-name\"/>"
            "      <when condition=\"/c1:cond\"/>"
            "      <choice name=\"choice\"/>"
            EXT_SUBELEM
            "    </case>"
            "  </choice>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    node = mod->parsed->data;
    while (node && strcmp(node->name, "ch") != 0) {
        node = node->next;
    }
    parsed = (struct lysp_node_case *)((struct lysp_node_choice *)node)->child;

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "case-name", 0, LYS_CASE, 1, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_WHEN(parsed->when, "/c1:cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "iff");
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
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_CASE, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"case-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:case2\"/>"
            "  <prefix value=\"c2\"/>"
            "  <choice name=\"ch\">"
            "    <case name=\"case-name\"/>"
            "  </choice>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_case *)((struct lysp_node_choice *)mod->parsed->data)->child;
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "case-name", 0, LYS_CASE, 1, NULL);
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 0);
}

static void
test_choice_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node *node = NULL;
    struct lysp_node_choice *parsed = NULL;
    uint16_t flags = LYS_CONFIG_W | LYS_MAND_TRUE | LYS_STATUS_CURR;

    /* max subelems */
    yin_data =
            "<module name=\"ch-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:ch1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:ch1\"/>"
            "  <prefix value=\"ch1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <choice name=\"choice-name\">"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <case name=\"sub-case\"/>"
            "    <choice name=\"choice\"/>"
            "    <config value=\"true\"/>"
            "    <container name=\"subcont\"/>"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"iff\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"list\"><config value=\"false\"/></list>"
            "    <mandatory value=\"true\" />"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"current\"/>"
            "    <when condition=\"/ch1:cond\"/>"
            EXT_SUBELEM
            "  </choice>"
            "  <choice name=\"choice2\">"
            "    <case name=\"sub-case\"/>"
            "    <default value=\"sub-case\"/>"
            "  </choice>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    node = mod->parsed->data;
    while (node && strcmp(node->name, "choice-name") != 0) {
        node = node->next;
    }
    parsed = (struct lysp_node_choice *)node;

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1, "choice-name", 1, LYS_CHOICE, 0, "ref");
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 1);
    CHECK_LYSP_WHEN(parsed->when, "/ch1:cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "iff");
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
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_CHOICE, "MY_MTU");

    parsed = (struct lysp_node_choice *)parsed->next;
    assert_string_equal(parsed->name, "choice2");
    assert_string_equal(parsed->dflt.str, "sub-case");

    /* min subelems */
    yin_data =
            "<module name=\"ch-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:ch2\"/>"
            "  <prefix value=\"ch2\"/>"
            "  <choice name=\"choice-name\" />"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    parsed = (struct lysp_node_choice *)mod->parsed->data;
    assert_string_equal(parsed->name, "choice-name");
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0, "choice-name", 0, LYS_CHOICE, 0, NULL);
    CHECK_POINTER(((struct lysp_node_leaf *)parsed)->when, 0);
}

static void
test_inout_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_action *rpc = NULL;
    struct lysp_node_action_inout *inout = NULL;

    /* max subelements */
    yin_data =
            "<module name=\"io-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:io1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:io1\"/>"
            "  <prefix value=\"io1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <grouping name=\"uses-name\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <rpc name=\"my-rpc\">"
            "    <input>"
            "      <anydata name=\"anyd\"/>"
            "      <anyxml name=\"anyx\"/>"
            "      <choice name=\"choice\"/>"
            "      <container name=\"subcont\"/>"
            "      <grouping name=\"sub-grp\"/>"
            "      <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "      <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "      <list name=\"list\"/>"
            "      <must condition=\"/io1:cond\"/>"
            "      <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "      <uses name=\"uses-name\"/>"
            EXT_SUBELEM
            "    </input>"
            "  </rpc>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    rpc = mod->parsed->rpcs;
    inout = &rpc->input;

    CHECK_LYSP_ACTION_INOUT(inout, 1, 1, 1, 1, LYS_INPUT, 1, 1);
    CHECK_LYSP_RESTR(inout->musts, "/io1:cond", NULL, NULL, NULL, 0, NULL);
    assert_string_equal(inout->typedefs->name, "tpdf");
    assert_string_equal(inout->groupings->name, "sub-grp");
    assert_string_equal(inout->child->name, "anyd");
    assert_int_equal(inout->child->nodetype, LYS_ANYDATA);
    assert_string_equal(inout->child->next->name, "anyx");
    assert_int_equal(inout->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(inout->child->next->next->name, "choice");
    assert_int_equal(inout->child->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(inout->child->next->next->next->name, "subcont");
    assert_int_equal(inout->child->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(inout->child->next->next->next->next->name, "leaf");
    assert_int_equal(inout->child->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(inout->child->next->next->next->next->next->name, "llist");
    assert_int_equal(inout->child->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(inout->child->next->next->next->next->next->next->name, "list");
    assert_int_equal(inout->child->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(inout->child->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(inout->child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(inout->child->next->next->next->next->next->next->next->next);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(inout->exts[0]), LY_STMT_INPUT, "MY_MTU");

    /* max subelements */
    yin_data =
            "<module name=\"io-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:io2\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:io2\"/>"
            "  <prefix value=\"io2\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <grouping name=\"uses-name\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <rpc name=\"my-rpc\">"
            "    <output>"
            "      <anydata name=\"anyd\"/>"
            "      <anyxml name=\"anyx\"/>"
            "      <choice name=\"choice\"/>"
            "      <container name=\"subcont\"/>"
            "      <grouping name=\"sub-grp\"/>"
            "      <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "      <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "      <list name=\"list\"/>"
            "      <must condition=\"/io2:cond\"/>"
            "      <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "      <uses name=\"uses-name\"/>"
            EXT_SUBELEM
            "    </output>"
            "  </rpc>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    rpc = mod->parsed->rpcs;
    inout = &rpc->output;

    CHECK_LYSP_ACTION_INOUT(inout, 1, 1, 1, 1, LYS_OUTPUT, 1, 1);
    assert_string_equal(inout->musts->arg.str, "/io2:cond");
    assert_string_equal(inout->typedefs->name, "tpdf");
    assert_string_equal(inout->groupings->name, "sub-grp");
    assert_string_equal(inout->child->name, "anyd");
    assert_int_equal(inout->child->nodetype, LYS_ANYDATA);
    assert_string_equal(inout->child->next->name, "anyx");
    assert_int_equal(inout->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(inout->child->next->next->name, "choice");
    assert_int_equal(inout->child->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(inout->child->next->next->next->name, "subcont");
    assert_int_equal(inout->child->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(inout->child->next->next->next->next->name, "leaf");
    assert_int_equal(inout->child->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(inout->child->next->next->next->next->next->name, "llist");
    assert_int_equal(inout->child->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(inout->child->next->next->next->next->next->next->name, "list");
    assert_int_equal(inout->child->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(inout->child->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(inout->child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(inout->exts[0]), LY_STMT_OUTPUT, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"io-mod3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:io3\"/>"
            "  <prefix value=\"i3\"/>"
            "  <rpc name=\"act\">"
            "    <input><leaf name=\"l\"><type name=\"empty\"/></leaf></input>"
            "  </rpc>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    yin_data =
            "<module name=\"io-mod4\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:io4\"/>"
            "  <prefix value=\"i4\"/>"
            "  <rpc name=\"act\">"
            "    <output><leaf name=\"l\"><type name=\"empty\"/></leaf></output>"
            "  </rpc>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    /* invalid combinations */
    yin_data =
            "<module name=\"io-mod5\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:io5\"/>"
            "  <prefix value=\"i5\"/>"
            "  <rpc name=\"act\">"
            "    <input name=\"test\"/>"
            "  </rpc>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"io-mod5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Unexpected attribute \"name\" of \"input\" element.", NULL, 1);
}

static void
test_action_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_action *actions = NULL;
    struct lysp_node_container *cont = NULL;
    uint16_t flags = LYS_STATUS_DEPRC;

    /* max subelems */
    yin_data =
            "<module name=\"act-mod1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:act1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:act1\"/>"
            "  <prefix value=\"a1\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <grouping name=\"uses-name\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <container name=\"c\">"
            "    <action name=\"act\">"
            "      <description><text>desc</text></description>"
            "      <grouping name=\"grouping\"/>"
            "      <if-feature name=\"iff\"/>"
            "      <input><uses name=\"uses-name\"/></input>"
            "      <output><must condition=\"/a1:cond\"/><leaf name=\"l\"><type name=\"string\"/></leaf></output>"
            "      <reference><text>ref</text></reference>"
            "      <status value=\"deprecated\"/>"
            "      <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            EXT_SUBELEM
            "    </action>"
            "  </container>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    cont = (struct lysp_node_container *)mod->parsed->data;
    while (cont && strcmp(cont->name, "c") != 0) {
        cont = (struct lysp_node_container *)cont->next;
    }
    actions = cont->actions;
    CHECK_LYSP_ACTION(actions, "desc", 1, flags, 1, 1, 1, 0, 0, 0, 1, 0,
            "act", LYS_ACTION, 1, 0, 0, 1, 1, 0, 1, "ref", 1);
    assert_string_equal(actions->iffeatures[0].str, "iff");
    assert_string_equal(actions->typedefs->name, "tpdf");
    assert_string_equal(actions->groupings->name, "grouping");
    assert_string_equal(actions->output.musts->arg.str, "/a1:cond");
    assert_string_equal(actions->input.child->name, "uses-name");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(actions->exts[0]), LY_STMT_ACTION, "MY_MTU");

    yin_data =
            "<module name=\"act-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:act2\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:act2\"/>"
            "  <prefix value=\"a2\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <grouping name=\"uses-name\"/>"
            "  <leaf name=\"cond\"><type name=\"string\"/></leaf>"
            "  <rpc name=\"act\">"
            "    <description><text>desc</text></description>"
            "    <grouping name=\"grouping\"/>"
            "    <if-feature name=\"iff\"/>"
            "    <input><uses name=\"uses-name\"/></input>"
            "    <output><must condition=\"/a2:cond\"/><leaf name=\"l\"><type name=\"string\"/></leaf></output>"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"deprecated\"/>"
            "    <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            EXT_SUBELEM
            "  </rpc>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    actions = mod->parsed->rpcs;
    CHECK_LYSP_ACTION(actions, "desc", 1, flags, 1, 1, 1, 0, 0, 0, 1, 0, "act", LYS_RPC, 1, 0, 0, 1, 1, 0, 0, "ref", 1);
    assert_string_equal(actions->iffeatures[0].str, "iff");
    assert_string_equal(actions->typedefs->name, "tpdf");
    assert_string_equal(actions->groupings->name, "grouping");
    assert_string_equal(actions->input.child->name, "uses-name");
    assert_string_equal(actions->output.musts->arg.str, "/a2:cond");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(actions->exts[0]), LY_STMT_RPC, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"act-mod4\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:act4\"/>"
            "  <prefix value=\"a4\"/>"
            "  <container name=\"c\">"
            "    <action name=\"act\" />"
            "  </container>"
            "</module>";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    cont = (struct lysp_node_container *)mod->parsed->data;
    actions = cont->actions;
    assert_string_equal(actions->name, "act");
}

static void
test_augment_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_node_augment *augments = NULL;

    yin_data =
            "<module name=\"aug-mod-test\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"urn:aug-test\">"
            "  <yang-version value=\"1.1\"/>"
            "  <namespace uri=\"urn:aug-test\"/>"
            "  <prefix value=\"aug-test\"/>"
            "  <extension name=\"c-define\">"
            "    <argument name=\"name\"/>"
            "  </extension>"
            "  <feature name=\"iff\"/>"
            "  <grouping name=\"uses\"/>"
            "  <leaf name=\"when-cond\">"
            "    <type name=\"string\"/>"
            "  </leaf>"
            "  <container name=\"target\"/>"
            "  <augment target-node=\"/aug-test:target\">"
            "    <action name=\"action\"/>"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <choice name=\"choice\">"
            "       <case name=\"case\"/>"
            "    </choice>"
            "    <container name=\"subcont\"/>"
            "    <description><text>desc</text></description>"
            "    <if-feature name=\"iff\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"list\"> <config value=\"false\"/> </list>"
            "    <notification name=\"notif\"/>"
            "    <reference><text>ref</text></reference>"
            "    <status value=\"current\"/>"
            "    <uses name=\"uses\"/>"
            "    <when condition=\"/aug-test:when-cond\"/>"
            EXT_SUBELEM
            "  </augment>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    augments = mod->parsed->augments;

    assert_string_equal(augments->nodeid, "/aug-test:target");
    assert_null(augments->parent);
    assert_int_equal(augments->nodetype, LYS_AUGMENT);
    assert_true(augments->flags & LYS_STATUS_CURR);
    assert_string_equal(augments->dsc, "desc");
    assert_string_equal(augments->ref, "ref");
    assert_string_equal(augments->when->cond, "/aug-test:when-cond");
    assert_string_equal(augments->iffeatures[0].str, "iff");
    assert_string_equal(augments->child->name, "anyd");
    assert_int_equal(augments->child->nodetype, LYS_ANYDATA);
    assert_string_equal(augments->child->next->name, "anyx");
    assert_int_equal(augments->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(augments->child->next->next->name, "choice");
    assert_int_equal(augments->child->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(((struct lysp_node_choice *)augments->child->next->next)->child->name, "case");
    assert_int_equal(((struct lysp_node_choice *)augments->child->next->next)->child->nodetype, LYS_CASE);
    assert_string_equal(augments->child->next->next->next->name, "subcont");
    assert_int_equal(augments->child->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(augments->child->next->next->next->next->name, "leaf");
    assert_int_equal(augments->child->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(augments->child->next->next->next->next->next->name, "llist");
    assert_int_equal(augments->child->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(augments->child->next->next->next->next->next->next->name, "list");
    assert_int_equal(augments->child->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(augments->child->next->next->next->next->next->next->next->name, "uses");
    assert_int_equal(augments->child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(augments->child->next->next->next->next->next->next->next->next);
    assert_string_equal(augments->actions->name, "action");
    assert_string_equal(augments->notifs->name, "notif");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(augments->exts[0]), LY_STMT_AUGMENT, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"aug-mod2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:aug2\"/>"
            "  <prefix value=\"a2\"/>"
            "  <container name=\"target\"/>"
            "  <augment target-node=\"/a2:target\" />"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    augments = mod->parsed->augments;
    assert_string_equal(augments->nodeid, "/a2:target");
}

static void
test_deviate_elem(void **state)
{
    const char *yin_data;

    /* invalid arguments */
    yin_data =
            "<module name=\"dev-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:dev1\"/><prefix value=\"d1\"/>"
            "  <container name=\"target\"/>"
            "  <deviation target-node=\"/d1:target\"><deviate value=\"\" /></deviation>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"dev-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);
    yin_data =
            "<module name=\"dev-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:dev2\"/><prefix value=\"d2\"/>"
            "  <container name=\"target\"/>"
            "  <deviation target-node=\"/d2:target\"><deviate value=\"invalid\" /></deviation>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"dev-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);
    yin_data =
            "<module name=\"dev-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:dev3\"/><prefix value=\"d3\"/>"
            "  <container name=\"target\"/>"
            "  <deviation target-node=\"/d3:target\"><deviate value=\"ad\" /></deviation>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"dev-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"ad\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);

    yin_data =
            "<module name=\"dev-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:dev4\"/><prefix value=\"d4\"/>"
            "  <container name=\"target\"/>"
            "  <deviation target-node=\"/d4:target\"><deviate value=\"adds\" /></deviation>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"dev-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"adds\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);
    yin_data =
            "<module name=\"dev-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:dev5\"/><prefix value=\"d5\"/>"
            "  <container name=\"target\"/>"
            "  <deviation target-node=\"/d5:target\">"
            "    <deviate value=\"not-supported\">"
            "      <must condition=\"c\"/>"
            "    </deviate>"
            "  </deviation>"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"dev-mod\" failed.", NULL, 0);
    CHECK_LOG_CTX("Deviate of this type doesn't allow \"must\" as it's sub-element.", NULL, 1);
}

static void
test_deviation_elem(void **state)
{
    const char *yin_data;

    /* invalid */
    yin_data =
            "<module name=\"dev\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:dev6\"/>"
            "  <prefix value=\"d6\"/>"
            "  <container name=\"target\"/>"
            "  <deviation target-node=\"/d6:target\"/>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"dev\" failed.", NULL, 0);
    CHECK_LOG_CTX("Missing mandatory sub-element \"deviate\" of \"deviation\" element.", NULL, 1);
}

static void
test_module_elem(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    struct lysp_module *lysp_mod = NULL;
    const char *a_mod =
            "<module name=\"a-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:a-mod\"/>"
            "  <prefix value=\"a\"/>"
            "</module>";
    char *b_mod =
            "<submodule name=\"b-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <belongs-to module=\"mod\">"
            "    <prefix value=\"pref\"/>"
            "  </belongs-to>"
            "</submodule>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, a_mod, LYS_IN_YIN, NULL));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, b_mod);

    /* max subelems */
    yin_data =
            "<module name=\"mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"ns\">"
            "    <yang-version value=\"1.1\"/>"
            "    <namespace uri=\"ns\"/>"
            "    <prefix value=\"pref\"/>"
            "    <include module=\"b-mod\"/>"
            "    <import module=\"a-mod\"><prefix value=\"imp-pref\"/></import>"
            "    <organization><text>org</text></organization>"
            "    <contact><text>contact</text></contact>"
            "    <description><text>desc</text></description>"
            "    <reference><text>ref</text></reference>"
            "    <revision date=\"2019-02-02\"/>"
            "    <extension name=\"c-define\">"
            "      <argument name=\"name\"/>"
            "    </extension>"
            "    <feature name=\"feature\"/>"
            "    <identity name=\"ident-name\"/>"
            "    <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "    <grouping name=\"uses-name\"/>"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <choice name=\"choice\"/>"
            "    <container name=\"cont\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"sub-list\"><config value=\"false\"/></list>"
            "    <uses name=\"uses-name\"/>"
            "    <augment target-node=\"/pref:cont\"/>"
            "    <deviation target-node=\"/pref:cont\">"
            "        <deviate value=\"not-supported\"/>"
            "    </deviation>"
            "    <notification name=\"notf\"/>"
            "    <rpc name=\"rpc-name\"/>"
            "    " EXT_SUBELEM ""
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    lysp_mod = mod->parsed;

    assert_string_equal(lysp_mod->mod->name, "mod");
    assert_string_equal(lysp_mod->revs[0].date, "2019-02-02");
    assert_string_equal(lysp_mod->mod->ns, "ns");
    assert_string_equal(lysp_mod->mod->prefix, "pref");
    assert_null(lysp_mod->mod->filepath);
    assert_string_equal(lysp_mod->mod->org, "org");
    assert_string_equal(lysp_mod->mod->contact, "contact");
    assert_string_equal(lysp_mod->mod->dsc, "desc");
    assert_string_equal(lysp_mod->mod->ref, "ref");
    assert_int_equal(lysp_mod->version, LYS_VERSION_1_1);
    CHECK_LYSP_IMPORT(lysp_mod->imports, NULL, 0, "a-mod", "imp-pref", NULL, "");
    assert_string_equal(lysp_mod->includes->name, "b-mod");
    assert_string_equal(lysp_mod->extensions->name, "c-define");
    assert_string_equal(lysp_mod->features->name, "feature");
    assert_string_equal(lysp_mod->identities->name, "ident-name");
    assert_string_equal(lysp_mod->typedefs->name, "tpdf");
    assert_string_equal(lysp_mod->groupings->name, "uses-name");
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
    assert_string_equal(lysp_mod->augments->nodeid, "/pref:cont");
    assert_string_equal(lysp_mod->rpcs->name, "rpc-name");
    assert_string_equal(lysp_mod->notifs->name, "notf");
    assert_string_equal(lysp_mod->deviations->nodeid, "/pref:cont");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(lysp_mod->exts[0]), LY_STMT_MODULE, "MY_MTU");

    /* min subelems */
    yin_data =
            "<module name=\"mod-min\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <namespace uri=\"ns2\"/>"
            "    <prefix value=\"pref2\"/>"
            "    <yang-version value=\"1.1\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    lysp_mod = mod->parsed;
    assert_string_equal(lysp_mod->mod->name, "mod-min");

    /* incorrect subelem order */
    yin_data =
            "<module name=\"mod-inv\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <feature name=\"feature\"/>"
            "    <namespace uri=\"ns3\"/>"
            "    <prefix value=\"pref3\"/>"
            "    <yang-version value=\"1.1\"/>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"mod-inv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid order of module\'s sub-elements \"namespace\" can\'t appear after \"feature\".", NULL, 1);
}

static void
test_submodule_elem(void **state)
{
    char *yin_data;
    const char *parent_data;
    struct lys_module *mod = NULL;
    struct lysp_submodule *lysp_submod = NULL;
    const char *a_mod =
            "<module name=\"a-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:a-mod\"/>"
            "  <prefix value=\"a\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, a_mod, LYS_IN_YIN, NULL));

    /* max subelements */
    yin_data =
            "<submodule name=\"mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"ns\">"
            "    <yang-version value=\"1.1\"/>"
            "    <belongs-to module=\"parent\">"
            "        <prefix value=\"pref\"/>"
            "    </belongs-to>"
            "    <import module=\"a-mod\"><prefix value=\"imp-pref\"/></import>"
            "    <organization><text>org</text></organization>"
            "    <contact><text>contact</text></contact>"
            "    <description><text>desc</text></description>"
            "    <reference><text>ref</text></reference>"
            "    <revision date=\"2019-02-02\"/>"
            "    <extension name=\"ext\"/>"
            "    <extension name=\"c-define\">"
            "      <argument name=\"name\"/>"
            "    </extension>"
            "    <feature name=\"feature\"/>"
            "    <grouping name=\"grp\"/>"
            "    <grouping name=\"uses-name\"/>"
            "    <identity name=\"ident-name\"/>"
            "    <typedef name=\"tpdf\"> <type name=\"string\"/> </typedef>"
            "    <anydata name=\"anyd\"/>"
            "    <anyxml name=\"anyx\"/>"
            "    <choice name=\"choice\"/>"
            "    <container name=\"cont\"/>"
            "    <leaf name=\"leaf\"> <type name=\"string\"/> </leaf>"
            "    <leaf-list name=\"llist\"> <type name=\"string\"/> </leaf-list>"
            "    <list name=\"sub-list\"><config value=\"false\"/></list>"
            "    <uses name=\"uses-name\"/>"
            "    <augment target-node=\"/pref:cont\"/>"
            "    <deviation target-node=\"/pref:cont\">"
            "        <deviate value=\"not-supported\"/>"
            "    </deviation>"
            "    <notification name=\"notf\"/>"
            "    <rpc name=\"rpc-name\"/>"
            "    " EXT_SUBELEM ""
            "</submodule>";

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, yin_data);
    parent_data =
            "<module name=\"parent\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:myext=\"ns\">"
            "  <namespace uri=\"ns\"/>"
            "  <prefix value=\"pref\"/>"
            "  <include module=\"mod\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, parent_data, LYS_IN_YIN, &mod));
    lysp_submod = mod->parsed->includes[0].submodule;
    assert_string_equal(lysp_submod->name, "mod");
    assert_string_equal(lysp_submod->revs[0].date, "2019-02-02");
    assert_string_equal(lysp_submod->prefix, "pref");
    assert_null(lysp_submod->filepath);
    assert_string_equal(lysp_submod->org, "org");
    assert_string_equal(lysp_submod->contact, "contact");
    assert_string_equal(lysp_submod->dsc, "desc");
    assert_string_equal(lysp_submod->ref, "ref");
    assert_int_equal(lysp_submod->version, LYS_VERSION_1_1);
    CHECK_LYSP_IMPORT(lysp_submod->imports, NULL, 0, "a-mod", "imp-pref", NULL, "");
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
    assert_string_equal(lysp_submod->augments->nodeid, "/pref:cont");
    assert_string_equal(lysp_submod->rpcs->name, "rpc-name");
    assert_string_equal(lysp_submod->notifs->name, "notf");
    assert_string_equal(lysp_submod->deviations->nodeid, "/pref:cont");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(lysp_submod->exts[0]), LY_STMT_SUBMODULE, "MY_MTU");

    /* min subelements */
    yin_data =
            "<submodule name=\"submod-min\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1\"/>"
            "    <belongs-to module=\"parent2\"><prefix value=\"pref\"/></belongs-to>"
            "</submodule>";

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, yin_data);

    parent_data =
            "<module name=\"parent2\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"ns2\"/>"
            "  <prefix value=\"pref2\"/>"
            "  <include module=\"submod-min\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, parent_data, LYS_IN_YIN, &mod));
    lysp_submod = mod->parsed->includes[0].submodule;

    assert_string_equal(lysp_submod->prefix, "pref");
    assert_int_equal(lysp_submod->version, LYS_VERSION_1_0);

    /* incorrect subelem order */
    yin_data =
            "<submodule name=\"submod-inv\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1\"/>"
            "    <reference><text>ref</text></reference>"
            "    <belongs-to module=\"parent3\"><prefix value=\"pref\"/></belongs-to>"
            "</submodule>";

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)yin_data);

    parent_data =
            "<module name=\"parent3\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"ns3\"/>"
            "  <prefix value=\"pref3\"/>"
            "  <include module=\"submod-inv\"/>"
            "</module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, parent_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"parent3\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"submod-inv\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid order of submodule's sub-elements \"belongs-to\" can't appear after \"reference\".", NULL, 1);
}

static void
test_yin_parse_module(void **state)
{
    const char *yin_data;
    struct lys_module *mod = NULL;
    const char *ext_mod =
            "<module name=\"example-extensions\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "  <namespace uri=\"urn:example:extensions\"/>"
            "  <prefix value=\"myext\"/>"
            "  <extension name=\"c-define\"><argument name=\"name\"/></extension>"
            "</module>";

    yin_data =
            "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"a\"> "
            "    <yang-version value=\"1.1\"/>"
            "    <namespace uri=\"urn:tests:extensions:metadata:a\"/>"
            "    <prefix value=\"a\"/>"
            "    <import module=\"ietf-yang-metadata\">"
            "        <prefix value=\"md\"/>"
            "    </import>"
            "    <feature name=\"f\"/>"
            "    <md:annotation name=\"x\">"
            "        <description>"
            "            <text>test</text>"
            "        </description>"
            "        <reference>"
            "            <text>test</text>"
            "        </reference>"
            "        <if-feature name=\"f\"/>"
            "        <status value=\"current\"/>"
            "        <type name=\"uint8\"/>"
            "        <units name=\"meters\"/>"
            "    </md:annotation>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));
    assert_null(mod->parsed->exts[0].child->next->child);
    assert_string_equal(mod->parsed->exts[0].child->next->arg, "test");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, ext_mod, LYS_IN_YIN, NULL));

    yin_data =
            "<module name=\"example-foo\""
            "    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
            "    xmlns:foo=\"urn:example:foo\""
            "    xmlns:myext=\"urn:example:extensions\">"
            "    <yang-version value=\"1\"/>"
            "    <namespace uri=\"urn:example:foo\"/>"
            "    <prefix value=\"foo\"/>"
            "    <import module=\"example-extensions\">"
            "        <prefix value=\"myext\"/>"
            "    </import>"
            "    <list name=\"interface\">"
            "        <key value=\"name\"/>"
            "        <leaf name=\"name\">"
            "            <type name=\"string\"/>"
            "        </leaf>"
            "        <leaf name=\"mtu\">"
            "            <type name=\"uint32\"/>"
            "            <description>"
            "                <text>The MTU of the interface.</text>"
            "            </description>"
            "            <myext:c-define name=\"MY_MTU\"/>"
            "        </leaf>"
            "    </list>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    yin_data =
            "<module name=\"example-foo-min\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1\"/>"
            "    <namespace uri=\"urn:example:foo-min\"/>"
            "    <prefix value=\"foo-min\"/>"
            "</module>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, &mod));

    yin_data = "<submodule name=\"example-foo-sub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"></submodule>";

    assert_int_equal(LY_EINVAL, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Input data contains submodule which cannot be parsed directly without its main module.", NULL, 0);

    yin_data =
            "<module name=\"example-foo-garb\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1\"/>"
            "    <namespace uri=\"urn:example:foo-garb\"/>"
            "    <prefix value=\"foo-garb\"/>"
            "</module>"
            "<module>";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, yin_data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"example-foo-garb\" failed.", NULL, 0);
    CHECK_LOG_CTX("Trailing garbage \"<module>\" after module, expected end-of-input.", NULL, 1);
}

static void
test_yin_parse_submodule(void **state)
{
    struct lys_module *mod;
    char *yin_data;
    const char *mod_a =
            "<module name=\"a\""
            "    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
            "    xmlns:a=\"urn:a\">"
            "    <yang-version value=\"1\"/>"
            "    <namespace uri=\"urn:a\"/>"
            "    <prefix value=\"a\"/>"
            "    <include module=\"asub\"/>"
            "    <feature name=\"bar\"/>"
            "    <container name=\"top\"/>"
            "</module>";

    yin_data =
            "<submodule name=\"asub\""
            "    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
            "    xmlns:a=\"urn:a\">"
            "    <yang-version value=\"1\"/>"
            "    <belongs-to module=\"a\">"
            "        <prefix value=\"a_pref\"/>"
            "    </belongs-to>"
            "    <feature name=\"fox\"/>"
            "    <notification name=\"bar-notif\">"
            "        <if-feature name=\"bar\"/>"
            "    </notification>"
            "    <notification name=\"fox-notif\">"
            "        <if-feature name=\"fox\"/>"
            "    </notification>"
            "    <augment target-node=\"/a_pref:top\">"
            "        <if-feature name=\"bar\"/>"
            "        <container name=\"bar-sub\"/>"
            "    </augment>"
            "    <augment target-node=\"/a_pref:top\">"
            "        <container name=\"bar-sub2\"/>"
            "    </augment>"
            "</submodule>";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, yin_data);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_a, LYS_IN_YIN, &mod));
    UTEST_TEARDOWN;
    UTEST_SETUP;

    yin_data =
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1\"/>"
            "    <belongs-to module=\"a\">"
            "        <prefix value=\"a_pref\"/>"
            "    </belongs-to>"
            "</submodule>";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, yin_data);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_a, LYS_IN_YIN, &mod));
    UTEST_TEARDOWN;
    UTEST_SETUP;

    yin_data =
            "<module name=\"inval\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "</module>";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, yin_data);
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, mod_a, LYS_IN_YIN, &mod), LY_EINVAL);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"asub\" failed.", NULL, 0);
    CHECK_LOG_CTX("Input data contains module when a submodule is expected.", NULL, 0);

    UTEST_TEARDOWN;
    UTEST_SETUP;

    yin_data =
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1\"/>"
            "    <belongs-to module=\"a\">"
            "        <prefix value=\"a_pref\"/>"
            "    </belongs-to>"
            "</submodule>"
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "    <yang-version value=\"1\"/>"
            "    <belongs-to module=\"a\">"
            "        <prefix value=\"a_pref\"/>"
            "    </belongs-to>"
            "</submodule>";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, yin_data);
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, mod_a, LYS_IN_YIN, &mod), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL, 0);
    CHECK_LOG_CTX("Parsing submodule \"asub\" failed.", NULL, 0);
    CHECK_LOG_CTX("Trailing garbage \"<submodule name...\" after submodule, expected end-of-input.", NULL, 1);
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        UTEST(test_yin_match_keyword),
        UTEST(test_yin_parse_content),
        UTEST(test_validate_value),
        UTEST(test_valid_module),
        UTEST(test_print_module),
        UTEST(test_print_submodule),
        UTEST(test_enum_elem),
        UTEST(test_bit_elem),
        UTEST(test_status_elem),
        UTEST(test_yin_element_elem),
        UTEST(test_yangversion_elem),
        UTEST(test_argument_elem),
        UTEST(test_belongsto_elem),
        UTEST(test_config_elem),
        UTEST(test_default_elem),
        UTEST(test_err_app_tag_elem),
        UTEST(test_err_msg_elem),
        UTEST(test_fracdigits_elem),
        UTEST(test_iffeature_elem),
        UTEST(test_length_elem),
        UTEST(test_modifier_elem),
        UTEST(test_namespace_elem),
        UTEST(test_pattern_elem),
        UTEST(test_value_position_elem),
        UTEST(test_prefix_elem),
        UTEST(test_range_elem),
        UTEST(test_reqinstance_elem),
        UTEST(test_revision_date_elem),
        UTEST(test_unique_elem),
        UTEST(test_units_elem),
        UTEST(test_yin_text_value_elem),
        UTEST(test_max_elems_elem),
        UTEST(test_min_elems_elem),
        UTEST(test_ordby_elem),
        UTEST(test_any_elem),
        UTEST(test_leaf_elem),
        UTEST(test_leaf_list_elem),
        UTEST(test_presence_elem),
        UTEST(test_key_elem),
        UTEST(test_uses_elem),
        UTEST(test_list_elem),
        UTEST(test_notification_elem),
        UTEST(test_grouping_elem),
        UTEST(test_container_elem),
        UTEST(test_case_elem),
        UTEST(test_choice_elem),
        UTEST(test_inout_elem),
        UTEST(test_action_elem),
        UTEST(test_augment_elem),
        UTEST(test_deviate_elem),
        UTEST(test_deviation_elem),
        UTEST(test_module_elem),
        UTEST(test_submodule_elem),
        UTEST(test_yin_parse_module),
        UTEST(test_yin_parse_submodule),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
