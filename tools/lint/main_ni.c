/**
 * @file main_ni.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool - noninteractive code
 *
 * Copyright (c) 2015-2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"
#include "../../src/libyang.h"

volatile int verbose = 0;

void
help(int shortout)
{
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "    yanglint [options] [-f { yang | yin | tree }] <file>...\n");
    fprintf(stdout, "        Validates the YANG module in <file>, and all its dependencies.\n\n");
    fprintf(stdout, "    yanglint [options] [-f { xml | json }] <schema>... <file>...\n");
    fprintf(stdout, "        Validates the YANG modeled data in <file> according to the <schema>.\n\n");
    fprintf(stdout, "    yanglint\n");
    fprintf(stdout, "        Starts interactive mode with more features.\n\n");

    if (shortout) {
        return;
    }
    fprintf(stdout, "Options:\n"
        "  -h, --help            Show this help message and exit.\n"
        "  -v, --version         Show version number and exit.\n"
        "  -V, --verbose         Show verbose messages, can be used multiple times to\n"
        "                        increase verbosity.\n"
        "  -p PATH, --path=PATH  Separated search path for yin and yang modules.\n\n"
        "  -f FORMAT, --format=FORMAT\n"
        "                        Convert to FORMAT. Supported formats: \n"
        "                        tree, yin, yang for schemas,\n"
        "                        xml, json for data.\n\n"
        "  -o OUTFILE, --output=OUTFILE\n"
        "                        Write the output to OUTFILE instead of stdout.\n\n"
        "  -F FEATURES, --features=FEATURES\n"
        "                        Features to support, default all.\n"
        "                        <modname>:[<feature>,]*\n\n"
        "  -d MODE, --default=MODE\n"
        "                        Print data with default values, according to the MODE\n"
        "                        (to print attributes, ietf-netconf-with-defaults model\n"
        "                        must be loaded):\n"
        "        all             - Add missing default nodes.\n"
        "        all-tagged      - Add missing default nodes and mark all the default\n"
        "                          nodes with the attribute.\n"
        "        trim            - Remove all nodes with a default value.\n"
        "        implicit-tagged - Add missing nodes and mark them with the attribute.\n\n"
        "  -t TYPE, --type=TYPE\n"
        "                        Specify data tree type in the input data file:\n"
        "        auto            - Resolve data type (one of the following) automatically\n"
        "                          (as pyang does) - applicable only on XML input data.\n"
        "        data            - Complete datastore with status data (default type).\n"
        "        config          - Configuration datastore (without status data).\n"
        "        get             - Result of the NETCONF <get> operation.\n"
        "        getconfig       - Result of the NETCONF <get-config> operation.\n"
        "        edit            - Content of the NETCONF <edit-config> operation.\n\n"
        "Tree output specific options:\n"
        "  --tree-help           Print help on tree symbols and exit.\n\n");
}

void
tree_help(void)
{
    fprintf(stdout, "Each node is printed as:\n\n");
    fprintf(stdout, "<status> <flags> <name> <opts> <type> <if-features>\n\n"
                    "  <status> is one of:\n"
                    "    + for current\n"
                    "    x for deprecated\n"
                    "    o for obsolete\n\n"
                    "  <flags> is one of:\n"
                    "    rw for configuration data\n"
                    "    ro for status data\n"
                    "    -x for RPCs\n"
                    "    -n for Notification\n\n"
                    "  <name> is the name of the node\n"
                    "    (<name>) means that the node is a choice node\n"
                    "    :(<name>) means that the node is a case node\n\n"
                    "    if the node is augmented into the tree from another module,\n"
                    "    it is printed with the module name as <module-name>:<name>.\n\n"
                    "  <opts> is one of:\n"
                    "    ? for an optional leaf or choice\n"
                    "    ! for a presence container\n"
                    "    * for a leaf-list or list\n"
                    "    [<keys>] for a list's keys\n\n"
                    "  <type> is the name of the type for leafs and leaf-lists\n"
                    "    If there is a default value defined, it is printed within\n"
                    "    angle brackets <default-value>.\n"
                    "    If the type is a leafref, the type is printed as -> TARGET`\n\n"
                    "  <if-features> is the list of features this node depends on,\n"
                    "    printed within curly brackets and a question mark {...}?\n\n");
}

void
version(void)
{
    fprintf(stdout, "yanglint %d.%d.%d\n", LY_VERSION_MAJOR, LY_VERSION_MINOR, LY_VERSION_MICRO);
}
void
libyang_verbclb(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    char *levstr;

    if (level <= verbose) {
        switch(level) {
        case LY_LLERR:
            levstr = "err :";
            break;
        case LY_LLWRN:
            levstr = "warn:";
            break;
        case LY_LLVRB:
            levstr = "verb:";
            break;
        default:
            levstr = "dbg :";
            break;
        }
        if (path) {
            fprintf(stderr, "%s %s (%s)\n", levstr, msg, path);
        } else {
            fprintf(stderr, "%s %s\n", levstr, msg);
        }
    }
}

int
main_ni(int argc, char* argv[])
{
    int ret = EXIT_FAILURE;
    int opt, opt_index = 0, i, featsize = 0;
    struct option options[] = {
        {"default",          required_argument, NULL, 'd'},
        {"format",           required_argument, NULL, 'f'},
        {"features",         required_argument, NULL, 'F'},
        {"help",             no_argument,       NULL, 'h'},
        {"tree-help",        no_argument,       NULL, 'H'},
        {"output",           required_argument, NULL, 'o'},
        {"path",             required_argument, NULL, 'p'},
        {"version",          no_argument,       NULL, 'v'},
        {"verbose",          no_argument,       NULL, 'V'},
        {"type",             required_argument, NULL, 't'},
        {NULL,               0,                 NULL, 0}
    };
    FILE *out = stdout;
    struct ly_ctx *ctx = NULL;
    const struct lys_module *mod;
    LYS_OUTFORMAT outformat_s = 0;
    LYS_INFORMAT informat_s;
    LYD_FORMAT informat_d, outformat_d = 0;
    char *searchpath = NULL;
    char **feat = NULL, *ptr, *featlist;
    struct stat st;
    uint32_t u;
    int options_dflt = 0, options_parser = 0;
    struct dataitem {
        const char *filename;
        LYD_FORMAT format;
        struct dataitem *next;
    } *data = NULL, *data_item;
    struct ly_set *mods = NULL;
    struct lyd_node *root = NULL, *node, *next, *subroot;
    struct lyxml_elem *xml = NULL;

    opterr = 0;
    while ((opt = getopt_long(argc, argv, "d:f:F:hHo:p:t:vV", options, &opt_index)) != -1) {
        switch (opt) {
        case 'd':
            if (!strcmp(optarg, "all")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_ALL;
            } else if (!strcmp(optarg, "all-tagged")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_ALL_TAG;
            } else if (!strcmp(optarg, "trim")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_TRIM;
            } else if (!strcmp(optarg, "implicit-tagged")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_IMPL_TAG;
            } else {
                fprintf(stderr, "yanglint error: unknown default mode %s\n", optarg);
                help(1);
                goto cleanup;
            }
            break;
        case 'f':
            if (!strcasecmp(optarg, "tree")) {
                outformat_s = LYS_OUT_TREE;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "yin")) {
                outformat_s = LYS_OUT_YIN;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "yang")) {
                outformat_s = LYS_OUT_YANG;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "xml")) {
                outformat_s = 0;
                outformat_d = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                outformat_s = 0;
                outformat_d = LYD_JSON;
            } else {
                fprintf(stderr, "yanglint error: unknown output format %s\n", optarg);
                help(1);
                goto cleanup;
            }
            break;
        case 'F':
            featsize++;
            if (!feat) {
                feat = malloc(sizeof *feat);
            } else {
                feat = realloc(feat, featsize * sizeof *feat);
            }
            feat[featsize - 1] = strdup(optarg);
            ptr = strchr(feat[featsize - 1], ':');
            if (!ptr) {
                fprintf(stderr, "Invalid format of the features specification (%s)", optarg);
                goto cleanup;
            }
            *ptr = '\0';

            break;
        case 'h':
            help(0);
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'H':
            tree_help();
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'o':
            if (out != stdout) {
                fclose(out);
            }
            out = fopen(optarg, "w");
            if (!out) {
                fprintf(stderr, "yanglint error: unable open output file %s (%s)\n", optarg, strerror(errno));
                goto cleanup;
            }
            break;
        case 'p':
            searchpath = optarg;
            if (stat(searchpath, &st) == -1) {
                fprintf(stderr, "Unable to use search path (%s) - %s.\n", searchpath, strerror(errno));
                goto cleanup;
            }
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "Provided search path is not a directory.\n");
                goto cleanup;
            }
            break;
        case 't':
            if (!strcmp(optarg, "auto")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_TYPEMASK;
            } else if (!strcmp(optarg, "config")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
            } else if (!strcmp(optarg, "get")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
            } else if (!strcmp(optarg, "getconfig")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
            } else if (!strcmp(optarg, "edit")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
            } else if (!strcmp(optarg, "data")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA;
            } else {
                fprintf(stderr, "yanglint error: unknown data tree type %s\n", optarg);
                help(1);
                goto cleanup;
            }
            break;
        case 'v':
            version();
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'V':
            verbose++;
            break;
        default:
            help(1);
            if (optopt) {
                fprintf(stderr, "yanglint error: invalid option: -%c\n", optopt);
            } else {
                fprintf(stderr, "yanglint error: invalid option: %s\n", argv[optind - 1]);
            }
            goto cleanup;
        }
    }

    /* check options compatibility */
    if (optind >= argc) {
        help(1);
        fprintf(stderr, "yanglint error: missing <file> to process\n");
        goto cleanup;;
    }
    if (outformat_s && outformat_s != LYS_OUT_TREE && (optind + 1) < argc) {
        /* we have multiple schemas to be printed as YIN or YANG */
        fprintf(stderr, "yanglint error: too many schemas to convert and store.\n");
        goto cleanup;
    }
    if (!outformat_d && options_dflt) {
        /* we have options for printing default nodes, but output is schema */
        fprintf(stderr, "yanglint warning: default mode is ignored when printing schema.\n");
    }
    if (!outformat_d && options_parser) {
        /* we have options for printing default nodes, but output is schema */
        fprintf(stderr, "yanglint warning: parser option is ignored when printing schema.\n");
    }

    /* set callback for printing libyang messages */
    ly_set_log_clb(libyang_verbclb, 1);

    /* create libyang context */
    ctx = ly_ctx_new(searchpath);
    if (!ctx) {
        goto cleanup;
    }

    /* derefered setting of verbosity in libyang after context initiation */
    ly_verb(verbose);

    mods = ly_set_new();

    /* divide input files */
    for (i = 0; i < argc - optind; i++) {
        /* get the file format */
        if ((ptr = strrchr(argv[optind + i], '.')) != NULL) {
            ++ptr;
            if (!strcmp(ptr, "yin")) {
                informat_s = LYS_IN_YIN;
                informat_d = 0;
            } else if (!strcmp(ptr, "yang")) {
                informat_s = LYS_IN_YANG;
                informat_d = 0;
            } else if (!strcmp(ptr, "xml")) {
                informat_s = 0;
                informat_d = LYD_XML;
            } else if (!strcmp(ptr, "json")) {
                informat_s = 0;
                informat_d = LYD_JSON;
            } else {
                fprintf(stderr, "yanglint error: input file in an unknown format \"%s\".\n", ptr);
                goto cleanup;
            }
        } else {
            fprintf(stderr, "yanglint error: input file \"%s\" without file extension - unknown format.\n",
                    argv[optind + i]);
            goto cleanup;
        }

        if (informat_s) {
            /* load/validate schema */
            if (verbose >= 2) {
                fprintf(stdout, "Validating %s schema file.\n", argv[optind + i]);
            }
            mod = lys_parse_path(ctx, argv[optind + i], informat_s);
            if (!mod) {
                goto cleanup;
            }
            ly_set_add(mods, (void *)mod, 0);
        } else {
            if ((options_parser & LYD_OPT_TYPEMASK) == LYD_OPT_TYPEMASK && informat_d != LYD_XML) {
                /* data file content autodetection is possible only for XML input */
                fprintf(stderr, "yanglint error: data type autodetection is applicable only to XML files.\n");
                goto cleanup;
            }

            /* remember data filename and its format */
            if (!data) {
                data = data_item = malloc(sizeof *data);
            } else {
                for (data_item = data; data_item->next; data_item = data_item->next);
                data_item->next = malloc(sizeof *data_item);
                data_item = data_item->next;
            }
            data_item->filename = argv[optind + i];
            data_item->format = informat_d;
            data_item->next = NULL;
        }
    }

    if (outformat_d && !data) {
        fprintf(stderr, "yanglint error: no input data file for the specified data output format.\n");
        goto cleanup;
    }

    /* enable specified features, if not specified, all the module's features are enabled */
    u = 4; /* skip internal libyang modules */
    while ((mod = ly_ctx_get_module_iter(ctx, &u))) {
        for (i = 0; i < featsize; i++) {
            if (!strcmp(feat[i], mod->name)) {
                /* parse features spec */
                featlist = strdup(feat[i] + strlen(feat[i]) + 1);
                ptr = NULL;
                while((ptr = strtok(ptr ? NULL : featlist, ","))) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Enabling feature %s in module %s.\n", ptr, mod->name);
                    }
                    if (lys_features_enable(mod, ptr)) {
                        fprintf(stderr, "Feature %s not defined in module %s.\n", ptr, mod->name);
                    }
                }
                free(featlist);
                break;
            }
        }
        if (i == featsize) {
            if (verbose >= 2) {
                fprintf(stdout, "Enabling all features in module %s.\n", mod->name);
            }
            lys_features_enable(mod, "*");
        }
    }

    /* convert (print) to FORMAT */
    if (outformat_s) {
        for (u = 0; u < mods->number; u++) {
            lys_print_file(out, (struct lys_module *)mods->set.g[u], outformat_s, NULL);
        }
    } else {
        ly_errno = 0;
        if (!options_parser) {
            /* LYD_OPT_DATA - status data fro ietf-yang-library are needed */
            root = ly_ctx_info(ctx);
        }
        for (data_item = data; data_item; data_item = data_item->next) {
            /* parse data file - via LYD_OPT_TRUSTED postpone validation when all data are loaded and merged */
            if ((options_parser & LYD_OPT_TYPEMASK) == LYD_OPT_TYPEMASK) {
                /* automatically detect data type from the data top level */
                xml = lyxml_parse_path(ctx, data_item->filename, 0);
                if (!xml) {
                    fprintf(stderr, "yanglint error: parsing XML data for data type autodetection failed.\n");
                    goto cleanup;
                }

                /* NOTE: namespace is ignored to simplify usage of this feature */
                if (!strcmp(xml->name, "data")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as complete datastore.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK);
                } else if (!strcmp(xml->name, "config")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as config data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
                } else if (!strcmp(xml->name, "get-reply")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <get> reply data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
                } else if (!strcmp(xml->name, "get-config-reply")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <get-config> reply data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
                } else if (!strcmp(xml->name, "edit-config")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <edit-config> data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
                } else {
                    fprintf(stderr, "yanglint error: invalid top-level element \"%s\" for data type autodetection.\n",
                            xml->name);
                    lyxml_free(ctx, xml);
                    goto cleanup;
                }

                node = lyd_parse_xml(ctx, &xml->child, LYD_OPT_TRUSTED | options_parser);
            } else {
                node = lyd_parse_path(ctx, data_item->filename, data_item->format, LYD_OPT_TRUSTED | options_parser);
            }
            if (ly_errno) {
                goto cleanup;
            }
            if (!root) {
                root = node;
            } else if (node) {
                /* merge results */
                if (lyd_merge(root, node, LYD_OPT_DESTRUCT | LYD_OPT_EXPLICIT)) {
                    fprintf(stderr, "yanglint error: merging multiple data trees failed.\n");
                    goto cleanup;
                }
            }
        }
        /* validate the data */
        if (outformat_d || root) {
            /* do not trust the input, invalidate all the data first */
            LY_TREE_FOR(root, subroot) {
                LY_TREE_DFS_BEGIN(subroot, next, node) {
                    node->validity = LYD_VAL_NOT;
                    LY_TREE_DFS_END(subroot, next, node)
                }
            }
            if (lyd_validate(&root, options_parser, NULL)) {
                goto cleanup;
            }
        }

        /* print only if data output format specified */
        if (outformat_d && root) {
            lyd_print_file(out, root, outformat_d, LYP_WITHSIBLINGS | LYP_FORMAT | options_dflt);
        }
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (out && out != stdout) {
        fclose(out);
    }
    ly_set_free(mods);
    for (i = 0; i < featsize; i++) {
        free(feat[i]);
    }
    free(feat);
    for (; data; data = data_item) {
        data_item = data->next;
        free(data);
    }
    lyxml_free(ctx, xml);
    lyd_free_withsiblings(root);
    ly_ctx_destroy(ctx, NULL);

    return ret;
}
