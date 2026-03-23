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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libyang/libyang.h>

#include "ly_common.h"
#include "path.h"
#include "xpath.h"

/* generated */
#include "yang_update_plugins.h"

/**
 * @brief Check the collected plugins they are valid and correct.
 *
 * @param[in] ctx Context to use for logging.
 * @param[in] plgs Array of sorted plugins.
 * @param[in] plg_count Count of @p plgs.
 * @return LY_ERR value.
 */
static LY_ERR
yu_plg_check(const struct ly_ctx *ctx, struct lyu_plg **plgs, uint32_t plg_count)
{
    LY_ERR rc = LY_SUCCESS;
    uint32_t i;

    /* check the plugins */
    for (i = 0; i < plg_count; ++i) {
        if (!strcmp(plgs[i]->revision_old, plgs[i]->revision_new)) {
            LOGERR(ctx, LY_EINVAL, "Invalid plugin of \"%s\" with the same revision old and revision new \"%s\".",
                    plgs[i]->module_name, plgs[i]->revision_new);
            rc = LY_EINVAL;
            goto cleanup;
        }

        if (strcmp(plgs[i]->revision_old, plgs[i]->revision_new) > 0) {
            LOGERR(ctx, LY_EINVAL, "Invalid plugin of \"%s\" with newer revision old \"%s\" than revision new \"%s\".",
                    plgs[i]->module_name, plgs[i]->revision_old, plgs[i]->revision_new);
            rc = LY_EINVAL;
            goto cleanup;
        }

        if ((i < plg_count - 1) && !strcmp(plgs[i]->revision_old, plgs[i + 1]->revision_old) &&
                !strcmp(plgs[i]->revision_new, plgs[i + 1]->revision_new)) {
            LOGERR(ctx, LY_EINVAL, "Duplicate plugin of \"%s\" revision old \"%s\" and revision new\"%s\".",
                    plgs[i]->module_name, plgs[i]->revision_old, plgs[i]->revision_new);
            rc = LY_EINVAL;
            goto cleanup;
        }

        if ((i < plg_count - 1) && strcmp(plgs[i]->revision_new, plgs[i + 1]->revision_old)) {
            LOGERR(ctx, LY_EINVAL, "Plugin of \"%s\" revision new \"%s\" does not match the next plugin "
                    "revision old \"%s\".", plgs[i]->module_name, plgs[i]->revision_new, plgs[i + 1]->revision_old);
            rc = LY_EINVAL;
            goto cleanup;
        }
    }

cleanup:
    return rc;
}

/**
 * @brief qsort() plugin for sorting plugins of a module in an ascending order based on their revision_old.
 */
static int
yu_plg_qsort_cmp(const void *item1, const void *item2)
{
    struct lyu_plg *plg1, *plg2;

    plg1 = *(struct lyu_plg **)item1;
    plg2 = *(struct lyu_plg **)item2;

    assert(!strcmp(plg1->module_name, plg2->module_name));

    return strcmp(plg1->revision_old, plg2->revision_old);
}

/**
 * @brief Collect all plugins for a module from the compiled plugins.
 *
 * @param[in] ctx Context to use for logging.
 * @param[in] mod_name Module name.
 * @param[in] revision_old Module old (first) revision.
 * @param[out] plgs Array of found plugins.
 * @param[out] plg_count Count of @p plgs.
 * @return LY_ERR value.
 */
static LY_ERR
yu_plg_collect(const struct ly_ctx *ctx, const char *mod_name, const char *revision_old, struct lyu_plg ***plgs,
        uint32_t *plg_count)
{
    LY_ERR rc = LY_SUCCESS;
    void *mem;
    uint32_t i;

    *plgs = NULL;
    *plg_count = 0;

    /* collect all the plugins of the module */
    for (i = 0; i < sizeof lyu_plugins / sizeof *lyu_plugins; ++i) {
        /* module name */
        if (strcmp(mod_name, lyu_plugins[i]->module_name)) {
            continue;
        }

        /* add a plugin */
        mem = realloc(*plgs, (*plg_count + 1) * sizeof **plgs);
        LY_CHECK_ERR_GOTO(!mem, LOGMEM(ctx); rc = LY_EMEM, cleanup);
        *plgs = mem;

        (*plgs)[*plg_count] = lyu_plugins[i];
        ++(*plg_count);
    }
    if (!*plg_count) {
        LOGERR(ctx, LY_ENOTFOUND, "Failed to find a plugin for \"%s\" YANG module.", mod_name);
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

    /* sort based on revision old */
    qsort(*plgs, *plg_count, sizeof **plgs, yu_plg_qsort_cmp);

    /* check the plugins */
    if ((rc = yu_plg_check(ctx, *plgs, *plg_count))) {
        goto cleanup;
    }

    /* remove any older revision plugins */
    for (i = 0; (i < *plg_count) && (strcmp(revision_old, (*plgs)[i]->revision_old) > 0); ++i) {}
    if (i == *plg_count) {
        /* no plugins left but okay if the data have already been updated */
        if (!strcmp(revision_old, (*plgs)[*plg_count]->revision_new)) {
            LOGVRB("Data already in the latest revision \"%s\".", revision_old);
        } else {
            LOGERR(ctx, LY_ENOTFOUND, "Data in revision \"%s\" and the latest plugin revision new is \"%s\".",
                    revision_old, (*plgs)[*plg_count]->revision_new);
            rc = LY_ENOTFOUND;
            goto cleanup;
        }
    }
    if (i) {
        memmove(*plgs, (*plgs) + i, (*plg_count - i) * sizeof **plgs);
        *plg_count -= i;

        if (!*plg_count) {
            free(*plgs);
            *plgs = NULL;
        }
    }

    /* check the first plugin */
    if (strcmp(revision_old, (*plgs)[0]->revision_old)) {
        LOGERR(ctx, LY_ENOTFOUND, "Failed to find a plugin for \"%s\" with revision old \"%s\".", mod_name, revision_old);
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

cleanup:
    if (rc) {
        free(*plgs);
        *plgs = NULL;
        *plg_count = 0;
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_update_find_new(const struct lys_module *mod_old, struct ly_ctx *ctx_new, const char *revision,
        struct lys_module **mod_new)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyu_plg **plgs = NULL;
    uint32_t i, plg_count = 0;

    LY_CHECK_ARG_RET(NULL, mod_old, ctx_new, mod_new, LY_EINVAL);
    if (!mod_old->revision) {
        LOGERR(ctx_new, LY_EINVAL, "Old module \"%s\" without a revision.", mod_old->name);
        return LY_EINVAL;
    }

    /* ietf-yang-schema-comparison needs to be in the context */
    if (!ly_ctx_get_module_implemented(ctx_new, "ietf-yang-schema-comparison") &&
            !ly_ctx_load_module(ctx_new, "ietf-yang-schema-comparison", NULL, NULL)) {
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

    /* collect plugins */
    if ((rc = yu_plg_collect(ctx_new, mod_old->name, mod_old->revision, &plgs, &plg_count))) {
        goto cleanup;
    }

    if (revision) {
        /* check the target module plugin exists */
        for (i = 0; i < plg_count; ++i) {
            if (!strcmp(plgs[i]->revision_new, revision)) {
                break;
            }
        }
        if (i == plg_count) {
            LOGERR(ctx_new, LY_ENOTFOUND, "New module \"%s\" revision %s plugin not found.", mod_old->name, revision);
            rc = LY_ENOTFOUND;
            goto cleanup;
        }
    } else {
        /* just use the latest revision */
        revision = plgs[plg_count - 1]->revision_new;
    }

    /* load the new YANG module */
    *mod_new = ly_ctx_load_module(ctx_new, mod_old->name, revision, NULL);
    if (!*mod_new) {
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
 * @brief Create a new context with a specific revision of a module.
 *
 * @param[in] mod_name Module name.
 * @param[in] revision Module revision.
 * @param[in] search_dirs Search dirs to use.
 * @param[in] mod_clb Module import callback to use.
 * @param[in] mod_clb_data User data for @p mod_clb.
 * @param[out] mod Loaded module.
 * @param[out] ctx Created context.
 * @return LY_ERR value.
 */
static LY_ERR
yu_ctx_new(const char *mod_name, const char *revision, const char * const *search_dirs, ly_module_imp_clb mod_clb,
        void *mod_clb_data, const struct lys_module **mod, struct ly_ctx **ctx)
{
    LY_ERR rc = LY_SUCCESS;
    uint32_t i;

    *ctx = NULL;

    /* create the context */
    if ((rc = ly_ctx_new(NULL, 0, ctx))) {
        goto cleanup;
    }
    for (i = 0; search_dirs && search_dirs[i]; ++i) {
        ly_ctx_set_searchdir(*ctx, search_dirs[i]);
    }
    if (mod_clb) {
        ly_ctx_set_module_imp_clb(*ctx, mod_clb, mod_clb_data);
    }

    /* load ietf-yang-schema-comparison */
    if (!ly_ctx_load_module(*ctx, "ietf-yang-schema-comparison", NULL, NULL)) {
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

    /* load the main module */
    *mod = ly_ctx_load_module(*ctx, mod_name, revision, NULL);
    if (!*mod) {
        rc = LY_ENOTFOUND;
        goto cleanup;
    }

cleanup:
    if (rc) {
        ly_ctx_destroy(*ctx);
        *ctx = NULL;
    }
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
    const struct lys_module *mod1, *mod2;
    struct lyd_node *data1 = NULL, *data2 = NULL;
    struct ly_path **rule_paths = NULL;
    const char * const *search_dirs = NULL;
    ly_module_imp_clb mod_clb = NULL;
    void *mod_clb_data = NULL;

    LY_CHECK_ARG_RET(NULL, mod_old, mod_new, data_new, LY_EINVAL);
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
    if ((rc = yu_plg_collect(mod_old->ctx, mod_old->name, mod_old->revision, &plgs, &plg_count))) {
        goto cleanup;
    }

    /* check there is the new module */
    for (i = 0; i < plg_count; ++i) {
        if (!strcmp(plgs[i]->revision_new, mod_new->revision)) {
            /* found the last used plugin */
            plg_count = i + 1;
            break;
        }
    }
    if (i == plg_count) {
        LOGERR(mod_new->ctx, LY_ENOTFOUND, "New module \"%s\" revision %s plugin not found.", mod_new->name,
                mod_new->revision);
        return LY_ENOTFOUND;
    }

    /* collect context params */
    search_dirs = ly_ctx_get_searchdirs(mod_old->ctx);
    mod_clb = ly_ctx_get_module_imp_clb(mod_old->ctx, &mod_clb_data);

    /* first iter */
    ctx1 = mod_old->ctx;
    mod1 = mod_old;
    data1 = (struct lyd_node *)data_old;

    for (i = 0; i < plg_count; ++i) {
        /* prepare the new context */
        if (i == plg_count - 1) {
            ctx2 = mod_new->ctx;
            mod2 = mod_new;
        } else {
            if ((rc = yu_ctx_new(plgs[i]->module_name, plgs[i]->revision_new, search_dirs, mod_clb, mod_clb_data, &mod2,
                    &ctx2))) {
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
