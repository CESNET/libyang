/**
 * @file printer_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data printers for libyang
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_DATA_H_
#define LY_PRINTER_DATA_H_

#include <unistd.h>

#include "tree_data.h"

struct ly_out;

/**
 * @defgroup dataprinterflags Data printer flags
 * @ingroup datatree
 *
 * Validity flags for data nodes.
 *
 * @{
 */
#define LYDP_WITHSIBLINGS  0x01  /**< Flag for printing also the (following) sibling nodes of the data node. */
#define LYDP_FORMAT        0x02  /**< Flag for formatted output. */
#define LYDP_KEEPEMPTYCONT 0x04  /**< Preserve empty non-presence containers */
#define LYDP_WD_MASK       0xF0  /**< Mask for with-defaults modes */
#define LYDP_WD_EXPLICIT   0x00  /**< Explicit mode - print only data explicitly being present in the data tree.
                                      Note that this is the default value when no WD option is specified. */
#define LYDP_WD_TRIM       0x10  /**< Do not print the nodes with the value equal to their default value */
#define LYDP_WD_ALL        0x20  /**< Include implicit default nodes */
#define LYDP_WD_ALL_TAG    0x40  /**< Same as #LYDP_WD_ALL but also adds attribute 'default' with value 'true' to
                                      all nodes that has its default value. The 'default' attribute has namespace:
                                      urn:ietf:params:xml:ns:netconf:default:1.0 and thus the attributes are
                                      printed only when the ietf-netconf-with-defaults module is present in libyang
                                      context (but in that case this namespace is always printed). */
#define LYDP_WD_IMPL_TAG   0x80  /**< Same as LYDP_WD_ALL_TAG but the attributes are added only to the nodes that
                                      are not explicitly present in the original data tree despite their
                                      value is equal to their default value.  There is the same limitation regarding
                                      the presence of ietf-netconf-with-defaults module in libyang context. */
/**
 * @}
 */

/**
 * @brief Common YANG data printer.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYDP_WITHSIBLINGS option is accepted.
 * @return Number of printed characters (excluding the null byte used to end the string) in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lyd_print(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, int options);

#endif /* LY_PRINTER_DATA_H_ */
