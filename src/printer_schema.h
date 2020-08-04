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

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_out;
struct lys_module;
struct lysc_node;
struct lysp_submodule;

/**
 * @addtogroup schematree
 * @{
 */

/**
 * @defgroup schemaprinterflags Schema output options
 * @{
 */
/* Keep the value 0x02 reserver for implicit LYS_PRINT_FORMAT */
#define LYS_OUTPUT_NO_SUBSTMT        0x10 /**< Print only top-level/referede node information,
                                               do not print information from the substatements */
//#define LYS_OUTOPT_TREE_RFC        0x01 /**< Conform to the RFC TODO tree output (only for tree format) */
//#define LYS_OUTOPT_TREE_GROUPING   0x02 /**< Print groupings separately (only for tree format) */
//#define LYS_OUTOPT_TREE_USES       0x04 /**< Print only uses instead the resolved grouping nodes (only for tree format) */
//#define LYS_OUTOPT_TREE_NO_LEAFREF 0x08 /**< Do not print the target of leafrefs (only for tree format) */

/** @} schemaprinterflags */

/**
 * @brief Schema output formats accepted by libyang [printer functions](@ref howtoschemasprinters).
 */
typedef enum {
    LYS_OUT_UNKNOWN = 0, /**< unknown format, used as return value in case of error */
    LYS_OUT_YANG = 1,    /**< YANG schema output format */
    LYS_OUT_YANG_COMPILED = 2, /**< YANG schema output format of the compiled schema tree */
    LYS_OUT_YIN = 3,     /**< YIN schema output format */

    LYS_OUT_TREE,        /**< Tree schema output format, for more information see the [printers](@ref howtoschemasprinters) page */
} LYS_OUTFORMAT;

/**
 * @brief Schema module printer.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] module Main module with the parsed schema to print.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_module(struct ly_out *out, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Schema submodule printer.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] module Main module of the submodule to print.
 * @param[in] submodule Parsed submodule to print.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_submodule(struct ly_out *out, const struct lys_module *module, const struct lysp_submodule *submodule,
                           LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a memory block.
 * It is up to caller to free the returned string by free().
 *
 * This is just a wrapper around lys_print() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, int options);

/**
 * @brief Print schema tree in the specified format into a file descriptor.
 *
 * This is just a wrapper around lys_print() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] fd File descriptor where to print the data.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, int options);

/**
 * @brief Print schema tree in the specified format into a file stream.
 *
 * This is just a wrapper around lys_print() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] module Schema tree to print.
 * @param[in] f File stream where to print the schema.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, int options);

/**
 * @brief Print schema tree in the specified format into a file.
 *
 * This is just a wrapper around lys_print() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] path File where to print the schema.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, int options);

/**
 * @brief Print schema tree in the specified format using a provided callback.
 *
 * This is just a wrapper around lys_print() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] module Schema tree to print.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                     const struct lys_module *module, LYS_OUTFORMAT format, int options);

/**
 * @brief Schema node printer.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] node Schema node to print, lys_find_node() can be used to get it from a path string.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_node(struct ly_out *out, const struct lysc_node *node, LYS_OUTFORMAT format, int line_length, int options);

/** @} schematree */

#ifdef __cplusplus
}
#endif

#endif /* LY_PRINTER_SCHEMA_H_ */
