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
#include "printer_internal.h"
#include "tree_schema.h"

/**
 * @brief Common schema printer.
 *
 * @param[in] out Prepared structure defining the type and details of the printer output.
 * @param[in] module Schema to print.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
static LY_ERR
lys_print_module(struct lyout *out, const struct lys_module *module, LYS_OUTFORMAT format, int UNUSED(line_length), int options)
{
    LY_ERR ret;

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
        LOGERR(module->ctx, LY_EINVAL, "Unknown output format.");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

/**
 * @brief Common schema node printer.
 *
 * @param[in] out Prepared structure defining the type and details of the printer output.
 * @param[in] ctx libyang context to search for top-level nodes.
 * @param[in] context_node Context node in the case of relative @p target_node path.
 * @param[in] format Output format.
 * @param[in] target_node Schema node path, use full module names as node's prefixes.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in lyout::printed.
 */
static LY_ERR
lys_print_target(struct lyout *out, struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                 const char *target_node, int UNUSED(line_length), int options)
{
    LY_ERR ret;
    const struct lysc_node *node;

    /* resolve the target_node */
    node = lys_find_node(ctx, context_node, target_node);
    if (!node) {
        return LY_EINVAL;
    }

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
        LOGERR(ctx, LY_EINVAL, "Unknown output format.");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

API ssize_t
lys_node_print_file(FILE *f, struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                    const char *target_node, int line_length, int options)
{
    struct lyout out;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, f, ctx || context_node, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = ctx ? ctx : context_node->module->ctx;
    out.type = LYOUT_STREAM;
    out.method.f = f;

    ret = lys_print_target(&out, ctx, context_node, format, target_node, line_length, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}

API ssize_t
lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct lyout out;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, f, module, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = module->ctx;
    out.type = LYOUT_STREAM;
    out.method.f = f;

    ret = lys_print_module(&out, module, format, line_length, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}

API ssize_t
lys_node_print_path(const char *path,  struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                    const char *target_node, int line_length, int options)
{
    FILE *f;
    ssize_t ret;

    LY_CHECK_ARG_RET(NULL, path, ctx || context_node, LY_EINVAL);

    f = fopen(path, "w");
    if (!f) {
        LOGERR(ctx ? ctx : context_node->module->ctx, LY_ESYS, "Failed to open file \"%s\" (%s).", path, strerror(errno));
        return LY_ESYS;
    }

    ret = lys_node_print_file(f, ctx, context_node, format, target_node, line_length, options);
    fclose(f);
    return ret;
}

API ssize_t
lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    FILE *f;
    ssize_t ret;

    LY_CHECK_ARG_RET(NULL, path, module, LY_EINVAL);

    f = fopen(path, "w");
    if (!f) {
        LOGERR(module->ctx, LY_ESYS, "Failed to open file \"%s\" (%s).", path, strerror(errno));
        return LY_ESYS;
    }

    ret = lys_print_file(f, module, format, line_length, options);
    fclose(f);
    return ret;
}

API ssize_t
lys_node_print_fd(int fd, struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                  const char *target_node, int line_length, int options)
{
    LY_ERR ret;
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, fd >= 0, ctx || context_node, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = ctx ? ctx : context_node->module->ctx;
    out.type = LYOUT_FD;
    out.method.fd = fd;

    ret = lys_print_target(&out, ctx, context_node, format, target_node, line_length, options);

    if (out.type == LYOUT_FDSTREAM) {
        /* close temporary stream based on the given file descriptor */
        fclose(out.method.f);
        /* move the original file descriptor to the end of the output file */
        lseek(fd, 0, SEEK_END);
    }

    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}

API ssize_t
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    LY_ERR ret;
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, fd >= 0, module, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = module->ctx;
    out.type = LYOUT_FD;
    out.method.fd = fd;

    ret = lys_print_module(&out, module, format, line_length, options);

    if (out.type == LYOUT_FDSTREAM) {
        /* close temporary stream based on the given file descriptor */
        fclose(out.method.f);
        /* move the original file descriptor to the end of the output file */
        lseek(fd, 0, SEEK_END);
    }

    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}

API ssize_t
lys_node_print_mem(char **strp, struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                   const char *target_node, int line_length, int options)
{
    struct lyout out;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, strp, ctx || context_node, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = ctx ? ctx : context_node->module->ctx;
    out.type = LYOUT_MEMORY;

    ret = lys_print_target(&out, ctx, context_node, format, target_node, line_length, options);
    if (ret) {
        /* error */
        *strp = NULL;
        return (-1) * ret;
    } else {
        /* success */
        *strp = out.method.mem.buf;
        return (ssize_t)out.printed;
    }
}

API ssize_t
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct lyout out;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, strp, module, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = module->ctx;
    out.type = LYOUT_MEMORY;

    ret = lys_print_module(&out, module, format, line_length, options);
    if (ret) {
        /* error */
        *strp = NULL;
        return (-1) * ret;
    } else {
        /* success */
        *strp = out.method.mem.buf;
        return (ssize_t)out.printed;
    }
}

API ssize_t
lys_node_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                   struct ly_ctx *ctx, const struct lysc_node *context_node, LYS_OUTFORMAT format,
                   const char *target_node, int line_length, int options)
{
    LY_ERR ret;
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, writeclb, ctx || context_node, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = ctx ? ctx : context_node->module->ctx;
    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    ret = lys_print_target(&out, ctx, context_node, format, target_node, line_length, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}

API ssize_t
lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lys_module *module,
              LYS_OUTFORMAT format, int line_length, int options)
{
    LY_ERR ret;
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, writeclb, module, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = module->ctx;
    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    ret = lys_print_module(&out, module, format, line_length, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}
