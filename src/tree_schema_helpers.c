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
    if (stmt) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, date_len, date, stmt);
    }
    return LY_EINVAL;
}

void
lysp_sort_revisions(struct lysp_revision *revs)
{
    uint8_t i, r;
    struct lysp_revision rev;

    for (i = 1, r = 0; revs && i < LY_ARRAY_SIZE(revs); i++) {
        if (strcmp(revs[i].date, revs[r].date) > 0) {
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

LY_ERR
lysp_parse_include(struct ly_parser_ctx *ctx, struct lysp_module *mod, const char *name, struct lysp_include *inc)
{
    struct lys_module *submod;
    const char *submodule_data = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;
    void (*submodule_data_free)(void *module_data, void *user_data) = NULL;

    /* Try to get submodule from the context, if already present */
    inc->submodule = ly_ctx_get_submodule(ctx->ctx, mod->name, name, inc->rev[0] ? inc->rev : NULL);
    if (!inc->submodule) {
        /* submodule not present in the context, get the input data and parse it */
        if (!(ctx->ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
            if (ctx->ctx->imp_clb) {
                if (ctx->ctx->imp_clb(mod->name, NULL, name, inc->rev, ctx->ctx->imp_clb_data,
                                      &format, &submodule_data, &submodule_data_free) == LY_SUCCESS) {
                    submod = lys_parse_mem_(ctx->ctx, submodule_data, format, inc->rev[0] ? inc->rev : NULL, mod->implemented);
                }
            }
            if (!submod && !(ctx->ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_file;
            }
        } else {
search_file:
            if (!(ctx->ctx->flags & LY_CTX_DISABLE_SEARCHDIRS)) {
                /* module was not received from the callback or there is no callback set */
                lys_module_localfile(ctx->ctx, name, inc->rev[0] ? inc->rev : NULL, mod->implemented, &submod);
                if (inc->submodule) {
                    ++inc->submodule->refcount;
                }
            }
            if (!submod && (ctx->ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_clb;
            }
        }
        if (submod) {
            /* check that we have really a submodule */
            if (!submod->parsed->submodule) {
                /* submodule is not a submodule */
                LOGVAL_YANG(ctx, LYVE_REFERENCE, "Included \"%s\" schema from \"%s\" is actually not a submodule.", name, mod->name);
                lys_module_free(submod, NULL);
                /* fix list of modules in context, since it was already changed */
                --ctx->ctx->list.count;
                return LY_EVALID;
            }
            /* check that the submodule belongs-to our module */
            if (strcmp(mod->name, submod->parsed->belongsto)) {
                LOGVAL_YANG(ctx, LYVE_REFERENCE, "Included \"%s\" submodule from \"%s\" belongs-to a different module \"%s\".",
                            name, mod->name, submod->parsed->belongsto);
                lys_module_free(submod, NULL);
                return LY_EVALID;
            }
            inc->submodule = submod->parsed;
            ++inc->submodule->refcount;
            free(submod);
        }
    } else {
        ++inc->submodule->refcount;
    }
    if (!inc->submodule) {
        if (ly_errcode(ctx->ctx) != LY_EVALID) {
            LOGVAL_YANG(ctx, LY_VCODE_INVAL, strlen(name), name, "include");
        } else {
            LOGVAL_YANG(ctx, LYVE_REFERENCE, "Including \"%s\" submodule into \"%s\" failed.", name, mod->name);
        }
        return LY_EVALID;
    }

    return LY_SUCCESS;
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
