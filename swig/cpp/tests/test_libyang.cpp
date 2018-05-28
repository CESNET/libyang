/*
 * @file test_libyang.cpp
 * @author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief unit tests for functions from libyang.h header
 *
 * Copyright (C) 2018 Deutsche Telekom AG.
 *
 * Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string.h>

#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"

#include "../tests/config.h"
#include "libyang.h"

/* include private header to be able to check internal values */
#include "../../../src/context.h"

#include "microtest.h"

TEST(test_ly_ctx_new)
{
    const char *yang_folder1 = TESTS_DIR "/data/files";
    const char *yang_folder2 = TESTS_DIR "/data:" TESTS_DIR "/data/files";

    try {
        auto ctx = S_Context(new Context(yang_folder1));
        ASSERT_FALSE(nullptr == ctx);
        auto list = std::shared_ptr<std::vector<std::string>>(ctx->get_searchdirs());
        ASSERT_FALSE(nullptr == list);
        ASSERT_EQ(1, list->size());

        ctx = S_Context(new Context(yang_folder2));
        ASSERT_FALSE(nullptr == ctx);
        list = std::shared_ptr<std::vector<std::string>>(ctx->get_searchdirs());
        ASSERT_FALSE(nullptr == list);
        ASSERT_EQ(2, list->size());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_new_invalid)
{
    const char *yang_folder = "INVALID_PATH";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_FALSE("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), "No Context"));
        return;
    }
}

TEST(test_ly_ctx_get_searchdirs)
{
    const char *yang_folder = TESTS_DIR "/data/files";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_FALSE(nullptr == ctx);

        auto list = std::shared_ptr<std::vector<std::string>>(ctx->get_searchdirs());
        ASSERT_EQ(1, list->size());
        ASSERT_EQ(yang_folder, list->at(0));
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_set_searchdir)
{
    const char *yang_folder = TESTS_DIR "/data/files";
    const char *new_yang_folder = TESTS_DIR "/schema/yin";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_FALSE(nullptr == ctx);

        auto list = std::shared_ptr<std::vector<std::string>>(ctx->get_searchdirs());
        ASSERT_EQ(1, list->size());
        ASSERT_EQ(yang_folder, list->at(0));

        ctx->set_searchdir(new_yang_folder);
        list = std::shared_ptr<std::vector<std::string>>(ctx->get_searchdirs());
        ASSERT_EQ(2, list->size());
        ASSERT_EQ(new_yang_folder, list->at(1));

        ctx->unset_searchdirs(0);
        list = std::shared_ptr<std::vector<std::string>>(ctx->get_searchdirs());
        ASSERT_EQ(1, list->size());
        ASSERT_EQ(new_yang_folder, list->at(0));
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_set_searchdir_invalid)
{
    const char *yang_folder = TESTS_DIR "/data/files";
    const char *new_yang_folder = TESTS_DIR "INVALID_PATH";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_FALSE(nullptr == ctx);

        ctx->set_searchdir(new_yang_folder);
        throw std::runtime_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), new_yang_folder));
        return;
    }
}

TEST(test_ly_ctx_info)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_FALSE(nullptr == ctx);

        auto info = ctx->info();
        ASSERT_FALSE(nullptr == info);
        ASSERT_EQ(LYD_VAL_OK, info->validity());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_load_module_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_FALSE(nullptr == ctx);

        auto module = ctx->load_module("invalid", nullptr);
        throw std::runtime_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), "invalid"));
        return;
    }
}

TEST(test_ly_ctx_load_get_module)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *name1 = "a";
    const char *name2 = "b";
    const char *revision = "2016-03-01";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);

        auto module = ctx->get_module("invalid");
        ASSERT_NULL(module);

        module = ctx->get_module(name1);
        ASSERT_NULL(module);

        module = ctx->load_module(name1);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name1, module->name());

        module = ctx->load_module(name2, revision);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name2, module->name());
        ASSERT_STREQ(revision, module->rev()->date());

        module = ctx->get_module(name2, "INVALID_REVISION");
        ASSERT_NULL(module);

        module = ctx->get_module(name1);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name1, module->name());

        module = ctx->get_module(name2, revision);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name2, module->name());
        ASSERT_STREQ(revision, module->rev()->date());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_get_module_older)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *name = "b";
    const char *revision = "2016-03-01";
    const char *revision_older = "2015-01-01";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module("c");
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("c", module->name());

        module = ctx->load_module(name, revision);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name, module->name());
        ASSERT_STREQ(revision, module->rev()->date());

        auto module_older = ctx->get_module_older(module);
        ASSERT_NOTNULL(module_older);
        ASSERT_STREQ(name, module_older->name());
        ASSERT_STREQ(revision_older, module_older->rev()->date());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_get_module_by_ns)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "a";
    const char *ns = "urn:a";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        module = ctx->get_module_by_ns(ns);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_clean)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "a";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        module = ctx->get_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        ctx->clean();

        module = ctx->get_module(module_name);
        ASSERT_NULL(module);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_parse_module_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *yang_file = TESTS_DIR "/api/files/b.yang";
    const char *module_name1 = "a";
    const char *module_name2 = "b";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);

        auto module = ctx->parse_module_path(yin_file, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name1, module->name());

        module = ctx->parse_module_path(yang_file, LYS_IN_YANG);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name2, module->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_parse_module_path_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);

        auto module = ctx->parse_module_path("INVALID_YANG_FILE", LYS_IN_YANG);
        throw std::logic_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), "INVALID_YANG_FILE"));
        return;
    }
}

TEST(test_ly_ctx_get_submodule)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *module_name = "a";
    const char *sub_name = "asub";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);

        auto submodule = ctx->get_submodule(module_name, nullptr, sub_name, nullptr);
        ASSERT_NOTNULL(submodule);
        ASSERT_STREQ(sub_name, submodule->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_get_submodule2)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *config_file = TESTS_DIR "/api/files/a.xml";
    const char *sub_name = "asub";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);

        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);
        ASSERT_NOTNULL(root->schema()->module());

        auto submodule = ctx->get_submodule2(root->schema()->module(), sub_name);
        ASSERT_NOTNULL(submodule);
        ASSERT_STREQ(sub_name, submodule->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_ctx_find_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *yang_file = TESTS_DIR "/api/files/b.yang";
    const char *schema_path1 = "/b:x/b:bubba";
    const char *schema_path2 = "/a:x/a:bubba";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);

        ctx->parse_module_path(yang_file, LYS_IN_YANG);
        auto set = ctx->find_path(schema_path1);
        ASSERT_NOTNULL(set);

        ctx->parse_module_path(yin_file, LYS_IN_YIN);
        set = ctx->find_path(schema_path2);
        ASSERT_NOTNULL(set);
        S_Set(new Set());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST(test_ly_set)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = S_Context(new Context(yang_folder));
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto set = S_Set(new Set());
        ASSERT_NOTNULL(set);
        ASSERT_EQ(0, set->number());

        set->add(root->child()->schema());
        ASSERT_EQ(1, set->number());

        set->add(root->schema());
        ASSERT_EQ(2, set->number());

        set->rm(root->schema());
        ASSERT_EQ(1, set->number());

        set->add(root->schema());
        ASSERT_EQ(2, set->number());

        set->rm_index(1);
        ASSERT_EQ(1, set->number());

        set->clean();
        ASSERT_EQ(0, set->number());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        return;
    }
}

TEST_MAIN();
