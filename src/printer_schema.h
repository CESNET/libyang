/**
 * @file printer_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema printers for libyang
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_SCHEMA_H_
#define LY_PRINTER_SCHEMA_H_

#include <stdio.h>
#include <unistd.h>

#include "tree_schema.h"

/**
 * @brief Print schema tree in the specified format into a memory block.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file descriptor.
 *
 * @param[in] module Schema tree to print.
 * @param[in] fd File descriptor where to print the data.
 * @param[in] format Schema output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file stream.
 *
 * @param[in] module Schema tree to print.
 * @param[in] f File stream where to print the schema.
 * @param[in] format Schema output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file.
 *
 * @param[in] path File where to print the schema.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Print schema tree in the specified format using a provided callback.
 *
 * @param[in] module Schema tree to print.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Schema output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                     const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

#endif /* LY_PRINTER_SCHEMA_H_ */
