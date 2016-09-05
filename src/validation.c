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

int
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
lyv_data_context(const struct lyd_node *node, int options, struct unres_data *unres)
{
    const struct lys_node *siter = NULL;
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node;

    assert(node);
    assert(unres);

    /* check if the node instance is enabled by if-feature */
    if (lys_is_disabled(node->schema, 2)) {
        LOGVAL(LYE_INELEM, LY_VLOG_LYD, node, node->schema->name);
        return EXIT_FAILURE;
    }

    /* check leafref/instance-identifier */
    if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        if (options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
            /* if leafref or instance-identifier, parse the value according to the
             * target's type, because the target leaf does not need to be present */
            if (leaf->value_type == LY_TYPE_LEAFREF || leaf->value_type == LY_TYPE_INST) {
                memset(&leaf->value, 0, sizeof leaf->value);
                if (lyp_parse_value_type(leaf, &((struct lys_node_leaf *)leaf->schema)->type, 0)) {
                    return EXIT_FAILURE;
                }
            }
        } else {
            /* if leafref or instance-identifier, store the node for later resolving */
            if (leaf->value_type == LY_TYPE_LEAFREF && !leaf->value.leafref) {
                if (unres_data_add(unres, (struct lyd_node *)node, UNRES_LEAFREF)) {
                    return EXIT_FAILURE;
                }
            } else if (leaf->value_type == LY_TYPE_INST) {
                if (unres_data_add(unres, (struct lyd_node *)node, UNRES_INSTID)) {
                    return EXIT_FAILURE;
                }
            }
        }
    }

    /* check all relevant when conditions */
    if ((node->when_status & LYD_WHEN) &&
            (!(options & LYD_OPT_TYPEMASK) || (options & (LYD_OPT_CONFIG | LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_RPCREPLY)))) {
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
    if (node->validity && lyp_is_rpc_action(node->schema)) {
        if ((node->prev != node) && node->prev->next) {
            for (siter = lys_getnext(node->schema, lys_parent(node->schema), node->schema->module, 0);
                    siter;
                    siter = lys_getnext(siter, lys_parent(siter), siter->module, 0)) {
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

struct eq_item {
    struct lyd_node *node;
    uint32_t hash;
    uint32_t over;
};

static int
eq_table_insert(struct eq_item *table, struct lyd_node *node, uint32_t hash, uint32_t tablesize, int action)
{
    uint32_t i, c;

    if (table[hash].node) {
        /* is it collision or is the cell just filled by an overflow item? */
        for (i = hash; table[i].node && table[i].hash != hash; i = (i + 1) % tablesize);
        if (!table[i].node) {
            goto first;
        }

        /* collision or instance duplication */
        c = table[i].over;
        do {
            if (table[i].hash != hash) {
                i = (i + 1) % tablesize;
                continue;
            }

            /* compare nodes */
            if (lyd_list_equal(node, table[i].node, action, 1)) {
                /* instance duplication */
                return EXIT_FAILURE;
            }
        } while (c--);

        /* collision, insert item into next free cell */
        table[hash].over++;
        for (i = (i + 1) % tablesize; table[i].node; i = (i + 1) % tablesize);
        table[i].hash = hash;
        table[i].node = node;
    } else {
first:
        /* first hash instance */
        table[hash].node = node;
        table[hash].hash = hash;
    }

    return EXIT_SUCCESS;
}

int
lyv_data_unique(struct lyd_node *node, struct lyd_node *start)
{
    struct lyd_node *diter, *key;
    struct lys_node_list *slist;
    struct ly_set *set;
    int i, j, n = 0, ret = EXIT_SUCCESS;
    uint32_t hash, u, usize = 0, hashmask;
    struct eq_item *keystable = NULL, **uniquetables = NULL;
    const char *id;

    /* get the first list/leaflist instance sibling */
    if (!start) {
        start = lyd_first_sibling(node);
    }

    /* check uniqueness of the list/leaflist instances (compare values) */
    set = ly_set_new();
    for (diter = start; diter; diter = diter->next) {
        if (diter->schema != node->schema) {
            /* check only instances of the same list/leaflist */
            continue;
        }

        /* remove the flag */
        diter->validity &= ~LYD_VAL_UNIQUE;

        /* store for comparison */
        ly_set_add(set, diter, LY_SET_OPT_USEASLIST);
    }

    if (set->number == 2) {
        /* simple comparison */
        if (lyd_list_equal(set->set.d[0], set->set.d[1], -1, 1)) {
            /* instance duplication */
            ly_set_free(set);
            return EXIT_FAILURE;
        }
    } else if (set->number > 2) {
        /* use hashes for comparison */
        /* first, allocate the table, the size depends on number of items in the set */
        for (u = 31; u > 0; u--) {
            usize = set->number << u;
            usize = usize >> u;
            if (usize == set->number) {
                break;
            }
        }
        if (u == 0) {
            usize = hashmask = 0xffffffff;
        } else {
            u = 32 - u;
            usize = 1 << u;
            hashmask = usize - 1;
        }
        keystable = calloc(usize, sizeof *keystable);
        if (!keystable) {
            LOGMEM;
            ret = EXIT_FAILURE;
            goto unique_cleanup;
        }
        n = 0;
        if (node->schema->nodetype == LYS_LIST) {
            n = ((struct lys_node_list *)node->schema)->unique_size;
            uniquetables = malloc(n * sizeof *uniquetables);
            if (!uniquetables) {
                LOGMEM;
                ret = EXIT_FAILURE;
                n = 0;
                goto unique_cleanup;
            }
            for (j = 0; j < n; j++) {
                uniquetables[j] = calloc(usize, sizeof **uniquetables);
                if (!uniquetables[j]) {
                    LOGMEM;
                    ret = EXIT_FAILURE;
                    goto unique_cleanup;
                }
            }
        }

        for (u = 0; u < set->number; u++) {
            /* get the hash for the instance - keys */
            if (node->schema->nodetype == LYS_LEAFLIST) {
                id = ((struct lyd_node_leaf_list *)set->set.d[u])->value_str;
                hash = dict_hash_multi(0, id, strlen(id));
            } else { /* LYS_LIST */
                for (hash = i = 0, key = set->set.d[u]->child;
                        i < ((struct lys_node_list *)set->set.d[u]->schema)->keys_size;
                        i++, key = key->next) {
                    id = ((struct lyd_node_leaf_list *)key)->value_str;
                    hash = dict_hash_multi(hash, id, strlen(id));
                }
            }
            /* finish the hash value */
            hash = dict_hash_multi(hash, NULL, 0) & hashmask;

            /* insert into the hashtable */
            if (eq_table_insert(keystable, set->set.d[u], hash, usize, 0)) {
                ret = EXIT_FAILURE;
                goto unique_cleanup;
            }

            /* and the same loop for unique (n is !0 only in case of list) - get the hash for the instances */
            for (j = 0; j < n; j++) {
                slist = (struct lys_node_list *)node->schema;
                id = NULL;
                for (i = hash = 0; i < slist->unique[j].expr_size; i++) {
                    diter = resolve_data_descendant_schema_nodeid(slist->unique[j].expr[i], set->set.d[u]->child);
                    if (diter) {
                        id = ((struct lyd_node_leaf_list *)diter)->value_str;
                    } else {
                        /* use default value */
                        id = lyd_get_unique_default(slist->unique[j].expr[i], set->set.d[u]);
                        if (ly_errno) {
                            ret = EXIT_FAILURE;
                            goto unique_cleanup;
                        }
                    }
                    if (!id) {
                        /* unique item not present nor has default value */
                        break;
                    }
                    hash = dict_hash_multi(hash, id, strlen(id));
                }
                if (!id) {
                    /* skip this list instance since its unique set is incomplete */
                    continue;
                }

                /* finish the hash value */
                hash = dict_hash_multi(hash, NULL, 0) & hashmask;

                /* insert into the hashtable */
                if (eq_table_insert(uniquetables[j], set->set.d[u], hash, usize, j + 1)) {
                    ret = EXIT_FAILURE;
                    goto unique_cleanup;
                }
            }
        }
    }

unique_cleanup:
    /* cleanup */
    ly_set_free(set);
    free(keystable);
    for (j = 0; j < n; j++) {
        if (!uniquetables[j]) {
            /* failed when allocating uniquetables[j], following j are not allocated */
            break;
        }
        free(uniquetables[j]);
    }
    free(uniquetables);

    return ret;
}

int
lyv_data_content(struct lyd_node *node, int options, struct unres_data *unres)
{
    const struct lys_node *schema, *siter;
    struct lyd_node *diter, *start = NULL;
    struct lys_ident *ident;
    struct lys_tpdf *tpdf;
    struct lys_type *type = NULL;
    struct lyd_node_leaf_list *leaf;
    int i, j = 0;
    uint8_t iff_size;
    struct lys_iffeature *iff;
    const char *id, *idname;

    assert(node);
    assert(node->schema);
    assert(unres);

    schema = node->schema; /* shortcut */

    if (node->validity & LYD_VAL_MAND) {
        /* check presence and correct order of all keys in case of list */
        if (schema->nodetype == LYS_LIST && !(options & (LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
            if (lyv_keys(node)) {
                return EXIT_FAILURE;
            }
        }

        if (schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_ANYDATA)) {
            /* check number of instances (similar to list uniqueness) for non-list nodes */

            /* find duplicity */
            start = lyd_first_sibling(node);
            for (diter = start; diter; diter = diter->next) {
                if (diter->schema == schema && diter != node) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, node, schema->name,
                           lys_parent(schema) ? lys_parent(schema)->name : "data tree");
                    return EXIT_FAILURE;
                }
            }
        }

        /* remove the flag */
        node->validity &= ~LYD_VAL_MAND;
    }

    if (!(options & (LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
        /* skip key uniqueness check in case of get/get-config data */
        if (schema->nodetype & (LYS_LIST | LYS_CONTAINER)) {
            LY_TREE_FOR(schema->child, siter) {
                if (siter->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
                    LY_TREE_FOR(node->child, diter) {
                        if (diter->schema == siter && (diter->validity & LYD_VAL_UNIQUE)) {
                            if (lyv_data_unique(diter, node->child)) {
                                return EXIT_FAILURE;
                            }
                            /* all schema instances checked, continue with another schema node */
                            break;
                        }
                    }
                }
            }
        }
    }

    if (node->validity) {
        /* status - of the node's schema node itself and all its parents that
         * cannot have their own instance (like a choice statement) */
        siter = node->schema;
        do {
            if (((siter->flags & LYS_STATUS_MASK) == LYS_STATUS_OBSLT) && (options & LYD_OPT_OBSOLETE)) {
                LOGVAL(LYE_OBSDATA, LY_VLOG_LYD, node, schema->name);
                return EXIT_FAILURE;
            }
            siter = lys_parent(siter);
        } while (siter && !(siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA)));

        /* status of the identity value */
        if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            if (options & LYD_OPT_OBSOLETE) {
                /* check that we are not instantiating obsolete type */
                tpdf = ((struct lys_node_leaf *)node->schema)->type.der;
                while (tpdf) {
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
                                 ident->flags, ident->module, ident->name, NULL)) {
                    LOGPATH(LY_VLOG_LYD, node);
                    return EXIT_FAILURE;
                }
            }
        }
    }

    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* since feature can be enabled/disabled, do this check despite the validity flag,
         * - check if the type value (enum, bit, identity) is disabled via feature  */
        leaf = (struct lyd_node_leaf_list *)node;
        switch (leaf->value_type) {
        case LY_TYPE_BITS:
            id = "Bit";
            /* get the count of bits */
            for (type = &((struct lys_node_leaf *)leaf->schema)->type; !type->info.bits.count; type = &type->der->type);
            for (j = iff_size = 0; j < type->info.bits.count; j++) {
                if (!leaf->value.bit[j]) {
                    continue;
                }
                idname = leaf->value.bit[j]->name;
                iff_size = leaf->value.bit[j]->iffeature_size;
                iff = leaf->value.bit[j]->iffeature;
                break;
nextbit:
                iff_size = 0;
            }
            break;
        case LY_TYPE_ENUM:
            id = "Enum";
            idname = leaf->value_str;
            iff_size = leaf->value.enm->iffeature_size;
            iff = leaf->value.enm->iffeature;
            break;
        case LY_TYPE_IDENT:
            id = "Identity";
            idname = leaf->value_str;
            iff_size = leaf->value.ident->iffeature_size;
            iff = leaf->value.ident->iffeature;
            break;
        default:
            iff_size = 0;
            break;
        }

        if (iff_size) {
            for (i = 0; i < iff_size; i++) {
                if (!resolve_iffeature(&iff[i])) {
                    LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, leaf->value_str, schema->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_LYD, node, "%s \"%s\" is disabled by its if-feature condition.",
                           id, idname);
                    return EXIT_FAILURE;
                }
            }
            if (leaf->value_type == LY_TYPE_BITS) {
                goto nextbit;
            }
        }
    }

    /* check must conditions */
    if (resolve_applies_must(node->schema) && unres_data_add(unres, node, UNRES_MUST) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
