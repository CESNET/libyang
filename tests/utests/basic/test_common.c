/**
 * @file test_common.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from common.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

static void
test_utf8(void **state)
{
    int target_idx = 59;
    struct ly_ctx *ctx = UTEST_LYCTX;
    /* array coresponds to the following YANG module: module test { namespace "urn:test"; prefix t; description "XXX"; } */
    char test_yang[] = {
        0x6d, 0x6f, 0x64, 0x75, 0x6c, 0x65, 0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x7b, 0x20,
        0x6e, 0x61, 0x6d, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x20, 0x22, 0x75, 0x72, 0x6e, 0x3a, 0x74, 0x65, 0x73, 0x74, 0x22, 0x3b, 0x20,
        0x70, 0x72, 0x65, 0x66, 0x69, 0x78, 0x20, 0x74, 0x3b, 0x20,
        0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x22,
        0x58, 0x58, 0x58, 0x58,
        0x22, 0x3b, 0x20,
        0x7d, 0x00
    };

    lys_parse_mem(ctx, test_yang, LYS_IN_YANG, NULL);

    /* test invalid UTF-8 characters in lyxml_getutf8
    * - https://en.wikipedia.org/wiki/UTF-8 */
    test_yang[target_idx] = (char)0x04;

    /* invalid values */
    assert_int_equal(LY_EVALID, lys_parse_mem(ctx, test_yang, LYS_IN_YANG, NULL));
    UTEST_LOG_CTX_CLEAN;

    test_yang[target_idx] = (char)0xc0;
    test_yang[target_idx + 1] = (char)0x20;
    assert_int_equal(LY_EVALID, lys_parse_mem(ctx, test_yang, LYS_IN_YANG, NULL));
    UTEST_LOG_CTX_CLEAN;

    test_yang[target_idx + 1] = (char)0x80;
    assert_int_equal(LY_EVALID, lys_parse_mem(ctx, test_yang, LYS_IN_YANG, NULL));
    UTEST_LOG_CTX_CLEAN;

    test_yang[target_idx] = (char)0xe0;
    test_yang[target_idx + 1] = (char)0x20;
    test_yang[target_idx + 2] = (char)0x80;
    assert_int_equal(LY_EVALID, lys_parse_mem(ctx, test_yang, LYS_IN_YANG, NULL));
    UTEST_LOG_CTX_CLEAN;

    test_yang[target_idx] = (char)0xf0;
    test_yang[target_idx + 1] = (char)0x20;
    test_yang[target_idx + 2] = (char)0x80;
    test_yang[target_idx + 3] = (char)0x80;
    assert_int_equal(LY_EVALID, lys_parse_mem(ctx, test_yang, LYS_IN_YANG, NULL));
    UTEST_LOG_CTX_CLEAN;
}

static void
test_parse_int(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lys_module *mod = NULL;
    struct lyd_node *node = NULL;
    const char *str;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " leaf l {"
            "    type int8;"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, &mod));

    /* correct value */
    str = "10";
    assert_int_equal(LY_SUCCESS, lyd_new_term(NULL, mod, "l", str, 0, &node));
    assert_string_equal("10", lyd_get_value(node));

    /* leading zeros are allowed, trailing whitespaces are allowed */
    str = "000\n\t  ";
    assert_int_equal(LY_SUCCESS, lyd_change_term(node, str));
    assert_string_equal("0", lyd_get_value(node));

    /* negative value */
    str = "-10";
    assert_int_equal(LY_SUCCESS, lyd_change_term(node, str));
    assert_string_equal("-10", lyd_get_value(node));

    /* out of bounds value */
    str = "128";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    str = "-129";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    /* NaN */
    str = "zero";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    /* mixing number with text */
    str = "10zero";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    str = "10  zero";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    lyd_free_all(node);
}

static void
test_parse_uint(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lys_module *mod = NULL;
    struct lyd_node *node = NULL;
    const char *str;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " leaf l {"
            "    type uint8;"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, &mod));
    /* correct value */
    str = "10";
    assert_int_equal(LY_SUCCESS, lyd_new_term(NULL, mod, "l", str, 0, &node));
    assert_string_equal("10", lyd_get_value(node));

    /* leading zeros are allowed, trailing whitespaces are allowed */
    str = "000\n\t  ";
    assert_int_equal(LY_SUCCESS, lyd_change_term(node, str));
    assert_string_equal("0", lyd_get_value(node));

    /* out of bounds value */
    str = "256";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    str = "-1";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    /* NaN */
    str = "zero";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    /* mixing number with text */
    str = "10zero";
    assert_int_equal(LY_EVALID, lyd_change_term(node, str));
    UTEST_LOG_CTX_CLEAN;

    lyd_free_all(node);
}

static void
test_parse_nodeid(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lys_module *mod_data = NULL;
    struct lys_module *mod_meta = NULL;
    struct lyd_node *node = NULL;
    struct lyd_meta *meta = NULL;
    const char *str;
    const char *yang_meta =
            "module my-meta {"
            " namespace \"urn:my-meta\";"
            " prefix mm;"
            " import ietf-yang-metadata { prefix md; }"
            " md:annotation a12_-. {"
            "   type string;"
            " }\n"
            "}";
    const char *yang_data =
            "module my-data {"
            " namespace \"urn:my-data\";"
            " prefix md;"
            " leaf interface { type string; }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang_meta, LYS_IN_YANG, &mod_meta));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang_data, LYS_IN_YANG, &mod_data));

    /* invalid starting character */
    lyd_new_path(NULL, ctx, "/my-data:interface", "eth0", 0, &node);
    str = "123";
    assert_int_equal(LY_EINVAL, lyd_new_meta(ctx, node, mod_meta, str, "delete", 0, &meta));
    UTEST_LOG_CTX_CLEAN;

    /* invalid character in the name */
    str = "a12_-.!";
    assert_int_equal(LY_EINVAL, lyd_new_meta(ctx, node, mod_meta, str, "delete", 0, &meta));
    UTEST_LOG_CTX_CLEAN;

    /* correct name */
    str = "a12_-.";
    assert_int_equal(LY_SUCCESS, lyd_new_meta(ctx, node, mod_meta, str, "delete", 0, &meta));
    lyd_free_all(node);
}

static void
test_parse_instance_predicate(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lys_module *mod = NULL;
    struct lyd_node *node = NULL;
    struct lyd_node *tree = NULL;
    struct ly_set *set = NULL;
    const char *yang = "module test {"
            " namespace \"urn:test\";"
            " prefix t;"
            " list user {"
            "   key \"name\";"
            "   leaf name { "
            "    type string;"
            "   }"
            " } "
            " list router {"
            "   key \"ip port\";"
            "   leaf ip { type string; }"
            "   leaf port { type uint16; }"
            " } "
            " leaf-list cipher {"
            "   type string;"
            " }"
            " leaf l {"
            "   type instance-identifier {"
            "     require-instance false;"
            "   }"
            " }"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, &mod));

    /* correct predicates */
    assert_int_equal(LY_SUCCESS, lyd_new_term(NULL, mod, "l", "/test:user[name='fred']", 0, &node));
    lyd_free_all(node);
    assert_int_equal(LY_SUCCESS, lyd_new_term(NULL, mod, "l", "/test:router[ip='192.0.2.1'][port='80']", 0, &node));
    lyd_free_all(node);
    assert_int_equal(LY_SUCCESS, lyd_new_term(NULL, mod, "l", "/test:cipher[. = 'blowfish-cbc']", 0, &node));
    lyd_free_all(node);

    assert_int_equal(LY_SUCCESS, lyd_new_path(NULL, ctx, "/test:user[name='alice']", NULL, 0, &tree));
    assert_int_equal(LY_SUCCESS, lyd_new_path(tree, ctx, "/test:user[name='bob']", NULL, 0, NULL));
    assert_int_equal(LY_SUCCESS, lyd_new_path(tree, ctx, "/test:user[name='cyril']", NULL, 0, NULL));

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/test:user[ 3 ]", &set));
    assert_non_null(set);
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    /* invalid predicates */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/test:user[ 0 ]", &set));
    assert_non_null(set);
    assert_int_equal(0, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/test:user[ -1 ]", &set));
    assert_non_null(set);
    assert_int_equal(0, set->count);
    ly_set_free(set, NULL);

    /* invalid node-identifier */
    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:user[$name='fred']", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:user[.name='fred']", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:user[13name='fred']", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:user[name]", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:user[name=  fred]", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:user[name='fred\"]", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:user[name=\"fred\"[3]", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    assert_int_equal(LY_EVALID, lyd_new_term(NULL, mod, "l", "/test:cipher[.=\"value\"[3]", 0, &node));
    UTEST_LOG_CTX_CLEAN;

    lyd_free_all(tree);
}

/**
 * @brief Helper function to test token translation.
 *
 * @param token The input token to translate.
 * @param tok_len The length of the input token.
 * @param is_nametest Whether the token is a name test.
 * @param mod The YANG module.
 * @param expected_ret The expected return value.
 * @param expected_out The expected output token.
 */
static void
assert_token_translation(const char *token, uint16_t tok_len, ly_bool is_nametest,
        struct lys_module *mod, LY_ERR expected_ret)
{
    char *out_token = NULL;
    const struct lys_module *context_mod = mod;

    LY_ERR ret = lyplg_type_xpath10_print_token(
            token,
            tok_len,
            is_nametest,
            &context_mod,
            mod->ctx,
            LY_VALUE_SCHEMA,
            mod->parsed,
            LY_VALUE_JSON,
            NULL,
            &out_token,
            NULL);

    assert_int_equal(expected_ret, ret);
    free(out_token);
}

static void
test_value_prefix_next(void **state)
{
    struct ly_ctx *ctx = UTEST_LYCTX;
    struct lys_module *mod = NULL;
    const char *yang =
            "module test {"
            "  namespace \"urn:test\";"
            "  prefix pref;"
            "}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, yang, LYS_IN_YANG, &mod));

    /* Empty/NULL token */
    assert_token_translation("", 0, 1, mod, LY_SUCCESS);

    /* Valid prefix with node (Prefix translation) */
    assert_token_translation("pref:node", 9, 1, mod, LY_SUCCESS);

    /* Valid prefix only (Nametest ending in colon) */
    assert_token_translation("pref:", 5, 1, mod, LY_SUCCESS);

    /* No prefix (Standard node) */
    assert_token_translation("node", 4, 1, mod, LY_SUCCESS);

    /* Unresolved prefix */
    assert_token_translation("pref1:node", 10, 1, mod, LY_EVALID);
    UTEST_LOG_CTX_CLEAN;

    /* Invalid token syntax (e.g., "::::") */
    assert_token_translation("::::", 4, 1, mod, LY_SUCCESS);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_utf8),
        UTEST(test_parse_int),
        UTEST(test_parse_uint),
        UTEST(test_parse_nodeid),
        UTEST(test_parse_instance_predicate),
        UTEST(test_value_prefix_next),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
