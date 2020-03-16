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

#include <stdio.h>
#include <unistd.h>

#include "tree_data.h"

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
#define LYDP_WD_ALL_TAG    0x40  /**< Same as #LYP_WD_ALL but also adds attribute 'default' with value 'true' to
                                      all nodes that has its default value. The 'default' attribute has namespace:
                                      urn:ietf:params:xml:ns:netconf:default:1.0 and thus the attributes are
                                      printed only when the ietf-netconf-with-defaults module is present in libyang
                                      context (but in that case this namespace is always printed). */
#define LYDP_WD_IMPL_TAG   0x80  /**< Same as LYP_WD_ALL_TAG but the attributes are added only to the nodes that
                                      are not explicitly present in the original data tree despite their
                                      value is equal to their default value.  There is the same limitation regarding
                                      the presence of ietf-netconf-with-defaults module in libyang context. */
#define LYDP_NETCONF       0x100 /**< Print the data tree for use in NETCONF meaning:
                                      - for RPC output - skip the top-level RPC node,
                                      - for action output - skip all the parents of and the action node itself,
                                      - for action input - enclose the data in an action element in the base YANG namespace,
                                      - for all other data - print the whole data tree normally. */
/**
 * @}
 */

/**
 * @brief Print data tree in the specified format into a memory block.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] format Data output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYP_WITHSIBLINGS option is accepted.
 * @return Number of printed characters (excluding the null byte used to end the string) in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format into a file descriptor.
 *
 * @param[in] fd File descriptor where to print the data.
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] format Data output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYP_WITHSIBLINGS option is accepted.
 * @return Number of printed characters (excluding the null byte used to end the string) in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format into a file stream.
 *
 * @param[in] f File stream where to print the schema.
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] format Data output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYP_WITHSIBLINGS option is accepted.
 * @return Number of printed characters (excluding the null byte used to end the string) in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format into a file.
 *
 * @param[in] path File where to print the schema.
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] format Data output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYP_WITHSIBLINGS option is accepted.
 * @return Number of printed characters (excluding the null byte used to end the string) in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format using the provided callback.
 *
 * @param[in] writeclb Callback function to write the data (see write(2)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] root Root node of the data tree to print. It can be actually any (not only real root)
 * node of the data tree to print the specific subtree.
 * @param[in] format Data output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYP_WITHSIBLINGS option is accepted.
 * @return Number of printed characters (excluding the null byte used to end the string) in case of success.
 * @return Negative value failure (absolute value corresponds to LY_ERR values).
 */
ssize_t lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lyd_node *root,
                      LYD_FORMAT format, int options);

#endif /* LY_PRINTER_DATA_H_ */
