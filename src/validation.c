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

#include "libyang.h"
#include "common.h"

static struct lys_node_leaf *
lyv_keys_present(struct lyd_node_list *list)
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

int
lyv_data_context(struct lys_node *schema, unsigned int line, int options)
{
    assert(schema);

    /* check if the node instance is enabled by if-feature */
    if (lys_is_disabled(schema, 2)) {
        LOGVAL(LYE_INELEM, line, schema->name);
        return EXIT_FAILURE;
    }

    /* check for (non-)presence of status data in edit-config data */
    if ((options & LYD_OPT_EDIT) && (schema->flags & LYS_CONFIG_R)) {
        LOGVAL(LYE_INELEM, line, schema->name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
int
lyv_data_content(struct lyd_node *node, unsigned int line, int options)
{
    struct lys_node *schema, *siter;
    struct lys_node *cs, *ch;
    struct lyd_node *diter, *start;

    assert(node);
    assert(node->schema);

    schema = node->schema; /* shortcut */

    /* check presence of all keys in case of list */
    if (schema->nodetype == LYS_LIST && !(options & LYD_OPT_FILTER)) {
        siter = (struct lys_node *)lyv_keys_present((struct lyd_node_list *)node);
        if (siter) {
            /* key not found in the data */
            LOGVAL(LYE_MISSELEM, line, siter->name, schema->name);
            return EXIT_FAILURE;
        }
    }

    /* mandatory children */
    if ((schema->nodetype & (LYS_CONTAINER | LYS_LIST)) && !(options & (LYD_OPT_FILTER | LYD_OPT_EDIT))) {
        siter = ly_check_mandatory(node);
        if (siter) {
            if (siter->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
                LOGVAL(LYE_SPEC, line, "Number of \"%s\" instances in \"%s\" does not follow min/max constraints.",
                       siter->name, siter->parent->name);
            } else {
                LOGVAL(LYE_MISSELEM, line, siter->name, siter->parent->name);
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

    /* check number of instances for non-list nodes */
    if (schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_ANYXML)) {
        /* find duplicity */
        for (diter = start; diter; diter = diter->next) {
            if (diter->schema == schema && diter != node) {
                if (options & LYD_OPT_FILTER) {
                    /* normalize the filter if needed */
                    switch (schema->nodetype) {
                    case LYS_CONTAINER:
                        if (!diter->child) {
                            /* previous instance is a selection node, so keep it
                             * and ignore the current instance - failure is returned
                             * but no ly_errno is set */
                            return EXIT_FAILURE;
                        }
                        if (!node->child) {
                            /* current instance is a selection node, so make the
                             * previous instance a a selection node (remove its
                             * children) and ignore the current instance */
                            while(diter->child) {
                                lyd_free(diter->child);
                            }
                            /* failure is returned but no ly_errno is set */
                            return EXIT_FAILURE;
                        }
                        /* TODO merging container used as a containment node */
                        break;
                    case LYS_LEAF:
                        if (((struct lyd_node_leaf *)diter)->value_str == ((struct lyd_node_leaf *)node)->value_str) {
                            /* failure is returned but no ly_errno is set */
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
                    LOGVAL(LYE_TOOMANY, line, schema->name, schema->parent ? schema->parent->name : "data tree");
                    return EXIT_FAILURE;
                }
            }
        }
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
                            LOGVAL(LYE_MCASEDATA, line, ch->name);
                            return EXIT_FAILURE;
                        } else {
                            continue;
                        }
                    }

                    if (siter->nodetype == LYS_CASE) {
                        if (siter->parent != ch) {
                            continue;
                        } else if (!cs || cs != siter) {
                            LOGVAL(LYE_MCASEDATA, line, ch->name);
                            return EXIT_FAILURE;
                        }
                    }

                    /* diter is from something else choice (subtree) */
                    break;
                }
            }
        }
    }

    /* uniqueness of (leaf-)list instances */
    if (schema->nodetype == LYS_LEAFLIST) {
        /* get the first leaf-list instance sibling */
        for (start = node;
                ((struct lyd_node_leaflist *)start)->lprev;
                start = (struct lyd_node *)((struct lyd_node_leaflist *)start)->lprev);

        /* check uniqueness of the leaf-list instances (compare values) */
        for (diter = start; diter; diter = (struct lyd_node *)((struct lyd_node_leaflist *)diter)->lnext) {
            if (diter == node) {
                continue;
            }

            if (!lyd_compare(diter, node, 0)) {
                if (options & LYD_OPT_FILTER) {
                    /* optimize filter and do not duplicate the same selection node,
                     * so this is not actually error, but the data are silently removed */
                    ((struct lyd_node_leaflist *)node)->lprev->lnext = NULL;
                    /* failure is returned but no ly_errno is set */
                    return EXIT_FAILURE;
                } else {
                    LOGVAL(LYE_DUPLEAFLIST, line, schema->name, ((struct lyd_node_leaflist *)node)->value_str);
                    return EXIT_FAILURE;
                }
            }
        }
    } else if (schema->nodetype == LYS_LIST) {
        /* get the first list instance sibling */
        for (start = node;
                ((struct lyd_node_list *)start)->lprev;
                start = (struct lyd_node *)((struct lyd_node_list *)start)->lprev);

        /* check uniqueness of the list instances */
        for (diter = start; diter; diter = (struct lyd_node *)((struct lyd_node_list *)diter)->lnext) {
            if (diter == node) {
                continue;
            }

            if (options & LYD_OPT_FILTER) {
                /* compare content match nodes */
                if (!lyd_filter_compare(diter, node)) {
                    /* merge both nodes */
                    /* add selection and containment nodes from result into the diter,
                     * but only in case the diter already contains some selection nodes,
                     * otherwise it already will return all the data */
                    lyd_filter_merge(diter, node);

                    /* not the error, just return no data */
                    ((struct lyd_node_list *)node)->lprev->lnext = NULL;
                    /* failure is returned but no ly_errno is set */
                    return EXIT_FAILURE;
                }
            } else {
                /* compare keys and unique combinations */
                if (!lyd_compare(diter, node, 1)) {
                    LOGVAL(LYE_DUPLIST, line, schema->name);
                    return EXIT_FAILURE;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
