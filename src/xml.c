/**
 * @file xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic XML parser implementation for libyang
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "libyang.h"
#include "xml.h"
#include "common.h"

/* Macro to test if character is whitespace */
#define is_xmlws(c) (c == 0x20 || c == 0x9 || c == 0xa || c == 0xd)

/* Macro to test if character is allowed to be a first character of an qualified identifier */
#define is_xmlqnamestartchar(c) ((c >= 'a' && c <= 'z') || c == '_' || \
        (c >= 'A' && c <= 'Z') || /* c == ':' || */ \
        (c >= 0x370 && c <= 0x1fff && c != 0x37e ) || \
        (c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
        c == 0x200d || (c >= 0x2070 && c <= 0x218f) || \
        (c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
        (c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
        (c >= 0x10000 && c <= 0xeffff))

/* Macro to test if character is allowed to be used in an qualified identifier */
#define is_xmlqnamechar(c) ((c >= 'a' && c <= 'z') || c == '_' || c == '-' || \
        (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || /* c == ':' || */ \
        c == '.' || c == 0xb7 || (c >= 0x370 && c <= 0x1fff && c != 0x37e ) ||\
        (c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
        c == 0x200d || (c >= 0x300 && c <= 0x36f) || \
        (c >= 0x2070 && c <= 0x218f) || (c >= 0x2030f && c <= 0x2040) || \
        (c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
        (c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
        (c >= 0x10000 && c <= 0xeffff))

/* Move input p by s characters, if EOF log with lyxml_context c */
#define move_input(c,p,s) p += s; LY_CHECK_ERR_RET(!p[0], LOGVAL(c->ctx, LY_VLOG_LINE, &c->line, LY_VCODE_EOF), LY_EVALID)

/* Ignore whitespaces in the input string p, if EOF log with lyxml_context c */
#define ign_xmlws(c,p) while (is_xmlws(*(p))) {if (*(p) == '\n') {++c->line;} ++p;}

static const char *
ign_todelim(register const char *input, const char *delim, size_t delim_len, size_t *newlines)
{
    size_t i;
    register const char *a, *b;

    (*newlines) = 0;
    for ( ; *input; ++input) {
        if (*input != *delim) {
            if (*input == '\n') {
                ++(*newlines);
            }
            continue;
        }
        a = input;
        b = delim;
        for (i = 0; i < delim_len; ++i) {
            if (*a++ != *b++) {
                break;
            }
        }
        if (i == delim_len) {
            return input;
        }
    }
    return NULL;
}

static LY_ERR
lyxml_getutf8(const char **input, unsigned int *utf8_char, size_t *bytes_read)
{
    unsigned int c, len;
    int aux;
    int i;

    c = (*input)[0];
    LY_CHECK_RET(!c, LY_EINVAL);

    /* process character byte(s) */
    if ((c & 0xf8) == 0xf0) {
        /* four bytes character */
        len = 4;

        c &= 0x07;
        for (i = 1; i <= 3; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                return LY_EINVAL;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if (c < 0x1000 || c > 0x10ffff) {
            return LY_EINVAL;
        }
    } else if ((c & 0xf0) == 0xe0) {
        /* three bytes character */
        len = 3;

        c &= 0x0f;
        for (i = 1; i <= 2; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                return LY_EINVAL;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if (c < 0x800 || (c > 0xd7ff && c < 0xe000) || c > 0xfffd) {
            return LY_EINVAL;
        }
    } else if ((c & 0xe0) == 0xc0) {
        /* two bytes character */
        len = 2;

        aux = (*input)[1];
        if ((aux & 0xc0) != 0x80) {
            return LY_EINVAL;
        }
        c = ((c & 0x1f) << 6) | (aux & 0x3f);

        if (c < 0x80) {
            return LY_EINVAL;
        }
    } else if (!(c & 0x80)) {
        /* one byte character */
        len = 1;

        if (c < 0x20 && c != 0x9 && c != 0xa && c != 0xd) {
            return LY_EINVAL;
        }
    } else {
        return LY_EINVAL;
    }

    (*utf8_char) = c;
    (*input) += len;
    if (bytes_read) {
        (*bytes_read) = len;
    }
    return LY_SUCCESS;
}

LY_ERR
lyxml_check_qname(struct lyxml_context *context, const char **input, unsigned int *term_char, size_t *term_char_len)
{
    unsigned int c;
    const char *id = (*input);
    LY_ERR rc;

    /* check NameStartChar (minus colon) */
    LY_CHECK_ERR_RET(lyxml_getutf8(input, &c, NULL) != LY_SUCCESS,
                     LOGVAL(context->ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INCHAR, (*input)[0]), LY_EVALID);
    LY_CHECK_ERR_RET(!is_xmlqnamestartchar(c),
                     LOGVAL(context->ctx, LY_VLOG_LINE, &context->line, LYVE_SYNTAX,
                            "Identifier \"%s\" starts with invalid character.", id),
                     LY_EVALID);

    /* check rest of the identifier */
    for (rc = lyxml_getutf8(input, &c, term_char_len);
         rc == LY_SUCCESS && is_xmlqnamechar(c);
         rc = lyxml_getutf8(input, &c, term_char_len));
    LY_CHECK_ERR_RET(rc != LY_SUCCESS, LOGVAL(context->ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INCHAR, (*input)[0]), LY_EVALID);

    (*term_char) = c;
    return LY_SUCCESS;
}

LY_ERR
lyxml_get_element(struct lyxml_context *context, const char **input, int UNUSED(options),
                  const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    struct ly_ctx *ctx = context->ctx; /* shortcut */
    const char *in = (*input);
    const char *endtag;
    const char *sectname;
    const char *id;
    size_t endtag_len, newlines;
    bool loop = true;
    unsigned int c;
    LY_ERR rc;
    uint32_t x;

    /* initialize output variables */
    (*prefix) = (*name) = NULL;
    (*prefix_len) = (*name_len) = 0;

    while (loop) {
        ign_xmlws(context, in);

        if (in[0] == '\0') {
            /* EOF */
            goto success;
        } else if (in[0] != '<') {
            return LY_EINVAL;
        }
        move_input(context, in, 1);

        if (in[0] == '!') {
            move_input(context, in, 1);
            /* sections to ignore */
            if (!strncmp(in, "--", 2)) {
                /* comment */
                move_input(context, in, 2);
                sectname = "Comment";
                endtag = "-->";
                endtag_len = 3;
            } else if (!strncmp(in, "[CDATA[", 7)) {
                /* CDATA section */
                move_input(context, in, 7);
                sectname = "CData";
                endtag = "]]>";
                endtag_len = 3;
            } else if (!strncmp(in, "DOCTYPE", 7)) {
                /* Document type declaration - not supported */
                LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_NSUPP, "Document Type Declaration");
                return LY_EVALID;
            }
            in = ign_todelim(in, endtag, endtag_len, &newlines);
            LY_CHECK_ERR_RET(!in, LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_NTERM, sectname), LY_EVALID);
            context->line += newlines;
            in += endtag_len;
        } else if (in[0] == '?') {
            in = ign_todelim(in, "?>", 2, &newlines);
            LY_CHECK_ERR_RET(!in, LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_NTERM, "Declaration"), LY_EVALID);
            context->line += newlines;
            in += 2;
        } else {
            /* element */
            ign_xmlws(context, in);
            LY_CHECK_ERR_RET(!in[0], LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_EOF), LY_EVALID);

            /* remember the identifier start before checking its format */
            id = in;
            rc = lyxml_check_qname(context, &in, &c, &endtag_len);
            LY_CHECK_RET(rc);
            if (c == ':') {
                /* we have prefixed identifier */
                endtag = in - endtag_len;

                rc = lyxml_check_qname(context, &in, &c, &endtag_len);
                LY_CHECK_RET(rc);

                (*prefix) = id;
                (*prefix_len) = endtag - id;
                id = endtag + 1;
            }
            if (!is_xmlws(c) && c != '/' && c != '>') {
                in = in - endtag_len;
                x = 0;
                memcpy(&x, in, endtag_len);
                LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INCHAR, x);
                return LY_EVALID;
            }
            in = in - endtag_len;
            (*name) = id;
            (*name_len) = in - id;

            loop = false;
        }
    }

success:
    /* move caller's input */
    (*input) = in;
    return LY_SUCCESS;
}

