/**
 * @file main_ni.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool - non-interactive code
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
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

#include "cmd.h"
#include "common.h"
#include "out.h"
#include "tools/config.h"
#include "yl_opt.h"

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
            "        <operational-file> with possible references to the operational datastore data.\n"
            "        To validate nested-notification or action, the <operational-file> is required.\n\n"
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

    printf("  -I FORMAT, --in-format=FORMAT\n"
            "                Load the data in one of the following formats:\n"
            "                xml, json, lyb\n"
            "                If input format not specified, it is detected from the file extension.\n\n");

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
            "                Evaluate XPATH expression over the data and print the nodes satisfying\n"
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
            "                In case of a nested notification or action, its parent existence\n"
            "                is also checked in these operational data.\n\n");

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

static int
fill_context_inputs(int argc, char *argv[], struct yl_opt *yo, struct ly_ctx **ctx)
{
    struct ly_in *in = NULL;
    struct schema_features *sf;
    struct lys_module *mod;
    const char *all_features[] = {"*", NULL};
    char *dir = NULL, *module = NULL, *filepath = NULL;

    /* Create libyang context. */
    if (yo->yang_lib_file) {
        /* ignore features */
        ly_set_erase(&yo->schema_features, free_features);

        if (ly_ctx_new_ylpath(yo->searchpaths, yo->yang_lib_file, LYD_UNKNOWN, yo->ctx_options, ctx)) {
            YLMSG_E("Unable to modify libyang context with yang-library data.\n");
            return -1;
        }
    } else {
        /* set imp feature flag if all should be enabled */
        yo->ctx_options |= !yo->schema_features.count ? LY_CTX_ENABLE_IMP_FEATURES : 0;

        if (ly_ctx_new(yo->searchpaths, yo->ctx_options, ctx)) {
            YLMSG_E("Unable to create libyang context\n");
            return -1;
        }
    }

    /* set callback providing run-time extension instance data */
    if (yo->schema_context_filename) {
        ly_ctx_set_ext_data_clb(*ctx, ext_data_clb, yo->schema_context_filename);
    }

    for (int i = 0; i < argc - optind; i++) {
        LYS_INFORMAT format_schema = LYS_IN_UNKNOWN;
        LYD_FORMAT format_data = yo->data_in_format;

        filepath = argv[optind + i];

        if (!filepath) {
            goto error;
        }
        if (get_input(filepath, &format_schema, &format_data, &in)) {
            goto error;
        }

        if (format_schema) {
            LY_ERR ret;
            uint8_t path_unset = 1; /* flag to unset the path from the searchpaths list (if not already present) */
            const char **features;

            /* parse the input */
            if (parse_schema_path(filepath, &dir, &module)) {
                goto error;
            }

            mod = NULL;
            if (yo->yang_lib_file) {
                /* just get the module, it should already be parsed */
                mod = ly_ctx_get_module_implemented(*ctx, module);
            }
            if (!mod) {
                /* add temporarily also the path of the module itself */
                if (ly_ctx_set_searchdir(*ctx, dir) == LY_EEXIST) {
                    path_unset = 0;
                }

                /* get features list for this module */
                if (!yo->schema_features.count) {
                    features = all_features;
                } else {
                    get_features(&yo->schema_features, module, &features);
                }

                /* parse module */
                ret = lys_parse(*ctx, in, format_schema, features, &mod);
                ly_ctx_unset_searchdir_last(*ctx, path_unset);
                if (ret) {
                    YLMSG_E("Parsing schema module \"%s\" failed.\n", filepath);
                    goto error;
                }
            }

            /* temporary cleanup */
            free(dir);
            dir = NULL;
            free(module);
            module = NULL;

            if (yo->schema_out_format || yo->feature_param_format) {
                /* module will be printed */
                if (ly_set_add(&yo->schema_modules, (void *)mod, 1, NULL)) {
                    YLMSG_E("Storing parsed schema module (%s) for print failed.\n", filepath);
                    goto error;
                }
            }
        } else if (format_data) {
            if (!fill_cmdline_file(&yo->data_inputs, in, filepath, format_data)) {
                goto error;
            }
            in = NULL;
        }

        ly_in_free(in, 1);
        in = NULL;
    }

    /* check that all specified features were applied, apply now if possible */
    while ((sf = get_features_not_applied(&yo->schema_features))) {
        /* try to find implemented or the latest revision of this module */
        mod = ly_ctx_get_module_implemented(*ctx, sf->mod_name);
        if (!mod) {
            mod = ly_ctx_get_module_latest(*ctx, sf->mod_name);
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

#ifndef NDEBUG
/**
 * @brief Enable specific debugging messages.
 *
 * @param[in] groups String in the form <group>[,group>]*.
 * @param[in,out] yo Options for yanglint.
 * return 0 on success.
 */
static int
set_debug_groups(char *groups, struct yl_opt *yo)
{
    int rc;
    char *str, *end;

    /* Process all debug arguments except the last one. */
    for (str = groups; (end = strchr(str, ',')); str = end + 1) {
        /* Temporary modify input string. */
        *end = '\0';
        rc = cmd_debug_exec(NULL, yo, str);
        *end = ',';
        if (rc) {
            return -1;
        }
    }
    /* Process single/last debug argument. */
    if (cmd_debug_exec(NULL, yo, str)) {
        return -1;
    }
    /* All debug arguments are valid, so they can apply. */
    if (cmd_debug_fin(NULL, yo)) {
        return -1;
    }

    return 0;
}

#endif

/**
 * @brief Process command line options and store the settings into the context.
 *
 * return -1 in case of error;
 * return 0 in case of success and ready to process
 * return 1 in case of success, but expect to exit.
 */
static int
fill_context(int argc, char *argv[], struct yl_opt *yo, struct ly_ctx **ctx)
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
        {"in-format",         required_argument, NULL, 'I'},
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
    uint32_t temp_lo = 0;

    yo->ctx_options = YL_DEFAULT_CTX_OPTIONS;
    yo->data_parse_options = YL_DEFAULT_DATA_PARSE_OPTIONS;
    yo->data_validate_options = YL_DEFAULT_DATA_VALIDATE_OPTIONS;
    yo->line_length = 0;

    opterr = 0;
#ifndef NDEBUG
    while ((opt = getopt_long(argc, argv, "hvVQf:I:p:DF:iP:qs:neE:t:d:lL:o:O:R:myY:Xx:G:", options, &opt_index)) != -1)
#else
    while ((opt = getopt_long(argc, argv, "hvVQf:I:p:DF:iP:qs:neE:t:d:lL:o:O:R:myY:Xx:", options, &opt_index)) != -1)
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
            if (yl_opt_update_out_format(optarg, yo)) {
                help(1);
                return -1;
            }
            break;

        case 'I': /* --in-format */
            if (!strcasecmp(optarg, "xml")) {
                yo->data_in_format = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                yo->data_in_format = LYD_JSON;
            } else if (!strcasecmp(optarg, "lyb")) {
                yo->data_in_format = LYD_LYB;
            } else {
                YLMSG_E("Unknown input format %s\n", optarg);
                help(1);
                return -1;
            }
            break;

        case 'p':   /* --path */
            if (searchpath_strcat(&yo->searchpaths, optarg)) {
                YLMSG_E("Storing searchpath failed.\n");
                return -1;
            }
            break;
        /* case 'p' */

        case 'D': /* --disable-search */
            if (yo->ctx_options & LY_CTX_DISABLE_SEARCHDIRS) {
                YLMSG_W("The -D option specified too many times.\n");
            }
            if (yo->ctx_options & LY_CTX_DISABLE_SEARCHDIR_CWD) {
                yo->ctx_options &= ~LY_CTX_DISABLE_SEARCHDIR_CWD;
                yo->ctx_options |= LY_CTX_DISABLE_SEARCHDIRS;
            } else {
                yo->ctx_options |= LY_CTX_DISABLE_SEARCHDIR_CWD;
            }
            break;

        case 'F': /* --features */
            if (parse_features(optarg, &yo->schema_features)) {
                return -1;
            }
            break;

        case 'i': /* --make-implemented */
            if (yo->ctx_options & LY_CTX_REF_IMPLEMENTED) {
                yo->ctx_options &= ~LY_CTX_REF_IMPLEMENTED;
                yo->ctx_options |= LY_CTX_ALL_IMPLEMENTED;
            } else {
                yo->ctx_options |= LY_CTX_REF_IMPLEMENTED;
            }
            break;

        case 'P': /* --schema-node */
            yo->schema_node_path = optarg;
            break;

        case 'q': /* --single-node */
            yo->schema_print_options |= LYS_PRINT_NO_SUBSTMT;
            break;

        case 's': /* --submodule */
            yo->submodule = optarg;
            break;

        case 'x': /* --ext-data */
            yo->schema_context_filename = optarg;
            break;

        case 'n': /* --not-strict */
            yo->data_parse_options &= ~LYD_PARSE_STRICT;
            break;

        case 'e': /* --present */
            yo->data_validate_options |= LYD_VALIDATE_PRESENT;
            break;

        case 't': /* --type */
            if (data_type_set) {
                YLMSG_E("The data type (-t) cannot be set multiple times.\n");
                return -1;
            }

            if (!strcasecmp(optarg, "config")) {
                yo->data_parse_options |= LYD_PARSE_NO_STATE;
                yo->data_validate_options |= LYD_VALIDATE_NO_STATE;
            } else if (!strcasecmp(optarg, "get")) {
                yo->data_parse_options |= LYD_PARSE_ONLY;
            } else if (!strcasecmp(optarg, "getconfig") || !strcasecmp(optarg, "get-config") || !strcasecmp(optarg, "edit")) {
                yo->data_parse_options |= LYD_PARSE_ONLY | LYD_PARSE_NO_STATE;
            } else if (!strcasecmp(optarg, "rpc")) {
                yo->data_type = LYD_TYPE_RPC_YANG;
            } else if (!strcasecmp(optarg, "nc-rpc")) {
                yo->data_type = LYD_TYPE_RPC_NETCONF;
            } else if (!strcasecmp(optarg, "reply")) {
                yo->data_type = LYD_TYPE_REPLY_YANG;
            } else if (!strcasecmp(optarg, "nc-reply")) {
                yo->data_type = LYD_TYPE_REPLY_NETCONF;
            } else if (!strcasecmp(optarg, "notif")) {
                yo->data_type = LYD_TYPE_NOTIF_YANG;
            } else if (!strcasecmp(optarg, "nc-notif")) {
                yo->data_type = LYD_TYPE_NOTIF_NETCONF;
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
                yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL;
            } else if (!strcasecmp(optarg, "all-tagged")) {
                yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL_TAG;
            } else if (!strcasecmp(optarg, "trim")) {
                yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_TRIM;
            } else if (!strcasecmp(optarg, "implicit-tagged")) {
                yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_IMPL_TAG;
            } else {
                YLMSG_E("Unknown default mode %s\n", optarg);
                help(1);
                return -1;
            }
            break;

        case 'E': /* --data-xpath */
            if (ly_set_add(&yo->data_xpath, optarg, 0, NULL)) {
                YLMSG_E("Storing XPath \"%s\" failed.\n", optarg);
                return -1;
            }
            break;

        case 'l': /* --list */
            yo->list = 1;
            break;

        case 'L': /* --tree-line-length */
            yo->line_length = atoi(optarg);
            break;

        case 'o': /* --output */
            if (yo->out) {
                YLMSG_E("Only a single output can be specified.\n");
                return -1;
            } else {
                if (ly_out_new_filepath(optarg, &yo->out)) {
                    YLMSG_E("Unable open output file %s (%s)\n", optarg, strerror(errno));
                    return -1;
                }
            }
            break;

        case 'O': /* --operational */
            if (yo->data_operational.path) {
                YLMSG_E("The operational datastore (-O) cannot be set multiple times.\n");
                return -1;
            }
            yo->data_operational.path = optarg;
            break;

        case 'R': /* --reply-rpc */
            if (yo->reply_rpc.path) {
                YLMSG_E("The PRC of the reply (-R) cannot be set multiple times.\n");
                return -1;
            }
            yo->reply_rpc.path = optarg;
            break;

        case 'm': /* --merge */
            yo->data_merge = 1;
            break;

        case 'y': /* --yang-library */
            yo->ctx_options &= ~LY_CTX_NO_YANGLIBRARY;
            break;

        case 'Y': /* --yang-library-file */
            yo->ctx_options &= ~LY_CTX_NO_YANGLIBRARY;
            yo->yang_lib_file = optarg;
            break;

        case 'X': /* --extended-leafref */
            yo->ctx_options |= LY_CTX_LEAFREF_EXTENDED;
            break;

#ifndef NDEBUG
        case 'G':   /* --debug */
            if (set_debug_groups(optarg, yo)) {
                return -1;
            }
            break;
            /* case 'G' */
#endif
        default:
            YLMSG_E("Invalid option or missing argument: -%c\n", optopt);
            return -1;
        } /* switch */
    }

    /* additional checks for the options combinations */
    if (!yo->list && (optind >= argc)) {
        help(1);
        YLMSG_E("Missing <schema> to process.\n");
        return 1;
    }

    if (cmd_data_dep(yo, 0)) {
        return -1;
    }
    if (cmd_print_dep(yo, 0)) {
        return -1;
    }

    if (yo->schema_out_format == LYS_OUT_TREE) {
        /* print tree from lysc_nodes */
        yo->ctx_options |= LY_CTX_SET_PRIV_PARSED;
    }

    /* process input files provided as standalone command line arguments,
     * schema modules are parsed and inserted into the context,
     * data files are just checked and prepared into internal structures for further processing */
    ret = fill_context_inputs(argc, argv, yo, ctx);
    if (ret) {
        return ret;
    }

    /* the second batch of checks */
    if (yo->schema_print_options && !yo->schema_out_format) {
        YLMSG_W("Schema printer options specified, but the schema output format is missing.\n");
    }
    if (yo->schema_parse_options && !yo->schema_modules.count) {
        YLMSG_W("Schema parser options specified, but no schema input file provided.\n");
    }
    if (yo->data_print_options && !yo->data_out_format) {
        YLMSG_W("data printer options specified, but the data output format is missing.\n");
    }
    if (((yo->data_parse_options != YL_DEFAULT_DATA_PARSE_OPTIONS) || yo->data_type) && !yo->data_inputs.count) {
        YLMSG_W("Data parser options specified, but no data input file provided.\n");
    }

    if (yo->schema_node_path) {
        /* turn off logging so that the message is not repeated */
        ly_temp_log_options(&temp_lo);
        /* search operation input */
        yo->schema_node = lys_find_path(*ctx, NULL, yo->schema_node_path, 0);
        if (!yo->schema_node) {
            /* restore logging so an error may be displayed */
            ly_temp_log_options(NULL);
            /* search operation output */
            yo->schema_node = lys_find_path(*ctx, NULL, yo->schema_node_path, 1);
            if (!yo->schema_node) {
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
    struct yl_opt yo = {0};
    struct ly_ctx *ctx = NULL;
    char *features_output = NULL;
    struct ly_set set = {0};
    uint32_t u;

    /* set callback for printing libyang messages */
    ly_set_log_clb(libyang_verbclb, 1);

    r = fill_context(argc, argv, &yo, &ctx);
    if (r < 0) {
        ret = EXIT_FAILURE;
    }
    if (r) {
        goto cleanup;
    }

    /* do the required job - parse, validate, print */

    if (yo.list) {
        /* print the list of schemas */
        ret = print_list(yo.out, ctx, yo.data_out_format);
        goto cleanup;
    } else {
        if (yo.feature_param_format) {
            for (u = 0; u < yo.schema_modules.count; u++) {
                if (collect_features(yo.schema_modules.objs[u], &set)) {
                    YLMSG_E("Unable to read features from a module.\n");
                    goto cleanup;
                }
                if (generate_features_output(yo.schema_modules.objs[u], &set, &features_output)) {
                    YLMSG_E("Unable to generate feature command output.\n");
                    goto cleanup;
                }
                ly_set_erase(&set, NULL);
            }
            ly_print(yo.out, "%s\n", features_output);
        } else if (yo.schema_out_format) {
            if (yo.schema_node) {
                ret = lys_print_node(yo.out, yo.schema_node, yo.schema_out_format, yo.line_length, yo.schema_print_options);
                if (ret) {
                    YLMSG_E("Unable to print schema node %s.\n", yo.schema_node_path);
                    goto cleanup;
                }
            } else if (yo.submodule) {
                const struct lysp_submodule *submod = ly_ctx_get_submodule_latest(ctx, yo.submodule);

                if (!submod) {
                    YLMSG_E("Unable to find submodule %s.\n", yo.submodule);
                    goto cleanup;
                }

                ret = lys_print_submodule(yo.out, submod, yo.schema_out_format, yo.line_length, yo.schema_print_options);
                if (ret) {
                    YLMSG_E("Unable to print submodule %s.\n", submod->name);
                    goto cleanup;
                }
            } else {
                for (u = 0; u < yo.schema_modules.count; ++u) {
                    ret = lys_print_module(yo.out, (struct lys_module *)yo.schema_modules.objs[u], yo.schema_out_format,
                            yo.line_length, yo.schema_print_options);
                    /* for YANG Tree Diagrams printing it's more readable to print a blank line between modules. */
                    if ((yo.schema_out_format == LYS_OUT_TREE) && (u + 1 < yo.schema_modules.count)) {
                        ly_print(yo.out, "\n");
                    }
                    if (ret) {
                        YLMSG_E("Unable to print module %s.\n", ((struct lys_module *)yo.schema_modules.objs[u])->name);
                        goto cleanup;
                    }
                }
            }
        }

        /* do the data validation despite the schema was printed */
        if (yo.data_inputs.size) {
            ret = process_data(ctx, yo.data_type, yo.data_merge, yo.data_out_format, yo.out, yo.data_parse_options,
                    yo.data_validate_options, yo.data_print_options, &yo.data_operational, &yo.reply_rpc, &yo.data_inputs,
                    &yo.data_xpath);
            if (ret) {
                goto cleanup;
            }
        }
    }

cleanup:
    /* cleanup */
    yl_opt_erase(&yo);
    ly_ctx_destroy(ctx);
    free(features_output);
    ly_set_erase(&set, NULL);

    return ret;
}
