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

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/times.h>
#include <string.h>

#include "commands.h"
#include "mreadline.h"

#include "../../src/libyang.h"

int done;
struct ly_ctx *ctx = NULL;
char *search_path;

void
usage(const char *progname)
{
    fprintf(stdout, "Usage: %s [[-h] [-v level] [-p dir] [-f format [-o file]] file.yin]\n\n", progname);
    fprintf(stdout, "  -h, --help             Print this text.\n");
    fprintf(stdout, "  -f, --format yang      Print data model in given format.\n");
    fprintf(stdout, "  -o, --output file      Print output to the specified file.\n");
    fprintf(stdout, "  -p, --path dir         Search path for data models.\n");
    fprintf(stdout, "  -v, --verbose level    Set verbosity level (0-3).\n");
    fprintf(stdout, "  file.yin               Input file in YIN format.\n\n");
    fprintf(stdout, "Executing without arguments starts an interactive version.\n\n");
}

int main_noninteractive(int argc, char *argv[])
{
    int c;
    int ret = EXIT_FAILURE;
    FILE *output = NULL;
    int fd = -1;
    struct ly_module *model;
    struct stat sb;
    char *addr = NULL;
    LY_MOUTFORMAT out_format = LY_OUT_YANG;

    int opt_i;
    struct option opt[] = {
            { "format",  required_argument, 0, 'f' },
            { "help",    no_argument,       0, 'h' },
            { "output",  required_argument, 0, 'o' },
            { "path",    required_argument, 0, 'p' },
            { "verbose", required_argument, 0, 'v' },
            { 0, 0, 0, 0 } };

    while ((c = getopt_long(argc, argv, "f:ho:p:v:", opt, &opt_i)) != -1) {
        switch (c) {
        case 'f':
            if (strcmp(optarg, "yang") && strcmp(optarg, "tree")) {
                fprintf(stderr, "Output format \"%s\" not supported.\n", optarg);
                goto cleanup;
            }
            if (strcmp(optarg, "tree") == 0) {
                out_format = LY_OUT_TREE;
            }

            if (!output) {
                output = stdout;
            }
            break;
        case 'h':
            usage(argv[0]);
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'o':
            output = fopen(optarg, "w");
            if (!output) {
                fprintf(stderr, "Unable to use output file (%s).\n", strerror(errno));
                goto cleanup;
            }
            break;
        case 'p':
            search_path = optarg;
            break;
        case 'v':
            ly_verb(atoi(optarg));
            break;
        default: /* '?' */
            usage(argv[0]);
            goto cleanup;
        }
    }

    if (optind != argc - 1) {
        usage(argv[0]);
        goto cleanup;
    }

    fd = open(argv[optind], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Opening input file failed (%s).\n", strerror(errno));
        goto cleanup;
    }
    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "Unable to get input file information (%s).\n", strerror(errno));
        goto cleanup;
    }
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);


    /* libyang */
    ctx = ly_ctx_new(search_path);
    model = ly_module_read(ctx, addr, LY_IN_YIN);
    if (!model) {
        fprintf(stderr, "Parsing data model failed.\n");
        goto cleanup;
    }

    if (output) {
        ly_model_print(output, model, out_format);
    }

    ret = EXIT_SUCCESS;

cleanup:
    ly_ctx_destroy(ctx);
    if (addr) {
        munmap(addr, sb.st_size);
    }
    if (fd != -1) {
        close(fd);
    }
    if (output && output != stdout) {
        fclose(output);
    }

    return ret;
}

int
main(int argc, char **argv)
{
    char *cmd, *cmdline, *cmdstart;
    int i, j;

    if (argc > 1) {
        return main_noninteractive(argc, argv);
    }

    initialize_readline();
    ctx = ly_ctx_new(search_path);

    while (!done) {
        /* get the command from user */
        cmdline = readline(PROMPT);

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
        for (i = 0; cmdline[i] && whitespace(cmdline[i]); i++);
        cmdstart = cmdline + i;
        for (j = 0; cmdline[i] && !whitespace(cmdline[i]); i++, j++);
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
                commands[i].func((const char*)cmdstart);
            }
        } else {
            /* if unknown command specified, tell it to user */
            fprintf(stderr, "%s: no such command, type 'help' for more information.\n", cmd);
        }
        add_history(cmdline);

        free(cmd);
        free(cmdline);
    }

    remove_all_hints();
    ly_ctx_destroy(ctx);
    free(search_path);

    return 0;
}
