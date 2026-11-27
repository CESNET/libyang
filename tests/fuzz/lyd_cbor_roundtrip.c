/**
 * @file lyd_cbor_roundtrip.c
 * @author OSS-Fuzz coverage pipeline
 * @brief fuzzing harness for the CBOR instance data format round trip.
 *
 * The fuzzed input is parsed as XML instance data against a fixed schema with
 * diverse node types, printed as YANG-CBOR (src/printer_cbor.c) and re-parsed
 * from the printed bytes (src/parser_cbor.c, src/lcbor.c), covering the CBOR
 * printer and deserializer on printer-valid structures. Requires the build to
 * have CBOR support enabled (libcbor present, ENABLE_CBOR_SUPPORT defined).
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
        "module fuzz-cbor {namespace urn:tests:fuzz-cbor;prefix x;"
        "container c {"
        "leaf s {type string;}"
        "leaf i {type int64;}"
        "leaf u {type uint64;}"
        "leaf b {type boolean;}"
        "leaf f {type decimal64 {fraction-digits 3;}}"
        "leaf-list ll {type int16;}"
        "list l {key k; leaf k {type string;} leaf v {type binary;}}"
        "leaf empty {type empty;}"
        "}}";

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
    struct ly_ctx *ctx = NULL;
    struct lyd_node *tree = NULL;
    struct lyd_node *tree2 = NULL;
    static bool log = false;
    char *data = NULL;
    char *cbor = NULL;
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

    data = malloc(len + 1);
    if (data == NULL) {
        goto cleanup;
    }
    memcpy(data, buf, len);
    data[len] = 0;

    if (lyd_parse_data_mem(ctx, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree) != LY_SUCCESS) {
        goto cleanup;
    }

    if (lyd_print_mem(&cbor, tree, LYD_CBOR, 0) != LY_SUCCESS) {
        goto cleanup;
    }

    lyd_parse_data_mem(ctx, cbor, LYD_CBOR, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree2);

cleanup:
    free(data);
    free(cbor);
    lyd_free_all(tree);
    lyd_free_all(tree2);
    ly_ctx_destroy(ctx);
    return 0;
}
