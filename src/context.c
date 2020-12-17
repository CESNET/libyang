/**
 * @file context.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Context implementations
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf */
#define _POSIX_C_SOURCE 200809L /* strdup */

#include "context.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "compat.h"
#include "hash_table.h"
#include "in.h"
#include "parser_data.h"
#include "path.h"
#include "plugins_types.h"
#include "schema_compile.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

#include "../models/ietf-datastores@2018-02-14.h"
#include "../models/ietf-inet-types@2013-07-15.h"
#include "../models/ietf-yang-library@2019-01-04.h"
#include "../models/ietf-yang-metadata@2016-08-05.h"
#include "../models/ietf-yang-types@2013-07-15.h"
#include "../models/yang@2020-06-17.h"
#define IETF_YANG_LIB_REV "2019-01-04"

static struct internal_modules_s {
    const char *name;
    const char *revision;
    const char *data;
    ly_bool implemented;
    LYS_INFORMAT format;
} internal_modules[] = {
    {"ietf-yang-metadata", "2016-08-05", (const char *)ietf_yang_metadata_2016_08_05_yang, 0, LYS_IN_YANG},
    {"yang", "2020-06-17", (const char *)yang_2020_06_17_yang, 1, LYS_IN_YANG},
    {"ietf-inet-types", "2013-07-15", (const char *)ietf_inet_types_2013_07_15_yang, 0, LYS_IN_YANG},
    {"ietf-yang-types", "2013-07-15", (const char *)ietf_yang_types_2013_07_15_yang, 0, LYS_IN_YANG},
    /* ietf-datastores and ietf-yang-library must be right here at the end of the list! */
    {"ietf-datastores", "2018-02-14", (const char *)ietf_datastores_2018_02_14_yang, 1, LYS_IN_YANG},
    {"ietf-yang-library", IETF_YANG_LIB_REV, (const char *)ietf_yang_library_2019_01_04_yang, 1, LYS_IN_YANG}
};

#define LY_INTERNAL_MODS_COUNT sizeof(internal_modules) / sizeof(struct internal_modules_s)

API LY_ERR
ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir)
{
    struct stat st;
    char *new_dir = NULL;

    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);

    if (search_dir) {
        new_dir = realpath(search_dir, NULL);
        LY_CHECK_ERR_RET(!new_dir,
                LOGERR(ctx, LY_ESYS, "Unable to use search directory \"%s\" (%s).", search_dir, strerror(errno)),
                LY_EINVAL);
        if (strcmp(search_dir, new_dir)) {
            LOGVRB("Canonicalizing search directory string from \"%s\" to \"%s\".", search_dir, new_dir);
        }
        LY_CHECK_ERR_RET(access(new_dir, R_OK | X_OK),
                LOGERR(ctx, LY_ESYS, "Unable to fully access search directory \"%s\" (%s).", new_dir, strerror(errno)); free(new_dir),
                LY_EINVAL);
        LY_CHECK_ERR_RET(stat(new_dir, &st),
                LOGERR(ctx, LY_ESYS, "stat() failed for \"%s\" (%s).", new_dir, strerror(errno)); free(new_dir),
                LY_ESYS);
        LY_CHECK_ERR_RET(!S_ISDIR(st.st_mode),
                LOGERR(ctx, LY_ESYS, "Given search directory \"%s\" is not a directory.", new_dir); free(new_dir),
                LY_EINVAL);
        /* avoid path duplication */
        for (uint32_t u = 0; u < ctx->search_paths.count; ++u) {
            if (!strcmp(new_dir, ctx->search_paths.objs[u])) {
                free(new_dir);
                return LY_EEXIST;
            }
        }
        if (ly_set_add(&ctx->search_paths, new_dir, 1, NULL)) {
            free(new_dir);
            return LY_EMEM;
        }

        /* new searchdir - possibly more latest revision available */
        ly_ctx_reset_latests(ctx);

        return LY_SUCCESS;
    } else {
        /* consider that no change is not actually an error */
        return LY_SUCCESS;
    }
}

API const char * const *
ly_ctx_get_searchdirs(const struct ly_ctx *ctx)
{
#define LY_CTX_SEARCHDIRS_SIZE_STEP 8
    void **new;

    LY_CHECK_ARG_RET(ctx, ctx, NULL);

    if (ctx->search_paths.count == ctx->search_paths.size) {
        /* not enough space for terminating NULL byte */
        new = realloc(((struct ly_ctx *)ctx)->search_paths.objs,
                (ctx->search_paths.size + LY_CTX_SEARCHDIRS_SIZE_STEP) * sizeof *ctx->search_paths.objs);
        LY_CHECK_ERR_RET(!new, LOGMEM(NULL), NULL);
        ((struct ly_ctx *)ctx)->search_paths.size += LY_CTX_SEARCHDIRS_SIZE_STEP;
        ((struct ly_ctx *)ctx)->search_paths.objs = new;
    }
    /* set terminating NULL byte to the strings list */
    ctx->search_paths.objs[ctx->search_paths.count] = NULL;

    return (const char * const *)ctx->search_paths.objs;
}

API LY_ERR
ly_ctx_unset_searchdir(struct ly_ctx *ctx, const char *value)
{
    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);

    if (!ctx->search_paths.count) {
        return LY_SUCCESS;
    }

    if (value) {
        /* remove specific search directory */
        uint32_t index;

        for (index = 0; index < ctx->search_paths.count; ++index) {
            if (!strcmp(value, ctx->search_paths.objs[index])) {
                break;
            }
        }
        if (index == ctx->search_paths.count) {
            LOGARG(ctx, value);
            return LY_EINVAL;
        } else {
            return ly_set_rm_index(&ctx->search_paths, index, free);
        }
    } else {
        /* remove them all */
        ly_set_erase(&ctx->search_paths, free);
        memset(&ctx->search_paths, 0, sizeof ctx->search_paths);
    }

    return LY_SUCCESS;
}

API LY_ERR
ly_ctx_unset_searchdir_last(struct ly_ctx *ctx, uint32_t count)
{
    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);

    for ( ; count > 0 && ctx->search_paths.count; --count) {
        LY_CHECK_RET(ly_set_rm_index(&ctx->search_paths, ctx->search_paths.count - 1, free))
    }

    return LY_SUCCESS;
}

API const struct lys_module *
ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision, const char **features)
{
    struct lys_module *result = NULL;
    struct lys_glob_unres unres = {0};
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);

    LY_CHECK_GOTO(ret = lysp_load_module(ctx, name, revision, 1, features, &unres, &result), cleanup);

    /* resolve unres and revert, if needed */
    LY_CHECK_GOTO(ret = lys_compile_unres_glob(ctx, &unres), cleanup);

cleanup:
    if (ret) {
        lys_compile_unres_glob_revert(ctx, &unres);
        result = NULL;
    }
    lys_compile_unres_glob_erase(ctx, &unres);
    return result;
}

API LY_ERR
ly_ctx_new(const char *search_dir, uint16_t options, struct ly_ctx **new_ctx)
{
    struct ly_ctx *ctx = NULL;
    struct lys_module *module;
    char *search_dir_list;
    char *sep, *dir;
    uint32_t i;
    struct ly_in *in = NULL;
    LY_ERR rc = LY_SUCCESS;
    struct lys_glob_unres unres = {0};

    LY_CHECK_ARG_RET(NULL, new_ctx, LY_EINVAL);

    ctx = calloc(1, sizeof *ctx);
    LY_CHECK_ERR_RET(!ctx, LOGMEM(NULL), LY_EMEM);

    /* dictionary */
    lydict_init(&ctx->dict);

#if 0 /* TODO when plugins implemented */
    /* plugins */
    ly_load_plugins();
#endif

    /* initialize thread-specific keys */
    while ((pthread_key_create(&ctx->errlist_key, ly_err_free)) == EAGAIN) {}
    while ((pthread_key_create(&ctx->log_location_key, ly_log_location_free)) == EAGAIN) {}

    /* models list */
    ctx->flags = options;
    if (search_dir) {
        search_dir_list = strdup(search_dir);
        LY_CHECK_ERR_GOTO(!search_dir_list, LOGMEM(NULL); rc = LY_EMEM, error);

        for (dir = search_dir_list; (sep = strchr(dir, ':')) != NULL && rc == LY_SUCCESS; dir = sep + 1) {
            *sep = 0;
            rc = ly_ctx_set_searchdir(ctx, dir);
            if (rc == LY_EEXIST) {
                /* ignore duplication */
                rc = LY_SUCCESS;
            }
        }
        if (*dir && (rc == LY_SUCCESS)) {
            rc = ly_ctx_set_searchdir(ctx, dir);
            if (rc == LY_EEXIST) {
                /* ignore duplication */
                rc = LY_SUCCESS;
            }
        }
        free(search_dir_list);

        /* If ly_ctx_set_searchdir() failed, the error is already logged. Just exit */
        if (rc != LY_SUCCESS) {
            goto error;
        }
    }
    ctx->module_set_id = 1;

    /* create dummy in */
    rc = ly_in_new_memory(internal_modules[0].data, &in);
    LY_CHECK_GOTO(rc, error);

    /* load internal modules */
    for (i = 0; i < ((options & LY_CTX_NO_YANGLIBRARY) ? (LY_INTERNAL_MODS_COUNT - 2) : LY_INTERNAL_MODS_COUNT); i++) {
        ly_in_memory(in, internal_modules[i].data);
        LY_CHECK_GOTO(rc = lys_create_module(ctx, in, internal_modules[i].format, internal_modules[i].implemented,
                NULL, NULL, NULL, &unres, &module), error);
    }

    /* resolve global unres */
    LY_CHECK_GOTO(rc = lys_compile_unres_glob(ctx, &unres), error);

    ly_in_free(in, 0);
    lys_compile_unres_glob_erase(ctx, &unres);
    *new_ctx = ctx;
    return rc;

error:
    ly_in_free(in, 0);
    lys_compile_unres_glob_erase(ctx, &unres);
    ly_ctx_destroy(ctx, NULL);
    return rc;
}

API uint16_t
ly_ctx_get_options(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_RET(ctx, ctx, 0);
    return ctx->flags;
}

API LY_ERR
ly_ctx_set_options(struct ly_ctx *ctx, uint16_t option)
{
    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);
    LY_CHECK_ERR_RET(option & LY_CTX_NO_YANGLIBRARY, LOGARG(ctx, option), LY_EINVAL);

    /* set the option(s) */
    ctx->flags |= option;

    return LY_SUCCESS;
}

API LY_ERR
ly_ctx_unset_options(struct ly_ctx *ctx, uint16_t option)
{
    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);
    LY_CHECK_ERR_RET(option & LY_CTX_NO_YANGLIBRARY, LOGARG(ctx, option), LY_EINVAL);

    /* unset the option(s) */
    ctx->flags &= ~option;

    return LY_SUCCESS;
}

API uint16_t
ly_ctx_get_module_set_id(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_RET(ctx, ctx, 0);
    return ctx->module_set_id;
}

API void
ly_ctx_set_module_imp_clb(struct ly_ctx *ctx, ly_module_imp_clb clb, void *user_data)
{
    LY_CHECK_ARG_RET(ctx, ctx, );

    ctx->imp_clb = clb;
    ctx->imp_clb_data = user_data;
}

API ly_module_imp_clb
ly_ctx_get_module_imp_clb(const struct ly_ctx *ctx, void **user_data)
{
    LY_CHECK_ARG_RET(ctx, ctx, NULL);

    if (user_data) {
        *user_data = ctx->imp_clb_data;
    }
    return ctx->imp_clb;
}

API const struct lys_module *
ly_ctx_get_module_iter(const struct ly_ctx *ctx, uint32_t *index)
{
    LY_CHECK_ARG_RET(ctx, ctx, index, NULL);

    if (*index < (unsigned)ctx->list.count) {
        return ctx->list.objs[(*index)++];
    } else {
        return NULL;
    }
}

/**
 * @brief Iterate over the modules in the given context. Returned modules must match the given key at the offset of
 * lysp_module and lysc_module structures (they are supposed to be placed at the same offset in both structures).
 *
 * @param[in] ctx Context where to iterate.
 * @param[in] key Key value to search for.
 * @param[in] key_size Optional length of the @p key. If zero, NULL-terminated key is expected.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @param[in,out] Iterator to pass between the function calls. On the first call, the variable is supposed to be
 * initiated to 0. After each call returning a module, the value is greater by 1 than the index of the returned
 * module in the context.
 * @return Module matching the given key, NULL if no such module found.
 */
static struct lys_module *
ly_ctx_get_module_by_iter(const struct ly_ctx *ctx, const char *key, size_t key_size, size_t key_offset, uint32_t *index)
{
    struct lys_module *mod;
    const char *value;

    for ( ; *index < ctx->list.count; ++(*index)) {
        mod = ctx->list.objs[*index];
        value = *(const char **)(((int8_t *)(mod)) + key_offset);
        if ((!key_size && !strcmp(key, value)) || (key_size && !strncmp(key, value, key_size) && (value[key_size] == '\0'))) {
            /* increment index for the next run */
            ++(*index);
            return mod;
        }
    }
    /* done */
    return NULL;
}

/**
 * @brief Unifying function for ly_ctx_get_module() and ly_ctx_get_module_ns()
 * @param[in] ctx Context where to search.
 * @param[in] key Name or Namespace as a search key.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @param[in] revision Revision date to match. If NULL, the matching module must have no revision. To search for the latest
 * revision module, use ly_ctx_get_module_latest_by().
 * @return Matching module if any.
 */
static struct lys_module *
ly_ctx_get_module_by(const struct ly_ctx *ctx, const char *key, size_t key_offset, const char *revision)
{
    struct lys_module *mod;
    uint32_t index = 0;

    while ((mod = ly_ctx_get_module_by_iter(ctx, key, 0, key_offset, &index))) {
        if (!revision) {
            if (!mod->revision) {
                /* found requested module without revision */
                return mod;
            }
        } else {
            if (mod->revision && !strcmp(mod->revision, revision)) {
                /* found requested module of the specific revision */
                return mod;
            }
        }
    }

    return NULL;
}

API struct lys_module *
ly_ctx_get_module_ns(const struct ly_ctx *ctx, const char *ns, const char *revision)
{
    LY_CHECK_ARG_RET(ctx, ctx, ns, NULL);
    return ly_ctx_get_module_by(ctx, ns, offsetof(struct lys_module, ns), revision);
}

API struct lys_module *
ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_by(ctx, name, offsetof(struct lys_module, name), revision);
}

/**
 * @brief Unifying function for ly_ctx_get_module_latest() and ly_ctx_get_module_latest_ns()
 * @param[in] ctx Context where to search.
 * @param[in] key Name or Namespace as a search key.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @return Matching module if any.
 */
static struct lys_module *
ly_ctx_get_module_latest_by(const struct ly_ctx *ctx, const char *key, size_t key_offset)
{
    struct lys_module *mod;
    uint32_t index = 0;

    while ((mod = ly_ctx_get_module_by_iter(ctx, key, 0, key_offset, &index))) {
        if (mod->latest_revision) {
            return mod;
        }
    }

    return NULL;
}

API struct lys_module *
ly_ctx_get_module_latest(const struct ly_ctx *ctx, const char *name)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_latest_by(ctx, name, offsetof(struct lys_module, name));
}

API struct lys_module *
ly_ctx_get_module_latest_ns(const struct ly_ctx *ctx, const char *ns)
{
    LY_CHECK_ARG_RET(ctx, ctx, ns, NULL);
    return ly_ctx_get_module_latest_by(ctx, ns, offsetof(struct lys_module, ns));
}

/**
 * @brief Unifying function for ly_ctx_get_module_implemented() and ly_ctx_get_module_implemented_ns()
 * @param[in] ctx Context where to search.
 * @param[in] key Name or Namespace as a search key.
 * @param[in] key_size Optional length of the @p key. If zero, NULL-terminated key is expected.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @return Matching module if any.
 */
static struct lys_module *
ly_ctx_get_module_implemented_by(const struct ly_ctx *ctx, const char *key, size_t key_size, size_t key_offset)
{
    struct lys_module *mod;
    uint32_t index = 0;

    while ((mod = ly_ctx_get_module_by_iter(ctx, key, key_size, key_offset, &index))) {
        if (mod->implemented) {
            return mod;
        }
    }

    return NULL;
}

API struct lys_module *
ly_ctx_get_module_implemented(const struct ly_ctx *ctx, const char *name)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_implemented_by(ctx, name, 0, offsetof(struct lys_module, name));
}

struct lys_module *
ly_ctx_get_module_implemented2(const struct ly_ctx *ctx, const char *name, size_t name_len)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_implemented_by(ctx, name, name_len, offsetof(struct lys_module, name));
}

API struct lys_module *
ly_ctx_get_module_implemented_ns(const struct ly_ctx *ctx, const char *ns)
{
    LY_CHECK_ARG_RET(ctx, ctx, ns, NULL);
    return ly_ctx_get_module_implemented_by(ctx, ns, 0, offsetof(struct lys_module, ns));
}

struct lysp_submodule *
ly_ctx_get_submodule(const struct ly_ctx *ctx, const struct lys_module *module, const char *submodule, const char *revision)
{
    const struct lys_module *mod;
    struct lysp_include *inc;
    uint32_t v;
    LY_ARRAY_COUNT_TYPE u;

    assert((ctx || module) && submodule);

    if (module) {
        if (!module->parsed) {
            return NULL;
        }

        /* check only this module */
        mod = module;
        goto check_mod;
    }

    for (v = 0; v < ctx->list.count; ++v) {
        mod = ctx->list.objs[v];
        if (!mod->parsed) {
            continue;
        }

check_mod:
        LY_ARRAY_FOR(mod->parsed->includes, u) {
            if (mod->parsed->includes[u].submodule && !strcmp(submodule, mod->parsed->includes[u].submodule->name)) {
                inc = &mod->parsed->includes[u];
                if (!revision) {
                    if (inc->submodule->latest_revision) {
                        return inc->submodule;
                    }
                } else if (inc->submodule->revs && !strcmp(revision, inc->submodule->revs[0].date)) {
                    return inc->submodule;
                }
            }
        }

        if (module) {
            /* do not check other modules */
            break;
        }
    }

    return NULL;
}

API void
ly_ctx_reset_latests(struct ly_ctx *ctx)
{
    struct lys_module *mod;

    for (uint32_t v = 0; v < ctx->list.count; ++v) {
        mod = ctx->list.objs[v];
        if (mod->latest_revision == 2) {
            mod->latest_revision = 1;
        }
        if (mod->parsed && mod->parsed->includes) {
            for (LY_ARRAY_COUNT_TYPE u = 0; u < LY_ARRAY_COUNT(mod->parsed->includes); ++u) {
                if (mod->parsed->includes[u].submodule->latest_revision == 2) {
                    mod->parsed->includes[u].submodule->latest_revision = 1;
                }
            }
        }
    }
}

API uint32_t
ly_ctx_internal_modules_count(const struct ly_ctx *ctx)
{
    if (!ctx) {
        return 0;
    }

    if (ctx->flags & LY_CTX_NO_YANGLIBRARY) {
        return LY_INTERNAL_MODS_COUNT - 2;
    } else {
        return LY_INTERNAL_MODS_COUNT;
    }
}

static LY_ERR
ylib_feature(struct lyd_node *parent, const struct lysp_module *pmod)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_feature *f;

    if (!pmod->mod->implemented) {
        /* no features can be enabled */
        return LY_SUCCESS;
    }

    LY_ARRAY_FOR(pmod->features, struct lysp_feature, f) {
        if (!(f->flags & LYS_FENABLED)) {
            continue;
        }

        LY_CHECK_RET(lyd_new_term(parent, NULL, "feature", f->name, 0, NULL));
    }

    LY_ARRAY_FOR(pmod->includes, u) {
        LY_ARRAY_FOR(pmod->includes[u].submodule->features, struct lysp_feature, f) {
            if (!(f->flags & LYS_FENABLED)) {
                continue;
            }

            LY_CHECK_RET(lyd_new_term(parent, NULL, "feature", f->name, 0, NULL));
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
ylib_deviation(struct lyd_node *parent, const struct lys_module *cur_mod, ly_bool bis)
{
    LY_ARRAY_COUNT_TYPE i;
    struct lys_module *mod;

    if (!cur_mod->implemented) {
        /* no deviations of the module for certain */
        return LY_SUCCESS;
    }

    LY_ARRAY_FOR(cur_mod->deviated_by, i) {
        mod = cur_mod->deviated_by[i];

        if (bis) {
            LY_CHECK_RET(lyd_new_term(parent, NULL, "deviation", mod->name, 0, NULL));
        } else {
            LY_CHECK_RET(lyd_new_list(parent, NULL, "deviation", 0, NULL, mod->name,
                    (mod->parsed->revs ? mod->parsed->revs[0].date : "")));
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
ylib_submodules(struct lyd_node *parent, const struct lysp_module *pmod, ly_bool bis)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE i;
    struct lyd_node *cont;
    struct lysp_submodule *submod;
    int r;
    char *str;

    LY_ARRAY_FOR(pmod->includes, i) {
        submod = pmod->includes[i].submodule;

        if (bis) {
            LY_CHECK_RET(lyd_new_list(parent, NULL, "submodule", 0, &cont, submod->name));

            if (submod->revs) {
                LY_CHECK_RET(lyd_new_term(cont, NULL, "revision", submod->revs[0].date, 0, NULL));
            }
        } else {
            LY_CHECK_RET(lyd_new_list(parent, NULL, "submodule", 0, &cont, submod->name,
                    (submod->revs ? submod->revs[0].date : "")));
        }

        if (submod->filepath) {
            r = asprintf(&str, "file://%s", submod->filepath);
            LY_CHECK_ERR_RET(r == -1, LOGMEM(pmod->mod->ctx), LY_EMEM);

            ret = lyd_new_term(cont, NULL, bis ? "location" : "schema", str, 0, NULL);
            free(str);
            LY_CHECK_RET(ret);
        }
    }

    return LY_SUCCESS;
}

API uint16_t
ly_ctx_get_yanglib_id(const struct ly_ctx *ctx)
{
    return ctx->module_set_id;
}

API LY_ERR
ly_ctx_get_yanglib_data(const struct ly_ctx *ctx, struct lyd_node **root_p)
{
    LY_ERR ret;
    uint32_t i;
    ly_bool bis = 0;
    int r;
    char *str;
    const struct lys_module *mod;
    struct lyd_node *root = NULL, *root_bis = NULL, *cont, *set_bis = NULL;

    LY_CHECK_ARG_RET(ctx, ctx, root_p, LY_EINVAL);

    mod = ly_ctx_get_module_implemented(ctx, "ietf-yang-library");
    LY_CHECK_ERR_RET(!mod, LOGERR(ctx, LY_EINVAL, "Module \"ietf-yang-library\" is not implemented."), LY_EINVAL);

    if (mod->parsed->revs && !strcmp(mod->parsed->revs[0].date, "2016-06-21")) {
        bis = 0;
    } else if (mod->parsed->revs && !strcmp(mod->parsed->revs[0].date, IETF_YANG_LIB_REV)) {
        bis = 1;
    } else {
        LOGERR(ctx, LY_EINVAL, "Incompatible ietf-yang-library version in context.");
        return LY_EINVAL;
    }

    LY_CHECK_GOTO(ret = lyd_new_inner(NULL, mod, "modules-state", 0, &root), error);

    if (bis) {
        LY_CHECK_GOTO(ret = lyd_new_inner(NULL, mod, "yang-library", 0, &root_bis), error);
        LY_CHECK_GOTO(ret = lyd_new_list(root_bis, NULL, "module-set", 0, &set_bis, "complete"), error);
    }

    for (i = 0; i < ctx->list.count; ++i) {
        mod = ctx->list.objs[i];
        if (!mod->parsed) {
            LOGERR(ctx, LY_ENOTFOUND, "Parsed module \"%s\" missing in the context.", mod->name);
            goto error;
        }

        /*
         * deprecated legacy
         */
        LY_CHECK_GOTO(ret = lyd_new_list(root, NULL, "module", 0, &cont, mod->name,
                (mod->parsed->revs ? mod->parsed->revs[0].date : "")), error);

        /* schema */
        if (mod->filepath) {
            r = asprintf(&str, "file://%s", mod->filepath);
            LY_CHECK_ERR_GOTO(r == -1, LOGMEM(ctx); ret = LY_EMEM, error);

            ret = lyd_new_term(cont, NULL, "schema", str, 0, NULL);
            free(str);
            LY_CHECK_GOTO(ret, error);
        }

        /* namespace */
        LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "namespace", mod->ns, 0, NULL), error);

        /* feature leaf-list */
        LY_CHECK_GOTO(ret = ylib_feature(cont, mod->parsed), error);

        /* deviation list */
        LY_CHECK_GOTO(ret = ylib_deviation(cont, mod, 0), error);

        /* conformance-type */
        LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "conformance-type", mod->implemented ? "implement" : "import", 0,
                NULL), error);

        /* submodule list */
        LY_CHECK_GOTO(ret = ylib_submodules(cont, mod->parsed, 0), error);

        /*
         * current revision
         */
        if (bis) {
            /* name and revision */
            if (mod->implemented) {
                LY_CHECK_GOTO(ret = lyd_new_list(set_bis, NULL, "module", 0, &cont, mod->name), error);

                if (mod->parsed->revs) {
                    LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "revision", mod->parsed->revs[0].date, 0, NULL), error);
                }
            } else {
                LY_CHECK_GOTO(ret = lyd_new_list(set_bis, NULL, "import-only-module", 0, &cont, mod->name,
                        (mod->parsed->revs ? mod->parsed->revs[0].date : "")), error);
            }

            /* namespace */
            LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "namespace", mod->ns, 0, NULL), error);

            /* location */
            if (mod->filepath) {
                r = asprintf(&str, "file://%s", mod->filepath);
                LY_CHECK_ERR_GOTO(r == -1, LOGMEM(ctx); ret = LY_EMEM, error);

                ret = lyd_new_term(cont, NULL, "location", str, 0, NULL);
                free(str);
                LY_CHECK_GOTO(ret, error);
            }

            /* submodule list */
            LY_CHECK_GOTO(ret = ylib_submodules(cont, mod->parsed, 1), error);

            /* feature list */
            LY_CHECK_GOTO(ret = ylib_feature(cont, mod->parsed), error);

            /* deviation */
            LY_CHECK_GOTO(ret = ylib_deviation(cont, mod, 1), error);
        }
    }

    /* IDs */
    r = asprintf(&str, "%u", ctx->module_set_id);
    LY_CHECK_ERR_GOTO(r == -1, LOGMEM(ctx); ret = LY_EMEM, error);
    ret = lyd_new_term(root, NULL, "module-set-id", str, 0, NULL);
    LY_CHECK_ERR_GOTO(ret, free(str), error);

    if (bis) {
        /* create one complete schema */
        LY_CHECK_ERR_GOTO(ret = lyd_new_list(root_bis, NULL, "schema", 0, &cont, "complete"), free(str), error);

        LY_CHECK_ERR_GOTO(ret = lyd_new_term(cont, NULL, "module-set", "complete", 0, NULL), free(str), error);

        /* content-id */
        LY_CHECK_ERR_GOTO(ret = lyd_new_term(root_bis, NULL, "content-id", str, 0, NULL), free(str), error);
    }
    free(str);

    if (root_bis) {
        if (lyd_insert_sibling(root, root_bis, &root)) {
            goto error;
        }
        root_bis = NULL;
    }

    LY_CHECK_GOTO(ret = lyd_validate_all(&root, NULL, LYD_VALIDATE_PRESENT, NULL), error);

    *root_p = root;
    return LY_SUCCESS;

error:
    lyd_free_all(root);
    lyd_free_all(root_bis);
    return ret;
}

API void
ly_ctx_destroy(struct ly_ctx *ctx, void (*private_destructor)(const struct lysc_node *node, void *priv))
{
    if (!ctx) {
        return;
    }

    /* models list */
    for ( ; ctx->list.count; ctx->list.count--) {
        /* remove the module */
        lys_module_free(ctx->list.objs[ctx->list.count - 1], private_destructor);
    }
    free(ctx->list.objs);

    /* search paths list */
    ly_set_erase(&ctx->search_paths, free);

    /* clean the error list */
    ly_err_clean(ctx, 0);
    pthread_key_delete(ctx->errlist_key);

    /* clean the error location data */
    ly_log_location_free(pthread_getspecific(ctx->log_location_key));
    pthread_setspecific(ctx->log_location_key, NULL);
    pthread_key_delete(ctx->log_location_key);

    /* dictionary */
    lydict_clean(&ctx->dict);

#if 0 /* TODO when plugins implemented */
    /* plugins - will be removed only if this is the last context */
    ly_clean_plugins();
#endif

    free(ctx);
}
