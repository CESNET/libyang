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

struct lyout;
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
    unsigned int depth;   /* depth level of the element to maintain the list of accessible namespace definitions */
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
    LYXML_ELEMENT = 0,    /* expecting XML element, call lyxml_get_element() */
    LYXML_ELEM_CONTENT,   /* expecting content of an element, call lyxml_get_string */
    LYXML_ATTRIBUTE,      /* expecting XML attribute, call lyxml_get_attribute() */
    LYXML_ATTR_CONTENT,   /* expecting value of an attribute, call lyxml_get_string */
    LYXML_END             /* end of input data */
};

struct lyxml_context {
    struct ly_ctx *ctx;
    uint64_t line;
    enum LYXML_PARSER_STATUS status; /* status providing information about the next expected object in input data */
    struct ly_set elements; /* list of not-yet-closed elements */
    struct ly_set ns;     /* handled with LY_SET_OPT_USEASLIST */
};

/**
 * @brief Parse input expecting an XML element.
 *
 * Able to silently skip comments, PIs and CData. DOCTYPE is not parseable, so it is reported as LY_EVALID error.
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
 * @param[out] name Element name. When LY_SUCCESS is returned but name is NULL, check context's status field:
 * - LYXML_END - end of input was reached
 * - LYXML_ELEMENT - closing element found, expecting now a sibling element so call lyxml_get_element() again
 * @param[out] name_len Length of the element name.
 * @return LY_ERR values.
 */
LY_ERR lyxml_get_element(struct lyxml_context *context, const char **input,
                         const char **prefix, size_t *prefix_len, const char **name, size_t *name_len);

/**
 * @brief Skip an element after its opening tag was parsed.
 *
 * @param[in] context XML context.
 * @param[in,out] input Input string to process, updated according to the read data.
 * @return LY_ERR values.
 */
LY_ERR lyxml_skip_element(struct lyxml_context *context, const char **input);

/**
 * @brief Parse input expecting an XML attribute (including XML namespace).
 *
 * Input string is not being modified, so the returned values are not NULL-terminated, instead their length
 * is returned.
 *
 * Namespace definitions are processed automatically and stored internally. To get namespace for a specific
 * prefix, use lyxml_get_ns(). This also means, that in case there are only the namespace definitions,
 * lyxml_get_attribute() can succeed, but nothing (name, prefix) is returned.
 *
 * The status member of the context is updated to provide information what the caller is supposed to call
 * after this function.
 *
 * @param[in] context XML context to track lines or store errors into libyang context.
 * @param[in,out] input Input string to process, updated according to the processed/read data so,
 * when succeeded, it points to the opening quote of the attribute's value.
 * @param[out] prefix Pointer to prefix if present in the attribute name, NULL otherwise.
 * @param[out] prefix_len Length of the prefix if any.
 * @param[out] name Attribute name. Can be NULL only in case there is actually no attribute, but namespaces.
 * @param[out] name_len Length of the element name.
 * @return LY_ERR values.
 */
LY_ERR lyxml_get_attribute(struct lyxml_context *context, const char **input,
                           const char **prefix, size_t *prefix_len, const char **name, size_t *name_len);

/**
 * @brief Parse input as XML text (attribute's values and element's content).
 *
 * Mixed content of XML elements is not allowed. Formating whitespaces before child element are ignored,
 * LY_EINVAL is returned in such a case (output is not set, no error is printed) and input is moved
 * to the beginning of a child definition.
 *
 * In the case of attribute's values, the input string is expected to start on a quotation mark to
 * select which delimiter (single or double quote) is used. Otherwise, the element content is being
 * parsed expected to be terminated by '<' character.
 *
 * If function succeeds, the string in a dynamically allocated output buffer is always NULL-terminated.
 *
 * The dynamically allocated buffer is used only when necessary because of a character or the supported entity
 * reference which modify the input data. These constructs are replaced by their real value, so in case the output
 * string will be again printed as an XML data, it may be necessary to correctly encode such characters.
 *
 * Optionally, the buffer, buffer_size, output, length and dynamic arguments (altogether) can be NULL.
 * In such a case, the XML text in @p input is just checked, the @p input pointer is moved after the XML text, but nothing is stored.
 *
 * @param[in] context XML context to track lines or store errors into libyang context.
 * @param[in,out] input Input string to process, updated according to the processed/read data.
 * @param[in, out] buffer Storage for the output string. If the parameter points to NULL, the buffer is allocated if needed.
 * Otherwise, when needed, the buffer is used and enlarged when necessary. Whenever the buffer is used, the string is NULL-terminated.
 * @param[in, out] buffer_size Allocated size of the returned buffer. If a buffer is provided by a caller, it
 * is not being reduced even if the string is shorter. On the other hand, it can be enlarged if needed.
 * @param[out] output Returns pointer to the resulting string - to the provided/allocated buffer if it was necessary to modify
 * the input string or directly into the input string (see the \p dynamic parameter).
 * @param[out] length Length of the \p output string.
 * @param[out] dynamic Flag if a dynamically allocated memory (\p buffer) was used and caller is supposed to free it at the end.
 * In case the value is zero, the \p output points directly into the \p input string.
 * @return LY_ERR value.
 */
LY_ERR lyxml_get_string(struct lyxml_context *context, const char **input, char **buffer, size_t *buffer_size, char **output, size_t *length, int *dynamic);

/**
 * @brief Get a namespace record for the given prefix in the current context.
 *
 * @param[in] context XML context to work with.
 * @param[in] prefix Pointer to the namespace prefix as taken from lyxml_get_attribute() or lyxml_get_element().
 * Can be NULL for default namespace.
 * @param[in] prefix_len Length of the prefix string (since it is not NULL-terminated when returned from lyxml_get_attribute() or
 * lyxml_get_element()).
 * @return The namespace record or NULL if the record for the specified prefix not found.
 */
const struct lyxml_ns *lyxml_ns_get(struct lyxml_context *context, const char *prefix, size_t prefix_len);

/**
 * @brief Print the given @p text as XML string which replaces some of the characters which cannot appear in XML data.
 *
 * @param[in] out Output structure for printing.
 * @param[in] text String to print.
 * @param[in] attribute Flag for attribute's value where a double quotes must be replaced.
 * @return LY_ERR values.
 */
LY_ERR lyxml_dump_text(struct lyout *out, const char *text, int attribute);

/**
 * @brief Remove the allocated working memory of the context.
 *
 * @param[in] context XML context to clear.
 */
void lyxml_context_clear(struct lyxml_context *context);

/**
 * @brief Find all possible prefixes in a value.
 *
 * @param[in] ctx XML context to use.
 * @param[in] value Value to check.
 * @param[in] value_len Value length.
 * @param[out] val_prefs Array of found prefixes.
 * @return LY_ERR value.
 */
LY_ERR lyxml_get_prefixes(struct lyxml_context *ctx, const char *value, size_t value_len, struct ly_prefix **val_prefs);

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

#endif /* LY_XML_H_ */
