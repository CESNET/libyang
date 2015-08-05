#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "libyang.h"
#include "resolve.h"
#include "common.h"
#include "parse.h"
#include "dict.h"
#include "tree_internal.h"

struct lys_tpdf *
resolve_superior_type(const char *name, const char *prefix, struct lys_module *module, struct lys_node *parent)
{
    int i, j, found = 0;
    struct lys_tpdf *tpdf;
    int tpdf_size;

    if (!prefix) {
        /* no prefix, try built-in types */
        for (i = 1; i < LY_DATA_TYPE_COUNT; i++) {
            if (!strcmp(ly_types[i].def->name, name)) {
                return ly_types[i].def;
            }
        }
    } else {
        if (!strcmp(prefix, module->prefix)) {
            /* prefix refers to the current module, ignore it */
            prefix = NULL;
        }
    }

    if (!prefix && parent) {
        /* search in local typedefs */
        while (parent) {
            switch (parent->nodetype) {
            case LYS_CONTAINER:
                tpdf_size = ((struct lys_node_container *)parent)->tpdf_size;
                tpdf = ((struct lys_node_container *)parent)->tpdf;
                break;

            case LYS_LIST:
                tpdf_size = ((struct lys_node_list *)parent)->tpdf_size;
                tpdf = ((struct lys_node_list *)parent)->tpdf;
                break;

            case LYS_GROUPING:
                tpdf_size = ((struct lys_node_grp *)parent)->tpdf_size;
                tpdf = ((struct lys_node_grp *)parent)->tpdf;
                break;

            case LYS_RPC:
                tpdf_size = ((struct lys_node_rpc *)parent)->tpdf_size;
                tpdf = ((struct lys_node_rpc *)parent)->tpdf;
                break;

            case LYS_NOTIF:
                tpdf_size = ((struct lys_node_notif *)parent)->tpdf_size;
                tpdf = ((struct lys_node_notif *)parent)->tpdf;
                break;

            case LYS_INPUT:
            case LYS_OUTPUT:
                tpdf_size = ((struct lys_node_rpc_inout *)parent)->tpdf_size;
                tpdf = ((struct lys_node_rpc_inout *)parent)->tpdf;
                break;

            default:
                parent = parent->parent;
                continue;
            }

            for (i = 0; i < tpdf_size; i++) {
                if (!strcmp(tpdf[i].name, name)) {
                    return &tpdf[i];
                }
            }

            parent = parent->parent;
        }
    } else if (prefix) {
        /* get module where to search */
        for (i = 0; i < module->imp_size; i++) {
            if (!strcmp(module->imp[i].prefix, prefix)) {
                module = module->imp[i].module;
                found = 1;
                break;
            }
        }
        if (!found) {
            return NULL;
        }
    }

    /* search in top level typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        if (!strcmp(module->tpdf[i].name, name)) {
            return &module->tpdf[i];
        }
    }

    /* search in submodules */
    for (i = 0; i < module->inc_size; i++) {
        for (j = 0; j < module->inc[i].submodule->tpdf_size; j++) {
            if (!strcmp(module->inc[i].submodule->tpdf[j].name, name)) {
                return &module->inc[i].submodule->tpdf[j];
            }
        }
    }

    return NULL;
}

static int
check_default(struct lys_type *type, const char *value)
{
    /* TODO - RFC 6020, sec. 7.3.4 */
    (void)type;
    (void)value;
    return EXIT_SUCCESS;
}

static int
check_key(struct lys_node_leaf *key, uint8_t flags, struct lys_node_leaf **list, int index, int line,
          const char *name, int len)
{
    char *dup = NULL;
    int j;

    /* existence */
    if (!key) {
        if (line > -1) {
            if (name[len] != '\0') {
                dup = strdup(name);
                dup[len] = '\0';
                name = dup;
            }
            LOGVAL(VE_KEY_MISS, line, name);
            free(dup);
        }
        return EXIT_FAILURE;
    }

    /* uniqueness */
    for (j = index - 1; j >= 0; j--) {
        if (list[index] == list[j]) {
            if (line > -1) {
                LOGVAL(VE_KEY_DUP, line, key->name);
            }
            return EXIT_FAILURE;
        }
    }

    /* key is a leaf */
    if (key->nodetype != LYS_LEAF) {
        if (line > -1) {
            LOGVAL(VE_KEY_NLEAF, line, key->name);
        }
        return EXIT_FAILURE;
    }

    /* type of the leaf is not built-in empty */
    if (key->type.base == LY_TYPE_EMPTY) {
        if (line > -1) {
            LOGVAL(VE_KEY_TYPE, line, key->name);
        }
        return EXIT_FAILURE;
    }

    /* config attribute is the same as of the list */
    if ((flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK)) {
        if (line > -1) {
            LOGVAL(VE_KEY_CONFIG, line, key->name);
        }
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
resolve_unique(struct lys_node *parent, const char *uniq_str, struct lys_unique *uniq_s, int line)
{
    char *uniq_val, *uniq_begin, *start;
    int i, j;

    /* count the number of unique values */
    uniq_val = uniq_begin = strdup(uniq_str);
    uniq_s->leafs_size = 0;
    while ((uniq_val = strpbrk(uniq_val, " \t\n"))) {
        uniq_s->leafs_size++;
        while (isspace(*uniq_val)) {
            uniq_val++;
        }
    }
    uniq_s->leafs_size++;
    uniq_s->leafs = calloc(uniq_s->leafs_size, sizeof *uniq_s->leafs);

    /* interconnect unique values with the leafs */
    uniq_val = uniq_begin;
    for (i = 0; uniq_val && i < uniq_s->leafs_size; i++) {
        start = uniq_val;
        if ((uniq_val = strpbrk(start, " \t\n"))) {
            *uniq_val = '\0'; /* add terminating NULL byte */
            uniq_val++;
            while (isspace(*uniq_val)) {
                uniq_val++;
            }
        } /* else only one nodeid present/left already NULL byte terminated */

        uniq_s->leafs[i] = (struct lys_node_leaf *)resolve_schema_nodeid(start, parent, parent->module, LYS_USES);
        if (!uniq_s->leafs[i] || uniq_s->leafs[i]->nodetype != LYS_LEAF) {
            if (line > -1) {
                LOGVAL(VE_INARG, line, start, "unique");
                if (!uniq_s->leafs[i]) {
                    LOGVAL(VE_SPEC, 0, "Target leaf not found.");
                } else {
                    LOGVAL(VE_SPEC, 0, "Target is not a leaf.");
                }
            }
            goto error;
        }

        for (j = 0; j < i; j++) {
            if (uniq_s->leafs[j] == uniq_s->leafs[i]) {
                if (line > -1) {
                    LOGVAL(VE_INARG, line, start, "unique");
                    LOGVAL(VE_SPEC, 0, "The identifier is not unique");
                }
                goto error;
            }
        }
    }

    free(uniq_begin);
    return EXIT_SUCCESS;

error:

    free(uniq_s->leafs);
    free(uniq_begin);

    return EXIT_FAILURE;
}

static int
resolve_grouping(struct lys_node *parent, struct lys_node_uses *uses, int line)
{
    struct lys_module *searchmod = NULL, *module = uses->module;
    struct lys_node *mnode, *mnode_aux;
    const char *name;
    int prefix_len = 0;
    int i;

    /* get referenced grouping */
    name = strchr(uses->name, ':');
    if (!name) {
        /* no prefix, search in local tree */
        name = uses->name;
    } else {
        /* there is some prefix, check if it refer the same data model */

        /* set name to correct position after colon */
        prefix_len = name - uses->name;
        name++;

        if (!strncmp(uses->name, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* prefix refers to the current module, ignore it */
            prefix_len = 0;
        }
    }

    /* search */
    if (prefix_len) {
        /* in top-level groupings of some other module */
        for (i = 0; i < module->imp_size; i++) {
            if (!strncmp(module->imp[i].prefix, uses->name, prefix_len)
                && !module->imp[i].prefix[prefix_len]) {
                searchmod = module->imp[i].module;
                break;
            }
        }
        if (!searchmod) {
            /* uses refers unknown data model */
            if (line > -1) {
                LOGVAL(VE_INPREFIX, line, name);
            }
            return EXIT_FAILURE;
        }

        LY_TREE_FOR(searchmod->data, mnode) {
            if (mnode->nodetype == LYS_GROUPING && !strcmp(mnode->name, name)) {
                uses->grp = (struct lys_node_grp *)mnode;
                return EXIT_SUCCESS;
            }
        }
    } else {
        /* in local tree hierarchy */
        for (mnode_aux = parent; mnode_aux; mnode_aux = mnode_aux->parent) {
            LY_TREE_FOR(mnode_aux->child, mnode) {
                if (mnode->nodetype == LYS_GROUPING && !strcmp(mnode->name, name)) {
                    uses->grp = (struct lys_node_grp *)mnode;
                    return EXIT_SUCCESS;
                }
            }
        }

        /* search in top level of the current module */
        LY_TREE_FOR(module->data, mnode) {
            if (mnode->nodetype == LYS_GROUPING && !strcmp(mnode->name, name)) {
                uses->grp = (struct lys_node_grp *)mnode;
                return EXIT_SUCCESS;
            }
        }

        /* search in top-level of included modules */
        for (i = 0; i < module->inc_size; i++) {
            LY_TREE_FOR(module->inc[i].submodule->data, mnode) {
                if (mnode->nodetype == LYS_GROUPING && !strcmp(mnode->name, name)) {
                    uses->grp = (struct lys_node_grp *)mnode;
                    return EXIT_SUCCESS;
                }
            }
        }
    }

    return EXIT_FAILURE;
}

static struct lys_feature *
resolve_feature(const char *name, struct lys_module *module, int line)
{
    const char *prefix;
    unsigned int prefix_len = 0;
    int i, j, found = 0;

    assert(name);
    assert(module);

    /* check prefix */
    prefix = name;
    name = strchr(prefix, ':');
    if (name) {
        /* there is prefix */
        prefix_len = name - prefix;
        name++;

        /* check whether the prefix points to the current module */
        if (!strncmp(prefix, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* then ignore prefix and works as there is no prefix */
            prefix_len = 0;
        }
    } else {
        /* no prefix, set pointers correctly */
        name = prefix;
    }

    if (prefix_len) {
        /* search in imported modules */
        for (i = 0; i < module->imp_size; i++) {
            if (!strncmp(module->imp[i].prefix, prefix, prefix_len) && !module->imp[i].prefix[prefix_len]) {
                module = module->imp[i].module;
                found = 1;
                break;
            }
        }
        if (!found) {
            /* identity refers unknown data model */
            if (line > -1) {
                LOGVAL(VE_INPREFIX, line, prefix);
            }
            return NULL;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            for (j = 0; j < module->inc[i].submodule->features_size; j++) {
                if (!strcmp(name, module->inc[i].submodule->features[j].name)) {
                    return &(module->inc[i].submodule->features[j]);
                }
            }
        }
    }

    /* search in the identified module */
    for (j = 0; j < module->features_size; j++) {
        if (!strcmp(name, module->features[j].name)) {
            return &module->features[j];
        }
    }

    /* not found */
    return NULL;
}

static struct lys_module *
resolve_import_in_includes_recursive(struct lys_module *mod, const char *prefix, uint32_t pref_len)
{
    int i, j;
    struct lys_submodule *sub_mod;
    struct lys_module *ret;

    for (i = 0; i < mod->inc_size; i++) {
        sub_mod = mod->inc[i].submodule;
        for (j = 0; j < sub_mod->imp_size; j++) {
            if ((pref_len == strlen(sub_mod->imp[j].prefix))
                    && !strncmp(sub_mod->imp[j].prefix, prefix, pref_len)) {
                return sub_mod->imp[j].module;
            }
        }
    }

    for (i = 0; i < mod->inc_size; i++) {
        ret = resolve_import_in_includes_recursive((struct lys_module *)mod->inc[i].submodule, prefix, pref_len);
        if (ret) {
            return ret;
        }
    }

    return NULL;
}

static struct lys_module *
resolve_prefixed_module(struct lys_module *mod, const char *prefix, uint32_t pref_len)
{
    int i;

    /* module itself */
    if (!strncmp(mod->prefix, prefix, pref_len) && mod->prefix[pref_len] == '\0') {
        return mod;
    }

    /* imported modules */
    for (i = 0; i < mod->imp_size; i++) {
        if (!strncmp(mod->imp[i].prefix, prefix, pref_len) && mod->imp[i].prefix[pref_len] == '\0') {
            return mod->imp[i].module;
        }
    }

    /* imports in includes */
    return resolve_import_in_includes_recursive(mod, prefix, pref_len);
}

struct lys_node *
resolve_child(struct lys_node *parent, const char *name, int len, LYS_NODE type)
{
    struct lys_node *child, *result;

    if (!len) {
        len = strlen(name);
    }

    LY_TREE_FOR(parent->child, child) {
        if (child->nodetype == LYS_USES) {
            /* search recursively */
            result = resolve_child(child, name, len, type);
            if (result) {
                return result;
            }
        }

        if (child->nodetype & type) {
            /* direct check */
            if (child->name == name || (!strncmp(child->name, name, len) && !child->name[len])) {
                return child;
            }
        }
    }

    return NULL;
}

/*
 * id - schema-nodeid
 *
 * node_type - LYS_AUGMENT (searches also RPCs and notifications)
 *           - LYS_USES    (only descendant-schema-nodeid allowed, ".." not allowed)
 *           - LYS_CHOICE  (search only start->child, only descendant-schema-nodeid allowed)
 */
struct lys_node *
resolve_schema_nodeid(const char *id, struct lys_node *start, struct lys_module *mod, LYS_NODE node_type)
{
    const char *name, *prefix;
    struct lys_node *sibling;
    int ret, nam_len, pref_len, is_relative = -1;
    struct lys_module *prefix_mod, *start_mod;
    /* 0 - in module, 1 - in 1st submodule, 2 - in 2nd submodule, ... */
    uint8_t in_submod = 0;
    /* 0 - in data, 1 - in RPCs, 2 - in notifications (relevant only with LYS_AUGMENT) */
    uint8_t in_mod_part = 0;

    assert(mod);
    assert(id);

    if ((ret = parse_schema_nodeid(id, &prefix, &pref_len, &name, &nam_len, &is_relative)) < 1) {
        return NULL;
    }
    id += ret;

    if (!is_relative && (node_type & (LYS_USES | LYS_CHOICE))) {
        return NULL;
    }

    /* absolute-schema-nodeid */
    if (!is_relative) {
        if (prefix) {
            start_mod = resolve_prefixed_module(mod, prefix, pref_len);
            if (!start_mod) {
                return NULL;
            }
            start = start_mod->data;
        } else {
            start = mod->data;
            start_mod = mod;
        }
    /* descendant-schema-nodeid */
    } else {
        assert(start);
        start = start->child;
        start_mod = start->module;
    }

    while (1) {
        sibling = NULL;
        LY_TREE_FOR(start, sibling) {
            /* name match */
            if ((sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len])
                    || (!strncmp(name, "input", 5) && (nam_len == 5) && (sibling->nodetype == LYS_INPUT))
                    || (!strncmp(name, "output", 6) && (nam_len == 6) && (sibling->nodetype == LYS_OUTPUT))) {

                /* prefix match check */
                if (prefix) {

                    prefix_mod = resolve_prefixed_module(mod, prefix, pref_len);
                    if (!prefix_mod) {
                        return NULL;
                    }

                    if (!sibling->module->type) {
                        if (prefix_mod != sibling->module) {
                            continue;
                        }
                    } else {
                        if (prefix_mod != ((struct lys_submodule *)sibling->module)->belongsto) {
                            continue;
                        }
                    }
                }

                /* the result node? */
                if (!id[0]) {
                    return sibling;
                }

                /* check for shorthand cases - then 'start' does not change */
                if (!sibling->parent || (sibling->parent->nodetype != LYS_CHOICE)
                        || (sibling->nodetype == LYS_CASE)) {
                    start = sibling->child;
                }
                break;
            }
        }

        /* we did not find the case in direct siblings */
        if (node_type == LYS_CHOICE) {
            return NULL;
        }

        /* no match */
        if (!sibling) {
            /* on augment search also RPCs and notifications, if we are in top-level */
            if ((node_type == LYS_AUGMENT) && (!start || !start->parent)) {
                /* we have searched all the data nodes */
                if (in_mod_part == 0) {
                    if (!in_submod) {
                        start = start_mod->rpc;
                    } else {
                        start = start_mod->inc[in_submod-1].submodule->rpc;
                    }
                    in_mod_part = 1;
                    continue;
                }
                /* we have searched all the RPCs */
                if (in_mod_part == 1) {
                    if (!in_submod) {
                        start = start_mod->notif;
                    } else {
                        start = start_mod->inc[in_submod-1].submodule->notif;
                    }
                    in_mod_part = 2;
                    continue;
                }
                /* we have searched all the notifications, nothing else to search in this module */
            }

            /* are we done with the included submodules as well? */
            if (in_submod == start_mod->inc_size) {
                return NULL;
            }

            /* we aren't, check the next one */
            ++in_submod;
            in_mod_part = 0;
            start = start_mod->inc[in_submod-1].submodule->data;
            continue;
        }

        /* we found our submodule */
        if (in_submod) {
            start_mod = (struct lys_module *)start_mod->inc[in_submod-1].submodule;
            in_submod = 0;
        }

        if ((ret = parse_schema_nodeid(id, &prefix, &pref_len, &name, &nam_len, &is_relative)) < 1) {
            return NULL;
        }
        id += ret;
    }

    /* cannot get here */
    return NULL;
}

static int
resolve_data_nodeid(const char *prefix, int pref_len, const char *name, int nam_len, struct lyd_node *data_source,
                    struct leafref_instid **parents)
{
    int flag;
    struct lys_module *mod;
    struct leafref_instid *item, *par_iter;
    struct lyd_node *node;

    if (prefix) {
        /* we have prefix, find appropriate module */
        mod = resolve_prefixed_module(data_source->schema->module, prefix, pref_len);
        if (!mod) {
            /* invalid prefix */
            return 1;
        }
    } else {
        /* no prefix, module is the same as of current node */
        mod = data_source->schema->module;
    }

    if (!*parents) {
        *parents = malloc(sizeof **parents);
        (*parents)->dnode = NULL;
        (*parents)->next = NULL;
    }
    for (par_iter = *parents; par_iter; par_iter = par_iter->next) {
        if (par_iter->dnode && (par_iter->dnode->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            /* skip */
            continue;
        }
        flag = 0;
        LY_TREE_FOR(par_iter->dnode ? par_iter->dnode->child : data_source, node) {
            if (node->schema->module == mod && !strncmp(node->schema->name, name, nam_len)
                    && node->schema->name[nam_len] == '\0') {
                /* matching target */
                if (!flag) {
                    /* replace leafref instead of the current parent */
                    par_iter->dnode = node;
                    flag = 1;
                } else {
                    /* multiple matching, so create new leafref structure */
                    item = malloc(sizeof *item);
                    item->dnode = node;
                    item->next = par_iter->next;
                    par_iter->next = item;
                    par_iter = par_iter->next;
                }
            }
        }
    }

    return !flag;
}

/* ... /node[source = destination] ... */
static int
resolve_path_predicate_data(const char *pred, struct leafref_instid **node_match)
{
    struct leafref_instid *source_match, *dest_match, *node, *node_prev = NULL;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times, i;

    do {
        if ((i = parse_path_predicate(pred, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            return -parsed+i;
        }
        parsed += i;
        pred += i;

        for (node = *node_match; node;) {
            /* source */
            source_match = NULL;
            /* must be leaf (key of a list) */
            if (resolve_data_nodeid(sour_pref, sour_pref_len, source, sour_len, node->dnode, &source_match)
                    || !source_match || source_match->next
                    || (source_match->dnode->schema->nodetype != LYS_LEAF)) {
                return -parsed;
            }

            /* destination */
            dest_match = calloc(1, sizeof *dest_match);
            dest_match->dnode = node->dnode;
            if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                            &dest_parent_times)) < 1) {
                return -parsed+i;
            }
            pke_parsed += i;
            for (i = 0; i < dest_parent_times; ++i) {
                dest_match->dnode = dest_match->dnode->parent;
                if (!dest_match->dnode) {
                    free(dest_match);
                    return -parsed;
                }
            }
            while (1) {
                if (resolve_data_nodeid(dest_pref, dest_pref_len, dest, dest_len, dest_match->dnode, &dest_match)
                        || !dest_match->dnode || dest_match->next) {
                    free(dest_match);
                    return -parsed;
                }

                if (pke_len == pke_parsed) {
                    break;
                }
                if ((i = parse_path_key_expr(path_key_expr+pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                             &dest_parent_times)) < 1) {
                    return -parsed+i;
                }
                pke_parsed += i;
            }

            /* check match between source and destination nodes */
            if (((struct lys_node_leaf *)source_match->dnode->schema)->type.base
                    != ((struct lys_node_leaf *)dest_match->dnode->schema)->type.base) {
                goto remove_leafref;
            }

            if (((struct lyd_node_leaf *)source_match->dnode)->value_str
                    != ((struct lyd_node_leaf *)dest_match->dnode)->value_str) {
                goto remove_leafref;
            }

            /* leafref is ok, continue check with next leafref */
            node_prev = node;
            node = node->next;
            continue;

remove_leafref:
            /* does not fulfill conditions, remove leafref record */
            if (node_prev) {
                node_prev->next = node->next;
                free(node);
                node = node_prev->next;
            } else {
                node = (*node_match)->next;
                free(*node_match);
                *node_match = node;
            }
        }
    } while (has_predicate);

    return parsed;
}

int
resolve_path_arg_data(struct leafref_instid *unres, const char *path, struct leafref_instid **ret)
{
    struct lyd_node *data;
    struct leafref_instid *riter = NULL, *raux;
    const char *prefix, *name;
    int pref_len, nam_len, has_predicate, parent_times, i;

    *ret = NULL;
    parent_times = 0;

    /* searching for nodeset */
    do {
        if ((i = parse_path_arg(path, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(DE_INCHAR, unres->line, path[-i], path-i);
            goto error;
        }
        path += i;

        if (!*ret) {
            *ret = calloc(1, sizeof **ret);
            for (i = 0; i < parent_times; ++i) {
                /* relative path */
                if (!*ret) {
                    /* error, too many .. */
                    LOGVAL(DE_INVAL, unres->line, path, unres->dnode->schema->name);
                    goto error;
                } else if (!(*ret)->dnode) {
                    /* first .. */
                    (*ret)->dnode = unres->dnode->parent;
                } else if (!(*ret)->dnode->parent) {
                    /* we are in root */
                    free(*ret);
                    *ret = NULL;
                } else {
                    /* multiple .. */
                    (*ret)->dnode = (*ret)->dnode->parent;
                }
            }

            /* absolute path */
            if (parent_times == -1) {
                for (data = unres->dnode; data->parent; data = data->parent);
                for (; data->prev->next; data = data->prev);
            }
        }

        /* node identifier */
        if (resolve_data_nodeid(prefix, pref_len, name, nam_len, data, ret)) {
            LOGVAL(DE_INELEMLEN, unres->line, nam_len, name);
            goto error;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be lists */
            for (raux = NULL, riter = *ret; riter; ) {
                if (riter->dnode->schema->nodetype == LYS_LIST &&
                        ((struct lys_node_list *)riter->dnode->schema)->keys) {
                    /* leafref is ok, continue check with next leafref */
                    raux = riter;
                    riter = riter->next;
                    continue;
                }

                /* does not fulfill conditions, remove leafref record */
                if (raux) {
                    raux->next = riter->next;
                    free(riter);
                    riter = raux->next;
                } else {
                    *ret = riter->next;
                    free(riter);
                    riter = *ret;
                }
            }
            if ((i = resolve_path_predicate_data(path, ret)) < 1) {
                LOGVAL(DE_INPRED, unres->line, path-i);
                goto error;
            }
            path += i;

            if (!*ret) {
                LOGVAL(DE_NORESOLV, unres->line, name);
                goto error;
            }
        }
    } while (path[0] != '\0');

    return 0;

error:

    while (*ret) {
        raux = (*ret)->next;
        free(*ret);
        *ret = raux;
    }

    return 1;
}

static int
resolve_path_predicate_schema(const char *pred, struct lys_module *mod, struct lys_node *source_node,
                              struct lys_node *dest_node, int line)
{
    struct lys_node *src_node, *dst_node;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times = 0, i;

    do {
        if ((i = parse_path_predicate(pred, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            if (line > -1) {
                LOGVAL(VE_NORESOLV, line, "predicate from part", pred-i);
            }
            return -parsed+i;
        }
        parsed += i;
        pred += i;

        /* source (must be leaf, from the same module) */
        if (sour_pref && (resolve_prefixed_module(mod, sour_pref, sour_pref_len) != mod)) {
            if (line > -1) {
                LOGVAL(VE_INPREF, line, sour_pref_len, sour_pref);
            }
            return -parsed;
        }

        src_node = resolve_child(source_node, source, sour_len, LYS_LEAF);
        if (!src_node) {
            if (line > -1) {
                LOGVAL(VE_NORESOLV, line, "child", source);
            }
            return -parsed;
        }

        /* destination */
        if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                     &dest_parent_times)) < 1) {
            if (line > -1) {
                LOGVAL(VE_NORESOLV, line, "predicate from part", path_key_expr-i);
            }
            return -parsed;
        }
        pke_parsed += i;

        /* dest_node is actually the parent of this leaf, so skip the first ".." */
        dst_node = dest_node;
        for (i = 1; i < dest_parent_times; ++i) {
            dst_node = dst_node->parent;
            if (!dst_node) {
                if (line > -1) {
                    LOGVAL(VE_NORESOLV, line, "predicate from part", path_key_expr);
                }
                return -parsed;
            }
        }
        while (1) {
            if (dest_pref && (resolve_prefixed_module(mod, dest_pref, dest_pref_len) != mod)) {
                if (line > -1) {
                    LOGVAL(VE_INPREF, line, dest_pref_len, dest_pref);
                }
                return -parsed;
            }
            dst_node = resolve_child(dst_node, dest, dest_len, LYS_CONTAINER | LYS_LIST | LYS_LEAF);
            if (!dst_node) {
                if (line > -1) {
                    LOGVAL(VE_NORESOLV, line, "child", dest);
                }
                return -parsed;
            }

            if (pke_len == pke_parsed) {
                break;
            }

            if ((i = parse_path_key_expr(path_key_expr+pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                         &dest_parent_times)) < 1) {
                if (line > -1) {
                    LOGVAL(VE_NORESOLV, line, "predicate from part", path_key_expr+pke_parsed-i);
                }
                return -parsed;
            }
            pke_parsed += i;
        }

        /* check source - dest match */
        if ((dst_node->nodetype != LYS_LEAF) || ((struct lys_node_leaf *)dst_node)->type.base
                != ((struct lys_node_leaf *)src_node)->type.base) {
            if (line > -1) {
                LOGVAL(VE_NORESOLV, line, "predicate", pred-parsed);
            }
            return -parsed;
        }
    } while (has_predicate);

    return parsed;
}

static int
resolve_path_arg_schema(struct lys_module *mod, const char *path, struct lys_node *parent_node, int line)
{
    struct lys_node *child, *node;
    const char *id, *prefix, *name;
    int pref_len, nam_len, parent_times, has_predicate;
    int i, first;

    first = 1;
    parent_times = 0;
    id = path;

    do {
        if ((i = parse_path_arg(id, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            if (line > -1) {
                LOGVAL(DE_INCHAR, line, id[-i], id-i);
            }
            return EXIT_FAILURE;
        }
        id += i;

        if (first) {
            if (parent_times == -1) {
                node = mod->data;
            } else if (parent_times > 0) {
                node = parent_node;
                for (i = 0; i < parent_times; ++i) {
                    node = node->parent;
                    if (!node) {
                        if (line > -1) {
                            LOGVAL(VE_NORESOLV, line, "leafref", path);
                        }
                        return EXIT_FAILURE;
                    }
                }
                node = node->child;
            }
            first = 0;
        } else {
            node = node->child;
        }

        /* node identifier */
        LY_TREE_FOR(node, child) {
            if (child->nodetype == LYS_GROUPING) {
                continue;
            }

            if (!strncmp(child->name, name, nam_len) && !child->name[nam_len]) {
                break;
            }
        }
        if (!child) {
            if (line > -1) {
                LOGVAL(VE_NORESOLV, line, "leafref", path);
            }
            return EXIT_FAILURE;
        }
        node = child;

        if (has_predicate) {
            /* we have predicate, so the current result must be list */
            if (node->nodetype != LYS_LIST) {
                if (line > -1) {
                    LOGVAL(VE_NORESOLV, line, "leafref", path);
                }
                return EXIT_FAILURE;
            }

            if ((i = resolve_path_predicate_schema(id, mod, node, parent_node, line)) < 1) {
                return EXIT_FAILURE;
            }
            id += i;
        }
    } while (id[0]);

    return EXIT_SUCCESS;
}

/* ... /node[target = value] ... */
static int
resolve_predicate(const char *pred, struct leafref_instid **node_match)
{
    struct leafref_instid *target_match, *node, *node_prev = NULL, *tmp;
    const char *prefix, *name, *value;
    int pref_len, nam_len, val_len, i, has_predicate, cur_idx, idx, parsed;

    idx = -1;
    parsed = 0;

    do {
        if ((i = parse_predicate(pred, &prefix, &pref_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1) {
            return -parsed+i;
        }
        parsed += i;
        pred += i;

        if (isdigit(name[0])) {
            idx = atoi(name);
        }

        for (cur_idx = 0, node = *node_match; node; ++cur_idx) {
            /* target */
            target_match = NULL;
            if ((name[0] == '.') || !value) {
                target_match = calloc(1, sizeof *target_match);
                target_match->dnode = node->dnode;
            } else if (resolve_data_nodeid(prefix, pref_len, name, nam_len, node->dnode, &target_match)) {
                return -parsed;
            }

            /* check that we have the correct type */
            if (name[0] == '.') {
                if (node->dnode->schema->nodetype != LYS_LEAFLIST) {
                    goto remove_instid;
                }
            } else if (value) {
                if (node->dnode->schema->nodetype != LYS_LIST) {
                    goto remove_instid;
                }
            }

            if ((value && (strncmp(((struct lyd_node_leaf *)target_match->dnode)->value_str, value, val_len)
                    || ((struct lyd_node_leaf *)target_match->dnode)->value_str[val_len]))
                    || (!value && (idx != cur_idx))) {
                goto remove_instid;
            }

            while (target_match) {
                tmp = target_match->next;
                free(target_match);
                target_match = tmp;
            }

            /* leafref is ok, continue check with next leafref */
            node_prev = node;
            node = node->next;
            continue;

remove_instid:
            while (target_match) {
                tmp = target_match->next;
                free(target_match);
                target_match = tmp;
            }

            /* does not fulfill conditions, remove leafref record */
            if (node_prev) {
                node_prev->next = node->next;
                free(node);
                node = node_prev->next;
            } else {
                node = (*node_match)->next;
                free(*node_match);
                *node_match = node;
            }
        }
    } while (has_predicate);

    return parsed;
}

int
resolve_instid(struct leafref_instid *unres, const char *path, int path_len, struct leafref_instid **ret)
{
    struct lyd_node *data;
    struct leafref_instid *riter = NULL, *raux;
    const char *apath = strndupa(path, path_len);
    const char *prefix, *name;
    int i, parsed, pref_len, nam_len, has_predicate;

    parsed = 0;

    /* we need root, absolute path */
    for (data = unres->dnode; data->parent; data = data->parent);
    for (; data->prev->next; data = data->prev);

    /* searching for nodeset */
    do {
        if ((i = parse_instance_identifier(apath, &prefix, &pref_len, &name, &nam_len, &has_predicate)) < 1) {
            LOGVAL(DE_INCHAR, unres->line, apath[-i], apath-i);
            goto error;
        }
        parsed += i;
        apath += i;

        if (resolve_data_nodeid(prefix, pref_len, name, nam_len, data, ret)) {
            LOGVAL(DE_INELEMLEN, unres->line, nam_len, name);
            goto error;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be list or leaf-list */
            for (raux = NULL, riter = *ret; riter; ) {
                if ((riter->dnode->schema->nodetype == LYS_LIST &&
                        ((struct lys_node_list *)riter->dnode->schema)->keys)
                        || (riter->dnode->schema->nodetype == LYS_LEAFLIST)) {
                    /* instid is ok, continue check with next instid */
                    raux = riter;
                    riter = riter->next;
                    continue;
                }

                /* does not fulfill conditions, remove inst record */
                if (raux) {
                    raux->next = riter->next;
                    free(riter);
                    riter = raux->next;
                } else {
                    *ret = riter->next;
                    free(riter);
                    riter = *ret;
                }
            }
            if ((i = resolve_predicate(apath, ret)) < 1) {
                LOGVAL(DE_INPRED, unres->line, apath-i);
                goto error;
            }
            parsed += i;
            apath += i;

            if (!*ret) {
                LOGVAL(DE_NORESOLV, unres->line, name);
                goto error;
            }
        }
    } while (apath[0] != '\0');

    return 0;

error:
    while (*ret) {
        raux = (*ret)->next;
        free(*ret);
        *ret = raux;
    }

    return 1;
}

static void
inherit_config_flag(struct lys_node *mnode)
{
    LY_TREE_FOR(mnode, mnode) {
        mnode->flags |= mnode->parent->flags & LYS_CONFIG_MASK;
        inherit_config_flag(mnode->child);
    }
}

static int
resolve_augment(struct lys_node_augment *aug, struct lys_node *parent, struct lys_module *module, int line)
{
    struct lys_node *sub, *aux;

    assert(module);

    /* resolve target node */
    aug->target = resolve_schema_nodeid(aug->target_name, parent, module, LYS_AUGMENT);
    if (!aug->target) {
        if (line > -1) {
            LOGVAL(VE_INARG, line, aug->target_name, "uses");
        }
        return EXIT_FAILURE;
    }

    if (!aug->child) {
        /* nothing to do */
        return EXIT_SUCCESS;
    }

    /* inherit config information from parent, augment does not have
     * config property, but we need to keep the information for subelements
     */
    aug->flags |= aug->target->flags & LYS_CONFIG_MASK;
    LY_TREE_FOR(aug->child, sub) {
        inherit_config_flag(sub);
    }

    /* reconnect augmenting data into the target - add them to the target child list */
    if (aug->target->child) {
        aux = aug->target->child->prev; /* remember current target's last node */
        aux->next = aug->child;         /* connect augmenting data after target's last node */
        aug->target->child->prev = aug->child->prev; /* new target's last node is last augmenting node */
        aug->child->prev = aux;         /* finish connecting of both child lists */
    } else {
        aug->target->child = aug->child;
    }

    return EXIT_SUCCESS;
}

int
resolve_uses(struct lys_node_uses *uses, int line, struct unres_item *unres)
{
    struct ly_ctx *ctx;
    struct lys_node *mnode = NULL, *mnode_aux;
    struct lys_refine *rfn;
    struct lys_restr *newmust;
    int i, j;
    uint8_t size;

    /* copy the data nodes from grouping into the uses context */
    LY_TREE_FOR(uses->grp->child, mnode) {
        mnode_aux = ly_mnode_dup(uses->module, mnode, uses->flags, 1, line, unres);
        if (!mnode_aux) {
            if (line > -1) {
                LOGVAL(VE_SPEC, line, "Copying data from grouping failed");
            }
            return EXIT_FAILURE;
        }
        if (ly_mnode_addchild((struct lys_node *)uses, mnode_aux)) {
            ly_mnode_free(mnode_aux);
            return EXIT_FAILURE;
        }
    }
    ctx = uses->module->ctx;

    /* apply refines */
    for (i = 0; i < uses->refine_size; i++) {
        rfn = &uses->refine[i];
        mnode = resolve_schema_nodeid(rfn->target_name, (struct lys_node *)uses, uses->module, LYS_USES);
        if (!mnode) {
            if (line > -1) {
                LOGVAL(VE_INARG, line, rfn->target_name, "uses");
            }
            return EXIT_FAILURE;
        }

        if (rfn->target_type && !(mnode->nodetype & rfn->target_type)) {
            if (line > -1) {
                LOGVAL(VE_SPEC, line, "refine substatements not applicable to the target-node");
            }
            return EXIT_FAILURE;
        }

        /* description on any nodetype */
        if (rfn->dsc) {
            lydict_remove(ctx, mnode->dsc);
            mnode->dsc = lydict_insert(ctx, rfn->dsc, 0);
        }

        /* reference on any nodetype */
        if (rfn->ref) {
            lydict_remove(ctx, mnode->ref);
            mnode->ref = lydict_insert(ctx, rfn->ref, 0);
        }

        /* config on any nodetype */
        if (rfn->flags & LYS_CONFIG_MASK) {
            mnode->flags &= ~LYS_CONFIG_MASK;
            mnode->flags |= (rfn->flags & LYS_CONFIG_MASK);
        }

        /* default value ... */
        if (rfn->mod.dflt) {
            if (mnode->nodetype == LYS_LEAF) {
                /* leaf */
                lydict_remove(ctx, ((struct lys_node_leaf *)mnode)->dflt);
                ((struct lys_node_leaf *)mnode)->dflt = lydict_insert(ctx, rfn->mod.dflt, 0);
            } else if (mnode->nodetype == LYS_CHOICE) {
                /* choice */
                ((struct lys_node_choice *)mnode)->dflt = resolve_schema_nodeid(rfn->mod.dflt, mnode, mnode->module, LYS_CHOICE);
                if (!((struct lys_node_choice *)mnode)->dflt) {
                    if (line > -1) {
                        LOGVAL(VE_INARG, line, rfn->mod.dflt, "default");
                    }
                    return EXIT_FAILURE;
                }
            }
        }

        /* mandatory on leaf, anyxml or choice */
        if (rfn->flags & LYS_MAND_MASK) {
            if (mnode->nodetype & (LYS_LEAF | LYS_ANYXML | LYS_CHOICE)) {
                /* remove current value */
                mnode->flags &= ~LYS_MAND_MASK;

                /* set new value */
                mnode->flags |= (rfn->flags & LYS_MAND_MASK);
            }
        }

        /* presence on container */
        if ((mnode->nodetype & LYS_CONTAINER) && rfn->mod.presence) {
            lydict_remove(ctx, ((struct lys_node_container *)mnode)->presence);
            ((struct lys_node_container *)mnode)->presence = lydict_insert(ctx, rfn->mod.presence, 0);
        }

        /* min/max-elements on list or leaf-list */
        /* magic - bit 3 in flags means min set, bit 4 says max set */
        if (mnode->nodetype == LYS_LIST) {
            if (rfn->flags & 0x04) {
                ((struct lys_node_list *)mnode)->min = rfn->mod.list.min;
            }
            if (rfn->flags & 0x08) {
                ((struct lys_node_list *)mnode)->max = rfn->mod.list.max;
            }
        } else if (mnode->nodetype == LYS_LEAFLIST) {
            if (rfn->flags & 0x04) {
                ((struct lys_node_leaflist *)mnode)->min = rfn->mod.list.min;
            }
            if (rfn->flags & 0x08) {
                ((struct lys_node_leaflist *)mnode)->max = rfn->mod.list.max;
            }
        }

        /* must in leaf, leaf-list, list, container or anyxml */
        if (rfn->must_size) {
            size = ((struct lys_node_leaf *)mnode)->must_size + rfn->must_size;
            newmust = realloc(((struct lys_node_leaf *)mnode)->must, size * sizeof *rfn->must);
            if (!newmust) {
                LOGMEM;
                return EXIT_FAILURE;
            }
            for (i = 0, j = ((struct lys_node_leaf *)mnode)->must_size; i < rfn->must_size; i++, j++) {
                newmust[j].expr = lydict_insert(ctx, rfn->must[i].expr, 0);
                newmust[j].dsc = lydict_insert(ctx, rfn->must[i].dsc, 0);
                newmust[j].ref = lydict_insert(ctx, rfn->must[i].ref, 0);
                newmust[j].eapptag = lydict_insert(ctx, rfn->must[i].eapptag, 0);
                newmust[j].emsg = lydict_insert(ctx, rfn->must[i].emsg, 0);
            }

            ((struct lys_node_leaf *)mnode)->must = newmust;
            ((struct lys_node_leaf *)mnode)->must_size = size;
        }
    }

    /* apply augments */
    for (i = 0; i < uses->augment_size; i++) {
        if (resolve_augment(&uses->augment[i], (struct lys_node *)uses, uses->module, line)) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

static struct lys_ident *
resolve_base_ident_sub(struct lys_module *module, struct lys_ident *ident, const char *basename)
{
    unsigned int i, j;
    struct lys_ident *base_iter = NULL;
    struct lys_ident_der *der;

    /* search module */
    for (i = 0; i < module->ident_size; i++) {
        if (!strcmp(basename, module->ident[i].name)) {

            if (!ident) {
                /* just search for type, so do not modify anything, just return
                 * the base identity pointer
                 */
                return &module->ident[i];
            }

            /* we are resolving identity definition, so now update structures */
            ident->base = base_iter = &module->ident[i];

            break;
        }
    }

    /* search submodules */
    if (!base_iter) {
        for (j = 0; j < module->inc_size; j++) {
            for (i = 0; i < module->inc[j].submodule->ident_size; i++) {
                if (!strcmp(basename, module->inc[j].submodule->ident[i].name)) {

                    if (!ident) {
                        return &module->inc[j].submodule->ident[i];
                    }

                    ident->base = base_iter = &module->inc[j].submodule->ident[i];
                    break;
                }
            }
        }
    }

    /* we found it somewhere */
    if (base_iter) {
        while (base_iter) {
            for (der = base_iter->der; der && der->next; der = der->next);
            if (der) {
                der->next = malloc(sizeof *der);
                der = der->next;
            } else {
                ident->base->der = der = malloc(sizeof *der);
            }
            der->next = NULL;
            der->ident = ident;

            base_iter = base_iter->base;
        }
        return ident->base;
    }

    return NULL;
}

static struct lys_ident *
resolve_base_ident(struct lys_module *module, struct lys_ident *ident, const char *basename, int line,
                   const char* parent)
{
    const char *name;
    int prefix_len = 0;
    int i, found = 0;
    struct lys_ident *result;

    /* search for the base identity */
    name = strchr(basename, ':');
    if (name) {
        /* set name to correct position after colon */
        prefix_len = name - basename;
        name++;

        if (!strncmp(basename, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* prefix refers to the current module, ignore it */
            prefix_len = 0;
        }
    } else {
        name = basename;
    }

    if (prefix_len) {
        /* get module where to search */
        for (i = 0; i < module->imp_size; i++) {
            if (!strncmp(module->imp[i].prefix, basename, prefix_len)
                && !module->imp[i].prefix[prefix_len]) {
                module = module->imp[i].module;
                found = 1;
                break;
            }
        }
        if (!found) {
            /* identity refers unknown data model */
            if (line > -1) {
                LOGVAL(VE_INPREFIX, line, basename);
            }
            return NULL;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            result = resolve_base_ident_sub((struct lys_module *)module->inc[i].submodule, ident, name);
            if (result) {
                return result;
            }
        }
    }

    /* search in the identified module */
    result = resolve_base_ident_sub(module, ident, name);
    if (!result && (line > -1)) {
        LOGVAL(VE_INARG, line, basename, parent);
    }

    return result;
}

struct lys_ident *
resolve_identityref(struct lys_ident *base, const char *name, const char *ns)
{
    struct lys_ident_der *der;

    if (!base || !name || !ns) {
        return NULL;
    }

    for(der = base->der; der; der = der->next) {
        if (!strcmp(der->ident->name, name) && ns == der->ident->module->ns) {
            /* we have match */
            return der->ident;
        }
    }

    /* not found */
    return NULL;
}

static int
resolve_unres_ident(struct lys_module *mod, struct lys_ident *ident, const char *base_name, int line)
{
    if (resolve_base_ident(mod, ident, base_name, (line == -1 ? -1 : 0), "ident")) {
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "identity", base_name);
    }
    return 1;
}

static int
resolve_unres_type_identref(struct lys_module *mod, struct lys_type *type, const char *base_name, int line)
{
    type->info.ident.ref = resolve_base_ident(mod, NULL, base_name, (line == -1 ? -1 : 0), "type");
    if (type->info.ident.ref) {
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "identityref", base_name);
    }
    return 1;
}

static int
resolve_unres_type_leafref(struct lys_module *mod, struct lys_type *type, struct lys_node *node, int line)
{
    if (!resolve_path_arg_schema(mod, type->info.lref.path, node, (line == -1 ? -1 : 0))) {
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "leafref", type->info.lref.path);
    }
    return 1;
}

static int
resolve_unres_type_der(struct lys_module *mod, struct lys_type *type, const char *type_name, int line)
{
    type->der = resolve_superior_type(type_name, type->prefix, mod, (struct lys_node *)type->der);
    if (type->der) {
        type->base = type->der->type.base;
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "type", type_name);
    }
    return 1;
}

static int
resolve_unres_augment(struct lys_module *mod, struct lys_node_augment *aug, int line)
{
    if (aug->parent->nodetype == LYS_USES) {
        if ((aug->target_name[0] != '/')
                && resolve_schema_nodeid(aug->target_name, aug->parent->child, mod, LYS_AUGMENT)) {
            return 0;
        }
    } else {
        if ((aug->target_name[0] == '/')
                && resolve_schema_nodeid(aug->target_name, mod->data, mod, LYS_AUGMENT)) {
            return 0;
        }
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "augment", aug->target_name);
    }
    return 1;
}

static int
resolve_unres_iffeature(struct lys_module *mod, struct lys_feature **feat_ptr, const char *feat_name, int line)
{
    *feat_ptr = resolve_feature(feat_name, mod, (line == -1 ? -1 : 0));
    if (*feat_ptr) {
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "if-feature", feat_name);
    }
    return 1;
}

static int
resolve_unres_uses(struct lys_node_uses *uses, int line, struct unres_item *unres)
{
    if (uses->grp || !resolve_grouping(uses->parent, uses, (line == -1 ? -1 : 0))) {
        if (!resolve_uses(uses, (line == -1 ? -1 : 0), unres)) {
            return 0;
        }
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "uses", uses->name);
    }
    return 1;
}

static int
resolve_unres_type_dflt(struct lys_type *type, const char *dflt, int line)
{
    if (!check_default(type, dflt)) {
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "type default", dflt);
    }
    return 1;
}

static int
resolve_unres_choice_dflt(struct lys_node_choice *choice, const char *dflt, int line)
{
    choice->dflt = resolve_child((struct lys_node *)choice, dflt, strlen(dflt), LYS_ANYXML | LYS_CASE
                                  | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST);
    if (choice->dflt) {
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "choice default", dflt);
    }
    return 1;
}

static int
resolve_unres_list_keys(struct lys_node_list *list, const char *keys_str, int line)
{
    int i, len;
    const char *value;

    for (i = 0; i < list->keys_size; ++i) {
        /* get the key name */
        if ((value = strpbrk(keys_str, " \t\n"))) {
            len = value - keys_str;
            while (isspace(value[0])) {
                value++;
            }
        } else {
            len = strlen(keys_str);
        }

        list->keys[i] = (struct lys_node_leaf *)resolve_child((struct lys_node *)list, keys_str, len, LYS_LEAF);

        if (check_key(list->keys[i], list->flags, list->keys, i, (line == -1 ? -1 : 0), keys_str, len)) {
            if (line > -1) {
                LOGVAL(VE_NORESOLV, line, "list keys", keys_str);
            }
            return 1;
        }

        /* prepare for next iteration */
        while (value && isspace(value[0])) {
            value++;
        }
        keys_str = value;
    }

    return 0;
}

static int
resolve_unres_list_uniq(struct lys_unique *uniq, const char *uniq_str, int line)
{
    if (!resolve_unique((struct lys_node *)uniq->leafs, uniq_str, uniq, (line == -1 ? -1 : 0))) {
        return 0;
    }

    if (line > -1) {
        LOGVAL(VE_NORESOLV, line, "list unique", uniq_str);
    }
    return 1;
}

static int
resolve_unres_when(struct lys_when *UNUSED(when), struct lys_node *UNUSED(start), int UNUSED(line))
{
    /* TODO */
    return 0;
}

static int
resolve_unres_must(struct lys_restr *UNUSED(must), struct lys_node *UNUSED(start), int UNUSED(line))
{
    /* TODO */
    return 0;
}

/* line == -1 means do not log, 0 means unknown */
static int
resolve_unres_item(struct lys_module *mod, void *item, enum UNRES_ITEM type, void *str_mnode, int line,
                   struct unres_item *unres)
{
    int ret = EXIT_FAILURE, has_str = 0;

    switch (type) {
    case UNRES_RESOLVED:
        LOGERR(LY_EINT, "Internal error (%s:%d).", __FILE__, __func__);
        break;
    case UNRES_IDENT:
        ret = resolve_unres_ident(mod, item, str_mnode, line);
        has_str = 1;
        break;
    case UNRES_TYPE_IDENTREF:
        ret = resolve_unres_type_identref(mod, item, str_mnode, line);
        has_str = 1;
        break;
    case UNRES_TYPE_LEAFREF:
        ret = resolve_unres_type_leafref(mod, item, str_mnode, line);
        has_str = 0;
        break;
    case UNRES_TYPE_DER:
        ret = resolve_unres_type_der(mod, item, str_mnode, line);
        has_str = 1;
        break;
    case UNRES_AUGMENT:
        ret = resolve_unres_augment(mod, item, line);
        has_str = 0;
        break;
    case UNRES_IFFEAT:
        ret = resolve_unres_iffeature(mod, item, str_mnode, line);
        has_str = 1;
        break;
    case UNRES_USES:
        ret = resolve_unres_uses(item, line, unres);
        has_str = 0;
        break;
    case UNRES_TYPE_DFLT:
        ret = resolve_unres_type_dflt(item, str_mnode, line);
        /* do not remove str_mnode (dflt), it's in a typedef */
        has_str = 0;
        break;
    case UNRES_CHOICE_DFLT:
        ret = resolve_unres_choice_dflt(item, str_mnode, line);
        has_str = 1;
        break;
    case UNRES_LIST_KEYS:
        ret = resolve_unres_list_keys(item, str_mnode, line);
        has_str = 1;
        break;
    case UNRES_LIST_UNIQ:
        ret = resolve_unres_list_uniq(item, str_mnode, line);
        has_str = 1;
        break;
    case UNRES_WHEN:
        ret = resolve_unres_when(item, str_mnode, line);
        has_str = 0;
        break;
    case UNRES_MUST:
        ret = resolve_unres_must(item, str_mnode, line);
        has_str = 0;
        break;
    }

    if (has_str && !ret) {
        lydict_remove(mod->ctx, str_mnode);
    }

    return ret;
}

int
resolve_unres(struct lys_module *mod, struct unres_item *unres)
{
    unsigned int i, resolved = 0;

    assert(unres);

    /* uses */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] != UNRES_USES) {
            continue;
        }
        if (!resolve_unres_item(mod, unres->item[i], unres->type[i], unres->str_mnode[i], unres->line[i], unres)) {
            unres->type[i] = UNRES_RESOLVED;
            ++resolved;
        }
    }

    /* augment */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] != UNRES_AUGMENT) {
            continue;
        }
        if (!resolve_unres_item(mod, unres->item[i], unres->type[i], unres->str_mnode[i], unres->line[i], unres)) {
            unres->type[i] = UNRES_RESOLVED;
            ++resolved;
        }
    }

    /* the rest */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] == UNRES_RESOLVED) {
            continue;
        }
        if (!resolve_unres_item(mod, unres->item[i], unres->type[i], unres->str_mnode[i], unres->line[i], unres)) {
            unres->type[i] = UNRES_RESOLVED;
            ++resolved;
        }
    }

    if (resolved < unres->count) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
add_unres_str(struct lys_module *mod, struct unres_item *unres, void *item, enum UNRES_ITEM type, const char *str,
              int line)
{
    str = lydict_insert(mod->ctx, str, 0);
    add_unres_mnode(mod, unres, item, type, (struct lys_node *)str, line);
}

void
add_unres_mnode(struct lys_module *mod, struct unres_item *unres, void *item, enum UNRES_ITEM type,
                struct lys_node *mnode, int line)
{
    assert(unres && item);

    if (!resolve_unres_item(mod, item, type, mnode, -1, unres)) {
        return;
    }

    unres->count++;
    unres->item = realloc(unres->item, unres->count*sizeof *unres->item);
    unres->item[unres->count-1] = item;
    unres->type = realloc(unres->type, unres->count*sizeof *unres->type);
    unres->type[unres->count-1] = type;
    unres->str_mnode = realloc(unres->str_mnode, unres->count*sizeof *unres->str_mnode);
    unres->str_mnode[unres->count-1] = mnode;
    unres->line = realloc(unres->line, unres->count*sizeof *unres->line);
    unres->line[unres->count-1] = line;
}

void
dup_unres(struct lys_module *mod, struct unres_item *unres, void *item, enum UNRES_ITEM type, void *new_item)
{
    int i;

    if (!item || !new_item) {
        return;
    }

    i = find_unres(unres, item, type);

    if (i == -1) {
        return;
    }

    if ((type == UNRES_TYPE_LEAFREF) || (type == UNRES_AUGMENT) || (type == UNRES_USES) || (type == UNRES_TYPE_DFLT)
            || (type == UNRES_WHEN) || (type == UNRES_MUST)) {
        add_unres_mnode(mod, unres, new_item, type, unres->str_mnode[i], 0);
    } else {
        add_unres_str(mod, unres, new_item, type, unres->str_mnode[i], 0);
    }
}

int
find_unres(struct unres_item *unres, void *item, enum UNRES_ITEM type)
{
    uint32_t ret = -1, i;

    for (i = 0; i < unres->count; ++i) {
        if ((unres->item[i] == item) && (unres->type[i] == type)) {
            ret = i;
            break;
        }
    }

    return ret;
}
