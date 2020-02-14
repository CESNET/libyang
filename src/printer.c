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

#include "common.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "log.h"
#include "printer_internal.h"
#include "plugins_types.h"

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

/**
 * @brief Check whether a node value equals to its default one.
 *
 * @param[in] node Term node to test.
 * @return 0 if no,
 * @return non-zero if yes.
 */
static int
ly_is_default(const struct lyd_node *node)
{
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lyd_node_term *term;
    size_t i;

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

        LY_ARRAY_FOR(llist->dflts, i) {
            /* compare with each possible default value */
            if (llist->type->plugin->compare(&term->value, llist->dflts[i])) {
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
        /* LYP_WD_EXPLICIT
         * - print only if it contains status data in its subtree */
        LYD_TREE_DFS_BEGIN(node, next, elem) {
            if (elem->schema->flags & LYS_CONFIG_R) {
                return 1;
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
        fsync(out->method.fd);
        break;
    case LYOUT_MEMORY:
    case LYOUT_CALLBACK:
        /* nothing to do */
        break;
    }

    free(out->buffered);
    out->buf_size = out->buf_len = 0;
}

LY_ERR
ly_write(struct lyout *out, const char *buf, size_t len)
{
    int written = 0;

    LYOUT_CHECK(out, out->status);

    if (out->hole_count) {
        /* we are buffering data after a hole */
        if (out->buf_len + len > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + len);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM_RET(NULL);
            }
            out->buf_size = out->buf_len + len;
        }

        memcpy(&out->buffered[out->buf_len], buf, len);
        out->buf_len += len;
        return LY_SUCCESS;
    }

repeat:
    switch (out->type) {
    case LYOUT_MEMORY:
        if (out->method.mem.len + len + 1 > out->method.mem.size) {
            out->method.mem.buf = ly_realloc(out->method.mem.buf, out->method.mem.len + len + 1);
            if (!out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM_RET(NULL);
            }
            out->method.mem.size = out->method.mem.len + len + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], buf, len);
        out->method.mem.len += len;
        out->method.mem.buf[out->method.mem.len] = '\0';

        out->printed += len;
        return LY_SUCCESS;
    case LYOUT_FD:
        written = write(out->method.fd, buf, len);
        break;
    case LYOUT_FDSTREAM:
    case LYOUT_STREAM:
        written =  fwrite(buf, sizeof *buf, len, out->method.f);
        break;
    case LYOUT_CALLBACK:
        written = out->method.clb.f(out->method.clb.arg, buf, len);
        break;
    }

    if (written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            goto repeat;
        }
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (%s).", __func__, strerror(errno));
        out->status = LY_ESYS;
        return LY_ESYS;
    } else if ((size_t)written != len) {
        LOGERR(out->ctx, LY_ESYS, "%s: writing data failed (unable to write %u from %u data).", __func__, len - (size_t)written, len);
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
