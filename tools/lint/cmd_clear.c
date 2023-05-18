/**
 * @file cmd_clear.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'clear' command of the libyang's yanglint tool.
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

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>

#include "libyang.h"

#include "common.h"

void
cmd_clear_help(void)
{
    printf("Usage: clear [-i] [-y]\n"
            "                  Replace the current context with an empty one, searchpaths\n"
            "                  are not kept.\n\n"
            "  -i, --make-implemented\n"
            "                When loading a module into the context, the imported 'referenced'\n"
            "                modules will also be implemented. If specified a second time,\n"
            "                all the modules will be implemented.\n"
            "  -y, --yang-library\n"
            "                  Load and implement internal \"ietf-yang-library\" YANG module.\n"
            "                  Note that this module includes definitions of mandatory state\n"
            "                  data that can result in unexpected data validation errors.\n"
            "  -Y FILE, --yang-library-file=FILE\n"
            "                Parse FILE with \"ietf-yang-library\" data and use them to\n"
            "                create an exact YANG schema context. Searchpaths defined so far\n"
            "                are used, but then deleted.\n");
}

/**
 * @brief Convert searchpaths into single string.
 *
 * @param[in] ctx Context with searchpaths.
 * @param[out] searchpaths Collection of paths in the single string. Paths are delimited by colon ":"
 * (on Windows, used semicolon ";" instead).
 * @return LY_ERR value.
 */
static LY_ERR
searchpaths_to_str(const struct ly_ctx *ctx, char **searchpaths)
{
    uint32_t i;
    int rc = 0;
    const char * const *dirs = ly_ctx_get_searchdirs(ctx);

    for (i = 0; dirs[i]; ++i) {
        rc = searchpath_strcat(searchpaths, dirs[i]);
        if (!rc) {
            break;
        }
    }

    return rc;
}

void
cmd_clear(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"make-implemented",    no_argument, NULL, 'i'},
        {"yang-library",        no_argument, NULL, 'y'},
        {"yang-library-file",   no_argument, NULL, 'Y'},
        {"help",             no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    uint16_t options_ctx = LY_CTX_NO_YANGLIBRARY;
    struct ly_ctx *ctx_new = NULL;
    char *ylibfile = NULL;
    char *searchpaths = NULL;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, commands[CMD_CLEAR].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'i':
            if (options_ctx & LY_CTX_REF_IMPLEMENTED) {
                options_ctx &= ~LY_CTX_REF_IMPLEMENTED;
                options_ctx |= LY_CTX_ALL_IMPLEMENTED;
            } else {
                options_ctx |= LY_CTX_REF_IMPLEMENTED;
            }
            break;
        case 'y':
            options_ctx &= ~LY_CTX_NO_YANGLIBRARY;
            break;
        case 'Y':
            options_ctx &= ~LY_CTX_NO_YANGLIBRARY;
            ylibfile = optarg;
            break;
        case 'h':
            cmd_clear_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (ylibfile) {
        /* Create context according to the provided yang-library data in a file but use already defined searchpaths. */
        if (searchpaths_to_str(*ctx, &searchpaths)) {
            YLMSG_E("Storing searchpaths failed.\n");
            goto cleanup;
        }
        if (ly_ctx_new_ylpath(searchpaths, ylibfile, LYD_UNKNOWN, options_ctx, &ctx_new)) {
            YLMSG_E("Unable to create libyang context with yang-library data.\n");
            goto cleanup;
        }
    } else {
        if (ly_ctx_new(NULL, options_ctx, &ctx_new)) {
            YLMSG_W("Failed to create context.\n");
            goto cleanup;
        }
    }

    /* Global variables in commands are also deleted. */
    cmd_free();

    ly_ctx_destroy(*ctx);
    *ctx = ctx_new;

cleanup:
    free_cmdline(argv);
    free(searchpaths);
}
