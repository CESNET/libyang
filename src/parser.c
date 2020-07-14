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

#include "parser.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "compat.h"
#include "dict.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "tree_data_internal.h"
#include "tree_schema_internal.h"

API LY_IN_TYPE
ly_in_type(const struct ly_in *in)
{
    LY_CHECK_ARG_RET(NULL, in, LY_IN_ERROR);
    return in->type;
}

API LY_ERR
ly_in_new_fd(int fd, struct ly_in **in)
{
    size_t length;
    char *addr;

    LY_CHECK_ARG_RET(NULL, fd >= 0, in, LY_EINVAL);

    LY_CHECK_RET(ly_mmap(NULL, fd, &length, (void **)&addr));
    if (!addr) {
        LOGERR(NULL, LY_EINVAL, "Empty input file.");
        return LY_EINVAL;
    }

    *in = calloc(1, sizeof **in);
    LY_CHECK_ERR_RET(!*in, LOGMEM(NULL); ly_munmap(addr, length), LY_EMEM);

    (*in)->type = LY_IN_FD;
    (*in)->method.fd = fd;
    (*in)->current = (*in)->start = (*in)->func_start = addr;
    (*in)->length = length;

    return LY_SUCCESS;
}

API int
ly_in_fd(struct ly_in *in, int fd)
{
    int prev_fd;
    size_t length;
    const char *addr;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_FD, -1);

    prev_fd = in->method.fd;

    if (fd != -1) {
        LY_CHECK_RET(ly_mmap(NULL, fd, &length, (void **)&addr), -1);
        if (!addr) {
            LOGERR(NULL, LY_EINVAL, "Empty input file.");
            return -1;
        }

        ly_munmap((char*)in->start, in->length);

        in->method.fd = fd;
        in->current = in->start = addr;
        in->length = length;
    }

    return prev_fd;
}

API LY_ERR
ly_in_new_file(FILE *f, struct ly_in **in)
{
    LY_CHECK_ARG_RET(NULL, f, in, LY_EINVAL);

    LY_CHECK_RET(ly_in_new_fd(fileno(f), in));

    /* convert the LY_IN_FD input handler into the LY_IN_FILE */
    (*in)->type = LY_IN_FILE;
    (*in)->method.f = f;

    return LY_SUCCESS;
}

API FILE *
ly_in_file(struct ly_in *in, FILE *f)
{
    FILE *prev_f;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_FILE, NULL);

    prev_f = in->method.f;

    if (f) {
        /* convert LY_IN_FILE handler into LY_IN_FD to be able to update it via ly_in_fd() */
        in->type = LY_IN_FD;
        in->method.fd = fileno(prev_f);
        if (ly_in_fd(in, fileno(f)) == -1) {
            in->type = LY_IN_FILE;
            in->method.f = prev_f;
            return NULL;
        }

        /* if success, convert the result back */
        in->type = LY_IN_FILE;
        in->method.f = f;
    }

    return prev_f;
}

API LY_ERR
ly_in_new_memory(const char *str, struct ly_in **in)
{
    LY_CHECK_ARG_RET(NULL, str, in, LY_EINVAL);

    *in = calloc(1, sizeof **in);
    LY_CHECK_ERR_RET(!*in, LOGMEM(NULL), LY_EMEM);

    (*in)->type = LY_IN_MEMORY;
    (*in)->start = (*in)->current = (*in)->func_start = str;

    return LY_SUCCESS;
}

API const char *
ly_in_memory(struct ly_in *in, const char *str)
{
    const char *data;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_MEMORY, NULL);

    data = in->current;

    if (str) {
        in->start = in->current = str;
    }

    return data;
}

API LY_ERR
ly_in_reset(struct ly_in *in)
{
    LY_CHECK_ARG_RET(NULL, in, LY_EINVAL);

    in->current = in->func_start = in->start;
    return LY_SUCCESS;
}

API LY_ERR
ly_in_new_filepath(const char *filepath, size_t len, struct ly_in **in)
{
	LY_ERR ret;
    char *fp;
    int fd;

    LY_CHECK_ARG_RET(NULL, filepath, in, LY_EINVAL);

    if (len) {
        fp = strndup(filepath, len);
    } else {
        fp = strdup(filepath);
    }

    fd = open(fp, O_RDONLY);
    LY_CHECK_ERR_RET(fd == -1, LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", fp, strerror(errno)); free(fp),
                     LY_ESYS);

    LY_CHECK_ERR_RET(ret = ly_in_new_fd(fd, in), free(fp), ret);

    /* convert the LY_IN_FD input handler into the LY_IN_FILE */
    (*in)->type = LY_IN_FILEPATH;
    (*in)->method.fpath.fd = fd;
    (*in)->method.fpath.filepath = fp;

    return LY_SUCCESS;
}

API const char *
ly_in_filepath(struct ly_in *in, const char *filepath, size_t len)
{
    int fd, prev_fd;
    char *fp = NULL;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_FILEPATH, filepath ? NULL : ((void *)-1));

    if (!filepath) {
        return in->method.fpath.filepath;
    }

    if (len) {
        fp = strndup(filepath, len);
    } else {
        fp = strdup(filepath);
    }

    /* replace filepath */
    fd = open(fp, O_RDONLY);
    LY_CHECK_ERR_RET(!fd, LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", fp, strerror(errno)); free(fp), NULL);

    /* convert LY_IN_FILEPATH handler into LY_IN_FD to be able to update it via ly_in_fd() */
    in->type = LY_IN_FD;
    prev_fd = ly_in_fd(in, fd);
    LY_CHECK_ERR_RET(prev_fd == -1, in->type = LY_IN_FILEPATH; free(fp), NULL);

    /* and convert the result back */
    in->type = LY_IN_FILEPATH;
    close(prev_fd);
    free(in->method.fpath.filepath);
    in->method.fpath.fd = fd;
    in->method.fpath.filepath = fp;

    return NULL;
}

void
lys_parser_fill_filepath(struct ly_ctx *ctx, struct ly_in *in, const char **filepath)
{
    char path[PATH_MAX];
#ifndef __APPLE__
    char proc_path[32];
    int len;
#endif

    LY_CHECK_ARG_RET(NULL, ctx, in, filepath, );
    if (*filepath) {
        /* filepath already set */
        return;
    }

    switch (in->type) {
    case LY_IN_FILEPATH:
        if (realpath(in->method.fpath.filepath, path) != NULL) {
            *filepath = lydict_insert(ctx, path, 0);
        } else {
            *filepath = lydict_insert(ctx, in->method.fpath.filepath, 0);
        }

        break;
    case LY_IN_FD:
#ifdef __APPLE__
        if (fcntl(in->method.fd, F_GETPATH, path) != -1) {
            *filepath = lydict_insert(ctx, path, 0);
        }
#else
        /* get URI if there is /proc */
        sprintf(proc_path, "/proc/self/fd/%d", in->method.fd);
        if ((len = readlink(proc_path, path, PATH_MAX - 1)) > 0) {
            *filepath = lydict_insert(ctx, path, len);
        }
#endif
        break;
    case LY_IN_MEMORY:
    case LY_IN_FILE:
        /* nothing to do */
        break;
    default:
        LOGINT(ctx);
        break;
    }

}

API void
ly_in_free(struct ly_in *in, int destroy)
{
    if (!in) {
        return;
    } else if (in->type == LY_IN_ERROR) {
        LOGINT(NULL);
        return;
    }

    if (destroy) {
        if (in->type == LY_IN_MEMORY) {
            free((char*)in->start);
        } else {
            ly_munmap((char*)in->start, in->length);

            if (in->type == LY_IN_FILE) {
                fclose(in->method.f);
            } else {
                close(in->method.fd);

                if (in->type == LY_IN_FILEPATH) {
                    free(in->method.fpath.filepath);
                }
            }
        }
    } else if (in->type != LY_IN_MEMORY) {
        ly_munmap((char*)in->start, in->length);

        if (in->type == LY_IN_FILEPATH) {
            close(in->method.fpath.fd);
            free(in->method.fpath.filepath);
        }
    }

    free(in);
}

LY_ERR
ly_in_read(struct ly_in *in, void *buf, size_t count)
{
    if (in->length && (in->length - (in->current - in->start) < count)) {
        /* EOF */
        return LY_EDENIED;
    }

    memcpy(buf, in->current, count);
    in->current += count;
    return LY_SUCCESS;
}

API size_t
ly_in_parsed(const struct ly_in *in)
{
    return in->current - in->func_start;
}

LY_ERR
ly_in_skip(struct ly_in *in, size_t count)
{
    if (in->length && (in->length - (in->current - in->start) < count)) {
        /* EOF */
        return LY_EDENIED;
    }

    in->current += count;
    return LY_SUCCESS;
}

void
lyd_ctx_free(struct lyd_ctx *lydctx)
{
    ly_set_erase(&lydctx->unres_node_type, NULL);
    ly_set_erase(&lydctx->unres_meta_type, NULL);
    ly_set_erase(&lydctx->when_check, NULL);
}

LY_ERR
lyd_parser_check_schema(struct lyd_ctx *lydctx, const struct lysc_node *snode)
{
    /* alternatively, we could provide line for the error messages, but it doesn't work for the LYB format */

    if ((lydctx->parse_options & LYD_PARSE_NO_STATE) && (snode->flags & LYS_CONFIG_R)) {
        LOGVAL(lydctx->data_ctx->ctx, LY_VLOG_LYSC, snode, LY_VCODE_INNODE, "state", snode->name);
        return LY_EVALID;
    }

    if (snode->nodetype & (LYS_RPC | LYS_ACTION)) {
        if (lydctx->int_opts & LYD_INTOPT_RPC) {
            if (lydctx->op_node) {
                LOGVAL(lydctx->data_ctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\", %s \"%s\" already parsed.",
                       lys_nodetype2str(snode->nodetype), snode->name,
                       lys_nodetype2str(lydctx->op_node->schema->nodetype), lydctx->op_node->schema->name);
                return LY_EVALID;
            }
        } else {
            LOGVAL(lydctx->data_ctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\".",
                   lys_nodetype2str(snode->nodetype), snode->name);
            return LY_EVALID;
        }
    } else if (snode->nodetype == LYS_NOTIF) {
        if (lydctx->int_opts & LYD_INTOPT_NOTIF) {
            if (lydctx->op_node) {
                LOGVAL(lydctx->data_ctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\", %s \"%s\" already parsed.",
                       lys_nodetype2str(snode->nodetype), snode->name,
                       lys_nodetype2str(lydctx->op_node->schema->nodetype), lydctx->op_node->schema->name);
                return LY_EVALID;
            }
        } else {
            LOGVAL(lydctx->data_ctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\".",
                   lys_nodetype2str(snode->nodetype), snode->name);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_parser_create_term(struct lyd_ctx *lydctx, const struct lysc_node *schema, const char *value, size_t value_len,
                       int *dynamic, int value_hints, ly_resolve_prefix_clb get_prefix, void *prefix_data,
                       LYD_FORMAT format, struct lyd_node **node)
{
    LY_ERR ret;

    ret = lyd_create_term(schema, value, value_len, dynamic, value_hints, get_prefix, prefix_data, format, node);
    if (ret == LY_EINCOMPLETE) {
        if (!(lydctx->parse_options & LYD_PARSE_ONLY)) {
            ly_set_add(&lydctx->unres_node_type, *node, LY_SET_OPT_USEASLIST);
        }
        ret = LY_SUCCESS;
    }
    return ret;
}

LY_ERR
lyd_parser_create_meta(struct lyd_ctx *lydctx, struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod,
                       const char *name, size_t name_len, const char *value, size_t value_len, int *dynamic, int value_hints,
                       ly_resolve_prefix_clb resolve_prefix, void *prefix_data, LYD_FORMAT format, const struct lysc_node *ctx_snode)
{
    LY_ERR ret;
    ret = lyd_create_meta(parent, meta, mod, name, name_len, value, value_len, dynamic, value_hints, resolve_prefix, prefix_data, format, ctx_snode);
    if (ret == LY_EINCOMPLETE) {
        ly_set_add(&lydctx->unres_meta_type, *meta, LY_SET_OPT_USEASLIST);
        ret = LY_SUCCESS;
    }
    return ret;
}
