/*
 * @file test_parser_yang.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from parser_yang.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../../src/tree_schema.c"
#include "../../src/parser_yang.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */

    if (path && path[0]) {
        snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
    } else {
        strncpy(logbuf, msg, BUFSIZE - 1);
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
    (void) state; /* unused */

    const char *str;
    struct ly_ctx *ctx;
    struct lys_module mod = {0};
    struct lysc_feature *f;
    struct lysc_iffeature *iff;

    str = "module test {namespace urn:test; prefix t;"
          "feature f1;feature f2 {if-feature f1;}}";
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    assert_int_equal(LY_EINVAL, lys_compile(NULL, 0));
    logbuf_assert("Invalid argument mod (lys_compile()).");
    assert_int_equal(LY_EINVAL, lys_compile(&mod, 0));
    logbuf_assert("Invalid argument mod->parsed (lys_compile()).");
    assert_int_equal(LY_SUCCESS, yang_parse(ctx, str, &mod.parsed));
    assert_int_equal(LY_SUCCESS, lys_compile(&mod, 0));
    assert_non_null(mod.compiled);
    assert_ptr_equal(mod.parsed->name, mod.compiled->name);
    assert_ptr_equal(mod.parsed->ns, mod.compiled->ns);
    /* features */
    assert_non_null(mod.compiled->features);
    assert_int_equal(2, LY_ARRAY_SIZE(mod.compiled->features));
    f = &mod.compiled->features[1];
    assert_non_null(f->iffeatures);
    assert_int_equal(1, LY_ARRAY_SIZE(f->iffeatures));
    iff = &f->iffeatures[0];
    assert_non_null(iff->expr);
    assert_non_null(iff->features);
    assert_int_equal(1, LY_ARRAY_SIZE(iff->features));
    assert_ptr_equal(&mod.compiled->features[0], iff->features[0]);

    lysc_module_free(mod.compiled, NULL);

    assert_int_equal(LY_SUCCESS, lys_compile(&mod, LYSC_OPT_FREE_SP));
    assert_non_null(mod.compiled);
    assert_string_equal("test", mod.compiled->name);
    assert_string_equal("urn:test", mod.compiled->ns);

    lysc_module_free(mod.compiled, NULL);
    mod.compiled = NULL;

    /* submodules cannot be compiled directly */
    str = "submodule test {belongs-to xxx {prefix x;}}";
    assert_int_equal(LY_SUCCESS, yang_parse(ctx, str, &mod.parsed));
    assert_int_equal(LY_EINVAL, lys_compile(&mod, 0));
    logbuf_assert("Submodules (test) are not supposed to be compiled, compile only the main modules.");
    assert_null(mod.compiled);

    lysp_module_free(mod.parsed);
    ly_ctx_destroy(ctx, NULL);
}

static void
test_feature(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;
    struct lys_module mod = {0}, *modp;
    const char *str;
    struct lysc_feature *f, *f1;

    str = "module a {namespace urn:a;prefix a;yang-version 1.1;\n"
          "feature f1 {description test1;reference test2;status current;} feature f2; feature f3;\n"
          "feature orfeature {if-feature \"f1 or f2\";}\n"
          "feature andfeature {if-feature \"f1 and f2\";}\n"
          "feature f6 {if-feature \"not f1\";}\n"
          "feature f7 {if-feature \"(f2 and f3) or (not f1)\";}\n"
          "feature f8 {if-feature \"f1 or f2 or f3 or orfeature or andfeature\";}\n"
          "feature f9 {if-feature \"not not f1\";}}";

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));
    assert_int_equal(LY_SUCCESS, yang_parse(ctx, str, &mod.parsed));
    assert_int_equal(LY_SUCCESS, lys_compile(&mod, 0));
    assert_non_null(mod.compiled);
    assert_non_null(mod.compiled->features);
    assert_int_equal(9, LY_ARRAY_SIZE(mod.compiled->features));
    /* all features are disabled by default */
    LY_ARRAY_FOR(mod.compiled->features, struct lysc_feature, f) {
        assert_int_equal(0, lysc_feature_value(f));
    }
    /* enable f1 */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(&mod, "f1"));
    f1 = &mod.compiled->features[0];
    assert_int_equal(1, lysc_feature_value(f1));

    /* enable orfeature */
    f = &mod.compiled->features[3];
    assert_int_equal(0, lysc_feature_value(f));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(&mod, "orfeature"));
    assert_int_equal(1, lysc_feature_value(f));

    /* enable andfeature - no possible since f2 is disabled */
    f = &mod.compiled->features[4];
    assert_int_equal(0, lysc_feature_value(f));
    assert_int_equal(LY_EDENIED, lys_feature_enable(&mod, "andfeature"));
    logbuf_assert("Feature \"andfeature\" cannot be enabled since it is disabled by its if-feature condition(s).");
    assert_int_equal(0, lysc_feature_value(f));

    /* first enable f2, so f5 can be enabled then */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(&mod, "f2"));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(&mod, "andfeature"));
    assert_int_equal(1, lysc_feature_value(f));

    /* f1 is enabled, so f6 cannot be enabled */
    f = &mod.compiled->features[5];
    assert_int_equal(0, lysc_feature_value(f));
    assert_int_equal(LY_EDENIED, lys_feature_enable(&mod, "f6"));
    logbuf_assert("Feature \"f6\" cannot be enabled since it is disabled by its if-feature condition(s).");
    assert_int_equal(0, lysc_feature_value(f));

    /* so disable f1 - andfeature will became also disabled */
    assert_int_equal(1, lysc_feature_value(f1));
    assert_int_equal(LY_SUCCESS, lys_feature_disable(&mod, "f1"));
    assert_int_equal(0, lysc_feature_value(f1));
    assert_int_equal(0, lysc_feature_value(&mod.compiled->features[4]));
    /* while orfeature is stille enabled */
    assert_int_equal(1, lysc_feature_value(&mod.compiled->features[3]));
    /* and finally f6 can be enabled */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(&mod, "f6"));
    assert_int_equal(1, lysc_feature_value(&mod.compiled->features[5]));

    /* complex evaluation of f7: f1 and f3 are disabled, while f2 is enabled */
    assert_int_equal(1, lysc_iffeature_value(&mod.compiled->features[6].iffeatures[0]));
    /* long evaluation of f8 to need to reallocate internal stack for operators */
    assert_int_equal(1, lysc_iffeature_value(&mod.compiled->features[7].iffeatures[0]));

    /* double negation of disabled f1 -> disabled */
    assert_int_equal(0, lysc_iffeature_value(&mod.compiled->features[8].iffeatures[0]));

    /* disable all features */
    assert_int_equal(LY_SUCCESS, lys_feature_disable(&mod, "*"));
    LY_ARRAY_FOR(mod.compiled->features, struct lysc_feature, f) {
        assert_int_equal(0, lys_feature_value(&mod, f->name));
    }
    /* re-setting already set feature */
    assert_int_equal(LY_SUCCESS, lys_feature_disable(&mod, "f1"));
    assert_int_equal(0, lys_feature_value(&mod, "f1"));

    /* enabling feature that cannot be enabled due to its if-features */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(&mod, "f1"));
    assert_int_equal(LY_EDENIED, lys_feature_enable(&mod, "andfeature"));
    logbuf_assert("Feature \"andfeature\" cannot be enabled since it is disabled by its if-feature condition(s).");
    assert_int_equal(LY_EDENIED, lys_feature_enable(&mod, "*"));
    logbuf_assert("Feature \"f6\" cannot be enabled since it is disabled by its if-feature condition(s).");
    /* test if not changed */
    assert_int_equal(1, lys_feature_value(&mod, "f1"));
    assert_int_equal(0, lys_feature_value(&mod, "f2"));

    /* invalid reference */
    assert_int_equal(LY_EINVAL, lys_feature_enable(&mod, "xxx"));
    logbuf_assert("Feature \"xxx\" not found in module \"a\".");

    lysc_module_free(mod.compiled, NULL);
    lysp_module_free(mod.parsed);

    /* some invalid expressions */
    assert_int_equal(LY_SUCCESS, yang_parse(ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f{if-feature f1;}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f1\" of if-feature - unable to find feature \"f1\".");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature 'f and';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f and\" of if-feature - unexpected end of expression.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f{if-feature 'or';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"or\" of if-feature - unexpected end of expression.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature '(f1';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"(f1\" of if-feature - non-matching opening and closing parentheses.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature 'f1)';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f1)\" of if-feature - non-matching opening and closing parentheses.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature ---;}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"---\" of if-feature - unable to find feature \"---\".");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(ctx, "module b{namespace urn:b; prefix b; feature f1; feature f2{if-feature 'not f1';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"not f1\" of if-feature - YANG 1.1 expression in YANG 1.0 module.");
    lysp_module_free(mod.parsed);

    /* import reference */
    assert_non_null(modp = lys_parse_mem(ctx, str, LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(modp, 0));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(modp, "f1"));
    assert_non_null(modp = lys_parse_mem(ctx, "module b{namespace urn:b; prefix b; import a {prefix a;} feature f1; feature f2{if-feature 'a:f1';}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(modp, 0));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(modp, "f2"));
    assert_int_equal(0, lys_feature_value(modp, "f1"));
    assert_int_equal(1, lys_feature_value(modp, "f2"));

    ly_ctx_destroy(ctx, NULL);
}

static void
test_identity(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;
    struct lys_module *mod1, *mod2;
    const char *mod1_str = "module a {namespace urn:a;prefix a; identity a1;}";
    const char *mod2_str = "module b {namespace urn:b;prefix b; import a {prefix a;}identity b1; identity b2; identity b3 {base b1; base b:b2; base a:a1;} identity b4 {base b:b1; base b3;}}";

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));
    assert_non_null(mod1 = lys_parse_mem(ctx, mod1_str, LYS_IN_YANG));
    assert_non_null(mod2 = lys_parse_mem(ctx, mod2_str, LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod2, 0));

    assert_non_null(mod1->compiled);
    assert_non_null(mod1->compiled->identities);
    assert_non_null(mod2->compiled);
    assert_non_null(mod2->compiled->identities);

    assert_non_null(mod1->compiled->identities[0].derived);
    assert_int_equal(1, LY_ARRAY_SIZE(mod1->compiled->identities[0].derived));
    assert_ptr_equal(mod1->compiled->identities[0].derived[0], &mod2->compiled->identities[2]);
    assert_non_null(mod2->compiled->identities[0].derived);
    assert_int_equal(2, LY_ARRAY_SIZE(mod2->compiled->identities[0].derived));
    assert_ptr_equal(mod2->compiled->identities[0].derived[0], &mod2->compiled->identities[2]);
    assert_ptr_equal(mod2->compiled->identities[0].derived[1], &mod2->compiled->identities[3]);
    assert_non_null(mod2->compiled->identities[1].derived);
    assert_int_equal(1, LY_ARRAY_SIZE(mod2->compiled->identities[1].derived));
    assert_ptr_equal(mod2->compiled->identities[1].derived[0], &mod2->compiled->identities[2]);
    assert_non_null(mod2->compiled->identities[2].derived);
    assert_int_equal(1, LY_ARRAY_SIZE(mod2->compiled->identities[2].derived));
    assert_ptr_equal(mod2->compiled->identities[2].derived[0], &mod2->compiled->identities[3]);

    ly_ctx_destroy(ctx, NULL);
}

static void
test_node_container(void **state)
{
    (void) state; /* unused */

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_node_container *cont;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));
    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;container c;}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_non_null(mod->compiled);
    assert_non_null((cont = (struct lysc_node_container*)mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, cont->nodetype);
    assert_string_equal("c", cont->name);
    assert_true(cont->flags & LYS_CONFIG_W);
    assert_true(cont->flags & LYS_STATUS_CURR);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;container c {config false; status deprecated; container child;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    logbuf_assert("Missing explicit \"deprecated\" status that was already specified in parent, inheriting.");
    assert_non_null(mod->compiled);
    assert_non_null((cont = (struct lysc_node_container*)mod->compiled->data));
    assert_true(cont->flags & LYS_CONFIG_R);
    assert_true(cont->flags & LYS_STATUS_DEPRC);
    assert_non_null((cont = (struct lysc_node_container*)cont->child));
    assert_int_equal(LYS_CONTAINER, cont->nodetype);
    assert_true(cont->flags & LYS_CONFIG_R);
    assert_true(cont->flags & LYS_STATUS_DEPRC);
    assert_string_equal("child", cont->name);

    ly_ctx_destroy(ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_module, logger_setup),
        cmocka_unit_test_setup(test_feature, logger_setup),
        cmocka_unit_test_setup(test_identity, logger_setup),
        cmocka_unit_test_setup(test_node_container, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
