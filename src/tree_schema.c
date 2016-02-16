
/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang schema data structures
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "context.h"
#include "parser.h"
#include "resolve.h"
#include "xml.h"
#include "xml_internal.h"
#include "tree_internal.h"
#include "validation.h"

API const struct lys_feature *
lys_is_disabled(const struct lys_node *node, int recursive)
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
    } else if (node->parent) {
        node = node->parent;
    } else {
        return NULL;
    }

    if (recursive == 2) {
        /* continue only if the node cannot have a data instance */
        if (node->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST)) {
            return NULL;
        }
    }
    goto check;
}

int
lys_get_sibling(const struct lys_node *siblings, const char *mod_name, int mod_name_len, const char *name,
                int nam_len, LYS_NODE type, const struct lys_node **ret)
{
    const struct lys_node *node, *parent = NULL;
    const struct lys_module *mod = NULL;
    const char *node_mod_name;

    assert(siblings && mod_name && name);
    assert(!(type & (LYS_USES | LYS_GROUPING)));

    /* fill the lengths in case the caller is so indifferent */
    if (!mod_name_len) {
        mod_name_len = strlen(mod_name);
    }
    if (!nam_len) {
        nam_len = strlen(name);
    }

    /* set mod correctly */
    parent = lys_parent(siblings);
    if (!parent) {
        mod = lys_node_module(siblings);
    }

    /* try to find the node */
    node = NULL;
    while ((node = lys_getnext(node, parent, mod, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE))) {
        if (!type || (node->nodetype & type)) {
            /* module name comparison */
            node_mod_name = lys_node_module(node)->name;
            if ((node_mod_name != mod_name) && (strncmp(node_mod_name, mod_name, mod_name_len) || node_mod_name[mod_name_len])) {
                continue;
            }

            /* direct name check */
            if ((node->name == name) || (!strncmp(node->name, name, nam_len) && !node->name[nam_len])) {
                if (ret) {
                    *ret = node;
                }
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_FAILURE;
}

int
lys_get_data_sibling(const struct lys_module *mod, const struct lys_node *siblings, const char *name, LYS_NODE type,
                     const struct lys_node **ret)
{
    const struct lys_node *node;

    assert(siblings && name);
    assert(!(type & (LYS_AUGMENT | LYS_USES | LYS_GROUPING | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)));

    /* find the beginning */
    while (siblings->prev->next) {
        siblings = siblings->prev;
    }

    if (!mod) {
        mod = siblings->module;
    }

    /* try to find the node */
    node = NULL;
    while ((node = lys_getnext(node, siblings->parent, mod, 0))) {
        if (!type || (node->nodetype & type)) {
            /* module check */
            if (lys_node_module(node) != lys_module(mod)) {
                continue;
            }

            /* direct name check */
            if ((node->name == name) || !strcmp(node->name, name)) {
                if (ret) {
                    *ret = node;
                }
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_FAILURE;
}

API const struct lys_node *
lys_getnext(const struct lys_node *last, const struct lys_node *parent, const struct lys_module *module, int options)
{
    const struct lys_node *next;

    if (!last) {
        /* first call */

        /* get know where to start */
        if (parent) {
            /* schema subtree */
            next = last = parent->child;
        } else {
            /* top level data */
            assert(module);
            next = last = module->data;
        }
    } else {
        /* continue after the last returned value */
        next = last->next;
    }

repeat:
    while (next && (next->nodetype == LYS_GROUPING)) {
        if (options & LYS_GETNEXT_WITHGROUPING) {
            return next;
        }
        next = next->next;
    }

    if (!next) {
        if (!last || lys_parent(last) == parent) {
            /* no next element */
            return NULL;
        }
        last = lys_parent(last);
        next = last->next;
        goto repeat;
    }

    switch (next->nodetype) {
    case LYS_INPUT:
    case LYS_OUTPUT:
        if (options & LYS_GETNEXT_WITHINOUT) {
            return next;
        } else {
            next = next->child;
            goto repeat;
        }
        break;

    case LYS_CASE:
        if (options & LYS_GETNEXT_WITHCASE) {
            return next;
        } else {
            next = next->child;
            goto repeat;
        }
        break;

    case LYS_USES:
        /* go into */
        next = next->child;
        goto repeat;

    case LYS_RPC:
    case LYS_NOTIF:
    case LYS_CONTAINER:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_LIST:
    case LYS_LEAFLIST:
        return next;

    case LYS_CHOICE:
        if (options & LYS_GETNEXT_WITHCHOICE) {
            return next;
        } else {
            /* go into */
            next = next->child;
            goto repeat;
        }
        break;

    default:
        /* we should not be here */
        return NULL;
    }


}

static const struct lys_node *
check_mand_getnext(const struct lys_node *last, const struct lys_node *parent, const struct lys_module *module)
{
    const struct lys_node *next;

    next = lys_getnext(last, parent, module, LYS_GETNEXT_WITHCHOICE);

repeat:
    if (next && next->nodetype == LYS_CONTAINER) {
        if (((struct lys_node_container *)next)->presence) {
            /* mandatory elements under the non-existing presence
             * container are not mandatory - 7.6.5, rule 1 */
            next = next->next;
        } else {
            /* go into */
            next = next->child;
        }
        goto repeat;
    }

    return next;
}

static const struct lys_node *
check_mand_check(const struct lys_node *node, const struct lys_node *stop, const struct lyd_node *data)
{
    struct lys_node *siter = NULL, *parent = NULL;
    struct lyd_node *diter = NULL;
    struct ly_set *set = NULL;
    unsigned int i;
    uint32_t minmax;

    if (node->flags & LYS_MAND_TRUE) {
        if (!data) {
            /* we have no data but a mandatory node */
            return node;
        }
        switch (node->nodetype) {
        case LYS_LEAF:
        case LYS_ANYXML:
        case LYS_CHOICE:
            if (node->parent && node->parent->nodetype == LYS_CASE) {
                /* 7.6.5, rule 2 */
                /* 7.9.4, rule 1 */
                if (node->parent->parent->parent == data->schema) {
                    /* the only case the node's siblings can exist is that the
                     * data node passed originally to ly_check_mandatory()
                     * had this choice as a child
                     */
                    /* try to find the node's siblings in data */
                    LY_TREE_FOR(data->child, diter) {
                        LY_TREE_FOR(node->parent->child, siter) {
                            if (siter == diter->schema) {
                                /* some sibling exists, rule applies */
                                break;
                            }
                        }
                        if (siter) {
                            break;
                        }
                    }
                }
                if (!siter) {
                    /* no sibling exists */
                    return NULL;
                }
            } else {
                for (parent = node->parent; parent && parent != stop; parent = parent->parent) {
                    if (parent->nodetype != LYS_CONTAINER) {
                        /* 7.6.5, rule 1, checking presence is not needed
                         * since it is done in check_mand_getnext()
                         */
                        ly_set_free(set);
                        return NULL;
                    }
                    /* add the parent to the list for searching in data tree */
                    if (!set) {
                        set = ly_set_new();
                    }
                    /* ignore return - memory error is logged and we will
                     * check at least the rest of nodes we have */
                    (void)ly_set_add(set, parent);
                }
            }

            /* search for instance */
            if (set) {
                for (i = 0; i < set->number; i++) {
                    LY_TREE_FOR(data->child, diter) {
                        if (diter->schema == set->sset[i]) {
                            break;
                        }
                    }
                    if (!diter) {
                        /* instance not found */
                        node = set->sset[i];
                        ly_set_free(set);
                        return node;
                    }
                    data = diter;
                }
                ly_set_free(set);
            }

            LY_TREE_FOR(data->child, diter) {
                if (diter->schema == node) {
                    return NULL;
                }
            }

            /* instance not found */
            /* 7.6.5, rule 3 (or 2) */
            /* 7.9.4, rule 2 */
            return node;
        default:
            /* error */
            break;
        }
    } else if (node->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* search for number of instances */
        minmax = 0;
        if (data) {
            LY_TREE_FOR(data->child, diter) {
                if (diter->schema == node) {
                    minmax++;
                }
            }
        }

        /* check the specified constraints */
        if (node->nodetype == LYS_LIST) {
            if (((struct lys_node_list *)node)->min && minmax < ((struct lys_node_list *)node)->min) {
                return node;
            }

            if (((struct lys_node_list *)node)->max && minmax > ((struct lys_node_list *)node)->max) {
                return node;
            }
        } else if (node->nodetype == LYS_LEAFLIST) {
            if (((struct lys_node_leaflist *)node)->min && minmax < ((struct lys_node_leaflist *)node)->min) {
                return node;
            }

            if (((struct lys_node_leaflist *)node)->max && minmax > ((struct lys_node_leaflist *)node)->max) {
                return node;
            }
        }
    }

    return NULL;
}

const struct lys_node *
ly_check_mandatory(const struct lyd_node *data, const struct lys_node *schema)
{
    const struct lys_node *siter, *saux, *saux2, *result, *parent = NULL, *parent2;
    const struct lyd_node *diter;
    int found;

    assert(data || schema);

    if (!data) { /* !data && schema */
        siter = schema;
    } else { /* data && !schema */
        schema = data->schema;
        siter = data->schema->child;
    }

repeat:
    while (siter) {
        if (lys_is_disabled(siter, 2)) {
            siter = siter->next;
            continue;
        }

        switch (siter->nodetype) {
        case LYS_CONTAINER:
        case LYS_LEAF:
        case LYS_ANYXML:
        case LYS_LIST:
        case LYS_LEAFLIST:
            /* check if there is some mandatory node; first test the siter itself ... */
            result = check_mand_check(siter, siter->parent, data);
            if (result) {
                return result;
            }
            /* ... and then the subtree */
            if (parent && siter->nodetype == LYS_CONTAINER && !((struct lys_node_container *)siter)->presence) {
                saux = NULL;
                while ((saux = check_mand_getnext(saux, siter, NULL))) {
                    result = check_mand_check(saux, siter, data);
                    if (result) {
                        return result;
                    }
                }
            }
            siter = siter->next;
            break;
        case LYS_CHOICE:
            /* search for instance */
            saux = siter;
            siter = siter->child;
            found = 0;
            parent2 = NULL;
repeat_choice:
            while (siter && data) {
                if (lys_is_disabled(siter, 2)) {
                    siter = siter->next;
                    continue;
                }

                switch (siter->nodetype) {
                case LYS_CONTAINER:
                case LYS_LEAF:
                case LYS_LEAFLIST:
                case LYS_LIST:
                case LYS_ANYXML:
                    LY_TREE_FOR(data->child, diter) {
                        if (diter->schema == siter) {
                            break;
                        }
                    }
                    if (diter) {
                        /* got instance */
                        /* check presence of mandatory siblings */
                        if (parent2 && parent2->nodetype == LYS_CASE) {
                            saux2 = NULL;
                            while ((saux2 = check_mand_getnext(saux2, parent2, NULL))) {
                                result = check_mand_check(saux2, parent2, data);
                                if (result) {
                                    return result;
                                }
                            }
                        }
                        siter = parent2 = NULL;
                        found = 1;
                        break;
                    }
                    siter = siter->next;
                    break;
                case LYS_CASE:
                case LYS_CHOICE:
                case LYS_USES:
                    /* go into */
                    if (!parent2) {
                        parent2 = siter;
                    }
                    siter = siter->child;
                    break;
                case LYS_AUGMENT:
                case LYS_GROUPING:
                    /* skip */
                    siter = siter->next;
                    break;
                default:
                    /* error */
                    break;
                }
            }

            if (parent2) {
                siter = parent2->next;
                if (parent2->parent == saux) {
                    parent2 = NULL;
                } else {
                    parent2 = parent2->parent;
                }
                goto repeat_choice;
            }

            if (!found && (saux->flags & LYS_MAND_TRUE)) {
                return saux;
            }

            /* go to next */
            siter = saux->next;

            break;
        case LYS_USES:
        case LYS_CASE:
            /* go into */
            parent = siter;
            siter = siter->child;
            break;
        default:
            /* can ignore, go to next */
            siter = siter->next;
            break;
        }
    }

    if (parent) {
        siter = parent->next;
        if (parent->parent == schema) {
            parent = NULL;
        } else {
            parent = parent->parent;
        }
        goto repeat;
    }

    return NULL;
}

void
lys_node_unlink(struct lys_node *node)
{
    struct lys_node *parent, *first;
    struct lys_module *main_module;

    if (!node) {
        return;
    }

    /* unlink from data model if necessary */
    if (node->module) {
        /* get main module with data tree */
        main_module = lys_node_module(node);
        if (main_module->data == node) {
            main_module->data = node->next;
        }
    }

    /* store pointers to important nodes */
    parent = node->parent;
    if (parent && (parent->nodetype == LYS_AUGMENT)) {
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
            while (first->prev->next) {
                first = first->prev;
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

struct lys_node_grp *
lys_find_grouping_up(const char *name, struct lys_node *start)
{
    struct lys_node *par_iter, *iter, *stop;

    for (par_iter = start; par_iter; par_iter = par_iter->parent) {
        /* top-level augment, look into module (uses augment is handled correctly below) */
        if (par_iter->parent && !par_iter->parent->parent && (par_iter->parent->nodetype == LYS_AUGMENT)) {
            par_iter = par_iter->parent->module->data;
            if (!par_iter) {
                break;
            }
        }

        if (par_iter->parent && (par_iter->parent->nodetype & (LYS_CHOICE | LYS_CASE | LYS_AUGMENT | LYS_USES))) {
            continue;
        }

        for (iter = par_iter, stop = NULL; iter; iter = iter->prev) {
            if (!stop) {
                stop = par_iter;
            } else if (iter == stop) {
                break;
            }
            if (iter->nodetype != LYS_GROUPING) {
                continue;
            }

            if (!strcmp(name, iter->name)) {
                return (struct lys_node_grp *)iter;
            }
        }
    }

    return NULL;
}

/*
 * get next grouping in the root's subtree, in the
 * first call, tha last is NULL
 */
static struct lys_node_grp *
lys_get_next_grouping(struct lys_node_grp *lastgrp, struct lys_node *root)
{
    struct lys_node *last = (struct lys_node *)lastgrp;
    struct lys_node *next;

    assert(root);

    if (!last) {
        last = root;
    }

    while (1) {
        if ((last->nodetype & (LYS_CONTAINER | LYS_CHOICE | LYS_LIST | LYS_GROUPING | LYS_INPUT | LYS_OUTPUT))) {
            next = last->child;
        } else {
            next = NULL;
        }
        if (!next) {
            if (last == root) {
                /* we are done */
                return NULL;
            }

            /* no children, go to siblings */
            next = last->next;
        }
        while (!next) {
            /* go back through parents */
            if (lys_parent(last) == root) {
                /* we are done */
                return NULL;
            }
            next = last->next;
            last = lys_parent(last);
        }

        if (next->nodetype == LYS_GROUPING) {
            return (struct lys_node_grp *)next;
        }

        last = next;
    }
}

/* logs directly */
int
lys_check_id(struct lys_node *node, struct lys_node *parent, struct lys_module *module)
{
    struct lys_node *start, *stop, *iter;
    struct lys_node_grp *grp;
    int down;

    assert(node);

    if (!parent) {
        assert(module);
    } else {
        module = parent->module;
    }

    switch (node->nodetype) {
    case LYS_GROUPING:
        /* 6.2.1, rule 6 */
        if (parent) {
            if (parent->child) {
                down = 1;
                start = parent->child;
            } else {
                down = 0;
                start = parent;
            }
        } else {
            down = 1;
            start = module->data;
        }
        /* go up */
        if (lys_find_grouping_up(node->name, start)) {
            LOGVAL(LYE_DUPID, 0, LY_VLOG_LYS, node, "grouping", node->name);
            return EXIT_FAILURE;
        }
        /* go down, because grouping can be defined after e.g. container in which is collision */
        if (down) {
            for (iter = start, stop = NULL; iter; iter = iter->prev) {
                if (!stop) {
                    stop = start;
                } else if (iter == stop) {
                    break;
                }
                if (!(iter->nodetype & (LYS_CONTAINER | LYS_CHOICE | LYS_LIST | LYS_GROUPING | LYS_INPUT | LYS_OUTPUT))) {
                    continue;
                }

                grp = NULL;
                while ((grp = lys_get_next_grouping(grp, iter))) {
                    if (node->name == grp->name) {
                        LOGVAL(LYE_DUPID, 0, LY_VLOG_LYS, node, "grouping", node->name);
                        return EXIT_FAILURE;
                    }
                }
            }
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_LIST:
    case LYS_CONTAINER:
    case LYS_CHOICE:
    case LYS_ANYXML:
        /* 6.2.1, rule 7 */
        if (parent) {
            iter = parent;
            while (iter && (iter->nodetype & (LYS_USES | LYS_CASE | LYS_CHOICE))) {
                iter = iter->parent;
            }
            if (!iter) {
                stop = NULL;
                iter = module->data;
            } else {
                stop = iter;
                iter = iter->child;
            }
        } else {
            stop = NULL;
            iter = module->data;
        }
        while (iter) {
            if (iter->nodetype & (LYS_USES | LYS_CASE)) {
                iter = iter->child;
                continue;
            }

            if (iter->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CONTAINER | LYS_CHOICE | LYS_ANYXML)) {
                if (iter->module == node->module && iter->name == node->name) {
                    LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, node, "Duplicated child identifier \"%s\" in \"%s\".", node->name,
                           stop ? stop->name : "(sub)module");
                    return EXIT_FAILURE;
                }
            }

            /* special case for choice - we must check the choice's name as
             * well as the names of nodes under the choice
             */
            if (iter->nodetype == LYS_CHOICE) {
                iter = iter->child;
                continue;
            }

            /* go to siblings */
            if (!iter->next) {
                /* no sibling, go to parent's sibling */
                do {
                    iter = iter->parent;
                    if (iter && iter->next) {
                        break;
                    }
                } while (iter != stop);

                if (iter == stop) {
                    break;
                }
            }
            iter = iter->next;
        }
        break;
    case LYS_CASE:
        /* 6.2.1, rule 8 */
        if (parent) {
            start = parent->child;
        } else {
            start = module->data;
        }

        LY_TREE_FOR(start, iter) {
            if (!(iter->nodetype & (LYS_ANYXML | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST))) {
                continue;
            }

            if (iter->module == node->module && iter->name == node->name) {
                LOGVAL(LYE_DUPID, 0, LY_VLOG_LYS, node, "case", node->name);
                return EXIT_FAILURE;
            }
        }
        break;
    default:
        /* no check needed */
        break;
    }

    return EXIT_SUCCESS;
}

/* logs directly */
int
lys_node_addchild(struct lys_node *parent, struct lys_module *module, struct lys_node *child)
{
    struct lys_node *iter;
    int type;

    assert(child);

    if (parent) {
        type = parent->nodetype;
        module = parent->module;
    } else {
        assert(module);
        type = 0;
    }

    /* checks */
    switch (type) {
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
            LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, parent, "Unexpected substatement \"%s\" in \"%s\" (%s).",
                   strnodetype(child->nodetype), strnodetype(parent->nodetype), parent->name);
            return EXIT_FAILURE;
        }

        break;
    case LYS_CHOICE:
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST))) {
            LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, parent, "Unexpected substatement \"%s\" in \"choice\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_CASE:
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES))) {
            LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, parent, "Unexpected substatement \"%s\" in \"case\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_RPC:
        if (!(child->nodetype & (LYS_INPUT | LYS_OUTPUT | LYS_GROUPING))) {
            LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, parent, "Unexpected substatement \"%s\" in \"rpc\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYXML:
        LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, parent, "The \"%s\" statement (%s) cannot have any data substatement.",
               strnodetype(parent->nodetype), parent->name);
        return EXIT_FAILURE;
    case LYS_AUGMENT:
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CASE | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF
                | LYS_LEAFLIST | LYS_LIST | LYS_USES))) {
            LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, parent, "Unexpected substatement \"%s\" in \"%s\" (%s).",
                   strnodetype(child->nodetype), strnodetype(parent->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_UNKNOWN:
        /* top level */
        if (!(child->nodetype &
                (LYS_ANYXML | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_GROUPING
                | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_RPC | LYS_NOTIF | LYS_AUGMENT))) {
            LOGVAL(LYE_SPEC, 0, LY_VLOG_LYS, parent, "Unexpected substatement \"%s\" in (sub)module \"%s\"",
                   strnodetype(child->nodetype), module->name);
            return EXIT_FAILURE;
        }

        break;
    }

    /* check identifier uniqueness */
    if (lys_check_id(child, parent, module)) {
        return EXIT_FAILURE;
    }

    if (child->parent) {
        lys_node_unlink(child);
    }

    if (!parent) {
        if (module->data) {
            module->data->prev->next = child;
            child->prev = module->data->prev;
            module->data->prev = child;
        } else {
            module->data = child;
        }
    } else {
        if (!parent->child) {
            /* the only/first child of the parent */
            parent->child = child;
            child->parent = parent;
            iter = child;
        } else {
            /* add a new child at the end of parent's child list */
            iter = parent->child->prev;
            iter->next = child;
            child->prev = iter;
        }
        while (iter->next) {
            iter = iter->next;
            iter->parent = parent;
        }
        parent->child->prev = iter;
    }

    return EXIT_SUCCESS;
}

API const struct lys_module *
lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format)
{
    struct lys_module *mod = NULL;

    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LYS_IN_YIN:
        mod = yin_read_module(ctx, data, 1);
        break;
    case LYS_IN_YANG:
    default:
        /* TODO */
        break;
    }

    return mod;
}

int
lys_submodule_parse(struct lys_module *module, const char *data, LYS_INFORMAT format, struct unres_schema *unres, struct lys_submodule **submodule)
{
    int ret;

    assert(module);
    assert(data);

    /* get the main module */
    module = lys_module(module);

    switch (format) {
    case LYS_IN_YIN:
        ret = yin_read_submodule(module, data, unres, submodule);
        break;
    case LYS_IN_YANG:
    default:
        /* TODO */
        ret = EXIT_FAILURE;
        break;
    }

    return ret;
}

API const struct lys_module *
lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format)
{
    int fd;
    const struct lys_module *ret;

    if (!ctx || !path) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        LOGERR(LY_ESYS, "Opening file \"%s\" failed (%s).", path, strerror(errno));
        return NULL;
    }

    ret = lys_parse_fd(ctx, fd, format);
    close(fd);
    return ret;
}

API const struct lys_module *
lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    const struct lys_module *module;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s).", strerror(errno));
        return NULL;
    }
    addr = mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        LOGERR(LY_EMEM, "Map file into memory failed (%s()).",__func__);
        return NULL;
    }
    module = lys_parse_mem(ctx, addr, format);
    munmap(addr, sb.st_size);

    return module;
}

int
lys_submodule_read(struct lys_module *module, int fd, LYS_INFORMAT format, struct unres_schema *unres, struct lys_submodule **submodule)
{
    int ret;
    struct stat sb;
    char *addr;

    assert(module);
    assert(fd >= 0);

    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s).", strerror(errno));
        return -1;
    }
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        LOGERR(LY_EMEM,"Map file into memory failed (%s()).",__func__);
        return -1;
    }
    ret = lys_submodule_parse(module, addr, format, unres, submodule);
    munmap(addr, sb.st_size);

    return ret;
}

static struct lys_restr *
lys_restr_dup(struct ly_ctx *ctx, struct lys_restr *old, int size)
{
    struct lys_restr *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    if (!result) {
        return NULL;
    }
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
lys_restr_free(struct ly_ctx *ctx, struct lys_restr *restr)
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

static int
lys_type_dup(struct lys_module *mod, struct lys_node *parent, struct lys_type *new, struct lys_type *old,
            struct unres_schema *unres)
{
    int i;

    new->module_name = lydict_insert(mod->ctx, old->module_name, 0);
    new->base = old->base;
    new->der = old->der;

    i = unres_schema_find(unres, old, UNRES_TYPE_DER);
    if (i != -1) {
        /* HACK (serious one) for unres */
        /* nothing else we can do but duplicate it immediately */
        new->der = (struct lys_tpdf *)lyxml_dup_elem(mod->ctx, (struct lyxml_elem *)old->der, NULL, 1);
        new->parent = (struct lys_tpdf *)parent;
        /* all these unres additions can fail even though they did not before */
        if (unres_schema_add_node(mod, unres, new, UNRES_TYPE_DER, parent, 0)) {
            return -1;
        }
        return EXIT_SUCCESS;
    }

    switch (new->base) {
    case LY_TYPE_BINARY:
        if (old->info.binary.length) {
            new->info.binary.length = lys_restr_dup(mod->ctx, old->info.binary.length, 1);
        }
        break;

    case LY_TYPE_BITS:
        new->info.bits.count = old->info.bits.count;
        if (new->info.bits.count) {
            new->info.bits.bit = calloc(new->info.bits.count, sizeof *new->info.bits.bit);
            if (!new->info.bits.bit) {
                LOGMEM;
                return -1;
            }
            for (i = 0; i < new->info.bits.count; i++) {
                new->info.bits.bit[i].name = lydict_insert(mod->ctx, old->info.bits.bit[i].name, 0);
                new->info.bits.bit[i].dsc = lydict_insert(mod->ctx, old->info.bits.bit[i].dsc, 0);
                new->info.bits.bit[i].ref = lydict_insert(mod->ctx, old->info.bits.bit[i].ref, 0);
                new->info.bits.bit[i].flags = old->info.bits.bit[i].flags;
                new->info.bits.bit[i].pos = old->info.bits.bit[i].pos;
            }
        }
        break;

    case LY_TYPE_DEC64:
        new->info.dec64.dig = old->info.dec64.dig;
        if (old->info.dec64.range) {
            new->info.dec64.range = lys_restr_dup(mod->ctx, old->info.dec64.range, 1);
        }
        break;

    case LY_TYPE_ENUM:
        new->info.enums.count = old->info.enums.count;
        if (new->info.enums.count) {
            new->info.enums.enm = calloc(new->info.enums.count, sizeof *new->info.enums.enm);
            if (!new->info.enums.enm) {
                LOGMEM;
                return -1;
            }
            for (i = 0; i < new->info.enums.count; i++) {
                new->info.enums.enm[i].name = lydict_insert(mod->ctx, old->info.enums.enm[i].name, 0);
                new->info.enums.enm[i].dsc = lydict_insert(mod->ctx, old->info.enums.enm[i].dsc, 0);
                new->info.enums.enm[i].ref = lydict_insert(mod->ctx, old->info.enums.enm[i].ref, 0);
                new->info.enums.enm[i].flags = old->info.enums.enm[i].flags;
                new->info.enums.enm[i].value = old->info.enums.enm[i].value;
            }
        }
        break;

    case LY_TYPE_IDENT:
        if (old->info.ident.ref) {
            new->info.ident.ref = old->info.ident.ref;
        } else {
            i = unres_schema_find(unres, old, UNRES_TYPE_IDENTREF);
            assert(i != -1);
            if (unres_schema_add_str(mod, unres, new, UNRES_TYPE_IDENTREF, unres->str_snode[i], 0)) {
                return -1;
            }
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
            new->info.num.range = lys_restr_dup(mod->ctx, old->info.num.range, 1);
        }
        break;

    case LY_TYPE_LEAFREF:
        new->info.lref.path = lydict_insert(mod->ctx, old->info.lref.path, 0);
        if (unres_schema_add_node(mod, unres, new, UNRES_TYPE_LEAFREF, parent, 0)) {
            return -1;
        }
        break;

    case LY_TYPE_STRING:
        if (old->info.str.length) {
            new->info.str.length = lys_restr_dup(mod->ctx, old->info.str.length, 1);
        }
        new->info.str.patterns = lys_restr_dup(mod->ctx, old->info.str.patterns, old->info.str.pat_count);
        break;

    case LY_TYPE_UNION:
        new->info.uni.count = old->info.uni.count;
        if (new->info.uni.count) {
            new->info.uni.types = calloc(new->info.uni.count, sizeof *new->info.uni.types);
            if (!new->info.uni.types) {
                LOGMEM;
                return -1;
            }
            for (i = 0; i < new->info.uni.count; i++) {
                if (lys_type_dup(mod, parent, &(new->info.uni.types[i]), &(old->info.uni.types[i]), unres)) {
                    return -1;
                }
            }
        }
        break;

    default:
        /* nothing to do for LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }

    return EXIT_SUCCESS;
}

void
lys_type_free(struct ly_ctx *ctx, struct lys_type *type)
{
    int i;

    assert(ctx);
    if (!type) {
        return;
    }

    lydict_remove(ctx, type->module_name);

    switch (type->base) {
    case LY_TYPE_BINARY:
        lys_restr_free(ctx, type->info.binary.length);
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
        lys_restr_free(ctx, type->info.dec64.range);
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
        lys_restr_free(ctx, type->info.num.range);
        free(type->info.num.range);
        break;

    case LY_TYPE_LEAFREF:
        lydict_remove(ctx, type->info.lref.path);
        break;

    case LY_TYPE_STRING:
        lys_restr_free(ctx, type->info.str.length);
        free(type->info.str.length);
        for (i = 0; i < type->info.str.pat_count; i++) {
            lys_restr_free(ctx, &type->info.str.patterns[i]);
        }
        free(type->info.str.patterns);
        break;

    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; i++) {
            lys_type_free(ctx, &type->info.uni.types[i]);
        }
        free(type->info.uni.types);
        break;

    default:
        /* nothing to do for LY_TYPE_IDENT, LY_TYPE_INST, LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }
}

static void
lys_tpdf_free(struct ly_ctx *ctx, struct lys_tpdf *tpdf)
{
    assert(ctx);
    if (!tpdf) {
        return;
    }

    lydict_remove(ctx, tpdf->name);
    lydict_remove(ctx, tpdf->dsc);
    lydict_remove(ctx, tpdf->ref);

    lys_type_free(ctx, &tpdf->type);

    lydict_remove(ctx, tpdf->units);
    lydict_remove(ctx, tpdf->dflt);
}

static struct lys_tpdf *
lys_tpdf_dup(struct lys_module *mod, struct lys_node *parent, struct lys_tpdf *old, int size, struct unres_schema *unres)
{
    struct lys_tpdf *result;
    int i, j;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    if (!result) {
        LOGMEM;
        return NULL;
    }
    for (i = 0; i < size; i++) {
        result[i].name = lydict_insert(mod->ctx, old[i].name, 0);
        result[i].dsc = lydict_insert(mod->ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(mod->ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].module = old[i].module;

        if (lys_type_dup(mod, parent, &(result[i].type), &(old[i].type), unres)) {
            for (j = 0; j <= i; ++j) {
                lys_tpdf_free(mod->ctx, &result[j]);
            }
            free(result);
            return NULL;
        }

        result[i].dflt = lydict_insert(mod->ctx, old[i].dflt, 0);
        result[i].units = lydict_insert(mod->ctx, old[i].units, 0);
    }

    return result;
}

static struct lys_when *
lys_when_dup(struct ly_ctx *ctx, struct lys_when *old)
{
    struct lys_when *new;

    if (!old) {
        return NULL;
    }

    new = calloc(1, sizeof *new);
    if (!new) {
        LOGMEM;
        return NULL;
    }
    new->cond = lydict_insert(ctx, old->cond, 0);
    new->dsc = lydict_insert(ctx, old->dsc, 0);
    new->ref = lydict_insert(ctx, old->ref, 0);

    return new;
}

void
lys_when_free(struct ly_ctx *ctx, struct lys_when *w)
{
    if (!w) {
        return;
    }

    lydict_remove(ctx, w->cond);
    lydict_remove(ctx, w->dsc);
    lydict_remove(ctx, w->ref);

    free(w);
}

static void
lys_augment_free(struct ly_ctx *ctx, struct lys_node_augment aug, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    struct lys_node *next, *sub;

    /* children from a resolved augment are freed under the target node */
    if (!aug.target) {
        LY_TREE_FOR_SAFE(aug.child, next, sub) {
            lys_node_free(sub, private_destructor);
        }
    }

    lydict_remove(ctx, aug.target_name);
    lydict_remove(ctx, aug.dsc);
    lydict_remove(ctx, aug.ref);

    free(aug.features);

    lys_when_free(ctx, aug.when);

    /* Do not free the children, they were appended somewhere and their
     * new parent will take care of them.
     */
}

static struct lys_node_augment *
lys_augment_dup(struct lys_module *module, struct lys_node *parent, struct lys_node_augment *old, int size)
{
    struct lys_node_augment *new = NULL;
    struct lys_node *old_child, *new_child;
    int i;

    if (!size) {
        return NULL;
    }

    new = calloc(size, sizeof *new);
    if (!new) {
        LOGMEM;
        return NULL;
    }
    for (i = 0; i < size; i++) {
        new[i].target_name = lydict_insert(module->ctx, old[i].target_name, 0);
        new[i].dsc = lydict_insert(module->ctx, old[i].dsc, 0);
        new[i].ref = lydict_insert(module->ctx, old[i].ref, 0);
        new[i].flags = old[i].flags;
        new[i].module = old[i].module;
        new[i].nodetype = old[i].nodetype;
        /* this must succeed, it was already resolved once */
        if (resolve_augment_schema_nodeid(new[i].target_name, parent->child, NULL,
                                          (const struct lys_node **)&new[i].target)) {
            LOGINT;
            free(new);
            return NULL;
        }
        new[i].parent = parent;

        /* Correct the augment nodes.
         * This function can only be called from lys_node_dup() with uses
         * being the node duplicated, so we must have a case of grouping
         * with a uses with augments. The augmented nodes have already been
         * copied and everything is almost fine except their parent is wrong
         * (it was set to their actual data parent, not an augment), and
         * the new augment does not have child pointer to its augment nodes,
         * so we just correct it.
         */
        LY_TREE_FOR(new[i].target->child, new_child) {
            if (new_child->name == old[i].child->name) {
                break;
            }
        }
        assert(new_child);
        new[i].child = new_child;
        LY_TREE_FOR(old[i].child, old_child) {
            /* all augment nodes were connected as siblings, there can be no more after this */
            if (old_child->parent != (struct lys_node *)&old[i]) {
                break;
            }

            assert(old_child->name == new_child->name);

            new_child->parent = (struct lys_node *)&new[i];
            new_child = new_child->next;
        }
    }

    return new;
}

static struct lys_refine *
lys_refine_dup(struct lys_module *mod, struct lys_refine *old, int size)
{
    struct lys_refine *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    if (!result) {
        LOGMEM;
        return NULL;
    }
    for (i = 0; i < size; i++) {
        result[i].target_name = lydict_insert(mod->ctx, old[i].target_name, 0);
        result[i].dsc = lydict_insert(mod->ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(mod->ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].target_type = old[i].target_type;

        result[i].must_size = old[i].must_size;
        result[i].must = lys_restr_dup(mod->ctx, old[i].must, old[i].must_size);

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

static void
lys_ident_free(struct ly_ctx *ctx, struct lys_ident *ident)
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
     * from the context. This is the current approach.
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

static void
lys_grp_free(struct ly_ctx *ctx, struct lys_node_grp *grp)
{
    int i;

    /* handle only specific parts for LYS_GROUPING */
    for (i = 0; i < grp->tpdf_size; i++) {
        lys_tpdf_free(ctx, &grp->tpdf[i]);
    }
    free(grp->tpdf);
}

static void
lys_rpc_inout_free(struct ly_ctx *ctx, struct lys_node_rpc_inout *io)
{
    int i;

    /* handle only specific parts for LYS_INPUT and LYS_OUTPUT */
    for (i = 0; i < io->tpdf_size; i++) {
        lys_tpdf_free(ctx, &io->tpdf[i]);
    }
    free(io->tpdf);
}

static void
lys_anyxml_free(struct ly_ctx *ctx, struct lys_node_anyxml *anyxml)
{
    int i;

    for (i = 0; i < anyxml->must_size; i++) {
        lys_restr_free(ctx, &anyxml->must[i]);
    }
    free(anyxml->must);

    lys_when_free(ctx, anyxml->when);
}

static void
lys_leaf_free(struct ly_ctx *ctx, struct lys_node_leaf *leaf)
{
    int i;

    if (leaf->child) {
        /* leafref backlinks */
        ly_set_free((struct ly_set *)leaf->child);
    }

    for (i = 0; i < leaf->must_size; i++) {
        lys_restr_free(ctx, &leaf->must[i]);
    }
    free(leaf->must);

    lys_when_free(ctx, leaf->when);

    lys_type_free(ctx, &leaf->type);
    lydict_remove(ctx, leaf->units);
    lydict_remove(ctx, leaf->dflt);
}

static void
lys_leaflist_free(struct ly_ctx *ctx, struct lys_node_leaflist *llist)
{
    int i;

    if (llist->child) {
        /* leafref backlinks */
        ly_set_free((struct ly_set *)llist->child);
    }

    for (i = 0; i < llist->must_size; i++) {
        lys_restr_free(ctx, &llist->must[i]);
    }
    free(llist->must);

    lys_when_free(ctx, llist->when);

    lys_type_free(ctx, &llist->type);
    lydict_remove(ctx, llist->units);
}

static void
lys_list_free(struct ly_ctx *ctx, struct lys_node_list *list)
{
    int i, j;

    /* handle only specific parts for LY_NODE_LIST */
    for (i = 0; i < list->tpdf_size; i++) {
        lys_tpdf_free(ctx, &list->tpdf[i]);
    }
    free(list->tpdf);

    for (i = 0; i < list->must_size; i++) {
        lys_restr_free(ctx, &list->must[i]);
    }
    free(list->must);

    lys_when_free(ctx, list->when);

    for (i = 0; i < list->unique_size; i++) {
        for (j = 0; j > list->unique[i].expr_size; j++) {
            lydict_remove(ctx, list->unique[i].expr[j]);
        }
        free(list->unique[i].expr);
    }
    free(list->unique);

    free(list->keys);
}

static void
lys_container_free(struct ly_ctx *ctx, struct lys_node_container *cont)
{
    int i;

    /* handle only specific parts for LY_NODE_CONTAINER */
    lydict_remove(ctx, cont->presence);

    for (i = 0; i < cont->tpdf_size; i++) {
        lys_tpdf_free(ctx, &cont->tpdf[i]);
    }
    free(cont->tpdf);

    for (i = 0; i < cont->must_size; i++) {
        lys_restr_free(ctx, &cont->must[i]);
    }
    free(cont->must);

    lys_when_free(ctx, cont->when);
}

static void
lys_feature_free(struct ly_ctx *ctx, struct lys_feature *f)
{
    lydict_remove(ctx, f->name);
    lydict_remove(ctx, f->dsc);
    lydict_remove(ctx, f->ref);
    free(f->features);
}

static void
lys_deviation_free(struct ly_ctx *ctx, struct lys_deviation *dev)
{
    int i, j, k;

    lydict_remove(ctx, dev->target_name);
    lydict_remove(ctx, dev->dsc);
    lydict_remove(ctx, dev->ref);

    for (i = 0; i < dev->deviate_size; i++) {
        lydict_remove(ctx, dev->deviate[i].dflt);
        lydict_remove(ctx, dev->deviate[i].units);

        if (dev->deviate[i].mod == LY_DEVIATE_DEL) {
            for (j = 0; j < dev->deviate[i].must_size; j++) {
                lys_restr_free(ctx, &dev->deviate[i].must[j]);
            }
            free(dev->deviate[i].must);

            for (j = 0; j < dev->deviate[i].unique_size; j++) {
                for (k = 0; k < dev->deviate[i].unique[j].expr_size; k++) {
                    lydict_remove(ctx, dev->deviate[i].unique[j].expr[k]);
                }
                free(dev->deviate[j].unique[j].expr);
            }
            free(dev->deviate[i].unique);
        }
    }
    free(dev->deviate);
}

static void
lys_uses_free(struct ly_ctx *ctx, struct lys_node_uses *uses, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i, j;

    for (i = 0; i < uses->refine_size; i++) {
        lydict_remove(ctx, uses->refine[i].target_name);
        lydict_remove(ctx, uses->refine[i].dsc);
        lydict_remove(ctx, uses->refine[i].ref);

        for (j = 0; j < uses->refine[i].must_size; j++) {
            lys_restr_free(ctx, &uses->refine[i].must[j]);
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
        lys_augment_free(ctx, uses->augment[i], private_destructor);
    }
    free(uses->augment);

    lys_when_free(ctx, uses->when);
}

void
lys_node_free(struct lys_node *node, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    struct ly_ctx *ctx;
    struct lys_node *sub, *next;

    if (!node) {
        return;
    }

    assert(node->module);
    assert(node->module->ctx);

    ctx = node->module->ctx;

    /* remove private object */
    if (node->private && private_destructor) {
        private_destructor(node, node->private);
    }

    /* common part */
    if (!(node->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
        free(node->features);
        lydict_remove(ctx, node->name);
        lydict_remove(ctx, node->dsc);
        lydict_remove(ctx, node->ref);
    }

    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LY_TREE_FOR_SAFE(node->child, next, sub) {
            lys_node_free(sub, private_destructor);
        }
    }

    /* specific part */
    switch (node->nodetype) {
    case LYS_CONTAINER:
        lys_container_free(ctx, (struct lys_node_container *)node);
        break;
    case LYS_CHOICE:
        lys_when_free(ctx, ((struct lys_node_choice *)node)->when);
        break;
    case LYS_LEAF:
        lys_leaf_free(ctx, (struct lys_node_leaf *)node);
        break;
    case LYS_LEAFLIST:
        lys_leaflist_free(ctx, (struct lys_node_leaflist *)node);
        break;
    case LYS_LIST:
        lys_list_free(ctx, (struct lys_node_list *)node);
        break;
    case LYS_ANYXML:
        lys_anyxml_free(ctx, (struct lys_node_anyxml *)node);
        break;
    case LYS_USES:
        lys_uses_free(ctx, (struct lys_node_uses *)node, private_destructor);
        break;
    case LYS_CASE:
        lys_when_free(ctx, ((struct lys_node_case *)node)->when);
        break;
    case LYS_AUGMENT:
        /* do nothing */
        break;
    case LYS_GROUPING:
    case LYS_RPC:
    case LYS_NOTIF:
        lys_grp_free(ctx, (struct lys_node_grp *)node);
        break;

    case LYS_INPUT:
    case LYS_OUTPUT:
        lys_rpc_inout_free(ctx, (struct lys_node_rpc_inout *)node);
        break;
    case LYS_UNKNOWN:
        LOGINT;
        break;
    }

    /* again common part */
    lys_node_unlink(node);
    free(node);
}

const struct lys_module *
lys_get_import_module(const struct lys_module *module, const char *prefix, int pref_len, const char *name, int name_len)
{
    const struct lys_module *main_module;
    int i;

    assert(!prefix || !name);

    if (prefix && !pref_len) {
        pref_len = strlen(prefix);
    }
    if (name && !name_len) {
        name_len = strlen(name);
    }

    main_module = lys_module(module);

    /* module own prefix, submodule own prefix, (sub)module own name */
    if ((!prefix || (!module->type && !strncmp(main_module->prefix, prefix, pref_len) && !main_module->prefix[pref_len])
                 || (module->type && !strncmp(module->prefix, prefix, pref_len) && !module->prefix[pref_len]))
            && (!name || (!strncmp(main_module->name, name, name_len) && !main_module->name[name_len]))) {
        return main_module;
    }

    for (i = 0; i < module->imp_size; ++i) {
        if ((!prefix || (!strncmp(module->imp[i].prefix, prefix, pref_len) && !module->imp[i].prefix[pref_len]))
                && (!name || (!strncmp(module->imp[i].module->name, name, name_len) && !module->imp[i].module->name[name_len]))) {
            return module->imp[i].module;
        }
    }

    return NULL;
}

/* free_int_mods - flag whether to free the internal modules as well */
static void
module_free_common(struct lys_module *module, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    struct ly_ctx *ctx;
    struct lys_node *next, *iter;
    unsigned int i;

    assert(module->ctx);
    ctx = module->ctx;

    /* just free the import array, imported modules will stay in the context */
    free(module->imp);

    /* submodules don't have data tree, the data nodes
     * are placed in the main module altogether */
    if (!module->type) {
        LY_TREE_FOR_SAFE(module->data, next, iter) {
            lys_node_free(iter, private_destructor);
        }
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
        lys_ident_free(ctx, &module->ident[i]);
    }
    module->ident_size = 0;
    free(module->ident);

    /* typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        lys_tpdf_free(ctx, &module->tpdf[i]);
    }
    free(module->tpdf);

    /* include */
    for (i = 0; i < module->inc_size; i++) {
        /* complete submodule free is done only from main module since
         * submodules propagate their includes to the main module */
        if (!module->type) {
            lys_submodule_free(module->inc[i].submodule, private_destructor);
        }
    }
    free(module->inc);

    /* augment */
    for (i = 0; i < module->augment_size; i++) {
        lys_augment_free(ctx, module->augment[i], private_destructor);
    }
    free(module->augment);

    /* features */
    for (i = 0; i < module->features_size; i++) {
        lys_feature_free(ctx, &module->features[i]);
    }
    free(module->features);

    /* deviations */
    for (i = 0; i < module->deviation_size; i++) {
        lys_deviation_free(ctx, &module->deviation[i]);
    }
    free(module->deviation);

    lydict_remove(ctx, module->name);
}

void
lys_submodule_free(struct lys_submodule *submodule, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    if (!submodule) {
        return;
    }

    /* common part with struct ly_module */
    module_free_common((struct lys_module *)submodule, private_destructor);

    /* no specific items to free */

    free(submodule);
}

struct lys_node *
lys_node_dup(struct lys_module *module, struct lys_node *parent, const struct lys_node *node, uint8_t flags,
             uint8_t nacm, struct unres_schema *unres)
{
    struct lys_node *retval = NULL, *child;
    struct ly_ctx *ctx = module->ctx;
    int i, j, rc;

    struct lys_node_container *cont = NULL;
    struct lys_node_container *cont_orig = (struct lys_node_container *)node;
    struct lys_node_choice *choice = NULL;
    struct lys_node_choice *choice_orig = (struct lys_node_choice *)node;
    struct lys_node_leaf *leaf = NULL;
    struct lys_node_leaf *leaf_orig = (struct lys_node_leaf *)node;
    struct lys_node_leaflist *llist = NULL;
    struct lys_node_leaflist *llist_orig = (struct lys_node_leaflist *)node;
    struct lys_node_list *list = NULL;
    struct lys_node_list *list_orig = (struct lys_node_list *)node;
    struct lys_node_anyxml *anyxml = NULL;
    struct lys_node_anyxml *anyxml_orig = (struct lys_node_anyxml *)node;
    struct lys_node_uses *uses = NULL;
    struct lys_node_uses *uses_orig = (struct lys_node_uses *)node;
    struct lys_node_grp *grp = NULL;
    struct lys_node_grp *grp_orig = (struct lys_node_grp *)node;
    struct lys_node_rpc *rpc = NULL;
    struct lys_node_rpc *rpc_orig = (struct lys_node_rpc *)node;
    struct lys_node_rpc_inout *io = NULL;
    struct lys_node_rpc_inout *io_orig = (struct lys_node_rpc_inout *)node;
    struct lys_node_rpc *ntf = NULL;
    struct lys_node_rpc *ntf_orig = (struct lys_node_rpc *)node;
    struct lys_node_case *cs = NULL;
    struct lys_node_case *cs_orig = (struct lys_node_case *)node;

    /* we cannot just duplicate memory since the strings are stored in
     * dictionary and we need to update dictionary counters.
     */

    switch (node->nodetype) {
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

    case LYS_GROUPING:
        grp = calloc(1, sizeof *grp);
        retval = (struct lys_node *)grp;
        break;

    case LYS_RPC:
        rpc = calloc(1, sizeof *rpc);
        retval = (struct lys_node *)rpc;
        break;

    case LYS_INPUT:
    case LYS_OUTPUT:
        io = calloc(1, sizeof *io);
        retval = (struct lys_node *)io;
        break;

    case LYS_NOTIF:
        ntf = calloc(1, sizeof *ntf);
        retval = (struct lys_node *)ntf;
        break;

    default:
        LOGINT;
        goto error;
    }

    if (!retval) {
        LOGMEM;
        return NULL;
    }

    /*
     * duplicate generic part of the structure
     */
    retval->name = lydict_insert(ctx, node->name, 0);
    retval->dsc = lydict_insert(ctx, node->dsc, 0);
    retval->ref = lydict_insert(ctx, node->ref, 0);
    retval->nacm = nacm;
    retval->flags = node->flags;
    if (!(retval->flags & LYS_CONFIG_MASK)) {
        /* set parent's config flag */
        retval->flags |= flags & LYS_CONFIG_MASK;
    }

    retval->module = module;
    retval->nodetype = node->nodetype;

    retval->prev = retval;

    retval->features_size = node->features_size;
    retval->features = calloc(retval->features_size, sizeof *retval->features);
    if (!retval->features) {
        LOGMEM;
        goto error;
    }
    for (i = 0; i < node->features_size; ++i) {
        retval->features[i] = (struct lys_feature *)retval;
        if (unres_schema_dup(module, unres, &node->features[i], UNRES_IFFEAT, &retval->features[i])) {
            retval->features[i] = node->features[i];
        }
    }

    /* connect it to the parent */
    if (lys_node_addchild(parent, retval->module, retval)) {
        goto error;
    }

    /* go recursively */
    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LY_TREE_FOR(node->child, child) {
            if (!lys_node_dup(module, retval, child, retval->flags, retval->nacm, unres)) {
                goto error;
            }
        }
    }

    /*
     * duplicate specific part of the structure
     */
    switch (node->nodetype) {
    case LYS_CONTAINER:
        if (cont_orig->when) {
            cont->when = lys_when_dup(ctx, cont_orig->when);
        }
        cont->presence = lydict_insert(ctx, cont_orig->presence, 0);

        cont->must_size = cont_orig->must_size;
        cont->tpdf_size = cont_orig->tpdf_size;

        cont->must = lys_restr_dup(ctx, cont_orig->must, cont->must_size);
        cont->tpdf = lys_tpdf_dup(module, node->parent, cont_orig->tpdf, cont->tpdf_size, unres);
        break;

    case LYS_CHOICE:
        if (choice_orig->when) {
            choice->when = lys_when_dup(ctx, choice_orig->when);
        }

        if (choice_orig->dflt) {
            rc = lys_get_sibling(choice->child, lys_node_module(retval)->name, 0, choice_orig->dflt->name, 0, LYS_ANYXML
                                         | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST
                                         | LYS_LIST, (const struct lys_node **)&choice->dflt);
            if (rc) {
                if (rc == EXIT_FAILURE) {
                    LOGINT;
                }
                goto error;
            }
        } else {
            /* useless to check return value, we don't know whether
             * there really wasn't any default defined or it just hasn't
             * been resolved, we just hope for the best :)
             */
            unres_schema_dup(module, unres, choice_orig, UNRES_CHOICE_DFLT, choice);
        }
        break;

    case LYS_LEAF:
        if (lys_type_dup(module, node->parent, &(leaf->type), &(leaf_orig->type), unres)) {
            goto error;
        }
        leaf->units = lydict_insert(module->ctx, leaf_orig->units, 0);

        if (leaf_orig->dflt) {
            leaf->dflt = lydict_insert(ctx, leaf_orig->dflt, 0);
            if (unres_schema_add_str(module, unres, &leaf->type, UNRES_TYPE_DFLT, leaf->dflt, 0) == -1) {
                goto error;
            }
        }

        leaf->must_size = leaf_orig->must_size;
        leaf->must = lys_restr_dup(ctx, leaf_orig->must, leaf->must_size);

        if (leaf_orig->when) {
            leaf->when = lys_when_dup(ctx, leaf_orig->when);
        }
        break;

    case LYS_LEAFLIST:
        if (lys_type_dup(module, node->parent, &(llist->type), &(llist_orig->type), unres)) {
            goto error;
        }
        llist->units = lydict_insert(module->ctx, llist_orig->units, 0);

        llist->min = llist_orig->min;
        llist->max = llist_orig->max;

        llist->must_size = llist_orig->must_size;
        llist->must = lys_restr_dup(ctx, llist_orig->must, llist->must_size);

        if (llist_orig->when) {
            llist->when = lys_when_dup(ctx, llist_orig->when);
        }
        break;

    case LYS_LIST:
        list->min = list_orig->min;
        list->max = list_orig->max;

        list->must_size = list_orig->must_size;
        list->must = lys_restr_dup(ctx, list_orig->must, list->must_size);

        list->tpdf_size = list_orig->tpdf_size;
        list->tpdf = lys_tpdf_dup(module, node->parent, list_orig->tpdf, list->tpdf_size, unres);

        list->keys_size = list_orig->keys_size;
        if (list->keys_size) {
            list->keys = calloc(list->keys_size, sizeof *list->keys);
            if (!list->keys) {
                LOGMEM;
                goto error;
            }

            /* we managed to resolve it before, resolve it again manually */
            if (list_orig->keys[0]) {
                for (i = 0; i < list->keys_size; ++i) {
                    rc = lys_get_sibling(list->child, lys_node_module(retval)->name, 0, list_orig->keys[i]->name, 0, LYS_LEAF,
                                         (const struct lys_node **)&list->keys[i]);
                    if (rc) {
                        if (rc == EXIT_FAILURE) {
                            LOGINT;
                        }
                        goto error;
                    }
                }
            /* it was not resolved yet, add unres copy */
            } else {
                if (unres_schema_dup(module, unres, list_orig, UNRES_LIST_KEYS, list)) {
                    LOGINT;
                    goto error;
                }
            }
        }

        list->unique_size = list_orig->unique_size;
        list->unique = malloc(list->unique_size * sizeof *list->unique);
        if (!list->unique) {
            LOGMEM;
            goto error;
        }
        for (i = 0; i < list->unique_size; ++i) {
            list->unique[i].expr_size = list_orig->unique[i].expr_size;
            list->unique[i].expr = malloc(list->unique[i].expr_size * sizeof *list->unique[i].expr);
            if (!list->unique[i].expr) {
                LOGMEM;
                goto error;
            }
            for (j = 0; j < list->unique[i].expr_size; j++) {
                list->unique[i].expr[j] = lydict_insert(ctx, list_orig->unique[i].expr[j], 0);

                /* if it stays in unres list, duplicate it also there */
                unres_schema_dup(module, unres, &list_orig->unique[i], UNRES_LIST_UNIQ, &list->unique[i]);
            }
        }

        if (list_orig->when) {
            list->when = lys_when_dup(ctx, list_orig->when);
        }
        break;

    case LYS_ANYXML:
        anyxml->must_size = anyxml_orig->must_size;
        anyxml->must = lys_restr_dup(ctx, anyxml_orig->must, anyxml->must_size);

        if (anyxml_orig->when) {
            anyxml->when = lys_when_dup(ctx, anyxml_orig->when);
        }
        break;

    case LYS_USES:
        uses->grp = uses_orig->grp;

        if (uses_orig->when) {
            uses->when = lys_when_dup(ctx, uses_orig->when);
        }

        uses->refine_size = uses_orig->refine_size;
        uses->refine = lys_refine_dup(module, uses_orig->refine, uses_orig->refine_size);
        uses->augment_size = uses_orig->augment_size;
        uses->augment = lys_augment_dup(module, (struct lys_node *)uses, uses_orig->augment, uses_orig->augment_size);
        if (!uses->child) {
            if (unres_schema_add_node(module, unres, uses, UNRES_USES, NULL, 0) == -1) {
                goto error;
            }
        }
        break;

    case LYS_CASE:
        if (cs_orig->when) {
            cs->when = lys_when_dup(ctx, cs_orig->when);
        }
        break;

    case LYS_GROUPING:
        grp->tpdf_size = grp_orig->tpdf_size;
        grp->tpdf = lys_tpdf_dup(module, node->parent, grp_orig->tpdf, grp->tpdf_size, unres);
        break;

    case LYS_RPC:
        rpc->tpdf_size = rpc_orig->tpdf_size;
        rpc->tpdf = lys_tpdf_dup(module, node->parent, rpc_orig->tpdf, rpc->tpdf_size, unres);
        break;

    case LYS_INPUT:
    case LYS_OUTPUT:
        io->tpdf_size = io_orig->tpdf_size;
        io->tpdf = lys_tpdf_dup(module, node->parent, io_orig->tpdf, io->tpdf_size, unres);
        break;

    case LYS_NOTIF:
        ntf->tpdf_size = ntf_orig->tpdf_size;
        ntf->tpdf = lys_tpdf_dup(module, node->parent, ntf_orig->tpdf, ntf->tpdf_size, unres);
        break;

    default:
        /* LY_NODE_AUGMENT */
        LOGINT;
        goto error;
    }

    return retval;

error:

    lys_node_free(retval, NULL);
    return NULL;
}

void
lys_free(struct lys_module *module, void (*private_destructor)(const struct lys_node *node, void *priv), int remove_from_ctx)
{
    struct ly_ctx *ctx;
    int i;

    if (!module) {
        return;
    }

    /* remove schema from the context */
    ctx = module->ctx;
    if (remove_from_ctx && ctx->models.used) {
        for (i = 0; i < ctx->models.used; i++) {
            if (ctx->models.list[i] == module) {
                /* move all the models to not change the order in the list */
                ctx->models.used--;
                memmove(&ctx->models.list[i], ctx->models.list[i + 1], (ctx->models.used - i) * sizeof *ctx->models.list);
                ctx->models.list[ctx->models.used] = NULL;
                /* we are done */
                break;
            }
        }
    }

    /* common part with struct ly_submodule */
    module_free_common(module, private_destructor);

    /* specific items to free */
    lydict_remove(ctx, module->ns);
    lydict_remove(ctx, module->prefix);

    free(module);
}

/*
 * op: 1 - enable, 0 - disable
 */
static int
lys_features_change(const struct lys_module *module, const char *name, int op)
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
                    lys_features_change(module->features[i].features[k]->module,
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
lys_features_enable(const struct lys_module *module, const char *feature)
{
    return lys_features_change(module, feature, 1);
}

API int
lys_features_disable(const struct lys_module *module, const char *feature)
{
    return lys_features_change(module, feature, 0);
}

API int
lys_features_state(const struct lys_module *module, const char *feature)
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

    /* feature definition not found */
    return -1;
}

API const char **
lys_features_list(const struct lys_module *module, uint8_t **states)
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
    if (!result) {
        LOGMEM;
        return NULL;
    }
    if (states) {
        *states = malloc((count + 1) * sizeof **states);
        if (!(*states)) {
            LOGMEM;
            free(result);
            return NULL;
        }
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
            if (states) {
                if (module->inc[j].submodule->features[i].flags & LYS_FENABLED) {
                    (*states)[count] = 1;
                } else {
                    (*states)[count] = 0;
                }
            }
            count++;
        }
    }

    /* terminating NULL byte */
    result[count] = NULL;

    return result;
}

struct lys_module *
lys_node_module(const struct lys_node *node)
{
    return node->module->type ? ((struct lys_submodule *)node->module)->belongsto : node->module;
}

struct lys_module *
lys_module(const struct lys_module *module)
{
    return (module->type ? ((struct lys_submodule *)module)->belongsto : (struct lys_module *)module);
}

API struct lys_node *
lys_parent(const struct lys_node *node)
{
    if (!node || !node->parent) {
        return NULL;
    }

    if (node->parent->nodetype == LYS_AUGMENT) {
        return ((struct lys_node_augment *)node->parent)->target;
    }

    return node->parent;
}

API void *
lys_set_private(const struct lys_node *node, void *priv)
{
    void *prev;

    if (!node) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    prev = node->private;
    ((struct lys_node *)node)->private = priv;

    return prev;
}
