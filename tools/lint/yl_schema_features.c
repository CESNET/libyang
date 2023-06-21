/**
 * @file yl_schema_features.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Control features for the schema.
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

#include <errno.h>
#include <stdlib.h> /* calloc */
#include <string.h> /* strcmp */

#include "compat.h" /* strndup */
#include "set.h" /* ly_set */

#include "common.h"
#include "yl_schema_features.h"

void
yl_schema_features_free(void *flist)
{
    struct yl_schema_features *rec = (struct yl_schema_features *)flist;

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
        struct yl_schema_features *sf = (struct yl_schema_features *)fset->objs[u];

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
    struct yl_schema_features *rec;
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
print_all_features(struct ly_out *out, const struct ly_ctx *ctx, uint8_t generate_features, char **features_param)
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
