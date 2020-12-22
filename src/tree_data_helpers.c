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
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"

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
lyd_node_children_p(struct lyd_node *node)
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
lyd_parent(const struct lyd_node *node)
{
    if (!node) {
        return NULL;
    }

    return (struct lyd_node *)(node)->parent;
}

API struct lyd_node *
lyd_child(const struct lyd_node *node)
{
    struct lyd_node **children;

    if (!node) {
        return NULL;
    }

    if (!node->schema) {
        /* opaq node */
        return ((struct lyd_node_opaq *)(node))->child;
    }

    children = lyd_node_children_p((struct lyd_node *)node);
    if (children) {
        return *children;
    } else {
        return NULL;
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
        return ((struct lyd_node_opaq *)(node))->child;
    }

    children = lyd_node_children_p((struct lyd_node *)node);
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
        case LY_PREF_XML:
            return ly_ctx_get_module_implemented_ns(LYD_CTX(node), opaq->name.module_ns);
        case LY_PREF_JSON:
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
lyd_parse_set_data_flags(struct lyd_node *node, struct ly_set *when_check, struct lyd_meta **meta, uint32_t options)
{
    struct lyd_meta *meta2, *prev_meta = NULL;

    if (lysc_has_when(node->schema)) {
        if (!(options & LYD_PARSE_ONLY)) {
            /* remember we need to evaluate this node's when */
            LY_CHECK_RET(ly_set_add(when_check, node, 1, NULL), );
        }
    }

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

API LY_ERR
lyd_any_copy_value(struct lyd_node *trg, const union lyd_any_value *value, LYD_ANYDATA_VALUETYPE value_type)
{
    struct lyd_node_any *t;

    assert(trg->schema->nodetype & LYS_ANYDATA);

    t = (struct lyd_node_any *)trg;

    /* free trg */
    switch (t->value_type) {
    case LYD_ANYDATA_DATATREE:
        lyd_free_all(t->value.tree);
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        FREE_STRING(LYD_CTX(trg), t->value.str);
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

LYB_HASH
lyb_hash(struct lysc_node *sibling, uint8_t collision_id)
{
    const struct lys_module *mod;
    uint32_t full_hash;
    LYB_HASH hash;

    if ((collision_id < LYS_NODE_HASH_COUNT) && sibling->hash[collision_id]) {
        return sibling->hash[collision_id];
    }

    mod = sibling->module;

    full_hash = dict_hash_multi(0, mod->name, strlen(mod->name));
    full_hash = dict_hash_multi(full_hash, sibling->name, strlen(sibling->name));
    if (collision_id) {
        size_t ext_len;

        if (collision_id > strlen(mod->name)) {
            /* fine, we will not hash more bytes, just use more bits from the hash than previously */
            ext_len = strlen(mod->name);
        } else {
            /* use one more byte from the module name than before */
            ext_len = collision_id;
        }
        full_hash = dict_hash_multi(full_hash, mod->name, ext_len);
    }
    full_hash = dict_hash_multi(full_hash, NULL, 0);

    /* use the shortened hash */
    hash = full_hash & (LYB_HASH_MASK >> collision_id);
    /* add colision identificator */
    hash |= LYB_HASH_COLLISION_ID >> collision_id;

    /* save this hash */
    if (collision_id < LYS_NODE_HASH_COUNT) {
        sibling->hash[collision_id] = hash;
    }

    return hash;
}

ly_bool
lyb_has_schema_model(const struct lysc_node *sibling, const struct lys_module **models)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(models, u) {
        if (sibling->module == models[u]) {
            return 1;
        }
    }

    return 0;
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
ly_free_prefix_data(LY_PREFIX_FORMAT format, void *prefix_data)
{
    struct ly_set *ns_list;
    struct lysc_prefix *prefixes;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;

    if (!prefix_data) {
        return;
    }

    switch (format) {
    case LY_PREF_XML:
        ns_list = prefix_data;
        for (i = 0; i < ns_list->count; ++i) {
            free(((struct lyxml_ns *)ns_list->objs[i])->prefix);
            free(((struct lyxml_ns *)ns_list->objs[i])->uri);
        }
        ly_set_free(ns_list, free);
        break;
    case LY_PREF_SCHEMA_RESOLVED:
        prefixes = prefix_data;
        LY_ARRAY_FOR(prefixes, u) {
            free(prefixes[u].prefix);
        }
        LY_ARRAY_FREE(prefixes);
        break;
    case LY_PREF_SCHEMA:
    case LY_PREF_JSON:
        break;
    }
}

LY_ERR
ly_dup_prefix_data(const struct ly_ctx *ctx, LY_PREFIX_FORMAT format, const void *prefix_data,
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
    case LY_PREF_SCHEMA:
        *prefix_data_p = (void *)prefix_data;
        break;
    case LY_PREF_SCHEMA_RESOLVED:
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
    case LY_PREF_XML:
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
    case LY_PREF_JSON:
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
ly_store_prefix_data(const struct ly_ctx *ctx, const char *value, size_t value_len, LY_PREFIX_FORMAT format,
        void *prefix_data, LY_PREFIX_FORMAT *format_p, void **prefix_data_p)
{
    LY_ERR ret = LY_SUCCESS;
    const char *start, *stop;
    const struct lys_module *mod;
    struct lyxml_ns *ns;
    struct ly_set *ns_list;
    struct lysc_prefix *prefixes = NULL, *val_pref;

    switch (format) {
    case LY_PREF_SCHEMA:
    case LY_PREF_XML:
        /* copy all referenced modules... */
        if (format == LY_PREF_XML) {
            /* ...as prefix - namespace pairs */
            LY_CHECK_GOTO(ret = ly_set_new(&ns_list), cleanup);
            *format_p = LY_PREF_XML;
            *prefix_data_p = ns_list;
        } else {
            /* ...as prefix - module pairs */
            assert(format == LY_PREF_SCHEMA);
            LY_ARRAY_CREATE_GOTO(ctx, prefixes, 0, ret, cleanup);
            *format_p = LY_PREF_SCHEMA_RESOLVED;
            *prefix_data_p = prefixes;
        }

        /* add all used prefixes */
        for (stop = start = value; (size_t)(stop - value) < value_len; start = stop) {
            size_t bytes;
            uint32_t c;

            ly_getutf8(&stop, &c, &bytes);
            if (is_xmlqnamestartchar(c)) {
                for (ly_getutf8(&stop, &c, &bytes);
                        is_xmlqnamechar(c) && (size_t)(stop - value) < value_len;
                        ly_getutf8(&stop, &c, &bytes)) {}
                stop = stop - bytes;
                if (*stop == ':') {
                    /* we have a possible prefix */
                    size_t len = stop - start;

                    /* do we already have the prefix? */
                    mod = ly_type_store_resolve_prefix(ctx, start, len, *format_p, *prefix_data_p);
                    if (!mod) {
                        mod = ly_type_store_resolve_prefix(ctx, start, len, format, prefix_data);
                        if (mod) {
                            if (*format_p == LY_PREF_XML) {
                                /* store a new prefix - namespace pair */
                                ns = calloc(1, sizeof *ns);
                                LY_CHECK_ERR_GOTO(!ns, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                                LY_CHECK_GOTO(ret = ly_set_add(ns_list, ns, 1, NULL), cleanup);

                                ns->prefix = strndup(start, len);
                                LY_CHECK_ERR_GOTO(!ns->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                                ns->uri = strdup(mod->ns);
                                LY_CHECK_ERR_GOTO(!ns->uri, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                            } else {
                                assert(*format_p == LY_PREF_SCHEMA_RESOLVED);
                                /* store a new prefix - module pair */
                                LY_ARRAY_NEW_GOTO(ctx, prefixes, val_pref, ret, cleanup);
                                *prefix_data_p = prefixes;

                                val_pref->prefix = strndup(start, len);
                                LY_CHECK_ERR_GOTO(!val_pref->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                                val_pref->mod = mod;
                            }
                        } /* else it is not even defined */
                    } /* else the prefix is already present */
                }
                stop = stop + bytes;
            }
        }
        break;
    case LY_PREF_SCHEMA_RESOLVED:
    case LY_PREF_JSON:
        /* simply copy all the prefix data */
        *format_p = format;
        LY_CHECK_GOTO(ret = ly_dup_prefix_data(ctx, format, prefix_data, prefix_data_p), cleanup);
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
ly_format2str(LY_PREFIX_FORMAT format)
{
    switch (format) {
    case LY_PREF_SCHEMA:
        return "schema imports";
    case LY_PREF_SCHEMA_RESOLVED:
        return "schema stored mapping";
    case LY_PREF_XML:
        return "XML prefixes";
    case LY_PREF_JSON:
        return "JSON module names";
    default:
        break;
    }

    return NULL;
}
