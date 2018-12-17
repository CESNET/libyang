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

enum YIN_ARGUMENT
match_argument_name(const char *name, size_t len)
{
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;
    size_t already_read = 0;

#define MOVE_DATA(DATA, COUNT) already_read+=COUNT;
#define IF_ARG(STR, LEN, STMT) if (!strncmp((name) + already_read, STR, LEN)) {MOVE_DATA(name, LEN);arg=STMT;}
#define IF_ARG_PREFIX(STR, LEN) if (!strncmp((name) + already_read, STR, LEN)) {MOVE_DATA(name, LEN);
#define IF_ARG_PREFIX_END }

    switch(*name) {
        case 'c':
            MOVE_DATA(name, 1);
            IF_ARG("ondition", 8, YIN_ARG_CONDITION);
        break;

        case 'd':
            MOVE_DATA(name, 1);
            IF_ARG("ate", 3, YIN_ARG_DATE);
        break;

        case 'm':
            MOVE_DATA(name, 1);
            IF_ARG("odule", 5, YIN_ARG_MODULE);
        break;

        case 'n':
            MOVE_DATA(name, 1);
            IF_ARG("ame", 3, YIN_ARG_NAME);
        break;

        case 't':
            MOVE_DATA(name, 1);
            IF_ARG_PREFIX("a", 1)
                IF_ARG("g", 1, YIN_ARG_TAG)
                else IF_ARG("rget-node", 9, YIN_ARG_TARGET_NODE)
            IF_ARG_PREFIX_END
            else IF_ARG("ext", 3, YIN_ARG_TEXT)
        break;

        case 'u':
            MOVE_DATA(name, 1);
            IF_ARG("ri", 2, YIN_ARG_URI)
        break;

        case 'v':
            MOVE_DATA(name, 1);
            IF_ARG("alue", 4, YIN_ARG_VALUE);
        break;
    }

    /* whole argument must be matched */
    if (already_read != len) {
        arg = YIN_ARG_NONE;
    }

    return arg;
}

LY_ERR
parser_belongs_to(struct lyxml_context *xml_ctx, const char **data, const char **belongsto, const char **prefix, struct lysp_ext **extensions)
{
    enum yang_keyword kw = YANG_NONE;
    LY_ERR ret = LY_SUCCESS;
    const char *prefix_out, *name;
    size_t prefix_len, name_len;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    /* check if belongs-to has argument module */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix_out, &prefix_len, &name, &name_len);
    LY_CHECK_RET1(ret);
    if (match_argument_name(name, name_len) != YIN_ARG_MODULE) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"module\".", name);
        return LY_EINVAL;
    }

    /* read content of argument */
    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_RET1(ret);
    *belongsto = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!belongsto, LOGMEM(xml_ctx->ctx), LY_EMEM);

    /* read substatements */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        ret = lyxml_get_attribute(xml_ctx, data, &prefix_out, &prefix_len, &name, &name_len);
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), ret);
        kw = match_keyword(name);

        switch (kw) {
        case YANG_PREFIX:
            ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
            *prefix = lydict_insert(xml_ctx->ctx, out, out_len);
            break;
        case YANG_CUSTOM:
            /* TODO parse extension */
            break;
        default:
            LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Unexpected attribute");
            return LY_EVALID;
        }
    }

    if (!prefix) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Missing prefix");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

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
    LY_CHECK_RET1(ret);
    if (match_argument_name(name, name_len) != YIN_ARG_URI) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"uri\".", name);
        return LY_EVALID;
    }

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_RET1(ret);
    (*mod_p)->ns = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!(*mod_p)->ns, LOGMEM(xml_ctx->ctx), LY_EMEM);

    /* namespace can only have one argument */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_RET1(ret);
    if (name) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Unexpected argument \"%s\".", name);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

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
    LY_CHECK_RET1(ret);
    if (match_argument_name(name, name_len) != YIN_ARG_VALUE) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"value\".", name);
        return LY_EVALID;
    }

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_RET1(ret);
    (*mod_p)->prefix = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!(*mod_p)->prefix, LOGMEM(xml_ctx->ctx), LY_EMEM);

    /* prefix element can only have one argument */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_RET1(ret);
    if (name) {
        LOGVAL(xml_ctx->ctx, LY_VLOG_LINE, &xml_ctx->line, LYVE_SYNTAX, "Unexpected argument \"%s\".", name);
        return LY_EVALID;
    }
    return LY_SUCCESS;
}

LY_ERR
parse_submodule(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod_p)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    /* check if module/submodule has argument "name" */
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
    (*mod_p)->name = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!(*mod_p)->name, LOGMEM(xml_ctx->ctx), LY_EMEM);

    /* read all attributes and their content only for testing */
    while (xml_ctx->status == LYXML_ATTRIBUTE) {
        lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        while (xml_ctx->status == LYXML_ATTR_CONTENT) {
            lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
        }
    }


    while (xml_ctx->status == LYXML_ELEMENT || xml_ctx->status == LYXML_ELEM_CONTENT) {
        ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
        kw = match_keyword(name);

        switch (kw) {
            case YANG_NAMESPACE:
                ret = parse_namespace(xml_ctx, data, mod_p);
            break;
            case YANG_PREFIX:
                ret = parse_prefix(xml_ctx, data, mod_p);
                /* TODO change lysp_check_prefix function to work with ctx and not parser_ctx */
                //LY_CHECK_RET(lysp_check_prefix(&xml_ctx->ctx, *mod_p, &((*mod_p)->prefix)), LY_EVALID);
            break;
            case YANG_BELONGS_TO:
                ret = parser_belongs_to(xml_ctx, data, &(*mod_p)->belongsto, &(*mod_p)->prefix, &(*mod_p)->extensions);
            break;

            default:
                /* error */
            break;
        }
    }

    return ret;
}

LY_ERR
yin_parse(struct ly_ctx *ctx, const char *data, struct lysp_module **mod_p)
{
    LY_ERR ret = LY_SUCCESS;
    enum yang_keyword kw = YANG_NONE;
    struct lyxml_context xml_ctx;

    memset(&xml_ctx, 0, sizeof xml_ctx);
    xml_ctx.ctx = ctx;
    xml_ctx.line = 1;

    const char *prefix, *name;
    size_t prefix_len, name_len;

    /* check if root element is module or submodule */
    ret = lyxml_get_element(&xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_GOTO(ret != LY_SUCCESS, error);
    kw = match_keyword(name);
    if (kw != YANG_MODULE && kw != YANG_SUBMODULE) {
        LOGVAL(xml_ctx.ctx, LY_VLOG_LINE, &xml_ctx.line, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".", name);
    }

    if (kw == YANG_SUBMODULE) {
        (*mod_p)->submodule = 1;
    }

    ret = parse_submodule(&xml_ctx, &data, mod_p);

    lyxml_context_clear(&xml_ctx);
    return ret;

error:
    lyxml_context_clear(&xml_ctx);
    return ret;
}
