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
#define _UTEST_MAIN_
#include "utests.h"

#include "common.h"
#include "context.h"
#include "in.h"
#include "schema_compile.h"
#include "tests_config.h"
#include "tree_schema_internal.h"
#include "utests.h"

static void
test_searchdirs(void **state)
{
    const char * const *list;

    /* invalid arguments */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_set_searchdir()).", NULL);
    assert_null(ly_ctx_get_searchdirs(NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_searchdirs()).", NULL);
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdir(NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_unset_searchdir()).", NULL);

    /* readable and executable, but not a directory */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/utest_context"));
    CHECK_LOG_CTX("Given search directory \""TESTS_BIN "/utest_context\" is not a directory.", NULL);
    /* not executable */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(UTEST_LYCTX, __FILE__));
    CHECK_LOG_CTX("Unable to fully access search directory \""__FILE__ "\" (Permission denied).", NULL);
    /* not existing */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(UTEST_LYCTX, "/nonexistingfile"));
    CHECK_LOG_CTX("Unable to use search directory \"/nonexistingfile\" (No such file or directory).", NULL);

    /* ly_set_add() fails */
    /* no change */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, NULL));

    /* correct path */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/utests"));
    assert_int_equal(1, UTEST_LYCTX->search_paths.count);
    assert_string_equal(TESTS_BIN "/utests", UTEST_LYCTX->search_paths.objs[0]);

    /* duplicated paths */
    assert_int_equal(LY_EEXIST, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/utests"));
    assert_int_equal(1, UTEST_LYCTX->search_paths.count);
    assert_string_equal(TESTS_BIN "/utests", UTEST_LYCTX->search_paths.objs[0]);

    /* another paths - add 8 to fill the initial buffer of the searchpaths list */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/CMakeFiles"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC "/../src"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC "/../CMakeModules"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC "/../doc"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN));
    assert_int_equal(7, UTEST_LYCTX->search_paths.count);

    /* get searchpaths */
    list = ly_ctx_get_searchdirs(UTEST_LYCTX);
    assert_non_null(list);
    assert_string_equal(TESTS_BIN "/utests", list[0]);
    assert_string_equal(TESTS_BIN "/CMakeFiles", list[1]);
    assert_string_equal(TESTS_SRC, list[5]);
    assert_string_equal(TESTS_BIN, list[6]);
    assert_null(list[7]);

    /* removing searchpaths */
    /* nonexisting */
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdir(UTEST_LYCTX, "/nonexistingfile"));
    CHECK_LOG_CTX("Invalid argument value (ly_ctx_unset_searchdir()).", NULL);
    /* first */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_BIN "/utests"));
    assert_string_not_equal(TESTS_BIN "/utests", list[0]);
    assert_int_equal(6, UTEST_LYCTX->search_paths.count);
    /* middle */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_SRC));
    assert_int_equal(5, UTEST_LYCTX->search_paths.count);
    /* last */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_BIN));
    assert_int_equal(4, UTEST_LYCTX->search_paths.count);
    /* all */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, NULL));
    assert_int_equal(0, UTEST_LYCTX->search_paths.count);

    /* again - no change */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, NULL));

    /* cleanup */
    ly_ctx_destroy(UTEST_LYCTX, NULL);

    /* test searchdir list in ly_ctx_new() */
    assert_int_equal(LY_EINVAL, ly_ctx_new("/nonexistingfile", 0, &UTEST_LYCTX));
    CHECK_LOG("Unable to use search directory \"/nonexistingfile\" (No such file or directory).", NULL);
    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_SRC ":"TESTS_BIN ":"TESTS_BIN ":"TESTS_SRC, LY_CTX_DISABLE_SEARCHDIRS, &UTEST_LYCTX));
    assert_int_equal(2, UTEST_LYCTX->search_paths.count);
    assert_string_equal(TESTS_SRC, UTEST_LYCTX->search_paths.objs[0]);
    assert_string_equal(TESTS_BIN, UTEST_LYCTX->search_paths.objs[1]);
}

static void
test_options(void **state)
{
    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX, NULL);

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0xffff, &UTEST_LYCTX));

    /* invalid arguments */
    assert_int_equal(0, ly_ctx_get_options(NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_options()).", NULL);

    assert_int_equal(LY_EINVAL, ly_ctx_set_options(NULL, 0));
    CHECK_LOG("Invalid argument ctx (ly_ctx_set_options()).", NULL);
    assert_int_equal(LY_EINVAL, ly_ctx_unset_options(NULL, 0));
    CHECK_LOG("Invalid argument ctx (ly_ctx_unset_options()).", NULL);

    /* option not allowed to be changed */
    assert_int_equal(LY_EINVAL, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_NO_YANGLIBRARY));
    CHECK_LOG_CTX("Invalid argument option (ly_ctx_set_options()).", NULL);
    assert_int_equal(LY_EINVAL, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_NO_YANGLIBRARY));
    CHECK_LOG_CTX("Invalid argument option (ly_ctx_set_options()).", NULL);

    /* unset */
    /* LY_CTX_ALL_IMPLEMENTED */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_ALL_IMPLEMENTED);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_ALL_IMPLEMENTED));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_ALL_IMPLEMENTED);

    /* LY_CTX_REF_IMPLEMENTED */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_REF_IMPLEMENTED);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_REF_IMPLEMENTED));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_REF_IMPLEMENTED);

    /* LY_CTX_DISABLE_SEARCHDIRS */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIRS);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIRS));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIRS);

    /* LY_CTX_DISABLE_SEARCHDIR_CWD */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIR_CWD));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);

    /* LY_CTX_PREFER_SEARCHDIRS */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_PREFER_SEARCHDIRS);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_PREFER_SEARCHDIRS));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_PREFER_SEARCHDIRS);

    assert_int_equal(UTEST_LYCTX->flags, ly_ctx_get_options(UTEST_LYCTX));

    /* set back */
    /* LY_CTX_ALL_IMPLEMENTED */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_ALL_IMPLEMENTED));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_ALL_IMPLEMENTED);

    /* LY_CTX_REF_IMPLEMENTED */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_REF_IMPLEMENTED));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_REF_IMPLEMENTED);

    /* LY_CTX_DISABLE_SEARCHDIRS */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIRS));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIRS);

    /* LY_CTX_DISABLE_SEARCHDIR_CWD */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIR_CWD));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);

    /* LY_CTX_PREFER_SEARCHDIRS */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_PREFER_SEARCHDIRS));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_PREFER_SEARCHDIRS);

    assert_int_equal(UTEST_LYCTX->flags, ly_ctx_get_options(UTEST_LYCTX));
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
test_models(void **state)
{
    struct ly_in *in;
    const char *str;
    struct lys_module *mod1, *mod2;
    struct lys_glob_unres unres = {0};

    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX, NULL);

    /* invalid arguments */
    assert_int_equal(0, ly_ctx_get_change_count(NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_change_count()).", NULL);

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &UTEST_LYCTX));
    assert_int_equal(UTEST_LYCTX->change_count, ly_ctx_get_change_count(UTEST_LYCTX));

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module x {namespace urn:x;prefix x;}", &in));
    assert_int_equal(LY_EINVAL, lys_create_module(UTEST_LYCTX, in, 4, 1, NULL, NULL, NULL, &unres, &mod1));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Invalid schema input format.", NULL);

    /* import callback */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)(str = "test"));
    assert_ptr_equal(test_imp_clb, UTEST_LYCTX->imp_clb);
    assert_ptr_equal(str, UTEST_LYCTX->imp_clb_data);
    assert_ptr_equal(test_imp_clb, ly_ctx_get_module_imp_clb(UTEST_LYCTX, (void **)&str));
    assert_string_equal("test", str);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, NULL, NULL);
    assert_null(UTEST_LYCTX->imp_clb);
    assert_null(UTEST_LYCTX->imp_clb_data);

    /* name collision of module and submodule */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule y {belongs-to a {prefix a;} revision 2018-10-30;}");
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module y {namespace urn:y;prefix y;include y;}", &in));
    assert_int_equal(LY_EVALID, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod1));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Name collision between module and submodule of name \"y\".", "Line number 1.");

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module a {namespace urn:a;prefix a;include y;revision 2018-10-30; }", &in));
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod1));
    assert_int_equal(LY_SUCCESS, lys_compile_unres_glob(UTEST_LYCTX, &unres));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module y {namespace urn:y;prefix y;}", &in));
    assert_int_equal(LY_EVALID, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod1));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Name collision between module and submodule of name \"y\".", "Line number 1.");

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule y {belongs-to b {prefix b;}}");
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module b {namespace urn:b;prefix b;include y;}", &in));
    assert_int_equal(LY_EVALID, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod1));
    lys_compile_unres_glob_revert(UTEST_LYCTX, &unres);
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Including \"y\" submodule into \"b\" failed.", NULL,
            "Name collision between submodules of name \"y\".", "Line number 1.");

    /* selecting correct revision of the submodules */
    ly_ctx_reset_latests(UTEST_LYCTX);
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule y {belongs-to a {prefix a;} revision 2018-10-31;}");
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module a {namespace urn:a;prefix a;include y; revision 2018-10-31;}", &in));
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 0, NULL, NULL, NULL, &unres, &mod2));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    assert_string_equal("2018-10-31", mod2->parsed->includes[0].submodule->revs[0].date);

    /* reloading module in case only the compiled module resists in the context */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module w {namespace urn:w;prefix w;revision 2018-10-24;}", &in));
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 0, NULL, NULL, NULL, &unres, &mod1));
    ly_in_free(in, 0);
    mod1->implemented = 1;
    assert_int_equal(LY_SUCCESS, lys_compile(mod1, 0, &unres));
    assert_int_equal(LY_SUCCESS, lys_compile_unres_glob(UTEST_LYCTX, &unres));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    assert_non_null(mod1->compiled);
    assert_non_null(mod1->parsed);

#if 0
    /* TODO in case we are able to remove the parsed schema, here we will test how it will handle missing import parsed schema */

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module z {namespace urn:z;prefix z;import w {prefix w;revision-date 2018-10-24;}}", &in));
    /* mod1->parsed is necessary to compile mod2 because of possible groupings, typedefs, ... */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, NULL, NULL);
    assert_int_equal(LY_ENOTFOUND, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, &mod2));
    /*logbuf_assert("Unable to reload \"w\" module to import it into \"z\", source data not found.");*/
    CHECK_LOG_CTX("Recompilation of module \"w\" failed.", NULL);
    assert_null(mod2);
    ly_in_free(in, 0);
#endif

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module z {namespace urn:z;prefix z;import w {prefix w;revision-date 2018-10-24;}}", &in));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module w {namespace urn:w;prefix w;revision 2018-10-24;}");
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod2));
    assert_int_equal(LY_SUCCESS, lys_compile_unres_glob(UTEST_LYCTX, &unres));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    assert_non_null(mod2);
    assert_non_null(mod1->parsed);
    assert_string_equal("w", mod1->name);
}

static void
test_imports(void **state)
{
    const struct lys_module *mod1, *mod2, *import;

    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX, NULL);

    /* import callback provides newer revision of module 'a' than present in context, so when importing 'a', the newer revision
     * from the callback should be loaded into the context and used as an import */
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &UTEST_LYCTX));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module a {namespace urn:a; prefix a; revision 2019-09-17;}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;revision 2019-09-16;}",
            LYS_IN_YANG, &mod1));
    assert_int_equal(1, mod1->latest_revision);
    assert_int_equal(1, mod1->implemented);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;import a {prefix a;}}",
            LYS_IN_YANG, &mod2));
    import = mod2->parsed->imports[0].module;
    assert_int_equal(2, import->latest_revision);
    assert_int_equal(0, mod1->latest_revision);
    assert_ptr_not_equal(mod1, import);
    assert_string_equal("2019-09-17", import->revision);
    assert_int_equal(0, import->implemented);
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "a", "2019-09-16"));
    ly_ctx_destroy(UTEST_LYCTX, NULL);

    /* import callback provides older revision of module 'a' than present in context, so when importing a, the newer revision
     * already present in the context should be selected and the callback's revision should not be loaded into the context */
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &UTEST_LYCTX));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module a {namespace urn:a; prefix a; revision 2019-09-17;}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;revision 2019-09-18;}",
            LYS_IN_YANG, &mod1));
    assert_int_equal(1, mod1->latest_revision);
    assert_int_equal(1, mod1->implemented);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;import a {prefix a;}}",
            LYS_IN_YANG, &mod2));
    import = mod2->parsed->imports[0].module;
    assert_ptr_equal(mod1, import);
    assert_int_equal(2, import->latest_revision);
    assert_int_equal(1, import->implemented);
    assert_string_equal("2019-09-18", import->revision);
    assert_null(ly_ctx_get_module(UTEST_LYCTX, "a", "2019-09-17"));
}

static void
test_get_models(void **state)
{
    struct lys_module *mod, *mod2;
    const char *str0 = "module a {namespace urn:a;prefix a;}";
    const char *str1 = "module a {namespace urn:a;prefix a;revision 2018-10-23;}";
    const char *str2 = "module a {namespace urn:a;prefix a;revision 2018-10-23;revision 2018-10-24;}";
    struct ly_in *in0, *in1, *in2;
    struct lys_glob_unres unres = {0};

    unsigned int index = 0;
    const char *names[] = {"ietf-yang-metadata", "yang", "ietf-inet-types", "ietf-yang-types", "ietf-datastores", "ietf-yang-library", "a", "a", "a"};

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str0, &in0));
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str1, &in1));
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str2, &in2));

    /* invalid arguments */
    assert_ptr_equal(NULL, ly_ctx_get_module(NULL, NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_module()).", NULL);
    assert_ptr_equal(NULL, ly_ctx_get_module(UTEST_LYCTX, NULL, NULL));
    CHECK_LOG_CTX("Invalid argument name (ly_ctx_get_module()).", NULL);
    assert_ptr_equal(NULL, ly_ctx_get_module_ns(NULL, NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_module_ns()).", NULL);
    assert_ptr_equal(NULL, ly_ctx_get_module_ns(UTEST_LYCTX, NULL, NULL));
    CHECK_LOG_CTX("Invalid argument ns (ly_ctx_get_module_ns()).", NULL);
    assert_null(ly_ctx_get_module(UTEST_LYCTX, "nonsence", NULL));

    /* internal modules */
    assert_null(ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-yang-types"));
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "yang");
    assert_non_null(mod);
    assert_non_null(mod->parsed);
    assert_string_equal("yang", mod->name);
    mod2 = ly_ctx_get_module_implemented_ns(UTEST_LYCTX, mod->ns);
    assert_ptr_equal(mod, mod2);
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "ietf-yang-metadata", "2016-08-05"));
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "ietf-yang-types", "2013-07-15"));
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "ietf-inet-types", "2013-07-15"));
    assert_non_null(ly_ctx_get_module_ns(UTEST_LYCTX, "urn:ietf:params:xml:ns:yang:ietf-datastores", "2018-02-14"));

    /* select module by revision */
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in1, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod));
    assert_int_equal(LY_SUCCESS, lys_compile_unres_glob(UTEST_LYCTX, &unres));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    /* invalid attempts - implementing module of the same name and inserting the same module */
    assert_int_equal(LY_EDENIED, lys_create_module(UTEST_LYCTX, in2, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, NULL));
    CHECK_LOG_CTX("Module \"a@2018-10-23\" is already implemented in the context.", NULL);
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_reset(in1);
    /* it is already there, fine */
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in1, LYS_IN_YANG, 0, NULL, NULL, NULL, &unres, NULL));
    /* insert the second module only as imported, not implemented */
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_reset(in2);
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in2, LYS_IN_YANG, 0, NULL, NULL, NULL, &unres, &mod2));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    assert_non_null(mod2);
    assert_ptr_not_equal(mod, mod2);
    mod = ly_ctx_get_module_latest(UTEST_LYCTX, "a");
    assert_ptr_equal(mod, mod2);
    mod2 = ly_ctx_get_module_latest_ns(UTEST_LYCTX, mod->ns);
    assert_ptr_equal(mod, mod2);
    /* work with module with no revision */
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in0, LYS_IN_YANG, 0, NULL, NULL, NULL, &unres, &mod));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    assert_ptr_equal(mod, ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_ptr_not_equal(mod, ly_ctx_get_module_latest(UTEST_LYCTX, "a"));

    str1 = "submodule b {belongs-to a {prefix a;}}";
    ly_in_free(in1, 0);
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str1, &in1));
    assert_int_equal(LY_EINVAL, lys_create_module(UTEST_LYCTX, in1, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod));
    CHECK_LOG_CTX("Input data contains submodule which cannot be parsed directly without its main module.", NULL);
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);

    while ((mod = (struct lys_module *)ly_ctx_get_module_iter(UTEST_LYCTX, &index))) {
        assert_string_equal(names[index - 1], mod->name);
    }
    assert_int_equal(9, index);

    /* cleanup */
    ly_in_free(in0, 0);
    ly_in_free(in1, 0);
    ly_in_free(in2, 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_searchdirs),
        UTEST(test_options),
        UTEST(test_models),
        UTEST(test_imports),
        UTEST(test_get_models),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
