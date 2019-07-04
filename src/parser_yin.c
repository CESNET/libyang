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
    [YIN_ARG_XMLNS] = "xmlns",
};

enum yang_keyword
yin_match_keyword(struct lyxml_context *xml_ctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len)
{
    const char *start = NULL;
    enum yang_keyword kw = YANG_NONE;
    const struct lyxml_ns *ns = NULL;

    if (!name || name_len == 0) {
        return YANG_NONE;
    }

    ns = lyxml_ns_get(xml_ctx, prefix, prefix_len);
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
    case 'x':
        already_read += 1;
        IF_ARG("mlns", 4, YIN_ARG_XMLNS);
        break;
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

static void free_arg_rec(struct lyxml_context *xml_ctx, struct yin_arg_record *record) {
    (void)xml_ctx; /* unused */
    if (record->dynamic_content) {
        free(record->content);
    }
}

LY_ERR
yin_load_attributes(struct lyxml_context *xml_ctx, const char **data, struct yin_arg_record **args)
{
    LY_ERR ret = LY_SUCCESS;
    struct yin_arg_record *argument_record = NULL;

    /* load all attributes first */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        LY_ARRAY_NEW_GOTO(xml_ctx->ctx, *args, argument_record, ret, cleanup);
        ret = lyxml_get_attribute(xml_ctx, data, &argument_record->prefix, &argument_record->prefix_len,
                                  &argument_record->name, &argument_record->name_len);
        LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);

        if (xml_ctx->status == LYXML_ATTR_CONTENT) {
            argument_record->content = NULL;
            argument_record->content_len = 0;
            argument_record->dynamic_content = 0;
            ret = lyxml_get_string(xml_ctx, data, &argument_record->content, &argument_record->content_len,
                                   &argument_record->content, &argument_record->content_len, &argument_record->dynamic_content);
            LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
        }
    }

cleanup:
    if (ret != LY_SUCCESS) {
        FREE_ARRAY(xml_ctx, *args, free_arg_rec);
        *args = NULL;
    }
    return ret;
}

LY_ERR
yin_parse_attribute(struct lyxml_context *xml_ctx, struct yin_arg_record **args, enum YIN_ARGUMENT arg_type,
                    const char **arg_val, uint8_t flags, enum yang_keyword current_element)
{
    enum YIN_ARGUMENT arg = YIN_ARG_UNKNOWN;
    struct yin_arg_record *iter = NULL;
    bool found = false;

    /* validation of attributes */
    LY_ARRAY_FOR(*args, struct yin_arg_record, iter) {
        /* yin arguments represented as attributes have no namespace, which in this case means no prefix */
        if (!iter->prefix) {
            arg = yin_match_argument_name(iter->name, iter->name_len);
            if (arg == YIN_ARG_NONE) {
                continue;
            } else if (arg == arg_type) {
                found = true;
                if (iter->dynamic_content) {
                    *arg_val = lydict_insert_zc(xml_ctx->ctx, iter->content);
                    LY_CHECK_RET(!(*arg_val), LY_EMEM);
                    /* string is no longer supposed to be freed when the sized array is freed */
                    iter->dynamic_content = 0;
                } else {
                    *arg_val = lydict_insert(xml_ctx->ctx, iter->content, iter->content_len);
                    LY_CHECK_RET(!(*arg_val), LY_EMEM);
                }
            } else {
                LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Unexpected attribute \"%.*s\" of %s element.", iter->name_len, iter->name, ly_stmt2str(current_element));
                return LY_EVALID;
            }
        }
    }

    if (flags & YIN_ARG_MANDATORY && !found) {
        LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Missing mandatory attribute \"%s\" of %s element.", yin_attr2str(arg_type), ly_stmt2str(current_element));
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief get record with given type.
 *
 * @param[in] type Type of wanted record.
 * @param[in] array_size Size of array.
 * @param[in] array Searched array.
 *
 * @return Pointer to desired record on success, NULL if element is not in the array.
 */
struct yin_subelement *
get_record(enum yang_keyword type, size_t array_size, struct yin_subelement *array)
{
    for (size_t i = 0; i < array_size; ++i) {
        if (array[i].type == type) {
            return &array[i];
        }
    }

    return NULL;
}

LY_ERR
yin_check_subelem_mandatory_constraint(struct lyxml_context *xml_ctx, struct yin_subelement *subelem_info,
                                       size_t subelem_info_size, enum yang_keyword current_element)
{
    for (size_t i = 0; i < subelem_info_size; ++i) {
        /* if there is element that is mandatory and isn't parsed log error and rturn LY_EVALID */
        if (subelem_info[i].flags & YIN_SUBELEM_MANDATORY && !(subelem_info[i].flags & YIN_SUBELEM_PARSED)) {
            LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Missing mandatory subelement %s of %s element", ly_stmt2str(subelem_info[i].type), ly_stmt2str(current_element));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
yin_check_subelem_first_constraint(struct lyxml_context *xml_ctx, struct yin_subelement *subelem_info,
                                   size_t subelem_info_size, enum yang_keyword current_element, struct yin_subelement *exp_first)
{
    for (size_t i = 0; i < subelem_info_size; ++i) {
        if (subelem_info[i].flags & YIN_SUBELEM_PARSED) {
            LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Subelement %s of %s element must be defined as first subelement.", exp_first->type, current_element);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Generic function for content parsing
 *
 * @param[in,out] xml_ctx Xml context.
 * @param[in] subelem_info array of valid subelement types and meta information,
 *            array must be ordered by subelem_info->type in ascending order.
 * @param[in] subelem_info_size Size of subelem_info array.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] current_element Type of current element.
 * @param[out] Where the text content of element should be stored. Text content is ignored if set to NULL.
 * @param[in] exts Extension instance to add to. Can be null if element cannot have extension as subelement.
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_content(struct lyxml_context *xml_ctx, struct yin_subelement *subelem_info, size_t subelem_info_size,
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

    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
        /* current element has subelements as content */
        if (ret == LY_EINVAL) {
            while (xml_ctx->status == LYXML_ELEMENT) {
                ret = lyxml_get_element(xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len);
                LY_CHECK_GOTO(ret, cleanup);
                if (!name.value) {
                    /* end of current element reached */
                    break;
                }
                ret = yin_load_attributes(xml_ctx, data, &subelem_attrs);
                LY_CHECK_GOTO(ret, cleanup);
                kw = yin_match_keyword(xml_ctx, name.value, name.len, prefix.value, prefix.len);

                /* check if this element can be child of current element */
                subelem_info_rec = get_record(kw, subelem_info_size, subelem_info);
                if (!subelem_info_rec) {
                    LOGVAL_PARSER(xml_ctx, LY_VCODE_UNEXP_SUBELEM, name.len, name.value, ly_stmt2str(current_element));
                    ret = LY_EVALID;
                    goto cleanup;
                }

                /* TODO macro to check order */
                /* if element is unique and already defined log error */
                if (subelem_info_rec->flags & YIN_SUBELEM_UNIQUE & YIN_SUBELEM_PARSED) {
                    LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX_YIN, "Redefinition of %s element in %s element.", kw, current_element);
                    return LY_EVALID;
                }
                if (subelem_info_rec->flags & YIN_SUBELEM_FIRST) {
                    yin_check_subelem_first_constraint(xml_ctx, subelem_info, subelem_info_size, current_element, subelem_info_rec);
                }
                subelem_info_rec->flags |= YIN_SUBELEM_PARSED;

                switch (kw) {
                case YANG_CUSTOM:
                    /* TODO write function to calculate index instead of hardcoded 0 */
                    ret = yin_parse_extension_instance(xml_ctx, &subelem_attrs, data, name2fullname(name.value, prefix.len),
                                                      namelen2fulllen(name.len, prefix.len),
                                                      kw2lyext_substmt(current_element), 0, exts);
                    LY_CHECK_GOTO(ret, cleanup);
                    break;
                case YANG_ACTION:
                    break;
                case YANG_ANYDATA:
                    break;
                case YANG_ANYXML:
                    break;
                case YANG_ARGUMENT:
                    ret = yin_parse_argument_element(xml_ctx, &subelem_attrs, data, (struct yin_argument_meta *)subelem_info_rec->dest, exts);
                    break;
                case YANG_AUGMENT:
                    break;
                case YANG_BASE:
                    break;
                case YANG_BELONGS_TO:
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
                    ret = yin_parse_meta_element(xml_ctx, data, kw, (const char **)subelem_info_rec->dest, exts);
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
                    break;
                case YANG_IMPORT:
                    ret = yin_parse_import(xml_ctx, &subelem_attrs, data, (struct lysp_module *)subelem_info_rec->dest);
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
                    ret = yin_parse_simple_element(xml_ctx, subelem_attrs, data, kw, (const char **)subelem_info_rec->dest,
                                                   YIN_ARG_URI, YIN_ARG_MANDATORY, exts);
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
                    ret = yin_parse_prefix(xml_ctx, subelem_attrs, (const char **)subelem_info_rec->dest, data, exts);
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
                    ret = yin_parse_revision_date(xml_ctx, &subelem_attrs, data, (char *)subelem_info_rec->dest, exts);
                    break;
                case YANG_RPC:
                    break;
                case YANG_STATUS:
                    ret = yin_parse_status(xml_ctx, &subelem_attrs, data, (uint16_t *)subelem_info_rec->dest, exts);
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
                    ret = yin_parse_yin_element_element(xml_ctx, subelem_attrs, data, (uint16_t *)subelem_info_rec->dest, exts);
                    break;
                case YIN_TEXT:
                    ret = yin_parse_content(xml_ctx, NULL, 0, data, YIN_TEXT, (const char **)subelem_info_rec->dest, NULL);
                    break;
                case YIN_VALUE:
                    break;
                default:
                    LOGINT(xml_ctx->ctx);
                    return LY_EINT;
                }
                LY_CHECK_GOTO(ret, cleanup);
                FREE_ARRAY(xml_ctx, subelem_attrs, free_arg_rec);
                subelem_attrs = NULL;
                subelem_info_rec = NULL;
            }
            LY_CHECK_RET(yin_check_subelem_mandatory_constraint(xml_ctx, subelem_info, subelem_info_size, current_element));

        } else {
            /* elements with text or none content */
            /* save text content, if text_content isn't set, it's just ignored */
            if (text_content) {
                if (dynamic) {
                    *text_content = lydict_insert_zc(xml_ctx->ctx, out);
                    if (!*text_content) {
                        free(out);
                        return LY_EMEM;
                    }
                } else {
                    if (out_len == 0) {
                        *text_content = NULL;
                    } else {
                        *text_content = lydict_insert(xml_ctx->ctx, out, out_len);
                    }
                }
            }
            /* load closing element */
            LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix.value, &prefix.len, &name.value, &name.len));
        }
    }

cleanup:
    FREE_ARRAY(xml_ctx, subelem_attrs, free_arg_rec);
    return ret;
}

LY_ERR
yin_parse_simple_element(struct lyxml_context *xml_ctx, struct yin_arg_record *attrs, const char **data, enum yang_keyword kw,
                         const char **value, enum YIN_ARGUMENT arg_type, uint8_t argument_flags, struct lysp_ext_instance **exts)
{
    LY_CHECK_RET(yin_parse_attribute(xml_ctx, &attrs, arg_type, value, argument_flags, kw));
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    return yin_parse_content(xml_ctx, subelems, 1, data, kw, NULL, exts);
}

LYEXT_SUBSTMT
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
 * @brief function to parse meta tags eg. elements with text element as child
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] args Sized array of arguments of current element.
 * @param[in,out] data Data to read from.
 * @param[out] value Where the content of meta tag should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_meta_element(struct lyxml_context *xml_ctx, const char **data, enum yang_keyword elem_type,
                       const char **value, struct lysp_ext_instance **exts)
{
    assert(elem_type == YANG_ORGANIZATION || elem_type == YANG_CONTACT || elem_type == YANG_DESCRIPTION || elem_type == YANG_REFERENCE);

    struct yin_subelement subelems[2] = {{YIN_TEXT, value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE | YIN_SUBELEM_FIRST},
                                         {YANG_CUSTOM, NULL, 0}};

    return yin_parse_content(xml_ctx, subelems, 2, data, elem_type, NULL, exts);
}

LY_ERR
yin_parse_revision_date(struct lyxml_context *xml_ctx, struct yin_arg_record **args, const char **data, char *rev,
                        struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    const char *temp_rev;
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, args, YIN_ARG_DATE, &temp_rev, YIN_ARG_MANDATORY, YANG_REVISION_DATE));
    LY_CHECK_RET(ret != LY_SUCCESS, ret);
    LY_CHECK_RET(lysp_check_date((struct lys_parser_ctx *)xml_ctx, temp_rev, strlen(temp_rev), "revision-date") != LY_SUCCESS, LY_EVALID);

    strcpy(rev, temp_rev);
    lydict_remove(xml_ctx->ctx, temp_rev);

    return yin_parse_content(xml_ctx, subelems, 1, data, YANG_REVISION_DATE, NULL, exts);
}

LY_ERR
yin_parse_prefix(struct lyxml_context *xml_ctx, struct yin_arg_record *attrs, const char **prefix, const char **data,
                 struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};
    LY_CHECK_RET(yin_parse_attribute(xml_ctx, &attrs, YIN_ARG_VALUE, prefix, YIN_ARG_MANDATORY, YANG_PREFIX));

    return yin_parse_content(xml_ctx, subelems, 1, data, YANG_REVISION_DATE, NULL, exts);
}

LY_ERR
yin_parse_import(struct lyxml_context *xml_ctx, struct yin_arg_record **import_args, const char **data, struct lysp_module *mod)
{
    struct lysp_import *imp;
    /* allocate new element in sized array for import */
    LY_ARRAY_NEW_RET(xml_ctx->ctx, mod->imports, imp, LY_EMEM);

    struct yin_subelement subelems[5] = {{YANG_PREFIX, &imp->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                         {YANG_DESCRIPTION, &imp->dsc, YIN_SUBELEM_UNIQUE},
                                         {YANG_REFERENCE, &imp->ref, YIN_SUBELEM_UNIQUE},
                                         {YANG_REVISION_DATE, imp->rev, YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};

    /* parse import attributes  */
    LY_CHECK_RET(yin_parse_attribute(xml_ctx, import_args, YIN_ARG_MODULE, &imp->name, YIN_ARG_MANDATORY, YANG_IMPORT));
    LY_CHECK_RET(yin_parse_content(xml_ctx, subelems, 5, data, YANG_IMPORT, NULL, &imp->exts));
    /* check prefix validity */
    LY_CHECK_RET(lysp_check_prefix((struct lys_parser_ctx *)xml_ctx, mod->imports, mod->mod->prefix, &imp->prefix), LY_EVALID);

    return yin_parse_content(xml_ctx, subelems, 5, data, YANG_IMPORT, NULL, &imp->exts);
}

LY_ERR
yin_parse_status(struct lyxml_context *xml_ctx, struct yin_arg_record **status_args, const char **data, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *value = NULL;
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_DUPELEM, "status");
        return LY_EVALID;
    }

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, status_args, YIN_ARG_VALUE, &value, YIN_ARG_MANDATORY, YANG_STATUS));
    if (strcmp(value, "current") == 0) {
        *flags |= LYS_STATUS_CURR;
    } else if (strcmp(value, "deprecated") == 0) {
        *flags |= LYS_STATUS_DEPRC;
    } else if (strcmp(value, "obsolete") == 0) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_INVAL_YIN, value, "status");
        lydict_remove(xml_ctx->ctx, value);
        return LY_EVALID;
    }
    lydict_remove(xml_ctx->ctx, value);

    return yin_parse_content(xml_ctx, subelems, 1, data, YANG_STATUS, NULL, exts);
}

LY_ERR
yin_parse_yin_element_element(struct lyxml_context *xml_ctx, struct yin_arg_record *attrs, const char **data,
                              uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[1] = {{YANG_CUSTOM, NULL, 0}};

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, &attrs, YIN_ARG_VALUE, &temp_val, YIN_ARG_MANDATORY, YANG_YIN_ELEMENT));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_YINELEM_TRUE;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_PARSER(xml_ctx, LY_VCODE_INVAL_YIN, temp_val, "yin-element");
        lydict_remove(xml_ctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(xml_ctx->ctx, temp_val);

    return yin_parse_content(xml_ctx, subelems, 1, data, YANG_YIN_ELEMENT, NULL, exts);
}

LY_ERR
yin_parse_extension_instance(struct lyxml_context *xml_ctx, struct yin_arg_record **attrs, const char **data, const char *ext_name,
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

    LY_ARRAY_NEW_RET(xml_ctx->ctx, *exts, e, LY_EMEM);

    e->yin = 0;
    /* store name and insubstmt info */
    e->name = lydict_insert(xml_ctx->ctx, ext_name, ext_name_len);
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
            last_subelem->stmt = lydict_insert(xml_ctx->ctx, iter->name, iter->name_len);
            LY_CHECK_ERR_RET(!last_subelem->stmt, LOGMEM(xml_ctx->ctx), LY_EMEM);
            if (iter->dynamic_content) {
                last_subelem->arg = lydict_insert_zc(xml_ctx->ctx, iter->content);
                LY_CHECK_ERR_RET(!last_subelem->arg, LOGMEM(xml_ctx->ctx), LY_EMEM);
            } else {
                last_subelem->arg = lydict_insert(xml_ctx->ctx, iter->content, iter->content_len);
                LY_CHECK_ERR_RET(!last_subelem->arg, LOGMEM(xml_ctx->ctx), LY_EMEM);
            }
        }
    }

    /* parse subelements */
    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
        if (ret == LY_EINVAL) {
            while (xml_ctx->status == LYXML_ELEMENT) {
                LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
                if (!name) {
                    /* end of extension instance reached */
                    break;
                }
                LY_CHECK_RET(yin_parse_element_generic(xml_ctx, name, name_len, prefix, prefix_len, data, &new_subelem));
                if (!e->child) {
                    e->child = new_subelem;
                } else {
                    last_subelem->next = new_subelem;
                }
                last_subelem = new_subelem;
            }
        }
    } else {
        LY_CHECK_RET(lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len));
        LY_CHECK_RET(name, LY_EINVAL);
    }

    return LY_SUCCESS;
}

LY_ERR
yin_parse_element_generic(struct lyxml_context *xml_ctx, const char *name, size_t name_len, const char *prefix,
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
    (*element)->stmt = lydict_insert(xml_ctx->ctx, name, name_len);
    LY_CHECK_ERR_RET(!(*element)->stmt, LOGMEM(xml_ctx->ctx), LY_EMEM);

    last = (*element)->child;
    /* load attributes */
    while(xml_ctx->status == LYXML_ATTRIBUTE) {
        /* add new element to linked-list */
        new = calloc(1, sizeof(*last));
        LY_CHECK_ERR_GOTO(ret, LOGMEM(xml_ctx->ctx), err);
        if (!(*element)->child) {
            /* save first */
            (*element)->child = new;
        } else {
            last->next = new;
        }
        last = new;

        last->flags |= LYS_YIN_ATTR;
        ret = lyxml_get_attribute(xml_ctx, data, &temp_prefix, &prefix_len, &temp_name, &temp_name_len);
        LY_CHECK_GOTO(ret, err);
        ret = lyxml_get_string(xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
        LY_CHECK_GOTO(ret, err);
        last->stmt = lydict_insert(xml_ctx->ctx, temp_name, temp_name_len);
        LY_CHECK_ERR_GOTO(!last->stmt, LOGMEM(xml_ctx->ctx); ret = LY_EMEM, err);
        /* attributes with prefix are ignored */
        if (!temp_prefix) {
            if (dynamic) {
                last->arg = lydict_insert_zc(xml_ctx->ctx, out);
                if (!last->arg) {
                    free(out);
                    LOGMEM(xml_ctx->ctx);
                    ret = LY_EMEM;
                    goto err;
                }
            } else {
                last->arg = lydict_insert(xml_ctx->ctx, out, out_len);
                LY_CHECK_ERR_GOTO(!last->arg, LOGMEM(xml_ctx->ctx); ret = LY_EMEM, err);
            }
        }
    }

    /* parse content of element */
    ret = lyxml_get_string(xml_ctx, data, &out, &out_len, &out, &out_len, &dynamic);
    if (ret == LY_EINVAL) {
        while (xml_ctx->status == LYXML_ELEMENT) {
            /* parse subelements */
            ret = lyxml_get_element(xml_ctx, data, &temp_prefix, &temp_prefix_len, &temp_name, &temp_name_len);
            LY_CHECK_GOTO(ret, err);
            if (!name) {
                /* end of element reached */
                break;
            }
            ret = yin_parse_element_generic(xml_ctx, temp_name, temp_name_len, temp_prefix, temp_prefix_len, data, &last->next);
            LY_CHECK_GOTO(ret, err);
            last = last->next;
        }
    } else {
        /* save element content */
        if (out_len != 0) {
            if (dynamic) {
                (*element)->arg = lydict_insert_zc(xml_ctx->ctx, out);
                if (!(*element)->arg) {
                    free(out);
                    LOGMEM(xml_ctx->ctx);
                    ret = LY_EMEM;
                    goto err;
                }
            } else {
                (*element)->arg = lydict_insert(xml_ctx->ctx, out, out_len);
                LY_CHECK_ERR_GOTO(!(*element)->arg, LOGMEM(xml_ctx->ctx); ret = LY_EMEM, err);
            }
        }
        /* read closing tag */
        ret = lyxml_get_element(xml_ctx, data, &temp_prefix, &prefix_len, &temp_name, &temp_name_len);
        LY_CHECK_GOTO(ret, err);
    }

    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return LY_SUCCESS;

err:
    FREE_ARRAY(xml_ctx, subelem_args, free_arg_rec);
    return ret;
}

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
LY_ERR
yin_parse_argument_element(struct lyxml_context *xml_ctx, struct yin_arg_record **attrs, const char **data,
                           struct yin_argument_meta *arg_meta, struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[2] = {{YANG_YIN_ELEMENT, arg_meta->flags, YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};

    LY_CHECK_RET(yin_parse_attribute(xml_ctx, attrs, YIN_ARG_NAME, arg_meta->argument, YIN_ARG_MANDATORY, YANG_ARGUMENT));

    return yin_parse_content(xml_ctx, subelems, 2, data, YANG_ARGUMENT, NULL, exts);
}

LY_ERR
yin_parse_extension(struct lyxml_context *xml_ctx, struct yin_arg_record **extension_args, const char **data, struct lysp_ext **extensions)
{
    struct lysp_ext *ex;
    LY_ARRAY_NEW_RET(xml_ctx->ctx, *extensions, ex, LY_EMEM);
    LY_CHECK_RET(yin_parse_attribute(xml_ctx, extension_args, YIN_ARG_NAME, &ex->name, YIN_ARG_MANDATORY, YANG_EXTENSION));

    struct yin_argument_meta arg_info = {&ex->flags, &ex->argument};
    struct yin_subelement subelems[5] = {{YANG_ARGUMENT, &arg_info, YIN_SUBELEM_UNIQUE},
                                         {YANG_DESCRIPTION, &ex->dsc, YIN_SUBELEM_UNIQUE},
                                         {YANG_REFERENCE, &ex->ref, YIN_SUBELEM_UNIQUE},
                                         {YANG_STATUS, &ex->flags, YIN_SUBELEM_UNIQUE},
                                         {YANG_CUSTOM, NULL, 0}};

    return yin_parse_content(xml_ctx, subelems, 5, data, YANG_EXTENSION, NULL, &ex->exts);
}

/**
 * @brief Parse module substatements.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in,out] data Data to read from.
 * @param[out] mod Parsed module structure.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_mod(struct lyxml_context *xml_ctx, struct yin_arg_record **mod_args, const char **data, struct lysp_module **mod)
{
    struct yin_subelement subelems[8] = {{YANG_DESCRIPTION, &(*mod)->mod->dsc, YIN_SUBELEM_UNIQUE},
                                         {YANG_IMPORT, *mod, 0},
                                         {YANG_ORGANIZATION, &(*mod)->mod->org, YIN_SUBELEM_UNIQUE},
                                         {YANG_CONTACT, &(*mod)->mod->contact, YIN_SUBELEM_UNIQUE},
                                         {YANG_PREFIX, &(*mod)->mod->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                         {YANG_REFERENCE, &(*mod)->mod->ref, YIN_SUBELEM_UNIQUE},
                                         {YANG_NAMESPACE, &(*mod)->mod->ns, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
                                         /* TODO add YANG_EXTENSION */
                                         {YANG_CUSTOM, NULL, 0}};


    LY_CHECK_RET(yin_parse_attribute(xml_ctx, mod_args, YIN_ARG_NAME, &(*mod)->mod->name, YIN_ARG_MANDATORY, YANG_MODULE));

    return yin_parse_content(xml_ctx, subelems, 8, data, YANG_MODULE, NULL, &(*mod)->exts);
}

LY_ERR
yin_parse_module(struct ly_ctx *ctx, const char *data, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    struct lys_parser_ctx parser_ctx;
    struct lyxml_context *xml_ctx = (struct lyxml_context *)&parser_ctx;
    struct lysp_module *mod_p = NULL;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct yin_arg_record *args = NULL;

    /* initialize xml context */
    memset(&parser_ctx, 0, sizeof parser_ctx);
    xml_ctx->ctx = ctx;
    xml_ctx->line = 1;

    /* check submodule */
    ret = lyxml_get_element(xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_GOTO(ret, cleanup);
    ret = yin_load_attributes(xml_ctx, &data, &args);
    LY_CHECK_GOTO(ret, cleanup);
    kw = yin_match_keyword(xml_ctx, name, name_len, prefix, prefix_len);
    if (kw == YANG_SUBMODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_MODULE) {
        LOGVAL_PARSER(xml_ctx, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".",
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
    ret = yin_parse_mod(xml_ctx, &args, &data, &mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret != LY_SUCCESS) {
        lysp_module_free(mod_p);
    }
    FREE_ARRAY(xml_ctx, args, free_arg_rec);
    lyxml_context_clear(xml_ctx);
    return ret;
}
