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

#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "libyang.h"
#include "../../src/parser_yang.c"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */

    if (path) {
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

    /* geenric extension */
    str = p = "nacm:default-deny-write;";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &str, &kw, &word, &len));
    assert_int_equal(YANG_CUSTOM, kw);
    assert_int_equal(23, len);
    assert_ptr_equal(p, word);

}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_helpers, logger_setup),
        cmocka_unit_test_setup(test_comments, logger_setup),
        cmocka_unit_test_setup(test_arg, logger_setup),
        cmocka_unit_test_setup(test_stmts, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
