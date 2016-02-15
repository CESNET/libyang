/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Wrapper for all libyang printers.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#define _GNU_SOURCE /* vasprintf(), vdprintf() */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "printer.h"

/* 0 - same, 1 - different */
int
nscmp(const struct lyd_node *node1, const struct lyd_node *node2)
{
    /* we have to cover submodules belonging to the same module */
    if (lys_node_module(node1->schema) == lys_node_module(node2->schema)) {
        /* belongs to the same module */
        return 0;
    } else {
        /* different modules */
        return 1;
    }
}

int
ly_print(struct lyout *out, const char *format, ...)
{
    int count = 0;
    char *msg = NULL, *aux;
    va_list ap;

    va_start(ap, format);

    switch(out->type) {
    case LYOUT_FD:
        count = vdprintf(out->method.fd, format, ap);
        break;
    case LYOUT_STREAM:
        count = vfprintf(out->method.f, format, ap);
        break;
    case LYOUT_MEMORY:
        count = vasprintf(&msg, format, ap);
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            aux = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM;
                va_end(ap);
                return -1;
            }
            out->method.mem.buf = aux;
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], msg, count);
        out->method.mem.len += count;
        out->method.mem.buf[out->method.mem.len] = '\0';
        free(msg);
        break;
    case LYOUT_CALLBACK:
        count = vasprintf(&msg, format, ap);
        count = out->method.clb.f(out->method.clb.arg, msg, count);
        free(msg);
        break;
    }

    va_end(ap);
    return count;
}

int
ly_write(struct lyout *out, const char *buf, size_t count)
{
    char *aux;

    switch(out->type) {
    case LYOUT_FD:
        return write(out->method.fd, buf, count);
    case LYOUT_STREAM:
        return fwrite(buf, sizeof *buf, count, out->method.f);
    case LYOUT_MEMORY:
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            aux = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM;
                return -1;
            }
            out->method.mem.buf = aux;
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], buf, count + 1);
        out->method.mem.len += count;
        return count;
    case LYOUT_CALLBACK:
        return out->method.clb.f(out->method.clb.arg, buf, count);
    }

    return 0;
}

static int
lys_print_(struct lyout *out, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    switch (format) {
    case LYS_OUT_YIN:
        return yin_print_model(out, module);
    case LYS_OUT_YANG:
        return yang_print_model(out, module);
    case LYS_OUT_TREE:
        return tree_print_model(out, module);
    case LYS_OUT_INFO:
        return info_print_model(out, module, target_node);
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}

API int
lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;

    if (!f || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_STREAM;
    out.method.f = f;

    return lys_print_(&out, module, format, target_node);
}

API int
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;

    if (fd < 0 || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_FD;
    out.method.fd = fd;

    return lys_print_(&out, module, format, target_node);
}

API int
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;
    int r;

    if (!strp || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_MEMORY;
    out.method.mem.buf = NULL;
    out.method.mem.len = 0;
    out.method.mem.size = 0;

    r = lys_print_(&out, module, format, target_node);

    *strp = out.method.mem.buf;
    return r;
}

API int
lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;

    if (!writeclb || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    return lys_print_(&out, module, format, target_node);
}

static int
lyd_print_(struct lyout *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    if (!root) {
        /* no data to print, but even empty tree is valid */
        if (out->type == LYOUT_MEMORY || out->type == LYOUT_CALLBACK) {
            ly_print(out, "");
        }
        return EXIT_SUCCESS;
    }

    switch (format) {
    case LYD_XML:
        return xml_print_data(out, root, 0, options);
    case LYD_XML_FORMAT:
        return xml_print_data(out, root, 1, options);
    case LYD_JSON:
        return json_print_data(out, root, options);
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}

API int
lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;

    if (!f) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_STREAM;
    out.method.f = f;

    return lyd_print_(&out, root, format, options);
}

API int
lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;

    if (fd < 0) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_FD;
    out.method.fd = fd;

    return lyd_print_(&out, root, format, options);
}

API int
lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;
    int r;

    if (!strp) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_MEMORY;
    out.method.mem.buf = NULL;
    out.method.mem.len = 0;
    out.method.mem.size = 0;

    r = lyd_print_(&out, root, format, options);

    *strp = out.method.mem.buf;
    return r;
}

API int
lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lyd_node *root,
              LYD_FORMAT format, int options)
{
    struct lyout out;

    if (!writeclb) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    return lyd_print_(&out, root, format, options);
}
