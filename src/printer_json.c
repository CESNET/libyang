/**
 * @file printer/json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON printer for libyang data structure
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "common.h"
#include "xml.h"
#include "tree.h"
#include "resolve.h"
#include "tree_internal.h"

#define INDENT ""
#define LEVEL (level*2)

void json_print_node(FILE *f, int level, struct lyd_node *node);

/* 0 - same, 1 - different */
static int
nscmp(struct lyd_node *node1, struct lyd_node *node2)
{
    struct lys_module *m1, *m2;

    /* we have to cover submodules belonging to the same module */
    if (node1->schema->module->type) {
        m1 = ((struct lys_submodule *)node1->schema->module)->belongsto;
    } else {
        m1 = node1->schema->module;
    }
    if (node2->schema->module->type) {
        m2 = ((struct lys_submodule *)node2->schema->module)->belongsto;
    } else {
        m2 = node2->schema->module;
    }
    if (m1 == m2) {
        /* belongs to the same module */
        return 0;
    } else {
        /* different modules */
        return 1;
    }
}

static void
json_print_leaf(FILE *f, int level, struct lyd_node *node, int onlyvalue)
{
    struct lyd_node_leaf *leaf = (struct lyd_node_leaf *)node;
    LY_DATA_TYPE data_type;
    const char *schema;

    if (!onlyvalue) {
        if (!node->parent || nscmp(node, node->parent)) {
            /* print "namespace" */
            if (node->schema->module->type) {
                /* submodule, get module */
                schema = ((struct lys_submodule *)node->schema->module)->belongsto->name;
            } else {
                schema = node->schema->module->name;
            }
            fprintf(f, "%*s\"%s:%s\": ", LEVEL, INDENT, schema, node->schema->name);
        } else {
            fprintf(f, "%*s\"%s\": ", LEVEL, INDENT, node->schema->name);
        }
    }

    data_type = leaf->value_type;

    switch (data_type & LY_DATA_TYPE_MASK) {
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
        fprintf(f, "\"%s\"", leaf->value_str ? leaf->value_str : "");
        break;

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
        fprintf(f, "%s", leaf->value_str ? leaf->value_str : "");
        break;

    case LY_TYPE_LEAFREF:
        json_print_leaf(f, level, leaf->value.leafref, 1);
        break;

    case LY_TYPE_EMPTY:
        fprintf(f, "[null]");
        break;

    default:
        /* error */
        fprintf(f, "\"(!error!)\"");
    }

    if (!onlyvalue) {
        fprintf(f, "%s\n", lyd_is_last(node) ? "" : ",");
    }
    return;
}

static void
json_print_container(FILE *f, int level, struct lyd_node *node)
{
    const char *schema;
    struct lyd_node *child;

    if (!node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        if (node->schema->module->type) {
            /* submodule, get module */
            schema = ((struct lys_submodule *)node->schema->module)->belongsto->name;
        } else {
            schema = node->schema->module->name;
        }
        fprintf(f, "%*s\"%s:%s\": {\n", LEVEL, INDENT, schema, node->schema->name);
    } else {
        fprintf(f, "%*s\"%s\": {\n", LEVEL, INDENT, node->schema->name);
    }
    LY_TREE_FOR(node->child, child) {
        json_print_node(f, level + 1, child);
    }
    fprintf(f, "%*s}%s\n", LEVEL, INDENT, lyd_is_last(node) ? "" : ",");
}

static void
json_print_list_internal(FILE *f, int level, struct lyd_node_list *list)
{
    struct lyd_node *child;

    fprintf(f, "%*s{\n", LEVEL, INDENT);

    LY_TREE_FOR(list->child, child) {
        json_print_node(f, level + 1, child);
    }

    fprintf(f, "%*s}%s\n", LEVEL, INDENT, (list->lnext ? "," : ""));
}

static void
json_print_leaf_list(FILE *f, int level, struct lyd_node *node, int is_list)
{
    const char *schema;
    struct lyd_node_list *list = (struct lyd_node_list *)node;
    struct lyd_node_leaflist *llist = (struct lyd_node_leaflist *)node;

    if ((is_list && list->lprev) || (!is_list && llist->lprev)) {
        /* this list is already printed */
        return;
    }

    if (!node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        if (node->schema->module->type) {
            /* submodule, get module */
            schema = ((struct lys_submodule *)node->schema->module)->belongsto->name;
        } else {
            schema = node->schema->module->name;
        }
        fprintf(f, "%*s\"%s:%s\": [\n", LEVEL, INDENT, schema, node->schema->name);
    } else {
        fprintf(f, "%*s\"%s\": [\n", LEVEL, INDENT, node->schema->name);
    }

    if (!is_list) {
        ++level;
    }
    while ((is_list && list) || (!is_list &&  llist)) {
        if (is_list) {
            json_print_list_internal(f, level + 1, list);
            list = list->lnext;
        } else {
            fprintf(f, "%*s", LEVEL, INDENT);
            json_print_leaf(f, level, (struct lyd_node *)llist, 1);
            fprintf(f, "%s\n", (llist->lnext ? "," : ""));
            llist = llist->lnext;
        }
    }
    if (!is_list) {
        --level;
    }

    fprintf(f, "%*s]%s\n", LEVEL, INDENT, lyd_is_last(node) ? "" : ",");
}

static void
json_print_anyxml(FILE *f, int level, struct lyd_node *node)
{
    struct lyd_node_anyxml *axml = (struct lyd_node_anyxml *)node;

    fprintf(f, "%*s\"%s\": [null]%s\n", LEVEL, INDENT, axml->value->name, lyd_is_last(node) ? "" : ",");
}

void
json_print_node(FILE *f, int level, struct lyd_node *node)
{
    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
        json_print_container(f, level, node);
        break;
    case LYS_LEAF:
        json_print_leaf(f, level, node, 0);
        break;
    case LYS_LEAFLIST:
        json_print_leaf_list(f, level, node, 0);
        break;
    case LYS_LIST:
        json_print_leaf_list(f, level, node, 1);
        break;
    case LYS_ANYXML:
        json_print_anyxml(f, level, node);
        break;
    default:
        LOGINT;
        break;
    }
}

API int
json_print_data(FILE *f, struct lyd_node *root)
{
    int level = 0;
    struct lyd_node *node;

    /* start */
    fprintf(f, "{\n");

    /* content */
    LY_TREE_FOR(root, node) {
        json_print_node(f, level + 1, node);
    }

    /* end */
    fprintf(f, "}\n");

    return EXIT_SUCCESS;
}
