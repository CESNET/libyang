/**
 * @file main.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yangupdate Data Update tool
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <libyang/libyang.h>

#include "yang_update.h"

static void
help(void)
{
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "    yangupdate [-hv]\n");
    fprintf(stdout, "    yangupdate -M mod -R 2026-04-02 -d data_old.json -o data_new.json\n\n");
    fprintf(stdout, "Options:\n"
            "  -h, --help                  Show this help message and exit.\n"
            "  -v, --verbose               Increase verbosity. Can be specified multiple times.\n"
            "  -s, --searchdir=SEARCH-DIR  Directory with YANG modules in all the required revisions\n"
            "                              and 'ietf-yang-schema-comparison-output' YANG module. Can be\n"
            "                              specified repeatedly.\n"
            "  -M, --module=MOD-NAME       Name of the YANG module of the current data.\n"
            "  -R, --old-module-revision=REVISION   Specific revision of the data. If not set, the\n"
            "                                       earliest revision is found based on the compiled plugins.\n"
            "  -F, --old-module-features=FEATURE*   Features to enable in the YANG module, separated\n"
            "                                       by a comma. Use no argument for no enabled features.\n"
            "  -N, --new-module-revision=REVISION   Specific revision the data should be updated to.\n"
            "                                       If not set, the latest revision is found based\n"
            "                                       on the compiled plugins.\n"
            "  -G, --new-module-features=FEATURE*   Specific features that should be enabled in the\n"
            "                                       new module of the updated data. Useful is there\n"
            "                                       is not an unambiguous chain of the plugins to use\n"
            "                                       to select the final plugin this way. Use no argument\n"
            "                                       for no enabled features.\n"
            "  -d, --data=DATA-FILE        Path to the YANG data file with the current data.\n"
            "  -f, --format=FORMAT         Format of the new updated YANG data, default\n"
            "                              JSON (xml/json/lyb).\n"
            "  -o, --output=DATA-FILE      File to write the new updated YANG data into,\n"
            "                              default is the STDOUT stream.\n"
            "  -c, --changes=OUTPUT-FILE   Print all the changes of the updated data compared to the\n"
            "                              current data.\n\n");
}

static int
parse_features(const char *optarg, char ***features, uint32_t *feat_count)
{
    const char *ptr, *ptr2;
    void *mem;

    if (!optarg) {
        /* no enabled features */
        *features = malloc(sizeof **features);
        (*features)[0] = NULL;

        return 0;
    }

    ptr = optarg;
    while (1) {
        ptr2 = strchr(ptr, ',');

        /* new feature */
        mem = realloc(*features, (*feat_count + 2) * sizeof **features);
        if (!mem) {
            fprintf(stderr, "yangupdate err: memory allocation failed (%s:%d)\n", __FILE__, __LINE__);
            return 1;
        }
        *features = mem;
        (*features)[*feat_count] = ptr2 ? strndup(ptr, ptr2 - ptr) : strdup(ptr);
        if (!(*features)[*feat_count]) {
            fprintf(stderr, "yangupdate err: memory allocation failed (%s:%d)\n", __FILE__, __LINE__);
            return 1;
        }

        ++(*feat_count);
        (*features)[*feat_count] = NULL;

        if (!ptr2) {
            break;
        }

        /* next */
        ptr = ptr2 + 1;
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    int o, opt_index = 0, verbosity = 0, rc = 0;
    struct option options[] = {
        {"help",                no_argument,       NULL, 'h'},
        {"verbose",             no_argument,       NULL, 'v'},
        {"searchdir",           required_argument, NULL, 's'},
        {"module",              required_argument, NULL, 'M'},
        {"old-module-revision", required_argument, NULL, 'R'},
        {"old-module-features", optional_argument, NULL, 'F'},
        {"new-module-revision", required_argument, NULL, 'N'},
        {"new-module-features", optional_argument, NULL, 'G'},
        {"data",                required_argument, NULL, 'd'},
        {"format",              required_argument, NULL, 'f'},
        {"output",              required_argument, NULL, 'o'},
        {"changes",             required_argument, NULL, 'c'},
        {NULL,                  0,                 NULL, 0}
    };
    struct ly_ctx *ctx_old = NULL, *ctx_new = NULL;
    struct lys_module *mod_old, *mod_new;
    struct lyd_node *data_old = NULL, *data_new = NULL;
    struct ly_in *in = NULL;
    const char *mod_name = NULL, **searchdirs = NULL, *data_old_path = NULL, *data_new_path = NULL;
    const char *changes_path = NULL, *mod_old_revision = NULL, *mod_new_revision = NULL;
    char **mod_old_features = NULL, **mod_new_features = NULL;
    uint32_t i, searchdir_count = 0, old_feature_count = 0, new_feature_count = 0;
    LYD_FORMAT format = LYD_JSON;
    FILE *f = NULL, *f_ch = NULL, *f_out;

    opterr = 0;
    while ((o = getopt_long(argc, argv, "hvs:M:R:F::N:G::d:f:o:c:", options, &opt_index)) != -1) {
        switch (o) {
        case 'h':
            help();
            return 1;

        case 'v':
            ++verbosity;
            break;

        case 's':
            searchdirs = realloc(searchdirs, (searchdir_count + 2) * sizeof *searchdirs);
            searchdirs[searchdir_count] = optarg;
            ++searchdir_count;
            searchdirs[searchdir_count] = NULL;
            break;

        case 'M':
            mod_name = optarg;
            break;

        case 'R':
            mod_old_revision = optarg;
            break;

        case 'F':
            if (!optarg && (optind < argc) && (argv[optind][0] != '-')) {
                /* assume the parameter is the optional argument */
                optarg = argv[optind++];
            }

            if (parse_features(optarg, &mod_old_features, &old_feature_count)) {
                rc = -1;
                goto cleanup;
            }
            break;

        case 'N':
            mod_new_revision = optarg;
            break;

        case 'G':
            if (!optarg && (optind < argc) && (argv[optind][0] != '-')) {
                /* assume the parameter is the optional argument */
                optarg = argv[optind++];
            }

            if (parse_features(optarg, &mod_new_features, &new_feature_count)) {
                rc = -1;
                goto cleanup;
            }
            break;

        case 'd':
            data_old_path = optarg;
            break;

        case 'f':
            if (!strcmp(optarg, "xml")) {
                format = LYD_XML;
            } else if (!strcmp(optarg, "json")) {
                format = LYD_JSON;
            } else if (!strcmp(optarg, "lyb")) {
                format = LYD_LYB;
            } else {
                fprintf(stderr, "yangupdate err: invalid format \"%s\"\n", optarg);
                rc = 1;
                goto cleanup;
            }
            break;

        case 'o':
            data_new_path = optarg;
            break;

        case 'c':
            changes_path = optarg;
            break;

        default:
            if (optopt) {
                fprintf(stderr, "yangupdate err: invalid option: -%c\n", optopt);
            } else {
                fprintf(stderr, "yangupdate err: invalid option: %s\n", argv[optind - 1]);
            }
            rc = 1;
            goto cleanup;
        }
    }

    /* redundant parameters */
    if (optind < argc) {
        fprintf(stderr, "yangupdate err: redundant parameters, use '-h' for help\n");
        rc = 1;
        goto cleanup;
    }

    /* missing parameters */
    if (!mod_name || !data_old_path) {
        help();
        rc = 1;
        goto cleanup;
    }

    /* set verbosity */
    ly_log_level(verbosity);

    /* load the old module */
    if (lyd_update_find_old(mod_name, mod_old_revision, (const char **)mod_old_features, searchdirs, &ctx_old, &mod_old)) {
        rc = -1;
        goto cleanup;
    }

    /* load the old data */
    if (lyd_parse_data_path(ctx_old, data_old_path, LYD_UNKNOWN, 0, LYD_VALIDATE_PRESENT, &data_old)) {
        rc = -1;
        goto cleanup;
    }

    /* load the new module */
    if (lyd_update_find_new(mod_old, mod_new_revision, (const char **)mod_new_features, &ctx_new, &mod_new)) {
        rc = -1;
        goto cleanup;
    }

    /* get the updated data */
    if (lyd_update(mod_old, data_old, mod_new, &data_new)) {
        rc = -1;
        goto cleanup;
    }

    /* print the transformed data */
    if (data_new_path) {
        f = fopen(data_new_path, "w");
        if (!f) {
            fprintf(stderr, "yangupdate err: failed to open \"%s\" (%s)\n", data_new_path, strerror(errno));
            rc = -1;
            goto cleanup;
        }

        f_out = f;
    } else {
        f_out = stdout;
    }
    if (lyd_print_file(f_out, data_new, format, LYD_PRINT_SIBLINGS)) {
        rc = -1;
        goto cleanup;
    }

    /* print the changes */
    if (changes_path) {
        f_ch = fopen(changes_path, "w");
        if (!f_ch) {
            fprintf(stderr, "yangupdate err: failed to open \"%s\" (%s)\n", changes_path, strerror(errno));
            rc = -1;
            goto cleanup;
        }

        if (lyd_update_print(data_old, data_new, f_ch)) {
            rc = -1;
            goto cleanup;
        }
    }

cleanup:
    free(searchdirs);
    for (i = 0; i < old_feature_count; ++i) {
        free(mod_old_features[i]);
    }
    free(mod_old_features);
    for (i = 0; i < new_feature_count; ++i) {
        free(mod_new_features[i]);
    }
    free(mod_new_features);
    if (f) {
        fclose(f);
    }
    if (f_ch) {
        fclose(f_ch);
    }
    ly_in_free(in, 0);
    lyd_free_siblings(data_old);
    lyd_free_siblings(data_new);
    ly_ctx_destroy(ctx_old);
    ly_ctx_destroy(ctx_new);

    return rc;
}
