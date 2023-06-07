/**
 * @file cmd_debug.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'verb' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2023-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef NDEBUG

#include "cmd.h"

#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

void
cmd_debug_help(void)
{
    printf("Usage: debug (dict | xpath | dep-sets)+\n");
}

int
cmd_debug_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_DEBUG].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_debug_help();
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
cmd_debug_dep(struct yl_opt *yo, int posc)
{
    (void) yo;

    if (!posc) {
        /* no argument */
        cmd_debug_help();
        return 1;
    }

    return 0;
}

int
cmd_debug_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) ctx;

    assert(posv);

    if (!strcasecmp("dict", posv)) {
        yo->dbg_groups |= LY_LDGDICT;
    } else if (!strcasecmp("xpath", posv)) {
        yo->dbg_groups |= LY_LDGXPATH;
    } else if (!strcasecmp("dep-sets", posv)) {
        yo->dbg_groups |= LY_LDGDEPSETS;
    } else {
        YLMSG_E("Unknown debug group \"%s\"\n", posv);
        return 1;
    }

    return 0;
}

int
cmd_debug_fin(struct ly_ctx *ctx, struct yl_opt *yo)
{
    (void) ctx;
    return ly_log_dbg_groups(yo->dbg_groups);
}

#endif
