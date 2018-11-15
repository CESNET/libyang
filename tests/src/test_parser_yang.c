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
#include "../../src/set.c"
#include "../../src/log.c"
#include "../../src/hash_table.c"
#include "../../src/xpath.c"
#include "../../src/parser_yang.c"
#include "../../src/context.c"
#include "../../src/tree_schema_helpers.c"
#include "../../src/tree_schema.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    if (store) {
        if (path && path[0]) {
            snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
        } else {
            strncpy(logbuf, msg, BUFSIZE - 1);
        }
        if (store > 0) {
            --store;
        }
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

#define TEST_DUP_GENERIC(PREFIX, MEMBER, VALUE1, VALUE2, FUNC, RESULT, LINE, CLEANUP) \
    str = PREFIX MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, FUNC(&ctx, &str, RESULT)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    CLEANUP

static void
test_helpers(void **state)
{
    (void) state; /* unused */

    const char *str;
    char *buf, *p;
    size_t len, size;
    int prefix;
    struct ly_parser_ctx ctx;
    ctx.ctx = NULL;
    ctx.line = 1;

    /* storing into buffer */
    str = "abcd";
    buf = NULL;
    size = len = 0;
    assert_int_equal(LY_SUCCESS, buf_add_char(NULL, &str, 2, &buf, &size, &len));
    assert_int_not_equal(0, size);
    assert_int_equal(2, len);
    assert_string_equal("cd", str);
    assert_false(strncmp("ab", buf, 2));
    free(buf);
    buf = NULL;

    /* invalid first characters */
    len = 0;
    str = "2invalid";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &str, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1));
    str = ".invalid";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &str, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1));
    str = "-invalid";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &str, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1));
    /* invalid following characters */
    len = 3; /* number of characters read before the str content */
    str = "!";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &str, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1));
    str = ":";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &str, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1));
    /* valid colon for prefixed identifiers */
    len = size = 0;
    p = NULL;
    str = "x:id";
    assert_int_equal(LY_SUCCESS, buf_store_char(&ctx, &str, Y_PREF_IDENTIF_ARG, &p, &len, &buf, &size, 0));
    assert_int_equal(1, len);
    assert_null(buf);
    assert_string_equal(":id", str);
    assert_int_equal('x', p[len - 1]);
    assert_int_equal(LY_SUCCESS, buf_store_char(&ctx, &str, Y_PREF_IDENTIF_ARG, &p, &len, &buf, &size, 1));
    assert_int_equal(2, len);
    assert_string_equal("id", str);
    assert_int_equal(':', p[len - 1]);
    free(buf);

    /* checking identifiers */
    assert_int_equal(LY_EVALID, check_identifierchar(&ctx, ':', 0, NULL));
    logbuf_assert("Invalid identifier character ':'. Line number 1.");
    assert_int_equal(LY_EVALID, check_identifierchar(&ctx, '#', 1, NULL));
    logbuf_assert("Invalid identifier first character '#'. Line number 1.");

    assert_int_equal(LY_SUCCESS, check_identifierchar(&ctx, 'a', 1, &prefix));
    assert_int_equal(0, prefix);
    assert_int_equal(LY_SUCCESS, check_identifierchar(&ctx, ':', 0, &prefix));
    assert_int_equal(1, prefix);
    assert_int_equal(LY_EVALID, check_identifierchar(&ctx, ':', 0, &prefix));
    assert_int_equal(1, prefix);
    assert_int_equal(LY_SUCCESS, check_identifierchar(&ctx, 'b', 0, &prefix));
    assert_int_equal(2, prefix);
    /* second colon is invalid */
    assert_int_equal(LY_EVALID, check_identifierchar(&ctx, ':', 0, &prefix));
    logbuf_assert("Invalid identifier character ':'. Line number 1.");
}

static void
test_comments(void **state)
{
    (void) state; /* unused */

    struct ly_parser_ctx ctx;
    const char *str, *p;
    char *word, *buf;
    size_t len;

    ctx.ctx = NULL;
    ctx.line = 1;

    str = " // this is a text of / one * line */ comment\nargument";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_string_equal("argument", word);
    assert_null(buf);
    assert_int_equal(8, len);

    str = "/* this is a \n * text // of / block * comment */\"arg\" + \"ume\" \n + \n \"nt\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_string_equal("argument", word);
    assert_ptr_equal(buf, word);
    assert_int_equal(8, len);
    free(word);

    str = p = " this is one line comment on last line";
    assert_int_equal(LY_SUCCESS, skip_comment(&ctx, &str, 1));
    assert_true(str[0] == '\0');

    str = p = " this is a not terminated comment x";
    assert_int_equal(LY_EVALID, skip_comment(&ctx, &str, 2));
    logbuf_assert("Unexpected end-of-file, non-terminated comment. Line number 5.");
    assert_true(str[0] == '\0');
}

static void
test_arg(void **state)
{
    (void) state; /* unused */

    struct ly_parser_ctx ctx;
    const char *str;
    char *word, *buf;
    size_t len;

    ctx.ctx = NULL;
    ctx.line = 1;

    /* missing argument */
    str = ";";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_MAYBE_STR_ARG, &word, &buf, &len));
    assert_null(word);

    str = "{";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \"{\", expected an argument. Line number 1.");

    /* invalid escape sequence */
    str = "\"\\s\"";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    logbuf_assert("Double-quoted string unknown special character \'\\s\'. Line number 1.");
    str = "\'\\s\'"; /* valid, since it is not an escape sequence in single quoted string */
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_int_equal(2, len);
    assert_string_equal("\\s\'", word);
    assert_int_equal('\0', str[0]); /* input has been eaten */

    /* invalid character after the argument */
    str = "hello\"";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \"\"\", expected unquoted string character, optsep, semicolon or opening brace. Line number 1.");
    str = "hello}";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \"}\", expected unquoted string character, optsep, semicolon or opening brace. Line number 1.");

    str = "hello/x\t"; /* slash is not an invalid character */
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_int_equal(7, len);
    assert_string_equal("hello/x\t", word);

    assert_null(buf);

    /* different quoting */
    str = "hello ";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_null(buf);
    assert_int_equal(5, len);
    assert_string_equal("hello ", word);

    str = "hello/*comment*/\n";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_null(buf);
    assert_int_equal(5, len);
    assert_false(strncmp("hello", word, len));


    str = "\"hello\\n\\t\\\"\\\\\";";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_null(buf);
    assert_int_equal(9, len);
    assert_string_equal("hello\\n\\t\\\"\\\\\";", word);

    ctx.indent = 14;
    str = "\"hello \t\n\t\t world!\"";
    /* - space and tabs before newline are stripped out
     * - space and tabs after newline (indentation) are stripped out
     */
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_non_null(buf);
    assert_ptr_equal(word, buf);
    assert_int_equal(14, len);
    assert_string_equal("hello\n  world!", word);
    free(buf);

    ctx.indent = 14;
    str = "\"hello\n \tworld!\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_non_null(buf);
    assert_ptr_equal(word, buf);
    assert_int_equal(12, len);
    assert_string_equal("hello\nworld!", word);
    free(buf);

    str = "\'hello\'";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_null(buf);
    assert_int_equal(5, len);
    assert_false(strncmp("hello", word, 5));

    str = "\"hel\"  +\t\n\"lo\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_ptr_equal(word, buf);
    assert_int_equal(5, len);
    assert_string_equal("hello", word);
    free(buf);
    str = "\"hel\"  +\t\nlo"; /* unquoted the second part */
    assert_int_equal(LY_EVALID, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    logbuf_assert("Both string parts divided by '+' must be quoted. Line number 5.");

    str = "\'he\'\t\n+ \"llo\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_ptr_equal(word, buf);
    assert_int_equal(5, len);
    assert_string_equal("hello", word);
    free(buf);

    str = " \t\n\"he\"+\'llo\'";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    assert_ptr_equal(word, buf);
    assert_int_equal(5, len);
    assert_string_equal("hello", word);
    free(buf);

    /* missing argument */
    str = ";";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &str, Y_STR_ARG, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \";\", expected an argument. Line number 7.");
}

static void
test_stmts(void **state)
{
    (void) state; /* unused */

    struct ly_parser_ctx ctx;
    const char *str, *p;
    enum yang_keyword kw;
    char *word;
    size_t len;

    ctx.ctx = NULL;
    ctx.line = 1;

    str = "\n// comment\n\tinput\t{";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_INPUT, kw);
    assert_int_equal(5, len);
    assert_string_equal("input\t{", word);
    assert_string_equal("\t{", str);

    str = "\t /* comment */\t output\n\t{";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_OUTPUT, kw);
    assert_int_equal(6, len);
    assert_string_equal("output\n\t{", word);
    assert_string_equal("\n\t{", str);

    str = "/input { "; /* invalid slash */
    assert_int_equal(LY_EVALID, get_keyword(&ctx, &str, &kw, &word, &len));
    logbuf_assert("Invalid identifier first character '/'. Line number 4.");

    str = "not-a-statement-nor-extension { "; /* invalid identifier */
    assert_int_equal(LY_EVALID, get_keyword(&ctx, &str, &kw, &word, &len));
    logbuf_assert("Invalid character sequence \"not-a-statement-nor-extension\", expected a keyword. Line number 4.");

    str = "path;"; /* missing sep after the keyword */
    assert_int_equal(LY_EVALID, get_keyword(&ctx, &str, &kw, &word, &len));
    logbuf_assert("Invalid character sequence \"path;\", expected a keyword followed by a separator. Line number 4.");

    str = "action ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ACTION, kw);
    assert_int_equal(6, len);
    str = "anydata ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ANYDATA, kw);
    assert_int_equal(7, len);
    str = "anyxml ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ANYXML, kw);
    assert_int_equal(6, len);
    str = "argument ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ARGUMENT, kw);
    assert_int_equal(8, len);
    str = "augment ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_AUGMENT, kw);
    assert_int_equal(7, len);
    str = "base ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_BASE, kw);
    assert_int_equal(4, len);
    str = "belongs-to ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_BELONGS_TO, kw);
    assert_int_equal(10, len);
    str = "bit ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_BIT, kw);
    assert_int_equal(3, len);
    str = "case ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_CASE, kw);
    assert_int_equal(4, len);
    str = "choice ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_CHOICE, kw);
    assert_int_equal(6, len);
    str = "config ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_CONFIG, kw);
    assert_int_equal(6, len);
    str = "contact ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_CONTACT, kw);
    assert_int_equal(7, len);
    str = "container ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_CONTAINER, kw);
    assert_int_equal(9, len);
    str = "default ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_DEFAULT, kw);
    assert_int_equal(7, len);
    str = "description ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_DESCRIPTION, kw);
    assert_int_equal(11, len);
    str = "deviate ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_DEVIATE, kw);
    assert_int_equal(7, len);
    str = "deviation ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_DEVIATION, kw);
    assert_int_equal(9, len);
    str = "enum ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ENUM, kw);
    assert_int_equal(4, len);
    str = "error-app-tag ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ERROR_APP_TAG, kw);
    assert_int_equal(13, len);
    str = "error-message ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ERROR_MESSAGE, kw);
    assert_int_equal(13, len);
    str = "extension ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_EXTENSION, kw);
    assert_int_equal(9, len);
    str = "feature ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_FEATURE, kw);
    assert_int_equal(7, len);
    str = "fraction-digits ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_FRACTION_DIGITS, kw);
    assert_int_equal(15, len);
    str = "grouping ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_GROUPING, kw);
    assert_int_equal(8, len);
    str = "identity ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_IDENTITY, kw);
    assert_int_equal(8, len);
    str = "if-feature ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_IF_FEATURE, kw);
    assert_int_equal(10, len);
    str = "import ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_IMPORT, kw);
    assert_int_equal(6, len);
    str = "include ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_INCLUDE, kw);
    assert_int_equal(7, len);
    str = "input{";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_INPUT, kw);
    assert_int_equal(5, len);
    str = "key ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_KEY, kw);
    assert_int_equal(3, len);
    str = "leaf ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_LEAF, kw);
    assert_int_equal(4, len);
    str = "leaf-list ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_LEAF_LIST, kw);
    assert_int_equal(9, len);
    str = "length ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_LENGTH, kw);
    assert_int_equal(6, len);
    str = "list ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_LIST, kw);
    assert_int_equal(4, len);
    str = "mandatory ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_MANDATORY, kw);
    assert_int_equal(9, len);
    str = "max-elements ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_MAX_ELEMENTS, kw);
    assert_int_equal(12, len);
    str = "min-elements ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_MIN_ELEMENTS, kw);
    assert_int_equal(12, len);
    str = "modifier ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_MODIFIER, kw);
    assert_int_equal(8, len);
    str = "module ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_MODULE, kw);
    assert_int_equal(6, len);
    str = "must ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_MUST, kw);
    assert_int_equal(4, len);
    str = "namespace ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_NAMESPACE, kw);
    assert_int_equal(9, len);
    str = "notification ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_NOTIFICATION, kw);
    assert_int_equal(12, len);
    str = "ordered-by ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ORDERED_BY, kw);
    assert_int_equal(10, len);
    str = "organization ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_ORGANIZATION, kw);
    assert_int_equal(12, len);
    str = "output ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_OUTPUT, kw);
    assert_int_equal(6, len);
    str = "path ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_PATH, kw);
    assert_int_equal(4, len);
    str = "pattern ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_PATTERN, kw);
    assert_int_equal(7, len);
    str = "position ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_POSITION, kw);
    assert_int_equal(8, len);
    str = "prefix ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_PREFIX, kw);
    assert_int_equal(6, len);
    str = "presence ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_PRESENCE, kw);
    assert_int_equal(8, len);
    str = "range ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_RANGE, kw);
    assert_int_equal(5, len);
    str = "reference ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_REFERENCE, kw);
    assert_int_equal(9, len);
    str = "refine ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_REFINE, kw);
    assert_int_equal(6, len);
    str = "require-instance ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_REQUIRE_INSTANCE, kw);
    assert_int_equal(16, len);
    str = "revision ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_REVISION, kw);
    assert_int_equal(8, len);
    str = "revision-date ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_REVISION_DATE, kw);
    assert_int_equal(13, len);
    str = "rpc ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_RPC, kw);
    assert_int_equal(3, len);
    str = "status ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_STATUS, kw);
    assert_int_equal(6, len);
    str = "submodule ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_SUBMODULE, kw);
    assert_int_equal(9, len);
    str = "type ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_TYPE, kw);
    assert_int_equal(4, len);
    str = "typedef ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_TYPEDEF, kw);
    assert_int_equal(7, len);
    str = "unique ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_UNIQUE, kw);
    assert_int_equal(6, len);
    str = "units ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_UNITS, kw);
    assert_int_equal(5, len);
    str = "uses ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_USES, kw);
    assert_int_equal(4, len);
    str = "value ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_VALUE, kw);
    assert_int_equal(5, len);
    str = "when ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_WHEN, kw);
    assert_int_equal(4, len);
    str = "yang-version ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_YANG_VERSION, kw);
    assert_int_equal(12, len);
    str = "yin-element ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_YIN_ELEMENT, kw);
    assert_int_equal(11, len);
    str = ";config false;";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_SEMICOLON, kw);
    assert_int_equal(1, len);
    assert_string_equal("config false;", str);
    str = "{ config false;";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_LEFT_BRACE, kw);
    assert_int_equal(1, len);
    assert_string_equal(" config false;", str);
    str = "}";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_RIGHT_BRACE, kw);
    assert_int_equal(1, len);
    assert_string_equal("", str);

    /* geenric extension */
    str = p = "nacm:default-deny-write;";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_CUSTOM, kw);
    assert_int_equal(23, len);
    assert_ptr_equal(p, word);
}

static struct lysp_module *
mod_renew(struct ly_parser_ctx *ctx, struct lysp_module *mod, uint8_t submodule)
{
    lysp_module_free(mod);
    mod = calloc(1, sizeof *mod);
    mod->ctx = ctx->ctx;
    mod->submodule = submodule;
    assert_non_null(mod);
    ctx->mod = mod;
    return mod;
}

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

    struct ly_parser_ctx ctx;
    struct lysp_module *mod;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.line = 1;
    ctx.indent = 0;

    mod = mod_renew(&ctx, NULL, 0);

    /* missing mandatory substatements */
    str = " name {}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    assert_string_equal("name", mod->name);
    logbuf_assert("Missing mandatory keyword \"namespace\" as a child of \"module\". Line number 1.");
    mod = mod_renew(&ctx, mod, 0);

    str = " name {namespace urn:x;}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    assert_string_equal("urn:x", mod->ns);
    logbuf_assert("Missing mandatory keyword \"prefix\" as a child of \"module\". Line number 1.");
    mod = mod_renew(&ctx, mod, 0);

    str = " name {namespace urn:x;prefix \"x\";}";
    assert_int_equal(LY_SUCCESS, parse_sub_module(&ctx, &str, mod));
    assert_string_equal("x", mod->prefix);
    mod = mod_renew(&ctx, mod, 0);

#define SCHEMA_BEGINNING " name {yang-version 1.1;namespace urn:x;prefix \"x\";"
#define SCHEMA_BEGINNING2 " name {namespace urn:x;prefix \"x\";"
#define TEST_NODE(NODETYPE, INPUT, NAME) \
        str = SCHEMA_BEGINNING INPUT; \
        assert_int_equal(LY_SUCCESS, parse_sub_module(&ctx, &str, mod)); \
        assert_non_null(mod->data); \
        assert_int_equal(NODETYPE, mod->data->nodetype); \
        assert_string_equal(NAME, mod->data->name); \
        mod = mod_renew(&ctx, mod, 0);
#define TEST_GENERIC(INPUT, TARGET, TEST) \
        str = SCHEMA_BEGINNING INPUT; \
        assert_int_equal(LY_SUCCESS, parse_sub_module(&ctx, &str, mod)); \
        assert_non_null(TARGET); \
        TEST; \
        mod = mod_renew(&ctx, mod, 0);
#define TEST_DUP(MEMBER, VALUE1, VALUE2, LINE, SUBMODULE) \
        TEST_DUP_GENERIC(SCHEMA_BEGINNING, MEMBER, VALUE1, VALUE2, \
                         parse_sub_module, mod, LINE, mod = mod_renew(&ctx, mod, SUBMODULE))

    /* duplicated namespace, prefix */
    TEST_DUP("namespace", "y", "z", "1", 0);
    TEST_DUP("prefix", "y", "z", "1", 0);
    TEST_DUP("contact", "a", "b", "1", 0);
    TEST_DUP("description", "a", "b", "1", 0);
    TEST_DUP("organization", "a", "b", "1", 0);
    TEST_DUP("reference", "a", "b", "1", 0);

    /* not allowed in module (submodule-specific) */
    str = SCHEMA_BEGINNING "belongs-to master {prefix m;}}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Invalid keyword \"belongs-to\" as a child of \"module\". Line number 1.");
    mod = mod_renew(&ctx, mod, 0);

    /* anydata */
    TEST_NODE(LYS_ANYDATA, "anydata test;}", "test");
    /* anyxml */
    TEST_NODE(LYS_ANYXML, "anyxml test;}", "test");
    /* augment */
    TEST_GENERIC("augment /somepath;}", mod->augments,
                 assert_string_equal("/somepath", mod->augments[0].nodeid));
    /* choice */
    TEST_NODE(LYS_CHOICE, "choice test;}", "test");
    /* contact 0..1 */
    TEST_GENERIC("contact \"firstname\" + \n\t\" surname\";}", mod->contact,
                 assert_string_equal("firstname surname", mod->contact));
    /* container */
    TEST_NODE(LYS_CONTAINER, "container test;}", "test");
    /* description 0..1 */
    TEST_GENERIC("description \'some description\';}", mod->dsc,
                 assert_string_equal("some description", mod->dsc));
    /* deviation */
    TEST_GENERIC("deviation /somepath {deviate not-supported;}}", mod->deviations,
                 assert_string_equal("/somepath", mod->deviations[0].nodeid));
    /* extension */
    TEST_GENERIC("extension test;}", mod->extensions,
                 assert_string_equal("test", mod->extensions[0].name));
    /* feature */
    TEST_GENERIC("feature test;}", mod->features,
                 assert_string_equal("test", mod->features[0].name));
    /* grouping */
    TEST_GENERIC("grouping grp;}", mod->groupings,
                 assert_string_equal("grp", mod->groupings[0].name));
    /* identity */
    TEST_GENERIC("identity test;}", mod->identities,
                 assert_string_equal("test", mod->identities[0].name));
    /* import */
    ly_ctx_set_module_imp_clb(ctx.ctx, test_imp_clb, "module zzz { namespace urn:zzz; prefix z;}");
    TEST_GENERIC("import zzz {prefix z;}}", mod->imports,
                 assert_string_equal("zzz", mod->imports[0].name));

    /* import - prefix collision */
    str = SCHEMA_BEGINNING "import zzz {prefix x;}}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Prefix \"x\" already used as module prefix. Line number 2.");
    mod = mod_renew(&ctx, mod, 0);
    str = SCHEMA_BEGINNING "import zzz {prefix y;}import zzz {prefix y;}}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Prefix \"y\" already used to import \"zzz\" module. Line number 2.");
    mod = mod_renew(&ctx, mod, 0);
    str = "module" SCHEMA_BEGINNING "import zzz {prefix y;}import zzz {prefix z;}}";
    assert_null(lys_parse_mem(ctx.ctx, str, LYS_IN_YANG));
    assert_int_equal(LY_EVALID, ly_errcode(ctx.ctx));
    logbuf_assert("Single revision of the module \"zzz\" referred twice.");

    /* include */
    store = 1;
    ly_ctx_set_module_imp_clb(ctx.ctx, test_imp_clb, "module xxx { namespace urn:xxx; prefix x;}");
    str = "module" SCHEMA_BEGINNING "include xxx;}";
    assert_null(lys_parse_mem(ctx.ctx, str, LYS_IN_YANG));
    assert_int_equal(LY_EVALID, ly_errcode(ctx.ctx));
    logbuf_assert("Included \"xxx\" schema from \"name\" is actually not a submodule.");
    store = -1;

    store = 1;
    ly_ctx_set_module_imp_clb(ctx.ctx, test_imp_clb, "submodule xxx {belongs-to wrong-name {prefix w;}}");
    str = "module" SCHEMA_BEGINNING "include xxx;}";
    assert_null(lys_parse_mem(ctx.ctx, str, LYS_IN_YANG));
    assert_int_equal(LY_EVALID, ly_errcode(ctx.ctx));
    logbuf_assert("Included \"xxx\" submodule from \"name\" belongs-to a different module \"wrong-name\".");
    store = -1;

    ly_ctx_set_module_imp_clb(ctx.ctx, test_imp_clb, "submodule xxx {belongs-to name {prefix x;}}");
    TEST_GENERIC("include xxx;}", mod->includes,
                 assert_string_equal("xxx", mod->includes[0].name));

    /* leaf */
    TEST_NODE(LYS_LEAF, "leaf test {type string;}}", "test");
    /* leaf-list */
    TEST_NODE(LYS_LEAFLIST, "leaf-list test {type string;}}", "test");
    /* list */
    TEST_NODE(LYS_LIST, "list test {key a;leaf a {type string;}}}", "test");
    /* notification */
    TEST_GENERIC("notification test;}", mod->notifs,
                 assert_string_equal("test", mod->notifs[0].name));
    /* organization 0..1 */
    TEST_GENERIC("organization \"CESNET a.l.e.\";}", mod->org,
                 assert_string_equal("CESNET a.l.e.", mod->org));
    /* reference 0..1 */
    TEST_GENERIC("reference RFC7950;}", mod->ref,
                 assert_string_equal("RFC7950", mod->ref));
    /* revision */
    TEST_GENERIC("revision 2018-10-12;}", mod->revs,
                 assert_string_equal("2018-10-12", mod->revs[0].date));
    /* rpc */
    TEST_GENERIC("rpc test;}", mod->rpcs,
                 assert_string_equal("test", mod->rpcs[0].name));
    /* typedef */
    TEST_GENERIC("typedef test{type string;}}", mod->typedefs,
                 assert_string_equal("test", mod->typedefs[0].name));
    /* uses */
    TEST_NODE(LYS_USES, "uses test;}", "test");
    /* yang-version */
    str = SCHEMA_BEGINNING2 "\n\tyang-version 10;}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Invalid value \"10\" of \"yang-version\". Line number 3.");
    mod = mod_renew(&ctx, mod, 0);
    str = SCHEMA_BEGINNING2 "yang-version 1.0;yang-version 1.1;}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Duplicate keyword \"yang-version\". Line number 3.");
    mod = mod_renew(&ctx, mod, 0);
    str = SCHEMA_BEGINNING2 "yang-version 1.0;}";
    assert_int_equal(LY_SUCCESS, parse_sub_module(&ctx, &str, mod));
    assert_int_equal(1, mod->version);
    mod = mod_renew(&ctx, mod, 0);
    str = SCHEMA_BEGINNING2 "yang-version \"1.1\";}";
    assert_int_equal(LY_SUCCESS, parse_sub_module(&ctx, &str, mod));
    assert_int_equal(2, mod->version);
    mod = mod_renew(&ctx, mod, 0);

    /* extensions */
    TEST_GENERIC("prefix:test;}", mod->exts,
                 assert_string_equal("prefix:test", mod->exts[0].name);
                 assert_int_equal(LYEXT_SUBSTMT_SELF, mod->exts[0].insubstmt));
    mod = mod_renew(&ctx, mod, 0);

    /* invalid substatement */
    str = SCHEMA_BEGINNING "must false;}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Invalid keyword \"must\" as a child of \"module\". Line number 3.");
    mod = mod_renew(&ctx, mod, 0);

    /* submodule */
    mod->submodule = 1;

    /* missing mandatory substatements */
    str = " subname {}";
    lydict_remove(ctx.ctx, mod->name);
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    assert_string_equal("subname", mod->name);
    logbuf_assert("Missing mandatory keyword \"belongs-to\" as a child of \"submodule\". Line number 3.");
    mod = mod_renew(&ctx, mod, 1);

    str = " subname {belongs-to name {prefix x;}}";
    lydict_remove(ctx.ctx, mod->name);
    assert_int_equal(LY_SUCCESS, parse_sub_module(&ctx, &str, mod));
    assert_string_equal("name", mod->belongsto);
    mod = mod_renew(&ctx, mod, 1);

#undef SCHEMA_BEGINNING
#define SCHEMA_BEGINNING " subname {belongs-to name {prefix x;}"

    /* duplicated namespace, prefix */
    TEST_DUP("belongs-to", "module1", "module2", "3", 1);

    /* not allowed in submodule (module-specific) */
    str = SCHEMA_BEGINNING "namespace \"urn:z\";}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Invalid keyword \"namespace\" as a child of \"submodule\". Line number 3.");
    mod = mod_renew(&ctx, mod, 1);
    str = SCHEMA_BEGINNING "prefix m;}}";
    assert_int_equal(LY_EVALID, parse_sub_module(&ctx, &str, mod));
    logbuf_assert("Invalid keyword \"prefix\" as a child of \"submodule\". Line number 3.");
    mod = mod_renew(&ctx, mod, 1);

#undef TEST_GENERIC
#undef TEST_NODE
#undef TEST_DUP
#undef SCHEMA_BEGINNING

    lysp_module_free(mod);
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_identity(void **state)
{
    (void) state; /* unused */

    struct ly_parser_ctx ctx;
    struct lysp_module *mod = NULL, m = {0};
    struct lysp_ident *ident = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.line = 1;
    ctx.indent = 0;
    ctx.mod = &m;
    ctx.mod->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_DUP_GENERIC(" test {", MEMBER, VALUE1, VALUE2, parse_identity, \
                     &ident, "1", FREE_ARRAY(ctx.ctx, ident, lysp_ident_free); ident = NULL)

    TEST_DUP("description", "a", "b");
    TEST_DUP("reference", "a", "b");
    TEST_DUP("status", "current", "obsolete");

    /* full content */
    str = " test {base \"a\";base b; description text;reference \'another text\';status current; if-feature x;if-feature y;prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_identity(&ctx, &str, &ident));
    assert_non_null(ident);
    assert_string_equal(" ...", str);
    FREE_ARRAY(ctx.ctx, ident, lysp_ident_free);
    ident = NULL;

    /* invalid substatement */
    str = " test {organization XXX;}";
    assert_int_equal(LY_EVALID, parse_identity(&ctx, &str, &ident));
    logbuf_assert("Invalid keyword \"organization\" as a child of \"identity\". Line number 1.");
    FREE_ARRAY(ctx.ctx, ident, lysp_ident_free);
    ident = NULL;

    /* identity duplication */
    str = "module a {namespace urn:a; prefix a; identity a; identity a;}";
    assert_int_equal(LY_EVALID, yang_parse(&ctx, str, &mod));
    logbuf_assert("Duplicate identifier \"a\" of identity statement. Line number 1.");
    assert_null(mod);

#undef TEST_DUP

    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_feature(void **state)
{
    (void) state; /* unused */

    struct ly_parser_ctx ctx;
    struct lysp_module *mod = NULL;
    struct lysp_feature *features = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.line = 1;
    ctx.indent = 0;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_DUP_GENERIC(" test {", MEMBER, VALUE1, VALUE2, parse_feature, \
                     &features, "1", FREE_ARRAY(ctx.ctx, features, lysp_feature_free); features = NULL)

    TEST_DUP("description", "a", "b");
    TEST_DUP("reference", "a", "b");
    TEST_DUP("status", "current", "obsolete");

    /* full content */
    str = " test {description text;reference \'another text\';status current; if-feature x;if-feature y;prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_feature(&ctx, &str, &features));
    assert_non_null(features);
    assert_string_equal(" ...", str);
    FREE_ARRAY(ctx.ctx, features, lysp_feature_free);
    features = NULL;

    /* invalid substatement */
    str = " test {organization XXX;}";
    assert_int_equal(LY_EVALID, parse_feature(&ctx, &str, &features));
    logbuf_assert("Invalid keyword \"organization\" as a child of \"feature\". Line number 1.");
    FREE_ARRAY(ctx.ctx, features, lysp_feature_free);
    features = NULL;

    /* feature duplication */
    str = "module a {namespace urn:a; prefix a; feature a; feature a;}";
    assert_int_equal(LY_EVALID, yang_parse(&ctx, str, &mod));
    logbuf_assert("Duplicate identifier \"a\" of feature statement. Line number 1.");
    assert_null(mod);

#undef TEST_DUP

    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_deviation(void **state)
{
    (void) state; /* unused */

    struct ly_parser_ctx ctx;
    struct lysp_deviation *d = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.line = 1;
    ctx.indent = 0;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_DUP_GENERIC(" test {deviate not-supported;", MEMBER, VALUE1, VALUE2, parse_deviation, \
                     &d, "1", FREE_ARRAY(ctx.ctx, d, lysp_deviation_free); d = NULL)

    TEST_DUP("description", "a", "b");
    TEST_DUP("reference", "a", "b");

    /* full content */
    str = " test {deviate not-supported;description text;reference \'another text\';prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviation(&ctx, &str, &d));
    assert_non_null(d);
    assert_string_equal(" ...", str);
    FREE_ARRAY(ctx.ctx, d, lysp_deviation_free);
    d = NULL;

    /* missing mandatory substatement */
    str = " test {description text;}";
    assert_int_equal(LY_EVALID, parse_deviation(&ctx, &str, &d));
    logbuf_assert("Missing mandatory keyword \"deviate\" as a child of \"deviation\". Line number 1.");
    FREE_ARRAY(ctx.ctx, d, lysp_deviation_free);
    d = NULL;

    /* invalid substatement */
    str = " test {deviate not-supported; status obsolete;}";
    assert_int_equal(LY_EVALID, parse_deviation(&ctx, &str, &d));
    logbuf_assert("Invalid keyword \"status\" as a child of \"deviation\". Line number 1.");
    FREE_ARRAY(ctx.ctx, d, lysp_deviation_free);
    d = NULL;

#undef TEST_DUP

    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_deviate(void **state)
{
    (void) state; /* unused */

    struct ly_parser_ctx ctx;
    struct lysp_deviate *d = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.line = 1;
    ctx.indent = 0;

    /* invalid cardinality */
#define TEST_DUP(TYPE, MEMBER, VALUE1, VALUE2) \
    TEST_DUP_GENERIC(TYPE" {", MEMBER, VALUE1, VALUE2, parse_deviate, \
                     &d, "1", lysp_deviate_free(ctx.ctx, d); free(d); d = NULL)

    TEST_DUP("add", "config", "true", "false");
    TEST_DUP("replace", "default", "int8", "uint8");
    TEST_DUP("add", "mandatory", "true", "false");
    TEST_DUP("add", "max-elements", "1", "2");
    TEST_DUP("add", "min-elements", "1", "2");
    TEST_DUP("replace", "type", "int8", "uint8");
    TEST_DUP("add", "units", "kilometers", "miles");

    /* full contents */
    str = " not-supported {prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &str, &d));
    assert_non_null(d);
    assert_string_equal(" ...", str);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;
    str = " add {units meters; must 1; must 2; unique x; unique y; default a; default b; config true; mandatory true; min-elements 1; max-elements 2; prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &str, &d));
    assert_non_null(d);
    assert_string_equal(" ...", str);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;
    str = " delete {units meters; must 1; must 2; unique x; unique y; default a; default b; prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &str, &d));
    assert_non_null(d);
    assert_string_equal(" ...", str);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;
    str = " replace {type string; units meters; default a; config true; mandatory true; min-elements 1; max-elements 2; prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &str, &d));
    assert_non_null(d);
    assert_string_equal(" ...", str);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;

    /* invalid substatements */
#define TEST_NOT_SUP(DEV, STMT, VALUE) \
    str = " "DEV" {"STMT" "VALUE";}..."; \
    assert_int_equal(LY_EVALID, parse_deviate(&ctx, &str, &d)); \
    logbuf_assert("Deviate \""DEV"\" does not support keyword \""STMT"\". Line number 1."); \
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL

    TEST_NOT_SUP("not-supported", "units", "meters");
    TEST_NOT_SUP("not-supported", "must", "1");
    TEST_NOT_SUP("not-supported", "unique", "x");
    TEST_NOT_SUP("not-supported", "default", "a");
    TEST_NOT_SUP("not-supported", "config", "true");
    TEST_NOT_SUP("not-supported", "mandatory", "true");
    TEST_NOT_SUP("not-supported", "min-elements", "1");
    TEST_NOT_SUP("not-supported", "max-elements", "2");
    TEST_NOT_SUP("not-supported", "type", "string");
    TEST_NOT_SUP("add", "type", "string");
    TEST_NOT_SUP("delete", "config", "true");
    TEST_NOT_SUP("delete", "mandatory", "true");
    TEST_NOT_SUP("delete", "min-elements", "1");
    TEST_NOT_SUP("delete", "max-elements", "2");
    TEST_NOT_SUP("delete", "type", "string");
    TEST_NOT_SUP("replace", "must", "1");
    TEST_NOT_SUP("replace", "unique", "a");

    str = " nonsence; ...";
    assert_int_equal(LY_EVALID, parse_deviate(&ctx, &str, &d));
    logbuf_assert("Invalid value \"nonsence\" of \"deviate\". Line number 1.");
    assert_null(d);

#undef TEST_NOT_SUP
#undef TEST_DUP

    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_container(void **state)
{
    (void) state; /* unused */

    struct lysp_module mod = {0};
    struct ly_parser_ctx ctx = {0};
    struct lysp_node_container *c = NULL;
    const char *str;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.line = 1;
    ctx.mod = &mod;
    ctx.mod->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    str = "cont {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_container(&ctx, &str, NULL, (struct lysp_node**)&c)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)c); c = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("presence", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    str = "cont {action x;anydata any;anyxml anyxml; choice ch;config false;container c;description test;grouping g;if-feature f; leaf l {type string;}"
          "leaf-list ll {type string;} list li;must 'expr';notification not; presence true; reference test;status current;typedef t {type int8;}uses g;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_container(&ctx, &str, NULL, (struct lysp_node**)&c));
    assert_non_null(c);
    assert_non_null(c->actions);
    assert_non_null(c->child);
    assert_string_equal("test", c->dsc);
    assert_non_null(c->exts);
    assert_non_null(c->groupings);
    assert_non_null(c->iffeatures);
    assert_non_null(c->musts);
    assert_non_null(c->notifs);
    assert_string_equal("true", c->presence);
    assert_string_equal("test", c->ref);
    assert_non_null(c->typedefs);
    assert_non_null(c->when);
    assert_null(c->parent);
    assert_null(c->next);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR, c->flags);
    ly_set_erase(&ctx.tpdfs_nodes, NULL);
    lysp_node_free(ctx.ctx, (struct lysp_node*)c); c = NULL;

    /* invalid */
    str = " cont {augment /root;} ...";
    assert_int_equal(LY_EVALID, parse_container(&ctx, &str, NULL, (struct lysp_node**)&c));
    logbuf_assert("Invalid keyword \"augment\" as a child of \"container\". Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)c); c = NULL;
    str = " cont {nonsence true;} ...";
    assert_int_equal(LY_EVALID, parse_container(&ctx, &str, NULL, (struct lysp_node**)&c));
    logbuf_assert("Invalid character sequence \"nonsence\", expected a keyword. Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)c); c = NULL;

    ly_ctx_destroy(ctx.ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_helpers, logger_setup),
        cmocka_unit_test_setup(test_comments, logger_setup),
        cmocka_unit_test_setup(test_arg, logger_setup),
        cmocka_unit_test_setup(test_stmts, logger_setup),
        cmocka_unit_test_setup(test_module, logger_setup),
        cmocka_unit_test_setup(test_identity, logger_setup),
        cmocka_unit_test_setup(test_feature, logger_setup),
        cmocka_unit_test_setup(test_deviation, logger_setup),
        cmocka_unit_test_setup(test_deviate, logger_setup),
        cmocka_unit_test_setup(test_container, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
