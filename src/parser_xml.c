/**
 * @file parser_xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML data parser for libyang
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "dict.h"
#include "log.h"
#include "plugins_types.h"
#include "set.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "xml.h"

/**
 * @brief internal context for XML YANG data parser.
 *
 * The leading part is compatible with the struct lyxml_context
 */
struct lyd_xml_ctx {
    struct ly_ctx *ctx;              /**< libyang context */
    uint64_t line;                   /**< number of the line being currently processed */
    enum LYXML_PARSER_STATUS status; /**< status providing information about the next expected object in input data */
    struct ly_set elements;          /**< list of not-yet-closed elements */
    struct ly_set ns;                /**< handled with LY_SET_OPT_USEASLIST */

    uint16_t options;                /**< various @ref dataparseroptions. */
    uint16_t path_len;               /**< used bytes in the path buffer */
#define LYD_PARSER_BUFSIZE 4078
    char path[LYD_PARSER_BUFSIZE];   /**< buffer for the generated path */
};

/**
 * @brief Parse XML attributes of the XML element of YANG data.
 *
 * @param[in] ctx XML YANG data parser context.
 * @param[in,out] data Pointer to the XML string representation of the YANG data to parse.
 * @param[out] attributes Resulting list of the parsed attributes. XML namespace definitions are not parsed
 * as attributes, they are stored internally in the parser context.
 * @reutn LY_ERR value.
 */
static LY_ERR
lydxml_attributes(struct lyd_xml_ctx *ctx, const char **data, struct lyd_attr **attributes)
{
    LY_ERR ret = LY_SUCCESS;
    unsigned int u;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct lyd_attr *attr = NULL, *last = NULL;
    const struct lyxml_ns *ns;
    struct ly_set attr_prefixes = {0};
    struct attr_prefix_s {
        const char *prefix;
        size_t prefix_len;
    } *attr_prefix;
    struct lys_module *mod;

    while(ctx->status == LYXML_ATTRIBUTE &&
            lyxml_get_attribute((struct lyxml_context*)ctx, data, &prefix, &prefix_len, &name, &name_len) == LY_SUCCESS) {
        int dynamic = 0;
        char *buffer = NULL, *value;
        size_t buffer_size = 0, value_len;

        if (!name) {
            /* seems like all the attrributes were internally processed as namespace definitions */
            continue;
        }

        /* get attribute value */
        ret = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &value, &value_len, &dynamic);
        LY_CHECK_GOTO(ret, cleanup);

        attr = calloc(1, sizeof *attr);
        LY_CHECK_ERR_GOTO(!attr, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);

        attr->name = lydict_insert(ctx->ctx, name, name_len);
        /* auxiliary store the prefix information and wait with resolving prefix to the time when all the namespaces,
         * defined in this element, are parsed, so we will get the correct namespace for this prefix */
        attr_prefix = malloc(sizeof *attr_prefix);
        attr_prefix->prefix = prefix;
        attr_prefix->prefix_len = prefix_len;
        ly_set_add(&attr_prefixes, attr_prefix, LY_SET_OPT_USEASLIST);

        /* TODO process value */

        if (last) {
            last->next = attr;
        } else {
            (*attributes) = attr;
        }
        last = attr;
    }

    /* resolve annotation pointers in all the attributes */
    for (last = *attributes, u = 0; u < attr_prefixes.count && last; u++, last = last->next) {
        attr_prefix = (struct attr_prefix_s*)attr_prefixes.objs[u];
        ns = lyxml_ns_get((struct lyxml_context *)ctx, attr_prefix->prefix, attr_prefix->prefix_len);
        mod = ly_ctx_get_module_implemented_ns(ctx->ctx, ns->uri);

        /* TODO get annotation */
    }

cleanup:

    ly_set_erase(&attr_prefixes, free);
    return ret;
}

/**
 * @brief Parse XML elements as children YANG data node of the specified parent node.
 *
 * @param[in] ctx XML YANG data parser context.
 * @param[in] parent Parent node where the children are inserted. NULL in case of parsing top-level elements.
 * @param[in,out] data Pointer to the XML string representation of the YANG data to parse.
 * @param[out] node Resulting list of the parsed nodes.
 * @reutn LY_ERR value.
 */
static LY_ERR
lydxml_nodes(struct lyd_xml_ctx *ctx, struct lyd_node_inner *parent, const char **data, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct lyd_attr *attributes = NULL;
    const struct lyxml_ns *ns;
    const struct lysc_node *snode;
    struct lys_module *mod;
    unsigned int parents_count = ctx->elements.count;
    struct lyd_node *cur = NULL, *prev = NULL;

    (*node) = NULL;

    while(ctx->status == LYXML_ELEMENT) {
        ret = lyxml_get_element((struct lyxml_context *)ctx, data, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_GOTO(ret, cleanup);
        if (!name) {
            /* closing previous element */
            if (ctx->elements.count < parents_count) {
                /* all siblings parsed */
                break;
            } else {
                continue;
            }
        }
        attributes = NULL;
        LY_CHECK_GOTO(lydxml_attributes(ctx, data, &attributes), cleanup);
        ns = lyxml_ns_get((struct lyxml_context *)ctx, prefix, prefix_len);
        if (!ns) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Unknown XML prefix \"%*.s\".", prefix_len, prefix);
            goto cleanup;
        }
        mod = ly_ctx_get_module_implemented_ns(ctx->ctx, ns->uri);
        if (!mod) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "No module with namespace \"%s\" in the context.", ns->uri);
            goto cleanup;
        }
        snode = lys_child(parent ? parent->schema : NULL, mod, name, name_len, 0, (ctx->options & LYD_OPT_RPCREPLY) ? LYS_GETNEXT_OUTPUT : 0);
        if (!snode) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Element \"%.*s\" not found in the \"%s\" module.", name_len, name, mod->name);
            goto cleanup;
        }

        /* allocate new node */
        switch (snode->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
            cur = calloc(1, sizeof(struct lyd_node_inner));
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            cur = calloc(1, sizeof(struct lyd_node_term));
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            cur = calloc(1, sizeof(struct lyd_node_any));
            break;
        /* TODO LYS_ACTION, LYS_NOTIF */
        default:
            LOGINT(ctx->ctx);
            goto cleanup;
        }
        if (!(*node)) {
            (*node) = cur;
        }
        cur->schema = snode;
        cur->parent = parent;
        if (parent) {
            parent->child->prev = cur;
        } else if (prev) {
            struct lyd_node *iter;
            for (iter = prev; iter->prev->next; iter = iter->prev);
            iter->prev = cur;
        }
        if (prev) {
            cur->prev = prev;
            prev->next = cur;
        } else {
            cur->prev = cur;
        }
        prev = cur;
        cur->attr = attributes;
        attributes = NULL;

        if (snode->nodetype & LYD_NODE_TERM) {
            int dynamic = 0;
            char *buffer = NULL, *value;
            size_t buffer_size = 0, value_len;

            if (ctx->status == LYXML_ELEM_CONTENT) {
                /* get the value */
                LY_ERR r = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &value, &value_len, &dynamic);
                if (r == LY_EINVAL) {
                    /* just indentation of a child element found */
                    LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_SYNTAX, "Child element inside terminal node \"%s\" found.", cur->schema->name);
                    goto cleanup;
                }
            } else {
                /* no content - validate empty value */
                value = "";
                value_len = 0;
            }
            ret = lyd_value_validate((struct lyd_node_term*)cur, value, value_len,
                                      LY_TYPE_VALIDATE_CANONIZE | (dynamic ? LY_TYPE_VALIDATE_DYNAMIC : 0));
            LY_CHECK_GOTO(ret, cleanup);
        } else if (snode->nodetype & LYD_NODE_INNER) {
            int dynamic = 0;
            char *buffer = NULL, *value;
            size_t buffer_size = 0, value_len;

            if (ctx->status == LYXML_ELEM_CONTENT) {
                LY_ERR r = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &value, &value_len, &dynamic);
                if (r != LY_EINVAL) {
                    LOGINT(ctx->ctx);
                    goto cleanup;
                }
            }
            if (ctx->status == LYXML_ELEMENT) {
                ret = lydxml_nodes(ctx, (struct lyd_node_inner*)cur, data, lyd_node_children_p(cur));
                LY_CHECK_GOTO(ret, cleanup);
            }
        }
        /* TODO anyxml/anydata */
    }

cleanup:
    lyd_free_attr(ctx->ctx, attributes, 1);
    return ret;
}

LY_ERR
lyd_parse_xml(struct ly_ctx *ctx, const char *data, int options, struct lyd_node **result)
{
    LY_ERR ret;
    struct lyd_xml_ctx xmlctx = {0};

    xmlctx.options = options;
    xmlctx.ctx = ctx;
    xmlctx.line = 1;

    ret = lydxml_nodes(&xmlctx, NULL, &data, result);
    if (ret) {
        lyd_free_all(*result);
        *result = NULL;
    }
    lyxml_context_clear((struct lyxml_context*)&xmlctx);
    return ret;
}
