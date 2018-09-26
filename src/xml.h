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

#include <stdint.h>

#include "context.h"
#include "set.h"

struct lyxml_ns {
    const char *element;  /* element where the namespace is defined */
    char *prefix;         /* prefix of the namespace, NULL for the default namespace */
    char *uri;            /* namespace URI */
};

enum LYXML_PARSER_STATUS {
    LYXML_STATUS_CDSECT,  /* CDATA section */
    LYXML_STATUS_COMMENT, /* XML comment */
};

struct lyxml_context {
    struct ly_ctx *ctx;
    uint64_t line;
    struct ly_set ns;     /* handled with LY_SET_OPT_USEASLIST */
};

#endif /* LY_XML_H_ */

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
LY_ERR lyxml_get_element(struct lyxml_context *context, const char **input,
                         const char **prefix, size_t *prefix_len, const char **name, size_t *name_len);

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
LY_ERR lyxml_get_attribute(struct lyxml_context *context, const char **input,
                           const char **prefix, size_t *prefix_len, const char **name, size_t *name_len);

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
LY_ERR lyxml_get_string(struct lyxml_context *context, const char **input, char **buffer, size_t *buffer_size);

/**
 * @brief Add namespace definition into XML context.
 *
 * Namespaces from a single element are supposed to be added sequentially together (not interleaved by a namespace from other
 * element). This mimic namespace visibility, since the namespace defined in element E is not visible from its parents or
 * siblings. On the other hand, namespace from a parent element can be redefined in a child element. This is also reflected
 * by lyxml_ns_get() which returns the most recent namespace definition for the given prefix.
 *
 * When leaving processing of a subtree of some element, caller is supposed to call lyxml_ns_rm() to remove all the namespaces
 * defined in such an element from the context.
 *
 * @param[in] context XML context to work with.
 * @param[in] element_name Pointer to the element name where the namespace is defined. Serve as an identifier to select
 * which namespaces are supposed to be removed via lyxml_ns_rm() when leaving the element's subtree.
 * @param[in] prefix Pointer to the namespace prefix as taken from lyxml_get_attribute(). Can be NULL for default namespace.
 * @param[in] prefix_len Length of the prefix string (since it is not NULL-terminated when returned from lyxml_get_attribute()).
 * @param[in] uri Namespace URI (value) to store. Value can be obtained via lyxml_get_string() and caller is not supposed to
 * work with the pointer when the function succeeds.
 * @return LY_ERR values.
 */
LY_ERR lyxml_ns_add(struct lyxml_context *context, const char *element_name, const char *prefix, size_t prefix_len, char *uri);

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
 * @brief Remove all the namespaces defined in the given element.
 *
 * @param[in] context XML context to work with.
 * @param[in] element_name Pointer to the element name where the namespaces are defined. Serve as an identifier previously provided
 * by lyxml_get_element()
 * @return LY_ERR values.
 */
LY_ERR lyxml_ns_rm(struct lyxml_context *context, const char *element_name);
