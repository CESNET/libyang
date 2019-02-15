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
    YIN_ARG_NONE = 0,
    YIN_ARG_NAME,
    YIN_ARG_TARGET_NODE,
    YIN_ARG_MODULE,
    YIN_ARG_VALUE,
    YIN_ARG_TEXT,
    YIN_ARG_CONDITION,
    YIN_ARG_URI,
    YIN_ARG_DATE,
    YIN_ARG_TAG,
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

#define IF_ARG(STR, LEN, STMT) if (!strncmp((name) + already_read, STR, LEN)) {already_read+=LEN;arg=STMT;}
#define IF_ARG_PREFIX(STR, LEN) if (!strncmp((name) + already_read, STR, LEN)) {already_read+=LEN;
#define IF_ARG_PREFIX_END }

    switch(*name) {
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

    return arg;
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
parse_namespace(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod_p)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    /* check if namespace has argument uri */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_RET(ret);
    if (match_argument_name(name, name_len) != YIN_ARG_URI) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"uri\".", name);
        return LY_EVALID;
    }

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_RET(ret);
    (*mod_p)->mod->ns = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!(*mod_p)->mod->ns, LOGMEM(xml_ctx->ctx), LY_EMEM);

    /* namespace can have only one argument */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_RET(ret);
    if (name) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Unexpected argument \"%s\".", name);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse prefix statement.
 *
 * @param[in] xml_ctx Xml context.
 * @param[in, out] data Data to reda from.
 * @param[out] mod_p Module to write to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_prefix(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod_p)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    /* check if prefix has argument value */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_RET(ret);
    if (match_argument_name(name, name_len) != YIN_ARG_VALUE) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"value\".", name);
        return LY_EVALID;
    }

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_RET(ret);
    (*mod_p)->mod->prefix = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!(*mod_p)->mod->prefix, LOGMEM(xml_ctx->ctx), LY_EMEM);

    /* prefix element can have only one argument */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_RET(ret);
    if (name) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Unexpected argument \"%s\".", name);
        return LY_EVALID;
    }
    return LY_SUCCESS;
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

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    /* check if module has argument "name" */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
    if (match_argument_name(name, name_len) != YIN_ARG_NAME) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"name\".", name);
    }

    /* read module name */
    if (xml_ctx->status != LYXML_ATTR_CONTENT) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Missing value of argument \"name\"");
    }
    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
    (*mod)->mod->name = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!(*mod)->mod->name, LOGMEM(xml_ctx->ctx), LY_EMEM);

    /* read all attributes and their content only for testing */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        while (xml_ctx->status == LYXML_ATTR_CONTENT) {
            lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
        }
    }

    while (xml_ctx->status == LYXML_ELEMENT || xml_ctx->status == LYXML_ELEM_CONTENT) {

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
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
        kw = match_keyword(name, name_len);

        switch (kw) {
            case YANG_NAMESPACE:
                ret = parse_namespace(xml_ctx, data, mod);
            break;
            case YANG_PREFIX:
                ret = parse_prefix(xml_ctx, data, mod);
                /* TODO change lysp_check_prefix function to work with ctx and not parser_ctx */
                //LY_CHECK_RET(lysp_check_prefix(&xml_ctx->ctx, *mod_p, &((*mod_p)->prefix)), LY_EVALID);
            break;

            default:
                /* error */
            break;
        }
    }

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
    kw = match_keyword(name, name_len);
    if (kw == YANG_MODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains module in situation when a submodule is expected.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_SUBMODULE) {
        /* TODO log error using LOGVAL_YIN macro */
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
    kw = match_keyword(name, name_len);
    if (kw == YANG_SUBMODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != YANG_MODULE) {
        /* TODO log error using LOGVAL_YIN macro */
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
