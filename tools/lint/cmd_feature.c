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
    printf("Usage: feature [-h] <module> [<module>]*\n"
            "                  Print features of all the module with state of each one.\n");
}

int
collect_features(const struct lys_module *mod, struct ly_set *set)
{
    struct lysp_feature *f = NULL;
    uint32_t idx = 0;

    while ((f = lysp_feature_next(f, mod->parsed, &idx))) {
        if (ly_set_add(set, (void *)f->name, 1, NULL)) {
            YLMSG_E("Memory allocation failed.\n");
            ly_set_erase(set, NULL);
            return 1;
        }
    }

    return 0;
}

void
cmd_feature(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index, i;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    struct ly_set set = {0};
    size_t max_len;
    uint32_t j;
    const char *name;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "h", options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_feature_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (argc == optind) {
        YLMSG_E("Missing modules to print.\n");
        goto cleanup;
    }

    for (i = 0; i < argc - optind; i++) {
        const struct lys_module *mod = ly_ctx_get_module_latest(*ctx, argv[optind + i]);
        if (!mod) {
            YLMSG_E("Module \"%s\" not found.\n", argv[optind + i]);
            goto cleanup;
        }

        /* collect features of the module */
        if (collect_features(mod, &set)) {
            goto cleanup;
        }

        /* header */
        printf("%s features:\n", mod->name);

        if (set.count) {
            /* get max len */
            max_len = 0;
            for (j = 0; j < set.count; ++j) {
                name = set.objs[j];
                if (strlen(name) > max_len) {
                    max_len = strlen(name);
                }
            }

            /* print features */
            for (j = 0; j < set.count; ++j) {
                name = set.objs[j];
                printf("\t%-*s (%s)\n", (int)max_len, name, lys_feature_value(mod, name) ? "off" : "on");
            }

            ly_set_erase(&set, NULL);
        } else {
            printf("\t(none)\n");
        }
    }

cleanup:
    free_cmdline(argv);
}
