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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "context.h"
#include "libyang.h"
#include "tree_internal.h"

void
lyp_set_implemented(struct ly_module *module)
{
    int i;

    module->implemented = 1;

    for (i = 0; i < module->inc_size; i++) {
        lyp_set_implemented((struct ly_module *)module->inc[i].submodule);
    }
}

struct ly_module *
lyp_search_file(struct ly_ctx *ctx, struct ly_module *module, const char *name, const char *revision)
{
    size_t len, flen;
    int fd;
    char *cwd;
    DIR *dir;
    struct dirent *file;
    LYS_INFORMAT format;
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
            result = (struct ly_module *)ly_submodule_read_fd(module, fd, format, module->implemented);
        } else {
            result = lys_read_import(ctx, fd, format);
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
