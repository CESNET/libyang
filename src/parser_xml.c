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
xml_get_value(struct lyd_node *node, struct lyxml_elem *xml, int options, struct unres_data *unres)
{
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node;
    int resolve;

    assert(node && (node->schema->nodetype & (LYS_LEAFLIST | LYS_LEAF)) && xml && unres);

    leaf->value_str = xml->content;
    xml->content = NULL;

    /* will be changed in case of union */
    leaf->value_type = ((struct lys_node_leaf *)node->schema)->type.base;

    if ((options & LYD_OPT_FILTER) && !leaf->value_str) {
        /* no value in filter (selection) node -> nothing more is needed */
        return EXIT_SUCCESS;
    }

    if (options & (LYD_OPT_FILTER | LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
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

    if (lyp_parse_value(leaf, xml, resolve, unres, LOGLINE(xml))) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, const struct lys_node *schema_parent, struct lyd_node *parent,
               struct lyd_node *prev, int options, struct unres_data *unres, struct lyd_node **result)
{
    struct lyd_node *diter, *dlast;
    struct lys_node *schema = NULL;
    struct lyd_attr *dattr, *dattr_iter;
    struct lyxml_attr *attr;
    struct lyxml_elem *tmp_xml, *child, *next;
    int i, havechildren, r;
    int ret = 0;

    assert(xml);
    assert(result);
    *result = NULL;

    if (!xml->ns || !xml->ns->value) {
        if (options & LYD_OPT_STRICT) {
            LOGVAL(LYE_XML_MISS, LOGLINE(xml), LY_VLOG_XML, xml, "element's", "namespace");
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
            LOGVAL(LYE_INELEM, LOGLINE(xml), LY_VLOG_LYD, parent, xml->name);
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
    } else {
        (*result)->prev = *result;
    }
    (*result)->schema = schema;
    (*result)->validity = LYD_VAL_NOT;

    /* check insert attribute and its values */
    if (options & LYD_OPT_EDIT) {
        i = 0;
        for (attr = xml->attr; attr; attr = attr->next) {
            if (attr->type != LYXML_ATTR_STD || !attr->ns ||
                    strcmp(attr->name, "insert") || strcmp(attr->ns->value, LY_NSYANG)) {
                continue;
            }

            /* insert attribute present */
            if (!(schema->flags & LYS_USERORDERED)) {
                /* ... but it is not expected */
                LOGVAL(LYE_INATTR, LOGLINE(xml), LY_VLOG_LYD, (*result), "insert", schema->name);
                return -1;
            }

            if (i) {
                LOGVAL(LYE_TOOMANY, LOGLINE(xml), LY_VLOG_LYD, (*result), "insert attributes", xml->name);
                return -1;
            }
            if (!strcmp(attr->value, "first") || !strcmp(attr->value, "last")) {
                i = 1;
            } else if (!strcmp(attr->value, "before") || !strcmp(attr->value, "after")) {
                i = 2;
            } else {
                LOGVAL(LYE_INARG, LOGLINE(xml), LY_VLOG_LYD, (*result), attr->value, attr->name);
                return -1;
            }
        }

        for (attr = xml->attr; attr; attr = attr->next) {
            if (attr->type != LYXML_ATTR_STD || !attr->ns ||
                    strcmp(attr->name, "value") || strcmp(attr->ns->value, LY_NSYANG)) {
                continue;
            }

            /* the value attribute is present */
            if (i < 2) {
                /* but it shouldn't */
                LOGVAL(LYE_INATTR, LOGLINE(xml), LY_VLOG_LYD, (*result), "value", schema->name);
                return -1;
            }
            i++;
        }
        if (i == 2) {
            /* missing value attribute for "before" or "after" */
            LOGVAL(LYE_MISSATTR, LOGLINE(xml), LY_VLOG_LYD, (*result), "value", xml->name);
            return -1;
        } else if (i > 3) {
            /* more than one instance of the value attribute */
            LOGVAL(LYE_TOOMANY, LOGLINE(xml), LY_VLOG_LYD, (*result), "value attributes", xml->name);
            return -1;
        }
    }

    /* first part of validation checks */
    if (!(options & LYD_OPT_TRUSTED) && lyv_data_context(*result, options, LOGLINE(xml), unres)) {
        goto error;
    }

    /* type specific processing */
    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* type detection and assigning the value */
        if (xml_get_value(*result, xml, options, unres)) {
            goto error;
        }
    } else if (schema->nodetype == LYS_ANYXML && !(options & LYD_OPT_FILTER)) {
        /* HACK unlink xml children and link them to a separate copy of xml */
        tmp_xml = calloc(1, sizeof *tmp_xml);
        if (!tmp_xml) {
            LOGMEM;
            goto error;
        }
        memcpy(tmp_xml, xml, sizeof *tmp_xml);
        /* keep attributes in the original */
        tmp_xml->attr = NULL;
        /* increase reference counters on strings */
        tmp_xml->name = lydict_insert(ctx, tmp_xml->name, 0);
        tmp_xml->content = lydict_insert(ctx, tmp_xml->content, 0);
        xml->child = NULL;
        /* xml is correct now */

        LY_TREE_FOR(tmp_xml->child, child) {
            child->parent = tmp_xml;
        }
        /* children are correct now */

        /* unlink manually */
        tmp_xml->parent = NULL;
        tmp_xml->next = NULL;
        tmp_xml->prev = tmp_xml;

        /* just to correct namespaces */
        lyxml_unlink_elem(ctx, tmp_xml, 1);
        /* tmp_xml is correct now */

        ((struct lyd_node_anyxml *)*result)->value = tmp_xml;
        /* we can safely continue with xml, it's like it was, only without children */
    }

    for (attr = xml->attr; attr; attr = attr->next) {
        if (attr->type != LYXML_ATTR_STD) {
            continue;
        } else if (!attr->ns) {
            LOGWRN("Ignoring \"%s\" attribute in \"%s\" element.", attr->name, xml->name);
            continue;
        }

        dattr = malloc(sizeof *dattr);
        if (!dattr) {
            goto error;
        }
        dattr->next = NULL;
        dattr->name = attr->name;
        dattr->value = attr->value;
        attr->name = NULL;
        attr->value = NULL;

        dattr->module = (struct lys_module *)ly_ctx_get_module_by_ns(ctx, attr->ns->value, NULL);
        if (!dattr->module) {
            LOGWRN("Attribute \"%s\" from unknown schema (\"%s\") - skipping.", attr->name, attr->ns->value);
            free(dattr);
            continue;
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

    /* rest of validation checks */
    ly_errno = 0;
    if (!(options & LYD_OPT_TRUSTED) && lyv_data_content(*result, options, LOGLINE(xml), unres)) {
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
    const struct lys_node *rpc = NULL;
    struct lyd_node *result = NULL, *next, *iter, *last;
    struct lyxml_elem *xmlstart, *xmlelem, *xmlaux;

    ly_errno = LY_SUCCESS;

    if (!ctx || !root) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    if (!(*root)) {
        /* empty tree - no work is needed */
        lyd_validate(NULL, options, ctx);
        return NULL;
    }

    if (lyp_check_options(options)) {
        LOGERR(LY_EINVAL, "%s: Invalid options (multiple data type flags set).", __func__);
        return NULL;
    }

    va_start(ap, options);
    if (options & LYD_OPT_RPCREPLY) {
        rpc = va_arg(ap,  struct lys_node*);
        if (!rpc || (rpc->nodetype != LYS_RPC)) {
            LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
            goto cleanup;
        }
    }

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        goto cleanup;
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
            LY_TREE_FOR_SAFE(result, next, iter) {
                lyd_free(iter);
            }
            result = NULL;
            goto cleanup;
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

    /* check leafrefs and/or instids if any */
    if (result && resolve_unres_data(unres)) {
        /* leafref & instid checking failed */
        LY_TREE_FOR_SAFE(result, next, iter) {
            lyd_free(iter);
        }
        result = NULL;
    }

cleanup:
    if (unres) {
        free(unres->node);
        free(unres->type);
#ifndef NDEBUG
        free(unres->line);
#endif
        free(unres);
    }
    va_end(ap);

    return result;
}
