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

#include "common.h"
#include "parser_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

/* originally static functions from tree_schema_free.c and parser_yang.c */
void lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext);
void lysp_deviation_free(struct ly_ctx *ctx, struct lysp_deviation *dev);
void lysp_grp_free(struct ly_ctx *ctx, struct lysp_grp *grp);
void lysp_action_free(struct ly_ctx *ctx, struct lysp_action *action);
void lysp_notif_free(struct ly_ctx *ctx, struct lysp_notif *notif);
void lysp_augment_free(struct ly_ctx *ctx, struct lysp_augment *augment);
void lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d);
void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);
void lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when);

LY_ERR buf_add_char(struct ly_ctx *ctx, struct ly_in *in, size_t len, char **buf, size_t *buf_len, size_t *buf_used);
LY_ERR buf_store_char(struct lys_yang_parser_ctx *ctx, struct ly_in *in, enum yang_arg arg, char **word_p,
                      size_t *word_len, char **word_b, size_t *buf_len, uint8_t need_buf, uint8_t *prefix);
LY_ERR get_keyword(struct lys_yang_parser_ctx *ctx, struct ly_in *in, enum ly_stmt *kw, char **word_p, size_t *word_len);
LY_ERR get_argument(struct lys_yang_parser_ctx *ctx, struct ly_in *in, enum yang_arg arg,
                    uint16_t *flags, char **word_p, char **word_b, size_t *word_len);
LY_ERR skip_comment(struct lys_yang_parser_ctx *ctx, struct ly_in *in, uint8_t comment);

LY_ERR parse_action(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_action **actions);
LY_ERR parse_any(struct lys_yang_parser_ctx *ctx, struct ly_in *in, enum ly_stmt kw, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_augment(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_augment **augments);
LY_ERR parse_case(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_container(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_deviate(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_deviate **deviates);
LY_ERR parse_deviation(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_deviation **deviations);
LY_ERR parse_grouping(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_grp **groupings);
LY_ERR parse_choice(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_leaf(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_leaflist(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_list(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_maxelements(struct lys_yang_parser_ctx *ctx, struct ly_in *in, uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts);
LY_ERR parse_minelements(struct lys_yang_parser_ctx *ctx, struct ly_in *in, uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts);
LY_ERR parse_module(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_module *mod);
LY_ERR parse_notif(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_notif **notifs);
LY_ERR parse_submodule(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_submodule *submod);
LY_ERR parse_uses(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_when(struct lys_yang_parser_ctx *ctx, struct ly_in *in, struct lysp_when **when_p);
LY_ERR parse_type_enum_value_pos(struct lys_yang_parser_ctx *ctx, struct ly_in *in, enum ly_stmt val_kw, int64_t *value, uint16_t *flags, struct lysp_ext_instance **exts);

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
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

#define TEST_DUP_GENERIC(PREFIX, MEMBER, VALUE1, VALUE2, FUNC, RESULT, LINE, CLEANUP) \
    in.current = PREFIX MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, FUNC(&ctx, &in, RESULT)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    CLEANUP

static void
test_helpers(void **state)
{
    (void) state; /* unused */

    struct ly_in in = {0};
    char *buf, *p;
    size_t len, size;
    struct lys_yang_parser_ctx ctx;
    ctx.format = LYS_IN_YANG;
    ctx.ctx = NULL;
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    uint8_t prefix = 0;

    /* storing into buffer */
    in.current = "abcd";
    buf = NULL;
    size = len = 0;
    assert_int_equal(LY_SUCCESS, buf_add_char(NULL, &in, 2, &buf, &size, &len));
    assert_int_not_equal(0, size);
    assert_int_equal(2, len);
    assert_string_equal("cd", in.current);
    assert_false(strncmp("ab", buf, 2));
    free(buf);
    buf = NULL;

    /* invalid first characters */
    len = 0;
    in.current = "2invalid";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &in, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    in.current = ".invalid";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &in, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    in.current = "-invalid";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &in, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    /* invalid following characters */
    len = 3; /* number of characters read before the str content */
    in.current = "!";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &in, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    in.current = ":";
    assert_int_equal(LY_EVALID, buf_store_char(&ctx, &in, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    /* valid colon for prefixed identifiers */
    len = size = 0;
    p = NULL;
    prefix = 0;
    in.current = "x:id";
    assert_int_equal(LY_SUCCESS, buf_store_char(&ctx, &in, Y_PREF_IDENTIF_ARG, &p, &len, &buf, &size, 0, &prefix));
    assert_int_equal(1, len);
    assert_null(buf);
    assert_string_equal(":id", in.current);
    assert_int_equal('x', p[len - 1]);
    assert_int_equal(LY_SUCCESS, buf_store_char(&ctx, &in, Y_PREF_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    assert_int_equal(2, len);
    assert_string_equal("id", in.current);
    assert_int_equal(':', p[len - 1]);
    free(buf);
    prefix = 0;

    /* checking identifiers */
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lys_parser_ctx *)&ctx, ':', 0, NULL));
    logbuf_assert("Invalid identifier character ':' (0x003a). Line number 1.");
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lys_parser_ctx *)&ctx, '#', 1, NULL));
    logbuf_assert("Invalid identifier first character '#' (0x0023). Line number 1.");

    assert_int_equal(LY_SUCCESS, lysp_check_identifierchar((struct lys_parser_ctx *)&ctx, 'a', 1, &prefix));
    assert_int_equal(0, prefix);
    assert_int_equal(LY_SUCCESS, lysp_check_identifierchar((struct lys_parser_ctx *)&ctx, ':', 0, &prefix));
    assert_int_equal(1, prefix);
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lys_parser_ctx *)&ctx, ':', 0, &prefix));
    assert_int_equal(1, prefix);
    assert_int_equal(LY_SUCCESS, lysp_check_identifierchar((struct lys_parser_ctx *)&ctx, 'b', 0, &prefix));
    assert_int_equal(2, prefix);
    /* second colon is invalid */
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lys_parser_ctx *)&ctx, ':', 0, &prefix));
    logbuf_assert("Invalid identifier character ':' (0x003a). Line number 1.");
}

static void
test_comments(void **state)
{
    (void) state; /* unused */

    struct ly_in in = {0};
    struct lys_yang_parser_ctx ctx;
    char *word, *buf;
    size_t len;

    ctx.format = LYS_IN_YANG;
    ctx.ctx = NULL;
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;

    in.current = " // this is a text of / one * line */ comment\nargument;";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_string_equal("argument;", word);
    assert_null(buf);
    assert_int_equal(8, len);

    in.current = "/* this is a \n * text // of / block * comment */\"arg\" + \"ume\" \n + \n \"nt\";";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_string_equal("argument", word);
    assert_ptr_equal(buf, word);
    assert_int_equal(8, len);
    free(word);

    in.current = " this is one line comment on last line";
    assert_int_equal(LY_SUCCESS, skip_comment(&ctx, &in, 1));
    assert_true(in.current[0] == '\0');

    in.current = " this is a not terminated comment x";
    assert_int_equal(LY_EVALID, skip_comment(&ctx, &in, 2));
    logbuf_assert("Unexpected end-of-input, non-terminated comment. Line number 5.");
    assert_true(in.current[0] == '\0');
}

static void
test_arg(void **state)
{
    (void) state; /* unused */

    struct lys_yang_parser_ctx ctx;
    struct ly_in in = {0};
    char *word, *buf;
    size_t len;

    ctx.format = LYS_IN_YANG;
    ctx.ctx = NULL;
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;

    /* missing argument */
    in.current = ";";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_MAYBE_STR_ARG, NULL, &word, &buf, &len));
    assert_null(word);

    in.current = "{";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \"{\", expected an argument. Line number 1.");

    /* invalid escape sequence */
    in.current = "\"\\s\"";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Double-quoted string unknown special character \'\\s\'. Line number 1.");
    in.current = "\'\\s\'"; /* valid, since it is not an escape sequence in single quoted string */
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_int_equal(2, len);
    assert_string_equal("\\s\'", word);
    assert_int_equal('\0', in.current[0]); /* input has been eaten */

    /* invalid character after the argument */
    in.current = "hello\"";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \"\"\", expected unquoted string character, optsep, semicolon or opening brace. Line number 1.");
    in.current = "hello}";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \"}\", expected unquoted string character, optsep, semicolon or opening brace. Line number 1.");

    /* invalid identifier-ref-arg-str */
    in.current = "pre:pre:value";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &len));

    in.current = "\"\";"; /* empty identifier is not allowed */
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_IDENTIF_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Statement argument is required. Line number 1.");
    logbuf_clean();
    in.current = "\"\";"; /* empty reference identifier is not allowed */
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Statement argument is required. Line number 1.");

    in.current = "hello/x\t"; /* slash is not an invalid character */
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_int_equal(7, len);
    assert_string_equal("hello/x\t", word);

    assert_null(buf);

    /* different quoting */
    in.current = "hello ";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_null(buf);
    assert_int_equal(5, len);
    assert_string_equal("hello ", word);

    in.current = "hello/*comment*/\n";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_null(buf);
    assert_int_equal(5, len);
    assert_false(strncmp("hello", word, len));


    in.current = "\"hello\\n\\t\\\"\\\\\";";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_non_null(buf);
    assert_int_equal(9, len);
    assert_string_equal("hello\n\t\"\\", word);
    free(buf);

    ctx.indent = 14;
    in.current = "\"hello \t\n\t\t world!\"";
    /* - space and tabs before newline are stripped out
     * - space and tabs after newline (indentation) are stripped out
     */
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_non_null(buf);
    assert_ptr_equal(word, buf);
    assert_int_equal(14, len);
    assert_string_equal("hello\n  world!", word);
    free(buf);
    /* In contrast to previous, the backslash-escaped tabs are expanded after trimming, so they are preserved */
    ctx.indent = 14;
    in.current = "\"hello \\t\n\t\\t world!\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_non_null(buf);
    assert_ptr_equal(word, buf);
    assert_int_equal(16, len);
    assert_string_equal("hello \t\n\t world!", word);
    free(buf);
    /* Do not handle whitespaces after backslash-escaped newline as indentation */
    ctx.indent = 14;
    in.current = "\"hello\\n\t\t world!\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_non_null(buf);
    assert_ptr_equal(word, buf);
    assert_int_equal(15, len);
    assert_string_equal("hello\n\t\t world!", word);
    free(buf);

    ctx.indent = 14;
    in.current = "\"hello\n \tworld!\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_non_null(buf);
    assert_ptr_equal(word, buf);
    assert_int_equal(12, len);
    assert_string_equal("hello\nworld!", word);
    free(buf);

    in.current = "\'hello\'";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_null(buf);
    assert_int_equal(5, len);
    assert_false(strncmp("hello", word, 5));

    in.current = "\"hel\"  +\t\n\"lo\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_ptr_equal(word, buf);
    assert_int_equal(5, len);
    assert_string_equal("hello", word);
    free(buf);
    in.current = "\"hel\"  +\t\nlo"; /* unquoted the second part */
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Both string parts divided by '+' must be quoted. Line number 6.");

    in.current = "\'he\'\t\n+ \"llo\"";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_ptr_equal(word, buf);
    assert_int_equal(5, len);
    assert_string_equal("hello", word);
    free(buf);

    in.current = " \t\n\"he\"+\'llo\'";
    assert_int_equal(LY_SUCCESS, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    assert_ptr_equal(word, buf);
    assert_int_equal(5, len);
    assert_string_equal("hello", word);
    free(buf);

    /* missing argument */
    in.current = ";";
    assert_int_equal(LY_EVALID, get_argument(&ctx, &in, Y_STR_ARG, NULL, &word, &buf, &len));
    logbuf_assert("Invalid character sequence \";\", expected an argument. Line number 8.");
}

static void
test_stmts(void **state)
{
    (void) state; /* unused */

    struct lys_yang_parser_ctx ctx;
    struct ly_in in = {0};
    const char *p;
    enum ly_stmt kw;
    char *word;
    size_t len;

    ctx.format = LYS_IN_YANG;
    ctx.ctx = NULL;
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;

    in.current = "\n// comment\n\tinput\t{";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_INPUT, kw);
    assert_int_equal(5, len);
    assert_string_equal("input\t{", word);
    assert_string_equal("\t{", in.current);

    in.current = "\t /* comment */\t output\n\t{";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_OUTPUT, kw);
    assert_int_equal(6, len);
    assert_string_equal("output\n\t{", word);
    assert_string_equal("\n\t{", in.current);
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_SYNTAX_LEFT_BRACE, kw);
    assert_int_equal(1, len);
    assert_string_equal("{", word);
    assert_string_equal("", in.current);

    in.current = "/input { "; /* invalid slash */
    assert_int_equal(LY_EVALID, get_keyword(&ctx, &in, &kw, &word, &len));
    logbuf_assert("Invalid identifier first character '/'. Line number 4.");

    in.current = "not-a-statement-nor-extension { "; /* invalid identifier */
    assert_int_equal(LY_EVALID, get_keyword(&ctx, &in, &kw, &word, &len));
    logbuf_assert("Invalid character sequence \"not-a-statement-nor-extension\", expected a keyword. Line number 4.");

    in.current = "path;"; /* missing sep after the keyword */
    assert_int_equal(LY_EVALID, get_keyword(&ctx, &in, &kw, &word, &len));
    logbuf_assert("Invalid character sequence \"path;\", expected a keyword followed by a separator. Line number 4.");

    in.current = "action ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ACTION, kw);
    assert_int_equal(6, len);
    in.current = "anydata ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ANYDATA, kw);
    assert_int_equal(7, len);
    in.current = "anyxml ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ANYXML, kw);
    assert_int_equal(6, len);
    in.current = "argument ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ARGUMENT, kw);
    assert_int_equal(8, len);
    in.current = "augment ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_AUGMENT, kw);
    assert_int_equal(7, len);
    in.current = "base ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_BASE, kw);
    assert_int_equal(4, len);
    in.current = "belongs-to ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_BELONGS_TO, kw);
    assert_int_equal(10, len);
    in.current = "bit ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_BIT, kw);
    assert_int_equal(3, len);
    in.current = "case ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_CASE, kw);
    assert_int_equal(4, len);
    in.current = "choice ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_CHOICE, kw);
    assert_int_equal(6, len);
    in.current = "config ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_CONFIG, kw);
    assert_int_equal(6, len);
    in.current = "contact ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_CONTACT, kw);
    assert_int_equal(7, len);
    in.current = "container ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_CONTAINER, kw);
    assert_int_equal(9, len);
    in.current = "default ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_DEFAULT, kw);
    assert_int_equal(7, len);
    in.current = "description ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_DESCRIPTION, kw);
    assert_int_equal(11, len);
    in.current = "deviate ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_DEVIATE, kw);
    assert_int_equal(7, len);
    in.current = "deviation ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_DEVIATION, kw);
    assert_int_equal(9, len);
    in.current = "enum ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ENUM, kw);
    assert_int_equal(4, len);
    in.current = "error-app-tag ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ERROR_APP_TAG, kw);
    assert_int_equal(13, len);
    in.current = "error-message ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ERROR_MESSAGE, kw);
    assert_int_equal(13, len);
    in.current = "extension ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_EXTENSION, kw);
    assert_int_equal(9, len);
    in.current = "feature ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_FEATURE, kw);
    assert_int_equal(7, len);
    in.current = "fraction-digits ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_FRACTION_DIGITS, kw);
    assert_int_equal(15, len);
    in.current = "grouping ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_GROUPING, kw);
    assert_int_equal(8, len);
    in.current = "identity ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_IDENTITY, kw);
    assert_int_equal(8, len);
    in.current = "if-feature ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_IF_FEATURE, kw);
    assert_int_equal(10, len);
    in.current = "import ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_IMPORT, kw);
    assert_int_equal(6, len);
    in.current = "include ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_INCLUDE, kw);
    assert_int_equal(7, len);
    in.current = "input{";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_INPUT, kw);
    assert_int_equal(5, len);
    in.current = "key ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_KEY, kw);
    assert_int_equal(3, len);
    in.current = "leaf ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_LEAF, kw);
    assert_int_equal(4, len);
    in.current = "leaf-list ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_LEAF_LIST, kw);
    assert_int_equal(9, len);
    in.current = "length ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_LENGTH, kw);
    assert_int_equal(6, len);
    in.current = "list ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_LIST, kw);
    assert_int_equal(4, len);
    in.current = "mandatory ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_MANDATORY, kw);
    assert_int_equal(9, len);
    in.current = "max-elements ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_MAX_ELEMENTS, kw);
    assert_int_equal(12, len);
    in.current = "min-elements ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_MIN_ELEMENTS, kw);
    assert_int_equal(12, len);
    in.current = "modifier ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_MODIFIER, kw);
    assert_int_equal(8, len);
    in.current = "module ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_MODULE, kw);
    assert_int_equal(6, len);
    in.current = "must ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_MUST, kw);
    assert_int_equal(4, len);
    in.current = "namespace ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_NAMESPACE, kw);
    assert_int_equal(9, len);
    in.current = "notification ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_NOTIFICATION, kw);
    assert_int_equal(12, len);
    in.current = "ordered-by ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ORDERED_BY, kw);
    assert_int_equal(10, len);
    in.current = "organization ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_ORGANIZATION, kw);
    assert_int_equal(12, len);
    in.current = "output ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_OUTPUT, kw);
    assert_int_equal(6, len);
    in.current = "path ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_PATH, kw);
    assert_int_equal(4, len);
    in.current = "pattern ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_PATTERN, kw);
    assert_int_equal(7, len);
    in.current = "position ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_POSITION, kw);
    assert_int_equal(8, len);
    in.current = "prefix ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_PREFIX, kw);
    assert_int_equal(6, len);
    in.current = "presence ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_PRESENCE, kw);
    assert_int_equal(8, len);
    in.current = "range ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_RANGE, kw);
    assert_int_equal(5, len);
    in.current = "reference ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_REFERENCE, kw);
    assert_int_equal(9, len);
    in.current = "refine ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_REFINE, kw);
    assert_int_equal(6, len);
    in.current = "require-instance ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_REQUIRE_INSTANCE, kw);
    assert_int_equal(16, len);
    in.current = "revision ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_REVISION, kw);
    assert_int_equal(8, len);
    in.current = "revision-date ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_REVISION_DATE, kw);
    assert_int_equal(13, len);
    in.current = "rpc ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_RPC, kw);
    assert_int_equal(3, len);
    in.current = "status ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_STATUS, kw);
    assert_int_equal(6, len);
    in.current = "submodule ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_SUBMODULE, kw);
    assert_int_equal(9, len);
    in.current = "type ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_TYPE, kw);
    assert_int_equal(4, len);
    in.current = "typedef ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_TYPEDEF, kw);
    assert_int_equal(7, len);
    in.current = "unique ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_UNIQUE, kw);
    assert_int_equal(6, len);
    in.current = "units ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_UNITS, kw);
    assert_int_equal(5, len);
    in.current = "uses ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_USES, kw);
    assert_int_equal(4, len);
    in.current = "value ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_VALUE, kw);
    assert_int_equal(5, len);
    in.current = "when ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_WHEN, kw);
    assert_int_equal(4, len);
    in.current = "yang-version ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_YANG_VERSION, kw);
    assert_int_equal(12, len);
    in.current = "yin-element ";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_YIN_ELEMENT, kw);
    assert_int_equal(11, len);
    in.current = ";config false;";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_SYNTAX_SEMICOLON, kw);
    assert_int_equal(1, len);
    assert_string_equal("config false;", in.current);
    in.current = "{ config false;";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_SYNTAX_LEFT_BRACE, kw);
    assert_int_equal(1, len);
    assert_string_equal(" config false;", in.current);
    in.current = "}";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_SYNTAX_RIGHT_BRACE, kw);
    assert_int_equal(1, len);
    assert_string_equal("", in.current);

    /* geenric extension */
    in.current = p = "nacm:default-deny-write;";
    assert_int_equal(LY_SUCCESS, get_keyword(&ctx, &in, &kw, &word, &len));
    assert_int_equal(LY_STMT_EXTENSION_INSTANCE, kw);
    assert_int_equal(23, len);
    assert_ptr_equal(p, word);
}

static void
test_minmax(void **state)
{
    *state = test_minmax;

    struct lys_yang_parser_ctx ctx = {0};
    uint16_t flags = 0;
    uint32_t value = 0;
    struct lysp_ext_instance *ext = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    in.current = " 1invalid; ...";
    assert_int_equal(LY_EVALID, parse_minelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Invalid value \"1invalid\" of \"min-elements\". Line number 1.");

    flags = value = 0;
    in.current = " -1; ...";
    assert_int_equal(LY_EVALID, parse_minelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Invalid value \"-1\" of \"min-elements\". Line number 1.");

    /* implementation limit */
    flags = value = 0;
    in.current = " 4294967296; ...";
    assert_int_equal(LY_EVALID, parse_minelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Value \"4294967296\" is out of \"min-elements\" bounds. Line number 1.");

    flags = value = 0;
    in.current = " 1; ...";
    assert_int_equal(LY_SUCCESS, parse_minelements(&ctx, &in, &value, &flags, &ext));
    assert_int_equal(LYS_SET_MIN, flags);
    assert_int_equal(1, value);

    flags = value = 0;
    in.current = " 1 {m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_minelements(&ctx, &in, &value, &flags, &ext));
    assert_int_equal(LYS_SET_MIN, flags);
    assert_int_equal(1, value);
    assert_non_null(ext);
    FREE_ARRAY(ctx.ctx, ext, lysp_ext_instance_free);
    ext = NULL;

    flags = value = 0;
    in.current = " 1 {config true;} ...";
    assert_int_equal(LY_EVALID, parse_minelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Invalid keyword \"config\" as a child of \"min-elements\". Line number 1.");

    in.current = " 1invalid; ...";
    assert_int_equal(LY_EVALID, parse_maxelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Invalid value \"1invalid\" of \"max-elements\". Line number 1.");

    flags = value = 0;
    in.current = " -1; ...";
    assert_int_equal(LY_EVALID, parse_maxelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Invalid value \"-1\" of \"max-elements\". Line number 1.");

    /* implementation limit */
    flags = value = 0;
    in.current = " 4294967296; ...";
    assert_int_equal(LY_EVALID, parse_maxelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Value \"4294967296\" is out of \"max-elements\" bounds. Line number 1.");

    flags = value = 0;
    in.current = " 1; ...";
    assert_int_equal(LY_SUCCESS, parse_maxelements(&ctx, &in, &value, &flags, &ext));
    assert_int_equal(LYS_SET_MAX, flags);
    assert_int_equal(1, value);

    flags = value = 0;
    in.current = " unbounded; ...";
    assert_int_equal(LY_SUCCESS, parse_maxelements(&ctx, &in, &value, &flags, &ext));
    assert_int_equal(LYS_SET_MAX, flags);
    assert_int_equal(0, value);

    flags = value = 0;
    in.current = " 1 {m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_maxelements(&ctx, &in, &value, &flags, &ext));
    assert_int_equal(LYS_SET_MAX, flags);
    assert_int_equal(1, value);
    assert_non_null(ext);
    FREE_ARRAY(ctx.ctx, ext, lysp_ext_instance_free);
    ext = NULL;

    flags = value = 0;
    in.current = " 1 {config true;} ...";
    assert_int_equal(LY_EVALID, parse_maxelements(&ctx, &in, &value, &flags, &ext));
    logbuf_assert("Invalid keyword \"config\" as a child of \"max-elements\". Line number 1.");

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static struct lysp_module *
mod_renew(struct lys_yang_parser_ctx *ctx)
{
    struct lysp_module *mod_p;
    static struct lys_module mod = {0};

    lysc_module_free(mod.compiled, NULL);
    lysp_module_free(mod.parsed);
    FREE_STRING(mod.ctx, mod.name);
    FREE_STRING(mod.ctx, mod.ns);
    FREE_STRING(mod.ctx, mod.prefix);
    FREE_STRING(mod.ctx, mod.filepath);
    FREE_STRING(mod.ctx, mod.org);
    FREE_STRING(mod.ctx, mod.contact);
    FREE_STRING(mod.ctx, mod.dsc);
    FREE_STRING(mod.ctx, mod.ref);
    memset(&mod, 0, sizeof mod);
    mod.ctx = ctx->ctx;

    mod_p = calloc(1, sizeof *mod_p);
    mod.parsed = mod_p;
    mod_p->mod = &mod;
    assert_non_null(mod_p);
    return mod_p;
}

static struct lysp_submodule *
submod_renew(struct lys_yang_parser_ctx *ctx, struct lysp_submodule *submod)
{
    lysp_submodule_free(ctx->ctx, submod);
    submod = calloc(1, sizeof *submod);
    assert_non_null(submod);
    return submod;
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
    *state = test_module;

    struct lys_yang_parser_ctx ctx;
    struct lysp_module *mod = NULL;
    struct lysp_submodule *submod = NULL;
    struct lys_module *m;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.indent = 0;

    mod = mod_renew(&ctx);

    /* missing mandatory substatements */
    in.current = " name {}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    assert_string_equal("name", mod->mod->name);
    logbuf_assert("Missing mandatory keyword \"namespace\" as a child of \"module\". Line number 1.");
    mod = mod_renew(&ctx);

    in.current = " name {namespace urn:x;}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    assert_string_equal("urn:x", mod->mod->ns);
    logbuf_assert("Missing mandatory keyword \"prefix\" as a child of \"module\". Line number 1.");
    mod = mod_renew(&ctx);

    in.current = " name {namespace urn:x;prefix \"x\";}";
    assert_int_equal(LY_SUCCESS, parse_module(&ctx, &in, mod));
    assert_string_equal("x", mod->mod->prefix);
    mod = mod_renew(&ctx);

#define SCHEMA_BEGINNING " name {yang-version 1.1;namespace urn:x;prefix \"x\";"
#define SCHEMA_BEGINNING2 " name {namespace urn:x;prefix \"x\";"
#define TEST_NODE(NODETYPE, INPUT, NAME) \
        in.current = SCHEMA_BEGINNING INPUT; \
        assert_int_equal(LY_SUCCESS, parse_module(&ctx, &in, mod)); \
        assert_non_null(mod->data); \
        assert_int_equal(NODETYPE, mod->data->nodetype); \
        assert_string_equal(NAME, mod->data->name); \
        mod = mod_renew(&ctx);
#define TEST_GENERIC(INPUT, TARGET, TEST) \
        in.current = SCHEMA_BEGINNING INPUT; \
        assert_int_equal(LY_SUCCESS, parse_module(&ctx, &in, mod)); \
        assert_non_null(TARGET); \
        TEST; \
        mod = mod_renew(&ctx);
#define TEST_DUP(MEMBER, VALUE1, VALUE2, LINE) \
        TEST_DUP_GENERIC(SCHEMA_BEGINNING, MEMBER, VALUE1, VALUE2, \
                         parse_module, mod, LINE, mod = mod_renew(&ctx))

    /* duplicated namespace, prefix */
    TEST_DUP("namespace", "y", "z", "1");
    TEST_DUP("prefix", "y", "z", "1");
    TEST_DUP("contact", "a", "b", "1");
    TEST_DUP("description", "a", "b", "1");
    TEST_DUP("organization", "a", "b", "1");
    TEST_DUP("reference", "a", "b", "1");

    /* not allowed in module (submodule-specific) */
    in.current = SCHEMA_BEGINNING "belongs-to master {prefix m;}}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    logbuf_assert("Invalid keyword \"belongs-to\" as a child of \"module\". Line number 1.");
    mod = mod_renew(&ctx);

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
    TEST_GENERIC("contact \"firstname\" + \n\t\" surname\";}", mod->mod->contact,
                 assert_string_equal("firstname surname", mod->mod->contact));
    /* container */
    TEST_NODE(LYS_CONTAINER, "container test;}", "test");
    /* description 0..1 */
    TEST_GENERIC("description \'some description\';}", mod->mod->dsc,
                 assert_string_equal("some description", mod->mod->dsc));
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
    in.current = SCHEMA_BEGINNING "import zzz {prefix x;}}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    logbuf_assert("Prefix \"x\" already used as module prefix. Line number 2.");
    mod = mod_renew(&ctx);
    in.current = SCHEMA_BEGINNING "import zzz {prefix y;}import zzz {prefix y;}}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    logbuf_assert("Prefix \"y\" already used to import \"zzz\" module. Line number 2.");
    mod = mod_renew(&ctx);
    in.current = "module name10 {yang-version 1.1;namespace urn:x;prefix \"x\";import zzz {prefix y;}import zzz {prefix z;}}";
    assert_int_equal(lys_parse_mem(ctx.ctx, in.current, LYS_IN_YANG, NULL), LY_SUCCESS);
    logbuf_assert("Single revision of the module \"zzz\" imported twice.");

    /* include */
    store = 1;
    ly_ctx_set_module_imp_clb(ctx.ctx, test_imp_clb, "module xxx { namespace urn:xxx; prefix x;}");
    in.current = "module" SCHEMA_BEGINNING "include xxx;}";
    assert_int_equal(lys_parse_mem(ctx.ctx, in.current, LYS_IN_YANG, NULL), LY_EVALID);
    logbuf_assert("Input data contains module in situation when a submodule is expected.");
    store = -1;

    store = 1;
    ly_ctx_set_module_imp_clb(ctx.ctx, test_imp_clb, "submodule xxx {belongs-to wrong-name {prefix w;}}");
    in.current = "module" SCHEMA_BEGINNING "include xxx;}";
    assert_int_equal(lys_parse_mem(ctx.ctx, in.current, LYS_IN_YANG, NULL), LY_EVALID);
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
    TEST_GENERIC("organization \"CESNET a.l.e.\";}", mod->mod->org,
                 assert_string_equal("CESNET a.l.e.", mod->mod->org));
    /* reference 0..1 */
    TEST_GENERIC("reference RFC7950;}", mod->mod->ref,
                 assert_string_equal("RFC7950", mod->mod->ref));
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
    in.current = SCHEMA_BEGINNING2 "\n\tyang-version 10;}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    logbuf_assert("Invalid value \"10\" of \"yang-version\". Line number 3.");
    mod = mod_renew(&ctx);
    in.current = SCHEMA_BEGINNING2 "yang-version 1;yang-version 1.1;}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    logbuf_assert("Duplicate keyword \"yang-version\". Line number 3.");
    mod = mod_renew(&ctx);
    in.current = SCHEMA_BEGINNING2 "yang-version 1;}";
    assert_int_equal(LY_SUCCESS, parse_module(&ctx, &in, mod));
    assert_int_equal(1, mod->mod->version);
    mod = mod_renew(&ctx);
    in.current = SCHEMA_BEGINNING2 "yang-version \"1.1\";}";
    assert_int_equal(LY_SUCCESS, parse_module(&ctx, &in, mod));
    assert_int_equal(2, mod->mod->version);
    mod = mod_renew(&ctx);

    struct lys_yang_parser_ctx *ctx_p = NULL;
    in.current = "module " SCHEMA_BEGINNING "} module q {namespace urn:q;prefixq;}";
    m = mod->mod;
    free(mod);
    m->parsed = NULL;
    assert_int_equal(LY_EVALID, yang_parse_module(&ctx_p, &in, m));
    logbuf_assert("Trailing garbage \"module q {names...\" after module, expected end-of-input. Line number 1.");
    yang_parser_ctx_free(ctx_p);
    mod = mod_renew(&ctx);

    in.current = "prefix " SCHEMA_BEGINNING "}";
    m = mod->mod;
    free(mod);
    m->parsed = NULL;
    assert_int_equal(LY_EVALID, yang_parse_module(&ctx_p, &in, m));
    yang_parser_ctx_free(ctx_p);
    logbuf_assert("Invalid keyword \"prefix\", expected \"module\" or \"submodule\". Line number 1.");
    mod = mod_renew(&ctx);

    in.current = "module " SCHEMA_BEGINNING "leaf enum {type enumeration {enum seven { position 7;}}}}";
    m = mod->mod;
    free(mod);
    m->parsed = NULL;
    assert_int_equal(LY_EVALID, yang_parse_module(&ctx_p, &in, m));
    yang_parser_ctx_free(ctx_p);
    logbuf_assert("Invalid keyword \"position\" as a child of \"enum\". Line number 1.");
    mod = mod_renew(&ctx);

    /* extensions */
    TEST_GENERIC("prefix:test;}", mod->exts,
                 assert_string_equal("prefix:test", mod->exts[0].name);
                 assert_int_equal(LYEXT_SUBSTMT_SELF, mod->exts[0].insubstmt));
    mod = mod_renew(&ctx);

    /* invalid substatement */
    in.current = SCHEMA_BEGINNING "must false;}";
    assert_int_equal(LY_EVALID, parse_module(&ctx, &in, mod));
    logbuf_assert("Invalid keyword \"must\" as a child of \"module\". Line number 3.");
    mod = mod_renew(&ctx);

    /* submodule */
    submod = submod_renew(&ctx, submod);

    /* missing mandatory substatements */
    in.current = " subname {}";
    lydict_remove(ctx.ctx, submod->name);
    assert_int_equal(LY_EVALID, parse_submodule(&ctx, &in, submod));
    assert_string_equal("subname", submod->name);
    logbuf_assert("Missing mandatory keyword \"belongs-to\" as a child of \"submodule\". Line number 3.");
    submod = submod_renew(&ctx, submod);

    in.current = " subname {belongs-to name {prefix x;}}";
    lydict_remove(ctx.ctx, submod->name);
    assert_int_equal(LY_SUCCESS, parse_submodule(&ctx, &in, submod));
    assert_string_equal("name", submod->belongsto);
    submod = submod_renew(&ctx, submod);

#undef SCHEMA_BEGINNING
#define SCHEMA_BEGINNING " subname {belongs-to name {prefix x;}"

    /* duplicated namespace, prefix */
    in.current = " subname {belongs-to name {prefix x;}belongs-to module1;belongs-to module2;} ...";
    assert_int_equal(LY_EVALID, parse_submodule(&ctx, &in, submod)); \
    logbuf_assert("Duplicate keyword \"belongs-to\". Line number 3."); \
    submod = submod_renew(&ctx, submod);

    /* not allowed in submodule (module-specific) */
    in.current = SCHEMA_BEGINNING "namespace \"urn:z\";}";
    assert_int_equal(LY_EVALID, parse_submodule(&ctx, &in, submod));
    logbuf_assert("Invalid keyword \"namespace\" as a child of \"submodule\". Line number 3.");
    submod = submod_renew(&ctx, submod);
    in.current = SCHEMA_BEGINNING "prefix m;}}";
    assert_int_equal(LY_EVALID, parse_submodule(&ctx, &in, submod));
    logbuf_assert("Invalid keyword \"prefix\" as a child of \"submodule\". Line number 3.");
    submod = submod_renew(&ctx, submod);

    in.current = "submodule " SCHEMA_BEGINNING "} module q {namespace urn:q;prefixq;}";
    lysp_submodule_free(ctx.ctx, submod);
    submod = NULL;
    assert_int_equal(LY_EVALID, yang_parse_submodule(&ctx_p, ctx.ctx, (struct lys_parser_ctx *)&ctx, &in, &submod));
    yang_parser_ctx_free(ctx_p);
    logbuf_assert("Trailing garbage \"module q {names...\" after submodule, expected end-of-input. Line number 1.");

    in.current = "prefix " SCHEMA_BEGINNING "}";
    assert_int_equal(LY_EVALID, yang_parse_submodule(&ctx_p, ctx.ctx, (struct lys_parser_ctx *)&ctx, &in, &submod));
    yang_parser_ctx_free(ctx_p);
    logbuf_assert("Invalid keyword \"prefix\", expected \"module\" or \"submodule\". Line number 1.");
    submod = submod_renew(&ctx, submod);

#undef TEST_GENERIC
#undef TEST_NODE
#undef TEST_DUP
#undef SCHEMA_BEGINNING

    lysp_module_free(mod);
    lysp_submodule_free(ctx.ctx, submod);
    ly_ctx_destroy(ctx.ctx, NULL);

    *state = NULL;
}

static void
test_deviation(void **state)
{
    (void) state; /* unused */

    struct lys_yang_parser_ctx ctx;
    struct lysp_deviation *d = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.indent = 0;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    TEST_DUP_GENERIC(" test {deviate not-supported;", MEMBER, VALUE1, VALUE2, parse_deviation, \
                     &d, "1", FREE_ARRAY(ctx.ctx, d, lysp_deviation_free); d = NULL)

    TEST_DUP("description", "a", "b");
    TEST_DUP("reference", "a", "b");

    /* full content */
    in.current = " test {deviate not-supported;description text;reference \'another text\';prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviation(&ctx, &in, &d));
    assert_non_null(d);
    assert_string_equal(" ...", in.current);
    FREE_ARRAY(ctx.ctx, d, lysp_deviation_free);
    d = NULL;

    /* missing mandatory substatement */
    in.current = " test {description text;}";
    assert_int_equal(LY_EVALID, parse_deviation(&ctx, &in, &d));
    logbuf_assert("Missing mandatory keyword \"deviate\" as a child of \"deviation\". Line number 1.");
    FREE_ARRAY(ctx.ctx, d, lysp_deviation_free);
    d = NULL;

    /* invalid substatement */
    in.current = " test {deviate not-supported; status obsolete;}";
    assert_int_equal(LY_EVALID, parse_deviation(&ctx, &in, &d));
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

    struct lys_yang_parser_ctx ctx;
    struct lysp_deviate *d = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
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
    in.current = " not-supported {prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &in, &d));
    assert_non_null(d);
    assert_string_equal(" ...", in.current);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;
    in.current = " add {units meters; must 1; must 2; unique x; unique y; default a; default b; config true; mandatory true; min-elements 1; max-elements 2; prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &in, &d));
    assert_non_null(d);
    assert_string_equal(" ...", in.current);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;
    in.current = " delete {units meters; must 1; must 2; unique x; unique y; default a; default b; prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &in, &d));
    assert_non_null(d);
    assert_string_equal(" ...", in.current);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;
    in.current = " replace {type string; units meters; default a; config true; mandatory true; min-elements 1; max-elements 2; prefix:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_deviate(&ctx, &in, &d));
    assert_non_null(d);
    assert_string_equal(" ...", in.current);
    lysp_deviate_free(ctx.ctx, d); free(d); d = NULL;

    /* invalid substatements */
#define TEST_NOT_SUP(DEV, STMT, VALUE) \
    in.current = " "DEV" {"STMT" "VALUE";}..."; \
    assert_int_equal(LY_EVALID, parse_deviate(&ctx, &in, &d)); \
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

    in.current = " nonsence; ...";
    assert_int_equal(LY_EVALID, parse_deviate(&ctx, &in, &d));
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

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_container *c = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "cont {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_container(&ctx, &in, NULL, (struct lysp_node**)&c)); \
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
    in.current = "cont {action x;anydata any;anyxml anyxml; choice ch;config false;container c;description test;grouping g;if-feature f; leaf l {type string;}"
          "leaf-list ll {type string;} list li;must 'expr';notification not; presence true; reference test;status current;typedef t {type int8;}uses g;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_container(&ctx, &in, NULL, (struct lysp_node**)&c));
    assert_non_null(c);
    assert_int_equal(LYS_CONTAINER, c->nodetype);
    assert_string_equal("cont", c->name);
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
    in.current = " cont {augment /root;} ...";
    assert_int_equal(LY_EVALID, parse_container(&ctx, &in, NULL, (struct lysp_node**)&c));
    logbuf_assert("Invalid keyword \"augment\" as a child of \"container\". Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)c); c = NULL;
    in.current = " cont {nonsence true;} ...";
    assert_int_equal(LY_EVALID, parse_container(&ctx, &in, NULL, (struct lysp_node**)&c));
    logbuf_assert("Invalid character sequence \"nonsence\", expected a keyword. Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)c); c = NULL;

    ctx.mod_version = 1; /* simulate YANG 1.0 */
    in.current = " cont {action x;} ...";
    assert_int_equal(LY_EVALID, parse_container(&ctx, &in, NULL, (struct lysp_node**)&c));
    logbuf_assert("Invalid keyword \"action\" as a child of \"container\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)c); c = NULL;

    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_leaf(void **state)
{
    *state = test_leaf;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_leaf *l = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    //ctx.mod->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_leaf(&ctx, &in, NULL, (struct lysp_node**)&l)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)l); l = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("default", "x", "y");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("type", "int8", "uint8");
    TEST_DUP("units", "text1", "text2");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content - without mandatory which is mutual exclusive with default */
    in.current = "l {config false;default \"xxx\";description test;if-feature f;"
          "must 'expr';reference test;status current;type string; units yyy;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaf(&ctx, &in, NULL, (struct lysp_node**)&l));
    assert_non_null(l);
    assert_int_equal(LYS_LEAF, l->nodetype);
    assert_string_equal("l", l->name);
    assert_string_equal("test", l->dsc);
    assert_string_equal("xxx", l->dflt.str);
    assert_string_equal("yyy", l->units);
    assert_string_equal("string", l->type.name);
    assert_non_null(l->exts);
    assert_non_null(l->iffeatures);
    assert_non_null(l->musts);
    assert_string_equal("test", l->ref);
    assert_non_null(l->when);
    assert_null(l->parent);
    assert_null(l->next);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR, l->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)l); l = NULL;

    /* full content - now with mandatory */
    in.current = "l {mandatory true; type string;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaf(&ctx, &in, NULL, (struct lysp_node**)&l));
    assert_non_null(l);
    assert_int_equal(LYS_LEAF, l->nodetype);
    assert_string_equal("l", l->name);
    assert_string_equal("string", l->type.name);
    assert_int_equal(LYS_MAND_TRUE, l->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)l); l = NULL;

    /* invalid */
    in.current = " l {description \"missing type\";} ...";
    assert_int_equal(LY_EVALID, parse_leaf(&ctx, &in, NULL, (struct lysp_node**)&l));
    logbuf_assert("Missing mandatory keyword \"type\" as a child of \"leaf\". Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)l); l = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_leaflist(void **state)
{
    *state = test_leaf;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_leaflist *ll = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "ll {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_leaflist(&ctx, &in, NULL, (struct lysp_node**)&ll)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)ll); ll = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("max-elements", "10", "20");
    TEST_DUP("min-elements", "10", "20");
    TEST_DUP("ordered-by", "user", "system");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("type", "int8", "uint8");
    TEST_DUP("units", "text1", "text2");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content - without min-elements which is mutual exclusive with default */
    in.current = "ll {config false;default \"xxx\"; default \"yyy\";description test;if-feature f;"
          "max-elements 10;must 'expr';ordered-by user;reference test;"
          "status current;type string; units zzz;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaflist(&ctx, &in, NULL, (struct lysp_node**)&ll));
    assert_non_null(ll);
    assert_int_equal(LYS_LEAFLIST, ll->nodetype);
    assert_string_equal("ll", ll->name);
    assert_string_equal("test", ll->dsc);
    assert_non_null(ll->dflts);
    assert_int_equal(2, LY_ARRAY_COUNT(ll->dflts));
    assert_string_equal("xxx", ll->dflts[0].str);
    assert_string_equal("yyy", ll->dflts[1].str);
    assert_string_equal("zzz", ll->units);
    assert_int_equal(10, ll->max);
    assert_int_equal(0, ll->min);
    assert_string_equal("string", ll->type.name);
    assert_non_null(ll->exts);
    assert_non_null(ll->iffeatures);
    assert_non_null(ll->musts);
    assert_string_equal("test", ll->ref);
    assert_non_null(ll->when);
    assert_null(ll->parent);
    assert_null(ll->next);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_USER | LYS_SET_MAX, ll->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)ll); ll = NULL;

    /* full content - now with min-elements */
    in.current = "ll {min-elements 10; type string;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaflist(&ctx, &in, NULL, (struct lysp_node**)&ll));
    assert_non_null(ll);
    assert_int_equal(LYS_LEAFLIST, ll->nodetype);
    assert_string_equal("ll", ll->name);
    assert_string_equal("string", ll->type.name);
    assert_int_equal(0, ll->max);
    assert_int_equal(10, ll->min);
    assert_int_equal(LYS_SET_MIN, ll->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)ll); ll = NULL;

    /* invalid */
    in.current = " ll {description \"missing type\";} ...";
    assert_int_equal(LY_EVALID, parse_leaflist(&ctx, &in, NULL, (struct lysp_node**)&ll));
    logbuf_assert("Missing mandatory keyword \"type\" as a child of \"leaf-list\". Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)ll); ll = NULL;

    ctx.mod_version = 1; /* simulate YANG 1.0 - default statement is not allowed */
    in.current = " ll {default xx; type string;} ...";
    assert_int_equal(LY_EVALID, parse_leaflist(&ctx, &in, NULL, (struct lysp_node**)&ll));
    logbuf_assert("Invalid keyword \"default\" as a child of \"leaf-list\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)ll); ll = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_list(void **state)
{
    *state = test_list;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_list *l = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_list(&ctx, &in, NULL, (struct lysp_node**)&l)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)l); l = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("key", "one", "two");
    TEST_DUP("max-elements", "10", "20");
    TEST_DUP("min-elements", "10", "20");
    TEST_DUP("ordered-by", "user", "system");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "l {action x;anydata any;anyxml anyxml; choice ch;config false;container c;description test;grouping g;if-feature f; key l; leaf l {type string;}"
          "leaf-list ll {type string;} list li;max-elements 10; min-elements 1;must 'expr';notification not; ordered-by system; reference test;"
          "status current;typedef t {type int8;}unique xxx;unique yyy;uses g;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_list(&ctx, &in, NULL, (struct lysp_node**)&l));
    assert_non_null(l);
    assert_int_equal(LYS_LIST, l->nodetype);
    assert_string_equal("l", l->name);
    assert_string_equal("test", l->dsc);
    assert_string_equal("l", l->key);
    assert_non_null(l->uniques);
    assert_int_equal(2, LY_ARRAY_COUNT(l->uniques));
    assert_string_equal("xxx", l->uniques[0].str);
    assert_string_equal("yyy", l->uniques[1].str);
    assert_int_equal(10, l->max);
    assert_int_equal(1, l->min);
    assert_non_null(l->exts);
    assert_non_null(l->iffeatures);
    assert_non_null(l->musts);
    assert_string_equal("test", l->ref);
    assert_non_null(l->when);
    assert_null(l->parent);
    assert_null(l->next);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM | LYS_SET_MAX | LYS_SET_MIN, l->flags);
    ly_set_erase(&ctx.tpdfs_nodes, NULL);
    lysp_node_free(ctx.ctx, (struct lysp_node*)l); l = NULL;

    /* invalid content */
    ctx.mod_version = 1; /* simulate YANG 1.0 */
    in.current = "l {action x;} ...";
    assert_int_equal(LY_EVALID, parse_list(&ctx, &in, NULL, (struct lysp_node**)&l));
    logbuf_assert("Invalid keyword \"action\" as a child of \"list\" - the statement is allowed only in YANG 1.1 modules. Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)l); l = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_choice(void **state)
{
    *state = test_choice;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_choice *ch = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "ch {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_choice(&ctx, &in, NULL, (struct lysp_node**)&ch)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)ch); ch = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("default", "a", "b");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content - without default due to a collision with mandatory */
    in.current = "ch {anydata any;anyxml anyxml; case c;choice ch;config false;container c;description test;if-feature f;leaf l {type string;}"
          "leaf-list ll {type string;} list li;mandatory true;reference test;status current;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_choice(&ctx, &in, NULL, (struct lysp_node**)&ch));
    assert_non_null(ch);
    assert_int_equal(LYS_CHOICE, ch->nodetype);
    assert_string_equal("ch", ch->name);
    assert_string_equal("test", ch->dsc);
    assert_non_null(ch->exts);
    assert_non_null(ch->iffeatures);
    assert_string_equal("test", ch->ref);
    assert_non_null(ch->when);
    assert_null(ch->parent);
    assert_null(ch->next);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_MAND_TRUE, ch->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)ch); ch = NULL;

    /* full content - the default missing from the previous node */
    in.current = "ch {default c;case c;} ...";
    assert_int_equal(LY_SUCCESS, parse_choice(&ctx, &in, NULL, (struct lysp_node**)&ch));
    assert_non_null(ch);
    assert_int_equal(LYS_CHOICE, ch->nodetype);
    assert_string_equal("ch", ch->name);
    assert_string_equal("c", ch->dflt.str);
    assert_int_equal(0, ch->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)ch); ch = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_case(void **state)
{
    *state = test_case;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_case *cs = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "cs {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_case(&ctx, &in, NULL, (struct lysp_node**)&cs)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)cs); cs = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "cs {anydata any;anyxml anyxml; choice ch;container c;description test;if-feature f;leaf l {type string;}"
          "leaf-list ll {type string;} list li;reference test;status current;uses grp;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_case(&ctx, &in, NULL, (struct lysp_node**)&cs));
    assert_non_null(cs);
    assert_int_equal(LYS_CASE, cs->nodetype);
    assert_string_equal("cs", cs->name);
    assert_string_equal("test", cs->dsc);
    assert_non_null(cs->exts);
    assert_non_null(cs->iffeatures);
    assert_string_equal("test", cs->ref);
    assert_non_null(cs->when);
    assert_null(cs->parent);
    assert_null(cs->next);
    assert_int_equal(LYS_STATUS_CURR, cs->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)cs); cs = NULL;

    /* invalid content */
    in.current = "cs {config true} ...";
    assert_int_equal(LY_EVALID, parse_case(&ctx, &in, NULL, (struct lysp_node**)&cs));
    logbuf_assert("Invalid keyword \"config\" as a child of \"case\". Line number 1.");
    lysp_node_free(ctx.ctx, (struct lysp_node*)cs); cs = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_any(void **state, enum ly_stmt kw)
{
    *state = test_any;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_anydata *any = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    if (kw == LY_STMT_ANYDATA) {
        ctx.mod_version = 2; /* simulate YANG 1.1 */
    } else {
        ctx.mod_version = 1; /* simulate YANG 1.0 */
    }

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_any(&ctx, &in, kw, NULL, (struct lysp_node**)&any)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)any); any = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "any {config true;description test;if-feature f;mandatory true;must 'expr';reference test;status current;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_any(&ctx, &in, kw, NULL, (struct lysp_node**)&any));
    assert_non_null(any);
    assert_int_equal(kw == LY_STMT_ANYDATA ? LYS_ANYDATA : LYS_ANYXML, any->nodetype);
    assert_string_equal("any", any->name);
    assert_string_equal("test", any->dsc);
    assert_non_null(any->exts);
    assert_non_null(any->iffeatures);
    assert_non_null(any->musts);
    assert_string_equal("test", any->ref);
    assert_non_null(any->when);
    assert_null(any->parent);
    assert_null(any->next);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR | LYS_MAND_TRUE, any->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)any); any = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_anydata(void **state)
{
    return test_any(state, LY_STMT_ANYDATA);
}

static void
test_anyxml(void **state)
{
    return test_any(state, LY_STMT_ANYXML);
}

static void
test_grouping(void **state)
{
    *state = test_grouping;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_grp *grp = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_grouping(&ctx, &in, NULL, &grp)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    FREE_ARRAY(ctx.ctx, grp, lysp_grp_free); grp = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    in.current = "grp {action x;anydata any;anyxml anyxml; choice ch;container c;description test;grouping g;leaf l {type string;}"
          "leaf-list ll {type string;} list li;notification not;reference test;status current;typedef t {type int8;}uses g;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_grouping(&ctx, &in, NULL, &grp));
    assert_non_null(grp);
    assert_int_equal(LYS_GROUPING, grp->nodetype);
    assert_string_equal("grp", grp->name);
    assert_string_equal("test", grp->dsc);
    assert_non_null(grp->exts);
    assert_string_equal("test", grp->ref);
    assert_null(grp->parent);
    assert_int_equal( LYS_STATUS_CURR, grp->flags);
    ly_set_erase(&ctx.tpdfs_nodes, NULL);
    FREE_ARRAY(ctx.ctx, grp, lysp_grp_free); grp = NULL;

    /* invalid content */
    in.current = "grp {config true} ...";
    assert_int_equal(LY_EVALID, parse_grouping(&ctx, &in, NULL, &grp));
    logbuf_assert("Invalid keyword \"config\" as a child of \"grouping\". Line number 1.");
    FREE_ARRAY(ctx.ctx, grp, lysp_grp_free); grp = NULL;

    in.current = "grp {must 'expr'} ...";
    assert_int_equal(LY_EVALID, parse_grouping(&ctx, &in, NULL, &grp));
    logbuf_assert("Invalid keyword \"must\" as a child of \"grouping\". Line number 1.");
    FREE_ARRAY(ctx.ctx, grp, lysp_grp_free); grp = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_action(void **state)
{
    *state = test_action;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_action *rpcs = NULL;
    struct lysp_node_container *c = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "func {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_action(&ctx, &in, NULL, &rpcs)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    FREE_ARRAY(ctx.ctx, rpcs, lysp_action_free); rpcs = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("input", "{leaf l1 {type empty;}} description a", "{leaf l2 {type empty;}} description a");
    TEST_DUP("output", "{leaf l1 {type empty;}} description a", "{leaf l2 {type empty;}} description a");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    in.current = "top;";
    assert_int_equal(LY_SUCCESS, parse_container(&ctx, &in, NULL, (struct lysp_node**)&c));
    in.current = "func {description test;grouping grp;if-feature f;reference test;status current;typedef mytype {type int8;} m:ext;"
          "input {anydata a1; anyxml a2; choice ch; container c; grouping grp; leaf l {type int8;} leaf-list ll {type int8;}"
          " list li; must 1; typedef mytypei {type int8;} uses grp; m:ext;}"
          "output {anydata a1; anyxml a2; choice ch; container c; grouping grp; leaf l {type int8;} leaf-list ll {type int8;}"
          " list li; must 1; typedef mytypeo {type int8;} uses grp; m:ext;}} ...";
    assert_int_equal(LY_SUCCESS, parse_action(&ctx, &in, (struct lysp_node*)c, &rpcs));
    assert_non_null(rpcs);
    assert_int_equal(LYS_ACTION, rpcs->nodetype);
    assert_string_equal("func", rpcs->name);
    assert_string_equal("test", rpcs->dsc);
    assert_non_null(rpcs->exts);
    assert_non_null(rpcs->iffeatures);
    assert_string_equal("test", rpcs->ref);
    assert_non_null(rpcs->groupings);
    assert_non_null(rpcs->typedefs);
    assert_int_equal(LYS_STATUS_CURR, rpcs->flags);
    /* input */
    assert_int_equal(rpcs->input.nodetype, LYS_INPUT);
    assert_non_null(rpcs->input.groupings);
    assert_non_null(rpcs->input.exts);
    assert_non_null(rpcs->input.musts);
    assert_non_null(rpcs->input.typedefs);
    assert_non_null(rpcs->input.data);
    /* output */
    assert_int_equal(rpcs->output.nodetype, LYS_OUTPUT);
    assert_non_null(rpcs->output.groupings);
    assert_non_null(rpcs->output.exts);
    assert_non_null(rpcs->output.musts);
    assert_non_null(rpcs->output.typedefs);
    assert_non_null(rpcs->output.data);

    ly_set_erase(&ctx.tpdfs_nodes, NULL);
    FREE_ARRAY(ctx.ctx, rpcs, lysp_action_free); rpcs = NULL;

    /* invalid content */
    in.current = "func {config true} ...";
    assert_int_equal(LY_EVALID, parse_action(&ctx, &in, NULL, &rpcs));
    logbuf_assert("Invalid keyword \"config\" as a child of \"rpc\". Line number 1.");
    FREE_ARRAY(ctx.ctx, rpcs, lysp_action_free); rpcs = NULL;

    *state = NULL;
    lysp_node_free(ctx.ctx, (struct lysp_node*)c);
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_notification(void **state)
{
    *state = test_notification;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_notif *notifs = NULL;
    struct lysp_node_container *c = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "func {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_notif(&ctx, &in, NULL, &notifs)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    FREE_ARRAY(ctx.ctx, notifs, lysp_notif_free); notifs = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    in.current = "top;";
    assert_int_equal(LY_SUCCESS, parse_container(&ctx, &in, NULL, (struct lysp_node**)&c));
    in.current = "ntf {anydata a1; anyxml a2; choice ch; container c; description test; grouping grp; if-feature f; leaf l {type int8;}"
          "leaf-list ll {type int8;} list li; must 1; reference test; status current; typedef mytype {type int8;} uses grp; m:ext;}";
    assert_int_equal(LY_SUCCESS, parse_notif(&ctx, &in, (struct lysp_node*)c, &notifs));
    assert_non_null(notifs);
    assert_int_equal(LYS_NOTIF, notifs->nodetype);
    assert_string_equal("ntf", notifs->name);
    assert_string_equal("test", notifs->dsc);
    assert_non_null(notifs->exts);
    assert_non_null(notifs->iffeatures);
    assert_string_equal("test", notifs->ref);
    assert_non_null(notifs->groupings);
    assert_non_null(notifs->typedefs);
    assert_non_null(notifs->musts);
    assert_non_null(notifs->data);
    assert_int_equal(LYS_STATUS_CURR, notifs->flags);

    ly_set_erase(&ctx.tpdfs_nodes, NULL);
    FREE_ARRAY(ctx.ctx, notifs, lysp_notif_free); notifs = NULL;

    /* invalid content */
    in.current = "ntf {config true} ...";
    assert_int_equal(LY_EVALID, parse_notif(&ctx, &in, NULL, &notifs));
    logbuf_assert("Invalid keyword \"config\" as a child of \"notification\". Line number 1.");
    FREE_ARRAY(ctx.ctx, notifs, lysp_notif_free); notifs = NULL;

    *state = NULL;
    lysp_node_free(ctx.ctx, (struct lysp_node*)c);
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_uses(void **state)
{
    *state = test_uses;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_node_uses *u = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_uses(&ctx, &in, NULL, (struct lysp_node**)&u)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    lysp_node_free(ctx.ctx, (struct lysp_node*)u); u = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "grpref {augment some/node;description test;if-feature f;reference test;refine some/other/node;status current;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_uses(&ctx, &in, NULL, (struct lysp_node**)&u));
    assert_non_null(u);
    assert_int_equal(LYS_USES, u->nodetype);
    assert_string_equal("grpref", u->name);
    assert_string_equal("test", u->dsc);
    assert_non_null(u->exts);
    assert_non_null(u->iffeatures);
    assert_string_equal("test", u->ref);
    assert_non_null(u->augments);
    assert_non_null(u->refines);
    assert_non_null(u->when);
    assert_null(u->parent);
    assert_null(u->next);
    assert_int_equal(LYS_STATUS_CURR, u->flags);
    lysp_node_free(ctx.ctx, (struct lysp_node*)u); u = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_augment(void **state)
{
    *state = test_augment;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_augment *a = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_augment(&ctx, &in, NULL, &a)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    FREE_ARRAY(ctx.ctx, a, lysp_augment_free); a = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "/target/nodeid {action x; anydata any;anyxml anyxml; case cs; choice ch;container c;description test;if-feature f;leaf l {type string;}"
          "leaf-list ll {type string;} list li;notification not;reference test;status current;uses g;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_augment(&ctx, &in, NULL, &a));
    assert_non_null(a);
    assert_int_equal(LYS_AUGMENT, a->nodetype);
    assert_string_equal("/target/nodeid", a->nodeid);
    assert_string_equal("test", a->dsc);
    assert_non_null(a->exts);
    assert_non_null(a->iffeatures);
    assert_string_equal("test", a->ref);
    assert_non_null(a->when);
    assert_null(a->parent);
    assert_int_equal(LYS_STATUS_CURR, a->flags);
    FREE_ARRAY(ctx.ctx, a, lysp_augment_free); a = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_when(void **state)
{
    *state = test_when;

    struct lys_yang_parser_ctx ctx = {0};
    struct lysp_when *w = NULL;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.mod_version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_when(&ctx, &in, &w)); \
    logbuf_assert("Duplicate keyword \""MEMBER"\". Line number 1."); \
    FREE_MEMBER(ctx.ctx, w, lysp_when_free); w = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
#undef TEST_DUP

    /* full content */
    in.current = "expression {description test;reference test;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_when(&ctx, &in, &w));
    assert_non_null(w);
    assert_string_equal("expression", w->cond);
    assert_string_equal("test", w->dsc);
    assert_string_equal("test", w->ref);
    assert_non_null(w->exts);
    FREE_MEMBER(ctx.ctx, w, lysp_when_free); w = NULL;

    /* empty condition */
    in.current = "\"\";";
    assert_int_equal(LY_SUCCESS, parse_when(&ctx, &in, &w));
    logbuf_assert("Empty argument of when statement does not make sense.");
    assert_non_null(w);
    assert_string_equal("", w->cond);
    FREE_MEMBER(ctx.ctx, w, lysp_when_free); w = NULL;

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

static void
test_value(void **state)
{
    *state = test_value;
    struct lys_yang_parser_ctx ctx;
    struct ly_in in = {0};

    ctx.format = LYS_IN_YANG;
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx.ctx));
    assert_non_null(ctx.ctx);
    ctx.pos_type = LY_VLOG_LINE;
    ctx.line = 1;
    ctx.indent = 0;
    int64_t val = 0;
    uint16_t flags = 0;

    in.current = "-0;";
    assert_int_equal(parse_type_enum_value_pos(&ctx, &in, LY_STMT_VALUE, &val, &flags, NULL), LY_SUCCESS);
    assert_int_equal(val, 0);

    in.current = "-0;";
    flags = 0;
    assert_int_equal(parse_type_enum_value_pos(&ctx, &in, LY_STMT_POSITION, &val, &flags, NULL), LY_EVALID);
    logbuf_assert("Invalid value \"-0\" of \"position\". Line number 1.");

    *state = NULL;
    ly_ctx_destroy(ctx.ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_helpers, logger_setup),
        cmocka_unit_test_setup(test_comments, logger_setup),
        cmocka_unit_test_setup(test_arg, logger_setup),
        cmocka_unit_test_setup(test_stmts, logger_setup),
        cmocka_unit_test_setup_teardown(test_minmax, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_module, logger_setup, logger_teardown),
        cmocka_unit_test_setup(test_deviation, logger_setup),
        cmocka_unit_test_setup(test_deviate, logger_setup),
        cmocka_unit_test_setup(test_container, logger_setup),
        cmocka_unit_test_setup_teardown(test_leaf, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_leaflist, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_list, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_choice, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_case, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_anydata, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_anyxml, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_action, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_notification, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_grouping, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_uses, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_augment, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_when, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_value, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
