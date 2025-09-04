/**
 * @file cmd_cmp.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief schema comparison of the libyang's yanglint tool.
 *
 * Copyright (c) 2025 CESNET, z.s.p.o.
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

/* only the non-interactive variant implemented */

static int
cmp_ctx_dup(const struct ly_ctx *ctx, struct yl_opt *yo, struct ly_ctx **dup)
{
    uint32_t idx;
    const struct lys_module *mod;

    /* create a new context */
    if (ly_ctx_new(yo->searchpaths, yo->ctx_options, dup)) {
        YLMSG_E("Unable to create libyang context.");
        return -1;
    }

    /* add all the implemented modules */
    idx = ly_ctx_internal_modules_count(ctx);
    while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
        if (!mod->implemented) {
            continue;
        }

        if (cmd_add_exec(dup, yo, mod->filepath)) {
            return -1;
        }
    }

    return 0;
}

int
cmd_cmp_exec(struct ly_ctx **ctx, struct yl_opt *yo)
{
    int rc = 0;
    struct ly_ctx *ctx2 = NULL;
    const struct lys_module *mod1, *mod2;
    struct lyd_node *data = NULL;

    /* create the second context */
    if ((rc = cmp_ctx_dup(*ctx, yo, &ctx2))) {
        goto cleanup;
    }

    /* add the compared modules to their contexts */
    if (cmd_add_exec(ctx, yo, yo->cmp_mod_path1)) {
        rc = -1;
        goto cleanup;
    }
    if (cmd_add_exec(&ctx2, yo, yo->cmp_mod_path2)) {
        rc = -1;
        goto cleanup;
    }

    /* get the 2 modules */
    mod1 = yo->schema_modules.objs[yo->schema_modules.count - 2];
    mod2 = yo->schema_modules.objs[yo->schema_modules.count - 1];

    /* generate comparison data */
    if (lysc_compare(*ctx, mod1, mod2, &data)) {
        rc = -1;
        goto cleanup;
    }

    if (yo->data_out_format) {
        /* print them */
        if (lyd_print_all(yo->out, data, yo->data_out_format, yo->data_print_options)) {
            rc = -1;
            goto cleanup;
        }
    }

cleanup:
    lyd_free_siblings(data);
    ly_ctx_destroy(ctx2);
    return rc;
}
