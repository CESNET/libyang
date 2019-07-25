/**
 * @file parser_yin.c
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
#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include "context.h"
#include "dict.h"
#include "xml.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "parser_yin.h"

/**
 * @brief check if given string is URI of yin namespace.
 * @param ns Namespace URI to check.
 *
 * @return true if ns equals YIN_NS_URI false otherwise.
 */
#define IS_YIN_NS(ns) (strcmp(ns, YIN_NS_URI) == 0)

static LY_ERR
yin_parse_config(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, uint16_t *flags,
                 struct lysp_ext_instance **exts);

const char *const yin_attr_list[] = {
    [YIN_ARG_NAME] = "name",
    [YIN_ARG_TARGET_NODE] = "target-node",
    [YIN_ARG_MODULE] = "module",
    [YIN_ARG_VALUE] = "value",
    [YIN_ARG_TEXT] = "text",
    [YIN_ARG_CONDITION] = "condition",
    [YIN_ARG_URI] = "uri",
    [YIN_ARG_DATE] = "date",
    [YIN_ARG_TAG] = "tag",
};

enum yang_keyword
yin_match_keyword(struct yin_parser_ctx *ctx, const char *name, size_t name_len,
                  const char *prefix, size_t prefix_len, enum yang_keyword parrent)
{
    const char *start = NULL;
    enum yang_keyword kw = YANG_NONE;
    const struct lyxml_ns *ns = NULL;

    if (!name || name_len == 0) {
        return YANG_NONE;
    }

    ns = lyxml_ns_get(&ctx->xml_ctx, prefix, prefix_len);
    if (ns) {
        if (!IS_YIN_NS(ns->uri)) {
            return YANG_CUSTOM;
        }
    } else {
        /* elements without namespace are automatically unknown */
        return YANG_NONE;
    }

    start = name;
    kw = lysp_match_kw(NULL, &name);

    if (name - start == (long int)name_len) {
        /* this is done because of collision in yang statement value and yang argument mapped to yin element value */
        if (kw == YANG_VALUE && parrent == YANG_ERROR_MESSAGE) {
            return YIN_VALUE;
        }
        return kw;
    } else {
        if (strncmp(start, "text", name_len) == 0) {
            return YIN_TEXT;
        } else if (strncmp(start, "value", name_len) == 0) {
            return YIN_VALUE;
        } else {
            return YANG_NONE;
        }
    }
}

enum YIN_ARGUMENT
yin_match_argument_name(const char *name, size_t len)
{
    enum YIN_ARGUMENT arg = YIN_ARG_UNKNOWN;
    size_t already_read = 0;
    LY_CHECK_RET(len == 0, YIN_ARG_NONE);

#define IF_ARG(STR, LEN, STMT) if (!strncmp((name) + already_read, STR, LEN)) {already_read+=LEN;arg=STMT;}
#define IF_ARG_PREFIX(STR, LEN) if (!strncmp((name) + already_read, STR, LEN)) {already_read+=LEN;
#define IF_ARG_PREFIX_END }

    switch (*name) {
    case 'c':
        already_read += 1;
        IF_ARG("ondition", 8, YIN_ARG_CONDITION);
        break;

    case 'd':
        already_read += 1;
        IF_ARG("ate", 3, YIN_ARG_DATE);
        break;

    case 'm':
        already_read += 1;
        IF_ARG("odule", 5, YIN_ARG_MODULE);
        break;

    case 'n':
        already_read += 1;
        IF_ARG("ame", 3, YIN_ARG_NAME);
        break;

    case 't':
        already_read += 1;
        IF_ARG_PREFIX("a", 1)
            IF_ARG("g", 1, YIN_ARG_TAG)
            else IF_ARG("rget-node", 9, YIN_ARG_TARGET_NODE)
        IF_ARG_PREFIX_END
        else IF_ARG("ext", 3, YIN_ARG_TEXT)
        break;

    case 'u':
        already_read += 1;
        IF_ARG("ri", 2, YIN_ARG_URI)
        break;

    case 'v':
        already_read += 1;
        IF_ARG("alue", 4, YIN_ARG_VALUE);
        break;
    }

    /* whole argument must be matched */
    if (already_read != len) {
        arg = YIN_ARG_UNKNOWN;
    }

#undef IF_ARG
#undef IF_ARG_PREFIX
#undef IF_ARG_PREFIX_END

    return arg;
}

/**
 * @brief free argument record, content loaded from lyxml_get_string() can be
 * dynamically allocated in some cases so it must be also freed.
 */
static void free_arg_rec(struct yin_parser_ctx *ctx, struct yin_arg_record *record) {
    (void)ctx; /* unused */
    if (record && record->dynamic_content) {
        free(record->content);
    }
}

LY_ERR
yin_load_attributes(struct yin_parser_ctx *ctx, const char **data, struct yin_arg_record **attrs)
{
    LY_ERR ret = LY_SUCCESS;
    struct yin_arg_record *argument_record = NULL;
    struct sized_string prefix, name;

    /* load all attributes */
    while (ctx->xml_ctx.status == LYXML_ATTRIBUTE) {
        ret = lyxml_get_attribute(&ctx->xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

        if (ctx->xml_ctx.status == LYXML_ATTR_CONTENT) {
            LY_ARRAY_NEW_GOTO(ctx->xml_ctx.ctx, *attrs, argument_record, ret, cleanup);
            argument_record->name = name.value;
            argument_record->name_len = name.len;
            argument_record->prefix = prefix.value;
            argument_record->prefix_len = prefix.len;
            ret = lyxml_get_string(&ctx->xml_ctx, data, &argument_record->content, &argument_record->content_len,
                                   &argument_record->content, &argument_record->content_len, &argument_record->dynamic_content);
            LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
        }
    }

cleanup:
    if (ret != LY_SUCCESS) {
        FREE_ARRAY(ctx, *attrs, free_arg_rec);
        *attrs = NULL;
    }
    return ret;
}

LY_ERR
yin_validate_value(struct yin_parser_ctx *ctx, enum yang_arg val_type, char *val, size_t len)
{
    int prefix = 0;
    unsigned int c;
    size_t utf8_char_len;
    size_t already_read = 0;
    while (already_read < len) {
        LY_CHECK_ERR_RET(ly_getutf8((const char **)&val, &c, &utf8_char_len),
                         LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INCHAR, (val)[-utf8_char_len]), LY_EVALID);
        already_read += utf8_char_len;
        LY_CHECK_ERR_RET(already_read > len, LOGINT(ctx->xml_ctx.ctx), LY_EINT);

        switch (val_type) {
        case Y_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar((struct lys_parser_ctx *)ctx, c, !already_read, NULL));
            break;
        case Y_PREF_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar((struct lys_parser_ctx *)ctx, c, !already_read, &prefix));
            break;
        case Y_STR_ARG:
        case Y_MAYBE_STR_ARG:
            LY_CHECK_RET(lysp_check_stringchar((struct lys_parser_ctx *)ctx, c));
            break;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse yin argument.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs ([Sized array](@ref sizedarrays)) of attributes.
 * @param[in,out] data Data to read from.
 * @param[in] arg_type Type of argument that is expected in parsed element (use YIN_ARG_NONE for elements without
 *            special argument).
 * @param[out] arg_val Where value of argument should be stored. Can be NULL if arg_type is specified as YIN_ARG_NONE.
 * @param[in] val_type Type of expected value of attribute.
 * @param[in] current_element Identification of current element, used for logging.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_attribute(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, enum YIN_ARGUMENT arg_type,
                    const char **arg_val, enum yang_arg val_type, enum yang_keyword current_element)
{
    enum YIN_ARGUMENT arg = YIN_ARG_UNKNOWN;
    struct yin_arg_record *iter = NULL;
    bool found = false;

    /* validation of attributes */
    LY_ARRAY_FOR(attrs, struct yin_arg_record, iter) {
        /* yin arguments represented as attributes have no namespace, which in this case means no prefix */
        if (!iter->prefix) {
            arg = yin_match_argument_name(iter->name, iter->name_len);
            if (arg == YIN_ARG_NONE) {
                continue;
            } else if (arg == arg_type) {
                LY_CHECK_ERR_RET(found, LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Duplicit definition of %s attribute in %s element",
                                 yin_attr2str(arg), ly_stmt2str(current_element)), LY_EVALID);
                found = true;
                LY_CHECK_RET(yin_validate_value(ctx, val_type, iter->content, iter->content_len));
                if (iter->dynamic_content) {
                    *arg_val = lydict_insert_zc(ctx->xml_ctx.ctx, iter->content);
                    LY_CHECK_RET(!(*arg_val), LY_EMEM);
                    /* string is no longer supposed to be freed when the sized array is freed */
                    iter->dynamic_content = 0;
                } else {
                    if (iter->content_len == 0) {
                        *arg_val = lydict_insert(ctx->xml_ctx.ctx, "", 0);
                    } else {
                        *arg_val = lydict_insert(ctx->xml_ctx.ctx, iter->content, iter->content_len);
                        LY_CHECK_RET(!(*arg_val), LY_EMEM);
                    }
                }
            } else {
                LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Unexpected attribute \"%.*s\" of %s element.", iter->name_len, iter->name, ly_stmt2str(current_element));
                return LY_EVALID;
            }
        }
    }

    /* anything else than Y_MAYBE_STR_ARG is mandatory */
    if (val_type != Y_MAYBE_STR_ARG && !found) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Missing mandatory attribute %s of %s element.", yin_attr2str(arg_type), ly_stmt2str(current_element));
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Get record with given type. Array must be sorted in ascending order by array[n].type.
 *
 * @param[in] type Type of wanted record.
 * @param[in] array_size Size of array.
 * @param[in] array Searched array.
 *
 * @return Pointer to desired record on success, NULL if element is not in the array.
 */
static struct yin_subelement *
get_record(enum yang_keyword type, signed char array_size, struct yin_subelement *array)
{
    signed char left = 0, right = array_size - 1, middle;

    while (left <= right) {
        middle = left + (right - left) / 2;

        if (array[middle].type == type) {
            return &array[middle];
        }

        if (array[middle].type < type) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }

    return NULL;
}

/**
 * @brief Helper function to check mandatory constraint of subelement.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] subelem_info Array of information about subelements.
 * @param[in] subelem_info_size Size of subelem_info array.
 * @param[in] current_element Identification of element that is currently being parsed, used for logging.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_check_subelem_mandatory_constraint(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info,
                                       signed char subelem_info_size, enum yang_keyword current_element)
{
    for (signed char i = 0; i < subelem_info_size; ++i) {
        /* if there is element that is mandatory and isn't parsed log error and return LY_EVALID */
        if (subelem_info[i].flags & YIN_SUBELEM_MANDATORY && !(subelem_info[i].flags & YIN_SUBELEM_PARSED)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Missing mandatory subelement %s of %s element.",
                          ly_stmt2str(subelem_info[i].type), ly_stmt2str(current_element));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Helper function to check "first" constraint of subelement.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] subelem_info Array of information about subelements.
 * @param[in] subelem_info_size Size of subelem_info array.
 * @param[in] current_element Identification of element that is currently being parsed, used for logging.
 * @param[in] exp_first Record in subelem_info array that is expected to be defined as first subelement.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_check_subelem_first_constraint(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info,
                                   signed char subelem_info_size, enum yang_keyword current_element,
                                   struct yin_subelement *exp_first)
{
    for (signed char i = 0; i < subelem_info_size; ++i) {
        if (subelem_info[i].flags & YIN_SUBELEM_PARSED) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Subelement %s of %s element must be defined as first subelement.",
                          ly_stmt2str(exp_first->type), ly_stmt2str(current_element));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Helper function to check if array of information about subelements is in ascending order.
 *
 * @param[in] subelem_info Array of information about subelements.
 * @param[in] subelem_info_size Size of subelem_info array.
 *
 * @return True iff subelem_info array is in ascending order, False otherwise.
 */
#ifndef NDEBUG
static bool
is_ordered(struct yin_subelement *subelem_info, signed char subelem_info_size)
{
    enum yang_keyword current = YANG_NONE; /* 0 (minimal value) */

    for (signed char i = 0; i < subelem_info_size; ++i) {
        if (subelem_info[i].type <= current) {
            return false;
        }
        current = subelem_info[i].type;
    }

    return true;
}
#endif

/**
 * @brief Parse simple element without any special constraints and argument mapped to yin attribute,
 * for example prefix or namespace element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] kw Type of current element.
 * @param[out] value Where value of attribute should be stored.
 * @param[in] arg_type Expected type of attribute.
 * @param[in] arg_val_type Type of expected value of attribute.
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_simple_element(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, enum yang_keyword kw,
                         const char **value, enum YIN_ARGUMENT arg_type, enum yang_arg arg_val_type, struct lysp_ext_instance **exts)
{
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, arg_type, value, arg_val_type, kw));
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    return yin_parse_content(ctx, subelems, 1, data, kw, NULL, exts);
}

/**
 * @brief Parse pattern element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] patterns Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_pattern(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                  struct lysp_type *type)
{
    const char *real_value = NULL;
    char *saved_value = NULL;
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, type->patterns, restr, LY_EMEM);
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &real_value, Y_STR_ARG, YANG_PATTERN));
    size_t len = strlen(real_value);

    saved_value = malloc(len + 2);
    LY_CHECK_ERR_RET(!saved_value, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    memmove(saved_value + 1, real_value, len);
    FREE_STRING(ctx->xml_ctx.ctx, real_value);
    saved_value[0] = 0x06;
    saved_value[len + 1] = '\0';
    restr->arg = lydict_insert_zc(ctx->xml_ctx.ctx, saved_value);
    LY_CHECK_ERR_RET(!restr->arg, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    type->flags |= LYS_SET_PATTERN;

    struct yin_subelement subelems[6] = {
                                            {YANG_DESCRIPTION, &restr->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_ERROR_APP_TAG, &restr->eapptag, YIN_SUBELEM_UNIQUE},
                                            {YANG_ERROR_MESSAGE, &restr->emsg, YIN_SUBELEM_UNIQUE},
                                            {YANG_MODIFIER, &restr->arg, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &restr->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };
    return yin_parse_content(ctx, subelems, 6, data, YANG_PATTERN, NULL, &restr->exts);
}

static LY_ERR
yin_parse_fracdigits(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                     struct lysp_type *type)
{
    const char *temp_val = NULL;
    char *ptr;
    unsigned long int num;

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_FRACTION_DIGITS));

    if (temp_val[0] == '\0' || (temp_val[0] == '0') || !isdigit(temp_val[0])) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "fraction-digits");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(temp_val, &ptr, 10);
    if (*ptr != '\0') {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "fraction-digits");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > 18)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "fraction-digits");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);
    type->fraction_digits = num;
    type->flags |= LYS_SET_FRDIGITS;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, &index, 0}
                                        };
    return yin_parse_content(ctx, subelems, 1, data, YANG_FRACTION_DIGITS, NULL, &type->exts);
}

/**
 * @brief Parse enum or bit element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] enum_kw Identification of actual keyword, can be set to YANG_BIT or YANG_ENUM.
 * @param[in,out] enums Enums or bits to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_enum_bit(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                   enum yang_keyword enum_kw, struct lysp_type *type)
{
    assert(enum_kw == YANG_BIT || enum_kw == YANG_ENUM);
    struct lysp_type_enum *en;
    struct lysp_type_enum **enums;

    if (enum_kw == YANG_BIT) {
        enums = &type->bits;
    } else {
        enums = &type->enums;
    }

    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *enums, en, LY_EMEM);
    type->flags |= (enum_kw == YANG_ENUM) ? LYS_SET_ENUM : LYS_SET_BIT;
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &en->name, Y_IDENTIF_ARG, enum_kw));
    if (enum_kw == YANG_ENUM) {
        LY_CHECK_RET(lysp_check_enum_name((struct lys_parser_ctx *)ctx, en->name, strlen(en->name)));
        YANG_CHECK_NONEMPTY((struct lys_parser_ctx *)ctx, strlen(en->name), "enum");
    }
    CHECK_UNIQUENESS((struct lys_parser_ctx *)ctx, *enums, name, ly_stmt2str(enum_kw), en->name);

    struct yin_subelement subelems[6] = {
                                            {YANG_DESCRIPTION, &en->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &en->iffeatures, 0},
                                            {YANG_REFERENCE, &en->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &en->flags, YIN_SUBELEM_UNIQUE},
                                            {(enum_kw == YANG_ENUM) ? YANG_VALUE : YANG_POSITION, en, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };
    return yin_parse_content(ctx, subelems, 6, data, enum_kw, NULL, &en->exts);
}

/**
 * @brief Parse simple element without any special constraints and argument mapped to yin attribute, that can have
 * more instances, such as base or if-feature.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] kw Type of current element.
 * @param[out] values Parsed values to add to.
 * @param[in] arg_type Expected type of attribute.
 * @param[in] arg_val_type Type of expected value of attribute.
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_simple_elements(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, enum yang_keyword kw,
                          const char ***values, enum YIN_ARGUMENT arg_type, enum yang_arg arg_val_type, struct lysp_ext_instance **exts)
{
    const char **value;
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *values, value, LY_EMEM);
    uint32_t index = LY_ARRAY_SIZE(*values) - 1;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, &index, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, arg_type, value, arg_val_type, kw));

    return yin_parse_content(ctx, subelems, 1, data, kw, NULL, exts);
}

/**
 * @brief Parse require instance element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @prama[out] type Type structure to store value, flag and extensions.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_pasrse_reqinstance(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs,
                       const char **data,  struct lysp_type *type)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    type->flags |= LYS_SET_REQINST;
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_REQUIRE_INSTANCE));
    if (strcmp(temp_val, "true") == 0) {
        type->require_instance = 1;
    } else if (strcmp(temp_val, "false") != 0) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "require-instance");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);

    return yin_parse_content(ctx, subelems, 1, data, YANG_REQUIRE_INSTANCE, NULL, &type->exts);
}

/**
 * @brief Parse modifier element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] pat Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_modifier(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                   const char **pat, struct lysp_ext_instance **exts)
{
    assert(**pat == 0x06);
    const char *temp_val;
    char *modified_val;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_MODIFIER));
    if (strcmp(temp_val, "invert-match") != 0) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "modifier");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xml_ctx.ctx, temp_val);

    /* allocate new value */
    modified_val = malloc(strlen(*pat) + 1);
    LY_CHECK_ERR_RET(!modified_val, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    strcpy(modified_val, *pat);
    lydict_remove(ctx->xml_ctx.ctx, *pat);

    /* modify the new value */
    modified_val[0] = 0x15;
    *pat = lydict_insert_zc(ctx->xml_ctx.ctx, modified_val);

    return yin_parse_content(ctx, subelems, 1, data, YANG_MODIFIER, NULL, exts);
}

/**
 * @brief Parse a restriction element (length, range or one instance of must).
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] restr_kw Identificaton of element that is being parsed, can be set to YANG_MUST, YANG_LENGTH or YANG_RANGE.
 * @param[in]
 */
static LY_ERR
yin_parse_restriction(struct yin_parser_ctx *ctx,  struct yin_arg_record *attrs, const char **data,
                      enum yang_keyword restr_kw, struct lysp_restr *restr)
{
    assert(restr_kw == YANG_MUST || restr_kw == YANG_LENGTH || restr_kw == YANG_RANGE);
    struct yin_subelement subelems[5] = {
                                            {YANG_DESCRIPTION, &restr->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_ERROR_APP_TAG, &restr->eapptag, YIN_SUBELEM_UNIQUE},
                                            {YANG_ERROR_MESSAGE, &restr->emsg, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &restr->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };
    /* argument of must is called condition, but argument of length and range is called value */
    enum YIN_ARGUMENT arg_type = (restr_kw == YANG_MUST) ? YIN_ARG_CONDITION : YIN_ARG_VALUE;
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, arg_type, &restr->arg, Y_STR_ARG, restr_kw));

    return yin_parse_content(ctx, subelems, 5, data, restr_kw, NULL, &restr->exts);
}

/**
 * @brief Parse must element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] restrs Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_must(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, struct lysp_restr **restrs)
{
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *restrs, restr, LY_EMEM);
    return yin_parse_restriction(ctx, attrs, data, YANG_MUST, restr);
}

/**
 * @brief Parse position or value element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] kw Type of current element, can be set to YANG_POSITION or YANG_VALUE.
 * @param[out] enm Enum structure to save value, flags and extensions.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_value_pos_element(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                            enum yang_keyword kw, struct lysp_type_enum *enm)
{
    assert(kw == YANG_POSITION || kw == YANG_VALUE);
    const char *temp_val = NULL;
    char *ptr;
    long int num;
    unsigned long int unum;

    /* set value flag */
    enm->flags |= LYS_SET_VALUE;

    /* get attribute value */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, kw));
    if (!temp_val || temp_val[0] == '\0' || (temp_val[0] == '+') ||
        ((temp_val[0] == '0') && (temp_val[1] != '\0')) || ((kw == YANG_POSITION) && !strcmp(temp_val, "-0"))) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, ly_stmt2str(kw));
        goto error;
    }

    /* convert value */
    errno = 0;
    if (kw == YANG_VALUE) {
        num = strtol(temp_val, &ptr, 10);
        if (num < INT64_C(-2147483648) || num > INT64_C(2147483647)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, ly_stmt2str(kw));
            goto error;
        }
    } else {
        unum = strtoul(temp_val, &ptr, 10);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, ly_stmt2str(kw));
            goto error;
        }
    }
    /* check if whole argument value was converted */
    if (*ptr != '\0') {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, ly_stmt2str(kw));
        goto error;
    }
    if (errno == ERANGE) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_OOB, strlen(temp_val), temp_val, ly_stmt2str(kw));
        goto error;
    }
    /* save correctly ternary operator can't be used because num and unum have different signes */
    if (kw == YANG_VALUE) {
        enm->value = num;
    } else {
        enm->value = unum;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);

    /* parse subelements */
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };
    return yin_parse_content(ctx, subelems, 1, data, kw, NULL, &enm->exts);

    error:
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
}


/**
 * @brief Parse belongs-to element.
 *
 * @param[in] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] submod Structure of submodule that is being parsed.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values
 */
static LY_ERR
yin_parse_belongs_to(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                     struct lysp_submodule *submod, struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[2] = {
                                            {YANG_PREFIX, &submod->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_MODULE, &submod->belongsto, Y_IDENTIF_ARG, YANG_BELONGS_TO));

    return yin_parse_content(ctx, subelems, 2, data, YANG_BELONGS_TO, NULL, exts);
}

/**
 * @brief Function to parse meta tags (description, contact, ...) eg. elements with
 * text element as child
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] Type of element can be set to YANG_ORGANIZATION or YANG_CONTACT or YANG_DESCRIPTION or YANG_REFERENCE.
 * @param[out] value Where the content of meta element should be stored.
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_meta_element(struct yin_parser_ctx *ctx, const char **data, enum yang_keyword elem_type,
                       const char **value, struct lysp_ext_instance **exts)
{
    assert(elem_type == YANG_ORGANIZATION || elem_type == YANG_CONTACT || elem_type == YANG_DESCRIPTION || elem_type == YANG_REFERENCE);

    struct yin_subelement subelems[2] = {
                                            {YANG_CUSTOM, NULL, 0},
                                            {YIN_TEXT, value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE | YIN_SUBELEM_FIRST}
                                        };

    return yin_parse_content(ctx, subelems, 2, data, elem_type, NULL, exts);
}

/**
 * @brief Parse error-message element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in,out] data Data to read from.
 * @param[out] value Where the content of error-message element should be stored.
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_err_msg_element(struct yin_parser_ctx *ctx, const char **data, const char **value,
                          struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[2] = {
                                            {YANG_CUSTOM, NULL, 0},
                                            {YIN_VALUE, value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE | YIN_SUBELEM_FIRST}
                                        };

    return yin_parse_content(ctx, subelems, 2, data, YANG_ERROR_MESSAGE, NULL, exts);
}

/**
 * @brief parse type element.
 *
 * @brief Parse position or value element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] type Type to wrote to.
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_type(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, struct lysp_type *type)
{
    struct yin_subelement subelems[11] = {
                                            {YANG_BASE, type, 0},
                                            {YANG_BIT, type, 0},
                                            {YANG_ENUM, type, 0},
                                            {YANG_FRACTION_DIGITS, type, YIN_SUBELEM_UNIQUE},
                                            {YANG_LENGTH, type, YIN_SUBELEM_UNIQUE},
                                            {YANG_PATH, type, YIN_SUBELEM_UNIQUE},
                                            {YANG_PATTERN, type, 0},
                                            {YANG_RANGE, type, YIN_SUBELEM_UNIQUE},
                                            {YANG_REQUIRE_INSTANCE, type, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPE, type},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &type->name, Y_PREF_IDENTIF_ARG, YANG_TYPE));
    return yin_parse_content(ctx, subelems, 11, data, YANG_TYPE, NULL, &type->exts);
}

/**
 * @brief Parse max-elements element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] max Value to write to.
 * @param[in] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_maxelements(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, uint32_t *max,
                      uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    char *ptr;
    unsigned long int num;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0},
                                        };

    *flags |= LYS_SET_MAX;
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_MAX_ELEMENTS));
    if (!temp_val || temp_val[0] == '\0' || temp_val[0] == '0' || (temp_val[0] != 'u' && !isdigit(temp_val[0]))) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "max-elements");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }

    if (strcmp(temp_val, "unbounded")) {
        errno = 0;
        num = strtoul(temp_val, &ptr, 10);
        if (*ptr != '\0') {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "max-elements");
            FREE_STRING(ctx->xml_ctx.ctx, temp_val);
            return LY_EVALID;
        }
        if ((errno == ERANGE) || (num > UINT32_MAX)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_OOB, strlen(temp_val), temp_val, "max-elements");
            FREE_STRING(ctx->xml_ctx.ctx, temp_val);
            return LY_EVALID;
        }
        *max = num;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);
    return yin_parse_content(ctx, subelems, 1, data, YANG_MAX_ELEMENTS, NULL, exts);
}

/**
 * @brief Parse max-elements element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] min Value to write to.
 * @param[in] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_minelements(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, uint32_t *min,
                      uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    char *ptr;
    unsigned long int num;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0},
                                        };

    *flags |= LYS_SET_MIN;
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_MIN_ELEMENTS));

    if (!temp_val || temp_val[0] == '\0' || (temp_val[0] == '0' && temp_val[1] != '\0')) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "min-elements");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(temp_val, &ptr, 10);
    if (ptr[0] != 0) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "min-elements");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    if (errno == ERANGE || num > UINT32_MAX) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_OOB, strlen(temp_val), temp_val, "min-elements");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    *min = num;
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);
    return yin_parse_content(ctx, subelems, 1, data, YANG_MIN_ELEMENTS, NULL, exts);
}

/**
 * @brief Parse min-elements or max-elements element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] parent Identification of parent element.
 * @param[in] current Identification of current element.
 * @param[in] dest Where the parsed value and flags should be stored.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_minmax(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                 enum yang_keyword parent, enum yang_keyword current, void *dest)
{
    assert(current == YANG_MAX_ELEMENTS || current == YANG_MIN_ELEMENTS);
    assert(parent == YANG_LEAF_LIST || parent == YANG_REFINE || parent == YANG_LIST || parent == YANG_DEVIATE);
    uint32_t *lim;
    uint16_t *flags;
    struct lysp_ext_instance **exts;

    if (parent == YANG_LEAF_LIST) {
        lim = (current == YANG_MAX_ELEMENTS) ? &((struct lysp_node_leaflist *)dest)->max : &((struct lysp_node_leaflist *)dest)->min;
        flags = &((struct lysp_node_leaflist *)dest)->flags;
        exts = &((struct lysp_node_leaflist *)dest)->exts;
    } else if (parent == YANG_REFINE) {
        lim = (current == YANG_MAX_ELEMENTS) ? &((struct lysp_refine *)dest)->max : &((struct lysp_refine *)dest)->min;
        flags = &((struct lysp_refine *)dest)->flags;
        exts = &((struct lysp_refine *)dest)->exts;
    } else if (parent == YANG_LIST) {
        lim = (current == YANG_MAX_ELEMENTS) ? &((struct lysp_node_list *)dest)->max : &((struct lysp_node_list *)dest)->min;
        flags = &((struct lysp_node_list *)dest)->flags;
        exts = &((struct lysp_node_list *)dest)->exts;
    } else {
        lim = ((struct minmax_dev_meta *)dest)->lim;
        flags = ((struct minmax_dev_meta *)dest)->flags;
        exts = ((struct minmax_dev_meta *)dest)->exts;
    }

    if (current == YANG_MAX_ELEMENTS) {
        LY_CHECK_RET(yin_parse_maxelements(ctx, attrs, data, lim, flags, exts));
    } else {
        LY_CHECK_RET(yin_parse_minelements(ctx, attrs, data, lim, flags, exts));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parser ordered-by element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[out] flags Flags to write to.
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_orderedby(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                    uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0},
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_ORDERED_BY));
    if (strcmp(temp_val, "system") == 0) {
        *flags |= LYS_ORDBY_SYSTEM;
    } else if (strcmp(temp_val, "user") == 0) {
        *flags |= LYS_ORDBY_USER;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "ordered-by");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);

    return yin_parse_content(ctx, subelems, 1, data, YANG_ORDERED_BY, NULL, exts);
}

/**
 * @brief parse any-data or any-xml element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] any_kw Identification of current element, can be set to YANG_ANY_DATA or YANG_ANY_XML
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_any(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
              enum yang_keyword any_kw, struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_anydata *any;

    /* create structure */
    any = calloc(1, sizeof *any);
    LY_CHECK_ERR_RET(!any, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    any->nodetype = (any_kw == YANG_ANYDATA) ? LYS_ANYDATA : LYS_ANYXML;
    any->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *(node_meta->siblings) = (struct lysp_node *)any;
    } else {
        for (iter = *(node_meta->siblings); iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)any;
    }

    /* parser argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &any->name, Y_IDENTIF_ARG, any_kw));

    struct yin_subelement subelems[9] = {
                                            {YANG_CONFIG, &any->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_DESCRIPTION, &any->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &any->iffeatures, 0},
                                            {YANG_MANDATORY, &any->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_MUST, &any->musts, 0},
                                            {YANG_REFERENCE, &any->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &any->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_WHEN, &any->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    return yin_parse_content(ctx, subelems, 9, data, any_kw, NULL, &any->exts);
}

/**
 * @brief parse leaf element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_leaf(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
               struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_leaf *leaf;

    /* create structure */
    leaf = calloc(1, sizeof *leaf);
    LY_CHECK_ERR_RET(!leaf, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    leaf->nodetype = LYS_LEAF;
    leaf->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *node_meta->siblings = (struct lysp_node *)leaf;
    } else {
        for (iter = *node_meta->siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)leaf;
    }

    /* parser argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &leaf->name, Y_IDENTIF_ARG, YANG_LEAF));

    /* parse content */
    struct yin_subelement subelems[12] = {
                                            {YANG_CONFIG, &leaf->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_DEFAULT, &leaf->dflt, YIN_SUBELEM_UNIQUE},
                                            {YANG_DESCRIPTION, &leaf->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &leaf->iffeatures, 0},
                                            {YANG_MANDATORY, &leaf->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_MUST, &leaf->musts, 0},
                                            {YANG_REFERENCE, &leaf->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &leaf->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPE, &leaf->type, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_MANDATORY},
                                            {YANG_UNITS, &leaf->units, YIN_SUBELEM_UNIQUE},
                                            {YANG_WHEN, &leaf->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    return yin_parse_content(ctx, subelems, 12, data, YANG_LEAF, NULL, &leaf->exts);
}

/**
 * @brief Parse leaf-list element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_leaflist(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                   struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_leaflist *llist;

    llist = calloc(1, sizeof *llist);
    LY_CHECK_ERR_RET(!llist, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    llist->nodetype = LYS_LEAFLIST;
    llist->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *node_meta->siblings = (struct lysp_node *)llist;
    } else {
        for (iter = *node_meta->siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)llist;
    }

    /* parse argument */
    yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &llist->name, Y_IDENTIF_ARG, YANG_LEAF_LIST);

    /* parse content */
    struct yin_subelement subelems[14] = {
                                            {YANG_CONFIG, &llist->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_DEFAULT, &llist->dflts, 0},
                                            {YANG_DESCRIPTION, &llist->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &llist->iffeatures, 0},
                                            {YANG_MAX_ELEMENTS, llist, YIN_SUBELEM_UNIQUE},
                                            {YANG_MIN_ELEMENTS, llist, YIN_SUBELEM_UNIQUE},
                                            {YANG_MUST, &llist->musts, 0},
                                            {YANG_ORDERED_BY, &llist->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &llist->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &llist->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPE, &llist->type, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_MANDATORY},
                                            {YANG_UNITS, &llist->units, YIN_SUBELEM_UNIQUE},
                                            {YANG_WHEN, &llist->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 14, data, YANG_LEAF_LIST, NULL, &llist->exts));

    /* invalid combination of subelements */
    if ((llist->min) && (llist->dflts)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INCHILDSTMSCOMB, "min-elements", "default", "leaf-list");
        return LY_EVALID;
    }
    if (llist->max && llist->min > llist->max) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SEMANTICS,
                    "Invalid combination of min-elements and max-elements: min value %u is bigger than the max value %u.",
                    llist->min, llist->max);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse typedef element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] typedef_meta Meta information about parent node and typedefs to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_typedef(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                  struct typedef_meta *typedef_meta)
{
    struct lysp_tpdf *tpdf;
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *typedef_meta->typedefs, tpdf, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &tpdf->name, Y_IDENTIF_ARG, YANG_TYPEDEF));

    /* parse content */
    struct yin_subelement subelems[7] = {
                                            {YANG_DEFAULT, &tpdf->dflt, YIN_SUBELEM_UNIQUE},
                                            {YANG_DESCRIPTION, &tpdf->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &tpdf->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &tpdf->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPE, &tpdf->type, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_MANDATORY},
                                            {YANG_UNITS, &tpdf->units, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 7, data, YANG_TYPEDEF, NULL, &tpdf->exts));

    /* store data for collision check */
    if (typedef_meta->parent && !(typedef_meta->parent->nodetype & (LYS_GROUPING | LYS_ACTION | LYS_INOUT | LYS_NOTIF))) {
        ly_set_add(&ctx->tpdfs_nodes, typedef_meta->parent, 0);
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse refine element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] refines Refines to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_refine(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                 struct lysp_refine **refines)
{
    struct lysp_refine *rf;

    /* allocate new refine */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *refines, rf, LY_EMEM);

    /* parse attribute */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_TARGET_NODE, &rf->nodeid, Y_STR_ARG, YANG_REFINE));
    YANG_CHECK_NONEMPTY((struct lys_parser_ctx *)ctx, strlen(rf->nodeid), "refine");

    /* parse content */
    struct yin_subelement subelems[11] = {
                                            {YANG_CONFIG, &rf->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_DEFAULT, &rf->dflts, 0},
                                            {YANG_DESCRIPTION, &rf->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &rf->iffeatures, 0},
                                            {YANG_MANDATORY, &rf->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_MAX_ELEMENTS, rf, YIN_SUBELEM_UNIQUE},
                                            {YANG_MIN_ELEMENTS, rf, YIN_SUBELEM_UNIQUE},
                                            {YANG_MUST, &rf->musts, 0},
                                            {YANG_PRESENCE, &rf->presence, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &rf->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    return yin_parse_content(ctx, subelems, 11, data, YANG_REFINE, NULL, &rf->exts);
}

/**
 * @brief Parse uses element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_uses(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
               struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_uses *uses;

    /* create structure */
    uses = calloc(1, sizeof *uses);
    LY_CHECK_ERR_RET(!uses, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    uses->nodetype = LYS_USES;
    uses->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *node_meta->siblings = (struct lysp_node *)uses;
    } else {
        for (iter = *node_meta->siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)uses;
    }

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &uses->name, Y_PREF_IDENTIF_ARG, YANG_USES));

    /* parse content */
    struct augment_meta augments = {(struct lysp_node *)uses, &uses->augments};
    struct yin_subelement subelems[8] = {
                                            {YANG_AUGMENT, &augments, 0},
                                            {YANG_DESCRIPTION, &uses->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &uses->iffeatures, 0},
                                            {YANG_REFERENCE, &uses->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFINE, &uses->refines, 0},
                                            {YANG_STATUS, &uses->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_WHEN, &uses->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 8, data, YANG_USES, NULL, &uses->exts));
    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, NULL, uses->augments, NULL, NULL));

    return LY_SUCCESS;
}

/**
 * @brief Parse revision element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] revs Parsed revisions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_revision(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                   struct lysp_revision **revs)
{
    struct lysp_revision *rev;
    const char *temp_date = NULL;

    /* allocate new reivison */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *revs, rev, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_DATE, &temp_date, Y_STR_ARG, YANG_REVISION));
    /* check value */
    if (lysp_check_date((struct lys_parser_ctx *)ctx, temp_date, strlen(temp_date), "revision")) {
        FREE_STRING(ctx->xml_ctx.ctx, temp_date);
        return LY_EVALID;
    }
    strcpy(rev->date, temp_date);
    FREE_STRING(ctx->xml_ctx.ctx, temp_date);

    /* parse content */
    struct yin_subelement subelems[3] = {
                                            {YANG_DESCRIPTION, &rev->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &rev->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    return yin_parse_content(ctx, subelems, 3, data, YANG_REVISION, NULL, &rev->exts);
}

/**
 * @brief Parse include element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] inc_meta Meta informatinou about module/submodule name and includes to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_include(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                  struct include_meta *inc_meta)
{
    struct lysp_include *inc;

    /* allocate new include */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *inc_meta->includes, inc, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_MODULE, &inc->name, Y_IDENTIF_ARG, YANG_INCLUDE));

    /* submodules share the namespace with the module names, so there must not be
     * a module of the same name in the context, no need for revision matching */
    if (!strcmp(inc_meta->name, inc->name) || ly_ctx_get_module_latest(ctx->xml_ctx.ctx, inc->name)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YANG,
                      "Name collision between module and submodule of name \"%s\".", inc->name);
        return LY_EVALID;
    }

    /* parse content */
    struct yin_subelement subelems[4] = {
                                            {YANG_DESCRIPTION, &inc->dsc, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_VER2},
                                            {YANG_REFERENCE, &inc->ref, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_VER2},
                                            {YANG_REVISION_DATE, &inc->rev, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    return yin_parse_content(ctx, subelems, 4, data, YANG_INCLUDE, NULL, &inc->exts);
}

/**
 * @brief Parse feature element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] features Features to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_feature(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                  struct lysp_feature **features)
{
    struct lysp_feature *feat;

    /* allocate new feature */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *features, feat, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &feat->name, Y_IDENTIF_ARG, YANG_FEATURE));

    /* parse content */
    struct yin_subelement subelems[5] = {
                                            {YANG_DESCRIPTION, &feat->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &feat->iffeatures, 0},
                                            {YANG_REFERENCE, &feat->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &feat->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    return yin_parse_content(ctx, subelems, 5, data, YANG_FEATURE, NULL, &feat->exts);
}

/**
 * @brief Parse identity element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] identities Identities to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_identity(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                   struct lysp_ident **identities)
{
    struct lysp_ident *ident;

    /* allocate new identity */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *identities, ident, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &ident->name, Y_IDENTIF_ARG, YANG_IDENTITY));

    /* parse content */
    struct yin_subelement subelems[6] = {
                                            {YANG_BASE, &ident->bases, 0},
                                            {YANG_DESCRIPTION, &ident->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &ident->iffeatures, YIN_SUBELEM_VER2},
                                            {YANG_REFERENCE, &ident->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &ident->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                        };
    return yin_parse_content(ctx, subelems, 6, data, YANG_IDENTITY, NULL, &ident->exts);
}

/**
 * @brief Parse list element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_list(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
               struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_list *list;

    /* create structure */
    list = calloc(1, sizeof *list);
    LY_CHECK_ERR_RET(!list, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    list->nodetype = LYS_LIST;
    list->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *node_meta->siblings = (struct lysp_node *)list;
    } else {
        for (iter = *node_meta->siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)list;
    }

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &list->name, Y_IDENTIF_ARG, YANG_LIST));

    /* parse list content */
    struct action_meta act_meta = {(struct lysp_node *)list, &list->actions};
    struct tree_node_meta new_node_meta = {(struct lysp_node *)list, &list->child};
    struct typedef_meta typedef_meta = {(struct lysp_node *)list, &list->typedefs};
    struct notif_meta notif_meta = {(struct lysp_node *)list, &list->notifs};
    struct grouping_meta gr_meta = {(struct lysp_node *)list, &list->groupings};
    struct yin_subelement subelems[25] = {
                                            {YANG_ACTION, &act_meta, 0},
                                            {YANG_ANYDATA, &new_node_meta, 0},
                                            {YANG_ANYXML, &new_node_meta, 0},
                                            {YANG_CHOICE, &new_node_meta, 0},
                                            {YANG_CONFIG, &list->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_CONTAINER, &new_node_meta, 0},
                                            {YANG_DESCRIPTION, &list->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_GROUPING, &gr_meta, 0},
                                            {YANG_IF_FEATURE, &list->iffeatures, 0},
                                            {YANG_KEY, &list->key, YIN_SUBELEM_UNIQUE},
                                            {YANG_LEAF, &new_node_meta, 0},
                                            {YANG_LEAF_LIST, &new_node_meta, 0},
                                            {YANG_LIST, &new_node_meta, 0},
                                            {YANG_MAX_ELEMENTS, list, YIN_SUBELEM_UNIQUE},
                                            {YANG_MIN_ELEMENTS, list, YIN_SUBELEM_UNIQUE},
                                            {YANG_MUST, &list->musts, 0},
                                            {YANG_NOTIFICATION, &notif_meta, 0},
                                            {YANG_ORDERED_BY, &list->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &list->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &list->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPEDEF, &typedef_meta, 0},
                                            {YANG_UNIQUE, &list->uniques, 0},
                                            {YANG_USES, &new_node_meta, 0},
                                            {YANG_WHEN, &list->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 25, data, YANG_LIST, NULL, &list->exts));

    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, list->groupings, NULL, list->actions, list->notifs));

    if (list->max && list->min > list->max) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SEMANTICS,
                    "Invalid combination of min-elements and max-elements: min value %u is bigger than the max value %u.",
                    list->min, list->max);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse notification element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] notif_meta Meta information about parent node and notifications to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_notification(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                       struct notif_meta *notif_meta)
{
    struct lysp_notif *notif;

    /* allocate new notification */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *notif_meta->notifs, notif, LY_EMEM);
    notif->nodetype = LYS_NOTIF;
    notif->parent = notif_meta->parent;

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &notif->name, Y_IDENTIF_ARG, YANG_NOTIFICATION));

    /* parse notification content */
    struct tree_node_meta node_meta = {(struct lysp_node *)notif, &notif->data};
    struct typedef_meta typedef_meta = {(struct lysp_node *)notif, &notif->typedefs};
    struct grouping_meta gr_meta = {(struct lysp_node *)notif, &notif->groupings};
    struct yin_subelement subelems[16] = {
                                            {YANG_ANYDATA, &node_meta, 0},
                                            {YANG_ANYXML, &node_meta, 0},
                                            {YANG_CHOICE, &node_meta, 0},
                                            {YANG_CONTAINER, &node_meta, 0},
                                            {YANG_DESCRIPTION, &notif->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_GROUPING, &gr_meta, 0},
                                            {YANG_IF_FEATURE, &notif->iffeatures, 0},
                                            {YANG_LEAF, &node_meta, 0},
                                            {YANG_LEAF_LIST, &node_meta, 0},
                                            {YANG_LIST, &node_meta, 0},
                                            {YANG_MUST, &notif->musts, YIN_SUBELEM_VER2},
                                            {YANG_REFERENCE, &notif->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &notif->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPEDEF, &typedef_meta, 0},
                                            {YANG_USES, &node_meta, 0},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 16, data, YANG_NOTIFICATION, NULL, &notif->exts));

    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, notif->groupings, NULL, NULL, NULL));

    return LY_SUCCESS;
}

/**
 * @brief Parse notification element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] notif_meta Meta information about parent node and notifications to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_grouping(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                   struct grouping_meta *gr_meta)
{
    struct lysp_grp *grp;

    /* create new grouping */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *gr_meta->groupings, grp, LY_EMEM);
    grp->nodetype = LYS_GROUPING;
    grp->parent = gr_meta->parent;

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &grp->name, Y_IDENTIF_ARG, YANG_GROUPING));

    /* parse grouping content */
    struct action_meta act_meta = {(struct lysp_node *)grp, &grp->actions};
    struct tree_node_meta node_meta = {(struct lysp_node *)grp, &grp->data};
    struct typedef_meta typedef_meta = {(struct lysp_node *)grp, &grp->typedefs};
    struct grouping_meta sub_grouping = {(struct lysp_node *)grp, &grp->groupings};
    struct notif_meta notif_meta = {(struct lysp_node *)grp, &grp->notifs};
    struct yin_subelement subelems[16] = {
                                            {YANG_ACTION, &act_meta, 0},
                                            {YANG_ANYDATA, &node_meta, 0},
                                            {YANG_ANYXML, &node_meta, 0},
                                            {YANG_CHOICE, &node_meta, 0},
                                            {YANG_CONTAINER, &node_meta, 0},
                                            {YANG_DESCRIPTION, &grp->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_GROUPING, &sub_grouping, 0},
                                            {YANG_LEAF, &node_meta, 0},
                                            {YANG_LEAF_LIST, &node_meta, 0},
                                            {YANG_LIST, &node_meta, 0},
                                            {YANG_NOTIFICATION, &notif_meta, 0},
                                            {YANG_REFERENCE, &grp->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &grp->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPEDEF, &typedef_meta, 0},
                                            {YANG_USES, &node_meta, 0},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 16, data, YANG_GROUPING, NULL, &grp->exts));
    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, grp->groupings, NULL, grp->actions, grp->notifs));

    return LY_SUCCESS;
}

/**
 * @brief Parse list element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_container(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                    struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_container *cont;

    /* create new container */
    cont = calloc(1, sizeof *cont);
    LY_CHECK_ERR_RET(!cont, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    cont->nodetype = LYS_CONTAINER;
    cont->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *node_meta->siblings = (struct lysp_node *)cont;
    } else {
        for (iter = *node_meta->siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)cont;
    }

    /* parse aegument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME,  &cont->name, Y_IDENTIF_ARG, YANG_CONTAINER));

    /* parse container content */
    struct action_meta act_meta = {(struct lysp_node *)cont, &cont->actions};
    struct tree_node_meta new_node_meta = {(struct lysp_node *)cont, &cont->child};
    struct grouping_meta grp_meta = {(struct lysp_node *)cont, &cont->groupings};
    struct typedef_meta typedef_meta = {(struct lysp_node *)cont, &cont->typedefs};
    struct notif_meta notif_meta = {(struct lysp_node *)cont, &cont->notifs};
    struct yin_subelement subelems[21] = {
                                            {YANG_ACTION, &act_meta, YIN_SUBELEM_VER2},
                                            {YANG_ANYDATA, &new_node_meta, YIN_SUBELEM_VER2},
                                            {YANG_ANYXML, &new_node_meta, 0},
                                            {YANG_CHOICE, &new_node_meta, 0},
                                            {YANG_CONFIG, &cont->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_CONTAINER, &new_node_meta, 0},
                                            {YANG_DESCRIPTION, &cont->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_GROUPING, &grp_meta, 0},
                                            {YANG_IF_FEATURE, &cont->iffeatures, 0},
                                            {YANG_LEAF, &new_node_meta, 0},
                                            {YANG_LEAF_LIST, &new_node_meta, 0},
                                            {YANG_LIST, &new_node_meta, 0},
                                            {YANG_MUST, &cont->musts, 0},
                                            {YANG_NOTIFICATION, &notif_meta, YIN_SUBELEM_VER2},
                                            {YANG_PRESENCE, &cont->presence, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &cont->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &cont->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPEDEF, &typedef_meta, 0},
                                            {YANG_USES, &new_node_meta, 0},
                                            {YANG_WHEN, &cont->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 21, data, YANG_CONTAINER, NULL, &cont->exts));
    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, cont->groupings, NULL, cont->actions, cont->notifs));

    return LY_SUCCESS;
}

/**
 * @brief Parse case element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_case(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
               struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_case *cas;

    /* create new case */
    cas = calloc(1, sizeof *cas);
    LY_CHECK_ERR_RET(!cas, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    cas->nodetype = LYS_CASE;
    cas->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *node_meta->siblings = (struct lysp_node *)cas;
    } else {
        for (iter = *node_meta->siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)cas;
    }

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &cas->name, Y_IDENTIF_ARG, YANG_CASE));

    /* parse case content */
    struct tree_node_meta new_node_meta = {(struct lysp_node *)cas, &cas->child};
    struct yin_subelement subelems[14] = {
                                            {YANG_ANYDATA, &new_node_meta, YIN_SUBELEM_VER2},
                                            {YANG_ANYXML, &new_node_meta, 0},
                                            {YANG_CHOICE, &new_node_meta, 0},
                                            {YANG_CONTAINER, &new_node_meta, 0},
                                            {YANG_DESCRIPTION, &cas->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &cas->iffeatures, 0},
                                            {YANG_LEAF, &new_node_meta, 0},
                                            {YANG_LEAF_LIST, &new_node_meta, 0},
                                            {YANG_LIST, &new_node_meta, 0},
                                            {YANG_REFERENCE, &cas->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &cas->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_USES, &new_node_meta, 0},
                                            {YANG_WHEN, &cas->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    return yin_parse_content(ctx, subelems, 14, data, YANG_CASE, NULL, &cas->exts);
}

/**
 * @brief Parse choice element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] node_meta Meta information about parent node and siblings.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_choice(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                 struct tree_node_meta *node_meta)
{
    struct lysp_node *iter;
    struct lysp_node_choice *choice;

    /* create new choice */
    choice = calloc(1, sizeof *choice);
    LY_CHECK_ERR_RET(!choice, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    choice->nodetype = LYS_CHOICE;
    choice->parent = node_meta->parent;

    /* insert into siblings */
    if (!*(node_meta->siblings)) {
        *node_meta->siblings = (struct lysp_node *)choice;
    } else {
        for (iter = *node_meta->siblings; iter->next; iter = iter->next);
        iter->next = (struct lysp_node *)choice;
    }

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &choice->name, Y_IDENTIF_ARG, YANG_CHOICE));

    /* parse choice content */
    struct tree_node_meta new_node_meta = {(struct lysp_node *)choice, &choice->child};
    struct yin_subelement subelems[17] = {
                                            {YANG_ANYDATA, &new_node_meta, YIN_SUBELEM_VER2},
                                            {YANG_ANYXML, &new_node_meta, 0},
                                            {YANG_CASE, &new_node_meta, 0},
                                            {YANG_CHOICE, &new_node_meta, YIN_SUBELEM_VER2},
                                            {YANG_CONFIG, &choice->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_CONTAINER, &new_node_meta, 0},
                                            {YANG_DEFAULT, &choice->dflt, YIN_SUBELEM_UNIQUE},
                                            {YANG_DESCRIPTION, &choice->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &choice->iffeatures, 0},
                                            {YANG_LEAF, &new_node_meta, 0},
                                            {YANG_LEAF_LIST, &new_node_meta, 0},
                                            {YANG_LIST, &new_node_meta, 0},
                                            {YANG_MANDATORY, &choice->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &choice->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &choice->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_WHEN, &choice->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    return yin_parse_content(ctx, subelems, 17, data, YANG_CHOICE, NULL, &choice->exts);
}

/**
 * @brief Parse input or output element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] inout_meta Meta information about parent node and siblings and input/output pointer to write to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_inout(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, enum yang_keyword inout_kw,
                struct inout_meta *inout_meta)
{
    /* initiate structure */
    inout_meta->inout_p->nodetype = (inout_kw == YANG_INPUT) ? LYS_INPUT : LYS_OUTPUT;
    inout_meta->inout_p->parent = inout_meta->parent;

    /* check attributes */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NONE, NULL, Y_MAYBE_STR_ARG, inout_kw));

    /* parser input/output content */
    struct tree_node_meta node_meta = {(struct lysp_node *)inout_meta->inout_p, &inout_meta->inout_p->data};
    struct grouping_meta grp_meta = {(struct lysp_node *)inout_meta->inout_p, &inout_meta->inout_p->groupings};
    struct typedef_meta typedef_meta = {(struct lysp_node *)inout_meta->inout_p, &inout_meta->inout_p->typedefs};
    struct yin_subelement subelems[12] = {
                                            {YANG_ANYDATA, &node_meta, YIN_SUBELEM_VER2},
                                            {YANG_ANYXML, &node_meta, 0},
                                            {YANG_CHOICE, &node_meta, 0},
                                            {YANG_CONTAINER, &node_meta, 0},
                                            {YANG_GROUPING, &grp_meta, 0},
                                            {YANG_LEAF, &node_meta, 0},
                                            {YANG_LEAF_LIST, &node_meta, 0},
                                            {YANG_LIST, &node_meta, 0},
                                            {YANG_MUST, &inout_meta->inout_p->musts, YIN_SUBELEM_VER2},
                                            {YANG_TYPEDEF, &typedef_meta, 0},
                                            {YANG_USES, &node_meta, 0},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 12, data, inout_kw, NULL, &inout_meta->inout_p->exts));

    /* finalize parent pointers to the reallocated items */
    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, inout_meta->inout_p->groupings,
                                                  NULL, NULL, NULL));

    return LY_SUCCESS;
}

/**
 * @brief Parse action element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] act_meta Meta information about parent node and actions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_action(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                 struct action_meta *act_meta)
{
    struct lysp_action *act;

    /* create new action */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *act_meta->actions, act, LY_EMEM);
    act->nodetype = LYS_ACTION;
    act->parent = act_meta->parent;

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &act->name, Y_IDENTIF_ARG, YANG_ACTION));

    /* parse content */
    struct grouping_meta grp_meta = {(struct lysp_node *)act, &act->groupings};
    struct typedef_meta typedef_meta = {(struct lysp_node *)act, &act->typedefs};
    struct inout_meta input = {(struct lysp_node *)act, &act->input};
    struct inout_meta output = {(struct lysp_node *)act, &act->output};
    struct yin_subelement subelems[9] = {
                                            {YANG_DESCRIPTION, &act->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_GROUPING, &grp_meta, 0},
                                            {YANG_IF_FEATURE, &act->iffeatures, 0},
                                            {YANG_INPUT, &input, YIN_SUBELEM_UNIQUE},
                                            {YANG_OUTPUT, &output, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &act->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &act->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_TYPEDEF, &typedef_meta, 0},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 9, data, YANG_ACTION, NULL, &act->exts));
    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, act->groupings, NULL, NULL, NULL));

    return LY_SUCCESS;
}

/**
 * @brief Parse augment element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of current element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] aug_meta Meta information about parent node and augments to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_augment(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                  struct augment_meta *aug_meta)
{
    struct lysp_augment *aug;

    /* create new augment */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *aug_meta->augments, aug, LY_EMEM);
    aug->nodetype = LYS_AUGMENT;
    aug->parent = aug_meta->parent;

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_TARGET_NODE, &aug->nodeid, Y_STR_ARG, YANG_AUGMENT));
    YANG_CHECK_NONEMPTY((struct lys_parser_ctx *)ctx, strlen(aug->nodeid), "augment");

    /* parser augment content */
    struct action_meta act_meta = {(struct lysp_node *)aug, &aug->actions};
    struct tree_node_meta node_meta = {(struct lysp_node *)aug, &aug->child};
    struct notif_meta notif_meta = {(struct lysp_node *)aug, &aug->notifs};
    struct yin_subelement subelems[17] = {
                                            {YANG_ACTION, &act_meta, YIN_SUBELEM_VER2},
                                            {YANG_ANYDATA, &node_meta, YIN_SUBELEM_VER2},
                                            {YANG_ANYXML, &node_meta, 0},
                                            {YANG_CASE, &node_meta, 0},
                                            {YANG_CHOICE, &node_meta, 0},
                                            {YANG_CONTAINER, &node_meta, 0},
                                            {YANG_DESCRIPTION, &aug->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_IF_FEATURE, &aug->iffeatures, 0},
                                            {YANG_LEAF, &node_meta, 0},
                                            {YANG_LEAF_LIST, &node_meta, 0},
                                            {YANG_LIST, &node_meta, 0},
                                            {YANG_NOTIFICATION, &notif_meta, YIN_SUBELEM_VER2},
                                            {YANG_REFERENCE, &aug->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &aug->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_USES, &node_meta, 0},
                                            {YANG_WHEN, &aug->when, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0},
                                         };
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 17, data, YANG_AUGMENT, NULL, &aug->exts));

    LY_CHECK_RET(lysp_parse_finalize_reallocated((struct lys_parser_ctx *)ctx, NULL, NULL, aug->actions, aug->notifs));

    return LY_SUCCESS;
}

static LY_ERR
yin_parse_deviate(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                  struct lysp_deviate **deviates)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t dev_mod;
    const char *temp_val;
    struct lysp_deviate *iter, *d;
    struct lysp_deviate_add *d_add = NULL;
    struct lysp_deviate_rpl *d_rpl = NULL;
    struct lysp_deviate_del *d_del = NULL;

    /* parse argument */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_DEVIATE));

    if (strcmp(temp_val, "not-supported") == 0) {
        dev_mod = LYS_DEV_NOT_SUPPORTED;
    } else if (strcmp(temp_val, "add") == 0) {
        dev_mod = LYS_DEV_ADD;
    } else if (strcmp(temp_val, "replace") == 0) {
        dev_mod = LYS_DEV_REPLACE;
    } else if (strcmp(temp_val, "delete") == 0) {
        dev_mod = LYS_DEV_DELETE;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL, strlen(temp_val), temp_val, "deviate");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);

    if (dev_mod == LYS_DEV_NOT_SUPPORTED) {
        d = calloc(1, sizeof *d);
        LY_CHECK_ERR_RET(!d, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
        struct yin_subelement subelems[1] = {
                                                {YANG_CUSTOM, NULL, 0}
                                            };
        ret = yin_parse_content(ctx, subelems, 1, data, YANG_DEVIATE, NULL, &d->exts);

    } else if (dev_mod == LYS_DEV_ADD) {
        d_add = calloc(1, sizeof *d_add);
        LY_CHECK_ERR_RET(!d_add, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_add;
        struct minmax_dev_meta min = {&d_add->min, &d_add->flags, &d_add->exts};
        struct minmax_dev_meta max = {&d_add->max, &d_add->flags, &d_add->exts};
        struct yin_subelement subelems[9] = {
                                                {YANG_CONFIG, &d_add->flags, YIN_SUBELEM_UNIQUE},
                                                {YANG_DEFAULT, &d_add->dflts, 0},
                                                {YANG_MANDATORY, &d_add->flags, YIN_SUBELEM_UNIQUE},
                                                {YANG_MAX_ELEMENTS, &max, YIN_SUBELEM_UNIQUE},
                                                {YANG_MIN_ELEMENTS, &min, YIN_SUBELEM_UNIQUE},
                                                {YANG_MUST, &d_add->musts, 0},
                                                {YANG_UNIQUE, &d_add->uniques, 0},
                                                {YANG_UNITS, &d_add->units, YIN_SUBELEM_UNIQUE},
                                                {YANG_CUSTOM, NULL, 0},
                                            };
        ret = yin_parse_content(ctx, subelems, 9, data, YANG_DEVIATE, NULL, &d_add->exts);

    } else if (dev_mod == LYS_DEV_REPLACE) {
        d_rpl = calloc(1, sizeof *d_rpl);
        LY_CHECK_ERR_RET(!d_rpl, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_rpl;
        struct minmax_dev_meta min = {&d_rpl->min, &d_rpl->flags, &d_rpl->exts};
        struct minmax_dev_meta max = {&d_rpl->max, &d_rpl->flags, &d_rpl->exts};
        struct yin_subelement subelems[8] = {
                                                {YANG_CONFIG, &d_rpl->flags, YIN_SUBELEM_UNIQUE},
                                                {YANG_DEFAULT, &d_rpl->dflt, YIN_SUBELEM_UNIQUE},
                                                {YANG_MANDATORY, &d_rpl->flags, YIN_SUBELEM_UNIQUE},
                                                {YANG_MAX_ELEMENTS, &max, YIN_SUBELEM_UNIQUE},
                                                {YANG_MIN_ELEMENTS, &min, YIN_SUBELEM_UNIQUE},
                                                {YANG_TYPE, &d_rpl->type, YIN_SUBELEM_UNIQUE},
                                                {YANG_UNITS, &d_rpl->units, YIN_SUBELEM_UNIQUE},
                                                {YANG_CUSTOM, NULL, 0},
                                            };
        ret = yin_parse_content(ctx, subelems, 8, data, YANG_DEVIATE, NULL, &d_rpl->exts);

    } else {
        d_del = calloc(1, sizeof *d_del);
        LY_CHECK_ERR_RET(!d_del, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_del;
        struct yin_subelement subelems[5] = {
                                                {YANG_DEFAULT, &d_del->dflts, 0},
                                                {YANG_MUST, &d_del->musts, 0},
                                                {YANG_UNIQUE, &d_del->uniques, 0},
                                                {YANG_UNITS, &d_del->units, YIN_SUBELEM_UNIQUE},
                                                {YANG_CUSTOM, NULL, 0},
                                            };
        ret = yin_parse_content(ctx, subelems, 5, data, YANG_DEVIATE, NULL, &d_del->exts);
    }
    LY_CHECK_GOTO(ret, cleanup);

    d->mod = dev_mod;
    /* insert into siblings */
    if (!*deviates) {
        *deviates = d;
    } else {
        for (iter = *deviates; iter->next; iter = iter->next);
        iter->next = d;
    }

    return ret;

cleanup:
    free(d);
    /* TODO log deviate error */
    return ret;
}

/**
 * @brief Map keyword type to substatement info.
 *
 * @param[in] kw Keyword type.
 *
 * @return correct LYEXT_SUBSTMT information.
 */
static LYEXT_SUBSTMT
kw2lyext_substmt(enum yang_keyword kw)
{
    switch (kw) {
    case YANG_ARGUMENT:
        return LYEXT_SUBSTMT_ARGUMENT;
    case YANG_BASE:
        return LYEXT_SUBSTMT_BASE;
    case YANG_BELONGS_TO:
        return LYEXT_SUBSTMT_BELONGSTO;
    case YANG_CONTACT:
        return LYEXT_SUBSTMT_CONTACT;
    case YANG_DEFAULT:
        return LYEXT_SUBSTMT_DEFAULT;
    case YANG_DESCRIPTION:
        return LYEXT_SUBSTMT_DESCRIPTION;
    case YANG_ERROR_APP_TAG:
        return LYEXT_SUBSTMT_ERRTAG;
    case YANG_ERROR_MESSAGE:
        return LYEXT_SUBSTMT_ERRMSG;
    case YANG_KEY:
        return LYEXT_SUBSTMT_KEY;
    case YANG_NAMESPACE:
        return LYEXT_SUBSTMT_NAMESPACE;
    case YANG_ORGANIZATION:
        return LYEXT_SUBSTMT_ORGANIZATION;
    case YANG_PATH:
        return LYEXT_SUBSTMT_PATH;
    case YANG_PREFIX:
        return LYEXT_SUBSTMT_PREFIX;
    case YANG_PRESENCE:
        return LYEXT_SUBSTMT_PRESENCE;
    case YANG_REFERENCE:
        return LYEXT_SUBSTMT_REFERENCE;
    case YANG_REVISION_DATE:
        return LYEXT_SUBSTMT_REVISIONDATE;
    case YANG_UNITS:
        return LYEXT_SUBSTMT_UNITS;
    case YANG_VALUE:
        return LYEXT_SUBSTMT_VALUE;
    case YANG_YANG_VERSION:
        return LYEXT_SUBSTMT_VERSION;
    case YANG_MODIFIER:
        return LYEXT_SUBSTMT_MODIFIER;
    case YANG_REQUIRE_INSTANCE:
        return LYEXT_SUBSTMT_REQINSTANCE;
    case YANG_YIN_ELEMENT:
        return LYEXT_SUBSTMT_YINELEM;
    case YANG_CONFIG:
        return LYEXT_SUBSTMT_CONFIG;
    case YANG_MANDATORY:
        return LYEXT_SUBSTMT_MANDATORY;
    case YANG_ORDERED_BY:
        return LYEXT_SUBSTMT_ORDEREDBY;
    case YANG_STATUS:
        return LYEXT_SUBSTMT_STATUS;
    case YANG_FRACTION_DIGITS:
        return LYEXT_SUBSTMT_FRACDIGITS;
    case YANG_MAX_ELEMENTS:
        return LYEXT_SUBSTMT_MAX;
    case YANG_MIN_ELEMENTS:
        return LYEXT_SUBSTMT_MIN;
    case YANG_POSITION:
        return LYEXT_SUBSTMT_POSITION;
    case YANG_UNIQUE:
        return LYEXT_SUBSTMT_UNIQUE;
    case YANG_IF_FEATURE:
        return LYEXT_SUBSTMT_IFFEATURE;
    default:
        return LYEXT_SUBSTMT_SELF;
    }
}

LY_ERR
yin_parse_content(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info, signed char subelem_info_size,
                  const char **data, enum yang_keyword current_element, const char **text_content, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    struct sized_string prefix, name;
    char *out = NULL;
    size_t out_len = 0;
    int dynamic = 0;
    struct yin_arg_record *attrs = NULL;
    enum yang_keyword kw = YANG_NONE;
    struct yin_subelement *subelem = NULL;
    struct lysp_type *type, *nested_type;

    assert(is_ordered(subelem_info, subelem_info_size));

    if (ctx->xml_ctx.status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(&ctx->xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
        /* current element has subelements as content */
        if (ret == LY_EINVAL) {
            while (ctx->xml_ctx.status == LYXML_ELEMENT) {
                ret = lyxml_get_element(&ctx->xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len);
                LY_CHECK_GOTO(ret, cleanup);
                if (!name.value) {
                    /* end of current element reached */
                    break;
                }
                ret = yin_load_attributes(ctx, data, &attrs);
                LY_CHECK_GOTO(ret, cleanup);
                kw = yin_match_keyword(ctx, name.value, name.len, prefix.value, prefix.len, current_element);

                /* check if this element can be child of current element */
                subelem = get_record(kw, subelem_info_size, subelem_info);
                if (!subelem) {
                    if (current_element == YANG_DEVIATE && isdevsub(kw)) {
                        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INDEV_YIN, ly_stmt2str(kw));
                    } else {
                        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_UNEXP_SUBELEM, name.len, name.value, ly_stmt2str(current_element));
                    }
                    ret = LY_EVALID;
                    goto cleanup;
                }

                /* TODO check relative order */

                /* flag check */
                if ((subelem->flags & YIN_SUBELEM_UNIQUE) && (subelem->flags & YIN_SUBELEM_PARSED)) {
                    /* subelement uniquenes */
                    LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Redefinition of %s element in %s element.", ly_stmt2str(kw), ly_stmt2str(current_element));
                    return LY_EVALID;
                }
                if (subelem->flags & YIN_SUBELEM_FIRST) {
                    /* subelement is supposed to be defined as first subelement */
                    ret = yin_check_subelem_first_constraint(ctx, subelem_info, subelem_info_size, current_element, subelem);
                    LY_CHECK_GOTO(ret, cleanup);
                }
                if (subelem->flags & YIN_SUBELEM_VER2) {
                    /* subelement is supported only in version 1.1 or higher */
                    if (ctx->mod_version < 2) {
                        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVCODE_INSUBELEM2, ly_stmt2str(kw), ly_stmt2str(current_element));
                        ret = LY_EVALID;
                        goto cleanup;
                    }
                }
                /* note that element was parsed for easy uniqueness check in next iterations */
                subelem->flags |= YIN_SUBELEM_PARSED;

                switch (kw) {
                /* call responsible function */
                case YANG_CUSTOM:
                    ret = yin_parse_extension_instance(ctx, attrs, data, name2fullname(name.value, prefix.len),
                                                      namelen2fulllen(name.len, prefix.len),
                                                      kw2lyext_substmt(current_element),
                                                      (subelem->dest) ? *((uint32_t*)subelem->dest) : 0, exts);
                    break;
                case YANG_ACTION:
                case YANG_RPC:
                    ret = yin_parse_action(ctx, attrs, data, (struct action_meta *)subelem->dest);
                    break;
                case YANG_ANYDATA:
                case YANG_ANYXML:
                    ret = yin_parse_any(ctx, attrs, data, kw, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_ARGUMENT:
                    ret = yin_parse_argument_element(ctx, attrs, data, (struct yin_argument_meta *)subelem->dest, exts);
                    break;
                case YANG_AUGMENT:
                    ret = yin_parse_augment(ctx, attrs, data, (struct augment_meta *)subelem->dest);
                    break;
                case YANG_BASE:
                    if (current_element == YANG_TYPE) {
                        type = (struct lysp_type *)subelem->dest;
                        ret = yin_parse_simple_elements(ctx, attrs, data, kw, &type->bases, YIN_ARG_NAME,
                                                        Y_PREF_IDENTIF_ARG, exts);
                        type->flags |= LYS_SET_BASE;
                    } else if (current_element == YANG_IDENTITY) {
                        ret = yin_parse_simple_elements(ctx, attrs, data, kw, (const char ***)subelem->dest,
                                                        YIN_ARG_NAME, Y_PREF_IDENTIF_ARG, exts);
                    } else {
                        LOGINT(ctx->xml_ctx.ctx);
                        ret = LY_EINT;
                    }
                    break;
                case YANG_BELONGS_TO:
                    ret = yin_parse_belongs_to(ctx, attrs, data, (struct lysp_submodule *)subelem->dest, exts);
                    break;
                case YANG_BIT:
                case YANG_ENUM:
                    ret = yin_parse_enum_bit(ctx, attrs, data, kw, (struct lysp_type *)subelem->dest);
                    break;
                case YANG_CASE:
                    ret = yin_parse_case(ctx, attrs, data, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_CHOICE:
                    ret = yin_parse_choice(ctx, attrs, data, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_CONFIG:
                    ret = yin_parse_config(ctx, attrs, data, (uint16_t *)subelem->dest, exts);
                    break;
                case YANG_CONTACT:
                case YANG_DESCRIPTION:
                case YANG_ORGANIZATION:
                case YANG_REFERENCE:
                    ret = yin_parse_meta_element(ctx, data, kw, (const char **)subelem->dest, exts);
                    break;
                case YANG_CONTAINER:
                    ret = yin_parse_container(ctx, attrs, data, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_DEFAULT:
                    if (subelem->flags & YIN_SUBELEM_UNIQUE) {
                        ret = yin_parse_simple_element(ctx, attrs, data, kw, (const char **)subelem->dest,
                                                       YIN_ARG_VALUE, Y_STR_ARG, exts);
                    } else {
                        ret = yin_parse_simple_elements(ctx, attrs, data, kw, (const char ***)subelem->dest,
                                                        YIN_ARG_VALUE, Y_STR_ARG, exts);
                    }
                    break;
                case YANG_DEVIATE:
                    ret = yin_parse_deviate(ctx, attrs, data, (struct lysp_deviate **)subelem->dest);
                    break;
                case YANG_DEVIATION:
                    break;
                case YANG_ERROR_APP_TAG:
                    ret = yin_parse_simple_element(ctx, attrs, data, kw, (const char **)subelem->dest,
                                                   YIN_ARG_VALUE, Y_STR_ARG, exts);
                    break;
                case YANG_ERROR_MESSAGE:
                    ret = yin_parse_err_msg_element(ctx, data, (const char **)subelem->dest, exts);
                    break;
                case YANG_EXTENSION:
                    ret = yin_parse_extension(ctx, attrs, data, (struct lysp_ext **)subelem->dest);
                    break;
                case YANG_FEATURE:
                    ret = yin_parse_feature(ctx, attrs, data, (struct lysp_feature **)subelem->dest);
                    break;
                case YANG_FRACTION_DIGITS:
                    ret = yin_parse_fracdigits(ctx, attrs, data, (struct lysp_type *)subelem->dest);
                    break;
                case YANG_GROUPING:
                    ret = yin_parse_grouping(ctx, attrs, data, (struct grouping_meta *)subelem->dest);
                    break;
                case YANG_IDENTITY:
                    ret = yin_parse_identity(ctx, attrs, data, (struct lysp_ident **)subelem->dest);
                    break;
                case YANG_IF_FEATURE:
                    ret = yin_parse_simple_elements(ctx, attrs, data, kw,
                                                    (const char ***)subelem->dest, YIN_ARG_NAME, Y_STR_ARG, exts);
                    break;
                case YANG_IMPORT:
                    ret = yin_parse_import(ctx, attrs, data, (struct lysp_module *)subelem->dest);
                    break;
                case YANG_INCLUDE:
                    ret = yin_parse_include(ctx, attrs, data, (struct include_meta *)subelem->dest);
                    break;
                case YANG_INPUT:
                case YANG_OUTPUT:
                    ret = yin_parse_inout(ctx, attrs, data, kw, (struct inout_meta *)subelem->dest);
                    break;
                case YANG_KEY:
                    ret = yin_parse_simple_element(ctx, attrs, data, kw, (const char **)subelem->dest, YIN_ARG_VALUE,
                                                   Y_STR_ARG, exts);
                    break;
                case YANG_LEAF:
                    ret = yin_parse_leaf(ctx, attrs, data, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_LEAF_LIST:
                    ret = yin_parse_leaflist(ctx, attrs, data, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_LENGTH:
                    type = (struct lysp_type *)subelem->dest;
                    type->length = calloc(1, sizeof *type->length);
                    LY_CHECK_ERR_GOTO(!type->length, LOGMEM(ctx->xml_ctx.ctx); ret = LY_EMEM, cleanup);
                    ret = yin_parse_restriction(ctx, attrs, data, kw, type->length);
                    type->flags |= LYS_SET_LENGTH;
                    break;
                case YANG_LIST:
                    ret = yin_parse_list(ctx, attrs, data, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_MANDATORY:
                    ret = yin_parse_mandatory(ctx, attrs, data, (uint16_t *)subelem->dest, exts);
                    break;
                case YANG_MAX_ELEMENTS:
                case YANG_MIN_ELEMENTS:
                    ret = yin_parse_minmax(ctx, attrs, data, current_element, kw, subelem->dest);
                    break;
                case YANG_MODIFIER:
                    ret = yin_parse_modifier(ctx, attrs, data, (const char **)subelem->dest, exts);
                    break;
                case YANG_MODULE:
                    break;
                case YANG_MUST:
                    ret = yin_parse_must(ctx, attrs, data, (struct lysp_restr **)subelem->dest);
                    break;
                case YANG_NAMESPACE:
                    ret = yin_parse_simple_element(ctx, attrs, data, kw, (const char **)subelem->dest,
                                                   YIN_ARG_URI, Y_STR_ARG, exts);
                    break;
                case YANG_NOTIFICATION:
                    ret = yin_parse_notification(ctx, attrs, data, (struct notif_meta *)subelem->dest);
                    break;
                case YANG_ORDERED_BY:
                    ret = yin_parse_orderedby(ctx, attrs, data, (uint16_t *)subelem->dest, exts);
                    break;
                case YANG_PATH:
                    type = (struct lysp_type *)subelem->dest;
                    ret = yin_parse_simple_element(ctx, attrs, data, kw, &type->path,
                                                   YIN_ARG_VALUE, Y_STR_ARG, exts);
                    type->flags |= LYS_SET_PATH;
                    break;
                case YANG_PATTERN:
                    ret = yin_parse_pattern(ctx, attrs, data, (struct lysp_type *)subelem->dest);
                    break;
                case YANG_VALUE:
                case YANG_POSITION:
                    ret = yin_parse_value_pos_element(ctx, attrs, data, kw,
                                                      (struct lysp_type_enum *)subelem->dest);
                    break;
                case YANG_PREFIX:
                    ret = yin_parse_simple_element(ctx, attrs, data, kw,
                                                   (const char **)subelem->dest, YIN_ARG_VALUE, Y_IDENTIF_ARG, exts);
                    break;
                case YANG_PRESENCE:
                    ret = yin_parse_simple_element(ctx, attrs, data, kw, (const char **)subelem->dest, YIN_ARG_VALUE,
                                                   Y_STR_ARG, exts);
                    break;
                case YANG_RANGE:
                    type = (struct lysp_type *)subelem->dest;
                    type->range = calloc(1, sizeof *type->range);
                    LY_CHECK_ERR_GOTO(!type->range, LOGMEM(ctx->xml_ctx.ctx); ret = LY_EMEM, cleanup);
                    ret = yin_parse_restriction(ctx, attrs, data, kw, type->range);
                    type->flags |=  LYS_SET_RANGE;
                    break;
                case YANG_REFINE:
                    ret = yin_parse_refine(ctx, attrs, data, (struct lysp_refine **)subelem->dest);
                    break;
                case YANG_REQUIRE_INSTANCE:
                    ret = yin_pasrse_reqinstance(ctx, attrs, data, (struct lysp_type *)subelem->dest);
                    break;
                case YANG_REVISION:
                    ret = yin_parse_revision(ctx, attrs, data, (struct lysp_revision **)subelem->dest);
                    break;
                case YANG_REVISION_DATE:
                    ret = yin_parse_revision_date(ctx, attrs, data, (char *)subelem->dest, exts);
                    break;
                case YANG_STATUS:
                    ret = yin_parse_status(ctx, attrs, data, (uint16_t *)subelem->dest, exts);
                    break;
                case YANG_SUBMODULE:
                    break;
                case YANG_TYPE:
                    if (current_element == YANG_DEVIATE) {
                        *(struct lysp_type **)subelem->dest = calloc(1, sizeof **(struct lysp_type **)subelem->dest);
                        LY_CHECK_ERR_GOTO(!(*(struct lysp_type **)subelem->dest), LOGMEM(ctx->xml_ctx.ctx); ret = LY_EMEM, cleanup);
                        type = *((struct lysp_type **)subelem->dest);
                    } else  {
                        type = (struct lysp_type *)subelem->dest;
                    }
                    /* type as child of another type */
                    if (current_element == YANG_TYPE) {
                        LY_ARRAY_NEW_GOTO(ctx->xml_ctx.ctx, type->types, nested_type, ret, cleanup);
                        type->flags |= LYS_SET_TYPE;
                        type = nested_type;
                    }
                    ret = yin_parse_type(ctx, attrs, data, type);
                    break;
                case YANG_TYPEDEF:
                    ret = yin_parse_typedef(ctx, attrs, data, (struct typedef_meta *)subelem->dest);
                    break;
                case YANG_UNIQUE:
                    ret = yin_parse_simple_elements(ctx, attrs, data, kw, (const char ***)subelem->dest,
                                                    YIN_ARG_TAG, Y_STR_ARG, exts);
                    break;
                case YANG_UNITS:
                    ret = yin_parse_simple_element(ctx, attrs, data, kw, (const char **)subelem->dest,
                                                   YIN_ARG_NAME, Y_STR_ARG, exts);
                    break;
                case YANG_USES:
                    ret = yin_parse_uses(ctx, attrs, data, (struct tree_node_meta *)subelem->dest);
                    break;
                case YANG_WHEN:
                    ret = yin_parse_when(ctx, attrs, data, (struct lysp_when **)subelem->dest);
                    break;
                case YANG_YANG_VERSION:
                    ret = yin_parse_yangversion(ctx, attrs, data, (uint8_t *)subelem->dest, exts);
                    break;
                case YANG_YIN_ELEMENT:
                    ret = yin_parse_yin_element_element(ctx, attrs, data, (uint16_t *)subelem->dest, exts);
                    break;
                case YIN_TEXT:
                case YIN_VALUE:
                    ret = yin_parse_content(ctx, NULL, 0, data, kw, (const char **)subelem->dest, NULL);
                    break;
                default:
                    LOGINT(ctx->xml_ctx.ctx);
                    return LY_EINT;
                }
                LY_CHECK_GOTO(ret, cleanup);
                FREE_ARRAY(ctx, attrs, free_arg_rec);
                attrs = NULL;
                subelem = NULL;
            }
        } else {
            /* elements with text or none content */
            /* save text content, if text_content isn't set, it's just ignored */
            /* no resources are allocated in this branch so no need to use cleanup label */
            LY_CHECK_RET(yin_validate_value(ctx, Y_STR_ARG, out, out_len));
            if (text_content) {
                if (dynamic) {
                    *text_content = lydict_insert_zc(ctx->xml_ctx.ctx, out);
                    if (!*text_content) {
                        free(out);
                        return LY_EMEM;
                    }
                } else {
                    if (out_len == 0) {
                        *text_content = lydict_insert(ctx->xml_ctx.ctx, "", 0);
                    } else {
                        *text_content = lydict_insert(ctx->xml_ctx.ctx, out, out_len);
                    }
                }
            }
            /* load closing element */
            LY_CHECK_RET(lyxml_get_element(&ctx->xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len));
        }

        /* mandatory subelemnts are checked only after whole element was succesfully parsed */
        LY_CHECK_RET(yin_check_subelem_mandatory_constraint(ctx, subelem_info, subelem_info_size, current_element));
    }

cleanup:
    FREE_ARRAY(ctx, attrs, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_revision_date(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, char *rev,
                        struct lysp_ext_instance **exts)
{
    const char *temp_rev;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_DATE, &temp_rev, Y_STR_ARG, YANG_REVISION_DATE));
    LY_CHECK_ERR_RET(lysp_check_date((struct lys_parser_ctx *)ctx, temp_rev, strlen(temp_rev), "revision-date") != LY_SUCCESS,
                     FREE_STRING(ctx->xml_ctx.ctx, temp_rev), LY_EVALID);

    strcpy(rev, temp_rev);
    FREE_STRING(ctx->xml_ctx.ctx, temp_rev);

    return yin_parse_content(ctx, subelems, 1, data, YANG_REVISION_DATE, NULL, exts);
}

/**
 * @brief Parse config element.
 *
 * @param[in] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs [Sized array](@ref sizedarrays) of attributes of import element.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_config(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, uint16_t *flags,
                 struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_CONFIG));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_CONFIG_W;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_CONFIG_R;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "config");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);

    return yin_parse_content(ctx, subelems, 1, data, YANG_CONFIG, NULL, exts);
}

LY_ERR
yin_parse_yangversion(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, uint8_t *version,
                      struct lysp_ext_instance **exts)
{
    const char *temp_version = NULL;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_version, Y_STR_ARG, YANG_YANG_VERSION));
    if (strcmp(temp_version, "1.0") == 0) {
        *version = LYS_VERSION_1_0;
    } else if (strcmp(temp_version, "1.1") == 0) {
        *version = LYS_VERSION_1_1;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_version, "yang-version");
        FREE_STRING(ctx->xml_ctx.ctx, temp_version);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_version);
    ctx->mod_version = *version;

    return yin_parse_content(ctx, subelems, 1, data, YANG_YANG_VERSION, NULL, exts);
}

LY_ERR
yin_parse_import(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, struct lysp_module *mod)
{
    struct lysp_import *imp;
    /* allocate new element in sized array for import */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, mod->imports, imp, LY_EMEM);

    struct yin_subelement subelems[5] = {
                                            {YANG_DESCRIPTION, &imp->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_PREFIX, &imp->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &imp->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_REVISION_DATE, imp->rev, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    /* parse import attributes */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_MODULE, &imp->name, Y_IDENTIF_ARG, YANG_IMPORT));
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 5, data, YANG_IMPORT, NULL, &imp->exts));
    /* check prefix validity */
    LY_CHECK_RET(lysp_check_prefix((struct lys_parser_ctx *)ctx, mod->imports, mod->mod->prefix, &imp->prefix), LY_EVALID);

    return yin_parse_content(ctx, subelems, 5, data, YANG_IMPORT, NULL, &imp->exts);
}

LY_ERR
yin_parse_mandatory(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, uint16_t *flags,
                    struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_MANDATORY));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_MAND_TRUE;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "mandatory");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);

    return yin_parse_content(ctx, subelems, 1, data, YANG_MANDATORY, NULL, exts);
}

LY_ERR
yin_parse_status(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, uint16_t *flags,
                 struct lysp_ext_instance **exts)
{
    const char *value = NULL;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &value, Y_STR_ARG, YANG_STATUS));
    if (strcmp(value, "current") == 0) {
        *flags |= LYS_STATUS_CURR;
    } else if (strcmp(value, "deprecated") == 0) {
        *flags |= LYS_STATUS_DEPRC;
    } else if (strcmp(value, "obsolete") == 0) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, value, "status");
        FREE_STRING(ctx->xml_ctx.ctx, value);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, value);

    return yin_parse_content(ctx, subelems, 1, data, YANG_STATUS, NULL, exts);
}

LY_ERR
yin_parse_when(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, struct lysp_when **when_p)
{
    struct lysp_when *when;
    when = calloc(1, sizeof *when);
    LY_CHECK_ERR_RET(!when, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
    yin_parse_attribute(ctx, attrs, YIN_ARG_CONDITION, &when->cond, Y_STR_ARG, YANG_WHEN);
    *when_p = when;
    struct yin_subelement subelems[3] = {
                                            {YANG_DESCRIPTION, &when->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &when->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    return yin_parse_content(ctx, subelems, 3, data, YANG_WHEN, NULL, &when->exts);
}

LY_ERR
yin_parse_yin_element_element(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                              uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[1] = {
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_YIN_ELEMENT));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_YINELEM_TRUE;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "yin-element");
        FREE_STRING(ctx->xml_ctx.ctx, temp_val);
        return LY_EVALID;
    }
    FREE_STRING(ctx->xml_ctx.ctx, temp_val);

    return yin_parse_content(ctx, subelems, 1, data, YANG_YIN_ELEMENT, NULL, exts);
}

LY_ERR
yin_parse_extension_instance(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, const char *ext_name,
                             int ext_name_len, LYEXT_SUBSTMT subelem, uint32_t subelem_index, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *out;
    const char *name, *prefix;
    size_t out_len, prefix_len, name_len;
    int dynamic;
    struct lysp_ext_instance *e;
    struct lysp_stmt *last_subelem = NULL, *new_subelem = NULL;
    struct yin_arg_record *iter;

    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *exts, e, LY_EMEM);

    e->yin = 0;
    /* store name and insubstmt info */
    e->name = lydict_insert(ctx->xml_ctx.ctx, ext_name, ext_name_len);
    e->insubstmt = subelem;
    e->insubstmt_index = subelem_index;
    e->yin |= LYS_YIN;

    /* store attributes as subelements */
    LY_ARRAY_FOR_ITER(attrs, struct yin_arg_record, iter) {
        if (!iter->prefix) {
            new_subelem = calloc(1, sizeof(*new_subelem));
            if (!e->child) {
                e->child = new_subelem;
            } else {
                last_subelem->next = new_subelem;
            }
            last_subelem = new_subelem;

            last_subelem->flags |= LYS_YIN_ATTR;
            last_subelem->stmt = lydict_insert(ctx->xml_ctx.ctx, iter->name, iter->name_len);
            LY_CHECK_ERR_RET(!last_subelem->stmt, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
            if (iter->dynamic_content) {
                last_subelem->arg = lydict_insert_zc(ctx->xml_ctx.ctx, iter->content);
                LY_CHECK_ERR_RET(!last_subelem->arg, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
            } else {
                last_subelem->arg = lydict_insert(ctx->xml_ctx.ctx, iter->content, iter->content_len);
                LY_CHECK_ERR_RET(!last_subelem->arg, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);
            }
        }
    }

    /* parse subelements */
    if (ctx->xml_ctx.status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(&ctx->xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
        if (ret == LY_EINVAL) {
            while (ctx->xml_ctx.status == LYXML_ELEMENT) {
                LY_CHECK_RET(lyxml_get_element(&ctx->xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
                if (!name) {
                    /* end of extension instance reached */
                    break;
                }
                LY_CHECK_RET(yin_parse_element_generic(ctx, name, name_len, data, &new_subelem));
                if (!e->child) {
                    e->child = new_subelem;
                } else {
                    last_subelem->next = new_subelem;
                }
                last_subelem = new_subelem;
            }
        } else {
            /* save text content */
            if (dynamic) {
                e->argument = lydict_insert_zc(ctx->xml_ctx.ctx, out);
                if (!e->argument) {
                    free(out);
                    return LY_EMEM;
                }
            } else {
                e->argument = lydict_insert(ctx->xml_ctx.ctx, out, out_len);
                LY_CHECK_RET(!e->argument, LY_EMEM);
            }
            LY_CHECK_RET(lyxml_get_element(&ctx->xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
            LY_CHECK_RET(name, LY_EINT);
        }
    }

    return LY_SUCCESS;
}

LY_ERR
yin_parse_element_generic(struct yin_parser_ctx *ctx, const char *name, size_t name_len, const char **data,
                          struct lysp_stmt **element)
{
    LY_ERR ret = LY_SUCCESS;
    const char *temp_prefix, *temp_name;
    char *out = NULL;
    size_t out_len, temp_name_len, temp_prefix_len, prefix_len;
    int dynamic;
    struct yin_arg_record *subelem_args = NULL;
    struct lysp_stmt *last = NULL, *new = NULL;

    /* allocate new structure for element */
    *element = calloc(1, sizeof(**element));
    (*element)->stmt = lydict_insert(ctx->xml_ctx.ctx, name, name_len);
    LY_CHECK_ERR_RET(!(*element)->stmt, LOGMEM(ctx->xml_ctx.ctx), LY_EMEM);

    last = (*element)->child;
    /* load attributes */
    while(ctx->xml_ctx.status == LYXML_ATTRIBUTE) {
        /* add new element to linked-list */
        new = calloc(1, sizeof(*last));
        LY_CHECK_ERR_GOTO(ret, LOGMEM(ctx->xml_ctx.ctx), err);
        if (!(*element)->child) {
            /* save first */
            (*element)->child = new;
        } else {
            last->next = new;
        }
        last = new;

        last->flags |= LYS_YIN_ATTR;
        ret = lyxml_get_attribute(&ctx->xml_ctx, data, &temp_prefix, &prefix_len, &temp_name, &temp_name_len);
        LY_CHECK_GOTO(ret, err);
        ret = lyxml_get_string(&ctx->xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
        LY_CHECK_GOTO(ret, err);
        last->stmt = lydict_insert(ctx->xml_ctx.ctx, temp_name, temp_name_len);
        LY_CHECK_ERR_GOTO(!last->stmt, LOGMEM(ctx->xml_ctx.ctx); ret = LY_EMEM, err);
        /* attributes with prefix are ignored */
        if (!temp_prefix) {
            if (dynamic) {
                last->arg = lydict_insert_zc(ctx->xml_ctx.ctx, out);
                if (!last->arg) {
                    free(out);
                    LOGMEM(ctx->xml_ctx.ctx);
                    ret = LY_EMEM;
                    goto err;
                }
            } else {
                last->arg = lydict_insert(ctx->xml_ctx.ctx, out, out_len);
                LY_CHECK_ERR_GOTO(!last->arg, LOGMEM(ctx->xml_ctx.ctx); ret = LY_EMEM, err);
            }
        }
    }

    /* parse content of element */
    ret = lyxml_get_string(&ctx->xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
    if (ret == LY_EINVAL) {
        while (ctx->xml_ctx.status == LYXML_ELEMENT) {
            /* parse subelements */
            ret = lyxml_get_element(&ctx->xml_ctx, data, &temp_prefix, &temp_prefix_len, &temp_name, &temp_name_len);
            LY_CHECK_GOTO(ret, err);
            if (!name) {
                /* end of element reached */
                break;
            }
            ret = yin_parse_element_generic(ctx, temp_name, temp_name_len, data, &last->next);
            LY_CHECK_GOTO(ret, err);
            last = last->next;
        }
    } else {
        /* save element content */
        if (out_len != 0) {
            if (dynamic) {
                (*element)->arg = lydict_insert_zc(ctx->xml_ctx.ctx, out);
                if (!(*element)->arg) {
                    free(out);
                    LOGMEM(ctx->xml_ctx.ctx);
                    ret = LY_EMEM;
                    goto err;
                }
            } else {
                (*element)->arg = lydict_insert(ctx->xml_ctx.ctx, out, out_len);
                LY_CHECK_ERR_GOTO(!(*element)->arg, LOGMEM(ctx->xml_ctx.ctx); ret = LY_EMEM, err);
            }
        }
        /* read closing tag */
        ret = lyxml_get_element(&ctx->xml_ctx, data, &temp_prefix, &prefix_len, &temp_name, &temp_name_len);
        LY_CHECK_GOTO(ret, err);
    }

    FREE_ARRAY(ctx, subelem_args, free_arg_rec);
    return LY_SUCCESS;

err:
    FREE_ARRAY(ctx, subelem_args, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_argument_element(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                           struct yin_argument_meta *arg_meta, struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[2] = {
                                            {YANG_YIN_ELEMENT, arg_meta->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, arg_meta->argument, Y_IDENTIF_ARG, YANG_ARGUMENT));

    return yin_parse_content(ctx, subelems, 2, data, YANG_ARGUMENT, NULL, exts);
}

LY_ERR
yin_parse_extension(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data, struct lysp_ext **extensions)
{
    struct lysp_ext *ex;
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *extensions, ex, LY_EMEM);
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &ex->name, Y_IDENTIF_ARG, YANG_EXTENSION));

    struct yin_argument_meta arg_info = {&ex->flags, &ex->argument};
    struct yin_subelement subelems[5] = {
                                            {YANG_ARGUMENT, &arg_info, YIN_SUBELEM_UNIQUE},
                                            {YANG_DESCRIPTION, &ex->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &ex->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_STATUS, &ex->flags, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    return yin_parse_content(ctx, subelems, 5, data, YANG_EXTENSION, NULL, &ex->exts);
}

/**
 * @brief Parse module substatements.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] mod_attrs Attributes of module element.
 * @param[in,out] data Data to read from.
 * @param[out] mod Parsed module structure.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_mod(struct yin_parser_ctx *ctx, struct yin_arg_record *mod_attrs, const char **data, struct lysp_module **mod)
{
    struct yin_subelement subelems[9] = {
                                            {YANG_CONTACT, &(*mod)->mod->contact, YIN_SUBELEM_UNIQUE},
                                            {YANG_DESCRIPTION, &(*mod)->mod->dsc, YIN_SUBELEM_UNIQUE},
                                            {YANG_EXTENSION, &(*mod)->exts, 0},
                                            {YANG_IMPORT, *mod, 0},
                                            {YANG_NAMESPACE, &(*mod)->mod->ns, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                            {YANG_ORGANIZATION, &(*mod)->mod->org, YIN_SUBELEM_UNIQUE},
                                            {YANG_PREFIX, &(*mod)->mod->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                            {YANG_REFERENCE, &(*mod)->mod->ref, YIN_SUBELEM_UNIQUE},
                                            {YANG_CUSTOM, NULL, 0}
                                        };

    LY_CHECK_RET(yin_parse_attribute(ctx, mod_attrs, YIN_ARG_NAME, &(*mod)->mod->name, Y_IDENTIF_ARG, YANG_MODULE));

    return yin_parse_content(ctx, subelems, 9, data, YANG_MODULE, NULL, &(*mod)->exts);
}

LY_ERR
yin_parse_module(struct ly_ctx *ctx, const char *data, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    struct lysp_module *mod_p = NULL;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    struct yin_arg_record *attrs = NULL;

    struct yin_parser_ctx yin_ctx;

    /* initialize context */
    memset(&yin_ctx, 0, sizeof yin_ctx);
    yin_ctx.xml_ctx.ctx = ctx;
    yin_ctx.xml_ctx.line = 1;


    /* check submodule */
    ret = lyxml_get_element(&yin_ctx.xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_GOTO(ret, cleanup);
    ret = yin_load_attributes(&yin_ctx, &data, &attrs);
    LY_CHECK_GOTO(ret, cleanup);
    kw = yin_match_keyword(&yin_ctx, name, name_len, prefix, prefix_len, YANG_NONE);
    if (kw == YANG_SUBMODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_MODULE) {
        LOGVAL_PARSER((struct lys_parser_ctx *)&yin_ctx, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".",
                    ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* allocate module */
    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(ctx), cleanup);
    mod_p->mod = mod;
    mod_p->parsing = 1;

    /* parse module substatements */
    ret = yin_parse_mod(&yin_ctx, attrs, &data, &mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret != LY_SUCCESS) {
        lysp_module_free(mod_p);
    }
    FREE_ARRAY(&yin_ctx, attrs, free_arg_rec);
    lyxml_context_clear(&yin_ctx.xml_ctx);
    return ret;
}
