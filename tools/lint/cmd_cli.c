/**
 * @file cli_verb.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'cli' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2024-2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "cmd.h"

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>

#include "libyang.h"
#include "linenoise/linenoise.h"

#include "common.h"
#include "yl_opt.h"

#define LY_CLI_SINGLELINE 0x1

uint8_t cli_flags;

void
cmd_cli_help(void)
{
    printf("Usage: cli [-s]\n"
            "                  Settings for the command line interface.\n\n"
            "  -s, --toggle-singleline\n"
            "                  The singleline settings toggles how the cli handles the overflow of typed text\n"
            "                  across the screen. By default, it puts overflow text on a new line. But this\n"
            "                  setting will make the text will scroll towards right to make room while\n"
            "                  sending much less ANSI escape codes.\n");
}

int
cmd_cli_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"toggle-singleline", no_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, interactive_cmd[CMD_CLI].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 's':
            yo->cli_flags |= LY_CLI_SINGLELINE;
            break;
        case 'h':
            cmd_cli_help();
            return 1;
        default:
            YLMSG_E("Unknown option.");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return 0;
}

int
cmd_cli_dep(struct yl_opt *yo, int posc)
{
    (void) yo;

    if (posc != 0) {
        YLMSG_E("Only options are expected.");
        cmd_cli_help();
        return 1;
    }

    return 0;
}

int
cmd_cli_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) ctx, (void) posv;

    if (yo->cli_flags & LY_CLI_SINGLELINE) {
        if (cli_flags & LY_CLI_SINGLELINE) {
            cli_flags &= ~LY_CLI_SINGLELINE;
            linenoiseSetMultiLine(1);
        } else {
            cli_flags |= LY_CLI_SINGLELINE;
            linenoiseSetMultiLine(0);
        }
    }

    return 0;
}
