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

#include "printer.h"
#include "printer_schema.h"
#include "printer_data.h"

/**
 * @brief Printer output structure specifying where the data are printed.
 */
struct ly_out {
    LY_OUT_TYPE type;     /**< type of the output to select the output method */
    union {
        int fd;          /**< file descriptor for LY_OUT_FD type */
        FILE *f;         /**< file structure for LY_OUT_FILE, LY_OUT_FDSTREAM and LY_OUT_FILEPATH types */
        struct {
            FILE *f;          /**< file stream from the original file descriptor, variable is mapped to the LY_OUT_FILE's f */
            int fd;           /**< original file descriptor, which was not used directly because of missing vdprintf() */
        } fdstream;      /**< structure for LY_OUT_FDSTREAM type, which is LY_OUT_FD when vdprintf() is missing */
        struct {
            FILE *f;          /**< file structure for LY_OUT_FILEPATH, variable is mapped to the LY_OUT_FILE's f */
            char *filepath;   /**< stored original filepath */
        } fpath;         /**< filepath structure for LY_OUT_FILEPATH */
        struct {
            char **buf;       /**< storage for the pointer to the memory buffer to store the output */
            size_t len;       /**< number of used bytes in the buffer */
            size_t size;      /**< allocated size of the buffer */
        } mem;           /**< memory buffer information for LY_OUT_MEMORY type */
        struct {
            ssize_t (*func)(void *arg, const void *buf, size_t count); /**< callback function */
            void *arg;        /**< optional argument for the callback function */
        } clb;           /**< printer callback for LY_OUT_CALLBACK type */
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
LY_ERR yang_print_parsed(struct ly_out *out, const struct lys_module *module);

/**
 * @brief YANG printer of the compiled schemas.
 *
 * This printer provides information about modules how they are understood by libyang.
 * Despite the format is inspired by YANG, it is not fully compatible and should not be
 * used as a standard YANG format.
 *
 * @param[in] out Output specification.
 * @param[in] module Schema to be printed (the compiled member is used).
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR yang_print_compiled(struct ly_out *out, const struct lys_module *module, int options);

/**
 * @brief YANG printer of the compiled schema node
 *
 * This printer provides information about modules how they are understood by libyang.
 * Despite the format is inspired by YANG, it is not fully compatible and should not be
 * used as a standard YANG format.
 *
 * @param[in] out Output specification.
 * @param[in] node Schema node to be printed including all its substatements.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR yang_print_compiled_node(struct ly_out *out, const struct lysc_node *node, int options);

/**
 * @brief YIN printer of the parsed schemas. Full YIN printer.
 *
 * @param[in] out Output specification.
 * @param[in] module Schema to be printed (the parsed member is used).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR yin_print_parsed(struct ly_out *out, const struct lys_module *module);

/**
 * @brief XML printer of the YANG data.
 *
 * @param[in] out Output specification.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
LY_ERR xml_print_data(struct ly_out *out, const struct lyd_node *root, int options);

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
 * @brief Flush the output from any internal buffers and clean any auxiliary data.
 * @param[in] out Output specification.
 */
void ly_print_flush(struct ly_out *out);

/**
 * @brief Create a hole in the output data that will be filled later.
 *
 * @param[in] out Output specification.
 * @param[in] len Length of the created hole.
 * @param[out] position Position of the hole, value must be later provided to the ly_write_skipped() call.
 * @return The number of bytes prepared for write. The number of the printed bytes is updated in lyout::printed
 * only in case the data are really written into the output.
 * @return Negative value in case of error, absolute value of the return code maps to LY_ERR value.
 */
ssize_t ly_write_skip(struct ly_out *out, size_t len, size_t *position);

/**
 * @brief Write data into the hole at given position.
 *
 * @param[in] out Output specification.
 * @param[in] position Position of the hole to fill, the value was provided by ly_write_skip().
 * @param[in] buf Memory buffer with the data to print.
 * @param[in] len Length of the data to print in the @p buf. Not that the length must correspond
 * to the len value specified in the corresponding ly_write_skip() call.
 * @return The number of bytes prepared for write. The number of the printed bytes is updated in lyout::printed
 * only in case the data are really written into the output.
 * @return Negative value in case of error, absolute value of the return code maps to LY_ERR value.
 */
ssize_t ly_write_skipped(struct ly_out *out, size_t position, const char *buf, size_t len);

#endif /* LY_PRINTER_INTERNAL_H_ */
