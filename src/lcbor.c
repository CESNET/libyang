/**
 * @file lcbor.c
 * @author MeherRushi <meherrrushi2@gmail.com>
 * @brief CBOR data parser for libyang (abstraction over libcbor)
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include "in_internal.h"
#include "lcbor.h"
#include "log.h"
#include "ly_common.h"

const char *
lycbor_token2str(enum cbor_type cbortype)
{
    switch (cbortype) {
    case CBOR_TYPE_UINT:
        return "unsigned integer";
    case CBOR_TYPE_NEGINT:
        return "negative integer";
    case CBOR_TYPE_BYTESTRING:
        return "byte string";
    case CBOR_TYPE_STRING:
        return "string";
    case CBOR_TYPE_ARRAY:
        return "array";
    case CBOR_TYPE_MAP:
        return "map";
    case CBOR_TYPE_TAG:
        return "tag";
    case CBOR_TYPE_FLOAT_CTRL:
        return "decimals and special values (true, false, nil, ...)";
    }

    return "";
}

void
lycbor_ctx_free(struct lycbor_ctx *cborctx)
{
    if (cborctx) {
        if (cborctx->cbor_data) {
            cbor_decref(&cborctx->cbor_data);
        }
        free(cborctx);
    }
}

/**
 * @brief Detect CBOR format variant from input data.
 *
 * @param[in] in Input structure to analyze.
 * @param[out] format Detected format.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_detect_format(struct ly_in *in, enum lyd_cbor_format *format)
{
    /* Simple heuristic: try to parse as CBOR and examine structure */
    /* For now, default to named format */
    (void)in;
    *format = LYD_CBOR_NAMED;
    return LY_SUCCESS;
}

LY_ERR
lycbor_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lycbor_ctx **cborctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lycbor_ctx *cborctx;
    struct cbor_load_result result = {0};
    enum lyd_cbor_format format;

    assert(ctx && in && cborctx_p);

    LY_CHECK_RET(lydcbor_detect_format(in, &format));

    /* Allocate and initialize CBOR context */
    cborctx = calloc(1, sizeof *cborctx);
    LY_CHECK_ERR_RET(!cborctx, LOGMEM(ctx), LY_EMEM);
    cborctx->ctx = ctx;
    cborctx->in = in;
    cborctx->format = format;

    /* input line logging */
    ly_log_location(NULL, NULL, in);

    /* load and parse CBOR data */
    cborctx->cbor_data = cbor_load((cbor_data)in->current, in->length, &result);
    if (!cborctx->cbor_data) {
        LOGVAL(ctx, NULL, LYVE_SYNTAX, "Failed to parse CBOR data.");
        free(cborctx);
        return LY_EVALID;
    }
    if (result.error.code != CBOR_ERR_NONE) {
        LOGVAL(ctx, NULL, LYVE_SYNTAX, "CBOR parsing error (code %d).", result.error.code);
        cbor_decref(&cborctx->cbor_data);
        free(cborctx);
        return LY_EVALID;
    }

    *cborctx_p = cborctx;
    return ret;
}
