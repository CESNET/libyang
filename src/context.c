/**
 * @file context.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief context implementation for libyang
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
#include "dict_private.h"
#include "parser.h"
#include "tree_internal.h"

#define YANG_FAKEMODULE_PATH "../models/yang@2016-02-11.h"
#define IETF_INET_TYPES_PATH "../models/ietf-inet-types@2013-07-15.h"
#define IETF_YANG_TYPES_PATH "../models/ietf-yang-types@2013-07-15.h"
#define IETF_YANG_LIB_PATH "../models/ietf-yang-library@2016-02-01.h"
#define IETF_YANG_LIB_REV "2016-02-01"

#include YANG_FAKEMODULE_PATH
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
    if (!ctx->models.list) {
        LOGMEM;
        free(ctx);
        return NULL;
    }
    ctx->models.used = 0;
    ctx->models.size = 16;
    if (search_dir) {
        cwd = get_current_dir_name();
        if (chdir(search_dir)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            free(cwd);
            ly_ctx_destroy(ctx, NULL);
            return NULL;
        }
        ctx->models.search_path = get_current_dir_name();
        chdir(cwd);
        free(cwd);
    }
    ctx->models.module_set_id = 1;

    /* load (fake) YANG module */
    if (!lys_parse_mem(ctx, (char *)yang_2016_02_11_yin, LYS_IN_YIN)) {
        ly_ctx_destroy(ctx, NULL);
        return NULL;
    }

    /* load ietf-inet-types */
    if (!lys_parse_mem(ctx, (char *)ietf_inet_types_2013_07_15_yin, LYS_IN_YIN)) {
        ly_ctx_destroy(ctx, NULL);
        return NULL;
    }

    /* load ietf-yang-types */
    if (!lys_parse_mem(ctx, (char *)ietf_yang_types_2013_07_15_yin, LYS_IN_YIN)) {
        ly_ctx_destroy(ctx, NULL);
        return NULL;
    }

    /* load ietf-yang-library */
    if (!lys_parse_mem(ctx, (char *)ietf_yang_library_2016_02_01_yin, LYS_IN_YIN)) {
        ly_ctx_destroy(ctx, NULL);
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
        free(ctx->models.search_path);
        ctx->models.search_path = get_current_dir_name();

        chdir(cwd);
        free(cwd);
    } else {
        free(ctx->models.search_path);
        ctx->models.search_path = NULL;
    }
}

API const char *
ly_ctx_get_searchdir(const struct ly_ctx *ctx)
{
    return ctx->models.search_path;
}

API void
ly_ctx_destroy(struct ly_ctx *ctx, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    if (!ctx) {
        return;
    }

    /* models list */
    for (i = 0; i < ctx->models.used; ++i) {
        lys_free(ctx->models.list[i], private_destructor, 0);
    }
    free(ctx->models.search_path);
    free(ctx->models.list);

    /* dictionary */
    lydict_clean(&ctx->dict);

    free(ctx);
}

API const struct lys_submodule *
ly_ctx_get_submodule(const struct lys_module *module, const char *name, const char *revision)
{
    struct lys_submodule *result;
    int i;

    if (!module || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    /* make sure that the provided module is not submodule */
    module = lys_module(module);

    /* search in submodules list */
    for (i = 0; i < module->inc_size; i++) {
        result = module->inc[i].submodule;
        if (!result || strcmp(name, result->name)) {
            continue;
        }

        if (!revision || (result->rev_size && !strcmp(revision, result->rev[0].date))) {
            return result;
        }
    }

    return NULL;
}

static const struct lys_module *
ly_ctx_get_module_by(const struct ly_ctx *ctx, const char *key, int offset, const char *revision)
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

API const struct lys_module *
ly_ctx_get_module_by_ns(const struct ly_ctx *ctx, const char *ns, const char *revision)
{
    return ly_ctx_get_module_by(ctx, ns, offsetof(struct lys_module, ns), revision);
}

API const struct lys_module *
ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision)
{
    return ly_ctx_get_module_by(ctx, name, offsetof(struct lys_module, name), revision);
}

API void
ly_ctx_set_module_clb(struct ly_ctx *ctx, ly_module_clb clb, void *user_data)
{
    ctx->module_clb = clb;
    ctx->module_clb_data = user_data;
}

API ly_module_clb
ly_ctx_get_module_clb(const struct ly_ctx *ctx, void **user_data)
{
    if (user_data) {
        *user_data = ctx->module_clb_data;
    }
    return ctx->module_clb;
}

API const struct lys_module *
ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision)
{
    const struct lys_module *module;
    char *module_data;
    void (*module_data_free)(char *module_data) = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;

    if (!ctx || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (ctx->module_clb) {
        module_data = ctx->module_clb(name, revision, ctx->module_clb_data, &format, &module_data_free);
        if (!module_data) {
            LOGERR(LY_EVALID, "User module retrieval callback failed!");
            return NULL;
        }
        module = lys_parse_mem(ctx, module_data, format);
        if (module_data_free) {
            module_data_free(module_data);
        } else {
            free(module_data);
        }
    } else {
        module = lyp_search_file(ctx, NULL, name, revision, NULL);
    }

    return module;
}

API const char **
ly_ctx_get_module_names(const struct ly_ctx *ctx)
{
    int i;
    const char **result = NULL;

    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    result = malloc((ctx->models.used+1) * sizeof *result);
    if (!result) {
        LOGMEM;
        return NULL;
    }

    for (i = 0; i < ctx->models.used; i++) {
        result[i] = ctx->models.list[i]->name;
    }
    result[i] = NULL;

    return result;
}

API const char **
ly_ctx_get_submodule_names(const struct ly_ctx *ctx, const char *module_name)
{
    int i;
    const char **result = NULL;
    const struct lys_module *mod;

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
    if (!result) {
        LOGMEM;
        return NULL;
    }

    for (i = 0; i < mod->inc_size && mod->inc[i].submodule; i++) {
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

        if (!lyd_new_leaf(parent, NULL, "feature", cur_mod->features[i].name)) {
            return EXIT_FAILURE;
        }
    }

    /* submodule features */
    for (i = 0; i < cur_mod->inc_size && cur_mod->inc[i].submodule; ++i) {
        for (j = 0; j < cur_mod->inc[i].submodule->features_size; ++j) {
            if (!(cur_mod->inc[i].submodule->features[j].flags & LYS_FENABLED)) {
                continue;
            }

            if (!lyd_new_leaf(parent, NULL, "feature", cur_mod->inc[i].submodule->features[j].name)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
ylib_deviation(struct lyd_node *parent, struct lys_module *cur_mod)
{
    int i;
    const char *revision;
    struct lyd_node *cont;

    for (i = 0; i < cur_mod->imp_size; ++i) {
        /* marks a deviating module */
        if (cur_mod->imp[i].external == 2) {
            revision = (cur_mod->imp[i].module->rev_size ? cur_mod->imp[i].module->rev[0].date : "");

            cont = lyd_new(parent, NULL, "deviation");
            if (!cont) {
                return EXIT_FAILURE;
            }

            if (!lyd_new_leaf(cont, NULL, "name", cur_mod->imp[i].module->name)) {
                return EXIT_FAILURE;
            }
            if (!lyd_new_leaf(cont, NULL, "revision", revision)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
ylib_submodules(struct lyd_node *parent, struct lys_module *cur_mod)
{
    int i;
    struct lyd_node *cont, *item;

    if (cur_mod->inc_size) {
        cont = lyd_new(parent, NULL, "submodules");
    }

    for (i = 0; i < cur_mod->inc_size && cur_mod->inc[i].submodule; ++i) {
        item = lyd_new(cont, NULL, "submodule");
        if (!item) {
            return EXIT_FAILURE;
        }

        if (!lyd_new_leaf(item, NULL, "name", cur_mod->inc[i].submodule->name)) {
            return EXIT_FAILURE;
        }
        if (!lyd_new_leaf(item, NULL, "revision", (cur_mod->inc[i].submodule->rev_size ?
                          cur_mod->inc[i].submodule->rev[0].date : ""))) {
            return EXIT_FAILURE;
        }
        if (cur_mod->inc[i].submodule->uri
                && !lyd_new_leaf(item, NULL, "schema", cur_mod->inc[i].submodule->uri)) {
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
    const struct lys_module *mod;
    struct lyd_node *root, *cont;

    mod = ly_ctx_get_module(ctx, "ietf-yang-library", IETF_YANG_LIB_REV);
    if (!mod || !mod->data) {
        LOGINT;
        return NULL;
    }

    root = lyd_new(NULL, mod, "modules-state");
    if (!root) {
        return NULL;
    }

    for (i = 0; i < ctx->models.used; ++i) {
        cont = lyd_new(root, NULL, "module");
        if (!cont) {
            lyd_free(root);
            return NULL;
        }

        if (!lyd_new_leaf(cont, NULL, "name", ctx->models.list[i]->name)) {
            lyd_free(root);
            return NULL;
        }
        if (!lyd_new_leaf(cont, NULL, "revision", (ctx->models.list[i]->rev_size ?
                              ctx->models.list[i]->rev[0].date : ""))) {
            lyd_free(root);
            return NULL;
        }
        if (ctx->models.list[i]->uri
                && !lyd_new_leaf(cont, NULL, "schema", ctx->models.list[i]->uri)) {
            lyd_free(root);
            return NULL;
        }
        if (!lyd_new_leaf(cont, NULL, "namespace", ctx->models.list[i]->ns)) {
            lyd_free(root);
            return NULL;
        }
        if (ylib_feature(cont, ctx->models.list[i])) {
            lyd_free(root);
            return NULL;
        }
        if (ylib_deviation(cont, ctx->models.list[i])) {
            lyd_free(root);
            return NULL;
        }
        if (ctx->models.list[i]->implemented
                && !lyd_new_leaf(cont, NULL, "conformance-type", "implement")) {
            lyd_free(root);
            return NULL;
        }
        if (!ctx->models.list[i]->implemented
                && !lyd_new_leaf(cont, NULL, "conformance-type", "import")) {
            lyd_free(root);
            return NULL;
        }
        if (ylib_submodules(cont, ctx->models.list[i])) {
            lyd_free(root);
            return NULL;
        }
    }

    sprintf(id, "%u", ctx->models.module_set_id);
    if (!lyd_new_leaf(root, mod, "module-set-id", id)) {
        lyd_free(root);
        return NULL;
    }

    if (lyd_validate(root, 0)) {
        lyd_free(root);
        return NULL;
    }

    return root;
}

API const struct lys_node *
ly_ctx_get_node(struct ly_ctx *ctx, const char *nodeid)
{
    const struct lys_node *ret;

    if (!ctx || !nodeid) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (resolve_json_absolute_schema_nodeid(nodeid, ctx, &ret)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return ret;
}
