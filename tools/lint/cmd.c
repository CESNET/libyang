/**
 * @file cmd.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool general commands
 *
 * Copyright (c) 2015-2020 CESNET, z.s.p.o.
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "common.h"
#include "compat.h"
#include "libyang.h"

COMMAND commands[];
extern int done;

#ifndef NDEBUG

void
cmd_debug_help(void)
{
    printf("Usage: debug (dict | xpath)+\n");
}

void
cmd_debug(struct ly_ctx **UNUSED(ctx), const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    uint32_t dbg_groups = 0;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, commands[CMD_DEBUG].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_debug_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }
    if (argc == optind) {
        /* no argument */
        cmd_debug_help();
        goto cleanup;
    }

    for (int i = 0; i < argc - optind; i++) {
        if (!strcasecmp("dict", argv[optind + i])) {
            dbg_groups |= LY_LDGDICT;
        } else if (!strcasecmp("xpath", argv[optind + i])) {
            dbg_groups |= LY_LDGXPATH;
        } else {
            YLMSG_E("Unknown debug group \"%s\"\n", argv[optind + 1]);
            goto cleanup;
        }
    }

    ly_log_dbg_groups(dbg_groups);

cleanup:
    free_cmdline(argv);
}

#endif

void
cmd_verb_help(void)
{
    printf("Usage: verb (error | warning | verbose | debug)\n");
}

void
cmd_verb(struct ly_ctx **UNUSED(ctx), const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, commands[CMD_VERB].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_verb_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (argc - optind > 1) {
        YLMSG_E("Only a single verbosity level can be set.\n");
        cmd_verb_help();
        goto cleanup;
    } else if (argc == optind) {
        /* no argument - print current value */
        LY_LOG_LEVEL level = ly_log_level(LY_LLERR);

        ly_log_level(level);
        printf("Current verbosity level: ");
        if (level == LY_LLERR) {
            printf("error\n");
        } else if (level == LY_LLWRN) {
            printf("warning\n");
        } else if (level == LY_LLVRB) {
            printf("verbose\n");
        } else if (level == LY_LLDBG) {
            printf("debug\n");
        }
        goto cleanup;
    }

    if (!strcasecmp("error", argv[optind]) || !strcmp("0", argv[optind])) {
        ly_log_level(LY_LLERR);
    } else if (!strcasecmp("warning", argv[optind]) || !strcmp("1", argv[optind])) {
        ly_log_level(LY_LLWRN);
    } else if (!strcasecmp("verbose", argv[optind]) || !strcmp("2", argv[optind])) {
        ly_log_level(LY_LLVRB);
    } else if (!strcasecmp("debug", argv[optind]) || !strcmp("3", argv[optind])) {
        ly_log_level(LY_LLDBG);
    } else {
        YLMSG_E("Unknown verbosity \"%s\"\n", argv[optind]);
        goto cleanup;
    }

cleanup:
    free_cmdline(argv);
}

void
cmd_quit(struct ly_ctx **UNUSED(ctx), const char *UNUSED(cmdline))
{
    done = 1;
    return;
}

void
cmd_help_help(void)
{
    printf("Usage: help [cmd ...]\n");
}

void
cmd_help(struct ly_ctx **UNUSED(ctx), const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, commands[CMD_HELP].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_help_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (argc == optind) {
generic_help:
        printf("Available commands:\n");
        for (uint16_t i = 0; commands[i].name; i++) {
            if (commands[i].helpstring != NULL) {
                printf("  %-15s %s\n", commands[i].name, commands[i].helpstring);
            }
        }
    } else {
        /* print specific help for the selected command(s) */

        for (int c = 0; c < argc - optind; ++c) {
            int8_t match = 0;

            /* get the command of the specified name */
            for (uint16_t i = 0; commands[i].name; i++) {
                if (strcmp(argv[optind + c], commands[i].name) == 0) {
                    match = 1;
                    if (commands[i].help_func != NULL) {
                        commands[i].help_func();
                    } else {
                        printf("%s: %s\n", argv[optind + c], commands[i].helpstring);
                    }
                    break;
                }
            }
            if (!match) {
                /* if unknown command specified, print the list of commands */
                printf("Unknown command \'%s\'\n", argv[optind + c]);
                goto generic_help;
            }
        }
    }

cleanup:
    free_cmdline(argv);
}

/* Also keep enum COMMAND_INDEX updated. */
COMMAND commands[] = {
    {"help", cmd_help, cmd_help_help, "Display commands description", "h"},
    {"add", cmd_add, cmd_add_help, "Add a new module from a specific file", "DF:hi"},
    {"load", cmd_load, cmd_load_help, "Load a new schema from the searchdirs", "F:hi"},
    {"print", cmd_print, cmd_print_help, "Print a module", "f:hL:o:P:q"},
    {"data", cmd_data, cmd_data_help, "Load, validate and optionally print instance data", "d:ef:F:hmo:O:r:nt:x:"},
    {"list", cmd_list, cmd_list_help, "List all the loaded modules", "f:h"},
    {"feature", cmd_feature, cmd_feature_help, "Print all features of module(s) with their state", "haf"},
    {"searchpath", cmd_searchpath, cmd_searchpath_help, "Print/set the search path(s) for schemas", "ch"},
    {"clear", cmd_clear, cmd_clear_help, "Clear the context - remove all the loaded modules", "iyh"},
    {"verb", cmd_verb, cmd_verb_help, "Change verbosity", "h"},
#ifndef NDEBUG
    {"debug", cmd_debug, cmd_debug_help, "Display specific debug message groups", "h"},
#endif
    {"quit", cmd_quit, NULL, "Quit the program", "h"},
    /* synonyms for previous commands */
    {"?", cmd_help, NULL, "Display commands description", "h"},
    {"exit", cmd_quit, NULL, "Quit the program", "h"},
    {NULL, NULL, NULL, NULL, NULL}
};
