/**
 * @file main.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's YANG Regular Expression tool
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
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
#include <sys/stat.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "../../src/libyang.h"

void
help(void)
{
    fprintf(stdout, "YANG Regular Expressions processor.\n");
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "    yangre [-hvV] -p <regexp1> [-i] [-p <regexp2> [-i] ...] <string>\n\n");
    fprintf(stdout, "Returns 0 if string matches the pattern(s), 1 if not and -1 on error.\n\n");
    fprintf(stdout, "Options:\n"
        "  -h, --help              Show this help message and exit.\n"
        "  -v, --version           Show version number and exit.\n"
        "  -V, --verbose           Print the processing information.\n"
        "  -i, --invert-match      Invert-match modifier for the closest preceeding\n"
        "                          pattern.\n"
        "  -p, --pattern=\"REGEXP\"  Regular expression including the quoting,\n"
        "                          which is applied the same way as in a YANG module.\n\n");
    fprintf(stdout, "Examples:\n"
        "  pattern \"[0-9a-fA-F]*\";      -> yangre -p '\"[0-9a-fA-F]*\"' '1F'\n"
        "  pattern '[a-zA-Z0-9\\-_.]*';  -> yangre -p \"'[a-zA-Z0-9\\-_.]*'\" 'a-b'\n"
        "  pattern [xX][mM][lL].*;      -> yangre -p '[xX][mM][lL].*' 'xml-encoding'\n\n");
    fprintf(stdout, "Note that to pass YANG quoting through your shell, you are supposed to use\n"
                    "the other quotation around. For not-quoted patterns, use single quotes.\n\n");
}

void
version(void)
{
    fprintf(stdout, "yangre %d.%d.%d\n", LY_VERSION_MAJOR, LY_VERSION_MINOR, LY_VERSION_MICRO);
}

void
pattern_error(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) path; /* unused */

    if (level == LY_LLERR && strcmp(msg, "Module \"yangre\" parsing failed.")) {
        fprintf(stderr, "yangre error: %s\n", msg);
    }
}

static const char *module_start = "module yangre {"
    "yang-version 1.1;"
    "namespace urn:cesnet:libyang:yangre;"
    "prefix re;"
    "leaf pattern {"
    "  type string {";
static const char *module_invertmatch = " { modifier invert-match; }";
static const char *module_match = ";";
static const char *module_end = "}}}";

int
main(int argc, char* argv[])
{
    int i, opt_index = 0, ret = -1, verbose = 0;
    struct option options[] = {
        {"help",             no_argument,       NULL, 'h'},
        {"invert-match",     no_argument,       NULL, 'i'},
        {"pattern",          required_argument, NULL, 'p'},
        {"version",          no_argument,       NULL, 'v'},
        {"verbose",          no_argument,       NULL, 'V'},
        {NULL,               0,                 NULL, 0}
    };
    char **patterns = NULL, *str = NULL, *modstr = NULL;
    int *invert_match = NULL;
    int patterns_count = 0;
    void *reallocated1, *reallocated2;
    struct ly_ctx *ctx = NULL;
    const struct lys_module *mod;

    opterr = 0;
    while ((i = getopt_long(argc, argv, "hivVp:", options, &opt_index)) != -1) {
        switch (i) {
        case 'h':
            help();
            ret = -2; /* continue to allow printing version and help at once */
            break;
        case 'i':
            if (!patterns_count || invert_match[patterns_count - 1]) {
                help();
                fprintf(stderr, "yangre error: invert-match option must follow some pattern.\n");
                goto cleanup;
            }
            invert_match[patterns_count - 1] = 1;
            break;
        case 'p':
            patterns_count++;
            reallocated1 = realloc(patterns, patterns_count * sizeof *patterns);
            reallocated2 = realloc(invert_match, patterns_count * sizeof *invert_match);
            if (!reallocated1 || !reallocated2) {
                fprintf(stderr, "yangre error: memory allocation error.\n");
                goto cleanup;
            }
            patterns = reallocated1;
            patterns[patterns_count - 1] = optarg;
            invert_match = reallocated2;
            invert_match[patterns_count - 1] = 0;
            break;
        case 'v':
            version();
            ret = -2; /* continue to allow printing version and help at once */
            break;
        case 'V':
            verbose = 1;
            break;
        default:
            help();
            if (optopt) {
                fprintf(stderr, "yangre error: invalid option: -%c\n", optopt);
            } else {
                fprintf(stderr, "yangre error: invalid option: %s\n", argv[optind - 1]);
            }
            goto cleanup;
        }
    }

    if (ret == -2) {
        goto cleanup;
    }

    /* check options compatibility */
    if (optind >= argc) {
        help();
        fprintf(stderr, "yangre error: missing <string> parameter to process.\n");
        goto cleanup;
    } else if (!patterns_count) {
        help();
        fprintf(stderr, "yangre error: missing patern parameter to use.\n");
        goto cleanup;
    }
    str = argv[optind];

    for (modstr = (char*)module_start, i = 0; i < patterns_count; i++) {
        asprintf(&modstr, "%s pattern %s%s", modstr, patterns[i], invert_match[i] ? module_invertmatch : module_match);
    }
    asprintf(&modstr, "%s%s", modstr, module_end);

    ctx = ly_ctx_new(NULL);
    if (!ctx) {
        goto cleanup;
    }

    ly_set_log_clb(pattern_error, 0);
    mod = lys_parse_mem(ctx, modstr, LYS_IN_YANG);
    if (!mod || !mod->data) {
        goto cleanup;
    }

    ret = lyd_validate_value(mod->data, str);
    if (verbose) {
        for (i = 0; i < patterns_count; i++) {
            fprintf(stdout, "pattern  %d: %s\n", i + 1, patterns[i]);
            fprintf(stdout, "matching %d: %s\n", i + 1, invert_match[i] ? "inverted" : "regular");
        }
        fprintf(stdout, "string    : %s\n", str);
        fprintf(stdout, "result    : %s\n", ret ? "not matching" : "matching");
    }

cleanup:
    ly_ctx_destroy(ctx, NULL);
    free(patterns);
    free(invert_match);
    free(modstr);

    return ret;
}
