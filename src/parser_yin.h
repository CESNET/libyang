/**
 * @file parser_yin.h
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @brief YIN parser.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_YIN_H_
#define LY_PARSER_YIN_H_

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "xml.h"

/* list of yin attribute strings */
extern const char *const yin_attr_list[];
#define yin_attr2str(STMT) yin_attr_list[STMT]

#define YIN_NS_URI "urn:ietf:params:xml:ns:yang:yin:1"

enum YIN_ARGUMENT {
    YIN_ARG_UNKNOWN = 0,   /**< parsed argument can not be matched with any supported yin argument keyword */
    YIN_ARG_NAME,          /**< argument name */
    YIN_ARG_TARGET_NODE,   /**< argument target-node */
    YIN_ARG_MODULE,        /**< argument module */
    YIN_ARG_VALUE,         /**< argument value */
    YIN_ARG_TEXT,          /**< argument text */
    YIN_ARG_CONDITION,     /**< argument condition */
    YIN_ARG_URI,           /**< argument uri */
    YIN_ARG_DATE,          /**< argument data */
    YIN_ARG_TAG,           /**< argument tag */
    YIN_ARG_XMLNS,         /**< argument xmlns */
    YIN_ARG_NONE,          /**< empty (special value) */
};

/**
 * @brief structure to store instance of xml attribute
 */
struct yin_arg_record {
    const char *prefix;   /**< start of prefix */
    size_t prefix_len;    /**< length of prefix */
    const char *name;     /**< start of name */
    size_t name_len;      /**< length of name */
    char *content;        /**< start of content */
    size_t content_len;   /**< length of content */
    int dynamic_content;  /**< is set to 1 iff content is dynamically allocated 0 otherwise */
};

/**
 * @brief Match argument name.
 *
 * @param[in] name String representing name.
 * @param[in] len Lenght of the name.
 *
 * @return YIN_ARGUMENT value.
 */
enum YIN_ARGUMENT yin_match_argument_name(const char *name, size_t len);

/**
 * @brief Parse content of whole element as text.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] args Sized array of arguments of current element.
 * @param[in,out] data Data to read from.
 * @param[out] value Where content of element should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_text_element(struct lyxml_context *xml_ctx, struct yin_arg_record **args, const char **data,
                              const char **value);

/**
 * @brief Parse import element.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] args Sized array of arguments of current element.
 * @param[in] module_prefix Prefix of the module to check prefix collisions.
 * @param[in,out] data Dta to read from.
 * @param[in,out] imports Parsed imports to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_import(struct lyxml_context *xml_ctx, struct yin_arg_record **args, const char *module_prefix,
                        const char **data, struct lysp_import **imports);

/**
 * @brief match yang keyword from yin data
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] name Name Start of keyword name
 * @param[in] name_len Lenght of keyword name.
 * @param[in] prefix Start of keyword prefix.
 * @param[in] prefix_len lenght of prefix.
 *
 * @return yang_keyword values.
 */
enum yang_keyword yin_match_keyword(struct lyxml_context *xml_ctx, const char *name, size_t name_len,
                                    const char *prefix, size_t prefix_len);

/**
 * @brief Parse status statement.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in,out] data Data to read from.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_status(struct lyxml_context *xml_ctx, struct yin_arg_record **status_args, const char **data,
                        uint16_t *flags, struct lysp_ext_instance **exts);

/**
 * @brief parse yin argument, arg_val is unchanged if argument arg_type wasn't found.
 *
 * @param[in] xml_ctx XML parser context.
 * @param[in,out] data Data to read from.
 * @param[in] arg_type Type of argument that is expected in parsed element (use YIN_ARG_NONE for elements without special arguments).
 * @param[out] arg_val Where value of argument should be stored. Can be NULL if arg_type is specified as YIN_ARG_NONE.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_attribute(struct lyxml_context *xml_ctx, struct yin_arg_record **args,
                           enum YIN_ARGUMENT arg_type, const char **arg_val);

/**
 * @brief Load all attributes from current element. Caller is supposed to free args array.
 *
 * @param[in,out] xml_ctx Xml context.
 * @param[in,out] data Data to read from, always moved to currently handled position.
 * @param[out] args Sized array of attributes.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_load_attributes(struct lyxml_context *xml_ctx, const char **data, struct yin_arg_record **args);

/**
 * @brief Parse yin-elemenet element.
 *
 * @param[in,out] xml_ctx Xml context.
 * @param[in] attrs Sized array of element attributes.
 * @param[in,out] data Data to read from, always moved to currently handled position.
 * @param[in,out] flags Flags to add to.
 * @prama[in,out] extensions Extension instance to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_yin_element_element(struct lyxml_context *xml_ctx, struct yin_arg_record **attrs, const char **data,
                                     uint16_t *flags, struct lysp_ext **extensions);

/**
 * @brief Parse the extension statement.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] extension_args Arguments of extension element.
 * @param[in,out] data Data to read from.
 * @param[in,out] extensions Extensions to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_extension(struct lyxml_context *xml_ctx, struct yin_arg_record **extension_args,
                           const char **data, struct lysp_ext **extensions);

/**
 * @brief Parse instance of extension.
 *
 * @param[in,out] xml_ctx Xml context.
 * @param[in] attrs Sized array of attributes.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] ext_name Name of the extension element.
 * @param[in] ext_name_len Length of extension name.
 * @param[in] insubstmt Type of the parrent element.
 * @param[in] insubstmt_index Index of the keyword instance this extension instance is a substatement of.
 * @param[out] exts  exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_extension_instance(struct lyxml_context *xml_ctx, struct yin_arg_record **attrs, const char **data,
                                    const char *ext_name, int ext_name_len, LYEXT_SUBSTMT insubstmt,
                                    uint32_t insubstmt_index, struct lysp_ext_instance **exts);

/**
 * @brief Parse yin element into generic structure.
 *
 * @param[in,out] xml_ctx Xml context.
 * @param[in] name Name of element.
 * @param[in] name_len Length of elements Name.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] element Where the element structure should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_element_generic(struct lyxml_context *xml_ctx, const char *name, size_t name_len,
                          const char **data, struct lysp_stmt **element);

#endif /* LY_PARSER_YIN_H_*/
