/**
 * @file xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
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

#define _GNU_SOURCE

#include "xml.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "compat.h"
#include "dict.h"
#include "parser_internal.h"
#include "printer_internal.h"
#include "tree.h"
#include "tree_data.h"

/* Move input p by s characters, if EOF log with lyxml_ctx c */
#define move_input(c,s) ly_in_skip(c->in, s); LY_CHECK_ERR_RET(!c->in->current[0], LOGVAL(c->ctx, LY_VLOG_LINE, &c->line, LY_VCODE_EOF), LY_EVALID)

/* Ignore whitespaces in the input string p */
#define ign_xmlws(c) while (is_xmlws(*(c)->in->current)) {if (*(c)->in->current == '\n') {++c->line;} ly_in_skip(c->in, 1);}

static LY_ERR lyxml_next_attr_content(struct lyxml_ctx *xmlctx, const char **value, size_t *value_len, int *ws_only,
                                      int *dynamic);

/**
 * @brief Ignore any characters until the delim of the size delim_len is read
 *
 * Detects number of read new lines.
 * Returns 0 if delim was found, non-zero if was not.
 */
static int
ign_todelim(register const char *input, const char *delim, size_t delim_len, size_t *newlines, size_t *parsed)
{
    size_t i;
    register const char *a, *b;

    (*newlines) = 0;
    (*parsed) = 0;
    for ( ; *input; ++input, ++(*parsed)) {
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
            /* delim found */
            return 0;
        }
    }

    /* delim not found */
    return -1;
}

/**
 * @brief Check/Get an XML identifier from the input string.
 *
 * The identifier must have at least one valid character complying the name start character constraints.
 * The identifier is terminated by the first character, which does not comply to the name character constraints.
 *
 * See https://www.w3.org/TR/xml-names/#NT-NCName
 *
 * @param[in] xmlctx XML context.
 * @param[out] start Pointer to the start of the identifier.
 * @param[out] end Pointer ot the end of the identifier.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_parse_identifier(struct lyxml_ctx *xmlctx, const char **start, const char **end)
{
    const char *s, *in;
    uint32_t c;
    size_t parsed;
    LY_ERR rc;

    in = s = xmlctx->in->current;

    /* check NameStartChar (minus colon) */
    LY_CHECK_ERR_RET(ly_getutf8(&in, &c, &parsed),
                     LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INCHAR, in[0]),
                     LY_EVALID);
    LY_CHECK_ERR_RET(!is_xmlqnamestartchar(c),
                     LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX,
                            "Identifier \"%s\" starts with an invalid character.", in - parsed),
                     LY_EVALID);

    /* check rest of the identifier */
    do {
        /* move only successfully parsed bytes */
        ly_in_skip(xmlctx->in, parsed);

        rc = ly_getutf8(&in, &c, &parsed);
        LY_CHECK_ERR_RET(rc, LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INCHAR, in[0]), LY_EVALID);
    } while (is_xmlqnamechar(c));

    *start = s;
    *end = xmlctx->in->current;
    return LY_SUCCESS;
}

/**
 * @brief Add namespace definition into XML context.
 *
 * Namespaces from a single element are supposed to be added sequentially together (not interleaved by a namespace from other
 * element). This mimic namespace visibility, since the namespace defined in element E is not visible from its parents or
 * siblings. On the other hand, namespace from a parent element can be redefined in a child element. This is also reflected
 * by lyxml_ns_get() which returns the most recent namespace definition for the given prefix.
 *
 * When leaving processing of a subtree of some element (after it is removed from xmlctx->elements), caller is supposed to call
 * lyxml_ns_rm() to remove all the namespaces defined in such an element from the context.
 *
 * @param[in] xmlctx XML context to work with.
 * @param[in] prefix Pointer to the namespace prefix. Can be NULL for default namespace.
 * @param[in] prefix_len Length of the prefix.
 * @param[in] uri Namespace URI (value) to store directly. Value is always spent.
 * @return LY_ERR values.
 */
LY_ERR
lyxml_ns_add(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len, char *uri)
{
    struct lyxml_ns *ns;

    ns = malloc(sizeof *ns);
    LY_CHECK_ERR_RET(!ns, LOGMEM(xmlctx->ctx), LY_EMEM);

    /* we need to connect the depth of the element where the namespace is defined with the
     * namespace record to be able to maintain (remove) the record when the parser leaves
     * (to its sibling or back to the parent) the element where the namespace was defined */
    ns->depth = xmlctx->elements.count;

    ns->uri = uri;
    if (prefix) {
        ns->prefix = strndup(prefix, prefix_len);
        LY_CHECK_ERR_RET(!ns->prefix, LOGMEM(xmlctx->ctx); free(ns->uri); free(ns), LY_EMEM);
    } else {
        ns->prefix = NULL;
    }

    LY_CHECK_ERR_RET(ly_set_add(&xmlctx->ns, ns, LY_SET_OPT_USEASLIST) == -1,
                     free(ns->prefix); free(ns->uri); free(ns), LY_EMEM);
    return LY_SUCCESS;
}

/**
 * @brief Remove all the namespaces defined in the element recently closed (removed from the xmlctx->elements).
 *
 * @param[in] xmlctx XML context to work with.
 */
void
lyxml_ns_rm(struct lyxml_ctx *xmlctx)
{
    unsigned int u;

    for (u = xmlctx->ns.count - 1; u + 1 > 0; --u) {
        if (((struct lyxml_ns *)xmlctx->ns.objs[u])->depth != xmlctx->elements.count + 1) {
            /* we are done, the namespaces from a single element are supposed to be together */
            break;
        }
        /* remove the ns structure */
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->prefix);
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->uri);
        free(xmlctx->ns.objs[u]);
        --xmlctx->ns.count;
    }

    if (!xmlctx->ns.count) {
        /* cleanup the xmlctx's namespaces storage */
        ly_set_erase(&xmlctx->ns, NULL);
    }
}

const struct lyxml_ns *
lyxml_ns_get(const struct ly_set *ns_set, const char *prefix, size_t prefix_len)
{
    unsigned int u;
    struct lyxml_ns *ns;

    for (u = ns_set->count - 1; u + 1 > 0; --u) {
        ns = (struct lyxml_ns *)ns_set->objs[u];
        if (prefix && prefix_len) {
            if (ns->prefix && !ly_strncmp(ns->prefix, prefix, prefix_len)) {
                return ns;
            }
        } else if (!ns->prefix) {
            /* default namespace */
            return ns;
        }
    }

    return NULL;
}

/**
 * @brief Skip in the input until EOF or just after the opening tag.
 * Handles special XML constructs (comment, cdata, doctype).
 *
 * @param[in] xmlctx XML context to use.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_skip_until_end_or_after_otag(struct lyxml_ctx *xmlctx)
{
    const struct ly_ctx *ctx = xmlctx->ctx; /* shortcut */
    const char *endtag, *sectname;
    size_t endtag_len, newlines, parsed;
    int rc;

    while (1) {
        ign_xmlws(xmlctx);

        if (xmlctx->in->current[0] == '\0') {
            /* EOF */
            if (xmlctx->elements.count) {
                LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_EOF);
                return LY_EVALID;
            }
            return LY_SUCCESS;
        } else if (xmlctx->in->current[0] != '<') {
            LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
                   xmlctx->in->current, "element tag start ('<')");
            return LY_EVALID;
        }
        move_input(xmlctx, 1);

        if (xmlctx->in->current[0] == '!') {
            move_input(xmlctx, 1);
            /* sections to ignore */
            if (!strncmp(xmlctx->in->current, "--", 2)) {
                /* comment */
                move_input(xmlctx, 2);
                sectname = "Comment";
                endtag = "-->";
                endtag_len = 3;
            } else if (!strncmp(xmlctx->in->current, "[CDATA[", 7)) {
                /* CDATA section */
                move_input(xmlctx, 7);
                sectname = "CData";
                endtag = "]]>";
                endtag_len = 3;
            } else if (!strncmp(xmlctx->in->current, "DOCTYPE", 7)) {
                /* Document type declaration - not supported */
                LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_NSUPP, "Document Type Declaration");
                return LY_EVALID;
            } else {
                LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Unknown XML section \"%.20s\".",
                       &xmlctx->in->current[-2]);
                return LY_EVALID;
            }
            rc = ign_todelim(xmlctx->in->current, endtag, endtag_len, &newlines, &parsed);
            LY_CHECK_ERR_RET(rc, LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_NTERM, sectname), LY_EVALID);
            xmlctx->line += newlines;
            ly_in_skip(xmlctx->in, parsed + endtag_len);
        } else if (xmlctx->in->current[0] == '?') {
            rc = ign_todelim(xmlctx->in->current, "?>", 2, &newlines, &parsed);
            LY_CHECK_ERR_RET(rc, LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_NTERM, "Declaration"), LY_EVALID);
            xmlctx->line += newlines;
            ly_in_skip(xmlctx->in, parsed + 2);
        } else {
            /* other non-WS character */
            break;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse QName.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] prefix Parsed prefix, may be NULL.
 * @param[out] prefix_len Length of @p prefix.
 * @param[out] name Parsed name.
 * @param[out] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_parse_qname(struct lyxml_ctx *xmlctx, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    const char *start, *end;

    *prefix = NULL;
    *prefix_len = 0;

    LY_CHECK_RET(lyxml_parse_identifier(xmlctx, &start, &end));
    if (end[0] == ':') {
        /* we have prefixed identifier */
        *prefix = start;
        *prefix_len = end - start;

        move_input(xmlctx, 1);
        LY_CHECK_RET(lyxml_parse_identifier(xmlctx, &start, &end));
    }

    *name = start;
    *name_len = end - start;
    return LY_SUCCESS;
}

/**
 * @brief Parse XML text content (value).
 *
 * @param[in] xmlctx XML context to use.
 * @param[in] endchar Expected character to mark value end.
 * @param[out] value Parsed value.
 * @param[out] length Length of @p value.
 * @param[out] ws_only Whether the value is empty/white-spaces only.
 * @param[out] dynamic Whether the value was dynamically allocated.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_parse_value(struct lyxml_ctx *xmlctx, char endchar, char **value, size_t *length, int *ws_only, int *dynamic)
{
#define BUFSIZE 24
#define BUFSIZE_STEP 128

    const struct ly_ctx *ctx = xmlctx->ctx; /* shortcut */
    const char *in = xmlctx->in->current, *start;
    char *buf = NULL;
    size_t offset;   /* read offset in input buffer */
    size_t len;      /* length of the output string (write offset in output buffer) */
    size_t size = 0; /* size of the output buffer */
    void *p;
    uint32_t n;
    size_t u;
    int ws = 1;

    assert(xmlctx);

    /* init */
    start = in;
    offset = len = 0;

    /* parse */
    while (in[offset]) {
        if (in[offset] == '&') {
            /* non WS */
            ws = 0;

            if (!buf) {
                /* prepare output buffer */
                buf = malloc(BUFSIZE);
                LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
                size = BUFSIZE;
            }

            /* allocate enough for the offset and next character,
             * we will need 4 bytes at most since we support only the predefined
             * (one-char) entities and character references */
            while (len + offset + 4 >= size) {
                buf = ly_realloc(buf, size + BUFSIZE_STEP);
                LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
                size += BUFSIZE_STEP;
            }

            if (offset) {
                /* store what we have so far */
                memcpy(&buf[len], in, offset);
                len += offset;
                in += offset;
                offset = 0;
            }

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
                    LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX,
                           "Entity reference \"%.*s\" not supported, only predefined references allowed.", 10, &in[offset-1]);
                    goto error;
                }
                offset = 0;
            } else {
                p = (void *)&in[offset - 1];
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
                    LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Invalid character reference \"%.*s\".", 12, p);
                    goto error;

                }

                LY_CHECK_ERR_GOTO(in[offset] != ';',
                                  LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INSTREXP,
                                         LY_VCODE_INSTREXP_len(&in[offset]), &in[offset], ";"),
                                  error);
                ++offset;
                LY_CHECK_ERR_GOTO(ly_pututf8(&buf[len], n, &u),
                                  LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX,
                                         "Invalid character reference \"%.*s\" (0x%08x).", 12, p, n),
                                  error);
                len += u;
                in += offset;
                offset = 0;
            }
        } else if (in[offset] == endchar) {
            /* end of string */
            if (buf) {
                /* realloc exact size string */
                buf = ly_realloc(buf, len + offset + 1);
                LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
                size = len + offset + 1;
                memcpy(&buf[len], in, offset);

                /* set terminating NULL byte */
                buf[len + offset] = '\0';
            }
            len += offset;
            in += offset;
            goto success;
        } else {
            if (!is_xmlws(in[offset])) {
                /* non WS */
                ws = 0;
            }

            /* log lines */
            if (in[offset] == '\n') {
                ++xmlctx->line;
            }

            /* continue */
            ++offset;
        }
    }

    /* EOF reached before endchar */
    LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_EOF);

error:
    free(buf);
    return LY_EVALID;

success:
    if (buf) {
        *value = buf;
        *dynamic = 1;
    } else {
        *value = (char *)start;
        *dynamic = 0;
    }
    *length = len;
    *ws_only = ws;

    ly_in_skip(xmlctx->in, in - xmlctx->in->current);
    return LY_SUCCESS;

#undef BUFSIZE
#undef BUFSIZE_STEP
}

/**
 * @brief Parse XML closing element and match it to a stored starting element.
 *
 * @param[in] xmlctx XML context to use.
 * @param[in] prefix Expected closing element prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] name Expected closing element name.
 * @param[in] name_len Length of @p name.
 * @param[in] empty Whether we are parsing a special "empty" element (with joined starting and closing tag) with no value.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_close_element(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len, const char *name, size_t name_len,
                    int empty)
{
    struct lyxml_elem *e;

    /* match opening and closing element tags */
    if (!xmlctx->elements.count) {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Stray closing element tag (\"%.*s\").",
               name_len, name);
        return LY_EVALID;
    }

    e = (struct lyxml_elem *)xmlctx->elements.objs[xmlctx->elements.count - 1];
    if ((e->prefix_len != prefix_len) || (e->name_len != name_len)
            || (prefix_len && strncmp(prefix, e->prefix, e->prefix_len)) || strncmp(name, e->name, e->name_len)) {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX,
               "Opening (\"%.*s%s%.*s\") and closing (\"%.*s%s%.*s\") elements tag mismatch.",
               e->prefix_len, e->prefix ? e->prefix : "", e->prefix ? ":" : "", e->name_len, e->name,
               prefix_len, prefix ? prefix : "", prefix ? ":" : "", name_len, name);
        return LY_EVALID;
    }

    /* opening and closing element tags matches, remove record from the opening tags list */
    ly_set_rm_index(&xmlctx->elements, xmlctx->elements.count - 1, free);

    /* remove also the namespaces connected with the element */
    lyxml_ns_rm(xmlctx);

    /* skip WS */
    ign_xmlws(xmlctx);

    /* special "<elem/>" element */
    if (empty && (xmlctx->in->current[0] == '/')) {
        move_input(xmlctx, 1);
    }

    /* parse closing tag */
    if (xmlctx->in->current[0] != '>') {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
               xmlctx->in->current, "element tag termination ('>')");
        return LY_EVALID;
    }

    /* move after closing tag without checking for EOF */
    ly_in_skip(xmlctx->in, 1);

    return LY_SUCCESS;
}

/**
 * @brief Store parsed opening element and parse any included namespaces.
 *
 * @param[in] xmlctx XML context to use.
 * @param[in] prefix Parsed starting element prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] name Parsed starting element name.
 * @param[in] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_open_element(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len, const char *name, size_t name_len)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_elem *e;
    const char *prev_input;
    char *value;
    size_t parsed, value_len;
    int ws_only, dynamic, is_ns;
    uint32_t c;

    /* store element opening tag information */
    e = malloc(sizeof *e);
    LY_CHECK_ERR_RET(!e, LOGMEM(xmlctx->ctx), LY_EMEM);
    e->name = name;
    e->prefix = prefix;
    e->name_len = name_len;
    e->prefix_len = prefix_len;
    ly_set_add(&xmlctx->elements, e, LY_SET_OPT_USEASLIST);

    /* skip WS */
    ign_xmlws(xmlctx);

    /* parse and store all namespaces */
    prev_input = xmlctx->in->current;
    is_ns = 1;
    while ((xmlctx->in->current[0] != '\0') && !ly_getutf8(&xmlctx->in->current, &c, &parsed) && is_xmlqnamestartchar(c)) {
        xmlctx->in->current -= parsed;

        /* parse attribute name */
        LY_CHECK_GOTO(ret = lyxml_parse_qname(xmlctx, &prefix, &prefix_len, &name, &name_len), cleanup);

        /* parse the value */
        LY_CHECK_GOTO(ret = lyxml_next_attr_content(xmlctx, (const char **)&value, &value_len, &ws_only, &dynamic), cleanup);

        /* store every namespace */
        if ((prefix && !ly_strncmp("xmlns", prefix, prefix_len)) || (!prefix && !ly_strncmp("xmlns", name, name_len))) {
            LY_CHECK_GOTO(ret = lyxml_ns_add(xmlctx, prefix ? name : NULL, prefix ? name_len : 0,
                                             dynamic ? value : strndup(value, value_len)), cleanup);
            dynamic = 0;
        } else {
            /* not a namespace */
            is_ns = 0;
        }
        if (dynamic) {
            free(value);
        }

        /* skip WS */
        ign_xmlws(xmlctx);

        if (is_ns) {
            /* we can actually skip all the namespaces as there is no reason to parse them again */
            prev_input = xmlctx->in->current;
        }
    }

cleanup:
    if (!ret) {
        xmlctx->in->current = prev_input;
    }
    return ret;
}

/**
 * @brief Move parser to the attribute content and parse it.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] value Parsed attribute value.
 * @param[out] value_len Length of @p value.
 * @param[out] ws_only Whether the value is empty/white-spaces only.
 * @param[out] dynamic Whether the value was dynamically allocated.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_next_attr_content(struct lyxml_ctx *xmlctx, const char **value, size_t *value_len, int *ws_only, int *dynamic)
{
    char quot;

    /* skip WS */
    ign_xmlws(xmlctx);

    /* skip '=' */
    if (xmlctx->in->current[0] == '\0') {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_EOF);
        return LY_EVALID;
    } else if (xmlctx->in->current[0] != '=') {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
               xmlctx->in->current, "'='");
        return LY_EVALID;
    }
    move_input(xmlctx, 1);

    /* skip WS */
    ign_xmlws(xmlctx);

    /* find quotes */
    if (xmlctx->in->current[0] == '\0') {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_EOF);
        return LY_EVALID;
    } else if ((xmlctx->in->current[0] != '\'') && (xmlctx->in->current[0] != '\"')) {
        LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
               xmlctx->in->current, "either single or double quotation mark");
        return LY_EVALID;
    }

    /* remember quote */
    quot = xmlctx->in->current[0];
    move_input(xmlctx, 1);

    /* parse attribute value */
    LY_CHECK_RET(lyxml_parse_value(xmlctx, quot, (char **)value, value_len, ws_only, dynamic));

    /* move after ending quote (without checking for EOF) */
    ly_in_skip(xmlctx->in, 1);

    return LY_SUCCESS;
}

/**
 * @brief Move parser to the next attribute and parse it.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] prefix Parsed attribute prefix.
 * @param[out] prefix_len Length of @p prefix.
 * @param[out] name Parsed attribute name.
 * @param[out] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_next_attribute(struct lyxml_ctx *xmlctx, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    const char *in;
    char *value;
    uint32_t c;
    size_t parsed, value_len;
    int ws_only, dynamic;

    /* skip WS */
    ign_xmlws(xmlctx);

    /* parse only possible attributes */
    while ((xmlctx->in->current[0] != '>') && (xmlctx->in->current[0] != '/')) {
        in = xmlctx->in->current;
        if (in[0] == '\0') {
            LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_EOF);
            return LY_EVALID;
        } else if ((ly_getutf8(&in, &c, &parsed) || !is_xmlqnamestartchar(c))) {
            LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(in - parsed), in - parsed,
                "element tag end ('>' or '/>') or an attribute");
            return LY_EVALID;
        }

        /* parse attribute name */
        LY_CHECK_RET(lyxml_parse_qname(xmlctx, prefix, prefix_len, name, name_len));

        if ((!*prefix || ly_strncmp("xmlns", *prefix, *prefix_len)) && (*prefix || ly_strncmp("xmlns", *name, *name_len))) {
            /* standard attribute */
            break;
        }

        /* namespace, skip it */
        LY_CHECK_RET(lyxml_next_attr_content(xmlctx, (const char **)&value, &value_len, &ws_only, &dynamic));
        if (dynamic) {
            free(value);
        }

        /* skip WS */
        ign_xmlws(xmlctx);
    }

    return LY_SUCCESS;
}

/**
 * @brief Move parser to the next element and parse it.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] prefix Parsed element prefix.
 * @param[out] prefix_len Length of @p prefix.
 * @param[out] name Parse element name.
 * @param[out] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_next_element(struct lyxml_ctx *xmlctx, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len,
                   int *closing)
{
    /* skip WS until EOF or after opening tag '<' */
    LY_CHECK_RET(lyxml_skip_until_end_or_after_otag(xmlctx));
    if (xmlctx->in->current[0] == '\0') {
        /* set return values */
        *prefix = *name = NULL;
        *prefix_len = *name_len = 0;
        return LY_SUCCESS;
    }

    if (xmlctx->in->current[0] == '/') {
        move_input(xmlctx, 1);
        *closing = 1;
    } else {
        *closing = 0;
    }

    /* skip WS */
    ign_xmlws(xmlctx);

    /* parse element name */
    LY_CHECK_RET(lyxml_parse_qname(xmlctx, prefix, prefix_len, name, name_len));

    return LY_SUCCESS;
}

LY_ERR
lyxml_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lyxml_ctx **xmlctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_ctx *xmlctx;
    int closing;

    /* new context */
    xmlctx = calloc(1, sizeof *xmlctx);
    LY_CHECK_ERR_RET(!xmlctx, LOGMEM(ctx), LY_EMEM);
    xmlctx->ctx = ctx;
    xmlctx->line = 1;
    xmlctx->in = in;

    /* parse next element, if any */
    LY_CHECK_GOTO(ret = lyxml_next_element(xmlctx, &xmlctx->prefix, &xmlctx->prefix_len, &xmlctx->name,
                                           &xmlctx->name_len, &closing), cleanup);

    if (xmlctx->in->current[0] == '\0') {
        /* update status */
        xmlctx->status = LYXML_END;
    } else if (closing) {
        LOGVAL(ctx, LY_VLOG_LINE, &xmlctx->line, LYVE_SYNTAX, "Stray closing element tag (\"%.*s\").",
               xmlctx->name_len, xmlctx->name);
        ret = LY_EVALID;
        goto cleanup;
    } else {
        /* open an element, also parses all enclosed namespaces */
        LY_CHECK_GOTO(ret = lyxml_open_element(xmlctx, xmlctx->prefix, xmlctx->prefix_len, xmlctx->name, xmlctx->name_len), cleanup);

        /* update status */
        xmlctx->status = LYXML_ELEMENT;
    }

cleanup:
    if (ret) {
        lyxml_ctx_free(xmlctx);
    } else {
        *xmlctx_p = xmlctx;
    }
    return ret;
}

LY_ERR
lyxml_ctx_next(struct lyxml_ctx *xmlctx)
{
    LY_ERR ret = LY_SUCCESS;
    int closing;
    struct lyxml_elem *e;

    /* if the value was not used, free it */
    if (((xmlctx->status == LYXML_ELEM_CONTENT) || (xmlctx->status == LYXML_ATTR_CONTENT)) && xmlctx->dynamic) {
        free((char *)xmlctx->value);
        xmlctx->value = NULL;
        xmlctx->dynamic = 0;
    }

    switch (xmlctx->status) {
    /* content |</elem> */
    case LYXML_ELEM_CONTENT:
        /* handle special case when empty content for "<elem/>" was returned */
        if (xmlctx->in->current[0] == '/') {
            assert(xmlctx->elements.count);
            e = (struct lyxml_elem *)xmlctx->elements.objs[xmlctx->elements.count - 1];

            /* close the element (parses closing tag) */
            ret = lyxml_close_element(xmlctx, e->prefix, e->prefix_len, e->name, e->name_len, 1);
            LY_CHECK_GOTO(ret, cleanup);

            /* update status */
            xmlctx->status = LYXML_ELEM_CLOSE;
            break;
        }
        /* fallthrough */

    /* </elem>| <elem2>* */
    case LYXML_ELEM_CLOSE:
        /* parse next element, if any */
        ret = lyxml_next_element(xmlctx, &xmlctx->prefix, &xmlctx->prefix_len, &xmlctx->name, &xmlctx->name_len, &closing);
        LY_CHECK_GOTO(ret, cleanup);

        if (xmlctx->in->current[0] == '\0') {
            /* update status */
            xmlctx->status = LYXML_END;
        } else if (closing) {
            /* close an element (parses also closing tag) */
            ret = lyxml_close_element(xmlctx, xmlctx->prefix, xmlctx->prefix_len, xmlctx->name, xmlctx->name_len, 0);
            LY_CHECK_GOTO(ret, cleanup);

            /* update status */
            xmlctx->status = LYXML_ELEM_CLOSE;
        } else {
            /* open an element, also parses all enclosed namespaces */
            ret = lyxml_open_element(xmlctx, xmlctx->prefix, xmlctx->prefix_len, xmlctx->name, xmlctx->name_len);
            LY_CHECK_GOTO(ret, cleanup);

            /* update status */
            xmlctx->status = LYXML_ELEMENT;
        }
        break;

    /* <elem| attr='val'* > content  */
    case LYXML_ELEMENT:

    /* attr='val'| attr='val'* > content */
    case LYXML_ATTR_CONTENT:
        /* parse attribute name, if any */
        ret = lyxml_next_attribute(xmlctx, &xmlctx->prefix, &xmlctx->prefix_len, &xmlctx->name, &xmlctx->name_len);
        LY_CHECK_GOTO(ret, cleanup);

        if (xmlctx->in->current[0] == '>') {
            /* no attributes but a closing tag */
            ly_in_skip(xmlctx->in, 1);
            if (!xmlctx->in->current[0]) {
                LOGVAL(xmlctx->ctx, LY_VLOG_LINE, &xmlctx->line, LY_VCODE_EOF);
                ret = LY_EVALID;
                goto cleanup;
            }

            /* parse element content */
            ret = lyxml_parse_value(xmlctx, '<', (char **)&xmlctx->value, &xmlctx->value_len, &xmlctx->ws_only,
                                    &xmlctx->dynamic);
            LY_CHECK_GOTO(ret, cleanup);

            if (!xmlctx->value_len) {
                /* use empty value, easier to work with */
                xmlctx->value = "";
                assert(!xmlctx->dynamic);
            }

            /* update status */
            xmlctx->status = LYXML_ELEM_CONTENT;
        } else if (xmlctx->in->current[0] == '/') {
            /* no content but we still return it */
            xmlctx->value = "";
            xmlctx->value_len = 0;
            xmlctx->ws_only = 1;
            xmlctx->dynamic = 0;

            /* update status */
            xmlctx->status = LYXML_ELEM_CONTENT;
        } else {
            /* update status */
            xmlctx->status = LYXML_ATTRIBUTE;
        }
        break;

    /* attr|='val' */
    case LYXML_ATTRIBUTE:
        /* skip formatting and parse value */
        ret = lyxml_next_attr_content(xmlctx, &xmlctx->value, &xmlctx->value_len, &xmlctx->ws_only, &xmlctx->dynamic);
        LY_CHECK_GOTO(ret, cleanup);

        /* update status */
        xmlctx->status = LYXML_ATTR_CONTENT;
        break;

    /* </elem>   |EOF */
    case LYXML_END:
        /* nothing to do */
        break;
    }

cleanup:
    if (ret) {
        /* invalidate context */
        xmlctx->status = LYXML_END;
    }
    return ret;
}

LY_ERR
lyxml_ctx_peek(struct lyxml_ctx *xmlctx, enum LYXML_PARSER_STATUS *next)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name, *prev_input;
    size_t prefix_len, name_len;
    int closing;

    prev_input = xmlctx->in->current;

    switch (xmlctx->status) {
    case LYXML_ELEM_CONTENT:
        if (xmlctx->in->current[0] == '/') {
            *next = LYXML_ELEM_CLOSE;
            break;
        }
        /* fallthrough */
    case LYXML_ELEM_CLOSE:
        /* parse next element, if any */
        ret = lyxml_next_element(xmlctx, &prefix, &prefix_len, &name, &name_len, &closing);
        LY_CHECK_GOTO(ret, cleanup);

        if (xmlctx->in->current[0] == '\0') {
            *next = LYXML_END;
        } else if (closing) {
            *next = LYXML_ELEM_CLOSE;
        } else {
            *next = LYXML_ELEMENT;
        }
        break;
    case LYXML_ELEMENT:
    case LYXML_ATTR_CONTENT:
        /* parse attribute name, if any */
        ret = lyxml_next_attribute(xmlctx, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_GOTO(ret, cleanup);

        if ((xmlctx->in->current[0] == '>') || (xmlctx->in->current[0] == '/')) {
            *next = LYXML_ELEM_CONTENT;
        } else {
            *next = LYXML_ATTRIBUTE;
        }
        break;
    case LYXML_ATTRIBUTE:
        *next = LYXML_ATTR_CONTENT;
        break;
    case LYXML_END:
        *next = LYXML_END;
        break;
    }

cleanup:
    xmlctx->in->current = prev_input;
    return ret;
}

void
lyxml_ctx_free(struct lyxml_ctx *xmlctx)
{
    uint32_t u;

    if (!xmlctx) {
        return;
    }

    if (((xmlctx->status == LYXML_ELEM_CONTENT) || (xmlctx->status == LYXML_ATTR_CONTENT)) && xmlctx->dynamic) {
        free((char *)xmlctx->value);
    }
    ly_set_erase(&xmlctx->elements, free);
    for (u = xmlctx->ns.count - 1; u + 1 > 0; --u) {
        /* remove the ns structure */
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->prefix);
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->uri);
        free(xmlctx->ns.objs[u]);
    }
    ly_set_erase(&xmlctx->ns, NULL);
    free(xmlctx);
}

LY_ERR
lyxml_dump_text(struct ly_out *out, const char *text, int attribute)
{
    LY_ERR ret;
    unsigned int u;

    if (!text) {
        return 0;
    }

    for (u = 0; text[u]; u++) {
        switch (text[u]) {
        case '&':
            ret = ly_print_(out, "&amp;");
            break;
        case '<':
            ret = ly_print_(out, "&lt;");
            break;
        case '>':
            /* not needed, just for readability */
            ret = ly_print_(out, "&gt;");
            break;
        case '"':
            if (attribute) {
                ret = ly_print_(out, "&quot;");
                break;
            }
            /* falls through */
        default:
            ret = ly_write_(out, &text[u], 1);
            break;
        }
        LY_CHECK_RET(ret);
    }

    return LY_SUCCESS;
}

LY_ERR
lyxml_get_prefixes(struct lyxml_ctx *xmlctx, const char *value, size_t value_len, struct ly_prefix **val_prefs)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    uint32_t c;
    const struct lyxml_ns *ns;
    const char *start, *stop;
    struct ly_prefix *prefixes = NULL;
    size_t len;

    for (stop = start = value; (size_t)(stop - value) < value_len; start = stop) {
        size_t bytes;
        ly_getutf8(&stop, &c, &bytes);
        if (is_xmlqnamestartchar(c)) {
            for (ly_getutf8(&stop, &c, &bytes);
                    is_xmlqnamechar(c) && (size_t)(stop - value) < value_len;
                    ly_getutf8(&stop, &c, &bytes)) {}
            stop = stop - bytes;
            if (*stop == ':') {
                /* we have a possible prefix */
                len = stop - start;
                ns = lyxml_ns_get(&xmlctx->ns, start, len);
                if (ns) {
                    struct ly_prefix *p = NULL;

                    /* check whether we do not already have this prefix stored */
                    LY_ARRAY_FOR(prefixes, u) {
                        if (!ly_strncmp(prefixes[u].id, start, len)) {
                            p = &prefixes[u];
                            break;
                        }
                    }
                    if (!p) {
                        LY_ARRAY_NEW_GOTO(xmlctx->ctx, prefixes, p, ret, error);
                        p->id = lydict_insert(xmlctx->ctx, start, len);
                        p->module_ns = lydict_insert(xmlctx->ctx, ns->uri, 0);
                    } /* else the prefix already present */
                }
            }
            stop = stop + bytes;
        }
    }

    *val_prefs = prefixes;
    return LY_SUCCESS;

error:
    LY_ARRAY_FOR(prefixes, u) {
        lydict_remove(xmlctx->ctx, prefixes[u].id);
        lydict_remove(xmlctx->ctx, prefixes[u].module_ns);
    }
    LY_ARRAY_FREE(prefixes);
    return ret;
}

LY_ERR
lyxml_value_compare(const char *value1, const struct ly_prefix *prefs1, const char *value2, const struct ly_prefix *prefs2)
{
    const char *ptr1, *ptr2, *ns1, *ns2;
    LY_ARRAY_COUNT_TYPE u1, u2;
    int len;

    if (!value1 && !value2) {
        return LY_SUCCESS;
    }
    if ((value1 && !value2) || (!value1 && value2)) {
        return LY_ENOT;
    }

    ptr1 = value1;
    ptr2 = value2;
    while (ptr1[0] && ptr2[0]) {
        if (ptr1[0] != ptr2[0]) {
            /* it can be a start of prefix that maps to the same module */
            ns1 = ns2 = NULL;
            u1 = u2 = 0;
            if (prefs1) {
                /* find module of the first prefix, if any */
                LY_ARRAY_FOR(prefs1, u1) {
                    len = strlen(prefs1[u1].id);
                    if (!strncmp(ptr1, prefs1[u1].id, len) && (ptr1[len] == ':')) {
                        ns1 = prefs1[u1].module_ns;
                        break;
                    }
                }
            }
            if (prefs2) {
                /* find module of the second prefix, if any */
                LY_ARRAY_FOR(prefs2, u2) {
                    len = strlen(prefs2[u2].id);
                    if (!strncmp(ptr2, prefs2[u2].id, len) && (ptr2[len] == ':')) {
                        ns2 = prefs2[u2].module_ns;
                        break;
                    }
                }
            }

            if (!ns1 || !ns2 || (ns1 != ns2)) {
                /* not a prefix or maps to different namespaces */
                break;
            }

            /* skip prefixes in both values (':' is skipped as iter) */
            ptr1 += strlen(prefs1[u1].id);
            ptr2 += strlen(prefs2[u2].id);
        }

        ++ptr1;
        ++ptr2;
    }
    if (ptr1[0] || ptr2[0]) {
        /* not a match or simply different lengths */
        return LY_ENOT;
    }

    return LY_SUCCESS;
}
