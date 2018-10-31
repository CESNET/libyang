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

void
lys_module_implement(struct lys_module *mod)
{
    assert(mod);
    if (mod->parsed) {
        mod->parsed->implemented = 1;
    }
    if (mod->compiled) {
        mod->compiled->implemented = 1;
    }
}

LY_ERR
lysp_load_module(struct ly_ctx *ctx, const char *name, const char *revision, int implement, int require_parsed, struct lys_module **mod)
{
    const char *submodule_data = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;
    void (*submodule_data_free)(void *module_data, void *user_data) = NULL;

    /* try to get the module from the context */
    if (revision) {
        *mod = (struct lys_module*)ly_ctx_get_module(ctx, name, revision);
    } else {
        *mod = (struct lys_module*)ly_ctx_get_module_latest(ctx, name);
    }

    if (!(*mod) || (require_parsed && !(*mod)->parsed)) {
        (*mod) = NULL;

        /* check collision with other implemented revision */
        if (implement && ly_ctx_get_module_implemented(ctx, name)) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE,
                   "Module \"%s\" is already present in other implemented revision.", name);
            return LY_EDENIED;
        }

        /* submodule not present in the context, get the input data and parse it */
        if (!(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
            if (ctx->imp_clb) {
                if (ctx->imp_clb(name, revision, NULL, NULL, ctx->imp_clb_data,
                                      &format, &submodule_data, &submodule_data_free) == LY_SUCCESS) {
                    *mod = lys_parse_mem_(ctx, submodule_data, format, revision, implement);
                }
            }
            if (!(*mod) && !(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_file;
            }
        } else {
search_file:
            if (!(ctx->flags & LY_CTX_DISABLE_SEARCHDIRS)) {
                /* module was not received from the callback or there is no callback set */
                lys_module_localfile(ctx, name, revision, implement, mod);
            }
            if (!(*mod) && (ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_clb;
            }
        }
    } else {
        /* we have module from the current context */
        if (implement && (ly_ctx_get_module_implemented(ctx, name) != *mod)) {
            /* check collision with other implemented revision */
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE,
                   "Module \"%s\" is already present in other implemented revision.", name);
            *mod = NULL;
            return LY_EDENIED;
        }

        /* circular check */
        if ((*mod)->parsed->parsing) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "A circular dependency (import) for module \"%s\".", name);
            *mod = NULL;
            return LY_EVALID;
        }
    }
    if (!(*mod)) {
        if (ly_errcode(ctx) != LY_EVALID) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_INVAL, strlen(name), name, "import");
        } else {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Loading \"%s\" module failed.", name);
        }
        return LY_EVALID;
    }

    if (implement) {
        /* mark the module implemented, check for collision was already done */
        lys_module_implement(*mod);
    }
    if (!revision && ((*mod)->parsed->latest_revision == 1)) {
        /* update the latest_revision flag - here we have selected the latest available schema */
        (*mod)->parsed->latest_revision = 2;
    }

    return LY_SUCCESS;
}

LY_ERR
lysp_load_submodule(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_include *inc)
{
    struct lys_module *submod;
    const char *submodule_data = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;
    void (*submodule_data_free)(void *module_data, void *user_data) = NULL;

    /* Try to get submodule from the context, if already present */
    inc->submodule = ly_ctx_get_submodule(ctx, mod->name, inc->name, inc->rev[0] ? inc->rev : NULL);
    if (!inc->submodule || (!inc->rev[0] && inc->submodule->latest_revision != 2)) {
        /* submodule not present in the context, get the input data and parse it */
        if (!(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
            if (ctx->imp_clb) {
                if (ctx->imp_clb(mod->name, NULL, inc->name, inc->rev[0] ? inc->rev : NULL, ctx->imp_clb_data,
                                      &format, &submodule_data, &submodule_data_free) == LY_SUCCESS) {
                    submod = lys_parse_mem_(ctx, submodule_data, format, inc->rev[0] ? inc->rev : NULL, mod->implemented);
                }
            }
            if (!submod && !(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_file;
            }
        } else {
search_file:
            if (!(ctx->flags & LY_CTX_DISABLE_SEARCHDIRS)) {
                /* module was not received from the callback or there is no callback set */
                lys_module_localfile(ctx, inc->name, inc->rev[0] ? inc->rev : NULL, mod->implemented, &submod);
            }
            if (!submod && (ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_clb;
            }
        }
        if (submod) {
            /* check that we have really a submodule */
            if (!submod->parsed->submodule) {
                /* submodule is not a submodule */
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Included \"%s\" schema from \"%s\" is actually not a submodule.",
                       inc->name, mod->name);
                lys_module_free(submod, NULL);
                /* fix list of modules in context, since it was already changed */
                --ctx->list.count;
                return LY_EVALID;
            }
            /* check that the submodule belongs-to our module */
            if (strcmp(mod->name, submod->parsed->belongsto)) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Included \"%s\" submodule from \"%s\" belongs-to a different module \"%s\".",
                       inc->name, mod->name, submod->parsed->belongsto);
                lys_module_free(submod, NULL);
                return LY_EVALID;
            }
            /* check circular dependency */
            if (submod->parsed->parsing) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "A circular dependency (include) for module \"%s\".",
                       submod->parsed->name);
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
        if (ly_errcode(ctx) != LY_EVALID) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Invalid value \"%s\" of include statement.", inc->name);
        } else {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Including \"%s\" submodule into \"%s\" failed.", inc->name, mod->name);
        }
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

#define FIND_MODULE(TYPE, MOD) \
    TYPE *imp; \
    if (!strncmp((MOD)->prefix, prefix, len) && (MOD)->prefix[len] == '\0') { \
        /* it is the prefix of the module itself */ \
        return (struct lys_module*)ly_ctx_get_module((MOD)->ctx, (MOD)->name, (MOD)->revs ? (MOD)->revs[0].date : NULL); \
    } \
    /* search in imports */ \
    LY_ARRAY_FOR((MOD)->imports, TYPE, imp) { \
        if (!strncmp(imp->prefix, prefix, len) && (MOD)->prefix[len] == '\0') { \
            return imp->module; \
        } \
    } \
    return NULL

struct lys_module *
lysc_module_find_prefix(struct lysc_module *mod, const char *prefix, size_t len)
{
    FIND_MODULE(struct lysc_import, mod);
}

struct lys_module *
lysp_module_find_prefix(struct lysp_module *mod, const char *prefix, size_t len)
{
    FIND_MODULE(struct lysp_import, mod);
}

struct lys_module *
lys_module_find_prefix(struct lys_module *mod, const char *prefix, size_t len)
{
    if (mod->compiled) {
        FIND_MODULE(struct lysc_import, mod->compiled);
    } else {
        FIND_MODULE(struct lysp_import, mod->parsed);
    }
}
