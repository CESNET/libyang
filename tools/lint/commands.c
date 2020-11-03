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

#define _GNU_SOURCE

#include "commands.h"

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "compat.h"
#include "libyang.h"

COMMAND commands[];
extern int done;
extern struct ly_ctx *ctx;

void
cmd_add_help(void)
{
    printf("add [-i] <path-to-model> [<paths-to-other-models> ...]\n");
    printf("\t-i         - make all the imported modules implemented\n");
}

void
cmd_load_help(void)
{
    printf("load [-i] <model-name> [<other-model-names> ...]\n");
    printf("\t-i         - make all the imported modules implemented\n");
}

void
cmd_clear_help(void)
{
    printf("clear [<yang-library> | -e]\n");
    printf("\t Replace the current context with an empty one, searchpaths are not kept.\n");
    printf("\t If <yang-library> path specified, load the modules according to the yang library data.\n");
    printf("\t Option '-e' causes ietf-yang-library will not be loaded.\n");
}

void
cmd_print_help(void)
{
    printf("print [-f (yang | yin | tree [<tree-options>] | info [-P <info-path>] [-(-s)ingle-node])] [-o <output-file>]"
           " <model-name>[@<revision>]\n");
    printf("\n");
    printf("\ttree-options:\t--tree-print-groupings\t(print top-level groupings in a separate section)\n");
    printf("\t             \t--tree-print-uses\t(print uses nodes instead the resolved grouping nodes)\n");
    printf("\t             \t--tree-no-leafref-target\t(do not print the target nodes of leafrefs)\n");
    printf("\t             \t--tree-path <data-path>\t(print only the specified subtree)\n");
    printf("\t             \t--tree-line-length <line-length>\t(wrap lines if longer than line-length,\n");
    printf("\t             \t\tnot a strict limit, longer lines can often appear)\n");
    printf("\n");
    printf("\tinfo-path:\t<data-path> | identity/<identity-name> | feature/<feature-name>\n");
    printf("\n");
    printf("\tschema-path:\t( /<module-name>:<node-identifier> )+\n");
}

void
cmd_data_help(void)
{
    printf("data [-(-s)trict] [-t TYPE] [-d DEFAULTS] [-o <output-file>] [-f (xml | json | lyb)] [-r <running-file-name>]\n");
    printf("     <data-file-name> [<RPC/action-data-file-name> | <yang-data name>]\n\n");
    printf("Accepted TYPEs:\n");
    printf("\tauto       - resolve data type (one of the following) automatically (as pyang does),\n");
    printf("\t             this option is applicable only in case of XML input data.\n");
    printf("\tdata       - LYD_OPT_DATA (default value) - complete datastore including status data.\n");
    printf("\tconfig     - LYD_OPT_CONFIG - complete configuration datastore.\n");
    printf("\tget        - LYD_OPT_GET - <get> operation result.\n");
    printf("\tgetconfig  - LYD_OPT_GETCONFIG - <get-config> operation result.\n");
    printf("\tedit       - LYD_OPT_EDIT - <edit-config>'s data (content of its <config> element).\n");
    printf("\trpc        - LYD_OPT_RPC - NETCONF RPC message.\n");
    printf("\trpcreply   - LYD_OPT_RPCREPLY (last parameter mandatory in this case)\n");
    printf("\tnotif      - LYD_OPT_NOTIF - NETCONF Notification message.\n");
    printf("\tyangdata   - LYD_OPT_DATA_TEMPLATE - yang-data extension (last parameter mandatory in this case)\n\n");
    printf("Accepted DEFAULTS:\n");
    printf("\tall        - add missing default nodes\n");
    printf("\tall-tagged - add missing default nodes and mark all the default nodes with the attribute.\n");
    printf("\ttrim       - remove all nodes with a default value\n");
    printf("\timplicit-tagged    - add missing nodes and mark them with the attribute\n\n");
    printf("Option -r:\n");
    printf("\tOptional parameter for 'rpc', 'rpcreply' and 'notif' TYPEs, the file contains running\n");
    printf("\tconfiguration datastore data referenced from the RPC/Notification. Note that the file is\n");
    printf("\tvalidated as 'data' TYPE. Special value '!' can be used as argument to ignore the\n");
    printf("\texternal references.\n\n");
    printf("\tIf an XPath expression (when/must) needs access to configuration data, you can provide\n");
    printf("\tthem in a file, which will be parsed as 'data' TYPE.\n\n");
}

void
cmd_xpath_help(void)
{
    printf("xpath [-t TYPE] [-x <additional-tree-file-name>] -e <XPath-expression>\n"
           "      <XML-data-file-name> [<JSON-rpc/action-schema-nodeid>]\n");
    printf("Accepted TYPEs:\n");
    printf("\tauto       - resolve data type (one of the following) automatically (as pyang does),\n");
    printf("\t             this option is applicable only in case of XML input data.\n");
    printf("\tconfig     - LYD_OPT_CONFIG\n");
    printf("\tget        - LYD_OPT_GET\n");
    printf("\tgetconfig  - LYD_OPT_GETCONFIG\n");
    printf("\tedit       - LYD_OPT_EDIT\n");
    printf("\trpc        - LYD_OPT_RPC\n");
    printf("\trpcreply   - LYD_OPT_RPCREPLY (last parameter mandatory in this case)\n");
    printf("\tnotif      - LYD_OPT_NOTIF\n\n");
    printf("Option -x:\n");
    printf("\tIf RPC/action/notification/RPC reply (for TYPEs 'rpc', 'rpcreply', and 'notif') includes\n");
    printf("\tan XPath expression (when/must) that needs access to the configuration data, you can provide\n");
    printf("\tthem in a file, which will be parsed as 'config'.\n");
}

void
cmd_list_help(void)
{
    printf("list [-f (xml | json)]\n\n");
    printf("\tBasic list output (no -f): i - imported module, I - implemented module\n");
}

void
cmd_feature_help(void)
{
    printf("feature [ -(-e)nable | -(-d)isable (* | <feature-name>[,<feature-name> ...]) ] <model-name>[@<revision>]\n");
}

void
cmd_searchpath_help(void)
{
    printf("searchpath [<model-dir-path> | --clear]\n\n");
    printf("\tThey are used to search for imports and includes of a model.\n");
    printf("\tThe \"load\" command uses these directories to find models directly.\n");
}

void
cmd_verb_help(void)
{
    printf("verb (error/0 | warning/1 | verbose/2 | debug/3)\n");
}

#ifndef NDEBUG

void
cmd_debug_help(void)
{
    printf("debug (dict | yang | yin | xpath | diff)+\n");
}

#endif

LYS_INFORMAT
get_schema_format(const char *path)
{
    char *ptr;

    if ((ptr = strrchr(path, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "yang")) {
            return LYS_IN_YANG;
        } else if (!strcmp(ptr, "yin")) {
             return LYS_IN_YIN;
        } else {
            fprintf(stderr, "Input file in an unknown format \"%s\".\n", ptr);
            return LYS_IN_UNKNOWN;
        }
    } else {
        fprintf(stdout, "Input file \"%s\" without extension - unknown format.\n", path);
        return LYS_IN_UNKNOWN;
    }
}

int
cmd_add(const char *arg)
{
    int path_len, ret = 1;
    char *path, *dir, *s, *arg_ptr;
    const struct lys_module *model;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;

    if (strlen(arg) < 5) {
        cmd_add_help();
        return 1;
    }

    arg_ptr = strdup(arg + 3 /* ignore "add" */);

    for (s = strstr(arg_ptr, "-i"); s ; s = strstr(s + 2, "-i")) {
        if (s[2] == '\0' || s[2] == ' ') {
            ly_ctx_set_options(ctx, LY_CTX_ALL_IMPLEMENTED);
            s[0] = s[1] = ' ';
        }
    }
    s = arg_ptr;

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
        int unset_path = 1;
        format = get_schema_format(path);
        if (format == LYS_IN_UNKNOWN) {
            free(path);
            goto cleanup;
        }

        /* add temporarily also the path of the module itself */
        dir = strdup(path);
        if (ly_ctx_set_searchdir(ctx, dirname(dir)) == LY_EEXIST) {
            unset_path = 0;
        }
        /* parse the file */
        lys_parse_path(ctx, path, format, &model);
        ly_ctx_unset_searchdir_last(ctx, unset_path);
        free(path);
        free(dir);

        if (!model) {
            /* libyang printed the error messages */
            goto cleanup;
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
    if (format == LYS_IN_UNKNOWN) {
        /* no schema on input */
        cmd_add_help();
        goto cleanup;
    }
    ret = 0;

cleanup:
    free(s);
    ly_ctx_unset_options(ctx, LY_CTX_ALL_IMPLEMENTED);

    return ret;
}

int
cmd_load(const char *arg)
{
    int name_len, ret = 1;
    char *name, *s, *arg_ptr;
    const struct lys_module *model;

    if (strlen(arg) < 6) {
        cmd_load_help();
        return 1;
    }

    arg_ptr = strdup(arg + 4 /* ignore "load" */);

    for (s = strstr(arg_ptr, "-i"); s ; s = strstr(s + 2, "-i")) {
        if (s[2] == '\0' || s[2] == ' ') {
            ly_ctx_set_options(ctx, LY_CTX_ALL_IMPLEMENTED);
            s[0] = s[1] = ' ';
        }
    }
    s = arg_ptr;

    while (arg_ptr[0] == ' ') {
        ++arg_ptr;
    }
    if (strchr(arg_ptr, ' ')) {
        name_len = strchr(arg_ptr, ' ') - arg_ptr;
    } else {
        name_len = strlen(arg_ptr);
    }
    name = strndup(arg_ptr, name_len);

    while (name) {
        model = ly_ctx_load_module(ctx, name, NULL, NULL);
        free(name);
        if (!model) {
            /* libyang printed the error messages */
            goto cleanup;
        }

        /* next model */
        arg_ptr += name_len;
        while (arg_ptr[0] == ' ') {
            ++arg_ptr;
        }
        if (strchr(arg_ptr, ' ')) {
            name_len = strchr(arg_ptr, ' ') - arg_ptr;
        } else {
            name_len = strlen(arg_ptr);
        }

        if (name_len) {
            name = strndup(arg_ptr, name_len);
        } else {
            name = NULL;
        }
    }
    ret = 0;

cleanup:
    free(s);
    ly_ctx_unset_options(ctx, LY_CTX_ALL_IMPLEMENTED);

    return ret;
}

int
cmd_print(const char *arg)
{
    int c, argc, option_index, ret = 1, tree_ll = 0, output_opts = 0;
    char **argv = NULL, *ptr, *model_name, *revision;
    const char *out_path = NULL, *target_path = NULL;
    const struct lys_module *module;
    LYS_OUTFORMAT format = LYS_OUT_TREE;
    struct ly_out *out = NULL;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
#if 0
        {"tree-print-groupings", no_argument, 0, 'g'},
        {"tree-print-uses", no_argument, 0, 'u'},
        {"tree-no-leafref-target", no_argument, 0, 'n'},
        {"tree-path", required_argument, 0, 'P'},
#endif
        {"info-path", required_argument, 0, 'P'},
        {"single-node", no_argument, 0, 's'},
#if 0
        {"tree-line-length", required_argument, 0, 'L'},
#endif
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc+2)*sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto cleanup;
        }
        argv = rlcd;
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "chf:go:guP:sL:", long_options, &option_index);
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
#if 0
            } else if (!strcmp(optarg, "tree")) {
                format = LYS_OUT_TREE;
            } else if (!strcmp(optarg, "tree-rfc")) {
                format = LYS_OUT_TREE;
                output_opts |= LYS_OUTOPT_TREE_RFC;
#endif
            } else if (!strcmp(optarg, "info")) {
                format = LYS_OUT_YANG_COMPILED;
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
#if 0
        case 'g':
            output_opts |= LYS_OUTOPT_TREE_GROUPING;
            break;
        case 'u':
            output_opts |= LYS_OUTOPT_TREE_USES;
            break;
        case 'n':
            output_opts |= LYS_OUTOPT_TREE_NO_LEAFREF;
            break;
#endif
        case 'P':
            target_path = optarg;
            break;
        case 's':
            output_opts |= LYS_PRINT_NO_SUBSTMT;
            break;
#if 0
        case 'L':
            tree_ll = atoi(optarg);
            break;
#endif
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* file name */
    if (optind == argc && !target_path) {
        fprintf(stderr, "Missing the module name.\n");
        goto cleanup;
    }

#if 0
    /* tree fromat with or without gropings */
    if ((output_opts || tree_ll) && format != LYS_OUT_TREE) {
        fprintf(stderr, "--tree options take effect only in case of the tree output format.\n");
    }
#endif

    if (!target_path) {
        /* module, revision */
        model_name = argv[optind];
        revision = NULL;
        if (strchr(model_name, '@')) {
            revision = strchr(model_name, '@');
            revision[0] = '\0';
            ++revision;
        }

        if (revision) {
            module = ly_ctx_get_module(ctx, model_name, revision);
        } else {
            module = ly_ctx_get_module_latest(ctx, model_name);
        }
#if 0
        if (!module) {
            /* not a module, try to find it as a submodule */
            module = (const struct lys_module *)ly_ctx_get_submodule(ctx, NULL, NULL, model_name, revision);
        }
#endif

        if (!module) {
            if (revision) {
                fprintf(stderr, "No (sub)module \"%s\" in revision %s found.\n", model_name, revision);
            } else {
                fprintf(stderr, "No (sub)module \"%s\" found.\n", model_name);
            }
            goto cleanup;
        }
    }

    if (out_path) {
        ret = ly_out_new_filepath(out_path, &out);
    } else {
        ret = ly_out_new_file(stdout, &out);
    }
    if (ret) {
        fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
        goto cleanup;
    }

    if (target_path) {
        const struct lysc_node *node = ly_ctx_get_node(ctx, NULL, target_path, 0);
        if (node) {
            ret = lys_print_node(out, node, format, tree_ll, output_opts);
        } else {
            fprintf(stderr, "The requested schema node \"%s\" does not exists.\n", target_path);
        }
    } else {
        ret = lys_print_module(out, module, format, tree_ll, output_opts);
    }

cleanup:
    free(*argv);
    free(argv);

    ly_out_free(out, NULL, out_path ? 1 : 0);

    return ret;
}

static int
parse_data(char *filepath, int *options, const struct lyd_node *tree, const char *rpc_act_file,
           struct lyd_node **result)
{
    struct lyd_node *data = NULL, *rpc_act = NULL;
    int opts = *options;
    struct ly_in *in;

    if (ly_in_new_filepath(filepath, 0, &in)) {
        fprintf(stderr, "Unable to open input YANG data file \"%s\".", filepath);
        return EXIT_FAILURE;
    }

#if 0
    if ((opts & LYD_OPT_TYPEMASK) == LYD_OPT_TYPEMASK) {
        /* automatically detect data type from the data top level */
        if (informat != LYD_XML) {
            fprintf(stderr, "Only XML data can be automatically explored.\n");
            return EXIT_FAILURE;
        }

        xml = lyxml_parse_path(ctx, filepath, 0);
        if (!xml) {
            fprintf(stderr, "Failed to parse XML data for automatic type detection.\n");
            return EXIT_FAILURE;
        }

        /* NOTE: namespace is ignored to simplify usage of this feature */

        if (!strcmp(xml->name, "data")) {
            fprintf(stdout, "Parsing %s as complete datastore.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_ADD_YANGLIB;
        } else if (!strcmp(xml->name, "config")) {
            fprintf(stdout, "Parsing %s as config data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
        } else if (!strcmp(xml->name, "get-reply")) {
            fprintf(stdout, "Parsing %s as <get> reply data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
        } else if (!strcmp(xml->name, "get-config-reply")) {
            fprintf(stdout, "Parsing %s as <get-config> reply data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
        } else if (!strcmp(xml->name, "edit-config")) {
            fprintf(stdout, "Parsing %s as <edit-config> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
        } else if (!strcmp(xml->name, "rpc")) {
            fprintf(stdout, "Parsing %s as <rpc> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
        } else if (!strcmp(xml->name, "rpc-reply")) {
            if (!rpc_act_file) {
                fprintf(stderr, "RPC/action reply data require additional argument (file with the RPC/action).\n");
                lyxml_free(ctx, xml);
                return EXIT_FAILURE;
            }
            fprintf(stdout, "Parsing %s as <rpc-reply> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
            rpc_act = lyd_parse_path(ctx, rpc_act_file, informat, LYD_OPT_RPC, val_tree);
            if (!rpc_act) {
                fprintf(stderr, "Failed to parse RPC/action.\n");
                lyxml_free(ctx, xml);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(xml->name, "notification")) {
            fprintf(stdout, "Parsing %s as <notification> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
        } else if (!strcmp(xml->name, "yang-data")) {
            fprintf(stdout, "Parsing %s as <yang-data> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_TEMPLATE;
            if (!rpc_act_file) {
                fprintf(stderr, "YANG-DATA require additional argument (name instance of yang-data extension).\n");
                lyxml_free(ctx, xml);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Invalid top-level element for automatic data type recognition.\n");
            lyxml_free(ctx, xml);
            return EXIT_FAILURE;
        }

        if (opts & LYD_OPT_RPCREPLY) {
            data = lyd_parse_xml(ctx, &xml->child, opts, rpc_act, val_tree);
        } else if (opts & (LYD_OPT_RPC | LYD_OPT_NOTIF)) {
            data = lyd_parse_xml(ctx, &xml->child, opts, val_tree);
        } else if (opts & LYD_OPT_DATA_TEMPLATE) {
            data = lyd_parse_xml(ctx, &xml->child, opts, rpc_act_file);
        } else {
            data = lyd_parse_xml(ctx, &xml->child, opts);
        }
        lyxml_free(ctx, xml);
    } else {
        if (opts & LYD_OPT_RPCREPLY) {
            if (!rpc_act_file) {
                fprintf(stderr, "RPC/action reply data require additional argument (file with the RPC/action).\n");
                return EXIT_FAILURE;
            }
            rpc_act = lyd_parse_path(ctx, rpc_act_file, informat, LYD_OPT_RPC, trees);
            if (!rpc_act) {
                fprintf(stderr, "Failed to parse RPC/action.\n");
                return EXIT_FAILURE;
            }
            if (trees) {
                const struct lyd_node **trees_new;
                unsigned int u;
                trees_new = lyd_trees_new(1, rpc_act);

                LY_ARRAY_FOR(trees, u) {
                    trees_new = lyd_trees_add(trees_new, trees[u]);
                }
                lyd_trees_free(trees, 0);
                trees = trees_new;
            } else {
                trees = lyd_trees_new(1, rpc_act);
            }
            data = lyd_parse_path(ctx, filepath, informat, opts, trees);
        } else if (opts & (LYD_OPT_RPC | LYD_OPT_NOTIF)) {
            data = lyd_parse_path(ctx, filepath, informat, opts, trees);
        } else if (opts & LYD_OPT_DATA_TEMPLATE) {
            if (!rpc_act_file) {
                fprintf(stderr, "YANG-DATA require additional argument (name instance of yang-data extension).\n");
                return EXIT_FAILURE;
            }
            data = lyd_parse_path(ctx, filepath, informat, opts, rpc_act_file);
        } else {
#endif

            lyd_parse_data(ctx, in, 0, opts, LYD_VALIDATE_PRESENT, &data);
#if 0
        }
    }
#endif
    ly_in_free(in, 0);

    lyd_free_all(rpc_act);

    if (ly_err_first(ctx)) {
        fprintf(stderr, "Failed to parse data.\n");
        lyd_free_all(data);
        return EXIT_FAILURE;
    }

    *result = data;
    *options = opts;
    return EXIT_SUCCESS;
}

int
cmd_data(const char *arg)
{
    int c, argc, option_index, ret = 1;
    int options = 0, printopt = 0;
    char **argv = NULL, *ptr;
    const char *out_path = NULL;
    struct lyd_node *data = NULL;
    struct lyd_node *tree = NULL;
    LYD_FORMAT outformat = 0;
    struct ly_out *out = NULL;
    static struct option long_options[] = {
        {"defaults", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"option", required_argument, 0, 't'},
        {"output", required_argument, 0, 'o'},
        {"running", required_argument, 0, 'r'},
        {"strict", no_argument, 0, 's'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc + 2) * sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto cleanup;
        }
        argv = rlcd;
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "d:hf:o:st:r:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'd':
            if (!strcmp(optarg, "all")) {
                printopt = (printopt & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL;
            } else if (!strcmp(optarg, "all-tagged")) {
                printopt = (printopt & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL_TAG;
            } else if (!strcmp(optarg, "trim")) {
                printopt = (printopt & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_TRIM;
            } else if (!strcmp(optarg, "implicit-tagged")) {
                printopt = (printopt & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_IMPL_TAG;
            }
            break;
        case 'h':
            cmd_data_help();
            ret = 0;
            goto cleanup;
        case 'f':
            if (!strcmp(optarg, "xml")) {
                outformat = LYD_XML;
            } else if (!strcmp(optarg, "json")) {
                outformat = LYD_JSON;
            } else if (!strcmp(optarg, "lyb")) {
                outformat = LYD_LYB;
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
#if 0
        case 'r':
            if (optarg[0] == '!') {
                /* ignore extenral dependencies to the running datastore */
                options |= LYD_OPT_NOEXTDEPS;
            } else {
                /* external file with the running datastore */
                val_tree = lyd_parse_path(ctx, optarg, LYD_XML, LYD_OPT_DATA_NO_YANGLIB, trees);
                if (!val_tree) {
                    fprintf(stderr, "Failed to parse the additional data tree for validation.\n");
                    goto cleanup;
                }
                if (!trees) {
                    trees = lyd_trees_new(1, val_tree);
                } else {
                    trees = lyd_trees_add(trees, val_tree);
                }
            }
            break;
#endif
        case 's':
            options |= LYD_PARSE_STRICT;
            break;
        case 't':
            if (!strcmp(optarg, "auto")) {
                /* no flags */
            } else if (!strcmp(optarg, "data")) {
                /* no flags */
            /*} else if (!strcmp(optarg, "config")) {
                options |= LYD_OPT_CONFIG;
            } else if (!strcmp(optarg, "get")) {
                options |= LYD_OPT_GET;
            } else if (!strcmp(optarg, "getconfig")) {
                options |= LYD_OPT_GETCONFIG;
            } else if (!strcmp(optarg, "edit")) {
                options |= LYD_OPT_EDIT;*/
            } else {
                fprintf(stderr, "Invalid parser option \"%s\".\n", optarg);
                cmd_data_help();
                goto cleanup;
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

    if (parse_data(argv[optind], &options, tree, argv[optind + 1], &data)) {
        goto cleanup;
    }

    if (out_path) {
        ret = ly_out_new_filepath(out_path, &out);
    } else {
        ret = ly_out_new_file(stdout, &out);
    }
    if (ret) {
        fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
        goto cleanup;
    }

    if (outformat) {
        ret = lyd_print_all(out, data, outformat, printopt);
        ret = ret < 0 ? ret * (-1) : 0;
    }

cleanup:
    free(*argv);
    free(argv);

    ly_out_free(out, NULL, out_path ? 1 : 0);

    lyd_free_all(data);

    return ret;
}
#if 0
int
cmd_xpath(const char *arg)
{
    int c, argc, option_index, ret = 1, long_str;
    char **argv = NULL, *ptr, *expr = NULL;
    unsigned int i, j;
    int options = 0;
    struct lyd_node *data = NULL, *node, *val_tree = NULL;
    struct lyd_node_leaf_list *key;
    struct ly_set *set;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"expr", required_argument, 0, 'e'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

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
            rlcd = realloc(argv, (argc + 2) * sizeof *argv);
            if (!rlcd) {
                fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                goto cleanup;
            }
            argv = rlcd;
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
        c = getopt_long(argc, argv, "he:t:x:", long_options, &option_index);
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
        case 't':
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
            } else if (!strcmp(optarg, "rpcreply")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
            } else if (!strcmp(optarg, "notif")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
            } else if (!strcmp(optarg, "yangdata")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_TEMPLATE;
            } else {
                fprintf(stderr, "Invalid parser option \"%s\".\n", optarg);
                cmd_data_help();
                goto cleanup;
            }
            break;
        case 'x':
            val_tree = lyd_parse_path(ctx, optarg, LYD_XML, LYD_OPT_CONFIG);
            if (!val_tree) {
                fprintf(stderr, "Failed to parse the additional data tree for validation.\n");
                goto cleanup;
            }
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

    if (parse_data(argv[optind], &options, val_tree, argv[optind + 1], &data)) {
        goto cleanup;
    }

    if (!(set = lyd_find_path(data, expr))) {
        goto cleanup;
    }

    /* print result */
    printf("Result:\n");
    if (!set->number) {
        printf("\tEmpty\n");
    } else {
        for (i = 0; i < set->number; ++i) {
            node = set->set.d[i];
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
            case LYS_ANYDATA:
                printf("\tAnydata ");
                break;
            default:
                printf("\tUnknown ");
                break;
            }
            printf("\"%s\"", node->schema->name);
            if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                printf(" (val: %s)", ((struct lyd_node_leaf_list *)node)->value_str);
            } else if (node->schema->nodetype == LYS_LIST) {
                key = (struct lyd_node_leaf_list *)node->child;
                printf(" (");
                for (j = 0; j < ((struct lys_node_list *)node->schema)->keys_size; ++j) {
                    if (j) {
                        printf(" ");
                    }
                    printf("\"%s\": %s", key->schema->name, key->value_str);
                    key = (struct lyd_node_leaf_list *)key->next;
                }
                printf(")");
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
#endif

int
print_list(FILE *out, struct ly_ctx *ctx, LYD_FORMAT outformat)
{
    struct lyd_node *ylib;
    uint32_t idx = 0, has_modules = 0;
    const struct lys_module *mod;

    if (outformat != LYD_UNKNOWN) {
        if (ly_ctx_get_yanglib_data(ctx, &ylib)) {
            fprintf(stderr, "Getting context info (ietf-yang-library data) failed.\n");
            return 1;
        }

        lyd_print_file(out, ylib, outformat, LYD_PRINT_WITHSIBLINGS);
        lyd_free_all(ylib);
        return 0;
    }

    /* iterate schemas in context and provide just the basic info */
    fprintf(out, "List of the loaded models:\n");
    while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
        has_modules++;

        /* conformance print */
        if (mod->implemented) {
            fprintf(out, "\tI");
        } else {
            fprintf(out, "\ti");
        }

        /* module print */
        fprintf(out, " %s", mod->name);
        if (mod->revision) {
            fprintf(out, "@%s", mod->revision);
        }

        /* submodules print */
        if (mod->parsed && mod->parsed->includes) {
            uint64_t u = 0;
            fprintf(out, " (");
            LY_ARRAY_FOR(mod->parsed->includes, u) {
                fprintf(out, "%s%s", !u ? "" : ",", mod->parsed->includes[u].name);
                if (mod->parsed->includes[u].rev[0]) {
                    fprintf(out, "@%s", mod->parsed->includes[u].rev);
                }
            }
            fprintf(out, ")");
        }

        /* finish the line */
        fprintf(out, "\n");
    }

    if (!has_modules) {
        fprintf(out, "\t(none)\n");
    }

    return 0;
}

int
cmd_list(const char *arg)
{
    char **argv = NULL, *ptr;
    int c, argc, option_index;
    LYD_FORMAT outformat = LYD_UNKNOWN;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc+2)*sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto error;
        }
        argv = rlcd;
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
                outformat = LYD_XML;
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

    return print_list(stdout, ctx, outformat);
}

int
cmd_feature(const char *arg)
{
    int c, argc, option_index, ret = 1, task = 0;
    char **argv = NULL, *ptr, *model_name, *revision, *feat_names = NULL;
    const struct lys_module *module;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"enable", required_argument, 0, 'e'},
        {"disable", required_argument, 0, 'd'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc + 2) * sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto cleanup;
        }
        argv = rlcd;
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

    /* module name */
    if (optind == argc) {
        fprintf(stderr, "Missing the module name.\n");
        goto cleanup;
    }

    revision = NULL;
    model_name = argv[optind];
    if (strchr(model_name, '@')) {
        revision = strchr(model_name, '@');
        revision[0] = '\0';
        ++revision;
    }

    if (!revision) {
        module = ly_ctx_get_module_implemented(ctx, model_name);
    } else {
        module = ly_ctx_get_module(ctx, model_name, revision);
        if (module && !module->implemented) {
            module = NULL;
        }
    }
#if 0
    if (!module) {
        /* not a module, try to find it as a submodule */
        module = (const struct lys_module *)ly_ctx_get_submodule(ctx, NULL, NULL, model_name, revision);
    }
#endif

    if (module == NULL) {
        if (revision) {
            fprintf(stderr, "No implemented (sub)module \"%s\" in revision %s found.\n", model_name, revision);
        } else {
            fprintf(stderr, "No implemented (sub)module \"%s\" found.\n", model_name);
        }
        goto cleanup;
    }

    if (!task) {
        size_t len, max_len = 0;
        uint32_t idx = 0;
        const struct lysp_feature *f = NULL;

        printf("%s features:\n", module->name);

        /* get the max len */
        while ((f = lysp_feature_next(f, module->parsed, &idx))) {
            len = strlen(f->name);
            if (len > max_len) {
                max_len = len;
            }
        }

        idx = 0;
        f = NULL;
        while ((f = lysp_feature_next(f, module->parsed, &idx))) {
            printf("\t%-*s (%s)\n", (int)max_len, f->name, (f->flags & LYS_FENABLED) ? "on" : "off");
        }
        if (!max_len) {
            printf("\t(none)\n");
        }
    } else {
        feat_names = strtok(feat_names, ",");
        while (feat_names) {
            /*if (((task == 1) && lys_feature_enable(module, feat_names))
                    || ((task == 2) && lys_feature_disable(module, feat_names))) {
                fprintf(stderr, "Feature \"%s\" not found.\n", feat_names);
                ret = 1;
            }*/
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
    const char * const *searchpaths;
    int index;
    struct stat st;

    for (path = strchr(arg, ' '); path && (path[0] == ' '); ++path);
    if (!path || (path[0] == '\0')) {
        searchpaths = ly_ctx_get_searchdirs(ctx);
        if (searchpaths) {
            for (index = 0; searchpaths[index]; index++) {
                fprintf(stdout, "%s\n", searchpaths[index]);
            }
        }
        return 0;
    }

    if ((!strncmp(path, "-h", 2) && (path[2] == '\0' || path[2] == ' ')) ||
        (!strncmp(path, "--help", 6) && (path[6] == '\0' || path[6] == ' '))) {
        cmd_searchpath_help();
        return 0;
    } else if (!strncmp(path, "--clear", 7) && (path[7] == '\0' || path[7] == ' ')) {
        ly_ctx_unset_searchdir(ctx, NULL);
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
cmd_clear(const char *arg)
{
    struct ly_ctx *ctx_new;
    int options = 0;
#if 0
    int i;
    char *ylpath;
    const char * const *searchpaths;
    LYD_FORMAT format;

    /* get optional yang library file name */
    for (i = 5; arg[i] && isspace(arg[i]); i++);
    if (arg[i]) {
        if (arg[i] == '-' && arg[i + 1] == 'e') {
            options = LY_CTX_NOYANGLIBRARY;
            goto create_empty;
        } else {
            ylpath = strdup(&arg[i]);
            format = detect_data_format(ylpath);
            if (format == LYD_UNKNOWN) {
                free(ylpath);
                fprintf(stderr, "Unable to resolve format of the yang library file, please add \".xml\" or \".json\" suffix.\n");
                goto create_empty;
            }
            searchpaths = ly_ctx_get_searchdirs(ctx);
            ctx_new = ly_ctx_new_ylpath(searchpaths ? searchpaths[0] : NULL, ylpath, format, 0);
            free(ylpath);
        }
    } else {
create_empty:
#else
    (void) arg; /* TODO yang-library support */
    {
#endif
        ly_ctx_new(NULL, options, &ctx_new);
    }

    if (!ctx_new) {
        fprintf(stderr, "Failed to create context.\n");
        return 1;
    }

    /* final switch */
    ly_ctx_destroy(ctx, NULL);
    ctx = ctx_new;

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
        ly_log_level(LY_LLERR);
#ifndef NDEBUG
        ly_log_dbg_groups(0);
#endif
    } else if (!strcmp(verb, "warning") || !strcmp(verb, "1")) {
        ly_log_level(LY_LLWRN);
#ifndef NDEBUG
        ly_log_dbg_groups(0);
#endif
    } else if (!strcmp(verb, "verbose")  || !strcmp(verb, "2")) {
        ly_log_level(LY_LLVRB);
#ifndef NDEBUG
        ly_log_dbg_groups(0);
#endif
    } else if (!strcmp(verb, "debug")  || !strcmp(verb, "3")) {
        ly_log_level(LY_LLDBG);
#ifndef NDEBUG
        ly_log_dbg_groups(LY_LDGDICT | LY_LDGYANG | LY_LDGYIN | LY_LDGXPATH | LY_LDGDIFF);
#endif
    } else {
        fprintf(stderr, "Unknown verbosity \"%s\"\n", verb);
        return 1;
    }

    return 0;
}

#ifndef NDEBUG

int
cmd_debug(const char *arg)
{
    const char *beg, *end;
    int grps = 0;
    if (strlen(arg) < 6) {
        cmd_debug_help();
        return 1;
    }

    end = arg + 6;
    while (end[0]) {
        for (beg = end; isspace(beg[0]); ++beg);
        if (!beg[0]) {
            break;
        }

        for (end = beg; (end[0] && !isspace(end[0])); ++end);

        if (!strncmp(beg, "dict", end - beg)) {
            grps |= LY_LDGDICT;
        } else if (!strncmp(beg, "yang", end - beg)) {
            grps |= LY_LDGYANG;
        } else if (!strncmp(beg, "yin", end - beg)) {
            grps |= LY_LDGYIN;
        } else if (!strncmp(beg, "xpath", end - beg)) {
            grps |= LY_LDGXPATH;
        } else if (!strncmp(beg, "diff", end - beg)) {
            grps |= LY_LDGDIFF;
        } else {
            fprintf(stderr, "Unknown debug group \"%.*s\"\n", (int)(end - beg), beg);
            return 1;
        }
    }
    ly_log_dbg_groups(grps);

    return 0;
}

#endif

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
        {"add", cmd_add, cmd_add_help, "Add a new model from a specific file"},
        {"load", cmd_load, cmd_load_help, "Load a new model from the searchdirs"},
        {"print", cmd_print, cmd_print_help, "Print a model"},
        {"data", cmd_data, cmd_data_help, "Load, validate and optionally print instance data"},
#if 0
        {"xpath", cmd_xpath, cmd_xpath_help, "Get data nodes satisfying an XPath expression"},
#endif
        {"list", cmd_list, cmd_list_help, "List all the loaded models"},
        {"feature", cmd_feature, cmd_feature_help, "Print/enable/disable all/specific features of models"},
        {"searchpath", cmd_searchpath, cmd_searchpath_help, "Print/set the search path(s) for models"},
        {"clear", cmd_clear, cmd_clear_help, "Clear the context - remove all the loaded models"},
        {"verb", cmd_verb, cmd_verb_help, "Change verbosity"},
#ifndef NDEBUG
        {"debug", cmd_debug, cmd_debug_help, "Display specific debug message groups"},
#endif
        {"quit", cmd_quit, NULL, "Quit the program"},
        /* synonyms for previous commands */
        {"?", cmd_help, NULL, "Display commands description"},
        {"exit", cmd_quit, NULL, "Quit the program"},
        {NULL, NULL, NULL, NULL}
};
