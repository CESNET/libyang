/**
 * @file parser_xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML data parser for libyang
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
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "parser.h"
#include "tree_internal.h"
#include "validation.h"
#include "xml_internal.h"

/* does not log */
static struct lys_node *
xml_data_search_schemanode(struct lyxml_elem *xml, struct lys_node *start, int options)
{
    struct lys_node *result, *aux;

    LY_TREE_FOR(start, result) {
        /* skip groupings ... */
        if (result->nodetype == LYS_GROUPING) {
            continue;
        /* ... and output in case of RPC ... */
        } else if (result->nodetype == LYS_OUTPUT && (options & LYD_OPT_RPC)) {
            continue;
        /* ... and input in case of RPC reply */
        } else if (result->nodetype == LYS_INPUT && (options & LYD_OPT_RPCREPLY)) {
            continue;
        }

        /* go into cases, choices, uses and in RPCs into input and output */
        if (result->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES | LYS_INPUT | LYS_OUTPUT)) {
            aux = xml_data_search_schemanode(xml, result->child, options);
            if (aux) {
                /* we have matching result */
                return aux;
            }
            /* else, continue with next schema node */
            continue;
        }

        /* match data nodes */
        if (ly_strequal(result->name, xml->name, 1)) {
            /* names matches, what about namespaces? */
            if (ly_strequal(lys_main_module(result->module)->ns, xml->ns->value, 1)) {
                /* we have matching result */
                return result;
            }
            /* else, continue with next schema node */
            continue;
        }
    }

    /* no match */
    return NULL;
}

/* logs directly */
static int
xml_get_value(struct lyd_node *node, struct lyxml_elem *xml, int options, int editbits)
{
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node;
    int resolvable;

    assert(node && (node->schema->nodetype & (LYS_LEAFLIST | LYS_LEAF)) && xml);

    if (options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
        resolvable = 0;
    } else {
        resolvable = 1;
    }

    leaf->value_str = lydict_insert(node->schema->module->ctx, xml->content, 0);

    if ((editbits & 0x10) && (node->schema->nodetype & LYS_LEAF) && (!leaf->value_str || !leaf->value_str[0])) {
        /* we have edit-config leaf/leaf-list with delete operation and no (empty) value,
         * this is not a bug since the node is just used as a kind of selection node */
        leaf->value_type = LY_TYPE_ERR;
        return EXIT_SUCCESS;
    }

    /* the value is here converted to a JSON format if needed in case of LY_TYPE_IDENT and LY_TYPE_INST or to a
     * canonical form of the value */
    if (!lyp_parse_value(&((struct lys_node_leaf *)leaf->schema)->type, &leaf->value_str, xml, NULL, leaf, 1,
                         resolvable, 0)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, struct lyd_node *parent, struct lyd_node *first_sibling,
               struct lyd_node *prev, int options, struct unres_data *unres, struct lyd_node **result,
               struct lyd_node **act_notif)
{
    struct lyd_node *diter, *dlast;
    struct lys_node *schema = NULL, *target;
    struct lys_node_augment *aug;
    struct lyd_attr *dattr, *dattr_iter;
    struct lyxml_attr *attr;
    struct lyxml_elem *child, *next;
    int i, j, havechildren, r, flag, pos, editbits = 0;
    int ret = 0;
    const char *str = NULL;

    assert(xml);
    assert(result);
    *result = NULL;

    if (!xml->ns || !xml->ns->value) {
        if (options & LYD_OPT_STRICT) {
            LOGVAL(LYE_XML_MISS, LY_VLOG_XML, xml, "element's", "namespace");
            return -1;
        } else {
            return 0;
        }
    }

    /* find schema node */
    if (!parent) {
        /* starting in root */
        for (i = 0; i < ctx->models.used; i++) {
            /* skip just imported modules, data can be coupled only with the implemented modules */
            if (!ctx->models.list[i]->implemented) {
                continue;
            }
            /* match data model based on namespace */
            if (ly_strequal(ctx->models.list[i]->ns, xml->ns->value, 1)) {
                /* get the proper schema node */
                schema = xml_data_search_schemanode(xml, ctx->models.list[i]->data, options);
                if (!schema) {
                    /* it still can be the specific case of this module containing an augment of another module
                     * top-level choice or top-level choice's case, bleh */
                    for (j = 0; j < ctx->models.list[i]->augment_size; ++j) {
                        aug = &ctx->models.list[i]->augment[j];
                        target = aug->target;
                        if (target->nodetype & (LYS_CHOICE | LYS_CASE)) {
                            /* 1) okay, the target is choice or case */
                            while (target && (target->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES))) {
                                target = lys_parent(target);
                            }
                            /* 2) now, the data node will be top-level, there are only non-data schema nodes */
                            if (!target) {
                                while ((schema = (struct lys_node *)lys_getnext(schema, (struct lys_node *)aug, NULL, 0))) {
                                    /* 3) alright, even the name matches, we found our schema node */
                                    if (ly_strequal(schema->name, xml->name, 1)) {
                                        break;
                                    }
                                }
                            }
                        }

                        if (schema) {
                            break;
                        }
                    }
                }
                break;
            }
        }
    } else {
        /* parsing some internal node, we start with parent's schema pointer */
        schema = xml_data_search_schemanode(xml, parent->schema->child, options);
    }
    if (!schema) {
        if ((options & LYD_OPT_STRICT) || ly_ctx_get_module_by_ns(ctx, xml->ns->value, NULL)) {
            LOGVAL(LYE_INELEM, LY_VLOG_LYD, parent, xml->name);
            return -1;
        } else {
            return 0;
        }
    } else if (!lys_node_module(schema)->implemented) {
        if (options & LYD_OPT_STRICT) {
            LOGVAL(LYE_INELEM, LY_VLOG_LYD, parent, xml->name);
            return -1;
        } else {
            return 0;
        }
    }

    /* create the element structure */
    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
        *result = calloc(1, sizeof **result);
        havechildren = 1;
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        *result = calloc(1, sizeof(struct lyd_node_leaf_list));
        havechildren = 0;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        *result = calloc(1, sizeof(struct lyd_node_anydata));
        havechildren = 0;
        break;
    default:
        LOGINT;
        return -1;
    }
    if (!(*result)) {
        LOGMEM;
        return -1;
    }

    (*result)->prev = *result;
    (*result)->schema = schema;
    (*result)->parent = parent;
    diter = NULL;
    if (parent && parent->child && schema->nodetype == LYS_LEAF && parent->schema->nodetype == LYS_LIST &&
        (pos = lys_is_key((struct lys_node_list *)parent->schema, (struct lys_node_leaf *)schema))) {
        /* it is key and we need to insert it into a correct place */
        for (i = 0, diter = parent->child;
                diter && i < (pos - 1) && diter->schema->nodetype == LYS_LEAF &&
                    lys_is_key((struct lys_node_list *)parent->schema, (struct lys_node_leaf *)diter->schema);
                i++, diter = diter->next);
        if (diter) {
            /* out of order insertion - insert list's key to the correct position, before the diter */
            if (options & LYD_OPT_STRICT) {
                LOGVAL(LYE_INORDER, LY_VLOG_LYD, *result, schema->name, diter->schema->name);
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, *result, "Invalid position of the key \"%s\" in a list \"%s\".",
                       schema->name, parent->schema->name);
                free(*result);
                *result = NULL;
                return -1;
            } else {
                LOGWRN("Invalid position of the key \"%s\" in a list \"%s\".", schema->name, parent->schema->name)
            }
            if (parent->child == diter) {
                parent->child = *result;
                /* update first_sibling */
                first_sibling = *result;
            }
            if (diter->prev->next) {
                diter->prev->next = *result;
            }
            (*result)->prev = diter->prev;
            diter->prev = *result;
            (*result)->next = diter;
        }
    }
    if (!diter) {
        /* simplified (faster) insert as the last node */
        if (parent && !parent->child) {
            parent->child = *result;
        }
        if (prev) {
            (*result)->prev = prev;
            prev->next = *result;

            /* fix the "last" pointer */
            first_sibling->prev = *result;
        } else {
            (*result)->prev = *result;
            first_sibling = *result;
        }
    }
    (*result)->validity = LYD_VAL_NOT;
    if (resolve_applies_when(schema, 0, NULL)) {
        (*result)->when_status = LYD_WHEN;
    }

    /* check insert attribute and its values */
    if (options & LYD_OPT_EDIT) {
        /* 0x01 - insert attribute present
         * 0x02 - insert is relative (before or after)
         * 0x04 - value attribute present
         * 0x08 - key attribute present
         * 0x10 - operation not allowing insert attribute
         */
        for (attr = xml->attr; attr; attr = attr->next) {
            if (attr->type != LYXML_ATTR_STD || !attr->ns) {
                /* not interesting attribute or namespace declaration */
                continue;
            }

            if (!strcmp(attr->name, "operation") && !strcmp(attr->ns->value, LY_NSNC)) {
                if (editbits & 0x10) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "operation attributes", xml->name);
                    return -1;
                }

                if (!strcmp(attr->value, "delete") || !strcmp(attr->value, "remove")) {
                    editbits |= 0x10;
                } else if (strcmp(attr->value, "create") &&
                        strcmp(attr->value, "merge") &&
                        strcmp(attr->value, "replace")) {
                    /* unknown operation */
                    LOGVAL(LYE_INVALATTR, LY_VLOG_LYD, (*result), attr->value, attr->name);
                    return -1;
                }
            } else if (!strcmp(attr->name, "insert") && !strcmp(attr->ns->value, LY_NSYANG)) {
                /* 'insert' attribute present */
                if (!(schema->flags & LYS_USERORDERED)) {
                    /* ... but it is not expected */
                    LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), "insert", schema->name);
                    return -1;
                }

                if (editbits & 0x01) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "insert attributes", xml->name);
                    return -1;
                }
                if (!strcmp(attr->value, "first") || !strcmp(attr->value, "last")) {
                    editbits |= 0x01;
                } else if (!strcmp(attr->value, "before") || !strcmp(attr->value, "after")) {
                    editbits |= 0x01 | 0x02;
                } else {
                    LOGVAL(LYE_INVALATTR, LY_VLOG_LYD, (*result), attr->value, attr->name);
                    return -1;
                }
                str = attr->name;
            } else if (!strcmp(attr->name, "value") && !strcmp(attr->ns->value, LY_NSYANG)) {
                if (editbits & 0x04) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "value attributes", xml->name);
                    return -1;
                } else if (schema->nodetype & LYS_LIST) {
                    LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), attr->name, schema->name);
                    return -1;
                }
                editbits |= 0x04;
                str = attr->name;
            } else if (!strcmp(attr->name, "key") && !strcmp(attr->ns->value, LY_NSYANG)) {
                if (editbits & 0x08) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "key attributes", xml->name);
                    return -1;
                } else if (schema->nodetype & LYS_LEAFLIST) {
                    LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), attr->name, schema->name);
                    return -1;
                }
                editbits |= 0x08;
                str = attr->name;
            }
        }

        /* report errors */
        if (editbits > 0x10 || (editbits && editbits < 0x10 &&
                (!(schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) || !(schema->flags & LYS_USERORDERED)))) {
            /* attributes in wrong elements */
            LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), str, xml->name);
            return -1;
        } else if (editbits == 3) {
            /* 0x01 | 0x02 - relative position, but value/key is missing */
            if (schema->nodetype & LYS_LIST) {
                LOGVAL(LYE_MISSATTR, LY_VLOG_LYD, (*result), "key", xml->name);
            } else { /* LYS_LEAFLIST */
                LOGVAL(LYE_MISSATTR, LY_VLOG_LYD, (*result), "value", xml->name);
            }
            return -1;
        } else if ((editbits & (0x04 | 0x08)) && !(editbits & 0x02)) {
            /* key/value without relative position */
            LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), (editbits & 0x04) ? "value" : "key", schema->name);
            return -1;
        }
    }

    /* type specific processing */
    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* type detection and assigning the value */
        if (xml_get_value(*result, xml, options, editbits)) {
            goto error;
        }
    } else if (schema->nodetype & LYS_ANYDATA) {
        /* store children values */
        if (xml->child) {
            child = xml->child;
            /* manually unlink all siblings and correct namespaces */
            xml->child = NULL;
            LY_TREE_FOR(child, next) {
                next->parent = NULL;
                lyxml_correct_elem_ns(ctx, next, 1, 1);
            }

            ((struct lyd_node_anydata *)*result)->value_type = LYD_ANYDATA_XML;
            ((struct lyd_node_anydata *)*result)->value.xml = child;
        } else {
            ((struct lyd_node_anydata *)*result)->value_type = LYD_ANYDATA_CONSTSTRING;
            ((struct lyd_node_anydata *)*result)->value.str = lydict_insert(ctx, xml->content, 0);
        }
    } else if (schema->nodetype & (LYS_RPC | LYS_ACTION)) {
        if (!(options & LYD_OPT_RPC) || *act_notif) {
            LOGVAL(LYE_INELEM, LY_VLOG_LYD, (*result), schema->name);
            LOGVAL(LYE_SPEC, LY_VLOG_LYD, (*result), "Unexpected %s node \"%s\".",
                   (schema->nodetype == LYS_RPC ? "rpc" : "action"), schema->name);
            goto error;
        }
        *act_notif = *result;
    } else if (schema->nodetype == LYS_NOTIF) {
        if (!(options & LYD_OPT_NOTIF) || *act_notif) {
            LOGVAL(LYE_INELEM, LY_VLOG_LYD, (*result), schema->name);
            LOGVAL(LYE_SPEC, LY_VLOG_LYD, (*result), "Unexpected notification node \"%s\".", schema->name);
            goto error;
        }
        *act_notif = *result;
    }

    /* first part of validation checks */
    if (!(options & LYD_OPT_TRUSTED) && lyv_data_context(*result, options, unres)) {
        goto error;
    }

    for (attr = xml->attr; attr; attr = attr->next) {
        flag = 0;
        if (attr->type != LYXML_ATTR_STD) {
            continue;
        } else if (!attr->ns) {
            if ((*result)->schema->nodetype != LYS_ANYXML ||
                    !ly_strequal((*result)->schema->name, "filter", 0) ||
                    !ly_strequal((*result)->schema->module->name, "ietf-netconf", 0)) {
                if (options & LYD_OPT_STRICT) {
                    LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), attr->name, xml->name);
                    LOGVAL(LYE_SPEC, LY_VLOG_LYD, (*result), "Attribute \"%s\" with no namespace (schema).",
                           attr->name);
                    goto error;
                } else {
                    LOGWRN("Ignoring \"%s\" attribute in \"%s\" element.", attr->name, xml->name);
                    continue;
                }
            } else {
                /* exception for filter's attributes */
                flag = 1;
            }
        }

        dattr = malloc(sizeof *dattr);
        if (!dattr) {
            goto error;
        }
        dattr->next = NULL;
        dattr->name = attr->name;
        if (flag && ly_strequal(attr->name, "select", 0)) {
            dattr->value = transform_xml2json(ctx, attr->value, xml, 1);
            if (!dattr->value) {
                free(dattr);
                goto error;
            }
            lydict_remove(ctx, attr->value);
        } else {
            dattr->value = attr->value;
        }
        attr->name = NULL;
        attr->value = NULL;

        if (!attr->ns) {
            /* filter's attributes, it actually has no namespace, but we need some for internal representation */
            dattr->module = (*result)->schema->module;
        } else {
            dattr->module = (struct lys_module *)ly_ctx_get_module_by_ns(ctx, attr->ns->value, NULL);
        }
        if (!dattr->module) {
            free(dattr);
            if (options & LYD_OPT_STRICT) {
                LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), attr->name, xml->name);
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, (*result), "Attribute \"%s\" from unknown schema (\"%s\").",
                       attr->name, attr->ns->value);
                goto error;
            } else {
                LOGWRN("Attribute \"%s\" from unknown schema (\"%s\") - skipping.", attr->name, attr->ns->value);
                continue;
            }
        }

        if (!(*result)->attr) {
            (*result)->attr = dattr;
        } else {
            for (dattr_iter = (*result)->attr; dattr_iter->next; dattr_iter = dattr_iter->next);
            dattr_iter->next = dattr;
        }
    }

    /* process children */
    if (havechildren && xml->child) {
        diter = dlast = NULL;
        LY_TREE_FOR_SAFE(xml->child, next, child) {
            if (schema->nodetype & (LYS_RPC | LYS_NOTIF)) {
                r = xml_parse_data(ctx, child, *result, (*result)->child, dlast, 0, unres, &diter, act_notif);
            } else {
                r = xml_parse_data(ctx, child, *result, (*result)->child, dlast, options, unres, &diter, act_notif);
            }
            if (r) {
                goto error;
            } else if (options & LYD_OPT_DESTRUCT) {
                lyxml_free(ctx, child);
            }
            if (diter && !diter->next) {
                /* the child was parsed/created and it was placed as the last child. The child can be inserted
                 * out of order (not as the last one) in case it is a list's key present out of the correct order */
                dlast = diter;
            }
        }
    }

    /* if we have empty non-presence container, we keep it, but mark it as default */
    if (schema->nodetype == LYS_CONTAINER && !(*result)->child &&
            !(*result)->attr && !((struct lys_node_container *)schema)->presence) {
        (*result)->dflt = 1;
    }

    /* rest of validation checks */
    ly_err_clean(1);
    if (!(options & LYD_OPT_TRUSTED) &&
            (lyv_data_content(*result, options, unres) ||
             lyv_multicases(*result, NULL, prev ? &first_sibling : NULL, 0, NULL))) {
        if (ly_errno) {
            goto error;
        } else {
            goto clear;
        }
    }

    /* validation successful */
    if ((*result)->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* postpone checking when there will be all list/leaflist instances */
        (*result)->validity = LYD_VAL_UNIQUE;
    } else {
        (*result)->validity = LYD_VAL_OK;
    }

    return ret;

error:
    ret--;

clear:
    /* cleanup */
    for (i = unres->count - 1; i >= 0; i--) {
        /* remove unres items connected with the node being removed */
        if (unres->node[i] == *result) {
            unres_data_del(unres, i);
        }
    }
    lyd_free(*result);
    *result = NULL;

    return ret;
}

API struct lyd_node *
lyd_parse_xml(struct ly_ctx *ctx, struct lyxml_elem **root, int options, ...)
{
    va_list ap;
    int r, i;
    struct unres_data *unres = NULL;
    const struct lyd_node *rpc_act = NULL, *data_tree = NULL;
    struct lyd_node *result = NULL, *iter, *last, *reply_parent = NULL, *reply_top = NULL, *act_notif = NULL;
    struct lyxml_elem *xmlstart, *xmlelem, *xmlaux;
    struct ly_set *set;

    ly_err_clean(1);

    if (!ctx || !root) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    if (lyp_check_options(options)) {
        LOGERR(LY_EINVAL, "%s: Invalid options (multiple data type flags set).", __func__);
        return NULL;
    }

    if (!(*root)) {
        /* empty tree - no work is needed */
        lyd_validate(&result, options, ctx);
        return result;
    }

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        return NULL;
    }

    va_start(ap, options);
    if (options & LYD_OPT_RPCREPLY) {
        rpc_act = va_arg(ap, const struct lyd_node *);
        if (!rpc_act || rpc_act->parent || !(rpc_act->schema->nodetype & (LYS_RPC | LYS_LIST | LYS_CONTAINER))) {
            LOGERR(LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *rpc_act).", __func__);
            goto error;
        }
        if (rpc_act->schema->nodetype == LYS_RPC) {
            /* RPC request */
            reply_top = reply_parent = _lyd_new(NULL, rpc_act->schema, 0);
        } else {
            /* action request */
            reply_top = lyd_dup(rpc_act, 1);
            LY_TREE_DFS_BEGIN(reply_top, iter, reply_parent) {
                if (reply_parent->schema->nodetype == LYS_ACTION) {
                    break;
                }
                LY_TREE_DFS_END(reply_top, iter, reply_parent);
            }
            if (!reply_parent) {
                LOGERR(LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *rpc_act).", __func__);
                lyd_free_withsiblings(reply_top);
                goto error;
            }
            lyd_free_withsiblings(reply_parent->child);
        }
    }
    if (options & (LYD_OPT_RPC | LYD_OPT_NOTIF | LYD_OPT_RPCREPLY)) {
        data_tree = va_arg(ap, const struct lyd_node *);
        if (data_tree) {
            LY_TREE_FOR((struct lyd_node *)data_tree, iter) {
                if (iter->parent) {
                    /* a sibling is not top-level */
                    LOGERR(LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *data_tree).", __func__);
                    goto error;
                }
            }

            /* move it to the beginning */
            for (; data_tree->prev->next; data_tree = data_tree->prev);

            /* LYD_OPT_NOSIBLINGS cannot be set in this case */
            if (options & LYD_OPT_NOSIBLINGS) {
                LOGERR(LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree with LYD_OPT_NOSIBLINGS).", __func__);
                goto error;
            }
        }
    }

    if (!(options & LYD_OPT_NOSIBLINGS)) {
        /* locate the first root to process */
        if ((*root)->parent) {
            xmlstart = (*root)->parent->child;
        } else {
            xmlstart = *root;
            while(xmlstart->prev->next) {
                xmlstart = xmlstart->prev;
            }
        }
    } else {
        xmlstart = *root;
    }

    if ((options & LYD_OPT_RPC)
            && !strcmp(xmlstart->name, "action") && !strcmp(xmlstart->ns->value, "urn:ietf:params:xml:ns:yang:1")) {
        /* it's an action, not a simple RPC */
        xmlstart = xmlstart->child;
    }

    iter = last = NULL;
    LY_TREE_FOR_SAFE(xmlstart, xmlaux, xmlelem) {
        r = xml_parse_data(ctx, xmlelem, reply_parent, result, last, options, unres, &iter, &act_notif);
        if (r) {
            if (reply_top) {
                result = reply_top;
            }
            goto error;
        } else if (options & LYD_OPT_DESTRUCT) {
            lyxml_free(ctx, xmlelem);
            *root = xmlaux;
        }
        if (iter) {
            last = iter;
        }
        if (!result) {
            result = iter;
        }

        if (options & LYD_OPT_NOSIBLINGS) {
            /* stop after the first processed root */
            break;
        }
    }

    if (reply_top) {
        result = reply_top;
    }

    if ((options & LYD_OPT_RPCREPLY) && (rpc_act->schema->nodetype != LYS_RPC)) {
        /* action reply */
        act_notif = reply_parent;
    } else if ((options & (LYD_OPT_RPC | LYD_OPT_NOTIF)) && !act_notif) {
        ly_vecode = LYVE_INELEM;
        LOGVAL(LYE_SPEC, LY_VLOG_LYD, result, "Missing %s node.", (options & LYD_OPT_RPC ? "action" : "notification"));
        goto error;
    }

    /* check for uniquness of top-level lists/leaflists because
     * only the inner instances were tested in lyv_data_content() */
    set = ly_set_new();
    LY_TREE_FOR(result, iter) {
        if (!(iter->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(iter->validity & LYD_VAL_UNIQUE)) {
            continue;
        }

        /* check each list/leaflist only once */
        i = set->number;
        if (ly_set_add(set, iter->schema, 0) != i) {
            /* already checked */
            continue;
        }

        if (lyv_data_unique(iter, result)) {
            ly_set_free(set);
            goto error;
        }
    }
    ly_set_free(set);

    /* add default values, resolve unres and check for mandatory nodes in final tree */
    if (lyd_defaults_add_unres(&result, options, ctx, data_tree, act_notif, unres)) {
        goto error;
    }
    if (!(options & LYD_OPT_TRUSTED) && lyd_check_mandatory_tree((act_notif ? act_notif : result), ctx, options)) {
        goto error;
    }

    free(unres->node);
    free(unres->type);
    free(unres);
    va_end(ap);

    return result;

error:
    lyd_free_withsiblings(result);
    free(unres->node);
    free(unres->type);
    free(unres);
    va_end(ap);

    return NULL;
}
