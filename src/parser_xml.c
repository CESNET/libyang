/**
 * @file xml.c
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
            if (ly_strequal(result->module->ns, xml->ns->value, 1)) {
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
xml_get_value(struct lyd_node *node, struct lyxml_elem *xml, int options)
{
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node;
    int resolve;

    assert(node && (node->schema->nodetype & (LYS_LEAFLIST | LYS_LEAF)) && xml);

    leaf->value_str = xml->content;
    xml->content = NULL;

    /* will be changed in case of union */
    leaf->value_type = ((struct lys_node_leaf *)node->schema)->type.base;

    if (options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
        resolve = 0;
    } else {
        resolve = 1;
    }

    if ((leaf->value_type == LY_TYPE_IDENT) || (leaf->value_type == LY_TYPE_INST)) {
        /* convert the path from the XML form using XML namespaces into the JSON format
         * using module names as namespaces
         */
        xml->content = leaf->value_str;
        leaf->value_str = transform_xml2json(leaf->schema->module->ctx, xml->content, xml, 1);
        lydict_remove(leaf->schema->module->ctx, xml->content);
        xml->content = NULL;
        if (!leaf->value_str) {
            return EXIT_FAILURE;
        }
    }

    if (lyp_parse_value(leaf, xml, resolve)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, const struct lys_node *schema_parent, struct lyd_node *parent,
               struct lyd_node *prev, int options, struct unres_data *unres, struct lyd_node **result)
{
    struct lyd_node *diter, *dlast, *first_sibling;
    struct lys_node *schema = NULL;
    struct lyd_attr *dattr, *dattr_iter;
    struct lyxml_attr *attr;
    struct lyxml_elem *child, *next;
    int i, havechildren, r, flag;
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
    if (schema_parent) {
        schema = xml_data_search_schemanode(xml, schema_parent->child, options);
    } else if (!parent) {
        /* starting in root */
        for (i = 0; i < ctx->models.used; i++) {
            /* match data model based on namespace */
            if (ly_strequal(ctx->models.list[i]->ns, xml->ns->value, 1)) {
                /* get the proper schema node */
                LY_TREE_FOR(ctx->models.list[i]->data, schema) {
                    /* skip nodes in module's data which are not expected here according to options' data type */
                    if (options & LYD_OPT_RPC) {
                        if (schema->nodetype != LYS_RPC) {
                            continue;
                        }
                    } else if (options & LYD_OPT_NOTIF) {
                        if (schema->nodetype != LYS_NOTIF) {
                            continue;
                        }
                    } else if (!(options & LYD_OPT_RPCREPLY)) {
                        /* rest of the data types except RPCREPLY which cannot be here */
                        if (schema->nodetype & (LYS_RPC | LYS_NOTIF)) {
                            continue;
                        }
                    }
                    if (ly_strequal(schema->name, xml->name, 1)) {
                        break;
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
    }

    /* create the element structure */
    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
        *result = calloc(1, sizeof **result);
        havechildren = 1;
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        *result = calloc(1, sizeof(struct lyd_node_leaf_list));
        havechildren = 0;
        break;
    case LYS_ANYXML:
        *result = calloc(1, sizeof(struct lyd_node_anyxml));
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

    (*result)->parent = parent;
    if (parent && !parent->child) {
        parent->child = *result;
    }
    if (prev) {
        (*result)->prev = prev;
        prev->next = *result;

        /* fix the "last" pointer */
        if (parent) {
            diter = parent->child;
        } else {
            for (diter = prev; diter->prev != prev; diter = diter->prev);
        }
        diter->prev = *result;
        first_sibling = diter;
    } else {
        (*result)->prev = *result;
        first_sibling = *result;
    }
    (*result)->schema = schema;
    (*result)->validity = LYD_VAL_NOT;
    if (resolve_applies_when(*result)) {
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
        i = 0;
        for (attr = xml->attr; attr; attr = attr->next) {
            if (attr->type != LYXML_ATTR_STD || !attr->ns) {
                /* not interesting attribute or namespace declaration */
                continue;
            }

            if (!strcmp(attr->name, "operation") && !strcmp(attr->ns->value, LY_NSNC)) {
                if (i & 0x10) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "operation attributes", xml->name);
                    return -1;
                }

                if (!strcmp(attr->value, "delete") || !strcmp(attr->value, "remove")) {
                    i |= 0x10;
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

                if (i & 0x01) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "insert attributes", xml->name);
                    return -1;
                }
                if (!strcmp(attr->value, "first") || !strcmp(attr->value, "last")) {
                    i |= 0x01;
                } else if (!strcmp(attr->value, "before") || !strcmp(attr->value, "after")) {
                    i |= 0x01 | 0x02;
                } else {
                    LOGVAL(LYE_INVALATTR, LY_VLOG_LYD, (*result), attr->value, attr->name);
                    return -1;
                }
                str = attr->name;
            } else if (!strcmp(attr->name, "value") && !strcmp(attr->ns->value, LY_NSYANG)) {
                if (i & 0x04) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "value attributes", xml->name);
                    return -1;
                } else if (schema->nodetype & LYS_LIST) {
                    LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), attr->name, schema->name);
                    return -1;
                }
                i |= 0x04;
                str = attr->name;
            } else if (!strcmp(attr->name, "key") && !strcmp(attr->ns->value, LY_NSYANG)) {
                if (i & 0x08) {
                    LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, (*result), "key attributes", xml->name);
                    return -1;
                } else if (schema->nodetype & LYS_LEAFLIST) {
                    LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), attr->name, schema->name);
                    return -1;
                }
                i |= 0x08;
                str = attr->name;
            }
        }

        /* report errors */
        if (i > 0x10 || (i && i < 0x10 &&
                (!(schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) || !(schema->flags & LYS_USERORDERED)))) {
            /* attributes in wrong elements */
            LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), str, xml->name);
            return -1;
        } else if (i == 3) {
            /* 0x01 | 0x02 - relative position, but value/key is missing */
            if (schema->nodetype & LYS_LIST) {
                LOGVAL(LYE_MISSATTR, LY_VLOG_LYD, (*result), "key", xml->name);
            } else { /* LYS_LEAFLIST */
                LOGVAL(LYE_MISSATTR, LY_VLOG_LYD, (*result), "value", xml->name);
            }
            return -1;
        } else if ((i & (0x04 | 0x08)) && !(i & 0x02)) {
            /* key/value without relative position */
            LOGVAL(LYE_INATTR, LY_VLOG_LYD, (*result), (i & 0x04) ? "value" : "key", schema->name);
            return -1;
        }
    }

    /* first part of validation checks */
    if (!(options & LYD_OPT_TRUSTED) && lyv_data_context(*result, options, unres)) {
        goto error;
    }

    /* type specific processing */
    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* type detection and assigning the value */
        if (xml_get_value(*result, xml, options)) {
            goto error;
        }
    } else if (schema->nodetype == LYS_ANYXML) {
        /* store children values */
        if (xml->child) {
            child = xml->child;
            /* manually unlink all siblings and correct namespaces */
            xml->child = NULL;
            LY_TREE_FOR(child, next) {
                next->parent = NULL;
                lyxml_correct_elem_ns(ctx, next, 1, 1);
            }

            ((struct lyd_node_anyxml *)*result)->xml_struct = 1;
            ((struct lyd_node_anyxml *)*result)->value.xml = child;
        } else {
            ((struct lyd_node_anyxml *)*result)->xml_struct = 0;
            ((struct lyd_node_anyxml *)*result)->value.str = lydict_insert(ctx, xml->content, 0);
        }
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
                r = xml_parse_data(ctx, child, NULL, *result, dlast, 0, unres, &diter);
            } else {
                r = xml_parse_data(ctx, child, NULL, *result, dlast, options, unres, &diter);
            }
            if (r) {
                goto error;
            } else if (options & LYD_OPT_DESTRUCT) {
                lyxml_free(ctx, child);
            }
            if (diter) {
                dlast = diter;
            }
        }
    }

    /* if we have empty non-presence container, we can remove it */
    if (!(options & LYD_OPT_KEEPEMPTYCONT) && schema->nodetype == LYS_CONTAINER && !(*result)->child &&
            !((struct lys_node_container *)schema)->presence) {
        goto clear;
    }

    /* rest of validation checks */
    ly_errno = 0;
    if (!(options & LYD_OPT_TRUSTED) &&
            (lyv_data_content(*result, options, unres) ||
             lyv_multicases(*result, NULL, first_sibling == *result ? NULL : first_sibling, 0, NULL))) {
        if (ly_errno) {
            goto error;
        } else {
            goto clear;
        }
    }

    /* validation successful */
    (*result)->validity = LYD_VAL_OK;

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
    int r;
    struct unres_data *unres = NULL;
    struct lys_node *rpc = NULL;
    struct lyd_node *result = NULL, *iter, *last;
    struct lyxml_elem *xmlstart, *xmlelem, *xmlaux;

    ly_errno = LY_SUCCESS;

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
        rpc = va_arg(ap,  struct lys_node*);
        if (!rpc || (rpc->nodetype != LYS_RPC)) {
            LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
            goto error;
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
    iter = result = last = NULL;

    LY_TREE_FOR_SAFE(xmlstart, xmlaux, xmlelem) {
        r = xml_parse_data(ctx, xmlelem, rpc, NULL, last, options, unres, &iter);
        if (r) {
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

    /* check for missing top level mandatory nodes */
    if (lyd_check_topmandatory(result, ctx, NULL, options)) {
        goto error;
    }

    /* add/validate default values, unres */
    if (lyd_validate_defaults_unres(&result, options, ctx, rpc, unres)) {
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
