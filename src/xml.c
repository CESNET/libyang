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

#include <ctype.h>
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

/**
 * Store UTF-8 character specified as 4byte integer into the dst buffer.
 * Returns number of written bytes (4 max), expects that dst has enough space.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F:    0xxxxxxx
 * 00000080 -- 000007FF:    110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF:    1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF:    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * Includes checking for valid characters (following RFC 7950, sec 9.4)
 */
static LY_ERR
lyxml_pututf8(char *dst, int32_t value, size_t *bytes_written)
{
    if (value < 0x80) {
        /* one byte character */
        if (value < 0x20 &&
                value != 0x09 &&
                value != 0x0a &&
                value != 0x0d) {
            return LY_EINVAL;
        }

        dst[0] = value;
        (*bytes_written) = 1;
    } else if (value < 0x800) {
        /* two bytes character */
        dst[0] = 0xc0 | (value >> 6);
        dst[1] = 0x80 | (value & 0x3f);
        (*bytes_written) = 2;
    } else if (value < 0xfffe) {
        /* three bytes character */
        if (((value & 0xf800) == 0xd800) ||
                (value >= 0xfdd0 && value <= 0xfdef)) {
            /* exclude surrogate blocks %xD800-DFFF */
            /* exclude noncharacters %xFDD0-FDEF */
            return LY_EINVAL;
        }

        dst[0] = 0xe0 | (value >> 12);
        dst[1] = 0x80 | ((value >> 6) & 0x3f);
        dst[2] = 0x80 | (value & 0x3f);

        (*bytes_written) = 3;
    } else if (value < 0x10fffe) {
        if ((value & 0xffe) == 0xffe) {
            /* exclude noncharacters %xFFFE-FFFF, %x1FFFE-1FFFF, %x2FFFE-2FFFF, %x3FFFE-3FFFF, %x4FFFE-4FFFF,
             * %x5FFFE-5FFFF, %x6FFFE-6FFFF, %x7FFFE-7FFFF, %x8FFFE-8FFFF, %x9FFFE-9FFFF, %xAFFFE-AFFFF,
             * %xBFFFE-BFFFF, %xCFFFE-CFFFF, %xDFFFE-DFFFF, %xEFFFE-EFFFF, %xFFFFE-FFFFF, %x10FFFE-10FFFF */
            return LY_EINVAL;
        }
        /* four bytes character */
        dst[0] = 0xf0 | (value >> 18);
        dst[1] = 0x80 | ((value >> 12) & 0x3f);
        dst[2] = 0x80 | ((value >> 6) & 0x3f);
        dst[3] = 0x80 | (value & 0x3f);

        (*bytes_written) = 4;
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

/**
 * @brief Parse input as XML text (attribute's values and element's content).
 *
 * Mixed content of XML elements is not allowed. Formating whitespaces before child element are ignored,
 * LY_EINVAL is returned in such a case (buffer is not filled, no error is printed) and input is moved
 * to the beginning of a child definition.
 *
 * In the case of attribute's values, the input string is expected to start on a quotation mark to
 * select which delimiter (single or double quote) is used. Otherwise, the element content is being
 * parsed expected to be terminated by '<' character.
 *
 * If function succeeds, the string in output buffer is always NULL-terminated.
 *
 * @param[in] context XML context to track lines or store errors into libyang context.
 * @param[in,out] input Input string to process, updated according to the processed/read data.
 * @param[out] buffer Storage of the output string. If NULL, the buffer is allocated. Otherwise, the buffer
 * is used and enlarged when necessary.
 * @param[out] buffer_size Allocated size of the returned buffer. If a buffer is provided by a caller, it
 * is not being reduced even if the string is shorter. On the other hand, it can be enlarged if needed.
 * @return LY_ERR value.
 */
LY_ERR
lyxml_get_string(struct lyxml_context *context, const char **input, char **buffer, size_t *buffer_size)
{
#define BUFSIZE 4096
#define BUFSIZE_STEP 4096
#define BUFSIZE_CHECK(CTX, BUF, SIZE, CURR, NEED) \
    if (CURR+NEED >= SIZE) { \
        BUF = ly_realloc(BUF, SIZE + BUFSIZE_STEP); \
        LY_CHECK_ERR_RET(!BUF, LOGMEM(CTX), LY_EMEM); \
        SIZE += BUFSIZE_STEP; \
    }

    struct ly_ctx *ctx = context->ctx; /* shortcut */
    const char *in = (*input);
    char *buf, delim;
    size_t offset;  /* read offset in input buffer */
    size_t len;     /* write offset in output buffer */
    size_t size;    /* size of the output buffer */
    void *p;
    int32_t n;
    size_t u;
    bool empty_content = false;
    LY_ERR rc;

    if (in[0] == '\'') {
        delim = '\'';
        ++in;
    } else if (in[0] == '"') {
        delim = '"';
        ++in;
    } else {
        delim = '<';
        empty_content = true;
    }

    if (empty_content) {
        /* only when processing element's content - try to ignore whitespaces used to format XML data
         * before element's child or closing tag */
        for (offset = 0; in[offset] && is_xmlws(in[offset]); ++offset);
        LY_CHECK_ERR_RET(!in[offset], LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_EOF), LY_EVALID);
        if (in[offset] == '<') {
            (*input) = in + offset;
            return LY_EINVAL;
        }
    } else {
        /* init */
        offset = 0;
    }

    /* prepare output buffer */
    if (*buffer) {
        buf = *buffer;
        size = *buffer_size;
    } else {
        buf = malloc(BUFSIZE);
        size = BUFSIZE;

        LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
    }
    len = 0;

    /* parse */
    while (in[offset]) {
        if (in[offset] == '&') {
            if (offset) {
                /* store what we have so far */
                BUFSIZE_CHECK(ctx, buf, size, len, offset);
                memcpy(&buf[len], in, offset);
                len += offset;
                in += offset;
                offset = 0;
            }
            /* process reference */
            /* we will need 4 bytes at most since we support only the predefined
             * (one-char) entities and character references */
            BUFSIZE_CHECK(ctx, buf, size, len, 4);
            ++offset;
            if (in[offset] != '#') {
                /* entity reference - only predefined references are supported */
                if (!strncmp(&in[offset], "lt;", 3)) {
                    buf[len++] = '<';
                    in += 4; /* &lt; */
                } else if (!strncmp(&in[offset], "gt;", 3)) {
                    buf[len++] = '>';
                    in += 4; /* &gt; */
                } else if (!strncmp(&in[offset], "amp;", 4)) {
                    buf[len++] = '&';
                    in += 5; /* &amp; */
                } else if (!strncmp(&in[offset], "apos;", 5)) {
                    buf[len++] = '\'';
                    in += 6; /* &apos; */
                } else if (!strncmp(&in[offset], "quot;", 5)) {
                    buf[len++] = '\"';
                    in += 6; /* &quot; */
                } else {
                    LOGVAL(ctx, LY_VLOG_LINE, &context->line, LYVE_SYNTAX,
                           "Entity reference \"%.*s\" not supported, only predefined references allowed.", 10, &in[offset-1]);
                    goto error;
                }
                offset = 0;
            } else {
                p = (void*)&in[offset - 1];
                /* character reference */
                ++offset;
                if (isdigit(in[offset])) {
                    for (n = 0; isdigit(in[offset]); offset++) {
                        n = (10 * n) + (in[offset] - '0');
                    }
                } else if (in[offset] == 'x' && isxdigit(in[offset + 1])) {
                    for (n = 0, ++offset; isxdigit(in[offset]); offset++) {
                        if (isdigit(in[offset])) {
                            u = (in[offset] - '0');
                        } else if (in[offset] > 'F') {
                            u = 10 + (in[offset] - 'a');
                        } else {
                            u = 10 + (in[offset] - 'A');
                        }
                        n = (16 * n) + u;
                    }
                } else {
                    LOGVAL(ctx, LY_VLOG_LINE, &context->line, LYVE_SYNTAX, "Invalid character reference \"%.*s\".", 12, p);
                    goto error;

                }
                LY_CHECK_ERR_GOTO(in[offset] != ';',
                                  LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INSTREXP,
                                         LY_VCODE_INSTREXP_len(&in[offset]), &in[offset], ";"),
                                  error);
                ++offset;
                rc = lyxml_pututf8(&buf[len], n, &u);
                LY_CHECK_ERR_GOTO(rc, LOGVAL(ctx, LY_VLOG_LINE, &context->line, LYVE_SYNTAX,
                                             "Invalid character reference %.*s (0x%08x).", 12, p, n),
                                  error);
                len += u;
                in += offset;
                offset = 0;
            }
        } else if (in[offset] == delim) {
            /* end of string */
            if (len + offset >= size) {
                buf = ly_realloc(buf, len + offset + 1);
                LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
                size = len + offset + 1;
            }
            memcpy(&buf[len], in, offset);
            len += offset;
            /* in case of element content, keep the leading <,
             * for attribute's value mova after the terminating quotation mark */
            if (delim == '<') {
                in += offset;
            } else {
                in += offset + 1;
            }
            goto success;
        } else {
            /* log lines */
            if (in[offset] == '\n') {
                ++context->line;
            }

            /* continue */
            ++offset;
        }
    }
    LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_EOF);
error:
    if (!(*buffer)) {
        free(buf);
    }
    return LY_EVALID;

success:
    if (!(*buffer) && size != len + 1) {
        /* not using provided buffer, so fit the allocated buffer to what we really have inside */
        p = realloc(buf, len + 1);
        /* ignore realloc fail because we are reducing the buffer,
         * so just return bigger buffer than needed */
        if (p) {
            size = len + 1;
            buf = p;
        }
    }
    /* set terminating NULL byte */
    buf[len] = '\0';

    (*input) = in;
    (*buffer) = buf;
    (*buffer_size) = size;
    return LY_SUCCESS;

#undef BUFSIZE
#undef BUFSIZE_STEP
#undef BUFSIZE_CHECK
}

/**
 * @brief Parse input expecting an XML attribute (including XML namespace).
 *
 * Input string is not being modified, so the returned values are not NULL-terminated, instead their length
 * is returned.
 *
 * In case of a namespace definition, prefix just contains xmlns string. In case of the default namespace,
 * prefix is NULL and the attribute name is xmlns.
 *
 * @param[in] context XML context to track lines or store errors into libyang context.
 * @param[in,out] input Input string to process, updated according to the processed/read data so,
 * when succeeded, it points to the opening quote of the attribute's value.
 * @param[out] prefix Pointer to prefix if present in the attribute name, NULL otherwise.
 * @param[out] prefix_len Length of the prefix if any.
 * @param[out] name Attribute name. LY_SUCCESS can be returned with NULL name only in case the
 * end of the element tag was reached.
 * @param[out] name_len Length of the element name.
 * @return LY_ERR values.
 */
LY_ERR
lyxml_get_attribute(struct lyxml_context *context, const char **input,
                    const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    struct ly_ctx *ctx = context->ctx; /* shortcut */
    const char *in = (*input);
    const char *id;
    const char *endtag;
    LY_ERR rc;
    unsigned int c;
    size_t endtag_len;

    /* initialize output variables */
    (*prefix) = (*name) = NULL;
    (*prefix_len) = (*name_len) = 0;

    /* skip initial whitespaces */
    ign_xmlws(context, in);

    if (in[0] == '\0') {
        /* EOF - not expected at this place */
        return LY_EINVAL;
    } else if (in[0] == '>' || in[0] == '/') {
        /* element terminated by > or /> */
        goto success;
    }

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
    if (!is_xmlws(c) && c != '=') {
        in = in - endtag_len;
        LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(in), in, "whitespace or '='");
        return LY_EVALID;
    }
    in = in - endtag_len;
    (*name) = id;
    (*name_len) = in - id;

    /* eat '=' and stop at the value beginning */
    ign_xmlws(context, in);
    if (in[0] != '=') {
        LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(in), in, "'='");
        return LY_EVALID;
    }
    ++in;
    ign_xmlws(context, in);
    if (in[0] != '\'' && in[0] != '"') {
        LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(in), in, "either single or double quotation mark");
        return LY_EVALID;
    }

success:
    /* move caller's input */
    (*input) = in;
    return LY_SUCCESS;
}

/**
 * @brief Parse input expecting an XML element.
 *
 * Able to silently skip comments, PIs and CData. DOCTYPE is not parsable, so it is reported as LY_EVALID error.
 * If '<' is not found in input, LY_EINVAL is returned (but no error is logged), so it is possible to continue
 * with parsing input as text content.
 *
 * Input string is not being modified, so the returned values are not NULL-terminated, instead their length
 * is returned.
 *
 * @param[in] context XML context to track lines or store errors into libyang context.
 * @param[in,out] input Input string to process, updated according to the processed/read data.
 * @param[in] options Currently unused options to modify input processing.
 * @param[out] prefix Pointer to prefix if present in the element name, NULL otherwise.
 * @param[out] prefix_len Length of the prefix if any.
 * @param[out] name Element name. LY_SUCCESS can be returned with NULL name only in case the
 * end of the input string was reached (EOF).
 * @param[out] name_len Length of the element name.
 * @return LY_ERR values.
 */
LY_ERR
lyxml_get_element(struct lyxml_context *context, const char **input,
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
                LOGVAL(ctx, LY_VLOG_LINE, &context->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(in), in,
                       "whitespace or element tag termination ('>' or '/>'");
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

