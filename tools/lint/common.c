/**
 * @file common.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool - common functions for both interactive and non-interactive mode.
 *
 * Copyright (c) 2023 CESNET, z.s.p.o.
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
#include "plugins_exts.h"
#include "yl_opt.h"

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

    if (get_format(filepath, format_schema, format_data)) {
        return -1;
    }

    if (in && ly_in_new_filepath(filepath, 0, in)) {
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
get_features(const struct ly_set *fset, const char *module, const char ***features)
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

LYS_INFORMAT
get_schema_format(const char *filename)
{
    char *ptr;

    if ((ptr = strrchr(filename, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "yang")) {
            return LYS_IN_YANG;
        } else if (!strcmp(ptr, "yin")) {
            return LYS_IN_YIN;
        } else {
            return LYS_IN_UNKNOWN;
        }
    } else {
        return LYS_IN_UNKNOWN;
    }
}

LYD_FORMAT
get_data_format(const char *filename)
{
    char *ptr;

    if ((ptr = strrchr(filename, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "xml")) {
            return LYD_XML;
        } else if (!strcmp(ptr, "json")) {
            return LYD_JSON;
        } else if (!strcmp(ptr, "lyb")) {
            return LYD_LYB;
        } else {
            return LYD_UNKNOWN;
        }
    } else {
        return LYD_UNKNOWN;
    }
}

int
get_format(const char *filepath, LYS_INFORMAT *schema_form, LYD_FORMAT *data_form)
{
    LYS_INFORMAT schema;
    LYD_FORMAT data;

    schema = !schema_form || !*schema_form ? LYS_IN_UNKNOWN : *schema_form;
    data = !data_form || !*data_form ? LYD_UNKNOWN : *data_form;

    if (!schema) {
        schema = get_schema_format(filepath);
    }
    if (!data) {
        data = get_data_format(filepath);
    }

    if (!schema && !data) {
        YLMSG_E("Input schema format for %s file not recognized.", filepath);
        return -1;
    } else if (!data && !schema) {
        YLMSG_E("Input data format for %s file not recognized.", filepath);
        return -1;
    }
    assert(schema || data);

    if (schema_form) {
        *schema_form = schema;
    }
    if (data_form) {
        *data_form = data;
    }

    return 0;
}

const struct lysc_node *
find_schema_path(const struct ly_ctx *ctx, const char *schema_path)
{
    const char *end, *module_name_end;
    char *module_name = NULL;
    const struct lysc_node *node = NULL, *parent_node = NULL, *parent_node_tmp = NULL;
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

LY_ERR
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

LY_ERR
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
