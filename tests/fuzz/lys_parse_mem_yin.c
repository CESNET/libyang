/**
 * @file lys_parse_mem_yin.c
 * @author OSS-Fuzz coverage pipeline
 * @brief fuzzing harness for lys_parse_mem() with LYS_IN_YIN input format.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "libyang.h"
#include "tests_config.h"

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
    struct ly_ctx *ctx = NULL;
    static bool log = false;
    char *data = NULL;
    LY_ERR err;

    if (!log) {
        ly_log_options(0);
        log = true;
    }

    err = ly_ctx_new(TESTS_SRC "/../modules", 0, &ctx);
    if (err != LY_SUCCESS) {
        return 0;
    }

    data = malloc(len + 1);
    if (data == NULL) {
        ly_ctx_destroy(ctx);
        return 0;
    }

    memcpy(data, buf, len);
    data[len] = 0;

    lys_parse_mem(ctx, data, LYS_IN_YIN, NULL);
    ly_ctx_destroy(ctx);
    free(data);
    return 0;
}
