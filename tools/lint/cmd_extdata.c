/**
 * @file cmd_extdata.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'extdata' command of the libyang's yanglint tool.
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

#include "cmd.h"

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>

#include "libyang.h"

#include "common.h"

char *filename;

void
cmd_extdata_free(void)
{
    free(filename);
    filename = NULL;
}

void
cmd_extdata_help(void)
{
    printf("Usage: extdata [--clear] [<extdata-file-path>]\n"
            "                 File containing the specific data required by an extension. Required by\n"
            "                 the schema-mount extension, for example, when the operational data are\n"
            "                 expected in the file. File format is guessed.\n");
}

void
cmd_extdata(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"clear", no_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    int8_t cleared = 0;
    int8_t file_count = 0;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, commands[CMD_EXTDATA].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'c':
            ly_ctx_set_ext_data_clb(*ctx, NULL, NULL);
            free(filename);
            filename = NULL;
            cleared = 1;
            break;
        case 'h':
            cmd_extdata_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    file_count = argc - 1 - cleared;

    if (!cleared && (file_count == 0)) {
        /* no argument - print the current file */
        printf("%s\n", filename ? filename : "No file set.");
    } else if (file_count == 1) {
        /* set callback providing run-time extension instance data */
        free(filename);
        filename = strdup(argv[optind]);
        if (!filename) {
            YLMSG_E("Memory allocation error.\n");
            goto cleanup;
        }
        ly_ctx_set_ext_data_clb(*ctx, ext_data_clb, filename);
    } else if (!cleared) {
        YLMSG_E("Only one file must be entered.\n");
    }

cleanup:
    free_cmdline(argv);
}
