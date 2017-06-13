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
#include "resolve.h"

/*
 * counter for references to the extensions plugins (for the number of contexts)
 * located in extensions.c
 */
extern unsigned int ext_plugins_ref;

#define IETF_YANG_METADATA_PATH "../models/ietf-yang-metadata@2016-08-05.h"
#define YANG_PATH "../models/yang@2017-02-20.h"
#define IETF_INET_TYPES_PATH "../models/ietf-inet-types@2013-07-15.h"
#define IETF_YANG_TYPES_PATH "../models/ietf-yang-types@2013-07-15.h"
#define IETF_YANG_LIB_PATH "../models/ietf-yang-library@2016-06-21.h"
#define IETF_YANG_LIB_REV "2016-06-21"

#include IETF_YANG_METADATA_PATH
#include YANG_PATH
#include IETF_INET_TYPES_PATH
#include IETF_YANG_TYPES_PATH
#include IETF_YANG_LIB_PATH

static struct internal_modules_s {
    const char *name;
    const char *revision;
    const char *data;
    uint8_t implemented;
    LYS_INFORMAT format;
} internal_modules[LY_INTERNAL_MODULE_COUNT] = {
    {"ietf-yang-metadata", "2016-08-05", (const char*)ietf_yang_metadata_2016_08_05_yin, 0, LYS_IN_YIN},
    {"yang", "2017-02-20", (const char*)yang_2017_02_20_yin, 1, LYS_IN_YIN},
    {"ietf-inet-types", "2013-07-15", (const char*)ietf_inet_types_2013_07_15_yin, 0, LYS_IN_YIN},
    {"ietf-yang-types", "2013-07-15", (const char*)ietf_yang_types_2013_07_15_yin, 0, LYS_IN_YIN},
    {"ietf-yang-library", "2016-06-21", (const char*)ietf_yang_library_2016_06_21_yin, 1, LYS_IN_YIN}
};

API struct ly_ctx *
ly_ctx_new(const char *search_dir)
{
    struct ly_ctx *ctx = NULL;
    struct lys_module *module;
    char *cwd = NULL;
    int i;

    ctx = calloc(1, sizeof *ctx);
    LY_CHECK_ERR_RETURN(!ctx, LOGMEM, NULL);

    /* dictionary */
    lydict_init(&ctx->dict);

    /* plugins */
    lyext_load_plugins();

    /* models list */
    ctx->models.list = calloc(16, sizeof *ctx->models.list);
    LY_CHECK_ERR_RETURN(!ctx->models.list, LOGMEM; free(ctx), NULL);
    ext_plugins_ref++;
    ctx->models.used = 0;
    ctx->models.size = 16;
    if (search_dir) {
        cwd = get_current_dir_name();
        if (chdir(search_dir)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            goto error;
        }
        ctx->models.search_paths = malloc(2 * sizeof *ctx->models.search_paths);
        LY_CHECK_ERR_GOTO(!ctx->models.search_paths, LOGMEM, error);
        ctx->models.search_paths[0] = get_current_dir_name();
        ctx->models.search_paths[1] = NULL;
        if (chdir(cwd)) {
            LOGWRN("Unable to return back to working directory \"%s\" (%s)",
                   cwd, strerror(errno));
        }
    }
    ctx->models.module_set_id = 1;

    /* load internal modules */
    for (i = 0; i < LY_INTERNAL_MODULE_COUNT; i++) {
        module = (struct lys_module *)lys_parse_mem(ctx, internal_modules[i].data, internal_modules[i].format);
        if (!module) {
            goto error;
        }
        module->implemented = internal_modules[i].implemented;
    }

    /* cleanup */
    free(cwd);

    return ctx;

error:
    free(cwd);
    ly_ctx_destroy(ctx, NULL);
    return NULL;
}

static struct ly_ctx *
ly_ctx_new_yl_common(const char *search_dir, const char *input, LYD_FORMAT format,
                     struct lyd_node* (*parser_func)(struct ly_ctx*, const char*, LYD_FORMAT, int,...))
{
    unsigned int u;
    struct lyd_node *module, *node;
    const char *name, *revision;
    struct ly_set features = {0, 0, {NULL}};
    const struct lys_module *mod;
    struct lyd_node *yltree = NULL;
    struct ly_ctx *ctx = NULL;

    /* create empty (with internal modules including ietf-yang-library) context */
    ctx = ly_ctx_new(search_dir);
    if (!ctx) {
        goto error;
    }

    /* parse yang library data tree */
    yltree = parser_func(ctx, input, format, LYD_OPT_DATA, NULL);
    if (!yltree) {
        goto error;
    }

    /* process the data tree */
    LY_TREE_FOR(yltree->child, module) {
        if (module->schema->nodetype == LYS_LEAF) {
            /* module-set-id - ignore it */
            continue;
        }

        /* initiate */
        name = NULL;
        revision = NULL;
        ly_set_clean(&features);

        LY_TREE_FOR(module->child, node) {
            if (!strcmp(node->schema->name, "name")) {
                name = ((struct lyd_node_leaf_list*)node)->value_str;
            } else if (!strcmp(node->schema->name, "revision")) {
                revision = ((struct lyd_node_leaf_list*)node)->value_str;
            } else if (!strcmp(node->schema->name, "feature")) {
                ly_set_add(&features, node, LY_SET_OPT_USEASLIST);
            } else if (!strcmp(node->schema->name, "conformance-type") &&
                    ((struct lyd_node_leaf_list*)node)->value.enm->value) {
                /* imported module - skip it, it will be loaded as a side effect
                 * of loading another module */
                goto next_module;
            }
        }

        /* use the gathered data to load the module */
        mod = ly_ctx_load_module(ctx, name, revision);
        if (!mod) {
            LOGERR(LY_EINVAL, "Unable to load module specified by yang library data.");
            goto error;
        }

        /* set features */
        for (u = 0; u < features.number; u++) {
            lys_features_enable(mod, ((struct lyd_node_leaf_list*)features.set.d[u])->value_str);
        }

next_module:;
    }

    if (0) {
        /* skip context destroy in case of success */
error:
        ly_ctx_destroy(ctx, NULL);
        ctx = NULL;
    }

    /* cleanup */
    if (yltree) {
        /* yang library data tree */
        lyd_free_withsiblings(yltree);
    }

    return ctx;
}

API struct ly_ctx *
ly_ctx_new_ylpath(const char *search_dir, const char *path, LYD_FORMAT format)
{
    return ly_ctx_new_yl_common(search_dir, path, format, lyd_parse_path);
}

API struct ly_ctx *
ly_ctx_new_ylmem(const char *search_dir, const char *data, LYD_FORMAT format)
{
    return ly_ctx_new_yl_common(search_dir, data, format, lyd_parse_mem);
}

API void
ly_ctx_set_allimplemented(struct ly_ctx *ctx)
{
    if (!ctx) {
        return;
    }

    ctx->models.flags |= LY_CTX_ALLIMPLEMENTED;
}

API void
ly_ctx_unset_allimplemented(struct ly_ctx *ctx)
{
    if (!ctx) {
        return;
    }

    ctx->models.flags &= ~LY_CTX_ALLIMPLEMENTED;
}

API void
ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir)
{
    char *cwd = NULL, *new;
    int index = 0;
    void *r;

    if (!ctx) {
        return;
    }

    if (search_dir) {
        cwd = get_current_dir_name();
        if (chdir(search_dir)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            goto cleanup;
        }

        new = get_current_dir_name();
        if (!ctx->models.search_paths) {
            ctx->models.search_paths = malloc(2 * sizeof *ctx->models.search_paths);
            LY_CHECK_ERR_GOTO(!ctx->models.search_paths, LOGMEM, cleanup);
            index = 0;
        } else {
            for (index = 0; ctx->models.search_paths[index]; index++) {
                /* check for duplicities */
                if (!strcmp(new, ctx->models.search_paths[index])) {
                    /* path is already present */
                    free(new);
                    goto success;
                }
            }
            r = realloc(ctx->models.search_paths, (index + 2) * sizeof *ctx->models.search_paths);
            LY_CHECK_ERR_GOTO(!r, LOGMEM, cleanup);
            ctx->models.search_paths = r;
        }
        ctx->models.search_paths[index] = new;
        ctx->models.search_paths[index + 1] = NULL;

success:
        if (chdir(cwd)) {
            LOGWRN("Unable to return back to working directory \"%s\" (%s)",
                   cwd, strerror(errno));
        }
    }

cleanup:
    free(cwd);
}

API const char * const *
ly_ctx_get_searchdirs(const struct ly_ctx *ctx)
{
    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }
    return (const char * const *)ctx->models.search_paths;
}

API void
ly_ctx_unset_searchdirs(struct ly_ctx *ctx)
{
    int i;

    if (!ctx->models.search_paths) {
        return;
    }

    for (i = 0; ctx->models.search_paths[i]; i++) {
        free(ctx->models.search_paths[i]);
    }
    free(ctx->models.search_paths);
    ctx->models.search_paths = NULL;
}

API void
ly_ctx_destroy(struct ly_ctx *ctx, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    if (!ctx) {
        return;
    }

    /* models list */
    for (; ctx->models.used > 0; ctx->models.used--) {
        /* remove the applied deviations and augments */
        lys_sub_module_remove_devs_augs(ctx->models.list[ctx->models.used - 1]);
        /* remove the module */
        lys_free(ctx->models.list[ctx->models.used - 1], private_destructor, 0);
    }
    if (ctx->models.search_paths) {
        for(i = 0; ctx->models.search_paths[i]; i++) {
            free(ctx->models.search_paths[i]);
        }
        free(ctx->models.search_paths);
    }
    free(ctx->models.list);

    /* dictionary */
    lydict_clean(&ctx->dict);

    /* plugins - will be removed only if this is the last context */
    ext_plugins_ref--;
    lyext_clean_plugins();

    /* clean the error list */
    ly_err_clean(0);

    free(ctx);
}

API const struct lys_submodule *
ly_ctx_get_submodule2(const struct lys_module *main_module, const char *submodule)
{
    const struct lys_submodule *result;
    int i;

    if (!main_module || !submodule) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    /* search in submodules list */
    for (i = 0; i < main_module->inc_size; i++) {
        result = main_module->inc[i].submodule;
        if (ly_strequal(submodule, result->name, 0)) {
            return result;
        }

        /* in YANG 1.1 all the submodules must be included in the main module, so we are done.
         * YANG 1.0 allows (is unclear about denying it) to include a submodule only in another submodule
         * but when libyang parses such a module it adds the include into the main module so we are also done.
         */
    }


    return NULL;
}

API const struct lys_submodule *
ly_ctx_get_submodule(const struct ly_ctx *ctx, const char *module, const char *revision, const char *submodule,
                     const char *sub_revision)
{
    const struct lys_module *mainmod;
    const struct lys_submodule *ret = NULL, *submod;
    uint32_t idx = 0;

    if (!ctx || !submodule || (revision && !module)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    while ((mainmod = ly_ctx_get_module_iter(ctx, &idx))) {
        if (module && strcmp(mainmod->name, module)) {
            /* main module name does not match */
            continue;
        }

        if (revision && (!mainmod->rev || strcmp(revision, mainmod->rev[0].date))) {
            /* main module revision does not match */
            continue;
        }

        submod = ly_ctx_get_submodule2(mainmod, submodule);
        if (!submod) {
            continue;
        }

        if (!sub_revision) {
            /* store only if newer */
            if (ret) {
                if (submod->rev && (!ret->rev || (strcmp(submod->rev[0].date, ret->rev[0].date) > 0))) {
                    ret = submod;
                }
            } else {
                ret = submod;
            }
        } else {
            /* store only if revision matches, we are done if it does */
            if (!submod->rev) {
                continue;
            } else if (!strcmp(sub_revision, submod->rev[0].date)) {
                ret = submod;
                break;
            }
        }
    }

    return ret;
}

static const struct lys_module *
ly_ctx_get_module_by(const struct ly_ctx *ctx, const char *key, int offset, const char *revision, int with_disabled)
{
    int i;
    struct lys_module *result = NULL;

    if (!ctx || !key) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    for (i = 0; i < ctx->models.used; i++) {
        if (!with_disabled && ctx->models.list[i]->disabled) {
            /* skip the disabled modules */
            continue;
        }
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
    return ly_ctx_get_module_by(ctx, ns, offsetof(struct lys_module, ns), revision, 0);
}

API const struct lys_module *
ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision)
{
    return ly_ctx_get_module_by(ctx, name, offsetof(struct lys_module, name), revision, 0);
}

API const struct lys_module *
ly_ctx_get_module_older(const struct ly_ctx *ctx, const struct lys_module *module)
{
    int i;
    const struct lys_module *result = NULL, *iter;

    if (!ctx || !module || !module->rev_size) {
        ly_errno = LY_EINVAL;
        return NULL;
    }


    for (i = 0; i < ctx->models.used; i++) {
        iter = ctx->models.list[i];
        if (iter->disabled) {
            /* skip the disabled modules */
            continue;
        }
        if (iter == module || !iter->rev_size) {
            /* iter is the module itself or iter has no revision */
            continue;
        }
        if (!ly_strequal(module->name, iter->name, 0)) {
            /* different module */
            continue;
        }
        if (strcmp(iter->rev[0].date, module->rev[0].date) < 0) {
            /* iter is older than module */
            if (result) {
                if (strcmp(iter->rev[0].date, result->rev[0].date) > 0) {
                    /* iter is newer than current result */
                    result = iter;
                }
            } else {
                result = iter;
            }
        }
    }

    return result;
}

API void
ly_ctx_set_module_imp_clb(struct ly_ctx *ctx, ly_module_imp_clb clb, void *user_data)
{
    if (!ctx) {
        ly_errno = LY_EINVAL;
        return;
    }

    ctx->imp_clb = clb;
    ctx->imp_clb_data = user_data;
}

API ly_module_imp_clb
ly_ctx_get_module_imp_clb(const struct ly_ctx *ctx, void **user_data)
{
    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (user_data) {
        *user_data = ctx->imp_clb_data;
    }
    return ctx->imp_clb;
}

API void
ly_ctx_set_module_data_clb(struct ly_ctx *ctx, ly_module_data_clb clb, void *user_data)
{
    if (!ctx) {
        ly_errno = LY_EINVAL;
        return;
    }

    ctx->data_clb = clb;
    ctx->data_clb_data = user_data;
}

API ly_module_data_clb
ly_ctx_get_module_data_clb(const struct ly_ctx *ctx, void **user_data)
{
    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (user_data) {
        *user_data = ctx->data_clb_data;
    }
    return ctx->data_clb;
}

const struct lys_module *
ly_ctx_load_sub_module(struct ly_ctx *ctx, struct lys_module *module, const char *name, const char *revision,
                       int implement, struct unres_schema *unres)
{
    const struct lys_module *mod;
    char *module_data;
    int i;
    void (*module_data_free)(void *module_data) = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;

    if (!module) {
        /* exception for internal modules */
        for (i = 0; i < LY_INTERNAL_MODULE_COUNT; i++) {
            if (ly_strequal(name, internal_modules[i].name, 0)) {
                if (!revision || ly_strequal(revision, internal_modules[i].revision, 0)) {
                    /* return internal module */
                    return (struct lys_module *)ly_ctx_get_module(ctx, name, revision);
                }
            }
        }
        if (revision) {
            /* try to get the schema with the specific revision from the context,
             * include the disabled modules in the search to avoid their duplication,
             * they are enabled by the subsequent call to lys_set_implemented() */
            for (i = LY_INTERNAL_MODULE_COUNT, mod = NULL; i < ctx->models.used; i++) {
                mod = ctx->models.list[i]; /* shortcut */
                if (ly_strequal(name, mod->name, 0) && mod->rev_size && !strcmp(revision, mod->rev[0].date)) {
                    break;
                }
                mod = NULL;
            }
            if (mod) {
                /* we get such a module, make it implemented */
                if (lys_set_implemented(mod)) {
                    /* the schema cannot be implemented */
                    mod = NULL;
                }
                return mod;
            }
        }
    }

    if (ctx->imp_clb) {
        if (module) {
            mod = lys_main_module(module);
            module_data = ctx->imp_clb(mod->name, (mod->rev_size ? mod->rev[0].date : NULL), name, revision, ctx->imp_clb_data, &format, &module_data_free);
        } else {
            module_data = ctx->imp_clb(name, revision, NULL, NULL, ctx->imp_clb_data, &format, &module_data_free);
        }
        if (!module_data) {
            if (module || revision) {
                /* we already know that the specified revision is not present in context, and we have no other
                 * option in case of submodules */
                LOGERR(LY_ESYS, "User module retrieval callback failed!");
                return NULL;
            } else {
                /* get the newest revision from the context */
                mod = ly_ctx_get_module_by(ctx, name, offsetof(struct lys_module, name), revision, 1);
                if (mod && mod->disabled) {
                    /* enable the required module */
                    lys_set_enabled(mod);
                }
                return mod;
            }
        }

        if (module) {
            mod = (struct lys_module *)lys_sub_parse_mem(module, module_data, format, unres);
        } else {
            mod = (struct lys_module *)lys_parse_mem_(ctx, module_data, format, 0, implement);
        }

        if (module_data_free) {
            module_data_free(module_data);
        }
    } else {
        mod = lyp_search_file(ctx, module, name, revision, implement, unres);
    }

    return mod;
}

API const struct lys_module *
ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision)
{
    if (!ctx || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return ly_ctx_load_sub_module(ctx, NULL, name, revision, 1, NULL);
}

/*
 * mods - set of removed modules, if NULL all modules are supposed to be removed so any backlink is invalid
 */
static void
ctx_modules_undo_backlinks(struct ly_ctx *ctx, struct ly_set *mods)
{
    int o;
    uint8_t j;
    unsigned int u, v;
    struct lys_module *mod;
    struct lys_node *elem, *next;
    struct lys_node_leaf *leaf;

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    for (o = LY_INTERNAL_MODULE_COUNT - 1; o < ctx->models.used; o++) {
        mod = ctx->models.list[o]; /* shortcut */

        /* 1) features */
        for (j = 0; j < mod->features_size; j++) {
            if (!mod->features[j].depfeatures) {
                continue;
            }
            for (v = 0; v < mod->features[j].depfeatures->number; v++) {
                if (!mods || ly_set_contains(mods, ((struct lys_feature *)mod->features[j].depfeatures->set.g[v])->module) != -1) {
                    /* depending feature is in module to remove */
                    ly_set_rm_index(mod->features[j].depfeatures, v);
                    v--;
                }
            }
            if (!mod->features[j].depfeatures->number) {
                /* all backlinks removed */
                ly_set_free(mod->features[j].depfeatures);
                mod->features[j].depfeatures = NULL;
            }
        }

        /* 2) identities */
        for (u = 0; u < mod->ident_size; u++) {
            if (!mod->ident[u].der) {
                continue;
            }
            for (v = 0; v < mod->ident[u].der->number; v++) {
                if (!mods || ly_set_contains(mods, ((struct lys_ident *)mod->ident[u].der->set.g[v])->module) != -1) {
                    /* derived identity is in module to remove */
                    ly_set_rm_index(mod->ident[u].der, v);
                    v--;
                }
            }
            if (!mod->ident[u].der->number) {
                /* all backlinks removed */
                ly_set_free(mod->ident[u].der);
                mod->ident[u].der = NULL;
            }
        }

        /* 3) leafrefs */
        for (elem = next = mod->data; elem; elem = next) {
            if (elem->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                leaf = (struct lys_node_leaf *)elem; /* shortcut */
                if (leaf->backlinks) {
                    if (!mods) {
                        /* remove all backlinks */
                        ly_set_free(leaf->backlinks);
                        leaf->backlinks = NULL;
                    } else {
                        for (v = 0; v < leaf->backlinks->number; v++) {
                            if (ly_set_contains(mods, leaf->backlinks->set.s[v]->module) != -1) {
                                /* derived identity is in module to remove */
                                ly_set_rm_index(leaf->backlinks, v);
                                v--;
                            }
                        }
                        if (!leaf->backlinks->number) {
                            /* all backlinks removed */
                            ly_set_free(leaf->backlinks);
                            leaf->backlinks = NULL;
                        }
                    }
                }
            }

            /* select next element to process */
            next = elem->child;
            /* child exception for leafs, leaflists, anyxml and groupings */
            if (elem->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA | LYS_GROUPING)) {
                next = NULL;
            }
            if (!next) {
                /* no children,  try siblings */
                next = elem->next;
            }
            while (!next) {
                /* parent is already processed, go to its sibling */
                elem = lys_parent(elem);
                if (!elem) {
                    /* we are done, no next element to process */
                    break;
                }
                /* no siblings, go back through parents */
                next = elem->next;
            }
        }
    }
}

static int
ctx_modules_redo_backlinks(struct ly_set *mods)
{
    unsigned int i, j, k, s;
    struct lys_module *mod;
    struct lys_node *next, *elem;
    struct lys_type *type;
    struct lys_feature *feat;

    for (i = 0; i < mods->number; ++i) {
        mod = (struct lys_module *)mods->set.g[i]; /* shortcut */

        /* identities */
        if (mod->implemented) {
            for (j = 0; j < mod->ident_size; j++) {
                for (k = 0; k < mod->ident[j].base_size; k++) {
                    resolve_identity_backlink_update(&mod->ident[j], mod->ident[j].base[k]);
                }
            }
        }

        /* features */
        for (j = 0; j < mod->features_size; j++) {
            for (k = 0; k < mod->features[j].iffeature_size; k++) {
                resolve_iffeature_getsizes(&mod->features[j].iffeature[k], NULL, &s);
                while (s--) {
                    feat = mod->features[j].iffeature[k].features[s]; /* shortcut */
                    if (!feat->depfeatures) {
                        feat->depfeatures = ly_set_new();
                    }
                    ly_set_add(feat->depfeatures, &mod->features[j], LY_SET_OPT_USEASLIST);
                }
            }
        }

        /* leafrefs */
        LY_TREE_DFS_BEGIN(mod->data, next, elem) {
            if (elem->nodetype == LYS_GROUPING) {
                goto next_sibling;
            }

            if (elem->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                type = &((struct lys_node_leaf *)elem)->type; /* shortcut */
                if (type->base == LY_TYPE_LEAFREF) {
                    lys_leaf_add_leafref_target(type->info.lref.target, elem);
                }
            }

            /* select element for the next run - children first */
            next = elem->child;

            /* child exception for leafs, leaflists and anyxml without children */
            if (elem->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                next = NULL;
            }
            if (!next) {
next_sibling:
                /* no children */
                if (elem == mod->data) {
                    /* we are done, (START) has no children */
                    break;
                }
                /* try siblings */
                next = elem->next;
            }
            while (!next) {
                /* parent is already processed, go to its sibling */
                elem = lys_parent(elem);

                /* no siblings, go back through parents */
                if (lys_parent(elem) == lys_parent(mod->data)) {
                    /* we are done, no next element to process */
                    break;
                }
                next = elem->next;
            }
        }
    }

    return 0;
}

API int
lys_set_disabled(const struct lys_module *module)
{
    struct ly_ctx *ctx; /* shortcut */
    struct lys_module *mod;
    struct ly_set *mods;
    uint8_t j, imported;
    int i, o;
    unsigned int u, v;

    if (!module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    } else if (module->disabled) {
        /* already disabled module */
        return EXIT_SUCCESS;
    }
    mod = (struct lys_module *)module;
    ctx = mod->ctx;

    /* avoid disabling internal modules */
    for (i = 0; i < LY_INTERNAL_MODULE_COUNT; i++) {
        if (mod == ctx->models.list[i]) {
            LOGERR(LY_EINVAL, "Internal module \"%s\" cannot be disabled.", mod->name);
            return EXIT_FAILURE;
        }
    }

    /* disable the module */
    mod->disabled = 1;

    /* get the complete list of modules to disable because of dependencies,
     * we are going also to disable all the imported (not implemented) modules
     * that are not used in any other module */
    mods = ly_set_new();
    ly_set_add(mods, mod, 0);
checkdependency:
    for (i = LY_INTERNAL_MODULE_COUNT; i < ctx->models.used; i++) {
        mod = ctx->models.list[i]; /* shortcut */
        if (mod->disabled) {
            /* skip the already disabled modules */
            continue;
        }

        /* check depndency of imported modules */
        for (j = 0; j < mod->imp_size; j++) {
            for (u = 0; u < mods->number; u++) {
                if (mod->imp[j].module == mods->set.g[u]) {
                    /* module is importing some module to disable, so it must be also disabled */
                    mod->disabled = 1;
                    ly_set_add(mods, mod, 0);
                    /* we have to start again because some of the already checked modules can
                     * depend on the one we have just decided to disable */
                    goto checkdependency;
                }
            }
        }
        /* check if the imported module is used in any module supposed to be kept */
        if (!mod->implemented) {
            imported = 0;
            for (o = LY_INTERNAL_MODULE_COUNT; o < ctx->models.used; o++) {
                if (ctx->models.list[o]->disabled) {
                    /* skip modules already disabled */
                    continue;
                }
                for (j = 0; j < ctx->models.list[o]->imp_size; j++) {
                    if (ctx->models.list[o]->imp[j].module == mod) {
                        /* the module is used in some other module not yet selected to be disabled */
                        imported = 1;
                        goto imported;
                    }
                }
            }
imported:
            if (!imported) {
                /* module is not implemented and neither imported by any other module in context
                 * which is supposed to be kept enabled after this operation, so we are going to disable also
                 * this module */
                mod->disabled = 1;
                ly_set_add(mods, mod, 0);
                /* we have to start again, this time not because other module can depend on this one
                 * (we know that there is no such module), but because the module can import module
                 * that could became useless. If there are no imports, we can continue */
                if (mod->imp_size) {
                    goto checkdependency;
                }
            }
        }
    }

    /* before removing applied deviations, augments and updating leafrefs, we have to enable the modules
     * to disable to allow all that operations */
    for (u = 0; u < mods->number; u++) {
        ((struct lys_module *)mods->set.g[u])->disabled = 0;
    }

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    ctx_modules_undo_backlinks(ctx, mods);

    /* remove the applied deviations and augments */
    for (u = 0; u < mods->number; u++) {
        lys_sub_module_remove_devs_augs((struct lys_module *)mods->set.g[u]);
    }

    /* now again disable the modules to disable and disable also all its submodules */
    for (u = 0; u < mods->number; u++) {
        mod = (struct lys_module *)mods->set.g[u];
        mod->disabled = 1;
        for (v = 0; v < mod->inc_size; v++) {
            mod->inc[v].submodule->disabled = 1;
        }
    }

    /* free the set */
    ly_set_free(mods);

    /* update the module-set-id */
    ctx->models.module_set_id++;

    return EXIT_SUCCESS;
}

static void
lys_set_enabled_(struct ly_set *mods, struct lys_module *mod)
{
    unsigned int i;

    ly_set_add(mods, mod, 0);
    mod->disabled = 0;

    for (i = 0; i < mod->inc_size; i++) {
        mod->inc[i].submodule->disabled = 0;
    }

    /* go recursively */
    for (i = 0; i < mod->imp_size; i++) {
        if (!mod->imp[i].module->disabled) {
            continue;
        }

        lys_set_enabled_(mods, mod->imp[i].module);
    }
}

API int
lys_set_enabled(const struct lys_module *module)
{
    struct ly_ctx *ctx; /* shortcut */
    struct lys_module *mod;
    struct ly_set *mods, *disabled;
    int i;
    unsigned int u, v, w;

    if (!module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    } else if (!module->disabled) {
        /* already enabled module */
        return EXIT_SUCCESS;
    }
    mod = (struct lys_module *)module;
    ctx = mod->ctx;

    /* avoid disabling internal modules */
    for (i = 0; i < LY_INTERNAL_MODULE_COUNT; i++) {
        if (mod == ctx->models.list[i]) {
            LOGERR(LY_EINVAL, "Internal module \"%s\" cannot be removed.", mod->name);
            return EXIT_FAILURE;
        }
    }

    mods = ly_set_new();
    disabled = ly_set_new();

    /* enable the module, including its dependencies */
    lys_set_enabled_(mods, mod);

    /* we will go through the all disabled modules in the context, if the module has no dependency (import)
     * that is still disabled AND at least one of its imported module is from the set we are enabling now,
     * it is going to be also enabled. This way we try to revert everething that was possibly done by
     * lys_set_disabled(). */
checkdependency:
    for (i = LY_INTERNAL_MODULE_COUNT; i < ctx->models.used; i++) {
        mod = ctx->models.list[i]; /* shortcut */
        if (!mod->disabled || ly_set_contains(disabled, mod) != -1) {
            /* skip the enabled modules */
            continue;
        }

        /* check imported modules */
        for (u = 0; u < mod->imp_size; u++) {
            if (mod->imp[u].module->disabled) {
                /* it has disabled dependency so it must stay disabled */
                break;
            }
        }
        if (u < mod->imp_size) {
            /* it has disabled dependency, continue with the next module in the context */
            continue;
        }

        /* get know if at least one of the imported modules is being enabled this time */
        for (u = 0; u < mod->imp_size; u++) {
            for (v = 0; v < mods->number; v++) {
                if (mod->imp[u].module == mods->set.g[v]) {
                    /* yes, it is, so they are connected and we are going to enable it as well,
                     * it is not necessary to call recursive lys_set_enable_() because we already
                     * know that there is no disabled import to enable */
                    mod->disabled = 0;
                    ly_set_add(mods, mod, 0);
                    for (w = 0; w < mod->inc_size; w++) {
                        mod->inc[w].submodule->disabled = 0;
                    }
                    /* we have to start again because some of the already checked modules can
                     * depend on the one we have just decided to enable */
                    goto checkdependency;
                }
            }
        }

        /* this module is disabled, but it does not depend on any other disabled module and none
         * of its imports was not enabled in this call. No future enabling of the disabled module
         * will change this so we can remember the module and skip it next time we will have to go
         * through the all context because of the checkdependency goto.
         */
        ly_set_add(disabled, mod, 0);
    }

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    ctx_modules_redo_backlinks(mods);

    /* re-apply the deviations and augments */
    for (v = 0; v < mods->number; v++) {
        lys_sub_module_apply_devs_augs((struct lys_module *)mods->set.g[v]);
    }

    /* free the sets */
    ly_set_free(mods);
    ly_set_free(disabled);

    /* update the module-set-id */
    ctx->models.module_set_id++;

    return EXIT_SUCCESS;
}

API int
ly_ctx_remove_module(const struct lys_module *module,
                     void (*private_destructor)(const struct lys_node *node, void *priv))
{
    struct ly_ctx *ctx; /* shortcut */
    struct lys_module *mod = NULL;
    struct ly_set *mods;
    uint8_t j, imported;
    int i, o;
    unsigned int u;

    if (!module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    mod = (struct lys_module *)module;
    ctx = mod->ctx;

    /* avoid removing internal modules ... */
    for (i = 0; i < LY_INTERNAL_MODULE_COUNT; i++) {
        if (mod == ctx->models.list[i]) {
            LOGERR(LY_EINVAL, "Internal module \"%s\" cannot be removed.", mod->name);
            return EXIT_FAILURE;
        }
    }
    /* ... and hide the module from the further processing of the context modules list */
    for (i = LY_INTERNAL_MODULE_COUNT; i < ctx->models.used; i++) {
        if (mod == ctx->models.list[i]) {
            ctx->models.list[i] = NULL;
            break;
        }
    }

    /* get the complete list of modules to remove because of dependencies,
     * we are going also to remove all the imported (not implemented) modules
     * that are not used in any other module */
    mods = ly_set_new();
    ly_set_add(mods, mod, 0);
checkdependency:
    for (i = LY_INTERNAL_MODULE_COUNT; i < ctx->models.used; i++) {
        mod = ctx->models.list[i]; /* shortcut */
        if (!mod) {
            /* skip modules already selected for removing */
            continue;
        }

        /* check depndency of imported modules */
        for (j = 0; j < mod->imp_size; j++) {
            for (u = 0; u < mods->number; u++) {
                if (mod->imp[j].module == mods->set.g[u]) {
                    /* module is importing some module to remove, so it must be also removed */
                    ly_set_add(mods, mod, 0);
                    ctx->models.list[i] = NULL;
                    /* we have to start again because some of the already checked modules can
                     * depend on the one we have just decided to remove */
                    goto checkdependency;
                }
            }
        }
        /* check if the imported module is used in any module supposed to be kept */
        if (!mod->implemented) {
            imported = 0;
            for (o = LY_INTERNAL_MODULE_COUNT; o < ctx->models.used; o++) {
                if (!ctx->models.list[o]) {
                    /* skip modules already selected for removing */
                    continue;
                }
                for (j = 0; j < ctx->models.list[o]->imp_size; j++) {
                    if (ctx->models.list[o]->imp[j].module == mod) {
                        /* the module is used in some other module not yet selected to be deleted */
                        imported = 1;
                        goto imported;
                    }
                }
            }
imported:
            if (!imported) {
                /* module is not implemented and neither imported by any other module in context
                 * which is supposed to be kept after this operation, so we are going to remove also
                 * this useless module */
                ly_set_add(mods, mod, 0);
                ctx->models.list[i] = NULL;
                /* we have to start again, this time not because other module can depend on this one
                 * (we know that there is no such module), but because the module can import module
                 * that could became useless. If there are no imports, we can continue */
                if (mod->imp_size) {
                    goto checkdependency;
                }
            }
        }
    }


    /* consolidate the modules list */
    for (i = o = LY_INTERNAL_MODULE_COUNT; i < ctx->models.used; i++) {
        if (ctx->models.list[o]) {
            /* used cell */
            o++;
        } else {
            /* the current output cell is empty, move here an input cell */
            ctx->models.list[o] = ctx->models.list[i];
            ctx->models.list[i] = NULL;
        }
    }
    /* get the last used cell to get know the number of used */
    while (!ctx->models.list[o]) {
        o--;
    }
    ctx->models.used = o + 1;
    ctx->models.module_set_id++;

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    ctx_modules_undo_backlinks(ctx, mods);

    /* free the modules */
    for (u = 0; u < mods->number; u++) {
        /* remove the applied deviations and augments */
        lys_sub_module_remove_devs_augs((struct lys_module *)mods->set.g[u]);
        /* remove the module */
        lys_free((struct lys_module *)mods->set.g[u], private_destructor, 0);
    }
    ly_set_free(mods);

    return EXIT_SUCCESS;
}

API void
ly_ctx_clean(struct ly_ctx *ctx, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    if (!ctx) {
        return;
    }

    /* models list */
    for (; ctx->models.used > LY_INTERNAL_MODULE_COUNT; ctx->models.used--) {
        /* remove the applied deviations and augments */
        lys_sub_module_remove_devs_augs(ctx->models.list[ctx->models.used - 1]);
        /* remove the module */
        lys_free(ctx->models.list[ctx->models.used - 1], private_destructor, 0);
        /* clean it for safer future use */
        ctx->models.list[ctx->models.used - 1] = NULL;
    }
    ctx->models.module_set_id++;

    /* maintain backlinks (actually done only with ietf-yang-library since its leafs can be target of leafref) */
    ctx_modules_undo_backlinks(ctx, NULL);
}

API const struct lys_module *
ly_ctx_get_module_iter(const struct ly_ctx *ctx, uint32_t *idx)
{
    if (!ctx || !idx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    for ( ; *idx < (unsigned)ctx->models.used; (*idx)++) {
        if (!ctx->models.list[(*idx)]->disabled) {
            return ctx->models.list[(*idx)++];
        }
    }

    return NULL;
}

API const struct lys_module *
ly_ctx_get_disabled_module_iter(const struct ly_ctx *ctx, uint32_t *idx)
{
    if (!ctx || !idx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    for ( ; *idx < (unsigned)ctx->models.used; (*idx)++) {
        if (ctx->models.list[(*idx)]->disabled) {
            return ctx->models.list[(*idx)++];
        }
    }

    return NULL;
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
    uint32_t i = 0, j;
    const struct lys_module *mod;
    struct lyd_node *cont;
    const char *ptr;

    if (cur_mod->deviated) {
        while ((mod = ly_ctx_get_module_iter(cur_mod->ctx, &i))) {
            if (mod == cur_mod) {
                continue;
            }

            for (j = 0; j < mod->deviation_size; ++j) {
                ptr = strstr(mod->deviation[j].target_name, cur_mod->name);
                if (ptr && ptr[strlen(cur_mod->name)] == ':') {
                    cont = lyd_new(parent, NULL, "deviation");
                    if (!cont) {
                        return EXIT_FAILURE;
                    }

                    if (!lyd_new_leaf(cont, NULL, "name", mod->name)) {
                        return EXIT_FAILURE;
                    }
                    if (!lyd_new_leaf(cont, NULL, "revision", (mod->rev_size ? mod->rev[0].date : ""))) {
                        return EXIT_FAILURE;
                    }

                    break;
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
    char *str;
    struct lyd_node *item;

    for (i = 0; i < cur_mod->inc_size && cur_mod->inc[i].submodule; ++i) {
        item = lyd_new(parent, NULL, "submodule");
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
        if (cur_mod->inc[i].submodule->filepath) {
            if (asprintf(&str, "file://%s", cur_mod->inc[i].submodule->filepath) == -1) {
                LOGMEM;
                return EXIT_FAILURE;
            } else if (!lyd_new_leaf(item, NULL, "schema", str)) {
                free(str);
                return EXIT_FAILURE;
            }
            free(str);
        }
    }

    return EXIT_SUCCESS;
}

API struct lyd_node *
ly_ctx_info(struct ly_ctx *ctx)
{
    int i;
    char id[8];
    char *str;
    const struct lys_module *mod;
    struct lyd_node *root, *cont;

    if (!ctx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

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
        if (ctx->models.list[i]->disabled) {
            /* skip the disabled modules */
            continue;
        }

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
        if (ctx->models.list[i]->filepath) {
            if (asprintf(&str, "file://%s", ctx->models.list[i]->filepath) == -1) {
                LOGMEM;
                lyd_free(root);
                return NULL;
            } else if (!lyd_new_leaf(cont, NULL, "schema", str)) {
                free(str);
                lyd_free(root);
                return NULL;
            }
            free(str);
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

    if (lyd_validate(&root, LYD_OPT_NOSIBLINGS, NULL)) {
        lyd_free(root);
        return NULL;
    }

    return root;
}

API const struct lys_node *
ly_ctx_get_node(struct ly_ctx *ctx, const struct lys_node *start, const char *nodeid)
{
    const struct lys_node *node;

    if (!ctx || !nodeid || ((nodeid[0] != '/') && !start)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    /* sets error and everything */
    node = resolve_json_nodeid(nodeid, ctx, start);

    return node;
}
