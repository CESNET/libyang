/**
 * @file parser.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common libyang parsers routines implementations
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
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "context.h"
#include "libyang.h"
#include "parser.h"
#include "resolve.h"
#include "tree_internal.h"

void
lyp_set_implemented(struct lys_module *module)
{
    int i;

    module->implemented = 1;

    for (i = 0; i < module->inc_size; i++) {
        lyp_set_implemented((struct lys_module *)module->inc[i].submodule);
    }
}

/*
 * (temporary) alternative for lys_read() + lys_parse() in case of import
 */
struct lys_module *
lys_read_import(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    struct unres_schema *unres;
    struct lys_module *module;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    unres = calloc(1, sizeof *unres);

    /*
     * TODO
     * This is just a temporary solution to make working automatic search for
     * imported modules. This doesn't work e.g. for streams (stdin)
     */
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    switch (format) {
    case LYS_IN_YIN:
        module = yin_read_module(ctx, addr, 0, unres);
        break;
    case LYS_IN_YANG:
    default:
        /* TODO */
        munmap(addr, sb.st_size);
        free(unres);
        return NULL;
    }
    munmap(addr, sb.st_size);

    if (resolve_unres(module, unres)) {
        lys_free(module);
        module = NULL;
    }
    free(unres->item);
    free(unres->type);
    free(unres->str_snode);
    free(unres->line);
    free(unres);

    return module;
}

struct lys_module *
lyp_search_file(struct ly_ctx *ctx, struct lys_module *module, const char *name, const char *revision)
{
    size_t len, flen;
    int fd;
    char *cwd, *model_path;
    DIR *dir;
    struct dirent *file;
    LYS_INFORMAT format;
    struct lys_module *result = NULL;
    int localsearch = 1;

    len = strlen(name);
    cwd = get_current_dir_name();

opendir_search:
    chdir(cwd);
    dir = opendir(cwd);
    LOGVRB("Searching for \"%s\" in %s.", name, cwd);
    if (!dir) {
        LOGWRN("Unable to open directory \"%s\" for searching referenced modules (%s)",
               cwd, strerror(errno));
        /* try search directory */
        goto searchpath;
    }

    while ((file = readdir(dir))) {
        if (strncmp(name, file->d_name, len) ||
                (file->d_name[len] != '.' && file->d_name[len] != '@')) {
            continue;
        }

        flen = strlen(file->d_name);
        if (revision && flen > len + 5) {
            /* check revision from the filename */
            /* TODO */
        }

        /* get type according to filename suffix */
        if (!strcmp(&file->d_name[flen - 4], ".yin")) {
            format = LYS_IN_YIN;
        } else if (!strcmp(&file->d_name[flen - 5], ".yang")) {
            format = LYS_IN_YANG;
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
            result = (struct lys_module *)lys_submodule_read(module, fd, format, module->implemented);
        } else {
            result = lys_read_import(ctx, fd, format);
        }
        close(fd);

        if (result) {
            asprintf(&model_path, "file://%s/%s", cwd, file->d_name);
            result->uri = lydict_insert(ctx, model_path, 0);
            free(model_path);
            break;
        }
    }

searchpath:
    if (!ctx->models.search_path) {
        LOGWRN("No search path defined for the current context.");
    } else if (!result && localsearch) {
        /* search in local directory done, try context's search_path */
        closedir(dir);
        cwd = strdup(ctx->models.search_path);
        localsearch = 0;
        goto opendir_search;
    }

cleanup:
    chdir(cwd);
    free(cwd);
    closedir(dir);

    return result;
}
