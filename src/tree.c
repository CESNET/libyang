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
#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "parse.h"
#include "parser.h"
#include "resolve.h"
#include "xml.h"
#include "tree_internal.h"

API struct lys_feature *
lys_is_disabled(struct lys_node *node, int recursive)
{
    int i;

check:
    if (node->nodetype != LYS_INPUT && node->nodetype != LYS_OUTPUT) {
        /* input/output does not have if-feature, so skip them */

        /* check local if-features */
        for (i = 0; i < node->features_size; i++) {
            if (!(node->features[i]->flags & LYS_FENABLED)) {
                return node->features[i];
            }
        }
    }

    if (!recursive) {
        return NULL;
    }

    /* go through parents */
    if (node->nodetype == LYS_AUGMENT) {
        /* go to parent actually means go to the target node */
        node = ((struct lys_node_augment *)node)->target;
        goto check;
    } else if (node->parent) {
        node = node->parent;
        goto check;
    }

    return NULL;
}

void ly_submodule_free(struct lys_submodule *submodule);

void
ly_mnode_unlink(struct lys_node *node)
{
    struct lys_node *parent, *first;

    if (!node) {
        return;
    }

    /* unlink from data model if necessary */
    if (node->module) {
        if (node->module->data == node) {
            node->module->data = node->next;
        } else if (node->module->rpc == node) {
            node->module->rpc = node->next;
        } else if (node->module->notif == node) {
            node->module->notif = node->next;
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
        parent = ((struct lys_node_augment *)parent)->target;
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
ly_mnode_addchild(struct lys_node *parent, struct lys_node *child)
{
    struct lys_node *last;

    assert(parent);
    assert(child);

    /* checks */
    switch (parent->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_GROUPING:
    case LYS_USES:
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CHOICE | LYS_CONTAINER | LYS_GROUPING | LYS_LEAF |
                 LYS_LEAFLIST | LYS_LIST | LYS_USES))) {
            LOGVAL(LYE_SPEC, 0, "Unexpected substatement \"%s\" in \"%s\" (%s).",
                   strnodetype(child->nodetype), strnodetype(parent->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_CHOICE:
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST))) {
            LOGVAL(LYE_SPEC, 0, "Unexpected substatement \"%s\" in \"choice\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_CASE:
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES))) {
            LOGVAL(LYE_SPEC, 0, "Unexpected substatement \"%s\" in \"case\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_RPC:
        if (!(child->nodetype & (LYS_INPUT | LYS_OUTPUT | LYS_GROUPING))) {
            LOGVAL(LYE_SPEC, 0, "Unexpected substatement \"%s\" in \"rpc\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYXML:
        LOGVAL(LYE_SPEC, 0, "The \"%s\" statement (%s) cannot have any substatement.",
               strnodetype(parent->nodetype), parent->name);
        return EXIT_FAILURE;
    case LYS_AUGMENT:
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CASE | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF
                | LYS_LEAFLIST | LYS_LIST | LYS_USES))) {
            LOGVAL(LYE_SPEC, 0, "Unexpected substatement \"%s\" in \"%s\" (%s).",
                   strnodetype(child->nodetype), strnodetype(parent->nodetype), parent->name);
            return EXIT_FAILURE;
        }
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

API struct lys_module *
lys_parse(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format)
{
    struct unres_schema *unres;
    struct lys_module *mod;

    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    unres = calloc(1, sizeof *unres);

    switch (format) {
    case LYS_IN_YIN:
        mod = yin_read_module(ctx, data, 1, unres);
        break;
    case LYS_IN_YANG:
        /* TODO */
        mod = NULL;
        break;
    default:
        mod = NULL;
        break;
    }

    if (resolve_unres(mod, unres)) {
        LOGERR(LY_EVALID, "There are unresolved items left.");
        lys_free(mod);
        mod = NULL;
    }
    free(unres->item);
    free(unres->type);
    free(unres->str_mnode);
    free(unres->line);
    free(unres);

    return mod;
}

struct lys_submodule *
ly_submodule_read(struct lys_module *module, const char *data, LYS_INFORMAT format, int implement)
{
    struct unres_schema *unres;
    struct lys_submodule *submod;

    assert(module);
    assert(data);

    unres = calloc(1, sizeof *unres);

    switch (format) {
    case LYS_IN_YIN:
        submod = yin_read_submodule(module, data, implement, unres);
        break;
    case LYS_IN_YANG:
        /* TODO */
        submod = NULL;
        break;
    default:
        submod = NULL;
        break;
    }

   if (resolve_unres((struct lys_module *)submod, unres)) {
        LOGERR(LY_EVALID, "There are unresolved items left.");
        ly_submodule_free(submod);
        submod = NULL;
    }
    free(unres->item);
    free(unres->type);
    free(unres->str_mnode);
    free(unres->line);
    free(unres);

    return submod;
}

struct lys_module *
lys_read_import(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    struct unres_schema *unres;
    struct lys_module *module;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    unres = calloc(1, sizeof *unres);

    /*
     * TODO
     * This is just a temporary solution to make working automatic search for
     * imported modules. This doesn't work e.g. for streams (stdin)
     */
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    switch (format) {
    case LYS_IN_YIN:
        module = yin_read_module(ctx, addr, 0, unres);
        break;
    case LYS_IN_YANG:
    default:
        /* TODO */
        munmap(addr, sb.st_size);
        return NULL;
    }
    munmap(addr, sb.st_size);

    if (resolve_unres(module, unres)) {
        LOGERR(LY_EVALID, "There are unresolved items left.");
        lys_free(module);
        module = NULL;
    }
    free(unres->item);
    free(unres->type);
    free(unres->str_mnode);
    free(unres->line);
    free(unres);

    return module;
}

API struct lys_module *
lys_read(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    struct lys_module *module;
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
    module = lys_parse(ctx, addr, format);
    munmap(addr, sb.st_size);

    return module;
}

struct lys_submodule *
ly_submodule_read_fd(struct lys_module *module, int fd, LYS_INFORMAT format, int implement)
{
    struct lys_submodule *submodule;
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
    submodule = ly_submodule_read(module, addr, format, implement);
    munmap(addr, sb.st_size);

    return submodule;

}

struct lys_restr *
ly_restr_dup(struct ly_ctx *ctx, struct lys_restr *old, int size)
{
    struct lys_restr *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].expr = lydict_insert(ctx, old[i].expr, 0);
        result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(ctx, old[i].ref, 0);
        result[i].eapptag = lydict_insert(ctx, old[i].eapptag, 0);
        result[i].emsg = lydict_insert(ctx, old[i].emsg, 0);
    }

    return result;
}

void
ly_restr_free(struct ly_ctx *ctx, struct lys_restr *restr)
{
    assert(ctx);
    if (!restr) {
        return;
    }

    lydict_remove(ctx, restr->expr);
    lydict_remove(ctx, restr->dsc);
    lydict_remove(ctx, restr->ref);
    lydict_remove(ctx, restr->eapptag);
    lydict_remove(ctx, restr->emsg);
}

void
ly_type_dup(struct lys_module *mod, struct lys_node *parent, struct lys_type *new, struct lys_type *old,
            struct unres_schema *unres)
{
    int i;

    new->prefix = lydict_insert(mod->ctx, old->prefix, 0);
    new->base = old->base;
    new->der = old->der;

    i = find_unres(unres, old, UNRES_TYPE_DER);
    if (i != -1) {
        /* HACK for unres */
        new->der = (struct lys_tpdf *)parent;
        add_unres_str(mod, unres, new, UNRES_TYPE_DER, unres->str_mnode[i], 0);
        return;
    }

    switch (new->base) {
    case LY_TYPE_BINARY:
        if (old->info.binary.length) {
            new->info.binary.length = ly_restr_dup(mod->ctx, old->info.binary.length, 1);
        }
        break;

    case LY_TYPE_BITS:
        new->info.bits.count = old->info.bits.count;
        if (new->info.bits.count) {
            new->info.bits.bit = calloc(new->info.bits.count, sizeof *new->info.bits.bit);
            for (i = 0; i < new->info.bits.count; i++) {
                new->info.bits.bit[i].name = lydict_insert(mod->ctx, old->info.bits.bit[i].name, 0);
                new->info.bits.bit[i].dsc = lydict_insert(mod->ctx, old->info.bits.bit[i].dsc, 0);
                new->info.bits.bit[i].ref = lydict_insert(mod->ctx, old->info.bits.bit[i].ref, 0);
                new->info.bits.bit[i].status = old->info.bits.bit[i].status;
                new->info.bits.bit[i].pos = old->info.bits.bit[i].pos;
            }
        }
        break;

    case LY_TYPE_DEC64:
        new->info.dec64.dig = old->info.dec64.dig;
        if (old->info.dec64.range) {
            new->info.dec64.range = ly_restr_dup(mod->ctx, old->info.dec64.range, 1);
        }
        break;

    case LY_TYPE_ENUM:
        new->info.enums.count = old->info.enums.count;
        if (new->info.enums.count) {
            new->info.enums.enm = calloc(new->info.enums.count, sizeof *new->info.enums.enm);
            for (i = 0; i < new->info.enums.count; i++) {
                new->info.enums.enm[i].name = lydict_insert(mod->ctx, old->info.enums.enm[i].name, 0);
                new->info.enums.enm[i].dsc = lydict_insert(mod->ctx, old->info.enums.enm[i].dsc, 0);
                new->info.enums.enm[i].ref = lydict_insert(mod->ctx, old->info.enums.enm[i].ref, 0);
                new->info.enums.enm[i].status = old->info.enums.enm[i].status;
                new->info.enums.enm[i].value = old->info.enums.enm[i].value;
            }
        }
        break;

    case LY_TYPE_IDENT:
        if (old->info.ident.ref) {
            new->info.ident.ref = old->info.ident.ref;
        } else {
            i = find_unres(unres, old, UNRES_TYPE_IDENTREF);
            assert(i != -1);
            add_unres_str(mod, unres, new, UNRES_TYPE_IDENTREF, unres->str_mnode[i], 0);
        }
        break;

    case LY_TYPE_INST:
        new->info.inst.req = old->info.inst.req;
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        if (old->info.num.range) {
            new->info.num.range = ly_restr_dup(mod->ctx, old->info.num.range, 1);
        }
        break;

    case LY_TYPE_LEAFREF:
        new->info.lref.path = lydict_insert(mod->ctx, old->info.lref.path, 0);
        add_unres_mnode(mod, unres, new, UNRES_TYPE_LEAFREF, parent, 0);
        break;

    case LY_TYPE_STRING:
        if (old->info.str.length) {
            new->info.str.length = ly_restr_dup(mod->ctx, old->info.str.length, 1);
        }
        new->info.str.patterns = ly_restr_dup(mod->ctx, old->info.str.patterns, old->info.str.pat_count);
        break;

    case LY_TYPE_UNION:
        new->info.uni.count = old->info.uni.count;
        if (new->info.uni.count) {
            new->info.uni.types = calloc(new->info.uni.count, sizeof *new->info.uni.types);
            for (i = 0; i < new->info.uni.count; i++) {
                ly_type_dup(mod, parent, &(new->info.uni.types[i]), &(old->info.uni.types[i]), unres);
            }
        }
        break;

    default:
        /* nothing to do for LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }
}

void
ly_type_free(struct ly_ctx *ctx, struct lys_type *type)
{
    int i;

    assert(ctx);
    if (!type) {
        return;
    }

    lydict_remove(ctx, type->prefix);

    switch (type->base) {
    case LY_TYPE_BINARY:
        ly_restr_free(ctx, type->info.binary.length);
        free(type->info.binary.length);
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; i++) {
            lydict_remove(ctx, type->info.bits.bit[i].name);
            lydict_remove(ctx, type->info.bits.bit[i].dsc);
            lydict_remove(ctx, type->info.bits.bit[i].ref);
        }
        free(type->info.bits.bit);
        break;

    case LY_TYPE_DEC64:
        ly_restr_free(ctx, type->info.dec64.range);
        free(type->info.dec64.range);
        break;

    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            lydict_remove(ctx, type->info.enums.enm[i].name);
            lydict_remove(ctx, type->info.enums.enm[i].dsc);
            lydict_remove(ctx, type->info.enums.enm[i].ref);
        }
        free(type->info.enums.enm);
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        ly_restr_free(ctx, type->info.num.range);
        free(type->info.num.range);
        break;

    case LY_TYPE_LEAFREF:
        lydict_remove(ctx, type->info.lref.path);
        break;

    case LY_TYPE_STRING:
        ly_restr_free(ctx, type->info.str.length);
        free(type->info.str.length);
        for (i = 0; i < type->info.str.pat_count; i++) {
            ly_restr_free(ctx, &type->info.str.patterns[i]);
        }
        free(type->info.str.patterns);
        break;

    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; i++) {
            ly_type_free(ctx, &type->info.uni.types[i]);
        }
        free(type->info.uni.types);
        break;

    default:
        /* nothing to do for LY_TYPE_IDENT, LY_TYPE_INST, LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }
}

struct lys_tpdf *
ly_tpdf_dup(struct lys_module *mod, struct lys_node *parent, struct lys_tpdf *old, int size, struct unres_schema *unres)
{
    struct lys_tpdf *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].name = lydict_insert(mod->ctx, old[i].name, 0);
        result[i].dsc = lydict_insert(mod->ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(mod->ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].module = old[i].module;

        ly_type_dup(mod, parent, &(result[i].type), &(old[i].type), unres);

        result[i].dflt = lydict_insert(mod->ctx, old[i].dflt, 0);
        result[i].units = lydict_insert(mod->ctx, old[i].units, 0);
    }

    return result;
}

void
ly_tpdf_free(struct ly_ctx *ctx, struct lys_tpdf *tpdf)
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

struct lys_when *
ly_when_dup(struct ly_ctx *ctx, struct lys_when *old)
{
    struct lys_when *new;

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
ly_when_free(struct ly_ctx *ctx, struct lys_when *w)
{
    if (!w) {
        return;
    }

    lydict_remove(ctx, w->cond);
    lydict_remove(ctx, w->dsc);
    lydict_remove(ctx, w->ref);

    free(w);
}

static struct lys_node_augment *
ly_augment_dup(struct lys_module *module, struct lys_node *parent, struct lys_node_augment *old, int size,
               struct unres_schema *unres)
{
    struct lys_node_augment *new = NULL;
    struct lys_node *snode;
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

        /* copy the augment nodes */
        assert(old[i].child);
        LY_TREE_FOR(old[i].child, snode) {
            ly_mnode_addchild((struct lys_node *)&new[i], ly_mnode_dup(module, snode, snode->flags, 1, unres));
        }
    }

    return new;
}

static struct lys_refine *
ly_refine_dup(struct lys_module *mod, struct lys_refine *old, int size, struct lys_node_uses *uses,
              struct unres_schema *unres)
{
    struct lys_refine *result;
    int i, j;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].target_name = lydict_insert(mod->ctx, old[i].target_name, 0);
        result[i].dsc = lydict_insert(mod->ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(mod->ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].target_type = old[i].target_type;

        result[i].must_size = old[i].must_size;
        result[i].must = ly_restr_dup(mod->ctx, old[i].must, old[i].must_size);
        for (j = 0; j < result[i].must_size; ++j) {
            add_unres_mnode(mod, unres, &result[i].must[j], UNRES_MUST, (struct lys_node *)uses, 0);
        }

        if (result[i].target_type & (LYS_LEAF | LYS_CHOICE)) {
            result[i].mod.dflt = lydict_insert(mod->ctx, old[i].mod.dflt, 0);
        } else if (result[i].target_type == LYS_CONTAINER) {
            result[i].mod.presence = lydict_insert(mod->ctx, old[i].mod.presence, 0);
        } else if (result[i].target_type & (LYS_LIST | LYS_LEAFLIST)) {
            result[i].mod.list = old[i].mod.list;
        }
    }

    return result;
}

void
ly_ident_free(struct ly_ctx *ctx, struct lys_ident *ident)
{
    struct lys_ident_der *der;

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
ly_grp_free(struct ly_ctx *ctx, struct lys_node_grp *grp)
{
    int i;

    /* handle only specific parts for LY_NODE_GROUPING */
    for (i = 0; i < grp->tpdf_size; i++) {
        ly_tpdf_free(ctx, &grp->tpdf[i]);
    }
    free(grp->tpdf);
}

void
ly_anyxml_free(struct ly_ctx *ctx, struct lys_node_anyxml *anyxml)
{
    int i;

    for (i = 0; i < anyxml->must_size; i++) {
        ly_restr_free(ctx, &anyxml->must[i]);
    }
    free(anyxml->must);

    ly_when_free(ctx, anyxml->when);
}

void
ly_leaf_free(struct ly_ctx *ctx, struct lys_node_leaf *leaf)
{
    int i;

    for (i = 0; i < leaf->must_size; i++) {
        ly_restr_free(ctx, &leaf->must[i]);
    }
    free(leaf->must);

    ly_when_free(ctx, leaf->when);

    ly_type_free(ctx, &leaf->type);
    lydict_remove(ctx, leaf->units);
    lydict_remove(ctx, leaf->dflt);
}

void
ly_leaflist_free(struct ly_ctx *ctx, struct lys_node_leaflist *llist)
{
    int i;

    for (i = 0; i < llist->must_size; i++) {
        ly_restr_free(ctx, &llist->must[i]);
    }
    free(llist->must);

    ly_when_free(ctx, llist->when);

    ly_type_free(ctx, &llist->type);
    lydict_remove(ctx, llist->units);
}

void
ly_list_free(struct ly_ctx *ctx, struct lys_node_list *list)
{
    int i;

    /* handle only specific parts for LY_NODE_LIST */
    for (i = 0; i < list->tpdf_size; i++) {
        ly_tpdf_free(ctx, &list->tpdf[i]);
    }
    free(list->tpdf);

    for (i = 0; i < list->must_size; i++) {
        ly_restr_free(ctx, &list->must[i]);
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
ly_container_free(struct ly_ctx *ctx, struct lys_node_container *cont)
{
    int i;

    /* handle only specific parts for LY_NODE_CONTAINER */
    lydict_remove(ctx, cont->presence);

    for (i = 0; i < cont->tpdf_size; i++) {
        ly_tpdf_free(ctx, &cont->tpdf[i]);
    }
    free(cont->tpdf);

    for (i = 0; i < cont->must_size; i++) {
        ly_restr_free(ctx, &cont->must[i]);
    }
    free(cont->must);

    ly_when_free(ctx, cont->when);
}

void
ly_feature_free(struct ly_ctx *ctx, struct lys_feature *f)
{
    lydict_remove(ctx, f->name);
    lydict_remove(ctx, f->dsc);
    lydict_remove(ctx, f->ref);
    free(f->features);
}

void
ly_deviation_free(struct ly_ctx *ctx, struct lys_deviation *dev)
{
    int i, j;

    lydict_remove(ctx, dev->target_name);
    lydict_remove(ctx, dev->dsc);
    lydict_remove(ctx, dev->ref);

    for (i = 0; i < dev->deviate_size; i++) {
        lydict_remove(ctx, dev->deviate[i].dflt);
        lydict_remove(ctx, dev->deviate[i].units);

        if (dev->deviate[i].mod == LY_DEVIATE_DEL) {
            for (j = 0; j < dev->deviate[i].must_size; j++) {
                ly_restr_free(ctx, &dev->deviate[i].must[j]);
            }
            free(dev->deviate[i].must);

            for (j = 0; j < dev->deviate[i].unique_size; j++) {
                free(dev->deviate[j].unique[j].leafs);
            }
            free(dev->deviate[i].unique);
        }
    }
    free(dev->deviate);
}

void
ly_augment_free(struct ly_ctx *ctx, struct lys_node_augment *aug)
{
    lydict_remove(ctx, aug->target_name);
    lydict_remove(ctx, aug->dsc);
    lydict_remove(ctx, aug->ref);

    free(aug->features);

    ly_when_free(ctx, aug->when);

    lyxml_free_elem(ctx, (struct lyxml_elem *)aug->child);
}

void
ly_uses_free(struct ly_ctx *ctx, struct lys_node_uses *uses)
{
    int i, j;

    for (i = 0; i < uses->refine_size; i++) {
        lydict_remove(ctx, uses->refine[i].target_name);
        lydict_remove(ctx, uses->refine[i].dsc);
        lydict_remove(ctx, uses->refine[i].ref);

        for (j = 0; j < uses->refine[j].must_size; j++) {
            ly_restr_free(ctx, &uses->refine[i].must[j]);
        }
        free(uses->refine[i].must);

        if (uses->refine[i].target_type & (LYS_LEAF | LYS_CHOICE)) {
            lydict_remove(ctx, uses->refine[i].mod.dflt);
        } else if (uses->refine[i].target_type & LYS_CONTAINER) {
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
ly_mnode_free(struct lys_node *node)
{
    struct ly_ctx *ctx;
    struct lys_node *sub, *next;

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
    case LYS_CONTAINER:
        ly_container_free(ctx, (struct lys_node_container *)node);
        break;
    case LYS_CHOICE:
        ly_when_free(ctx, ((struct lys_node_choice *)node)->when);
        break;
    case LYS_LEAF:
        ly_leaf_free(ctx, (struct lys_node_leaf *)node);
        break;
    case LYS_LEAFLIST:
        ly_leaflist_free(ctx, (struct lys_node_leaflist *)node);
        break;
    case LYS_LIST:
        ly_list_free(ctx, (struct lys_node_list *)node);
        break;
    case LYS_ANYXML:
        ly_anyxml_free(ctx, (struct lys_node_anyxml *)node);
        break;
    case LYS_USES:
        ly_uses_free(ctx, (struct lys_node_uses *)node);
        break;
    case LYS_CASE:
        ly_when_free(ctx, ((struct lys_node_case *)node)->when);
        break;
    case LYS_AUGMENT:
        /* do nothing */
        break;
    case LYS_GROUPING:
    case LYS_RPC:
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:
        ly_grp_free(ctx, (struct lys_node_grp *)node);
        break;
    }

    /* again common part */
    ly_mnode_unlink(node);
    free(node);
}

static void
module_free_common(struct lys_module *module)
{
    struct ly_ctx *ctx;
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
                lys_free(module->imp[i].module);
                break;
            }
        }
    }
    free(module->imp);

    while (module->data) {
        ly_mnode_free(module->data);
    }
    while (module->rpc) {
        ly_mnode_free(module->rpc);
    }
    while (module->notif) {
        ly_mnode_free(module->notif);
    }

    lydict_remove(ctx, module->dsc);
    lydict_remove(ctx, module->ref);
    lydict_remove(ctx, module->org);
    lydict_remove(ctx, module->contact);

    /* revisions */
    for (i = 0; i < module->rev_size; i++) {
        lydict_remove(ctx, module->rev[i].dsc);
        lydict_remove(ctx, module->rev[i].ref);
    }
    free(module->rev);

    /* identities */
    for (i = 0; i < module->ident_size; i++) {
        ly_ident_free(ctx, &module->ident[i]);
    }
    module->ident_size = 0;
    free(module->ident);

    /* typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        ly_tpdf_free(ctx, &module->tpdf[i]);
    }
    free(module->tpdf);

    /* include */
    for (i = 0; i < module->inc_size; i++) {
        ly_submodule_free(module->inc[i].submodule);
    }
    free(module->inc);

    /* augment */
    for (i = 0; i < module->augment_size; i++) {
        ly_augment_free(ctx, &module->augment[i]);
    }
    free(module->augment);

    /* features */
    for (i = 0; i < module->features_size; i++) {
        ly_feature_free(ctx, &module->features[i]);
    }
    free(module->features);

    /* deviations */
    for (i = 0; i < module->deviation_size; i++) {
        ly_deviation_free(ctx, &module->deviation[i]);
    }
    free(module->deviation);

    lydict_remove(ctx, module->name);
}

void
ly_submodule_free(struct lys_submodule *submodule)
{
    if (!submodule) {
        return;
    }

    submodule->inc_size = 0;
    free(submodule->inc);
    submodule->inc = NULL;

    /* common part with struct ly_module */
    module_free_common((struct lys_module *)submodule);

    /* no specific items to free */

    free(submodule);
}

static struct lys_node_leaf *
ly_uniq_find(struct lys_node_list *list, struct lys_node_leaf *orig_leaf)
{
    struct lys_node *mnode, *mnode2, *ret = NULL, *parent1, *parent2;
    int depth = 1, i;

    /* find the correct direct descendant of list in orig_leaf */
    mnode = (struct lys_node *)orig_leaf;
    while (1) {
        if (!mnode->parent) {
            return NULL;
        }
        if (!strcmp(mnode->parent->name, list->name)) {
            break;
        }

        mnode = mnode->parent;
        ++depth;
    }

    /* make sure the nodes are equal */
    parent1 = mnode->parent->parent;
    parent2 = list->parent;
    while (1) {
        if ((parent1 && !parent2) || (!parent1 && parent2)) {
            return NULL;
        }

        if (parent1 == parent2) {
            break;
        }

        parent1 = parent1->parent;
        parent2 = parent2->parent;
    }

    /* find the descendant in the list */
    LY_TREE_FOR(list->child, mnode2) {
        if (!strcmp(mnode2->name, mnode->name)) {
            ret = mnode2;
            break;
        }
    }

    if (!ret) {
        return NULL;
    }

    /* continue traversing both trees, the nodes are always truly equal */
    while (1) {
        --depth;
        if (!depth) {
            if (ret->nodetype != LYS_LEAF) {
                return NULL;
            }
            return (struct lys_node_leaf *)ret;
        }
        mnode = (struct lys_node *)orig_leaf;
        for (i = 0; i < depth-1; ++i) {
            mnode = mnode->parent;
        }
        LY_TREE_FOR(ret->child, mnode2) {
            if (!strcmp(mnode2->name, mnode->name)) {
                ret = mnode2;
                break;
            }
        }
        if (!mnode2) {
            return NULL;
        }
    }
}

struct lys_node *
ly_mnode_dup(struct lys_module *module, struct lys_node *mnode, uint8_t flags, int recursive,
             struct unres_schema *unres)
{
    struct lys_node *retval = NULL, *aux, *child;
    struct ly_ctx *ctx = module->ctx;
    int i, j;

    struct lys_node_container *cont;
    struct lys_node_container *cont_orig = (struct lys_node_container *)mnode;
    struct lys_node_choice *choice;
    struct lys_node_choice *choice_orig = (struct lys_node_choice *)mnode;
    struct lys_node_leaf *leaf;
    struct lys_node_leaf *leaf_orig = (struct lys_node_leaf *)mnode;
    struct lys_node_leaflist *llist;
    struct lys_node_leaflist *llist_orig = (struct lys_node_leaflist *)mnode;
    struct lys_node_list *list;
    struct lys_node_list *list_orig = (struct lys_node_list *)mnode;
    struct lys_node_anyxml *anyxml;
    struct lys_node_anyxml *anyxml_orig = (struct lys_node_anyxml *)mnode;
    struct lys_node_uses *uses;
    struct lys_node_uses *uses_orig = (struct lys_node_uses *)mnode;
    struct lys_node_grp *mix;
    struct lys_node_grp *mix_orig = (struct lys_node_grp *)mnode;
    struct lys_node_case *cs;
    struct lys_node_case *cs_orig = (struct lys_node_case *)mnode;

    /* we cannot just duplicate memory since the strings are stored in
     * dictionary and we need to update dictionary counters.
     */

    switch (mnode->nodetype) {
    case LYS_CONTAINER:
        cont = calloc(1, sizeof *cont);
        retval = (struct lys_node *)cont;
        break;

    case LYS_CHOICE:
        choice = calloc(1, sizeof *choice);
        retval = (struct lys_node *)choice;
        break;

    case LYS_LEAF:
        leaf = calloc(1, sizeof *leaf);
        retval = (struct lys_node *)leaf;
        break;

    case LYS_LEAFLIST:
        llist = calloc(1, sizeof *llist);
        retval = (struct lys_node *)llist;
        break;

    case LYS_LIST:
        list = calloc(1, sizeof *list);
        retval = (struct lys_node *)list;
        break;

    case LYS_ANYXML:
        anyxml = calloc(1, sizeof *anyxml);
        retval = (struct lys_node *)anyxml;
        break;

    case LYS_USES:
        uses = calloc(1, sizeof *uses);
        retval = (struct lys_node *)uses;
        break;

    case LYS_CASE:
        cs = calloc(1, sizeof *cs);
        retval = (struct lys_node *)cs;
        break;

        /* exact same structure */
    case LYS_GROUPING:
    case LYS_RPC:
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:
        mix = calloc(1, sizeof *mix);
        retval = (struct lys_node *)mix;
        break;

    default:
        LOGINT;
        return NULL;
    }

    /*
     * duplicate generic part of the structure
     */
    retval->name = lydict_insert(ctx, mnode->name, 0);
    retval->dsc = lydict_insert(ctx, mnode->dsc, 0);
    retval->ref = lydict_insert(ctx, mnode->ref, 0);
    retval->flags = mnode->flags;
    if (!(retval->flags & LYS_CONFIG_MASK)) {
        /* set parent's config flag */
        retval->flags |= flags & LYS_CONFIG_MASK;
    }

    retval->module = module;
    retval->nodetype = mnode->nodetype;

    retval->prev = retval;

    retval->features_size = mnode->features_size;
    retval->features = calloc(retval->features_size, sizeof *retval->features);
    for (i = 0; i < mnode->features_size; ++i) {
        if (dup_unres(module, unres, &mnode->features[i], UNRES_IFFEAT, &retval->features[i])) {
            retval->features[i] = mnode->features[i];
        }
    }

    if (recursive) {
        /* go recursively */
        LY_TREE_FOR(mnode->child, child) {
            aux = ly_mnode_dup(module, child, retval->flags, 1, unres);
            if (!aux || ly_mnode_addchild(retval, aux)) {
                LOGINT;
                ly_mnode_free(retval);
                return NULL;
            }
        }
    }

    /*
     * duplicate specific part of the structure
     */
    switch (mnode->nodetype) {
    case LYS_CONTAINER:
        if (cont_orig->when) {
            cont->when = ly_when_dup(ctx, cont_orig->when);
            add_unres_mnode(module, unres, cont->when, UNRES_WHEN, retval, 0);
        }
        cont->presence = lydict_insert(ctx, cont_orig->presence, 0);

        cont->must_size = cont_orig->must_size;
        cont->tpdf_size = cont_orig->tpdf_size;

        cont->must = ly_restr_dup(ctx, cont_orig->must, cont->must_size);
        for (i = 0; i < cont->must_size; ++i) {
            add_unres_mnode(module, unres, &cont->must[i], UNRES_MUST, retval, 0);
        }

        cont->tpdf = ly_tpdf_dup(module, mnode->parent, cont_orig->tpdf, cont->tpdf_size, unres);
        break;

    case LYS_CHOICE:
        if (choice_orig->when) {
            choice->when = ly_when_dup(ctx, choice_orig->when);
            add_unres_mnode(module, unres, choice->when, UNRES_WHEN, retval, 0);
        }

        if (choice_orig->dflt) {
            choice->dflt = resolve_child((struct lys_node *)choice, choice_orig->dflt->name, 0, LYS_ANYXML
                                         | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST
                                         | LYS_LIST);
            assert(choice->dflt);
        } else {
            dup_unres(module, unres, choice_orig, UNRES_CHOICE_DFLT, choice);
        }
        break;

    case LYS_LEAF:
        ly_type_dup(module, mnode->parent, &(leaf->type), &(leaf_orig->type), unres);
        leaf->units = lydict_insert(module->ctx, leaf_orig->units, 0);

        if (leaf_orig->dflt) {
            leaf->dflt = lydict_insert(ctx, leaf_orig->dflt, 0);
            add_unres_str(module, unres, &leaf->type, UNRES_TYPE_DFLT, leaf->dflt, 0);
        }

        leaf->must_size = leaf_orig->must_size;
        leaf->must = ly_restr_dup(ctx, leaf_orig->must, leaf->must_size);
        for (i = 0; i < leaf->must_size; ++i) {
            add_unres_mnode(module, unres, &leaf->must[i], UNRES_MUST, retval, 0);
        }

        if (leaf_orig->when) {
            leaf->when = ly_when_dup(ctx, leaf_orig->when);
            add_unres_mnode(module, unres, leaf->when, UNRES_WHEN, retval, 0);
        }
        break;

    case LYS_LEAFLIST:
        ly_type_dup(module, mnode->parent, &(llist->type), &(llist_orig->type), unres);
        llist->units = lydict_insert(module->ctx, llist_orig->units, 0);

        llist->min = llist_orig->min;
        llist->max = llist_orig->max;

        llist->must_size = llist_orig->must_size;
        llist->must = ly_restr_dup(ctx, llist_orig->must, llist->must_size);
        for (i = 0; i < llist->must_size; ++i) {
            add_unres_mnode(module, unres, &llist->must[i], UNRES_MUST, retval, 0);
        }

        if (llist_orig->when) {
            llist->when = ly_when_dup(ctx, llist_orig->when);
            add_unres_mnode(module, unres, llist->when, UNRES_WHEN, retval, 0);
        }
        break;

    case LYS_LIST:
        list->min = list_orig->min;
        list->max = list_orig->max;

        list->must_size = list_orig->must_size;
        list->tpdf_size = list_orig->tpdf_size;
        list->keys_size = list_orig->keys_size;
        list->unique_size = list_orig->unique_size;

        list->must = ly_restr_dup(ctx, list_orig->must, list->must_size);
        for (i = 0; i < list->must_size; ++i) {
            add_unres_mnode(module, unres, &list->must[i], UNRES_MUST, retval, 0);
        }

        list->tpdf = ly_tpdf_dup(module, mnode->parent, list_orig->tpdf, list->tpdf_size, unres);

        if (list->keys_size) {
            list->keys = calloc(list->keys_size, sizeof *list->keys);

            /* we managed to resolve it before, resolve it again manually */
            if (list_orig->keys[0]) {
                for (i = 0; i < list->keys_size; ++i) {
                    list->keys[i] = (struct lys_node_leaf *)resolve_child((struct lys_node *)list,
                                                                          list_orig->keys[i]->name, 0, LYS_LEAF);
                    assert(list->keys[i]);
                }
            /* it was not resolved yet, add unres copy */
            } else {
                if (dup_unres(module, unres, list_orig, UNRES_LIST_KEYS, list)) {
                    LOGINT;
                    ly_mnode_free(retval);
                    return NULL;
                }
            }
        }

        list->unique = calloc(list->unique_size, sizeof *list->unique);
        if (list_orig->unique) {
            for (i = 0; i < list->unique_size; ++i) {
                list->unique[i].leafs = calloc(list->unique[i].leafs_size, sizeof *list->unique[i].leafs);
                for (j = 0; j < list->unique[i].leafs_size; j++) {
                    list->unique[i].leafs[j] = ly_uniq_find(list, list_orig->unique[i].leafs[j]);
                }
            }
        } else {
            for (i = 0; i < list->unique_size; ++i) {
                /* HACK for unres */
                list->unique[i].leafs = (struct lys_node_leaf **)list;
                dup_unres(module, unres, &list_orig->unique[i], UNRES_LIST_UNIQ, &list->unique[i]);
            }
        }

        if (list_orig->when) {
            list->when = ly_when_dup(ctx, list_orig->when);
            add_unres_mnode(module, unres, list->when, UNRES_WHEN, retval, 0);
        }
        break;

    case LYS_ANYXML:
        anyxml->must_size = anyxml_orig->must_size;
        anyxml->must = ly_restr_dup(ctx, anyxml_orig->must, anyxml->must_size);
        for (i = 0; i < anyxml->must_size; ++i) {
            add_unres_mnode(module, unres, &anyxml->must[i], UNRES_MUST, retval, 0);
        }

        if (anyxml_orig->when) {
            anyxml->when = ly_when_dup(ctx, anyxml_orig->when);
            add_unres_mnode(module, unres, anyxml->when, UNRES_WHEN, retval, 0);
        }
        break;

    case LYS_USES:
        uses->grp = uses_orig->grp;

        if (uses_orig->when) {
            uses->when = ly_when_dup(ctx, uses_orig->when);
            add_unres_mnode(module, unres, uses->when, UNRES_WHEN, (struct lys_node *)uses, 0);
        }

        uses->refine_size = uses_orig->refine_size;
        uses->refine = ly_refine_dup(module, uses_orig->refine, uses_orig->refine_size, uses, unres);
        uses->augment_size = uses_orig->augment_size;
        uses->augment = ly_augment_dup(module, (struct lys_node *)uses, uses_orig->augment, uses_orig->augment_size, unres);
        add_unres_mnode(module, unres, uses, UNRES_USES, NULL, 0);
        break;

    case LYS_CASE:
        if (cs_orig->when) {
            cs->when = ly_when_dup(ctx, cs_orig->when);
            add_unres_mnode(module, unres, cs->when, UNRES_WHEN, retval, 0);
        }
        break;

    case LYS_GROUPING:
    case LYS_RPC:
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:
        mix->tpdf_size = mix_orig->tpdf_size;
        mix->tpdf = ly_tpdf_dup(module, mnode->parent, mix_orig->tpdf, mix->tpdf_size, unres);
        break;

    default:
        /* LY_NODE_AUGMENT */
        LOGINT;
        ly_mnode_free(retval);
        return NULL;
    }

    return retval;
}

API void
lys_free(struct lys_module *module)
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

/*
 * op: 1 - enable, 0 - disable
 */
static int
ly_features_change(struct lys_module *module, const char *name, int op)
{
    int all = 0;
    int i, j, k;

    if (!module || !name || !strlen(name)) {
        return EXIT_FAILURE;
    }

    if (!strcmp(name, "*")) {
        /* enable all */
        all = 1;
    }

    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        if (all || !strcmp(module->features[i].name, name)) {
            if (op) {
                module->features[i].flags |= LYS_FENABLED;
                /* enable referenced features (recursion) */
                for (k = 0; k < module->features[i].features_size; k++) {
                    ly_features_change(module->features[i].features[k]->module,
                                       module->features[i].features[k]->name, op);
                }
            } else {
                module->features[i].flags &= ~LYS_FENABLED;
            }
            if (!all) {
                return EXIT_SUCCESS;
            }
        }
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            if (all || !strcmp(module->inc[j].submodule->features[i].name, name)) {
                if (op) {
                    module->inc[j].submodule->features[i].flags |= LYS_FENABLED;
                } else {
                    module->inc[j].submodule->features[i].flags &= ~LYS_FENABLED;
                }
                if (!all) {
                    return EXIT_SUCCESS;
                }
            }
        }
    }

    /* TODO submodules of submodules ... */

    if (all) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

API int
lys_features_enable(struct lys_module *module, const char *feature)
{
    return ly_features_change(module, feature, 1);
}

API int
lys_features_disable(struct lys_module *module, const char *feature)
{
    return ly_features_change(module, feature, 0);
}

API int
lys_features_state(struct lys_module *module, const char *feature)
{
    int i, j;

    if (!module || !feature) {
        return -1;
    }

    /* search for the specified feature */
    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        if (!strcmp(feature, module->features[i].name)) {
            if (module->features[i].flags & LYS_FENABLED) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            if (!strcmp(feature, module->inc[j].submodule->features[i].name)) {
                if (module->inc[j].submodule->features[i].flags & LYS_FENABLED) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }

    /* TODO submodules of submodules ... */

    /* feature definition not found */
    return -1;
}

API const char **
lys_features_list(struct lys_module *module, uint8_t **states)
{
    const char **result = NULL;
    int i, j;
    unsigned int count;

    if (!module) {
        return NULL;
    }

    count = module->features_size;
    for (i = 0; i < module->inc_size; i++) {
        count += module->inc[i].submodule->features_size;
    }
    result = malloc((count + 1) * sizeof *result);
    if (states) {
        *states = malloc((count + 1) * sizeof **states);
    }
    count = 0;

    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        result[count] = module->features[i].name;
        if (states) {
            if (module->features[i].flags & LYS_FENABLED) {
                (*states)[count] = 1;
            } else {
                (*states)[count] = 0;
            }
        }
        count++;
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            result[count] = module->inc[j].submodule->features[i].name;
            if (module->inc[j].submodule->features[i].flags & LYS_FENABLED) {
                (*states)[count] = 1;
            } else {
                (*states)[count] = 0;
            }
            count++;
        }
    }

    /* TODO submodules of submodules ... */

    /* terminating NULL byte */
    result[count] = NULL;

    return result;
}

API struct lyd_node *
lyd_parse(struct ly_ctx *ctx, const char *data, LYD_FORMAT format)
{
    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LYD_XML:
        return xml_read_data(ctx, data);
    case LYD_JSON:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
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
        switch(((struct lyd_node_leaf *)node)->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_leaf *)node)->value.string);
            break;
        case LY_TYPE_BITS:
            if (((struct lyd_node_leaf *)node)->value.bit) {
                free(((struct lyd_node_leaf *)node)->value.bit);
            }
            break;
        default:
            /* TODO nothing needed : LY_TYPE_BOOL, LY_TYPE_DEC64*/
            break;
        }
    }

    if (node->prev->next) {
        node->prev->next = node->next;
    } else if (node->parent) {
        /* first node */
        node->parent->child = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }

    free(node);
}

API int
lyd_is_last(struct lyd_node *node)
{
    struct lyd_node *n;

    if (!node->next) {
        return 1;
    }

    for (n = node->next; n; n = n->next) {
        switch (n->schema->nodetype) {
        case LYS_LIST:
            if (!((struct lyd_node_list *)n)->lprev) {
                return 0;
            }
            break;
        case LYS_LEAFLIST:
            if (!((struct lyd_node_leaflist *)n)->lprev) {
                return 0;
            }
            break;
        default:
            return 0;
        }
    }
    return 1;
}
