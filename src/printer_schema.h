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

#include "printer.h"
#include "tree_schema.h"


/**
 * @defgroup schemaprinterflags Schema output options
 * @ingroup schema
 *
 * @{
 */
#define LYS_OUTPUT_NO_SUBSTMT        0x10 /**< Print only top-level/referede node information,
                                               do not print information from the substatements */
//#define LYS_OUTOPT_TREE_RFC        0x01 /**< Conform to the RFC TODO tree output (only for tree format) */
//#define LYS_OUTOPT_TREE_GROUPING   0x02 /**< Print groupings separately (only for tree format) */
//#define LYS_OUTOPT_TREE_USES       0x04 /**< Print only uses instead the resolved grouping nodes (only for tree format) */
//#define LYS_OUTOPT_TREE_NO_LEAFREF 0x08 /**< Do not print the target of leafrefs (only for tree format) */

/**
 * @} schemaprinterflags
 */

/**
 * @brief Schema module printer.
 *
 * @param[in] out Printer handler for a specific output. Use lyp_*() functions to create the handler and lyp_free() to remove the handler.
 * @param[in] module Schema to print.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print(struct lyp_out *out, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options);

/**
 * @brief Schema node printer.
 *
 * @param[in] out Printer handler for a specific output. Use lyp_*() functions to create the handler and lyp_free() to remove the handler.
 * @param[in] node Schema node to print, lys_find_node() can be used to get it from a path string.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return Number of printed bytes in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lys_print_node(struct lyp_out *out, const struct lysc_node *node, LYS_OUTFORMAT format, int line_length, int options);

#endif /* LY_PRINTER_SCHEMA_H_ */
