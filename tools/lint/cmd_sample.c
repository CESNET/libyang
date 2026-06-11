/**
 * @file cmd_sample.c
 * @author Petr Hanzlik <Petr.Hanzlik@cesnet.cz>
 * @brief 'sample-skeleton' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "cmd.h"

#include <getopt.h>
#include <stdint.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

#define SPACE_COUNT 2

static void print_xml_nodes(struct ly_out *out, const struct lysc_node *node, int space_count);
static void print_json_nodes(struct ly_out *out, const struct lysc_node *node, const char *name, int space_count, int *need_comma);

void
cmd_sample_help(void)
{
    printf("Usage: sample [-f (xml | json)] [-o OUTFILE] <module-name>[@revision]\n"
            "                  Generate and print a sample data skeleton from a loaded schema module.\n\n"
            "  -f FORMAT, --format=FORMAT\n"
            "                  Print the sample skeleton in the specified FORMAT.\n"
            "                  Supported formats: xml, json.\n"
            "  -o OUTFILE, --output=OUTFILE\n"
            "                  Write the output to OUTFILE instead of stdout.\n"
            "  -h, --help\n"
            "                  Display this help message.\n");
}

int
cmd_sample_dep(struct yl_opt *yo, int posc)
{
    if (yo->interactive && !posc) {
        YLMSG_E("Missing module name to generate skeleton for.\n");
        return 1;
    }

    if (!yo->data_out_format) {
        YLMSG_E("Missing required format (-f xml | json).\n");
        return 1;
    }

    return 0;
}

int
cmd_sample_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    char **argv = NULL;
    struct option options[] = {
        {"format", required_argument, NULL, 'f'},
        {"output", required_argument, NULL, 'o'},
        {"help",   no_argument,       NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &argv))) {
        return rc;
    }

    /* reset getopt */
    optind = 0;
    while ((opt = getopt_long(argc, argv, "f:o:h", options, &opt_index)) != -1) {
        switch (opt) {
        case 'f': /* --format */
            if (yl_opt_update_data_out_format(optarg, yo)) {
                rc = 1;
                goto cleanup;
            }
            break;

        case 'o': /* --output */
            if (yo->out) {
                ly_out_free(yo->out, NULL, 0);
                yo->out = NULL;
            }
            if (ly_out_new_filepath(optarg, &yo->out)) {
                YLMSG_E("Unable to open output file %s.\n", optarg);
                rc = 1;
                goto cleanup;
            }
            break;

        case 'h': /* --help */
            cmd_sample_help();
            rc = 1;
            goto cleanup;

        default:
            YLMSG_E("Unknown option.\n");
            rc = 1;
            goto cleanup;
        }
    }

    *posc = argc - optind;
    *posv = &argv[optind];

    return 0;

cleanup:
    free_cmdline(argv);
    return rc;
}

/**
 * @brief Print a specific number of spaces for indentation.
 *
 * @param[in,out] out Libyang output handler to print to.
 * @param[in] count Number of spaces to print.
 */
static void
print_space(struct ly_out *out, int count)
{
    for (int i = 0; i < count; i++) {
        ly_print(out, " ");
    }
}

/**
 * @brief Handle and print the default values or closing XML tags for a schema node.
 *
 * @param[in,out] out Libyang output handler to print to.
 * @param[in] node Compiled schema node currently being processed.
 * @param[in] space_count Current indentation level (number of spaces).
 */
static void
handle_xml_default(struct ly_out *out, const struct lysc_node *node, int space_count)
{
    switch (node->nodetype) {
    case LYS_LEAF: {
        struct lysc_node_leaf *leaf = (struct lysc_node_leaf *)node;

        if (leaf->dflt.str) {
            ly_print(out, ">%s</%s>\n", leaf->dflt.str, node->name);
        } else {
            ly_print(out, "/>\n");
        }
        break;
    }
    case LYS_LEAFLIST: {
        struct lysc_node_leaflist *llist = (struct lysc_node_leaflist *)node;

        if (llist->dflts) {
            LY_ARRAY_COUNT_TYPE i;

            LY_ARRAY_FOR(llist->dflts, i) {
                print_space(out, space_count);
                if (node->parent && (node->module != node->parent->module)) {
                    ly_print(out, "<%s xmlns=\"%s\">%s</%s>\n", llist->name, node->module->ns, llist->dflts[i].str, llist->name);
                } else {
                    ly_print(out, "<%s>%s</%s>\n", llist->name, llist->dflts[i].str, llist->name);
                }
            }
        } else {
            print_space(out, space_count);
            if (node->parent && (node->module != node->parent->module)) {
                ly_print(out, "<%s xmlns=\"%s\"></%s>\n", llist->name, node->module->ns, llist->name);
            } else {
                ly_print(out, "<%s></%s>\n", llist->name, llist->name);
            }
        }
        break;
    }
    case LYS_CHOICE: {
        struct lysc_node_choice *choice = (struct lysc_node_choice *)node;

        if (choice->dflt) {
            print_xml_nodes(out, lysc_node_child((const struct lysc_node *)choice->dflt), space_count);
        } else {
            if (lysc_node_child(node)) {
                print_xml_nodes(out, lysc_node_child(lysc_node_child(node)), space_count);
            }
        }
        break;
    }
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_ANYDATA:
    case LYS_ANYXML:
        ly_print(out, "/>\n");
        break;
    }
}

/**
 * @brief Traverse the compiled schema tree and print XML skeleton tags.
 *
 * @param[in,out] out Libyang output handler to print to.
 * @param[in] node Compiled schema node to begin traversing.
 * @param[in] space_count Current indentation level.
 */
static void
print_xml_nodes(struct ly_out *out, const struct lysc_node *node, int space_count)
{
    const uint32_t PRINT_MASK = LYS_CONTAINER | LYS_LIST | LYS_LEAF | LYS_ANYXML | LYS_ANYDATA;
    int nodetype;

    while (node) {
        nodetype = node->nodetype;

        if (nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            node = node->next;
            continue;
        }

        if (nodetype & PRINT_MASK) {
            print_space(out, space_count);

            if (!node->parent) {
                ly_print(out, "<%s xmlns=\"%s\"", node->name, node->module->ns);
            } else if (node->parent && (node->module != node->parent->module)) {
                ly_print(out, "<%s xmlns=\"%s\"", node->name, node->module->ns);
            } else {
                ly_print(out, "<%s", node->name);
            }
        }

        if (lysc_node_child(node)) {
            if (nodetype & PRINT_MASK) {
                ly_print(out, ">\n");
            }

            if (nodetype == LYS_CHOICE) {
                handle_xml_default(out, node, space_count);
                node = node->next;
                continue;
            } else {
                print_xml_nodes(out, lysc_node_child(node), space_count + SPACE_COUNT);
            }

            if (nodetype & PRINT_MASK) {
                print_space(out, space_count);
                ly_print(out, "</%s>\n", node->name);
            }
        } else {
            handle_xml_default(out, node, space_count);
        }

        node = node->next;
    }
}

/**
 * @brief Initialize and print the top-level XML skeleton structure.
 *
 * @param[in,out] out  Libyang output handler to print to.
 * @param[in] root Root compiled schema node of the module being processed.
 */
static void
print_xml_skeleton(struct ly_out *out, struct lysc_node *root)
{
    struct lysc_node *node = root;
    int spaces = SPACE_COUNT;

    ly_print(out, "<?xml version=\'1.0\' encoding=\'UTF-8\'?>\n"
            "<data xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n");
    print_xml_nodes(out, node, spaces);
    ly_print(out, "</data>\n");

}

/**
 * @brief Determine if a YANG base type requires string quotes in JSON encoding.
 *
 * @param[in] basetype Libyang data type to evaluate.
 * @return 1 if the type requires quotes (string-like), 0 if it must be printed raw (numeric/boolean).
 */
static int
is_json_quote_required(LY_DATA_TYPE basetype)
{
    switch (basetype) {
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_BOOL:
        return 0;

    case LY_TYPE_UNKNOWN:
    case LY_TYPE_STRING:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
    case LY_TYPE_DEC64:
    case LY_TYPE_BINARY:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
    case LY_TYPE_LEAFREF:
    case LY_TYPE_UNION:
    case LY_TYPE_EMPTY:
        return 1;
    }

    return 1;
}

/**
 * @brief Handle and print the default values or closing JSON tags for a schema node.
 *
 * @param[in,out] out Libyang output handler to print to.
 * @param[in] node Compiled schema node currently being processed.
 * @param[in] space_count Current indentation level (number of spaces).
 * @param[in,out] need_comma  Pointer to the comma state variable.
 */
static void
handle_json_default(struct ly_out *out, const struct lysc_node *node, int space_count, int *need_comma)
{
    switch (node->nodetype) {
    case LYS_LEAF: {
        struct lysc_node_leaf *leaf = (struct lysc_node_leaf *)node;

        if (leaf->dflt.str) {
            if (is_json_quote_required(leaf->type->basetype)) {
                ly_print(out, " \"%s\"", leaf->dflt.str);
            } else {
                ly_print(out, " %s", leaf->dflt.str);
            }
        } else {
            if (leaf->type->basetype == LY_TYPE_EMPTY) {
                ly_print(out, " [null]");
            } else {
                ly_print(out, " null");
            }
        }
        break;
    }
    case LYS_LEAFLIST: {
        struct lysc_node_leaflist *llist = (struct lysc_node_leaflist *)node;

        if (llist->dflts) {
            LY_ARRAY_COUNT_TYPE i;

            ly_print(out, " [\n");

            LY_ARRAY_FOR(llist->dflts, i) {
                if (i == 0) {
                    print_space(out, space_count + SPACE_COUNT);
                } else {
                    ly_print(out, ",\n");
                    print_space(out, space_count + SPACE_COUNT);
                }

                if (is_json_quote_required(llist->type->basetype)) {
                    ly_print(out, "\"%s\"", llist->dflts[i].str);
                } else {
                    ly_print(out, "%s", llist->dflts[i].str);
                }
            }
            ly_print(out, "\n");
            print_space(out, space_count);
            ly_print(out, "]");
        } else {
            ly_print(out, " []");
        }
        break;
    }
    case LYS_CHOICE: {
        struct lysc_node_choice *choice = (struct lysc_node_choice *)node;

        if (choice->dflt) {
            print_json_nodes(out, lysc_node_child((const struct lysc_node *)choice->dflt), NULL, space_count, need_comma);
        } else {
            if (lysc_node_child(node)) {
                print_json_nodes(out, lysc_node_child(lysc_node_child(node)), NULL, space_count, need_comma);
            }
        }
        break;
    }
    case LYS_CONTAINER:
    case LYS_ANYDATA:
    case LYS_ANYXML:
        ly_print(out, " {}");
        break;
    case LYS_LIST:
        ly_print(out, " []");
        break;
    }
}

/**
 * @brief Traverse the compiled schema tree and print JSON skeleton tags.
 *
 * @param[in,out] out Libyang output handler to print to.
 * @param[in] node Compiled schema node to begin traversing.
 * @param[in] name Name of the module, or NULL if inherited.
 * @param[in] space_count Current indentation level; dynamically incremented and decremented during traversal.
 * @param[in,out] need_comma  Pointer to the state variable tracking JSON comma formatting. Set to 1 if a trailing comma is required before printing the  * next sibling, or 0 if the node is the first element in a block.
 */
static void
print_json_nodes(struct ly_out *out, const struct lysc_node *node, const char *name, int space_count, int *need_comma)
{
    int nodetype;
    const uint32_t PRINT_MASK = LYS_CONTAINER | LYS_LIST | LYS_LEAF | LYS_ANYXML | LYS_ANYDATA | LYS_LEAFLIST;

    while (node) {
        nodetype = node->nodetype;

        if (nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            node = node->next;
            continue;
        }

        if (nodetype & PRINT_MASK) {
            if (*need_comma) {
                ly_print(out, ",\n");
            }

            print_space(out, space_count);

            if (name && !node->parent) {
                ly_print(out, "\"%s:%s\":", node->module->name, node->name);
            } else if (node->parent && (node->module != node->parent->module)) {
                ly_print(out, "\"%s:%s\":", node->module->name, node->name);
            } else {
                ly_print(out, "\"%s\":", node->name);
            }
        }

        if (lysc_node_child(node)) {
            if (nodetype & PRINT_MASK) {
                if (nodetype == LYS_LIST) {
                    ly_print(out, " [\n");
                    print_space(out, space_count + SPACE_COUNT);
                    ly_print(out, "{\n");
                    space_count += 2 * SPACE_COUNT;
                } else {
                    ly_print(out, " {\n");
                    space_count += SPACE_COUNT;
                }
                *need_comma = 0;
            }

            if (nodetype == LYS_CHOICE) {
                handle_json_default(out, node, space_count, need_comma);
                node = node->next;
                continue;
            } else {
                print_json_nodes(out, lysc_node_child(node), NULL, space_count, need_comma);
            }

            if (nodetype & PRINT_MASK) {
                if (nodetype == LYS_LIST) {
                    space_count -= SPACE_COUNT;
                    ly_print(out, "\n");
                    print_space(out, space_count);
                    ly_print(out, "}\n");
                    space_count -= SPACE_COUNT;
                    print_space(out, space_count);
                    ly_print(out, "]");
                } else {
                    space_count -= SPACE_COUNT;
                    ly_print(out, "\n");
                    print_space(out, space_count);
                    ly_print(out, "}");
                }
                *need_comma = 1;
            }
        } else {
            handle_json_default(out, node, space_count, need_comma);

            if (nodetype & PRINT_MASK) {
                *need_comma = 1;
            }
        }

        node = node->next;
    }
}

/**
 * @brief Initialize and print the top-level JSON skeleton structure.
 *
 * @param[in,out] out  Libyang output handler to print to.
 * @param[in] root Root compiled schema node of the module being processed.
 * @param[in] name Name of the module, used to correctly prefix the top-level JSON objects according to RFC 7951 namespace rules.
 */
static void
print_json_skeleton(struct ly_out *out, struct lysc_node *root, const char *name)
{
    struct lysc_node *node = root;
    int spaces = SPACE_COUNT, need_comma = 0;

    ly_print(out, "{\n");
    print_json_nodes(out, node, name, spaces, &need_comma);
    ly_print(out, "\n}\n");
}

int
cmd_sample_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    const struct lys_module *mod;
    char *revision, *name;
    uint8_t out_alloc = 0;

    name = strdup(posv);
    if (!name) {
        YLMSG_E("Memory allocation failed.");
        return 1;
    }
    revision = strchr(name, '@');
    if (revision) {
        revision[0] = '\0';
        ++revision;
    }

    mod = revision ?
            ly_ctx_get_module(*ctx, name, revision) :
            ly_ctx_get_module_latest(*ctx, name);

    if (!mod || !mod->compiled) {
        YLMSG_E("Error: Module %s not loaded or has no compiled schema.", name);
        free(name);
        return 1;
    }

    if (!mod->compiled->data) {
        free(name);
        return 0;
    }

    if (!yo->out) {
        if (ly_out_new_file(stdout, &yo->out)) {
            YLMSG_E("Unable to allocate output handler.");
            free(name);
            return 1;
        }
        out_alloc = 1;
    }

    if (yo->data_out_format == LYD_XML) {
        print_xml_skeleton(yo->out, mod->compiled->data);
    } else {
        print_json_skeleton(yo->out, mod->compiled->data, mod->name);
    }

    if (out_alloc) {
        ly_out_free(yo->out, NULL, 0);
        yo->out = NULL;
    }

    free(name);
    return 0;
}
