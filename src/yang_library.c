#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "parser.h"

static struct ly_mnode *
ylib_get_next_sibling(struct ly_mnode *siblings, struct ly_mnode *prev)
{
    struct ly_mnode *sibling, *mnode;
    int found = 0;

    LY_TREE_FOR(siblings, sibling) {
        if (sibling->nodetype == LY_NODE_GROUPING) {
            continue;
        }

        if (sibling->nodetype == LY_NODE_USES) {
            mnode = ylib_get_next_sibling(sibling->child, (found ? NULL : prev));
            if (mnode) {
                return mnode;
            }
            continue;
        }

        if (found || !prev) {
            return sibling;
        }

        if (prev == sibling) {
            found = 1;
        }
    }

    return NULL;
}

static void
ylib_append_children(struct lyd_node *parent, struct lyd_node *child)
{
    struct lyd_node *parent_last_child;

    if (!parent || !child) {
        return;
    }

    if (!parent->child) {
        parent->child = child;
        return;
    }

    parent_last_child = parent->child->prev;
    parent->child->prev = child->prev;

    parent_last_child->next = child;
    child->prev = parent_last_child;
}

static void
ylib_append_list(struct lyd_node_list *sibling, struct lyd_node_list *list)
{
    struct lyd_node *sibling_last;
    struct lyd_node_list *sibling_last_list;

    if (!sibling || !list) {
        return;
    }

    sibling_last = sibling->prev;
    sibling->prev = list->prev;
    sibling_last->next = (struct lyd_node *)list;
    list->prev = sibling_last->next;

    sibling_last_list = sibling->lprev;
    sibling->lprev = list->lprev;
    sibling_last_list->lnext = list;
    list->lprev = sibling_last_list->lnext;
}

static void
ylib_append_llist(struct lyd_node_leaflist *sibling, struct lyd_node_leaflist *llist)
{
    struct lyd_node *sibling_last;
    struct lyd_node_leaflist *sibling_last_llist;

    if (!sibling || !llist) {
        return;
    }

    sibling_last = sibling->prev;
    sibling->prev = llist->prev;
    sibling_last->next = (struct lyd_node *)llist;
    llist->prev = sibling_last->next;

    sibling_last_llist = sibling->lprev;
    sibling->lprev = llist->lprev;
    sibling_last_llist->lnext = llist;
    llist->lprev = sibling_last_llist->lnext;
}

static struct lyd_node *
ylib_name_space(struct ly_ctx *ctx, struct ly_mnode *name_node, const char *name)
{
    struct lyd_node_leaf *dleaf;

    dleaf = calloc(1, sizeof *dleaf);
    dleaf->prev = (struct lyd_node *)dleaf;
    dleaf->schema = name_node;

    dleaf->value_str = lydict_insert(ctx, name, 0);
    dleaf->value.string = dleaf->value_str;
    dleaf->value_type = LY_TYPE_STRING;

    return (struct lyd_node *)dleaf;
}

static struct lyd_node *
ylib_revision(struct ly_ctx *ctx, struct ly_mnode *revision_node, struct ly_revision *rev, uint8_t rev_size)
{
    int i, max = 0;
    struct lyd_node_leaf *dleaf;

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
ylib_schema(struct ly_mnode *schema_node)
{
    struct lyd_node_leaf *dleaf;

    dleaf = calloc(1, sizeof *dleaf);
    dleaf->prev = (struct lyd_node *)dleaf;
    dleaf->schema = schema_node;

    /* TODO */
    dleaf->value_str = NULL;
    dleaf->value.string = dleaf->value_str;
    dleaf->value_type = LY_TYPE_STRING;

    return (struct lyd_node *)dleaf;
}

static struct lyd_node *
ylib_feature(struct ly_ctx *ctx, struct ly_mnode *feature_node, struct ly_module *mod)
{
    int i, j;
    struct lyd_node_leaflist *dllist, *ret = NULL;

    /* module features */
    for (i = 0; i < mod->features_size; ++i) {
        if (!(mod->features[i].flags & LY_NODE_FENABLED)) {
            continue;
        }
        dllist = calloc(1, sizeof *dllist);
        dllist->prev = (struct lyd_node *)dllist;
        dllist->lprev = dllist;
        dllist->schema = feature_node;

        dllist->value_str = lydict_insert(ctx, mod->features[i].name, 0);
        dllist->value.string = dllist->value_str;
        dllist->value_type = LY_TYPE_STRING;

        if (ret) {
            ylib_append_llist(ret, dllist);
        } else {
            ret = dllist;
        }
    }

    /* submodule features */
    for (i = 0; i < mod->inc_size; ++i) {
        for (j = 0; j < mod->inc[i].submodule->features_size; ++j) {
            if (!(mod->inc[i].submodule->features[j].flags & LY_NODE_FENABLED)) {
                continue;
            }
            dllist = calloc(1, sizeof *dllist);
            dllist->schema = feature_node;
            dllist->prev = (struct lyd_node *)dllist;
            dllist->lprev = dllist;

            dllist->value_str = lydict_insert(ctx, mod->inc[i].submodule->features[j].name, 0);
            dllist->value.string = dllist->value_str;
            dllist->value_type = LY_TYPE_STRING;

            if (ret) {
                ylib_append_llist(ret, dllist);
            } else {
                ret = dllist;
            }
        }
    }

    return (struct lyd_node *)ret;
}

static struct lyd_node *
ylib_deviation(struct ly_ctx *ctx, struct ly_mnode *deviation_node, struct ly_module *mod, struct ly_module **modules, int mod_count)
{
    int i, j, k;
    struct ly_module *target_module;
    struct lyd_node *dnode;
    struct lyd_node_list *ret = NULL, *dlist;
    struct ly_mnode *deviation_child;

    for (i = 0; i < mod_count; ++i) {
        for (k = 0; k < modules[i]->deviation_size; ++k) {
            if (modules[i]->deviation[k].target->module->type) {
                target_module = ((struct ly_submodule *)modules[i]->deviation[k].target->module)->belongsto;
            } else {
                target_module = modules[i]->deviation[k].target->module;
            }

            /* we found a module deviating our module */
            if (target_module == mod) {
                dlist = calloc(1, sizeof *dlist);
                dlist->prev = (struct lyd_node *)dlist;
                dlist->lprev = dlist;
                dlist->schema = deviation_node;

                deviation_child = NULL;
                while ((deviation_child = ylib_get_next_sibling(deviation_node->child, deviation_child))) {
                    dnode = NULL;

                    if (!strcmp(deviation_child->name, "name")) {
                        dnode = ylib_name_space(ctx, deviation_child, modules[i]->name);
                    } else if (!strcmp(deviation_child->name, "revision")) {
                        dnode = ylib_revision(ctx, deviation_child, modules[i]->rev, modules[i]->rev_size);
                    }

                    if (dnode) {
                        ylib_append_children((struct lyd_node *)dlist, dnode);
                    }
                }

                if (ret) {
                    ylib_append_list(ret, dlist);
                } else {
                    ret = dlist;
                }
            }
        }

        for (j = 0; j < modules[i]->inc_size; ++j) {
            for (k = 0; k < modules[i]->inc[j].submodule->deviation_size; ++k) {
                if (modules[i]->inc[j].submodule->deviation[k].target->module->type) {
                    target_module = ((struct ly_submodule *)
                                    modules[i]->inc[j].submodule->deviation[k].target->module)->belongsto;
                } else {
                    target_module = modules[i]->inc[j].submodule->deviation[k].target->module;
                }

                /* we found a submodule deviating our module */
                if (target_module == mod) {
                    dlist = calloc(1, sizeof *dlist);
                    dlist->prev = (struct lyd_node *)dlist;
                    dlist->lprev = dlist;
                    dlist->schema = deviation_node;

                    deviation_child = NULL;
                    while ((deviation_child = ylib_get_next_sibling(deviation_node->child, deviation_child))) {
                        dnode = NULL;

                        if (!strcmp(deviation_child->name, "name")) {
                            dnode = ylib_name_space(ctx, deviation_child, modules[i]->inc[j].submodule->name);
                        } else if (!strcmp(deviation_child->name, "revision")) {
                            dnode = ylib_revision(ctx, deviation_child, modules[i]->inc[j].submodule->rev,
                                                  modules[i]->inc[j].submodule->rev_size);
                        }

                        if (dnode) {
                            ylib_append_children((struct lyd_node *)dlist, dnode);
                        }
                    }

                    if (ret) {
                        ylib_append_list(ret, dlist);
                    } else {
                        ret = dlist;
                    }
                }
            }
        }
    }

    return (struct lyd_node *)ret;
}

static struct lyd_node *
ylib_conformance(struct ly_ctx *ctx, struct ly_mnode *conformance_node, int implemented)
{
    struct lyd_node_leaf *dleaf;

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
ylib_submodules(struct ly_ctx *ctx, struct ly_mnode *submodules_node, struct ly_include *inc, uint8_t inc_size)
{
    int i;
    struct ly_mnode *submodule_node, *submodule_child;
    struct lyd_node *ret = NULL, *dnode;
    struct lyd_node_list *dsubmodule = NULL, *dlist;

    ret = calloc(1, sizeof *ret);
    ret->prev = ret;
    ret->schema = submodules_node;

    submodule_node = NULL;
    while ((submodule_node = ylib_get_next_sibling(submodules_node->child, submodule_node))) {
        if (!strcmp(submodule_node->name, "submodule")) {
            for (i = 0; i < inc_size; ++i) {
                dlist = calloc(1, sizeof *dlist);
                dlist->prev = (struct lyd_node *)dlist;
                dlist->lprev = dlist;
                dlist->schema = submodule_node;

                submodule_child = NULL;
                while ((submodule_child = ylib_get_next_sibling(submodule_node->child, submodule_child))) {
                    dnode = NULL;

                    if (!strcmp(submodule_child->name, "name")) {
                        dnode = ylib_name_space(ctx, submodule_child, inc[i].submodule->name);
                    } else if (!strcmp(submodule_child->name, "revision")) {
                        dnode = ylib_revision(ctx, submodule_child, inc[i].submodule->rev, inc[i].submodule->rev_size);
                    } else if (!strcmp(submodule_child->name, "schema")) {
                        dnode = ylib_schema(submodule_child);
                    }

                    if (dnode) {
                        ylib_append_children((struct lyd_node *)dlist, dnode);
                    }
                }

                if (dsubmodule) {
                    ylib_append_list(dsubmodule, dlist);
                } else {
                    dsubmodule = dlist;
                }
            }
        }
    }

    ret->child = (struct lyd_node *)dsubmodule;

    return ret;
}

static struct lyd_node *
ylib_module_set_id(struct ly_ctx *ctx, struct ly_mnode *modules_set_id_node)
{
    struct lyd_node_leaf *dleaf;
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
ly_ylib_get(struct ly_ctx *ctx)
{
    int i;
    struct ly_module *mod;
    struct ly_mnode *modules_child, *module_child;
    struct lyd_node *root, *dnode;
    struct lyd_node_list *dlist, *dmodule = NULL;

    mod = ly_ctx_get_module(ctx, "ietf-yang-library", NULL);
    if (!mod) {
        mod = lyp_search_file(ctx, NULL, "ietf-yang-library", NULL);
    }
    if (!mod || !mod->data || strcmp(mod->data->name, "modules")) {
        return NULL;
    }

    root = calloc(1, sizeof *root);
    if (!root) {
        return NULL;
    }

    root->prev = root;
    root->schema = mod->data;

    modules_child = NULL;
    while ((modules_child = ylib_get_next_sibling(mod->data->next->child, modules_child))) {
        if (!strcmp(modules_child->name, "module")) {
            for (i = 0; i < ctx->models.used; ++i) {
                if (ctx->models.list[i]->type) {
                    /* skip submodules, they will be a part
                     * of their parent
                     */
                    continue;
                }

                dlist = calloc(1, sizeof *dlist);
                dlist->prev = (struct lyd_node *)dlist;
                dlist->lprev = dlist;
                dlist->schema = modules_child;

                module_child = NULL;
                while ((module_child = ylib_get_next_sibling(modules_child->child, module_child))) {
                    dnode = NULL;

                    if (!strcmp(module_child->name, "name")) {
                        dnode = ylib_name_space(ctx, module_child, ctx->models.list[i]->name);
                    } else if (!strcmp(module_child->name, "revision")) {
                        dnode = ylib_revision(ctx, module_child, ctx->models.list[i]->rev, ctx->models.list[i]->rev_size);
                    } else if (!strcmp(module_child->name, "schema")) {
                        dnode = ylib_schema(module_child);
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
                        ylib_append_children((struct lyd_node *)dlist, dnode);
                    }
                }

                if (dmodule) {
                    ylib_append_list(dmodule, dlist);
                } else {
                    dmodule = dlist;
                }
            }
        }
    }

    assert(dmodule);
    ylib_append_children(root, (struct lyd_node *)dmodule);

    dnode = NULL;
    modules_child = NULL;
    while ((modules_child = ylib_get_next_sibling(mod->data->next->child, modules_child))) {
        if (!strcmp(modules_child->name, "module-set-id")) {
            dnode = ylib_module_set_id(ctx, modules_child);
        }
    }

    assert(dnode);
    ylib_append_children(root, dnode);

    return root;
}
