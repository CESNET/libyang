/**
 * @file printer/json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON printer for libyang data structure
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "common.h"
#include "printer.h"
#include "tree_data.h"
#include "resolve.h"
#include "tree_internal.h"

#define INDENT ""
#define LEVEL (level*2)

static void json_print_nodes(struct lyout *out, int level, const struct lyd_node *root, int withsiblings, int toplevel,
                             int options);

static int
json_print_string(struct lyout *out, const char *text)
{
    unsigned int i, n;

    if (!text) {
        return 0;
    }

    ly_write(out, "\"", 1);
    for (i = n = 0; text[i]; i++) {
        if (text[i] < 0x20) {
            /* control character */
            n += ly_print(out, "\\u%.4X");
        } else {
            switch (text[i]) {
            case '"':
                n += ly_print(out, "\\\"");
                break;
            case '\\':
                n += ly_print(out, "\\\\");
                break;
            default:
                ly_write(out, &text[i], 1);
                n++;
            }
        }
    }
    ly_write(out, "\"", 1);

    return n + 2;
}

static void
json_print_attrs(struct lyout *out, int level, const struct lyd_node *node, const struct lys_module *wdmod)
{
    struct lyd_attr *attr;

    if (wdmod) {
        ly_print(out, "%*s\"%s:default\":\"true\"", LEVEL, INDENT, wdmod->name);
        ly_print(out, "%s%s", node->attr ? "," : "", (level ? "\n" : ""));
    }
    for (attr = node->attr; attr; attr = attr->next) {
        if (attr->module != node->schema->module) {
            ly_print(out, "%*s\"%s:%s\":", LEVEL, INDENT, attr->module->name, attr->name);
        } else {
            ly_print(out, "%*s\"%s\":", LEVEL, INDENT, attr->name);
        }
        json_print_string(out, attr->value ? attr->value : "");
        ly_print(out, "%s%s", attr->next ? "," : "", (level ? "\n" : ""));
    }
}

static void
json_print_leaf(struct lyout *out, int level, const struct lyd_node *node, int onlyvalue, int toplevel, int options)
{
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node;
    const char *schema = NULL;
    const struct lys_module *wdmod = NULL;
    LY_DATA_TYPE datatype;

    if ((node->dflt && (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG))) ||
            (!node->dflt && (options & LYP_WD_ALL_TAG) && lyd_wd_default(leaf))) {
        /* we have implicit OR explicit default node */
        /* get with-defaults module */
        wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL);
    }

    if (!onlyvalue) {
        if (toplevel || !node->parent || nscmp(node, node->parent)) {
            /* print "namespace" */
            schema = lys_node_module(node->schema)->name;
            ly_print(out, "%*s\"%s:%s\":%s", LEVEL, INDENT, schema, node->schema->name, (level ? " " : ""));
        } else {
            ly_print(out, "%*s\"%s\":%s", LEVEL, INDENT, node->schema->name, (level ? " " : ""));
        }
    }

    datatype = leaf->value_type & LY_DATA_TYPE_MASK;
contentprint:
    switch (datatype) {
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
    case LY_TYPE_DEC64:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
    case LY_TYPE_BOOL:
        json_print_string(out, leaf->value_str);
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
        ly_print(out, "%s", leaf->value_str[0] ? leaf->value_str : "null");
        break;

    case LY_TYPE_LEAFREF:
        datatype = lyd_leaf_type(leaf);
        goto contentprint;

    case LY_TYPE_EMPTY:
        ly_print(out, "[null]");
        break;

    default:
        /* error */
        ly_print(out, "\"(!error!)\"");
    }

    /* print attributes as sibling leafs */
    if (!onlyvalue && (node->attr || wdmod)) {
        if (schema) {
            ly_print(out, ",%s%*s\"@%s:%s\":%s{%s", (level ? "\n" : ""), LEVEL, INDENT, schema, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        } else {
            ly_print(out, ",%s%*s\"@%s\":%s{%s", (level ? "\n" : ""), LEVEL, INDENT, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        }
        json_print_attrs(out, level + 1, node, wdmod);
        ly_print(out, "%*s}", LEVEL, INDENT);
    }

    return;
}

static void
json_print_container(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    const char *schema;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        schema = lys_node_module(node->schema)->name;
        ly_print(out, "%*s\"%s:%s\":%s{%s", LEVEL, INDENT, schema, node->schema->name, (level ? " " : ""), (level ? "\n" : ""));
    } else {
        ly_print(out, "%*s\"%s\":%s{%s", LEVEL, INDENT, node->schema->name, (level ? " " : ""), (level ? "\n" : ""));
    }
    if (level) {
        level++;
    }
    if (node->attr) {
        ly_print(out, "%*s\"@\":%s{%s", LEVEL, INDENT, (level ? " " : ""), (level ? "\n" : ""));
        json_print_attrs(out, (level? level + 1 : level), node, NULL);
        ly_print(out, "%*s}", LEVEL, INDENT);
        if (node->child) {
            ly_print(out, ",%s", (level ? "\n" : ""));
        }
    }
    json_print_nodes(out, level, node->child, 1, 0, options);
    if (level) {
        level--;
    }
    ly_print(out, "%*s}", LEVEL, INDENT);
}

static void
json_print_leaf_list(struct lyout *out, int level, const struct lyd_node *node, int is_list, int toplevel, int options)
{
    const char *schema = NULL;
    const struct lyd_node *list = node;
    int flag_empty = 0, flag_attrs = 0;

    if (is_list && !list->child) {
        /* empty, e.g. in case of filter */
        flag_empty = 1;
    }

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        schema = lys_node_module(node->schema)->name;
        ly_print(out, "%*s\"%s:%s\":", LEVEL, INDENT, schema, node->schema->name);
    } else {
        ly_print(out, "%*s\"%s\":", LEVEL, INDENT, node->schema->name);
    }

    if (flag_empty) {
        ly_print(out, "%snull", (level ? " " : ""));
        return;
    }
    ly_print(out, "%s[%s", (level ? " " : ""), (level ? "\n" : ""));

    if (!is_list && level) {
        ++level;
    }

    while (list) {
        if (is_list) {
            /* list print */
            if (level) {
                ++level;
            }
            ly_print(out, "%*s{%s", LEVEL, INDENT, (level ? "\n" : ""));
            if (level) {
                ++level;
            }
            if (list->attr) {
                ly_print(out, "%*s\"@\":%s{%s", LEVEL, INDENT, (level ? " " : ""), (level ? "\n" : ""));
                json_print_attrs(out, level + 1, list, NULL);
                ly_print(out, "%*s}%s", LEVEL, INDENT, list->child ? ",\n" : "");
            }
            json_print_nodes(out, level, list->child, 1, 0, options);
            if (level) {
                --level;
            }
            ly_print(out, "%*s}", LEVEL, INDENT);
            if (level) {
                --level;
            }
        } else {
            /* leaf-list print */
            ly_print(out, "%*s", LEVEL, INDENT);
            json_print_leaf(out, level, list, 1, toplevel, options);
            if (list->attr) {
                flag_attrs = 1;
            }
        }
        for (list = list->next; list && list->schema != node->schema; list = list->next);
        if (list) {
            ly_print(out, ",%s", (level ? "\n" : ""));
        }
    }

    if (!is_list && level) {
        --level;
    }

    ly_print(out, "%s%*s]", (level ? "\n" : ""), LEVEL, INDENT);

    /* attributes */
    if (!is_list && flag_attrs) {
        if (schema) {
            ly_print(out, ",%s%*s\"@%s:%s\":%s[%s", (level ? "\n" : ""), LEVEL, INDENT, schema, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        } else {
            ly_print(out, ",%s%*s\"@%s\":%s[%s", (level ? "\n" : ""), LEVEL, INDENT, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        }
        if (level) {
            level++;
        }
        for (list = node; list; ) {
            if (list->attr) {
                ly_print(out, "%*s{%s", LEVEL, INDENT, (level ? " " : ""));
                json_print_attrs(out, 0, list, NULL);
                ly_print(out, "%*s}", LEVEL, INDENT);
            } else {
                ly_print(out, "%*snull", LEVEL, INDENT);
            }


            for (list = list->next; list && list->schema != node->schema; list = list->next);
            if (list) {
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
        }
        if (level) {
            level--;
        }
        ly_print(out, "%s%*s]", (level ? "\n" : ""), LEVEL, INDENT);
    }
}

static void
json_print_anydata(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    const char *schema = NULL;
    struct lyd_node_anydata *any = (struct lyd_node_anydata *)node;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        schema = lys_node_module(node->schema)->name;
        ly_print(out, "%*s\"%s:%s\":%s{%s", LEVEL, INDENT, schema, node->schema->name, (level ? " " : ""), (level ? "\n" : ""));
    } else {
        ly_print(out, "%*s\"%s\":%s{%s", LEVEL, INDENT, node->schema->name, (level ? " " : ""), (level ? "\n" : ""));
    }
    if (level) {
        level++;
    }

    switch (any->value_type) {
    case LYD_ANYDATA_DATATREE:
        json_print_nodes(out, level, any->value.tree, 1, 0, options);
        break;
    case LYD_ANYDATA_JSON:
        if (any->value.str) {
            ly_print(out, "%*s%s\n", LEVEL, INDENT, any->value.str);
        }
        break;
    default:
        /* other formats are not supported */
        LOGWRN("Unable to print anydata content (type %d) as JSON.", any->value_type);
        break;
    }

    /* print attributes as sibling leaf */
    if (node->attr) {
        if (schema) {
            ly_print(out, ",\n%*s\"@%s:%s\": {\n", LEVEL, INDENT, schema, node->schema->name);
        } else {
            ly_print(out, ",\n%*s\"@%s\": {\n", LEVEL, INDENT, node->schema->name);
        }
        json_print_attrs(out, (level ? level + 1 : level), node, NULL);
        ly_print(out, "%*s}", LEVEL, INDENT);
    }


    if (level) {
        level--;
    }
    ly_print(out, "%*s}", LEVEL, INDENT);
}

static void
json_print_nodes(struct lyout *out, int level, const struct lyd_node *root, int withsiblings, int toplevel, int options)
{
    const struct lyd_node *node, *iter;

    LY_TREE_FOR(root, node) {
        if (!lyd_wd_toprint(node, options)) {
            continue;
        }

        switch (node->schema->nodetype) {
        case LYS_RPC:
        case LYS_NOTIF:
        case LYS_CONTAINER:
            if (node->prev->next) {
                /* print the previous comma */
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
            json_print_container(out, level, node, toplevel, options);
            break;
        case LYS_LEAF:
            if (node->prev->next) {
                /* print the previous comma */
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
            json_print_leaf(out, level, node, 0, toplevel, options);
            break;
        case LYS_LEAFLIST:
        case LYS_LIST:
            /* is it already printed? */
            for (iter = node->prev; iter->next; iter = iter->prev) {
                if (iter == node) {
                    continue;
                }
                if (iter->schema == node->schema) {
                    /* the list has alread some previous instance and therefore it is already printed */
                    break;
                }
            }
            if (!iter->next) {
                if (node->prev->next) {
                    /* print the previous comma */
                    ly_print(out, ",%s", (level ? "\n" : ""));
                }

                /* print the list/leaflist */
                json_print_leaf_list(out, level, node, node->schema->nodetype == LYS_LIST ? 1 : 0, toplevel, options);
            }
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (node->prev->next) {
                /* print the previous comma */
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
            json_print_anydata(out, level, node, toplevel, options);
            break;
        default:
            LOGINT;
            break;
        }

        if (!withsiblings) {
            break;
        }
    }
    if (root && level) {
        ly_print(out, "\n");
    }
}

int
json_print_data(struct lyout *out, const struct lyd_node *root, int options)
{
    int level = 0;

    if (options & LYP_FORMAT) {
        ++level;
    }

    /* start */
    ly_print(out, "{%s", (level ? "\n" : ""));

    /* content */
    json_print_nodes(out, level, root, options & LYP_WITHSIBLINGS, 1, options);

    /* end */
    ly_print(out, "}%s", (level ? "\n" : ""));
    ly_print_flush(out);

    return EXIT_SUCCESS;
}
