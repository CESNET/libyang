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
 * @defgroup schemaprinterflags Schema output options
 * @ingroup schema
 *
 * @{
 */
#define LYS_OUTPUT_NO_SUBST          0x10 /**< Print only top-level/referede node information,
                                               do not print information from the substatements */
//#define LYS_OUTOPT_TREE_RFC        0x01 /**< Conform to the RFC TODO tree output (only for tree format) */
//#define LYS_OUTOPT_TREE_GROUPING   0x02 /**< Print groupings separately (only for tree format) */
//#define LYS_OUTOPT_TREE_USES       0x04 /**< Print only uses instead the resolved grouping nodes (only for tree format) */
//#define LYS_OUTOPT_TREE_NO_LEAFREF 0x08 /**< Do not print the target of leafrefs (only for tree format) */

/**
 * @} schemaprinterflags
 */


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
 * @brief Print schema node in the specified format into a memory block.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] ctx libyang context to get schema node in case of missing @p context_node.
 * @param[in] context_node Context node in case of the relative @p target_node path.
 * @param[in] format Schema output format.
 * @param[in] target_node Schema node path, use full module names as node's prefixes.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_node_print_mem(char **strp, struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                           const char *target_node, int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file descriptor.
 *
 * @param[in] fd File descriptor where to print the data.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file descriptor.
 *
 * @param[in] fd File descriptor where to print the data.
 * @param[in] ctx libyang context to get schema node in case of missing @p context_node.
 * @param[in] context_node Context node in case of the relative @p target_node path.
 * @param[in] format Schema output format.
 * @param[in] target_node Schema node path, use full module names as node's prefixes.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_node_print_fd(int fd, struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                          const char *target_node, int line_length, int options);

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
ssize_t lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format,
                       int line_length, int options);

/**
 * @brief Print schema node in the specified format into a file stream.
 *
 * @param[in] f File stream where to print the schema.
 * @param[in] ctx libyang context to get schema node in case of missing @p context_node.
 * @param[in] context_node Context node in case of the relative @p target_node path.
 * @param[in] format Schema output format.
 * @param[in] target_node Schema node path, use full module names as node's prefixes.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_node_print_file(FILE *f, struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                            const char *target_node, int line_length, int options);

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
 * @brief Print schema tree in the specified format into a file.
 *
 * @param[in] path File where to print the schema.
 * @param[in] ctx libyang context to get schema node in case of missing @p context_node.
 * @param[in] context_node Context node in case of the relative @p target_node path.
 * @param[in] format Schema output format.
 * @param[in] target_node Schema node path, use full module names as node's prefixes.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_node_print_path(const char *path,  struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                            const char *target_node, int line_length, int options);

/**
 * @brief Print schema tree in the specified format using a provided callback.
 *
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                     const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Print schema node in the specified format using a provided callback.
 *
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] ctx libyang context to get schema node in case of missing @p context_node.
 * @param[in] context_node Context node in case of the relative @p target_node path.
 * @param[in] format Schema output format.
 * @param[in] target_node Schema node path, use full module names as node's prefixes.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_node_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                           struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                           const char *target_node, int line_length, int options);

#endif /* LY_PRINTER_SCHEMA_H_ */
