/**
 * @file tree.c
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

#include <assert.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "parser.h"
#include "xml.h"
#include "tree_internal.h"

void ly_submodule_free(struct ly_submodule *submodule);

static const char *
strnodetype(LY_NODE_TYPE type)
{
    switch (type) {
    case LY_NODE_AUGMENT:
        return "augment";
    case LY_NODE_CONTAINER:
        return "container";
    case LY_NODE_CHOICE:
        return "choice";
    case LY_NODE_LEAF:
        return "leaf";
    case LY_NODE_LEAFLIST:
        return "leaf-list";
    case LY_NODE_LIST:
        return "list";
    case LY_NODE_ANYXML:
        return "anyxml";
    case LY_NODE_USES:
        return "uses";
    case LY_NODE_GROUPING:
        return "grouping";
    case LY_NODE_CASE:
        return "case";
    case LY_NODE_INPUT:
        return "input";
    case LY_NODE_OUTPUT:
        return "output";
    case LY_NODE_NOTIF:
        return "notification";
    case LY_NODE_RPC:
        return "rpc";
    }

    return NULL;
}

struct ly_mnode_leaf *
find_leaf(struct ly_mnode *parent, const char *name, int len)
{
    struct ly_mnode *child;
    struct ly_mnode_leaf *result;

    if (!len) {
        len = strlen(name);
    }

    LY_TREE_FOR(parent->child, child) {
        switch (child->nodetype) {
        case LY_NODE_LEAF:
            /* direct check */
            if (child->name == name || (!strncmp(child->name, name, len) && !child->name[len])) {
                return (struct ly_mnode_leaf *)child;
            }
            break;
        case LY_NODE_USES:
            /* search recursively */
            result = find_leaf(child, name, len);
            if (result) {
                return result;
            }
            break;
        default:
            /* ignore */
            break;
        }
    }

    return NULL;
}

void
ly_mnode_unlink(struct ly_mnode *node)
{
    struct ly_mnode *parent, *first;

    if (!node) {
        return;
    }

    /* unlink from data model if necessary */
    if (node->module) {
        if (node->module->data == node) {
            node->module->data = node->next;
        }
    }

    /* store pointers to important nodes */
    parent = node->parent;
    if (parent && !parent->nodetype) {
        /* handle augments - first, unlink it from the augment parent ... */
        if (parent->child == node) {
            parent->child = node->next;
        }
        /* and then continue with the target parent */
        parent = ((struct ly_augment *)parent)->target;
    }

    /* unlink from parent */
    if (parent) {
        if (parent->child == node) {
            parent->child = node->next;
        }
        node->parent = NULL;
    }

    /* unlink from siblings */
    if (node->prev == node) {
        /* there are no more siblings */
        return;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last element */
        if (parent) {
            first = parent->child;
        } else {
            first = node;
            while (node->prev->next) {
                first = node->prev;
            }
        }
        first->prev = node->prev;
    }
    if (node->prev->next) {
        node->prev->next = node->next;
    }

    /* clean up the unlinked element */
    node->next = NULL;
    node->prev = node;
}

/*
 * Add child model node at the end of the parent's child list.
 * If the child is connected somewhere (has a parent), it is completely
 * unlinked and none of the following conditions applies.
 * If the child has prev sibling(s), they are ignored (child is added at the
 * end of the child list).
 * If the child has next sibling(s), all of them are connected with the parent.
 */
int
ly_mnode_addchild(struct ly_mnode *parent, struct ly_mnode *child)
{
    struct ly_mnode *last;

    assert(parent);
    assert(child);

    /* checks */
    switch (parent->nodetype) {
    case LY_NODE_CONTAINER:
    case LY_NODE_LIST:
    case LY_NODE_GROUPING:
    case LY_NODE_USES:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        if (!(child->nodetype &
                (LY_NODE_ANYXML | LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_GROUPING | LY_NODE_LEAF |
                 LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_USES))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"%s\" (%s).",
                   strnodetype(child->nodetype), strnodetype(parent->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_CHOICE:
        if (!(child->nodetype &
                (LY_NODE_ANYXML | LY_NODE_CASE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"choice\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_CASE:
        if (!(child->nodetype &
                (LY_NODE_ANYXML | LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_USES))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"case\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_RPC:
        if (!(child->nodetype & (LY_NODE_INPUT | LY_NODE_OUTPUT | LY_NODE_GROUPING))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"rpc\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_LEAF:
    case LY_NODE_LEAFLIST:
    case LY_NODE_ANYXML:
        LOGVAL(VE_SPEC, 0, "The \"%s\" statement (%s) cannot have any substatement.",
               strnodetype(parent->nodetype), parent->name);
        return EXIT_FAILURE;
    case LY_NODE_AUGMENT:
        LOGVAL(VE_SPEC, 0, "Internal error (%s:%d)", __FILE__, __LINE__);
        return EXIT_FAILURE;
    }

    if (child->parent) {
        ly_mnode_unlink(child);
    }

    if (!parent->child) {
        /* the only/first child of the parent */
        parent->child = child;
        child->parent = parent;
        last = child;
    } else {
        /* add a new child at the end of parent's child list */
        last = parent->child->prev;
        last->next = child;
        child->prev = last;
    }
    while (last->next) {
        last = last->next;
        last->parent = parent;
    }
    parent->child->prev = last;

    return EXIT_SUCCESS;
}

/*
 * id - schema-nodeid
 *
 * node_type - LY_NODE_AUGMENT (searches also RPCs and notifications)
 *           - LY_NODE_USES    (the caller is actually either an augment or refine in a uses, only
 *                              descendant-schema-nodeid allowed, ".." not allowed)
 *           - LY_NODE_CHOICE  (search only start->child, only descendant-schema-nodeid allowed)
 */
struct ly_mnode *
resolve_schema_nodeid(const char *id, struct ly_mnode *start, struct ly_module *mod, LY_NODE_TYPE node_type)
{
    const char *name, *prefix, *ptr;
    struct ly_mnode *sibling;
    struct ly_submodule *sub_mod;
    uint32_t i, j, nam_len, pref_len;

    assert(mod);
    assert(id);

    if (id[0] == '/') {
        if (node_type & (LY_NODE_USES | LY_NODE_CHOICE)) {
            return NULL;
        }
        ptr = strchr(id+1, '/');
        prefix = id+1;
    } else {
        ptr = strchr(id, '/');
        prefix = id;
    }
    pref_len = (ptr ? (unsigned)(ptr-prefix) : strlen(prefix));

    ptr = strnchr(prefix, ':', pref_len);
    /* there is prefix */
    if (ptr) {
        nam_len = (pref_len-(ptr-prefix))-1;
        pref_len = ptr-prefix;
        name = ptr+1;

    /* no prefix used */
    } else {
        name = prefix;
        nam_len = pref_len;
        prefix = NULL;
    }

    /* absolute-schema-nodeid */
    if (id[0] == '/') {
        start = NULL;

        /* it is not the local prefix */
        if (prefix && strncmp(prefix, mod->prefix, pref_len)) {
            /* check imports */
            for (i = 0; i < mod->imp_size; i++) {
                if (!strncmp(mod->imp[i].prefix, prefix, pref_len)) {
                    start = mod->imp[i].module->data;
                    break;
                }
            }

            /* no match - check include imports */
            if (!start) {
                for (i = 0; i < mod->inc_size; i++) {
                    sub_mod = mod->inc[i].submodule;
                    for (j = 0; j < sub_mod->imp_size; j++) {
                        if (!strncmp(sub_mod->imp[j].prefix, prefix, pref_len)) {
                            start = sub_mod->imp[j].module->data;
                            break;
                        }
                    }

                    if (start) {
                        break;
                    }
                }
            }

            /* no match */
            if (!start) {
                return NULL;
            }

        /* it is likely the local prefix */
        } else {
            start = mod->data;
        }

    /* descendant-schema-nodeid */
    } else {
        assert(start);
        start = start->child;
    }

    while (1) {
        if (!strcmp(name, ".")) {
            /* this node - start does not change */
        } else if (!strcmp(name, "..")) {
            /* ".." is not allowed in refines and augment sin uses, there is no need for it there */
            if (!start || (node_type == LY_NODE_USES)) {
                return NULL;
            }
            start = start->parent;
        } else {
            sibling = NULL;
            LY_TREE_FOR(start, sibling) {
                /* match */
                if (!strncmp(name, sibling->name, nam_len)) {
                    /* prefix check, it's not our own */
                    if (prefix && strncmp(sibling->module->prefix, prefix, pref_len)) {

                        /* in choice and the prefix is not ours, error for sure */
                        if (node_type == LY_NODE_CHOICE) {
                            return NULL;
                        }

                        /* import prefix check */
                        for (i = 0; i < sibling->module->imp_size; i++) {
                            if (!strncmp(sibling->module->imp[i].prefix, prefix, pref_len)
                                    && (sibling->module->imp[i].module == sibling->module)) {
                                break;
                            }
                        }

                        /* import prefix check failed */
                        if (i == sibling->module->imp_size) {
                            /* include import prefix check */
                            for (i = 0; i < sibling->module->inc_size; i++) {
                                sub_mod = sibling->module->inc[i].submodule;
                                for (j = 0; j < sub_mod->imp_size; j++) {
                                    if (!strncmp(sub_mod->imp[j].prefix, prefix, pref_len)
                                            && (sub_mod->imp[j].module == sibling->module)) {
                                        break;
                                    }
                                }

                                if (j < sub_mod->imp_size) {
                                    break;
                                }
                            }

                            /* include import prefix check failed too - definite fail */
                            if (i == sibling->module->inc_size) {
                                return NULL;
                            }
                        }
                    }

                    /* the result node? */
                    ptr = name+nam_len;
                    if (!ptr[0]) {
                        return sibling;
                    }
                    assert(ptr[0] == '/');

                    /* check for shorthand cases - then 'start' does not change */
                    if (!sibling->parent || (sibling->parent->nodetype != LY_NODE_CHOICE)
                            || (sibling->nodetype == LY_NODE_CASE)) {
                        start = sibling->child;
                    }
                    break;
                }
            }

            /* we did not find the case in direct siblings */
            if (node_type == LY_NODE_CHOICE) {
                return NULL;
            }

            /* no match */
            if (!sibling) {
                /* on augment search also RPCs and notifications, if we are in top-level */
                if ((node_type == LY_NODE_AUGMENT) && !start->parent) {
                    /* we have searched all the data nodes */
                    if (start == start->module->data) {
                        start = start->module->rpc;
                        if (start) {
                            continue;
                        }
                    }
                    /* we have searched all the RPCs */
                    if (start == start->module->rpc) {
                        start = start->module->notif;
                        if (start) {
                            continue;
                        }
                    }
                    /* we have searched all the notifications, nothing else to search in */
                    if (start == start->module->notif) {
                        return NULL;
                    }
                }
                return NULL;
            }
        }

        assert((*(name+nam_len) == '/') || (*(name+nam_len) == '\0'));

        /* make prefix point to the next node name */
        prefix = name+nam_len;
        if (!prefix[0]) {
            return start;
        }
        ++prefix;

        /* parse prefix and node name */
        ptr = strchr(prefix, '/');
        pref_len = (ptr ? (unsigned)(ptr-prefix) : strlen(prefix));
        ptr = strnchr(prefix, ':', pref_len);

        /* there is prefix */
        if (ptr) {
            nam_len = (pref_len-(ptr-prefix))-1;
            pref_len = ptr-prefix;
            name = ptr+1;

        /* no prefix used */
        } else {
            name = prefix;
            nam_len = pref_len;
            prefix = NULL;
        }
    }

    /* cannot get here */
    return NULL;
}

API struct ly_module *
ly_module_read(struct ly_ctx *ctx, const char *data, LY_MINFORMAT format)
{
    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LY_IN_YIN:
        return yin_read_module(ctx, data);
    case LY_IN_YANG:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
}

struct ly_submodule *
ly_submodule_read(struct ly_module *module, const char *data, LY_MINFORMAT format)
{
    assert(module);
    assert(data);

    switch (format) {
    case LY_IN_YIN:
        return yin_read_submodule(module, data);
    case LY_IN_YANG:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
}

API struct ly_module *
ly_module_read_fd(struct ly_ctx *ctx, int fd, LY_MINFORMAT format)
{
    struct ly_module *module;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    /*
     * TODO
     * This is just a temporary solution to make working automatic search for
     * imported modules. This doesn't work e.g. for streams (stdin)
     */
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    module = ly_module_read(ctx, addr, format);
    munmap(addr, sb.st_size);

    return module;
}

struct ly_submodule *
ly_submodule_read_fd(struct ly_module *module, int fd, LY_MINFORMAT format)
{
    struct ly_submodule *submodule;
    struct stat sb;
    char *addr;

    assert(module);
    assert(fd >= 0);

    /*
     * TODO
     * This is just a temporary solution to make working automatic search for
     * imported modules. This doesn't work e.g. for streams (stdin)
     */
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    /* TODO addr error check */
    submodule = ly_submodule_read(module, addr, format);
    munmap(addr, sb.st_size);

    return submodule;

}

void
ly_type_dup(struct ly_ctx *ctx, struct ly_type *new, struct ly_type *old)
{
    int i;

    new->prefix = lydict_insert(ctx, old->prefix, 0);
    new->base = old->base;
    new->der = old->der;

    switch (new->base) {
    case LY_TYPE_ENUM:
        new->info.enums.count = old->info.enums.count;
        if (new->info.enums.count) {
            new->info.enums.list = calloc(new->info.enums.count, sizeof *new->info.enums.list);
            for (i = 0; i < new->info.enums.count; i++) {
                new->info.enums.list[i].name = lydict_insert(ctx, old->info.enums.list[i].name, 0);
                new->info.enums.list[i].dsc = lydict_insert(ctx, old->info.enums.list[i].dsc, 0);
                new->info.enums.list[i].ref = lydict_insert(ctx, old->info.enums.list[i].ref, 0);
            }
        }
        break;
    default:
        /* TODO */
        break;
    }
}

void
ly_type_free(struct ly_ctx *ctx, struct ly_type *type)
{
    int i;

    assert(ctx);
    if (!type) {
        return;
    }

    lydict_remove(ctx, type->prefix);

    switch (type->base) {
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; i++) {
            lydict_remove(ctx, type->info.bits.bit[i].name);
            lydict_remove(ctx, type->info.bits.bit[i].dsc);
            lydict_remove(ctx, type->info.bits.bit[i].ref);
        }
        free(type->info.bits.bit);
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            lydict_remove(ctx, type->info.enums.list[i].name);
            lydict_remove(ctx, type->info.enums.list[i].dsc);
            lydict_remove(ctx, type->info.enums.list[i].ref);
        }
        free(type->info.enums.list);
        break;
    default:
        /* TODO */
        break;
    }
}

struct ly_tpdf *
ly_tpdf_dup(struct ly_ctx *ctx, struct ly_tpdf *old, int size)
{
    struct ly_tpdf *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].name = lydict_insert(ctx, old[i].name, 0);
        result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].module = old[i].module;

        ly_type_dup(ctx, &(result[i].type), &(old[i].type));

        result[i].dflt = lydict_insert(ctx, old[i].dflt, 0);
        result[i].units = lydict_insert(ctx, old[i].units, 0);
    }

    return result;
}

void
ly_tpdf_free(struct ly_ctx *ctx, struct ly_tpdf *tpdf)
{
    assert(ctx);
    if (!tpdf) {
        return;
    }

    lydict_remove(ctx, tpdf->name);
    lydict_remove(ctx, tpdf->dsc);
    lydict_remove(ctx, tpdf->ref);

    ly_type_free(ctx, &tpdf->type);

    lydict_remove(ctx, tpdf->units);
    lydict_remove(ctx, tpdf->dflt);
}

struct ly_must *
ly_must_dup(struct ly_ctx *ctx, struct ly_must *old, int size)
{
    struct ly_must *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].cond = lydict_insert(ctx, old[i].cond, 0);
        result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(ctx, old[i].ref, 0);
        result[i].eapptag = lydict_insert(ctx, old[i].eapptag, 0);
        result[i].emsg = lydict_insert(ctx, old[i].emsg, 0);
    }

    return result;
}

void
ly_must_free(struct ly_ctx *ctx, struct ly_must *must)
{
    assert(ctx);
    if (!must) {
        return;
    }

    lydict_remove(ctx, must->cond);
    lydict_remove(ctx, must->dsc);
    lydict_remove(ctx, must->ref);
    lydict_remove(ctx, must->eapptag);
    lydict_remove(ctx, must->emsg);
}

struct ly_when *
ly_when_dup(struct ly_ctx *ctx, struct ly_when *old)
{
    struct ly_when *new;

    if (!old) {
        return NULL;
    }

    new = calloc(1, sizeof *new);
    new->cond = lydict_insert(ctx, old->cond, 0);
    new->dsc = lydict_insert(ctx, old->dsc, 0);
    new->ref = lydict_insert(ctx, old->ref, 0);

    return new;
}

void
ly_when_free(struct ly_ctx *ctx, struct ly_when *w)
{
    if (!w) {
        return;
    }

    lydict_remove(ctx, w->cond);
    lydict_remove(ctx, w->dsc);
    lydict_remove(ctx, w->ref);

    free(w);
}

struct ly_augment *
ly_augment_dup(struct ly_module *module, struct ly_mnode *parent, struct ly_augment *old, int size)
{
    struct ly_augment *new = NULL;
    int i = -1;

    if (!size) {
        return NULL;
    }

    new = calloc(size, sizeof *new);
    for (i = 0; i < size; i++) {
        new[i].target_name = lydict_insert(module->ctx, old[i].target_name, 0);
        new[i].dsc = lydict_insert(module->ctx, old[i].dsc, 0);
        new[i].ref = lydict_insert(module->ctx, old[i].ref, 0);
        new[i].flags = old[i].flags;
        /* .target = NULL; .nodetype = 0 */

        new[i].parent = parent;

        /* copy the definition of augment nodes */
        if (old[i].child) {
            new[i].child = (struct ly_mnode *)lyxml_dup_elem(module->ctx, (struct lyxml_elem *)old[i].child, NULL, 1);
        }
    }

    return new;
}

struct ly_refine *
ly_refine_dup(struct ly_ctx *ctx, struct ly_refine *old, int size)
{
    struct ly_refine *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].target = lydict_insert(ctx, old[i].target, 0);
        result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].target_type = old[i].target_type;
        result[i].must_size = old[i].must_size;
        result[i].must = ly_must_dup(ctx, old[i].must, old[i].must_size);
        if (result[i].target_type & (LY_NODE_LEAF | LY_NODE_CHOICE)) {
            result[i].mod.dflt = lydict_insert(ctx, old[i].mod.dflt, 0);
        } else if (result[i].target_type == LY_NODE_CONTAINER) {
            result[i].mod.presence = lydict_insert(ctx, old[i].mod.presence, 0);
        } else if (result[i].target_type & (LY_NODE_LIST | LY_NODE_LEAFLIST)) {
            result[i].mod.list = old[i].mod.list;
        }
    }

    return result;
}

void
ly_ident_free(struct ly_ctx *ctx, struct ly_ident *ident)
{
    struct ly_ident_der *der;

    assert(ctx);
    if (!ident) {
        return;
    }

    /*
     * if caller free only a single data model which is used (its identity is
     * reference from identity in another module), this silly freeing can lead
     * to segmentation fault. But without noting if the module is used by some
     * other, it cannot be solved.
     *
     * Possible solution is to not allow caller to remove particular schema
     * from the context.
     */
    while (ident->der) {
        der = ident->der;
        ident->der = der->next;
        free(der);
    }

    lydict_remove(ctx, ident->name);
    lydict_remove(ctx, ident->dsc);
    lydict_remove(ctx, ident->ref);

}

void
ly_grp_free(struct ly_ctx *ctx, struct ly_mnode_grp *grp)
{
    int i;

    /* handle only specific parts for LY_NODE_GROUPING */
    for (i = 0; i < grp->tpdf_size; i++) {
        ly_tpdf_free(ctx, &grp->tpdf[i]);
    }
    free(grp->tpdf);
}

void
ly_anyxml_free(struct ly_ctx *ctx, struct ly_mnode_anyxml *anyxml)
{
    int i;

    for (i = 0; i < anyxml->must_size; i++) {
        ly_must_free(ctx, &anyxml->must[i]);
    }
    free(anyxml->must);

    ly_when_free(ctx, anyxml->when);
}

void
ly_leaf_free(struct ly_ctx *ctx, struct ly_mnode_leaf *leaf)
{
    int i;

    for (i = 0; i < leaf->must_size; i++) {
        ly_must_free(ctx, &leaf->must[i]);
    }
    free(leaf->must);

    ly_when_free(ctx, leaf->when);

    ly_type_free(ctx, &leaf->type);
    lydict_remove(ctx, leaf->units);
    lydict_remove(ctx, leaf->dflt);
}

void
ly_leaflist_free(struct ly_ctx *ctx, struct ly_mnode_leaflist *llist)
{
    int i;

    for (i = 0; i < llist->must_size; i++) {
        ly_must_free(ctx, &llist->must[i]);
    }
    free(llist->must);

    ly_when_free(ctx, llist->when);

    ly_type_free(ctx, &llist->type);
    lydict_remove(ctx, llist->units);
}

void
ly_list_free(struct ly_ctx *ctx, struct ly_mnode_list *list)
{
    int i;

    /* handle only specific parts for LY_NODE_LIST */
    for (i = 0; i < list->tpdf_size; i++) {
        ly_tpdf_free(ctx, &list->tpdf[i]);
    }
    free(list->tpdf);

    for (i = 0; i < list->must_size; i++) {
        ly_must_free(ctx, &list->must[i]);
    }
    free(list->must);

    ly_when_free(ctx, list->when);

    for (i = 0; i < list->unique_size; i++) {
        free(list->unique[i].leafs);
    }
    free(list->unique);

    free(list->keys);
}

void
ly_container_free(struct ly_ctx *ctx, struct ly_mnode_container *cont)
{
    int i;

    /* handle only specific parts for LY_NODE_CONTAINER */
    lydict_remove(ctx, cont->presence);

    for (i = 0; i < cont->tpdf_size; i++) {
        ly_tpdf_free(ctx, &cont->tpdf[i]);
    }
    free(cont->tpdf);

    for (i = 0; i < cont->must_size; i++) {
        ly_must_free(ctx, &cont->must[i]);
    }
    free(cont->must);

    ly_when_free(ctx, cont->when);
}

void
ly_feature_free(struct ly_ctx *ctx, struct ly_feature *f)
{
    lydict_remove(ctx, f->name);
    lydict_remove(ctx, f->dsc);
    lydict_remove(ctx, f->ref);
    free(f->features);
}

void
ly_augment_free(struct ly_ctx *ctx, struct ly_augment *aug)
{
    lydict_remove(ctx, aug->target_name);
    lydict_remove(ctx, aug->dsc);
    lydict_remove(ctx, aug->ref);

    ly_when_free(ctx, aug->when);

    lyxml_free_elem(ctx, (struct lyxml_elem *)aug->child);
}

void
ly_uses_free(struct ly_ctx *ctx, struct ly_mnode_uses *uses)
{
    int i, j;

    for (i = 0; i < uses->refine_size; i++) {
        lydict_remove(ctx, uses->refine[i].target);
        lydict_remove(ctx, uses->refine[i].dsc);
        lydict_remove(ctx, uses->refine[i].ref);

        for (j = 0; j < uses->refine[j].must_size; j++) {
            ly_must_free(ctx, &uses->refine[i].must[j]);
        }
        free(uses->refine[i].must);

        if (uses->refine[i].target_type & (LY_NODE_LEAF | LY_NODE_CHOICE)) {
            lydict_remove(ctx, uses->refine[i].mod.dflt);
        } else if (uses->refine[i].target_type & LY_NODE_CONTAINER) {
            lydict_remove(ctx, uses->refine[i].mod.presence);
        }
    }
    free(uses->refine);

    for (i = 0; i < uses->augment_size; i++) {
        ly_augment_free(ctx, &uses->augment[i]);
    }
    free(uses->augment);

    ly_when_free(ctx, uses->when);
}

void
ly_mnode_free(struct ly_mnode *node)
{
    struct ly_ctx *ctx;
    struct ly_mnode *sub, *next;

    if (!node) {
        return;
    }

    assert(node->module);
    assert(node->module->ctx);

    ctx = node->module->ctx;

    /* common part */
    LY_TREE_FOR_SAFE(node->child, next, sub) {
        ly_mnode_free(sub);
    }
    free(node->features);

    lydict_remove(ctx, node->name);
    lydict_remove(ctx, node->dsc);
    lydict_remove(ctx, node->ref);

    /* specific part */
    switch (node->nodetype) {
    case LY_NODE_CONTAINER:
        ly_container_free(ctx, (struct ly_mnode_container *)node);
        break;
    case LY_NODE_CHOICE:
        ly_when_free(ctx, ((struct ly_mnode_choice *)node)->when);
        break;
    case LY_NODE_LEAF:
        ly_leaf_free(ctx, (struct ly_mnode_leaf *)node);
        break;
    case LY_NODE_LEAFLIST:
        ly_leaflist_free(ctx, (struct ly_mnode_leaflist *)node);
        break;
    case LY_NODE_LIST:
        ly_list_free(ctx, (struct ly_mnode_list *)node);
        break;
    case LY_NODE_ANYXML:
        ly_anyxml_free(ctx, (struct ly_mnode_anyxml *)node);
        break;
    case LY_NODE_USES:
        ly_uses_free(ctx, (struct ly_mnode_uses *)node);
        break;
    case LY_NODE_CASE:
        ly_when_free(ctx, ((struct ly_mnode_case *)node)->when);
        break;
    case LY_NODE_AUGMENT:
        /* do nothing */
        break;
    case LY_NODE_GROUPING:
    case LY_NODE_RPC:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        ly_grp_free(ctx, (struct ly_mnode_grp *)node);
        break;
    }

    /* again common part */
    ly_mnode_unlink(node);
    free(node);
}

static void
module_free_common(struct ly_module *module)
{
    struct ly_ctx *ctx;
    struct ly_mnode *mnode;
    unsigned int i;
    int j, l;

    assert(module->ctx);
    ctx = module->ctx;

    /* as first step, free the imported modules */
    for (i = 0; i < module->imp_size; i++) {
        /* get the imported module from the context and then free,
         * this check is necessary because the imported module can
         * be already removed
         */
        l = ctx->models.used;
        for (j = 0; j < l; j++) {
            if (ctx->models.list[j] == module->imp[i].module) {
                ly_module_free(module->imp[i].module);
                break;
            }
        }
    }
    free(module->imp);

    while (module->data) {
        mnode = module->data;
        module->data = mnode;
        ly_mnode_free(mnode);
    }

    lydict_remove(ctx, module->dsc);
    lydict_remove(ctx, module->ref);
    lydict_remove(ctx, module->org);
    lydict_remove(ctx, module->contact);

    for (i = 0; i < module->rev_size; i++) {
        lydict_remove(ctx, module->rev[i].dsc);
        lydict_remove(ctx, module->rev[i].ref);
    }
    free(module->rev);

    for (i = 0; i < module->ident_size; i++) {
        ly_ident_free(ctx, &module->ident[i]);
    }
    module->ident_size = 0;
    free(module->ident);

    for (i = 0; i < module->tpdf_size; i++) {
        ly_tpdf_free(ctx, &module->tpdf[i]);
    }
    free(module->tpdf);

    for (i = 0; i < module->inc_size; i++) {
        ly_submodule_free(module->inc[i].submodule);
    }
    free(module->inc);

    for (i = 0; i < module->augment_size; i++) {
        ly_augment_free(ctx, &module->augment[i]);
    }
    free(module->augment);

    for (i = 0; i < module->features_size; i++) {
        ly_feature_free(ctx, &module->features[i]);
    }
    free(module->features);

    lydict_remove(ctx, module->name);
}

void
ly_submodule_free(struct ly_submodule *submodule)
{
    if (!submodule) {
        return;
    }

    submodule->inc_size = 0;
    free(submodule->inc);
    submodule->inc = NULL;

    /* common part with struct ly_module */
    module_free_common((struct ly_module *)submodule);

    /* no specific items to free */

    free(submodule);
}

struct ly_mnode *
ly_mnode_dup(struct ly_module *module, struct ly_mnode *mnode, uint8_t flags, int recursive, unsigned int line)
{
    struct ly_mnode *retval = NULL, *aux, *child;
    struct ly_ctx *ctx = module->ctx;
    int i, j;

    struct ly_mnode_container *cont;
    struct ly_mnode_container *cont_orig = (struct ly_mnode_container *)mnode;
    struct ly_mnode_choice *choice;
    struct ly_mnode_choice *choice_orig = (struct ly_mnode_choice *)mnode;
    struct ly_mnode_leaf *leaf;
    struct ly_mnode_leaf *leaf_orig = (struct ly_mnode_leaf *)mnode;
    struct ly_mnode_leaflist *llist;
    struct ly_mnode_leaflist *llist_orig = (struct ly_mnode_leaflist *)mnode;
    struct ly_mnode_list *list;
    struct ly_mnode_list *list_orig = (struct ly_mnode_list *)mnode;
    struct ly_mnode_anyxml *anyxml;
    struct ly_mnode_anyxml *anyxml_orig = (struct ly_mnode_anyxml *)mnode;
    struct ly_mnode_uses *uses;
    struct ly_mnode_uses *uses_orig = (struct ly_mnode_uses *)mnode;
    struct ly_mnode_grp *mix;
    struct ly_mnode_grp *mix_orig = (struct ly_mnode_grp *)mnode;
    struct ly_mnode_case *cs;
    struct ly_mnode_case *cs_orig = (struct ly_mnode_case *)mnode;

    /* we cannot just duplicate memory since the strings are stored in
     * dictionary and we need to update dictionary counters.
     */

    switch (mnode->nodetype) {
    case LY_NODE_CONTAINER:
        cont = calloc(1, sizeof *cont);
        retval = (struct ly_mnode *)cont;
        break;

    case LY_NODE_CHOICE:
        choice = calloc(1, sizeof *choice);
        retval = (struct ly_mnode *)choice;
        break;

    case LY_NODE_LEAF:
        leaf = calloc(1, sizeof *leaf);
        retval = (struct ly_mnode *)leaf;
        break;

    case LY_NODE_LEAFLIST:
        llist = calloc(1, sizeof *llist);
        retval = (struct ly_mnode *)llist;
        break;

    case LY_NODE_LIST:
        list = calloc(1, sizeof *list);
        retval = (struct ly_mnode *)list;
        break;

    case LY_NODE_ANYXML:
        anyxml = calloc(1, sizeof *anyxml);
        retval = (struct ly_mnode *)anyxml;
        break;

    case LY_NODE_USES:
        uses = calloc(1, sizeof *uses);
        retval = (struct ly_mnode *)uses;
        break;

    case LY_NODE_CASE:
        cs = calloc(1, sizeof *cs);
        retval = (struct ly_mnode *)cs;
        break;

        /* exact same structure */
    case LY_NODE_GROUPING:
    case LY_NODE_RPC:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        mix = calloc(1, sizeof *mix);
        retval = (struct ly_mnode *)mix;
        break;

    default:
        goto error;
    }

    /*
     * duplicate generic part of the structure
     */
    retval->name = lydict_insert(ctx, mnode->name, 0);
    retval->dsc = lydict_insert(ctx, mnode->dsc, 0);
    retval->ref = lydict_insert(ctx, mnode->ref, 0);
    retval->flags = mnode->flags;
    if (!(retval->flags & LY_NODE_CONFIG_MASK)) {
        /* set parent's config flag */
        retval->flags |= flags & LY_NODE_CONFIG_MASK;
    }

    retval->module = module;
    retval->nodetype = mnode->nodetype;

    retval->prev = retval;

    retval->features_size = mnode->features_size;
    retval->features = calloc(retval->features_size, sizeof *retval->features);
    memcpy(retval->features, mnode->features, retval->features_size * sizeof *retval->features);

    if (recursive) {
        /* go recursively */
        LY_TREE_FOR(mnode->child, child) {
            aux = ly_mnode_dup(module, child, retval->flags, 1, line);
            if (!aux || ly_mnode_addchild(retval, aux)) {
                goto error;
            }
        }
    }

    /*
     * duplicate specific part of the structure
     */
    switch (mnode->nodetype) {
    case LY_NODE_CONTAINER:
        cont->when = ly_when_dup(ctx, cont_orig->when);
        cont->presence = lydict_insert(ctx, cont_orig->presence, 0);

        cont->must_size = cont_orig->must_size;
        cont->tpdf_size = cont_orig->tpdf_size;

        cont->must = ly_must_dup(ctx, cont_orig->must, cont->must_size);
        cont->tpdf = ly_tpdf_dup(ctx, cont_orig->tpdf, cont->tpdf_size);
        break;

    case LY_NODE_CHOICE:
        choice->when = ly_when_dup(ctx, choice_orig->when);
        if (choice->dflt) {
            LY_TREE_FOR(choice->child, child) {
                if (child->name == choice_orig->dflt->name) {
                    choice->dflt = child;
                    break;
                }
            }
        }
        break;

    case LY_NODE_LEAF:
        ly_type_dup(ctx, &(leaf->type), &(leaf_orig->type));
        leaf->units = lydict_insert(ctx, leaf_orig->units, 0);
        leaf->dflt = lydict_insert(ctx, leaf_orig->dflt, 0);

        leaf->must_size = leaf_orig->must_size;
        leaf->must = ly_must_dup(ctx, leaf_orig->must, leaf->must_size);

        leaf->when = ly_when_dup(ctx, leaf_orig->when);
        break;

    case LY_NODE_LEAFLIST:

        ly_type_dup(ctx, &(llist->type), &(llist_orig->type));
        llist->units = lydict_insert(ctx, llist_orig->units, 0);

        llist->min = llist_orig->min;
        llist->max = llist_orig->max;

        llist->must_size = llist_orig->must_size;
        llist->must = ly_must_dup(ctx, llist_orig->must, llist->must_size);

        llist->when = ly_when_dup(ctx, llist_orig->when);
        break;

    case LY_NODE_LIST:
        list->min = list_orig->min;
        list->max = list_orig->max;

        list->must_size = list_orig->must_size;
        list->tpdf_size = list_orig->tpdf_size;
        list->keys_size = list_orig->keys_size;
        list->unique_size = list_orig->unique_size;

        list->must = ly_must_dup(ctx, list_orig->must, list->must_size);
        list->tpdf = ly_tpdf_dup(ctx, list_orig->tpdf, list->tpdf_size);

        if (list->keys_size) {
            list->keys = calloc(list->keys_size, sizeof *list->keys);
            for (i = 0; i < list->keys_size; i++) {
                list->keys[i] = find_leaf(retval, list_orig->keys[i]->name, 0);
            }
        }
        if (list->unique_size) {
            list->unique = calloc(list->unique_size, sizeof *list->unique);
            for (i = 0; i < list->unique_size; i++) {
                list->unique[i].leafs = calloc(list->unique[i].leafs_size, sizeof *list->unique[i].leafs);
                for (j = 0; j < list->unique[i].leafs_size; j++) {
                    list->unique[i].leafs[j] = find_leaf(retval, list_orig->unique[i].leafs[j]->name, 0);
                }
            }
        }
        list->when = ly_when_dup(ctx, list_orig->when);
        break;

    case LY_NODE_ANYXML:
        anyxml->must_size = anyxml_orig->must_size;
        anyxml->must = ly_must_dup(ctx, anyxml_orig->must, anyxml->must_size);
        anyxml->when = ly_when_dup(ctx, anyxml_orig->when);
        break;

    case LY_NODE_USES:
        uses->grp = uses_orig->grp;
        uses->when = ly_when_dup(ctx, uses_orig->when);
        uses->refine_size = uses_orig->refine_size;
        uses->refine = ly_refine_dup(ctx, uses_orig->refine, uses_orig->refine_size);
        uses->augment_size = uses_orig->augment_size;
        uses->augment = ly_augment_dup(module, (struct ly_mnode *)uses, uses_orig->augment, uses_orig->augment_size);
        if (resolve_uses(uses, line)) {
            goto error;
        }
        break;

    case LY_NODE_CASE:
        cs->when = ly_when_dup(ctx, cs_orig->when);
        break;

    case LY_NODE_GROUPING:
    case LY_NODE_RPC:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        mix->tpdf_size = mix_orig->tpdf_size;
        mix->tpdf = ly_tpdf_dup(ctx, mix_orig->tpdf, mix->tpdf_size);
        break;

    default:
        /* LY_NODE_AUGMENT */
        goto error;
    }

    return retval;

error:
    LOGDBG("error: %s:%d", __FILE__, __LINE__);

    ly_mnode_free(retval);
    return NULL;
}

API void
ly_module_free(struct ly_module *module)
{
    struct ly_ctx *ctx;
    int i;

    if (!module) {
        return;
    }

    /* remove schema from the context */
    ctx = module->ctx;
    if (ctx->models.used) {
        for (i = 0; i < ctx->models.used; i++) {
            if (ctx->models.list[i] == module) {
                /* replace the position in the list by the last module in the list */
                ctx->models.used--;
                ctx->models.list[i] = ctx->models.list[ctx->models.used];
                ctx->models.list[ctx->models.used] = NULL;
                /* we are done */
                break;
            }
        }
    }

    /* common part with struct ly_submodule */
    module_free_common(module);

    /* specific items to free */
    lydict_remove(module->ctx, module->ns);
    lydict_remove(module->ctx, module->prefix);

    free(module);
}
