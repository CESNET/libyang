/*
 * @file set.c
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

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

#include "context.h"
#include "log.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

#include "test_schema.h"

void
test_getnext(void **state)
{
    *state = test_getnext;

    struct ly_ctx *ctx;
    const struct lys_module *mod;
    const struct lysc_node *node = NULL, *four;
    const struct lysc_node_container *cont;
    const struct lysc_action *rpc;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, "module a {yang-version 1.1; namespace urn:a;prefix a;"
                                        "container a { container one {presence test;} leaf two {type string;} leaf-list three {type string;}"
                                        "  list four {config false;} choice x { leaf five {type string;} case y {leaf six {type string;}}}"
                                        "  anyxml seven; action eight {input {leaf eight-input {type string;}} output {leaf eight-output {type string;}}}"
                                        "  notification nine {leaf nine-data {type string;}}}"
                                        "leaf b {type string;} leaf-list c {type string;} list d {config false;}"
                                        "choice x { case empty-x { choice empty-xc { case nothing;}} leaf e {type string;} case y {leaf f {type string;}}} anyxml g;"
                                        "rpc h {input {leaf h-input {type string;}} output {leaf h-output {type string;}}}"
                                        "rpc i;"
                                        "notification j {leaf i-data {type string;}}"
                                        "notification k;}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("a", node->name);
    cont = (const struct lysc_node_container*)node;
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("b", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("c", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("d", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("e", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("f", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("g", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("h", node->name);
    rpc = (const struct lysc_action*)node;
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("i", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("j", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("k", node->name);
    assert_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    /* Inside container */
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("one", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("two", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("three", node->name);
    assert_non_null(node = four = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("four", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("five", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("six", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("seven", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("eight", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    assert_string_equal("nine", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, 0));
    /* Inside RPC */
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)rpc, mod->compiled, 0));
    assert_string_equal("h-input", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node*)rpc, mod->compiled, 0));

    /* options */
    assert_non_null(node = lys_getnext(four, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCHOICE));
    assert_string_equal("x", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCHOICE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(four, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_NOCHOICE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(four, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("five", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("y", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node*)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, LYS_GETNEXT_INTONPCONT));
    assert_string_equal("one", node->name);

    assert_non_null(node = lys_getnext(NULL, (const struct lysc_node*)rpc, mod->compiled, LYS_GETNEXT_OUTPUT));
    assert_string_equal("h-output", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node*)rpc, mod->compiled, LYS_GETNEXT_OUTPUT));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, "module c {namespace urn:c;prefix c; rpc c;}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, 0));
    assert_string_equal("c", node->name);
    assert_null(node = lys_getnext(node, NULL, mod->compiled, 0));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, "module d {namespace urn:d;prefix d; notification d;}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, 0));
    assert_string_equal("d", node->name);
    assert_null(node = lys_getnext(node, NULL, mod->compiled, 0));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, "module e {namespace urn:e;prefix e; container c {container cc;} leaf a {type string;}}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, 0));
    assert_string_equal("c", node->name);
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, LYS_GETNEXT_INTONPCONT));
    assert_string_equal("a", node->name);

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}
void
test_date(void **state)
{
    *state = test_date;

    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, NULL, 0, "date"));
    logbuf_assert("Invalid argument date (lysp_check_date()).");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "x", 1, "date"));
    logbuf_assert("Invalid argument date_len (lysp_check_date()).");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "nonsencexx", 10, "date"));
    logbuf_assert("Invalid value \"nonsencexx\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "123x-11-11", 10, "date"));
    logbuf_assert("Invalid value \"123x-11-11\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-13-11", 10, "date"));
    logbuf_assert("Invalid value \"2018-13-11\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-11-41", 10, "date"));
    logbuf_assert("Invalid value \"2018-11-41\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02-29", 10, "date"));
    logbuf_assert("Invalid value \"2018-02-29\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018.02-28", 10, "date"));
    logbuf_assert("Invalid value \"2018.02-28\" of \"date\".");
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02.28", 10, "date"));
    logbuf_assert("Invalid value \"2018-02.28\" of \"date\".");

    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-11-11", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-02-28", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2016-02-29", 10, "date"));

    *state = NULL;
}

void
test_revisions(void **state)
{
    (void) state; /* unused */

    struct lysp_revision *revs = NULL, *rev;

    logbuf_clean();
    /* no error, it just does nothing */
    lysp_sort_revisions(NULL);
    logbuf_assert("");

    /* revisions are stored in wrong order - the newest is the last */
    LY_ARRAY_NEW_RET(NULL, revs, rev,);
    strcpy(rev->date, "2018-01-01");
    LY_ARRAY_NEW_RET(NULL, revs, rev,);
    strcpy(rev->date, "2018-12-31");

    assert_int_equal(2, LY_ARRAY_COUNT(revs));
    assert_string_equal("2018-01-01", &revs[0]);
    assert_string_equal("2018-12-31", &revs[1]);
    /* the order should be fixed, so the newest revision will be the first in the array */
    lysp_sort_revisions(revs);
    assert_string_equal("2018-12-31", &revs[0]);
    assert_string_equal("2018-01-01", &revs[1]);

    LY_ARRAY_FREE(revs);
}

void
test_typedef(void **state)
{
    *state = test_typedef;

    struct ly_ctx *ctx = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    str = "module a {namespace urn:a; prefix a; typedef binary {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"binary\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef bits {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"bits\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef boolean {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"boolean\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef decimal64 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"decimal64\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef empty {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"empty\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef enumeration {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"enumeration\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef int8 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"int8\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef int16 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"int16\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef int32 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"int32\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef int64 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"int64\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef instance-identifier {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"instance-identifier\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef identityref {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"identityref\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef leafref {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"leafref\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef string {type int8;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"string\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef union {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"union\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef uint8 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"uint8\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef uint16 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"uint16\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef uint32 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"uint32\" of typedef - name collision with a built-in type. Line number 1.");
    str = "module a {namespace urn:a; prefix a; typedef uint64 {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"uint64\" of typedef - name collision with a built-in type. Line number 1.");

    str = "module mytypes {namespace urn:types; prefix t; typedef binary_ {type string;} typedef bits_ {type string;} typedef boolean_ {type string;} "
          "typedef decimal64_ {type string;} typedef empty_ {type string;} typedef enumeration_ {type string;} typedef int8_ {type string;} typedef int16_ {type string;}"
          "typedef int32_ {type string;} typedef int64_ {type string;} typedef instance-identifier_ {type string;} typedef identityref_ {type string;}"
          "typedef leafref_ {type string;} typedef string_ {type int8;} typedef union_ {type string;} typedef uint8_ {type string;} typedef uint16_ {type string;}"
          "typedef uint32_ {type string;} typedef uint64_ {type string;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);

    str = "module a {namespace urn:a; prefix a; typedef test {type string;} typedef test {type int8;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"test\" of typedef - name collision with another top-level type. Line number 1.");

    str = "module a {namespace urn:a; prefix a; typedef x {type string;} container c {typedef x {type int8;}}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"x\" of typedef - scoped type collide with a top-level type. Line number 1.");

    str = "module a {namespace urn:a; prefix a; container c {container d {typedef y {type int8;}} typedef y {type string;}}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"y\" of typedef - name collision with another scoped type. Line number 1.");

    str = "module a {namespace urn:a; prefix a; container c {typedef y {type int8;} typedef y {type string;}}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"y\" of typedef - name collision with sibling type. Line number 1.");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type string;}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"x\" of typedef - name collision with another top-level type. Line number 1.");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule b {belongs-to a {prefix a;} container c {typedef x {type string;}}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"x\" of typedef - scoped type collide with a top-level type. Line number 1.");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type int8;}}");
    str = "module a {namespace urn:a; prefix a; include b; container c {typedef x {type string;}}}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid name \"x\" of typedef - scoped type collide with a top-level type. Line number 1.");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

void
test_accessible_tree(void **state)
{
    *state = test_accessible_tree;

    struct ly_ctx *ctx = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));
    logbuf_clean();

    /* config -> config */
    str =
        "module a {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
            "container cont2 {"
                "leaf l2 {"
                    "must ../../cont/l;"
                    "type leafref {"
                        "path /cont/l;"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("");

    /* config -> state leafref */
    str =
        "module b {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "config false;"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
            "container cont2 {"
                "leaf l2 {"
                    "type leafref {"
                        "path /cont/l;"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Invalid leafref path \"/cont/l\" - target is supposed to represent configuration data"
        " (as the leafref does), but it does not. /b:cont2/l2");
    logbuf_clean();

    /* config -> state must */
    str =
        "module b {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "config false;"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
            "container cont2 {"
                "leaf l2 {"
                    "must ../../cont/l;"
                    "type empty;"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("Schema node \"l\" not found (../../cont/) with context node \"/b:cont2/l2\".");
    logbuf_clean();

    /* state -> config */
    str =
        "module c {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
            "container cont2 {"
                "config false;"
                "leaf l2 {"
                    "must ../../cont/l;"
                    "type leafref {"
                        "path /cont/l;"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("");

    /* notif -> state */
    str =
        "module d {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "config false;"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
            "notification notif {"
                "leaf l2 {"
                    "must ../../cont/l;"
                    "type leafref {"
                        "path /cont/l;"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("");

    /* notif -> notif */
    str =
        "module e {"
            "namespace urn:a;"
            "prefix a;"
            "notification notif {"
                "leaf l {"
                    "type empty;"
                "}"
                "leaf l2 {"
                    "must ../../notif/l;"
                    "type leafref {"
                        "path /notif/l;"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("");

    /* rpc input -> state */
    str =
        "module f {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "config false;"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
            "rpc rp {"
                "input {"
                    "leaf l2 {"
                        "must ../../cont/l;"
                        "type leafref {"
                            "path /cont/l;"
                        "}"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("");

    /* rpc input -> rpc input */
    str =
        "module g {"
            "namespace urn:a;"
            "prefix a;"
            "rpc rp {"
                "input {"
                    "leaf l {"
                        "type empty;"
                    "}"
                    "leaf l2 {"
                        "must ../l;"
                        "type leafref {"
                            "path /rp/l;"
                        "}"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("");

    /* rpc input -> rpc output leafref */
    str =
        "module h {"
            "namespace urn:a;"
            "prefix a;"
            "rpc rp {"
                "input {"
                    "leaf l2 {"
                        "type leafref {"
                            "path /rp/l;"
                        "}"
                    "}"
                "}"
                "output {"
                    "leaf l {"
                        "type empty;"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Not found node \"l\" in path. /h:rp/l2");
    logbuf_clean();

    /* rpc input -> rpc output must */
    str =
        "module h {"
            "namespace urn:a;"
            "prefix a;"
            "rpc rp {"
                "input {"
                    "leaf l2 {"
                        "must ../l;"
                        "type empty;"
                    "}"
                "}"
                "output {"
                    "leaf l {"
                        "type empty;"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("Schema node \"l\" not found (../) with context node \"/h:rp/l2\".");
    logbuf_clean();

    /* rpc input -> notif leafref */
    str =
        "module i {"
            "namespace urn:a;"
            "prefix a;"
            "rpc rp {"
                "input {"
                    "leaf l2 {"
                        "type leafref {"
                            "path ../../notif/l;"
                        "}"
                    "}"
                "}"
            "}"
            "notification notif {"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Not found node \"notif\" in path. /i:rp/l2");
    logbuf_clean();

    /* rpc input -> notif must */
    str =
        "module i {"
            "namespace urn:a;"
            "prefix a;"
            "rpc rp {"
                "input {"
                    "leaf l2 {"
                        "must /notif/l;"
                        "type empty;"
                    "}"
                "}"
            "}"
            "notification notif {"
                "leaf l {"
                    "type empty;"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("Schema node \"l\" not found (/notif/) with context node \"/i:rp/l2\".");
    logbuf_clean();

    /* action output -> state */
    str =
        "module j {"
            "yang-version 1.1;"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "list ll {"
                    "key k;"
                    "leaf k {"
                        "type string;"
                    "}"
                    "action act {"
                        "output {"
                            "leaf l2 {"
                                "must /cont/l;"
                                "type leafref {"
                                    "path ../../../l;"
                                "}"
                            "}"
                        "}"
                    "}"
                "}"
                "leaf l {"
                    "config false;"
                    "type empty;"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("");

    /* action output -> action input leafref */
    str =
        "module k {"
            "yang-version 1.1;"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "list ll {"
                    "key k;"
                    "leaf k {"
                        "type string;"
                    "}"
                    "action act {"
                        "input {"
                            "leaf l {"
                                "type empty;"
                            "}"
                        "}"
                        "output {"
                            "leaf l2 {"
                                "type leafref {"
                                    "path ../l;"
                                "}"
                            "}"
                        "}"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Not found node \"l\" in path. /k:cont/ll/act/l2");
    logbuf_clean();

    /* action output -> action input must */
    str =
        "module k {"
            "yang-version 1.1;"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "list ll {"
                    "key k;"
                    "leaf k {"
                        "type string;"
                    "}"
                    "action act {"
                        "input {"
                            "leaf l {"
                                "type empty;"
                            "}"
                        "}"
                        "output {"
                            "leaf l2 {"
                                "must /cont/ll/act/l;"
                                "type empty;"
                            "}"
                        "}"
                    "}"
                "}"
            "}"
        "}";
    assert_int_equal(lys_parse_mem(ctx, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("Schema node \"l\" not found (/cont/ll/act/) with context node \"/k:cont/ll/act/l2\".");
    logbuf_clean();

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}
