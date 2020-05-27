/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic libyang printers functions.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "printer.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "log.h"
#include "plugins_types.h"
#include "printer_data.h"
#include "printer_internal.h"
#include "tree.h"
#include "tree_schema.h"

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

int
ly_is_default(const struct lyd_node *node)
{
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lyd_node_term *term;
    LY_ARRAY_SIZE_TYPE u;

    assert(node->schema->nodetype & LYD_NODE_TERM);
    term = (const struct lyd_node_term *)node;

    if (node->schema->nodetype == LYS_LEAF) {
        leaf = (const struct lysc_node_leaf *)node->schema;
        if (!leaf->dflt) {
            return 0;
        }

        /* compare with the default value */
        if (leaf->type->plugin->compare(&term->value, leaf->dflt)) {
            return 0;
        }
    } else {
        llist = (const struct lysc_node_leaflist *)node->schema;
        if (!llist->dflts) {
            return 0;
        }

        LY_ARRAY_FOR(llist->dflts, u) {
            /* compare with each possible default value */
            if (llist->type->plugin->compare(&term->value, llist->dflts[u])) {
                return 0;
            }
        }
    }

    return 1;
}

int
ly_should_print(const struct lyd_node *node, int options)
{
    const struct lyd_node *next, *elem;

    if (options & LYDP_WD_TRIM) {
        /* do not print default nodes */
        if (node->flags & LYD_DEFAULT) {
            /* implicit default node/NP container with only default nodes */
            return 0;
        } else if (node->schema->nodetype & LYD_NODE_TERM) {
            if (ly_is_default(node)) {
                /* explicit default node */
                return 0;
            }
        }
    } else if ((node->flags & LYD_DEFAULT) && !(options & LYDP_WD_MASK) && !(node->schema->flags & LYS_CONFIG_R)) {
        /* LYDP_WD_EXPLICIT
         * - print only if it contains status data in its subtree */
        LYD_TREE_DFS_BEGIN(node, next, elem) {
            if ((elem->schema->nodetype != LYS_CONTAINER) || (elem->schema->flags & LYS_PRESENCE)) {
                if (elem->schema->flags & LYS_CONFIG_R) {
                    return 1;
                }
            }
            LYD_TREE_DFS_END(node, next, elem)
        }
        return 0;
    } else if ((node->flags & LYD_DEFAULT) && (node->schema->nodetype == LYS_CONTAINER) && !(options & LYDP_KEEPEMPTYCONT)) {
        /* avoid empty default containers */
        LYD_TREE_DFS_BEGIN(node, next, elem) {
            if (elem->schema->nodetype != LYS_CONTAINER) {
                return 1;
            }
            assert(elem->flags & LYD_DEFAULT);
            LYD_TREE_DFS_END(node, next, elem)
        }
        return 0;
    }

    return 1;
}

API LY_OUT_TYPE
ly_out_type(const struct ly_out *out)
{
    LY_CHECK_ARG_RET(NULL, out, LY_OUT_ERROR);
    return out->type;
}

API struct ly_out *
ly_out_new_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg)
{
    struct ly_out *out;

    out = calloc(1, sizeof *out);
    LY_CHECK_ERR_RET(!out, LOGMEM(NULL), NULL);

    out->type = LY_OUT_CALLBACK;
    out->method.clb.func = writeclb;
    out->method.clb.arg = arg;

    return out;
}

API ssize_t (*ly_out_clb(struct ly_out *out, ssize_t (*writeclb)(void *arg, const void *buf, size_t count)))(void *arg, const void *buf, size_t count)
{
    void *prev_clb;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_CALLBACK, NULL);

    prev_clb = out->method.clb.func;

    if (writeclb) {
        out->method.clb.func = writeclb;
    }

    return prev_clb;
}

API void *
ly_out_clb_arg(struct ly_out *out, void *arg)
{
    void *prev_arg;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_CALLBACK, NULL);

    prev_arg = out->method.clb.arg;

    if (arg) {
        out->method.clb.arg = arg;
    }

    return prev_arg;
}

API struct ly_out *
ly_out_new_fd(int fd)
{
    struct ly_out *out;

    out = calloc(1, sizeof *out);
    LY_CHECK_ERR_RET(!out, LOGMEM(NULL), NULL);

#ifdef HAVE_VDPRINTF
    out->type = LY_OUT_FD;
    out->method.fd = fd;
#else
    /* Without vdfprintf(), change the printing method to printing to a FILE stream.
     * To preserve the original file descriptor, duplicate it and use it to open file stream. */
    out->type = LY_OUT_FDSTREAM;
    out->method.fdstream.fd = fd;

    fd = dup(out->method.fdstream.fd);
    if (fd < 0) {
        LOGERR(NULL, LY_ESYS, "Unable to duplicate provided file descriptor (%d) for printing the output (%s).",
               out->method.fdstream.fd, strerror(errno));
        free(out);
        return NULL;
    }
    out->method.fdstream.f = fdopen(fd, "a");
    if (!out->method.fdstream.f) {
        LOGERR(NULL, LY_ESYS, "Unable to open provided file descriptor (%d) for printing the output (%s).",
               out->method.fdstream.fd, strerror(errno));
        free(out);
        fclose(fd);
        return NULL;
    }
#endif

    return out;
}

API int
ly_out_fd(struct ly_out *out, int fd)
{
    int prev_fd;

    LY_CHECK_ARG_RET(NULL, out, out->type <= LY_OUT_FDSTREAM, -1);

    if (out->type == LY_OUT_FDSTREAM) {
        prev_fd = out->method.fdstream.fd;
    } else { /* LY_OUT_FD */
        prev_fd = out->method.fd;
    }

    if (fd != -1) {
        /* replace output stream */
        if (out->type == LY_OUT_FDSTREAM) {
            int streamfd;
            FILE *stream;

            streamfd = dup(fd);
            if (streamfd < 0) {
                LOGERR(NULL, LY_ESYS, "Unable to duplicate provided file descriptor (%d) for printing the output (%s).", fd, strerror(errno));
                return -1;
            }
            stream = fdopen(streamfd, "a");
            if (!stream) {
                LOGERR(NULL, LY_ESYS, "Unable to open provided file descriptor (%d) for printing the output (%s).", fd, strerror(errno));
                close(streamfd);
                return -1;
            }
            /* close only the internally created stream, file descriptor is returned and supposed to be closed by the caller */
            fclose(out->method.fdstream.f);
            out->method.fdstream.f = stream;
            out->method.fdstream.fd = streamfd;
        } else { /* LY_OUT_FD */
            out->method.fd = fd;
        }
    }

    return prev_fd;
}

API struct ly_out *
ly_out_new_file(FILE *f)
{
    struct ly_out *out;

    out = calloc(1, sizeof *out);
    LY_CHECK_ERR_RET(!out, LOGMEM(NULL), NULL);

    out->type = LY_OUT_FILE;
    out->method.f = f;

    return out;
}

API FILE *
ly_out_file(struct ly_out *out, FILE *f)
{
    FILE *prev_f;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_FILE, NULL);

    prev_f = out->method.f;

    if (f) {
        out->method.f = f;
    }

    return prev_f;
}

API struct ly_out *
ly_out_new_memory(char **strp, size_t size)
{
    struct ly_out *out;

    out = calloc(1, sizeof *out);
    LY_CHECK_ERR_RET(!out, LOGMEM(NULL), NULL);

    out->type = LY_OUT_MEMORY;
    out->method.mem.buf = strp;
    if (!size) {
        /* buffer is supposed to be allocated */
        *strp = NULL;
    } else if (*strp) {
        /* there is already buffer to use */
        out->method.mem.size = size;
    }

    return out;
}

char *
ly_out_memory(struct ly_out *out, char **strp, size_t size)
{
    char *data;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_MEMORY, NULL);

    data = *out->method.mem.buf;

    if (strp) {
        out->method.mem.buf = strp;
        out->method.mem.len = out->method.mem.size = 0;
        out->printed = 0;
        if (!size) {
            /* buffer is supposed to be allocated */
            *strp = NULL;
        } else if (*strp) {
            /* there is already buffer to use */
            out->method.mem.size = size;
        }
    }

    return data;
}

API LY_ERR
ly_out_reset(struct ly_out *out)
{
    LY_CHECK_ARG_RET(NULL, out, LY_EINVAL);

    switch(out->type) {
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return LY_EINT;
    case LY_OUT_FD:
        if ((lseek(out->method.fd, 0, SEEK_SET) == -1) && errno != ESPIPE) {
            LOGERR(NULL, LY_ESYS, "Seeking output file descriptor failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        if (errno != ESPIPE && ftruncate(out->method.fd, 0) == -1) {
            LOGERR(NULL, LY_ESYS, "Truncating output file failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        break;
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILE:
    case LY_OUT_FILEPATH:
        if ((fseek(out->method.f, 0, SEEK_SET) == -1) && errno != ESPIPE) {
            LOGERR(NULL, LY_ESYS, "Seeking output file stream failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        if (errno != ESPIPE && ftruncate(fileno(out->method.f), 0) == -1) {
            LOGERR(NULL, LY_ESYS, "Truncating output file failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        break;
    case LY_OUT_MEMORY:
        if (out->method.mem.buf && *out->method.mem.buf) {
            memset(*out->method.mem.buf, 0, out->method.mem.len);
        }
        out->printed = 0;
        out->method.mem.len = 0;
        break;
    case LY_OUT_CALLBACK:
        /* nothing to do (not seekable) */
        break;
    }

    return LY_SUCCESS;
}

API struct ly_out *
ly_out_new_filepath(const char *filepath)
{
    struct ly_out *out;

    out = calloc(1, sizeof *out);
    LY_CHECK_ERR_RET(!out, LOGMEM(NULL), NULL);

    out->type = LY_OUT_FILEPATH;
    out->method.fpath.f = fopen(filepath, "w");
    if (!out->method.fpath.f) {
        LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", filepath, strerror(errno));
        return NULL;
    }
    out->method.fpath.filepath = strdup(filepath);
    return out;
}

API const char *
ly_out_filepath(struct ly_out *out, const char *filepath)
{
    FILE *f;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_FILEPATH, filepath ? NULL : ((void *)-1));

    if (!filepath) {
        return out->method.fpath.filepath;
    }

    /* replace filepath */
    f = out->method.fpath.f;
    out->method.fpath.f = fopen(filepath, "w");
    if (!out->method.fpath.f) {
        LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", filepath, strerror(errno));
        out->method.fpath.f = f;
        return ((void *)-1);
    }
    fclose(f);
    free(out->method.fpath.filepath);
    out->method.fpath.filepath = strdup(filepath);

    return NULL;
}

API void
ly_out_free(struct ly_out *out, void (*clb_arg_destructor)(void *arg), int destroy)
{
    if (!out) {
        return;
    }

    switch (out->type) {
    case LY_OUT_CALLBACK:
        if (clb_arg_destructor) {
            clb_arg_destructor(out->method.clb.arg);
        }
        break;
    case LY_OUT_FDSTREAM:
        fclose(out->method.fdstream.f);
        if (destroy) {
            close(out->method.fdstream.fd);
        }
        break;
    case LY_OUT_FD:
        if (destroy) {
            close(out->method.fd);
        }
        break;
    case LY_OUT_FILE:
        if (destroy) {
            fclose(out->method.f);
        }
        break;
    case LY_OUT_MEMORY:
        if (destroy) {
            free(*out->method.mem.buf);
        }
        break;
    case LY_OUT_FILEPATH:
        free(out->method.fpath.filepath);
        fclose(out->method.fpath.f);
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
    }
    free(out);
}

API ssize_t
ly_print(struct ly_out *out, const char *format, ...)
{
    int count = 0;
    char *msg = NULL, *aux;
    va_list ap;

    LYOUT_CHECK(out, -1 * out->status);

    va_start(ap, format);

    switch (out->type) {
    case LY_OUT_FD:
#ifdef HAVE_VDPRINTF
        count = vdprintf(out->method.fd, format, ap);
        break;
#else
        /* never should be here since ly_out_fd() is supposed to set type to LY_OUT_FDSTREAM in case vdprintf() is missing */
        LOGINT(NULL);
        va_end(ap);
        return -LY_EINT;
#endif
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
        count = vfprintf(out->method.f, format, ap);
        break;
    case LY_OUT_MEMORY:
        if ((count = vasprintf(&msg, format, ap)) < 0) {
            break;
        }
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            aux = ly_realloc(*out->method.mem.buf, out->method.mem.len + count + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                va_end(ap);
                return -LY_EMEM;
            }
            *out->method.mem.buf = aux;
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&(*out->method.mem.buf)[out->method.mem.len], msg, count);
        out->method.mem.len += count;
        (*out->method.mem.buf)[out->method.mem.len] = '\0';
        free(msg);
        break;
    case LY_OUT_CALLBACK:
        if ((count = vasprintf(&msg, format, ap)) < 0) {
            break;
        }
        count = out->method.clb.func(out->method.clb.arg, msg, count);
        free(msg);
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
        va_end(ap);
        return -LY_EINT;
    }

    va_end(ap);

    if (count < 0) {
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (%s).", __func__, strerror(errno));
        out->status = LY_ESYS;
        return -LY_ESYS;
    } else {
        if (out->type == LY_OUT_FDSTREAM) {
            /* move the original file descriptor to the end of the output file */
            lseek(out->method.fdstream.fd, 0, SEEK_END);
        }
        out->printed += count;
        return count;
    }
}

API void
ly_print_flush(struct ly_out *out)
{
    switch (out->type) {
    case LY_OUT_FDSTREAM:
        /* move the original file descriptor to the end of the output file */
        lseek(out->method.fdstream.fd, 0, SEEK_END);
        fflush(out->method.fdstream.f);
        break;
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
        fflush(out->method.f);
        break;
    case LY_OUT_FD:
        fsync(out->method.fd);
        break;
    case LY_OUT_MEMORY:
    case LY_OUT_CALLBACK:
        /* nothing to do */
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
    }

    free(out->buffered);
    out->buf_size = out->buf_len = 0;
}

API ssize_t
ly_write(struct ly_out *out, const char *buf, size_t len)
{
    int written = 0;

    LYOUT_CHECK(out, -1 * out->status);

    if (out->hole_count) {
        /* we are buffering data after a hole */
        if (out->buf_len + len > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + len);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM(NULL);
                return -LY_EMEM;
            }
            out->buf_size = out->buf_len + len;
        }

        memcpy(&out->buffered[out->buf_len], buf, len);
        out->buf_len += len;
        return len;
    }

repeat:
    switch (out->type) {
    case LY_OUT_MEMORY:
        if (out->method.mem.len + len + 1 > out->method.mem.size) {
            *out->method.mem.buf = ly_realloc(*out->method.mem.buf, out->method.mem.len + len + 1);
            if (!*out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                return -LY_EMEM;
            }
            out->method.mem.size = out->method.mem.len + len + 1;
        }
        memcpy(&(*out->method.mem.buf)[out->method.mem.len], buf, len);
        out->method.mem.len += len;
        (*out->method.mem.buf)[out->method.mem.len] = '\0';

        out->printed += len;
        return len;
    case LY_OUT_FD:
        written = write(out->method.fd, buf, len);
        break;
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
        written =  fwrite(buf, sizeof *buf, len, out->method.f);
        break;
    case LY_OUT_CALLBACK:
        written = out->method.clb.func(out->method.clb.arg, buf, len);
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return -LY_EINT;
    }

    if (written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            goto repeat;
        }
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (%s).", __func__, strerror(errno));
        out->status = LY_ESYS;
        return -LY_ESYS;
    } else if ((size_t)written != len) {
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (unable to write %u from %u data).", __func__, len - (size_t)written, len);
        out->status = LY_ESYS;
        return -LY_ESYS;
    } else {
        if (out->type == LY_OUT_FDSTREAM) {
            /* move the original file descriptor to the end of the output file */
            lseek(out->method.fdstream.fd, 0, SEEK_END);
        }
        out->printed += written;
        return written;
    }
}

ssize_t
ly_write_skip(struct ly_out *out, size_t count, size_t *position)
{
    LYOUT_CHECK(out, -1 * out->status);

    switch (out->type) {
    case LY_OUT_MEMORY:
        if (out->method.mem.len + count > out->method.mem.size) {
            *out->method.mem.buf = ly_realloc(*out->method.mem.buf, out->method.mem.len + count);
            if (!(*out->method.mem.buf)) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                out->status = LY_EMEM;
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
    case LY_OUT_FD:
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
    case LY_OUT_CALLBACK:
        /* buffer the hole */
        if (out->buf_len + count > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + count);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                out->status = LY_EMEM;
                LOGMEM(NULL);
                return -LY_EMEM;
            }
            out->buf_size = out->buf_len + count;
        }

        /* save the current position */
        *position = out->buf_len;

        /* skip the memory */
        out->buf_len += count;

        /* increase hole counter */
        ++out->hole_count;

        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return -LY_EINT;
    }

    return count;
}

ssize_t
ly_write_skipped(struct ly_out *out, size_t position, const char *buf, size_t count)
{
    ssize_t ret = LY_SUCCESS;

    LYOUT_CHECK(out, -1 * out->status);

    switch (out->type) {
    case LY_OUT_MEMORY:
        /* write */
        memcpy(&(*out->method.mem.buf)[position], buf, count);
        break;
    case LY_OUT_FD:
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
    case LY_OUT_CALLBACK:
        if (out->buf_len < position + count) {
            out->status = LY_EMEM;
            LOGMEM(NULL);
            return -LY_EMEM;
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
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return -LY_EINT;
    }

    if (out->type == LY_OUT_FILEPATH) {
        /* move the original file descriptor to the end of the output file */
        lseek(out->method.fdstream.fd, 0, SEEK_END);
    }
    return ret < 0 ? (-1 * ret) : LY_SUCCESS;
}
