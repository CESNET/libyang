/**
 * @file lcbor.h
 * @author MeherRushi <meherrrushi2@gmail.com>
 * @brief CBOR data parser routines for libyang (abstraction over libcbor)
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_CBOR_H_
#define LY_CBOR_H_

#include <stddef.h>
#include <stdint.h>
/* using libcbor as the low-level parser */
#include <cbor.h>


#include "log.h"
#include "set.h"

struct ly_ctx;
struct ly_in;

/**
 * @brief CBOR format variants for different encoding schemes
 */
enum lyd_cbor_format
{
    LYD_CBOR_NAMED, /**< CBOR with named identifiers (JSON-like) */
    LYD_CBOR_SID    /**< CBOR with Schema Item identifiers (future implementation) */
};

struct lycbor_ctx {
    const struct ly_ctx *ctx; /**< libyang context */
    struct ly_in *in;   /**< input structure */
    
    cbor_item_t *cbor_data; /**< parsed CBOR data */
    
    enum lyd_cbor_format format; /**< CBOR format variant */
    
    struct {
        cbor_item_t *cbor_data; /**< parsed CBOR data */
        enum lyd_cbor_format format; /**< CBOR format variant */
        const char *input;
    } backup;
};

/**
 * @brief Get a human-readable name for a CBOR type.
 *
 * @param[in] cbortype CBOR type.
 * @return String representation of the CBOR type.
 */
const char *lycbor_token2str(enum cbor_type cbortype);

/**
 * @brief Create new CBOR context for parsing.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input handler.
 * @param[out] cborctx_p Pointer to store the created CBOR context.
 * @return LY_ERR value.
 */
LY_ERR
lycbor_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lycbor_ctx **cborctx_p);

/**
 * @brief Free CBOR context.
 *
 * @param[in] cborctx CBOR context to free.
 */
void
lycbor_ctx_free(struct lycbor_ctx *cborctx);

#endif /* LY_CBOR_H_ */
