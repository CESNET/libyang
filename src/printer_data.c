/**
 * @file printer_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic data printers functions.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "printer_internal.h"
#include "printer_data.h"
#include "tree_schema.h"
#include "tree_data.h"

/**
 * @brief Common YANG data printer.
 *
 * @param[in] out Prepared structure defining the type and details of the printer output.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYP_WITHSIBLINGS option is accepted.
 * @return LY_ERR value.
 */
API ssize_t
lyd_print(struct lyp_out *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    LY_ERR ret;
    size_t printed_prev;

    LY_CHECK_ARG_RET(NULL, out, root, -LY_EINVAL);

    printed_prev = out->printed;

    switch (format) {
    case LYD_XML:
        ret = xml_print_data(out, root, options);
        break;
#if 0
    case LYD_JSON:
        ret = json_print_data(out, root, options);
        break;
    case LYD_LYB:
        ret = lyb_print_data(out, root, options);
        break;
#endif
    default:
        LOGERR(out->ctx, LY_EINVAL, "Unknown output format.");
        ret = LY_EINVAL;
        break;
    }

    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)(out->printed - printed_prev);
    }
}
