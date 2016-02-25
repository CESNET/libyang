/**
 * @file commands.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool commands
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include "commands.h"
#include "../../src/libyang.h"
#include "../../src/tree_schema.h"
#include "../../src/tree_data.h"
#include "../../src/parser.h"
#include "../../src/xpath.h"

COMMAND commands[];
extern int done;
extern struct ly_ctx *ctx;

void
cmd_add_help(void)
{
    printf("add <path-to-model> [<other-models> ...]\n");
}

void
cmd_print_help(void)
{
    printf("print [-f (yang | yin | tree | info)] [-t <info-target-node>] [-o <output-file>] <model-name>[@<revision>]\n\n");
    printf("\tinfo-target-node: <absolute-schema-node> | typedef/<typedef-name> |\n");
    printf("\t                  | identity/<identity-name> | feature/<feature-name> |\n");
    printf("\t                  | grouping/<grouping-name>(<absolute-schema-nodeid>) |\n");
    printf("\t                  | type/<absolute-schema-node-leaf-or-leaflist>\n");
    printf("\n");
    printf("\tabsolute-schema-nodeid: ( /(<import-prefix>:)<node-identifier> )+\n");
}

void
cmd_data_help(void)
{
    printf("data [-(-s)trict] [-x OPTION] [-o <output-file>] [-f (xml | json)]  <data-file-name>\n");
    printf("Accepted OPTIONs:\n");
    printf("\tauto       - resolve data type (one of the following) automatically (as pyang does),\n");
    printf("\t             this option is applicable only in case of XML input data.\n");
    printf("\tconfig     - LYD_OPT_CONFIG\n");
    printf("\tget        - LYD_OPT_GET\n");
    printf("\tgetconfig  - LYD_OPT_GETCONFIG\n");
    printf("\tedit       - LYD_OPT_EDIT\n");
    printf("\trpc        - LYD_OPT_RPC\n");
    /* printf("\trpcreply   - LYD_OPT_RPCREPLY\n"); */
    printf("\tnotif      - LYD_OPT_NOTIF\n");
    printf("\tfilter     - LYD_OPT_FILTER\n");
}

void
cmd_xpath_help(void)
{
    printf("xpath -e <XPath-expression> <XML-data-file-name>\n");
}

void
cmd_list_help(void)
{
    printf("list [-f (xml | json)]\n");
}

void
cmd_feature_help(void)
{
    printf("feature [ -(-e)nable | -(-d)isable (* | <feature-name>[,<feature-name> ...]) ] <model-name>[@<revision>]\n");
}

void
cmd_searchpath_help(void)
{
    printf("searchpath <model-dir-path>\n");
}

void
cmd_verb_help(void)
{
    printf("verb (error/0 | warning/1 | verbose/2 | debug/3)\n");
}

int
cmd_add(const char *arg)
{
    int path_len;
    char *ptr, *path;
    const char *arg_ptr;
    const struct lys_module *model;
    LYS_INFORMAT format;

    if (strlen(arg) < 5) {
        cmd_add_help();
        return 1;
    }

    arg_ptr = arg + strlen("add ");
    while (arg_ptr[0] == ' ') {
        ++arg_ptr;
    }
    if (strchr(arg_ptr, ' ')) {
        path_len = strchr(arg_ptr, ' ') - arg_ptr;
    } else {
        path_len = strlen(arg_ptr);
    }

    path = strndup(arg_ptr, path_len);

    while (path) {
        if ((ptr = strrchr(path, '.')) != NULL) {
            ++ptr;
            if (!strcmp(ptr, "yin")) {
                format = LYS_IN_YIN;
            } else if (!strcmp(ptr, "yang")) {
                format = LYS_IN_YANG;
            } else {
                fprintf(stderr, "Input file in an unknown format \"%s\".\n", ptr);
                free(path);
                return 1;
            }
        } else {
            fprintf(stdout, "Input file \"%.*s\" without extension, assuming YIN format.\n", path_len, arg_ptr);
            format = LYS_IN_YIN;
        }

        model = lys_parse_path(ctx, path, format);
        free(path);

        if (!model) {
            /* libyang printed the error messages */
            return 1;
        }

        /* next model */
        arg_ptr += path_len;
        while (arg_ptr[0] == ' ') {
            ++arg_ptr;
        }
        if (strchr(arg_ptr, ' ')) {
            path_len = strchr(arg_ptr, ' ') - arg_ptr;
        } else {
            path_len = strlen(arg_ptr);
        }

        if (path_len) {
            path = strndup(arg_ptr, path_len);
        } else {
            path = NULL;
        }
    }

    return 0;
}

int
cmd_print(const char *arg)
{
    int c, i, argc, option_index, ret = 1;
    char **argv = NULL, *ptr, *target_node = NULL, *model_name, *revision;
    const char **names, *out_path = NULL;
    const struct lys_module *model, *parent_model;
    LYS_OUTFORMAT format = LYS_OUT_TREE;
    FILE *output = stdout;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {"target-node", required_argument, 0, 't'},
        {NULL, 0, 0, 0}
    };

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        argv = realloc(argv, (argc+2)*sizeof *argv);
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hf:o:t:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_print_help();
            ret = 0;
            goto cleanup;
        case 'f':
            if (!strcmp(optarg, "yang")) {
                format = LYS_OUT_YANG;
            } else if (!strcmp(optarg, "yin")) {
                format = LYS_OUT_YIN;
            } else if (!strcmp(optarg, "tree")) {
                format = LYS_OUT_TREE;
            } else if (!strcmp(optarg, "info")) {
                format = LYS_OUT_INFO;
            } else {
                fprintf(stderr, "Unknown output format \"%s\".\n", optarg);
                goto cleanup;
            }
            break;
        case 'o':
            if (out_path) {
                fprintf(stderr, "Output specified twice.\n");
                goto cleanup;
            }
            out_path = optarg;
            break;
        case 't':
            target_node = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* file name */
    if (optind == argc) {
        fprintf(stderr, "Missing the model name.\n");
        goto cleanup;
    }

    /* model, revision */
    model_name = argv[optind];
    revision = NULL;
    if (strchr(model_name, '@')) {
        revision = strchr(model_name, '@');
        revision[0] = '\0';
        ++revision;
    }

    model = ly_ctx_get_module(ctx, model_name, revision);
    if (model == NULL) {
        names = ly_ctx_get_module_names(ctx);
        for (i = 0; names[i]; i++) {
            if (!model) {
                parent_model = ly_ctx_get_module(ctx, names[i], NULL);
                model = (struct lys_module *)ly_ctx_get_submodule(parent_model, model_name, revision);
            }
        }
        free(names);
    }

    if (model == NULL) {
        if (revision) {
            fprintf(stderr, "No model \"%s\" in revision %s found.\n", model_name, revision);
        } else {
            fprintf(stderr, "No model \"%s\" found.\n", model_name);
        }
        goto cleanup;
    }

    if (out_path) {
        output = fopen(out_path, "w");
        if (!output) {
            fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
            goto cleanup;
        }
    }

    ret = lys_print_file(output, model, format, target_node);

cleanup:
    free(*argv);
    free(argv);

    if (output && (output != stdout)) {
        fclose(output);
    }

    return ret;
}

int
cmd_data(const char *arg)
{
    int c, argc, option_index, ret = 1;
    int options = 0;
    size_t len;
    char **argv = NULL, *ptr;
    const char *out_path = NULL;
    struct lyd_node *data = NULL;
    struct lyxml_elem *xml;
    LYD_FORMAT outformat = LYD_UNKNOWN, informat = LYD_UNKNOWN;
    FILE *output = stdout;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"option", required_argument, 0, 'x'},
        {"output", required_argument, 0, 'o'},
        {"strict", no_argument, 0, 's'},
        {NULL, 0, 0, 0}
    };

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        argv = realloc(argv, (argc+2)*sizeof *argv);
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hf:o:sx:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_data_help();
            ret = 0;
            goto cleanup;
        case 'f':
            if (!strcmp(optarg, "xml")) {
                outformat = LYD_XML_FORMAT;
            } else if (!strcmp(optarg, "json")) {
                outformat = LYD_JSON;
            } else {
                fprintf(stderr, "Unknown output format \"%s\".\n", optarg);
                goto cleanup;
            }
            break;
        case 'o':
            if (out_path) {
                fprintf(stderr, "Output specified twice.\n");
                goto cleanup;
            }
            out_path = optarg;
            break;
        case 's':
            options |= LYD_OPT_STRICT;
            options |= LYD_OPT_OBSOLETE;
            break;
        case 'x':
            if (!strcmp(optarg, "auto")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_TYPEMASK;
            } else if (!strcmp(optarg, "config")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
            } else if (!strcmp(optarg, "get")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
            } else if (!strcmp(optarg, "getconfig")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
            } else if (!strcmp(optarg, "edit")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
            } else if (!strcmp(optarg, "rpc")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
            /* support for RPC replies is missing, because it requires to provide
             * also pointer to the reply's RPC request
            } else if (!strcmp(optarg, "rpcreply")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
             */
            } else if (!strcmp(optarg, "notif")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
            } else if (!strcmp(optarg, "filter")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_FILTER;
            }
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* file name */
    if (optind == argc) {
        fprintf(stderr, "Missing the data file name.\n");
        goto cleanup;
    }

    /* detect input format according to file suffix */
    len = strlen(argv[optind]);
    if (len >= 5 && !strcmp(&argv[optind][len - 4], ".xml")) {
        informat = LYD_XML;
    } else if (len >= 6 && !strcmp(&argv[optind][len - 5], ".json")) {
        informat = LYD_JSON;
    } else {
        fprintf(stderr, "Unable to resolve format of the input file, please add \".xml\" or \".json\" suffix.\n");
        goto cleanup;
    }

    if ((options & LYD_OPT_TYPEMASK) == LYD_OPT_TYPEMASK) {
        /* automatically detect data type from the data top level */
        if (informat != LYD_XML) {
            fprintf(stderr, "Only XML data can be automatically explored.\n");
            goto cleanup;
        }

        xml = lyxml_parse_path(ctx, argv[optind], 0);
        if (!xml) {
            fprintf(stderr, "Failed to parse XML data for automatic type detection.\n");
            goto cleanup;
        }

        /* NOTE: namespace is ignored to simplify usage of this feature */

        if (!strcmp(xml->name, "data")) {
            fprintf(stdout, "Parsing %s as complete datastore.\n", argv[optind]);
            options = options & ~LYD_OPT_TYPEMASK;
        } else if (!strcmp(xml->name, "config")) {
            fprintf(stdout, "Parsing %s as config data.\n", argv[optind]);
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
        } else if (!strcmp(xml->name, "get-reply")) {
            fprintf(stdout, "Parsing %s as <get> reply data.\n", argv[optind]);
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
        } else if (!strcmp(xml->name, "get-config-reply")) {
            fprintf(stdout, "Parsing %s as <get-config> reply data.\n", argv[optind]);
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
        } else if (!strcmp(xml->name, "edit-config")) {
            fprintf(stdout, "Parsing %s as <edit-config> data.\n", argv[optind]);
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
        } else if (!strcmp(xml->name, "rpc")) {
            fprintf(stdout, "Parsing %s as <rpc> data.\n", argv[optind]);
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
        /* support for RPC replies is missing, because it requires to provide
         * also pointer to the reply's RPC request
        } else if (!strcmp(xml->name, "rpc-reply")) {
            fprintf(stdout, "Parsing %s as <rpc-reply> data.\n");
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
         */
        } else if (!strcmp(xml->name, "notification")) {
            fprintf(stdout, "Parsing %s as <notification> data.\n", argv[optind]);
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
        } else if (!strcmp(xml->name, "filter")) {
            fprintf(stdout, "Parsing %s as subtree filter data.\n", argv[optind]);
            options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_FILTER;
        } else {
            fprintf(stderr, "Invalid top-level element for automatic data type recognition.\n");
            lyxml_free(ctx, xml);
            goto cleanup;
        }

        data = lyd_parse_xml(ctx, &xml->child, options);
        lyxml_free(ctx, xml);
    } else {
        data = lyd_parse_path(ctx, argv[optind], informat, options);
    }
    if (ly_errno) {
        fprintf(stderr, "Failed to parse data.\n");
        goto cleanup;
    }

    if (out_path) {
        output = fopen(out_path, "w");
        if (!output) {
            fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
            goto cleanup;
        }
    }

    if (outformat != LYD_UNKNOWN) {
        lyd_print_file(output, data, outformat, 1);
    }

    ret = 0;

cleanup:
    free(*argv);
    free(argv);

    if (output && (output != stdout)) {
        fclose(output);
    }

    lyd_free_withsiblings(data);

    return ret;
}

int
cmd_xpath(const char *arg)
{
    int c, argc, option_index, ret = 1, long_str;
    char **argv = NULL, *ptr, *expr = NULL;
    unsigned int i, j;
    struct lyd_node *data = NULL, *node;
    struct ly_set *set, *keys;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"expr", required_argument, 0, 'e'},
        {NULL, 0, 0, 0}
    };

    long_str = 0;
    argc = 1;
    argv = malloc(2 * sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        if (long_str) {
            ptr[-1] = ' ';
            if (ptr[strlen(ptr) - 1] == long_str) {
                long_str = 0;
                ptr[strlen(ptr) - 1] = '\0';
            }
        } else {
            argv = realloc(argv, (argc + 2) * sizeof *argv);
            argv[argc] = ptr;
            if (ptr[0] == '"') {
                long_str = '"';
                ++argv[argc];
            }
            if (ptr[0] == '\'') {
                long_str = '\'';
                ++argv[argc];
            }
            if (ptr[strlen(ptr) - 1] == long_str) {
                long_str = 0;
                ptr[strlen(ptr) - 1] = '\0';
            }
            ++argc;
        }
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "he:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_xpath_help();
            ret = 0;
            goto cleanup;
        case 'e':
            expr = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    if (optind == argc) {
        fprintf(stderr, "Missing the file with data.\n");
        goto cleanup;
    }

    if (!expr) {
        fprintf(stderr, "Missing the XPath expression.\n");
        goto cleanup;
    }

    /* data file */
    data = lyd_parse_path(ctx, argv[optind], LYD_XML, 0);
    if (ly_errno) {
        fprintf(stderr, "Failed to parse data.\n");
        goto cleanup;
    }

    if (!(set = lyd_get_node(data, expr))) {
        goto cleanup;
    }

    /* print result */
    printf("Result:\n");
    if (!set->number) {
        printf("\tEmpty\n");
    } else {
        for (i = 0; i < set->number; ++i) {
            node = set->dset[i];
            switch (node->schema->nodetype) {
            case LYS_CONTAINER:
                printf("\tContainer ");
                break;
            case LYS_LEAF:
                printf("\tLeaf ");
                break;
            case LYS_LEAFLIST:
                printf("\tLeaflist ");
                break;
            case LYS_LIST:
                printf("\tList ");
                break;
            case LYS_ANYXML:
                printf("\tAnyxml ");
                break;
            default:
                printf("\tUnknown ");
                break;
            }
            printf("\"%s\"", node->schema->name);
            if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                printf(" (val: %s)", ((struct lyd_node_leaf_list *)node)->value_str);
            } else if (node->schema->nodetype == LYS_LIST) {
                keys = lyd_get_list_keys(node);
                if (keys && keys->number) {
                    printf(" (");
                    for (j = 0; j < keys->number; ++j) {
                        if (j) {
                            printf(" ");
                        }
                        printf("\"%s\": %s", keys->dset[j]->schema->name,
                               ((struct lyd_node_leaf_list *)keys->dset[j])->value_str);
                    }
                    printf(")");
                }
                ly_set_free(keys);
            }
            printf("\n");
        }
    }
    printf("\n");

    ly_set_free(set);
    ret = 0;

cleanup:
    free(*argv);
    free(argv);

    lyd_free_withsiblings(data);

    return ret;
}

int
cmd_list(const char *arg)
{
    struct lyd_node *ylib = NULL, *module, *submodule, *node;
    int has_modules = 0, flag;
    char **argv = NULL, *ptr;
    int c, argc, option_index;
    LYD_FORMAT outformat = LYD_UNKNOWN;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {NULL, 0, 0, 0}
    };

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        argv = realloc(argv, (argc+2)*sizeof *argv);
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hf:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_data_help();
            free(*argv);
            free(argv);
            return 0;
        case 'f':
            if (!strcmp(optarg, "xml")) {
                outformat = LYD_XML_FORMAT;
            } else if (!strcmp(optarg, "json")) {
                outformat = LYD_JSON;
            } else {
                fprintf(stderr, "Unknown output format \"%s\".\n", optarg);
                goto error;
            }
            break;
        case '?':
            /* getopt_long() prints message */
            goto error;
        }
    }
    if (optind != argc) {
        fprintf(stderr, "Unknown parameter \"%s\"\n", argv[optind]);
error:
        free(*argv);
        free(argv);
        return 1;
    }
    free(*argv);
    free(argv);

    ylib = ly_ctx_info(ctx);
    if (!ylib) {
        fprintf(stderr, "Getting context info (ietf-yang-library data) failed.\n");
        return 1;
    }

    if (outformat != LYD_UNKNOWN) {
        lyd_print_file(stdout, ylib, outformat, LYP_WITHSIBLINGS);
        lyd_free(ylib);
        return 0;
    }

    LY_TREE_FOR(ylib->child, node) {
        if (!strcmp(node->schema->name, "module-set-id")) {
            printf("List of the loaded models (mod-set-id %s):\n", ((struct lyd_node_leaf_list *)node)->value_str);
            break;
        }
    }
    assert(node);

    LY_TREE_FOR(ylib->child, module) {
        if (!strcmp(module->schema->name, "module")) {
            has_modules = 1;

            /* module print */
            LY_TREE_FOR(module->child, node) {
                if (!strcmp(node->schema->name, "name")) {
                    printf("\t%s", ((struct lyd_node_leaf_list *)node)->value_str);
                } else if (!strcmp(node->schema->name, "revision")) {
                    if (((struct lyd_node_leaf_list *)node)->value_str[0] != '\0') {
                        printf("@%s", ((struct lyd_node_leaf_list *)node)->value_str);
                    }
                }
            }

            /* submodules print */
            LY_TREE_FOR(module->child, submodule) {
                if (!strcmp(submodule->schema->name, "submodules")) {
                    printf(" (");
                    flag = 0;
                    LY_TREE_FOR(submodule->child, submodule) {
                        if (!strcmp(submodule->schema->name, "submodule")) {
                            LY_TREE_FOR(submodule->child, node) {
                                if (!strcmp(node->schema->name, "name")) {
                                    printf("%s%s", flag ? "," : "", ((struct lyd_node_leaf_list *)node)->value_str);
                                } else if (!strcmp(node->schema->name, "revision")) {
                                    if (((struct lyd_node_leaf_list *)node)->value_str[0] != '\0') {
                                        printf("@%s", ((struct lyd_node_leaf_list *)node)->value_str);
                                    }
                                }
                            }
                            flag++;
                        }
                    }
                    printf(")");
                    break;
                }
            }
            printf("\n");
        }
    }

    if (!has_modules) {
        printf("\t(none)\n");
    }

    lyd_free(ylib);
    return 0;
}

int
cmd_feature(const char *arg)
{
    int c, i, argc, option_index, ret = 1, task = 0;
    unsigned int max_len;
    char **argv = NULL, *ptr, *model_name, *revision, *feat_names = NULL;
    const char **names;
    uint8_t *states;
    const struct lys_module *model, *parent_model;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"enable", required_argument, 0, 'e'},
        {"disable", required_argument, 0, 'd'},
        {NULL, 0, 0, 0}
    };

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        argv = realloc(argv, (argc+2)*sizeof *argv);
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "he:d:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_feature_help();
            ret = 0;
            goto cleanup;
        case 'e':
            if (task) {
                fprintf(stderr, "Only one of enable or disable can be specified.\n");
                goto cleanup;
            }
            task = 1;
            feat_names = optarg;
            break;
        case 'd':
            if (task) {
                fprintf(stderr, "Only one of enable, or disable can be specified.\n");
                goto cleanup;
            }
            task = 2;
            feat_names = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* model name */
    if (optind == argc) {
        fprintf(stderr, "Missing the model name.\n");
        goto cleanup;
    }

    revision = NULL;
    model_name = argv[optind];
    if (strchr(model_name, '@')) {
        revision = strchr(model_name, '@');
        revision[0] = '\0';
        ++revision;
    }

    model = ly_ctx_get_module(ctx, model_name, revision);
    if (model == NULL) {
        names = ly_ctx_get_module_names(ctx);
        for (i = 0; names[i]; i++) {
            if (!model) {
                parent_model = ly_ctx_get_module(ctx, names[i], NULL);
                model = (struct lys_module *)ly_ctx_get_submodule(parent_model, model_name, revision);
            }
        }
        free(names);
    }
    if (model == NULL) {
        if (revision) {
            fprintf(stderr, "No model \"%s\" in revision %s found.\n", model_name, revision);
        } else {
            fprintf(stderr, "No model \"%s\" found.\n", model_name);
        }
        goto cleanup;
    }

    if (!task) {
        printf("%s features:\n", model->name);

        names = lys_features_list(model, &states);

        /* get the max len */
        max_len = 0;
        for (i = 0; names[i]; ++i) {
            if (strlen(names[i]) > max_len) {
                max_len = strlen(names[i]);
            }
        }
        for (i = 0; names[i]; ++i) {
            printf("\t%-*s (%s)\n", max_len, names[i], states[i] ? "on" : "off");
        }
        free(names);
        free(states);
        if (!i) {
            printf("\t(none)\n");
        }
    } else {
        feat_names = strtok(feat_names, ",");
        while (feat_names) {
            if (((task == 1) && lys_features_enable(model, feat_names))
                    || ((task == 2) && lys_features_disable(model, feat_names))) {
                fprintf(stderr, "Feature \"%s\" not found.\n", feat_names);
                ret = 1;
            }
            feat_names = strtok(NULL, ",");
        }
    }

cleanup:
    free(*argv);
    free(argv);

    return ret;
}

int
cmd_searchpath(const char *arg)
{
    const char *path;
    struct stat st;

    if (strchr(arg, ' ') == NULL) {
        fprintf(stderr, "Missing the search path.\n");
        return 1;
    }
    path = strchr(arg, ' ')+1;

    if (!strcmp(path, "-h") || !strcmp(path, "--help")) {
        cmd_searchpath_help();
        return 0;
    }

    if (stat(path, &st) == -1) {
        fprintf(stderr, "Failed to stat the search path (%s).\n", strerror(errno));
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "\"%s\" is not a directory.\n", path);
        return 1;
    }

    ly_ctx_set_searchdir(ctx, path);

    return 0;
}

int
cmd_clear(const char *UNUSED(arg))
{
    ly_ctx_destroy(ctx, NULL);
    ctx = ly_ctx_new(NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return 1;
    }
    return 0;
}

int
cmd_verb(const char *arg)
{
    const char *verb;
    if (strlen(arg) < 5) {
        cmd_verb_help();
        return 1;
    }

    verb = arg + 5;
    if (!strcmp(verb, "error") || !strcmp(verb, "0")) {
        ly_verb(0);
    } else if (!strcmp(verb, "warning") || !strcmp(verb, "1")) {
        ly_verb(1);
    } else if (!strcmp(verb, "verbose")  || !strcmp(verb, "2")) {
        ly_verb(2);
    } else if (!strcmp(verb, "debug")  || !strcmp(verb, "3")) {
        ly_verb(3);
    } else {
        fprintf(stderr, "Unknown verbosity \"%s\"\n", verb);
        return 1;
    }

    return 0;
}

int
cmd_quit(const char *UNUSED(arg))
{
    done = 1;
    return 0;
}

int
cmd_help(const char *arg)
{
    int i;
    char *args = strdup(arg);
    char *cmd = NULL;

    strtok(args, " ");
    if ((cmd = strtok(NULL, " ")) == NULL) {

generic_help:
        fprintf(stdout, "Available commands:\n");

        for (i = 0; commands[i].name; i++) {
            if (commands[i].helpstring != NULL) {
                fprintf(stdout, "  %-15s %s\n", commands[i].name, commands[i].helpstring);
            }
        }
    } else {
        /* print specific help for the selected command */

        /* get the command of the specified name */
        for (i = 0; commands[i].name; i++) {
            if (strcmp(cmd, commands[i].name) == 0) {
                break;
            }
        }

        /* execute the command's help if any valid command specified */
        if (commands[i].name) {
            if (commands[i].help_func != NULL) {
                commands[i].help_func();
            } else {
                printf("%s\n", commands[i].helpstring);
            }
        } else {
            /* if unknown command specified, print the list of commands */
            printf("Unknown command \'%s\'\n", cmd);
            goto generic_help;
        }
    }

    free(args);
    return 0;
}

COMMAND commands[] = {
        {"help", cmd_help, NULL, "Display commands description"},
        {"add", cmd_add, cmd_add_help, "Add a new model"},
        {"print", cmd_print, cmd_print_help, "Print model"},
        {"data", cmd_data, cmd_data_help, "Load, validate and optionally print instance data"},
        {"xpath", cmd_xpath, cmd_xpath_help, "Get data nodes satisfying an XPath expression"},
        {"list", cmd_list, cmd_list_help, "List all the loaded models"},
        {"feature", cmd_feature, cmd_feature_help, "Print/enable/disable all/specific features of models"},
        {"searchpath", cmd_searchpath, cmd_searchpath_help, "Set the search path for models"},
        {"clear", cmd_clear, NULL, "Clear the context - remove all the loaded models"},
        {"verb", cmd_verb, cmd_verb_help, "Change verbosity"},
        {"quit", cmd_quit, NULL, "Quit the program"},
        /* synonyms for previous commands */
        {"?", cmd_help, NULL, "Display commands description"},
        {"exit", cmd_quit, NULL, "Quit the program"},
        {NULL, NULL, NULL, NULL}
};
