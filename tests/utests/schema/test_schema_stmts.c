/*
 * @file test_schema_stmts.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for YANG (YIN) statements in (sub)modules
 *
 * Copyright (c) 2018-2020 CESNET, z.s.p.o.
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

#include "log.h"
#include "context.h"
#include "test_schema.h"
#include "tree_schema.h"

#include "test_schema.h"

void
test_identity(void **state)
{
    *state = test_identity;

    struct ly_ctx *ctx;
    const struct lys_module *mod, *mod_imp;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    /*
     * parsing YANG
     */
    TEST_STMT_DUP(ctx, 1, 0, "identity id", "description", "a", "b", "1");
    TEST_STMT_DUP(ctx, 1, 0, "identity id", "reference", "a", "b", "1");
    TEST_STMT_DUP(ctx, 1, 0, "identity id", "status", "current", "obsolete", "1");

    /* full content */
    TEST_SCHEMA_OK(ctx, 1, 0, "identityone",
                   "identity test {base \"a\";base b; description text;reference \'another text\';status current; if-feature x;if-feature y; identityone:ext;}"
                   "identity a; identity b; extension ext; feature x; feature y;", mod);
    assert_non_null(mod->parsed->identities);
    assert_int_equal(3, LY_ARRAY_COUNT(mod->parsed->identities));

    /* invalid substatement */
    TEST_STMT_SUBSTM_ERR(ctx, 0, "identity", "organization", "XXX");

    /*
     * parsing YIN
     */
    /* max subelems */
    TEST_SCHEMA_OK(ctx, 1, 1, "identityone-yin", "<identity name=\"ident-name\">"
                       "<if-feature name=\"iff\"/>"
                       "<base name=\"base-name\"/>"
                       "<status value=\"deprecated\"/>"
                       "<description><text>desc</text></description>"
                       "<reference><text>ref</text></reference>"
                       /* TODO yin-extension-prefix-compilation-bug "<myext:ext xmlns:myext=\"urn:libyang:test:identityone-yin\"/>" */
                   "</identity><extension name=\"ext\"/><identity name=\"base-name\"/><feature name=\"iff\"/>", mod);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->identities));
    assert_string_equal(mod->parsed->identities[0].name, "ident-name");
    assert_string_equal(mod->parsed->identities[0].bases[0], "base-name");
    assert_string_equal(mod->parsed->identities[0].iffeatures[0].str, "iff");
    assert_string_equal(mod->parsed->identities[0].dsc, "desc");
    assert_string_equal(mod->parsed->identities[0].ref, "ref");
    assert_true(mod->parsed->identities[0].flags & LYS_STATUS_DEPRC);
    /*assert_string_equal(mod->parsed->identities[0].exts[0].name, "ext");
    assert_non_null(mod->parsed->identities[0].exts[0].compiled);
    assert_int_equal(mod->parsed->identities[0].exts[0].yin, 1);
    assert_int_equal(mod->parsed->identities[0].exts[0].insubstmt_index, 0);
    assert_int_equal(mod->parsed->identities[0].exts[0].insubstmt, LYEXT_SUBSTMT_SELF);*/

    /* min subelems */
    TEST_SCHEMA_OK(ctx, 1, 1, "identitytwo-yin", "<identity name=\"ident-name\" />", mod);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->parsed->identities));
    assert_string_equal(mod->parsed->identities[0].name, "ident-name");

    /* invalid substatement */
    TEST_SCHEMA_ERR(ctx, 0, 1, "inv", "<identity name=\"ident-name\"><if-feature name=\"iff\"/></identity>",
                    "Invalid sub-elemnt \"if-feature\" of \"identity\" element - this sub-element is allowed only in modules with version 1.1 or newer. Line number 1.");

    /*
     * compiling
     */
    TEST_SCHEMA_OK(ctx, 0, 0, "a", "identity a1;", mod_imp);
    TEST_SCHEMA_OK(ctx, 1, 0, "b", "import a {prefix a;}"
                   "identity b1; identity b2; identity b3 {base b1; base b:b2; base a:a1;}"
                   "identity b4 {base b:b1; base b3;}", mod);
    assert_non_null(mod_imp->compiled);
    assert_non_null(mod_imp->identities);
    assert_non_null(mod->identities);
    assert_non_null(mod_imp->identities[0].derived);
    assert_int_equal(1, LY_ARRAY_COUNT(mod_imp->identities[0].derived));
    assert_ptr_equal(mod_imp->identities[0].derived[0], &mod->identities[2]);
    assert_non_null(mod->identities[0].derived);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->identities[0].derived));
    assert_ptr_equal(mod->identities[0].derived[0], &mod->identities[2]);
    assert_ptr_equal(mod->identities[0].derived[1], &mod->identities[3]);
    assert_non_null(mod->identities[1].derived);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->identities[1].derived));
    assert_ptr_equal(mod->identities[1].derived[0], &mod->identities[2]);
    assert_non_null(mod->identities[2].derived);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->identities[2].derived));
    assert_ptr_equal(mod->identities[2].derived[0], &mod->identities[3]);

    TEST_SCHEMA_OK(ctx, 1, 0, "c", "identity c2 {base c1;} identity c1;", mod);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->identities[1].derived));
    assert_ptr_equal(mod->identities[1].derived[0], &mod->identities[0]);

    TEST_SCHEMA_ERR(ctx, 0, 0, "inv", "identity i1;identity i1;", "Duplicate identifier \"i1\" of identity statement. /inv:{identity='i1'}");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule inv_sub {belongs-to inv {prefix inv;} identity i1;}");
    TEST_SCHEMA_ERR(ctx, 0, 0, "inv", "include inv_sub;identity i1;",
                    "Duplicate identifier \"i1\" of identity statement. /inv:{identity='i1'}");
    TEST_SCHEMA_ERR(ctx, 0, 0,"inv", "identity i1 {base i2;}", "Unable to find base (i2) of identity \"i1\". /inv:{identity='i1'}");
    TEST_SCHEMA_ERR(ctx, 0, 0,"inv", "identity i1 {base i1;}", "Identity \"i1\" is derived from itself. /inv:{identity='i1'}");
    TEST_SCHEMA_ERR(ctx, 0, 0,"inv", "identity i1 {base i2;}identity i2 {base i3;}identity i3 {base i1;}",
                    "Identity \"i1\" is indirectly derived from itself. /inv:{identity='i3'}");

    /* base in non-implemented module */
    ly_ctx_set_module_imp_clb(ctx, test_imp_clb,
                              "module base {namespace \"urn\"; prefix b; identity i1; identity i2 {base i1;}}");
    TEST_SCHEMA_OK(ctx, 0, 0, "ident", "import base {prefix b;} identity ii {base b:i1;}", mod);

    /* default value from non-implemented module */
    TEST_SCHEMA_ERR(ctx, 0, 0, "ident2", "import base {prefix b;} leaf l {type identityref {base b:i1;} default b:i2;}",
                    "Invalid default - value does not fit the type (Invalid identityref \"b:i2\" value"
                    " - identity found in non-implemented module \"base\".). /ident2:l");

    /* default value in typedef from non-implemented module */
    TEST_SCHEMA_ERR(ctx, 0, 0, "ident2", "import base {prefix b;} typedef t1 {type identityref {base b:i1;} default b:i2;}"
                    "leaf l {type t1;}", "Invalid default - value does not fit the type (Invalid"
                    " identityref \"b:i2\" value - identity found in non-implemented module \"base\".). /ident2:l");

    /*
     * printing
     */

    /*
     * cleanup
     */

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}

void
test_feature(void **state)
{
    *state = test_feature;

    struct ly_ctx *ctx;
    const struct lys_module *mod;
    const struct lysc_feature *f, *f1;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    /*
     * parsing YANG
     */

    TEST_STMT_DUP(ctx, 1, 0, "feature f", "description", "a", "b", "1");
    TEST_STMT_DUP(ctx, 1, 0, "feature f", "reference", "a", "b", "1");
    TEST_STMT_DUP(ctx, 1, 0, "feature f", "status", "current", "obsolete", "1");

    /* full content */
    TEST_SCHEMA_OK(ctx, 1, 0, "featureone",
                   "feature test {description text;reference \'another text\';status current; if-feature x; if-feature y; featureone:ext;}"
                   "extension ext; feature x; feature y;", mod);
    assert_non_null(mod->parsed->features);
    assert_int_equal(3, LY_ARRAY_COUNT(mod->parsed->features));

    /* invalid substatement */
    TEST_STMT_SUBSTM_ERR(ctx, 0, "feature", "organization", "XXX");

    /*
     * parsing YIN
     */
    /* max subelems */
    TEST_SCHEMA_OK(ctx, 0, 1, "featureone-yin", "<feature name=\"feature-name\">"
                       "<if-feature name=\"iff\"/>"
                       "<status value=\"deprecated\"/>"
                       "<description><text>desc</text></description>"
                       "<reference><text>ref</text></reference>"
                       /* TODO yin-extension-prefix-compilation-bug "<myext:ext xmlns:myext=\"urn:libyang:test:featureone-yin\"/>" */
                   "</feature><extension name=\"ext\"/><feature name=\"iff\"/>", mod);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->features));
    assert_string_equal(mod->parsed->features[0].name, "feature-name");
    assert_string_equal(mod->parsed->features[0].dsc, "desc");
    assert_true(mod->parsed->features[0].flags & LYS_STATUS_DEPRC);
    assert_string_equal(mod->parsed->features[0].iffeatures[0].str, "iff");
    assert_string_equal(mod->parsed->features[0].ref, "ref");
    /*assert_string_equal(mod->parsed->features[0].exts[0].name, "ext");
    assert_int_equal(mod->parsed->features[0].exts[0].insubstmt_index, 0);
    assert_int_equal(mod->parsed->features[0].exts[0].insubstmt, LYEXT_SUBSTMT_SELF);*/

    /* min subelems */
    TEST_SCHEMA_OK(ctx, 0, 1, "featuretwo-yin", "<feature name=\"feature-name\"/>", mod)
    assert_int_equal(1, LY_ARRAY_COUNT(mod->parsed->features));
    assert_string_equal(mod->parsed->features[0].name, "feature-name");

    /* invalid substatement */
    TEST_SCHEMA_ERR(ctx, 0, 1, "inv", "<feature name=\"feature-name\"><organization><text>org</text></organization></feature>",
                    "Unexpected sub-element \"organization\" of \"feature\" element. Line number 1.");

    /*
     * compiling
     */

    TEST_SCHEMA_OK(ctx, 1, 0, "a", "feature f1 {description test1;reference test2;status current;} feature f2; feature f3;\n"
                   "feature orfeature {if-feature \"f1 or f2\";}\n"
                   "feature andfeature {if-feature \"f1 and f2\";}\n"
                   "feature f6 {if-feature \"not f1\";}\n"
                   "feature f7 {if-feature \"(f2 and f3) or (not f1)\";}\n"
                   "feature f8 {if-feature \"f1 or f2 or f3 or orfeature or andfeature\";}\n"
                   "feature f9 {if-feature \"not not f1\";}", mod);
    assert_non_null(mod->features);
    assert_int_equal(9, LY_ARRAY_COUNT(mod->features));

    /* all features are disabled by default */
    LY_ARRAY_FOR(mod->features, struct lysc_feature, f) {
        assert_int_equal(LY_ENOT, lysc_feature_value(f));
    }
    /* enable f1 */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "f1"));
    f1 = &mod->features[0];
    assert_int_equal(LY_SUCCESS, lysc_feature_value(f1));

    /* enable orfeature */
    f = &mod->features[3];
    assert_int_equal(LY_ENOT, lysc_feature_value(f));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "orfeature"));
    assert_int_equal(LY_SUCCESS, lysc_feature_value(f));

    /* enable andfeature - no possible since f2 is disabled */
    f = &mod->features[4];
    assert_int_equal(LY_ENOT, lysc_feature_value(f));
    assert_int_equal(LY_EDENIED, lys_feature_enable(mod, "andfeature"));
    logbuf_assert("Feature \"andfeature\" cannot be enabled since it is disabled by its if-feature condition(s).");
    assert_int_equal(LY_ENOT, lysc_feature_value(f));

    /* first enable f2, so f5 can be enabled then */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "f2"));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "andfeature"));
    assert_int_equal(LY_SUCCESS, lysc_feature_value(f));

    /* f1 is enabled, so f6 cannot be enabled */
    f = &mod->features[5];
    assert_int_equal(LY_ENOT, lysc_feature_value(f));
    assert_int_equal(LY_EDENIED, lys_feature_enable(mod, "f6"));
    logbuf_assert("Feature \"f6\" cannot be enabled since it is disabled by its if-feature condition(s).");
    assert_int_equal(LY_ENOT, lysc_feature_value(f));

    /* so disable f1 - andfeature will became also disabled */
    assert_int_equal(LY_SUCCESS, lysc_feature_value(f1));
    assert_int_equal(LY_SUCCESS, lys_feature_disable(mod, "f1"));
    assert_int_equal(LY_ENOT, lysc_feature_value(f1));
    assert_int_equal(LY_ENOT, lysc_feature_value(&mod->features[4]));
    /* while orfeature is stille enabled */
    assert_int_equal(LY_SUCCESS, lysc_feature_value(&mod->features[3]));
    /* and finally f6 can be enabled */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "f6"));
    assert_int_equal(LY_SUCCESS, lysc_feature_value(&mod->features[5]));

    /* complex evaluation of f7: f1 and f3 are disabled, while f2 is enabled */
    assert_int_equal(LY_SUCCESS, lysc_iffeature_value(&mod->features[6].iffeatures[0]));
    /* long evaluation of f8 to need to reallocate internal stack for operators */
    assert_int_equal(LY_SUCCESS, lysc_iffeature_value(&mod->features[7].iffeatures[0]));

    /* double negation of disabled f1 -> disabled */
    assert_int_equal(LY_ENOT, lysc_iffeature_value(&mod->features[8].iffeatures[0]));

    /* disable all features */
    assert_int_equal(LY_SUCCESS, lys_feature_disable(mod, "*"));
    LY_ARRAY_FOR(mod->features, struct lysc_feature, f) {
        assert_int_equal(LY_ENOT, lys_feature_value(mod, f->name));
    }
    /* re-setting already set feature */
    assert_int_equal(LY_SUCCESS, lys_feature_disable(mod, "f1"));
    assert_int_equal(LY_ENOT, lys_feature_value(mod, "f1"));

    /* enabling feature that cannot be enabled due to its if-features */
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "f1"));
    assert_int_equal(LY_EDENIED, lys_feature_enable(mod, "andfeature"));
    logbuf_assert("Feature \"andfeature\" cannot be enabled since it is disabled by its if-feature condition(s).");
    assert_int_equal(LY_EDENIED, lys_feature_enable(mod, "*"));
    logbuf_assert("Feature \"f6\" cannot be enabled since it is disabled by its if-feature condition(s).");
    /* test if not changed */
    assert_int_equal(LY_SUCCESS, lys_feature_value(mod, "f1"));
    assert_int_equal(LY_ENOT, lys_feature_value(mod, "f2"));

    TEST_SCHEMA_OK(ctx, 0, 0, "b", "feature f1 {if-feature f2;}feature f2;", mod);
    assert_non_null(mod->features);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->features));
    assert_non_null(mod->features[0].iffeatures);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->features[0].iffeatures));
    assert_non_null(mod->features[0].iffeatures[0].features);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->features[0].iffeatures[0].features));
    assert_ptr_equal(&mod->features[1], mod->features[0].iffeatures[0].features[0]);
    assert_non_null(mod->features);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->features));
    assert_non_null(mod->features[1].depfeatures);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->features[1].depfeatures));
    assert_ptr_equal(&mod->features[0], mod->features[1].depfeatures[0]);

    /* invalid reference */
    assert_int_equal(LY_ENOTFOUND, lys_feature_enable(mod, "xxx"));
    logbuf_assert("Feature \"xxx\" not found in module \"b\".");

    /* some invalid expressions */
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f{if-feature f1;}",
                    "Invalid value \"f1\" of if-feature - unable to find feature \"f1\". /inv:{feature='f'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f2{if-feature 'f and';}",
                    "Invalid value \"f and\" of if-feature - unexpected end of expression. /inv:{feature='f2'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f{if-feature 'or';}",
                    "Invalid value \"or\" of if-feature - unexpected end of expression. /inv:{feature='f'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f2{if-feature '(f1';}",
                    "Invalid value \"(f1\" of if-feature - non-matching opening and closing parentheses. /inv:{feature='f2'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f2{if-feature 'f1)';}",
                    "Invalid value \"f1)\" of if-feature - non-matching opening and closing parentheses. /inv:{feature='f2'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f2{if-feature ---;}",
                    "Invalid value \"---\" of if-feature - unable to find feature \"---\". /inv:{feature='f2'}");
    TEST_SCHEMA_ERR(ctx, 0, 0, "inv", "feature f1; feature f2{if-feature 'not f1';}",
                    "Invalid value \"not f1\" of if-feature - YANG 1.1 expression in YANG 1.0 module. /inv:{feature='f2'}");
    TEST_SCHEMA_ERR(ctx, 0, 0, "inv", "feature f1; feature f1;",
                    "Duplicate identifier \"f1\" of feature statement. /inv:{feature='f1'}");

    ly_ctx_set_module_imp_clb(ctx, test_imp_clb, "submodule inv_sub {belongs-to inv {prefix inv;} feature f1;}");
    TEST_SCHEMA_ERR(ctx, 0, 0, "inv", "include inv_sub;feature f1;",
                    "Duplicate identifier \"f1\" of feature statement. /inv:{feature='f1'}");
    TEST_SCHEMA_ERR(ctx, 0, 0, "inv", "feature f1 {if-feature f2;} feature f2 {if-feature f1;}",
                    "Feature \"f1\" is indirectly referenced from itself. /inv:{feature='f2'}");
    TEST_SCHEMA_ERR(ctx, 0, 0, "inv", "feature f1 {if-feature f1;}",
                    "Feature \"f1\" is referenced from itself. /inv:{feature='f1'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f {if-feature ();}",
                    "Invalid value \"()\" of if-feature - number of features in expression does not match the required number of operands for the operations. /inv:{feature='f'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f {if-feature 'f1(';}",
                    "Invalid value \"f1(\" of if-feature - non-matching opening and closing parentheses. /inv:{feature='f'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f {if-feature 'and f1';}",
                    "Invalid value \"and f1\" of if-feature - missing feature/expression before \"and\" operation. /inv:{feature='f'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f {if-feature 'f1 not ';}",
                    "Invalid value \"f1 not \" of if-feature - unexpected end of expression. /inv:{feature='f'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f {if-feature 'f1 not not ';}",
                    "Invalid value \"f1 not not \" of if-feature - unexpected end of expression. /inv:{feature='f'}");
    TEST_SCHEMA_ERR(ctx, 1, 0, "inv", "feature f1; feature f2; feature f {if-feature 'or f1 f2';}",
                    "Invalid value \"or f1 f2\" of if-feature - missing feature/expression before \"or\" operation. /inv:{feature='f'}");

    /* import reference */
    assert_non_null(mod = ly_ctx_get_module(ctx, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "f1"));
    TEST_SCHEMA_OK(ctx, 0, 0, "c", "import a {prefix a;} feature f1; feature f2{if-feature 'a:f1';}", mod);
    assert_int_equal(LY_SUCCESS, lys_feature_enable(mod, "f2"));
    assert_int_equal(LY_ENOT, lys_feature_value(mod, "f1"));
    assert_int_equal(LY_SUCCESS, lys_feature_value(mod, "f2"));

    /*
     * printing
     */

    /*
     * cleanup
     */

    *state = NULL;
    ly_ctx_destroy(ctx, NULL);
}
