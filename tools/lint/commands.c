/**
 * @file commands.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool commands
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
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include "commands.h"
#include "../../src/libyang.h"
#include "../../src/tree.h"
#include "../../src/parser.h"

COMMAND commands[];
extern int done;
extern struct ly_ctx *ctx;
extern char *search_path;

void
cmd_add_help(void)
{
    printf("add <path-to-model>\n");
}

void
cmd_print_help(void)
{
    printf("print [-f (yang | tree | info)] [-t <info-target-node>] [-o <output-file>] <model-name>\n\n");
    printf("\tinfo-target-node: <absolute-schema-node> | typedef/<typedef-name> |\n");
    printf("\t                  | identity/<identity-name> | feature/<feature-name> |\n");
    printf("\t                  | type/<absolute-schema-node-leaf-or-leaflist>\n");
}

void
cmd_data_help(void)
{
    printf("data [-f (xml | json)] [-o <output-file>] <data-file-name>\n");
}

void
cmd_list_help(void)
{
    printf("list\n");
}

void
cmd_feature_help(void)
{
    printf("feature -(-p)rint | (-(-e)nable | -(-d)isable (* | <feature-name>)) <model-name>\n");
}

void
cmd_searchpath_help(void)
{
    printf("searchpath <model-dir-path>\n");
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
    char *addr, *ptr;
    const char *path;
    struct ly_module *model;
    struct stat sb;
    LY_MINFORMAT format;

    if (strlen(arg) < 5) {
        cmd_add_help();
        return 1;
    }

    path = (arg + strlen("add "));

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

    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "Input file not a file.\n");
        close(fd);
        return 1;
    }

    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    model = lys_parse(ctx, addr, format);
    munmap(addr, sb.st_size);
    close(fd);

    if (!model) {
        /* libyang printed the error messages */
        return 1;
    }

    return 0;
}

int
cmd_print(const char *arg)
{
    int c, i, argc, option_index, ret = 1;
    char **argv = NULL, *ptr, *target_node = NULL, **names;
    const char *out_path = NULL;
    struct ly_module *model, *parent_model;
    LY_MOUTFORMAT format = LY_OUT_TREE;
    FILE *output = stdout;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {"target-node", required_argument, 0, 't'},
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

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hf:o:t:", long_options, &option_index);
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
            } else if (!strcmp(optarg, "info")) {
                format = LY_OUT_INFO;
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
        case 't':
            target_node = optarg;
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

    model = ly_ctx_get_module(ctx, argv[optind], NULL);
    if (model == NULL) {
        names = ly_ctx_get_module_names(ctx);
        for (i = 0; names[i]; i++) {
            if (!model) {
                parent_model = ly_ctx_get_module(ctx, names[i], NULL);
                model = (struct ly_module *)ly_ctx_get_submodule(parent_model, argv[optind], NULL);
            }
            free(names[i]);
        }
        free(names);
    }

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

    ret = ly_model_print(output, model, format, target_node);

cleanup:
    free(*argv);
    free(argv);

    if (output && (output != stdout)) {
        fclose(output);
    }

    return ret;
}

int
cmd_data(const char *arg)
{
    int c, argc, option_index, ret = 1, fd = -1;
    struct stat sb;
    char **argv = NULL, *ptr, *addr;
    const char *out_path = NULL;
    struct lyd_node *data = NULL;
    LY_DFORMAT format = LY_DATA_UNKNOWN;
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

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hf:o:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_data_help();
            ret = 0;
            goto cleanup;
        case 'f':
            if (!strcmp(optarg, "xml")) {
                format = LY_DATA_XML;
            } else if (!strcmp(optarg, "json")) {
                format = LY_DATA_JSON;
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
        fprintf(stderr, "Missing the data file name.\n");
        goto cleanup;
    }

    fd = open(argv[optind], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "The input file could not be opened (%s).\n", strerror(errno));
        goto cleanup;
    }

    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "Unable to get input file information (%s).\n", strerror(errno));
        goto cleanup;
    }

    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "Input file not a file.\n");
        goto cleanup;
    }

    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    data = xml_read_data(ctx, addr);
    munmap(addr, sb.st_size);

    if (data == NULL) {
        fprintf(stderr, "Failed to parse data.\n");
        goto cleanup;
    }

    if (out_path) {
        output = fopen(out_path, "w");
        if (!output) {
            fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
            goto cleanup;
        }

        if (format == LY_DATA_UNKNOWN) {
            /* default */
            format = LY_DATA_XML;
        }
    }

    if (format != LY_DATA_UNKNOWN) {
        ly_data_print(output, data, format);
    }

cleanup:
    free(*argv);
    free(argv);

    if (output && (output != stdout)) {
        fclose(output);
    }

    if (fd != -1) {
        close(fd);
    }

    lyd_node_siblings_free(data);

    return ret;
}

int
cmd_list(const char *UNUSED(arg))
{
    char **names, **sub_names;
    int i, j;

    printf("List of the loaded models:\n");

    names = ly_ctx_get_module_names(ctx);

    for (i = 0; names[i]; ++i) {
        printf("\t%s\n", names[i]);

        sub_names = ly_ctx_get_submodule_names(ctx, names[i]);
        for (j = 0; sub_names[j]; ++j) {
            printf("\t\t%s\n", sub_names[j]);
            free(sub_names[j]);
        }
        free(sub_names);

        free(names[i]);
    }
    free(names);

    if (i == 0) {
        printf("\t(none)\n");
    }

    return 0;
}

int
cmd_feature(const char *arg)
{
    int c, i, argc, option_index, ret = 1, task = -1;
    char **argv = NULL, *ptr, **names, **enable_state;
    const char *feat_name = NULL;
    struct ly_module *model, *parent_model;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"print", no_argument, 0, 'p'},
        {"enable", required_argument, 0, 'e'},
        {"disable", required_argument, 0, 'd'},
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

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hpe:d:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_feature_help();
            ret = 0;
            goto cleanup;
        case 'p':
            if (task != -1) {
                fprintf(stderr, "Only one of print, enable, or disable can be specified.\n");
                goto cleanup;
            }
            task = 0;
            break;
        case 'e':
            if (task != -1) {
                fprintf(stderr, "Only one of print, enable, or disable can be specified.\n");
                goto cleanup;
            }
            task = 1;
            feat_name = optarg;
            break;
        case 'd':
            if (task != -1) {
                fprintf(stderr, "Only one of print, enable, or disable can be specified.\n");
                goto cleanup;
            }
            task = 2;
            feat_name = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* model name */
    if (optind == argc) {
        fprintf(stderr, "Missing the model name.\n");
        goto cleanup;
    }
    model = ly_ctx_get_module(ctx, argv[optind], NULL);
    if (model == NULL) {
        names = ly_ctx_get_module_names(ctx);
        for (i = 0; names[i]; i++) {
            if (!model) {
                parent_model = ly_ctx_get_module(ctx, names[i], NULL);
                model = (struct ly_module *)ly_ctx_get_submodule(parent_model, argv[optind], NULL);
            }
            free(names[i]);
        }
        free(names);
    }
    if (model == NULL) {
        fprintf(stderr, "No model \"%s\" found.\n", argv[optind]);
        goto cleanup;
    }

    if (task == -1) {
        fprintf(stderr, "One of print, enable, or disable must be specified.\n");
        goto cleanup;
    }

    if (task == 0) {
        printf("%s features:\n", model->name);

        names = ly_get_features(model, &enable_state);
        for (i = 0; names[i]; ++i) {
            printf("\t%s %s\n", names[i], enable_state[i]);
            free(names[i]);
            free(enable_state[i]);
        }
        free(names);
        free(enable_state);
        if (!i) {
            printf("\t(none)\n");
        }
    } else if (task == 1) {
        if (ly_features_enable(model, feat_name)) {
            fprintf(stderr, "Feature \"%s\" not found.\n", feat_name);
            ret = 1;
        }
    } else if (task == 2) {
        if (ly_features_disable(model, feat_name)) {
            fprintf(stderr, "Feature \"%s\" not found.\n", feat_name);
            ret = 1;
        }
    }

cleanup:
    free(*argv);
    free(argv);

    return ret;
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

    ly_ctx_set_searchdir(ctx, search_path);

    return 0;
}

int
cmd_clear(const char *UNUSED(arg))
{
    ly_ctx_destroy(ctx);
    ctx = ly_ctx_new(search_path);
    return 0;
}

int
cmd_verb(const char *arg)
{
    const char *verb;
    if (strlen(arg) < 5) {
        cmd_verb_help();
        return 1;
    }

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
        fprintf(stderr, "Unknown verbosity \"%s\"\n", verb);
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
        {"data", cmd_data, cmd_data_help, "Load, validate and optionally print data"},
        {"list", cmd_list, cmd_list_help, "List all the loaded models"},
        {"feature", cmd_feature, cmd_feature_help, "Print/enable/disable all/specific features of models"},
        {"searchpath", cmd_searchpath, cmd_searchpath_help, "Set the search path for models"},
        {"clear", cmd_clear, NULL, "Clear the context - remove all the loaded models"},
        {"verb", cmd_verb, cmd_verb_help, "Change verbosity"},
        {"quit", cmd_quit, NULL, "Quit the program"},
        /* synonyms for previous commands */
        {"?", cmd_help, NULL, "Display commands description"},
        {"exit", cmd_quit, NULL, "Quit the program"},
        {NULL, NULL, NULL, NULL}
};
