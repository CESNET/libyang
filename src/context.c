/**
 * @file context.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief context implementation for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
#include "context.h"
#include "dict.h"
#include "parser.h"
#include "tree_internal.h"

#define IETF_INET_TYPES_PATH "../models/ietf-inet-types@2013-07-15.h"
#define IETF_YANG_TYPES_PATH "../models/ietf-yang-types@2013-07-15.h"
#define IETF_YANG_LIB_PATH "../models/ietf-yang-library@2015-07-03.h"

#include IETF_INET_TYPES_PATH
#include IETF_YANG_TYPES_PATH
#include IETF_YANG_LIB_PATH

API struct ly_ctx *
ly_ctx_new(const char *search_dir)
{
    struct ly_ctx *ctx;
    char *cwd;

    ctx = calloc(1, sizeof *ctx);
    if (!ctx) {
        LOGMEM;
        return NULL;
    }

    /* dictionary */
    lydict_init(&ctx->dict);

    /* models list */
    ctx->models.list = calloc(16, sizeof *ctx->models.list);
    ctx->models.used = 0;
    ctx->models.size = 16;
    if (search_dir) {
        cwd = get_current_dir_name();
        if (chdir(search_dir)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            free(cwd);
            ly_ctx_destroy(ctx);
            return NULL;
        }
        ctx->models.search_path = get_current_dir_name();
        chdir(cwd);
        free(cwd);
    }
    ctx->models.module_set_id = 1;

    /* load ietf-inet-types */
    ctx->models.list[0] = lys_parse(ctx, (char *)ietf_inet_types_2013_07_15_yin, LYS_IN_YIN);
    if (!ctx->models.list[0]) {
        ly_ctx_destroy(ctx);
        return NULL;
    }

    /* load ietf-yang-types */
    ctx->models.list[1] = lys_parse(ctx, (char *)ietf_yang_types_2013_07_15_yin, LYS_IN_YIN);
    if (!ctx->models.list[1]) {
        ly_ctx_destroy(ctx);
        return NULL;
    }

    /* load ietf-yang-library */
    ctx->models.list[2] = lys_parse(ctx, (char *)ietf_yang_library_2015_07_03_yin, LYS_IN_YIN);
    if (!ctx->models.list[2]) {
        ly_ctx_destroy(ctx);
        return NULL;
    }

    return ctx;
}

API void
ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir)
{
    char *cwd;

    if (!ctx) {
        return;
    }

    if (search_dir) {
        cwd = get_current_dir_name();
        if (chdir(search_dir)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            free(cwd);
            return;
        }
        ctx->models.search_path = get_current_dir_name();
        chdir(cwd);
        free(cwd);
    } else {
        free(ctx->models.search_path);
        ctx->models.search_path = NULL;
    }
}

const char *
ly_ctx_get_searchdir(struct ly_ctx *ctx)
{
    return ctx->models.search_path;
}

API void
ly_ctx_destroy(struct ly_ctx *ctx)
{
    if (!ctx) {
        return;
    }

    /* models list */
    while (ctx->models.used) {
        lys_free(ctx->models.list[0], 1);
    }
    free(ctx->models.search_path);
    free(ctx->models.list);

    /* dictionary */
    lydict_clean(&ctx->dict);

    free(ctx);
}

API struct lys_submodule *
ly_ctx_get_submodule(struct lys_module *module, const char *name, const char *revision)
{
    struct lys_submodule *result;
    int i;

    if (!module || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    /* TODO search also for submodules not directly available from the main module */

    /* search in modules included by the main module */
    if (module->type) {
        module = ((struct lys_submodule *)module)->belongsto;
    }
    for (i = 0; i < module->inc_size; i++) {
        result = module->inc[i].submodule;
        if (strcmp(name, result->name)) {
            continue;
        }

        if (!revision || (result->rev_size && !strcmp(revision, result->rev[0].date))) {
            return result;
        }
    }

    return NULL;
}

static struct lys_module *
ly_ctx_get_module_by(struct ly_ctx *ctx, const char *key, int offset, const char *revision)
{
    int i;
    struct lys_module *result = NULL;

    if (!ctx || !key) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    for (i = 0; i < ctx->models.used; i++) {
        /* use offset to get address of the pointer to string (char**), remember that offset is in
         * bytes, so we have to cast the pointer to the module to (char*), finally, we want to have
         * string not the pointer to string
         */
        if (!ctx->models.list[i] || strcmp(key, *(char**)(((char*)ctx->models.list[i]) + offset))) {
            continue;
        }

        if (!revision) {
            /* compare revisons and remember the newest one */
            if (result) {
                if (!ctx->models.list[i]->rev_size) {
                    /* the current have no revision, keep the previous with some revision */
                    continue;
                }
                if (result->rev_size && strcmp(ctx->models.list[i]->rev[0].date, result->rev[0].date) < 0) {
                    /* the previous found matching module has a newer revision */
                    continue;
                }
            }

            /* remember the current match and search for newer version */
            result = ctx->models.list[i];
        } else {
            if (ctx->models.list[i]->rev_size && !strcmp(revision, ctx->models.list[i]->rev[0].date)) {
                /* matching revision */
                result = ctx->models.list[i];
                break;
            }
        }
    }

    return result;

}

API struct lys_module *
ly_ctx_get_module_by_ns(struct ly_ctx *ctx, const char *ns, const char *revision)
{
    return ly_ctx_get_module_by(ctx, ns, offsetof(struct lys_module, ns), revision);
}

API struct lys_module *
ly_ctx_get_module(struct ly_ctx *ctx, const char *name, const char *revision)
{
    return ly_ctx_get_module_by(ctx, name, offsetof(struct lys_module, name), revision);
}

API const char **
ly_ctx_get_module_names(struct ly_ctx *ctx)
{
    int i;
    const char **result = NULL;

    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    result = malloc((ctx->models.used+1) * sizeof *result);

    for (i = 0; i < ctx->models.used; i++) {
        result[i] = ctx->models.list[i]->name;
    }
    result[i] = NULL;

    return result;
}

API const char **
ly_ctx_get_submodule_names(struct ly_ctx *ctx, const char *module_name)
{
    int i;
    const char **result = NULL;
    struct lys_module *mod;

    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    mod = ly_ctx_get_module(ctx, module_name, NULL);
    if (!mod) {
        LOGERR(LY_EVALID, "Data model \"%s\" not loaded", module_name);
        return NULL;
    }

    result = malloc((mod->inc_size+1) * sizeof *result);

    for (i = 0; i < mod->inc_size; i++) {
        result[i] = mod->inc[i].submodule->name;
    }
    result[i] = NULL;

    return result;
}

static int
ylib_feature(struct lyd_node *parent, struct lys_module *cur_mod)
{
    int i, j;

    /* module features */
    for (i = 0; i < cur_mod->features_size; ++i) {
        if (!(cur_mod->features[i].flags & LYS_FENABLED)) {
            continue;
        }

        if (!lyd_new_leaf_str(parent, NULL, "feature", LY_TYPE_STRING, cur_mod->features[i].name)) {
            return EXIT_FAILURE;
        }
    }

    /* submodule features */
    for (i = 0; i < cur_mod->inc_size; ++i) {
        for (j = 0; j < cur_mod->inc[i].submodule->features_size; ++j) {
            if (!(cur_mod->inc[i].submodule->features[j].flags & LYS_FENABLED)) {
                continue;
            }

            if (!lyd_new_leaf_str(parent, NULL, "feature", LY_TYPE_STRING, cur_mod->inc[i].submodule->features[j].name)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
ylib_deviation(struct lyd_node *parent, struct lys_module *cur_mod, struct ly_ctx *ctx)
{
    int i, j, k;
    struct lys_module *target_module, *mod_iter;
    struct lyd_node *cont;

    for (i = 0; i < ctx->models.used; ++i) {
        mod_iter = ctx->models.list[i];
        for (k = 0; k < mod_iter->deviation_size; ++k) {
            if (mod_iter->deviation[k].target->module->type) {
                target_module = ((struct lys_submodule *)mod_iter->deviation[k].target->module)->belongsto;
            } else {
                target_module = mod_iter->deviation[k].target->module;
            }

            /* we found a module deviating our module */
            if (target_module == cur_mod) {
                cont = lyd_new(parent, NULL, "deviation");
                if (!cont) {
                    return EXIT_FAILURE;
                }

                if (!lyd_new_leaf_str(cont, NULL, "name", LY_TYPE_STRING, mod_iter->name)) {
                    return EXIT_FAILURE;
                }
                if (!lyd_new_leaf_str(cont, NULL, "revision", LY_TYPE_STRING,
                        (mod_iter->rev_size ? mod_iter->rev[0].date : ""))) {
                    return EXIT_FAILURE;
                }
            }
        }

        for (j = 0; j < mod_iter->inc_size; ++j) {
            for (k = 0; k < mod_iter->inc[j].submodule->deviation_size; ++k) {
                if (mod_iter->inc[j].submodule->deviation[k].target->module->type) {
                    target_module = ((struct lys_submodule *)
                                    mod_iter->inc[j].submodule->deviation[k].target->module)->belongsto;
                } else {
                    target_module = mod_iter->inc[j].submodule->deviation[k].target->module;
                }

                /* we found a submodule deviating our module */
                if (target_module == cur_mod) {
                    cont = lyd_new(parent, NULL, "deviation");
                    if (!cont) {
                        return EXIT_FAILURE;
                    }

                    if (!lyd_new_leaf_str(cont, NULL, "name", LY_TYPE_STRING, mod_iter->inc[j].submodule->name)) {
                        return EXIT_FAILURE;
                    }
                    if (!lyd_new_leaf_str(cont, NULL, "revision", LY_TYPE_STRING,
                                          (mod_iter->inc[j].submodule->rev_size ?
                                           mod_iter->inc[j].submodule->rev[0].date : ""))) {
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
ylib_submodules(struct lyd_node *parent, struct lys_module *cur_mod)
{
    int i;
    struct lyd_node *cont;

    for (i = 0; i < cur_mod->inc_size; ++i) {
        cont = lyd_new(parent, NULL, "submodule");
        if (!cont) {
            return EXIT_FAILURE;
        }

        if (!lyd_new_leaf_str(cont, NULL, "name", LY_TYPE_STRING, cur_mod->inc[i].submodule->name)) {
            return EXIT_FAILURE;
        }
        if (!lyd_new_leaf_str(cont, NULL, "revision", LY_TYPE_STRING, (cur_mod->inc[i].submodule->rev_size ?
                              cur_mod->inc[i].submodule->rev[0].date : ""))) {
            return EXIT_FAILURE;
        }
        if (cur_mod->inc[i].submodule->uri
                && !lyd_new_leaf_str(cont, NULL, "schema", LY_TYPE_STRING, cur_mod->inc[i].submodule->uri)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

API struct lyd_node *
ly_ctx_info(struct ly_ctx *ctx)
{
    int i;
    char id[8];
    struct lys_module *mod;
    struct lyd_node *root, *cont;

    mod = ly_ctx_get_module(ctx, "ietf-yang-library", NULL);
    if (!mod) {
        mod = lyp_search_file(ctx, NULL, "ietf-yang-library", NULL);
    }
    if (!mod || !mod->data || strcmp(mod->data->next->name, "modules")) {
        return NULL;
    }

    root = lyd_new(NULL, mod, "modules");
    if (!root) {
        return NULL;
    }

    for (i = 0; i < ctx->models.used; ++i) {
        cont = lyd_new(root, NULL, "module");
        if (!cont) {
            lyd_free(root);
            return NULL;
        }

        if (!lyd_new_leaf_str(cont, NULL, "name", LY_TYPE_STRING, ctx->models.list[i]->name)) {
            lyd_free(root);
            return NULL;
        }
        if (!lyd_new_leaf_str(cont, NULL, "revision", LY_TYPE_STRING, (ctx->models.list[i]->rev_size ?
                              ctx->models.list[i]->rev[0].date : ""))) {
            lyd_free(root);
            return NULL;
        }
        if (ctx->models.list[i]->uri
                && !lyd_new_leaf_str(cont, NULL, "schema", LY_TYPE_STRING, ctx->models.list[i]->uri)) {
            lyd_free(root);
            return NULL;
        }
        if (!lyd_new_leaf_str(cont, NULL, "namespace", LY_TYPE_STRING, ctx->models.list[i]->ns)) {
            lyd_free(root);
            return NULL;
        }
        if (ylib_feature(cont, ctx->models.list[i])) {
            lyd_free(root);
            return NULL;
        }
        if (ylib_deviation(cont, ctx->models.list[i], ctx)) {
            lyd_free(root);
            return NULL;
        }
        if (ctx->models.list[i]->implemented
                && !lyd_new_leaf_str(cont, NULL, "conformance", LY_TYPE_ENUM, "implement")) {
            lyd_free(root);
            return NULL;
        }
        if (!ctx->models.list[i]->implemented
                && !lyd_new_leaf_str(cont, NULL, "conformance", LY_TYPE_ENUM, "import")) {
            lyd_free(root);
            return NULL;
        }
        if (ylib_submodules(cont, ctx->models.list[i])) {
            lyd_free(root);
            return NULL;
        }
    }

    sprintf(id, "%u", ctx->models.module_set_id);
    if (!lyd_new_leaf_str(root, mod, "module-set-id", LY_TYPE_STRING, id)) {
        lyd_free(root);
        return NULL;
    }

    if (lyd_validate(root, 0)) {
        lyd_free(root);
        return NULL;
    }

    return root;
}
