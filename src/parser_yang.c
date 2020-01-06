/**
 * @file parser_yang.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG parser
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "dict.h"
#include "log.h"
#include "plugins_exts.h"
#include "set.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

/**
 * @brief Insert WORD into the libyang context's dictionary and store as TARGET.
 * @param[in] CTX yang parser context to access libyang context.
 * @param[in] BUF buffer in case the word is not a constant and can be inserted directly (zero-copy)
 * @param[out] TARGET variable where to store the pointer to the inserted value.
 * @param[in] WORD string to store.
 * @param[in] LEN length of the string in WORD to store.
 */
#define INSERT_WORD(CTX, BUF, TARGET, WORD, LEN) \
    if (BUF) {(TARGET) = lydict_insert_zc((CTX)->ctx, WORD);}\
    else {(TARGET) = lydict_insert((CTX)->ctx, LEN ? WORD : "", LEN);}

/**
 * @brief Move the DATA pointer by COUNT items. Also updates the indent value in yang parser context
 * @param[in] CTX yang parser context to update its indent value.
 * @param[in,out] DATA pointer to move
 * @param[in] COUNT number of items for which the DATA pointer is supposed to move on.
 */
#define MOVE_INPUT(CTX, DATA, COUNT) (*(DATA))+=COUNT;(CTX)->indent+=COUNT

/**
 * @brief Loop through all substatements providing, return if there are none.
 *
 * @param[in] CTX yang parser context for logging.
 * @param[in] DATA Raw data to read from.
 * @param[out] KW YANG keyword read.
 * @param[out] WORD Pointer to the keyword itself.
 * @param[out] WORD_LEN Length of the keyword.
 * @param[out] ERR Variable for error storing.
 *
 * @return In case there are no substatements or a fatal error encountered.
 */
#define YANG_READ_SUBSTMT_FOR(CTX, DATA, KW, WORD, WORD_LEN, ERR, CHECKGOTO) \
    LY_CHECK_RET(get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN)); \
    if (KW == LY_STMT_SYNTAX_SEMICOLON) { \
        CHECKGOTO; \
        return LY_SUCCESS; \
    } \
    if (KW != LY_STMT_SYNTAX_LEFT_BRACE) { \
        LOGVAL_PARSER(CTX, LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", expected \";\" or \"{\".", ly_stmt2str(KW)); \
        return LY_EVALID; \
    } \
    for (ERR = get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN); \
            !ERR && (KW != LY_STMT_SYNTAX_RIGHT_BRACE); \
            ERR = get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN))

LY_ERR parse_container(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_uses(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_choice(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_case(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_list(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_grouping(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_grp **groupings);

/**
 * @brief Add another character to dynamic buffer, a low-level function.
 *
 * Enlarge if needed. Updates \p input as well as \p buf_used.
 *
 * @param[in] ctx libyang context for logging.
 * @param[in, out] input Input string to process.
 * @param[in] len Number of bytes to get from the input string and copy into the buffer.
 * @param[in,out] buf Buffer to use, can be moved by realloc().
 * @param[in,out] buf_len Current size of the buffer.
 * @param[in,out] buf_used Currently used characters of the buffer.
 *
 * @return LY_ERR values.
 */
LY_ERR
buf_add_char(struct ly_ctx *ctx, const char **input, size_t len, char **buf, size_t *buf_len, size_t *buf_used)
{
    if (*buf_len <= (*buf_used) + len) {
        *buf_len += 16;
        *buf = ly_realloc(*buf, *buf_len);
        LY_CHECK_ERR_RET(!*buf, LOGMEM(ctx), LY_EMEM);
    }
    if (*buf_used) {
        memcpy(&(*buf)[*buf_used], *input, len);
    } else {
        memcpy(*buf, *input, len);
    }

    (*buf_used) += len;
    (*input) += len;

    return LY_SUCCESS;
}

/**
 * @brief Store a single UTF8 character. It depends whether in a dynamically-allocated buffer or just as a pointer to the data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] input Pointer to the string where to get the character to store. Automatically moved to the next character
 * when function returns.
 * @param[in] arg Type of the input string to select method of checking character validity.
 * @param[in,out] word_p Word pointer. If buffer (\p word_b) was not yet needed, it is just a pointer to the first
 * stored character. If buffer was needed (\p word_b is non-NULL or \p need_buf is set), it is pointing to the buffer.
 * @param[in,out] word_len Current length of the word pointed to by \p word_p.
 * @param[in,out] word_b Word buffer. Is kept NULL as long as it is not requested (word is a substring of the data).
 * @param[in,out] buf_len Current length of \p word_b.
 * @param[in] need_buf Flag if the dynamically allocated buffer is required.
 * @param[in,out] prefix Storage for internally used flag in case of possible prefixed identifiers:
 * 0 - colon not yet found (no prefix)
 * 1 - \p c is the colon character
 * 2 - prefix already processed, now processing the identifier
 *
 * @return LY_ERR values.
 */
LY_ERR
buf_store_char(struct lys_parser_ctx *ctx, const char **input, enum yang_arg arg, char **word_p,
               size_t *word_len, char **word_b, size_t *buf_len, int need_buf, int *prefix)
{
    unsigned int c;
    size_t len;

    /* check  valid combination of input paremeters - if need_buf specified, word_b must be provided */
    assert(!need_buf || (need_buf && word_b));

    /* get UTF8 code point (and number of bytes coding the character) */
    LY_CHECK_ERR_RET(ly_getutf8(input, &c, &len),
                     LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, (*input)[-len]), LY_EVALID);
    (*input) -= len;
    if (c == '\n') {
        ctx->indent = 0;
    } else {
        /* note - even the multibyte character is count as 1 */
        ++ctx->indent;
    }

    /* check character validity */
    switch (arg) {
    case Y_IDENTIF_ARG:
        LY_CHECK_RET(lysp_check_identifierchar(ctx, c, !(*word_len), NULL));
        break;
    case Y_PREF_IDENTIF_ARG:
        LY_CHECK_RET(lysp_check_identifierchar(ctx, c, !(*word_len), prefix));
        break;
    case Y_STR_ARG:
    case Y_MAYBE_STR_ARG:
        LY_CHECK_RET(lysp_check_stringchar(ctx, c));
        break;
    }

    if (word_b && *word_b) {
        /* add another character into buffer */
        if (buf_add_char(ctx->ctx, input, len, word_b, buf_len, word_len)) {
            return LY_EMEM;
        }

        /* in case of realloc */
        *word_p = *word_b;
    } else if (word_b && need_buf) {
        /* first time we need a buffer, copy everything read up to now */
        if (*word_len) {
            *word_b = malloc(*word_len);
            LY_CHECK_ERR_RET(!*word_b, LOGMEM(ctx->ctx), LY_EMEM);
            *buf_len = *word_len;
            memcpy(*word_b, *word_p, *word_len);
        }

        /* add this new character into buffer */
        if (buf_add_char(ctx->ctx, input, len, word_b, buf_len, word_len)) {
            return LY_EMEM;
        }

        /* in case of realloc */
        *word_p = *word_b;
    } else {
        /* just remember the first character pointer */
        if (!*word_p) {
            *word_p = (char *)(*input);
        }
        /* ... and update the word's length */
        (*word_len) += len;
        (*input) += len;
    }

    return LY_SUCCESS;
}

/**
 * @brief Skip YANG comment in data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, automatically moved after the comment.
 * @param[in] comment Type of the comment to process:
 *                    1 for a one-line comment,
 *                    2 for a block comment.
 *
 * @return LY_ERR values.
 */
LY_ERR
skip_comment(struct lys_parser_ctx *ctx, const char **data, int comment)
{
    /* internal statuses: 0 - comment ended,
     *                    1 - in line comment,
     *                    2 - in block comment,
     *                    3 - in block comment with last read character '*'
     */
    while (**data && comment) {
        switch (comment) {
        case 1:
            if (**data == '\n') {
                comment = 0;
                ++ctx->line;
            }
            break;
        case 2:
            if (**data == '*') {
                comment = 3;
            } else if (**data == '\n') {
                ++ctx->line;
            }
            break;
        case 3:
            if (**data == '/') {
                comment = 0;
            } else if (**data != '*') {
                if (**data == '\n') {
                    ++ctx->line;
                }
                comment = 2;
            }
            break;
        default:
            LOGINT_RET(ctx->ctx);
        }

        if (**data == '\n') {
            ctx->indent = 0;
        } else {
            ++ctx->indent;
        }
        ++(*data);
    }

    if (!**data && (comment > 1)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX, "Unexpected end-of-input, non-terminated comment.");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Read a quoted string from data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] arg Type of YANG keyword argument expected.
 * @param[out] word_p Pointer to the read quoted string.
 * @param[out] word_b Pointer to a dynamically-allocated buffer holding the read quoted string. If not needed,
 * set to NULL. Otherwise equal to \p word_p.
 * @param[out] word_len Length of the read quoted string.
 * @param[out] buf_len Length of the dynamically-allocated buffer \p word_b.
 * @param[in] indent Current indent (number of YANG spaces). Needed for correct multi-line string
 * indenation in the final quoted string.
 *
 * @return LY_ERR values.
 */
static LY_ERR
read_qstring(struct lys_parser_ctx *ctx, const char **data, enum yang_arg arg, char **word_p, char **word_b, size_t *word_len,
             size_t *buf_len)
{
    /* string: 0 - string ended, 1 - string with ', 2 - string with ", 3 - string with " with last character \,
     *         4 - string finished, now skipping whitespaces looking for +,
     *         5 - string continues after +, skipping whitespaces */
    unsigned int string, block_indent = 0, current_indent = 0, need_buf = 0;
    const char *c;
    int prefix = 0;
    unsigned int trailing_ws = 0; /* current number of stored trailing whitespace characters */

    if (**data == '\"') {
        string = 2;
        current_indent = block_indent = ctx->indent + 1;
    } else {
        assert(**data == '\'');
        string = 1;
    }
    MOVE_INPUT(ctx, data, 1);

    while (**data && string) {
        switch (string) {
        case 1:
            switch (**data) {
            case '\'':
                /* string may be finished, but check for + */
                string = 4;
                MOVE_INPUT(ctx, data, 1);
                break;
            default:
                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                break;
            }
            break;
        case 2:
            switch (**data) {
            case '\"':
                /* string may be finished, but check for + */
                string = 4;
                MOVE_INPUT(ctx, data, 1);
                trailing_ws = 0;
                break;
            case '\\':
                /* special character following */
                string = 3;

                /* the backslash sequence is substituted, so we will need a buffer to store the result */
                need_buf = 1;

                /* move forward to the escaped character */
                ++(*data);

                /* note that the trailing whitespaces are supposed to be trimmed before substitution of
                 * backslash-escaped characters (RFC 7950, 6.1.3), so we have to zero the trailing whitespaces counter */
                trailing_ws = 0;

                /* since the backslash-escaped character is handled as first non-whitespace character, stop eating indentation */
                current_indent = block_indent;
                break;
            case ' ':
                if (current_indent < block_indent) {
                    ++current_indent;
                    MOVE_INPUT(ctx, data, 1);
                } else {
                    /* check and store whitespace character */
                    LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                    trailing_ws++;
                }
                break;
            case '\t':
                if (current_indent < block_indent) {
                    assert(need_buf);
                    current_indent += 8;
                    ctx->indent += 8;
                    for (; current_indent > block_indent; --current_indent, --ctx->indent) {
                        /* store leftover spaces from the tab */
                        c = " ";
                        LY_CHECK_RET(buf_store_char(ctx, &c, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                        trailing_ws++;
                    }
                    ++(*data);
                } else {
                    /* check and store whitespace character */
                    LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                    trailing_ws++;
                    /* additional characters for indentation - only 1 was count in buf_store_char */
                    ctx->indent += 7;
                }
                break;
            case '\n':
                if (block_indent) {
                    /* we will be removing the indents so we need our own buffer */
                    need_buf = 1;

                    /* remove trailing tabs and spaces */
                    (*word_len) = *word_len - trailing_ws;

                    /* restart indentation */
                    current_indent = 0;
                }

                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));

                /* maintain line number */
                ++ctx->line;

                /* reset context indentation counter for possible string after this one */
                ctx->indent = 0;
                trailing_ws = 0;
                break;
            default:
                /* first non-whitespace character, stop eating indentation */
                current_indent = block_indent;

                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                trailing_ws = 0;
                break;
            }
            break;
        case 3:
            /* string encoded characters */
            switch (**data) {
            case 'n':
                c = "\n";
                break;
            case 't':
                c = "\t";
                break;
            case '\"':
                c = *data;
                break;
            case '\\':
                c = *data;
                break;
            default:
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Double-quoted string unknown special character '\\%c'.", **data);
                return LY_EVALID;
            }

            /* check and store character */
            LY_CHECK_RET(buf_store_char(ctx, &c, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));

            string = 2;
            ++(*data);
            break;
        case 4:
            switch (**data) {
            case '+':
                /* string continues */
                string = 5;
                need_buf = 1;
                break;
            case '\n':
                ++ctx->line;
                /* fallthrough */
            case ' ':
            case '\t':
                /* just skip */
                break;
            default:
                /* string is finished */
                goto string_end;
            }
            MOVE_INPUT(ctx, data, 1);
            break;
        case 5:
            switch (**data) {
            case '\n':
                ++ctx->line;
                /* fallthrough */
            case ' ':
            case '\t':
                /* skip */
                break;
            case '\'':
                string = 1;
                break;
            case '\"':
                string = 2;
                break;
            default:
                /* it must be quoted again */
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Both string parts divided by '+' must be quoted.");
                return LY_EVALID;
            }
            MOVE_INPUT(ctx, data, 1);
            break;
        default:
            return LY_EINT;
        }
    }

string_end:
    if (arg <= Y_PREF_IDENTIF_ARG && !(*word_len)) {
        /* empty identifier */
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Statement argument is required.");
        return LY_EVALID;
    }
    return LY_SUCCESS;
}

/**
 * @brief Get another YANG string from the raw data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] arg Type of YANG keyword argument expected.
 * @param[out] flags optional output argument to get flag of the argument's quoting (LYS_*QOUTED - see [schema node flags](@ref snodeflags))
 * @param[out] word_p Pointer to the read string. Can return NULL if \p arg is #Y_MAYBE_STR_ARG.
 * @param[out] word_b Pointer to a dynamically-allocated buffer holding the read string. If not needed,
 * set to NULL. Otherwise equal to \p word_p.
 * @param[out] word_len Length of the read string.
 *
 * @return LY_ERR values.
 */
LY_ERR
get_argument(struct lys_parser_ctx *ctx, const char **data, enum yang_arg arg,
             uint16_t *flags, char **word_p, char **word_b, size_t *word_len)
{
    size_t buf_len = 0;
    int prefix = 0;
    /* word buffer - dynamically allocated */
    *word_b = NULL;

    /* word pointer - just a pointer to data */
    *word_p = NULL;

    *word_len = 0;
    while (**data) {
        switch (**data) {
        case '\'':
        case '\"':
            if (*word_len) {
                /* invalid - quotes cannot be in unquoted string and only optsep, ; or { can follow it */
                LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, 1, *data,
                            "unquoted string character, optsep, semicolon or opening brace");
                return LY_EVALID;
            }
            if (flags) {
                (*flags) |= (**data) == '\'' ? LYS_SINGLEQUOTED : LYS_DOUBLEQUOTED;
            }
            LY_CHECK_RET(read_qstring(ctx, data, arg, word_p, word_b, word_len, &buf_len));
            goto str_end;
        case '/':
            if ((*data)[1] == '/') {
                /* one-line comment */
                MOVE_INPUT(ctx, data, 2);
                LY_CHECK_RET(skip_comment(ctx, data, 1));
            } else if ((*data)[1] == '*') {
                /* block comment */
                MOVE_INPUT(ctx, data, 2);
                LY_CHECK_RET(skip_comment(ctx, data, 2));
            } else {
                /* not a comment after all */
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, &buf_len, 0, &prefix));
            }
            break;
        case ' ':
            if (*word_len) {
                /* word is finished */
                goto str_end;
            }
            MOVE_INPUT(ctx, data, 1);
            break;
        case '\t':
            if (*word_len) {
                /* word is finished */
                goto str_end;
            }
            /* tabs count for 8 spaces */
            ctx->indent += 8;

            ++(*data);
            break;
        case '\n':
            if (*word_len) {
                /* word is finished */
                goto str_end;
            }
            /* reset indent */
            ctx->indent = 0;

            /* track line numbers */
            ++ctx->line;

            ++(*data);
            break;
        case ';':
        case '{':
            if (*word_len || (arg == Y_MAYBE_STR_ARG)) {
                /* word is finished */
                goto str_end;
            }

            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, 1, *data, "an argument");
            return LY_EVALID;
        case '}':
            /* invalid - braces cannot be in unquoted string (opening braces terminates the string and can follow it) */
            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, 1, *data,
                        "unquoted string character, optsep, semicolon or opening brace");
            return LY_EVALID;
        default:
            LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, &buf_len, 0, &prefix));
            break;
        }
    }

    /* unexpected end of loop */
    LOGVAL_PARSER(ctx, LY_VCODE_EOF);
    return LY_EVALID;

str_end:
    /* terminating NULL byte for buf */
    if (*word_b) {
        (*word_b) = ly_realloc(*word_b, (*word_len) + 1);
        LY_CHECK_ERR_RET(!(*word_b), LOGMEM(ctx->ctx), LY_EMEM);
        (*word_b)[*word_len] = '\0';
        *word_p = *word_b;
    }

    return LY_SUCCESS;
}

/**
 * @brief Get another YANG keyword from the raw data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] kw YANG keyword read.
 * @param[out] word_p Pointer to the keyword in the data. Useful for extension instances.
 * @param[out] word_len Length of the keyword in the data. Useful for extension instances.
 *
 * @return LY_ERR values.
 */
LY_ERR
get_keyword(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt *kw, char **word_p, size_t *word_len)
{
    int prefix;
    const char *word_start;
    unsigned int c;
    size_t len;

    if (word_p) {
        *word_p = NULL;
        *word_len = 0;
    }

    /* first skip "optsep", comments */
    while (**data) {
        switch (**data) {
        case '/':
            if ((*data)[1] == '/') {
                /* one-line comment */
                MOVE_INPUT(ctx, data, 2);
                LY_CHECK_RET(skip_comment(ctx, data, 1));
            } else if ((*data)[1] == '*') {
                /* block comment */
                MOVE_INPUT(ctx, data, 2);
                LY_CHECK_RET(skip_comment(ctx, data, 2));
            } else {
                /* error - not a comment after all, keyword cannot start with slash */
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid identifier first character '/'.");
                return LY_EVALID;
            }
            continue;
        case '\n':
            /* skip whitespaces (optsep) */
            ++ctx->line;
            ctx->indent = 0;
            break;
        case ' ':
            /* skip whitespaces (optsep) */
            ++ctx->indent;
            break;
        case '\t':
            /* skip whitespaces (optsep) */
            ctx->indent += 8;
            break;
        default:
            /* either a keyword start or an invalid character */
            goto keyword_start;
        }

        ++(*data);
    }

keyword_start:
    word_start = *data;
    *kw = lysp_match_kw(ctx, data);

    if (*kw == LY_STMT_SYNTAX_SEMICOLON || *kw == LY_STMT_SYNTAX_LEFT_BRACE || *kw == LY_STMT_SYNTAX_RIGHT_BRACE) {
        goto success;
    }

    if (*kw != LY_STMT_NONE) {
        /* make sure we have the whole keyword */
        switch (**data) {
        case '\n':
        case '\t':
        case ' ':
            /* mandatory "sep" is just checked, not eaten so nothing in the context is updated */
            break;
        case ':':
            /* keyword is not actually a keyword, but prefix of an extension.
             * To avoid repeated check of the prefix syntax, move to the point where the colon was read
             * and we will be checking the keyword (extension instance) itself */
            prefix = 1;
            MOVE_INPUT(ctx, data, 1);
            goto extension;
        case '{':
            /* allowed only for input and output statements which can be without arguments */
            if (*kw == LY_STMT_INPUT || *kw == LY_STMT_OUTPUT) {
                break;
            }
            /* fallthrough */
        default:
            MOVE_INPUT(ctx, data, 1);
            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, (int)(*data - word_start), word_start,
                        "a keyword followed by a separator");
            return LY_EVALID;
        }
    } else {
        /* still can be an extension */
        prefix = 0;
extension:
        while (**data && (**data != ' ') && (**data != '\t') && (**data != '\n') && (**data != '{') && (**data != ';')) {
            LY_CHECK_ERR_RET(ly_getutf8(data, &c, &len),
                             LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, (*data)[-len]), LY_EVALID);
            ++ctx->indent;
            /* check character validity */
            LY_CHECK_RET(lysp_check_identifierchar(ctx, c, *data - len == word_start ? 1 : 0, &prefix));
        }
        if (!**data) {
            LOGVAL_PARSER(ctx, LY_VCODE_EOF);
            return LY_EVALID;
        }

        /* prefix is mandatory for extension instances */
        if (prefix != 2) {
            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, (int)(*data - word_start), word_start, "a keyword");
            return LY_EVALID;
        }

        *kw = LY_STMT_EXTENSION_INSTANCE;
    }
success:
    if (word_p) {
        *word_p = (char *)word_start;
        *word_len = *data - word_start;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse extension instance substatements.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] kw Statement keyword value matching @p word value.
 * @param[in] word Extension instance substatement name (keyword).
 * @param[in] word_len Extension instance substatement name length.
 * @param[in,out] child Children of this extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_ext_substmt(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt kw, char *word, size_t word_len,
                  struct lysp_stmt **child)
{
    char *buf;
    LY_ERR ret = LY_SUCCESS;
    enum ly_stmt child_kw;
    struct lysp_stmt *stmt, *par_child;

    stmt = calloc(1, sizeof *stmt);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(NULL), LY_EMEM);

    /* insert into parent statements */
    if (!*child) {
        *child = stmt;
    } else {
        for (par_child = *child; par_child->next; par_child = par_child->next);
        par_child->next = stmt;
    }

    stmt->stmt = lydict_insert(ctx->ctx, word, word_len);
    stmt->kw = kw;

    /* get optional argument */
    LY_CHECK_RET(get_argument(ctx, data, Y_MAYBE_STR_ARG, &stmt->flags, &word, &buf, &word_len));

    if (word) {
        if (buf) {
            stmt->arg = lydict_insert_zc(ctx->ctx, word);
        } else {
            stmt->arg = lydict_insert(ctx->ctx, word, word_len);
        }
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, child_kw, word, word_len, ret, ) {
        LY_CHECK_RET(parse_ext_substmt(ctx, data, child_kw, word, word_len, &stmt->child));
    }
    return ret;
}

/**
 * @brief Parse extension instance.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] ext_name Extension instance substatement name (keyword).
 * @param[in] ext_name_len Extension instance substatement name length.
 * @param[in] insubstmt Type of the keyword this extension instance is a substatement of.
 * @param[in] insubstmt_index Index of the keyword instance this extension instance is a substatement of.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_ext(struct lys_parser_ctx *ctx, const char **data, const char *ext_name, int ext_name_len, LYEXT_SUBSTMT insubstmt,
          uint32_t insubstmt_index, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    struct lysp_ext_instance *e;
    enum ly_stmt kw;

    LY_ARRAY_NEW_RET(ctx->ctx, *exts, e, LY_EMEM);

    /* store name and insubstmt info */
    e->name = lydict_insert(ctx->ctx, ext_name, ext_name_len);
    e->insubstmt = insubstmt;
    e->insubstmt_index = insubstmt_index;

    /* get optional argument */
    LY_CHECK_RET(get_argument(ctx, data, Y_MAYBE_STR_ARG, NULL, &word, &buf, &word_len));

    if (word) {
        INSERT_WORD(ctx, buf, e->argument, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        LY_CHECK_RET(parse_ext_substmt(ctx, data, kw, word, word_len, &e->child));
    }
    return ret;
}

/**
 * @brief Parse a generic text field without specific constraints. Those are contact, organization,
 * description, etc...
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] substmt Type of this substatement.
 * @param[in] substmt_index Index of this substatement.
 * @param[in,out] value Place to store the parsed value.
 * @param[in] arg Type of the YANG keyword argument (of the value).
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_text_field(struct lys_parser_ctx *ctx, const char **data, LYEXT_SUBSTMT substmt, uint32_t substmt_index,
                 const char **value, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*value) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyext_substmt2str(substmt));
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, arg, NULL, &word, &buf, &word_len));

    /* store value and spend buf if allocated */
    INSERT_WORD(ctx, buf, *value, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, substmt, substmt_index, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), lyext_substmt2str(substmt));
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the yang-version statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] version Storage for the parsed information.
 * @param[in, out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_yangversion(struct lys_parser_ctx *ctx, const char **data, uint8_t *version, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*version) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "yang-version");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 3) && !strncmp(word, "1.0", word_len)) {
        *version = LYS_VERSION_1_0;
    } else if ((word_len == 3) && !strncmp(word, "1.1", word_len)) {
        *version = LYS_VERSION_1_1;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "yang-version");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_VERSION, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "yang-version");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the belongs-to statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] belongsto Place to store the parsed value.
 * @param[in,out] prefix Place to store the parsed belongs-to prefix value.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_belongsto(struct lys_parser_ctx *ctx, const char **data, const char **belongsto, const char **prefix, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*belongsto) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "belongs-to");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));

    INSERT_WORD(ctx, buf, *belongsto, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, prefix, Y_IDENTIF_ARG, exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_BELONGSTO, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "belongs-to");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!*prefix) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "prefix", "belongs-to");
        return LY_EVALID;
    }
    return ret;
}

/**
 * @brief Parse the revision-date statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] rev Array to store the parsed value in.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_revisiondate(struct lys_parser_ctx *ctx, const char **data, char *rev, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (rev[0]) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "revision-date");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    /* check value */
    if (lysp_check_date(ctx, word, word_len, "revision-date")) {
        free(buf);
        return LY_EVALID;
    }

    /* store value and spend buf if allocated */
    strncpy(rev, word, word_len);
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_REVISIONDATE, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "revision-date");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the include statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] module_name Name of the module to check name collisions.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] includes Parsed includes to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_include(struct lys_parser_ctx *ctx, const char *module_name, const char **data, struct lysp_include **includes)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_include *inc;

    LY_ARRAY_NEW_RET(ctx->ctx, *includes, inc, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));

    INSERT_WORD(ctx, buf, inc->name, word, word_len);

    /* submodules share the namespace with the module names, so there must not be
     * a module of the same name in the context, no need for revision matching */
    if (!strcmp(module_name, inc->name) || ly_ctx_get_module_latest(ctx->ctx, inc->name)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Name collision between module and submodule of name \"%s\".", inc->name);
        return LY_EVALID;
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "description", "include");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &inc->dsc, Y_STR_ARG, &inc->exts));
            break;
        case LY_STMT_REFERENCE:
            PARSER_CHECK_STMTVER2_RET(ctx, "reference", "include");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &inc->ref, Y_STR_ARG, &inc->exts));
            break;
        case LY_STMT_REVISION_DATE:
            LY_CHECK_RET(parse_revisiondate(ctx, data, inc->rev, &inc->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &inc->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "include");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the import statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] module_prefix Prefix of the module to check prefix collisions.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] imports Parsed imports to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_import(struct lys_parser_ctx *ctx, const char *module_prefix, const char **data, struct lysp_import **imports)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_import *imp;

    LY_ARRAY_NEW_RET(ctx->ctx, *imports, imp, LY_EVALID);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, imp->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, &imp->prefix, Y_IDENTIF_ARG, &imp->exts));
            LY_CHECK_RET(lysp_check_prefix(ctx, *imports, module_prefix, &imp->prefix), LY_EVALID);
            break;
        case LY_STMT_DESCRIPTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "description", "import");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &imp->dsc, Y_STR_ARG, &imp->exts));
            break;
        case LY_STMT_REFERENCE:
            PARSER_CHECK_STMTVER2_RET(ctx, "reference", "import");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &imp->ref, Y_STR_ARG, &imp->exts));
            break;
        case LY_STMT_REVISION_DATE:
            LY_CHECK_RET(parse_revisiondate(ctx, data, imp->rev, &imp->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &imp->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "import");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    LY_CHECK_ERR_RET(!imp->prefix, LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "prefix", "import"), LY_EVALID);

    return ret;
}

/**
 * @brief Parse the revision statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] revs Parsed revisions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_revision(struct lys_parser_ctx *ctx, const char **data, struct lysp_revision **revs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_revision *rev;

    LY_ARRAY_NEW_RET(ctx->ctx, *revs, rev, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    /* check value */
    if (lysp_check_date(ctx, word, word_len, "revision")) {
        free(buf);
        return LY_EVALID;
    }

    strncpy(rev->date, word, word_len);
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &rev->dsc, Y_STR_ARG, &rev->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &rev->ref, Y_STR_ARG, &rev->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &rev->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "revision");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse a generic text field that can have more instances such as base.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] substmt Type of this substatement.
 * @param[in,out] texts Parsed values to add to.
 * @param[in] arg Type of the expected argument.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_text_fields(struct lys_parser_ctx *ctx, const char **data, LYEXT_SUBSTMT substmt, const char ***texts, enum yang_arg arg,
                  struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    const char **item;
    size_t word_len;
    enum ly_stmt kw;

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(ctx->ctx, *texts, item, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, arg, NULL, &word, &buf, &word_len));

    INSERT_WORD(ctx, buf, *item, word, word_len);
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, substmt, LY_ARRAY_SIZE(*texts) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), lyext_substmt2str(substmt));
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the config statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_config(struct lys_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_CONFIG_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "config");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_CONFIG_W;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_CONFIG_R;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "config");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_CONFIG, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "config");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the mandatory statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_mandatory(struct lys_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_MAND_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "mandatory");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_MAND_TRUE;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "mandatory");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MANDATORY, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "mandatory");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse a restriction such as range or length.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] restr_kw Type of this particular restriction.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_restr(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt restr_kw, struct lysp_restr *restr)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    YANG_CHECK_NONEMPTY(ctx, word_len, ly_stmt2str(restr_kw));
    INSERT_WORD(ctx, buf, restr->arg, word, word_len);
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(restr_kw));
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse a restriction that can have more instances such as must.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] restr_kw Type of this particular restriction.
 * @param[in,out] restrs Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_restrs(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt restr_kw, struct lysp_restr **restrs)
{
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(ctx->ctx, *restrs, restr, LY_EMEM);
    return parse_restr(ctx, data, restr_kw, restr);
}

/**
 * @brief Parse the status statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_status(struct lys_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "status");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 7) && !strncmp(word, "current", word_len)) {
        *flags |= LYS_STATUS_CURR;
    } else if ((word_len == 10) && !strncmp(word, "deprecated", word_len)) {
        *flags |= LYS_STATUS_DEPRC;
    } else if ((word_len == 8) && !strncmp(word, "obsolete", word_len)) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "status");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_STATUS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "status");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the when statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] when_p When pointer to parse to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_when(struct lys_parser_ctx *ctx, const char **data, struct lysp_when **when_p)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_when *when;

    if (*when_p) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "when");
        return LY_EVALID;
    }

    when = calloc(1, sizeof *when);
    LY_CHECK_ERR_RET(!when, LOGMEM(ctx->ctx), LY_EMEM);

    /* get value */
    LY_CHECK_ERR_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len), free(when), LY_EMEM);
    YANG_CHECK_NONEMPTY(ctx, word_len, "when");
    INSERT_WORD(ctx, buf, when->cond, word, word_len);

    *when_p = when;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &when->dsc, Y_STR_ARG, &when->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &when->ref, Y_STR_ARG, &when->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &when->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "when");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the anydata or anyxml statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] kw Type of this particular keyword.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_any(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt kw, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    struct lysp_node_anydata *any;

    /* create new structure and insert into siblings */
    LY_LIST_NEW_RET(ctx->ctx, siblings, any, next, LY_EMEM);

    any->nodetype = kw == LY_STMT_ANYDATA ? LYS_ANYDATA : LYS_ANYXML;
    any->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, any->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &any->flags, &any->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &any->dsc, Y_STR_ARG, &any->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &any->iffeatures, Y_STR_ARG, &any->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &any->flags, &any->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &any->musts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &any->ref, Y_STR_ARG, &any->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &any->flags, &any->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &any->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &any->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw),
                   (any->nodetype & LYS_ANYDATA) == LYS_ANYDATA ? ly_stmt2str(LY_STMT_ANYDATA) : ly_stmt2str(LY_STMT_ANYXML));
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the value or position statement. Substatement of type enum statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] val_kw Type of this particular keyword.
 * @param[in,out] value Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_type_enum_value_pos(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt val_kw, int64_t *value, uint16_t *flags,
                          struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    long int num = 0;
    unsigned long int unum = 0;
    enum ly_stmt kw;

    if (*flags & LYS_SET_VALUE) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(val_kw));
        return LY_EVALID;
    }
    *flags |= LYS_SET_VALUE;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if (!word_len || (word[0] == '+') || ((word[0] == '0') && (word_len > 1)) || ((val_kw == LY_STMT_POSITION) && !strncmp(word, "-0", 2))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
        goto error;
    }

    errno = 0;
    if (val_kw == LY_STMT_VALUE) {
        num = strtol(word, &ptr, 10);
        if (num < INT64_C(-2147483648) || num > INT64_C(2147483647)) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
            goto error;
        }
    } else {
        unum = strtoul(word, &ptr, 10);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
            goto error;
        }
    }
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
        goto error;
    }
    if (errno == ERANGE) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, word_len, word, ly_stmt2str(val_kw));
        goto error;
    }
    if (val_kw == LY_STMT_VALUE) {
        *value = num;
    } else {
        *value = unum;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, val_kw == LY_STMT_VALUE ? LYEXT_SUBSTMT_VALUE : LYEXT_SUBSTMT_POSITION, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(val_kw));
            return LY_EVALID;
        }
    }
    return ret;

error:
    free(buf);
    return LY_EVALID;
}

/**
 * @brief Parse the enum or bit statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] enum_kw Type of this particular keyword.
 * @param[in,out] enums Enums or bits to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_enum(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt enum_kw, struct lysp_type_enum **enums)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_type_enum *enm;

    LY_ARRAY_NEW_RET(ctx->ctx, *enums, enm, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, enum_kw == LY_STMT_ENUM ? Y_STR_ARG : Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    if (enum_kw == LY_STMT_ENUM) {
        ret = lysp_check_enum_name(ctx, (const char *)word, word_len);
        LY_CHECK_ERR_RET(ret, free(buf), ret);
    } /* else nothing specific for YANG_BIT */

    INSERT_WORD(ctx, buf, enm->name, word, word_len);
    CHECK_UNIQUENESS(ctx, *enums, name, ly_stmt2str(enum_kw), enm->name);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &enm->dsc, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", ly_stmt2str(enum_kw));
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &enm->iffeatures, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &enm->ref, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &enm->flags, &enm->exts));
            break;
        case LY_STMT_VALUE:
            LY_CHECK_ERR_RET(enum_kw == LY_STMT_BIT, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw),
                             ly_stmt2str(enum_kw)), LY_EVALID);
            LY_CHECK_RET(parse_type_enum_value_pos(ctx, data, kw, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_POSITION:
            LY_CHECK_ERR_RET(enum_kw == LY_STMT_ENUM, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw),
                             ly_stmt2str(enum_kw)), LY_EVALID);
            LY_CHECK_RET(parse_type_enum_value_pos(ctx, data, kw, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &enm->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(enum_kw));
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the fraction-digits statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] fracdig Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_fracdigits(struct lys_parser_ctx *ctx, const char **data, uint8_t *fracdig, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    unsigned long int num;
    enum ly_stmt kw;

    if (*fracdig) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "fraction-digits");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if (!word_len || (word[0] == '0') || !isdigit(word[0])) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(word, &ptr, 10);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > 18)) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }
    *fracdig = num;
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_FRACDIGITS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "fraction-digits");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the require-instance statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] reqinst Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_reqinstance(struct lys_parser_ctx *ctx, const char **data, uint8_t *reqinst, uint16_t *flags,
                       struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_SET_REQINST) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "require-instance");
        return LY_EVALID;
    }
    *flags |= LYS_SET_REQINST;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *reqinst = 1;
    } else if ((word_len != 5) || strncmp(word, "false", word_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "require-instance");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_REQINSTANCE, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "require-instance");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the modifier statement. Substatement of type pattern statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] pat Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_pattern_modifier(struct lys_parser_ctx *ctx, const char **data, const char **pat, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if ((*pat)[0] == 0x15) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "modifier");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len != 12) || strncmp(word, "invert-match", word_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "modifier");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    /* replace the value in the dictionary */
    buf = malloc(strlen(*pat) + 1);
    LY_CHECK_ERR_RET(!buf, LOGMEM(ctx->ctx), LY_EMEM);
    strcpy(buf, *pat);
    lydict_remove(ctx->ctx, *pat);

    assert(buf[0] == 0x06);
    buf[0] = 0x15;
    *pat = lydict_insert_zc(ctx->ctx, buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MODIFIER, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "modifier");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the pattern statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] patterns Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_pattern(struct lys_parser_ctx *ctx, const char **data, struct lysp_restr **patterns)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(ctx->ctx, *patterns, restr, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    /* add special meaning first byte */
    if (buf) {
        buf = realloc(buf, word_len + 2);
        word = buf;
    } else {
        buf = malloc(word_len + 2);
    }
    LY_CHECK_ERR_RET(!buf, LOGMEM(ctx->ctx), LY_EMEM);
    memmove(buf + 1, word, word_len);
    buf[0] = 0x06; /* pattern's default regular-match flag */
    buf[word_len + 1] = '\0'; /* terminating NULL byte */
    restr->arg = lydict_insert_zc(ctx->ctx, buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_MODIFIER:
            PARSER_CHECK_STMTVER2_RET(ctx, "modifier", "pattern");
            LY_CHECK_RET(parse_type_pattern_modifier(ctx, data, &restr->arg, &restr->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "pattern");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] type Type to wrote to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_type(struct lys_parser_ctx *ctx, const char **data, struct lysp_type *type)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_type *nest_type;

    if (type->name) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "type");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, type->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_BASE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_BASE, &type->bases, Y_PREF_IDENTIF_ARG, &type->exts));
            type->flags |= LYS_SET_BASE;
            break;
        case LY_STMT_BIT:
            LY_CHECK_RET(parse_type_enum(ctx, data, kw, &type->bits));
            type->flags |= LYS_SET_BIT;
            break;
        case LY_STMT_ENUM:
            LY_CHECK_RET(parse_type_enum(ctx, data, kw, &type->enums));
            type->flags |= LYS_SET_ENUM;
            break;
        case LY_STMT_FRACTION_DIGITS:
            LY_CHECK_RET(parse_type_fracdigits(ctx, data, &type->fraction_digits, &type->exts));
            type->flags |= LYS_SET_FRDIGITS;
            break;
        case LY_STMT_LENGTH:
            if (type->length) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                return LY_EVALID;
            }
            type->length = calloc(1, sizeof *type->length);
            LY_CHECK_ERR_RET(!type->length, LOGMEM(ctx->ctx), LY_EMEM);

            LY_CHECK_RET(parse_restr(ctx, data, kw, type->length));
            type->flags |= LYS_SET_LENGTH;
            break;
        case LY_STMT_PATH:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PATH, 0, &type->path, Y_STR_ARG, &type->exts));
            type->flags |= LYS_SET_PATH;
            break;
        case LY_STMT_PATTERN:
            LY_CHECK_RET(parse_type_pattern(ctx, data, &type->patterns));
            type->flags |= LYS_SET_PATTERN;
            break;
        case LY_STMT_RANGE:
            if (type->range) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                return LY_EVALID;
            }
            type->range = calloc(1, sizeof *type->range);
            LY_CHECK_ERR_RET(!type->range, LOGMEM(ctx->ctx), LY_EMEM);

            LY_CHECK_RET(parse_restr(ctx, data, kw, type->range));
            type->flags |= LYS_SET_RANGE;
            break;
        case LY_STMT_REQUIRE_INSTANCE:
            LY_CHECK_RET(parse_type_reqinstance(ctx, data, &type->require_instance, &type->flags, &type->exts));
            /* LYS_SET_REQINST checked and set inside parse_type_reqinstance() */
            break;
        case LY_STMT_TYPE:
            LY_ARRAY_NEW_RET(ctx->ctx, type->types, nest_type, LY_EMEM);
            LY_CHECK_RET(parse_type(ctx, data, nest_type));
            type->flags |= LYS_SET_TYPE;
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &type->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "type");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the leaf statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_leaf(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_leaf *leaf;

    /* create new leaf structure */
    LY_LIST_NEW_RET(ctx->ctx, siblings, leaf, next, LY_EMEM);
    leaf->nodetype = LYS_LEAF;
    leaf->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, leaf->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &leaf->dflt, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &leaf->dsc, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &leaf->iffeatures, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &leaf->musts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &leaf->ref, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(parse_type(ctx, data, &leaf->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &leaf->units, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &leaf->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &leaf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "leaf");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!leaf->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "leaf");
        return LY_EVALID;
    }
    if ((leaf->flags & LYS_MAND_TRUE) && (leaf->dflt)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMSCOMB, "mandatory", "default", "leaf");
        return LY_EVALID;
    }

    return ret;
}

/**
 * @brief Parse the max-elements statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] max Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_maxelements(struct lys_parser_ctx *ctx, const char **data, uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    unsigned long int num;
    enum ly_stmt kw;

    if (*flags & LYS_SET_MAX) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "max-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MAX;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if (!word_len || (word[0] == '0') || ((word[0] != 'u') && !isdigit(word[0]))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "max-elements");
        free(buf);
        return LY_EVALID;
    }

    if (ly_strncmp("unbounded", word, word_len)) {
        errno = 0;
        num = strtoul(word, &ptr, 10);
        /* we have not parsed the whole argument */
        if ((size_t)(ptr - word) != word_len) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "max-elements");
            free(buf);
            return LY_EVALID;
        }
        if ((errno == ERANGE) || (num > UINT32_MAX)) {
            LOGVAL_PARSER(ctx, LY_VCODE_OOB, word_len, word, "max-elements");
            free(buf);
            return LY_EVALID;
        }

        *max = num;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MAX, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "max-elements");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the min-elements statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] min Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_minelements(struct lys_parser_ctx *ctx, const char **data, uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    unsigned long int num;
    enum ly_stmt kw;

    if (*flags & LYS_SET_MIN) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "min-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MIN;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if (!word_len || !isdigit(word[0]) || ((word[0] == '0') && (word_len > 1))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(word, &ptr, 10);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > UINT32_MAX)) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }
    *min = num;
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MIN, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "min-elements");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the ordered-by statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_orderedby(struct lys_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_ORDBY_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "ordered-by");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 6) && !strncmp(word, "system", word_len)) {
        *flags |= LYS_ORDBY_SYSTEM;
    } else if ((word_len == 4) && !strncmp(word, "user", word_len)) {
        *flags |= LYS_ORDBY_USER;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "ordered-by");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_ORDEREDBY, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "ordered-by");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the leaf-list statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_leaflist(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_leaflist *llist;

    /* create new leaf-list structure */
    LY_LIST_NEW_RET(ctx->ctx, siblings, llist, next, LY_EMEM);
    llist->nodetype = LYS_LEAFLIST;
    llist->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, llist->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &llist->flags, &llist->exts));
            break;
        case LY_STMT_DEFAULT:
            PARSER_CHECK_STMTVER2_RET(ctx, "default", "leaf-list");
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, &llist->dflts, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &llist->dsc, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &llist->iffeatures, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, data, &llist->max, &llist->flags, &llist->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, data, &llist->min, &llist->flags, &llist->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &llist->musts));
            break;
        case LY_STMT_ORDERED_BY:
            LY_CHECK_RET(parse_orderedby(ctx, data, &llist->flags, &llist->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &llist->ref, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &llist->flags, &llist->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(parse_type(ctx, data, &llist->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &llist->units, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &llist->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &llist->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "llist");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!llist->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "leaf-list");
        return LY_EVALID;
    }
    if ((llist->min) && (llist->dflts)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMSCOMB, "min-elements", "default", "leaf-list");
        return LY_EVALID;
    }
    if (llist->max && llist->min > llist->max) {
        LOGVAL_PARSER(ctx, LYVE_SEMANTICS,
                    "Invalid combination of min-elements and max-elements: min value %u is bigger than the max value %u.",
                    llist->min, llist->max);
        return LY_EVALID;
    }

    return ret;
}

/**
 * @brief Parse the refine statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] refines Refines to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_refine(struct lys_parser_ctx *ctx, const char **data, struct lysp_refine **refines)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_refine *rf;

    LY_ARRAY_NEW_RET(ctx->ctx, *refines, rf, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));
    YANG_CHECK_NONEMPTY(ctx, word_len, "refine");
    INSERT_WORD(ctx, buf, rf->nodeid, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &rf->flags, &rf->exts));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, &rf->dflts, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &rf->dsc, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", "refine");
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &rf->iffeatures, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, data, &rf->max, &rf->flags, &rf->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, data, &rf->min, &rf->flags, &rf->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &rf->musts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &rf->flags, &rf->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &rf->ref, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_PRESENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PRESENCE, 0, &rf->presence, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &rf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "refine");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the typedef statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] typedefs Typedefs to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_typedef(struct lys_parser_ctx *ctx, struct lysp_node *parent, const char **data, struct lysp_tpdf **typedefs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_tpdf *tpdf;

    LY_ARRAY_NEW_RET(ctx->ctx, *typedefs, tpdf, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, tpdf->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &tpdf->dflt, Y_STR_ARG, &tpdf->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &tpdf->dsc, Y_STR_ARG, &tpdf->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &tpdf->ref, Y_STR_ARG, &tpdf->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &tpdf->flags, &tpdf->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(parse_type(ctx, data, &tpdf->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &tpdf->units, Y_STR_ARG, &tpdf->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &tpdf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "typedef");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!tpdf->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "typedef");
        return LY_EVALID;
    }

    /* store data for collision check */
    if (parent && !(parent->nodetype & (LYS_GROUPING | LYS_ACTION | LYS_INOUT | LYS_NOTIF))) {
        LY_CHECK_RET(ly_set_add(&ctx->tpdfs_nodes, parent, 0) == -1, LY_EMEM);
    }

    return ret;
}

/**
 * @brief Parse the input or output statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] kw Type of this particular keyword
 * @param[in,out] inout_p Input/output pointer to write to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_inout(struct lys_parser_ctx *ctx, const char **data, enum ly_stmt inout_kw, struct lysp_node *parent, struct lysp_action_inout *inout_p)
{
    LY_ERR ret = LY_SUCCESS;
    char *word;
    size_t word_len;
    enum ly_stmt kw;

    if (inout_p->nodetype) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(inout_kw));
        return LY_EVALID;
    }

    /* initiate structure */
    inout_p->nodetype = &((struct lysp_action*)parent)->input == inout_p ? LYS_INPUT : LYS_OUTPUT;
    inout_p->parent = parent;

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", ly_stmt2str(inout_kw));
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)inout_p, data, &inout_p->typedefs));
            break;
        case LY_STMT_MUST:
            PARSER_CHECK_STMTVER2_RET(ctx, "must", ly_stmt2str(inout_kw));
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &inout_p->musts));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)inout_p, &inout_p->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &inout_p->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(inout_kw));
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);

checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, inout_p->groupings, NULL, NULL, NULL));

    if (!inout_p->data) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "data-def-stmt", ly_stmt2str(inout_kw));
        return LY_EVALID;
    }

    return ret;
}

/**
 * @brief Parse the action statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] actions Actions to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_action(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_action **actions)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_action *act;

    LY_ARRAY_NEW_RET(ctx->ctx, *actions, act, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, act->name, word, word_len);
    act->nodetype = LYS_ACTION;
    act->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &act->dsc, Y_STR_ARG, &act->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &act->iffeatures, Y_STR_ARG, &act->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &act->ref, Y_STR_ARG, &act->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &act->flags, &act->exts));
            break;

        case LY_STMT_INPUT:
            LY_CHECK_RET(parse_inout(ctx, data, kw, (struct lysp_node*)act, &act->input));
            break;
        case LY_STMT_OUTPUT:
            LY_CHECK_RET(parse_inout(ctx, data, kw, (struct lysp_node*)act, &act->output));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)act, data, &act->typedefs));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)act, &act->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &act->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), parent ? "action" : "rpc");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, act->groupings, NULL, NULL, NULL));

    return ret;
}

/**
 * @brief Parse the notification statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] notifs Notifications to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_notif(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_notif **notifs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_notif *notif;

    LY_ARRAY_NEW_RET(ctx->ctx, *notifs, notif, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, notif->name, word, word_len);
    notif->nodetype = LYS_NOTIF;
    notif->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &notif->dsc, Y_STR_ARG, &notif->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &notif->iffeatures, Y_STR_ARG, &notif->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &notif->ref, Y_STR_ARG, &notif->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &notif->flags, &notif->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "notification");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)notif, &notif->data));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_case(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;

        case LY_STMT_MUST:
            PARSER_CHECK_STMTVER2_RET(ctx, "must", "notification");
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &notif->musts));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)notif, data, &notif->typedefs));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)notif, &notif->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &notif->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "notification");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, notif->groupings, NULL, NULL, NULL));

    return ret;
}

/**
 * @brief Parse the grouping statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] groupings Groupings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_grouping(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_grp **groupings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_grp *grp;

    LY_ARRAY_NEW_RET(ctx->ctx, *groupings, grp, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, grp->name, word, word_len);
    grp->nodetype = LYS_GROUPING;
    grp->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &grp->dsc, Y_STR_ARG, &grp->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &grp->ref, Y_STR_ARG, &grp->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &grp->flags, &grp->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "grouping");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)grp, &grp->data));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)grp, data, &grp->typedefs));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "grouping");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)grp, &grp->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)grp, &grp->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "grouping");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)grp, &grp->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &grp->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "grouping");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, grp->groupings, NULL, grp->actions, grp->notifs));

    return ret;
}

/**
 * @brief Parse the augment statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] augments Augments to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_augment(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_augment **augments)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_augment *aug;

    LY_ARRAY_NEW_RET(ctx->ctx, *augments, aug, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));
    YANG_CHECK_NONEMPTY(ctx, word_len, "augment");
    INSERT_WORD(ctx, buf, aug->nodeid, word, word_len);
    aug->nodetype = LYS_AUGMENT;
    aug->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &aug->dsc, Y_STR_ARG, &aug->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &aug->iffeatures, Y_STR_ARG, &aug->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &aug->ref, Y_STR_ARG, &aug->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &aug->flags, &aug->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &aug->when));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "augment");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)aug, &aug->child));
            break;
        case LY_STMT_CASE:
            LY_CHECK_RET(parse_case(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;

        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "augment");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)aug, &aug->actions));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "augment");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)aug, &aug->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &aug->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "augment");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, NULL, NULL, aug->actions, aug->notifs));

    return ret;
}

/**
 * @brief Parse the uses statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_uses(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_uses *uses;

    /* create uses structure */
    LY_LIST_NEW_RET(ctx->ctx, siblings, uses, next, LY_EMEM);
    uses->nodetype = LYS_USES;
    uses->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, uses->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &uses->dsc, Y_STR_ARG, &uses->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &uses->iffeatures, Y_STR_ARG, &uses->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &uses->ref, Y_STR_ARG, &uses->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &uses->flags, &uses->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &uses->when));
            break;

        case LY_STMT_REFINE:
            LY_CHECK_RET(parse_refine(ctx, data, &uses->refines));
            break;
        case LY_STMT_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, data, (struct lysp_node*)uses, &uses->augments));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &uses->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "uses");
            return LY_EVALID;
        }
    }
checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, NULL, uses->augments, NULL, NULL));

    return ret;
}

/**
 * @brief Parse the case statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_case(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_case *cas;

    /* create new case structure */
    LY_LIST_NEW_RET(ctx->ctx, siblings, cas, next, LY_EMEM);
    cas->nodetype = LYS_CASE;
    cas->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, cas->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &cas->dsc, Y_STR_ARG, &cas->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &cas->iffeatures, Y_STR_ARG, &cas->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &cas->ref, Y_STR_ARG, &cas->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &cas->flags, &cas->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &cas->when));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "case");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)cas, &cas->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &cas->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "case");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the choice statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_choice(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_choice *choice;

    /* create new choice structure */
    LY_LIST_NEW_RET(ctx->ctx, siblings, choice, next, LY_EMEM);
    choice->nodetype = LYS_CHOICE;
    choice->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, choice->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &choice->flags, &choice->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &choice->dsc, Y_STR_ARG, &choice->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &choice->iffeatures, Y_STR_ARG, &choice->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &choice->flags, &choice->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &choice->ref, Y_STR_ARG, &choice->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &choice->flags, &choice->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &choice->when));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &choice->dflt, Y_PREF_IDENTIF_ARG, &choice->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "choice");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)choice, &choice->child));
            break;
        case LY_STMT_CASE:
            LY_CHECK_RET(parse_case(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case LY_STMT_CHOICE:
            PARSER_CHECK_STMTVER2_RET(ctx, "choice", "choice");
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &choice->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "choice");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    if ((choice->flags & LYS_MAND_TRUE) && choice->dflt) {
        LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMSCOMB, "mandatory", "default", "choice");
        return LY_EVALID;
    }
    return ret;
}

/**
 * @brief Parse the container statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_container(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_container *cont;

    /* create new container structure */
    LY_LIST_NEW_RET(ctx->ctx, siblings, cont, next, LY_EMEM);
    cont->nodetype = LYS_CONTAINER;
    cont->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, cont->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &cont->flags, &cont->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &cont->dsc, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &cont->iffeatures, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &cont->ref, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &cont->flags, &cont->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &cont->when));
            break;
        case LY_STMT_PRESENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PRESENCE, 0, &cont->presence, Y_STR_ARG, &cont->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "container");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)cont, &cont->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)cont, data, &cont->typedefs));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &cont->musts));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "container");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)cont, &cont->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)cont, &cont->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "container");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)cont, &cont->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &cont->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "container");
            return LY_EVALID;
        }
    }
checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, cont->groupings, NULL, cont->actions, cont->notifs));
    return ret;
}

/**
 * @brief Parse the list statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_list(struct lys_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_list *list;

    /* create new list structure */
    LY_LIST_NEW_RET(ctx->ctx, siblings, list, next, LY_EMEM);
    list->nodetype = LYS_LIST;
    list->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, list->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &list->flags, &list->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &list->dsc, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &list->iffeatures, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &list->ref, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &list->flags, &list->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &list->when));
            break;
        case LY_STMT_KEY:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_KEY, 0, &list->key, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, data, &list->max, &list->flags, &list->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, data, &list->min, &list->flags, &list->exts));
            break;
        case LY_STMT_ORDERED_BY:
            LY_CHECK_RET(parse_orderedby(ctx, data, &list->flags, &list->exts));
            break;
        case LY_STMT_UNIQUE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_UNIQUE, &list->uniques, Y_STR_ARG, &list->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "list");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)list, &list->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)list, &list->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)list, data, &list->typedefs));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &list->musts));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "list");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)list, &list->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)list, &list->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "list");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)list, &list->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &list->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "list");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, list->groupings, NULL, list->actions, list->notifs));

    if (list->max && list->min > list->max) {
        LOGVAL_PARSER(ctx, LYVE_SEMANTICS,
                    "Invalid combination of min-elements and max-elements: min value %u is bigger than the max value %u.",
                    list->min, list->max);
        return LY_EVALID;
    }

    return ret;
}

/**
 * @brief Parse the yin-element statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_yinelement(struct lys_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_YINELEM_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "yin-element");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_YINELEM_TRUE;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "yin-element");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_YINELEM, 0, exts));
            LY_CHECK_RET(ret);            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "yin-element");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the argument statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] argument Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_argument(struct lys_parser_ctx *ctx, const char **data, const char **argument, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*argument) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "argument");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, *argument, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_YIN_ELEMENT:
            LY_CHECK_RET(parse_yinelement(ctx, data, flags, exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_ARGUMENT, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "argument");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the extension statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] extensions Extensions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_extension(struct lys_parser_ctx *ctx, const char **data, struct lysp_ext **extensions)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_ext *ex;

    LY_ARRAY_NEW_RET(ctx->ctx, *extensions, ex, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, ex->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &ex->dsc, Y_STR_ARG, &ex->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &ex->ref, Y_STR_ARG, &ex->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &ex->flags, &ex->exts));
            break;
        case LY_STMT_ARGUMENT:
            LY_CHECK_RET(parse_argument(ctx, data, &ex->argument, &ex->flags, &ex->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &ex->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "extension");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the deviate statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] deviates Deviates to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_deviate(struct lys_parser_ctx *ctx, const char **data, struct lysp_deviate **deviates)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len, dev_mod;
    enum ly_stmt kw;
    struct lysp_deviate *d;
    struct lysp_deviate_add *d_add = NULL;
    struct lysp_deviate_rpl *d_rpl = NULL;
    struct lysp_deviate_del *d_del = NULL;
    const char **d_units = NULL, ***d_uniques = NULL, ***d_dflts = NULL;
    struct lysp_restr **d_musts = NULL;
    uint16_t *d_flags = 0;
    uint32_t *d_min = 0, *d_max = 0;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 13) && !strncmp(word, "not-supported", word_len)) {
        dev_mod = LYS_DEV_NOT_SUPPORTED;
    } else if ((word_len == 3) && !strncmp(word, "add", word_len)) {
        dev_mod = LYS_DEV_ADD;
    } else if ((word_len == 7) && !strncmp(word, "replace", word_len)) {
        dev_mod = LYS_DEV_REPLACE;
    } else if ((word_len == 6) && !strncmp(word, "delete", word_len)) {
        dev_mod = LYS_DEV_DELETE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, word_len, word, "deviate");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    /* create structure */
    switch (dev_mod) {
    case LYS_DEV_NOT_SUPPORTED:
        d = calloc(1, sizeof *d);
        LY_CHECK_ERR_RET(!d, LOGMEM(ctx->ctx), LY_EMEM);
        break;
    case LYS_DEV_ADD:
        d_add = calloc(1, sizeof *d_add);
        LY_CHECK_ERR_RET(!d_add, LOGMEM(ctx->ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_add;
        d_units = &d_add->units;
        d_uniques = &d_add->uniques;
        d_dflts = &d_add->dflts;
        d_musts = &d_add->musts;
        d_flags = &d_add->flags;
        d_min = &d_add->min;
        d_max = &d_add->max;
        break;
    case LYS_DEV_REPLACE:
        d_rpl = calloc(1, sizeof *d_rpl);
        LY_CHECK_ERR_RET(!d_rpl, LOGMEM(ctx->ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_rpl;
        d_units = &d_rpl->units;
        d_flags = &d_rpl->flags;
        d_min = &d_rpl->min;
        d_max = &d_rpl->max;
        break;
    case LYS_DEV_DELETE:
        d_del = calloc(1, sizeof *d_del);
        LY_CHECK_ERR_RET(!d_del, LOGMEM(ctx->ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_del;
        d_units = &d_del->units;
        d_uniques = &d_del->uniques;
        d_dflts = &d_del->dflts;
        d_musts = &d_del->musts;
        break;
    default:
        assert(0);
        LOGINT_RET(ctx->ctx);
    }
    d->mod = dev_mod;

    /* insert into siblings */
    LY_LIST_INSERT(deviates, d, next);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_CONFIG:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_config(ctx, data, d_flags, &d->exts));
                break;
            }
            break;
        case LY_STMT_DEFAULT:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            case LYS_DEV_REPLACE:
                LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &d_rpl->dflt, Y_STR_ARG, &d->exts));
                break;
            default:
                LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, d_dflts, Y_STR_ARG, &d->exts));
                break;
            }
            break;
        case LY_STMT_MANDATORY:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_mandatory(ctx, data, d_flags, &d->exts));
                break;
            }
            break;
        case LY_STMT_MAX_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_maxelements(ctx, data, d_max, d_flags, &d->exts));
                break;
            }
            break;
        case LY_STMT_MIN_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_minelements(ctx, data, d_min, d_flags, &d->exts));
                break;
            }
            break;
        case LY_STMT_MUST:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_REPLACE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_restrs(ctx, data, kw, d_musts));
                break;
            }
            break;
        case LY_STMT_TYPE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_ADD:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                if (d_rpl->type) {
                    LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                    return LY_EVALID;
                }
                d_rpl->type = calloc(1, sizeof *d_rpl->type);
                LY_CHECK_ERR_RET(!d_rpl->type, LOGMEM(ctx->ctx), LY_EMEM);
                LY_CHECK_RET(parse_type(ctx, data, d_rpl->type));
                break;
            }
            break;
        case LY_STMT_UNIQUE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_REPLACE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_UNIQUE, d_uniques, Y_STR_ARG, &d->exts));
                break;
            }
            break;
        case LY_STMT_UNITS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, d_units, Y_STR_ARG, &d->exts));
                break;
            }
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &d->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "deviate");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the deviation statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] deviations Deviations to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_deviation(struct lys_parser_ctx *ctx, const char **data, struct lysp_deviation **deviations)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_deviation *dev;

    LY_ARRAY_NEW_RET(ctx->ctx, *deviations, dev, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, NULL, &word, &buf, &word_len));
    YANG_CHECK_NONEMPTY(ctx, word_len, "deviation");
    INSERT_WORD(ctx, buf, dev->nodeid, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &dev->dsc, Y_STR_ARG, &dev->exts));
            break;
        case LY_STMT_DEVIATE:
            LY_CHECK_RET(parse_deviate(ctx, data, &dev->deviates));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &dev->ref, Y_STR_ARG, &dev->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &dev->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "deviation");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!dev->deviates) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "deviate", "deviation");
        return LY_EVALID;
    }

    return ret;
}

/**
 * @brief Parse the feature statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] features Features to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_feature(struct lys_parser_ctx *ctx, const char **data, struct lysp_feature **features)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_feature *feat;

    LY_ARRAY_NEW_RET(ctx->ctx, *features, feat, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, feat->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &feat->dsc, Y_STR_ARG, &feat->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &feat->iffeatures, Y_STR_ARG, &feat->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &feat->ref, Y_STR_ARG, &feat->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &feat->flags, &feat->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &feat->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "feature");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the identity statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] identities Identities to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_identity(struct lys_parser_ctx *ctx, const char **data, struct lysp_ident **identities)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_ident *ident;

    LY_ARRAY_NEW_RET(ctx->ctx, *identities, ident, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, ident->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &ident->dsc, Y_STR_ARG, &ident->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", "identity");
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &ident->iffeatures, Y_STR_ARG, &ident->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &ident->ref, Y_STR_ARG, &ident->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &ident->flags, &ident->exts));
            break;
        case LY_STMT_BASE:
            if (ident->bases && ctx->mod_version < 2) {
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Identity can be derived from multiple base identities only in YANG 1.1 modules");
                return LY_EVALID;
            }
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_BASE, &ident->bases, Y_PREF_IDENTIF_ARG, &ident->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &ident->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "identity");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse module substatements.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] mod Module to write to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_module(struct lys_parser_ctx *ctx, const char **data, struct lysp_module *mod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw, prev_kw = 0;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;
    struct lysp_submodule *dup;

    /* (sub)module name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, mod->mod->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {

#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {LOGVAL_PARSER(ctx, LY_VCODE_INORD, ly_stmt2str(kw), ly_stmt2str(prev_kw)); return LY_EVALID;}mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case LY_STMT_NAMESPACE:
        case LY_STMT_PREFIX:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case LY_STMT_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case LY_STMT_INCLUDE:
        case LY_STMT_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case LY_STMT_ORGANIZATION:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case LY_STMT_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_AUGMENT:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_GROUPING:
        case LY_STMT_IDENTITY:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_NOTIFICATION:
        case LY_STMT_RPC:
        case LY_STMT_TYPEDEF:
        case LY_STMT_USES:
        case LY_STMT_EXTENSION_INSTANCE:
            mod_stmt = Y_MOD_BODY;
            break;
        default:
            /* error handled in the next switch */
            break;
        }
#undef CHECK_ORDER

        prev_kw = kw;
        switch (kw) {
        /* module header */
        case LY_STMT_YANG_VERSION:
            LY_CHECK_RET(parse_yangversion(ctx, data, &mod->mod->version, &mod->exts));
            ctx->mod_version = mod->mod->version;
            break;
        case LY_STMT_NAMESPACE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_NAMESPACE, 0, &mod->mod->ns, Y_STR_ARG, &mod->exts));
            break;
        case LY_STMT_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, &mod->mod->prefix, Y_IDENTIF_ARG, &mod->exts));
            break;

        /* linkage */
        case LY_STMT_INCLUDE:
            LY_CHECK_RET(parse_include(ctx, mod->mod->name, data, &mod->includes));
            break;
        case LY_STMT_IMPORT:
            LY_CHECK_RET(parse_import(ctx, mod->mod->prefix, data, &mod->imports));
            break;

        /* meta */
        case LY_STMT_ORGANIZATION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ORGANIZATION, 0, &mod->mod->org, Y_STR_ARG, &mod->exts));
            break;
        case LY_STMT_CONTACT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_CONTACT, 0, &mod->mod->contact, Y_STR_ARG, &mod->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &mod->mod->dsc, Y_STR_ARG, &mod->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &mod->mod->ref, Y_STR_ARG, &mod->exts));
            break;

        /* revision */
        case LY_STMT_REVISION:
            LY_CHECK_RET(parse_revision(ctx, data, &mod->revs));
            break;

        /* body */
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "module");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, NULL, &mod->data));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, NULL, &mod->data));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, NULL, &mod->data));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, NULL, &mod->data));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, NULL, &mod->data));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, NULL, &mod->data));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, NULL, &mod->data));
            break;

        case LY_STMT_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, data, NULL, &mod->augments));
            break;
        case LY_STMT_DEVIATION:
            LY_CHECK_RET(parse_deviation(ctx, data, &mod->deviations));
            break;
        case LY_STMT_EXTENSION:
            LY_CHECK_RET(parse_extension(ctx, data, &mod->extensions));
            break;
        case LY_STMT_FEATURE:
            LY_CHECK_RET(parse_feature(ctx, data, &mod->features));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, NULL, &mod->groupings));
            break;
        case LY_STMT_IDENTITY:
            LY_CHECK_RET(parse_identity(ctx, data, &mod->identities));
            break;
        case LY_STMT_NOTIFICATION:
            LY_CHECK_RET(parse_notif(ctx, data, NULL, &mod->notifs));
            break;
        case LY_STMT_RPC:
            LY_CHECK_RET(parse_action(ctx, data, NULL, &mod->rpcs));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, NULL, data, &mod->typedefs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &mod->exts));
            break;

        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "module");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);

checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, mod->groupings, mod->augments, mod->rpcs, mod->notifs));

    /* mandatory substatements */
    if (!mod->mod->ns) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "namespace", "module");
        return LY_EVALID;
    } else if (!mod->mod->prefix) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "prefix", "module");
        return LY_EVALID;
    }

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule(ctx->ctx, NULL, mod->mod->name, NULL);
    if (dup) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Name collision between module and submodule of name \"%s\".", mod->mod->name);
        return LY_EVALID;
    }

    return ret;
}

/**
 * @brief Parse submodule substatements.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] submod Parsed submodule structure.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_submodule(struct lys_parser_ctx *ctx, const char **data, struct lysp_submodule *submod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw, prev_kw = 0;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;
    struct lysp_submodule *dup;

    /* submodule name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, submod->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {

#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {LOGVAL_PARSER(ctx, LY_VCODE_INORD, ly_stmt2str(kw), ly_stmt2str(prev_kw)); return LY_EVALID;}mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case LY_STMT_BELONGS_TO:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case LY_STMT_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case LY_STMT_INCLUDE:
        case LY_STMT_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case LY_STMT_ORGANIZATION:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case LY_STMT_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_AUGMENT:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_GROUPING:
        case LY_STMT_IDENTITY:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_NOTIFICATION:
        case LY_STMT_RPC:
        case LY_STMT_TYPEDEF:
        case LY_STMT_USES:
        case LY_STMT_EXTENSION_INSTANCE:
            mod_stmt = Y_MOD_BODY;
            break;
        default:
            /* error handled in the next switch */
            break;
        }
#undef CHECK_ORDER

        prev_kw = kw;
        switch (kw) {
        /* module header */
        case LY_STMT_YANG_VERSION:
            LY_CHECK_RET(parse_yangversion(ctx, data, &submod->version, &submod->exts));
            ctx->mod_version = submod->version;
            break;
        case LY_STMT_BELONGS_TO:
            LY_CHECK_RET(parse_belongsto(ctx, data, &submod->belongsto, &submod->prefix, &submod->exts));
            break;

        /* linkage */
        case LY_STMT_INCLUDE:
            LY_CHECK_RET(parse_include(ctx, submod->name, data, &submod->includes));
            break;
        case LY_STMT_IMPORT:
            LY_CHECK_RET(parse_import(ctx, submod->prefix, data, &submod->imports));
            break;

        /* meta */
        case LY_STMT_ORGANIZATION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ORGANIZATION, 0, &submod->org, Y_STR_ARG, &submod->exts));
            break;
        case LY_STMT_CONTACT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_CONTACT, 0, &submod->contact, Y_STR_ARG, &submod->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &submod->dsc, Y_STR_ARG, &submod->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &submod->ref, Y_STR_ARG, &submod->exts));
            break;

        /* revision */
        case LY_STMT_REVISION:
            LY_CHECK_RET(parse_revision(ctx, data, &submod->revs));
            break;

        /* body */
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "submodule");
            /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, NULL, &submod->data));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, NULL, &submod->data));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, NULL, &submod->data));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, NULL, &submod->data));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, NULL, &submod->data));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, data, NULL, &submod->data));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, data, NULL, &submod->data));
            break;

        case LY_STMT_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, data, NULL, &submod->augments));
            break;
        case LY_STMT_DEVIATION:
            LY_CHECK_RET(parse_deviation(ctx, data, &submod->deviations));
            break;
        case LY_STMT_EXTENSION:
            LY_CHECK_RET(parse_extension(ctx, data, &submod->extensions));
            break;
        case LY_STMT_FEATURE:
            LY_CHECK_RET(parse_feature(ctx, data, &submod->features));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, NULL, &submod->groupings));
            break;
        case LY_STMT_IDENTITY:
            LY_CHECK_RET(parse_identity(ctx, data, &submod->identities));
            break;
        case LY_STMT_NOTIFICATION:
            LY_CHECK_RET(parse_notif(ctx, data, NULL, &submod->notifs));
            break;
        case LY_STMT_RPC:
            LY_CHECK_RET(parse_action(ctx, data, NULL, &submod->rpcs));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, NULL, data, &submod->typedefs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &submod->exts));
            break;

        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "submodule");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);

checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated(ctx, submod->groupings, submod->augments, submod->rpcs, submod->notifs));

    /* mandatory substatements */
    if (!submod->belongsto) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "belongs-to", "submodule");
        return LY_EVALID;
    }

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule(ctx->ctx, NULL, submod->name, NULL);
    if (dup && strcmp(dup->belongsto, submod->belongsto)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Name collision between submodules of name \"%s\".", dup->name);
        return LY_EVALID;
    }

    return ret;
}

LY_ERR
yang_parse_submodule(struct lys_parser_ctx **context, struct ly_ctx *ly_ctx, struct lys_parser_ctx *main_ctx, const char *data, struct lysp_submodule **submod)
{
    LY_ERR ret = LY_SUCCESS;
    char *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_submodule *mod_p = NULL;

    /* create context */
    *context = calloc(1, sizeof **context);
    LY_CHECK_ERR_RET(!(*context), LOGMEM(ly_ctx), LY_EMEM);
    (*context)->ctx = ly_ctx;
    (*context)->pos_type = LY_VLOG_LINE;
    (*context)->line = 1;

    /* map the typedefs and groupings list from main context to the submodule's context */
    memcpy(&(*context)->tpdfs_nodes, &main_ctx->tpdfs_nodes, sizeof main_ctx->tpdfs_nodes);
    memcpy(&(*context)->grps_nodes, &main_ctx->grps_nodes, sizeof main_ctx->grps_nodes);

    /* "module"/"submodule" */
    ret = get_keyword(*context, &data, &kw, &word, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (kw == LY_STMT_MODULE) {
        LOGERR((*context)->ctx, LY_EDENIED, "Input data contains module in situation when a submodule is expected.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != LY_STMT_SUBMODULE) {
        LOGVAL_PARSER(*context, LY_VCODE_MOD_SUBOMD, ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM((*context)->ctx), cleanup);
    mod_p->parsing = 1;

    /* substatements */
    ret = parse_submodule(*context, &data, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* read some trailing spaces or new lines */
    while(*data && isspace(*data)) {
        data++;
    }
    if (*data) {
        LOGVAL_PARSER(*context, LY_VCODE_TRAILING_SUBMOD, 15, data, strlen(data) > 15 ? "..." : "");
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p->parsing = 0;
    *submod = mod_p;

cleanup:
    if (ret) {
        lysp_submodule_free((*context)->ctx, mod_p);
        lys_parser_ctx_free(*context);
        *context = NULL;
    }

    return ret;
}

LY_ERR
yang_parse_module(struct lys_parser_ctx **context, const char *data, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    char *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_module *mod_p = NULL;

    /* create context */
    *context = calloc(1, sizeof **context);
    LY_CHECK_ERR_RET(!(*context), LOGMEM(mod->ctx), LY_EMEM);
    (*context)->ctx = mod->ctx;
    (*context)->pos_type = LY_VLOG_LINE;
    (*context)->line = 1;

    /* "module"/"submodule" */
    ret = get_keyword(*context, &data, &kw, &word, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (kw == LY_STMT_SUBMODULE) {
        LOGERR((*context)->ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != LY_STMT_MODULE) {
        LOGVAL_PARSER((*context), LY_VCODE_MOD_SUBOMD, ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM((*context)->ctx), cleanup);
    mod_p->mod = mod;
    mod_p->parsing = 1;

    /* substatements */
    ret = parse_module(*context, &data, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* read some trailing spaces or new lines */
    while(*data && isspace(*data)) {
        data++;
    }
    if (*data) {
        LOGVAL_PARSER(*context, LY_VCODE_TRAILING_MOD, 15, data, strlen(data) > 15 ? "..." : "");
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret) {
        lysp_module_free(mod_p);
        lys_parser_ctx_free(*context);
        *context = NULL;
    }

    return ret;
}
