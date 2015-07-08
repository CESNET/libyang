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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../libyang.h"
#include "../common.h"
#include "../context.h"
#include "../xml.h"

#define LY_NSNC "urn:ietf:params:xml:ns:netconf:base:1.0"

struct ly_mnode *
xml_data_search_schemanode(struct lyxml_elem *xml, struct ly_mnode *start)
{
    struct ly_mnode *result, *aux;

    LY_TREE_FOR(start, result) {
        /* skip groupings */
        if (result->nodetype == LY_NODE_GROUPING) {
            continue;
        }

        /* go into cases, choices, uses */
        if (result->nodetype & (LY_NODE_CHOICE | LY_NODE_CASE | LY_NODE_USES)) {
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

struct lyd_node *
xml_parse_data(struct ly_ctx *ctx, struct lyxml_elem *xml, struct lyd_node *parent, struct lyd_node *prev)
{
    struct lyd_node *result, *aux;
    struct ly_mnode *schema = NULL;
    int i;

    if (!xml) {
        return NULL;
    }
    if (!xml->ns || !xml->ns->value) {
        LOGVAL(VE_XML_MISS, LOGLINE(xml), "element's", "namespace");
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
        LOGVAL(VE_INSTMT, LOGLINE(xml), xml->name);
        return NULL;
    }

    /* TODO: fit this into different types of nodes */
    switch (schema->nodetype) {
    case LY_NODE_LIST:
        result = calloc(1, sizeof(struct lyd_node_list));
        break;
    default:
        result = calloc(1, sizeof *result);
    }
    result->parent = parent;
    result->prev = prev;
    result->schema = schema;

    /* type specific processing */
    if (schema->nodetype == LY_NODE_LIST) {
        /* pointers to next and previous instances of the same list */
        for (aux = result->prev; aux; aux = aux->prev) {
            if (aux->schema == result->schema) {
                /* instances of the same list */
                ((struct lyd_node_list *)aux)->lnext = (struct lyd_node_list *)result;
                ((struct lyd_node_list *)result)->lprev = (struct lyd_node_list *)aux;
                break;
            }
        }
    }

    /* process children */
    if (xml->child) {
        result->child = xml_parse_data(ctx, xml->child, result, NULL);
        if (!result->child) {
            free(result);
            return NULL;
        }

    }

    /* process siblings */
    if (xml->next) {
        result->next = xml_parse_data(ctx, xml->next, parent, result);
        if (!result->next) {
            free(result);
            return NULL;
        }
    }

    /* fix the "last" pointer */
    if (!result->prev) {
        for (aux = result; aux->next; aux = aux->next);
        result->prev = aux;
    }
    return result;
}

struct lyd_node *
xml_read_data(struct ly_ctx *ctx, const char *data)
{
    struct lyxml_elem *xml;
    struct lyd_node *result;

    xml = lyxml_read(ctx, data, 0);
    if (!xml) {
        return NULL;
    }

    /* check the returned data - the root must be config or data in NETCONF namespace */
    if (!xml->ns || strcmp(xml->ns->value, LY_NSNC) || (strcmp(xml->name, "data") && strcmp(xml->name, "config"))) {
        LOGERR(LY_EINVAL, "XML data parser expect <data> or <config> root in \"%s\" namespace.", LY_NSNC);
        return NULL;
    }

    result = xml_parse_data(ctx, xml->child, NULL, NULL);
    lyxml_free_elem(ctx, xml);

    return result;
}
