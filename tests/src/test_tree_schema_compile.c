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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "../../src/common.h"
#include "../../src/tree_schema_internal.h"
#include "../../src/xpath.h"
#include "../../src/plugins_types.h"

void lysc_feature_free(struct ly_ctx *ctx, struct lysc_feature *feat);
void lys_parser_ctx_free(struct lys_parser_ctx *ctx);

LY_ERR lys_path_token(const char **path, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len,
                      int *parent_times, int *has_predicate);

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
#   define logbuf_assert(str) assert_string_equal(logbuf, str);logbuf_clean()
#else
#   define logbuf_assert(str)
#endif

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
reset_mod(struct lys_module *module)
{
    struct ly_ctx *ctx = module->ctx;
    lysc_module_free(module->compiled, NULL);
    lysp_module_free(module->parsed);

    FREE_STRING(ctx, module->name);
    FREE_STRING(ctx, module->ns);
    FREE_STRING(ctx, module->prefix);
    FREE_STRING(ctx, module->filepath);
    FREE_STRING(ctx, module->org);
    FREE_STRING(ctx, module->contact);
    FREE_STRING(ctx, module->dsc);
    FREE_STRING(ctx, module->ref);
    FREE_ARRAY(ctx, module->off_features, lysc_feature_free);

    memset(module, 0, sizeof *module);
    module->ctx = ctx;
    module->implemented = 1;
}

static void
test_module(void **state)
{
    *state = test_module;

    const char *str;
    struct lys_parser_ctx *ctx = NULL;
    struct lys_module mod = {0};
    struct lysc_feature *f;
    struct lysc_iffeature *iff;

    str = "module test {namespace urn:test; prefix t;"
          "feature f1;feature f2 {if-feature f1;}}";
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &mod.ctx));
    reset_mod(&mod);

    assert_int_equal(LY_EINVAL, lys_compile(NULL, 0));
    logbuf_assert("Invalid argument mod (lys_compile()).");
    assert_int_equal(LY_EINVAL, lys_compile(&mod, 0));
    logbuf_assert("Invalid argument mod->parsed (lys_compile()).");
    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, str, &mod));
    lys_parser_ctx_free(ctx);
    mod.implemented = 0;
    assert_int_equal(LY_SUCCESS, lys_compile(&mod, 0));
    assert_null(mod.compiled);
    mod.implemented = 1;
    assert_int_equal(LY_SUCCESS, lys_compile(&mod, 0));
    assert_non_null(mod.compiled);
    assert_string_equal("test", mod.name);
    assert_string_equal("urn:test", mod.ns);
    assert_string_equal("t", mod.prefix);
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

    lysc_module_free(mod.compiled, NULL);
    mod.compiled = NULL;

    /* submodules cannot be compiled directly */
    str = "submodule test {belongs-to xxx {prefix x;}}";
    assert_int_equal(LY_EINVAL, yang_parse_module(&ctx, str, &mod));
    lys_parser_ctx_free(ctx);
    logbuf_assert("Input data contains submodule which cannot be parsed directly without its main module.");
    assert_null(mod.parsed);
    reset_mod(&mod);

    /* data definition name collision in top level */
    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module aa {namespace urn:aa;prefix aa;"
                                                  "leaf a {type string;} container a{presence x;}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Duplicate identifier \"a\" of data definition/RPC/action/Notification statement. /aa:a");
    assert_null(mod.compiled);
    reset_mod(&mod);

    *state = NULL;
    ly_ctx_destroy(mod.ctx, NULL);
}

static void
test_feature(void **state)
{
    *state = test_feature;

    struct lys_parser_ctx *ctx = NULL;
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

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &mod.ctx));
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, str, &mod));
    lys_parser_ctx_free(ctx);
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

    assert_non_null(modp = lys_parse_mem(mod.ctx, "module b {namespace urn:b;prefix b;"
                                         "feature f1 {if-feature f2;}feature f2;}", LYS_IN_YANG));
    assert_non_null(modp->compiled);
    assert_non_null(modp->compiled->features);
    assert_int_equal(2, LY_ARRAY_SIZE(modp->compiled->features));
    assert_non_null(modp->compiled->features[0].iffeatures);
    assert_int_equal(1, LY_ARRAY_SIZE(modp->compiled->features[0].iffeatures));
    assert_non_null(modp->compiled->features[0].iffeatures[0].features);
    assert_int_equal(1, LY_ARRAY_SIZE(modp->compiled->features[0].iffeatures[0].features));
    assert_ptr_equal(&modp->compiled->features[1], modp->compiled->features[0].iffeatures[0].features[0]);
    assert_non_null(modp->compiled->features);
    assert_int_equal(2, LY_ARRAY_SIZE(modp->compiled->features));
    assert_non_null(modp->compiled->features[1].depfeatures);
    assert_int_equal(1, LY_ARRAY_SIZE(modp->compiled->features[1].depfeatures));
    assert_ptr_equal(&modp->compiled->features[0], modp->compiled->features[1].depfeatures[0]);

    /* invalid reference */
    assert_int_equal(LY_EINVAL, lys_feature_enable(&mod, "xxx"));
    logbuf_assert("Feature \"xxx\" not found in module \"a\".");

    reset_mod(&mod);

    /* some invalid expressions */
    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f{if-feature f1;}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f1\" of if-feature - unable to find feature \"f1\". /b:{feature='f'}");
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature 'f and';}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f and\" of if-feature - unexpected end of expression. /b:{feature='f2'}");
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f{if-feature 'or';}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"or\" of if-feature - unexpected end of expression. /b:{feature='f'}");
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature '(f1';}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"(f1\" of if-feature - non-matching opening and closing parentheses. /b:{feature='f2'}");
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature 'f1)';}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f1)\" of if-feature - non-matching opening and closing parentheses. /b:{feature='f2'}");
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature ---;}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"---\" of if-feature - unable to find feature \"---\". /b:{feature='f2'}");
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{namespace urn:b; prefix b; feature f1; feature f2{if-feature 'not f1';}}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"not f1\" of if-feature - YANG 1.1 expression in YANG 1.0 module. /b:{feature='f2'}");
    reset_mod(&mod);

    assert_int_equal(LY_SUCCESS, yang_parse_module(&ctx, "module b{namespace urn:b; prefix b; feature f1; feature f1;}", &mod));
    lys_parser_ctx_free(ctx);
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Duplicate identifier \"f1\" of feature statement. /b:{feature='f1'}");
    reset_mod(&mod);

    ly_ctx_set_module_imp_clb(mod.ctx, test_imp_clb, "submodule sz {belongs-to z {prefix z;} feature f1;}");
    assert_null(lys_parse_mem(mod.ctx, "module z{namespace urn:z; prefix z; include sz;feature f1;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"f1\" of feature statement. /z:{feature='f1'}");

    assert_null(lys_parse_mem(mod.ctx, "module aa{namespace urn:aa; prefix aa; feature f1 {if-feature f2;} feature f2 {if-feature f1;}}", LYS_IN_YANG));
    logbuf_assert("Feature \"f1\" is indirectly referenced from itself. /aa:{feature='f2'}");
    assert_null(lys_parse_mem(mod.ctx, "module ab{namespace urn:ab; prefix ab; feature f1 {if-feature f1;}}", LYS_IN_YANG));
    logbuf_assert("Feature \"f1\" is referenced from itself. /ab:{feature='f1'}");

    assert_null(lys_parse_mem(mod.ctx, "module bb{yang-version 1.1; namespace urn:bb; prefix bb; feature f {if-feature ();}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"()\" of if-feature - number of features in expression does not match the required number "
            "of operands for the operations. /bb:{feature='f'}");
    assert_null(lys_parse_mem(mod.ctx, "module bb{yang-version 1.1; namespace urn:bb; prefix bb; feature f1; feature f {if-feature 'f1(';}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"f1(\" of if-feature - non-matching opening and closing parentheses. /bb:{feature='f'}");
    assert_null(lys_parse_mem(mod.ctx, "module bb{yang-version 1.1; namespace urn:bb; prefix bb; feature f1; feature f {if-feature 'and f1';}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"and f1\" of if-feature - missing feature/expression before \"and\" operation. /bb:{feature='f'}");
    assert_null(lys_parse_mem(mod.ctx, "module bb{yang-version 1.1; namespace urn:bb; prefix bb; feature f1; feature f {if-feature 'f1 not ';}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"f1 not \" of if-feature - unexpected end of expression. /bb:{feature='f'}");
    assert_null(lys_parse_mem(mod.ctx, "module bb{yang-version 1.1; namespace urn:bb; prefix bb; feature f1; feature f {if-feature 'f1 not not ';}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"f1 not not \" of if-feature - unexpected end of expression. /bb:{feature='f'}");
    assert_null(lys_parse_mem(mod.ctx, "module bb{yang-version 1.1; namespace urn:bb; prefix bb; feature f1; feature f2; "
                              "feature f {if-feature 'or f1 f2';}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"or f1 f2\" of if-feature - missing feature/expression before \"or\" operation. /bb:{feature='f'}");

    /* import reference */
    assert_non_null(modp = lys_parse_mem(mod.ctx, str, LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(modp, "f1"));
    assert_non_null(modp = lys_parse_mem(mod.ctx, "module c{namespace urn:c; prefix c; import a {prefix a;} feature f1; feature f2{if-feature 'a:f1';}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(modp, "f2"));
    assert_int_equal(0, lys_feature_value(modp, "f1"));
    assert_int_equal(1, lys_feature_value(modp, "f2"));

    *state = NULL;
    ly_ctx_destroy(mod.ctx, NULL);
}

static void
test_identity(void **state)
{
    *state = test_identity;

    struct ly_ctx *ctx;
    struct lys_module *mod1, *mod2;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));
    assert_non_null(mod1 = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a; identity a1;}", LYS_IN_YANG));
    assert_non_null(mod2 = lys_parse_mem(ctx, "module b {yang-version 1.1;namespace urn:b;prefix b; import a {prefix a;}"
                                         "identity b1; identity b2; identity b3 {base b1; base b:b2; base a:a1;}"
                                         "identity b4 {base b:b1; base b3;}}", LYS_IN_YANG));

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

    assert_non_null(mod2 = lys_parse_mem(ctx, "module c {yang-version 1.1;namespace urn:c;prefix c;"
                                             "identity c2 {base c1;} identity c1;}", LYS_IN_YANG));
    assert_int_equal(1, LY_ARRAY_SIZE(mod2->compiled->identities[1].derived));
    assert_ptr_equal(mod2->compiled->identities[1].derived[0], &mod2->compiled->identities[0]);

    assert_null(lys_parse_mem(ctx, "module aa{namespace urn:aa; prefix aa; identity i1;identity i1;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"i1\" of identity statement. /aa:{identity='i1'}");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule sbb {belongs-to bb {prefix bb;} identity i1;}");
    assert_null(lys_parse_mem(ctx, "module bb{namespace urn:bb; prefix bb; include sbb;identity i1;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"i1\" of identity statement. /bb:{identity='i1'}");

    assert_null(lys_parse_mem(ctx, "module cc{namespace urn:cc; prefix cc; identity i1 {base i2;}}", LYS_IN_YANG));
    logbuf_assert("Unable to find base (i2) of identity \"i1\". /cc:{identity='i1'}");

    assert_null(lys_parse_mem(ctx, "module dd{namespace urn:dd; prefix dd; identity i1 {base i1;}}", LYS_IN_YANG));
    logbuf_assert("Identity \"i1\" is derived from itself. /dd:{identity='i1'}");
    assert_null(lys_parse_mem(ctx, "module de{namespace urn:de; prefix de; identity i1 {base i2;}identity i2 {base i3;}identity i3 {base i1;}}", LYS_IN_YANG));
    logbuf_assert("Identity \"i1\" is indirectly derived from itself. /de:{identity='i3'}");

    *state = NULL;
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
    assert_non_null(mod->compiled);
    assert_non_null((cont = (struct lysc_node_container*)mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, cont->nodetype);
    assert_string_equal("c", cont->name);
    assert_true(cont->flags & LYS_CONFIG_W);
    assert_true(cont->flags & LYS_STATUS_CURR);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;container c {config false; status deprecated; container child;}}", LYS_IN_YANG));
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

static void
test_node_leaflist(void **state)
{
    *state = test_node_leaflist;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;
    struct lysc_node_leaflist *ll;
    struct lysc_node_leaf *l;
    const char *dflt;
    int dynamic;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;"
                                        "typedef mytype {type union {type leafref {path ../target;} type string;}}"
                                        "leaf-list ll1 {type union {type decimal64 {fraction-digits 2;} type mytype;}}"
                                        "leaf-list ll2 {type leafref {path ../target;}}"
                                        "leaf target {type int8;}}",
                                        LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union*)type)->types);
    assert_int_equal(3, LY_ARRAY_SIZE(((struct lysc_type_union*)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union*)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_LEAFREF, ((struct lysc_type_union*)type)->types[1]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union*)type)->types[2]->basetype);
    assert_non_null(((struct lysc_type_leafref*)((struct lysc_type_union*)type)->types[1])->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref*)((struct lysc_type_union*)type)->types[1])->realtype->basetype);
    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref*)type)->realtype->basetype);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;leaf-list ll {type string;}}", LYS_IN_YANG));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_int_equal(0, ll->min);
    assert_int_equal((uint32_t)-1, ll->max);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {yang-version 1.1;namespace urn:c;prefix c;typedef mytype {type int8;default 10;}"
                                        "leaf-list ll1 {type mytype;default 1; default 1; config false;}"
                                        "leaf-list ll2 {type mytype; ordered-by user;}}", LYS_IN_YANG));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_non_null(ll->dflts);
    assert_int_equal(6, ll->type->refcount); /* 3x type's reference, 3x default value's reference (typedef's default does not reference own type) */
    assert_int_equal(2, LY_ARRAY_SIZE(ll->dflts));
    assert_string_equal("1", dflt = ll->dflts[0]->realtype->plugin->print(ll->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("1", dflt = ll->dflts[1]->realtype->plugin->print(ll->dflts[1], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM | LYS_SET_DFLT | LYS_SET_CONFIG, ll->flags);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data->next));
    assert_non_null(ll->dflts);
    assert_int_equal(6, ll->type->refcount); /* 3x type's reference, 3x default value's reference */
    assert_int_equal(1, LY_ARRAY_SIZE(ll->dflts));
    assert_string_equal("10", dflt = ll->dflts[0]->realtype->plugin->print(ll->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, ll->flags);

    /* ordered-by is ignored for state data, RPC/action output parameters and notification content */
    assert_non_null(mod = lys_parse_mem(ctx, "module d {yang-version 1.1;namespace urn:d;prefix d;"
                                        "leaf-list ll {config false; type string; ordered-by user;}}", LYS_IN_YANG));
    /* but warning is present: */
    logbuf_assert("The ordered-by statement is ignored in lists representing state data (/d:ll).");
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM | LYS_SET_CONFIG, ll->flags);
    logbuf_clean();

    assert_non_null(mod = lys_parse_mem(ctx, "module e {yang-version 1.1;namespace urn:e;prefix e;"
                                        "rpc oper {output {leaf-list ll {type string; ordered-by user;}}}}", LYS_IN_YANG));
    logbuf_assert("The ordered-by statement is ignored in lists representing RPC/action output parameters (/e:oper/output/ll).");
    logbuf_clean();

    assert_non_null(mod = lys_parse_mem(ctx, "module f {yang-version 1.1;namespace urn:f;prefix f;"
                                        "notification event {leaf-list ll {type string; ordered-by user;}}}", LYS_IN_YANG));
    logbuf_assert("The ordered-by statement is ignored in lists representing notification content (/f:event/ll).");

    /* forward reference in default */
    assert_non_null(mod = lys_parse_mem(ctx, "module g {yang-version 1.1; namespace urn:g;prefix g;"
                                        "leaf ref {type instance-identifier {require-instance true;} default \"/g:g\";}"
                                        "leaf-list g {type string;}}", LYS_IN_YANG));
    assert_non_null(l = (struct lysc_node_leaf*)mod->compiled->data);
    assert_string_equal("ref", l->name);
    assert_non_null(l->dflt);
    assert_null(l->dflt->canonical_cache);

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;leaf-list ll {type empty;}}", LYS_IN_YANG));
    logbuf_assert("Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules. /aa:ll");

    assert_null(lys_parse_mem(ctx, "module bb {yang-version 1.1;namespace urn:bb;prefix bb;leaf-list ll {type empty; default x;}}", LYS_IN_YANG));
    logbuf_assert("Invalid leaf-lists's default value \"x\" which does not fit the type (Invalid empty value \"x\".). /bb:ll");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;"
                                        "leaf-list ll {config false;type string; default one;default two;default one;}}", LYS_IN_YANG));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_non_null(ll->dflts);
    assert_int_equal(3, LY_ARRAY_SIZE(ll->dflts));
    assert_null(lys_parse_mem(ctx, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;"
                              "leaf-list ll {type string; default one;default two;default one;}}", LYS_IN_YANG));
    logbuf_assert("Configuration leaf-list has multiple defaults of the same value \"one\". /dd:ll");

    assert_null(lys_parse_mem(ctx, "module ee {yang-version 1.1; namespace urn:ee;prefix ee;"
                              "leaf ref {type instance-identifier {require-instance true;} default \"/ee:g\";}}", LYS_IN_YANG));
    logbuf_assert("Invalid default - value does not fit the type "
                  "(Invalid instance-identifier \"/ee:g\" value - path \"/ee:g\" does not exists in the YANG schema.). /ee:ref");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_node_list(void **state)
{
    *state = test_node_list;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_node_list *list;
    struct lysc_node *child;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;feature f;"
                                        "list l1 {key \"x y\"; ordered-by user; leaf y{type string;if-feature f;} leaf x {type string; when 1;}}"
                                        "list l2 {config false;leaf value {type string;}}}", LYS_IN_YANG));
    list = (struct lysc_node_list*)mod->compiled->data;
    assert_non_null(list);
    assert_non_null(list->child);
    assert_string_equal("x", list->child->name);
    assert_true(list->child->flags & LYS_KEY);
    assert_string_equal("y", list->child->next->name);
    assert_true(list->child->next->flags & LYS_KEY);
    assert_non_null(list->child);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, list->flags);
    assert_true(list->child->flags & LYS_KEY);
    assert_true(list->child->next->flags & LYS_KEY);
    list = (struct lysc_node_list*)mod->compiled->data->next;
    assert_non_null(list);
    assert_non_null(list->child);
    assert_false(list->child->flags & LYS_KEY);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM | LYS_SET_CONFIG | LYS_KEYLESS, list->flags);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;"
                                        "list l {key a; unique \"a c/b:b\"; unique \"c/e d\";"
                                        "leaf a {type string; default x;} leaf d {type string;config false;}"
                                        "container c {leaf b {type string;}leaf e{type string;config false;}}}}", LYS_IN_YANG));
    list = (struct lysc_node_list*)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_string_equal("a", list->child->name);
    assert_true(list->child->flags & LYS_KEY);
    assert_null(((struct lysc_node_leaf*)list->child)->dflt);
    assert_non_null(list->uniques);
    assert_int_equal(2, LY_ARRAY_SIZE(list->uniques));
    assert_int_equal(2, LY_ARRAY_SIZE(list->uniques[0]));
    assert_string_equal("a", list->uniques[0][0]->name);
    assert_true(list->uniques[0][0]->flags & LYS_UNIQUE);
    assert_string_equal("b", list->uniques[0][1]->name);
    assert_true(list->uniques[0][1]->flags & LYS_UNIQUE);
    assert_int_equal(2, LY_ARRAY_SIZE(list->uniques[1]));
    assert_string_equal("e", list->uniques[1][0]->name);
    assert_true(list->uniques[1][0]->flags & LYS_UNIQUE);
    assert_string_equal("d", list->uniques[1][1]->name);
    assert_true(list->uniques[1][1]->flags & LYS_UNIQUE);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {yang-version 1.1;namespace urn:c;prefix c;"
                                        "list l {key a;leaf a {type empty;}}}", LYS_IN_YANG));
    list = (struct lysc_node_list*)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_string_equal("a", list->child->name);
    assert_true(list->child->flags & LYS_KEY);
    assert_int_equal(LY_TYPE_EMPTY, ((struct lysc_node_leaf*)list->child)->type->basetype);

    /* keys order */
    assert_non_null(mod = lys_parse_mem(ctx, "module d {yang-version 1.1;namespace urn:d;prefix d;"
                                        "list l {key \"d b c\";leaf a {type string;} leaf b {type string;} leaf c {type string;} leaf d {type string;}}}", LYS_IN_YANG));
    list = (struct lysc_node_list*)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_non_null(child = list->child);
    assert_string_equal("d", child->name);
    assert_true(child->flags & LYS_KEY);
    assert_non_null(child = child->next);
    assert_string_equal("b", child->name);
    assert_true(child->flags & LYS_KEY);
    assert_non_null(child = child->next);
    assert_string_equal("c", child->name);
    assert_true(child->flags & LYS_KEY);
    assert_non_null(child = child->next);
    assert_string_equal("a", child->name);
    assert_false(child->flags & LYS_KEY);

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;list l;}", LYS_IN_YANG));
    logbuf_assert("Missing key in list representing configuration data. /aa:l");

    assert_null(lys_parse_mem(ctx, "module bb {yang-version 1.1; namespace urn:bb;prefix bb;"
                              "list l {key x; leaf x {type string; when 1;}}}", LYS_IN_YANG));
    logbuf_assert("List's key must not have any \"when\" statement. /bb:l/x");

    assert_null(lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;feature f;"
                              "list l {key x; leaf x {type string; if-feature f;}}}", LYS_IN_YANG));
    logbuf_assert("List's key must not have any \"if-feature\" statement. /cc:l/x");

    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;"
                              "list l {key x; leaf x {type string; config false;}}}", LYS_IN_YANG));
    logbuf_assert("Key of the configuration list must not be status leaf. /dd:l/x");

    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;"
                              "list l {config false;key x; leaf x {type string; config true;}}}", LYS_IN_YANG));
    logbuf_assert("Configuration node cannot be child of any state data node. /ee:l/x");

    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;"
                              "list l {key x; leaf-list x {type string;}}}", LYS_IN_YANG));
    logbuf_assert("The list's key \"x\" not found. /ff:l");

    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;"
                              "list l {key x; unique y;leaf x {type string;} leaf-list y {type string;}}}", LYS_IN_YANG));
    logbuf_assert("Unique's descendant-schema-nodeid \"y\" refers to leaf-list node instead of a leaf. /gg:l");

    assert_null(lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;"
                              "list l {key x; unique \"x y\";leaf x {type string;} leaf y {config false; type string;}}}", LYS_IN_YANG));
    logbuf_assert("Unique statement \"x y\" refers to leafs with different config type. /hh:l");

    assert_null(lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;"
                              "list l {key x; unique a:x;leaf x {type string;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid descendant-schema-nodeid value \"a:x\" - prefix \"a\" not defined in module \"ii\". /ii:l");

    assert_null(lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;"
                              "list l {key x; unique c/x;leaf x {type string;}container c {leaf y {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid descendant-schema-nodeid value \"c/x\" - target node not found. /jj:l");

    assert_null( lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;"
                               "list l {key x; unique c^y;leaf x {type string;}container c {leaf y {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid descendant-schema-nodeid value \"c^\" - missing \"/\" as node-identifier separator. /kk:l");

    assert_null(lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;"
                              "list l {key \"x y x\";leaf x {type string;}leaf y {type string;}}}", LYS_IN_YANG));
    logbuf_assert("Duplicated key identifier \"x\". /ll:l");

    assert_null(lys_parse_mem(ctx, "module mm {namespace urn:mm;prefix mm;"
                              "list l {key x;leaf x {type empty;}}}", LYS_IN_YANG));
    logbuf_assert("List's key cannot be of \"empty\" type until it is in YANG 1.1 module. /mm:l/x");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_node_choice(void **state)
{
    *state = test_node_choice;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_node_choice *ch;
    struct lysc_node_case *cs;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;feature f;"
                                        "choice ch {default a:b; when \"true()\"; case a {leaf a1 {type string;}leaf a2 {type string;}}"
                                        "leaf b {type string;}}}", LYS_IN_YANG));
    ch = (struct lysc_node_choice*)mod->compiled->data;
    assert_non_null(ch);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR, ch->flags);
    assert_int_equal(1, LY_ARRAY_SIZE(ch->when));
    assert_null(ch->when[0]->context);
    cs = ch->cases;
    assert_non_null(cs);
    assert_string_equal("a", cs->name);
    assert_ptr_equal(ch, cs->parent);
    assert_non_null(cs->child);
    assert_string_equal("a1", cs->child->name);
    assert_non_null(cs->child->next);
    assert_string_equal("a2", cs->child->next->name);
    assert_ptr_equal(cs, cs->child->parent);
    cs = (struct lysc_node_case*)cs->next;
    assert_non_null(cs);
    assert_string_equal("b", cs->name);
    assert_int_equal(LYS_STATUS_CURR | LYS_SET_DFLT, cs->flags);
    assert_ptr_equal(ch, cs->parent);
    assert_non_null(cs->child);
    assert_string_equal("b", cs->child->name);
    assert_ptr_equal(cs, cs->child->parent);
    assert_ptr_equal(ch->cases->child->next, cs->child->prev);
    assert_ptr_equal(ch->cases->child->next->next, cs->child);
    assert_ptr_equal(ch->cases->child->prev, cs->child);
    assert_ptr_equal(ch->dflt, cs);

    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                              "choice ch {case a {leaf x {type string;}}leaf x {type string;}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"x\" of data definition/RPC/action/Notification statement. /aa:ch/x/x");
    assert_null(lys_parse_mem(ctx, "module aa2 {namespace urn:aa2;prefix aa;"
                              "choice ch {case a {leaf y {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"y\" of data definition/RPC/action/Notification statement. /aa2:ch/b/y");
    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;"
                              "choice ch {case a {leaf x {type string;}}leaf a {type string;}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"a\" of case statement. /bb:ch/a");
    assert_null(lys_parse_mem(ctx, "module bb2 {namespace urn:bb2;prefix bb;"
                              "choice ch {case b {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"b\" of case statement. /bb2:ch/b");

    assert_null(lys_parse_mem(ctx, "module ca {namespace urn:ca;prefix ca;"
                              "choice ch {default c;case a {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Default case \"c\" not found. /ca:ch");
    assert_null(lys_parse_mem(ctx, "module cb {namespace urn:cb;prefix cb; import a {prefix a;}"
                              "choice ch {default a:a;case a {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid default case referencing a case from different YANG module (by prefix \"a\"). /cb:ch");
    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;"
                              "choice ch {default a;case a {leaf x {mandatory true;type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Mandatory node \"x\" under the default case \"a\". /cc:ch");
    /* TODO check with mandatory nodes from augment placed into the case */

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_node_anydata(void **state)
{
    *state = test_node_anydata;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_node_anydata *any;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1;namespace urn:a;prefix a;"
                                        "anydata any {config false;mandatory true;}}", LYS_IN_YANG));
    any = (struct lysc_node_anydata*)mod->compiled->data;
    assert_non_null(any);
    assert_int_equal(LYS_ANYDATA, any->nodetype);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_MAND_TRUE | LYS_SET_CONFIG, any->flags);

    logbuf_clean();
    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;"
                                        "anyxml any;}", LYS_IN_YANG));
    any = (struct lysc_node_anydata*)mod->compiled->data;
    assert_non_null(any);
    assert_int_equal(LYS_ANYXML, any->nodetype);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR, any->flags);
    logbuf_assert("Use of anyxml to define configuration data is not recommended."); /* warning */

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;anydata any;}", LYS_IN_YANG));
    logbuf_assert("Invalid keyword \"anydata\" as a child of \"module\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_action(void **state)
{
    *state = test_action;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    const struct lysc_action *rpc;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;"
                                        "rpc a {input {leaf x {type int8;} leaf y {type int8;}} output {leaf result {type int16;}}}}", LYS_IN_YANG));
    rpc = mod->compiled->rpcs;
    assert_non_null(rpc);
    assert_int_equal(1, LY_ARRAY_SIZE(rpc));
    assert_int_equal(LYS_ACTION, rpc->nodetype);
    assert_int_equal(LYS_STATUS_CURR, rpc->flags);
    assert_string_equal("a", rpc->name);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1; namespace urn:b;prefix b; container top {"
                                        "action b {input {leaf x {type int8;} leaf y {type int8;}}"
                                        "output {must \"result > 25\"; must \"/top\"; leaf result {type int16;}}}}}", LYS_IN_YANG));
    rpc = lysc_node_actions(mod->compiled->data);
    assert_non_null(rpc);
    assert_int_equal(1, LY_ARRAY_SIZE(rpc));
    assert_int_equal(LYS_ACTION, rpc->nodetype);
    assert_int_equal(LYS_STATUS_CURR, rpc->flags);
    assert_string_equal("b", rpc->name);
    assert_null(rpc->input.musts);
    assert_int_equal(2, LY_ARRAY_SIZE(rpc->output.musts));

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;container top {action x;}}", LYS_IN_YANG));
    logbuf_assert("Invalid keyword \"action\" as a child of \"container\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;leaf x{type string;} rpc x;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"x\" of data definition/RPC/action/Notification statement. /bb:x");
    assert_null(lys_parse_mem(ctx, "module cc {yang-version 1.1; namespace urn:cc;prefix cc;container c {leaf y {type string;} action y;}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"y\" of data definition/RPC/action/Notification statement. /cc:c/y");
    assert_null(lys_parse_mem(ctx, "module dd {yang-version 1.1; namespace urn:dd;prefix dd;container c {action z; action z;}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"z\" of data definition/RPC/action/Notification statement. /dd:c/z");
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule eesub {belongs-to ee {prefix ee;} notification w;}");
    assert_null(lys_parse_mem(ctx, "module ee {yang-version 1.1; namespace urn:ee;prefix ee;include eesub; rpc w;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"w\" of data definition/RPC/action/Notification statement. /ee:w");

    assert_null(lys_parse_mem(ctx, "module ff {yang-version 1.1; namespace urn:ff;prefix ff; rpc test {input {container a {leaf b {type string;}}}}"
                              "augment /test/input/a {action invalid {input {leaf x {type string;}}}}}", LYS_IN_YANG));
    logbuf_assert("Action \"invalid\" is placed inside another RPC/action. /ff:{augment='/test/input/a'}/invalid");

    assert_null(lys_parse_mem(ctx, "module gg {yang-version 1.1; namespace urn:gg;prefix gg; notification test {container a {leaf b {type string;}}}"
                              "augment /test/a {action invalid {input {leaf x {type string;}}}}}", LYS_IN_YANG));
    logbuf_assert("Action \"invalid\" is placed inside Notification. /gg:{augment='/test/a'}/invalid");

    assert_null(lys_parse_mem(ctx, "module hh {yang-version 1.1; namespace urn:hh;prefix hh; notification test {container a {uses grp;}}"
                              "grouping grp {action invalid {input {leaf x {type string;}}}}}", LYS_IN_YANG));
    logbuf_assert("Action \"invalid\" is placed inside Notification. /hh:test/a/{uses='grp'}/invalid");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_notification(void **state)
{
    *state = test_notification;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    const struct lysc_notif *notif;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;"
                                        "notification a1 {leaf x {type int8;}} notification a2;}", LYS_IN_YANG));
    notif = mod->compiled->notifs;
    assert_non_null(notif);
    assert_int_equal(2, LY_ARRAY_SIZE(notif));
    assert_int_equal(LYS_NOTIF, notif->nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif->flags);
    assert_string_equal("a1", notif->name);
    assert_non_null(notif->data);
    assert_string_equal("x", notif->data->name);
    assert_int_equal(LYS_NOTIF, notif[1].nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif[1].flags);
    assert_string_equal("a2", notif[1].name);
    assert_null(notif[1].data);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1; namespace urn:b;prefix b; container top {"
                                        "notification b1 {leaf x {type int8;}} notification b2 {must \"/top\";}}}", LYS_IN_YANG));
    notif = lysc_node_notifs(mod->compiled->data);
    assert_non_null(notif);
    assert_int_equal(2, LY_ARRAY_SIZE(notif));
    assert_int_equal(LYS_NOTIF, notif->nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif->flags);
    assert_string_equal("b1", notif->name);
    assert_non_null(notif->data);
    assert_string_equal("x", notif->data->name);
    assert_int_equal(LYS_NOTIF, notif[1].nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif[1].flags);
    assert_string_equal("b2", notif[1].name);
    assert_null(notif[1].data);
    assert_int_equal(1, LY_ARRAY_SIZE(notif[1].musts));

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;container top {notification x;}}", LYS_IN_YANG));
    logbuf_assert("Invalid keyword \"notification\" as a child of \"container\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;leaf x{type string;} notification x;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"x\" of data definition/RPC/action/Notification statement. /bb:x");
    assert_null(lys_parse_mem(ctx, "module cc {yang-version 1.1; namespace urn:cc;prefix cc;container c {leaf y {type string;} notification y;}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"y\" of data definition/RPC/action/Notification statement. /cc:c/y");
    assert_null(lys_parse_mem(ctx, "module dd {yang-version 1.1; namespace urn:dd;prefix dd;container c {notification z; notification z;}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"z\" of data definition/RPC/action/Notification statement. /dd:c/z");
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule eesub {belongs-to ee {prefix ee;} rpc w;}");
    assert_null(lys_parse_mem(ctx, "module ee {yang-version 1.1; namespace urn:ee;prefix ee;include eesub; notification w;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"w\" of data definition/RPC/action/Notification statement. /ee:w");

    assert_null(lys_parse_mem(ctx, "module ff {yang-version 1.1; namespace urn:ff;prefix ff; rpc test {input {container a {leaf b {type string;}}}}"
                              "augment /test/input/a {notification invalid {leaf x {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Notification \"invalid\" is placed inside RPC/action. /ff:{augment='/test/input/a'}/invalid");

    assert_null(lys_parse_mem(ctx, "module gg {yang-version 1.1; namespace urn:gg;prefix gg; notification test {container a {leaf b {type string;}}}"
                              "augment /test/a {notification invalid {leaf x {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Notification \"invalid\" is placed inside another Notification. /gg:{augment='/test/a'}/invalid");

    assert_null(lys_parse_mem(ctx, "module hh {yang-version 1.1; namespace urn:hh;prefix hh; rpc test {input {container a {uses grp;}}}"
                              "grouping grp {notification invalid {leaf x {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Notification \"invalid\" is placed inside RPC/action. /hh:test/input/a/{uses='grp'}/invalid");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

/**
 * actually the same as length restriction (tested in test_type_length()), so just check the correct handling in appropriate types,
 * do not test the expression itself
 */
static void
test_type_range(void **state)
{
    *state = test_type_range;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;leaf l {type int8 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT8, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(-128, ((struct lysc_type_num*)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_64);
    assert_int_equal(127, ((struct lysc_type_num*)type)->range->parts[1].min_64);
    assert_int_equal(127, ((struct lysc_type_num*)type)->range->parts[1].max_64);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;leaf l {type int16 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT16, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(-32768, ((struct lysc_type_num*)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_64);
    assert_int_equal(32767, ((struct lysc_type_num*)type)->range->parts[1].min_64);
    assert_int_equal(32767, ((struct lysc_type_num*)type)->range->parts[1].max_64);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c;leaf l {type int32 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT32, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(INT64_C(-2147483648), ((struct lysc_type_num*)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_64);
    assert_int_equal(INT64_C(2147483647), ((struct lysc_type_num*)type)->range->parts[1].min_64);
    assert_int_equal(INT64_C(2147483647), ((struct lysc_type_num*)type)->range->parts[1].max_64);

    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d;leaf l {type int64 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT64, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(INT64_C(-9223372036854775807) - INT64_C(1), ((struct lysc_type_num*)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_64);
    assert_int_equal(INT64_C(9223372036854775807), ((struct lysc_type_num*)type)->range->parts[1].min_64);
    assert_int_equal(INT64_C(9223372036854775807), ((struct lysc_type_num*)type)->range->parts[1].max_64);

    assert_non_null(mod = lys_parse_mem(ctx, "module e {namespace urn:e;prefix e;leaf l {type uint8 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT8, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num*)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_u64);
    assert_int_equal(255, ((struct lysc_type_num*)type)->range->parts[1].min_u64);
    assert_int_equal(255, ((struct lysc_type_num*)type)->range->parts[1].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module f {namespace urn:f;prefix f;leaf l {type uint16 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT16, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num*)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_u64);
    assert_int_equal(65535, ((struct lysc_type_num*)type)->range->parts[1].min_u64);
    assert_int_equal(65535, ((struct lysc_type_num*)type)->range->parts[1].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module g {namespace urn:g;prefix g;leaf l {type uint32 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT32, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num*)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_u64);
    assert_int_equal(UINT64_C(4294967295), ((struct lysc_type_num*)type)->range->parts[1].min_u64);
    assert_int_equal(UINT64_C(4294967295), ((struct lysc_type_num*)type)->range->parts[1].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module h {namespace urn:h;prefix h;leaf l {type uint64 {range min..10|max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT64, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num*)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_num*)type)->range->parts[1].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_num*)type)->range->parts[1].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module i {namespace urn:i;prefix i;typedef mytype {type uint8 {range 10..100;}}"
                                             "typedef mytype2 {type mytype;} leaf l {type mytype2;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount);
    assert_int_equal(LY_TYPE_UINT8, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));

    assert_non_null(mod = lys_parse_mem(ctx, "module j {namespace urn:j;prefix j;"
                                             "typedef mytype {type uint8 {range 1..100{description \"one to hundred\";reference A;}}}"
                                             "leaf l {type mytype {range 1..10 {description \"one to ten\";reference B;}}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UINT8, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_string_equal("one to ten", ((struct lysc_type_num*)type)->range->dsc);
    assert_string_equal("B", ((struct lysc_type_num*)type)->range->ref);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));
    assert_int_equal(1, ((struct lysc_type_num*)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num*)type)->range->parts[0].max_u64);

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_length(void **state)
{
    *state = test_type_length;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;leaf l {type binary {length min {error-app-tag errortag;error-message error;}}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_string_equal("errortag", ((struct lysc_type_bin*)type)->length->eapptag);
    assert_string_equal("error", ((struct lysc_type_bin*)type)->length->emsg);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(0, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(0, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;leaf l {type binary {length max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c;leaf l {type binary {length min..max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(0, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d;leaf l {type binary {length 5;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(5, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(5, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module e {namespace urn:e;prefix e;leaf l {type binary {length 1..10;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(1, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module f {namespace urn:f;prefix f;leaf l {type binary {length 1..10|20..30;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(1, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);
    assert_int_equal(20, ((struct lysc_type_bin*)type)->length->parts[1].min_u64);
    assert_int_equal(30, ((struct lysc_type_bin*)type)->length->parts[1].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module g {namespace urn:g;prefix g;leaf l {type binary {length \"16 | 32\";}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(16, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(16, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);
    assert_int_equal(32, ((struct lysc_type_bin*)type)->length->parts[1].min_u64);
    assert_int_equal(32, ((struct lysc_type_bin*)type)->length->parts[1].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module h {namespace urn:h;prefix h;typedef mytype {type binary {length 10;}}"
                                             "leaf l {type mytype {length \"10\";}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module i {namespace urn:i;prefix i;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length \"50\";}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(50, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(50, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module j {namespace urn:j;prefix j;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length \"10..30|60..100\";}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(30, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);
    assert_int_equal(60, ((struct lysc_type_bin*)type)->length->parts[1].min_u64);
    assert_int_equal(100, ((struct lysc_type_bin*)type)->length->parts[1].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module k {namespace urn:k;prefix k;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length \"10..80\";}}leaf ll {type mytype;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(80, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);
    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(2, type->refcount);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(100, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module l {namespace urn:l;prefix l;typedef mytype {type string {length 10..100;}}"
                                             "typedef mytype2 {type mytype {pattern '[0-9]*';}} leaf l {type mytype2 {pattern '[0-4]*';}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str*)type)->length);
    assert_non_null(((struct lysc_type_str*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_str*)type)->length->parts[0].min_u64);
    assert_int_equal(100, ((struct lysc_type_str*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module m {namespace urn:m;prefix m;typedef mytype {type string {length 10;}}"
                                             "leaf l {type mytype {length min..max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str*)type)->length);
    assert_non_null(((struct lysc_type_str*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_str*)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_str*)type)->length->parts[0].max_u64);

    /* invalid values */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;leaf l {type binary {length -10;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - value \"-10\" does not fit the type limitations. /aa:l");
    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;leaf l {type binary {length 18446744073709551616;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - invalid value \"18446744073709551616\". /bb:l");
    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;leaf l {type binary {length \"max .. 10\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - unexpected data after max keyword (.. 10). /cc:l");
    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;leaf l {type binary {length 50..10;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - values are not in ascending order (10). /dd:l");
    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;leaf l {type binary {length \"50 | 10\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - values are not in ascending order (10). /ee:l");
    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;leaf l {type binary {length \"x\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - unexpected data (x). /ff:l");
    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;leaf l {type binary {length \"50 | min\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - unexpected data before min keyword (50 | ). /gg:l");
    assert_null(lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;leaf l {type binary {length \"| 50\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - unexpected beginning of the expression (| 50). /hh:l");
    assert_null(lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;leaf l {type binary {length \"10 ..\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - unexpected end of the expression after \"..\" (10 ..). /ii:l");
    assert_null(lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;leaf l {type binary {length \".. 10\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - unexpected \"..\" without a lower bound. /jj:l");
    assert_null(lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;leaf l {type binary {length \"10 |\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - unexpected end of the expression (10 |). /kk:l");
    assert_null(lys_parse_mem(ctx, "module kl {namespace urn:kl;prefix kl;leaf l {type binary {length \"10..20 | 15..30\";}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - values are not in ascending order (15). /kl:l");

    assert_null(lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;typedef mytype {type binary {length 10;}}"
                                   "leaf l {type mytype {length 11;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - the derived restriction (11) is not equally or more limiting. /ll:l");
    assert_null(lys_parse_mem(ctx, "module mm {namespace urn:mm;prefix mm;typedef mytype {type binary {length 10..100;}}"
                                   "leaf l {type mytype {length 1..11;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - the derived restriction (1..11) is not equally or more limiting. /mm:l");
    assert_null(lys_parse_mem(ctx, "module nn {namespace urn:nn;prefix nn;typedef mytype {type binary {length 10..100;}}"
                                   "leaf l {type mytype {length 20..110;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - the derived restriction (20..110) is not equally or more limiting. /nn:l");
    assert_null(lys_parse_mem(ctx, "module oo {namespace urn:oo;prefix oo;typedef mytype {type binary {length 10..100;}}"
                                   "leaf l {type mytype {length 20..30|110..120;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - the derived restriction (20..30|110..120) is not equally or more limiting. /oo:l");
    assert_null(lys_parse_mem(ctx, "module pp {namespace urn:pp;prefix pp;typedef mytype {type binary {length 10..11;}}"
                                             "leaf l {type mytype {length 15;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - the derived restriction (15) is not equally or more limiting. /pp:l");
    assert_null(lys_parse_mem(ctx, "module qq {namespace urn:qq;prefix qq;typedef mytype {type binary {length 10..20|30..40;}}"
                                             "leaf l {type mytype {length 15..35;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - the derived restriction (15..35) is not equally or more limiting. /qq:l");
    assert_null(lys_parse_mem(ctx, "module rr {namespace urn:rr;prefix rr;typedef mytype {type binary {length 10;}}"
                                             "leaf l {type mytype {length 10..35;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid length restriction - the derived restriction (10..35) is not equally or more limiting. /rr:l");

    assert_null(lys_parse_mem(ctx, "module ss {namespace urn:ss;prefix ss;leaf l {type binary {pattern '[0-9]*';}}}", LYS_IN_YANG));
    logbuf_assert("Invalid type restrictions for binary type. /ss:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_pattern(void **state)
{
    *state = test_type_pattern;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1; namespace urn:a;prefix a;leaf l {type string {"
                                        "pattern .* {error-app-tag errortag;error-message error;}"
                                        "pattern [0-9].*[0-9] {modifier invert-match;}}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
    assert_string_equal("errortag", ((struct lysc_type_str*)type)->patterns[0]->eapptag);
    assert_string_equal("error", ((struct lysc_type_str*)type)->patterns[0]->emsg);
    assert_string_equal(".*", ((struct lysc_type_str*)type)->patterns[0]->expr);
    assert_int_equal(0, ((struct lysc_type_str*)type)->patterns[0]->inverted);
    assert_null(((struct lysc_type_str*)type)->patterns[1]->eapptag);
    assert_null(((struct lysc_type_str*)type)->patterns[1]->emsg);
    assert_string_equal("[0-9].*[0-9]", ((struct lysc_type_str*)type)->patterns[1]->expr);
    assert_int_equal(1, ((struct lysc_type_str*)type)->patterns[1]->inverted);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;typedef mytype {type string {pattern '[0-9]*';}}"
                                             "typedef mytype2 {type mytype {length 10;}} leaf l {type mytype2 {pattern '[0-4]*';}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
    assert_string_equal("[0-9]*", ((struct lysc_type_str*)type)->patterns[0]->expr);
    assert_int_equal(3, ((struct lysc_type_str*)type)->patterns[0]->refcount);
    assert_string_equal("[0-4]*", ((struct lysc_type_str*)type)->patterns[1]->expr);
    assert_int_equal(1, ((struct lysc_type_str*)type)->patterns[1]->refcount);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c;typedef mytype {type string {pattern '[0-9]*';}}"
                                             "leaf l {type mytype {length 10;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
    assert_string_equal("[0-9]*", ((struct lysc_type_str*)type)->patterns[0]->expr);
    assert_int_equal(2, ((struct lysc_type_str*)type)->patterns[0]->refcount);

    /* test substitutions */
    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d;leaf l {type string {"
                                        "pattern '^\\p{IsLatinExtended-A}$';}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
    assert_string_equal("^\\p{IsLatinExtended-A}$", ((struct lysc_type_str*)type)->patterns[0]->expr);
    /* TODO check some data "^ř$" */

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_enum(void **state)
{
    *state = test_type_enum;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1; namespace urn:a;prefix a;feature f; leaf l {type enumeration {"
                                        "enum automin; enum min {value -2147483648;}enum one {if-feature f; value 1;}"
                                        "enum two; enum seven {value 7;}enum eight;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_ENUM, type->basetype);
    assert_non_null(((struct lysc_type_enum*)type)->enums);
    assert_int_equal(6, LY_ARRAY_SIZE(((struct lysc_type_enum*)type)->enums));
    assert_non_null(((struct lysc_type_enum*)type)->enums[2].iffeatures);
    assert_string_equal("automin", ((struct lysc_type_enum*)type)->enums[0].name);
    assert_int_equal(0, ((struct lysc_type_enum*)type)->enums[0].value);
    assert_string_equal("min", ((struct lysc_type_enum*)type)->enums[1].name);
    assert_int_equal(-2147483648, ((struct lysc_type_enum*)type)->enums[1].value);
    assert_string_equal("one", ((struct lysc_type_enum*)type)->enums[2].name);
    assert_int_equal(1, ((struct lysc_type_enum*)type)->enums[2].value);
    assert_string_equal("two", ((struct lysc_type_enum*)type)->enums[3].name);
    assert_int_equal(2, ((struct lysc_type_enum*)type)->enums[3].value);
    assert_string_equal("seven", ((struct lysc_type_enum*)type)->enums[4].name);
    assert_int_equal(7, ((struct lysc_type_enum*)type)->enums[4].value);
    assert_string_equal("eight", ((struct lysc_type_enum*)type)->enums[5].name);
    assert_int_equal(8, ((struct lysc_type_enum*)type)->enums[5].value);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1; namespace urn:b;prefix b;feature f; typedef mytype {type enumeration {"
                                        "enum 11; enum min {value -2147483648;}enum x$&;"
                                        "enum two; enum seven {value 7;}enum eight;}} leaf l { type mytype {enum seven;enum eight;}}}",
                                        LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_ENUM, type->basetype);
    assert_non_null(((struct lysc_type_enum*)type)->enums);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_enum*)type)->enums));
    assert_string_equal("seven", ((struct lysc_type_enum*)type)->enums[0].name);
    assert_int_equal(7, ((struct lysc_type_enum*)type)->enums[0].value);
    assert_string_equal("eight", ((struct lysc_type_enum*)type)->enums[1].name);
    assert_int_equal(8, ((struct lysc_type_enum*)type)->enums[1].value);


    /* invalid cases */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; feature f; leaf l {type enumeration {"
                                   "enum one {if-feature f;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid keyword \"if-feature\" as a child of \"enum\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
                                   "enum one {value -2147483649;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"-2147483649\" of \"value\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
                                   "enum one {value 2147483648;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"2147483648\" of \"value\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
                                   "enum one; enum one;}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"one\" of enum statement. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
                                   "enum '';}}}", LYS_IN_YANG));
    logbuf_assert("Enum name must not be zero-length. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
                                   "enum ' x';}}}", LYS_IN_YANG));
    logbuf_assert("Enum name must not have any leading or trailing whitespaces (\" x\"). Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
                                   "enum 'x ';}}}", LYS_IN_YANG));
    logbuf_assert("Enum name must not have any leading or trailing whitespaces (\"x \"). Line number 1.");
    assert_non_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
                                  "enum 'inva\nlid';}}}", LYS_IN_YANG));
    logbuf_assert("Control characters in enum name should be avoided (\"inva\nlid\", character number 5).");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; leaf l {type enumeration;}}", LYS_IN_YANG));
    logbuf_assert("Missing enum substatement for enumeration type. /bb:l");

    assert_null(lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;typedef mytype {type enumeration {enum one;}}"
                                             "leaf l {type mytype {enum two;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid enumeration - derived type adds new item \"two\". /cc:l");

    assert_null(lys_parse_mem(ctx, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;typedef mytype {type enumeration {enum one;}}"
                                             "leaf l {type mytype {enum one {value 1;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid enumeration - value of the item \"one\" has changed from 0 to 1 in the derived type. /dd:l");
    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;leaf l {type enumeration {enum x {value 2147483647;}enum y;}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid enumeration - it is not possible to auto-assign enum value for \"y\" since the highest value is already 2147483647. /ee:l");

    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;leaf l {type enumeration {enum x {value 1;}enum y {value 1;}}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid enumeration - value 1 collide in items \"y\" and \"x\". /ff:l");

    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;typedef mytype {type enumeration;}"
                                             "leaf l {type mytype {enum one;}}}", LYS_IN_YANG));
    logbuf_assert("Missing enum substatement for enumeration type mytype. /gg:l");

    assert_null(lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh; typedef mytype {type enumeration {enum one;}}"
                                        "leaf l {type mytype {enum one;}}}", LYS_IN_YANG));
    logbuf_assert("Enumeration type can be subtyped only in YANG 1.1 modules. /hh:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_bits(void **state)
{
    *state = test_type_bits;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1; namespace urn:a;prefix a;feature f; leaf l {type bits {"
                                        "bit automin; bit one {if-feature f; position 1;}"
                                        "bit two; bit seven {position 7;}bit eight;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_BITS, type->basetype);
    assert_non_null(((struct lysc_type_bits*)type)->bits);
    assert_int_equal(5, LY_ARRAY_SIZE(((struct lysc_type_bits*)type)->bits));
    assert_non_null(((struct lysc_type_bits*)type)->bits[1].iffeatures);
    assert_string_equal("automin", ((struct lysc_type_bits*)type)->bits[0].name);
    assert_int_equal(0, ((struct lysc_type_bits*)type)->bits[0].position);
    assert_string_equal("one", ((struct lysc_type_bits*)type)->bits[1].name);
    assert_int_equal(1, ((struct lysc_type_bits*)type)->bits[1].position);
    assert_string_equal("two", ((struct lysc_type_bits*)type)->bits[2].name);
    assert_int_equal(2, ((struct lysc_type_bits*)type)->bits[2].position);
    assert_string_equal("seven", ((struct lysc_type_bits*)type)->bits[3].name);
    assert_int_equal(7, ((struct lysc_type_bits*)type)->bits[3].position);
    assert_string_equal("eight", ((struct lysc_type_bits*)type)->bits[4].name);
    assert_int_equal(8, ((struct lysc_type_bits*)type)->bits[4].position);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1;namespace urn:b;prefix b;feature f; typedef mytype {type bits {"
                                        "bit automin; bit one;bit two; bit seven {position 7;}bit eight;}} leaf l { type mytype {bit eight;bit seven;bit automin;}}}",
                                        LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_BITS, type->basetype);
    assert_non_null(((struct lysc_type_bits*)type)->bits);
    assert_int_equal(3, LY_ARRAY_SIZE(((struct lysc_type_bits*)type)->bits));
    assert_string_equal("automin", ((struct lysc_type_bits*)type)->bits[0].name);
    assert_int_equal(0, ((struct lysc_type_bits*)type)->bits[0].position);
    assert_string_equal("seven", ((struct lysc_type_bits*)type)->bits[1].name);
    assert_int_equal(7, ((struct lysc_type_bits*)type)->bits[1].position);
    assert_string_equal("eight", ((struct lysc_type_bits*)type)->bits[2].name);
    assert_int_equal(8, ((struct lysc_type_bits*)type)->bits[2].position);


    /* invalid cases */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; feature f; leaf l {type bits {"
                                   "bit one {if-feature f;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid keyword \"if-feature\" as a child of \"bit\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit one {position -1;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"-1\" of \"position\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit one {position 4294967296;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"4294967296\" of \"position\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit one; bit one;}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"one\" of bit statement. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit '11';}}}", LYS_IN_YANG));
    logbuf_assert("Invalid identifier first character '1'. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit 'x1$1';}}}", LYS_IN_YANG));
    logbuf_assert("Invalid identifier character '$'. Line number 1.");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; leaf l {type bits;}}", LYS_IN_YANG));
    logbuf_assert("Missing bit substatement for bits type. /bb:l");

    assert_null(lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;typedef mytype {type bits {bit one;}}"
                                             "leaf l {type mytype {bit two;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid bits - derived type adds new item \"two\". /cc:l");

    assert_null(lys_parse_mem(ctx, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;typedef mytype {type bits {bit one;}}"
                                             "leaf l {type mytype {bit one {position 1;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid bits - position of the item \"one\" has changed from 0 to 1 in the derived type. /dd:l");
    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;leaf l {type bits {bit x {position 4294967295;}bit y;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid bits - it is not possible to auto-assign bit position for \"y\" since the highest value is already 4294967295. /ee:l");

    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;leaf l {type bits {bit x {position 1;}bit y {position 1;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid bits - position 1 collide in items \"y\" and \"x\". /ff:l");

    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;typedef mytype {type bits;}"
                                             "leaf l {type mytype {bit one;}}}", LYS_IN_YANG));
    logbuf_assert("Missing bit substatement for bits type mytype. /gg:l");

    assert_null(lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh; typedef mytype {type bits {bit one;}}"
                                        "leaf l {type mytype {bit one;}}}", LYS_IN_YANG));
    logbuf_assert("Bits type can be subtyped only in YANG 1.1 modules. /hh:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_dec64(void **state)
{
    *state = test_type_dec64;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;leaf l {type decimal64 {"
                                        "fraction-digits 2;range min..max;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_DEC64, type->basetype);
    assert_int_equal(2, ((struct lysc_type_dec*)type)->fraction_digits);
    assert_non_null(((struct lysc_type_dec*)type)->range);
    assert_non_null(((struct lysc_type_dec*)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_dec*)type)->range->parts));
    assert_int_equal(INT64_C(-9223372036854775807) - INT64_C(1), ((struct lysc_type_dec*)type)->range->parts[0].min_64);
    assert_int_equal(INT64_C(9223372036854775807), ((struct lysc_type_dec*)type)->range->parts[0].max_64);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;typedef mytype {type decimal64 {"
                                        "fraction-digits 2;range '3.14 | 5.1 | 10';}}leaf l {type mytype;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_DEC64, type->basetype);
    assert_int_equal(2, ((struct lysc_type_dec*)type)->fraction_digits);
    assert_non_null(((struct lysc_type_dec*)type)->range);
    assert_non_null(((struct lysc_type_dec*)type)->range->parts);
    assert_int_equal(3, LY_ARRAY_SIZE(((struct lysc_type_dec*)type)->range->parts));
    assert_int_equal(314, ((struct lysc_type_dec*)type)->range->parts[0].min_64);
    assert_int_equal(314, ((struct lysc_type_dec*)type)->range->parts[0].max_64);
    assert_int_equal(510, ((struct lysc_type_dec*)type)->range->parts[1].min_64);
    assert_int_equal(510, ((struct lysc_type_dec*)type)->range->parts[1].max_64);
    assert_int_equal(1000, ((struct lysc_type_dec*)type)->range->parts[2].min_64);
    assert_int_equal(1000, ((struct lysc_type_dec*)type)->range->parts[2].max_64);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c;typedef mytype {type decimal64 {"
                                        "fraction-digits 2;range '1 .. 65535';}}leaf l {type mytype;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_int_equal(LY_TYPE_DEC64, type->basetype);
    assert_int_equal(2, ((struct lysc_type_dec*)type)->fraction_digits);
    assert_non_null(((struct lysc_type_dec*)type)->range);
    assert_non_null(((struct lysc_type_dec*)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_dec*)type)->range->parts));
    assert_int_equal(100, ((struct lysc_type_dec*)type)->range->parts[0].min_64);
    assert_int_equal(6553500, ((struct lysc_type_dec*)type)->range->parts[0].max_64);

    /* invalid cases */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits 0;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"0\" of \"fraction-digits\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits -1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"-1\" of \"fraction-digits\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits 19;}}}", LYS_IN_YANG));
    logbuf_assert("Value \"19\" is out of \"fraction-digits\" bounds. Line number 1.");

    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64;}}", LYS_IN_YANG));
    logbuf_assert("Missing fraction-digits substatement for decimal64 type. /aa:l");

    assert_null(lys_parse_mem(ctx, "module ab {namespace urn:ab;prefix ab; typedef mytype {type decimal64;}leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Missing fraction-digits substatement for decimal64 type mytype. /ab:l");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; leaf l {type decimal64 {fraction-digits 2;"
                                        "range '3.142';}}}", LYS_IN_YANG));
    logbuf_assert("Range boundary \"3.142\" of decimal64 type exceeds defined number (2) of fraction digits. /bb:l");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc; leaf l {type decimal64 {fraction-digits 2;"
                                        "range '4 | 3.14';}}}", LYS_IN_YANG));
    logbuf_assert("Invalid range restriction - values are not in ascending order (3.14). /cc:l");

    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd; typedef mytype {type decimal64 {fraction-digits 2;}}"
                                        "leaf l {type mytype {fraction-digits 3;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid fraction-digits substatement for type not directly derived from decimal64 built-in type. /dd:l");

    assert_null(lys_parse_mem(ctx, "module de {namespace urn:de;prefix de; typedef mytype {type decimal64 {fraction-digits 2;}}"
                                        "typedef mytype2 {type mytype {fraction-digits 3;}}leaf l {type mytype2;}}", LYS_IN_YANG));
    logbuf_assert("Invalid fraction-digits substatement for type \"mytype2\" not directly derived from decimal64 built-in type. /de:l");

    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:c;prefix c;typedef mytype {type decimal64 {"
                              "fraction-digits 18;range '-10 .. 0';}}leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Invalid range restriction - invalid value \"-10000000000000000000\". /ee:l");
    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:c;prefix c;typedef mytype {type decimal64 {"
                              "fraction-digits 18;range '0 .. 10';}}leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Invalid range restriction - invalid value \"10000000000000000000\". /ee:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_instanceid(void **state)
{
    *state = test_type_instanceid;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;typedef mytype {type instance-identifier {require-instance false;}}"
                                        "leaf l1 {type instance-identifier {require-instance true;}}"
                                        "leaf l2 {type mytype;} leaf l3 {type instance-identifier;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INST, type->basetype);
    assert_int_equal(1, ((struct lysc_type_instanceid*)type)->require_instance);

    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INST, type->basetype);
    assert_int_equal(0, ((struct lysc_type_instanceid*)type)->require_instance);

    type = ((struct lysc_node_leaf*)mod->compiled->data->next->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INST, type->basetype);
    assert_int_equal(1, ((struct lysc_type_instanceid*)type)->require_instance);

    /* invalid cases */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type instance-identifier {require-instance yes;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"yes\" of \"require-instance\". Line number 1.");

    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type instance-identifier {fraction-digits 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid type restrictions for instance-identifier type. /aa:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_identityref(void **state)
{
    *state = test_type_identityref;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1;namespace urn:a;prefix a;identity i; identity j; identity k {base i;}"
                                        "typedef mytype {type identityref {base i;}}"
                                        "leaf l1 {type mytype;} leaf l2 {type identityref {base a:k; base j;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_IDENT, type->basetype);
    assert_non_null(((struct lysc_type_identityref*)type)->bases);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_identityref*)type)->bases));
    assert_string_equal("i", ((struct lysc_type_identityref*)type)->bases[0]->name);

    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_IDENT, type->basetype);
    assert_non_null(((struct lysc_type_identityref*)type)->bases);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_identityref*)type)->bases));
    assert_string_equal("k", ((struct lysc_type_identityref*)type)->bases[0]->name);
    assert_string_equal("j", ((struct lysc_type_identityref*)type)->bases[1]->name);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1;namespace urn:b;prefix b;import a {prefix a;}"
                                        "leaf l {type identityref {base a:k; base a:j;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_IDENT, type->basetype);
    assert_non_null(((struct lysc_type_identityref*)type)->bases);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_identityref*)type)->bases));
    assert_string_equal("k", ((struct lysc_type_identityref*)type)->bases[0]->name);
    assert_string_equal("j", ((struct lysc_type_identityref*)type)->bases[1]->name);

    /* invalid cases */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type identityref;}}", LYS_IN_YANG));
    logbuf_assert("Missing base substatement for identityref type. /aa:l");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; typedef mytype {type identityref;}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Missing base substatement for identityref type mytype. /bb:l");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc; identity i; typedef mytype {type identityref {base i;}}"
                                        "leaf l {type mytype {base i;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid base substatement for the type not directly derived from identityref built-in type. /cc:l");

    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd; identity i; typedef mytype {type identityref {base i;}}"
                                        "typedef mytype2 {type mytype {base i;}}leaf l {type mytype2;}}", LYS_IN_YANG));
    logbuf_assert("Invalid base substatement for the type \"mytype2\" not directly derived from identityref built-in type. /dd:l");

    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee; identity i; identity j;"
                                        "leaf l {type identityref {base i;base j;}}}", LYS_IN_YANG));
    logbuf_assert("Multiple bases in identityref type are allowed only in YANG 1.1 modules. /ee:l");

    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff; identity i;leaf l {type identityref {base j;}}}", LYS_IN_YANG));
    logbuf_assert("Unable to find base (j) of identityref. /ff:l");

    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;leaf l {type identityref {base x:j;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid prefix used for base (x:j) of identityref. /gg:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_leafref(void **state)
{
    *state = test_type_leafref;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;
    const char *path, *name, *prefix;
    size_t prefix_len, name_len;
    int parent_times, has_predicate;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    /* lys_path_token() */
    path = "invalid_path";
    parent_times = 0;
    assert_int_equal(LY_EINVAL, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    path = "..";
    parent_times = 0;
    assert_int_equal(LY_EINVAL, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    path = "..[";
    parent_times = 0;
    assert_int_equal(LY_EINVAL, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    path = "../";
    parent_times = 0;
    assert_int_equal(LY_EINVAL, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    path = "/";
    parent_times = 0;
    assert_int_equal(LY_EINVAL, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));

    path = "../../pref:id/xxx[predicate]/invalid!!!";
    parent_times = 0;
    assert_int_equal(LY_SUCCESS, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    assert_string_equal("/xxx[predicate]/invalid!!!", path);
    assert_int_equal(4, prefix_len);
    assert_int_equal(0, strncmp("pref", prefix, prefix_len));
    assert_int_equal(2, name_len);
    assert_int_equal(0, strncmp("id", name, name_len));
    assert_int_equal(2, parent_times);
    assert_int_equal(0, has_predicate);
    assert_int_equal(LY_SUCCESS, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    assert_string_equal("[predicate]/invalid!!!", path);
    assert_int_equal(0, prefix_len);
    assert_null(prefix);
    assert_int_equal(3, name_len);
    assert_int_equal(0, strncmp("xxx", name, name_len));
    assert_int_equal(1, has_predicate);
    path += 11;
    assert_int_equal(LY_EINVAL, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    assert_string_equal("!!!", path);
    assert_int_equal(0, prefix_len);
    assert_null(prefix);
    assert_int_equal(7, name_len);
    assert_int_equal(0, strncmp("invalid", name, name_len));

    path = "/absolute/prefix:path";
    parent_times = 0;
    assert_int_equal(LY_SUCCESS, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    assert_string_equal("/prefix:path", path);
    assert_int_equal(0, prefix_len);
    assert_null(prefix);
    assert_int_equal(8, name_len);
    assert_int_equal(0, strncmp("absolute", name, name_len));
    assert_int_equal(-1, parent_times);
    assert_int_equal(0, has_predicate);
    assert_int_equal(LY_SUCCESS, lys_path_token(&path, &prefix, &prefix_len, &name, &name_len, &parent_times, &has_predicate));
    assert_int_equal(0, *path);
    assert_int_equal(6, prefix_len);
    assert_int_equal(0, strncmp("prefix", prefix, prefix_len));
    assert_int_equal(4, name_len);
    assert_int_equal(0, strncmp("path", name, name_len));
    assert_int_equal(0, has_predicate);

    /* complete leafref paths */
    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1;namespace urn:a;prefix a;"
                                        "leaf ref1 {type leafref {path /a:target1;}} leaf ref2 {type leafref {path /a/target2; require-instance false;}}"
                                        "leaf target1 {type string;}container a {leaf target2 {type uint8;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/a:target1", ((struct lysc_type_leafref*)type)->path);
    assert_ptr_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref*)type)->realtype->basetype);
    assert_int_equal(1, ((struct lysc_type_leafref*)type)->require_instance);
    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/a/target2", ((struct lysc_type_leafref*)type)->path);
    assert_ptr_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_UINT8, ((struct lysc_type_leafref*)type)->realtype->basetype);
    assert_int_equal(0, ((struct lysc_type_leafref*)type)->require_instance);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b; typedef mytype {type leafref {path /b:target;}}"
                                        "typedef mytype2 {type mytype;} typedef mytype3 {type leafref {path /target;}} leaf ref {type mytype2;}"
                                        "leaf target {type leafref {path ../realtarget;}} leaf realtarget {type string;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/b:target", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref*)type)->realtype->basetype);
    assert_int_equal(1, ((struct lysc_type_leafref* )type)->require_instance);

    /* prefixes are reversed to check using correct context of the path! */
    assert_non_null(mod = lys_parse_mem(ctx, "module c {yang-version 1.1;namespace urn:c;prefix b; import b {prefix c;}"
                                        "typedef mytype3 {type c:mytype {require-instance false;}}"
                                        "leaf ref1 {type b:mytype3;}leaf ref2 {type c:mytype2;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/b:target", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_not_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref*)type)->realtype->basetype);
    assert_int_equal(0, ((struct lysc_type_leafref* )type)->require_instance);
    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/b:target", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_not_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_int_equal(1, ((struct lysc_type_leafref* )type)->require_instance);

    /* non-prefixed nodes in path are supposed to be from the module where the leafref type is instantiated */
    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d; import b {prefix b;}"
                                        "leaf ref {type b:mytype3;}leaf target {type int8;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/target", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_not_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref*)type)->realtype->basetype);
    assert_int_equal(1, ((struct lysc_type_leafref* )type)->require_instance);

    /* conditional leafrefs */
    assert_non_null(mod = lys_parse_mem(ctx, "module e {yang-version 1.1;namespace urn:e;prefix e;feature f1; feature f2;"
                                        "leaf ref1 {if-feature 'f1 and f2';type leafref {path /target;}}"
                                        "leaf target {if-feature f1; type boolean;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/target", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_BOOL, ((struct lysc_type_leafref*)type)->realtype->basetype);

    assert_non_null(mod = lys_parse_mem(ctx, "module f {namespace urn:f;prefix f;"
                                        "list interface{key name;leaf name{type string;}list address {key ip;leaf ip {type string;}}}"
                                        "container default-address{leaf ifname{type leafref{ path \"../../interface/name\";}}"
                                          "leaf address {type leafref{ path \"../../interface[  name = current()/../ifname ]/address/ip\";}}}}",
                                        LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)(*lysc_node_children_p(mod->compiled->data->prev, 0))->prev)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("../../interface[  name = current()/../ifname ]/address/ip", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref*)type)->realtype->basetype);

    assert_non_null(mod = lys_parse_mem(ctx, "module g {namespace urn:g;prefix g;"
                                        "leaf source {type leafref {path \"/endpoint-parent[id=current()/../field]/endpoint/name\";}}"
                                        "leaf field {type int32;}list endpoint-parent {key id;leaf id {type int32;}"
                                        "list endpoint {key name;leaf name {type string;}}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/endpoint-parent[id=current()/../field]/endpoint/name", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref*)type)->realtype->basetype);

    /* leafref to imported (not yet implemented) module */
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module h {namespace urn:h;prefix h; leaf h  {type uint16;}}");
    assert_non_null(mod = lys_parse_mem(ctx, "module i {namespace urn:i;prefix i;import h {prefix h;}"
                                        "leaf i {type leafref {path /h:h;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_UINT16, ((struct lysc_type_leafref*)type)->realtype->basetype);
    assert_non_null(mod = ly_ctx_get_module_implemented(ctx, "h"));
    assert_int_equal(1, mod->implemented);
    assert_non_null(mod->compiled->data);
    assert_string_equal("h", mod->compiled->data->name);

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module j {namespace urn:j;prefix j; leaf j  {type string;}}");
    assert_non_null(mod = lys_parse_mem(ctx, "module k {namespace urn:k;prefix k;import j {prefix j;}"
                                        "leaf i {type leafref {path \"/ilist[name = current()/../j:j]/value\";}}"
                                        "list ilist {key name; leaf name {type string;} leaf value {type uint16;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_UINT16, ((struct lysc_type_leafref*)type)->realtype->basetype);
    assert_non_null(mod = ly_ctx_get_module_implemented(ctx, "j"));
    assert_int_equal(1, mod->implemented);
    assert_non_null(mod->compiled->data);
    assert_string_equal("j", mod->compiled->data->name);

    /* invalid paths */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path ../a/invalid;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path - unable to find \"../a/invalid\". /aa:ref1");
    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path ../../toohigh;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path \"../../toohigh\" - too many \"..\" in the path. /bb:ref1");
    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path /a:invalid;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path - unable to find module connected with the prefix of the node \"/a:invalid\". /cc:ref1");
    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;leaf target1 {type string;}container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path '/a[target2 = current()/../target1]/target2';}}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path - node \"/a\" is expected to be a list, but it is container. /dd:ref1");
    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path /a!invalid;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path at character 3 (/a!invalid). /ee:ref1");
    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path /a;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path \"/a\" - target node is container instead of leaf or leaf-list. /ff:ref1");
    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;container a {leaf target2 {type uint8; status deprecated;}}"
                                        "leaf ref1 {type leafref {path /a/target2;}}}", LYS_IN_YANG));
    logbuf_assert("A current definition \"ref1\" is not allowed to reference deprecated definition \"target2\". /gg:ref1");
    assert_null(lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;"
                                        "leaf ref1 {type leafref;}}", LYS_IN_YANG));
    logbuf_assert("Missing path substatement for leafref type. /hh:ref1");
    assert_null(lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;typedef mytype {type leafref;}"
                                        "leaf ref1 {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Missing path substatement for leafref type mytype. /ii:ref1");
    assert_null(lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;feature f;"
                                        "leaf ref {type leafref {path /target;}}leaf target {if-feature f;type string;}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path \"/target\" - set of features applicable to the leafref target is not a subset of features "
                  "applicable to the leafref itself. /jj:ref");
    assert_null(lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;"
                                        "leaf ref {type leafref {path /target;}}leaf target {type string;config false;}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path \"/target\" - target is supposed to represent configuration data (as the leafref does), but it does not. /kk:ref");

    assert_null(lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;"
                                        "leaf ref {type leafref {path /target; require-instance true;}}leaf target {type string;}}", LYS_IN_YANG));
    logbuf_assert("Leafref type can be restricted by require-instance statement only in YANG 1.1 modules. /ll:ref");
    assert_null(lys_parse_mem(ctx, "module mm {namespace urn:mm;prefix mm;typedef mytype {type leafref {path /target;require-instance false;}}"
                                        "leaf ref {type mytype;}leaf target {type string;}}", LYS_IN_YANG));
    logbuf_assert("Leafref type \"mytype\" can be restricted by require-instance statement only in YANG 1.1 modules. /mm:ref");

    assert_null(lys_parse_mem(ctx, "module nn {namespace urn:nn;prefix nn;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name is current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name i\" - missing \"=\" after node-identifier. /nn:address");

    assert_null(lys_parse_mem(ctx, "module oo {namespace urn:oo;prefix oo;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../ifname/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../ifname/ip\" - missing predicate termination. /oo:address");

    assert_null(lys_parse_mem(ctx, "module pp {namespace urn:pp;prefix pp;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[x:name=current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[x:name=current()/../ifname]\" - prefix \"x\" not defined in module \"pp\". /pp:address");

    assert_null(lys_parse_mem(ctx, "module qq {namespace urn:qq;prefix qq;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[id=current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[id=current()/../ifname]\" - predicate's key node \"id\" not found. /qq:address");

    assert_null(lys_parse_mem(ctx, "module rr {namespace urn:rr;prefix rr;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name=current() /  .. / ifname][name=current()/../test]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../test]\" - multiple equality tests for the key \"name\". /rr:address");

    assert_null(lys_parse_mem(ctx, "module ss {namespace urn:ss;prefix ss;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = ../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name = ../ifname]\" - missing current-function-invocation. /ss:address");

    assert_null(lys_parse_mem(ctx, "module tt {namespace urn:tt;prefix tt;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name = current()../ifname]\" - missing \"/\" after current-function-invocation. /tt:address");

    assert_null(lys_parse_mem(ctx, "module uu {namespace urn:uu;prefix uu;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/..ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name = current()/..ifname]\" - missing \"/\" in \"../\" rel-path-keyexpr pattern. /uu:address");

    assert_null(lys_parse_mem(ctx, "module vv {namespace urn:vv;prefix vv;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name = current()/ifname]\" - at least one \"..\" is expected in rel-path-keyexpr. /vv:address");

    assert_null(lys_parse_mem(ctx, "module ww {namespace urn:ww;prefix ww;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/../]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name = current()/../]\" - at least one node-identifier is expected in rel-path-keyexpr. /ww:address");

    assert_null(lys_parse_mem(ctx, "module xx {namespace urn:xx;prefix xx;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/../$node]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid node identifier in leafref path predicate - character 22 (of [name = current()/../$node]). /xx:address");

    assert_null(lys_parse_mem(ctx, "module yy {namespace urn:yy;prefix yy;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../x:ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../x:ifname]\" - unable to find module of the node \"ifname\" in rel-path-keyexpr. /yy:address");

    assert_null(lys_parse_mem(ctx, "module zz {namespace urn:zz;prefix zz;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../xxx]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../xxx]\" - unable to find node \"current()/../xxx\" in the rel-path-keyexpr. /zz:address");

    assert_null(lys_parse_mem(ctx, "module zza {namespace urn:zza;prefix zza;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}container c;"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../c]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../c]\" - rel-path-keyexpr \"current()/../c\" refers container instead of leaf. /zza:address");

    assert_null(lys_parse_mem(ctx, "module zzb {namespace urn:zzb;prefix zzb;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}container c;}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[c=current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    logbuf_assert("Invalid leafref path predicate \"[c=current()/../ifname]\" - predicate's key node \"c\" not found. /zzb:address");

    /* circular chain */
    assert_null(lys_parse_mem(ctx, "module aaa {namespace urn:aaa;prefix aaa;"
                                        "leaf ref1 {type leafref {path /ref2;}}"
                                        "leaf ref2 {type leafref {path /ref3;}}"
                                        "leaf ref3 {type leafref {path /ref4;}}"
                                        "leaf ref4 {type leafref {path /ref1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid leafref path \"/ref1\" - circular chain of leafrefs detected. /aaa:ref4");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_empty(void **state)
{
    *state = test_type_empty;

    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                                        "leaf l {type empty; default x;}}", LYS_IN_YANG));
    logbuf_assert("Invalid leaf's default value \"x\" which does not fit the type (Invalid empty value \"x\".). /aa:l");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;typedef mytype {type empty; default x;}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Invalid type \"mytype\" - \"empty\" type must not have a default value (x). /bb:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_union(void **state)
{
    *state = test_type_union;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1;namespace urn:a;prefix a; typedef mybasetype {type string;}"
                                        "typedef mytype {type union {type int8; type mybasetype;}}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(2, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union*)type)->types);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_union*)type)->types));
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_union*)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union*)type)->types[1]->basetype);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1;namespace urn:b;prefix b; typedef mybasetype {type string;}"
                                        "typedef mytype {type union {type int8; type mybasetype;}}"
                                        "leaf l {type union {type decimal64 {fraction-digits 2;} type mytype;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union*)type)->types);
    assert_int_equal(3, LY_ARRAY_SIZE(((struct lysc_type_union*)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union*)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_union*)type)->types[1]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union*)type)->types[2]->basetype);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {yang-version 1.1;namespace urn:c;prefix c; typedef mybasetype {type string;}"
                                        "typedef mytype {type union {type leafref {path ../target;} type mybasetype;}}"
                                        "leaf l {type union {type decimal64 {fraction-digits 2;} type mytype;}}"
                                        "leaf target {type leafref {path ../realtarget;}} leaf realtarget {type int8;}}",
                                        LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union*)type)->types);
    assert_int_equal(3, LY_ARRAY_SIZE(((struct lysc_type_union*)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union*)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_LEAFREF, ((struct lysc_type_union*)type)->types[1]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union*)type)->types[2]->basetype);
    assert_non_null(((struct lysc_type_leafref*)((struct lysc_type_union*)type)->types[1])->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref*)((struct lysc_type_union*)type)->types[1])->realtype->basetype);

    /* invalid unions */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;typedef mytype {type union;}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Missing type substatement for union type mytype. /aa:l");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;leaf l {type union;}}", LYS_IN_YANG));
   logbuf_assert("Missing type substatement for union type. /bb:l");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;typedef mytype {type union{type int8; type string;}}"
                                        "leaf l {type mytype {type string;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid type substatement for the type not directly derived from union built-in type. /cc:l");

    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;typedef mytype {type union{type int8; type string;}}"
                                        "typedef mytype2 {type mytype {type string;}}leaf l {type mytype2;}}", LYS_IN_YANG));
    logbuf_assert("Invalid type substatement for the type \"mytype2\" not directly derived from union built-in type. /dd:l");

    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;typedef mytype {type union{type mytype; type string;}}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Invalid \"mytype\" type reference - circular chain of types detected. /ee:l");
    assert_null(lys_parse_mem(ctx, "module ef {namespace urn:ef;prefix ef;typedef mytype {type mytype2;}"
                                        "typedef mytype2 {type mytype;} leaf l {type mytype;}}", LYS_IN_YANG));
    logbuf_assert("Invalid \"mytype\" type reference - circular chain of types detected. /ef:l");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_dflt(void **state)
{
    *state = test_type_union;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;
    struct lysc_node_leaf *leaf;
    int dynamic;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    /* default is not inherited from union's types */
    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a; typedef mybasetype {type string;default hello;units xxx;}"
                                        "leaf l {type union {type decimal64 {fraction-digits 2;} type mybasetype;}}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union*)type)->types);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_union*)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union*)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union*)type)->types[1]->basetype);
    assert_null(((struct lysc_node_leaf*)mod->compiled->data)->dflt);
    assert_null(((struct lysc_node_leaf*)mod->compiled->data)->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b; typedef mybasetype {type string;default hello;units xxx;}"
                                        "leaf l {type mybasetype;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount); /* 2x type reference, 1x default value's reference (typedf's default does not reference own type)*/
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_non_null(leaf = (struct lysc_node_leaf*)mod->compiled->data);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("xxx", leaf->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c; typedef mybasetype {type string;default hello;units xxx;}"
                                        "leaf l {type mybasetype; default goodbye;units yyy;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount); /* 2x type reference, 1x default value's reference */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf*)mod->compiled->data;
    assert_string_equal("goodbye", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("yyy", leaf->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d; typedef mybasetype {type string;default hello;units xxx;}"
                                        "typedef mytype {type mybasetype;}leaf l1 {type mytype; default goodbye;units yyy;}"
                                        "leaf l2 {type mytype;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(6, type->refcount); /* 4x type reference, 2x default value's reference (1 shared compiled type of typedefs which default does not reference own type) */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf*)mod->compiled->data;
    assert_string_equal("goodbye", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("yyy", leaf->units);
    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(6, type->refcount); /* 4x type reference, 2x default value's reference (1 shared compiled type of typedefs which default does not reference own type) */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf*)mod->compiled->data->next;
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("xxx", leaf->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module e {namespace urn:e;prefix e; typedef mybasetype {type string;}"
                                        "typedef mytype {type mybasetype; default hello;units xxx;}leaf l {type mytype;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(4, type->refcount); /* 3x type reference, 1x default value's reference (typedef's default does not reference own type) */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf*)mod->compiled->data;
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("xxx", leaf->units);

    /* mandatory leaf does not takes default value from type */
    assert_non_null(mod = lys_parse_mem(ctx, "module f {namespace urn:f;prefix f;typedef mytype {type string; default hello;units xxx;}"
                                        "leaf l {type mytype; mandatory true;}}", LYS_IN_YANG));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_null(((struct lysc_node_leaf*)mod->compiled->data)->dflt);
    assert_string_equal("xxx", ((struct lysc_node_leaf*)mod->compiled->data)->units);

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_status(void **state)
{
    *state = test_status;

    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                                        "container c {status deprecated; leaf l {status current; type string;}}}", LYS_IN_YANG));
    logbuf_assert("A \"current\" status is in conflict with the parent's \"deprecated\" status. /aa:c/l");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;"
                                        "container c {status obsolete; leaf l {status current; type string;}}}", LYS_IN_YANG));
    logbuf_assert("A \"current\" status is in conflict with the parent's \"obsolete\" status. /bb:c/l");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;"
                                        "container c {status obsolete; leaf l {status deprecated; type string;}}}", LYS_IN_YANG));
    logbuf_assert("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status. /cc:c/l");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:dd;prefix d;"
                                        "container c {leaf l {status obsolete; type string;}}"
                                        "container d {leaf m {when \"../../c/l\"; type string;}}}", LYS_IN_YANG));
    logbuf_assert("A current definition \"m\" is not allowed to reference obsolete definition \"l\". /cc:d/m");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_grouping(void **state)
{
    *state = test_grouping;

    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    /* result ok, but a warning about not used locally scoped grouping printed */
    assert_non_null(lys_parse_mem(ctx, "module a {namespace urn:a;prefix a; grouping grp1 {leaf a1 {type string;}}"
                                  "container a {leaf x {type string;} grouping grp2 {leaf a2 {type string;}}}}", LYS_IN_YANG));
    logbuf_assert("Locally scoped grouping \"grp2\" not used.");
    logbuf_clean();

    /* result ok - when statement or leafref target must be checked only at the place where the grouping is really instantiated */
    assert_non_null(lys_parse_mem(ctx, "module b {namespace urn:b;prefix b; grouping grp {"
                                  "leaf ref {type leafref {path \"../name\";}}"
                                  "leaf cond {type string; when \"../name = 'specialone'\";}}}", LYS_IN_YANG));
    logbuf_assert("");


    /* invalid - error in a non-instantiated grouping */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                                        "grouping grp {leaf x {type leafref;}}}", LYS_IN_YANG));
    logbuf_assert("Missing path substatement for leafref type. /aa:{grouping='grp'}/x");
    logbuf_clean();
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                                        "container a {grouping grp {leaf x {type leafref;}}}}", LYS_IN_YANG));
    logbuf_assert("Missing path substatement for leafref type. /aa:a/{grouping='grp'}/x");


    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_uses(void **state)
{
    *state = test_uses;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    const struct lysc_node *parent, *child;
    const struct lysc_node_container *cont;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module grp {namespace urn:grp;prefix g; typedef mytype {type string;} feature f;"
                              "grouping grp {leaf x {type mytype;} leaf y {type string; if-feature f;}}}");
    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;import grp {prefix g;}"
                                        "grouping grp_a_top {leaf a1 {type int8;}}"
                                        "container a {uses grp_a; uses grp_a_top; uses g:grp; grouping grp_a {leaf a2 {type uint8;}}}}", LYS_IN_YANG));
    assert_non_null((parent = mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, parent->nodetype);
    assert_non_null((child = ((struct lysc_node_container*)parent)->child));
    assert_string_equal("a2", child->name);
    assert_ptr_equal(mod, child->module);
    assert_non_null((child = child->next));
    assert_string_equal("a1", child->name);
    assert_ptr_equal(mod, child->module);
    assert_non_null((child = child->next));
    assert_string_equal("x", child->name);
    assert_ptr_equal(mod, child->module);
    assert_non_null((child = child->next));
    assert_string_equal("y", child->name);
    assert_non_null(child->iffeatures);
    assert_int_equal(1, LY_ARRAY_SIZE(child->iffeatures));
    assert_int_equal(1, LY_ARRAY_SIZE(child->iffeatures[0].features));
    assert_string_equal("f", child->iffeatures[0].features[0]->name);
    assert_int_equal(LY_EINVAL, lys_feature_enable(mod->compiled->imports[0].module, "f"));
    logbuf_assert("Module \"grp\" is not implemented so all its features are permanently disabled without a chance to change it.");
    assert_int_equal(0, lysc_iffeature_value(&child->iffeatures[0]));

    /* make the imported module implemented and enable the feature */
    assert_non_null(mod = ly_ctx_get_module(ctx, "grp", NULL));
    assert_int_equal(LY_SUCCESS, lys_set_implemented(mod));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "f"));
    assert_string_equal("f", child->iffeatures[0].features[0]->name);
    assert_int_equal(1, lysc_iffeature_value(&child->iffeatures[0]));

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule bsub {belongs-to b {prefix b;} grouping grp {leaf b {when 1; type string;} leaf c {type string;}}}");
    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;include bsub;uses grp {when 2;}}", LYS_IN_YANG));
    assert_non_null(mod->compiled->data);
    assert_int_equal(LYS_LEAF, mod->compiled->data->nodetype);
    assert_string_equal("b", mod->compiled->data->name);
    assert_int_equal(2, LY_ARRAY_SIZE(mod->compiled->data->when));
    assert_int_equal(1, mod->compiled->data->when[0]->refcount);
    assert_non_null(mod->compiled->data->when[0]->context);
    assert_string_equal("b", mod->compiled->data->when[0]->context->name);
    assert_int_equal(2, mod->compiled->data->when[1]->refcount);
    assert_null(mod->compiled->data->when[1]->context);

    assert_int_equal(LYS_LEAF, mod->compiled->data->next->nodetype);
    assert_string_equal("c", mod->compiled->data->next->name);
    assert_int_equal(1, LY_ARRAY_SIZE(mod->compiled->data->next->when));
    assert_int_equal(2, mod->compiled->data->next->when[0]->refcount);
    assert_null(mod->compiled->data->next->when[0]->context);

    logbuf_clean();
    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:ii;prefix ii;"
                                        "grouping grp {leaf l {type string;}leaf k {type string; status obsolete;}}"
                                        "uses grp {status deprecated;}}", LYS_IN_YANG));
    assert_int_equal(LYS_LEAF, mod->compiled->data->nodetype);
    assert_string_equal("l", mod->compiled->data->name);
    assert_true(LYS_STATUS_DEPRC & mod->compiled->data->flags);
    assert_int_equal(LYS_LEAF, mod->compiled->data->next->nodetype);
    assert_string_equal("k", mod->compiled->data->next->name);
    assert_true(LYS_STATUS_OBSLT & mod->compiled->data->next->flags);
    logbuf_assert(""); /* no warning about inheriting deprecated flag from uses */

    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d; grouping grp {container g;}"
                                        "container top {uses grp {augment g {leaf x {type int8;}}}}}", LYS_IN_YANG));
    assert_non_null(mod->compiled->data);
    assert_non_null(child = lysc_node_children(mod->compiled->data, 0));
    assert_string_equal("g", child->name);
    assert_non_null(child = lysc_node_children(child, 0));
    assert_string_equal("x", child->name);

    assert_non_null(mod = lys_parse_mem(ctx, "module e {yang-version 1.1;namespace urn:e;prefix e; grouping grp {action g { description \"super g\";}}"
                                        "container top {action e; uses grp {refine g {description \"ultra g\";}}}}", LYS_IN_YANG));
    assert_non_null(mod->compiled->data);
    cont = (const struct lysc_node_container*)mod->compiled->data;
    assert_non_null(cont->actions);
    assert_int_equal(2, LY_ARRAY_SIZE(cont->actions));
    assert_string_equal("e", cont->actions[1].name);
    assert_string_equal("g", cont->actions[0].name);
    assert_string_equal("ultra g", cont->actions[0].dsc);

    assert_non_null(mod = lys_parse_mem(ctx, "module f {yang-version 1.1;namespace urn:f;prefix f; grouping grp {notification g { description \"super g\";}}"
                                        "container top {notification f; uses grp {refine g {description \"ultra g\";}}}}", LYS_IN_YANG));
    assert_non_null(mod->compiled->data);
    cont = (const struct lysc_node_container*)mod->compiled->data;
    assert_non_null(cont->notifs);
    assert_int_equal(2, LY_ARRAY_SIZE(cont->notifs));
    assert_string_equal("f", cont->notifs[1].name);
    assert_string_equal("g", cont->notifs[0].name);
    assert_string_equal("ultra g", cont->notifs[0].dsc);

    /* empty grouping */
    assert_non_null(mod = lys_parse_mem(ctx, "module g {namespace urn:g;prefix g; grouping grp; uses grp;}", LYS_IN_YANG));
    assert_null(mod->compiled->data);

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;uses missinggrp;}", LYS_IN_YANG));
    logbuf_assert("Grouping \"missinggrp\" referenced by a uses statement not found. /aa:{uses='missinggrp'}");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;uses grp;"
                                        "grouping grp {leaf a{type string;}uses grp1;}"
                                        "grouping grp1 {leaf b {type string;}uses grp2;}"
                                        "grouping grp2 {leaf c {type string;}uses grp;}}", LYS_IN_YANG));
    logbuf_assert("Grouping \"grp\" references itself through a uses statement. /bb:{uses='grp'}/{uses='grp1'}/{uses='grp2'}/{uses='grp'}");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;uses a:missingprefix;}", LYS_IN_YANG));
    logbuf_assert("Invalid prefix used for grouping reference. /cc:{uses='a:missingprefix'}");

    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;grouping grp{leaf a{type string;}}"
                                        "leaf a {type string;}uses grp;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"a\" of data definition/RPC/action/Notification statement. /dd:{uses='grp'}/dd:a");

    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;grouping grp {leaf l {type string; status deprecated;}}"
                                        "uses grp {status obsolete;}}", LYS_IN_YANG));
    logbuf_assert("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status. /ee:{uses='grp'}/ee:l");

    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;grouping grp {leaf l {type string;}}"
                                        "leaf l {type int8;}uses grp;}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"l\" of data definition/RPC/action/Notification statement. /ff:{uses='grp'}/ff:l");
    assert_null(lys_parse_mem(ctx, "module fg {namespace urn:fg;prefix fg;grouping grp {leaf m {type string;}}"
                                        "uses grp;leaf m {type int8;}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"m\" of data definition/RPC/action/Notification statement. /fg:m");


    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg; grouping grp {container g;}"
                              "leaf g {type string;}"
                              "container top {uses grp {augment /g {leaf x {type int8;}}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid descendant-schema-nodeid value \"/g\" - absolute-schema-nodeid used. /gg:top/{uses='grp'}/{augment='/g'}");

    assert_non_null(mod = lys_parse_mem(ctx, "module hh {yang-version 1.1;namespace urn:hh;prefix hh;"
                                        "grouping grp {notification g { description \"super g\";}}"
                                        "container top {notification h; uses grp {refine h {description \"ultra h\";}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid descendant-schema-nodeid value \"h\" - target node not found. /hh:top/{uses='grp'}/{refine='h'}");

    assert_non_null(mod = lys_parse_mem(ctx, "module ii {yang-version 1.1;namespace urn:ii;prefix ii;"
                                        "grouping grp {action g { description \"super g\";}}"
                                        "container top {action i; uses grp {refine i {description \"ultra i\";}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid descendant-schema-nodeid value \"i\" - target node not found. /ii:top/{uses='grp'}/{refine='i'}");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_refine(void **state)
{
    *state = test_refine;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_node *parent, *child;
    struct lysc_node_leaf *leaf;
    struct lysc_node_leaflist *llist;
    int dynamic;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(lys_parse_mem(ctx, "module grp {yang-version 1.1;namespace urn:grp;prefix g; feature f;typedef mytype {type string; default cheers!;}"
                                       "grouping grp {container c {leaf l {type mytype; default goodbye;}"
                                       "leaf-list ll {type mytype; default goodbye; max-elements 6;}"
                                       "choice ch {default a; leaf a {type int8;}leaf b{type uint8;}}"
                                       "leaf x {type mytype; mandatory true; must 1;}"
                                       "anydata a {mandatory false; if-feature f; description original; reference original;}"
                                       "container c {config false; leaf l {type string;}}}}}", LYS_IN_YANG));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1;namespace urn:a;prefix a;import grp {prefix g;}feature fa;"
                                        "uses g:grp {refine c/l {default hello; config false;}"
                                        "refine c/ll {default hello;default world; min-elements 2; max-elements 5;}"
                                        "refine c/ch {default b;config true; if-feature fa;}"
                                        "refine c/x {mandatory false; must ../ll;description refined; reference refined;}"
                                        "refine c/a {mandatory true; must 1; if-feature fa;description refined; reference refined;}"
                                        "refine c/c {config true;presence indispensable;}}}", LYS_IN_YANG));
    assert_non_null((parent = mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, parent->nodetype);
    assert_string_equal("c", parent->name);
    assert_non_null((leaf = (struct lysc_node_leaf*)((struct lysc_node_container*)parent)->child));
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("l", leaf->name);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(LYS_CONFIG_R, leaf->flags & LYS_CONFIG_MASK);
    assert_non_null(llist = (struct lysc_node_leaflist*)leaf->next);
    assert_int_equal(LYS_LEAFLIST, llist->nodetype);
    assert_string_equal("ll", llist->name);
    assert_int_equal(2, LY_ARRAY_SIZE(llist->dflts));
    assert_string_equal("hello", llist->dflts[0]->realtype->plugin->print(llist->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("world", llist->dflts[1]->realtype->plugin->print(llist->dflts[1], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(2, llist->min);
    assert_int_equal(5, llist->max);
    assert_non_null(child = llist->next);
    assert_int_equal(LYS_CHOICE, child->nodetype);
    assert_string_equal("ch", child->name);
    assert_string_equal("b", ((struct lysc_node_choice*)child)->dflt->name);
    assert_true(LYS_SET_DFLT & ((struct lysc_node_choice*)child)->dflt->flags);
    assert_false(LYS_SET_DFLT & ((struct lysc_node_choice*)child)->cases[0].flags);
    assert_non_null(child->iffeatures);
    assert_int_equal(1, LY_ARRAY_SIZE(child->iffeatures));
    assert_non_null(leaf = (struct lysc_node_leaf*)child->next);
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("x", leaf->name);
    assert_false(LYS_MAND_TRUE & leaf->flags);
    assert_string_equal("cheers!", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_non_null(leaf->musts);
    assert_int_equal(2, LY_ARRAY_SIZE(leaf->musts));
    assert_string_equal("refined", leaf->dsc);
    assert_string_equal("refined", leaf->ref);
    assert_non_null(child = leaf->next);
    assert_int_equal(LYS_ANYDATA, child->nodetype);
    assert_string_equal("a", child->name);
    assert_true(LYS_MAND_TRUE & child->flags);
    assert_non_null(((struct lysc_node_anydata*)child)->musts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_node_anydata*)child)->musts));
    assert_non_null(child->iffeatures);
    assert_int_equal(2, LY_ARRAY_SIZE(child->iffeatures));
    assert_string_equal("refined", child->dsc);
    assert_string_equal("refined", child->ref);
    assert_non_null(child = child->next);
    assert_int_equal(LYS_CONTAINER, child->nodetype);
    assert_string_equal("c", child->name);
    assert_true(LYS_PRESENCE & child->flags);
    assert_true(LYS_CONFIG_W & child->flags);
    assert_true(LYS_CONFIG_W & ((struct lysc_node_container*)child)->child->flags);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1;namespace urn:b;prefix b;import grp {prefix g;}"
                                        "uses g:grp {status deprecated; refine c/x {default hello; mandatory false;}}}", LYS_IN_YANG));
    assert_non_null((leaf = (struct lysc_node_leaf*)((struct lysc_node_container*)mod->compiled->data)->child->prev->prev->prev));
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("x", leaf->name);
    assert_false(LYS_MAND_TRUE & leaf->flags);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);

    /* invalid */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;import grp {prefix g;}"
                                        "uses g:grp {refine c {default hello;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of default - container cannot hold default value(s). /aa:{uses='g:grp'}/{refine='c'}");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;import grp {prefix g;}"
                                        "uses g:grp {refine c/l {default hello; default world;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of default - leaf cannot hold 2 default values. /bb:{uses='g:grp'}/{refine='c/l'}");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;import grp {prefix g;}"
                                        "uses g:grp {refine c/ll {default hello; default world;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of default in leaf-list - the default statement is allowed only in YANG 1.1 modules. /cc:{uses='g:grp'}/{refine='c/ll'}");

    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;import grp {prefix g;}"
                                        "uses g:grp {refine c/ll {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of mandatory - leaf-list cannot hold mandatory statement. /dd:{uses='g:grp'}/{refine='c/ll'}");

    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;import grp {prefix g;}"
                                        "uses g:grp {refine c/l {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of mandatory - leaf already has \"default\" statement. /ee:{uses='g:grp'}/{refine='c/l'}");
    assert_null(lys_parse_mem(ctx, "module ef {namespace urn:ef;prefix ef;import grp {prefix g;}"
                                        "uses g:grp {refine c/ch {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of mandatory - choice already has \"default\" statement. /ef:{uses='g:grp'}/{refine='c/ch'}");

    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;import grp {prefix g;}"
                                        "uses g:grp {refine c/ch/a/a {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of mandatory under the default case. /ff:{uses='g:grp'}/{refine='c/ch/a/a'}");

    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;import grp {prefix g;}"
                                        "uses g:grp {refine c/x {default hello;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of default - the node is mandatory. /gg:{uses='g:grp'}/{refine='c/x'}");

    assert_null(lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;import grp {prefix g;}"
                                        "uses g:grp {refine c/c/l {config true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of config - configuration node cannot be child of any state data node. /hh:{uses='g:grp'}/{refine='c/c/l'}");

    assert_null(lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;grouping grp {leaf l {type string; status deprecated;}}"
                                        "uses grp {status obsolete;}}", LYS_IN_YANG));
    logbuf_assert("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status. /ii:{uses='grp'}/ii:l");

    assert_null(lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;import grp {prefix g;}"
                                        "uses g:grp {refine c/x {presence nonsence;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of presence statement - leaf cannot hold the presence statement. /jj:{uses='g:grp'}/{refine='c/x'}");

    assert_null(lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;import grp {prefix g;}"
                                        "uses g:grp {refine c/ch {must 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of must statement - choice cannot hold any must statement. /kk:{uses='g:grp'}/{refine='c/ch'}");

    assert_null(lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;import grp {prefix g;}"
                                        "uses g:grp {refine c/x {min-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of min-elements statement - leaf cannot hold this statement. /ll:{uses='g:grp'}/{refine='c/x'}");

    assert_null(lys_parse_mem(ctx, "module mm {namespace urn:mm;prefix mm;import grp {prefix g;}"
                                        "uses g:grp {refine c/ll {min-elements 10;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid refine of min-elements statement - \"min-elements\" is bigger than \"max-elements\". /mm:{uses='g:grp'}/{refine='c/ll'}");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_augment(void **state)
{
    *state = test_augment;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    const struct lysc_node *node;
    const struct lysc_node_choice *ch;
    const struct lysc_node_case *c;
    const struct lysc_action *rpc;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module a {namespace urn:a;prefix a; typedef atype {type string;}"
                              "container top {leaf a {type string;}}}");
    assert_non_null(lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;import a {prefix a;}"
                                  "leaf b {type a:atype;}}", LYS_IN_YANG));
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module c {namespace urn:c;prefix c; import a {prefix a;}"
                              "augment /a:top/ { container c {leaf c {type a:atype;}}}}");
    assert_non_null(lys_parse_mem(ctx, "module d {namespace urn:d;prefix d;import a {prefix a;} import c {prefix c;}"
                                  "augment /a:top/c:c/ { leaf d {type a:atype;} leaf c {type string;}}}", LYS_IN_YANG));
    assert_non_null((mod = ly_ctx_get_module_implemented(ctx, "a")));
    assert_non_null(ly_ctx_get_module_implemented(ctx, "b"));
    assert_non_null(ly_ctx_get_module_implemented(ctx, "c"));
    assert_non_null(ly_ctx_get_module_implemented(ctx, "d"));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal(node->name, "top");
    assert_non_null(node = lysc_node_children(node, 0));
    assert_string_equal(node->name, "a");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "c");
    assert_non_null(node = lysc_node_children(node, 0));
    assert_string_equal(node->name, "c");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "d");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "c");

    assert_non_null((mod = lys_parse_mem(ctx, "module e {namespace urn:e;prefix e;choice ch {leaf a {type string;}}"
                                         "augment /ch/c {when 1; leaf lc2 {type uint16;}}"
                                         "augment /ch { when 1; leaf b {type int8;} case c {leaf lc1 {type uint8;}}}}", LYS_IN_YANG)));
    assert_non_null((ch = (const struct lysc_node_choice*)mod->compiled->data));
    assert_null(mod->compiled->data->next);
    assert_string_equal("ch", ch->name);
    assert_non_null(c = ch->cases);
    assert_string_equal("a", c->name);
    assert_null(c->when);
    assert_string_equal("a", c->child->name);
    assert_non_null(c = (const struct lysc_node_case*)c->next);
    assert_string_equal("b", c->name);
    assert_non_null(c->when);
    assert_string_equal("b", c->child->name);
    assert_non_null(c = (const struct lysc_node_case*)c->next);
    assert_string_equal("c", c->name);
    assert_non_null(c->when);
    assert_string_equal("lc1", ((const struct lysc_node_case*)c)->child->name);
    assert_null(((const struct lysc_node_case*)c)->child->when);
    assert_string_equal("lc2", ((const struct lysc_node_case*)c)->child->next->name);
    assert_non_null(((const struct lysc_node_case*)c)->child->next->when);
    assert_ptr_equal(ch->cases->child->prev, ((const struct lysc_node_case*)c)->child->next);
    assert_null(c->next);

    assert_non_null((mod = lys_parse_mem(ctx, "module f {namespace urn:f;prefix f;grouping g {leaf a {type string;}}"
                                         "container c;"
                                         "augment /c {uses g;}}", LYS_IN_YANG)));
    assert_non_null(node = lysc_node_children(mod->compiled->data, 0));
    assert_string_equal(node->name, "a");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule gsub {belongs-to g {prefix g;}"
                                  "augment /c {container sub;}}");
    assert_non_null(mod = lys_parse_mem(ctx, "module g {namespace urn:g;prefix g;include gsub; container c;"
                                        "augment /c/sub {leaf main {type string;}}}", LYS_IN_YANG));
    assert_non_null(mod->compiled->data);
    assert_string_equal("c", mod->compiled->data->name);
    assert_non_null(node = ((struct lysc_node_container*)mod->compiled->data)->child);
    assert_string_equal("sub", node->name);
    assert_non_null(node = ((struct lysc_node_container*)node)->child);
    assert_string_equal("main", node->name);

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module himp {namespace urn:hi;prefix hi;container top; rpc func;}");
    assert_non_null(mod = lys_parse_mem(ctx, "module h {namespace urn:h;prefix h;import himp {prefix hi;}container top;"
                                        "augment /hi:top {container p {presence XXX; leaf x {mandatory true;type string;}}}"
                                        "augment /hi:top {list ll {key x;leaf x {type string;}leaf y {mandatory true; type string;}}}"
                                        "augment /hi:top {leaf l {type string; mandatory true; config false;}}"
                                        "augment /top {leaf l {type string; mandatory true;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_non_null(node = ((struct lysc_node_container*)node)->child);
    assert_string_equal("l", node->name);
    assert_true(node->flags & LYS_MAND_TRUE);
    assert_non_null(mod = ly_ctx_get_module_implemented(ctx, "himp"));
    assert_non_null(node = mod->compiled->data);
    assert_non_null(node = ((struct lysc_node_container*)node)->child);
    assert_string_equal("p", node->name);
    assert_non_null(node = node->next);
    assert_string_equal("ll", node->name);
    assert_non_null(node = node->next);
    assert_string_equal("l", node->name);
    assert_true(node->flags & LYS_CONFIG_R);

    assert_non_null(lys_parse_mem(ctx, "module i {namespace urn:i;prefix i;import himp {prefix hi;}"
                                        "augment /hi:func/input {leaf x {type string;}}"
                                        "augment /hi:func/output {leaf y {type string;}}}", LYS_IN_YANG));
    assert_non_null(mod = ly_ctx_get_module_implemented(ctx, "himp"));
    assert_non_null(rpc = mod->compiled->rpcs);
    assert_int_equal(1, LY_ARRAY_SIZE(rpc));
    assert_non_null(rpc->input.data);
    assert_string_equal("x", rpc->input.data->name);
    assert_null(rpc->input.data->next);
    assert_non_null(rpc->output.data);
    assert_string_equal("y", rpc->output.data->name);
    assert_null(rpc->output.data->next);

    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; container c {leaf a {type string;}}"
                                        "augment /x {leaf a {type int8;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid absolute-schema-nodeid value \"/x\" - target node not found. /aa:{augment='/x'}");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; container c {leaf a {type string;}}"
                                        "augment /c {leaf a {type int8;}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"a\" of data definition/RPC/action/Notification statement. /bb:{augment='/c'}/a");


    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc; container c {leaf a {type string;}}"
                                        "augment /c/a {leaf a {type int8;}}}", LYS_IN_YANG));
    logbuf_assert("Augment's absolute-schema-nodeid \"/c/a\" refers to a leaf node which is not an allowed augment's target. /cc:{augment='/c/a'}");

    assert_null(lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd; container c {leaf a {type string;}}"
                                        "augment /c {case b {leaf d {type int8;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid augment of container node which is not allowed to contain case node \"b\". /dd:{augment='/c'}");

    assert_null(lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee; import himp {prefix hi;}"
                                        "augment /hi:top {container c {leaf d {mandatory true; type int8;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid augment adding mandatory node \"c\" without making it conditional via when statement. /ee:{augment='/hi:top'}");

    assert_null(lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff; container top;"
                                        "augment ../top {leaf x {type int8;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid absolute-schema-nodeid value \"../top\" - missing starting \"/\". /ff:{augment='../top'}");

    assert_null(lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg; rpc func;"
                                        "augment /func {leaf x {type int8;}}}", LYS_IN_YANG));
    logbuf_assert("Augment's absolute-schema-nodeid \"/func\" refers to a RPC/action node which is not an allowed augment's target. /gg:{augment='/func'}");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_deviation(void **state)
{
    *state = test_deviation;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    const struct lysc_node *node;
    const struct lysc_node_list *list;
    const struct lysc_node_leaflist *llist;
    const struct lysc_node_leaf *leaf;
    const char *str;
    int dynamic;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module a {namespace urn:a;prefix a;"
                              "container top {leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
                              "choice ch {default c; case b {leaf b{type string;}} case a {leaf a{type string;} leaf x {type string;}}"
                              " case c {leaf c{type string;}}}"
                              "rpc func1 { input { leaf x {type int8;}} output {leaf y {type int8;}}}"
                              "rpc func2;}");
    assert_non_null(lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;import a {prefix a;}"
                                  "deviation /a:top/a:b {deviate not-supported;}"
                                  "deviation /a:ch/a:a/a:x {deviate not-supported;}"
                                  "deviation /a:ch/a:c/a:c {deviate not-supported;}"
                                  "deviation /a:ch/a:b {deviate not-supported;}"
                                  "deviation /a:ch/a:a/a:a {deviate not-supported;}"
                                  "deviation /a:func1/a:input {deviate not-supported;}"
                                  "deviation /a:func1/a:output {deviate not-supported;}"
                                  "deviation /a:func2 {deviate not-supported;}}", LYS_IN_YANG));
    assert_non_null((mod = ly_ctx_get_module_implemented(ctx, "a")));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal(node->name, "top");
    assert_non_null(node = lysc_node_children(node, 0));
    assert_string_equal(node->name, "a");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "c");
    assert_null(node = node->next);
    assert_non_null(node = mod->compiled->data->next);
    assert_string_equal("ch", node->name);
    assert_null(((struct lysc_node_choice*)node)->dflt);
    assert_null(((struct lysc_node_choice*)node)->cases);
    assert_int_equal(1, LY_ARRAY_SIZE(mod->compiled->rpcs));
    assert_null(mod->compiled->rpcs[0].input.data);
    assert_null(mod->compiled->rpcs[0].output.data);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c; typedef mytype {type string; units kilometers;}"
                                        "leaf c1 {type mytype;} leaf c2 {type mytype; units meters;} leaf c3 {type mytype; units meters;}"
                                        "deviation /c1 {deviate add {units meters;}}"
                                        "deviation /c2 {deviate delete {units meters;}}"
                                        "deviation /c3 {deviate replace {units centimeters;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("c1", node->name);
    assert_string_equal("meters", ((struct lysc_node_leaf*)node)->units);
    assert_non_null(node = node->next);
    assert_string_equal("c2", node->name);
    assert_null(((struct lysc_node_leaf*)node)->units);
    assert_non_null(node = node->next);
    assert_string_equal("c3", node->name);
    assert_string_equal("centimeters", ((struct lysc_node_leaf*)node)->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d; leaf c1 {type string; must 1;}"
                                        "container c2 {presence yes; must 1; must 2;} leaf c3 {type string; must 1; must 3;}"
                                        "deviation /c1 {deviate add {must 3;}}"
                                        "deviation /c2 {deviate delete {must 2;}}"
                                        "deviation /c3 {deviate delete {must 3; must 1;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("c1", node->name);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_node_leaf*)node)->musts));
    assert_string_equal("3", ((struct lysc_node_leaf*)node)->musts[1].cond->expr);
    assert_non_null(node = node->next);
    assert_string_equal("c2", node->name);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_node_container*)node)->musts));
    assert_string_equal("1", ((struct lysc_node_container*)node)->musts[0].cond->expr);
    assert_non_null(node = node->next);
    assert_string_equal("c3", node->name);
    assert_null(((struct lysc_node_leaf*)node)->musts);

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module e {yang-version 1.1; namespace urn:e;prefix e; typedef mytype {type string; default nothing;}"
                              "choice a {default a;leaf a {type string;} leaf b {type string;} leaf c {type string; mandatory true;}}"
                              "choice b {default a;leaf a {type string;} leaf b {type string;}}"
                              "leaf c {default hello; type string;}"
                              "leaf-list d {default hello; default world; type string;}"
                              "leaf c2 {type mytype;} leaf-list d2 {type mytype;}}");
    assert_non_null(lys_parse_mem(ctx, "module f {yang-version 1.1; namespace urn:f;prefix f;import e {prefix x;}"
                                  "deviation /x:a {deviate delete {default a;}}"
                                  "deviation /x:b {deviate delete {default x:a;}}"
                                  "deviation /x:c {deviate delete {default hello;}}"
                                  "deviation /x:d {deviate delete {default world;}}}", LYS_IN_YANG));
    assert_non_null((mod = ly_ctx_get_module_implemented(ctx, "e")));
    assert_non_null(node = mod->compiled->data);
    assert_null(((struct lysc_node_choice*)node)->dflt);
    assert_non_null(node = node->next);
    assert_null(((struct lysc_node_choice*)node)->dflt);
    assert_non_null(leaf = (struct lysc_node_leaf*)node->next);
    assert_null(leaf->dflt);
    assert_non_null(llist = (struct lysc_node_leaflist*)leaf->next);
    assert_int_equal(1, LY_ARRAY_SIZE(llist->dflts));
    assert_string_equal("hello", llist->dflts[0]->realtype->plugin->print(llist->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_non_null(leaf = (struct lysc_node_leaf*)llist->next);
    assert_string_equal("nothing", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(5, leaf->dflt->realtype->refcount); /* 3x type reference, 2x default value reference (typedef's default does not reference own type) */
    assert_non_null(llist = (struct lysc_node_leaflist*)leaf->next);
    assert_int_equal(1, LY_ARRAY_SIZE(llist->dflts));
    assert_string_equal("nothing", llist->dflts[0]->realtype->plugin->print(llist->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);

    assert_non_null(lys_parse_mem(ctx, "module g {yang-version 1.1; namespace urn:g;prefix g;import e {prefix x;}"
                                  "deviation /x:b {deviate add {default x:b;}}"
                                  "deviation /x:c {deviate add {default bye;}}"
                                  "deviation /x:d {deviate add {default all; default people;}}"
                                  "deviation /x:c2 {deviate add {default hi; must 1;}}"
                                  "deviation /x:d2 {deviate add {default hi; default all;}}}", LYS_IN_YANG));
    assert_non_null((mod = ly_ctx_get_module_implemented(ctx, "e")));
    assert_non_null(node = mod->compiled->data);
    assert_null(((struct lysc_node_choice*)node)->dflt);
    assert_non_null(node = node->next);
    assert_non_null(((struct lysc_node_choice*)node)->dflt);
    assert_string_equal("b", ((struct lysc_node_choice*)node)->dflt->name);
    assert_non_null(leaf = (struct lysc_node_leaf*)node->next);
    assert_non_null(leaf->dflt);
    assert_string_equal("bye", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_non_null(llist = (struct lysc_node_leaflist*)leaf->next);
    assert_int_equal(3, LY_ARRAY_SIZE(llist->dflts));
    assert_string_equal("hello", llist->dflts[0]->realtype->plugin->print(llist->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("all", llist->dflts[1]->realtype->plugin->print(llist->dflts[1], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("people", llist->dflts[2]->realtype->plugin->print(llist->dflts[2], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_non_null(leaf = (struct lysc_node_leaf*)llist->next);
    assert_non_null(leaf->dflt);
    assert_string_equal("hi", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(6, leaf->dflt->realtype->refcount); /* 3x type reference, 3x default value reference
    - previous type's default values were replaced by node's default values where d2 now has 2 default values */
    assert_int_equal(1, LY_ARRAY_SIZE(leaf->musts));
    assert_ptr_equal(leaf->musts[0].module, ly_ctx_get_module_implemented(ctx, "g"));
    assert_non_null(llist = (struct lysc_node_leaflist*)leaf->next);
    assert_int_equal(2, LY_ARRAY_SIZE(llist->dflts));
    assert_string_equal("hi", llist->dflts[0]->realtype->plugin->print(llist->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_string_equal("all", llist->dflts[1]->realtype->plugin->print(llist->dflts[1], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);

    assert_non_null(lys_parse_mem(ctx, "module h {yang-version 1.1; namespace urn:h;prefix h;import e {prefix x;}"
                                  "deviation /x:b {deviate replace {default x:a;}}"
                                  "deviation /x:c {deviate replace {default hello;}}}", LYS_IN_YANG));
    assert_non_null((mod = ly_ctx_get_module_implemented(ctx, "e")));
    assert_non_null(node = mod->compiled->data);
    assert_null(((struct lysc_node_choice*)node)->dflt);
    assert_non_null(node = node->next);
    assert_non_null(((struct lysc_node_choice*)node)->dflt);
    assert_string_equal("a", ((struct lysc_node_choice*)node)->dflt->name);
    assert_non_null(leaf = (struct lysc_node_leaf*)node->next);
    assert_non_null(leaf->dflt);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "module i {namespace urn:i;prefix i;"
                              "list l1 {key a; leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
                              "list l2 {key a; unique \"b c\"; unique \"d\"; leaf a {type string;} leaf b {type string;}"
                              "         leaf c {type string;} leaf d {type string;}}}");
    assert_non_null(lys_parse_mem(ctx, "module j {namespace urn:j;prefix j;import i {prefix i;}"
                                  "augment /i:l1 {leaf j_c {type string;}}"
                                  "deviation /i:l1 {deviate add {unique \"i:b j_c\"; }}"
                                  "deviation /i:l1 {deviate add {unique \"i:c\";}}"
                                  "deviation /i:l2 {deviate delete {unique \"d\"; unique \"b c\";}}}", LYS_IN_YANG));
    assert_non_null((mod = ly_ctx_get_module_implemented(ctx, "i")));
    assert_non_null(list = (struct lysc_node_list*)mod->compiled->data);
    assert_string_equal("l1", list->name);
    assert_int_equal(2, LY_ARRAY_SIZE(list->uniques));
    assert_int_equal(2, LY_ARRAY_SIZE(list->uniques[0]));
    assert_string_equal("b", list->uniques[0][0]->name);
    assert_string_equal("j_c", list->uniques[0][1]->name);
    assert_int_equal(1, LY_ARRAY_SIZE(list->uniques[1]));
    assert_string_equal("c", list->uniques[1][0]->name);
    assert_non_null(list = (struct lysc_node_list*)list->next);
    assert_string_equal("l2", list->name);
    assert_null(list->uniques);

    assert_non_null(mod = lys_parse_mem(ctx, "module k {namespace urn:k;prefix k; leaf a {type string;}"
                                        "container top {leaf x {type string;} leaf y {type string; config false;}}"
                                        "deviation /a {deviate add {config false; }}"
                                        "deviation /top {deviate add {config false;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_true(node->flags & LYS_CONFIG_R);
    assert_non_null(node = node->next);
    assert_string_equal("top", node->name);
    assert_true(node->flags & LYS_CONFIG_R);
    assert_non_null(node = lysc_node_children(node, 0));
    assert_string_equal("x", node->name);
    assert_true(node->flags & LYS_CONFIG_R);
    assert_non_null(node = node->next);
    assert_string_equal("y", node->name);
    assert_true(node->flags & LYS_CONFIG_R);

    assert_non_null(mod = lys_parse_mem(ctx, "module l {namespace urn:l;prefix l; leaf a {config false; type string;}"
                                        "container top {config false; leaf x {type string;}}"
                                        "deviation /a {deviate replace {config true;}}"
                                        "deviation /top {deviate replace {config true;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_true(node->flags & LYS_CONFIG_W);
    assert_non_null(node = node->next);
    assert_string_equal("top", node->name);
    assert_true(node->flags & LYS_CONFIG_W);
    assert_non_null(node = lysc_node_children(node, 0));
    assert_string_equal("x", node->name);
    assert_true(node->flags & LYS_CONFIG_W);

    assert_non_null(mod = lys_parse_mem(ctx, "module m {namespace urn:m;prefix m;"
                                        "container a {leaf a {type string;}}"
                                        "container b {leaf b {mandatory true; type string;}}"
                                        "deviation /a/a {deviate add {mandatory true;}}"
                                        "deviation /b/b {deviate replace {mandatory false;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_true((node->flags & LYS_MAND_MASK) == LYS_MAND_TRUE);
    assert_true((lysc_node_children(node, 0)->flags & LYS_MAND_MASK) == LYS_MAND_TRUE);
    assert_non_null(node = node->next);
    assert_string_equal("b", node->name);
    assert_false(node->flags & LYS_MAND_MASK); /* just unset on container */
    assert_true((lysc_node_children(node, 0)->flags & LYS_MAND_MASK) == LYS_MAND_FALSE);

    assert_non_null(mod = lys_parse_mem(ctx, "module n {yang-version 1.1; namespace urn:n;prefix n;"
                                        "leaf a {default test; type string;}"
                                        "leaf b {mandatory true; type string;}"
                                        "deviation /a {deviate add {mandatory true;} deviate delete {default test;}}"
                                        "deviation /b {deviate add {default test;} deviate replace {mandatory false;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_null(((struct lysc_node_leaf*)node)->dflt);
    assert_true((node->flags & LYS_MAND_MASK) == LYS_MAND_TRUE);
    assert_non_null(node = node->next);
    assert_string_equal("b", node->name);
    assert_non_null(((struct lysc_node_leaf*)node)->dflt);
    assert_true((node->flags & LYS_MAND_MASK) == LYS_MAND_FALSE);

    assert_non_null(mod = lys_parse_mem(ctx, "module o {namespace urn:o;prefix o;"
                                        "leaf-list a {type string;}"
                                        "list b {config false;}"
                                        "leaf-list c {min-elements 1; max-elements 10; type string;}"
                                        "list d {min-elements 10; max-elements 100; config false;}"
                                        "deviation /a {deviate add {min-elements 1; max-elements 10;}}"
                                        "deviation /b {deviate add {min-elements 10; max-elements 100;}}"
                                        "deviation /c {deviate replace {min-elements 10; max-elements 100;}}"
                                        "deviation /d {deviate replace {min-elements 1; max-elements 10;}}}", LYS_IN_YANG));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_int_equal(1, ((struct lysc_node_leaflist*)node)->min);
    assert_int_equal(10, ((struct lysc_node_leaflist*)node)->max);
    assert_non_null(node = node->next);
    assert_string_equal("b", node->name);
    assert_int_equal(10, ((struct lysc_node_list*)node)->min);
    assert_int_equal(100, ((struct lysc_node_list*)node)->max);
    assert_non_null(node = node->next);
    assert_string_equal("c", node->name);
    assert_int_equal(10, ((struct lysc_node_leaflist*)node)->min);
    assert_int_equal(100, ((struct lysc_node_leaflist*)node)->max);
    assert_non_null(node = node->next);
    assert_string_equal("d", node->name);
    assert_int_equal(1, ((struct lysc_node_list*)node)->min);
    assert_int_equal(10, ((struct lysc_node_list*)node)->max);

    assert_non_null(mod = lys_parse_mem(ctx, "module p {yang-version 1.1; namespace urn:p;prefix p; typedef mytype {type int8; default 1;}"
                                        "leaf a {type string; default 10;} leaf-list b {type string;}"
                                        "deviation /a {deviate replace {type mytype;}}"
                                        "deviation /b {deviate replace {type mytype;}}}", LYS_IN_YANG));
    assert_non_null(leaf = (struct lysc_node_leaf*)mod->compiled->data);
    assert_string_equal("a", leaf->name);
    assert_int_equal(LY_TYPE_INT8, leaf->type->basetype);
    assert_string_equal("10", leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(10, leaf->dflt->uint8);
    assert_non_null(llist = (struct lysc_node_leaflist*)leaf->next);
    assert_string_equal("b", llist->name);
    assert_int_equal(LY_TYPE_INT8, llist->type->basetype);
    assert_int_equal(1, LY_ARRAY_SIZE(llist->dflts));
    assert_string_equal("1", llist->dflts[0]->realtype->plugin->print(llist->dflts[0], LYD_XML, NULL, NULL, &dynamic));
    assert_int_equal(0, dynamic);
    assert_int_equal(1, llist->dflts[0]->uint8);

    /* instance-identifiers with NULL canonical_cach are changed to string types with a canonical_cache value equal to the original value */
    assert_non_null(mod = lys_parse_mem(ctx, "module q {yang-version 1.1; namespace urn:q;prefix q; import e {prefix e;}"
                                        "leaf q {type instance-identifier; default \"/e:d2\";}"
                                        "leaf-list ql {type instance-identifier; default \"/e:d\"; default \"/e:d2\";}}", LYS_IN_YANG));
    assert_non_null(lys_parse_mem(ctx, "module qdev {yang-version 1.1; namespace urn:qdev;prefix qd; import q {prefix q;}"
                                  "deviation /q:q { deviate replace {type string;}}"
                                  "deviation /q:ql { deviate replace {type string;}}}", LYS_IN_YANG));
    assert_non_null(leaf = (struct lysc_node_leaf*)mod->compiled->data);
    assert_int_equal(LY_TYPE_STRING, leaf->dflt->realtype->basetype);
    assert_non_null(leaf->dflt->canonical_cache);
    assert_string_equal("/e:d2", leaf->dflt->canonical_cache);
    assert_non_null(llist = (struct lysc_node_leaflist*)leaf->next);
    assert_int_equal(2, LY_ARRAY_SIZE(llist->dflts));
    assert_int_equal(2, LY_ARRAY_SIZE(llist->dflts_mods));
    assert_ptr_equal(llist->dflts_mods[0], mod);
    assert_int_equal(LY_TYPE_STRING, llist->dflts[0]->realtype->basetype);
    assert_string_equal("/e:d", llist->dflts[0]->canonical_cache);
    assert_ptr_equal(llist->dflts_mods[1], mod);
    assert_int_equal(LY_TYPE_STRING, llist->dflts[0]->realtype->basetype);
    assert_string_equal("/e:d2", llist->dflts[1]->canonical_cache);

    assert_non_null(mod = lys_parse_mem(ctx, "module r {yang-version 1.1; namespace urn:r;prefix r;"
                                        "typedef mytype {type uint8; default 200;}"
                                        "leaf r {type mytype;} leaf-list lr {type mytype;}"
                                        "deviation /r:r {deviate replace {type string;}}"
                                        "deviation /r:lr {deviate replace {type string;}}}", LYS_IN_YANG));
    assert_non_null(leaf = (struct lysc_node_leaf*)mod->compiled->data);
    assert_string_equal("r", leaf->name);
    assert_null(leaf->dflt);
    assert_null(leaf->dflt_mod);
    assert_non_null(llist = (struct lysc_node_leaflist* )leaf->next);
    assert_string_equal("lr", llist->name);
    assert_null(llist->dflts);
    assert_null(llist->dflts_mods);

    assert_non_null(mod = lys_parse_mem(ctx, "module s {yang-version 1.1; namespace urn:s;prefix s;"
                                        "leaf s {type instance-identifier {require-instance true;} default /s:x;}"
                                        "leaf x {type string;} leaf y {type string;}"
                                        "deviation /s:s {deviate replace {default /s:y;}}}", LYS_IN_YANG));
    assert_non_null(leaf = (struct lysc_node_leaf*)mod->compiled->data);
    assert_string_equal("s", leaf->name);
    assert_non_null(leaf->dflt);
    assert_non_null(str = leaf->dflt->realtype->plugin->print(leaf->dflt, LYD_XML, lys_get_prefix, mod, &dynamic));
    assert_string_equal("/s:y", str);
    if (dynamic) { free((char*)str); }

    assert_null(lys_parse_mem(ctx, "module aa1 {namespace urn:aa1;prefix aa1;import a {prefix a;}"
                              "deviation /a:top/a:z {deviate not-supported;}}", LYS_IN_YANG));
    logbuf_assert("Invalid absolute-schema-nodeid value \"/a:top/a:z\" - target node not found. /aa1:{deviation='/a:top/a:z'}");
    assert_non_null(lys_parse_mem(ctx, "module aa2 {namespace urn:aa2;prefix aa2;import a {prefix a;}"
                              "deviation /a:top/a:a {deviate not-supported;}"
                              "deviation /a:top/a:a {deviate add {default error;}}}", LYS_IN_YANG));
    /* warning */
    logbuf_assert("Useless multiple (2) deviates on node \"/a:top/a:a\" since the node is not-supported.");

    assert_null(lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;import a {prefix a;}"
                              "deviation a:top/a:a {deviate not-supported;}}", LYS_IN_YANG));
    logbuf_assert("Invalid absolute-schema-nodeid value \"a:top/a:a\" - missing starting \"/\". /bb:{deviation='a:top/a:a'}");

    assert_null(lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc; container c;"
                              "deviation /c {deviate add {units meters;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of container node - it is not possible to add \"units\" property. /cc:{deviation='/c'}");
    assert_null(lys_parse_mem(ctx, "module cd {namespace urn:cd;prefix cd; leaf c {type string; units centimeters;}"
                              "deviation /c {deviate add {units meters;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"units\" property which already exists (with value \"centimeters\"). /cd:{deviation='/c'}");

    assert_null(lys_parse_mem(ctx, "module dd1 {namespace urn:dd1;prefix dd1; container c;"
                              "deviation /c {deviate delete {units meters;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of container node - it is not possible to delete \"units\" property. /dd1:{deviation='/c'}");
    assert_null(lys_parse_mem(ctx, "module dd2 {namespace urn:dd2;prefix dd2; leaf c {type string;}"
                              "deviation /c {deviate delete {units meters;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"units\" property \"meters\" which is not present. /dd2:{deviation='/c'}");
    assert_null(lys_parse_mem(ctx, "module dd3 {namespace urn:dd3;prefix dd3; leaf c {type string; units centimeters;}"
                              "deviation /c {deviate delete {units meters;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"units\" property \"meters\" which does not match the target's property value \"centimeters\"."
            " /dd3:{deviation='/c'}");

    assert_null(lys_parse_mem(ctx, "module ee1 {namespace urn:ee1;prefix ee1; container c;"
                              "deviation /c {deviate replace {units meters;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of container node - it is not possible to replace \"units\" property. /ee1:{deviation='/c'}");
    assert_null(lys_parse_mem(ctx, "module ee2 {namespace urn:ee2;prefix ee2; leaf c {type string;}"
                              "deviation /c {deviate replace {units meters;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing \"units\" property \"meters\" which is not present. /ee2:{deviation='/c'}");

    /* the default is already deleted in /e:a byt module f */
    assert_null(lys_parse_mem(ctx, "module ff1 {namespace urn:ff1;prefix ff1; import e {prefix e;}"
                              "deviation /e:a {deviate delete {default x:a;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"default\" property \"x:a\" which is not present. /ff1:{deviation='/e:a'}");
    assert_null(lys_parse_mem(ctx, "module ff2 {namespace urn:ff2;prefix ff2; import e {prefix e;}"
                              "deviation /e:b {deviate delete {default x:a;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"default\" property \"x:a\" of choice. "
                  "The prefix does not match any imported module of the deviation module. /ff2:{deviation='/e:b'}");
    assert_null(lys_parse_mem(ctx, "module ff3 {namespace urn:ff3;prefix ff3; import e {prefix e;}"
                              "deviation /e:b {deviate delete {default e:b;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"default\" property \"e:b\" of choice does not match the default case name \"a\". /ff3:{deviation='/e:b'}");
    assert_null(lys_parse_mem(ctx, "module ff4 {namespace urn:ff4;prefix ff4; import e {prefix e;}"
                              "deviation /e:b {deviate delete {default ff4:a;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"default\" property \"ff4:a\" of choice. The prefix does not match the default case's module. /ff4:{deviation='/e:b'}");
    assert_null(lys_parse_mem(ctx, "module ff5 {namespace urn:ff5;prefix ff5; anyxml a;"
                              "deviation /a {deviate delete {default x;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of anyxml node - it is not possible to delete \"default\" property. /ff5:{deviation='/a'}");
    assert_null(lys_parse_mem(ctx, "module ff6 {namespace urn:ff6;prefix ff6; import e {prefix e;}"
                              "deviation /e:c {deviate delete {default hi;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"default\" property \"hi\" which does not match the target's property value \"hello\". /ff6:{deviation='/e:c'}");
    assert_null(lys_parse_mem(ctx, "module ff7 {namespace urn:ff7;prefix ff7; import e {prefix e;}"
                              "deviation /e:d {deviate delete {default hi;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"default\" property \"hi\" which does not match any of the target's property values. /ff7:{deviation='/e:d'}");

    assert_null(lys_parse_mem(ctx, "module gg1 {namespace urn:gg1;prefix gg1; import e {prefix e;}"
                              "deviation /e:b {deviate add {default e:a;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"default\" property which already exists (with value \"a\"). /gg1:{deviation='/e:b'}");
    assert_null(lys_parse_mem(ctx, "module gg2 {namespace urn:gg2;prefix gg2; import e {prefix e;}"
                              "deviation /e:a {deviate add {default x:a;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"default\" property \"x:a\" of choice. "
                  "The prefix does not match any imported module of the deviation module. /gg2:{deviation='/e:a'}");
    assert_null(lys_parse_mem(ctx, "module gg3 {namespace urn:gg3;prefix gg3; import e {prefix e;}"
                              "deviation /e:a {deviate add {default a;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"default\" property \"a\" of choice - the specified case does not exists. /gg3:{deviation='/e:a'}");
    assert_null(lys_parse_mem(ctx, "module gg4 {namespace urn:gg4;prefix gg4; import e {prefix e;}"
                              "deviation /e:c {deviate add {default hi;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"default\" property which already exists (with value \"hello\"). /gg4:{deviation='/e:c'}");
    assert_null(lys_parse_mem(ctx, "module gg4 {namespace urn:gg4;prefix gg4; import e {prefix e;}"
                              "deviation /e:a {deviate add {default e:c;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"default\" property \"e:c\" of choice - mandatory node \"c\" under the default case. /gg4:{deviation='/e:a'}");
    assert_null(lys_parse_mem(ctx, "module gg5 {namespace urn:gg5;prefix gg5; leaf x {type string; mandatory true;}"
                              "deviation /x {deviate add {default error;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation combining default value and mandatory leaf. /gg5:{deviation='/x'}");

    assert_null(lys_parse_mem(ctx, "module hh1 {yang-version 1.1; namespace urn:hh1;prefix hh1; import e {prefix e;}"
                              "deviation /e:d {deviate replace {default hi;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of leaf-list node - it is not possible to replace \"default\" property. /hh1:{deviation='/e:d'}");

    assert_null(lys_parse_mem(ctx, "module ii1 {namespace urn:ii1;prefix ii1; import i {prefix i;}"
                              "deviation /i:l1 {deviate delete {unique x;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"unique\" property \"x\" which does not match any of the target's property values. /ii1:{deviation='/i:l1'}");
    assert_null(lys_parse_mem(ctx, "module ii2 {namespace urn:ii2;prefix ii2; import i {prefix i;} leaf x { type string;}"
                              "deviation /i:l2 {deviate delete {unique d;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation deleting \"unique\" property \"d\" which does not match any of the target's property values. /ii2:{deviation='/i:l2'}");
    assert_null(lys_parse_mem(ctx, "module ii3 {namespace urn:ii3;prefix ii3; leaf x { type string;}"
                              "deviation /x {deviate delete {unique d;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of leaf node - it is not possible to delete \"unique\" property. /ii3:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module ii4 {namespace urn:ii4;prefix ii4; leaf x { type string;}"
                              "deviation /x {deviate add {unique d;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of leaf node - it is not possible to add \"unique\" property. /ii4:{deviation='/x'}");

    assert_null(lys_parse_mem(ctx, "module jj1 {namespace urn:jj1;prefix jj1; choice ch {case a {leaf a{type string;}}}"
                              "deviation /ch/a {deviate add {config false;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of case node - it is not possible to add \"config\" property. /jj1:{deviation='/ch/a'}");
    assert_null(lys_parse_mem(ctx, "module jj2 {namespace urn:jj2;prefix jj2; container top {config false; leaf x {type string;}}"
                              "deviation /top/x {deviate add {config true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of config - configuration node cannot be child of any state data node. /jj2:{deviation='/top/x'}");
    assert_null(lys_parse_mem(ctx, "module jj3 {namespace urn:jj3;prefix jj3; container top {leaf x {type string;}}"
                              "deviation /top/x {deviate replace {config false;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing \"config\" property \"config false\" which is not present. /jj3:{deviation='/top/x'}");
    assert_null(lys_parse_mem(ctx, "module jj4 {namespace urn:jj4;prefix jj4; choice ch {case a {leaf a{type string;}}}"
                              "deviation /ch/a {deviate replace {config false;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of case node - it is not possible to replace \"config\" property. /jj4:{deviation='/ch/a'}");
    assert_null(lys_parse_mem(ctx, "module jj5 {namespace urn:jj5;prefix jj5; container top {leaf x {type string; config true;}}"
                              "deviation /top {deviate add {config false;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of config - configuration node cannot be child of any state data node. /jj5:{deviation='/top'}");
    assert_null(lys_parse_mem(ctx, "module jj6 {namespace urn:jj6;prefix jj6; leaf x {config false; type string;}"
                              "deviation /x {deviate add {config true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"config\" property which already exists (with value \"config false\"). /jj6:{deviation='/x'}");

    assert_null(lys_parse_mem(ctx, "module kk1 {namespace urn:kk1;prefix kk1; container top {leaf a{type string;}}"
                              "deviation /top {deviate add {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of mandatory - container cannot hold mandatory statement. /kk1:{deviation='/top'}");
    assert_null(lys_parse_mem(ctx, "module kk2 {namespace urn:kk2;prefix kk2; container top {leaf a{type string;}}"
                              "deviation /top {deviate replace {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing \"mandatory\" property \"mandatory true\" which is not present. /kk2:{deviation='/top'}");
    assert_null(lys_parse_mem(ctx, "module kk3 {namespace urn:kk3;prefix kk3; container top {leaf x {type string;}}"
                              "deviation /top/x {deviate replace {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing \"mandatory\" property \"mandatory true\" which is not present. /kk3:{deviation='/top/x'}");
    assert_null(lys_parse_mem(ctx, "module kk4 {namespace urn:kk4;prefix kk4; leaf x {mandatory true; type string;}"
                              "deviation /x {deviate add {mandatory false;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"mandatory\" property which already exists (with value \"mandatory true\"). /kk4:{deviation='/x'}");

    assert_null(lys_parse_mem(ctx, "module ll1 {namespace urn:ll1;prefix ll1; leaf x {default test; type string;}"
                              "deviation /x {deviate add {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation combining default value and mandatory leaf. /ll1:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module ll2 {yang-version 1.1; namespace urn:ll2;prefix ll2; leaf-list x {default test; type string;}"
                              "deviation /x {deviate add {min-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation combining default value and mandatory leaf-list. /ll2:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module ll2 {namespace urn:ll2;prefix ll2; choice ch {default a; leaf a {type string;} leaf b {type string;}}"
                              "deviation /ch {deviate add {mandatory true;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation combining default case and mandatory choice. /ll2:{deviation='/ch'}");

    assert_null(lys_parse_mem(ctx, "module mm1 {namespace urn:mm1;prefix mm1; leaf-list x {min-elements 10; type string;}"
                              "deviation /x {deviate add {max-elements 5;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid combination of min-elements and max-elements after deviation: min value 10 is bigger than max value 5. /mm1:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm2 {namespace urn:mm2;prefix mm2; leaf-list x {max-elements 10; type string;}"
                              "deviation /x {deviate add {min-elements 20;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid combination of min-elements and max-elements after deviation: min value 20 is bigger than max value 10. /mm2:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm3 {namespace urn:mm3;prefix mm3; list x {min-elements 5; max-elements 10; config false;}"
                              "deviation /x {deviate replace {max-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid combination of min-elements and max-elements after deviation: min value 5 is bigger than max value 1. /mm3:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm4 {namespace urn:mm4;prefix mm4; list x {min-elements 5; max-elements 10; config false;}"
                              "deviation /x {deviate replace {min-elements 20;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid combination of min-elements and max-elements after deviation: min value 20 is bigger than max value 10. /mm4:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm5 {namespace urn:mm5;prefix mm5; leaf-list x {type string; min-elements 5;}"
                              "deviation /x {deviate add {min-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"min-elements\" property which already exists (with value \"5\"). /mm5:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm6 {namespace urn:mm6;prefix mm6; list x {config false; min-elements 5;}"
                              "deviation /x {deviate add {min-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"min-elements\" property which already exists (with value \"5\"). /mm6:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm7 {namespace urn:mm7;prefix mm7; leaf-list x {type string; max-elements 5;}"
                              "deviation /x {deviate add {max-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"max-elements\" property which already exists (with value \"5\"). /mm7:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm8 {namespace urn:mm8;prefix mm8; list x {config false; max-elements 5;}"
                              "deviation /x {deviate add {max-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation adding \"max-elements\" property which already exists (with value \"5\"). /mm8:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm9 {namespace urn:mm9;prefix mm9; leaf-list x {type string;}"
                              "deviation /x {deviate replace {min-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing with \"min-elements\" property \"1\" which is not present. /mm9:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm10 {namespace urn:mm10;prefix mm10; list x {config false;}"
                              "deviation /x {deviate replace {min-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing with \"min-elements\" property \"1\" which is not present. /mm10:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm11 {namespace urn:mm11;prefix mm11; leaf-list x {type string;}"
                              "deviation /x {deviate replace {max-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing with \"max-elements\" property \"1\" which is not present. /mm11:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module mm12 {namespace urn:mm12;prefix mm12; list x {config false; }"
                              "deviation /x {deviate replace {max-elements 1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing with \"max-elements\" property \"1\" which is not present. /mm12:{deviation='/x'}");

    assert_null(lys_parse_mem(ctx, "module nn1 {namespace urn:nn1;prefix nn1; anyxml x;"
                              "deviation /x {deviate replace {type string;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation of anyxml node - it is not possible to replace \"type\" property. /nn1:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module nn2 {namespace urn:nn2;prefix nn2; leaf-list x {type string;}"
                              "deviation /x {deviate replace {type empty;}}}", LYS_IN_YANG));
    logbuf_assert("Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules. /nn2:{deviation='/x'}");

    assert_null(lys_parse_mem(ctx, "module oo1 {namespace urn:oo1;prefix oo1; leaf x {type uint16; default 300;}"
                                  "deviation /x {deviate replace {type uint8;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing leaf's type - the leaf's default value \"300\" does not match the type "
                  "(Value \"300\" is out of uint8's min/max bounds.). /oo1:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module oo2 {yang-version 1.1;namespace urn:oo2;prefix oo2; leaf-list x {type uint16; default 10; default 300;}"
                                  "deviation /x {deviate replace {type uint8;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation replacing leaf-list's type - the leaf-list's default value \"300\" does not match the type "
                  "(Value \"300\" is out of uint8's min/max bounds.). /oo2:{deviation='/x'}");
    assert_null(lys_parse_mem(ctx, "module oo3 {namespace urn:oo3;prefix oo3; leaf x {type uint8;}"
                                  "deviation /x {deviate add {default 300;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid deviation setting \"default\" property \"300\" which does not fit the type "
                  "(Value \"300\" is out of uint8's min/max bounds.). /oo3:{deviation='/x'}");

/* TODO recompiling reference object after deviation changes schema tree
    assert_non_null(lys_parse_mem(ctx, "module pp {namespace urn:pp;prefix pp; leaf l { type leafref {path /c/x;}}"
                                  "container c {leaf x {type string;} leaf y {type string;}}}", LYS_IN_YANG));
    assert_null(lys_parse_mem(ctx, "module pp1 {namespace urn:pp1;prefix pp1; import pp {prefix pp;}"
                              "deviation /pp:c/pp:x {deviate not-supported;}}", LYS_IN_YANG));
    logbuf_assert("???. /pp:l}");
*/

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_when(void **state)
{
    *state = test_when;

    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_null(lys_parse_mem(ctx,
        "module a {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "leaf l {"
                    "when \"/cont/lst[val='25']\";"
                    "type empty;"
                "}"
                "list lst {"
                    "key \"k\";"
                    "leaf k {"
                        "type uint8;"
                    "}"
                    "leaf val {"
                        "when /cont2;"
                        "type int32;"
                    "}"
                "}"
            "}"
            "container cont2 {"
                "presence \"a\";"
                "when ../cont/l;"
            "}"
        "}"
    , LYS_IN_YANG));
    logbuf_assert("When condition of \"cont2\" includes a self-reference (referenced by when of \"l\"). /a:cont2");

    assert_null(lys_parse_mem(ctx,
        "module a {"
            "namespace urn:a;"
            "prefix a;"
            "container cont {"
                "leaf l {"
                    "when \"/cont/lst[val='25']\";"
                    "type empty;"
                "}"
                "list lst {"
                    "key \"k\";"
                    "leaf k {"
                        "type uint8;"
                    "}"
                    "leaf val {"
                        "when /cont2;"
                        "type int32;"
                    "}"
                "}"
            "}"
            "container cont2 {"
                "presence \"a\";"
                "when ../cont/lst/val;"
            "}"
        "}"
    , LYS_IN_YANG));
    logbuf_assert("When condition of \"cont2\" includes a self-reference (referenced by when of \"val\"). /a:cont2");

    assert_null(lys_parse_mem(ctx,
        "module a {"
            "namespace urn:a;"
            "prefix a;"
            "leaf val {"
                "type int64;"
                "when \"../val='25'\";"
            "}"
        "}"
    , LYS_IN_YANG));
    logbuf_assert("When condition of \"val\" is accessing its own conditional node. /a:val");

    assert_null(lys_parse_mem(ctx,
        "module a {"
            "namespace urn:a;"
            "prefix a;"
            "grouping grp {"
                "leaf val {"
                    "type int64;"
                "}"
            "}"
            "uses grp {"
                "when \"val='25'\";"
            "}"
        "}"
    , LYS_IN_YANG));
    logbuf_assert("When condition of \"val\" is accessing its own conditional node. /a:val");

    assert_null(lys_parse_mem(ctx,
        "module a {"
            "namespace urn:a;"
            "prefix a;"
            "augment /cont {"
                "when \"val='25'\";"
                "leaf val {"
                    "type int64;"
                "}"
            "}"
            "container cont;"
        "}"
    , LYS_IN_YANG));
    logbuf_assert("When condition of \"val\" is accessing its own conditional node. /a:cont/val");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_module, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_feature, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_identity, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_length, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_range, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_pattern, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_enum, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_bits, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_dec64, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_instanceid, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_identityref, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_leafref, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_empty, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_union, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_type_dflt, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_status, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_node_container, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_node_leaflist, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_node_list, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_node_choice, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_node_anydata, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_action, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_notification, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_grouping, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_uses, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_refine, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_augment, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_deviation, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_when, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
