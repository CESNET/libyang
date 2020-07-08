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

#include "printer_schema.h"

#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "log.h"
#include "printer.h"
#include "printer_internal.h"
#include "tree_schema.h"

API LY_ERR
lys_print(struct ly_out *out, const struct lys_module *module, LYS_OUTFORMAT format, int UNUSED(line_length), int options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, module, LY_EINVAL);

    /* reset number of printed bytes */
    out->func_printed = 0;

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

    return ret;
}

static LY_ERR
lys_print_(struct ly_out *out, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, LY_EINVAL);

    ret = lys_print(out, module, format, line_length, options);

    ly_out_free(out, NULL, 0);
    return ret;
}

API LY_ERR
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, strp, module, LY_EINVAL);

    /* init */
    *strp = NULL;

    LY_CHECK_RET(ly_out_new_memory(strp, 0, &out));
    return lys_print_(out, module, format, line_length, options);
}

API LY_ERR
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, fd != -1, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_fd(fd, &out));
    return lys_print_(out, module, format, line_length, options);
}

API LY_ERR
lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, f, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_file(f, &out));
    return lys_print_(out, module, format, line_length, options);
}

API LY_ERR
lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, path, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_filepath(path, &out));
    return lys_print_(out, module, format, line_length, options);
}

API LY_ERR
lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
              const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, writeclb, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_clb(writeclb, arg, &out));
    return lys_print_(out, module, format, line_length, options);
}

API LY_ERR
lys_print_node(struct ly_out *out, const struct lysc_node *node, LYS_OUTFORMAT format, int UNUSED(line_length), int options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, node, LY_EINVAL);

    /* reset number of printed bytes */
    out->func_printed = 0;

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

    return ret;
}

