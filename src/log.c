/**
 * @file log.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Logger routines implementations
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
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

#include "compat.h"
#include "in_internal.h"
#include "ly_common.h"
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
THREAD_LOCAL char last_msg[LY_LAST_MSG_SIZE];
#ifndef NDEBUG
ATOMIC_T ly_ldbg_groups = 0;
#endif

THREAD_LOCAL struct ly_log_location_s log_location = {0};

LIBYANG_API_DEF const char *
ly_strerr(LY_ERR err)
{
    /* ignore plugin flag */
    err &= ~LY_EPLUGIN;

    switch (err) {
    case LY_SUCCESS:
        return "Success";
    case LY_EMEM:
        return "Out of memory";
    case LY_ESYS:
        return "System call failed";
    case LY_EINVAL:
        return "Invalid value";
    case LY_EEXIST:
        return "Already exists";
    case LY_ENOTFOUND:
        return "Not found";
    case LY_EINT:
        return "Internal error";
    case LY_EVALID:
        return "Validation failed";
    case LY_EDENIED:
        return "Operation denied";
    case LY_EINCOMPLETE:
        return "Operation incomplete";
    case LY_ERECOMPILE:
        return "Recompilation required";
    case LY_ENOT:
        return "Negative result";
    case LY_EOTHER:
        return "Another failure reason";
    case LY_EPLUGIN:
        break;
    }

    /* unreachable */
    return "Unknown";
}

LIBYANG_API_DEF const char *
ly_strvecode(LY_VECODE vecode)
{
    switch (vecode) {
    case LYVE_SUCCESS:
        return "Success";
    case LYVE_SYNTAX:
        return "General syntax error";
    case LYVE_SYNTAX_YANG:
        return "YANG syntax error";
    case LYVE_SYNTAX_YIN:
        return "YIN syntax error";
    case LYVE_REFERENCE:
        return "Reference error";
    case LYVE_XPATH:
        return "XPath error";
    case LYVE_SEMANTICS:
        return "Semantic error";
    case LYVE_SYNTAX_XML:
        return "XML syntax error";
    case LYVE_SYNTAX_JSON:
        return "JSON syntax error";
    case LYVE_DATA:
        return "YANG data error";
    case LYVE_OTHER:
        return "Another error";
    }

    /* unreachable */
    return "Unknown";
}

LIBYANG_API_DEF const char *
ly_last_logmsg(void)
{
    return last_msg;
}

LIBYANG_API_DEF LY_ERR
ly_err_new(struct ly_err_item **err, LY_ERR ecode, LY_VECODE vecode, char *data_path, char *apptag,
        const char *err_format, ...)
{
    char *msg = NULL;
    struct ly_err_item *e;

    if (!err || (ecode == LY_SUCCESS)) {
        /* nothing to do */
        return ecode;
    }

    e = calloc(1, sizeof *e);
    LY_CHECK_ERR_RET(!e, LOGMEM(NULL), LY_EMEM);

    e->prev = (*err) ? (*err)->prev : e;
    if (*err) {
        (*err)->prev->next = e;
    }

    /* fill in the information */
    e->level = LY_LLERR;
    e->err = ecode;
    e->vecode = vecode;
    e->data_path = data_path;
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

    return e->err;
}

static struct ly_ctx_data_err *
ly_err_data_get(const struct ly_ctx *ctx)
{
    struct ly_ctx_data *ctx_data;
    pthread_t tid = pthread_self();
    uint32_t i;

    /* get context data */
    ctx_data = ly_ctx_data_get(ctx);

    pthread_rwlock_rdlock(&ctx_data->err_rwlock);

    /* find the thread-specific err */
    for (i = 0; i < ctx_data->err_count; ++i) {
        if (!memcmp(&ctx_data->errs[i]->tid, &tid, sizeof tid)) {
            return ctx_data->errs[i];
        }
    }

    pthread_rwlock_unlock(&ctx_data->err_rwlock);

    pthread_rwlock_wrlock(&ctx_data->err_rwlock);

    /* no need to retry the search, this thread is executing this function */

    /* not found, so create it */
    ctx_data->errs = ly_realloc(ctx_data->errs, (ctx_data->err_count + 1) * sizeof *ctx_data->errs);
    ctx_data->errs[ctx_data->err_count] = calloc(1, sizeof **ctx_data->errs);
    memcpy(&ctx_data->errs[ctx_data->err_count]->tid, &tid, sizeof tid);

    ++ctx_data->err_count;

    return ctx_data->errs[ctx_data->err_count - 1];
}

LIBYANG_API_DEF const struct ly_err_item *
ly_err_first(const struct ly_ctx *ctx)
{
    struct ly_ctx_data_err *err_data;

    if (!ctx) {
        return NULL;
    }

    /* get context err data */
    err_data = ly_err_data_get(ctx);

    return err_data ? err_data->err : NULL;
}

LIBYANG_API_DEF const struct ly_err_item *
ly_err_last(const struct ly_ctx *ctx)
{
    struct ly_ctx_data_err *err_data;

    if (!ctx) {
        return NULL;
    }

    /* get context err data */
    err_data = ly_err_data_get(ctx);

    if (!err_data) {
        return NULL;
    }

    return err_data->err ? err_data->err->prev : NULL;
}

void
ly_err_move(struct ly_ctx *src_ctx, struct ly_ctx *trg_ctx)
{
    struct ly_ctx_data_err *err_data;
    struct ly_err_item *err = NULL;

    /* get src context err data */
    err_data = ly_err_data_get(src_ctx);
    err = err_data->err;
    err_data->err = NULL;

    /* set them for trg */
    err_data = ly_err_data_get(trg_ctx);
    ly_err_free(err_data->err);
    err_data->err = err;
}

LIBYANG_API_DEF void
ly_err_free(void *ptr)
{
    struct ly_err_item *e, *next;

    /* clean the error list */
    LY_LIST_FOR_SAFE(ptr, next, e) {
        free(e->msg);
        free(e->data_path);
        free(e->schema_path);
        free(e->apptag);
        free(e);
    }
}

LIBYANG_API_DEF void
ly_err_clean(const struct ly_ctx *ctx, struct ly_err_item *eitem)
{
    struct ly_ctx_data_err *err_data;
    struct ly_err_item *e;

    err_data = ly_err_data_get(ctx);
    if (err_data->err == eitem) {
        eitem = NULL;
    }

    if (!eitem) {
        /* free all err */
        ly_err_free(err_data->err);
        err_data->err = NULL;
    } else {
        /* disconnect the error */
        for (e = err_data->err; e && (e->next != eitem); e = e->next) {}
        assert(e);
        e->next = NULL;
        err_data->err->prev = e;

        /* free this err and newer */
        ly_err_free(eitem);
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

LIBYANG_API_DEF uint32_t *
ly_temp_log_options(uint32_t *opts)
{
    uint32_t *prev_lo = temp_ly_log_opts;

    temp_ly_log_opts = opts;

    return prev_lo;
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
ly_set_log_clb(ly_log_clb clb)
{
    log_clb = clb;
}

LIBYANG_API_DEF ly_log_clb
ly_get_log_clb(void)
{
    return log_clb;
}

void
ly_log_location(const struct lysc_node *scnode, const struct lyd_node *dnode, const char *path, const struct ly_in *in)
{
    if (scnode) {
        ly_set_add(&log_location.scnodes, (void *)scnode, 1, NULL);
    }
    if (dnode || (!scnode && !path && !in)) {
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

const struct lyd_node *
ly_log_location_dnode(uint32_t idx)
{
    if (idx < log_location.dnodes.count) {
        return log_location.dnodes.dnodes[idx];
    }

    return NULL;
}

uint32_t
ly_log_location_dnode_count(void)
{
    return log_location.dnodes.count;
}

/**
 * @brief Store generated error in a context.
 *
 * @param[in] ctx Context to use.
 * @param[in] level Message log level.
 * @param[in] err Error number.
 * @param[in] vecode Error validation error code.
 * @param[in] msg Error message, always spent.
 * @param[in] data_path Error data path, always spent.
 * @param[in] schema_path Error schema path, always spent.
 * @param[in] line Error input line, if any.
 * @param[in] apptag Error app tag, always spent.
 * @return LY_ERR value.
 */
static LY_ERR
log_store(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR err, LY_VECODE vecode, char *msg, char *data_path,
        char *schema_path, uint64_t line, char *apptag)
{
    struct ly_ctx_data_err *err_data;
    struct ly_err_item *e, *last;

    assert(ctx && (level < LY_LLVRB));

    /* get context err data */
    err_data = ly_err_data_get(ctx);

    e = err_data->err;
    if (!e) {
        /* if we are only to fill in path, there must have been an error stored */
        assert(msg);
        e = calloc(1, sizeof *e);
        LY_CHECK_GOTO(!e, mem_fail);
        e->prev = e;
        e->next = NULL;

        err_data->err = e;
    } else if (!msg) {
        /* only filling the path */
        assert(data_path || schema_path);

        /* find last error */
        e = e->prev;
        do {
            if (e->level == LY_LLERR) {
                /* fill the path */
                if (data_path) {
                    free(e->data_path);
                    e->data_path = data_path;
                } else {
                    free(e->schema_path);
                    e->schema_path = schema_path;
                }
                return LY_SUCCESS;
            }
            e = e->prev;
        } while (e->prev->next);
        /* last error was not found */
        assert(0);
    } else if ((temp_ly_log_opts && ((*temp_ly_log_opts & LY_LOSTORE_LAST) == LY_LOSTORE_LAST)) ||
            (!temp_ly_log_opts && ((ATOMIC_LOAD_RELAXED(ly_log_opts) & LY_LOSTORE_LAST) == LY_LOSTORE_LAST))) {
        /* overwrite last message */
        free(e->msg);
        free(e->data_path);
        free(e->schema_path);
        free(e->apptag);
    } else {
        /* store new message */
        last = e->prev;
        e->prev = calloc(1, sizeof *e);
        LY_CHECK_GOTO(!e->prev, mem_fail);
        e = e->prev;
        e->prev = last;
        e->next = NULL;
        last->next = e;
    }

    /* fill in the information */
    e->level = level;
    e->err = err;
    e->vecode = vecode;
    e->msg = msg;
    e->data_path = data_path;
    e->schema_path = schema_path;
    e->line = line;
    e->apptag = apptag;
    return LY_SUCCESS;

mem_fail:
    LOGMEM(NULL);
    free(msg);
    free(data_path);
    free(schema_path);
    free(apptag);
    return LY_EMEM;
}

/**
 * @brief Log data path/schema path/line to stderr after the message has been printed.
 *
 * @param[in] data_path Error data path.
 * @param[in] schema_path Error schema path.
 * @param[in] line Error input line.
 */
static void
log_stderr_path_line(const char *data_path, const char *schema_path, uint64_t line)
{
    ly_bool par = 0;

    if (data_path) {
        fprintf(stderr, "%sdata path: %s", " (", data_path);
        par = 1;
    }

    if (schema_path) {
        fprintf(stderr, "%sschemadata path: %s", par ? ", " : " (", schema_path);
        par = 1;
    }

    if (line) {
        fprintf(stderr, "%sline: %" PRIu64, par ? ", " : " (", line);
        par = 1;
    }

    fprintf(stderr, par ? ")\n" : "\n");
}

/**
 * @brief Log a message.
 *
 * @param[in] ctx Context to use.
 * @param[in] level Message log level.
 * @param[in] err Error number.
 * @param[in] vecode Error validation error code.
 * @param[in] data_path Error data path, always spent.
 * @param[in] schema_path Error schema path, always spent.
 * @param[in] line Error input line, if any.
 * @param[in] apptag Error app tag.
 * @param[in] format Error message format.
 * @param[in] args Error message format arguments.
 */
static void
log_vprintf(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR err, LY_VECODE vecode, char *data_path,
        char *schema_path, uint64_t line, const char *apptag, const char *format, va_list args)
{
    char *dyn_msg = NULL;
    const char *msg;
    ly_bool free_strs = 1, lolog, lostore;

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
        goto cleanup;
    }

    if (err == LY_EMEM) {
        /* no not use more dynamic memory */
        vsnprintf(last_msg, LY_LAST_MSG_SIZE, format, args);
        msg = last_msg;
    } else {
        /* print into a single message */
        if (vasprintf(&dyn_msg, format, args) == -1) {
            LOGMEM(ctx);
            goto cleanup;
        }
        msg = dyn_msg;

        /* store as the last message */
        strncpy(last_msg, msg, LY_LAST_MSG_SIZE - 1);
    }

    /* store the error/warning in the context (if we need to store errors internally, it does not matter what are
     * the user log options), if the message is not dynamic, it would most likely fail to store (no memory) */
    if ((level < LY_LLVRB) && ctx && lostore && dyn_msg) {
        free_strs = 0;
        if (log_store(ctx, level, err, vecode, dyn_msg, data_path, schema_path, line, apptag ? strdup(apptag) : NULL)) {
            goto cleanup;
        }
    }

    /* if we are only storing errors internally, never print the message (yet) */
    if (lolog) {
        if (log_clb) {
            log_clb(level, msg, data_path, schema_path, line);
        } else {
            fprintf(stderr, "libyang[%d]: ", level);
            fprintf(stderr, "%s", msg);
            log_stderr_path_line(data_path, schema_path, line);
        }
    }

cleanup:
    if (free_strs) {
        free(data_path);
        free(schema_path);
        free(dyn_msg);
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
    log_vprintf(NULL, LY_LLDBG, 0, 0, NULL, NULL, 0, NULL, dbg_format, ap);
    va_end(ap);

    free(dbg_format);
}

#endif

void
ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR err, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(ctx, level, err, 0, NULL, NULL, 0, NULL, format, ap);
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
    } else if (lysc_data_parent(snode) != lyd_node_schema(parent)) {
        /* not a direct descendant node */
        return LY_SUCCESS;
    }

    /* get module to print, if any */
    mod = snode->module;
    prev_mod = lyd_node_module(parent);
    if (prev_mod == mod) {
        mod = NULL;
    }

    /* realloc string */
    len = *str ? strlen(*str) : 0;
    new_len = len + 1 + (mod ? strlen(mod->name) + 1 : 0) + strlen(snode->name);
    mem = realloc(*str, new_len + 1);
    LY_CHECK_ERR_RET(!mem, LOGMEM(LYD_CTX(parent)), LY_EMEM);
    *str = mem;

    /* print the last schema node */
    sprintf(*str + len, "/%s%s%s", mod ? mod->name : "", mod ? ":" : "", snode->name);
    return LY_SUCCESS;
}

LY_ERR
ly_vlog_build_data_path(const struct ly_ctx *ctx, char **path)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lyd_node *dnode = NULL;

    *path = NULL;

    if (log_location.dnodes.count) {
        dnode = log_location.dnodes.objs[log_location.dnodes.count - 1];
        if (!dnode) {
            /* special root node */
            assert(log_location.dnodes.count == 1);
            *path = strdup("/");
            LY_CHECK_ERR_GOTO(!*path, LOGMEM(ctx); rc = LY_EMEM, cleanup);
            goto cleanup;
        }

        if (dnode->parent || !lysc_data_parent(dnode->schema)) {
            /* data node with all of its parents */
            *path = lyd_path(log_location.dnodes.objs[log_location.dnodes.count - 1], LYD_PATH_STD, NULL, 0);
            LY_CHECK_ERR_GOTO(!*path, LOGMEM(ctx); rc = LY_EMEM, cleanup);
        } else {
            /* data parsers put all the parent nodes in the set, but they are not connected */
            *path = lyd_path_set(&log_location.dnodes, LYD_PATH_STD);
            LY_CHECK_ERR_GOTO(!*path, LOGMEM(ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* sometimes the last node is not created yet and we only have the schema node */
    if (log_location.scnodes.count) {
        rc = ly_vlog_build_path_append(path, log_location.scnodes.objs[log_location.scnodes.count - 1], dnode);
        LY_CHECK_GOTO(rc, cleanup);
    }

cleanup:
    if (rc) {
        free(*path);
        *path = NULL;
    }
    return rc;
}

/**
 * @brief Build log path/input line from the stored log location information.
 *
 * @param[in] ctx Context to use.
 * @param[out] data_path Generated data path.
 * @param[out] schema_path Generated data path.
 * @param[out] line Input line.
 * @return LY_ERR value.
 */
static LY_ERR
ly_vlog_build_path_line(const struct ly_ctx *ctx, char **data_path, char **schema_path, uint64_t *line)
{
    int r;
    char *path;

    *data_path = NULL;
    *schema_path = NULL;
    *line = 0;

    /* data/schema node */
    if (log_location.dnodes.count) {
        LY_CHECK_RET(ly_vlog_build_data_path(ctx, data_path));
    } else if (log_location.scnodes.count) {
        *schema_path = lysc_path(log_location.scnodes.objs[log_location.scnodes.count - 1], LYSC_PATH_LOG, NULL, 0);
        LY_CHECK_ERR_RET(!*schema_path, LOGMEM(ctx), LY_EMEM);
    }

    if (log_location.paths.count && ((const char *)(log_location.paths.objs[log_location.paths.count - 1]))[0]) {
        /* append the provided path string to data/schema path, if any */
        if (*data_path) {
            r = asprintf(&path, "%s%s", *data_path, (char *)log_location.paths.objs[log_location.paths.count - 1]);
        } else if (*schema_path) {
            r = asprintf(&path, "%s%s", *schema_path, (char *)log_location.paths.objs[log_location.paths.count - 1]);
        } else {
            r = asprintf(&path, "%s", (char *)log_location.paths.objs[log_location.paths.count - 1]);
        }
        LY_CHECK_ERR_RET(r == -1, LOGMEM(ctx), LY_EMEM);

        if (*data_path) {
            free(*data_path);
            *data_path = path;
        } else {
            free(*schema_path);
            *schema_path = path;
        }
    }

    /* line */
    if (log_location.inputs.count) {
        *line = ((struct ly_in *)log_location.inputs.objs[log_location.inputs.count - 1])->line;
    }

    return LY_SUCCESS;
}

void
ly_vlog(const struct ly_ctx *ctx, const char *apptag, LY_VECODE code, const char *format, ...)
{
    va_list ap;
    char *data_path = NULL, *schema_path = NULL;
    uint64_t line = 0;

    if (ctx) {
        ly_vlog_build_path_line(ctx, &data_path, &schema_path, &line);
    }

    va_start(ap, format);
    log_vprintf(ctx, LY_LLERR, LY_EVALID, code, data_path, schema_path, line, apptag, format, ap);
    /* path is spent and should not be freed! */
    va_end(ap);
}

/**
 * @brief Print a log message from an extension plugin callback.
 *
 * @param[in] ctx libyang context to store the error record. If not provided, the error is just printed.
 * @param[in] plugin_name Name of the plugin generating the message.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] err Error type code.
 * @param[in] data_path Error data path, always spent.
 * @param[in] schema_path Error schema path, always spent.
 * @param[in] line Error input line, if any.
 * @param[in] format Format string to print.
 * @param[in] ap Var arg list for @p format.
 */
static void
ly_ext_log(const struct ly_ctx *ctx, const char *plugin_name, LY_LOG_LEVEL level, LY_ERR err, char *data_path,
        char *schema_path, uint64_t line, const char *format, va_list ap)
{
    char *plugin_msg;

    if (ATOMIC_LOAD_RELAXED(ly_ll) < level) {
        return;
    }
    if (asprintf(&plugin_msg, "Ext plugin \"%s\": %s", plugin_name, format) == -1) {
        LOGMEM(ctx);
        return;
    }

    log_vprintf(ctx, level, (level == LY_LLERR ? LY_EPLUGIN : 0) | err, LYVE_OTHER, data_path, schema_path, line, NULL,
            plugin_msg, ap);
    free(plugin_msg);
}

LIBYANG_API_DEF void
lyplg_ext_parse_log(const struct lysp_ctx *pctx, const struct lysp_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err,
        const char *format, ...)
{
    va_list ap;
    char *data_path, *schema_path;
    uint64_t line;

    ly_vlog_build_path_line(PARSER_CTX(pctx), &data_path, &schema_path, &line);

    va_start(ap, format);
    ly_ext_log(PARSER_CTX(pctx), ext->plugin->id, level, err, data_path, schema_path, line, format, ap);
    va_end(ap);
}

LIBYANG_API_DEF void
lyplg_ext_compile_log(const struct lysc_ctx *cctx, const struct lysc_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err,
        const char *format, ...)
{
    va_list ap;
    char *schema_path = NULL;

    if (cctx && !(schema_path = strdup(cctx->path))) {
        LOGMEM(cctx->ctx);
        return;
    }

    va_start(ap, format);
    ly_ext_log(ext->module->ctx, ext->def->plugin->id, level, err, NULL, schema_path, 0, format, ap);
    va_end(ap);
}

LIBYANG_API_DEF void
lyplg_ext_compile_log_path(const char *path, const struct lysc_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err,
        const char *format, ...)
{
    va_list ap;
    char *schema_path = NULL;

    if (path && !(schema_path = strdup(path))) {
        LOGMEM(ext->module->ctx);
        return;
    }

    va_start(ap, format);
    ly_ext_log(ext->module->ctx, ext->def->plugin->id, level, err, NULL, schema_path, 0, format, ap);
    va_end(ap);
}

/**
 * @brief Serves only for creating ap.
 */
static void
_lyplg_ext_compile_log_err(const struct ly_err_item *eitem, const struct lysc_ext_instance *ext, ...)
{
    va_list ap;
    char *data_path = NULL, *schema_path = NULL;

    if (eitem->data_path) {
        data_path = strdup(eitem->data_path);
    }
    if (eitem->schema_path) {
        schema_path = strdup(eitem->schema_path);
    }

    va_start(ap, ext);
    ly_ext_log(ext->module->ctx, ext->def->plugin->id, eitem->level, eitem->err, data_path, schema_path, eitem->line, "%s", ap);
    va_end(ap);
}

LIBYANG_API_DEF void
lyplg_ext_compile_log_err(const struct ly_err_item *eitem, const struct lysc_ext_instance *ext)
{
    _lyplg_ext_compile_log_err(eitem, ext, eitem->msg);
}

/**
 * @brief Exact same functionality as ::ly_err_print() but has variable arguments so log_vprintf() can be called.
 */
static void
_ly_err_print(const struct ly_ctx *ctx, const struct ly_err_item *eitem, const char *format, ...)
{
    va_list ap;
    char *data_path = NULL, *schema_path = NULL;

    LY_CHECK_ARG_RET(ctx, eitem, );

    if (eitem->data_path) {
        data_path = strdup(eitem->data_path);
    }
    if (eitem->schema_path) {
        schema_path = strdup(eitem->schema_path);
    }

    va_start(ap, format);
    log_vprintf(ctx, eitem->level, eitem->err, eitem->vecode, data_path, schema_path, eitem->line, eitem->apptag, format, ap);
    va_end(ap);
}

LIBYANG_API_DEF void
ly_err_print(const struct ly_ctx *ctx, const struct ly_err_item *eitem)
{
    /* String ::ly_err_item.msg cannot be used directly because it may contain the % character */
    _ly_err_print(ctx, eitem, "%s", eitem->msg);
}
