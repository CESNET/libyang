/**
 * @file common.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief common internal definitions for libyang
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "tree_schema.h"

const char *const ly_stmt_list[] = {
    [YANG_ACTION] = "action",
    [YANG_ANYDATA] = "anydata",
    [YANG_ANYXML] = "anyxml",
    [YANG_ARGUMENT] = "argument",
    [YANG_AUGMENT] = "augment",
    [YANG_BASE] = "base",
    [YANG_BELONGS_TO] = "belongs-to",
    [YANG_BIT] = "bit",
    [YANG_CASE] = "case",
    [YANG_CHOICE] = "choice",
    [YANG_CONFIG] = "config",
    [YANG_CONTACT] = "contact",
    [YANG_CONTAINER] = "container",
    [YANG_CUSTOM] = "<extension-instance>",
    [YANG_DEFAULT] = "default",
    [YANG_DESCRIPTION] = "description",
    [YANG_DEVIATE] = "deviate",
    [YANG_DEVIATION] = "deviation",
    [YANG_ENUM] = "enum",
    [YANG_ERROR_APP_TAG] = "error-app-tag",
    [YANG_ERROR_MESSAGE] = "error-message",
    [YANG_EXTENSION] = "extension",
    [YANG_FEATURE] = "feature",
    [YANG_FRACTION_DIGITS] = "fraction-digits",
    [YANG_GROUPING] = "grouping",
    [YANG_IDENTITY] = "identitiy",
    [YANG_IF_FEATURE] = "if-feature",
    [YANG_IMPORT] = "import",
    [YANG_INCLUDE] = "include",
    [YANG_INPUT] = "input",
    [YANG_KEY] = "key",
    [YANG_LEAF] = "leaf",
    [YANG_LEAF_LIST] = "leaf-list",
    [YANG_LENGTH] = "length",
    [YANG_LIST] = "list",
    [YANG_MANDATORY] = "mandatory",
    [YANG_MAX_ELEMENTS] = "max-elements",
    [YANG_MIN_ELEMENTS] = "min-elements",
    [YANG_MODIFIER] = "modifier",
    [YANG_MODULE] = "module",
    [YANG_MUST] = "must",
    [YANG_NAMESPACE] = "namespace",
    [YANG_NOTIFICATION] = "notification",
    [YANG_ORDERED_BY] = "ordered-by",
    [YANG_ORGANIZATION] = "organization",
    [YANG_OUTPUT] = "output",
    [YANG_PATH] = "path",
    [YANG_PATTERN] = "pattern",
    [YANG_POSITION] = "position",
    [YANG_PREFIX] = "prefix",
    [YANG_PRESENCE] = "presence",
    [YANG_RANGE] = "range",
    [YANG_REFERENCE] = "reference",
    [YANG_REFINE] = "refine",
    [YANG_REQUIRE_INSTANCE] = "require-instance",
    [YANG_REVISION] = "revision",
    [YANG_REVISION_DATE] = "revision-date",
    [YANG_RPC] = "rpc",
    [YANG_STATUS] = "status",
    [YANG_SUBMODULE] = "submodule",
    [YANG_TYPE] = "type",
    [YANG_TYPEDEF] = "typedef",
    [YANG_UNIQUE] = "unique",
    [YANG_UNITS] = "units",
    [YANG_USES] = "uses",
    [YANG_VALUE] = "value",
    [YANG_WHEN] = "when",
    [YANG_YANG_VERSION] = "yang-version",
    [YANG_YIN_ELEMENT] = "yin-element",
    [YANG_SEMICOLON] = ";",
    [YANG_LEFT_BRACE] = "{",
    [YANG_RIGHT_BRACE] = "}",
};

const char *const lyext_substmt_list[] = {
    [LYEXT_SUBSTMT_ARGUMENT] = "argument",
    [LYEXT_SUBSTMT_BASE] = "base",
    [LYEXT_SUBSTMT_BELONGSTO] = "belongs-to",
    [LYEXT_SUBSTMT_CONTACT] = "contact",
    [LYEXT_SUBSTMT_DEFAULT] = "default",
    [LYEXT_SUBSTMT_DESCRIPTION] = "description",
    [LYEXT_SUBSTMT_ERRTAG] = "error-app-tag",
    [LYEXT_SUBSTMT_ERRMSG] = "error-message",
    [LYEXT_SUBSTMT_KEY] = "key",
    [LYEXT_SUBSTMT_NAMESPACE] = "namespace",
    [LYEXT_SUBSTMT_ORGANIZATION] = "organization",
    [LYEXT_SUBSTMT_PATH] = "path",
    [LYEXT_SUBSTMT_PREFIX] = "prefix",
    [LYEXT_SUBSTMT_PRESENCE] = "presence",
    [LYEXT_SUBSTMT_REFERENCE] = "reference",
    [LYEXT_SUBSTMT_REVISIONDATE] = "revision-date",
    [LYEXT_SUBSTMT_UNITS] = "units",
    [LYEXT_SUBSTMT_VALUE] = "value",
    [LYEXT_SUBSTMT_VERSION] = "yang-version",
    [LYEXT_SUBSTMT_MODIFIER] = "modifier",
    [LYEXT_SUBSTMT_REQINSTANCE] = "require-instance",
    [LYEXT_SUBSTMT_YINELEM] = "yin-element",
    [LYEXT_SUBSTMT_CONFIG] = "config",
    [LYEXT_SUBSTMT_MANDATORY] = "mandatory",
    [LYEXT_SUBSTMT_ORDEREDBY] = "ordered-by",
    [LYEXT_SUBSTMT_STATUS] = "status",
    [LYEXT_SUBSTMT_FRACDIGITS] = "fraction-digits",
    [LYEXT_SUBSTMT_MAX] = "max-elements",
    [LYEXT_SUBSTMT_MIN] = "min-elements",
    [LYEXT_SUBSTMT_POSITION] = "position",
    [LYEXT_SUBSTMT_UNIQUE] = "unique",
    [LYEXT_SUBSTMT_IFFEATURE] = "if-feature",
};

const char *const ly_devmod_list[] = {
    [LYS_DEV_NOT_SUPPORTED] = "not-supported",
    [LYS_DEV_ADD] = "add",
    [LYS_DEV_DELETE] = "delete",
    [LYS_DEV_REPLACE] = "replace",
};

void *
ly_realloc(void *ptr, size_t size)
{
    void *new_mem;

    new_mem = realloc(ptr, size);
    if (!new_mem) {
        free(ptr);
    }

    return new_mem;
}

size_t
LY_VCODE_INSTREXP_len(const char *str)
{
    size_t len = 0;
    if (!str) {
        return len;
    } else if (!str[0]) {
        return 1;
    }
    for (len = 1; len < LY_VCODE_INSTREXP_MAXLEN && str[len]; ++len);
    return len;
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
