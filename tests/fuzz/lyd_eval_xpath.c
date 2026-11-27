/**
 * @file lyd_eval_xpath.c
 * @author OSS-Fuzz coverage pipeline
 * @brief fuzzing harness for lyd_eval_xpath() over a fixed data tree.
 *
 * The fuzzed input is used as an XPath expression evaluated against a small
 * static data tree, exercising the XPath expression parser and evaluator
 * (src/xpath.c, src/path.c) on attacker-controlled strings.
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

static const char *schema =
        "module fuzz-xp {namespace urn:tests:fuzz-xp;prefix x;"
        "container c {list l {key k; leaf k {type string;} leaf v {type int32;}}"
        "leaf-list s {type string;} leaf b {type boolean;}}}";
static const char *dataxml =
        "<c xmlns=\"urn:tests:fuzz-xp\">"
        "<l><k>a</k><v>1</v></l>"
        "<l><k>b</k><v>2</v></l>"
        "<s>x</s><s>y</s>"
        "<b>true</b>"
        "</c>";

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
    struct ly_ctx *ctx = NULL;
    struct lyd_node *tree = NULL;
    static bool log = false;
    char *data = NULL;
    ly_bool result = false;
    LY_ERR err;

    if (!log) {
        ly_log_options(0);
        log = true;
    }

    err = ly_ctx_new(TESTS_SRC "/../modules", 0, &ctx);
    if (err != LY_SUCCESS) {
        return 0;
    }

    if (lys_parse_mem(ctx, schema, LYS_IN_YANG, NULL) != LY_SUCCESS) {
        goto cleanup;
    }
    if (lyd_parse_data_mem(ctx, dataxml, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree) != LY_SUCCESS) {
        goto cleanup;
    }

    data = malloc(len + 1);
    if (data == NULL) {
        goto cleanup;
    }
    memcpy(data, buf, len);
    data[len] = 0;

    lyd_eval_xpath(tree, data, &result);

cleanup:
    free(data);
    lyd_free_all(tree);
    ly_ctx_destroy(ctx);
    return 0;
}
