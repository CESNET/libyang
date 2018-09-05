/**
 * @file log.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Logger routines implementations
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "libyang.h"
#include "common.h"
#include "context.h"

THREAD_LOCAL enum int_log_opts log_opt;
volatile uint8_t ly_log_level = LY_LLWRN;
volatile uint8_t ly_log_opts = LY_LOLOG | LY_LOSTORE_LAST;
static void (*ly_log_clb)(LY_LOG_LEVEL level, const char *msg, const char *path);
static volatile int path_flag = 1;
#ifndef NDEBUG
volatile int ly_log_dbg_groups = 0;
#endif

API LY_VECODE
ly_vecode(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    i = ly_err_first(ctx);
    if (i) {
        return i->prev->vecode;
    }

    return LYVE_SUCCESS;
}

API const char *
ly_errmsg(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    LY_CHECK_ARG_NON_NULL_RETURN(NULL, ctx, NULL);

    i = ly_err_first(ctx);
    if (i) {
        return i->prev->msg;
    }

    return NULL;
}

API const char *
ly_errpath(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    LY_CHECK_ARG_NON_NULL_RETURN(NULL, ctx, NULL);

    i = ly_err_first(ctx);
    if (i) {
        return i->prev->path;
    }

    return NULL;
}

API const char *
ly_errapptag(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    LY_CHECK_ARG_NON_NULL_RETURN(NULL, ctx, NULL);

    i = ly_err_first(ctx);
    if (i) {
        return i->prev->apptag;
    }

    return NULL;
}

API struct ly_err_item *
ly_err_first(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_NON_NULL_RETURN(NULL, ctx, NULL);

    return pthread_getspecific(ctx->errlist_key);
}

void
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

API void
ly_err_clean(struct ly_ctx *ctx, struct ly_err_item *eitem)
{
    struct ly_err_item *i, *first;

    first = ly_err_first(ctx);
    if (first == eitem) {
        eitem = NULL;
    }
    if (eitem) {
        /* disconnect the error */
        for (i = first; i && (i->next != eitem); i = i->next);
        assert(i);
        i->next = NULL;
        first->prev = i;
        /* free this err and newer */
        ly_err_free(eitem);
    } else {
        /* free all err */
        ly_err_free(first);
        pthread_setspecific(ctx->errlist_key, NULL);
    }
}

API LY_LOG_LEVEL
ly_verb(LY_LOG_LEVEL level)
{
    LY_LOG_LEVEL prev = ly_log_level;

    ly_log_level = level;
    return prev;
}

API int
ly_log_options(int opts)
{
    uint8_t prev = ly_log_opts;

    ly_log_opts = opts;
    return prev;
}

API void
ly_verb_dbg(int dbg_groups)
{
#ifndef NDEBUG
    ly_log_dbg_groups = dbg_groups;
#else
    (void)dbg_groups;
#endif
}

API void
ly_set_log_clb(void (*clb)(LY_LOG_LEVEL level, const char *msg, const char *path), int path)
{
    ly_log_clb = clb;
    path_flag = path;
}

API void
(*ly_get_log_clb(void))(LY_LOG_LEVEL, const char *, const char *)
{
    return ly_log_clb;
}

static LY_ERR
log_store(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, LY_VECODE vecode, char *msg, char *path, char *apptag)
{
    struct ly_err_item *eitem, *last;

    assert(ctx && (level < LY_LLVRB));

    eitem = pthread_getspecific(ctx->errlist_key);
    if (!eitem) {
        /* if we are only to fill in path, there must have been an error stored */
        assert(msg);
        eitem = malloc(sizeof *eitem);
        LY_CHECK_GOTO(!eitem, mem_fail);
        eitem->prev = eitem;
        eitem->next = NULL;

        pthread_setspecific(ctx->errlist_key, eitem);
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
    } else if ((log_opt != ILO_STORE) && ((ly_log_opts & LY_LOSTORE_LAST) == LY_LOSTORE_LAST)) {
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
log_vprintf(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, LY_VECODE vecode, char *path,
            const char *format, va_list args)
{
    char *msg = NULL;
    int free_strs;

    if ((log_opt == ILO_ERR2WRN) && (level == LY_LLERR)) {
        /* change error to warning */
        level = LY_LLWRN;
    }

    if ((log_opt == ILO_IGNORE) || (level > ly_log_level)) {
        /* do not print or store the message */
        free(path);
        return;
    }

    if ((no == LY_EVALID) && (vecode == LYVE_SUCCESS)) {
        /* assume we are inheriting the error, so inherit vecode as well */
        vecode = ly_vecode(ctx);
    }

    /* store the error/warning (if we need to store errors internally, it does not matter what are the user log options) */
    if ((level < LY_LLVRB) && ctx && ((ly_log_opts & LY_LOSTORE) || (log_opt == ILO_STORE))) {
        if (!format) {
            assert(path);
            /* postponed print of path related to the previous error, do not rewrite stored original message */
            if (log_store(ctx, level, no, vecode, NULL, path, NULL)) {
                return;
            }
            msg = "Path is related to the previous error message.";
        } else {
            if (vasprintf(&msg, format, args) == -1) {
                LOGMEM(ctx);
                free(path);
                return;
            }
            if (log_store(ctx, level, no, vecode, msg, path, NULL)) {
                return;
            }
        }
        free_strs = 0;
    } else {
        if (vasprintf(&msg, format, args) == -1) {
            LOGMEM(ctx);
            free(path);
            return;
        }
        free_strs = 1;
    }

    /* if we are only storing errors internally, never print the message (yet) */
    if ((ly_log_opts & LY_LOLOG) && (log_opt != ILO_STORE)) {
        if (ly_log_clb) {
            ly_log_clb(level, msg, path);
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

void
ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(ctx, level, no, 0, NULL, format, ap);
    va_end(ap);
}

API void
ly_err_print(struct ly_err_item *eitem)
{
    if (ly_log_opts & LY_LOLOG) {
        if (ly_log_clb) {
            ly_log_clb(eitem->level, eitem->msg, eitem->path);
        } else {
            fprintf(stderr, "libyang[%d]: %s%s", eitem->level, eitem->msg, eitem->path ? " " : "\n");
            if (eitem->path) {
                fprintf(stderr, "(path: %s)\n", eitem->path);
            }
        }
    }
}

