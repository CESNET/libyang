/*
 * commands.c
 * Author Michal Vasko <mvasko@cesnet.cz>
 *
 * Implementation of dev-datastore commands.
 *
 * Copyright (C) 2014 CESNET, z.s.p.o.
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
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is, and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */
#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include "../../src/libyang.h"
#include "../../src/tree.h"

#include "commands.h"
#include "mreadline.h"

struct model_hint *model_hints, *model_hints_end;

extern int done;
extern struct ly_ctx *ctx;
extern char *search_path;

void
add_hint(const char *name)
{
    if (model_hints == NULL) {
        model_hints = malloc(sizeof(struct model_hint));
        model_hints_end = model_hints;
        model_hints_end->hint = strdup(name);
        model_hints_end->next = NULL;
    } else {
        model_hints_end->next = malloc(sizeof(struct model_hint));
        model_hints_end->next->hint = strdup(name);
        model_hints_end->next->next = NULL;
        model_hints_end = model_hints_end->next;
    }
}

void
remove_hint(const char *name)
{
    struct model_hint *prev, *cur;

    if (model_hints == NULL) {
        return;
    }

    if (strcmp(model_hints->hint, name) == 0) {
        if (model_hints == model_hints_end) {
            free(model_hints->hint);
            free(model_hints);
            model_hints = NULL;
            model_hints_end = NULL;
        } else {
            prev = model_hints;
            model_hints = model_hints->next;
            free(prev->hint);
            free(prev);
        }
        return;
    }

    prev = model_hints;
    cur = model_hints->next;
    while (cur != NULL) {
        if (strcmp(cur->hint, name) == 0) {
            if (cur == model_hints_end) {
                model_hints_end = prev;
            }
            prev->next = cur->next;
            free(cur->hint);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void
remove_all_hints(void)
{
    struct model_hint *prev, *cur;

    if (model_hints == NULL) {
        return;
    }

    cur = model_hints;
    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
        free(prev->hint);
        free(prev);
    }

    model_hints = NULL;
    model_hints_end = NULL;
}

void
cmd_add_help(void)
{
    printf("add <path-to-model>\n");
}

void
cmd_print_help(void)
{
    printf("print [-f (yang | tree)] [-o <output-file>] <model-name>\n");
}

void
cmd_list_help(void)
{
    printf("list\n");
}

void
cmd_searchpath_help(void)
{
    printf("searchpath <model-dir-path> (on success removes all the models!)\n");
}

void
cmd_verb_help(void)
{
    printf("verb (error | warning | verbose | debug)\n");
}

int
cmd_add(const char *arg)
{
    int fd;
    char *argv, *path, *addr, *ptr;
    struct ly_module *model;
    struct stat sb;
    LY_MINFORMAT format;

    argv = strdupa(arg + strlen("add "));

    if ((path = strtok(argv, " ")) == NULL) {
        cmd_add_help();
        return 1;
    }

    if ((ptr = strrchr(path, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "yin")) {
            format = LY_IN_YIN;
        } else if (!strcmp(ptr, "yang")) {
            format = LY_IN_YANG;
        } else {
            fprintf(stderr, "Input file in an unknown format \"%s\".\n", ptr);
            return 1;
        }
    } else {
        fprintf(stdout, "Input file without extension, assuming YIN format.\n");
        format = LY_IN_YIN;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Opening input file failed (%s).\n", strerror(errno));
        return 1;
    }
    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "Unable to get input file information (%s).\n", strerror(errno));
        close(fd);
        return 1;
    }
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    model = ly_module_read(ctx, addr, format);
    munmap(addr, sb.st_size);
    close(fd);

    if (!model) {
        /* libyang printed the error messages */
        return 1;
    }

    add_hint(model->name);

    return 0;
}

int
cmd_print(const char *arg)
{
    int c, argc, option_index = 0, ret = 1;
    char **argv = NULL, *ptr;
    const char *out_path = NULL;
    struct ly_module *model;
    LY_MOUTFORMAT format = LY_OUT_TREE;
    FILE *output = stdout;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {NULL, 0, 0, 0}
    };

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        argv = realloc(argv, (argc+2)*sizeof *argv);
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    while (1) {
        c = getopt_long(argc, argv, "hf:o:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_print_help();
            ret = 0;
            goto cleanup;
        case 'f':
            if (!strcmp(optarg, "yang")) {
                format = LY_OUT_YANG;
            } else if (!strcmp(optarg, "tree")) {
                format = LY_OUT_TREE;
            } else {
                fprintf(stderr, "Unknown output format \"%s\".\n", optarg);
                goto cleanup;
            }
            break;
        case 'o':
            if (out_path) {
                fprintf(stderr, "Output specified twice.\n");
                goto cleanup;
            }
            out_path = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* file name */
    if (optind == argc) {
        fprintf(stderr, "Missing the model name.\n");
        goto cleanup;
    }
    model = ly_ctx_get_module(ctx, argv[optind], NULL, 0);

    if (model == NULL) {
        fprintf(stderr, "No model \"%s\" found.\n", argv[optind]);
        goto cleanup;
    }

    if (out_path) {
        output = fopen(out_path, "w");
        if (!output) {
            fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
            goto cleanup;
        }
    }

    ret = ly_model_print(output, model, format);

cleanup:
    free(*argv);
    free(argv);

    if (output && (output != stdout)) {
        fclose(output);
    }

    return ret;
}

int
cmd_list(const char *UNUSED(arg))
{
    struct model_hint *hint;

    printf("List of the loaded models:\n");

    hint = model_hints;
    if (!hint) {
        printf("\t(none)\n");
    } else {
        for (; hint; hint = hint->next) {
            printf("\t%s\n", hint->hint);
        }
    }

    return 0;
}

int
cmd_searchpath(const char *arg)
{
    const char *path;
    struct stat st;

    if (strchr(arg, ' ') == NULL) {
        fprintf(stderr, "Missing the search path.\n");
        return 1;
    }
    path = strchr(arg, ' ')+1;

    if (!strcmp(path, "-h") || !strcmp(path, "--help")) {
        cmd_searchpath_help();
        return 0;
    }

    if (stat(path, &st) == -1) {
        fprintf(stderr, "Failed to stat the search path (%s).\n", strerror(errno));
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "\"%s\" is not a directory.\n", path);
        return 1;
    }

    free(search_path);
    search_path = strdup(path);

    ly_ctx_destroy(ctx);
    ctx = ly_ctx_new(search_path);
    remove_all_hints();

    printf("All models have been removed.\n");

    return 0;
}

int
cmd_verb(const char *arg)
{
    const char *verb;

    verb = arg + 5;
    if (strcmp(verb, "error") == 0) {
        ly_verb(0);
    } else if (strcmp(verb, "warning") == 0) {
        ly_verb(1);
    } else if (strcmp(verb, "verbose") == 0) {
        ly_verb(2);
    } else if (strcmp(verb, "debug") == 0) {
        ly_verb(3);
    } else {
        fprintf(stderr, "Unknown verbosity \"%s\"", verb);
        return 1;
    }

    return 0;
}

int
cmd_quit(const char *UNUSED(arg))
{
    done = 1;
    return 0;
}

int
cmd_help(const char *arg)
{
    int i;
    char *args = strdupa(arg);
    char *cmd = NULL;

    strtok(args, " ");
    if ((cmd = strtok(NULL, " ")) == NULL) {

generic_help:
        fprintf(stdout, "Available commands:\n");

        for (i = 0; commands[i].name; i++) {
            if (commands[i].helpstring != NULL) {
                fprintf(stdout, "  %-15s %s\n", commands[i].name, commands[i].helpstring);
            }
        }
    } else {
        /* print specific help for the selected command */

        /* get the command of the specified name */
        for (i = 0; commands[i].name; i++) {
            if (strcmp(cmd, commands[i].name) == 0) {
                break;
            }
        }

        /* execute the command's help if any valid command specified */
        if (commands[i].name) {
            if (commands[i].help_func != NULL) {
                commands[i].help_func();
            } else {
                printf("%s\n", commands[i].helpstring);
            }
        } else {
            /* if unknown command specified, print the list of commands */
            printf("Unknown command \'%s\'\n", cmd);
            goto generic_help;
        }
    }

    return 0;
}

COMMAND commands[] = {
        {"help", cmd_help, NULL, "Display commands description"},
        {"add", cmd_add, cmd_add_help, "Add a new model"},
        {"print", cmd_print, cmd_print_help, "Print model"},
        {"list", cmd_list, cmd_list_help, "List all the loaded models"},
        {"searchpath", cmd_searchpath, cmd_searchpath_help, "Set the search path for models"},
        {"verb", cmd_verb, cmd_verb_help, "Change verbosity"},
        {"quit", cmd_quit, NULL, "Quit the program"},
        /* synonyms for previous commands */
        {"?", cmd_help, NULL, "Display commands description"},
        {"exit", cmd_quit, NULL, "Quit the program"},
        {NULL, NULL, NULL, NULL}
};
