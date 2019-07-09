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
#define name2fullname(name, prefix_len) (prefix_len != 0 ? name - (prefix_len + 1) : name)
#define namelen2fulllen(name_len, prefix_len) (prefix_len != 0 ? name_len + prefix_len + 1 : name_len)

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

struct yin_parser_ctx {
    struct lyxml_context xml_ctx;  /**< context for xml parser */
    uint8_t mod_version;            /**< module's version */
};

/* flags to encode cardinality of subelement */
#define YIN_SUBELEM_MANDATORY   0x01    /**< is set when subelement is mandatory */
#define YIN_SUBELEM_UNIQUE      0x02    /**< is set when subelement is unique */
#define YIN_SUBELEM_FIRST       0x08    /**< is set when subelement is actually yang argument mapped to yin element */

#define YIN_SUBELEM_PARSED      0x80    /**< is set during parsing when given subelement is encountered for the first
                                             time to simply check validity of given constraints */

struct yin_subelement {
    enum yang_keyword type; /**< type of keyword */
    void *dest;             /**< meta infromation passed to responsible function (information about where parsed subelement should be stored) */
    uint8_t flags;          /**< describes cardianlity of subelement can be set to YIN_SUBELEM_MANDATORY and YIN_SUBELEM_UNIQUE and YIN_SUBELEM_FIRST */
};

/* helper structure just to make code look simpler */
struct sized_string {
    const char *value;
    size_t len;
};

/* Meta information passed to yin_parse_argument function,
   holds information about where content of argument element will be stored. */
struct yin_argument_meta {
    uint16_t *flags;        /**< Argument flags */
    const char **argument;  /**< Argument value */
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
 * @brief Generic function for content parsing
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] subelem_info array of valid subelement types and meta information,
 *            array must be ordered by subelem_info->type in ascending order.
 * @param[in] subelem_info_size Size of subelem_info array.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] current_element Type of current element.
 * @param[out] text_content Where the text content of element should be stored if any. Text content is ignored if set to NULL.
 * @param[in,out] exts Extension instance to add to. Can be se to null if element cannot have extension as subelements.

 * @return LY_ERR values.
 */
LY_ERR yin_parse_content(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info, signed char subelem_info_size,
                         const char **data, enum yang_keyword current_element, const char **text_content,
                         struct lysp_ext_instance **exts);

/**
 * @brief Parse import element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Sized array of attributes of import element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] mod Structure of module that is being parsed.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_import(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs,
                        const char **data, struct lysp_module *mod);

/**
 * @brief Match yang keyword from yin data.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] name Start of keyword name
 * @param[in] name_len Lenght of keyword name.
 * @param[in] prefix Start of keyword prefix.
 * @param[in] prefix_len lenght of prefix.
 *
 * @return yang_keyword values.
 */
enum yang_keyword yin_match_keyword(struct yin_parser_ctx *ctx, const char *name, size_t name_len,
                                    const char *prefix, size_t prefix_len);

/**
 * @brief Parse status element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Atributes of status element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_status(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data,
                        uint16_t *flags, struct lysp_ext_instance **exts);

/**
 * @brief Parse revision date element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Sized array of attributes of revision-date element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] rev Array to store the parsed value in.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_revision_date(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data,
                               char *rev, struct lysp_ext_instance **exts);

/**
 * @brief Load all attributes from current element. Caller is supposed to free attrs array.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] attrs Sized array of attributes.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_load_attributes(struct yin_parser_ctx *ctx, const char **data, struct yin_arg_record **attrs);

/**
 * @brief Parse yin-elemenet element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Sized array of element attributes.
 * @param[in,out] data Data to read from, always moved to currently handled position.
 * @param[in,out] flags Flags to add to.
 * @prama[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_yin_element_element(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                                     uint16_t *flags, struct lysp_ext_instance **exts);

/**
 * @brief Parse argument element.
 *
 * @param[in,out] xml_ctx Xml context.
 * @param[in] attrs Attributes of this element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] arg_meta Meta information about destionation af prased data.
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_argument_element(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data,
                                  struct yin_argument_meta *arg_meta, struct lysp_ext_instance **exts);

/**
 * @brief Parse the extension statement.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Attributes of extension element.
 * @param[in,out] data Data to read from.
 * @param[in,out] extensions Extensions to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_extension(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs,
                           const char **data, struct lysp_ext **extensions);

/**
 * @brief Parse instance of extension.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Sized array of attributes.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] ext_name Name of the extension element.
 * @param[in] ext_name_len Length of extension name.
 * @param[in] subelem Type of the keyword this extension instance is a subelement of.
 * @param[in] subelem_index Index of the keyword instance this extension instance is a subelement of
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_extension_instance(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data,
                                    const char *ext_name, int ext_name_len, LYEXT_SUBSTMT subelem,
                                    uint32_t subelem_index, struct lysp_ext_instance **exts);

/**
 * @brief Parse yin element into generic structure.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] name Name of element.
 * @param[in] name_len Length of elements Name.
 * @param[in] prefix Element prefix.
 * @param[in] prefix_len Length of element prefix.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] element Where the element structure should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_element_generic(struct yin_parser_ctx *ctx, const char *name, size_t name_len, const char *prefix,
                                 size_t prefix_len, const char **data, struct lysp_stmt **element);

#endif /* LY_PARSER_YIN_H_*/
