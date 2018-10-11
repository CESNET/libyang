/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "libyang.h"
#include "common.h"

API void
lysp_module_free(struct lysp_module *module)
{
    struct ly_ctx *ctx;

    LY_CHECK_ARG_RET(NULL, module,);
    ctx = module->ctx;

    lydict_remove(ctx, module->name);
    lydict_remove(ctx, module->ns);
    lydict_remove(ctx, module->prefix);

    free(module);
}
