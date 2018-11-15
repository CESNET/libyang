/*
 * @file set.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from context.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "tests/config.h"
#include "../../src/common.c"
#include "../../src/log.c"
#include "../../src/set.c"
#include "../../src/hash_table.c"
#include "../../src/xpath.c"
#include "../../src/parser_yang.c"
#include "../../src/context.c"
#include "../../src/tree_schema_helpers.c"
#include "../../src/tree_schema_free.c"
#include "../../src/tree_schema_compile.c"
#include "../../src/tree_schema.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#include <stdio.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

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

static int
logger_setup(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 1);
#endif
    return 0;
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

static void
test_searchdirs(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;
    const char * const *list;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    /* invalid arguments */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(NULL, NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_set_searchdir()).");
    assert_null(ly_ctx_get_searchdirs(NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_get_searchdirs()).");
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdirs(NULL, NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_unset_searchdirs()).");

    /* readable and executable, but not a directory */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(ctx, TESTS_BIN"/src_context"));
    logbuf_assert("Given search directory \""TESTS_BIN"/src_context\" is not a directory.");
    /* not executable */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(ctx, __FILE__));
    logbuf_assert("Unable to use search directory \""__FILE__"\" (Permission denied)");
    /* not existing */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(ctx, "/nonexistingfile"));
    logbuf_assert("Unable to use search directory \"/nonexistingfile\" (No such file or directory)");

    /* ly_set_add() fails */
    /* no change */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, NULL));

    /* correct path */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_BIN"/src"));
    assert_int_equal(1, ctx->search_paths.count);
    assert_string_equal(TESTS_BIN"/src", ctx->search_paths.objs[0]);

    /* duplicated paths */
    assert_int_equal(LY_EEXIST, ly_ctx_set_searchdir(ctx, TESTS_BIN"/src"));
    assert_int_equal(1, ctx->search_paths.count);
    assert_string_equal(TESTS_BIN"/src", ctx->search_paths.objs[0]);

    /* another paths - add 8 to fill the initial buffer of the searchpaths list */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_BIN"/CMakeFiles"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC"/../src"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC"/../CMakeModules"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC"/../doc"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_SRC));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, TESTS_BIN));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(ctx, "/tmp"));
    assert_int_equal(8, ctx->search_paths.count);

    /* get searchpaths */
    list = ly_ctx_get_searchdirs(ctx);
    assert_non_null(list);
    assert_string_equal(TESTS_BIN"/src", list[0]);
    assert_string_equal(TESTS_BIN"/CMakeFiles", list[1]);
    assert_string_equal(TESTS_SRC, list[5]);
    assert_string_equal(TESTS_BIN, list[6]);
    assert_string_equal("/tmp", list[7]);
    assert_null(list[8]);

    /* removing searchpaths */
    /* nonexisting */
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdirs(ctx, "/nonexistingfile"));
    logbuf_assert("Invalid argument value (ly_ctx_unset_searchdirs()).");
    /* first */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, TESTS_BIN"/src"));
    assert_string_not_equal(TESTS_BIN"/src", list[0]);
    assert_int_equal(7, ctx->search_paths.count);
    /* middle */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, TESTS_SRC));
    assert_int_equal(6, ctx->search_paths.count);
    /* last */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, "/tmp"));
    assert_int_equal(5, ctx->search_paths.count);
    /* all */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, NULL));
    assert_int_equal(0, ctx->search_paths.count);

    /* again - no change */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdirs(ctx, NULL));

    /* cleanup */
    ly_ctx_destroy(ctx, NULL);

    /* test searchdir list in ly_ctx_new() */
    assert_int_equal(LY_EINVAL, ly_ctx_new("/nonexistingfile", 0, &ctx));
    logbuf_assert("Unable to use search directory \"/nonexistingfile\" (No such file or directory)");
    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_SRC":/tmp:/tmp:"TESTS_SRC, 0, &ctx));
    assert_int_equal(2, ctx->search_paths.count);
    assert_string_equal(TESTS_SRC, ctx->search_paths.objs[0]);
    assert_string_equal("/tmp", ctx->search_paths.objs[1]);

    /* cleanup */
    ly_ctx_destroy(ctx, NULL);
}

static void
test_options(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0xffffffff, &ctx));

    /* invalid arguments */
    assert_int_equal(0, ly_ctx_get_options(NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_get_options()).");

    assert_int_equal(LY_EINVAL, ly_ctx_set_option(NULL, 0));
    logbuf_assert("Invalid argument ctx (ly_ctx_set_option()).");
    assert_int_equal(LY_EINVAL, ly_ctx_unset_option(NULL, 0));
    logbuf_assert("Invalid argument ctx (ly_ctx_unset_option()).");

    /* option not allowed to be changed */
    assert_int_equal(LY_EINVAL, ly_ctx_set_option(ctx, LY_CTX_NOYANGLIBRARY));
    logbuf_assert("Invalid argument option (ly_ctx_set_option()).");
    assert_int_equal(LY_EINVAL, ly_ctx_set_option(ctx, LY_CTX_NOYANGLIBRARY));
    logbuf_assert("Invalid argument option (ly_ctx_set_option()).");


    /* unset */
    /* LY_CTX_ALLIMPLEMENTED */
    assert_int_not_equal(0, ctx->flags & LY_CTX_ALLIMPLEMENTED);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_option(ctx, LY_CTX_ALLIMPLEMENTED));
    assert_int_equal(0, ctx->flags & LY_CTX_ALLIMPLEMENTED);

    /* LY_CTX_DISABLE_SEARCHDIRS */
    assert_int_not_equal(0, ctx->flags & LY_CTX_DISABLE_SEARCHDIRS);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_option(ctx, LY_CTX_DISABLE_SEARCHDIRS));
    assert_int_equal(0, ctx->flags & LY_CTX_DISABLE_SEARCHDIRS);

    /* LY_CTX_DISABLE_SEARCHDIR_CWD */
    assert_int_not_equal(0, ctx->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_option(ctx, LY_CTX_DISABLE_SEARCHDIR_CWD));
    assert_int_equal(0, ctx->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);

    /* LY_CTX_PREFER_SEARCHDIRS */
    assert_int_not_equal(0, ctx->flags & LY_CTX_PREFER_SEARCHDIRS);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_option(ctx, LY_CTX_PREFER_SEARCHDIRS));
    assert_int_equal(0, ctx->flags & LY_CTX_PREFER_SEARCHDIRS);

    /* LY_CTX_TRUSTED */
    assert_int_not_equal(0, ctx->flags & LY_CTX_TRUSTED);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_option(ctx, LY_CTX_TRUSTED));
    assert_int_equal(0, ctx->flags & LY_CTX_TRUSTED);

    assert_int_equal(ctx->flags, ly_ctx_get_options(ctx));

    /* set back */
    /* LY_CTX_ALLIMPLEMENTED */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_option(ctx, LY_CTX_ALLIMPLEMENTED));
    assert_int_not_equal(0, ctx->flags & LY_CTX_ALLIMPLEMENTED);

    /* LY_CTX_DISABLE_SEARCHDIRS */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_option(ctx, LY_CTX_DISABLE_SEARCHDIRS));
    assert_int_not_equal(0, ctx->flags & LY_CTX_DISABLE_SEARCHDIRS);

    /* LY_CTX_DISABLE_SEARCHDIR_CWD */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_option(ctx, LY_CTX_DISABLE_SEARCHDIR_CWD));
    assert_int_not_equal(0, ctx->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);

    /* LY_CTX_PREFER_SEARCHDIRS */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_option(ctx, LY_CTX_PREFER_SEARCHDIRS));
    assert_int_not_equal(0, ctx->flags & LY_CTX_PREFER_SEARCHDIRS);

    /* LY_CTX_TRUSTED */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_option(ctx, LY_CTX_TRUSTED));
    assert_int_not_equal(0, ctx->flags & LY_CTX_TRUSTED);

    assert_int_equal(ctx->flags, ly_ctx_get_options(ctx));

    /* cleanup */
    ly_ctx_destroy(ctx, NULL);
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
test_models(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;
    const char *str;
    struct lys_module *mod1, *mod2;

    /* invalid arguments */
    assert_int_equal(0, ly_ctx_get_module_set_id(NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_get_module_set_id()).");

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));
    assert_int_equal(ctx->module_set_id, ly_ctx_get_module_set_id(ctx));

    assert_null(lys_parse_mem(ctx, "module x {namespace urn:x;prefix x;}", 3));
    logbuf_assert("Invalid schema input format.");

    /* import callback */
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, (void*)(str = "test"));
    assert_ptr_equal(test_imp_clb, ctx->imp_clb);
    assert_ptr_equal(str, ctx->imp_clb_data);
    assert_ptr_equal(test_imp_clb, ly_ctx_get_module_imp_clb(ctx, (void**)&str));
    assert_string_equal("test", str);

    ly_ctx_set_module_imp_clb(ctx, NULL, NULL);
    assert_null(ctx->imp_clb);
    assert_null(ctx->imp_clb_data);

    /* name collision of module and submodule */
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule y {belongs-to a {prefix a;} revision 2018-10-30;}");
    assert_null(lys_parse_mem(ctx, "module y {namespace urn:y;prefix y;include y;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, ly_errcode(ctx));
    logbuf_assert("Name collision between module and submodule of name \"y\". Line number 1.");

    assert_non_null(lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;include y;revision 2018-10-30; }", LYS_IN_YANG));
    assert_null(lys_parse_mem(ctx, "module y {namespace urn:y;prefix y;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, ly_errcode(ctx));
    logbuf_assert("Name collision between module and submodule of name \"y\". Line number 1.");

    store = 1;
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule y {belongs-to b {prefix b;}}");
    assert_null(lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;include y;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, ly_errcode(ctx));
    logbuf_assert("Name collision between submodules of name \"y\". Line number 1.");
    store = -1;

    /* selecting correct revision of the submodules */
    ly_ctx_reset_latests(ctx);
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule y {belongs-to a {prefix a;} revision 2018-10-31;}");
    mod2 = lys_parse_mem_(ctx, "module a {namespace urn:a;prefix a;include y; revision 2018-10-31;}", LYS_IN_YANG, 0, NULL, NULL, NULL);
    assert_non_null(mod2);
    assert_string_equal("2018-10-31", mod2->parsed->includes[0].submodule->revs[0].date);

    /* reloading module in case only the compiled module resists in the context */
    mod1 = lys_parse_mem(ctx, "module w {namespace urn:w;prefix w;revision 2018-10-24;}", LYS_IN_YANG);
    assert_non_null(mod1);
    assert_int_equal(LY_SUCCESS, lys_compile(mod1, LYSC_OPT_FREE_SP));
    assert_non_null(mod1->compiled);
    assert_null(mod1->parsed);
    mod2 = lys_parse_mem(ctx, "module z {namespace urn:z;prefix z;import w {prefix w;revision-date 2018-10-24;}}", LYS_IN_YANG);
    assert_non_null(mod2);
    /* mod1->parsed is necessary to compile mod2 because of possible groupings, typedefs, ... */
    ly_ctx_set_module_imp_clb(ctx, NULL, NULL);
    assert_int_equal(LY_ENOTFOUND, lys_compile(mod2, 0));
    logbuf_assert("Unable to reload \"w\" module to import it into \"z\", source data not found.");
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module w {namespace urn:w;prefix w;revision 2018-10-24;}");
    assert_int_equal(LY_SUCCESS, lys_compile(mod2, 0));
    assert_non_null(mod1->parsed);
    assert_string_equal("w", mod1->parsed->name);

    /* cleanup */
    ly_ctx_destroy(ctx, NULL);
}

static void
test_get_models(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;
    const struct lys_module *mod, *mod2;
    const char *str0 = "module a {namespace urn:a;prefix a;}";
    const char *str1 = "module a {namespace urn:a;prefix a;revision 2018-10-23;}";
    const char *str2 = "module a {namespace urn:a;prefix a;revision 2018-10-23;revision 2018-10-24;}";

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    /* invalid arguments */
    assert_ptr_equal(NULL, ly_ctx_get_module(NULL, NULL, NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_get_module()).");
    assert_ptr_equal(NULL, ly_ctx_get_module(ctx, NULL, NULL));
    logbuf_assert("Invalid argument name (ly_ctx_get_module()).");
    assert_ptr_equal(NULL, ly_ctx_get_module_ns(NULL, NULL, NULL));
    logbuf_assert("Invalid argument ctx (ly_ctx_get_module_ns()).");
    assert_ptr_equal(NULL, ly_ctx_get_module_ns(ctx, NULL, NULL));
    logbuf_assert("Invalid argument ns (ly_ctx_get_module_ns()).");
    assert_null(ly_ctx_get_module(ctx, "nonsence", NULL));

    /* internal modules */
    assert_null(ly_ctx_get_module_implemented(ctx, "ietf-yang-types"));
    mod = ly_ctx_get_module_implemented(ctx, "yang");
    assert_non_null(mod);
    assert_non_null(mod->parsed);
    assert_string_equal("yang", mod->parsed->name);
    mod2 = ly_ctx_get_module_implemented_ns(ctx, mod->parsed->ns);
    assert_ptr_equal(mod, mod2);
    assert_non_null(ly_ctx_get_module(ctx, "ietf-yang-metadata", "2016-08-05"));
    assert_non_null(ly_ctx_get_module(ctx, "ietf-yang-types", "2013-07-15"));
    assert_non_null(ly_ctx_get_module(ctx, "ietf-inet-types", "2013-07-15"));
    assert_non_null(ly_ctx_get_module_ns(ctx, "urn:ietf:params:xml:ns:yang:ietf-datastores", "2017-08-17"));

    /* select module by revision */
    mod = lys_parse_mem(ctx, str1, LYS_IN_YANG);
    /* invalid attempts - implementing module of the same name and inserting the same module */
    assert_null(lys_parse_mem(ctx, str2, LYS_IN_YANG));
    logbuf_assert("Module \"a\" is already implemented in the context.");
    assert_null(lys_parse_mem_(ctx, str1, LYS_IN_YANG, 0, NULL, NULL, NULL));
    logbuf_assert("Module \"a\" of revision \"2018-10-23\" is already present in the context.");
    /* insert the second module only as imported, not implemented */
    mod2 = lys_parse_mem_(ctx, str2, LYS_IN_YANG, 0, NULL, NULL, NULL);
    assert_non_null(mod);
    assert_non_null(mod2);
    assert_ptr_not_equal(mod, mod2);
    mod = ly_ctx_get_module_latest(ctx, "a");
    assert_ptr_equal(mod, mod2);
    mod2 = ly_ctx_get_module_latest_ns(ctx, mod->parsed->ns);
    assert_ptr_equal(mod, mod2);
    /* work with module with no revision */
    mod = lys_parse_mem_(ctx, str0, LYS_IN_YANG, 0, NULL, NULL, NULL);
    assert_non_null(mod);
    assert_ptr_equal(mod, ly_ctx_get_module(ctx, "a", NULL));
    assert_ptr_not_equal(mod, ly_ctx_get_module_latest(ctx, "a"));

    str1 = "submodule b {belongs-to a {prefix a;}}";
    assert_null(lys_parse_mem(ctx, str1, LYS_IN_YANG));
    logbuf_assert("Input data contains submodule \"b\" which cannot be parsed directly without its main module.");

    /* cleanup */
    ly_ctx_destroy(ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_searchdirs, logger_setup),
        cmocka_unit_test_setup(test_options, logger_setup),
        cmocka_unit_test_setup(test_models, logger_setup),
        cmocka_unit_test_setup(test_get_models, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
