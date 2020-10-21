/**
 * @file in_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Internal structures and functions for libyang parsers
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_IN_INTERNAL_H_
#define LY_IN_INTERNAL_H_

#include "in.h"
#include "tree_schema_internal.h"

/**
 * @brief Parser input structure specifying where the data are read.
 */
struct ly_in {
    LY_IN_TYPE type;        /**< type of the output to select the output method */
    const char *current;    /**< Current position in the input data */
    const char *func_start; /**< Input data position when the last parser function was executed */
    const char *start;      /**< Input data start */
    size_t length;          /**< mmap() length (if used) */
    union {
        int fd;             /**< file descriptor for LY_IN_FD type */
        FILE *f;            /**< file structure for LY_IN_FILE and LY_IN_FILEPATH types */
        struct {
            int fd;         /**< file descriptor for LY_IN_FILEPATH */
            char *filepath; /**< stored original filepath */
        } fpath;            /**< filepath structure for LY_IN_FILEPATH */
    } method;               /**< type-specific information about the output */
};

/**
 * @brief Read bytes from an input.
 *
 * @param[in] in Input structure.
 * @param[in] buf Destination buffer.
 * @param[in] count Number of bytes to read.
 * @return LY_SUCCESS on success,
 * @return LY_EDENIED on EOF.
 */
LY_ERR ly_in_read(struct ly_in *in, void *buf, size_t count);

/**
 * @brief Just skip bytes in an input.
 *
 * @param[in] in Input structure.
 * @param[in] count Number of bytes to skip.
 * @return LY_SUCCESS on success,
 * @return LY_EDENIED on EOF.
 */
LY_ERR ly_in_skip(struct ly_in *in, size_t count);

#endif /* LY_IN_INTERNAL_H_ */
