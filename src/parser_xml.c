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
#include <assert.h>

#include "context.h"
#include "dict.h"
#include "log.h"
#include "plugins_types.h"
#include "set.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "xml.h"
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
lydxml_attributes(struct lyd_xml_ctx *ctx, struct ly_set *attrs_data, const struct lysc_node *sparent, struct lyd_attr **attr)
{
    LY_ERR ret = LY_EVALID, rc;
    const struct lyxml_ns *ns;
    struct lys_module *mod;

    for (unsigned int u = 0; u < attrs_data->count; ++u) {
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
                       ns, attr_data->prefix_len, attr_data->prefix, attr_data->prefix_len ? ":" : "", attr_data->name_len,
                       attr_data->name);
            }
            goto skip_attr;
        }

        rc = lyd_create_attr(NULL, attr, mod, attr_data->name, attr_data->name_len, attr_data->value,
                             attr_data->value_len, &attr_data->dynamic, lydxml_resolve_prefix, ctx, LYD_XML, sparent);
        if (rc == LY_EINCOMPLETE) {
            ly_set_add(&ctx->incomplete_type_validation_attrs, attr, LY_SET_OPT_USEASLIST);
        } else if (rc) {
            ret = rc;
            goto cleanup;
        }
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
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_nodes(struct lyd_xml_ctx *ctx, struct lyd_node_inner *parent, const char **data, struct lyd_node **first)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct ly_set attrs_data = {0};
    const struct lyxml_ns *ns;
    struct lyd_attr *attr = NULL, *attr2;
    const struct lysc_node *snode;
    struct lys_module *mod;
    unsigned int parents_count = ctx->elements.count;
    struct lyd_node *cur = NULL, *key_anchor;
    int dynamic = 0;
    char *buffer = NULL, *value;
    size_t buffer_size = 0, value_len;

    while (ctx->status == LYXML_ELEMENT) {
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
            /* first parse all attributes so we have all the namespaces available */
            if (lydxml_attributes_parse(ctx, data, &attrs_data) != LY_SUCCESS) {
                ret = LY_EVALID;
                goto cleanup;
            }
        }

        ns = lyxml_ns_get((struct lyxml_context *)ctx, prefix, prefix_len);
        if (!ns) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", prefix_len, prefix);
            ret = LY_EVALID;
            goto cleanup;
        }
        mod = ly_ctx_get_module_implemented_ns(ctx->ctx, ns->uri);
        if (!mod) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "No module with namespace \"%s\" in the context.", ns->uri);
            ret = LY_EVALID;
            goto cleanup;
        }
        /* leave if-feature check for validation */
        snode = lys_find_child(parent ? parent->schema : NULL, mod, name, name_len, 0, LYS_GETNEXT_NOSTATECHECK);
        if (!snode) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_REFERENCE, "Element \"%.*s\" not found in the \"%s\" module.",
                   name_len, name, mod->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        if ((ctx->options & LYD_OPT_NO_STATE) && (snode->flags & LYS_CONFIG_R)) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LY_VCODE_INSTATE, snode->name);
            return LY_EVALID;
        }

        /* create actual attributes so that prefixes are available in the context */
        if (attrs_data.count) {
            LY_CHECK_GOTO(ret = lydxml_attributes(ctx, &attrs_data, snode, &attr), cleanup);
        }

        if (snode->nodetype & (LYS_ACTION | LYS_NOTIF)) {
            LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_DATA, "Unexpected %s element \"%.*s\".",
                   snode->nodetype == LYS_ACTION ? "RPC/action" : "notification", name_len, name);
            ret = LY_EVALID;
            goto cleanup;
        }

        if (snode->nodetype & LYD_NODE_TERM) {
            if (ctx->status == LYXML_ELEM_CONTENT) {
                /* get the value */
                ret = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &value, &value_len, &dynamic);
                if (ret != LY_SUCCESS) {
                    if (ret == LY_EINVAL) {
                        /* just indentation of a child element found */
                        LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_SYNTAX, "Child element inside terminal node \"%s\" found.",
                               snode->name);
                    }
                    goto cleanup;
                }
            } else {
                /* no content - validate empty value */
                value = "";
                value_len = 0;
                dynamic = 0;
            }

            /* create node */
            ret = lyd_create_term(snode, value, value_len, &dynamic, lydxml_resolve_prefix, ctx, LYD_XML, &cur);
            /* buffer spent */
            buffer = NULL;
            if (ret == LY_EINCOMPLETE) {
                if (!(ctx->options & LYD_OPT_PARSE_ONLY)) {
                    ly_set_add(&ctx->incomplete_type_validation, cur, LY_SET_OPT_USEASLIST);
                }
            } else if (ret) {
                goto cleanup;
            }

            if (parent && (cur->schema->flags & LYS_KEY)) {
                /* check the key order, the anchor must always be the last child */
                key_anchor = lyd_get_prev_key_anchor(parent->child, cur->schema);
                if ((!key_anchor && parent->child) || (key_anchor && key_anchor->next)) {
                    if (ctx->options & LYD_OPT_STRICT) {
                        LOGVAL(ctx->ctx, LY_VLOG_LINE, &ctx->line, LYVE_DATA, "Invalid position of the key \"%s\" in a list.",
                                cur->schema->name);
                        ret = LY_EVALID;
                        goto cleanup;
                    } else {
                        LOGWRN(ctx->ctx, "Invalid position of the key \"%s\" in a list.", cur->schema->name);
                    }
                }
            }
        } else if (snode->nodetype & LYD_NODE_INNER) {
            if (ctx->status == LYXML_ELEM_CONTENT) {
                LY_ERR r = lyxml_get_string((struct lyxml_context *)ctx, data, &buffer, &buffer_size, &value, &value_len, &dynamic);
                if (r != LY_EINVAL && (r != LY_SUCCESS || value_len != 0)) {
                    LOGINT(ctx->ctx);
                    ret = LY_EINT;
                    goto cleanup;
                }
            }

            /* create node */
            ret = lyd_create_inner(snode, &cur);
            LY_CHECK_GOTO(ret, cleanup);

            /* process children */
            if (ctx->status == LYXML_ELEMENT && parents_count != ctx->elements.count) {
                ret = lydxml_nodes(ctx, (struct lyd_node_inner *)cur, data, lyd_node_children_p(cur));
                LY_CHECK_GOTO(ret, cleanup);
            }

            if (!(ctx->options & LYD_OPT_PARSE_ONLY)) {
                /* new node validation, autodelete CANNOT occur, all nodes are new */
                ret = lyd_validate_new(lyd_node_children_p(cur), snode, NULL);
                LY_CHECK_GOTO(ret, cleanup);

                /* add any missing default children */
                ret = lyd_validate_defaults_r((struct lyd_node_inner *)cur, lyd_node_children_p(cur), NULL, NULL,
                                              &ctx->incomplete_type_validation, &ctx->when_check);
                LY_CHECK_GOTO(ret, cleanup);
            }

            /* hash now that all keys should be parsed, rehash for key-less list */
            if (snode->nodetype == LYS_LIST) {
                lyd_hash(cur);
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
                    LY_CHECK_GOTO(ret, cleanup);
                    break;
                case LYXML_ATTRIBUTE:
                    lyxml_get_attribute((struct lyxml_context *)ctx, data, &p, &p_len, &n, &n_len);
                    break;
                case LYXML_ELEM_CONTENT:
                case LYXML_ATTR_CONTENT:
                    ret = lyxml_get_string((struct lyxml_context *)ctx, data, NULL, NULL, NULL, NULL, NULL);
                    /* not an error, just incorrect XML parser status */
                    if (ret && (ret != LY_EINVAL)) {
                        goto cleanup;
                    }
                    break;
                case LYXML_END:
                    /* end of data */
                    LOGINT(ctx->ctx);
                    ret = LY_EINT;
                    goto cleanup;
                }
            }

            /* get value */
            if (start != *data) {
                /* data now points after the anydata's closing element tag, we need just end of its content */
                for (stop = *data - 1; *stop != '<'; --stop);
                start = lydict_insert(ctx->ctx, start, stop - start);
            } else {
                start = NULL;
            }

            /* create node */
            ret = lyd_create_any(snode, start, LYD_ANYDATA_XML, &cur);
            if (ret) {
                lydict_remove(ctx->ctx, start);
                goto cleanup;
            }
        }

        /* correct flags */
        if (!(snode->nodetype & (LYS_ACTION | LYS_NOTIF)) && snode->when) {
            if (ctx->options & LYD_OPT_TRUSTED) {
                /* just set it to true */
                cur->flags |= LYD_WHEN_TRUE;
            } else {
                /* remember we need to evaluate this node's when */
                ly_set_add(&ctx->when_check, cur, LY_SET_OPT_USEASLIST);
            }
        }
        if (ctx->options & LYD_OPT_TRUSTED) {
            /* node is valid */
            cur->flags &= ~LYD_NEW;
        }
        LY_LIST_FOR(attr, attr2) {
            if (!strcmp(attr2->name, "default") && !strcmp(attr2->annotation->module->name, "ietf-netconf-with-defaults")
                    && attr2->value.boolean) {
                /* node is default according to the metadata */
                cur->flags |= LYD_DEFAULT;
            }
        }

        /* add attributes */
        assert(!cur->attr);
        cur->attr = attr;
        attr = NULL;

        /* insert */
        lyd_insert_node((struct lyd_node *)parent, first, cur);

        cur = NULL;
    }

    /* success */
    ret = LY_SUCCESS;

cleanup:
    free(buffer);
    lyd_free_attr(ctx->ctx, attr, 1);
    lyd_free_tree(cur);
    for (unsigned int u = 0; u < attrs_data.count; ++u) {
        if (((struct attr_data_s*)attrs_data.objs[u])->dynamic) {
            free(((struct attr_data_s*)attrs_data.objs[u])->value);
        }
    }
    ly_set_erase(&attrs_data, free);
    if (ret && *first) {
        lyd_free_siblings(*first);
        *first = NULL;
    }
    return ret;
}

LY_ERR
lyd_parse_xml(struct ly_ctx *ctx, const char *data, int options, struct lyd_node **tree)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_xml_ctx xmlctx = {0};
    uint32_t i = 0;
    const struct lys_module *mod;
    struct lyd_node *first, *next, **first2;

    xmlctx.options = options & LYD_OPT_MASK;
    xmlctx.ctx = ctx;
    xmlctx.line = 1;

    /* init */
    *tree = NULL;

    /* parse XML data */
    ret = lydxml_nodes(&xmlctx, NULL, &data, tree);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(options & LYD_OPT_PARSE_ONLY)) {
        next = *tree;
        while (1) {
            if (options & LYD_VALOPT_DATA_ONLY) {
                mod = lyd_data_next_module(&next, &first);
            } else {
                mod = lyd_mod_next_module(next, NULL, 0, ctx, &i, &first);
            }
            if (!mod) {
                break;
            }
            if (first == *tree) {
                /* make sure first2 changes are carried to tree */
                first2 = tree;
            } else {
                first2 = &first;
            }

            /* validate new top-level nodes, autodelete CANNOT occur, all nodes are new */
            ret = lyd_validate_new(first2, NULL, mod);
            LY_CHECK_GOTO(ret, cleanup);

            /* add all top-level defaults for this module */
            ret = lyd_validate_defaults_r(NULL, first2, NULL, mod, &xmlctx.incomplete_type_validation, &xmlctx.when_check);
            LY_CHECK_GOTO(ret, cleanup);

            /* finish incompletely validated terminal values/attributes and when conditions */
            ret = lyd_validate_unres(tree, &xmlctx.when_check, &xmlctx.incomplete_type_validation,
                                     &xmlctx.incomplete_type_validation_attrs, LYD_XML, lydxml_resolve_prefix, ctx);
            LY_CHECK_GOTO(ret, cleanup);

            /* perform final validation that assumes the data tree is final */
            ret = lyd_validate_siblings_r(*first2, NULL, mod, options & LYD_VALOPT_MASK);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    /* there should be no unresolved types stored */
    assert(!(options & LYD_OPT_PARSE_ONLY) || (!xmlctx.incomplete_type_validation.count
           && !xmlctx.incomplete_type_validation_attrs.count && !xmlctx.when_check.count));

    ly_set_erase(&xmlctx.incomplete_type_validation, NULL);
    ly_set_erase(&xmlctx.incomplete_type_validation_attrs, NULL);
    ly_set_erase(&xmlctx.when_check, NULL);
    lyxml_context_clear((struct lyxml_context *)&xmlctx);
    if (ret) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    return ret;
}
