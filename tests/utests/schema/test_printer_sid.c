/**
 * @file test_printer_sid.c
 * @author: Petr Hanzlik <Petr.Hanzlik@cesnet.cz>
 * @brief unit tests for functions from printer_sid.c
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "context.h"
#include "tree_data.h"
#include "tree_schema.h"

/* 3 SID items: module/a01, /a01:cont, /a01:cont/l */
static const char *mod_v1 =
        "module a01 {\n"
        "  yang-version 1.1;\n"
        "  namespace \"urn:a01\";\n"
        "  prefix a01;\n"
        "  revision 2024-01-01;\n"
        "  container cont {\n"
        "    leaf l {\n"
        "      type string;\n"
        "    }\n"
        "  }\n"
        "}\n";

/* 5 SID items: v1 plus /a01:cont/l2 and /a01:cont/l3 */
static const char *mod_v2 =
        "module a01 {\n"
        "  yang-version 1.1;\n"
        "  namespace \"urn:a01\";\n"
        "  prefix a01;\n"
        "  revision 2024-02-02;\n"
        "  container cont {\n"
        "    leaf l {\n"
        "      type string;\n"
        "    }\n"
        "    leaf l2 {\n"
        "      type string;\n"
        "    }\n"
        "    leaf l3 {\n"
        "      type string;\n"
        "    }\n"
        "  }\n"
        "}\n";

/* 4 SID items: compared to v1 drops /a01:cont/l and adds /a01:cont/m and /a01:cont/n */
static const char *mod_v3 =
        "module a01 {\n"
        "  yang-version 1.1;\n"
        "  namespace \"urn:a01\";\n"
        "  prefix a01;\n"
        "  revision 2024-03-03;\n"
        "  container cont {\n"
        "    leaf m {\n"
        "      type string;\n"
        "    }\n"
        "    leaf n {\n"
        "      type string;\n"
        "    }\n"
        "  }\n"
        "}\n";

/* 7 SID items: compared to v3 keeps only /a01:cont and adds 5 new leaves;
   the dropped items /a01:cont/l, /a01:cont/m and /a01:cont/n become obsolete
   and keep holding SIDs 102, 200 and 300 */
static const char *mod_v4 =
        "module a01 {\n"
        "  yang-version 1.1;\n"
        "  namespace \"urn:a01\";\n"
        "  prefix a01;\n"
        "  revision 2024-04-04;\n"
        "  container cont {\n"
        "    leaf p1 {\n"
        "      type string;\n"
        "    }\n"
        "    leaf p2 {\n"
        "      type string;\n"
        "    }\n"
        "    leaf p3 {\n"
        "      type string;\n"
        "    }\n"
        "    leaf p4 {\n"
        "      type string;\n"
        "    }\n"
        "    leaf p5 {\n"
        "      type string;\n"
        "    }\n"
        "  }\n"
        "}\n";

/**
 * @brief Compare the compact JSON print of a data tree with an expected string.
 */
static void
check_json_tree(const struct lyd_node *tree, const char *expected)
{
    char *buffer = NULL;

    assert_int_equal(LY_SUCCESS, lyd_print_mem(&buffer, (struct lyd_node *)tree, LYD_JSON, LYD_PRINT_SHRINK));
    CHECK_STRING(buffer, expected);
    free(buffer);
}

/**
 * @brief Create a context with the given module implemented and ietf-sid-file loaded.
 */
static void
new_sid_ctx(struct ly_ctx **ctx, const char *mod_str, struct lys_module **mod)
{
    struct ly_in *in;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(TESTS_SRC "/../modules", LY_CTX_DISABLE_SEARCHDIR_CWD, ctx));
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(mod_str, &in));
    assert_int_equal(LY_SUCCESS, lys_parse(*ctx, in, LYS_IN_YANG, NULL, mod));
    ly_in_free(in, 0);
    assert_non_null(ly_ctx_load_module(*ctx, "ietf-sid-file", NULL, NULL));
}

/**
 * @brief Check and clear the last error logged in @p ctx (CHECK_LOG_CTX for private contexts).
 */
static void
check_log_ctx(struct ly_ctx *ctx, const char *msg)
{
    struct ly_err_item *e = (struct ly_err_item *)ly_err_last(ctx);

    assert_non_null(e);
    CHECK_STRING(e->msg, msg);
    assert_null(e->data_path);
    assert_null(e->schema_path);
    assert_int_equal(e->line, 0);
    ly_err_clean(ctx, e);
}

/**
 * @brief Test case: argument checks of the public API functions lys_sid_gen(),
 * lys_sid_update() and lys_sid_range_add(). These fail in the API wrapper without
 * logging any error into the context.
 */
static void
test_args(void **state)
{
    struct lys_module *mod;
    struct lyd_node *sid_file = NULL, *updated = NULL;

    UTEST_ADD_MODULE(mod_v1, LYS_IN_YANG, NULL, &mod);

    /* lys_sid_gen */
    assert_int_equal(LY_EINVAL, lys_sid_gen(NULL, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_int_equal(LY_EINVAL, lys_sid_gen(mod, 100, 0, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_int_equal(LY_EINVAL, lys_sid_gen(mod, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", NULL));

    /* the ietf-sid-file module is not implemented in the context yet */
    assert_int_equal(LY_ENOTFOUND, lys_sid_gen(mod, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_non_null(ly_ctx_load_module(_UC->ctx, "ietf-sid-file", NULL, NULL));
    assert_int_equal(LY_SUCCESS, lys_sid_gen(mod, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_non_null(sid_file);

    /* lys_sid_update */
    assert_int_equal(LY_EINVAL, lys_sid_update(NULL, sid_file, LYS_SID_FILE_UNPUBLISHED, "d", &updated));
    assert_int_equal(LY_EINVAL, lys_sid_update(mod, NULL, LYS_SID_FILE_UNPUBLISHED, "d", &updated));
    assert_int_equal(LY_EINVAL, lys_sid_update(mod, sid_file, LYS_SID_FILE_UNPUBLISHED, "d", NULL));

    /* lys_sid_range_add */
    assert_int_equal(LY_EINVAL, lys_sid_range_add(NULL, 200, 1));
    assert_int_equal(LY_EINVAL, lys_sid_range_add(sid_file, 200, 0));

    lyd_free_all(sid_file);
}

/**
 * @brief Test case: lys_sid_gen() fails on an undersized assignment range
 * and generates the expected data tree with a sufficient one.
 */
static void
test_gen(void **state)
{
    struct lys_module *mod;
    struct lyd_node *sid_file = NULL;

    UTEST_ADD_MODULE(mod_v1, LYS_IN_YANG, NULL, &mod);
    assert_non_null(ly_ctx_load_module(_UC->ctx, "ietf-sid-file", NULL, NULL));

    /* the module has 3 SID items but the range covers only 2 */
    assert_int_equal(LY_EINVAL, lys_sid_gen(mod, 100, 2, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_null(sid_file);
    CHECK_LOG_CTX(".sid assignment range(s) size 2 is too small, 1 extra SID(s) are required.", NULL, 0);

    /* exact range size */
    assert_int_equal(LY_SUCCESS, lys_sid_gen(mod, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_non_null(sid_file);
    check_json_tree(sid_file,
            "{\"ietf-sid-file:sid-file\":{"
            "\"module-name\":\"a01\","
            "\"module-revision\":\"2024-01-01\","
            "\"sid-file-status\":\"unpublished\","
            "\"description\":\"d\","
            "\"assignment-range\":[{\"entry-point\":\"100\",\"size\":\"3\"}],"
            "\"item\":["
            "{\"namespace\":\"module\",\"identifier\":\"a01\",\"status\":\"unstable\",\"sid\":\"100\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont\",\"status\":\"unstable\",\"sid\":\"101\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont/l\",\"status\":\"unstable\",\"sid\":\"102\"}"
            "]}}");

    lyd_free_all(sid_file);
}

/**
 * @brief Test case: chained update scenario on a .sid file: undersized
 * updates fail, assignment ranges are added (zero-size and overlapping rejected),
 * an update with removed/added items succeeds and is finally republished as stable.
 * A last update fails on range exhaustion: obsolete items keep holding their SIDs,
 * so no free SID is left in any range even though the raw item count fits.
 */
static void
test_flow(void **state)
{
    struct lys_module *mod, *mod2, *mod3, *mod4;
    struct ly_ctx *ctx2 = NULL, *ctx3 = NULL, *ctx4 = NULL;
    struct lyd_node *prev = NULL, *updated = NULL, *published = NULL;
    struct lyd_node *node;
    char *json = NULL;

    /* --- stage with v1 ---
    generate the initial .sid file with the range [100, 102] */
    UTEST_ADD_MODULE(mod_v1, LYS_IN_YANG, NULL, &mod);
    assert_non_null(ly_ctx_load_module(_UC->ctx, "ietf-sid-file", NULL, NULL));
    assert_int_equal(LY_SUCCESS, lys_sid_gen(mod, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", &prev));
    assert_non_null(prev);
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&json, prev, LYD_JSON, LYD_PRINT_SHRINK));
    lyd_free_all(prev);
    prev = NULL;

    /* --- stage with v2 --- */
    new_sid_ctx(&ctx2, mod_v2, &mod2);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx2, json, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &prev));
    free(json);
    json = NULL;

    /* v2 has 5 items but the ranges cover only 3 */
    assert_int_equal(LY_EINVAL, lys_sid_update(mod2, prev, LYS_SID_FILE_UNPUBLISHED, "d", &updated));
    assert_null(updated);
    check_log_ctx(ctx2, ".sid assignment range(s) size 3 is too small, 2 extra SID(s) are required.");

    /* overlapping range rejected */
    assert_int_equal(LY_EINVAL, lys_sid_range_add(prev, 102, 5));
    check_log_ctx(ctx2, "The assignment range [102, 106] overlaps the existing assignment range [100, 102].");

    /* valid but still insufficient range is appended */
    assert_int_equal(LY_SUCCESS, lys_sid_range_add(prev, 200, 1));
    assert_int_equal(LY_SUCCESS,
            lyd_find_path(prev, "/ietf-sid-file:sid-file/assignment-range[entry-point='100']/size", 0, &node));
    CHECK_STRING(lyd_get_value(node), "3");
    assert_int_equal(LY_SUCCESS,
            lyd_find_path(prev, "/ietf-sid-file:sid-file/assignment-range[entry-point='200']/size", 0, &node));
    CHECK_STRING(lyd_get_value(node), "1");

    /* total capacity 4 still does not cover the 5 items of v2 */
    assert_int_equal(LY_EINVAL, lys_sid_update(mod2, prev, LYS_SID_FILE_UNPUBLISHED, "d", &updated));
    assert_null(updated);
    check_log_ctx(ctx2, ".sid assignment range(s) size 4 is too small, 1 extra SID(s) are required.");

    /* another range provides enough SIDs */
    assert_int_equal(LY_SUCCESS, lys_sid_range_add(prev, 300, 5));

    /* carry the .sid file over to the next stage */
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&json, prev, LYD_JSON, LYD_PRINT_SHRINK));
    lyd_free_all(prev);
    prev = NULL;
    ly_ctx_destroy(ctx2);
    ctx2 = NULL;

    /* --- stage with v3 --- */
    new_sid_ctx(&ctx3, mod_v3, &mod3);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx3, json, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &prev));
    free(json);
    json = NULL;

    /* update to v3 - drops /a01:cont/l (kept as obsolete), adds /a01:cont/m and /a01:cont/n
       which get the next free SIDs walking the ranges forward (200, then 300) */
    assert_int_equal(LY_SUCCESS, lys_sid_update(mod3, prev, LYS_SID_FILE_UNPUBLISHED, "d", &updated));
    assert_non_null(updated);
    check_json_tree(updated,
            "{\"ietf-sid-file:sid-file\":{"
            "\"module-name\":\"a01\","
            "\"module-revision\":\"2024-03-03\","
            "\"sid-file-version\":1,"
            "\"sid-file-status\":\"unpublished\","
            "\"description\":\"d\","
            "\"assignment-range\":["
            "{\"entry-point\":\"100\",\"size\":\"3\"},"
            "{\"entry-point\":\"200\",\"size\":\"1\"},"
            "{\"entry-point\":\"300\",\"size\":\"5\"}],"
            "\"item\":["
            "{\"namespace\":\"module\",\"identifier\":\"a01\",\"status\":\"unstable\",\"sid\":\"100\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont\",\"status\":\"unstable\",\"sid\":\"101\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont/l\",\"status\":\"obsolete\",\"sid\":\"102\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont/m\",\"status\":\"unstable\",\"sid\":\"200\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont/n\",\"status\":\"unstable\",\"sid\":\"300\"}"
            "]}}");
    lyd_free_all(prev);
    prev = NULL;

    /* republish the file as stable - the version is incremented, the sid-file-status leaf
       and unstable item statuses disappear, the obsolete item is preserved */
    assert_int_equal(LY_SUCCESS, lys_sid_update(mod3, updated, LYS_SID_FILE_PUBLISHED, "d", &published));
    assert_non_null(published);
    check_json_tree(published,
            "{\"ietf-sid-file:sid-file\":{"
            "\"module-name\":\"a01\","
            "\"module-revision\":\"2024-03-03\","
            "\"sid-file-version\":2,"
            "\"description\":\"d\","
            "\"assignment-range\":["
            "{\"entry-point\":\"100\",\"size\":\"3\"},"
            "{\"entry-point\":\"200\",\"size\":\"1\"},"
            "{\"entry-point\":\"300\",\"size\":\"5\"}],"
            "\"item\":["
            "{\"namespace\":\"module\",\"identifier\":\"a01\",\"sid\":\"100\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont\",\"sid\":\"101\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont/l\",\"status\":\"obsolete\",\"sid\":\"102\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont/m\",\"sid\":\"200\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/a01:cont/n\",\"sid\":\"300\"}"
            "]}}");
    lyd_free_all(updated);
    updated = NULL;

    /* --- stage with v4 --- */
    /* v4 has 7 items, so the raw size check (capacity 9) passes, but the obsolete
       items l/m/n still lock SIDs 102/200/300 and only 301-304 are free - the 5th
       new leaf finds no free SID in any assignment range */
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&json, published, LYD_JSON, LYD_PRINT_SHRINK));
    lyd_free_all(published);
    ly_ctx_destroy(ctx3);

    new_sid_ctx(&ctx4, mod_v4, &mod4);
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx4, json, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &prev));
    free(json);
    json = NULL;

    assert_int_equal(LY_EINVAL, lys_sid_update(mod4, prev, LYS_SID_FILE_UNPUBLISHED, "d", &updated));
    assert_null(updated);
    check_log_ctx(ctx4, ".sid assignment range(s) size 9 is too small, 1 extra SID(s) are required.");

    lyd_free_all(prev);
    ly_ctx_destroy(ctx4);
}

/**
 * @brief Test case: SID items are also collected from the data trees of extension
 * instances that define their own data trees outside the standard module trees -
 * sx:structure (with the contents of the merged sx:augment-structure; the structure
 * root itself, the synthetic top container named by the extension argument, gets no
 * SID and only prefixes the descendant identifiers) and rc:yang-data (the template
 * container is the data tree root, so the extension argument is not among the items).
 */
static void
test_exts(void **state)
{
    struct lys_module *mod;
    struct lyd_node *sid_file = NULL;
    static const char *mod_struct =
            "module s1 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:s1\";\n"
            "  prefix s1;\n"
            "  import ietf-yang-structure-ext { prefix sx; }\n"
            "  revision 2024-01-01;\n"
            "  container c {\n"
            "    leaf l { type string; }\n"
            "  }\n"
            "  sx:structure struct {\n"
            "    container x {\n"
            "      list l {\n"
            "        key \"id\";\n"
            "        leaf id { type uint32; }\n"
            "        leaf v { type string; }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  sx:augment-structure \"/s1:struct/x\" {\n"
            "    leaf al { type string; }\n"
            "  }\n"
            "}\n";
    static const char *mod_yangdata =
            "module yd1 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:yd1\";\n"
            "  prefix yd1;\n"
            "  import ietf-restconf { revision-date 2017-01-26; prefix rc; }\n"
            "  revision 2024-01-01;\n"
            "  rc:yang-data template {\n"
            "    container y {\n"
            "      leaf l { type string; }\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* sx:structure: 8 SID items, the root "struct" itself is not included (the structure
       storage points below the synthetic top container), augments merged in */
    UTEST_ADD_MODULE(mod_struct, LYS_IN_YANG, NULL, &mod);
    assert_non_null(ly_ctx_load_module(_UC->ctx, "ietf-sid-file", NULL, NULL));
    assert_int_equal(LY_SUCCESS, lys_sid_gen(mod, 100, 8, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_non_null(sid_file);
    check_json_tree(sid_file,
            "{\"ietf-sid-file:sid-file\":{"
            "\"module-name\":\"s1\","
            "\"module-revision\":\"2024-01-01\","
            "\"sid-file-status\":\"unpublished\","
            "\"description\":\"d\","
            "\"dependency-revision\":[{\"module-name\":\"ietf-yang-structure-ext\",\"module-revision\":\"2020-06-17\"}],"
            "\"assignment-range\":[{\"entry-point\":\"100\",\"size\":\"8\"}],"
            "\"item\":["
            "{\"namespace\":\"module\",\"identifier\":\"s1\",\"status\":\"unstable\",\"sid\":\"100\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/s1:c\",\"status\":\"unstable\",\"sid\":\"101\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/s1:c/l\",\"status\":\"unstable\",\"sid\":\"102\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/s1:struct/x\",\"status\":\"unstable\",\"sid\":\"103\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/s1:struct/x/al\",\"status\":\"unstable\",\"sid\":\"104\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/s1:struct/x/l\",\"status\":\"unstable\",\"sid\":\"105\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/s1:struct/x/l/id\",\"status\":\"unstable\",\"sid\":\"106\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/s1:struct/x/l/v\",\"status\":\"unstable\",\"sid\":\"107\"}"
            "]}}");
    lyd_free_all(sid_file);
    sid_file = NULL;

    /* rc:yang-data: 3 SID items, the "template" name is not among them */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(_UC->ctx, TESTS_DIR_MODULES_YANG));
    UTEST_ADD_MODULE(mod_yangdata, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_SUCCESS, lys_sid_gen(mod, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_non_null(sid_file);
    check_json_tree(sid_file,
            "{\"ietf-sid-file:sid-file\":{"
            "\"module-name\":\"yd1\","
            "\"module-revision\":\"2024-01-01\","
            "\"sid-file-status\":\"unpublished\","
            "\"description\":\"d\","
            "\"dependency-revision\":[{\"module-name\":\"ietf-restconf\",\"module-revision\":\"2017-01-26\"}],"
            "\"assignment-range\":[{\"entry-point\":\"100\",\"size\":\"3\"}],"
            "\"item\":["
            "{\"namespace\":\"module\",\"identifier\":\"yd1\",\"status\":\"unstable\",\"sid\":\"100\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/yd1:y\",\"status\":\"unstable\",\"sid\":\"101\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/yd1:y/l\",\"status\":\"unstable\",\"sid\":\"102\"}"
            "]}}");
    lyd_free_all(sid_file);
}

/**
 * @brief Test case: augment coverage. A module's .sid file must include the data
 * nodes it augments into another module (RFC 9595), and a module's own .sid must
 * not include nodes augmented into it by other modules.
 */
static void
test_augment(void **state)
{
    struct lys_module *base, *aug;
    struct lyd_node *sid_file = NULL;
    static const char *mod_base =
            "module b1 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:b1\";\n"
            "  prefix b1;\n"
            "  revision 2024-01-01;\n"
            "  container cont {\n"
            "    leaf l { type string; }\n"
            "  }\n"
            "}\n";
    static const char *mod_aug =
            "module a1 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:a1\";\n"
            "  prefix a1;\n"
            "  import b1 { prefix b1; }\n"
            "  revision 2024-01-01;\n"
            "  augment \"/b1:cont\" {\n"
            "    leaf x { type string; }\n"
            "  }\n"
            "}\n";

    UTEST_ADD_MODULE(mod_base, LYS_IN_YANG, NULL, &base);
    UTEST_ADD_MODULE(mod_aug, LYS_IN_YANG, NULL, &aug);
    assert_non_null(ly_ctx_load_module(_UC->ctx, "ietf-sid-file", NULL, NULL));

    /* the augmenting module's .sid contains the node it augments in: /b1:cont/a1:x */
    assert_int_equal(LY_SUCCESS, lys_sid_gen(aug, 100, 2, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_non_null(sid_file);
    check_json_tree(sid_file,
            "{\"ietf-sid-file:sid-file\":{"
            "\"module-name\":\"a1\","
            "\"module-revision\":\"2024-01-01\","
            "\"sid-file-status\":\"unpublished\","
            "\"description\":\"d\","
            "\"dependency-revision\":[{\"module-name\":\"b1\",\"module-revision\":\"2024-01-01\"}],"
            "\"assignment-range\":[{\"entry-point\":\"100\",\"size\":\"2\"}],"
            "\"item\":["
            "{\"namespace\":\"module\",\"identifier\":\"a1\",\"status\":\"unstable\",\"sid\":\"100\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/b1:cont/a1:x\",\"status\":\"unstable\",\"sid\":\"101\"}"
            "]}}");
    lyd_free_all(sid_file);
    sid_file = NULL;

    /* the base module's own .sid must not contain the node augmented in by a1 */
    assert_int_equal(LY_SUCCESS, lys_sid_gen(base, 100, 3, LYS_SID_FILE_UNPUBLISHED, "d", &sid_file));
    assert_non_null(sid_file);
    check_json_tree(sid_file,
            "{\"ietf-sid-file:sid-file\":{"
            "\"module-name\":\"b1\","
            "\"module-revision\":\"2024-01-01\","
            "\"sid-file-status\":\"unpublished\","
            "\"description\":\"d\","
            "\"assignment-range\":[{\"entry-point\":\"100\",\"size\":\"3\"}],"
            "\"item\":["
            "{\"namespace\":\"module\",\"identifier\":\"b1\",\"status\":\"unstable\",\"sid\":\"100\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/b1:cont\",\"status\":\"unstable\",\"sid\":\"101\"},"
            "{\"namespace\":\"data\",\"identifier\":\"/b1:cont/l\",\"status\":\"unstable\",\"sid\":\"102\"}"
            "]}}");
    lyd_free_all(sid_file);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_args),
        UTEST(test_gen),
        UTEST(test_flow),
        UTEST(test_exts),
        UTEST(test_augment),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
