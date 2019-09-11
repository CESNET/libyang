/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "context.h"
#include "dict.h"
#include "log.h"
#include "set.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

API const struct lysc_node *
lys_getnext(const struct lysc_node *last, const struct lysc_node *parent, const struct lysc_module *module, int options)
{
    const struct lysc_node *next = NULL;
    struct lysc_node **snode;
    int action_flag = 0, notif_flag = 0;
    const struct lysc_action *actions;
    const struct lysc_notif *notifs;
    unsigned int u;

    LY_CHECK_ARG_RET(NULL, parent || module, NULL);

next:
    if (!last) {
        /* first call */

        /* get know where to start */
        if (parent) {
            /* schema subtree */
            if (parent->nodetype == LYS_CHOICE && (options & LYS_GETNEXT_WITHCASE)) {
                if (((struct lysc_node_choice*)parent)->cases) {
                    next = last = (const struct lysc_node*)&((struct lysc_node_choice*)parent)->cases[0];
                }
            } else {
                snode = lysc_node_children_p(parent, (options & LYS_GETNEXT_OUTPUT) ? LYS_CONFIG_R : LYS_CONFIG_W);
                /* do not return anything if the node does not have any children */
                if (snode && *snode) {
                    next = last = *snode;
                }
            }
        } else {
            /* top level data */
            next = last = module->data;
        }
        if (!next) {
            /* try to get action or notification */
            goto repeat;
        }
        /* test if the next can be returned */
        goto check;

    } else if (last->nodetype == LYS_ACTION) {
        action_flag = 1;
        if (last->parent) {
            actions = lysc_node_actions(last->parent);
        } else {
            actions = module->rpcs;
        }
        LY_ARRAY_FOR(actions, u) {
            if (&actions[u] == (struct lysc_action*)last) {
                break;
            }
        }
        if (u + 1 < LY_ARRAY_SIZE(actions)) {
            next = (struct lysc_node*)(&actions[u + 1]);
        }
        goto repeat;
    } else if (last->nodetype == LYS_NOTIF) {
        action_flag = notif_flag = 1;
        if (last->parent) {
            notifs = lysc_node_notifs(last->parent);
        } else {
            notifs = module->notifs;
        }
        LY_ARRAY_FOR(notifs, u) {
            if (&notifs[u] == (struct lysc_notif*)last) {
                break;
            }
        }
        if (u + 1 < LY_ARRAY_SIZE(notifs)) {
            next = (struct lysc_node*)(&notifs[u + 1]);
        }
        goto repeat;
    }

    next = last->next;
repeat:
    if (next && parent && parent->nodetype == LYS_CASE && next->parent != parent) {
        /* inside case (as an explicit parent, not when diving into it from choice),
         * limit the list of children only to the specific case */
        next = NULL;
    }
    if (!next) {
        /* possibly go back to parent */
        if (last && last->parent != parent) {
            last = last->parent;
            goto next;
        } else if (!action_flag) {
            action_flag = 1;
            next = parent ? (struct lysc_node*)lysc_node_actions(parent) : (struct lysc_node*)module->rpcs;
        } else if (!notif_flag) {
            notif_flag = 1;
            next = parent ? (struct lysc_node*)lysc_node_notifs(parent) : (struct lysc_node*)module->notifs;
        } else {
            return NULL;
        }
        goto repeat;
    }
check:
    switch (next->nodetype) {
    case LYS_ACTION:
    case LYS_NOTIF:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
    case LYS_LIST:
    case LYS_LEAFLIST:
    case LYS_CASE:
        break;
    case LYS_CONTAINER:
        if (!(((struct lysc_node_container *)next)->flags & LYS_PRESENCE) && (options & LYS_GETNEXT_INTONPCONT)) {
            if (((struct lysc_node_container *)next)->child) {
                /* go into */
                next = ((struct lysc_node_container *)next)->child;
            } else {
                next = next->next;
            }
            goto repeat;
        }
        break;
    case LYS_CHOICE:
        if (options & LYS_GETNEXT_WITHCHOICE) {
            return next;
        } else if ((options & LYS_GETNEXT_NOCHOICE) || !((struct lysc_node_choice *)next)->cases) {
            next = next->next;
        } else {
            /* go into */
            if (options & LYS_GETNEXT_WITHCASE) {
                next = (struct lysc_node*)((struct lysc_node_choice *)next)->cases;
            } else {
                next = ((struct lysc_node_choice *)next)->cases->child;
            }
        }
        goto repeat;
    default:
        /* we should not be here */
        LOGINT(module ? module->mod->ctx : parent->module->ctx);
        return NULL;
    }

    if (!(options & LYS_GETNEXT_NOSTATECHECK)) {
        /* check if the node is disabled by if-feature */
        if (lys_is_disabled(next, 0)) {
            next = next->next;
            goto repeat;
        }
    }
    return next;
}

API const struct lysc_node *
lys_child(const struct lysc_node *parent, const struct lys_module *module,
          const char *name, size_t name_len, uint16_t nodetype, int options)
{
    const struct lysc_node *node = NULL;

    LY_CHECK_ARG_RET(NULL, module, name, NULL);
    if (!nodetype) {
        nodetype = 0xffff;
    }

    while ((node = lys_getnext(node, parent, module->compiled, options))) {
        if (!(node->nodetype & nodetype)) {
            continue;
        }
        if (node->module != module) {
            continue;
        }

        if (name_len) {
            if (!strncmp(node->name, name, name_len) && !node->name[name_len]) {
                return node;
            }
        } else {
            if (!strcmp(node->name, name)) {
                return node;
            }
        }
    }
    return NULL;
}



API char *
lysc_path(struct lysc_node *node, LY_PATH_TYPE pathtype, char *buffer, size_t buflen)
{
    struct lysc_node *iter;
    char *path = NULL;
    int len = 0;

    LY_CHECK_ARG_RET(NULL, node, NULL);
    if (buffer) {
        LY_CHECK_ARG_RET(node->module->ctx, buflen > 1, NULL);
    }

    switch (pathtype) {
    case LY_PATH_LOG:
        for (iter = node; iter && len >= 0; iter = iter->parent) {
            char *s = buffer ? strdup(buffer) : path;
            char *id;

            switch (iter->nodetype) {
            case LYS_USES:
                asprintf(&id, "{uses='%s'}", iter->name);
                break;
            case LYS_GROUPING:
                asprintf(&id, "{grouping='%s'}", iter->name);
                break;
            case LYS_AUGMENT:
                asprintf(&id, "{augment='%s'}", iter->name);
                break;
            default:
                id = strdup(iter->name);
                break;
            }

            if (!iter->parent || iter->parent->module != iter->module) {
                /* print prefix */
                if (buffer) {
                    len = snprintf(buffer, buflen, "/%s:%s%s", iter->module->name, id, s ? s : "");
                } else {
                    len = asprintf(&path, "/%s:%s%s", iter->module->name, id, s ? s : "");
                }
            } else {
                /* prefix is the same as in parent */
                if (buffer) {
                    len = snprintf(buffer, buflen, "/%s%s", id, s ? s : "");
                } else {
                    len = asprintf(&path, "/%s%s", id, s ? s : "");
                }
            }
            free(s);
            free(id);

            if (buffer && buflen <= (size_t)len) {
                /* not enough space in buffer */
                break;
            }
        }

        if (len < 0) {
            free(path);
            path = NULL;
        } else if (len == 0) {
            if (buffer) {
                strcpy(buffer, "/");
            } else {
                path = strdup("/");
            }
        }
        break;
    }

    if (buffer) {
        return buffer;
    } else {
        return path;
    }
}

API int
lysc_feature_value(const struct lysc_feature *feature)
{
    LY_CHECK_ARG_RET(NULL, feature, -1);
    return feature->flags & LYS_FENABLED ? 1 : 0;
}

uint8_t
lysc_iff_getop(uint8_t *list, int pos)
{
    uint8_t *item;
    uint8_t mask = 3, result;

    assert(pos >= 0);

    item = &list[pos / 4];
    result = (*item) & (mask << 2 * (pos % 4));
    return result >> 2 * (pos % 4);
}

static int
lysc_iffeature_value_(const struct lysc_iffeature *iff, int *index_e, int *index_f)
{
    uint8_t op;
    int a, b;

    op = lysc_iff_getop(iff->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        /* resolve feature */
        return lysc_feature_value(iff->features[(*index_f)++]);
    case LYS_IFF_NOT:
        /* invert result */
        return lysc_iffeature_value_(iff, index_e, index_f) ? 0 : 1;
    case LYS_IFF_AND:
    case LYS_IFF_OR:
        a = lysc_iffeature_value_(iff, index_e, index_f);
        b = lysc_iffeature_value_(iff, index_e, index_f);
        if (op == LYS_IFF_AND) {
            return a && b;
        } else { /* LYS_IFF_OR */
            return a || b;
        }
    }

    return 0;
}

API int
lysc_iffeature_value(const struct lysc_iffeature *iff)
{
    int index_e = 0, index_f = 0;

    LY_CHECK_ARG_RET(NULL, iff, -1);

    if (iff->expr) {
        return lysc_iffeature_value_(iff, &index_e, &index_f);
    }
    return 0;
}

/**
 * @brief Enable/Disable the specified feature in the module.
 *
 * If the feature is already set to the desired value, LY_SUCCESS is returned.
 * By changing the feature, also all the feature which depends on it via their
 * if-feature statements are again evaluated (disabled if a if-feature statemen
 * evaluates to false).
 *
 * @param[in] mod Module where to set (search for) the feature.
 * @param[in] name Name of the feature to set. Asterisk ('*') can be used to
 * set all the features in the module.
 * @param[in] value Desired value of the feature: 1 (enable) or 0 (disable).
 * @return LY_ERR value.
 */
static LY_ERR
lys_feature_change(const struct lys_module *mod, const char *name, int value)
{
    int all = 0;
    unsigned int u, changed_count, disabled_count;
    struct lysc_feature *f, **df;
    struct lysc_iffeature *iff;
    struct ly_set *changed;
    struct ly_ctx *ctx = mod->ctx; /* shortcut */

    if (!strcmp(name, "*")) {
        /* enable all */
        all = 1;
    }

    if (!mod->compiled) {
        LOGERR(ctx, LY_EINVAL, "Module \"%s\" is not implemented so all its features are permanently disabled without a chance to change it.",
               mod->name);
        return LY_EINVAL;
    }
    if (!mod->compiled->features) {
        if (all) {
            /* no feature to enable */
            return LY_SUCCESS;
        }
        LOGERR(ctx, LY_EINVAL, "Unable to switch feature since the module \"%s\" has no features.", mod->name);
        return LY_EINVAL;
    }

    changed = ly_set_new();
    changed_count = 0;

run:
    for (disabled_count = u = 0; u < LY_ARRAY_SIZE(mod->compiled->features); ++u) {
        f = &mod->compiled->features[u];
        if (all || !strcmp(f->name, name)) {
            if ((value && (f->flags & LYS_FENABLED)) || (!value && !(f->flags & LYS_FENABLED))) {
                if (all) {
                    /* skip already set features */
                    continue;
                } else {
                    /* feature already set correctly */
                    ly_set_free(changed, NULL);
                    return LY_SUCCESS;
                }
            }

            if (value) { /* enable */
                /* check referenced features if they are enabled */
                LY_ARRAY_FOR(f->iffeatures, struct lysc_iffeature, iff) {
                    if (!lysc_iffeature_value(iff)) {
                        if (all) {
                            ++disabled_count;
                            goto next;
                        } else {
                            LOGERR(ctx, LY_EDENIED,
                                   "Feature \"%s\" cannot be enabled since it is disabled by its if-feature condition(s).",
                                   f->name);
                            ly_set_free(changed, NULL);
                            return LY_EDENIED;
                        }
                    }
                }
                /* enable the feature */
                f->flags |= LYS_FENABLED;
            } else { /* disable */
                /* disable the feature */
                f->flags &= ~LYS_FENABLED;
            }

            /* remember the changed feature */
            ly_set_add(changed, f, LY_SET_OPT_USEASLIST);

            if (!all) {
                /* stop in case changing a single feature */
                break;
            }
        }
next:
        ;
    }

    if (!all && !changed->count) {
        LOGERR(ctx, LY_EINVAL, "Feature \"%s\" not found in module \"%s\".", name, mod->name);
        ly_set_free(changed, NULL);
        return LY_EINVAL;
    }

    if (value && all && disabled_count) {
        if (changed_count == changed->count) {
            /* no change in last run -> not able to enable all ... */
            /* ... print errors */
            for (u = 0; disabled_count && u < LY_ARRAY_SIZE(mod->compiled->features); ++u) {
                if (!(mod->compiled->features[u].flags & LYS_FENABLED)) {
                    LOGERR(ctx, LY_EDENIED,
                           "Feature \"%s\" cannot be enabled since it is disabled by its if-feature condition(s).",
                           mod->compiled->features[u].name);
                    --disabled_count;
                }
            }
            /* ... restore the original state */
            for (u = 0; u < changed->count; ++u) {
                f = changed->objs[u];
                /* re-disable the feature */
                f->flags &= ~LYS_FENABLED;
            }

            ly_set_free(changed, NULL);
            return LY_EDENIED;
        } else {
            /* we did some change in last run, try it again */
            changed_count = changed->count;
            goto run;
        }
    }

    /* reflect change(s) in the dependent features */
    for (u = 0; u < changed->count; ++u) {
        /* If a dependent feature is enabled, it can be now changed by the change (to false) of the value of
         * its if-feature statements. The reverse logic, automatically enable feature when its feature is enabled
         * is not done - by default, features are disabled and must be explicitely enabled. */
        f = changed->objs[u];
        LY_ARRAY_FOR(f->depfeatures, struct lysc_feature*, df) {
            if (!((*df)->flags & LYS_FENABLED)) {
                /* not enabled, nothing to do */
                continue;
            }
            /* check the feature's if-features which could change by the previous change of our feature */
            LY_ARRAY_FOR((*df)->iffeatures, struct lysc_iffeature, iff) {
                if (!lysc_iffeature_value(iff)) {
                    /* the feature must be disabled now */
                    (*df)->flags &= ~LYS_FENABLED;
                    /* add the feature into the list of changed features */
                    ly_set_add(changed, *df, LY_SET_OPT_USEASLIST);
                    break;
                }
            }
        }
    }

    ly_set_free(changed, NULL);
    return LY_SUCCESS;
}

API LY_ERR
lys_feature_enable(const struct lys_module *module, const char *feature)
{
    LY_CHECK_ARG_RET(NULL, module, feature, LY_EINVAL);

    return lys_feature_change((struct lys_module*)module, feature, 1);
}

API LY_ERR
lys_feature_disable(const struct lys_module *module, const char *feature)
{
    LY_CHECK_ARG_RET(NULL, module, feature, LY_EINVAL);

    return lys_feature_change((struct lys_module*)module, feature, 0);
}

API int
lys_feature_value(const struct lys_module *module, const char *feature)
{
    struct lysc_feature *f;
    struct lysc_module *mod;
    unsigned int u;

    LY_CHECK_ARG_RET(NULL, module, module->compiled, feature, -1);
    mod = module->compiled;

    /* search for the specified feature */
    for (u = 0; u < LY_ARRAY_SIZE(mod->features); ++u) {
        f = &mod->features[u];
        if (!strcmp(f->name, feature)) {
            if (f->flags & LYS_FENABLED) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    /* feature definition not found */
    return -1;
}

API const struct lysc_iffeature *
lys_is_disabled(const struct lysc_node *node, int recursive)
{
    unsigned int u;

    LY_CHECK_ARG_RET(NULL, node, NULL);

    while(node) {
        if (node->nodetype & LYS_CHOICE) {
            return NULL;
        }

        if (node->iffeatures) {
            /* check local if-features */
            LY_ARRAY_FOR(node->iffeatures, u) {
                if (!lysc_iffeature_value(&node->iffeatures[u])) {
                    return &node->iffeatures[u];
                }
            }
        }

        if (!recursive) {
            return NULL;
        }

        /* go through parents */
        node = node->parent;
    }
    return NULL;
}

LY_ERR
lys_set_implemented_internal(struct lys_module *mod, uint8_t value)
{
    struct lys_module *m;

    LY_CHECK_ARG_RET(NULL, mod, LY_EINVAL);

    if (mod->implemented) {
        return LY_SUCCESS;
    }

    /* we have module from the current context */
    m = ly_ctx_get_module_implemented(mod->ctx, mod->name);
    if (m) {
        if (m != mod) {
            /* check collision with other implemented revision */
            LOGERR(mod->ctx, LY_EDENIED, "Module \"%s\" is present in the context in other implemented revision (%s).",
                   mod->name, mod->revision ? mod->revision : "module without revision");
            return LY_EDENIED;
        } else {
            /* mod is already implemented */
            return LY_SUCCESS;
        }
    }

    /* mark the module implemented, check for collision was already done */
    mod->implemented = value;

    /* compile the schema */
    LY_CHECK_RET(lys_compile(mod, LYSC_OPT_INTERNAL));

    return LY_SUCCESS;
}

API LY_ERR
lys_set_implemented(struct lys_module *mod)
{
    return lys_set_implemented_internal(mod, 1);
}

struct lysp_submodule *
lys_parse_mem_submodule(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, struct lys_parser_ctx *main_ctx,
                        LY_ERR (*custom_check)(struct ly_ctx*, struct lysp_module*, struct lysp_submodule*, void*), void *check_data)
{
    LY_ERR ret = LY_EINVAL;
    struct lysp_submodule *submod = NULL, *latest_sp;
    struct lys_parser_ctx *context = NULL;
    struct yin_parser_ctx *yin_context = NULL;

    LY_CHECK_ARG_RET(ctx, ctx, data, NULL);

    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_submodule(&yin_context, ctx, main_ctx, data, &submod);
        context = (struct lys_parser_ctx *)yin_context;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_submodule(&context, ctx, main_ctx, data, &submod);
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        break;
    }
    LY_CHECK_RET(ret, NULL);

    /* make sure that the newest revision is at position 0 */
    lysp_sort_revisions(submod->revs);

    if (custom_check) {
        LY_CHECK_GOTO(custom_check((*context).ctx, NULL, submod, check_data), error);
    }

    /* decide the latest revision */
    latest_sp = ly_ctx_get_submodule((*context).ctx, submod->belongsto, submod->name, NULL);
    if (latest_sp) {
        if (submod->revs) {
            if (!latest_sp->revs) {
                /* latest has no revision, so mod is anyway newer */
                submod->latest_revision = latest_sp->latest_revision;
                latest_sp->latest_revision = 0;
            } else {
                if (strcmp(submod->revs[0].date, latest_sp->revs[0].date) > 0) {
                    submod->latest_revision = latest_sp->latest_revision;
                    latest_sp->latest_revision = 0;
                }
            }
        }
    } else {
        submod->latest_revision = 1;
    }

    /* remap possibly changed and reallocated typedefs and groupings list back to the main context */
    memcpy(&main_ctx->tpdfs_nodes, &(*context).tpdfs_nodes, sizeof main_ctx->tpdfs_nodes);
    memcpy(&main_ctx->grps_nodes, &(*context).grps_nodes, sizeof main_ctx->grps_nodes);

    if (format == LYS_IN_YANG) {
        lys_parser_ctx_free(context);
    } else {
        yin_parser_ctx_free(yin_context);
    }
    return submod;

error:
    lysp_submodule_free(ctx, submod);
    if (format == LYS_IN_YANG) {
        lys_parser_ctx_free(context);
    } else {
        yin_parser_ctx_free(yin_context);
    }
    return NULL;
}

struct lys_module *
lys_parse_mem_module(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, int implement,
                     LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
                     void *check_data)
{
    struct lys_module *mod = NULL, *latest, *mod_dup;
    struct lysp_import *imp;
    struct lysp_include *inc;
    LY_ERR ret = LY_EINVAL;
    unsigned int u, i;
    struct lys_parser_ctx *context = NULL;
    struct yin_parser_ctx *yin_context = NULL;

    LY_CHECK_ARG_RET(ctx, ctx, data, NULL);

    mod = calloc(1, sizeof *mod);
    LY_CHECK_ERR_RET(!mod, LOGMEM(ctx), NULL);
    mod->ctx = ctx;

    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_module(&yin_context, data, mod);
        context = (struct lys_parser_ctx *)yin_context;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_module(&context, data, mod);
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        break;
    }
    LY_CHECK_ERR_RET(ret, lys_module_free(mod, NULL), NULL);

    /* make sure that the newest revision is at position 0 */
    lysp_sort_revisions(mod->parsed->revs);
    if (mod->parsed->revs) {
        mod->revision = lydict_insert(ctx, mod->parsed->revs[0].date, 0);
    }

    if (custom_check) {
        LY_CHECK_GOTO(custom_check(ctx, mod->parsed, NULL, check_data), error);
    }

    if (implement) {
        /* mark the loaded module implemented */
        if (ly_ctx_get_module_implemented(ctx, mod->name)) {
            LOGERR(ctx, LY_EDENIED, "Module \"%s\" is already implemented in the context.", mod->name);
            goto error;
        }
        mod->implemented = 1;
    }

    /* check for duplicity in the context */
    mod_dup = (struct lys_module*)ly_ctx_get_module(ctx, mod->name, mod->revision);
    if (mod_dup) {
        if (mod_dup->parsed) {
            /* error */
            if (mod->parsed->revs) {
                LOGERR(ctx, LY_EEXIST, "Module \"%s\" of revision \"%s\" is already present in the context.",
                       mod->name, mod->parsed->revs[0].date);
            } else {
                LOGERR(ctx, LY_EEXIST, "Module \"%s\" with no revision is already present in the context.",
                       mod->name);
            }
            goto error;
        } else {
            /* add the parsed data to the currently compiled-only module in the context */
            mod_dup->parsed = mod->parsed;
            mod_dup->parsed->mod = mod_dup;
            mod->parsed = NULL;
            lys_module_free(mod, NULL);
            mod = mod_dup;
            goto finish_parsing;
        }
    }

#if 0
    /* hack for NETCONF's edit-config's operation attribute. It is not defined in the schema, but since libyang
     * implements YANG metadata (annotations), we need its definition. Because the ietf-netconf schema is not the
     * internal part of libyang, we cannot add the annotation into the schema source, but we do it here to have
     * the anotation definitions available in the internal schema structure. There is another hack in schema
     * printers to do not print this internally added annotation. */
    if (ly_strequal(mod->name, "ietf-netconf", 0)) {
        if (lyp_add_ietf_netconf_annotations(mod)) {
            lys_free(mod, NULL, 1, 1);
            return NULL;
        }
    }
#endif

    if (!mod->implemented) {
        /* pre-compile features and extension definitions of the module */
        LY_CHECK_GOTO(lys_feature_precompile(NULL, ctx, mod, mod->parsed->features, &mod->off_features), error);
        LY_CHECK_GOTO(lys_extension_precompile(NULL, ctx, mod, mod->parsed->extensions, &mod->off_extensions), error);
    }

    /* decide the latest revision */
    latest = (struct lys_module*)ly_ctx_get_module_latest(ctx, mod->name);
    if (latest) {
        if (mod->revision) {
            if (!latest->revision) {
                /* latest has no revision, so mod is anyway newer */
                mod->latest_revision = latest->latest_revision;
                latest->latest_revision = 0;
            } else if (strcmp(mod->revision, latest->revision) > 0) {
                mod->latest_revision = latest->latest_revision;
                latest->latest_revision = 0;
            }
        }
    } else {
        mod->latest_revision = 1;
    }

    /* add into context */
    ly_set_add(&ctx->list, mod, LY_SET_OPT_USEASLIST);

finish_parsing:
    /* resolve imports */
    mod->parsed->parsing = 1;
    LY_ARRAY_FOR(mod->parsed->imports, u) {
        imp = &mod->parsed->imports[u];
        if (!imp->module && lysp_load_module(ctx, imp->name, imp->rev[0] ? imp->rev : NULL, 0, 0, &imp->module)) {
            goto error_ctx;
        }
        /* check for importing the same module twice */
        for (i = 0; i < u; ++i) {
            if (imp->module == mod->parsed->imports[i].module) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Single revision of the module \"%s\" referred twice.", imp->name);
                goto error_ctx;
            }
        }
    }
    LY_ARRAY_FOR(mod->parsed->includes, u) {
        inc = &mod->parsed->includes[u];
        if (!inc->submodule && lysp_load_submodule(context, mod->parsed, inc)) {
            goto error_ctx;
        }
        if (!mod->implemented) {
            /* pre-compile features and extension definitions of the module */
            LY_CHECK_GOTO(lys_feature_precompile(NULL, ctx, mod, inc->submodule->features, &mod->off_features), error);
            LY_CHECK_GOTO(lys_extension_precompile(NULL, ctx, mod, mod->parsed->extensions, &mod->off_extensions), error);
        }
    }
    mod->parsed->parsing = 0;

    /* check name collisions - typedefs and TODO groupings */
    LY_CHECK_GOTO(lysp_check_typedefs(context, mod->parsed), error_ctx);

    if (format == LYS_IN_YANG) {
        lys_parser_ctx_free(context);
    } else {
        yin_parser_ctx_free(yin_context);
    }
    return mod;

error_ctx:
    ly_set_rm(&ctx->list, mod, NULL);
error:
    lys_module_free(mod, NULL);
    ly_set_erase(&context->tpdfs_nodes, NULL);
    if (format == LYS_IN_YANG) {
        lys_parser_ctx_free(context);
    } else {
        yin_parser_ctx_free(yin_context);
    }

    return NULL;
}

API struct lys_module *
lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format)
{
    struct lys_module *mod;

    mod = lys_parse_mem_module(ctx, data, format, 1, NULL, NULL);
    LY_CHECK_RET(!mod, NULL);

    if (lys_compile(mod, 0)) {
        ly_set_rm(&ctx->list, mod, NULL);
        lys_module_free(mod, NULL);
        return NULL;
    }
    return mod;
}

static void
lys_parse_set_filename(struct ly_ctx *ctx, const char **filename, int fd)
{
    char path[PATH_MAX];

#ifdef __APPLE__
    if (fcntl(fd, F_GETPATH, path) != -1) {
        *filename = lydict_insert(ctx, path, 0);
    }
#else
    int len;
    char proc_path[32];

    /* get URI if there is /proc */
    sprintf(proc_path, "/proc/self/fd/%d", fd);
    if ((len = readlink(proc_path, path, PATH_MAX - 1)) > 0) {
        *filename = lydict_insert(ctx, path, len);
    }
#endif
}

void *
lys_parse_fd_(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, int implement, struct lys_parser_ctx *main_ctx,
                    LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
                    void *check_data)
{
    void *result;
    struct lys_module *mod = NULL;
    struct lysp_submodule *submod = NULL;
    size_t length;
    char *addr;

    LY_CHECK_ARG_RET(ctx, ctx, NULL);
    if (fd < 0) {
        LOGARG(ctx, fd);
        return NULL;
    }

    LY_CHECK_RET(ly_mmap(ctx, fd, &length, (void **)&addr), NULL);
    if (!addr) {
        LOGERR(ctx, LY_EINVAL, "Empty schema file.");
        return NULL;
    }

    if (main_ctx) {
        result = submod = lys_parse_mem_submodule(ctx, addr, format, main_ctx, custom_check, check_data);
    } else {
        result = mod = lys_parse_mem_module(ctx, addr, format, implement, custom_check, check_data);
        if (mod && implement && lys_compile(mod, 0)) {
            ly_set_rm(&ctx->list, mod, NULL);
            lys_module_free(mod, NULL);
            result = mod = NULL;
        }
    }
    ly_munmap(addr, length);

    if (mod && !mod->filepath) {
        lys_parse_set_filename(ctx, &mod->filepath, fd);
    } else if (submod && !submod->filepath) {
        lys_parse_set_filename(ctx, &submod->filepath, fd);
    }

    return result;
}

struct lys_module *
lys_parse_fd_module(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, int implement,
                    LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
                    void *check_data)
{
    return (struct lys_module*)lys_parse_fd_(ctx, fd, format, implement, NULL, custom_check, check_data);
}

struct lysp_submodule *
lys_parse_fd_submodule(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, struct lys_parser_ctx *main_ctx,
                       LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
                       void *check_data)
{
    assert(main_ctx);
    return (struct lysp_submodule*)lys_parse_fd_(ctx, fd, format, 0, main_ctx, custom_check, check_data);
}

API struct lys_module *
lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    return lys_parse_fd_module(ctx, fd, format, 1, NULL, NULL);
}

struct lys_module *
lys_parse_path_(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format, int implement,
                LY_ERR (*custom_check)(struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data), void *check_data)
{
    int fd;
    struct lys_module *mod;
    const char *rev, *dot, *filename;
    size_t len;

    LY_CHECK_ARG_RET(ctx, ctx, path, NULL);

    fd = open(path, O_RDONLY);
    LY_CHECK_ERR_RET(fd == -1, LOGERR(ctx, LY_ESYS, "Opening file \"%s\" failed (%s).", path, strerror(errno)), NULL);

    mod = lys_parse_fd_module(ctx, fd, format, implement, custom_check, check_data);
    close(fd);
    LY_CHECK_RET(!mod, NULL);

    /* check that name and revision match filename */
    filename = strrchr(path, '/');
    if (!filename) {
        filename = path;
    } else {
        filename++;
    }
    rev = strchr(filename, '@');
    dot = strrchr(filename, '.');

    /* name */
    len = strlen(mod->name);
    if (strncmp(filename, mod->name, len) ||
            ((rev && rev != &filename[len]) || (!rev && dot != &filename[len]))) {
        LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, mod->name);
    }
    if (rev) {
        len = dot - ++rev;
        if (!mod->parsed->revs || len != 10 || strncmp(mod->parsed->revs[0].date, rev, len)) {
            LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                   mod->parsed->revs ? mod->parsed->revs[0].date : "none");
        }
    }

    if (!mod->filepath) {
        /* store URI */
        char rpath[PATH_MAX];
        if (realpath(path, rpath) != NULL) {
            mod->filepath = lydict_insert(ctx, rpath, 0);
        } else {
            mod->filepath = lydict_insert(ctx, path, 0);
        }
    }

    return mod;
}

API struct lys_module *
lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format)
{
    return lys_parse_path_(ctx, path, format, 1, NULL, NULL);
}

API LY_ERR
lys_search_localfile(const char * const *searchpaths, int cwd, const char *name, const char *revision,
                     char **localfile, LYS_INFORMAT *format)
{
    size_t len, flen, match_len = 0, dir_len;
    int i, implicit_cwd = 0, ret = EXIT_FAILURE;
    char *wd, *wn = NULL;
    DIR *dir = NULL;
    struct dirent *file;
    char *match_name = NULL;
    LYS_INFORMAT format_aux, match_format = 0;
    struct ly_set *dirs;
    struct stat st;

    LY_CHECK_ARG_RET(NULL, localfile, LY_EINVAL);

    /* start to fill the dir fifo with the context's search path (if set)
     * and the current working directory */
    dirs = ly_set_new();
    if (!dirs) {
        LOGMEM(NULL);
        return EXIT_FAILURE;
    }

    len = strlen(name);
    if (cwd) {
        wd = get_current_dir_name();
        if (!wd) {
            LOGMEM(NULL);
            goto cleanup;
        } else {
            /* add implicit current working directory (./) to be searched,
             * this directory is not searched recursively */
            if (ly_set_add(dirs, wd, 0) == -1) {
                goto cleanup;
            }
            implicit_cwd = 1;
        }
    }
    if (searchpaths) {
        for (i = 0; searchpaths[i]; i++) {
            /* check for duplicities with the implicit current working directory */
            if (implicit_cwd && !strcmp(dirs->objs[0], searchpaths[i])) {
                implicit_cwd = 0;
                continue;
            }
            wd = strdup(searchpaths[i]);
            if (!wd) {
                LOGMEM(NULL);
                goto cleanup;
            } else if (ly_set_add(dirs, wd, 0) == -1) {
                goto cleanup;
            }
        }
    }
    wd = NULL;

    /* start searching */
    while (dirs->count) {
        free(wd);
        free(wn); wn = NULL;

        dirs->count--;
        wd = (char *)dirs->objs[dirs->count];
        dirs->objs[dirs->count] = NULL;
        LOGVRB("Searching for \"%s\" in %s.", name, wd);

        if (dir) {
            closedir(dir);
        }
        dir = opendir(wd);
        dir_len = strlen(wd);
        if (!dir) {
            LOGWRN(NULL, "Unable to open directory \"%s\" for searching (sub)modules (%s).", wd, strerror(errno));
        } else {
            while ((file = readdir(dir))) {
                if (!strcmp(".", file->d_name) || !strcmp("..", file->d_name)) {
                    /* skip . and .. */
                    continue;
                }
                free(wn);
                if (asprintf(&wn, "%s/%s", wd, file->d_name) == -1) {
                    LOGMEM(NULL);
                    goto cleanup;
                }
                if (stat(wn, &st) == -1) {
                    LOGWRN(NULL, "Unable to get information about \"%s\" file in \"%s\" when searching for (sub)modules (%s)",
                           file->d_name, wd, strerror(errno));
                    continue;
                }
                if (S_ISDIR(st.st_mode) && (dirs->count || !implicit_cwd)) {
                    /* we have another subdirectory in searchpath to explore,
                     * subdirectories are not taken into account in current working dir (dirs->set.g[0]) */
                    if (ly_set_add(dirs, wn, 0) == -1) {
                        goto cleanup;
                    }
                    /* continue with the next item in current directory */
                    wn = NULL;
                    continue;
                } else if (!S_ISREG(st.st_mode)) {
                    /* not a regular file (note that we see the target of symlinks instead of symlinks */
                    continue;
                }

                /* here we know that the item is a file which can contain a module */
                if (strncmp(name, file->d_name, len) ||
                        (file->d_name[len] != '.' && file->d_name[len] != '@')) {
                    /* different filename than the module we search for */
                    continue;
                }

                /* get type according to filename suffix */
                flen = strlen(file->d_name);
                if (!strcmp(&file->d_name[flen - 5], ".yang")) {
                    format_aux = LYS_IN_YANG;
                /* TODO YIN parser
                } else if (!strcmp(&file->d_name[flen - 4], ".yin")) {
                    format_aux = LYS_IN_YIN;
                */
                } else {
                    /* not supportde suffix/file format */
                    continue;
                }

                if (revision) {
                    /* we look for the specific revision, try to get it from the filename */
                    if (file->d_name[len] == '@') {
                        /* check revision from the filename */
                        if (strncmp(revision, &file->d_name[len + 1], strlen(revision))) {
                            /* another revision */
                            continue;
                        } else {
                            /* exact revision */
                            free(match_name);
                            match_name = wn;
                            wn = NULL;
                            match_len = dir_len + 1 + len;
                            match_format = format_aux;
                            goto success;
                        }
                    } else {
                        /* continue trying to find exact revision match, use this only if not found */
                        free(match_name);
                        match_name = wn;
                        wn = NULL;
                        match_len = dir_len + 1 +len;
                        match_format = format_aux;
                        continue;
                    }
                } else {
                    /* remember the revision and try to find the newest one */
                    if (match_name) {
                        if (file->d_name[len] != '@' ||
                                lysp_check_date(NULL, &file->d_name[len + 1], flen - (format_aux == LYS_IN_YANG ? 5 : 4) - len - 1, NULL)) {
                            continue;
                        } else if (match_name[match_len] == '@' &&
                                (strncmp(&match_name[match_len + 1], &file->d_name[len + 1], LY_REV_SIZE - 1) >= 0)) {
                            continue;
                        }
                        free(match_name);
                    }

                    match_name = wn;
                    wn = NULL;
                    match_len = dir_len + 1 + len;
                    match_format = format_aux;
                    continue;
                }
            }
        }
    }

success:
    (*localfile) = match_name;
    match_name = NULL;
    if (format) {
        (*format) = match_format;
    }
    ret = EXIT_SUCCESS;

cleanup:
    free(wn);
    free(wd);
    if (dir) {
        closedir(dir);
    }
    free(match_name);
    ly_set_free(dirs, free);

    return ret;
}

