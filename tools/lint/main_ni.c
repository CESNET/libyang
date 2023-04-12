/**
 * @file main_ni.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool - non-interactive code
 *
 * Copyright (c) 2020 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#include "libyang.h"
#include "plugins_exts.h"

#include "common.h"
#include "out.h"
#include "tools/config.h"

/**
 * @brief Context structure to hold and pass variables in a structured form.
 */
struct context {
    /* libyang context for the run */
    const char *yang_lib_file;
    uint16_t ctx_options;
    struct ly_ctx *ctx;

    /* prepared output (--output option or stdout by default) */
    struct ly_out *out;

    char *searchpaths;

    /* options flags */
    uint8_t list;        /* -l option to print list of schemas */

    /* line length for 'tree' format */
    size_t line_length; /* --tree-line-length */

    /*
     * schema
     */
    /* set schema modules' features via --features option (struct schema_features *) */
    struct ly_set schema_features;

    /* set of loaded schema modules (struct lys_module *) */
    struct ly_set schema_modules;

    /* options to parse and print schema modules */
    uint32_t schema_parse_options;
    uint32_t schema_print_options;

    /* specification of printing schema node subtree, option --schema-node */
    const char *schema_node_path;
    const struct lysc_node *schema_node;
    const char *submodule;

    /* name of file containing explicit context passed to callback
     * for schema-mount extension.  This also causes a callback to
     * be registered.
     */
    char *schema_context_filename;

    /* value of --format in case of schema format */
    LYS_OUTFORMAT schema_out_format;
    ly_bool feature_param_format;

    /*
     * data
     */
    /* various options based on --type option */
    enum lyd_type data_type;
    uint32_t data_parse_options;
    uint32_t data_validate_options;
    uint32_t data_print_options;

    /* flag for --merge option */
    uint8_t data_merge;

    /* value of --format in case of data format */
    LYD_FORMAT data_out_format;

    /* input data files (struct cmdline_file *) */
    struct ly_set data_inputs;

    /* storage for --operational */
    struct cmdline_file data_operational;

    /* storage for --reply-rpc */
    struct cmdline_file reply_rpc;

    /* storage for --data-xpath */
    struct ly_set data_xpath;
};

static void
erase_context(struct context *c)
{
    /* data */
    ly_set_erase(&c->data_inputs, free_cmdline_file);
    ly_in_free(c->data_operational.in, 1);
    ly_set_erase(&c->data_xpath, NULL);

    /* schema */
    ly_set_erase(&c->schema_features, free_features);
    ly_set_erase(&c->schema_modules, NULL);

    /* context */
    free(c->searchpaths);
    c->searchpaths = NULL;

    ly_out_free(c->out, NULL,  0);
    ly_ctx_destroy(c->ctx);

    if (c->schema_context_filename) {
        free(c->schema_context_filename);
    }
}

static void
version(void)
{
    printf("yanglint %s\n", PROJECT_VERSION);
}

static void
help(int shortout)
{

    printf("Example usage:\n"
            "    yanglint [-f { yang | yin | info}] <schema>...\n"
            "        Validates the YANG module <schema>(s) and all its dependencies, optionally printing\n"
            "        them in the specified format.\n\n"
            "    yanglint [-f { xml | json }] <schema>... <file>...\n"
            "        Validates the YANG modeled data <file>(s) according to the <schema>(s) optionally\n"
            "        printing them in the specified format.\n\n"
            "    yanglint -t (nc-)rpc/notif [-O <operational-file>] <schema>... <file>\n"
            "        Validates the YANG/NETCONF RPC/notification <file> according to the <schema>(s) using\n"
            "        <operational-file> with possible references to the operational datastore data.\n\n"
            "    yanglint -t nc-reply -R <rpc-file> [-O <operational-file>] <schema>... <file>\n"
            "        Validates the NETCONF rpc-reply <file> of RPC <rpc-file> according to the <schema>(s)\n"
            "        using <operational-file> with possible references to the operational datastore data.\n\n"
            "    yanglint\n"
            "        Starts interactive mode with more features.\n\n");

    if (shortout) {
        return;
    }
    printf("Options:\n"
            "  -h, --help    Show this help message and exit.\n"
            "  -v, --version Show version number and exit.\n"
            "  -V, --verbose Increase libyang verbosity and show verbose messages. If specified\n"
            "                a second time, show even debug messages.\n"
            "  -Q, --quiet   Decrease libyang verbosity and hide warnings. If specified a second\n"
            "                time, hide errors so no libyang messages are printed.\n");

    printf("  -f FORMAT, --format=FORMAT\n"
            "                Convert input into FORMAT. Supported formats: \n"
            "                yang, yin, tree, info and feature-param for schemas,\n"
            "                xml, json, and lyb for data.\n\n");

    printf("  -p PATH, --path=PATH\n"
            "                Search path for schema (YANG/YIN) modules. The option can be\n"
            "                used multiple times. The current working directory and the\n"
            "                path of the module being added is used implicitly. Subdirectories\n"
            "                are also searched\n\n");

    printf("  -D, --disable-searchdir\n"
            "                Do not implicitly search in current working directory for\n"
            "                schema modules. If specified a second time, do not even\n"
            "                search in the module directory (all modules must be \n"
            "                explicitly specified).\n\n");

    printf("  -F FEATURES, --features=FEATURES\n"
            "                Specific module features to support in the form <module-name>:(<feature>,)*\n"
            "                Use <feature> '*' to enable all features of a module. This option can be\n"
            "                specified multiple times, to enable features in multiple modules. If this\n"
            "                option is not specified, all the features in all the implemented modules\n"
            "                are enabled.\n\n");

    printf("  -i, --make-implemented\n"
            "                Make the imported modules \"referenced\" from any loaded\n"
            "                module also implemented. If specified a second time, all the\n"
            "                modules are set implemented.\n\n");

    printf("  -P PATH, --schema-node=PATH\n"
            "                Print only the specified subtree of the schema.\n"
            "                The PATH is the XPath subset mentioned in documentation as\n"
            "                the Path format. The option can be combined with --single-node\n"
            "                option to print information only about the specified node.\n"
            "  -q, --single-node\n"
            "                Supplement to the --schema-node option to print information\n"
            "                only about a single node specified as PATH argument.\n\n");

    printf("  -s SUBMODULE, --submodule=SUBMODULE\n"
            "                Print the specific submodule instead of the main module.\n\n");

    printf("  -x FILE, --ext-data=FILE\n"
            "                File containing the specific data required by an extension. Required by\n"
            "                the schema-mount extension, for example, when the operational data are\n"
            "                expected in the file. File format is guessed.\n\n");

    printf("  -n, --not-strict\n"
            "                Do not require strict data parsing (silently skip unknown data),\n"
            "                has no effect for schemas.\n\n");

    printf("  -e, --present Validate only with the schema modules whose data actually\n"
            "                exist in the provided input data files. Takes effect only\n"
            "                with the 'data' or 'config' TYPEs. Used to avoid requiring\n"
            "                mandatory nodes from modules which data are not present in the\n"
            "                provided input data files.\n\n");

    printf("  -t TYPE, --type=TYPE\n"
            "                Specify data tree type in the input data file(s):\n"
            "        data          - Complete datastore with status data (default type).\n"
            "        config        - Configuration datastore (without status data).\n"
            "        get           - Data returned by the NETCONF <get> operation.\n"
            "        getconfig     - Data returned by the NETCONF <get-config> operation.\n"
            "        edit          - Config content of the NETCONF <edit-config> operation.\n"
            "        rpc           - Invocation of a YANG RPC/action, defined as input.\n"
            "        nc-rpc        - Similar to 'rpc' but expect and check also the NETCONF\n"
            "                        envelopes <rpc> or <action>.\n"
            "        reply         - Reply to a YANG RPC/action, defined as output. Note that\n"
            "                        the reply data are expected inside a container representing\n"
            "                        the original RPC/action invocation.\n"
            "        nc-reply      - Similar to 'reply' but expect and check also the NETCONF\n"
            "                        envelope <rpc-reply> with output data nodes as direct\n"
            "                        descendants. The original RPC/action invocation is expected\n"
            "                        in a separate parameter '-R' and is parsed as 'nc-rpc'.\n"
            "        notif         - Notification instance of a YANG notification.\n"
            "        nc-notif      - Similar to 'notif' but expect and check also the NETCONF\n"
            "                        envelope <notification> with element <eventTime> and its\n"
            "                        sibling as the actual notification.\n\n");

    printf("  -d MODE, --default=MODE\n"
            "                Print data with default values, according to the MODE\n"
            "                (to print attributes, ietf-netconf-with-defaults model\n"
            "                must be loaded):\n"
            "      all             - Add missing default nodes.\n"
            "      all-tagged      - Add missing default nodes and mark all the default\n"
            "                        nodes with the attribute.\n"
            "      trim            - Remove all nodes with a default value.\n"
            "      implicit-tagged - Add missing nodes and mark them with the attribute.\n\n");

    printf("  -E XPATH, --data-xpath=XPATH\n"
            "                Evaluate XPATH expression over the data and print the nodes satysfying\n"
            "                the expression. The output format is specific and the option cannot\n"
            "                be combined with the -f and -d options. Also all the data\n"
            "                inputs are merged into a single data tree where the expression\n"
            "                is evaluated, so the -m option is always set implicitly.\n\n");

    printf("  -l, --list    Print info about the loaded schemas.\n"
            "                (i - imported module, I - implemented module)\n"
            "                In case the '-f' option with data encoding is specified,\n"
            "                the list is printed as \"ietf-yang-library\" data.\n\n");

    printf("  -L LINE_LENGTH, --tree-line-length=LINE_LENGTH\n"
            "                The limit of the maximum line length on which the 'tree'\n"
            "                format will try to be printed.\n\n");

    printf("  -o OUTFILE, --output=OUTFILE\n"
            "                Write the output to OUTFILE instead of stdout.\n\n");

    printf("  -O FILE, --operational=FILE\n"
            "                Provide optional data to extend validation of the '(nc-)rpc',\n"
            "                '(nc-)reply' or '(nc-)notif' TYPEs. The FILE is supposed to contain\n"
            "                the operational datastore referenced from the operation.\n"
            "                In case of a nested operation, its parent existence is also\n"
            "                checked in these operational data.\n\n");

    printf("  -R FILE, --reply-rpc=FILE\n"
            "                Provide source RPC for parsing of the 'nc-reply' TYPE. The FILE\n"
            "                is supposed to contain the source 'nc-rpc' operation of the reply.\n\n");

    printf("  -m, --merge   Merge input data files into a single tree and validate at\n"
            "                once. The option has effect only for 'data' and 'config' TYPEs.\n\n");

    printf("  -y, --yang-library\n"
            "                Load and implement internal \"ietf-yang-library\" YANG module.\n"
            "                Note that this module includes definitions of mandatory state\n"
            "                data that can result in unexpected data validation errors.\n\n");

    printf("  -Y FILE, --yang-library-file=FILE\n"
            "                Parse FILE with \"ietf-yang-library\" data and use them to\n"
            "                create an exact YANG schema context. If specified, the '-F'\n"
            "                parameter (enabled features) is ignored.\n\n");

    printf("  -X, --extended-leafref\n"
            "                Allow usage of deref() XPath function within leafref\n\n");

#ifndef NDEBUG
    printf("  -G GROUPS, --debug=GROUPS\n"
            "                Enable printing of specific debugging message group\n"
            "                (nothing will be printed unless verbosity is set to debug):\n"
            "                <group>[,<group>]* (dict, xpath, dep-sets)\n\n");
#endif
}

static void
libyang_verbclb(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    char *levstr;

    switch (level) {
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
        fprintf(stderr, "libyang %s %s (%s)\n", levstr, msg, path);
    } else {
        fprintf(stderr, "libyang %s %s\n", levstr, msg);
    }
}

static struct schema_features *
get_features_not_applied(const struct ly_set *fset)
{
    for (uint32_t u = 0; u < fset->count; ++u) {
        struct schema_features *sf = fset->objs[u];

        if (!sf->applied) {
            return sf;
        }
    }

    return NULL;
}

static LY_ERR
ext_data_clb(const struct lysc_ext_instance *ext, void *user_data, void **ext_data, ly_bool *ext_data_free)
{
    struct ly_ctx *ctx;
    struct lyd_node *data = NULL;

    ctx = ext->module->ctx;
    if (user_data) {
        lyd_parse_data_path(ctx, user_data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &data);
    }

    *ext_data = data;
    *ext_data_free = 1;
    return LY_SUCCESS;
}

static LY_ERR
searchpath_strcat(char **searchpaths, const char *path)
{
    uint64_t len;
    char *new;

    if (!(*searchpaths)) {
        *searchpaths = strdup(path);
        return LY_SUCCESS;
    }

    len = strlen(*searchpaths) + strlen(path) + strlen(PATH_SEPARATOR);
    new = realloc(*searchpaths, sizeof(char) * len + 1);
    if (!new) {
        return LY_EMEM;
    }
    strcat(new, PATH_SEPARATOR);
    strcat(new, path);
    *searchpaths = new;

    return LY_SUCCESS;
}

static int
fill_context_inputs(int argc, char *argv[], struct context *c)
{
    struct ly_in *in = NULL;
    struct schema_features *sf;
    struct lys_module *mod;
    const char *all_features[] = {"*", NULL};
    char *dir = NULL, *module = NULL;

    /* Create libyang context. */
    if (c->yang_lib_file) {
        /* ignore features */
        ly_set_erase(&c->schema_features, free_features);

        if (ly_ctx_new_ylpath(c->searchpaths, c->yang_lib_file, LYD_UNKNOWN, c->ctx_options, &c->ctx)) {
            YLMSG_E("Unable to modify libyang context with yang-library data.\n");
            return -1;
        }
    } else {
        /* set imp feature flag if all should be enabled */
        c->ctx_options |= !c->schema_features.count ? LY_CTX_ENABLE_IMP_FEATURES : 0;

        if (ly_ctx_new(c->searchpaths, c->ctx_options, &c->ctx)) {
            YLMSG_E("Unable to create libyang context\n");
            return -1;
        }
    }

    /* set callback providing run-time extension instance data */
    if (c->schema_context_filename) {
        ly_ctx_set_ext_data_clb(c->ctx, ext_data_clb, c->schema_context_filename);
    }

    /* process the operational and/or reply RPC content if any */
    if (c->data_operational.path) {
        if (get_input(c->data_operational.path, NULL, &c->data_operational.format, &c->data_operational.in)) {
            return -1;
        }
    }
    if (c->reply_rpc.path) {
        if (get_input(c->reply_rpc.path, NULL, &c->reply_rpc.format, &c->reply_rpc.in)) {
            return -1;
        }
    }

    for (int i = 0; i < argc - optind; i++) {
        LYS_INFORMAT format_schema = LYS_IN_UNKNOWN;
        LYD_FORMAT format_data = LYD_UNKNOWN;

        if (get_input(argv[optind + i], &format_schema, &format_data, &in)) {
            goto error;
        }

        if (format_schema) {
            LY_ERR ret;
            uint8_t path_unset = 1; /* flag to unset the path from the searchpaths list (if not already present) */
            const char **features;

            /* parse the input */
            if (parse_schema_path(argv[optind + i], &dir, &module)) {
                goto error;
            }

            if (c->yang_lib_file) {
                /* just get the module, it should already be parsed */
                mod = ly_ctx_get_module_implemented(c->ctx, module);
                if (!mod) {
                    YLMSG_E("Schema module \"%s\" not implemented in yang-library data.\n", module);
                    goto error;
                }
            } else {
                /* add temporarily also the path of the module itself */
                if (ly_ctx_set_searchdir(c->ctx, dir) == LY_EEXIST) {
                    path_unset = 0;
                }

                /* get features list for this module */
                if (!c->schema_features.count) {
                    features = all_features;
                } else {
                    get_features(&c->schema_features, module, &features);
                }

                /* parse module */
                ret = lys_parse(c->ctx, in, format_schema, features, &mod);
                ly_ctx_unset_searchdir_last(c->ctx, path_unset);
                if (ret) {
                    YLMSG_E("Parsing schema module \"%s\" failed.\n", argv[optind + i]);
                    goto error;
                }
            }

            /* temporary cleanup */
            free(dir);
            dir = NULL;
            free(module);
            module = NULL;

            if (c->schema_out_format || c->feature_param_format) {
                /* module will be printed */
                if (ly_set_add(&c->schema_modules, (void *)mod, 1, NULL)) {
                    YLMSG_E("Storing parsed schema module (%s) for print failed.\n", argv[optind + i]);
                    goto error;
                }
            }
        } else if (format_data) {
            if (!fill_cmdline_file(&c->data_inputs, in, argv[optind + i], format_data)) {
                goto error;
            }
            in = NULL;
        }

        ly_in_free(in, 1);
        in = NULL;
    }

    /* check that all specified features were applied, apply now if possible */
    while ((sf = get_features_not_applied(&c->schema_features))) {
        /* try to find implemented or the latest revision of this module */
        mod = ly_ctx_get_module_implemented(c->ctx, sf->mod_name);
        if (!mod) {
            mod = ly_ctx_get_module_latest(c->ctx, sf->mod_name);
        }
        if (!mod) {
            YLMSG_E("Specified features not applied, module \"%s\" not loaded.\n", sf->mod_name);
            goto error;
        }

        /* we have the module, implement it if needed and enable the specific features */
        if (lys_set_implemented(mod, (const char **)sf->features)) {
            YLMSG_E("Implementing module \"%s\" failed.\n", mod->name);
            goto error;
        }
        sf->applied = 1;
    }

    return 0;

error:
    ly_in_free(in, 1);
    free(dir);
    free(module);
    return -1;
}

/**
 * @brief Process command line options and store the settings into the context.
 *
 * return -1 in case of error;
 * return 0 in case of success and ready to process
 * return 1 in case of success, but expect to exit.
 */
static int
fill_context(int argc, char *argv[], struct context *c)
{
    int ret;

    int opt, opt_index;
    struct option options[] = {
        {"help",              no_argument,       NULL, 'h'},
        {"version",           no_argument,       NULL, 'v'},
        {"verbose",           no_argument,       NULL, 'V'},
        {"quiet",             no_argument,       NULL, 'Q'},
        {"format",            required_argument, NULL, 'f'},
        {"path",              required_argument, NULL, 'p'},
        {"disable-searchdir", no_argument,       NULL, 'D'},
        {"features",          required_argument, NULL, 'F'},
        {"make-implemented",  no_argument,       NULL, 'i'},
        {"schema-node",       required_argument, NULL, 'P'},
        {"single-node",       no_argument,       NULL, 'q'},
        {"submodule",         required_argument, NULL, 's'},
        {"ext-data",          required_argument, NULL, 'x'},
        {"not-strict",        no_argument,       NULL, 'n'},
        {"present",           no_argument,       NULL, 'e'},
        {"type",              required_argument, NULL, 't'},
        {"default",           required_argument, NULL, 'd'},
        {"data-xpath",        required_argument, NULL, 'E'},
        {"list",              no_argument,       NULL, 'l'},
        {"tree-line-length",  required_argument, NULL, 'L'},
        {"output",            required_argument, NULL, 'o'},
        {"operational",       required_argument, NULL, 'O'},
        {"reply-rpc",         required_argument, NULL, 'R'},
        {"merge",             no_argument,       NULL, 'm'},
        {"yang-library",      no_argument,       NULL, 'y'},
        {"yang-library-file", required_argument, NULL, 'Y'},
        {"extended-leafref",  no_argument,       NULL, 'X'},
#ifndef NDEBUG
        {"debug",            required_argument, NULL, 'G'},
#endif
        {NULL,               0,                 NULL, 0}
    };
    uint8_t data_type_set = 0;

    c->ctx_options = YL_DEFAULT_CTX_OPTIONS;
    c->data_parse_options = YL_DEFAULT_DATA_PARSE_OPTIONS;
    c->data_validate_options = YL_DEFAULT_DATA_VALIDATE_OPTIONS;
    c->line_length = 0;

    opterr = 0;
#ifndef NDEBUG
    while ((opt = getopt_long(argc, argv, "hvVQf:p:DF:iP:qs:neE:t:d:lL:o:O:R:myY:Xx:G:", options, &opt_index)) != -1)
#else
    while ((opt = getopt_long(argc, argv, "hvVQf:p:DF:iP:qs:neE:t:d:lL:o:O:R:myY:Xx:", options, &opt_index)) != -1)
#endif
    {
        switch (opt) {
        case 'h': /* --help */
            help(0);
            return 1;

        case 'v': /* --version */
            version();
            return 1;

        case 'V': { /* --verbose */
            LY_LOG_LEVEL verbosity = ly_log_level(LY_LLERR);

            if (verbosity < LY_LLDBG) {
                ++verbosity;
            }
            ly_log_level(verbosity);
            break;
        } /* case 'V' */

        case 'Q': { /* --quiet */
            LY_LOG_LEVEL verbosity = ly_log_level(LY_LLERR);

            if (verbosity == LY_LLERR) {
                /* turn logging off */
                ly_log_options(LY_LOSTORE_LAST);
            } else if (verbosity > LY_LLERR) {
                --verbosity;
            }
            ly_log_level(verbosity);
            break;
        } /* case 'Q' */

        case 'f': /* --format */
            if (!strcasecmp(optarg, "yang")) {
                c->schema_out_format = LYS_OUT_YANG;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "yin")) {
                c->schema_out_format = LYS_OUT_YIN;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "info")) {
                c->schema_out_format = LYS_OUT_YANG_COMPILED;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "tree")) {
                c->schema_out_format = LYS_OUT_TREE;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "xml")) {
                c->schema_out_format = 0;
                c->data_out_format = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                c->schema_out_format = 0;
                c->data_out_format = LYD_JSON;
            } else if (!strcasecmp(optarg, "lyb")) {
                c->schema_out_format = 0;
                c->data_out_format = LYD_LYB;
            } else if (!strcasecmp(optarg, "feature-param")) {
                c->feature_param_format = 1;
            } else {
                YLMSG_E("Unknown output format %s\n", optarg);
                help(1);
                return -1;
            }
            break;

        case 'p': { /* --path */
            struct stat st;

            if (stat(optarg, &st) == -1) {
                YLMSG_E("Unable to use search path (%s) - %s.\n", optarg, strerror(errno));
                return -1;
            }
            if (!S_ISDIR(st.st_mode)) {
                YLMSG_E("Provided search path is not a directory.\n");
                return -1;
            }

            if (searchpath_strcat(&c->searchpaths, optarg)) {
                YLMSG_E("Storing searchpath failed.\n");
                return -1;
            }

            break;
        } /* case 'p' */

        case 'D': /* --disable-search */
            if (c->ctx_options & LY_CTX_DISABLE_SEARCHDIRS) {
                YLMSG_W("The -D option specified too many times.\n");
            }
            if (c->ctx_options & LY_CTX_DISABLE_SEARCHDIR_CWD) {
                c->ctx_options &= ~LY_CTX_DISABLE_SEARCHDIR_CWD;
                c->ctx_options |= LY_CTX_DISABLE_SEARCHDIRS;
            } else {
                c->ctx_options |= LY_CTX_DISABLE_SEARCHDIR_CWD;
            }
            break;

        case 'F': /* --features */
            if (parse_features(optarg, &c->schema_features)) {
                return -1;
            }
            break;

        case 'i': /* --make-implemented */
            if (c->ctx_options & LY_CTX_REF_IMPLEMENTED) {
                c->ctx_options &= ~LY_CTX_REF_IMPLEMENTED;
                c->ctx_options |= LY_CTX_ALL_IMPLEMENTED;
            } else {
                c->ctx_options |= LY_CTX_REF_IMPLEMENTED;
            }
            break;

        case 'P': /* --schema-node */
            c->schema_node_path = optarg;
            break;

        case 'q': /* --single-node */
            c->schema_print_options |= LYS_PRINT_NO_SUBSTMT;
            break;

        case 's': /* --submodule */
            c->submodule = optarg;
            break;

        case 'x': /* --ext-data */
            c->schema_context_filename = strdup(optarg);
            break;

        case 'n': /* --not-strict */
            c->data_parse_options &= ~LYD_PARSE_STRICT;
            break;

        case 'e': /* --present */
            c->data_validate_options |= LYD_VALIDATE_PRESENT;
            break;

        case 't': /* --type */
            if (data_type_set) {
                YLMSG_E("The data type (-t) cannot be set multiple times.\n");
                return -1;
            }

            if (!strcasecmp(optarg, "config")) {
                c->data_parse_options |= LYD_PARSE_NO_STATE;
                c->data_validate_options |= LYD_VALIDATE_NO_STATE;
            } else if (!strcasecmp(optarg, "get")) {
                c->data_parse_options |= LYD_PARSE_ONLY;
            } else if (!strcasecmp(optarg, "getconfig") || !strcasecmp(optarg, "get-config")) {
                c->data_parse_options |= LYD_PARSE_ONLY | LYD_PARSE_NO_STATE;
            } else if (!strcasecmp(optarg, "edit")) {
                c->data_parse_options |= LYD_PARSE_ONLY;
            } else if (!strcasecmp(optarg, "rpc")) {
                c->data_type = LYD_TYPE_RPC_YANG;
            } else if (!strcasecmp(optarg, "nc-rpc")) {
                c->data_type = LYD_TYPE_RPC_NETCONF;
            } else if (!strcasecmp(optarg, "reply")) {
                c->data_type = LYD_TYPE_REPLY_YANG;
            } else if (!strcasecmp(optarg, "nc-reply")) {
                c->data_type = LYD_TYPE_REPLY_NETCONF;
            } else if (!strcasecmp(optarg, "notif")) {
                c->data_type = LYD_TYPE_NOTIF_YANG;
            } else if (!strcasecmp(optarg, "nc-notif")) {
                c->data_type = LYD_TYPE_NOTIF_NETCONF;
            } else if (!strcasecmp(optarg, "data")) {
                /* default option */
            } else {
                YLMSG_E("Unknown data tree type %s\n", optarg);
                help(1);
                return -1;
            }

            data_type_set = 1;
            break;

        case 'd': /* --default */
            if (!strcasecmp(optarg, "all")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL;
            } else if (!strcasecmp(optarg, "all-tagged")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL_TAG;
            } else if (!strcasecmp(optarg, "trim")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_TRIM;
            } else if (!strcasecmp(optarg, "implicit-tagged")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_IMPL_TAG;
            } else {
                YLMSG_E("Unknown default mode %s\n", optarg);
                help(1);
                return -1;
            }
            break;

        case 'E': /* --data-xpath */
            if (ly_set_add(&c->data_xpath, optarg, 0, NULL)) {
                YLMSG_E("Storing XPath \"%s\" failed.\n", optarg);
                return -1;
            }
            break;

        case 'l': /* --list */
            c->list = 1;
            break;

        case 'L': /* --tree-line-length */
            c->line_length = atoi(optarg);
            break;

        case 'o': /* --output */
            if (c->out) {
                YLMSG_E("Only a single output can be specified.\n");
                return -1;
            } else {
                if (ly_out_new_filepath(optarg, &c->out)) {
                    YLMSG_E("Unable open output file %s (%s)\n", optarg, strerror(errno));
                    return -1;
                }
            }
            break;

        case 'O': /* --operational */
            if (c->data_operational.path) {
                YLMSG_E("The operational datastore (-O) cannot be set multiple times.\n");
                return -1;
            }
            c->data_operational.path = optarg;
            break;

        case 'R': /* --reply-rpc */
            if (c->reply_rpc.path) {
                YLMSG_E("The PRC of the reply (-R) cannot be set multiple times.\n");
                return -1;
            }
            c->reply_rpc.path = optarg;
            break;

        case 'm': /* --merge */
            c->data_merge = 1;
            break;

        case 'y': /* --yang-library */
            c->ctx_options &= ~LY_CTX_NO_YANGLIBRARY;
            break;

        case 'Y': /* --yang-library-file */
            c->ctx_options &= ~LY_CTX_NO_YANGLIBRARY;
            c->yang_lib_file = optarg;
            break;

        case 'X': /* --extended-leafref */
            c->ctx_options |= LY_CTX_LEAFREF_EXTENDED;
            break;

#ifndef NDEBUG
        case 'G': { /* --debug */
            uint32_t dbg_groups = 0;
            const char *ptr = optarg;

            while (ptr[0]) {
                if (!strncasecmp(ptr, "dict", sizeof "dict" - 1)) {
                    dbg_groups |= LY_LDGDICT;
                    ptr += sizeof "dict" - 1;
                } else if (!strncasecmp(ptr, "xpath", sizeof "xpath" - 1)) {
                    dbg_groups |= LY_LDGXPATH;
                    ptr += sizeof "xpath" - 1;
                } else if (!strncasecmp(ptr, "dep-sets", sizeof "dep-sets" - 1)) {
                    dbg_groups |= LY_LDGDEPSETS;
                    ptr += sizeof "dep-sets" - 1;
                }

                if (ptr[0]) {
                    if (ptr[0] != ',') {
                        YLMSG_E("Unknown debug group string \"%s\"\n", optarg);
                        return -1;
                    }
                    ++ptr;
                }
            }
            ly_log_dbg_groups(dbg_groups);
            break;
        } /* case 'G' */
#endif
        default:
            YLMSG_E("Invalid option or missing argument: -%c\n", optopt);
            return -1;
        } /* switch */
    }

    /* additional checks for the options combinations */
    if (!c->list && (optind >= argc)) {
        help(1);
        YLMSG_E("Missing <schema> to process.\n");
        return 1;
    }

    if (c->data_xpath.count) {
        c->data_merge = 1;
    }
    if (c->data_xpath.count && (c->schema_out_format || c->data_out_format)) {
        YLMSG_E("The --format option cannot be combined with --data-xpath option.\n");
        return -1;
    }
    if (c->data_xpath.count && (c->data_print_options & LYD_PRINT_WD_MASK)) {
        YLMSG_E("The --default option cannot be combined with --data-xpath option.\n");
        return -1;
    }

    if (c->data_merge) {
        if (c->data_type || (c->data_parse_options & LYD_PARSE_ONLY)) {
            /* switch off the option, incompatible input data type */
            c->data_merge = 0;
        } else {
            /* postpone validation after the merge of all the input data */
            c->data_parse_options |= LYD_PARSE_ONLY;
        }
    }

    if (c->data_operational.path && !c->data_type) {
        YLMSG_E("Operational datastore takes effect only with RPCs/Actions/Replies/Notification input data types.\n");
        c->data_operational.path = NULL;
    }

    if (c->reply_rpc.path && (c->data_type != LYD_TYPE_REPLY_NETCONF)) {
        YLMSG_E("Source RPC is needed only for NETCONF Reply input data type.\n");
        c->data_operational.path = NULL;
    } else if (!c->reply_rpc.path && (c->data_type == LYD_TYPE_REPLY_NETCONF)) {
        YLMSG_E("Missing source RPC (-R) for NETCONF Reply input data type.\n");
        return -1;
    }

    if ((c->schema_out_format != LYS_OUT_TREE) && c->line_length) {
        YLMSG_E("--tree-line-length take effect only in case of the tree output format.\n");
    }

    /* default output stream */
    if (!c->out) {
        if (ly_out_new_file(stdout, &c->out)) {
            YLMSG_E("Unable to set stdout as output.\n");
            return -1;
        }
    }

    if (c->schema_out_format == LYS_OUT_TREE) {
        /* print tree from lysc_nodes */
        c->ctx_options |= LY_CTX_SET_PRIV_PARSED;
    }

    /* process input files provided as standalone command line arguments,
     * schema modules are parsed and inserted into the context,
     * data files are just checked and prepared into internal structures for further processing */
    ret = fill_context_inputs(argc, argv, c);
    if (ret) {
        return ret;
    }

    /* the second batch of checks */
    if (c->schema_print_options && !c->schema_out_format) {
        YLMSG_W("Schema printer options specified, but the schema output format is missing.\n");
    }
    if (c->schema_parse_options && !c->schema_modules.count) {
        YLMSG_W("Schema parser options specified, but no schema input file provided.\n");
    }
    if (c->data_print_options && !c->data_out_format) {
        YLMSG_W("data printer options specified, but the data output format is missing.\n");
    }
    if (((c->data_parse_options != YL_DEFAULT_DATA_PARSE_OPTIONS) || c->data_type) && !c->data_inputs.count) {
        YLMSG_W("Data parser options specified, but no data input file provided.\n");
    }

    if (c->schema_node_path) {
        c->schema_node = lys_find_path(c->ctx, NULL, c->schema_node_path, 0);
        if (!c->schema_node) {
            c->schema_node = lys_find_path(c->ctx, NULL, c->schema_node_path, 1);

            if (!c->schema_node) {
                YLMSG_E("Invalid schema path.\n");
                return -1;
            }
        }
    }

    return 0;
}

int
main_ni(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS, r;
    struct context c = {0};
    char *features_output = NULL;
    struct ly_set set = {0};
    uint32_t u;

    /* set callback for printing libyang messages */
    ly_set_log_clb(libyang_verbclb, 1);

    r = fill_context(argc, argv, &c);
    if (r < 0) {
        ret = EXIT_FAILURE;
    }
    if (r) {
        goto cleanup;
    }

    /* do the required job - parse, validate, print */

    if (c.list) {
        /* print the list of schemas */
        ret = print_list(c.out, c.ctx, c.data_out_format);
        goto cleanup;
    } else {
        if (c.feature_param_format) {
            for (u = 0; u < c.schema_modules.count; u++) {
                if (collect_features(c.schema_modules.objs[u], &set)) {
                    YLMSG_E("Unable to read features from a module.\n");
                    goto cleanup;
                }
                if (generate_features_output(c.schema_modules.objs[u], &set, &features_output)) {
                    YLMSG_E("Unable to generate feature command output.\n");
                    goto cleanup;
                }
                ly_set_erase(&set, NULL);
            }
            ly_print(c.out, "%s\n", features_output);
        } else if (c.schema_out_format) {
            if (c.schema_node) {
                ret = lys_print_node(c.out, c.schema_node, c.schema_out_format, c.line_length, c.schema_print_options);
                if (ret) {
                    YLMSG_E("Unable to print schema node %s.\n", c.schema_node_path);
                    goto cleanup;
                }
            } else if (c.submodule) {
                const struct lysp_submodule *submod = ly_ctx_get_submodule_latest(c.ctx, c.submodule);

                if (!submod) {
                    YLMSG_E("Unable to find submodule %s.\n", c.submodule);
                    goto cleanup;
                }

                ret = lys_print_submodule(c.out, submod, c.schema_out_format, c.line_length, c.schema_print_options);
                if (ret) {
                    YLMSG_E("Unable to print submodule %s.\n", submod->name);
                    goto cleanup;
                }
            } else {
                for (u = 0; u < c.schema_modules.count; ++u) {
                    ret = lys_print_module(c.out, (struct lys_module *)c.schema_modules.objs[u], c.schema_out_format,
                            c.line_length, c.schema_print_options);
                    /* for YANG Tree Diagrams printing it's more readable to print a blank line between modules. */
                    if ((c.schema_out_format == LYS_OUT_TREE) && (u + 1 < c.schema_modules.count)) {
                        ly_print(c.out, "\n");
                    }
                    if (ret) {
                        YLMSG_E("Unable to print module %s.\n", ((struct lys_module *)c.schema_modules.objs[u])->name);
                        goto cleanup;
                    }
                }
            }
        }

        /* do the data validation despite the schema was printed */
        if (c.data_inputs.size) {
            ret = process_data(c.ctx, c.data_type, c.data_merge, c.data_out_format, c.out, c.data_parse_options,
                    c.data_validate_options, c.data_print_options, &c.data_operational, &c.reply_rpc, &c.data_inputs,
                    &c.data_xpath);
            if (ret) {
                goto cleanup;
            }
        }
    }

cleanup:
    /* cleanup */
    erase_context(&c);
    free(features_output);
    ly_set_erase(&set, NULL);

    return ret;
}
