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

#include "common.h"
#include "context.h"
#include "libyang.h"
#include "xml.h"
#include "tree_schema_internal.h"
#include "string.h"

LY_ERR
match_argument_name()
{

}

#define MOVE_START(array, offset) array = &array[offset];
#define IF_KW(STR, LEN, SHIFT, STMT) if (!strncmp((word), STR, LEN)) {MOVE_START(word, SHIFT);kw=STMT;}
#define IF_KW_PREFIX(STR, LEN, SHIFT) if (!strncmp((word), STR, LEN)) {MOVE_START(word, SHIFT);
#define IF_KW_PREFIX_END }

enum yang_keyword
match_keyword(const char *word, size_t len)
{
    if (!word) {
        return YANG_NONE;
    }
    enum yang_keyword kw = YANG_NONE;

    /* try to match the keyword */
    switch (*word) {
    case 'a':
        MOVE_START(word, 1);
        IF_KW("rgument", len, 7, YANG_ARGUMENT)
        else IF_KW("ugment", len, 6, YANG_AUGMENT)
        else IF_KW("ction", len, 5, YANG_ACTION)
        else IF_KW_PREFIX("ny", len, 2)
            IF_KW("data", len, 4, YANG_ANYDATA)
            else IF_KW("xml", len, 3, YANG_ANYXML)
        IF_KW_PREFIX_END
        break;
    case 'b':
        MOVE_START(word, 1);
        IF_KW("ase", len, 3, YANG_BASE)
        else IF_KW("elongs-to", len, 9, YANG_BELONGS_TO)
        else IF_KW("it", len, 2, YANG_BIT)
        break;
    case 'c':
        MOVE_START(word, 1);
        IF_KW("ase", len, 3, YANG_CASE)
        else IF_KW("hoice", len, 5, YANG_CHOICE)
        else IF_KW_PREFIX("on", len, 2)
            IF_KW("fig", len, 3, YANG_CONFIG)
            else IF_KW_PREFIX("ta", len, 2)
                IF_KW("ct", len, 2, YANG_CONTACT)
                else IF_KW("iner", len, 4, YANG_CONTAINER)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'd':
        MOVE_START(word, 1);
        IF_KW_PREFIX("e", len, 1)
            IF_KW("fault", len, 5, YANG_DEFAULT)
            else IF_KW("scription", len, 9, YANG_DESCRIPTION)
            else IF_KW_PREFIX("viat", len, 4)
                IF_KW("e", len, 1, YANG_DEVIATE)
                else IF_KW("ion", len, 3, YANG_DEVIATION)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'e':
        MOVE_START(word, 1);
        IF_KW("num", len, 3, YANG_ENUM)
        else IF_KW_PREFIX("rror-", len, 5)
            IF_KW("app-tag", len, 7, YANG_ERROR_APP_TAG)
            else IF_KW("message", len, 7, YANG_ERROR_MESSAGE)
        IF_KW_PREFIX_END
        else IF_KW("xtension", len, 8, YANG_EXTENSION)
        break;
    case 'f':
        MOVE_START(word, 1);
        IF_KW("eature", len, 6, YANG_FEATURE)
        else IF_KW("raction-digits", len, 14, YANG_FRACTION_DIGITS)
        break;
    case 'g':
        MOVE_START(word, 1);
        IF_KW("rouping", len, 7, YANG_GROUPING)
        break;
    case 'i':
        MOVE_START(word, 1);
        IF_KW("dentity", len, 7, YANG_IDENTITY)
        else IF_KW("f-feature", len, 9, YANG_IF_FEATURE)
        else IF_KW("mport", len, 5, YANG_IMPORT)
        else IF_KW_PREFIX("n", len, 1)
            IF_KW("clude", len, 5, YANG_INCLUDE)
            else IF_KW("put", len, 3, YANG_INPUT)
        IF_KW_PREFIX_END
        break;
    case 'k':
        MOVE_START(word, 1);
        IF_KW("ey", len, 2, YANG_KEY)
        break;
    case 'l':
        MOVE_START(word, 1);
        IF_KW_PREFIX("e", len, 1)
            IF_KW("af-list", len, 7, YANG_LEAF_LIST)
            else IF_KW("af", len, 2, YANG_LEAF)
            else IF_KW("ngth", len, 4, YANG_LENGTH)
        IF_KW_PREFIX_END
        else IF_KW("ist", len, 3, YANG_LIST)
        break;
    case 'm':
        MOVE_START(word, 1);
        IF_KW_PREFIX("a", len, 1)
            IF_KW("ndatory", len, 7, YANG_MANDATORY)
            else IF_KW("x-elements", len, 10, YANG_MAX_ELEMENTS)
        IF_KW_PREFIX_END
        else IF_KW("in-elements", len, 11, YANG_MIN_ELEMENTS)
        else IF_KW("ust", len, 3, YANG_MUST)
        else IF_KW_PREFIX("od", len, 2)
            IF_KW("ule", len, 3, YANG_MODULE)
            else IF_KW("ifier", len, 5, YANG_MODIFIER)
        IF_KW_PREFIX_END
        break;
    case 'n':
        MOVE_START(word, 1);
        IF_KW("amespace", len, 8, YANG_NAMESPACE)
        else IF_KW("otification", len, 11, YANG_NOTIFICATION)
        break;
    case 'o':
        MOVE_START(word, 1);
        IF_KW_PREFIX("r", len, 1)
            IF_KW("dered-by", len, 8, YANG_ORDERED_BY)
            else IF_KW("ganization", len, 10, YANG_ORGANIZATION)
        IF_KW_PREFIX_END
        else IF_KW("utput", len, 5, YANG_OUTPUT)
        break;
    case 'p':
        MOVE_START(word, 1);
        IF_KW("ath", len, 3, YANG_PATH)
        else IF_KW("attern", len, 6, YANG_PATTERN)
        else IF_KW("osition", len, 7, YANG_POSITION)
        else IF_KW_PREFIX("re", len, 2)
            IF_KW("fix", len, 3, YANG_PREFIX)
            else IF_KW("sence", len, 5, YANG_PRESENCE)
        IF_KW_PREFIX_END
        break;
    case 'r':
        MOVE_START(word, 1);
        IF_KW("ange", len, 4, YANG_RANGE)
        else IF_KW_PREFIX("e", len, 1)
            IF_KW_PREFIX("f", len, 1)
                IF_KW("erence", len, 6, YANG_REFERENCE)
                else IF_KW("ine", len, 3, YANG_REFINE)
            IF_KW_PREFIX_END
            else IF_KW("quire-instance", len, 14, YANG_REQUIRE_INSTANCE)
            else IF_KW("vision-date", len, 11, YANG_REVISION_DATE)
            else IF_KW("vision", len, 6, YANG_REVISION)
        IF_KW_PREFIX_END
        else IF_KW("pc", len, 2, YANG_RPC)
        break;
    case 's':
        MOVE_START(word, 1);
        IF_KW("tatus", len, 5, YANG_STATUS)
        else IF_KW("ubmodule", len, 8, YANG_SUBMODULE)
        break;
    case 't':
        MOVE_START(word, 1);
        IF_KW("ypedef", len, 6, YANG_TYPEDEF)
        else IF_KW("ype", len, 3, YANG_TYPE)
        break;
    case 'u':
        MOVE_START(word, 1);
        IF_KW_PREFIX("ni", len, 2)
            IF_KW("que", len, 3, YANG_UNIQUE)
            else IF_KW("ts", len, 2, YANG_UNITS)
        IF_KW_PREFIX_END
        else IF_KW("ses", len, 3, YANG_USES)
        break;
    case 'v':
        MOVE_START(word, 1);
        IF_KW("alue", len, 4, YANG_VALUE)
        break;
    case 'w':
        MOVE_START(word, 1);
        IF_KW("hen", len, 3, YANG_WHEN)
        break;
    case 'y':
        MOVE_START(word, 1);
        IF_KW("ang-version", len, 11, YANG_YANG_VERSION)
        else IF_KW("in-element", len, 10, YANG_YIN_ELEMENT)
        break;
    default:
        break;
    }

    return kw;
}

LY_ERR
parse_submodule(struct lyxml_context *xml_ctx, const char **data, struct lysp_module **mod_p)
{
    LY_ERR ret = 0;

    const char *prefix, *name, *elem;
    size_t prefix_len, name_len;

    /* inefficient keyword check - just temporary */
    ret = lyxml_get_element(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
    if ((strncmp("module", name, name_len) != 0) && (strncmp("submodule", name, name_len) != 0)) {
        LOGVAL(xml_ctx->ctx, xml_ctx->line, &xml_ctx->line, LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\".", name);
    }

    /* inefficient argument name check - just temporary */
    ret = lyxml_get_attribute(xml_ctx, data, &prefix, &prefix_len, &name, &name_len);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);
    if (strncmp("name", name, name_len)) {
        LOGVAL(xml_ctx->ctx, xml_ctx->line, &xml_ctx->line, LYVE_SYNTAX, "Invalid argument name \"%s\", expected \"name\".", name);
    }

    char *buf = NULL, *out = NULL;
    size_t buf_len = 0, out_len = 0;
    int dynamic;

    ret = lyxml_get_string(xml_ctx, data, &buf, &buf_len, &out, &out_len, &dynamic);
    LY_CHECK_ERR_RET(ret != LY_SUCCESS, LOGMEM(xml_ctx->ctx), LY_EMEM);

    (*mod_p)->name = lydict_insert(xml_ctx->ctx, out, out_len);
    return 0;
}

LY_ERR
yin_parse(struct ly_ctx *ctx, const char *data, struct lysp_module **mod_p)
{
    LY_ERR ret = 0;
    struct ly_parser_ctx context = {0};

    struct lyxml_context xml_ctx;
    xml_ctx.ctx = ctx;

    const char *prefix, *name, *elem;
    size_t prefix_len, name_len;

    parse_submodule(&xml_ctx, &data, mod_p);

    return LY_SUCCESS;
}
