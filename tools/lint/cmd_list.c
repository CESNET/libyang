/**
 * @file cmd_list.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'list' command of the libyang's yanglint tool.
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

#include "cmd.h"

#include <getopt.h>
#include <stdio.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

void
cmd_list_help(void)
{
    printf("Usage: list [-f (xml | json)]\n"
            "                  Print the list of modules in the current context\n\n"
            "  -f FORMAT, --format=FORMAT\n"
            "                  Print the list as ietf-yang-library data in the specified\n"
            "                  data FORMAT. If format not specified, a simple list is\n"
            "                  printed with an indication of imported (i) / implemented (I)\n"
            "                  modules.\n");
}

int
cmd_list_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"format", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    yo->data_out_format = LYD_UNKNOWN;

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_LIST].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'f': /* --format */
            if (!strcasecmp(optarg, "xml")) {
                yo->data_out_format = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                yo->data_out_format = LYD_JSON;
            } else {
                YLMSG_E("Unknown output format %s\n", optarg);
                cmd_list_help();
                return 1;
            }
            break;
        case 'h':
            cmd_list_help();
            return 1;
        default:
            YLMSG_E("Unknown option.\n");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return 0;
}

int
cmd_list_dep(struct yl_opt *yo, int posc)
{
    if (posc) {
        YLMSG_E("No positional arguments are allowed.\n");
        return 1;
    }
    if (ly_out_new_file(stdout, &yo->out)) {
        YLMSG_E("Unable to print to the standard output.\n");
        return 1;
    }
    yo->out_stdout = 1;

    return 0;
}

int
cmd_list_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) posv;
    return print_list(yo->out, *ctx, yo->data_out_format);
}
