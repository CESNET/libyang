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

#include "printer_data.h"

#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "log.h"
#include "printer.h"
#include "printer_internal.h"
#include "tree_data.h"

/**
 * @brief Common YANG data printer.
 *
 * @param[in] out Prepared structure defining the type and details of the printer output.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags). With \p format LYD_LYB, only #LYDP_WITHSIBLINGS option is accepted.
 * @return LY_ERR value.
 */
API ssize_t
lyd_print(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, int options)
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

static LY_ERR
lyd_print_(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    ssize_t result;

    LY_CHECK_ARG_RET(NULL, out, LY_EINVAL);

    result = lyd_print(out, root, format, options);

    ly_out_free(out, NULL, 0);

    if (result < 0) {
        return (-1) * result;
    } else {
        return LY_SUCCESS;
    }
}

API LY_ERR
lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, strp, root, LY_EINVAL);

    /* init */
    *strp = NULL;

    out = ly_out_new_memory(strp, 0);
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, fd != -1, root, LY_EINVAL);

    out = ly_out_new_fd(fd);
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, f, root, LY_EINVAL);

    out = ly_out_new_file(f);
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, path, root, LY_EINVAL);

    out = ly_out_new_filepath(path);
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
              const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, writeclb, root, LY_EINVAL);

    out = ly_out_new_clb(writeclb, arg);
    return lyd_print_(out, root, format, options);
}
