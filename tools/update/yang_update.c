/**
 * @file yang_update.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yang update source
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include "yang_update.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libyang/libyang.h>

#include "ly_common.h"
#include "path.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/* generated */
#include "yang_update_plugins.h"

/**
 * @brief Add a plugin into an array.
 *
 * @param[in] plg Plugin to add.
 * @param[in] idx Index to add on.
 * @param[in,out] plgs Array of plugins to add to.
 * @param[in,out] plg_count Count of @p plgs.
 * @return LY_ERR value.
 */
static LY_ERR
yu_plg_add(struct lyu_plg *plg, uint32_t idx, struct lyu_plg ***plgs, uint32_t *plg_count)
{
    void *mem;

    assert(idx <= *plg_count);

    mem = realloc(*plgs, (*plg_count + 1) * sizeof **plgs);
    LY_CHECK_ERR_RET(!mem, LOGMEM(NULL), LY_EMEM);
    *plgs = mem;

    if (idx < *plg_count) {
        memmove((*plgs) + idx + 1, (*plgs) + idx, (*plg_count - idx) * sizeof **plgs);
    }

    (*plgs)[idx] = plg;
    ++(*plg_count);

    return LY_SUCCESS;
}

/**
 * @brief Perform basic checks of a plugin.
 *
 * @param[in] ctx Context to use for logging.
 * @param[in] plg Plugin to check.
 * @return LY_ERR value.
 */
static LY_ERR
yu_plg_check(const struct ly_ctx *ctx, const struct lyu_plg *plg)
{
    uint32_t i;
    struct lyu_plg_module *imp;

    /* module name */
    if (!plg->module_name) {
        LOGERR(ctx, LY_EINVAL, "Plugin without its \"module_name\".");
        return LY_EINVAL;
    }

    /* revisions */
    if (!plg->revision_old || !plg->revision_new) {
        LOGERR(ctx, LY_EINVAL, "Plugin \"%s\" without its \"revision_old\" or \"revision_new\".", plg->module_name);
        return LY_EINVAL;
    } else if (lys_check_date(ctx, plg->revision_old, strlen(plg->revision_old), "revision_old")) {
        return LY_EINVAL;
    } else if (lys_check_date(ctx, plg->revision_new, strlen(plg->revision_new), "revision_new")) {
        return LY_EINVAL;
    } else if (strcmp(plg->revision_old, plg->revision_new) >= 0) {
        LOGERR(ctx, LY_EINVAL, "Plugin \"%s\" with invalid revision old %s and new %s.", plg->module_name,
                plg->revision_old, plg->revision_new);
        return LY_EINVAL;
    }

    /* imports */
    if (plg->imports_old) {
        for (i = 0; plg->imports_old[i].name; ++i) {
            imp = &plg->imports_old[i];

            if (imp->revision && lys_check_date(ctx, imp->revision, strlen(imp->revision), "revision")) {
                return LY_EINVAL;
            } else if (imp->features && imp->features[0] && !imp->implemented) {
                LOGERR(ctx, LY_EINVAL, "Old import \"%s\" with features must be implemented.", imp->name);
                return LY_EINVAL;
            }
        }
    }
    if (plg->imports_new) {
        for (i = 0; plg->imports_new[i].name; ++i) {
            imp = &plg->imports_new[i];

            if (imp->revision && lys_check_date(ctx, imp->revision, strlen(imp->revision), "revision")) {
                return LY_EINVAL;
            } else if (imp->features && imp->features[0] && !imp->implemented) {
                LOGERR(ctx, LY_EINVAL, "New import \"%s\" with features must be implemented.", imp->name);
                return LY_EINVAL;
            }
        }
    }

    /* rules checked when the paths are compiled */

    return LY_SUCCESS;
}

/**
 * @brief Concatenate features into a string.
 *
 * @param[in] features Array of features terminated by NULL.
 * @return String with all the features.
 */
static char *
yu_features_str(const char **features)
{
    uint32_t i, len = 0, used;
    char *str;

    /* count the length */
    for (i = 0; features[i]; ++i) {
        if (i) {
            len += 2;
        }

        len += 1 + strlen(features[i]) + 1;
    }

    str = malloc(len + 1);
    LY_CHECK_ERR_RET(!str, LOGMEM(NULL), NULL);
    used = 0;

    /* print */
    for (i = 0; features[i]; ++i) {
        if (i) {
            used += sprintf(str + used, ", ");
        }

        used += sprintf(str + used, "\"%s\"", features[i]);
    }
    assert(used == len);

    return str;
}

/**
 * @brief Compare revisions and feature arrays for an exact match.
 *
 * @param[in] rev1 First revision.
 * @param[in] feats1 First feature array, unset means no enabled features.
 * @param[in] rev2 Second revision.
 * @param[in] feats2 Second feature array, unset means any enabled features (skip check).
 * @return 1 if the revisions and features match;
 * @return 0 otherwise.
 */
static ly_bool
yu_rev_feat_equal(const char *rev1, const char **feats1, const char *rev2, const char **feats2)
{
    const char *no_feats[] = {NULL};
    uint32_t i, j;
    ly_bool found;

    assert(rev1);

    if (!rev2 && !feats2) {
        /* never matches */
        return 0;
    }

    /* revision */
    if (rev2 && strcmp(rev1, rev2)) {
        return 0;
    }

    /* features */
    if (feats2) {
        if (!feats1) {
            /* unset means no enabled features */
            feats1 = no_feats;
        }

        for (i = 0; feats1[i]; ++i) {
            found = 0;
            for (j = 0; feats2[j]; ++j) {
                if (!strcmp(feats1[i], feats2[j])) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                /* not found */
                return 0;
            }
        }

        /* check all features matched */
        for (j = 0; feats2[j]; ++j) {}
        if (i != j) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Collect all plugins for a module from the compiled plugins starting at the end with new module.
 *
 * @param[in] ctx Context to use for logging.
 * @param[in] mod_name Module name.
 * @param[in] revision_old Old module revision (first).
 * @param[in] features_old Old module enabled features.
 * @param[in] revision_new Optional new module revision (last). If not set, find the latest.
 * @param[in] features_new Optional new module enabled features. If not set, use those of the latest revision.
 * @param[in,out] plgs Array of found plugins.
 * @param[in,out] plg_count Count of @p plgs.
 * @return LY_ERR value.
 */
static LY_ERR
yu_plg_collect_new(const struct ly_ctx *ctx, const char *mod_name, const char *revision_old, const char **features_old,
        const char *revision_new, const char **features_new, struct lyu_plg ***plgs, uint32_t *plg_count)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyu_plg *plg;
    uint32_t i, orig_plg_count;
    const char *rev, **feats;
    char *feats_str;
    ly_bool found;

    assert(revision_new || features_new);

    orig_plg_count = *plg_count;

    /* find the last plugin */
    plg = NULL;
    for (i = 0; i < sizeof lyu_plugins / sizeof *lyu_plugins; ++i) {
        /* module name */
        if (strcmp(mod_name, lyu_plugins[i]->module_name)) {
            continue;
        }

        /* compare new revision and/or features */
        if (!yu_rev_feat_equal(lyu_plugins[i]->revision_new, lyu_plugins[i]->features_new, revision_new, features_new)) {
            continue;
        }

        if (!plg) {
            /* first match */
            plg = lyu_plugins[i];
        } else if (!revision_new && (strcmp(plg->revision_new, lyu_plugins[i]->revision_new) < 0)) {
            /* newer revision */
            plg = lyu_plugins[i];
        } else if (revision_new) {
            /* same revision possibly with a different set of features */
            if (features_new) {
                feats_str = yu_features_str(features_new);
                LOGERR(ctx, LY_EDENIED, "Ambiguous last plugin of \"%s\" with new revision %s and features %s.",
                        mod_name, revision_new, feats_str);
                free(feats_str);
            } else {
                LOGERR(ctx, LY_EDENIED, "Ambiguous last plugin of \"%s\" with new revision %s.",
                        mod_name, revision_new);
            }
            rc = LY_EDENIED;
            goto cleanup;
        }
    }

    if (!plg) {
        if (features_new) {
            feats_str = yu_features_str(features_new);
            LOGERR(ctx, LY_ENOTFOUND, "Failed to find the last plugin of \"%s\" with new revision %s and features %s.",
                    mod_name, revision_new, feats_str);
            free(feats_str);
        } else {
            LOGERR(ctx, LY_ENOTFOUND, "Failed to find the last plugin of \"%s\" with new revision %s.",
                    mod_name, revision_new);
        }
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

    /* add a plugin */
    LY_CHECK_GOTO(rc = yu_plg_add(lyu_plugins[i], orig_plg_count, plgs, plg_count), cleanup);

    /* continue search from the end */
    rev = (*plgs)[*plg_count - 1]->revision_old;
    feats = (*plgs)[*plg_count - 1]->features_old;

    while (1) {
        /* check end */
        if (orig_plg_count && yu_rev_feat_equal((*plgs)[orig_plg_count - 1]->revision_new,
                (*plgs)[orig_plg_count - 1]->features_new, rev, feats)) {
            break;
        } else if (!orig_plg_count && yu_rev_feat_equal(revision_old, features_old, rev, feats)) {
            break;
        }

        found = 0;
        for (i = 0; i < sizeof lyu_plugins / sizeof *lyu_plugins; ++i) {
            /* module name */
            if (strcmp(mod_name, lyu_plugins[i]->module_name)) {
                continue;
            }

            /* compare new revision and features */
            if (!yu_rev_feat_equal(lyu_plugins[i]->revision_old, lyu_plugins[i]->features_old, rev, feats)) {
                continue;
            }

            if (!found) {
                found = 1;
            } else if (found) {
                /* more matches */
                found = 2;
                break;
            }

            /* add a plugin */
            LY_CHECK_GOTO(rc = yu_plg_add(lyu_plugins[i], orig_plg_count, plgs, plg_count), cleanup);
        }

        if (!found) {
            /* failed next search */
            feats_str = yu_features_str(feats);
            LOGERR(ctx, LY_ENOTFOUND, "Failed to find the next plugin of \"%s\" with new revision %s and features %s.",
                    mod_name, rev, feats_str);
            free(feats_str);
            rc = LY_ENOTFOUND;
            goto cleanup;
        } else if (found == 1) {
            /* continue search */
        } else if (found == 2) {
            /* ambiguous next search */
            feats_str = yu_features_str(feats);
            LOGERR(ctx, LY_EDENIED, "Ambiguous next plugin of \"%s\" with old revision %s and features %s.",
                    mod_name, rev, feats_str);
            free(feats_str);
            rc = LY_EDENIED;
            goto cleanup;
        }

        /* next iter */
        rev = (*plgs)[orig_plg_count]->revision_old;
        feats = (*plgs)[orig_plg_count]->features_old;
    }

cleanup:
    if (rc) {
        *plg_count = orig_plg_count;
    }
    return rc;
}

/**
 * @brief Collect all plugins for a module from the compiled plugins starting at the beginning with old module.
 *
 * @param[in] ctx Context to use for logging.
 * @param[in] mod_name Module name.
 * @param[in] revision_old Old module revision (first).
 * @param[in] features_old Old module enabled features.
 * @param[in] revision_new Optional new module revision (last). If not set, find the latest.
 * @param[in] features_new Optional new module enabled features. If not set, use those of the latest revision.
 * @param[out] plgs Array of found plugins.
 * @param[out] plg_count Count of @p plgs.
 * @return LY_ERR value.
 */
static LY_ERR
yu_plg_collect(const struct ly_ctx *ctx, const char *mod_name, const char *revision_old, const char **features_old,
        const char *revision_new, const char **features_new, struct lyu_plg ***plgs, uint32_t *plg_count)
{
    LY_ERR rc = LY_SUCCESS;
    uint32_t i;
    const char *rev, **feats;
    char *feats_str;
    ly_bool checks, found;

    assert(ctx && mod_name && revision_old && features_old && plgs && plg_count);

    *plgs = NULL;
    *plg_count = 0;

    /* start from the beginning */
    rev = revision_old;
    feats = features_old;

    /* check the plugins only once */
    checks = 1;
    while (1) {
        found = 0;
        for (i = 0; i < sizeof lyu_plugins / sizeof *lyu_plugins; ++i) {
            /* check the plugin */
            if (checks && (rc = yu_plg_check(ctx, lyu_plugins[i]))) {
                goto cleanup;
            }

            /* module name */
            if (strcmp(mod_name, lyu_plugins[i]->module_name)) {
                continue;
            }

            /* compare old revision and features */
            if (!yu_rev_feat_equal(lyu_plugins[i]->revision_old, lyu_plugins[i]->features_old, rev, feats)) {
                continue;
            }

            if (!found) {
                found = 1;
            } else if (found) {
                /* more matches */
                found = 2;
                break;
            }

            /* add a plugin */
            LY_CHECK_GOTO(rc = yu_plg_add(lyu_plugins[i], *plg_count, plgs, plg_count), cleanup);
        }
        checks = 0;

        if (!found) {
            if (!revision_new && !features_new) {
                /* done, latest found */
                break;
            }

            /* failed next search */
            feats_str = yu_features_str(feats);
            LOGERR(ctx, LY_ENOTFOUND, "Failed to find the next plugin of \"%s\" with old revision %s and features %s.",
                    mod_name, rev, feats_str);
            free(feats_str);
            rc = LY_ENOTFOUND;
            goto cleanup;
        } else if (found == 1) {
            if (yu_rev_feat_equal((*plgs)[*plg_count - 1]->revision_new, (*plgs)[*plg_count - 1]->features_new,
                    revision_new, features_new)) {
                /* done */
                break;
            }

            /* continue search */
        } else if (found == 2) {
            if (!revision_new && !features_new) {
                /* ambiguous next search */
                feats_str = yu_features_str(feats);
                LOGERR(ctx, LY_EDENIED, "Ambiguous plugin of \"%s\" with old revision %s and features %s.",
                        mod_name, rev, feats_str);
                free(feats_str);
                rc = LY_EDENIED;
                goto cleanup;
            }

            /* special search, try to match plugins based on new revision and/or features */
            LY_CHECK_GOTO(rc = yu_plg_collect_new(ctx, mod_name, rev, feats, revision_new, features_new, plgs, plg_count),
                    cleanup);
            break;
        }

        /* next iter */
        rev = (*plgs)[*plg_count - 1]->revision_new;
        feats = (*plgs)[*plg_count - 1]->features_new;
    }

cleanup:
    if (rc) {
        free(*plgs);
        *plgs = NULL;
        *plg_count = 0;
    }
    return rc;
}

struct yu_module_imp_arg {
    const char * const *search_dirs;
    const struct lyu_plg_module *imports;
};

/**
 * @brief Context module import frree callback for libyang.
 */
static void
yu_module_imp_data_free_clb(void *module_data, void *UNUSED(user_data))
{
    free(module_data);
}

/**
 * @brief Context module import callback for libyang.
 */
static LY_ERR
yu_module_imp_clb(const char *mod_name, const char *mod_rev, const char *submod_name, const char *submod_rev,
        void *user_data, LYS_INFORMAT *format, const char **module_data, ly_module_imp_data_free_clb *free_module_data)
{
    struct yu_module_imp_arg *arg = user_data;
    LY_ERR rc = LY_SUCCESS;
    const struct lyu_plg_module *imp;
    uint32_t i;
    char *path = NULL, *data;
    LYS_INFORMAT fmt;
    FILE *f = NULL;
    long size;

    /* try to find the (sub)module */
    if (submod_name) {
        LY_CHECK_GOTO(rc = lys_search_localfile(arg->search_dirs, 0, submod_name, submod_rev, &path, &fmt), cleanup);
    } else {
        /* look for the module in the imports */
        imp = NULL;
        for (i = 0; arg->imports && arg->imports[i].name; ++i) {
            if (!strcmp(arg->imports[i].name, mod_name)) {
                imp = &arg->imports[i];
                break;
            }
        }

        if (imp && mod_rev && imp->revision && strcmp(mod_rev, imp->revision)) {
            LOGERR(NULL, LY_EDENIED, "Module \"%s\" imported in revision %s but plugin requires revision %s.",
                    mod_name, mod_rev, imp->revision);
            rc = LY_EDENIED;
            goto cleanup;
        } else if (imp && imp->revision) {
            mod_rev = imp->revision;
        }

        LY_CHECK_GOTO(rc = lys_search_localfile(arg->search_dirs, 0, mod_name, mod_rev, &path, &fmt), cleanup);
    }

    if (!path) {
        /* modue not found */
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

    /* load module data */
    f = fopen(path, "r");
    if (!f) {
        LOGERR(NULL, LY_ESYS, "Failed to open \"%s\" (%s).", path, strerror(errno));
        rc = LY_ESYS;
        goto cleanup;
    }

    if (fseek(f, 0, SEEK_END)) {
        LOGERR(NULL, LY_ESYS, "Failed to seek \"%s\" (%s).", path, strerror(errno));
        rc = LY_ESYS;
        goto cleanup;
    }
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = malloc(size + 1);
    if (!data) {
        LOGMEM(NULL);
        rc = LY_EMEM;
        goto cleanup;
    }
    fread(data, 1, size, f);
    data[size] = '\0';

    *format = fmt;
    *module_data = data;
    *free_module_data = yu_module_imp_data_free_clb;

cleanup:
    if (f) {
        fclose(f);
    }
    free(path);
    return rc;
}

/**
 * @brief Check that the context contains all the explicit imports in their desired state
 * and make changes if possible.
 *
 * @param[in] ctx Context to use.
 * @param[in] imports Array of imports to use.
 * @return LY_ERR value.
 */
static LY_ERR
yu_ctx_load_check_imports(struct ly_ctx *ctx, const struct lyu_plg_module *imports)
{
    LY_ERR rc = LY_SUCCESS;
    struct lys_module *mod;
    uint32_t i;

    if (!imports) {
        /* nothing to check */
        goto cleanup;
    }

    /* implement modules/enable only the requested features */
    for (i = 0; imports[i].name; ++i) {
        if (!imports[i].implemented) {
            continue;
        }

        /* try to get the module from the context */
        if (imports[i].revision) {
            mod = ly_ctx_get_module(ctx, imports[i].name, imports[i].revision);
        } else {
            mod = ly_ctx_get_module_implemented(ctx, imports[i].name);
        }

        if (!mod) {
            /* load the module */
            mod = ly_ctx_load_module(ctx, imports[i].name, imports[i].revision, imports[i].features);
            if (!mod) {
                rc = LY_ENOTFOUND;
                goto cleanup;
            }
        } else if (imports[i].features) {
            /* set the features */
            LY_CHECK_GOTO(rc = lys_set_implemented(mod, imports[i].features), cleanup);
        }
    }

    /* check imported modules */
    for (i = 0; imports[i].name; ++i) {
        if (imports[i].implemented) {
            continue;
        }

        /* try to get the module from the context */
        if (imports[i].revision) {
            mod = ly_ctx_get_module(ctx, imports[i].name, imports[i].revision);
        } else {
            mod = ly_ctx_get_module_latest(ctx, imports[i].name);
        }

        if (!mod) {
            LOGERR(NULL, LY_EINVAL, "Imported plugin module \"%s%s%s\" not imported by any module.", imports[i].name,
                    imports[i].revision ? "@" : "", imports[i].revision ? imports[i].revision : "");
            rc = LY_EINVAL;
            goto cleanup;
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Create a new context with a specific revision of a module.
 *
 * @param[in] mod_name Module name.
 * @param[in] revision Module revision.
 * @param[in] features Features to enable.
 * @param[in] search_dirs Search dirs to use.
 * @param[in] imports All the specific imports to use.
 * @param[out] ctx Created context.
 * @param[out] mod Loaded module.
 * @return LY_ERR value.
 */
static LY_ERR
yu_ctx_load(const char *mod_name, const char *revision, const char **features, const char * const *search_dirs,
        const struct lyu_plg_module *imports, struct ly_ctx **ctx, struct lys_module **mod)
{
    LY_ERR rc = LY_SUCCESS;
    struct yu_module_imp_arg arg;
    uint32_t i;

    *ctx = NULL;

    /* create the context */
    if ((rc = ly_ctx_new(NULL, 0, ctx))) {
        goto cleanup;
    }

    /* set the import callback */
    arg.search_dirs = search_dirs;
    arg.imports = imports;
    ly_ctx_set_module_imp_clb(*ctx, yu_module_imp_clb, &arg);

    /* load the main module */
    *mod = ly_ctx_load_module(*ctx, mod_name, revision, features);
    if (!*mod) {
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

    /* check the context and plugin imports */
    LY_CHECK_GOTO(rc = yu_ctx_load_check_imports(*ctx, imports), cleanup);

    /* load ietf-yang-schema-comparison */
    if (!ly_ctx_load_module(*ctx, "ietf-yang-schema-comparison", NULL, NULL)) {
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

    /* make the context usable by user */
    ly_ctx_set_module_imp_clb(*ctx, NULL, NULL);
    for (i = 0; search_dirs && search_dirs[i]; ++i) {
        ly_ctx_set_searchdir(*ctx, search_dirs[i]);
    }

cleanup:
    if (rc) {
        ly_ctx_destroy(*ctx);
        *ctx = NULL;
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_update_find_new(const struct lys_module *mod_old, const char *revision_new, const char **features_new,
        struct ly_ctx **ctx_new, struct lys_module **mod_new)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyu_plg **plgs = NULL;
    uint32_t plg_count = 0;

    LY_CHECK_ARG_RET(NULL, mod_old, mod_old->implemented, ctx_new, mod_new, LY_EINVAL);
    if (!mod_old->revision) {
        LOGERR(mod_old->ctx, LY_EINVAL, "Old module \"%s\" without a revision.", mod_old->name);
        return LY_EINVAL;
    } else if (revision_new && (strcmp(mod_old->revision, revision_new) >= 0)) {
        LOGERR(mod_old->ctx, LY_EINVAL, "Module \"%s\" old revision %s not earlier than its new revision %s.",
                mod_old->name, mod_old->revision, revision_new);
        return LY_EINVAL;
    }

    /* collect correct plugin chain */
    if ((rc = yu_plg_collect(mod_old->ctx, mod_old->name, mod_old->revision, mod_old->compiled->features, revision_new,
            features_new, &plgs, &plg_count))) {
        goto cleanup;
    }

    /* create the context and load the new YANG module */
    if ((rc = yu_ctx_load(mod_old->name, plgs[plg_count - 1]->revision_new, plgs[plg_count - 1]->features_new,
            ly_ctx_get_searchdirs(mod_old->ctx), plgs[plg_count - 1]->imports_new, ctx_new, mod_new))) {
        goto cleanup;
    }

    /* ietf-yang-schema-comparison needs to be in the context */
    if (!ly_ctx_get_module_implemented(*ctx_new, "ietf-yang-schema-comparison") &&
            !ly_ctx_load_module(*ctx_new, "ietf-yang-schema-comparison", NULL, NULL)) {
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

cleanup:
    free(plgs);
    return rc;
}

/**
 * @brief Update a data node.
 *
 * @param[in] snode1 Old schema node, if any.
 * @param[in] data1 Old data tree.
 * @param[in] node1 Old data node, if any.
 * @param[in] snode2 New schema node, if any.
 * @param[in] parent2 New data node parent, if any.
 * @param[in] schema_diff Schema comparison data tree.
 * @param[in] rules Array of rules to use.
 * @param[in] rule_paths Array of compiled paths of @p rules.
 * @param[in] rule_count Count of @p rules and @p rule_paths.
 * @param[in,out] user_data Pointer to arbitrary user data.
 * @param[out] node2 Created new node.
 * @param[out] gen_path_match Set if a generic schema (without predicates) data path rule matched.
 * @return LY_ERR value.
 */
static LY_ERR
yu_update_node(const struct lysc_node *snode1, const struct lyd_node *data1, const struct lyd_node *node1,
        const struct lysc_node *snode2, struct lyd_node *parent2, const struct lyd_node *schema_diff,
        const struct lyu_plg_rule *rules, struct ly_path **rule_paths, uint32_t rule_count, const void **user_data,
        struct lyd_node **node2, ly_bool *gen_path_match)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lyd_node *sdiff_node;
    struct lyd_node *match1;
    const struct lysc_node *snode;
    const struct lyu_plg_rule *rule = NULL;
    char *path = NULL, *path2 = NULL;
    uint32_t i;

    *node2 = NULL;
    *gen_path_match = 0;

    snode = snode1 ? snode1 : snode2;

    /* generate the node schema path */
    path = lysc_path(snode, LYSC_PATH_DATA, NULL, 0);

    /* find a change in schema-comparison data */
    if (asprintf(&path2, "/ietf-yang-schema-comparison:schema-comparison/schema[1]/node-comparison[node='%s']", path) == -1) {
        LOGMEM(snode->module->ctx);
        rc = LY_EMEM;
        goto cleanup;
    }
    lyd_find_path(schema_diff, path2, 0, (struct lyd_node **)&sdiff_node);

    /* find a matching rule */
    for (i = 0; i < rule_count; ++i) {
        if (node1) {
            /* evaluate compiled data path to check it is selected */
            ly_path_eval(rule_paths[i], data1, NULL, &match1);
            if (node1 == match1) {
                rule = &rules[i];
                break;
            }
        }

        if (!strcmp(rules[i].node_path, path)) {
            /* schema data path matched */
            rule = &rules[i];
            *gen_path_match = 1;
            break;
        }
    }

    if (rule) {
        /* apply the rule */
        if ((rc = rule->node_cb(data1, node1, snode1, parent2, snode2, sdiff_node, user_data, node2))) {
            goto cleanup;
        }
    } else if (node1 && snode2) {
        /* just make a copy */
        if ((rc = lyd_dup_single_to_ctx(node1, snode2->module->ctx, parent2, 0, node2))) {
            goto cleanup;
        }
    }

cleanup:
    free(path);
    free(path2);
    return rc;
}

/**
 * @brief Collect all siblings on a schema level.
 *
 * @param[in] mod Module to traverse, for top-level siblings.
 * @param[in] sparent Schema parent to traverse, for nested siblings.
 * @param[out] siblings Array of siblings.
 * @param[out] sibling_count Count of @p siblings.
 * @return LY_ERR value.
 */
static LY_ERR
yu_getnext_collect(const struct lys_module *mod, const struct lysc_node *sparent, const struct lysc_node ***siblings,
        uint32_t *sibling_count)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysc_node *snode = NULL;
    void *mem;

    *siblings = NULL;
    *sibling_count = 0;

    if (!sparent && !mod) {
        /* node not present in old or new schema */
        goto cleanup;
    }

    /* collect all the siblings */
    while ((snode = lys_getnext(snode, sparent, mod ? mod->compiled : NULL, 0))) {
        mem = realloc(*siblings, (*sibling_count + 1) * sizeof **siblings);
        LY_CHECK_ERR_GOTO(!mem, LOGMEM(snode->module->ctx); rc = LY_EMEM, cleanup);
        *siblings = mem;

        (*siblings)[*sibling_count] = snode;
        ++(*sibling_count);
    }

cleanup:
    if (rc) {
        free(*siblings);
        *siblings = NULL;
        *sibling_count = 0;
    }

    return rc;
}

/**
 * @brief Update all siblings on a level, recursively.
 *
 * @param[in] mod1 Old module.
 * @param[in] sparent1 Old schema parent.
 * @param[in] data1 Old data.
 * @param[in] parent1 Old data parent.
 * @param[in] mod2 New module.
 * @param[in] sparent2 New schema parent.
 * @param[in] schema_diff Schema comparison generated data.
 * @param[in] rules Array of rules.
 * @param[in] rule_paths Array of compiled paths of @p rules.
 * @param[in] rule_count Count of @p rules and @p rule_paths.
 * @param[in,out] parent2 New data parent.
 * @param[in,out].first2 New forst top-level data sibling.
 * @return LY_ERR value.
 */
static LY_ERR
yu_update_data_r(const struct lys_module *mod1, const struct lysc_node *sparent1, const struct lyd_node *data1,
        const struct lyd_node *parent1, const struct lys_module *mod2, const struct lysc_node *sparent2,
        const struct lyd_node *schema_diff, const struct lyu_plg_rule *rules, struct ly_path **rule_paths,
        uint32_t rule_count, struct lyd_node *parent2, struct lyd_node **first2)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysc_node **siblings1 = NULL, **siblings2 = NULL, *snode1, *snode2;
    const struct lyd_node *node1;
    struct lyd_node *node2;
    uint32_t i, sibling1_count = 0, sibling2_count = 0;
    ly_bool schema_repeat, gen_path_match;
    const void *user_data = NULL;

    /* collect siblings */
    if ((rc = yu_getnext_collect(mod1, sparent1, &siblings1, &sibling1_count))) {
        goto cleanup;
    }
    if ((rc = yu_getnext_collect(mod2, sparent2, &siblings2, &sibling2_count))) {
        goto cleanup;
    }

    while (sibling1_count || sibling2_count) {
        snode1 = NULL;
        snode2 = NULL;
        i = 0;

        /* find matching siblings or just use whatever are left unmatched */
        if (sibling1_count) {
            snode1 = siblings1[0];
        }
        if (sibling2_count) {
            if (snode1) {
                for (; i < sibling2_count; ++i) {
                    if (!strcmp(snode1->name, siblings2[i]->name)) {
                        snode2 = siblings2[i];
                        break;
                    }
                }
            } else {
                snode2 = siblings2[0];
            }
        }

        /* find the first data node, if any */
        node1 = NULL;
        if (snode1 && data1) {
            lyd_find_sibling_val(parent1 ? lyd_child(parent1) : data1, snode1, NULL, 0, (struct lyd_node **)&node1);
        }

        do {
            /* update the node */
            if ((rc = yu_update_node(snode1, data1, node1, snode2, parent2, schema_diff, rules, rule_paths, rule_count,
                    &user_data, &node2, &gen_path_match))) {
                goto cleanup;
            }

            if (node2) {
                /* insert if needed */
                if (!parent2) {
                    if ((rc = lyd_insert_sibling(*first2, node2, first2))) {
                        goto cleanup;
                    }
                }

                /* recursively for all the children */
                if ((rc = yu_update_data_r(NULL, snode1, data1, node1, NULL, snode2, schema_diff, rules, rule_paths,
                        rule_count, node2, first2))) {
                    goto cleanup;
                }
            }

            if (node1 && node1->next && (node1->next->schema == node1->schema)) {
                /* update next old instance */
                node1 = node1->next;
                schema_repeat = 1;
            } else if (gen_path_match && (snode2->nodetype & (LYS_LIST | LYS_LEAFLIST)) && (node1 || node2)) {
                /* create new instance */
                node1 = NULL;
                schema_repeat = 1;
            } else {
                /* schema node(s) processed, break */
                schema_repeat = 0;
            }
        } while (schema_repeat);

        /* remove processed nodes */
        if (snode1) {
            --sibling1_count;
            if (sibling1_count) {
                memmove(siblings1, siblings1 + 1, sibling1_count * sizeof *siblings1);
            }
        }
        if (snode2) {
            --sibling2_count;
            if (i < sibling2_count) {
                memmove(siblings2 + i, siblings2 + i + 1, (sibling2_count - i) * sizeof *siblings2);
            }
        }
    }

cleanup:
    free(siblings1);
    free(siblings2);
    return rc;
}

/**
 * @brief Compile paths of rules.
 *
 * @param[in] ctx Contex to use for logging.
 * @param[in] rules Array of rules to use.
 * @param[out] rule_paths Compiled paths of @p rules.
 * @param[out] rule_count Count of @p rules and @p rule_paths.
 * @return LY_ERR value.
 */
static LY_ERR
yu_compile_rule_paths(const struct ly_ctx *ctx, const struct lyu_plg_rule *rules, struct ly_path ***rule_paths,
        uint32_t *rule_count)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyxp_expr *expr = NULL;
    uint32_t i;

    /* count the rules */
    for (i = 0; rules[i].node_path; ++i) {}
    *rule_count = i;

    /* prepare the array */
    *rule_paths = calloc(*rule_count, sizeof **rule_paths);
    LY_CHECK_ERR_GOTO(!*rule_paths, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    for (i = 0; i < *rule_count; ++i) {
        /* parse */
        if ((rc = ly_path_parse(ctx, NULL, rules[i].node_path, 0, 0, LY_PATH_BEGIN_ABSOLUTE, LY_PATH_PREFIX_FIRST,
                LY_PATH_PRED_SIMPLE, &expr))) {
            goto cleanup;
        }

        /* compile */
        if ((rc = ly_path_compile(ctx, NULL, expr, LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY, 0, LY_VALUE_JSON, NULL,
                &(*rule_paths)[i]))) {
            goto cleanup;
        }

        /* next iter */
        lyxp_expr_free(expr);
        expr = NULL;
    }

cleanup:
    lyxp_expr_free(expr);
    if (rc && *rule_paths) {
        for (i = 0; i < *rule_count; ++i) {
            ly_path_free((*rule_paths)[i]);
        }
        free(*rule_paths);
        *rule_paths = NULL;
    }
    if (rc) {
        *rule_count = 0;
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_update(const struct lys_module *mod_old, const struct lyd_node *data_old, const struct lys_module *mod_new,
        struct lyd_node **data_new)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *schema_diff = NULL;
    struct lyu_plg **plgs = NULL;
    uint32_t i, j, rule_count = 0, plg_count = 0;
    struct ly_ctx *ctx1 = NULL, *ctx2 = NULL;
    struct lys_module *mod1, *mod2;
    struct lyd_node *data1 = NULL, *data2 = NULL;
    struct ly_path **rule_paths = NULL;

    LY_CHECK_ARG_RET(NULL, mod_old, mod_old->implemented, mod_new, mod_new->implemented, data_new, LY_EINVAL);
    if (strcmp(mod_old->name, mod_new->name)) {
        LOGERR(mod_new->ctx, LY_EINVAL, "Old module \"%s\" and new module \"%s\" mismatch.", mod_old->name, mod_new->name);
        return LY_EINVAL;
    } else if (!mod_old->revision) {
        LOGERR(mod_new->ctx, LY_EINVAL, "Old module \"%s\" without a revision.", mod_old->name);
        return LY_EINVAL;
    } else if (!mod_new->revision) {
        LOGERR(mod_new->ctx, LY_EINVAL, "New module \"%s\" without a revision.", mod_new->name);
        return LY_EINVAL;
    } else if (strcmp(mod_old->revision, mod_new->revision) >= 0) {
        LOGERR(mod_new->ctx, LY_EINVAL, "Old module \"%s\" revision %s not earlier than new module \"%s\" revision %s.",
                mod_old->name, mod_old->revision, mod_new->name, mod_new->revision);
        return LY_EINVAL;
    }

    *data_new = NULL;

    /* collect plugins */
    if ((rc = yu_plg_collect(mod_old->ctx, mod_old->name, mod_old->revision, mod_old->compiled->features,
            mod_new->revision, mod_new->compiled->features, &plgs, &plg_count))) {
        goto cleanup;
    }

    /* first iter */
    ctx1 = mod_old->ctx;
    mod1 = (struct lys_module *)mod_old;
    data1 = (struct lyd_node *)data_old;

    for (i = 0; i < plg_count; ++i) {
        /* prepare the new context */
        if (i == plg_count - 1) {
            ctx2 = mod_new->ctx;
            mod2 = (struct lys_module *)mod_new;
        } else {
            if ((rc = yu_ctx_load(plgs[i]->module_name, plgs[i]->revision_new, plgs[i]->features_new,
                    ly_ctx_get_searchdirs(mod_old->ctx), plgs[i]->imports_new, &ctx2, &mod2))) {
                goto cleanup;
            }
        }

        /* compare the modules */
        if ((rc = lys_compare(ctx2, mod1, mod2, &schema_diff))) {
            goto cleanup;
        }

        /* compile the paths of all the rules */
        if ((rc = yu_compile_rule_paths(ctx2, plgs[i]->rules, &rule_paths, &rule_count))) {
            goto cleanup;
        }

        /* update the data */
        if ((rc = yu_update_data_r(mod1, NULL, data1, NULL, mod2, NULL, schema_diff, plgs[i]->rules, rule_paths,
                rule_count, NULL, &data2))) {
            goto cleanup;
        }

        /* validate the data */
        if ((rc = lyd_validate_module(&data2, mod2, 0, NULL))) {
            goto cleanup;
        }

        /* next iter */
        if (!i) {
            ctx1 = NULL;
            data1 = NULL;
        }

        lyd_free_siblings(schema_diff);
        schema_diff = NULL;
        for (j = 0; j < rule_count; ++j) {
            ly_path_free(rule_paths[j]);
        }
        free(rule_paths);
        rule_paths = NULL;
        rule_count = 0;

        lyd_free_siblings(data1);
        data1 = data2;
        data2 = NULL;
        mod1 = mod2;
        mod2 = NULL;
        ly_ctx_destroy(ctx1);
        ctx1 = ctx2;
        ctx2 = NULL;
    }

cleanup:
    lyd_free_siblings(schema_diff);
    for (j = 0; j < rule_count; ++j) {
        ly_path_free(rule_paths[j]);
    }
    free(rule_paths);
    free(plgs);

    if (rc) {
        if (data1 != data_old) {
            lyd_free_siblings(data1);
        }
        if (ctx1 != mod_old->ctx) {
            ly_ctx_destroy(ctx1);
        }
        lyd_free_siblings(data2);
        if (ctx2 != mod_new->ctx) {
            ly_ctx_destroy(ctx2);
        }
    } else {
        *data_new = data1;
    }
    return rc;
}

enum lyu_change {
    LYU_CREATE,
    LYU_DELETE,
    LYU_REPLACE,
    LYU_COPY
};

struct lyu_matches {
    struct lyu_match {
        char *path;
        enum lyu_change change;
        const char *old_val;
        int old_dflt;
        const char *new_val;
        int new_dflt;
    } *items;
    uint32_t count;
    uint32_t max_path_len;
};

/**
 * @brief Get string value of a change.
 *
 * @param[in] ch Change to use.
 * @return String value of @p ch.
 */
static const char *
yu_print_change2str(enum lyu_change ch)
{
    switch (ch) {
    case LYU_CREATE:
        return "CREATED";
    case LYU_DELETE:
        return "DELETED";
    case LYU_REPLACE:
        return "REPLACED BY";
    case LYU_COPY:
        return "COPIED";
    default:
        return NULL;
    }
}

/**
 * @brief Add a new match.
 *
 * @param[in] old Old matched node.
 * @param[in] new New matched node.
 * @param[in,out] matches Matches to add to.
 * @return LY_ERR value.
 */
static LY_ERR
yu_print_collect_add(const struct lyd_node *old, const struct lyd_node *new, struct lyu_matches *matches)
{
    struct lyu_match *match;
    const struct lyd_node *node;
    void *mem;
    uint32_t len;

    assert(old || new);

    node = old ? old : new;

    /* add a new match */
    mem = realloc(matches->items, (matches->count + 1) * sizeof *matches->items);
    LY_CHECK_ERR_RET(!mem, LOGMEM(LYD_CTX(node)), LY_EMEM);
    matches->items = mem;

    match = &matches->items[matches->count];
    memset(match, 0, sizeof *match);
    ++matches->count;

    /* path */
    if (node->schema->nodetype == LYS_LEAFLIST) {
        match->path = lyd_path(node, LYD_PATH_STD_NO_LAST_PRED, NULL, 0);
    } else {
        match->path = lyd_path(node, LYD_PATH_STD, NULL, 0);
    }

    /* change */
    if (!old) {
        match->change = LYU_CREATE;
    } else if (!new) {
        match->change = LYU_DELETE;
    } else if (lyd_compare_single(old, new, 0)) {
        match->change = LYU_REPLACE;
    } else {
        match->change = LYU_COPY;
    }

    /* old value */
    if ((match->change == LYU_DELETE) || (match->change == LYU_REPLACE)) {
        match->old_val = lyd_get_value(old);
        if (old->flags & LYD_DEFAULT) {
            match->old_dflt = 1;
        }
    }

    /* new value */
    if ((match->change == LYU_CREATE) || (match->change == LYU_REPLACE)) {
        match->new_val = lyd_get_value(new);
        if (new->flags & LYD_DEFAULT) {
            match->new_dflt = 1;
        }
    }

    /* max path length */
    len = strlen(match->path);
    if (len > matches->max_path_len) {
        matches->max_path_len = len;
    }

    return LY_SUCCESS;
}

/**
 * @brief Erase the collected matches.
 *
 * @param[in] matches Matches to erase.
 */
static void
yu_print_collect_erase(struct lyu_matches *matches)
{
    uint32_t i;

    for (i = 0; i < matches->count; ++i) {
        free(matches->items[i].path);
    }
    free(matches->items);

    memset(matches, 0, sizeof *matches);
}

/**
 * @brief Collect all the matching nodes (pairs) from the old and new sibling list, recursively.
 *
 * @param[in] ctx Context for logging.
 * @param[in] sibling1 First sibling list.
 * @param[in] sibling2 Second sibling list.
 * @param[in,out] matches Matches to add to.
 * @return LY_ERR value.
 */
static LY_ERR
yu_print_collect_r(const struct ly_ctx *ctx, const struct lyd_node *sibling1, const struct lyd_node *sibling2,
        struct lyu_matches *matches)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lyd_node *node1, *node2;

    LY_LIST_FOR(sibling1, node1) {
        /* find a matching sibling2 */
        node2 = NULL;
        if (node1->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            lyd_find_sibling_first(sibling2, node1, (struct lyd_node **)&node2);
        } else {
            lyd_find_sibling_val(sibling2, node1->schema, NULL, 0, (struct lyd_node **)&node2);
        }

        /* store it */
        LY_CHECK_GOTO(rc = yu_print_collect_add(node1, node2, matches), cleanup);

        /* recursive */
        LY_CHECK_GOTO(rc = yu_print_collect_r(ctx, lyd_child_no_keys(node1), lyd_child_no_keys(node2), matches), cleanup);
    }

    LY_LIST_FOR(sibling2, node2) {
        /* find a matching sibling1 */
        node1 = NULL;
        if (node2->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
            lyd_find_sibling_first(sibling1, node2, (struct lyd_node **)&node1);
        } else {
            lyd_find_sibling_val(sibling1, node2->schema, NULL, 0, (struct lyd_node **)&node1);
        }

        if (node1) {
            /* both nodes already stored */
            continue;
        }

        /* store it */
        LY_CHECK_GOTO(rc = yu_print_collect_add(node1, node2, matches), cleanup);

        /* recursive */
        LY_CHECK_GOTO(rc = yu_print_collect_r(ctx, NULL, lyd_child_no_keys(node2), matches), cleanup);
    }

cleanup:
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_update_print(const struct lyd_node *data_old, const struct lyd_node *data_new, FILE *out)
{
    LY_ERR rc = LY_SUCCESS;
    const struct ly_ctx *ctx;
    struct lyu_matches matches = {0};
    struct lyu_match *match;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, out, LY_EINVAL);

    if (!data_old && !data_new) {
        goto cleanup;
    }

    ctx = data_new ? LYD_CTX(data_new) : LYD_CTX(data_old);

    /* collect all the matching nodes */
    LY_CHECK_GOTO(rc = yu_print_collect_r(ctx, data_old, data_new, &matches), cleanup);

    /* print all the matches */
    for (i = 0; i < matches.count; ++i) {
        match = &matches.items[i];

        switch (match->change) {
        case LYU_CREATE:
            if (match->new_val) {
                fprintf(out, "%-*s  %s %s\"%s\"\n", (int)matches.max_path_len, match->path,
                        yu_print_change2str(match->change), match->new_dflt ? "DEFAULT " : "", match->new_val);
            } else {
                fprintf(out, "%-*s  %s\n", (int)matches.max_path_len, match->path, yu_print_change2str(match->change));
            }
            break;
        case LYU_DELETE:
            if (match->old_val) {
                fprintf(out, "%-*s  %s %s\"%s\"\n", (int)matches.max_path_len, match->path,
                        yu_print_change2str(match->change), match->old_dflt ? "DEFAULT " : "", match->old_val);
            } else {
                fprintf(out, "%-*s  %s\n", (int)matches.max_path_len, match->path, yu_print_change2str(match->change));
            }
            break;
        case LYU_REPLACE:
            fprintf(out, "%-*s  %s\"%s\" %s %s\"%s\"\n", (int)matches.max_path_len, match->path,
                    match->old_dflt ? "DEFAULT " : "", match->old_val, yu_print_change2str(match->change),
                    match->new_dflt ? "DEFAULT " : "", match->new_val);
            break;
        case LYU_COPY:
            fprintf(out, "%-*s  %s\n", (int)matches.max_path_len, match->path, yu_print_change2str(match->change));
            break;
        }
    }

cleanup:
    yu_print_collect_erase(&matches);
    return rc;
}
