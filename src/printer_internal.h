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
#include "printer_data.h"

/**
 * @brief Types of the printer's output
 */
typedef enum LYOUT_TYPE {
    LYOUT_FD,          /**< file descriptor */
    LYOUT_STREAM,      /**< FILE stream */
    LYOUT_FDSTREAM,    /**< FILE stream based on duplicated file descriptor */
    LYOUT_MEMORY,      /**< memory */
    LYOUT_CALLBACK     /**< print via provided callback */
} LYOUT_TYPE;

/**
 * @brief Printer output structure specifying where the data are printed.
 */
struct lyout {
    LYOUT_TYPE type;     /**< type of the output to select the output method */
    union {
        int fd;          /**< file descriptor for LYOUT_FD type */
        FILE *f;         /**< file structure for LYOUT_STREAM and LYOUT_FDSTREAM types */
        struct {
            char *buf;        /**< pointer to the memory buffer to store the output */
            size_t len;       /**< number of used bytes in the buffer */
            size_t size;      /**< allocated size of the buffer */
        } mem;           /**< memory buffer information for LYOUT_MEMORY type */
        struct {
            ssize_t (*f)(void *arg, const void *buf, size_t count); /**< callback function */
            void *arg;        /**< optional argument for the callback function */
        } clb;           /**< printer callback for LYOUT_CALLBACK type */
    } method;            /**< type-specific information about the output */

    char *buffered;      /**< additional buffer for holes, used only for LYB data format */
    size_t buf_len;      /**< number of used bytes in the additional buffer for holes, used only for LYB data format */
    size_t buf_size;     /**< allocated size of the buffer for holes, used only for LYB data format */
    size_t hole_count;   /**< hole counter, used only for LYB data format */

    size_t printed;      /**< Number of printed bytes */

    const struct ly_ctx *ctx;   /**< libyang context for error logging */
    LY_ERR status;       /**< current status of the printer */
};

/**
 * @brief Informational structure for YANG statements
 */
struct ext_substmt_info_s {
    const char *name;      /**< name of the statement */
    const char *arg;       /**< name of YIN's attribute to present the statement */
    int flags;             /**< various flags to clarify printing of the statement */
#define SUBST_FLAG_YIN 0x1 /**< has YIN element */
#define SUBST_FLAG_ID 0x2  /**< the value is identifier -> no quotes */
};

/* filled in printer.c */
extern struct ext_substmt_info_s ext_substmt_info[];

/**
 * @brief macro to check current status of the printer.
 */
#define LYOUT_CHECK(LYOUT, ...) if (LYOUT->status) {return __VA_ARGS__;}

/**
 * @brief YANG printer of the parsed schemas. Full YANG printer.
 *
 * @param[in] out Output specification.
 * @param[in] module Schema to be printed (the parsed member is used).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR yang_print_parsed(struct lyout *out, const struct lys_module *module);

/**
 * @brief YANG printer of the compiled schemas.
 *
 * This printer provides information about modules how they are understood by libyang.
 * Despite the format is inspired by YANG, it is not fully compatible and should not be
 * used as a standard YANG format.
 *
 * @param[in] out Output specification.
 * @param[in] module Schema to be printed (the compiled member is used).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR yang_print_compiled(struct lyout *out, const struct lys_module *module);

/**
 * @brief YIN printer of the parsed schemas. Full YIN printer.
 *
 * @param[in] out Output specification.
 * @param[in] module Schema to be printed (the parsed member is used).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR yin_print_parsed(struct lyout *out, const struct lys_module *module);

/**
 * @brief XML printer of the YANG data.
 *
 * @param[in] out Output specification.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR xml_print_data(struct lyout *out, const struct lyd_node *root, int options);

/**
 * @brief Check whether a node value equals to its default one.
 *
 * @param[in] node Term node to test.
 * @return 0 if no,
 * @return non-zero if yes.
 */
int ly_is_default(const struct lyd_node *node);

/**
 * @brief Check whether the node should even be printed.
 *
 * @param[in] node Node to check.
 * @param[in] options Printer options.
 * @return 0 if no.
 * @return non-zero if yes.
 */
int ly_should_print(const struct lyd_node *node, int options);

/**
 * @brief Generic printer of the given format string into the specified output.
 *
 * Alternatively, ly_write() can be used.
 *
 * @param[in] out Output specification.
 * @param[in] format format string to be printed.
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR ly_print(struct lyout *out, const char *format, ...);

/**
 * @brief Flush the output from any internal buffers and clean any auxiliary data.
 * @param[in] out Output specification.
 */
void ly_print_flush(struct lyout *out);

/**
 * @brief Generic printer of the given string buffer into the specified output.
 *
 * Alternatively, ly_print() can be used.
 *
 * As an extension for printing holes (skipping some data until they are known),
 * ly_write_skip() and ly_write_skipped() can be used.
 *
 * @param[in] out Output specification.
 * @param[in] buf Memory buffer with the data to print.
 * @param[in] len Length of the data to print in the @p buf.
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR ly_write(struct lyout *out, const char *buf, size_t len);

/**
 * @brief Create a hole in the output data that will be filled later.
 *
 * @param[in] out Output specification.
 * @param[in] len Length of the created hole.
 * @param[out] position Position of the hole, value must be later provided to the ly_write_skipped() call.
 * @return LY_ERR value. The number of the printed bytes is updated in lyout::printed
 * only in case the data are really written into the output.
 */
LY_ERR ly_write_skip(struct lyout *out, size_t len, size_t *position);

/**
 * @brief Write data into the hole at given position.
 *
 * @param[in] out Output specification.
 * @param[in] position Position of the hole to fill, the value was provided by ly_write_skip().
 * @param[in] buf Memory buffer with the data to print.
 * @param[in] len Length of the data to print in the @p buf. Not that the length must correspond
 * to the len value specified in the corresponding ly_write_skip() call.
 * @return LY_ERR value. The number of the printed bytes is updated in lyout::printed
 * only in case the data are really written into the output.
 */
LY_ERR ly_write_skipped(struct lyout *out, size_t position, const char *buf, size_t len);

#endif /* LY_PRINTER_INTERNAL_H_ */
