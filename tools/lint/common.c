/**
 * @file common.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool - common functions for both interactive and non-interactive mode.
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L /* strdup, strndup */

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "compat.h"
#include "libyang.h"

int
parse_schema_path(const char *path, char **dir, char **module)
{
    char *p;

    assert(dir);
    assert(module);

    /* split the path to dirname and basename for further work */
    *dir = strdup(path);
    *module = strrchr(*dir, '/');
    if (!(*module)) {
        *module = *dir;
        *dir = strdup("./");
    } else {
        *module[0] = '\0'; /* break the dir */
        *module = strdup((*module) + 1);
    }
    /* get the pure module name without suffix or revision part of the filename */
    if ((p = strchr(*module, '@'))) {
        /* revision */
        *p = '\0';
    } else if ((p = strrchr(*module, '.'))) {
        /* fileformat suffix */
        *p = '\0';
    }

    return 0;
}

int
get_input(const char *filepath, LYS_INFORMAT *format_schema, LYD_FORMAT *format_data, struct ly_in **in)
{
    struct stat st;

    /* check that the filepath exists and is a regular file */
    if (stat(filepath, &st) == -1) {
        YLMSG_E("Unable to use input filepath (%s) - %s.\n", filepath, strerror(errno));
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        YLMSG_E("Provided input file (%s) is not a regular file.\n", filepath);
        return -1;
    }

    if ((format_schema && !*format_schema) || (format_data && !*format_data)) {
        /* get the file format */
        if (get_format(filepath, format_schema, format_data)) {
            return -1;
        }
    }

    if (ly_in_new_filepath(filepath, 0, in)) {
        YLMSG_E("Unable to process input file.\n");
        return -1;
    }

    return 0;
}

void
free_features(void *flist)
{
    struct schema_features *rec = (struct schema_features *)flist;

    if (rec) {
        free(rec->mod_name);
        if (rec->features) {
            for (uint32_t u = 0; rec->features[u]; ++u) {
                free(rec->features[u]);
            }
            free(rec->features);
        }
        free(rec);
    }
}

void
get_features(struct ly_set *fset, const char *module, const char ***features)
{
    /* get features list for this module */
    for (uint32_t u = 0; u < fset->count; ++u) {
        struct schema_features *sf = (struct schema_features *)fset->objs[u];

        if (!strcmp(module, sf->mod_name)) {
            /* matched module - explicitly set features */
            *features = (const char **)sf->features;
            sf->applied = 1;
            return;
        }
    }

    /* features not set so disable all */
    *features = NULL;
}

int
parse_features(const char *fstring, struct ly_set *fset)
{
    struct schema_features *rec;
    uint32_t count;
    char *p, **fp;

    rec = calloc(1, sizeof *rec);
    if (!rec) {
        YLMSG_E("Unable to allocate features information record (%s).\n", strerror(errno));
        return -1;
    }
    if (ly_set_add(fset, rec, 1, NULL)) {
        YLMSG_E("Unable to store features information (%s).\n", strerror(errno));
        free(rec);
        return -1;
    }

    /* fill the record */
    p = strchr(fstring, ':');
    if (!p) {
        YLMSG_E("Invalid format of the features specification (%s).\n", fstring);
        return -1;
    }
    rec->mod_name = strndup(fstring, p - fstring);

    count = 0;
    while (p) {
        size_t len = 0;
        char *token = p + 1;

        p = strchr(token, ',');
        if (!p) {
            /* the last item, if any */
            len = strlen(token);
        } else {
            len = p - token;
        }

        if (len) {
            fp = realloc(rec->features, (count + 1) * sizeof *rec->features);
            if (!fp) {
                YLMSG_E("Unable to store features list information (%s).\n", strerror(errno));
                return -1;
            }
            rec->features = fp;
            fp = &rec->features[count++]; /* array item to set */
            (*fp) = strndup(token, len);
        }
    }

    /* terminating NULL */
    fp = realloc(rec->features, (count + 1) * sizeof *rec->features);
    if (!fp) {
        YLMSG_E("Unable to store features list information (%s).\n", strerror(errno));
        return -1;
    }
    rec->features = fp;
    rec->features[count++] = NULL;

    return 0;
}

struct cmdline_file *
fill_cmdline_file(struct ly_set *set, struct ly_in *in, const char *path, LYD_FORMAT format)
{
    struct cmdline_file *rec;

    rec = malloc(sizeof *rec);
    if (!rec) {
        YLMSG_E("Allocating memory for data file information failed.\n");
        return NULL;
    }
    rec->in = in;
    rec->path = path;
    rec->format = format;

    if (set && ly_set_add(set, rec, 1, NULL)) {
        free(rec);
        YLMSG_E("Storing data file information failed.\n");
        return NULL;
    }

    return rec;
}

int
collect_features(const struct lys_module *mod, struct ly_set *set)
{
    struct lysp_feature *f = NULL;
    uint32_t idx = 0;

    while ((f = lysp_feature_next(f, mod->parsed, &idx))) {
        if (ly_set_add(set, (void *)f->name, 1, NULL)) {
            YLMSG_E("Memory allocation failed.\n");
            ly_set_erase(set, NULL);
            return 1;
        }
    }

    return 0;
}

void
print_features(struct ly_out *out, const struct lys_module *mod, const struct ly_set *set)
{
    size_t max_len;
    uint32_t j;
    const char *name;

    /* header */
    ly_print(out, "%s:\n", mod->name);

    /* no features */
    if (!set->count) {
        ly_print(out, "\t(none)\n\n");
        return;
    }

    /* get max len, so the statuses of all the features will be aligned */
    max_len = 0;
    for (j = 0; j < set->count; ++j) {
        name = set->objs[j];
        if (strlen(name) > max_len) {
            max_len = strlen(name);
        }
    }

    /* print features */
    for (j = 0; j < set->count; ++j) {
        name = set->objs[j];
        ly_print(out, "\t%-*s (%s)\n", (int)max_len, name, lys_feature_value(mod, name) ? "off" : "on");
    }

    ly_print(out, "\n");
}

int
generate_features_output(const struct lys_module *mod, const struct ly_set *set, char **features_param)
{
    uint32_t j;
    /*
     * features_len - length of all the features in the current module
     * added_len - length of a string to be added, = features_len + extra necessary length
     * param_len - length of the parameter before appending new string
    */
    size_t features_len, added_len, param_len;
    char *tmp;

    features_len = 0;
    for (j = 0; j < set->count; j++) {
        features_len += strlen(set->objs[j]);
    }

    if (j == 0) {
        /* no features */
        added_len = strlen("-F ") + strlen(mod->name) + strlen(":");
    } else {
        /* j = comma count, -1 because of trailing comma */
        added_len = strlen("-F ") + strlen(mod->name) + strlen(":") + features_len + j - 1;
    }

    /* to avoid strlen(NULL) if this is the first call */
    param_len = 0;
    if (*features_param) {
        param_len = strlen(*features_param);
    }

    /* +1 because of white space at the beginning */
    tmp = realloc(*features_param, param_len + added_len + 1 + 1);
    if (!tmp) {
        goto error;
    } else {
        *features_param = tmp;
    }
    sprintf(*features_param + param_len, " -F %s:", mod->name);

    for (j = 0; j < set->count; j++) {
        strcat(*features_param, set->objs[j]);
        /* no trailing comma */
        if (j != (set->count - 1)) {
            strcat(*features_param, ",");
        }
    }

    return 0;

error:
    YLMSG_E("Memory allocation failed (%s:%d, %s).\n", __FILE__, __LINE__, strerror(errno));
    return 1;
}

int
print_all_features(struct ly_out *out, const struct ly_ctx *ctx, ly_bool generate_features, char **features_param)
{
    int ret = 0;
    uint32_t i = 0;
    struct lys_module *mod;
    struct ly_set set = {0};

    while ((mod = ly_ctx_get_module_iter(ctx, &i)) != NULL) {
        /* only care about implemented modules */
        if (!mod->implemented) {
            continue;
        }

        /* always erase the set, so the previous module's features don't carry over to the next module's features */
        ly_set_erase(&set, NULL);

        if (collect_features(mod, &set)) {
            ret = 1;
            goto cleanup;
        }

        if (generate_features && generate_features_output(mod, &set, features_param)) {
            ret = 1;
            goto cleanup;
        }
        print_features(out, mod, &set);
    }

cleanup:
    ly_set_erase(&set, NULL);
    return ret;
}

void
free_cmdline_file(void *cmdline_file)
{
    struct cmdline_file *rec = (struct cmdline_file *)cmdline_file;

    if (rec) {
        ly_in_free(rec->in, 1);
        free(rec);
    }
}

void
free_cmdline(char *argv[])
{
    if (argv) {
        free(argv[0]);
        free(argv);
    }
}

int
parse_cmdline(const char *cmdline, int *argc_p, char **argv_p[])
{
    int count;
    char **vector;
    char *ptr;
    char qmark = 0;

    assert(cmdline);
    assert(argc_p);
    assert(argv_p);

    /* init */
    optind = 0; /* reinitialize getopt() */
    count = 1;
    vector = malloc((count + 1) * sizeof *vector);
    vector[0] = strdup(cmdline);

    /* command name */
    strtok(vector[0], " ");

    /* arguments */
    while ((ptr = strtok(NULL, " "))) {
        size_t len;
        void *r;

        len = strlen(ptr);

        if (qmark) {
            /* still in quotated text */
            /* remove NULL termination of the previous token since it is not a token,
             * but a part of the quotation string */
            ptr[-1] = ' ';

            if ((ptr[len - 1] == qmark) && (ptr[len - 2] != '\\')) {
                /* end of quotation */
                qmark = 0;
                /* shorten the argument by the terminating quotation mark */
                ptr[len - 1] = '\0';
            }
            continue;
        }

        /* another token in cmdline */
        ++count;
        r = realloc(vector, (count + 1) * sizeof *vector);
        if (!r) {
            YLMSG_E("Memory allocation failed (%s:%d, %s).\n", __FILE__, __LINE__, strerror(errno));
            free(vector);
            return -1;
        }
        vector = r;
        vector[count - 1] = ptr;

        if ((ptr[0] == '"') || (ptr[0] == '\'')) {
            /* remember the quotation mark to identify end of quotation */
            qmark = ptr[0];

            /* move the remembered argument after the quotation mark */
            ++vector[count - 1];

            /* check if the quotation is terminated within this token */
            if ((ptr[len - 1] == qmark) && (ptr[len - 2] != '\\')) {
                /* end of quotation */
                qmark = 0;
                /* shorten the argument by the terminating quotation mark */
                ptr[len - 1] = '\0';
            }
        }
    }
    vector[count] = NULL;

    *argc_p = count;
    *argv_p = vector;

    return 0;
}

int
get_format(const char *filename, LYS_INFORMAT *schema, LYD_FORMAT *data)
{
    char *ptr;
    LYS_INFORMAT informat_s;
    LYD_FORMAT informat_d;

    /* get the file format */
    if ((ptr = strrchr(filename, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "yang")) {
            informat_s = LYS_IN_YANG;
            informat_d = 0;
        } else if (!strcmp(ptr, "yin")) {
            informat_s = LYS_IN_YIN;
            informat_d = 0;
        } else if (!strcmp(ptr, "xml")) {
            informat_s = 0;
            informat_d = LYD_XML;
        } else if (!strcmp(ptr, "json")) {
            informat_s = 0;
            informat_d = LYD_JSON;
        } else if (!strcmp(ptr, "lyb")) {
            informat_s = 0;
            informat_d = LYD_LYB;
        } else {
            YLMSG_E("Input file \"%s\" in an unknown format \"%s\".\n", filename, ptr);
            return 0;
        }
    } else {
        YLMSG_E("Input file \"%s\" without file extension - unknown format.\n", filename);
        return 1;
    }

    if (informat_d) {
        if (!data) {
            YLMSG_E("Input file \"%s\" not expected to contain data instances (unexpected format).\n", filename);
            return 2;
        }
        (*data) = informat_d;
    } else if (informat_s) {
        if (!schema) {
            YLMSG_E("Input file \"%s\" not expected to contain schema definition (unexpected format).\n", filename);
            return 3;
        }
        (*schema) = informat_s;
    }

    return 0;
}

int
print_list(struct ly_out *out, struct ly_ctx *ctx, LYD_FORMAT outformat)
{
    struct lyd_node *ylib;
    uint32_t idx = 0, has_modules = 0;
    const struct lys_module *mod;

    if (outformat != LYD_UNKNOWN) {
        if (ly_ctx_get_yanglib_data(ctx, &ylib, "%u", ly_ctx_get_change_count(ctx))) {
            YLMSG_E("Getting context info (ietf-yang-library data) failed. If the YANG module is missing or not implemented, use an option to add it internally.\n");
            return 1;
        }

        lyd_print_all(out, ylib, outformat, 0);
        lyd_free_all(ylib);
        return 0;
    }

    /* iterate schemas in context and provide just the basic info */
    ly_print(out, "List of the loaded models:\n");
    while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
        has_modules++;

        /* conformance print */
        if (mod->implemented) {
            ly_print(out, "    I");
        } else {
            ly_print(out, "    i");
        }

        /* module print */
        ly_print(out, " %s", mod->name);
        if (mod->revision) {
            ly_print(out, "@%s", mod->revision);
        }

        /* submodules print */
        if (mod->parsed && mod->parsed->includes) {
            uint64_t u = 0;

            ly_print(out, " (");
            LY_ARRAY_FOR(mod->parsed->includes, u) {
                ly_print(out, "%s%s", !u ? "" : ",", mod->parsed->includes[u].name);
                if (mod->parsed->includes[u].rev[0]) {
                    ly_print(out, "@%s", mod->parsed->includes[u].rev);
                }
            }
            ly_print(out, ")");
        }

        /* finish the line */
        ly_print(out, "\n");
    }

    if (!has_modules) {
        ly_print(out, "\t(none)\n");
    }

    ly_print_flush(out);
    return 0;
}

int
evaluate_xpath(const struct lyd_node *tree, const char *xpath)
{
    struct ly_set *set = NULL;

    if (lyd_find_xpath(tree, xpath, &set)) {
        return -1;
    }

    /* print result */
    printf("XPath \"%s\" evaluation result:\n", xpath);
    if (!set->count) {
        printf("\tEmpty\n");
    } else {
        for (uint32_t u = 0; u < set->count; ++u) {
            struct lyd_node *node = (struct lyd_node *)set->objs[u];

            printf("  %s \"%s\"", lys_nodetype2str(node->schema->nodetype), node->schema->name);
            if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                printf(" (value: \"%s\")\n", lyd_get_value(node));
            } else if (node->schema->nodetype == LYS_LIST) {
                printf(" (");
                for (struct lyd_node *key = ((struct lyd_node_inner *)node)->child; key && lysc_is_key(key->schema); key = key->next) {
                    printf("%s\"%s\": \"%s\";", (key != ((struct lyd_node_inner *)node)->child) ? " " : "",
                            key->schema->name, lyd_get_value(key));
                }
                printf(")\n");
            }
        }
    }

    ly_set_free(set, NULL);
    return 0;
}

LY_ERR
process_data(struct ly_ctx *ctx, enum lyd_type data_type, uint8_t merge, LYD_FORMAT format, struct ly_out *out,
        uint32_t options_parse, uint32_t options_validate, uint32_t options_print, struct cmdline_file *operational_f,
        struct cmdline_file *rpc_f, struct ly_set *inputs, struct ly_set *xpaths)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *tree = NULL, *op = NULL, *envp = NULL, *merged_tree = NULL, *oper_tree = NULL;
    char *path = NULL;
    struct ly_set *set = NULL;

    /* additional operational datastore */
    if (operational_f && operational_f->in) {
        ret = lyd_parse_data(ctx, NULL, operational_f->in, operational_f->format, LYD_PARSE_ONLY, 0, &oper_tree);
        if (ret) {
            YLMSG_E("Failed to parse operational datastore file \"%s\".\n", operational_f->path);
            goto cleanup;
        }
    }

    for (uint32_t u = 0; u < inputs->count; ++u) {
        struct cmdline_file *input_f = (struct cmdline_file *)inputs->objs[u];

        switch (data_type) {
        case LYD_TYPE_DATA_YANG:
            ret = lyd_parse_data(ctx, NULL, input_f->in, input_f->format, options_parse, options_validate, &tree);
            break;
        case LYD_TYPE_RPC_YANG:
        case LYD_TYPE_REPLY_YANG:
        case LYD_TYPE_NOTIF_YANG:
            ret = lyd_parse_op(ctx, NULL, input_f->in, input_f->format, data_type, &tree, &op);
            break;
        case LYD_TYPE_RPC_NETCONF:
        case LYD_TYPE_NOTIF_NETCONF:
            ret = lyd_parse_op(ctx, NULL, input_f->in, input_f->format, data_type, &envp, &op);

            /* adjust pointers */
            for (tree = op; lyd_parent(tree); tree = lyd_parent(tree)) {}
            break;
        case LYD_TYPE_REPLY_NETCONF:
            /* parse source RPC operation */
            assert(rpc_f && rpc_f->in);
            ret = lyd_parse_op(ctx, NULL, rpc_f->in, rpc_f->format, LYD_TYPE_RPC_NETCONF, &envp, &op);
            if (ret) {
                YLMSG_E("Failed to parse source NETCONF RPC operation file \"%s\".\n", rpc_f->path);
                goto cleanup;
            }

            /* adjust pointers */
            for (tree = op; lyd_parent(tree); tree = lyd_parent(tree)) {}

            /* free input */
            lyd_free_siblings(lyd_child(op));

            /* we do not care */
            lyd_free_all(envp);
            envp = NULL;

            ret = lyd_parse_op(ctx, op, input_f->in, input_f->format, data_type, &envp, NULL);
            break;
        default:
            YLMSG_E("Internal error (%s:%d).\n", __FILE__, __LINE__);
            goto cleanup;
        }

        if (ret) {
            YLMSG_E("Failed to parse input data file \"%s\".\n", input_f->path);
            goto cleanup;
        }

        if (merge) {
            /* merge the data so far parsed for later validation and print */
            if (!merged_tree) {
                merged_tree = tree;
            } else {
                ret = lyd_merge_siblings(&merged_tree, tree, LYD_MERGE_DESTRUCT);
                if (ret) {
                    YLMSG_E("Merging %s with previous data failed.\n", input_f->path);
                    goto cleanup;
                }
            }
            tree = NULL;
        } else if (format) {
            /* print */
            switch (data_type) {
            case LYD_TYPE_DATA_YANG:
                lyd_print_all(out, tree, format, options_print);
                break;
            case LYD_TYPE_RPC_YANG:
            case LYD_TYPE_REPLY_YANG:
            case LYD_TYPE_NOTIF_YANG:
            case LYD_TYPE_RPC_NETCONF:
            case LYD_TYPE_NOTIF_NETCONF:
                lyd_print_tree(out, tree, format, options_print);
                break;
            case LYD_TYPE_REPLY_NETCONF:
                /* just the output */
                lyd_print_tree(out, lyd_child(tree), format, options_print);
                break;
            default:
                assert(0);
            }
        } else {
            /* validation of the RPC/Action/reply/Notification with the operational datastore, if any */
            switch (data_type) {
            case LYD_TYPE_DATA_YANG:
                /* already validated */
                break;
            case LYD_TYPE_RPC_YANG:
            case LYD_TYPE_RPC_NETCONF:
                ret = lyd_validate_op(tree, oper_tree, LYD_TYPE_RPC_YANG, NULL);
                break;
            case LYD_TYPE_REPLY_YANG:
            case LYD_TYPE_REPLY_NETCONF:
                ret = lyd_validate_op(tree, oper_tree, LYD_TYPE_REPLY_YANG, NULL);
                break;
            case LYD_TYPE_NOTIF_YANG:
            case LYD_TYPE_NOTIF_NETCONF:
                ret = lyd_validate_op(tree, oper_tree, LYD_TYPE_NOTIF_YANG, NULL);
                break;
            default:
                assert(0);
            }
            if (ret) {
                if (operational_f->path) {
                    YLMSG_E("Failed to validate input data file \"%s\" with operational datastore \"%s\".\n",
                            input_f->path, operational_f->path);
                } else {
                    YLMSG_E("Failed to validate input data file \"%s\".\n", input_f->path);
                }
                goto cleanup;
            }

            if (op && oper_tree && lyd_parent(op)) {
                /* check operation parent existence */
                path = lyd_path(lyd_parent(op), LYD_PATH_STD, NULL, 0);
                if (!path) {
                    ret = LY_EMEM;
                    goto cleanup;
                }
                if ((ret = lyd_find_xpath(oper_tree, path, &set))) {
                    goto cleanup;
                }
                if (!set->count) {
                    YLMSG_E("Operation \"%s\" parent \"%s\" not found in the operational data.\n", LYD_NAME(op), path);
                    ret = LY_EVALID;
                    goto cleanup;
                }
            }
        }

        /* next iter */
        lyd_free_all(tree);
        tree = NULL;
        lyd_free_all(envp);
        envp = NULL;
    }

    if (merge) {
        /* validate the merged result */
        ret = lyd_validate_all(&merged_tree, ctx, LYD_VALIDATE_PRESENT, NULL);
        if (ret) {
            YLMSG_E("Merged data are not valid.\n");
            goto cleanup;
        }

        if (format) {
            /* and print it */
            lyd_print_all(out, merged_tree, format, options_print);
        }

        for (uint32_t u = 0; xpaths && (u < xpaths->count); ++u) {
            if (evaluate_xpath(merged_tree, (const char *)xpaths->objs[u])) {
                goto cleanup;
            }
        }
    }

cleanup:
    lyd_free_all(tree);
    lyd_free_all(envp);
    lyd_free_all(merged_tree);
    lyd_free_all(oper_tree);
    free(path);
    ly_set_free(set, NULL);
    return ret;
}

const struct lysc_node *
find_schema_path(const struct ly_ctx *ctx, const char *schema_path)
{
    const char *end, *module_name_end;
    char *module_name = NULL;
    const struct lysc_node *node = NULL, *parent_node = NULL, *parent_node_tmp;
    const struct lys_module *module;
    size_t node_name_len;
    ly_bool found_exact_match = 0;

    /* iterate over each '/' in the path */
    while (schema_path) {
        /* example: schema_path = /listen/endpoint
         * end == NULL for endpoint, end exists for listen */
        end = strchr(schema_path + 1, '/');
        if (end) {
            node_name_len = end - schema_path - 1;
        } else {
            node_name_len = strlen(schema_path + 1);
        }

        /* ex: schema_path = /ietf-interfaces:interfaces/interface/ietf-ip:ipv4 */
        module_name_end = strchr(schema_path, ':');
        if (module_name_end && (!end || (module_name_end < end))) {
            /* only get module's name, if it is in the current scope */
            free(module_name);
            /* - 1 because module_name_end points to ':' */
            module_name = strndup(schema_path + 1, module_name_end - schema_path - 1);
            if (!module_name) {
                YLMSG_E("Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                parent_node = NULL;
                goto cleanup;
            }
            /* move the pointer to the beginning of the node's name - 1 */
            schema_path = module_name_end;

            /* recalculate the length of the node's name, because the module prefix mustn't be compared later */
            if (module_name_end < end) {
                node_name_len = end - module_name_end - 1;
            } else if (!end) {
                node_name_len = strlen(module_name_end + 1);
            }
        }

        module = ly_ctx_get_module_implemented(ctx, module_name);
        if (!module) {
            /* unknown module name */
            parent_node = NULL;
            goto cleanup;
        }

        /* iterate over the node's siblings / module's top level containers */
        while ((node = lys_getnext(node, parent_node, module->compiled, LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHCHOICE))) {
            if (end && !strncmp(node->name, schema_path + 1, node_name_len) && (node->name[node_name_len] == '\0')) {
                /* check if the whole node's name matches and it's not just a common prefix */
                parent_node = node;
                break;
            } else if (!strncmp(node->name, schema_path + 1, node_name_len)) {
                /* do the same here, however if there is no exact match, use the last node with the same prefix */
                if (strlen(node->name) == node_name_len) {
                    parent_node = node;
                    found_exact_match = 1;
                    break;
                } else {
                    parent_node_tmp = node;
                }
            }
        }

        if (!end && !found_exact_match) {
            /* no exact match */
            parent_node = parent_node_tmp;
        }
        found_exact_match = 0;

        /* next iter */
        schema_path = strchr(schema_path + 1, '/');
    }

cleanup:
    free(module_name);
    return parent_node;
}
