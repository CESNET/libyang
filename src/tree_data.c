/**
 * @file tree_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang data structures
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
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "tree_data.h"
#include "parser.h"
#include "resolve.h"
#include "xml.h"
#include "tree_internal.h"
#include "validation.h"

API struct lyd_node *
lyd_parse(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options)
{
    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LYD_XML:
        return xml_read_data(ctx, data, options);
    case LYD_JSON:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
}

API struct lyd_node *
lyd_new(struct lyd_node *parent, struct lys_module *module, const char *name)
{
    struct lyd_node *ret;
    struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
    }

    if (resolve_sibling(module, siblings, NULL, 0, name, strlen(name), LYS_CONTAINER | LYS_INPUT | LYS_OUTPUT
                        | LYS_NOTIF | LYS_RPC, &snode) || !snode) {
        return NULL;
    }

    ret = calloc(1, sizeof *ret);
    ret->schema = snode;
    ret->prev = ret;
    if (parent) {
        if (lyd_insert(parent, ret, 0)) {
            free(ret);
            return NULL;
        }
    }

    return ret;
}

API struct lyd_node *
lyd_new_leaf_val(struct lyd_node *parent, struct lys_module *module, const char *name, LY_DATA_TYPE type,
                 lyd_val value)
{
    struct lyd_node_leaf_list *ret;
    struct lys_node *snode = NULL, *siblings;
    struct lys_type *stype;
    struct lys_module *src_mod, *dst_mod;
    char *val_str = NULL, str_num[22];
    const char *prefix;
    int i, str_len = 0, prev_len;
    uint64_t exp;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
    }

    if (resolve_sibling(module, siblings, NULL, 0, name, strlen(name), LYS_LEAFLIST | LYS_LEAF, &snode)
            || !snode) {
        return NULL;
    }

    switch (type) {
    case LY_TYPE_BINARY:
        val_str = (char *)lydict_insert(snode->module->ctx, value.binary, 0);
        break;

    case LY_TYPE_BITS:
        /* find the type definition */
        for (stype = &((struct lys_node_leaf *)snode)->type; stype->der->module; stype = &stype->der->type) {
            if (stype->base != LY_TYPE_BITS) {
                LOGINT;
                return NULL;
            }
        }

        /* concatenate set bits */
        for (i = 0; i < stype->info.bits.count; ++i) {
            if (!value.bit[i]) {
                continue;
            }

            prev_len = str_len;
            str_len += strlen(value.bit[i]->name) + 1;
            val_str = realloc((char *)val_str, str_len * sizeof(char));

            if (prev_len) {
                val_str[prev_len] = ' ';
                ++prev_len;
            }
            strcpy(val_str + prev_len, value.bit[i]->name);
        }

        val_str = (char *)lydict_insert_zc(snode->module->ctx, val_str);
        break;

    case LY_TYPE_BOOL:
        if (value.bool) {
            val_str = (char *)lydict_insert(snode->module->ctx, "true", 4);
        } else {
            val_str = (char *)lydict_insert(snode->module->ctx, "false", 5);
        }
        break;

    case LY_TYPE_DEC64:
        /* find the type definition */
        for (stype = &((struct lys_node_leaf *)snode)->type; stype->der->module; stype = &stype->der->type) {
            if (stype->base != LY_TYPE_DEC64) {
                LOGINT;
                return NULL;
            }
        }

        for (i = 0, exp = 1; i < stype->info.dec64.dig; ++i, exp *= 10);
        sprintf(str_num, "%01.1Lf", ((long double)value.dec64) / exp);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_EMPTY:
        break;

    case LY_TYPE_ENUM:
        val_str = (char *)lydict_insert(snode->module->ctx, value.enm->name, 0);
        break;

    case LY_TYPE_IDENT:
        /* TODO move to function if used somewhere else (module -> import prefix) */
        src_mod = value.ident->module;
        if (src_mod->type) {
            src_mod = ((struct lys_submodule *)src_mod)->belongsto;
        }
        dst_mod = snode->module;
        if (dst_mod->type) {
            dst_mod = ((struct lys_submodule *)dst_mod)->belongsto;
        }
        if (src_mod != dst_mod) {
            for (i = 0; i < src_mod->imp_size; ++i) {
                if (src_mod->imp[i].module == dst_mod) {
                    prefix = src_mod->imp[i].prefix;
                    break;
                }
            }
            if (!prefix) {
                LOGINT;
                return NULL;
            }
        } else {
            prefix = NULL;
        }

        if (!prefix) {
            val_str = (char *)lydict_insert(snode->module->ctx, value.ident->name, 0);
        } else {
            val_str = malloc((strlen(prefix) + 1 + strlen(value.ident->name) + 1) * sizeof(char));
            sprintf(val_str, "%s:%s", prefix, value.ident->name);
            val_str = (char *)lydict_insert_zc(snode->module->ctx, val_str);
        }
        break;

    case LY_TYPE_INST:
        /* TODO */
        break;

    case LY_TYPE_LEAFREF:
        val_str = (char *)lydict_insert(snode->module->ctx, ((struct lyd_node_leaf_list *)value.leafref)->value_str, 0);
        break;

    case LY_TYPE_STRING:
        val_str = (char *)lydict_insert(snode->module->ctx, value.string, 0);
        break;

    case LY_TYPE_INT8:
        sprintf(str_num, "%hhd", value.int8);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_INT16:
        sprintf(str_num, "%hd", value.int16);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_INT32:
        sprintf(str_num, "%d", value.int32);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_INT64:
        sprintf(str_num, "%ld", value.int64);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_UINT8:
        sprintf(str_num, "%hhu", value.uint8);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_UINT16:
        sprintf(str_num, "%hu", value.uint16);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_UINT32:
        sprintf(str_num, "%u", value.uint32);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    case LY_TYPE_UINT64:
        sprintf(str_num, "%lu", value.uint64);
        val_str = (char *)lydict_insert(snode->module->ctx, str_num, 0);
        break;

    default:
        LOGINT;
        return NULL;
    }

    ret = calloc(1, sizeof *ret);
    ret->schema = snode;
    ret->prev = (struct lyd_node *)ret;
    if (parent) {
        if (lyd_insert(parent, (struct lyd_node *)ret, 0)) {
            free(ret);
            lydict_remove(snode->module->ctx, val_str);
            return NULL;
        }
    }
    ret->value = value;
    ret->value_str = val_str;
    ret->value_type = type;

    return (struct lyd_node *)ret;
}

API struct lyd_node *
lyd_new_leaf_str(struct lyd_node *parent, struct lys_module *module, const char *name, LY_DATA_TYPE type,
                 const char *val_str)
{
    struct lyd_node_leaf_list *ret;
    struct lys_node *snode = NULL, *siblings;
    struct lys_type *stype, *utype;
    int found;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
    }

    if (resolve_sibling(module, siblings, NULL, 0, name, strlen(name), LYS_LEAFLIST | LYS_LEAF, &snode)
            || !snode) {
        return NULL;
    }

    ret = calloc(1, sizeof *ret);
    ret->schema = snode;
    ret->prev = (struct lyd_node *)ret;
    if (parent) {
        if (lyd_insert(parent, (struct lyd_node *)ret, 0)) {
            free(ret);
            return NULL;
        }
    }
    ret->value_str = val_str;
    ret->value_type = type;

    /* get the correct type struct */
    stype = &((struct lys_node_leaf *)snode)->type;
    if (stype->base == LY_TYPE_UNION) {
        found = 0;
        utype = stype;
        stype = lyp_get_next_union_type(utype, NULL, &found);
        while (stype && (stype->base != type)) {
            found = 0;
            stype = lyp_get_next_union_type(utype, stype, &found);
        }
        if (!stype) {
            free(ret);
            return NULL;
        }
    }

    if (lyp_parse_value(ret, stype, 1, NULL, 0)) {
        free(ret);
        return NULL;
    }

    return (struct lyd_node *)ret;

}

API struct lyd_node *
lyd_new_anyxml(struct lyd_node *parent, struct lys_module *module, const char *name, const char *val_xml)
{
    struct lyd_node_anyxml *ret;
    struct lys_node *siblings, *snode;
    struct lyxml_elem *root, *first_child, *last_child, *child;
    struct ly_ctx *ctx;
    char *xml;

    if ((!parent && !module) || !name || !val_xml) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (!parent) {
        siblings = module->data;
        ctx = module->ctx;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
        ctx = parent->schema->module->ctx;
    }

    if (resolve_sibling(module, siblings, NULL, 0, name, strlen(name), LYS_ANYXML, &snode)
            || !snode) {
        return NULL;
    }

    ret = calloc(1, sizeof *ret);
    ret->schema = snode;
    ret->prev = (struct lyd_node *)ret;
    if (parent) {
        if (lyd_insert(parent, (struct lyd_node *)ret, 0)) {
            free(ret);
            return NULL;
        }
    }

    /* add fake root so we can parse the data */
    asprintf(&xml, "<root>%s</root>", val_xml);
    root = lyxml_read(ctx, xml, 0);
    free(xml);

    /* remove the root */
    first_child = NULL;
    LY_TREE_FOR(root->child, child) {
        lyxml_unlink_elem(ctx, child, 1);
        if (!first_child) {
            first_child = child;
            last_child = child;
        } else {
            last_child->next = child;
            child->prev = last_child;
            last_child = child;
        }
    }
    first_child->prev = last_child;
    lyxml_free_elem(ctx, root);

    ret->value = first_child;

    return (struct lyd_node *)ret;
}

API int
lyd_insert(struct lyd_node *parent, struct lyd_node *node, int options)
{
    struct lys_node *sparent;
    struct lyd_node *iter, *next, *last;

    if (!node || !parent) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink(node);
    }

    /* check placing the node to the appropriate place according to the schema */
    sparent = node->schema->parent;
    while (!(sparent->nodetype & (LYS_CONTAINER | LYS_LIST))) {
        sparent = sparent->parent;
    }
    if (sparent != parent->schema) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (!parent->child) {
        /* add as the only child of the parent */
        parent->child = node;
    } else {
        /* add as the last child of the parent */
        parent->child->prev->next = node;
        node->prev = parent->child->prev;
        for (iter = node; iter->next; iter = iter->next);
        parent->child->prev = iter;
    }
    LY_TREE_FOR(node, iter) {
        iter->parent = parent;
        last = iter; /* remember the last of the inserted nodes */
    }

    ly_errno = 0;
    LY_TREE_FOR_SAFE(node, next, iter) {
        /* various validation checks */
        if (lyv_data_content(iter, 0, options)) {
            if (ly_errno) {
                return EXIT_FAILURE;
            } else {
                lyd_free(iter);
            }
        }

        if (iter == last) {
            /* we are done - checking only the inserted nodes */
            break;
        }
    }

    return EXIT_SUCCESS;
}

API int
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node, int options)
{
    struct lys_node *par1, *par2;
    struct lyd_node *iter, *next, *last;

    if (!node || !sibling) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink(node);
    }

    /* check placing the node to the appropriate place according to the schema */
    for (par1 = sibling->schema->parent; par1 && (par1->nodetype & (LYS_CONTAINER | LYS_LIST)); par1 = par1->parent);
    for (par2 = node->schema->parent; par2 && (par2->nodetype & (LYS_CONTAINER | LYS_LIST)); par2 = par2->parent);
    if (par1 != par2) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    LY_TREE_FOR(node, iter) {
        iter->parent = sibling->parent;
        last = iter; /* remember the last of the inserted nodes */
    }

    if (sibling->next) {
        /* adding into a middle - fix the prev pointer of the node after inserted nodes */
        last->next = sibling->next;
        sibling->next->prev = last;
    } else {
        /* at the end - fix the prev pointer of the first node */
        if (sibling->parent) {
            sibling->parent->child->prev = last;
        } else {
            for (iter = sibling; iter->prev->next; iter = iter->prev);
            iter->prev = last;
        }
    }
    sibling->next = node;
    node->prev = sibling;

    ly_errno = 0;
    LY_TREE_FOR_SAFE(node, next, iter) {
        /* various validation checks */
        if (lyv_data_content(iter, 0, options)) {
            if (ly_errno) {
                return EXIT_FAILURE;
            } else {
                lyd_free(iter);
            }
        }

        if (iter == last) {
            /* we are done - checking only the inserted nodes */
            break;
        }
    }

    return EXIT_SUCCESS;
}

API int
lyd_unlink(struct lyd_node *node)
{
    struct lyd_node *iter;

    if (!node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* unlink from siblings */
    if (node->prev->next) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last node */
        iter = node->prev;
        while (iter->prev != node) {
            iter = iter->prev;
        }
        /* update the "last" pointer from the first node */
        iter->prev = node->prev;
    }

    /* unlink from parent */
    if (node->parent) {
        if (node->parent->child == node) {
            /* the node is the first child */
            node->parent->child = node->next;
        }
        node->parent = NULL;
    }

    node->next = NULL;
    node->prev = node;

    return EXIT_SUCCESS;
}

static void
lyd_attr_free(struct ly_ctx *ctx, struct lyd_attr *attr)
{
    if (!attr) {
        return;
    }

    if (attr->next) {
        lyd_attr_free(ctx, attr->next);
    }
    lydict_remove(ctx, attr->name);
    lydict_remove(ctx, attr->value);
    free(attr);
}

API void
lyd_free(struct lyd_node *node)
{
    struct lyd_node *next, *child;

    if (!node) {
        return;
    }

    if (!(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
        /* free children */
        LY_TREE_FOR_SAFE(node->child, next, child) {
            lyd_free(child);
        }
    } else if (node->schema->nodetype == LYS_ANYXML) {
        lyxml_free_elem(node->schema->module->ctx, ((struct lyd_node_anyxml *)node)->value);
    } else {
        /* free value */
        switch(((struct lyd_node_leaf_list *)node)->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_leaf_list *)node)->value.string);
            break;
        case LY_TYPE_BITS:
            if (((struct lyd_node_leaf_list *)node)->value.bit) {
                free(((struct lyd_node_leaf_list *)node)->value.bit);
            }
            break;
        default:
            /* TODO nothing needed : LY_TYPE_BOOL, LY_TYPE_DEC64*/
            break;
        }
    }

    lyd_unlink(node);
    lyd_attr_free(node->schema->module->ctx, node->attr);
    free(node);
}

int
lyd_compare(struct lyd_node *first, struct lyd_node *second, int unique)
{
    struct lys_node_list *slist;
    struct lys_node *snode;
    struct lyd_node *diter;
    const char *val1, *val2;
    int i, j;

    assert(first);
    assert(second);

    if (first->schema != second->schema) {
        return 1;
    }

    switch (first->schema->nodetype) {
    case LYS_LEAFLIST:
        /* compare values */
        if (((struct lyd_node_leaf_list *)first)->value_str == ((struct lyd_node_leaf_list *)second)->value_str) {
            return 0;
        }
        return 1;
    case LYS_LIST:
        slist = (struct lys_node_list *)first->schema;

        if (unique) {
            /* compare unique leafs */
            for (i = 0; i < slist->unique_size; i++) {
                for (j = 0; j < slist->unique[i].leafs_size; j++) {
                    snode = (struct lys_node *)slist->unique[i].leafs[j];
                    /* use default values if the instances of unique leafs are not present */
                    val1 = val2 = ((struct lys_node_leaf *)snode)->dflt;
                    LY_TREE_FOR(first->child, diter) {
                        if (diter->schema == snode) {
                            val1 = ((struct lyd_node_leaf_list *)diter)->value_str;
                            break;
                        }
                    }
                    LY_TREE_FOR(second->child, diter) {
                        if (diter->schema == snode) {
                            val2 = ((struct lyd_node_leaf_list *)diter)->value_str;
                            break;
                        }
                    }
                    if (val1 != val2) {
                        break;
                    }
                }
                if (j && j == slist->unique[i].leafs_size) {
                    /* all unique leafs are the same in this set */
                    return 0;
                }
            }
        }

        /* compare keys */
        for (i = 0; i < slist->keys_size; i++) {
            snode = (struct lys_node *)slist->keys[i];
            val1 = val2 = NULL;
            LY_TREE_FOR(first->child, diter) {
                if (diter->schema == snode) {
                    val1 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    break;
                }
            }
            LY_TREE_FOR(second->child, diter) {
                if (diter->schema == snode) {
                    val2 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    break;
                }
            }
            if (val1 != val2) {
                return 1;
            }
        }

        return 0;
    default:
        /* no additional check is needed */
        return 0;
    }
}

API struct lyd_set *
lyd_set_new(void)
{
    return calloc(1, sizeof(struct lyd_set));
}

API void
lyd_set_free(struct lyd_set *set)
{
    if (!set) {
        return;
    }

    free(set->set);
    free(set);
}

API int
lyd_set_add(struct lyd_set *set, struct lyd_node *node)
{
    struct lyd_node **new;

    if (!set) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (set->size == set->number) {
        new = realloc(set->set, (set->size + 8) * sizeof *(set->set));
        if (!new) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        set->size += 8;
        set->set = new;
    }

    set->set[set->number++] = node;

    return EXIT_SUCCESS;
}
