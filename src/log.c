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

/* how many bytes add when enlarging buffers */
#define LY_BUF_STEP 128

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

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

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

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

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

    LY_CHECK_ARG_RET(NULL, ctx, NULL);

    i = ly_err_first(ctx);
    if (i) {
        return i->prev->apptag;
    }

    return NULL;
}

API struct ly_err_item *
ly_err_first(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_RET(NULL, ctx, NULL);

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

#ifndef NDEBUG

void
ly_log_dbg(int group, const char *format, ...)
{
    char *dbg_format;
    const char *str_group;
    va_list ap;

    if (!(ly_log_dbg_groups & group)) {
        return;
    }

    switch (group) {
    case LY_LDGDICT:
        str_group = "DICT";
        break;
    case LY_LDGYANG:
        str_group = "YANG";
        break;
    case LY_LDGYIN:
        str_group = "YIN";
        break;
    case LY_LDGXPATH:
        str_group = "XPATH";
        break;
    case LY_LDGDIFF:
        str_group = "DIFF";
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
ly_vlog_build_path_print(const struct ly_ctx *ctx, char **path, uint16_t *index, const char *str, uint16_t str_len, uint16_t *length)
{
    void *mem;
    uint16_t step;

    if ((*index) < str_len) {
        /* enlarge buffer */
        step = (str_len < LY_BUF_STEP) ? LY_BUF_STEP : str_len;
        mem = realloc(*path, *length + *index + step + 1);
        LY_CHECK_ERR_RET(!mem, LOGMEM(ctx), LY_EMEM);
        *path = mem;

        /* move data, lengths */
        memmove(&(*path)[*index + step], &(*path)[*index], *length);
        (*index) += step;
    }

    (*index) -= str_len;
    memcpy(&(*path)[*index], str, str_len);
    *length += str_len;

    return 0;
}

LY_ERR
ly_vlog_build_path(const struct ly_ctx *ctx, enum LY_VLOG_ELEM elem_type, const void *elem, char **path, int UNUSED(schema_all_prefixes))
{
    uint16_t length, index;
    size_t len;
    LY_ERR rc = LY_SUCCESS;

    length = 0;
    *path = malloc(1);
    LY_CHECK_ERR_RET(!(*path), LOGMEM(ctx), LY_EMEM);
    index = 0;

    while (elem) {
        switch (elem_type) {
        case LY_VLOG_LYS:
            /* TODO */
            break;
        case LY_VLOG_LYD:
#if 0 /* TODO when data tree present */
            name = ((struct lyd_node *)elem)->schema->name;
            if (!((struct lyd_node *)elem)->parent ||
                    lyd_node_module((struct lyd_node *)elem) != lyd_node_module(((struct lyd_node *)elem)->parent)) {
                prefix = lyd_node_module((struct lyd_node *)elem)->name;
            } else {
                prefix = NULL;
            }

            /* handle predicates (keys) in case of lists */
            if (((struct lyd_node *)elem)->schema->nodetype == LYS_LIST) {
                dlist = (struct lyd_node *)elem;
                slist = (struct lys_node_list *)((struct lyd_node *)elem)->schema;
                if (slist->keys_size) {
                    /* schema list with keys - use key values in predicates */
                    for (i = slist->keys_size - 1; i > -1; i--) {
                        LY_TREE_FOR(dlist->child, diter) {
                            if (diter->schema == (struct lys_node *)slist->keys[i]) {
                                break;
                            }
                        }
                        if (diter && ((struct lyd_node_leaf_list *)diter)->value_str) {
                            if (strchr(((struct lyd_node_leaf_list *)diter)->value_str, '\'')) {
                                val_start = "=\"";
                                val_end = "\"]";
                            } else {
                                val_start = "='";
                                val_end = "']";
                            }

                            /* print value */
                            if (ly_vlog_build_path_print(path, &index, val_end, 2, &length)) {
                                return -1;
                            }
                            len = strlen(((struct lyd_node_leaf_list *)diter)->value_str);
                            if (ly_vlog_build_path_print(path, &index,
                                    ((struct lyd_node_leaf_list *)diter)->value_str, len, &length)) {
                                return -1;
                            }

                            /* print schema name */
                            if (ly_vlog_build_path_print(path, &index, val_start, 2, &length)) {
                                return -1;
                            }
                            len = strlen(diter->schema->name);
                            if (ly_vlog_build_path_print(path, &index, diter->schema->name, len, &length)) {
                                return -1;
                            }

                            if (lyd_node_module(dlist) != lyd_node_module(diter)) {
                                if (ly_vlog_build_path_print(path, &index, ":", 1, &length)) {
                                    return -1;
                                }
                                len = strlen(lyd_node_module(diter)->name);
                                if (ly_vlog_build_path_print(path, &index, lyd_node_module(diter)->name, len, &length)) {
                                    return -1;
                                }
                            }

                            if (ly_vlog_build_path_print(path, &index, "[", 1, &length)) {
                                return -1;
                            }
                        }
                    }
                } else {
                    /* schema list without keys - use instance position */
                    i = j = lyd_list_pos(dlist);
                    len = 1;
                    while (j > 9) {
                        ++len;
                        j /= 10;
                    }

                    if (ly_vlog_build_path_print(path, &index, "]", 1, &length)) {
                        return -1;
                    }

                    str = malloc(len + 1);
                    LY_CHECK_ERR_RETURN(!str, LOGMEM(NULL), -1);
                    sprintf(str, "%d", i);

                    if (ly_vlog_build_path_print(path, &index, str, len, &length)) {
                        free(str);
                        return -1;
                    }
                    free(str);

                    if (ly_vlog_build_path_print(path, &index, "[", 1, &length)) {
                        return -1;
                    }
                }
            } else if (((struct lyd_node *)elem)->schema->nodetype == LYS_LEAFLIST &&
                    ((struct lyd_node_leaf_list *)elem)->value_str) {
                if (strchr(((struct lyd_node_leaf_list *)elem)->value_str, '\'')) {
                    val_start = "[.=\"";
                    val_end = "\"]";
                } else {
                    val_start = "[.='";
                    val_end = "']";
                }

                if (ly_vlog_build_path_print(path, &index, val_end, 2, &length)) {
                    return -1;
                }
                len = strlen(((struct lyd_node_leaf_list *)elem)->value_str);
                if (ly_vlog_build_path_print(path, &index, ((struct lyd_node_leaf_list *)elem)->value_str, len, &length)) {
                    return -1;
                }
                if (ly_vlog_build_path_print(path, &index, val_start, 4, &length)) {
                    return -1;
                }
            }

            /* check if it is yang-data top element */
            if (!((struct lyd_node *)elem)->parent) {
                ext_name = lyp_get_yang_data_template_name(elem);
                if (ext_name) {
                    if (ly_vlog_build_path_print(path, &index, name, strlen(name), &length)) {
                        return -1;
                    }
                    if (ly_vlog_build_path_print(path, &index, "/", 1, &length)) {
                        return -1;
                    }
                    yang_data_extension = 1;
                    name = ext_name;
               }
            }

            elem = ((struct lyd_node *)elem)->parent;
            break;
#endif
        case LY_VLOG_STR:
            len = strlen((const char *)elem);
            rc = ly_vlog_build_path_print(ctx, path, &index, (const char *)elem, len, &length);
            LY_CHECK_RET(rc != LY_SUCCESS, rc);
            goto success;
        case LY_VLOG_LINE:

            goto success;
        default:
            /* shouldn't be here */
            LOGINT_RET(ctx);
        }

#if 0 /* TODO when data/schema tree present */
        if (name) {
            if (ly_vlog_build_path_print(ctx, path, &index, name, strlen(name), &length)) {
                return -1;
            }
            if (prefix) {
                if (yang_data_extension && ly_vlog_build_path_print(path, &index, "#", 1, &length)) {
                    return -1;
                }
                if (ly_vlog_build_path_print(ctx, path, &index, ":", 1, &length)) {
                    return -1;
                }
                if (ly_vlog_build_path_print(ctx, path, &index, prefix, strlen(prefix), &length)) {
                    return -1;
                }
            }
        }
        if (ly_vlog_build_path_print(ctx, path, &index, "/", 1, &length)) {
            return -1;
        }
        if ((elem_type == LY_VLOG_LYS) && !elem && sparent && (sparent->nodetype == LYS_AUGMENT)) {
            len = strlen(((struct lys_node_augment *)sparent)->target_name);
            if (ly_vlog_build_path_print(ctx, path, &index, ((struct lys_node_augment *)sparent)->target_name, len, &length)) {
                return -1;
            }
        }
#endif
    }

success:
    memmove(*path, (*path) + index, length);
    (*path)[length] = '\0';
    return LY_SUCCESS;
}

void
ly_vlog(const struct ly_ctx *ctx, enum LY_VLOG_ELEM elem_type, const void *elem, LY_VECODE code, const char *format, ...)
{
    va_list ap;
    char* path = NULL;
    const struct ly_err_item *first;

    if (path_flag && (elem_type != LY_VLOG_NONE)) {
        if (elem_type == LY_VLOG_PREV) {
            /* use previous path */
            first = ly_err_first(ctx);
            if (first && first->prev->path) {
                path = strdup(first->prev->path);
            }
        } else {
            /* print path */
            if (!elem) {
                /* top-level */
                path = strdup("/");
            } else {
                ly_vlog_build_path(ctx, elem_type, elem, &path, 0);
            }
        }
    }

    va_start(ap, format);
    log_vprintf(ctx, LY_LLERR, LY_EVALID, code, path, format, ap);
    /* path is spent and should not be freed! */
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

