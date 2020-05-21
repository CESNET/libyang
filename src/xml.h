/**
 * @file xml.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic XML parser routines.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_XML_H_
#define LY_XML_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "set.h"
#include "tree_schema.h"

struct ly_out;
struct ly_prefix;

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

struct lyxml_ns {
    char *prefix;         /* prefix of the namespace, NULL for the default namespace */
    char *uri;            /* namespace URI */
    uint32_t depth;       /* depth level of the element to maintain the list of accessible namespace definitions */
};

/* element tag identifier for matching opening and closing tags */
struct lyxml_elem {
    const char *prefix;
    const char *name;
    size_t prefix_len;
    size_t name_len;
};

/**
 * @brief Status of the parser providing information what is expected next (which function is supposed to be called).
 */
enum LYXML_PARSER_STATUS {
    LYXML_ELEMENT,        /* opening XML element parsed */
    LYXML_ELEM_CLOSE,     /* closing XML element parsed */
    LYXML_ELEM_CONTENT,   /* XML element context parsed */
    LYXML_ATTRIBUTE,      /* XML attribute parsed */
    LYXML_ATTR_CONTENT,   /* XML attribute content parsed */
    LYXML_END             /* end of input data */
};

struct lyxml_ctx {
    enum LYXML_PARSER_STATUS status; /* status providing information about the last parsed object, following attributes
                                        are filled based on it */
    union {
        const char *prefix; /* LYXML_ELEMENT, LYXML_ATTRIBUTE */
        const char *value;  /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT */
    };
    union {
        size_t prefix_len;  /* LYXML_ELEMENT, LYXML_ATTRIBUTE */
        size_t value_len;   /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT */
    };
    union {
        const char *name;   /* LYXML_ELEMENT, LYXML_ATTRIBUTE */
        int ws_only;        /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT */
    };
    union {
        size_t name_len;    /* LYXML_ELEMENT, LYXML_ATTRIBUTE */
        int dynamic;        /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT */
    };

    const struct ly_ctx *ctx;
    uint64_t line;
    const char *input;
    struct ly_set elements; /* list of not-yet-closed elements */
    struct ly_set ns;       /* handled with LY_SET_OPT_USEASLIST */
};

LY_ERR lyxml_ctx_new(const struct ly_ctx *ctx, const char *input, struct lyxml_ctx **xmlctx);

LY_ERR lyxml_ctx_next(struct lyxml_ctx *xmlctx);

LY_ERR lyxml_ctx_peek(struct lyxml_ctx *xmlctx, enum LYXML_PARSER_STATUS *next);

/**
 * @brief Get a namespace record for the given prefix in the current context.
 *
 * @param[in] xmlctx XML context to work with.
 * @param[in] prefix Pointer to the namespace prefix as taken from lyxml_get_attribute() or lyxml_get_element().
 * Can be NULL for default namespace.
 * @param[in] prefix_len Length of the prefix string (since it is not NULL-terminated when returned from lyxml_get_attribute() or
 * lyxml_get_element()).
 * @return The namespace record or NULL if the record for the specified prefix not found.
 */
const struct lyxml_ns *lyxml_ns_get(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len);

/**
 * @brief Print the given @p text as XML string which replaces some of the characters which cannot appear in XML data.
 *
 * @param[in] out Output structure for printing.
 * @param[in] text String to print.
 * @param[in] attribute Flag for attribute's value where a double quotes must be replaced.
 * @return LY_ERR values.
 */
LY_ERR lyxml_dump_text(struct ly_out *out, const char *text, int attribute);

/**
 * @brief Remove the allocated working memory of the context.
 *
 * @param[in] xmlctx XML context to clear.
 */
void lyxml_ctx_free(struct lyxml_ctx *xmlctx);

/**
 * @brief Find all possible prefixes in a value.
 *
 * @param[in] xmlctx XML context to use.
 * @param[in] value Value to check.
 * @param[in] value_len Value length.
 * @param[out] val_prefs Array of found prefixes.
 * @return LY_ERR value.
 */
LY_ERR lyxml_get_prefixes(struct lyxml_ctx *xmlctx, const char *value, size_t value_len, struct ly_prefix **val_prefs);

/**
 * @brief Compare values and their prefix mappings.
 *
 * @param[in] value1 First value.
 * @param[in] prefs1 First value prefixes.
 * @param[in] value2 Second value.
 * @param[in] prefs2 Second value prefixes.
 * @return LY_SUCCESS if values are equal.
 * @return LY_ENOT if values are not equal.
 * @return LY_ERR on error.
 */
LY_ERR lyxml_value_compare(const char *value1, const struct ly_prefix *prefs1, const char *value2,
                           const struct ly_prefix *prefs2);

void *lyxml_elem_dup(void *item);

void *lyxml_ns_dup(void *item);

#endif /* LY_XML_H_ */
