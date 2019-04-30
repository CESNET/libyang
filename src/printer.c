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
    int fd;
    FILE *stream;
#endif

    LYOUT_CHECK(out, out->status);

    va_start(ap, format);

    switch (out->type) {
    case LYOUT_FD:
#ifdef HAVE_VDPRINTF
        count = vdprintf(out->method.fd, format, ap);
        break;
#else
        /* Without vdfprintf(), change the printing method to printing to a FILE stream.
         * To preserve the original file descriptor, duplicate it and use it to open file stream.
         * Due to a standalone LYOUT_FDSTREAM, ly*_print_fd() functions are supposed to detect the
         * change and close the stream on their exit. */
        fd = dup(out->method.fd);
        if (fd < 0) {
            LOGERR(NULL, LY_ESYS, "Unable to duplicate provided file descriptor (%d) for printing the output (%s).",
                   out->method.fd, strerror(errno));
            va_end(ap);
            out->status = LY_ESYS;
            return LY_ESYS;
        }
        stream = fdopen(fd, "a");
        if (!stream) {
            LOGERR(NULL, LY_ESYS, "Unable to open provided file descriptor (%d) for printing the output (%s).",
                   out->method.fd, strerror(errno));
            va_end(ap);
            out->status = LY_ESYS;
            return LY_ESYS;
        }
        out->method.f = stream;
        out->type = LYOUT_FDSTREAM;
#endif
        /* fall through */
    case LYOUT_FDSTREAM:
    case LYOUT_STREAM:
        count = vfprintf(out->method.f, format, ap);
        break;
    case LYOUT_MEMORY:
        if ((count = vasprintf(&msg, format, ap)) < 0) {
            break;
        }
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            aux = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                va_end(ap);
                return LY_EMEM;
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
        if ((count = vasprintf(&msg, format, ap)) < 0) {
            break;
        }
        count = out->method.clb.f(out->method.clb.arg, msg, count);
        free(msg);
        break;
    }

    va_end(ap);

    if (count < 0) {
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (%s).", __func__, strerror(errno));
        out->status = LY_ESYS;
        return LY_ESYS;
    } else {
        out->printed += count;
        return LY_SUCCESS;
    }
}

void
ly_print_flush(struct lyout *out)
{
    switch (out->type) {
    case LYOUT_FDSTREAM:
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

    LYOUT_CHECK(out, out->status);

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

repeat:
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

        out->printed += count;
        return LY_SUCCESS;
    case LYOUT_FD:
        written = write(out->method.fd, buf, count);
        break;
    case LYOUT_FDSTREAM:
    case LYOUT_STREAM:
        written =  fwrite(buf, sizeof *buf, count, out->method.f);
        break;
    case LYOUT_CALLBACK:
        written = out->method.clb.f(out->method.clb.arg, buf, count);
        break;
    }

    if (written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            goto repeat;
        }
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (%s).", __func__, strerror(errno));
        out->status = LY_ESYS;
        return LY_ESYS;
    } else if ((size_t)written != count) {
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (unable to write %u from %u data).", __func__, count - (size_t)written, count);
        out->status = LY_ESYS;
        return LY_ESYS;
    } else {
        out->printed += written;
        return LY_SUCCESS;
    }
}

LY_ERR
ly_write_skip(struct lyout *out, size_t count, size_t *position)
{
    LYOUT_CHECK(out, out->status);

    switch (out->type) {
    case LYOUT_MEMORY:
        if (out->method.mem.len + count > out->method.mem.size) {
            out->method.mem.buf = ly_realloc(out->method.mem.buf, out->method.mem.len + count);
            if (!out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                out->status = LY_ESYS;
                LOGMEM_RET(NULL);
            }
            out->method.mem.size = out->method.mem.len + count;
        }

        /* save the current position */
        *position = out->method.mem.len;

        /* skip the memory */
        out->method.mem.len += count;

        /* update printed bytes counter despite we actually printed just a hole */
        out->printed += count;
        break;
    case LYOUT_FD:
    case LYOUT_FDSTREAM:
    case LYOUT_STREAM:
    case LYOUT_CALLBACK:
        /* buffer the hole */
        if (out->buf_len + count > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + count);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                out->status = LY_ESYS;
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

    LYOUT_CHECK(out, out->status);

    switch (out->type) {
    case LYOUT_MEMORY:
        /* write */
        memcpy(&out->method.mem.buf[position], buf, count);
        break;
    case LYOUT_FD:
    case LYOUT_FDSTREAM:
    case LYOUT_STREAM:
    case LYOUT_CALLBACK:
        if (out->buf_len < position + count) {
            out->status = LY_ESYS;
            LOGMEM_RET(NULL);
        }

        /* write into the hole */
        memcpy(&out->buffered[position], buf, count);

        /* decrease hole counter */
        --out->hole_count;

        if (!out->hole_count) {
            /* all holes filled, we can write the buffer,
             * printed bytes counter is updated by ly_write() */
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

    ret = lys_print_(&out, module, format, line_length, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
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
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    LY_ERR ret;
    struct lyout out;

    LY_CHECK_ARG_RET(NULL, fd >= 0, module, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = module->ctx;
    out.type = LYOUT_FD;
    out.method.fd = fd;

    ret = lys_print_(&out, module, format, line_length, options);

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
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, int line_length, int options)
{
    struct lyout out;
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, strp, module, LY_EINVAL);

    memset(&out, 0, sizeof out);
    out.ctx = module->ctx;
    out.type = LYOUT_MEMORY;

    ret = lys_print_(&out, module, format, line_length, options);
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

    ret = lys_print_(&out, module, format, line_length, options);
    if (ret) {
        /* error */
        return (-1) * ret;
    } else {
        /* success */
        return (ssize_t)out.printed;
    }
}
