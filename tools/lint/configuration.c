/**
 * @file configuration.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yanglint configuration
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(_WINDOWS)
  #include <Shlobj.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <pwd.h>
#endif

#include "common.h"
#include "configuration.h"
#include "../../linenoise/linenoise.h"

/* Yanglint home (appended to ~/) */
#define YL_DIR ".yanglint"

char *
get_yanglint_dir(void)
{
    int ret;
    char *user_home, *yl_dir;

#if defined(_WINDOWS)
    PWSTR wPath = NULL;

     // Retrieve the LocalAppData standard folder's path
    if (SHGetKnownFolderPath(&FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &wPath) != S_OK) {

      LPTSTR errorMessage = NULL;
      getLastWINAPIErrorMessage(errorMessage);
      fprintf(stderr, "Determining home directory failed. %s", errorMessage);
      LocalFree(errorMessage);

      return NULL;
    }
    // Convert Unicode path to UTF8
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wPath, -1, NULL, 0, NULL, NULL);
    user_home = malloc(utf8Size);
    WideCharToMultiByte(CP_UTF8, 0, wPath, -1, user_home, utf8Size, NULL, NULL);
    CoTaskMemFree(wPath);
#else
    struct passwd *pw;
    if (!(pw = getpwuid(getuid()))) {
        fprintf(stderr, "Determining home directory failed (%s).\n", strerror(errno));
        return NULL;
    }
    user_home = pw->pw_dir;
#endif

    yl_dir = malloc(strlen(user_home) + 1 + strlen(YL_DIR) + 1);
    if (!yl_dir) {
        fprintf(stderr, "Memory allocation failed (%s).\n", strerror(errno));
        return NULL;
    }

#if defined(_WINDOWS)
      sprintf(yl_dir, "%s\\%s", user_home, YL_DIR);
      free(user_home);
#else
      sprintf(yl_dir, "%s/%s", user_home, YL_DIR);
#endif

    ret = access(yl_dir, R_OK | X_OK);

    if (ret == -1) {
        if (errno == ENOENT) {
            /* directory does not exist */
            fprintf(stdout, "Configuration directory \"%s\" does not exist, creating it.\n", yl_dir);
            if (mkdir(yl_dir, 00700)) {
                fprintf(stderr, "Configuration directory \"%s\" cannot be created (%s).\n", yl_dir, strerror(errno));
                free(yl_dir);
                return NULL;
            }
        } else {
            fprintf(stderr, "Configuration directory \"%s\" exists but cannot be accessed (%s).\n", yl_dir, strerror(errno));
            free(yl_dir);
            return NULL;
        }
    }

    return yl_dir;
}

void
load_config(void)
{
    char *yl_dir, *history_file;
    if ((yl_dir = get_yanglint_dir()) == NULL) {
        return;
    }

    history_file = malloc(strlen(yl_dir) + 9);
    if (!history_file) {
        fprintf(stderr, "Memory allocation failed (%s).\n", strerror(errno));
        free(yl_dir);
        return;
    }

    sprintf(history_file, "%s/history", yl_dir);
    if (access(history_file, F_OK) && (errno == ENOENT)) {
        fprintf(stdout, "No saved history.\n");
    } else if (linenoiseHistoryLoad(history_file)) {
        fprintf(stderr, "Failed to load history.\n");
    }

    free(history_file);
    free(yl_dir);
}

void
store_config(void)
{
    char *yl_dir, *history_file;

    if ((yl_dir = get_yanglint_dir()) == NULL) {
        return;
    }

    history_file = malloc(strlen(yl_dir) + 9);
    if (!history_file) {
        fprintf(stderr, "Memory allocation failed (%s).\n", strerror(errno));
        free(yl_dir);
        return;
    }

    sprintf(history_file, "%s/history", yl_dir);
    if (linenoiseHistorySave(history_file)) {
        fprintf(stderr, "Failed to save history.\n");
    }

    free(history_file);
    free(yl_dir);
}
