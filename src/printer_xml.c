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
#include "xml.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "resolve.h"
#include "tree_internal.h"

#define INDENT ""
#define LEVEL (level*2)

void xml_print_node(FILE *f, int level, struct lyd_node *node);

static void
xml_print_attrs(FILE *f, struct lyd_node *node)
{
    struct lyd_attr *attr;
    struct lyd_ns *ns;

    for (attr = node->attr; attr; attr = attr->next) {
        if (attr->type == LYD_ATTR_NS) {
            ns = (struct lyd_ns *)attr;
            if (ns->prefix) {
                fprintf(f, " xmlns:%s=\"%s\"", ns->prefix, ns->value);
            } else {
                fprintf(f, " xmlns=\"%s\"", ns->value);
            }

        } else {
            if (attr->ns && attr->ns->prefix) {
                fprintf(f, " %s:%s=\"%s\"", attr->ns->prefix, attr->name, attr->value);
            } else {
                fprintf(f, " %s=\"%s\"", attr->name, attr->value);
            }
        }
    }
}

/* (*prefixes) and (*namespaces) must be freed, but values must be ignored! (they point to the dictionary,
 * but were not inserted, ref_count is temporarily wrong!) */
static char *
transform_data_json2xml(struct ly_ctx *ctx, const char *json_data, char ***prefixes, char ***namespaces,
                          uint32_t *ns_count)
{
    const char *in, *id;
    char *out, *col, *mod_name;
    size_t out_size, out_used, id_len;
    struct lys_module *mod;
    uint32_t i;

    assert(prefixes && namespaces && ns_count);

    *ns_count = 0;
    *prefixes = NULL;
    *namespaces = NULL;

    if (!json_data) {
        /* empty value */
        return strdup("");
    }

    in = json_data;
    out_size = strlen(in)+1;
    out = malloc(out_size);
    out_used = 0;

    while (1) {
        col = strchr(in, ':');
        /* we're finished, copy the remaining part */
        if (!col) {
            strcpy(&out[out_used], in);
            out_used += strlen(in)+1;
            assert(out_size == out_used);
            return out;
        }
        id = strpbrk_backwards(col-1, "/ [", (col-in)-1);
        if ((id[0] == '/') || (id[0] == ' ') || (id[0] == '[')) {
            ++id;
        }
        id_len = col-id;

        /* get the module */
        mod_name = strndup(id, id_len);
        mod = ly_ctx_get_module(ctx, mod_name, NULL);
        free(mod_name);

        /* remember the new namespace definition */
        for (i = 0; i < *ns_count; ++i) {
            if ((*namespaces)[i] == mod->ns) {
                break;
            }
        }
        if (i == *ns_count) {
            ++(*ns_count);
            *prefixes = realloc(*prefixes, *ns_count * sizeof **prefixes);
            *namespaces = realloc(*namespaces, *ns_count * sizeof **namespaces);
            (*prefixes)[*ns_count-1] = (char *)mod->prefix;
            (*namespaces)[*ns_count-1] = (char *)mod->ns;
        }

        /* adjust out size */
        out_size += strlen(mod->prefix)-id_len;
        out = realloc(out, out_size);

        /* copy the data before prefix */
        strncpy(&out[out_used], in, id-in);
        out_used += id-in;

        /* copy the model name */
        strcpy(&out[out_used], mod->prefix);
        out_used += strlen(mod->prefix);

        /* copy ':' */
        out[out_used] = ':';
        ++out_used;

        /* finally adjust in pointer for next round */
        in = col+1;
    }

    /* unreachable */
    assert(0);
    return NULL;
}

static void
xml_print_leaf(FILE *f, int level, struct lyd_node *node)
{
    struct lyd_node_leaf *leaf = (struct lyd_node_leaf *)node;
    char **prefs, **nss, *xml_data;
    uint32_t ns_count, i;

    fprintf(f, "%*s<%s", LEVEL, INDENT, node->schema->name);

    xml_print_attrs(f, node);

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
        fprintf(f, ">%s</%s>\n", (leaf->value_str ? leaf->value_str : ""), node->schema->name);
        break;

    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
        xml_data = transform_data_json2xml(node->schema->module->ctx, ((struct lyd_node_leaf *)node)->value_str,
                                               &prefs, &nss, &ns_count);
        for (i = 0; i < ns_count; ++i) {
            fprintf(f, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
        }
        free(prefs);
        free(nss);

        if (xml_data[0]) {
            fprintf(f, ">%s</%s>\n", xml_data, node->schema->name);
        } else {
            fprintf(f, "/>\n");
        }
        free(xml_data);
        break;

    case LY_TYPE_LEAFREF:
        fprintf(f, ">%s</%s>\n", ((struct lyd_node_leaf *)(leaf->value.leafref))->value_str, node->schema->name);
        break;

    case LY_TYPE_EMPTY:
        fprintf(f, "/>\n");
        break;

    default:
        /* error */
        fprintf(f, "\"(!error!)\"");
    }
}

static void
xml_print_container(FILE *f, int level, struct lyd_node *node)
{
    struct lyd_node *child;

    fprintf(f, "%*s<%s", LEVEL, INDENT, node->schema->name);

    xml_print_attrs(f, node);

    fprintf(f, ">\n");

    LY_TREE_FOR(node->child, child) {
        xml_print_node(f, level + 1, child);
    }

    fprintf(f, "%*s</%s>\n", LEVEL, INDENT, node->schema->name);
}

static void
xml_print_list(FILE *f, int level, struct lyd_node *node, int is_list)
{
    struct lyd_node *child;

    if (is_list) {
        /* list print */
        fprintf(f, "%*s<%s", LEVEL, INDENT, node->schema->name);

        xml_print_attrs(f, node);
        fprintf(f, ">\n");

        LY_TREE_FOR(node->child, child) {
            xml_print_node(f, level + 1, child);
        }

        fprintf(f, "%*s</%s>\n", LEVEL, INDENT, node->schema->name);
    } else {
        /* leaf-list print */
        xml_print_leaf(f, level, node);
    }
}

static void
xml_print_anyxml(FILE *f, int level, struct lyd_node *node)
{
    FILE *stream;
    char *buf, *ptr, *line;
    size_t buf_size;
    struct lyd_node_anyxml *axml = (struct lyd_node_anyxml *)node;

    if (axml->value) {
        /* dump the anyxml into a buffer */
        stream = open_memstream(&buf, &buf_size);
        lyxml_dump(stream, axml->value, 0);
        fclose(stream);

        line = strtok_r(buf, "\n", &ptr);
        do {
            fprintf(f, "%*s%s\n", LEVEL, INDENT, line);
        } while ((line = strtok_r(NULL, "\n", &ptr)));

        free(buf);
    } else {
        fprintf(f, "%*s<%s/>\n", LEVEL, INDENT, node->schema->name);
    }
}

void
xml_print_node(FILE *f, int level, struct lyd_node *node)
{
    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
        xml_print_container(f, level, node);
        break;
    case LYS_LEAF:
        xml_print_leaf(f, level, node);
        break;
    case LYS_LEAFLIST:
        xml_print_list(f, level, node, 0);
        break;
    case LYS_LIST:
        xml_print_list(f, level, node, 1);
        break;
    case LYS_ANYXML:
        xml_print_anyxml(f, level, node);
        break;
    default:
        LOGINT;
        break;
    }
}

API int
xml_print_data(FILE *f, struct lyd_node *root)
{
    int level = 0;
    struct lyd_node *node;

    /* start */
    fprintf(f, "<libyang>\n");

    /* content */
    LY_TREE_FOR(root, node) {
        xml_print_node(f, level + 1, node);
    }

    /* end */
    fprintf(f, "</libyang>\n");

    return EXIT_SUCCESS;
}

