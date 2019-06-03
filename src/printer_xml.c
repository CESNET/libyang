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
    int toplevel;       /**< top-level flag */
};

#define LEVEL ctx->level                     /**< current level */
#define INDENT ((LEVEL) ? (LEVEL)*2 : 0),""  /**< indentation parameters for printer functions */
#define LEVEL_INC if (LEVEL) {LEVEL++;}      /**< increase indentation level */
#define LEVEL_DEC if (LEVEL) {LEVEL--;}      /**< decrease indentation level */

/**
 * TODO
 */
struct mlist {
    struct mlist *next;
    struct lys_module *module;
} *mlist = NULL, *mlist_new;

#if 0
static LY_ERR
modlist_add(struct mlist **mlist, const struct lys_module *mod)
{
    struct mlist *iter;

    for (iter = *mlist; iter; iter = iter->next) {
        if (mod == iter->module) {
            break;
        }
    }

    if (!iter) {
        iter = malloc(sizeof *iter);
        LY_CHECK_ERR_RET(!iter, LOGMEM(mod->ctx), LY_EMEM);
        iter->next = *mlist;
        iter->module = (struct lys_module *)mod;
        *mlist = iter;
    }

    return LY_SUCCESS;
}
#endif

/**
 * TODO
 */
static void
xml_print_ns(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    struct lyd_node *next, *cur, *child;
    struct lyd_attr *attr;

#if 0
    struct mlist *mlist = NULL, *miter;

    const struct lys_module *wdmod = NULL;

    /* add node attribute modules */
    for (attr = node->attr; attr; attr = attr->next) {
        if (!strcmp(node->schema->name, "filter") &&
                (!strcmp(node->schema->module->name, "ietf-netconf") ||
                 !strcmp(node->schema->module->name, "notifications"))) {
            /* exception for NETCONF's filter attributes */
            continue;
        } else {
            r = modlist_add(&mlist, lys_main_module(attr->annotation->module));
        }
        if (r) {
            goto print;
        }
    }
#endif

    /* add node children nodes and attribute modules */
    switch (node->schema->nodetype) {
    case LYS_LEAFLIST:
    case LYS_LEAF:
        /* TODO ietf-netconf-with-defaults namespace */
#if 0
        if (node->dflt && (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG))) {
            /* get with-defaults module and print its namespace */
            wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL, 1);
            if (wdmod && modlist_add(&mlist, wdmod)) {
                goto print;
            }
        }
#endif
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
#if 0
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
        if (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG)) {
            /* get with-defaults module and print its namespace */
            wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL, 1);
            if (wdmod && modlist_add(&mlist, wdmod)) {
                goto print;
            }
        }
#endif
        LY_LIST_FOR(((struct lyd_node_inner*)node)->child, child) {
            LYD_TREE_DFS_BEGIN(child, next, cur) {
                for (attr = cur->attr; attr; attr = attr->next) {
                    if (!strcmp(cur->schema->name, "filter") &&
                            (!strcmp(cur->schema->module->name, "ietf-netconf") ||
                             !strcmp(cur->schema->module->name, "notifications"))) {
                        /* exception for NETCONF's filter attributes */
                        continue;
                    } else {
                        /* TODO annotations r = modlist_add(&mlist, lys_main_module(attr->annotation->module)); */
                    }
                }
            LYD_TREE_DFS_END(child, next, cur)}
        }
        break;
    default:
        break;
    }
#if 0
print:
    /* print used namespaces */
    while (mlist) {
        miter = mlist;
        mlist = mlist->next;

        ly_print(ctx->out, " xmlns:%s=\"%s\"", miter->module->prefix, miter->module->ns);
        free(miter);
    }
#endif
}

/**
 * TODO
 */
static LY_ERR
xml_print_attrs(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    (void) ctx;
    (void) node;

#if 0
    struct lyd_attr *attr;
    const char **prefs, **nss;
    const char *xml_expr = NULL, *mod_name;
    uint32_t ns_count, i;
    int rpc_filter = 0;
    const struct lys_module *wdmod = NULL;
    char *p;
    size_t len;

    LY_PRINT_SET;

    /* with-defaults */
    if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        if ((node->dflt && (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG))) ||
                (!node->dflt && (options & LYP_WD_ALL_TAG) && lyd_wd_default((struct lyd_node_leaf_list *)node))) {
            /* we have implicit OR explicit default node */
            /* get with-defaults module */
            wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL, 1);
            if (wdmod) {
                /* print attribute only if context include with-defaults schema */
                ly_print(out, " %s:default=\"true\"", wdmod->prefix);
            }
        }
    }
    /* technically, check for the extension get-filter-element-attributes from ietf-netconf */
    if (!strcmp(node->schema->name, "filter")
            && (!strcmp(node->schema->module->name, "ietf-netconf") || !strcmp(node->schema->module->name, "notifications"))) {
        rpc_filter = 1;
    }

    for (attr = node->attr; attr; attr = attr->next) {
        if (rpc_filter) {
            /* exception for NETCONF's filter's attributes */
            if (!strcmp(attr->name, "select")) {
                /* xpath content, we have to convert the JSON format into XML first */
                xml_expr = transform_json2xml(node->schema->module, attr->value_str, 0, &prefs, &nss, &ns_count);
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
            ly_print(out, " %s=\"", attr->name);
        } else {
            ly_print(out, " %s:%s=\"", attr->annotation->module->prefix, attr->name);
        }

        switch (attr->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
        case LY_TYPE_BITS:
        case LY_TYPE_ENUM:
        case LY_TYPE_BOOL:
        case LY_TYPE_DEC64:
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
            if (attr->value_str) {
                /* xml_expr can contain transformed xpath */
                lyxml_dump_text(out, xml_expr ? xml_expr : attr->value_str, LYXML_DATA_ATTR);
            }
            break;

        case LY_TYPE_IDENT:
            if (!attr->value_str) {
                break;
            }
            p = strchr(attr->value_str, ':');
            assert(p);
            len = p - attr->value_str;
            mod_name = attr->annotation->module->name;
            if (!strncmp(attr->value_str, mod_name, len) && !mod_name[len]) {
                lyxml_dump_text(out, ++p, LYXML_DATA_ATTR);
            } else {
                /* avoid code duplication - use instance-identifier printer which gets necessary namespaces to print */
                goto printinst;
            }
            break;
        case LY_TYPE_INST:
printinst:
            xml_expr = transform_json2xml(node->schema->module, ((struct lyd_node_leaf_list *)node)->value_str, 1,
                                          &prefs, &nss, &ns_count);
            if (!xml_expr) {
                /* error */
                return EXIT_FAILURE;
            }

            for (i = 0; i < ns_count; ++i) {
                ly_print(out, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
            }
            free(prefs);
            free(nss);

            lyxml_dump_text(out, xml_expr, LYXML_DATA_ATTR);
            lydict_remove(node->schema->module->ctx, xml_expr);
            break;

        /* LY_TYPE_LEAFREF not allowed */
        case LY_TYPE_EMPTY:
            break;

        default:
            /* error */
            LOGINT(node->schema->module->ctx);
            return EXIT_FAILURE;
        }

        ly_print(out, "\"");

        if (xml_expr) {
            lydict_remove(node->schema->module->ctx, xml_expr);
        }
    }
#endif

    return LY_SUCCESS;
}

/**
 * @brief Print generic XML element despite of the data node type.
 *
 * Prints the element name, attributes and necessary namespaces.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_node_open(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    if (ctx->toplevel || !node->parent || node->schema->module != node->parent->schema->module) {
        /* print "namespace" */
        ly_print(ctx->out, "%*s<%s xmlns=\"%s\"", INDENT, node->schema->name, node->schema->module->ns);
    } else {
        ly_print(ctx->out, "%*s<%s", INDENT, node->schema->name);
    }

    if (ctx->toplevel) {
        xml_print_ns(ctx, node);
        ctx->toplevel = 0;
    }

    LY_CHECK_RET(xml_print_attrs(ctx, node));

    return LY_SUCCESS;
}

static LY_ERR xml_print_node(struct xmlpr_ctx *ctx, const struct lyd_node *node);

/**
 * @brief Print XML element representing lyd_node_term.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_term(struct xmlpr_ctx *ctx, const struct lyd_node_term *node)
{
    LY_CHECK_RET(xml_print_node_open(ctx, (struct lyd_node *)node));

    if (((struct lysc_node_leaf*)node->schema)->type->plugin->flags & LY_TYPE_FLAG_PREFIXES) {
        /* TODO get prefixes from the value and print namespaces */
    }

    if (!node->value.canonized || !node->value.canonized[0]) {
        ly_print(ctx->out, "/>%s", LEVEL ? "\n" : "");
    } else {
        ly_print(ctx->out, ">");
        lyxml_dump_text(ctx->out, node->value.canonized, 0);
        ly_print(ctx->out, "</%s>%s", node->schema->name, LEVEL ? "\n" : "");
    }

    return LY_SUCCESS;
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

    LY_CHECK_RET(xml_print_node_open(ctx, (struct lyd_node *)node));

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

#if 0
static int
xml_print_anydata(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    char *buf;
    struct lyd_node_anydata *any = (struct lyd_node_anydata *)node;
    struct lyd_node *iter;
    const char *ns;

    LY_PRINT_SET;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        ns = lyd_node_module(node)->ns;
        ly_print(out, "%*s<%s xmlns=\"%s\"", INDENT, node->schema->name, ns);
    } else {
        ly_print(out, "%*s<%s", INDENT, node->schema->name);
    }

    if (toplevel) {
        xml_print_ns(out, node, options);
    }
    if (xml_print_attrs(out, node, options)) {
        return EXIT_FAILURE;
    }
    if (!(void*)any->value.tree || (any->value_type == LYD_ANYDATA_CONSTSTRING && !any->value.str[0])) {
        /* no content */
        ly_print(out, "/>%s", level ? "\n" : "");
    } else {
        if (any->value_type == LYD_ANYDATA_DATATREE) {
            /* print namespaces in the anydata data tree */
            LY_TREE_FOR(any->value.tree, iter) {
                xml_print_ns(out, iter, options);
            }
        }
        /* close opening tag ... */
        ly_print(out, ">");
        /* ... and print anydata content */
        switch (any->value_type) {
        case LYD_ANYDATA_CONSTSTRING:
            lyxml_dump_text(out, any->value.str, LYXML_DATA_ELEM);
            break;
        case LYD_ANYDATA_DATATREE:
            if (any->value.tree) {
                if (level) {
                    ly_print(out, "\n");
                }
                LY_TREE_FOR(any->value.tree, iter) {
                    if (xml_print_node(out, level ? level + 1 : 0, iter, 0, (options & ~(LYP_WITHSIBLINGS | LYP_NETCONF)))) {
                        return EXIT_FAILURE;
                    }
                }
            }
            break;
        case LYD_ANYDATA_XML:
            lyxml_print_mem(&buf, any->value.xml, (level ? LYXML_PRINT_FORMAT | LYXML_PRINT_NO_LAST_NEWLINE : 0)
                                                   | LYXML_PRINT_SIBLINGS);
            ly_print(out, "%s%s", level ? "\n" : "", buf);
            free(buf);
            break;
        case LYD_ANYDATA_SXML:
            /* print without escaping special characters */
            ly_print(out, "%s", any->value.str);
            break;
        case LYD_ANYDATA_JSON:
        case LYD_ANYDATA_LYB:
            /* JSON and LYB format is not supported */
            LOGWRN(node->schema->module->ctx, "Unable to print anydata content (type %d) as XML.", any->value_type);
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_SXMLD:
        case LYD_ANYDATA_JSOND:
        case LYD_ANYDATA_LYBD:
            /* dynamic strings are used only as input parameters */
            assert(0);
            break;
        }

        /* closing tag */
        ly_print(out, "</%s>%s", node->schema->name, level ? "\n" : "");
    }

    LY_PRINT_RET(node->schema->module->ctx);
}
#endif

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

#if 0
    if (!lyd_wd_toprint(node, ctx->options)) {
        /* wd says do not print */
        return EXIT_SUCCESS;
    }
#endif

    switch (node->schema->nodetype) {
#if 0
    case LYS_NOTIF:
    case LYS_ACTION:
#endif
    case LYS_CONTAINER:
    case LYS_LIST:
        ret = xml_print_inner(ctx, (const struct lyd_node_inner*)node);
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        ret = xml_print_term(ctx, (const struct lyd_node_term*)node);
        break;
#if 0
    case LYS_ANYXML:
    case LYS_ANYDATA:
        ret = xml_print_anydata(ctx, node);
        break;
#endif
    default:
        LOGINT(node->schema->module->ctx);
        ret = LY_EINT;
        break;
    }

    return ret;
}

LY_ERR
xml_print_data(struct lyout *out, const struct lyd_node *root, int options)
{
    const struct lyd_node *node;
    struct xmlpr_ctx ctx_ = {.out = out, .level = (options & LYDP_FORMAT ? 1 : 0), .options = options, .toplevel = 1}, *ctx = &ctx_;

    if (!root) {
        if (out->type == LYOUT_MEMORY || out->type == LYOUT_CALLBACK) {
            ly_print(out, "");
        }
        goto finish;
    }

    /* content */
    LY_LIST_FOR(root, node) {
        if (xml_print_node(ctx, node)) {
            return EXIT_FAILURE;
        }
        if (!(options & LYDP_WITHSIBLINGS)) {
            break;
        }
    }

finish:
    ly_print_flush(out);
    return LY_SUCCESS;
}

