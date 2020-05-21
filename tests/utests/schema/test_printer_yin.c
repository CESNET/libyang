/*
 * @file test_printer_yin.c
 * @author: Fred Gan <ganshaolong@vip.qq.com>
 * @brief unit tests for functions from printer_yin.c
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
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

#include "../../src/context.h"
#include "../../src/printer_schema.h"

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

static int
logger_setup(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif
    return 0;
}

static int
logger_teardown(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
#endif
    return 0;
}

void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif


static void
test_module(void **state)
{
    *state = test_module;

    struct ly_ctx *ctx = {0};
    const struct lys_module *mod;

    const char * orig =
            "module all {\n"
            "    yang-version 1.1;\n"
            "    namespace \"urn:all\";\n"
            "    prefix all_mod;\n\n"
            "    import ietf-yang-types {\n"
            "        prefix yt;\n"
            "        revision-date 2013-07-15;\n"
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
            "        }\n\n"
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
            "}\n";


    const char * ori_res =
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
            "    <revision-date date=\"2013-07-15\"/>\n"
            "    <description>\n"
            "      <text>YANG types</text>\n"
            "    </description>\n"
            "    <reference>\n"
            "      <text>RFC reference</text>\n"
            "    </reference>\n"
            "  </import>\n\n"
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
            "    </leaf>\n"
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
            "</module>\n";

    char *printed;
    struct lyp_out *out;

    assert_non_null(out = lyp_new_memory(&printed, 0));
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, orig, LYS_IN_YANG));
    assert_int_equal(strlen(ori_res), lys_print(out, mod, LYS_OUT_YIN, 0, 0));
    assert_string_equal(printed, ori_res);

    /*
    lyp_memory_clean(out);
    assert_int_equal(strlen(compiled), lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0, 0));
    assert_string_equal(printed, compiled);
    */

    /* note that the printed is freed here, so it must not be freed via lyp_free()! */
    free(printed);

    *state = NULL;
    lyp_free(out, NULL, 0);
    ly_ctx_destroy(ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_module, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

