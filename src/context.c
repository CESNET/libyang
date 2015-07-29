/**
 * @file context.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief context implementation for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
#include "context.h"
#include "dict.h"

API struct ly_ctx *
ly_ctx_new(const char *search_dir)
{
    struct ly_ctx *ctx;
    char *cwd;

    ctx = calloc(1, sizeof *ctx);
    if (!ctx) {
        LOGMEM;
        return NULL;
    }

    /* dictionary */
    lydict_init(&ctx->dict);

    /* models list */
    ctx->models.list = calloc(16, sizeof *ctx->models.list);
    ctx->models.used = 0;
    ctx->models.size = 16;
    if (search_dir) {
        cwd = get_current_dir_name();
        if (chdir(search_dir)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            free(cwd);
            ly_ctx_destroy(ctx);
            return NULL;
        }
        ctx->models.search_path = get_current_dir_name();
        chdir(cwd);
        free(cwd);
    }

    return ctx;
}

API void
ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir)
{
    char *cwd;

    if (!ctx) {
        return;
    }

    if (search_dir) {
        cwd = get_current_dir_name();
        if (chdir(search_dir)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            free(cwd);
            return;
        }
        ctx->models.search_path = get_current_dir_name();
        chdir(cwd);
        free(cwd);
    } else {
        free(ctx->models.search_path);
        ctx->models.search_path = NULL;
    }
}

API void
ly_ctx_destroy(struct ly_ctx *ctx)
{
    if (!ctx) {
        return;
    }

    /* models list */
    while (ctx->models.used) {
        ly_module_free(ctx->models.list[0]);
    }
    free(ctx->models.search_path);
    free(ctx->models.list);

    /* dictionary */
    lydict_clean(&ctx->dict);

    free(ctx);
}

API struct ly_submodule *
ly_ctx_get_submodule(struct ly_module *module, const char *name, const char *revision)
{
    struct ly_submodule *result;
    int i;

    if (!module || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    /* search in modules included by the main module */
    if (module->type) {
        module = ((struct ly_submodule *)module)->belongsto;
    }
    for (i = 0; i < module->inc_size; i++) {
        result = module->inc[i].submodule;
        if (strcmp(name, result->name)) {
            continue;
        }

        if (!revision || (result->rev_size && !strcmp(revision, result->rev[0].date))) {
            return result;
        }
    }

    return NULL;
}

API struct ly_module *
ly_ctx_get_module(struct ly_ctx *ctx, const char *name, const char *revision)
{
    int i;
    struct ly_module *result = NULL;

    if (!ctx || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    for (i = 0; i < ctx->models.used; i++) {
        result = ctx->models.list[i];
        if (!result || strcmp(name, result->name)) {
            continue;
        }

        if (!revision || (result->rev_size && !strcmp(revision, result->rev[0].date))) {
            return result;
        }
    }

    return NULL;
}

API char **
ly_ctx_get_module_names(struct ly_ctx *ctx)
{
    int i;
    char **result = NULL;

    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    result = malloc((ctx->models.used+1) * sizeof *result);

    for (i = 0; i < ctx->models.used; i++) {
        result[i] = strdup(ctx->models.list[i]->name);
    }
    result[i] = NULL;

    return result;
}

API char **
ly_ctx_get_submodule_names(struct ly_ctx *ctx, const char *name)
{
    int i;
    char **result = NULL;
    struct ly_module *mod;

    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    mod = ly_ctx_get_module(ctx, name, NULL);
    if (!mod) {
        LOGERR(LY_EVALID, "Data model \"%s\" not loaded", name);
        return NULL;
    }

    result = malloc((mod->inc_size+1) * sizeof *result);

    for (i = 0; i < mod->inc_size; i++) {
        result[i] = strdup(mod->inc[i].submodule->name);
    }
    result[i] = NULL;

    return result;
}
