/**
 * @file main.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool
 *
 * Copyright (c) 2015-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L /* strdup */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "cmd.h"
#include "common.h"
#include "completion.h"
#include "configuration.h"
#include "linenoise/linenoise.h"
#include "linenoise/utf8.h"
#include "yl_opt.h"

int done;
struct ly_ctx *ctx = NULL;

/* main_ni.c */
int main_ni(int argc, char *argv[]);

/**
 * @brief Find command and execute.
 *
 * @param[in] comm Array of commands.
 * @param[in] cmdline Input from command line.
 * @param[in] cmdlen Length of @p cmdline.
 * @param[in,out] yo Context for yanglint.
 * return 1 if command was found and execute.
 */
static ly_bool
execute_command(COMMAND *comm, char *cmdline, size_t cmdlen, struct yl_opt *yo)
{
    int posc = 0, i, j;
    char *empty = NULL;
    char **posv = &empty;
    ly_bool cmd_found = 0;

    /* execute the command if any valid specified */
    for (i = 0; comm[i].name; i++) {
        if (strncmp(cmdline, comm[i].name, cmdlen) || (comm[i].name[cmdlen] != '\0')) {
            continue;
        }

        cmd_found = 1;
        if (comm[i].opt_func && comm[i].opt_func(yo, cmdline, &posv, &posc)) {
            break;
        }
        if (comm[i].dep_func && comm[i].dep_func(yo, posc)) {
            break;
        }
        if (posc) {
            for (j = 0; j < posc; j++) {
                yo->last_one = (j + 1) == posc;
                if (comm[i].exec_func(&ctx, yo, posv[j])) {
                    break;
                }
            }
        } else {
            comm[i].exec_func(&ctx, yo, NULL);
        }
        if (comm[i].fin_func) {
            comm[i].fin_func(ctx, yo);
        }

        break;
    }

    return cmd_found;
}

int
main(int argc, char *argv[])
{
    int cmdlen;
    struct yl_opt yo = {0};
    char *cmdline;
    uint8_t cmd_found;

    if (argc > 1) {
        /* run in non-interactive mode */
        return main_ni(argc, argv);
    }
    yo.interactive = 1;

    /* continue in interactive mode */
    linenoiseSetCompletionCallback(complete_cmd);
    linenoiseSetEncodingFunctions(linenoiseUtf8PrevCharLen, linenoiseUtf8NextCharLen, linenoiseUtf8ReadCode);
    linenoiseSetMultiLine(1);
    load_config();

    if (ly_ctx_new(NULL, YL_DEFAULT_CTX_OPTIONS, &ctx)) {
        YLMSG_E("Failed to create context.");
        return 1;
    }

    while (!done) {
        cmd_found = 0;

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
        for (cmdlen = 0; cmdline[cmdlen] && (cmdline[cmdlen] != ' '); cmdlen++) {}

        /* execute interactive or non-interactive command */
        cmd_found = execute_command(commands, cmdline, cmdlen, &yo);
        if (!cmd_found) {
            /* try if command is in 'interactive_cmd' that has extra commands only for interactive mode */
            cmd_found = execute_command(interactive_cmd, cmdline, cmdlen, &yo);
        }

        if (!cmd_found) {
            /* if unknown command specified, tell it to user */
            YLMSG_E("Unknown command \"%.*s\", type 'help' for more information.", cmdlen, cmdline);
        }

        linenoiseHistoryAdd(cmdline);
        free(cmdline);
        yl_opt_erase(&yo);
    }

    /* Global variables in commands are freed. */
    cmd_free();

    store_config();
    ly_ctx_destroy(ctx);

    return 0;
}
