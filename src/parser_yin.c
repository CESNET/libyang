/**
 * @file parser_yin.c
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @brief YIN parser.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "libyang.h"
#include "xml.h"
#include "tree_schema_internal.h"

enum YIN_ARGUMENT {
    YIN_ARG_NONE = 0,      /**< unrecognized argument */
    YIN_ARG_NAME,          /**< argument name */
    YIN_ARG_TARGET_NODE,   /**<argument target-node */
    YIN_ARG_MODULE,        /**< argument module */
    YIN_ARG_VALUE,         /**< argument value */
    YIN_ARG_TEXT,          /**< argument text */
    YIN_ARG_CONDITION,     /**< argument condition */
    YIN_ARG_URI,           /**< argument uri */
    YIN_ARG_DATE,          /**< argument data */
    YIN_ARG_TAG,           /**< argument tag */
    YIN_ARG_XMLNS,         /**< argument xmlns */
};

/**
 * @brief Match argument name.
 *
 * @param[in] name String representing name.
 * @param[in] len Lenght of the name.
 *
 * @reurn YIN_ARGUMENT value.
 */
enum YIN_ARGUMENT
match_argument_name(const char *name, size_t len)
{
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;
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
        arg = YIN_ARG_NONE;
    }

#undef IF_ARG
#undef IF_ARG_PREFIX
#undef IF_ARG_PREFIX_END

    return arg;
}

/**
 * @brief parse xmlns statement
 *
 * @param[in] xml_ctx XML parser context.
 * @param[in, out] data Data to reda from.
 * @param[in] prefix
 */
LY_ERR
parse_xmlns(struct lyxml_context *xml_ctx, const char **data, const char *prefix, size_t prefix_len, char *element)
{
    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic = 0;
    LY_ERR ret = LY_SUCCESS;

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_RET(ret != LY_SUCCESS, ret);
    LY_CHECK_ERR_RET(out_len == 0, LOGVAL_YANG(xml_ctx, LYVE_SYNTAX_YIN, "Missing value of xmlns attribute"), LY_EEXIST);
    lyxml_ns_add(xml_ctx, element, prefix, prefix_len, out, out_len);

    return LY_SUCCESS;
}

/**
 * @brief Parse content of whole element as text.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in] element_name Name of element, name is necesary to remove xmlns definitions at the end of element
 * @param[in] data Data to read from.
 * @param[out] value Where content of element should be stored.
 */
LY_ERR
parse_text_element(struct lyxml_context *xml_ctx, char *element_name, const char **data, const char **value)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;

    const char *prefix, *name;
    size_t prefix_len, name_len;

    /* parse module attributes */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);

        arg = match_argument_name(name, name_len);
        if (arg) {
            parse_xmlns(xml_ctx, data, prefix, prefix_len, "module");
        } else {
            /* unrecognized attribute, still can be namespace definition eg. xmlns:foo=.... */
            if (match_argument_name(prefix, prefix_len) == YIN_ARG_XMLNS) {
                /* in this case prefix of namespace is actually name of attribute */
                parse_xmlns(xml_ctx, data, name, name_len, "module");
            } else {
                /* unrecognized or unexpected attribute */
                LOGERR(xml_ctx->ctx, LY_EDENIED, "Invalid argument in module element");
                return LY_EVALID;
            }
            break;
        }
    }

    LY_CHECK_RET(xml_ctx->status != LYXML_ELEM_CONTENT, LY_EVALID);

    if (xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
        LY_CHECK_RET(ret);
        *value = lydict_insert(xml_ctx->ctx, out, out_len);
        LY_CHECK_ERR_RET(!(*value), LOGMEM(xml_ctx->ctx), LY_EMEM);
    }

    lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    lyxml_ns_rm(xml_ctx, element_name);
    return LY_SUCCESS;
}

// LY_ERR
// parser_belongs_to(struct lyxml_context *xml_ctx, const char **data, const char **belongsto, const char **prefix, struct lysp_ext **extensions)
// {
//     enum yang_keyword kw = YANG_NONE;
//     LY_ERR ret = LY_SUCCESS;
//     const char *prefix_out, *name;
//     size_t prefix_len, name_len;

//     char *buf = NULL, *out = NULL;
//     size_t buf_len = 0, out_len = 0;
//     int dynamic;

//     /* check if belongs-to has argument module */
//     ret = lyxml_get_attribute(xml_ctx, data, &prefix_out, &prefix_len, &name, &name_len);
//     LY_CHECK_RET1(ret);
//     if (match_argument_name(name, name_len) != YIN_ARG_MODULE) {
//         LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"module\".", name);
//         return LY_EINVAL;
//     }

//     /* read content of argument */
//     ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
//     LY_CHECK_RET1(ret);
//     *belongsto = lydict_insert(xml_ctx->ctx, out, out_len);
//     LY_CHECK_ERR_RET(!belongsto, LOGMEM(xml_ctx->ctx), LY_EMEM);

//     /* read substatements */
//     while (xml_ctx->status == LYXML_ATTRIBUTE) {
//         ret = lyxml_get_attribute(xml_ctx, data, &prefix_out, &prefix_len, &name, &name_len);
//         LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), ret);
//         kw = match_keyword(name);

//         switch (kw) {
//         case YANG_PREFIX:
//             ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
//             *prefix = lydict_insert(xml_ctx->ctx, out, out_len);
//             break;
//         case YANG_CUSTOM:
//             /* TODO parse extension */
//             break;
//         default:
//             LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Unexpected attribute");
//             return LY_EVALID;
//         }
//     }

//     if (!prefix) {
//         LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Missing prefix");
//         return LY_EVALID;
//     }

//     return LY_SUCCESS;
// }

/**
 * @brief Parse namespace statement.
 *
 * @param[in] xml_ctx xml context.
 * @param[in, out] data Data to read from.
 * @param[in, out] mod_p Module to write to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_namespace(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;

    /* parse namespace attributes */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);

        arg = match_argument_name(name, name_len);

        switch (arg) {
        case YIN_ARG_XMLNS:
            parse_xmlns(xml_ctx, data, prefix, prefix_len, "namespace");
            break;
        case YIN_ARG_URI:
            LY_CHECK_RET(ret);
            if (match_argument_name(name, name_len) != YIN_ARG_URI) {
                LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"uri\".", name);
                return LY_EVALID;
            }
            ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
            LY_CHECK_RET(ret);
            (*mod)->mod->ns = lydict_insert(xml_ctx->ctx, out, out_len);
            LY_CHECK_ERR_RET(!(*mod)->mod->ns, LOGMEM(xml_ctx->ctx), LY_EMEM);
            break;
        default:
            /* unrecognized attribute, still can be namespace definition eg. xmlns:foo=.... */
            if (match_argument_name(prefix, prefix_len) == YIN_ARG_XMLNS) {
                /* in this case prefix of namespace is actually name of attribute */
                parse_xmlns(xml_ctx, data, name, name_len, "namespace");
            } else {
                /* unrecognized or unexpected attribute */
                LOGERR(xml_ctx->ctx, LY_EDENIED, "Invalid argument in namespace element");
                return LY_EVALID;
            }
            break;
        }
    }

    /* remove local xmlns definitions */
    lyxml_ns_rm(xml_ctx, "namespace");
    return LY_SUCCESS;
}

static LY_ERR
yin_parse_arguments(struct lyxml_context *xml_ctx, const char **data, enum YIN_ARGUMENT arg_type, const char **arg_val, char *elem_name)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;

    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);

        arg = match_argument_name(name, name_len);

        if (arg == YIN_ARG_XMLNS) {
            parse_xmlns(xml_ctx, data, prefix, prefix_len, elem_name);
            break;
        } else if (arg == arg_type) {
            LY_CHECK_RET(ret);
            ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
            LY_CHECK_RET(ret);
            *arg_val = lydict_insert(xml_ctx->ctx, out, out_len);
            LY_CHECK_ERR_RET(!(*arg_val), LOGMEM(xml_ctx->ctx), LY_EMEM);
            break;
        } else {
            /* unrecognized attribute, still can be namespace definition eg. xmlns:foo=.... */
            if (match_argument_name(prefix, prefix_len) == YIN_ARG_XMLNS) {
                /* in this case prefix of namespace is actually name of attribute */
                parse_xmlns(xml_ctx, data, name, name_len, elem_name);
            } else {
                /* unrecognized or unexpected attribute */
                LOGERR(xml_ctx->ctx, LY_EDENIED, "Invalid argument in namespace element");
                return LY_EVALID;
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse prefix statement.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in, out] data Data to reda from.
 * @param[out] mod Module to write to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_prefix(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod)
{
    LY_ERR ret = LY_SUCCESS;

    /* parse attributes */
    ret = yin_parse_arguments(xml_ctx, data, YIN_ARG_VALUE, &(*mod)->mod->prefix, "prefix");
    LY_CHECK_RET(ret != LY_SUCCESS, ret);
    /* remove local xmlns definitions */
    ret = lyxml_ns_rm(xml_ctx, "prefix");
    return ret;
}

// static LY_ERR
// yin_parse_revision_date(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod)
// {

//     return LY_SUCCESS;
// }

static LY_ERR
yin_parse_import(struct lyxml_context *xml_ctx, const char *module_prefix, const char **data, struct lysp_module **mod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw;
    struct lysp_import *imp;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;

    /* TODO fix attribute parsing and yin_parse_revision_date function */
    /* valid attributes module, xmlns */
    /* parse import attributes */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);

        arg = match_argument_name(name, name_len);

        switch (arg) {
        case YIN_ARG_XMLNS:
            parse_xmlns(xml_ctx, data, prefix, prefix_len, "prefix");
            break;
        case YIN_ARG_MODULE:
            LY_CHECK_RET(ret);
            if (match_argument_name(name, name_len) != YIN_ARG_VALUE) {
                LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"value\".", name);
                return LY_EVALID;
            }
            ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
            LY_CHECK_RET(ret);
            (*mod)->mod->prefix = lydict_insert(xml_ctx->ctx, out, out_len);
            LY_CHECK_ERR_RET(!(*mod)->mod->ns, LOGMEM(xml_ctx->ctx), LY_EMEM);
            break;
        default:
            /* unrecognized attribute, still can be namespace definition eg. xmlns:foo=.... */
            if (match_argument_name(prefix, prefix_len) == YIN_ARG_XMLNS) {
                /* in this case prefix of namespace is actually name of attribute */
                parse_xmlns(xml_ctx, data, name, name_len, "prefix");
            } else {
                /* unrecognized or unexpected attribute */
                LOGERR(xml_ctx->ctx, LY_EDENIED, "Invalid argument in prefix element");
                return LY_EVALID;
            }
            break;
        }
    }



    /* valid subelements description, prefix, reference, revision-data */
    /* allocate sized array for imports */
    LY_ARRAY_NEW_RET(xml_ctx->ctx, (*mod)->imports, imp, LY_EVALID);

    /* get value */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_RET(ret);
    if (match_argument_name(name, name_len) != YIN_ARG_MODULE) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"module\".", name);
        return LY_EVALID;
    }
    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_RET(ret);
    imp->name = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!imp->name, LOGMEM(xml_ctx->ctx), LY_EMEM);

    while ((ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len) == LY_SUCCESS && name != NULL)) {
        kw = match_keyword(name, name_len, prefix_len);
        switch (kw) {
        case YANG_PREFIX:
            parse_prefix(xml_ctx, data, mod);
            break;
        case YANG_DESCRIPTION:
            parse_text_element(xml_ctx, "description", data, &((*mod)->mod->dsc));
            break;
        case YANG_REFERENCE:
            parse_text_element(xml_ctx, "reference", data, &((*mod)->imports->ref));
            break;
        case YANG_REVISION_DATE:
            /* this is attribute of import element */
        case YANG_CUSTOM:
            /* TODO parse extension */
        default:
            /* TODO log error */
            return LY_EVALID;
        }
    }

    /* TODO add log macro and log error */
    LY_CHECK_RET(!imp->prefix);
    return ret;
}

/**
 * @brief Parse module substatements.
 *
 * @param[in] xml_ctx xml context.
 * @param[in, out] data Data to read from.
 * @param[out] mod Parsed module structure
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_mod(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    /* parse module attributes */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);

        arg = match_argument_name(name, name_len);

        switch (arg) {
        case YIN_ARG_XMLNS:
            parse_xmlns(xml_ctx, data, prefix, prefix_len, "module");
            break;
        case YIN_ARG_NAME:
            /* check for multiple definitions of name */
            LY_CHECK_ERR_RET((*mod)->mod->name, LOGVAL_YANG(xml_ctx, LYVE_SYNTAX_YIN, "Duplicit definition of module name \"%s\"", (*mod)->mod->name), LY_EEXIST);

            /* read module name */
            if (xml_ctx->status != LYXML_ATTR_CONTENT) {
                LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Missing value of argument \"name\".");
            }
            ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
            LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
            (*mod)->mod->name = lydict_insert(xml_ctx->ctx, out, out_len);
            LY_CHECK_ERR_RET(!(*mod)->mod->name, LOGMEM(xml_ctx->ctx), LY_EMEM);
            break;
        default:
            /* unrecognized attribute, still can be namespace definition eg. xmlns:foo=.... */
            if (match_argument_name(prefix, prefix_len) == YIN_ARG_XMLNS) {
                /* in this case prefix of namespace is actually name of attribute */
                parse_xmlns(xml_ctx, data, name, name_len, "module");
            } else {
                /* unrecognized or unexpected attribute */
                LOGERR(xml_ctx->ctx, LY_EDENIED, "Invalid argument in module element");
                return LY_EVALID;
            }
            break;
        }
    }

    LY_CHECK_ERR_RET(!(*mod)->mod->name, LOGVAL_YANG(xml_ctx, LYVE_SYNTAX_YIN, "Missing argument name of a module", (*mod)->mod->name), LY_ENOTFOUND);

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_ERR_RET(ret != LY_EINVAL, LOGVAL_YANG(xml_ctx, LYVE_SYNTAX_YIN, "Expected new xml element after module element."), LY_EINVAL);

    /* loop over all elements and parse them */
    while (xml_ctx->status != LYXML_END) {
/* TODO ADD error log to macro */
#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {return LY_EVALID;}mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case YANG_NAMESPACE:
        case YANG_PREFIX:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case YANG_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case YANG_INCLUDE:
        case YANG_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case YANG_ORGANIZATION:
        case YANG_CONTACT:
        case YANG_DESCRIPTION:
        case YANG_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case YANG_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case YANG_ANYDATA:
        case YANG_ANYXML:
        case YANG_AUGMENT:
        case YANG_CHOICE:
        case YANG_CONTAINER:
        case YANG_DEVIATION:
        case YANG_EXTENSION:
        case YANG_FEATURE:
        case YANG_GROUPING:
        case YANG_IDENTITY:
        case YANG_LEAF:
        case YANG_LEAF_LIST:
        case YANG_LIST:
        case YANG_NOTIFICATION:
        case YANG_RPC:
        case YANG_TYPEDEF:
        case YANG_USES:
        case YANG_CUSTOM:
            mod_stmt = Y_MOD_BODY;
            break;
        default:
            /* error will be handled in the next switch */
            break;
        }
#undef CHECK_ORDER

        ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_RET(ret != LY_SUCCESS, LY_EMEM);

        if (name) {
            kw = match_keyword(name, name_len, prefix_len);
            switch (kw) {

            /* module header */
            case YANG_NAMESPACE:
                LY_CHECK_RET(parse_namespace(xml_ctx, data, mod));
                break;
            case YANG_PREFIX:
                LY_CHECK_RET(parse_prefix(xml_ctx, data, mod));
                break;

            /* linkage */
            case YANG_IMPORT:
                yin_parse_import(xml_ctx, (*mod)->mod->prefix, data, mod);
                break;

            /* meta */
            case YANG_ORGANIZATION:
                LY_CHECK_RET(parse_text_element(xml_ctx, "organization", data, &(*mod)->mod->org));
                break;
            case YANG_CONTACT:
                LY_CHECK_RET(parse_text_element(xml_ctx, "contact", data, &(*mod)->mod->contact));
                break;
            case YANG_DESCRIPTION:
                LY_CHECK_RET(parse_text_element(xml_ctx, "description", data, &(*mod)->mod->dsc));
                break;
            case YANG_REFERENCE:
                LY_CHECK_RET(parse_text_element(xml_ctx, "reference", data, &(*mod)->mod->ref));
                break;

            default:
                return LY_EVALID;
                break;
            }
        }
    }

    lyxml_ns_rm(xml_ctx, "module");
    return ret;
}

/**
 * @brief Parse yin submodule.
 *
 * @param[in] ctx Context of YANG schemas.
 * @param[in] data Data to read from.
 * @param[out] submod Module to write to.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_submodule(struct ly_ctx *ctx, const char *data, struct lysp_submodule **submod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    struct lyxml_context xml_ctx;
    struct lysp_submodule *mod_p = NULL;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    /* initialize xml context */
    memset(&xml_ctx, 0, sizeof xml_ctx);
    xml_ctx.ctx = ctx;
    xml_ctx.line = 1;

    /* check submodule */
    ret = lyxml_get_element(&xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    kw = match_keyword(name, name_len, prefix_len);
    if (kw == YANG_MODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains module in situation when a submodule is expected.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_SUBMODULE) {
        LOGVAL_YANG(&xml_ctx, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".",
               ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* allocate module */
    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(ctx), cleanup);
    mod_p->parsing = 1;

    /* parser submodule substatements */
    //ret = parse_submod(&xml_ctx, &data, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    mod_p->parsing = 0;
    *submod = mod_p;

cleanup:
    if (ret) {
        lysp_submodule_free(ctx, mod_p);
    }

    lyxml_context_clear(&xml_ctx);
    return ret;
}

/**
 * @brief Parse yin module.
 *
 * @param[in] ctx Context of YANG schemas.
 * @param[in] data Data to read from.
 * @param[out] mod Module to write to.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_module(struct ly_ctx *ctx, const char *data, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    struct lyxml_context xml_ctx;
    struct lysp_module *mod_p = NULL;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    /* initialize xml context */
    memset(&xml_ctx, 0, sizeof xml_ctx);
    xml_ctx.ctx = ctx;
    xml_ctx.line = 1;

    /* check submodule */
    ret = lyxml_get_element(&xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
    kw = match_keyword(name, name_len, prefix_len);
    if (kw == YANG_SUBMODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_MODULE) {
        LOGVAL_YANG(&xml_ctx, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".",
               ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* allocate module */
    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(ctx), cleanup);
    mod_p->mod = mod;
    mod_p->parsing = 1;

    /* parser module substatements */
    ret = parse_mod(&xml_ctx, &data, &mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret) {
        lysp_module_free(mod_p);
    }

    lyxml_context_clear(&xml_ctx);
    return ret;
}
