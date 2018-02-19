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

#include <Libyang.hpp>
#include <Tree_Data.hpp>
#include <Tree_Schema.hpp>

#include "../tests/config.h"
#include "libyang.h"

/* include private header to be able to check internal values */
#include "../../src/context.h"

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
        ASSERT_FALSE(e.what());
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
        return;
    }
}

TEST_MAIN();
