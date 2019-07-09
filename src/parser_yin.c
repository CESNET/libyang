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
yin_match_keyword(struct yin_parser_ctx *ctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len)
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
    if (record->dynamic_content) {
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

/**
 * @brief Parse yin argument.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Array of attributes.
 * @param[in,out] data Data to read from.
 * @param[in] arg_type Type of argument that is expected in parsed element (use YIN_ARG_NONE for elements without special argument).
 * @param[out] arg_val Where value of argument should be stored. Can be NULL if arg_type is specified as YIN_ARG_NONE.
 * @param[in] val_type Type of expected value of attribute.
 * @param[in] current_element Identification of current element, used for logging.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_attribute(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, enum YIN_ARGUMENT arg_type,
                    const char **arg_val, enum yang_arg val_type, enum yang_keyword current_element)
{
    enum YIN_ARGUMENT arg = YIN_ARG_UNKNOWN;
    struct yin_arg_record *iter = NULL;
    bool found = false;

    /* validation of attributes */
    LY_ARRAY_FOR(*attrs, struct yin_arg_record, iter) {
        /* yin arguments represented as attributes have no namespace, which in this case means no prefix */
        if (!iter->prefix) {
            arg = yin_match_argument_name(iter->name, iter->name_len);
            if (arg == YIN_ARG_NONE) {
                continue;
            } else if (arg == arg_type) {
                LY_CHECK_ERR_RET(found, LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Duplicit definition of %s attribute in %s element",
                                 yin_attr2str(arg), ly_stmt2str(current_element)), LY_EVALID);
                found = true;
                if (iter->dynamic_content) {
                    *arg_val = lydict_insert_zc(ctx->xml_ctx.ctx, iter->content);
                    LY_CHECK_RET(!(*arg_val), LY_EMEM);
                    /* string is no longer supposed to be freed when the sized array is freed */
                    iter->dynamic_content = 0;
                } else {
                    *arg_val = lydict_insert(ctx->xml_ctx.ctx, iter->content, iter->content_len);
                    LY_CHECK_RET(!(*arg_val), LY_EMEM);
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

    /* TODO check validity according to val_type */

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

LY_ERR
yin_check_subelem_mandatory_constraint(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info,
                                       signed char subelem_info_size, enum yang_keyword current_element)
{
    for (signed char i = 0; i < subelem_info_size; ++i) {
        /* if there is element that is mandatory and isn't parsed log error and rturn LY_EVALID */
        if (subelem_info[i].flags & YIN_SUBELEM_MANDATORY && !(subelem_info[i].flags & YIN_SUBELEM_PARSED)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Missing mandatory subelement %s of %s element.",
                          ly_stmt2str(subelem_info[i].type), ly_stmt2str(current_element));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
yin_check_subelem_first_constraint(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info,
                                   signed char subelem_info_size, enum yang_keyword current_element, struct yin_subelement *exp_first)
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

/* TODO add something like ifdef NDEBUG around this function, this is supposed to be checked only in debug mode */
bool
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

/**
 * @brief Parse simple element without any special constraints and argument mapped to yin attribute,
 * for example prefix or namespace element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Attributes of current element.
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
    LY_CHECK_RET(yin_parse_attribute(ctx, &attrs, arg_type, value, arg_val_type, kw));
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    return yin_parse_content(ctx, subelems, 1, data, kw, NULL, exts);
}

/**
 * @brief Parse simple element without any special constraints and argument mapped to yin attribute, that can have
 * more instances, such as base or if-feature.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] attrs Attributes of current element.
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
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, &index, 0}};
    LY_CHECK_RET(yin_parse_attribute(ctx, &attrs, arg_type, value, arg_val_type, kw));

    return yin_parse_content(ctx, subelems, 1, data, kw, NULL, exts);
}

/**
 * @brief function to parse meta tags (description, contact, ...) eg. elements with
 * text element as child
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] args Sized array of arguments of current element.
 * @param[in,out] data Data to read from.
 * @param[out] value Where the content of meta element should be stored.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_meta_element(struct yin_parser_ctx *ctx, const char **data, enum yang_keyword elem_type,
                       const char **value, struct lysp_ext_instance **exts)
{
    assert(elem_type == YANG_ORGANIZATION || elem_type == YANG_CONTACT || elem_type == YANG_DESCRIPTION || elem_type == YANG_REFERENCE);

    struct yin_subelement subelems[2] = {{YANG_CUSTOM, NULL, 0},
                                         {YIN_TEXT, value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE | YIN_SUBELEM_FIRST}};

    return yin_parse_content(ctx, subelems, 2, data, elem_type, NULL, exts);
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


/**
 * @brief Parse belongs-to element.
 *
 * @param[in] ctx Yin parser context for logging and to store current state.
 * @param[in] attrs Array of attributes of belongs-to element.
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
    struct yin_subelement subelems[2] = {{YANG_PREFIX, &submod->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};
    LY_CHECK_RET(yin_parse_attribute(ctx, &attrs, YIN_ARG_MODULE, &submod->belongsto, Y_IDENTIF_ARG, YANG_BELONGS_TO));

    return yin_parse_content(ctx, subelems, 2, data, YANG_BELONGS_TO, NULL, exts);
}

LY_ERR
yin_parse_content(struct yin_parser_ctx *ctx, struct yin_subelement *subelem_info, signed char subelem_info_size,
                  const char **data, enum yang_keyword current_element, const char **text_content, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    struct sized_string prefix, name;
    char *out;
    size_t out_len;
    int dynamic;
    struct yin_arg_record *subelem_attrs = NULL;
    enum yang_keyword kw = YANG_NONE;
    struct yin_subelement *subelem_info_rec = NULL;
    uint32_t index = 0;
    struct lysp_type *type;
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
                ret = yin_load_attributes(ctx, data, &subelem_attrs);
                LY_CHECK_GOTO(ret, cleanup);
                kw = yin_match_keyword(ctx, name.value, name.len, prefix.value, prefix.len);

                /* check if this element can be child of current element */
                subelem_info_rec = get_record(kw, subelem_info_size, subelem_info);
                if (!subelem_info_rec) {
                    LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_UNEXP_SUBELEM, name.len, name.value, ly_stmt2str(current_element));
                    ret = LY_EVALID;
                    goto cleanup;
                }

                /* TODO check relative order */

                /* if element is unique and already defined log error */
                if ((subelem_info_rec->flags & YIN_SUBELEM_UNIQUE) && (subelem_info_rec->flags & YIN_SUBELEM_PARSED)) {
                    LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Redefinition of %s element in %s element.", ly_stmt2str(kw), ly_stmt2str(current_element));
                    return LY_EVALID;
                }
                if (subelem_info_rec->flags & YIN_SUBELEM_FIRST) {
                    LY_CHECK_RET(yin_check_subelem_first_constraint(ctx, subelem_info, subelem_info_size, current_element, subelem_info_rec));
                }
                subelem_info_rec->flags |= YIN_SUBELEM_PARSED;

                switch (kw) {
                case YANG_CUSTOM:
                    index = (subelem_info_rec->dest) ? *((uint32_t*)subelem_info_rec->dest) : 0;
                    ret = yin_parse_extension_instance(ctx, &subelem_attrs, data, name2fullname(name.value, prefix.len),
                                                      namelen2fulllen(name.len, prefix.len),
                                                      kw2lyext_substmt(current_element), index, exts);
                    break;
                case YANG_ACTION:
                    break;
                case YANG_ANYDATA:
                    break;
                case YANG_ANYXML:
                    break;
                case YANG_ARGUMENT:
                    ret = yin_parse_argument_element(ctx, &subelem_attrs, data, (struct yin_argument_meta *)subelem_info_rec->dest, exts);
                    break;
                case YANG_AUGMENT:
                    break;
                case YANG_BASE:
                    if (current_element == YANG_IDENTITY) {
                        type = (struct lysp_type *)subelem_info_rec->dest;
                        ret = yin_parse_simple_elements(ctx, subelem_attrs, data, kw, &type->bases, YIN_ARG_NAME,
                                                        Y_PREF_IDENTIF_ARG, exts);
                        type->flags |= LYS_SET_BASE;
                    } else if (current_element == YANG_TYPE) {
                        ret = yin_parse_simple_elements(ctx, subelem_attrs, data, kw, (const char ***)subelem_info_rec->dest,
                                                        YIN_ARG_NAME, Y_PREF_IDENTIF_ARG, exts);
                    } else {
                        LOGINT(ctx->xml_ctx.ctx);
                        ret = LY_EINT;
                    }
                    break;
                case YANG_BELONGS_TO:
                    ret = yin_parse_belongs_to(ctx, subelem_attrs, data, (struct lysp_submodule *)subelem_info_rec->dest, exts);
                    break;
                case YANG_BIT:
                    break;
                case YANG_CASE:
                    break;
                case YANG_CHOICE:
                    break;
                case YANG_CONFIG:
                    break;
                case YANG_CONTACT:
                case YANG_DESCRIPTION:
                case YANG_ORGANIZATION:
                case YANG_REFERENCE:
                    ret = yin_parse_meta_element(ctx, data, kw, (const char **)subelem_info_rec->dest, exts);
                    break;
                case YANG_CONTAINER:
                    break;
                case YANG_DEFAULT:
                    break;
                case YANG_DEVIATE:
                    break;
                case YANG_DEVIATION:
                    break;
                case YANG_ENUM:
                    break;
                case YANG_ERROR_APP_TAG:
                    break;
                case YANG_ERROR_MESSAGE:
                    break;
                case YANG_EXTENSION:
                    ret = yin_parse_extension(ctx, &subelem_attrs, data, (struct lysp_ext **)subelem_info_rec->dest);
                    break;
                case YANG_FEATURE:
                    break;
                case YANG_FRACTION_DIGITS:
                    break;
                case YANG_GROUPING:
                    break;
                case YANG_IDENTITY:
                    break;
                case YANG_IF_FEATURE:
                    ret = yin_parse_simple_elements(ctx, subelem_attrs, data, kw,
                                                    (const char ***)subelem_info_rec->dest, YIN_ARG_VALUE, Y_STR_ARG, exts);
                    break;
                case YANG_IMPORT:
                    ret = yin_parse_import(ctx, &subelem_attrs, data, (struct lysp_module *)subelem_info_rec->dest);
                    break;
                case YANG_INCLUDE:
                    break;
                case YANG_INPUT:
                    break;
                case YANG_KEY:
                    break;
                case YANG_LEAF:
                    break;
                case YANG_LEAF_LIST:
                    break;
                case YANG_LENGTH:
                    break;
                case YANG_LIST:
                    break;
                case YANG_MANDATORY:
                    break;
                case YANG_MAX_ELEMENTS:
                    break;
                case YANG_MIN_ELEMENTS:
                    break;
                case YANG_MODIFIER:
                    break;
                case YANG_MODULE:
                    break;
                case YANG_MUST:
                    break;
                case YANG_NAMESPACE:
                    ret = yin_parse_simple_element(ctx, subelem_attrs, data, kw,
                                                   (const char **)subelem_info_rec->dest, YIN_ARG_URI, Y_STR_ARG, exts);
                    break;
                case YANG_NOTIFICATION:
                    break;
                case YANG_ORDERED_BY:
                    break;
                case YANG_OUTPUT:
                    break;
                case YANG_PATH:
                    break;
                case YANG_PATTERN:
                    break;
                case YANG_POSITION:
                    break;
                case YANG_PREFIX:
                    ret = yin_parse_simple_element(ctx, subelem_attrs, data, kw,
                                                   (const char **)subelem_info_rec->dest, YIN_ARG_VALUE, Y_IDENTIF_ARG, exts);
                    break;
                case YANG_PRESENCE:
                    break;
                case YANG_RANGE:
                    break;
                case YANG_REFINE:
                    break;
                case YANG_REQUIRE_INSTANCE:
                    break;
                case YANG_REVISION:
                    break;
                case YANG_REVISION_DATE:
                    ret = yin_parse_revision_date(ctx, &subelem_attrs, data, (char *)subelem_info_rec->dest, exts);
                    break;
                case YANG_RPC:
                    break;
                case YANG_STATUS:
                    ret = yin_parse_status(ctx, &subelem_attrs, data, (uint16_t *)subelem_info_rec->dest, exts);
                    break;
                case YANG_SUBMODULE:
                    break;
                case YANG_TYPE:
                    break;
                case YANG_TYPEDEF:
                    break;
                case YANG_UNIQUE:
                    break;
                case YANG_UNITS:
                    break;
                case YANG_USES:
                    break;
                case YANG_VALUE:
                    break;
                case YANG_WHEN:
                    break;
                case YANG_YANG_VERSION:
                    break;
                case YANG_YIN_ELEMENT:
                    ret = yin_parse_yin_element_element(ctx, subelem_attrs, data, (uint16_t *)subelem_info_rec->dest, exts);
                    break;
                case YIN_TEXT:
                    ret = yin_parse_content(ctx, NULL, 0, data, YIN_TEXT, (const char **)subelem_info_rec->dest, NULL);
                    break;
                case YIN_VALUE:
                    break;
                default:
                    LOGINT(ctx->xml_ctx.ctx);
                    return LY_EINT;
                }
                LY_CHECK_GOTO(ret, cleanup);
                FREE_ARRAY(ctx, subelem_attrs, free_arg_rec);
                subelem_attrs = NULL;
                subelem_info_rec = NULL;
            }
        } else {
            /* elements with text or none content */
            /* save text content, if text_content isn't set, it's just ignored */
            if (text_content) {
                if (dynamic) {
                    *text_content = lydict_insert_zc(ctx->xml_ctx.ctx, out);
                    if (!*text_content) {
                        free(out);
                        return LY_EMEM;
                    }
                } else {
                    if (out_len == 0) {
                        *text_content = NULL;
                    } else {
                        *text_content = lydict_insert(ctx->xml_ctx.ctx, out, out_len);
                    }
                }
            }
            /* load closing element */
            LY_CHECK_RET(lyxml_get_element(&ctx->xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len));
        }

        LY_CHECK_RET(yin_check_subelem_mandatory_constraint(ctx, subelem_info, subelem_info_size, current_element));
    }

cleanup:
    FREE_ARRAY(ctx, subelem_attrs, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_revision_date(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data, char *rev,
                        struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    const char *temp_rev;
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_DATE, &temp_rev, Y_STR_ARG, YANG_REVISION_DATE));
    LY_CHECK_RET(ret != LY_SUCCESS, ret);
    LY_CHECK_RET(lysp_check_date((struct lys_parser_ctx *)ctx, temp_rev, strlen(temp_rev), "revision-date") != LY_SUCCESS, LY_EVALID);

    strcpy(rev, temp_rev);
    FREE_STRING(ctx->xml_ctx.ctx, temp_rev);

    return yin_parse_content(ctx, subelems, 1, data, YANG_REVISION_DATE, NULL, exts);
}

LY_ERR
yin_parse_import(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data, struct lysp_module *mod)
{
    struct lysp_import *imp;
    /* allocate new element in sized array for import */
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, mod->imports, imp, LY_EMEM);

    struct yin_subelement subelems[5] = {{YANG_DESCRIPTION, &imp->dsc, YIN_SUBELEM_UNIQUE},
                                         {YANG_PREFIX, &imp->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                         {YANG_REFERENCE, &imp->ref, YIN_SUBELEM_UNIQUE},
                                         {YANG_REVISION_DATE, imp->rev, YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};

    /* parse import attributes  */
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_MODULE, &imp->name, Y_IDENTIF_ARG, YANG_IMPORT));
    LY_CHECK_RET(yin_parse_content(ctx, subelems, 5, data, YANG_IMPORT, NULL, &imp->exts));
    /* check prefix validity */
    LY_CHECK_RET(lysp_check_prefix((struct lys_parser_ctx *)ctx, mod->imports, mod->mod->prefix, &imp->prefix), LY_EVALID);

    return yin_parse_content(ctx, subelems, 5, data, YANG_IMPORT, NULL, &imp->exts);
}

LY_ERR
yin_parse_status(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *value = NULL;
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_DUPELEM, "status");
        return LY_EVALID;
    }

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
yin_parse_yin_element_element(struct yin_parser_ctx *ctx, struct yin_arg_record *attrs, const char **data,
                              uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    LY_CHECK_RET(yin_parse_attribute(ctx, &attrs, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, YANG_YIN_ELEMENT));
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
yin_parse_extension_instance(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data, const char *ext_name,
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
    LY_ARRAY_FOR_ITER(*attrs, struct yin_arg_record, iter) {
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
                LY_CHECK_RET(yin_parse_element_generic(ctx, name, name_len, prefix, prefix_len, data, &new_subelem));
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
yin_parse_element_generic(struct yin_parser_ctx *ctx, const char *name, size_t name_len, const char *prefix,
                          size_t prefix_len, const char **data, struct lysp_stmt **element)
{
    LY_ERR ret = LY_SUCCESS;
    const char *temp_prefix, *temp_name;
    char *out = NULL;
    size_t out_len, temp_name_len, temp_prefix_len;
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
            ret = yin_parse_element_generic(ctx, temp_name, temp_name_len, temp_prefix, temp_prefix_len, data, &last->next);
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
yin_parse_argument_element(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data,
                           struct yin_argument_meta *arg_meta, struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[2] = {{YANG_YIN_ELEMENT, arg_meta->flags, YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};

    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, arg_meta->argument, Y_IDENTIF_ARG, YANG_ARGUMENT));

    return yin_parse_content(ctx, subelems, 2, data, YANG_ARGUMENT, NULL, exts);
}

LY_ERR
yin_parse_extension(struct yin_parser_ctx *ctx, struct yin_arg_record **attrs, const char **data, struct lysp_ext **extensions)
{
    struct lysp_ext *ex;
    LY_ARRAY_NEW_RET(ctx->xml_ctx.ctx, *extensions, ex, LY_EMEM);
    LY_CHECK_RET(yin_parse_attribute(ctx, attrs, YIN_ARG_NAME, &ex->name, Y_IDENTIF_ARG, YANG_EXTENSION));

    struct yin_argument_meta arg_info = {&ex->flags, &ex->argument};
    struct yin_subelement subelems[5] = {{YANG_ARGUMENT, &arg_info, YIN_SUBELEM_UNIQUE},
                                         {YANG_DESCRIPTION, &ex->dsc, YIN_SUBELEM_UNIQUE},
                                         {YANG_REFERENCE, &ex->ref, YIN_SUBELEM_UNIQUE},
                                         {YANG_STATUS, &ex->flags, YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};

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
yin_parse_mod(struct yin_parser_ctx *ctx, struct yin_arg_record **mod_attrs, const char **data, struct lysp_module **mod)
{
    struct yin_subelement subelems[9] = {{YANG_CONTACT, &(*mod)->mod->contact, YIN_SUBELEM_UNIQUE},
                                         {YANG_DESCRIPTION, &(*mod)->mod->dsc, YIN_SUBELEM_UNIQUE},
                                         {YANG_EXTENSION, &(*mod)->exts, 0},
                                         {YANG_IMPORT, *mod, 0},
                                         {YANG_NAMESPACE, &(*mod)->mod->ns, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                         {YANG_ORGANIZATION, &(*mod)->mod->org, YIN_SUBELEM_UNIQUE},
                                         {YANG_PREFIX, &(*mod)->mod->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                         {YANG_REFERENCE, &(*mod)->mod->ref, YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};

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

    struct yin_arg_record *args = NULL;

    struct yin_parser_ctx yin_ctx;

    /* initialize context */
    memset(&yin_ctx, 0, sizeof yin_ctx);
    yin_ctx.xml_ctx.ctx = ctx;
    yin_ctx.xml_ctx.line = 1;


    /* check submodule */
    ret = lyxml_get_element(&yin_ctx.xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_GOTO(ret, cleanup);
    ret = yin_load_attributes(&yin_ctx, &data, &args);
    LY_CHECK_GOTO(ret, cleanup);
    kw = yin_match_keyword(&yin_ctx, name, name_len, prefix, prefix_len);
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
    ret = yin_parse_mod(&yin_ctx, &args, &data, &mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret != LY_SUCCESS) {
        lysp_module_free(mod_p);
    }
    FREE_ARRAY(&yin_ctx, args, free_arg_rec);
    lyxml_context_clear(&yin_ctx.xml_ctx);
    return ret;
}
