/**
 * @file printer_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Internal structures and functions for libyang
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_INTERNAL_H_
#define LY_PRINTER_INTERNAL_H_

#include "printer_schema.h"

typedef enum LYOUT_TYPE {
    LYOUT_FD,          /**< file descriptor */
    LYOUT_STREAM,      /**< FILE stream */
    LYOUT_FDSTREAM,    /**< FILE stream based on duplicated file descriptor */
    LYOUT_MEMORY,      /**< memory */
    LYOUT_CALLBACK     /**< print via provided callback */
} LYOUT_TYPE;

struct lyout {
    LYOUT_TYPE type;
    union {
        int fd;
        FILE *f;
        struct {
            char *buf;
            size_t len;
            size_t size;
        } mem;
        struct {
            ssize_t (*f)(void *arg, const void *buf, size_t count);
            void *arg;
        } clb;
    } method;

    /* buffer for holes */
    char *buffered;
    size_t buf_len;
    size_t buf_size;

    /* hole counter */
    size_t hole_count;

    /* counter for printed bytes */
    size_t printed;

    /* libyang context for error logging */
    struct ly_ctx *ctx;
    LY_ERR status;
};

#define LYOUT_CHECK(LYOUT, ...) if (LYOUT->status) {return __VA_ARGS__;}

struct ext_substmt_info_s {
    const char *name;
    const char *arg;
    int flags;
#define SUBST_FLAG_YIN 0x1 /**< has YIN element */
#define SUBST_FLAG_ID 0x2  /**< the value is identifier -> no quotes */
};

/* filled in printer.c */
extern struct ext_substmt_info_s ext_substmt_info[];


/**
 * @brief
 */
LY_ERR yang_print_parsed(struct lyout *out, const struct lys_module *module);

/**
 * @brief
 */
LY_ERR yang_print_compiled(struct lyout *out, const struct lys_module *module);

LY_ERR ly_print(struct lyout *out, const char *format, ...);

void ly_print_flush(struct lyout *out);

LY_ERR ly_write(struct lyout *out, const char *buf, size_t count);

#endif /* LY_PRINTER_INTERNAL_H_ */
