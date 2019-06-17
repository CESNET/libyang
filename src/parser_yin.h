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
    YIN_ARG_UNKNOWN = 0,   /**< parsed argument can not be matched with any known yin argument keyword */
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
    YIN_ARG_NONE,          /**< */
};

struct yin_arg_record {
    const char *prefix;
    size_t prefix_len;
    const char *name;
    size_t name_len;
    char *content;
    size_t content_len;
    int dynamic_content;
};

/**
 * @brief Match argument name.
 *
 * @param[in] name String representing name.
 * @param[in] len Lenght of the name.
 *
 * @return YIN_ARGUMENT value.
 */
enum YIN_ARGUMENT match_argument_name(const char *name, size_t len);

/**
 * @brief Parse content of whole element as text.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in,out] data Data to read from.
 * @param[out] value Where content of element should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR parse_text_element(struct lyxml_context *xml_ctx, const char **data, const char **value);

/**
 * @brief Parse import element.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] module_prefix Prefix of the module to check prefix collisions.
 * @param[in,out] data Dta to read from.
 * @param[in,out] imports Parsed imports to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_import(struct lyxml_context *xml_ctx, const char *module_prefix, const char **data, struct lysp_import **imports);

#endif /* LY_PARSER_YIN_H_*/
