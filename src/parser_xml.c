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
    struct ly_set incomplete_type_validation; /**< set of nodes validated with LY_EINCOMPLETE result */
};

/**
 * @brief XML-parser's implementation of ly_type_resolve_prefix() callback to provide mapping between prefixes used in the values to the schema
 * via XML namespaces.
 */
static const struct lys_module *
lydxml_resolve_prefix(struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void *parser)
{
    const struct lyxml_ns *ns;
    struct lyxml_context *xmlctx = (struct lyxml_context*)parser;

    ns = lyxml_ns_get(xmlctx, prefix, prefix_len);
    if (!ns) {
        return NULL;
    }

    return ly_ctx_get_module_implemented_ns(ctx, ns->uri);
}

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
    struct lyd_node *cur = NULL, *prev = NULL, *last = NULL;

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
        if (ctx->status == LYXML_ATTRIBUTE) {
            LY_CHECK_GOTO(lydxml_attributes(ctx, data, &attributes), cleanup);
        }

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
        case LYS_ACTION:
            if ((ctx->options & LYD_OPT_TYPEMASK) != LYD_OPT_RPC) {
                LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_RESTRICTION, "Unexpected RPC/action element \"%.*s\" in %s data set.",
                       name_len, name, lyd_parse_options_type2str(ctx->options & LYD_OPT_TYPEMASK));
                goto cleanup;
            }
            cur = calloc(1, sizeof(struct lyd_node_inner));
            break;
        case LYS_NOTIF:
            if ((ctx->options & LYD_OPT_TYPEMASK) != LYD_OPT_RPC) {
                LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_RESTRICTION, "Unexpected Notification element \"%.*s\" in %s data set.",
                       name_len, name, lyd_parse_options_type2str(ctx->options));
                goto cleanup;
            }
            cur = calloc(1, sizeof(struct lyd_node_inner));
            break;
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
        default:
            LOGINT(ctx->ctx);
            goto cleanup;
        }
        if (!(*node)) {
            (*node) = cur;
        }
        last = cur;
        cur->schema = snode;
        cur->prev = cur;
        cur->parent = parent;
        if (parent) {
            if (prev && cur->schema->nodetype == LYS_LEAF && (cur->schema->flags & LYS_KEY)) {
                /* it is key and we need to insert it into a correct place */
                struct lysc_node_leaf **keys = ((struct lysc_node_list*)parent->schema)->keys;
                unsigned int cur_index, key_index;
                struct lyd_node *key;

                for (cur_index = 0; keys[cur_index] != (struct lysc_node_leaf*)cur->schema; ++cur_index);
                for (key = prev; !(key->schema->flags & LYS_KEY) && key->prev != prev; key = key->prev);
                for (; key->schema->flags & LYS_KEY; key = key->prev) {
                    for (key_index = 0; keys[key_index] != (struct lysc_node_leaf*)key->schema; ++key_index);
                    if (key_index < cur_index) {
                        /* cur key is supposed to be placed after the key */
                        cur->next = key->next;
                        cur->prev = key;
                        key->next = cur;
                        if (cur->next) {
                            cur->next->prev = cur;
                        } else {
                            parent->child->prev = cur;
                        }
                        break;
                    }
                    if (key->prev == prev) {
                        /* current key is supposed to be the first child from the current children */
                        key = NULL;
                        break;
                    }
                }
                if (!key || !(key->schema->flags & LYS_KEY)) {
                    /* current key is supposed to be the first child from the current children */
                    cur->next = parent->child;
                    cur->prev = parent->child->prev;
                    parent->child->prev = cur;
                    parent->child = cur;
                }
                if (cur->next) {
                    last = prev;
                    if (ctx->options & LYD_OPT_STRICT) {
                        LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_RESTRICTION, "Invalid position of the key \"%.*s\" in a list.",
                               name_len, name);
                        goto cleanup;
                    } else {
                        LOGWRN(ctx->ctx, "Invalid position of the key \"%.*s\" in a list.", name_len, name);
                    }
                }
            } else {
                /* last child of the parent */
                if (prev) {
                    parent->child->prev = cur;
                    prev->next = cur;
                    cur->prev = prev;
                }
            }
        } else {
            /* top level */
            if (prev) {
                /* last top level node */
                struct lyd_node *iter;
                for (iter = prev; iter->prev->next; iter = iter->prev);
                iter->prev = cur;
                prev->next = cur;
                cur->prev = prev;
            } /* first top level node - nothing more to do */
        }
        prev = last;
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
            ret = lyd_value_parse((struct lyd_node_term*)cur, value, value_len, dynamic, 0, lydxml_resolve_prefix, ctx, LYD_XML, NULL);
            if (ret == LY_EINCOMPLETE) {
                ly_set_add(&ctx->incomplete_type_validation, cur, LY_SET_OPT_USEASLIST);
            } else if (ret) {
                if (dynamic){
                    free(value);
                }
                goto cleanup;
            }
        } else if (snode->nodetype & LYD_NODE_INNER) {
            int dynamic = 0;
            char *buffer = NULL, *value;
            size_t buffer_size = 0, value_len;

            if (ctx->status == LYXML_ELEM_CONTENT) {
                LY_ERR r = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &value, &value_len, &dynamic);
                if (r != LY_EINVAL && (r != LY_SUCCESS || value_len != 0)) {
                    LOGINT(ctx->ctx);
                    goto cleanup;
                }
            }
            /* process children */
            if (ctx->status == LYXML_ELEMENT && parents_count != ctx->elements.count) {
                ret = lydxml_nodes(ctx, (struct lyd_node_inner*)cur, data, lyd_node_children_p(cur));
                LY_CHECK_GOTO(ret, cleanup);
            }
        } else if (snode->nodetype & LYD_NODE_ANY) {
            unsigned int cur_element_index = ctx->elements.count;
            const char *start = *data, *stop;
            const char *p, *n;
            size_t p_len, n_len;

            /* skip children data and store them as a string */
            while (cur_element_index <= ctx->elements.count) {
                switch (ctx->status) {
                case LYXML_ELEMENT:
                    ret = lyxml_get_element((struct lyxml_context *)ctx, data, &p, &p_len, &n, &n_len);
                    break;
                case LYXML_ATTRIBUTE:
                    lyxml_get_attribute((struct lyxml_context*)ctx, data, &p, &p_len, &n, &n_len);
                    break;
                case LYXML_ELEM_CONTENT:
                case LYXML_ATTR_CONTENT:
                    ret = lyxml_get_string((struct lyxml_context *)ctx, data, NULL, NULL, NULL, NULL, NULL);
                    if (ret == LY_EINVAL) {
                        /* not an error, just incorrect XML parser status */
                        ret = LY_SUCCESS;
                    }
                    break;
                case LYXML_END:
                    /* unexpected end of data */
                    LOGINT(ctx->ctx);
                    goto cleanup;
                }
                LY_CHECK_GOTO(ret, cleanup);
            }
            /* data now points after the anydata's closing element tag, we need just end of its content */
            for (stop = *data - 1; *stop != '<'; --stop);

            ((struct lyd_node_any*)cur)->value_type = LYD_ANYDATA_XML;
            ((struct lyd_node_any*)cur)->value.xml = lydict_insert(ctx->ctx, start, stop - start);
        }

        /* calculate the hash and insert it into parent (list with keys is handled when its keys are inserted) */
        lyd_hash(cur);
        lyd_insert_hash(cur);

        /* if we have empty non-presence container, we keep it, but mark it as default */
        if (cur->schema->nodetype == LYS_CONTAINER && !((struct lyd_node_inner*)cur)->child &&
                !cur->attr && !(((struct lysc_node_container*)cur->schema)->flags & LYS_PRESENCE)) {
            cur->flags |= LYD_DEFAULT;
        }

        /* TODO context validation */
    }

    /* TODO add missing siblings default elements */

cleanup:
    lyd_free_attr(ctx->ctx, attributes, 1);
    return ret;
}

LY_ERR
lyd_parse_xml(struct ly_ctx *ctx, const char *data, int options, const struct lyd_node **trees, struct lyd_node **result)
{
    LY_ERR ret;
    struct lyd_node_inner *parent = NULL;
    struct lyd_xml_ctx xmlctx = {0};

    xmlctx.options = options;
    xmlctx.ctx = ctx;
    xmlctx.line = 1;

    /* init */
    *result = NULL;

    if (!data || !data[0]) {
        goto no_data;
    }

    if (options & LYD_OPT_RPCREPLY) {
        /* TODO prepare container for RPC reply, for which we need RPC
         * - prepare *result as top-level node
         * - prepare parent as the RPC/action node */
        (void)trees;
    }

    ret = lydxml_nodes(&xmlctx, parent, &data, *result ? &parent->child : result);
    if (ret) {
        lyd_free_all(*result);
        *result = NULL;
    } else {
        /* finish incompletely validated terminal values */
        for (unsigned int u = 0; u < xmlctx.incomplete_type_validation.count; u++) {
            struct lyd_node_term *node = (struct lyd_node_term*)xmlctx.incomplete_type_validation.objs[u];
            const struct lyd_node **result_trees = NULL;

            /* prepare sized array for validator */
            if (*result) {
                result_trees = lyd_trees_new(1, *result);
            }
            /* validate and store the value of the node */
            ret = lyd_value_parse(node, node->value.canonized, node->value.canonized ? strlen(node->value.canonized) : 0, 0, 1,
                                  lydxml_resolve_prefix, ctx, LYD_XML, result_trees);
            lyd_trees_free(result_trees, 0);
            if (ret) {
                lyd_free_all(*result);
                *result = NULL;
                break;
            }
        }

        if (!(*result)) {
no_data:
            /* no data */
            if (options & (LYD_OPT_RPC | LYD_OPT_NOTIF)) {
                /* error, top level node identify RPC and Notification */
                LOGERR(ctx, LY_EINVAL, "Invalid input data of data parser - expected %s which cannot be empty.",
                       lyd_parse_options_type2str(options));
            } else {
                /* others - no work is needed, just check for missing mandatory nodes */
                /* TODO lyd_validate(&result, options, ctx); */
            }
        }
    }

    ly_set_erase(&xmlctx.incomplete_type_validation, NULL);
    lyxml_context_clear((struct lyxml_context*)&xmlctx);
    return ret;
}
