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
#include "common.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "libyang.h"
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

struct lysp_load_module_check_data {
    const char *name;
    const char *revision;
    const char *path;
    const char* submoduleof;
};

static LY_ERR
lysp_load_module_check(struct ly_ctx *ctx, struct lysp_module *mod, void *data)
{
    struct lysp_load_module_check_data *info = data;
    const char *filename, *dot, *rev;
    size_t len;

    if (info->name) {
        /* check name of the parsed model */
        if (strcmp(info->name, mod->name)) {
            LOGERR(ctx, LY_EINVAL, "Unexpected module \"%s\" parsed instead of \"%s\").", mod->name, info->name);
            return LY_EINVAL;
        }
    }
    if (info->revision) {
        /* check revision of the parsed model */
        if (!mod->revs || strcmp(info->revision, mod->revs[0].date)) {
            LOGERR(ctx, LY_EINVAL, "Module \"%s\" parsed with the wrong revision (\"%s\" instead \"%s\").", mod->name,
                   mod->revs[0].date, info->revision);
            return LY_EINVAL;
        }
    }
    if (info->submoduleof) {
        /* check that we have really a submodule */
        if (!mod->submodule) {
            /* submodule is not a submodule */
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Included \"%s\" schema from \"%s\" is actually not a submodule.",
                   mod->name, info->submoduleof);
            return LY_EVALID;
        }
        /* check that the submodule belongs-to our module */
        if (strcmp(info->submoduleof, mod->belongsto)) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Included \"%s\" submodule from \"%s\" belongs-to a different module \"%s\".",
                   mod->name, info->submoduleof, mod->belongsto);
            return LY_EVALID;
        }
        /* check circular dependency */
        if (mod->parsing) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "A circular dependency (include) for module \"%s\".", mod->name);
            return LY_EVALID;
        }
    }
    if (info->path) {
        /* check that name and revision match filename */
        filename = strrchr(info->path, '/');
        if (!filename) {
            filename = info->path;
        } else {
            filename++;
        }
        /* name */
        len = strlen(mod->name);
        rev = strchr(filename, '@');
        dot = strrchr(info->path, '.');
        if (strncmp(filename, mod->name, len) ||
                ((rev && rev != &filename[len]) || (!rev && dot != &filename[len]))) {
            LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, mod->name);
        }
        /* revision */
        if (rev) {
            len = dot - ++rev;
            if (!mod->revs || len != 10 || strncmp(mod->revs[0].date, rev, len)) {
                LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                       mod->revs ? mod->revs[0].date : "none");
            }
        }
    }
    return LY_SUCCESS;
}

LY_ERR
lys_module_localfile(struct ly_ctx *ctx, const char *name, const char *revision, int implement,
                     struct lys_module **result)
{
    int fd;
    char *filepath = NULL;
    LYS_INFORMAT format;
    struct lys_module *mod = NULL;
    LY_ERR ret = LY_SUCCESS;
    struct lysp_load_module_check_data check_data = {0};

    LY_CHECK_RET(lys_search_localfile(ly_ctx_get_searchdirs(ctx), !(ctx->flags & LY_CTX_DISABLE_SEARCHDIR_CWD), name, revision,
                                      &filepath, &format));
    LY_CHECK_ERR_RET(!filepath, LOGERR(ctx, LY_ENOTFOUND, "Data model \"%s%s%s\" not found in local searchdirs.",
                                       name, revision ? "@" : "", revision ? revision : ""), LY_ENOTFOUND);


    LOGVRB("Loading schema from \"%s\" file.", filepath);

    /* open the file */
    fd = open(filepath, O_RDONLY);
    LY_CHECK_ERR_GOTO(fd < 0, LOGERR(ctx, LY_ESYS, "Unable to open data model file \"%s\" (%s).",
                                     filepath, strerror(errno)); ret = LY_ESYS, cleanup);

    check_data.name = name;
    check_data.revision = revision;
    check_data.path = filepath;
    mod = lys_parse_fd_(ctx, fd, format, implement,
                        lysp_load_module_check, &check_data);
    close(fd);
    LY_CHECK_ERR_GOTO(!mod, ly_errcode(ctx), cleanup);

    if (!mod->parsed->filepath) {
        char rpath[PATH_MAX];
        if (realpath(filepath, rpath) != NULL) {
            mod->parsed->filepath = lydict_insert(ctx, rpath, 0);
        } else {
            mod->parsed->filepath = lydict_insert(ctx, filepath, 0);
        }
    }

    *result = mod;

    /* success */
cleanup:
    free(filepath);
    return ret;
}

LY_ERR
lysp_load_module(struct ly_ctx *ctx, const char *name, const char *revision, int implement, int require_parsed, struct lys_module **mod)
{
    const char *module_data = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;
    void (*module_data_free)(void *module_data, void *user_data) = NULL;
    struct lysp_load_module_check_data check_data = {0};

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

        /* module not present in the context, get the input data and parse it */
        if (!(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
            if (ctx->imp_clb) {
                if (ctx->imp_clb(name, revision, NULL, NULL, ctx->imp_clb_data,
                                      &format, &module_data, &module_data_free) == LY_SUCCESS) {
                    check_data.name = name;
                    check_data.revision = revision;
                    *mod = lys_parse_mem_(ctx, module_data, format, implement,
                                          lysp_load_module_check, &check_data);
                    if (module_data_free) {
                        module_data_free((void*)module_data, ctx->imp_clb_data);
                    }
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

        if ((*mod) && !revision && ((*mod)->parsed->latest_revision == 1)) {
            /* update the latest_revision flag - here we have selected the latest available schema,
             * consider that even the callback provides correct latest revision */
            (*mod)->parsed->latest_revision = 2;
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
        if ((*mod)->parsed && (*mod)->parsed->parsing) {
            LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "A circular dependency (import) for module \"%s\".", name);
            *mod = NULL;
            return LY_EVALID;
        }
    }
    if (!(*mod)) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "%s \"%s\" module failed.", implement ? "Loading" : "Importing", name);
        return LY_EVALID;
    }

    if (implement) {
        /* mark the module implemented, check for collision was already done */
        lys_module_implement(*mod);
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
    struct lysp_load_module_check_data check_data = {0};

    /* Try to get submodule from the context, if already present */
    inc->submodule = ly_ctx_get_submodule(ctx, mod->name, inc->name, inc->rev[0] ? inc->rev : NULL);
    if (!inc->submodule || (!inc->rev[0] && inc->submodule->latest_revision != 2)) {
        /* submodule not present in the context, get the input data and parse it */
        if (!(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
            if (ctx->imp_clb) {
                if (ctx->imp_clb(mod->name, NULL, inc->name, inc->rev[0] ? inc->rev : NULL, ctx->imp_clb_data,
                                      &format, &submodule_data, &submodule_data_free) == LY_SUCCESS) {
                    check_data.name = inc->name;
                    check_data.revision = inc->rev[0] ? inc->rev : NULL;
                    check_data.submoduleof = mod->name;
                    submod = lys_parse_mem_(ctx, submodule_data, format, mod->implemented,
                                            lysp_load_module_check, &check_data);
                    if (submodule_data_free) {
                        submodule_data_free((void*)submodule_data, ctx->imp_clb_data);
                    }
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
            if (!inc->rev[0] && (submod->parsed->latest_revision == 1)) {
                /* update the latest_revision flag - here we have selected the latest available schema,
                 * consider that even the callback provides correct latest revision */
                submod->parsed->latest_revision = 2;
            }

            inc->submodule = submod->parsed;
            ++inc->submodule->refcount;
            free(submod);
        }
    } else {
        ++inc->submodule->refcount;
    }
    if (!inc->submodule) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Including \"%s\" submodule into \"%s\" failed.", inc->name, mod->name);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

#define FIND_MODULE(TYPE, MOD) \
    TYPE *imp; \
    if (!strncmp((MOD)->prefix, prefix, len) && (MOD)->prefix[len] == '\0') { \
        /* it is the prefix of the module itself */ \
        return (struct lys_module*)ly_ctx_get_module((MOD)->ctx, (MOD)->name, ((struct lysc_module*)(MOD))->revision); \
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
