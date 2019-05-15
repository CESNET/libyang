/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"

static int
cmp_str(const char *refstr, const char *str, size_t strlen)
{

    if (strlen) {
        int r = strncmp(refstr, str, strlen);
        if (!r && !refstr[strlen]) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return strcmp(refstr, str);
    }
}

API const struct lyd_node *
lyd_search(const struct lyd_node *first, const struct lys_module *module,
          const char *name, size_t name_len, uint16_t nodetype, const char *value, size_t value_len)
{
    const struct lyd_node *node = NULL;
    const struct lysc_node *snode;

    LY_CHECK_ARG_RET(NULL, module, name, NULL);
    if (!nodetype) {
        nodetype = 0xffff;
    }

    LY_LIST_FOR(first, node) {
        snode = node->schema;
        if (!(snode->nodetype & nodetype)) {
            continue;
        }
        if (snode->module != module) {
            continue;
        }

        if (cmp_str(snode->name, name, name_len)) {
            continue;
        }

        if (value) {
            if (snode->nodetype == LYS_LIST) {
                /* TODO handle value as keys of the list instance */
            } else if (snode->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                if (cmp_str(((struct lyd_node_term*)node)->value.canonized, value, value_len)) {
                    continue;
                }
            } else {
                continue;
            }
        }

        /* all criteria passed */
        return node;
    }
    return NULL;
}

static struct lyd_node *
lyd_parse_mem_(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, va_list ap)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *result = NULL;
    const struct lyd_node *rpc_act = NULL, *data_tree = NULL, *iter;
    const char *yang_data_name = NULL;

    if (lyd_parse_check_options(ctx, options, __func__)) {
        return NULL;
    }

    if (options & LYD_OPT_RPCREPLY) {
        rpc_act = va_arg(ap, const struct lyd_node *);
        if (!rpc_act || rpc_act->parent || !(rpc_act->schema->nodetype & (LYS_ACTION | LYS_LIST | LYS_CONTAINER))) {
            LOGERR(ctx, LY_EINVAL, "Data parser invalid variable parameter (const struct lyd_node *rpc_act).");
            return NULL;
        }
    }
    if (options & (LYD_OPT_RPC | LYD_OPT_NOTIF | LYD_OPT_RPCREPLY)) {
        data_tree = va_arg(ap, const struct lyd_node *);
        if (data_tree) {
            if (options & LYD_OPT_NOEXTDEPS) {
                LOGERR(ctx, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree and LYD_OPT_NOEXTDEPS set).",
                       __func__);
                return NULL;
            }

            LY_LIST_FOR(data_tree, iter) {
                if (iter->parent) {
                    /* a sibling is not top-level */
                    LOGERR(ctx, LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *data_tree).", __func__);
                    return NULL;
                }
            }

            /* move it to the beginning */
            for (; data_tree->prev->next; data_tree = data_tree->prev);

            /* LYD_OPT_NOSIBLINGS cannot be set in this case */
            if (options & LYD_OPT_NOSIBLINGS) {
                LOGERR(ctx, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree with LYD_OPT_NOSIBLINGS).", __func__);
                return NULL;
            }
        }
    }
    if (options & LYD_OPT_DATA_TEMPLATE) {
        yang_data_name = va_arg(ap, const char *);
    }

    if (!format) {
        /* TODO try to detect format from the content */
    }

    switch (format) {
    case LYD_XML:
        ret = lyd_parse_xml(ctx, data, options, &result);
        break;
#if 0
    case LYD_JSON:
        ret = lyd_parse_json(ctx, data, options, rpc_act, data_tree, yang_data_name);
        break;
    case LYD_LYB:
        ret = lyd_parse_lyb(ctx, data, options, data_tree, yang_data_name, NULL);
        break;
#endif
    case LYD_UNKNOWN:
        LOGINT(ctx);
        break;
    }

    if (ret) {
        lyd_free_all(result);
        result = NULL;
    }

    return result;
}

API struct lyd_node *
lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, ...)
{
    va_list ap;
    struct lyd_node *result;

    va_start(ap, options);
    result = lyd_parse_mem_(ctx, data, format, options, ap);
    va_end(ap);

    return result;
}

static struct lyd_node *
lyd_parse_fd_(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, va_list ap)
{
    struct lyd_node *result;
    size_t length;
    char *addr;

    LY_CHECK_ARG_RET(ctx, ctx, NULL);
    if (fd < 0) {
        LOGARG(ctx, fd);
        return NULL;
    }

    LY_CHECK_RET(ly_mmap(ctx, fd, &length, (void **)&addr), NULL);
    result = lyd_parse_mem_(ctx, addr ? addr : "", format, options, ap);
    ly_munmap(addr, length);

    return result;
}

API struct lyd_node *
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...)
{
    struct lyd_node *ret;
    va_list ap;

    va_start(ap, options);
    ret = lyd_parse_fd_(ctx, fd, format, options, ap);
    va_end(ap);

    return ret;
}

API struct lyd_node *
lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, ...)
{
    int fd;
    struct lyd_node *result;
    size_t len;
    va_list ap;

    LY_CHECK_ARG_RET(ctx, ctx, path, NULL);

    fd = open(path, O_RDONLY);
    LY_CHECK_ERR_RET(fd == -1, LOGERR(ctx, LY_ESYS, "Opening file \"%s\" failed (%s).", path, strerror(errno)), NULL);

    if (!format) {
        /* unknown format - try to detect it from filename's suffix */
        len = strlen(path);

        /* ignore trailing whitespaces */
        for (; len > 0 && isspace(path[len - 1]); len--);

        if (len >= 5 && !strncmp(&path[len - 4], ".xml", 4)) {
            format = LYD_XML;
#if 0
        } else if (len >= 6 && !strncmp(&path[len - 5], ".json", 5)) {
            format = LYD_JSON;
        } else if (len >= 5 && !strncmp(&path[len - 4], ".lyb", 4)) {
            format = LYD_LYB;
#endif
        } /* else still unknown, try later to detect it from the content */
    }

    va_start(ap, options);
    result = lyd_parse_fd_(ctx, fd, format, options, ap);

    va_end(ap);
    close(fd);

    return result;
}




