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

API LY_ERR
lyd_print(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    LY_ERR ret = LY_EINVAL;

    LY_CHECK_ARG_RET(NULL, out, root, LY_EINVAL);

    /* reset number of printed bytes */
    out->func_printed = 0;

    switch (format) {
    case LYD_XML:
        ret = xml_print_data(out, root, options);
        break;
#if 0
    case LYD_JSON:
        ret = json_print_data(out, root, options);
        break;
#endif
    case LYD_LYB:
        ret = lyb_print_data(out, root, options);
        break;
    case LYD_SCHEMA:
        LOGERR(out->ctx, LY_EINVAL, "Invalid output format.");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

static LY_ERR
lyd_print_(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, LY_EINVAL);

    ret = lyd_print(out, root, format, options);

    ly_out_free(out, NULL, 0);
    return ret;
}

API LY_ERR
lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, strp, root, LY_EINVAL);

    /* init */
    *strp = NULL;

    LY_CHECK_RET(ly_out_new_memory(strp, 0, &out));
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, fd != -1, root, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_fd(fd, &out));
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, f, root, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_file(f, &out));
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, path, root, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_filepath(path, &out));
    return lyd_print_(out, root, format, options);
}

API LY_ERR
lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
              const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, writeclb, root, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_clb(writeclb, arg, &out));
    return lyd_print_(out, root, format, options);
}
