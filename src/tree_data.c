
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
#define _XOPEN_SOURCE 700

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "tree_data.h"
#include "parser.h"
#include "resolve.h"
#include "xml_internal.h"
#include "tree_internal.h"
#include "validation.h"

static struct lyd_node *
lyd_parse_(struct ly_ctx *ctx, const struct lys_node *parent, const char *data, LYD_FORMAT format, int options)
{
    struct lyxml_elem *xml;
    struct lyd_node *result = NULL;

    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LYD_XML:
    case LYD_XML_FORMAT:
        xml = lyxml_read_data(ctx, data, 0);
        if (parent) {
            result = lyd_parse_output_xml(parent, xml, options);
        } else {
            result = lyd_parse_xml(ctx, xml, options);
        }
        lyxml_free(ctx, xml);
        break;
    case LYD_JSON:
        result = lyd_parse_json(ctx, parent, data, options);
        break;
    default:
        /* error */
        return NULL;
    }

    return result;
}

API struct lyd_node *
lyd_parse_data(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options)
{
    return lyd_parse_(ctx, NULL, data, format, options);
}

API struct lyd_node *
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options)
{
    struct lyd_node *ret;
    struct stat sb;
    char *data;

    if (!ctx || (fd == -1)) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s).", strerror(errno));
        return NULL;
    }

    data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        LOGERR(LY_ESYS, "Mapping file descriptor into memory failed.");
        return NULL;
    }

    ret = lyd_parse_data(ctx, data, format, options);
    munmap(data, sb.st_size);
    return ret;
}

API struct lyd_node *
lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options)
{
    int fd;
    struct lyd_node *ret;

    if (!ctx || !path) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        LOGERR(LY_ESYS, "Failed to open data file \"%s\" (%s).", path, strerror(errno));
        return NULL;
    }

    ret = lyd_parse_fd(ctx, fd, format, options);
    close(fd);
    return ret;
}

API struct lyd_node *
lyd_parse_output_data(const struct lys_node *rpc, const char *data, LYD_FORMAT format, int options)
{
    if (!rpc || (rpc->nodetype != LYS_RPC)) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    return lyd_parse_(rpc->module->ctx, rpc, data, format, options);
}

API struct lyd_node *
lyd_new(struct lyd_node *parent, const struct lys_module *module, const char *name)
{
    struct lyd_node *ret;
    const struct lys_node *snode = NULL, *siblings;

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

    if (lys_get_data_sibling(module, siblings, name, LYS_CONTAINER | LYS_LIST | LYS_NOTIF | LYS_RPC, &snode)
            || !snode) {
        return NULL;
    }

    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->schema = (struct lys_node *)snode;
    ret->prev = ret;
    if (parent) {
        if (lyd_insert(parent, ret)) {
            lyd_free(ret);
            return NULL;
        }
    }

    return ret;
}

API struct lyd_node *
lyd_new_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    struct lyd_node_leaf_list *ret;
    const struct lys_node *snode = NULL, *siblings;
    struct lys_type *stype, *type;
    int found;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            ly_errno = LY_EINVAL;
            return NULL;
        }
        siblings = parent->schema->child;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_LEAFLIST | LYS_LEAF, &snode) || !snode) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    /* create the new leaf */
    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->schema = (struct lys_node *)snode;
    ret->prev = (struct lyd_node *)ret;
    ret->value_str = lydict_insert((module ? module->ctx : parent->schema->module->ctx), val_str, 0);

    /* resolve the type correctly */
    stype = &((struct lys_node_leaf *)snode)->type;
    if (stype->base == LY_TYPE_UNION) {
        found = 0;
        type = NULL;
        while ((type = lyp_get_next_union_type(stype, type, &found))) {
            ret->value_type = type->base;
            if (!lyp_parse_value(ret, type, 1, NULL, UINT_MAX)) {
                /* success! */
                break;
            }
            found = 0;
        }

        if (!type) {
            /* fail */
            ly_errno = LY_EINVAL;
            lyd_free((struct lyd_node *)ret);
            return NULL;
        }
    } else {
        ret->value_type = stype->base;
        if (lyp_parse_value(ret, stype, 1, NULL, 0)) {
            lyd_free((struct lyd_node *)ret);
            ly_errno = LY_EINVAL;
            return NULL;
        }
    }

    /* connect to parent */
    if (parent) {
        if (lyd_insert(parent, (struct lyd_node *)ret)) {
            lyd_free((struct lyd_node *)ret);
            return NULL;
        }
    }

    return (struct lyd_node *)ret;

}

API struct lyd_node *
lyd_new_anyxml(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_xml)
{
    struct lyd_node_anyxml *ret;
    const struct lys_node *siblings, *snode;
    struct lyxml_elem *root;
    struct ly_ctx *ctx;
    char *xml;

    if ((!parent && !module) || !name) {
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

    if (lys_get_data_sibling(module, siblings, name, LYS_ANYXML, &snode) || !snode) {
        return NULL;
    }

    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->schema = (struct lys_node *)snode;
    ret->prev = (struct lyd_node *)ret;
    if (parent) {
        if (lyd_insert(parent, (struct lyd_node *)ret)) {
            lyd_free((struct lyd_node *)ret);
            return NULL;
        }
    }

    /* add fake root and top-level and parse the data */
    asprintf(&xml, "<root><%s>%s</%s></root>", name, (val_xml ? val_xml : ""), name);
    root = lyxml_read_data(ctx, xml, 0);
    free(xml);
    if (!root) {
        lyd_free((struct lyd_node *)ret);
        return NULL;
    }

    /* remove the root */
    ret->value = root->child;
    lyxml_unlink_elem(ctx, root->child, 1);

    lyxml_free(ctx, root);

    return (struct lyd_node *)ret;
}

API struct lyd_node *
lyd_new_output(const struct lys_node *rpc_schema)
{
    struct lyd_node *ret;
    struct lys_node *output;

    if (!rpc_schema || (rpc_schema->nodetype != LYS_RPC)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (rpc_schema->child->nodetype == LYS_OUTPUT) {
        output = rpc_schema->child;
    } else if (rpc_schema->child->next->nodetype == LYS_OUTPUT) {
        output = rpc_schema->child->next;
    } else {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->schema = output;
    ret->prev = ret;

    return (struct lyd_node *)ret;
}

API int
lyd_insert(struct lyd_node *parent, struct lyd_node *node)
{
    struct lys_node *sparent;
    struct lyd_node *iter;

    if (!node || !parent) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* check placing the node to the appropriate place according to the schema (if LYS_OUTPUT is returned,
     * the parent's schema will never match and it fails as it should) */
    for (sparent = lys_parent(node->schema);
         sparent && !(sparent->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_OUTPUT | LYS_NOTIF));
         sparent = lys_parent(sparent));
    if (sparent != parent->schema) {
        return EXIT_FAILURE;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink(node);
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
    }

    return EXIT_SUCCESS;
}

static int
lyd_insert_sibling(struct lyd_node *sibling, struct lyd_node *node, int before)
{
    struct lys_node *par1, *par2;
    struct lyd_node *iter, *last;

    if (sibling == node) {
        return EXIT_SUCCESS;
    }

    /* check placing the node to the appropriate place according to the schema */
    for (par1 = lys_parent(sibling->schema);
         par1 && !(par1->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF));
         par1 = lys_parent(par1));
    for (par2 = lys_parent(node->schema);
         par2 && !(par2->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF));
         par2 = lys_parent(par2));
    if (par1 != par2) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink(node);
    }

    LY_TREE_FOR(node, iter) {
        iter->parent = sibling->parent;
        last = iter;
    }

    if (before) {
        if (sibling->prev->next) {
            /* adding into the list */
            sibling->prev->next = node;
        } else if (sibling->parent) {
            /* at the beginning */
            sibling->parent->child = node;
        }
        node->prev = sibling->prev;
        sibling->prev = last;
        last->next = sibling;
    } else {
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
    }

    return EXIT_SUCCESS;
}

API int
lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node)
{
    if (!node || !sibling || lyd_insert_sibling(sibling, node, 1)) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

API int
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node)
{
    if (!node || !sibling || lyd_insert_sibling(sibling, node, 0)) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

API int
lyd_validate(struct lyd_node *node, int options)
{
    struct lyd_node *next, *iter, *to_free = NULL;

    ly_errno = 0;
    LY_TREE_DFS_BEGIN(node, next, iter) {
        if (to_free) {
            lyd_free(to_free);
            to_free = NULL;
        }

        if (lyv_data_context(iter, options, 0, NULL)) {
            return EXIT_FAILURE;
        }
        if (lyv_data_content(iter, options, 0, NULL)) {
            if (ly_errno) {
                return EXIT_FAILURE;
            } else {
                /* safe deferred removal */
                to_free = iter;
                if (iter == node) {
                    /* removing the whole subtree */
                    break;
                }
            }
        }

        LY_TREE_DFS_END(node, next, iter);
    }

    if (to_free) {
        lyd_free(to_free);
        to_free = NULL;
    }

    return EXIT_SUCCESS;
}

/* create an attribute copy */
static struct lyd_attr *
lyd_dup_attr(struct ly_ctx *ctx, struct lyd_node *parent, struct lyd_attr *attr)
{
    struct lyd_attr *ret;

    /* allocate new attr */
    if (!parent->attr) {
        parent->attr = malloc(sizeof *parent->attr);
        ret = parent->attr;
    } else {
        for (ret = parent->attr; ret->next; ret = ret->next);
        ret->next = malloc(sizeof *ret);
        ret = ret->next;
    }
    if (!ret) {
        LOGMEM;
        return NULL;
    }

    /* fill new attr except */
    ret->next = NULL;
    ret->module = attr->module;
    ret->name = lydict_insert(ctx, attr->name, 0);
    ret->value = lydict_insert(ctx, attr->value, 0);

    return ret;
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
        if (node->parent) {
            iter = node->parent->child;
        } else {
            iter = node->prev;
            while (iter->prev != node) {
                iter = iter->prev;
            }
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

API struct lyd_node *
lyd_dup(const struct lyd_node *node, int recursive)
{
    const struct lyd_node *next, *elem;
    struct lyd_node *ret, *parent, *new_node;
    struct lyd_attr *attr;
    struct lyd_node_leaf_list *new_leaf;
    struct lyd_node_anyxml *new_axml;
    struct lys_type *type;

    if (!node) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    ret = NULL;
    parent = NULL;

    /* LY_TREE_DFS */
    for (elem = next = node; elem; elem = next) {

        /* fill specific part */
        switch (elem->schema->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            new_leaf = malloc(sizeof *new_leaf);
            new_node = (struct lyd_node *)new_leaf;
            if (!new_node) {
                LOGMEM;
                return NULL;
            }

            new_leaf->value = ((struct lyd_node_leaf_list *)elem)->value;
            new_leaf->value_str = lydict_insert(elem->schema->module->ctx,
                                                ((struct lyd_node_leaf_list *)elem)->value_str, 0);
            new_leaf->value_type = ((struct lyd_node_leaf_list *)elem)->value_type;
            /* bits type must be treated specially */
            if (new_leaf->value_type == LY_TYPE_BITS) {
                for (type = &((struct lys_node_leaf *)elem->schema)->type; type->der->module; type = &type->der->type) {
                    if (type->base != LY_TYPE_BITS) {
                        LOGINT;
                        lyd_free(new_node);
                        lyd_free(ret);
                        return NULL;
                    }
                }

                new_leaf->value.bit = malloc(type->info.bits.count * sizeof *new_leaf->value.bit);
                if (!new_leaf->value.bit) {
                    LOGMEM;
                    lyd_free(new_node);
                    lyd_free(ret);
                    return NULL;
                }
                memcpy(new_leaf->value.bit, ((struct lyd_node_leaf_list *)elem)->value.bit,
                       type->info.bits.count * sizeof *new_leaf->value.bit);
            }
            break;
        case LYS_ANYXML:
            new_axml = malloc(sizeof *new_axml);
            new_node = (struct lyd_node *)new_axml;
            if (!new_node) {
                LOGMEM;
                return NULL;
            }

            new_axml->value = lyxml_dup_elem(elem->schema->module->ctx, ((struct lyd_node_anyxml *)elem)->value,
                                             NULL, 1);
            break;
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_NOTIF:
        case LYS_RPC:
            new_node = malloc(sizeof *new_node);
            if (!new_node) {
                LOGMEM;
                return NULL;
            }
            new_node->child = NULL;
            break;
        default:
            lyd_free(ret);
            LOGINT;
            return NULL;
        }

        /* fill common part */
        new_node->schema = elem->schema;
        new_node->attr = NULL;
        LY_TREE_FOR(elem->attr, attr) {
            lyd_dup_attr(elem->schema->module->ctx, new_node, attr);
        }
        new_node->next = NULL;
        new_node->prev = new_node;
        new_node->parent = NULL;

        if (!ret) {
            ret = new_node;
        }
        if (parent) {
            if (lyd_insert(parent, new_node)) {
                lyd_free(ret);
                LOGINT;
                return NULL;
            }
        }

        if (!recursive) {
            break;
        }

        /* LY_TREE_DFS_END */
        /* select element for the next run - children first */
        next = elem->child;
        /* child exception for lyd_node_leaf and lyd_node_leaflist */
        if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
            next = NULL;
        }
        if (!next) {
            /* no children, so try siblings */
            next = elem->next;
        } else {
            parent = new_node;
        }
        while (!next) {
            /* no siblings, go back through parents */
            elem = elem->parent;
            if (elem->parent == node->parent) {
                break;
            }
            if (!parent) {
                lyd_free(ret);
                LOGINT;
                return NULL;
            }
            parent = parent->parent;
            /* parent is already processed, go to its sibling */
            next = elem->next;
        }
    }

    return ret;
}

API void
lyd_free_attr(struct ly_ctx *ctx, struct lyd_node *parent, struct lyd_attr *attr, int recursive)
{
    struct lyd_attr *iter;

    if (!ctx || !attr) {
        return;
    }

    if (parent) {
        if (parent->attr == attr) {
            if (recursive) {
                parent->attr = NULL;
            } else {
                parent->attr = attr->next;
            }
        } else {
            for (iter = parent->attr; iter->next != attr; iter = iter->next);
            if (iter->next) {
                if (recursive) {
                    iter->next = NULL;
                } else {
                    iter->next = attr->next;
                }
            }
        }
    }

    if (!recursive) {
        attr->next = NULL;
    }

    for(iter = attr; iter; ) {
        attr = iter;
        iter = iter->next;

        lydict_remove(ctx, attr->name);
        lydict_remove(ctx, attr->value);
        free(attr);
    }
}

struct lyd_node *
lyd_attr_parent(struct lyd_node *root, struct lyd_attr *attr)
{
    struct lyd_node *next, *elem;
    struct lyd_attr *node_attr;

    LY_TREE_DFS_BEGIN(root, next, elem) {
        for (node_attr = elem->attr; node_attr; node_attr = node_attr->next) {
            if (node_attr == attr) {
                return elem;
            }
        }
        LY_TREE_DFS_END(root, next, elem)
    }

    return NULL;
}

API struct lyd_attr *
lyd_insert_attr(struct lyd_node *parent, const char *name, const char *value)
{
    struct lyd_attr *a, *iter;
    struct ly_ctx *ctx;
    const struct lys_module *module;
    const char *p;
    char *aux;

    if (!parent || !name || !value) {
        return NULL;
    }
    ctx = parent->schema->module->ctx;

    if ((p = strchr(name, ':'))) {
        /* search for the namespace */
        aux = strndup(name, p - name);
        if (!aux) {
            LOGMEM;
            return NULL;
        }
        module = ly_ctx_get_module(ctx, aux, NULL);
        free(aux);
        name = p + 1;

        if (!module) {
            /* module not found */
            LOGERR(LY_EINVAL, "Attribute prefix does not match any schema in the context.");
            return NULL;
        }
    } else {
        /* no prefix -> module is the same as for the parent */
        module = parent->schema->module;
    }

    a = malloc(sizeof *a);
    if (!a) {
        LOGMEM;
        return NULL;
    }
    a->module = (struct lys_module *)module;
    a->next = NULL;
    a->name = lydict_insert(ctx, name, 0);
    a->value = lydict_insert(ctx, value, 0);

    if (!parent->attr) {
        parent->attr = a;
    } else {
        for (iter = parent->attr; iter->next; iter = iter->next);
        iter->next = a;
    }

    return a;
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
        lyxml_free(node->schema->module->ctx, ((struct lyd_node_anyxml *)node)->value);
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
    lyd_free_attr(node->schema->module->ctx, node, node->attr, 1);
    free(node);
}

API char *
lyxml_serialize(const struct lyxml_elem *anyxml)
{
    FILE *stream;
    char *buf;
    size_t buf_size;

    if (!anyxml) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    stream = open_memstream(&buf, &buf_size);
    if (!stream) {
        ly_errno = LY_ESYS;
        return NULL;
    }
    if (lyxml_dump_file(stream, anyxml, 0) == 0) {
        free(buf);
        buf = NULL;
        ly_errno = LY_EINVAL;
    }
    fclose(stream);

    return buf;
}

int
lyd_compare(struct lyd_node *first, struct lyd_node *second, int unique)
{
    struct lys_node_list *slist;
    const struct lys_node *snode = NULL;
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
                for (j = 0; j < slist->unique[i].expr_size; j++) {
                    /* first */
                    diter = resolve_data_nodeid(slist->unique[i].expr[j], first->child);
                    if (diter) {
                        val1 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    } else {
                        /* use default value */
                        if (resolve_schema_nodeid(slist->unique[i].expr[j], first->schema->child, first->schema->module, LYS_LEAF, &snode)) {
                            /* error, but unique expression was checked when the schema was parsed */
                            return -1;
                        }
                        val1 = ((struct lys_node_leaf *)snode)->dflt;
                    }

                    /* second */
                    diter = resolve_data_nodeid(slist->unique[i].expr[j], second->child);
                    if (diter) {
                        val2 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    } else {
                        /* use default value */
                        if (resolve_schema_nodeid(slist->unique[i].expr[j], second->schema->child, second->schema->module, LYS_LEAF, &snode)) {
                            /* error, but unique expression was checked when the schema was parsed */
                            return -1;
                        }
                        val2 = ((struct lys_node_leaf *)snode)->dflt;
                    }

                    if (val1 != val2) {
                        break;
                    }
                }
                if (j && j == slist->unique[i].expr_size) {
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
