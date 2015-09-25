/**
 * @file yang_library.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Static YANG ietf-yang-library implementation
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "parser.h"
#include "tree_internal.h"

static struct lyd_node *
ylib_name_space(struct ly_ctx *ctx, struct lys_node *name_node, const char *name)
{
    struct lyd_node_leaf_list *dleaf;

    dleaf = calloc(1, sizeof *dleaf);
    dleaf->prev = (struct lyd_node *)dleaf;
    dleaf->schema = name_node;

    dleaf->value_str = lydict_insert(ctx, name, 0);
    dleaf->value.string = dleaf->value_str;
    dleaf->value_type = LY_TYPE_STRING;

    return (struct lyd_node *)dleaf;
}

static struct lyd_node *
ylib_revision(struct ly_ctx *ctx, struct lys_node *revision_node, struct lys_revision *rev, uint8_t rev_size)
{
    int i, max = 0;
    struct lyd_node_leaf_list *dleaf;

    dleaf = calloc(1, sizeof *dleaf);
    dleaf->prev = (struct lyd_node *)dleaf;
    dleaf->schema = revision_node;

    if (rev_size) {
        for (i = 1; i < rev_size; ++i) {
            if (strcmp(rev[i].date, rev[max].date) > 0) {
                max = i;
            }
        }
        dleaf->value_str = lydict_insert(ctx, rev[max].date, 0);
    } else {
        dleaf->value_str = lydict_insert(ctx, "", 0);
    }
    dleaf->value.string = dleaf->value_str;
    dleaf->value_type = LY_TYPE_STRING;

    return (struct lyd_node *)dleaf;
}

static struct lyd_node *
ylib_schema(struct ly_ctx *ctx, struct lys_node *schema_node, const char *uri)
{
    struct lyd_node_leaf_list *dleaf;

    if (!uri) {
        return NULL;
    }

    dleaf = calloc(1, sizeof *dleaf);
    dleaf->prev = (struct lyd_node *)dleaf;
    dleaf->schema = schema_node;

    dleaf->value_str = lydict_insert(ctx, uri, 0);
    dleaf->value.string = dleaf->value_str;
    dleaf->value_type = LY_TYPE_STRING;

    return (struct lyd_node *)dleaf;
}

static struct lyd_node *
ylib_feature(struct ly_ctx *ctx, struct lys_node *feature_node, struct lys_module *mod)
{
    int i, j;
    struct lyd_node_leaf_list *dllist, *ret = NULL;

    /* module features */
    for (i = 0; i < mod->features_size; ++i) {
        if (!(mod->features[i].flags & LYS_FENABLED)) {
            continue;
        }
        dllist = calloc(1, sizeof *dllist);
        dllist->prev = (struct lyd_node *)dllist;
        dllist->schema = feature_node;

        dllist->value_str = lydict_insert(ctx, mod->features[i].name, 0);
        dllist->value.string = dllist->value_str;
        dllist->value_type = LY_TYPE_STRING;

        if (ret) {
            lyd_insert_after(ret->prev, (struct lyd_node *)dllist, LYD_OPT_STRICT);
        } else {
            ret = dllist;
        }
    }

    /* submodule features */
    for (i = 0; i < mod->inc_size; ++i) {
        for (j = 0; j < mod->inc[i].submodule->features_size; ++j) {
            if (!(mod->inc[i].submodule->features[j].flags & LYS_FENABLED)) {
                continue;
            }
            dllist = calloc(1, sizeof *dllist);
            dllist->schema = feature_node;
            dllist->prev = (struct lyd_node *)dllist;

            dllist->value_str = lydict_insert(ctx, mod->inc[i].submodule->features[j].name, 0);
            dllist->value.string = dllist->value_str;
            dllist->value_type = LY_TYPE_STRING;

            if (ret) {
                lyd_insert_after(ret->prev, (struct lyd_node *)dllist, LYD_OPT_STRICT);
            } else {
                ret = dllist;
            }
        }
    }

    return (struct lyd_node *)ret;
}

static struct lyd_node *
ylib_deviation(struct ly_ctx *ctx, struct lys_node *deviation_node, struct lys_module *mod, struct lys_module **modules, int mod_count)
{
    int i, j, k;
    struct lys_module *target_module;
    struct lyd_node *dnode;
    struct lyd_node *ret = NULL, *dlist, *dlast;
    struct lys_node *deviation_child;

    for (i = 0; i < mod_count; ++i) {
        for (k = 0; k < modules[i]->deviation_size; ++k) {
            if (modules[i]->deviation[k].target->module->type) {
                target_module = ((struct lys_submodule *)modules[i]->deviation[k].target->module)->belongsto;
            } else {
                target_module = modules[i]->deviation[k].target->module;
            }

            /* we found a module deviating our module */
            if (target_module == mod) {
                dlist = calloc(1, sizeof *dlist);
                dlist->prev = (struct lyd_node *)dlist;
                dlist->schema = deviation_node;

                deviation_child = NULL;
                while ((deviation_child = lys_getnext(deviation_child, deviation_node, NULL, 0))) {
                    dnode = NULL;

                    if (!strcmp(deviation_child->name, "name")) {
                        dnode = ylib_name_space(ctx, deviation_child, modules[i]->name);
                    } else if (!strcmp(deviation_child->name, "revision")) {
                        dnode = ylib_revision(ctx, deviation_child, modules[i]->rev, modules[i]->rev_size);
                    }

                    if (dnode) {
                        lyd_insert((struct lyd_node *)dlist, dnode, LYD_OPT_STRICT);
                    }
                }

                if (ret) {
                    lyd_insert_after((struct lyd_node *)dlast, (struct lyd_node *)dlist, LYD_OPT_STRICT);
                } else {
                    ret = dlist;
                }
                dlast = dlist;
            }
        }

        for (j = 0; j < modules[i]->inc_size; ++j) {
            for (k = 0; k < modules[i]->inc[j].submodule->deviation_size; ++k) {
                if (modules[i]->inc[j].submodule->deviation[k].target->module->type) {
                    target_module = ((struct lys_submodule *)
                                    modules[i]->inc[j].submodule->deviation[k].target->module)->belongsto;
                } else {
                    target_module = modules[i]->inc[j].submodule->deviation[k].target->module;
                }

                /* we found a submodule deviating our module */
                if (target_module == mod) {
                    dlist = calloc(1, sizeof *dlist);
                    dlist->prev = (struct lyd_node *)dlist;
                    dlist->schema = deviation_node;

                    deviation_child = NULL;
                    while ((deviation_child = lys_getnext(deviation_child, deviation_node, NULL, 0))) {
                        dnode = NULL;

                        if (!strcmp(deviation_child->name, "name")) {
                            dnode = ylib_name_space(ctx, deviation_child, modules[i]->inc[j].submodule->name);
                        } else if (!strcmp(deviation_child->name, "revision")) {
                            dnode = ylib_revision(ctx, deviation_child, modules[i]->inc[j].submodule->rev,
                                                  modules[i]->inc[j].submodule->rev_size);
                        }

                        if (dnode) {
                            lyd_insert((struct lyd_node *)dlist, dnode, LYD_OPT_STRICT);
                        }
                    }

                    if (ret) {
                        lyd_insert_after((struct lyd_node *)dlast, (struct lyd_node *)dlist, LYD_OPT_STRICT);
                    } else {
                        ret = dlist;
                    }
                    dlast = dlist;
                }
            }
        }
    }

    return (struct lyd_node *)ret;
}

static struct lyd_node *
ylib_conformance(struct ly_ctx *ctx, struct lys_node *conformance_node, int implemented)
{
    struct lyd_node_leaf_list *dleaf;

    dleaf = calloc(1, sizeof *dleaf);
    dleaf->prev = (struct lyd_node *)dleaf;
    dleaf->schema = conformance_node;

    if (implemented) {
        dleaf->value_str = lydict_insert(ctx, "implement", 0);
    } else {
        dleaf->value_str = lydict_insert(ctx, "import", 0);
    }
    dleaf->value.string = dleaf->value_str;
    dleaf->value_type = LY_TYPE_STRING;

    return (struct lyd_node *)dleaf;
}

static struct lyd_node *
ylib_submodules(struct ly_ctx *ctx, struct lys_node *submodules_node, struct lys_include *inc, uint8_t inc_size)
{
    int i;
    struct lys_node *submodule_node, *submodule_child;
    struct lyd_node *ret = NULL, *dnode;
    struct lyd_node *dlist = NULL;

    ret = calloc(1, sizeof *ret);
    ret->prev = ret;
    ret->schema = submodules_node;

    submodule_node = NULL;
    while ((submodule_node = lys_getnext(submodule_node, submodules_node, NULL, 0))) {
        if (!strcmp(submodule_node->name, "submodule")) {
            for (i = 0; i < inc_size; ++i) {
                dlist = calloc(1, sizeof *dlist);
                dlist->prev = (struct lyd_node *)dlist;
                dlist->schema = submodule_node;

                submodule_child = NULL;
                while ((submodule_child = lys_getnext(submodule_child, submodule_node, NULL, 0))) {
                    dnode = NULL;

                    if (!strcmp(submodule_child->name, "name")) {
                        dnode = ylib_name_space(ctx, submodule_child, inc[i].submodule->name);
                    } else if (!strcmp(submodule_child->name, "revision")) {
                        dnode = ylib_revision(ctx, submodule_child, inc[i].submodule->rev, inc[i].submodule->rev_size);
                    } else if (!strcmp(submodule_child->name, "schema")) {
                        dnode = ylib_schema(ctx, submodule_child, inc[i].submodule->uri);
                    }

                    if (dnode) {
                        lyd_insert((struct lyd_node *)dlist, dnode, LYD_OPT_STRICT);
                    }
                }

                lyd_insert(ret, (struct lyd_node *)dlist, LYD_OPT_STRICT);
            }
        }
    }

    return ret;
}

static struct lyd_node *
ylib_module_set_id(struct ly_ctx *ctx, struct lys_node *modules_set_id_node)
{
    struct lyd_node_leaf_list *dleaf;
    char id[8];

    dleaf = calloc(1, sizeof *dleaf);
    dleaf->prev = (struct lyd_node *)dleaf;
    dleaf->schema = modules_set_id_node;

    sprintf(id, "%u", ctx->models.module_set_id);
    dleaf->value_str = lydict_insert(ctx, id, 0);
    dleaf->value.string = dleaf->value_str;
    dleaf->value_type = LY_TYPE_STRING;

    return (struct lyd_node *)dleaf;
}

API struct lyd_node *
ly_ctx_info(struct ly_ctx *ctx)
{
    int i;
    struct lys_module *mod;
    struct lys_node *modules_child, *module_child;
    struct lyd_node *root, *dnode;
    struct lyd_node *dlist = NULL;

    mod = ly_ctx_get_module(ctx, "ietf-yang-library", NULL);
    if (!mod) {
        mod = lyp_search_file(ctx, NULL, "ietf-yang-library", NULL);
    }
    if (!mod || !mod->data || strcmp(mod->data->next->name, "modules")) {
        return NULL;
    }

    root = calloc(1, sizeof *root);
    if (!root) {
        return NULL;
    }

    root->prev = root;
    root->schema = mod->data->next;

    modules_child = NULL;
    while ((modules_child = lys_getnext(modules_child, mod->data->next, NULL, 0))) {
        if (!strcmp(modules_child->name, "module")) {
            for (i = 0; i < ctx->models.used; ++i) {
                dlist = calloc(1, sizeof *dlist);
                dlist->prev = (struct lyd_node *)dlist;
                dlist->schema = modules_child;

                module_child = NULL;
                while ((module_child = lys_getnext(module_child, modules_child, NULL, 0))) {
                    dnode = NULL;

                    if (!strcmp(module_child->name, "name")) {
                        dnode = ylib_name_space(ctx, module_child, ctx->models.list[i]->name);
                    } else if (!strcmp(module_child->name, "revision")) {
                        dnode = ylib_revision(ctx, module_child, ctx->models.list[i]->rev, ctx->models.list[i]->rev_size);
                    } else if (!strcmp(module_child->name, "schema")) {
                        dnode = ylib_schema(ctx, module_child, ctx->models.list[i]->uri);
                    } else if (!strcmp(module_child->name, "namespace")) {
                        dnode = ylib_name_space(ctx, module_child, ctx->models.list[i]->ns);
                    } else if (!strcmp(module_child->name, "feature")) {
                        dnode = ylib_feature(ctx, module_child, ctx->models.list[i]);
                    } else if (!strcmp(module_child->name, "deviation")) {
                        if (ctx->models.list[i]->deviated) {
                            dnode = ylib_deviation(ctx, module_child, ctx->models.list[i], ctx->models.list, ctx->models.used);
                        }
                    } else if (!strcmp(module_child->name, "conformance")) {
                        dnode = ylib_conformance(ctx, module_child, ctx->models.list[i]->implemented);
                    } else if (!strcmp(module_child->name, "submodules")) {
                        if (ctx->models.list[i]->inc_size) {
                            dnode = ylib_submodules(ctx, module_child, ctx->models.list[i]->inc, ctx->models.list[i]->inc_size);
                        }
                    }

                    if (dnode) {
                        lyd_insert((struct lyd_node *)dlist, dnode, LYD_OPT_STRICT);
                    }
                }

                lyd_insert(root, (struct lyd_node *)dlist, LYD_OPT_STRICT);
            }
        }
    }

    assert(dlist);

    dnode = NULL;
    modules_child = NULL;
    while ((modules_child = lys_getnext(modules_child, mod->data->next, NULL, 0))) {
        if (!strcmp(modules_child->name, "module-set-id")) {
            dnode = ylib_module_set_id(ctx, modules_child);
        }
    }

    assert(dnode);
    lyd_insert(root, dnode, LYD_OPT_STRICT);

    return root;
}
