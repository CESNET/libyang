/*
 * @file test_printer_yang.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from printer_yang.c
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"
#include "context.h"
#include "out.h"
#include "printer_schema.h"
#include "tree_schema.h"
#include "utests.h"

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
    const char *orig = "module a {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:test:a\";\n"
            "  prefix a;\n\n"
            "  import ietf-yang-types {\n"
            "    prefix yt;\n"
            "    revision-date 2013-07-15;\n"
            "    description\n"
            "      \"YANG types\";\n"
            "    reference\n"
            "      \"RFC reference\";\n"
            "  }\n\n"
            "  organization\n"
            "    \"ORG\";\n"
            "  contact\n"
            "    \"Radek Krejci.\";\n"
            "  description\n"
            "    \"Long multiline\n"
            "      description.\";\n"
            "  reference\n"
            "    \"some reference\";\n"
            "}\n";
    char *compiled = "module a {\n"
            "  namespace \"urn:test:a\";\n"
            "  prefix a;\n\n"
            "  organization\n"
            "    \"ORG\";\n"
            "  contact\n"
            "    \"Radek Krejci.\";\n"
            "  description\n"
            "    \"Long multiline\n"
            "      description.\";\n"
            "  reference\n"
            "    \"some reference\";\n"
            "}\n";
    char *printed;
    struct ly_out *out;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, orig, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YANG, 0, 0));
    assert_int_equal(strlen(orig), ly_out_printed(out));
    assert_string_equal(printed, orig);
    ly_out_reset(out);
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YANG_COMPILED, 0, 0));
    assert_int_equal(strlen(compiled), ly_out_printed(out));
    assert_string_equal(printed, compiled);
    ly_out_reset(out);

    orig = "module b {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:test:b\";\n"
            "  prefix b;\n\n"
            "  revision 2019-04-16 {\n"
            "    description\n"
            "      \"text\";\n"
            "    reference\n"
            "      \"text\";\n"
            "  }\n"
            "  revision 2019-04-15 {\n"
            "    description\n"
            "      \"initial revision\";\n"
            "  }\n\n"
            "  feature f1 {\n"
            "    status current;\n"
            "    description\n"
            "      \"text\";\n"
            "    reference\n"
            "      \"text\";\n"
            "  }\n\n"
            "  feature f2 {\n"
            "    if-feature \"not f1\";\n"
            "  }\n"
            "}\n";
    compiled = "module b {\n"
            "  namespace \"urn:test:b\";\n"
            "  prefix b;\n\n"
            "  revision 2019-04-16;\n"
            "}\n";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, orig, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YANG, 0, 0));
    assert_int_equal(strlen(orig), ly_out_printed(out));
    assert_string_equal(printed, orig);
    ly_out_reset(out);
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YANG_COMPILED, 0, 0));
    assert_int_equal(strlen(compiled), ly_out_printed(out));
    assert_string_equal(printed, compiled);
    ly_out_reset(out);

    orig = "module c {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:test:c\";\n"
            "  prefix c;\n\n"
            "  feature f1;\n\n"
            "  identity i1 {\n"
            "    if-feature \"f1\";\n"
            "    description\n"
            "      \"text\";\n"
            "    reference\n"
            "      \"text32\";\n"
            "  }\n\n"
            "  identity i2 {\n"
            "    base i1;\n"
            "    status obsolete;\n"
            "  }\n"
            "}\n";
    compiled = "module c {\n"
            "  namespace \"urn:test:c\";\n"
            "  prefix c;\n"
            "}\n";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, orig, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YANG, 0, 0));
    assert_int_equal(strlen(orig), ly_out_printed(out));
    assert_string_equal(printed, orig);
    ly_out_reset(out);
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YANG_COMPILED, 0, 0));
    assert_int_equal(strlen(compiled), ly_out_printed(out));
    assert_string_equal(printed, compiled);

    *state = NULL;
    ly_out_free(out, NULL, 1);
    ly_ctx_destroy(ctx, NULL);
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
test_submodule(void **state)
{
    *state = test_submodule;

    struct ly_ctx *ctx = {0};
    const struct lys_module *mod;
    const char *mod_yang = "module a {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:test:a\";\n"
            "  prefix a;\n\n"
            "  include a-sub;\n"
            "}\n";
    char *submod_yang = "submodule a-sub {\n"
            "  yang-version 1.1;\n"
            "  belongs-to a {\n"
            "    prefix a;\n"
            "  }\n\n"
            "  import ietf-yang-types {\n"
            "    prefix yt;\n"
            "    revision-date 2013-07-15;\n"
            "  }\n\n"
            "  organization\n"
            "    \"ORG\";\n"
            "  contact\n"
            "    \"Radek Krejci.\";\n"
            "  description\n"
            "    \"Long multiline\n"
            "      description.\";\n"
            "  reference\n"
            "    \"some reference\";\n"
            "}\n";
    char *printed;
    struct ly_out *out;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, submod_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(ctx, mod_yang, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_submodule(out, mod, mod->parsed->includes[0].submodule, LYS_OUT_YANG, 0, 0));
    assert_int_equal(strlen(submod_yang), ly_out_printed(out));
    assert_string_equal(printed, submod_yang);

    *state = NULL;
    ly_out_free(out, NULL, 1);
    ly_ctx_destroy(ctx, NULL);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_module, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_submodule, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
