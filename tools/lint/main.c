/**
 * @file main.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool
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
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/times.h>
#include <string.h>

#include "commands.h"
#include "completion.h"
#include "../../linenoise/linenoise.h"
#include "../../src/libyang.h"

int done;
struct ly_ctx *ctx = NULL;

int
main(void)
{
    char *cmd, *cmdline, *cmdstart;
    int i, j;

    linenoiseSetCompletionCallback(complete_cmd);
    ctx = ly_ctx_new(NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return 1;
    }

    while (!done) {
        /* get the command from user */
        cmdline = linenoise(PROMPT);

        /* EOF -> exit */
        if (cmdline == NULL) {
            done = 1;
            cmdline = strdup("quit");
        }

        /* empty line -> wait for another command */
        if (*cmdline == '\0') {
            free(cmdline);
            continue;
        }

        /* isolate the command word. */
        for (i = 0; cmdline[i] && (cmdline[i] == ' '); i++);
        cmdstart = cmdline + i;
        for (j = 0; cmdline[i] && (cmdline[i] != ' '); i++, j++);
        cmd = strndup(cmdstart, j);

        /* parse the command line */
        for (i = 0; commands[i].name; i++) {
            if (strcmp(cmd, commands[i].name) == 0) {
                break;
            }
        }

        /* execute the command if any valid specified */
        if (commands[i].name) {
            /* display help */
            if ((strchr(cmdstart, ' ') != NULL) && ((strncmp(strchr(cmdstart, ' ')+1, "-h", 2) == 0)
                    || (strncmp(strchr(cmdstart, ' ')+1, "--help", 6) == 0))) {
                if (commands[i].help_func != NULL) {
                    commands[i].help_func();
                } else {
                    printf("%s\n", commands[i].helpstring);
                }
            } else {
                commands[i].func((const char *)cmdstart);
            }
        } else {
            /* if unknown command specified, tell it to user */
            fprintf(stderr, "%s: no such command, type 'help' for more information.\n", cmd);
        }
        linenoiseHistoryAdd(cmdline);

        free(cmd);
        free(cmdline);
    }

    ly_ctx_destroy(ctx, NULL);

    return 0;
}
