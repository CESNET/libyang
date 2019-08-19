/**
 * @file parser_yin.h
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @brief YIN parser header file.
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

/**
 * @brief Get start of full name including possible prefix.
 *
 * @param[in] name Start of name in input data.
 * @param[in] prefix_len Lenght of prefix.
 *
 * @return Pointer to begining of full element name including prefix.
 */
#define name2fname(name, prefix_len) (prefix_len != 0 ? name - (prefix_len + 1) : name)

/**
 * @brief Get length of full name including possible prefix.
 *
 * @param[in] name_len Lenght of name without prefix.
 * @param[in] prefix_len Lenght of prefix.
 *
 * @return Lenght of full name including possible prefix and ':' delimiter.
 */
#define len2flen(name_len, prefix_len) (prefix_len != 0 ? name_len + prefix_len + 1 : name_len)

#define VALID_VALS1 " Only valid value is \"%s\"."
#define VALID_VALS2 " Valid values are \"%s\" and \"%s\"."
#define VALID_VALS3 " Valid values are \"%s\", \"%s\" and \"%s\"."
#define VALID_VALS4 " Valid values are \"%s\", \"%s\", \"%s\" and \"%s\"."

/* shortcut to determin if keyword can in general be subelement of deviation regardles of it's type */
#define isdevsub(kw) (kw == YANG_CONFIG || kw == YANG_DEFAULT || kw == YANG_MANDATORY || \
                      kw == YANG_MAX_ELEMENTS || kw == YANG_MIN_ELEMENTS ||              \
                      kw == YANG_MUST || kw == YANG_TYPE || kw == YANG_UNIQUE ||         \
                      kw == YANG_UNITS || kw == YANG_CUSTOM)

/**
 * @brief insert string into dictionary and store as target.
 *
 * @param[in] CTX libyang context.
 * @param[out] TARGET variable where to store the pointer to the inserted value.
 * @param[in] DYNAMIC Set to 1 if STR is dynamically allocated, 0 otherwise. If set to 1, zerocopy version of lydict_insert is used.
 * @param[in] STR string to store.
 * @param[in] LEN length of the string in WORD to store.
 */
#define INSERT_STRING(CTX, TARGET, DYNAMIC, STR, LEN) \
    if (DYNAMIC) {(TARGET) = lydict_insert_zc(CTX, STR);} \
    else {(TARGET) = lydict_insert(CTX, LEN ? STR : "", LEN);}

enum yin_argument {
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

/* flags to set constraints of subelements */
#define YIN_SUBELEM_MANDATORY   0x01    /**< is set when subelement is mandatory */
#define YIN_SUBELEM_UNIQUE      0x02    /**< is set when subelement is unique */
#define YIN_SUBELEM_FIRST       0x04    /**< is set when subelement is actually yang argument mapped to yin element */
#define YIN_SUBELEM_VER2        0x08    /**< subelemnt is allowed only in modules with version at least 2 (YANG 1.1) */

#define YIN_SUBELEM_PARSED      0x80    /**< is set during parsing when given subelement is encountered for the first
                                             time to simply check validity of given constraints */

struct yin_subelement {
    enum yang_keyword type; /**< type of keyword */
    void *dest;             /**< meta infromation passed to responsible function (mostly information about where parsed subelement should be stored) */
    uint8_t flags;          /**< describes constraints of subelement can be set to YIN_SUBELEM_MANDATORY, YIN_SUBELEM_UNIQUE, YIN_SUBELEM_FIRST and YIN_SUBELEM_VER2 */
};

/* Meta information passed to yin_parse_argument function,
   holds information about where content of argument element will be stored. */
struct yin_argument_meta {
    uint16_t *flags;        /**< Argument flags */
    const char **argument;  /**< Argument value */
};

/**
 * @brief Meta information passed to functions working with tree_schema,
 *        that require additional information about parent node.
 */
struct tree_node_meta {
    struct lysp_node *parent;       /**< parent node */
    struct lysp_node **nodes;    /**< linked list of siblings */
};

/**
 * @brief Meta information passed to yin_parse_import function.
 */
struct import_meta {
    const char *prefix;             /**< module prefix. */
    struct lysp_import **imports;   /**< imports to add to. */
};

/**
 * @brief Meta information passed to yin_parse_include function.
 */
struct include_meta {
    const char *name;               /**< Module/submodule name. */
    struct lysp_include **includes; /**< [Sized array](@ref sizedarrays) of parsed includes to add to. */
};

/**
 * @brief Meta information passed to yin_parse_inout function.
 */
struct inout_meta {
    struct lysp_node *parent;          /**< Parent node. */
    struct lysp_action_inout *inout_p; /**< inout_p Input/output pointer to write to. */
};

/**
 * @brief Meta information passed to yin_parse_minmax function.
 */
struct minmax_dev_meta {
    uint32_t *lim;                      /**< min/max value to write to. */
    uint16_t *flags;                    /**< min/max flags to write to. */
    struct lysp_ext_instance **exts;    /**< extension instances to add to. */
};

/**
 * @brief Match argument name.
 *
 * @param[in] name String representing name.
 * @param[in] len Lenght of the name.
 *
 * @return yin_argument values.
 */
enum yin_argument yin_match_argument_name(const char *name, size_t len);

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
 * @param[in,out] exts Extension instance to add to. Can be set to null if element cannot have extension as subelements.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_content(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info, signed char subelem_info_size,
                         const char **data, enum yang_keyword current_element, const char **text_content,
                         struct lysp_ext_instance **exts);

/**
 * @brief Check that val is valid UTF8 character sequence of val_type.
 *        Doesn't check empty string, only character validity.
 *
 * @param[in] ctx Yin parser context for logging.
 * @param[in] val_type Type of the input string to select method of checking character validity.
 * @param[in] val Input to validate.
 * @param[in] len Length of input.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_validate_value(struct yin_parser_ctx *ctx, enum yang_arg val_type, char *val, size_t len);

/**
 * @brief Match yang keyword from yin data.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] name Start of keyword name
 * @param[in] name_len Lenght of keyword name.
 * @param[in] prefix Start of keyword prefix.
 * @param[in] prefix_len Lenght of prefix.
 * @param[in] parrent Identification of parrent element, use YANG_NONE for elements without parrent.
 *
 * @return yang_keyword values.
 */
enum yang_keyword yin_match_keyword(struct yin_parser_ctx *ctx, const char *name, size_t name_len,
                                    const char *prefix, size_t prefix_len, enum yang_keyword parrent);

/**
 * @brief Load all attributes of element into ([sized array](@ref sizedarrays)). Caller is suposed to free the array.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] attrs ([Sized array](@ref sizedarrays)) of attributes.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_load_attributes(struct yin_parser_ctx *ctx, const char **data, struct yin_arg_record **attrs);

/**
 * @brief Parse instance of extension.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of extension instance.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] ext_name Name of the extension element.
 * @param[in] ext_name_len Length of extension name.
 * @param[in] subelem Type of the keyword this extension instance is a subelement of.
 * @param[in] subelem_index Index of the keyword instance this extension instance is a subelement of
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_extension_instance(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                                    const char *ext_name, int ext_name_len, LYEXT_SUBSTMT subelem,
                                    uint32_t subelem_index, struct lysp_ext_instance **exts);

/**
 * @brief Parse yin element into generic structure.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] name Name of element.
 * @param[in] name_len Length of elements Name.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] element Where the element structure should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_element_generic(struct yin_parser_ctx *ctx, const char *name, size_t name_len, const char **data,
                                 struct lysp_stmt **element);

/**
 * @brief Parse module element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] mod_attrs Attributes of module element.
 * @param[in,out] data Data to read from.
 * @param[out] mod Parsed module structure.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_mod(struct yin_parser_ctx *ctx, struct yin_arg_record *mod_attrs,
                     const char **data, struct lysp_module *mod);


/**
 * @brief Parse submodule element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] mod_attrs Attributes of submodule element.
 * @param[in,out] data Data to read from.
 * @param[out] submod Parsed submodule structure.
 *
 * @return LY_ERR values.
 */
LY_ERR yin_parse_submod(struct yin_parser_ctx *ctx, struct yin_arg_record *mod_attrs,
                        const char **data, struct lysp_submodule *submod);

/**
 * @brief free argument record, content loaded from lyxml_get_string() can be
 * dynamically allocated in some cases so it must be also freed.
 *
 * @param ctx unused just to fulfill signature of callback for FREE_ARRAY.
 * @param[in] record Record to free.
 */
void free_arg_rec(struct yin_parser_ctx *ctx, struct yin_arg_record *record);

#endif /* LY_PARSER_YIN_H_*/
