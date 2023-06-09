/**
 * @file cmd_print.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'print' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2015-2023 CESNET, z.s.p.o.
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
#include "yl_opt.h"

void
cmd_print_help(void)
{
    printf("Usage: print [-f (yang | yin | tree [-q -P PATH -L LINE_LENGTH ] | info [-q -P PATH])]\n"
            "            [-o OUTFILE] [<module-name1>[@revision]] ...\n"
            "                  Print a schema module. The <module-name> is not required\n"
            "                  only in case the -P option is specified. For yang, yin and tree\n"
            "                  formats, a submodule can also be printed.\n\n"
            "  -f FORMAT, --format=FORMAT\n"
            "                  Print the module in the specified FORMAT. If format not\n"
            "                  specified, the 'tree' format is used.\n"
            "  -L LINE_LENGTH, --tree-line-length=LINE_LENGTH\n"
            "                 The limit of the maximum line length on which the 'tree'\n"
            "                 format will try to be printed.\n"
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

int
cmd_print_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"format", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'},
        {"tree-line-length", required_argument, NULL, 'L'},
        {"output", required_argument, NULL, 'o'},
        {"schema-node", required_argument, NULL, 'P'},
        {"single-node", no_argument, NULL, 'q'},
        {NULL, 0, NULL, 0}
    };

    yo->schema_out_format = LYS_OUT_TREE;

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_PRINT].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'o': /* --output */
            if (yo->out) {
                YLMSG_E("Only a single output can be specified.\n");
                return 1;
            } else {
                if (ly_out_new_filepath(optarg, &yo->out)) {
                    YLMSG_E("Unable open output file %s (%s)\n", optarg, strerror(errno));
                    return 1;
                }
            }
            break;

        case 'f': /* --format */
            if (yl_opt_update_schema_out_format(optarg, yo)) {
                cmd_print_help();
                return 1;
            }
            break;

        case 'L': /* --tree-line-length */
            yo->line_length = atoi(optarg);
            break;

        case 'P': /* --schema-node */
            yo->schema_node_path = optarg;
            break;

        case 'q': /* --single-node */
            yo->schema_print_options |= LYS_PRINT_NO_SUBSTMT;
            break;

        case 'h':
            cmd_print_help();
            return 1;
        default:
            YLMSG_E("Unknown option.\n");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return 0;
}

int
cmd_print_dep(struct yl_opt *yo, int posc)
{
    /* file name */
    if (yo->interactive && !posc && !yo->schema_node_path) {
        YLMSG_E("Missing the name of the module to print.\n");
        return 1;
    }

    if ((yo->schema_out_format != LYS_OUT_TREE) && yo->line_length) {
        YLMSG_W("--tree-line-length take effect only in case of the tree output format.\n");
    }

    if (!yo->out) {
        if (ly_out_new_file(stdout, &yo->out)) {
            YLMSG_E("Could not use stdout to print output.\n");
        }
        yo->out_stdout = 1;
    }

    return 0;
}

static LY_ERR
print_submodule(struct ly_out *out, struct ly_ctx **ctx, char *name, char *revision, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR erc;
    const struct lysp_submodule *submodule;

    submodule = revision ?
            ly_ctx_get_submodule(*ctx, name, revision) :
            ly_ctx_get_submodule_latest(*ctx, name);

    erc = submodule ?
            lys_print_submodule(out, submodule, format, line_length, options) :
            LY_ENOTFOUND;

    return erc;
}

static LY_ERR
print_module(struct ly_out *out, struct ly_ctx **ctx, char *name, char *revision, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR erc;
    struct lys_module *module;

    module = revision ?
            ly_ctx_get_module(*ctx, name, revision) :
            ly_ctx_get_module_latest(*ctx, name);

    erc = module ?
            lys_print_module(out, module, format, line_length, options) :
            LY_ENOTFOUND;

    return erc;
}

static int
cmd_print_module(const char *posv, struct ly_out *out, struct ly_ctx **ctx, LYS_OUTFORMAT format,
        size_t line_length, uint32_t options)
{
    int rc = 0;
    LY_ERR erc;
    char *name = NULL, *revision;
    ly_bool search_submodul;

    name = strdup(posv);
    /* get revision */
    revision = strchr(name, '@');
    if (revision) {
        revision[0] = '\0';
        ++revision;
    }

    erc = print_module(out, ctx, name, revision, format, line_length, options);

    if (erc == LY_ENOTFOUND) {
        search_submodul = 1;
        erc = print_submodule(out, ctx, name, revision, format, line_length, options);
    } else {
        search_submodul = 0;
    }

    if (erc == LY_SUCCESS) {
        rc = 0;
    } else if (erc == LY_ENOTFOUND) {
        if (revision) {
            YLMSG_E("No (sub)module \"%s\" in revision %s found.\n", name, revision);
        } else {
            YLMSG_E("No (sub)module \"%s\" found.\n", name);
        }
        rc = 1;
    } else {
        if (search_submodul) {
            YLMSG_E("Unable to print submodule %s.\n", name);
        } else {
            YLMSG_E("Unable to print module %s.\n", name);
        }
        rc = 1;
    }

    free(name);
    return rc;
}

int
cmd_print_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    int rc = 0;

    if (yo->schema_out_format == LYS_OUT_TREE) {
        /* print tree from lysc_nodes */
        ly_ctx_set_options(*ctx, LY_CTX_SET_PRIV_PARSED);
    }

    if (yo->schema_node_path) {
        const struct lysc_node *node;

        node = find_schema_path(*ctx, yo->schema_node_path);
        if (!node) {
            YLMSG_E("The requested schema node \"%s\" does not exists.\n", yo->schema_node_path);
            return 1;
        }

        if (lys_print_node(yo->out, node, yo->schema_out_format, yo->line_length, yo->schema_print_options)) {
            YLMSG_E("Unable to print schema node %s.\n", yo->schema_node_path);
            return 1;
        }
    } else {
        rc = cmd_print_module(posv, yo->out, ctx, yo->schema_out_format, yo->line_length, yo->schema_print_options);
        if (!yo->last_one) {
            /* for YANG Tree Diagrams printing it's more readable to print a blank line between modules. */
            if (yo->schema_out_format == LYS_OUT_TREE) {
                ly_print(yo->out, "\n");
            }
        }
    }

    return rc;
}
