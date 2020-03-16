/**
 * @file printer_xml.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML printer for libyang data structure
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "log.h"
#include "plugins_types.h"
#include "printer_data.h"
#include "printer_internal.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "xml.h"

/**
 * @brief XML printer context.
 */
struct xmlpr_ctx {
    struct lyout *out;  /**< output specification */
    unsigned int level; /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
    int options;        /**< [Data printer flags](@ref dataprinterflags) */
    const struct ly_ctx *ctx; /**< libyang context */
    struct ly_set prefix;   /**< printed namespace prefixes */
    struct ly_set ns;   /**< printed namespaces */
};

#define LEVEL ctx->level                     /**< current level */
#define INDENT ((LEVEL) ? (LEVEL)*2 : 0),""  /**< indentation parameters for printer functions */
#define LEVEL_INC if (LEVEL) {LEVEL++;}      /**< increase indentation level */
#define LEVEL_DEC if (LEVEL) {LEVEL--;}      /**< decrease indentation level */

#define LYXML_PREFIX_REQUIRED 0x01  /**< The prefix is not just a suggestion but a requirement. */

/**
 * @brief Print a namespace if not already printed.
 *
 * @param[in] ctx XML printer context.
 * @param[in] ns Namespace to print, expected to be in dictionary.
 * @param[in] new_prefix Suggested new prefix, NULL for a default namespace without prefix. Stored in the dictionary.
 * @param[in] prefix_opts Prefix options changing the meaning of parameters.
 * @return Printed prefix of the namespace to use.
 */
static const char *
xml_print_ns(struct xmlpr_ctx *ctx, const char *ns, const char *new_prefix, int prefix_opts)
{
    int i;

    for (i = ctx->ns.count - 1; i > -1; --i) {
        if (!new_prefix) {
            /* find default namespace */
            if (!ctx->prefix.objs[i]) {
                if (ctx->ns.objs[i] != ns) {
                    /* different default namespace */
                    i = -1;
                }
                break;
            }
        } else {
            /* find prefixed namespace */
            if (ctx->ns.objs[i] == ns) {
                if (!ctx->prefix.objs[i]) {
                    /* default namespace is not interesting */
                    continue;
                }

                if (!strcmp(ctx->prefix.objs[i], new_prefix) || !(prefix_opts & LYXML_PREFIX_REQUIRED)) {
                    /* the same prefix or can be any */
                    break;
                }
            }
        }
    }

    if (i == -1) {
        /* suitable namespace not found, must be printed */
        ly_print(ctx->out, " xmlns%s%s=\"%s\"", new_prefix ? ":" : "", new_prefix ? new_prefix : "", ns);

        /* and added into namespaces */
        if (new_prefix) {
            new_prefix = lydict_insert(ctx->ctx, new_prefix, 0);
        }
        ly_set_add(&ctx->prefix, (void *)new_prefix, LY_SET_OPT_USEASLIST);
        i = ly_set_add(&ctx->ns, (void *)ns, LY_SET_OPT_USEASLIST);
    }

    /* return it */
    return ctx->prefix.objs[i];
}

/**
 * @brief XML mapping of YANG modules to prefixes in values.
 *
 * Implementation of ly_clb_get_prefix
 */
static const char *
xml_print_get_prefix(const struct lys_module *mod, void *private)
{
    struct ly_set *ns_list = (struct ly_set*)private;

    ly_set_add(ns_list, (void*)mod, 0);
    return mod->prefix;
}

/**
 * TODO
 */
static void
xml_print_meta(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    struct lyd_meta *meta;
    const struct lys_module *mod;
    struct ly_set ns_list = {0};
#if 0
    const char **prefs, **nss;
    const char *xml_expr = NULL, *mod_name;
    uint32_t ns_count, i;
    int rpc_filter = 0;
    char *p;
    size_t len;
#endif
    int dynamic;
    unsigned int u;

    /* with-defaults */
    if (node->schema->nodetype & LYD_NODE_TERM) {
        if (((node->flags & LYD_DEFAULT) && (ctx->options & (LYDP_WD_ALL_TAG | LYDP_WD_IMPL_TAG))) ||
                ((ctx->options & LYDP_WD_ALL_TAG) && ly_is_default(node))) {
            /* we have implicit OR explicit default node, print attribute only if context include with-defaults schema */
            mod = ly_ctx_get_module_latest(node->schema->module->ctx, "ietf-netconf-with-defaults");
            if (mod) {
                ly_print(ctx->out, " %s:default=\"true\"", xml_print_ns(ctx, mod->ns, mod->prefix, 0));
            }
        }
    }
#if 0
    /* technically, check for the extension get-filter-element-attributes from ietf-netconf */
    if (!strcmp(node->schema->name, "filter")
            && (!strcmp(node->schema->module->name, "ietf-netconf") || !strcmp(node->schema->module->name, "notifications"))) {
        rpc_filter = 1;
    }
#endif
    for (meta = node->meta; meta; meta = meta->next) {
        const char *value = meta->value.realtype->plugin->print(&meta->value, LYD_XML, xml_print_get_prefix, &ns_list, &dynamic);

        /* print namespaces connected with the value's prefixes */
        for (u = 0; u < ns_list.count; ++u) {
            mod = (const struct lys_module *)ns_list.objs[u];
            xml_print_ns(ctx, mod->ns, mod->prefix, 1);
        }
        ly_set_erase(&ns_list, NULL);

#if 0
        if (rpc_filter) {
            /* exception for NETCONF's filter's attributes */
            if (!strcmp(meta->name, "select")) {
                /* xpath content, we have to convert the JSON format into XML first */
                xml_expr = transform_json2xml(node->schema->module, meta->value_str, 0, &prefs, &nss, &ns_count);
                if (!xml_expr) {
                    /* error */
                    return EXIT_FAILURE;
                }

                for (i = 0; i < ns_count; ++i) {
                    ly_print(out, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
                }
                free(prefs);
                free(nss);
            }
            ly_print(out, " %s=\"", meta->name);
        } else {
#endif
            /* print the metadata with its namespace */
            mod = meta->annotation->module;
            ly_print(ctx->out, " %s:%s=\"", xml_print_ns(ctx, mod->ns, mod->prefix, 1), meta->name);
#if 0
        }
#endif

        /* print metadata value */
        if (value && value[0]) {
            lyxml_dump_text(ctx->out, value, 1);
        }
        ly_print(ctx->out, "\"");
        if (dynamic) {
            free((void *)value);
        }
    }
}

/**
 * @brief Print generic XML element despite of the data node type.
 *
 * Prints the element name, attributes and necessary namespaces.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 */
static void
xml_print_node_open(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    /* print node name */
    ly_print(ctx->out, "%*s<%s", INDENT, node->schema->name);

    /* print default namespace */
    xml_print_ns(ctx, node->schema->module->ns, NULL, 0);

    /* print metadata */
    xml_print_meta(ctx, node);
}

static LY_ERR
xml_print_attr(struct xmlpr_ctx *ctx, const struct lyd_node_opaq *node)
{
    const struct ly_attr *attr;
    const char *pref;
    uint32_t u;

    LY_LIST_FOR(node->attr, attr) {
        pref = NULL;
        if (attr->prefix.pref) {
            /* print attribute namespace */
            switch (attr->format) {
            case LYD_XML:
                pref = xml_print_ns(ctx, attr->prefix.ns, attr->prefix.pref, 0);
                break;
            case LYD_SCHEMA:
                /* cannot be created */
                LOGINT(node->ctx);
                return LY_EINT;
            }
        }

        /* print namespaces connected with the value's prefixes */
        if (attr->val_prefs) {
            LY_ARRAY_FOR(attr->val_prefs, u) {
                xml_print_ns(ctx, attr->val_prefs[u].ns, attr->val_prefs[u].pref, LYXML_PREFIX_REQUIRED);
            }
        }

        /* print the attribute with its prefix and value */
        ly_print(ctx->out, " %s%s%s=\"%s\"", pref ? pref : "", pref ? ":" : "", attr->name, attr->value);
    }

    return LY_SUCCESS;
}

static LY_ERR
xml_print_opaq_open(struct xmlpr_ctx *ctx, const struct lyd_node_opaq *node)
{
    /* print node name */
    ly_print(ctx->out, "%*s<%s", INDENT, node->name);

    /* print default namespace */
    switch (node->format) {
    case LYD_XML:
        xml_print_ns(ctx, node->prefix.ns, NULL, 0);
        break;
    case LYD_SCHEMA:
        /* cannot be created */
        LOGINT(node->ctx);
        return LY_EINT;
    }

    /* print attributes */
    LY_CHECK_RET(xml_print_attr(ctx, node));

    return LY_SUCCESS;
}

static LY_ERR xml_print_node(struct xmlpr_ctx *ctx, const struct lyd_node *node);

/**
 * @brief Print XML element representing lyd_node_term.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 */
static void
xml_print_term(struct xmlpr_ctx *ctx, const struct lyd_node_term *node)
{
    struct ly_set ns_list = {0};
    unsigned int u;
    int dynamic;
    const char *value;

    xml_print_node_open(ctx, (struct lyd_node *)node);
    value = ((struct lysc_node_leaf *)node->schema)->type->plugin->print(&node->value, LYD_XML, xml_print_get_prefix, &ns_list, &dynamic);

    /* print namespaces connected with the values's prefixes */
    for (u = 0; u < ns_list.count; ++u) {
        const struct lys_module *mod = (const struct lys_module *)ns_list.objs[u];
        ly_print(ctx->out, " xmlns:%s=\"%s\"", mod->prefix, mod->ns);
    }
    ly_set_erase(&ns_list, NULL);

    if (!value || !value[0]) {
        ly_print(ctx->out, "/>%s", LEVEL ? "\n" : "");
    } else {
        ly_print(ctx->out, ">");
        lyxml_dump_text(ctx->out, value, 0);
        ly_print(ctx->out, "</%s>%s", node->schema->name, LEVEL ? "\n" : "");
    }
    if (dynamic) {
        free((void *)value);
    }
}

/**
 * @brief Print XML element representing lyd_node_inner.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_inner(struct xmlpr_ctx *ctx, const struct lyd_node_inner *node)
{
    LY_ERR ret;
    struct lyd_node *child;

    xml_print_node_open(ctx, (struct lyd_node *)node);

    if (!node->child) {
        ly_print(ctx->out, "/>%s", ctx->level ? "\n" : "");
        return LY_SUCCESS;
    }

    /* children */
    ly_print(ctx->out, ">%s", ctx->level ? "\n" : "");

    LEVEL_INC;
    LY_LIST_FOR(node->child, child) {
        ret = xml_print_node(ctx, child);
        LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
    }
    LEVEL_DEC;

    ly_print(ctx->out, "%*s</%s>%s", INDENT, node->schema->name, LEVEL ? "\n" : "");

    return LY_SUCCESS;
}

static LY_ERR
xml_print_anydata(struct xmlpr_ctx *ctx, const struct lyd_node_any *node)
{
    struct lyd_node_any *any = (struct lyd_node_any *)node;
    struct lyd_node *iter;
    int prev_opts;
    LY_ERR ret;

    xml_print_node_open(ctx, (struct lyd_node *)node);

    if (!any->value.tree) {
        /* no content */
no_content:
        ly_print(ctx->out, "/>%s", LEVEL ? "\n" : "");
        return LY_SUCCESS;
    } else {
        switch (any->value_type) {
        case LYD_ANYDATA_DATATREE:
            /* close opening tag and print data */
            prev_opts = ctx->options;
            ctx->options &= ~(LYDP_WITHSIBLINGS | LYDP_NETCONF);
            LEVEL_INC;

            ly_print(ctx->out, ">%s", LEVEL ? "\n" : "");
            LY_LIST_FOR(any->value.tree, iter) {
                ret = xml_print_node(ctx, iter);
                LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
            }

            LEVEL_DEC;
            ctx->options = prev_opts;
            break;
        case LYD_ANYDATA_STRING:
            /* escape XML-sensitive characters */
            if (!any->value.str[0]) {
                goto no_content;
            }
            /* close opening tag and print data */
            ly_print(ctx->out, ">");
            lyxml_dump_text(ctx->out, any->value.str, 0);
            break;
        case LYD_ANYDATA_XML:
            /* print without escaping special characters */
            if (!any->value.str[0]) {
                goto no_content;
            }
            ly_print(ctx->out, ">%s", any->value.str);
            break;
        case LYD_ANYDATA_JSON:
#if 0 /* TODO LYB format */
        case LYD_ANYDATA_LYB:
#endif
            /* JSON and LYB format is not supported */
            LOGWRN(node->schema->module->ctx, "Unable to print anydata content (type %d) as XML.", any->value_type);
            goto no_content;
        }

        /* closing tag */
        if (any->value_type == LYD_ANYDATA_DATATREE) {
            ly_print(ctx->out, "%*s</%s>%s", INDENT, node->schema->name, LEVEL ? "\n" : "");
        } else {
            ly_print(ctx->out, "</%s>%s", node->schema->name, LEVEL ? "\n" : "");
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
xml_print_opaq(struct xmlpr_ctx *ctx, const struct lyd_node_opaq *node)
{
    LY_ERR ret;
    struct lyd_node *child;
    uint32_t u;

    LY_CHECK_RET(xml_print_opaq_open(ctx, node));

    if (node->value[0]) {
        /* print namespaces connected with the value's prefixes */
        if (node->val_prefs) {
            LY_ARRAY_FOR(node->val_prefs, u) {
                xml_print_ns(ctx, node->val_prefs[u].ns, node->val_prefs[u].pref, LYXML_PREFIX_REQUIRED);
            }
        }

        ly_print(ctx->out, ">%s", node->value);
    }

    if (node->child) {
        /* children */
        if (!node->value[0]) {
            ly_print(ctx->out, ">%s", ctx->level ? "\n" : "");
        }

        LEVEL_INC;
        LY_LIST_FOR(node->child, child) {
            ret = xml_print_node(ctx, child);
            LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
        }
        LEVEL_DEC;

        ly_print(ctx->out, "%*s</%s>%s", INDENT, node->name, LEVEL ? "\n" : "");
    } else if (node->value[0]) {
        ly_print(ctx->out, "</%s>%s", node->name, LEVEL ? "\n" : "");
    } else {
        /* no value or children */
        ly_print(ctx->out, "/>%s", ctx->level ? "\n" : "");
    }

    return LY_SUCCESS;
}

/**
 * @brief Print XML element representing lyd_node.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_node(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t ns_count;

    if (!ly_should_print(node, ctx->options)) {
        /* do not print at all */
        return LY_SUCCESS;
    }

    /* remember namespace definition count on this level */
    ns_count = ctx->ns.count;

    if (!node->schema) {
        ret = xml_print_opaq(ctx, (const struct lyd_node_opaq *)node);
    } else {
        switch (node->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_NOTIF:
        case LYS_ACTION:
            ret = xml_print_inner(ctx, (const struct lyd_node_inner *)node);
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            xml_print_term(ctx, (const struct lyd_node_term *)node);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            ret = xml_print_anydata(ctx, (const struct lyd_node_any *)node);
            break;
        default:
            LOGINT(node->schema->module->ctx);
            ret = LY_EINT;
            break;
        }
    }

    /* remove all added namespaces */
    while (ns_count < ctx->ns.count) {
        FREE_STRING(ctx->ctx, ctx->prefix.objs[ctx->prefix.count - 1]);
        ly_set_rm_index(&ctx->prefix, ctx->prefix.count - 1, NULL);
        ly_set_rm_index(&ctx->ns, ctx->ns.count - 1, NULL);
    }

    return ret;
}

LY_ERR
xml_print_data(struct lyout *out, const struct lyd_node *root, int options)
{
    const struct lyd_node *node;
    struct xmlpr_ctx ctx = {0};

    if (!root) {
        if (out->type == LYOUT_MEMORY || out->type == LYOUT_CALLBACK) {
            ly_print(out, "");
        }
        goto finish;
    }

    ctx.out = out;
    ctx.level = (options & LYDP_FORMAT ? 1 : 0);
    ctx.options = options;
    ctx.ctx = LYD_NODE_CTX(root);

    /* content */
    LY_LIST_FOR(root, node) {
        LY_CHECK_RET(xml_print_node(&ctx, node));
        if (!(options & LYDP_WITHSIBLINGS)) {
            break;
        }
    }

finish:
    assert(!ctx.prefix.count && !ctx.ns.count);
    ly_set_erase(&ctx.prefix, NULL);
    ly_set_erase(&ctx.ns, NULL);
    ly_print_flush(out);
    return LY_SUCCESS;
}

