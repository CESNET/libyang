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
#include <dirent.h>
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
ly_ctx_destroy(struct ly_ctx *ctx)
{
    int i;

    if (!ctx) {
        ly_errno = LY_EINVAL;
        return;
    }

    /* models list */
    if (ctx->models.used) {
        for (i = 0; i < ctx->models.size; i++) {
            if (ctx->models.list[i]) {
                ly_module_free(ctx->models.list[i]);
            }
        }
    }
    free(ctx->models.search_path);
    free(ctx->models.list);

    /* dictionary */
    lydict_clean(&ctx->dict);

    free(ctx);
}

static struct ly_module *
search_file(struct ly_ctx *ctx, struct ly_module *module, const char *name, const char *revision)
{
    size_t len, flen;
    int fd;
    char *cwd;
    DIR *dir;
    struct dirent *file;
    LY_MINFORMAT format;
    struct ly_module *result = NULL;
    int localsearch = 1;

    len = strlen(name);
    cwd = get_current_dir_name();
    dir = opendir(cwd);
    LOGVRB("Searching for \"%s\" in %s.", name, cwd);
    if (!dir) {
        LOGWRN("Unable to open local directory for searching referenced modules (%s)",
               strerror(errno));
        /* try search directory */
        goto searchpath;
    }

search:
    while ((file = readdir(dir))) {
        if (strncmp(name, file->d_name, len)) {
            continue;
        }

        flen = strlen(file->d_name);
        if (revision && flen > len + 5) {
            /* check revision from the filename */
            /* TODO */
        }

        /* get type according to filename suffix */
        if (!strcmp(&file->d_name[flen - 4], ".yin")) {
            format = LY_IN_YIN;
        } else if (!strcmp(&file->d_name[flen - 5], ".yang")) {
            format = LY_IN_YANG;
        } else {
            continue;
        }

        /* open the file */
        fd = open(file->d_name, O_RDONLY);
        if (fd < 0) {
            LOGERR(LY_ESYS, "Unable to open data model file \"%s\" (%s).",
                   file->d_name, strerror(errno));
            goto cleanup;
        }

        if (module) {
            result = (struct ly_module *)ly_submodule_read_fd(module, fd, format);
        } else {
            result = ly_module_read_fd(ctx, fd, format);
        }
        close(fd);

        if (result) {
            break;
        }
    }

searchpath:
    if (!ctx->models.search_path) {
        LOGWRN("No search path defined for the current context.");
    } else if (!result && localsearch) {
        /* search in local directory done, try context's search_path */
        closedir(dir);
        dir = opendir(ctx->models.search_path);
        if (!dir) {
            LOGERR(LY_ESYS, "Unable to open data model search directory \"%s\" (%s).",
                   ctx->models.search_path, strerror(errno));
            goto cleanup;
        }

        chdir(ctx->models.search_path);
        LOGVRB("Searching for \"%s\" in %s.", name, ctx->models.search_path);

        localsearch = 0;
        goto search;
    }

cleanup:
    chdir(cwd);
    free(cwd);
    closedir(dir);

    return result;
}

struct ly_submodule *
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

    /* not found in context, try to get it from the search directory */
    result = (struct ly_submodule *)search_file(module->ctx, module, name, revision);
    if (!result) {
        LOGERR(LY_EVALID, "Submodule \"%s\" of the \"%s\" data model not found (search path is \"%s\")",
               name, module->name, module->ctx->models.search_path);
    }

    return result;
}

API struct ly_module *
ly_ctx_get_module(struct ly_ctx *ctx, const char *name, const char *revision, int read)
{
    int i;
    struct ly_module *result = NULL;

    if (!ctx || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    for (i = 0; i < ctx->models.size; i++) {
        result = ctx->models.list[i];
        if (!result || strcmp(name, result->name)) {
            continue;
        }

        if (!revision || (result->rev_size && !strcmp(revision, result->rev[0].date))) {
            return result;
        }
    }

    if (!read) {
        return result;
    }

    /* not found in context, try to get it from the search directory */
    result = search_file(ctx, NULL, name, revision);
    if (!result) {
        LOGERR(LY_EVALID, "Data model \"%s\" not found (search path is \"%s\")", name, ctx->models.search_path);
    }

    return result;
}
