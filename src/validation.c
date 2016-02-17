/**
 * @file validation.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data tree validation functions
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
#include <string.h>

#include "common.h"
#include "validation.h"
#include "libyang.h"
#include "xpath.h"
#include "parser.h"
#include "resolve.h"
#include "tree_internal.h"
#include "xml_internal.h"

static struct lys_node_leaf *
lyv_keys_present(const struct lyd_node *list)
{
    struct lyd_node *aux;
    struct lys_node_list *schema;
    int i;

    schema = (struct lys_node_list *)list->schema;

    for (i = 0; i < schema->keys_size; i++) {
        for (aux = list->child; aux; aux = aux->next) {
            if (aux->schema == (struct lys_node *)schema->keys[i]) {
                break;
            }
        }
        if (!aux) {
            /* key not found in the data */
            return schema->keys[i];
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Compare filter nodes
 *
 * @param[in] first The first data node to compare
 * @param[in] second The second node to compare
 * @return 0 if both filter nodes selects the same data.
 */
static int
filter_compare(const struct lyd_node *first, const struct lyd_node *second)
{
    struct lyd_node *diter1, *diter2;
    int match, c1, c2;

    assert(first);
    assert(second);

    if (first->schema != second->schema) {
        return 1;
    }


    switch (first->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
        /* check if all the content match nodes are the same */
        c1 = 0;
        LY_TREE_FOR(first->child, diter1) {
            if (!(diter1->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                continue;
            } else if (!((struct lyd_node_leaf_list *)diter1)->value_str) {
                /* selection node */
                continue;
            }

            match = 0;
            LY_TREE_FOR(second->child, diter2) {
                if (diter2->schema != diter1->schema) {
                    continue;
                } else if (!ly_strequal(((struct lyd_node_leaf_list *)diter1)->value_str,
                                        ((struct lyd_node_leaf_list *)diter2)->value_str)) {
                    continue;
                }
                match = 1;
                c1++;
            }
            if (!match) {
                return 1;
            }
        }
        /* get number of content match nodes in the second to get know if there are some
         * that are not present in first
         */
        c2 = 0;
        LY_TREE_FOR(second->child, diter2) {
            if (!(diter2->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                continue;
            } else if (!((struct lyd_node_leaf_list *)diter2)->value_str) {
                /* selection node */
                continue;
            }
            c2++;
        }
        if (c1 != c2) {
            return 1;
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        if (!ly_strequal(((struct lyd_node_leaf_list *)first)->value_str,
                         ((struct lyd_node_leaf_list *)second)->value_str)) {
            return 1;
        }
        break;
    default:
        /* no more tests are needed */
        break;
    }
    return 0;
}

static int
filter_merge(struct lyd_node *to, struct lyd_node *from)
{
    struct lyd_node *diter1, *diter2;
    unsigned int i, j;
    struct ly_set *s1 = NULL, *s2 = NULL;
    int copy;
    int ret = EXIT_FAILURE;

    if (!to || !from || to->schema != from->schema) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    switch(to->schema->nodetype) {
    case LYS_LIST:
    case LYS_CONTAINER:
        if (!from->child) {
            /* from is selection node, so we want to make the to selection node now */
            while (to->child) {
                lyd_free(to->child);
            }
        } else if (to->child) {
            /* both to and from are containment nodes and it was already checked
             * (by calling filter_compare()) that they selects the same target.
             * Therefore we can skip the content match nodes (they are the same in
             * both of them) and merge only the selection and containment nodes */

            /* first, get know if to and from contain some selection or containment
             * nodes. Because if one of them does not contain any such a node it
             * selects all the data so it does not make sense to limit it by any
             * selection/containment node.
             */
            s1 = ly_set_new();
            s2 = ly_set_new();
            if (!s1 || !s2) {
                LOGMEM;
                goto cleanup;
            }
            LY_TREE_FOR(to->child, diter1) {
                /* is selection node */
                if ((diter1->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                        && !((struct lyd_node_leaf_list *)diter1)->value_str) {
                    if (ly_set_add(s1, diter1)) {
                        goto cleanup;
                    }
                } else if ((diter1->schema->nodetype == LYS_ANYXML) && !((struct lyd_node_anyxml *)diter1)->value->child) {
                    if (ly_set_add(s1, diter1)) {
                        goto cleanup;
                    }
                } else if (diter1->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) {
                    /* or containment node */
                    if (ly_set_add(s1, diter1)) {
                        goto cleanup;
                    }
                }
            }

            LY_TREE_FOR(from->child, diter2) {
                /* is selection node */
                if ((diter2->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                        && !((struct lyd_node_leaf_list *)diter2)->value_str) {
                    if (ly_set_add(s2, diter2)) {
                        goto cleanup;
                    }
                } else if ((diter2->schema->nodetype == LYS_ANYXML) && !((struct lyd_node_anyxml *)diter2)->value->child) {
                    if (ly_set_add(s2, diter2)) {
                        goto cleanup;
                    }
                } else if (diter2->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) {
                    /* or containment node */
                    if (ly_set_add(s2, diter2)) {
                        goto cleanup;
                    }
                }
            }

            if (!s1->number) {
                /* to already selects all content, so nothing is needed */
                break;
            } else if (!s2->number) {
                /* from selects all content, so make to select it too by
                 * removing all selection and containment nodes
                 */
                for (i = 0; i < s1->number; i++) {
                    lyd_free(s1->dset[i]);
                }
                break;
            } else {
                /* both contain some selection or containment node(s), so merge them */
                for (j = 0; j < s2->number; j++) { /* from */
                    copy = 0;
                    for (i = 0; i < s1->number; i++) { /* to */
                        if (s1->dset[i]->schema != s2->dset[j]->schema) {
                            continue;
                        }

                        /* we have something similar to diter1, explore it more */
                        switch (s2->dset[j]->schema->nodetype) {
                        case LYS_LIST:
                        case LYS_CONTAINER:
                            if (!filter_compare(s2->dset[j], s1->dset[i])) {
                                /* merge the two containers into the to */
                                filter_merge(s1->dset[i], s2->dset[j]);
                            } else {
                                /* check that some of them is not a selection node */
                                if (!s2->dset[j]->child) {
                                    /* from is selection node, so keep only it because to selects subset */
                                    lyd_free(s1->dset[i]);
                                    /* set the flag to copy the from child at the end */
                                    copy = 1;
                                    continue;
                                } else if (!s1->dset[i]->child) {
                                    /* to is already selection node, so ignore the from child */
                                } else {
                                    /* they are different so keep trying to search for some other matching instance */
                                    continue;
                                }
                            }

                            break;
                        case LYS_ANYXML:
                        case LYS_LEAFLIST:
                        case LYS_LEAF:
                            /* here it can be only a selection node, so do not duplicate it (keep i < s1->number) */
                            break;
                        default:
                            /* keep compiler silent */
                            break;
                        }

                        /* we have a match, so do not duplicate the current from child and go to check next from child */
                        /* i < s1->number */
                        break;
                    }

                    if (copy || i == s1->number) {
                        /* the node is not yet present in to, so move it there */
                        lyd_unlink(s2->dset[j]);
                        if (to->child) {
                            to->child->prev->next = s2->dset[j];
                            s2->dset[j]->prev = to->child->prev;
                            to->child->prev = s2->dset[j];
                        } else {
                            to->child = s2->dset[j];
                        }
                        s2->dset[j]->parent = to;
                    }
                }
            }
        } /* else from is empty, so nothing to do */

        break;

    default:
        /* no other type needed to cover,
         * keep the default branch to make compiler silent */
        break;
    }
    ret = EXIT_SUCCESS;

cleanup:
    ly_set_free(s1);
    ly_set_free(s2);

    return ret;
}

int
lyv_data_value(struct lyd_node *node, int options)
{
    int rc;

    assert(node);

    if (!(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        /* nothing to check */
        return EXIT_SUCCESS;
    }

    switch (((struct lys_node_leaf *)node->schema)->type.base) {
    case LY_TYPE_LEAFREF:
        if (!((struct lyd_node_leaf_list *)node)->value.leafref) {
            if (!(options & (LYD_OPT_FILTER | LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
                /* try to resolve leafref */
                rc = resolve_unres_data_item(node, UNRES_LEAFREF, 0, 0);
                if (rc) {
                    return EXIT_FAILURE;
                }
            } /* in other cases the leafref is always unresolved */
        }
        break;
    case LY_TYPE_INST:
        if (!(options & (LYD_OPT_FILTER | LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) &&
                ((struct lys_node_leaf *)node->schema)->type.info.inst.req > -1) {
            /* try to resolve instance-identifier to get know if the target exists */
            rc = resolve_unres_data_item(node, UNRES_INSTID, 0, 0);
            if (rc) {
                return EXIT_FAILURE;
            }
        }
        break;
    default:
        /* do nothing */
        break;
    }

    return EXIT_SUCCESS;
}

int
lyv_data_context(const struct lyd_node *node, int options, unsigned int line, struct unres_data *unres)
{
    struct lyd_node *iter;
    struct lys_node *siter = NULL;

    assert(node);

    /* check if the node instance is enabled by if-feature */
    if (lys_is_disabled(node->schema, 2)) {
        LOGVAL(LYE_INELEM, line, LY_VLOG_LYD, node, node->schema->name);
        return EXIT_FAILURE;
    }

    /* check all relevant when conditions */
    if (unres) {
        if (unres_data_add(unres, (struct lyd_node *)node, UNRES_WHEN, line) == -1) {
            return EXIT_FAILURE;
        }
    } else {
        if (resolve_unres_data_item((struct lyd_node *)node, UNRES_WHEN, 0, line)) {
            return EXIT_FAILURE;
        }
    }

    /* check for (non-)presence of status data in edit-config data */
    if ((options & (LYD_OPT_EDIT | LYD_OPT_GETCONFIG | LYD_OPT_CONFIG)) && (node->schema->flags & LYS_CONFIG_R)) {
        LOGVAL(LYE_INELEM, line, LY_VLOG_LYD, node, node->schema->name);
        return EXIT_FAILURE;
    }

    /* check elements order in case of RPC's input and output */
    if (node->validity && lyp_is_rpc(node->schema)) {
        siter = node->schema->prev;
        for (iter = node->prev; iter->next; iter = iter->prev) {
            while (siter->next) {
                if (siter == iter->schema) {
                    break;
                }
                siter = siter->prev;
            }

            if (!siter->next) {
                /* schema node of the node's predecessors not found in node's schema node predecessors
                 * so the elements are in wrong order */
                LOGVAL(LYE_INORDER, line, LY_VLOG_LYD, node, node->schema->name, iter->schema->name);
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int
lyv_data_content(struct lyd_node *node, int options, unsigned int line, struct unres_data *unres)
{
    const struct lys_node *schema, *siter;
    const struct lys_node *cs, *ch;
    struct lyd_node *diter, *start;
    struct lys_ident *ident;
    struct lys_tpdf *tpdf;

    assert(node);
    assert(node->schema);

    schema = node->schema; /* shortcut */

    if (node->validity) {
        /* check presence of all keys in case of list */
        if (schema->nodetype == LYS_LIST && !(options & (LYD_OPT_FILTER | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
            siter = (struct lys_node *)lyv_keys_present(node);
            if (siter) {
                /* key not found in the data */
                LOGVAL(LYE_MISSELEM, line, LY_VLOG_LYD, node, siter->name, schema->name);
                return EXIT_FAILURE;
            }
        }

        /* mandatory children */
        if ((schema->nodetype & (LYS_CONTAINER | LYS_LIST))
                && !(options & (LYD_OPT_FILTER | LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
            siter = ly_check_mandatory(node, NULL);
            if (siter) {
                if (siter->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
                    LOGVAL(LYE_SPEC, line, LY_VLOG_LYD, node,
                           "Number of \"%s\" instances in \"%s\" does not follow min/max constraints.",
                           siter->name, siter->parent->name);
                } else {
                    LOGVAL(LYE_MISSELEM, line, LY_VLOG_LYD, node, siter->name, siter->parent->name);
                }
                return EXIT_FAILURE;
            }
        }

        /* get the first sibling */
        if (node->parent) {
            start = node->parent->child;
        } else {
            for (start = node; start->prev->next; start = start->prev);
        }

        /* check that there are no data from different choice case */
        if (!(options & LYD_OPT_FILTER)) {
            /* init loop condition */
            ch = schema;

            while (ch->parent && (ch->parent->nodetype & (LYS_CASE | LYS_CHOICE))) {
                if (ch->parent->nodetype == LYS_CHOICE) {
                    cs = NULL;
                    ch = ch->parent;
                } else { /* ch->parent->nodetype == LYS_CASE */
                    cs = ch->parent;
                    ch = ch->parent->parent;
                }

                for (diter = start; diter; diter = diter->next) {
                    if (diter == node) {
                        continue;
                    }

                    /* find correct level to compare */
                    for (siter = diter->schema->parent; siter; siter = siter->parent) {
                        if (siter->nodetype == LYS_CHOICE) {
                            if (siter == ch) {
                                LOGVAL(LYE_MCASEDATA, line, LY_VLOG_LYD, node, ch->name);
                                return EXIT_FAILURE;
                            } else {
                                continue;
                            }
                        }

                        if (siter->nodetype == LYS_CASE) {
                            if (siter->parent != ch) {
                                continue;
                            } else if (!cs || cs != siter) {
                                LOGVAL(LYE_MCASEDATA, line, LY_VLOG_LYD, node, ch->name);
                                return EXIT_FAILURE;
                            }
                        }

                        /* diter is from something else choice (subtree) */
                        break;
                    }
                }
            }
        }

        /* keep this check the last since in case of filter it affects the data and can modify the tree */
        /* check number of instances (similar to list uniqueness) for non-list nodes */
        if (schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_ANYXML)) {
            /* find duplicity */
            for (diter = start; diter; diter = diter->next) {
                if (diter->schema == schema && diter != node) {
                    if (options & LYD_OPT_FILTER) {
                        /* normalize the filter if needed */
                        switch (schema->nodetype) {
                        case LYS_CONTAINER:
                            if (!filter_compare(diter, node)) {
                                /* merge the two containers, diter will be kept ... */
                                filter_merge(diter, node);
                                /* ... and node will be removed (ly_errno is not set) */
                                return EXIT_FAILURE;
                            } else {
                                /* check that some of them is not a selection node */
                                if (!diter->child) {
                                    /* keep diter since it selects all such containers
                                     * and let remove the node since it selects just a subset */
                                    return EXIT_FAILURE;
                                } else if (!node->child) {
                                    /* keep the node and remove diter since it selects subset
                                     * of what is selected by node */
                                    lyd_free(diter);
                                }
                                /* keep them as they are */
                                return EXIT_SUCCESS;
                            }
                            break;
                        case LYS_LEAF:
                            if (!((struct lyd_node_leaf_list *)diter)->value_str
                                    && ((struct lyd_node_leaf_list *)node)->value_str) {
                                /* the first instance is selection node but the new instance is content match node ->
                                 * since content match node also works as selection node. keep only the new instance
                                 */
                                lyd_free(diter);
                                /* return success to keep the node in the tree */
                                return EXIT_SUCCESS;
                            } else if (!((struct lyd_node_leaf_list *)node)->value_str
                                    || ly_strequal(((struct lyd_node_leaf_list *)diter)->value_str,
                                                   ((struct lyd_node_leaf_list *)node)->value_str)) {
                                /* keep the previous instance and remove the current one ->
                                 * return failure but do not set ly_errno */
                                return EXIT_FAILURE;
                            }
                            break;
                        case LYS_ANYXML:
                            /* filtering according to the anyxml content is not allowed,
                             * so anyxml is always a selection node with no content.
                             * Therefore multiple instances of anyxml does not make sense
                             */
                            /* failure is returned but no ly_errno is set */
                            return EXIT_FAILURE;
                        default:
                            /* not possible, but necessary to silence compiler warnings */
                            break;
                        }
                        /* we are done */
                        break;
                    } else {
                        LOGVAL(LYE_TOOMANY, line, LY_VLOG_LYD, node, schema->name,
                               schema->parent ? schema->parent->name : "data tree");
                        return EXIT_FAILURE;
                    }
                }
            }
        } else if (schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            /* uniqueness of list/leaflist instances */

            /* get the first list/leaflist instance sibling */
            if (options & (LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
                /* skip key uniqueness check in case of get/get-config data */
                start = NULL;
            } else {
                diter = start;
                start = NULL;
                while(diter) {
                    if (diter == node) {
                        diter = diter->next;
                        continue;
                    }

                    if (diter->schema == node->schema) {
                        /* the same list instance */
                        start = diter;
                        break;
                    }
                    diter = diter->next;
                }
            }

            /* check uniqueness of the list/leaflist instances (compare values) */
            for (diter = start; diter; diter = diter->next) {
                if (diter->schema != node->schema || diter == node ||
                        diter->validity) { /* skip comparison that will be done in future when checking diter as node */
                    continue;
                }

                if (options & LYD_OPT_FILTER) {
                    /* compare content match nodes */
                    if (!filter_compare(diter, node)) {
                        /* merge both nodes */
                        /* add selection and containment nodes from result into the diter,
                         * but only in case the diter already contains some selection nodes,
                         * otherwise it already will return all the data */
                        filter_merge(diter, node);

                        /* not the error, just return no data */
                        /* failure is returned but no ly_errno is set */
                        return EXIT_FAILURE;
                    } else if (node->schema->nodetype == LYS_LEAFLIST) {
                        /* in contrast to lists, leaflists can be still safely optimized if one of them
                         * is selection node. In that case wee need to keep the other node, which is content
                         * match node and it somehow limit the data to be filtered.
                         */
                        if (!((struct lyd_node_leaf_list *)diter)->value_str) {
                            /* the other instance is selection node, keep the new one whatever it is */
                            lyd_free(diter);
                            break;
                        } else if (!((struct lyd_node_leaf_list *)node)->value_str) {
                            /* the new instance is selection node, keep the previous instance which is
                             * content match node */
                            /* failure is returned but no ly_errno is set */
                            return EXIT_FAILURE;
                        }
                    }
                } else if (!lyd_compare(diter, node, 1)) { /* comparing keys and unique combinations */
                    LOGVAL(LYE_DUPLIST, line, LY_VLOG_LYD, node, schema->name);
                    return EXIT_FAILURE;
                }
            }
        }

        /* status - of the node's schema node itself and all its parents that
         * cannot have their own instance (like a choice statement) */
        siter = node->schema;
        do {
            if (((siter->flags & LYS_STATUS_MASK) == LYS_STATUS_OBSLT) && (options & LYD_OPT_OBSOLETE)) {
                LOGVAL(LYE_OBSDATA, line, LY_VLOG_LYD, node, schema->name);
                return EXIT_FAILURE;
            }
            siter = siter->parent;
        } while(siter && !(siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST)));

        /* status of the identity value */
        if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            if (options & LYD_OPT_OBSOLETE) {
                /* check that we are not instantiating obsolete type */
                tpdf = ((struct lys_node_leaf *)node->schema)->type.der;
                while(tpdf) {
                    if ((tpdf->flags & LYS_STATUS_MASK) == LYS_STATUS_OBSLT) {
                        LOGVAL(LYE_OBSTYPE, line, LY_VLOG_LYD, node, schema->name, tpdf->name);
                        return EXIT_FAILURE;
                    }
                    tpdf = tpdf->type.der;
                }
            }
            if (((struct lyd_node_leaf_list *)node)->value_type == LY_TYPE_IDENT) {
                ident = ((struct lyd_node_leaf_list *)node)->value.ident;
                if (lyp_check_status(schema->flags, schema->module, schema->name,
                                 ident->flags, ident->module, ident->name, line, schema)) {
                    return EXIT_FAILURE;
                }
            }
        }
    }

    /* check must conditions */
    if (unres) {
        if (unres_data_add(unres, node, UNRES_MUST, line) == -1) {
            return EXIT_FAILURE;
        }
    } else {
        if (resolve_unres_data_item(node, UNRES_MUST, 0, line)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
