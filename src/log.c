/**
 * @file log.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Logger routines implementations
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include "log.h"

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "in_internal.h"
#include "plugins_exts.h"
#include "set.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

ATOMIC_T ly_ll = (uint_fast32_t)LY_LLWRN;
ATOMIC_T ly_log_opts = (uint_fast32_t)(LY_LOLOG | LY_LOSTORE_LAST);
THREAD_LOCAL uint32_t *temp_ly_log_opts;
static ly_log_clb log_clb;
static ATOMIC_T path_flag = 1;
THREAD_LOCAL char last_msg[LY_LAST_MSG_SIZE];
#ifndef NDEBUG
ATOMIC_T ly_ldbg_groups = 0;
#endif

THREAD_LOCAL struct ly_log_location_s log_location = {0};

LIBYANG_API_DEF const char *
ly_last_errmsg(void)
{
    return last_msg;
}

LIBYANG_API_DEF LY_ERR
ly_errcode(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    i = ly_err_last(ctx);
    if (i) {
        return i->no;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_VECODE
ly_vecode(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    i = ly_err_last(ctx);
    if (i) {
        return i->vecode;
    }

    return LYVE_SUCCESS;
}

LIBYANG_API_DEF const char *
ly_errmsg(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    i = ly_err_last(ctx);
    if (i) {
        return i->msg;
    }

    return NULL;
}

LIBYANG_API_DEF const char *
ly_errpath(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    i = ly_err_last(ctx);
    if (i) {
        return i->path;
    }

    return NULL;
}

LIBYANG_API_DEF const char *
ly_errapptag(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    i = ly_err_last(ctx);
    if (i) {
        return i->apptag;
    }

    return NULL;
}

LIBYANG_API_DEF LY_ERR
ly_err_new(struct ly_err_item **err, LY_ERR ecode, LY_VECODE vecode, char *path, char *apptag, const char *err_format, ...)
{
    char *msg = NULL;
    struct ly_err_item *e;

    if (!err || (ecode == LY_SUCCESS)) {
        /* nothing to do */
        return ecode;
    }

    e = malloc(sizeof *e);
    LY_CHECK_ERR_RET(!e, LOGMEM(NULL), LY_EMEM);
    e->prev = (*err) ? (*err)->prev : e;
    e->next = NULL;
    if (*err) {
        (*err)->prev->next = e;
    }

    /* fill in the information */
    e->level = LY_LLERR;
    e->no = ecode;
    e->vecode = vecode;
    e->path = path;
    e->apptag = apptag;

    if (err_format) {
        va_list print_args;

        va_start(print_args, err_format);

        if (vasprintf(&msg, err_format, print_args) == -1) {
            /* we don't have anything more to do, just set msg to NULL to avoid undefined content,
             * still keep the information about the original error instead of LY_EMEM or other printf's error */
            msg = NULL;
        }

        va_end(print_args);
    }
    e->msg = msg;

    if (!(*err)) {
        *err = e;
    }

    return e->no;
}

LIBYANG_API_DEF struct ly_err_item *
ly_err_first(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    return ctx->errlist;
}

LIBYANG_API_DEF struct ly_err_item *
ly_err_last(const struct ly_ctx *ctx)
{
    const struct ly_err_item *e;

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    e = ctx->errlist;
    return e ? e->prev : NULL;
}

void
ly_err_move(struct ly_ctx *src_ctx, struct ly_ctx *trg_ctx)
{
    const struct ly_err_item *e;

    /* clear any current errors */
    ly_err_clean(trg_ctx, NULL);

    /* get the errors in src */
    e = src_ctx->errlist;
    src_ctx->errlist = NULL;

    /* set them for trg */
    trg_ctx->errlist = e;
}

LIBYANG_API_DEF void
ly_err_free(void *ptr)
{
    struct ly_err_item *i, *next;

    /* clean the error list */
    for (i = (struct ly_err_item *)ptr; i; i = next) {
        next = i->next;
        free(i->msg);
        free(i->path);
        free(i->apptag);
        free(i);
    }
}

LIBYANG_API_DEF void
ly_err_clean(struct ly_ctx *ctx, struct ly_err_item *eitem)
{
    struct ly_err_item *i, *first;

    first = ly_err_first(ctx);
    if (first == eitem) {
        eitem = NULL;
    }
    if (eitem) {
        /* disconnect the error */
        for (i = first; i && (i->next != eitem); i = i->next) {}
        assert(i);
        i->next = NULL;
        first->prev = i;
        /* free this err and newer */
        ly_err_free(eitem);
    } else {
        /* free all err */
        ly_err_free(first);
        ctx->errlist = NULL;
    }
}

LIBYANG_API_DEF LY_LOG_LEVEL
ly_log_level(LY_LOG_LEVEL level)
{
    LY_LOG_LEVEL prev = ATOMIC_LOAD_RELAXED(ly_ll);

    ATOMIC_STORE_RELAXED(ly_ll, level);
    return prev;
}

LIBYANG_API_DEF uint32_t
ly_log_options(uint32_t opts)
{
    uint32_t prev = ATOMIC_LOAD_RELAXED(ly_log_opts);

    ATOMIC_STORE_RELAXED(ly_log_opts, opts);
    return prev;
}

LIBYANG_API_DEF void
ly_temp_log_options(uint32_t *opts)
{
    temp_ly_log_opts = opts;
}

LIBYANG_API_DEF uint32_t
ly_log_dbg_groups(uint32_t dbg_groups)
{
#ifndef NDEBUG
    uint32_t prev = ATOMIC_LOAD_RELAXED(ly_ldbg_groups);

    ATOMIC_STORE_RELAXED(ly_ldbg_groups, dbg_groups);
    return prev;
#else
    (void)dbg_groups;
    return 0;
#endif
}

LIBYANG_API_DEF void
ly_set_log_clb(ly_log_clb clb, ly_bool path)
{
    log_clb = clb;
    ATOMIC_STORE_RELAXED(path_flag, path);
}

LIBYANG_API_DEF ly_log_clb
ly_get_log_clb(void)
{
    return log_clb;
}

void
ly_log_location(const struct lysc_node *scnode, const struct lyd_node *dnode, const char *path, const struct ly_in *in,
        uint64_t line)
{
    if (scnode) {
        ly_set_add(&log_location.scnodes, (void *)scnode, 1, NULL);
    }
    if (dnode) {
        ly_set_add(&log_location.dnodes, (void *)dnode, 1, NULL);
    }
    if (path) {
        char *s = strdup(path);

        LY_CHECK_ERR_RET(!s, LOGMEM(NULL), );
        ly_set_add(&log_location.paths, s, 1, NULL);
    }
    if (in) {
        ly_set_add(&log_location.inputs, (void *)in, 1, NULL);
    }
    if (line) {
        log_location.line = line;
    }
}

void
ly_log_location_revert(uint32_t scnode_steps, uint32_t dnode_steps, uint32_t path_steps, uint32_t in_steps)
{
    for (uint32_t i = scnode_steps; i && log_location.scnodes.count; i--) {
        log_location.scnodes.count--;
    }

    for (uint32_t i = dnode_steps; i && log_location.dnodes.count; i--) {
        log_location.dnodes.count--;
    }

    for (uint32_t i = path_steps; i && log_location.paths.count; i--) {
        ly_set_rm_index(&log_location.paths, log_location.paths.count - 1, free);
    }

    for (uint32_t i = in_steps; i && log_location.inputs.count; i--) {
        log_location.inputs.count--;
    }

    /* deallocate the empty sets */
    if (scnode_steps && !log_location.scnodes.count) {
        ly_set_erase(&log_location.scnodes, NULL);
    }
    if (dnode_steps && !log_location.dnodes.count) {
        ly_set_erase(&log_location.dnodes, NULL);
    }
    if (path_steps && !log_location.paths.count) {
        ly_set_erase(&log_location.paths, free);
    }
    if (in_steps && !log_location.inputs.count) {
        ly_set_erase(&log_location.inputs, NULL);
    }
}

static LY_ERR
log_store(struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, LY_VECODE vecode, char *msg, char *path, char *apptag)
{
    struct ly_err_item *eitem, *last;

    assert(ctx && (level < LY_LLVRB));

    eitem = ctx->errlist;
    if (!eitem) {
        /* if we are only to fill in path, there must have been an error stored */
        assert(msg);
        eitem = malloc(sizeof *eitem);
        LY_CHECK_GOTO(!eitem, mem_fail);
        eitem->prev = eitem;
        eitem->next = NULL;

        ctx->errlist = eitem;
    } else if (!msg) {
        /* only filling the path */
        assert(path);

        /* find last error */
        eitem = eitem->prev;
        do {
            if (eitem->level == LY_LLERR) {
                /* fill the path */
                free(eitem->path);
                eitem->path = path;
                return LY_SUCCESS;
            }
            eitem = eitem->prev;
        } while (eitem->prev->next);
        /* last error was not found */
        assert(0);
    } else if ((temp_ly_log_opts && ((*temp_ly_log_opts & LY_LOSTORE_LAST) == LY_LOSTORE_LAST)) ||
            (!temp_ly_log_opts && ((ATOMIC_LOAD_RELAXED(ly_log_opts) & LY_LOSTORE_LAST) == LY_LOSTORE_LAST))) {
        /* overwrite last message */
        free(eitem->msg);
        free(eitem->path);
        free(eitem->apptag);
    } else {
        /* store new message */
        last = eitem->prev;
        eitem->prev = malloc(sizeof *eitem);
        LY_CHECK_GOTO(!eitem->prev, mem_fail);
        eitem = eitem->prev;
        eitem->prev = last;
        eitem->next = NULL;
        last->next = eitem;
    }

    /* fill in the information */
    eitem->level = level;
    eitem->no = no;
    eitem->vecode = vecode;
    eitem->msg = msg;
    eitem->path = path;
    eitem->apptag = apptag;
    return LY_SUCCESS;

mem_fail:
    LOGMEM(NULL);
    free(msg);
    free(path);
    free(apptag);
    return LY_EMEM;
}

static void
log_vprintf(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, LY_VECODE vecode, char *path, const char *apptag,
        const char *format, va_list args)
{
    char *msg = NULL;
    ly_bool free_strs, lolog, lostore;

    /* learn effective logger options */
    if (temp_ly_log_opts) {
        lolog = *temp_ly_log_opts & LY_LOLOG;
        lostore = *temp_ly_log_opts & LY_LOSTORE;
    } else {
        lolog = ATOMIC_LOAD_RELAXED(ly_log_opts) & LY_LOLOG;
        lostore = ATOMIC_LOAD_RELAXED(ly_log_opts) & LY_LOSTORE;
    }

    if (level > ATOMIC_LOAD_RELAXED(ly_ll)) {
        /* do not print or store the message */
        free(path);
        return;
    }

    if (no == LY_EMEM) {
        /* just print it, anything else would most likely fail anyway */
        if (lolog) {
            if (log_clb) {
                log_clb(level, LY_EMEM_MSG, path);
            } else {
                fprintf(stderr, "libyang[%d]: ", level);
                vfprintf(stderr, format, args);
                if (path) {
                    fprintf(stderr, " (path: %s)\n", path);
                } else {
                    fprintf(stderr, "\n");
                }
            }
        }
        free(path);
        return;
    }

    /* print into a single message */
    if (vasprintf(&msg, format, args) == -1) {
        LOGMEM(ctx);
        free(path);
        return;
    }

    /* store as the last message */
    strncpy(last_msg, msg, LY_LAST_MSG_SIZE - 1);

    /* store the error/warning in the context (if we need to store errors internally, it does not matter what are
     * the user log options) */
    if ((level < LY_LLVRB) && ctx && lostore) {
        if (((no & ~LY_EPLUGIN) == LY_EVALID) && (vecode == LYVE_SUCCESS)) {
            /* assume we are inheriting the error, so inherit vecode as well */
            vecode = ly_vecode(ctx);
        }
        if (log_store(ctx, level, no, vecode, msg, path, apptag ? strdup(apptag) : NULL)) {
            return;
        }
        free_strs = 0;
    } else {
        free_strs = 1;
    }

    /* if we are only storing errors internally, never print the message (yet) */
    if (lolog) {
        if (log_clb) {
            log_clb(level, msg, path);
        } else {
            fprintf(stderr, "libyang[%d]: %s%s", level, msg, path ? " " : "\n");
            if (path) {
                fprintf(stderr, "(path: %s)\n", path);
            }
        }
    }

    if (free_strs) {
        free(path);
        free(msg);
    }
}

#ifndef NDEBUG

void
ly_log_dbg(uint32_t group, const char *format, ...)
{
    char *dbg_format;
    const char *str_group;
    va_list ap;

    if (!(ATOMIC_LOAD_RELAXED(ly_ldbg_groups) & group)) {
        return;
    }

    switch (group) {
    case LY_LDGDICT:
        str_group = "DICT";
        break;
    case LY_LDGXPATH:
        str_group = "XPATH";
        break;
    case LY_LDGDEPSETS:
        str_group = "DEPSETS";
        break;
    default:
        LOGINT(NULL);
        return;
    }

    if (asprintf(&dbg_format, "%s: %s", str_group, format) == -1) {
        LOGMEM(NULL);
        return;
    }

    va_start(ap, format);
    log_vprintf(NULL, LY_LLDBG, 0, 0, NULL, NULL, dbg_format, ap);
    va_end(ap);
}

#endif

void
ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(ctx, level, no, 0, NULL, NULL, format, ap);
    va_end(ap);
}

/**
 * @brief Append a schema node name to a generated data path, only if it fits.
 *
 * @param[in,out] str Generated path to update.
 * @param[in] snode Schema node to append.
 * @param[in] parent Last printed data node.
 * @return LY_ERR value.
 */
static LY_ERR
ly_vlog_build_path_append(char **str, const struct lysc_node *snode, const struct lyd_node *parent)
{
    const struct lys_module *mod, *prev_mod;
    uint32_t len, new_len;
    void *mem;

    if (snode->nodetype & (LYS_CHOICE | LYS_CASE)) {
        /* schema-only node */
        return LY_SUCCESS;
    } else if (lysc_data_parent(snode) != parent->schema) {
        /* not a direct descendant node */
        return LY_SUCCESS;
    }

    /* get module to print, if any */
    mod = snode->module;
    prev_mod = (parent->schema) ? parent->schema->module : lyd_owner_module(parent);
    if (prev_mod == mod) {
        mod = NULL;
    }

    /* realloc string */
    len = strlen(*str);
    new_len = len + 1 + (mod ? strlen(mod->name) + 1 : 0) + strlen(snode->name);
    mem = realloc(*str, new_len + 1);
    LY_CHECK_ERR_RET(!mem, LOGMEM(LYD_CTX(parent)), LY_EMEM);
    *str = mem;

    /* print the last schema node */
    sprintf(*str + len, "/%s%s%s", mod ? mod->name : "", mod ? ":" : "", snode->name);
    return LY_SUCCESS;
}

/**
 * @brief Build log path from the stored log location information.
 *
 * @param[in] ctx Context to use.
 * @param[out] path Generated log path.
 * @return LY_ERR value.
 */
static LY_ERR
ly_vlog_build_path(const struct ly_ctx *ctx, char **path)
{
    int r;
    char *str = NULL, *prev = NULL;
    const struct lyd_node *dnode;

    *path = NULL;

    if (log_location.paths.count && ((const char *)(log_location.paths.objs[log_location.paths.count - 1]))[0]) {
        /* simply get what is in the provided path string */
        r = asprintf(path, "Path \"%s\".", (const char *)log_location.paths.objs[log_location.paths.count - 1]);
        LY_CHECK_ERR_RET(r == -1, LOGMEM(ctx), LY_EMEM);
    } else {
        /* data/schema node */
        if (log_location.dnodes.count) {
            dnode = log_location.dnodes.objs[log_location.dnodes.count - 1];
            if (dnode->parent || !lysc_data_parent(dnode->schema)) {
                /* data node with all of its parents */
                str = lyd_path(log_location.dnodes.objs[log_location.dnodes.count - 1], LYD_PATH_STD, NULL, 0);
                LY_CHECK_ERR_RET(!str, LOGMEM(ctx), LY_EMEM);
            } else {
                /* data parsers put all the parent nodes in the set, but they are not connected */
                str = lyd_path_set(&log_location.dnodes, LYD_PATH_STD);
                LY_CHECK_ERR_RET(!str, LOGMEM(ctx), LY_EMEM);
            }

            /* sometimes the last node is not created yet and we only have the schema node */
            if (log_location.scnodes.count) {
                ly_vlog_build_path_append(&str, log_location.scnodes.objs[log_location.scnodes.count - 1], dnode);
            }

            r = asprintf(path, "Data location \"%s\"", str);
            free(str);
            LY_CHECK_ERR_RET(r == -1, LOGMEM(ctx), LY_EMEM);
        } else if (log_location.scnodes.count) {
            str = lysc_path(log_location.scnodes.objs[log_location.scnodes.count - 1], LYSC_PATH_LOG, NULL, 0);
            LY_CHECK_ERR_RET(!str, LOGMEM(ctx), LY_EMEM);

            r = asprintf(path, "Schema location \"%s\"", str);
            free(str);
            LY_CHECK_ERR_RET(r == -1, LOGMEM(ctx), LY_EMEM);
        }

        /* line */
        prev = *path;
        if (log_location.line) {
            r = asprintf(path, "%s%sine number %" PRIu64, prev ? prev : "", prev ? ", l" : "L", log_location.line);
            free(prev);
            LY_CHECK_ERR_RET(r == -1, LOGMEM(ctx), LY_EMEM);

            log_location.line = 0;
        } else if (log_location.inputs.count) {
            r = asprintf(path, "%s%sine number %" PRIu64, prev ? prev : "", prev ? ", l" : "L",
                    ((struct ly_in *)log_location.inputs.objs[log_location.inputs.count - 1])->line);
            free(prev);
            LY_CHECK_ERR_RET(r == -1, LOGMEM(ctx), LY_EMEM);
        }

        if (*path) {
            prev = *path;
            r = asprintf(path, "%s.", prev);
            free(prev);
            LY_CHECK_ERR_RET(r == -1, LOGMEM(ctx), LY_EMEM);
        }
    }

    return LY_SUCCESS;
}

void
ly_vlog(const struct ly_ctx *ctx, const char *apptag, LY_VECODE code, const char *format, ...)
{
    va_list ap;
    char *path = NULL;

    if (ATOMIC_LOAD_RELAXED(path_flag) && ctx) {
        ly_vlog_build_path(ctx, &path);
    }

    va_start(ap, format);
    log_vprintf(ctx, LY_LLERR, LY_EVALID, code, path, apptag, format, ap);
    /* path is spent and should not be freed! */
    va_end(ap);
}

/**
 * @brief Print a log message from an extension plugin callback.
 *
 * @param[in] ctx libyang context to store the error record. If not provided, the error is just printed.
 * @param[in] plugin_name Name of the plugin generating the message.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] err_no Error type code.
 * @param[in] path Optional path of the error, used if set.
 * @param[in] format Format string to print.
 * @param[in] ap Var arg list for @p format.
 */
static void
ly_ext_log(const struct ly_ctx *ctx, const char *plugin_name, LY_LOG_LEVEL level, LY_ERR err_no, char *path,
        const char *format, va_list ap)
{
    char *plugin_msg;

    if (ATOMIC_LOAD_RELAXED(ly_ll) < level) {
        return;
    }
    if (asprintf(&plugin_msg, "Ext plugin \"%s\": %s", plugin_name, format) == -1) {
        LOGMEM(ctx);
        return;
    }

    log_vprintf(ctx, level, (level == LY_LLERR ? LY_EPLUGIN : 0) | err_no, LYVE_OTHER, path, NULL, plugin_msg, ap);
    free(plugin_msg);
}

LIBYANG_API_DEF void
lyplg_ext_parse_log(const struct lysp_ctx *pctx, const struct lysp_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err_no,
        const char *format, ...)
{
    va_list ap;
    char *path = NULL;

    if (ATOMIC_LOAD_RELAXED(path_flag)) {
        ly_vlog_build_path(PARSER_CTX(pctx), &path);
    }

    va_start(ap, format);
    ly_ext_log(PARSER_CTX(pctx), ext->record->plugin.id, level, err_no, path, format, ap);
    va_end(ap);
}

LIBYANG_API_DEF void
lyplg_ext_compile_log(const struct lysc_ctx *cctx, const struct lysc_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err_no,
        const char *format, ...)
{
    va_list ap;
    char *path = NULL;

    if (cctx && (asprintf(&path, "Path \"%s\".", cctx->path) == -1)) {
        LOGMEM(cctx->ctx);
        return;
    }

    va_start(ap, format);
    ly_ext_log(ext->module->ctx, ext->def->plugin->id, level, err_no, path, format, ap);
    va_end(ap);
}

LIBYANG_API_DEF void
lyplg_ext_compile_log_path(const char *path, const struct lysc_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err_no,
        const char *format, ...)
{
    va_list ap;
    char *log_path = NULL;

    if (path && (asprintf(&log_path, "Path \"%s\".", path) == -1)) {
        LOGMEM(ext->module->ctx);
        return;
    }

    va_start(ap, format);
    ly_ext_log(ext->module->ctx, ext->def->plugin->id, level, err_no, log_path, format, ap);
    va_end(ap);
}

/**
 * @brief Serves only for creating ap.
 */
static void
_lyplg_ext_compile_log_err(const struct ly_err_item *err, const struct lysc_ext_instance *ext, ...)
{
    va_list ap;

    va_start(ap, ext);
    ly_ext_log(ext->module->ctx, ext->def->plugin->id, err->level, err->no, err->path ? strdup(err->path) : NULL, "%s", ap);
    va_end(ap);
}

LIBYANG_API_DEF void
lyplg_ext_compile_log_err(const struct ly_err_item *err, const struct lysc_ext_instance *ext)
{
    _lyplg_ext_compile_log_err(err, ext, err->msg);
}

/**
 * @brief Exact same functionality as ::ly_err_print() but has variable arguments so log_vprintf() can be called.
 */
static void
_ly_err_print(const struct ly_ctx *ctx, struct ly_err_item *eitem, const char *format, ...)
{
    va_list ap;
    char *path_dup = NULL;

    LY_CHECK_ARG_RET(ctx, eitem, );

    if (eitem->path) {
        /* duplicate path because it will be freed */
        path_dup = strdup(eitem->path);
        LY_CHECK_ERR_RET(!path_dup, LOGMEM(ctx), );
    }

    va_start(ap, format);
    log_vprintf(ctx, eitem->level, eitem->no, eitem->vecode, path_dup, eitem->apptag, format, ap);
    va_end(ap);
}

LIBYANG_API_DEF void
ly_err_print(const struct ly_ctx *ctx, struct ly_err_item *eitem)
{
    /* String ::ly_err_item.msg cannot be used directly because it may contain the % character */
    _ly_err_print(ctx, eitem, "%s", eitem->msg);
}
