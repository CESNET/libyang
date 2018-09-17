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

#define YANG_ERR_GOTO(ERR, GO) if (ERR) goto GO;

#define YANG_ERR_RET(ERR) if (ERR) return ERR;

#define YANG_READ_SUBSTMT_FOR(CTX, DATA, KW, WORD, WORD_LEN, ERR) \
    ERR = get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN); \
    YANG_ERR_RET(ERR); \
    \
    if (KW == YANG_SEMICOLON) { \
        return ERR; \
    } \
    if (KW != YANG_LEFT_BRACE) { \
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", expected \";\" or \"{\".", kw2str(KW)); \
        return LY_EVALID; \
    } \
    for (ERR = get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN); \
            !ERR && (KW != YANG_RIGHT_BRACE); \
            ERR = get_keyword(CTX, DATA, &KW, &WORD, &WORD_LEN))

enum yang_module_stmt {
    Y_MOD_MODULE_HEADER,
    Y_MOD_LINKAGE,
    Y_MOD_META,
    Y_MOD_REVISION,
    Y_MOD_BODY
};

enum yang_arg {
    Y_IDENTIF_ARG,
    Y_PREF_IDENTIF_ARG,
    Y_STR_ARG,
    Y_MAYBE_STR_ARG
};

enum yang_keyword {
    YANG_NONE = 0,
    YANG_ACTION,
    YANG_ANYDATA,
    YANG_ANYXML,
    YANG_ARGUMENT,
    YANG_AUGMENT,
    YANG_BASE,
    YANG_BELONGS_TO,
    YANG_BIT,
    YANG_CASE,
    YANG_CHOICE,
    YANG_CONFIG,
    YANG_CONTACT,
    YANG_CONTAINER,
    YANG_DEFAULT,
    YANG_DESCRIPTION,
    YANG_DEVIATE,
    YANG_DEVIATION,
    YANG_ENUM,
    YANG_ERROR_APP_TAG,
    YANG_ERROR_MESSAGE,
    YANG_EXTENSION,
    YANG_FEATURE,
    YANG_FRACTION_DIGITS,
    YANG_GROUPING,
    YANG_IDENTITY,
    YANG_IF_FEATURE,
    YANG_IMPORT,
    YANG_INCLUDE,
    YANG_INPUT,
    YANG_KEY,
    YANG_LEAF,
    YANG_LEAF_LIST,
    YANG_LENGTH,
    YANG_LIST,
    YANG_MANDATORY,
    YANG_MAX_ELEMENTS,
    YANG_MIN_ELEMENTS,
    YANG_MODIFIER,
    YANG_MODULE,
    YANG_MUST,
    YANG_NAMESPACE,
    YANG_NOTIFICATION,
    YANG_ORDERED_BY,
    YANG_ORGANIZATION,
    YANG_OUTPUT,
    YANG_PATH,
    YANG_PATTERN,
    YANG_POSITION,
    YANG_PREFIX,
    YANG_PRESENCE,
    YANG_RANGE,
    YANG_REFERENCE,
    YANG_REFINE,
    YANG_REQUIRE_INSTANCE,
    YANG_REVISION,
    YANG_REVISION_DATE,
    YANG_RPC,
    YANG_STATUS,
    YANG_SUBMODULE,
    YANG_TYPE,
    YANG_TYPEDEF,
    YANG_UNIQUE,
    YANG_UNITS,
    YANG_USES,
    YANG_VALUE,
    YANG_WHEN,
    YANG_YANG_VERSION,
    YANG_YIN_ELEMENT,

    YANG_SEMICOLON,
    YANG_LEFT_BRACE,
    YANG_RIGHT_BRACE,
    YANG_CUSTOM
};

static LY_ERR parse_container(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings);
static LY_ERR parse_uses(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings);
static LY_ERR parse_choice(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings);
static LY_ERR parse_case(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings);
static LY_ERR parse_list(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings);
static LY_ERR parse_grouping(struct ly_ctx *ctx, const char **data, struct lysp_grp **groupings);

static const char *
kw2str(enum yang_keyword kw)
{
    switch (kw) {
    case YANG_ACTION:
        return "action";
    case YANG_ANYDATA:
        return "anydata";
    case YANG_ANYXML:
        return "anyxml";
    case YANG_ARGUMENT:
        return "argument";
    case YANG_AUGMENT:
        return "augment";
    case YANG_BASE:
        return "base";
    case YANG_BELONGS_TO:
        return "belongs-to";
    case YANG_BIT:
        return "bit";
    case YANG_CASE:
        return "case";
    case YANG_CHOICE:
        return "choice";
    case YANG_CONFIG:
        return "config";
    case YANG_CONTACT:
        return "contact";
    case YANG_CONTAINER:
        return "container";
    case YANG_CUSTOM:
        return "<extension-instance>";
    case YANG_DEFAULT:
        return "default";
    case YANG_DESCRIPTION:
        return "description";
    case YANG_DEVIATE:
        return "deviate";
    case YANG_DEVIATION:
        return "deviation";
    case YANG_ENUM:
        return "enum";
    case YANG_ERROR_APP_TAG:
        return "error-app-tag";
    case YANG_ERROR_MESSAGE:
        return "error-message";
    case YANG_EXTENSION:
        return "extension";
    case YANG_FEATURE:
        return "feature";
    case YANG_FRACTION_DIGITS:
        return "fraction-digits";
    case YANG_GROUPING:
        return "grouping";
    case YANG_IDENTITY:
        return "identitiy";
    case YANG_IF_FEATURE:
        return "if-feature";
    case YANG_IMPORT:
        return "import";
    case YANG_INCLUDE:
        return "include";
    case YANG_INPUT:
        return "input";
    case YANG_KEY:
        return "key";
    case YANG_LEAF:
        return "leaf";
    case YANG_LEAF_LIST:
        return "leaf-list";
    case YANG_LENGTH:
        return "length";
    case YANG_LIST:
        return "list";
    case YANG_MANDATORY:
        return "mandatory";
    case YANG_MAX_ELEMENTS:
        return "max-elements";
    case YANG_MIN_ELEMENTS:
        return "min-elements";
    case YANG_MODIFIER:
        return "modifier";
    case YANG_MODULE:
        return "module";
    case YANG_MUST:
        return "must";
    case YANG_NAMESPACE:
        return "namespace";
    case YANG_NOTIFICATION:
        return "notification";
    case YANG_ORDERED_BY:
        return "ordered-by";
    case YANG_ORGANIZATION:
        return "organization";
    case YANG_OUTPUT:
        return "output";
    case YANG_PATH:
        return "path";
    case YANG_PATTERN:
        return "pattern";
    case YANG_POSITION:
        return "position";
    case YANG_PREFIX:
        return "prefix";
    case YANG_PRESENCE:
        return "presence";
    case YANG_RANGE:
        return "range";
    case YANG_REFERENCE:
        return "reference";
    case YANG_REFINE:
        return "refine";
    case YANG_REQUIRE_INSTANCE:
        return "require-instance";
    case YANG_REVISION:
        return "revision";
    case YANG_REVISION_DATE:
        return "revision-date";
    case YANG_RPC:
        return "rpc";
    case YANG_STATUS:
        return "status";
    case YANG_SUBMODULE:
        return "submodule";
    case YANG_TYPE:
        return "type";
    case YANG_TYPEDEF:
        return "typedef";
    case YANG_UNIQUE:
        return "unique";
    case YANG_UNITS:
        return "units";
    case YANG_USES:
        return "uses";
    case YANG_VALUE:
        return "value";
    case YANG_WHEN:
        return "when";
    case YANG_YANG_VERSION:
        return "yang-version";
    case YANG_YIN_ELEMENT:
        return "yin-element";
    case YANG_SEMICOLON:
        return ";";
    case YANG_LEFT_BRACE:
        return "{";
    case YANG_RIGHT_BRACE:
        return "}";
    default:
        return "";
    }
}

static const char *
substmt2str(LYEXT_SUBSTMT substmt)
{
    switch (substmt) {
    case LYEXT_SUBSTMT_ARGUMENT:
        return "argument";
    case LYEXT_SUBSTMT_BASE:
        return "base";
    case LYEXT_SUBSTMT_BELONGSTO:
        return "belongs-to";
    case LYEXT_SUBSTMT_CONTACT:
        return "contact";
    case LYEXT_SUBSTMT_DEFAULT:
        return "default";
    case LYEXT_SUBSTMT_DESCRIPTION:
        return "description";
    case LYEXT_SUBSTMT_ERRTAG:
        return "error-app-tag";
    case LYEXT_SUBSTMT_ERRMSG:
        return "error-message";
    case LYEXT_SUBSTMT_KEY:
        return "key";
    case LYEXT_SUBSTMT_NAMESPACE:
        return "namespace";
    case LYEXT_SUBSTMT_ORGANIZATION:
        return "organization";
    case LYEXT_SUBSTMT_PATH:
        return "path";
    case LYEXT_SUBSTMT_PREFIX:
        return "prefix";
    case LYEXT_SUBSTMT_PRESENCE:
        return "presence";
    case LYEXT_SUBSTMT_REFERENCE:
        return "reference";
    case LYEXT_SUBSTMT_REVISIONDATE:
        return "revision-date";
    case LYEXT_SUBSTMT_UNITS:
        return "units";
    case LYEXT_SUBSTMT_VALUE:
        return "value";
    case LYEXT_SUBSTMT_VERSION:
        return "yang-version";
    case LYEXT_SUBSTMT_MODIFIER:
        return "modifier";
    case LYEXT_SUBSTMT_REQINSTANCE:
        return "require-instance";
    case LYEXT_SUBSTMT_YINELEM:
        return "yin-element";
    case LYEXT_SUBSTMT_CONFIG:
        return "config";
    case LYEXT_SUBSTMT_MANDATORY:
        return "mandatory";
    case LYEXT_SUBSTMT_ORDEREDBY:
        return "ordered-by";
    case LYEXT_SUBSTMT_STATUS:
        return "status";
    case LYEXT_SUBSTMT_FRACDIGITS:
        return "fraction-digits";
    case LYEXT_SUBSTMT_MAX:
        return "max-elements";
    case LYEXT_SUBSTMT_MIN:
        return "min-elements";
    case LYEXT_SUBSTMT_POSITION:
        return "position";
    case LYEXT_SUBSTMT_UNIQUE:
        return "unique";
    case LYEXT_SUBSTMT_IFFEATURE:
        return "if-feature";
    default:
        return "";
    }
}

static const char *
nodetype2str(uint16_t nodetype)
{
    switch (nodetype) {
    case LYS_CONTAINER:
        return "container";
    case LYS_CHOICE:
        return "choice";
    case LYS_LEAF:
        return "leaf";
    case LYS_LEAFLIST:
        return "leaf-list";
    case LYS_LIST:
        return "list";
    case LYS_ANYXML:
        return "anyxml";
    case LYS_CASE:
        return "case";
    case LYS_USES:
        return "uses";
    case LYS_ANYDATA:
        return "anydata";
    default:
        return "";
    }
}

static const char *
devmod2str(uint8_t devmod)
{
    switch (devmod) {
    case LYS_DEV_NOT_SUPPORTED:
        return "not-supported";
    case LYS_DEV_ADD:
        return "add";
    case LYS_DEV_DELETE:
        return "delete";
    case LYS_DEV_REPLACE:
        return "replace";
    default:
        return "";
    }
}

static LY_ERR
buf_add_char(struct ly_ctx *ctx, char c, char **buf, int *buf_len, int buf_used)
{
    if (*buf_len == buf_used) {
        *buf_len += 16;
        *buf = ly_realloc(*buf, *buf_len);
        LY_CHECK_ERR_RET(!*buf, LOGMEM(ctx), LY_EMEM);
    }

    (*buf)[buf_used] = c;
    return LY_SUCCESS;
}

static LY_ERR
buf_store_char(struct ly_ctx *ctx, const char *c, char **word_p, int *word_len, char **word_b, int *buf_len, int need_buf)
{
    if (word_b && *word_b) {
        /* add another character into buffer */
        if (buf_add_char(ctx, *c, word_b, buf_len, *word_len)) {
            return LY_EMEM;
        }

        /* in case of realloc */
        *word_p = *word_b;
    } else if (need_buf) {
        /* first time we need a buffer, copy everything read up to now */
        if (*word_len) {
            *word_b = malloc(*word_len);
            LY_CHECK_ERR_RET(!*word_b, LOGMEM(ctx), LY_EMEM);
            *buf_len = *word_len;
            memcpy(*word_b, *word_p, *word_len);
        }

        /* add this new character into buffer */
        if (buf_add_char(ctx, *c, word_b, buf_len, *word_len)) {
            return LY_EMEM;
        }

        /* in case of realloc */
        *word_p = *word_b;
    } else {
        /* just remember the first character pointer */
        if (!*word_p) {
            *word_p = (char *)c;
        }
    }

    ++(*word_len);
    return LY_SUCCESS;
}

static LY_ERR
check_char(struct ly_ctx *ctx, char c, enum yang_arg arg, int first, int *prefix)
{
    if ((arg == Y_STR_ARG) || (arg == Y_MAYBE_STR_ARG)) {
        if ((c < ' ') && (c != '\t') && (c != '\n')) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHAR, c);
            return LY_EVALID;
        }
    } else if ((arg == Y_IDENTIF_ARG) || (arg == Y_PREF_IDENTIF_ARG)) {
        if (c < ' ') {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHAR, c);
            return LY_EVALID;
        }

        if (first || (*prefix == 2)) {
            /* either first character of the whole identifier or of the name after prefix */
            if (!isalpha(c) && (c != '_')) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX_YANG, "Invalid identifier first character '%c'.", c);
                return LY_EVALID;
            }
            if (*prefix == 2) {
                *prefix = 1;
            }
        } else {
            /* other characters */
            if (!isalnum(c) && (c != '_') && (c != '-') && (c != '.')) {
                if ((arg == Y_PREF_IDENTIF_ARG) && !*prefix && (c == ':')) {
                    /* this is fine */
                    *prefix = 2;
                } else {
                    LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX_YANG, "Invalid identifier character '%c'.", c);
                    return LY_EVALID;
                }
            }
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
skip_comment(struct ly_ctx *ctx, const char **data, int comment)
{
    /* comment: 0 - comment ended, 1 - in line comment, 2 - in block comment, 3 - in block comment with last read character '*' */

    while (**data && comment) {
        switch (comment) {
        case 1:
            if (**data == '\n') {
                comment = 0;
            }
            break;
        case 2:
            if (**data == '*') {
                comment = 3;
            }
            break;
        case 3:
            if (**data == '/') {
                comment = 0;
            } else {
                comment = 2;
            }
            break;
        default:
            LOGINT_RET(ctx);
        }

        ++(*data);
    }

    if (!**data && (comment > 1)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX, "Unexpected end-of-file, non-terminated comment.");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

static LY_ERR
read_qstring(struct ly_ctx *ctx, const char **data, enum yang_arg arg, char **word_p, char **word_b, int *word_len,
             int *buf_len, int indent)
{
    /* string: 0 - string ended, 1 - string with ', 2 - string with ", 3 - string with " with last character \,
     *         4 - string finished, now skipping whitespaces looking for +,
     *         5 - string continues after +, skipping whitespaces */
    int string, str_nl_indent = 0, need_buf = 0, prefix = 0;
    const char *c;

    if (**data == '\"') {
        /* indent of the " itself */
        ++indent;
        string = 2;
    } else {
        assert(**data == '\'');
        string = 1;
    }
    ++(*data);

    while (**data && string) {
        switch (string) {
        case 1:
            switch (**data) {
            case '\'':
                /* string may be finished, but check for + */
                string = 4;
                break;
            default:
                /* check and store character */
                if (check_char(ctx, **data, arg, !*word_len, &prefix)) {
                    return LY_EVALID;
                }
                if (buf_store_char(ctx, *data, word_p, word_len, word_b, buf_len, need_buf)) {
                    return LY_EMEM;
                }
                break;
            }
            break;
        case 2:
            switch (**data) {
            case '\"':
                /* string may be finished, but check for + */
                string = 4;
                break;
            case '\\':
                /* special character following */
                string = 3;
                break;
            case ' ':
                if (str_nl_indent) {
                    --str_nl_indent;
                } else {
                    /* check and store character */
                    if (check_char(ctx, **data, arg, !*word_len, &prefix)) {
                        return LY_EVALID;
                    }
                    if (buf_store_char(ctx, *data, word_p, word_len, word_b, buf_len, need_buf)) {
                        return LY_EMEM;
                    }
                }
                break;
            case '\t':
                if (str_nl_indent) {
                    if (str_nl_indent < 9) {
                        str_nl_indent = 0;
                    } else {
                        str_nl_indent -= 8;
                    }
                } else {
                    /* check and store character */
                    if (check_char(ctx, **data, arg, !*word_len, &prefix)) {
                        return LY_EVALID;
                    }
                    if (buf_store_char(ctx, *data, word_p, word_len, word_b, buf_len, need_buf)) {
                        return LY_EMEM;
                    }
                }
                break;
            case '\n':
                str_nl_indent = indent;
                if (indent) {
                    /* we will be removing the indents so we need our own buffer */
                    need_buf = 1;
                }

                /* check and store character */
                if (check_char(ctx, **data, arg, !*word_len, &prefix)) {
                    return LY_EVALID;
                }
                if (buf_store_char(ctx, *data, word_p, word_len, word_b, buf_len, need_buf)) {
                    return LY_EMEM;
                }
                break;
            default:
                /* first non-whitespace character, clear current indent */
                str_nl_indent = 0;

                /* check and store character */
                if (check_char(ctx, **data, arg, !*word_len, &prefix)) {
                    return LY_EVALID;
                }
                if (buf_store_char(ctx, *data, word_p, word_len, word_b, buf_len, need_buf)) {
                    return LY_EMEM;
                }
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
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX_YANG,
                       "Double-quoted string unknown special character '\\%c'.\n", **data);
                return LY_EVALID;
            }

            /* check and store character */
            if (check_char(ctx, *c, arg, !*word_len, &prefix)) {
                return LY_EVALID;
            }
            if (buf_store_char(ctx, c, word_p, word_len, word_b, buf_len, need_buf)) {
                return LY_EMEM;
            }

            string = 2;
            break;
        case 4:
            switch (**data) {
            case '+':
                /* string continues */
                string = 5;
                break;
            case ' ':
            case '\t':
            case '\n':
                /* just skip */
                break;
            default:
                /* string is finished */
                goto string_end;
            }
            break;
        case 5:
            switch (**data) {
            case ' ':
            case '\t':
            case '\n':
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
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX_YANG,
                       "Both string parts divided by '+' must be quoted.\n");
                return LY_EVALID;
            }
            break;
        default:
            return LY_EINT;
        }

        ++(*data);
    }

string_end:
    return LY_SUCCESS;
}

/* read another word - character sequence without whitespaces (logs directly)
 * - there can be whitespaces if they are a part of string
 * - strings are always returned separately even if not separated by whitespaces
 * - strings are returned without ' or "
 * - strings divided by + are returned concatenated
 * - comments are skipped
 */
static LY_ERR
get_string(struct ly_ctx *ctx, const char **data, enum yang_arg arg, char **word_p, char **word_b, int *word_len)
{
    int buf_len = 0, slash = 0, indent = 0, prefix = 0;
    LY_ERR ret;

    /* word buffer - dynamically allocated */
    *word_b = NULL;

    /* word pointer - just a pointer to data */
    *word_p = NULL;

    *word_len = 0;
    while (**data) {
        if (slash) {
            if (**data == '/') {
                /* one-line comment */
                ret = skip_comment(ctx, data, 1);
                if (ret) {
                    return ret;
                }
            } else if (**data == '*') {
                /* block comment */
                ret = skip_comment(ctx, data, 2);
                if (ret) {
                    return ret;
                }
            } else {
                /* not a comment after all */
                ret = buf_store_char(ctx, (*data) - 1, word_p, word_len, word_b, &buf_len, 0);
                if (ret) {
                    return ret;
                }
            }
            slash = 0;
        }

        switch (**data) {
        case '\'':
        case '\"':
            if (*word_len) {
                /* we want strings always in a separate word, leave it */
                goto str_end;
            }
            ret = read_qstring(ctx, data, arg, word_p, word_b, word_len, &buf_len, indent);
            if (ret) {
                return ret;
            }
            goto str_end;
        case '/':
            slash = 1;
            break;
        case ' ':
            if (*word_len) {
                /* word is finished */
                goto str_end;
            }
            /* increase indent */
            ++indent;
            break;
        case '\t':
            if (*word_len) {
                /* word is finished */
                goto str_end;
            }
            /* tabs count for 8 spaces */
            indent += 8;
            break;
        case '\n':
            if (*word_len) {
                /* word is finished */
                goto str_end;
            }
            /* reset indent */
            indent = 0;
            break;
        case ';':
        case '{':
            if (*word_len || (arg == Y_MAYBE_STR_ARG)) {
                /* word is finished */
                goto str_end;
            }

            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INSTREXP, 1, *data, "an argument");
            return LY_EVALID;
        default:
            if (check_char(ctx, **data, arg, !*word_len, &prefix)) {
                return LY_EVALID;
            }

            if (buf_store_char(ctx, *data, word_p, word_len, word_b, &buf_len, 0)) {
                return LY_EMEM;
            }
            break;
        }

        ++(*data);
    }

str_end:
    /* ending '\0' for buf */
    if (*word_b) {
        if (buf_add_char(ctx, '\0', word_b, &buf_len, *word_len)) {
            return LY_EMEM;
        }
        *word_p = *word_b;
    }

    return LY_SUCCESS;
}

static LY_ERR
get_keyword(struct ly_ctx *ctx, const char **data, enum yang_keyword *kw, char **word_p, int *word_len)
{
    LY_ERR ret;
    int prefix;
    const char *word_start;
    /* slash: 0 - nothing, 1 - last character was '/' */
    int slash = 0;

    if (word_p) {
        *word_p = NULL;
        *word_len = 0;
    }

    /* first skip "optsep", comments */
    while (**data) {
        if (slash) {
            if (**data == '/') {
                /* one-line comment */
                ret = skip_comment(ctx, data, 1);
                if (ret) {
                    return ret;
                }
            } else if (**data == '*') {
                /* block comment */
                ret = skip_comment(ctx, data, 2);
                if (ret) {
                    return ret;
                }
            } else {
                /* not a comment after all */
                goto keyword_start;
            }
            slash = 0;
        }

        switch (**data) {
        case '/':
            slash = 1;
            break;
        case ' ':
        case '\t':
        case '\n':
            /* skip whitespaces (optsep) */
            break;
        default:
            /* either a keyword start or an invalid character */
            goto keyword_start;
        }

        ++(*data);
    }

keyword_start:
    word_start = *data;
    *kw = YANG_NONE;

    /* read the keyword itself */
    switch (**data) {
    case 'a':
        ++(*data);
        if (!strncmp(*data, "rgument", 7)) {
            *data += 7;
            *kw = YANG_ARGUMENT;
        } else if (!strncmp(*data, "ugment", 6)) {
            *data += 6;
            *kw = YANG_AUGMENT;
        } else if (!strncmp(*data, "ction", 5)) {
            *data += 5;
            *kw = YANG_ACTION;
        } else if (!strncmp(*data, "ny", 2)) {
            *data += 2;
            if (!strncmp(*data, "data", 4)) {
                *data += 4;
                *kw = YANG_ANYDATA;
            } else if (!strncmp(*data, "xml", 3)) {
                *data += 3;
                *kw = YANG_ANYXML;
            }
        }
        break;
    case 'b':
        ++(*data);
        if (!strncmp(*data, "ase", 3)) {
            *data += 3;
            *kw = YANG_BASE;
        } else if (!strncmp(*data, "elongs-to", 9)) {
            *data += 9;
            *kw = YANG_BELONGS_TO;
        } else if (!strncmp(*data, "it", 2)) {
            *data += 2;
            *kw = YANG_BIT;
        }
        break;
    case 'c':
        ++(*data);
        if (!strncmp(*data, "ase", 3)) {
            *data += 3;
            *kw = YANG_CASE;
        } else if (!strncmp(*data, "hoice", 5)) {
            *data += 5;
            *kw = YANG_CHOICE;
        } else if (!strncmp(*data, "on", 2)) {
            *data += 2;
            if (!strncmp(*data, "fig", 3)) {
                *data += 3;
                *kw = YANG_CONFIG;
            } else if (!strncmp(*data, "ta", 2)) {
                *data += 2;
                if (!strncmp(*data, "ct", 2)) {
                    *data += 2;
                    *kw = YANG_CONTACT;
                } else if (!strncmp(*data, "iner", 4)) {
                    *data += 4;
                    *kw = YANG_CONTAINER;
                }
            }
        }
        break;
    case 'd':
        ++(*data);
        if (**data == 'e') {
            ++(*data);
            if (!strncmp(*data, "fault", 5)) {
                *data += 5;
                *kw = YANG_DEFAULT;
            } else if (!strncmp(*data, "scription", 9)) {
                *data += 9;
                *kw = YANG_DESCRIPTION;
            } else if (!strncmp(*data, "viat", 4)) {
                *data += 4;
                if (**data == 'e') {
                    ++(*data);
                    *kw = YANG_DEVIATE;
                } else if (!strncmp(*data, "ion", 3)) {
                    *data += 3;
                    *kw = YANG_DEVIATION;
                }
            }
        }
        break;
    case 'e':
        ++(*data);
        if (!strncmp(*data, "num", 3)) {
            *data += 3;
            *kw = YANG_ENUM;
        } else if (!strncmp(*data, "rror-", 5)) {
            *data += 5;
            if (!strncmp(*data, "app-tag", 7)) {
                *data += 7;
                *kw = YANG_ERROR_APP_TAG;
            } else if (!strncmp(*data, "message", 7)) {
                *data += 7;
                *kw = YANG_ERROR_MESSAGE;
            }
        } else if (!strncmp(*data, "xtension", 8)) {
            *data += 8;
            *kw = YANG_EXTENSION;
        }
        break;
    case 'f':
        ++(*data);
        if (!strncmp(*data, "eature", 6)) {
            *data += 6;
            *kw = YANG_FEATURE;
        } else if (!strncmp(*data, "raction-digits", 14)) {
            *data += 14;
            *kw = YANG_FRACTION_DIGITS;
        }
        break;
    case 'g':
        ++(*data);
        if (!strncmp(*data, "rouping", 7)) {
            *data += 7;
            *kw = YANG_GROUPING;
        }
        break;
    case 'i':
        ++(*data);
        if (!strncmp(*data, "dentity", 7)) {
            *data += 7;
            *kw = YANG_IDENTITY;
        } else if (!strncmp(*data, "f-feature", 9)) {
            *data += 9;
            *kw = YANG_IF_FEATURE;
        } else if (!strncmp(*data, "mport", 5)) {
            *data += 5;
            *kw = YANG_IMPORT;
        } else if (**data == 'n') {
            ++(*data);
            if (!strncmp(*data, "clude", 5)) {
                *data += 5;
                *kw = YANG_INCLUDE;
            } else if (!strncmp(*data, "put", 3)) {
                *data += 3;
                *kw = YANG_INPUT;
            }
        }
        break;
    case 'k':
        ++(*data);
        if (!strncmp(*data, "ey", 2)) {
            *data += 2;
            *kw = YANG_KEY;
        }
        break;
    case 'l':
        ++(*data);
        if (**data == 'e') {
            ++(*data);
            if (!strncmp(*data, "af", 2)) {
                *data += 2;
                if (**data != '-') {
                    *kw = YANG_LEAF;
                } else if (!strncmp(*data, "-list", 5)) {
                    *data += 5;
                    *kw = YANG_LEAF_LIST;
                }
            } else if (!strncmp(*data, "ngth", 4)) {
                *data += 4;
                *kw = YANG_LENGTH;
            }
        } else if (!strncmp(*data, "ist", 3)) {
            *data += 3;
            *kw = YANG_LIST;
        }
        break;
    case 'm':
        ++(*data);
        if (**data == 'a') {
            ++(*data);
            if (!strncmp(*data, "ndatory", 7)) {
                *data += 7;
                *kw = YANG_MANDATORY;
            } else if (!strncmp(*data, "x-elements", 10)) {
                *data += 10;
                *kw = YANG_MAX_ELEMENTS;
            }
        } else if (!strncmp(*data, "in-elements", 11)) {
            *data += 11;
            *kw = YANG_MIN_ELEMENTS;
        } else if (!strncmp(*data, "ust", 3)) {
            *data += 3;
            *kw = YANG_MUST;
        } else if (!strncmp(*data, "od", 2)) {
            *data += 2;
            if (!strncmp(*data, "ule", 3)) {
                *data += 3;
                *kw = YANG_MODULE;
            } else if (!strncmp(*data, "ifier", 5)) {
                *data += 3;
                *kw = YANG_MODIFIER;
            }
        }
        break;
    case 'n':
        ++(*data);
        if (!strncmp(*data, "amespace", 8)) {
            *data += 8;
            *kw = YANG_NAMESPACE;
        } else if (!strncmp(*data, "otification", 11)) {
            *data += 11;
            *kw = YANG_NOTIFICATION;
        }
        break;
    case 'o':
        ++(*data);
        if (**data == 'r') {
            ++(*data);
            if (!strncmp(*data, "dered-by", 8)) {
                *data += 8;
                *kw = YANG_ORDERED_BY;
            } else if (!strncmp(*data, "ganization", 10)) {
                *data += 10;
                *kw = YANG_ORGANIZATION;
            }
        } else if (!strncmp(*data, "utput", 5)) {
            *data += 5;
            *kw = YANG_OUTPUT;
        }
        break;
    case 'p':
        ++(*data);
        if (!strncmp(*data, "at", 2)) {
            *data += 2;
            if (**data == 'h') {
                ++(*data);
                *kw = YANG_PATH;
            } else if (!strncmp(*data, "tern", 4)) {
                *data += 4;
                *kw = YANG_PATTERN;
            }
        } else if (!strncmp(*data, "osition", 7)) {
            *data += 7;
            *kw = YANG_POSITION;
        } else if (!strncmp(*data, "re", 2)) {
            *data += 2;
            if (!strncmp(*data, "fix", 3)) {
                *data += 3;
                *kw = YANG_PREFIX;
            } else if (!strncmp(*data, "sence", 5)) {
                *data += 5;
                *kw = YANG_PRESENCE;
            }
        }
        break;
    case 'r':
        ++(*data);
        if (!strncmp(*data, "ange", 4)) {
            *data += 4;
            *kw = YANG_RANGE;
        } else if (**data == 'e') {
            ++(*data);
            if (**data == 'f') {
                ++(*data);
                if (!strncmp(*data, "erence", 6)) {
                    *data += 6;
                    *kw = YANG_REFERENCE;
                } else if (!strncmp(*data, "ine", 3)) {
                    *data += 3;
                    *kw = YANG_REFINE;
                }
            } else if (!strncmp(*data, "quire-instance", 14)) {
                *data += 14;
                *kw = YANG_REQUIRE_INSTANCE;
            } else if (!strncmp(*data, "vision", 6)) {
                *data += 6;
                if (**data != '-') {
                    *kw = YANG_REVISION;
                } else if (!strncmp(*data, "-date", 5)) {
                    *data += 5;
                    *kw = YANG_REVISION_DATE;
                }
            }
        } else if (!strncmp(*data, "pc", 2)) {
            *data += 2;
            *kw = YANG_RPC;
        }
        break;
    case 's':
        ++(*data);
        if (!strncmp(*data, "tatus", 5)) {
            *data += 5;
            *kw = YANG_STATUS;
        } else if (!strncmp(*data, "ubmodule", 8)) {
            *data += 8;
            *kw = YANG_SUBMODULE;
        }
        break;
    case 't':
        ++(*data);
        if (!strncmp(*data, "ype", 3)) {
            *data += 3;
            if (**data != 'd') {
                *kw = YANG_TYPE;
            } else if (!strncmp(*data, "def", 3)) {
                *data += 3;
                *kw = YANG_TYPEDEF;
            }
        }
        break;
    case 'u':
        ++(*data);
        if (!strncmp(*data, "ni", 2)) {
            *data += 2;
            if (!strncmp(*data, "que", 3)) {
                *data += 3;
                *kw = YANG_UNIQUE;
            } else if (!strncmp(*data, "ts", 2)) {
                *data += 2;
                *kw = YANG_UNITS;
            }
        } else if (!strncmp(*data, "ses", 3)) {
            *data += 3;
            *kw = YANG_USES;
        }
        break;
    case 'v':
        ++(*data);
        if (!strncmp(*data, "alue", 4)) {
            *data += 4;
            *kw = YANG_VALUE;
        }
        break;
    case 'w':
        ++(*data);
        if (!strncmp(*data, "hen", 3)) {
            *data += 3;
            *kw = YANG_WHEN;
        }
        break;
    case 'y':
        ++(*data);
        if (!strncmp(*data, "ang-version", 11)) {
            *data += 11;
            *kw = YANG_YANG_VERSION;
        } else if (!strncmp(*data, "in-element", 10)) {
            *data += 10;
            *kw = YANG_YIN_ELEMENT;
        }
        break;
    case ';':
        ++(*data);
        *kw = YANG_SEMICOLON;
        break;
    case '{':
        ++(*data);
        *kw = YANG_LEFT_BRACE;
        break;
    case '}':
        ++(*data);
        *kw = YANG_RIGHT_BRACE;
        break;
    default:
        break;
    }

    if (*kw != YANG_NONE) {
        /* make sure we have the whole keyword */
        switch (**data) {
        case ' ':
        case '\t':
        case '\n':
            /* mandatory "sep" */
            break;
        default:
            ++(*data);
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INSTREXP, (int)(*data - word_start), word_start,
                   "a keyword followed by a separator");
            return LY_EVALID;
        }
    } else {
        /* still can be an extension */
        prefix = 0;
        while (**data && (**data != ' ') && (**data != '\t') && (**data != '\n') && (**data != '{') && (**data != ';')) {
            if (check_char(ctx, **data, Y_PREF_IDENTIF_ARG, *data == word_start ? 1 : 0, &prefix)) {
                return LY_EVALID;
            }
            ++(*data);
        }
        if (!**data) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_EOF);
            return LY_EVALID;
        }

        /* prefix is mandatory for extension instances */
        if (prefix != 1) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INSTREXP, (int)(*data - word_start), word_start, "a keyword");
            return LY_EVALID;
        }

        *kw = YANG_CUSTOM;
    }

    if (word_p) {
        *word_p = (char *)word_start;
        *word_len = *data - word_start;
    }

    return LY_SUCCESS;
}

static LY_ERR
parse_ext_substmt(struct ly_ctx *ctx, const char **data, char *word, int word_len,
                  struct lysp_stmt **child)
{
    char *buf;
    LY_ERR ret = 0;
    enum yang_keyword kw;
    struct lysp_stmt *stmt, *par_child;

    stmt = calloc(1, sizeof *stmt);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(NULL), LY_EMEM);

    stmt->stmt = lydict_insert(ctx, word, word_len);

    /* get optional argument */
    ret = get_string(ctx, data, Y_MAYBE_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        stmt->arg = lydict_insert_zc(ctx, word);
    } else {
        stmt->arg = lydict_insert(ctx, word, word_len);
    }

    /* insert into parent statements */
    if (!*child) {
        *child = stmt;
    } else {
        for (par_child = *child; par_child->next; par_child = par_child->next);
        par_child->next = stmt;
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        ret = parse_ext_substmt(ctx, data, word, word_len, &stmt->child);
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_ext(struct ly_ctx *ctx, const char **data, const char *ext_name, int ext_name_len, LYEXT_SUBSTMT insubstmt,
          uint32_t insubstmt_index, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    struct lysp_ext_instance *e;
    enum yang_keyword kw;

    LYSP_ARRAY_NEW_RET(ctx, exts, e, LY_EMEM);

    /* store name and insubstmt info */
    e->name = lydict_insert(ctx, ext_name, ext_name_len);
    e->insubstmt = insubstmt;
    e->insubstmt_index = insubstmt_index;

    /* get optional argument */
    ret = get_string(ctx, data, Y_MAYBE_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        e->argument = lydict_insert_zc(ctx, word);
    } else {
        e->argument = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        ret = parse_ext_substmt(ctx, data, word, word_len, &e->child);
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_text_field(struct ly_ctx *ctx, const char **data, LYEXT_SUBSTMT substmt, uint32_t substmt_index,
                 const char **value, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*value) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, substmt2str(substmt));
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, arg, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    /* store value and spend buf if allocated */
    if (buf) {
        *value = lydict_insert_zc(ctx, word);
    } else {
        *value = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, substmt, substmt_index, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), substmt2str(substmt));
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_yangversion(struct ly_ctx *ctx, const char **data, struct lysp_module *mod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (mod->version) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "yang-version");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 3) && !strncmp(word, "1.0", word_len)) {
        mod->version = LYS_VERSION_1_0;
    } else if ((word_len == 3) && !strncmp(word, "1.1", word_len)) {
        mod->version = LYS_VERSION_1_1;
    } else {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "yang-version");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_VERSION, 0, &mod->exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "yang-version");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_belongsto(struct ly_ctx *ctx, const char **data, const char **belongsto, const char **prefix, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*belongsto) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "belongs-to");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        *belongsto = lydict_insert_zc(ctx, word);
    } else {
        *belongsto = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_PREFIX:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, prefix, Y_IDENTIF_ARG, exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_BELONGSTO, 0, exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "belongs-to");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    /* mandatory substatements */
    if (!*prefix) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "prefix", "belongs-to");
        return LY_EVALID;
    }

    return ret;
}

static LY_ERR
parse_revisiondate(struct ly_ctx *ctx, const char **data, char *rev, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (rev[0]) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "revision-date");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    /* check value */
    if (lysp_check_date(ctx, word, word_len, "revision-date")) {
        free(buf);
        return LY_EVALID;
    }

    /* store value and spend buf if allocated */
    strncpy(rev, word, word_len);
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_REVISIONDATE, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "revision-date");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_include(struct ly_ctx *ctx, const char **data, struct lysp_include **includes)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_include *inc;

    LYSP_ARRAY_NEW_RET(ctx, includes, inc, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        inc->name = lydict_insert_zc(ctx, word);
    } else {
        inc->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &inc->dsc, Y_STR_ARG, &inc->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &inc->ref, Y_STR_ARG, &inc->exts);
            break;
        case YANG_REVISION_DATE:
            ret = parse_revisiondate(ctx, data, inc->rev, &inc->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &inc->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "include");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_import(struct ly_ctx *ctx, const char **data, struct lysp_import **imports)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_import *imp;

    LYSP_ARRAY_NEW_RET(ctx, imports, imp, LY_EVALID);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        imp->name = lydict_insert_zc(ctx, word);
    } else {
        imp->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_PREFIX:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, &imp->prefix, Y_IDENTIF_ARG, &imp->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &imp->dsc, Y_STR_ARG, &imp->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &imp->ref, Y_STR_ARG, &imp->exts);
            break;
        case YANG_REVISION_DATE:
            ret = parse_revisiondate(ctx, data, imp->rev, &imp->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &imp->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "import");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    /* mandatory substatements */
    if (!imp->prefix) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "prefix", "import");
        return LY_EVALID;
    }

    return ret;
}

static LY_ERR
parse_revision(struct ly_ctx *ctx, const char **data, struct lysp_revision **revs)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_revision *rev;

    LYSP_ARRAY_NEW_RET(ctx, revs, rev, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    /* check value */
    if (lysp_check_date(ctx, word, word_len, "revision")) {
        return LY_EVALID;
    }

    strncpy(rev->rev, word, word_len);
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &rev->dsc, Y_STR_ARG, &rev->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &rev->ref, Y_STR_ARG, &rev->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &rev->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "revision");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_text_fields(struct ly_ctx *ctx, const char **data, LYEXT_SUBSTMT substmt, const char ***texts, enum yang_arg arg,
                  struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int count, word_len;
    enum yang_keyword kw;

    /* allocate new pointer */
    for (count = 1; (*texts) && (*texts)[count - 1]; ++count);
    *texts = realloc(*texts, count * sizeof **texts);
    LY_CHECK_ERR_RET(!*texts, LOGMEM(ctx), LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, arg, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        (*texts)[count - 1] = lydict_insert_zc(ctx, word);
    } else {
        (*texts)[count - 1] = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, substmt, count - 1, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), substmt2str(substmt));
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_config(struct ly_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*flags & LYS_CONFIG_MASK) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "config");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_CONFIG_W;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_CONFIG_R;
    } else {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "config");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_CONFIG, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "config");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_mandatory(struct ly_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*flags & LYS_MAND_MASK) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "mandatory");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_MAND_TRUE;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "mandatory");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MANDATORY, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "mandatory");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_restr(struct ly_ctx *ctx, const char **data, enum yang_keyword restr_kw, struct lysp_restr *restr)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        restr->arg = lydict_insert_zc(ctx, word);
    } else {
        restr->arg = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts);
            break;
        case YANG_ERROR_APP_TAG:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts);
            break;
        case YANG_ERROR_MESSAGE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &restr->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), kw2str(restr_kw));
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_restrs(struct ly_ctx *ctx, const char **data, enum yang_keyword restr_kw, struct lysp_restr **restrs)
{
    struct lysp_restr *restr;

    LYSP_ARRAY_NEW_RET(ctx, restrs, restr, LY_EMEM);

    return parse_restr(ctx, data, restr_kw, restr);
}

static LY_ERR
parse_status(struct ly_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "status");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 7) && !strncmp(word, "current", word_len)) {
        *flags |= LYS_STATUS_CURR;
    } else if ((word_len == 10) && !strncmp(word, "deprecated", word_len)) {
        *flags |= LYS_STATUS_DEPRC;
    } else if ((word_len == 8) && !strncmp(word, "obsolete", word_len)) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "status");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_STATUS, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "status");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_when(struct ly_ctx *ctx, const char **data, struct lysp_when **when_p)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_when *when;

    if (*when_p) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "when");
        return LY_EVALID;
    }

    when = calloc(1, sizeof *when);
    LY_CHECK_ERR_RET(!when, LOGMEM(ctx), LY_EMEM);
    *when_p = when;

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        when->cond = lydict_insert_zc(ctx, word);
    } else {
        when->cond = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &when->dsc, Y_STR_ARG, &when->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &when->ref, Y_STR_ARG, &when->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &when->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "when");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_any(struct ly_ctx *ctx, const char **data, enum yang_keyword kw, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    struct lysp_node *iter;
    struct lysp_node_anydata *any;

    /* create structure */
    any = calloc(1, sizeof *any);
    LY_CHECK_ERR_RET(!any, LOGMEM(ctx), LY_EMEM);
    any->nodetype = kw == YANG_ANYDATA ? LYS_ANYDATA : LYS_ANYXML;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)any;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)any;
    }

    /* get name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        any->name = lydict_insert_zc(ctx, word);
    } else {
        any->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            ret = parse_config(ctx, data, &any->flags, &any->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &any->dsc, Y_STR_ARG, &any->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &any->iffeatures, Y_STR_ARG, &any->exts);
            break;
        case YANG_MANDATORY:
            ret = parse_mandatory(ctx, data, &any->flags, &any->exts);
            break;
        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &any->musts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &any->ref, Y_STR_ARG, &any->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &any->flags, &any->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &any->when);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &any->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), nodetype2str(any->nodetype));
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_type_enum_value_pos(struct ly_ctx *ctx, const char **data, enum yang_keyword val_kw, int64_t *value, uint16_t *flags,
                          struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word, *ptr;
    int word_len;
    long int num;
    unsigned long int unum;
    enum yang_keyword kw;

    if (*flags & LYS_SET_VALUE) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, kw2str(val_kw));
        return LY_EVALID;
    }
    *flags |= LYS_SET_VALUE;

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (!word_len || (word[0] == '+') || ((word[0] == '0') && (word_len > 1)) || ((val_kw == YANG_VALUE) && !strncmp(word, "-0", 2))) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, kw2str(val_kw));
        free(buf);
        return LY_EVALID;
    }

    errno = 0;
    if (val_kw == YANG_VALUE) {
        num = strtol(word, &ptr, 10);
    } else {
        unum = strtoul(word, &ptr, 10);
    }
    /* we have not parsed the whole argument */
    if (ptr - word != word_len) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, kw2str(val_kw));
        free(buf);
        return LY_EVALID;
    }
    if (errno == ERANGE) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_OOB, word_len, word, kw2str(val_kw));
        free(buf);
        return LY_EVALID;
    }
    if (val_kw == YANG_VALUE) {
        *value = num;
    } else {
        *value = unum;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, val_kw == YANG_VALUE ? LYEXT_SUBSTMT_VALUE : LYEXT_SUBSTMT_POSITION, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), kw2str(val_kw));
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_type_enum(struct ly_ctx *ctx, const char **data, enum yang_keyword enum_kw, struct lysp_type_enum **enums)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_type_enum *enm;

    LYSP_ARRAY_NEW_RET(ctx, enums, enm, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        enm->name = lydict_insert_zc(ctx, word);
    } else {
        enm->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &enm->dsc, Y_STR_ARG, &enm->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &enm->iffeatures, Y_STR_ARG, &enm->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &enm->ref, Y_STR_ARG, &enm->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &enm->flags, &enm->exts);
            break;
        case YANG_VALUE:
        case YANG_POSITION:
            ret = parse_type_enum_value_pos(ctx, data, kw, &enm->value, &enm->flags, &enm->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &enm->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), kw2str(enum_kw));
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_type_fracdigits(struct ly_ctx *ctx, const char **data, uint8_t *fracdig, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word, *ptr;
    int word_len;
    unsigned long int num;
    enum yang_keyword kw;

    if (*fracdig) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "fraction-digits");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (!word_len || (word[0] == '0') || !isdigit(word[0])) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(word, &ptr, 10);
    /* we have not parsed the whole argument */
    if (ptr - word != word_len) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > 18)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_OOB, word_len, word, "fraction-digits");
        free(buf);
        return LY_EVALID;
    }
    *fracdig = num;
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_FRACDIGITS, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "fraction-digits");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_type_reqinstance(struct ly_ctx *ctx, const char **data, uint8_t *reqinst, uint16_t *flags,
                       struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*flags & LYS_SET_REQINST) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "require-instance");
        return LY_EVALID;
    }
    *flags |= LYS_SET_REQINST;

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *reqinst = 1;
    } else if ((word_len != 5) || strncmp(word, "false", word_len)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "require-instance");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_REQINSTANCE, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "require-instance");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_type_pattern_modifier(struct ly_ctx *ctx, const char **data, const char **pat, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if ((*pat)[0] == 0x15) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "modifier");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len != 12) || strncmp(word, "invert-match", word_len)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "modifier");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    /* replace the value in the dictionary */
    buf = malloc(strlen(*pat) + 1);
    LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
    strcpy(buf, *pat);
    lydict_remove(ctx, *pat);

    assert(buf[0] == 0x06);
    buf[0] = 0x15;
    *pat = lydict_insert_zc(ctx, buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MODIFIER, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "modifier");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_type_pattern(struct ly_ctx *ctx, const char **data, struct lysp_restr **patterns)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_restr *restr;

    LYSP_ARRAY_NEW_RET(ctx, patterns, restr, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    /* add special meaning first byte */
    if (buf) {
        buf = realloc(buf, word_len + 2);
        word = buf;
    } else {
        buf = malloc(word_len + 2);
    }
    LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
    memmove(buf + 1, word, word_len + 1);
    word[0] = 0x06;
    restr->arg = lydict_insert_zc(ctx, word);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts);
            break;
        case YANG_ERROR_APP_TAG:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts);
            break;
        case YANG_ERROR_MESSAGE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts);
            break;
        case YANG_MODIFIER:
            ret = parse_type_pattern_modifier(ctx, data, &restr->arg, &restr->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &restr->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "pattern");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_type(struct ly_ctx *ctx, const char **data, struct lysp_type *type)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_type *nest_type;

    if (type->name) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "type");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_PREF_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        type->name = lydict_insert_zc(ctx, word);
    } else {
        type->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_BASE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_BASE, &type->bases, Y_PREF_IDENTIF_ARG, &type->exts);
            break;
        case YANG_BIT:
            ret = parse_type_enum(ctx, data, kw, &type->bits);
            break;
        case YANG_ENUM:
            ret = parse_type_enum(ctx, data, kw, &type->enums);
            break;
        case YANG_FRACTION_DIGITS:
            ret = parse_type_fracdigits(ctx, data, &type->fraction_digits, &type->exts);
            break;
        case YANG_LENGTH:
            if (type->length) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, kw2str(kw));
                return LY_EVALID;
            }
            type->length = calloc(1, sizeof *type->length);
            LY_CHECK_ERR_RET(!type->length, LOGMEM(ctx), LY_EMEM);

            ret = parse_restr(ctx, data, kw, type->length);
            break;
        case YANG_PATH:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_PATH, 0, &type->path, Y_STR_ARG, &type->exts);
            break;
        case YANG_PATTERN:
            ret = parse_type_pattern(ctx, data, &type->patterns);
            break;
        case YANG_RANGE:
            if (type->range) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, kw2str(kw));
                return LY_EVALID;
            }
            type->range = calloc(1, sizeof *type->range);
            LY_CHECK_ERR_RET(!type->range, LOGMEM(ctx), LY_EVALID);

            ret = parse_restr(ctx, data, kw, type->range);
            break;
        case YANG_REQUIRE_INSTANCE:
            ret = parse_type_reqinstance(ctx, data, &type->require_instance, &type->flags, &type->exts);
            break;
        case YANG_TYPE:
            {
                LYSP_ARRAY_NEW_RET(ctx, &type->types, nest_type, LY_EMEM);
            }
            ret = parse_type(ctx, data, nest_type);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &type->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "when");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_leaf(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_leaf *leaf;

    /* create structure */
    leaf = calloc(1, sizeof *leaf);
    LY_CHECK_ERR_RET(!leaf, LOGMEM(ctx), LY_EMEM);
    leaf->nodetype = LYS_LEAF;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)leaf;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)leaf;
    }

    /* get name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        leaf->name = lydict_insert_zc(ctx, word);
    } else {
        leaf->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            ret = parse_config(ctx, data, &leaf->flags, &leaf->exts);
            break;
        case YANG_DEFAULT:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &leaf->dflt, Y_STR_ARG, &leaf->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &leaf->dsc, Y_STR_ARG, &leaf->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &leaf->iffeatures, Y_STR_ARG, &leaf->exts);
            break;
        case YANG_MANDATORY:
            ret = parse_mandatory(ctx, data, &leaf->flags, &leaf->exts);
            break;
        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &leaf->musts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &leaf->ref, Y_STR_ARG, &leaf->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &leaf->flags, &leaf->exts);
            break;
        case YANG_TYPE:
            ret = parse_type(ctx, data, &leaf->type);
            break;
        case YANG_UNITS:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &leaf->units, Y_STR_ARG, &leaf->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &leaf->when);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &leaf->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "leaf");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    /* mandatory substatements */
    if (!leaf->type.name) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "type", "leaf");
        return LY_EVALID;
    }

    return ret;
}

static LY_ERR
parse_maxelements(struct ly_ctx *ctx, const char **data, uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word, *ptr;
    int word_len;
    unsigned long int num;
    enum yang_keyword kw;

    if (*flags & LYS_SET_MAX) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "max-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MAX;

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (!word_len || (word[0] == '0') || ((word[0] != 'u') && !isdigit(word[0]))) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "max-elements");
        free(buf);
        return LY_EVALID;
    }

    if (strncmp(word, "unbounded", word_len)) {
        errno = 0;
        num = strtoul(word, &ptr, 10);
        /* we have not parsed the whole argument */
        if (ptr - word != word_len) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "max-elements");
            free(buf);
            return LY_EVALID;
        }
        if ((errno == ERANGE) || (num > UINT32_MAX)) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_OOB, word_len, word, "max-elements");
            free(buf);
            return LY_EVALID;
        }

        *max = num;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MAX, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "max-elements");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_minelements(struct ly_ctx *ctx, const char **data, uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word, *ptr;
    int word_len;
    unsigned long int num;
    enum yang_keyword kw;

    if (*flags & LYS_SET_MIN) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "min-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MIN;

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (!word_len || !isdigit(word[0]) || ((word[0] == '0') && (word_len > 1))) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(word, &ptr, 10);
    /* we have not parsed the whole argument */
    if (ptr - word != word_len) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > UINT32_MAX)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_OOB, word_len, word, "min-elements");
        free(buf);
        return LY_EVALID;
    }
    *min = num;
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_MIN, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "min-elements");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_orderedby(struct ly_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*flags & LYS_ORDBY_MASK) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "ordered-by");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 6) && !strncmp(word, "system", word_len)) {
        *flags |= LYS_ORDBY_SYSTEM;
    } else if ((word_len == 4) && !strncmp(word, "user", word_len)) {
        *flags |= LYS_ORDBY_USER;
    } else {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "ordered-by");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_ORDEREDBY, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "ordered-by");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_leaflist(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_leaflist *llist;

    /* create structure */
    llist = calloc(1, sizeof *llist);
    LY_CHECK_ERR_RET(!llist, LOGMEM(ctx), LY_EMEM);
    llist->nodetype = LYS_LEAFLIST;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)llist;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)llist;
    }

    /* get name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        llist->name = lydict_insert_zc(ctx, word);
    } else {
        llist->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            ret = parse_config(ctx, data, &llist->flags, &llist->exts);
            break;
        case YANG_DEFAULT:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, &llist->dflts, Y_STR_ARG, &llist->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &llist->dsc, Y_STR_ARG, &llist->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &llist->iffeatures, Y_STR_ARG, &llist->exts);
            break;
        case YANG_MAX_ELEMENTS:
            ret = parse_maxelements(ctx, data, &llist->max, &llist->flags, &llist->exts);
            break;
        case YANG_MIN_ELEMENTS:
            ret = parse_minelements(ctx, data, &llist->min, &llist->flags, &llist->exts);
            break;
        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &llist->musts);
            break;
        case YANG_ORDERED_BY:
            ret = parse_orderedby(ctx, data, &llist->flags, &llist->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &llist->ref, Y_STR_ARG, &llist->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &llist->flags, &llist->exts);
            break;
        case YANG_TYPE:
            ret = parse_type(ctx, data, &llist->type);
            break;
        case YANG_UNITS:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &llist->units, Y_STR_ARG, &llist->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &llist->when);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &llist->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "llist");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    /* mandatory substatements */
    if (!llist->type.name) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "type", "leaf-list");
        return LY_EVALID;
    }

    return ret;
}

static LY_ERR
parse_refine(struct ly_ctx *ctx, const char **data, struct lysp_refine **refines)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_refine *rf;

    LYSP_ARRAY_NEW_RET(ctx, refines, rf, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        rf->nodeid = lydict_insert_zc(ctx, word);
    } else {
        rf->nodeid = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            ret = parse_config(ctx, data, &rf->flags, &rf->exts);
            break;
        case YANG_DEFAULT:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, &rf->dflts, Y_STR_ARG, &rf->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &rf->dsc, Y_STR_ARG, &rf->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &rf->iffeatures, Y_STR_ARG, &rf->exts);
            break;
        case YANG_MAX_ELEMENTS:
            ret = parse_maxelements(ctx, data, &rf->max, &rf->flags, &rf->exts);
            break;
        case YANG_MIN_ELEMENTS:
            ret = parse_minelements(ctx, data, &rf->min, &rf->flags, &rf->exts);
            break;
        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &rf->musts);
            break;
        case YANG_MANDATORY:
            ret = parse_mandatory(ctx, data, &rf->flags, &rf->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &rf->ref, Y_STR_ARG, &rf->exts);
            break;
        case YANG_PRESENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_PRESENCE, 0, &rf->presence, Y_STR_ARG, &rf->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &rf->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "refine");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_typedef(struct ly_ctx *ctx, const char **data, struct lysp_tpdf **typedefs)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_tpdf *tpdf;

    LYSP_ARRAY_NEW_RET(ctx, typedefs, tpdf, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        tpdf->name = lydict_insert_zc(ctx, word);
    } else {
        tpdf->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DEFAULT:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &tpdf->dflt, Y_STR_ARG, &tpdf->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &tpdf->dsc, Y_STR_ARG, &tpdf->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &tpdf->ref, Y_STR_ARG, &tpdf->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &tpdf->flags, &tpdf->exts);
            break;
        case YANG_TYPE:
            ret = parse_type(ctx, data, &tpdf->type);
            break;
        case YANG_UNITS:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, &tpdf->units, Y_STR_ARG, &tpdf->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &tpdf->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "typedef");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    /* mandatory substatements */
    if (!tpdf->type.name) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "type", "typedef");
        return LY_EVALID;
    }

    return ret;
}

static LY_ERR
parse_inout(struct ly_ctx *ctx, const char **data, enum yang_keyword kw, struct lysp_action_inout **inout_p)
{
    LY_ERR ret = 0;
    char *word;
    int word_len;
    struct lysp_action_inout *inout;

    if (*inout_p) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, kw2str(kw));
        return LY_EVALID;
    }

    /* create structure */
    inout = calloc(1, sizeof *inout);
    LY_CHECK_ERR_RET(!inout, LOGMEM(ctx), LY_EMEM);
    *inout_p = inout;

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &inout->data);
            break;
        case YANG_CHOICE:
            ret = parse_choice(ctx, data, &inout->data);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &inout->data);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &inout->data);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &inout->data);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &inout->data);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &inout->data);
            break;

        case YANG_TYPEDEF:
            ret = parse_typedef(ctx, data, &inout->typedefs);
            break;
        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &inout->musts);
            break;
        case YANG_GROUPING:
            ret = parse_grouping(ctx, data, &inout->groupings);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &inout->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "input/output");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_action(struct ly_ctx *ctx, const char **data, struct lysp_action **actions)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_action *act;

    LYSP_ARRAY_NEW_RET(ctx, actions, act, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        act->name = lydict_insert_zc(ctx, word);
    } else {
        act->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &act->dsc, Y_STR_ARG, &act->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &act->iffeatures, Y_STR_ARG, &act->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &act->ref, Y_STR_ARG, &act->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &act->flags, &act->exts);
            break;

        case YANG_INPUT:
            ret = parse_inout(ctx, data, kw, &act->input);
            break;
        case YANG_OUTPUT:
            ret = parse_inout(ctx, data, kw, &act->output);
            break;

        case YANG_TYPEDEF:
            ret = parse_typedef(ctx, data, &act->typedefs);
            break;
        case YANG_GROUPING:
            ret = parse_grouping(ctx, data, &act->groupings);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &act->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "action");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_notif(struct ly_ctx *ctx, const char **data, struct lysp_notif **notifs)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_notif *notif;

    LYSP_ARRAY_NEW_RET(ctx, notifs, notif, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        notif->name = lydict_insert_zc(ctx, word);
    } else {
        notif->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &notif->dsc, Y_STR_ARG, &notif->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &notif->iffeatures, Y_STR_ARG, &notif->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &notif->ref, Y_STR_ARG, &notif->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &notif->flags, &notif->exts);
            break;

        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &notif->data);
            break;
        case YANG_CHOICE:
            ret = parse_case(ctx, data, &notif->data);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &notif->data);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &notif->data);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &notif->data);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &notif->data);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &notif->data);
            break;

        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &notif->musts);
            break;
        case YANG_TYPEDEF:
            ret = parse_typedef(ctx, data, &notif->typedefs);
            break;
        case YANG_GROUPING:
            ret = parse_grouping(ctx, data, &notif->groupings);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &notif->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "notification");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_grouping(struct ly_ctx *ctx, const char **data, struct lysp_grp **groupings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_grp *grp;

    LYSP_ARRAY_NEW_RET(ctx, groupings, grp, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        grp->name = lydict_insert_zc(ctx, word);
    } else {
        grp->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &grp->dsc, Y_STR_ARG, &grp->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &grp->ref, Y_STR_ARG, &grp->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &grp->flags, &grp->exts);
            break;

        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &grp->data);
            break;
        case YANG_CHOICE:
            ret = parse_choice(ctx, data, &grp->data);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &grp->data);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &grp->data);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &grp->data);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &grp->data);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &grp->data);
            break;

        case YANG_TYPEDEF:
            ret = parse_typedef(ctx, data, &grp->typedefs);
            break;
        case YANG_ACTION:
            ret = parse_action(ctx, data, &grp->actions);
            break;
        case YANG_GROUPING:
            ret = parse_grouping(ctx, data, &grp->groupings);
            break;
        case YANG_NOTIFICATION:
            ret = parse_notif(ctx, data, &grp->notifs);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &grp->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "augment");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_augment(struct ly_ctx *ctx, const char **data, struct lysp_augment **augments)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_augment *aug;

    LYSP_ARRAY_NEW_RET(ctx, augments, aug, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        aug->nodeid = lydict_insert_zc(ctx, word);
    } else {
        aug->nodeid = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &aug->dsc, Y_STR_ARG, &aug->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &aug->iffeatures, Y_STR_ARG, &aug->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &aug->ref, Y_STR_ARG, &aug->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &aug->flags, &aug->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &aug->when);
            break;

        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &aug->child);
            break;
        case YANG_CASE:
            ret = parse_case(ctx, data, &aug->child);
            break;
        case YANG_CHOICE:
            ret = parse_choice(ctx, data, &aug->child);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &aug->child);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &aug->child);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &aug->child);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &aug->child);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &aug->child);
            break;

        case YANG_ACTION:
            ret = parse_action(ctx, data, &aug->actions);
            break;
        case YANG_NOTIFICATION:
            ret = parse_notif(ctx, data, &aug->notifs);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &aug->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "augment");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_uses(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_uses *uses;

    /* create structure */
    uses = calloc(1, sizeof *uses);
    LY_CHECK_ERR_RET(!uses, LOGMEM(ctx), LY_EMEM);
    uses->nodetype = LYS_USES;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)uses;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)uses;
    }

    /* get name */
    ret = get_string(ctx, data, Y_PREF_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        uses->name = lydict_insert_zc(ctx, word);
    } else {
        uses->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &uses->dsc, Y_STR_ARG, &uses->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &uses->iffeatures, Y_STR_ARG, &uses->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &uses->ref, Y_STR_ARG, &uses->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &uses->flags, &uses->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &uses->when);
            break;

        case YANG_REFINE:
            ret = parse_refine(ctx, data, &uses->refines);
            break;
        case YANG_AUGMENT:
            ret = parse_augment(ctx, data, &uses->augments);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &uses->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "uses");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_case(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_case *cas;

    /* create structure */
    cas = calloc(1, sizeof *cas);
    LY_CHECK_ERR_RET(!cas, LOGMEM(ctx), LY_EMEM);
    cas->nodetype = LYS_CASE;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)cas;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)cas;
    }

    /* get name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        cas->name = lydict_insert_zc(ctx, word);
    } else {
        cas->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &cas->dsc, Y_STR_ARG, &cas->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &cas->iffeatures, Y_STR_ARG, &cas->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &cas->ref, Y_STR_ARG, &cas->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &cas->flags, &cas->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &cas->when);
            break;

        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &cas->child);
            break;
        case YANG_CHOICE:
            ret = parse_case(ctx, data, &cas->child);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &cas->child);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &cas->child);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &cas->child);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &cas->child);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &cas->child);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &cas->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "case");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_choice(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_choice *choic;

    /* create structure */
    choic = calloc(1, sizeof *choic);
    LY_CHECK_ERR_RET(!choic, LOGMEM(ctx), LY_EMEM);
    choic->nodetype = LYS_CHOICE;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)choic;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)choic;
    }

    /* get name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        choic->name = lydict_insert_zc(ctx, word);
    } else {
        choic->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            ret = parse_config(ctx, data, &choic->flags, &choic->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &choic->dsc, Y_STR_ARG, &choic->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &choic->iffeatures, Y_STR_ARG, &choic->exts);
            break;
        case YANG_MANDATORY:
            ret = parse_mandatory(ctx, data, &choic->flags, &choic->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &choic->ref, Y_STR_ARG, &choic->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &choic->flags, &choic->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &choic->when);
            break;
        case YANG_DEFAULT:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &choic->dflt, Y_IDENTIF_ARG, &choic->exts);
            break;

        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &choic->child);
            break;
        case YANG_CASE:
            ret = parse_case(ctx, data, &choic->child);
            break;
        case YANG_CHOICE:
            ret = parse_choice(ctx, data, &choic->child);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &choic->child);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &choic->child);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &choic->child);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &choic->child);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &choic->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "choice");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_container(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_container *cont;

    /* create structure */
    cont = calloc(1, sizeof *cont);
    LY_CHECK_ERR_RET(!cont, LOGMEM(ctx), LY_EMEM);
    cont->nodetype = LYS_CONTAINER;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)cont;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)cont;
    }

    /* get name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        cont->name = lydict_insert_zc(ctx, word);
    } else {
        cont->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            ret = parse_config(ctx, data, &cont->flags, &cont->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &cont->dsc, Y_STR_ARG, &cont->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &cont->iffeatures, Y_STR_ARG, &cont->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &cont->ref, Y_STR_ARG, &cont->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &cont->flags, &cont->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &cont->when);
            break;
        case YANG_PRESENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_PRESENCE, 0, &cont->presence, Y_STR_ARG, &cont->exts);
            break;

        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &cont->child);
            break;
        case YANG_CHOICE:
            ret = parse_choice(ctx, data, &cont->child);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &cont->child);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &cont->child);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &cont->child);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &cont->child);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &cont->child);
            break;

        case YANG_TYPEDEF:
            ret = parse_typedef(ctx, data, &cont->typedefs);
            break;
        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &cont->musts);
            break;
        case YANG_ACTION:
            ret = parse_action(ctx, data, &cont->actions);
            break;
        case YANG_GROUPING:
            ret = parse_grouping(ctx, data, &cont->groupings);
            break;
        case YANG_NOTIFICATION:
            ret = parse_notif(ctx, data, &cont->notifs);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &cont->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "container");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_list(struct ly_ctx *ctx, const char **data, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_node *iter;
    struct lysp_node_list *list;

    /* create structure */
    list = calloc(1, sizeof *list);
    LY_CHECK_ERR_RET(!list, LOGMEM(ctx), LY_EMEM);
    list->nodetype = LYS_LIST;

    /* insert into siblings */
    if (!*siblings) {
        *siblings = (struct lysp_node *)list;
    } else {
        for (iter = *siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)list;
    }

    /* get name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        list->name = lydict_insert_zc(ctx, word);
    } else {
        list->name = lydict_insert(ctx, word, word_len);
    }

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            ret = parse_config(ctx, data, &list->flags, &list->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &list->dsc, Y_STR_ARG, &list->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &list->iffeatures, Y_STR_ARG, &list->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &list->ref, Y_STR_ARG, &list->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &list->flags, &list->exts);
            break;
        case YANG_WHEN:
            ret = parse_when(ctx, data, &list->when);
            break;
        case YANG_KEY:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_KEY, 0, &list->key, Y_STR_ARG, &list->exts);
            break;
        case YANG_MAX_ELEMENTS:
            ret = parse_maxelements(ctx, data, &list->max, &list->flags, &list->exts);
            break;
        case YANG_MIN_ELEMENTS:
            ret = parse_minelements(ctx, data, &list->min, &list->flags, &list->exts);
            break;
        case YANG_ORDERED_BY:
            ret = parse_orderedby(ctx, data, &list->flags, &list->exts);
            break;
        case YANG_UNIQUE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_UNIQUE, &list->uniques, Y_STR_ARG, &list->exts);
            break;

        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &list->child);
            break;
        case YANG_CHOICE:
            ret = parse_choice(ctx, data, &list->child);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &list->child);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &list->child);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &list->child);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &list->child);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &list->child);
            break;

        case YANG_TYPEDEF:
            ret = parse_typedef(ctx, data, &list->typedefs);
            break;
        case YANG_MUST:
            ret = parse_restrs(ctx, data, kw, &list->musts);
            break;
        case YANG_ACTION:
            ret = parse_action(ctx, data, &list->actions);
            break;
        case YANG_GROUPING:
            ret = parse_grouping(ctx, data, &list->groupings);
            break;
        case YANG_NOTIFICATION:
            ret = parse_notif(ctx, data, &list->notifs);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &list->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "container");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_yinelement(struct ly_ctx *ctx, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*flags & LYS_YINELEM_MASK) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "yin-element");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 4) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_YINELEM_TRUE;
    } else if ((word_len == 5) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "yin-element");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_YINELEM, 0, exts);
            YANG_ERR_RET(ret);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "yin-element");
            return LY_EVALID;
        }
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_argument(struct ly_ctx *ctx, const char **data, const char **argument, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;

    if (*argument) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_DUPSTMT, "argument");
        return LY_EVALID;
    }

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        *argument = lydict_insert_zc(ctx, word);
    } else {
        *argument = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_YIN_ELEMENT:
            ret = parse_yinelement(ctx, data, flags, exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_ARGUMENT, 0, exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "argument");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_extension(struct ly_ctx *ctx, const char **data, struct lysp_ext **extensions)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_ext *ex;

    LYSP_ARRAY_NEW_RET(ctx, extensions, ex, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        ex->name = lydict_insert_zc(ctx, word);
    } else {
        ex->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &ex->dsc, Y_STR_ARG, &ex->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &ex->ref, Y_STR_ARG, &ex->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &ex->flags, &ex->exts);
            break;
        case YANG_ARGUMENT:
            ret = parse_argument(ctx, data, &ex->argument, &ex->flags, &ex->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &ex->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "extension");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_deviate(struct ly_ctx *ctx, const char **data, struct lysp_deviate **deviates)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len, dev_mod;
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
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if ((word_len == 13) && !strncmp(word, "not-supported", word_len)) {
        dev_mod = LYS_DEV_NOT_SUPPORTED;
    } else if ((word_len == 3) && !strncmp(word, "add", word_len)) {
        dev_mod = LYS_DEV_ADD;
    } else if ((word_len == 7) && !strncmp(word, "replace", word_len)) {
        dev_mod = LYS_DEV_REPLACE;
    } else if ((word_len == 6) && !strncmp(word, "delete", word_len)) {
        dev_mod = LYS_DEV_DELETE;
    } else {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, word_len, word, "deviate");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    /* create structure */
    switch (dev_mod) {
    case LYS_DEV_NOT_SUPPORTED:
        d = calloc(1, sizeof *d);
        LY_CHECK_ERR_RET(!d, LOGMEM(ctx), LY_EMEM);
        break;
    case LYS_DEV_ADD:
        d_add = calloc(1, sizeof *d_add);
        LY_CHECK_ERR_RET(!d_add, LOGMEM(ctx), LY_EMEM);
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
        LY_CHECK_ERR_RET(!d_rpl, LOGMEM(ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_rpl;
        d_units = &d_rpl->units;
        d_flags = &d_rpl->flags;
        d_min = &d_rpl->min;
        d_max = &d_rpl->max;
        break;
    case LYS_DEV_DELETE:
        d_del = calloc(1, sizeof *d_del);
        LY_CHECK_ERR_RET(!d_del, LOGMEM(ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_del;
        d_units = &d_del->units;
        d_uniques = &d_del->uniques;
        d_dflts = &d_del->dflts;
        d_musts = &d_del->musts;
        d_flags = &d_del->flags;
        d_min = &d_del->min;
        d_max = &d_del->max;
        break;
    default:
        assert(0);
        LOGINT_RET(ctx);
    }
    d->mod = dev_mod;

    /* insert into siblings */
    if (!*deviates) {
        *deviates = d;
    } else {
        for (iter = *deviates; iter->next; iter = iter->next);
        iter->next = d;
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_CONFIG:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                ret = parse_config(ctx, data, d_flags, &d->exts);
                break;
            }
            break;
        case YANG_DEFAULT:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            case LYS_DEV_REPLACE:
                ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DEFAULT, 0, &d_rpl->dflt, Y_STR_ARG, &d->exts);
                break;
            default:
                ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_DEFAULT, d_dflts, Y_STR_ARG, &d->exts);
                break;
            }
            break;
        case YANG_MANDATORY:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                ret = parse_mandatory(ctx, data, d_flags, &d->exts);
                break;
            }
            break;
        case YANG_MAX_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                ret = parse_maxelements(ctx, data, d_max, d_flags, &d->exts);
                break;
            }
            break;
        case YANG_MIN_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                ret = parse_minelements(ctx, data, d_min, d_flags, &d->exts);
                break;
            }
            break;
        case YANG_MUST:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                ret = parse_restrs(ctx, data, kw, d_musts);
                break;
            }
            break;
        case YANG_TYPE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_ADD:
            case LYS_DEV_DELETE:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                d_rpl->type = calloc(1, sizeof *d_rpl->type);
                LY_CHECK_ERR_RET(!d_rpl->type, LOGMEM(ctx), LY_EMEM);
                ret = parse_type(ctx, data, d_rpl->type);
                break;
            }
            break;
        case YANG_UNIQUE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_REPLACE:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_UNIQUE, d_uniques, Y_STR_ARG, &d->exts);
                break;
            }
            break;
        case YANG_UNITS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INDEV, devmod2str(dev_mod), kw2str(kw));
                return LY_EVALID;
            default:
                ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_UNITS, 0, d_units, Y_STR_ARG, &d->exts);
                break;
            }
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &d->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "deviate");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_deviation(struct ly_ctx *ctx, const char **data, struct lysp_deviation **deviations)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_deviation *dev;

    LYSP_ARRAY_NEW_RET(ctx, deviations, dev, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        dev->nodeid = lydict_insert_zc(ctx, word);
    } else {
        dev->nodeid = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &dev->dsc, Y_STR_ARG, &dev->exts);
            break;
        case YANG_DEVIATE:
            ret = parse_deviate(ctx, data, &dev->deviates);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &dev->ref, Y_STR_ARG, &dev->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &dev->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "deviation");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    /* mandatory substatements */
    if (!dev->deviates) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "deviate", "deviation");
        return LY_EVALID;
    }

    return ret;
}

static LY_ERR
parse_feature(struct ly_ctx *ctx, const char **data, struct lysp_feature **features)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_feature *feat;

    LYSP_ARRAY_NEW_RET(ctx, features, feat, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        feat->name = lydict_insert_zc(ctx, word);
    } else {
        feat->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &feat->dsc, Y_STR_ARG, &feat->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &feat->iffeatures, Y_STR_ARG, &feat->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &feat->ref, Y_STR_ARG, &feat->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &feat->flags, &feat->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &feat->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "feature");
            return LY_EMEM;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_identity(struct ly_ctx *ctx, const char **data, struct lysp_ident **identities)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw;
    struct lysp_ident *ident;

    LYSP_ARRAY_NEW_RET(ctx, identities, ident, LY_EMEM);

    /* get value */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        ident->name = lydict_insert_zc(ctx, word);
    } else {
        ident->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &ident->dsc, Y_STR_ARG, &ident->exts);
            break;
        case YANG_IF_FEATURE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_IFFEATURE, &ident->iffeatures, Y_STR_ARG, &ident->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &ident->ref, Y_STR_ARG, &ident->exts);
            break;
        case YANG_STATUS:
            ret = parse_status(ctx, data, &ident->flags, &ident->exts);
            break;
        case YANG_BASE:
            ret = parse_text_fields(ctx, data, LYEXT_SUBSTMT_BASE, &ident->bases, Y_PREF_IDENTIF_ARG, &ident->exts);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &ident->exts);
            break;
        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "identity");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    return ret;
}

static LY_ERR
parse_sub_module(struct ly_ctx *ctx, const char **data, struct lysp_module *mod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    int word_len;
    enum yang_keyword kw, prev_kw = 0;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;

    /* (sub)module name */
    ret = get_string(ctx, data, Y_IDENTIF_ARG, &word, &buf, &word_len);
    YANG_ERR_RET(ret);

    if (buf) {
        mod->name = lydict_insert_zc(ctx, word);
    } else {
        mod->name = lydict_insert(ctx, word, word_len);
    }

    YANG_READ_SUBSTMT_FOR(ctx, data, kw, word, word_len, ret) {
        YANG_ERR_RET(ret);

        switch (kw) {
        /* module header */
        case YANG_NAMESPACE:
        case YANG_PREFIX:
            if (mod->submodule) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "submodule");
                return LY_EVALID;
            }
            /* fallthrough */
        case YANG_BELONGS_TO:
            if ((kw == YANG_BELONGS_TO) && !mod->submodule) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), "module");
                return LY_EVALID;
            }
            /* fallthrough */
        case YANG_YANG_VERSION:
            if (mod_stmt > Y_MOD_MODULE_HEADER) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INORD, kw2str(kw), kw2str(prev_kw));
                return LY_EVALID;
            }
            break;
        /* linkage */
        case YANG_INCLUDE:
        case YANG_IMPORT:
            if (mod_stmt > Y_MOD_LINKAGE) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INORD, kw2str(kw), kw2str(prev_kw));
                return LY_EVALID;
            }
            mod_stmt = Y_MOD_LINKAGE;
            break;
        /* meta */
        case YANG_ORGANIZATION:
        case YANG_CONTACT:
        case YANG_DESCRIPTION:
        case YANG_REFERENCE:
            if (mod_stmt > Y_MOD_META) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INORD, kw2str(kw), kw2str(prev_kw));
                return LY_EVALID;
            }
            mod_stmt = Y_MOD_META;
            break;

        /* revision */
        case YANG_REVISION:
            if (mod_stmt > Y_MOD_REVISION) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INORD, kw2str(kw), kw2str(prev_kw));
                return LY_EVALID;
            }
            mod_stmt = Y_MOD_REVISION;
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
        prev_kw = kw;

        switch (kw) {
        /* module header */
        case YANG_YANG_VERSION:
            ret = parse_yangversion(ctx, data, mod);
            break;
        case YANG_NAMESPACE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_NAMESPACE, 0, &mod->ns, Y_STR_ARG, &mod->exts);
            break;
        case YANG_PREFIX:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_PREFIX, 0, &mod->prefix, Y_IDENTIF_ARG, &mod->exts);
            break;
        case YANG_BELONGS_TO:
            ret = parse_belongsto(ctx, data, &mod->belongsto, &mod->prefix, &mod->exts);
            break;

        /* linkage */
        case YANG_INCLUDE:
            ret = parse_include(ctx, data, &mod->includes);
            break;
        case YANG_IMPORT:
            ret = parse_import(ctx, data, &mod->imports);
            break;

        /* meta */
        case YANG_ORGANIZATION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_ORGANIZATION, 0, &mod->org, Y_STR_ARG, &mod->exts);
            break;
        case YANG_CONTACT:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_CONTACT, 0, &mod->contact, Y_STR_ARG, &mod->exts);
            break;
        case YANG_DESCRIPTION:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_DESCRIPTION, 0, &mod->dsc, Y_STR_ARG, &mod->exts);
            break;
        case YANG_REFERENCE:
            ret = parse_text_field(ctx, data, LYEXT_SUBSTMT_REFERENCE, 0, &mod->ref, Y_STR_ARG, &mod->exts);
            break;

        /* revision */
        case YANG_REVISION:
            ret = parse_revision(ctx, data, &mod->revs);
            break;

        /* body */
        case YANG_ANYDATA:
        case YANG_ANYXML:
            ret = parse_any(ctx, data, kw, &mod->data);
            break;
        case YANG_CHOICE:
            ret = parse_choice(ctx, data, &mod->data);
            break;
        case YANG_CONTAINER:
            ret = parse_container(ctx, data, &mod->data);
            break;
        case YANG_LEAF:
            ret = parse_leaf(ctx, data, &mod->data);
            break;
        case YANG_LEAF_LIST:
            ret = parse_leaflist(ctx, data, &mod->data);
            break;
        case YANG_LIST:
            ret = parse_list(ctx, data, &mod->data);
            break;
        case YANG_USES:
            ret = parse_uses(ctx, data, &mod->data);
            break;

        case YANG_AUGMENT:
            ret = parse_augment(ctx, data, &mod->augments);
            break;
        case YANG_DEVIATION:
            ret = parse_deviation(ctx, data, &mod->deviations);
            break;
        case YANG_EXTENSION:
            ret = parse_extension(ctx, data, &mod->extensions);
            break;
        case YANG_FEATURE:
            ret = parse_feature(ctx, data, &mod->features);
            break;
        case YANG_GROUPING:
            ret = parse_grouping(ctx, data, &mod->groupings);
            break;
        case YANG_IDENTITY:
            ret = parse_identity(ctx, data, &mod->identities);
            break;
        case YANG_NOTIFICATION:
            ret = parse_notif(ctx, data, &mod->notifs);
            break;
        case YANG_RPC:
            ret = parse_action(ctx, data, &mod->rpcs);
            break;
        case YANG_TYPEDEF:
            ret = parse_typedef(ctx, data, &mod->typedefs);
            break;
        case YANG_CUSTOM:
            ret = parse_ext(ctx, data, word, word_len, LYEXT_SUBSTMT_SELF, 0, &mod->exts);
            break;

        default:
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INCHILDSTMT, kw2str(kw), mod->submodule ? "submodule" : "module");
            return LY_EVALID;
        }
        YANG_ERR_RET(ret);
    }
    YANG_ERR_RET(ret);

    /* mandatory substatements */
    if (mod->submodule) {
        if (!mod->belongsto) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "belongs-to", "submodule");
            return LY_EVALID;
        }
    } else {
        if (!mod->ns) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "namespace", "module");
            return LY_EVALID;
        } else if (!mod->prefix) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_MISSTMT, "prefix", "module");
            return LY_EVALID;
        }
    }

    return ret;
}

LY_ERR
yang_parse(struct ly_ctx *ctx, const char *data, struct lysp_module **mod_p)
{
    LY_ERR ret = 0;
    char *word, *buf;
    int word_len;
    const char *data_start;
    enum yang_keyword kw;
    struct lysp_module *mod;

    data_start = data;

    /* "module"/"submodule" */
    ret = get_keyword(ctx, &data, &kw, &word, &word_len);
    YANG_ERR_GOTO(ret, error);

    if ((kw != YANG_MODULE) && (kw != YANG_SUBMODULE)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".\n",
               kw2str(kw));
        goto error;
    }

    mod = calloc(1, sizeof *mod);
    LY_CHECK_ERR_GOTO(!mod, LOGMEM(ctx), error);
    if (kw == YANG_SUBMODULE) {
        mod->submodule = 1;
    }

    /* substatements */
    ret = parse_sub_module(ctx, &data, mod);
    YANG_ERR_GOTO(ret, error);

    /* read some trailing spaces or new lines */
    ret = get_string(ctx, &data, Y_MAYBE_STR_ARG, &word, &buf, &word_len);
    YANG_ERR_GOTO(ret, error);

    if (word) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_SYNTAX, "Invalid character sequence \"%.*s\", expected end-of-file.",
               word_len, word);
        free(buf);
        goto error;
    }
    assert(!buf);

    *mod_p = mod;
    return ret;

error:
    LOGERR(ctx, LY_EINVAL, "Module parsing failed on line %d.", lysp_get_data_line(data_start, data - data_start));
    /* TODO free module */
    return ret;
}
