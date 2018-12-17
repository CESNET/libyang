/**
 * @file tree_schema_helpers.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsing and validation helper functions
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _XOPEN_SOURCE

#include <ctype.h>
#include <limits.h>
#include <time.h>

#include "libyang.h"
#include "common.h"
#include "tree_schema_internal.h"

LY_ERR
lysp_check_prefix(struct ly_parser_ctx *ctx, struct lysp_module *module, const char **value)
{
    struct lysp_import *i;

    if (module->prefix && &module->prefix != value && !strcmp(module->prefix, *value)) {
        LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE,
               "Prefix \"%s\" already used as module prefix.", *value);
        return LY_EEXIST;
    }
    if (module->imports) {
        LY_ARRAY_FOR(module->imports, struct lysp_import, i) {
            if (i->prefix && &i->prefix != value && !strcmp(i->prefix, *value)) {
                LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE,
                       "Prefix \"%s\" already used to import \"%s\" module.", *value, i->name);
                return LY_EEXIST;
            }
        }
    }
    return LY_SUCCESS;
}

LY_ERR
lysp_check_date(struct ly_ctx *ctx, const char *date, int date_len, const char *stmt)
{
    int i;
    struct tm tm, tm_;
    char *r;

    LY_CHECK_ARG_RET(ctx, date, LY_EINVAL);
    LY_CHECK_ERR_RET(date_len != LY_REV_SIZE - 1, LOGARG(ctx, date_len), LY_EINVAL);

    /* check format */
    for (i = 0; i < date_len; i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') {
                goto error;
            }
        } else if (!isdigit(date[i])) {
            goto error;
        }
    }

    /* check content, e.g. 2018-02-31 */
    memset(&tm, 0, sizeof tm);
    r = strptime(date, "%Y-%m-%d", &tm);
    if (!r || r != &date[LY_REV_SIZE - 1]) {
        goto error;
    }
    memcpy(&tm_, &tm, sizeof tm);
    mktime(&tm_); /* mktime modifies tm_ if it refers invalid date */
    if (tm.tm_mday != tm_.tm_mday) { /* e.g 2018-02-29 -> 2018-03-01 */
        /* checking days is enough, since other errors
         * have been checked by strptime() */
        goto error;
    }

    return LY_SUCCESS;

error:
    LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, date_len, date, stmt);
    return LY_EINVAL;
}

void
lysp_sort_revisions(struct lysp_revision *revs)
{
    uint8_t i, r;
    struct lysp_revision rev;

    for (i = 1, r = 0; revs && i < LY_ARRAY_SIZE(revs); i++) {
        if (strcmp(revs[i].rev, revs[r].rev) > 0) {
            r = i;
        }
    }

    if (r) {
        /* the newest revision is not on position 0, switch them */
        memcpy(&rev, &revs[0], sizeof rev);
        memcpy(&revs[0], &revs[r], sizeof rev);
        memcpy(&revs[r], &rev, sizeof rev);
    }
}

struct lysc_module *
lysc_module_find_prefix(struct lysc_module *mod, const char *prefix, size_t len)
{
    struct lysc_import *imp;

    assert(mod);

    if (!strncmp(mod->prefix, prefix, len) && mod->prefix[len] == '\0') {
        /* it is the prefix of the module itself */
        return mod;
    }

    /* search in imports */
    LY_ARRAY_FOR(mod->imports, struct lysc_import, imp) {
        if (!strncmp(imp->prefix, prefix, len) && mod->prefix[len] == '\0') {
            return imp->module;
        }
    }

    return NULL;
}

enum yang_keyword
match_keyword(char *data)
{
/* TODO make this function usable in get_keyword function */
#define MOVE_INPUT(DATA, COUNT) (data)+=COUNT;
#define IF_KW(STR, LEN, STMT) if (!strncmp((data), STR, LEN)) {MOVE_INPUT(data, LEN);kw=STMT;}
#define IF_KW_PREFIX(STR, LEN) if (!strncmp((data), STR, LEN)) {MOVE_INPUT(data, LEN);
#define IF_KW_PREFIX_END }

    enum yang_keyword kw = YANG_NONE;
    /* read the keyword itself */
    switch (*data) {
    case 'a':
        MOVE_INPUT(data, 1);
        IF_KW("rgument", 7, YANG_ARGUMENT)
        else IF_KW("ugment", 6, YANG_AUGMENT)
        else IF_KW("ction", 5, YANG_ACTION)
        else IF_KW_PREFIX("ny", 2)
            IF_KW("data", 4, YANG_ANYDATA)
            else IF_KW("xml", 3, YANG_ANYXML)
        IF_KW_PREFIX_END
        break;
    case 'b':
        MOVE_INPUT(data, 1);
        IF_KW("ase", 3, YANG_BASE)
        else IF_KW("elongs-to", 9, YANG_BELONGS_TO)
        else IF_KW("it", 2, YANG_BIT)
        break;
    case 'c':
        MOVE_INPUT(data, 1);
        IF_KW("ase", 3, YANG_CASE)
        else IF_KW("hoice", 5, YANG_CHOICE)
        else IF_KW_PREFIX("on", 2)
            IF_KW("fig", 3, YANG_CONFIG)
            else IF_KW_PREFIX("ta", 2)
                IF_KW("ct", 2, YANG_CONTACT)
                else IF_KW("iner", 4, YANG_CONTAINER)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'd':
        MOVE_INPUT(data, 1);
        IF_KW_PREFIX("e", 1)
            IF_KW("fault", 5, YANG_DEFAULT)
            else IF_KW("scription", 9, YANG_DESCRIPTION)
            else IF_KW_PREFIX("viat", 4)
                IF_KW("e", 1, YANG_DEVIATE)
                else IF_KW("ion", 3, YANG_DEVIATION)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'e':
        MOVE_INPUT(data, 1);
        IF_KW("num", 3, YANG_ENUM)
        else IF_KW_PREFIX("rror-", 5)
            IF_KW("app-tag", 7, YANG_ERROR_APP_TAG)
            else IF_KW("message", 7, YANG_ERROR_MESSAGE)
        IF_KW_PREFIX_END
        else IF_KW("xtension", 8, YANG_EXTENSION)
        break;
    case 'f':
        MOVE_INPUT(data, 1);
        IF_KW("eature", 6, YANG_FEATURE)
        else IF_KW("raction-digits", 14, YANG_FRACTION_DIGITS)
        break;
    case 'g':
        MOVE_INPUT(data, 1);
        IF_KW("rouping", 7, YANG_GROUPING)
        break;
    case 'i':
        MOVE_INPUT(data, 1);
        IF_KW("dentity", 7, YANG_IDENTITY)
        else IF_KW("f-feature", 9, YANG_IF_FEATURE)
        else IF_KW("mport", 5, YANG_IMPORT)
        else IF_KW_PREFIX("n", 1)
            IF_KW("clude", 5, YANG_INCLUDE)
            else IF_KW("put", 3, YANG_INPUT)
        IF_KW_PREFIX_END
        break;
    case 'k':
        MOVE_INPUT(data, 1);
        IF_KW("ey", 2, YANG_KEY)
        break;
    case 'l':
        MOVE_INPUT(data, 1);
        IF_KW_PREFIX("e", 1)
            IF_KW("af-list", 7, YANG_LEAF_LIST)
            else IF_KW("af", 2, YANG_LEAF)
            else IF_KW("ngth", 4, YANG_LENGTH)
        IF_KW_PREFIX_END
        else IF_KW("ist", 3, YANG_LIST)
        break;
    case 'm':
        MOVE_INPUT(data, 1);
        IF_KW_PREFIX("a", 1)
            IF_KW("ndatory", 7, YANG_MANDATORY)
            else IF_KW("x-elements", 10, YANG_MAX_ELEMENTS)
        IF_KW_PREFIX_END
        else IF_KW("in-elements", 11, YANG_MIN_ELEMENTS)
        else IF_KW("ust", 3, YANG_MUST)
        else IF_KW_PREFIX("od", 2)
            IF_KW("ule", 3, YANG_MODULE)
            else IF_KW("ifier", 5, YANG_MODIFIER)
        IF_KW_PREFIX_END
        break;
    case 'n':
        MOVE_INPUT(data, 1);
        IF_KW("amespace", 8, YANG_NAMESPACE)
        else IF_KW("otification", 11, YANG_NOTIFICATION)
        break;
    case 'o':
        MOVE_INPUT(data, 1);
        IF_KW_PREFIX("r", 1)
            IF_KW("dered-by", 8, YANG_ORDERED_BY)
            else IF_KW("ganization", 10, YANG_ORGANIZATION)
        IF_KW_PREFIX_END
        else IF_KW("utput", 5, YANG_OUTPUT)
        break;
    case 'p':
        MOVE_INPUT(data, 1);
        IF_KW("ath", 3, YANG_PATH)
        else IF_KW("attern", 6, YANG_PATTERN)
        else IF_KW("osition", 7, YANG_POSITION)
        else IF_KW_PREFIX("re", 2)
            IF_KW("fix", 3, YANG_PREFIX)
            else IF_KW("sence", 5, YANG_PRESENCE)
        IF_KW_PREFIX_END
        break;
    case 'r':
        MOVE_INPUT(data, 1);
        IF_KW("ange", 4, YANG_RANGE)
        else IF_KW_PREFIX("e", 1)
            IF_KW_PREFIX("f", 1)
                IF_KW("erence", 6, YANG_REFERENCE)
                else IF_KW("ine", 3, YANG_REFINE)
            IF_KW_PREFIX_END
            else IF_KW("quire-instance", 14, YANG_REQUIRE_INSTANCE)
            else IF_KW("vision-date", 11, YANG_REVISION_DATE)
            else IF_KW("vision", 6, YANG_REVISION)
        IF_KW_PREFIX_END
        else IF_KW("pc", 2, YANG_RPC)
        break;
    case 's':
        MOVE_INPUT(data, 1);
        IF_KW("tatus", 5, YANG_STATUS)
        else IF_KW("ubmodule", 8, YANG_SUBMODULE)
        break;
    case 't':
        MOVE_INPUT(data, 1);
        IF_KW("ypedef", 6, YANG_TYPEDEF)
        else IF_KW("ype", 3, YANG_TYPE)
        break;
    case 'u':
        MOVE_INPUT(data, 1);
        IF_KW_PREFIX("ni", 2)
            IF_KW("que", 3, YANG_UNIQUE)
            else IF_KW("ts", 2, YANG_UNITS)
        IF_KW_PREFIX_END
        else IF_KW("ses", 3, YANG_USES)
        break;
    case 'v':
        MOVE_INPUT(data, 1);
        IF_KW("alue", 4, YANG_VALUE)
        break;
    case 'w':
        MOVE_INPUT(data, 1);
        IF_KW("hen", 3, YANG_WHEN)
        break;
    case 'y':
        MOVE_INPUT(data, 1);
        IF_KW("ang-version", 11, YANG_YANG_VERSION)
        else IF_KW("in-element", 10, YANG_YIN_ELEMENT)
        break;
    case ';':
        MOVE_INPUT(data, 1);
        kw = YANG_SEMICOLON;
        //goto success;
        break;
    case '{':
        MOVE_INPUT(data, 1);
        kw = YANG_LEFT_BRACE;
        //goto success;
        break;
    case '}':
        MOVE_INPUT(data, 1);
        kw = YANG_RIGHT_BRACE;
        //goto success;
        break;
    default:
        break;
    }

    /* TODO important fix whole keyword must be matched */
    return kw;
}
