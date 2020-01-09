/**
 * @file printer_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic schema printers functions.
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
#include "printer.h"
#include "printer_internal.h"
#include "tree_schema.h"

API ssize_t
lys_print(struct lyp_out *out, const struct lys_module *module, LYS_OUTFORMAT format, int UNUSED(line_length), int options)
{
    LY_ERR ret;
    size_t printed_prev;

    LY_CHECK_ARG_RET(NULL, out, module, -LY_EINVAL);

    printed_prev = out->printed;

    switch (format) {
    case LYS_OUT_YANG:
        ret = yang_print_parsed(out, module);
        break;
    case LYS_OUT_YANG_COMPILED:
        ret = yang_print_compiled(out, module, options);
        break;
    case LYS_OUT_YIN:
        ret = yin_print_parsed(out, module);
        break;
    /* TODO not yet implemented
    case LYS_OUT_TREE:
        ret = tree_print_model(out, module, target_node, line_length, options);
        break;
    case LYS_OUT_INFO:
        ret = info_print_model(out, module, target_node);
        break;
    */
    default:
        LOGERR(module->ctx, LY_EINVAL, "Unsupported output format.");
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

API ssize_t
lys_print_node(struct lyp_out *out, const struct lysc_node *node, LYS_OUTFORMAT format, int UNUSED(line_length), int options)
{
    LY_ERR ret;
    size_t printed_prev;

    LY_CHECK_ARG_RET(NULL, out, node, -LY_EINVAL);

    printed_prev = out->printed;

    switch (format) {
    case LYS_OUT_YANG_COMPILED:
        ret = yang_print_compiled_node(out, node, options);
        break;
    /* TODO not yet implemented
    case LYS_OUT_YIN:
        ret = yin_print_parsed(out, module);
        break;
    case LYS_OUT_TREE:
        ret = tree_print_model(out, module, target_node, line_length, options);
        break;
    */
    default:
        LOGERR(NULL, LY_EINVAL, "Unsupported output format.");
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

