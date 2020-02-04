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
#include "plugins_exts_internal.h"
#include "validation.h"

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
    struct ly_set incomplete_type_validation_attrs; /**< set of attributes validated with LY_EINCOMPLETE result */
    struct ly_set when_check;        /**< set of nodes with "when" conditions */
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

struct attr_data_s {
    const char *prefix;
    const char *name;
    char *value;
    size_t prefix_len;
    size_t name_len;
    size_t value_len;
    int dynamic;
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
lydxml_attributes_parse(struct lyd_xml_ctx *ctx, const char **data, struct ly_set *attrs_data)
{
    LY_ERR ret = LY_SUCCESS;
    unsigned int u;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct attr_data_s *attr_data;

    while(ctx->status == LYXML_ATTRIBUTE &&
            lyxml_get_attribute((struct lyxml_context*)ctx, data, &prefix, &prefix_len, &name, &name_len) == LY_SUCCESS) {
        char *buffer = NULL;
        size_t buffer_size = 0;

        if (!name) {
            /* seems like all the attrributes were internally processed as namespace definitions */
            continue;
        }

        /* auxiliary store the prefix information and value string, because we have to wait with resolving prefix
         * to the time when all the namespaces, defined in this element, are parsed. With the prefix we can find the
         * annotation definition for the attribute and correctly process the value */
        attr_data = malloc(sizeof *attr_data);
        attr_data->prefix = prefix;
        attr_data->name = name;
        attr_data->prefix_len = prefix_len;
        attr_data->name_len = name_len;
        ret = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &attr_data->value, &attr_data->value_len, &attr_data->dynamic);
        LY_CHECK_ERR_GOTO(ret, free(attr_data), error);
        ly_set_add(attrs_data, attr_data, LY_SET_OPT_USEASLIST);
    }

    return LY_SUCCESS;

error:
    for (u = 0; u < attrs_data->count; ++u) {
        if (((struct attr_data_s*)attrs_data->objs[u])->dynamic) {
            free(((struct attr_data_s*)attrs_data->objs[u])->value);
        }
    }
    ly_set_erase(attrs_data, free);
    return ret;
}

static LY_ERR
lydxml_attributes(struct lyd_xml_ctx *ctx, struct ly_set *attrs_data, struct lyd_node *parent)
{
    LY_ERR ret = LY_EVALID, rc;
    struct lyd_attr *attr = NULL, *last = NULL;
    const struct lyxml_ns *ns;
    struct lys_module *mod;

    for (unsigned int u = 0; u < attrs_data->count; ++u) {
        unsigned int v;
        struct lysc_ext_instance *ant = NULL;
        struct attr_data_s *attr_data = (struct attr_data_s*)attrs_data->objs[u];

        if (!attr_data->prefix_len) {
            /* in XML, all attributes must be prefixed
             * TODO exception for NETCONF filters which are supposed to map to the ietf-netconf without prefix */
            if (ctx->options & LYD_OPT_STRICT) {
                LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Missing mandatory prefix for XML attribute \"%.*s\".",
                       attr_data->name_len, attr_data->name);
            }
skip_attr:
            if (attr_data->dynamic) {
                free(attr_data->value);
                attr_data->dynamic = 0;
            }
            continue;
        }

        /* get namespace of the attribute to find its annotation definition */
        ns = lyxml_ns_get((struct lyxml_context *)ctx, attr_data->prefix, attr_data->prefix_len);
        if (!ns) {
            /* unknown namespace, ignore the attribute */
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", attr_data->prefix_len, attr_data->prefix);
            goto cleanup;
        }
        mod = ly_ctx_get_module_implemented_ns(ctx->ctx, ns->uri);
        if (!mod) {
            /* module is not implemented or not present in the schema */
            if (ctx->options & LYD_OPT_STRICT) {
                LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE,
                       "Unknown (or not implemented) YANG module with namespace \"%s\" for attribute \"%.*s%s%.*s\".",
                       ns, attr_data->prefix_len, attr_data->prefix, attr_data->prefix_len ? ":" : "", attr_data->name_len, attr_data->name);
            }
            goto skip_attr;
        }

        LY_ARRAY_FOR(mod->compiled->exts, v) {
            if (mod->compiled->exts[v].def->plugin == lyext_plugins_internal[LYEXT_PLUGIN_INTERNAL_ANNOTATION].plugin &&
                    !ly_strncmp(mod->compiled->exts[v].argument, attr_data->name, attr_data->name_len)) {
                /* we have the annotation definition */
                ant = &mod->compiled->exts[v];
                break;
            }
        }
        if (!ant) {
            /* attribute is not defined as a metadata annotation (RFC 7952) */
            if (ctx->options & LYD_OPT_STRICT) {
                LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Annotation definition for attribute \"%s:%.*s\" not found.",
                       mod->name, attr_data->name_len, attr_data->name);
            }
            goto skip_attr;
        }

        attr = calloc(1, sizeof *attr);
        LY_CHECK_ERR_GOTO(!attr, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);
        attr->parent = parent;
        attr->annotation = ant;
        rc = lyd_value_parse_attr(attr, attr_data->value, attr_data->value_len, attr_data->dynamic, 0, lydxml_resolve_prefix, ctx, LYD_XML, NULL);
        if (rc == LY_EINCOMPLETE) {
            ly_set_add(&ctx->incomplete_type_validation_attrs, attr, LY_SET_OPT_USEASLIST);
        } else if (rc) {
            ret = rc;
            free(attr);
            goto cleanup;
        }
        attr_data->dynamic = 0; /* value eaten by lyd_value_parse_attr() */
        attr->name = lydict_insert(ctx->ctx, attr_data->name, attr_data->name_len);

        if (last) {
            last->next = attr;
        } else {
            parent->attr = attr;
        }
        last = attr;
    }
    ret = LY_SUCCESS;

cleanup:

    for (unsigned int u = 0; u < attrs_data->count; ++u) {
        if (((struct attr_data_s*)attrs_data->objs[u])->dynamic) {
            free(((struct attr_data_s*)attrs_data->objs[u])->value);
        }
    }
    ly_set_erase(attrs_data, free);

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
    struct ly_set attrs_data = {0};
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
        if (ctx->status == LYXML_ATTRIBUTE) {
            LY_CHECK_GOTO(lydxml_attributes_parse(ctx, data, &attrs_data), error);
        }

        ns = lyxml_ns_get((struct lyxml_context *)ctx, prefix, prefix_len);
        if (!ns) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", prefix_len, prefix);
            goto error;
        }
        mod = ly_ctx_get_module_implemented_ns(ctx->ctx, ns->uri);
        if (!mod) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "No module with namespace \"%s\" in the context.", ns->uri);
            goto error;
        }
        snode = lys_child(parent ? parent->schema : NULL, mod, name, name_len, 0, 0);
        if (!snode) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Element \"%.*s\" not found in the \"%s\" module.", name_len, name, mod->name);
            goto error;
        }

        /* allocate new node */
        switch (snode->nodetype) {
        case LYS_ACTION:
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_DATA, "Unexpected RPC/action element \"%.*s\".",
                   name_len, name);
            goto error;
            /*cur = calloc(1, sizeof(struct lyd_node_inner));
            break;*/
        case LYS_NOTIF:
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_DATA, "Unexpected notification element \"%.*s\".",
                   name_len, name);
            goto error;
            /*cur = calloc(1, sizeof(struct lyd_node_inner));
            break;*/
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
            ret = LY_EINT;
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
                struct lysc_node *key_s;
                unsigned int cur_index, key_index;
                struct lyd_node *key;

                for (cur_index = 0, key_s = ((struct lysc_node_list*)parent->schema)->child;
                        key_s && key_s != cur->schema;
                        ++cur_index, key_s = key_s->next);
                for (key = prev;
                        !(key->schema->flags & LYS_KEY) && key->prev != prev;
                        key = key->prev);
                for (; key->schema->flags & LYS_KEY; key = key->prev) {
                    for (key_index = 0, key_s = ((struct lysc_node_list*)parent->schema)->child;
                            key_s && key_s != key->schema;
                            ++key_index, key_s = key_s->next);
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
                        LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_DATA, "Invalid position of the key \"%.*s\" in a list.",
                               name_len, name);
                        goto error;
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
        LY_CHECK_GOTO(ret = lydxml_attributes(ctx, &attrs_data, cur), cleanup);

        if (snode->nodetype & LYD_NODE_TERM) {
            int dynamic = 0;
            char *buffer = NULL, *value;
            size_t buffer_size = 0, value_len;

            if (ctx->status == LYXML_ELEM_CONTENT) {
                /* get the value */
                ret = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &value, &value_len, &dynamic);
                if (ret == LY_EINVAL) {
                    /* just indentation of a child element found */
                    LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_SYNTAX, "Child element inside terminal node \"%s\" found.", cur->schema->name);
                    goto cleanup;
                }
                ret = LY_SUCCESS;
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
                    ret = LY_EINT;
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
            while (ctx->status != LYXML_END && cur_element_index <= ctx->elements.count) {
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
                    /* end of data */
                    LOGINT(ctx->ctx);
                    ret = LY_EINT;
                    goto cleanup;
                }
                LY_CHECK_GOTO(ret, cleanup);
            }
            ((struct lyd_node_any*)cur)->value_type = LYD_ANYDATA_XML;
            if (start != *data) {
                /* data now points after the anydata's closing element tag, we need just end of its content */
                for (stop = *data - 1; *stop != '<'; --stop);
                ((struct lyd_node_any*)cur)->value.xml = lydict_insert(ctx->ctx, start, stop - start);
            }
        }

        /* remember we need to evaluate this node's when */
        if (!(snode->nodetype & (LYS_ACTION | LYS_NOTIF)) && snode->when) {
            ly_set_add(&ctx->when_check, cur, LY_SET_OPT_USEASLIST);
        }

        /* calculate the hash and insert it into parent (list with keys is handled when its keys are inserted) */
        lyd_hash(cur);
        lyd_insert_hash(cur);

        /* if we have empty non-presence container, we keep it, but mark it as default */
        if (cur->schema->nodetype == LYS_CONTAINER && !((struct lyd_node_inner*)cur)->child &&
                !cur->attr && !(((struct lysc_node_container*)cur->schema)->flags & LYS_PRESENCE)) {
            cur->flags |= LYD_DEFAULT;
        }
    }

    /* TODO add missing siblings default elements */

cleanup:
    for (unsigned int u = 0; u < attrs_data.count; ++u) {
        if (((struct attr_data_s*)attrs_data.objs[u])->dynamic) {
            free(((struct attr_data_s*)attrs_data.objs[u])->value);
        }
    }
    ly_set_erase(&attrs_data, free);
    return ret;

error:
    ret = LY_EVALID;
    goto cleanup;
}

LY_ERR
lyd_parse_xml(struct ly_ctx *ctx, const char *data, int options, struct lyd_node **result)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node_inner *parent = NULL;
    const struct lyd_node **result_trees = NULL;
    struct lyd_xml_ctx xmlctx = {0};

    xmlctx.options = options;
    xmlctx.ctx = ctx;
    xmlctx.line = 1;

    /* init */
    *result = NULL;

#if 0
    if (options & LYD_OPT_RPCREPLY) {
        /* prepare container for RPC reply, for which we need RPC
         * - prepare *result as top-level node
         * - prepare parent as the RPC/action node */
        const struct lyd_node *action;
        for (action = trees[0]; action && action->schema->nodetype != LYS_ACTION; action = lyd_node_children(action)) {
            /* skip list's keys */
            for ( ;action && action->schema->nodetype == LYS_LEAF; action = action->next);
            if (action && action->schema->nodetype == LYS_ACTION) {
                break;
            }
        }
        if (!action) {
            LOGERR(ctx, LY_EINVAL, "Data parser invalid argument trees - the first item in the array must be the RPC/action request when parsing %s.",
                   lyd_parse_options_type2str(options));
            return LY_EINVAL;
        }
        parent = (struct lyd_node_inner*)lyd_dup(action, NULL, LYD_DUP_WITH_PARENTS);
        LY_CHECK_ERR_RET(!parent, LOGERR(ctx, ly_errcode(ctx), "Unable to duplicate RPC/action container for RPC/action reply."), ly_errcode(ctx));
        for (*result = (struct lyd_node*)parent; (*result)->parent; *result = (struct lyd_node*)(*result)->parent);
    }
#endif

    if (!data || !data[0]) {
        /* no data - just check for missing mandatory nodes */
        goto validation;
    }

    ret = lydxml_nodes(&xmlctx, parent, &data, *result ? &parent->child : result);
    LY_CHECK_GOTO(ret, cleanup);

    /* prepare sized array for validator */
    if (*result) {
        result_trees = lyd_trees_new(1, *result);
    }

    /* finish incompletely validated terminal values/attributes and when conditions */
    ret = lyd_validate_unres(&xmlctx.incomplete_type_validation, &xmlctx.incomplete_type_validation_attrs,
                             &xmlctx.when_check, LYD_XML, lydxml_resolve_prefix, ctx, result_trees);
    LY_CHECK_GOTO(ret, cleanup);

validation:
#if 0
    if ((!(*result) || (parent && !parent->child)) && (options & (LYD_OPT_RPC | LYD_OPT_NOTIF))) {
        /* error, missing top level node identify RPC and Notification */
        LOGERR(ctx, LY_EINVAL, "Invalid input data of data parser - expected %s which cannot be empty.",
               lyd_parse_options_type2str(options));
        ret = LY_EINVAL;
        goto cleanup;
    }
#endif

    /* context node and other validation tasks that depend on other data nodes */
    ret = lyd_validate_data(result_trees, NULL, 0, ctx, options);
    LY_CHECK_GOTO(result, cleanup);

cleanup:
    ly_set_erase(&xmlctx.incomplete_type_validation, NULL);
    ly_set_erase(&xmlctx.incomplete_type_validation_attrs, NULL);
    ly_set_erase(&xmlctx.when_check, NULL);
    lyxml_context_clear((struct lyxml_context*)&xmlctx);
    lyd_trees_free(result_trees, 0);
    if (ret) {
        lyd_free_all(*result);
        *result = NULL;
    }
    return ret;
}
