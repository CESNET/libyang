/**
 * @file lcbor.h
 * @author MeherRushi <meherrrushi2@gmail.com>
 * @brief CBOR data parser for libyang (abstraction over libcbor)
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifdef ENABLE_CBOR_SUPPORT

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

        return "object";
    case LYJSON_OBJECT_NEXT:
        return "object next";
    case LYJSON_OBJECT_CLOSED:
        return "object closed";
    case LYJSON_ARRAY:
        return "array";
    case LYJSON_ARRAY_NEXT:
        return "array next";
    case LYJSON_ARRAY_CLOSED:
        return "array closed";
    case LYJSON_OBJECT_NAME:
        return "object name";
    case LYJSON_NUMBER:
        return "number";
    case LYJSON_STRING:
        return "string";
    case LYJSON_TRUE:
        return "true";
    case LYJSON_FALSE:
        return "false";
    case LYJSON_NULL:
        return "null";
    case LYJSON_END:
        return "end of input";
    }

    return "";
}

/**
 * @brief Free CBOR context.
 *
 * @param[in] cborctx CBOR context to free.
 */
void lycbor_ctx_free(struct lycbor_ctx *cborctx)
{
    if (cborctx)
    {
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

/**
 * @brief Create new CBOR context for parsing.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input handler.
 * @param[out] cborctx_p Pointer to store the created CBOR context.
 * @return LY_ERR value.
 */
LY_ERR
lycbor_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lycbor_ctx **cborctx_p)
{
    /* TODO : Need to restructure error handling here */
    LY_ERR ret = LY_SUCCESS;
    struct lycbor_ctx *cborctx;
    struct cbor_load_result result = {0};
    enum lyd_cbor_format format;

    assert(ctx && in && cborctx_p);

    /* TODO : error handling after the detect_format function call */
    ret = lydcbor_detect_format(in, &format);

    /* Allocate and initialize CBOR context */
    cborctx = calloc(1, sizeof *cborctx);
    LY_CHECK_ERR_RET(!cborctx, LOGMEM(ctx), LY_EMEM);
    cborctx->ctx = ctx;
    cborctx->in = in;
    cborctx->format = format;

    /* load and parse CBOR data */
    cborctx->cbor_data = cbor_load(in->current, in->length, &result);
    if (!cborctx->cbor_data) {
        LOGVAL(ctx, LYVE_SYNTAX, "Failed to parse CBOR data.");
        free(cborctx);
        return LY_EVALID;
    }
    if (result.error.code != CBOR_ERR_NONE) {
        LOGVAL(ctx, LYVE_SYNTAX, "CBOR parsing error (code %d).", result.error.code);
        cbor_decref(&cborctx->cbor_data);
        free(cborctx);
        return LY_EVALID;
    }

    /* input line logging */
    ly_log_location(NULL, NULL, NULL, in);

    *cborctx_p = cborctx;
    return ret;
}

#endif /* ENABLE_CBOR_SUPPORT */