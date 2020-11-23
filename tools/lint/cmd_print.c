/**
 * @file cmd_print.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'print' command of the libyang's yanglint tool.
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

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"

void
cmd_print_help(void)
{
    printf("Usage: print [-f (yang | yin | tree | info [-q -P PATH])] [-o OUTFILE]\n"
            "            [<module-name1>[@revision]] ...\n"
            "                  Print a schema module. The <module-name> is not required\n"
            "                  only in case the -P option is specified.\n\n"
            "  -f FORMAT, --format=FORMAT\n"
            "                  Print the module in the specified FORMAT. If format not\n"
            "                  specified, the 'tree' format is used.\n"
            "  -P PATH, --schema-node=PATH\n"
            "                 Print only the specified subtree of the schema.\n"
            "                 The PATH is the XPath subset mentioned in documentation as\n"
            "                 the Path format. The option can be combined with --single-node\n"
            "                 option to print information only about the specified node.\n"
            "  -q, --single-node\n"
            "                 Supplement to the --schema-node option to print information\n"
            "                 only about a single node specified as PATH argument.\n"
            "  -o OUTFILE, --output=OUTFILE\n"
            "                  Write the output to OUTFILE instead of stdout.\n");
}

void
cmd_print(struct ly_ctx **ctx, const char *cmdline)
{
    LY_ERR ret;
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"format", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'},
        {"output", required_argument, NULL, 'o'},
        {"schema-node", required_argument, NULL, 'P'},
        {"single-node", no_argument, NULL, 'q'},
        {NULL, 0, NULL, 0}
    };
    uint16_t options_print = 0;
    const char *node_path = NULL;
    LYS_OUTFORMAT format = LYS_OUT_TREE;
    const char *out_path = NULL;
    struct ly_out *out = NULL;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "f:ho:P:q", options, &opt_index)) != -1) {
        switch (opt) {
        case 'o': /* --output */
            if (out) {
                if (ly_out_filepath(out, optarg) != NULL) {
                    YLMSG_E("Unable open output file %s (%s)\n", optarg, strerror(errno));
                    goto cleanup;
                }
            } else {
                if (ly_out_new_filepath(optarg, &out)) {
                    YLMSG_E("Unable open output file %s (%s)\n", optarg, strerror(errno));
                    goto cleanup;
                }
            }
            break;

        case 'f': /* --format */
            if (!strcasecmp(optarg, "yang")) {
                format = LYS_OUT_YANG;
            } else if (!strcasecmp(optarg, "yin")) {
                format = LYS_OUT_YIN;
            } else if (!strcasecmp(optarg, "info")) {
                format = LYS_OUT_YANG_COMPILED;
            } else if (!strcasecmp(optarg, "tree")) {
                format = LYS_OUT_TREE;
            } else {
                YLMSG_E("Unknown output format %s\n", optarg);
                cmd_print_help();
                goto cleanup;
            }
            break;

        case 'P': /* --schema-node */
            node_path = optarg;
            break;

        case 'q': /* --single-node */
            options_print |= LYS_PRINT_NO_SUBSTMT;
            break;

        case 'h':
            cmd_print_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    /* file name */
    if ((argc == optind) && !node_path) {
        YLMSG_E("Missing the name of the module to print.\n");
        goto cleanup;
    }

    if (out_path) {
        ret = ly_out_new_filepath(out_path, &out);
    } else {
        ret = ly_out_new_file(stdout, &out);
    }
    if (ret) {
        YLMSG_E("Could not open the output file (%s).\n", strerror(errno));
        goto cleanup;
    }

    if (node_path) {
        const struct lysc_node *node;
        node = lys_find_path(*ctx, NULL, node_path, 0);
        if (!node) {
            node = lys_find_path(*ctx, NULL, node_path, 1);

            if (!node) {
                YLMSG_E("The requested schema node \"%s\" does not exists.\n", node_path);
                goto cleanup;
            }
        }
        if (lys_print_node(out, node, format, 0, options_print)) {
            YLMSG_E("Unable to print schema node %s.\n", node_path);
            goto cleanup;
        }
    } else {
        for (int i = 0; i < argc - optind; i++) {
            struct lys_module *module;
            char *revision;

            /* get revision */
            revision = strchr(argv[optind + i], '@');
            if (revision) {
                revision[0] = '\0';
                ++revision;
            }

            if (revision) {
                module = ly_ctx_get_module(*ctx, argv[optind + i], revision);
            } else {
                module = ly_ctx_get_module_latest(*ctx, argv[optind + i]);
            }
            if (!module) {
                /* TODO try to find it as a submodule */
                if (revision) {
                    YLMSG_E("No (sub)module \"%s\" in revision %s found.\n", argv[optind + i], revision);
                } else {
                    YLMSG_E("No (sub)module \"%s\" found.\n", argv[optind + i]);
                }
                goto cleanup;
            }
            if (lys_print_module(out, module, format, 0, options_print)) {
                YLMSG_E("Unable to print module %s.\n", argv[optind + i]);
                goto cleanup;
            }
        }
    }

cleanup:
    free_cmdline(argv);
    ly_out_free(out, NULL, out_path ? 1 : 0);
}
