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


#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common.h"
#include "xml.h"

/*
 * Macro to test if character is #x20 | #x9 | #xA | #xD (whitespace)
 */
#define is_xmlws(c) (c == 0x20 || c == 0x9 || c == 0xa || c == 0xd)

#define is_xmlnamestartchar(c) ((c >= 'a' && c <= 'z') || c == '_' || \
		(c >= 'A' && c <= 'Z') || c == ':' || \
		(c >= 0x370 && c <= 0x1fff && c != 0x37e ) || \
		(c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
		c == 0x200d || (c >= 0x2070 && c <= 0x218f) || \
		(c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
		(c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
		(c >= 0x10000 && c <= 0xeffff))

#define is_xmlnamechar(c) ((c >= 'a' && c <= 'z') || c == '_' || c == '-' || \
		(c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ':' || \
		c == '.' || c == 0xb7 || (c >= 0x370 && c <= 0x1fff && c != 0x37e ) ||\
		(c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
		c == 0x200d || (c >= 0x300 && c <= 0x36f) || \
		(c >= 0x2070 && c <= 0x218f) || (c >= 0x2030f && c <= 0x2040) || \
		(c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
		(c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
		(c >= 0x10000 && c <= 0xeffff))

#define ign_xmlws(p) while(is_xmlws(*p)) {p++;}

API int lyxml_unlink_attr(struct lyxml_attr *attr)
{
	struct lyxml_attr *a;

	if (!attr) {
		LY_ERR(LY_EINVAL, NULL);
		return EXIT_FAILURE;
	}

	if (!attr->parent) {
		return EXIT_SUCCESS;
	}

	a = attr->parent->attr;
	if (!a) {
		LY_ERR(LY_EINVAL, "Broken structure (%s).", __func__);
		return EXIT_FAILURE;
	} else if (a == attr) {
		attr->parent->attr = attr->next;
	} else {
		while(a->next != attr) {
			a = a->next;
		}

		if (!a) {
			LY_ERR(LY_EINVAL, "Broken structure (%s).", __func__);
			return EXIT_FAILURE;
		}

		a->next = attr->next;
	}

	attr->parent = NULL;
	attr->next = NULL;

	return EXIT_SUCCESS;
}

API int lyxml_unlink_elem(struct lyxml_elem *elem)
{
	struct lyxml_elem *e;

	if (!elem) {
		LY_ERR(LY_EINVAL, NULL);
		return EXIT_FAILURE;
	}

	if (!elem->parent) {
		return EXIT_SUCCESS;
	}

	e = elem->parent->child;
	if (!e) {
		LY_ERR(LY_EINVAL, "Broken structure (%s).", __func__);
		return EXIT_FAILURE;
	} else if (e == elem) {
		/* child element of parent is going to be the next element after the
		 * one being unlinked
		 */
		if (e == e->next) {
			elem->parent->child = NULL;
		} else {
			elem->parent->child = e->next;
		}
	}

	/* remove elem from ring list of sibling elements */
	while (e != elem) {
		e = e->next;
	}
	if (!e) {
		LY_ERR(LY_EINVAL, "Broken structure (%s).", __func__);
		return EXIT_FAILURE;
	}
	e->prev->next = e->next;
	e->next->prev = e->prev;

	/* clean up the unlinked element */
	e->next = e;
	e->prev = e;

	return EXIT_SUCCESS;
}

API void lyxml_free_attr(struct lyxml_attr *attr)
{
	if (!attr) {
		return;
	}

	lyxml_unlink_attr(attr);
	free(attr->name);
	free(attr->value);
	free(attr);
}

API void lyxml_free_attrs(struct lyxml_elem *elem)
{
	struct lyxml_attr *a, *next;
	if (!elem || !elem->attr) {
		return;
	}

	a = elem->attr;
	do {
		next = a->next;

		free(a->name);
		free(a->value);
		free(a);

		a = next;
	} while (a);
}

static void lyxml_free_elem_(struct lyxml_elem *elem)
{
	struct lyxml_elem *e, *next;

	if (!elem) {
		return;
	}

	lyxml_free_attrs(elem);
	if (elem->child) {
		e = elem->child;
		e->prev->next = NULL;
		do {
			next = e->next;
			lyxml_free_elem_(e);
			e = next;
		} while (e);
	}
	free(elem->name);
	free(elem->content);
	free(elem);
}

API void lyxml_free_elem(struct lyxml_elem *elem)
{
	if (!elem) {
		return;
	}

	lyxml_unlink_elem(elem);
	lyxml_free_elem_(elem);
}

API int lyxml_add_attr(struct lyxml_elem *parent, struct lyxml_attr *attr)
{
	struct lyxml_attr *a;

	if (!parent || !attr) {
		LY_ERR(LY_EINVAL, NULL);
		return EXIT_FAILURE;
	}

	/* (re)link attribute to parent */
	if (attr->parent) {
		lyxml_unlink_attr(attr);
	}
	attr->parent = parent;

	/* link parent to attribute */
	if (parent->attr) {
		for (a = parent->attr; a->next; a = a->next);
		a->next = attr;
	} else {
		parent->attr = attr;
	}

	return EXIT_SUCCESS;
}

API int lyxml_add_child(struct lyxml_elem *parent, struct lyxml_elem *elem)
{
	struct lyxml_elem *e;

	if (!parent || !elem) {
		LY_ERR(LY_EINVAL, NULL);
		return EXIT_FAILURE;
	}

	/* (re)link element to parent */
	if (elem->parent) {
		lyxml_unlink_elem(elem);
	}
	elem->parent = parent;

	/* link parent to element */
	if (parent->child) {
		e = parent->child;
		elem->prev = e->prev;
		elem->next = e;
		elem->prev->next = elem;
		elem->next->prev = elem;
	} else {
		parent->child = elem;
		elem->next = elem;
		elem->prev = elem;
	}

	return EXIT_SUCCESS;
}

/**
 * @brief Get the first UTF-8 character value (4bytes) from buffer
 * @param[in] buf pointr to the current position in input buffer
 * @param[out] read Number of processed bytes in buf (length of UTF-8
 * character).
 * @return UTF-8 value as 4 byte number. 0 means error, only UTF-8 characters
 * valid for XML are returned, so:
 * #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
 * = any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F: 	0xxxxxxx
 * 00000080 -- 000007FF: 	110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF: 	1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF: 	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 */
static int getutf8(const char *buf, unsigned int *read)
{
	int c, aux;
	int i;

	/* check input variable */
	if (!buf || !read) {
		LY_ERR(LY_EINVAL, NULL);
		return 0;
	}
	c = buf[0];
	*read = 0;

	/* buf is NULL terminated string, so 0 means EOF */
	if (!c) {
		LY_ERR(LY_EEOF, NULL);
		return 0;
	}
	*read = 1;

	/* process character byte(s) */
	if ((c && 0xf8) == 0xf0) {
		/* four bytes character */
		*read = 4;

		c &= 0x07;
		for (i = 1; i <= 3; i++) {
			aux = buf[i];
			if ((aux & 0xc0) != 0x80) {
				LY_ERR(LY_EINVAL, NULL);
				return 0;
			}

			c = (c << 6) | (aux & 0x3f);
		}


		if (c < 0x1000 || c > 0x10ffff) {
			LY_ERR(LY_EINVAL, NULL);
		    return 0;
		}
	} else if ((c & 0xf0) == 0xe0) {
		/* three bytes character */
		*read = 3;

		c &= 0x0f;
		for (i = 1; i <= 2; i++) {
			aux = buf[i];
			if ((aux & 0xc0) != 0x80) {
				LY_ERR(LY_EINVAL, NULL);
				return 0;
			}

			c = (c << 6) | (aux & 0x3f);
		}


		if (c < 0x800 || (c > 0xd7ff && c < 0xe000) || c > 0xfffd ) {
			LY_ERR(LY_EINVAL, NULL);
		    return 0;
		}
	} else if ((c & 0xe0) == 0xc0) {
		/* two bytes character */
		*read = 2;

		aux = buf[1];
		if ((aux & 0xc0) != 0x80) {
			LY_ERR(LY_EINVAL, NULL);
		    return 0;
		}
		c = ((c & 0x1f) << 6) | (aux & 0x3f);

		if (c < 0x80) {
			LY_ERR(LY_EINVAL, NULL);
		    return 0;
		}
	} else if (!(c & 0x80)) {
		/* one byte character */
		if (c < 0x20 && c != 0x9 && c != 0xa && c != 0xd) {
			/* invalid character */
			LY_ERR(LY_EINVAL, NULL);
			return 0;
		}
	} else {
		/* invalid character */
		LY_ERR(LY_EINVAL, NULL);
		return 0;
	}

	return c;
}

/**
 * Store UTF-8 character specified as 4byte integer into the dst buffer.
 * Returns number of written bytes (4 max), expects that dst has enough space.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F: 	0xxxxxxx
 * 00000080 -- 000007FF: 	110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF: 	1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF: 	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 */
static unsigned int pututf8(char *dst, int32_t value)
{
	if (value < 0x80) {
		/* one byte character */
		dst[0] = value;

		return 1;
	} else if (value < 0x800) {
		/* two bytes character */
		dst[0] = 0xc0 | (value >> 6);
	    dst[1] = 0x80 | (value & 0x3f);

		return 2;
	} else if (value < 0x10000) {
		/* three bytes character */
		dst[0] = 0xe0 | (value >> 12);
	    dst[1] = 0x80 | ((value >> 6) & 0x3f);
	    dst[2] = 0x80 | (value & 0x3f);

		return 3;
	} else if (value < 0x200000) {
		/* four bytes character */
		dst[0] = 0xf0 | (value >> 18);
		dst[1] = 0x80 | ((value >> 12) & 0x3f);
		dst[2] = 0x80 | ((value >> 6) & 0x3f);
		dst[3] = 0x80 | (value & 0x3f);

		return 4;
	} else {
		/* out of range */
		LY_ERR(LY_EINVAL, NULL);
		return 0;
	}
}

static int parse_ignore(const char *data, const char *endstr,
		                unsigned int *len)
{
	unsigned int slen;
	const char *c = data;

	slen = strlen(endstr);

	while (*c && memcmp(c, endstr, slen)) {
		c++;
	}
	if (!*c) {
		LY_ERR(LY_EWELLFORM, "Missing close sequence \"%s\".", endstr);
		return EXIT_FAILURE;
	}
	c += slen;

	*len = c - data;
	return EXIT_SUCCESS;
}

static char *parse_text(const char *data, char delim, unsigned int *len)
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
			LY_ERR(LY_EWELLFORM, "Invalid element content, \"]]>\" found.");
			goto error;
		}

loop:

		if (o > BUFSIZE - 3) {
			/* add buffer into the result */
			if (result) {
				size = size + o;
				aux = realloc(result, size + 1);
				result = aux;
			} else {
				size = o;
				result = malloc((size + 1) * sizeof *result);
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
				o--; /* we don't write any data in this iteration */
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
					LY_ERR(LY_EWELLFORM,
					       "Invalid entity reference, only predefined entity references are supported.");
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
						LY_ERR(LY_EWELLFORM,
						       "Invalid character reference, missing semicolon.");
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
					LY_ERR(LY_EWELLFORM, "Invalid character reference.");
					goto error;

				}
				r = pututf8(&buf[o], n);
				if (!r) {
					LY_ERR(LY_EWELLFORM, "Invalid character reference value.");
					goto error;
				}
				o += r - 1; /* o is ++ in for loop */
				(*len)++;
			}
		} else {
			buf[o] = data[*len];
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
			aux = realloc(result, *len + 1);
			result = aux;
		} else {
			size = o;
			result = malloc(size * sizeof *result);
		}
		memcpy(&result[size - o], buf, o);
	}
	result[size] = '\0';

	return result;

error:
	free(result);
	return NULL;
}

static struct lyxml_attr *parse_attr(const char *data, unsigned int *len)
{
	const char *c = data, *delim;
	int uc;
	struct lyxml_attr *attr = NULL;
	unsigned int size;

	/* process name part of the attribute */
	uc = getutf8(c, &size);
	if (!is_xmlnamestartchar(uc)) {
		LY_ERR(LY_EWELLFORM, "Invalid NameStartChar of the attribute");
		return NULL;
	}
	c += size;
	uc = getutf8(c, &size);
	while (is_xmlnamechar(uc)) {
		c += size;
		uc = getutf8(c, &size);
	}

	attr = calloc(1, sizeof *attr);

	/* store the name */
	size = c - data;
	attr->name = malloc((size + 1) * sizeof *attr->name);
	memcpy(attr->name, data, size);
	attr->name[size] = '\0';

	/* check Eq mark that can be surrounded by whitespaces */
	ign_xmlws(c);
	if (*c != '=') {
		LY_ERR(LY_EWELLFORM, "Invalid attribute definition, \"=\" expected.");
		goto error;
	}
	c++;
	ign_xmlws(c);

	/* process value part of the attribute */
	if (!*c || (*c != '"' && *c != '\'')) {
		LY_ERR(LY_EWELLFORM, "Invalid attribute value, \" or \' expected.");
		goto error;
	}
	delim = c;
	attr->value = parse_text(++c, *delim, &size);
	if (ly_errno) {
		goto error;
	}

	*len = c + size + 1 - data; /* +1 is delimiter size */
	return attr;

error:
	lyxml_free_attr(attr);
	return NULL;
}

static struct lyxml_elem *parse_elem(const char *data, unsigned int *len)
{
	const char *c = data, *e;
	const char *lws; /* leading white space for handling mixed content */
	int uc;
	char *str;
	struct lyxml_elem *elem, *child;
	struct lyxml_attr *attr;
	unsigned int size;

	*len = 0;

	if (*c != '<') {
		return NULL;
	}

	/* locate element name */
	c++;
	e = c;

	uc = getutf8(e, &size);
	if (!is_xmlnamestartchar(uc)) {
		LY_ERR(LY_EWELLFORM, "Invalid NameStartChar of the attribute");
		return NULL;
	}
	e += size;
	uc = getutf8(e, &size);
	while (is_xmlnamechar(uc)) {
		e += size;
		uc = getutf8(e, &size);
	}
	if (!*e) {
		LY_ERR(LY_EWELLFORM, "Unexpected end of input data.");
		return NULL;
	}

	/* allocate element structure */
	elem = calloc(1, sizeof *elem);
	elem->next = elem;
	elem->prev = elem;

	/* store the name into the element structure */
	elem->name = malloc((e - c + 1) * sizeof *elem->name);
	memcpy(elem->name, c, e - c);
	elem->name[e - c] = '\0';
	c = e;

process:
	ly_errno = 0;
	ign_xmlws(c);
	if (!memcmp("/>", c, 2)) {
		/* we are done, it was EmptyElemTag */
		c += 2;
	} else if (*c == '>') {
		/* process element content */
		c++;
		lws = NULL;

		while (*c) {
			if (!memcmp(c, "</", 2)) {
				if (lws) {
					/* leading white spaces were actually content */
					goto store_content;
				}

				/* Etag */
				c += 2;
				/* get name and check it */
				e = c;
				uc = getutf8(e, &size);
				if (!is_xmlnamestartchar(uc)) {
					LY_ERR(LY_EWELLFORM,
					       "Invalid NameStartChar of the attribute");
					goto error;
				}
				e += size;
				uc = getutf8(e, &size);
				while (is_xmlnamechar(uc)) {
					e += size;
					uc = getutf8(e, &size);
				}
				if (!*e) {
					LY_ERR(LY_EWELLFORM, "Unexpected end of input data.");
					goto error;
				}

				/* check that it corresponds to opening tag */
				size = e - c;
				str = malloc((size + 1) * sizeof *str);
				memcpy(str, c, e - c);
				str[e - c] = '\0';
				if (size != strlen(elem->name) ||
						memcmp(str, elem->name, size)) {
					LY_ERR(LY_EWELLFORM,
					       "Mixed opening (%s) and closing (%s) element tag",
					       elem->name);
					goto error;
				}
				free(str);
				c = e;

				ign_xmlws(c);
				if (*c != '>') {
					LY_ERR(LY_EWELLFORM,
					       "Close element tag \"%s\" contain additional data.",
					       elem->name);
					goto error;
				}
				c++;
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
					/* leading white spaces were only formatting */
					lws = NULL;
				}
				if (elem->content) {
					/* we have a mixed content */
					child = calloc(1, sizeof *child);
					child->content = elem->content;
					elem->content = NULL;
					lyxml_add_child(elem, child);
				}
				child = parse_elem(c, &size);
				if (!child) {
					LY_ERR(LY_EWELLFORM, "Unexpected end of input data.");
					goto error;
				}
				lyxml_add_child(elem, child);
				c += size; /* move after processed child element */
			} else if (is_xmlws(*c)) {
				lws = c;
				ign_xmlws(c);
			} else {
store_content:
				/* store text content */
				if (lws) {
					/* process content including the leading white spaces */
					c = lws;
					lws = NULL;
				}
				elem->content = parse_text(c, '<', &size);
				if (ly_errno) {
					goto error;
				}
				c += size; /* move after processed text content */

				if (elem->child) {
					/* we have a mixed content */
					child = calloc(1, sizeof *child);
					child->content = elem->content;
					elem->content = NULL;
					lyxml_add_child(elem, child);
				}
			}
		}
	} else {
		/* process attribute */
		attr = parse_attr(c, &size);
		if (!attr) {
			LY_ERR(LY_EWELLFORM, "Unexpected end of input data.");
			goto error;
		}
		lyxml_add_attr(elem, attr);
		c += size; /* move after processed attribute */

		/* go back to finish element processing */
		goto process;
	}

	*len = c - data;

	return elem;

error:
	lyxml_free_elem(elem);

	return NULL;
}

API struct lyxml_elem *lyxml_read(const char *data, int UNUSED(options))
{
	const char *c = data;
	unsigned int len;
	struct lyxml_elem *root = NULL;

	if (!data) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	/* process document */
	while (*c) {
		if (is_xmlws(*c)) {
			/* skip whitespaces */
			c++;
		} else if (!memcmp(c, "<?", 2)) {
			/* XMLDecl or PI - ignore it */
			c += 2;
			if (parse_ignore(c, "?>", &len)) {
				LY_ERR(LY_EWELLFORM, "Missing close sequence \"?>\".");
				return NULL;
			}
			c += len;
		} else if (!memcmp(c, "<!--", 4)) {
			/* Comment - ignore it */
			c += 2;
			if (parse_ignore(c, "-->", &len)) {
				LY_ERR(LY_EWELLFORM, "Missing close sequence \"-->\".");
				return NULL;
			}
			c += len;
		} else if (!memcmp(c, "<!", 2)) {
			/* DOCTYPE */
			/* TODO - standalone ignore counting < and > */
			LY_ERR(LY_EINVAL, "DOCTYPE not implemented.");
			return NULL;
		} else if (*c == '<') {
			/* element - process it in next loop to strictly follow XML
			 * format
			 */
			break;
		}
	}

	root = parse_elem(c, &len);
	if (!root) {
		return NULL;
	}
	c += len;

	/* ignore the rest of document where can be comments, PIs and whitespaces,
	 * note that we are not detecting syntax errors in these parts
	 */
	ign_xmlws(c);
	if (*c) {
		LY_WRN("There are some not parsed data:\n%s", c);
	}

	return root;
}

API struct lyxml_elem *lyxml_read_fd(int fd, int UNUSED(options))
{
	if (fd == -1) {
		ly_errno = LY_EINVAL;
		return NULL;
	}

	return NULL;
}

API struct lyxml_elem *lyxml_read_file(const char *filename,
		                               int UNUSED(options))
{
	if (!filename) {
		LY_ERR(LY_EINVAL, NULL);
		return NULL;
	}

	return NULL;
}

