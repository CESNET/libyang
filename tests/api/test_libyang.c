/*
 * @file test_libyang.c
 * @author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief unit tests for functions from libyang.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "../config.h"
#include "../../src/libyang.h"

/* include private header to be able to check internal values */
#include "../../src/context.h"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;
const struct lys_module *module = NULL;

int
generic_init(char *config_file, char *yin_file, char *yang_file, char *yang_folder)
{
    LYD_FORMAT in_format;
    char *schema1 = NULL;
    char *schema2 = NULL;
    char *config = NULL;
    struct stat sb_schema1, sb_schema2, sb_config;
    int fd = -1;

    if (!config_file || !yang_file || !yin_file || !yang_folder) {
        goto error;
    }

    in_format = LYD_XML;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    fd = open(yin_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_schema1) == -1 || !S_ISREG(sb_schema1.st_mode)) {
        goto error;
    }

    schema1 = mmap(NULL, sb_schema1.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    fd = open(yang_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_schema2) == -1 || !S_ISREG(sb_schema2.st_mode)) {
        goto error;
    }

    schema2 = mmap(NULL, sb_schema2.st_size + 2, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);

    fd = open(config_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_config) == -1 || !S_ISREG(sb_config.st_mode)) {
        goto error;
    }

    config = mmap(NULL, sb_config.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    fd = -1;

    if (!lys_parse_mem(ctx, schema1, LYS_IN_YIN)) {
        goto error;
    }

    if (!(module = lys_parse_mem(ctx, schema2, LYS_IN_YANG))) {
        goto error;
    }

    root = lyd_parse_mem(ctx, config, in_format, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    /* cleanup */
    munmap(config, sb_config.st_size);
    munmap(schema1, sb_schema1.st_size);
    munmap(schema2, sb_schema2.st_size + 2);

    return 0;

error:
    if (schema1) {
        munmap(schema1, sb_schema1.st_size);
    }
    if (schema2) {
        munmap(schema2, sb_schema2.st_size + 2);
    }
    if (config) {
        munmap(config, sb_config.st_size);
    }
    if (fd != -1) {
        close(fd);
    }

    return -1;
}

static int
setup_f(void **state)
{
    (void) state; /* unused */
    char *config_file = TESTS_DIR"/api/files/a.xml";
    char *yin_file = TESTS_DIR"/api/files/a.yin";
    char *yang_file = TESTS_DIR"/api/files/b.yang";
    char *yang_folder = TESTS_DIR"/api/files";
    int rc;

    rc = generic_init(config_file, yin_file, yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (root) {
        lyd_free_withsiblings(root);
    }

    if (ctx) {
        ly_ctx_destroy(ctx, NULL);
    }

    root = NULL;
    ctx = NULL;

    return 0;
}

static void
test_ly_ctx_new(void **state)
{
    char *yang_folder = TESTS_DIR"/data/files";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_new_invalid(void **state)
{
    char *yang_folder = "INVALID_PATH";
    (void) state; /* unused */
    ctx = ly_ctx_new(yang_folder);
    if (ctx) {
        fail();
    }
}

static void
test_ly_ctx_get_searchdirs(void **state)
{
    const char * const *result;
    char yang_folder[PATH_MAX];
    (void) state; /* unused */

    assert_ptr_not_equal(realpath(TESTS_DIR"/data/files", yang_folder), NULL);

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    result = ly_ctx_get_searchdirs(ctx);
    if (!result) {
        fail();
    }
    assert_string_equal(yang_folder, result[0]);
    assert_ptr_equal(NULL, result[1]);

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_set_searchdir(void **state)
{
    const char * const *result;
    char yang_folder[PATH_MAX];
    char new_yang_folder[PATH_MAX];
    (void) state; /* unused */

    assert_ptr_not_equal(realpath(TESTS_DIR"/data/files", yang_folder), NULL);
    assert_ptr_not_equal(realpath(TESTS_DIR"/schema/yin", new_yang_folder), NULL);

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    ly_ctx_set_searchdir(ctx, new_yang_folder);
    result = ly_ctx_get_searchdirs(ctx);
    if (!result) {
        fail();
    }

    assert_string_equal(yang_folder, result[0]);
    assert_string_equal(new_yang_folder, result[1]);
    assert_ptr_equal(NULL, result[2]);

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_set_searchdir_invalid(void **state)
{
    const char * const *result;
    char yang_folder[PATH_MAX];
    char *new_yang_folder = "INVALID_PATH";
    (void) state; /* unused */

    assert_ptr_not_equal(realpath(TESTS_DIR"/data/files", yang_folder), NULL);

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        fail();
    }

    /* adding duplicity - the path is not duplicated */
    ly_ctx_set_searchdir(NULL, yang_folder);
    result = ly_ctx_get_searchdirs(ctx);
    if (!result) {
        fail();
    }
    assert_string_equal(yang_folder, result[0]);
    assert_ptr_equal(NULL, result[1]);

    /* adding invalid path, previous is kept */
    ly_ctx_set_searchdir(ctx, new_yang_folder);
    result = ly_ctx_get_searchdirs(ctx);
    if (!result) {
        fail();
    }
    assert_string_equal(yang_folder, result[0]);
    assert_ptr_equal(NULL, result[1]);

    ly_ctx_unset_searchdirs(ctx);
    result = ly_ctx_get_searchdirs(ctx);
    if (result) {
        fail();
    }

    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_info(void **state)
{
    struct lyd_node *node;
    (void) state; /* unused */

    node = ly_ctx_info(NULL);
    if (node) {
        fail();
    }

    node = ly_ctx_info(ctx);
    if (!node) {
        fail();
    }

    assert_int_equal(LYD_VAL_OK, node->validity);

    lyd_free_withsiblings(node);
}

static void
test_ly_ctx_get_module(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    const char *name1 = "a";
    const char *name2 = "b";
    const char *revision = "2016-03-01";

    module = ly_ctx_get_module(NULL, name1, NULL);
    if (module) {
        fail();
    }

    module = ly_ctx_get_module(ctx, NULL, NULL);
    if (module) {
        fail();
    }

    module = ly_ctx_get_module(ctx, "invalid", NULL);
    if (module) {
        fail();
    }

    module = ly_ctx_get_module(ctx, name1, NULL);
    if (!module) {
        fail();
    }

    assert_string_equal("a", module->name);

    module = ly_ctx_get_module(ctx, name1, "invalid");
    if (module) {
        fail();
    }

    module = ly_ctx_get_module(ctx, name1, revision);
    if (!module) {
        fail();
    }

    assert_string_equal(revision, module->rev->date);

    module = ly_ctx_get_module(ctx, name2, NULL);
    if (!module) {
        fail();
    }

    assert_string_equal("b", module->name);

    module = ly_ctx_get_module(ctx, name2, "invalid");
    if (module) {
        fail();
    }

    module = ly_ctx_get_module(ctx, name2, revision);
    if (!module) {
        fail();
    }

    assert_string_equal(revision, module->rev->date);
}

static void
test_ly_ctx_get_module_older(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module = NULL;
    const struct lys_module *module_older = NULL;
    const char *name = "a";
    const char *revision = "2016-03-01";
    const char *revision_older = "2015-01-01";

    module_older = ly_ctx_get_module_older(NULL, module);
    if (module_older) {
        fail();
    }

    module_older = ly_ctx_get_module_older(ctx, NULL);
    if (module_older) {
        fail();
    }

    module = ly_ctx_load_module(ctx, "c", NULL);
    if (!module) {
        fail();
    }

    module = ly_ctx_load_module(ctx, name, revision);
    if (!module) {
        fail();
    }

    module_older = ly_ctx_get_module_older(ctx, module);
    if (!module_older) {
        fail();
    }

    assert_string_equal(revision_older, module_older->rev->date);
}

static void
test_ly_ctx_load_module(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    const char *name = "a";
    const char *revision = "2015-01-01";

    module = ly_ctx_load_module(NULL, name, revision);
    if (module) {
        fail();
    }

    module = ly_ctx_load_module(ctx, NULL, revision);
    if (module) {
        fail();
    }

    module = ly_ctx_load_module(ctx, "INVALID_NAME", revision);
    if (module) {
        fail();
    }

    module = ly_ctx_load_module(ctx, "c", NULL);
    if (!module) {
        fail();
    }

    assert_string_equal("c", module->name);

    module = ly_ctx_get_module(ctx, "a", revision);
    if (!module) {
        fail();
    }

    assert_string_equal("a", module->name);

    module = ly_ctx_get_module(ctx, "b", revision);
    if (!module) {
        fail();
    }

    assert_string_equal("b", module->name);
}

static void
test_ly_ctx_clean(void **state)
{
    (void) state; /* unused */
    const struct lys_module *mod;
    struct ly_ctx *ctx;
    uint32_t dict_used;
    uint16_t setid;
    int modules_count;

    ctx = ly_ctx_new(TESTS_DIR"/api/files/");
    /* remember starting values */
    setid = ctx->models.module_set_id;
    modules_count = ctx->models.used;
    dict_used = ctx->dict.used;

    /* add a module */
    mod = ly_ctx_load_module(ctx, "x", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_int_equal(modules_count + 1, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);

    /* clean the context */
    ly_ctx_clean(ctx, NULL);
    assert_int_equal(setid + 2, ctx->models.module_set_id);
    assert_int_equal(modules_count, ctx->models.used);
    assert_int_equal(dict_used, ctx->dict.used);

    /* add a module again ... */
    mod = ly_ctx_load_module(ctx, "x", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_int_equal(modules_count + 1, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);
    /* .. and add some string into dictionary */
    assert_ptr_not_equal(lydict_insert(ctx, "qwertyuiop", 0), NULL);

    /* clean the context */
    ly_ctx_clean(ctx, NULL);
    assert_int_equal(setid + 4, ctx->models.module_set_id);
    assert_int_equal(modules_count, ctx->models.used);
    assert_int_equal(dict_used + 1, ctx->dict.used);

    /* cleanup */
    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_clean2(void **state)
{
    (void) state; /* unused */
    const char *yang_dep = "module x {"
                    "  namespace uri:x;"
                    "  prefix x;"
                    "  import ietf-yang-library { prefix yl; }"
                    "  leaf x { config false; type leafref { path /yl:modules-state/yl:module/yl:name; } } }";
    struct ly_ctx *ctx;
    const struct lys_module *mod;
    struct lys_node_leaf *leaf;

    ctx = ly_ctx_new(NULL);
    assert_ptr_not_equal(ctx, NULL);

    /* load module depending by leafref on internal ietf-yang-library */
    assert_ptr_not_equal(lys_parse_mem(ctx, yang_dep, LYS_IN_YANG), NULL);

    /* get the target leaf in ietf-yang-library */
    mod = ctx->models.list[4];
    /* magic: leaf = /yl:modules-state/yl:module/yl:name */
    leaf = (struct lys_node_leaf *)mod->data->next->child->next->child->prev->child->child;
    assert_true(leaf->backlinks && leaf->backlinks->number == 1);

    /* clean the context ... */
    ly_ctx_clean(ctx, NULL);

    /* ... and check that the leafref backlinks are removed */
    assert_true(!leaf->backlinks || !leaf->backlinks->number);

    /* cleanup */
    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_ctx_remove_module(void **state)
{
    (void) state; /* unused */
    const struct lys_module *mod;
    uint32_t dict_used;
    uint16_t setid;
    int modules_count;

    ctx = ly_ctx_new(TESTS_DIR"/api/files/");
    /* remember starting values */
    setid = ctx->models.module_set_id;
    modules_count = ctx->models.used;
    dict_used = ctx->dict.used;

    mod = ly_ctx_load_module(ctx, "x", NULL);
    ly_ctx_remove_module(mod, NULL);

    /* add a module */
    mod = ly_ctx_load_module(ctx, "y", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count + 2, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);

    /* remove the imported module (x), that should cause removing also the loaded module (y) */
    mod = ly_ctx_get_module(ctx, "x", NULL);
    assert_ptr_not_equal(mod, NULL);
    ly_ctx_remove_module(mod, NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count, ctx->models.used);
    assert_int_equal(dict_used, ctx->dict.used);

    /* add a module again ... */
    mod = ly_ctx_load_module(ctx, "y", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count + 2, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);
    /* ... now remove the loaded module, the imported module is supposed to be removed because it is not
     * used in any other module */
    ly_ctx_remove_module(mod, NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count, ctx->models.used);
    assert_int_equal(dict_used, ctx->dict.used);

    /* add a module again ... */
    mod = ly_ctx_load_module(ctx, "y", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count + 2, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);
    /* and mark even the imported module 'x' as implemented ... */
    assert_int_equal(lys_set_implemented(mod->imp[0].module), EXIT_SUCCESS);
    /* ... now remove the loaded module, the imported module is supposed to be kept because it is implemented */
    ly_ctx_remove_module(mod, NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count + 1, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);
    ly_ctx_clean(ctx, NULL);

    /* add a module again ... */
    mod = ly_ctx_load_module(ctx, "y", NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count + 2, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);
    /* and add another one also importing module 'x' ... */
    assert_ptr_not_equal(ly_ctx_load_module(ctx, "z", NULL), NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count + 3, ctx->models.used);
    /* ... now remove the first loaded module, the imported module is supposed to be kept because it is used
     * by the second loaded module */
    ly_ctx_remove_module(mod, NULL);
    assert_true(setid < ctx->models.module_set_id);
    setid = ctx->models.module_set_id;
    assert_int_equal(modules_count + 2, ctx->models.used);
    assert_int_not_equal(dict_used, ctx->dict.used);
}

static void
test_ly_ctx_remove_module2(void **state)
{
    (void) state; /* unused */
    const char *yang_main = "module x {"
                    "  namespace uri:x;"
                    "  prefix x;"
                    "  feature x;"
                    "  identity basex;"
                    "  leaf x { type string; } }";
    const char *yang_dep = "module y {"
                    "  namespace uri:y;"
                    "  prefix y;"
                    "  import x { prefix x; }"
                    "  feature y { if-feature x:x; }"
                    "  identity y { base x:basex; }"
                    "  leaf y { type leafref { path /x:x; } } }";
    const struct lys_module *mod;
    struct lys_node_leaf *leaf;

    ctx = ly_ctx_new(NULL);
    assert_ptr_not_equal(ctx, NULL);

    /* load both modules, y depends on x and x will contain several backlinks to y */
    assert_ptr_not_equal((mod = lys_parse_mem(ctx, yang_main, LYS_IN_YANG)), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, yang_dep, LYS_IN_YANG), NULL);

    /* check that there are the expected backlinks */
    leaf = (struct lys_node_leaf *)mod->data;
    assert_true(mod->features[0].depfeatures && mod->features[0].depfeatures->number);
    assert_true(mod->ident[0].der && mod->ident[0].der->number);
    assert_true(leaf->backlinks && leaf->backlinks->number);

    /* remove y ... */
    mod = ly_ctx_get_module(ctx, "y", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_int_equal(ly_ctx_remove_module(mod, NULL), 0);

    /* ... make sure that x is still present ... */
    mod = ly_ctx_get_module(ctx, "x", NULL);
    assert_ptr_not_equal(mod, NULL);
    leaf = (struct lys_node_leaf *)mod->data;

    /* ... and check that the backlinks in it were removed */
    assert_true(!mod->features[0].depfeatures || !mod->features[0].depfeatures->number);
    assert_true(!mod->ident[0].der || !mod->ident[0].der->number);
    assert_true(!leaf->backlinks || !leaf->backlinks->number);
}

static void
test_lys_set_enabled(void **state)
{
    (void) state; /* unused */
    const struct lys_module *mod;

    ctx = ly_ctx_new(NULL);
    assert_ptr_not_equal(ctx, NULL);

    /* test failures - invalid input */
    assert_int_not_equal(lys_set_enabled(NULL), 0);

    /* test success - enabled module */
    mod = ly_ctx_get_module(ctx, "ietf-yang-library", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_int_equal(lys_set_enabled(mod), 0);
}

/* include also some test for lys_set_enabled() */
static void
test_lys_set_disabled(void **state)
{
    (void) state; /* unused */
    const struct lys_module *mod, *modx, *mody;
    const char *yang_x = "module x {"
                    "  namespace uri:x;"
                    "  prefix x;"
                    "  container x { presence yes; }}";
    const char *yang_y = "module y {"
                    "  namespace uri:y;"
                    "  prefix y;"
                    "  import x { prefix x;}"
                    "  augment /x:x {"
                    "    leaf y { type string;}}}";

    ctx = ly_ctx_new(NULL);
    assert_ptr_not_equal(ctx, NULL);

    /* test failures - invalid input */
    assert_int_not_equal(lys_set_disabled(NULL), 0);

    /* test failures - internal module */
    mod = ly_ctx_get_module(ctx, "ietf-yang-library", NULL);
    assert_ptr_not_equal(mod, NULL);
    assert_int_not_equal(lys_set_disabled(mod), 0);

    /* test success - disabling y extending x */
    modx = lys_parse_mem(ctx, yang_x, LYS_IN_YANG);
    assert_ptr_not_equal(modx, NULL);
    mody = lys_parse_mem(ctx, yang_y, LYS_IN_YANG);
    assert_ptr_not_equal(mody, NULL);

    /* all the modules are enabled ... */
    assert_int_equal(mody->disabled, 0);
    assert_int_equal(modx->disabled, 0);
    /* ... and the y's augment is applied */
    assert_ptr_not_equal(modx->data->child, NULL);

    /* by disabling y ... */
    assert_int_equal(lys_set_disabled(mody), 0);
    /* ... y is disabled while x stays enabled (it is implemented) ...*/
    assert_int_equal(mody->disabled, 1);
    assert_int_equal(modx->disabled, 0);
    /* ... and y's augment disappeared from x */
    assert_ptr_equal(modx->data->child, NULL);

    /* by enabling it, everything goes back */
    assert_int_equal(lys_set_enabled(mody), 0);
    assert_int_equal(mody->disabled, 0);
    assert_int_equal(modx->disabled, 0);
    assert_ptr_not_equal(modx->data->child, NULL);

    /* by disabling x ... */
    assert_int_equal(lys_set_disabled(modx), 0);
    /* ... both x and y are disabled (y depends on x) ...*/
    assert_int_equal(mody->disabled, 1);
    assert_int_equal(modx->disabled, 1);
    /* ... and y's augment disappeared from x */
    assert_ptr_equal(modx->data->child, NULL);

    /* by enabling it, everything goes back */
    assert_int_equal(lys_set_enabled(modx), 0);
    assert_int_equal(mody->disabled, 0);
    assert_int_equal(modx->disabled, 0);
    assert_ptr_not_equal(modx->data->child, NULL);
}


static void
test_ly_ctx_get_module_by_ns(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    const char *ns = "urn:a";
    const char *revision = NULL;

    module = ly_ctx_get_module_by_ns(NULL, ns, revision);
    if (module) {
        fail();
    }

    module = ly_ctx_get_module_by_ns(ctx, NULL, revision);
    if (module) {
        fail();
    }

    module = ly_ctx_get_module_by_ns(ctx, ns, revision);
    if (!module) {
        fail();
    }

    assert_string_equal("a", module->name);

    module = ly_ctx_get_module_by_ns(ctx, "urn:b", revision);
    if (!module) {
        fail();
    }

    assert_string_equal("b", module->name);
}

static void
test_ly_ctx_get_submodule(void **state)
{
    (void) state; /* unused */
    const struct lys_submodule *submodule;
    const char *mod_name = "a";
    const char *sub_name = "asub";
    const char *revision = NULL;

    submodule = ly_ctx_get_submodule(NULL, mod_name, revision, sub_name, NULL);
    if (submodule) {
        fail();
    }

    submodule = ly_ctx_get_submodule(ctx, NULL, revision, sub_name, "2010-02-08");
    if (submodule) {
        fail();
    }

    submodule = ly_ctx_get_submodule(ctx, mod_name, revision, NULL, NULL);
    if (submodule) {
        fail();
    }

    submodule = ly_ctx_get_submodule(ctx, mod_name, revision, sub_name, NULL);
    if (!submodule) {
        fail();
    }

    assert_string_equal("asub", submodule->name);

    submodule = ly_ctx_get_submodule(ctx, "b", revision, "bsub", NULL);
    if (!submodule) {
        fail();
    }

    assert_string_equal("bsub", submodule->name);
}

static void
test_ly_ctx_get_submodule2(void **state)
{
    (void) state; /* unused */
    const struct lys_submodule *submodule;
    const char *sub_name1 = "asub";
    const char *sub_name2 = "bsub";

    submodule = ly_ctx_get_submodule2(NULL, sub_name1);
    if (submodule) {
        fail();
    }

    submodule = ly_ctx_get_submodule2(root->schema->module, NULL);
    if (submodule) {
        fail();
    }

    submodule = ly_ctx_get_submodule2(root->schema->module, sub_name1);
    if (!submodule) {
        fail();
    }

    assert_string_equal("asub", submodule->name);

    submodule = ly_ctx_get_submodule2(module, sub_name2);
    if (!submodule) {
        fail();
    }

    assert_string_equal("bsub", submodule->name);
}

static void
test_ly_ctx_get_node(void **state)
{
    (void) state; /* unused */
    const struct lys_node *node;
    const char *nodeid1 = "/a:x/bubba";
    const char *nodeid2 = "/b:x/bubba";
    const char *nodeid3 = "/a:x/choic/con/con/lef";

    node = ly_ctx_get_node(NULL, root->schema, nodeid1);
    if (node) {
        fail();
    }

    node = ly_ctx_get_node(ctx, root->schema, NULL);
    if (node) {
        fail();
    }

    node = ly_ctx_get_node(ctx, root->schema, nodeid1);
    if (!node) {
        fail();
    }

    assert_string_equal("bubba", node->name);

    node = ly_ctx_get_node(ctx, root->schema, nodeid2);
    if (!node) {
        fail();
    }

    assert_string_equal("bubba", node->name);

    node = ly_ctx_get_node(ctx, root->schema, nodeid3);
    if (!node) {
        fail();
    }

    assert_string_equal("lef", node->name);
}

static void
test_ly_set_new(void **state)
{
    (void) state; /* unused */
    struct ly_set *set;

    set = ly_set_new();
    if (!set) {
        fail();
    }

    free(set);
}

static void
test_ly_set_add(void **state)
{
    (void) state; /* unused */
    struct ly_set *set;
    int rc;

    set = ly_set_new();
    if (!set) {
        fail();
    }

    rc = ly_set_add(NULL, root->child->schema, 0);
    if(rc != -1) {
        fail();
    }

    rc = ly_set_add(set, NULL, 0);
    if(rc != -1) {
        fail();
    }

    rc = ly_set_add(set, root->child->schema, 0);
    if(rc == -1) {
        fail();
    }

    ly_set_free(set);
}

static void
test_ly_set_rm(void **state)
{
    (void) state; /* unused */
    struct ly_set *set;
    int rc;

    set = ly_set_new();
    if (!set) {
        fail();
    }

    rc = ly_set_rm(NULL, root->child->schema);
    if(!rc) {
        fail();
    }

    rc = ly_set_rm(set, NULL);
    if(!rc) {
        fail();
    }

    rc = ly_set_add(set, root->child->schema, 0);
    if(rc) {
        fail();
    }

    rc = ly_set_rm(set, root->child->schema);
    if(rc) {
        fail();
    }

    ly_set_free(set);
}

static void
test_ly_set_rm_index(void **state)
{
    (void) state; /* unused */
    struct ly_set *set;
    int rc;

    set = ly_set_new();
    if (!set) {
        fail();
    }

    rc = ly_set_rm_index(NULL, 0);
    if(!rc) {
        fail();
    }

    rc = ly_set_add(set, root->child->schema, 0);
    if(rc) {
        fail();
    }

    rc = ly_set_rm_index(set, 0);
    if(rc) {
        fail();
    }

    ly_set_free(set);
}

static void
test_ly_set_free(void **state)
{
    (void) state; /* unused */
    struct ly_set *set;

    set = ly_set_new();
    if (!set) {
        fail();
    }

    ly_set_free(set);

    if (!set) {
        fail();
    }
}

static void
test_ly_verb(void **state)
{
    (void) state; /* unused */

    ly_verb(LY_LLERR);
}

void clb_custom(LY_LOG_LEVEL level, const char *msg, const char *path )
{
    (void) level; /* unused */
    (void) msg; /* unused */
    (void) path; /* unused */
}

static void
test_ly_get_log_clb(void **state)
{
    (void) state; /* unused */
    void *clb = NULL;

    clb = ly_get_log_clb();
    assert_ptr_equal(clb, NULL);
}

static void
test_ly_set_log_clb(void **state)
{
    (void) state; /* unused */
    void *clb = NULL;
    void *clb_new = NULL;

    clb = ly_get_log_clb();

    ly_set_log_clb(clb_custom,0);

    clb_new = ly_get_log_clb();

    assert_ptr_not_equal(clb, clb_new);
}

static void
test_ly_errno_location(void **state)
{
    (void) state; /* unused */
    char *yang_folder = "INVALID_PATH";

    LY_ERR *error;

    error = ly_errno_location();

    assert_int_equal(LY_SUCCESS, *error);

    ctx = ly_ctx_new(yang_folder);
    if (ctx) {
        fail();
    }

    error = ly_errno_location();

    assert_int_equal(LY_ESYS, *error);
    ly_ctx_destroy(ctx, NULL);
}

static void
test_ly_errmsg(void **state)
{
    (void) state; /* unused */
    const char *msg;
    char *yang_folder = "INVALID_PATH";
    char *compare = "Unable to use search directory \"INVALID_PATH\" (No such file or directory)";

    ctx = ly_ctx_new(yang_folder);
    if (ctx) {
        fail();
    }

    msg = ly_errmsg();

    assert_string_equal(compare, msg);
}

static void
test_ly_errpath(void **state)
{
    (void) state; /* unused */
    const char *path;
    char *compare = "";

    path = ly_errpath();

    assert_string_equal(compare, path);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ly_ctx_new),
        cmocka_unit_test(test_ly_ctx_new_invalid),
        cmocka_unit_test(test_ly_ctx_get_searchdirs),
        cmocka_unit_test(test_ly_ctx_set_searchdir),
        cmocka_unit_test(test_ly_ctx_set_searchdir_invalid),
        cmocka_unit_test_setup_teardown(test_ly_ctx_info, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_ctx_get_module, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_ctx_get_module_older, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_ctx_load_module, setup_f, teardown_f),
        cmocka_unit_test_teardown(test_ly_ctx_remove_module, teardown_f),
        cmocka_unit_test_teardown(test_ly_ctx_remove_module2, teardown_f),
        cmocka_unit_test_teardown(test_lys_set_enabled, teardown_f),
        cmocka_unit_test_teardown(test_lys_set_disabled, teardown_f),
        cmocka_unit_test(test_ly_ctx_clean),
        cmocka_unit_test(test_ly_ctx_clean2),
        cmocka_unit_test_setup_teardown(test_ly_ctx_get_module_by_ns, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_ctx_get_submodule, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_ctx_get_submodule2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_ctx_get_node, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_set_new, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_set_add, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_set_rm, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_set_rm_index, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_set_free, setup_f, teardown_f),
        cmocka_unit_test(test_ly_verb),
        cmocka_unit_test(test_ly_get_log_clb),
        cmocka_unit_test(test_ly_set_log_clb),
        cmocka_unit_test(test_ly_errno_location),
        cmocka_unit_test(test_ly_errmsg),
        cmocka_unit_test_setup_teardown(test_ly_errpath, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
