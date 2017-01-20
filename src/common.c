/**
 * @file common.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common libyang routines implementations
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
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
#include "xpath.h"
#include "context.h"
#include "libyang.h"

/* libyang errno */
LY_ERR ly_errno_int = LY_EINT;
LY_VECODE ly_vecode_unkn = LYVE_SUCCESS;
uint8_t ly_vlog_hide_def;
static pthread_once_t ly_err_once = PTHREAD_ONCE_INIT;
static pthread_key_t ly_err_key;
#ifdef __linux__
struct ly_err ly_err_main = {LY_SUCCESS, LYVE_SUCCESS, 0, 0, 0, NULL, {0}, {0}, {0}, {0}};
#endif

static void
ly_err_free(void *ptr)
{
    struct ly_err *e = (struct ly_err *)ptr;
    struct ly_err_item *i, *next;

    /* clean the error list */
    for (i = e->errlist; i; i = next) {
        next = i->next;
        free(i->msg);
        free(i->path);
        free(i);
    }
    e->errlist = NULL;

#ifdef __linux__
    /* in __linux__ we use static memory in the main thread,
     * so this check is for programs terminating the main()
     * function by pthread_exit() :)
     */
    if (e != &ly_err_main) {
#else
    {
#endif
        free(e);
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

void
ly_err_clean(int with_errno)
{
    struct ly_err_item *i, *next;

    i = ly_err_location()->errlist;
    ly_err_location()->errlist = NULL;
    for (; i; i = next) {
        next = i->next;
        free(i->msg);
        free(i->path);
        free(i);
    }

    if (with_errno) {
        ly_err_location()->no = LY_SUCCESS;
        ly_err_location()->code = LYVE_SUCCESS;
    }
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

API LY_VECODE *
ly_vecode_location(void)
{
    struct ly_err *e;

    e = ly_err_location();
    if (!e) {
        return &ly_vecode_unkn;
    }
    return &(e->code);
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

API const char *
ly_errapptag(void)
{
    struct ly_err *e;

    e = ly_err_location();
    if (!e) {
        return NULL;
    }
    return e->apptag;
}

uint8_t *
ly_vlog_hide_location(void)
{
    struct ly_err *e;

    e = ly_err_location();
    if (!e) {
        return &ly_vlog_hide_def;
    }
    return &(e->vlog_hide);
}

uint8_t *
ly_buf_used_location(void)
{
    struct ly_err *e;

    e = ly_err_location();
    return &(e->buf_used);
}

char *
ly_buf(void)
{
    struct ly_err *e;

    e = ly_err_location();
    return e->buf;
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
    case LYS_ACTION:
        return "action";
    case LYS_ANYDATA:
        return "anydata";
    }

    return NULL;
}

const char *
transform_module_name2import_prefix(const struct lys_module *module, const char *module_name)
{
    uint16_t i;

    if (!module_name) {
        return NULL;
    }

    if (!strcmp(lys_main_module(module)->name, module_name)) {
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
    const char *cur_expr, *end, *prefix, *ptr;
    char *out, *name;
    size_t out_size, out_used, name_len;
    const struct lys_module *mod;
    uint32_t i, j;
    struct lyxp_expr *exp;

    assert(module && expr && ((!prefixes && !namespaces && !ns_count) || (prefixes && namespaces && ns_count)));

    if (ns_count) {
        *ns_count = 0;
        *prefixes = NULL;
        *namespaces = NULL;
    }

    out_size = strlen(expr) + 1;
    out = malloc(out_size);
    if (!out) {
        LOGMEM;
        return NULL;
    }
    out_used = 0;

    exp = lyxp_parse_expr(expr);
    if (!exp) {
        free(out);
        return NULL;
    }

    for (i = 0; i < exp->used; ++i) {
        cur_expr = &exp->expr[exp->expr_pos[i]];

        /* copy WS */
        if (i && ((end = exp->expr + exp->expr_pos[i - 1] + exp->tok_len[i - 1]) != cur_expr)) {
            strncpy(&out[out_used], end, cur_expr - end);
            out_used += cur_expr - end;
        }

        if ((exp->tokens[i] == LYXP_TOKEN_NAMETEST) && (end = strnchr(cur_expr, ':', exp->tok_len[i]))) {
            /* get the module */
            name_len = end - cur_expr;
            if (!schema) {
                name = strndup(cur_expr, name_len);
                mod = ly_ctx_get_module(module->ctx, name, NULL);
                free(name);
                if (!mod) {
                    LOGVAL(LYE_INMOD_LEN, LY_VLOG_NONE, NULL, name_len, cur_expr);
                    goto error;
                }
                prefix = mod->prefix;
            } else {
                name = strndup(cur_expr, name_len);
                prefix = transform_module_name2import_prefix(module, name);
                free(name);
                if (!prefix) {
                    LOGVAL(LYE_INMOD_LEN, LY_VLOG_NONE, NULL, name_len, cur_expr);
                    goto error;
                }
            }

            /* remember the namespace definition (only if it's new) */
            if (!schema && ns_count) {
                for (j = 0; j < *ns_count; ++j) {
                    if (ly_strequal((*namespaces)[j], mod->ns, 1)) {
                        break;
                    }
                }
                if (j == *ns_count) {
                    ++(*ns_count);
                    *prefixes = ly_realloc(*prefixes, *ns_count * sizeof **prefixes);
                    if (!(*prefixes)) {
                        LOGMEM;
                        goto error;
                    }
                    *namespaces = ly_realloc(*namespaces, *ns_count * sizeof **namespaces);
                    if (!(*namespaces)) {
                        LOGMEM;
                        goto error;
                    }
                    (*prefixes)[*ns_count - 1] = mod->prefix;
                    (*namespaces)[*ns_count - 1] = mod->ns;
                }
            }

            /* adjust out size (it can even decrease in some strange cases) */
            out_size += strlen(prefix) - name_len;
            out = ly_realloc(out, out_size);
            if (!out) {
                LOGMEM;
                goto error;
            }

            /* copy the model name */
            strcpy(&out[out_used], prefix);
            out_used += strlen(prefix);

            /* copy the rest */
            strncpy(&out[out_used], end, exp->tok_len[i] - name_len);
            out_used += exp->tok_len[i] - name_len;
        } else if ((exp->tokens[i] == LYXP_TOKEN_LITERAL) && (end = strnchr(cur_expr, ':', exp->tok_len[i]))) {
            ptr = end;
            while (isalnum(ptr[-1]) || (ptr[-1] == '_') || (ptr[-1] == '-') || (ptr[-1] == '.')) {
                --ptr;
            }

            /* get the module */
            name_len = end - ptr;
            if (!schema) {
                prefix = NULL;
                name = strndup(ptr, name_len);
                mod = ly_ctx_get_module(module->ctx, name, NULL);
                free(name);
                if (mod) {
                    prefix = mod->prefix;
                }
            } else {
                name = strndup(ptr, name_len);
                prefix = transform_module_name2import_prefix(module, name);
                free(name);
            }

            if (prefix) {
                /* remember the namespace definition (only if it's new) */
                if (!schema && ns_count) {
                    for (j = 0; j < *ns_count; ++j) {
                        if (ly_strequal((*namespaces)[j], mod->ns, 1)) {
                            break;
                        }
                    }
                    if (j == *ns_count) {
                        ++(*ns_count);
                        *prefixes = ly_realloc(*prefixes, *ns_count * sizeof **prefixes);
                        if (!(*prefixes)) {
                            LOGMEM;
                            goto error;
                        }
                        *namespaces = ly_realloc(*namespaces, *ns_count * sizeof **namespaces);
                        if (!(*namespaces)) {
                            LOGMEM;
                            goto error;
                        }
                        (*prefixes)[*ns_count - 1] = mod->prefix;
                        (*namespaces)[*ns_count - 1] = mod->ns;
                    }
                }

                /* adjust out size (it can even decrease in some strange cases) */
                out_size += strlen(prefix) - name_len;
                out = ly_realloc(out, out_size);
                if (!out) {
                    LOGMEM;
                    goto error;
                }

                /* copy any beginning */
                strncpy(&out[out_used], cur_expr, ptr - cur_expr);
                out_used += ptr - cur_expr;

                /* copy the model name */
                strcpy(&out[out_used], prefix);
                out_used += strlen(prefix);

                /* copy the rest */
                strncpy(&out[out_used], end, (exp->tok_len[i] - name_len) - (ptr - cur_expr));
                out_used += (exp->tok_len[i] - name_len) - (ptr - cur_expr);
            } else {
                strncpy(&out[out_used], &exp->expr[exp->expr_pos[i]], exp->tok_len[i]);
                out_used += exp->tok_len[i];
            }
        } else {
            strncpy(&out[out_used], &exp->expr[exp->expr_pos[i]], exp->tok_len[i]);
            out_used += exp->tok_len[i];
        }
    }
    out[out_used] = '\0';

    lyxp_exp_free(exp);
    return lydict_insert_zc(module->ctx, out);

error:
    if (!schema && ns_count) {
        free(*prefixes);
        free(*namespaces);
    }
    free(out);
    lyxp_exp_free(exp);
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
transform_xml2json(struct ly_ctx *ctx, const char *expr, struct lyxml_elem *xml, int use_ctx_data_clb, int log)
{
    const char *end, *cur_expr, *ptr;
    char *out, *prefix;
    uint16_t i;
    size_t out_size, out_used, pref_len;
    const struct lys_module *mod;
    const struct lyxml_ns *ns;
    struct lyxp_expr *exp;

    out_size = strlen(expr) + 1;
    out = malloc(out_size);
    if (!out) {
        if (log) {
            LOGMEM;
        }
        return NULL;
    }
    out_used = 0;

    exp = lyxp_parse_expr(expr);
    if (!exp) {
        free(out);
        return NULL;
    }

    for (i = 0; i < exp->used; ++i) {
        cur_expr = &exp->expr[exp->expr_pos[i]];

        /* copy WS */
        if (i && ((end = exp->expr + exp->expr_pos[i - 1] + exp->tok_len[i - 1]) != cur_expr)) {
            strncpy(&out[out_used], end, cur_expr - end);
            out_used += cur_expr - end;
        }

        if ((exp->tokens[i] == LYXP_TOKEN_NAMETEST) && (end = strnchr(cur_expr, ':', exp->tok_len[i]))) {
            /* get the module */
            pref_len = end - cur_expr;
            prefix = strndup(cur_expr, pref_len);
            if (!prefix) {
                if (log) {
                    LOGMEM;
                }
                goto error;
            }
            ns = lyxml_get_ns(xml, prefix);
            free(prefix);
            if (!ns) {
                if (log) {
                    LOGVAL(LYE_XML_INVAL, LY_VLOG_XML, xml, "namespace prefix");
                    LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL,
                        "XML namespace with prefix \"%.*s\" not defined.", pref_len, cur_expr);
                }
                goto error;
            }
            mod = ly_ctx_get_module_by_ns(ctx, ns->value, NULL);
            if (use_ctx_data_clb && ctx->data_clb) {
                if (!mod) {
                    mod = ctx->data_clb(ctx, NULL, ns->value, 0, ctx->data_clb_data);
                } else if (!mod->implemented) {
                    mod = ctx->data_clb(ctx, mod->name, mod->ns, LY_MODCLB_NOT_IMPLEMENTED, ctx->data_clb_data);
                }
            }
            if (!mod) {
                if (log) {
                    LOGVAL(LYE_XML_INVAL, LY_VLOG_XML, xml, "module namespace");
                    LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL,
                        "Module with the namespace \"%s\" could not be found.", ns->value);
                }
                goto error;
            }

            /* adjust out size (it can even decrease in some strange cases) */
            out_size += strlen(mod->name) - pref_len;
            out = ly_realloc(out, out_size);
            if (!out) {
                if (log) {
                    LOGMEM;
                }
                goto error;
            }

            /* copy the model name */
            strcpy(&out[out_used], mod->name);
            out_used += strlen(mod->name);

            /* copy the rest */
            strncpy(&out[out_used], end, exp->tok_len[i] - pref_len);
            out_used += exp->tok_len[i] - pref_len;
        } else if ((exp->tokens[i] == LYXP_TOKEN_LITERAL) && (end = strnchr(cur_expr, ':', exp->tok_len[i]))) {
            ptr = end;
            while (isalnum(ptr[-1]) || (ptr[-1] == '_') || (ptr[-1] == '-') || (ptr[-1] == '.')) {
                --ptr;
            }

            /* get the module */
            pref_len = end - cur_expr;
            prefix = strndup(cur_expr, pref_len);
            if (!prefix) {
                if (log) {
                    LOGMEM;
                }
                goto error;
            }
            ns = lyxml_get_ns(xml, prefix);
            free(prefix);
            if (!ns) {
                if (log) {
                    LOGVAL(LYE_XML_INVAL, LY_VLOG_XML, xml, "namespace prefix");
                    LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL,
                        "XML namespace with prefix \"%.*s\" not defined.", pref_len, cur_expr);
                }
                goto error;
            }
            mod = ly_ctx_get_module_by_ns(ctx, ns->value, NULL);
            if (mod) {
                /* adjust out size (it can even decrease in some strange cases) */
                out_size += strlen(mod->name) - pref_len;
                out = ly_realloc(out, out_size);
                if (!out) {
                    LOGMEM;
                    goto error;
                }

                /* copy any beginning */
                strncpy(&out[out_used], cur_expr, ptr - cur_expr);
                out_used += ptr - cur_expr;

                /* copy the model name */
                strcpy(&out[out_used], mod->name);
                out_used += strlen(mod->name);

                /* copy the rest */
                strncpy(&out[out_used], end, (exp->tok_len[i] - pref_len) - (ptr - cur_expr));
                out_used += (exp->tok_len[i] - pref_len) - (ptr - cur_expr);
            } else {
                strncpy(&out[out_used], &exp->expr[exp->expr_pos[i]], exp->tok_len[i]);
                out_used += exp->tok_len[i];
            }
        } else {
            strncpy(&out[out_used], &exp->expr[exp->expr_pos[i]], exp->tok_len[i]);
            out_used += exp->tok_len[i];
        }
    }
    out[out_used] = '\0';

    lyxp_exp_free(exp);
    return lydict_insert_zc(ctx, out);

error:
    free(out);
    lyxp_exp_free(exp);
    return NULL;
}

const char *
transform_schema2json(const struct lys_module *module, const char *expr)
{
    const char *end, *cur_expr, *ptr;
    char *out;
    uint16_t i;
    size_t out_size, out_used, pref_len;
    const struct lys_module *mod;
    struct lyxp_expr *exp = NULL;

    out_size = strlen(expr) + 1;
    out = malloc(out_size);
    if (!out) {
        LOGMEM;
        return NULL;
    }
    out_used = 0;

    exp = lyxp_parse_expr(expr);
    if (!exp) {
        goto error;
    }

    for (i = 0; i < exp->used; ++i) {
        cur_expr = &exp->expr[exp->expr_pos[i]];

        /* copy WS */
        if (i && ((end = exp->expr + exp->expr_pos[i - 1] + exp->tok_len[i - 1]) != cur_expr)) {
            strncpy(&out[out_used], end, cur_expr - end);
            out_used += cur_expr - end;
        }

        if ((exp->tokens[i] == LYXP_TOKEN_NAMETEST) && (end = strnchr(cur_expr, ':', exp->tok_len[i]))) {
            /* get the module */
            pref_len = end - cur_expr;
            mod = lys_get_import_module(module, cur_expr, pref_len, NULL, 0);
            if (!mod) {
                LOGVAL(LYE_INMOD_LEN, LY_VLOG_NONE, NULL, pref_len, cur_expr);
                goto error;
            }

            /* adjust out size (it can even decrease in some strange cases) */
            out_size += strlen(mod->name) - pref_len;
            out = ly_realloc(out, out_size);
            if (!out) {
                LOGMEM;
                goto error;
            }

            /* copy the model name */
            strcpy(&out[out_used], mod->name);
            out_used += strlen(mod->name);

            /* copy the rest */
            strncpy(&out[out_used], end, exp->tok_len[i] - pref_len);
            out_used += exp->tok_len[i] - pref_len;
        } else if ((exp->tokens[i] == LYXP_TOKEN_LITERAL) && (end = strnchr(cur_expr, ':', exp->tok_len[i]))) {
            ptr = end;
            while (isalnum(ptr[-1]) || (ptr[-1] == '_') || (ptr[-1] == '-') || (ptr[-1] == '.')) {
                --ptr;
            }

            /* get the module */
            pref_len = end - ptr;
            mod = lys_get_import_module(module, ptr, pref_len, NULL, 0);
            if (mod) {
                /* adjust out size (it can even decrease in some strange cases) */
                out_size += strlen(mod->name) - pref_len;
                out = ly_realloc(out, out_size);
                if (!out) {
                    LOGMEM;
                    goto error;
                }

                /* copy any beginning */
                strncpy(&out[out_used], cur_expr, ptr - cur_expr);
                out_used += ptr - cur_expr;

                /* copy the model name */
                strcpy(&out[out_used], mod->name);
                out_used += strlen(mod->name);

                /* copy the rest */
                strncpy(&out[out_used], end, (exp->tok_len[i] - pref_len) - (ptr - cur_expr));
                out_used += (exp->tok_len[i] - pref_len) - (ptr - cur_expr);
            } else {
                strncpy(&out[out_used], &exp->expr[exp->expr_pos[i]], exp->tok_len[i]);
                out_used += exp->tok_len[i];
            }
        } else {
            strncpy(&out[out_used], &exp->expr[exp->expr_pos[i]], exp->tok_len[i]);
            out_used += exp->tok_len[i];
        }
    }
    out[out_used] = '\0';

    lyxp_exp_free(exp);
    return lydict_insert_zc(module->ctx, out);

error:
    free(out);
    lyxp_exp_free(exp);
    return NULL;
}

const char *
transform_iffeat_schema2json(const struct lys_module *module, const char *expr)
{
    const char *in, *id;
    char *out, *col;
    size_t out_size, out_used, id_len, rc;
    const struct lys_module *mod;

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
        rc = parse_identifier(id);
        if (rc < id_len) {
            LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, id[rc], &id[rc]);
            free(out);
            return NULL;
        }

        /* get the module */
        mod = lys_get_import_module(module, id, id_len, NULL, 0);
        if (!mod) {
            LOGVAL(LYE_INMOD_LEN, LY_VLOG_NONE, NULL, id_len, id);
            free(out);
            return NULL;
        }

        /* adjust out size (it can even decrease in some strange cases) */
        out_size += strlen(mod->name) - id_len;
        out = ly_realloc(out, out_size);
        if (!out) {
            LOGMEM;
            return NULL;
        }

        /* copy the data before prefix */
        strncpy(&out[out_used], in, id - in);
        out_used += id - in;

        /* copy the model name */
        strcpy(&out[out_used], mod->name);
        out_used += strlen(mod->name);

        /* copy ':' */
        out[out_used] = ':';
        ++out_used;

        /* finally adjust in pointer for next round */
        in = col + 1;
    }

    /* unreachable */
    LOGINT;
    return NULL;
}

int
ly_new_node_validity(const struct lys_node *schema)
{
    int validity;

    validity = LYD_VAL_OK;
    switch (schema->nodetype) {
    case LYS_LEAF:
    case LYS_LEAFLIST:
        if (((struct lys_node_leaf *)schema)->type.base == LY_TYPE_LEAFREF) {
            validity |= LYD_VAL_LEAFREF;
        }
        validity |= LYD_VAL_MAND;
        break;
    case LYS_LIST:
        validity |= LYD_VAL_UNIQUE;
        /* fallthrough */
    case LYS_CONTAINER:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        validity |= LYD_VAL_MAND;
        break;
    default:
        break;
    }

    return validity;
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

int64_t
dec_pow(uint8_t exp)
{
    int64_t ret = 1;
    uint8_t i;

    for (i = 0; i < exp; ++i) {
        ret *= 10;
    }

    return ret;
}

int
dec64cmp(int64_t num1, uint8_t dig1, int64_t num2, uint8_t dig2)
{
    if (dig1 < dig2) {
        num2 /= dec_pow(dig2 - dig1);
    } else if (dig1 > dig2) {
        num1 /= dec_pow(dig1 - dig2);
    }

    if (num1 == num2) {
        return 0;
    }
    return (num1 > num2 ? 1 : -1);
}
