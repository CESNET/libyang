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
static LY_ERR
lyd_print_(struct lyout *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    LY_ERR ret;

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

    return ret;
}

API ssize_t
lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, f, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.type = LYOUT_STREAM;
    out.method.f = f;

    if (root) {
        out.ctx = LYD_NODE_CTX(root);
    }

    ret = lyd_print_(&out, root, format, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}

API ssize_t
lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    FILE *f;
    ssize_t ret;

    LY_CHECK_ARG_RET(NULL, path, LY_EINVAL);

    f = fopen(path, "w");
    if (!f) {
        LOGERR(root ? LYD_NODE_CTX(root) : NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", path, strerror(errno));
        return LY_ESYS;
    }

    ret = lyd_print_file(f, root, format, options);
    fclose(f);
    return ret;
}

API ssize_t
lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    LY_ERR ret;
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, fd >= 0, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.type = LYOUT_FD;
    out.method.fd = fd;

    if (root) {
        out.ctx = LYD_NODE_CTX(root);
    }

    ret = lyd_print_(&out, root, format, options);

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
lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, strp, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.type = LYOUT_MEMORY;

    if (root) {
        out.ctx = LYD_NODE_CTX(root);
    }

    ret = lyd_print_(&out, root, format, options);
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
lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lyd_node *root,
              LYD_FORMAT format, int options)
{
    LY_ERR ret;
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, writeclb, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    if (root) {
        out.ctx = LYD_NODE_CTX(root);
    }

    ret = lyd_print_(&out, root, format, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}
