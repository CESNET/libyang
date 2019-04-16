/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Wrapper for all libyang printers.
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
#include <stdarg.h>
#include <string.h>

#include "printer_internal.h"

/**
 * @brief informational structure shared by printers
 */
struct ext_substmt_info_s ext_substmt_info[] = {
  {NULL, NULL, 0},                              /**< LYEXT_SUBSTMT_SELF */
  {"argument", "name", SUBST_FLAG_ID},          /**< LYEXT_SUBSTMT_ARGUMENT */
  {"base", "name", SUBST_FLAG_ID},              /**< LYEXT_SUBSTMT_BASE */
  {"belongs-to", "module", SUBST_FLAG_ID},      /**< LYEXT_SUBSTMT_BELONGSTO */
  {"contact", "text", SUBST_FLAG_YIN},          /**< LYEXT_SUBSTMT_CONTACT */
  {"default", "value", 0},                      /**< LYEXT_SUBSTMT_DEFAULT */
  {"description", "text", SUBST_FLAG_YIN},      /**< LYEXT_SUBSTMT_DESCRIPTION */
  {"error-app-tag", "value", 0},                /**< LYEXT_SUBSTMT_ERRTAG */
  {"error-message", "value", SUBST_FLAG_YIN},   /**< LYEXT_SUBSTMT_ERRMSG */
  {"key", "value", 0},                          /**< LYEXT_SUBSTMT_KEY */
  {"namespace", "uri", 0},                      /**< LYEXT_SUBSTMT_NAMESPACE */
  {"organization", "text", SUBST_FLAG_YIN},     /**< LYEXT_SUBSTMT_ORGANIZATION */
  {"path", "value", 0},                         /**< LYEXT_SUBSTMT_PATH */
  {"prefix", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_PREFIX */
  {"presence", "value", 0},                     /**< LYEXT_SUBSTMT_PRESENCE */
  {"reference", "text", SUBST_FLAG_YIN},        /**< LYEXT_SUBSTMT_REFERENCE */
  {"revision-date", "date", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_REVISIONDATE */
  {"units", "name", 0},                         /**< LYEXT_SUBSTMT_UNITS */
  {"value", "value", SUBST_FLAG_ID},            /**< LYEXT_SUBSTMT_VALUE */
  {"yang-version", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_VERSION */
  {"modifier", "value", SUBST_FLAG_ID},         /**< LYEXT_SUBSTMT_MODIFIER */
  {"require-instance", "value", SUBST_FLAG_ID}, /**< LYEXT_SUBSTMT_REQINST */
  {"yin-element", "value", SUBST_FLAG_ID},      /**< LYEXT_SUBSTMT_YINELEM */
  {"config", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_CONFIG */
  {"mandatory", "value", SUBST_FLAG_ID},        /**< LYEXT_SUBSTMT_MANDATORY */
  {"ordered-by", "value", SUBST_FLAG_ID},       /**< LYEXT_SUBSTMT_ORDEREDBY */
  {"status", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_STATUS */
  {"fraction-digits", "value", SUBST_FLAG_ID},  /**< LYEXT_SUBSTMT_DIGITS */
  {"max-elements", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_MAX */
  {"min-elements", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_MIN */
  {"position", "value", SUBST_FLAG_ID},         /**< LYEXT_SUBSTMT_POSITION */
  {"unique", "tag", 0},                         /**< LYEXT_SUBSTMT_UNIQUE */
};

LY_ERR
ly_print(struct lyout *out, const char *format, ...)
{
    int count = 0;
    char *msg = NULL, *aux;
    va_list ap;
#ifndef HAVE_VDPRINTF
    FILE *stream;
#endif

    va_start(ap, format);

    switch (out->type) {
    case LYOUT_FD:
#ifdef HAVE_VDPRINTF
        count = vdprintf(out->method.fd, format, ap);
#else
        stream = fdopen(dup(out->method.fd), "a+");
        if (stream) {
            count = vfprintf(stream, format, ap);
            fclose(stream);
        }
#endif
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
                LOGMEM(NULL);
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

    if (count < 0) {
        return LY_EOTHER;
    } else {
        return LY_SUCCESS;
    }
}

void
ly_print_flush(struct lyout *out)
{
    switch (out->type) {
    case LYOUT_STREAM:
        fflush(out->method.f);
        break;
    case LYOUT_FD:
    case LYOUT_MEMORY:
    case LYOUT_CALLBACK:
        /* nothing to do */
        break;
    }
}

LY_ERR
ly_write(struct lyout *out, const char *buf, size_t count)
{
    int written = 0;

    if (out->hole_count) {
        /* we are buffering data after a hole */
        if (out->buf_len + count > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + count);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM_RET(NULL);
            }
            out->buf_size = out->buf_len + count;
        }

        memcpy(&out->buffered[out->buf_len], buf, count);
        out->buf_len += count;
        return LY_SUCCESS;
    }

    switch (out->type) {
    case LYOUT_MEMORY:
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            out->method.mem.buf = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM_RET(NULL);
            }
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], buf, count);
        out->method.mem.len += count;
        out->method.mem.buf[out->method.mem.len] = '\0';
        return LY_SUCCESS;
    case LYOUT_FD:
        written = write(out->method.fd, buf, count);
        break;
    case LYOUT_STREAM:
        written =  fwrite(buf, sizeof *buf, count, out->method.f);
        break;
    case LYOUT_CALLBACK:
        written = out->method.clb.f(out->method.clb.arg, buf, count);
        break;
    }

    if (written < 0) {
        return LY_EOTHER;
    } else {
        return LY_SUCCESS;
    }
}

LY_ERR
ly_write_skip(struct lyout *out, size_t count, size_t *position)
{
    switch (out->type) {
    case LYOUT_MEMORY:
        if (out->method.mem.len + count > out->method.mem.size) {
            out->method.mem.buf = ly_realloc(out->method.mem.buf, out->method.mem.len + count);
            if (!out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM_RET(NULL);
            }
            out->method.mem.size = out->method.mem.len + count;
        }

        /* save the current position */
        *position = out->method.mem.len;

        /* skip the memory */
        out->method.mem.len += count;
        break;
    case LYOUT_FD:
    case LYOUT_STREAM:
    case LYOUT_CALLBACK:
        /* buffer the hole */
        if (out->buf_len + count > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + count);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM_RET(NULL);
            }
            out->buf_size = out->buf_len + count;
        }

        /* save the current position */
        *position = out->buf_len;

        /* skip the memory */
        out->buf_len += count;

        /* increase hole counter */
        ++out->hole_count;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_write_skipped(struct lyout *out, size_t position, const char *buf, size_t count)
{
    LY_ERR ret = LY_SUCCESS;

    switch (out->type) {
    case LYOUT_MEMORY:
        /* write */
        memcpy(&out->method.mem.buf[position], buf, count);
        break;
    case LYOUT_FD:
    case LYOUT_STREAM:
    case LYOUT_CALLBACK:
        if (out->buf_len < position + count) {
            LOGMEM_RET(NULL);
        }

        /* write into the hole */
        memcpy(&out->buffered[position], buf, count);

        /* decrease hole counter */
        --out->hole_count;

        if (!out->hole_count) {
            /* all holes filled, we can write the buffer */
            ret = ly_write(out, out->buffered, out->buf_len);
            out->buf_len = 0;
        }
        break;
    }

    return ret;
}

static LY_ERR
lys_print_(struct lyout *out, const struct lys_module *module, LYS_OUTFORMAT format, int UNUSED(line_length), int UNUSED(options))
{
    LY_ERR ret;

    switch (format) {
    case LYS_OUT_YANG:
        ret = yang_print_parsed(out, module);
        break;
    case LYS_OUT_YANG_COMPILED:
        ret = yang_print_compiled(out, module);
        break;
    /* TODO not yet implemented
    case LYS_OUT_YIN:
        lys_disable_deviations((struct lys_module *)module);
        ret = yin_print_model(out, module);
        lys_enable_deviations((struct lys_module *)module);
        break;
    case LYS_OUT_TREE:
        ret = tree_print_model(out, module, target_node, line_length, options);
        break;
    case LYS_OUT_INFO:
        ret = info_print_model(out, module, target_node);
        break;
    case LYS_OUT_JSON:
        ret = jsons_print_model(out, module, target_node);
        break;
    */
    default:
        LOGERR(module->ctx, LY_EINVAL, "Unknown output format.");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

API LY_ERR
lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, f, module, LY_EINVAL);

    memset(&out, 0, sizeof out);

    out.type = LYOUT_STREAM;
    out.method.f = f;

    return lys_print_(&out, module, format, line_length, options);
}

API LY_ERR
lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    FILE *f;
    LY_ERR ret;

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

API LY_ERR
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, fd >= 0, module, LY_EINVAL);

    memset(&out, 0, sizeof out);

    out.type = LYOUT_FD;
    out.method.fd = fd;

    return lys_print_(&out, module, format, line_length, options);
}

API LY_ERR
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct lyout out;
    LY_ERR r;

    LY_CHECK_ARG_RET(NULL, strp, module, LY_EINVAL);

    memset(&out, 0, sizeof out);

    out.type = LYOUT_MEMORY;

    r = lys_print_(&out, module, format, line_length, options);

    *strp = out.method.mem.buf;
    return r;
}

API LY_ERR
lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lys_module *module,
              LYS_OUTFORMAT format, int line_length, int options)
{
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, writeclb, module, LY_EINVAL);

    memset(&out, 0, sizeof out);

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    return lys_print_(&out, module, format, line_length, options);
}
