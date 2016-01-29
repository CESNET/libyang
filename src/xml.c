/**
 * @file xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML parser implementation for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "common.h"
#include "dict_private.h"
#include "printer.h"
#include "parser.h"
#include "tree_schema.h"
#include "xml_internal.h"

#ifndef NDEBUG
static unsigned int lws_lineno = 0;
#endif
static unsigned int lineno = 0;

#define ign_xmlws(p)                                                    \
	while (is_xmlws(*p)) {                                              \
		COUNTLINE(*p);                                                      \
		p++;                                                            \
	}

static struct lyxml_attr *lyxml_dup_attr(struct ly_ctx *ctx, struct lyxml_elem *parent, struct lyxml_attr *attr);

API const struct lyxml_ns *
lyxml_get_ns(const struct lyxml_elem *elem, const char *prefix)
{
    struct lyxml_attr *attr;
    int len;

    if (!elem) {
        return NULL;
    }

    if (!prefix) {
        len = 0;
    } else {
        len = strlen(prefix) + 1;
    }

    for (attr = elem->attr; attr; attr = attr->next) {
        if (attr->type != LYXML_ATTR_NS) {
            continue;
        }
        if (!attr->name) {
            if (!len) {
                /* default namespace found */
                if (!attr->value) {
                    /* empty default namespace -> no default namespace */
                    return NULL;
                }
                return (struct lyxml_ns *)attr;
            }
        } else if (len && !memcmp(attr->name, prefix, len)) {
            /* prefix found */
            return (struct lyxml_ns *)attr;
        }
    }

    /* go recursively */
    return lyxml_get_ns(elem->parent, prefix);
}

static void
lyxml_correct_attr_ns(struct ly_ctx *ctx, struct lyxml_attr *attr, struct lyxml_elem *attr_parent, int copy_ns)
{
    const struct lyxml_ns *tmp_ns;
    struct lyxml_elem *ns_root, *attr_root;

    if ((attr->type != LYXML_ATTR_NS) && attr->ns) {
        /* find the root of attr */
        for (attr_root = attr_parent; attr_root->parent; attr_root = attr_root->parent);

        /* find the root of attr NS */
        for (ns_root = attr->ns->parent; ns_root->parent; ns_root = ns_root->parent);

        /* attr NS is defined outside attr parent subtree */
        if (ns_root != attr_root) {
            if (copy_ns) {
                tmp_ns = attr->ns;
                /* we may have already copied the NS over? */
                attr->ns = lyxml_get_ns(attr->ns->parent, tmp_ns->prefix);

                /* we haven't copied it over, copy it now */
                if (!attr->ns) {
                    attr->ns = (struct lyxml_ns *)lyxml_dup_attr(ctx, attr_parent, (struct lyxml_attr *)tmp_ns);
                }
            } else {
                attr->ns = NULL;
            }
        }
    }
}

static struct lyxml_attr *
lyxml_dup_attr(struct ly_ctx *ctx, struct lyxml_elem *parent, struct lyxml_attr *attr)
{
    struct lyxml_attr *result, *a;

    if (!attr || !parent) {
        return NULL;
    }

    if (attr->type == LYXML_ATTR_NS) {
        /* this is correct, despite that all attributes seems like a standard
         * attributes (struct lyxml_attr), some of them can be namespace
         * definitions (and in that case they are struct lyxml_ns).
         */
        result = (struct lyxml_attr *)calloc(1, sizeof (struct lyxml_ns));
    } else {
        result = calloc(1, sizeof (struct lyxml_attr));
    }
    if (!result) {
        LOGMEM;
        return NULL;
    }
    result->value = lydict_insert(ctx, attr->value, 0);
    result->name = lydict_insert(ctx, attr->name, 0);
    result->type = attr->type;

    /* set namespace in case of standard attributes */
    if (result->type == LYXML_ATTR_STD && attr->ns) {
        result->ns = attr->ns;
        lyxml_correct_attr_ns(ctx, result, parent, 1);
    }

    /* set parent pointer in case of namespace attribute */
    if (result->type == LYXML_ATTR_NS) {
        ((struct lyxml_ns *)result)->parent = parent;
    }

    /* put attribute into the parent's attributes list */
    if (parent->attr) {
        /* go to the end of the list */
        for (a = parent->attr; a->next; a = a->next);
        /* and append new attribute */
        a->next = result;
    } else {
        /* add the first attribute in the list */
        parent->attr = result;
    }

    return result;
}

/* copy_ns: 0 - set invalid namespaces to NULL, 1 - copy them into this subtree */
static void
lyxml_correct_elem_ns(struct ly_ctx *ctx, struct lyxml_elem *elem, int copy_ns, int correct_attrs)
{
    const struct lyxml_ns *tmp_ns;
    struct lyxml_elem *elem_root, *ns_root, *tmp, *iter;
    struct lyxml_attr *attr;

    /* find the root of elem */
    for (elem_root = elem; elem_root->parent; elem_root = elem_root->parent);

    LY_TREE_DFS_BEGIN(elem, tmp, iter) {
        if (iter->ns) {
            /* find the root of elem NS */
            for (ns_root = iter->ns->parent; ns_root; ns_root = ns_root->parent);

            /* elem NS is defined outside elem subtree */
            if (ns_root != elem_root) {
                if (copy_ns) {
                    tmp_ns = iter->ns;
                    /* we may have already copied the NS over? */
                    iter->ns = lyxml_get_ns(iter, tmp_ns->prefix);

                    /* we haven't copied it over, copy it now */
                    if (!iter->ns) {
                        iter->ns = (struct lyxml_ns *)lyxml_dup_attr(ctx, iter, (struct lyxml_attr *)tmp_ns);
                    }
                } else {
                    iter->ns = NULL;
                }
            }
        }
        if (correct_attrs) {
            LY_TREE_FOR(iter->attr, attr) {
                lyxml_correct_attr_ns(ctx, attr, elem_root, copy_ns);
            }
        }
        LY_TREE_DFS_END(elem, tmp, iter);
    }
}

struct lyxml_elem *
lyxml_dup_elem(struct ly_ctx *ctx, struct lyxml_elem *elem, struct lyxml_elem *parent, int recursive)
{
    struct lyxml_elem *result, *child;
    struct lyxml_attr *attr;

    if (!elem) {
        return NULL;
    }

    result = calloc(1, sizeof *result);
    if (!result) {
        LOGMEM;
        return NULL;
    }
    result->content = lydict_insert(ctx, elem->content, 0);
    result->name = lydict_insert(ctx, elem->name, 0);
    result->flags = elem->flags;
#ifndef NDEBUG
    result->line = elem->line;
#endif
    result->prev = result;

    if (parent) {
        lyxml_add_child(ctx, parent, result);
    }

    /* keep old namespace for now */
    result->ns = elem->ns;

    /* correct namespaces */
    lyxml_correct_elem_ns(ctx, result, 1, 0);

    /* duplicate attributes */
    for (attr = elem->attr; attr; attr = attr->next) {
        lyxml_dup_attr(ctx, result, attr);
    }

    if (!recursive) {
        return result;
    }

    /* duplicate children */
    LY_TREE_FOR(elem->child, child) {
        lyxml_dup_elem(ctx, child, result, 1);
    }

    return result;
}

void
lyxml_unlink_elem(struct ly_ctx *ctx, struct lyxml_elem *elem, int copy_ns)
{
    struct lyxml_elem *parent, *first;

    if (!elem) {
        return;
    }

    /* store pointers to important nodes */
    parent = elem->parent;

    /* unlink from parent */
    if (parent) {
        if (parent->child == elem) {
            /* we unlink the first child */
            /* update the parent's link */
            parent->child = elem->next;
        }
        /* forget about the parent */
        elem->parent = NULL;
    }

    if (copy_ns < 2) {
        lyxml_correct_elem_ns(ctx, elem, copy_ns, 1);
    }

    /* unlink from siblings */
    if (elem->prev == elem) {
        /* there are no more siblings */
        return;
    }
    if (elem->next) {
        elem->next->prev = elem->prev;
    } else {
        /* unlinking the last element */
        if (parent) {
            first = parent->child;
        } else {
            first = elem;
            while (elem->prev->next) {
                first = elem->prev;
            }
        }
        first->prev = elem->prev;
    }
    if (elem->prev->next) {
        elem->prev->next = elem->next;
    }

    /* clean up the unlinked element */
    elem->next = NULL;
    elem->prev = elem;
}

API void
lyxml_unlink(struct ly_ctx *ctx, struct lyxml_elem *elem)
{
    if (!elem) {
        return;
    }

    lyxml_unlink_elem(ctx, elem, 1);
}

void
lyxml_free_attr(struct ly_ctx *ctx, struct lyxml_elem *parent, struct lyxml_attr *attr)
{
    struct lyxml_attr *aiter, *aprev;

    if (!attr) {
        return;
    }

    if (parent) {
        /* unlink attribute from the parent's list of attributes */
        aprev = NULL;
        for (aiter = parent->attr; aiter; aiter = aiter->next) {
            if (aiter == attr) {
                break;
            }
            aprev = aiter;
        }
        if (!aiter) {
            /* attribute to remove not found */
            return;
        }

        if (!aprev) {
            /* attribute is first in parent's list of attributes */
            parent->attr = attr->next;
        } else {
            /* reconnect previous attribute to the next */
            aprev->next = attr->next;
        }
    }
    lydict_remove(ctx, attr->name);
    lydict_remove(ctx, attr->value);
    free(attr);
}

void
lyxml_free_attrs(struct ly_ctx *ctx, struct lyxml_elem *elem)
{
    struct lyxml_attr *a, *next;
    if (!elem || !elem->attr) {
        return;
    }

    a = elem->attr;
    do {
        next = a->next;

        lydict_remove(ctx, a->name);
        lydict_remove(ctx, a->value);
        free(a);

        a = next;
    } while (a);
}

static void
lyxml_free_elem(struct ly_ctx *ctx, struct lyxml_elem *elem)
{
    struct lyxml_elem *e, *next;

    if (!elem) {
        return;
    }

    lyxml_free_attrs(ctx, elem);
    LY_TREE_FOR_SAFE(elem->child, next, e) {
        lyxml_free_elem(ctx, e);
    }
    lydict_remove(ctx, elem->name);
    lydict_remove(ctx, elem->content);
    free(elem);
}

API void
lyxml_free(struct ly_ctx *ctx, struct lyxml_elem *elem)
{
    if (!elem) {
        return;
    }

    lyxml_unlink_elem(ctx, elem, 2);
    lyxml_free_elem(ctx, elem);
}

API const char *
lyxml_get_attr(const struct lyxml_elem *elem, const char *name, const char *ns)
{
    struct lyxml_attr *a;

    assert(elem);
    assert(name);

    for (a = elem->attr; a; a = a->next) {
        if (a->type != LYXML_ATTR_STD) {
            continue;
        }

        if (!strcmp(name, a->name)) {
            if ((!ns && !a->ns) || (ns && a->ns && !strcmp(ns, a->ns->value))) {
                return a->value;
            }
        }
    }

    return NULL;
}

int
lyxml_add_child(struct ly_ctx *ctx, struct lyxml_elem *parent, struct lyxml_elem *elem)
{
    struct lyxml_elem *e;

    assert(parent);
    assert(elem);

    /* (re)link element to parent */
    if (elem->parent) {
        lyxml_unlink_elem(ctx, elem, 1);
    }
    elem->parent = parent;

    /* link parent to element */
    if (parent->child) {
        e = parent->child;
        elem->prev = e->prev;
        elem->next = NULL;
        elem->prev->next = elem;
        e->prev = elem;
    } else {
        parent->child = elem;
        elem->prev = elem;
        elem->next = NULL;
    }

    return EXIT_SUCCESS;
}

int
lyxml_getutf8(const char *buf, unsigned int *read, unsigned int line)
{
    int c, aux;
    int i;

    c = buf[0];
    *read = 0;

    /* buf is NULL terminated string, so 0 means EOF */
    if (!c) {
        LOGVAL(LYE_EOF, line);
        return 0;
    }
    *read = 1;

    /* process character byte(s) */
    if ((c & 0xf8) == 0xf0) {
        /* four bytes character */
        *read = 4;

        c &= 0x07;
        for (i = 1; i <= 3; i++) {
            aux = buf[i];
            if ((aux & 0xc0) != 0x80) {
                LOGVAL(LYE_XML_INVAL, line, "input character");
                return 0;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if (c < 0x1000 || c > 0x10ffff) {
            LOGVAL(LYE_XML_INVAL, line, "input character");
            return 0;
        }
    } else if ((c & 0xf0) == 0xe0) {
        /* three bytes character */
        *read = 3;

        c &= 0x0f;
        for (i = 1; i <= 2; i++) {
            aux = buf[i];
            if ((aux & 0xc0) != 0x80) {
                LOGVAL(LYE_XML_INVAL, line, "input character");
                return 0;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if (c < 0x800 || (c > 0xd7ff && c < 0xe000) || c > 0xfffd) {
            LOGVAL(LYE_XML_INVAL, line, "input character");
            return 0;
        }
    } else if ((c & 0xe0) == 0xc0) {
        /* two bytes character */
        *read = 2;

        aux = buf[1];
        if ((aux & 0xc0) != 0x80) {
            LOGVAL(LYE_XML_INVAL, line, "input character");
            return 0;
        }
        c = ((c & 0x1f) << 6) | (aux & 0x3f);

        if (c < 0x80) {
            LOGVAL(LYE_XML_INVAL, line, "input character");
            return 0;
        }
    } else if (!(c & 0x80)) {
        /* one byte character */
        if (c < 0x20 && c != 0x9 && c != 0xa && c != 0xd) {
            /* invalid character */
            LOGVAL(LYE_XML_INVAL, line, "input character");
            return 0;
        }
    } else {
        /* invalid character */
        LOGVAL(LYE_XML_INVAL, line, "input character");
        return 0;
    }

    return c;
}

/* logs directly */
static int
parse_ignore(const char *data, const char *endstr, unsigned int *len)
{
    unsigned int slen;
    const char *c = data;

    slen = strlen(endstr);

    while (*c && memcmp(c, endstr, slen)) {
        COUNTLINE(*c);
        c++;
    }
    if (!*c) {
        LOGVAL(LYE_XML_MISS, lineno, "closing sequence", endstr);
        return EXIT_FAILURE;
    }
    c += slen;

    *len = c - data;
    return EXIT_SUCCESS;
}

/* logs directly */
static char *
parse_text(const char *data, char delim, unsigned int *len)
{
#define BUFSIZE 1024

    char buf[BUFSIZE];
    char *result = NULL, *aux;
    unsigned int r;
    int o, size = 0;
    int cdsect = 0;
    int32_t n;

    for (*len = o = 0; cdsect || data[*len] != delim; o++) {
        if (!data[*len] || (!cdsect && !memcmp(&data[*len], "]]>", 2))) {
            LOGVAL(LYE_XML_INVAL, lineno, "element content, \"]]>\" found");
            goto error;
        }

loop:

        if (o > BUFSIZE - 3) {
            /* add buffer into the result */
            if (result) {
                size = size + o;
                aux = ly_realloc(result, size + 1);
                result = aux;
            } else {
                size = o;
                result = malloc((size + 1) * sizeof *result);
            }
            if (!result) {
                LOGMEM;
                return NULL;
            }
            memcpy(&result[size - o], buf, o);

            /* write again into the beginning of the buffer */
            o = 0;
        }

        if (cdsect || !memcmp(&data[*len], "<![CDATA[", 9)) {
            /* CDSect */
            if (!cdsect) {
                cdsect = 1;
                *len += 9;
            }
            if (data[*len] && !memcmp(&data[*len], "]]>", 3)) {
                *len += 3;
                cdsect = 0;
                o--;            /* we don't write any data in this iteration */
            } else {
                buf[o] = data[*len];
                (*len)++;
            }
        } else if (data[*len] == '&') {
            (*len)++;
            if (data[*len] != '#') {
                /* entity reference - only predefined refs are supported */
                if (!memcmp(&data[*len], "lt;", 3)) {
                    buf[o] = '<';
                    *len += 3;
                } else if (!memcmp(&data[*len], "gt;", 3)) {
                    buf[o] = '>';
                    *len += 3;
                } else if (!memcmp(&data[*len], "amp;", 4)) {
                    buf[o] = '&';
                    *len += 4;
                } else if (!memcmp(&data[*len], "apos;", 5)) {
                    buf[o] = '\'';
                    *len += 5;
                } else if (!memcmp(&data[*len], "quot;", 5)) {
                    buf[o] = '\"';
                    *len += 5;
                } else {
                    LOGVAL(LYE_XML_INVAL, lineno, "entity reference (only predefined references are supported)");
                    goto error;
                }
            } else {
                /* character reference */
                (*len)++;
                if (isdigit(data[*len])) {
                    for (n = 0; isdigit(data[*len]); (*len)++) {
                        n = (10 * n) + (data[*len] - '0');
                    }
                    if (data[*len] != ';') {
                        LOGVAL(LYE_XML_INVAL, lineno, "character reference, missing semicolon");
                        goto error;
                    }
                } else if (data[(*len)++] == 'x' && isxdigit(data[*len])) {
                    for (n = 0; isxdigit(data[*len]); (*len)++) {
                        if (isdigit(data[*len])) {
                            r = (data[*len] - '0');
                        } else if (data[*len] > 'F') {
                            r = 10 + (data[*len] - 'a');
                        } else {
                            r = 10 + (data[*len] - 'A');
                        }
                        n = (16 * n) + r;
                    }
                } else {
                    LOGVAL(LYE_XML_INVAL, lineno, "character reference");
                    goto error;

                }
                r = pututf8(&buf[o], n, lineno);
                if (!r) {
                    LOGVAL(LYE_XML_INVAL, lineno, "character reference value");
                    goto error;
                }
                o += r - 1;     /* o is ++ in for loop */
                (*len)++;
            }
        } else {
            buf[o] = data[*len];
            COUNTLINE(buf[o]);
            (*len)++;
        }
    }

    if (delim == '<' && !memcmp(&data[*len], "<![CDATA[", 9)) {
        /* ignore loop's end condition on beginning of CDSect */
        goto loop;
    }
#undef BUFSIZE

    if (o) {
        if (result) {
            size = size + o;
            aux = realloc(result, size + 1);
            result = aux;
        } else {
            size = o;
            result = malloc((size + 1) * sizeof *result);
        }
        if (!result) {
            LOGMEM;
            return NULL;
        }
        memcpy(&result[size - o], buf, o);
    }
    if (result) {
        result[size] = '\0';
    } else {
        size = 0;
        result = strdup("");
    }

    return result;

error:
    free(result);
    return NULL;
}

/* logs directly */
static struct lyxml_attr *
parse_attr(struct ly_ctx *ctx, const char *data, unsigned int *len, struct lyxml_elem *parent)
{
    const char *c = data, *start, *delim;
    char prefix[32];
    int uc;
    struct lyxml_attr *attr = NULL, *a;
    unsigned int size;

    /* check if it is attribute or namespace */
    if (!memcmp(c, "xmlns", 5)) {
        /* namespace */
        attr = calloc(1, sizeof (struct lyxml_ns));
        if (!attr) {
            LOGMEM;
            return NULL;
        }
        attr->type = LYXML_ATTR_NS;
        ((struct lyxml_ns *)attr)->parent = parent;
        c += 5;
        if (*c != ':') {
            /* default namespace, prefix will be empty */
            goto equal;
        }
        c++;                    /* go after ':' to the prefix value */
    } else {
        /* attribute */
        attr = calloc(1, sizeof *attr);
        if (!attr) {
            LOGMEM;
            return NULL;
        }
        attr->type = LYXML_ATTR_STD;
    }

    /* process name part of the attribute */
    start = c;
    uc = lyxml_getutf8(c, &size, lineno);
    if (!is_xmlnamestartchar(uc)) {
        LOGVAL(LYE_XML_INVAL, lineno, "NameStartChar of the attribute");
        free(attr);
        return NULL;
    }
    c += size;
    uc = lyxml_getutf8(c, &size, lineno);
    while (is_xmlnamechar(uc)) {
        if (attr->type == LYXML_ATTR_STD && *c == ':') {
            /* attribute in a namespace */
            start = c + 1;

            /* look for the prefix in namespaces */
            memcpy(prefix, data, c - data);
            prefix[c - data] = '\0';
            attr->ns = lyxml_get_ns(parent, prefix);
        }
        c += size;
        uc = lyxml_getutf8(c, &size, lineno);
    }

    /* store the name */
    size = c - start;
    attr->name = lydict_insert(ctx, start, size);

equal:
    /* check Eq mark that can be surrounded by whitespaces */
    ign_xmlws(c);
    if (*c != '=') {
        LOGVAL(LYE_XML_INVAL, lineno, "attribute definition, \"=\" expected");
        goto error;
    }
    c++;
    ign_xmlws(c);

    /* process value part of the attribute */
    if (!*c || (*c != '"' && *c != '\'')) {
        LOGVAL(LYE_XML_INVAL, lineno, "attribute value, \" or \' expected");
        goto error;
    }
    delim = c;
    attr->value = lydict_insert_zc(ctx, parse_text(++c, *delim, &size));
    if (ly_errno) {
        goto error;
    }

    *len = c + size + 1 - data; /* +1 is delimiter size */

    /* put attribute into the parent's attributes list */
    if (parent->attr) {
        /* go to the end of the list */
        for (a = parent->attr; a->next; a = a->next);
        /* and append new attribute */
        a->next = attr;
    } else {
        /* add the first attribute in the list */
        parent->attr = attr;
    }

    return attr;

error:
    lyxml_free_attr(ctx, NULL, attr);
    return NULL;
}

/* logs directly */
static struct lyxml_elem *
parse_elem(struct ly_ctx *ctx, const char *data, unsigned int *len, struct lyxml_elem *parent)
{
    const char *c = data, *start, *e;
    const char *lws;    /* leading white space for handling mixed content */
    int uc;
    char *str;
    char prefix[32] = { 0 };
    unsigned int prefix_len = 0;
    struct lyxml_elem *elem = NULL, *child;
    struct lyxml_attr *attr;
    unsigned int size;
    int nons_flag = 0, closed_flag = 0;

    *len = 0;

    if (*c != '<') {
        return NULL;
    }

    /* locate element name */
    c++;
    e = c;

    uc = lyxml_getutf8(e, &size, lineno);
    if (!is_xmlnamestartchar(uc)) {
        LOGVAL(LYE_XML_INVAL, lineno, "NameStartChar of the element");
        return NULL;
    }
    e += size;
    uc = lyxml_getutf8(e, &size, lineno);
    while (is_xmlnamechar(uc)) {
        if (*e == ':') {
            if (prefix_len) {
                LOGVAL(LYE_XML_INVAL, lineno, "element name, multiple colons found");
                goto error;
            }
            /* element in a namespace */
            start = e + 1;

            /* look for the prefix in namespaces */
            memcpy(prefix, c, prefix_len = e - c);
            prefix[prefix_len] = '\0';
            c = start;
        }
        e += size;
        uc = lyxml_getutf8(e, &size, lineno);
    }
    if (!*e) {
        LOGVAL(LYE_EOF, lineno);
        return NULL;
    }

    /* allocate element structure */
    elem = calloc(1, sizeof *elem);
    if (!elem) {
        LOGMEM;
        return NULL;
    }
#ifndef NDEBUG
    elem->line = lineno;
#endif
    elem->next = NULL;
    elem->prev = elem;
    if (parent) {
        lyxml_add_child(ctx, parent, elem);
    }

    /* store the name into the element structure */
    elem->name = lydict_insert(ctx, c, e - c);
    c = e;

process:
    ly_errno = 0;
    ign_xmlws(c);
    if (!memcmp("/>", c, 2)) {
        /* we are done, it was EmptyElemTag */
        c += 2;
        closed_flag = 1;
    } else if (*c == '>') {
        /* process element content */
        c++;
        lws = NULL;

        while (*c) {
            if (!memcmp(c, "</", 2)) {
                if (lws && !elem->child) {
                    /* leading white spaces were actually content */
                    goto store_content;
                }

                /* Etag */
                c += 2;
                /* get name and check it */
                e = c;
                uc = lyxml_getutf8(e, &size, lineno);
                if (!is_xmlnamestartchar(uc)) {
                    LOGVAL(LYE_XML_INVAL, lineno, "NameStartChar of the attribute");
                    goto error;
                }
                e += size;
                uc = lyxml_getutf8(e, &size, lineno);
                while (is_xmlnamechar(uc)) {
                    if (*e == ':') {
                        /* element in a namespace */
                        start = e + 1;

                        /* look for the prefix in namespaces */
                        if (memcmp(prefix, c, e - c)) {
                            LOGVAL(LYE_SPEC, lineno,
                                   "Mixed opening (%s) and closing element tags (different namespaces).", elem->name);
                            goto error;
                        }
                        c = start;
                    }
                    e += size;
                    uc = lyxml_getutf8(e, &size, lineno);
                }
                if (!*e) {
                    LOGVAL(LYE_EOF, lineno);
                    goto error;
                }

                /* check that it corresponds to opening tag */
                size = e - c;
                str = malloc((size + 1) * sizeof *str);
                if (!str) {
                    LOGMEM;
                    goto error;
                }
                memcpy(str, c, e - c);
                str[e - c] = '\0';
                if (size != strlen(elem->name) || memcmp(str, elem->name, size)) {
                    LOGVAL(LYE_SPEC, lineno, "Mixed opening (%s) and closing (%s) element tags.", elem->name, str);
                    free(str);
                    goto error;
                }
                free(str);
                c = e;

                ign_xmlws(c);
                if (*c != '>') {
                    LOGVAL(LYE_SPEC, lineno, "Close element tag \"%s\" contain additional data.", elem->name);
                    goto error;
                }
                c++;
                closed_flag = 1;
                break;

            } else if (!memcmp(c, "<?", 2)) {
                if (lws) {
                    /* leading white spaces were only formatting */
                    lws = NULL;
                }
                /* PI - ignore it */
                c += 2;
                if (parse_ignore(c, "?>", &size)) {
                    goto error;
                }
                c += size;
            } else if (!memcmp(c, "<!--", 4)) {
                if (lws) {
                    /* leading white spaces were only formatting */
                    lws = NULL;
                }
                /* Comment - ignore it */
                c += 4;
                if (parse_ignore(c, "-->", &size)) {
                    goto error;
                }
                c += size;
            } else if (!memcmp(c, "<![CDATA[", 9)) {
                /* CDSect */
                goto store_content;
            } else if (*c == '<') {
                if (lws) {
                    if (elem->flags & LYXML_ELEM_MIXED) {
                        /* we have a mixed content */
                        goto store_content;
                    } else {
                        /* leading white spaces were only formatting */
                        lws = NULL;
                    }
                }
                if (elem->content) {
                    /* we have a mixed content */
                    child = calloc(1, sizeof *child);
                    if (!child) {
                        LOGMEM;
                        goto error;
                    }
                    child->content = elem->content;
                    elem->content = NULL;
                    lyxml_add_child(ctx, elem, child);
                    elem->flags |= LYXML_ELEM_MIXED;
                }
                child = parse_elem(ctx, c, &size, elem);
                if (!child) {
                    goto error;
                }
                c += size;      /* move after processed child element */
            } else if (is_xmlws(*c)) {
                lws = c;
#ifndef NDEBUG
                lws_lineno = lineno;
#endif
                ign_xmlws(c);
            } else {
store_content:
                /* store text content */
                if (lws) {
                    /* process content including the leading white spaces */
                    c = lws;
#ifndef NDEBUG
                    lineno = lws_lineno;
#endif
                    lws = NULL;
                }
                elem->content = lydict_insert_zc(ctx, parse_text(c, '<', &size));
                if (ly_errno) {
                    goto error;
                }
                c += size;      /* move after processed text content */

                if (elem->child) {
                    /* we have a mixed content */
                    child = calloc(1, sizeof *child);
                    if (!child) {
                        LOGMEM;
                        goto error;
                    }
                    child->content = elem->content;
                    elem->content = NULL;
                    lyxml_add_child(ctx, elem, child);
                    elem->flags |= LYXML_ELEM_MIXED;
                }
            }
        }
    } else {
        /* process attribute */
        attr = parse_attr(ctx, c, &size, elem);
        if (!attr) {
            goto error;
        }
        c += size;              /* move after processed attribute */

        /* check namespace */
        if (attr->type == LYXML_ATTR_NS) {
            if (!prefix[0] && !attr->name) {
                if (attr->value) {
                    /* default prefix */
                    elem->ns = (struct lyxml_ns *)attr;
                } else {
                    /* xmlns="" -> no namespace */
                    nons_flag = 1;
                }
            } else if (prefix[0] && attr->name && !memcmp(attr->name, prefix, prefix_len + 1)) {
                /* matching namespace with prefix */
                elem->ns = (struct lyxml_ns *)attr;
            }
        }

        /* go back to finish element processing */
        goto process;
    }

    *len = c - data;

    if (!closed_flag) {
        LOGVAL(LYE_XML_MISS, lineno, "closing element tag", elem->name);
        goto error;
    }

    if (!elem->ns && !nons_flag && parent) {
        elem->ns = lyxml_get_ns(parent, prefix_len ? prefix : NULL);
    }

    return elem;

error:
    lyxml_free(ctx, elem);

    return NULL;
}

/* logs directly */
API struct lyxml_elem *
lyxml_read_data(struct ly_ctx *ctx, const char *data, int options)
{
    const char *c = data;
    unsigned int len;
    struct lyxml_elem *root, *first = NULL, *next;

    ly_errno = LY_SUCCESS;

#ifndef NDEBUG
    /* TODO: threads support */
    lineno = 1;
#endif

repeat:
    /* process document */
    while (*c) {
        if (is_xmlws(*c)) {
            /* skip whitespaces */
            ign_xmlws(c);
        } else if (!memcmp(c, "<?", 2)) {
            /* XMLDecl or PI - ignore it */
            c += 2;
            if (parse_ignore(c, "?>", &len)) {
                return NULL;
            }
            c += len;
        } else if (!memcmp(c, "<!--", 4)) {
            /* Comment - ignore it */
            c += 2;
            if (parse_ignore(c, "-->", &len)) {
                return NULL;
            }
            c += len;
        } else if (!memcmp(c, "<!", 2)) {
            /* DOCTYPE */
            /* TODO - standalone ignore counting < and > */
            LOGERR(LY_EINVAL, "DOCTYPE not supported in XML documents.");
            return NULL;
        } else if (*c == '<') {
            /* element - process it in next loop to strictly follow XML
             * format
             */
            break;
        } else {
            LOGVAL(LYE_XML_INCHAR, lineno, c);
            return NULL;
        }
    }

    root = parse_elem(ctx, c, &len, NULL);
    if (!root) {
        if (first) {
            LY_TREE_FOR_SAFE(first, next, root) {
                lyxml_free(ctx, root);
            }
        }
        return NULL;
    } else if (!first) {
        first = root;
    } else {
        first->prev->next = root;
        root->prev = first->prev;
        first->prev = root;
    }
    c += len;

    /* ignore the rest of document where can be comments, PIs and whitespaces,
     * note that we are not detecting syntax errors in these parts
     */
    ign_xmlws(c);
    if (*c) {
        if (options & LYXML_READ_MULTIROOT) {
            goto repeat;
        } else {
            LOGWRN("There are some not parsed data:\n%s", c);
        }
    }

    return first;
}

API struct lyxml_elem *
lyxml_read_path(struct ly_ctx *ctx, const char *filename, int options)
{
    struct lyxml_elem *elem = NULL;
    struct stat sb;
    int fd;
    char *addr;

    if (!filename || !ctx) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        LOGERR(LY_EINVAL,"Opening file \"%s\" failed.", filename);
        return NULL;
    }
    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_EINVAL, "Unable to get file \"%s\" information.\n", filename);
        goto error;
    }
    if (!S_ISREG(sb.st_mode)) {
        LOGERR(LY_EINVAL, "File \"%s\" not a file.\n", filename);
        goto error;
    }
    addr = mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        LOGERR(LY_EMEM,"Map file into memory failed (%s()).", __func__);
        goto error;
    }

    elem = lyxml_read_data(ctx, addr, options);
    munmap(addr, sb.st_size);
    close(fd);

    return elem;

error:
    if (fd != -1) {
        close(fd);
    }

    return NULL;
}

int
lyxml_dump_text(struct lyout *out, const char *text)
{
    unsigned int i, n;

    if (!text) {
        return 0;
    }

    for (i = n = 0; text[i]; i++) {
        switch (text[i]) {
        case '&':
            n += ly_print(out, "&amp;");
            break;
        case '<':
            n += ly_print(out, "&lt;");
            break;
        case '>':
            /* not needed, just for readability */
            n += ly_print(out, "&gt;");
            break;
        default:
            ly_write(out, &text[i], 1);
            n++;
        }
    }

    return n;
}

static int
dump_elem(struct lyout *out, const struct lyxml_elem *e, int level, int options)
{
    int size = 0;
    struct lyxml_attr *a;
    struct lyxml_elem *child;
    const char *delim, *delim_outer;
    int indent;

    if (!e->name) {
        /* mixed content */
        if (e->content) {
            return lyxml_dump_text(out, e->content);
        } else {
            return 0;
        }
    }

    delim = delim_outer = (options & LYXML_DUMP_FORMAT) ? "\n" : "";
    indent = 2 * level;
    if ((e->flags & LYXML_ELEM_MIXED) || (e->parent && (e->parent->flags & LYXML_ELEM_MIXED))) {
        delim = "";
    }
    if (e->parent && (e->parent->flags & LYXML_ELEM_MIXED)) {
        delim_outer = "";
        indent = 0;
    }

    if (!(options & (LYXML_DUMP_OPEN | LYXML_DUMP_CLOSE | LYXML_DUMP_ATTRS)) || (options & LYXML_DUMP_OPEN))  {
        /* opening tag */
        if (e->ns && e->ns->prefix) {
            size += ly_print(out, "%*s<%s:%s", indent, "", e->ns->prefix, e->name);
        } else {
            size += ly_print(out, "%*s<%s", indent, "", e->name);
        }
    } else if (options & LYXML_DUMP_CLOSE) {
        indent = 0;
        goto close;
    }

    /* attributes */
    for (a = e->attr; a; a = a->next) {
        if (a->type == LYXML_ATTR_NS) {
            if (a->name) {
                size += ly_print(out, " xmlns:%s=\"%s\"", a->name, a->value ? a->value : "");
            } else {
                size += ly_print(out, " xmlns=\"%s\"", a->value ? a->value : "");
            }
        } else if (a->ns && a->ns->prefix) {
            size += ly_print(out, " %s:%s=\"%s\"", a->ns->prefix, a->name, a->value);
        } else {
            size += ly_print(out, " %s=\"%s\"", a->name, a->value);
        }
    }

    /* apply options */
    if ((options & LYXML_DUMP_CLOSE) && (options & LYXML_DUMP_OPEN)) {
        size += ly_print(out, "/>%s", delim);
        return size;
    } else if (options & LYXML_DUMP_OPEN) {
        ly_print(out, ">");
        return ++size;
    } else if (options & LYXML_DUMP_ATTRS) {
        return size;
    }

    if (!e->child && !e->content) {
        size += ly_print(out, "/>%s", delim);
        return size;
    } else if (e->content) {
        ly_print(out, ">");
        size++;

        size += lyxml_dump_text(out, e->content);

        if (e->ns && e->ns->prefix) {
            size += ly_print(out, "</%s:%s>%s", e->ns->prefix, e->name, delim);
        } else {
            size += ly_print(out, "</%s>%s", e->name, delim);
        }
        return size;
    } else {
        size += ly_print(out, ">%s", delim);
    }

    /* go recursively */
    LY_TREE_FOR(e->child, child) {
        if (options & LYXML_DUMP_FORMAT) {
            size += dump_elem(out, child, level + 1, LYXML_DUMP_FORMAT);
        } else {
            size += dump_elem(out, child, level, 0);
        }
    }

close:
    /* closing tag */
    if (e->ns && e->ns->prefix) {
        size += ly_print(out, "%*s</%s:%s>%s", indent, "", e->ns->prefix, e->name, delim_outer);
    } else {
        size += ly_print(out, "%*s</%s>%s", indent, "", e->name, delim_outer);
    }

    return size;
}

static int
dump_siblings(struct lyout *out, const struct lyxml_elem *e, int options)
{
    const struct lyxml_elem *start, *iter;
    int ret = 0;

    if (e->parent) {
        start = e->parent->child;
    } else {
        start = e;
        while(start->prev && start->prev->next) {
            start = start->prev;
        }
    }

    LY_TREE_FOR(start, iter) {
        ret += dump_elem(out, iter, 0, options);
    }

    return ret;
}

API int
lyxml_dump_file(FILE *stream, const struct lyxml_elem *elem, int options)
{
    struct lyout out;

    if (!stream || !elem) {
        return 0;
    }

    out.type = LYOUT_STREAM;
    out.method.f = stream;

    if (options & LYXML_DUMP_SIBLINGS) {
        return dump_siblings(&out, elem, options);
    } else {
        return dump_elem(&out, elem, 0, options);
    }
}

API int
lyxml_dump_fd(int fd, const struct lyxml_elem *elem, int options)
{
    struct lyout out;

    if (fd < 0 || !elem) {
        return 0;
    }

    out.type = LYOUT_FD;
    out.method.fd = fd;

    if (options & LYXML_DUMP_SIBLINGS) {
        return dump_siblings(&out, elem, options);
    } else {
        return dump_elem(&out, elem, 0, options);
    }
}

API int
lyxml_dump_mem(char **strp, const struct lyxml_elem *elem, int options)
{
    struct lyout out;
    int r;

    if (!strp || !elem) {
        return 0;
    }

    out.type = LYOUT_MEMORY;
    out.method.mem.buf = NULL;
    out.method.mem.len = 0;
    out.method.mem.size = 0;

    if (options & LYXML_DUMP_SIBLINGS) {
        r = dump_siblings(&out, elem, options);
    } else {
        r = dump_elem(&out, elem, 0, options);
    }

    *strp = out.method.mem.buf;
    return r;
}

API int
lyxml_dump_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lyxml_elem *elem, int options)
{
    struct lyout out;

    if (!writeclb || !elem) {
        return 0;
    }

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    if (options & LYXML_DUMP_SIBLINGS) {
        return dump_siblings(&out, elem, options);
    } else {
        return dump_elem(&out, elem, 0, options);
    }
}
