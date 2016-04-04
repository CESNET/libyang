/**
 * @file validation.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data tree validation functions
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
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

static int
lyv_keys(const struct lyd_node *list)
{
    struct lyd_node *child;
    struct lys_node_list *schema = (struct lys_node_list *)list->schema; /* shortcut */
    int i;

    for (i = 0, child = list->child; i < schema->keys_size; i++, child = child->next) {
        if (!child || child->schema != (struct lys_node *)schema->keys[i]) {
            /* key not found on the correct place */
            LOGVAL(LYE_MISSELEM, LY_VLOG_LYD, list, schema->keys[i]->name, schema->name);
            for ( ; child; child = child->next) {
                if (child->schema == (struct lys_node *)schema->keys[i]) {
                    LOGVAL(LYE_SPEC, LY_VLOG_LYD, child, "Invalid position of the key element.");
                    break;
                }
            }
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
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
            if (!(options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
                /* try to resolve leafref */
                rc = resolve_unres_data_item(node, UNRES_LEAFREF);
                if (rc) {
                    return EXIT_FAILURE;
                }
            } /* in other cases the leafref is always unresolved */
        }
        break;
    case LY_TYPE_INST:
        if (!(options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) &&
                ((struct lys_node_leaf *)node->schema)->type.info.inst.req > -1) {
            /* try to resolve instance-identifier to get know if the target exists */
            rc = resolve_unres_data_item(node, UNRES_INSTID);
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
lyv_data_context(const struct lyd_node *node, int options, struct unres_data *unres)
{
    const struct lys_node *siter = NULL;

    assert(node);
    assert(unres);

    /* check if the node instance is enabled by if-feature */
    if (lys_is_disabled(node->schema, 2)) {
        LOGVAL(LYE_INELEM, LY_VLOG_LYD, node, node->schema->name);
        return EXIT_FAILURE;
    }

    /* check leafref/instance-identifier */
    if ((node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) &&
            !(options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
        /* remove possible unres flags from type */
        ((struct lyd_node_leaf_list *)node)->value_type &= LY_DATA_TYPE_MASK;

        /* if leafref or instance-identifier, store the node for later resolving */
        if (((struct lyd_node_leaf_list *)node)->value_type == LY_TYPE_LEAFREF) {
            if (unres_data_add(unres, (struct lyd_node *)node, UNRES_LEAFREF)) {
                return EXIT_FAILURE;
            }
        } else if (((struct lyd_node_leaf_list *)node)->value_type == LY_TYPE_INST) {
            if (unres_data_add(unres, (struct lyd_node *)node, UNRES_INSTID)) {
                return EXIT_FAILURE;
            }
        }
    }

    /* check all relevant when conditions */
    if ((!(options & LYD_OPT_TYPEMASK) || (options & LYD_OPT_CONFIG)) && (node->when_status & LYD_WHEN)) {
        if (unres_data_add(unres, (struct lyd_node *)node, UNRES_WHEN)) {
            return EXIT_FAILURE;
        }
    }

    /* check for (non-)presence of status data in edit-config data */
    if ((options & (LYD_OPT_EDIT | LYD_OPT_GETCONFIG | LYD_OPT_CONFIG)) && (node->schema->flags & LYS_CONFIG_R)) {
        LOGVAL(LYE_INELEM, LY_VLOG_LYD, node, node->schema->name);
        return EXIT_FAILURE;
    }

    /* check elements order in case of RPC's input and output */
    if (node->validity && lyp_is_rpc(node->schema)) {
        if ((node->prev != node) && node->prev->next) {
            for (siter = lys_getnext(node->schema, node->schema->parent, node->schema->module, 0);
                    siter;
                    siter = lys_getnext(siter, siter->parent, siter->module, 0)) {
                if (siter == node->prev->schema) {
                    /* data predecessor has the schema node after
                     * the schema node of the data node being checked */
                    LOGVAL(LYE_INORDER, LY_VLOG_LYD, node, node->schema->name, siter->name);
                    return EXIT_FAILURE;
                }
            }

        }
    }

    return EXIT_SUCCESS;
}

int
lyv_data_content(struct lyd_node *node, int options, struct unres_data *unres)
{
    const struct lys_node *schema, *siter;
    struct lyd_node *diter, *start;
    struct lys_ident *ident;
    struct lys_tpdf *tpdf;

    assert(node);
    assert(node->schema);
    assert(unres);

    schema = node->schema; /* shortcut */

    if (node->validity) {
        /* check presence and correct order of all keys in case of list */
        if (schema->nodetype == LYS_LIST && !(options & (LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
            if (lyv_keys(node)) {
                return EXIT_FAILURE;
            }
        }

        /* mandatory children */
        if ((schema->nodetype & (LYS_CONTAINER | LYS_LIST))
                && !(options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
            siter = ly_check_mandatory(node, NULL);
            if (siter) {
                if (siter->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
                    LOGVAL(LYE_INCOUNT, LY_VLOG_LYD, node, siter->name, siter->parent->name);
                } else {
                    LOGVAL(LYE_MISSELEM, LY_VLOG_LYD, node, siter->name, siter->parent->name);
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

        /* keep this check the last since in case of filter it affects the data and can modify the tree */
        /* check number of instances (similar to list uniqueness) for non-list nodes */
        if (schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_ANYXML)) {
            /* find duplicity */
            for (diter = start; diter; diter = diter->next) {
                if (diter->schema == schema && diter != node) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, node, schema->name,
                           schema->parent ? schema->parent->name : "data tree");
                    return EXIT_FAILURE;
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
                if (!lyd_compare(diter, node, 1)) { /* comparing keys and unique combinations */
                    LOGVAL(LYE_DUPLIST, LY_VLOG_LYD, node, schema->name);
                    return EXIT_FAILURE;
                }
            }
        }

        /* status - of the node's schema node itself and all its parents that
         * cannot have their own instance (like a choice statement) */
        siter = node->schema;
        do {
            if (((siter->flags & LYS_STATUS_MASK) == LYS_STATUS_OBSLT) && (options & LYD_OPT_OBSOLETE)) {
                LOGVAL(LYE_OBSDATA, LY_VLOG_LYD, node, schema->name);
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
                        LOGVAL(LYE_OBSTYPE, LY_VLOG_LYD, node, schema->name, tpdf->name);
                        return EXIT_FAILURE;
                    }
                    tpdf = tpdf->type.der;
                }
            }
            if (((struct lyd_node_leaf_list *)node)->value_type == LY_TYPE_IDENT) {
                ident = ((struct lyd_node_leaf_list *)node)->value.ident;
                if (lyp_check_status(schema->flags, schema->module, schema->name,
                                 ident->flags, ident->module, ident->name, schema)) {
                    return EXIT_FAILURE;
                }
            }
        }
    }

    /* check must conditions */
    if (resolve_applies_must(node) && unres_data_add(unres, node, UNRES_MUST) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
