/**
 * @file completion.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool auto completion
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "commands.h"
#include "../../linenoise/linenoise.h"
#include "../../src/libyang.h"

extern struct ly_ctx *ctx;

static void
get_cmd_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    int i;

    *match_count = 0;
    *matches = NULL;

    for (i = 0; commands[i].name; i++) {
        if (!strncmp(hint, commands[i].name, strlen(hint))) {
            ++(*match_count);
            *matches = realloc(*matches, *match_count * sizeof **matches);
            (*matches)[*match_count-1] = strdup(commands[i].name);
        }
    }
}

static void
get_path_multiple_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *ptr, *path;
    DIR *dir;
    struct dirent *ent;

    *match_count = 0;
    *matches = NULL;

    ptr = strrchr(hint, ' ');
    while (*ptr == ' ') {
        ++ptr;
    }

    path = ptr;
    ptr = strrchr(path, '/');

    /* new relative path */
    if (ptr == NULL) {
        ptr = path;
        dir = opendir(".");
    } else {
        char buf[FILENAME_MAX];

        ++ptr;
        sprintf(buf, "%.*s", (int)(ptr - path), path);

        dir = opendir(buf);
    }

    if (dir == NULL) {
        fprintf(stderr, "opendir failed (%s)\n", strerror(errno));
        return;
    }

    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.') {
            continue;
        }

        /* some serious pointer fun */
        if (!strncmp(ptr, ent->d_name, strlen(ptr))) {
            ++(*match_count);
            *matches = realloc(*matches, *match_count * sizeof **matches);
            //asprintf(&(*matches)[*match_count-1], "%.*s%s", (int)(ptr-hint), hint, ent->d_name);
            (*matches)[*match_count-1] = malloc((ptr-hint)+strlen(ent->d_name)+1);
            strncpy((*matches)[*match_count-1], hint, ptr-hint);
            strcpy((*matches)[*match_count-1]+(ptr-hint), ent->d_name);
        }
    }

    closedir(dir);
}

static void
get_path_skip_opts_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *ptr, *path;
    DIR *dir;
    struct dirent *ent;

    *match_count = 0;
    *matches = NULL;

    ptr = strchr(hint, ' ');
    while (*ptr == ' ') {
        ++ptr;
    }

    /* options - skip them */
    while (*ptr == '-') {
        ptr = strchr(ptr, ' ');
        /* option is last - no hint */
        if (!ptr) {
            return;
        }
        while (*ptr == ' ') {
            ++ptr;
        }

        if (ptr[0] == '\'') {
            ptr = strchr(ptr + 1, '\'');
            if (ptr) {
                ++ptr;
            }
        } else if (ptr[0] == '"') {
            ptr = strchr(ptr + 1, '"');
            if (ptr) {
                ++ptr;
            }
        } else {
            ptr = strchr(ptr, ' ');
        }
        /* option argument is last - no hint */
        if (!ptr || !ptr[0]) {
            return;
        }
        while (*ptr == ' ') {
            ++ptr;
        }
    };

    path = ptr;
    ptr = strrchr(path, '/');

    /* new relative path */
    if (ptr == NULL) {
        ptr = path;
        dir = opendir(".");
    } else {
        char buf[FILENAME_MAX];

        ++ptr;
        sprintf(buf, "%.*s", (int)(ptr - path), path);

        dir = opendir(buf);
    }

    if (dir == NULL) {
        fprintf(stderr, "opendir failed (%s)\n", strerror(errno));
        return;
    }

    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.') {
            continue;
        }

        /* some serious pointer fun */
        if (!strncmp(ptr, ent->d_name, strlen(ptr))) {
            ++(*match_count);
            *matches = realloc(*matches, *match_count * sizeof **matches);
            //asprintf(&(*matches)[*match_count-1], "%.*s%s", (int)(ptr-hint), hint, ent->d_name);
            (*matches)[*match_count-1] = malloc((ptr-hint)+strlen(ent->d_name)+1);
            strncpy((*matches)[*match_count-1], hint, ptr-hint);
            strcpy((*matches)[*match_count-1]+(ptr-hint), ent->d_name);
        }
    }

    closedir(dir);
}

static void
get_model_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    int i, j, no_arg;
    const char *ptr;
    const char **names, **sub_names;

    *match_count = 0;
    *matches = NULL;

    /* skip the command */
    ptr = strchr(hint, ' ');
    while (*ptr == ' ') {
        ++ptr;
    }

    /* options - skip them */
    while (*ptr == '-') {
        /* -p, --print do not have an argument */
        if (!strncmp(ptr, "-p", 2) || !strncmp(ptr, "--print", 7)) {
            no_arg = 1;
        } else {
            no_arg = 0;
        }

        ptr = strchr(ptr, ' ');
        /* option is last - no hint */
        if (!ptr) {
            return;
        }
        while (*ptr == ' ') {
            ++ptr;
        }

        if (no_arg) {
            continue;
        }

        ptr = strchr(ptr, ' ');
        /* option argument is last - no hint */
        if (!ptr) {
            return;
        }
        while (*ptr == ' ') {
            ++ptr;
        }
    };

    /* now ptr points to the model name hint, can be just "" */
    names = ly_ctx_get_module_names(ctx);
    for (i = 0; names[i]; ++i) {
        if (!strncmp(ptr, names[i], strlen(ptr))) {
            ++(*match_count);
            *matches = realloc(*matches, *match_count * sizeof **matches);
            (*matches)[*match_count-1] = malloc((ptr-hint)+strlen(names[i])+1);
            strncpy((*matches)[*match_count-1], hint, ptr-hint);
            strcpy((*matches)[*match_count-1]+(ptr-hint), names[i]);
        }

        sub_names = ly_ctx_get_submodule_names(ctx, names[i]);
        for (j = 0; sub_names[j]; ++j) {
            if (!strncmp(ptr, sub_names[j], strlen(ptr))) {
                ++(*match_count);
                *matches = realloc(*matches, *match_count * sizeof **matches);
                (*matches)[*match_count-1] = malloc((ptr-hint)+strlen(sub_names[j])+1);
                strncpy((*matches)[*match_count-1], hint, ptr-hint);
                strcpy((*matches)[*match_count-1]+(ptr-hint), sub_names[j]);
            }
        }
        free(sub_names);
    }
    free(names);
}

void
complete_cmd(const char *buf, linenoiseCompletions *lc)
{
    char **matches = NULL;
    unsigned int match_count = 0, i;

    if (!strncmp(buf, "add ", 4)) {
        get_path_multiple_completion(buf, &matches, &match_count);
    } else if (!strncmp(buf, "searchpath ", 11) || !strncmp(buf, "data ", 5)
            || !strncmp(buf, "config ", 7) || !strncmp(buf, "filter ", 7)
            || !strncmp(buf, "xpath ", 6)) {
        get_path_skip_opts_completion(buf, &matches, &match_count);
    } else if (!strncmp(buf, "print ", 6) || !strncmp(buf, "feature ", 8)) {
        get_model_completion(buf, &matches, &match_count);
    } else {
        get_cmd_completion(buf, &matches, &match_count);
    }

    for (i = 0; i < match_count; ++i) {
        linenoiseAddCompletion(lc, matches[i]);
        free(matches[i]);
    }
    free(matches);
}
