
/**
 * @file printer_xml.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief XML printer for libyang data structure
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
#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "common.h"
#include "printer.h"
#include "xml_internal.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "resolve.h"
#include "tree_internal.h"

#define INDENT ""
#define LEVEL (level ? level*2-2 : 0)

void xml_print_node(struct lyout *out, int level, struct lyd_node *node);

static void
xml_print_attrs(struct lyout *out, struct lyd_node *node)
{
    struct lyd_attr *attr;
    struct lyd_ns *ns;

    for (attr = node->attr; attr; attr = attr->next) {
        if (attr->type == LYD_ATTR_NS) {
            ns = (struct lyd_ns *)attr;
            if (ns->prefix) {
                ly_print(out, " xmlns:%s=\"%s\"", ns->prefix, ns->value);
            } else {
                ly_print(out, " xmlns=\"%s\"", ns->value);
            }

        } else {
            if (attr->ns && attr->ns->prefix) {
                ly_print(out, " %s:%s=\"", attr->ns->prefix, attr->name);
            } else {
                ly_print(out, " %s=\"", attr->name);
            }
            lyxml_dump_text(out, attr->value);
            ly_print(out, "\"");
        }
    }
}

static void
xml_print_leaf(struct lyout *out, int level, struct lyd_node *node)
{
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node;
    char **prefs, **nss;
    const char *xml_expr;
    uint32_t ns_count, i;

    ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);

    xml_print_attrs(out, node);

    switch (leaf->value_type & LY_DATA_TYPE_MASK) {
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
        ly_print(out, ">");
        lyxml_dump_text(out, leaf->value_str);
        ly_print(out, "</%s>", node->schema->name);
        break;

    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
        xml_expr = transform_json2xml(node->schema->module, ((struct lyd_node_leaf_list *)node)->value_str,
                                      &prefs, &nss, &ns_count);
        if (!xml_expr) {
            /* error */
            ly_print(out, "\"(!error!)\"");
            return;
        }

        for (i = 0; i < ns_count; ++i) {
            ly_print(out, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
        }
        free(prefs);
        free(nss);

        if (xml_expr[0]) {
            ly_print(out, ">");
            lyxml_dump_text(out, xml_expr);
            ly_print(out, "</%s>", node->schema->name);
        } else {
            ly_print(out, "/>");
        }
        lydict_remove(node->schema->module->ctx, xml_expr);
        break;

    case LY_TYPE_LEAFREF:
        ly_print(out, ">");
        lyxml_dump_text(out, ((struct lyd_node_leaf_list *)(leaf->value.leafref))->value_str);
        ly_print(out, "</%s>", node->schema->name);
        break;

    case LY_TYPE_EMPTY:
        ly_print(out, "/>");
        break;

    default:
        /* error */
        ly_print(out, "\"(!error!)\"");
    }

    if (level) {
        ly_print(out, "\n");
    }
}

static void
xml_print_container(struct lyout *out, int level, struct lyd_node *node)
{
    struct lyd_node *child;

    ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);

    xml_print_attrs(out, node);

    if (!node->child) {
        ly_print(out, "/>%s", level ? "\n" : "");
        return;
    }
    ly_print(out, ">%s", level ? "\n" : "");

    LY_TREE_FOR(node->child, child) {
        xml_print_node(out, level ? level + 1 : 0, child);
    }

    ly_print(out, "%*s</%s>%s", LEVEL, INDENT, node->schema->name, level ? "\n" : "");
}

static void
xml_print_list(struct lyout *out, int level, struct lyd_node *node, int is_list)
{
    struct lyd_node *child;

    if (is_list) {
        /* list print */
        ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);

        xml_print_attrs(out, node);

        if (!node->child) {
            ly_print(out, "/>%s", level ? "\n" : "");
            return;
        }
        ly_print(out, ">%s", level ? "\n" : "");

        LY_TREE_FOR(node->child, child) {
            xml_print_node(out, level ? level + 1 : 0, child);
        }

        ly_print(out, "%*s</%s>%s", LEVEL, INDENT, node->schema->name, level ? "\n" : "");
    } else {
        /* leaf-list print */
        xml_print_leaf(out, level, node);
    }
}

static void
xml_print_anyxml(struct lyout *out, int level, struct lyd_node *node)
{
    FILE *stream;
    char *buf;
    size_t buf_size;
    struct lyd_node_anyxml *axml = (struct lyd_node_anyxml *)node;

    if (axml->value) {
        ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);

        xml_print_attrs(out, node);
        ly_print(out, ">%s", level ? "\n" : "");

        /* dump the anyxml into a buffer */
        stream = open_memstream(&buf, &buf_size);
        lyxml_dump(stream, axml->value, 0);
        fclose(stream);

        ly_print(out, "%s</%s>%s", buf, node->schema->name, level ? "\n" : "");
        free(buf);
    } else {
        ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);
        xml_print_attrs(out, node);
        ly_print(out, "/>%s", level ? "\n" : "");
    }
}

void
xml_print_node(struct lyout *out, int level, struct lyd_node *node)
{
    switch (node->schema->nodetype) {
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_CONTAINER:
        xml_print_container(out, level, node);
        break;
    case LYS_LEAF:
        xml_print_leaf(out, level, node);
        break;
    case LYS_LEAFLIST:
        xml_print_list(out, level, node, 0);
        break;
    case LYS_LIST:
        xml_print_list(out, level, node, 1);
        break;
    case LYS_ANYXML:
        xml_print_anyxml(out, level, node);
        break;
    default:
        LOGINT;
        break;
    }
}

int
xml_print_data(struct lyout *out, struct lyd_node *root, int format)
{
    struct lyd_node *node;

    /* content */
    LY_TREE_FOR(root, node) {
        xml_print_node(out, format ? 1 : 0, node);
    }

    return EXIT_SUCCESS;
}

