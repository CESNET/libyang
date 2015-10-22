/**
 * @file xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML data parser for libyang
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
#include <ctype.h>
#include <errno.h>
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

#define LY_NSNC "urn:ietf:params:xml:ns:netconf:base:1.0"

/* does not log */
static struct lys_node *
xml_data_search_schemanode(struct lyxml_elem *xml, struct lys_node *start)
{
    struct lys_node *result, *aux;

    LY_TREE_FOR(start, result) {
        /* skip groupings */
        if (result->nodetype == LYS_GROUPING) {
            continue;
        }

        /* go into cases, choices, uses and in RPCs into input and output */
        if (result->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES | LYS_INPUT | LYS_OUTPUT)) {
            aux = xml_data_search_schemanode(xml, result->child);
            if (aux) {
                /* we have matching result */
                return aux;
            }
            /* else, continue with next schema node */
            continue;
        }

        /* match data nodes */
        if (result->name == xml->name) {
            /* names matches, what about namespaces? */
            if (result->module->ns == xml->ns->value) {
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
    struct lys_type *type, *stype;
    int resolve, found;

    assert(node && (node->schema->nodetype & (LYS_LEAFLIST | LYS_LEAF)) && xml && unres);

    stype = &((struct lys_node_leaf *)node->schema)->type;
    leaf->value_str = xml->content;
    xml->content = NULL;

    /* will be changed in case of union */
    leaf->value_type = stype->base;

    if ((options & LYD_OPT_FILTER) && !leaf->value_str) {
        /* no value in filter (selection) node -> nothing more is needed */
        return EXIT_SUCCESS;
    }

    if (options & (LYD_OPT_FILTER | LYD_OPT_EDIT)) {
        resolve = 0;
    } else {
        resolve = 1;
    }

    if ((stype->base == LY_TYPE_IDENT) || (stype->base == LY_TYPE_INST)) {
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

    if (stype->base == LY_TYPE_UNION) {
        found = 0;
        type = lyp_get_next_union_type(stype, NULL, &found);
        while (type) {
            leaf->value_type = type->base;

            /* in these cases we use JSON format */
            if ((type->base == LY_TYPE_IDENT) || (type->base == LY_TYPE_INST)) {
                xml->content = leaf->value_str;
                leaf->value_str = transform_xml2json(leaf->schema->module->ctx, xml->content, xml, 0);
                if (!leaf->value_str) {
                    leaf->value_str = xml->content;
                    xml->content = NULL;

                    found = 0;
                    type = lyp_get_next_union_type(stype, type, &found);
                    continue;
                }
            }

            if (!lyp_parse_value(leaf, type, resolve, unres, UINT_MAX)) {
                break;
            }

            if ((type->base == LY_TYPE_IDENT) || (type->base == LY_TYPE_INST)) {
                lydict_remove(leaf->schema->module->ctx, leaf->value_str);
                leaf->value_str = xml->content;
                xml->content = NULL;
            }

            found = 0;
            type = lyp_get_next_union_type(stype, type, &found);
        }

        if (!type) {
            LOGVAL(LYE_INVAL, LOGLINE(xml), (leaf->value_str ? leaf->value_str : ""), xml->name);
            return EXIT_FAILURE;
        }
    } else if (lyp_parse_value(leaf, stype, resolve, unres, LOGLINE(xml))) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static struct lyd_node *
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, struct lyd_node *parent, struct lyd_node *prev,
               int options, struct unres_data *unres)
{
    struct lyd_node *result = NULL, *diter;
    struct lys_node *schema = NULL;
    struct lyxml_attr *attr;
    struct lyxml_elem *first_child, *last_child, *child;
    int i, havechildren;

    if (!xml) {
        LOGINT;
        return NULL;
    }
    if (!xml->ns || !xml->ns->value) {
        LOGVAL(LYE_XML_MISS, LOGLINE(xml), "element's", "namespace");
        return NULL;
    }

    /* find schema node */
    if (!parent) {
        /* starting in root */
        for (i = 0; i < ctx->models.used; i++) {
            /* match data model based on namespace */
            if (ctx->models.list[i]->ns == xml->ns->value) {
                /* get the proper schema node */
                LY_TREE_FOR(ctx->models.list[i]->data, schema) {
                    if (schema->name == xml->name) {
                        break;
                    }
                }
                break;
            }
        }
    } else {
        /* parsing some internal node, we start with parent's schema pointer */
        schema = xml_data_search_schemanode(xml, parent->schema->child);
    }
    if (!schema) {
        if ((options & LYD_OPT_STRICT) || ly_ctx_get_module_by_ns(ctx, xml->ns->value, NULL)) {
            LOGVAL(LYE_INELEM, LOGLINE(xml), xml->name);
            return NULL;
        } else {
            goto siblings;
        }
    }

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
                LOGVAL(LYE_INATTR, LOGLINE(xml), "insert", schema->name);
                return NULL;
            }

            if (i) {
                LOGVAL(LYE_TOOMANY, LOGLINE(xml), "insert attributes", xml->name);
                return NULL;
            }
            if (!strcmp(attr->value, "first") || !strcmp(attr->value, "last")) {
                i = 1;
            } else if (!strcmp(attr->value, "before") || !strcmp(attr->value, "after")) {
                i = 2;
            } else {
                LOGVAL(LYE_INARG, LOGLINE(xml), attr->value, attr->name);
                return NULL;
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
                LOGVAL(LYE_INATTR, LOGLINE(xml), "value", schema->name);
                return NULL;
            }
            i++;
        }
        if (i == 2) {
            /* missing value attribute for "before" or "after" */
            LOGVAL(LYE_MISSATTR, LOGLINE(xml), "value", xml->name);
            return NULL;
        } else if (i > 3) {
            /* more than one instance of the value attribute */
            LOGVAL(LYE_TOOMANY, LOGLINE(xml), "value attributes", xml->name);
            return NULL;
        }
    }

    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
        result = calloc(1, sizeof *result);
        havechildren = 1;
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        result = calloc(1, sizeof(struct lyd_node_leaf_list));
        havechildren = 0;
        break;
    case LYS_ANYXML:
        result = calloc(1, sizeof(struct lyd_node_anyxml));
        havechildren = 0;
        break;
    default:
        LOGINT;
        return NULL;
    }
    result->parent = parent;
    if (parent && !parent->child) {
        parent->child = result;
    }
    if (prev) {
        result->prev = prev;
        prev->next = result;

        /* fix the "last" pointer */
        for (diter = prev; diter->prev != prev; diter = diter->prev);
        diter->prev = result;
    } else {
        result->prev = result;
    }
    result->schema = schema;

    if (lyv_data_context(result, options, LOGLINE(xml), unres)) {
        goto error;
    }

    /* type specific processing */
    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* type detection and assigning the value */
        if (xml_get_value(result, xml, options, unres)) {
            goto error;
        }
    } else if (schema->nodetype == LYS_ANYXML && !(options & LYD_OPT_FILTER)) {
        /* unlink xml children, they will be the anyxml value */
        first_child = last_child = NULL;
        LY_TREE_FOR(xml->child, child) {
            lyxml_unlink_elem(ctx, child, 1);
            if (!first_child) {
                first_child = child;
                last_child = child;
            } else {
                last_child->next = child;
                child->prev = last_child;
                last_child = child;
            }
        }
        if (first_child) {
            first_child->prev = last_child;
        }

        ((struct lyd_node_anyxml *)result)->value = first_child;
        /* we can safely continue with xml, it's like it was, only without children */
    }

    /* process children */
    if (havechildren && xml->child) {
        if (schema->nodetype & (LYS_RPC | LYS_NOTIF)) {
            xml_parse_data(ctx, xml->child, result, NULL, 0, unres);
        } else {
            xml_parse_data(ctx, xml->child, result, NULL, options, unres);
        }
        if (ly_errno) {
            goto error;
        }
    }

    result->attr = (struct lyd_attr *)xml->attr;
    xml->attr = NULL;

    /* various validation checks */
    ly_errno = 0;
    if (lyv_data_content(result, options, LOGLINE(xml), unres)) {
        if (ly_errno) {
            goto error;
        } else {
            goto cleargotosiblings;
        }
    }

siblings:
    /* process siblings */
    if (xml->next) {
        if (result) {
            xml_parse_data(ctx, xml->next, parent, result, options, unres);
        } else {
            xml_parse_data(ctx, xml->next, parent, prev, options, unres);
        }
        if (ly_errno) {
            goto error;
        }
    }

    return result;

error:

    /* cleanup */
    lyd_free(result);

    return NULL;

cleargotosiblings:

    /* remove the result ... */
    lyd_free(result);
    result = NULL;

    /* ... and then go to siblings label */
    goto siblings;
}

API struct lyd_node *
lyd_parse_xml(struct ly_ctx *ctx, struct lyxml_elem *root, int options)
{
    struct lyd_node *result, *next, *iter;
    struct unres_data *unres = NULL;

    if (!ctx || !root) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    unres = calloc(1, sizeof *unres);

    ly_errno = 0;
    result = xml_parse_data(ctx, root->child, NULL, NULL, options, unres);

    /* check leafrefs and/or instids if any */
    if (result && resolve_unres_data(unres)) {
        /* leafref & instid checking failed */
        LY_TREE_FOR_SAFE(result, next, iter) {
            lyd_free(iter);
        }
        result = NULL;
    }

    free(unres->node);
    free(unres->type);
#ifndef NDEBUG
    free(unres->line);
#endif
    free(unres);

    return result;
}
