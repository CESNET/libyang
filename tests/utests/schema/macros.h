/*
 * @file macros.h
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief macros for schema tests
 *
 * Copyright (c) 2018-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#ifndef TESTS_UTESTS_SCHEMA_MACROS_H_
#define TESTS_UTESTS_SCHEMA_MACROS_H_

#define TEST_YANG_MODULE_10(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "module "MOD_NAME" { namespace "MOD_NS"; prefix "MOD_PREFIX"; "CONTENT"}"

#define TEST_YANG_MODULE_11(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "module "MOD_NAME" {yang-version 1.1; namespace "MOD_NS"; prefix "MOD_PREFIX"; "CONTENT"}"

#define TEST_YIN_MODULE_10(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\""MOD_NAME"\">" \
    "<namespace uri=\""MOD_NS"\"/><prefix value=\""MOD_PREFIX"\"/>"CONTENT"</module>"

#define TEST_YIN_MODULE_11(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\""MOD_NAME"\"><yang-version value=\"1.1\"/>" \
    "<namespace uri=\""MOD_NS"\"/><prefix value=\""MOD_PREFIX"\"/>"CONTENT"</module>"

#define TEST_SCHEMA_STR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, STR) \
    if (YIN) { \
        if (RFC7950) { \
            STR = TEST_YIN_MODULE_11(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } else { \
            STR = TEST_YIN_MODULE_10(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } \
    } else { /* YANG */ \
        if (RFC7950) { \
            STR = TEST_YANG_MODULE_11(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } else { \
            STR = TEST_YANG_MODULE_10(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } \
    }

#define TEST_SCHEMA_OK(CTX, RFC7950, YIN, MOD_NAME, CONTENT, RESULT) \
    { \
    const char *test_str__; \
    TEST_SCHEMA_STR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, test_str__) \
    assert_non_null(RESULT = lys_parse_mem(CTX, test_str__, YIN ? LYS_IN_YIN : LYS_IN_YANG)); \
    }

#define TEST_SCHEMA_ERR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, ERRMSG) \
    { \
    const char *test_str__; \
    TEST_SCHEMA_STR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, test_str__) \
    assert_null(lys_parse_mem(CTX, test_str__, YIN ? LYS_IN_YIN : LYS_IN_YANG)); \
    logbuf_assert(ERRMSG); \
    }

#define TEST_STMT_DUP(CTX, RFC7950, YIN, STMT, MEMBER, VALUE1, VALUE2, LINE) \
    if (YIN) { \
        TEST_SCHEMA_ERR(CTX, RFC7950, YIN, "dup", "", "Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    } else { \
        TEST_SCHEMA_ERR(CTX, RFC7950, YIN, "dup", STMT"{"MEMBER" "VALUE1";"MEMBER" "VALUE2";}", \
                        "Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    }

#define TEST_STMT_SUBSTM_ERR(CTX, RFC7950, STMT, SUBSTMT, VALUE); \
        TEST_SCHEMA_ERR(CTX, RFC7950, 0, "inv", STMT" test {"SUBSTMT" "VALUE";}", "Invalid keyword \""SUBSTMT"\" as a child of \""STMT"\". Line number 1.");

#endif /* TESTS_UTESTS_SCHEMA_MACROS_H_ */
