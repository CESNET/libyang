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

#define MOVE_INPUT(DATA, COUNT) already_read+=COUNT;
#define IF_KW(STR, LEN, STMT) if (!strncmp((name) + already_read, STR, LEN)) {MOVE_INPUT(name, LEN);arg=STMT;}
#define IF_KW_PREFIX(STR, LEN) if (!strncmp((name) + already_read, STR, LEN)) {MOVE_INPUT(name, LEN);
#define IF_KW_PREFIX_END }

enum YIN_ARGUMENT
match_argument_name(const char *name, size_t len)
{
    enum YIN_ARGUMENT arg = YIN_ARG_NONE;
    size_t already_read = 0;

    switch(*name) {
        case 'c':
            MOVE_INPUT(name, 1);
            IF_KW("ondition", 8, YIN_ARG_CONDITION);
        break;

        case 'd':
            MOVE_INPUT(name, 1);
            IF_KW("ate", 3, YIN_ARG_DATE);
        break;

        case 'm':
            MOVE_INPUT(name, 1);
            IF_KW("odule", 5, YIN_ARG_MODULE);
        break;

        case 'n':
            MOVE_INPUT(name, 1);
            IF_KW("ame", 3, YIN_ARG_NAME);
        break;

        case 't':
            MOVE_INPUT(name, 1);
            IF_KW_PREFIX("a", 1)
                IF_KW("g", 1, YIN_ARG_TAG)
                else IF_KW("rget-node", 9, YIN_ARG_TARGET_NODE)
            IF_KW_PREFIX_END
            else IF_KW("ext", 3, YIN_ARG_TEXT)
        break;

        case 'u':
            MOVE_INPUT(name, 1);
            IF_KW("ri", 2, YIN_ARG_URI)
        break;

        case 'v':
            MOVE_INPUT(name, 1);
            IF_KW("alue", 4, YIN_ARG_VALUE);
        break;
    }

    /* whole keyword must be matched */
    if (already_read != len) {
        arg = YIN_ARG_NONE;
    }

    return arg;
}

LY_ERR
parse_submodule(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod_p)
{
    LY_ERR ret = 0;

    const char *prefix, *name;
    size_t prefix_len, name_len;

    /* check if module/submodule has argument "name" */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
    if (match_argument_name(name, name_len) != YIN_ARG_NAME) {
        LOGVAL(xml_ctx->ctx, xml_ctx->line, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"name\".", name);
    }

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    /* read name of module */
    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
    (*mod_p)->name = lydict_insert(xml_ctx->ctx, out, out_len);
    LY_CHECK_ERR_RET(!(*mod_p)->name, LOGMEM(xml_ctx->ctx), LY_EMEM);
    return 0;
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

    /* check if root keyword is module or submodule */
    ret = lyxml_get_element(&xml_ctx, &data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx.ctx), LY_EMEM);
    kw = match_keyword(name);
    if (kw != YANG_MODULE && kw != YANG_SUBMODULE) {
        LOGVAL(xml_ctx.ctx, xml_ctx.line, &xml_ctx.line, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".", name);
    }

    if (kw == YANG_SUBMODULE) {
        (*mod_p)->submodule = 1;
    }

    ret = parse_submodule(&xml_ctx, &data, mod_p);

    return ret;
}
