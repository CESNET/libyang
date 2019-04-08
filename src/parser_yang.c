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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>

#include "common.h"
#include "context.h"
#include "libyang.h"
#include "tree_schema_internal.h"

/* Macro to check YANG's yang-char grammar rule */
#define is_yangutf8char(c) ((c >= 0x20 && c <= 0xd77) || c == 0x09 || c == 0x0a || c == 0x0d || \
        (c >= 0xe000 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
        (c >= 0x10000 && c <= 0x1fffd) || (c >= 0x20000 && c <= 0x2fffd) || \
        (c >= 0x30000 && c <= 0x3fffd) || (c >= 0x40000 && c <= 0x2fffd) || \
        (c >= 0x50000 && c <= 0x5fffd) || (c >= 0x60000 && c <= 0x6fffd) || \
        (c >= 0x70000 && c <= 0x7fffd) || (c >= 0x80000 && c <= 0x8fffd) || \
        (c >= 0x90000 && c <= 0x9fffd) || (c >= 0xa0000 && c <= 0xafffd) || \
        (c >= 0xb0000 && c <= 0xbfffd) || (c >= 0xc0000 && c <= 0xcfffd) || \
        (c >= 0xd0000 && c <= 0xdfffd) || (c >= 0xe0000 && c <= 0xefffd) || \
        (c >= 0xf0000 && c <= 0xffffd) || (c >= 0x100000 && c <= 0x10fffd))

/**
 * @brief Try to find object with MEMBER string matching the IDENT in the given ARRAY.
 * Macro logs an error message and returns LY_EVALID in case of existence of a matching object.
 *
 * @param[in] CTX yang parser context for logging.
 * @param[in] ARRAY [sized array](@ref sizedarrays) of a generic objects with member named MEMBER to search.
 * @param[in] MEMBER Name of the member of the objects in the ARRAY to compare.
 * @param[in] STMT Name of the compared YANG statements for logging.
 * @param[in] IDENT String trying to find in the ARRAY's objects inside the MEMBER member.
 */
#define CHECK_UNIQUENESS(CTX, ARRAY, MEMBER, STMT, IDENT) \
    if (ARRAY) { \
        for (unsigned int u = 0; u < LY_ARRAY_SIZE(ARRAY) - 1; ++u) { \
            if (!strcmp((ARRAY)[u].MEMBER, IDENT)) { \
                LOGVAL_YANG(CTX, LY_VCODE_DUPIDENT, IDENT, STMT); \
                return LY_EVALID; \
            } \
        } \
    }

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
    else {(TARGET) = lydict_insert((CTX)->ctx, WORD, LEN);}

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
    if (KW == YANG_SEMICOLON) { \
        CHECKGOTO; \
        return LY_SUCCESS; \
    } \
    if (KW != YANG_LEFT_BRACE) { \
        LOGVAL_YANG(CTX, LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", expected \";\" or \"{\".", ly_stmt2str(KW)); \
        return LY_EVALID; \
    } \
    for (ERR = get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN); \
            !ERR && (KW != YANG_RIGHT_BRACE); \
            ERR = get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN))

/**
 * @brief Check module version is at least 2 (YANG 1.1) because of the keyword presence.
 * Logs error message and returns LY_EVALID in case of module in YANG version 1.0.
 * @param[in] CTX yang parser context to get current module and for logging.
 * @param[in] KW keyword allowed only in YANG version 1.1 (or later) - for logging.
 * @param[in] PARENT parent statement where the KW is present - for logging.
 */
#define YANG_CHECK_STMTVER2_RET(CTX, KW, PARENT) \
    if ((CTX)->mod_version < 2) {LOGVAL_YANG((CTX), LY_VCODE_INCHILDSTMT2, KW, PARENT); return LY_EVALID;}

static LY_ERR parse_container(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
static LY_ERR parse_uses(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
static LY_ERR parse_choice(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
static LY_ERR parse_case(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
static LY_ERR parse_list(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings);
static LY_ERR parse_grouping(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_grp **groupings);

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
static LY_ERR
buf_add_char(struct ly_ctx *ctx, const char **input, size_t len, char **buf, size_t *buf_len, size_t *buf_used)
{
    if (*buf_len <= (*buf_used) + len) {
        *buf_len += 16;
        *buf = ly_realloc(*buf, *buf_len);
        LY_CHECK_ERR_RET(!*buf, LOGMEM(ctx), LY_EMEM);
    }
    memcpy(&(*buf)[*buf_used], *input, len);

    (*buf_used) += len;
    (*input) += len;

    return LY_SUCCESS;
}

/**
 * @brief Check that \p c is valid UTF8 code point for YANG string.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @return LY_ERR values.
 */
static LY_ERR
check_stringchar(struct ly_parser_ctx *ctx, unsigned int c)
{
    if (!is_yangutf8char(c)) {
        LOGVAL_YANG(ctx, LY_VCODE_INCHAR, c);
        return LY_EVALID;
    }
    return LY_SUCCESS;
}

/**
 * @brief Check that \p c is valid UTF8 code point for YANG identifier.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @param[in] first Flag to check the first character of an identifier, which is more restricted.
 * @param[in,out] prefix Storage for internally used flag in case of possible prefixed identifiers:
 * 0 - colon not yet found (no prefix)
 * 1 - \p c is the colon character
 * 2 - prefix already processed, now processing the identifier
 *
 * If the identifier cannot be prefixed, NULL is expected.
 * @return LY_ERR values.
 */
static LY_ERR
check_identifierchar(struct ly_parser_ctx *ctx, unsigned int c, int first, int *prefix)
{
    if (first || (prefix && (*prefix) == 1)) {
        if (!is_yangidentstartchar(c)) {
            LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Invalid identifier first character '%c'.", c);
            return LY_EVALID;
        }
        if (prefix) {
            if (first) {
                (*prefix) = 0;
            } else {
                (*prefix) = 2;
            }
        }
    } else if (c == ':' && prefix && (*prefix) == 0) {
        (*prefix) = 1;
    } else if (!is_yangidentchar(c)) {
        LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Invalid identifier character '%c'.", c);
        return LY_EVALID;
    }

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
 *
 * @return LY_ERR values.
 */
static LY_ERR
buf_store_char(struct ly_parser_ctx *ctx, const char **input, enum yang_arg arg,
               char **word_p, size_t *word_len, char **word_b, size_t *buf_len, int need_buf)
{
    int prefix = 0;
    unsigned int c;
    size_t len;

    /* get UTF8 code point (and number of bytes coding the character) */
    LY_CHECK_ERR_RET(ly_getutf8(input, &c, &len),
                     LOGVAL_YANG(ctx, LY_VCODE_INCHAR, (*input)[-len]), LY_EVALID);
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
        LY_CHECK_RET(check_identifierchar(ctx, c, !(*word_len), NULL));
        break;
    case Y_PREF_IDENTIF_ARG:
        LY_CHECK_RET(check_identifierchar(ctx, c, !(*word_len), &prefix));
        break;
    case Y_STR_ARG:
    case Y_MAYBE_STR_ARG:
        LY_CHECK_RET(check_stringchar(ctx, c));
        break;
    }

    if (word_b && *word_b) {
        /* add another character into buffer */
        if (buf_add_char(ctx->ctx, input, len, word_b, buf_len, word_len)) {
            return LY_EMEM;
        }

        /* in case of realloc */
        *word_p = *word_b;
    } else if (need_buf) {
        /* check  valid combination of input paremeters - if need_buf specified, word_b must be provided */
        assert(word_b);

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
static LY_ERR
skip_comment(struct ly_parser_ctx *ctx, const char **data, int comment)
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
            } else {
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
        LOGVAL_YANG(ctx, LYVE_SYNTAX, "Unexpected end-of-file, non-terminated comment.");
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
read_qstring(struct ly_parser_ctx *ctx, const char **data, enum yang_arg arg, char **word_p, char **word_b, size_t *word_len,
             size_t *buf_len)
{
    /* string: 0 - string ended, 1 - string with ', 2 - string with ", 3 - string with " with last character \,
     *         4 - string finished, now skipping whitespaces looking for +,
     *         5 - string continues after +, skipping whitespaces */
    unsigned int string, block_indent = 0, current_indent = 0, need_buf = 0;
    const char *c;

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
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf));
                break;
            }
            break;
        case 2:
            switch (**data) {
            case '\"':
                /* string may be finished, but check for + */
                string = 4;
                MOVE_INPUT(ctx, data, 1);
                break;
            case '\\':
                /* special character following */
                string = 3;
                ++(*data);
                break;
            case ' ':
                if (current_indent < block_indent) {
                    ++current_indent;
                    MOVE_INPUT(ctx, data, 1);
                } else {
                    /* check and store character */
                    LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf));
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
                        LY_CHECK_RET(buf_store_char(ctx, &c, arg, word_p, word_len, word_b, buf_len, need_buf));
                    }
                    ++(*data);
                } else {
                    /* check and store character */
                    LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf));
                    /* additional characters for indentation - only 1 was count in buf_store_char */
                    ctx->indent += 7;
                }
                break;
            case '\n':
                if (block_indent) {
                    /* we will be removing the indents so we need our own buffer */
                    need_buf = 1;

                    /* remove trailing tabs and spaces */
                    while ((*word_len) && ((*word_p)[(*word_len) - 1] == '\t' || (*word_p)[(*word_len) - 1] == ' ')) {
                        --(*word_len);
                    }

                    /* start indentation */
                    current_indent = 0;
                }

                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf));

                /* maintain line number */
                ++ctx->line;

                /* reset context indentation counter for possible string after this one */
                ctx->indent = 0;
                break;
            default:
                /* first non-whitespace character, stop eating indentation */
                current_indent = block_indent;

                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, buf_len, need_buf));
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
                LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Double-quoted string unknown special character '\\%c'.", **data);
                return LY_EVALID;
            }

            /* check and store character */
            LY_CHECK_RET(buf_store_char(ctx, &c, arg, word_p, word_len, word_b, buf_len, need_buf));

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
                LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Both string parts divided by '+' must be quoted.");
                return LY_EVALID;
            }
            MOVE_INPUT(ctx, data, 1);
            break;
        default:
            return LY_EINT;
        }
    }

string_end:
    return LY_SUCCESS;
}

/**
 * @brief Get another YANG string from the raw data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] arg Type of YANG keyword argument expected.
 * @param[out] word_p Pointer to the read string. Can return NULL if \p arg is #Y_MAYBE_STR_ARG.
 * @param[out] word_b Pointer to a dynamically-allocated buffer holding the read string. If not needed,
 * set to NULL. Otherwise equal to \p word_p.
 * @param[out] word_len Length of the read string.
 *
 * @return LY_ERR values.
 */
static LY_ERR
get_argument(struct ly_parser_ctx *ctx, const char **data, enum yang_arg arg, char **word_p, char **word_b, size_t *word_len)
{
    size_t buf_len = 0;

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
                LOGVAL_YANG(ctx, LY_VCODE_INSTREXP, 1, *data,
                            "unquoted string character, optsep, semicolon or opening brace");
                return LY_EVALID;
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
                LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, &buf_len, 0));
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

            LOGVAL_YANG(ctx, LY_VCODE_INSTREXP, 1, *data, "an argument");
            return LY_EVALID;
        case '}':
            /* invalid - braces cannot be in unquoted string (opening braces terminates the string and can follow it) */
            LOGVAL_YANG(ctx, LY_VCODE_INSTREXP, 1, *data,
                        "unquoted string character, optsep, semicolon or opening brace");
            return LY_EVALID;
        default:
            LY_CHECK_RET(buf_store_char(ctx, data, arg, word_p, word_len, word_b, &buf_len, 0));
            break;
        }
    }

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
static LY_ERR
get_keyword(struct ly_parser_ctx *ctx, const char **data, enum yang_keyword *kw, char **word_p, size_t *word_len)
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
                LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Invalid identifier first character '/'.");
                return LY_EVALID;
            }
            break;
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

#define IF_KW(STR, LEN, STMT) if (!strncmp(*(data), STR, LEN)) {MOVE_INPUT(ctx, data, LEN);*kw=STMT;}
#define IF_KW_PREFIX(STR, LEN) if (!strncmp(*(data), STR, LEN)) {MOVE_INPUT(ctx, data, LEN);
#define IF_KW_PREFIX_END }

keyword_start:
    word_start = *data;
    *kw = YANG_NONE;

    /* read the keyword itself */
    switch (**data) {
    case 'a':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("rgument", 7, YANG_ARGUMENT)
        else IF_KW("ugment", 6, YANG_AUGMENT)
        else IF_KW("ction", 5, YANG_ACTION)
        else IF_KW_PREFIX("ny", 2)
            IF_KW("data", 4, YANG_ANYDATA)
            else IF_KW("xml", 3, YANG_ANYXML)
        IF_KW_PREFIX_END
        break;
    case 'b':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("ase", 3, YANG_BASE)
        else IF_KW("elongs-to", 9, YANG_BELONGS_TO)
        else IF_KW("it", 2, YANG_BIT)
        break;
    case 'c':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("ase", 3, YANG_CASE)
        else IF_KW("hoice", 5, YANG_CHOICE)
        else IF_KW_PREFIX("on", 2)
            IF_KW("fig", 3, YANG_CONFIG)
            else IF_KW_PREFIX("ta", 2)
                IF_KW("ct", 2, YANG_CONTACT)
                else IF_KW("iner", 4, YANG_CONTAINER)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'd':
        MOVE_INPUT(ctx, data, 1);
        IF_KW_PREFIX("e", 1)
            IF_KW("fault", 5, YANG_DEFAULT)
            else IF_KW("scription", 9, YANG_DESCRIPTION)
            else IF_KW_PREFIX("viat", 4)
                IF_KW("e", 1, YANG_DEVIATE)
                else IF_KW("ion", 3, YANG_DEVIATION)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'e':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("num", 3, YANG_ENUM)
        else IF_KW_PREFIX("rror-", 5)
            IF_KW("app-tag", 7, YANG_ERROR_APP_TAG)
            else IF_KW("message", 7, YANG_ERROR_MESSAGE)
        IF_KW_PREFIX_END
        else IF_KW("xtension", 8, YANG_EXTENSION)
        break;
    case 'f':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("eature", 6, YANG_FEATURE)
        else IF_KW("raction-digits", 14, YANG_FRACTION_DIGITS)
        break;
    case 'g':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("rouping", 7, YANG_GROUPING)
        break;
    case 'i':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("dentity", 7, YANG_IDENTITY)
        else IF_KW("f-feature", 9, YANG_IF_FEATURE)
        else IF_KW("mport", 5, YANG_IMPORT)
        else IF_KW_PREFIX("n", 1)
            IF_KW("clude", 5, YANG_INCLUDE)
            else IF_KW("put", 3, YANG_INPUT)
        IF_KW_PREFIX_END
        break;
    case 'k':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("ey", 2, YANG_KEY)
        break;
    case 'l':
        MOVE_INPUT(ctx, data, 1);
        IF_KW_PREFIX("e", 1)
            IF_KW("af-list", 7, YANG_LEAF_LIST)
            else IF_KW("af", 2, YANG_LEAF)
            else IF_KW("ngth", 4, YANG_LENGTH)
        IF_KW_PREFIX_END
        else IF_KW("ist", 3, YANG_LIST)
        break;
    case 'm':
        MOVE_INPUT(ctx, data, 1);
        IF_KW_PREFIX("a", 1)
            IF_KW("ndatory", 7, YANG_MANDATORY)
            else IF_KW("x-elements", 10, YANG_MAX_ELEMENTS)
        IF_KW_PREFIX_END
        else IF_KW("in-elements", 11, YANG_MIN_ELEMENTS)
        else IF_KW("ust", 3, YANG_MUST)
        else IF_KW_PREFIX("od", 2)
            IF_KW("ule", 3, YANG_MODULE)
            else IF_KW("ifier", 5, YANG_MODIFIER)
        IF_KW_PREFIX_END
        break;
    case 'n':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("amespace", 8, YANG_NAMESPACE)
        else IF_KW("otification", 11, YANG_NOTIFICATION)
        break;
    case 'o':
        MOVE_INPUT(ctx, data, 1);
        IF_KW_PREFIX("r", 1)
            IF_KW("dered-by", 8, YANG_ORDERED_BY)
            else IF_KW("ganization", 10, YANG_ORGANIZATION)
        IF_KW_PREFIX_END
        else IF_KW("utput", 5, YANG_OUTPUT)
        break;
    case 'p':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("ath", 3, YANG_PATH)
        else IF_KW("attern", 6, YANG_PATTERN)
        else IF_KW("osition", 7, YANG_POSITION)
        else IF_KW_PREFIX("re", 2)
            IF_KW("fix", 3, YANG_PREFIX)
            else IF_KW("sence", 5, YANG_PRESENCE)
        IF_KW_PREFIX_END
        break;
    case 'r':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("ange", 4, YANG_RANGE)
        else IF_KW_PREFIX("e", 1)
            IF_KW_PREFIX("f", 1)
                IF_KW("erence", 6, YANG_REFERENCE)
                else IF_KW("ine", 3, YANG_REFINE)
            IF_KW_PREFIX_END
            else IF_KW("quire-instance", 14, YANG_REQUIRE_INSTANCE)
            else IF_KW("vision-date", 11, YANG_REVISION_DATE)
            else IF_KW("vision", 6, YANG_REVISION)
        IF_KW_PREFIX_END
        else IF_KW("pc", 2, YANG_RPC)
        break;
    case 's':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("tatus", 5, YANG_STATUS)
        else IF_KW("ubmodule", 8, YANG_SUBMODULE)
        break;
    case 't':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("ypedef", 6, YANG_TYPEDEF)
        else IF_KW("ype", 3, YANG_TYPE)
        break;
    case 'u':
        MOVE_INPUT(ctx, data, 1);
        IF_KW_PREFIX("ni", 2)
            IF_KW("que", 3, YANG_UNIQUE)
            else IF_KW("ts", 2, YANG_UNITS)
        IF_KW_PREFIX_END
        else IF_KW("ses", 3, YANG_USES)
        break;
    case 'v':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("alue", 4, YANG_VALUE)
        break;
    case 'w':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("hen", 3, YANG_WHEN)
        break;
    case 'y':
        MOVE_INPUT(ctx, data, 1);
        IF_KW("ang-version", 11, YANG_YANG_VERSION)
        else IF_KW("in-element", 10, YANG_YIN_ELEMENT)
        break;
    case ';':
        MOVE_INPUT(ctx, data, 1);
        *kw = YANG_SEMICOLON;
        goto success;
    case '{':
        MOVE_INPUT(ctx, data, 1);
        *kw = YANG_LEFT_BRACE;
        goto success;
    case '}':
        MOVE_INPUT(ctx, data, 1);
        *kw = YANG_RIGHT_BRACE;
        goto success;
    default:
        break;
    }

#undef IF_KW
#undef IF_KW_PREFIX
#undef IF_KW_PREFIX_END

    if (*kw != YANG_NONE) {
        /* make sure we have the whole keyword */
        switch (**data) {
        case '\n':
            ++ctx->line;
            /* fallthrough */
        case ' ':
        case '\t':
            /* mandatory "sep" */
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
            if (*kw == YANG_INPUT || *kw == YANG_OUTPUT) {
                break;
            }
            /* fallthrough */
        default:
            MOVE_INPUT(ctx, data, 1);
            LOGVAL_YANG(ctx, LY_VCODE_INSTREXP, (int)(*data - word_start), word_start,
                        "a keyword followed by a separator");
            return LY_EVALID;
        }
    } else {
        /* still can be an extension */
        prefix = 0;
extension:
        while (**data && (**data != ' ') && (**data != '\t') && (**data != '\n') && (**data != '{') && (**data != ';')) {
            LY_CHECK_ERR_RET(ly_getutf8(data, &c, &len),
                             LOGVAL_YANG(ctx, LY_VCODE_INCHAR, (*data)[-len]), LY_EVALID);
            ++ctx->indent;
            /* check character validity */
            LY_CHECK_RET(check_identifierchar(ctx, c, *data - len == word_start ? 1 : 0, &prefix));
        }
        if (!**data) {
            LOGVAL_YANG(ctx, LY_VCODE_EOF);
            return LY_EVALID;
        }

        /* prefix is mandatory for extension instances */
        if (prefix != 2) {
            LOGVAL_YANG(ctx, LY_VCODE_INSTREXP, (int)(*data - word_start), word_start, "a keyword");
            return LY_EVALID;
        }

        *kw = YANG_CUSTOM;
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
 * @param[in] word Extension instance substatement name (keyword).
 * @param[in] word_len Extension instance substatement name length.
 * @param[in,out] child Children of this extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_ext_substmt(struct ly_parser_ctx *ctx, const char **data, char *word, size_t word_len,
                  struct lysp_stmt **child)
{
    char *buf;
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw;
    struct lysp_stmt *stmt, *par_child;

    stmt = calloc(1, sizeof *stmt);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(NULL), LY_EMEM);

    stmt->stmt = lydict_insert(ctx->ctx, word, word_len);

    /* get optional argument */
    LY_CHECK_RET(get_argument(ctx, data, Y_MAYBE_STR_ARG, &word, &buf, &word_len));

    if (word) {
        if (buf) {
            stmt->arg = lydict_insert_zc(ctx->ctx, word);
        } else {
            stmt->arg = lydict_insert(ctx->ctx, word, word_len);
        }
    }

    /* insert into parent statements */
    if (!*child) {
        *child = stmt;
    } else {
        for (par_child = *child; par_child->next; par_child = par_child->next);
        par_child->next = stmt;
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, ) {
        LY_CHECK_RET(parse_ext_substmt(ctx, data, word, word_len, &stmt->child));
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
parse_ext(struct ly_parser_ctx *ctx, const char **data, const char *ext_name, int ext_name_len, LYEXT_SUBSTMT insubstmt,
          uint32_t insubstmt_index, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    struct lysp_ext_instance *e;
    enum yang_keyword kw;

    LY_ARRAY_NEW_RET(ctx->ctx, *exts, e, LY_EMEM);

    /* store name and insubstmt info */
    e->name = lydict_insert(ctx->ctx, ext_name, ext_name_len);
    e->insubstmt = insubstmt;
    e->insubstmt_index = insubstmt_index;

    /* get optional argument */
    LY_CHECK_RET(get_argument(ctx, data, Y_MAYBE_STR_ARG, &word, &buf, &word_len));

    if (word) {
        INSERT_WORD(ctx, buf, e->argument, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        LY_CHECK_RET(parse_ext_substmt(ctx, data, word, word_len, &e->child));
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
parse_text_field(struct ly_parser_ctx *ctx, const char **data, LYEXT_SUBSTMT substmt, uint32_t substmt_index,
                 const char **value, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*value) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, lyext_substmt2str(substmt));
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, arg, &word, &buf, &word_len));

    /* store value and spend buf if allocated */
    INSERT_WORD(ctx, buf, *value, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, substmt, substmt_index, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), lyext_substmt2str(substmt));
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
parse_yangversion(struct ly_parser_ctx *ctx, const char **data, uint8_t *version, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*version) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "yang-version");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 3) && !strncmp(word, "1.0", word_len)) {
        *version = LYS_VERSION_1_0;
    } else if ((word_len == 3) && !strncmp(word, "1.1", word_len)) {
        *version = LYS_VERSION_1_1;
    } else {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "yang-version");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_VERSION, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "yang-version");
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
parse_belongsto(struct ly_parser_ctx *ctx, const char **data, const char **belongsto, const char **prefix, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*belongsto) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "belongs-to");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));

    INSERT_WORD(ctx, buf, *belongsto, word, word_len);
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, prefix, Y_IDENTIF_ARG, exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_BELONGSTO, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "belongs-to");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!*prefix) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "prefix", "belongs-to");
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
parse_revisiondate(struct ly_parser_ctx *ctx, const char **data, char *rev, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (rev[0]) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "revision-date");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

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
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_REVISIONDATE, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "revision-date");
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
parse_include(struct ly_parser_ctx *ctx, const char *module_name, const char **data, struct lysp_include **includes)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_include *inc;

    LY_ARRAY_NEW_RET(ctx->ctx, *includes, inc, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));

    INSERT_WORD(ctx, buf, inc->name, word, word_len);

    /* submodules share the namespace with the module names, so there must not be
     * a module of the same name in the context, no need for revision matching */
    if (!strcmp(module_name, inc->name) || ly_ctx_get_module_latest(ctx->ctx, inc->name)) {
        LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Name collision between module and submodule of name \"%s\".", inc->name);
        return LY_EVALID;
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            YANG_CHECK_STMTVER2_RET(ctx, "description", "include");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &inc->dsc, Y_STR_ARG, &inc->exts));
            break;
        case YANG_REFERENCE:
            YANG_CHECK_STMTVER2_RET(ctx, "reference", "include");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &inc->ref, Y_STR_ARG, &inc->exts));
            break;
        case YANG_REVISION_DATE:
            LY_CHECK_RET(parse_revisiondate(ctx, data, inc->rev, &inc->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &inc->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "include");
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
parse_import(struct ly_parser_ctx *ctx, const char *module_prefix, const char **data, struct lysp_import **imports)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_import *imp;

    LY_ARRAY_NEW_RET(ctx->ctx, *imports, imp, LY_EVALID);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, imp->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, &imp->prefix, Y_IDENTIF_ARG, &imp->exts));
            LY_CHECK_RET(lysp_check_prefix(ctx, *imports, module_prefix, &imp->prefix), LY_EVALID);
            break;
        case YANG_DESCRIPTION:
            YANG_CHECK_STMTVER2_RET(ctx, "description", "import");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &imp->dsc, Y_STR_ARG, &imp->exts));
            break;
        case YANG_REFERENCE:
            YANG_CHECK_STMTVER2_RET(ctx, "reference", "import");
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &imp->ref, Y_STR_ARG, &imp->exts));
            break;
        case YANG_REVISION_DATE:
            LY_CHECK_RET(parse_revisiondate(ctx, data, imp->rev, &imp->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &imp->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "import");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    LY_CHECK_ERR_RET(!imp->prefix, LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "prefix", "import"), LY_EVALID);

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
parse_revision(struct ly_parser_ctx *ctx, const char **data, struct lysp_revision **revs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_revision *rev;

    LY_ARRAY_NEW_RET(ctx->ctx, *revs, rev, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    /* check value */
    if (lysp_check_date(ctx, word, word_len, "revision")) {
        return LY_EVALID;
    }

    strncpy(rev->date, word, word_len);
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &rev->dsc, Y_STR_ARG, &rev->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &rev->ref, Y_STR_ARG, &rev->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &rev->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "revision");
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
parse_text_fields(struct ly_parser_ctx *ctx, const char **data, LYEXT_SUBSTMT substmt, const char ***texts, enum yang_arg arg,
                  struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    const char **item;
    size_t word_len;
    enum yang_keyword kw;

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(ctx->ctx, *texts, item, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, arg, &word, &buf, &word_len));

    INSERT_WORD(ctx, buf, *item, word, word_len);
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, substmt, LY_ARRAY_SIZE(*texts) - 1, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), lyext_substmt2str(substmt));
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
parse_config(struct ly_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*flags & LYS_CONFIG_MASK) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "config");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_CONFIG_W;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_CONFIG_R;
    } else {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "config");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_CONFIG, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "config");
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
parse_mandatory(struct ly_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*flags & LYS_MAND_MASK) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "mandatory");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_MAND_TRUE;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "mandatory");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MANDATORY, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "mandatory");
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
parse_restr(struct ly_parser_ctx *ctx, const char **data, enum yang_keyword restr_kw, struct lysp_restr *restr)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    INSERT_WORD(ctx, buf, restr->arg, word, word_len);
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case YANG_ERROR_APP_TAG:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case YANG_ERROR_MESSAGE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &restr->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(restr_kw));
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
parse_restrs(struct ly_parser_ctx *ctx, const char **data, enum yang_keyword restr_kw, struct lysp_restr **restrs)
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
parse_status(struct ly_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "status");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 7) && !strncmp(word, "current", word_len)) {
        *flags |= LYS_STATUS_CURR;
    } else if ((word_len == 10) && !strncmp(word, "deprecated", word_len)) {
        *flags |= LYS_STATUS_DEPRC;
    } else if ((word_len == 8) && !strncmp(word, "obsolete", word_len)) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "status");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_STATUS, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "status");
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
static LY_ERR
parse_when(struct ly_parser_ctx *ctx, const char **data, struct lysp_when **when_p)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_when *when;

    if (*when_p) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "when");
        return LY_EVALID;
    }

    when = calloc(1, sizeof *when);
    LY_CHECK_ERR_RET(!when, LOGMEM(ctx->ctx), LY_EMEM);
    *when_p = when;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, when->cond, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &when->dsc, Y_STR_ARG, &when->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &when->ref, Y_STR_ARG, &when->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &when->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "when");
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
static LY_ERR
parse_any(struct ly_parser_ctx *ctx, const char **data, enum yang_keyword kw, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    struct lysp_node *iter;
    struct lysp_node_anydata *any;

    /* create structure */
    any = calloc(1, sizeof *any);
    LY_CHECK_ERR_RET(!any, LOGMEM(ctx->ctx), LY_EMEM);
    any->nodetype = kw == YANG_ANYDATA ? LYS_ANYDATA : LYS_ANYXML;
    any->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)any;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)any;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, any->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &any->flags, &any->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &any->dsc, Y_STR_ARG, &any->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &any->iffeatures, Y_STR_ARG, &any->exts));
            break;
        case YANG_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &any->flags, &any->exts));
            break;
        case YANG_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &any->musts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &any->ref, Y_STR_ARG, &any->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &any->flags, &any->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &any->when));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &any->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw),
                   (any->nodetype & LYS_ANYDATA) == LYS_ANYDATA ? ly_stmt2str(YANG_ANYDATA) : ly_stmt2str(YANG_ANYXML));
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
static LY_ERR
parse_type_enum_value_pos(struct ly_parser_ctx *ctx, const char **data, enum yang_keyword val_kw, int64_t *value, uint16_t *flags,
                          struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    long int num;
    unsigned long int unum;
    enum yang_keyword kw;

    if (*flags & LYS_SET_VALUE) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(val_kw));
        return LY_EVALID;
    }
    *flags |= LYS_SET_VALUE;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if (!word_len || (word[0] == '+') || ((word[0] == '0') && (word_len > 1)) || ((val_kw == YANG_VALUE) && !strncmp(word, "-0", 2))) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
        goto error;
    }

    errno = 0;
    if (val_kw == YANG_VALUE) {
        num = strtol(word, &ptr, 10);
        if (num < INT64_C(-2147483648) || num > INT64_C(2147483647)) {
            LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
            goto error;
        }
    } else {
        unum = strtoul(word, &ptr, 10);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
            goto error;
        }
    }
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, ly_stmt2str(val_kw));
        goto error;
    }
    if (errno == ERANGE) {
        LOGVAL_YANG(ctx, LY_VCODE_OOB, word_len, word, ly_stmt2str(val_kw));
        goto error;
    }
    if (val_kw == YANG_VALUE) {
        *value = num;
    } else {
        *value = unum;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, val_kw == YANG_VALUE ? LYEXT_SUBSTMT_VALUE : LYEXT_SUBSTMT_POSITION, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(val_kw));
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
parse_type_enum(struct ly_parser_ctx *ctx, const char **data, enum yang_keyword enum_kw, struct lysp_type_enum **enums)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len, u;
    enum yang_keyword kw;
    struct lysp_type_enum *enm;

    LY_ARRAY_NEW_RET(ctx->ctx, *enums, enm, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, enum_kw == YANG_ENUM ? Y_STR_ARG : Y_IDENTIF_ARG, &word, &buf, &word_len));
    if (enum_kw == YANG_ENUM) {
        if (!word_len) {
            LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Enum name must not be zero-length.");
            free(buf);
            return LY_EVALID;
        } else if (isspace(word[0]) || isspace(word[word_len - 1])) {
            LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Enum name must not have any leading or trailing whitespaces (\"%.*s\").",
                        word_len, word);
            free(buf);
            return LY_EVALID;
        } else {
            for (u = 0; u < word_len; ++u) {
                if (iscntrl(word[u])) {
                    LOGWRN(ctx->ctx, "Control characters in enum name should be avoided (\"%.*s\", character number %d).",
                           word_len, word, u + 1);
                    break;
                }
            }
        }
    } else { /* YANG_BIT */

    }
    INSERT_WORD(ctx, buf, enm->name, word, word_len);
    CHECK_UNIQUENESS(ctx, *enums, name, ly_stmt2str(enum_kw), enm->name);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &enm->dsc, Y_STR_ARG, &enm->exts));
            break;
        case YANG_IF_FEATURE:
            YANG_CHECK_STMTVER2_RET(ctx, "if-feature", ly_stmt2str(enum_kw));
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &enm->iffeatures, Y_STR_ARG, &enm->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &enm->ref, Y_STR_ARG, &enm->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &enm->flags, &enm->exts));
            break;
        case YANG_VALUE:
        case YANG_POSITION:
            LY_CHECK_RET(parse_type_enum_value_pos(ctx, data, kw, &enm->value, &enm->flags, &enm->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &enm->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(enum_kw));
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
parse_type_fracdigits(struct ly_parser_ctx *ctx, const char **data, uint8_t *fracdig, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    unsigned long int num;
    enum yang_keyword kw;

    if (*fracdig) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "fraction-digits");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if (!word_len || (word[0] == '0') || !isdigit(word[0])) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(word, &ptr, 10);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > 18)) {
        LOGVAL_YANG(ctx, LY_VCODE_OOB, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }
    *fracdig = num;
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_FRACDIGITS, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "fraction-digits");
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
parse_type_reqinstance(struct ly_parser_ctx *ctx, const char **data, uint8_t *reqinst, uint16_t *flags,
                       struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*flags & LYS_SET_REQINST) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "require-instance");
        return LY_EVALID;
    }
    *flags |= LYS_SET_REQINST;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *reqinst = 1;
    } else if ((word_len != 5) || strncmp(word, "false", word_len)) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "require-instance");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_REQINSTANCE, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "require-instance");
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
parse_type_pattern_modifier(struct ly_parser_ctx *ctx, const char **data, const char **pat, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if ((*pat)[0] == 0x15) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "modifier");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len != 12) || strncmp(word, "invert-match", word_len)) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "modifier");
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
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MODIFIER, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "modifier");
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
parse_type_pattern(struct ly_parser_ctx *ctx, const char **data, struct lysp_restr **patterns)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(ctx->ctx, *patterns, restr, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

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
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case YANG_ERROR_APP_TAG:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case YANG_ERROR_MESSAGE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case YANG_MODIFIER:
            YANG_CHECK_STMTVER2_RET(ctx, "modifier", "pattern");
            LY_CHECK_RET(parse_type_pattern_modifier(ctx, data, &restr->arg, &restr->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &restr->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "pattern");
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
parse_type(struct ly_parser_ctx *ctx, const char **data, struct lysp_type *type)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_type *nest_type;

    if (type->name) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "type");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_PREF_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, type->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_BASE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_BASE, &type->bases, Y_PREF_IDENTIF_ARG, &type->exts));
            type->flags |= LYS_SET_BASE;
            break;
        case YANG_BIT:
            LY_CHECK_RET(parse_type_enum(ctx, data, kw, &type->bits));
            type->flags |= LYS_SET_BIT;
            break;
        case YANG_ENUM:
            LY_CHECK_RET(parse_type_enum(ctx, data, kw, &type->enums));
            type->flags |= LYS_SET_ENUM;
            break;
        case YANG_FRACTION_DIGITS:
            LY_CHECK_RET(parse_type_fracdigits(ctx, data, &type->fraction_digits, &type->exts));
            type->flags |= LYS_SET_FRDIGITS;
            break;
        case YANG_LENGTH:
            if (type->length) {
                LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                return LY_EVALID;
            }
            type->length = calloc(1, sizeof *type->length);
            LY_CHECK_ERR_RET(!type->length, LOGMEM(ctx->ctx), LY_EMEM);

            LY_CHECK_RET(parse_restr(ctx, data, kw, type->length));
            type->flags |= LYS_SET_LENGTH;
            break;
        case YANG_PATH:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PATH, 0, &type->path, Y_STR_ARG, &type->exts));
            type->flags |= LYS_SET_PATH;
            break;
        case YANG_PATTERN:
            LY_CHECK_RET(parse_type_pattern(ctx, data, &type->patterns));
            type->flags |= LYS_SET_PATTERN;
            break;
        case YANG_RANGE:
            if (type->range) {
                LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                return LY_EVALID;
            }
            type->range = calloc(1, sizeof *type->range);
            LY_CHECK_ERR_RET(!type->range, LOGMEM(ctx->ctx), LY_EVALID);

            LY_CHECK_RET(parse_restr(ctx, data, kw, type->range));
            type->flags |= LYS_SET_RANGE;
            break;
        case YANG_REQUIRE_INSTANCE:
            LY_CHECK_RET(parse_type_reqinstance(ctx, data, &type->require_instance, &type->flags, &type->exts));
            /* LYS_SET_REQINST checked and set inside parse_type_reqinstance() */
            break;
        case YANG_TYPE:
            LY_ARRAY_NEW_RET(ctx->ctx, type->types, nest_type, LY_EMEM);
            LY_CHECK_RET(parse_type(ctx, data, nest_type));
            type->flags |= LYS_SET_TYPE;
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &type->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "type");
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
static LY_ERR
parse_leaf(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_leaf *leaf;

    /* create structure */
    leaf = calloc(1, sizeof *leaf);
    LY_CHECK_ERR_RET(!leaf, LOGMEM(ctx->ctx), LY_EMEM);
    leaf->nodetype = LYS_LEAF;
    leaf->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)leaf;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)leaf;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, leaf->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &leaf->flags, &leaf->exts));
            break;
        case YANG_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &leaf->dflt, Y_STR_ARG, &leaf->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &leaf->dsc, Y_STR_ARG, &leaf->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &leaf->iffeatures, Y_STR_ARG, &leaf->exts));
            break;
        case YANG_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &leaf->flags, &leaf->exts));
            break;
        case YANG_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &leaf->musts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &leaf->ref, Y_STR_ARG, &leaf->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &leaf->flags, &leaf->exts));
            break;
        case YANG_TYPE:
            LY_CHECK_RET(parse_type(ctx, data, &leaf->type));
            break;
        case YANG_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &leaf->units, Y_STR_ARG, &leaf->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &leaf->when));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &leaf->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "leaf");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!leaf->type.name) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "type", "leaf");
        return LY_EVALID;
    }
    if ((leaf->flags & LYS_MAND_TRUE) && (leaf->dflt)) {
        LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMSCOMB, "mandatory", "default", "leaf");
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
static LY_ERR
parse_maxelements(struct ly_parser_ctx *ctx, const char **data, uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    unsigned long int num;
    enum yang_keyword kw;

    if (*flags & LYS_SET_MAX) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "max-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MAX;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if (!word_len || (word[0] == '0') || ((word[0] != 'u') && !isdigit(word[0]))) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "max-elements");
        free(buf);
        return LY_EVALID;
    }

    if (strncmp(word, "unbounded", word_len)) {
        errno = 0;
        num = strtoul(word, &ptr, 10);
        /* we have not parsed the whole argument */
        if ((size_t)(ptr - word) != word_len) {
            LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "max-elements");
            free(buf);
            return LY_EVALID;
        }
        if ((errno == ERANGE) || (num > UINT32_MAX)) {
            LOGVAL_YANG(ctx, LY_VCODE_OOB, word_len, word, "max-elements");
            free(buf);
            return LY_EVALID;
        }

        *max = num;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MAX, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "max-elements");
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
static LY_ERR
parse_minelements(struct ly_parser_ctx *ctx, const char **data, uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word, *ptr;
    size_t word_len;
    unsigned long int num;
    enum yang_keyword kw;

    if (*flags & LYS_SET_MIN) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "min-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MIN;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if (!word_len || !isdigit(word[0]) || ((word[0] == '0') && (word_len > 1))) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(word, &ptr, 10);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > UINT32_MAX)) {
        LOGVAL_YANG(ctx, LY_VCODE_OOB, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }
    *min = num;
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MIN, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "min-elements");
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
parse_orderedby(struct ly_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*flags & LYS_ORDBY_MASK) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "ordered-by");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 6) && !strncmp(word, "system", word_len)) {
        *flags |= LYS_ORDBY_SYSTEM;
    } else if ((word_len == 4) && !strncmp(word, "user", word_len)) {
        *flags |= LYS_ORDBY_USER;
    } else {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "ordered-by");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_ORDEREDBY, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "ordered-by");
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
static LY_ERR
parse_leaflist(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_leaflist *llist;

    /* create structure */
    llist = calloc(1, sizeof *llist);
    LY_CHECK_ERR_RET(!llist, LOGMEM(ctx->ctx), LY_EMEM);
    llist->nodetype = LYS_LEAFLIST;
    llist->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)llist;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)llist;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, llist->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &llist->flags, &llist->exts));
            break;
        case YANG_DEFAULT:
            YANG_CHECK_STMTVER2_RET(ctx, "default", "leaf-list");
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, &llist->dflts, Y_STR_ARG, &llist->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &llist->dsc, Y_STR_ARG, &llist->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &llist->iffeatures, Y_STR_ARG, &llist->exts));
            break;
        case YANG_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, data, &llist->max, &llist->flags, &llist->exts));
            break;
        case YANG_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, data, &llist->min, &llist->flags, &llist->exts));
            break;
        case YANG_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &llist->musts));
            break;
        case YANG_ORDERED_BY:
            LY_CHECK_RET(parse_orderedby(ctx, data, &llist->flags, &llist->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &llist->ref, Y_STR_ARG, &llist->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &llist->flags, &llist->exts));
            break;
        case YANG_TYPE:
            LY_CHECK_RET(parse_type(ctx, data, &llist->type));
            break;
        case YANG_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &llist->units, Y_STR_ARG, &llist->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &llist->when));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &llist->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "llist");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!llist->type.name) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "type", "leaf-list");
        return LY_EVALID;
    }
    if ((llist->min) && (llist->dflts)) {
        LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMSCOMB, "min-elements", "default", "leaf-list");
        return LY_EVALID;
    }
    if (llist->max && llist->min > llist->max) {
        LOGVAL_YANG(ctx, LYVE_SEMANTICS,
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
parse_refine(struct ly_parser_ctx *ctx, const char **data, struct lysp_refine **refines)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_refine *rf;

    LY_ARRAY_NEW_RET(ctx->ctx, *refines, rf, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, rf->nodeid, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &rf->flags, &rf->exts));
            break;
        case YANG_DEFAULT:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, &rf->dflts, Y_STR_ARG, &rf->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &rf->dsc, Y_STR_ARG, &rf->exts));
            break;
        case YANG_IF_FEATURE:
            YANG_CHECK_STMTVER2_RET(ctx, "if-feature", "refine");
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &rf->iffeatures, Y_STR_ARG, &rf->exts));
            break;
        case YANG_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, data, &rf->max, &rf->flags, &rf->exts));
            break;
        case YANG_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, data, &rf->min, &rf->flags, &rf->exts));
            break;
        case YANG_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &rf->musts));
            break;
        case YANG_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &rf->flags, &rf->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &rf->ref, Y_STR_ARG, &rf->exts));
            break;
        case YANG_PRESENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PRESENCE, 0, &rf->presence, Y_STR_ARG, &rf->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &rf->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "refine");
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
parse_typedef(struct ly_parser_ctx *ctx, struct lysp_node *parent, const char **data, struct lysp_tpdf **typedefs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_tpdf *tpdf;

    LY_ARRAY_NEW_RET(ctx->ctx, *typedefs, tpdf, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, tpdf->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &tpdf->dflt, Y_STR_ARG, &tpdf->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &tpdf->dsc, Y_STR_ARG, &tpdf->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &tpdf->ref, Y_STR_ARG, &tpdf->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &tpdf->flags, &tpdf->exts));
            break;
        case YANG_TYPE:
            LY_CHECK_RET(parse_type(ctx, data, &tpdf->type));
            break;
        case YANG_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &tpdf->units, Y_STR_ARG, &tpdf->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &tpdf->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "typedef");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!tpdf->type.name) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "type", "typedef");
        return LY_EVALID;
    }

    /* store data for collision check */
    if (parent) {
        ly_set_add(&ctx->tpdfs_nodes, parent, 0);
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
parse_inout(struct ly_parser_ctx *ctx, const char **data, enum yang_keyword inout_kw, struct lysp_node *parent, struct lysp_action_inout *inout_p)
{
    LY_ERR ret = LY_SUCCESS;
    char *word;
    size_t word_len;
    enum yang_keyword kw;
    unsigned int u;
    struct lysp_node *child;

    if (inout_p->nodetype) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(inout_kw));
        return LY_EVALID;
    }

    /* initiate structure */
    inout_p->nodetype = LYS_INOUT;
    inout_p->parent = parent;

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", ly_stmt2str(inout_kw));
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)inout_p, &inout_p->data));
            break;
        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)inout_p, data, &inout_p->typedefs));
            break;
        case YANG_MUST:
            YANG_CHECK_STMTVER2_RET(ctx, "must", ly_stmt2str(inout_kw));
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &inout_p->musts));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)inout_p, &inout_p->groupings));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &inout_p->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(inout_kw));
            return LY_EVALID;
        }
    }
    /* finalize parent pointers to the reallocated items */
    LY_ARRAY_FOR(inout_p->groupings, u) {
        LY_LIST_FOR(inout_p->groupings[u].data, child) {
            child->parent = (struct lysp_node*)&inout_p->groupings[u];
        }
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
static LY_ERR
parse_action(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_action **actions)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_action *act;
    struct lysp_node *child;
    unsigned int u;

    LY_ARRAY_NEW_RET(ctx->ctx, *actions, act, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, act->name, word, word_len);
    act->nodetype = LYS_ACTION;
    act->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &act->dsc, Y_STR_ARG, &act->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &act->iffeatures, Y_STR_ARG, &act->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &act->ref, Y_STR_ARG, &act->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &act->flags, &act->exts));
            break;

        case YANG_INPUT:
            LY_CHECK_RET(parse_inout(ctx, data, kw, (struct lysp_node*)act, &act->input));
            break;
        case YANG_OUTPUT:
            LY_CHECK_RET(parse_inout(ctx, data, kw, (struct lysp_node*)act, &act->output));
            break;

        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)act, data, &act->typedefs));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)act, &act->groupings));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &act->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), parent ? "action" : "rpc");
            return LY_EVALID;
        }
    }
    /* finalize parent pointers to the reallocated items */
    LY_ARRAY_FOR(act->groupings, u) {
        LY_LIST_FOR(act->groupings[u].data, child) {
            child->parent = (struct lysp_node*)&act->groupings[u];
        }
    }
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
static LY_ERR
parse_notif(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_notif **notifs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_notif *notif;
    struct lysp_node *child;
    unsigned int u;

    LY_ARRAY_NEW_RET(ctx->ctx, *notifs, notif, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, notif->name, word, word_len);
    notif->nodetype = LYS_NOTIF;
    notif->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &notif->dsc, Y_STR_ARG, &notif->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &notif->iffeatures, Y_STR_ARG, &notif->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &notif->ref, Y_STR_ARG, &notif->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &notif->flags, &notif->exts));
            break;

        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "notification");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)notif, &notif->data));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_case(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)notif, &notif->data));
            break;

        case YANG_MUST:
            YANG_CHECK_STMTVER2_RET(ctx, "must", "notification");
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &notif->musts));
            break;
        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)notif, data, &notif->typedefs));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)notif, &notif->groupings));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &notif->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "notification");
            return LY_EVALID;
        }
    }
    /* finalize parent pointers to the reallocated items */
    LY_ARRAY_FOR(notif->groupings, u) {
        LY_LIST_FOR(notif->groupings[u].data, child) {
            child->parent = (struct lysp_node*)&notif->groupings[u];
        }
    }
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
static LY_ERR
parse_grouping(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_grp **groupings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_grp *grp;
    struct lysp_node *child;
    unsigned int u;

    LY_ARRAY_NEW_RET(ctx->ctx, *groupings, grp, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, grp->name, word, word_len);
    grp->nodetype = LYS_GROUPING;
    grp->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &grp->dsc, Y_STR_ARG, &grp->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &grp->ref, Y_STR_ARG, &grp->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &grp->flags, &grp->exts));
            break;

        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "grouping");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)grp, &grp->data));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)grp, &grp->data));
            break;

        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)grp, data, &grp->typedefs));
            break;
        case YANG_ACTION:
            YANG_CHECK_STMTVER2_RET(ctx, "action", "grouping");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)grp, &grp->actions));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)grp, &grp->groupings));
            break;
        case YANG_NOTIFICATION:
            YANG_CHECK_STMTVER2_RET(ctx, "notification", "grouping");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)grp, &grp->notifs));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &grp->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "grouping");
            return LY_EVALID;
        }
    }
    /* finalize parent pointers to the reallocated items */
    LY_ARRAY_FOR(grp->groupings, u) {
        LY_LIST_FOR(grp->groupings[u].data, child) {
            child->parent = (struct lysp_node*)&grp->groupings[u];
        }
    }
    return ret;
}

/**
 * @brief Parse the refine statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] augments Augments to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
parse_augment(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_augment **augments)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_augment *aug;

    LY_ARRAY_NEW_RET(ctx->ctx, *augments, aug, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, aug->nodeid, word, word_len);
    aug->nodetype = LYS_AUGMENT;
    aug->parent = parent;

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &aug->dsc, Y_STR_ARG, &aug->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &aug->iffeatures, Y_STR_ARG, &aug->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &aug->ref, Y_STR_ARG, &aug->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &aug->flags, &aug->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &aug->when));
            break;

        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "augment");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)aug, &aug->child));
            break;
        case YANG_CASE:
            LY_CHECK_RET(parse_case(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)aug, &aug->child));
            break;

        case YANG_ACTION:
            YANG_CHECK_STMTVER2_RET(ctx, "action", "augment");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)aug, &aug->actions));
            break;
        case YANG_NOTIFICATION:
            YANG_CHECK_STMTVER2_RET(ctx, "notification", "augment");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)aug, &aug->notifs));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &aug->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "augment");
            return LY_EVALID;
        }
    }
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
static LY_ERR
parse_uses(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_uses *uses;

    /* create structure */
    uses = calloc(1, sizeof *uses);
    LY_CHECK_ERR_RET(!uses, LOGMEM(ctx->ctx), LY_EMEM);
    uses->nodetype = LYS_USES;
    uses->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)uses;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)uses;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_PREF_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, uses->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &uses->dsc, Y_STR_ARG, &uses->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &uses->iffeatures, Y_STR_ARG, &uses->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &uses->ref, Y_STR_ARG, &uses->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &uses->flags, &uses->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &uses->when));
            break;

        case YANG_REFINE:
            LY_CHECK_RET(parse_refine(ctx, data, &uses->refines));
            break;
        case YANG_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, data, (struct lysp_node*)uses, &uses->augments));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &uses->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "uses");
            return LY_EVALID;
        }
    }
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
static LY_ERR
parse_case(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_case *cas;

    /* create structure */
    cas = calloc(1, sizeof *cas);
    LY_CHECK_ERR_RET(!cas, LOGMEM(ctx->ctx), LY_EMEM);
    cas->nodetype = LYS_CASE;
    cas->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)cas;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)cas;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, cas->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &cas->dsc, Y_STR_ARG, &cas->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &cas->iffeatures, Y_STR_ARG, &cas->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &cas->ref, Y_STR_ARG, &cas->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &cas->flags, &cas->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &cas->when));
            break;

        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "case");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)cas, &cas->child));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)cas, &cas->child));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &cas->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "case");
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
static LY_ERR
parse_choice(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_choice *choice;

    /* create structure */
    choice = calloc(1, sizeof *choice);
    LY_CHECK_ERR_RET(!choice, LOGMEM(ctx->ctx), LY_EMEM);
    choice->nodetype = LYS_CHOICE;
    choice->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)choice;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)choice;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, choice->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &choice->flags, &choice->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &choice->dsc, Y_STR_ARG, &choice->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &choice->iffeatures, Y_STR_ARG, &choice->exts));
            break;
        case YANG_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, data, &choice->flags, &choice->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &choice->ref, Y_STR_ARG, &choice->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &choice->flags, &choice->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &choice->when));
            break;
        case YANG_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &choice->dflt, Y_PREF_IDENTIF_ARG, &choice->exts));
            break;

        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "choice");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)choice, &choice->child));
            break;
        case YANG_CASE:
            LY_CHECK_RET(parse_case(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case YANG_CHOICE:
            YANG_CHECK_STMTVER2_RET(ctx, "choice", "choice");
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)choice, &choice->child));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &choice->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "choice");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    if ((choice->flags & LYS_MAND_TRUE) && choice->dflt) {
        LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMSCOMB, "mandatory", "default", "choice");
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
static LY_ERR
parse_container(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_container *cont;
    unsigned int u;

    /* create structure */
    cont = calloc(1, sizeof *cont);
    LY_CHECK_ERR_RET(!cont, LOGMEM(ctx->ctx), LY_EMEM);
    cont->nodetype = LYS_CONTAINER;
    cont->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)cont;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)cont;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, cont->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &cont->flags, &cont->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &cont->dsc, Y_STR_ARG, &cont->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &cont->iffeatures, Y_STR_ARG, &cont->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &cont->ref, Y_STR_ARG, &cont->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &cont->flags, &cont->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &cont->when));
            break;
        case YANG_PRESENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PRESENCE, 0, &cont->presence, Y_STR_ARG, &cont->exts));
            break;

        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "container");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)cont, &cont->child));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)cont, &cont->child));
            break;

        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)cont, data, &cont->typedefs));
            break;
        case YANG_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &cont->musts));
            break;
        case YANG_ACTION:
            YANG_CHECK_STMTVER2_RET(ctx, "action", "container");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)cont, &cont->actions));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)cont, &cont->groupings));
            break;
        case YANG_NOTIFICATION:
            YANG_CHECK_STMTVER2_RET(ctx, "notification", "container");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)cont, &cont->notifs));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &cont->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "container");
            return LY_EVALID;
        }
    }
    /* finalize parent pointers to the reallocated items */
    LY_ARRAY_FOR(cont->groupings, u) {
        LY_LIST_FOR(cont->groupings[u].data, iter) {
            iter->parent = (struct lysp_node*)&cont->groupings[u];
        }
    }
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
static LY_ERR
parse_list(struct ly_parser_ctx *ctx, const char **data, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_list *list;
    unsigned int u;

    /* create structure */
    list = calloc(1, sizeof *list);
    LY_CHECK_ERR_RET(!list, LOGMEM(ctx->ctx), LY_EMEM);
    list->nodetype = LYS_LIST;
    list->parent = parent;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)list;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)list;
    }

    /* get name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, list->name, word, word_len);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_CONFIG:
            LY_CHECK_RET(parse_config(ctx, data, &list->flags, &list->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &list->dsc, Y_STR_ARG, &list->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &list->iffeatures, Y_STR_ARG, &list->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &list->ref, Y_STR_ARG, &list->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &list->flags, &list->exts));
            break;
        case YANG_WHEN:
            LY_CHECK_RET(parse_when(ctx, data, &list->when));
            break;
        case YANG_KEY:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_KEY, 0, &list->key, Y_STR_ARG, &list->exts));
            break;
        case YANG_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, data, &list->max, &list->flags, &list->exts));
            break;
        case YANG_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, data, &list->min, &list->flags, &list->exts));
            break;
        case YANG_ORDERED_BY:
            LY_CHECK_RET(parse_orderedby(ctx, data, &list->flags, &list->exts));
            break;
        case YANG_UNIQUE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_UNIQUE, &list->uniques, Y_STR_ARG, &list->exts));
            break;

        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "list");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, (struct lysp_node*)list, &list->child));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, (struct lysp_node*)list, &list->child));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, (struct lysp_node*)list, &list->child));
            break;

        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node*)list, data, &list->typedefs));
            break;
        case YANG_MUST:
            LY_CHECK_RET(parse_restrs(ctx, data, kw, &list->musts));
            break;
        case YANG_ACTION:
            YANG_CHECK_STMTVER2_RET(ctx, "action", "list");
            LY_CHECK_RET(parse_action(ctx, data, (struct lysp_node*)list, &list->actions));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, (struct lysp_node*)list, &list->groupings));
            break;
        case YANG_NOTIFICATION:
            YANG_CHECK_STMTVER2_RET(ctx, "notification", "list");
            LY_CHECK_RET(parse_notif(ctx, data, (struct lysp_node*)list, &list->notifs));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &list->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "list");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
    /* finalize parent pointers to the reallocated items */
    LY_ARRAY_FOR(list->groupings, u) {
        LY_LIST_FOR(list->groupings[u].data, iter) {
            iter->parent = (struct lysp_node*)&list->groupings[u];
        }
    }
checks:
    if (list->max && list->min > list->max) {
        LOGVAL_YANG(ctx, LYVE_SEMANTICS,
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
parse_yinelement(struct ly_parser_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*flags & LYS_YINELEM_MASK) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "yin-element");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_YINELEM_TRUE;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "yin-element");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_YINELEM, 0, exts));
            LY_CHECK_RET(ret);            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "yin-element");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the yin-element statement.
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
parse_argument(struct ly_parser_ctx *ctx, const char **data, const char **argument, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;

    if (*argument) {
        LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, "argument");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, *argument, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_YIN_ELEMENT:
            LY_CHECK_RET(parse_yinelement(ctx, data, flags, exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_ARGUMENT, 0, exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "argument");
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
parse_extension(struct ly_parser_ctx *ctx, const char **data, struct lysp_ext **extensions)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_ext *ex;

    LY_ARRAY_NEW_RET(ctx->ctx, *extensions, ex, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, ex->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &ex->dsc, Y_STR_ARG, &ex->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &ex->ref, Y_STR_ARG, &ex->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &ex->flags, &ex->exts));
            break;
        case YANG_ARGUMENT:
            LY_CHECK_RET(parse_argument(ctx, data, &ex->argument, &ex->flags, &ex->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &ex->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "extension");
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
static LY_ERR
parse_deviate(struct ly_parser_ctx *ctx, const char **data, struct lysp_deviate **deviates)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len, dev_mod;
    enum yang_keyword kw;
    struct lysp_deviate *iter, *d;
    struct lysp_deviate_add *d_add = NULL;
    struct lysp_deviate_rpl *d_rpl = NULL;
    struct lysp_deviate_del *d_del = NULL;
    const char **d_units, ***d_uniques, ***d_dflts;
    struct lysp_restr **d_musts;
    uint16_t *d_flags;
    uint32_t *d_min, *d_max;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));

    if ((word_len == 13) && !strncmp(word, "not-supported", word_len)) {
        dev_mod = LYS_DEV_NOT_SUPPORTED;
    } else if ((word_len == 3) && !strncmp(word, "add", word_len)) {
        dev_mod = LYS_DEV_ADD;
    } else if ((word_len == 7) && !strncmp(word, "replace", word_len)) {
        dev_mod = LYS_DEV_REPLACE;
    } else if ((word_len == 6) && !strncmp(word, "delete", word_len)) {
        dev_mod = LYS_DEV_DELETE;
    } else {
        LOGVAL_YANG(ctx, LY_VCODE_INVAL, word_len, word, "deviate");
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
        d_flags = &d_del->flags;
        break;
    default:
        assert(0);
        LOGINT_RET(ctx->ctx);
    }
    d->mod = dev_mod;

    /* insert into siblings */
    if (!*deviates) {
        *deviates = d;
    } else {
        for (iter = *deviates; iter->next; iter = iter->next);
        iter->next = d;
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_CONFIG:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_config(ctx, data, d_flags, &d->exts));
                break;
            }
            break;
        case YANG_DEFAULT:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            case LYS_DEV_REPLACE:
                LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &d_rpl->dflt, Y_STR_ARG, &d->exts));
                break;
            default:
                LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, d_dflts, Y_STR_ARG, &d->exts));
                break;
            }
            break;
        case YANG_MANDATORY:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_mandatory(ctx, data, d_flags, &d->exts));
                break;
            }
            break;
        case YANG_MAX_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_maxelements(ctx, data, d_max, d_flags, &d->exts));
                break;
            }
            break;
        case YANG_MIN_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_minelements(ctx, data, d_min, d_flags, &d->exts));
                break;
            }
            break;
        case YANG_MUST:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_REPLACE:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_restrs(ctx, data, kw, d_musts));
                break;
            }
            break;
        case YANG_TYPE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_ADD:
            case LYS_DEV_DELETE:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                if (d_rpl->type) {
                    LOGVAL_YANG(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                    return LY_EVALID;
                }
                d_rpl->type = calloc(1, sizeof *d_rpl->type);
                LY_CHECK_ERR_RET(!d_rpl->type, LOGMEM(ctx->ctx), LY_EMEM);
                LY_CHECK_RET(parse_type(ctx, data, d_rpl->type));
                break;
            }
            break;
        case YANG_UNIQUE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_REPLACE:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_UNIQUE, d_uniques, Y_STR_ARG, &d->exts));
                break;
            }
            break;
        case YANG_UNITS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL_YANG(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), ly_stmt2str(kw));
                return LY_EVALID;
            default:
                LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, d_units, Y_STR_ARG, &d->exts));
                break;
            }
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &d->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "deviate");
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
static LY_ERR
parse_deviation(struct ly_parser_ctx *ctx, const char **data, struct lysp_deviation **deviations)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_deviation *dev;

    LY_ARRAY_NEW_RET(ctx->ctx, *deviations, dev, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_STR_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, dev->nodeid, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &dev->dsc, Y_STR_ARG, &dev->exts));
            break;
        case YANG_DEVIATE:
            LY_CHECK_RET(parse_deviate(ctx, data, &dev->deviates));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &dev->ref, Y_STR_ARG, &dev->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &dev->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "deviation");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);
checks:
    /* mandatory substatements */
    if (!dev->deviates) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "deviate", "deviation");
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
static LY_ERR
parse_feature(struct ly_parser_ctx *ctx, const char **data, struct lysp_feature **features)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_feature *feat;

    LY_ARRAY_NEW_RET(ctx->ctx, *features, feat, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, feat->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &feat->dsc, Y_STR_ARG, &feat->exts));
            break;
        case YANG_IF_FEATURE:
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &feat->iffeatures, Y_STR_ARG, &feat->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &feat->ref, Y_STR_ARG, &feat->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &feat->flags, &feat->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &feat->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "feature");
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
static LY_ERR
parse_identity(struct ly_parser_ctx *ctx, const char **data, struct lysp_ident **identities)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_ident *ident;

    LY_ARRAY_NEW_RET(ctx->ctx, *identities, ident, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, ident->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret,) {
        switch (kw) {
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &ident->dsc, Y_STR_ARG, &ident->exts));
            break;
        case YANG_IF_FEATURE:
            YANG_CHECK_STMTVER2_RET(ctx, "if-feature", "identity");
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &ident->iffeatures, Y_STR_ARG, &ident->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &ident->ref, Y_STR_ARG, &ident->exts));
            break;
        case YANG_STATUS:
            LY_CHECK_RET(parse_status(ctx, data, &ident->flags, &ident->exts));
            break;
        case YANG_BASE:
            if (ident->bases && ctx->mod_version < 2) {
                LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Identity can be derived from multiple base identities only in YANG 1.1 modules");
                return LY_EVALID;
            }
            LY_CHECK_RET(parse_text_fields(ctx, data, LYEXT_SUBSTMT_BASE, &ident->bases, Y_PREF_IDENTIF_ARG, &ident->exts));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &ident->exts));
            break;
        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "identity");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Finalize some of the (sub)module structure after parsing.
 *
 * Update parent pointers in the nodes inside grouping/RPC/Notification, which could be reallocated.
 *
 * @param[in] mod Parsed module to be updated.
 * @return LY_ERR value (currently only LY_SUCCESS, but it can change in future).
 */
static LY_ERR
parse_sub_module_finalize(struct lysp_module *mod)
{
    unsigned int u;
    struct lysp_node *child;

    /* finalize parent pointers to the reallocated items */
    LY_ARRAY_FOR(mod->groupings, u) {
        LY_LIST_FOR(mod->groupings[u].data, child) {
            child->parent = (struct lysp_node*)&mod->groupings[u];
        }
    }

    /* TODO the same finalization for rpcs and notifications, do also in the relevant nodes */

    return LY_SUCCESS;
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
static LY_ERR
parse_module(struct ly_parser_ctx *ctx, const char **data, struct lysp_module *mod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw, prev_kw = 0;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;
    struct lysp_submodule *dup;

    /* (sub)module name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, mod->mod->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {

#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {LOGVAL_YANG(ctx, LY_VCODE_INORD, ly_stmt2str(kw), ly_stmt2str(prev_kw)); return LY_EVALID;}mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case YANG_NAMESPACE:
        case YANG_PREFIX:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case YANG_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case YANG_INCLUDE:
        case YANG_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case YANG_ORGANIZATION:
        case YANG_CONTACT:
        case YANG_DESCRIPTION:
        case YANG_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case YANG_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case YANG_ANYDATA:
        case YANG_ANYXML:
        case YANG_AUGMENT:
        case YANG_CHOICE:
        case YANG_CONTAINER:
        case YANG_DEVIATION:
        case YANG_EXTENSION:
        case YANG_FEATURE:
        case YANG_GROUPING:
        case YANG_IDENTITY:
        case YANG_LEAF:
        case YANG_LEAF_LIST:
        case YANG_LIST:
        case YANG_NOTIFICATION:
        case YANG_RPC:
        case YANG_TYPEDEF:
        case YANG_USES:
        case YANG_CUSTOM:
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
        case YANG_YANG_VERSION:
            LY_CHECK_RET(parse_yangversion(ctx, data, &mod->mod->version, &mod->exts));
            ctx->mod_version = mod->mod->version;
            break;
        case YANG_NAMESPACE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_NAMESPACE, 0, &mod->mod->ns, Y_STR_ARG, &mod->exts));
            break;
        case YANG_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, &mod->mod->prefix, Y_IDENTIF_ARG, &mod->exts));
            break;

        /* linkage */
        case YANG_INCLUDE:
            LY_CHECK_RET(parse_include(ctx, mod->mod->name, data, &mod->includes));
            break;
        case YANG_IMPORT:
            LY_CHECK_RET(parse_import(ctx, mod->mod->prefix, data, &mod->imports));
            break;

        /* meta */
        case YANG_ORGANIZATION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ORGANIZATION, 0, &mod->mod->org, Y_STR_ARG, &mod->exts));
            break;
        case YANG_CONTACT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_CONTACT, 0, &mod->mod->contact, Y_STR_ARG, &mod->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &mod->mod->dsc, Y_STR_ARG, &mod->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &mod->mod->ref, Y_STR_ARG, &mod->exts));
            break;

        /* revision */
        case YANG_REVISION:
            LY_CHECK_RET(parse_revision(ctx, data, &mod->revs));
            break;

        /* body */
        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "module");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, NULL, &mod->data));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, NULL, &mod->data));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, NULL, &mod->data));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, NULL, &mod->data));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, NULL, &mod->data));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, NULL, &mod->data));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, NULL, &mod->data));
            break;

        case YANG_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, data, NULL, &mod->augments));
            break;
        case YANG_DEVIATION:
            LY_CHECK_RET(parse_deviation(ctx, data, &mod->deviations));
            break;
        case YANG_EXTENSION:
            LY_CHECK_RET(parse_extension(ctx, data, &mod->extensions));
            break;
        case YANG_FEATURE:
            LY_CHECK_RET(parse_feature(ctx, data, &mod->features));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, NULL, &mod->groupings));
            break;
        case YANG_IDENTITY:
            LY_CHECK_RET(parse_identity(ctx, data, &mod->identities));
            break;
        case YANG_NOTIFICATION:
            LY_CHECK_RET(parse_notif(ctx, data, NULL, &mod->notifs));
            break;
        case YANG_RPC:
            LY_CHECK_RET(parse_action(ctx, data, NULL, &mod->rpcs));
            break;
        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, NULL, data, &mod->typedefs));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &mod->exts));
            break;

        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "module");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);

checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(parse_sub_module_finalize(mod));

    /* mandatory substatements */
    if (!mod->mod->ns) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "namespace", "module");
        return LY_EVALID;
    } else if (!mod->mod->prefix) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "prefix", "module");
        return LY_EVALID;
    }

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule(ctx->ctx, NULL, mod->mod->name, NULL);
    if (dup) {
        LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Name collision between module and submodule of name \"%s\".", mod->mod->name);
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
static LY_ERR
parse_submodule(struct ly_parser_ctx *ctx, const char **data, struct lysp_submodule *submod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum yang_keyword kw, prev_kw = 0;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;
    struct lysp_submodule *dup;

    /* submodule name */
    LY_CHECK_RET(get_argument(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len));
    INSERT_WORD(ctx, buf, submod->name, word, word_len);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret, goto checks) {

#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {LOGVAL_YANG(ctx, LY_VCODE_INORD, ly_stmt2str(kw), ly_stmt2str(prev_kw)); return LY_EVALID;}mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case YANG_BELONGS_TO:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case YANG_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case YANG_INCLUDE:
        case YANG_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case YANG_ORGANIZATION:
        case YANG_CONTACT:
        case YANG_DESCRIPTION:
        case YANG_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case YANG_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case YANG_ANYDATA:
        case YANG_ANYXML:
        case YANG_AUGMENT:
        case YANG_CHOICE:
        case YANG_CONTAINER:
        case YANG_DEVIATION:
        case YANG_EXTENSION:
        case YANG_FEATURE:
        case YANG_GROUPING:
        case YANG_IDENTITY:
        case YANG_LEAF:
        case YANG_LEAF_LIST:
        case YANG_LIST:
        case YANG_NOTIFICATION:
        case YANG_RPC:
        case YANG_TYPEDEF:
        case YANG_USES:
        case YANG_CUSTOM:
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
        case YANG_YANG_VERSION:
            LY_CHECK_RET(parse_yangversion(ctx, data, &submod->version, &submod->exts));
            ctx->mod_version = submod->version;
            break;
        case YANG_BELONGS_TO:
            LY_CHECK_RET(parse_belongsto(ctx, data, &submod->belongsto, &submod->prefix, &submod->exts));
            break;

        /* linkage */
        case YANG_INCLUDE:
            LY_CHECK_RET(parse_include(ctx, submod->name, data, &submod->includes));
            break;
        case YANG_IMPORT:
            LY_CHECK_RET(parse_import(ctx, submod->prefix, data, &submod->imports));
            break;

        /* meta */
        case YANG_ORGANIZATION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_ORGANIZATION, 0, &submod->org, Y_STR_ARG, &submod->exts));
            break;
        case YANG_CONTACT:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_CONTACT, 0, &submod->contact, Y_STR_ARG, &submod->exts));
            break;
        case YANG_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &submod->dsc, Y_STR_ARG, &submod->exts));
            break;
        case YANG_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &submod->ref, Y_STR_ARG, &submod->exts));
            break;

        /* revision */
        case YANG_REVISION:
            LY_CHECK_RET(parse_revision(ctx, data, &submod->revs));
            break;

        /* body */
        case YANG_ANYDATA:
            YANG_CHECK_STMTVER2_RET(ctx, "anydata", "submodule");
            /* fall through */
        case YANG_ANYXML:
            LY_CHECK_RET(parse_any(ctx, data, kw, NULL, &submod->data));
            break;
        case YANG_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, data, NULL, &submod->data));
            break;
        case YANG_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, data, NULL, &submod->data));
            break;
        case YANG_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, data, NULL, &submod->data));
            break;
        case YANG_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, data, NULL, &submod->data));
            break;
        case YANG_LIST:
            LY_CHECK_RET(parse_list(ctx, data, NULL, &submod->data));
            break;
        case YANG_USES:
            LY_CHECK_RET(parse_uses(ctx, data, NULL, &submod->data));
            break;

        case YANG_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, data, NULL, &submod->augments));
            break;
        case YANG_DEVIATION:
            LY_CHECK_RET(parse_deviation(ctx, data, &submod->deviations));
            break;
        case YANG_EXTENSION:
            LY_CHECK_RET(parse_extension(ctx, data, &submod->extensions));
            break;
        case YANG_FEATURE:
            LY_CHECK_RET(parse_feature(ctx, data, &submod->features));
            break;
        case YANG_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, data, NULL, &submod->groupings));
            break;
        case YANG_IDENTITY:
            LY_CHECK_RET(parse_identity(ctx, data, &submod->identities));
            break;
        case YANG_NOTIFICATION:
            LY_CHECK_RET(parse_notif(ctx, data, NULL, &submod->notifs));
            break;
        case YANG_RPC:
            LY_CHECK_RET(parse_action(ctx, data, NULL, &submod->rpcs));
            break;
        case YANG_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, NULL, data, &submod->typedefs));
            break;
        case YANG_CUSTOM:
            LY_CHECK_RET(parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &submod->exts));
            break;

        default:
            LOGVAL_YANG(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "submodule");
            return LY_EVALID;
        }
    }
    LY_CHECK_RET(ret);

checks:
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(parse_sub_module_finalize((struct lysp_module*)submod));

    /* mandatory substatements */
    if (!submod->belongsto) {
        LOGVAL_YANG(ctx, LY_VCODE_MISSTMT, "belongs-to", "submodule");
        return LY_EVALID;
    }

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule(ctx->ctx, NULL, submod->name, NULL);
    if (dup && strcmp(dup->belongsto, submod->belongsto)) {
        LOGVAL_YANG(ctx, LYVE_SYNTAX_YANG, "Name collision between submodules of name \"%s\".", dup->name);
        return LY_EVALID;
    }

    return ret;
}

LY_ERR
yang_parse_submodule(struct ly_parser_ctx *context, const char *data, struct lysp_submodule **submod)
{
    LY_ERR ret = LY_SUCCESS;
    char *word, *buf;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_submodule *mod_p = NULL;

    /* "module"/"submodule" */
    ret = get_keyword(context, &data, &kw, &word, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (kw == YANG_MODULE) {
        LOGERR(context->ctx, LY_EDENIED, "Input data contains module in situation when a submodule is expected.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_SUBMODULE) {
        LOGVAL_YANG(context, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".",
               ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(context->ctx), cleanup);
    mod_p->parsing = 1;

    /* substatements */
    ret = parse_submodule(context, &data, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* read some trailing spaces or new lines */
    ret = get_argument(context, &data, Y_MAYBE_STR_ARG, &word, &buf, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (word) {
        LOGVAL_YANG(context, LYVE_SYNTAX, "Invalid character sequence \"%.*s\", expected end-of-file.",
               word_len, word);
        free(buf);
        ret = LY_EVALID;
        goto cleanup;
    }
    assert(!buf);

    mod_p->parsing = 0;
    *submod = mod_p;

cleanup:
    if (ret) {
        lysp_submodule_free(context->ctx, mod_p);
    }

    return ret;
}

LY_ERR
yang_parse_module(struct ly_parser_ctx *context, const char *data, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    char *word, *buf;
    size_t word_len;
    enum yang_keyword kw;
    struct lysp_module *mod_p = NULL;

    /* "module"/"submodule" */
    ret = get_keyword(context, &data, &kw, &word, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (kw == YANG_SUBMODULE) {
        LOGERR(context->ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_MODULE) {
        LOGVAL_YANG(context, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".",
               ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(context->ctx), cleanup);
    mod_p->mod = mod;
    mod_p->parsing = 1;

    /* substatements */
    ret = parse_module(context, &data, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* read some trailing spaces or new lines */
    ret = get_argument(context, &data, Y_MAYBE_STR_ARG, &word, &buf, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (word) {
        LOGVAL_YANG(context, LYVE_SYNTAX, "Invalid character sequence \"%.*s\", expected end-of-file.",
               word_len, word);
        free(buf);
        ret = LY_EVALID;
        goto cleanup;
    }
    assert(!buf);

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret) {
        lysp_module_free(mod_p);
    }

    return ret;
}
