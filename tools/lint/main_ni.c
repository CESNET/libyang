/**
 * @file main_ni.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool - noninteractive code
 *
 * Copyright (c) 2015-2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"
#include "../../src/libyang.h"

volatile int verbose = 0;

void
help(int shortout)
{
    fprintf(stdout, "Usage: yanglint [[options] <filename>...]\n\n");
    fprintf(stdout, "Validates the YANG module in <filename>, and all its dependencies.\n");
    fprintf(stdout, "Running yanglint without any argument opens interactive mode with much more features.\n\n");

    if (shortout) {
        return;
    }
    fprintf(stdout, "Options:\n"
        "  -h, --help            Show this help message and exit.\n"
        "  -v, --version         Show version number and exit.\n"
        "  -V, --verbose         Show verbose messages.\n"
        "  -f FORMAT, --format=FORMAT\n"
        "                        Convert to FORMAT. Supported formats: tree, yin, yang.\n"
        "  -o OUTFILE, --output=OUTFILE\n"
        "                        Write the output to OUTFILE instead of stdout.\n"
        "  -F FEATURES, --features=FEATURES\n"
        "                        Features to support, default all.\n"
        "                        <modname>:[<feature>,]*\n"
        "  -p PATH, --path=PATH  Separated search path for yin and yang modules.\n\n"
        "Tree output specific options:\n"
        "  --tree-help           Print help on tree symbols and exit.\n");
}

void
tree_help(void)
{
    fprintf(stdout, "Each node is printed as:\n\n");
    fprintf(stdout, "<status> <flags> <name> <opts> <type> <if-features>\n\n"
                    "  <status> is one of:\n"
                    "    + for current\n"
                    "    x for deprecated\n"
                    "    o for obsolete\n\n"
                    "  <flags> is one of:\n"
                    "    rw for configuration data\n"
                    "    ro for status data\n"
                    "    -x for RPCs\n"
                    "    -n for Notification\n\n"
                    "  <name> is the name of the node\n"
                    "    (<name>) means that the node is a choice node\n"
                    "    :(<name>) means that the node is a case node\n\n"
                    "    if the node is augmented into the tree from another module,\n"
                    "    it is printed with the module name as <module-name>:<name>.\n\n"
                    "  <opts> is one of:\n"
                    "    ? for an optional leaf or choice\n"
                    "    ! for a presence container\n"
                    "    * for a leaf-list or list\n"
                    "    [<keys>] for a list's keys\n\n"
                    "  <type> is the name of the type for leafs and leaf-lists\n"
                    "    If there is a default value defined, it is printed within\n"
                    "    angle brackets <default-value>.\n"
                    "    If the type is a leafref, the type is printed as -> TARGET`\n\n"
                    "  <if-features> is the list of features this node depends on,\n"
                    "    printed within curly brackets and a question mark {...}?\n\n");
}

void
version(void)
{
    fprintf(stdout, "yanglint %d.%d.%d\n", LY_VERSION_MAJOR, LY_VERSION_MINOR, LY_VERSION_MICRO);
}
void
libyang_verbclb(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    if (verbose && level < LY_LLDBG) {
        if (path) {
            fprintf(stderr, "%s (%s)\n", msg, path);
        } else {
            fprintf(stderr, "%s\n", msg);
        }
    }
}

int
main_ni(int argc, char* argv[])
{
    int ret = EXIT_FAILURE;
    int opt, opt_index = 0, i, featsize = 0;
    struct option options[] = {
        {"format",           required_argument, NULL, 'f'},
        {"features",         required_argument, NULL, 'F'},
        {"help",             no_argument,       NULL, 'h'},
        {"tree-help",        no_argument,       NULL, 'H'},
        {"output",           required_argument, NULL, 'o'},
        {"path",             required_argument, NULL, 'p'},
        {"version",          no_argument,       NULL, 'v'},
        {"verbose",          no_argument,       NULL, 'V'},
        {NULL,               0,                 NULL, 0}
    };
    FILE *out = stdout;
    struct ly_ctx *ctx = NULL;
    const struct lys_module **mods = NULL, *mod;
    LYS_OUTFORMAT outformat = 0;
    LYS_INFORMAT informat;
    char *searchpath = NULL;
    char **feat = NULL, *ptr, *featlist;
    struct stat st;
    uint32_t u;

    opterr = 0;
    while ((opt = getopt_long(argc, argv, "d:f:F:hHo:p:vV", options, &opt_index)) != -1) {
        switch (opt) {
        case 'f':
            if (!strcasecmp(optarg, "tree")) {
                outformat = LYS_OUT_TREE;
            } else if (!strcasecmp(optarg, "yin")) {
                outformat = LYS_OUT_YIN;
            } else if (!strcasecmp(optarg, "yang")) {
                outformat = LYS_OUT_YANG;
            } else {
                fprintf(stderr, "yanglint error: unsupported format %s\n", optarg);
                goto cleanup;
            }
            break;
        case 'F':
            featsize++;
            if (!feat) {
                feat = malloc(sizeof *feat);
            } else {
                feat = realloc(feat, featsize * sizeof *feat);
            }
            feat[featsize - 1] = strdup(optarg);
            ptr = strchr(feat[featsize - 1], ':');
            if (!ptr) {
                fprintf(stderr, "Invalid format of the features specification (%s)", optarg);
                goto cleanup;
            }
            *ptr = '\0';

            break;
        case 'h':
            help(0);
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'H':
            tree_help();
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'o':
            out = fopen(optarg, "w");
            if (!out) {
                fprintf(stderr, "yanglint error: unable open output file %s (%s)\n", optarg, strerror(errno));
                goto cleanup;
            }
            break;
        case 'p':
            searchpath = optarg;
            if (stat(searchpath, &st) == -1) {
                fprintf(stderr, "Unable to use search path (%s) - %s.\n", searchpath, strerror(errno));
                goto cleanup;
            }
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "Provided search path is not a directory.\n");
                goto cleanup;
            }
            break;
        case 'v':
            version();
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'V':
            verbose = 1;
            break;
        default:
            help(1);
            if (optopt) {
                fprintf(stderr, "yanglint error: invalid option: -%c\n", optopt);
            } else {
                fprintf(stderr, "yanglint error: invalid option: %s\n", argv[optind - 1]);
            }
            goto cleanup;
        }
    }

    if (optind >= argc) {
        help(1);
        fprintf(stderr, "yanglint error: missing <filename> to validate\n");
        goto cleanup;;
    } else if (outformat && outformat != LYS_OUT_TREE && (optind + 1) < argc) {
        /* we have multiple schemas to be printed as YIN or YANG */
        fprintf(stderr, "yanglint error: too many files to convert\n");
        goto cleanup;
    }

    /* create libyang context */
    ctx = ly_ctx_new(searchpath);
    if (!ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto cleanup;
    }

    /* derefered setting of verbosity in libyang after context initiation */
    if (verbose) {
        ly_verb(LY_LLVRB);
        ly_set_log_clb(libyang_verbclb, 1);
    }

    mods = malloc((argc - optind) * sizeof *mods);
    if (!mods) {
        fprintf(stderr, "Memory allocation failed.\n");
        goto cleanup;
    }
    for (i = 0; i < argc - optind; i++) {
        /* validate schema */
        informat = get_schema_format(argv[optind]);
        if (informat == LYS_IN_UNKNOWN) {
            goto cleanup;
        }
        if (verbose) fprintf(stdout, "Validating %s file.\n", argv[optind]);
        mods[i] = lys_parse_path(ctx, argv[optind], informat);
        if (!mods[i]) {
            goto cleanup;
        }
    }


    if (outformat) {
        /* enable specified features, if not specified, all the module's features are enabled */
        u = 4; /* skip internal libyang modules */
        while ((mod = ly_ctx_get_module_iter(ctx, &u))) {
            for (i = 0; i < featsize; i++) {
                if (!strcmp(feat[i], mod->name)) {
                    /* parse features spec */
                    featlist = strdup(feat[i] + strlen(feat[i]) + 1);
                    ptr = NULL;
                    while((ptr = strtok(ptr ? NULL : featlist, ","))) {
                        if (verbose) fprintf(stdout, "Enabling feature %s in module %s.\n", ptr, mod->name);
                        if (lys_features_enable(mod, ptr)) {
                            fprintf(stderr, "Feature %s not defined in module %s.\n", ptr, mod->name);
                        }
                    }
                    free(featlist);
                    break;
                }
            }
            if (i == featsize) {
                if (verbose) fprintf(stdout, "Enabling all features in module %s.\n", mod->name);
                lys_features_enable(mod, "*");
            }
        }


        /* convert (print) to FORMAT */
        for (i = 0; i < argc - optind; i++) {
            lys_print_file(out, mods[i], outformat, NULL);
        }
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (out && out != stdout) {
        fclose(out);
    }
    free(mods);
    for (i = 0; i < featsize; i++) {
        free(feat[i]);
    }
    free(feat);
    ly_ctx_destroy(ctx, NULL);

    return ret;
}
