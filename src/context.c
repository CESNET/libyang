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
#define IETF_YANG_LIB_PATH "../models/ietf-yang-library@2016-06-21.h"
#define IETF_YANG_LIB_REV "2016-06-21"

#include YANG_FAKEMODULE_PATH
#include IETF_INET_TYPES_PATH
#include IETF_YANG_TYPES_PATH
#include IETF_YANG_LIB_PATH

#define INTERNAL_MODULES_COUNT 4
static struct internal_modules_s {
    const char *name;
    const char *revision;
    const char *data;
    uint8_t implemented;
    LYS_INFORMAT format;
} internal_modules[INTERNAL_MODULES_COUNT] = {
    {"yang", "2016-02-11", (const char*)yang_2016_02_11_yin, 1, LYS_IN_YIN},
    {"ietf-inet-types", "2013-07-15", (const char*)ietf_inet_types_2013_07_15_yin, 0, LYS_IN_YIN},
    {"ietf-yang-types", "2013-07-15", (const char*)ietf_yang_types_2013_07_15_yin, 0, LYS_IN_YIN},
    {"ietf-yang-library", "2016-06-21", (const char*)ietf_yang_library_2016_06_21_yin, 1, LYS_IN_YIN}
};

API struct ly_ctx *
ly_ctx_new(const char *search_dir)
{
    struct ly_ctx *ctx;
    struct lys_module *module;
    char *cwd;
    int i;

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
        if (chdir(cwd)) {
            LOGWRN("Unable to return back to working directory \"%s\" (%s)",
                   cwd, strerror(errno));
        }
        free(cwd);
    }
    ctx->models.module_set_id = 1;

    /* load internal modules */
    for (i = 0; i < INTERNAL_MODULES_COUNT; i++) {
        module = (struct lys_module *)lys_parse_mem(ctx, internal_modules[i].data, internal_modules[i].format);
        if (!module) {
            ly_ctx_destroy(ctx, NULL);
            return NULL;
        }
        module->implemented = internal_modules[i].implemented;
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

        if (chdir(cwd)) {
            LOGWRN("Unable to return back to working directory \"%s\" (%s)",
                   cwd, strerror(errno));
        }
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

    /* clean the error list */
    ly_err_clean();

    free(ctx);
}

API const struct lys_submodule *
ly_ctx_get_submodule2(const struct lys_module *main_module, const char *submodule)
{
    struct lys_submodule *result;
    int i;

    if (!main_module || !submodule) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    /* search in submodules list */
    for (i = 0; i < main_module->inc_size; i++) {
        result = main_module->inc[i].submodule;
        if (result && ly_strequal(submodule, result->name, 0)) {
            return result;
        }
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
        for (i = 0; i < INTERNAL_MODULES_COUNT; i++) {
            if (ly_strequal(name, internal_modules[i].name, 0)) {
                if (!revision || ly_strequal(revision, internal_modules[i].revision, 0)) {
                    /* return internal module */
                    return (struct lys_module *)ly_ctx_get_module(ctx, name, revision);
                }
            }
        }
        if (revision) {
            /* try to get the schema with the specific revision from the context */
            mod = ly_ctx_get_module(ctx, name, revision);
            if (mod) {
                /* we get such a module, make it implemented */
                if (lys_set_implemented(mod)) {
                    /* the schema cannot be implemented */
                    mod = NULL;
                }
                return mod;
            }
        }
    } else {
        /* searching for submodule, try if it is already loaded */
        mod = (struct lys_module *)ly_ctx_get_submodule2(module, name);
        if (mod) {
            if (!revision || (mod->rev_size && ly_strequal(mod->rev[0].date, revision, 0))) {
                /* success */
                return mod;
            } else {
                /* there is already another revision of the submodule */
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, mod->rev[0].date, "revision");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Multiple revisions of a submodule included.");
                return NULL;
            }
        }
    }

    if (ctx->module_clb) {
        if (module) {
            mod = lys_main_module(module);
            module_data = ctx->module_clb(mod->name, (mod->rev_size ? mod->rev[0].date : NULL), name, revision, ctx->module_clb_data, &format, &module_data_free);
        } else {
            module_data = ctx->module_clb(name, revision, NULL, NULL, ctx->module_clb_data, &format, &module_data_free);
        }
        if (!module_data) {
            if (module || revision) {
                /* we already know that the specified revision is not present in context, and we have no other
                 * option in case of submodules */
                LOGERR(LY_ESYS, "User module retrieval callback failed!");
                return NULL;
            } else {
                /* get the newest revision from the context */
                return ly_ctx_get_module(ctx, name, revision);
            }
        }

        if (module) {
            mod = (struct lys_module *)lys_submodule_parse(module, module_data, format, unres);
        } else {
            mod = (struct lys_module *)lys_parse_mem(ctx, module_data, format);
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
static int
ctx_modules_maintain_backlinks(struct ly_ctx *ctx, struct ly_set *mods)
{
    int o;
    uint8_t j;
    unsigned int u, v;
    struct lys_module *mod;
    struct lys_node *elem, *next;
    struct lys_node_leaf *leaf;

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    for (o = INTERNAL_MODULES_COUNT - 1; o < ctx->models.used; o++) {
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
        /* identities */
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

        /* leafrefs */
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

    return EXIT_SUCCESS;
}

API int
ly_ctx_remove_module(struct ly_ctx *ctx, const char *name, const char *revision,
                     void (*private_destructor)(const struct lys_node *node, void *priv))
{
    struct lys_module *mod = NULL;
    struct ly_set *mods;
    uint8_t j, imported;
    int i, o;
    unsigned int u;

    if (!ctx || !name) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* get the module */
    mod = (struct lys_module *)ly_ctx_get_module(ctx, name, revision);
    if (!mod) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }
    /* avoid removing internal modules ... */
    for (i = 0; i < INTERNAL_MODULES_COUNT; i++) {
        if (mod == ctx->models.list[i]) {
            LOGERR(LY_EINVAL, "Internal module \"%s\" cannot be removed.", name);
            return EXIT_FAILURE;
        }
    }
    /* ... and hide the module from the further processing of the context modules list */
    for (i = INTERNAL_MODULES_COUNT; i < ctx->models.used; i++) {
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
    for (i = INTERNAL_MODULES_COUNT; i < ctx->models.used; i++) {
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
            for (o = INTERNAL_MODULES_COUNT; o < ctx->models.used; o++) {
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
                 * (we know that there is no such module), but because the module can have import
                 * that could became useless. If there are no imports, we can continue */
                if (mod->imp_size) {
                    goto checkdependency;
                }
            }
        }
    }


    /* consolidate the modules list */
    for (i = o = INTERNAL_MODULES_COUNT; i < ctx->models.used; i++) {
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
    ctx_modules_maintain_backlinks(ctx, mods);

    /* free the modules */
    for (u = 0; u < mods->number; u++) {
        /* remove the module */
        lys_free((struct lys_module *)mods->set.g[u], private_destructor, 0);
    }
    ly_set_free(mods);

    return EXIT_SUCCESS;
}

API void
ly_ctx_clean(struct ly_ctx *ctx, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    if (!ctx) {
        return;
    }

    /* models list */
    for (i = INTERNAL_MODULES_COUNT; i < ctx->models.used; ++i) {
        lys_free(ctx->models.list[i], private_destructor, 0);
        ctx->models.list[i] = NULL;
    }
    ctx->models.used = INTERNAL_MODULES_COUNT;
    ctx->models.module_set_id++;

    /* maintain backlinks (actually done only with ietf-yang-library since its leafs cna be target of leafref) */
    ctx_modules_maintain_backlinks(ctx, NULL);
}

API const struct lys_module *
ly_ctx_get_module_iter(const struct ly_ctx *ctx, uint32_t *idx)
{
    if (!ctx || !idx) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (*idx >= (unsigned)ctx->models.used) {
        return NULL;
    }

    return ctx->models.list[(*idx)++];
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
