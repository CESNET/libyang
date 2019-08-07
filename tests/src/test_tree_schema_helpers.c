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

#include "../../src/common.h"
#include "../../src/context.h"
#include "../../src/tree_schema_internal.h"

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

    ly_set_log_clb(logger, 0);

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

static void
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

    assert_int_equal(2, LY_ARRAY_SIZE(revs));
    assert_string_equal("2018-01-01", &revs[0]);
    assert_string_equal("2018-12-31", &revs[1]);
    /* the order should be fixed, so the newest revision will be the first in the array */
    lysp_sort_revisions(revs);
    assert_string_equal("2018-12-31", &revs[0]);
    assert_string_equal("2018-01-01", &revs[1]);

    LY_ARRAY_FREE(revs);
}

static LY_ERR test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
                           const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
                           const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YANG;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
test_typedef(void **state)
{
    *state = test_typedef;

    struct ly_ctx *ctx = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    str = "module a {namespace urn:a; prefix a; typedef binary {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"binary\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef bits {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"bits\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef boolean {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"boolean\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef decimal64 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"decimal64\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef empty {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"empty\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef enumeration {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"enumeration\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef int8 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"int8\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef int16 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"int16\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef int32 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"int32\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef int64 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"int64\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef instance-identifier {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"instance-identifier\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef identityref {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"identityref\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef leafref {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"leafref\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef string {type int8;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"string\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef union {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"union\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef uint8 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"uint8\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef uint16 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"uint16\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef uint32 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"uint32\" of typedef - name collision with a built-in type.");
    str = "module a {namespace urn:a; prefix a; typedef uint64 {type string;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"uint64\" of typedef - name collision with a built-in type.");

    str = "module mytypes {namespace urn:types; prefix t; typedef binary_ {type string;} typedef bits_ {type string;} typedef boolean_ {type string;} "
          "typedef decimal64_ {type string;} typedef empty_ {type string;} typedef enumeration_ {type string;} typedef int8_ {type string;} typedef int16_ {type string;}"
          "typedef int32_ {type string;} typedef int64_ {type string;} typedef instance-identifier_ {type string;} typedef identityref_ {type string;}"
          "typedef leafref_ {type string;} typedef string_ {type int8;} typedef union_ {type string;} typedef uint8_ {type string;} typedef uint16_ {type string;}"
          "typedef uint32_ {type string;} typedef uint64_ {type string;}}";
    assert_non_null(lys_parse_mem(ctx, str, LYS_IN_YANG));

    str = "module a {namespace urn:a; prefix a; typedef test {type string;} typedef test {type int8;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"test\" of typedef - name collision with another top-level type.");

    str = "module a {namespace urn:a; prefix a; typedef x {type string;} container c {typedef x {type int8;}}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"x\" of typedef - scoped type collide with a top-level type.");

    str = "module a {namespace urn:a; prefix a; container c {container d {typedef y {type int8;}} typedef y {type string;}}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"y\" of typedef - name collision with another scoped type.");

    str = "module a {namespace urn:a; prefix a; container c {typedef y {type int8;} typedef y {type string;}}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"y\" of typedef - name collision with sibling type.");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type string;}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"x\" of typedef - name collision with another top-level type.");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule b {belongs-to a {prefix a;} container c {typedef x {type string;}}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"x\" of typedef - scoped type collide with a top-level type.");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type int8;}}");
    str = "module a {namespace urn:a; prefix a; include b; container c {typedef x {type string;}}}";
    assert_null(lys_parse_mem(ctx, str, LYS_IN_YANG));
    logbuf_assert("Invalid name \"x\" of typedef - scoped type collide with a top-level type.");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_date, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_revisions, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_typedef, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
