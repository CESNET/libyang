/**
 * @file tree_data_helpers.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsing and validation helper functions for data trees
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _POSIX_C_SOURCE 200809L /* strdup, strndup */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "log.h"
#include "lyb.h"
#include "parser_data.h"
#include "printer_data.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"
#include "xml.h"

struct lyd_node *
lys_getnext_data(const struct lyd_node *last, const struct lyd_node *sibling, const struct lysc_node **slast,
        const struct lysc_node *parent, const struct lysc_module *module)
{
    const struct lysc_node *siter = NULL;
    struct lyd_node *match = NULL;

    assert(parent || module);
    assert(!last || (slast && *slast));

    if (slast) {
        siter = *slast;
    }

    if (last && last->next && (last->next->schema == siter)) {
        /* return next data instance */
        return last->next;
    }

    /* find next schema node data instance */
    while ((siter = lys_getnext(siter, parent, module, 0))) {
        if (!lyd_find_sibling_val(sibling, siter, NULL, 0, &match)) {
            break;
        }
    }

    if (slast) {
        *slast = siter;
    }
    return match;
}

struct lyd_node **
lyd_node_child_p(struct lyd_node *node)
{
    assert(node);

    if (!node->schema) {
        return &((struct lyd_node_opaq *)node)->child;
    } else {
        switch (node->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
            return &((struct lyd_node_inner *)node)->child;
        default:
            return NULL;
        }
    }
}

API struct lyd_node *
lyd_child_no_keys(const struct lyd_node *node)
{
    struct lyd_node **children;

    if (!node) {
        return NULL;
    }

    if (!node->schema) {
        /* opaq node */
        return ((struct lyd_node_opaq *)node)->child;
    }

    children = lyd_node_child_p((struct lyd_node *)node);
    if (children) {
        struct lyd_node *child = *children;
        while (child && child->schema && (child->schema->flags & LYS_KEY)) {
            child = child->next;
        }
        return child;
    } else {
        return NULL;
    }
}

API const struct lys_module *
lyd_owner_module(const struct lyd_node *node)
{
    const struct lysc_node *schema;
    const struct lyd_node_opaq *opaq;

    if (!node) {
        return NULL;
    }

    if (!node->schema) {
        opaq = (struct lyd_node_opaq *)node;
        switch (opaq->format) {
        case LY_VALUE_XML:
            return ly_ctx_get_module_implemented_ns(LYD_CTX(node), opaq->name.module_ns);
        case LY_VALUE_JSON:
            return ly_ctx_get_module_implemented(LYD_CTX(node), opaq->name.module_name);
        default:
            return NULL;
        }
    }

    for (schema = node->schema; schema->parent; schema = schema->parent) {}
    return schema->module;
}

const struct lys_module *
lyd_mod_next_module(struct lyd_node *tree, const struct lys_module *module, const struct ly_ctx *ctx, uint32_t *i,
        struct lyd_node **first)
{
    struct lyd_node *iter;
    const struct lys_module *mod;

    /* get the next module */
    if (module) {
        if (*i) {
            mod = NULL;
        } else {
            mod = module;
            ++(*i);
        }
    } else {
        do {
            mod = ly_ctx_get_module_iter(ctx, i);
        } while (mod && !mod->implemented);
    }

    /* find its data */
    *first = NULL;
    if (mod) {
        LY_LIST_FOR(tree, iter) {
            if (lyd_owner_module(iter) == mod) {
                *first = iter;
                break;
            }
        }
    }

    return mod;
}

const struct lys_module *
lyd_data_next_module(struct lyd_node **next, struct lyd_node **first)
{
    const struct lys_module *mod;

    if (!*next) {
        /* all data traversed */
        *first = NULL;
        return NULL;
    }

    *first = *next;

    /* prepare next */
    mod = lyd_owner_module(*next);
    LY_LIST_FOR(*next, *next) {
        if (lyd_owner_module(*next) != mod) {
            break;
        }
    }

    return mod;
}

LY_ERR
lyd_parse_check_keys(struct lyd_node *node)
{
    const struct lysc_node *skey = NULL;
    const struct lyd_node *key;

    assert(node->schema->nodetype == LYS_LIST);

    key = lyd_child(node);
    while ((skey = lys_getnext(skey, node->schema, NULL, 0)) && (skey->flags & LYS_KEY)) {
        if (!key || (key->schema != skey)) {
            LOGVAL(LYD_CTX(node), LY_VCODE_NOKEY, skey->name);
            return LY_EVALID;
        }

        key = key->next;
    }

    return LY_SUCCESS;
}

void
lyd_parse_set_data_flags(struct lyd_node *node, struct ly_set *when_check, struct ly_set *exts_check, struct lyd_meta **meta,
        uint32_t options)
{
    struct lyd_meta *meta2, *prev_meta = NULL;

    if (lysc_has_when(node->schema)) {
        if (!(options & LYD_PARSE_ONLY)) {
            /* remember we need to evaluate this node's when */
            LY_CHECK_RET(ly_set_add(when_check, node, 1, NULL), );
        }
    }
    LY_CHECK_RET(lysc_node_ext_tovalidate(exts_check, node), );

    LY_LIST_FOR(*meta, meta2) {
        if (!strcmp(meta2->name, "default") && !strcmp(meta2->annotation->module->name, "ietf-netconf-with-defaults") &&
                meta2->value.boolean) {
            /* node is default according to the metadata */
            node->flags |= LYD_DEFAULT;

            /* delete the metadata */
            if (prev_meta) {
                prev_meta->next = meta2->next;
            } else {
                *meta = (*meta)->next;
            }
            lyd_free_meta_single(meta2);
            break;
        }

        prev_meta = meta2;
    }
}

API const char *
lyd_get_value(const struct lyd_node *node)
{
    if (!node) {
        return NULL;
    }

    if (!node->schema) {
        return ((struct lyd_node_opaq *)node)->value;
    } else if (node->schema->nodetype & LYD_NODE_TERM) {
        const struct lyd_value *val = &((struct lyd_node_term *)node)->value;
        return val->canonical ? val->canonical :
               val->realtype->plugin->print(LYD_CTX(node), val, LY_VALUE_CANON, NULL, NULL, NULL);
    }
    return NULL;
}

API const char *
lyd_get_meta_value(const struct lyd_meta *meta)
{
    if (!meta) {
        return NULL;
    }

    return meta->value.canonical ? meta->value.canonical :
           meta->value.realtype->plugin->print(meta->annotation->module->ctx, &meta->value, LY_VALUE_CANON, NULL, NULL, NULL);
}

API LY_ERR
lyd_any_value_str(const struct lyd_node *any, char **value_str)
{
    const struct lyd_node_any *a;
    struct lyd_node *tree = NULL;
    const char *str = NULL;
    ly_bool dynamic = 0;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, any, value_str, LY_EINVAL);
    LY_CHECK_ARG_RET(NULL, any->schema, any->schema->nodetype & LYS_ANYDATA, LY_EINVAL);

    a = (struct lyd_node_any *)any;
    *value_str = NULL;

    if (!a->value.str) {
        /* there is no value in the union */
        return LY_SUCCESS;
    }

    switch (a->value_type) {
    case LYD_ANYDATA_LYB:
        /* parse into a data tree */
        ret = lyd_parse_data_mem(LYD_CTX(any), a->value.mem, LYD_LYB, LYD_PARSE_ONLY, 0, &tree);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;
        break;
    case LYD_ANYDATA_DATATREE:
        tree = a->value.tree;
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        /* simply use the string */
        str = a->value.str;
        break;
    }

    if (tree) {
        /* print into a string */
        ret = lyd_print_mem(value_str, tree, LYD_XML, LYD_PRINT_WITHSIBLINGS);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        assert(str);
        *value_str = strdup(str);
        LY_CHECK_ERR_GOTO(!*value_str, LOGMEM(LYD_CTX(any)), cleanup);
    }

    /* success */

cleanup:
    if (dynamic) {
        lyd_free_all(tree);
    }
    return ret;
}

API LY_ERR
lyd_any_copy_value(struct lyd_node *trg, const union lyd_any_value *value, LYD_ANYDATA_VALUETYPE value_type)
{
    struct lyd_node_any *t;

    LY_CHECK_ARG_RET(NULL, trg, LY_EINVAL);
    LY_CHECK_ARG_RET(NULL, trg->schema, trg->schema->nodetype & LYS_ANYDATA, LY_EINVAL);

    t = (struct lyd_node_any *)trg;

    /* free trg */
    switch (t->value_type) {
    case LYD_ANYDATA_DATATREE:
        lyd_free_all(t->value.tree);
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        lydict_remove(LYD_CTX(trg), t->value.str);
        break;
    case LYD_ANYDATA_LYB:
        free(t->value.mem);
        break;
    }
    t->value.str = NULL;

    if (!value) {
        /* only free value in this case */
        return LY_SUCCESS;
    }

    /* copy src */
    t->value_type = value_type;
    switch (value_type) {
    case LYD_ANYDATA_DATATREE:
        if (value->tree) {
            LY_CHECK_RET(lyd_dup_siblings(value->tree, NULL, LYD_DUP_RECURSIVE, &t->value.tree));
        }
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        if (value->str) {
            LY_CHECK_RET(lydict_insert(LYD_CTX(trg), value->str, 0, &t->value.str));
        }
        break;
    case LYD_ANYDATA_LYB:
        if (value->mem) {
            int len = lyd_lyb_data_length(value->mem);
            LY_CHECK_RET(len == -1, LY_EINVAL);
            t->value.mem = malloc(len);
            LY_CHECK_ERR_RET(!t->value.mem, LOGMEM(LYD_CTX(trg)), LY_EMEM);
            memcpy(t->value.mem, value->mem, len);
        }
        break;
    }

    return LY_SUCCESS;
}

void
lyd_del_move_root(struct lyd_node **root, const struct lyd_node *to_del, const struct lys_module *mod)
{
    if (*root && (lyd_owner_module(*root) != mod)) {
        /* there are no data of mod so this is simply the first top-level sibling */
        mod = NULL;
    }

    if ((*root != to_del) || (*root)->parent) {
        return;
    }

    *root = (*root)->next;
    if (mod && *root && (lyd_owner_module(to_del) != lyd_owner_module(*root))) {
        /* there are no more nodes from mod */
        *root = lyd_first_sibling(*root);
    }
}

void
ly_free_prefix_data(LY_VALUE_FORMAT format, void *prefix_data)
{
    struct ly_set *ns_list;
    struct lysc_prefix *prefixes;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;

    if (!prefix_data) {
        return;
    }

    switch (format) {
    case LY_VALUE_XML:
        ns_list = prefix_data;
        for (i = 0; i < ns_list->count; ++i) {
            free(((struct lyxml_ns *)ns_list->objs[i])->prefix);
            free(((struct lyxml_ns *)ns_list->objs[i])->uri);
        }
        ly_set_free(ns_list, free);
        break;
    case LY_VALUE_SCHEMA_RESOLVED:
        prefixes = prefix_data;
        LY_ARRAY_FOR(prefixes, u) {
            free(prefixes[u].prefix);
        }
        LY_ARRAY_FREE(prefixes);
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_SCHEMA:
    case LY_VALUE_JSON:
        break;
    }
}

LY_ERR
ly_dup_prefix_data(const struct ly_ctx *ctx, LY_VALUE_FORMAT format, const void *prefix_data,
        void **prefix_data_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_ns *ns;
    struct lysc_prefix *prefixes = NULL, *orig_pref;
    struct ly_set *ns_list, *orig_ns;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;

    assert(!*prefix_data_p);

    switch (format) {
    case LY_VALUE_SCHEMA:
        *prefix_data_p = (void *)prefix_data;
        break;
    case LY_VALUE_SCHEMA_RESOLVED:
        /* copy all the value prefixes */
        orig_pref = (struct lysc_prefix *)prefix_data;
        LY_ARRAY_CREATE_GOTO(ctx, prefixes, LY_ARRAY_COUNT(orig_pref), ret, cleanup);
        *prefix_data_p = prefixes;

        LY_ARRAY_FOR(orig_pref, u) {
            if (orig_pref[u].prefix) {
                prefixes[u].prefix = strdup(orig_pref[u].prefix);
                LY_CHECK_ERR_GOTO(!prefixes[u].prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
            }
            prefixes[u].mod = orig_pref[u].mod;
            LY_ARRAY_INCREMENT(prefixes);
        }
        break;
    case LY_VALUE_XML:
        /* copy all the namespaces */
        LY_CHECK_GOTO(ret = ly_set_new(&ns_list), cleanup);
        *prefix_data_p = ns_list;

        orig_ns = (struct ly_set *)prefix_data;
        for (i = 0; i < orig_ns->count; ++i) {
            ns = calloc(1, sizeof *ns);
            LY_CHECK_ERR_GOTO(!ns, LOGMEM(ctx); ret = LY_EMEM, cleanup);
            LY_CHECK_GOTO(ret = ly_set_add(ns_list, ns, 1, NULL), cleanup);

            if (((struct lyxml_ns *)orig_ns->objs[i])->prefix) {
                ns->prefix = strdup(((struct lyxml_ns *)orig_ns->objs[i])->prefix);
                LY_CHECK_ERR_GOTO(!ns->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
            }
            ns->uri = strdup(((struct lyxml_ns *)orig_ns->objs[i])->uri);
            LY_CHECK_ERR_GOTO(!ns->uri, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        }
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_JSON:
        assert(!prefix_data);
        *prefix_data_p = NULL;
        break;
    }

cleanup:
    if (ret) {
        ly_free_prefix_data(format, *prefix_data_p);
        *prefix_data_p = NULL;
    }
    return ret;
}

LY_ERR
ly_store_prefix_data(const struct ly_ctx *ctx, const char *value, size_t value_len, LY_VALUE_FORMAT format,
        const void *prefix_data, LY_VALUE_FORMAT *format_p, void **prefix_data_p)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lys_module *mod;
    const struct lyxml_ns *ns;
    struct lyxml_ns *new_ns;
    struct ly_set *ns_list;
    struct lysc_prefix *prefixes = NULL, *val_pref;
    const char *value_iter, *value_next, *value_end;
    uint32_t substr_len;
    ly_bool is_prefix;

    switch (format) {
    case LY_VALUE_SCHEMA:
        /* copy all referenced modules as prefix - module pairs */
        if (!*prefix_data_p) {
            /* new prefix data */
            LY_ARRAY_CREATE_GOTO(ctx, prefixes, 0, ret, cleanup);
            *format_p = LY_VALUE_SCHEMA_RESOLVED;
            *prefix_data_p = prefixes;
        } else {
            /* reuse prefix data */
            assert(*format_p == LY_VALUE_SCHEMA_RESOLVED);
            prefixes = *prefix_data_p;
        }

        /* add all used prefixes */
        value_end = value + value_len;
        for (value_iter = value; value_iter; value_iter = value_next) {
            substr_len = ly_value_prefix_next(value_iter, value_end, &is_prefix, &value_next);
            if (is_prefix) {
                /* we have a possible prefix. Do we already have the prefix? */
                mod = ly_resolve_prefix(ctx, value_iter, substr_len, *format_p, *prefix_data_p);
                if (!mod) {
                    mod = ly_resolve_prefix(ctx, value_iter, substr_len, format, prefix_data);
                    if (mod) {
                        assert(*format_p == LY_VALUE_SCHEMA_RESOLVED);
                        /* store a new prefix - module pair */
                        LY_ARRAY_NEW_GOTO(ctx, prefixes, val_pref, ret, cleanup);
                        *prefix_data_p = prefixes;

                        val_pref->prefix = strndup(value_iter, substr_len);
                        LY_CHECK_ERR_GOTO(!val_pref->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                        val_pref->mod = mod;
                    } /* else it is not even defined */
                } /* else the prefix is already present */
            }
        }
        break;
    case LY_VALUE_XML:
        /* copy all referenced namespaces as prefix - namespace pairs */
        if (!*prefix_data_p) {
            /* new prefix data */
            LY_CHECK_GOTO(ret = ly_set_new(&ns_list), cleanup);
            *format_p = LY_VALUE_XML;
            *prefix_data_p = ns_list;
        } else {
            /* reuse prefix data */
            assert(*format_p == LY_VALUE_XML);
            ns_list = *prefix_data_p;
        }

        /* add all used prefixes */
        value_end = value + value_len;
        for (value_iter = value; value_iter; value_iter = value_next) {
            substr_len = ly_value_prefix_next(value_iter, value_end, &is_prefix, &value_next);
            if (is_prefix) {
                /* we have a possible prefix. Do we already have the prefix? */
                ns = lyxml_ns_get(ns_list, value_iter, substr_len);
                if (!ns) {
                    ns = lyxml_ns_get(prefix_data, value_iter, substr_len);
                    if (ns) {
                        /* store a new prefix - namespace pair */
                        new_ns = calloc(1, sizeof *new_ns);
                        LY_CHECK_ERR_GOTO(!new_ns, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                        LY_CHECK_GOTO(ret = ly_set_add(ns_list, new_ns, 1, NULL), cleanup);

                        new_ns->prefix = strndup(value_iter, substr_len);
                        LY_CHECK_ERR_GOTO(!new_ns->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                        new_ns->uri = strdup(ns->uri);
                        LY_CHECK_ERR_GOTO(!new_ns->uri, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                    } /* else it is not even defined */
                } /* else the prefix is already present */
            }
        }
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_SCHEMA_RESOLVED:
    case LY_VALUE_JSON:
        if (!*prefix_data_p) {
            /* new prefix data - simply copy all the prefix data */
            *format_p = format;
            LY_CHECK_GOTO(ret = ly_dup_prefix_data(ctx, format, prefix_data, prefix_data_p), cleanup);
        } /* else reuse prefix data - the prefix data are always the same, nothing to do */
        break;
    }

cleanup:
    if (ret) {
        ly_free_prefix_data(*format_p, *prefix_data_p);
        *prefix_data_p = NULL;
    }
    return ret;
}

const char *
ly_format2str(LY_VALUE_FORMAT format)
{
    switch (format) {
    case LY_VALUE_CANON:
        return "canonical";
    case LY_VALUE_SCHEMA:
        return "schema imports";
    case LY_VALUE_SCHEMA_RESOLVED:
        return "schema stored mapping";
    case LY_VALUE_XML:
        return "XML prefixes";
    case LY_VALUE_JSON:
        return "JSON module names";
    default:
        break;
    }

    return NULL;
}
