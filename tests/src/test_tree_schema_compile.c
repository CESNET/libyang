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

#include "../../src/common.c"
#include "../../src/log.c"
#include "../../src/set.c"
#include "../../src/xpath.c"
#include "../../src/parser_yang.c"
#include "../../src/tree_schema_helpers.c"
#include "../../src/tree_schema_free.c"
#include "../../src/tree_schema_compile.c"
#include "../../src/tree_schema.c"
#include "../../src/context.c"
#include "../../src/hash_table.c"

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
test_module(void **state)
{
    (void) state; /* unused */

    const char *str;
    struct ly_parser_ctx ctx = {0};
    struct lys_module mod = {0};
    struct lysc_feature *f;
    struct lysc_iffeature *iff;

    str = "module test {namespace urn:test; prefix t;"
          "feature f1;feature f2 {if-feature f1;}}";
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));

    assert_int_equal(LY_EINVAL, lys_compile(NULL, 0));
    logbuf_assert("Invalid argument mod (lys_compile()).");
    assert_int_equal(LY_EINVAL, lys_compile(&mod, 0));
    logbuf_assert("Invalid argument mod->parsed (lys_compile()).");
    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, str, &mod.parsed));
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
    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, str, &mod.parsed));
    assert_int_equal(LY_EINVAL, lys_compile(&mod, 0));
    logbuf_assert("Submodules (test) are not supposed to be compiled, compile only the main modules.");
    assert_null(mod.compiled);
    lysp_module_free(mod.parsed);

    /* data definition name collision in top level */
    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module aa {namespace urn:aa;prefix aa;"
                                                  "leaf a {type string;} container a{presence x;}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Duplicate identifier \"a\" of data definition statement.");
    assert_null(mod.compiled);
    lysp_module_free(mod.parsed);

    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_feature(void **state)
{
    *state = test_feature;

    struct ly_parser_ctx ctx = {0};
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

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, str, &mod.parsed));
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
    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f{if-feature f1;}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f1\" of if-feature - unable to find feature \"f1\".");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature 'f and';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f and\" of if-feature - unexpected end of expression.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f{if-feature 'or';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"or\" of if-feature - unexpected end of expression.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature '(f1';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"(f1\" of if-feature - non-matching opening and closing parentheses.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature 'f1)';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"f1)\" of if-feature - non-matching opening and closing parentheses.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{yang-version 1.1;namespace urn:b; prefix b; feature f1; feature f2{if-feature ---;}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"---\" of if-feature - unable to find feature \"---\".");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{namespace urn:b; prefix b; feature f1; feature f2{if-feature 'not f1';}}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Invalid value \"not f1\" of if-feature - YANG 1.1 expression in YANG 1.0 module.");
    lysp_module_free(mod.parsed);

    assert_int_equal(LY_SUCCESS, yang_parse(&ctx, "module b{namespace urn:b; prefix b; feature f1; feature f1;}", &mod.parsed));
    assert_int_equal(LY_EVALID, lys_compile(&mod, 0));
    logbuf_assert("Duplicate identifier \"f1\" of feature statement.");
    lysp_module_free(mod.parsed);

    ly_ctx_set_module_imp_clb(ctx.ctx, test_imp_clb, "submodule sb {belongs-to b {prefix b;} feature f1;}");
    assert_non_null(modp = lys_parse_mem(ctx.ctx, "module b{namespace urn:b; prefix b; include sb;feature f1;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(modp, 0));
    logbuf_assert("Duplicate identifier \"f1\" of feature statement.");

    /* import reference */
    assert_non_null(modp = lys_parse_mem(ctx.ctx, str, LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(modp, 0));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(modp, "f1"));
    assert_non_null(modp = lys_parse_mem(ctx.ctx, "module c{namespace urn:c; prefix c; import a {prefix a;} feature f1; feature f2{if-feature 'a:f1';}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(modp, 0));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(modp, "f2"));
    assert_int_equal(0, lys_feature_value(modp, "f1"));
    assert_int_equal(1, lys_feature_value(modp, "f2"));

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_identity(void **state)
{
    *state = test_identity;

    struct ly_ctx *ctx;
    struct lys_module *mod1, *mod2;
    const char *mod1_str = "module a {namespace urn:a;prefix a; identity a1;}";
    const char *mod2_str = "module b {yang-version 1.1;namespace urn:b;prefix b; import a {prefix a;}identity b1; identity b2; identity b3 {base b1; base b:b2; base a:a1;} identity b4 {base b:b1; base b3;}}";

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));
    assert_non_null(mod1 = lys_parse_mem(ctx, mod1_str, LYS_IN_YANG));
    assert_non_null(mod2 = lys_parse_mem(ctx, mod2_str, LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod1, 0));
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

    assert_non_null(mod1 = lys_parse_mem(ctx, "module c{namespace urn:c; prefix c; identity i1;identity i1;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod1, 0));
    logbuf_assert("Duplicate identifier \"i1\" of identity statement.");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule sd {belongs-to d {prefix d;} identity i1;}");
    assert_non_null(mod1 = lys_parse_mem(ctx, "module d{namespace urn:d; prefix d; include sd;identity i1;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod1, 0));
    logbuf_assert("Duplicate identifier \"i1\" of identity statement.");

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

static void
test_node_leaflist(void **state)
{
    *state = test_node_leaflist;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_type *type;
    struct lysc_node_leaflist *ll;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;"
                                        "typedef mytype {type union {type leafref {path ../target;} type string;}}"
                                        "leaf-list ll1 {type union {type decimal64 {fraction-digits 2;} type mytype;}}"
                                        "leaf-list ll2 {type leafref {path ../target;}}"
                                        "leaf target {type int8;}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_int_equal(0, ll->min);
    assert_int_equal((uint32_t)-1, ll->max);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {yang-version 1.1;namespace urn:c;prefix c;typedef mytype {type int8;default 10;}"
                                        "leaf-list ll1 {type mytype;default 1; default 1; config false;}"
                                        "leaf-list ll2 {type mytype; ordered-by user;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_non_null(ll->dflts);
    assert_int_equal(3, ll->type->refcount);
    assert_int_equal(2, LY_ARRAY_SIZE(ll->dflts));
    assert_string_equal("1", ll->dflts[0]);
    assert_string_equal("1", ll->dflts[1]);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, ll->flags);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data->next));
    assert_non_null(ll->dflts);
    assert_int_equal(3, ll->type->refcount);
    assert_int_equal(1, LY_ARRAY_SIZE(ll->dflts));
    assert_string_equal("10", ll->dflts[0]);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, ll->flags);

    /* ordered-by is ignored for state data, RPC/action output parameters and notification content
     * TODO test also, RPC output parameters and notification content */
    assert_non_null(mod = lys_parse_mem(ctx, "module d {yang-version 1.1;namespace urn:d;prefix d;"
                                        "leaf-list ll {config false; type string; ordered-by user;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    /* but warning is present: */
    logbuf_assert("The ordered-by statement is ignored in lists representing state data, RPC/action output parameters or notification content ().");
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, ll->flags);

    /* invalid */
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;leaf-list ll {type empty;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {yang-version 1.1;namespace urn:bb;prefix bb;leaf-list ll {type empty; default x;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Leaf-list of type \"empty\" must not have a default value (x).");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;"
                                        "leaf-list ll {config false;type string; default one;default two;default one;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist*)mod->compiled->data));
    assert_non_null(ll->dflts);
    assert_int_equal(3, LY_ARRAY_SIZE(ll->dflts));
    assert_non_null(mod = lys_parse_mem(ctx, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;"
                                        "leaf-list ll {type string; default one;default two;default one;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Configuration leaf-list has multiple defaults of the same value \"one\".");

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

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;feature f;"
                                        "list l1 {key \"x y\"; ordered-by user; leaf x {type string; when 1;}leaf y{type string;if-feature f;}}"
                                        "list l2 {config false;leaf value {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    list = (struct lysc_node_list*)mod->compiled->data;
    assert_non_null(list);
    assert_non_null(list->keys);
    assert_int_equal(2, LY_ARRAY_SIZE(list->keys));
    assert_string_equal("x", list->keys[0]->name);
    assert_string_equal("y", list->keys[1]->name);
    assert_non_null(list->child);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, list->flags);
    assert_true(list->child->flags & LYS_KEY);
    assert_true(list->child->next->flags & LYS_KEY);
    list = (struct lysc_node_list*)mod->compiled->data->next;
    assert_non_null(list);
    assert_null(list->keys);
    assert_non_null(list->child);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, list->flags);
    assert_false(list->child->flags & LYS_KEY);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;"
                                        "list l {key a; unique \"a c/b:b\"; unique \"c/e d\";"
                                        "leaf a {type string; default x;} leaf d {type string;config false;}"
                                        "container c {leaf b {type string;}leaf e{type string;config false;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    list = (struct lysc_node_list*)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_non_null(list->keys);
    assert_int_equal(1, LY_ARRAY_SIZE(list->keys));
    assert_string_equal("a", list->keys[0]->name);
    assert_true(list->keys[0]->flags & LYS_KEY);
    assert_null(list->keys[0]->dflt);
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    list = (struct lysc_node_list*)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_non_null(list->keys);
    assert_int_equal(1, LY_ARRAY_SIZE(list->keys));
    assert_string_equal("a", list->keys[0]->name);
    assert_true(list->keys[0]->flags & LYS_KEY);
    assert_int_equal(LY_TYPE_EMPTY, list->keys[0]->type->basetype);

    /* invalid */
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;list l;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing key in list representing configuration data.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {yang-version 1.1; namespace urn:bb;prefix bb;"
                                        "list l {key x; leaf x {type string; when 1;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("List's key \"x\" must not have any \"when\" statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;feature f;"
                                        "list l {key x; leaf x {type string; if-feature f;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("List's key \"x\" must not have any \"if-feature\" statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;"
                                        "list l {key x; leaf x {type string; config false;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Key of the configuration list must not be status leaf.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;"
                                        "list l {config false;key x; leaf x {type string; config true;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Configuration node cannot be child of any state data node.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;"
                                        "list l {key x; leaf-list x {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("The list's key \"x\" not found.");

    assert_non_null(mod = lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;"
                                        "list l {key x; unique y;leaf x {type string;} leaf-list y {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Unique's descendant-schema-nodeid \"y\" refers to a leaf-list node instead of a leaf.");

    assert_non_null(mod = lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;"
                                        "list l {key x; unique \"x y\";leaf x {type string;} leaf y {config false; type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Unique statement \"x y\" refers to leafs with different config type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;"
                                        "list l {key x; unique a:x;leaf x {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid descendant-schema-nodeid value \"a:x\" - prefix \"a\" not defined in module \"ii\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;"
                                        "list l {key x; unique c/x;leaf x {type string;}container c {leaf y {type string;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid descendant-schema-nodeid value \"c/x\" - target node not found.");

    assert_non_null(mod = lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;"
                                        "list l {key x; unique c^y;leaf x {type string;}container c {leaf y {type string;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid descendant-schema-nodeid value \"c^\" - missing \"/\" as node-identifier separator.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;"
                                        "list l {key \"x y x\";leaf x {type string;}leaf y {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Duplicated key identifier \"x\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module mm {namespace urn:mm;prefix mm;"
                                        "list l {key x;leaf x {type empty;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Key of a list can be of type \"empty\" only in YANG 1.1 modules.");

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
                                        "choice ch {default a:b; case a {leaf a1 {type string;}leaf a2 {type string;}}"
                                        "leaf b {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    ch = (struct lysc_node_choice*)mod->compiled->data;
    assert_non_null(ch);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR, ch->flags);
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

    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                                        "choice ch {case a {leaf x {type string;}}leaf x {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Duplicate identifier \"x\" of data definition statement.");
    assert_non_null(mod = lys_parse_mem(ctx, "module aa2 {namespace urn:aa2;prefix aa;"
                                        "choice ch {case a {leaf y {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Duplicate identifier \"y\" of data definition statement.");
    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;"
                                        "choice ch {case a {leaf x {type string;}}leaf a {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Duplicate identifier \"a\" of case statement.");
    assert_non_null(mod = lys_parse_mem(ctx, "module bb2 {namespace urn:bb2;prefix bb;"
                                        "choice ch {case b {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Duplicate identifier \"b\" of case statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ca {namespace urn:ca;prefix ca;"
                                        "choice ch {default c;case a {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Default case \"c\" not found.");
    assert_non_null(mod = lys_parse_mem(ctx, "module cb {namespace urn:cb;prefix cb; import a {prefix a;}"
                                        "choice ch {default a:a;case a {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid default case referencing a case from different YANG module (by prefix \"a\").");
    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;"
                                        "choice ch {default a;case a {leaf x {mandatory true;type string;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Mandatory node \"x\" under the default case \"a\".");
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    any = (struct lysc_node_anydata*)mod->compiled->data;
    assert_non_null(any);
    assert_int_equal(LYS_ANYDATA, any->nodetype);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_MAND_TRUE, any->flags);

    logbuf_clean();
    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;"
                                        "anyxml any;}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    any = (struct lysc_node_anydata*)mod->compiled->data;
    assert_non_null(any);
    assert_int_equal(LYS_ANYXML, any->nodetype);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR, any->flags);
    logbuf_assert("Use of anyxml to define configuration data is not recommended."); /* warning */

    /* invalid */
    assert_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;anydata any;}", LYS_IN_YANG));
    logbuf_assert("Invalid keyword \"anydata\" as a child of \"module\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount);
    assert_int_equal(LY_TYPE_UINT8, type->basetype);
    assert_non_null(((struct lysc_type_num*)type)->range);
    assert_non_null(((struct lysc_type_num*)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_num*)type)->range->parts));

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c;leaf l {type binary {length min..max;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(0, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d;leaf l {type binary {length 5;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(5, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(5, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module e {namespace urn:e;prefix e;leaf l {type binary {length 1..10;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(1, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module f {namespace urn:f;prefix f;leaf l {type binary {length 1..10|20..30;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module i {namespace urn:i;prefix i;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length \"50\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin*)type)->length);
    assert_non_null(((struct lysc_type_bin*)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_bin*)type)->length->parts));
    assert_int_equal(50, ((struct lysc_type_bin*)type)->length->parts[0].min_u64);
    assert_int_equal(50, ((struct lysc_type_bin*)type)->length->parts[0].max_u64);

    assert_non_null(mod = lys_parse_mem(ctx, "module j {namespace urn:j;prefix j;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length \"10..30|60..100\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;leaf l {type binary {length -10;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - value \"-10\" does not fit the type limitations.");
    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;leaf l {type binary {length 18446744073709551616;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - invalid value \"18446744073709551616\".");
    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;leaf l {type binary {length \"max .. 10\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - unexpected data after max keyword (.. 10).");
    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;leaf l {type binary {length 50..10;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - values are not in ascending order (10).");
    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;leaf l {type binary {length \"50 | 10\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - values are not in ascending order (10).");
    assert_non_null(mod = lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;leaf l {type binary {length \"x\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - unexpected data (x).");
    assert_non_null(mod = lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;leaf l {type binary {length \"50 | min\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - unexpected data before min keyword (50 | ).");
    assert_non_null(mod = lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;leaf l {type binary {length \"| 50\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - unexpected beginning of the expression (| 50).");
    assert_non_null(mod = lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;leaf l {type binary {length \"10 ..\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - unexpected end of the expression after \"..\" (10 ..).");
    assert_non_null(mod = lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;leaf l {type binary {length \".. 10\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - unexpected \"..\" without a lower bound.");
    assert_non_null(mod = lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;leaf l {type binary {length \"10 |\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - unexpected end of the expression (10 |).");
    assert_non_null(mod = lys_parse_mem(ctx, "module kl {namespace urn:kl;prefix kl;leaf l {type binary {length \"10..20 | 15..30\";}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - values are not in ascending order (15).");

    assert_non_null(mod = lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;typedef mytype {type binary {length 10;}}"
                                             "leaf l {type mytype {length 11;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - the derived restriction (11) is not equally or more limiting.");
    assert_non_null(mod = lys_parse_mem(ctx, "module mm {namespace urn:mm;prefix mm;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length 1..11;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - the derived restriction (1..11) is not equally or more limiting.");
    assert_non_null(mod = lys_parse_mem(ctx, "module nn {namespace urn:nn;prefix nn;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length 20..110;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - the derived restriction (20..110) is not equally or more limiting.");
    assert_non_null(mod = lys_parse_mem(ctx, "module oo {namespace urn:oo;prefix oo;typedef mytype {type binary {length 10..100;}}"
                                             "leaf l {type mytype {length 20..30|110..120;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - the derived restriction (20..30|110..120) is not equally or more limiting.");
    assert_non_null(mod = lys_parse_mem(ctx, "module pp {namespace urn:pp;prefix pp;typedef mytype {type binary {length 10..11;}}"
                                             "leaf l {type mytype {length 15;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - the derived restriction (15) is not equally or more limiting.");
    assert_non_null(mod = lys_parse_mem(ctx, "module qq {namespace urn:qq;prefix qq;typedef mytype {type binary {length 10..20|30..40;}}"
                                             "leaf l {type mytype {length 15..35;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - the derived restriction (15..35) is not equally or more limiting.");
    assert_non_null(mod = lys_parse_mem(ctx, "module rr {namespace urn:rr;prefix rr;typedef mytype {type binary {length 10;}}"
                                             "leaf l {type mytype {length 10..35;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid length restriction - the derived restriction (10..35) is not equally or more limiting.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ss {namespace urn:rr;prefix rr;leaf l {type binary {pattern '[0-9]*';}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid type restrictions for binary type.");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
    assert_string_equal("errortag", ((struct lysc_type_str*)type)->patterns[0]->eapptag);
    assert_string_equal("error", ((struct lysc_type_str*)type)->patterns[0]->emsg);
    assert_int_equal(0, ((struct lysc_type_str*)type)->patterns[0]->inverted);
    assert_null(((struct lysc_type_str*)type)->patterns[1]->eapptag);
    assert_null(((struct lysc_type_str*)type)->patterns[1]->emsg);
    assert_int_equal(1, ((struct lysc_type_str*)type)->patterns[1]->inverted);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;typedef mytype {type string {pattern '[0-9]*';}}"
                                             "typedef mytype2 {type mytype {length 10;}} leaf l {type mytype2 {pattern '[0-4]*';}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
    assert_int_equal(3, ((struct lysc_type_str*)type)->patterns[0]->refcount);
    assert_int_equal(1, ((struct lysc_type_str*)type)->patterns[1]->refcount);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c;typedef mytype {type string {pattern '[0-9]*';}}"
                                             "leaf l {type mytype {length 10;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
    assert_int_equal(2, ((struct lysc_type_str*)type)->patterns[0]->refcount);

    /* test substitutions */
    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d;leaf l {type string {"
                                        "pattern '^\\p{IsLatinExtended-A}$';}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_str*)type)->patterns);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_type_str*)type)->patterns));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; leaf l {type enumeration;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing enum substatement for enumeration type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;typedef mytype {type enumeration {enum one;}}"
                                             "leaf l {type mytype {enum two;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid enumeration - derived type adds new item \"two\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;typedef mytype {type enumeration {enum one;}}"
                                             "leaf l {type mytype {enum one {value 1;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid enumeration - value of the item \"one\" has changed from 0 to 1 in the derived type.");
    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;leaf l {type enumeration {enum x {value 2147483647;}enum y;}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid enumeration - it is not possible to auto-assign enum value for \"y\" since the highest value is already 2147483647.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;leaf l {type enumeration {enum x {value 1;}enum y {value 1;}}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid enumeration - value 1 collide in items \"y\" and \"x\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;typedef mytype {type enumeration;}"
                                             "leaf l {type mytype {enum one;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing enum substatement for enumeration type mytype.");

    assert_non_null(mod = lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh; typedef mytype {type enumeration {enum one;}}"
                                        "leaf l {type mytype {enum one;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Enumeration type can be subtyped only in YANG 1.1 modules.");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
                                        "bit automin; bit one;bit two; bit seven {value 7;}bit eight;}} leaf l { type mytype {bit eight;bit seven;bit automin;}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
                                   "bit one {value 4294967296;}}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"4294967296\" of \"value\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit one; bit one;}}}", LYS_IN_YANG));
    logbuf_assert("Duplicate identifier \"one\" of bit statement. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit '11';}}}", LYS_IN_YANG));
    logbuf_assert("Invalid identifier first character '1'. Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
                                   "bit 'x1$1';}}}", LYS_IN_YANG));
    logbuf_assert("Invalid identifier character '$'. Line number 1.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; leaf l {type bits;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing bit substatement for bits type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;typedef mytype {type bits {bit one;}}"
                                             "leaf l {type mytype {bit two;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid bits - derived type adds new item \"two\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;typedef mytype {type bits {bit one;}}"
                                             "leaf l {type mytype {bit one {position 1;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid bits - position of the item \"one\" has changed from 0 to 1 in the derived type.");
    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;leaf l {type bits {bit x {position 4294967295;}bit y;}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid bits - it is not possible to auto-assign bit position for \"y\" since the highest value is already 4294967295.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;leaf l {type bits {bit x {value 1;}bit y {value 1;}}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid bits - position 1 collide in items \"y\" and \"x\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;typedef mytype {type bits;}"
                                             "leaf l {type mytype {bit one;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing bit substatement for bits type mytype.");

    assert_non_null(mod = lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh; typedef mytype {type bits {bit one;}}"
                                        "leaf l {type mytype {bit one;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Bits type can be subtyped only in YANG 1.1 modules.");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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

    /* invalid cases */
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits 0;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"0\" of \"fraction-digits\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits -1;}}}", LYS_IN_YANG));
    logbuf_assert("Invalid value \"-1\" of \"fraction-digits\". Line number 1.");
    assert_null(lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits 19;}}}", LYS_IN_YANG));
    logbuf_assert("Value \"19\" is out of \"fraction-digits\" bounds. Line number 1.");

    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing fraction-digits substatement for decimal64 type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ab {namespace urn:ab;prefix ab; typedef mytype {type decimal64;}leaf l {type mytype;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing fraction-digits substatement for decimal64 type mytype.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; leaf l {type decimal64 {fraction-digits 2;"
                                        "range '3.142';}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Range boundary \"3.142\" of decimal64 type exceeds defined number (2) of fraction digits.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc; leaf l {type decimal64 {fraction-digits 2;"
                                        "range '4 | 3.14';}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid range restriction - values are not in ascending order (3.14).");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd; typedef mytype {type decimal64 {fraction-digits 2;}}"
                                        "leaf l {type mytype {fraction-digits 3;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid fraction-digits substatement for type not directly derived from decimal64 built-in type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module de {namespace urn:de;prefix de; typedef mytype {type decimal64 {fraction-digits 2;}}"
                                        "typedef mytype2 {type mytype {fraction-digits 3;}}leaf l {type mytype2;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid fraction-digits substatement for type \"mytype2\" not directly derived from decimal64 built-in type.");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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

    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type instance-identifier {fraction-digits 1;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid type restrictions for instance-identifier type.");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_IDENT, type->basetype);
    assert_non_null(((struct lysc_type_identityref*)type)->bases);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_type_identityref*)type)->bases));
    assert_string_equal("k", ((struct lysc_type_identityref*)type)->bases[0]->name);
    assert_string_equal("j", ((struct lysc_type_identityref*)type)->bases[1]->name);

    /* invalid cases */
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa; leaf l {type identityref;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing base substatement for identityref type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb; typedef mytype {type identityref;}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing base substatement for identityref type mytype.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc; identity i; typedef mytype {type identityref {base i;}}"
                                        "leaf l {type mytype {base i;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid base substatement for the type not directly derived from identityref built-in type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd; identity i; typedef mytype {type identityref {base i;}}"
                                        "typedef mytype2 {type mytype {base i;}}leaf l {type mytype2;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid base substatement for the type \"mytype2\" not directly derived from identityref built-in type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee; identity i; identity j;"
                                        "leaf l {type identityref {base i;base j;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Multiple bases in identityref type are allowed only in YANG 1.1 modules.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff; identity i;leaf l {type identityref {base j;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Unable to find base (j) of identityref.");

    assert_non_null(mod = lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;leaf l {type identityref {base x:j;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid prefix used for base (x:j) of identityref.");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)(*lysc_node_children_p(mod->compiled->data->prev))->prev)->type;
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/endpoint-parent[id=current()/../field]/endpoint/name", ((struct lysc_type_leafref* )type)->path);
    assert_ptr_equal(mod, ((struct lysc_type_leafref*)type)->path_context);
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref*)type)->realtype->basetype);

    /* invalid paths */
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path ../a/invalid;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path - unable to find \"../a/invalid\".");
    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path ../../toohigh;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path \"../../toohigh\" - too many \"..\" in the path.");
    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path /a:invalid;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path - unable to find module connected with the prefix of the node \"/a:invalid\".");
    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;leaf target1 {type string;}container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path '/a[target2 = current()/../target1]/target2';}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path - node \"/a\" is expected to be a list, but it is container.");
    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path /a!invalid;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path at character 3 (/a!invalid).");
    assert_non_null(mod = lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;container a {leaf target2 {type uint8;}}"
                                        "leaf ref1 {type leafref {path /a;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path \"/a\" - target node is container instead of leaf or leaf-list.");
    assert_non_null(mod = lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;container a {leaf target2 {type uint8; status deprecated;}}"
                                        "leaf ref1 {type leafref {path /a/target2;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("A current definition \"ref1\" is not allowed to reference deprecated definition \"target2\".");
    assert_non_null(mod = lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;"
                                        "leaf ref1 {type leafref;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing path substatement for leafref type.");
    assert_non_null(mod = lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;typedef mytype {type leafref;}"
                                        "leaf ref1 {type mytype;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing path substatement for leafref type mytype.");
    assert_non_null(mod = lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;feature f;"
                                        "leaf ref {type leafref {path /target;}}leaf target {if-feature f;type string;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path \"/target\" - set of features applicable to the leafref target is not a subset of features "
                  "applicable to the leafref itself.");
    assert_non_null(mod = lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;"
                                        "leaf ref {type leafref {path /target;}}leaf target {type string;config false;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path \"/target\" - target is supposed to represent configuration data (as the leafref does), but it does not.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;"
                                        "leaf ref {type leafref {path /target; require-instance true;}}leaf target {type string;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Leafref type can be restricted by require-instance statement only in YANG 1.1 modules.");
    assert_non_null(mod = lys_parse_mem(ctx, "module mm {namespace urn:mm;prefix mm;typedef mytype {type leafref {path /target;require-instance false;}}"
                                        "leaf ref {type mytype;}leaf target {type string;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Leafref type \"mytype\" can be restricted by require-instance statement only in YANG 1.1 modules.");

    assert_non_null(mod = lys_parse_mem(ctx, "module nn {namespace urn:nn;prefix nn;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name is current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name i\" - missing \"=\" after node-identifier.");

    assert_non_null(mod = lys_parse_mem(ctx, "module oo {namespace urn:oo;prefix oo;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../ifname/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../ifname/ip\" - missing predicate termination.");

    assert_non_null(mod = lys_parse_mem(ctx, "module pp {namespace urn:pp;prefix pp;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[x:name=current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[x:name=current()/../ifname]\" - prefix \"x\" not defined in module \"pp\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module qq {namespace urn:qq;prefix qq;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[id=current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[id=current()/../ifname]\" - predicate's key node \"id\" not found.");

    assert_non_null(mod = lys_parse_mem(ctx, "module rr {namespace urn:rr;prefix rr;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name=current() /  .. / ifname][name=current()/../test]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../test]\" - multiple equality tests for the key \"name\".");

    assert_non_null(mod = lys_parse_mem(ctx, "module ss {namespace urn:ss;prefix ss;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = ../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name = ../ifname]\" - missing current-function-invocation.");

    assert_non_null(mod = lys_parse_mem(ctx, "module tt {namespace urn:tt;prefix tt;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name = current()../ifname]\" - missing \"/\" after current-function-invocation.");

    assert_non_null(mod = lys_parse_mem(ctx, "module uu {namespace urn:uu;prefix uu;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/..ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name = current()/..ifname]\" - missing \"/\" in \"../\" rel-path-keyexpr pattern.");

    assert_non_null(mod = lys_parse_mem(ctx, "module vv {namespace urn:vv;prefix vv;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name = current()/ifname]\" - at least one \"..\" is expected in rel-path-keyexpr.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ww {namespace urn:ww;prefix ww;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/../]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name = current()/../]\" - at least one node-identifier is expected in rel-path-keyexpr.");

    assert_non_null(mod = lys_parse_mem(ctx, "module xx {namespace urn:xx;prefix xx;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}"
                                        "leaf address {type leafref{ path \"/interface[name = current()/../$node]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid node identifier in leafref path predicate - character 22 (of [name = current()/../$node]).");

    assert_non_null(mod = lys_parse_mem(ctx, "module yy {namespace urn:yy;prefix yy;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../x:ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../x:ifname]\" - unable to find module of the node \"ifname\" in rel-path_keyexpr.");

    assert_non_null(mod = lys_parse_mem(ctx, "module zz {namespace urn:zz;prefix zz;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../xxx]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../xxx]\" - unable to find node \"current()/../xxx\" in the rel-path_keyexpr.");

    assert_non_null(mod = lys_parse_mem(ctx, "module zza {namespace urn:zza;prefix zza;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}container c;"
                                        "leaf address {type leafref{ path \"/interface[name=current()/../c]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[name=current()/../c]\" - rel-path_keyexpr \"current()/../c\" refers container instead of leaf.");

    assert_non_null(mod = lys_parse_mem(ctx, "module zzb {namespace urn:zzb;prefix zzb;"
                                        "list interface{key name;leaf name{type string;}leaf ip {type string;}container c;}"
                                        "leaf ifname{type leafref{ path \"../interface/name\";}}"
                                        "leaf address {type leafref{ path \"/interface[c=current()/../ifname]/ip\";}}}",
                                        LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path predicate \"[c=current()/../ifname]\" - predicate's key node \"c\" not found.");

    /* circular chain */
    assert_non_null(mod = lys_parse_mem(ctx, "module aaa {namespace urn:aaa;prefix aaa;"
                                        "leaf ref1 {type leafref {path /ref2;}}"
                                        "leaf ref2 {type leafref {path /ref3;}}"
                                        "leaf ref3 {type leafref {path /ref4;}}"
                                        "leaf ref4 {type leafref {path /ref1;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid leafref path \"/ref1\" - circular chain of leafrefs detected.");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_type_empty(void **state)
{
    *state = test_type_empty;

    struct ly_ctx *ctx;
    struct lys_module *mod;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    /* invalid */
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                                        "leaf l {type empty; default x;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Leaf of type \"empty\" must not have a default value (x).");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;typedef mytype {type empty; default x;}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid type \"mytype\" - \"empty\" type must not have a default value (x).");

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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;typedef mytype {type union;}"
                                        "leaf l {type mytype;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing type substatement for union type mytype.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;leaf l {type union;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Missing type substatement for union type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;typedef mytype {type union{type int8; type string;}}"
                                        "leaf l {type mytype {type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid type substatement for the type not directly derived from union built-in type.");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;typedef mytype {type union{type int8; type string;}}"
                                        "typedef mytype2 {type mytype {type string;}}leaf l {type mytype2;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid type substatement for the type \"mytype2\" not directly derived from union built-in type.");

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

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    /* default is not inherited from union's types */
    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a; typedef mybasetype {type string;default hello;units xxx;}"
                                        "leaf l {type union {type decimal64 {fraction-digits 2;} type mybasetype;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(2, type->refcount);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_string_equal("hello", ((struct lysc_node_leaf*)mod->compiled->data)->dflt);
    assert_string_equal("xxx", ((struct lysc_node_leaf*)mod->compiled->data)->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:c;prefix c; typedef mybasetype {type string;default hello;units xxx;}"
                                        "leaf l {type mybasetype; default goodbye;units yyy;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(2, type->refcount);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_string_equal("goodbye", ((struct lysc_node_leaf*)mod->compiled->data)->dflt);
    assert_string_equal("yyy", ((struct lysc_node_leaf*)mod->compiled->data)->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module d {namespace urn:d;prefix d; typedef mybasetype {type string;default hello;units xxx;}"
                                        "typedef mytype {type mybasetype;}leaf l1 {type mytype; default goodbye;units yyy;}"
                                        "leaf l2 {type mytype;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(4, type->refcount);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_string_equal("goodbye", ((struct lysc_node_leaf*)mod->compiled->data)->dflt);
    assert_string_equal("yyy", ((struct lysc_node_leaf*)mod->compiled->data)->units);
    type = ((struct lysc_node_leaf*)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(4, type->refcount);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_string_equal("hello", ((struct lysc_node_leaf*)mod->compiled->data->next)->dflt);
    assert_string_equal("xxx", ((struct lysc_node_leaf*)mod->compiled->data->next)->units);

    assert_non_null(mod = lys_parse_mem(ctx, "module e {namespace urn:e;prefix e; typedef mybasetype {type string;}"
                                        "typedef mytype {type mybasetype; default hello;units xxx;}leaf l {type mytype;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_string_equal("hello", ((struct lysc_node_leaf*)mod->compiled->data)->dflt);
    assert_string_equal("xxx", ((struct lysc_node_leaf*)mod->compiled->data)->units);

    /* mandatory leaf does not takes default value from type */
    assert_non_null(mod = lys_parse_mem(ctx, "module f {namespace urn:f;prefix f;typedef mytype {type string; default hello;units xxx;}"
                                        "leaf l {type mytype; mandatory true;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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
    struct lys_module *mod;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;"
                                        "container c {status deprecated; leaf l {status current; type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("A \"current\" status is in conflict with the parent's \"deprecated\" status.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;"
                                        "container c {status obsolete; leaf l {status current; type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("A \"current\" status is in conflict with the parent's \"obsolete\" status.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;"
                                        "container c {status obsolete; leaf l {status deprecated; type string;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status.");

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

static void
test_uses(void **state)
{
    *state = test_uses;

    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lysc_node *parent, *child;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module grp {namespace urn:grp;prefix g; typedef mytype {type string;}"
                                        "grouping grp {leaf x {type mytype;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));


    assert_non_null(mod = lys_parse_mem(ctx, "module a {namespace urn:a;prefix a;import grp {prefix g;}"
                                        "grouping grp_a_top {leaf a1 {type int8;}}"
                                        "container a {uses grp_a; uses grp_a_top; uses g:grp; grouping grp_a {leaf a2 {type uint8;}}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
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

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule bsub {belongs-to b {prefix b;} grouping grp {leaf b {type string;}}}");
    assert_non_null(mod = lys_parse_mem(ctx, "module b {namespace urn:b;prefix b;include bsub;uses grp;}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_non_null(mod->compiled->data);
    assert_int_equal(LYS_LEAF, mod->compiled->data->nodetype);
    assert_string_equal("b", mod->compiled->data->name);

    logbuf_clean();
    assert_non_null(mod = lys_parse_mem(ctx, "module c {namespace urn:ii;prefix ii;"
                                        "grouping grp {leaf l {type string;}leaf k {type string; status obsolete;}}"
                                        "uses grp {status deprecated;}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_int_equal(LYS_LEAF, mod->compiled->data->nodetype);
    assert_string_equal("l", mod->compiled->data->name);
    assert_true(LYS_STATUS_DEPRC & mod->compiled->data->flags);
    assert_int_equal(LYS_LEAF, mod->compiled->data->next->nodetype);
    assert_string_equal("k", mod->compiled->data->next->name);
    assert_true(LYS_STATUS_OBSLT & mod->compiled->data->next->flags);
    logbuf_assert(""); /* no warning about inheriting deprecated flag from uses */

    /* invalid */
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;uses missinggrp;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Grouping \"missinggrp\" referenced by a uses statement not found.");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;uses grp;"
                                        "grouping grp {leaf a{type string;}uses grp1;}"
                                        "grouping grp1 {leaf b {type string;}uses grp2;}"
                                        "grouping grp2 {leaf c {type string;}uses grp;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Grouping \"grp\" references itself through a uses statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;uses a:missingprefix;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid prefix used for grouping reference (a:missingprefix).");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;grouping grp{leaf a{type string;}}"
                                        "leaf a {type string;}uses grp;}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Duplicate identifier \"a\" of data definition statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;grouping grp {leaf l {type string; status deprecated;}}"
                                        "uses grp {status obsolete;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status.");

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

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &ctx));

    assert_non_null(mod = lys_parse_mem(ctx, "module grp {yang-version 1.1;namespace urn:grp;prefix g; feature f;typedef mytype {type string; default cheers!;}"
                                        "grouping grp {container c {leaf l {type mytype; default goodbye;}"
                                        "leaf-list ll {type mytype; default goodbye;}"
                                        "choice ch {default a; leaf a {type int8;}leaf b{type uint8;}}"
                                        "leaf x {type mytype; mandatory true; must 1;}"
                                        "anydata a {mandatory false; if-feature f;}"
                                        "container c {config false; leaf l {type string;}}}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));

    assert_non_null(mod = lys_parse_mem(ctx, "module a {yang-version 1.1;namespace urn:a;prefix a;import grp {prefix g;}feature fa;"
                                        "uses g:grp {refine c/l {default hello; config false;}"
                                        "refine c/ll {default hello;default world; min-elements 2; max-elements 5;}"
                                        "refine c/ch {default b;config true; if-feature fa;}"
                                        "refine c/x {mandatory false; must ../ll;}"
                                        "refine c/a {mandatory true; must 1; if-feature fa;}"
                                        "refine c/c {config true;presence indispensable;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_non_null((parent = mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, parent->nodetype);
    assert_string_equal("c", parent->name);
    assert_non_null((child = ((struct lysc_node_container*)parent)->child));
    assert_int_equal(LYS_LEAF, child->nodetype);
    assert_string_equal("l", child->name);
    assert_string_equal("hello", ((struct lysc_node_leaf*)child)->dflt);
    assert_int_equal(LYS_CONFIG_R, child->flags & LYS_CONFIG_MASK);
    assert_non_null(child = child->next);
    assert_int_equal(LYS_LEAFLIST, child->nodetype);
    assert_string_equal("ll", child->name);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_node_leaflist*)child)->dflts));
    assert_string_equal("hello", ((struct lysc_node_leaflist*)child)->dflts[0]);
    assert_string_equal("world", ((struct lysc_node_leaflist*)child)->dflts[1]);
    assert_int_equal(2, ((struct lysc_node_leaflist*)child)->min);
    assert_int_equal(5, ((struct lysc_node_leaflist*)child)->max);
    assert_non_null(child = child->next);
    assert_int_equal(LYS_CHOICE, child->nodetype);
    assert_string_equal("ch", child->name);
    assert_string_equal("b", ((struct lysc_node_choice*)child)->dflt->name);
    assert_true(LYS_SET_DFLT & ((struct lysc_node_choice*)child)->dflt->flags);
    assert_false(LYS_SET_DFLT & ((struct lysc_node_choice*)child)->cases[0].flags);
    assert_non_null(child->iffeatures);
    assert_int_equal(1, LY_ARRAY_SIZE(child->iffeatures));
    assert_non_null(child = child->next);
    assert_int_equal(LYS_LEAF, child->nodetype);
    assert_string_equal("x", child->name);
    assert_false(LYS_MAND_TRUE & child->flags);
    assert_string_equal("cheers!", ((struct lysc_node_leaf*)child)->dflt);
    assert_non_null(((struct lysc_node_leaf*)child)->musts);
    assert_int_equal(2, LY_ARRAY_SIZE(((struct lysc_node_leaf*)child)->musts));
    assert_non_null(child = child->next);
    assert_int_equal(LYS_ANYDATA, child->nodetype);
    assert_string_equal("a", child->name);
    assert_true(LYS_MAND_TRUE & child->flags);
    assert_non_null(((struct lysc_node_anydata*)child)->musts);
    assert_int_equal(1, LY_ARRAY_SIZE(((struct lysc_node_anydata*)child)->musts));
    assert_non_null(child->iffeatures);
    assert_int_equal(2, LY_ARRAY_SIZE(child->iffeatures));
    assert_non_null(child = child->next);
    assert_int_equal(LYS_CONTAINER, child->nodetype);
    assert_string_equal("c", child->name);
    assert_true(LYS_PRESENCE & child->flags);
    assert_true(LYS_CONFIG_W & child->flags);
    assert_true(LYS_CONFIG_W & ((struct lysc_node_container*)child)->child->flags);

    assert_non_null(mod = lys_parse_mem(ctx, "module b {yang-version 1.1;namespace urn:b;prefix b;import grp {prefix g;}"
                                        "uses g:grp {status deprecated; refine c/x {default hello; mandatory false;}}}", LYS_IN_YANG));
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0));
    assert_non_null((child = ((struct lysc_node_container*)mod->compiled->data)->child->prev->prev->prev));
    assert_int_equal(LYS_LEAF, child->nodetype);
    assert_string_equal("x", child->name);
    assert_false(LYS_MAND_TRUE & child->flags);
    assert_string_equal("hello", ((struct lysc_node_leaf*)child)->dflt);

    /* invalid */
    assert_non_null(mod = lys_parse_mem(ctx, "module aa {namespace urn:aa;prefix aa;import grp {prefix g;}"
                                        "uses g:grp {refine c {default hello;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of default in \"c\" - container cannot hold default value(s).");

    assert_non_null(mod = lys_parse_mem(ctx, "module bb {namespace urn:bb;prefix bb;import grp {prefix g;}"
                                        "uses g:grp {refine c/l {default hello; default world;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of default in \"c/l\" - leaf cannot hold 2 default values.");

    assert_non_null(mod = lys_parse_mem(ctx, "module cc {namespace urn:cc;prefix cc;import grp {prefix g;}"
                                        "uses g:grp {refine c/ll {default hello; default world;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of default in leaf-list - the default statement is allowed only in YANG 1.1 modules.");

    assert_non_null(mod = lys_parse_mem(ctx, "module dd {namespace urn:dd;prefix dd;import grp {prefix g;}"
                                        "uses g:grp {refine c/ll {mandatory true;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of mandatory in \"c/ll\" - leaf-list cannot hold mandatory statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ee {namespace urn:ee;prefix ee;import grp {prefix g;}"
                                        "uses g:grp {refine c/l {mandatory true;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of mandatory in \"c/l\" - leaf already has \"default\" statement.");
    assert_non_null(mod = lys_parse_mem(ctx, "module ef {namespace urn:ef;prefix ef;import grp {prefix g;}"
                                        "uses g:grp {refine c/ch {mandatory true;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of mandatory in \"c/ch\" - choice already has \"default\" statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ff {namespace urn:ff;prefix ff;import grp {prefix g;}"
                                        "uses g:grp {refine c/ch/a/a {mandatory true;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of mandatory in \"c/ch/a/a\" - leaf under the default case.");

    assert_non_null(mod = lys_parse_mem(ctx, "module gg {namespace urn:gg;prefix gg;import grp {prefix g;}"
                                        "uses g:grp {refine c/x {default hello;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of default in \"c/x\" - the node is mandatory.");

    assert_non_null(mod = lys_parse_mem(ctx, "module hh {namespace urn:hh;prefix hh;import grp {prefix g;}"
                                        "uses g:grp {refine c/c/l {config true;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of config in \"c/c/l\" - configuration node cannot be child of any state data node.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ii {namespace urn:ii;prefix ii;grouping grp {leaf l {type string; status deprecated;}}"
                                        "uses grp {status obsolete;}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status.");

    assert_non_null(mod = lys_parse_mem(ctx, "module jj {namespace urn:jj;prefix jj;import grp {prefix g;}"
                                        "uses g:grp {refine c/x {presence nonsence;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of presence statement in \"c/x\" - leaf cannot hold the presence statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module kk {namespace urn:kk;prefix kk;import grp {prefix g;}"
                                        "uses g:grp {refine c/ch {must 1;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of must statement in \"c/ch\" - choice cannot hold any must statement.");

    assert_non_null(mod = lys_parse_mem(ctx, "module ll {namespace urn:ll;prefix ll;import grp {prefix g;}"
                                        "uses g:grp {refine c/x {min-elements 1;}}}", LYS_IN_YANG));
    assert_int_equal(LY_EVALID, lys_compile(mod, 0));
    logbuf_assert("Invalid refine of min-elements statement in \"c/x\" - leaf cannot hold this statement.");

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
        cmocka_unit_test_setup_teardown(test_uses, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_refine, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
