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
#define _POSIX_C_SOURCE 200809L /* strdup */

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
#include "tree_data.h"
#include "tree_schema.h"

volatile LY_LOG_LEVEL ly_ll = LY_LLWRN;
volatile uint32_t ly_log_opts = LY_LOLOG | LY_LOSTORE_LAST;
static ly_log_clb log_clb;
static volatile ly_bool path_flag = 1;
#ifndef NDEBUG
volatile uint32_t ly_ldbg_groups = 0;
#endif

/* how many bytes add when enlarging buffers */
#define LY_BUF_STEP 128

API LY_ERR
ly_errcode(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    i = ly_err_last(ctx);
    if (i) {
        return i->no;
    }

    return LY_SUCCESS;
}

API LY_VECODE
ly_vecode(const struct ly_ctx *ctx)
{
    struct ly_err_item *i;

    i = ly_err_last(ctx);
    if (i) {
        return i->vecode;
    }

    return LYVE_SUCCESS;
}

API const char *
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

API const char *
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

API const char *
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

API struct ly_err_item *
ly_err_new(LY_LOG_LEVEL level, LY_ERR no, LY_VECODE vecode, char *msg, char *path, char *apptag)
{
    struct ly_err_item *eitem;

    eitem = malloc(sizeof *eitem);
    LY_CHECK_ERR_RET(!eitem, LOGMEM(NULL), NULL);
    eitem->prev = eitem;
    eitem->next = NULL;

    /* fill in the information */
    eitem->level = level;
    eitem->no = no;
    eitem->vecode = vecode;
    eitem->msg = msg;
    eitem->path = path;
    eitem->apptag = apptag;

    return eitem;
}

API struct ly_err_item *
ly_err_first(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    return pthread_getspecific(ctx->errlist_key);
}

API struct ly_err_item *
ly_err_last(const struct ly_ctx *ctx)
{
    const struct ly_err_item *e;

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    e = pthread_getspecific(ctx->errlist_key);
    return e ? e->prev : NULL;
}

API void
ly_err_free(void *ptr)
{
    struct ly_err_item *i, *next;

    /* clean the error list */
    for (i = (struct ly_err_item *)ptr; i; i = next) {
        next = i->next;
        if (i->msg && strcmp(i->msg, LY_EMEM_MSG)) {
            free(i->msg);
        }
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
        for (i = first; i && (i->next != eitem); i = i->next) {}
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
ly_log_level(LY_LOG_LEVEL level)
{
    LY_LOG_LEVEL prev = ly_ll;

    ly_ll = level;
    return prev;
}

API uint32_t
ly_log_options(uint32_t opts)
{
    uint32_t prev = ly_log_opts;

    ly_log_opts = opts;
    return prev;
}

API uint32_t
ly_log_dbg_groups(uint32_t dbg_groups)
{
#ifndef NDEBUG
    uint32_t prev = ly_ldbg_groups;

    ly_ldbg_groups = dbg_groups;
    return prev;
#else
    (void)dbg_groups;
    return 0;
#endif
}

API void
ly_set_log_clb(ly_log_clb clb, ly_bool path)
{
    log_clb = clb;
    path_flag = path;
}

API ly_log_clb
ly_get_log_clb(void)
{
    return log_clb;
}

void
ly_log_location(const struct ly_ctx *ctx, const struct lysc_node *scnode, const struct lyd_node *dnode,
        const char *path, const struct ly_in *in, uint64_t line, ly_bool reset)
{
    struct ly_log_location_s *loc = pthread_getspecific(ctx->log_location_key);

    if (!loc) {
        reset = 0; /* no needed */
        loc = calloc(1, sizeof *loc);
        LY_CHECK_ERR_RET(!loc, LOGMEM(ctx), );

        pthread_setspecific(ctx->log_location_key, loc);
    }

    if (scnode) {
        ly_set_add(&loc->scnodes, (void *)scnode, 1, NULL);
    } else if (reset) {
        ly_set_erase(&loc->scnodes, NULL);
    }

    if (dnode) {
        ly_set_add(&loc->dnodes, (void *)dnode, 1, NULL);
    } else if (reset) {
        ly_set_erase(&loc->dnodes, NULL);
    }

    if (path) {
        char *s = strdup(path);
        LY_CHECK_ERR_RET(!s, LOGMEM(ctx), );
        ly_set_add(&loc->paths, s, 1, NULL);
    } else if (reset) {
        ly_set_erase(&loc->paths, free);
    }

    if (in) {
        ly_set_add(&loc->inputs, (void *)in, 1, NULL);
    } else if (reset) {
        ly_set_erase(&loc->inputs, NULL);
    }

    if (line) {
        loc->line = line;
    }
}

void
ly_log_location_revert(const struct ly_ctx *ctx, uint32_t scnode_steps, uint32_t dnode_steps,
        uint32_t path_steps, uint32_t in_steps)
{

    struct ly_log_location_s *loc = pthread_getspecific(ctx->log_location_key);

    if (!loc) {
        return;
    }

    for (uint32_t i = scnode_steps; i && loc->scnodes.count; i--) {
        loc->scnodes.count--;
    }

    for (uint32_t i = dnode_steps; i && loc->dnodes.count; i--) {
        loc->dnodes.count--;
    }

    for (uint32_t i = path_steps; i && loc->paths.count; i--) {
        ly_set_rm_index(&loc->paths, loc->paths.count - 1, free);
    }

    for (uint32_t i = in_steps; i && loc->inputs.count; i--) {
        loc->inputs.count--;
    }
}

void
ly_log_location_free(void *ptr)
{
    struct ly_log_location_s *loc = (struct ly_log_location_s *)ptr;

    if (loc) {
        ly_set_erase(&loc->scnodes, NULL);
        ly_set_erase(&loc->dnodes, NULL);
        ly_set_erase(&loc->paths, free);
        ly_set_erase(&loc->inputs, NULL);
        free(loc);
    }
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
    } else if ((ly_log_opts & LY_LOSTORE_LAST) == LY_LOSTORE_LAST) {
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
    ly_bool free_strs;

    if (level > ly_ll) {
        /* do not print or store the message */
        free(path);
        return;
    }

    /* store the error/warning (if we need to store errors internally, it does not matter what are the user log options) */
    if ((level < LY_LLVRB) && ctx && (ly_log_opts & LY_LOSTORE)) {
        assert(format);
        if (vasprintf(&msg, format, args) == -1) {
            LOGMEM(ctx);
            free(path);
            return;
        }
        if (((no & ~LY_EPLUGIN) == LY_EVALID) && (vecode == LYVE_SUCCESS)) {
            /* assume we are inheriting the error, so inherit vecode as well */
            vecode = ly_vecode(ctx);
        }
        if (log_store(ctx, level, no, vecode, msg, path, NULL)) {
            return;
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
    if (ly_log_opts & LY_LOLOG) {
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

    if (!(ly_ldbg_groups & group)) {
        return;
    }

    switch (group) {
    case LY_LDGDICT:
        str_group = "DICT";
        break;
    case LY_LDGXPATH:
        str_group = "XPATH";
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
    log_vprintf(NULL, LY_LLDBG, 0, 0, NULL, dbg_format, ap);
    va_end(ap);
}

#endif

void
ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(ctx, level, no, 0, NULL, format, ap);
    va_end(ap);
}

static LY_ERR
ly_vlog_build_path(const struct ly_ctx *ctx, struct ly_log_location_s *location, char **path)
{
    int rc;
    char *str = NULL, *prev = NULL;

    *path = NULL;

    if (location->paths.count && ((const char *)(location->paths.objs[location->paths.count - 1]))[0]) {
        /* simply get what is in the provided path string */
        *path = strdup((const char *)location->paths.objs[location->paths.count - 1]);
        LY_CHECK_ERR_RET(!(*path), LOGMEM(ctx), LY_EMEM);
    } else {
        /* generate location string */
        if (location->scnodes.count) {
            str = lysc_path(location->scnodes.objs[location->scnodes.count - 1], LYSC_PATH_LOG, NULL, 0);
            LY_CHECK_ERR_RET(!str, LOGMEM(ctx), LY_EMEM);

            rc = asprintf(path, "Schema location %s", str);
            free(str);
            LY_CHECK_ERR_RET(rc == -1, LOGMEM(ctx), LY_EMEM);
        }
        if (location->dnodes.count) {
            prev = *path;
            str = lyd_path(location->dnodes.objs[location->dnodes.count - 1], LYD_PATH_STD, NULL, 0);
            LY_CHECK_ERR_RET(!str, LOGMEM(ctx), LY_EMEM);

            rc = asprintf(path, "%s%sata location %s", prev ? prev : "", prev ? ", d" : "D", str);
            free(str);
            free(prev);
            LY_CHECK_ERR_RET(rc == -1, LOGMEM(ctx), LY_EMEM);
        }
        if (location->line) {
            prev = *path;
            rc = asprintf(path, "%s%sine number %" PRIu64, prev ? prev : "", prev ? ", l" : "L", location->line);
            free(prev);
            LY_CHECK_ERR_RET(rc == -1, LOGMEM(ctx), LY_EMEM);

            location->line = 0;
        } else if (location->inputs.count) {
            prev = *path;
            rc = asprintf(path, "%s%sine number %" PRIu64, prev ? prev : "", prev ? ", l" : "L",
                    ((struct ly_in *)location->inputs.objs[location->inputs.count - 1])->line);
            free(prev);
            LY_CHECK_ERR_RET(rc == -1, LOGMEM(ctx), LY_EMEM);
        }

        if (*path) {
            prev = *path;
            rc = asprintf(path, "%s.", prev);
            free(prev);
            LY_CHECK_ERR_RET(rc == -1, LOGMEM(ctx), LY_EMEM);
        }
    }

    return LY_SUCCESS;
}

void
ly_vlog(const struct ly_ctx *ctx, LY_VECODE code, const char *format, ...)
{
    va_list ap;
    char *path = NULL;

    if (path_flag && ctx) {
        /* get the location information */
        struct ly_log_location_s *location = pthread_getspecific(ctx->log_location_key);
        if (location) {
            ly_vlog_build_path(ctx, location, &path);
        }
    }

    va_start(ap, format);
    log_vprintf(ctx, LY_LLERR, LY_EVALID, code, path, format, ap);
    /* path is spent and should not be freed! */
    va_end(ap);
}

API void
lyext_log(const struct lysc_ext_instance *ext, LY_LOG_LEVEL level, LY_ERR err_no, const char *path, const char *format, ...)
{
    va_list ap;
    char *plugin_msg;
    int ret;

    if (ly_ll < level) {
        return;
    }
    ret = asprintf(&plugin_msg, "Extension plugin \"%s\": %s)", ext->def->plugin->id, format);
    if (ret == -1) {
        LOGMEM(ext->module->ctx);
        return;
    }

    va_start(ap, format);
    log_vprintf(ext->module->ctx, level, (level == LY_LLERR ? LY_EPLUGIN : 0) | err_no, LYVE_OTHER, path ? strdup(path) : NULL, plugin_msg, ap);
    va_end(ap);

    free(plugin_msg);
}

/**
 * @brief Exact same functionality as ::ly_err_print() but has variable arguments so va_start() can
 * be used and an empty va_list created.
 */
static void
_ly_err_print(const struct ly_ctx *ctx, struct ly_err_item *eitem, ...)
{
    va_list ap;
    char *path_dup = NULL;

    LY_CHECK_ARG_RET(ctx, eitem, );

    if (eitem->path) {
        /* duplicate path because it will be freed */
        path_dup = strdup(eitem->path);
        LY_CHECK_ERR_RET(!path_dup, LOGMEM(ctx), );
    }

    va_start(ap, eitem);
    log_vprintf(ctx, eitem->level, eitem->no, eitem->vecode, eitem->path, eitem->msg, ap);
    va_end(ap);

    if (path_dup) {
        eitem->path = path_dup;
    }
}

API void
ly_err_print(const struct ly_ctx *ctx, struct ly_err_item *eitem)
{
    _ly_err_print(ctx, eitem);
}
