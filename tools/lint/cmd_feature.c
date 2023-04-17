/**
 * @file cmd_feature.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief 'feature' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2015-2021 CESNET, z.s.p.o.
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

#include "libyang.h"

#include "common.h"

void
cmd_feature_help(void)
{
    printf("Usage: feature [-f] <module> [<module>]*\n"
            "       feature -a [-f]\n"
            "                  Print features of all the modules with state of each one.\n\n"
            "  -f <module1, module2, ...>, --feature-param <module1, module2, ...>\n"
            "                  Generate features parameter for the command \"add\" \n"
            "                  in the form of -F <module-name>:<features>\n"
            "  -a, --all \n"
            "                  Print features of all implemented modules.\n");
}

void
cmd_feature(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    char *features_output = NULL;
    int opt, opt_index, i;
    ly_bool generate_features = 0, print_all = 0;
    struct ly_set set = {0};
    const struct lys_module *mod;
    struct ly_out *out = NULL;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {"all", no_argument, NULL, 'a'},
        {"feature-param", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, commands[CMD_FEATURE].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_feature_help();
            goto cleanup;
        case 'a':
            print_all = 1;
            break;
        case 'f':
            generate_features = 1;
            break;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (ly_out_new_file(stdout, &out)) {
        YLMSG_E("Unable to print to the standard output.\n");
        goto cleanup;
    }

    if (print_all) {
        if (print_all_features(out, *ctx, generate_features, &features_output)) {
            YLMSG_E("Printing all features failed.\n");
            goto cleanup;
        }
        if (generate_features) {
            printf("%s\n", features_output);
        }
        goto cleanup;
    }

    if (argc == optind) {
        YLMSG_E("Missing modules to print.\n");
        goto cleanup;
    }

    for (i = 0; i < argc - optind; i++) {
        /* always erase the set, so the previous module's features don't carry over to the next module's features */
        ly_set_erase(&set, NULL);

        mod = ly_ctx_get_module_latest(*ctx, argv[optind + i]);
        if (!mod) {
            YLMSG_E("Module \"%s\" not found.\n", argv[optind + i]);
            goto cleanup;
        }

        /* collect features of the module */
        if (collect_features(mod, &set)) {
            goto cleanup;
        }

        if (generate_features) {
            if (generate_features_output(mod, &set, &features_output)) {
                goto cleanup;
            }
            /* don't print features and their state of each module if generating features parameter */
            continue;
        }

        print_features(out, mod, &set);
    }

    if (generate_features) {
        printf("%s\n", features_output);
    }

cleanup:
    free_cmdline(argv);
    ly_out_free(out, NULL, 0);
    ly_set_erase(&set, NULL);
    free(features_output);
}
