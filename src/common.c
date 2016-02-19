/**
 * @file common.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common libyang routines implementations
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "tree_internal.h"
#include "libyang.h"

/* libyang errno */
LY_ERR ly_errno_int = LY_EINT;
static pthread_once_t ly_err_once = PTHREAD_ONCE_INIT;
static pthread_key_t ly_err_key;
#ifdef __linux__
struct ly_err ly_err_main = {LY_SUCCESS, 0, {0}, {0}};
#endif

static void
ly_err_free(void *ptr)
{
#ifdef __linux__
    /* in __linux__ we use static memory in the main thread,
     * so this check is for programs terminating the main()
     * function by pthread_exit() :)
     */
    if (ptr != &ly_err_main) {
#else
    {
#endif
        free(ptr);
    }
}

static void
ly_err_createkey(void)
{
    int r;

    /* initiate */
    while ((r = pthread_key_create(&ly_err_key, ly_err_free)) == EAGAIN);
    pthread_setspecific(ly_err_key, NULL);
}

struct ly_err *
ly_err_location(void)
{
    struct ly_err *e;

    pthread_once(&ly_err_once, ly_err_createkey);
    e = pthread_getspecific(ly_err_key);
    if (!e) {
        /* prepare ly_err storage */
#ifdef __linux__
        if (getpid() == syscall(SYS_gettid)) {
            /* main thread - use global variable instead of thread-specific variable. */
            e = &ly_err_main;
        } else {
#else
        {
#endif /* __linux__ */
            e = calloc(1, sizeof *e);
        }
        pthread_setspecific(ly_err_key, e);
    }

    return e;
}

API LY_ERR *
ly_errno_location(void)
{
    struct ly_err *e;

    e = ly_err_location();
    if (!e) {
        return &ly_errno_int;
    }
    return &(e->no);
}

API const char *
ly_errmsg(void)
{
    struct ly_err *e;

    e = ly_err_location();
    if (!e) {
        return NULL;
    }
    return e->msg;
}

API const char *
ly_errpath(void)
{
    struct ly_err *e;

    e = ly_err_location();
    if (!e) {
        return NULL;
    }
    return &e->path[e->path_index];
}

#ifndef  __USE_GNU

char *get_current_dir_name(void)
{
    char tmp[PATH_MAX];

    if (getcwd(tmp, sizeof(tmp)))
        return strdup(tmp);
    return NULL;
}

#endif

const char *
strpbrk_backwards(const char *s, const char *accept, unsigned int s_len)
{
    const char *sc;

    for (; *s != '\0' && s_len; --s, --s_len) {
        for (sc = accept; *sc != '\0'; ++sc) {
            if (*s == *sc) {
                return s;
            }
        }
    }
    return s;
}

char *
strnchr(const char *s, int c, unsigned int len)
{
    for (; *s != (char)c; ++s, --len) {
        if ((*s == '\0') || (!len)) {
            return NULL;
        }
    }
    return (char *)s;
}

const char *
strnodetype(LYS_NODE type)
{
    switch (type) {
    case LYS_UNKNOWN:
        return NULL;
    case LYS_AUGMENT:
        return "augment";
    case LYS_CONTAINER:
        return "container";
    case LYS_CHOICE:
        return "choice";
    case LYS_LEAF:
        return "leaf";
    case LYS_LEAFLIST:
        return "leaf-list";
    case LYS_LIST:
        return "list";
    case LYS_ANYXML:
        return "anyxml";
    case LYS_GROUPING:
        return "grouping";
    case LYS_CASE:
        return "case";
    case LYS_INPUT:
        return "input";
    case LYS_OUTPUT:
        return "output";
    case LYS_NOTIF:
        return "notification";
    case LYS_RPC:
        return "rpc";
    case LYS_USES:
        return "uses";
    }

    return NULL;
}

const char *
transform_module_name2import_prefix(const struct lys_module *module, const char *module_name)
{
    uint16_t i;

    if (!strcmp(lys_module(module)->name, module_name)) {
        /* the same for module and submodule */
        return module->prefix;
    }

    for (i = 0; i < module->imp_size; ++i) {
        if (!strcmp(module->imp[i].module->name, module_name)) {
            return module->imp[i].prefix;
        }
    }

    return NULL;
}

static const char *
_transform_json2xml(const struct lys_module *module, const char *expr, int schema, const char ***prefixes,
                    const char ***namespaces, uint32_t *ns_count)
{
    const char *in, *id, *prefix;
    char *out, *col, *name;
    size_t out_size, out_used, id_len;
    const struct lys_module *mod;
    uint32_t i;

    assert(module && expr && ((!prefixes && !namespaces && !ns_count) || (prefixes && namespaces && ns_count)));

    if (ns_count) {
        *ns_count = 0;
        *prefixes = NULL;
        *namespaces = NULL;
    }

    in = expr;
    out_size = strlen(in) + 1;
    out = malloc(out_size);
    if (!out) {
        LOGMEM;
        return NULL;
    }
    out_used = 0;

    while (1) {
        col = strchr(in, ':');
        /* we're finished, copy the remaining part */
        if (!col) {
            strcpy(&out[out_used], in);
            out_used += strlen(in) + 1;
            assert(out_size == out_used);
            return lydict_insert_zc(module->ctx, out);
        }
        id = strpbrk_backwards(col - 1, "/ [\'\"", (col - in) - 1);
        if ((id[0] == '/') || (id[0] == ' ') || (id[0] == '[') || (id[0] == '\'') || (id[0] == '\"')) {
            ++id;
        }
        id_len = col - id;

        /* get the module */
        if (!schema) {
            name = strndup(id, id_len);
            mod = ly_ctx_get_module(module->ctx, name, NULL);
            free(name);
            if (!mod) {
                LOGVAL(LYE_INMOD_LEN, 0, 0, NULL, id_len, id);
                goto fail;
            }
            prefix = mod->prefix;
        } else {
            name = strndup(id, id_len);
            prefix = transform_module_name2import_prefix(module, name);
            free(name);
            if (!prefix) {
                LOGVAL(LYE_INMOD_LEN, 0, 0, NULL, id_len, id);
                goto fail;
            }
        }

        /* remember the namespace definition (only if it's new) */
        if (!schema && ns_count) {
            for (i = 0; i < *ns_count; ++i) {
                if ((*namespaces)[i] == mod->ns) {
                    break;
                }
            }
            if (i == *ns_count) {
                ++(*ns_count);
                *prefixes = ly_realloc(*prefixes, *ns_count * sizeof **prefixes);
                if (!(*prefixes)) {
                    LOGMEM;
                    goto fail;
                }
                *namespaces = ly_realloc(*namespaces, *ns_count * sizeof **namespaces);
                if (!(*namespaces)) {
                    LOGMEM;
                    goto fail;
                }
                (*prefixes)[*ns_count - 1] = mod->prefix;
                (*namespaces)[*ns_count - 1] = mod->ns;
            }
        }

        /* adjust out size */
        out_size += strlen(prefix) - id_len;
        out = ly_realloc(out, out_size);
        if (!out) {
            LOGMEM;
            goto fail;
        }

        /* copy the data before prefix */
        strncpy(&out[out_used], in, id-in);
        out_used += id - in;

        /* copy the model prefix */
        strcpy(&out[out_used], prefix);
        out_used += strlen(prefix);

        /* copy ':' */
        out[out_used] = ':';
        ++out_used;

        /* finally adjust in pointer for next round */
        in = col + 1;
    }

    /* unreachable */
    LOGINT;

fail:
    if (!schema && ns_count) {
        free(*prefixes);
        free(*namespaces);
    }
    free(out);
    return NULL;
}

const char *
transform_json2xml(const struct lys_module *module, const char *expr, const char ***prefixes, const char ***namespaces,
                   uint32_t *ns_count)
{
    return _transform_json2xml(module, expr, 0, prefixes, namespaces, ns_count);
}

const char *
transform_json2schema(const struct lys_module *module, const char *expr)
{
    return _transform_json2xml(module, expr, 1, NULL, NULL, NULL);
}

const char *
transform_xml2json(struct ly_ctx *ctx, const char *expr, struct lyxml_elem *xml, int log)
{
    const char *in, *id;
    char *out, *col, *prefix;
    size_t out_size, out_used, id_len, rc;
    const struct lys_module *mod;
    const struct lyxml_ns *ns;

    in = expr;
    out_size = strlen(in)+1;
    out = malloc(out_size);
    if (!out) {
        if (log) {
            LOGMEM;
        }
        return NULL;
    }
    out_used = 0;

    while (1) {
        col = strchr(in, ':');
        /* we're finished, copy the remaining part */
        if (!col) {
            strcpy(&out[out_used], in);
            out_used += strlen(in)+1;
            assert(out_size == out_used);
            return lydict_insert_zc(ctx, out);
        }
        id = strpbrk_backwards(col-1, "/ [\'\"", (col-in)-1);
        if ((id[0] == '/') || (id[0] == ' ') || (id[0] == '[') || (id[0] == '\'') || (id[0] == '\"')) {
            ++id;
        }
        id_len = col-id;
        rc = parse_identifier(id);
        if (rc < id_len) {
            if (log) {
                LOGVAL(LYE_INCHAR, LOGLINE(xml), LY_VLOG_XML, xml, id[rc], &id[rc]);
            }
            free(out);
            return NULL;
        }

        /* get the module */
        prefix = strndup(id, id_len);
        if (!prefix) {
            if (log) {
                LOGMEM;
            }
            free(out);
            return NULL;
        }
        ns = lyxml_get_ns(xml, prefix);
        free(prefix);
        if (!ns) {
            if (log) {
                LOGVAL(LYE_SPEC, LOGLINE(xml), LY_VLOG_XML, xml,
                       "XML namespace with prefix \"%.*s\" not defined.", id_len, id);
            }
            free(out);
            return NULL;
        }
        mod = ly_ctx_get_module_by_ns(ctx, ns->value, NULL);
        if (!mod) {
            if (log) {
                LOGVAL(LYE_SPEC, LOGLINE(xml), LY_VLOG_XML, xml,
                       "Module with the namespace \"%s\" could not be found.", ns->value);
            }
            free(out);
            return NULL;
        }

        /* adjust out size (it can even decrease in some strange cases) */
        out_size += strlen(mod->name)-id_len;
        out = ly_realloc(out, out_size);
        if (!out) {
            if (log) {
                LOGMEM;
            }
            return NULL;
        }

        /* copy the data before prefix */
        strncpy(&out[out_used], in, id-in);
        out_used += id-in;

        /* copy the model name */
        strcpy(&out[out_used], mod->name);
        out_used += strlen(mod->name);

        /* copy ':' */
        out[out_used] = ':';
        ++out_used;

        /* finally adjust in pointer for next round */
        in = col+1;
    }

    /* unreachable */
    LOGINT;
    return NULL;
}

const char *
transform_schema2json(const struct lys_module *module, const char *expr, uint32_t line)
{
    const char *in, *id;
    char *out, *col;
    size_t out_size, out_used, id_len, rc;
    const struct lys_module *mod;

    in = expr;
    out_size = strlen(in)+1;
    out = malloc(out_size);
    if (!out) {
        LOGMEM;
        return NULL;
    }
    out_used = 0;

    while (1) {
        col = strchr(in, ':');
        /* we're finished, copy the remaining part */
        if (!col) {
            strcpy(&out[out_used], in);
            out_used += strlen(in)+1;
            assert(out_size == out_used);
            return lydict_insert_zc(module->ctx, out);
        }
        id = strpbrk_backwards(col-1, "/ [\'\"", (col-in)-1);
        if ((id[0] == '/') || (id[0] == ' ') || (id[0] == '[') || (id[0] == '\'') || (id[0] == '\"')) {
            ++id;
        }
        id_len = col-id;
        rc = parse_identifier(id);
        if (rc < id_len) {
            LOGVAL(LYE_INCHAR, line, 0, NULL, id[rc], &id[rc]);
            free(out);
            return NULL;
        }

        /* get the module */
        mod = lys_get_import_module(module, id, id_len, NULL, 0);
        if (!mod) {
            LOGVAL(LYE_INMOD_LEN, line, 0, NULL, id_len, id);
            free(out);
            return NULL;
        }

        /* adjust out size (it can even decrease in some strange cases) */
        out_size += strlen(mod->name)-id_len;
        out = ly_realloc(out, out_size);
        if (!out) {
            LOGMEM;
            return NULL;
        }

        /* copy the data before prefix */
        strncpy(&out[out_used], in, id-in);
        out_used += id-in;

        /* copy the model name */
        strcpy(&out[out_used], mod->name);
        out_used += strlen(mod->name);

        /* copy ':' */
        out[out_used] = ':';
        ++out_used;

        /* finally adjust in pointer for next round */
        in = col+1;
    }

    /* unreachable */
    LOGINT;
    return NULL;
}

void *
ly_realloc(void *ptr, size_t size)
{
    void *new_mem;

    new_mem = realloc(ptr, size);
    if (!new_mem) {
        free(ptr);
    }

    return new_mem;
}

int
ly_strequal_(const char *s1, const char *s2)
{
    if (s1 == s2) {
        return 1;
    } else if (!s1 || !s2) {
        return 0;
    } else {
        for ( ; *s1 == *s2; s1++, s2++) {
            if (*s1 == '\0') {
                return 1;
            }
        }
        return 0;
    }
}
